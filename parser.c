/* parser.c */
#include "parser.h"
#include "lexer.h"
#include "semantic.h"

// 动作类型
#define SHIFT 1
#define REDUCE 2
#define ACC   3
#define ERR   0

// --- 产生式编号 ---
// 1: S -> id = E ;
// 2: S -> if ( C ) M S
// 3: S -> if ( C ) M S N else M S
// 4: S -> { L }
// 5: L -> L M S
// 6: L -> S
// 7: E -> E + T
// 8: E -> T
// 9: T -> id
// 10: T -> num
// 11: C -> T relop T
// 12: M -> eps
// 13: N -> eps
// 14: E -> E - T
#define PROD_COUNT 15

typedef struct {
    int lhs; // 左部非终结符ID
    int len; // 右部长度
} Production;

Production prods[PROD_COUNT];

// --- 分析表 ---
typedef struct {
    int type; // SHIFT/REDUCE/ACC/ERR
    int val;  // State ID 或 Production ID
} ActionEntry;

ActionEntry actionTable[MAX_STATES][MAX_CODE_LEN];
int gotoTable[MAX_STATES][8]; // 8个非终结符

// --- 栈 ---
int stateStack[MAX_STACK];
SemNode symStack[MAX_STACK];
int top = 0;

// --- 初始化函数 ---
void setProd(int id, int lhs, int len) { prods[id].lhs = lhs; prods[id].len = len; }
void setAction(int s, TokenType t, int type, int val) { actionTable[s][t].type = type; actionTable[s][t].val = val; }
void setGoto(int s, int nt, int next) { gotoTable[s][nt] = next; }

