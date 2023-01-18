#pragma once
#include <map>
#include <string>
#include "koopa.h"
using namespace std;

#define alignment16(a) ((a + 0x0F) & (~0x0F))  // 16对齐(变大)
enum IMPORTANCE {
    PRIMARY,
    SECONDARY
};

static int ptr_frame = 0;
static int minus_frame = 0;

// static map<string, bool> occupyMap;
static map<koopa_raw_value_t, pair<string, IMPORTANCE>> regMap;
static map<koopa_raw_value_t, int> stackMap;
static int empty_reg = 0;
static string getReg();
static void CleanRegMap(const string& reg_str);
static void UpdateRegMap(const koopa_raw_value_t& value, const string& reg_str, const IMPORTANCE& imp);
static string LinkSaveLoad(const string& action, const string& reg_name, int bias);
static string LinkRiscv(const string& action, const string& destination = "", const string& source_left = "", const string& source_right = "");
void Visit(const koopa_raw_program_t& program, std::string& str);  // 访问 raw program
void Visit(const koopa_raw_slice_t& slice, std::string& str);      // 访问 raw slice
void Visit(const koopa_raw_function_t& func, std::string& str);    // 访问函数
void Visit(const koopa_raw_basic_block_t& bb, std::string& str);   // 访问基本块
void Visit(const koopa_raw_value_t& value, std::string& str);      // 访问指令
string Visit(const koopa_raw_store_t& st, string& str);
string Visit(const koopa_raw_load_t& st, string& str);
void Visit(const koopa_raw_return_t& ret, std::string& str);       // 访问return指令
string Visit(const koopa_raw_binary_t& bnry, std::string& str);    // 访问二元运算指令
void Visit(const koopa_raw_branch_t& br, std::string& str);
void Visit(const koopa_raw_jump_t& jp, std::string& str);
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
    // 执行一些其他的必要操作
    string tmp_str_now;
    string func_name(func->name);
    func_name.erase(0, 1);
    assert(!func_name.empty());
    tmp_str_now += func_name;
    tmp_str_now += ":\n";
    for (size_t i = 0; i < func->bbs.len; ++i) {
        koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t)func->bbs.buffer[i];
        for (size_t j = 0; j < bb->insts.len; ++j) {
            koopa_raw_value_t value = (koopa_raw_value_t)bb->insts.buffer[j];
            switch (value->ty->tag) {  // 103行以enum储存
                case KOOPA_RTT_INT32:
                    ptr_frame += 4;  // 4个字节
                    break;
                case KOOPA_RTT_UNIT:
                    ptr_frame += 0;
                    break;
                case KOOPA_RTT_ARRAY:
                    // cout << "Type Array Error, Not Finish Yet" << endl;
                    break;
                case KOOPA_RTT_POINTER:
                    ptr_frame += 4;  // 4个字节
                    break;
                case KOOPA_RTT_FUNCTION:
                    // cout << "Type Func Error, Not Finish Yet" << endl;
                    break;
            }
        }
    }
    // cout << "stack need to -" << ptr_frame << " Bytes at first" << endl;
    minus_frame = -alignment16(ptr_frame);
    // cout << "stack need to " << minus_frame << " Bytes" << endl;
    ptr_frame = 0;
    assert(minus_frame <= 0);
    if (minus_frame >= -2048) {
        tmp_str_now += LinkRiscv("addi", "sp", "sp", to_string(minus_frame));
    } else {
        string tmp_var_name = getReg();
        tmp_str_now += LinkRiscv("li", tmp_var_name, to_string(minus_frame));
        tmp_str_now += LinkRiscv("add", "sp", "sp", tmp_var_name);
    }
    str += tmp_str_now;
    // ...
    // 访问所有基本块
    Visit(func->bbs, str);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t& bb, std::string& str) {
    string block_name(bb->name);
    block_name.erase(0, 1);
    assert(!block_name.empty());
    str += block_name;
    str += ":\n";

    // 执行一些其他的必要操作
    // ...
    // 访问所有指令
    Visit(bb->insts, str);
}

