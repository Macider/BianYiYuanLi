#pragma once
#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>  //分割字符串用
#include <string>
using namespace std;

// 所有 AST 的基类
class BaseAST {
   public:
    //对函数、基本块、变量计数,当前大小为直接可用的最小编号,默认初始化为0?
    static int func_count;
    static int block_count;
    static int var_count;
    virtual ~BaseAST() = default;
    virtual void Dump(string& str) const = 0;
};


// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
   public:
    // 用智能指针管理对象
    unique_ptr<BaseAST> func_def;
    CompUnitAST(unique_ptr<BaseAST>& func_def) {
        //cout << "CompUnitAST created!" << endl;
        this->func_def = move(func_def);
    }
    void Dump(string& str) const override {
        func_def->Dump(str);
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
   public:
    unique_ptr<BaseAST> func_type;
    string ident;
    unique_ptr<BaseAST> block;
    FuncDefAST(unique_ptr<BaseAST>& func_type, string& ident, unique_ptr<BaseAST>& block) {
        //cout << "FuncDefAST created!" << endl;
        this->func_type = move(func_type);
        this->ident = move(ident);
        this->block = move(block);
    }
    void Dump(string& str) const override {
        str += "fun @";
        str += ident;
        this->func_count++;  // 先用再加，从0开始
        str += "(): ";
        func_type->Dump(str);
        str += " {\n";
        //str += "%entry:\n";
        block->Dump(str);
        str += "\n}\n";
    }
};
//: func_type(func_type), ident(ident), block(block)

class FuncTypeAST : public BaseAST {
   public:
    void Dump(string& str) const override {
        str += "i32";
    }
};
class BlockAST : public BaseAST {
   public:
    unique_ptr<BaseAST> stmt;
    BlockAST(unique_ptr<BaseAST>& stmt) {
        this->stmt = move(stmt);
    }
    void Dump(string& str) const override {
        str += "%Block";
        str += to_string(this->block_count);
        this->block_count++;
        str += ":\n";
        stmt->Dump(str);
        /* cout << "out BlockAST";
        cout << "str = " << str << endl; */
    }
};
class StmtAST : public BaseAST {
   public:
    unique_ptr<BaseAST> exp;
    StmtAST(unique_ptr<BaseAST>& exp) {
        this->exp = move(exp);
    }
    void Dump(string& str) const override {
        string tmp_str;
        exp->Dump(tmp_str);
        if (BaseAST::var_count){
            str += tmp_str;
            str += "ret ";
            str += "%";
            str += to_string(BaseAST::var_count - 1);  // 用上一个变量  //有问题待改正
        }
        else{
            str += "ret ";
            str += tmp_str;
        }
        
        /* cout << "out StmtAST";
        cout << "str = " << str << endl; */
    }
    /* int number;
    StmtAST(int number) : number(number){}
    void Dump(string& str) const override {
       str += "ret ";
       str += to_string(number);
    } */
};

class ExpAST : public BaseAST {
   public:
    unique_ptr<BaseAST> unary_exp;
    ExpAST(unique_ptr<BaseAST>& unary_exp) {
        this->unary_exp = move(unary_exp);
    }
    void Dump(string& str) const override {
        unary_exp->Dump(str);
        /* cout << "out ExpAST";
        cout << "str = " << str << endl; */
    }
};
class PrimaryExpAST : public BaseAST {
   public:
    unique_ptr<BaseAST> exp;
    int number;
    int No;
    PrimaryExpAST(int number, int No): No(No) {
        this->exp = nullptr;
        this->number = number;
    }
    PrimaryExpAST(unique_ptr<BaseAST>& exp,int No): No(No) {
        this->exp = move(exp);
    }
    void Dump(string& str) const override {
        if(No == 0){
            //str += "(";
            exp->Dump(str);
            //str += ")";
            return;
        }
        if (No == 1) {
            str += to_string(number);
            return;
        }
        assert("PrimaryExpAST Dump Error!");
    }
};
class UnaryExpAST : public BaseAST {
   public:
    unique_ptr<BaseAST> primary_exp;
    string unary_op;
    unique_ptr<BaseAST> unary_exp;
    int No;
    UnaryExpAST(unique_ptr<BaseAST>& primary_exp,int No):No(No) {
        this->primary_exp = move(primary_exp);
        this->unary_op = "";        //string不能被赋值为nullptr
        this->unary_exp = nullptr;  
    }
    UnaryExpAST(string& unary_op, unique_ptr<BaseAST>& unary_exp, int No): No(No) {
        this->primary_exp = nullptr;
        this->unary_op = move(unary_op);
        this->unary_exp = move(unary_exp);
    }
    void Dump(string& str) const override {
        if (No == 0){
            primary_exp->Dump(str);
            return;
        }
        if (No == 1) {
            string tmp_str1, tmp_str2, lastline, last;    //当前、内层、上一行、上一个操作数
            unary_exp->Dump(tmp_str2);   // 内层的要先处理
            istringstream is1(tmp_str2);  // 用到#include <sstream>
            while (getline(is1, lastline)) {
                istringstream is2(lastline);    //lastline为最后一行
                is2 >> last;  // last要么是最内层的number，要么是某个%变量
            }
            if (unary_op == "+") {  // 单目加号当没看到
                if (last[0] == '%') {  // 内部是有意义表达式而非number
                    str += tmp_str2;
                }
                return;
            }
            // getline(is2,last,' ');
            //is >> last;     
            tmp_str1 += "%";
            tmp_str1 += to_string(this->var_count);  // 分配新变量要等内部处理完毕
            this->var_count++;
            tmp_str1 += " = ";
            if (unary_op == "-"){
                tmp_str1 += "sub 0, ";
                tmp_str1 += last;
                tmp_str1 += "\n";
                if (last[0] == '%') {  // 内部是有意义表达式而非number
                    str += tmp_str2;
                }
                str += tmp_str1;
                /* cout << "out UnaryExpAST";
                cout << "str = " << str << endl; */
                return;
            }
            if (unary_op == "!") {
                tmp_str1 += "eq ";
                tmp_str1 += last;
                tmp_str1 += ", 0\n";
                if (last[0] == '%')  // 内部是有意义表达式而非number
                    str += tmp_str2;
                str += tmp_str1;
                /* cout << "out UnaryExpAST";
                cout << "str = " << str << endl; */
                return;
            }
        }
        assert("UnaryExpAST Dump Error!");
    }
};
/*
class AST : public BaseAST {
    public:
};
*/

/* 以下为该文件的备注 */
// AST Define，即定义了Abstract syntax tree抽象语法树
// 通过不同的构造函数实现A-->B|C，引入了No记号(B.No=0,C.No=1)
// Dump时，传string引用将Koopa-IR储存在str中
// Dump不需要严格前序遍历，根据情况决定前中后序