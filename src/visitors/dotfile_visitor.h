// -----------------------------------------------------------------------------
// Brandy ast visitor to generate AST dotfile
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef DOTFILE_VISITOR_H
#define DOTFILE_VISITOR_H

#pragma once

#include "../ast_nodes.h"
#include <stdio.h>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  struct dotfile_visitor : ast_visitor
  {
    dotfile_visitor(const char *fileLocation);
    ~dotfile_visitor();

    ast_visitor::visitor_result visit(abstract_node *node) override;
    ast_visitor::visitor_result visit(module_node *node) override;
    ast_visitor::visitor_result visit(class_node *node) override;
    ast_visitor::visitor_result visit(function_node *node) override;
    ast_visitor::visitor_result visit(parameter_node *node) override;
    ast_visitor::visitor_result visit(property_node *node) override;
    ast_visitor::visitor_result visit(var_node *node) override;
    ast_visitor::visitor_result visit(binary_operator_node *node) override;
    ast_visitor::visitor_result visit(unary_operator_node *node) override;
    ast_visitor::visitor_result visit(absolute_value_node *node) override;
    ast_visitor::visitor_result visit(member_access_node *node) override;
    ast_visitor::visitor_result visit(call_node *node) override;
    ast_visitor::visitor_result visit(cast_node *node) override;
    ast_visitor::visitor_result visit(index_node *node) override;
    ast_visitor::visitor_result visit(literal_node *node) override;
    ast_visitor::visitor_result visit(lambda_node *node) override;
    ast_visitor::visitor_result visit(name_reference_node *node) override;
    ast_visitor::visitor_result visit(goto_node *node) override;
    ast_visitor::visitor_result visit(label_node *node) override;
    ast_visitor::visitor_result visit(return_node *node) override;
    ast_visitor::visitor_result visit(break_node *node) override;
    ast_visitor::visitor_result visit(if_node *node) override;
    ast_visitor::visitor_result visit(while_node *node) override;
    ast_visitor::visitor_result visit(for_node *node) override;
    ast_visitor::visitor_result visit(import_node *node) override;
    ast_visitor::visitor_result visit(meta_block_node *node) override;
    ast_visitor::visitor_result visit(attribute_node *node) override;
    ast_visitor::visitor_result visit(type_node *node) override;
    ast_visitor::visitor_result visit(scope_node *node) override;

    void format_node(abstract_node *node, const char *name);
    void format_node(abstract_node *node, const char *name, const char *str);
    void format_node(abstract_node *node, const char *name, const token &tok);
    void format_node(abstract_node *node, const char *name, const std::vector<token> &tok, char delimiter = '.');

    FILE *m_file;
    abstract_node *m_connectTo;
    size_t m_numconnected;
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
