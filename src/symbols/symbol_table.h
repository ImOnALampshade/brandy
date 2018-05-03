// -----------------------------------------------------------------------------
// Brandy symbol table
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#pragma once

#include "../token.h"
#include <unordered_map>
#include <vector>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  struct symbol;

  // ---------------------------------------------------------------------------

  struct symbol_table
  {
    std::unordered_map<token, symbol *> m_members;

    symbol *resolve_name(const token &name);

    // Returns true if the symbol was inserted, false otherwise (If it already
    // existed)
    bool add_symbol(const token &name, symbol *sym);
  };

  // ---------------------------------------------------------------------------

  struct symbol_table_stack
  {
    std::vector<symbol_table *> m_tables;

    symbol *resolve_name(const token &name);

    void push_table(symbol_table *table);
    void pop_table();
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
