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
    name(),
    symbol_type(invalid),
    node(nullptr),
    type(nullptr),
    is_implicit(false)
  {
  }

  symbol::symbol(const token &name, kind symbolType, abstract_node *node) :
    name(name),
    symbol_type(symbolType),
    node(node),
    type(nullptr),
    is_implicit(false)
  {
  }

  // ---------------------------------------------------------------------------

  symbol_table g_baseSymbolTable;
  
  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
