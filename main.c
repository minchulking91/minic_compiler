#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include <string.h>

#include "ASTNode.h"
#include "SNode.h"

#define DEFAULT_READ_FILE "out.ast"
#define VALUE_BUF_SIZE 16
#define STACK_SIZE 128
#define STRING_POOL_SIZE 128
#define HASH_SIZE 128
#define BLOCK_STACK_SIZE 10
FILE* fp;

//symbolTable
char stringPool[STRING_POOL_SIZE];
int hashIndex[HASH_SIZE];
SNode* symbolTable[HASH_SIZE];
int blockStack[BLOCK_STACK_SIZE];
int blockTop;
SName* getSymbolName(char* ident);
int getHashIndex(char* );

/**
void addSymbol();
void findSymbol();
void setBlock();
void resetBlock();
 */

//for import AST
ASTNode* importTree(char*);
ASTNode *readNode(FILE *fp);

//for printTree
void printNode(ASTNode* node, int indent);
void printTree(ASTNode* node, int indent);


int main() {
    ASTNode* root = importTree(NULL);

    return 0;
}

ASTNode* importTree(char* filename){
    ASTNode* stack[STACK_SIZE] = {0, };
    int flag[STACK_SIZE] = {0, };
    ASTNode* root;
    int top = 0;
    fp = NULL;
    fp = fopen(filename == NULL ? DEFAULT_READ_FILE : filename, "r");
    if(fp == NULL){
        printf("invalid file to open\n");
        exit(1);
    }
    root = readNode(fp);
    stack[top] = root;
    flag[top] = 0;
    while(!feof(fp)){
        /**
         * flag[top]
         * 0; current is top's child
         * 1; current is top's brother
         */
        ASTNode* current = readNode(fp);
        if(flag[top] == 0){
            if(current == NULL){ //next is brother
                flag[top] = 1;
            }else{ //current is child - push
                stack[top]->child = current;
                top++;
                stack[top] = current;
                flag[top] = 0;
            }
        }else{
            if(current == NULL){ //complete read child and brother - pop
                stack[top] = NULL;
                top--;
                flag[top] = 1;
            }else{ //current is brother
                stack[top]->brother = current;
                stack[top] = current;
                flag[top] = 0;
            }
        }
    }
    fclose(fp);
    return root;
}

ASTNode *readNode(FILE *fp) {
    int number;
    char valueBuf[VALUE_BUF_SIZE];
    ASTNode* node;
    if(fp == NULL){
        return NULL;
    }
    if(feof(fp)){
        return NULL;
    }
    fscanf(fp, "%d", &number);
    if(number == -1){
        return NULL;
    }
    node = (ASTNode*) malloc(sizeof(ASTNode));
    memset(node, 0, sizeof(ASTNode));
    if(number < 0){ //meanful node
        int len;
        node->token.number = -number;
        //get value
        memset(valueBuf, 0, sizeof(char)*VALUE_BUF_SIZE);
        fscanf(fp, "%s", valueBuf);
        len = strlen(valueBuf);
        node->token.value = (char*)malloc(sizeof(char) * (len+1));
        node->noderep = terminal;
        memset(node->token.value, 0, sizeof(char) * (len+1));
        strcpy(node->token.value, valueBuf);
    }else{
        node->token.number = number;
        node->noderep = nonterm;
    }
    return node;
}

/*print AST function*/
void printNode(ASTNode* node, int indent){
    int i;
    for(i=1; i<=indent; i++){
        printf(" ");
    }
    if(node->noderep == terminal){
        printf(" Terminal: %s", node->token.value);
    }else{
        printf(" Nonterminal: %d", node->token.number);
    }
    printf("\n");
}

void printTree(ASTNode* node, int indent){
    ASTNode* p = node;
    while(p!=NULL){
        printNode(p, indent);
        if(p->noderep == nonterm) printTree(p->child, indent+5);
        p = p->brother;
    }
}

/*symbol table*/
SName* getSymbolName(char* ident){
    SName* name;
    char* p;
    if(ident == NULL){
        return NULL;
    }
    name = (SName*) malloc(sizeof(SName));
    memset(name, 0, sizeof(SName));

    p = strstr(stringPool, ident);
    if(p == NULL){
        //add string
        int start = strlen(stringPool);
        int length = strlen(ident);
        strcat(stringPool, ident);
        name->index = start;
        name->length = length;
    }else{
        int start = ((int)stringPool - (int)p);
        int length = strlen(ident);
        name->index = start;
        name->length = length;
    }

    return name;
}