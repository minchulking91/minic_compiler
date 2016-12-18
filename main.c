#include <stdio.h>
#include <stdlib.h>
#include <mem.h>

#include "Node.h"
#include "Symbol.h"
#include "Ucode.h"

#define DEFAULT_READ_FILE "out.ast"
#define VALUE_BUF_SIZE 16
#define AST_STACK_SIZE 128
#define LABEL_SIZE 10

int lexical_level = 2;

//for import AST
Node* importTree(char*);
Node *readNode(FILE *fp);

//for printTree
void printNode(Node* node, int indent);
void printTree(Node* node, int indent);

void genCode(Node *node);

void processDeclaration(Node *node);

void processSimpleVariable(Node *node, enum TYPE_SPEC typeSpecifier, enum TYPE_QUALIFIER typeQualifier);

void processArrayVariable(Node *node, enum TYPE_SPEC typeSpecifier, enum TYPE_QUALIFIER typeQualifier);

void rv_emit(Node *node);

void processFuncHeader(Node *node);

void icg_error(int reason);

void emit1(enum U_CODE code, int param);

void emit0(enum U_CODE code);

void emitJump(enum U_CODE code, char label[5]);

void emit2(enum U_CODE code, int param1, int param2);

void processCondition(Node *node);

void genLabel(char *label);

void emitLabel(char *label);

void emit3(enum U_CODE code, int param1, int param2, int param3);

void processFunction(Node *node);

void emitSymbols(int block);

int main() {
    Node* root = importTree(NULL);
    genCode(root);
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
        else icg_error(3);
    }
    //emit symbols in block(1)
    emitSymbols(1);
    //step 2: process the function part
    for(p=node->child; p; p=p->brother){
        if(p->token.number == FUNC_DEF) processFunction(p);
    }

    //step 3: gen codes for staring routine
    emit1(bgn, blockOffset[0]-1);
    emit0(ldp);
    emitJump(call, "main");
    emit0(endop);
}

void emitSymbols(int block) {

}

void processFunction(Node *node) { //node is FUNC_DEF
    char* functionName;
    int stIndex;
    functionName = node->child->child->brother->token.value;

    stIndex = lookup(functionName);
    Symbol* symbol = symbolTable[stIndex];
    if(symbol->typeQualifier != FUNC_TYPE){
        //invalid symbol!
        return;
    }

    //function header
    int size; //param's size + local variable size
    setBlock();
    /**
     * size :
     *  1. FUNC_HEAD의 FORMAL_PARA를 해결 해야 한다.
     *  2. COMPOUND_ST의 DCL_LIST를 해결 해야 한다.
     */
    //get size from FORMAL_PARA
    Node* formal_para = node->child->child->brother->brother;
    if(formal_para->token.number != FORMAL_PARA){
        printf("error(FUNC_DEF) : token number is expected FORMAL_PARA(%d) but %d\n", FORMAL_PARA, node->token.number);
        return;
    }
    //get size from DCL_LIST
    emit3(proc, size, blockStack[blockTop], lexical_level);
    //function body 수행



    stackTop++;
    paramNum = tblStack[stackTop - 1]->st[stIndex].width;
    tblStack[stackTop] = tblStack[stackTop - 1]->st[stIndex].link;

    offsetStack[stackTop] = 1;

    for (p = ptr->son->son->brother->brother->son; p; p = p->brother) {
        if (p->token.tokenNumber == PARAM_DCL) {
            printf("[[%s]]\n", nodeName[p->son->son->token.tokenNumber]);
            processFuncDeclaration(p->son);
        }
    }

    // 선언부 처리
    for (p = ptr->son->brother->son->son; p; p = p->brother) {
        if (p->token.tokenNumber == DCL) {
            processDeclaration(p->son);
        } else {
            icg_error(3);
        }
    }

    // 함수 시작 코드 생성
    emitProc(functionName, offsetStack[stackTop] - 1 + noArgument, tblStack[stackTop]->base, 2);
    //emitProc(functionName, offsetStack[stackTop]-1, tblStack[stackTop]->base, 2);



    for (i = 0; i < noArgument; i++) {
        argNum++;
        emitSym("sym", tblStack[stackTop]->base,
                i + 1, 1);
    }


    for (i = 0; i < tblStack[stackTop]->num; i++) {
        argNum++;
        emitSym("sym", tblStack[stackTop]->base,
                tblStack[stackTop]->st[i].offset + noArgument, tblStack[stackTop]->st[i].width);
    }

    //buildUp = TRUE;



    for (i = 0; i < tblStack[stackTop]->num; i++) {

        if (tblStack[stackTop]->st[i].needInitialValue) {
            emit1("ldc", tblStack[stackTop]->st[i].initialValue);
            emit2("str", tblStack[stackTop]->base, tblStack[stackTop]->st[i].offset);
        }
    }

    /*
    for( i = paramNum; i < tblStack[stackTop]->num;i++)
    {
        emit1("ldc", tblStack[stackTop]->st[i].initialValue );
        emit2("str", tblStack[stackTop]->base, tblStack[stackTop]->st[i].offset);
    }*/

    // 소스코드내의 문장 처리
    for (p = ptr->son; p; p = p->brother) {
        //printf( "############# %d\n", p->token.tokenValue );
        if (p->token.tokenNumber == COMPOUND_ST) {
            processStatement(p);
        }
    }


    if (!flag_returned) {
        emit0("ret");
    }
    emit0("end");

    buildUp = FALSE;

    stackTop--;



}


