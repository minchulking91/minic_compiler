#include <stdio.h>
#include <stdlib.h>
#include <mem.h>

#include "Node.h"
#include "Symbol.h"
#include "Ucode.h"

#define VALUE_BUF_SIZE 16
#define AST_STACK_SIZE 128

#define LABEL_SIZE 10
#define LEXICAL_LEVEL 2

//for import AST
Node* importTree(char*);
Node *readNode(FILE *fp);

//for processing AST Node
void genCode(Node *node);
void processDeclaration(Node *node);
void processSimpleVariable(Node *node, enum TYPE_SPEC typeSpecifier, enum TYPE_QUALIFIER typeQualifier);
void processArrayVariable(Node *node, enum TYPE_SPEC typeSpecifier, enum TYPE_QUALIFIER typeQualifier);
void processFuncHeader(Node *node);
void processFunction(Node *node);
void processPARAM_DCL(Node *node);
void processStatement(Node *node);
void processCondition(Node *node);
void processRepVariable(Node *node, enum TYPE_SPEC spec, enum TYPE_QUALIFIER qualifier);
void processOperator(Node *node);
void genLabel(char *label);

void rv_emit(Node *node);
int checkPredefined(Node *node);
void emitSymbols();
void strsep(char*, char, int);
int main(int argc, char* argv[]) {
    char fileNameBuf[100];
    memset(fileNameBuf, 0, sizeof(char)*100);
    if( argc != 2){
        printf("Invalid argument!\n - ucode_gen.exe inputFile.ast.out\n");
        return 1;
    }
    strcat(fileNameBuf, argv[1]);
    Node* root = importTree(fileNameBuf);
    //open file for write uc
    if(root == NULL){
        return 1;
    }

    strsep(fileNameBuf, '.', 2);
    sprintf(fileNameBuf, "%s.uc", fileNameBuf);
    openUCodeOutFile(fileNameBuf);

    strsep(fileNameBuf, '.', 1);
    sprintf(fileNameBuf, "%s.st", fileNameBuf);
    openSTOutFile(fileNameBuf);

    genCode(root);

    closeUCodeOut();
    closeSTOut();
    return 0;
}

void genCode(Node *node) {
    Node *p;
    //initSymbolTable
    initSymbol();
    //step 1: process the declaration part
    for(p=node->child; p; p=p->brother){
        if(p->token.number == DCL) processDeclaration(p->child);
        else if(p->token.number == FUNC_DEF) processFuncHeader(p->child);
        else printf("error(genCode) : node is not DCL or FUNC_DEF\n");
    }
    printSymbolTable("GLOBAL declaration & funcHeader");
    //emit symbols in block(1)
    emitSymbols();
    //step 2: process the function part
    for(p=node->child; p; p=p->brother){
        if(p->token.number == FUNC_DEF) processFunction(p);
    }

    //step 3: gen codes for staring routine
    Block* block = getBlock();
    emit1(bgn, block->offset-1);
    emit0(ldp);
    emitJump(call, "main");
    emit0(endop);
}
void processDeclaration(Node *node) {//node = DCL_SPEC
    Node *p, *q;
    enum TYPE_SPEC typeSpecifier;
    enum TYPE_QUALIFIER typeQualifier;

    if(node->token.number != DCL_SPEC) printf("error(Declaration) : node is not DCL_SPEC\n");
    //step 1: process DCL_SPEC
    typeSpecifier = INT_TYPE;
    typeQualifier = VAR_TYPE;
    p = node->child;
    while(p){ //dcl_spec의 자식들을 탐색
        if(p->token.number == INT_NODE) typeSpecifier = INT_TYPE;
        else if(p->token.number == CONST_NODE) typeQualifier = CONST_TYPE;
        else {
            printf("not yet implemented\n");
            return;
        }
        p = p->brother;
    }

    //step 2: process DCL_ITEM
    p = node->brother;
    if(p->token.number != DCL_ITEM) printf("error(Declaration) : node is not DCL_SPEC\n");
    while(p){
        q = p->child;
        switch(q->token.number){
            case SIMPLE_VAR:
                processSimpleVariable(q, typeSpecifier, typeQualifier);
                break;
            case ARRAY_VAR:
                processArrayVariable(q, typeSpecifier, typeQualifier);
                break;
            default:
                printf("error(DCL) : invalid variable type\n");
                break;
        }
        p = p->brother;
    }
}