// --- 填充分析表 ---
void initParser() {
    // 1. 初始化产生式信息
    setProd(1, 1, 4); // S -> id = E ;
    setProd(2, 1, 6); // S -> if ( C ) M S
    setProd(3, 1, 10); // S -> if ( C ) M S N else M S
    setProd(4, 1, 3); // S -> { L }
    setProd(5, 2, 3); // L -> L M S
    setProd(6, 2, 1); // L -> S
    setProd(7, 3, 3); // E -> E + T
    setProd(8, 3, 1); // E -> T
    setProd(9, 4, 1); // T -> id
    setProd(10, 4, 1); // T -> num
    setProd(11, 5, 3); // C -> T relop T
    setProd(12, 6, 0); // M -> eps
    setProd(13, 7, 0); // N -> eps
    setProd(14, 3, 3); // E -> E - T

    // 2. 初始化表格 (清零)
    memset(actionTable, 0, sizeof(actionTable));
    memset(gotoTable, 0, sizeof(gotoTable));

    // --- State 0: Start ---
    setAction(0, TOK_IF, SHIFT, 2);
    setAction(0, TOK_LBRACE, SHIFT, 3);
    setAction(0, TOK_ID, SHIFT, 4);
    setGoto(0, 1, 1); // S -> 1

    // State 1: ACC
    setAction(1, TOK_END, ACC, 0);

    // --- S -> if ... ---
    // State 2: if (
    setAction(2, TOK_LPAREN, SHIFT, 5);

    // State 5: if ( C
    setAction(5, TOK_ID, SHIFT, 6);
    setAction(5, TOK_NUM, SHIFT, 7);
    setGoto(5, 5, 8); // C -> 8
    setGoto(5, 4, 9); // T -> 9

    // State 6: id (as T)
    setAction(6, TOK_RELOP, REDUCE, 9);
    setAction(6, TOK_RPAREN, REDUCE, 9);
    setAction(6, TOK_PLUS, REDUCE, 9);
    setAction(6, TOK_MINUS, REDUCE, 9);
    setAction(6, TOK_SEMI, REDUCE, 9);

    // State 7: num (as T)
    setAction(7, TOK_RELOP, REDUCE, 10);
    setAction(7, TOK_RPAREN, REDUCE, 10);
    setAction(7, TOK_PLUS, REDUCE, 10);
    setAction(7, TOK_MINUS, REDUCE, 10);
    setAction(7, TOK_SEMI, REDUCE, 10);

    // State 9: T relop ...
    setAction(9, TOK_RELOP, SHIFT, 10);

    // State 10: T relop T
    setAction(10, TOK_ID, SHIFT, 6);
    setAction(10, TOK_NUM, SHIFT, 7);
    setGoto(10, 4, 11); // T -> 11

    // State 11: T relop T . -> Reduce C
    setAction(11, TOK_RPAREN, REDUCE, 11);

    // State 8: if ( C )
    setAction(8, TOK_RPAREN, SHIFT, 12);

    // State 12: if ( C ) . M
    setAction(12, TOK_LBRACE, REDUCE, 12);
    setAction(12, TOK_ID, REDUCE, 12);
    setAction(12, TOK_IF, REDUCE, 12);
    setGoto(12, 6, 13); // M -> 13

    // State 13: if ( C ) M . S
    setAction(13, TOK_IF, SHIFT, 2);
    setAction(13, TOK_LBRACE, SHIFT, 3);
    setAction(13, TOK_ID, SHIFT, 4);
    setGoto(13, 1, 14); // S -> 14

    // State 14: if ( C ) M S .
    setAction(14, TOK_ELSE, REDUCE, 13); // Reduce N->eps
    setAction(14, TOK_IF, REDUCE, 2);     // 语句衔接
    setAction(14, TOK_ID, REDUCE, 2);
    setAction(14, TOK_LBRACE, REDUCE, 2);
    setAction(14, TOK_SEMI, REDUCE, 2);
    setAction(14, TOK_RBRACE, REDUCE, 2);
    setAction(14, TOK_END, REDUCE, 2);
    setGoto(14, 7, 15); // N -> 15

    // State 15: ... N . else
    setAction(15, TOK_ELSE, SHIFT, 16);

    // State 16: else M
    setAction(16, TOK_LBRACE, REDUCE, 12);
    setAction(16, TOK_ID, REDUCE, 12);
    setAction(16, TOK_IF, REDUCE, 12);
    setGoto(16, 6, 17); // M -> 17

    // State 17: else M . S
    setAction(17, TOK_IF, SHIFT, 2);
    setAction(17, TOK_LBRACE, SHIFT, 3);
    setAction(17, TOK_ID, SHIFT, 4);
    setGoto(17, 1, 18); // S -> 18

    // State 18: ... else M S .
    setAction(18, TOK_IF, REDUCE, 3); // 语句衔接
    setAction(18, TOK_ID, REDUCE, 3);
    setAction(18, TOK_LBRACE, REDUCE, 3);
    setAction(18, TOK_SEMI, REDUCE, 3);
    setAction(18, TOK_RBRACE, REDUCE, 3);
    setAction(18, TOK_END, REDUCE, 3);

    // --- S -> { L } ---
    // State 3: {
    setAction(3, TOK_IF, SHIFT, 2);
    setAction(3, TOK_ID, SHIFT, 4);
    setAction(3, TOK_LBRACE, SHIFT, 3);
    setGoto(3, 2, 19); // L -> 19
    setGoto(3, 1, 20); // S -> 20

    // State 20: L -> S .
    setAction(20, TOK_RBRACE, REDUCE, 6);
    setAction(20, TOK_ID, REDUCE, 6);
    setAction(20, TOK_IF, REDUCE, 6);

    // State 19: { L . } or { L . M S }
    setAction(19, TOK_RBRACE, SHIFT, 21);
    setAction(19, TOK_ID, REDUCE, 12);
    setAction(19, TOK_IF, REDUCE, 12);
    setGoto(19, 6, 22); // M -> 22

    // State 21: { L } .
    setAction(21, TOK_IF, REDUCE, 4); // 语句衔接
    setAction(21, TOK_ID, REDUCE, 4);
    setAction(21, TOK_LBRACE, REDUCE, 4);
    setAction(21, TOK_SEMI, REDUCE, 4);
    setAction(21, TOK_ELSE, REDUCE, 4);
    setAction(21, TOK_RBRACE, REDUCE, 4);
    setAction(21, TOK_END, REDUCE, 4);

    // State 22: L M . S
    setAction(22, TOK_IF, SHIFT, 2);
    setAction(22, TOK_ID, SHIFT, 4);
    setGoto(22, 1, 23); // S -> 23

    // State 23: L M S .
    setAction(23, TOK_RBRACE, REDUCE, 5);
    setAction(23, TOK_ID, REDUCE, 5);
    setAction(23, TOK_IF, REDUCE, 5);

    // --- S -> id = E ; ---
    // State 4: id =
    setAction(4, TOK_ASSIGN, SHIFT, 24);

    // State 24: id = E
    setAction(24, TOK_ID, SHIFT, 26);
    setAction(24, TOK_NUM, SHIFT, 27);
    setGoto(24, 3, 25); // E -> 25
    setGoto(24, 4, 28); // T -> 28

    // State 26: id (as T)
    setAction(26, TOK_PLUS, REDUCE, 9);
    setAction(26, TOK_MINUS, REDUCE, 9);
    setAction(26, TOK_SEMI, REDUCE, 9);
    setAction(26, TOK_RPAREN, REDUCE, 9);

    // State 27: num (as T)
    setAction(27, TOK_PLUS, REDUCE, 10);
    setAction(27, TOK_MINUS, REDUCE, 10);
    setAction(27, TOK_SEMI, REDUCE, 10);
    setAction(27, TOK_RPAREN, REDUCE, 10);

    // State 28: E -> T .
    setAction(28, TOK_PLUS, REDUCE, 8);
    setAction(28, TOK_MINUS, REDUCE, 8);
    setAction(28, TOK_SEMI, REDUCE, 8);

    // State 25: id = E . ; or + or -
    setAction(25, TOK_SEMI, SHIFT, 29);
    setAction(25, TOK_PLUS, SHIFT, 30);
    setAction(25, TOK_MINUS, SHIFT, 33);

    // --- 加法 ---
    // State 30: E + T
    setAction(30, TOK_ID, SHIFT, 26);
    setAction(30, TOK_NUM, SHIFT, 27);
    setGoto(30, 4, 31); // T -> 31

    // State 31: E + T .
    setAction(31, TOK_SEMI, REDUCE, 7);
    setAction(31, TOK_PLUS, REDUCE, 7);
    setAction(31, TOK_MINUS, REDUCE, 7);

    // --- 减法 ---
    // State 33: E -
    setAction(33, TOK_ID, SHIFT, 26);
    setAction(33, TOK_NUM, SHIFT, 27);
    setGoto(33, 4, 34); // T -> 34

    // State 34: E - T .
    setAction(34, TOK_SEMI, REDUCE, 14);
    setAction(34, TOK_PLUS, REDUCE, 14);
    setAction(34, TOK_MINUS, REDUCE, 14);

    // State 29: id = E ; .
    setAction(29, TOK_ELSE, REDUCE, 1);
    setAction(29, TOK_RBRACE, REDUCE, 1);
    setAction(29, TOK_SEMI, REDUCE, 1);
    setAction(29, TOK_END, REDUCE, 1);
    setAction(29, TOK_IF, REDUCE, 1);
    setAction(29, TOK_ID, REDUCE, 1);
}

