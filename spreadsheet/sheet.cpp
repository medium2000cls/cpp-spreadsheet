#include "sheet.h"

#include <functional>
#include <iostream>

#include "common.h"

using namespace std::literals;

template<>
SheetInterfaceSingleton<Sheet>* SheetInterfaceSingleton<Sheet>::instance_ptr_ = nullptr;

Sheet::Sheet() {
    SheetInterfaceSingleton<Sheet>::InstanceReset(*this);
}

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }
    std::unique_ptr<CellInterface> new_cell = std::make_unique<Cell>(text);
    std::vector<Position> new_referenced_cells_vector = new_cell->GetReferencedCells();
    std::unordered_set<Position, PositionHasher> referenced_cells(new_referenced_cells_vector.begin(), new_referenced_cells_vector.end());
    std::unordered_set<Position, PositionHasher> checked_by_chain_cells;
    if (!CheckCyclicalDependencies(pos, referenced_cells, checked_by_chain_cells)) {
        throw CircularDependencyException("Circular dependency");
    }
    InvalidationCash(pos);
    if (cells_.count(pos)) { RemoveChild(pos, cells_[pos]->GetReferencedCells()); }
    AddChild(pos, new_referenced_cells_vector);
    ResetReferenced(pos, new_cell.get());
    cells_[pos] = std::move(new_cell);
    SetPrintableSize(pos);
}

void Sheet::SetPrintableSize(Position pos) const {
    if (printable_size_) {
        printable_size_->rows = std::max(printable_size_->rows - 1, pos.row) + 1;
        printable_size_->cols = std::max(printable_size_->cols - 1, pos.col) + 1;
    } else {
        FillPrintableSize();
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }
    if (cells_.count(pos)) { return cells_.at(pos).get(); }
    return nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }
    if (cells_.count(pos)) { return cells_.at(pos).get(); }
    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }
    if (cells_.count(pos)) {
        if (printable_size_->cols <= pos.col + 1 || printable_size_->rows <= pos.row + 1) {
            printable_size_.reset();
        }
        cells_.erase(pos);
    }
}

Size Sheet::GetPrintableSize() const {
    if (!printable_size_) {
        FillPrintableSize();
    }
    return *printable_size_;
}

void Sheet::FillPrintableSize() const {
    Size printable_size;
    for (const auto& cell : cells_) {
        printable_size.rows = std::max(printable_size.rows - 1, cell.first.row) + 1;
        printable_size.cols = std::max(printable_size.cols - 1, cell.first.col) + 1;
    }
    printable_size_.emplace(printable_size);
}

static std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
    std::visit(
        [&](const auto& x) {
            output << x;
        },
        value);
    return output;
}

void Sheet::PrintValues(std::ostream& output) const {
    Size printable_size = GetPrintableSize();
    for (int row = 0; row < printable_size.rows; ++row) {
        bool first_value = true;
        for (int col = 0; col < printable_size.cols; ++col) {
            if (!first_value) { output << '\t'; }
            first_value = false;

            const CellInterface* cell = GetCell({row, col});
            if (cell) {
                output << cell->GetValue();
            } else {
                output << "";
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    Size printable_size = GetPrintableSize();
    for (int row = 0; row < printable_size.rows; ++row) {
        bool first_value = true;
        for (int col = 0; col < printable_size.cols; ++col) {
            if (!first_value) { output << '\t'; }
            first_value = false;

            const CellInterface* cell = GetCell({row, col});
            if (cell) {
                output << cell->GetText();
            } else {
                output << "";
            }
        }
        output << '\n';
    }
}

bool Sheet::CheckCyclicalDependencies(Position tested_pos, const std::unordered_set<Position, PositionHasher>& cells, std::unordered_set<Position, PositionHasher>& checked_cells) {
    if (cells.count(tested_pos)) {
        return false;
    }
    for (const Position& ref_pos : cells) {
        if (checked_cells.count(ref_pos)) {
            continue;
        }
        else if (referenced_cells_.count(ref_pos) && !CheckCyclicalDependencies(tested_pos, referenced_cells_[ref_pos], checked_cells)) {
            return false;
        }
        checked_cells.insert(ref_pos);
    }
    return true;
}

void Sheet::InvalidationCash(Position pos) {
    if (child_cells_.count(pos)) {
        for (const Position& child_pos : child_cells_[pos]) {
            InvalidationCash(child_pos);
        }
    }
    if (cells_.count(pos)) {
        cells_.at(pos)->ClearCash();
    }
}

void Sheet::RemoveChild(Position pos, const std::vector<Position>& cells) {
    for (const Position& ref_pos : cells) {
        if (child_cells_.count(ref_pos)) {
            child_cells_[ref_pos].erase(pos);
        }
    }
}

void Sheet::AddChild(Position pos, const std::vector<Position>& cell) {
    for (const Position& ref_pos : cell) {
        child_cells_[ref_pos].insert(pos);
    }
}

void Sheet::ResetReferenced(Position pos, const CellInterface* cell) {
    std::vector<Position> referenced_cells_vector = cell->GetReferencedCells();
    auto move_begin = std::make_move_iterator(referenced_cells_vector.begin());
    auto move_end = std::make_move_iterator(referenced_cells_vector.end());
    referenced_cells_[pos] = {move_begin, move_end};
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}