// -----------------------------------------------------------------------------
// Brandy language tokens
// Howard Hughes
// -----------------------------------------------------------------------------

#include "qualifiers.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  namespace qualifier_types
  {
#define QUALIFIER_VALUE(val) #val,
    const char *names[] =
    {
#include "qualifiers.inl"
      nullptr
    };
#undef TOKEN_VALUE
  }
}

// -----------------------------------------------------------------------------
