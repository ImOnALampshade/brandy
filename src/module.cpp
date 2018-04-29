// -----------------------------------------------------------------------------
// Read file utility
// Howard Hughes
// -----------------------------------------------------------------------------

#include "lexer.h"
#include "module.h"
#include "../lib/termcolor/termcolor.hpp"
#include <iostream>
#include <stdio.h>
#include <iomanip>

#define TODO(s)
#define until(exp) while(!(exp))

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  void module::load(const char *filename)
  {
    m_filePath = filename;
    // Open the file
    FILE *pfile = fopen(filename, "rt");
    if (!pfile)
    {
      TODO("Throw here!");
      std::cout << "File not found " << filename << std::endl;
      return;
    }

    // This isn't portable though basically every modern file system supports it
    // But we're just using it to call std::vector::reserve, so if it gives us a
    // bad result, that doesn't do anything bad - just leads to an extra call to
    // malloc/new, and maybe wastes a bit of memory
    fseek(pfile, 0, SEEK_END);
    size_t fileLen = ftell(pfile) + 3; // Add 3 for the \0 and trailing \n
    rewind(pfile);

    m_text.reserve(fileLen);

    m_text.push_back('\0');

    while (!feof(pfile))
    {
      char c = fgetc(pfile);
      if (c == EOF) break;

      m_text.push_back(c);
    }

    // Ensure the file ends with a newline and null terminator
    m_text.push_back('\n');
    m_text.push_back('\0');

    m_text.shrink_to_fit();

    if (!brandy::tokenize_string(m_text.data() + 1, m_tokens))
    {
      TODO("Throw here!");
      std::cout << "Failed to lex file " << filename << std::endl;
      return;
    }
  }

  // ---------------------------------------------------------------------------

  static const char *backtrack_to_newline(const char *s)
  {
    until (*s == '\n' || *s == '\0')
      --s;

    return s + 1;
  }

  static const char *start_of_next_line(const char *s)
  {
    until (*s == '\n' || *s == '\0')
      ++s;

    return s + 1;
  }

  static size_t line_length(const char *s)
  {
    size_t len = 0;
    until (*s == '\n' || *s == '\0')
      ++len,++s;
    return len;
  }

  void module::print_error(error_base &e)
  {
    using std::cout;
    using std::endl;

    size_t tokenIndex = e.token_index;
    while(m_tokens[tokenIndex].type() == token_types::NEWLINE)
      ++tokenIndex;

    const token &t = m_tokens[tokenIndex];
    const char *middleLine = backtrack_to_newline(t.text());

    std::string message = e.error_message();

    size_t lineNo = t.line_number();
    size_t colNo = (size_t)(t.text() - middleLine);
    size_t lineLen = line_length(middleLine);

    cout << termcolor::bold << m_filePath << ":" <<  lineNo << ":" << colNo << ": ";
    cout << termcolor::red << e.severity() << ": " << termcolor::reset << termcolor::bold;
    cout << message << "\n    ";

    size_t i = e.token_index;
    while (i > 0 && m_tokens[i - 1].line_number() == lineNo)
      --i;

    while (m_tokens[i].line_number() == lineNo)
    {
      size_t type = m_tokens[i].type();

      if(token_types::LITERALS_START < type && type < token_types::LITERALS_END)
        std::cout << termcolor::blue;

      else if (token_types::KEYWORDS_START < type && type < token_types::KEYWORDS_END)
        std::cout << termcolor::green;

      else if(token_types::COMMENTS_START < type && type < token_types::COMMENTS_END)
        std::cout << termcolor::magenta;

      else if(token_types::OPERATORS_START < type && type < token_types::OPERATORS_END)
        std::cout << termcolor::yellow;

      else if (token_types::NEWLINE == type)
        break;

      std::cout << m_tokens[i] << termcolor::reset << termcolor::bold;
      ++i;
    }


    cout << "\n    " << std::setw(colNo) << "";
    cout << termcolor::green << std::setw(t.length()) << std::setfill('^') << '^' << endl;

    cout << termcolor::reset;

    cout << token_types::names[t.type()] << " (" << t << ") :" << e.token_index << "\n";
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