void processSimpleVariable(Node *node, enum TYPE_SPEC typeSpecifier, enum TYPE_QUALIFIER typeQualifier) {
    Node *p = node->child; //ident
    Node *q = node->brother; //initial value
    int size, initValue;
    int sign = 1;
    if(node->token.number != SIMPLE_VAR) printf("error : token number is expected SIMPLE_VAR(%d) but %d", SIMPLE_VAR, node->token.number);
    if(typeQualifier == CONST_TYPE){
        if(q == NULL){
            printf("%s must have a constant value\n", node->child->token.value);
            return;
        }
        if(q->token.number == UNARY_MINUS){
            sign = -1;
            q = q->child;
        }
        initValue = sign * atoi(q->token.value);
        insert(p->token.value, typeSpecifier, typeQualifier, typeSize(typeSpecifier), initValue);

    }else{ //variable type
        size = typeSize(typeSpecifier);
        insert(p->token.value, typeSpecifier, typeQualifier, size, 0);
    }
}
void processArrayVariable(Node *node, enum TYPE_SPEC typeSpecifier, enum TYPE_QUALIFIER typeQualifier) {
    Node *p = node->child;
    int size = 0;
    if(node->token.number != ARRAY_VAR) {
        printf("error(ARRAY_VAR) : token number is expected ARRAY_VAR(%d) but %d", ARRAY_VAR, node->token.number);
        return;
    }
    if(p->brother == NULL)
        printf("error(ARRAY_VAR) : array size must be specified\n");
    else size = atoi(p->brother->token.value);

    size *= typeSize(typeSpecifier);
    insert(p->token.value, typeSpecifier, typeQualifier, size, 0);

}
void processFuncHeader(Node *node) {
    int noArguments;
    enum TYPE_SPEC returnType = VOID_TYPE;
    Node *p;
    if(node->token.number != FUNC_HEAD){ printf("error in processFuncHeader\n");}
    //step 1:
    p = node->child->child;
    while(p){
        if(p->token.number == INT_NODE) returnType = INT_TYPE;
        else if(p->token.number == VOID_NODE) returnType = VOID_TYPE;
        else printf("invalid function return type\n");
        p=p->brother;
    }
    //step 2: count the number of formal pararmeters
    p=node->child->brother->brother; //FORMAL_PARA
    p=p->child;//PARAM_DCL
    noArguments = 0;
    while(p){
        noArguments++;
        p=p->brother;
    }
    //step 3: insert the function name
    insert(node->child->brother->token.value, returnType, FUNC_TYPE, noArguments, 0);
}
void processFunction(Node *node) { //node is FUNC_DEF
    char* functionName;
    char buf[40] = {0, };
    functionName = node->child->child->brother->token.value;

    Symbol* symbol = lookup(functionName);
    if(symbol == NULL){
        //not defined function
        return;
    }
    if(symbol->typeQualifier != FUNC_TYPE){
        //invalid symbol!
        return;
    }
    //insert symbol from FORMAL_PARA and DCL_LIST
    setBlock();
    Node *formal_para = node->child->child->brother->brother;
    if(formal_para->token.number != FORMAL_PARA){
        return;
    }
    Node *p = formal_para->child; //PARAM_DCL
    while(p){
        processPARAM_DCL(p->child);
        p=p->brother;
    }
    //process DCL_LIST
    Node *dcl_list = node->child->brother->child;
    if(dcl_list->token.number != DCL_LIST){
        return;
    }
    p = dcl_list->child;
    while(p){
        processDeclaration(p->child);
        p=p->brother;
    }

    //print symbolTable
    sprintf(buf, "setblock func(%s)", functionName);
    printSymbolTable(buf);

    //emit proc
    Block *block = getBlock();
    emitProc(functionName, block->number, block->offset-1, LEXICAL_LEVEL);
    emitSymbols();
    //process stat_list
    p = dcl_list->brother->child; //stat_list->child
    int flag = 0;
    while(p){
        processStatement(p);
        if(p->brother == NULL && p->token.number == RETURN_ST){
            flag = 1;
        }
        p=p->brother;
    }
    //return value 마지막 줄에 return은 생략되어 있을 수 있다.
    if(!flag){
        emit0(ret);
    }
    emit0(endop);
    unsetBlock();
    memset(buf, 0, 40);
    sprintf(buf, "unsetblock func(%s)", functionName);
    printSymbolTable(buf);
}

