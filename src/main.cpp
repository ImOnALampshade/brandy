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
#include "namereferenceresolvervisitor.h"
#include "binopnodereplacervisitor.h"

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

template<typename visitor_type>
void walk_with(brandy::module_node *module)
{
  visitor_type visitor;
  brandy::walk_node(module, &visitor);
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

  auto file = load_file(CURRENT_FLAGS.input_file());

  if (!file)
  {
    std::cout << "Failed to open " << CURRENT_FLAGS.input_file() << std::endl;
    return -1;
  }

  // Tokenize the file
  std::vector<brandy::token> tokens;
  brandy::tokenize_string(file.get(), tokens);

  brandy::parser parser(std::move(tokens));

  try
  {
    auto module = parser.parse_module();

    walk_with<brandy::function_return_visitor>(module.get());
    walk_with<brandy::symbol_table_filler_visitor>(module.get());
    walk_with<brandy::name_reference_resolver_visitor>(module.get());
    walk_with<brandy::bin_op_replacer_visitor>(module.get());

    if (CURRENT_FLAGS.dump_ast())
      walk_with<brandy::tree_dump_visitor>(module.get());

    if (CURRENT_FLAGS.dump_ast_graph())
      walk_with<brandy::dotfile_visitor>(module.get());
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
