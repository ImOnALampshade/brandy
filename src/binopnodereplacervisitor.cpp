// -----------------------------------------------------------------------------
// AST visitor for replacing Binary Operator nodes with call nodes
// Drew Jacobsen
// -----------------------------------------------------------------------------

#include "binopnodereplacervisitor.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  ast_visitor::visitor_result bin_op_replacer_visitor::visit(binary_operator_node *node)
  {
    std::unique_ptr<call_node> newCallNode = std::make_unique<call_node>();

    token nameToken;
    switch(node->operation.type())
    {
    case token_types::ASSIGNMENT:
      nameToken = token("@assign", token_types::IDENTIFIER);
      break;

    case token_types::ASSIGNMENT_ADD:
      nameToken = token("@assign_add", token_types::IDENTIFIER);
      break;

    case token_types::ASSIGNMENT_SUBTRACT:
      nameToken = token("@assign_subtract", token_types::IDENTIFIER);
      break;

    case token_types::ASSIGNMENT_MULTIPLY:
      nameToken = token("@assign_multiply", token_types::IDENTIFIER);
      break;

    case token_types::ASSIGNMENT_DIVIDE:
      nameToken = token("@assign_divide", token_types::IDENTIFIER);
      break;

    case token_types::ASSIGNMENT_MODULO:
      nameToken = token("@assign_modulo", token_types::IDENTIFIER);
      break;

    case token_types::ASSIGNMENT_BITWISE_AND:
      nameToken = token("@assign_bitwise_and", token_types::IDENTIFIER);
      break;

    case token_types::ASSIGNMENT_BITWISE_OR:
      nameToken = token("@assign_bitwise_or", token_types::IDENTIFIER);
      break;

    case token_types::ASSIGNMENT_BITWISE_XOR:
      nameToken = token("@assign_bitwise_xor", token_types::IDENTIFIER);
      break;

    case token_types::ASSIGNMENT_BITWISE_LEFT_SHIFT:
      nameToken = token("@assign_bitwise_left_shift", token_types::IDENTIFIER);
      break;

    case token_types::ASSIGNMENT_BITWISE_RIGHT_SHIFT:
      nameToken = token("@assign_bitwise_right_shift", token_types::IDENTIFIER);
      break;

    case token_types::ASSIGNMENT_LOGICAL_AND:
      nameToken = token("@assign_logical_and", token_types::IDENTIFIER);
      break;

    case token_types::ASSIGNMENT_LOGICAL_OR:
      nameToken = token("@assign_logical_or", token_types::IDENTIFIER);
      break;

    case token_types::ADD:
      nameToken = token("@add", token_types::IDENTIFIER);
      break;

    case token_types::SUBTRACT:
      nameToken = token("@subtract", token_types::IDENTIFIER);
      break;

    case token_types::ASTRISK:
      nameToken = token("@astrisk", token_types::IDENTIFIER);
      break;

    case token_types::DIVIDE:
      nameToken = token("@divide", token_types::IDENTIFIER);
      break;

    case token_types::MODULO:
      nameToken = token("@modulo", token_types::IDENTIFIER);
      break;

    case token_types::AMPERSAND:
      nameToken = token("@ampersand", token_types::IDENTIFIER);
      break;

    case token_types::BITWISE_OR:
      nameToken = token("@bitwise_or", token_types::IDENTIFIER);
      break;

    case token_types::BITWISE_XOR:
      nameToken = token("@bitwise_xor", token_types::IDENTIFIER);
      break;

    case token_types::BITWISE_LEFT_SHIFT:
      nameToken = token("@bitwise_left_shift", token_types::IDENTIFIER);
      break;

    case token_types::BITWISE_RIGHT_SHIFT:
      nameToken = token("@bitwise_right_shift", token_types::IDENTIFIER);
      break;

    case token_types::LOGICAL_AND:
      nameToken = token("@logical_and", token_types::IDENTIFIER);
      break;

    case token_types::LOGICAL_OR:
      nameToken = token("@logical_or", token_types::IDENTIFIER);
      break;

    case token_types::BITWISE_NOT:
      nameToken = token("@bitwise_not", token_types::IDENTIFIER);
      break;

    case token_types::LOGICAL_NOT:
      nameToken = token("@logical_not", token_types::IDENTIFIER);
      break;

    //case token_types::DOT: // this is a special case and handeled elseewhere

    case token_types::EQUALITY:
      nameToken = token("@equality", token_types::IDENTIFIER);
      break;

    case token_types::INEQUALITY:
      nameToken = token("@inequality", token_types::IDENTIFIER);
      break;

    case token_types::GREATER_THAN:
      nameToken = token("@greater_than", token_types::IDENTIFIER);
      break;

    case token_types::LESS_THAN:
      nameToken = token("@less_than", token_types::IDENTIFIER);
      break;

    case token_types::GREATER_THAN_OR_EQUAL:
      nameToken = token("@greather_than_or_equal", token_types::IDENTIFIER);
      break;

    case token_types::LESS_THAN_OR_EQUAL:
      nameToken = token("@less_than_or_equal", token_types::IDENTIFIER);
      break;

    //case token_types::TUPLE_EXPANSION: //this one is t obe ignored

    }

    std::unique_ptr<member_access_node> newMemAccNode = std::make_unique<member_access_node>();
    newMemAccNode->member_name = nameToken;
    newMemAccNode->left = std::move(node->left);

    newCallNode->left = std::move(newMemAccNode);
    newCallNode->parameters.push_back(std::move(node->right));
    
    replacement_node = std::move(newCallNode);
    return ast_visitor::replace;
  }
}
