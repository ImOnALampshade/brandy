// -----------------------------------------------------------------------------
// Brandy ast visitor to generate AST dotfile
// Howard Hughes
// -----------------------------------------------------------------------------

#include "dotfile_visitor.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  dotfile_visitor::dotfile_visitor(const char *fileLocation) :
    m_file(fopen(fileLocation, "wt")),
    m_connectTo(nullptr)
  {
    fputs(
      "strict digraph {\n"
      "\tgraph [dpi=300];\n"
      "\tnode [shape=plaintext fontname=\"Source Code Pro\" fontsize=12];\n",
      m_file
    );
  }

  dotfile_visitor::~dotfile_visitor()
  {
    fputs("}\n", m_file);
    fclose(m_file);
  }

#define DOTFILE_VISTOR(nodetype, ...)                                 \
  ast_visitor::visitor_result dotfile_visitor::visit(nodetype *node)  \
  {                                                                   \
    format_node(node, __VA_ARGS__);                                   \
    return ast_visitor::visit(node);                                  \
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

  // ---------------------------------------------------------------------------

  DOTFILE_VISTOR(module_node, "module");
  DOTFILE_VISTOR(class_node, "class", node->name);
  DOTFILE_VISTOR(function_node, "function", node->name);
  DOTFILE_VISTOR(parameter_node, "parameter", node->name);
  DOTFILE_VISTOR(property_node, "property", node->name);
  DOTFILE_VISTOR(var_node, "var", node->name);
  DOTFILE_VISTOR(binary_operator_node, "binary operation", node->operation);
  DOTFILE_VISTOR(unary_operator_node, "unary operation", node->operation);
  DOTFILE_VISTOR(absolute_value_node, "absolute value");
  DOTFILE_VISTOR(member_access_node, "member access", node->member_name);
  DOTFILE_VISTOR(call_node, "call");
  DOTFILE_VISTOR(cast_node, "cast");
  DOTFILE_VISTOR(index_node, "index");
  DOTFILE_VISTOR(literal_node, "literal", node->value);
  DOTFILE_VISTOR(lambda_node, "lambda");
  DOTFILE_VISTOR(name_reference_node, "name reference", node->name);
  DOTFILE_VISTOR(goto_node, "goto", node->target);
  DOTFILE_VISTOR(label_node, "label", node->name);
  DOTFILE_VISTOR(return_node, "return");
  DOTFILE_VISTOR(break_node, "break", node->break_to_label);
  DOTFILE_VISTOR(if_node, "if");
  DOTFILE_VISTOR(while_node, "while");
  DOTFILE_VISTOR(for_node, "for", node->names, ',');
  DOTFILE_VISTOR(import_node, "import", node->path);
  DOTFILE_VISTOR(meta_block_node, "meta");
  DOTFILE_VISTOR(attribute_node, "attribute");
  DOTFILE_VISTOR(type_node, "type", node->name);
  DOTFILE_VISTOR(scope_node, "scope");

  // ---------------------------------------------------------------------------

  void dotfile_visitor::format_node(
    abstract_node *node,
    const char *name
  )
  {
    if (!m_connectTo)
      fprintf(m_file, "\tptr%p [label=\"[%s]\"];\n", node, name);
  }

  void dotfile_visitor::format_node(
    abstract_node *node,
    const char *name,
    const char *str
  )
  {
    if (!m_connectTo)
      fprintf(m_file, "\tptr%p [label=\"[%s]\\n[%s]\"];\n", node, name, str);
  }

  void dotfile_visitor::format_node(
    abstract_node *node,
    const char *name,
    const token &tok
  )
  {
    if (!m_connectTo)
    {
      if (tok.type() == token_types::_STRING)
        fprintf(m_file,
          "\tptr%p [label=\"[%s]\\n[%s]\\n\\\"%.*s\\\"\"];\n",
          node,
          name,
          token_types::names[tok.type()],
          (int) (tok.length() - 2),
          tok.text() + 1
        );
      else if (tok.text())
        fprintf(m_file,
          "\tptr%p [label=\"[%s]\\n[%s]\\n%.*s\"];\n",
          node,
          name,
          token_types::names[tok.type()],
          (int) (tok.length()),
          tok.text()
        );
      else
        format_node(node, name);
    }
  }

  void dotfile_visitor::format_node(
    abstract_node *node,
    const char *name,
    const std::vector<token> &tokens,
    char delimiter
  )
  {
    if (!m_connectTo)
    {
      fprintf(m_file, "\tptr%p [label=\"[%s]\\n", node, name);

      bool first = true;
      for (const token &tok : tokens)
      {
        if (first)
          fprintf(m_file,
            "%.*s",
            (int) (tok.length()),
            tok.text()
          );
        else
          fprintf(m_file,
            "%c%.*s",
            delimiter,
            (int) (tok.length()),
            tok.text()
          );
        first = false;
      }

      fputs("\"];\n", m_file);
    }
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
