// -----------------------------------------------------------------------------
// Brandy language tokens
// Howard Hughes
// -----------------------------------------------------------------------------

#include "token.h"
#include <algorithm>

// -----------------------------------------------------------------------------

namespace brandy
{
  namespace token_types
  {
#define TOKEN_VALUE(val, regex) #val
    const char *names[] =
    {
#include "token_types.inl"
      nullptr
    };
#undef TOKEN_VALUE
#define TOKEN_VALUE(val, regex) regex
    const char *regex[] =
    {
#include "token_types.inl"
      nullptr
    };
#undef TOKEN_VALUE
  }

  token token::invalid;

  token::token() :
    m_text(nullptr),
    m_len(0),
    m_tokType(token_types::INVALID)
  {
  }

  token::token(const char *str, token_types::type type) :
    m_text(str),
    m_len(strlen(m_text)),
    m_tokType(type),
    m_lineNum(0)
  {
  }

  token::token(const char *str, size_t length, token_types::type type) :
    m_text(str),
    m_len(length),
    m_tokType(type),
    m_lineNum(0)
  {
  }

  const char       *token::text() const { return m_text; }
  size_t          token::length() const { return m_len; }
  token_types::type token::type() const { return m_tokType; }
  size_t     token::line_number() const { return m_lineNum; }

  void token::line_number(size_t n) { m_lineNum = n; }

#define GET_16_BITS(d) (*((const std::uint16_t *) (d)))

  std::int32_t token::hash_code() const
  {
    size_t      len = m_len;
    const char *data = m_text;

    std::uint32_t hash = std::uint32_t(len), tmp;
    int rem;

    if (len == 0) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (; len > 0; len--) {
      hash += GET_16_BITS(data);
      tmp = (GET_16_BITS(data + 2) << 11) ^ hash;
      hash = (hash << 16) ^ tmp;
      data += 2 * sizeof(uint16_t);
      hash += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
    case 3:
      hash += GET_16_BITS(data);
      hash ^= hash << 16;
      hash ^= ((signed char)data[sizeof(uint16_t)]) << 18;
      hash += hash >> 11;
      break;

    case 2:
      hash += GET_16_BITS(data);
      hash ^= hash << 11;
      hash += hash >> 17;
      break;

    case 1:
      hash += (signed char)*data;
      hash ^= hash << 10;
      hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
  }

  bool token::operator==(const token &rhs) const
  {
    return tokcmp(*this, rhs) == 0;
  }

  bool token::operator!=(const token &rhs) const
  {
    return tokcmp(*this, rhs) != 0;
  }

  int tokcmp(const token &tok1, const token &tok2)
  {
    int minlen = std::min(tok1.length(), tok2.length());
    return std::strncmp(tok1.text(), tok2.text(), minlen);
  }

  int tokcmp(const char *str, const token &tok)
  {
    return std::strncmp(tok.text(), str, tok.length());
  }

  int tokcmp(const token &tok, const char *str)
  {
    return std::strncmp(tok.text(), str, tok.length());
  }

  std::ostream &operator<<(std::ostream &os, const brandy::token &tok)
  {
    for (size_t i = 0; i < tok.length(); ++i)
    {
      char c = tok.text()[i];
      switch (c)
      {
      case '\n':
        os << "\\n";
        break;
      case '\t':
        os << "\\t";
        break;
      default:
        os << c;
      }
    }

    return os;
  }
}

// -----------------------------------------------------------------------------

size_t std::hash<brandy::token>::operator()(const brandy::token &tok) const
{
  return tok.hash_code();
}


// -----------------------------------------------------------------------------
