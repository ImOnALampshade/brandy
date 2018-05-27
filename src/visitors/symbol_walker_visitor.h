// -----------------------------------------------------------------------------
// Brandy ast visitor to walk symbols
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef SYMBOL_WALKER_VISITOR_H
#define SYMBOL_WALKER_VISITOR_H

#pragma once

#include "../ast_nodes.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  struct symbol_walker_visitor : ast_visitor
  {
    symbol_walker_visitor();

    virtual ast_visitor::visitor_result visit(abstract_node *node) override;
    virtual ast_visitor::visitor_result visit(module_node *node) override;
    virtual ast_visitor::visitor_result visit(statement_node *node) override;
    virtual ast_visitor::visitor_result visit(symbol_node *node) override;
    virtual ast_visitor::visitor_result visit(expression_node *node) override;
    virtual ast_visitor::visitor_result visit(meta_node *node) override;
    virtual ast_visitor::visitor_result visit(meta_block_node *node) override;
    virtual ast_visitor::visitor_result visit(attribute_node *node) override;
    virtual ast_visitor::visitor_result visit(return_node *node) override;
    virtual ast_visitor::visitor_result visit(break_node *node) override;
    virtual ast_visitor::visitor_result visit(goto_node *node) override;
    virtual ast_visitor::visitor_result visit(if_node *node) override;
    virtual ast_visitor::visitor_result visit(while_node *node) override;
    virtual ast_visitor::visitor_result visit(for_node *node) override;
    virtual ast_visitor::visitor_result visit(import_node *node) override;
    virtual ast_visitor::visitor_result visit(label_node *node) override;
    virtual ast_visitor::visitor_result visit(class_node *node) override;
    virtual ast_visitor::visitor_result visit(function_node *node) override;
    virtual ast_visitor::visitor_result visit(var_node *node) override;
    virtual ast_visitor::visitor_result visit(parameter_node *node) override;
    virtual ast_visitor::visitor_result visit(property_node *node) override;
    virtual ast_visitor::visitor_result visit(binary_operator_node *node) override;
    virtual ast_visitor::visitor_result visit(unary_operator_node *node) override;
    virtual ast_visitor::visitor_result visit(absolute_value_node *node) override;
    virtual ast_visitor::visitor_result visit(literal_node *node) override;
    virtual ast_visitor::visitor_result visit(tuple_literal_node *node) override;
    virtual ast_visitor::visitor_result visit(dict_literal_node *node) override;
    virtual ast_visitor::visitor_result visit(lambda_node *node) override;
    virtual ast_visitor::visitor_result visit(name_reference_node *node) override;
    virtual ast_visitor::visitor_result visit(post_expression_node *node) override;
    virtual ast_visitor::visitor_result visit(member_access_node *node) override;
    virtual ast_visitor::visitor_result visit(call_node *node) override;
    virtual ast_visitor::visitor_result visit(cast_node *node) override;
    virtual ast_visitor::visitor_result visit(index_node *node) override;
    virtual ast_visitor::visitor_result visit(type_node *node) override;
    virtual ast_visitor::visitor_result visit(scope_node *node) override;

    symbol_table_stack m_symbolTableStack;
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