void processPARAM_DCL(Node *node) { //node == DCL_SPEC
    Node *p;
    enum TYPE_SPEC typeSpecifier;
    enum TYPE_QUALIFIER typeQualifier;

    if(node->token.number != DCL_SPEC) printf("error(PARAM_DCL) : node is not DCL_SPEC\n");
    //step 1: process DCL_SPEC
    typeSpecifier = INT_TYPE;
    typeQualifier = VAR_TYPE;
    p = node->child;
    while(p){ //dcl_spec의 자식들을 탐색
        if(p->token.number == INT_NODE) typeSpecifier = INT_TYPE;
        else if(p->token.number == CONST_NODE) typeQualifier = CONST_TYPE;
        else {
            printf("not yet implemented\n");
            return;
        }
        p = p->brother;
    }

    //step 2: process SIMPLE_VAR or ARRAY_VAR
    p = node->brother;

    switch(p->token.number){
        case SIMPLE_VAR:
            processSimpleVariable(p, typeSpecifier, typeQualifier);
            break;
        case ARRAY_VAR:
            processRepVariable(p, REP_TYPE, typeQualifier);
            break;
        default:
            printf("error(DCL) : invalid variable type\n");
            break;
    }
}
void processStatement(Node *node){
    switch(node->token.number){
        case COMPOUND_ST:
        {
            Node *p = node->child->brother;
            p = p->child;
            while(p){
                processStatement(p);
                p=p->brother;
            }
            break;
        }
        case EXP_ST:
            if(node->child != NULL)processOperator(node->child);
            break;
        case RETURN_ST:
        {
            Node *p;
            if(node->child != NULL){
                p = node->child;
                if(p->noderep == nonterm) processOperator(p);
                else rv_emit(p);
                emit0(retv);
            }else emit0(ret);
            break;
        }
        case IF_ST: {
            char label[LABEL_SIZE];
            genLabel(label);
            processCondition(node->child);
            emitJump(fjp, label);
            processStatement(node->child->brother);
            emitLabel(label);
            break;
        }
        case IF_ELSE_ST: {
            char label1[LABEL_SIZE], label2[LABEL_SIZE];
            genLabel(label1);
            genLabel(label2);
            processCondition(node->child); //condition
            emitJump(fjp, label1);
            processStatement(node->child->brother); //true
            emitJump(ujp, label2);
            emitLabel(label1);
            processStatement(node->child->brother->brother); //false
            emitLabel(label2);
            break;
        }
        case WHILE_ST:{
            char label1[LABEL_SIZE], label2[LABEL_SIZE];
            genLabel(label1);
            genLabel(label2);
            emitLabel(label1);
            processCondition(node->child); //condition
            emitJump(fjp, label2);
            processStatement(node->child->brother); //loop body
            emitJump(ujp, label1);
            emitLabel(label2);
            break;
        }
        default:
            printf("not yet implemented.\n");
            break;
    }
}

void processCondition(Node *node) {
    if(node->noderep == nonterm) processOperator(node);
    else rv_emit(node);
}

void processRepVariable(Node *node, enum TYPE_SPEC spec, enum TYPE_QUALIFIER qualifier) {
    Node *p = node->child;
    if(node->token.number != ARRAY_VAR) {
        printf("error(RepVariable) : token number is expected ARRAY_VAR(%d) but %d", ARRAY_VAR, node->token.number);
        return;
    }
    insert(p->token.value, spec, qualifier, typeSize(spec), 0);

}