// 访问指令
void Visit(const koopa_raw_value_t& value, std::string& str) {
    // cout << "Visiting Inst" << endl;
    //  根据指令类型判断后续需要如何访问
    //  在这时就应该知道用了什么寄存器了
    const auto& kind = value->kind;
    string tmp_str_reg;  // 不能在switch不同分支中定义同名变量
    switch (kind.tag) {
        case KOOPA_RVT_INTEGER:
            // 访问 integer 指令
            // cout << "KOOPA_RVT_INTEGER" << endl;
            tmp_str_reg = Visit(kind.data.integer, str);
            UpdateRegMap(value, tmp_str_reg, SECONDARY);
            break;
        case KOOPA_RVT_ZERO_INIT:
            // cout << "KOOPA_RVT_ZERO_INIT" << endl;
            break;
        case KOOPA_RVT_UNDEF:
            // cout << "KOOPA_RVT_UNDEF" << endl;
            break;
        case KOOPA_RVT_AGGREGATE:
            // cout << "KOOPA_RVT_AGGREGATE" << endl;
            break;
        case KOOPA_RVT_FUNC_ARG_REF:
            // cout << "KOOPA_RVT_FUNC_ARG_REF" << endl;
            break;
        case KOOPA_RVT_BLOCK_ARG_REF:
            // cout << "KOOPA_RVT_INTEGER" << endl;
            break;
        case KOOPA_RVT_ALLOC:
            // cout << "KOOPA_RVT_ALLOC" << endl;
            if (value->ty->tag == KOOPA_RTT_POINTER) {
                stackMap.insert(make_pair(value, ptr_frame));
                // 分配栈上空间,将value与int绑定
                ptr_frame += 4;  // 该怎么确认这里的数字呢
                                 // cout << "Alloc Int, Allocate Stack Space" << endl;
            }
            break;
        case KOOPA_RVT_GLOBAL_ALLOC:
            // cout << "KOOPA_RVT_GLOBAL_ALLOC" << endl;
            break;
        case KOOPA_RVT_LOAD:
            // cout << "KOOPA_RVT_LOAD" << endl;
            tmp_str_reg = Visit(kind.data.load, str);
            UpdateRegMap(value, tmp_str_reg, PRIMARY);
            stackMap.insert(make_pair(value, ptr_frame));
            ptr_frame += 4;  // 该怎么确认这里的数字呢
            // 分配栈上空间,将value与int绑定
            break;
        case KOOPA_RVT_STORE:
            // cout << "KOOPA_RVT_STORE" << endl;
            Visit(kind.data.store, str);
            break;
        case KOOPA_RVT_GET_PTR:
            // cout << "KOOPA_RVT_GET_PTR" << endl;
            break;
        case KOOPA_RVT_GET_ELEM_PTR:
            // cout << "KOOPA_RVT_GET_ELEM_PTR" << endl;
            break;
        case KOOPA_RVT_BINARY:
            // cout << "KOOPA_RVT_BINARY" << endl;
            //  访问二元运算指令
            tmp_str_reg = Visit(kind.data.binary, str);
            UpdateRegMap(value, tmp_str_reg, PRIMARY);
            stackMap.insert(make_pair(value, ptr_frame));
            ptr_frame += 4;
            break;
        case KOOPA_RVT_BRANCH:
            // cout << "KOOPA_RVT_BRANCH" << endl;
            Visit(kind.data.branch, str);
            break;
        case KOOPA_RVT_JUMP:
            Visit(kind.data.jump, str);
            // cout << "KOOPA_RVT_JUMP" << endl;
            break;
        case KOOPA_RVT_CALL:
            // cout << "KOOPA_RVT_CALL" << endl;
            break;
        case KOOPA_RVT_RETURN:
            // cout << "KOOPA_RVT_RETURN" << endl;
            //  访问 return 指令
            Visit(kind.data.ret, str);
            break;
        default:
            // 其他类型暂时遇不到
            assert(false);
    }
}

// 访问对应类型指令的函数

// 访问整数指令     //不涉及栈
string Visit(const koopa_raw_integer_t& intgr, std::string& str) {
    // cout << "   Visiting Int" << endl;
    if (intgr.value == 0) {
        return "x0";
    }
    string tmp_str_reg = getReg();
    string tmp_str_value = to_string(intgr.value);
    str += LinkRiscv("li", tmp_str_reg, tmp_str_value);
    return tmp_str_reg;
}

