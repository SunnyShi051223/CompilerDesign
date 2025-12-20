//
// Created by 32874 on 2025/12/20.
//

/* semantic.c */
#include "semantic.h"

Quad quadArray[MAX_QUADS];
int NXQ = 0;

void emit(char *op, char *arg1, char *arg2, int result, int isJump) {
    strcpy(quadArray[NXQ].op, op);
    strcpy(quadArray[NXQ].arg1, arg1);
    strcpy(quadArray[NXQ].arg2, arg2);
    quadArray[NXQ].result = result;
    quadArray[NXQ].isJump = isJump;
    NXQ++;
}

// type: 0=TrueList, 1=FalseList, 2=NextList
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
        int qIdx = list[i];
        quadArray[qIdx].result = target;
    }
}

void printQuads() {
    printf("\n--- Quadruples ---\n");
    for(int i=0; i<NXQ; i++) {
        printf("%d:\t(%s, %s, %s, ", i, quadArray[i].op, quadArray[i].arg1, quadArray[i].arg2);
        if (quadArray[i].isJump) printf("%d)\n", quadArray[i].result);
        else printf("%s)\n", "_"); // 赋值语句最后一位通常为空或临时变量
    }
}