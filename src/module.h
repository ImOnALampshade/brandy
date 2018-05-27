// -----------------------------------------------------------------------------
// Read file utility
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef READ_FILE_H
#define READ_FILE_H

#pragma once

#include "ast_nodes.h"
#include "error.h"
#include "token.h"
#include <memory>
#include <vector>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  struct module
  {
    void load(const char *file);

    module_node *root_node();
    const module_node *root_node() const;

    void output_msg(error &e);
    
    std::string m_filePath;
    std::vector<char> m_text;
    std::vector<token> m_tokens;
    std::unique_ptr<module_node> m_module;
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