// 访问load指令     //待修正
string Visit(const koopa_raw_load_t& ld, string& str) {
    // cout << "   Visiting Load" << endl;
    const auto& src = ld.src;
    assert(src->ty->tag == KOOPA_RTT_POINTER);
    int src_num = stackMap[src];
    // int src_int = src->kind.data.integer.value;
    assert(-2048 <= src_num && src_num < 2048);
    string tmp_str_reg = getReg();
    str += LinkSaveLoad("lw", tmp_str_reg, src_num);
    int dest_num = ptr_frame;
    str += LinkSaveLoad("sw", tmp_str_reg, dest_num);
    // str += LinkRiscv("sw", tmp_str_reg, dest_link, "");
    return tmp_str_reg;
}

// 访问store指令        //待修正
string Visit(const koopa_raw_store_t& st, string& str) {
    // cout << "   Visiting Store" << endl;
    const auto& value = st.value;
    const auto& dest = st.dest;  // dest里面装着啥,指针?
    string tmp_str_value;
    assert(dest->ty->tag == KOOPA_RTT_POINTER);
    if (!regMap.count(value))
        Visit(value, tmp_str_value);
    str += tmp_str_value;
    string tmp_str_reg = regMap[value].first;  // sw只能对寄存器使用
    int dest_num = stackMap[dest];
    // int dest_int = dest->kind.data.integer.value;
    // cout << "dest_num==" << dest_num << endl;
    assert(-2048 <= dest_num && dest_num < 2047);
    str += LinkSaveLoad("sw", tmp_str_reg, dest_num);
    return tmp_str_reg;
}

