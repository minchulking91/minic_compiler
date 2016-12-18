//
// Created by minch on 2016-12-17.
//

#ifndef MINIC_COMPILER_SNODE_H
#define MINIC_COMPILER_SNODE_H

#include <stdlib.h>
#include <mem.h>
#define HASH_SIZE 17
#define SYMBOL_TABLE_SIZE 32
#define LEVEL_STACK_SIZE 16

enum TYPE_SPEC{
    INT_TYPE, VOID_TYPE
};
enum TYPE_QUALIFIER{
    VAR_TYPE, CONST_TYPE, FUNC_TYPE
};

typedef struct symbolType{
    char* name;
    int block;
    int offset;
    enum TYPE_SPEC typeSpecifier;
    enum TYPE_QUALIFIER typeQualifier;
    int size;
    int initValue;
    struct symbolType* next;
}Symbol;

typedef struct blockType{

}Block;
int hashTable[HASH_SIZE];
Symbol* symbolTable[SYMBOL_TABLE_SIZE];
int symbolTop;

int lvTable[LEVEL_STACK_SIZE];
int lvTop;

Symbol* insert(char *name, enum TYPE_SPEC typeSpecifier, enum TYPE_QUALIFIER typeQualifier, int size, int initValue);
Symbol* lookup(char *name);
void initSymbol();
int typeSize(int specifier);
int setBlock();
int unsetBlock();
int getHashIndex(char* name);

#endif //MINIC_COMPILER_SNODE_H
