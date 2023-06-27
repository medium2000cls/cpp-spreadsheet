#pragma once

#include <forward_list>
#include <functional>
#include <stdexcept>
#include "FormulaLexer.h"
#include "common.h"

namespace ASTImpl {
class Expr;
}

class ParsingError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class FormulaAST {
public:
    explicit FormulaAST(std::unique_ptr<ASTImpl::Expr> root_expr,
                        std::forward_list<Position> cells);
    FormulaAST(FormulaAST&&) = default;
    FormulaAST& operator=(FormulaAST&&) = default;
    ~FormulaAST();
public:
    double Execute(std::function<double(Position)> callback_func) const;

    void Print(std::ostream& out) const;
    void PrintFormula(std::ostream& out) const;

    std::set<Position>& GetCells() {
        return cells_;
    }

    const std::set<Position>& GetCells() const {
        return cells_;
    }

private:
    std::unique_ptr<ASTImpl::Expr> root_expr_;
    std::set<Position> cells_;
};

FormulaAST ParseFormulaAST(std::istream& in);
FormulaAST ParseFormulaAST(std::string_view in_str);
