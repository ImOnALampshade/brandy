// -----------------------------------------------------------------------------
// Brandy language symbol definition
// Howard Hughes
// -----------------------------------------------------------------------------

#include "symbol.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  symbol::symbol() :
    m_name(),
    m_symbolType(invalid),
    m_node(nullptr),
    m_type(nullptr)
  {
  }

  symbol::symbol(const token &name, kind symbolType, abstract_node *node) :
    m_name(name),
    m_symbolType(symbolType),
    m_node(node),
    m_type(nullptr)
  {
  }

  // ---------------------------------------------------------------------------

  abstract_node &symbol::node() { return *m_node; }

  // ---------------------------------------------------------------------------

  const abstract_node &symbol::node() const { return *m_node; }
  token                symbol::name() const { return m_name; }
  symbol::kind  symbol::symbol_type() const { return m_symbolType; }
  type            *symbol::var_type() const { return m_type; }

  // ---------------------------------------------------------------------------

  void symbol::var_type(type *t) { m_type = t; }

  // ---------------------------------------------------------------------------

  symbol_table g_baseSymbolTable;
  
  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
