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

  class symbol
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

    abstract_node &node();
    
    const abstract_node &node() const;
    token                name() const;
    kind          symbol_type() const;
    type            *var_type() const;

    void var_type(type *t);

  private:
    token m_name;
    kind m_symbolType;
    abstract_node *m_node;
    type *m_type;
  };

  // ---------------------------------------------------------------------------

  typedef std::unordered_map<token, symbol> symbol_table;
  typedef std::vector<symbol_table *> symbol_stack;

  extern symbol_table g_baseSymbolTable;

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
