//
// Created by 32874 on 2025/12/20.
//

#include "semantic.h"

Quad quadArray[MAX_QUADS];
int NXQ = 0;
int tempCount = 0;

void emit(char *op, char *arg1, char *arg2, int result, int isJump) {
    strcpy(quadArray[NXQ].op, op);
    strcpy(quadArray[NXQ].arg1, arg1);
    strcpy(quadArray[NXQ].arg2, arg2);
    quadArray[NXQ].result = result;
    quadArray[NXQ].isJump = isJump;
    NXQ++;
}

void makeList(SemNode *node, int index, int type) {
    if (type == 0) { node->trueList[0] = index; node->tl_count = 1; }
    if (type == 1) { node->falseList[0] = index; node->fl_count = 1; }
    if (type == 2) { node->nextList[0] = index; node->nl_count = 1; }
}

void mergeList(int *dest, int *count, int *src, int src_cnt) {
    for(int i=0; i<src_cnt; i++) {
        dest[*count + i] = src[i];
    }
    *count += src_cnt;
}

void backpatch(int *list, int count, int target) {
    for(int i=0; i<count; i++) {
        quadArray[list[i]].result = target;
    }
}

char* newTemp() {
    static char buf[10];
    sprintf(buf, "T%d", ++tempCount);
    return buf;
}

void printQuads() {
    printf("\n%-5s %-5s %-5s %-5s %-5s\n", "Label", "Op", "Arg1", "Arg2", "Result");
    printf("----------------------------------------\n");
    for(int i=0; i<NXQ; i++) {
        printf("%-5d (%-5s, %-5s, %-5s, ", i, quadArray[i].op, quadArray[i].arg1, quadArray[i].arg2);
        if (quadArray[i].isJump) printf("%d)\n", quadArray[i].result);
        else printf("%s)\n", "_"); // 赋值/运算四元式的第四项通常在中间代码里不显式打印，或作为结果
    }
}