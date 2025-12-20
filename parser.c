//
// Created by 32874 on 2025/12/20.
//

/* parser.c */
#include "parser.h"
#include "lexer.h"
#include "semantic.h"

// 动作类型
#define ACT_ERR 0
#define ACT_SHIFT 1
#define ACT_REDUCE 2
#define ACT_ACC 3

// 产生式编号宏
#define P_S_ASSIGN 1  // S -> id = num; (简化)
#define P_S_IF     2  // S -> if ( C ) M S
#define P_C_RELOP  3  // C -> id relop id
#define P_M_EPS    4  // M -> epsilon

typedef struct {
    int type; // SHIFT/REDUCE
    int val;  // NextState or ProductionIndex
} Action;

// 栈
int stateStack[MAX_STACK];
SemNode symStack[MAX_STACK];
int top = 0;

// --- 模拟分析表 (仅针对 demo 用例: if ( a > b ) x = 1 ;) ---
// 真实课设中，这个表应该是 50x20 的大数组
Action getAction(int state, TokenType tok) {
    Action a = {ACT_ERR, 0};

    // 状态 0: Start. 期待 'if' 或 'id'
    if (state == 0 && tok == TOK_IF) { a.type = ACT_SHIFT; a.val = 2; return a; }

    // 状态 2: 读了 'if'，期待 '('
    if (state == 2 && tok == TOK_LPAREN) { a.type = ACT_SHIFT; a.val = 3; return a; }

    // 状态 3: 读了 '(', 期待 'id' (进入条件 C)
    if (state == 3 && tok == TOK_ID) { a.type = ACT_SHIFT; a.val = 4; return a; }

    // 状态 4: 读了 'id', 期待 'relop'
    if (state == 4 && tok == TOK_RELOP) { a.type = ACT_SHIFT; a.val = 5; return a; }

    // 状态 5: 读了 'relop', 期待 'id'
    if (state == 5 && tok == TOK_ID) { a.type = ACT_SHIFT; a.val = 6; return a; }

    // 状态 6: 读了 id > id. 此时应该归约 C -> id relop id
    // 这里因为是 SLR，要看 FOLLOW 集，假设下一个是 ')'，则归约
    if (state == 6 && tok == TOK_RPAREN) { a.type = ACT_REDUCE; a.val = P_C_RELOP; return a; }

    // 状态 7 (GOTO C后): 读了 C，期待 ')'
    if (state == 7 && tok == TOK_RPAREN) { a.type = ACT_SHIFT; a.val = 8; return a; }

    // 状态 8: 读了 ')', 此时需要 M -> epsilon.
    // SLR表中，如果下一个是 id 或 if (S的开头)，则先归约 M
    if (state == 8 && tok == TOK_ID) { a.type = ACT_REDUCE; a.val = P_M_EPS; return a; }

    // 状态 9 (GOTO M后): 期待 S (这里简化为只接受 x=1)
    if (state == 9 && tok == TOK_ID) { a.type = ACT_SHIFT; a.val = 10; return a; } // id

    // 状态 10: x, 期待 =
    if (state == 10 && tok == TOK_ASSIGN) { a.type = ACT_SHIFT; a.val = 11; return a; }

    // 状态 11: =, 期待 num
    if (state == 11 && tok == TOK_NUM) { a.type = ACT_SHIFT; a.val = 12; return a; }

    // 状态 12: num, 期待 ;
    if (state == 12 && tok == TOK_SEMI) { a.type = ACT_SHIFT; a.val = 13; return a; }

    // 状态 13: ;, 归约 S -> id = num ;
    if (state == 13 && (tok == TOK_END || tok == TOK_ELSE)) { a.type = ACT_REDUCE; a.val = P_S_ASSIGN; return a; }

    // 状态 14 (GOTO S后): 归约 S -> if ( C ) M S
    if (state == 14 && tok == TOK_END) { a.type = ACT_REDUCE; a.val = P_S_IF; return a; }

    // 接受
    if (state == 1 && tok == TOK_END) { a.type = ACT_ACC; return a; }

    return a;
}

