// -----------------------------------------------------------------------------
// Brandy compiler flags
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef COMPILER_FLAGS_H
#define COMPILER_FLAGS_H

#pragma once

#include <stack>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  class compiler_flags
  {
  public:
    bool parse_options(int argc, const char **argv);

    bool dump_parser_stack();
    bool dump_ast();
    const char *input_file();

    void push_options();

    static compiler_flags &current();
  private:
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
