// -----------------------------------------------------------------------------
// AST visitor for making a dotfile
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef AST_DOTFILE_VISITOR_H
#define AST_DOTFILE_VISITOR_H

#pragma once

#include "astnodes.h"
#include <string>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  class dotfile_visitor : public ast_visitor
  {
  public:
    dotfile_visitor();
    ~dotfile_visitor();

    virtual ast_visitor::visitor_result visit(abstract_node *node) override;
    virtual ast_visitor::visitor_result visit(module_node *node) override;
    virtual ast_visitor::visitor_result visit(symbol_node *node) override;
    virtual ast_visitor::visitor_result visit(expression_node *node) override;
    virtual ast_visitor::visitor_result visit(post_expression_node *node) override;
    virtual ast_visitor::visitor_result visit(statement_node *node) override;
    virtual ast_visitor::visitor_result visit(class_node *node) override;
    virtual ast_visitor::visitor_result visit(function_node *node) override;
    virtual ast_visitor::visitor_result visit(var_node *node) override;
    virtual ast_visitor::visitor_result visit(parameter_node *node) override;
    virtual ast_visitor::visitor_result visit(property_node *node) override;
    virtual ast_visitor::visitor_result visit(binary_operator_node *node) override;
    virtual ast_visitor::visitor_result visit(unary_operator_node *node) override;
    virtual ast_visitor::visitor_result visit(member_access_node *node) override;
    virtual ast_visitor::visitor_result visit(call_node *node) override;
    virtual ast_visitor::visitor_result visit(cast_node *node) override;
    virtual ast_visitor::visitor_result visit(index_node *node) override;
    virtual ast_visitor::visitor_result visit(literal_node *node) override;
    virtual ast_visitor::visitor_result visit(lambda_capture_node *node) override;
    virtual ast_visitor::visitor_result visit(lambda_node *node) override;
    virtual ast_visitor::visitor_result visit(name_reference_node *node) override;
    virtual ast_visitor::visitor_result visit(goto_node *node) override;
    virtual ast_visitor::visitor_result visit(label_node *node) override;
    virtual ast_visitor::visitor_result visit(return_node *node) override;
    virtual ast_visitor::visitor_result visit(break_node *node) override;
    virtual ast_visitor::visitor_result visit(continue_node *node) override;
    virtual ast_visitor::visitor_result visit(if_node *node) override;
    virtual ast_visitor::visitor_result visit(while_node *node) override;
    virtual ast_visitor::visitor_result visit(for_node *node) override;
    virtual ast_visitor::visitor_result visit(import_node *node) override;
    virtual ast_visitor::visitor_result visit(typedef_node *node) override;
    virtual ast_visitor::visitor_result visit(type_node *node) override;
    virtual ast_visitor::visitor_result visit(tuple_node *);
    virtual ast_visitor::visitor_result visit(delegate_node *);
    virtual ast_visitor::visitor_result visit(plain_type_node *);
    virtual ast_visitor::visitor_result visit(decltype_node *);
    virtual ast_visitor::visitor_result visit(post_type_node *node) override;
    virtual ast_visitor::visitor_result visit(type_indirect_node *node) override;
    virtual ast_visitor::visitor_result visit(type_array_node *node) override;
    virtual ast_visitor::visitor_result visit(type_template_node *node) override;
    virtual ast_visitor::visitor_result visit(qualifier_node *node) override;
    virtual ast_visitor::visitor_result visit(attribute_node *node) override;
    virtual ast_visitor::visitor_result visit(scope_node *node) override;

  private:
    void format_node(abstract_node *node, const char *name);
    void format_node(abstract_node *node, const char *name, const token &tok);
    void format_node(abstract_node *node, const char *name, const std::vector<token> &tok);
    
    FILE *m_file;
    abstract_node *m_connectTo;
    size_t m_numconnected;
  };


  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
