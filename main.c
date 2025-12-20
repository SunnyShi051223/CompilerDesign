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

    // 请确保 Test.txt 在工作目录下
    fp = fopen("Test.txt", "r");
    if (fp == NULL) {
        // 如果失败，尝试写入一个默认文件
        fp = fopen("Test.txt", "w");
        fprintf(fp, "if(a>b){\n    a=b;\n}\nelse{\n    if(a==b){\n        a=a+1;\n    }\n    else if(a<b){\n        b=b+1;\n    }\n}");
        fclose(fp);
        fp = fopen("Test.txt", "r");
        if(fp == NULL) { perror("File Error"); return 1; }
    }

    len = fread(code, 1, MAX_CODE_LEN - 1, fp);
    code[len] = '\0';
    fclose(fp);

    printf("Source Code:\n%s\n\n", code);

    initLexer(code);
    SLR1_Parser();
    printQuads();

    return 0;
}