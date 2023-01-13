#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include "astdef.h"
#include "koopa.h"
#include "visitkoopa.h"

using namespace std;

int BaseAST::func_count = 0;
int BaseAST::block_count = 0;
int BaseAST::var_count = 0;
// astdef.h中的静态成员变量
// 定义在.h中会引发multiple define

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE* yyin;
extern FILE* yyout;
extern int yyparse(unique_ptr<BaseAST>& ast);

int main(int argc, const char* argv[]) {
    //cout << "Main start!" << endl;

    // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
    // compiler 模式 输入文件 -o 输出文件
    //cout << "parameter prase start!" << flush;
    assert(argc == 5);
    auto mode = argv[1];
    auto input = argv[2];
    auto output = argv[4];
    //cout << "   end!" << endl;

    // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
    //cout << "openfile start!" << flush;
    yyin = fopen(input, "r");
    assert(yyin);
    /* yyout = freopen(output, "w", stdout);
    assert(yyout); */
    //cout << "   end!" << endl;

    // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件
    // 自底向上扫描生成AST抽象语法树
    //cout << "yy parse start!" << flush;
    unique_ptr<BaseAST> ast;
    auto yyparseRet = yyparse(ast);
    assert(!yyparseRet);
    //cout << "   end!" << endl;

    // 调用BaseAST.Dump自顶向下扫描生成Koopa-IR
    //cout << "Dump start!" << flush;
    string koopaStr;
    ast->Dump(koopaStr);
    assert(!koopaStr.empty());
    const char* koopaChar = koopaStr.c_str();

    string modeStr(mode);
    if (modeStr == "-koopa") {
        std::cout << koopaStr;
        return 0;
    }

    // 解析字符串 str, 得到 Koopa IR 程序       Lv2.1源代码照搬
    koopa_program_t program;
    koopa_error_code_t koopaPraseRet = koopa_parse_from_string(koopaChar, &program);
    assert(koopaPraseRet == KOOPA_EC_SUCCESS);                              // 确保解析时没有出错
    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();  // 创建raw program builder, 用来构建raw program
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);    // 将Koopa IR程序转换为raw program
    koopa_delete_program(program);                                          // 释放 Koopa IR 程序占用的内存

    string riscvStr;
    Visit(raw, riscvStr);

    if (modeStr == "-riscv") {
        std::cout << riscvStr;
    }
    if (modeStr == "-koopa-riscv") {
        std::cout << koopaStr;
        std::cout << riscvStr;
    }
    // 处理完成, 释放 raw program builder 占用的内存
    // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
    // 所以不要在 raw program 处理完毕之前释放 builder
    koopa_delete_raw_program_builder(builder);

    return 0;
}

/* 以下为该文件的备注 */
// 流程：解析命令行参数--打开输入输出文件--自底向上扫描(AST)
//      --自顶向上扫描(KoopaIR)--解析KoopaIR字符串--DFS访问KoopaIR程序
// 通过传递koopaStr与riscvStr的引用，将koopa与riscv的代码化为字符串形式
// 本文件中cout用于调试，std::cout为输出，故可以直接ctrl+H将"// Cout"与" Cout"相互替换
// Program下有变量Value和函数Function，Function下有BasicBlock，BasicBlock下有Value

// docker run -it --rm -v C:\Users\Macider\Desktop\sysy-make-template:/root/compiler maxxing/compiler-dev
// C:\Users\Macider\Desktop\sysy-make-template
// cd compiler
// build/compiler -riscv debug/hello.c -o debug/hello.riscv