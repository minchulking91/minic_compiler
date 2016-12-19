//
// Created by minch on 2016-12-20.
//

#include "Ucode.h"

FILE *uout = NULL;

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

void emit1(enum U_CODE code, int param) {
    fprintf(uout?uout:stdout, "\t\t%s\t\t%d\n", ucodeName[code], param);
}

void emit2(enum U_CODE code, int param1, int param2) {
    fprintf(uout?uout:stdout, "\t\t%s\t\t%d\t%d\n", ucodeName[code], param1, param2);
}

void emit3(enum U_CODE code, int param1, int param2, int param3) {
    fprintf(uout?uout:stdout, "\t\t%s\t\t%d\t%d\t%d\n", ucodeName[code], param1, param2, param3);
}

void emitLabel(char *label) {
    fprintf(uout?uout:stdout, "%s\t\t%s\n", label, ucodeName[nop]);
}

void emitSymbol(enum U_CODE code, int block, int offset, int size, char *name) {
    fprintf(uout?uout:stdout, "\t\t%s\t\t%d\t%d\t%d\t//%s\n", ucodeName[code], block, offset, size, name);
}

void emitProc(char *name, int number, int size, int lexical_lv) {
    fprintf(uout?uout:stdout, "%s\t%s\t%d\t%d\t%d\n", name, ucodeName[proc], size, number, lexical_lv);
}

void emitJump(enum U_CODE code, char* label) {
    fprintf(uout?uout:stdout, "\t\t%s\t\t%s\n", ucodeName[code], label);
}

void emit0(enum U_CODE code) {
    fprintf(uout?uout:stdout, "\t\t%s\t\n", ucodeName[code]);
}

int openUCodeOutFile(char* fileName){
    if(uout != NULL){
        //already open
        printf("error(openUCodeOutFile) : file is already open! \n - ucode print in STDOUT\n");
        return 1;
    }
    uout = fopen(fileName, "w");
    if(uout == NULL){
        printf("error(openUCodeOutFile) : invalid file to open %s!\n - ucode print in STDOUT\n", fileName);
        return 1;
    }
    printf("ucode is print in %s\n", fileName);
    return 0;
}
int closeUCodeOut(){
    fclose(uout);
    uout = NULL;
}