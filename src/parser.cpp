// -----------------------------------------------------------------------------
// Brandy language tokens
// Howard Hughes
// -----------------------------------------------------------------------------

#include "flags.h"
#include "parser.h"
#include <cassert>
#include <iostream>
#include <stack>

// -----------------------------------------------------------------------------

namespace brandy
{
  using std::move;
  using std::make_unique;

  // ---------------------------------------------------------------------------

  class rule_tracker
  {
  public:
    rule_tracker(const char *text) : m_text(text) { ++depth; }
    ~rule_tracker() { --depth; }

    void accept()
    {
      outStack.emplace(depth, m_text);
    }

    static void dump()
    {
      while (!outStack.empty())
      {
        auto top = outStack.top();

        for (size_t i = 0; i < top.first - 1; ++i)
          std::cout << "  ";

        std::cout << top.second << std::endl;

        outStack.pop();
      }
    }

  private:
    const char *m_text;
    static size_t depth;
    static std::stack<std::pair<size_t, const char *>> outStack;
  };

  size_t rule_tracker::depth = 0;
  std::stack<std::pair<size_t, const char*>> rule_tracker::outStack;

  // ---------------------------------------------------------------------------

#define ENTER_RULE(name) rule_tracker ruleDontFuckWithThis(#name)
#define REJECT_RULE() return nullptr
#define REJECT_RULE_ERROR(msg) throw parsing_error(msg, m_current)
#define ACCEPT_RULE(retval) do { ruleDontFuckWithThis.accept(); retval->end = m_current; return move(retval); } while(false)
#define ACCEPT_RULE_AND_DUMP(retval) do { ruleDontFuckWithThis.accept(); retval->end = m_current; rule_tracker::dump(); return move(retval); } while(false)
#define NEWLINE_GAURD() newline_gaurd gaurdDontFuckWithThis(this)

  // ---------------------------------------------------------------------------

  parsing_error::parsing_error(const char *error, std::vector<token>::const_iterator position) :
    m_errStr(error),
    m_pos(position)
  {
  }

  const char *parsing_error::error_str() const
  {
    return m_errStr;
  }

  std::vector<token>::const_iterator parsing_error::position() const
  {
    return m_pos;
  }

  // ---------------------------------------------------------------------------

  parser::parser() :
    m_tokens(),
    m_current(),
    m_disallowNewLines(),
    m_lastLineNum(1)
  {
    m_disallowNewLines.push(false);
  }

  parser::parser(const std::vector<token> &tokens) :
    m_tokens(tokens),
    m_current(m_tokens.begin()),
    m_disallowNewLines(),
    m_lastLineNum(1)
  {
    m_disallowNewLines.push(false);
  }

  parser::parser(std::vector<token> &&tokens) :
    m_tokens(move(tokens)),
    m_current(m_tokens.begin()),
    m_disallowNewLines(),
    m_lastLineNum(1)
  {
    m_disallowNewLines.push(false);
  }

  // ---------------------------------------------------------------------------

  bool parser::at_end_of_stream()
  {
    return m_current >= m_tokens.end();
  }

  // ---------------------------------------------------------------------------

  unique_ptr<module_node> parser::parse_module()
  {
    return accept_module();
  }

  // ---------------------------------------------------------------------------

  const std::vector<token> &parser::tokens() const
  {
    return m_tokens;
  }

  // ---------------------------------------------------------------------------

  unique_ptr<module_node> parser::accept_module()
  {
    ENTER_RULE(module);

    auto moduleNode = create_node<module_node>();

    while (!at_end_of_stream())
    {
      if (auto symbol = accept_symbol())
        moduleNode->members.push_back(move(symbol));
      else if (auto statement = accept_statement())
        moduleNode->statements.push_back(move(statement));
      else
        break;
    }
    if (brandy::compiler_flags::current().dump_parser_stack())
      ACCEPT_RULE_AND_DUMP(moduleNode);
    else
      ACCEPT_RULE(moduleNode);
  }

  // ---------------------------------------------------------------------------

  unique_ptr<symbol_node> parser::accept_symbol()
  {
    ENTER_RULE(symbol);
    unique_ptr<attribute_node> attributes;

    if (accept(token_types::ATTRIBUTE_START))
    {
      attributes = move(accept_attribute());
      if (!attributes) REJECT_RULE_ERROR("No attributes found in attribute list");
      expect(token_types::CLOSE_BRACKET);
    }

    if (auto functionNode = accept_function())
    {
      functionNode->attributes = move(attributes);
      ACCEPT_RULE(functionNode);
    }
    else if (auto classNode = accept_class())
    {
      classNode->attributes = move(attributes);
      ACCEPT_RULE(classNode);
    }
    else if (auto varNode = accept_var())
    {
      varNode->attributes = move(attributes);
      ACCEPT_RULE(varNode);
    }
    else if (auto propertyNode = accept_property())
    {
      propertyNode->attributes = move(attributes);
      ACCEPT_RULE(propertyNode);
    }
    else if (auto typedefNode = accept_typedef())
    {
      typedefNode->attributes = move(attributes);
      ACCEPT_RULE(typedefNode);
    }
    else if (attributes)
    {
      REJECT_RULE_ERROR("Attributes have no matching symbol");
    }
    else
      REJECT_RULE();
  }

