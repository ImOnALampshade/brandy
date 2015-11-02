// -----------------------------------------------------------------------------
// AST visitor for resolving name references to symbol definitions
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef NAME_REFERENCE_RESOLVER_H
#define NAME_REFERENCE_RESOLVER_H

#pragma once

#include "symbolwalkervisitor.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  class name_reference_resolver_visitor : public symbol_table_visitor
  {
  public:
    ast_visitor::visitor_result visit(name_reference_node *node) override;
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
