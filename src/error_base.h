// -----------------------------------------------------------------------------
// Brandy syntax tree nodes
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef ERROR_BASE_H
#define ERROR_BASE_H

#pragma once

#include <string>

namespace brandy
{
  struct error_base
  {
    error_base(size_t i) : token_index(i) {}
    virtual ~error_base() {}

    virtual std::string error_message() = 0;
    virtual const char *severity() = 0;

    const size_t token_index;
  };
}

#endif
