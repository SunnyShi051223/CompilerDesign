//
// Created by 32874 on 2025/12/20.
//

#ifndef COMPILERDESIGN_PARSER_H
#define COMPILERDESIGN_PARSER_H

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

//产生式：
#define PROD_COUNT 15

//动作：
#define ACT_ERR    0
#define ACT_SHIFT  1
#define ACT_REDUCE 2
#define ACT_ACC    3

#define MAX_ITEMS 200
#define MAX_RULES 200

//Action表
typedef struct {
    int type;
    int val;
} ActionEntry;

//产生式左右位
typedef struct {
    int lhs;
    int len;
} Production;

#define IS_TERMINAL(s) ((s) > 0)
#define IS_NONTERMINAL(s) ((s) < 0)
#define GET_NT(s) (-(s))

//非终
typedef enum {
    NT_S = 1, NT_L = 2, NT_E = 3, NT_T = 4, NT_C = 5, NT_M = 6, NT_N = 7
} NonTerminal;

//完整产生式
typedef struct {
    int id;
    int lhs;
    int rhs[10];
    int len;
} GenRule;

//LR(0)
typedef struct {
    int ruleIndex;
    int dotPos;
} Item;

//DFA
typedef struct {
    int id;
    Item items[MAX_ITEMS];
    int itemCount;
} State;

static ActionEntry actionTable[MAX_STATES][MAX_CODE_LEN];
static int gotoTable[MAX_STATES][8];

//产生式查询，用于REUDCE归约
static Production prods[PROD_COUNT];

static int stateStack[MAX_STACK];
static SemNode symStack[MAX_STACK];
static int top = 0;


static GenRule rules[MAX_RULES];
static int ruleCount = 0;
static State states[MAX_STATES];
static int stateCount = 0;

static bool followSet[10][20];

// --- 辅助函数 ---
static void setProd(int id, int lhs, int len) { prods[id].lhs = lhs; prods[id].len = len; }
static void setAction(int s, int t, int type, int val) { actionTable[s][t].type = type; actionTable[s][t].val = val; }
static void setGoto(int s, int nt, int next) { gotoTable[s][nt] = next; }


void SLR1_Parser();

#endif //COMPILERDESIGN_PARSER_H