// 访问二元运算指令(比较、算术、位运算)
// 先从栈中读出，再存回栈上
string Visit(const koopa_raw_binary_t& bnry, std::string& str) {
    // cout << "   Visiting Binary" << endl;
    const auto& left = bnry.lhs;  // const auto& 为只读不拷贝？
    const auto& right = bnry.rhs;
    const auto& op = bnry.op;                   // 利用enum储存
    string left_reg, right_reg, right_num_str;  // right分量有可能是立即数
    if (left->kind.tag == KOOPA_RVT_INTEGER) {
        if (!stackMap.count(left))
            Visit(left, str);
        left_reg = regMap[left].first;
    } else {
        if (!regMap.count(left)) {
            if (!stackMap.count(left))
                Visit(left, str);
            int left_dest_num = stackMap[left];
            left_reg = getReg();
            str += LinkSaveLoad("lw", left_reg, left_dest_num);
        } else
            left_reg = regMap[left].first;
    }
    string tmp_str_right;  // 考虑到在右侧为int型时，部分指令无需加载
    int right_num = right->kind.data.integer.value;
    if (right->kind.tag == KOOPA_RVT_INTEGER) {
        right_num_str = to_string(right_num);
        if (!stackMap.count(right))
            Visit(right, tmp_str_right);
        right_reg = regMap[right].first;
        // str += "right reg is";
        // str += right_reg;
    } else {
        if (!regMap.count(right)) {
            if (!stackMap.count(right))
                Visit(right, str);
            int right_dest_num = stackMap[left];
            right_reg = getReg();
            str += LinkSaveLoad("lw", right_reg, right_dest_num);
        } else
            right_reg = regMap[right].first;
    }
    // 读取左分量的寄存器(右分量也许是立即数)
    string tmp_str_reg;  // 不能在switch中不同case定义同名变量
    switch (op) {
        case KOOPA_RBO_NOT_EQ:  // bool(a^b)-->(a!=b)
            if (right->kind.tag == KOOPA_RVT_INTEGER && -2048 <= right_num && right_num < 2048) {
                // right_num_str = to_string(right->kind.data.integer.value);
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                    tmp_str_reg = left_reg;
                else  // 此时右分量不占有寄存器，所以跳过right
                    tmp_str_reg = getReg();
                str += LinkRiscv("xori", tmp_str_reg, left_reg, right_num_str);  // c = (a ^ b)
            } else {
                str += tmp_str_right;
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                    tmp_str_reg = left_reg;
                else
                    tmp_str_reg = getReg();
                str += LinkRiscv("xor", tmp_str_reg, left_reg, right_reg);  // c = (a ^ b)
            }
            str += LinkRiscv("snez", tmp_str_reg, tmp_str_reg);  // c = (c != 0)即c = bool(c)
            str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
            return tmp_str_reg;
        case KOOPA_RBO_EQ:  // !(a^b)-->(a==b)
            if (right->kind.tag == KOOPA_RVT_INTEGER && -2048 <= right_num && right_num < 2048) {
                // right_num_str = to_string(right->kind.data.integer.value);
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                    tmp_str_reg = left_reg;
                else  // 此时右分量不占有寄存器，所以跳过right
                    tmp_str_reg = getReg();
                str += LinkRiscv("xori", tmp_str_reg, left_reg, right_num_str);  // c = (a ^ b)
            } else {
                str += tmp_str_right;
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                    tmp_str_reg = left_reg;
                else  // 此时右分量不占有寄存器，所以跳过right
                    tmp_str_reg = getReg();
                str += LinkRiscv("xor", tmp_str_reg, left_reg, right_reg);  // c = (a ^ b)
            }
            str += LinkRiscv("seqz", tmp_str_reg, tmp_str_reg);  // c = (c == 0)即c = !c
            str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
            return tmp_str_reg;
        case KOOPA_RBO_GT:
            str += tmp_str_right;
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("sgt", tmp_str_reg, left_reg, right_reg);
            str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
            return tmp_str_reg;
        case KOOPA_RBO_LT:
            str += tmp_str_right;
            //     right_reg = regMap[right].first;
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("slt", tmp_str_reg, left_reg, right_reg);
            str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
            return tmp_str_reg;
        case KOOPA_RBO_GE:  // !(a < b)-->(a >= b)
            str += tmp_str_right;
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("slt", tmp_str_reg, left_reg, right_reg);  // c = (a < b)
            str += LinkRiscv("seqz", tmp_str_reg, tmp_str_reg);         // c = (c == 0)即c = !c
            str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
            return tmp_str_reg;
        case KOOPA_RBO_LE:  // !(a > b)-->(a <= b)
            str += tmp_str_right;
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("sgt", tmp_str_reg, left_reg, right_reg);  // c = (a > b)
            str += LinkRiscv("seqz", tmp_str_reg, tmp_str_reg);         // c = (c == 0)即c = !c
            str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
            return tmp_str_reg;
        case KOOPA_RBO_ADD:
            if (right->kind.tag == KOOPA_RVT_INTEGER && -2048 <= right_num && right_num < 2048) {
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                    tmp_str_reg = left_reg;
                else
                    tmp_str_reg = getReg();
                str += LinkRiscv("addi", tmp_str_reg, left_reg, right_num_str);
                str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
                return tmp_str_reg;
            } else {
                str += tmp_str_right;
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                    tmp_str_reg = left_reg;
                else
                    tmp_str_reg = getReg();
                str += LinkRiscv("add", tmp_str_reg, left_reg, right_reg);
                str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
                return tmp_str_reg;
            }
        case KOOPA_RBO_SUB:
            str += tmp_str_right;
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("sub", tmp_str_reg, left_reg, right_reg);
            str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
            return tmp_str_reg;
        case KOOPA_RBO_MUL:
            str += tmp_str_right;
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("mul", tmp_str_reg, left_reg, right_reg);
            str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
            return tmp_str_reg;
        case KOOPA_RBO_DIV:
            str += tmp_str_right;
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("div", tmp_str_reg, left_reg, right_reg);
            str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
            return tmp_str_reg;
        case KOOPA_RBO_MOD:  // 取余和取模不同 需要修改？ 不用,C++的%就是取余
            str += tmp_str_right;
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("rem", tmp_str_reg, left_reg, right_reg);
            str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
            return tmp_str_reg;
        case KOOPA_RBO_AND:
            if (right->kind.tag == KOOPA_RVT_INTEGER && -2048 <= right_num && right_num < 2048) {
                // right_num_str = to_string(right->kind.data.integer.value);
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                    tmp_str_reg = left_reg;
                else
                    tmp_str_reg = getReg();
                str += LinkRiscv("andi", tmp_str_reg, left_reg, right_num_str);
                str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
                return tmp_str_reg;
            } else {
                str += tmp_str_right;
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                    tmp_str_reg = left_reg;
                else
                    tmp_str_reg = getReg();
                str += LinkRiscv("and", tmp_str_reg, left_reg, right_reg);
                str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
                return tmp_str_reg;
            }
        case KOOPA_RBO_OR:
            if (right->kind.tag == KOOPA_RVT_INTEGER && -2048 <= right_num && right_num < 2048) {
                // right_num_str = to_string(right->kind.data.integer.value);
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                    tmp_str_reg = left_reg;
                else
                    tmp_str_reg = getReg();
                str += LinkRiscv("ori", tmp_str_reg, left_reg, right_num_str);
                str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
                return tmp_str_reg;
            } else {
                str += tmp_str_right;
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                    tmp_str_reg = left_reg;
                else
                    tmp_str_reg = getReg();
                str += LinkRiscv("or", tmp_str_reg, left_reg, right_reg);
                str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
                return tmp_str_reg;
            }
        case KOOPA_RBO_XOR:
            if (right->kind.tag == KOOPA_RVT_INTEGER && -2048 <= right_num && right_num < 2048) {
                // right_num_str = to_string(right->kind.data.integer.value);
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                    tmp_str_reg = left_reg;
                else
                    tmp_str_reg = getReg();
                str += LinkRiscv("xori", tmp_str_reg, left_reg, right_num_str);
                str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
                return tmp_str_reg;
            } else {
                str += tmp_str_right;
                if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                    tmp_str_reg = left_reg;
                else
                    tmp_str_reg = getReg();
                str += LinkRiscv("xor", tmp_str_reg, left_reg, right_reg);
                str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
                return tmp_str_reg;
            }
        case KOOPA_RBO_SHL:
            str += tmp_str_right;
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("sll", tmp_str_reg, left_reg, right_reg);
            str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
            return tmp_str_reg;
        case KOOPA_RBO_SHR:
            str += tmp_str_right;
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("srl", tmp_str_reg, left_reg, right_reg);
            str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
            return tmp_str_reg;
        case KOOPA_RBO_SAR:
            str += tmp_str_right;
            if (left->kind.tag == KOOPA_RVT_INTEGER && left_reg != "x0")
                tmp_str_reg = left_reg;
            else if (right->kind.tag == KOOPA_RVT_INTEGER && right_reg != "x0")
                tmp_str_reg = right_reg;
            else
                tmp_str_reg = getReg();
            str += LinkRiscv("sra", tmp_str_reg, left_reg, right_reg);
            str += LinkSaveLoad("sw", tmp_str_reg, ptr_frame);
            return tmp_str_reg;
        default:
            assert("Visit binary Error!");
            return "";
    }
}

