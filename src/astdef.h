#pragma once
#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <stack>
#include <string>
#include "symboltable.h"
using namespace std;

enum dump_mode {
    NORMAL_MODE,  // 正常,向传入的str链接新部分,返回字符串
    NUMBER_MODE   // const专用,只计算数值并返回
};
static stack<pair<string, string>> loopStack;  // 储存循环的开始、结尾

// string boolize(BaseAST& ast, string& last);     //bool()
inline bool isInt(const string& str);
inline string LinkKoopa(const string& destination = "", const string& action = "", const string& source1 = "", const string& source2 = "", const string& source3 = "");

// 类的声明
/*
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
 */

// 所有 AST 的基类
class BaseAST {
   public:
    // 对函数、基本块、变量计数,保存直接可用的最小编号,在main.cpp中初始化
    static int func_count;
    static int block_count;
    static int var_count;
    static shared_ptr<SymbolTable> symbol_table;
    virtual ~BaseAST() = default;
    virtual string Dump(string& str, const dump_mode mode = NORMAL_MODE) = 0;
    inline string createVar() {
        string tmp_var_name = "%";
        tmp_var_name += to_string(this->var_count);
        this->var_count++;
        return tmp_var_name;
    }
    inline string createBlock() {  // 只应该在开始、跳转时被用到
        string tmp_block_name = "%Block";
        tmp_block_name += to_string(this->block_count);
        this->block_count++;
        return tmp_block_name;
    }
    string boolize(string& last) {
        if (last == "0" || last == "1")  // bool(0)->0,bool(1)->1
            return "";
        else {
            string tmp_str_ret;
            string tmp_var_name = createVar();
            tmp_str_ret = LinkKoopa(tmp_var_name, "ne", last, "0");
            last = tmp_var_name;
            return tmp_str_ret;
        }
    }
};

class CompUnitAST : public BaseAST {
   public:
    unique_ptr<BaseAST> func_def;
    CompUnitAST(unique_ptr<BaseAST>& func_def) {
        // cout << "CompUnitAST created!" << endl;
        this->func_def = move(func_def);
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in CompUnitAST" << endl;
        func_def->Dump(str, mode);
        // cout << "Dump out CompUnitAST";
        // cout << "\nstr = " << str << endl;
        return "";
    }
};

class DeclAST : public BaseAST {
   public:
    unique_ptr<BaseAST> const_decl_or_var_decl;
    DeclAST(unique_ptr<BaseAST>& const_decl_or_var_decl) {
        // cout << "DeclAST created!" << endl;
        this->const_decl_or_var_decl = move(const_decl_or_var_decl);
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in DeclAST" << endl;
        string tmp_str = const_decl_or_var_decl->Dump(str, mode);
        // cout << "Dump out DeclAST";
        // cout << "\nstr = " << str << endl;
        return tmp_str;
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
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in ConstDeclAST" << endl;
        string tmp_str = myconst_def->Dump(str, NUMBER_MODE);
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
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in BTypeAST" << endl;
        assert(int_str == "int");
        string tmp_str = "i32";
        // cout << "Dump out BTypeAST";
        // cout << "\nstr = " << str << endl;
        return tmp_str;
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
        this->No = 0;
    }
    MyconstDefAST(unique_ptr<BaseAST>& myconst_def, string& comma, unique_ptr<BaseAST>& const_def) {
        // cout << "MyconstDefAST created!" << endl;
        this->myconst_def = move(myconst_def);
        this->const_def = move(const_def);
        this->comma = move(comma);
        this->No = 1;
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
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
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in ConstDefAST" << endl;
        //      计算出该常量，并插入符号表
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
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in ConstInitValAST" << endl;
        //      计算出表达式的值,返回给ConstDef
        string tmp_str = const_exp->Dump(str, mode);
        // cout << "Dump out ConstInitValAST";
        // cout << "\nstr = " << str << endl;
        return tmp_str;
    }
};

class VarDeclAST : public BaseAST {
   public:
    unique_ptr<BaseAST> btype;
    unique_ptr<BaseAST> myvar_def;
    string semicolon;
    VarDeclAST(unique_ptr<BaseAST>& btype, unique_ptr<BaseAST>& myvar_def, string& semicolon) {
        // cout << "VarDeclAST created!" << endl;
        this->btype = move(btype);
        this->myvar_def = move(myvar_def);
        this->semicolon = move(semicolon);
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in VarDeclAST" << endl;
        string tmp_str;
        string tmp_str_type = btype->Dump(tmp_str);
        myvar_def->Dump(tmp_str, mode);
        string now_line;  // 当前行
        istringstream is1(tmp_str);
        while (getline(is1, now_line)) {
            string now_word;  // 当前单词
            istringstream is2(now_line);
            while (is2 >> now_word) {
                if (now_word == "alloc") {  // 该行是变量定义
                    now_line += " ";
                    now_line += tmp_str_type;
                }
            }
            now_line += "\n";
            str += now_line;  // 丑陋但能用的处理手段
        }
        // cout << "Dump out VarDeclAST";
        // cout << "\nstr = " << str << endl;
        return "";
    }
};

