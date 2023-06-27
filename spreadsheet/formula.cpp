#include "formula.h"

#include <algorithm>
#include <sstream>
#include "FormulaAST.h"

using namespace std::literals;

namespace {
struct CellValueVisitor {
    double operator()(double value) const {
        return value;
    }
    double operator()(const std::string& value) const {
        try {
            if (value.empty()) {
                return 0;
            }
            size_t pos = 0;
            double cell_double = std::stod(value,&pos);
            if (pos == value.size()) {
                return cell_double;
            }
            else {
                throw FormulaError(FormulaError::Category::Value);
            }
        }
        catch (...) {
            throw FormulaError(FormulaError::Category::Value);
        }
    }
    double operator()(const FormulaError& exc) {
        throw exc;
    }
};

class Formula : public FormulaInterface {
public:
    explicit Formula(std::string_view expression) try : ast_(ParseFormulaAST(expression)) {}
    catch (const std::exception& exc) {
        std::throw_with_nested(FormulaException(exc.what()));
    }

    Value Evaluate(const SheetInterface& sheet) const override {
        try {
            auto callback_cell = [&sheet](Position pos) -> double {
                const CellInterface* cell = sheet.GetCell(pos);
                if (cell) {
                    return std::visit(CellValueVisitor(), cell->GetValue());
                }
                return 0;
            };
            return ast_.Execute(callback_cell);
        } catch (const FormulaError& exc) {
            return exc;
        }
    }

    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        return out.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        const std::set<Position>& cells = ast_.GetCells();
        return {cells.begin(), cells.end()};
    }

private:
    FormulaAST ast_;
};
}// namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(expression);
}