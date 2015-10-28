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
#include "qualifiers.h"
      COUNT
    };
#undef QUALIFIER_VALUE

    const char *names[];
  }

  // ---------------------------------------------------------------------------
}

#endif