class MyvarDefAST : public BaseAST {
   public:
    unique_ptr<BaseAST> var_def;
    unique_ptr<BaseAST> myvar_def;
    string comma;
    int No;
    MyvarDefAST(unique_ptr<BaseAST>& var_def) {
        // cout << "MyvarDefAST created!" << endl;
        this->var_def = move(var_def);
        this->myvar_def = nullptr;
        this->comma = "";
        this->No = 0;
    }
    MyvarDefAST(unique_ptr<BaseAST>& myvar_def, string& comma, unique_ptr<BaseAST>& var_def) {
        // cout << "MyvarDefAST created!" << endl;
        this->myvar_def = move(myvar_def);
        this->var_def = move(var_def);
        this->comma = move(comma);
        this->No = 1;
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in MyvarDefAST" << endl;
        if (No == 0) {
            string tmp_var_def = var_def->Dump(str);
            // cout << "Dump out MyvarDefAST";
            // cout << "\nstr = " << str << endl;
            return "";
        }
        if (No == 1) {
            myvar_def->Dump(str, mode);
            var_def->Dump(str, mode);
            // cout << "Dump out MyvarDefAST";
            // cout << "\nstr = " << str << endl;
            return "";
        }
        return "";
    }
};

class VarDefAST : public BaseAST {
   public:
    string ident;
    string assign;
    unique_ptr<BaseAST> init_val;
    int No;
    VarDefAST(string& ident) {
        // cout << "VarDefAST created!" << endl;
        this->ident = move(ident);
        this->assign = "";
        this->init_val = nullptr;
        this->No = 0;
    }
    VarDefAST(string& ident, string& assign, unique_ptr<BaseAST>& init_val) {
        // cout << "VarDefAST created!" << endl;
        this->ident = move(ident);
        this->assign = move(assign);
        this->init_val = move(init_val);
        this->No = 1;
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in VarDefAST" << endl;
        if (No == 0) {
            string tmp_var_name = "@";
            tmp_var_name += ident;
            this->symbol_table->insertSymbol(ident, tmp_var_name);  // 变量名可能会在加入符号表时改变
            string tmp_str_now;
            if (!this->symbol_table->existUseless(tmp_var_name)) {
                tmp_str_now = LinkKoopa(tmp_var_name, "alloc", "", "");
            }  // 不一定要alloc  // alloc的类型在VarDecl中
            str += tmp_str_now;
            // cout << "Dump out VarDefAST";
            // cout << "\nstr = " << str << endl;
            return "";
        }
        if (No == 1) {
            string tmp_str_init_val;
            string last_init = init_val->Dump(tmp_str_init_val, mode);
            if (!isInt(last_init))
                str += tmp_str_init_val;
            string tmp_var_name = "@";
            tmp_var_name += ident;
            this->symbol_table->insertSymbol(ident, tmp_var_name);
            string tmp_str_now;
            if (!this->symbol_table->existUseless(tmp_var_name)) {
                tmp_str_now = LinkKoopa(tmp_var_name, "alloc", "", "");
            }  // 不一定要alloc
            tmp_str_now += LinkKoopa("", "store", last_init, tmp_var_name);
            str += tmp_str_now;
            // cout << "Dump out VarDefAST";
            // cout << "\nstr = " << str << endl;
            return "";
        }
        return "";
    }
};

class InitValAST : public BaseAST {
   public:
    unique_ptr<BaseAST> exp;
    InitValAST(unique_ptr<BaseAST>& exp) {
        // cout << "InitValAST created!" << endl;
        this->exp = move(exp);
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in InitValAST" << endl;
        string tmp_str = exp->Dump(str, mode);
        // cout << "Dump out InitValAST";
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
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in FuncDefAST" << endl;
        string tmp_str_now;
        tmp_str_now += "fun @";
        tmp_str_now += ident;
        this->func_count++;  // 先用再加，从0开始
        assert(round_left == "(" && round_right == ")");
        tmp_str_now += "(): ";
        string tmp_str_type = func_type->Dump(tmp_str_now, mode);
        tmp_str_now += tmp_str_type;
        tmp_str_now += " {\n";
        string tmp_block_name = createBlock();
        tmp_str_now += tmp_block_name + ":\n";
        string block_name_last = block->Dump(tmp_str_now, mode);
        tmp_str_now += "}\n";
        str += tmp_str_now;
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
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in FuncTypeAST" << endl;
        assert(int_str == "int");
        string tmp_str = "i32";
        // cout << "Dump out FuncTypeAST";
        // cout << "\nstr = " << str << endl;
        return tmp_str;
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
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in BlockAST" << endl;
        assert(curly_left == "{" && curly_right == "}");
        string tmp_str;
        auto tmp_map = this->symbol_table->uselessMap;
        this->symbol_table = shared_ptr<SymbolTable>(new SymbolTable(this->symbol_table));  // 新建符号表
        this->symbol_table->uselessMap = tmp_map;
        string block_name = myblock_item->Dump(tmp_str, mode);
        this->symbol_table->parent->uselessMap = this->symbol_table->symbolMap;
        // cout << "this->symbol_table->symbolMap.size==" << this->symbol_table->symbolMap.size() << endl;
        // cout << "this->symbol_table->parent->uselessMap.size==" << this->symbol_table->parent->uselessMap.size() << endl;
        this->symbol_table = this->symbol_table->parent;                        // 删除?符号表
        this->symbol_table->uselessMap.insert(tmp_map.begin(), tmp_map.end());  // 合并uselessMap
        // this->symbol_table->printSymbolTable();
        str += tmp_str;
        // cout << "Dump out BlockAST";
        // cout << "\nstr = " << str << endl;
        return block_name;
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
        this->No = 0;
    }
    MyblockItemAST(unique_ptr<BaseAST>& myblock_item, unique_ptr<BaseAST>& block_item) {
        // cout << "MyblockItemAST created!" << endl;
        this->myblock_item = move(myblock_item);
        this->block_item = move(block_item);
        this->No = 1;
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in MyblockItemAST" << endl;
        if (No == 0) {
            // cout << "Do Nothing" << endl;
            // cout << "Dump out MyblockItemAST";
            // cout << "\nstr = " << str << endl;
            return "";
        }
        if (No == 1) {
            string tmp_str1 = myblock_item->Dump(str, mode);
            if (!tmp_str1.empty())
                str += tmp_str1 + ":\n";  // 层层传上来的Block名字
            string tmp_str2 = block_item->Dump(str, mode);
            // cout << "Dump out MyblockItemAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str2;
        }
        return "";
    }
};

