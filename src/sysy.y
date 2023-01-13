%code requires {
  #include <iostream>
  #include <memory>
  #include <string>
  #include "astdef.h"
  using namespace std;
}
%code provides {
  int yylex();
  void yyerror(unique_ptr<BaseAST> &ast, const char *s);
}
%{

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针     //AST即抽象语法树
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况                    //不允许
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN LESS_EQUAL GREATER_EQUAL EQUAL NOT_EQUAL AND OR NOT LESS GREATER ASSIGN ADD SUBTRACT MULTIPLY DIVIDE MODULE
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp
%type <int_val> Number
%type <str_val> UnaryOp

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit: FuncDef {
      /* auto comp_unit = make_unique<CompUnitAST>();
      comp_unit->func_def = unique_ptr<BaseAST>($1);
      ast = move(comp_unit); */
    //cout << "CompUnit-->FuncDef" << endl;
    auto func_def = unique_ptr<BaseAST>($1);
    ast = make_unique<CompUnitAST>(func_def);
  }

// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
FuncDef: FuncType IDENT '(' ')' Block {
    //auto ast = new FuncDefAST();
    //ast->func_type = unique_ptr<BaseAST>($1);
    //ast->ident = *unique_ptr<string>($2);
    //ast->block = unique_ptr<BaseAST>($5);
    //cout << "FuncDef-->FuncType IDENT ( ) Block" << endl;
    auto func_type = unique_ptr<BaseAST>($1);
    auto ident = *unique_ptr<string>($2);
    auto block = unique_ptr<BaseAST>($5);
    auto ast = new FuncDefAST(func_type, ident, block);
    $$ = ast;
  }

// 同上, 不再解释
FuncType: INT {
    //cout << "FuncType-->i32" << endl;
    auto ast = new FuncTypeAST();
    $$ = ast;
  }

Block: '{' Stmt '}' {
    //cout << "Block-->{ Stmt }" << endl;
    auto stmt = unique_ptr<BaseAST>($2);
    auto ast = new BlockAST(stmt);
    $$ = ast;
  }

Stmt: RETURN Exp ';' {
    /* auto number = $2;
    auto ast = new StmtAST(number); */
    //cout << "Stmt-->RETURN Exp ;" <<endl;
    auto exp = unique_ptr<BaseAST>($2);
    auto ast = new StmtAST(exp);
    $$ = ast;
  }

Exp: LOrExp {
    //cout << "Exp-->UnaryExp" << endl;
    auto lor_exp = unique_ptr<BaseAST>($1);
    auto ast = new ExpAST(lor_exp);
    $$ = ast;
  }

PrimaryExp: '(' Exp ')' {
    //No==0
    //cout << "PrimaryExp-->( Exp )" << endl;
    auto exp = unique_ptr<BaseAST>($2);
    auto ast = new PrimaryExpAST(exp);
    $$ = ast;
  }
  | Number {
    //No==1
    //cout << "PrimaryExp-->Number" << endl;
    auto number = $1;
    auto ast = new PrimaryExpAST(number);
    $$ = ast;
  }

Number: INT_CONST {
    //cout << "Number-->" << int($1) << endl;
    //$$ = new string(to_string($1));
    auto ast = int($1);
    $$ = ast;
  }

UnaryExp: PrimaryExp {
    //No==0
    //cout << "UnaryExp-->PrimaryExp" << endl;
    auto primary_exp = unique_ptr<BaseAST>($1);
    auto ast = new UnaryExpAST(primary_exp);
    $$ = ast;
  }
  | UnaryOp UnaryExp {
    //No==1
    //cout << "UnaryExp-->UnaryOp UnaryExp" << endl;
    auto unary_op = *unique_ptr<string>($1);
    auto unary_exp = unique_ptr<BaseAST>($2);
    auto ast = new UnaryExpAST(unary_op, unary_exp);
    $$ = ast;
  }

UnaryOp: ADD {
    //cout << "UnaryOp-->+" << endl; 
    string* ast = new string("+"); 
    $$ = ast;
  }
  | SUBTRACT {
    //cout << "UnaryOp-->-" << endl; 
    string* ast = new string("-"); 
    $$ = ast;
  }
  | NOT {
    //cout << "UnaryOp-->!" << endl; 
    string* ast = new string("!"); 
    $$ = ast;
  }

MulExp: UnaryExp {
    //No==0
    //cout << "MulExp-->UnaryExp" << endl; 
    auto unary_exp = unique_ptr<BaseAST>($1);
    auto ast = new MulExpAST(unary_exp);
    $$ = ast;
  }
  | MulExp MULTIPLY UnaryExp{
    //No==1
    //cout << "MulExp-->MulExp * UnaryExp" << endl; 
    auto mul_exp = unique_ptr<BaseAST>($1);
    string binary_op("*");
    auto unary_exp = unique_ptr<BaseAST>($3);
    auto ast = new MulExpAST(mul_exp, binary_op, unary_exp);
    $$ = ast;
  }
  | MulExp DIVIDE UnaryExp{
    //No==1
    //cout << "MulExp-->MulExp / UnaryExp" << endl; 
    auto mul_exp = unique_ptr<BaseAST>($1);
    string binary_op("/");
    auto unary_exp = unique_ptr<BaseAST>($3);
    auto ast = new MulExpAST(mul_exp, binary_op, unary_exp);
    $$ = ast;
  }
  | MulExp MODULE UnaryExp{
    //No==1
    //cout << "MulExp-->MulExp % UnaryExp" << endl; 
    auto mul_exp = unique_ptr<BaseAST>($1);
    string binary_op("%");
    auto unary_exp = unique_ptr<BaseAST>($3);
    auto ast = new MulExpAST(mul_exp, binary_op, unary_exp);
    $$ = ast;
  }

AddExp: MulExp {
    //No==0
    //cout << "AddExp-->MulExp" << endl; 
    auto mul_exp = unique_ptr<BaseAST>($1);
    auto ast = new AddExpAST(mul_exp);
    $$ = ast;
  }
  | AddExp ADD MulExp{
    //No==1
    //cout << "AddExp-->AddExp + MulExp" << endl; 
    auto add_exp = unique_ptr<BaseAST>($1);
    string binary_op("+");
    auto mul_exp = unique_ptr<BaseAST>($3);
    auto ast = new AddExpAST(add_exp, binary_op, mul_exp);
    $$ = ast;
  }
  | AddExp SUBTRACT MulExp{
    //No==1
    //cout << "AddExp-->AddExp - MulExp" << endl; 
    auto add_exp = unique_ptr<BaseAST>($1);
    string binary_op("-");
    auto mul_exp = unique_ptr<BaseAST>($3);
    auto ast = new AddExpAST(add_exp, binary_op, mul_exp);
    $$ = ast;
  }

RelExp: AddExp{
    auto add_exp = unique_ptr<BaseAST>($1);
    auto ast = new RelExpAST(add_exp);
    $$ = ast;
  }
  | RelExp LESS AddExp{
    auto rel_exp = unique_ptr<BaseAST>($1);
    string binary_op("<");
    auto add_exp = unique_ptr<BaseAST>($3);
    auto ast = new RelExpAST(rel_exp, binary_op, add_exp);
    $$ = ast;
  }
  | RelExp GREATER AddExp{
    auto rel_exp = unique_ptr<BaseAST>($1);
    string binary_op(">");
    auto add_exp = unique_ptr<BaseAST>($3);
    auto ast = new RelExpAST(rel_exp, binary_op, add_exp);
    $$ = ast;
  }
  | RelExp LESS_EQUAL AddExp{
    auto rel_exp = unique_ptr<BaseAST>($1);
    string binary_op("<=");
    auto add_exp = unique_ptr<BaseAST>($3);
    auto ast = new RelExpAST(rel_exp, binary_op, add_exp);
    $$ = ast;
  }
  | RelExp GREATER_EQUAL AddExp{
    auto rel_exp = unique_ptr<BaseAST>($1);
    string binary_op(">=");
    auto add_exp = unique_ptr<BaseAST>($3);
    auto ast = new RelExpAST(rel_exp, binary_op, add_exp);
    $$ = ast;
  }

EqExp: RelExp{
    auto rel_exp = unique_ptr<BaseAST>($1);
    auto ast = new EqExpAST(rel_exp);
    $$ = ast;
  }
  | EqExp EQUAL RelExp{
    auto eq_exp = unique_ptr<BaseAST>($1);
    string binary_op("==");
    auto rel_exp = unique_ptr<BaseAST>($3);
    auto ast = new EqExpAST(eq_exp, binary_op, rel_exp);
    $$ = ast;
  }
  | EqExp NOT_EQUAL RelExp{
    auto eq_exp = unique_ptr<BaseAST>($1);
    string binary_op("!=");
    auto rel_exp = unique_ptr<BaseAST>($3);
    auto ast = new EqExpAST(eq_exp, binary_op, rel_exp);
    $$ = ast;
  }

LAndExp: EqExp{
    auto eq_exp = unique_ptr<BaseAST>($1);
    auto ast = new LAndExpAST(eq_exp);
    $$ = ast;
  }
  | LAndExp AND EqExp{
    auto land_exp = unique_ptr<BaseAST>($1);
    string binary_op("&&");
    auto eq_exp = unique_ptr<BaseAST>($3);
    auto ast = new LAndExpAST(land_exp, binary_op, eq_exp);
    $$ = ast;
  }

LOrExp: LAndExp{
    auto land_exp = unique_ptr<BaseAST>($1);
    auto ast = new LOrExpAST(land_exp);
    $$ = ast;
  }
  | LOrExp OR LAndExp{
    auto lor_exp = unique_ptr<BaseAST>($1);
    string binary_op("||");
    auto land_exp = unique_ptr<BaseAST>($3);
    auto ast = new LOrExpAST(lor_exp, binary_op, land_exp);
    $$ = ast;
  }
%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}

/* 以下为该文件的备注 */
// .y文件用于Bison语法分析，即生成AST(Abstract syntax tree 抽象语法树)
// 自底向上地分析，生成最右推导，具体过程//cout即可见
// AST类被储存在astdef.h中，即AST define，具体包括%type <ast_val>一行与CompUnit的AST
// 该文件中的//cout均用于调试，故可以直接ctrl+H将Cout与//Cout相互替换

/* 屎山重构计划 */
// ×似乎会有bug 应当尝试将new换为make_unique，待尝试