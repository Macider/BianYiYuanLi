#pragma once
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
using namespace std;

// 所有 AST 的基类
class BaseAST {
   public:
    // 对函数、基本块、变量计数,保存直接可用的最小编号,在main.cpp中初始化
    static int func_count;
    static int block_count;
    static int var_count;
    virtual ~BaseAST() = default;
    virtual string Dump(string& str) const = 0;
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
    string Dump(string& str) const override {
        // cout << "Dump in CompUnitAST" << endl;
        func_def->Dump(str);
        // cout << "Dump out CompUnitAST";
        // cout << "\nstr = " << str << endl;
        return "";
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
   public:
    unique_ptr<BaseAST> func_type;
    string ident;
    unique_ptr<BaseAST> block;
    FuncDefAST(unique_ptr<BaseAST>& func_type, string& ident, unique_ptr<BaseAST>& block) {
        // cout << "FuncDefAST created!" << endl;
        this->func_type = move(func_type);
        this->ident = move(ident);
        this->block = move(block);
    }
    string Dump(string& str) const override {
        // cout << "Dump in FuncDefAST" << endl;
        str += "fun @";
        str += ident;
        this->func_count++;  // 先用再加，从0开始
        str += "(): ";
        func_type->Dump(str);
        str += " {\n";
        // str += "%entry:\n";
        block->Dump(str);
        str += "}\n";
        // cout << "Dump out FuncDefAST";
        // cout << "\nstr = " << str << endl;
        return "";
    }
};

class FuncTypeAST : public BaseAST {
   public:
    string Dump(string& str) const override {
        // cout << "Dump in FuncTypeAST" << endl;
        str += "i32";
        // cout << "Dump out FuncTypeAST";
        // cout << "\nstr = " << str << endl;
        return "i32";
    }
};
class BlockAST : public BaseAST {
   public:
    unique_ptr<BaseAST> stmt;
    BlockAST(unique_ptr<BaseAST>& stmt) {
        // cout << "BlockAST created!" << endl;
        this->stmt = move(stmt);
    }
    string Dump(string& str) const override {
        // cout << "Dump in BlockAST" << endl;
        str += "%Block";
        string tmp_block_name = to_string(this->block_count);
        str += tmp_block_name;
        this->block_count++;
        str += ":\n";
        stmt->Dump(str);
        // cout << "Dump out BlockAST";
        // cout << "\nstr = " << str << endl;
        return tmp_block_name;  // 先返回块名吧，反正现在用不上
    }
};
class StmtAST : public BaseAST {
   public:
    unique_ptr<BaseAST> exp;
    StmtAST(unique_ptr<BaseAST>& exp) {
        // cout << "StmtAST created!" << endl;
        this->exp = move(exp);
    }
    string Dump(string& str) const override {
        // cout << "Dump in StmtAST" << endl;
        string tmp_str_exp, last_exp;
        last_exp = exp->Dump(tmp_str_exp);
        if (last_exp[0] == '%')
            str += tmp_str_exp;
        str += "ret ";
        str += last_exp;
        str += "\n";
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
    string Dump(string& str) const override {
        // cout << "Dump in ExpAST" << endl;
        string tmp_str = lor_exp->Dump(str);
        // cout << "Dump out ExpAST";
        // cout << "\nstr = " << str << endl;
        return tmp_str;
    }
};

class PrimaryExpAST : public BaseAST {
   public:
    unique_ptr<BaseAST> exp;
    int number;
    int No;
    PrimaryExpAST(unique_ptr<BaseAST>& exp) {
        // cout << "PrimaryExpAST created!" << endl;
        this->exp = move(exp);
        No = 0;
    }
    PrimaryExpAST(int number) {
        // cout << "PrimaryExpAST created!" << endl;
        this->exp = nullptr;
        this->number = number;
        No = 1;
    }
    string Dump(string& str) const override {
        // cout << "Dump in PrimaryExpAST" << endl;
        if (No == 0) {
            string tmp_str = exp->Dump(str);
            // cout << "Dump out PrimaryExpAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str;
        }
        if (No == 1) {
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
    string Dump(string& str) const override {
        // cout << "Dump in UnaryExpAST" << endl;
        if (No == 0) {
            string tmp_str = primary_exp->Dump(str);
            // cout << "Dump out UnaryExpAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str;
        }
        if (No == 1) {
            string tmp_str_now, tmp_str_unary, last_unary;  // 当前、内层、unary的"变量名"
            last_unary = unary_exp->Dump(tmp_str_unary);    // 内层的要先处理
            if (unary_op == "+") {                          // 单目加号当没看到
                if (last_unary[0] == '%')                   // 内部是有意义表达式而非number
                    str += tmp_str_unary;
                // cout << "Dump out UnaryExpAST";
                // cout << "\nstr = " << str << endl;
                return last_unary;
            }
            if (last_unary[0] == '%')  // 内部是有意义表达式而非number
                str += tmp_str_unary;
            string tmp_var_name;
            if (unary_op == "-") {
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
    string Dump(string& str) const override {
        // cout << "Dump in MulExpAST" << endl;
        if (No == 0) {
            string str_tmp = unary_exp->Dump(str);
            // cout << "Dump out MulExpAST";
            // cout << "\nstr = " << str << endl;
            return str_tmp;
        }
        if (No == 1) {
            string tmp_str_now, tmp_str_mul, tmp_str_unary;
            string last_mul, last_unary;
            last_mul = mul_exp->Dump(tmp_str_mul);        // 处理MulExp分量
            last_unary = unary_exp->Dump(tmp_str_unary);  // 处理UnaryExp分量
            if (last_mul[0] == '%')
                str += tmp_str_mul;
            if (last_unary[0] == '%')
                str += tmp_str_unary;
            string tmp_var_name;
            if (binary_op == "*") {
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
    string Dump(string& str) const override {
        // cout << "Dump in AddExpAST" << endl;
        if (No == 0) {
            string tmp_str = mul_exp->Dump(str);
            // cout << "Dump out AddExpAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str;
        }
        if (No == 1) {
            string tmp_str_now, tmp_str_add, tmp_str_mul;
            string last_add, last_mul;
            last_add = add_exp->Dump(tmp_str_add);  // 处理AddExp分量
            last_mul = mul_exp->Dump(tmp_str_mul);  // 处理MulExp分量
            if (last_add[0] == '%')
                str += tmp_str_add;
            if (last_mul[0] == '%')
                str += tmp_str_mul;
            string tmp_var_name;
            if (binary_op == "+") {
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
    string Dump(string& str) const override {
        // cout << "Dump in RelExpAST" << endl;
        if (No == 0) {
            string tmp_str = add_exp->Dump(str);
            // cout << "Dump out RelExpAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str;
        }
        if (No == 1) {
            string tmp_str_now, tmp_str_rel, tmp_str_add;
            string last_rel, last_add;
            last_rel = rel_exp->Dump(tmp_str_rel);  // 处理RelExp分量
            last_add = add_exp->Dump(tmp_str_add);  // 处理AddExp分量
            if (last_rel[0] == '%')
                str += tmp_str_rel;
            if (last_add[0] == '%')
                str += tmp_str_add;
            string tmp_var_name;
            if (binary_op == "<") {
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
    string Dump(string& str) const override {
        // cout << "Dump in EqExpAST" << endl;
        if (No == 0) {
            string tmp_str = rel_exp->Dump(str);
            // cout << "Dump out EqExpAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str;
        }
        if (No == 1) {
            string tmp_str_now, tmp_str_eq, tmp_str_rel;
            string last_eq, last_rel;
            last_eq = eq_exp->Dump(tmp_str_eq);     // 处理EqExp分量
            last_rel = rel_exp->Dump(tmp_str_rel);  // 处理RelExp分量
            if (last_eq[0] == '%')
                str += tmp_str_eq;
            if (last_rel[0] == '%')
                str += tmp_str_rel;
            string tmp_var_name;
            if (binary_op == "==") {
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
    string Dump(string& str) const override {
        // cout << "Dump in LAndExpAST" << endl;
        if (No == 0) {
            string tmp_str = eq_exp->Dump(str);
            // cout << "Dump out LAndExpAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str;
        }
        if (No == 1) {
            string tmp_str_now, tmp_str_land, tmp_str_eq;
            string last_land, last_eq;
            last_land = land_exp->Dump(tmp_str_land);  // 处理LAndExp分量
            last_eq = eq_exp->Dump(tmp_str_eq);        // 处理EqExp分量
            if (last_land[0] == '%')
                str += tmp_str_land;
            if (last_eq[0] == '%')
                str += tmp_str_eq;
            string tmp_var_name;
            // 涉及布尔运算，需要先将分量转为0/1
            tmp_var_name = "%";
            tmp_var_name += to_string(this->var_count);
            tmp_str_now += tmp_var_name;
            this->var_count++;
            tmp_str_now += " = ne ";  // bool(last_land)
            tmp_str_now += last_land;
            tmp_str_now += ", 0\n";
            last_land = tmp_var_name;
            // 以上将last_land转为bool类型
            tmp_var_name = "%";
            tmp_var_name += to_string(this->var_count);
            tmp_str_now += tmp_var_name;
            this->var_count++;
            tmp_str_now += " = ne ";  // bool(last_eq)
            tmp_str_now += last_eq;
            tmp_str_now += ", 0\n";
            last_eq = tmp_var_name;
            // 以上将last_eq转为bool类型
            if (binary_op == "&&") {  // bool(a)&bool(b)-->a&&b
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
    string Dump(string& str) const override {
        // cout << "Dump in LOrExpAST" << endl;
        if (No == 0) {
            string tmp_str = land_exp->Dump(str);
            // cout << "Dump out LOrExpAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str;
        }
        if (No == 1) {
            string tmp_str_now, tmp_str_lor, tmp_str_land;
            string last_lor, last_land;
            last_lor = lor_exp->Dump(tmp_str_lor);     // 处理EqExp分量
            last_land = land_exp->Dump(tmp_str_land);  // 处理LAndExp分量
            if (last_lor[0] == '%')
                str += tmp_str_lor;
            if (last_land[0] == '%')
                str += tmp_str_land;
            string tmp_var_name;
            // 涉及布尔运算，需要先将分量转为0/1
            tmp_var_name = "%";
            tmp_var_name += to_string(this->var_count);
            tmp_str_now += tmp_var_name;
            this->var_count++;
            tmp_str_now += " = ne ";  // bool(last_land)
            tmp_str_now += last_lor;
            tmp_str_now += ", 0\n";
            last_lor = tmp_var_name;
            // 以上将last_lor转为bool类型
            tmp_var_name = "%";
            tmp_var_name += to_string(this->var_count);
            tmp_str_now += tmp_var_name;
            this->var_count++;
            tmp_str_now += " = ne ";  // bool(last_eq)
            tmp_str_now += last_land;
            tmp_str_now += ", 0\n";
            last_land = tmp_var_name;
            // 以上将last_land转为bool类型
            if (binary_op == "||") {  // bool(a|b)-->a||b
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

/* 以下为该文件的备注 */
// AST Define，即定义了Abstract syntax tree抽象语法树
// 通过不同的构造函数实现A-->B|C，引入了No记号(B.No=0,C.No=1)
// 成员变量定义顺序依照产生式中出现的顺序
// 构造函数参数顺序依照对应产生式中出现的顺序
// Dump时，传string引用将KoopaIR储存在str中
// Dump在结合当前语句情况的基础上后序遍历得到koopaIR
// 返回值str主要用于存储变量名、块名等信息
// 该文件中Cout均用于调试，ctrl+H替换即可

/* 屎山重构计划 */
// Block的编号可以将Block0换为Entry

/* AST模板 */
/* class AST : public BaseAST {
    public:
    string Dump(string& str) const override {
    }
}; */

/* 零散未删除代码 */
/* #include <sstream>  //分割字符串用 */
/* getline(is2,last,' ');
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