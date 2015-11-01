// -----------------------------------------------------------------------------
// Brandy language AST nodes
// Howard Hughes
// -----------------------------------------------------------------------------

#include "astnodes.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  ast_visitor::~ast_visitor() { }

#define VISIT_BASE(node_type, base_node) ast_visitor::visitor_result ast_visitor::visit(node_type *node) { return visit(static_cast<base_node *>(node)); }
  ast_visitor::visitor_result ast_visitor::visit(abstract_node *node) { return ast_visitor::resume; }
  VISIT_BASE(module_node, abstract_node)
  VISIT_BASE(symbol_node, abstract_node)
  VISIT_BASE(expression_node, statement_node)
  VISIT_BASE(post_expression_node, statement_node)
  VISIT_BASE(statement_node, abstract_node)
  VISIT_BASE(class_node, symbol_node)
  VISIT_BASE(function_node, symbol_node)
  VISIT_BASE(var_node, symbol_node)
  VISIT_BASE(parameter_node, var_node)
  VISIT_BASE(property_node, symbol_node)
  VISIT_BASE(binary_operator_node, expression_node)
  VISIT_BASE(unary_operator_node, expression_node)
  VISIT_BASE(member_access_node, post_expression_node)
  VISIT_BASE(call_node, post_expression_node)
  VISIT_BASE(cast_node, post_expression_node)
  VISIT_BASE(index_node, post_expression_node)
  VISIT_BASE(tuple_expansion_node, post_expression_node)
  VISIT_BASE(literal_node, expression_node)
  VISIT_BASE(tuple_literal_node, expression_node)
  VISIT_BASE(table_literal_node, expression_node)
  VISIT_BASE(lambda_capture_node, abstract_node)
  VISIT_BASE(lambda_node, expression_node)
  VISIT_BASE(name_reference_node, expression_node)
  VISIT_BASE(goto_node, statement_node)
  VISIT_BASE(label_node, statement_node)
  VISIT_BASE(return_node, statement_node)
  VISIT_BASE(break_node, statement_node)
  VISIT_BASE(continue_node, statement_node)
  VISIT_BASE(if_node, statement_node)
  VISIT_BASE(while_node, statement_node)
  VISIT_BASE(for_node, statement_node)
  VISIT_BASE(import_node, statement_node)
  VISIT_BASE(meta_node, statement_node)
  VISIT_BASE(typedef_node, symbol_node)
  VISIT_BASE(type_node, abstract_node)
  VISIT_BASE(tuple_node, type_node)
  VISIT_BASE(delegate_node, type_node)
  VISIT_BASE(plain_type_node, type_node)
  VISIT_BASE(decltype_node, type_node)
  VISIT_BASE(post_type_node, abstract_node)
  VISIT_BASE(type_indirect_node, post_type_node)
  VISIT_BASE(type_array_node, post_type_node)
  VISIT_BASE(type_template_node, post_type_node)
  VISIT_BASE(qualifier_node, abstract_node)
  VISIT_BASE(attribute_node, abstract_node)
  VISIT_BASE(scope_node, abstract_node)

  // ---------------------------------------------------------------------------

  abstract_node::~abstract_node() { }

  // ---------------------------------------------------------------------------

#define WALK_DECL(node_type)\
  ast_visitor::visitor_result node_type::internal_visit(ast_visitor *visitor)\
  {\
    return visitor->visit(this);\
  }\
  void node_type::internal_walk(ast_visitor *visitor)
