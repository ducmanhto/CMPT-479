// 64b804de70653b72415b8647897f113a

#include "ExprTree.h"
#include "ExprOps.h"
#include <cassert>

// You will want to implement these functions within this file.

using exprtree::Environment;
using exprtree::ExprVisitor;
using exprtree::Expression;
using exprtree::Literal;
using exprtree::Operation;
using exprtree::OpCode;
using exprtree::Symbol;

namespace exprtree {


std::optional<int64_t>
evaluate(const ExprTree& tree, const Environment& environment) {
  struct EvalVisitor : ExprVisitor {
    const Environment& env;
    std::optional<int64_t> result;
    
    EvalVisitor(const Environment& e) : env(e) {}

  private:
    void visitImpl(const Literal& lit) override {
      result = lit.value;
    }
    void visitImpl(const Symbol& sym) override {
      result = env.get(sym.name);
    }
    void visitImpl(const Operation& op) override {
      // evaluate children
      EvalVisitor L(env), R(env);
      op.lhs.accept(L);
      op.rhs.accept(R);

      if (!L.result || !R.result) {
        result = std::nullopt;
        return;
      }

      int64_t a = *L.result;
      int64_t b = *R.result;

      switch (op.opCode) {
        case ADD:
          result = a + b;
          break;
        case SUBTRACT:
          result = a - b;
          break;
        case MULTIPLY:
          result = a * b;
          break;
        case DIVIDE:
          if (b == 0)
            result = std::nullopt;
          else
            result = a / b;
          break;
        default:
          assert(false && "unknown OpCode");
      }
    }
  };

  EvalVisitor ev(environment);
  tree.accept(ev);
  return ev.result;
}


std::unordered_map<std::string,size_t>
countSymbols(const ExprTree& tree) {
  std::unordered_map<std::string,size_t> count;

  struct SymbolCount : ExprVisitor {
    std::unordered_map<std::string,size_t>& count;
    SymbolCount(decltype(count)& m) : count(m) {}

    private:
      void visitImpl(const Literal&) override {}
      void visitImpl(const Symbol& sym) override {
        ++count[sym.name];
      }
      void visitImpl(const Operation& op) override {
        op.lhs.accept(*this);
        op.rhs.accept(*this);
      }
  };
  
  SymbolCount sc(count);
  tree.accept(sc);
  return count;
}


std::unordered_map<OpCode,size_t>
countOps(const ExprTree& tree) {
  std::unordered_map<OpCode,size_t> count;

  struct OperationCount : ExprVisitor {
    std::unordered_map<OpCode,size_t>& count;
    OperationCount(decltype(count)& m) : count(m){}

    private:
      void visitImpl(const Literal&) override {}
      void visitImpl(const Symbol&) override {}
      void visitImpl(const Operation& op) override {
        ++count[op.opCode];
        op.lhs.accept(*this);
        op.rhs.accept(*this);
      }
  };
  
  OperationCount oc(count);
  tree.accept(oc);
  return count;
}


}
