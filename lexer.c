/* lexer.c */
#include "lexer.h"
#include <ctype.h>

static const char *src;
static int pos = 0;

void initLexer(const char *source) {
    src = source;
    pos = 0;
}

Token getToken() {
    Token t;
    char ch = src[pos];

    // 1. 跳过空白符
    while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
        pos++;
        ch = src[pos];
    }

    // 2. 处理结束符
    if (ch == '\0') {
        t.type = TOK_END;
        strcpy(t.value, "$");
        return t;
    }

    // 3. 识别标识符 (ID) 和 关键字 (Keyword)
    if (isalpha(ch)) {
        int i = 0;
        while (isalnum(ch)) {
            if (i < 31) t.value[i++] = ch; // 防止溢出
            pos++;
            ch = src[pos];
        }
        t.value[i] = '\0';

        if (strcmp(t.value, "if") == 0) t.type = TOK_IF;
        else if (strcmp(t.value, "else") == 0) t.type = TOK_ELSE;
        else t.type = TOK_ID;
        return t;
    }

    // 4. 识别数字 (NUM)
    if (isdigit(ch)) {
        int i = 0;
        while (isdigit(ch)) {
            if (i < 31) t.value[i++] = ch;
            pos++;
            ch = src[pos];
        }
        t.value[i] = '\0';
        t.type = TOK_NUM;
        return t;
    }

    // 5. 识别运算符和界符
    pos++; // 先移动指针，预备读取下一个
    switch (ch) {
        // --- 赋值与相等 ---
        case '=':
            if (src[pos] == '=') {
                pos++;
                t.type = TOK_RELOP;
                strcpy(t.value, "==");
            } else {
                t.type = TOK_ASSIGN;
                strcpy(t.value, "=");
            }
            break;

            // --- 大于 & 大于等于 ---
        case '>':
            if (src[pos] == '=') {
                pos++;
                t.type = TOK_RELOP;
                strcpy(t.value, ">=");
            } else {
                t.type = TOK_RELOP;
                strcpy(t.value, ">");
            }
            break;

            // --- 小于 & 小于等于 ---
        case '<':
            if (src[pos] == '=') {
                pos++;
                t.type = TOK_RELOP;
                strcpy(t.value, "<=");
            } else {
                t.type = TOK_RELOP;
                strcpy(t.value, "<");
            }
            break;

            // --- 不等于 (新增) ---
        case '!':
            if (src[pos] == '=') {
                pos++;
                t.type = TOK_RELOP;
                strcpy(t.value, "!=");
            } else {
                // 如果只是 '!'，在当前C子集中可能是非法语义，或者逻辑非
                // 这里暂且报错，除非文法支持逻辑非
                printf("Lexical Error: Unexpected character '!'\n");
                exit(1);
            }
            break;

            // --- 单字符符号 ---
        case '+': t.type = TOK_PLUS;  strcpy(t.value, "+"); break;
        case '-': t.type = TOK_MINUS; strcpy(t.value, "-"); break;
        case '(': t.type = TOK_LPAREN; strcpy(t.value, "("); break;
        case ')': t.type = TOK_RPAREN; strcpy(t.value, ")"); break;
        case '{': t.type = TOK_LBRACE; strcpy(t.value, "{"); break;
        case '}': t.type = TOK_RBRACE; strcpy(t.value, "}"); break;
        case ';': t.type = TOK_SEMI;   strcpy(t.value, ";"); break;

        default:
            t.type = TOK_END;
            printf("Lexical Error: Unknown character '%c'\n", ch);
            exit(1);
    }
    return t;
}