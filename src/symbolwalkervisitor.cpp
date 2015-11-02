// -----------------------------------------------------------------------------
// AST visitor for walking the symbol table (for inheriting from)
// Howard Hughes
// -----------------------------------------------------------------------------

#include "symbolwalkervisitor.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  symbol_table_visitor::symbol_table_visitor()
  {
    m_symStack.push_back(&g_baseSymbolTable);
  }

  // ---------------------------------------------------------------------------
  
  ast_visitor::visitor_result symbol_table_visitor::visit(module_node *node)
  {
    m_symStack.push_back(&node->symbols);
    walk_node(node, this, false);
    m_symStack.pop_back();
    return ast_visitor::stop;
  }
  
  ast_visitor::visitor_result symbol_table_visitor::visit(class_node *node)
  {
    m_symStack.push_back(&node->symbols);
    walk_node(node, this, false);
    m_symStack.pop_back();
    return ast_visitor::stop;
  }
  
  ast_visitor::visitor_result symbol_table_visitor::visit(scope_node *node)
  {
    m_symStack.push_back(&node->symbols);
    walk_node(node, this, false);
    m_symStack.pop_back();
    return ast_visitor::stop;
  }

  // ---------------------------------------------------------------------------
  
  symbol *symbol_table_visitor::get_symbol(token name)
  {
    for (auto it = m_symStack.rbegin(); it != m_symStack.rend(); ++it)
    {
      auto table = *it;

      auto found = table->find(name);

      if (found != table->end())
        return &found->second;
    }

    return nullptr;
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
