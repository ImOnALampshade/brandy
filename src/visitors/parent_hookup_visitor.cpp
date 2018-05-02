// -----------------------------------------------------------------------------
// Brandy ast visitor for hookuping up AST nodes to their parents
// Howard Hughes
// -----------------------------------------------------------------------------

#include "parent_hookup_visitor.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  parent_hookup_visitor::parent_hookup_visitor() :
    m_connectTo(nullptr)
  {
  }

  ast_visitor::visitor_result parent_hookup_visitor::visit(abstract_node *node)
  {
    if (m_connectTo)
      node->parent = m_connectTo;
    else
    {
      m_connectTo = node;
      walk_node(node, this, false);
    }

    return ast_visitor::stop;
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