void emitJump(enum U_CODE code, char* label) {
    printf("%s\t%s\n", ucodeName[code], label);
}

void emit0(enum U_CODE code) {
    printf("%s\n", ucodeName[code]);
}

void emit1(enum U_CODE code, int param) {
    printf("%s\t%d\n", ucodeName[code], param);
}
void emit2(enum U_CODE code, int param1, int param2) {
    printf("%s\t%d\t%d\n", ucodeName[code], param1, param2);
}

void emitLabel(char *label) {
    printf("%s\t%s\n", ucodeName[nop], label);
}

void emit3(enum U_CODE code, int param1, int param2, int param3) {
    printf("%s\t%d\t%d\t%d\n", ucodeName[code], param1, param2, param3);
}
void icg_error(int reason) {

}

void processFuncHeader(Node *node) {
    int noArguments;
    enum TYPE_SPEC returnType = VOID_TYPE;
    int stIndex;
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


void processDeclaration(Node *node) {
    Node *p, *q;
    enum TYPE_SPEC typeSpecifier;
    enum TYPE_QUALIFIER typeQualifier;

    if(node->token.number != DCL_SPEC) icg_error(4);
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
    if(p->token.number != DCL_ITEM) icg_error(5);
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



void processOperator(Node *node){
    int lvalue = 0;
    switch(node->token.number){
        case ASSIGN_OP:
        {
            int stIndex = -1;
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
                stIndex = lookup(lhs->token.value);
                if(stIndex == -1){
                    printf("error(OPERATOR) : undefined variable : %s\n", lhs->token.value);
                    return;
                }
                emit2(str, symbolTable[stIndex]->block, symbolTable[stIndex]->offset);
            } else
                emit0(sti);
            break;
        }
        case ADD_ASSIGN: case SUB_ASSIGN: case MUL_ASSIGN: case DIV_ASSIGN: case MOD_ASSIGN:
        {
            int stIndex = -1;
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
                stIndex = lookup(lhs->token.value);
                if(stIndex == -1){
                    printf("undefined variable : %s\n", lhs->child->token.value);
                    return;
                }
                emit2(str, symbolTable[stIndex]->block, symbolTable[stIndex]->offset);
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
            int stIndex;
            int amount = 1;
            if(p->noderep == nonterm) processOperator(p);
            else rv_emit(p);
            q=p;
            while(q->noderep != terminal) q = q->child;
            if(!q || (q->token.number != IDENT)){
                printf("increment/decrement operators can not be applied in expression\n");
                return;
            }
            stIndex = lookup(q->token.value);
            if(stIndex == -1) return;
            switch(node->token.number){
                case PRE_INC: emit0(incop); break;
                case PRE_DEC: emit0(decop); break;
                case POST_INC: emit0(incop); break;
                case POST_DEC: emit0(decop); break;
                default: break;
            }
            if(p->noderep == terminal){
                stIndex = lookup(p->token.value);
                if(stIndex == -1) return;;
                emit2(str, symbolTable[stIndex]->block, symbolTable[stIndex]->offset);
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
            int stIndex = -1;
            Node *indexExp = node->child->brother;
            if(indexExp->noderep == nonterm) processOperator(indexExp);
            else rv_emit(indexExp);
            stIndex = lookup(node->child->token.value);
            if(stIndex == -1){
                printf("undefined variable : %s\n", node->child->token.value);
                return;
            }
            emit2(lda, symbolTable[stIndex]->block, symbolTable[stIndex]->offset);
            emit0(add);
            if(!lvalue) emit0(ldi);
            break;
        }
        case CALL:
        {
            Node *p = node->child; //function name
            char *functionName;
            int stIndex, noArguments;
            //if(checkPredefined(p)) break;
            //handle for user function
            functionName = p->token.value;
            stIndex = lookup(functionName);
            if(stIndex == -1) break; //undefined function
            noArguments = symbolTable[stIndex]->size;
            emit0(ldp);
            p=p->brother; //ACTUAL_PARAM
            while(p){
                if(p->noderep == nonterm) processOperator(p);
                else rv_emit(p);
                noArguments--;
                p=p->brother;
            }
            if(!noArguments){
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


void rv_emit(Node *node) {
    int stIndex;
    if(node->token.number == NUMBER)
        emit1(ldc, atoi(node->token.value));
    else{
        stIndex = lookup(node->token.value);
        if(stIndex == -1) return;
        if(symbolTable[stIndex]->typeQualifier == CONST_TYPE)
            emit1(ldc, symbolTable[stIndex]->initValue);
        else if(symbolTable[stIndex]->size> 1)
            emit2(lda, symbolTable[stIndex]->block, symbolTable[stIndex]->offset);
        else
            emit2(lod, symbolTable[stIndex]->block, symbolTable[stIndex]->offset);
    }
}

void processStatement(Node *node){
    int returnWithValue = 0;
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
                returnWithValue = 1;
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



void genLabel(char *label) {

}

void processCondition(Node *node) {
    if(node->noderep == nonterm) processOperator(node);
    else rv_emit(node);
}

Node* importTree(char* filename){
    FILE* fp = NULL;
    Node* stack[AST_STACK_SIZE] = {0, };
    int flag[AST_STACK_SIZE] = {0, };
    Node* root;
    int top = 0;
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

/*print AST function*/
void printNode(Node* node, int indent){
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

void printTree(Node* node, int indent){
    Node* p = node;
    while(p!=NULL){
        printNode(p, indent);
        if(p->noderep == nonterm) printTree(p->child, indent+5);
        p = p->brother;
    }
}
