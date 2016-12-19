//
// Created by minch on 2016-12-18.
//

#ifndef MINIC_COMPILER_UCODE_H
#define MINIC_COMPILER_UCODE_H
#include <stdio.h>
enum U_CODE{
    notop, neg, incop, decop, dup, //unary
    add, sub, mult, divop, modop, swp, //binary
    andop, orop, gt, lt, ge, le, eq, ne, //binary
    lod, str, ldc, lda, //stack control
    ujp, tjp, fjp, //flow control
    chkh, chkl,
    ldi, sti, //indirect (call, load by address)
    call, ret, retv, ldp, proc, endop, //function
    nop, bgn, sym
};

void emitSymbol(enum U_CODE code, int block, int offset, int size, char *name);

void emitProc(char *name, int number, int offset, int lexical_lv);

void emitJump(enum U_CODE code, char* label);

void emit0(enum U_CODE code);

void emit1(enum U_CODE code, int param);

void emit2(enum U_CODE code, int param1, int param2);

void emitLabel(char *label);

void emit3(enum U_CODE code, int param1, int param2, int param3);

int openUCodeOutFile(char* fileName);
int closeUCodeOut();
#endif //MINIC_COMPILER_UCODE_H
