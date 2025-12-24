//
// Created by 32874 on 2025/12/20.
//

#ifndef COMPILERDESIGN_COMMON_H
#define COMPILERDESIGN_COMMON_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// --- 常量定义 ---
#define MAX_CODE_LEN 2000
#define MAX_STACK 500
#define MAX_QUADS 500
#define MAX_STATES 100
#define MAX_TOKENS 20

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

// --- 四元式 ---
typedef struct {
    char op[10];
    char arg1[10];
    char arg2[10];
    int result;     // 跳转地址 或 临时变量编号
    int isJump;     // 1=跳转指令(打印时打印result), 0=运算/赋值
} Quad;

// --- 语义属性节点 ---
typedef struct {
    char name[32];  // 变量名 (如 "a", "T1")
    int quad;       // 指令地址 (用于 M)

    // 回填链表
    int trueList[50];  int tl_count;
    int falseList[50]; int fl_count;
    int nextList[50];  int nl_count;
} SemNode;

// --- 全局变量 ---
extern Quad quadArray[MAX_QUADS];
extern int NXQ;

#endif //COMPILERDESIGN_COMMON_H
