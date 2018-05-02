
set(SOURCES
  main.cpp
  lexer.cpp
  token.cpp
  module.cpp
  parser.cpp
  ast_nodes.cpp
  visitors/dotfile_visitor.cpp
  visitors/parent_hookup_visitor.cpp
  symbols/symbol.cpp
  symbols/symbol_table.cpp
)

set(HEADERS
  lexer.h
  token.h
  token_types.inl
  module.h
  parser.h
  ast_nodes.h
  ast_nodes_decl.h
  visitors/dotfile_visitor.h
  visitors/parent_hookup_visitor.h
  symbols/symbol.h
  symbols/symbol_table.h
)

add_executable(brandy ${SOURCES} ${HEADERS})

source_group("" FILES ${SOURCES} ${HEADERS})
source_group("Lexer" FILES lexer.h lexer.cpp)
source_group("Token" FILES token.h token.cpp token_types.inl)
source_group("Module" FILES module.h module.cpp)
source_group("Parser" FILES parser.h parser.cpp)
source_group("AST" FILES ast_nodes.h ast_nodes.cpp ast_nodes_decl.h)
source_group("Visitors\\Dotfile" FILES visitors/dotfile_visitor.h visitors/dotfile_visitor.cpp)
source_group("Visitors\\Parent Hookup" FILES visitors/parent_hookup_visitor.h visitors/parent_hookup_visitor.cpp)
source_group("Symbols\\Symbol" FILES symbols/symbol.h symbols/symbol.cpp)
source_group("Symbols\\Table" FILES symbols/symbol_table.h symbols/symbol_table.cpp)

target_compile_features(brandy PUBLIC cxx_std_14)

