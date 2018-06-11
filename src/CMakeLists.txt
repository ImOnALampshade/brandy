
set(SOURCES
  main.cpp
  lexer.cpp
  token.cpp
  module.cpp
  parser.cpp
  ast_nodes.cpp
  at_nodes.cpp
  visitors/dotfile_visitor.cpp
  visitors/parent_hookup_visitor.cpp
  visitors/symbol_walker_visitor.cpp
  visitors/symbol_resolver_visitor.cpp
  symbols/symbol.cpp
  symbols/symbol_table.cpp
  symbols/builtins.cpp
)

set(HEADERS
  lexer.h
  token.h
  token_types.inl
  module.h
  parser.h
  ast_nodes.h
  ast_nodes_decl.h
  at_nodes.h
  error.h
  visitors/dotfile_visitor.h
  visitors/parent_hookup_visitor.h
  visitors/symbol_walker_visitor.h
  visitors/symbol_resolver_visitor.h
  symbols/symbol.h
  symbols/symbol_table.h
  symbols/builtins.h
)

add_executable(brandy ${SOURCES} ${HEADERS})

source_group("" FILES ${SOURCES} ${HEADERS})
source_group("Lexer" FILES lexer.h lexer.cpp)
source_group("Token" FILES token.h token.cpp token_types.inl)
source_group("Module" FILES module.h module.cpp)
source_group("Parser" FILES parser.h parser.cpp)
source_group("AST" FILES ast_nodes.h ast_nodes.cpp ast_nodes_decl.h)
source_group("Action Tree" FILES at_nodes.h at_nodes.cpp)
source_group("Visitors\\Dotfile" FILES visitors/dotfile_visitor.h visitors/dotfile_visitor.cpp)
source_group("Visitors\\Parent Hookup" FILES visitors/parent_hookup_visitor.h visitors/parent_hookup_visitor.cpp)
source_group("Visitors\\Symbol Walker" FILES visitors/symbol_walker_visitor.h visitors/symbol_walker_visitor.cpp)
source_group("Visitors\\Symbol Resolver" FILES visitors/symbol_resolver_visitor.h visitors/symbol_resolver_visitor.cpp)
source_group("Symbols\\Symbol" FILES symbols/symbol.h symbols/symbol.cpp)
source_group("Symbols\\Table" FILES symbols/symbol_table.h symbols/symbol_table.cpp)
source_group("Symbols\\Builtins" FILES symbols/builtins.h symbols/builtins.cpp)

target_compile_features(brandy PUBLIC cxx_std_14)

