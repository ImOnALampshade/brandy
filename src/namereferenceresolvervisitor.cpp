// -----------------------------------------------------------------------------
// AST visitor for resolving name references to symbol definitions
// Howard Hughes
// -----------------------------------------------------------------------------

#include "namereferenceresolvervisitor.h"
#include <iostream>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  ast_visitor::visitor_result name_reference_resolver_visitor::visit(name_reference_node *node)
  {
    node->resolved_symbol = get_symbol(node->name);
    if (!node->resolved_symbol)
    {
      // TOOD: Name error
    }
    return ast_visitor::resume;
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
