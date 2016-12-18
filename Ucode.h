//
// Created by minch on 2016-12-18.
//

#ifndef MINIC_COMPILER_UCODE_H
#define MINIC_COMPILER_UCODE_H
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
char* ucodeName[]={
    "notop", "neg", "inc", "dec", "dup", //unary
    "add", "sub", "mult", "div", "mod", "swp", //binary
    "and", "or", "gt", "lt", "ge", "le", "eq", "ne", //binary
    "lod", "str", "ldc", "lda", //stack control
    "ujp", "tjp", "fjp", //flow control
    "chkh", "chkl",
    "ldi", "sti", //indirect (call, load by address)
    "call", "ret", "retv", "ldp", "proc", "end", //function
    "nop", "bgn", "sym"
};
#endif //MINIC_COMPILER_UCODE_H
