// -----------------------------------------------------------------------------
// Brandy language symbol definition
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef SYMBOL_H
#define SYMBOL_H

#pragma once

#include "tokens.h"
#include "type.h"
#include <unordered_map>
#include <vector>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------
  
  struct abstract_node;
  
  // ---------------------------------------------------------------------------

  struct symbol
  {
  public:
    enum kind {
      invalid,
      function,
      variable,
      type_name, // AKA, class
      label,
      property,
      import,
      typedef_name // AKA, typedef
    };

    symbol();
    symbol(const token &name, kind symbolType, abstract_node *node);

    token name;
    kind symbol_type;
    abstract_node *node;
    type_reference type;
    bool is_implicit;
  };

  // ---------------------------------------------------------------------------

  typedef std::unordered_map<token, symbol> symbol_table;
  typedef std::vector<symbol_table *> symbol_stack;

  extern symbol_table g_baseSymbolTable;

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
