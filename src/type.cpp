// -----------------------------------------------------------------------------
// Brandy language types
// Howard Hughes
// -----------------------------------------------------------------------------

#include "type.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  namespace
  {
    // -------------------------------------------------------------------------

    type *common_table[7][7] =
    {
      { &builtin::boolean, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
      { nullptr, &builtin::i8, &builtin::i16, &builtin::i32, &builtin::i64, &builtin::f32, &builtin::f64 },
      { nullptr, &builtin::i16, &builtin::i16, &builtin::i32, &builtin::i64, &builtin::f32, &builtin::f64 },
      { nullptr, &builtin::i32, &builtin::i32, &builtin::i32, &builtin::i64, &builtin::f32, &builtin::f64 },
      { nullptr, &builtin::i64, &builtin::i64, &builtin::i64, &builtin::i64, &builtin::f32, &builtin::f64 },
      { nullptr, &builtin::f32, &builtin::f32, &builtin::f32, &builtin::f32, &builtin::f32, &builtin::f64 },
      { nullptr, &builtin::f64, &builtin::f64, &builtin::f64, &builtin::f64, &builtin::f64, &builtin::f64 }
    };

    const std::pair<type *, size_t> common_table_index[] = 
    {
      { &builtin::boolean, 0 },
      { &builtin::i8,      1 },
      { &builtin::i16,     2 },
      { &builtin::i32,     3 },
      { &builtin::i64,     4 },
      { &builtin::f32,     5 },
      { &builtin::f64,     6 }
    };

    // -------------------------------------------------------------------------
  }

  // ---------------------------------------------------------------------------

  type::type()
  {
  }

  void type::set_base(type *base)
  {
    m_base = base;
  }

  void type::add_member(const token &name, symbol_node *node)
  {
    auto pair = std::make_pair(name, node);
    m_members.insert(pair);
  }

  // ---------------------------------------------------------------------------

  bool type::is_primitive() const
  {
    return
      this == &builtin::boolean ||
      this == &builtin::i8 ||
      this == &builtin::i16 ||
      this == &builtin::i32 ||
      this == &builtin::i64 ||
      this == &builtin::f32 ||
      this == &builtin::f64;
  }

  bool type::is_any_float() const
  {
    return
      this == &builtin::f32 ||
      this == &builtin::f64;
  }

  bool type::is_any_int() const
  {
    return
      this == &builtin::i8 ||
      this == &builtin::i16 ||
      this == &builtin::i32 ||
      this == &builtin::i64;
  }

  // ---------------------------------------------------------------------------

  symbol_node *type::get_member(const token &name)
  {
    auto found = m_members.find(name);

    if (found == m_members.end())
      return nullptr;
    else
      return found->second;
  }

  // ---------------------------------------------------------------------------

  type *type::common(const type *t1, const type *t2)
  {
    if (t1->is_primitive())
    {
      // If T1 is builtin but T2 is not, then there's a problem
      if (!t2->is_primitive())
        return nullptr;

      size_t i1, i2;

      for (auto index : common_table_index)
      {
        if (t1 == index.first)
          i1 = index.second;
        if (t2 == index.first)
          i2 = index.second;
      }

      return common_table[i1][i2];
    }
    // If T2 is primitive but T1 is not, then there's a problem
    else if (t2->is_primitive())
      return nullptr;

    // If they are the same, return the first one
    if (t1 == t2)
      return const_cast<type *>(t1);

    // If both are classes, then return object 
    // TOOD: Find common base class between the two
    if (t1->check_flag_all(flags::is_class) && t2->check_flag_all(flags::is_class))
      return &builtin::object;

    // Can't find anything else
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  bool type::check_flag_all(flags flag) const
  {
    return (m_flags & flag) == flag;
  }

  bool type::check_flag_any(flags flag) const
  {
    return (m_flags & flag) != 0;
  }

  void type::set_flag(flags flag, bool value)
  {
    if (value)
      m_flags |= flag;
    else
      m_flags &= ~flag;
  }
  
  // ---------------------------------------------------------------------------

  namespace builtin
  {
    type boolean;
    type i8;
    type i16;
    type i32;
    type i64;
    type f32;
    type f64;
    type string;
    type object;

    void setup_types()
    {
    }
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