// 访问br指令
void Visit(const koopa_raw_branch_t& br, std::string& str) {
    const auto& cond = br.cond;
    const auto& true_bb = br.true_bb;
    const auto& false_bb = br.false_bb;
    assert(cond->kind.tag != KOOPA_RVT_INTEGER);

    // Step 1: load
    string tmp_str_cond, cond_reg;
    if (!regMap.count(cond)) {
        if (!stackMap.count(cond))
            Visit(cond, tmp_str_cond);
        int cond_dest_num = stackMap[cond];
        cond_reg = getReg();
        tmp_str_cond += LinkSaveLoad("lw", cond_reg, cond_dest_num);
    } else
        cond_reg = regMap[cond].first;

    // Step 2: true part
    string block_name_true(true_bb->name);
    block_name_true.erase(0, 1);
    assert(!block_name_true.empty());
    string tmp_str_true = LinkRiscv("bnez", cond_reg, block_name_true);

    // j->false
    string block_name_false(false_bb->name);
    block_name_false.erase(0, 1);
    assert(!block_name_false.empty());
    string tmp_str_false = LinkRiscv("j", block_name_false);

    str += tmp_str_cond + tmp_str_true + tmp_str_false;
}

// 访问jump指令
void Visit(const koopa_raw_jump_t& jp, std::string& str) {
    const auto& target = jp.target;
    string block_name(target->name);
    block_name.erase(0, 1);
    assert(!block_name.empty());
    str += LinkRiscv("j", block_name);
}