// GOTO 表
int getGoto(int state, int nonTerminal) {
    // 0: C, 1: M, 2: S
    if (state == 3 && nonTerminal == 0) return 7;  // 3 + C -> 7
    if (state == 8 && nonTerminal == 1) return 9;  // 8 + M -> 9
    if (state == 9 && nonTerminal == 2) return 14; // 9 + S -> 14
    if (state == 0 && nonTerminal == 2) return 1;  // 0 + S -> 1 (ACC state)
    return 0;
}

void SLR1_Parser() {
    stateStack[top] = 0;
    Token token = getToken();

    printf("Starting Parser...\n");

    while(1) {
        int s = stateStack[top];
        Action act = getAction(s, token.type);

        if (act.type == ACT_SHIFT) {
            printf("SHIFT to %d, Token: %s\n", act.val, token.value);
            top++;
            stateStack[top] = act.val;

            // 将 Token 信息保存到语义栈
            strcpy(symStack[top].name, token.value);
            // 清空列表
            symStack[top].tl_count = symStack[top].fl_count = symStack[top].nl_count = 0;

            token = getToken();
        }
        else if (act.type == ACT_REDUCE) {
            printf("REDUCE by Prod %d\n", act.val);

            SemNode newVal; // 新生成的非终结符属性
            newVal.tl_count = newVal.fl_count = newVal.nl_count = 0;
            int popLen = 0;
            int nonTerm = 0; // 0:C, 1:M, 2:S

            // --- 语义动作 ---
            switch(act.val) {
                case P_C_RELOP: // C -> id relop id
                    popLen = 3; nonTerm = 0;
                    // 栈: id(top-2), relop(top-1), id(top)
                    makeList(&newVal, NXQ, 0);   // TrueList
                    makeList(&newVal, NXQ+1, 1); // FalseList
                    emit("j>", symStack[top-2].name, symStack[top].name, 0, 1);
                    emit("j", "-", "-", 0, 1);
                    break;

                case P_M_EPS: // M -> epsilon
                    popLen = 0; nonTerm = 1;
                    newVal.quad = NXQ; // 记录当前指令地址
                    break;

                case P_S_ASSIGN: // S -> id = num ;
                    popLen = 4; nonTerm = 2;
                    // 栈: id(top-3), =(top-2), num(top-1), ;(top)
                    emit("=", symStack[top-1].name, "-", 0, 0); // 简单赋值，result暂存为0表示占位
                    // 修改上一句赋值的 result 为 id
                    sprintf(quadArray[NXQ-1].arg2, "%s", symStack[top-3].name); // hack: 将arg2设为目标变量
                    break;

                case P_S_IF: // S -> if ( C ) M S
                    popLen = 5; nonTerm = 2; // if, (, C, ), M, S (注意M是空的，不在Action栈中占位，但在逻辑上存在)
                    // 修正：在手动SLR中，M已经归约过了，所以在栈里是：
                    // if(top-5), ((top-4), C(top-3), )(top-2), M(top-1), S(top)

                    // 回填 C.True -> M.quad
                    backpatch(symStack[top-3].trueList, symStack[top-3].tl_count, symStack[top-1].quad);
                    // C.False -> NXQ (假定S.next就是NXQ，这里简单处理)
                    backpatch(symStack[top-3].falseList, symStack[top-3].fl_count, NXQ);

                    mergeList(newVal.nextList, &newVal.nl_count, symStack[top].nextList, symStack[top].nl_count);
                    break;
            }

            // 弹栈
            top -= popLen;

            // 查 Goto 并压栈
            int nextState = getGoto(stateStack[top], nonTerm);
            top++;
            stateStack[top] = nextState;
            symStack[top] = newVal;
        }
        else if (act.type == ACT_ACC) {
            printf("Analysis Success!\n");
            return;
        }
        else {
            printf("Syntax Error at state %d, token %s\n", s, token.value);
            exit(1);
        }
    }
}