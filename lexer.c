//
// Created by 32874 on 2025/12/20.
//

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

    // 跳过空白
    while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
        pos++;
        ch = src[pos];
    }

    if (ch == '\0') {
        t.type = TOK_END;
        strcpy(t.value, "$");
        return t;
    }

    // 标识符 & 关键字
    if (isalpha(ch)) {
        int i = 0;
        while (isalnum(ch)) {
            t.value[i++] = ch;
            pos++;
            ch = src[pos];
        }
        t.value[i] = '\0';

        if (strcmp(t.value, "if") == 0) t.type = TOK_IF;
        else if (strcmp(t.value, "else") == 0) t.type = TOK_ELSE;
        else t.type = TOK_ID;
        return t;
    }

    // 数字
    if (isdigit(ch)) {
        int i = 0;
        while (isdigit(ch)) {
            t.value[i++] = ch;
            pos++;
            ch = src[pos];
        }
        t.value[i] = '\0';
        t.type = TOK_NUM;
        return t;
    }

    // 符号
    pos++;
    switch (ch) {
        case '=':
            if (src[pos] == '=') { pos++; t.type = TOK_RELOP; strcpy(t.value, "=="); }
            else { t.type = TOK_ASSIGN; strcpy(t.value, "="); }
            break;
        case '>': t.type = TOK_RELOP; strcpy(t.value, ">"); break;
        case '<': t.type = TOK_RELOP; strcpy(t.value, "<"); break;
        case '+': t.type = TOK_PLUS;  strcpy(t.value, "+"); break;
        case '(': t.type = TOK_LPAREN; strcpy(t.value, "("); break;
        case ')': t.type = TOK_RPAREN; strcpy(t.value, ")"); break;
        case '{': t.type = TOK_LBRACE; strcpy(t.value, "{"); break;
        case '}': t.type = TOK_RBRACE; strcpy(t.value, "}"); break;
        case ';': t.type = TOK_SEMI;   strcpy(t.value, ";"); break;
        default:  t.type = TOK_END; printf("Lexical Error: %c\n", ch); exit(1);
    }
    return t;
}