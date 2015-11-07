// -----------------------------------------------------------------------------
// Brandy language symbol table filler
// Howard Hughes
// -----------------------------------------------------------------------------

#include "symbolfillervisitor.h"

// -----------------------------------------------------------------------------

namespace brandy
{
  // ---------------------------------------------------------------------------

  ast_visitor::visitor_result symbol_table_filler_visitor::visit(module_node *node)
  {
    ast_visitor::visit(node);

    m_symStack.push_back(&node->symbols);
    walk_node(node, this, false);
    m_symStack.pop_back();
    return ast_visitor::stop;
  }

  ast_visitor::visitor_result symbol_table_filler_visitor::visit(class_node *node)
  {
    ast_visitor::visit(node);

    insert_node(node, node->name, symbol::type_name);

    m_symStack.push_back(&node->symbols);
    walk_node(node, this, false);
    m_symStack.pop_back();
    return ast_visitor::stop;
  }

  ast_visitor::visitor_result symbol_table_filler_visitor::visit(scope_node *node)
  {
    ast_visitor::visit(node);

    m_symStack.push_back(&node->symbols);
    walk_node(node, this, false);
    m_symStack.pop_back();
    return ast_visitor::stop;
  }

  // ---------------------------------------------------------------------------

  ast_visitor::visitor_result symbol_table_filler_visitor::visit(lambda_node *node)
  {
    ast_visitor::visit(node);

    m_symStack.push_back(&node->scope->symbols);

    for (auto &param : node->parameters)
      insert_node(param.get(), param->name, symbol::variable);

    walk_node(node->scope, this, false);

    m_symStack.pop_back();

    return ast_visitor::stop;
  }

  // ---------------------------------------------------------------------------

  ast_visitor::visitor_result symbol_table_filler_visitor::visit(var_node *node)
  {
    ast_visitor::visit(node);
    insert_node(node, node->name, symbol::variable);
    return ast_visitor::resume;
  }

  ast_visitor::visitor_result symbol_table_filler_visitor::visit(function_node *node)
  {
    ast_visitor::visit(node);
    insert_node(node, node->name, symbol::function);

    m_symStack.push_back(&node->scope->symbols);

    for (auto &param : node->parameters)
      insert_node(param.get(), param->name, symbol::variable);

    walk_node(node->scope, this, false);

    m_symStack.pop_back();

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result symbol_table_filler_visitor::visit(label_node *node)
  {
    ast_visitor::visit(node);
    insert_node(node, node->name, symbol::label);
    return ast_visitor::resume;
  }

  ast_visitor::visitor_result symbol_table_filler_visitor::visit(property_node *node)
  {
    ast_visitor::visit(node);
    insert_node(node, node->name, symbol::property);

    if (node->getter)
    {
      m_symStack.push_back(&node->getter->symbols);
      walk_node(node->getter, this, false);
      m_symStack.pop_back();
    }

    if (node->setter)
    {
      m_symStack.push_back(&node->setter->symbols);
      if (node->setter_value)
        insert_node(node->setter_value.get(), node->setter_value->name, symbol::variable);
      else
        insert_node(nullptr, token("value", 5, token_types::IDENTIFIER), symbol::variable);
      m_symStack.pop_back();
    }

    return ast_visitor::stop;
  }

  ast_visitor::visitor_result symbol_table_filler_visitor::visit(import_node *node)
  {
    ast_visitor::visit(node);
    insert_node(node, node->effective_name, symbol::import);
    return ast_visitor::resume;
  }

  ast_visitor::visitor_result symbol_table_filler_visitor::visit(typedef_node *node)
  {
    ast_visitor::visit(node);
    insert_node(node, node->name, symbol::typedef_name);
    return ast_visitor::resume;
  }

  // ---------------------------------------------------------------------------

  ast_visitor::visitor_result symbol_table_filler_visitor::visit(binary_operator_node *node)
  {
    ast_visitor::visit(node);

    // Only check on assignment operations
    if (node->operation.type() != token_types::ASSIGNMENT)
      return ast_visitor::resume;

    // If the left hand side of the assignment is a name reference
    if (auto nameRef = dynamic_cast<name_reference_node *>(node->left.get()))
    {
      for (auto table : m_symStack)
      {
        // If this table has the name we're looking for in it, then return (Was declared earlier)
        if (table->find(nameRef->name) != table->end())
          return ast_visitor::resume;
      }

      // Didn't find it, add it (implicit declaration)
      auto pair = std::make_pair(nameRef->name, symbol(nameRef->name, symbol::variable, nameRef));
      pair.second.is_implicit = true;
      m_symStack.back()->insert(pair);
    }

    return ast_visitor::resume;
  }

  // ---------------------------------------------------------------------------

  void symbol_table_filler_visitor::insert_node(abstract_node *node, const token &name, symbol::kind type)
  {
    auto &table = *m_symStack.back();
    auto found = table.find(name);

    if (found == table.end())
    {
      auto pair = std::make_pair(name, symbol(name, type, node));
      table.insert(pair);
    }
    else
    {
      // TODO: Emit name error, variable redefinition in same scope
    }
  }

  // ---------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

