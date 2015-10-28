// -----------------------------------------------------------------------------
// An AST visitor to dump the tree to stdout
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef TREE_DUMP_VISITOR_H
#define TREE_DUMP_VISITOR_H

#pragma once

#include "astnodes.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  class tree_dump_visitor : public ast_visitor
  {
    virtual ast_visitor::visitor_result visit(abstract_node *node);
    virtual ast_visitor::visitor_result visit(module_node *node);
    virtual ast_visitor::visitor_result visit(symbol_node *node);
    virtual ast_visitor::visitor_result visit(expression_node *node);
    virtual ast_visitor::visitor_result visit(post_expression_node *node);
    virtual ast_visitor::visitor_result visit(statement_node *node);
    virtual ast_visitor::visitor_result visit(class_node *node);
    virtual ast_visitor::visitor_result visit(function_node *node);
    virtual ast_visitor::visitor_result visit(var_node *node);
    virtual ast_visitor::visitor_result visit(parameter_node *node);
    virtual ast_visitor::visitor_result visit(property_node *node);
    virtual ast_visitor::visitor_result visit(binary_operator_node *node);
    virtual ast_visitor::visitor_result visit(unary_operator_node *node);
    virtual ast_visitor::visitor_result visit(member_access_node *node);
    virtual ast_visitor::visitor_result visit(call_node *node);
    virtual ast_visitor::visitor_result visit(cast_node *node);
    virtual ast_visitor::visitor_result visit(index_node *node);
    virtual ast_visitor::visitor_result visit(lambda_capture_node *node);
    virtual ast_visitor::visitor_result visit(lambda_node *node);
    virtual ast_visitor::visitor_result visit(literal_node *node);
    virtual ast_visitor::visitor_result visit(name_reference_node *node);
    virtual ast_visitor::visitor_result visit(goto_node *node);
    virtual ast_visitor::visitor_result visit(label_node *node);
    virtual ast_visitor::visitor_result visit(return_node *node);
    virtual ast_visitor::visitor_result visit(break_node *node);
    virtual ast_visitor::visitor_result visit(continue_node *node);
    virtual ast_visitor::visitor_result visit(if_node *node);
    virtual ast_visitor::visitor_result visit(while_node *node);
    virtual ast_visitor::visitor_result visit(for_node *node);
    virtual ast_visitor::visitor_result visit(import_node *node);
    virtual ast_visitor::visitor_result visit(type_node *node);
    virtual ast_visitor::visitor_result visit(qualifier_node *node);
    virtual ast_visitor::visitor_result visit(attribute_node *node);
    virtual ast_visitor::visitor_result visit(scope_node *node);
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
