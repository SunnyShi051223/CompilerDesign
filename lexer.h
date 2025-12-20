//
// Created by 32874 on 2025/12/20.
//

#ifndef COMPILERDESIGN_LEXER_H
#define COMPILERDESIGN_LEXER_H


#include "common.h"

// 初始化词法分析器
void initLexer(const char *source);

// 获取下一个 Token
Token getToken();

#endif //COMPILERDESIGN_LEXER_H
