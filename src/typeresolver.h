// -----------------------------------------------------------------------------
// Brandy type resolution visitor
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef TYPE_RESOLVER_H
#define TYPE_RESOLVER_H

#pragma once

#include "astnodes.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  struct assignment_graph
  {
  public:
    typedef size_t node_reference;

    struct node
    {
      abstract_node *ast_node;
      std::vector<node_reference> children;
    };

    assignment_graph();

    node *get_node(node_reference reference);

  private:
    std::vector<node> m_nodes;
  };

  // ---------------------------------------------------------------------------

  class type_resolver : public ast_visitor
  {
  public:
    ast_visitor::visitor_result visit(module_node *node) override;
    ast_visitor::visitor_result visit(binary_operator_node *node) override;
    ast_visitor::visitor_result visit(function_node *node) override;
    ast_visitor::visitor_result visit(lambda_node *node) override;
    ast_visitor::visitor_result visit(var_node *node) override;

  private:
    assignment_graph m_assignments;
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
