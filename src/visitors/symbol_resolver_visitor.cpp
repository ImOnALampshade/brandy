// -----------------------------------------------------------------------------
// Brandy ast visitor to resolve symbol references
// Howard Hughes
// -----------------------------------------------------------------------------

#include "symbol_resolver_visitor.h"
#include "../symbols/builtins.h"
#include "../module.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  namespace
  {
    const token &get_binary_operation_name(token_types::type operation)
    {
      switch (operation)
      {
      case token_types::EQUALITY:
      {
        static const token nameToken("@equality", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::INEQUALITY:
      {
        static const token nameToken("@inequality", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::LESS_THAN_OR_EQUAL:
      {
        static const token nameToken("@less_than_or_equal", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::GREATER_THAN_OR_EQUAL:
      {
        static const token nameToken("@greater_than_or_equal", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::LESS_THAN:
      {
        static const token nameToken("@less_than", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::GREATER_THAN:
      {
        static const token nameToken("@greater_than", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::APPROX_EQUALITY:
      {
        static const token nameToken("@approx_equality", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::APPROX_INEQUALITY:
      {
        static const token nameToken("@approx_inequality", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::APPROX_LESS_THAN:
      {
        static const token nameToken("@approx_less_than", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::APPROX_GREATER_THAN:
      {
        static const token nameToken("@approx_greater_than", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::APPROX_LESS_THAN_OR_EQUAL:
      {
        static const token nameToken("@approx_less_than_or_equal", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::APPROX_GREATER_THAN_OR_EQUAL:
      {
        static const token nameToken("@approx_greater_than_or_equal", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::ADDITION:
      {
        static const token nameToken("@addition", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::SUBTRACTION:
      {
        static const token nameToken("@subtraction", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::MULTIPLICATION:
      {
        static const token nameToken("@multiplication", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::DIVIDE:
      {
        static const token nameToken("@divide", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::MODULO:
      {
        static const token nameToken("@modulo", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::DOUBLE_MODULO:
      {
        static const token nameToken("@double_modulo", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::EXPONENT:
      {
        static const token nameToken("@exponent", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::SINGLE_QUESTION:
      {
        static const token nameToken("@single_question", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::DOUBLE_QUESTION:
      {
        static const token nameToken("@double_question", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::BITWISE_AND:
      {
        static const token nameToken("@bitwise_and", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::BITWISE_OR:
      {
        static const token nameToken("@bitwise_or", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::BITWISE_XOR:
      {
        static const token nameToken("@bitwise_xor", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::LOGICAL_AND:
      {
        static const token nameToken("@logical_and", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::LOGICAL_OR:
      {
        static const token nameToken("@logical_or", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::SINGLE_RIGHT_ARROW:
      {
        static const token nameToken("@single_right_arrow", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::SINGLE_LEFT_ARROW:
      {
        static const token nameToken("@single_left_arrow", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::SINGLE_RIGHT_LONG_ARROW:
      {
        static const token nameToken("@single_right_long_arrow", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::SINGLE_LEFT_LONG_ARROW:
      {
        static const token nameToken("@single_left_long_arrow", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::BIDIRECTIONAL_ARROW:
      {
        static const token nameToken("@bidirectional_arrow", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::SINGLE_RIGHT_FAT_ARROW:
      {
        static const token nameToken("@single_right_fat_arrow", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::SINGLE_RIGHT_LONG_FAT_ARROW:
      {
        static const token nameToken("@single_right_long_fat_arrow", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::SINGLE_LEFT_LONG_FAT_ARROW:
      {
        static const token nameToken("@single_left_long_fat_arrow", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::BIDIRECTIONAL_FAT_ARROW:
      {
        static const token nameToken("@bidirectional_fat_arrow", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::ASSIGNMENT:
      {
        static const token nameToken("@assignment", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::ASSIGNMENT_ADDITION:
      {
        static const token nameToken("@assignment_addition", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::ASSIGNMENT_SUBTRACTION:
      {
        static const token nameToken("@assignment_subtraction", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::ASSIGNMENT_MULTIPLICATION:
      {
        static const token nameToken("@assignment_multiplication", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::ASSIGNMENT_DIVIDE:
      {
        static const token nameToken("@assignment_divide", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::ASSIGNMENT_MODULO:
      {
        static const token nameToken("@assignment_modulo", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::ASSIGNMENT_DOUBLE_MODULO:
      {
        static const token nameToken("@assignment_double_modulo", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::ASSIGNMENT_EXPONENT:
      {
        static const token nameToken("@assignment_exponent", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::ASSIGNMENT_BITWISE_AND:
      {
        static const token nameToken("@assignment_bitwise_and", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::ASSIGNMENT_BITWISE_OR:
      {
        static const token nameToken("@assignment_bitwise_or", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::ASSIGNMENT_BITWISE_XOR:
      {
        static const token nameToken("@assignment_bitwise_xor", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::ASSIGNMENT_LOGICAL_AND:
      {
        static const token nameToken("@assignment_logical_and", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::ASSIGNMENT_LOGICAL_OR:
      {
        static const token nameToken("@assignment_logical_or", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::ASSIGNMENT_BITSHIFT_RIGHT:
      {
        static const token nameToken("@assignment_bitshift_right", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::ASSIGNMENT_BITSHIFT_LEFT:
      {
        static const token nameToken("@assignment_bitshift_left", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::ASSIGNMENT_BITSHIFT_LOGICAL_RIGHT:
      {
        static const token nameToken("@assignment_bitshift_logical_right", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::ASSIGNMENT_BITSHIFT_LOGICAL_LEFT:
      {
        static const token nameToken("@assignment_bitshift_logical_left", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::PIPE_SINGLE_RIGHT:
      {
        static const token nameToken("@pipe_single_right", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::PIPE_SINGLE_LEFT:
      {
        static const token nameToken("@pipe_single_left", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::PIPE_DOUBLE_RIGHT:
      {
        static const token nameToken("@pipe_double_right", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::PIPE_DOUBLE_LEFT:
      {
        static const token nameToken("@pipe_double_left", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::PIPE_TRIPLE_RIGHT:
      {
        static const token nameToken("@pipe_triple_right", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::PIPE_TRIPLE_LEFT:
      {
        static const token nameToken("@pipe_triple_left", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::DOUBLE_DOT:
      {
        static const token nameToken("@double_dot", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::TRIPLE_DOT:
      {
        static const token nameToken("@triple_dot", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::DOUBLE_COLON:
      {
        static const token nameToken("@double_colon", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::TRIPLE_COLON:
      {
        static const token nameToken("@triple_colon", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::LOGICAL_BITSHIFT_RIGHT:
      {
        static const token nameToken("@logical_bitshift_right", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::LOGICAL_BITSHIFT_LEFT:
      {
        static const token nameToken("@logical_bitshift_left", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::BITSHIFT_RIGHT:
      {
        static const token nameToken("@bitshift_right", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::BITSHIFT_LEFT:
      {
        static const token nameToken("@bitshift_left", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::DOT_MINUS:
      {
        static const token nameToken("@dot_minus", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::DOT_EQUAL:
      {
        static const token nameToken("@dot_equal", token_types::IDENTIFIER);
        return nameToken;
      }
      case token_types::DOUBLE_DOT_EQUAL:
      {
        static const token nameToken("@double_dot_equal", token_types::IDENTIFIER);
        return nameToken;
      }
      default:
        assert(false); // Invalid token passed in
        return token::invalid;
      }
    }

    const token &get_unary_operation_name(token_types::type operation, bool isPost)
    {
      switch (operation)
      {
      case token_types::ADDITION:
      {
        static const token tokenName("@addition", token_types::IDENTIFIER);
        return tokenName;
      }
      case token_types::SUBTRACTION:
      {
        static const token tokenName("@subtraction", token_types::IDENTIFIER);
        return tokenName;
      }
      case token_types::MULTIPLICATION:
      {
        static const token tokenName("@multiplication", token_types::IDENTIFIER);
        return tokenName;
      }
      case token_types::EXPONENT:
      {
        static const token tokenName("@exponent", token_types::IDENTIFIER);
        return tokenName;
      }
      case token_types::INCREMENT:
      {
        static const token postTokenName("@post_increment", token_types::IDENTIFIER);
        static const token preTokenName("@pre_increment", token_types::IDENTIFIER);
        return isPost ? postTokenName : preTokenName;
      }
      case token_types::DECREMENT:
      {
        static const token postTokenName("@post_decrement", token_types::IDENTIFIER);
        static const token preTokenName("@pre_decrement", token_types::IDENTIFIER);
        return isPost ? postTokenName : preTokenName;
      }
      case token_types::BITWISE_XOR:
      {
        static const token tokenName("@bitwise_xor", token_types::IDENTIFIER);
        return tokenName;
      }
      case token_types::BITWISE_NOT:
      {
        static const token tokenName("@bitwise_not", token_types::IDENTIFIER);
        return tokenName;
      }
      case token_types::LOGICAL_NOT:
      {
        static const token tokenName("@logical_not", token_types::IDENTIFIER);
        return tokenName;
      }
      case token_types::DOUBLE_DOT:
      {
        static const token tokenName("@double_dot", token_types::IDENTIFIER);
        return tokenName;
      }
      case token_types::TRIPLE_DOT:
      {
        static const token tokenName("@triple_dot", token_types::IDENTIFIER);
        return tokenName;
      }
      default:
        assert(false); // invalid token passed in
        return token::invalid;
      }
    }


    struct return_type_deducer_visitor : ast_visitor
    {
      return_type_deducer_visitor() :
        m_type(nullptr)
      {
      }

      virtual ast_visitor::visitor_result visit(return_node *node) override final
      {
        if (node->return_expression)
        {
          if (!node->return_expression->resulting_type)
          {
            // TODO: type error
          }
          if (!m_type)
            m_type = node->return_expression->resulting_type;
          else
          {
            m_type = m_type->common_type(node->return_expression->resulting_type);
            if (!m_type)
            {
              error err(node->firstToken, "No common types found for disparate return types", error::sev_err);
              printf("TODO: need to print error here: %s %i\n", __FILE__, __LINE__);
              return ast_visitor::stop;
            }
          }
        }

        return ast_visitor::stop;
      }

      type_symbol *m_type;
    };
  }

  // ---------------------------------------------------------------------------

  ast_visitor::visitor_result symbol_resolver_visitor::visit(module_node *node)
  {
    m_current_module_stack.push(node);
    symbol_walker_visitor::visit(node);
    m_current_module_stack.pop();

    return ast_visitor::stop;
  }

  // ---------------------------------------------------------------------------

  ast_visitor::visitor_result symbol_resolver_visitor::visit(function_node *node)
  {
    if (node->function_symbol.m_returnType)
    {
      // If return type is already set, we already walked this.
      return ast_visitor::stop;
    }

    // Walk down the table first
    symbol_walker_visitor::visit(node);

    if (!node->return_type_node)
    {
      return_type_deducer_visitor rtdv;
      walk_node(node, &rtdv, false);
      node->function_symbol.m_returnType = rtdv.m_type;
    }
    else
    {
      node->function_symbol.m_returnType = node->return_type_node->type_symbol;
    }

    for (auto &paramNode : node->parameters)
    {
      type_symbol *pType = paramNode->type->type_symbol;
      node->function_symbol.m_paramTypes.push_back(pType);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result symbol_resolver_visitor::visit(lambda_node *node)
  {
    symbol_walker_visitor::visit(node);

    if (!node->return_type_node)
    {
      return_type_deducer_visitor rtdv;
      walk_node(node, &rtdv, false);
      node->return_type = rtdv.m_type;
    }
    else
    {
      node->return_type = node->return_type_node->type_symbol;
    }

    return ast_visitor::stop;
  }

  // ---------------------------------------------------------------------------

  ast_visitor::visitor_result symbol_resolver_visitor::visit(var_node *node)
  {
    walk_node(node, this, false);

    if (node->type)
    {
      node->resulting_type = node->type->type_symbol;
    }
    else if (node->initial_value)
    {
      node->resulting_type = node->initial_value->resulting_type;
    }
    else
    {
      assert(false);
      // TODO: parser error?
      // This should have already been handled by the parser
    }

    symbol_walker_visitor::visit(node);
    return ast_visitor::stop;
  }

  // ---------------------------------------------------------------------------

  ast_visitor::visitor_result symbol_resolver_visitor::visit(binary_operator_node *node)
  {
    walk_node(node, this, false);

    auto &tokenName = get_binary_operation_name(node->operation.type());

    auto opSym = node->left->resulting_type->get_member(tokenName);
    if (!opSym || !opSym->is_function())
    {
      error err(node->firstToken, "Unsported types for operator", error::sev_err);
      current_module()->module->output_msg(err);
      goto exit;
    }

    auto opFn = static_cast<function_symbol *>(opSym);
    type_symbol *paramTypeArr[2] =
    {
      node->left->resulting_type,
      node->right->resulting_type
    };

    auto concreate = opFn->get_concrete(paramTypeArr, 2);

    node->resulting_type = concreate->m_returnType;
    node->implementation = concreate;

    if (!node->resulting_type)
    {
      walk_node(concreate->m_astNode, this);
      node->resulting_type = concreate->m_returnType;
    }

  exit:
    symbol_walker_visitor::visit(node);
    return ast_visitor::stop;
  }

  // ---------------------------------------------------------------------------

  ast_visitor::visitor_result symbol_resolver_visitor::visit(unary_operator_node *node)
  {
    walk_node(node, this, false);

    auto &tokenName = get_unary_operation_name(node->operation.type(), node->is_post_expression);

    auto opSym = node->expression->resulting_type->get_member(tokenName);
    if (!opSym || !opSym->is_function())
    {
      error err(node->firstToken, "Unsported types for operator", error::sev_err);
      current_module()->module->output_msg(err);
      goto exit;
    }

    auto opFn = static_cast<function_symbol *>(opSym);
    type_symbol *paramTypeArr[1] =
    {
      node->expression->resulting_type
    };

    auto concreate = opFn->get_concrete(paramTypeArr, 1);

    node->resulting_type = concreate->m_returnType;
    node->implementation = concreate;

    if (!node->resulting_type)
    {
      walk_node(concreate->m_astNode, this);
      node->resulting_type = concreate->m_returnType;
    }

  exit:
    symbol_walker_visitor::visit(node);
    return ast_visitor::stop;
  }

  // ---------------------------------------------------------------------------

  ast_visitor::visitor_result symbol_resolver_visitor::visit(literal_node *node)
  {
    switch (node->value.type())
    {
    case token_types::_INT8:
      node->resulting_type = &int8_symbol;
      break;
    case token_types::_INT16:
      node->resulting_type = &int16_symbol;
      break;
    case token_types::_INT32:
      node->resulting_type = &int32_symbol;
      break;
    case token_types::_INT64:
      node->resulting_type = &int64_symbol;
      break;
    case token_types::_UINT8:
      node->resulting_type = &uint8_symbol;
      break;
    case token_types::_UINT16:
      node->resulting_type = &uint16_symbol;
      break;
    case token_types::_UINT32:
      node->resulting_type = &uint32_symbol;
      break;
    case token_types::_UINT64:
      node->resulting_type = &uint64_symbol;
      break;
    case token_types::_FLOAT32:
      node->resulting_type = &float32_symbol;
      break;
    case token_types::_FLOAT64:
      node->resulting_type = &float64_symbol;
      break;
    case token_types::_CHAR:
      node->resulting_type = &char_symbol;
      break;
    case token_types::_STRING:
      node->resulting_type = &string_symbol;
      break;
    case token_types::REGEX:
      node->resulting_type = &regex_symbol;
      break;
    case token_types::_TRUE:
    case token_types::_FALSE:
      node->resulting_type = &bool_symbol;
      break;
    case token_types::NONE:
      node->resulting_type = &null_type_symbol;
      break;
    default:
      assert(false); // Invalid token type for a literal, parser messed up
    }

    symbol_walker_visitor::visit(node);
    return ast_visitor::stop;
  }

  // ---------------------------------------------------------------------------

  ast_visitor::visitor_result symbol_resolver_visitor::visit(name_reference_node *node)
  {
    walk_node(node, this, false);
    node->symbol = m_symbolTableStack.resolve_name(node->name);
    if (!node->symbol)
    {
      error err(node->firstToken, "Could not resolve name", error::sev_err);
      current_module()->module->output_msg(err);
      goto exit;
    }

    if (node->symbol->is_function())
    {
      auto fnSym = static_cast<function_symbol *>(node->symbol);
      if (fnSym->m_concreteFunctions.size() == 1)
        node->symbol = fnSym->m_concreteFunctions[0];
      else
      {
        // TODO: Resolve overload by looking at parent node
      }
    }

    node->resulting_type = node->symbol->get_type();

  exit:
    symbol_walker_visitor::visit(node);
    return ast_visitor::stop;
  }

  // ---------------------------------------------------------------------------

  ast_visitor::visitor_result symbol_resolver_visitor::visit(member_access_node *node)
  {
    walk_node(node, this, false);

    symbol *resolvedName = node->expression->resulting_type->get_member(node->member_name);

    if (!resolvedName)
    {
      error err(node->firstToken, "Could not resolve member name", error::sev_err);
      current_module()->module->output_msg(err);
      goto exit;
    }

    node->resulting_type = resolvedName->get_type();
    node->symbol = resolvedName;

  exit:
    symbol_walker_visitor::visit(node);
    return ast_visitor::stop;
  }

  // ---------------------------------------------------------------------------

  ast_visitor::visitor_result symbol_resolver_visitor::visit(call_node *node)
  {
    walk_node(node, this, false);


    if (auto fnType = dynamic_cast<function_type_symbol_type *>(node->expression->resulting_type))
    {
      node->resulting_type = fnType->m_returnType;

      if (auto nameRef = dynamic_cast<name_reference_node *>(node->expression.get()))
      {
        if (auto concrete = dynamic_cast<concrete_function_symbol *>(nameRef->symbol))
          node->implementation = concrete;
        else if (auto fn = dynamic_cast<function_symbol *>(nameRef->symbol))
        {
          type_symbol **paramTypeArr = reinterpret_cast<type_symbol **>(alloca(node->arguments.size() * sizeof(type_symbol *)));
          for (size_t i = 0; i < node->arguments.size(); ++i)
            paramTypeArr[i] = node->arguments[i]->resulting_type;
          node->implementation = fn->get_concrete(paramTypeArr, node->arguments.size());
        }
      }
      else if (auto memberAccess = dynamic_cast<member_access_node *>(node->expression.get()))
      {
        if (auto concrete = dynamic_cast<concrete_function_symbol *>(memberAccess->symbol))
          node->implementation = concrete;
        else if (auto fn = dynamic_cast<function_symbol *>(memberAccess->symbol))
        {
          type_symbol **paramTypeArr = reinterpret_cast<type_symbol **>(alloca(node->arguments.size() * sizeof(type_symbol *)));
          for (size_t i = 0; i < node->arguments.size(); ++i)
            paramTypeArr[i] = node->arguments[i]->resulting_type;
          node->implementation = fn->get_concrete(paramTypeArr, node->arguments.size());
        }
      }
    }
    else
    {
      static const token callName("@call", token_types::IDENTIFIER);

      auto callSym = node->expression->resulting_type->get_member(callName);
      if (!callSym || !callSym->is_function())
      {
        error err(node->firstToken, "Cannot call this value", error::sev_err);
        current_module()->module->output_msg(err);
        goto exit;
      }

      auto callFn = static_cast<function_symbol *>(callSym);
      type_symbol **paramTypeArr = reinterpret_cast<type_symbol **>(alloca(node->arguments.size() * sizeof(type_symbol *)));
      for (size_t i = 0; i < node->arguments.size(); ++i)
        paramTypeArr[i] = node->arguments[i]->resulting_type;
      auto concrete = callFn->get_concrete(paramTypeArr, node->arguments.size());

      node->resulting_type = concrete->m_returnType;
      node->implementation = concrete;

      if (!node->resulting_type)
      {
        walk_node(concrete->m_astNode, this);
        node->resulting_type = concrete->m_returnType;
      }
    }

  exit:
    symbol_walker_visitor::visit(node);
    return ast_visitor::stop;
  }

  // ---------------------------------------------------------------------------

  ast_visitor::visitor_result symbol_resolver_visitor::visit(cast_node *node)
  {
    walk_node(node, this, false);

    node->resulting_type = node->cast_to->type_symbol;

    symbol_walker_visitor::visit(node);
    return ast_visitor::stop;
  }

  // ---------------------------------------------------------------------------

  ast_visitor::visitor_result symbol_resolver_visitor::visit(index_node *node)
  {
    walk_node(node, this, false);

    static const token idxName("@index", token_types::IDENTIFIER);

    auto idxSym = node->expression->resulting_type->get_member(idxName);
    if (!idxSym || !idxSym->is_function())
    {
      error err(node->firstToken, "Cannot index this value", error::sev_err);
      current_module()->module->output_msg(err);
      goto exit;
    }

    auto idxFn = static_cast<function_symbol *>(idxSym);
    type_symbol *paramTypeArr[1] =
    {
      node->index->resulting_type
    };
    auto concrete = idxFn->get_concrete(paramTypeArr, 1);

    node->resulting_type = concrete->m_returnType;
    node->implementation = concrete;

    if (!node->resulting_type)
    {
      walk_node(concrete->m_astNode, this);
      node->resulting_type = concrete->m_returnType;
    }

    exit:
    symbol_walker_visitor::visit(node);
    return ast_visitor::stop;
  }

  // ---------------------------------------------------------------------------

  ast_visitor::visitor_result symbol_resolver_visitor::visit(type_node *node)
  {
    symbol *resolvedName = m_symbolTableStack.resolve_name(node->name.front());

    if (!resolvedName)
    {
      error err(node->firstToken, "Cannot resolve the name of this type", error::sev_err);
      current_module()->module->output_msg(err);
      goto exit;
    }

    for (size_t i = 1; i < node->name.size(); ++i)
    {
      resolvedName = resolvedName->get_member(node->name[i]);
      if (!resolvedName)
      {
        error err(node->firstToken, "Could not resolve member name", error::sev_err);
        current_module()->module->output_msg(err);
        goto exit;
      }
    }

    if (!resolvedName->is_type())
    {
      error err(node->firstToken, "Not a type", error::sev_err);
      current_module()->module->output_msg(err);
      goto exit;
    }

    node->type_symbol = dynamic_cast<type_symbol *>(resolvedName);

    exit:
    return symbol_walker_visitor::visit(node);
  }

  // ---------------------------------------------------------------------------

  module_node *symbol_resolver_visitor::current_module()
  {
    return m_current_module_stack.top();
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
