// -----------------------------------------------------------------------------
// Brandy language tokens
// Howard Hughes
// -----------------------------------------------------------------------------

#ifndef PARSER_H
#define PARSER_H

#pragma once

#include "astnodes.h"
#include "tokens.h"
#include <stack>
#include <vector>

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  class parsing_error
  {
  public:
    parsing_error(const char *error, std::vector<token>::const_iterator position);

    const char *error_str() const;
    std::vector<token>::const_iterator position() const;
    
  private:
    const char *m_errStr;
    std::vector<token>::const_iterator m_pos;
  };

  // ---------------------------------------------------------------------------

  class parser
  {
  public:
    parser();
    parser(const std::vector<token> &tokens);
    parser(std::vector<token> &&tokens);
    
    bool at_end_of_stream();

    unique_ptr<module_node> parse_module();

    const std::vector<token> &tokens() const;

  private:
    unique_ptr<module_node> accept_module();

    unique_ptr<symbol_node> accept_symbol();
    unique_ptr<expression_node> accept_expression();
    unique_ptr<expression_node> accept_post_expression();
    unique_ptr<statement_node> accept_statement();

    unique_ptr<class_node> accept_class();
    unique_ptr<function_node> accept_function();
    unique_ptr<var_node> accept_var();
    unique_ptr<parameter_node> accept_parameter();
    unique_ptr<property_node> accept_property();

    unique_ptr<member_access_node> accept_member_access();
    unique_ptr<call_node> accept_call();
    unique_ptr<cast_node> accept_cast();
    unique_ptr<index_node> accept_index();

    unique_ptr<literal_node> accept_literal();
    unique_ptr<lambda_capture_node> accept_lambda_capture();
    unique_ptr<lambda_node> accept_lambda();
    unique_ptr<name_reference_node> accept_name_reference();
    unique_ptr<expression_node> accept_value();

    unique_ptr<expression_node> accept_generic_expression(
      unique_ptr<expression_node>(parser::*leftRecurse)(), 
      unique_ptr<expression_node>(parser::*rightRecurse)(),
      const token_types::type *operators);

    unique_ptr<expression_node> accept_generic_expression(
      unique_ptr<expression_node>(parser::*recurse)(),
      const token_types::type *operators);

    unique_ptr<expression_node> accept_unary_operator();
    unique_ptr<expression_node> accept_multiplication();
    unique_ptr<expression_node> accept_addition();
    unique_ptr<expression_node> accept_bitwise_shift();
    unique_ptr<expression_node> accept_comparison();
    unique_ptr<expression_node> accept_equality();
    unique_ptr<expression_node> accept_bitwise_conjunction();
    unique_ptr<expression_node> accept_logical_conjunction();
    unique_ptr<expression_node> accept_assignment();

    unique_ptr<statement_node> accept_delimited_statement();
    unique_ptr<statement_node> accept_goto();
    unique_ptr<label_node> accept_label();
    unique_ptr<statement_node> accept_return();
    unique_ptr<statement_node> accept_break();
    unique_ptr<statement_node> accept_continue();
    
    unique_ptr<statement_node> accept_free_statement();
    unique_ptr<if_node> accept_if();
    unique_ptr<if_node> accept_else();
    unique_ptr<while_node> accept_while();
    unique_ptr<for_node> accept_for();
    unique_ptr<import_node> accept_import();
    unique_ptr<typedef_node> accept_typedef();

    unique_ptr<type_node> accept_type();
    unique_ptr<post_type_node> accept_post_type_node();

    unique_ptr<qualifier_node> accept_qualifier();
    unique_ptr<attribute_node> accept_attribute();
    unique_ptr<scope_node> accept_scope();

    bool accept(token_types::type type);
    void expect(token_types::type type);

    // Special case for expecting a semicolon - also looks for newlines
    void expect_semicolon();

    bool allows_for_noparen_call();

    void disallow_skip_newlines();
    void allow_skip_newlines();
    void pop_skip_newlines();

    const token &last_token();
    const token &current_token();

    template<typename node_type>
    typename std::unique_ptr<node_type> create_node()
    {
      auto node = make_unique<node_type>();
      node->begin = m_current;
      return node;
    }

    class newline_gaurd
    {
    public:
      newline_gaurd(parser *p) : m_parser(p) { m_parser->disallow_skip_newlines(); }
      ~newline_gaurd() { m_parser->pop_skip_newlines(); }
    private:
      parser *m_parser;
    };

    const std::vector<token> m_tokens;
    std::vector<token>::const_iterator m_current;
    //unsigned m_disallowNewlines;
    std::stack<bool> m_disallowNewLines;
    size_t m_lastLineNum;
  };

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

#endif