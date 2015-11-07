// -----------------------------------------------------------------------------
// Brandy language DFA (lexer)
// Howard Hughes
// -----------------------------------------------------------------------------

#include "lexer.h"
#include <assert.h>
#include <ctype.h>
#include <vector>

// -----------------------------------------------------------------------------

namespace brandy
{
  const size_t INVALID_EDGE = size_t(-1);

  // There are 210 states last I checked
  const size_t APPROX_STATE_COUNT = 256;

  // ---------------------------------------------------------------------------

  lexer::lexer()
  {
    // Make our vector large enough for all states to avoid reallocation
    m_states.reserve(APPROX_STATE_COUNT);
    m_states.emplace_back(token_types::INVALID);
  }

  // ---------------------------------------------------------------------------

  bool lexer::read_token(const char *stream, token *outputTok) const
  {
    const state *current = &m_states.front(), *previous = nullptr;

    auto currentToken = std::make_pair(token_types::INVALID, size_t(0));
    auto lastGoodToken = currentToken;

    size_t &i = currentToken.second;
    do
    {
      currentToken.first = current->accept;

      if (current->accept != token_types::INVALID)
        lastGoodToken = currentToken;

      previous = current;
      current = get_next_from(current, stream[i]);
    } while (current && stream[i++]);

    if (currentToken.first == token_types::INVALID && lastGoodToken.second != 0)
      currentToken = lastGoodToken;

    *outputTok = token(stream, currentToken.second, currentToken.first);

    return outputTok->type() != token_types::INVALID;
  }

  // ---------------------------------------------------------------------------

  lexer::state_reference lexer::root() const
  {
    return 0;
  }

  lexer::state_reference lexer::create_state(token_types::type accept)
  {
    m_states.emplace_back(accept);
    return m_states.back().index = m_states.size() - 1;;
  }

  void lexer::add_edge(state_reference from, state_reference to, char c)
  {
    m_states[from].edges[unsigned char(c)] = to;
  }

  void lexer::add_letter_edge(state_reference from, state_reference to)
  {
    m_states[from].letter_edge = to;
  }

  void lexer::add_number_edge(state_reference from, state_reference to)
  {
    m_states[from].number_edge = to;
  }

  void lexer::add_default_edge(state_reference from, state_reference to)
  {
    m_states[from].default = to;
  }

  lexer::state_reference lexer::get_edge(state_reference state, char c) const
  {
    auto found = m_states[state].edges.find(c);
    if (found != m_states[state].edges.end())
      return found->second;
    else
      return INVALID_EDGE;
  }

  // ---------------------------------------------------------------------------

  void lexer::dump_dotfile(FILE *f)
  {
    fputs("digraph G {\n", f);

    auto add_edge = [this, f](const state &from, const char *name, size_t i)
    {
      if (i == INVALID_EDGE) return;
      const state &to = m_states[i];

      fprintf(f, "  %s%i -> %s%i [label=\"%s\"];\n",
        token_types::names[from.accept], from.index,
        token_types::names[to.accept], to.index,
        name);
    };

    for (const state &s1 : m_states)
    {
      fprintf(f, "  %s%i [label=\"", token_types::names[s1.accept], s1.index);

      const char *nodeName = token_types::names[s1.accept];
      while (*nodeName)
        fputc(tolower(*(nodeName++)), f);

      fputs("\"];\n", f);

      for (const std::pair<char, size_t> &edge : s1.edges)
      {
        char str[] = { edge.first, '\0' };
        char *c = str;
        switch (edge.first)
        {
        case '\n':
          c = "\\n";
          break;
        case ' ':
          c = "| |";
          break;
        case '\"':
          c = "Quote";
          break;
        case '\\':
          c = "\\\\";
          break;
        }

        add_edge(s1, c, edge.second);
      }

      add_edge(s1, "default", s1.default);
      add_edge(s1, "letter", s1.letter_edge);
      add_edge(s1, "number", s1.number_edge);
    }

    fputc('}', f);
  }

  // ---------------------------------------------------------------------------