  unique_ptr<expression_node> parser::accept_expression()
  {
    ENTER_RULE(expression);

    if (auto assignment = accept_assignment())
      ACCEPT_RULE(assignment);
    else
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
        memberNode->left = move(leftNode);
        leftNode = move(memberNode);
      }
      else if (auto callNode = accept_call())
      {
        callNode->left = move(leftNode);
        leftNode = move(callNode);
      }
      else if (auto castNode = accept_cast())
      {
        castNode->left = move(leftNode);
        leftNode = move(castNode);
      }
      else if (auto indexNode = accept_index())
      {
        indexNode->left = move(leftNode);
        leftNode = move(indexNode);
      }
      else
        ACCEPT_RULE(leftNode);
    }
  }

  unique_ptr<statement_node> parser::accept_statement()
  {
    ENTER_RULE(statement);

    if (auto freeNode = accept_free_statement())
      ACCEPT_RULE(freeNode);
    else if (auto statementNode = accept_delimited_statement())
    {
      expect_semicolon();
      ACCEPT_RULE(statementNode);
    }
    else
      REJECT_RULE();
  }

  // ---------------------------------------------------------------------------

  unique_ptr<class_node> parser::accept_class()
  {
    ENTER_RULE(class);

    auto classNode = create_node<class_node>();
    if (!accept(token_types::CLASS))
      REJECT_RULE();

    expect(token_types::IDENTIFIER);

    classNode->name = last_token();

    if (accept(token_types::OPEN_PAREN))
    {
      do
      {
        auto typeNode = accept_type();
        if (!typeNode) REJECT_RULE_ERROR("Expected base class");

        classNode->base_classes.push_back(move(typeNode));
      } while (accept(token_types::COMMA));

      expect(token_types::CLOSE_PAREN);
    }

    expect(token_types::OPEN_CURLY);

    while (!accept(token_types::CLOSE_CURLY))
    {
      if (auto symbol = accept_symbol())
        classNode->members.push_back(move(symbol));
      else
        REJECT_RULE_ERROR("Classes can only contain functions, variables, or properties");
    }

    ACCEPT_RULE(classNode);
  }

  unique_ptr<function_node> parser::accept_function()
  {
    ENTER_RULE(function);

    auto functionNode = create_node<function_node>();
    if (!accept(token_types::FUNCTION))
      REJECT_RULE();

    expect(token_types::IDENTIFIER);

    functionNode->name = last_token();

    if (accept(token_types::OPEN_PAREN))
    {

      if (auto firstParam = accept_parameter())
      {
        functionNode->parameters.push_back(move(firstParam));

        while (accept(token_types::COMMA))
        {
          auto param = accept_parameter();
          if (!param) REJECT_RULE_ERROR("Comma in function called not followed by a parameter");

          functionNode->parameters.push_back(move(param));
        }
      }

      expect(token_types::CLOSE_PAREN);
    }
      
    functionNode->return_type = accept_type();

    functionNode->scope = accept_scope();

    if (!functionNode->scope)
      REJECT_RULE_ERROR("No scope found accompanying function");
    else
      ACCEPT_RULE(functionNode);
  }

  unique_ptr<var_node> parser::accept_var()
  {
    ENTER_RULE(variable);

    auto varNode = create_node<var_node>();
    if (!accept(token_types::VAR))
      REJECT_RULE();

    expect(token_types::IDENTIFIER);

    varNode->name = last_token();

    if (accept(token_types::COLON))
    {
      varNode->type = accept_type();
      if (!varNode->type) REJECT_RULE_ERROR("Colons must be followed by a type in variable declarations.");
    }

    if (accept(token_types::ASSIGNMENT))
    {
      varNode->expression = accept_expression();
      if (!varNode->expression) REJECT_RULE_ERROR("Variable assignment must be followed by an expression");
    }

    ACCEPT_RULE(varNode);
  }

  unique_ptr<parameter_node> parser::accept_parameter()
  {
    ENTER_RULE(parameter);

    auto paramNode = create_node<parameter_node>();

    if (accept(token_types::ATTRIBUTE_START))
    {
      paramNode->attributes = accept_attribute();
      expect(token_types::CLOSE_BRACKET);
    }

    if (!accept(token_types::IDENTIFIER))
    {
      if (paramNode->attributes)
        REJECT_RULE_ERROR("No matching parameter for attributes");
      else
        REJECT_RULE();
    }

    paramNode->name = last_token();

    if (accept(token_types::AS))
    {
      paramNode->type = accept_type();
      if (!paramNode->type) REJECT_RULE_ERROR("Missing typen name for parameter");
    }

    if (accept(token_types::ASSIGNMENT))
    {
      paramNode->default_value = accept_expression();
      if (!paramNode->type) REJECT_RULE_ERROR("Equal sign found, but no default value given for parameter");
    }

    ACCEPT_RULE(paramNode);
  }

  unique_ptr<property_node> parser::accept_property()
  {
    ENTER_RULE(property);

    auto propertyNode = create_node<property_node>();
    if (!accept(token_types::PROPERTY))
      REJECT_RULE();

    expect(token_types::IDENTIFIER);

    propertyNode->name = last_token();

    if (accept(token_types::COLON))
    {
      propertyNode->type = accept_type();
      if (!propertyNode->type) REJECT_RULE_ERROR("No type following : in property definition");
    }

    bool opened = accept(token_types::OPEN_CURLY);

    if (accept(token_types::GET))
    {
      propertyNode->getter = accept_scope();
    }
    if (accept(token_types::SET))
    {
      if (accept(token_types::IDENTIFIER))
        propertyNode->name = last_token();

      propertyNode->setter = accept_scope();
    }
    if (!propertyNode->getter && accept(token_types::GET))
    {
      // maybe the getter came second
      propertyNode->getter = accept_scope();
    }

    if (opened) accept(token_types::CLOSE_CURLY);

    ACCEPT_RULE(propertyNode);
  }

  // ---------------------------------------------------------------------------

  unique_ptr<member_access_node> parser::accept_member_access()
  {
    ENTER_RULE(member access);

    auto memberAccess = create_node<member_access_node>();

    if (!accept(token_types::DOT))
      REJECT_RULE();

    expect(token_types::IDENTIFIER);
    memberAccess->member_name = last_token();

    ACCEPT_RULE(memberAccess);
  }

  unique_ptr<call_node> parser::accept_call()
  {
    ENTER_RULE(call);

    auto callNode = create_node<call_node>();

    if (accept(token_types::OPEN_PAREN))
    {
      if (auto firstParam = accept_expression())
      {
        callNode->parameters.push_back(move(firstParam));

        while (accept(token_types::COMMA))
        {
          auto param = accept_expression();
          if (!param) REJECT_RULE_ERROR("Comma in function call not followed by expression");
          callNode->parameters.push_back(move(param));
        }
      }

      expect(token_types::CLOSE_PAREN);

      ACCEPT_RULE(callNode);
    }
    else if (allows_for_noparen_call())
    {
      // Maybe a call with no parens, but only if followed by an expression.
      // Do not allow newlines in the middle of expresions when doing this!
      disallow_skip_newlines();

      if (auto fistParam = accept_expression())
      {
        pop_skip_newlines();

        callNode->parameters.push_back(move(fistParam));

        while (accept(token_types::COMMA))
        {
          auto param = accept_expression();
          if (!param) REJECT_RULE_ERROR("Comma in function call not followed by expression on same line");
          callNode->parameters.push_back(move(param));
        }

        ACCEPT_RULE(callNode);
      }

      pop_skip_newlines();
    }

    REJECT_RULE();
  }

  unique_ptr<cast_node> parser::accept_cast()
  {
    ENTER_RULE(cast);

    auto castNode = create_node<cast_node>();
    if (!accept(token_types::AS))
      REJECT_RULE();

    castNode->type = accept_type();

    if (!castNode->type) REJECT_RULE_ERROR("Casting operation requiers a type name");

    ACCEPT_RULE(castNode);
  }

  unique_ptr<index_node> parser::accept_index()
  {
    ENTER_RULE(index);

    auto indexNode = create_node<index_node>();

    if (!accept(token_types::OPEN_BRACKET))
      REJECT_RULE();

    indexNode->index = accept_expression();

    if (!indexNode->index) REJECT_RULE_ERROR("Index operation requires an expression to index with");

    expect(token_types::CLOSE_BRACKET);

    ACCEPT_RULE(indexNode);
  }

  // ---------------------------------------------------------------------------

  unique_ptr<literal_node> parser::accept_literal()
  {
    ENTER_RULE(literal);

    static const token_types::type literals[] =
    {
      token_types::INTEGER_LITERAL,
      token_types::FLOAT_LITERAL,
      token_types::STRING_LITERAL,
      token_types::CHAR_LITERAL,
      token_types::TRUE,
      token_types::FALSE,
      token_types::NIL,
    };

    auto literalNode = create_node<literal_node>();
    for (token_types::type allow : literals)
    {
      if (accept(allow))
      {
        literalNode->value = last_token();

        ACCEPT_RULE(literalNode);
      }
    }

    REJECT_RULE();
  }

  unique_ptr<lambda_capture_node> parser::accept_lambda_capture()
  {
    ENTER_RULE(lambda_capture);

    auto captureNode = create_node<lambda_capture_node>();

    if (accept(token_types::VALUE) || accept(token_types::REFERENCE))
    {
      captureNode->capture_type = last_token();
      captureNode->name = accept_name_reference();
      if (!captureNode->name) REJECT_RULE_ERROR("No name reference assosciated with capture by value in lambda capture list");
      ACCEPT_RULE(captureNode);
    }
    else if (auto name = accept_name_reference())
    {
      captureNode->capture_type = token("ref", 3, token_types::REFERENCE);
      captureNode->name = move(name);
      ACCEPT_RULE(captureNode);
    }
    else
      REJECT_RULE();
  }

  unique_ptr<lambda_node> parser::accept_lambda()
  {
    ENTER_RULE(lambda);

    auto lambdaNode = create_node<lambda_node>();

    if (!accept(token_types::LAMBDA))
      REJECT_RULE();

    if (accept(token_types::OPEN_BRACKET))
    {
      if (auto firstCapture = accept_lambda_capture())
      {
        lambdaNode->captures.push_back(move(firstCapture));

        while (accept(token_types::COMMA))
        {
          auto capture = accept_lambda_capture();
          if (!capture) REJECT_RULE_ERROR("Comma in lambda capture list not followed by a captured value");
          lambdaNode->captures.push_back(move(capture));
        }
      }

      expect(token_types::CLOSE_BRACKET);
    }

    bool opened = accept(token_types::OPEN_PAREN);

    if (auto firstParam = accept_parameter())
    {
      lambdaNode->parameters.push_back(move(firstParam));

      while (accept(token_types::COMMA))
      {
        auto param = accept_parameter();
        if (!param) REJECT_RULE_ERROR("Comma in lambda definition not followed by a parameter");

        lambdaNode->parameters.push_back(move(param));
      }
    }

    if (opened)
    {
      expect(token_types::CLOSE_PAREN);
      lambdaNode->return_type = accept_type();
    }

    lambdaNode->scope = accept_scope();

    if (!lambdaNode->scope)
      REJECT_RULE_ERROR("No scope found accompanying lambda");
    else
      ACCEPT_RULE(lambdaNode);
  }

  unique_ptr<name_reference_node> parser::accept_name_reference()
  {
    ENTER_RULE(name reference);

    auto nameRefNode = create_node<name_reference_node>();
    if (!accept(token_types::IDENTIFIER))
      REJECT_RULE();

    nameRefNode->name = last_token();

    ACCEPT_RULE(nameRefNode);
  }

  unique_ptr<expression_node> parser::accept_value()
  {
    ENTER_RULE(value);

    if (auto litteral = accept_literal())
      ACCEPT_RULE(litteral);
    else if (auto nameRef = accept_name_reference())
      ACCEPT_RULE(nameRef);
    else if (auto lambda = accept_lambda())
      ACCEPT_RULE(lambda);
    if (accept(token_types::OPEN_PAREN))
    {
      auto node = accept_expression();
      accept(token_types::CLOSE_PAREN);
      ACCEPT_RULE(node);
    }

    REJECT_RULE();
  }

  // ---------------------------------------------------------------------------

  unique_ptr<expression_node> parser::accept_unary_operator()
  {
    ENTER_RULE(unary operator);

    static const token_types::type unary_operators[] =
    {
      token_types::SUBTRACT,
      token_types::BITWISE_NOT,
      token_types::BITWISE_AND,
      token_types::MULTIPLY,
      token_types::LOGICAL_NOT
    };

    auto unaryOpNode = create_node<unary_operator_node>();

    for (token_types::type op : unary_operators)
    {
      if (!accept(op)) continue;

      unaryOpNode->operation = last_token();
      unaryOpNode->expression = accept_unary_operator();
      ACCEPT_RULE(unaryOpNode);
    }

    if (auto postExprNode = accept_post_expression())
      ACCEPT_RULE(postExprNode);
    else
      REJECT_RULE();
  }

  // ---------------------------------------------------------------------------

  unique_ptr<expression_node> parser::accept_generic_expression(
    unique_ptr<expression_node>(parser::*leftRecurse)(),
    unique_ptr<expression_node>(parser::*rightRecurse)(),
    const token_types::type *operators)
  {
    auto binOpNode = create_node<binary_operator_node>();

    auto leftNode = (this->*leftRecurse)();
    if (!leftNode)
      return nullptr;

  again:
    for (auto op = operators; *op; ++op)
    {
      disallow_skip_newlines();
      if (!accept(*op))
      {
        pop_skip_newlines();
        continue;
      }
      pop_skip_newlines();

      binOpNode->left = move(leftNode);
      binOpNode->operation = last_token();

      binOpNode->right = (this->*rightRecurse)();

      if (!binOpNode) REJECT_RULE_ERROR("No right hand side for binary operator expression");

      leftNode = move(binOpNode);
      binOpNode = create_node<binary_operator_node>();
      goto again;
    }

    return leftNode;
  }

  unique_ptr<expression_node> parser::accept_generic_expression(
    unique_ptr<expression_node>(parser::*recurse)(),
    const token_types::type *operators)
  {
    return accept_generic_expression(recurse, recurse, operators);
  }

  unique_ptr<expression_node> parser::accept_multiplication()
  {
    ENTER_RULE(multiplication);

    static const token_types::type operators[] =
    {
      token_types::MULTIPLY,
      token_types::DIVIDE,
      token_types::MODULO,
      token_types::INVALID
    };

    auto node = accept_generic_expression(&parser::accept_unary_operator, operators);

    if (node)
      ACCEPT_RULE(node);
    else
      REJECT_RULE();
  }

  unique_ptr<expression_node> parser::accept_addition()
  {
    ENTER_RULE(addition);

    static const token_types::type operators[] =
    {
      token_types::ADD,
      token_types::SUBTRACT,
      token_types::INVALID
    };

    auto node = accept_generic_expression(&parser::accept_multiplication, operators);

    if (node)
      ACCEPT_RULE(node);
    else
      REJECT_RULE();
  }

  unique_ptr<expression_node> parser::accept_bitwise_shift()
  {
    ENTER_RULE(bitwise shift);

    static const token_types::type operators[] =
    {
      token_types::BITWISE_LEFT_SHIFT,
      token_types::BITWISE_RIGHT_SHIFT,
      token_types::INVALID
    };

    auto node = accept_generic_expression(&parser::accept_addition, operators);

    if (node)
      ACCEPT_RULE(node);
    else
      REJECT_RULE();
  }

  unique_ptr<expression_node> parser::accept_comparison()
  {
    ENTER_RULE(comparison);

    static const token_types::type operators[] =
    {
      token_types::GREATER_THAN,
      token_types::LESS_THAN,
      token_types::GREATER_THAN_OR_EQUAL,
      token_types::LESS_THAN_OR_EQUAL,
      token_types::INVALID
    };

    auto node = accept_generic_expression(&parser::accept_bitwise_shift, operators);

    if (node)
      ACCEPT_RULE(node);
    else
      REJECT_RULE();
  }

  unique_ptr<expression_node> parser::accept_equality()
  {
    ENTER_RULE(equality);

    static const token_types::type operators[] =
    {
      token_types::EQUALITY,
      token_types::INEQUALITY,
      token_types::INVALID
    };

    auto node = accept_generic_expression(&parser::accept_comparison, operators);

    if (node)
      ACCEPT_RULE(node);
    else
      REJECT_RULE();
  }

  unique_ptr<expression_node> parser::accept_bitwise_conjunction()
  {
    ENTER_RULE(bitwise conjunction);

    static const token_types::type operators[] =
    {
      token_types::BITWISE_AND,
      token_types::BITWISE_OR,
      token_types::BITWISE_XOR,
      token_types::INVALID
    };

    auto node = accept_generic_expression(&parser::accept_equality, operators);

    if (node)
      ACCEPT_RULE(node);
    else
      REJECT_RULE();
  }

  unique_ptr<expression_node> parser::accept_logical_conjunction()
  {
    ENTER_RULE(logical conjunction);

    static const token_types::type operators[] =
    {
      token_types::LOGICAL_AND,
      token_types::LOGICAL_OR,
      token_types::INVALID
    };

    auto node = accept_generic_expression(&parser::accept_bitwise_conjunction, operators);

    if (node)
      ACCEPT_RULE(node);
    else
      REJECT_RULE();
  }

  unique_ptr<expression_node> parser::accept_assignment()
  {
    ENTER_RULE(logical conjunction);

    static const token_types::type operators[] =
    {
      token_types::ASSIGNMENT,
      token_types::ASSIGNMENT_ADD,
      token_types::ASSIGNMENT_SUBTRACT,
      token_types::ASSIGNMENT_MULTIPLY,
      token_types::ASSIGNMENT_DIVIDE,
      token_types::ASSIGNMENT_MODULO,
      token_types::ASSIGNMENT_BITWISE_LEFT_SHIFT,
      token_types::ASSIGNMENT_BITWISE_RIGHT_SHIFT,
      token_types::ASSIGNMENT_BITWISE_AND,
      token_types::ASSIGNMENT_BITWISE_OR,
      token_types::ASSIGNMENT_BITWISE_XOR,
      token_types::ASSIGNMENT_LOGICAL_AND,
      token_types::ASSIGNMENT_LOGICAL_OR,
      token_types::INVALID
    };

    auto node = accept_generic_expression(&parser::accept_logical_conjunction, &parser::accept_expression, operators);

    if (node)
      ACCEPT_RULE(node);
    else
      REJECT_RULE();
  }

  // ---------------------------------------------------------------------------

  unique_ptr<statement_node> parser::accept_delimited_statement()
  {
    ENTER_RULE(delimited statement);

    if (auto exprNode = accept_expression())
      ACCEPT_RULE(exprNode);
    else if (auto varNode = accept_var())
      ACCEPT_RULE(varNode);
    else if (auto returnNode = accept_return())
      ACCEPT_RULE(returnNode);
    else if (auto breakNode = accept_break())
      ACCEPT_RULE(breakNode);
    else if (auto continueNode = accept_break())
      ACCEPT_RULE(continueNode);
    else if (auto labelNode = accept_label())
      ACCEPT_RULE(labelNode);
    else if (auto gotoNode = accept_goto())
      ACCEPT_RULE(gotoNode);

    REJECT_RULE();
  }

