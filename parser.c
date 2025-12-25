/* parser.c */
#include "parser.h"
#include "lexer.h"
#include "semantic.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// ==========================================
// 内部私有结构体 (仅生成器算法使用)
// ==========================================

#define IS_TERMINAL(s) ((s) > 0)
#define IS_NONTERMINAL(s) ((s) < 0)
#define GET_NT(s) (-(s))

// 非终结符枚举
typedef enum {
    NT_S = 1, NT_L = 2, NT_E = 3, NT_T = 4, NT_C = 5, NT_M = 6, NT_N = 7
} NonTerminal;

// 文法规则结构 (生成器用)
typedef struct {
    int id;
    int lhs;
    int rhs[10];
    int len;
} GenRule;

// 项目 (Item)
typedef struct {
    int ruleIndex;
    int dotPos;
} Item;

// 状态 (State)
typedef struct {
    int id;
    Item items[MAX_ITEMS];
    int itemCount;
} State;

// ==========================================
// 静态全局变量
// ==========================================

// 分析表与产生式数组
static ActionEntry actionTable[MAX_STATES][MAX_CODE_LEN];
static int gotoTable[MAX_STATES][8];
static Production prods[PROD_COUNT];

// 分析栈
static int stateStack[MAX_STACK];
static SemNode symStack[MAX_STACK];
static int top = 0;

// 生成器专用变量
static GenRule rules[MAX_RULES];
static int ruleCount = 0;
static State states[MAX_STATES];
static int stateCount = 0;
static bool followSet[10][20];

// ==========================================
// 辅助函数实现
// ==========================================

// 设置运行时产生式信息
static void setProd(int id, int lhs, int len) {
    prods[id].lhs = lhs;
    prods[id].len = len;
}

// 设置 Action 表项
static void setAction(int s, int t, int type, int val) {
    actionTable[s][t].type = type;
    actionTable[s][t].val = val;
}

// 设置 Goto 表项
static void setGoto(int s, int nt, int next) {
    gotoTable[s][nt] = next;
}

// ==========================================
// PART 1: 自动生成算法实现 (Auto Generator)
// ==========================================

// 1.1 添加文法规则
static void addRule(int id, int lhs, int r1, int r2, int r3, int r4, int r5, int r6, int r7) {
    int idx = ruleCount++;
    rules[idx].id = id; rules[idx].lhs = lhs;
    int rhs[] = {r1, r2, r3, r4, r5, r6, r7};
    int len = 0;
    for(int i=0; i<7; i++) {
        if(rhs[i] == 0) break;
        rules[idx].rhs[i] = rhs[i];
        len++;
    }
    rules[idx].len = len;
}

// 1.2 初始化文法
static void initGrammarGenerator() {
    ruleCount = 0;
    addRule(0, 0, -NT_S, 0, 0, 0, 0, 0, 0); // S' -> S
    addRule(1, NT_S, TOK_ID, TOK_ASSIGN, -NT_E, TOK_SEMI, 0, 0, 0);
    addRule(2, NT_S, TOK_IF, TOK_LPAREN, -NT_C, TOK_RPAREN, -NT_M, -NT_S, 0);

    // Rule 3: S -> if ( C ) M S N else M S
    rules[ruleCount].id = 3; rules[ruleCount].lhs = NT_S;
    int r3[] = {TOK_IF, TOK_LPAREN, -NT_C, TOK_RPAREN, -NT_M, -NT_S, -NT_N, TOK_ELSE, -NT_M, -NT_S};
    for(int i=0; i<10; i++) rules[ruleCount].rhs[i] = r3[i];
    rules[ruleCount].len = 10;
    ruleCount++;

    addRule(4, NT_S, TOK_LBRACE, -NT_L, TOK_RBRACE, 0, 0, 0, 0);
    addRule(5, NT_L, -NT_L, -NT_M, -NT_S, 0, 0, 0, 0);
    addRule(6, NT_L, -NT_S, 0, 0, 0, 0, 0, 0);
    addRule(7, NT_E, -NT_E, TOK_PLUS, -NT_T, 0, 0, 0, 0);
    addRule(8, NT_E, -NT_T, 0, 0, 0, 0, 0, 0);
    addRule(9, NT_T, TOK_ID, 0, 0, 0, 0, 0, 0);
    addRule(10, NT_T, TOK_NUM, 0, 0, 0, 0, 0, 0);
    addRule(11, NT_C, -NT_T, TOK_RELOP, -NT_T, 0, 0, 0, 0);
    addRule(12, NT_M, 0, 0, 0, 0, 0, 0, 0);
    addRule(13, NT_N, 0, 0, 0, 0, 0, 0, 0);
    addRule(14, NT_E, -NT_E, TOK_MINUS, -NT_T, 0, 0, 0, 0);
}

