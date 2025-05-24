// 64b804de70653b72415b8647897f113a

#pragma once

#include <iostream>
#include <functional>           
#include <iterator>             
#include <list>
#include <optional>
#include <unordered_set>
#include <vector>

#include "ExprTree.h"

namespace traversal {


// You will want to define enough constructs in this header to make it so that
// the traverse function works when passed either a std::list or an ExprTree.
// You *could* add other constructs.
template<typename GraphKind>
struct GraphTraits {
    using Unspecialized = typename GraphKind::UnspecializedError;
};

// std::list<T> specialization
template<typename T>
struct GraphTraits<std::list<T>> {
    using Graph = std::list<T>;
    using Node = typename Graph::iterator;
    using Key = const void*;

    static std::optional<Node>
    getFirst(Graph& l) {
        if (l.empty())  return std::nullopt;
        return l.begin();
    }

    static std::vector<Node>
    getNeighbors(Graph& l, Node it) {
      auto nxt = std::next(it);
      if (nxt != l.end()) return std::vector<Node>{nxt};
      return {};
    }

    static Key
    getKey(Node it) {
      return static_cast<Key>(&*it);
    }
};

// exprtree::ExprTree specialization
template <>
struct GraphTraits<exprtree::ExprTree> {
    using Graph = exprtree::ExprTree;
    using Node = const exprtree::Expression*;
    using Key = const void*;

    static std::optional<Node>
    getFirst(Graph& tree) {
        struct RootVisitor : exprtree::ExprVisitor {
            Node root = nullptr;
            void visitImpl(const exprtree::Literal& L)   override { if (!root) root = &L; }
            void visitImpl(const exprtree::Symbol& S)    override { if (!root) root = &S; }
            void visitImpl(const exprtree::Operation& O) override { if (!root) root = &O; }
        } visit;
        tree.accept(visit);
        if (visit.root) return visit.root;
        return std::nullopt;
    }

    static std::vector<Node>
    getNeighbors(Graph& , Node n) {
        struct NeighborVisitor : exprtree::ExprVisitor {
            std::vector<Node> children;
            void visitImpl(const exprtree::Operation& op) override {
                children.push_back(&op.lhs);
                children.push_back(&op.rhs);
            }
        } nv;
        n->accept(nv);
        return nv.children;
    }

    static Key
    getKey(Node n) {
        return static_cast<Key>(n);
    }
};

// You should feel free to add template arguments if you want to, but the
// invocations in the test suite should remain valid.
template<class GraphKind, class OnNode, class OnEdge, class Traits = GraphTraits<GraphKind>>
void traverse(GraphKind& graph, OnNode onNode, OnEdge onEdge) {
    using Node = typename Traits::Node;
    using Key = typename Traits::Key;

    std::unordered_set<Key> visited;

    std::function<void(Node)> dfs = [&](Node n) {
        Key k = Traits::getKey(n);

        if (visited.count(k)) return;
        visited.insert(k);

        onNode(n);
        for (auto child: Traits::getNeighbors(graph, n)) {
            onEdge(n, child);
            dfs(child);
        }
    };

    if (auto root = Traits::getFirst(graph)) {
        dfs(*root);
    }
}


}

