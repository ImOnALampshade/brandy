// -----------------------------------------------------------------------------
// Brandy parser
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef PARSER_H
#define PARSER_H

#include "ast_nodes.h"
#include "module.h"
#include "token.h"
#include <memory>
#include <stack>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  using std::unique_ptr;

  struct parser
  {
    parser();
    parser(module *m);

    unique_ptr<module_node> accept_module();

    unique_ptr<symbol_node> accpet_symbol();
    unique_ptr<statement_node> accept_statement();

    unique_ptr<attribute_node> accept_attributes();

    unique_ptr<function_node> accept_function();
    unique_ptr<class_node> accept_class();
    unique_ptr<property_node> accept_property();
    unique_ptr<label_node> accept_label();
    unique_ptr<parameter_node> accept_parameter();

    unique_ptr<return_node> accept_return();
    unique_ptr<break_node> accept_break();
    unique_ptr<goto_node> accept_goto();
    unique_ptr<if_node> accept_if();
    unique_ptr<while_node> accept_while();
    unique_ptr<for_node> accept_for();
    unique_ptr<import_node> accept_import();
    unique_ptr<meta_block_node> accept_meta_block();

    unique_ptr<type_node> accept_type();

    unique_ptr<scope_node> accept_scope();

    unique_ptr<expression_node> accept_expression();

    unique_ptr<expression_node> accept_post_expression();
    unique_ptr<member_access_node> accept_member_access();
    unique_ptr<call_node> accept_call();
    unique_ptr<cast_node> accept_cast();
    unique_ptr<index_node> accept_index();

    unique_ptr<expression_node> accept_value();
    unique_ptr<expression_node> accept_literal();
    unique_ptr<lambda_node> accept_lambda();
    unique_ptr<name_reference_node> accept_name_reference();

    unique_ptr<expression_node> accept_generic_expression(
      unique_ptr<expression_node>(parser::*leftRecurse)(),
      unique_ptr<expression_node>(parser::*rightRecurse)(),
      const token_types::type *operators);

    unique_ptr<expression_node> accept_generic_expression(
      unique_ptr<expression_node>(parser::*recurse)(),
      const token_types::type *operators);

    unique_ptr<expression_node> accept_unary_expression();
    unique_ptr<expression_node> accept_arrow_expression();
    unique_ptr<expression_node> accept_binary_expression();
    unique_ptr<expression_node> accept_assignment_expression();
    unique_ptr<expression_node> accept_pipe_expression();

    size_t next_non_whitespace();
    bool accept(token_types::type type);
    void expect(token_types::type type);
    void expect_semicolon();

    template<typename node_type>
    unique_ptr<node_type> create_node();

    const token &last_token();
    const token &current_token();
    const token &token_at(size_t i);
    size_t num_tokens();

    bool accept_indent();

    void push_current();
    void pop_current();

    void add_indent();
    void pop_indent();

    bool at_end_of_stream();

    size_t m_currentIndex;
    std::stack<size_t> m_currentStack;
    module *m_module;
    unsigned m_indent;
  };

  template<typename node_type>
  unique_ptr<node_type> parser::create_node()
  {
    auto node = std::make_unique<node_type>();
    node->firstToken = m_currentIndex;
    return node;
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif
