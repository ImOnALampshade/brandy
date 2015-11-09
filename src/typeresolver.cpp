// -----------------------------------------------------------------------------
// Brandy type resolution visitor
// Howard Hughes
// -----------------------------------------------------------------------------

#include "typeresolver.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  namespace
  {
    bool is_assignment(const token &tok)
    {
      return
        token_types::ASSIGNMENT_START < tok.type() &&
        tok.type() < token_types::ASSIGNMENT_END;
    }
  }

  // ---------------------------------------------------------------------------

  assignment_graph::assignment_graph()
  {
  }

  assignment_graph::node *assignment_graph::get_node(node_reference reference)
  {
    return (reference < m_nodes.size()) ? &m_nodes[reference] : nullptr;
  }

  // ---------------------------------------------------------------------------

  class assignment_graph_builder : public ast_visitor
  {
  public:
    assignment_graph_builder(assignment_graph *graph) :
      m_graph(graph)
    {
    }

  private:
    assignment_graph *m_graph;
  };

  // ---------------------------------------------------------------------------

  ast_visitor::visitor_result type_resolver::visit(module_node *node)
  {
    // Walk our children to find types that we can find easily
    walk_node(node, this, false);

    // Build up our assignment graph
    assignment_graph_builder graph_builder(&m_assignments);
    walk_node(node, &graph_builder);

    // Don't walk our children, we already did
    return ast_visitor::stop;
  }

  ast_visitor::visitor_result type_resolver::visit(binary_operator_node *node)
  {
    return ast_visitor::resume;
  }

  ast_visitor::visitor_result type_resolver::visit(function_node *node)
  {
    // If any of the parameters have no defined type, then this is a template
    // we can't resolve types here until later
    for (auto &param : node->parameters)
      if (param->type.get() == nullptr) return ast_visitor::stop;

    return ast_visitor::resume;
  }

  ast_visitor::visitor_result type_resolver::visit(lambda_node *node)
  {
    // If any of the parameters have no defined type, then this is a template
    // we can't resolve types here until later
    for (auto &param : node->parameters)
      if (param->type.get() == nullptr) return ast_visitor::stop;

    return ast_visitor::resume;
  }

  ast_visitor::visitor_result type_resolver::visit(var_node *node)
  {
    // Walk this node
    walk_node(node, this, false);

    // If this node has a type, then we don't need to do any work to fill in this node
    if (node->type.get() && node->type->resulting_type)
    {
      // Our type is just the type of the operation
      node->var_type = node->type->resulting_type;
    }
    // If there is an assignment and we can determine its type
    else if (node->expression && node->expression->resulting_type)
    {
      // Fill in our type with the resulting type of the expression
      node->var_type = node->expression->resulting_type;
    }

    return ast_visitor::stop;
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
