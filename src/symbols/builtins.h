// -----------------------------------------------------------------------------
// Brandy builtin symbols
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef BUILTIN_SYMBOL_H
#define BUILTIN_SYMBOL_H

#pragma once

#include "symbol.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  struct builtin_type_symbol : type_symbol
  {
    const token &name() const override final;
    const token &doc() const override final;

    virtual bool is_int_type() const { return false; }
    virtual bool is_uint_type() const { return false; }
    virtual bool is_float_type() const { return false; }
    virtual unsigned byte_size() const = 0;

    token m_name;
    token m_doc;
  };

  // ---------------------------------------------------------------------------

  extern struct void_symbol_type : builtin_type_symbol
  {
    type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;

    unsigned byte_size() const override { return 0; }
  } void_symbol;

  extern struct int8_symbol_type : builtin_type_symbol
  {
    type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;

    bool is_int_type() const override { return true; }
    unsigned byte_size() const override { return 1; }
  } int8_symbol;

  extern struct int16_symbol_type : builtin_type_symbol
  {
    type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;

    bool is_int_type() const override { return true; }
    unsigned byte_size() const override { return 2; }
  } int16_symbol;

  extern struct int32_symbol_type : builtin_type_symbol
  {
    type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;

    bool is_int_type() const override { return true; }
    unsigned byte_size() const override { return 4; }
  } int32_symbol;

  extern struct int64_symbol_type : builtin_type_symbol
  {
    type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;

    bool is_int_type() const override { return true; }
    unsigned byte_size() const override { return 8; }
  } int64_symbol;

  extern struct uint8_symbol_type : builtin_type_symbol
  {
    type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;

    bool is_uint_type() const override { return true; }
    unsigned byte_size() const override { return 1; }
  } uint8_symbol;

  extern struct uint16_symbol_type : builtin_type_symbol
  {
    type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;

    bool is_uint_type() const override { return true; }
    unsigned byte_size() const override { return 2; }
  } uint16_symbol;

  extern struct uint32_symbol_type : builtin_type_symbol
  {
    type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;

    bool is_uint_type() const override { return true; }
    unsigned byte_size() const override { return 4; }
  } uint32_symbol;

  extern struct uint64_symbol_type : builtin_type_symbol
  {
    type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;

    bool is_uint_type() const override { return true; }
    unsigned byte_size() const override { return 8; }
  } uint64_symbol;

  extern struct float32_symbol_type : builtin_type_symbol
  {
    type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;

    bool is_float_type() const override { return true; }
    unsigned byte_size() const override { return 4; }
  } float32_symbol;

  extern struct float64_symbol_type : builtin_type_symbol
  {
    type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;

    bool is_float_type() const override { return true; }
    unsigned byte_size() const override { return 8; }
  } float64_symbol;

  extern struct char_symbol_type : builtin_type_symbol
  {
    type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;

    unsigned byte_size() const override { return 1; }
  } char_symbol;

  extern struct string_symbol_type : builtin_type_symbol
  {
    type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;

    unsigned byte_size() const override { return unsigned(-1); }
  } string_symbol;

  extern struct regex_symbol_type : builtin_type_symbol
  {
    type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;

    unsigned byte_size() const override { return unsigned(-1); }
  } regex_symbol;

  extern struct boolean_symbol_type : builtin_type_symbol
  {
    type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;

    unsigned byte_size() const override { return unsigned(-1); }
  } bool_symbol;

  extern struct null_type_symbol_type : builtin_type_symbol
  {
    type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;

    unsigned byte_size() const override { return unsigned(-1); }
  } null_type_symbol;

  struct function_type_symbol_type : builtin_type_symbol
  {
    type_symbol *m_returnType;
    std::vector<type_symbol *> m_paramTypes;

    type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;

    unsigned byte_size() const override { return unsigned(-1); }
  };

  extern struct type_type_symbol_type : builtin_type_symbol
  {
    type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;

    unsigned byte_size() const override { return unsigned(-1); }
  } type_type_symbol;

  extern struct import_type_symbol_type : builtin_type_symbol
  {
    type_symbol *common_type(type_symbol *secondType) override final;
    symbol *get_member(const token &name) override final;

    unsigned byte_size() const override { return 0; }
  } import_type_symbol;

  extern variable_symbol true_var_symbol;
  extern variable_symbol false_var_symbol;
  extern variable_symbol null_var_symbol;

  type_symbol *get_function_type(type_symbol *return_value, type_symbol **params, size_t nParams);

  void create_builtin();

  symbol_table *get_builtins();

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