class BlockItemAST : public BaseAST {  // 该怎么处理需要好好深思
   public:
    unique_ptr<BaseAST> decl_or_stmt;
    BlockItemAST(unique_ptr<BaseAST>& decl_or_stmt) {
        // cout << "BlockItemAST created!" << endl;
        this->decl_or_stmt = move(decl_or_stmt);
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in BlockItemAST" << endl;
        string tmp_str = decl_or_stmt->Dump(str, mode);
        // cout << "Dump out BlockItemAST";
        // cout << "\nstr = " << str << endl;
        return tmp_str;
    }
};

class StmtAST : public BaseAST {
   public:
    unique_ptr<BaseAST> match_stmt_or_open_stmt;
    StmtAST(unique_ptr<BaseAST>& match_stmt_or_open_stmt) {
        // cout << "StmtAST created!" << endl;
        this->match_stmt_or_open_stmt = move(match_stmt_or_open_stmt);
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in StmtAST" << endl;
        string block_name = match_stmt_or_open_stmt->Dump(str, mode);
        // cout << "Dump out StmtAST";
        // cout << "\nstr = " << str << endl;
        return block_name;
    }
};

class MatchStmtAST : public BaseAST {
   public:
    string if_str;
    string round_left;
    unique_ptr<BaseAST> exp;
    string round_right;
    unique_ptr<BaseAST> match_stmt_1;
    string else_str;
    unique_ptr<BaseAST> match_stmt_2;
    unique_ptr<BaseAST> other;
    int No;
    MatchStmtAST(string& if_str, string& round_left, unique_ptr<BaseAST>& exp, string& round_right, unique_ptr<BaseAST>& match_stmt_1, string& else_str, unique_ptr<BaseAST>& match_stmt_2) {
        // cout << "MatchStmtAST created!" << endl;
        this->if_str = move(if_str);
        this->round_left = move(round_left);
        this->exp = move(exp);
        this->round_right = move(round_right);
        this->match_stmt_1 = move(match_stmt_1);
        this->else_str = move(else_str);
        this->match_stmt_2 = move(match_stmt_2);
        this->No = 0;
    }
    MatchStmtAST(unique_ptr<BaseAST>& other) {
        // cout << "MatchStmtAST created!" << endl;
        this->other = move(other);
        this->No = 1;
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in MatchStmtAST" << endl;
        if (No == 0) {
            string tmp_str_now, str_exp, str_then, str_else;
            // ↓遍历exp then else 分量并赋名
            string last_exp = exp->Dump(str_exp, NUMBER_MODE);  // 此处应当尝试直接求值
            if (isInt(last_exp)) {
                int exp_num = stoi(last_exp.c_str());
                if (exp_num) {                                            // then-end
                    string block_name_1 = match_stmt_1->Dump(str, mode);  // then
                    return block_name_1;
                    // if (!block_name_1.empty())
                    //     return block_name_1;
                } else {                                                  // else-end
                    string block_name_2 = match_stmt_2->Dump(str, mode);  // else
                    return block_name_2;
                    // if (!block_name_2.empty())
                    //     return block_name_2 + ":\n";
                }
                return "";
            } else
                tmp_str_now += str_exp;

            string block_name_then = createBlock();                    // 先确定基本块名字再去遍历
            string block_name_1 = match_stmt_1->Dump(str_then, mode);  // then
            string block_name_else = createBlock();
            string block_name_2 = match_stmt_2->Dump(str_else, mode);  // else
            string block_name_end = createBlock();

            // ↓链接then分量
            tmp_str_now += LinkKoopa("", "br", last_exp, block_name_then, block_name_else);
            tmp_str_now += block_name_then + ":\n";
            tmp_str_now += str_then;
            if (!block_name_1.empty())
                tmp_str_now += block_name_1 + ":\n";  // 防止then以ret结尾
            tmp_str_now += LinkKoopa("", "jump", block_name_end);
            // ↓链接else分量
            tmp_str_now += block_name_else + ":\n";
            tmp_str_now += str_else;
            if (!block_name_2.empty())
                tmp_str_now += block_name_2 + ":\n";  // 防止else以return结尾
            tmp_str_now += LinkKoopa("", "jump", block_name_end);
            // ↓准备链接end分量
            // tmp_str_now += block_name_end + ":\n";
            str += tmp_str_now;
            // cout << "Dump out MatchStmtAST";
            // cout << "\nstr = " << str << endl;
            return block_name_end;
        }
        if (No == 1) {
            string tmp_str_now;
            string block_name = other->Dump(tmp_str_now, mode);
            str += tmp_str_now;
            // str += block_name + ":\n";
            // 这里也许需要+=blockname,在正确的情况下加
            // cout << "Dump out MatchStmtAST";
            // cout << "\nstr = " << str << endl;
            //  return tmp_block_name;
            return block_name;
        }
        return "";
    }
};

