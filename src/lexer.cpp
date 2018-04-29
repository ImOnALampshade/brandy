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
    m_states[from].edges[(unsigned char)c] = to;
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
    m_states[from].default_edge = to;
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

  token_types::type lexer::accept_value(state_reference state) const
  {
    return m_states[state].accept;
  }

  void lexer::accept_value(state_reference state, token_types::type t)
  {
    m_states[state].accept = t;
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
        token_types::names[from.accept], (int) from.index,
        token_types::names[to.accept], (int) to.index,
        name);
    };

    for (const state &s1 : m_states)
    {
      fprintf(f, "  %s%i [label=\"", token_types::names[s1.accept], (int) s1.index);

      const char *nodeName = token_types::names[s1.accept];
      while (*nodeName)
        fputc(tolower(*(nodeName++)), f);

      fputs("\"];\n", f);

      for (const std::pair<char, size_t> &edge : s1.edges)
      {
        char str[] = { edge.first, '\0' };
        const char *c = str;
        switch (edge.first)
        {
        case '\n':
          c = "Newline";
          break;
        case ' ':
          c = "Space";
          break;
        case '\t':
          c = "Tab";
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

      add_edge(s1, "default", s1.default_edge);
      add_edge(s1, "letter", s1.letter_edge);
      add_edge(s1, "number", s1.number_edge);
    }

    fputc('}', f);
  }

  // ---------------------------------------------------------------------------

  inline const lexer::state *
  lexer::get_next_from(const state *state, char c) const
  {
    auto found = state->edges.find(c);
    if (found != state->edges.end())
      return &m_states[found->second];
    else if (state->letter_edge != INVALID_EDGE && isalpha(c))
      return &m_states[state->letter_edge];
    else if (state->number_edge != INVALID_EDGE && isdigit(c))
      return &m_states[state->number_edge];
    else if (state->default_edge != INVALID_EDGE)
      return &m_states[state->default_edge];
    else
      return nullptr;
  }

  // ---------------------------------------------------------------------------

  lexer::state::state(token_types::type accept) :
    accept(accept),
    number_edge(INVALID_EDGE),
    letter_edge(INVALID_EDGE),
    default_edge(INVALID_EDGE)
  {
  }

  // ---------------------------------------------------------------------------

  lexer gBrandyLexer;

  // ---------------------------------------------------------------------------

  static void add_operator(
    lexer::state_reference from,
    token_types::type finish,
    const char *str)
  {
    const char *startStr = str;
    while (str[1] != 0)
    {
      lexer::state_reference intermediate = gBrandyLexer.get_edge(from, *str);

      if (intermediate == INVALID_EDGE)
      {
        intermediate = gBrandyLexer.create_state();
        gBrandyLexer.add_edge(from, intermediate, *str);
      }

      from = intermediate;
      ++str;
    }

    lexer::state_reference acceptState = gBrandyLexer.get_edge(from, *str);

    if (acceptState == INVALID_EDGE)
      gBrandyLexer.add_edge(from, gBrandyLexer.create_state(finish), *str);
    else if (gBrandyLexer.accept_value(acceptState) == token_types::INVALID)
      gBrandyLexer.accept_value(acceptState, finish);
    else
    {
      const char *currName = token_types::names[finish];
      const char *oldName = token_types::names[gBrandyLexer.accept_value(acceptState)];
      printf("Duplicate operator %s, %s, old operator was %s\n",
        startStr,
        currName,
        oldName);
    }
  }

  static void add_keyword(
    lexer::state_reference from,
    lexer::state_reference identifier,
    lexer::state_reference finish,
    const char *str)
  {
    while (str[1] != 0)
    {
      // get the edge from here to the next letter in the string
      lexer::state_reference intermediate = gBrandyLexer.get_edge(from, *str);

      // Check for no edge or one that goes to THE identifier state
      // (not just any identifier)
      if (intermediate == INVALID_EDGE || intermediate == identifier)
      {
        // Create a new edge and hook it up
        intermediate = gBrandyLexer.create_state(token_types::IDENTIFIER);

        gBrandyLexer.add_letter_edge(intermediate, identifier);
        gBrandyLexer.add_number_edge(intermediate, identifier);
        gBrandyLexer.add_edge(intermediate, identifier, '_');
        gBrandyLexer.add_edge(intermediate, identifier, '$');
        gBrandyLexer.add_edge(intermediate, identifier, '@');

        gBrandyLexer.add_edge(from, intermediate, *str);
      }

      from = intermediate;
      ++str;
    }

    gBrandyLexer.add_letter_edge(finish, identifier);
    gBrandyLexer.add_number_edge(finish, identifier);
    gBrandyLexer.add_edge(finish, identifier, '_');
    gBrandyLexer.add_edge(finish, identifier, '$');
    gBrandyLexer.add_edge(finish, identifier, '@');

    // Add an edge to the finishing state
    gBrandyLexer.add_edge(from, finish, *str);
  }

  void setup_lexer()
  {
    auto root = gBrandyLexer.root();

    auto identifier = gBrandyLexer.create_state(token_types::IDENTIFIER);
    gBrandyLexer.add_edge(root, identifier, '_');
    gBrandyLexer.add_edge(root, identifier, '$');
    gBrandyLexer.add_edge(root, identifier, '@');
    gBrandyLexer.add_letter_edge(root, identifier);

    gBrandyLexer.add_edge(identifier, identifier, '_');
    gBrandyLexer.add_edge(identifier, identifier, '$');
    gBrandyLexer.add_edge(identifier, identifier, '@');
    gBrandyLexer.add_letter_edge(identifier, identifier);
    gBrandyLexer.add_number_edge(identifier, identifier);

    auto whitespace = gBrandyLexer.create_state(token_types::WHITESPACE);
    gBrandyLexer.add_edge(root, whitespace, ' ');
    gBrandyLexer.add_edge(root, whitespace, '\t');
    gBrandyLexer.add_edge(whitespace, whitespace, ' ');
    gBrandyLexer.add_edge(whitespace, whitespace, '\t');

    auto newline = gBrandyLexer.create_state(token_types::NEWLINE);
    gBrandyLexer.add_edge(root, newline, '\n');

    auto div = gBrandyLexer.create_state();
    auto lineComment = gBrandyLexer.create_state(token_types::LINE_COMMENT);
    auto lineCommentEnd = gBrandyLexer.create_state(token_types::LINE_COMMENT);
    gBrandyLexer.add_edge(root, div, '/');
    gBrandyLexer.add_edge(div, lineComment, '/');
    gBrandyLexer.add_default_edge(lineComment, lineComment);
    gBrandyLexer.add_edge(lineComment, lineCommentEnd, '\n');

    auto blockCommentStart = gBrandyLexer.create_state();
    auto blockCommentEnd1 = gBrandyLexer.create_state();
    auto blockCommentEnd2 = gBrandyLexer.create_state(token_types::BLOCK_COMMENT);
    gBrandyLexer.add_edge(div, blockCommentStart, '*');
    gBrandyLexer.add_default_edge(blockCommentStart, blockCommentStart);
    gBrandyLexer.add_edge(blockCommentStart, blockCommentEnd1, '*');
    gBrandyLexer.add_default_edge(blockCommentEnd1, blockCommentStart);
    gBrandyLexer.add_edge(blockCommentEnd1, blockCommentEnd2, '/');

    auto hash = gBrandyLexer.create_state();
    auto shebang = gBrandyLexer.create_state(token_types::SHEBANG);
    auto shebangEnd = gBrandyLexer.create_state(token_types::SHEBANG);
    gBrandyLexer.add_edge(root, hash, '#');
    gBrandyLexer.add_edge(hash, shebang, '!');
    gBrandyLexer.add_default_edge(shebang, shebang);
    gBrandyLexer.add_edge(shebang, shebangEnd, '\n');

    auto number = gBrandyLexer.create_state(token_types::_INT32);
    auto intType = gBrandyLexer.create_state();
    auto uintType = gBrandyLexer.create_state();
    gBrandyLexer.add_number_edge(root, number);
    gBrandyLexer.add_number_edge(number, number);
    gBrandyLexer.add_edge(number, intType, 'i');
    gBrandyLexer.add_edge(number, intType, 'I');
    gBrandyLexer.add_edge(number, uintType, 'u');
    gBrandyLexer.add_edge(number, uintType, 'U');

    auto decimal = gBrandyLexer.create_state();
    auto decimalNumber = gBrandyLexer.create_state(token_types::_FLOAT64);
    gBrandyLexer.add_edge(number, decimal, '.');
    gBrandyLexer.add_number_edge(decimal, decimalNumber);
    gBrandyLexer.add_number_edge(decimalNumber, decimalNumber);

    auto exponent = gBrandyLexer.create_state();
    auto exponentPlus = gBrandyLexer.create_state();
    auto exponentMinus = gBrandyLexer.create_state();
    auto exponentValue = gBrandyLexer.create_state(token_types::_FLOAT64);
    gBrandyLexer.add_edge(number, exponent, 'e');
    gBrandyLexer.add_edge(number, exponent, 'E');
    gBrandyLexer.add_edge(decimalNumber, exponent, 'e');
    gBrandyLexer.add_edge(decimalNumber, exponent, 'E');
    gBrandyLexer.add_edge(exponent, exponentPlus, '+');
    gBrandyLexer.add_edge(exponent, exponentMinus, '-');
    gBrandyLexer.add_number_edge(exponentPlus, exponentValue);
    gBrandyLexer.add_number_edge(exponentMinus, exponentValue);
    gBrandyLexer.add_number_edge(exponent, exponentValue);
    gBrandyLexer.add_number_edge(exponentValue, exponentValue);

    auto floatType = gBrandyLexer.create_state(token_types::_FLOAT32);
    gBrandyLexer.add_edge(decimalNumber, floatType, 'f');
    gBrandyLexer.add_edge(decimalNumber, floatType, 'F');
    gBrandyLexer.add_edge(exponentValue, floatType, 'f');
    gBrandyLexer.add_edge(exponentValue, floatType, 'F');

    auto i8  = gBrandyLexer.create_state(token_types::_INT8);
    auto i1  = gBrandyLexer.create_state();
    auto i16 = gBrandyLexer.create_state(token_types::_INT16);
    auto i3  = gBrandyLexer.create_state();
    auto i32 = gBrandyLexer.create_state(token_types::_INT32);
    auto i6  = gBrandyLexer.create_state();
    auto i64 = gBrandyLexer.create_state(token_types::_INT64);
    gBrandyLexer.add_edge(intType, i8, '8');
    gBrandyLexer.add_edge(intType, i1, '1');
    gBrandyLexer.add_edge(intType, i3, '3');
    gBrandyLexer.add_edge(intType, i6, '6');
    gBrandyLexer.add_edge(i1, i16, '6');
    gBrandyLexer.add_edge(i3, i32, '2');
    gBrandyLexer.add_edge(i6, i64, '4');

    auto u8  = gBrandyLexer.create_state(token_types::_UINT8);
    auto u1  = gBrandyLexer.create_state();
    auto u16 = gBrandyLexer.create_state(token_types::_UINT16);
    auto u3  = gBrandyLexer.create_state();
    auto u32 = gBrandyLexer.create_state(token_types::_UINT32);
    auto u6  = gBrandyLexer.create_state();
    auto u64 = gBrandyLexer.create_state(token_types::_UINT64);
    gBrandyLexer.add_edge(uintType, u8, '8');
    gBrandyLexer.add_edge(uintType, u1, '1');
    gBrandyLexer.add_edge(uintType, u3, '3');
    gBrandyLexer.add_edge(uintType, u6, '6');
    gBrandyLexer.add_edge(u1, u16, '6');
    gBrandyLexer.add_edge(u3, u32, '2');
    gBrandyLexer.add_edge(u6, u64, '4');

    auto stringStart = gBrandyLexer.create_state();
    auto stringInner = gBrandyLexer.create_state();
    auto strEscapeSequence = gBrandyLexer.create_state();
    auto stringEnd = gBrandyLexer.create_state(token_types::_STRING);
    gBrandyLexer.add_edge(root, stringStart, '"');
    gBrandyLexer.add_default_edge(stringStart, stringInner);
    gBrandyLexer.add_default_edge(stringInner, stringInner);
    gBrandyLexer.add_edge(stringStart, strEscapeSequence, '\\');
    gBrandyLexer.add_edge(stringInner, strEscapeSequence, '\\');
    gBrandyLexer.add_default_edge(strEscapeSequence, stringInner);
    gBrandyLexer.add_edge(stringInner, stringEnd, '"');

    auto charStart = gBrandyLexer.create_state();
    auto chEscapeSequence = gBrandyLexer.create_state();
    auto charEnd = gBrandyLexer.create_state(token_types::_CHAR);
    gBrandyLexer.add_edge(root, charStart, '\'');
    gBrandyLexer.add_default_edge(charStart, charStart);
    gBrandyLexer.add_edge(charStart, chEscapeSequence, '\\');
    gBrandyLexer.add_default_edge(chEscapeSequence, charStart);
    gBrandyLexer.add_edge(charStart, charEnd, '\'');

    auto docStrStart2 = gBrandyLexer.create_state(token_types::_STRING);
    auto docStrStart3 = gBrandyLexer.create_state();
    auto docStrEnd1 = gBrandyLexer.create_state();
    auto docStrEnd2 = gBrandyLexer.create_state();
    auto docStrEnd3 = gBrandyLexer.create_state(token_types::DOCUMENTION_BLOCK);
    gBrandyLexer.add_edge(stringStart, docStrStart2, '"');
    gBrandyLexer.add_edge(docStrStart2, docStrStart3, '"');
    gBrandyLexer.add_default_edge(docStrStart3, docStrStart3);
    gBrandyLexer.add_default_edge(docStrEnd1, docStrStart3);
    gBrandyLexer.add_default_edge(docStrEnd2, docStrStart3);
    gBrandyLexer.add_edge(docStrStart3, docStrEnd1, '"');
    gBrandyLexer.add_edge(docStrEnd1, docStrEnd2, '"');
    gBrandyLexer.add_edge(docStrEnd2, docStrEnd3, '"');

    auto regexStart = gBrandyLexer.create_state();
    auto regexEnd = gBrandyLexer.create_state(token_types::REGEX);
    auto regexEndOpt = gBrandyLexer.create_state(token_types::REGEX);
    gBrandyLexer.add_default_edge(div, regexStart);
    gBrandyLexer.add_default_edge(regexStart, regexStart);
    gBrandyLexer.add_edge(regexStart, regexEnd, '/');
    gBrandyLexer.add_letter_edge(regexEnd, regexEndOpt);

    for (int i = token_types::OPERATORS_START + 1; i < token_types::OPERATORS_END; ++i)
      add_operator(
        root,
        token_types::type(i),
        token_types::regex[i]);

    for (int i = token_types::KEYWORDS_START + 1; i < token_types::KEYWORDS_END; ++i)
      add_keyword(
        root,
        identifier,
        gBrandyLexer.create_state(token_types::type(i)),
        token_types::regex[i]);

#define KEYWORD(accept, word) \
  add_keyword( \
    root, \
    identifier, \
    gBrandyLexer.create_state(token_types::accept), \
    word);

    KEYWORD(LOGICAL_AND, "and");
    KEYWORD(LOGICAL_OR, "or");
    KEYWORD(LOGICAL_NOT, "not");
  }

  // ---------------------------------------------------------------------------

  bool tokenize_string(const char *str, std::vector<token> &tokens)
  {
    size_t lineNum = 1;

    while (*str)
    {
      brandy::token tok;
      if (!brandy::gBrandyLexer.read_token(str, &tok)) return false;

      tok.line_number(lineNum);
      tokens.push_back(tok);

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
      case token_types::SHEBANG:
      case token_types::NEWLINE:
      case token_types::LINE_COMMENT:
        // Increment the counter when hitting a line comment or newline
        ++lineNum;
        break;

      default: break;
      }

      str += tok.length();
    }

    return true;
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
