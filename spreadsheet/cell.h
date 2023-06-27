#pragma once
#include <optional>
#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
public:
    Cell(std::string_view text);
    ~Cell();

    void Set(std::string_view text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;
    void ClearCash() override;

private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

private:
    std::unique_ptr<Impl> impl_;
    mutable std::optional<Value> cash_;
};

