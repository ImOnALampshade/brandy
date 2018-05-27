// -----------------------------------------------------------------------------
// Brandy ast visitor to walk symbols
// Howard Hughes
// -----------------------------------------------------------------------------

#include "symbol_walker_visitor.h"
#include "../symbols/builtins.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  symbol_walker_visitor::symbol_walker_visitor()
  {
    m_symbolTableStack.push_table(get_builtins());
  }

  // ---------------------------------------------------------------------------

#define WALK_PUSH_TABLE(node_type)                                          \
  ast_visitor::visitor_result symbol_walker_visitor::visit(node_type *node) \
  {                                                                         \
    ast_visitor::visit(node);                                               \
                                                                            \
    m_symbolTableStack.push_table(&node->sym_table);                        \
    walk_node(node, this, false);                                           \
    m_symbolTableStack.pop_table();                                         \
                                                                            \
    return ast_visitor::stop;                                               \
  }

#define WALK_NO_TABLE(node_type)                                            \
  ast_visitor::visitor_result symbol_walker_visitor::visit(node_type *node) \
  {                                                                         \
    return ast_visitor::visit(node);                                        \
  }

  WALK_NO_TABLE(abstract_node)
  WALK_PUSH_TABLE(module_node)
  WALK_NO_TABLE(statement_node)
  WALK_NO_TABLE(symbol_node)
  WALK_NO_TABLE(expression_node)
  WALK_NO_TABLE(meta_node)
  WALK_NO_TABLE(meta_block_node)
  WALK_NO_TABLE(attribute_node)
  WALK_NO_TABLE(return_node)
  WALK_NO_TABLE(break_node)
  WALK_NO_TABLE(goto_node)
  WALK_PUSH_TABLE(if_node)
  WALK_PUSH_TABLE(while_node)
  WALK_PUSH_TABLE(for_node)
  WALK_NO_TABLE(import_node)
  WALK_NO_TABLE(label_node)
  WALK_PUSH_TABLE(class_node)
  WALK_PUSH_TABLE(function_node)
  WALK_NO_TABLE(var_node)
  WALK_NO_TABLE(parameter_node)
  WALK_NO_TABLE(property_node)
  WALK_NO_TABLE(binary_operator_node)
  WALK_NO_TABLE(unary_operator_node)
  WALK_NO_TABLE(absolute_value_node)
  WALK_NO_TABLE(literal_node)
  WALK_NO_TABLE(tuple_literal_node)
  WALK_NO_TABLE(dict_literal_node)
  WALK_PUSH_TABLE(lambda_node)
  WALK_NO_TABLE(name_reference_node)
  WALK_NO_TABLE(post_expression_node)
  WALK_NO_TABLE(member_access_node)
  WALK_NO_TABLE(call_node)
  WALK_NO_TABLE(cast_node)
  WALK_NO_TABLE(index_node)
  WALK_NO_TABLE(type_node)
  WALK_PUSH_TABLE(scope_node)

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
