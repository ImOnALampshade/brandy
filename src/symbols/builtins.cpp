// -----------------------------------------------------------------------------
// Brandy builtin symbols
// Howard Hughes
// -----------------------------------------------------------------------------

#include "builtins.h"
#include <deque>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  namespace
  {
    symbol_table builtin_table;
  }

  // ---------------------------------------------------------------------------

  type_symbol *void_symbol_type::common_type(type_symbol *secondType)
  {
    return nullptr;
  }

  symbol *void_symbol_type::get_member(const token &name)
  {
    return nullptr; // void has no symbols
  }

  // ---------------------------------------------------------------------------

  type_symbol *int8_symbol_type::common_type(type_symbol *secondType)
  {
    if (auto *builtinSecondType = dynamic_cast<builtin_type_symbol *>(secondType))
    {
      if (builtinSecondType->is_int_type())
        return secondType;
      if (builtinSecondType->is_float_type())
        return secondType;
      if (builtinSecondType == &bool_symbol)
        return secondType;
    }

    return nullptr;
  }

  symbol *int8_symbol_type::get_member(const token &name)
  {
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  type_symbol *int16_symbol_type::common_type(type_symbol *secondType)
  {
    if (auto *builtinSecondType = dynamic_cast<builtin_type_symbol *>(secondType))
    {
      if (builtinSecondType->is_int_type())
        return (byte_size() > builtinSecondType->byte_size()) ? this : secondType;
      if (builtinSecondType->is_float_type())
        return secondType;
      if (builtinSecondType == &bool_symbol)
        return secondType;
    }

    return nullptr;
  }

  symbol *int16_symbol_type::get_member(const token &name)
  {
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  type_symbol *int32_symbol_type::common_type(type_symbol *secondType)
  {
    if (auto *builtinSecondType = dynamic_cast<builtin_type_symbol *>(secondType))
    {
      if (builtinSecondType->is_int_type())
        return (byte_size() > builtinSecondType->byte_size()) ? this : secondType;
      if (builtinSecondType->is_float_type())
        return secondType;
      if (builtinSecondType == &bool_symbol)
        return secondType;
    }

    return nullptr;
  }

  symbol *int32_symbol_type::get_member(const token &name)
  {
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  type_symbol *int64_symbol_type::common_type(type_symbol *secondType)
  {
    if (auto *builtinSecondType = dynamic_cast<builtin_type_symbol *>(secondType))
    {
      if (builtinSecondType->is_int_type())
        return (byte_size() > builtinSecondType->byte_size()) ? this : secondType;
      if (builtinSecondType->is_float_type())
        return secondType;
      if (builtinSecondType == &bool_symbol)
        return secondType;
    }

    return nullptr;
  }

  symbol *int64_symbol_type::get_member(const token &name)
  {
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  type_symbol *uint8_symbol_type::common_type(type_symbol *secondType)
  {
    if (auto *builtinSecondType = dynamic_cast<builtin_type_symbol *>(secondType))
    {
      if (builtinSecondType->is_uint_type())
        return secondType;
      if (builtinSecondType->is_float_type())
        return secondType;
      if (builtinSecondType == &bool_symbol)
        return secondType;
    }

    return nullptr;
  }

  symbol *uint8_symbol_type::get_member(const token &name)
  {
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  type_symbol *uint16_symbol_type::common_type(type_symbol *secondType)
  {
    if (auto *builtinSecondType = dynamic_cast<builtin_type_symbol *>(secondType))
    {
      if (builtinSecondType->is_uint_type())
        return (byte_size() > builtinSecondType->byte_size()) ? this : secondType;
      if (builtinSecondType->is_float_type())
        return secondType;
      if (builtinSecondType == &bool_symbol)
        return secondType;
    }

    return nullptr;
  }

  symbol *uint16_symbol_type::get_member(const token &name)
  {
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  type_symbol *uint32_symbol_type::common_type(type_symbol *secondType)
  {
    if (auto *builtinSecondType = dynamic_cast<builtin_type_symbol *>(secondType))
    {
      if (builtinSecondType->is_uint_type())
        return (byte_size() > builtinSecondType->byte_size()) ? this : secondType;
      if (builtinSecondType->is_float_type())
        return secondType;
      if (builtinSecondType == &bool_symbol)
        return secondType;
    }

    return nullptr;
  }

  symbol *uint32_symbol_type::get_member(const token &name)
  {
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  type_symbol *uint64_symbol_type::common_type(type_symbol *secondType)
  {
    if (auto *builtinSecondType = dynamic_cast<builtin_type_symbol *>(secondType))
    {
      if (builtinSecondType->is_uint_type())
        return (byte_size() > builtinSecondType->byte_size()) ? this : secondType;
      if (builtinSecondType->is_float_type())
        return secondType;
      if (builtinSecondType == &bool_symbol)
        return secondType;
    }

    return nullptr;
  }

  symbol *uint64_symbol_type::get_member(const token &name)
  {
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  type_symbol *float32_symbol_type::common_type(type_symbol *secondType)
  {
    if (auto *builtinSecondType = dynamic_cast<builtin_type_symbol *>(secondType))
    {
      if (builtinSecondType->is_int_type())
        return this;
      if (builtinSecondType->is_float_type())
        return secondType;
    }

    return nullptr;
  }

  symbol *float32_symbol_type::get_member(const token &name)
  {
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  type_symbol *float64_symbol_type::common_type(type_symbol *secondType)
  {
    if (auto *builtinSecondType = dynamic_cast<builtin_type_symbol *>(secondType))
    {
      if (builtinSecondType->is_int_type())
        return this;
      if (builtinSecondType->is_float_type())
        return (byte_size() > builtinSecondType->byte_size()) ? this : secondType;
    }

    return nullptr;
  }

  symbol *float64_symbol_type::get_member(const token &name)
  {
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  type_symbol *char_symbol_type::common_type(type_symbol *secondType)
  {
    return nullptr;
  }

  symbol *char_symbol_type::get_member(const token &name)
  {
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  type_symbol *string_symbol_type::common_type(type_symbol *secondType)
  {
    return nullptr;
  }

  symbol *string_symbol_type::get_member(const token &name)
  {
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  type_symbol *regex_symbol_type::common_type(type_symbol *secondType)
  {
    return nullptr;
  }

  symbol *regex_symbol_type::get_member(const token &name)
  {
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  type_symbol *boolean_symbol_type::common_type(type_symbol *secondType)
  {
    if (auto *builtinSecondType = dynamic_cast<builtin_type_symbol *>(secondType))
    {
      if (builtinSecondType->is_int_type())
        return this;
      if (builtinSecondType->is_float_type())
        return (byte_size() > builtinSecondType->byte_size()) ? this : secondType;
    }

    return nullptr;
  }

  symbol *boolean_symbol_type::get_member(const token &name)
  {
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  type_symbol *null_type_symbol_type::common_type(type_symbol *secondType)
  {
    return secondType;
  }

  symbol *null_type_symbol_type::get_member(const token &name)
  {
    return nullptr;
  }

  // ---------------------------------------------------------------------------


  type_symbol *function_type_symbol_type::common_type(type_symbol *secondType)
  {
    return nullptr;
  }

  symbol *function_type_symbol_type::get_member(const token &name)
  {
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  type_symbol *type_type_symbol_type::common_type(type_symbol *secondType)
  {
    return nullptr;
  }

  symbol *type_type_symbol_type::get_member(const token &name)
  {
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  type_symbol *import_type_symbol_type::common_type(type_symbol *secondType)
  {
    return nullptr;
  }

  symbol *import_type_symbol_type::get_member(const token &name)
  {
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  void_symbol_type void_symbol;
  int8_symbol_type int8_symbol;
  int16_symbol_type int16_symbol;
  int32_symbol_type int32_symbol;
  int64_symbol_type int64_symbol;
  uint8_symbol_type uint8_symbol;
  uint16_symbol_type uint16_symbol;
  uint32_symbol_type uint32_symbol;
  uint64_symbol_type uint64_symbol;
  float32_symbol_type float32_symbol;
  float64_symbol_type float64_symbol;
  char_symbol_type char_symbol;
  string_symbol_type string_symbol;
  regex_symbol_type regex_symbol;
  boolean_symbol_type bool_symbol;
  null_type_symbol_type null_type_symbol;
  type_type_symbol_type type_type_symbol;
  import_type_symbol_type import_type_symbol;

  function_symbol print_function_symbol;

  // ---------------------------------------------------------------------------

  std::deque<function_type_symbol_type> functionTypes;

  type_symbol *get_function_type(type_symbol *return_value, type_symbol **params, size_t nParams)
  {
    for (function_type_symbol_type &fnType : functionTypes)
    {
      if (
        fnType.m_returnType != return_value ||
        fnType.m_paramTypes.size() != nParams
        )
        continue;

      for (size_t i = 0; i < nParams; ++i)
      {
        if (params[i] != fnType.m_paramTypes[i])
          goto try_next;
      }
      return &fnType;

    try_next:
      ;
    }

    function_type_symbol_type newType;
    newType.m_returnType = return_value;
    newType.m_paramTypes.insert(newType.m_paramTypes.end(), params, params + nParams);

    functionTypes.emplace_back(std::move(newType));
    return &functionTypes.back();
  }

  // ---------------------------------------------------------------------------

  void create_builtin()
  {
    static concrete_function_symbol printStr;
    printStr.m_returnType = &void_symbol;
    printStr.m_paramTypes.push_back(&string_symbol);

    print_function_symbol.m_concreteFunctions.push_back(&printStr);

    builtin_table.add_symbol(token("print", token_types::IDENTIFIER), &print_function_symbol);
  }

  symbol_table *get_builtins()
  {
    return &builtin_table;
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
