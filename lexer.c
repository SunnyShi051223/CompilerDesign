/* lexer.c */
#include "lexer.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// DFA 状态定义
typedef enum {
    S0,         // 初态
    Sid,        // 标识符状态
    Snum,       // 数字状态
    S_less,     // < 状态
    S_greater,  // > 状态
    S_eq,       // = 状态
    S_not,      // ! 状态
    DONE        // 终态 (辅助状态，用于跳出循环)
} State;

static const char *src;
static int pos = 0;

void initLexer(const char *source) {
    src = source;
    pos = 0;
}

// 辅助函数：判断是否为界符
static int isBoundary(char ch) {
    return strchr("(){};", ch) != NULL;
}

const char* getTokenName(int type) {
    switch(type) {
        case TOK_END:    return "TOK_END";
        case TOK_IF:     return "TOK_IF";
        case TOK_ELSE:   return "TOK_ELSE";
        case TOK_ID:     return "TOK_ID";
        case TOK_NUM:    return "TOK_NUM";
        case TOK_RELOP:  return "TOK_RELOP";
        case TOK_ASSIGN: return "TOK_ASSIGN";
        case TOK_PLUS:   return "TOK_PLUS";
        case TOK_MINUS:  return "TOK_MINUS";
        case TOK_LPAREN: return "TOK_LPAREN";
        case TOK_RPAREN: return "TOK_RPAREN";
        case TOK_LBRACE: return "TOK_LBRACE";
        case TOK_RBRACE: return "TOK_RBRACE";
        case TOK_SEMI:   return "TOK_SEMI";
        default:         return "UNKNOWN";
    }
}

// 基于 DFA 状态机实现的 getToken
Token getToken() {
    Token t;
    // 初始化 Token
    t.type = TOK_END;
    memset(t.value, 0, sizeof(t.value));

    State state = S0;
    int i = 0; // buffer index
    char ch;

    while (state != DONE) {
        ch = src[pos++]; // 读取下一个字符

        switch (state) {
            case S0: // --- 初态 ---
                if (ch == '\0') {
                    t.type = TOK_END;
                    strcpy(t.value, "$");
                    state = DONE;
                }
                else if (isspace(ch)) {
                    // WS -> S0 (忽略空白，保持初态)
                    // 实际上是重新开始循环，不做任何操作
                }
                else if (isalpha(ch)) {
                    // 报告: Letter -> Sid
                    state = Sid;
                    t.value[i++] = ch;
                }
                else if (isdigit(ch)) {
                    // 报告: Digit -> Snum
                    state = Snum;
                    t.value[i++] = ch;
                }
                else if (ch == '<') { state = S_less; t.value[i++] = ch; }
                else if (ch == '>') { state = S_greater; t.value[i++] = ch; }
                else if (ch == '=') { state = S_eq; t.value[i++] = ch; }
                else if (ch == '!') { state = S_not; t.value[i++] = ch; }
                else if (ch == '+') { t.type = TOK_PLUS; strcpy(t.value, "+"); state = DONE; }
                else if (ch == '-') { t.type = TOK_MINUS; strcpy(t.value, "-"); state = DONE; }
                else if (isBoundary(ch)) {
                    // 报告: BOUNDARY -> Zbound (直接识别)
                    t.value[0] = ch; t.value[1] = '\0';
                    switch(ch) {
                        case '(': t.type = TOK_LPAREN; break;
                        case ')': t.type = TOK_RPAREN; break;
                        case '{': t.type = TOK_LBRACE; break;
                        case '}': t.type = TOK_RBRACE; break;
                        case ';': t.type = TOK_SEMI; break;
                    }
                    state = DONE;
                }
                else {
                    // 非法字符
                    printf("[Lexical Error] Illegal Character '%c' at position %d\n", ch, pos);
                    exit(1);
                }
                break;

            case Sid: // --- 标识符状态 ---
                if (isalnum(ch)) {
                    // 报告: Letter/Digit -> Sid
                    if (i < 31) t.value[i++] = ch;
                } else {
                    // 报告: 其他字符 -> Zdone (标识符结束，需回退)
                    pos--; // 回退字符 (Retract)
                    t.value[i] = '\0';
                    // 查表判断是 Keyword 还是 ID
                    if (strcmp(t.value, "if") == 0) t.type = TOK_IF;
                    else if (strcmp(t.value, "else") == 0) t.type = TOK_ELSE;
                    else t.type = TOK_ID;
                    state = DONE;
                }
                break;

            case Snum: // --- 常数状态 ---
                if (isdigit(ch)) {
                    // 报告: Digit -> Snum
                    if (i < 31) t.value[i++] = ch;
                } else {
                    // 报告: 其他字符 -> Znum (需回退)
                    pos--;
                    t.value[i] = '\0';
                    t.type = TOK_NUM;
                    state = DONE;
                }
                break;

            case S_less: // --- < 状态 ---
                if (ch == '=') {
                    // 报告: = -> Zrelop (<=)
                    t.value[i++] = ch; t.value[i] = '\0';
                    t.type = TOK_RELOP;
                    state = DONE;
                } else {
                    // 报告: 其他 -> Zrelop (<)，需回退
                    pos--;
                    t.type = TOK_RELOP;
                    state = DONE;
                }
                break;

            case S_greater: // --- > 状态 ---
                if (ch == '=') {
                    t.value[i++] = ch; t.value[i] = '\0';
                    t.type = TOK_RELOP;
                    state = DONE;
                } else {
                    pos--;
                    t.type = TOK_RELOP;
                    state = DONE;
                }
                break;

            case S_eq: // --- = 状态 ---
                if (ch == '=') {
                    // 报告: = -> Zrelop (==)
                    t.value[i++] = ch; t.value[i] = '\0';
                    t.type = TOK_RELOP;
                    state = DONE;
                } else {
                    // 报告: 其他 -> Zassign (=)，需回退
                    pos--;
                    t.type = TOK_ASSIGN;
                    state = DONE;
                }
                break;

            case S_not: // --- ! 状态 ---
                if (ch == '=') {
                    // 报告: = -> Zrelop (!=)
                    t.value[i++] = ch; t.value[i] = '\0';
                    t.type = TOK_RELOP;
                    state = DONE;
                } else {
                    // 报告: 其他 -> Zerr (非法单独 !)
                    printf("[Lexical Error] Unexpected character '!' without '='.\n");
                    exit(1);
                }
                break;

            default:
                state = DONE;
                break;
        }
    }
    return t;
}