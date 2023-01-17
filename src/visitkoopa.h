#pragma once
#include <map>
#include <string>
#include "koopa.h"
using namespace std;

// static map<string, bool> occupyMap;
static map<koopa_raw_value_t, string> regMap;
static int empty_reg = 0;
static string getReg();
static void CleanRegMap(const string& reg_str);
static void UpdateRegMap(const koopa_raw_value_t& value, const string& reg_str);
static string LinkRiscv(const string& inst_name, const string& destination = "", const string& source_left = "", const string& source_right = "");
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
            UpdateRegMap(value, tmp_str_reg);
            break;
        case KOOPA_RVT_INTEGER:
            // 访问 integer 指令
            tmp_str_reg = Visit(kind.data.integer, str);
            UpdateRegMap(value, tmp_str_reg);
            break;
        default:
            // 其他类型暂时遇不到
            assert(false);
    }
}

// 访问对应类型指令的函数

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
        str += LinkRiscv("mv", "a0", result_reg);
    }
    str += LinkRiscv("ret");
    return;
}

// 访问二元运算指令(比较、算术、位运算)
string Visit(const koopa_raw_binary_t& bnry, std::string& str) {
    const auto& left = bnry.lhs;  // const auto& 为只读不拷贝？
    const auto& right = bnry.rhs;
    const auto& op = bnry.op;                   // 利用enum储存
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
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                    tmp_str_reg = left_reg;
                else  // 此时右分量不占有寄存器，所以跳过right
                    tmp_str_reg = getReg();
                str += LinkRiscv("xori", tmp_str_reg, left_reg, right_num_str);  // c = (a ^ b)
            } else {
                if (!regMap.count(right))
                    Visit(right, str);
                right_reg = regMap[right];
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                    tmp_str_reg = left_reg;
                else
                    tmp_str_reg = getReg();
                str += LinkRiscv("xor", tmp_str_reg, left_reg, right_reg);  // c = (a ^ b)
            }
            str += LinkRiscv("snez", tmp_str_reg, tmp_str_reg);  // c = (c != 0)即c = bool(c)
            return tmp_str_reg;
        case KOOPA_RBO_EQ:  // !(a^b)-->(a==b)
            if (right->kind.tag == KOOPA_RVT_INTEGER) {
                right_num_str = to_string(right->kind.data.integer.value);
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                    tmp_str_reg = left_reg;
                else  // 此时右分量不占有寄存器，所以跳过right
                    tmp_str_reg = getReg();
                str += LinkRiscv("xori", tmp_str_reg, left_reg, right_num_str);  // c = (a ^ b)
            } else {
                if (!regMap.count(right))
                    Visit(right, str);
                right_reg = regMap[right];
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                    tmp_str_reg = left_reg;
                else  // 此时右分量不占有寄存器，所以跳过right
                    tmp_str_reg = getReg();
                str += LinkRiscv("xor", tmp_str_reg, left_reg, right_reg);  // c = (a ^ b)
            }
            str += LinkRiscv("seqz", tmp_str_reg, tmp_str_reg);  // c = (c == 0)即c = !c
            return tmp_str_reg;
        case KOOPA_RBO_GT:
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("sgt", tmp_str_reg, left_reg, right_reg);
            return tmp_str_reg;
        case KOOPA_RBO_LT:
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("slt", tmp_str_reg, left_reg, right_reg);
            return tmp_str_reg;
        case KOOPA_RBO_GE:  // !(a < b)-->(a >= b)
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("slt", tmp_str_reg, left_reg, right_reg);  // c = (a < b)
            str += LinkRiscv("seqz", tmp_str_reg, tmp_str_reg);         // c = (c == 0)即c = !c
            return tmp_str_reg;
        case KOOPA_RBO_LE:  // !(a > b)-->(a <= b)
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("sgt", tmp_str_reg, left_reg, right_reg);  // c = (a > b)
            str += LinkRiscv("seqz", tmp_str_reg, tmp_str_reg);         // c = (c == 0)即c = !c
            return tmp_str_reg;
        case KOOPA_RBO_ADD:
            if (right->kind.tag == KOOPA_RVT_INTEGER) {
                right_num_str = to_string(right->kind.data.integer.value);
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                    tmp_str_reg = left_reg;
                else
                    tmp_str_reg = getReg();
                str += LinkRiscv("addi", tmp_str_reg, left_reg, right_num_str);
                return tmp_str_reg;
            } else {
                if (!regMap.count(right))
                    Visit(right, str);
                right_reg = regMap[right];
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                    tmp_str_reg = left_reg;
                else
                    tmp_str_reg = getReg();
                str += LinkRiscv("add", tmp_str_reg, left_reg, right_reg);
                return tmp_str_reg;
            }
        case KOOPA_RBO_SUB:
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("sub", tmp_str_reg, left_reg, right_reg);
            return tmp_str_reg;
        case KOOPA_RBO_MUL:
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("mul", tmp_str_reg, left_reg, right_reg);
            return tmp_str_reg;
        case KOOPA_RBO_DIV:
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("div", tmp_str_reg, left_reg, right_reg);
            return tmp_str_reg;
        case KOOPA_RBO_MOD:  // 取余和取模不同 需要修改？ 不用,C++的%就是取余
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("rem", tmp_str_reg, left_reg, right_reg);
            return tmp_str_reg;
        case KOOPA_RBO_AND:
            if (right->kind.tag == KOOPA_RVT_INTEGER) {
                right_num_str = to_string(right->kind.data.integer.value);
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                    tmp_str_reg = left_reg;
                else
                    tmp_str_reg = getReg();
                str += LinkRiscv("andi", tmp_str_reg, left_reg, right_num_str);
                return tmp_str_reg;
            } else {
                if (!regMap.count(right))
                    Visit(right, str);
                right_reg = regMap[right];
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                    tmp_str_reg = left_reg;
                else
                    tmp_str_reg = getReg();
                str += LinkRiscv("and", tmp_str_reg, left_reg, right_reg);
                return tmp_str_reg;
            }
        case KOOPA_RBO_OR:
            if (right->kind.tag == KOOPA_RVT_INTEGER) {
                right_num_str = to_string(right->kind.data.integer.value);
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                    tmp_str_reg = left_reg;
                else
                    tmp_str_reg = getReg();
                str += LinkRiscv("ori", tmp_str_reg, left_reg, right_num_str);
                return tmp_str_reg;
            } else {
                if (!regMap.count(right))
                    Visit(right, str);
                right_reg = regMap[right];
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                    tmp_str_reg = left_reg;
                else
                    tmp_str_reg = getReg();
                str += LinkRiscv("or", tmp_str_reg, left_reg, right_reg);
                return tmp_str_reg;
            }
        case KOOPA_RBO_XOR:
            if (right->kind.tag == KOOPA_RVT_INTEGER) {
                right_num_str = to_string(right->kind.data.integer.value);
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                    tmp_str_reg = left_reg;
                else
                    tmp_str_reg = getReg();
                str += LinkRiscv("xori", tmp_str_reg, left_reg, right_num_str);
                return tmp_str_reg;
            } else {
                if (!regMap.count(right))
                    Visit(right, str);
                right_reg = regMap[right];
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                    tmp_str_reg = left_reg;
                else
                    tmp_str_reg = getReg();
                str += LinkRiscv("xor", tmp_str_reg, left_reg, right_reg);
                return tmp_str_reg;
            }
        case KOOPA_RBO_SHL:
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("sll", tmp_str_reg, left_reg, right_reg);
            return tmp_str_reg;
        case KOOPA_RBO_SHR:
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("srl", tmp_str_reg, left_reg, right_reg);
            return tmp_str_reg;
        case KOOPA_RBO_SAR:
            if (!regMap.count(right))
                Visit(right, str);
            right_reg = regMap[right];
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")  // 短路效应所以写成一行
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("sra", tmp_str_reg, left_reg, right_reg);
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
    string tmp_str_reg = getReg();
    string tmp_str_value = to_string(intgr.value);
    str += LinkRiscv("li", tmp_str_reg, tmp_str_value);
    return tmp_str_reg;
}