// 访问return指令
void Visit(const koopa_raw_return_t& ret, std::string& str) {
    // cout << "Visiting Return" << endl;
    const auto& value = ret.value;
    // ret.value->kind.data.integer.value;
    //  没加载到寄存器的话需要visit,否则mv即可
    string tmp_str_value;
    if (value) {
        string result_reg;
        // if (!regMap.count(value))
        //     Visit(value, tmp_str_value);
        // result_reg = regMap[value].first;

        if (value->kind.tag == KOOPA_RVT_INTEGER) {
            if (!regMap.count(value))
                Visit(value, tmp_str_value);
            result_reg = regMap[value].first;
        } else {
            if (!regMap.count(value)) {
                if (!stackMap.count(value))
                    Visit(value, tmp_str_value);
                int value_dest_num = stackMap[value];
                result_reg = getReg();
                tmp_str_value += LinkSaveLoad("lw", result_reg, value_dest_num);
            } else
                result_reg = regMap[value].first;
        }
        tmp_str_value += LinkRiscv("mv", "a0", result_reg);
    }
    str += tmp_str_value;

    string tmp_str_now;
    minus_frame = -minus_frame;
    // assert(minus_frame >= 0);
    if (minus_frame < 2048) {
        tmp_str_now += LinkRiscv("addi", "sp", "sp", to_string(minus_frame));
    } else {
        string tmp_var_name = getReg();
        tmp_str_now += LinkRiscv("li", tmp_var_name, to_string(minus_frame));
        tmp_str_now += LinkRiscv("add", "sp", "sp", tmp_var_name);
    }
    str += tmp_str_now;

    str += LinkRiscv("ret");
    return;
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
    if (reg_name.empty()) {
        empty_reg = 0;
        reg_name = getReg();  // 权宜之计
        CleanRegMap(reg_name);
    } else {
        empty_reg++;
    }
    return reg_name;
}

static void UpdateRegMap(const koopa_raw_value_t& value, const string& reg_str, const IMPORTANCE& imp) {
    CleanRegMap(reg_str);
    regMap.insert(make_pair(value, make_pair(reg_str, imp)));
}
static void CleanRegMap(const string& reg_str) {
    if (reg_str != "x0") {
        for (auto iter = regMap.begin(); iter != regMap.end();) {
            if (iter->second.first == reg_str) {
                // std::cout << "reomve from regMap" << endl;
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

static string LinkSaveLoad(const string& action, const string& reg_name, int bias) {
    string tmp_str_ret;
    assert(!action.empty());
    assert(!reg_name.empty());
    if (-2048 <= bias && bias < 2048) {
        tmp_str_ret += action;
        tmp_str_ret += " ";
        tmp_str_ret += reg_name;
        tmp_str_ret += ", ";
        tmp_str_ret += to_string(bias);
        tmp_str_ret += "(sp)\n";
    } else {
        string tmp_str_reg = getReg();
        tmp_str_ret += LinkRiscv("li", tmp_str_reg, to_string(bias));
        tmp_str_ret += LinkRiscv("add", tmp_str_reg, tmp_str_reg, "sp");
        tmp_str_ret += LinkRiscv(action, reg_name, tmp_str_reg);
    }
    return tmp_str_ret;
}

static string LinkRiscv(const string& action, const string& destination, const string& source_left, const string& source_right) {
    string tmp_str_ret;
    assert(!action.empty());
    tmp_str_ret += action;
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
// 需求：先扫描所有语句得到需要的栈空间
// 将VisitInst改为前序遍历:Visit时

/* 零散未删除代码 */
/* int position_space = tmp_str_left.find(' ');
int position_comma = tmp_str_left.find(',');
left_reg = tmp_str_left.substr(position_space + 1, position_comma - position_space - 1); */