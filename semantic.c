/* semantic.c */
#include "semantic.h"

Quad quadArray[MAX_QUADS];
int NXQ = 0;
int tempCount = 0;

//生成四元式
void emit(char *op, char *arg1, char *arg2, int result, int isJump) {
    strcpy(quadArray[NXQ].op, op);
    strcpy(quadArray[NXQ].arg1, arg1);
    strcpy(quadArray[NXQ].arg2, arg2);
    quadArray[NXQ].result = result;
    strcpy(quadArray[NXQ].res, "_"); // 初始化 res 为占位符
    quadArray[NXQ].isJump = isJump;

    // [过程展示]
    printf("  [Semantic] Emit: (%s, %s, %s, ", op, arg1, arg2);
    if(isJump) printf("%d)\n", result); else printf("_)\n");

    NXQ++;
}

void makeList(SemNode *node, int index, int type) {
    if (type == 0) { node->trueList[0] = index; node->tl_count = 1; }
    if (type == 1) { node->falseList[0] = index; node->fl_count = 1; }
    if (type == 2) { node->nextList[0] = index; node->nl_count = 1; }
    printf("  [Semantic] MakeList: index %d\n", index);
}

void mergeList(int *dest, int *count, int *src, int src_cnt) {
    for(int i=0; i<src_cnt; i++) {
        dest[*count + i] = src[i];
    }
    *count += src_cnt;
}

void backpatch(int *list, int count, int target) {
    printf("  [Semantic] Backpatch: target %d applied to list [", target);
    for(int i=0; i<count; i++) {
        int qIdx = list[i];
        quadArray[qIdx].result = target;
        printf("%d ", qIdx);
    }
    printf("]\n");
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

        //如果是跳转指令，打印数字；如果是运算指令，打印字符串变量名
        if (quadArray[i].isJump) {
            printf("%d)\n", quadArray[i].result);
        } else {
            printf("%s)\n", quadArray[i].res);
        }
    }
}