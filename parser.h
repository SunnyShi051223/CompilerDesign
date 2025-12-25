//
// Created by 32874 on 2025/12/20.
//

#ifndef COMPILERDESIGN_PARSER_H
#define COMPILERDESIGN_PARSER_H

#include "common.h"

#define PROD_COUNT 15
#define ACT_ERR    0
#define ACT_SHIFT  1
#define ACT_REDUCE 2
#define ACT_ACC    3
#define MAX_ITEMS 200
#define MAX_RULES 20

typedef struct {
    int type;
    int val;
} ActionEntry;

typedef struct {
    int lhs;
    int len;
} Production;

void SLR1_Parser();

#endif //COMPILERDESIGN_PARSER_H
