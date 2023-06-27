#pragma once

#include <functional>
#include <optional>

#include "cell.h"
#include "common.h"

class Sheet : public SheetInterface {
public:
    Sheet();
    ~Sheet();
    void SetCell(Position pos, std::string text) override;
    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;
    void ClearCell(Position pos) override;
    Size GetPrintableSize() const override;
    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;


private:
    std::unordered_map<Position, std::unique_ptr<CellInterface>, PositionHasher> cells_;
    std::unordered_map<Position, std::unordered_set<Position, PositionHasher>, PositionHasher> child_cells_;
    std::unordered_map<Position, std::unordered_set<Position, PositionHasher>, PositionHasher> referenced_cells_;
    mutable std::optional<Size>   printable_size_;

private:
    bool CheckCyclicalDependencies(Position tested_pos, const std::unordered_set<Position, PositionHasher>& cells, std::unordered_set<Position, PositionHasher>& checked_cells);
    void InvalidationCash(Position pos);
    void RemoveChild(Position pos, const std::vector<Position>& cells);
    void AddChild(Position pos, const std::vector<Position>& cell);
    void ResetReferenced(Position pos, const CellInterface* cell = nullptr);
    void FillPrintableSize() const;
    void SetPrintableSize(Position pos) const;
};


