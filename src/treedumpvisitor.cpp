// -----------------------------------------------------------------------------
// An AST visitor to dump the tree to stdout
// Howard Hughes
// -----------------------------------------------------------------------------

#include "treedumpvisitor.h"
#include <vector>
#include <sstream>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  namespace
  {
    class node_printer : public std::stringstream
    {
    public:
      node_printer()
      {
        for (auto printer : alive_printers)
          (*this) << "  ";

        alive_printers.push_back(this);
      }

      node_printer(const std::string& text)
      {
        for (size_t i = 0; i < alive_printers.size(); ++i)
          (*this) << "  ";

        alive_printers.push_back(this);
        (*this) << text;
      }

      ~node_printer()
      {
        alive_printers.pop_back();

        if (alive_printers.empty())
          printf("%s\n", this->str().c_str());
        else
          (*alive_printers.back()) << "\n" << this->str();
      }

    private:
      static std::vector<node_printer*> alive_printers;
    };

    std::vector<node_printer*> node_printer::alive_printers;
  }

  // ---------------------------------------------------------------------------

#define VISIT_NODE(node_type, print) \
  ast_visitor::visitor_result tree_dump_visitor::visit(node_type *node) \
  {\
    node_printer printer; \
    printer << #node_type "   " << print << std::endl; \
    walk_node(node, this, false); \
    return ast_visitor::stop; \
  }
#define VISIT_NODE2(node_type) VISIT_NODE(node_type, "")

  VISIT_NODE2(abstract_node)
  VISIT_NODE2(module_node)
  VISIT_NODE(symbol_node, node->name)
  VISIT_NODE2(expression_node)
  VISIT_NODE2(post_expression_node)
  VISIT_NODE2(statement_node)
  VISIT_NODE(class_node, node->name)
  VISIT_NODE(function_node, node->name)
  VISIT_NODE(var_node, node->name)
  VISIT_NODE2(parameter_node)
  VISIT_NODE2(property_node)
  VISIT_NODE(binary_operator_node, node->operation)
  VISIT_NODE(unary_operator_node, node->operation)
  VISIT_NODE(member_access_node, node->member_name)
  VISIT_NODE2(call_node)
  VISIT_NODE2(cast_node)
  VISIT_NODE2(index_node)
  VISIT_NODE(lambda_capture_node, node->capture_type);
  VISIT_NODE2(lambda_node);
  VISIT_NODE(literal_node, node->value)
  VISIT_NODE(name_reference_node, node->name)
  VISIT_NODE(goto_node, node->target_name)
  VISIT_NODE(label_node, node->name)
  VISIT_NODE2(return_node)
  VISIT_NODE(break_node, node->count)
  VISIT_NODE(continue_node, node->count)
  VISIT_NODE2(if_node)
  VISIT_NODE2(while_node)
  VISIT_NODE(for_node, node->loop_var_name)
  VISIT_NODE2(import_node)
  VISIT_NODE2(type_node)
  VISIT_NODE2(qualifier_node)
  VISIT_NODE2(attribute_node)
  VISIT_NODE2(scope_node)

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
