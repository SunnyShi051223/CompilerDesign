/* lexer.c */
#include "lexer.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char *src;
static int pos = 0;

void initLexer(const char *source) {
    src = source;
    pos = 0;
}

const char* getTokenName(int type) {
    switch(type) {
        case TOK_END:    return "TOK_END";
        case TOK_IF:     return "TOK_IF";
        case TOK_ELSE:   return "TOK_ELSE";
        case TOK_ID:     return "TOK_ID";
        case TOK_NUM:    return "TOK_NUM";
        case TOK_RELOP:  return "TOK_RELOP";
        case TOK_ASSIGN: return "TOK_ASSIGN";
        case TOK_PLUS:   return "TOK_PLUS";
        case TOK_MINUS:  return "TOK_MINUS";
        case TOK_LPAREN: return "TOK_LPAREN";
        case TOK_RPAREN: return "TOK_RPAREN";
        case TOK_LBRACE: return "TOK_LBRACE";
        case TOK_RBRACE: return "TOK_RBRACE";
        case TOK_SEMI:   return "TOK_SEMI";
        default:         return "UNKNOWN";
    }
}

Token getToken() {
    Token t;
    char ch = src[pos];

    while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
        pos++;
        ch = src[pos];
    }

    if (ch == '\0') {
        t.type = TOK_END;
        strcpy(t.value, "$");
        return t;
    }

    if (isalpha(ch)) {
        int i = 0;
        while (isalnum(ch)) {
            if (i < 31) t.value[i++] = ch;
            pos++;
            ch = src[pos];
        }
        t.value[i] = '\0';

        if (strcmp(t.value, "if") == 0) t.type = TOK_IF;
        else if (strcmp(t.value, "else") == 0) t.type = TOK_ELSE;
        else t.type = TOK_ID;
        return t;
    }

    if (isdigit(ch)) {
        int i = 0;
        while (isdigit(ch)) {
            if (i < 31) t.value[i++] = ch;
            pos++;
            ch = src[pos];
        }
        t.value[i] = '\0';
        t.type = TOK_NUM;
        return t;
    }

    pos++;
    switch (ch) {
        case '=':
            if (src[pos] == '=') { pos++; t.type = TOK_RELOP; strcpy(t.value, "=="); }
            else { t.type = TOK_ASSIGN; strcpy(t.value, "="); }
            break;
        case '>':
            if (src[pos] == '=') { pos++; t.type = TOK_RELOP; strcpy(t.value, ">="); }
            else { t.type = TOK_RELOP; strcpy(t.value, ">"); }
            break;
        case '<':
            if (src[pos] == '=') { pos++; t.type = TOK_RELOP; strcpy(t.value, "<="); }
            else { t.type = TOK_RELOP; strcpy(t.value, "<"); }
            break;
        case '!':
            if (src[pos] == '=') { pos++; t.type = TOK_RELOP; strcpy(t.value, "!="); }
            else { printf("Lexical Error: Unexpected character '!'\n"); exit(1); }
            break;
        case '+': t.type = TOK_PLUS;  strcpy(t.value, "+"); break;
        case '-': t.type = TOK_MINUS; strcpy(t.value, "-"); break;
        case '(': t.type = TOK_LPAREN; strcpy(t.value, "("); break;
        case ')': t.type = TOK_RPAREN; strcpy(t.value, ")"); break;
        case '{': t.type = TOK_LBRACE; strcpy(t.value, "{"); break;
        case '}': t.type = TOK_RBRACE; strcpy(t.value, "}"); break;
        case ';': t.type = TOK_SEMI;   strcpy(t.value, ";"); break;
        default:
            t.type = TOK_END;
            printf("Lexical Error: Unknown character '%c'\n", ch);
            exit(1);
    }
    return t;
}