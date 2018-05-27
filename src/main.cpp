#include "module.h"
#include "lexer.h"
#include "parser.h"
#include "error.h"
#include "symbols/builtins.h"
#include "../lib/argparse/argparse.hpp"
#include "visitors/dotfile_visitor.h"
#include "visitors/parent_hookup_visitor.h"
#include "visitors/symbol_resolver_visitor.h"
#include <iostream>

int main(int argc, const char **argv)
{
  using namespace brandy;

#ifdef _WIN32
  // So that the VS debugger console window will stay open
  atexit([]() {
    std::cin.get();
  });
#endif

  setup_lexer();

  ArgumentParser argParser;

  // add some arguments to search for
  argParser.addArgument("-i", "--input", 1, false);
  argParser.addArgument("--dotfile", 1);

  // parse the command-line arguments - throws if invalid format
  argParser.parse(argc, argv);

  auto inputFile = argParser.retrieve<std::string>("input");
  auto outputDotfile = argParser.retrieve<std::string>("dotfile");

  module mainModule;

  try
  {
    struct parser p(&mainModule);
    mainModule.load(inputFile.c_str());
    mainModule.m_module = p.accept_module();
  }
  catch(error &e)
  {
    mainModule.output_msg(e);
    return 1;
  }

  mainModule.root_node()->module = &mainModule;

  create_builtin();

  parent_hookup_visitor parentVisitor;
  walk_node(mainModule.root_node(), &parentVisitor);

  dotfile_visitor visitor(outputDotfile.c_str());
  walk_node(mainModule.root_node(), &visitor);

  symbol_resolver_visitor srv;
  walk_node(mainModule.root_node(), &srv);

  return 0;
}
