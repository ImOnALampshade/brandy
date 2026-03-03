// -----------------------------------------------------------------------------
// Brandy LLVM IR code generator
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef LLVM_IR_VISITOR_H
#define LLVM_IR_VISITOR_H

#pragma once

#include "astnodes.h"

#include <string>
#include <unordered_map>
#include <vector>

// Forward-declare LLVM types to avoid pulling all of LLVM into every TU.
namespace llvm
{
  class LLVMContext;
  class Module;
  template<typename T, typename Inserter>
  class IRBuilder;
  class ConstantFolder;
  class IRBuilderDefaultInserter;
  class Type;
  class Value;
  class Function;
  class BasicBlock;
  class AllocaInst;
}

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  /// Generates LLVM IR from the Brandy AST.
  ///
  /// Call generate() after all earlier analysis passes have run (symbol filling,
  /// name-reference resolution, etc.) but *before* the binary-operator-replacer
  /// so that arithmetic nodes are still binary_operator_node instances.
  ///
  /// print_ir() emits the finished module to a file (or stdout when path is
  /// nullptr).
  class llvm_ir_generator
  {
  public:
    explicit llvm_ir_generator(const std::string &module_name);
    ~llvm_ir_generator();

    /// Walk the module and emit LLVM IR.
    void generate(module_node *node);

    /// Write the IR to @p output_file, or to stdout if nullptr.
    void print_ir(const char *output_file = nullptr);

  private:
    // ------------------------------------------------------------------
    // Expression code-generation: each returns an llvm::Value *.
    // ------------------------------------------------------------------

    llvm::Value *codegen_expr(expression_node *node);
    llvm::Value *codegen_literal(literal_node *node);
    llvm::Value *codegen_name_ref(name_reference_node *node);
    llvm::Value *codegen_binary_op(binary_operator_node *node);
    llvm::Value *codegen_unary_op(unary_operator_node *node);
    llvm::Value *codegen_call(call_node *node);
    llvm::Value *codegen_cast(cast_node *node);

    /// Returns the alloca / global pointer for an lvalue expression (for
    /// assignments).  Returns nullptr if the node is not an lvalue.
    llvm::Value *codegen_lvalue(expression_node *node);

    // ------------------------------------------------------------------
    // Statement code-generation.
    // ------------------------------------------------------------------

    void codegen_stmt(statement_node *node);
    void codegen_scope(scope_node *node, bool push_new_scope = false);
    void codegen_return(return_node *node);
    void codegen_if(if_node *node);
    void codegen_while(while_node *node);
    void codegen_for(for_node *node);
    void codegen_var_stmt(var_node *node);

    // ------------------------------------------------------------------
    // Top-level declaration code-generation.
    // ------------------------------------------------------------------

    /// First pass: create all function prototypes so mutual recursion works.
    void declare_function(function_node *node);

    /// Second pass: fill in the function body.
    void define_function(function_node *node);

    void codegen_module(module_node *node);

    // ------------------------------------------------------------------
    // Type helpers.
    // ------------------------------------------------------------------

    /// Map a brandy type * to the corresponding llvm::Type *.
    llvm::Type *brandy_type_to_llvm(brandy::type *t);

    /// Map a brandy type_reference to the corresponding llvm::Type *.
    llvm::Type *type_ref_to_llvm(const type_reference &ref);

    /// Infer the LLVM type of an expression by inspecting the AST.
    llvm::Type *infer_type(expression_node *node);

    /// Resolve a type_node (e.g. plain_type_node) to an llvm::Type *.
    llvm::Type *type_node_to_llvm(type_node *node);

    /// Fallback integer type (i64).
    llvm::Type *default_int_type();

    /// Fallback floating-point type (double).
    llvm::Type *default_float_type();

    // ------------------------------------------------------------------
    // Helpers.
    // ------------------------------------------------------------------

    /// Create an alloca in the entry block of the current function so that
    /// mem2reg can promote it to a register.
    llvm::AllocaInst *create_entry_alloca(const std::string &name, llvm::Type *ty);

    /// Whether @p node's type is a floating-point type.
    bool is_float_value(llvm::Value *v);

    /// Whether @p node's type is an unsigned integer type.
    bool is_unsigned_type(brandy::type *t);

    // ------------------------------------------------------------------
    // State.
    // ------------------------------------------------------------------

    llvm::LLVMContext *m_ctx;
    llvm::Module      *m_module;
    llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter> *m_builder;

    /// Currently compiled function (nullptr at module scope).
    llvm::Function    *m_current_fn;

    /// Pending module-level statements that will be emitted into
    /// @__brandy_init.
    std::vector<statement_node *> m_pending_stmts;

    /// Maps a brandy abstract_node * (var / parameter) → its alloca.
    std::unordered_map<abstract_node *, llvm::Value *> m_value_map;

    /// Maps a function name → its llvm::Function *.
    std::unordered_map<std::string, llvm::Function *> m_fn_map;
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
