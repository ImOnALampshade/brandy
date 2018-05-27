// -----------------------------------------------------------------------------
// Brandy ast visitor to resolve symbol references
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef SYMBOL_RESOLVER_VISITOR_H
#define SYMBOL_RESOLVER_VISITOR_H

#pragma once

#include "symbol_walker_visitor.h"
#include <stack>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  struct symbol_resolver_visitor : symbol_walker_visitor
  {
    ast_visitor::visitor_result visit(module_node *node) override;
    ast_visitor::visitor_result visit(function_node *node) override;
    ast_visitor::visitor_result visit(lambda_node *node) override;
    ast_visitor::visitor_result visit(var_node *node) override;
    ast_visitor::visitor_result visit(binary_operator_node *node) override;
    ast_visitor::visitor_result visit(unary_operator_node *node) override;
    ast_visitor::visitor_result visit(literal_node *node) override;
    ast_visitor::visitor_result visit(name_reference_node *node) override;
    ast_visitor::visitor_result visit(member_access_node *node) override;
    ast_visitor::visitor_result visit(call_node *node) override;
    ast_visitor::visitor_result visit(cast_node *node) override;
    ast_visitor::visitor_result visit(index_node *node) override;
    ast_visitor::visitor_result visit(type_node *node) override;

    module_node *current_module();

    std::stack<module_node *> m_current_module_stack;
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