void processOperator(Node *node){
    static int lvalue = 0;
    switch(node->token.number){
        case ASSIGN_OP:
        {
            Node *lhs = node->child, *rhs = node->child->brother;
            //step 1: gen left-hand side first
            if(lhs->noderep == nonterm){ //lhs is array
                lvalue = 1;
                processOperator(lhs);
                lvalue = 0;
            }
            //step 2: gen right-hand side next
            if(rhs->noderep == nonterm){ processOperator(rhs); }
            else{ rv_emit(rhs); }
            //step 3: gen a store instruction
            if(lhs->noderep == terminal){ //simple variable
                Symbol *symbol = lookup(lhs->token.value);
                if(symbol == NULL){
                    printf("error(OPERATOR) : undefined variable : %s\n", lhs->token.value);
                    return;
                }
                emit2(str, symbol->block, symbol->offset);
            } else
                emit0(sti);
            break;
        }
        case ADD_ASSIGN: case SUB_ASSIGN: case MUL_ASSIGN: case DIV_ASSIGN: case MOD_ASSIGN:
        {
            Node *lhs = node->child, *rhs=node->child->brother;
            int nodeNumber = node->token.number;
            node->token.number = ASSIGN_OP;
            //step 1: code gen for left-hand side
            if(lhs->noderep == nonterm){
                lvalue = 1;
                processOperator(lhs);
                lvalue = 0;
            }
            node->token.number = nodeNumber;
            //step 2: code gen for repeating part
            if(lhs->noderep == nonterm)
                processOperator(lhs);
            else rv_emit(lhs);
            //step 3: code gen for right-hand side
            if(rhs->noderep == nonterm)
                processOperator(rhs);
            else rv_emit(rhs);
            //step 4: emit the corresponding oper code
            switch(node->token.number){
                case ADD_ASSIGN : emit0(add); break;
                case SUB_ASSIGN : emit0(sub); break;
                case MUL_ASSIGN : emit0(mult); break;
                case DIV_ASSIGN : emit0(divop); break;
                case MOD_ASSIGN : emit0(modop); break;
                default: break;
            }
            //step 5: code gen for store code
            if(lhs->noderep == terminal){
                Symbol *symbol = lookup(lhs->token.value);
                if(symbol == NULL){
                    printf("undefined variable : %s\n", lhs->child->token.value);
                    return;
                }
                emit2(str, symbol->block, symbol->offset);
            }else
                emit0(sti);
            break;
        }
        case ADD: case SUB: case MUL: case DIV: case MOD:
        case EQ: case NE: case GT: case LT: case GE: case LE:
        case LOGICAL_AND: case LOGICAL_OR:
        {
            Node *lhs = node->child, *rhs = node->child->brother;
            //step 1:visit left operand
            if(lhs->noderep == nonterm) processOperator(lhs);
            else rv_emit(lhs);
            //step 2:visit right operand
            if(rhs->noderep == nonterm) processOperator(rhs);
            else rv_emit(rhs);
            //step 3:visit root
            switch(node->token.number){
                case ADD: emit0(add); break;
                case SUB: emit0(sub); break;
                case MUL: emit0(mult); break;
                case DIV: emit0(divop); break;
                case MOD: emit0(modop); break;
                case EQ: emit0(eq); break;
                case NE: emit0(ne); break;
                case GT: emit0(gt); break;
                case LT: emit0(lt); break;
                case GE: emit0(ge); break;
                case LE: emit0(le); break;
                case LOGICAL_AND: emit0(andop); break;
                case LOGICAL_OR: emit0(orop); break;
                default: break;
            }
            break;
        }
        case UNARY_MINUS: case LOGICAL_NOT:
        {
            Node *p = node->child;
            if(p->noderep == nonterm) processOperator(p);
            else rv_emit(p);
            switch(node->token.number){
                case UNARY_MINUS: emit0(neg); break;
                case LOGICAL_NOT: emit0(notop); break;
                default: break;
            }
        }
        case PRE_INC: case PRE_DEC: case POST_INC: case POST_DEC:
        {
            Node *p = node->child;
            Node *q;
            Symbol *s;
            if(p->noderep == nonterm) processOperator(p);
            else rv_emit(p);
            q=p;

            while(q->noderep != terminal) q = q->child;
            if(!q || (q->token.number != IDENT)){
                printf("increment/decrement operators can not be applied in expression\n");
                return;
            }
            s = lookup(q->token.value);
            if(s == NULL) return;
            switch(node->token.number){
                case PRE_INC: emit0(incop); break;
                case PRE_DEC: emit0(decop); break;
                case POST_INC: emit0(incop); break;
                case POST_DEC: emit0(decop); break;
                default: break;
            }
            if(p->noderep == terminal){
                s = lookup(p->token.value);
                if(s == NULL) return;
                emit2(str, s->block, s->offset);
            }else if(node->token.number == INDEX){
                lvalue = 1;
                processOperator(p);
                lvalue = 0;
                emit0(swp);
                emit0(sti);
            }
            else printf("error in increment/decrement operators\n");
            break;
        }
        case INDEX:
        {
            Node *indexExp = node->child->brother;
            Symbol *s;
            if(indexExp->noderep == nonterm) processOperator(indexExp);
            else rv_emit(indexExp);
            s = lookup(node->child->token.value);
            if(s == NULL){
                printf("undefined variable : %s\n", node->child->token.value);
                return;
            }
            if(s->typeSpecifier == REP_TYPE){
                emit2(lod, s->block, s->offset);
            }else if(s->typeSpecifier == INT_TYPE) {
                emit2(lda, s->block, s->offset);
            }
            emit0(add);
            if(!lvalue) emit0(ldi);
            break;
        }
        case CALL:
        {
            Node *p = node->child; //function name
            char *functionName;
            int noArguments;
            Symbol *s;
            if(checkPredefined(p)) break;
            //handle for user function
            functionName = p->token.value;
            s = lookup(functionName);
            if(s == NULL) break; //undefined function
            noArguments = s->size;
            emit0(ldp);
            p=p->brother->child; //ACTUAL_PARAM's child
            while(p){
                if(p->noderep == nonterm) processOperator(p);
                else rv_emit(p);
                noArguments--;
                p=p->brother;
            }
            if(noArguments){
                printf("%s : invalid actual arguments", functionName);
            }
            emitJump(call, node->child->token.value);
            break;
        }
        default:
            printf("error(OPERATOR) : not invalid operator");
            break;
    }
}
void genLabel(char *label) {
    static int lc = 0;
    memset(label, 0, sizeof(char)*LABEL_SIZE);
    sprintf(label, "$$%d", lc);
    lc++;
}
void rv_emit(Node *node) {
    if(node->token.number == NUMBER)
        emit1(ldc, atoi(node->token.value));
    else{
        Symbol *s = lookup(node->token.value);
        if(s == NULL) return;
        if(s->typeQualifier == CONST_TYPE)
            emit1(ldc, s->initValue);
        else if(s->size> 1)
            emit2(lda, s->block, s->offset);
        else
            emit2(lod, s->block, s->offset);
    }
}
int checkPredefined(Node *node) {
    char *function_name = node->token.value;
    Node* param = node->brother->child;
    if(!strcmp(function_name, "write")){
        if(param == NULL || param->noderep != terminal || param->brother != NULL){
            printf("error(write) : invalid param!\n");
            return 1;
        }
        Symbol *s = lookup(param->token.value);
        if(s == NULL){
            printf("error(write) : undefined symbol! %s\n", param->token.value);
            return 1;
        }
        emit0(ldp);
        emit2(lod, s->block, s->offset);
        emitJump(call, function_name);
        return 1;
    }
    if(!strcmp(function_name, "read")){
        if(param == NULL || param->noderep != terminal || param->brother != NULL){
            printf("error(write) : invalid param!\n");
            return 1;
        }
        Symbol *s = lookup(param->token.value);
        if(s == NULL){
            printf("error(read) : undefined symbol! %s\n", param->token.value);
            return 1;
        }
        emit0(ldp);
        emit2(lda, s->block, s->offset);
        emitJump(call, function_name);
        return 1;
    }
    if(!strcmp(function_name, "lf")){
        if(param != NULL){
            printf("error(lf) : invalid param!\n");
            return 1;
        }
        emitJump(call, function_name);
        return 1;
    }
    return 0;
}
void emitSymbols() {
    int size = getSymbolCount();
    for(int i=0; i<size; i++){
        struct symbolType *s = getSymbol(i);
        if(s->typeQualifier == VAR_TYPE) {
            emitSymbol(sym, s->block, s->offset, s->size, s->name);
        }
    }
}

/***Import Tree***/

Node* importTree(char* filename){
    FILE* fp = NULL;
    Node* stack[AST_STACK_SIZE] = {0, };
    int flag[AST_STACK_SIZE] = {0, };
    Node* root;
    int top = 0;
    fp = fopen(filename, "r");
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
        Node* current = readNode(fp);
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

Node *readNode(FILE *fp) {
    int number;
    char valueBuf[VALUE_BUF_SIZE];
    Node* node;
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
    node = (Node*) malloc(sizeof(Node));
    memset(node, 0, sizeof(Node));
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

void strsep(char* str, char sep, int count){
    int len = strlen(str);
    int cnt = 0;
    for(int i=len-1; i>=0; i--){
        if(*(str+i) == sep){
            //i is sep
            memset((str+i), 0, sizeof(char)*len-i);
            cnt++;
            if(cnt == count) {
                break;
            }
        }
    }
}