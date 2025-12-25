/* common.h */
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// --- 常量定义 ---
#define MAX_CODE_LEN 2000
#define MAX_STACK 500
#define MAX_QUADS 500
#define MAX_STATES 100

// --- Token 定义 ---
typedef enum {
    TOK_END = 0,    // $
    TOK_IF,         // if
    TOK_ELSE,       // else
    TOK_ID,         // identifier
    TOK_NUM,        // number
    TOK_RELOP,      // >, <, ==, !=, >=, <=
    TOK_ASSIGN,     // =
    TOK_PLUS,       // +
    TOK_MINUS,      // -
    TOK_LPAREN,     // (
    TOK_RPAREN,     // )
    TOK_LBRACE,     // {
    TOK_RBRACE,     // }
    TOK_SEMI        // ;
} TokenType;

typedef struct {
    TokenType type;
    char value[32];
} Token;

// --- 四元式 (修改版) ---
typedef struct {
    char op[10];      // 操作符
    char arg1[10];    // 操作数1
    char arg2[10];    // 操作数2
    char res[10];     // [新增] 字符串结果 (用于存储变量名 T1, a 等)
    int result;       // [原有] 整数结果 (用于存储跳转行号 10, 2 等)
    int isJump;       // 1: 跳转指令, 0: 运算/赋值指令
} Quad;

// --- 语义属性节点 ---
typedef struct {
    char name[32];
    int quad;
    int trueList[50];  int tl_count;
    int falseList[50]; int fl_count;
    int nextList[50];  int nl_count;
} SemNode;

// --- 全局变量 ---
extern Quad quadArray[MAX_QUADS];
extern int NXQ;

#endif