class OtherStmtAST : public BaseAST {
   public:
    unique_ptr<BaseAST> lval;
    string assign;
    unique_ptr<BaseAST> exp;
    string semicolon;
    unique_ptr<BaseAST> block;
    string while_str;
    string round_left;
    string break_str;
    string round_right;
    unique_ptr<BaseAST> stmt;
    string continue_str;
    string return_str;
    int No;
    OtherStmtAST(unique_ptr<BaseAST>& lval, string& assign, unique_ptr<BaseAST>& exp, string& semicolon) {
        // cout << "OtherStmtAST created!" << endl;
        this->lval = move(lval);
        this->assign = move(assign);
        this->exp = move(exp);
        this->semicolon = move(semicolon);
        this->return_str = "";
        this->No = 0;
    }
    OtherStmtAST(unique_ptr<BaseAST>& exp, string& semicolon) {
        // cout << "OtherStmtAST created!" << endl;
        this->exp = move(exp);
        this->semicolon = move(semicolon);
        this->lval = nullptr;
        this->return_str = "";
        this->No = 1;
    }
    OtherStmtAST(string& semicolon) {
        // cout << "OtherStmtAST created!" << endl;
        this->semicolon = move(semicolon);
        this->return_str = "";
        this->No = 2;
    }
    OtherStmtAST(unique_ptr<BaseAST>& block) {
        // cout << "OtherStmtAST created!" << endl;
        this->block = move(block);
        this->No = 3;
    }
    OtherStmtAST(string& while_str, string& round_left, unique_ptr<BaseAST>& exp, string& round_right, unique_ptr<BaseAST>& stmt) {
        // cout << "OtherStmtAST created!" << endl;
        this->while_str = move(while_str);
        this->round_left = move(round_left);
        this->exp = move(exp);
        this->round_right = move(round_right);
        this->stmt = move(stmt);
        this->No = 4;
    }
    OtherStmtAST(string& break_or_continue_or_return, string& semicolon) {
        // cout << "OtherStmtAST created!" << endl;
        if (break_or_continue_or_return == "break") {
            this->break_str = move(break_or_continue_or_return);
            this->semicolon = move(semicolon);
            this->No = 5;
            // cout << "         5" << endl;
        } else if (break_or_continue_or_return == "continue") {
            this->continue_str = move(break_or_continue_or_return);
            this->semicolon = move(semicolon);
            this->No = 6;
            // cout << "         6" << endl;
        } else if (break_or_continue_or_return == "return") {
            this->return_str = move(break_or_continue_or_return);
            this->semicolon = move(semicolon);
            this->No = 8;
        }
    }
    OtherStmtAST(string& return_str, unique_ptr<BaseAST>& exp, string& semicolon) {
        // cout << "OtherStmtAST created!" << endl;
        this->return_str = move(return_str);
        this->exp = move(exp);
        this->semicolon = move(semicolon);
        this->No = 7;
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in OtherStmtAST" << endl;
        if (No == 0) {
            string tmp_str_exp;
            string last_lval = lval->Dump(tmp_str_exp, mode);  // lval不会链接
            // 变量在左侧其实不需要load
            assert(last_lval[0] == '@');
            string last_exp = exp->Dump(tmp_str_exp, mode);
            if (!isInt(last_exp))
                str += tmp_str_exp;
            string tmp_str_now = LinkKoopa("", "store", last_exp, last_lval);
            str += tmp_str_now;
            // cout << "Dump out OtherStmtAST";
            // cout << "\nstr = " << str << endl;
            return "";
        }
        if (No == 1) {
            exp->Dump(str, mode);
            // cout << "Dump out OtherStmtAST";
            // cout << "\nstr = " << str << endl;
            return "";
        }
        if (No == 2) {
            // cout << "Do Nothing" << endl;
            // cout << "Dump out OtherStmtAST";
            // cout << "\nstr = " << str << endl;
            return "";
        }
        if (No == 3) {
            string block_name_next = block->Dump(str, mode);
            // cout << "Dump out OtherStmtAST";
            // cout << "\nstr = " << str << endl;
            return block_name_next;
        }
        if (No == 4) {
            string tmp_str_now, last_exp, str_exp, str_stmt;
            string block_name_1 = createBlock();  // 入口
            tmp_str_now += LinkKoopa("", "jump", block_name_1);
            tmp_str_now += block_name_1 + ":\n";
            last_exp = exp->Dump(str_exp, mode);
            if (!isInt(last_exp))
                tmp_str_now += str_exp;
            string block_name_2 = createBlock();
            string block_name_4 = createBlock();  // 出口
            loopStack.push(make_pair(block_name_1, block_name_4));
            /*// cout << "Push" << block_name_1 << " and " << block_name_4 << endl; */
            string block_name_3 = stmt->Dump(str_stmt, mode);
            tmp_str_now += LinkKoopa("", "br", last_exp, block_name_2, block_name_4);
            tmp_str_now += block_name_2 + ":\n";
            tmp_str_now += str_stmt;
            if (!block_name_3.empty())
                tmp_str_now += block_name_3 + ":\n";
            tmp_str_now += LinkKoopa("", "jump", block_name_1);
            str += tmp_str_now;
            loopStack.pop();
            // last_exp要有
            // 循环:
            // exp_str
            // br last_exp,循环体,循环外
            // 循环体:Stmt
            // 循环外:
            // cout << "Dump out OtherStmtAST";
            // cout << "\nstr = " << str << endl;
            return block_name_4;
        }
        if (No == 5) {  // break
            assert(break_str == "break");
            assert(!loopStack.empty());
            // cout << "BREAKKKKKKKKK" << endl;
            // cout << "Stack.top.first=" << loopStack.top().first << endl;
            // cout << "Stack.top.second=" << loopStack.top().second << endl;
            string block_to_jump = loopStack.top().second;
            str += LinkKoopa("", "jump", block_to_jump);
            string block_name_next = createBlock();
            str += block_name_next + ":\n";
            // cout << "Dump out OtherStmtAST";
            // cout << "\nstr = " << str << endl;
            return "";
            // return block_name_next;
        }
        if (No == 6) {  // continue
            assert(continue_str == "continue");
            // cout << "CONTINNNNNNNNUE" << endl;
            assert(!loopStack.empty());
            // cout << "Stack.top.first=" << loopStack.top().first << endl;
            // cout << "Stack.top.second=" << loopStack.top().second << endl;
            string block_to_jump = loopStack.top().first;
            str += LinkKoopa("", "jump", block_to_jump);
            string block_name_next = createBlock();
            str += block_name_next + ":\n";
            return "";
            // cout << "Dump out OtherStmtAST";
            // cout << "\nstr = " << str << endl;
            // return block_name_next;
        }
        if (No == 7) {
            // cout << "\nTest:str = " << str << endl;
            string tmp_str_exp, last_exp;
            last_exp = exp->Dump(tmp_str_exp, mode);
            if (!isInt(last_exp))
                str += tmp_str_exp;
            assert(return_str == "return");
            string tmp_str_now = LinkKoopa("", "ret", last_exp, "");
            assert(semicolon == ";");
            string block_name_next = createBlock();
            // tmp_str_now += block_name_next + ":\n";   //直接加会留下最后一个空基本块
            str += tmp_str_now;
            // cout << "Dump out OtherStmtAST";
            // cout << "\nstr = " << str << endl;
            return block_name_next;
        }
        if (No == 8) {
            string tmp_str_now = LinkKoopa("", "ret", "0", "");
            str += tmp_str_now;
            string block_name_next = createBlock();
            // tmp_str_now += block_name_next + ":\n";   //直接加会留下最后一个空基本块
            // cout << "Dump out OtherStmtAST";
            // cout << "\nstr = " << str << endl;
            return block_name_next;
        }
        return "";
    }
};

