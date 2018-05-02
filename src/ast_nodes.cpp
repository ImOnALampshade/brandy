// -----------------------------------------------------------------------------
// Brandy syntax tree nodes
// Howard Hughes
// -----------------------------------------------------------------------------

#include "ast_nodes.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------


  // ---------------------------------------------------------------------------

  ast_visitor::~ast_visitor()
  {
  }

#define VISIT_BASE(node_type, base_node)                            \
  ast_visitor::visitor_result ast_visitor::visit(node_type *node)   \
  {                                                                 \
    return visit(static_cast<base_node *>(node));                   \
  }

  ast_visitor::visitor_result ast_visitor::visit(abstract_node *node)
  {
    return ast_visitor::resume;
  }

  VISIT_BASE(module_node, abstract_node);
  VISIT_BASE(statement_node, abstract_node);
  VISIT_BASE(symbol_node, statement_node);
  VISIT_BASE(expression_node, statement_node);
  VISIT_BASE(meta_node, statement_node);
  VISIT_BASE(meta_block_node, meta_node);
  VISIT_BASE(attribute_node, meta_node);
  VISIT_BASE(return_node, statement_node);
  VISIT_BASE(break_node, statement_node);
  VISIT_BASE(goto_node, statement_node);
  VISIT_BASE(if_node, statement_node);
  VISIT_BASE(while_node, statement_node);
  VISIT_BASE(for_node, statement_node);
  VISIT_BASE(import_node, statement_node);
  VISIT_BASE(label_node, symbol_node);
  VISIT_BASE(class_node, symbol_node);
  VISIT_BASE(function_node, symbol_node);
  VISIT_BASE(var_node, symbol_node);
  VISIT_BASE(parameter_node, var_node);
  VISIT_BASE(property_node, symbol_node);
  VISIT_BASE(binary_operator_node, expression_node);
  VISIT_BASE(unary_operator_node, expression_node);
  VISIT_BASE(absolute_value_node, expression_node);
  VISIT_BASE(literal_node, expression_node);
  VISIT_BASE(tuple_literal_node, expression_node);
  VISIT_BASE(dict_literal_node, expression_node);
  VISIT_BASE(lambda_node, expression_node);
  VISIT_BASE(name_reference_node, expression_node);
  VISIT_BASE(post_expression_node, expression_node);
  VISIT_BASE(member_access_node, post_expression_node);
  VISIT_BASE(call_node, post_expression_node);
  VISIT_BASE(cast_node, post_expression_node);
  VISIT_BASE(index_node, post_expression_node);
  VISIT_BASE(type_node, abstract_node);
  VISIT_BASE(scope_node, abstract_node);

  // ---------------------------------------------------------------------------

#define NODE_METHODS(node_type)                                               \
  ast_visitor::visitor_result node_type::internal_visit(ast_visitor *visitor) \
  {                                                                           \
    return visitor->visit(this);                                              \
  }                                                                           \
  void node_type::internal_walk(ast_visitor *visitor)

#define WALK_BASE(base_node)           \
  do                                   \
  {                                    \
    base_node::internal_walk(visitor); \
  } while(false)

#define WALK_ALL(vec)           \
  do                            \
  {                             \
    for (auto &node : vec)      \
      walk_node(node, visitor); \
  } while(false)

