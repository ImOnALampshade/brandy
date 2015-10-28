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

#define WALK_CHILDREN(connecTo) do { m_connectTo = &connecTo; walk_node(node, this, false); m_connectTo = nullptr; walk_node(node, this, false); } while(false);

  // ---------------------------------------------------------------------------

  dotfile_visitor::dotfile_visitor(FILE *f) : m_file(f), m_connectTo(nullptr)
  {
  }

  ast_visitor::visitor_result dotfile_visitor::visit(module_node *node)
  {
    std::string module = "module";
    WALK_CHILDREN(module);

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(class_node *node)
  {
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> class_", m_connectTo->c_str());
      token_to_file(m_file, node->name);
      fputs(";\n", m_file);
    }
    else
    {
      fprintf(m_file, "class_");
      token_to_file(m_file, node->name);
      fputs(" [shape=box,label=\"class\\n", m_file);
      token_to_file(m_file, node->name);
      fputs("\"];\n", m_file);

      std::string name = "class_";
      name.append(node->name.text(), node->name.length());
      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(function_node *node)
  {
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> function_", m_connectTo->c_str());
      token_to_file(m_file, node->name);
      fputs(";\n", m_file);
    }
    else
    {
      fprintf(m_file, "  function_");
      token_to_file(m_file, node->name);
      fputs(" [shape=box,label=\"function\\n", m_file);
      token_to_file(m_file, node->name);
      fputs("\"];\n", m_file);

      std::string function = "function_";
      function.append(node->name.text(), node->name.length());
      sanatize(function);
      WALK_CHILDREN(function);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(var_node *node)
  {
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> var_", m_connectTo->c_str());
      token_to_file(m_file, node->name);
      fputs(";\n", m_file);
    }
    else
    {
      fprintf(m_file, "  var_");
      token_to_file(m_file, node->name);
      fputs(" [shape=box,label=\"variable\\n", m_file);
      token_to_file(m_file, node->name);
      fputs("\"];\n", m_file);

      std::string function = "var_";
      function.append(node->name.text(), node->name.length());
      WALK_CHILDREN(function);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(parameter_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> parameter_%p;\n", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("parameter", ptr);

      fprintf(m_file, "  %s [shape=box,label=\"parameter\\n", name.c_str());
      token_to_file(m_file, node->name);
      fputs("\"];\n", m_file);

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(property_node *node)
  {
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> property_", m_connectTo->c_str());
      token_to_file(m_file, node->name);
      fputs(";\n", m_file);
    }
    else
    {
      fprintf(m_file, "  property_");
      token_to_file(m_file, node->name);
      fputs(" [shape=box,label=\"property\\n", m_file);
      token_to_file(m_file, node->name);
      fputs("\"];\n", m_file);

      std::string function = "property_";
      function.append(node->name.text(), node->name.length());
      WALK_CHILDREN(function);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(binary_operator_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> binaryop_%p;\n", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("binaryop", ptr);

      fprintf(m_file, "  %s [shape=box,label=\"operator ", name.c_str());
      token_to_file(m_file, node->operation);
      fputs("\"];\n", m_file);

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(unary_operator_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> unaryop_%p;\n", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("unaryop", ptr);

      fprintf(m_file, "  %s [shape=box,label=\"operator ", name.c_str());
      token_to_file(m_file, node->operation);
      fputs("\"];\n", m_file);

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(member_access_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> memberaccess_%p;\n", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("memberaccess", ptr);

      fprintf(m_file, "  %s [shape=box,label=\"member access\\n", name.c_str());
      token_to_file(m_file, node->member_name);
      fputs("\"];\n", m_file);

      WALK_CHILDREN(name);
    }
    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(call_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> call_%p", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("call", ptr);

      fprintf(m_file, "  %s [shape=box,label=\"call\"];\n", name.c_str());

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(cast_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> cast_%p;\n", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("cast", ptr);

      fprintf(m_file, "  %s [shape=box,label=\"cast\"];\n", name.c_str());

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(index_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> index_%p;\n", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("index", ptr);

      fprintf(m_file, "  %s [shape=box,label=\"index\"];\n", name.c_str());

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(literal_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> literal_%p;\n", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("literal", ptr);

      fprintf(m_file, "  %s [shape=box,label=\"literal\\n", name.c_str());
      token_to_file(m_file, node->value);
      fputs("\"];\n", m_file);

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }
  
  ast_visitor::visitor_result dotfile_visitor::visit(lambda_capture_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> lambdacapture_%p;\n", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("lambdacapture", ptr);
      fprintf(m_file, "  %s [shape=box,label=\"lambda capture\\n", name.c_str());
      token_to_file(m_file, node->capture_type);
      fputs("\"];\n", m_file);

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(lambda_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> lambda_%p;\n", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("lambda", ptr);
      fprintf(m_file, "  %s [shape=box,label=\"lambda\"];\n", name.c_str());

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(name_reference_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> nameref_%p;\n", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("nameref", ptr);

      fprintf(m_file, "  %s [shape=box,label=\"name reference\\n", name.c_str());
      token_to_file(m_file, node->name);
      fputs("\"];\n", m_file);

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(goto_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> goto_%p;\n", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("goto", ptr);

      fprintf(m_file, "  %s [shape=box,label=\"goto\\n", name.c_str());
      token_to_file(m_file, node->target_name);
      fputs("\"];\n", m_file);

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(label_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> label_%p;\n", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("label", ptr);

      fprintf(m_file, "  %s [shape=box,label=\"label\\n", name.c_str());
      token_to_file(m_file, node->name);
      fputs("\"];\n", m_file);

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(return_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> return_%p;\n", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("return", ptr);

      fprintf(m_file, "  %s [shape=box,label=\"return\"];\n", name.c_str());

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(break_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> break_%p;\n", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("break", ptr);

      if (node->count.text())
      {
        fprintf(m_file, "  %s [shape=box,label=\"break\\n", name.c_str());
        token_to_file(m_file, node->count);
        fputs("\"];\n", m_file);
      }
      else
      {
        fprintf(m_file, "  %s [shape=box,label=\"break\"];\n", name.c_str());
      }

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(continue_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> continue_%p;\n", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("continue", ptr);

      if (node->count.text())
      {
        fprintf(m_file, "  %s [shape=box,label=\"continue\\n", name.c_str());
        token_to_file(m_file, node->count);
        fputs("\"];\n", m_file);
      }
      else
      {
        fprintf(m_file, "  %s [shape=box,label=\"continue\"];\n", name.c_str());
      }

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(if_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> if_%p", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("if", ptr);

      fprintf(m_file, "  %s [shape=box,label=\"if\"];\n", name.c_str());

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(while_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> while_%p", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("while", ptr);

      fprintf(m_file, "  %s [shape=box,label=\"while\"];\n", name.c_str());

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(for_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> for_%p", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("for", ptr);

      fprintf(m_file, "  %s [shape=box,label=\"for\"];\n", name.c_str());

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(import_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> import_%p", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("import", ptr);

      fprintf(m_file, "  %s [shape=box,label=\"import\\n", name.c_str());
      bool first = true;

      for (const token &tok : node->name_path)
      {
        if (!first) fputc('.', m_file);
        token_to_file(m_file, tok);
        first = false;
      }

      fputs("\"];\n", m_file);

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(type_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> type_%p", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("type", ptr);

      fprintf(m_file, "  %s [shape=box,label=\"type\\n", name.c_str());
      bool first = true;

      for (const token &tok : node->name)
      {
        if (!first) fputc('.', m_file);
        token_to_file(m_file, tok);
        first = false;
      }

      fputs("\"];\n", m_file);

      WALK_CHILDREN(name);
    }
    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(qualifier_node *node) 
  { 
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> type_%p", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("type", ptr);

      fprintf(m_file, "  %s [shape=box,label=\"qualifier\\n", name.c_str());
      fputs(qualifier_types::names[node->qualifier], m_file);
      fputs("\"];\n", m_file);

      WALK_CHILDREN(name);
    }
    return ast_visitor::stop;
  }
  
  ast_visitor::visitor_result dotfile_visitor::visit(attribute_node *node) 
  { 
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> attr_%p", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = make_name("attr", ptr);

      fprintf(m_file, "  %s [shape=box,label=\"attributes\"];\n", name.c_str());

      WALK_CHILDREN(name);
    }
    return ast_visitor::stop;
  }

  ast_visitor::visitor_result dotfile_visitor::visit(scope_node *node)
  {
    auto ptr = (void *)node;
    if (m_connectTo)
    {
      fprintf(m_file, "  %s -> scope_%p\n", m_connectTo->c_str(), ptr);
    }
    else
    {
      std::string name = "scope_";
      char buffer[16];
      sprintf(buffer, "%p", ptr);
      name += buffer;

      fprintf(m_file, "  %s [shape=box,label=\"scope\"];\n", name.c_str());

      WALK_CHILDREN(name);
    }

    return ast_visitor::stop;
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------