class OpenStmtAST : public BaseAST {
   public:
    string if_str;
    string round_left;
    unique_ptr<BaseAST> exp;
    string round_right;
    unique_ptr<BaseAST> stmt;
    unique_ptr<BaseAST> match_stmt;
    string else_str;
    unique_ptr<BaseAST> open_stmt;
    int No;
    OpenStmtAST(string& if_str, string& round_left, unique_ptr<BaseAST>& exp, string& round_right, unique_ptr<BaseAST>& stmt) {
        // cout << "OpenStmtAST created!" << endl;
        this->if_str = move(if_str);
        this->round_left = move(round_left);
        this->exp = move(exp);
        this->round_right = move(round_right);
        this->stmt = move(stmt);
        this->No = 0;
    }
    OpenStmtAST(string& if_str, string& round_left, unique_ptr<BaseAST>& exp, string& round_right, unique_ptr<BaseAST>& match_stmt, string& else_str, unique_ptr<BaseAST>& open_stmt) {
        // cout << "OpenStmtAST created!" << endl;
        this->if_str = move(if_str);
        this->round_left = move(round_left);
        this->exp = move(exp);
        this->round_right = move(round_right);
        this->match_stmt = move(match_stmt);
        this->else_str = move(else_str);
        this->open_stmt = move(open_stmt);
        this->No = 1;
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in OpenStmtAST" << endl;
        if (No == 0) {
            string tmp_str_now, str_exp, str_then, str_else;
            // ↓遍历exp then分量并赋名
            string last_exp = exp->Dump(str_exp, mode);
            if (isInt(last_exp)) {
                int exp_num = stoi(last_exp.c_str());
                if (exp_num) {                                    // then-end
                    string block_name_1 = stmt->Dump(str, mode);  // then
                    return block_name_1;
                    // if (!block_name_1.empty())
                    //     return block_name_1 + ":\n";
                }
                return "";
            } else
                tmp_str_now += str_exp;
            string block_name_then = createBlock();
            string block_name_1 = stmt->Dump(str_then, mode);  // then
            string block_name_end = createBlock();
            // ↓链接then分量
            tmp_str_now += LinkKoopa("", "br", last_exp, block_name_then, block_name_end);
            tmp_str_now += block_name_then + ":\n";
            tmp_str_now += str_then;  // 考虑str_then可能以ret结尾
            if (!block_name_1.empty())
                tmp_str_now += block_name_1 + ":\n";  // 防止then以ret结尾
            tmp_str_now += LinkKoopa("", "jump", block_name_end);
            // ↓准备链接end分量
            // tmp_str_now += block_name_end + ":\n";
            str += tmp_str_now;
            // cout << "Dump out OpenStmtAST";
            // cout << "\nstr = " << str << endl;
            return block_name_end;
        }
        if (No == 1) {
            string tmp_str_now, str_exp, str_then, str_else;
            // ↓遍历exp then else分量并赋名
            string last_exp = exp->Dump(str_exp, mode);
            if (isInt(last_exp)) {
                int exp_num = stoi(last_exp.c_str());
                if (exp_num) {                                          // then-end
                    string block_name_1 = match_stmt->Dump(str, mode);  // then
                    return block_name_1;
                    // if (!block_name_1.empty())
                    //     return block_name_1 + ":\n";
                } else {                                               // else-end
                    string block_name_2 = open_stmt->Dump(str, mode);  // else
                    return block_name_2;
                    // if (!block_name_2.empty())
                    //     return block_name_2 + ":\n";
                }
                return "";
            } else
                tmp_str_now += str_exp;
            string block_name_then = createBlock();
            string block_name_1 = match_stmt->Dump(str_then, mode);  // then
            string block_name_else = createBlock();
            string block_name_2 = open_stmt->Dump(str_else, mode);  // else
            string block_name_end = createBlock();
            // ↓链接then分量
            tmp_str_now += LinkKoopa("", "br", last_exp, block_name_then, block_name_else);
            tmp_str_now += block_name_then + ":\n";
            tmp_str_now += str_then;
            if (!block_name_1.empty())
                tmp_str_now += block_name_1 + ":\n";  // 防止then以ret结尾
            tmp_str_now += LinkKoopa("", "jump", block_name_end);
            // ↓链接else分量
            tmp_str_now += block_name_else + ":\n";
            tmp_str_now += str_else;
            if (!block_name_2.empty())
                tmp_str_now += block_name_2 + ":\n";  // 防止else以ret结尾
            tmp_str_now += LinkKoopa("", "jump", block_name_end);
            // ↓准备链接end分量
            // tmp_str_now += block_name_end + ":\n";    //直接加会产生空块
            str += tmp_str_now;
            // cout << "Dump out OpenStmtAST";
            // cout << "\nstr = " << str << endl;
            return block_name_end;
        }
        // cout << "Dump out OpenStmtAST";
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
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in ExpAST" << endl;
        string tmp_str = lor_exp->Dump(str, mode);
        // cout << "Dump out ExpAST";
        // cout << "\nstr = " << str << endl;
        return tmp_str;
    }
};

