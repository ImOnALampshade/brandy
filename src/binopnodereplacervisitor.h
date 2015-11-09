// -----------------------------------------------------------------------------
// AST visitor for replacing Binary Operator nodes with call nodes
// Drew Jacobsen
// -----------------------------------------------------------------------------

#ifndef BIN_OP_WALKER
#define BIN_OP_WALKER

#pragma once

#include "astnodes.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  class bin_op_replacer_visitor : public ast_visitor
  {
  public:
    //bin_op_visitor();

    ast_visitor::visitor_result visit(binary_operator_node *node) override;

  private:
    //nothing im pretty sure
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
