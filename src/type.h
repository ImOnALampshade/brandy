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

  class type
  {
  public:
    enum flags : std::uint32_t
    {
      is_basic       = 1 << 0,
      is_class       = 1 << 1,
      is_struct      = 1 << 2,
      is_interface   = 1 << 3,
      is_abstract    = 1 << 4,
      is_inheritable = 1 << 5
    };

    type();

    void set_base(type *base);
    void add_member(const token &name, symbol_node *node);

    bool is_primitive() const;
    bool is_any_float() const;
    bool is_any_int() const;

    symbol_node *get_member(const token &name);

    static type *common(const type *t1, const type *t2);

    bool check_flag_all(flags flag) const;
    bool check_flag_any(flags flag) const;
    void set_flag(flags flag, bool value);
  private:
    type *m_base;
    std::uint32_t m_flags;
    std::unordered_map<token, symbol_node *> m_members;
  };

  // Built-in types
  namespace builtin
  {
    extern type boolean;
    extern type i8;
    extern type i16;
    extern type i32;
    extern type i64;
    extern type f32;
    extern type f64;
    extern type string;
    extern type object;

    void setup_types();
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
