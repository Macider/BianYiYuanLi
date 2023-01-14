#pragma once
#include <map>
#include <string>
#include "koopa.h"
using namespace std;

static map<koopa_raw_value_t, string> regMap;
// static map<string, bool> occupyMap;
static int empty_reg = 0;
string getReg();
void updateRegMap(const koopa_raw_value_t& value, const string& reg_str);
void Visit(const koopa_raw_program_t& program, std::string& str);  // 访问 raw program
void Visit(const koopa_raw_slice_t& slice, std::string& str);      // 访问 raw slice
void Visit(const koopa_raw_function_t& func, std::string& str);    // 访问函数
void Visit(const koopa_raw_basic_block_t& bb, std::string& str);   // 访问基本块
void Visit(const koopa_raw_value_t& value, std::string& str);      // 访问指令
void Visit(const koopa_raw_return_t& ret, std::string& str);       // 访问return指令
string Visit(const koopa_raw_binary_t& bnry, std::string& str);    // 访问二元运算指令
string Visit(const koopa_raw_integer_t& intgr, std::string& str);  // 访问整数指令

// 访问 raw program
void Visit(const koopa_raw_program_t& program, std::string& str) {
    // 执行一些其他的必要操作
    // ...
    // 访问所有全局变量
    Visit(program.values, str);
    // 访问所有函数
    str += ".text\n";
    str += ".global main\n";
    Visit(program.funcs, str);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t& slice, std::string& str) {
    for (size_t i = 0; i < slice.len; ++i) {
        auto ptr = slice.buffer[i];
        // 根据 slice 的 kind 决定将 ptr 视作何种元素
        switch (slice.kind) {
            case KOOPA_RSIK_FUNCTION:  // 访问函数
                Visit(reinterpret_cast<koopa_raw_function_t>(ptr), str);
                break;
            case KOOPA_RSIK_BASIC_BLOCK:  // 访问基本块
                Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr), str);
                break;
            case KOOPA_RSIK_VALUE:  // 访问指令
                Visit(reinterpret_cast<koopa_raw_value_t>(ptr), str);
                break;
            default:  // 我们暂时不会遇到其他内容, 于是不对其做任何处理
                assert(false);
        }
    }
}

// 访问函数
void Visit(const koopa_raw_function_t& func, std::string& str) {
    string func_name(func->name);
    func_name.erase(0, 1);
    assert(!func_name.empty());
    str += func_name;
    str += ":\n";
    // 执行一些其他的必要操作
    // ...
    // 访问所有基本块
    Visit(func->bbs, str);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t& bb, std::string& str) {
    // 执行一些其他的必要操作
    // ...
    // 访问所有指令
    Visit(bb->insts, str);
}

// 访问指令
void Visit(const koopa_raw_value_t& value, std::string& str) {
    // 根据指令类型判断后续需要如何访问
    // 在这时就应该知道用了什么寄存器了
    const auto& kind = value->kind;
    string tmp_str_reg;  // 不能在switch不同分支中定义同名变量
    switch (kind.tag) {
        case KOOPA_RVT_RETURN:
            // 访问 return 指令
            Visit(kind.data.ret, str);
            break;
        case KOOPA_RVT_BINARY:
            // 访问二元运算指令
            tmp_str_reg = Visit(kind.data.binary, str);
            updateRegMap(value, tmp_str_reg);
            break;
        case KOOPA_RVT_INTEGER:
            // 访问 integer 指令
            tmp_str_reg = Visit(kind.data.integer, str);
            updateRegMap(value, tmp_str_reg);
            break;
        default:
            // 其他类型暂时遇不到
            assert(false);
    }
}

// 访问对应类型指令的函数
// ...
// 访问return指令
void Visit(const koopa_raw_return_t& ret, std::string& str) {
    // cout << "Visiting value" << endl;
    const auto& value = ret.value;
    // ret.value->kind.data.integer.value;
    //  没加载到寄存器的话需要visit,否则mv即可
    if (value) {
        if (!regMap.count(value))
            Visit(value, str);
        string result_reg = regMap[value];
        str += "mv a0, ";
        str += result_reg;
        str += "\n";
    }
    str += "ret\n";
    return;
}

