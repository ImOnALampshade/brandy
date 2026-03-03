// -----------------------------------------------------------------------------
// Brandy compiler flags
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef COMPILER_FLAGS_H
#define COMPILER_FLAGS_H

#pragma once

#include <vector>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  class compiler_flags
  {
  public:
    compiler_flags();

    bool parse_options(int argc, const char **argv);

    bool dump_parser_stack();
    bool dump_ast();
    bool dump_ast_graph();
    bool emit_llvm();
    const char *input_file();
    const char *output_file();

    void push_options();

    static compiler_flags &current();
  private:
    bool m_dumpParserStack;
    bool m_dumpAst;
    bool m_dumpAstGraph;
    bool m_emitLlvm;
    const char *m_inputFile;
    const char *m_outputFile;
  };

#define CURRENT_FLAGS (brandy::compiler_flags::current())

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
