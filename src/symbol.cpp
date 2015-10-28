// -----------------------------------------------------------------------------
// Brandy language symbol definition
// Howard Hughes
// -----------------------------------------------------------------------------

#include "symbol.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  symbol::symbol(const token &name, type symbolType, abstract_node *node) :
    m_name(name),
    m_symbolType(symbolType),
    m_node(node)
  {
  }

  // ---------------------------------------------------------------------------

  abstract_node &symbol::node() { return *m_node; }

  // ---------------------------------------------------------------------------

  const abstract_node &symbol::node() const { return *m_node; }
  token                symbol::name() const { return m_name; }
  symbol::type  symbol::symbol_type() const { return m_symbolType; }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