static string getReg() {
    string reg_name;
    if (empty_reg < 7) {
        reg_name += "t";
        reg_name += to_string(empty_reg);
    }
    if (7 <= empty_reg && empty_reg < 14) {
        reg_name += "a";
        reg_name += to_string(empty_reg - 6);
    }
    /* assert(!reg_name.empty() && "getReg Error!");
    empty_reg++; */
    if (reg_name.empty()) {
        empty_reg = 0;
        reg_name = getReg();  // 权宜之计
        CleanRegMap(reg_name);
    } else {
        empty_reg++;
    }
    return reg_name;
}
static void UpdateRegMap(const koopa_raw_value_t& value, const string& reg_str) {
    CleanRegMap(reg_str);
    regMap.insert(make_pair(value, reg_str));
}
static void CleanRegMap(const string& reg_str) {
    if (reg_str != "x0") {
        for (auto iter = regMap.begin(); iter != regMap.end();) {
            if (iter->second == reg_str) {
                // cout << "reomve from regMap" << endl;
                iter = regMap.erase(iter);
            } else
                ++iter;
        }
    }
    /* DoSth rd, rs1, rs2
     分配rd时, 已经准备好rs1与rs2, rd覆盖rs无影响,
     下一次取寄存器前, 该次访问已结束,
     若需要updateRegMap则自然隐式调用cleanRegMap */
}
static string LinkRiscv(const string& inst_name, const string& destination, const string& source_left, const string& source_right) {
    string tmp_str_ret;
    assert(!inst_name.empty());
    tmp_str_ret += inst_name;
    if (destination.empty()) {
        tmp_str_ret += "\n";
        return tmp_str_ret;
    }
    tmp_str_ret += " ";
    tmp_str_ret += destination;
    if (source_left.empty()) {
        tmp_str_ret += "\n";
        return tmp_str_ret;
    }
    tmp_str_ret += ", ";
    tmp_str_ret += source_left;
    if (source_right.empty()) {
        tmp_str_ret += "\n";
        return tmp_str_ret;
    }
    tmp_str_ret += ", ";
    tmp_str_ret += source_right;
    tmp_str_ret += "\n";
    return tmp_str_ret;
}

/* 以下为该文件的备注 */
// updateRegMap与getReg均为临时函数
// 该文件中的Cout均用于调试
// CreateRiscv用于连接字符串形成一句riscv代码，后三个参数可缺省

/* 屎山重构计划 */
// updateRegMap函数有些低效
// getReg函数将被重构
// 假设立即数不复用，则立即数的寄存器应当被使用
// getReg中保留有trick,预计将在lv4被优化

/* 零散未删除代码 */
/* int position_space = tmp_str_left.find(' ');
int position_comma = tmp_str_left.find(',');
left_reg = tmp_str_left.substr(position_space + 1, position_comma - position_space - 1); */