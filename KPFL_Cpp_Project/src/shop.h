// shop.h - Product (abstract), Merchandise, FoodItem, Receipt, StadiumShop.
#pragma once
#include <map>
#include <memory>
#include "common.h"

namespace kpfl {

// ===================== PRODUCT (ABSTRACT) =====================
class Product {
    string id, name;
    double price;   // per item, includes 15% GCT
    int stock;
public:
    Product(const string& id_, const string& name_, double price_, int stock_) {
        id = toUpper(cleanText(id_, "Product ID"));
        name = cleanText(name_, "Product name");
        if (price_ < 0) throw InvalidDataException("Price cannot be negative.");
        if (stock_ < 0) throw InvalidDataException("Stock cannot be negative.");
        price = price_; stock = stock_;
    }
    virtual ~Product() = default;

    // POLYMORPHISM: each product type reports its own category / file label.
    virtual string getCategory() const = 0;
    virtual string describe() const {
        return padRight(id, 6) + " " + padRight(name, 26) + " " + padRight(getCategory(), 12) + " $" +
               padLeft(money(price), 9) + "  stock " + to_string(stock);
    }
    virtual string saveExtra() const = 0;     // type-specific field written to the data file
    virtual char typeCode() const = 0;             // 'M' or 'F'

    void addStock(int q) {
        if (q <= 0) throw InvalidDataException("Restock quantity must be positive.");
        stock += q;
    }
    void removeStock(int q) {
        if (q <= 0) throw InvalidDataException("Quantity must be positive.");
        if (q > stock) throw InvalidDataException("Not enough stock of " + name + " (have " + to_string(stock) + ").");
        stock -= q;
    }
    const string& getId() const { return id; }
    const string& getName() const { return name; }
    double getPrice() const { return price; }
    int getStock() const { return stock; }
};

class Merchandise : public Product {
    string size;
public:
    Merchandise(const string& id, const string& name, double price, int stock, const string& size_)
        : Product(id, name, price, stock), size(trim(size_).empty() ? "One size" : cleanText(size_, "Size")) {}
    string getCategory() const override { return "Merchandise"; }
    string describe() const override { return Product::describe() + " (size " + size + ")"; }
    string saveExtra() const override { return size; }
    char typeCode() const override { return 'M'; }
};

class FoodItem : public Product {
    bool beverage;
public:
    FoodItem(const string& id, const string& name, double price, int stock, bool isBeverage)
        : Product(id, name, price, stock), beverage(isBeverage) {}
    string getCategory() const override { return beverage ? "Beverage" : "Food"; }
    string saveExtra() const override { return beverage ? "1" : "0"; }
    char typeCode() const override { return 'F'; }
};

// ===================== RECEIPT =====================
// All prices INCLUDE 15% GCT, so: subtotal = total / 1.15 and tax = total - subtotal.
class Receipt {
    vector<string> lines;
    double total = 0, subtotal = 0, tax = 0, payment = 0, change = 0;
public:
    static constexpr double GCT_RATE = 0.15;

    void addLine(const string& description, int quantity, double unitPrice) {
        double lineTotal = quantity * unitPrice;
        total += lineTotal;
        lines.push_back(padRight(description, 28) + " x" + padRight(to_string(quantity), 3) + " @ $" +
                        padLeft(money(unitPrice), 9) + " = $" + padLeft(money(lineTotal), 10));
    }
    // RULE: payment must cover the total.
    void finish(double pay) {
        if (pay < 0) throw InvalidDataException("Payment cannot be negative.");
        total = roundMoney(total);
        if (pay < total)
            throw InsufficientPaymentException("Payment of $" + money(pay) + " is not enough. Total due is $" + money(total) + ".");
        payment = roundMoney(pay);
        subtotal = roundMoney(total / (1 + GCT_RATE));
        tax = roundMoney(total - subtotal);
        change = roundMoney(pay - total);
    }
    double getTotal() const { return total; }
    double getSubtotal() const { return subtotal; }
    double getTax() const { return tax; }
    double getChange() const { return change; }
    string print() const {
        string s = "----------- RECEIPT -----------\n";
        for (const auto& l : lines) s += l + "\n";
        s += "Subtotal (before GCT): $" + money(subtotal) + "\n";
        s += "GCT (15%):             $" + money(tax) + "\n";
        s += "TOTAL:                 $" + money(total) + "\n";
        s += "Paid:                  $" + money(payment) + "\n";
        s += "Change:                $" + money(change) + "\n";
        return s;
    }
};

// ===================== STADIUM SHOP =====================
class StadiumShop {
    map<string, shared_ptr<Product>> inventory;
    double totalSales = 0;
    int transactionCount = 0;
public:
    void addProduct(const shared_ptr<Product>& p) {
        if (!p) throw InvalidDataException("Product cannot be null.");
        if (inventory.count(p->getId())) throw InvalidDataException("Product ID " + p->getId() + " already exists.");
        inventory[p->getId()] = p;
    }
    shared_ptr<Product> findProduct(const string& id) const {
        auto it = inventory.find(toUpper(trim(id)));
        return it == inventory.end() ? nullptr : it->second;
    }
    void restock(const string& id, int qty) {
        auto p = findProduct(id);
        if (!p) throw InvalidDataException("Unknown product " + id);
        p->addStock(qty);
    }
    // Order: 1) validate everything  2) build receipt (checks payment)  3) only then reduce stock.
    // So a failed sale never changes the inventory.
    Receipt purchase(const map<string, int>& cart, double payment) {
        if (cart.empty()) throw InvalidDataException("The cart is empty.");
        Receipt receipt;
        for (const auto& item : cart) {
            auto p = findProduct(item.first);
            if (!p) throw InvalidDataException("Unknown product " + item.first);
            if (item.second <= 0) throw InvalidDataException("Quantity must be positive for " + p->getName());
            if (item.second > p->getStock())
                throw InvalidDataException("Not enough stock of " + p->getName() + " (have " + to_string(p->getStock()) + ").");
            receipt.addLine(p->getName(), item.second, p->getPrice());
        }
        receipt.finish(payment);
        for (const auto& item : cart) findProduct(item.first)->removeStock(item.second);
        totalSales += receipt.getTotal();
        transactionCount++;
        return receipt;
    }
    void restoreTotals(double sales, int count) { totalSales = sales; transactionCount = count; }
    vector<shared_ptr<Product>> getProducts() const {
        vector<shared_ptr<Product>> v;
        for (const auto& kv : inventory) v.push_back(kv.second);
        return v;
    }
    double getTotalSales() const { return totalSales; }
    int getTransactionCount() const { return transactionCount; }
};

}  // namespace kpfl
