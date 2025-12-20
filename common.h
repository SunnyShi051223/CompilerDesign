//
// Created by 32874 on 2025/12/20.
//

#ifndef COMPILERDESIGN_COMMON_H
#define COMPILERDESIGN_COMMON_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// --- 常量定义 ---
#define MAX_CODE_LEN 1000
#define MAX_STACK 100
#define MAX_QUADS 100

// --- Token 定义 ---
typedef enum {
    TOK_END = 0,    // $
    TOK_IF,         // if
    TOK_ELSE,       // else
    TOK_ID,         // 标识符
    TOK_NUM,        // 数字
    TOK_RELOP,      // >, <, ==
    TOK_ASSIGN,     // =
    TOK_LPAREN,     // (
    TOK_RPAREN,     // )
    TOK_LBRACE,     // {
    TOK_RBRACE,     // }
    TOK_SEMI        // ;
} TokenType;

typedef struct {
    TokenType type;
    char value[32]; // 存储变量名或数值
} Token;

// --- 四元式定义 ---
typedef struct {
    char op[10];
    char arg1[10];
    char arg2[10];
    int result;     // 跳转目标 或 临时变量
    int isJump;     // 标记 result 是否为跳转地址（用于打印区分）
} Quad;

// --- 语义属性节点 (用于符号栈) ---
// 每一个文法符号(S, E, C...)都关联一个这样的结构
typedef struct {
    char name[32];  // 变量名或临时变量名
    int quad;       // M 标记用的指令地址

    // 回填链表 (简单的静态数组实现)
    int trueList[20];  int tl_count;
    int falseList[20]; int fl_count;
    int nextList[20];  int nl_count;
} SemNode;

// --- 全局变量声明 ---
extern Quad quadArray[MAX_QUADS];
extern int NXQ; // Next Quad Pointer

#endif //COMPILERDESIGN_COMMON_H
