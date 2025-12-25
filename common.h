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

// --- 四元式 ---
typedef struct {
    char op[10];
    char arg1[10];
    char arg2[10];
    int result;
    int isJump;
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