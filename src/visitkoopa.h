#pragma once
#include <string>
#include "koopa.h"
using namespace std;

void Visit(const koopa_raw_program_t& program, std::string& str);  // 访问 raw program
void Visit(const koopa_raw_slice_t& slice, std::string& str);      // 访问 raw slice
void Visit(const koopa_raw_function_t& func, std::string& str);  // 访问函数
void Visit(const koopa_raw_basic_block_t& bb, std::string& str);  // 访问基本块
void Visit(const koopa_raw_value_t& value, std::string& str);  // 访问指令
void Visit(const koopa_raw_return_t& ret, std::string& str);  // 访问return指令
void Visit(const koopa_raw_integer_t& intgr, std::string& str);  // 访问整数指令

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
    const auto& kind = value->kind;
    switch (kind.tag) {
        case KOOPA_RVT_RETURN:
            // 访问 return 指令
            Visit(kind.data.ret, str);
            break;
        case KOOPA_RVT_INTEGER:
            // 访问 integer 指令
            Visit(kind.data.integer, str);
            break;
        default:
            // 其他类型暂时遇不到
            assert(false);
    }
}

// 访问对应类型指令的函数定义略
// 视需求自行实现
// ...
// 访问return指令
void Visit(const koopa_raw_return_t& ret, std::string& str) {
    const auto& value = ret.value;
    //ret.value->kind.data.integer.value;
    if (value) {
        Visit(ret.value, str);
    }
    str += "ret\n";
}

//访问整数指令
void Visit(const koopa_raw_integer_t& intgr, std::string& str) {
    str += "li a0, ";    //草草实现以后再改
    str += to_string(intgr.value);
    str += "\n";
}