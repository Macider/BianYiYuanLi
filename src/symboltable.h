#pragma once
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <variant>

using namespace std;

typedef variant<int, double> symbol_type;

class A {
   public:
    map<int, int> AMap;
    shared_ptr<A> parent;  // 指向上一层
    bool existSymbol() {
        auto now = shared_ptr<A>(this);
        while (now) {
            if (AMap.empty()) {
                AMap.insert(make_pair(1, 1));
            }
            // use(now);
            // cout << "now is available" << endl;
            now = now->parent;  // 不可以copy，只能move
        }
        return false;
    }
};

class SymbolTable : public enable_shared_from_this<SymbolTable> {
   public:
    map<string, shared_ptr<symbol_type>> symbolMap;
    shared_ptr<SymbolTable> parent;  // 指向上一层符号表
    SymbolTable() {
        // cout << "Create Symbol Table!" << endl;
        this->parent = nullptr;
    }
    SymbolTable(shared_ptr<SymbolTable>& parent) {
        // cout << "Create Symbol Table!" << endl;
        this->symbolMap.clear();
        this->parent = move(parent);             // parent需要更新所以可以直接move
        parent = shared_ptr<SymbolTable>(this);  // 很容易出问题，多检查
    }
    ~SymbolTable() {
        // cout << "delete Symbol Table" << endl;
    }
    bool existSymbol(const string& var_name) {
        // auto now_table = shared_ptr<SymbolTable>(this);
        auto now_table = shared_from_this();
        while (now_table) {
            if (now_table->symbolMap.count(var_name))
                return true;
            now_table = now_table->parent;
        }
        return false;
    }
    void insertSymbol(const string& var_name, int var_info) {
        if (symbolMap.count(var_name)) {
            // cout << "Insert SymbolTable Error!" << endl;
            return;
        }
        symbol_type tmp_var_info = var_info;
        auto var = make_shared<symbol_type>(tmp_var_info);
        symbolMap.insert(make_pair(var_name, var));
    }
    void insertSymbol(const string& var_name, const double& var_info) {
        if (symbolMap.count(var_name)) {
            // cout << "Insert SymbolTable Error!" << endl;
            return;
        }
        // int map_size = symbolMap.size();
        symbol_type tmp_var_info = var_info;
        auto var = make_shared<symbol_type>(tmp_var_info);
        symbolMap.insert(make_pair(var_name, var));
        // cout << "Original Size is " << map_size << " while New Size is" << symbolMap.size() << endl;
    }
    shared_ptr<symbol_type> getSymbol(const string& var_name) {
        auto now_table = shared_from_this();
        while (now_table) {
            if (now_table->symbolMap.count(var_name))
                return (now_table->symbolMap[var_name]);
            now_table = now_table->parent;
        }
        // cout << "Get Symbol Error" << endl;
        return nullptr;
    }
    void printSymbolTable() {
        std::cout << "\nPrinting Symbol Table!" << endl;
        auto now_table = shared_from_this();
        while (now_table) {
            int map_size = now_table->symbolMap.size();
            std::cout << "There is " << map_size << " symbols in this map" << endl;
            now_table = now_table->parent;
        }
        std::cout << "Print Symbol Table Finished!" << endl;
    }
};

/* 以下 为该文件的注释 */
// 该文件用于存储符号表相关代码
// 提供existSymbol()、getSymbol()、insertSymbol()接口
// 该文件中单独的//cout均用于调试，全字匹配后替换即可
// 表项为pair< string, shared_ptr<symbol_type> >,symbol_type是对variant<int,?>的别名
// parent指针不适合使用unique_ptr,改为shared_ptr.
//      原因：unique_ptr不支持copy, now = now->parent不被允许
// 也不行,shared_ptr<...>(this)会导致double delete,
//      从enable_shared_from_this<...>继承,调用shared_from_this()
// pair.second为指针目的在于get后的修改能反馈在(很可能不是当前层的)符号表中
