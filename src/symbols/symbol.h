// -----------------------------------------------------------------------------
// Brandy symbols
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef SYMBOL_H
#define SYMBOL_H

#pragma once

#include "../ast_nodes_decl.h"
#include "../token.h"
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
    symbol(symbol_node *astNode);
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

    variable_symbol();

    bool is_variable() const override final { return true; }
  };

  // ---------------------------------------------------------------------------

  struct property_symbol : symbol
  {
    type_symbol *m_type;
    // getter/setter?

    property_symbol();

    bool is_property() const override final { return true; }
  };

  // ---------------------------------------------------------------------------

  struct function_symbol : symbol
  {
    type_symbol *m_type;
    // inner function?
    // Parameter list?
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif