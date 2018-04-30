
set(SOURCES
  main.cpp
  lexer.cpp
  token.cpp
  module.cpp
  parser.cpp
  ast_nodes.cpp
  visitors/dotfile_visitor.cpp
)

set(HEADERS
  lexer.h
  token.h
  token_types.inl
  module.h
  parser.h
  ast_nodes.h
  visitors/dotfile_visitor.h
)

add_executable(brandy ${SOURCES} ${HEADERS})

source_group("" FILES ${SOURCES} ${HEADERS})
source_group("Lexer" FILES lexer.h lexer.cpp)
source_group("Token" FILES token.h token.cpp token_types.inl)
source_group("Module" FILES module.h module.cpp)
source_group("Parser" FILES parser.h parser.cpp)
source_group("AST" FILES ast_nodes.h ast_nodes.cpp)
source_group("Visitors\\Dotfile" FILES visitors/dotfile_visitor.h visitors/dotfile_visitor.cpp)

target_compile_features(brandy PUBLIC cxx_std_14)