#define WALK_BASE(base_node) base_node::internal_walk(visitor);
#define WALK_ALL(vec) for (auto &node : vec) walk_node(node, visitor)
#define WALK(ptr) if (ptr) walk_node(ptr, visitor)

  WALK_DECL(abstract_node)
  {
    //visitor->visit(this);
  }

  WALK_DECL(module_node)
  {
    WALK_BASE(abstract_node);
    WALK_ALL(members);
    WALK_ALL(statements);
  }

  WALK_DECL(statement_node)
  {
    WALK_BASE(abstract_node);
  }

  WALK_DECL(symbol_node)
  {
    WALK_BASE(statement_node);
    WALK(attributes);
  }

  WALK_DECL(expression_node)
  {
    WALK_BASE(statement_node);
  }

  WALK_DECL(post_expression_node)
  {
    WALK_BASE(expression_node);
    WALK(left);
  }

  WALK_DECL(class_node)
  {
    WALK_BASE(symbol_node);
    WALK_ALL(base_classes);
    WALK_ALL(members);
  }

  WALK_DECL(function_node)
  {
    WALK_BASE(symbol_node);
    WALK_ALL(parameters);
    WALK(return_type);
    WALK(scope);
  }

  WALK_DECL(var_node)
  {
    WALK_BASE(symbol_node);
    WALK(type);
    WALK(expression);
  }

  WALK_DECL(parameter_node)
  {
    WALK_BASE(var_node);
    WALK(default_value);
  }

  WALK_DECL(property_node)
  {
    WALK_BASE(symbol_node);
    WALK(type);
    WALK(getter);
    WALK(setter);
    WALK(setter_value);
  }

  WALK_DECL(binary_operator_node)
  {
    WALK_BASE(expression_node);
    WALK(left);
    WALK(right);
  }

  WALK_DECL(unary_operator_node)
  {
    WALK_BASE(expression_node);
    WALK(expression);
  }

  WALK_DECL(member_access_node)
  {
    WALK_BASE(post_expression_node);
  }

  WALK_DECL(call_node)
  {
    WALK_BASE(post_expression_node);
    WALK_ALL(parameters);
  }

  WALK_DECL(cast_node)
  {
    WALK_BASE(post_expression_node);
    WALK(type);
  }

  WALK_DECL(index_node)
  {
    WALK_BASE(post_expression_node);
    WALK(index);
  }

  WALK_DECL(tuple_expansion_node)
  {
    WALK_BASE(post_expression_node);
  }

  WALK_DECL(literal_node)
  {
    WALK_BASE(expression_node);
  }

  WALK_DECL(tuple_literal_node)
  {
    WALK_BASE(expression_node);
    WALK_ALL(items);
  }

  WALK_DECL(table_literal_node)
  {
    WALK_BASE(expression_node);
    WALK_ALL(keys);
    WALK_ALL(values);
  }

  WALK_DECL(lambda_capture_node)
  {
    WALK_BASE(abstract_node);
    WALK(name);
  }

  WALK_DECL(lambda_node)
  {
    WALK_BASE(expression_node);
    WALK_ALL(captures);
    WALK_ALL(parameters);
    WALK(return_type);
    WALK(scope);
  }

  WALK_DECL(name_reference_node)
  {
    WALK_BASE(expression_node);
  }

  WALK_DECL(goto_node)
  {
    WALK_BASE(statement_node);
  }

  WALK_DECL(label_node)
  {
    WALK_BASE(statement_node);
  }

  WALK_DECL(return_node)
  {
    WALK_BASE(statement_node);
    WALK(value);
  }

  WALK_DECL(break_node)
  {
    WALK_BASE(statement_node);
  }

  WALK_DECL(continue_node)
  {
    WALK_BASE(statement_node);
  }

  WALK_DECL(if_node)
  {
    WALK_BASE(statement_node);
    WALK(condition);
    WALK(scope);
    WALK(else_clause);
  }

  WALK_DECL(while_node)
  {
    WALK_BASE(statement_node);
    WALK(condition);
    WALK(scope);
  }

  WALK_DECL(for_node)
  {
    WALK_BASE(statement_node);
    WALK(loop_iterator);
    WALK(loop_start);
    WALK(loop_end);
    WALK(loop_increment);
    WALK(condition);
    WALK(scope);
  }

  WALK_DECL(import_node)
  {
    WALK_BASE(statement_node);
  }

  WALK_DECL(meta_node)
  {
    WALK_BASE(statement_node);
    WALK_ALL(symbols);
    WALK_ALL(statements);
  }

  WALK_DECL(typedef_node)
  {
    WALK_BASE(statement_node);
    WALK(attributes);
    WALK(type);
  }

  WALK_DECL(type_node)
  {
    WALK_BASE(abstract_node);
  }

  WALK_DECL(tuple_node)
  {
    WALK_BASE(type_node);
    WALK_ALL(inner_types);
  }

  WALK_DECL(delegate_node)
  {
    WALK_BASE(type_node);
    WALK_ALL(parameter_types);
    WALK(return_type);
  }

  WALK_DECL(plain_type_node)
  {
    WALK_BASE(type_node);
    WALK_ALL(qualifiers);
    WALK_ALL(post_type);
  }

  WALK_DECL(decltype_node)
  {
    WALK_BASE(type_node);
    WALK(decltype_expression);
  }

  WALK_DECL(post_type_node)
  {
    WALK_BASE(abstract_node);
  }

  WALK_DECL(type_indirect_node)
  {
    WALK_BASE(post_type_node);
  }

  WALK_DECL(type_array_node)
  {
    WALK_BASE(post_type_node);
    WALK(array_size);
  }

  WALK_DECL(type_template_node)
  {
    WALK_BASE(post_type_node);
    WALK_ALL(template_parameters);
  }

  WALK_DECL(qualifier_node)
  {
    WALK_BASE(abstract_node);
  }

  WALK_DECL(attribute_node)
  {
    WALK_BASE(abstract_node);
    WALK_ALL(attributes);
  }

  WALK_DECL(scope_node)
  {
    WALK_BASE(abstract_node);
    WALK_ALL(statements);
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
