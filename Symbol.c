//
// Created by minch on 2016-12-18.
//
#include <stdio.h>
#include "Symbol.h"
char* typeSpecName[]={
    "INT_TYPE", "VOID_TYPE", "REP_TYPE"
};
char* typeQualName[] = {
    "VAR_TYPE", "CONST_TYPE", "FUNC_TYPE"
};
FILE* stout = NULL;
void initSymbol() {
    memset(hashTable, -1, sizeof(int)*SYMBOL_TABLE_SIZE);
    memset(symbolTable, 0, sizeof(Symbol*)*SYMBOL_TABLE_SIZE);
    symbolTop = 0;


    memset(lvTable, 0, sizeof(int)*LEVEL_STACK_SIZE);

    Block* block = (Block*) malloc(sizeof(Block));
    memset(block, 0, sizeof(Block));
    block->number = 1;
    block->offset = 1;
    lvTable[0] = block;
    lvTop = 1;
}


Symbol* insert(char *name, enum TYPE_SPEC typeSpecifier, enum TYPE_QUALIFIER typeQualifier, int size, int initValue) {
    //return symbols index;
    int hashIndex = getHashIndex(name);


    Symbol *symbol = (Symbol*) malloc(sizeof(Symbol));
    memset(symbol, 0, sizeof(Symbol));
    symbol->name = name;
    symbol->typeQualifier = typeQualifier;
    symbol->typeSpecifier = typeSpecifier;
    Block *block = topBlock();
    switch(typeQualifier){
        case VAR_TYPE:
            symbol->block = block->number;
            symbol->size = size;
            symbol->initValue = 0;
            symbol->offset = block->offset;
            block->offset += size;
            break;
        case CONST_TYPE:
            symbol->block = 0;
            symbol->size = 0;
            symbol->initValue = initValue;
            symbol->offset = 0;
            break;
        case FUNC_TYPE:
            symbol->block = 0;
            symbol->size = size;
            symbol->initValue = 0;
            symbol->offset = 0;
            break;
        default:
            break;
    }
    if(hashTable[hashIndex] != -1){
        symbol->nextIndex = hashTable[hashIndex];
    }else{
        symbol->nextIndex = -1;
    }
    hashTable[hashIndex] = symbolTop;
    symbolTable[symbolTop] = symbol;
    symbolTop++;
    return symbol;
}

Symbol* lookup(char* name){
    int hashIndex = getHashIndex(name);
    if(hashTable[hashIndex]==-1){
        return NULL;
    }
    int index = hashTable[hashIndex];
    Symbol *p = symbolTable[index];
    while(p){
        if(strcmp(p->name, name) == 0){
            return p;
        }
        p=symbolTable[p->nextIndex];
    }
    return NULL;
}

int typeSize(int specifier) {
    switch(specifier){
        case INT_TYPE: case REP_TYPE:
            return 1;
        default:
            return 0;
    }
}
/**
 * current block이 증가하며 앞으로 insert되는 symbol들은 current block들을 block으로 가진다.
 *
 * @return
 */
void setBlock(){ //push
    Block* block = (Block*) malloc(sizeof(Block));
    memset(block, 0, sizeof(Block));
    block->number = topBlock()->number + 1;
    block->offset = 1;
    block->stIndex = symbolTop;
    lvTable[lvTop] = block;
    lvTop++;
}
/**
 * block 시작 인덱스이후에 있는 모든 심볼들을 제거한다.
 * @return
 */
void unsetBlock(){ //pop
    int blockNumber = topBlock()->number;
//    char buf[100] = {0, };
//    sprintf(buf, "before unset block %d", blockNumber);
//    printSymbolTable(buf);

    //remove sybols
    Block *block = topBlock();
    for(int i=symbolTop-1; i>=block->stIndex; i--) {
        //remove symbol
        Symbol* symbol = symbolTable[i];
        if(symbol->nextIndex != -1){
            Symbol* next = symbolTable[symbol->nextIndex];
            int nextIndex = symbol->nextIndex;
            hashTable[getHashIndex(symbol->name)] = nextIndex;
        }else{
            hashTable[getHashIndex(symbol->name)] = -1;
        }
        free(symbolTable[i]);
        symbolTable[i] = NULL;
    }
    symbolTop = (block->stIndex);
    //remove block
    free(block);
    lvTable[lvTop-1] = NULL;
    lvTop--;
//    memset(buf, 0, 100);
//    sprintf(buf, "after unset block %d", blockNumber);
//    printSymbolTable(buf);
}

Block* getBlock(){
    return lvTable[lvTop-1];
}
int getSymbolCount(){
    Block *block = topBlock();
    for(int i=block->stIndex; i<SYMBOL_TABLE_SIZE; i++){
        if(symbolTable[i] == NULL){
            return (i - block->stIndex);
        }
    }
    return 0;
}
Symbol* getSymbol(int index){
    return symbolTable[index + topBlock()->stIndex];
}
Block* topBlock(){
    return lvTable[lvTop-1];
}
int getHashIndex(char* name){
    int len = strlen(name);
    int sum = 0;
    for(int i=0; i<len; i++){
        sum = sum + (int)*(name+i);
    }
    return sum % HASH_SIZE;
}
void printSymbolTable(char *label) {
    fprintf(stout ? stout : stdout, "\n-----------------------------------------------------------------------------\n");
    fprintf(stout ? stout : stdout, "    SYMBOL TABLE : %s", label);
    fprintf(stout ? stout : stdout, "\n-----------------------------------------------------------------------------\n");
    fprintf(stout ? stout : stdout, "    %6s %6s %6s %6s %5s %5s %10s %10s %10s", "index", "name", "block", "offset", "size", "init", "typeSpec", "typeQual", "nextIndex");
    fprintf(stout ? stout : stdout, "\n-----------------------------------------------------------------------------\n");
    for(int i=symbolTop-1; i>=0; i--){
        Symbol *s = symbolTable[i];
        fprintf(stout ? stout : stdout, "%6d %8s %6d %6d %5d %5d %10s %10s", i, s->name, s->block, s->offset, s->size, s->initValue, typeSpecName[s->typeSpecifier], typeQualName[s->typeQualifier]);
        if(s->nextIndex == -1){
            fprintf(stout ? stout : stdout, "      -- \n");
        }else {
            fprintf(stout ? stout : stdout, "     %3d\n", s->nextIndex);
        }
    }
    fprintf(stout ? stout : stdout, "-----------------------------------------------------------------------------\n");
    fprintf(stout ? stout : stdout, "    HASH TABLE\n");
    for(int i=0; i<HASH_SIZE; i++){
        fprintf(stout ? stout : stdout, "|%3d ", i);
    }
    fprintf(stout ? stout : stdout, "|\n");
    for(int i=0; i<HASH_SIZE; i++){
        if(hashTable[i] == -1){
            fprintf(stout ? stout : stdout, "| -- ");
        }else{
            fprintf(stout ? stout : stdout, "|%3d ", hashTable[i]);
        }

    }
    fprintf(stout ? stout : stdout, "|\n");
}
int openSTOutFile(char* fileName){
    if(stout != NULL){
        //already open
        printf("error(openSTOutFile) : file is already open! \n - symboltable print in STDOUT\n");
        return 1;
    }
    stout = fopen(fileName, "w");
    if(stout == NULL){
        printf("error(openSTOutFile) : invalid file to open %s!\n - symboltable print in STDOUT\n", fileName);
        return 1;
    }
    printf("symbolTable is print in %s\n", fileName);
    return 0;
}
int closeSTOut(){
    fclose(stout);
    stout = NULL;
}