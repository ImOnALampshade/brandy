// -----------------------------------------------------------------------------
// Brandy language tokens
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef QUALIFIERS_H
#define QUALIFIERS_H

#pragma once

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  namespace qualifier_types
  {
#define QUALIFIER_VALUE(val) val,
    enum type
    {
#include "qualifiers.inl"
      COUNT
    };
#undef QUALIFIER_VALUE

    extern const char *names[];
  }

  // ---------------------------------------------------------------------------
}

#endif
