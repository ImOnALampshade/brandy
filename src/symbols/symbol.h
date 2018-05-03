// -----------------------------------------------------------------------------
// Brandy symbols
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef SYMBOL_H
#define SYMBOL_H

#pragma once

#include "../ast_nodes_decl.h"
#include "../token.h"
#include "symbol_table.h"
#include <unordered_map>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  struct label_symbol;
  struct variable_symbol;
  struct property_symbol;
  struct function_symbol;
  struct type_symbol;
  struct import_symbol;

  // ---------------------------------------------------------------------------

  struct symbol
  {
    virtual ~symbol() { }

    const token &name() const;
    const token &doc() const;

    virtual bool is_label() const { return false; }
    virtual bool is_variable() const { return false; }
    virtual bool is_property() const { return false; }
    virtual bool is_function() const { return false; }
    virtual bool is_type() const { return false; }
    virtual bool is_import() const { return false; }

    symbol_node *m_astNode;
  };

  // ---------------------------------------------------------------------------

  struct label_symbol : symbol
  {
    bool is_label() const override final { return true; }
  };

  // ---------------------------------------------------------------------------

  struct variable_symbol : symbol
  {
    type_symbol *m_type;

    bool is_variable() const override final { return true; }
  };

  // ---------------------------------------------------------------------------

  struct property_symbol : symbol
  {
    type_symbol *m_type;
    // getter/setter?

    bool is_property() const override final { return true; }
  };

  // ---------------------------------------------------------------------------

  struct function_symbol : symbol
  {
    type_symbol *m_returnType;
    std::vector<type_symbol *> m_paramTypes;
    // inner function?
    // default arguments?

    bool is_function() const override final { return true; }
  };

  // ---------------------------------------------------------------------------

  struct type_symbol : symbol
  {
    symbol_table *m_members;

    bool is_type() const override final { return true; }
  };

  // ---------------------------------------------------------------------------

  struct import_symbol : symbol
  {
    symbol_node *m_symbols;

    bool is_import() const override final { return true; }
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
