// -----------------------------------------------------------------------------
// Brandy language DFA (lexer)
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef LEXER_H
#define LEXER_H

#pragma once

#include "token.h"
#include <map>
#include <memory>
#include <utility>
#include <vector>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  class lexer
  {
  public:
    typedef size_t state_reference;
    lexer();

    bool read_token(const char *stream, token *outputTok) const;

    state_reference root() const;
    state_reference create_state(token_types::type accept = token_types::INVALID);
    void add_edge(state_reference from, state_reference to, char c);
    void add_letter_edge(state_reference from, state_reference to);
    void add_number_edge(state_reference from, state_reference to);
    void add_default_edge(state_reference from, state_reference to);
    state_reference get_edge(state_reference state, char c) const;

    token_types::type accept_value(state_reference state) const;
    void accept_value(state_reference state, token_types::type t);

    void dump_dotfile(FILE *f);

  // private:
    struct state;
    inline const state *get_next_from(const state *state, char c) const;

    struct state
    {
    public:
      state(token_types::type accept);

      token_types::type      accept;
      std::map<char, size_t> edges;
      size_t                 number_edge;
      size_t                 letter_edge;
      size_t                 default_edge;
      size_t                 index;
    };

    std::vector<state> m_states;
  };

  extern lexer gBrandyLexer;

  void setup_lexer();

  bool tokenize_string(const char *str, std::vector<token> &tokens);

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
