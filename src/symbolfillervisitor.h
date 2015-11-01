// -----------------------------------------------------------------------------
// Brandy language symbol table filler
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef SYMBOL_TABLE_FILLER_VISTOR_H
#define SYMBOL_TABLE_FILLER_VISTOR_H

#pragma once

#include "astnodes.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  class symbol_table_filler_visitor : public ast_visitor
  {
  public:
    ast_visitor::visitor_result visit(module_node *node) override;
    ast_visitor::visitor_result visit(class_node *node) override;
    ast_visitor::visitor_result visit(scope_node *node) override;

    ast_visitor::visitor_result visit(lambda_node *node) override;

    ast_visitor::visitor_result visit(var_node *node) override;
    ast_visitor::visitor_result visit(function_node *node) override;
    ast_visitor::visitor_result visit(label_node *node) override;
    ast_visitor::visitor_result visit(property_node *node) override;
    ast_visitor::visitor_result visit(import_node *node) override;
    ast_visitor::visitor_result visit(typedef_node *node) override;

    ast_visitor::visitor_result visit(binary_operator_node *node) override;
  private:
    void insert_node(abstract_node *node, const token &name, symbol::kind type);

    symbol_stack m_symStack;
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
