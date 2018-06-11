// -----------------------------------------------------------------------------
// Brandy error class
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef ERROR_BASE_H
#define ERROR_BASE_H

#pragma once

#include <string>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  struct error
  {
    enum severity
    {
      // A warning - compilation can continue
      sev_warn,
      // An error - the compiler may proceed but no code can be generated
      sev_err,
      // A terminal error - the compiler must abort
      sev_term
    };

    error(size_t i, std::string msg, severity sev) :
      token_index(i),
      message(msg),
      error_severity(sev)
    {
    }

    const size_t token_index;
    const std::string message;
    const severity error_severity;
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
