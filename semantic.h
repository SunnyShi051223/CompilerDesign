//
// Created by 32874 on 2025/12/20.
//

#ifndef COMPILERDESIGN_SEMANTIC_H
#define COMPILERDESIGN_SEMANTIC_H

#include "common.h"

//生成四元式
void emit(char *op, char *arg1, char *arg2, int result, int isJump);
//创建链表
void makeList(SemNode *node, int index, int type);
//合并链表
void mergeList(int *dest, int *count, int *src, int src_cnt);
//回填
void backpatch(int *list, int count, int target);
//创建临时变量，用于存储中间结果
char* newTemp();
void printQuads();

#endif //COMPILERDESIGN_SEMANTIC_H