  inline const lexer::state *lexer::get_next_from(const state *state, char c) const
  {
    auto found = state->edges.find(c);
    if (found != state->edges.end())
      return &m_states[found->second];
    else if (state->letter_edge != INVALID_EDGE && isalpha(c))
      return &m_states[state->letter_edge];
    else if (state->number_edge != INVALID_EDGE && isdigit(c))
      return &m_states[state->number_edge];
    else if (state->default != INVALID_EDGE)
      return &m_states[state->default];
    else
      return nullptr;
  }

  // ---------------------------------------------------------------------------

  lexer::state::state(token_types::type accept) :
    accept(accept),
    letter_edge(INVALID_EDGE),
    number_edge(INVALID_EDGE),
    default(INVALID_EDGE)
  {
  }

  // ---------------------------------------------------------------------------

  lexer gBrandyLexer;

  // ---------------------------------------------------------------------------

  static void add_operator_edges(lexer::state_reference from, char op, token_types::type regular, token_types::type side_effect)
  {
    auto opState = gBrandyLexer.create_state(regular);
    gBrandyLexer.add_edge(from, opState, op);
    gBrandyLexer.add_edge(opState, gBrandyLexer.create_state(side_effect), '=');
  }

  static void add_keyword(lexer::state_reference from, lexer::state_reference identifier, lexer::state_reference finish, const char *str)
  {
    while (str[1] != 0)
    {
      // get the edge from here to the next letter in the string
      lexer::state_reference intermediate = gBrandyLexer.get_edge(from, *str);

      // Check for no edge or one that goes to THE identifier state (not just any identifier)
      if (intermediate == INVALID_EDGE || intermediate == identifier)
      {
        // Create a new edge and hook it up
        intermediate = gBrandyLexer.create_state(token_types::IDENTIFIER);

        gBrandyLexer.add_letter_edge(intermediate, identifier);
        gBrandyLexer.add_number_edge(intermediate, identifier);
        gBrandyLexer.add_edge(intermediate, identifier, '_');
        gBrandyLexer.add_edge(intermediate, identifier, '$');

        gBrandyLexer.add_edge(from, intermediate, *str);
      }

      from = intermediate;
      ++str;
    }

    gBrandyLexer.add_letter_edge(finish, identifier);
    gBrandyLexer.add_number_edge(finish, identifier);
    gBrandyLexer.add_edge(finish, identifier, '_');
    gBrandyLexer.add_edge(finish, identifier, '$');

    // Add an edge to the finishing state
    gBrandyLexer.add_edge(from, finish, *str);
  }

