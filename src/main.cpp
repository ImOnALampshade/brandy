// -----------------------------------------------------------------------------
// Brandy language tokens
// Howard Hughes
// -----------------------------------------------------------------------------

#include "flags.h"
#include "lexer.h"
#include "parser.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "functionreturnvisitor.h"
#include "dotfilevisitor.h"
#include "treedumpvisitor.h"
#include "symbolfillervisitor.h"

std::unique_ptr<char[]> load_file(const char *filename)
{
  std::unique_ptr<char[]> source;
  FILE *fp = fopen(filename, "rt");
  if (fp)
  {
    if (fseek(fp, 0L, SEEK_END) == 0)
    {
      long bufsize = ftell(fp);
      if (bufsize == -1) return nullptr;

      source = std::make_unique<char[]>(bufsize + 1);

      if (fseek(fp, 0L, SEEK_SET) != 0) return nullptr;

      size_t newLen = fread(source.get(), sizeof(char), bufsize, fp);
      source[newLen] = '\0';
    }
    fclose(fp);
  }

  return std::move(source);
}

int main(int argc, const char **argv)
{
  brandy::compiler_flags opts;
  if (!opts.parse_options(argc, argv))
  {
    return -1;
  }

  opts.push_options();

  brandy::setup_lexer();

  auto file = load_file(brandy::compiler_flags::current().input_file());

  std::vector<brandy::token> tokens;
  brandy::tokenize_string(file.get(), tokens);

  brandy::parser parser(std::move(tokens));

  try
  {
    auto module = parser.parse_module();

    brandy::function_return_visitor returnVisitor;
    walk_node(module, &returnVisitor);

    if (brandy::compiler_flags::current().dump_ast())
    {
      brandy::tree_dump_visitor visitor;
      walk_node(module, &visitor);
    }

    FILE *f = fopen("ast.gv", "wt");

    if (f)
    {
      fputs("digraph G {\n", f);
      fputs("  graph [dpi=300];\n", f);

      brandy::dotfile_visitor visitor(f);

      walk_node(module, &visitor);

      fputs("}\n", f);

      fclose(f);
    }

    brandy::symbol_table_filler_visitor visitor;
    walk_node(module, &visitor);

    for (auto &pair : module->symbols)
    {
      std::cout << pair.first << std::endl;
    }
  }
  catch (brandy::parsing_error &err)
  {
    auto position = err.position();
    if (position == parser.tokens().end())
      --position;

    std::cout << "Error on line " << position->line_number() << ": " << err.error_str() << std::endl;
  }

  std::cin.get();
  return 0;
}
