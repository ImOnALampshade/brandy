// -----------------------------------------------------------------------------
// Brandy syntax tree nodes
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef AST_NODES_H
#define AST_NODES_H

#pragma once

#include "ast_nodes_decl.h"
#include "token.h"
#include <memory>
#include <vector>
#include <assert.h>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  template<typename T>
  using unique_vec = std::vector<std::unique_ptr<T>>;
  using std::unique_ptr;

  // ---------------------------------------------------------------------------

  struct ast_visitor
  {
    enum visitor_result { resume, stop, replace, rewalk };

    virtual ~ast_visitor();

    virtual ast_visitor::visitor_result visit(abstract_node *node);
    virtual ast_visitor::visitor_result visit(module_node *node);
    virtual ast_visitor::visitor_result visit(statement_node *node);
    virtual ast_visitor::visitor_result visit(symbol_node *node);
    virtual ast_visitor::visitor_result visit(expression_node *node);
    virtual ast_visitor::visitor_result visit(meta_node *node);
    virtual ast_visitor::visitor_result visit(meta_block_node *node);
    virtual ast_visitor::visitor_result visit(attribute_node *node);
    virtual ast_visitor::visitor_result visit(return_node *node);
    virtual ast_visitor::visitor_result visit(break_node *node);
    virtual ast_visitor::visitor_result visit(goto_node *node);
    virtual ast_visitor::visitor_result visit(if_node *node);
    virtual ast_visitor::visitor_result visit(while_node *node);
    virtual ast_visitor::visitor_result visit(for_node *node);
    virtual ast_visitor::visitor_result visit(import_node *node);
    virtual ast_visitor::visitor_result visit(label_node *node);
    virtual ast_visitor::visitor_result visit(class_node *node);
    virtual ast_visitor::visitor_result visit(function_node *node);
    virtual ast_visitor::visitor_result visit(var_node *node);
    virtual ast_visitor::visitor_result visit(parameter_node *node);
    virtual ast_visitor::visitor_result visit(property_node *node);
    virtual ast_visitor::visitor_result visit(binary_operator_node *node);
    virtual ast_visitor::visitor_result visit(unary_operator_node *node);
    virtual ast_visitor::visitor_result visit(absolute_value_node *node);
    virtual ast_visitor::visitor_result visit(literal_node *node);
    virtual ast_visitor::visitor_result visit(tuple_literal_node *node);
    virtual ast_visitor::visitor_result visit(dict_literal_node *node);
    virtual ast_visitor::visitor_result visit(lambda_node *node);
    virtual ast_visitor::visitor_result visit(name_reference_node *node);
    virtual ast_visitor::visitor_result visit(post_expression_node *node);
    virtual ast_visitor::visitor_result visit(member_access_node *node);
    virtual ast_visitor::visitor_result visit(call_node *node);
    virtual ast_visitor::visitor_result visit(cast_node *node);
    virtual ast_visitor::visitor_result visit(index_node *node);
    virtual ast_visitor::visitor_result visit(type_node *node);
    virtual ast_visitor::visitor_result visit(scope_node *node);

    std::unique_ptr<abstract_node> replacement_node;
  };

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
      default:
        break;
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
      default:
        break;
      }
    }

    node->internal_walk(visitor);
  }

  // ---------------------------------------------------------------------------

  struct abstract_node
  {
    size_t firstToken, lastToken;
    abstract_node *parent;

    virtual ~abstract_node() { }

    virtual ast_visitor::visitor_result internal_visit(ast_visitor *visitor);
    virtual void internal_walk(ast_visitor *visitor);
  };

  #define AST_NODE_METHODS                                                     \
    ast_visitor::visitor_result internal_visit(ast_visitor *visitor) override; \
    void internal_walk(ast_visitor *visitor) override;

  // ---------------------------------------------------------------------------

  struct module_node : abstract_node
  {
    unique_vec<symbol_node> symbols;
    unique_vec<statement_node> statements;

    AST_NODE_METHODS
  };

  // ---------------------------------------------------------------------------

  struct statement_node : abstract_node
  {
    AST_NODE_METHODS
  };

  struct symbol_node : statement_node
  {
    token name;
    token doc;
    unique_ptr<attribute_node> attributes;

    AST_NODE_METHODS
  };

  struct expression_node : statement_node
  {
    // type

    AST_NODE_METHODS
  };

  // ---------------------------------------------------------------------------

  struct meta_node : statement_node
  {
    AST_NODE_METHODS
  };

  struct meta_block_node : meta_node
  {
    unique_ptr<scope_node> inner_scope;

    AST_NODE_METHODS
  };

  struct attribute_node : meta_node
  {
    unique_vec<expression_node> attributes;

    AST_NODE_METHODS
  };

  // ---------------------------------------------------------------------------

  struct return_node : statement_node
  {
    unique_ptr<expression_node> return_expression;

    AST_NODE_METHODS
  };

  struct break_node : statement_node
  {
    token break_to_label;
    token break_type;

    AST_NODE_METHODS
  };

  struct goto_node : statement_node
  {
    token target;

    AST_NODE_METHODS
  };

  struct if_node : statement_node
  {
    unique_ptr<expression_node> condition;
    unique_ptr<if_node> elif_clause;
    unique_ptr<scope_node> inner_scope;

    AST_NODE_METHODS
  };

  struct while_node : statement_node
  {
    unique_ptr<expression_node> condition;
    unique_ptr<scope_node> inner_scope;

    AST_NODE_METHODS;
  };

  struct for_node : statement_node
  {
    std::vector<token> names;
    unique_ptr<expression_node> expression;
    unique_ptr<scope_node> inner_scope;

    AST_NODE_METHODS
  };

  struct import_node : symbol_node
  {
    token path;

    AST_NODE_METHODS
  };

  // ---------------------------------------------------------------------------

  struct label_node : symbol_node
  {
    AST_NODE_METHODS
  };

  // ---------------------------------------------------------------------------

  struct class_node : symbol_node
  {
    unique_vec<type_node> base_classes;
    unique_vec<symbol_node> members;

    AST_NODE_METHODS
  };

  struct function_node : symbol_node
  {
    unique_vec<parameter_node> parameters;
    unique_ptr<type_node> return_type;
    unique_ptr<scope_node> inner_scope;
    bool is_method;

    AST_NODE_METHODS
  };

  struct var_node : symbol_node
  {
    unique_ptr<type_node> type;
    unique_ptr<expression_node> initial_value;

    AST_NODE_METHODS
  };

  struct parameter_node : var_node
  {
    AST_NODE_METHODS
  };

  struct property_node : symbol_node
  {
    unique_ptr<type_node> type;
    unique_ptr<scope_node> getter;
    unique_ptr<scope_node> setter;

    AST_NODE_METHODS
  };

  // ---------------------------------------------------------------------------

  struct binary_operator_node : expression_node
  {
    unique_ptr<expression_node> left;
    unique_ptr<expression_node> right;
    token operation;

    AST_NODE_METHODS
  };

  struct unary_operator_node : expression_node
  {
    unique_ptr<expression_node> expression;
    token operation;

    AST_NODE_METHODS
  };

  struct absolute_value_node : expression_node
  {
    unique_ptr<expression_node> expression;

    AST_NODE_METHODS
  };

  struct literal_node : expression_node
  {
    token value;

    AST_NODE_METHODS
  };

  struct tuple_literal_node : expression_node
  {
    unique_vec<expression_node> values;

    AST_NODE_METHODS
  };

  struct dict_literal_node : expression_node
  {
    unique_vec<expression_node> keys;
    unique_vec<expression_node> values;

    AST_NODE_METHODS
  };

  struct lambda_node : expression_node
  {
    unique_vec<parameter_node> parameters;
    unique_ptr<type_node> return_type;
    unique_ptr<scope_node> inner_scope;

    AST_NODE_METHODS
  };

  struct name_reference_node : expression_node
  {
    token name;

    AST_NODE_METHODS
  };

  // ---------------------------------------------------------------------------

  struct post_expression_node : expression_node
  {
    unique_ptr<expression_node> expression;

    AST_NODE_METHODS
  };

  struct member_access_node : post_expression_node
  {
    token member_name;

    AST_NODE_METHODS
  };

  struct call_node : post_expression_node
  {
    unique_vec<expression_node> arguments;

    AST_NODE_METHODS
  };

  struct cast_node : post_expression_node
  {
    unique_ptr<type_node> cast_to;

    AST_NODE_METHODS
  };

  struct index_node : post_expression_node
  {
    unique_ptr<expression_node> index;

    AST_NODE_METHODS
  };

  // ---------------------------------------------------------------------------

  struct type_node : abstract_node
  {
    std::vector<token> name;

    AST_NODE_METHODS
  };

  struct scope_node : abstract_node
  {
    unique_vec<statement_node> statements;

    AST_NODE_METHODS
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
