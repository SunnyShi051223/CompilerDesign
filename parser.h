//
// Created by 32874 on 2025/12/20.
//

#ifndef COMPILERDESIGN_PARSER_H
#define COMPILERDESIGN_PARSER_H

#include "common.h"

// 产生式数量 (0-14)
#define PROD_COUNT 15

// 动作类型
#define ACT_ERR    0
#define ACT_SHIFT  1
#define ACT_REDUCE 2
#define ACT_ACC    3

// 生成器限制常量
#define MAX_ITEMS 200
#define MAX_RULES 20

// 动作表项 (存储分析表的每一个格子)
typedef struct {
    int type; // ACT_SHIFT, ACT_REDUCE, ACT_ACC, ACT_ERR
    int val;  // State ID 或 Production ID
} ActionEntry;

// 运行时产生式信息 (用于 Reduce 时获取左部和长度)
typedef struct {
    int lhs; // 左部非终结符
    int len; // 右部长度
} Production;

/**
 * SLR(1) 语法分析器主入口
 * * 流程：
 * 1. 初始化文法和 Follow 集
 * 2. 自动构建 LR(0) 项目集规范族 (DFA)
 * 3. 根据 DFA 和 Follow 集自动生成 Action/Goto 表
 * 4. 进入分析循环 (Driver Loop)
 */
void SLR1_Parser();


#endif //COMPILERDESIGN_PARSER_H
