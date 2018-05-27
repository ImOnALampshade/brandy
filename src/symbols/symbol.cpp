// -----------------------------------------------------------------------------
// Brandy symbols
// Howard Hughes
// -----------------------------------------------------------------------------

#include "builtins.h"
#include "symbol.h"
#include "../ast_nodes.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  const token &symbol::name() const
  {
    return m_astNode->name;
  }

  const token &symbol::doc() const
  {
    return m_astNode->doc;
  }

  // ---------------------------------------------------------------------------

  type_symbol *concrete_function_symbol::get_type() const
  {
    return get_function_type(
      m_returnType, 
      const_cast<type_symbol **>(m_paramTypes.data()), 
      m_paramTypes.size()
    );
  }

  concrete_function_symbol *function_symbol::get_concrete(type_symbol **paramTypes, size_t nParams)
  {
    concrete_function_symbol *retVal = nullptr;

    for (concrete_function_symbol *sym : m_concreteFunctions)
    {
      if (sym->m_paramTypes.size() != nParams)
        continue;

      type_symbol **paramTypesArgs = paramTypes;
      for (type_symbol *paramTypeFn : sym->m_paramTypes)
      {
        // If they're the same type, then this looks good
        if (paramTypeFn == *paramTypesArgs)
          ++paramTypesArgs;

        // If there is a common type, also looks good
        else if (paramTypeFn->common_type(*paramTypesArgs))
          ++paramTypesArgs;

        // Otherwise, go forward
        else
          goto wrong_args;
      }

      if (retVal) {}
        // TODO: ambiguity error
      
      retVal = sym;

    wrong_args:
      ;
    }

    return retVal;
  }

  // ---------------------------------------------------------------------------

  type_symbol *type_symbol::get_type() const
  {
    return &type_type_symbol;
  }

  // ---------------------------------------------------------------------------

  type_symbol *import_symbol::get_type() const
  {
    return &import_type_symbol;
  }

  symbol *import_symbol::get_member(const token &name)
  {
    return m_symbols->resolve_name(name);
  }

  // ---------------------------------------------------------------------------

  const token &builtin_type_symbol::name() const
  {
    return m_name;
  }
  
  const token &builtin_type_symbol::doc() const
  {
    return m_doc;
  }

  // ---------------------------------------------------------------------------

  type_symbol *class_type_symbol::common_type(type_symbol *secondType)
  {
    return nullptr;
  }

  symbol *class_type_symbol::get_member(const token &name)
  {
    return m_members->resolve_name(name);
  }
 
  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

