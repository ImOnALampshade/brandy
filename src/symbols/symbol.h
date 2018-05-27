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
  struct builtin_type_symbol;

  // ---------------------------------------------------------------------------

  struct symbol
  {
    virtual ~symbol() { }

    virtual const token &name() const;
    virtual const token &doc() const;

    virtual bool is_label() const { return false; }
    virtual bool is_variable() const { return false; }
    virtual bool is_property() const { return false; }
    virtual bool is_function() const { return false; }
    virtual bool is_type() const { return false; }
    virtual bool is_import() const { return false; }

    virtual type_symbol *get_type() const { return nullptr; }
    virtual symbol *get_member(const token &name) { return nullptr; }

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
    type_symbol *m_type = nullptr;

    virtual type_symbol *get_type() const { return m_type; }
    bool is_variable() const override final { return true; }
  };

  // ---------------------------------------------------------------------------

  struct property_symbol : symbol
  {
    type_symbol *m_type = nullptr;
    // getter/setter?

    virtual type_symbol *get_type() const { return m_type; }
    bool is_property() const override final { return true; }
  };

  // ---------------------------------------------------------------------------

  struct concrete_function_symbol : symbol
  {
    type_symbol *m_returnType = nullptr;
    std::vector<type_symbol *> m_paramTypes;

    virtual type_symbol *get_type() const;
    bool is_function() const override final { return true; }
  };

  struct function_symbol : symbol
  {
    std::vector<concrete_function_symbol *> m_concreteFunctions;
    
    virtual type_symbol *get_type() const { return nullptr; }
    bool is_function() const override final { return true; }

    concrete_function_symbol *get_concrete(type_symbol **paramTypes, size_t nParams);
  };

  // ---------------------------------------------------------------------------

  struct type_symbol : symbol
  {
    bool is_type() const override { return true; }

    virtual type_symbol *get_type() const;

    virtual type_symbol *common_type(type_symbol *secondType) = 0;
  };

  // ---------------------------------------------------------------------------

  struct import_symbol : symbol
  {
    symbol_table *m_symbols = nullptr;

    virtual type_symbol *get_type() const;
    bool is_import() const override final { return true; }

    symbol *get_member(const token &name) override final;
  };

  // ---------------------------------------------------------------------------

  struct class_type_symbol : type_symbol
  {
    type_symbol *base_class = nullptr;
    symbol_table *m_members = nullptr;

    virtual type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
