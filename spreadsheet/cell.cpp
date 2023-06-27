#include <iostream>
#include <string>
#include "common.h"
#include "sheet.h"
#include "cell.h"

class Cell::Impl {
public:
    Impl() = default;
    virtual ~Impl() = default;
    virtual Cell::Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
};

class Cell::EmptyImpl : public Impl {
public:
    EmptyImpl() = default;

public:
    Value GetValue() const override {
        return "";
    }
    std::string GetText() const override {
        return "";
    }
    std::vector<Position> GetReferencedCells() const override {
        return {};
    }
};

class Cell::TextImpl : public Impl {
public:
    TextImpl() = default;
    TextImpl(std::string_view text) : text_(std::string(text)) {}

public:
    Cell::Value GetValue() const override {
        if (text_[0] == ESCAPE_SIGN) {
            return text_.substr(1);
        }
        return text_;
    }
    std::string GetText() const override {
        return text_;
    }
    std::vector<Position> GetReferencedCells() const override {
        return {};
    }

private:
    std::string text_;
};

class Sheet;

class Cell::FormulaImpl : public Impl {
public:
    FormulaImpl() = default;
    FormulaImpl(std::string_view text) : formula_ptr_(ParseFormula(std::string(text))) {}

public:
    Value GetValue() const override {
        if(!formula_ptr_) { throw std::runtime_error("Invalid formula"); }
        auto value = formula_ptr_->Evaluate(sheet_);

        if (std::holds_alternative<double>(value)) {
            return std::get<double>(value);
        }
        else if (std::holds_alternative<FormulaError>(value)) {
            return std::get<FormulaError>(value);
        }
        else {
            throw std::runtime_error("Invalid formula");
        }
    }
    std::string GetText() const override {
        if(!formula_ptr_) { throw std::runtime_error("Invalid formula"); }
        return FORMULA_SIGN + formula_ptr_->GetExpression();
    }
    std::vector<Position> GetReferencedCells() const override {
        return formula_ptr_->GetReferencedCells();
    }

private:
    const SheetInterface& sheet_ = SheetInterfaceSingleton<Sheet>::GetInstance();
    std::unique_ptr<FormulaInterface> formula_ptr_;
};

Cell::Cell(std::string_view text) {
    Set(text);
}

Cell::~Cell() {}

void Cell::Set(std::string_view text) {
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
    }
    else if (text[0] == FORMULA_SIGN && text.size() > 1) {
        impl_ = std::make_unique<FormulaImpl>(text.substr(1));
    }
    else {
        impl_ = std::make_unique<TextImpl>(text);
    }
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    if (cash_) {
        return *cash_;
    }
    else {
        cash_ = impl_->GetValue();
        return *cash_;
    }
}
std::string Cell::GetText() const {
    return impl_->GetText();
}
std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}
void Cell::ClearCash() {
    cash_.reset();
}