#define CONDITIONAL_STATEMENT(name) \
  do \
            { \
    NEWLINE_GAURD();\
    auto ifNode = create_node<if_node>();\
    if (accept(token_types::IF)) \
                        {\
      ifNode->condition = accept_expression();\
      if(!ifNode->condition) REJECT_RULE_ERROR("No condotion on a conditional " #name);\
      ifNode->scope = create_node<scope_node>();\
      name##Node->end = m_current;\
      ifNode->scope->statements.push_back(move(name##Node));\
      ACCEPT_RULE(ifNode);\
                        }\
                                                                                                                                else if (accept(token_types::UNLESS))\
              {\
      auto notNode = create_node<unary_operator_node>();\
      notNode->operation = token("!", 1, token_types::LOGICAL_NOT);\
      notNode->expression = accept_expression();\
      if (!notNode->expression) REJECT_RULE_ERROR("No condition on a conditional " #name);\
      notNode->end = m_current;\
      ifNode->condition = move(notNode);\
      ifNode->scope = create_node<scope_node>();\
      name##Node->end = m_current;\
      ifNode->scope->statements.push_back(move(name##Node));\
      ACCEPT_RULE(ifNode);\
              }\
            } while (false)\

  unique_ptr<statement_node> parser::accept_goto()
  {
    ENTER_RULE(goto);

    auto gotoNode = create_node<goto_node>();
    if (!accept(token_types::GOTO))
      REJECT_RULE();

    expect(token_types::IDENTIFIER);

    gotoNode->target_name = last_token();

    CONDITIONAL_STATEMENT(goto);

    ACCEPT_RULE(gotoNode);
  }

  unique_ptr<label_node> parser::accept_label()
  {
    ENTER_RULE(label);

    auto labelNode = create_node<label_node>();
    if (!accept(token_types::LABEL))
      REJECT_RULE();

    expect(token_types::IDENTIFIER);

    labelNode->name = last_token();

    ACCEPT_RULE(labelNode);
  }

  unique_ptr<statement_node> parser::accept_return()
  {
    ENTER_RULE(return);

    auto returnNode = create_node<return_node>();
    if (!accept(token_types::RETURN))
      REJECT_RULE();

    returnNode->value = accept_expression();

    CONDITIONAL_STATEMENT(return);

    ACCEPT_RULE(returnNode);
  }

  unique_ptr<statement_node> parser::accept_break()
  {
    ENTER_RULE(break);

    auto breakNode = create_node<break_node>();

    if (!accept(token_types::BREAK))
      REJECT_RULE();

    if (accept(token_types::INTEGER_LITERAL))
      breakNode->count = last_token();

    CONDITIONAL_STATEMENT(break);

    ACCEPT_RULE(breakNode);
  }

  unique_ptr<statement_node> parser::accept_continue()
  {
    ENTER_RULE(break);
    auto continueNode = create_node<continue_node>();

    if (!accept(token_types::BREAK))
      REJECT_RULE();

    if (accept(token_types::INTEGER_LITERAL))
      continueNode->count = last_token();

    CONDITIONAL_STATEMENT(continue);

    ACCEPT_RULE(continueNode);
  }

  // ---------------------------------------------------------------------------

  unique_ptr<statement_node> parser::accept_free_statement()
  {
    ENTER_RULE(free statement);

    if (auto ifNode = accept_if())
      ACCEPT_RULE(ifNode);
    else if (auto whileNode = accept_while())
      ACCEPT_RULE(whileNode);
    else if (auto forNode = accept_for())
      ACCEPT_RULE(forNode);
    else if (auto importNode = accept_import())
      ACCEPT_RULE(importNode);
    else
      REJECT_RULE();
  }

  unique_ptr<if_node> parser::accept_if()
  {
    ENTER_RULE(if);

    auto ifNode = create_node<if_node>();
    if (accept(token_types::IF))
    {
      ifNode->condition = accept_expression();
      if (!ifNode->condition) REJECT_RULE_ERROR("No condition given for if statment");

      ifNode->scope = accept_scope();
      if (!ifNode->scope) REJECT_RULE_ERROR("No matching scope to if statement");

      ifNode->else_clause = accept_else();

      ACCEPT_RULE(ifNode);
    }
    else if (accept(token_types::UNLESS))
    {
      auto notNode = create_node<unary_operator_node>();
      notNode->operation = token("!", 1, token_types::LOGICAL_NOT);
      notNode->expression = accept_expression();
      if (!notNode->expression) REJECT_RULE_ERROR("No codnition given for unless statement");

      notNode->end = m_current;
      ifNode->condition = move(notNode);

      ifNode->scope = accept_scope();
      if (!ifNode->scope) REJECT_RULE_ERROR("No matching scope to unless statement");

      ifNode->else_clause = accept_else();

      ACCEPT_RULE(ifNode);
    }

    REJECT_RULE();
  }

  unique_ptr<if_node> parser::accept_else()
  {
    auto elseNode = create_node<if_node>();
    ENTER_RULE(else);

    if (accept(token_types::ELIF))
    {
      elseNode->condition = accept_expression();
      if (!elseNode->condition) REJECT_RULE_ERROR("No condition given for elif statement");

      elseNode->scope = accept_scope();
      if (!elseNode->scope) REJECT_RULE_ERROR("No matching scope to elif statement");

      elseNode->else_clause = accept_else();

      ACCEPT_RULE(elseNode);
    }
    else if (accept(token_types::ELSE))
    {
      elseNode->condition = nullptr;

      elseNode->scope = accept_scope();
      if (!elseNode->scope) REJECT_RULE_ERROR("No matching scope to else statement");

      elseNode->else_clause = nullptr;

      ACCEPT_RULE(elseNode);
    }

    REJECT_RULE();
  }

  unique_ptr<while_node> parser::accept_while()
  {
    ENTER_RULE(while);

    auto whileNode = create_node<while_node>();

    if (accept(token_types::WHILE))
    {
      whileNode->condition = accept_expression();
      if (!whileNode->condition) REJECT_RULE_ERROR("No condition given for while statement");

      whileNode->scope = accept_scope();
      if (!whileNode->scope) REJECT_RULE_ERROR("No matching scope to while statement");

      ACCEPT_RULE(whileNode);
    }
    else if (accept(token_types::UNTIL))
    {
      auto notNode = create_node<unary_operator_node>();
      notNode->operation = token("!", 1, token_types::LOGICAL_NOT);
      notNode->expression = accept_expression();
      if (!notNode->expression) REJECT_RULE_ERROR("No condition given for until statement");

      notNode->end = m_current;
      whileNode->condition = move(notNode);

      whileNode->scope = accept_scope();
      if (!whileNode->scope) REJECT_RULE_ERROR("No matching scope to until statement");

      ACCEPT_RULE(whileNode);
    }

    REJECT_RULE();
  }

  unique_ptr<for_node> parser::accept_for()
  {
    ENTER_RULE(for);

    auto forNode = create_node<for_node>();
    if (!accept(token_types::FOR))
      REJECT_RULE();

    expect(token_types::IDENTIFIER);

    forNode->loop_var_name = last_token();

    if (accept(token_types::IN))
    {
      forNode->loop_iterator = accept_expression();
      if (!forNode->loop_iterator) REJECT_RULE_ERROR("No iterator expression in for statement");

      if (accept(token_types::IF))
      {
        forNode->condition = accept_expression();
        if (!forNode->condition) REJECT_RULE_ERROR("No condition given for conditional for statement");
      }

      forNode->scope = accept_scope();
      if (!forNode->scope) REJECT_RULE_ERROR("No matching scope to for statement");

      ACCEPT_RULE(forNode);
    }
    else if (accept(token_types::FROM))
    {
      forNode->loop_start = accept_expression();
      if (!forNode->loop_start) REJECT_RULE_ERROR("No starting expression in for statement");

      expect(token_types::TO);

      forNode->loop_end = accept_expression();
      if (!forNode->loop_end) REJECT_RULE_ERROR("No ending expression in for statement");

      if (accept(token_types::EVERY))
      {
        forNode->loop_increment = accept_expression();
        if (!forNode->loop_increment) REJECT_RULE_ERROR("No every expression in for statement");
      }

      if (accept(token_types::IF))
      {
        forNode->condition = accept_expression();
        if (!forNode->condition) REJECT_RULE_ERROR("No condition given for conditional for statement");
      }

      forNode->scope = accept_scope();
      if (!forNode->scope) REJECT_RULE_ERROR("No matching scope to for statement");

      ACCEPT_RULE(forNode);
    }

    REJECT_RULE();
  }

  unique_ptr<import_node> parser::accept_import()
  {
    ENTER_RULE(import);

    auto importNode = create_node<import_node>();
    bool meta = false;

    if (accept(token_types::META))
    {
      expect(token_types::IMPORT);
      meta = true;
    }
    else if (!accept(token_types::IMPORT))
    {
      REJECT_RULE();
    }

    expect(token_types::IDENTIFIER);
    importNode->name_path.push_back(last_token());

    while (accept(token_types::DOT))
    {
      expect(token_types::IDENTIFIER);
      importNode->name_path.push_back(last_token());
    }

    if (accept(token_types::AS))
    {
      expect(token_types::IDENTIFIER);
      importNode->effective_name = last_token();
    }
    else
    {
      importNode->effective_name = importNode->name_path.front();
    }

    ACCEPT_RULE(importNode);
  }

  // ---------------------------------------------------------------------------
  
  unique_ptr<typedef_node> parser::accept_typedef()
  {
    ENTER_RULE(typedef);

    auto typedefNode = create_node<typedef_node>();

    if (accept(token_types::TYPEDEF))
    {
      expect(token_types::IDENTIFIER);
      
      typedefNode->name = last_token();

      accept(token_types::AS);

      typedefNode->type = accept_type();
      if (!typedefNode->type) REJECT_RULE_ERROR("typename not present in typedef");

      ACCEPT_RULE(typedefNode);
    }

    REJECT_RULE();
  }

  // ---------------------------------------------------------------------------

  unique_ptr<type_node> parser::accept_type()
  {
    ENTER_RULE(type);

    auto tupleNode = create_node<tuple_node>();
    auto delegateNode = create_node<delegate_node>();
    auto decltypeNode = create_node<decltype_node>();

    if (accept(token_types::OPEN_CURLY))
    {
      do
      {
        auto type = accept_type();
        if (!type) REJECT_RULE_ERROR("Tuple must contain at least one element");
        tupleNode->inner_types.push_back(move(type));
      } while (accept(token_types::COMMA));

      expect(token_types::CLOSE_CURLY);

      ACCEPT_RULE(tupleNode);
    }
    else if (accept(token_types::FUNCTION))
    {
      expect(token_types::OPEN_PAREN);

      if (auto firstType = accept_type())
      {
        delegateNode->parameter_types.push_back(move(firstType));

        while (accept(token_types::COMMA))
        {
          auto type = accept_type();
          if (!type) REJECT_RULE_ERROR("Comma in delegate parameter list not followed by type");
          delegateNode->parameter_types.push_back(move(type));
        }
      }

      expect(token_types::CLOSE_PAREN);

      delegateNode->return_type = accept_type();
      if (!delegateNode->return_type) REJECT_RULE_ERROR("Need to have return type on delegate type");

      ACCEPT_RULE(delegateNode);
    }
    else if (accept(token_types::DECLTYPE))
    {
      decltypeNode->decltype_expression = accept_expression();
      if (!decltypeNode->decltype_expression) REJECT_RULE_ERROR("No expression following decltype");

      ACCEPT_RULE(decltypeNode);
    }
    else
    {
      auto plainType = create_node<plain_type_node>();

      while (auto qualifier = accept_qualifier())
      {
        plainType->qualifiers.push_back(move(qualifier));
      }

      do
      {
        if (!accept(token_types::IDENTIFIER))
        {
          if (plainType->qualifiers.size() == 0 &&
            plainType->name.size() == 0)
            REJECT_RULE();
          else
            REJECT_RULE_ERROR("Expected an identifier in typename");
        }

        plainType->name.push_back(last_token());
      } while (accept(token_types::DOT));

      while (auto postType = accept_post_type_node())
      {
        plainType->post_type.push_back(move(postType));
      }

      ACCEPT_RULE(plainType);
    }
  }

  unique_ptr<post_type_node> parser::accept_post_type_node()
  {
    ENTER_RULE(post_type);

    auto indirectNode = create_node<type_indirect_node>();
    auto arrayNode = create_node<type_array_node>();
    auto templateNode = create_node<type_template_node>();

    if (accept(token_types::MULTIPLY) || accept(token_types::BITWISE_AND))
    {
      indirectNode->indirection_type = last_token();
      ACCEPT_RULE(indirectNode);
    }
    else if (accept(token_types::OPEN_BRACKET))
    {
      arrayNode->array_size = accept_expression();
      ACCEPT_RULE(arrayNode);
    }
    else if (accept(token_types::OPEN_PAREN))
    {
      if (auto firstParam = accept_expression())
      {
        templateNode->template_parameters.push_back(move(firstParam));

        do
        {
          auto param = accept_expression();
          if (!param) REJECT_RULE_ERROR("Comma in template parameter list not followed by expression");
          templateNode->template_parameters.push_back(move(param));
        } while (accept(token_types::COMMA));
      }

      ACCEPT_RULE(templateNode);
    }

    REJECT_RULE();
  }

  // ---------------------------------------------------------------------------

  unique_ptr<qualifier_node> parser::accept_qualifier()
  {
    // TODO: qualifiers
    return nullptr;
  }

  unique_ptr<attribute_node> parser::accept_attribute()
  {
    ENTER_RULE(attribute);

    auto attrNode = create_node<attribute_node>();

    auto expression = accept_expression();
    if (!expression) REJECT_RULE();

    attrNode->attributes.push_back(move(expression));

    while (accept(token_types::COMMA))
    {
      expression = accept_expression();
      if (!expression) REJECT_RULE_ERROR("No expression following comma in attribute list");

      attrNode->attributes.push_back(move(expression));
    }

    ACCEPT_RULE(attrNode);
  }

  unique_ptr<scope_node> parser::accept_scope()
  {
    ENTER_RULE(scope);

    auto scopeNode = create_node<scope_node>();

    if (accept(token_types::COLON))
    {
      auto statement = accept_statement();
      if (!statement) REJECT_RULE_ERROR("No statement following colon in scope");

      scopeNode->statements.push_back(move(statement));

      ACCEPT_RULE(scopeNode);
    }
    else if (accept(token_types::OPEN_CURLY))
    {
      while (auto statement = accept_statement())
      {
        scopeNode->statements.push_back(move(statement));
      }

      expect(token_types::CLOSE_CURLY);
      ACCEPT_RULE(scopeNode);
    }

    REJECT_RULE();
  }

  // ---------------------------------------------------------------------------

  bool parser::accept(token_types::type type)
  {
    if (at_end_of_stream()) return false;

    // If currently not allowing for newlines,
    // don't accept tokens on a different line
    if (m_disallowNewLines.top())
    {
      if (m_current->line_number() != m_lastLineNum)
        return false;
    }

    if (m_current->type() == type)
    {
      m_lastLineNum = m_current->line_number();
      ++m_current;
      // accepted_token(type);
      return true;
    }
    else
      return false;
  }

  void parser::expect(token_types::type type)
  {
    if (!accept(type))
      throw parsing_error("Expected token of different type", m_current);
  }

  // ---------------------------------------------------------------------------

  void parser::expect_semicolon()
  {
    // Check if we're at the end of the stream (IE, the last line of code)
    if (at_end_of_stream())
    {
      // accepted_token(newline);
    }
    // Check if we're on the next line
    else if (m_current->line_number() > m_lastLineNum)
    {
      // accepted_token(newline);
    }
    // Accept a semicolon
    else if (m_current->type() == token_types::SEMICOLON)
    {
      m_lastLineNum = m_current->line_number();
      ++m_current;
      // accepted_token(semicolon);
    }
    else
    {
      throw parsing_error("Expected a semicolon or newline", m_current);
    }
  }

  // ---------------------------------------------------------------------------

  bool parser::allows_for_noparen_call()
  {
    if (at_end_of_stream())
    {
      return false;
    }
    else
    {
      for (auto it = m_current; it != m_tokens.end(); ++it)
      {
        if (it->line_number() != m_lastLineNum)
          break;
        else if (it->type() == token_types::COMMA)
          return true;
      }

      return 
        m_current->type() != token_types::SUBTRACT &&
        m_current->type() != token_types::BITWISE_AND &&
        m_current->type() != token_types::MULTIPLY;
    }
  }

  // ---------------------------------------------------------------------------

  void parser::disallow_skip_newlines()
  {
    //++m_disallowNewlines;
    m_disallowNewLines.push(true);
  }

  void parser::allow_skip_newlines()
  {
    //assert(m_disallowNewlines > 0);
    //--m_disallowNewlines;
    m_disallowNewLines.push(false);
  }

  void parser::pop_skip_newlines()
  {
    m_disallowNewLines.pop();
  }

  // ---------------------------------------------------------------------------

  const token &parser::last_token()
  {
    return *(m_current - 1);
  }

  const token &parser::current_token()
  {
    return *m_current;
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
