// -----------------------------------------------------------------------------
// Brandy symbol table
// Howard Hughes
// -----------------------------------------------------------------------------

#include "symbol_table.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  symbol *symbol_table::resolve_name(const token &name)
  {
    auto found = m_members.find(name);

    if (found != m_members.end())
      return found->second;
    else
      return nullptr;
  }

  bool symbol_table::add_symbol(const token &name, symbol *sym)
  {
    auto result = m_members.emplace(name, sym);
    return result.second;
  }

  // ---------------------------------------------------------------------------

  symbol *symbol_table_stack::resolve_name(const token &name)
  {
    for (auto it = m_tables.rbegin(); it != m_tables.rend(); ++it)
    {
      symbol *found = (*it)->resolve_name(name);
      if (found) return found;
    }
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  void symbol_table_stack::push_table(symbol_table *table)
  {
    m_tables.push_back(table);
  }

  void symbol_table_stack::pop_table()
  {
    m_tables.pop_back();
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
