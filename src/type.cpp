// -----------------------------------------------------------------------------
// Brandy language types
// Howard Hughes
// -----------------------------------------------------------------------------

#include "type.h"
#include "symbol.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  type::type() :
    base(nullptr),
    flag(0),
    size(0)
  {
  }

  void type::add_member(const token &name, symbol_node *node)
  {
    auto pair = std::make_pair(name, node);
    members.insert(pair);
  }

  // ---------------------------------------------------------------------------

  symbol_node *type::get_member(const token &name)
  {
    auto found = members.find(name);

    if (found == members.end())
      return nullptr;
    else
      return found->second;
  }

  // ---------------------------------------------------------------------------

  type *type::common(type *t1, type *t2)
  {
    // If either is null, then the common type is the other
    if (!t1) return t2;
    if (!t2) return t1;

    if (t1 == t2) return t1;

    if (t1->check_flag_all(is_primitive))
    {
      // If T1 is builtin but T2 is not, then there's a problem
      if (!t2->check_flag_all(is_primitive))
        return nullptr;

      else if (t1->check_flag_all(is_int))
      {
        // If T2 is a floating point number, then return it
        if (t2->check_flag_all(is_float))
          return t2;

        // If T2 is also an int
        else if (t2->check_flag_all(is_int))
        {
          // If one is unsigned but the other isn't, then there is no common type
          if (t1->check_flag_all(is_unsigned) != t2->check_flag_all(is_unsigned))
            return nullptr;
          // If one of them is larger, return that one
          else if (t1->size > t2->size)
            return t1;
          else if (t1->size < t2->size)
            return t2;
          // They are the same type
          else
            return t1;
        }
        else
          return nullptr;
      }

      else  if (t1->check_flag_all(is_float))
      {
        // If T2 is also a floating point number, return the larger of the two
        if (t2->check_flag_all(is_float))
          return t1->size > t2->size ? t1 : t2;

        // If T2 is an int, return the float
        else if (t2->check_flag_all(is_int))
          return t1;
      }
      else
      {
        // No common type otherwise
        // (boolean, string, object, and type have no common types with anything)
        return nullptr;
      }
    }
    // If T2 is primitive but T1 is not, then there's a problem
    else if (t2->check_flag_all(is_primitive))
      return nullptr;

    // If both are classes, then return object 
    // TOOD: Find common base class between the two
    if (t1->check_flag_all(is_class) && t2->check_flag_all(is_class))
      return &builtin::object;

    // Can't find anything else
    return nullptr;
  }

  // ---------------------------------------------------------------------------

  bool type::check_flag_all(std::uint32_t flag_) const
  {
    return (flag & flag_) == flag_;
  }

  bool type::check_flag_any(std::uint32_t flag_) const
  {
    return (flag & flag_) != 0;
  }

  void type::set_flag(std::uint32_t flag_, bool value)
  {
    if (value)
      flag |= flag_;
    else
      flag &= ~flag_;
  }

  // ---------------------------------------------------------------------------

  type_reference type_reference::common(type_reference t1, type_reference t2)
  {
    type_reference result(type::common(t1.inner_type, t2.inner_type));
    result.is_const = t1.is_const || t2.is_const;

    return result;
  }

  // ---------------------------------------------------------------------------

  namespace builtin
  {
    type boolean;

    type i8;
    type i16;
    type i32;
    type i64;

    type ui8;
    type ui16;
    type ui32;
    type ui64;

    type f32;
    type f64;

    type string;
    type object;
    type type_type;
    type void_type;

    static void add_builtin_type(const char *name, type *t)
    {
      token nameTok(name, token_types::IDENTIFIER);
      symbol typeSymbol(nameTok, symbol::type_name, nullptr);

      g_baseSymbolTable[nameTok] = typeSymbol;
    }

    void setup_types()
    {
      add_builtin_type("bool", &boolean);

      add_builtin_type("i8", &i8);
      add_builtin_type("i16", &i16);
      add_builtin_type("i32", &i32);
      add_builtin_type("i64", &i64);

      add_builtin_type("byte", &i8);
      add_builtin_type("short", &i16);
      add_builtin_type("int", &i32);
      add_builtin_type("long", &i64);

      add_builtin_type("ui8", &ui8);
      add_builtin_type("ui16", &ui16);
      add_builtin_type("ui32", &ui32);
      add_builtin_type("ui64", &ui64);

      add_builtin_type("ubyte", &ui8);
      add_builtin_type("ushort", &ui16);
      add_builtin_type("uint", &ui32);
      add_builtin_type("ulong", &ui64);

      add_builtin_type("f32", &f32);
      add_builtin_type("f64", &f64);

      add_builtin_type("float", &f32);
      add_builtin_type("double", &f64);

      add_builtin_type("string", &string);
      add_builtin_type("object", &object);
      add_builtin_type("type", &type_type);
      add_builtin_type("void", &void_type);

      boolean.set_flag(type::is_primitive);
      boolean.size = 4;

      i8.set_flag(type::is_primitive | type::is_int);
      i16.set_flag(type::is_primitive | type::is_int);
      i32.set_flag(type::is_primitive | type::is_int);
      i64.set_flag(type::is_primitive | type::is_int);
      i8.size = 1;
      i16.size = 2;
      i32.size = 4;
      i64.size = 8;

      ui8.set_flag(type::is_primitive | type::is_int | type::is_unsigned);
      ui16.set_flag(type::is_primitive | type::is_int | type::is_unsigned);
      ui32.set_flag(type::is_primitive | type::is_int | type::is_unsigned);
      ui64.set_flag(type::is_primitive | type::is_int | type::is_unsigned);
      ui8.size = 1;
      ui16.size = 2;
      ui32.size = 4;
      ui64.size = 8;

      f32.set_flag(type::is_primitive | type::is_float);
      f64.set_flag(type::is_primitive | type::is_float);
      f32.size = 4;
      f64.size = 8;

      object.set_flag(type::is_class | type::is_inheritable);
    }
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