class LValAST : public BaseAST {  // 调用者进行load操作
   public:
    string ident;
    LValAST(string& ident) {
        // cout << "LValAST created!" << endl;
        this->ident = move(ident);
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {  // 返回一个last值(数字str/变量名)
        // cout << "Dump in LValAST" << endl;
        // this->symbol_table->printSymbolTable();
        shared_ptr<symbol_type> symbol = this->symbol_table->getSymbol(ident);
        if (holds_alternative<int>(*symbol)) {
            string tmp_str_num = to_string(get<int>(*symbol));  // 储存着const变量对应的值
            // cout << "Dump out LValAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str_num;
        }
        if (holds_alternative<string>(*symbol)) {
            string tmp_var_name = get<string>(*symbol);  // 储存着const变量对应的值
            // cout << "Dump out LValAST";
            // cout << "\nstr = " << str << endl;
            return tmp_var_name;
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
        this->No = 0;
    }
    PrimaryExpAST(unique_ptr<BaseAST>& lval) {
        // cout << "PrimaryExpAST created!" << endl;
        this->lval = move(lval);
        this->round_left = "";
        this->exp = nullptr;
        this->round_right = "";
        this->No = 1;
    }
    PrimaryExpAST(int number) {
        // cout << "PrimaryExpAST created!" << endl;
        this->number = number;
        this->round_left = "";
        this->exp = nullptr;
        this->round_right = "";
        this->lval = nullptr;
        this->No = 2;
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in PrimaryExpAST" << endl;
        if (No == 0) {
            string tmp_str = exp->Dump(str, mode);  // exp不会进行链接
            // cout << "Dump out PrimaryExpAST";
            // cout << "\nstr = " << str << endl;
            return tmp_str;
        }
        if (No == 1) {
            string last_lval = lval->Dump(str, mode);  // lval不会进行链接
            if (isInt(last_lval)) {
                // cout << "Dump out PrimaryExpAST";
                // cout << "\nstr = " << str << endl;
                return last_lval;
            } else {
                // assert(mode != NUMBER_MODE);
                string tmp_var_name = createVar();
                string tmp_str_now = LinkKoopa(tmp_var_name, "load", last_lval, "");
                str += tmp_str_now;
                // cout << "Dump out PrimaryExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
        }
        if (No == 2) {
            if (mode == NUMBER_MODE) {
                int num_primary = number;
                string tmp_str_num = to_string(num_primary);
                // cout << "Dump out UnaryExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_str_num;
            } else {
                string tmp_str = to_string(number);
                str += tmp_str;
                // cout << "Dump out PrimaryExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_str;
            }
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
        this->No = 0;
    }
    UnaryExpAST(string& unary_op, unique_ptr<BaseAST>& unary_exp) {
        // cout << "UnaryExpAST created!" << endl;
        this->primary_exp = nullptr;
        this->unary_op = move(unary_op);
        this->unary_exp = move(unary_exp);
        this->No = 1;
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
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
            if (mode == NUMBER_MODE && isInt(last_unary)) {
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
                string tmp_var_name = createVar();
                tmp_str_now += LinkKoopa(tmp_var_name, "sub", "0", last_unary);
                str += tmp_str_now;
                // cout << "Dump out UnaryExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
            if (unary_op == "!") {
                string tmp_var_name = createVar();
                tmp_str_now += LinkKoopa(tmp_var_name, "eq", last_unary, "0");
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
        this->No = 0;
    }
    MulExpAST(unique_ptr<BaseAST>& mul_exp, string& binary_op, unique_ptr<BaseAST>& unary_exp) {
        // cout << "MulExpAST created!" << endl;
        this->mul_exp = move(mul_exp);
        this->binary_op = move(binary_op);
        this->unary_exp = move(unary_exp);
        this->No = 1;
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
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
            if (mode == NUMBER_MODE && isInt(last_mul) && isInt(last_unary)) {
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
                string tmp_var_name = createVar();
                tmp_str_now += LinkKoopa(tmp_var_name, "mul", last_mul, last_unary);
                str += tmp_str_now;
                // cout << "Dump out MulExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
            if (binary_op == "/") {
                string tmp_var_name = createVar();
                tmp_str_now += LinkKoopa(tmp_var_name, "div", last_mul, last_unary);
                str += tmp_str_now;
                // cout << "Dump out MulExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
            if (binary_op == "%") {
                string tmp_var_name = createVar();
                tmp_str_now += LinkKoopa(tmp_var_name, "mod", last_mul, last_unary);
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
        this->No = 0;
    }
    AddExpAST(unique_ptr<BaseAST>& add_exp, string& binary_op, unique_ptr<BaseAST>& mul_exp) {
        // cout << "AddExpAST created!" << endl;
        this->add_exp = move(add_exp);
        this->binary_op = move(binary_op);
        this->mul_exp = move(mul_exp);
        this->No = 1;
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
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
            if (mode == NUMBER_MODE && isInt(last_mul) && isInt(last_add)) {
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
                string tmp_var_name = createVar();
                tmp_str_now += LinkKoopa(tmp_var_name, "add", last_add, last_mul);
                str += tmp_str_now;
                // cout << "Dump out AddExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
            if (binary_op == "-") {
                string tmp_var_name = createVar();
                tmp_str_now += LinkKoopa(tmp_var_name, "sub", last_add, last_mul);
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
        this->No = 0;
    }
    RelExpAST(unique_ptr<BaseAST>& rel_exp, string& binary_op, unique_ptr<BaseAST>& add_exp) {
        // cout << "RelExpAST created!" << endl;
        this->rel_exp = move(rel_exp);
        this->binary_op = move(binary_op);
        this->add_exp = move(add_exp);
        this->No = 1;
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
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
            if (mode == NUMBER_MODE && isInt(last_rel) && isInt(last_add)) {
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
                string tmp_var_name = createVar();
                tmp_str_now += LinkKoopa(tmp_var_name, "lt", last_rel, last_add);
                str += tmp_str_now;
                // cout << "Dump out RelExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
            if (binary_op == ">") {
                string tmp_var_name = createVar();
                tmp_str_now += LinkKoopa(tmp_var_name, "gt", last_rel, last_add);
                str += tmp_str_now;
                // cout << "Dump out RelExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
            if (binary_op == "<=") {
                string tmp_var_name = createVar();
                tmp_str_now += LinkKoopa(tmp_var_name, "le", last_rel, last_add);
                str += tmp_str_now;
                // cout << "Dump out RelExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
            if (binary_op == ">=") {
                string tmp_var_name = createVar();
                tmp_str_now += LinkKoopa(tmp_var_name, "ge", last_rel, last_add);
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
        this->No = 0;
    }
    EqExpAST(unique_ptr<BaseAST>& eq_exp, string& binary_op, unique_ptr<BaseAST>& rel_exp) {
        // cout << "EqExpAST created!" << endl;
        this->eq_exp = move(eq_exp);
        this->binary_op = move(binary_op);
        this->rel_exp = move(rel_exp);
        this->No = 1;
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
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
            if (mode == NUMBER_MODE && isInt(last_eq) && isInt(last_rel)) {
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
                string tmp_var_name = createVar();
                tmp_str_now += LinkKoopa(tmp_var_name, "eq", last_eq, last_rel);
                str += tmp_str_now;
                // cout << "Dump out EqExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_var_name;
            }
            if (binary_op == "!=") {
                string tmp_var_name = createVar();
                tmp_str_now += LinkKoopa(tmp_var_name, "ne", last_eq, last_rel);
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
        this->No = 0;
    }
    LAndExpAST(unique_ptr<BaseAST>& land_exp, string& binary_op, unique_ptr<BaseAST>& eq_exp) {
        // cout << "LAndExpAST created!" << endl;
        this->land_exp = move(land_exp);
        this->binary_op = move(binary_op);
        this->eq_exp = move(eq_exp);
        this->No = 1;
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
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
            if (mode == NUMBER_MODE && isInt(last_land) && isInt(last_eq)) {
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
            // 以下需要短路求值
            if (binary_op == "&&") {  // bool(a)&bool(b)-->a&&b
                string tmp_var_name = createVar();
                tmp_str_now += LinkKoopa(tmp_var_name, "and", last_land, last_eq);
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
        this->No = 0;
    }
    LOrExpAST(unique_ptr<BaseAST>& lor_exp, string& binary_op, unique_ptr<BaseAST>& land_exp) {
        // cout << "LOrExpAST created!" << endl;
        this->lor_exp = move(lor_exp);
        this->binary_op = move(binary_op);
        this->land_exp = move(land_exp);
        this->No = 1;
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
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
            if (mode == NUMBER_MODE && isInt(last_lor) && isInt(last_land)) {
                int num_lor = stoi(last_lor.c_str());
                int num_land = stoi(last_land.c_str());
                string tmp_str_num = to_string(num_lor || num_land);
                // cout << "Dump out LOrExpAST";
                // cout << "\nstr = " << str << endl;
                return tmp_str_num;
            }

            // 以下需要短路求值
            if (binary_op == "||") {  // bool(a)|bool(b)-->a||b
                // int result = 1;
                // if (lhs == 0) {
                //     result = rhs != 0;
                // }
                // 先不考虑优化，建立好对应的语句
                // %1 = eq, last_lor, 0
                // br %1, ==0, !=0
                // ==0:
                // %2 = ne, last_land, 0
                // !=0:

                // return varname
                if (!isInt(last_lor))
                    str += tmp_str_lor;

                // string tmp_var_name = createVar();
                // tmp_str_now += LinkKoopa(tmp_var_name, "eq", last_lor, "0");
                // string tmp_block_true = createBlock();
                // string tmp_block_false = createBlock();
                // tmp_str_now += LinkKoopa("", "br", tmp_var_name, tmp_block_true, tmp_block_false);
                // tmp_str_now += tmp_block_true + ":\n";
                // tmp_var_name = createVar();
                if (!isInt(last_land))
                    str += tmp_str_land;
                // 涉及布尔运算，需要先将分量转为0/1
                tmp_str_now += boolize(last_lor);   // 将last_lor转为bool类型
                tmp_str_now += boolize(last_land);  // 将last_land转为bool类型

                string tmp_var_name = createVar();
                tmp_str_now += LinkKoopa(tmp_var_name, "or", last_lor, last_land);
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
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
        // cout << "Dump in ConstExpAST" << endl;
        string tmp_str = exp->Dump(str, mode);
        // cout << "Dump out ConstExpAST";
        // cout << "\nstr = " << str << endl;
        return tmp_str;
    }
};

inline string LinkKoopa(const string& destination, const string& action, const string& source1, const string& source2, const string& source3) {
    string tmp_str_ret;
    if (!destination.empty()) {
        tmp_str_ret += destination;
        tmp_str_ret += " = ";
    }
    if (!action.empty()) {
        tmp_str_ret += action;
    } else {
        tmp_str_ret += "Link Error";
        // cout << "Link Error" << endl;
    }
    if (!source1.empty()) {
        tmp_str_ret += " ";
        tmp_str_ret += source1;
    }
    if (!source2.empty()) {
        tmp_str_ret += ", ";
        tmp_str_ret += source2;
    }
    if (!source3.empty()) {
        tmp_str_ret += ", ";
        tmp_str_ret += source3;
    }
    tmp_str_ret += "\n";
    return tmp_str_ret;
}

inline bool isInt(const string& str) {
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
// 通过不同的构造函数实现A-->B|C，为在Dump时区分而引入了No记号(B.No=0,C.No=1)
// 成员变量定义顺序依照产生式中出现的顺序
// 构造函数参数顺序依照对应产生式中出现的顺序
// Dump时，传string引用将KoopaIR储存在str中
// Dump后序遍历并综合连接子节点的str得到koopaIR
// 返回值str主要用于存储变量名、块名、num_str等信息
// 该文件中// cout均用于调试，ctrl+H替换即可

/* 屎山重构计划 */
// Block的编号可以将Block0换为Entry
// !的部分可以考虑加入对0的特判，不过好像用处不大
// primary.number怎么初始化？考虑将Number变成AST
// 不要反复alloc，同名变量在同一层用过的话可以接着用那块储存空间

/* AST模板 */
/* class AST : public BaseAST {
    public:
    AST(){
    }
    string Dump(string& str, const dump_mode mode = NORMAL_MODE) override {
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
