// -----------------------------------------------------------------------------
// Brandy language symbol definition
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef SYMBOL_H
#define SYMBOL_H

#pragma once

#include "tokens.h"
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
    enum type {
      function,
      variable,
      class_name,
      label,
      property,
      import
    };

    symbol(const token &name, type symbolType, abstract_node *node);

    abstract_node &node();
    
    const abstract_node &node() const;
    token                name() const;
    type          symbol_type() const;

  private:
    token m_name;
    type m_symbolType;
    abstract_node *m_node;
  };

  // ---------------------------------------------------------------------------

  typedef std::unordered_map<token, symbol> symbol_table;
  typedef std::vector<symbol_table *> symbol_stack;

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
