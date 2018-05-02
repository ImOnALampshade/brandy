// -----------------------------------------------------------------------------
// Brandy syntax tree nodes
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef AST_NODES_DECL_H
#define AST_NODES_DECL_H

#pragma once

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  struct abstract_node;
  struct module_node;
  struct statement_node;
  struct symbol_node;
  struct expression_node;
  struct meta_node;
  struct meta_block_node;
  struct attribute_node;
  struct return_node;
  struct break_node;
  struct goto_node;
  struct if_node;
  struct while_node;
  struct for_node;
  struct import_node;
  struct label_node;
  struct class_node;
  struct function_node;
  struct var_node;
  struct parameter_node;
  struct property_node;
  struct binary_operator_node;
  struct unary_operator_node;
  struct absolute_value_node;
  struct literal_node;
  struct tuple_literal_node;
  struct dict_literal_node;
  struct lambda_node;
  struct name_reference_node;
  struct post_expression_node;
  struct member_access_node;
  struct call_node;
  struct cast_node;
  struct index_node;
  struct type_node;
  struct scope_node;

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
