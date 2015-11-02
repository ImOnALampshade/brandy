// -----------------------------------------------------------------------------
// AST visitor for making a dotfile
// Howard Hughes
// -----------------------------------------------------------------------------

#include "dotfilevisitor.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  dotfile_visitor::dotfile_visitor() : m_file(fopen("ast.gv", "wt")), m_connectTo(nullptr)
  {
    fputs(
      "strict digraph {\n"
      "\tgraph [dpi=300];\n"
      "\tnode [shape=plaintext fontname=\"Source Code Pro\" fontsize=12];\n"
      "\tedge [arrowhead=diamond];\n", m_file);
  }

  dotfile_visitor::~dotfile_visitor()
  {
    fputs("}\n", m_file);
    fclose(m_file);
  }

#define DOTFILE_VISTOR(nodetype, ...) ast_visitor::visitor_result dotfile_visitor::visit(nodetype *node) { format_node(node, __VA_ARGS__); return ast_visitor::visit(node); }

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

  DOTFILE_VISTOR(module_node, "module");
  DOTFILE_VISTOR(class_node, "class", node->name);
  DOTFILE_VISTOR(function_node, "function", node->name);
  DOTFILE_VISTOR(var_node, "variable", node->name)
  DOTFILE_VISTOR(parameter_node, "parameter", node->name);
  DOTFILE_VISTOR(property_node, "property", node->name)
  DOTFILE_VISTOR(binary_operator_node, "binary operatio", node->operation);
  DOTFILE_VISTOR(unary_operator_node, "unary operation", node->operation)
  DOTFILE_VISTOR(member_access_node, "member access");
  DOTFILE_VISTOR(call_node, "call");
  DOTFILE_VISTOR(cast_node, "call");
  DOTFILE_VISTOR(index_node, "index");
  DOTFILE_VISTOR(tuple_expansion_node, "tuple expansion");
  DOTFILE_VISTOR(literal_node, "literal", node->value);
  DOTFILE_VISTOR(tuple_literal_node, "tuple literal");
  DOTFILE_VISTOR(table_literal_node, "table literal");
  DOTFILE_VISTOR(lambda_capture_node, "lambda capture", node->capture_type);
  DOTFILE_VISTOR(lambda_node, "lambda");
  DOTFILE_VISTOR(name_reference_node, "name reference", node->name);
  DOTFILE_VISTOR(goto_node, "goto", node->target_name);
  DOTFILE_VISTOR(label_node, "label", node->name);
  DOTFILE_VISTOR(return_node, "return");
  DOTFILE_VISTOR(break_node, "break", node->count);
  DOTFILE_VISTOR(continue_node, "continue", node->count);
  DOTFILE_VISTOR(if_node, "if");
  DOTFILE_VISTOR(while_node, "while");
  DOTFILE_VISTOR(for_node, "for");
  DOTFILE_VISTOR(import_node, "import", node->name_path);
  DOTFILE_VISTOR(meta_node, "meta");
  DOTFILE_VISTOR(typedef_node, "typedef", node->name);
  DOTFILE_VISTOR(tuple_node, "tuple");
  DOTFILE_VISTOR(delegate_node, "delegate");
  DOTFILE_VISTOR(plain_type_node, "type", node->name);
  DOTFILE_VISTOR(decltype_node, "decltype");
  DOTFILE_VISTOR(type_indirect_node, "indirect", node->indirection_type);
  DOTFILE_VISTOR(type_array_node, "array");
  DOTFILE_VISTOR(type_template_node, "template");
  DOTFILE_VISTOR(qualifier_node, "qualifier", qualifier_types::names[node->qualifier]);
  DOTFILE_VISTOR(attribute_node, "attributes");
  DOTFILE_VISTOR(scope_node, "scope");

  // ---------------------------------------------------------------------------

  void dotfile_visitor::format_node(abstract_node *node, const char *name)
  {
    if (!m_connectTo)
      fprintf(m_file, "\tptr%p [label=\"[%s]\"];\n", node, name);
  }

  void dotfile_visitor::format_node(abstract_node *node, const char *name, const char *str)
  {
    if (!m_connectTo)
      fprintf(m_file, "\tptr%p [label=\"[%s]\\n[%s]\"];\n", node, name, str);
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