// 1.3 初始化 Follow 集
static void initFollowSets() {
    memset(followSet, 0, sizeof(followSet));
#define F(nt, tok) followSet[nt][tok] = true

    F(NT_S, TOK_END); F(NT_S, TOK_ELSE); F(NT_S, TOK_RBRACE);
    F(NT_L, TOK_RBRACE); F(NT_L, TOK_IF); F(NT_L, TOK_ID); F(NT_L, TOK_LBRACE);
    F(NT_E, TOK_SEMI); F(NT_E, TOK_PLUS); F(NT_E, TOK_MINUS);
    F(NT_T, TOK_SEMI); F(NT_T, TOK_PLUS); F(NT_T, TOK_MINUS); F(NT_T, TOK_RELOP); F(NT_T, TOK_RPAREN);
    F(NT_C, TOK_RPAREN);
    F(NT_M, TOK_IF); F(NT_M, TOK_ID); F(NT_M, TOK_LBRACE);
    F(NT_N, TOK_ELSE);
}

// 1.4 DFA 构建辅助函数
static bool isSameItem(Item a, Item b) {
    return a.ruleIndex == b.ruleIndex && a.dotPos == b.dotPos;
}

static void addItemToState(State *s, Item item) {
    for(int i=0; i<s->itemCount; i++) if(isSameItem(s->items[i], item)) return;
    s->items[s->itemCount++] = item;
}

static void closure(State *s) {
    bool changed = true;
    while(changed) {
        changed = false;
        int currentCount = s->itemCount;
        for(int i=0; i<currentCount; i++) {
            Item it = s->items[i];
            GenRule rule = rules[it.ruleIndex];
            if(it.dotPos < rule.len && IS_NONTERMINAL(rule.rhs[it.dotPos])) {
                int B = GET_NT(rule.rhs[it.dotPos]);
                for(int r=0; r<ruleCount; r++) {
                    if(rules[r].lhs == B) {
                        Item newItem = {r, 0};
                        bool exists = false;
                        for(int k=0; k<s->itemCount; k++)
                            if(isSameItem(s->items[k], newItem)) { exists = true; break; }
                        if(!exists) { s->items[s->itemCount++] = newItem; changed = true; }
                    }
                }
            }
        }
    }
}

static int findState(State *target) {
    for(int i=0; i<stateCount; i++) {
        if(states[i].itemCount != target->itemCount) continue;
        bool match = true;
        for(int j=0; j<target->itemCount; j++) {
            bool found = false;
            for(int k=0; k<states[i].itemCount; k++)
                if(isSameItem(target->items[j], states[i].items[k])) { found = true; break; }
            if(!found) { match = false; break; }
        }
        if(match) return i;
    }
    return -1;
}

