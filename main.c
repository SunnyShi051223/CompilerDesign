#include <stdio.h>

/* main.c */
#include <stdio.h>
#include "common.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"

int main() {
    // 测试用例：if ( a > b ) x = 1 ;
    const char *code = "if ( a > b ) x = 1 ;";

    printf("Source Code: %s\n\n", code);

    // 1. 初始化
    initLexer(code);

    // 2. 启动语法分析 (内部调用语义分析)
    SLR1_Parser();

    // 3. 输出四元式
    printQuads();

    return 0;
}
