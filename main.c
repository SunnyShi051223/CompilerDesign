/* main.c */
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"

int main() {
    FILE *fp;
    char code[MAX_CODE_LEN];
    size_t len;

    fp = fopen("Test.txt", "r");

    len = fread(code, 1, MAX_CODE_LEN - 1, fp);
    code[len] = '\0';
    fclose(fp);

    printf("Source Code:\n---------------------\n%s\n---------------------\n\n", code);

    // --- 1. 词法分析过程展示 (二元式输出) ---
    printf("Step 1: Lexical Analysis (Tokens)\n");
    printf("----------------------------------\n");
    initLexer(code);
    Token t;
    do {
        t = getToken();
        if (t.type != TOK_END)
            printf("Token: (<Type:%d>, %s)\n", t.type, t.value);
    } while (t.type != TOK_END);
    printf("----------------------------------\n\n");

    // --- 2. 语法分析 & 语义分析 ---
    printf("Step 2: Syntax & Semantic Analysis\n");
    printf("----------------------------------\n");
    initLexer(code); // 重置
    SLR1_Parser();

    // --- 3. 中间代码输出 ---
    printQuads();

    return 0;
}