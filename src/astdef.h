#pragma once
#include <memory>
#include<iostream>
using namespace std;

// 所有 AST 的基类
class BaseAST {
   public:
    virtual ~BaseAST() = default;
    virtual void Dump() const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
   public:
    // 用智能指针管理对象
    std::unique_ptr<BaseAST> func_def;
    void Dump() const override {
        func_def->Dump();
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
   public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;
    void Dump() const override {
        std::cout << "fun ";
        std::cout << "@" << ident;
        std::cout << "(): ";
        func_type->Dump();
        std::cout << " {" << endl;
        std::cout << "%entry:" << endl;
        block->Dump();
        std::cout << "\n}" << endl;
    }
};

class FuncTypeAST : public BaseAST {
    public:
     void Dump() const override {
        std::cout << "i32";
     }
};
class BlockAST : public BaseAST {
    public:
     std::unique_ptr<BaseAST> stmt;
     void Dump() const override {
        stmt->Dump();
     }
};
class StmtAST : public BaseAST {
    public:
     int number;
     void Dump() const override {
        std::cout << "ret ";
        std::cout << number;
     }
};
