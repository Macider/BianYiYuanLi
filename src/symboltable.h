#pragma once
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <variant>

using namespace std;

typedef variant<int, string> symbol_type;

class SymbolTable : public enable_shared_from_this<SymbolTable> {
   public:
    map<string, shared_ptr<symbol_type>> symbolMap;
    map<string, shared_ptr<symbol_type>> uselessMap;
    shared_ptr<SymbolTable> parent;  // 指向上一层符号表
    SymbolTable() {
        // cout << "Create Symbol Table!" << endl;
        this->parent = nullptr;
    }
    SymbolTable(shared_ptr<SymbolTable>& parent) {
        // cout << "Create Symbol Table!" << endl;
        this->parent = move(parent);  // parent需要更新所以可以直接move
        // parent = shared_from_this();  // 很容易出问题，多检查
    }
    ~SymbolTable() {
        // cout << "delete Symbol Table" << endl;
    }
    bool existSymbol(const string& var_name) {
        // auto now_table = shared_ptr<SymbolTable>(this);
        auto now_table = shared_from_this();
        while (now_table) {
            for (auto iter = now_table->symbolMap.begin(); iter != now_table->symbolMap.end();) {
                shared_ptr<symbol_type> symbol = iter->second;
                if (holds_alternative<string>(*symbol))
                    if (get<string>(*symbol) == var_name)
                        return true;
                ++iter;
            }
            now_table = now_table->parent;
        }
        return false;
    }
    bool existUseless(const string& var_name) {
        // printSymbolTable();
        for (auto iter = uselessMap.begin(); iter != uselessMap.end();) {
            shared_ptr<symbol_type> symbol = iter->second;
            if (holds_alternative<string>(*symbol))
                if (get<string>(*symbol) == var_name)
                    return true;
            ++iter;
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
    void insertSymbol(const string& var_name, string& var_info) {
        // cout << "Insert Symbol" << endl;
        for (auto iter = symbolMap.begin(); iter != symbolMap.end();) {
            shared_ptr<symbol_type> symbol = iter->second;
            if (holds_alternative<string>(*symbol))
                if (get<string>(*symbol) == var_name) {
                    // cout << "Insert SymbolTable Error!" << endl;
                }
            ++iter;
        }
        while (existSymbol(var_info)) {
            var_info += "_";
            // cout << "Change VarInfo" << endl;
        }
        symbol_type tmp_var_info = var_info;  // var_info才是在程序中使用的名字
        auto var = make_shared<symbol_type>(tmp_var_info);
        symbolMap.insert(make_pair(var_name, var));
        // printSymbolTable();
    }
    shared_ptr<symbol_type> getSymbol(const string& var_name) {
        auto now_table = shared_from_this();
        while (now_table) {
            if (now_table->symbolMap.count(var_name))  // 返回最近层的符号
                return (now_table->symbolMap[var_name]);
            now_table = now_table->parent;
        }
        // cout << "Get Symbol Error" << endl;
        return nullptr;
    }
    void printSymbolTable() {
        std::cout << "\nPrinting Symbol Table!" << endl;
        auto now_table = shared_from_this();
        string prefix = "..";
        while (now_table) {
            int map_size = now_table->symbolMap.size();
            std::cout << prefix << "There is " << map_size << " symbols in this map" << endl;
            std::cout << prefix;
            for (auto iter = now_table->symbolMap.begin(); iter != now_table->symbolMap.end();) {
                shared_ptr<symbol_type> symbol = iter->second;
                if (holds_alternative<string>(*symbol))
                    std::cout << get<string>(*symbol);
                ++iter;
            }
            cout << endl;
            now_table = now_table->parent;
            prefix += "..";
        }
        std::cout << "Print Symbol Table Finished!" << endl;

        std::cout << "Printing Useless Table!" << endl;
        int useless_size = uselessMap.size();
        std::cout << "There is " << useless_size << " symbols in useless map" << endl;
        for (auto iter = uselessMap.begin(); iter != uselessMap.end();) {
            shared_ptr<symbol_type> symbol = iter->second;
            if (holds_alternative<string>(*symbol))
                std::cout << get<string>(*symbol);
            ++iter;
        }
        cout << endl;
        std::cout << "Print Useless Table Finished!" << endl;
    }
};

/* 以下 为该文件的注释 */
// 该文件用于存储符号表相关代码
// 提供existSymbol()、getSymbol()、insertSymbol()接口
// 该文件中单独的//cout均用于调试，全字匹配后替换即可
// 表项为pair< string, shared_ptr<symbol_type> >,symbol_type是对variant<int,string>的别名
// parent指针不适合使用unique_ptr,改为shared_ptr.
//      原因：unique_ptr不支持copy, now = now->parent不被允许
// 也不行,shared_ptr<...>(this)会导致double delete,
//      从enable_shared_from_this<...>继承,调用shared_from_this()
// pair.second为指针目的在于get后的修改能反馈在(很可能不是当前层的)符号表中

// 未删除的无用代码
/* class A {
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
}; */
