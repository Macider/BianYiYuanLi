#pragma once
#include <memory>
#include<iostream>
using namespace std;

// 所有 AST 的基类
class BaseAST {
   public:
    virtual ~BaseAST() = default;
    virtual void Dump(std::string& str) const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
   public:
    // 用智能指针管理对象
    std::unique_ptr<BaseAST> func_def;
    void Dump(std::string& str) const override {
        func_def->Dump(str);
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
   public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;
    void Dump(std::string& str) const override {
        //std::cout << "fun ";
        //std::cout << "@" << ident;
        //std::cout << "(): ";
        str += "fun @";
        str += ident;
        str += "(): ";
        func_type->Dump(str);
        ///std::cout << " {" << endl;
        //std::cout << "%entry:" << endl;
        str += " {\n";
        str += "%entry:\n";
        block->Dump(str);
        //std::cout << "\n}" << endl;
        str += "\n}\n";
    }
};

class FuncTypeAST : public BaseAST {
    public:
     void Dump(std::string& str) const override {
        //std::cout << "i32";
        str += "i32";
     }
};
class BlockAST : public BaseAST {
    public:
     std::unique_ptr<BaseAST> stmt;
     void Dump(std::string& str) const override {
        stmt->Dump(str);
     }
};
class StmtAST : public BaseAST {
    public:
     int number;
     void Dump(std::string& str) const override {
        //std::cout << "ret ";
        //std::cout << number;
        str += "ret ";
        str += to_string(number);
     }
};
