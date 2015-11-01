// -----------------------------------------------------------------------------
// AST visitor for making a dotfile
// Howard Hughes
// -----------------------------------------------------------------------------

#include "dotfilevisitor.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  static void token_to_file(FILE *f, const token &tok)
  {
    for (size_t i = 0; i < tok.length(); ++i)
    {
      char c = tok.text()[i];
      switch (c)
      {
      case '"':
        fputs("\\\"", f);
      case '@':
        break;
      default:
        fputc(tok.text()[i], f);
        break;
      }
    }
  }

  static std::string make_name(const char *base, const void *ptr)
  {
    std::string name(base);
    char buffer[16];
    sprintf(buffer, "_%p", ptr);
    name += buffer;
    return std::move(name);
  }

  static void sanatize(std::string &s)
  {
    size_t i = 0;

    while ((i = s.find_first_of('@', i)) != std::string::npos)
    {
      s.erase(i, 1);
    }
  }

#define WALK_CHILDREN(connecTo) do { } while(false);

  // ---------------------------------------------------------------------------

  dotfile_visitor::dotfile_visitor() : m_file(fopen("ast.gv", "wt")), m_connectTo(nullptr)
  {
    fputs(
      "strict digraph {\n"
      "\tgraph [dpi=300];\n"
      "\tnode [shape=plaintext fontname=\"Source Code Pro\" fontsize=12]; \n", m_file);
  }

  dotfile_visitor::~dotfile_visitor()
  {
    fputs("}\n", m_file);
    fclose(m_file);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(abstract_node *node)
  {
    if (m_connectTo)
    {
      if (m_numconnected == 0)
        fprintf(m_file, "\tptr%p -> { ", m_connectTo);

      fprintf(m_file, "ptr%p ", node);
      ++m_numconnected;
    }
    else
    {
      m_connectTo = node;
      m_numconnected = 0;

      walk_node(node, this, false);

      if (m_numconnected > 0)
      {
        fputs("};\n", m_file);
      }

      m_connectTo = nullptr;
      walk_node(node, this, false);
    }
    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(module_node *node)
  {
    format_node(node, "module");
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(symbol_node *node)
  {
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(expression_node *node)
  {
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(post_expression_node *node)
  {
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(statement_node *node)
  {
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(class_node *node)
  {
    format_node(node, "class", node->name);
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(function_node *node)
  {
    format_node(node, "function", node->name);
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(var_node *node)
  {
    format_node(node, "variable", node->name);
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(parameter_node *node)
  {
    format_node(node, "parameter", node->name);
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(property_node *node)
  {
    format_node(node, "property", node->name);
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(binary_operator_node *node)
  {
    format_node(node, "binary operation", node->operation);
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(unary_operator_node *node)
  {
    format_node(node, "unary operation", node->operation);
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(member_access_node *node)
  {
    format_node(node, "member access");
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(call_node *node)
  {
    format_node(node, "call");
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(cast_node *node)
  {
    format_node(node, "call");
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(index_node *node)
  {
    format_node(node, "index");
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(literal_node *node)
  {
    format_node(node, "literal", node->value);
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(lambda_capture_node *node)
  {
    format_node(node, "lambda capture", node->capture_type);
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(lambda_node *node)
  {
    format_node(node, "lambda");
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(name_reference_node *node)
  {
    format_node(node, "name reference", node->name);
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(goto_node *node)
  {
    format_node(node, "goto", node->target_name);
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(label_node *node)
  {
    format_node(node, "label", node->name);
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(return_node *node)
  {
    format_node(node, "return");
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(break_node *node)
  {
    format_node(node, "break", node->count);
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(continue_node *node)
  {
    format_node(node, "continue", node->count);
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(if_node *node)
  {
    format_node(node, "if");
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(while_node *node)
  {
    format_node(node, "while");
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(for_node *node)
  {
    format_node(node, "for");
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(import_node *node)
  {
    format_node(node, "import", node->name_path);
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(typedef_node *node)
  {
    format_node(node, "typedef");
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(type_node *node)
  {
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(tuple_node *node)
  {
    format_node(node, "tuple");
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(delegate_node *node)
  {
    format_node(node, "delegate");
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(plain_type_node *node)
  {
    format_node(node, "type", node->name);
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(decltype_node *node)
  {
    format_node(node, "decltype");
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(post_type_node *node)
  {
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(type_indirect_node *node)
  {
    format_node(node, "indirect", node->indirection_type);
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(type_array_node *node)
  {
    format_node(node, "array");
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(type_template_node *node)
  {
    format_node(node, "template");
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(qualifier_node *node)
  {
    format_node(node, "qualifier");
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(attribute_node *node)
  {
    format_node(node, "attributes");
    return ast_visitor::visit(node);
  }

  ast_visitor::visitor_result dotfile_visitor::visit(scope_node *node)
  {
    format_node(node, "scope");
    return ast_visitor::visit(node);
  }

  // ---------------------------------------------------------------------------

  void dotfile_visitor::format_node(abstract_node *node, const char *name)
  {
    if (!m_connectTo)
      fprintf(m_file, "\tptr%p [label=\"[%s]\"];\n", node, name);
  }

  void dotfile_visitor::format_node(abstract_node *node, const char *name, const token &tok)
  {
    if (!m_connectTo)
    {
      if (tok.type() == token_types::STRING_LITERAL)
        fprintf(m_file, "\tptr%p [label=\"[%s]\\n[%s]\\n\\\"%.*s\\\"\"];\n", node, name, token_types::names[tok.type()], tok.length() - 2, tok.text() + 1);
      else if (tok.text())
        fprintf(m_file, "\tptr%p [label=\"[%s]\\n[%s]\\n%.*s\"];\n", node, name, token_types::names[tok.type()], tok.length(), tok.text());
      else
        format_node(node, name);
    }
  }

  void dotfile_visitor::format_node(abstract_node *node, const char *name, const std::vector<token> &tokens)
  {
    if (!m_connectTo)
    {
      fprintf(m_file, "\tptr%p [label=\"[%s]\\n", node, name);

      bool first = true;
      for (const token &tok : tokens)
      {
        if (first)
          fprintf(m_file, "%.*s", tok.length(), tok.text());
        else
          fprintf(m_file, ".%.*s", tok.length(), tok.text());
        first = false;
      }

      fputs("\"];\n", m_file);
    }
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
