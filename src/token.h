// -----------------------------------------------------------------------------
// Brandy language tokens
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef TOKEN_H
#define TOKEN_H

#pragma once

#include <cstdint>
#include <functional>
#include <ostream>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  namespace token_types
  {
#define TOKEN_VALUE(val, regex) val
    enum type
    {
#include "token_types.inl"
      COUNT
    };
#undef TOKEN_VALUE

    extern const char *names[];
    extern const char *regex[];
  }

  // ---------------------------------------------------------------------------

  class token
  {
  public:
    static token invalid;

    token();
    token(const char *str, token_types::type type);
    token(const char *str, size_t length, token_types::type type);

    const char       *text() const;
    size_t          length() const;
    token_types::type type() const;
    size_t     line_number() const;

    void line_number(size_t n);

    std::int32_t hash_code() const;

    bool operator==(const token &rhs) const;
    bool operator!=(const token &rhs) const;

  // private:
    const char *m_text;
    size_t m_len;
    token_types::type m_tokType;
    size_t m_lineNum;
  };

  // strcmp for tokens
  int tokcmp(const token &tok1, const token &tok2);
  int tokcmp(const char *str, const token &tok);
  int tokcmp(const token &tok, const char *str);

  std::ostream &operator<<(std::ostream &os, const brandy::token &tok);

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

namespace std
{
  template<>
  struct hash<brandy::token>
  {
  public:
    typedef brandy::token argument_type;
    typedef size_t result_type;

    size_t operator()(const brandy::token &tok);
  };
}

// -----------------------------------------------------------------------------

#endif