// 访问二元运算指令(比较、算术、位运算)
string Visit(const koopa_raw_binary_t& bnry, std::string& str) {
    const auto& left = bnry.lhs;  // const auto& 为只读不拷贝？
    const auto& right = bnry.rhs;
    const auto& op = bnry.op;  // 利用enum储存
    string tmp_str_now;
    string left_reg, right_reg, right_num_str;  // right分量有可能是立即数
    if (!regMap.count(left))
        Visit(left, str);
    left_reg = regMap[left];
    // 读取左分量的寄存器(右分量也许是立即数)
    string tmp_str_reg;  // 不能在switch中不同case定义同名变量
    switch (op) {
        case KOOPA_RBO_NOT_EQ:  // bool(a^b)-->(a!=b)
            if (right->kind.tag == KOOPA_RVT_INTEGER) {
                right_num_str = to_string(right->kind.data.integer.value);
                tmp_str_now += "xori ";  // c = (a ^ b)
                tmp_str_reg = getReg();
                tmp_str_now += tmp_str_reg;
                tmp_str_now += ", ";
                tmp_str_now += left_reg;
                tmp_str_now += ", ";
                tmp_str_now += right_num_str;
                tmp_str_now += "\n";
            } else {
                if (!regMap.count(right))
                    Visit(right, str);
                right_reg = regMap[right];
                tmp_str_now += "xor ";  // c = (a ^ b)
                tmp_str_reg = getReg();
                tmp_str_now += tmp_str_reg;
                tmp_str_now += ", ";
                tmp_str_now += left_reg;
                tmp_str_now += ", ";
                tmp_str_now += right_reg;
                tmp_str_now += "\n";
            }
            tmp_str_now += "snez ";  // c = (c != 0)即c = bool(c)
            tmp_str_now += tmp_str_reg;
            tmp_str_now += ", ";
            tmp_str_now += tmp_str_reg;
            tmp_str_now += "\n";
            str += tmp_str_now;
            return tmp_str_reg;
        case KOOPA_RBO_EQ:  // !(a^b)-->(a==b)
            if (right->kind.tag == KOOPA_RVT_INTEGER) {
                right_num_str = to_string(right->kind.data.integer.value);
                tmp_str_now += "xori ";  // c = (a ^ b)
                tmp_str_reg = getReg();
                tmp_str_now += tmp_str_reg;
                tmp_str_now += ", ";
                tmp_str_now += left_reg;
                tmp_str_now += ", ";
                tmp_str_now += right_num_str;
                tmp_str_now += "\n";
            } else {
                if (!regMap.count(right))
                    Visit(right, str);
                right_reg = regMap[right];
                tmp_str_now += "xor ";  // c = (a ^ b)
                tmp_str_reg = getReg();
                tmp_str_now += tmp_str_reg;
                tmp_str_now += ", ";
                tmp_str_now += left_reg;
                tmp_str_now += ", ";
                tmp_str_now += right_reg;
                tmp_str_now += "\n";
            }
            tmp_str_now += "seqz ";  // c = (c == 0)即c = !c
            tmp_str_now += tmp_str_reg;
            tmp_str_now += ", ";
            tmp_str_now += tmp_str_reg;
            tmp_str_now += "\n";
            str += tmp_str_now;
            return tmp_str_reg;
        case KOOPA_RBO_GT:
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            tmp_str_now += "sgt ";
            tmp_str_reg = getReg();
            tmp_str_now += tmp_str_reg;
            tmp_str_now += ", ";
            tmp_str_now += left_reg;
            tmp_str_now += ", ";
            tmp_str_now += right_reg;
            tmp_str_now += "\n";
            str += tmp_str_now;
            return tmp_str_reg;
        case KOOPA_RBO_LT:
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            tmp_str_now += "slt ";
            tmp_str_reg = getReg();
            tmp_str_now += tmp_str_reg;
            tmp_str_now += ", ";
            tmp_str_now += left_reg;
            tmp_str_now += ", ";
            tmp_str_now += right_reg;
            tmp_str_now += "\n";
            str += tmp_str_now;
            return tmp_str_reg;
        case KOOPA_RBO_GE:          // !(a < b)-->(a >= b)
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            tmp_str_now += "slt ";  // c = (a < b)
            tmp_str_reg = getReg();
            tmp_str_now += tmp_str_reg;
            tmp_str_now += ", ";
            tmp_str_now += left_reg;
            tmp_str_now += ", ";
            tmp_str_now += right_reg;
            tmp_str_now += "\n";
            tmp_str_now += "seqz ";  // c = (c == 0)即c = !c
            tmp_str_now += tmp_str_reg;
            tmp_str_now += ", ";
            tmp_str_now += tmp_str_reg;
            tmp_str_now += "\n";
            str += tmp_str_now;
            return tmp_str_reg;
        case KOOPA_RBO_LE:          // !(a > b)-->(a <= b)
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            tmp_str_now += "sgt ";  // c = (a > b)
            tmp_str_reg = getReg();
            tmp_str_now += tmp_str_reg;
            tmp_str_now += ", ";
            tmp_str_now += left_reg;
            tmp_str_now += ", ";
            tmp_str_now += right_reg;
            tmp_str_now += "\n";
            tmp_str_now += "seqz ";  // c = (c == 0)即c = !c
            tmp_str_now += tmp_str_reg;
            tmp_str_now += ", ";
            tmp_str_now += tmp_str_reg;
            tmp_str_now += "\n";
            str += tmp_str_now;
            return tmp_str_reg;
        case KOOPA_RBO_ADD:
            if (right->kind.tag == KOOPA_RVT_INTEGER) {
                right_num_str = to_string(right->kind.data.integer.value);
                tmp_str_now += "addi ";
                tmp_str_reg = getReg();
                tmp_str_now += tmp_str_reg;
                tmp_str_now += ", ";
                tmp_str_now += left_reg;
                tmp_str_now += ", ";
                tmp_str_now += right_num_str;
                tmp_str_now += "\n";
                str += tmp_str_now;
                return tmp_str_reg;
            } else {
                if (!regMap.count(right))
                    Visit(right, str);
                right_reg = regMap[right];
                tmp_str_now += "add ";
                tmp_str_reg = getReg();
                tmp_str_now += tmp_str_reg;
                tmp_str_now += ", ";
                tmp_str_now += left_reg;
                tmp_str_now += ", ";
                tmp_str_now += right_reg;
                tmp_str_now += "\n";
                str += tmp_str_now;
                return tmp_str_reg;
            }
        case KOOPA_RBO_SUB:
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            tmp_str_now += "sub ";
            tmp_str_reg = getReg();
            tmp_str_now += tmp_str_reg;
            tmp_str_now += ", ";
            tmp_str_now += left_reg;
            tmp_str_now += ", ";
            tmp_str_now += right_reg;
            tmp_str_now += "\n";
            str += tmp_str_now;
            return tmp_str_reg;
        case KOOPA_RBO_MUL:
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            tmp_str_now += "mul ";
            tmp_str_reg = getReg();
            tmp_str_now += tmp_str_reg;
            tmp_str_now += ", ";
            tmp_str_now += left_reg;
            tmp_str_now += ", ";
            tmp_str_now += right_reg;
            tmp_str_now += "\n";
            str += tmp_str_now;
            return tmp_str_reg;
        case KOOPA_RBO_DIV:
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            tmp_str_now += "div ";
            tmp_str_reg = getReg();
            tmp_str_now += tmp_str_reg;
            tmp_str_now += ", ";
            tmp_str_now += left_reg;
            tmp_str_now += ", ";
            tmp_str_now += right_reg;
            tmp_str_now += "\n";
            str += tmp_str_now;
            return tmp_str_reg;
        case KOOPA_RBO_MOD:  // 取余和取模不同 需要修改？ 不用,C++的%就是取余
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            tmp_str_now += "rem ";
            tmp_str_reg = getReg();
            tmp_str_now += tmp_str_reg;
            tmp_str_now += ", ";
            tmp_str_now += left_reg;
            tmp_str_now += ", ";
            tmp_str_now += right_reg;
            tmp_str_now += "\n";
            str += tmp_str_now;
            return tmp_str_reg;
        case KOOPA_RBO_AND:
            if (right->kind.tag == KOOPA_RVT_INTEGER) {
                right_num_str = to_string(right->kind.data.integer.value);
                tmp_str_now += "andi ";
                tmp_str_reg = getReg();
                tmp_str_now += tmp_str_reg;
                tmp_str_now += ", ";
                tmp_str_now += left_reg;
                tmp_str_now += ", ";
                tmp_str_now += right_num_str;
                tmp_str_now += "\n";
                str += tmp_str_now;
                return tmp_str_reg;
            } else {
                if (!regMap.count(right))
                    Visit(right, str);
                right_reg = regMap[right];
                tmp_str_now += "and ";
                tmp_str_reg = getReg();
                tmp_str_now += tmp_str_reg;
                tmp_str_now += ", ";
                tmp_str_now += left_reg;
                tmp_str_now += ", ";
                tmp_str_now += right_reg;
                tmp_str_now += "\n";
                str += tmp_str_now;
                return tmp_str_reg;
            }
        case KOOPA_RBO_OR:
            if (right->kind.tag == KOOPA_RVT_INTEGER) {
                right_num_str = to_string(right->kind.data.integer.value);
                tmp_str_now += "ori ";
                tmp_str_reg = getReg();
                tmp_str_now += tmp_str_reg;
                tmp_str_now += ", ";
                tmp_str_now += left_reg;
                tmp_str_now += ", ";
                tmp_str_now += right_num_str;
                tmp_str_now += "\n";
                str += tmp_str_now;
                return tmp_str_reg;
            } else {
                if (!regMap.count(right))
                    Visit(right, str);
                right_reg = regMap[right];
                tmp_str_now += "or ";
                tmp_str_reg = getReg();
                tmp_str_now += tmp_str_reg;
                tmp_str_now += ", ";
                tmp_str_now += left_reg;
                tmp_str_now += ", ";
                tmp_str_now += right_reg;
                tmp_str_now += "\n";
                str += tmp_str_now;
                return tmp_str_reg;
            }
        case KOOPA_RBO_XOR:
            if (right->kind.tag == KOOPA_RVT_INTEGER) {
                right_num_str = to_string(right->kind.data.integer.value);
                tmp_str_now += "xori ";
                tmp_str_reg = getReg();
                tmp_str_now += tmp_str_reg;
                tmp_str_now += ", ";
                tmp_str_now += left_reg;
                tmp_str_now += ", ";
                tmp_str_now += right_num_str;
                tmp_str_now += "\n";
                str += tmp_str_now;
                return tmp_str_reg;
            } else {
                if (!regMap.count(right))
                    Visit(right, str);
                right_reg = regMap[right];
                tmp_str_now += "or ";
                tmp_str_reg = getReg();
                tmp_str_now += tmp_str_reg;
                tmp_str_now += ", ";
                tmp_str_now += left_reg;
                tmp_str_now += ", ";
                tmp_str_now += right_reg;
                tmp_str_now += "\n";
                str += tmp_str_now;
                return tmp_str_reg;
            }
        case KOOPA_RBO_SHL:
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            tmp_str_now += "sll ";
            tmp_str_reg = getReg();
            tmp_str_now += tmp_str_reg;
            tmp_str_now += ", ";
            tmp_str_now += left_reg;
            tmp_str_now += ", ";
            tmp_str_now += right_reg;
            tmp_str_now += "\n";
            str += tmp_str_now;
            return tmp_str_reg;
        case KOOPA_RBO_SHR:
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            tmp_str_now += "srl ";
            tmp_str_reg = getReg();
            tmp_str_now += tmp_str_reg;
            tmp_str_now += ", ";
            tmp_str_now += left_reg;
            tmp_str_now += ", ";
            tmp_str_now += right_reg;
            tmp_str_now += "\n";
            str += tmp_str_now;
            return tmp_str_reg;
        case KOOPA_RBO_SAR:
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            tmp_str_now += "sra ";
            tmp_str_reg = getReg();
            tmp_str_now += tmp_str_reg;
            tmp_str_now += ", ";
            tmp_str_now += left_reg;
            tmp_str_now += ", ";
            tmp_str_now += right_reg;
            tmp_str_now += "\n";
            str += tmp_str_now;
            return tmp_str_reg;
        default:
            assert("Visit binary Error!");
            return "";
    }
}

