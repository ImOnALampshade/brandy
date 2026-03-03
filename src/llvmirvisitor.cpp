// -----------------------------------------------------------------------------
// Brandy LLVM IR code generator
// Howard Hughes
// -----------------------------------------------------------------------------

#include "llvmirvisitor.h"
#include "type.h"
#include "tokens.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------
  // Internal helpers
  // ---------------------------------------------------------------------------

  namespace
  {
    std::string tok_str(const token &t)
    {
      return std::string(t.text(), t.length());
    }

    bool tok_eq(const token &t, const char *s)
    {
      size_t len = std::strlen(s);
      return t.length() == len && std::memcmp(t.text(), s, len) == 0;
    }
  }

  // ---------------------------------------------------------------------------
  // Constructor / destructor
  // ---------------------------------------------------------------------------

  llvm_ir_generator::llvm_ir_generator(const std::string &module_name)
    : m_current_fn(nullptr)
  {
    m_ctx     = new llvm::LLVMContext();
    m_module  = new llvm::Module(module_name, *m_ctx);
    m_builder = new llvm::IRBuilder<>(*m_ctx);
  }

  llvm_ir_generator::~llvm_ir_generator()
  {
    delete m_builder;
    delete m_module;
    delete m_ctx;
  }

  // ---------------------------------------------------------------------------
  // Type helpers
  // ---------------------------------------------------------------------------

  llvm::Type *llvm_ir_generator::default_int_type()
  {
    return llvm::Type::getInt64Ty(*m_ctx);
  }

  llvm::Type *llvm_ir_generator::default_float_type()
  {
    return llvm::Type::getDoubleTy(*m_ctx);
  }

  llvm::Type *llvm_ir_generator::brandy_type_to_llvm(brandy::type *t)
  {
    if (!t)
      return default_int_type();

    using namespace brandy::builtin;

    if (t == &boolean) return llvm::Type::getInt1Ty(*m_ctx);
    if (t == &i8  || t == &ui8)  return llvm::Type::getInt8Ty(*m_ctx);
    if (t == &i16 || t == &ui16) return llvm::Type::getInt16Ty(*m_ctx);
    if (t == &i32 || t == &ui32) return llvm::Type::getInt32Ty(*m_ctx);
    if (t == &i64 || t == &ui64) return llvm::Type::getInt64Ty(*m_ctx);
    if (t == &f32) return llvm::Type::getFloatTy(*m_ctx);
    if (t == &f64) return llvm::Type::getDoubleTy(*m_ctx);
    if (t == &void_type) return llvm::Type::getVoidTy(*m_ctx);
    if (t == &string) return llvm::PointerType::get(llvm::Type::getInt8Ty(*m_ctx), 0);

    // Fallback for unknown / class types
    return default_int_type();
  }

  llvm::Type *llvm_ir_generator::type_ref_to_llvm(const type_reference &ref)
  {
    if (!ref.inner_type)
      return default_int_type();

    llvm::Type *base = brandy_type_to_llvm(ref.inner_type);

    // Walk qualifiers (pointer / array / reference)
    for (auto &mod : ref.qualifiers)
    {
      if (mod.modifier == type_modifiers::pointer ||
          mod.modifier == type_modifiers::reference)
      {
        base = llvm::PointerType::get(base, 0);
      }
      else if (mod.modifier == type_modifiers::array)
      {
        base = llvm::ArrayType::get(base, mod.array_size);
      }
    }

    return base;
  }

  llvm::Type *llvm_ir_generator::type_node_to_llvm(type_node *node)
  {
    if (!node)
      return default_int_type();

    // If the resulting_type was resolved by an earlier pass, use it.
    if (node->resulting_type)
      return type_ref_to_llvm(node->resulting_type);

    // Try to interpret a plain_type_node by name.
    if (auto *ptn = dynamic_cast<plain_type_node *>(node))
    {
      if (!ptn->name.empty())
      {
        std::string name = tok_str(ptn->name[0]);

        // Determine base type from name
        llvm::Type *base = nullptr;
        if      (name == "bool")                     base = llvm::Type::getInt1Ty(*m_ctx);
        else if (name == "i8"   || name == "byte")   base = llvm::Type::getInt8Ty(*m_ctx);
        else if (name == "i16"  || name == "short")  base = llvm::Type::getInt16Ty(*m_ctx);
        else if (name == "i32"  || name == "int")    base = llvm::Type::getInt32Ty(*m_ctx);
        else if (name == "i64"  || name == "long")   base = llvm::Type::getInt64Ty(*m_ctx);
        else if (name == "ui8"  || name == "ubyte")  base = llvm::Type::getInt8Ty(*m_ctx);
        else if (name == "ui16" || name == "ushort") base = llvm::Type::getInt16Ty(*m_ctx);
        else if (name == "ui32" || name == "uint")   base = llvm::Type::getInt32Ty(*m_ctx);
        else if (name == "ui64" || name == "ulong")  base = llvm::Type::getInt64Ty(*m_ctx);
        else if (name == "f32"  || name == "float")  base = llvm::Type::getFloatTy(*m_ctx);
        else if (name == "f64"  || name == "double") base = llvm::Type::getDoubleTy(*m_ctx);
        else if (name == "void")                     base = llvm::Type::getVoidTy(*m_ctx);
        else if (name == "string")                   base = llvm::PointerType::get(llvm::Type::getInt8Ty(*m_ctx), 0);
        else                                         base = default_int_type();

        // Apply pointer / reference / array post-type nodes
        for (auto &post : ptn->post_type)
        {
          if (dynamic_cast<type_indirect_node *>(post.get()))
            base = llvm::PointerType::get(base, 0);
          else if (auto *arr = dynamic_cast<type_array_node *>(post.get()))
          {
            // If the size expression is a known integer constant, use it;
            // otherwise fall back to a pointer (unsized array).
            if (arr->array_size)
            {
              if (auto *lit = dynamic_cast<literal_node *>(arr->array_size.get()))
              {
                std::string s = tok_str(lit->value);
                uint64_t sz = (uint64_t)std::stoull(s);
                base = llvm::ArrayType::get(base, sz);
              }
              else
                base = llvm::PointerType::get(base, 0);
            }
            else
              base = llvm::PointerType::get(base, 0);
          }
        }

        return base;
      }
    }

    return default_int_type();
  }

  llvm::Type *llvm_ir_generator::infer_type(expression_node *node)
  {
    if (!node)
      return default_int_type();

    // Use the resolved type if available
    if (node->resulting_type)
      return type_ref_to_llvm(node->resulting_type);

    if (auto *lit = dynamic_cast<literal_node *>(node))
    {
      switch (lit->value.type())
      {
      case token_types::I8_LITERAL:   return llvm::Type::getInt8Ty(*m_ctx);
      case token_types::I16_LITERAL:  return llvm::Type::getInt16Ty(*m_ctx);
      case token_types::I32_LITERAL:  return llvm::Type::getInt32Ty(*m_ctx);
      case token_types::I64_LITERAL:  return llvm::Type::getInt64Ty(*m_ctx);
      case token_types::UI8_LITERAL:  return llvm::Type::getInt8Ty(*m_ctx);
      case token_types::UI16_LITERAL: return llvm::Type::getInt16Ty(*m_ctx);
      case token_types::UI32_LITERAL: return llvm::Type::getInt32Ty(*m_ctx);
      case token_types::UI64_LITERAL: return llvm::Type::getInt64Ty(*m_ctx);
      case token_types::F32_LITERAL:  return llvm::Type::getFloatTy(*m_ctx);
      case token_types::F64_LITERAL:  return llvm::Type::getDoubleTy(*m_ctx);
      case token_types::TRUE:
      case token_types::FALSE:        return llvm::Type::getInt1Ty(*m_ctx);
      case token_types::STRING_LITERAL: return llvm::PointerType::get(llvm::Type::getInt8Ty(*m_ctx), 0);
      case token_types::NIL:          return llvm::PointerType::get(llvm::Type::getInt8Ty(*m_ctx), 0);
      default:                        return default_int_type();
      }
    }

    if (auto *ref = dynamic_cast<name_reference_node *>(node))
    {
      auto it = m_value_map.find(ref->resolved_symbol ? ref->resolved_symbol->node : nullptr);
      if (it != m_value_map.end())
      {
        // The alloca's allocated type is what we want
        if (auto *alloca = llvm::dyn_cast<llvm::AllocaInst>(it->second))
          return alloca->getAllocatedType();
      }
    }

    if (auto *binop = dynamic_cast<binary_operator_node *>(node))
    {
      llvm::Type *lt = infer_type(binop->left.get());
      llvm::Type *rt = infer_type(binop->right.get());
      if (lt->isDoubleTy() || rt->isDoubleTy()) return default_float_type();
      if (lt->isFloatTy()  || rt->isFloatTy())  return llvm::Type::getFloatTy(*m_ctx);
      return lt; // int types: use left type
    }

    return default_int_type();
  }

  bool llvm_ir_generator::is_float_value(llvm::Value *v)
  {
    return v && (v->getType()->isFloatTy() || v->getType()->isDoubleTy());
  }

  bool llvm_ir_generator::is_unsigned_type(brandy::type *t)
  {
    if (!t) return false;
    return t->check_flag_all(brandy::type::is_unsigned);
  }

  // ---------------------------------------------------------------------------
  // Entry-block alloca helper
  // ---------------------------------------------------------------------------

  llvm::AllocaInst *llvm_ir_generator::create_entry_alloca(const std::string &name, llvm::Type *ty)
  {
    assert(m_current_fn && "create_entry_alloca called outside a function");
    llvm::BasicBlock &entry = m_current_fn->getEntryBlock();
    llvm::IRBuilder<> tmp_builder(&entry, entry.begin());
    return tmp_builder.CreateAlloca(ty, nullptr, name);
  }

  // Promote two values to a common type for binary operations.
  // After this call, *lhs and *rhs have the same LLVM type.
  static void promote_to_common(llvm::IRBuilder<> *builder,
                                 llvm::Value *&lhs, llvm::Value *&rhs,
                                 bool is_unsigned)
  {
    llvm::Type *lt = lhs->getType();
    llvm::Type *rt = rhs->getType();
    if (lt == rt) return;

    // Both integer: promote to the wider type
    if (lt->isIntegerTy() && rt->isIntegerTy())
    {
      unsigned lbits = lt->getIntegerBitWidth();
      unsigned rbits = rt->getIntegerBitWidth();
      if (lbits < rbits)
        lhs = is_unsigned ? builder->CreateZExt(lhs, rt, "zext")
                          : builder->CreateSExt(lhs, rt, "sext");
      else
        rhs = is_unsigned ? builder->CreateZExt(rhs, lt, "zext")
                          : builder->CreateSExt(rhs, lt, "sext");
      return;
    }

    // One float, one int: promote int to float
    if (lt->isFloatingPointTy() && rt->isIntegerTy())
    {
      rhs = is_unsigned ? builder->CreateUIToFP(rhs, lt, "uitofp")
                        : builder->CreateSIToFP(rhs, lt, "sitofp");
      return;
    }
    if (rt->isFloatingPointTy() && lt->isIntegerTy())
    {
      lhs = is_unsigned ? builder->CreateUIToFP(lhs, rt, "uitofp")
                        : builder->CreateSIToFP(lhs, rt, "sitofp");
      return;
    }

    // Both float: promote to the wider type
    if (lt->isFloatingPointTy() && rt->isFloatingPointTy())
    {
      if (lt->getPrimitiveSizeInBits() < rt->getPrimitiveSizeInBits())
        lhs = builder->CreateFPExt(lhs, rt, "fpext");
      else
        rhs = builder->CreateFPExt(rhs, lt, "fpext");
    }
  }

  // ---------------------------------------------------------------------------
  // Literal code-gen
  // ---------------------------------------------------------------------------

  llvm::Value *llvm_ir_generator::codegen_literal(literal_node *node)
  {
    const token &tok = node->value;
    std::string text = tok_str(tok);

    switch (tok.type())
    {
    case token_types::I8_LITERAL:
      return llvm::ConstantInt::get(llvm::Type::getInt8Ty(*m_ctx),
                                    (int8_t)std::stoll(text), true);
    case token_types::I16_LITERAL:
      return llvm::ConstantInt::get(llvm::Type::getInt16Ty(*m_ctx),
                                    (int16_t)std::stoll(text), true);
    case token_types::I32_LITERAL:
      return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*m_ctx),
                                    (int32_t)std::stoll(text), true);
    case token_types::I64_LITERAL:
      return llvm::ConstantInt::get(llvm::Type::getInt64Ty(*m_ctx),
                                    std::stoll(text), true);
    case token_types::UI8_LITERAL:
      return llvm::ConstantInt::get(llvm::Type::getInt8Ty(*m_ctx),
                                    (uint8_t)std::stoull(text), false);
    case token_types::UI16_LITERAL:
      return llvm::ConstantInt::get(llvm::Type::getInt16Ty(*m_ctx),
                                    (uint16_t)std::stoull(text), false);
    case token_types::UI32_LITERAL:
      return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*m_ctx),
                                    (uint32_t)std::stoull(text), false);
    case token_types::UI64_LITERAL:
      return llvm::ConstantInt::get(llvm::Type::getInt64Ty(*m_ctx),
                                    std::stoull(text), false);
    case token_types::F32_LITERAL:
      return llvm::ConstantFP::get(llvm::Type::getFloatTy(*m_ctx),
                                   std::stod(text));
    case token_types::F64_LITERAL:
      return llvm::ConstantFP::get(llvm::Type::getDoubleTy(*m_ctx),
                                   std::stod(text));
    case token_types::TRUE:
      return llvm::ConstantInt::get(llvm::Type::getInt1Ty(*m_ctx), 1);
    case token_types::FALSE:
      return llvm::ConstantInt::get(llvm::Type::getInt1Ty(*m_ctx), 0);
    case token_types::NIL:
      return llvm::ConstantPointerNull::get(
        llvm::PointerType::get(llvm::Type::getInt8Ty(*m_ctx), 0));
    case token_types::STRING_LITERAL:
    {
      // Strip the surrounding quotes
      std::string str = text;
      if (str.size() >= 2 && str.front() == '"' && str.back() == '"')
        str = str.substr(1, str.size() - 2);
      return m_builder->CreateGlobalStringPtr(str, ".str");
    }
    case token_types::CHAR_LITERAL:
    {
      char ch = (text.size() >= 2) ? text[1] : 0;
      return llvm::ConstantInt::get(llvm::Type::getInt8Ty(*m_ctx), (uint8_t)ch);
    }
    default:
      // Plain integer literal (no suffix)
      return llvm::ConstantInt::get(default_int_type(), std::stoll(text), true);
    }
  }

  // ---------------------------------------------------------------------------
  // Name reference code-gen
  // ---------------------------------------------------------------------------

  llvm::Value *llvm_ir_generator::codegen_lvalue(expression_node *node)
  {
    if (auto *ref = dynamic_cast<name_reference_node *>(node))
    {
      abstract_node *key = ref->resolved_symbol ? ref->resolved_symbol->node : nullptr;
      auto it = m_value_map.find(key);
      if (it != m_value_map.end())
        return it->second;
    }
    return nullptr;
  }

  llvm::Value *llvm_ir_generator::codegen_name_ref(name_reference_node *node)
  {
    abstract_node *key = node->resolved_symbol ? node->resolved_symbol->node : nullptr;
    auto it = m_value_map.find(key);
    if (it == m_value_map.end())
    {
      std::cerr << "Warning: unresolved name '" << tok_str(node->name) << "'\n";
      return llvm::ConstantInt::get(default_int_type(), 0);
    }

    llvm::Value *ptr = it->second;

    // If the value is an alloca or global, load from it.
    if (auto *alloca = llvm::dyn_cast<llvm::AllocaInst>(ptr))
      return m_builder->CreateLoad(alloca->getAllocatedType(), ptr, tok_str(node->name));

    if (auto *gv = llvm::dyn_cast<llvm::GlobalVariable>(ptr))
      return m_builder->CreateLoad(gv->getValueType(), ptr, tok_str(node->name));

    // Otherwise it's already a value (e.g., a function argument).
    return ptr;
  }

  // ---------------------------------------------------------------------------
  // Binary operator code-gen
  // ---------------------------------------------------------------------------

  llvm::Value *llvm_ir_generator::codegen_binary_op(binary_operator_node *node)
  {
    token_types::type op = node->operation.type();

    // Handle assignment specially: we need the lvalue address of the left side.
    if (op == token_types::ASSIGNMENT)
    {
      llvm::Value *rhs  = codegen_expr(node->right.get());
      llvm::Value *lval = codegen_lvalue(node->left.get());

      // Handle implicit variable declaration (e.g. `x = expr` at module level
      // or inside a function, without a prior `var x` declaration).
      if (!lval && rhs)
      {
        if (auto *ref = dynamic_cast<name_reference_node *>(node->left.get()))
        {
          std::string vname = tok_str(ref->name);
          abstract_node *key = ref->resolved_symbol ? ref->resolved_symbol->node : nullptr;

          if (m_current_fn)
          {
            // Local implicit variable
            llvm::AllocaInst *alloca = create_entry_alloca(vname, rhs->getType());
            m_value_map[key] = alloca;
            lval = alloca;
          }
          else
          {
            // Global implicit variable
            llvm::Constant *init = llvm::Constant::getNullValue(rhs->getType());
            auto *gv = new llvm::GlobalVariable(*m_module, rhs->getType(), false,
                                                llvm::GlobalValue::InternalLinkage,
                                                init, vname);
            m_value_map[key] = gv;
            lval = gv;
          }
        }
      }

      if (lval && rhs)
        m_builder->CreateStore(rhs, lval);
      return rhs;
    }

    llvm::Value *lhs = codegen_expr(node->left.get());
    llvm::Value *rhs = codegen_expr(node->right.get());
    if (!lhs || !rhs)
      return llvm::ConstantInt::get(default_int_type(), 0);

    // Determine signedness from resolved type (best-effort)
    bool is_unsigned = false;
    if (node->left->resulting_type.inner_type)
      is_unsigned = is_unsigned_type(node->left->resulting_type.inner_type);

    // Promote both operands to a common type.
    promote_to_common(m_builder, lhs, rhs, is_unsigned);

    bool is_fp = is_float_value(lhs);

    switch (op)
    {
    // Arithmetic
    case token_types::ADD:
      return is_fp ? m_builder->CreateFAdd(lhs, rhs, "fadd")
                   : m_builder->CreateAdd(lhs, rhs, "add");
    case token_types::SUBTRACT:
      return is_fp ? m_builder->CreateFSub(lhs, rhs, "fsub")
                   : m_builder->CreateSub(lhs, rhs, "sub");
    case token_types::ASTRISK:
      return is_fp ? m_builder->CreateFMul(lhs, rhs, "fmul")
                   : m_builder->CreateMul(lhs, rhs, "mul");
    case token_types::DIVIDE:
      if (is_fp) return m_builder->CreateFDiv(lhs, rhs, "fdiv");
      return is_unsigned ? m_builder->CreateUDiv(lhs, rhs, "udiv")
                         : m_builder->CreateSDiv(lhs, rhs, "sdiv");
    case token_types::MODULO:
      if (is_fp) return m_builder->CreateFRem(lhs, rhs, "frem");
      return is_unsigned ? m_builder->CreateURem(lhs, rhs, "urem")
                         : m_builder->CreateSRem(lhs, rhs, "srem");

    // Bitwise
    case token_types::AMPERSAND:     return m_builder->CreateAnd(lhs, rhs, "and");
    case token_types::BITWISE_OR:    return m_builder->CreateOr(lhs, rhs, "or");
    case token_types::BITWISE_XOR:   return m_builder->CreateXor(lhs, rhs, "xor");
    case token_types::BITWISE_LEFT_SHIFT:
      return m_builder->CreateShl(lhs, rhs, "shl");
    case token_types::BITWISE_RIGHT_SHIFT:
      return is_unsigned ? m_builder->CreateLShr(lhs, rhs, "lshr")
                         : m_builder->CreateAShr(lhs, rhs, "ashr");

    // Logical (short-circuit not implemented; generate bitwise for now)
    case token_types::LOGICAL_AND:   return m_builder->CreateAnd(lhs, rhs, "land");
    case token_types::LOGICAL_OR:    return m_builder->CreateOr(lhs, rhs, "lor");

    // Comparison
    case token_types::EQUALITY:
      return is_fp ? m_builder->CreateFCmpOEQ(lhs, rhs, "fcmpeq")
                   : m_builder->CreateICmpEQ(lhs, rhs, "icmpeq");
    case token_types::INEQUALITY:
      return is_fp ? m_builder->CreateFCmpONE(lhs, rhs, "fcmpne")
                   : m_builder->CreateICmpNE(lhs, rhs, "icmpne");
    case token_types::GREATER_THAN:
      if (is_fp)       return m_builder->CreateFCmpOGT(lhs, rhs, "fcmpgt");
      return is_unsigned ? m_builder->CreateICmpUGT(lhs, rhs, "ucmpgt")
                         : m_builder->CreateICmpSGT(lhs, rhs, "scmpgt");
    case token_types::LESS_THAN:
      if (is_fp)       return m_builder->CreateFCmpOLT(lhs, rhs, "fcmplt");
      return is_unsigned ? m_builder->CreateICmpULT(lhs, rhs, "ucmplt")
                         : m_builder->CreateICmpSLT(lhs, rhs, "scmplt");
    case token_types::GREATER_THAN_OR_EQUAL:
      if (is_fp)       return m_builder->CreateFCmpOGE(lhs, rhs, "fcmpge");
      return is_unsigned ? m_builder->CreateICmpUGE(lhs, rhs, "ucmpge")
                         : m_builder->CreateICmpSGE(lhs, rhs, "scmpge");
    case token_types::LESS_THAN_OR_EQUAL:
      if (is_fp)       return m_builder->CreateFCmpOLE(lhs, rhs, "fcmple");
      return is_unsigned ? m_builder->CreateICmpULE(lhs, rhs, "ucmple")
                         : m_builder->CreateICmpSLE(lhs, rhs, "scmple");

    // Compound assignment: op-then-assign
    case token_types::ASSIGNMENT_ADD:
    case token_types::ASSIGNMENT_SUBTRACT:
    case token_types::ASSIGNMENT_MULTIPLY:
    case token_types::ASSIGNMENT_DIVIDE:
    case token_types::ASSIGNMENT_MODULO:
    case token_types::ASSIGNMENT_BITWISE_AND:
    case token_types::ASSIGNMENT_BITWISE_OR:
    case token_types::ASSIGNMENT_BITWISE_XOR:
    case token_types::ASSIGNMENT_BITWISE_LEFT_SHIFT:
    case token_types::ASSIGNMENT_BITWISE_RIGHT_SHIFT:
    {
      // Create a synthetic binary_operator_node with the plain op
      binary_operator_node synthetic;
      // We already have lhs/rhs values; compute result then store
      token plain_op;
      switch (op)
      {
      case token_types::ASSIGNMENT_ADD:                plain_op = token("+", token_types::ADD); break;
      case token_types::ASSIGNMENT_SUBTRACT:           plain_op = token("-", token_types::SUBTRACT); break;
      case token_types::ASSIGNMENT_MULTIPLY:           plain_op = token("*", token_types::ASTRISK); break;
      case token_types::ASSIGNMENT_DIVIDE:             plain_op = token("/", token_types::DIVIDE); break;
      case token_types::ASSIGNMENT_MODULO:             plain_op = token("%", token_types::MODULO); break;
      case token_types::ASSIGNMENT_BITWISE_AND:        plain_op = token("&", token_types::AMPERSAND); break;
      case token_types::ASSIGNMENT_BITWISE_OR:         plain_op = token("|", token_types::BITWISE_OR); break;
      case token_types::ASSIGNMENT_BITWISE_XOR:        plain_op = token("^", token_types::BITWISE_XOR); break;
      case token_types::ASSIGNMENT_BITWISE_LEFT_SHIFT: plain_op = token("<<", token_types::BITWISE_LEFT_SHIFT); break;
      case token_types::ASSIGNMENT_BITWISE_RIGHT_SHIFT:plain_op = token(">>", token_types::BITWISE_RIGHT_SHIFT); break;
      default: break;
      }
      // Build a temporary node with values we already computed
      // Instead, create the combined operation manually
      binary_operator_node inner_node;
      inner_node.operation = plain_op;
      // We can't set .left/.right to already-computed values easily,
      // so recompute from scratch with the inner op using lhs/rhs.
      llvm::Value *result = nullptr;
      switch (plain_op.type())
      {
      case token_types::ADD:      result = is_fp ? m_builder->CreateFAdd(lhs,rhs,"fadd") : m_builder->CreateAdd(lhs,rhs,"add"); break;
      case token_types::SUBTRACT: result = is_fp ? m_builder->CreateFSub(lhs,rhs,"fsub") : m_builder->CreateSub(lhs,rhs,"sub"); break;
      case token_types::ASTRISK:  result = is_fp ? m_builder->CreateFMul(lhs,rhs,"fmul") : m_builder->CreateMul(lhs,rhs,"mul"); break;
      case token_types::DIVIDE:   result = is_fp ? m_builder->CreateFDiv(lhs,rhs,"fdiv") : (is_unsigned ? m_builder->CreateUDiv(lhs,rhs,"udiv") : m_builder->CreateSDiv(lhs,rhs,"sdiv")); break;
      case token_types::MODULO:   result = is_fp ? m_builder->CreateFRem(lhs,rhs,"frem") : (is_unsigned ? m_builder->CreateURem(lhs,rhs,"urem") : m_builder->CreateSRem(lhs,rhs,"srem")); break;
      case token_types::AMPERSAND:          result = m_builder->CreateAnd(lhs,rhs,"and"); break;
      case token_types::BITWISE_OR:         result = m_builder->CreateOr(lhs,rhs,"or");  break;
      case token_types::BITWISE_XOR:        result = m_builder->CreateXor(lhs,rhs,"xor"); break;
      case token_types::BITWISE_LEFT_SHIFT: result = m_builder->CreateShl(lhs,rhs,"shl"); break;
      case token_types::BITWISE_RIGHT_SHIFT:result = is_unsigned ? m_builder->CreateLShr(lhs,rhs,"lshr") : m_builder->CreateAShr(lhs,rhs,"ashr"); break;
      default: result = lhs; break;
      }
      llvm::Value *lval = codegen_lvalue(node->left.get());
      if (lval && result)
        m_builder->CreateStore(result, lval);
      return result;
    }

    default:
      std::cerr << "Warning: unhandled binary operator\n";
      return lhs;
    }
  }

  // ---------------------------------------------------------------------------
  // Unary operator code-gen
  // ---------------------------------------------------------------------------

  llvm::Value *llvm_ir_generator::codegen_unary_op(unary_operator_node *node)
  {
    llvm::Value *val = codegen_expr(node->expression.get());
    if (!val)
      return llvm::ConstantInt::get(default_int_type(), 0);

    switch (node->operation.type())
    {
    case token_types::SUBTRACT:
      return is_float_value(val) ? m_builder->CreateFNeg(val, "fneg")
                                 : m_builder->CreateNeg(val, "neg");
    case token_types::BITWISE_NOT:
      return m_builder->CreateNot(val, "bitnot");
    case token_types::LOGICAL_NOT:
    {
      // Compare with zero, then extend to i1
      llvm::Value *zero = llvm::ConstantInt::get(val->getType(), 0);
      return m_builder->CreateICmpEQ(val, zero, "lnot");
    }
    default:
      return val;
    }
  }

  // ---------------------------------------------------------------------------
  // Call code-gen
  // ---------------------------------------------------------------------------

  llvm::Value *llvm_ir_generator::codegen_call(call_node *node)
  {
    // Collect argument values (evaluate first so we can determine types)
    std::vector<llvm::Value *> args;
    for (auto &param : node->parameters)
    {
      if (auto *v = codegen_expr(param.get()))
        args.push_back(v);
    }

    // Callee is a plain name reference?
    if (auto *name_ref = dynamic_cast<name_reference_node *>(node->left.get()))
    {
      std::string fn_name = tok_str(name_ref->name);
      auto it = m_fn_map.find(fn_name);
      if (it != m_fn_map.end())
      {
        llvm::Function *fn = it->second;
        // Coerce argument types if needed
        std::vector<llvm::Value *> coerced;
        unsigned idx = 0;
        for (auto &arg : fn->args())
        {
          if (idx < args.size())
          {
            llvm::Value *v = args[idx];
            if (v->getType() != arg.getType())
            {
              // Try simple casts
              if (arg.getType()->isIntegerTy() && v->getType()->isIntegerTy())
                v = m_builder->CreateIntCast(v, arg.getType(), true, "icast");
              else if (arg.getType()->isFloatingPointTy() && v->getType()->isIntegerTy())
                v = m_builder->CreateSIToFP(v, arg.getType(), "sitofp");
              else if (arg.getType()->isIntegerTy() && v->getType()->isFloatingPointTy())
                v = m_builder->CreateFPToSI(v, arg.getType(), "fptosi");
            }
            coerced.push_back(v);
          }
          ++idx;
        }
        if (fn->getReturnType()->isVoidTy())
        {
          m_builder->CreateCall(fn, coerced);
          return nullptr;
        }
        return m_builder->CreateCall(fn, coerced, "call");
      }

      // Declare as external and call (e.g. print, or C library functions)
      llvm::Type *ret_ty = default_int_type();
      std::vector<llvm::Type *> param_tys;
      for (auto *a : args) param_tys.push_back(a->getType());
      llvm::FunctionType *ft = llvm::FunctionType::get(ret_ty, param_tys, false);
      llvm::FunctionCallee callee = m_module->getOrInsertFunction(fn_name, ft);
      return m_builder->CreateCall(callee, args, "extcall");
    }

    // Callee is a member access — may be an operator rewrite from the
    // bin_op_replacer, or a genuine method call.
    if (auto *mem = dynamic_cast<member_access_node *>(node->left.get()))
    {
      std::string method = tok_str(mem->member_name);

      // Generate the "object" (left-hand side of the dot)
      llvm::Value *obj = codegen_expr(mem->left.get());
      if (!obj) obj = llvm::ConstantInt::get(default_int_type(), 0);

      // Single argument (from the replaced binary op)
      llvm::Value *arg = args.empty() ? nullptr : args[0];

      // Promote obj and arg to a common type
      if (arg)
        promote_to_common(m_builder, obj, arg, false);

      bool is_fp = is_float_value(obj);

      // Arithmetic operators
      if      (method == "@add")        return is_fp ? m_builder->CreateFAdd(obj,arg,"fadd") : m_builder->CreateAdd(obj,arg,"add");
      else if (method == "@subtract")   return is_fp ? m_builder->CreateFSub(obj,arg,"fsub") : m_builder->CreateSub(obj,arg,"sub");
      else if (method == "@astrisk")    return is_fp ? m_builder->CreateFMul(obj,arg,"fmul") : m_builder->CreateMul(obj,arg,"mul");
      else if (method == "@divide")     return is_fp ? m_builder->CreateFDiv(obj,arg,"fdiv") : m_builder->CreateSDiv(obj,arg,"sdiv");
      else if (method == "@modulo")     return is_fp ? m_builder->CreateFRem(obj,arg,"frem") : m_builder->CreateSRem(obj,arg,"srem");
      // Bitwise operators
      else if (method == "@ampersand")  return m_builder->CreateAnd(obj,arg,"and");
      else if (method == "@bitwise_or") return m_builder->CreateOr(obj,arg,"or");
      else if (method == "@bitwise_xor")return m_builder->CreateXor(obj,arg,"xor");
      else if (method == "@bitwise_left_shift")  return m_builder->CreateShl(obj,arg,"shl");
      else if (method == "@bitwise_right_shift") return m_builder->CreateAShr(obj,arg,"ashr");
      // Logical operators
      else if (method == "@logical_and")return m_builder->CreateAnd(obj,arg,"land");
      else if (method == "@logical_or") return m_builder->CreateOr(obj,arg,"lor");
      // Comparison operators
      else if (method == "@equality")   return is_fp ? m_builder->CreateFCmpOEQ(obj,arg,"fcmpeq") : m_builder->CreateICmpEQ(obj,arg,"icmpeq");
      else if (method == "@inequality") return is_fp ? m_builder->CreateFCmpONE(obj,arg,"fcmpne") : m_builder->CreateICmpNE(obj,arg,"icmpne");
      else if (method == "@greater_than")        return is_fp ? m_builder->CreateFCmpOGT(obj,arg,"fcmpgt") : m_builder->CreateICmpSGT(obj,arg,"scmpgt");
      else if (method == "@less_than")           return is_fp ? m_builder->CreateFCmpOLT(obj,arg,"fcmplt") : m_builder->CreateICmpSLT(obj,arg,"scmplt");
      else if (method == "@greater_than_or_equal") return is_fp ? m_builder->CreateFCmpOGE(obj,arg,"fcmpge") : m_builder->CreateICmpSGE(obj,arg,"scmpge");
      else if (method == "@less_than_or_equal")  return is_fp ? m_builder->CreateFCmpOLE(obj,arg,"fcmple") : m_builder->CreateICmpSLE(obj,arg,"scmple");
      // Assignment operator
      else if (method == "@assign")
      {
        llvm::Value *lval = codegen_lvalue(mem->left.get());
        if (lval && arg)
          m_builder->CreateStore(arg, lval);
        return arg;
      }
      // Compound assignment operators
      else if (method == "@assign_add")
      {
        llvm::Value *res = is_fp ? m_builder->CreateFAdd(obj,arg,"fadd") : m_builder->CreateAdd(obj,arg,"add");
        llvm::Value *lval = codegen_lvalue(mem->left.get());
        if (lval) m_builder->CreateStore(res, lval);
        return res;
      }
      else if (method == "@assign_subtract")
      {
        llvm::Value *res = is_fp ? m_builder->CreateFSub(obj,arg,"fsub") : m_builder->CreateSub(obj,arg,"sub");
        llvm::Value *lval = codegen_lvalue(mem->left.get());
        if (lval) m_builder->CreateStore(res, lval);
        return res;
      }
      else if (method == "@assign_multiply")
      {
        llvm::Value *res = is_fp ? m_builder->CreateFMul(obj,arg,"fmul") : m_builder->CreateMul(obj,arg,"mul");
        llvm::Value *lval = codegen_lvalue(mem->left.get());
        if (lval) m_builder->CreateStore(res, lval);
        return res;
      }

      // Unknown method: emit a warning and return zero
      std::cerr << "Warning: unhandled member call '" << method << "'\n";
      return llvm::ConstantInt::get(default_int_type(), 0);
    }

    // Fallback
    std::cerr << "Warning: unhandled call expression\n";
    return llvm::ConstantInt::get(default_int_type(), 0);
  }

  // ---------------------------------------------------------------------------
  // Cast code-gen
  // ---------------------------------------------------------------------------

  llvm::Value *llvm_ir_generator::codegen_cast(cast_node *node)
  {
    llvm::Value *src = codegen_expr(node->left.get());
    if (!src) return llvm::ConstantInt::get(default_int_type(), 0);

    llvm::Type *dst_ty = type_node_to_llvm(node->type.get());
    if (src->getType() == dst_ty)
      return src;

    if (dst_ty->isIntegerTy() && src->getType()->isIntegerTy())
      return m_builder->CreateIntCast(src, dst_ty, true, "icast");
    if (dst_ty->isFloatingPointTy() && src->getType()->isIntegerTy())
      return m_builder->CreateSIToFP(src, dst_ty, "sitofp");
    if (dst_ty->isIntegerTy() && src->getType()->isFloatingPointTy())
      return m_builder->CreateFPToSI(src, dst_ty, "fptosi");
    if (dst_ty->isFloatingPointTy() && src->getType()->isFloatingPointTy())
      return m_builder->CreateFPCast(src, dst_ty, "fpcast");
    if (dst_ty->isPointerTy())
      return m_builder->CreateBitCast(src, dst_ty, "ptrcast");

    return src;
  }

  // ---------------------------------------------------------------------------
  // Generic expression dispatcher
  // ---------------------------------------------------------------------------

  llvm::Value *llvm_ir_generator::codegen_expr(expression_node *node)
  {
    if (!node) return nullptr;

    if (auto *n = dynamic_cast<literal_node *>(node))
      return codegen_literal(n);

    if (auto *n = dynamic_cast<name_reference_node *>(node))
      return codegen_name_ref(n);

    if (auto *n = dynamic_cast<binary_operator_node *>(node))
      return codegen_binary_op(n);

    if (auto *n = dynamic_cast<unary_operator_node *>(node))
      return codegen_unary_op(n);

    if (auto *n = dynamic_cast<call_node *>(node))
      return codegen_call(n);

    if (auto *n = dynamic_cast<cast_node *>(node))
      return codegen_cast(n);

    // member_access_node on its own (not part of a call) — return object
    if (auto *n = dynamic_cast<member_access_node *>(node))
      return codegen_expr(n->left.get());

    std::cerr << "Warning: unhandled expression node\n";
    return llvm::ConstantInt::get(default_int_type(), 0);
  }

  // ---------------------------------------------------------------------------
  // Variable declaration
  // ---------------------------------------------------------------------------

  void llvm_ir_generator::codegen_var_stmt(var_node *node)
  {
    std::string name = tok_str(node->name);

    // Determine type
    llvm::Type *ty = nullptr;
    if (node->type)
      ty = type_node_to_llvm(node->type.get());
    else if (node->expression)
      ty = infer_type(node->expression.get());
    if (!ty)
      ty = default_int_type();

    if (m_current_fn)
    {
      // Local variable: alloca in entry block
      llvm::AllocaInst *alloca = create_entry_alloca(name, ty);
      m_value_map[node] = alloca;

      if (node->expression)
      {
        llvm::Value *init = codegen_expr(node->expression.get());
        if (init)
        {
          // Coerce type if needed
          if (init->getType() != ty)
          {
            if (ty->isIntegerTy() && init->getType()->isIntegerTy())
              init = m_builder->CreateIntCast(init, ty, true, "icast");
            else if (ty->isFloatingPointTy() && init->getType()->isIntegerTy())
              init = m_builder->CreateSIToFP(init, ty, "sitofp");
            else if (ty->isIntegerTy() && init->getType()->isFloatingPointTy())
              init = m_builder->CreateFPToSI(init, ty, "fptosi");
          }
          m_builder->CreateStore(init, alloca);
        }
      }
    }
    else
    {
      // Global variable
      llvm::Constant *init_val = nullptr;
      if (node->expression)
      {
        // Can only use constant initialisers for globals
        if (auto *lit = dynamic_cast<literal_node *>(node->expression.get()))
          init_val = llvm::dyn_cast<llvm::Constant>(codegen_literal(lit));
      }
      if (!init_val)
        init_val = llvm::Constant::getNullValue(ty);

      auto *gv = new llvm::GlobalVariable(*m_module, ty, false,
                                          llvm::GlobalValue::InternalLinkage,
                                          init_val, name);
      m_value_map[node] = gv;
    }
  }

  // ---------------------------------------------------------------------------
  // Return statement
  // ---------------------------------------------------------------------------

  void llvm_ir_generator::codegen_return(return_node *node)
  {
    if (!node->value)
    {
      m_builder->CreateRetVoid();
      return;
    }

    llvm::Value *val = codegen_expr(node->value.get());
    if (!val)
    {
      m_builder->CreateRetVoid();
      return;
    }

    // Coerce to function return type if possible
    if (m_current_fn)
    {
      llvm::Type *ret_ty = m_current_fn->getReturnType();
      if (ret_ty->isVoidTy())
      {
        m_builder->CreateRetVoid();
        return;
      }
      if (val->getType() != ret_ty)
      {
        if (ret_ty->isIntegerTy() && val->getType()->isIntegerTy())
          val = m_builder->CreateIntCast(val, ret_ty, true, "ret_icast");
        else if (ret_ty->isFloatingPointTy() && val->getType()->isIntegerTy())
          val = m_builder->CreateSIToFP(val, ret_ty, "ret_sitofp");
        else if (ret_ty->isIntegerTy() && val->getType()->isFloatingPointTy())
          val = m_builder->CreateFPToSI(val, ret_ty, "ret_fptosi");
      }
    }

    m_builder->CreateRet(val);
  }

  // ---------------------------------------------------------------------------
  // If statement
  // ---------------------------------------------------------------------------

  void llvm_ir_generator::codegen_if(if_node *node)
  {
    llvm::Value *cond = codegen_expr(node->condition.get());
    if (!cond)
      return;

    // Convert to i1 if it's an integer
    if (!cond->getType()->isIntegerTy(1))
    {
      llvm::Value *zero = llvm::ConstantInt::get(cond->getType(), 0);
      cond = m_builder->CreateICmpNE(cond, zero, "ifcond");
    }

    llvm::BasicBlock *then_bb = llvm::BasicBlock::Create(*m_ctx, "if.then", m_current_fn);
    llvm::BasicBlock *else_bb = node->else_clause
                                  ? llvm::BasicBlock::Create(*m_ctx, "if.else", m_current_fn)
                                  : nullptr;
    llvm::BasicBlock *merge_bb = llvm::BasicBlock::Create(*m_ctx, "if.end", m_current_fn);

    m_builder->CreateCondBr(cond, then_bb, else_bb ? else_bb : merge_bb);

    // Then block
    m_builder->SetInsertPoint(then_bb);
    if (node->scope)
      codegen_scope(node->scope.get());
    if (!m_builder->GetInsertBlock()->getTerminator())
      m_builder->CreateBr(merge_bb);

    // Else block
    if (else_bb)
    {
      m_builder->SetInsertPoint(else_bb);
      codegen_if(node->else_clause.get()); // recurse (handles elif)
      if (!m_builder->GetInsertBlock()->getTerminator())
        m_builder->CreateBr(merge_bb);
    }

    m_builder->SetInsertPoint(merge_bb);
  }

  // ---------------------------------------------------------------------------
  // While loop
  // ---------------------------------------------------------------------------

  void llvm_ir_generator::codegen_while(while_node *node)
  {
    llvm::BasicBlock *cond_bb  = llvm::BasicBlock::Create(*m_ctx, "while.cond", m_current_fn);
    llvm::BasicBlock *body_bb  = llvm::BasicBlock::Create(*m_ctx, "while.body", m_current_fn);
    llvm::BasicBlock *after_bb = llvm::BasicBlock::Create(*m_ctx, "while.end",  m_current_fn);

    m_builder->CreateBr(cond_bb);

    // Condition
    m_builder->SetInsertPoint(cond_bb);
    llvm::Value *cond = codegen_expr(node->condition.get());
    if (cond && !cond->getType()->isIntegerTy(1))
    {
      llvm::Value *zero = llvm::ConstantInt::get(cond->getType(), 0);
      cond = m_builder->CreateICmpNE(cond, zero, "whilecond");
    }
    if (cond)
      m_builder->CreateCondBr(cond, body_bb, after_bb);
    else
      m_builder->CreateBr(after_bb);

    // Body
    m_builder->SetInsertPoint(body_bb);
    if (node->scope)
      codegen_scope(node->scope.get());
    if (!m_builder->GetInsertBlock()->getTerminator())
      m_builder->CreateBr(cond_bb);

    m_builder->SetInsertPoint(after_bb);
  }

  // ---------------------------------------------------------------------------
  // For (numeric range) loop
  // ---------------------------------------------------------------------------

  void llvm_ir_generator::codegen_for(for_node *node)
  {
    // Brandy for: for x from <start> to <end> [ every <inc> ]
    // Synthesise a counting loop.
    if (!node->loop_start || !node->loop_end)
    {
      // for-in loop (iterator-based) — not supported in this release
      std::cerr << "Warning: for-in loop not supported in LLVM codegen\n";
      return;
    }

    llvm::Type *loop_ty = default_int_type();

    // Allocate loop variable
    llvm::AllocaInst *loop_var = create_entry_alloca(tok_str(node->loop_var_name), loop_ty);

    llvm::Value *start = codegen_expr(node->loop_start.get());
    llvm::Value *end   = codegen_expr(node->loop_end.get());
    llvm::Value *inc   = node->loop_increment
                           ? codegen_expr(node->loop_increment.get())
                           : llvm::ConstantInt::get(loop_ty, 1);

    if (!start || !end || !inc)
      return;

    // Coerce types
    auto coerce = [&](llvm::Value *v) -> llvm::Value * {
      if (v->getType() != loop_ty)
        return m_builder->CreateIntCast(v, loop_ty, true, "loop_cast");
      return v;
    };
    start = coerce(start); end = coerce(end); inc = coerce(inc);

    m_builder->CreateStore(start, loop_var);

    llvm::BasicBlock *cond_bb  = llvm::BasicBlock::Create(*m_ctx, "for.cond",  m_current_fn);
    llvm::BasicBlock *body_bb  = llvm::BasicBlock::Create(*m_ctx, "for.body",  m_current_fn);
    llvm::BasicBlock *latch_bb = llvm::BasicBlock::Create(*m_ctx, "for.latch", m_current_fn);
    llvm::BasicBlock *after_bb = llvm::BasicBlock::Create(*m_ctx, "for.end",   m_current_fn);

    m_builder->CreateBr(cond_bb);

    // Condition: loop_var < end
    m_builder->SetInsertPoint(cond_bb);
    llvm::Value *cur = m_builder->CreateLoad(loop_ty, loop_var, "for.cur");
    llvm::Value *cmp = m_builder->CreateICmpSLT(cur, end, "for.cmp");
    m_builder->CreateCondBr(cmp, body_bb, after_bb);

    // Body
    m_builder->SetInsertPoint(body_bb);
    // Register the loop variable so it can be referenced by name
    if (node->loop_var_name.type() != token_types::INVALID)
    {
      // We temporarily expose the loop variable through the symbol mechanism.
      // Since there is no loop_var symbol_node here, we use the for_node itself
      // as the key.
      m_value_map[node] = loop_var;
    }
    if (node->scope)
      codegen_scope(node->scope.get());
    if (!m_builder->GetInsertBlock()->getTerminator())
      m_builder->CreateBr(latch_bb);

    // Latch: increment
    m_builder->SetInsertPoint(latch_bb);
    llvm::Value *cur2  = m_builder->CreateLoad(loop_ty, loop_var, "for.cur2");
    llvm::Value *next  = m_builder->CreateAdd(cur2, inc, "for.next");
    m_builder->CreateStore(next, loop_var);
    m_builder->CreateBr(cond_bb);

    m_builder->SetInsertPoint(after_bb);
    m_value_map.erase(node);
  }

  // ---------------------------------------------------------------------------
  // Scope (block of statements)
  // ---------------------------------------------------------------------------

  void llvm_ir_generator::codegen_scope(scope_node *node, bool /*push_new_scope*/)
  {
    for (auto &stmt : node->statements)
      codegen_stmt(stmt.get());
  }

  // ---------------------------------------------------------------------------
  // Generic statement dispatcher
  // ---------------------------------------------------------------------------

  void llvm_ir_generator::codegen_stmt(statement_node *node)
  {
    if (!node) return;

    // Don't emit instructions after a terminator
    if (m_current_fn && m_builder->GetInsertBlock() &&
        m_builder->GetInsertBlock()->getTerminator())
      return;

    if (auto *n = dynamic_cast<return_node *>(node))   { codegen_return(n); return; }
    if (auto *n = dynamic_cast<if_node *>(node))       { codegen_if(n);     return; }
    if (auto *n = dynamic_cast<while_node *>(node))    { codegen_while(n);  return; }
    if (auto *n = dynamic_cast<for_node *>(node))      { codegen_for(n);    return; }
    if (auto *n = dynamic_cast<var_node *>(node))      { codegen_var_stmt(n); return; }

    // Label / goto
    if (auto *n = dynamic_cast<label_node *>(node))
    {
      std::string lbl = tok_str(n->name) + ".lbl";
      llvm::BasicBlock *bb = llvm::BasicBlock::Create(*m_ctx, lbl, m_current_fn);
      if (!m_builder->GetInsertBlock()->getTerminator())
        m_builder->CreateBr(bb);
      m_builder->SetInsertPoint(bb);
      return;
    }

    if (auto *n = dynamic_cast<goto_node *>(node))
    {
      // gotos are rare; emit an unconditional branch to a stub block
      std::cerr << "Warning: goto not fully supported\n";
      llvm::BasicBlock *stub = llvm::BasicBlock::Create(*m_ctx, "goto.stub", m_current_fn);
      m_builder->CreateBr(stub);
      m_builder->SetInsertPoint(stub);
      return;
    }

    // Expression used as statement (e.g. a function call, assignment, …)
    if (auto *n = dynamic_cast<expression_node *>(node))
    {
      codegen_expr(n);
      return;
    }

    std::cerr << "Warning: unhandled statement node\n";
  }

  // ---------------------------------------------------------------------------
  // Function declaration (prototype)
  // ---------------------------------------------------------------------------

  namespace
  {
    /// Scan a scope recursively for the first non-void return statement.
    /// Returns nullptr if none found.
    expression_node *find_first_return_expr(scope_node *scope)
    {
      if (!scope) return nullptr;
      for (auto &stmt : scope->statements)
      {
        if (auto *ret = dynamic_cast<return_node *>(stmt.get()))
        {
          if (ret->value) return ret->value.get();
        }
        if (auto *iff = dynamic_cast<if_node *>(stmt.get()))
        {
          if (auto *e = find_first_return_expr(iff->scope.get())) return e;
          if (iff->else_clause)
            if (auto *e = find_first_return_expr(iff->else_clause->scope.get())) return e;
        }
        if (auto *w = dynamic_cast<while_node *>(stmt.get()))
          if (auto *e = find_first_return_expr(w->scope.get())) return e;
      }
      return nullptr;
    }
  }

  void llvm_ir_generator::declare_function(function_node *node)
  {
    std::string name = tok_str(node->name);

    // Determine return type
    llvm::Type *ret_ty;
    if (node->return_type)
      ret_ty = type_node_to_llvm(node->return_type.get());
    else
    {
      // Infer from return statements
      expression_node *ret_expr = find_first_return_expr(node->scope.get());
      ret_ty = ret_expr ? infer_type(ret_expr) : llvm::Type::getVoidTy(*m_ctx);
    }

    // Collect parameter types
    std::vector<llvm::Type *> param_tys;
    for (auto &param : node->parameters)
    {
      llvm::Type *pty = param->type
                          ? type_node_to_llvm(param->type.get())
                          : default_int_type();
      param_tys.push_back(pty);
    }

    llvm::FunctionType *ft = llvm::FunctionType::get(ret_ty, param_tys, false);
    llvm::Function *fn = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, m_module);

    // Name the arguments
    {
      unsigned idx = 0;
      for (auto &arg : fn->args())
      {
        if (idx < node->parameters.size())
          arg.setName(tok_str(node->parameters[idx]->name));
        ++idx;
      }
    }

    m_fn_map[name] = fn;
    // Store the function pointer in the value map keyed by the function node
    m_value_map[node] = fn;
  }

  // ---------------------------------------------------------------------------
  // Function definition (body)
  // ---------------------------------------------------------------------------

  void llvm_ir_generator::define_function(function_node *node)
  {
    if (!node->scope) return; // declaration only

    std::string name = tok_str(node->name);
    auto it = m_fn_map.find(name);
    if (it == m_fn_map.end()) return;

    llvm::Function *fn = it->second;
    m_current_fn = fn;

    // Create entry basic block
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(*m_ctx, "entry", fn);
    m_builder->SetInsertPoint(entry);

    // Allocate parameters on the stack so they can be assigned to
    unsigned idx = 0;
    for (auto &arg : fn->args())
    {
      parameter_node *param = node->parameters[idx].get();
      llvm::Type *pty = arg.getType();
      llvm::AllocaInst *alloca = create_entry_alloca(std::string(arg.getName()), pty);
      m_builder->CreateStore(&arg, alloca);
      m_value_map[param] = alloca;
      ++idx;
    }

    // Emit body
    codegen_scope(node->scope.get());

    // Add a fallthrough return if needed
    if (!m_builder->GetInsertBlock()->getTerminator())
    {
      if (fn->getReturnType()->isVoidTy())
        m_builder->CreateRetVoid();
      else
        m_builder->CreateRet(llvm::Constant::getNullValue(fn->getReturnType()));
    }

    // Clean up parameter entries (they're local to this function)
    for (auto &param : node->parameters)
      m_value_map.erase(param.get());

    m_current_fn = nullptr;
  }

  // ---------------------------------------------------------------------------
  // Module code-gen
  // ---------------------------------------------------------------------------

  void llvm_ir_generator::codegen_module(module_node *node)
  {
    // First pass: declare all functions so recursive calls resolve.
    for (auto &member : node->members)
    {
      if (auto *fn = dynamic_cast<function_node *>(member.get()))
        declare_function(fn);
    }

    // Second pass: emit global variable declarations.
    for (auto &member : node->members)
    {
      if (auto *vn = dynamic_cast<var_node *>(member.get()))
        codegen_var_stmt(vn);
    }

    // Third pass: emit function bodies.
    for (auto &member : node->members)
    {
      if (auto *fn = dynamic_cast<function_node *>(member.get()))
        define_function(fn);
    }

    // Collect module-level statements that are not symbol declarations.
    std::vector<statement_node *> mod_stmts;
    for (auto &stmt : node->statements)
      mod_stmts.push_back(stmt.get());

    // Emit module-level statements into @__brandy_init.
    if (!mod_stmts.empty())
    {
      llvm::FunctionType *init_ty = llvm::FunctionType::get(llvm::Type::getVoidTy(*m_ctx), false);
      llvm::Function *init_fn = llvm::Function::Create(
        init_ty, llvm::Function::ExternalLinkage, "__brandy_init", m_module);
      m_current_fn = init_fn;

      llvm::BasicBlock *entry = llvm::BasicBlock::Create(*m_ctx, "entry", init_fn);
      m_builder->SetInsertPoint(entry);

      for (auto *stmt : mod_stmts)
        codegen_stmt(stmt);

      if (!m_builder->GetInsertBlock()->getTerminator())
        m_builder->CreateRetVoid();

      m_current_fn = nullptr;
    }
  }

  // ---------------------------------------------------------------------------
  // Public entry points
  // ---------------------------------------------------------------------------

  void llvm_ir_generator::generate(module_node *node)
  {
    // Initialise built-in types so type comparisons work.
    brandy::builtin::setup_types();
    codegen_module(node);

    // Verify the module and report any errors.
    std::string err;
    llvm::raw_string_ostream err_stream(err);
    if (llvm::verifyModule(*m_module, &err_stream))
      std::cerr << "LLVM module verification error: " << err << "\n";
  }

  void llvm_ir_generator::print_ir(const char *output_file)
  {
    if (!output_file)
    {
      m_module->print(llvm::outs(), nullptr);
      return;
    }

    std::error_code ec;
    llvm::raw_fd_ostream out(output_file, ec, llvm::sys::fs::OF_Text);
    if (ec)
    {
      std::cerr << "Error opening output file '" << output_file << "': "
                << ec.message() << "\n";
      return;
    }
    m_module->print(out, nullptr);
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
