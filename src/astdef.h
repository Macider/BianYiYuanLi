#pragma once
#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include "symboltable.h"
using namespace std;

enum dump_mode {
    VAR_MODE,   // 正常,向传入的str链接新部分,返回字符串(一般为变量名)
    CONST_MODE  // Const系列专用,只计算数值并返回
};

// string boolize(BaseAST& ast, string& last);     //bool()
static bool isInt(const string& str);

// 类的声明
class BaseAST;
class CompUnitAST;
class DeclAST;
class ConstDeclAST;
class BTypeAST;
class MyconstDefAST;
class ConstDefAST;
class ConstInitValAST;
class FuncDefAST;
class FuncTypeAST;
class BlockAST;
class MyblockItemAST;
class BlockItemAST;
class StmtAST;
class ExpAST;
class LValAST;
class PrimaryExpAST;
class UnaryExpAST;
class UnaryOpAST;
class MulExpAST;
class AddExpAST;
class RelExpAST;
class EqExpAST;
class LAndExpAST;
class LOrExpAST;
class ConstExpAST;

// 所有 AST 的基类
class BaseAST {
   public:
    // 对函数、基本块、变量计数,保存直接可用的最小编号,在main.cpp中初始化
    static int func_count;
    static int block_count;
    static int var_count;
    static shared_ptr<SymbolTable> symbol_table;
    virtual ~BaseAST() = default;
    virtual string Dump(string& str, const dump_mode mode = VAR_MODE) = 0;
    string boolize(string& last) {
        if (last == "0" || last == "1")  // bool(0)->0,bool(1)->1
            return "";
        else {
            string tmp_var_name, tmp_str_ret;
            tmp_var_name = "%";
            tmp_var_name += to_string(this->var_count);
            tmp_str_ret += tmp_var_name;
            this->var_count++;
            tmp_str_ret += " = ne ";  // bool(last)
            tmp_str_ret += last;
            tmp_str_ret += ", 0\n";
            last = tmp_var_name;
            return tmp_str_ret;
        }
    }
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
   public:
    // 用智能指针管理对象
    unique_ptr<BaseAST> func_def;
    CompUnitAST(unique_ptr<BaseAST>& func_def) {
        // cout << "CompUnitAST created!" << endl;
        this->func_def = move(func_def);
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in CompUnitAST" << endl;
        func_def->Dump(str, mode);
        // cout << "Dump out CompUnitAST";
        // cout << "\nstr = " << str << endl;
        return "";
    }
};

class DeclAST : public BaseAST {
   public:
    unique_ptr<BaseAST> const_decl;
    DeclAST(unique_ptr<BaseAST>& const_decl) {
        // cout << "DeclAST created!" << endl;
        this->const_decl = move(const_decl);
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in DeclAST" << endl;
        string tmp_str = const_decl->Dump(str, mode);
        // cout << "Dump out DeclAST";
        // cout << "\nstr = " << str << endl;
        return "";
    }
};

class ConstDeclAST : public BaseAST {
   public:
    string const_str;
    unique_ptr<BaseAST> btype;
    unique_ptr<BaseAST> myconst_def;
    string semicolon;
    ConstDeclAST(string& const_str, unique_ptr<BaseAST>& btype, unique_ptr<BaseAST>& myconst_def, string& semicolon) {
        // cout << "ConstDeclAST created!" << endl;
        this->const_str = move(const_str);
        this->btype = move(btype);
        this->myconst_def = move(myconst_def);
        this->semicolon = move(semicolon);
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in ConstDeclAST" << endl;
        string tmp_str = myconst_def->Dump(str, CONST_MODE);
        // cout << "Dump out ConstDeclAST";
        // cout << "\nstr = " << str << endl;
        return "";
    }
};

class BTypeAST : public BaseAST {
   public:
    string int_str;
    BTypeAST(string& int_str) {
        // cout << "BTypeAST created!" << endl;
        this->int_str = move(int_str);
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in BTypeAST" << endl;
        assert(int_str == "int");
        // str += "i32";
        // cout << "Dump out BTypeAST";
        // cout << "\nstr = " << str << endl;
        return "";
    }
};

class MyconstDefAST : public BaseAST {
   public:
    unique_ptr<BaseAST> const_def;
    unique_ptr<BaseAST> myconst_def;
    string comma;
    int No;
    MyconstDefAST(unique_ptr<BaseAST>& const_def) {
        // cout << "MyconstDefAST created!" << endl;
        this->const_def = move(const_def);
        this->myconst_def = nullptr;
        this->comma = "";
        No = 0;
    }
    MyconstDefAST(unique_ptr<BaseAST>& myconst_def, string& comma, unique_ptr<BaseAST>& const_def) {
        // cout << "MyconstDefAST created!" << endl;
        this->myconst_def = move(myconst_def);
        this->const_def = move(const_def);
        this->comma = move(comma);
        No = 1;
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in MyconstDefAST" << endl;
        if (No == 0) {
            string tmp_str = const_def->Dump(str, mode);
            // cout << "Dump out MyconstDefAST";
            // cout << "\nstr = " << str << endl;
            return "";
        }
        if (No == 1) {
            string tmp_str = myconst_def->Dump(str, mode);
            tmp_str += const_def->Dump(str, mode);
            // cout << "Dump out MyconstDefAST";
            // cout << "\nstr = " << str << endl;
            return "";
        }
        return "";
    }
};