// 访问整数指令
string Visit(const koopa_raw_integer_t& intgr, std::string& str) {
    if (intgr.value == 0) {
        return "x0";
    }
    str += "li ";
    string tmp_str_reg = getReg();
    str += tmp_str_reg;
    str += ", ";
    str += to_string(intgr.value);
    str += "\n";
    return tmp_str_reg;
}

string getReg() {
    string reg_name;
    if (empty_reg < 7) {
        reg_name += "t";
        reg_name += to_string(empty_reg);
    }
    if (7 <= empty_reg && empty_reg < 14) {
        reg_name += "a";
        reg_name += to_string(empty_reg - 6);
    }
    //assert(!reg_name.empty() && "getReg Error!");
    if (reg_name.empty()){
        empty_reg = 0;
        reg_name = getReg();    //权宜之计
    }else{
        empty_reg++;
    }
    return reg_name;
}
void updateRegMap(const koopa_raw_value_t& value, const string& reg_str) {
    if (reg_str != "x0") {
        for (auto iter = regMap.begin(); iter != regMap.end();) {
            if (iter->second == reg_str) {
                // cout << "reomve from regMap" << endl;
                iter = regMap.erase(iter);
            } else
                ++iter;
        }
    }
    // 清除该寄存器内的数据
    regMap.insert(make_pair(value, reg_str));
}

/* 以下为该文件的备注 */
// updateRegMap与getReg均为临时函数
// 该文件中的Cout均用于调试

/* 屎山重构计划 */
// updateRegMap函数有些低效
// getReg函数将被重构
// 考虑优化add->addi

/* 零散未删除代码 */
/* int position_space = tmp_str_left.find(' ');
int position_comma = tmp_str_left.find(',');
left_reg = tmp_str_left.substr(position_space + 1, position_comma - position_space - 1); */