// -----------------------------------------------------------------------------
// Brandy syntax tree nodes
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef ACTION_TREE_NODES_H
#define ACTION_TREE_NODES_H

#pragma once

#include "ast_nodes.h"
#include <cstdint>
#include <string>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  struct value_reference
  {
    enum
    {
      type_name_reference,
      type_intermediate
    };

    // Serial number of the value
    std::uint32_t type;
    token origin_token;
    std::string ir_name;

    void generate_ir_name();
  };

  // ---------------------------------------------------------------------------

  struct abstract_action
  {
  };

  // ---------------------------------------------------------------------------

  struct symbol_def_action : abstract_action
  {
    std::string name;

    virtual symbol *get_symbol() = 0;
  };

  struct function_def_action : symbol_def_action
  {
    concrete_function_symbol *m_symbol;

    symbol *get_symbol() override { return m_symbol; }
  };

  struct class_def_action : symbol_def_action
  {
    class_type_symbol *m_symbol;

    symbol *get_symbol() override { return m_symbol; }
  };

  struct var_def_action : symbol_def_action
  {
    variable_symbol *m_symbol;
    bool is_global;

    symbol *get_symbol() override { return m_symbol; }
  };

  // ---------------------------------------------------------------------------

  struct statement_action : abstract_action
  {
  };

  struct binary_operator_action : statement_action
  {
    value_reference left;
    value_reference right;
    value_reference result;
    token operation;
  };

  struct unary_operator_action : statement_action
  {
    value_reference value;
    value_reference result;
    token operation;
  };

  struct call_action : statement_action
  {
    value_reference call;
    std::vector<value_reference> arguments;
    value_reference result;
  };

  struct index_action : statement_action
  {
    value_reference iterable;
    value_reference index;
    value_reference result;
  };

  struct access_action : statement_action
  {
    value_reference object;
    value_reference result;
    token member_name;
  };

  struct for_action : statement_action
  {
    value_reference iterable;
    unique_vec<statement_action> statements;
  };

  struct if_action : statement_action
  {
    value_reference condition;
    unique_ptr<if_action> elif_action;
    unique_vec<statement_action> statements;
  };

  struct while_action : statement_action
  {
    value_reference condition;
    unique_vec<statement_action> statements;
  }

  // ---------------------------------------------------------------------------

  struct action_tree_root
  {
    unique_vec<symbol_def_action> symbols;
    unique_vec<statement_action> statements;
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