class ConstDefAST : public BaseAST {
   public:
    string ident;
    string assign;
    unique_ptr<BaseAST> const_init_val;
    ConstDefAST(string& ident, string& assign, unique_ptr<BaseAST>& const_init_val) {
        // cout << "ConstDefAST created!" << endl;
        this->ident = move(ident);
        this->assign = move(assign);
        this->const_init_val = move(const_init_val);
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in ConstDefAST" << endl;
        //  计算出该常量，并插入符号表
        string tmp_str_num = const_init_val->Dump(str, mode);
        int tmp_num = stoi(tmp_str_num.c_str());
        this->symbol_table->insertSymbol(ident, tmp_num);
        // cout << "Dump out ConstDefAST";
        // cout << "\nstr = " << str << endl;
        return "";
    }
};

class ConstInitValAST : public BaseAST {
   public:
    unique_ptr<BaseAST> const_exp;
    ConstInitValAST(unique_ptr<BaseAST>& const_exp) {
        // cout << "ConstInitValAST created!" << endl;
        this->const_exp = move(const_exp);
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in ConstInitValAST" << endl;
        //  计算出表达式的值
        string tmp_str = const_exp->Dump(str, mode);
        // cout << "Dump out ConstInitValAST";
        // cout << "\nstr = " << str << endl;
        return tmp_str;
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
   public:
    unique_ptr<BaseAST> func_type;
    string ident;
    string round_left;
    string round_right;
    unique_ptr<BaseAST> block;
    FuncDefAST(unique_ptr<BaseAST>& func_type, string& ident, string& round_left, string& round_right, unique_ptr<BaseAST>& block) {
        // cout << "FuncDefAST created!" << endl;
        this->func_type = move(func_type);
        this->ident = move(ident);
        this->round_left = move(round_left);
        this->round_right = move(round_right);
        this->block = move(block);
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in FuncDefAST" << endl;
        str += "fun @";
        str += ident;
        this->func_count++;  // 先用再加，从0开始
        assert(round_left == "(" && round_right == ")");
        str += "(): ";
        func_type->Dump(str, mode);
        str += " {\n";
        // str += "%entry:\n";
        block->Dump(str, mode);
        str += "}\n";
        // cout << "Dump out FuncDefAST";
        // cout << "\nstr = " << str << endl;
        return "";
    }
};

class FuncTypeAST : public BaseAST {
   public:
    string int_str;
    FuncTypeAST(string& int_str) {
        // cout << "FuncDefAST created!" << endl;
        this->int_str = move(int_str);
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in FuncTypeAST" << endl;
        assert(int_str == "int");
        str += "i32";
        // cout << "Dump out FuncTypeAST";
        // cout << "\nstr = " << str << endl;
        return "";
    }
};

class BlockAST : public BaseAST {
   public:
    string curly_left;
    unique_ptr<BaseAST> myblock_item;
    string curly_right;
    BlockAST(string& curly_left, unique_ptr<BaseAST>& myblock_item, string& curly_right) {
        // cout << "BlockAST created!" << endl;
        this->curly_left = move(curly_left);
        this->myblock_item = move(myblock_item);
        this->curly_right = move(curly_right);
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in BlockAST" << endl;
        assert(curly_left == "{" && curly_right == "}");
        str += "%Block";
        string tmp_block_name = to_string(this->block_count);
        str += tmp_block_name;
        this->block_count++;
        str += ":\n";
        myblock_item->Dump(str, mode);
        // cout << "Dump out BlockAST";
        // cout << "\nstr = " << str << endl;
        return tmp_block_name;  // 先返回块名吧，反正现在用不上
    }
};

class MyblockItemAST : public BaseAST {
   public:
    unique_ptr<BaseAST> myblock_item;
    unique_ptr<BaseAST> block_item;
    int No;
    MyblockItemAST() {
        // cout << "MyblockItemAST created!" << endl;
        this->myblock_item = nullptr;
        this->block_item = nullptr;
        No = 0;
    }
    MyblockItemAST(unique_ptr<BaseAST>& myblock_item, unique_ptr<BaseAST>& block_item) {
        // cout << "MyblockItemAST created!" << endl;
        this->myblock_item = move(myblock_item);
        this->block_item = move(block_item);
        No = 1;
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in MyblockItemAST" << endl;
        if (No == 0) {
            // cout << "Do Nothing" << endl;
            // cout << "Dump out MyblockItemAST";
            // cout << "\nstr = " << str << endl;
            return "";
        }
        if (No == 1) {
            string tmp_str;
            tmp_str += myblock_item->Dump(str, mode);
            tmp_str += block_item->Dump(str, mode);
            // cout << "Dump out MyblockItemAST";
            // cout << "\nstr = " << str << endl;
            return "";
        }
        return "";
    }
};

class BlockItemAST : public BaseAST {  // 该怎么处理需要好好深思
   public:
    unique_ptr<BaseAST> decl;
    unique_ptr<BaseAST> stmt;
    int No;
    BlockItemAST(unique_ptr<BaseAST>& decl_or_stmt, const int No) {
        // cout << "BlockItemAST created!" << endl;
        this->No = No;
        if (No == 0) {
            this->decl = move(decl_or_stmt);
            this->stmt = nullptr;
        }
        if (No == 1) {
            this->stmt = move(decl_or_stmt);
            this->decl = nullptr;
        }
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in BlockItemAST" << endl;
        if (No == 0) {
            // cout << "No==" << No << endl;
            string tmp_str = decl->Dump(str, mode);  // 调用虚函数时通过作用域解析操作符(::)指明
            // cout << "Dump out BlockItemAST";
            // cout << "\nstr = " << str << endl;
            return "";
        }
        if (No == 1) {
            // cout << "No==" << No << endl;
            string tmp_str = stmt->Dump(str, mode);
            // cout << "Dump out BlockItemAST";
            // cout << "\nstr = " << str << endl;
            return "";
        }
        return "";
    }
};

class StmtAST : public BaseAST {
   public:
    string return_str;
    unique_ptr<BaseAST> exp;
    string semicolon;
    StmtAST(string& return_str, unique_ptr<BaseAST>& exp, string& semicolon) {
        // cout << "StmtAST created!" << endl;
        this->return_str = move(return_str);
        this->exp = move(exp);
        this->semicolon = move(semicolon);
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in StmtAST" << endl;
        string tmp_str_exp, last_exp;
        last_exp = exp->Dump(tmp_str_exp, mode);
        if (!isInt(last_exp))
            str += tmp_str_exp;
        assert(return_str == "return");
        str += "ret ";
        str += last_exp;
        str += "\n";
        assert(semicolon == ";");
        // cout << "Dump out StmtAST";
        // cout << "\nstr = " << str << endl;
        return "";
    }
};

class ExpAST : public BaseAST {
   public:
    unique_ptr<BaseAST> lor_exp;
    ExpAST(unique_ptr<BaseAST>& lor_exp) {
        // cout << "ExpAST created!" << endl;
        this->lor_exp = move(lor_exp);
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in ExpAST" << endl;
        string tmp_str = lor_exp->Dump(str, mode);
        // cout << "Dump out ExpAST";
        // cout << "\nstr = " << str << endl;
        return tmp_str;
    }
};

class LValAST : public BaseAST {
   public:
    string ident;
    LValAST(string& ident) {
        // cout << "LValAST created!" << endl;
        this->ident = move(ident);
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in LValAST" << endl;
        // this->symbol_table->printSymbolTable();
        // cout << "Print Finished" << endl;
        shared_ptr<symbol_type> symbol = this->symbol_table->getSymbol(ident);
        if (holds_alternative<int>(*symbol)) {
            string tmp_str_num = to_string(get<int>(*symbol));  // 储存着const变量对应的值
            // cout << "Dump out LValAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str_num;
        }
        return "";
    }
};

class PrimaryExpAST : public BaseAST {
   public:
    string round_left;
    unique_ptr<BaseAST> exp;
    string round_right;
    unique_ptr<BaseAST> lval;
    int number;
    int No;
    PrimaryExpAST(string& round_left, unique_ptr<BaseAST>& exp, string& round_right) {
        // cout << "PrimaryExpAST created!" << endl;
        this->round_left = move(round_left);
        this->exp = move(exp);
        this->round_right = move(round_right);
        this->lval = nullptr;
        No = 0;
    }
    PrimaryExpAST(unique_ptr<BaseAST>& lval) {
        // cout << "PrimaryExpAST created!" << endl;
        this->lval = move(lval);
        this->round_left = "";
        this->exp = nullptr;
        this->round_right = "";
        No = 1;
    }
    PrimaryExpAST(int number) {
        // cout << "PrimaryExpAST created!" << endl;
        this->number = number;
        this->round_left = "";
        this->exp = nullptr;
        this->round_right = "";
        this->lval = nullptr;
        No = 2;
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in PrimaryExpAST" << endl;
        if (No == 0) {
            string tmp_str = exp->Dump(str, mode);
            // cout << "Dump out PrimaryExpAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str;
        }
        if (No == 1) {
            string tmp_str = lval->Dump(str, mode);
            // cout << "Dump out PrimaryExpAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str;
        }
        if (No == 2) {
            if (mode == CONST_MODE) {
                int num_primary = number;
                string tmp_str_num = to_string(num_primary);
                // cout << "Dump out UnaryExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_str_num;
            }
            string tmp_str = to_string(number);
            str += tmp_str;
            // cout << "Dump out PrimaryExpAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str;
        }
        assert("PrimaryExpAST Dump Error!");
        return "";
    }
};

class UnaryExpAST : public BaseAST {
   public:
    unique_ptr<BaseAST> primary_exp;
    string unary_op;
    unique_ptr<BaseAST> unary_exp;
    int No;
    UnaryExpAST(unique_ptr<BaseAST>& primary_exp) {
        // cout << "UnaryExpAST created!" << endl;
        this->primary_exp = move(primary_exp);
        this->unary_op = "";  // string不能被赋值为nullptr
        this->unary_exp = nullptr;
        No = 0;
    }
    UnaryExpAST(string& unary_op, unique_ptr<BaseAST>& unary_exp) {
        // cout << "UnaryExpAST created!" << endl;
        this->primary_exp = nullptr;
        this->unary_op = move(unary_op);
        this->unary_exp = move(unary_exp);
        No = 1;
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in UnaryExpAST" << endl;
        if (No == 0) {
            string tmp_str = primary_exp->Dump(str, mode);
            // cout << "Dump out UnaryExpAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str;
        }
        if (No == 1) {
            string tmp_str_now, tmp_str_unary, last_unary;      // 当前、内层、unary的"变量名"
            last_unary = unary_exp->Dump(tmp_str_unary, mode);  // 内层的要先处理
            if (mode == CONST_MODE) {
                int num_unary = stoi(last_unary.c_str());
                string tmp_str_num;
                if (unary_op == "+")
                    tmp_str_num = to_string(num_unary);
                if (unary_op == "-")
                    tmp_str_num = to_string(-num_unary);
                if (unary_op == "!")
                    tmp_str_num = to_string(!num_unary);
                // cout << "Dump out UnaryExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_str_num;
            }
            if (unary_op == "+") {       // 单目加号当没看到
                if (!isInt(last_unary))  // 内部是有意义表达式而非number
                    str += tmp_str_unary;
                // cout << "Dump out UnaryExpAST";
                // cout << "\nstr = " << str << endl;
                return last_unary;
            }
            if (!isInt(last_unary))  // 内部是有意义表达式而非number
                str += tmp_str_unary;
            if (unary_op == "-") {
                string tmp_var_name;
                tmp_var_name = "%";
                tmp_var_name += to_string(this->var_count);
                tmp_str_now += tmp_var_name;
                this->var_count++;
                tmp_str_now += " = sub 0, ";
                tmp_str_now += last_unary;
                tmp_str_now += "\n";
                str += tmp_str_now;
                // cout << "Dump out UnaryExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
            if (unary_op == "!") {
                string tmp_var_name;
                tmp_var_name = "%";
                tmp_var_name += to_string(this->var_count);
                tmp_str_now += tmp_var_name;
                this->var_count++;
                tmp_str_now += " = eq ";
                tmp_str_now += last_unary;
                tmp_str_now += ", 0\n";
                str += tmp_str_now;
                // cout << "Dump out UnaryExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
        }
        assert("UnaryExpAST Dump Error!");
        return "";
    }
};

class MulExpAST : public BaseAST {
   public:
    unique_ptr<BaseAST> unary_exp;
    string binary_op;  //  具体为* / % 姑且这么称呼
    unique_ptr<BaseAST> mul_exp;
    int No;
    MulExpAST(unique_ptr<BaseAST>& unary_exp) {
        // cout << "MulExpAST created!" << endl;
        this->unary_exp = move(unary_exp);
        this->binary_op = "";  // string不能被赋值为nullptr
        this->mul_exp = nullptr;
        No = 0;
    }
    MulExpAST(unique_ptr<BaseAST>& mul_exp, string& binary_op, unique_ptr<BaseAST>& unary_exp) {
        // cout << "MulExpAST created!" << endl;
        this->mul_exp = move(mul_exp);
        this->binary_op = move(binary_op);
        this->unary_exp = move(unary_exp);
        No = 1;
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in MulExpAST" << endl;
        if (No == 0) {
            string str_tmp = unary_exp->Dump(str, mode);
            // cout << "Dump out MulExpAST";
            // cout << "\nstr = " << str << endl;
            return str_tmp;
        }
        if (No == 1) {
            string tmp_str_now, tmp_str_mul, tmp_str_unary;
            string last_mul, last_unary;
            last_mul = mul_exp->Dump(tmp_str_mul, mode);        // 处理MulExp分量
            last_unary = unary_exp->Dump(tmp_str_unary, mode);  // 处理UnaryExp分量
            if (mode == CONST_MODE) {
                int num_mul = stoi(last_mul.c_str());
                int num_unary = stoi(last_unary.c_str());
                string tmp_str_num;
                if (binary_op == "*")
                    tmp_str_num = to_string(num_mul * num_unary);
                if (binary_op == "/")
                    tmp_str_num = to_string(num_mul / num_unary);
                if (binary_op == "%")
                    tmp_str_num = to_string(num_mul % num_unary);
                // cout << "Dump out MulExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_str_num;
            }
            if (!isInt(last_mul))
                str += tmp_str_mul;
            if (!isInt(last_unary))
                str += tmp_str_unary;
            if (binary_op == "*") {
                string tmp_var_name;
                tmp_var_name = "%";
                tmp_var_name += to_string(this->var_count);
                tmp_str_now += tmp_var_name;
                this->var_count++;
                tmp_str_now += " = mul ";
                tmp_str_now += last_mul;
                tmp_str_now += ", ";
                tmp_str_now += last_unary;
                tmp_str_now += "\n";
                str += tmp_str_now;
                // cout << "Dump out MulExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
            if (binary_op == "/") {
                string tmp_var_name;
                tmp_var_name = "%";
                tmp_var_name += to_string(this->var_count);
                tmp_str_now += tmp_var_name;
                this->var_count++;
                tmp_str_now += " = div ";
                tmp_str_now += last_mul;
                tmp_str_now += ", ";
                tmp_str_now += last_unary;
                tmp_str_now += "\n";
                str += tmp_str_now;
                // cout << "Dump out MulExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
            if (binary_op == "%") {
                string tmp_var_name;
                tmp_var_name = "%";
                tmp_var_name += to_string(this->var_count);
                tmp_str_now += tmp_var_name;
                this->var_count++;
                tmp_str_now += " = mod ";
                tmp_str_now += last_mul;
                tmp_str_now += ", ";
                tmp_str_now += last_unary;
                tmp_str_now += "\n";
                str += tmp_str_now;
                // cout << "Dump out MulExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
        }
        assert("MulExpAST Dump Error!");
        return "";
    }
};

class AddExpAST : public BaseAST {
   public:
    unique_ptr<BaseAST> mul_exp;
    string binary_op;  // 具体为+ - 姑且这么称呼
    unique_ptr<BaseAST> add_exp;
    int No;
    AddExpAST(unique_ptr<BaseAST>& mul_exp) {
        // cout << "AddExpAST created!" << endl;
        this->mul_exp = move(mul_exp);
        this->binary_op = "";  // string不能被赋值为nullptr
        this->add_exp = nullptr;
        No = 0;
    }
    AddExpAST(unique_ptr<BaseAST>& add_exp, string& binary_op, unique_ptr<BaseAST>& mul_exp) {
        // cout << "AddExpAST created!" << endl;
        this->add_exp = move(add_exp);
        this->binary_op = move(binary_op);
        this->mul_exp = move(mul_exp);
        No = 1;
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in AddExpAST" << endl;
        if (No == 0) {
            string tmp_str = mul_exp->Dump(str, mode);
            // cout << "Dump out AddExpAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str;
        }
        if (No == 1) {
            string tmp_str_now, tmp_str_add, tmp_str_mul;
            string last_add, last_mul;
            last_add = add_exp->Dump(tmp_str_add, mode);  // 处理AddExp分量
            last_mul = mul_exp->Dump(tmp_str_mul, mode);  // 处理MulExp分量
            if (mode == CONST_MODE) {
                int num_add = stoi(last_add.c_str());
                int num_mul = stoi(last_mul.c_str());
                string tmp_str_num;
                if (binary_op == "+")
                    tmp_str_num = to_string(num_add + num_mul);
                if (binary_op == "-")
                    tmp_str_num = to_string(num_add - num_mul);
                // cout << "Dump out AddExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_str_num;
            }
            if (!isInt(last_add))
                str += tmp_str_add;
            if (!isInt(last_mul))
                str += tmp_str_mul;
            if (binary_op == "+") {
                string tmp_var_name;
                tmp_var_name = "%";
                tmp_var_name += to_string(this->var_count);
                tmp_str_now += tmp_var_name;
                this->var_count++;
                tmp_str_now += " = add ";
                tmp_str_now += last_add;
                tmp_str_now += ", ";
                tmp_str_now += last_mul;
                tmp_str_now += "\n";
                str += tmp_str_now;
                // cout << "Dump out AddExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
            if (binary_op == "-") {
                string tmp_var_name;
                tmp_var_name = "%";
                tmp_var_name += to_string(this->var_count);
                tmp_str_now += tmp_var_name;
                this->var_count++;
                tmp_str_now += " = sub ";
                tmp_str_now += last_add;
                tmp_str_now += ", ";
                tmp_str_now += last_mul;
                tmp_str_now += "\n";
                str += tmp_str_now;
                // cout << "Dump out AddExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
        }
        assert("AddExpAST Dump Error!");
        return "";
    }
};

class RelExpAST : public BaseAST {
   public:
    unique_ptr<BaseAST> add_exp;
    string binary_op;  // 具体为< <= > >= 姑且这么称呼
    unique_ptr<BaseAST> rel_exp;
    int No;
    RelExpAST(unique_ptr<BaseAST>& add_exp) {
        // cout << "RelExpAST created!" << endl;
        this->add_exp = move(add_exp);
        this->binary_op = "";  // string不能被赋值为nullptr
        this->rel_exp = nullptr;
        No = 0;
    }
    RelExpAST(unique_ptr<BaseAST>& rel_exp, string& binary_op, unique_ptr<BaseAST>& add_exp) {
        // cout << "RelExpAST created!" << endl;
        this->rel_exp = move(rel_exp);
        this->binary_op = move(binary_op);
        this->add_exp = move(add_exp);
        No = 1;
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in RelExpAST" << endl;
        if (No == 0) {
            string tmp_str = add_exp->Dump(str, mode);
            // cout << "Dump out RelExpAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str;
        }
        if (No == 1) {
            string tmp_str_now, tmp_str_rel, tmp_str_add;
            string last_rel, last_add;
            last_rel = rel_exp->Dump(tmp_str_rel, mode);  // 处理RelExp分量
            last_add = add_exp->Dump(tmp_str_add, mode);  // 处理AddExp分量
            if (mode == CONST_MODE) {
                int num_rel = stoi(last_rel.c_str());
                int num_add = stoi(last_add.c_str());
                string tmp_str_num;
                if (binary_op == "<")
                    tmp_str_num = to_string(num_rel < num_add);
                if (binary_op == ">")
                    tmp_str_num = to_string(num_rel > num_add);
                if (binary_op == "<=")
                    tmp_str_num = to_string(num_rel <= num_add);
                if (binary_op == ">=")
                    tmp_str_num = to_string(num_rel >= num_add);
                // cout << "Dump out RelExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_str_num;
            }
            if (!isInt(last_rel))
                str += tmp_str_rel;
            if (!isInt(last_add))
                str += tmp_str_add;
            if (binary_op == "<") {
                string tmp_var_name;
                tmp_var_name = "%";
                tmp_var_name += to_string(this->var_count);
                tmp_str_now += tmp_var_name;
                this->var_count++;
                tmp_str_now += " = lt ";
                tmp_str_now += last_rel;
                tmp_str_now += ", ";
                tmp_str_now += last_add;
                tmp_str_now += "\n";
                str += tmp_str_now;
                // cout << "Dump out RelExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
            if (binary_op == ">") {
                string tmp_var_name;
                tmp_var_name = "%";
                tmp_var_name += to_string(this->var_count);
                tmp_str_now += tmp_var_name;
                this->var_count++;
                tmp_str_now += " = gt ";
                tmp_str_now += last_rel;
                tmp_str_now += ", ";
                tmp_str_now += last_add;
                tmp_str_now += "\n";
                str += tmp_str_now;
                // cout << "Dump out RelExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
            if (binary_op == "<=") {
                string tmp_var_name;
                tmp_var_name = "%";
                tmp_var_name += to_string(this->var_count);
                tmp_str_now += tmp_var_name;
                this->var_count++;
                tmp_str_now += " = le ";
                tmp_str_now += last_rel;
                tmp_str_now += ", ";
                tmp_str_now += last_add;
                tmp_str_now += "\n";
                str += tmp_str_now;
                // cout << "Dump out RelExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
            if (binary_op == ">=") {
                string tmp_var_name;
                tmp_var_name = "%";
                tmp_var_name += to_string(this->var_count);
                tmp_str_now += tmp_var_name;
                this->var_count++;
                tmp_str_now += " = ge ";
                tmp_str_now += last_rel;
                tmp_str_now += ", ";
                tmp_str_now += last_add;
                tmp_str_now += "\n";
                str += tmp_str_now;
                // cout << "Dump out RelExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
        }
        assert("RelExpAST Dump Error!");
        return "";
    }
};

class EqExpAST : public BaseAST {
   public:
    unique_ptr<BaseAST> rel_exp;
    string binary_op;  // 具体为== != 姑且这么称呼
    unique_ptr<BaseAST> eq_exp;
    int No;
    EqExpAST(unique_ptr<BaseAST>& rel_exp) {
        // cout << "EqExpAST created!" << endl;
        this->rel_exp = move(rel_exp);
        this->binary_op = "";  // string不能被赋值为nullptr
        this->eq_exp = nullptr;
        No = 0;
    }
    EqExpAST(unique_ptr<BaseAST>& eq_exp, string& binary_op, unique_ptr<BaseAST>& rel_exp) {
        // cout << "EqExpAST created!" << endl;
        this->eq_exp = move(eq_exp);
        this->binary_op = move(binary_op);
        this->rel_exp = move(rel_exp);
        No = 1;
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in EqExpAST" << endl;
        if (No == 0) {
            string tmp_str = rel_exp->Dump(str, mode);
            // cout << "Dump out EqExpAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str;
        }
        if (No == 1) {
            string tmp_str_now, tmp_str_eq, tmp_str_rel;
            string last_eq, last_rel;
            last_eq = eq_exp->Dump(tmp_str_eq, mode);     // 处理EqExp分量
            last_rel = rel_exp->Dump(tmp_str_rel, mode);  // 处理RelExp分量
            if (mode == CONST_MODE) {
                int num_eq = stoi(last_eq.c_str());
                int num_rel = stoi(last_rel.c_str());
                string tmp_str_num;
                if (binary_op == "==")
                    tmp_str_num = to_string(num_eq == num_rel);
                if (binary_op == "!=")
                    tmp_str_num = to_string(num_eq != num_rel);
                // cout << "Dump out EqExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_str_num;
            }
            if (!isInt(last_eq))
                str += tmp_str_eq;
            if (!isInt(last_rel))
                str += tmp_str_rel;
            if (binary_op == "==") {
                string tmp_var_name;
                tmp_var_name = "%";
                tmp_var_name += to_string(this->var_count);
                tmp_str_now += tmp_var_name;
                this->var_count++;
                tmp_str_now += " = eq ";
                tmp_str_now += last_eq;
                tmp_str_now += ", ";
                tmp_str_now += last_rel;
                tmp_str_now += "\n";
                str += tmp_str_now;
                // cout << "Dump out EqExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
            if (binary_op == "!=") {
                string tmp_var_name;
                tmp_var_name = "%";
                tmp_var_name += to_string(this->var_count);
                tmp_str_now += tmp_var_name;
                this->var_count++;
                tmp_str_now += " = ne ";
                tmp_str_now += last_eq;
                tmp_str_now += ", ";
                tmp_str_now += last_rel;
                tmp_str_now += "\n";
                str += tmp_str_now;
                // cout << "Dump out EqExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
        }
        assert("EqExpAST Dump Error!");
        return "";
    }
};

class LAndExpAST : public BaseAST {
   public:
    unique_ptr<BaseAST> eq_exp;
    string binary_op;  // 具体为+ - 姑且这么称呼
    unique_ptr<BaseAST> land_exp;
    int No;
    LAndExpAST(unique_ptr<BaseAST>& eq_exp) {
        // cout << "LAndExpAST created!" << endl;
        this->eq_exp = move(eq_exp);
        this->binary_op = "";  // string不能被赋值为nullptr
        this->land_exp = nullptr;
        No = 0;
    }
    LAndExpAST(unique_ptr<BaseAST>& land_exp, string& binary_op, unique_ptr<BaseAST>& eq_exp) {
        // cout << "LAndExpAST created!" << endl;
        this->land_exp = move(land_exp);
        this->binary_op = move(binary_op);
        this->eq_exp = move(eq_exp);
        No = 1;
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in LAndExpAST" << endl;
        if (No == 0) {
            string tmp_str = eq_exp->Dump(str, mode);
            // cout << "Dump out LAndExpAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str;
        }
        if (No == 1) {
            string tmp_str_now, tmp_str_land, tmp_str_eq;
            string last_land, last_eq;
            last_land = land_exp->Dump(tmp_str_land, mode);  // 处理LAndExp分量
            last_eq = eq_exp->Dump(tmp_str_eq, mode);        // 处理EqExp分量
            if (mode == CONST_MODE) {
                int num_land = stoi(last_land.c_str());
                int num_eq = stoi(last_eq.c_str());
                string tmp_str_num = to_string(num_land && num_eq);
                // cout << "Dump out LAndExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_str_num;
            }
            if (!isInt(last_land))
                str += tmp_str_land;
            if (!isInt(last_eq))
                str += tmp_str_eq;
            // 涉及布尔运算，需要先将分量转为0/1
            tmp_str_now += boolize(last_land);  // 将last_land转为bool类型
            tmp_str_now += boolize(last_eq);    // 将last_eq转为bool类型
            if (binary_op == "&&") {            // bool(a)&bool(b)-->a&&b
                string tmp_var_name;
                tmp_var_name = "%";
                tmp_var_name += to_string(this->var_count);
                tmp_str_now += tmp_var_name;
                this->var_count++;
                tmp_str_now += " = and ";  // c = a&b
                tmp_str_now += last_land;
                tmp_str_now += ", ";
                tmp_str_now += last_eq;
                tmp_str_now += "\n";
                str += tmp_str_now;
                // cout << "Dump out LAndExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
        }
        assert("LAndExpAST Dump Error!");
        return "";
    }
};

class LOrExpAST : public BaseAST {
   public:
    unique_ptr<BaseAST> land_exp;
    string binary_op;  // 具体为+ - 姑且这么称呼
    unique_ptr<BaseAST> lor_exp;
    int No;
    LOrExpAST(unique_ptr<BaseAST>& land_exp) {
        // cout << "LOrExpAST created!" << endl;
        this->land_exp = move(land_exp);
        this->binary_op = "";  // string不能被赋值为nullptr
        this->lor_exp = nullptr;
        No = 0;
    }
    LOrExpAST(unique_ptr<BaseAST>& lor_exp, string& binary_op, unique_ptr<BaseAST>& land_exp) {
        // cout << "LOrExpAST created!" << endl;
        this->lor_exp = move(lor_exp);
        this->binary_op = move(binary_op);
        this->land_exp = move(land_exp);
        No = 1;
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in LOrExpAST" << endl;

        if (No == 0) {
            string tmp_str = land_exp->Dump(str, mode);
            // cout << "Dump out LOrExpAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str;
        }
        if (No == 1) {
            string tmp_str_now, tmp_str_lor, tmp_str_land;
            string last_lor, last_land;
            last_lor = lor_exp->Dump(tmp_str_lor, mode);     // 处理EqExp分量
            last_land = land_exp->Dump(tmp_str_land, mode);  // 处理LAndExp分量
            if (mode == CONST_MODE) {
                int num_lor = stoi(last_lor.c_str());
                int num_land = stoi(last_land.c_str());
                string tmp_str_num = to_string(num_lor || num_land);
                // cout << "Dump out LOrExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_str_num;
            }
            if (!isInt(last_lor))
                str += tmp_str_lor;
            if (!isInt(last_land))
                str += tmp_str_land;
            // 涉及布尔运算，需要先将分量转为0/1
            tmp_str_now += boolize(last_lor);   // 将last_lor转为bool类型
            tmp_str_now += boolize(last_land);  // 将last_land转为bool类型
            if (binary_op == "||") {            // bool(a)|bool(b)-->a||b
                string tmp_var_name;
                tmp_var_name = "%";
                tmp_var_name += to_string(this->var_count);
                tmp_str_now += tmp_var_name;
                this->var_count++;
                tmp_str_now += " = or ";  // c = (a|b)
                tmp_str_now += last_lor;
                tmp_str_now += ", ";
                tmp_str_now += last_land;
                tmp_str_now += "\n";
                str += tmp_str_now;
                // cout << "Dump out LOrExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
        }
        assert("LOrExpAST Dump Error!");
        return "";
    }
};

class ConstExpAST : public BaseAST {
   public:
    unique_ptr<BaseAST> exp;
    ConstExpAST(unique_ptr<BaseAST>& exp) {
        // cout << "ConstExpAST created!" << endl;
        this->exp = move(exp);
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        // cout << "Dump in ConstExpAST" << endl;
        string tmp_str = exp->Dump(str, mode);
        // cout << "Dump out ConstExpAST";
        // cout << "\nstr = " << str << endl;
        return tmp_str;
    }
};

static bool isInt(const string& str) {
    istringstream tmp_stream(str);
    int i;
    char c;
    if (!(tmp_stream >> i))
        return false;
    if (tmp_stream >> c)
        return false;
    return true;
}

/* 以下为该文件的备注 */
// AST Define，即定义了Abstract syntax tree抽象语法树
// 通过不同的构造函数实现A-->B|C，引入了No记号(B.No=0,C.No=1)
// 成员变量定义顺序依照产生式中出现的顺序
// 构造函数参数顺序依照对应产生式中出现的顺序
// Dump时，传string引用将KoopaIR储存在str中
// Dump在结合当前语句情况的基础上后序遍历得到koopaIR
// 返回值str主要用于存储变量名、块名等信息
// 该文件中Cout均用于调试，ctrl+H替换即可
// 问题: BlockItem-->Decl|Stmt, Dump时无法选择调用哪一个Dump
// 调用虚函数Dump时用作用域操作符指明调用哪一个,如StmtAST::Dump()

/* 屎山重构计划 */
// Block的编号可以将Block0换为Entry
// !的部分可以考虑加入对0的特判，不过好像用处不大
// primary.number怎么初始化？考虑将Number变成AST

/* AST模板 */
/* class AST : public BaseAST {
    public:
    AST(){
    }
    string Dump(string& str, const dump_mode mode = VAR_MODE) override {
        return "";
    }
}; */

/* 零散未删除代码 */
/* #include <sstream>  //分割字符串用 */
/* getline(is2, last, ' ');
is >> last; */
/* string lastline_mul, lastline_unary;
istringstream is_mul1(tmp_str_mul);
while (getline(is_mul1, lastline_mul)) {
    istringstream is_mul2(lastline_mul);  // lastline为最后一行
    is_mul2 >> last_mul;                  // last要么是最内层的number，要么是某个%变量
}
istringstream is_unary1(tmp_str_unary);
while (getline(is_unary1, lastline_unary)) {
    istringstream is_unary2(lastline_unary);
    is_unary2 >> last_unary;
} */
/* tmp_var_name = "%";
tmp_var_name += to_string(this->var_count);
tmp_str_now += tmp_var_name;
this->var_count++;
tmp_str_now += " = ne ";  // bool(last_eq)
tmp_str_now += last_land;
tmp_str_now += ", 0\n";
last_land = tmp_var_name;
// 以上将last_land转为bool类型 */
