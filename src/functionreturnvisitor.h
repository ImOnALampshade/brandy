// -----------------------------------------------------------------------------
// An AST visitor that adds the implicit "return" to one-statement functions
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef FUNCTION_RETURN_H
#define FUNCTION_RETURN_H

#pragma once

#include "astnodes.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  class function_return_visitor : public ast_visitor
  {
    virtual ast_visitor::visitor_result visit(function_node *node);
    virtual ast_visitor::visitor_result visit(lambda_node *node);
    virtual ast_visitor::visitor_result visit(property_node *node);
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