void SLR1_Parser() {
    initParser();
    stateStack[top] = 0;
    Token token = getToken();

    printf("Starting Table-Driven Parser...\n");

    while(1) {
        int s = stateStack[top];
        ActionEntry act = actionTable[s][token.type];

        if (act.type == SHIFT) {
            printf("SHIFT  %-5s -> State %d\n", token.value, act.val);
            top++;
            stateStack[top] = act.val;
            strcpy(symStack[top].name, token.value);
            // 清空属性列表
            symStack[top].tl_count = symStack[top].fl_count = symStack[top].nl_count = 0;
            token = getToken();
        }
        else if (act.type == REDUCE) {
            int prodID = act.val;
            int lhs = prods[prodID].lhs;
            int len = prods[prodID].len;
            printf("REDUCE Rule %d\n", prodID);

            SemNode newVal;
            newVal.tl_count = newVal.fl_count = newVal.nl_count = 0;

            switch(prodID) {
                case 1: // S -> id = E ;
                    emit("=", symStack[top-1].name, "-", 0, 0);
                    sprintf(quadArray[NXQ-1].arg2, "%s", symStack[top-3].name);
                    break;
                case 2: // S -> if ( C ) M S
                    backpatch(symStack[top-3].trueList, symStack[top-3].tl_count, symStack[top-1].quad);
                    backpatch(symStack[top-3].falseList, symStack[top-3].fl_count, NXQ);
                    mergeList(newVal.nextList, &newVal.nl_count, symStack[top].nextList, symStack[top].nl_count);
                    break;
                case 3: // S -> if ( C ) M S N else M S
                {
                    int iC = top-7; int iM1 = top-5; int iS1 = top-4;
                    int iN = top-3; int iM2 = top-1; int iS2 = top;
                    backpatch(symStack[iC].trueList, symStack[iC].tl_count, symStack[iM1].quad);
                    backpatch(symStack[iC].falseList, symStack[iC].fl_count, symStack[iM2].quad);
                    mergeList(newVal.nextList, &newVal.nl_count, symStack[iS1].nextList, symStack[iS1].nl_count);
                    mergeList(newVal.nextList, &newVal.nl_count, symStack[iN].nextList, symStack[iN].nl_count);
                    mergeList(newVal.nextList, &newVal.nl_count, symStack[iS2].nextList, symStack[iS2].nl_count);
                    backpatch(symStack[iN].nextList, symStack[iN].nl_count, NXQ);
                }
                    break;
                case 4: // S -> { L }
                    mergeList(newVal.nextList, &newVal.nl_count, symStack[top-1].nextList, symStack[top-1].nl_count);
                    break;
                case 5: // L -> L M S
                    backpatch(symStack[top-2].nextList, symStack[top-2].nl_count, symStack[top-1].quad);
                    mergeList(newVal.nextList, &newVal.nl_count, symStack[top].nextList, symStack[top].nl_count);
                    break;
                case 6: // L -> S
                    mergeList(newVal.nextList, &newVal.nl_count, symStack[top].nextList, symStack[top].nl_count);
                    break;
                case 7: // E -> E + T
                    strcpy(newVal.name, newTemp());
                    emit("+", symStack[top-2].name, symStack[top].name, 0, 0);
                    sprintf(quadArray[NXQ-1].arg2, "%s", newVal.name);
                    break;
                case 8: // E -> T
                    strcpy(newVal.name, symStack[top].name);
                    break;
                case 9: // T -> id
                case 10: // T -> num
                    strcpy(newVal.name, symStack[top].name);
                    break;
                case 11: // C -> T relop T
                    makeList(&newVal, NXQ, 0);
                    makeList(&newVal, NXQ+1, 1);
                    emit("j", symStack[top-2].name, symStack[top].name, 0, 1);
                    char op[5]; sprintf(op, "j%s", symStack[top-1].name);
                    strcpy(quadArray[NXQ-1].op, op);
                    emit("j", "-", "-", 0, 1);
                    break;
                case 12: // M -> eps
                    newVal.quad = NXQ;
                    break;
                case 13: // N -> eps
                    makeList(&newVal, NXQ, 2);
                    emit("j", "-", "-", 0, 1);
                    break;
                case 14: // E -> E - T
                    strcpy(newVal.name, newTemp());
                    emit("-", symStack[top-2].name, symStack[top].name, 0, 0);
                    sprintf(quadArray[NXQ-1].arg2, "%s", newVal.name);
                    break;
            }

            top -= len;

            int nextState = gotoTable[stateStack[top]][lhs];
            if (nextState == 0 && lhs != 1) {
                printf("GOTO Error: State %d, LHS %d\n", stateStack[top], lhs);
                exit(1);
            }
            top++;
            stateStack[top] = nextState;
            symStack[top] = newVal;
        }
        else if (act.type == ACC) {
            printf("Analysis Success!\n");
            return;
        }
        else {
            printf("Syntax Error at State %d, Token %s (Type %d)\n", s, token.value, token.type);
            exit(1);
        }
    }
}