// -----------------------------------------------------------------------------
// AST visitor for walking the symbol table (for inheriting from)
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef SYMBOL_TABLE_WALKER
#define SYMBOL_TABLE_WALKER

#pragma once

#include "astnodes.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  class symbol_table_visitor : public ast_visitor
  {
  public:
    symbol_table_visitor();

    ast_visitor::visitor_result visit(module_node *node) override;
    ast_visitor::visitor_result visit(class_node *node) override;
    ast_visitor::visitor_result visit(scope_node *node) override;

    symbol *get_symbol(token name);
  private:
    symbol_stack m_symStack;
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