  void setup_lexer()
  {
    auto root = gBrandyLexer.root();
    auto identifier = gBrandyLexer.create_state(token_types::IDENTIFIER);

    {
      auto whitespace = gBrandyLexer.create_state(token_types::WHITESPACE);
      gBrandyLexer.add_edge(root, whitespace, ' ');
      gBrandyLexer.add_edge(root, whitespace, '\t');

      gBrandyLexer.add_edge(whitespace, whitespace, ' ');
      gBrandyLexer.add_edge(whitespace, whitespace, '\t');
    }

    {
      auto newline = gBrandyLexer.create_state(token_types::NEWLINE);
      gBrandyLexer.add_edge(root, newline, '\n');
    }

    {
      auto at = gBrandyLexer.create_state();
      auto attribute = gBrandyLexer.create_state(token_types::ATTRIBUTE_START);

      gBrandyLexer.add_edge(root, at, '@');
      gBrandyLexer.add_edge(at, attribute, '[');
      gBrandyLexer.add_default_edge(at, identifier);

      gBrandyLexer.add_edge(root, identifier, '_');
      gBrandyLexer.add_edge(root, identifier, '$');
      gBrandyLexer.add_letter_edge(root, identifier);

      gBrandyLexer.add_edge(identifier, identifier, '_');
      gBrandyLexer.add_edge(identifier, identifier, '$');
      gBrandyLexer.add_letter_edge(identifier, identifier);
      gBrandyLexer.add_number_edge(identifier, identifier);
    }

    {
      auto lit_i8 = gBrandyLexer.create_state(token_types::I8_LITERAL);
      auto lit_i16 = gBrandyLexer.create_state(token_types::I16_LITERAL);
      auto lit_i32 = gBrandyLexer.create_state(token_types::I32_LITERAL);
      auto lit_i64 = gBrandyLexer.create_state(token_types::I64_LITERAL);
      
      auto lit_ui8 = gBrandyLexer.create_state(token_types::UI8_LITERAL);
      auto lit_ui16 = gBrandyLexer.create_state(token_types::UI16_LITERAL);
      auto lit_ui32 = gBrandyLexer.create_state(token_types::UI32_LITERAL);
      auto lit_ui64 = gBrandyLexer.create_state(token_types::UI64_LITERAL);
      
      auto dot = gBrandyLexer.create_state();
      auto lit_f32 = gBrandyLexer.create_state(token_types::F32_LITERAL);
      auto lit_f64 = gBrandyLexer.create_state(token_types::F64_LITERAL);
      auto exp = gBrandyLexer.create_state();
      auto expPlus = gBrandyLexer.create_state();
      auto expMinus = gBrandyLexer.create_state();
      auto expVal = gBrandyLexer.create_state(token_types::F64_LITERAL);

      gBrandyLexer.add_number_edge(root, lit_i32);
      gBrandyLexer.add_number_edge(lit_i32, lit_i32);
      gBrandyLexer.add_edge(lit_i32, lit_i8, 'b');
      gBrandyLexer.add_edge(lit_i32, lit_i8, 'B');
      gBrandyLexer.add_edge(lit_i32, lit_i16, 's');
      gBrandyLexer.add_edge(lit_i32, lit_i16, 'S');
      gBrandyLexer.add_edge(lit_i32, lit_i64, 'l');
      gBrandyLexer.add_edge(lit_i32, lit_i64, 'L');
      gBrandyLexer.add_edge(lit_i32, lit_f32, 'f');
      gBrandyLexer.add_edge(lit_i32, lit_f32, 'F');

      gBrandyLexer.add_edge(lit_i32, lit_ui32, 'u');
      gBrandyLexer.add_edge(lit_i32, lit_ui32, 'U');

      gBrandyLexer.add_edge(lit_ui32, lit_ui8, 'b');
      gBrandyLexer.add_edge(lit_ui32, lit_ui8, 'B');
      gBrandyLexer.add_edge(lit_ui32, lit_ui16, 's');
      gBrandyLexer.add_edge(lit_ui32, lit_ui16, 'S');
      gBrandyLexer.add_edge(lit_ui32, lit_ui64, 'l');
      gBrandyLexer.add_edge(lit_ui32, lit_ui64, 'L');

      gBrandyLexer.add_edge(lit_i32, dot, '.');

      gBrandyLexer.add_number_edge(dot, lit_f64);
      gBrandyLexer.add_number_edge(lit_f64, lit_f64);
      gBrandyLexer.add_edge(lit_f64, lit_f32, 'f');
      gBrandyLexer.add_edge(lit_f64, lit_f32, 'F');

      gBrandyLexer.add_edge(lit_f64, exp, 'e');
      gBrandyLexer.add_edge(lit_f64, exp, 'E');

      gBrandyLexer.add_edge(exp, expPlus, '+');
      gBrandyLexer.add_edge(exp, expMinus, '-');
      gBrandyLexer.add_number_edge(exp, expVal);
      gBrandyLexer.add_number_edge(expPlus, expVal);
      gBrandyLexer.add_number_edge(expMinus, expVal);
      gBrandyLexer.add_number_edge(expVal, expVal);
      gBrandyLexer.add_edge(expVal, lit_f32, 'f');
      gBrandyLexer.add_edge(expVal, lit_f32, 'F');
    }

    {
      auto str = gBrandyLexer.create_state();
      auto escape = gBrandyLexer.create_state();
      auto escChar = gBrandyLexer.create_state();
      auto endStr = gBrandyLexer.create_state(token_types::STRING_LITERAL);

      gBrandyLexer.add_edge(root, str, '"');

      gBrandyLexer.add_default_edge(str, str);
      gBrandyLexer.add_edge(str, escape, '\\');

      gBrandyLexer.add_edge(escape, escChar, 'n');
      gBrandyLexer.add_edge(escape, escChar, 'r');
      gBrandyLexer.add_edge(escape, escChar, 't');
      gBrandyLexer.add_edge(escape, escChar, 'b');
      gBrandyLexer.add_edge(escape, escChar, '"');
      gBrandyLexer.add_edge(escape, escChar, '\\');

      gBrandyLexer.add_default_edge(escChar, str);

      gBrandyLexer.add_edge(escChar, endStr, '"');
      gBrandyLexer.add_edge(str, endStr, '"');
    }

    {
      auto chr = gBrandyLexer.create_state();
      auto chrMid = gBrandyLexer.create_state();
      auto escape = gBrandyLexer.create_state();
      auto escChar = gBrandyLexer.create_state();
      auto endChr = gBrandyLexer.create_state(token_types::CHAR_LITERAL);

      gBrandyLexer.add_edge(root, chr, '\'');

      gBrandyLexer.add_default_edge(chr, chrMid);
      gBrandyLexer.add_edge(chr, escape, '\\');

      gBrandyLexer.add_edge(chr, gBrandyLexer.create_state(), '\n');


      gBrandyLexer.add_edge(escape, escChar, 'n');
      gBrandyLexer.add_edge(escape, escChar, 'r');
      gBrandyLexer.add_edge(escape, escChar, 't');
      gBrandyLexer.add_edge(escape, escChar, 'b');
      gBrandyLexer.add_edge(escape, escChar, '\'');
      gBrandyLexer.add_edge(escape, escChar, '\\');

      gBrandyLexer.add_edge(escChar, endChr, '\'');
      gBrandyLexer.add_edge(chrMid, endChr, '\'');
    }

    {
      auto div = gBrandyLexer.create_state(token_types::DIVIDE);
      auto assign = gBrandyLexer.create_state(token_types::ASSIGNMENT_DIVIDE);
      auto blockInner = gBrandyLexer.create_state();
      auto blockEnd1 = gBrandyLexer.create_state();
      auto blockEnd2 = gBrandyLexer.create_state(token_types::BLOCK_COMMENT);
      auto lineComment = gBrandyLexer.create_state();
      auto lineCommentEnd = gBrandyLexer.create_state(token_types::LINE_COMMENT);

      gBrandyLexer.add_edge(root, div, '/');
      gBrandyLexer.add_edge(div, assign, '=');

      gBrandyLexer.add_edge(div, blockInner, '*');
      gBrandyLexer.add_default_edge(blockInner, blockInner);
      gBrandyLexer.add_default_edge(blockEnd1, blockInner);
      gBrandyLexer.add_edge(blockInner, blockEnd1, '*');
      gBrandyLexer.add_edge(blockEnd1, blockEnd2, '/');

      gBrandyLexer.add_edge(div, lineComment, '/');
      gBrandyLexer.add_default_edge(lineComment, lineComment);
      gBrandyLexer.add_edge(lineComment, lineCommentEnd, '\n');
      gBrandyLexer.add_edge(lineComment, lineCommentEnd, '\r');
      gBrandyLexer.add_edge(lineComment, lineCommentEnd, '\0');
    }

    add_operator_edges(root, '+', token_types::ADD, token_types::ASSIGNMENT_ADD);
    add_operator_edges(root, '-', token_types::SUBTRACT, token_types::ASSIGNMENT_SUBTRACT);
    add_operator_edges(root, '*', token_types::ASTRISK, token_types::ASSIGNMENT_MULTIPLY);
    add_operator_edges(root, '%', token_types::MODULO, token_types::ASSIGNMENT_MODULO);
    add_operator_edges(root, '^', token_types::BITWISE_XOR, token_types::ASSIGNMENT_BITWISE_XOR);

    {
      auto greater = gBrandyLexer.create_state(token_types::GREATER_THAN);
      auto greaterEqual = gBrandyLexer.create_state(token_types::GREATER_THAN_OR_EQUAL);
      auto rShift = gBrandyLexer.create_state(token_types::BITWISE_RIGHT_SHIFT);
      auto rShiftAssign = gBrandyLexer.create_state(token_types::ASSIGNMENT_BITWISE_RIGHT_SHIFT);

      gBrandyLexer.add_edge(root, greater, '>');
      gBrandyLexer.add_edge(greater, greaterEqual, '=');
      gBrandyLexer.add_edge(greater, rShift, '>');
      gBrandyLexer.add_edge(rShift, rShiftAssign, '=');
    }

    {
      auto less = gBrandyLexer.create_state(token_types::LESS_THAN);
      auto lessEqual = gBrandyLexer.create_state(token_types::LESS_THAN_OR_EQUAL);
      auto lShift = gBrandyLexer.create_state(token_types::BITWISE_LEFT_SHIFT);
      auto lShiftAssign = gBrandyLexer.create_state(token_types::ASSIGNMENT_BITWISE_LEFT_SHIFT);

      gBrandyLexer.add_edge(root, less, '<');
      gBrandyLexer.add_edge(less, lessEqual, '=');
      gBrandyLexer.add_edge(less, lShift, '<');
      gBrandyLexer.add_edge(lShift, lShiftAssign, '=');
    }

    {
      auto assign = gBrandyLexer.create_state(token_types::ASSIGNMENT);
      auto equality = gBrandyLexer.create_state(token_types::EQUALITY);

      gBrandyLexer.add_edge(root, assign, '=');
      gBrandyLexer.add_edge(assign, equality, '=');
    }

    {
      auto bitwiseAnd = gBrandyLexer.create_state(token_types::AMPERSAND);
      auto logicalAnd = gBrandyLexer.create_state(token_types::LOGICAL_AND);

      auto bitwiseAndAssign = gBrandyLexer.create_state(token_types::ASSIGNMENT_BITWISE_AND);
      auto logicalAndAssign = gBrandyLexer.create_state(token_types::ASSIGNMENT_LOGICAL_AND);

      gBrandyLexer.add_edge(root, bitwiseAnd, '&');
      gBrandyLexer.add_edge(bitwiseAnd, logicalAnd, '&');
      gBrandyLexer.add_edge(bitwiseAnd, bitwiseAndAssign, '=');
      gBrandyLexer.add_edge(logicalAnd, logicalAndAssign, '=');
    }

    {
      auto bitwiseOr = gBrandyLexer.create_state(token_types::BITWISE_OR);
      auto logicalOr = gBrandyLexer.create_state(token_types::LOGICAL_OR);

      auto bitwiseOrAssign = gBrandyLexer.create_state(token_types::ASSIGNMENT_BITWISE_OR);
      auto logicalOrAssign = gBrandyLexer.create_state(token_types::ASSIGNMENT_LOGICAL_OR);

      gBrandyLexer.add_edge(root, bitwiseOr, '|');
      gBrandyLexer.add_edge(bitwiseOr, logicalOr, '|');
      gBrandyLexer.add_edge(bitwiseOr, bitwiseOrAssign, '=');
      gBrandyLexer.add_edge(logicalOr, logicalOrAssign, '=');
    }

    {
      auto not = gBrandyLexer.create_state(token_types::LOGICAL_NOT);
      auto inequality = gBrandyLexer.create_state(token_types::INEQUALITY);

      gBrandyLexer.add_edge(root, not, '!');
      gBrandyLexer.add_edge(not, inequality, '=');
    }

    {
      auto dot = gBrandyLexer.create_state(token_types::DOT);
      auto twoDot = gBrandyLexer.create_state();
      auto tupleExpand = gBrandyLexer.create_state(token_types::TUPLE_EXPANSION);

      gBrandyLexer.add_edge(root, dot, '.');
      gBrandyLexer.add_edge(dot, twoDot, '.');
      gBrandyLexer.add_edge(twoDot, tupleExpand, '.');
    }

    {
      auto docStart = gBrandyLexer.create_state();
      auto docStr = gBrandyLexer.create_state();
      auto docEnd = gBrandyLexer.create_state(token_types::DOCUMENTION_BLOCK);

      gBrandyLexer.add_edge(root, docStart, '`');
      gBrandyLexer.add_edge(docStart, docEnd, '`');
      gBrandyLexer.add_default_edge(docStart, docStr);
      gBrandyLexer.add_default_edge(docStr, docStr);
      gBrandyLexer.add_edge(docStr, docEnd, '`');
    }

    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::META), "meta");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::IMPORT), "import");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::FUNCTION), "func");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::FUNCTION), "function");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::LAMBDA), "lambda");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::CLASS), "class");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::IF), "if");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::ELIF), "elif");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::ELSE), "else");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::UNLESS), "unless");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::FOR), "for");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::IN), "in");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::FROM), "from");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::TO), "to");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::EVERY), "every");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::WHILE), "while");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::DO), "do");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::UNTIL), "until");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::EXPORT), "export");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::ENUM), "enum");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::PROPERTY), "property");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::GET), "get");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::SET), "set");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::VAR), "var");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::RETURN), "return");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::YIELD), "yield");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::BREAK), "break");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::CONTINUE), "continue");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::AS), "as");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::TRUE), "true");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::GOTO), "goto");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::LABEL), "label");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::LOGICAL_AND), "and");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::LOGICAL_OR), "or");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::LOGICAL_NOT), "not");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::FALSE), "false");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::NIL), "nil");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::VALUE), "val");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::VALUE), "value");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::REFERENCE), "ref");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::REFERENCE), "reference");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::TYPEDEF), "typedef");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::DECLTYPE), "decltype");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::TYPENAME), "typename");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::CONST), "const");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::STATIC), "static");
    add_keyword(root, identifier, gBrandyLexer.create_state(token_types::VIRTUAL), "virtual");

    gBrandyLexer.add_edge(root, gBrandyLexer.create_state(token_types::BITWISE_NOT), '~');
    gBrandyLexer.add_edge(root, gBrandyLexer.create_state(token_types::COLON), ':');
    gBrandyLexer.add_edge(root, gBrandyLexer.create_state(token_types::SEMICOLON), ';');
    gBrandyLexer.add_edge(root, gBrandyLexer.create_state(token_types::COMMA), ',');
    gBrandyLexer.add_edge(root, gBrandyLexer.create_state(token_types::OPEN_CURLY), '{');
    gBrandyLexer.add_edge(root, gBrandyLexer.create_state(token_types::CLOSE_CURLY), '}');
    gBrandyLexer.add_edge(root, gBrandyLexer.create_state(token_types::OPEN_PAREN), '(');
    gBrandyLexer.add_edge(root, gBrandyLexer.create_state(token_types::CLOSE_PAREN), ')');
    gBrandyLexer.add_edge(root, gBrandyLexer.create_state(token_types::OPEN_BRACKET), '[');
    gBrandyLexer.add_edge(root, gBrandyLexer.create_state(token_types::CLOSE_BRACKET), ']');
  }

  // ---------------------------------------------------------------------------

  bool tokenize_string(const char *str, std::vector<token> &tokens)
  {
    size_t lineNum = 1;

    while (*str)
    {
      brandy::token tok;
      if (!brandy::gBrandyLexer.read_token(str, &tok)) return false;


      switch (tok.type())
      {
      case token_types::BLOCK_COMMENT:
        // Block comments can have any number of newlines in them, so loop over
        // it and check for any newlines
        for (size_t i = 0; i < tok.length(); ++i)
        {
          if (tok.text()[i] == '\n')
            ++lineNum;
        }
        break;
      case token_types::LINE_COMMENT:
      case token_types::NEWLINE:
        // Increment the counter when hitting a line comment
        ++lineNum;
        break;
      case token_types::WHITESPACE:
        // Do nothing for whitespace
        break;
      default:
        // Add the character
        tok.line_number(lineNum);
        tokens.push_back(tok);
        break;
      }

      str += tok.length();
    }

    return true;
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
