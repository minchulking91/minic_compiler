//
// Created by minch on 2016-12-18.
//
#include "Symbol.h"

void delete(Symbol **pType);

void initSymbol() {
    memset(hashTable, -1, sizeof(int)*SYMBOL_TABLE_SIZE);
    memset(symbolTable, 0, sizeof(Symbol*)*SYMBOL_TABLE_SIZE);
    symbolTop = 0;

    lvTop = 0;
    memset(lvTable, -1, sizeof(int)*LEVEL_STACK_SIZE);
    lvTable[lvTop] = 0;

}


Symbol* insert(char *name, enum TYPE_SPEC typeSpecifier, enum TYPE_QUALIFIER typeQualifier, int size, int initValue) {
    //return symbols index;
    int index = getHashIndex(name);
    Symbol *p = symbolTable[index];
    hashTable[index] =

    Symbol *symbol = (Symbol*) malloc(sizeof(Symbol));
    memset(symbol, 0, sizeof(Symbol));
    symbol->name = name;
    symbol->typeQualifier = typeQualifier;
    symbol->typeSpecifier = typeSpecifier;
    symbol->block = block;
    symbol->offset = offset;
    symbol->size = size;
    symbol->initValue = initValue;

    symbolTable[symbolTop] = symbol;
    symbolTop++;
    blockOffset[blockTop]+=size;

    return symbolTop-1;
}

Symbol* lookup(char* name){
    int hashIndex = getHashIndex(name);
    int index = hashTable[hashIndex];
    if(symbolTable[index]==NULL){
        return NULL;
    }
    Symbol* p = symbolTable[index];
    while(p){
        if(strcmp(p->name, name) == 0){
            return p;
        }
        p=p->next;
    }
    return NULL;
}

int typeSize(int specifier) {
    switch(specifier){
        case INT_TYPE:
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
int setBlock(){
    lvTop++;
    lvTable[lvTop] = symbolTop + 1;
}
/**
 * block 시작 인덱스이후에 있는 모든 심볼들을 제거한다.
 * @return
 */
int unsetBlock(){
    int s = lvTable[lvTop];
    for(int i=s; i<=symbolTop; i++){
        //remove symbol
        delete(&symbolTable[i]);
    }
    lvTable[lvTop] = -1;
    lvTop--;
    symbolTop = s-1;
}

void delete(Symbol **pType) {
    free(*pType);
    *pType = NULL;
}

int getHashIndex(char* name){
    int len = strlen(name);
    int sum = 0;
    for(int i=0; i<len; i++){
        sum = sum + (int)*(name+i);
    }
    return sum % HASH_SIZE;
}