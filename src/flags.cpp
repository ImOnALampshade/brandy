// -----------------------------------------------------------------------------
// Brandy compiler flags
// Howard Hughes
// -----------------------------------------------------------------------------

#include "flags.h"
#include <stack>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  namespace
  {
    std::stack<compiler_flags> g_options_stack;
  }

  // ---------------------------------------------------------------------------
  
  compiler_flags::compiler_flags() :
    m_dumpParserStack(false),
    m_dumpAst(false),
    m_dumpAstGraph(false),
    m_inputFile(nullptr)
  {
  }
  
  // ---------------------------------------------------------------------------

  bool compiler_flags::parse_options(int argc, const char **argv)
  {
    for (int i = 0; i < argc; ++i)
    {
      if (strcmp(argv[i], "--dump-parser-stack") == 0)
      {
        m_dumpParserStack = true;
      }
      else if (strcmp(argv[i], "--dump-ast") == 0)
      {
        m_dumpAst = true;
      }
      else if (strcmp(argv[i], "--dump-ast-graph") == 0)
      {
        m_dumpAstGraph = true;
      }
      else
      {
        m_inputFile = argv[i];
      }
    }

    return true;
  }

  // ---------------------------------------------------------------------------

  bool compiler_flags::dump_parser_stack()
  {
    return m_dumpParserStack;
  }
 
  bool compiler_flags::dump_ast()
  {
    return m_dumpAst;
  }

  bool compiler_flags::dump_ast_graph()
  {
    return m_dumpAstGraph;
  }

  // ---------------------------------------------------------------------------

  const char *compiler_flags::input_file()
  {
    return m_inputFile;
  }

  // ---------------------------------------------------------------------------

  void compiler_flags::push_options()
  {
    g_options_stack.push(*this);
  }

  // ---------------------------------------------------------------------------

  compiler_flags &compiler_flags::current()
  {
    return g_options_stack.top();
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