#define WALK(ptr)              \
  do                           \
  {                            \
    if (ptr)                   \
      walk_node(ptr, visitor); \
  } while(false)

  // ---------------------------------------------------------------------------

  NODE_METHODS(abstract_node)
  {
  }

  NODE_METHODS(module_node)
  {
    WALK_BASE(abstract_node);
    WALK_ALL(symbols);
    WALK_ALL(statements);
  }

  NODE_METHODS(statement_node)
  {
    WALK_BASE(abstract_node);
  }

  NODE_METHODS(symbol_node)
  {
    WALK_BASE(statement_node);
    WALK(attributes);
  }

  NODE_METHODS(expression_node)
  {
    WALK_BASE(statement_node);
  }

  NODE_METHODS(meta_node)
  {
    WALK_BASE(statement_node);
  }

  NODE_METHODS(meta_block_node)
  {
    WALK_BASE(meta_node);
    WALK(inner_scope);
  }

  NODE_METHODS(attribute_node)
  {
    WALK_BASE(meta_node);
    WALK_ALL(attributes);
  }

  NODE_METHODS(return_node)
  {
    WALK_BASE(statement_node);
    WALK(return_expression);
  }

  NODE_METHODS(break_node)
  {
    WALK_BASE(statement_node);
  }

  NODE_METHODS(goto_node)
  {
    WALK_BASE(statement_node);
  }

  NODE_METHODS(if_node)
  {
    WALK_BASE(statement_node);
    WALK(condition);
    WALK(elif_clause);
    WALK(inner_scope);
  }

  NODE_METHODS(while_node)
  {
    WALK_BASE(statement_node);
    WALK(condition);
    WALK(inner_scope);
  }

  NODE_METHODS(for_node)
  {
    WALK_BASE(statement_node);
    WALK(expression);
    WALK(inner_scope);
  }

  NODE_METHODS(import_node)
  {
    WALK_BASE(statement_node);
  }

  NODE_METHODS(label_node)
  {
    WALK_BASE(symbol_node);
  }

  NODE_METHODS(class_node)
  {
    WALK_BASE(symbol_node);
    WALK_ALL(base_classes);
    WALK_ALL(members);
  }

  NODE_METHODS(function_node)
  {
    WALK_BASE(symbol_node);
    WALK_ALL(parameters);
    WALK(return_type);
    WALK(inner_scope);
  }

  NODE_METHODS(var_node)
  {
    WALK_BASE(symbol_node);
    WALK(type);
    WALK(initial_value);
  }

  NODE_METHODS(parameter_node)
  {
    WALK_BASE(var_node);
  }

  NODE_METHODS(property_node)
  {
    WALK_BASE(symbol_node);
    WALK(type);
    WALK(getter);
    WALK(setter);
  }

  NODE_METHODS(binary_operator_node)
  {
    WALK_BASE(expression_node);
    WALK(left);
    WALK(right);
  }

  NODE_METHODS(unary_operator_node)
  {
    WALK_BASE(expression_node);
    WALK(expression);
  }

  NODE_METHODS(absolute_value_node)
  {
    WALK_BASE(expression_node);
    WALK(expression);
  }

  NODE_METHODS(literal_node)
  {
    WALK_BASE(expression_node);
  }

  NODE_METHODS(tuple_literal_node)
  {
    WALK_BASE(expression_node);
    WALK_ALL(values);
  }

  NODE_METHODS(dict_literal_node)
  {
    WALK_BASE(expression_node);
    WALK_ALL(keys);
    WALK_ALL(values);
  }

  NODE_METHODS(lambda_node)
  {
    WALK_BASE(expression_node);
    WALK_ALL(parameters);
    WALK(return_type);
    WALK(inner_scope);
  }

  NODE_METHODS(name_reference_node)
  {
    WALK_BASE(expression_node);
  }

  NODE_METHODS(post_expression_node)
  {
    WALK_BASE(expression_node);
    WALK(expression);
  }

  NODE_METHODS(member_access_node)
  {
    WALK_BASE(post_expression_node);
  }

  NODE_METHODS(call_node)
  {
    WALK_BASE(post_expression_node);
    WALK_ALL(arguments);
  }

  NODE_METHODS(cast_node)
  {
    WALK_BASE(post_expression_node);
    WALK(cast_to);
  }

  NODE_METHODS(index_node)
  {
    WALK_BASE(post_expression_node);
    WALK(index);
  }

  NODE_METHODS(type_node)
  {
    WALK_BASE(abstract_node);
  }

  NODE_METHODS(scope_node)
  {
    WALK_BASE(abstract_node);
    WALK_ALL(statements);
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
