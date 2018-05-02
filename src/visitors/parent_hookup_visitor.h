// -----------------------------------------------------------------------------
// Brandy ast visitor for hookuping up AST nodes to their parents
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef PARENT_HOOKUP_VISITOR_H
#define PARENT_HOOKUP_VISITOR_H

#pragma once

#include "../ast_nodes.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  struct parent_hookup_visitor : ast_visitor
  {
    parent_hookup_visitor();

    ast_visitor::visitor_result visit(abstract_node *node) override;
    
    abstract_node *m_connectTo;
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
