// -----------------------------------------------------------------------------
// Read file utility
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef READ_FILE_H
#define READ_FILE_H

#pragma once

#include "error_base.h"
#include "token.h"
#include <memory>
#include <vector>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  class module
  {
  public:
    void load(const char *file);

    // ast_root &root_node();
    // const ast_root &root_node() const;

    void print_error(error_base &e);

  // private:
    std::string m_filePath;
    std::vector<char> m_text;
    std::vector<token> m_tokens;
    // ast_root
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
