add_executable(brandy
  main.cpp
  lexer.cpp
  lexer.h
  token.cpp
  token.h
  token_types.inl
  module.h
  module.cpp
  parser.h
  parser.cpp
  ast_nodes.h
  ast_nodes.cpp
  visitors/dotfile_visitor.h
  visitors/dotfile_visitor.cpp
)
target_compile_features(brandy PUBLIC cxx_std_14)
