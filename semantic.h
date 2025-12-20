//
// Created by 32874 on 2025/12/20.
//

#ifndef COMPILERDESIGN_SEMANTIC_H
#define COMPILERDESIGN_SEMANTIC_H

#include "common.h"

void emit(char *op, char *arg1, char *arg2, int result, int isJump);
void makeList(SemNode *node, int index, int type); // 0:true, 1:false, 2:next
void mergeList(int *dest, int *count, int *src, int src_cnt);
void backpatch(int *list, int count, int target);
char* newTemp();
void printQuads();

#endif //COMPILERDESIGN_SEMANTIC_H
