// -----------------------------------------------------------------------------
// An AST visitor that adds the implicit "return" to one-statement functions
// Howard Hughes
// -----------------------------------------------------------------------------

#include "functionreturnvisitor.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  ast_visitor::visitor_result function_return_visitor::visit(function_node *node)
  {
    if (node->scope->statements.size() == 1)
    {
      // Only do it if the first statement is an expression
      if (auto expr = dynamic_cast<expression_node *>(node->scope->statements[0].get()))
      {
        // Release the ownership of the statement
        node->scope->statements[0].release();

        // Create a return node, set its value to the statement we read
        auto returnNode = std::make_unique<return_node>();
        returnNode->value = std::unique_ptr<expression_node>(expr);

        // Put our return statement in the statements list
        node->scope->statements[0] = move(returnNode);
      }
    }

    return ast_visitor::resume;
  }

  ast_visitor::visitor_result function_return_visitor::visit(lambda_node *node)
  {
    if (node->scope->statements.size() == 1)
    {
      // Only do it if the first statement is an expression
      if (auto expr = dynamic_cast<expression_node *>(node->scope->statements[0].get()))
      {
        // Release the ownership of the statement
        node->scope->statements[0].release();

        // Create a return node, set its value to the statement we read
        auto returnNode = std::make_unique<return_node>();
        returnNode->value = std::unique_ptr<expression_node>(expr);

        // Put our return statement in the statements list
        node->scope->statements[0] = move(returnNode);
      }
    }

    return ast_visitor::resume;
  }

  ast_visitor::visitor_result function_return_visitor::visit(property_node *node)
  {
    if (node->getter && node->getter->statements.size() == 1)
    {
      if (auto expr = dynamic_cast<expression_node *>(node->getter->statements[0].get()))
      {
        node->getter->statements[0].release();

        auto returnNode = std::make_unique<return_node>();
        returnNode->value = std::unique_ptr<expression_node>(expr);

        node->getter->statements[0] = move(returnNode);
      }
    }

    return ast_visitor::resume;
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
