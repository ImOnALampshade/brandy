// -----------------------------------------------------------------------------
// Brandy syntax tree nodes
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef ACTION_TREE_NODES_H
#define ACTION_TREE_NODES_H

#pragma once

#include "ast_nodes.h"
#include <cstdint>
#include <string>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  void value_reference::generate_ir_name()
  {
    static std::uint64_t tempCounter = 0;

    switch(type)
    {
    case type_name_reference:
      ir_name.assign(origin_token.text(), origin_token.length());
      break;
    case type_intermediate:
      ir_name = "temp_" + std::to_string(tempCounter);
      ++tempCounter;
      break;
    }
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