// 1.5 核心：自动计算 Action 和 Goto 表
static void generateSLRTable() {
    printf("[AutoGenerator] Initializing Grammar & Follow Sets...\n");
    initGrammarGenerator();
    initFollowSets();

    printf("[AutoGenerator] Building DFA & Parsing Table...\n");
    stateCount = 0;
    states[0].id = 0; states[0].itemCount = 0;
    Item startItem = {0, 0}; addItemToState(&states[0], startItem); closure(&states[0]);
    stateCount++;

    int processed = 0;
    while(processed < stateCount) {
        State *current = &states[processed];
        int symbols[50]; int symCount = 0;

        // 收集转移符号
        for(int i=0; i<current->itemCount; i++) {
            Item it = current->items[i];
            GenRule r = rules[it.ruleIndex];
            if(it.dotPos < r.len) {
                int sym = r.rhs[it.dotPos];
                bool has = false;
                for(int k=0; k<symCount; k++) if(symbols[k] == sym) has = true;
                if(!has) symbols[symCount++] = sym;
            }
        }

        // GOTO 运算
        for(int k=0; k<symCount; k++) {
            int X = symbols[k];
            State newState; newState.itemCount = 0;
            for(int i=0; i<current->itemCount; i++) {
                Item it = current->items[i];
                GenRule r = rules[it.ruleIndex];
                if(it.dotPos < r.len && r.rhs[it.dotPos] == X) {
                    Item nextItem = {it.ruleIndex, it.dotPos + 1};
                    addItemToState(&newState, nextItem);
                }
            }
            closure(&newState);
            if(newState.itemCount > 0) {
                int nextStateId = findState(&newState);
                if(nextStateId == -1) {
                    newState.id = stateCount; states[stateCount] = newState;
                    nextStateId = stateCount; stateCount++;
                }
                if(IS_TERMINAL(X)) setAction(processed, X, ACT_SHIFT, nextStateId);
                else setGoto(processed, GET_NT(X), nextStateId);
            }
        }
        processed++;
    }

    // 填充 REDUCE 和 ACC
    for(int i=0; i<stateCount; i++) {
        for(int j=0; j<states[i].itemCount; j++) {
            Item it = states[i].items[j];
            GenRule r = rules[it.ruleIndex];
            if(it.dotPos == r.len) {
                if(r.lhs == 0) {
                    setAction(i, TOK_END, ACT_ACC, 0);
                } else {
                    // 遍历所有 Token (包括 TOK_END = 0)
                    for(int t=0; t<20; t++) {
                        if(followSet[r.lhs][t]) setAction(i, t, ACT_REDUCE, r.id);
                    }
                }
            }
        }
    }
    printf("[AutoGenerator] Table Generation Complete! Total States: %d\n", stateCount);
}

// ==========================================
// PART 2: Parser 驱动程序 (Driver)
// ==========================================

// 初始化
static void initParser() {
    // 1. 初始化运行时产生式信息
    setProd(1, 1, 4); setProd(2, 1, 6); setProd(3, 1, 10); setProd(4, 1, 3);
    setProd(5, 2, 3); setProd(6, 2, 1); setProd(7, 3, 3); setProd(8, 3, 1);
    setProd(9, 4, 1); setProd(10, 4, 1); setProd(11, 5, 3); setProd(12, 6, 0);
    setProd(13, 7, 0); setProd(14, 3, 3);

    // 2. 清空表格
    memset(actionTable, 0, sizeof(actionTable));
    memset(gotoTable, 0, sizeof(gotoTable));

    // 3. 执行自动生成
    generateSLRTable();
}

// 对外接口：启动分析
void SLR1_Parser() {
    initParser();
    stateStack[top] = 0;
    Token token = getToken();

    printf("Starting SLR(1) Parser...\n");

    while(1) {
        int s = stateStack[top];
        ActionEntry act = actionTable[s][token.type];

        if (act.type == ACT_ERR) {
            printf("Syntax Error at State %d, Token %s (Type %d)\n", s, token.value, token.type);
            exit(1);
        }
        else if (act.type == ACT_SHIFT) {
            printf("SHIFT  %-5s -> State %d\n", token.value, act.val);
            top++;
            stateStack[top] = act.val;

            // 语义栈操作
            strcpy(symStack[top].name, token.value);
            symStack[top].tl_count = symStack[top].fl_count = symStack[top].nl_count = 0;

            token = getToken();
        }
        else if (act.type == ACT_REDUCE) {
            int prodID = act.val;
            int len = prods[prodID].len;
            int lhs = prods[prodID].lhs;

            printf("REDUCE Rule %d\n", prodID);

            SemNode newVal;
            // 语义属性初始化
            newVal.tl_count = newVal.fl_count = newVal.nl_count = 0;
            newVal.quad = 0;
            memset(newVal.name, 0, sizeof(newVal.name));

            // 执行语义动作
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

            top++;
            stateStack[top] = nextState;
            symStack[top] = newVal;
        }
        else if (act.type == ACT_ACC) {
            printf("Analysis Success!\n");
            return;
        }
    }
}