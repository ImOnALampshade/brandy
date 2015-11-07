// -----------------------------------------------------------------------------
// Brandy language types
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef TYPE_H
#define TYPE_H

#pragma once

#include "tokens.h"
#include <unordered_map>
#include <cstdint>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  struct symbol_node;

  struct type
  {
    enum flags : std::uint32_t
    {
      is_class       = 1 << 0,
      is_struct      = 1 << 1,
      is_interface   = 1 << 2,
      is_abstract    = 1 << 3,
      is_inheritable = 1 << 4,
      is_primitive   = 1 << 5,
      is_int         = 1 << 6,
      is_unsigned    = 1 << 7,
      is_float       = 1 << 8
    };

    type();

    void add_member(const token &name, symbol_node *node);

    symbol_node *get_member(const token &name);

    static type *common(type *t1, type *t2);

    bool check_flag_all(std::uint32_t flag) const;
    bool check_flag_any(std::uint32_t flag) const;
    void set_flag(std::uint32_t flag, bool value = true);

    type *base;
    std::uint32_t flag;
    std::unordered_map<token, symbol_node *> members;
    size_t size;
  };

  // ---------------------------------------------------------------------------

  struct type_reference
  {
    type_reference(type *inner = nullptr) :
      inner_type(inner) {}

    static type_reference common(type_reference t1, type_reference t2);

    type *inner_type;
    bool is_const;
  };

  // ---------------------------------------------------------------------------

  // Built-in types
  namespace builtin
  {
    extern type boolean;
    extern type i8;
    extern type i16;
    extern type i32;
    extern type i64;
    extern type ui8;
    extern type ui16;
    extern type ui32;
    extern type ui64;
    extern type f32;
    extern type f64;
    extern type string;
    extern type object;
    extern type type_type;
    extern type void_type;

    void setup_types();
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
