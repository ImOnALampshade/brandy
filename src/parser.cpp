// -----------------------------------------------------------------------------
// Brandy parser
// Howard Hughes
// -----------------------------------------------------------------------------

#include "parser.h"
#include "error_base.h"
#include <assert.h>
#include <iostream>

// -----------------------------------------------------------------------------

#define ENTER_RULE(name)          \
  do {                            \
   push_current();                \
   m_ruleStack.push_back(#name);  \
  } while(false)

#define REJECT_RULE()       \
  do {                      \
    pop_current();          \
    m_ruleStack.pop_back(); \
    return nullptr;         \
  } while(false)

#define REJECT_RULE_ERROR(msg)           \
  do {                                   \
    size_t saveIndex = m_currentIndex;   \
    pop_current();                       \
    m_ruleStack.clear();                 \
    throw parser_error(msg, saveIndex);  \
  } while(false)

#define ACCEPT_RULE(retval)  \
  do {                       \
    m_currentStack.pop();    \
    m_ruleStack.pop_back();  \
    return retval;           \
  } while(false)

#define DISALLOW_SKIP_NEWLINES()  \
  do {                            \
    m_allowNewlines.push(false);  \
  } while(false)
#define ALLOW_SKIP_NEWLINES()    \
  do {                           \
    m_allowNewlines.push(true);  \
  } while(false)
#define POP_SKIP_NEWLINES()  \
  do {                       \
    m_allowNewlines.pop();   \
  } while(false)

#define INDENT_GAURD() indent_gaurd __definedIndentGaurd(this)
#define NEWLINE_GAURD(value) newline_gaurd __definedNewlineGaurd(this, value)

namespace brandy
{
  namespace
  {
    const size_t expected_indent_size = 2;

    struct indent_gaurd
    {
      indent_gaurd(parser *p) :
        m_parser(p)
      {
        m_parser->add_indent();
      }

      ~indent_gaurd()
      {
        m_parser->pop_indent();
      }

      parser *m_parser;
    };

    struct newline_gaurd
    {
      newline_gaurd(parser *p, bool value) :
        m_parser(p)
      {
        m_parser->m_allowNewlines.push(value);
      }

      ~newline_gaurd()
      {
        m_parser->m_allowNewlines.pop();
      }

      parser *m_parser;
    };
  }

  struct parser_error : error_base
  {
    parser_error(const char *msg, size_t tokenIndex) :
      error_base(tokenIndex),
      m_msg(msg)
    {
    }

    const char *m_msg;

    std::string error_message()
    {
      return m_msg;
    }

    const char *severity() { return "error"; }
  };

  // ---------------------------------------------------------------------------

  parser::parser() :
    m_currentIndex(0),
    m_module(nullptr),
    m_indent(0)
  {
    m_allowNewlines.push(true);
  }

  parser::parser(module *m) :
    m_currentIndex(0),
    m_module(m),
    m_indent(0)
  {
    m_allowNewlines.push(true);
  }

  // ---------------------------------------------------------------------------

  unique_ptr<module_node> parser::accept_module()
  {
    ENTER_RULE(module);

    auto moduleNode = create_node<module_node>();

    while (!at_end_of_stream())
    {
      if (auto symbol = accpet_symbol())
        moduleNode->symbols.emplace_back(std::move(symbol));

      else if (auto statement = accept_statement())
        moduleNode->statements.emplace_back(std::move(statement));

      else
      {
        REJECT_RULE_ERROR("Parser stuck!");
      }
    }

    ACCEPT_RULE(moduleNode);
  }

  // ---------------------------------------------------------------------------

  unique_ptr<symbol_node> parser::accpet_symbol()
  {
    ENTER_RULE(symbol);

    if (!accept_indent())
      REJECT_RULE();

    auto attributes = accept_attributes();

    if (auto functionNode = accept_function())
    {
      functionNode->attributes = std::move(attributes);
      ACCEPT_RULE(functionNode);
    }
    else if (auto classNode = accept_class())
    {
      classNode->attributes = std::move(attributes);
      ACCEPT_RULE(classNode);
    }
    else if (auto propertyNode = accept_property())
    {
      propertyNode->attributes = std::move(attributes);
      ACCEPT_RULE(propertyNode);
    }
    else if (attributes)
    {
      // TODO: Nice error message here? Maybe get edit distance from expected
      // word? We know they're missing something like property, or variable, etc
      // if (current_token().type() == token_types::IDENTIFIER)
      //   friendly_error();

      REJECT_RULE_ERROR("attributes missing a symbol");
    }
    else
      REJECT_RULE();
  }

  unique_ptr<statement_node> parser::accept_statement()
  {
    ENTER_RULE(statement);
    NEWLINE_GAURD(true);

    if (!accept_indent())
      REJECT_RULE();

    if (auto labelNode = accept_label())
      ACCEPT_RULE(labelNode);
    else if (auto ifNode = accept_if())
      ACCEPT_RULE(ifNode);
    else if (auto whileNode = accept_while())
      ACCEPT_RULE(whileNode);
    else if (auto forNode = accept_for())
      ACCEPT_RULE(forNode);
    else if (auto importNode = accept_import())
      ACCEPT_RULE(importNode);
    else if (auto metaNode = accept_meta_block())
      ACCEPT_RULE(metaNode);
    else if (auto expNode = accept_expression())
    {
      expect_semicolon();
      ACCEPT_RULE(expNode);
    }
    else if (auto returnNode = accept_return())
    {
      expect_semicolon();
      ACCEPT_RULE(returnNode);
    }
    else if (auto breakNode = accept_break())
    {
      expect_semicolon();
      ACCEPT_RULE(breakNode);
    }
    else if (auto gotoNode = accept_goto())
    {
      expect_semicolon();
      ACCEPT_RULE(gotoNode);
    }
    else
      REJECT_RULE();
  }

  // ---------------------------------------------------------------------------

  unique_ptr<attribute_node> parser::accept_attributes()
  {
    ENTER_RULE(attribute);

    if (!accept(token_types::ATTRIBUTE_START))
      REJECT_RULE();

    auto attrNode = create_node<attribute_node>();

    if (accept(token_types::CLOSE_SQUARE))
      ACCEPT_RULE(attrNode);

    do
    {
      auto expNode = accept_expression();

      if (expNode)
        attrNode->attributes.emplace_back(std::move(expNode));
      else
        REJECT_RULE_ERROR("expected expression in attribute list");
    }
    while (accept(token_types::COMMA));

    ACCEPT_RULE(attrNode);
  }

  // ---------------------------------------------------------------------------

  unique_ptr<function_node> parser::accept_function()
  {
    ENTER_RULE(function);

    if (accept(token_types::FUNCTION) || accept(token_types::METHOD))
    {
      auto functionNode = create_node<function_node>();
      functionNode->is_method = (last_token().type() == token_types::METHOD);

      expect(token_types::IDENTIFIER);
      functionNode->name = last_token();

      if (accept(token_types::OPEN_PAREN))
      {
        do
        {
          auto param = accept_parameter();

          if (param)
            functionNode->parameters.push_back(std::move(param));
          else
            REJECT_RULE_ERROR("Expected parameter");
        } while(accept(token_types::COMMA));

        expect(token_types::CLOSE_PAREN);
      }

      functionNode->return_type = accept_type();

      if (accept(token_types::SINGLE_RIGHT_ARROW))
      {
        if (accept(token_types::DOCUMENTION_BLOCK))
          functionNode->doc = last_token();

        functionNode->inner_scope = accept_scope();
        ACCEPT_RULE(functionNode);
      }

      else if (accept(token_types::SINGLE_RIGHT_FAT_ARROW))
      {
        if (accept(token_types::DOCUMENTION_BLOCK))
          functionNode->doc = last_token();

        functionNode->inner_scope = create_node<scope_node>();
        auto returnNode = create_node<return_node>();
        returnNode->return_expression = accept_expression();
        functionNode->inner_scope->statements.push_back(std::move(returnNode));

        ACCEPT_RULE(functionNode);
      }
      else
        REJECT_RULE_ERROR("Expected an arrow (-> or =>) after function declaration");
    }

    REJECT_RULE();
  }

  unique_ptr<class_node> parser::accept_class()
  {
    ENTER_RULE(class);

    if (accept(token_types::CLASS))
    {
      INDENT_GAURD();

      auto classNode = create_node<class_node>();
      expect(token_types::IDENTIFIER);
      classNode->name = last_token();

      if (accept(token_types::EXTENDS))
        do
        {
          auto type = accept_type();
          if (!type)
            REJECT_RULE_ERROR("Expected a base class");
          classNode->base_classes.push_back(std::move(type));
        } while(accept(token_types::COMMA));

      if (accept(token_types::DOCUMENTION_BLOCK))
        classNode->doc = last_token();

      while (auto symbol = accpet_symbol())
        classNode->members.push_back(std::move(symbol));

      ACCEPT_RULE(classNode);
    }

    REJECT_RULE();
  }

  unique_ptr<property_node> parser::accept_property()
  {
    ENTER_RULE(property);

    if (accept(token_types::PROPERTY))
    {
      INDENT_GAURD();

      auto propertyNode = create_node<property_node>();
      expect(token_types::IDENTIFIER);
      propertyNode->name = last_token();

      if (accept(token_types::COLON))
      {
        propertyNode->type = accept_type();
        if (!propertyNode->type)
          REJECT_RULE_ERROR(
            "Expected type name following colon in property definition"
          );
      }

      if (accept(token_types::DOCUMENTION_BLOCK))
        propertyNode->doc = last_token();

      bool gotSet = false;

      if (accept(token_types::SET))
      {
        gotSet = true;

        if (accept(token_types::SINGLE_RIGHT_ARROW))
          propertyNode->setter = accept_scope();
        else if (accept(token_types::SINGLE_RIGHT_FAT_ARROW))
          REJECT_RULE_ERROR(
            "You can't use => for a property setter definition, use -> instead"
          );
        else
          REJECT_RULE_ERROR(
            "Expected a -> after 'set' in property definition"
          );
      }

      if (accept(token_types::GET))
      {
        if (accept(token_types::SINGLE_RIGHT_ARROW))
          propertyNode->getter = accept_scope();

        else if (accept(token_types::SINGLE_RIGHT_FAT_ARROW))
        {
          propertyNode->getter = create_node<scope_node>();
          auto returnNode = create_node<return_node>();
          returnNode->return_expression = accept_expression();
          if (!returnNode->return_expression)
            REJECT_RULE_ERROR(
              "Expected expression following => in property getter"
            );

          propertyNode->getter->statements.push_back(std::move(returnNode));
        }
        else
          REJECT_RULE_ERROR(
            "Expecting a -> or => after 'get' in property definition"
          );
      }

      if (!gotSet && accept(token_types::SET))
      {
        if (accept(token_types::SINGLE_RIGHT_ARROW))
          propertyNode->setter = accept_scope();
        else if (accept(token_types::SINGLE_RIGHT_FAT_ARROW))
          REJECT_RULE_ERROR(
            "You can't use => for a property setter definition, use -> instead"
          );
        else
          REJECT_RULE_ERROR(
            "Expected a -> after 'set' in property definition"
          );
      }

      ACCEPT_RULE(propertyNode);
    }

    REJECT_RULE();
  }

  unique_ptr<label_node> parser::accept_label()
  {
    ENTER_RULE(label);

    if (accept(token_types::COLON) && accept(token_types::IDENTIFIER))
    {
      auto labelNode = create_node<label_node>();
      labelNode->name = last_token();
      ACCEPT_RULE(labelNode);
    }

    REJECT_RULE();
  }

  unique_ptr<parameter_node> parser::accept_parameter()
  {
    ENTER_RULE(parameter);

    if (accept(token_types::IDENTIFIER))
    {
      auto parameterNode = create_node<parameter_node>();
      accept(token_types::IDENTIFIER);
      parameterNode->name = last_token();

      expect(token_types::COLON);

      parameterNode->type = accept_type();
      if (!parameterNode->type)
        REJECT_RULE_ERROR("Expected a type following : in parameter");

      if (accept(token_types::ASSIGNMENT))
      {
        parameterNode->initial_value = accept_expression();

        if (!parameterNode->initial_value)
          REJECT_RULE_ERROR(
            "Expected a default value following = in parameter"
          );
      }

      ACCEPT_RULE(parameterNode);
    }

    REJECT_RULE();
  }

  // ---------------------------------------------------------------------------

  unique_ptr<return_node> parser::accept_return()
  {
    ENTER_RULE(return);

    if (accept(token_types::RETURN))
    {
      auto returnNode = create_node<return_node>();
      returnNode->return_expression = accept_expression();

      ACCEPT_RULE(returnNode);
    }

    REJECT_RULE();
  }

  unique_ptr<break_node> parser::accept_break()
  {
    ENTER_RULE(break);

    if (accept(token_types::BREAK) || accept(token_types::CONTINUE))
    {
      auto breakNode = create_node<break_node>();
      breakNode->break_type = last_token();
      if (accept(token_types::IDENTIFIER))
        breakNode->break_to_label = last_token();

      ACCEPT_RULE(breakNode);
    }

    REJECT_RULE();
  }

  unique_ptr<goto_node> parser::accept_goto()
  {
    ENTER_RULE(goto);

    if (accept(token_types::GOTO))
    {
      expect(token_types::IDENTIFIER);

      auto gotoNode = create_node<goto_node>();
      gotoNode->target = last_token();
      ACCEPT_RULE(gotoNode);
    }

    REJECT_RULE();
  }

  unique_ptr<if_node> parser::accept_if()
  {
    ENTER_RULE(if);

    if (accept(token_types::IF))
    {
      auto ifNode = create_node<if_node>();
      ifNode->condition = accept_expression();

      if (!ifNode->condition)
        REJECT_RULE_ERROR("Expected an expression following if");

      ifNode->inner_scope = accept_scope();

      auto curr = ifNode.get();

      while (accept(token_types::ELIF))
      {
        curr->elif_clause = create_node<if_node>();
        curr->elif_clause->condition = accept_expression();

        if (!curr->elif_clause->condition)
          REJECT_RULE_ERROR("Expected an expression following elif");

        curr->elif_clause->inner_scope = accept_scope();
        curr = curr->elif_clause.get();
      }

      if (accept(token_types::ELSE))
      {
        curr->elif_clause = create_node<if_node>();
        curr->elif_clause->inner_scope = accept_scope();
      }

      ACCEPT_RULE(ifNode);
    }

    REJECT_RULE();
  }

  unique_ptr<while_node> parser::accept_while()
  {
    ENTER_RULE(while);

    if (accept(token_types::WHILE))
    {
      auto whileNode = create_node<while_node>();
      whileNode->condition = accept_expression();

      if (!whileNode->condition)
        REJECT_RULE_ERROR("Expected an expression following while");

      whileNode->inner_scope = accept_scope();

      ACCEPT_RULE(whileNode);
    }

    REJECT_RULE();
  }

  unique_ptr<for_node> parser::accept_for()
  {
    ENTER_RULE(for);

    if (accept(token_types::FOR))
    {
      auto forNode = create_node<for_node>();

      do
      {
        expect(token_types::IDENTIFIER);
        forNode->names.push_back(last_token());
      } while(accept(token_types::COMMA));

      expect(token_types::IN);

      forNode->expression = accept_expression();

      if (!forNode->expression)
        REJECT_RULE_ERROR("No expression following \"in\" in for loop");

      forNode->inner_scope = accept_scope();

      ACCEPT_RULE(forNode);
    }

    REJECT_RULE();
  }

  unique_ptr<import_node> parser::accept_import()
  {
    ENTER_RULE(import);

    if (accept(token_types::IMPORT))
    {
      auto importNode = create_node<import_node>();

      expect(token_types::_STRING);

      importNode->path = last_token();

      if (accept(token_types::AS))
      {
        expect(token_types::IDENTIFIER);
        importNode->name = last_token();
      }

      ACCEPT_RULE(importNode);
    }

    REJECT_RULE();
  }

  unique_ptr<meta_block_node> parser::accept_meta_block()
  {
    ENTER_RULE(meta);

    if (accept(token_types::META_START))
    {
      auto metaBlockNode = create_node<meta_block_node>();
      metaBlockNode->inner_scope = accept_scope();
      ACCEPT_RULE(metaBlockNode);
    }

    REJECT_RULE();
  }

  // ---------------------------------------------------------------------------

  unique_ptr<type_node> parser::accept_type()
  {
    ENTER_RULE(type);

    if (accept(token_types::IDENTIFIER))
    {
      auto typeNode = create_node<type_node>();
      typeNode->name.push_back(last_token());

      while (accept(token_types::DOT))
      {
        expect(token_types::IDENTIFIER);
        typeNode->name.push_back(last_token());
      }

      ACCEPT_RULE(typeNode);
    }

    REJECT_RULE();
  }

  // ---------------------------------------------------------------------------

  unique_ptr<scope_node> parser::accept_scope()
  {
    ENTER_RULE(scope);
    INDENT_GAURD();

    auto scopeNode = create_node<scope_node>();

    while (auto statement = accept_statement())
    {
      scopeNode->statements.push_back(std::move(statement));
    }

    ACCEPT_RULE(scopeNode);
  }

  // ---------------------------------------------------------------------------

  unique_ptr<expression_node> parser::accept_expression()
  {
    ENTER_RULE(expression);

    if (auto exp = accept_assignment())
      ACCEPT_RULE(exp);

    REJECT_RULE();
  }

  unique_ptr<expression_node> parser::accept_post_expression()
  {
    ENTER_RULE(post expression);

    unique_ptr<expression_node> leftNode = accept_value();
    if (!leftNode) REJECT_RULE();

    for (;;)
    {
      if (auto memberNode = accept_member_access())
      {
        memberNode->expression = std::move(leftNode);
        leftNode = std::move(memberNode);
      }
      else if (auto callNode = accept_call())
      {
        callNode->expression = std::move(leftNode);
        leftNode = std::move(callNode);
      }
      else if (auto castNode = accept_cast())
      {
        castNode->expression = std::move(leftNode);
        leftNode = std::move(castNode);
      }
      else if (auto indexNode = accept_index())
      {
        indexNode->expression = std::move(leftNode);
        leftNode = std::move(indexNode);
      }
      else
      {
        ACCEPT_RULE(leftNode);
        /*
        DISALLOW_SKIP_NEWLINES();
        if (accept(token_types::INCREMENT) || accept(token_types::DECREMENT))
        {
          POP_SKIP_NEWLINES();

          auto unaryNode = create_node<unary_operator_node>();
          unaryNode->operation = last_token();
          unaryNode->expression = std::move(leftNode);
          unaryNode->is_post_expression = true;

          leftNode = std::move(unaryNode);
        }
        else
        {
          POP_SKIP_NEWLINES();
          ACCEPT_RULE(leftNode);
        }
        */
      }
    }
  }

  unique_ptr<member_access_node> parser::accept_member_access()
  {
    ENTER_RULE(member_access);

    if (accept(token_types::DOT))
    {
      auto memberAccesNode = create_node<member_access_node>();
      expect(token_types::IDENTIFIER);
      memberAccesNode->member_name = last_token();

      ACCEPT_RULE(memberAccesNode);
    }

    REJECT_RULE();
  }

  unique_ptr<call_node> parser::accept_call()
  {
    ENTER_RULE(call);

    DISALLOW_SKIP_NEWLINES();

    if (accept(token_types::OPEN_PAREN))
    {
      POP_SKIP_NEWLINES();

      auto callNode = create_node<call_node>();

      if (accept(token_types::CLOSE_PAREN))
        ACCEPT_RULE(callNode);

      do
      {
        auto expression = accept_expression();

        if (!expression)
          REJECT_RULE_ERROR("Expected an expression here");

        callNode->arguments.push_back(std::move(expression));
      } while (accept(token_types::COMMA));

      expect(token_types::CLOSE_PAREN);

      ACCEPT_RULE(callNode);
    }
    else
      POP_SKIP_NEWLINES();

    REJECT_RULE();
  }

  unique_ptr<cast_node> parser::accept_cast()
  {
    ENTER_RULE(cast);

    if (accept(token_types::AS))
    {
      auto castNode = create_node<cast_node>();
      castNode->cast_to = accept_type();

      if (!castNode->cast_to)
        REJECT_RULE_ERROR("Expected a type to cast to");

      ACCEPT_RULE(castNode);
    }

    REJECT_RULE();
  }

  unique_ptr<index_node> parser::accept_index()
  {
    ENTER_RULE(index);

    DISALLOW_SKIP_NEWLINES();

    if (accept(token_types::OPEN_SQUARE))
    {
      POP_SKIP_NEWLINES();

      auto indexNode = create_node<index_node>();
      indexNode->index = accept_expression();

      if (!indexNode->index)
        REJECT_RULE_ERROR("Expected an index");

      expect(token_types::CLOSE_SQUARE);

      ACCEPT_RULE(indexNode);
    }
    else
      POP_SKIP_NEWLINES();

    REJECT_RULE();
  }

  // ---------------------------------------------------------------------------

  unique_ptr<expression_node> parser::accept_value()
  {
    ENTER_RULE(value);

    if (auto literalNode = accept_literal())
      ACCEPT_RULE(literalNode);

    else if (auto lambdaNode = accept_lambda())
      ACCEPT_RULE(lambdaNode);

    else if (auto nameReferenceNode = accept_name_reference())
      ACCEPT_RULE(nameReferenceNode);

    else if (accept(token_types::OPEN_PAREN))
    {
      auto expression = accept_expression();
      if (expression && accept(token_types::CLOSE_PAREN))
        ACCEPT_RULE(expression);
    }

    REJECT_RULE();
  }

  unique_ptr<expression_node> parser::accept_literal()
  {
    ENTER_RULE(literal);

    if (
      accept(token_types::_INT8) ||
      accept(token_types::_INT16) ||
      accept(token_types::_INT32) ||
      accept(token_types::_INT64) ||
      accept(token_types::_UINT8) ||
      accept(token_types::_UINT16) ||
      accept(token_types::_UINT32) ||
      accept(token_types::_UINT64) ||
      accept(token_types::_FLOAT32) ||
      accept(token_types::_FLOAT64) ||
      accept(token_types::_CHAR) ||
      accept(token_types::_STRING) ||
      accept(token_types::REGEX) ||
      accept(token_types::_TRUE) ||
      accept(token_types::_FALSE) ||
      accept(token_types::NONE)
    )
    {
      auto literalNode = create_node<literal_node>();
      literalNode->value = last_token();
      ACCEPT_RULE(literalNode);
    }

    REJECT_RULE();
  }

  unique_ptr<lambda_node> parser::accept_lambda()
  {
    ENTER_RULE(lambda);

    if (accept(token_types::LAMBDA))
    {
      auto lambdaNode = create_node<lambda_node>();

      if (accept(token_types::OPEN_PAREN))
      {
        do
        {
          auto param = accept_parameter();

          if (param)
            lambdaNode->parameters.push_back(std::move(param));
          else
            REJECT_RULE_ERROR("Expected parameter");
        } while(accept(token_types::COMMA));

        expect(token_types::CLOSE_PAREN);
      }

      lambdaNode->return_type = accept_type();

      if (accept(token_types::SINGLE_RIGHT_ARROW))
      {
        lambdaNode->inner_scope = accept_scope();
        ACCEPT_RULE(lambdaNode);
      }

      else if (accept(token_types::SINGLE_RIGHT_FAT_ARROW))
      {
        lambdaNode->inner_scope = create_node<scope_node>();
        auto returnNode = create_node<return_node>();
        returnNode->return_expression = accept_expression();
        lambdaNode->inner_scope->statements.push_back(std::move(returnNode));

        ACCEPT_RULE(lambdaNode);
      }
      else
        REJECT_RULE_ERROR(
          "Expected an arrow (-> or =>) after function declaration"
        );
    }

    REJECT_RULE();
  }

  unique_ptr<name_reference_node> parser::accept_name_reference()
  {
    ENTER_RULE(name_reference);

    if (accept(token_types::IDENTIFIER))
    {
      auto nameReferenceNode = create_node<name_reference_node>();
      nameReferenceNode->name = last_token();
      ACCEPT_RULE(nameReferenceNode);
    }

    REJECT_RULE();
  }

  // ---------------------------------------------------------------------------

  unique_ptr<expression_node> parser::accept_generic_expression(
    unique_ptr<expression_node>(parser::*leftRecurse)(),
    unique_ptr<expression_node>(parser::*rightRecurse)(),
    const token_types::type *operators)
  {
    ENTER_RULE(accept_generic_expression);


    auto leftNode = (this->*leftRecurse)();
    if (!leftNode)
      REJECT_RULE();

  again:
    for (auto op = operators; *op; ++op)
    {
      push_current();
      DISALLOW_SKIP_NEWLINES();
      if (!accept(*op))
      {
        POP_SKIP_NEWLINES();
        continue;
      }
      POP_SKIP_NEWLINES();

      auto operation = last_token();

      auto rightNode = (this->*rightRecurse)();

      if (!rightNode)
      {
        pop_current();
        ACCEPT_RULE(leftNode);
      }
      else
      {
        m_currentStack.pop();
      }


      auto binOpNode = create_node<binary_operator_node>();
      binOpNode->left = move(leftNode);
      binOpNode->right = move(rightNode);
      binOpNode->operation = operation;


      leftNode = move(binOpNode);
      goto again;
    }

    ACCEPT_RULE(leftNode);
  }

  unique_ptr<expression_node> parser::accept_generic_expression(
    unique_ptr<expression_node>(parser::*recurse)(),
    const token_types::type *operators)
  {
    return accept_generic_expression(recurse, recurse, operators);
  }

  // ---------------------------------------------------------------------------

  unique_ptr<expression_node> parser::accept_unary_operator()
  {
    ENTER_RULE(unary_operator);

    static const token_types::type unary_operators[] =
    {
      token_types::INCREMENT,
      token_types::DECREMENT,
      token_types::ADDITION,
      token_types::SUBTRACTION,
      token_types::BITWISE_XOR,
      token_types::DOUBLE_DOT,
      token_types::TRIPLE_DOT,
      token_types::LOGICAL_NOT,
      token_types::BITWISE_NOT,
      token_types::MULTIPLICATION,
      token_types::EXPONENT,
      token_types::BITWISE_AND,
      token_types::SIZEOF,
      token_types::ALIGNOF
    };

    for (token_types::type op : unary_operators)
    {
      if (!accept(op)) continue;

      auto unaryOpNode = create_node<unary_operator_node>();

      unaryOpNode->operation = last_token();
      unaryOpNode->expression = accept_unary_operator();
      unaryOpNode->is_post_expression = false;
      ACCEPT_RULE(unaryOpNode);
    }

    if (auto postExprNode = accept_post_expression())
      ACCEPT_RULE(postExprNode);

    REJECT_RULE();
  }

#define BINARY_EXPRESSION(name, next, ...)                            \
  unique_ptr<expression_node> parser::name()                          \
  {                                                                   \
    ENTER_RULE(name);                                                 \
    static const token_types::type operators[] =                      \
    {                                                                 \
      __VA_ARGS__,                                                    \
      token_types::INVALID                                            \
    };                                                                \
    auto node = accept_generic_expression(&parser::next, operators);  \
    if (node) ACCEPT_RULE(node);                                      \
    else REJECT_RULE();                                               \
  }

  BINARY_EXPRESSION(
    accept_arrow,
    accept_unary_operator,

    token_types::SINGLE_RIGHT_ARROW,
    token_types::SINGLE_LEFT_ARROW,
    token_types::SINGLE_RIGHT_LONG_ARROW,
    token_types::SINGLE_LEFT_LONG_ARROW,
    token_types::BIDIRECTIONAL_ARROW,
    token_types::SINGLE_RIGHT_FAT_ARROW,
    token_types::SINGLE_RIGHT_LONG_FAT_ARROW,
    token_types::SINGLE_LEFT_LONG_FAT_ARROW,
    token_types::BIDIRECTIONAL_FAT_ARROW
  );

  BINARY_EXPRESSION(
    accept_expansion,
    accept_arrow,

    token_types::DOUBLE_DOT,
    token_types::TRIPLE_DOT
  );

  BINARY_EXPRESSION(
    accept_null_coalesce,
    accept_expansion,

    token_types::DOUBLE_QUESTION
  );

  BINARY_EXPRESSION(
    accept_exponent,
    accept_null_coalesce,

    token_types::EXPONENT
  );

  BINARY_EXPRESSION(
    accept_multiplication,
    accept_exponent,

    token_types::MULTIPLICATION,
    token_types::DIVIDE,
    token_types::MODULO,
    token_types::DOUBLE_MODULO
  );

  BINARY_EXPRESSION(
    accept_addition,
    accept_multiplication,
    token_types::ADDITION,
    token_types::SUBTRACTION
  );

  BINARY_EXPRESSION(
    accept_bitwise_shift,
    accept_addition,

    token_types::BITSHIFT_LEFT,
    token_types::BITSHIFT_RIGHT,
    token_types::LOGICAL_BITSHIFT_LEFT,
    token_types::LOGICAL_BITSHIFT_RIGHT
  );

  BINARY_EXPRESSION(
    accept_comparison,
    accept_bitwise_shift,

    token_types::GREATER_THAN,
    token_types::LESS_THAN,
    token_types::GREATER_THAN_OR_EQUAL,
    token_types::LESS_THAN_OR_EQUAL,
    token_types::APPROX_GREATER_THAN,
    token_types::APPROX_LESS_THAN,
    token_types::APPROX_GREATER_THAN_OR_EQUAL,
    token_types::APPROX_LESS_THAN_OR_EQUAL
  );

  BINARY_EXPRESSION(
    accept_equality, // ACCEPT IT! IT'S [the current year]
    accept_comparison,

    token_types::EQUALITY,
    token_types::INEQUALITY,
    token_types::APPROX_EQUALITY,
    token_types::APPROX_INEQUALITY
  );

  BINARY_EXPRESSION(
    accept_bitwise_and,
    accept_equality,

    token_types::BITWISE_AND
  );

  BINARY_EXPRESSION(
    accept_bitwise_xor,
    accept_bitwise_and,

    token_types::BITWISE_AND
  );

  BINARY_EXPRESSION(
    accept_bitwise_or,
    accept_bitwise_xor,

    token_types::BITWISE_AND
  );

  BINARY_EXPRESSION(
    accept_logical_and,
    accept_bitwise_or,

    token_types::LOGICAL_AND
  );

  BINARY_EXPRESSION(
    accept_logical_or,
    accept_logical_and,

    token_types::LOGICAL_OR
  );

  BINARY_EXPRESSION(
    accept_pipe,
    accept_logical_or,

    token_types::PIPE_SINGLE_RIGHT,
    token_types::PIPE_SINGLE_LEFT,
    token_types::PIPE_DOUBLE_RIGHT,
    token_types::PIPE_DOUBLE_LEFT,
    token_types::PIPE_TRIPLE_RIGHT,
    token_types::PIPE_TRIPLE_LEFT
  );

  BINARY_EXPRESSION(
    accept_assignment,
    accept_pipe,

    token_types::ASSIGNMENT_CREATE,
    token_types::ASSIGNMENT,
    token_types::ASSIGNMENT_ADDITION,
    token_types::ASSIGNMENT_SUBTRACTION,
    token_types::ASSIGNMENT_MULTIPLICATION,
    token_types::ASSIGNMENT_DIVIDE,
    token_types::ASSIGNMENT_MODULO,
    token_types::ASSIGNMENT_DOUBLE_MODULO,
    token_types::ASSIGNMENT_EXPONENT,
    token_types::ASSIGNMENT_BITSHIFT_LEFT,
    token_types::ASSIGNMENT_BITSHIFT_RIGHT,
    token_types::ASSIGNMENT_BITSHIFT_LOGICAL_LEFT,
    token_types::ASSIGNMENT_BITSHIFT_LOGICAL_RIGHT,
    token_types::ASSIGNMENT_BITWISE_AND,
    token_types::ASSIGNMENT_BITWISE_OR,
    token_types::ASSIGNMENT_BITWISE_XOR,
    token_types::ASSIGNMENT_LOGICAL_AND,
    token_types::ASSIGNMENT_LOGICAL_OR
  );

  // ---------------------------------------------------------------------------

  size_t parser::next_valid_token()
  {
    size_t i = m_currentIndex;
    while (
      i < num_tokens() && (
        (m_allowNewlines.top() && token_at(i).type() == token_types::NEWLINE) ||
        token_at(i).type() == token_types::WHITESPACE ||
        token_at(i).type() == token_types::LINE_COMMENT ||
        token_at(i).type() == token_types::BLOCK_COMMENT ||
        token_at(i).type() == token_types::SHEBANG
      )
    )
    {
      ++i;
    }

    return i;
  }

  bool parser::accept(token_types::type type)
  {
    size_t i = next_valid_token();

    if (i < num_tokens() && token_at(i).type() == type)
    {
      m_currentIndex = i + 1;
      return true;
    }
    else
      return false;
  }

  void parser::expect(token_types::type type)
  {
    if (!accept(type))
      throw parser_error("Expected a different token here", m_currentIndex);
  }

  void parser::expect_semicolon()
  {
    size_t saveCurr = m_currentIndex;
    if (!accept(token_types::SEMICOLON))
    {
      size_t lineNo = current_token().line_number();

      m_currentIndex = saveCurr;
      m_currentIndex = next_valid_token();

      if (current_token().line_number() == lineNo)
        throw parser_error("Expected a semicolon or newline", m_currentIndex);
    }
  }

  // ---------------------------------------------------------------------------

  size_t parser::last_token_index()
  {
    size_t i = m_currentIndex;
    do
    {
      if (i == 0)
        return num_tokens();
      --i;
    } while (
      i < m_module->m_tokens.size() &&
      (
        token_at(i).type() == token_types::WHITESPACE ||
        token_at(i).type() == token_types::NEWLINE ||
        token_at(i).type() == token_types::LINE_COMMENT ||
        token_at(i).type() == token_types::BLOCK_COMMENT ||
        token_at(i).type() == token_types::SHEBANG
      )
    );

    return i;
  }

  const token &parser::last_token()
  {
    size_t i = last_token_index();

    return i == num_tokens() ? token::invalid : token_at(i);
  }

  const token &parser::current_token()
  {
    if (m_currentIndex < m_module->m_tokens.size())
      return token_at(m_currentIndex);
    else
      return token::invalid;
  }

  const token &parser::token_at(size_t i)
  {
    return m_module->m_tokens[i];
  }

  size_t parser::num_tokens()
  {
    return m_module->m_tokens.size();
  }

  // ---------------------------------------------------------------------------

  bool parser::accept_indent()
  {
    size_t i = last_token_index();
    size_t j = next_valid_token();

    if (i == num_tokens() || j == num_tokens())
      return m_indent == 0;

    size_t lnI = token_at(i).line_number();
    size_t lnJ = token_at(j).line_number();

    if (lnI == lnJ)
    {
      m_currentIndex = j;
      return true;
    }
    else if (j == 0)
    {
      return m_indent == 0;
    }

    const token &w = token_at(j - 1);

    if (
        w.type() != token_types::WHITESPACE ||
        w.line_number() != lnJ
      )
    {
      m_currentIndex = j;
      return m_indent == 0;
    }

    if (w.length() / expected_indent_size == m_indent)
    {
      m_currentIndex = j;
      return true;
    }

    return false;
  }

  // ---------------------------------------------------------------------------

  void parser::push_current()
  {
    m_currentStack.push(m_currentIndex);
  }

  void parser::pop_current()
  {
    assert(!m_currentStack.empty());

    m_currentIndex = m_currentStack.top();
    m_currentStack.pop();
  }

  void parser::add_indent()
  {
    ++m_indent;
  }

  void parser::pop_indent()
  {
    --m_indent;
  }

  // ---------------------------------------------------------------------------

  bool parser::at_end_of_stream()
  {
    return next_valid_token() == num_tokens();
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
