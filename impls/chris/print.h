#ifndef PRINT_H
#define PRINT_H

#include "ast.h"

#include <iostream>

inline void printTree(std::ostream& out, const TreeNode& node)
{
    if (node.kind == NodeKind::ROOT)
    {
        assert(!node.empty());
        printTree(out, node.children()[0]);
    }
    else if (node.kind == NodeKind::LIST)
    {
        out << "(";
        std::string sep = "";
        for (const auto& child : node.children())
        {
            out << sep;
            printTree(out, child);
            sep = " ";
        }
        out << ")";
    }
    else if (node.kind == NodeKind::VECTOR)
    {
        out << "[";
        std::string sep = "";
        for (const auto& child : node.children())
        {
            out << sep;
            printTree(out, child);
            sep = " ";
        }
        out << "]";
    }
    else if (node.kind == NodeKind::HASHMAP)
    {
        out << "{";
        std::string sep = "";
        for (const auto& child : node.children())
        {
            out << sep;
            printTree(out, child);
            sep = " ";
        }
        out << "}";
    }
    else if (isBool(node))
    {
        out << (node.getBool() ? "true" : "false");
    }
    else if (isNumber(node))
    {
        out << node.getNumber();
    }
    else if (isNil(node))
    {
        out << "nil";
    }
    else if (isFunc(node))
    {
        // TODO - Better names?
        out << "FUNCTION";
    }
    else if (isString(node))
    {
        out << node.string();
    }
    else
    {
        // Strictly speaking redundant
        out << node.symbol();
    }
}

#endif /* PRINT_H */
