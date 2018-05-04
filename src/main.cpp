#include "module.h"
#include "lexer.h"
#include "parser.h"
#include "error_base.h"
#include "../lib/argparse/argparse.hpp"
#include "visitors/dotfile_visitor.h"
#include "visitors/parent_hookup_visitor.h"
#include <iostream>

int main(int argc, const char **argv)
{
  using namespace brandy;

  setup_lexer();

  ArgumentParser parser;

  // add some arguments to search for
  parser.addArgument("-i", "--input", 1, false);
  parser.addArgument("--dotfile", 1);

  // parse the command-line arguments - throws if invalid format
  parser.parse(argc, argv);

  // if we get here, the configuration is valid
  module mainModule;

  auto inputFile = parser.retrieve<std::string>("input");
  auto outputDotfile = parser.retrieve<std::string>("dotfile");

  mainModule.load(inputFile.c_str());
  struct parser p(&mainModule);

   //size_t i = 0;
   //for (auto &token : mainModule.m_tokens)
   //{
   //  std::cout << i++ << ": " << token_types::names[token.type()] << ": (" << token << ")" << std::endl;
   //}

  try
  {
    auto moduleNode = p.accept_module();

    parent_hookup_visitor parentVisitor;
    walk_node(moduleNode, &parentVisitor);

    dotfile_visitor visitor(outputDotfile.c_str());
    walk_node(moduleNode, &visitor);
  }
  catch(error_base &e)
  {
    mainModule.print_error(e);
    // std::cin.get();
  }
}
