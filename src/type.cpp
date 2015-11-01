// -----------------------------------------------------------------------------
// Brandy language types
// Howard Hughes
// -----------------------------------------------------------------------------

#include "type.h"

// -----------------------------------------------------------------------------

namespace brandy
{
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

  bool type::is_builtin() const
  {
    return
      this == &builtin::boolean ||
      this == &builtin::i8      ||
      this == &builtin::i16     ||
      this == &builtin::i32     ||
      this == &builtin::i64     ||
      this == &builtin::f32     ||
      this == &builtin::f64     ||
      this == &builtin::string  ||
      this == &builtin::object;
  }

  symbol_node *type::get_member(const token &name)
  {
    auto found = m_members.find(name);

    if (found == m_members.end())
      return nullptr;
    else
      return found->second;
  }

  type *type::common(const type &t1, const type &t2)
  {
    return nullptr;
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
