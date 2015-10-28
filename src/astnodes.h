// -----------------------------------------------------------------------------
// Brandy language AST nodes
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef AST_NODES_H
#define AST_NODES_H

#pragma once

#include "qualifiers.h"
#include "symbol.h"
#include "tokens.h"
#include <cassert>
#include <memory>
#include <vector>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------
  // Forward declarations

  struct abstract_node;

  struct module_node;

  struct symbol_node;
  struct expression_node;
  struct post_expression_node;
  struct statement_node;

  struct class_node;
  struct function_node;
  struct var_node;
  struct parameter_node;
  struct property_node;

  struct binary_operator_node;
  struct unary_operator_node;
  struct member_access_node;
  struct call_node;
  struct cast_node;
  struct index_node;
  struct literal_node;
  struct lambda_capture_node;
  struct lambda_node;
  struct name_reference_node;

  struct goto_node;
  struct label_node;
  struct return_node;
  struct break_node;
  struct continue_node;
  struct if_node;
  struct while_node;
  struct for_node;
  struct import_node;

  struct type_node;
  struct qualifier_node;
  struct attribute_node;
  struct scope_node;

  // ---------------------------------------------------------------------------

  class ast_visitor
  {
  public:
    enum visitor_result { resume, stop, replace, rewalk };

    virtual ~ast_visitor();

    virtual ast_visitor::visitor_result visit(abstract_node *node);
    virtual ast_visitor::visitor_result visit(module_node *node);
    virtual ast_visitor::visitor_result visit(symbol_node *node);
    virtual ast_visitor::visitor_result visit(expression_node *node);
    virtual ast_visitor::visitor_result visit(post_expression_node *node);
    virtual ast_visitor::visitor_result visit(statement_node *node);
    virtual ast_visitor::visitor_result visit(class_node *node);
    virtual ast_visitor::visitor_result visit(function_node *node);
    virtual ast_visitor::visitor_result visit(var_node *node);
    virtual ast_visitor::visitor_result visit(parameter_node *node);
    virtual ast_visitor::visitor_result visit(property_node *node);
    virtual ast_visitor::visitor_result visit(binary_operator_node *node);
    virtual ast_visitor::visitor_result visit(unary_operator_node *node);
    virtual ast_visitor::visitor_result visit(member_access_node *node);
    virtual ast_visitor::visitor_result visit(call_node *node);
    virtual ast_visitor::visitor_result visit(cast_node *node);
    virtual ast_visitor::visitor_result visit(index_node *node);
    virtual ast_visitor::visitor_result visit(literal_node *node);
    virtual ast_visitor::visitor_result visit(lambda_capture_node *node);
    virtual ast_visitor::visitor_result visit(lambda_node *node);
    virtual ast_visitor::visitor_result visit(name_reference_node *node);
    virtual ast_visitor::visitor_result visit(goto_node *node);
    virtual ast_visitor::visitor_result visit(label_node *node);
    virtual ast_visitor::visitor_result visit(return_node *node);
    virtual ast_visitor::visitor_result visit(break_node *node);
    virtual ast_visitor::visitor_result visit(continue_node *node);
    virtual ast_visitor::visitor_result visit(if_node *node);
    virtual ast_visitor::visitor_result visit(while_node *node);
    virtual ast_visitor::visitor_result visit(for_node *node);
    virtual ast_visitor::visitor_result visit(import_node *node);
    virtual ast_visitor::visitor_result visit(type_node *node);
    virtual ast_visitor::visitor_result visit(qualifier_node *node);
    virtual ast_visitor::visitor_result visit(attribute_node *node);
    virtual ast_visitor::visitor_result visit(scope_node *node);

    std::unique_ptr<abstract_node> replacement_node;
  };

  template<typename T>
  using unique_vector = std::vector<std::unique_ptr<T>>;
  using std::unique_ptr;

  // ---------------------------------------------------------------------------

  // Walks a node
  template<typename node_type>
  void walk_node(unique_ptr<node_type> &node, ast_visitor *visitor, bool visit = true)
  {
    if (visit)
    {
    start:
      auto result = node->internal_visit(visitor);

      switch (result)
      {
      case ast_visitor::replace:
      {
        auto replacement = visitor->replacement_node.release();
        node = unique_ptr<node_type>(static_cast<node_type *>(replacement));
      }
      // Fall through to visit the node with its new value
      case ast_visitor::rewalk:
        goto start;
      case ast_visitor::stop:
        return;
      }
    }

    node->internal_walk(visitor);
  }

  // Walks a node
  template<typename node_type>
  void walk_node(node_type *node, ast_visitor *visitor, bool visit = true)
  {
    if (visit)
    {
    start:
      auto result = node->internal_visit(visitor);
      assert(result != ast_visitor::replace); // Can't replace the node when taking it by pointer

      switch (result)
      {
      case ast_visitor::rewalk:
        goto start;
      case ast_visitor::stop:
        return;
      }
    }

    node->internal_walk(visitor);
  }

  // ---------------------------------------------------------------------------
  // Base node type

  struct abstract_node
  {
    std::vector<token>::const_iterator begin, end;
    virtual ~abstract_node();

    virtual ast_visitor::visitor_result internal_visit(ast_visitor *visitor) = 0;
    virtual void internal_walk(ast_visitor *visitor) = 0;
  };

  // ---------------------------------------------------------------------------
  // Module

  struct module_node : public abstract_node
  {
    unique_vector<symbol_node> members;
    unique_vector<statement_node> statements;
    symbol_table symbols;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  // ---------------------------------------------------------------------------
  // Basic types

  struct statement_node : public abstract_node
  {
    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct symbol_node : public statement_node
  {
    token name;
    unique_ptr<attribute_node> attributes;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct expression_node : public statement_node
  {
    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct post_expression_node : public expression_node
  {
    unique_ptr<expression_node> left;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  // ---------------------------------------------------------------------------
  // Value types

  struct class_node : public symbol_node
  {
    unique_vector<type_node> base_classes;
    unique_vector<symbol_node> members;
    symbol_table symbols;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct function_node : public symbol_node
  {
    unique_vector<parameter_node> parameters;
    unique_ptr<type_node> return_type;
    unique_ptr<scope_node> scope;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct var_node : public symbol_node
  {
    unique_ptr<type_node> type;
    unique_ptr<expression_node> expression;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct parameter_node : public var_node
  {
    unique_ptr<expression_node> default_value;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct property_node : public symbol_node
  {
    unique_ptr<type_node> type;
    unique_ptr<scope_node> getter;
    unique_ptr<scope_node> setter;
    unique_ptr<parameter_node> setter_value;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  // ---------------------------------------------------------------------------
  // Expressions

  struct binary_operator_node : public expression_node
  {
    unique_ptr<expression_node> left;
    unique_ptr<expression_node> right;
    token operation;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct unary_operator_node : public expression_node
  {
    unique_ptr<expression_node> expression;
    token operation;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct member_access_node : public post_expression_node
  {
    token member_name;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct call_node : public post_expression_node
  {
    unique_vector<expression_node> parameters;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct cast_node : public post_expression_node
  {
    unique_ptr<type_node> type;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct index_node : public post_expression_node
  {
    unique_ptr<expression_node> index;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct literal_node : public expression_node
  {
    token value;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct lambda_capture_node : public abstract_node
  {
    unique_ptr<name_reference_node> name;
    token capture_type;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct lambda_node : public expression_node
  {
    unique_vector<lambda_capture_node> captures;
    unique_vector<parameter_node> parameters;
    unique_ptr<type_node> return_type;
    unique_ptr<scope_node> scope;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct name_reference_node : public expression_node
  {
    token name;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  // ---------------------------------------------------------------------------
  // Statements

  struct goto_node : public statement_node
  {
    token target_name;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct label_node : public statement_node
  {
    token name;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct return_node : public statement_node
  {
    unique_ptr<expression_node> value;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct break_node : public statement_node
  {
    token count;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct continue_node : public statement_node
  {
    token count;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct if_node : public statement_node
  {
    unique_ptr<expression_node> condition;
    unique_ptr<if_node> else_clause;
    unique_ptr<scope_node> scope;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct while_node : public statement_node
  {
    unique_ptr<expression_node> condition;
    unique_ptr<scope_node> scope;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct for_node : public statement_node
  {
    token loop_var_name;
    unique_ptr<expression_node> loop_iterator;
    unique_ptr<expression_node> loop_start;
    unique_ptr<expression_node> loop_end;
    unique_ptr<expression_node> loop_increment;
    unique_ptr<expression_node> condition;
    unique_ptr<scope_node> scope;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct import_node : public statement_node
  {
    std::vector<token> name_path;
    token effective_name;
    bool is_meta;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  // ---------------------------------------------------------------------------
  // Others

  struct type_node : public abstract_node
  {
    std::vector<token> name;
    unique_vector<qualifier_node> qualifiers;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct qualifier_node : public abstract_node
  {
    qualifier_types::type qualifier;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct attribute_node : public abstract_node
  {
    unique_vector<expression_node> attributes;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  struct scope_node : public abstract_node
  {
    unique_vector<statement_node> statements;
    symbol_table symbols;

    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override;
    void internal_walk(ast_visitor *visitor) override;
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
