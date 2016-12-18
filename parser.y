%{
	#include "Node.h"
	#include <stdio.h>
	#include <stdlib.h>
	#include <mem.h>
	#include <stdarg.h>
	#define DEFAULT_OUT_FILE "out.ast"
	FILE* fp;
	Node* root;
	char* nodeName[] = {
    	"ACTUAL_PARAM", "ADD",			"ADD_ASSIGN",	"ARRAY_VAR",
    	"ASSIGN_OP", 	"CALL", 		"COMPOUND_ST", 	"CONST_NODE",
    	"DCL", 			"DCL_ITEM", 	"DCL_LIST", 	"DCL_SPEC",
    	"DIV", 			"DIV_ASSIGN", 	"EQ", 			"ERROR_NODE",
    	"EXP_ST", 		"FORMAL_PARA", 	"FUNC_DEF", 	"FUNC_HEAD",
    	"GE", 			"GT", 			"IDENT", 		"IF_ELSE_ST",
    	"IF_ST", 		"INDEX", 		"INT_NODE", 	"LE",
    	"LOGICAL_AND", 	"LOGICAL_NOT", 	"LOGICAL_OR",	"LT",
    	"MOD", 			"MOD_ASSIGN", 	"MUL", 			"MUL_ASSIGN",
    	"NE", 			"NUMBER", 		"PARAM_DCL", 	"POST_DEC",
    	"POST_INC", 	"PRE_DEC", 		"PRE_INC", 		"PROGRAM",
    	"RETURN_ST", 	"SIMPLE_VAR", 	"STAT_LIST", 	"SUB",
    	"SUB_ASSIGN", 	"UNARY_MINUS", 	"VOID_NODE", 	"WHILE_ST"};

    /***functions***/
    /*build AST functions*/
    Node* makeNode(int number, char* value);
    Node* makeTree(int nodeNumber, Node* first);
    Node* linkBrother(Node* node, ...);
    //Node* rebro(Node* node, int lv); hide this function
    Node* reverseBrother(Node* node);
    /*print AST function*/
    //void printNode(Node* node, int indent); hide this function
    void printTree(Node* node, int indent);
    /* export AST to FILE */
    //void exportNode(Node* node); hide this function
    void exportTree(Node* root, char* filename);
    int ismeanfulNode(Node* node);
%}
%union {
  char* string;
  Node* node;
}
%start mini_c
%token <string> tident tnumber 
%token <string> tconst telse tif tint treturn tvoid twhile
%token <string> taddAssign tsubAssign tmulAssign tdivAssign tmodAssign
%token <string> tor tand tequal tnotequ tgreate tlesse tinc tdec
%type <node> mini_c translation_unit external_dcl function_def function_header dcl_spec dcl_specifiers dcl_specifier type_qualifier type_specifier function_name formal_param opt_formal_param formal_param_list param_dcl compound_st opt_dcl_list declaration_list declaration init_dcl_list init_declarator declarator opt_stat_list opt_number statement_list statement expression expression_st opt_expression if_st while_st return_st assignment_exp logical_or_exp logical_and_exp equality_exp relational_exp additive_exp multiplicative_exp unary_exp postfix_exp opt_actual_param actual_param actual_param_list primary_exp
%%
mini_c 
	: translation_unit { 
		root = makeTree(PROGRAM, reverseBrother($1)); //reverse list
		$$ = root;
	};
translation_unit 
	: external_dcl { }
	| translation_unit external_dcl { 
		//List
		$$ = linkBrother($1, $2, NULL);
	};
external_dcl
	: function_def { }
	| declaration { };
function_def	
	: function_header compound_st { 
		$$ = makeTree(FUNC_DEF, linkBrother($2, $1, NULL)); 
		//link brother에서 거꾸로 연결하기 때문에 역순으로 넣어줌. 
	};
function_header		
	: dcl_spec function_name formal_param {
		$$ = makeTree(FUNC_HEAD, linkBrother($3, $2, $1, NULL)); 
	};
dcl_spec
	: dcl_specifiers {
		$$ = makeTree(DCL_SPEC, reverseBrother($1)); //reverse list
	};
dcl_specifiers
	: dcl_specifier { }
	| dcl_specifiers dcl_specifier { 
		//List
		$$ = linkBrother($1, $2, NULL);
	};
dcl_specifier
	: type_qualifier {}
	| type_specifier {};
type_qualifier
	: tconst { 
		$$ = makeTree(CONST_NODE, NULL);
	};
type_specifier
	: tint { 
		$$ = makeTree(INT_NODE, NULL); 
	}
	| tvoid { 
		$$ = makeTree(VOID_NODE, NULL);
	};
function_name
	: tident { 
		$$ = makeNode(IDENT, $1);
	};
formal_param
	: '(' opt_formal_param ')' { 
		$$ = makeTree(FORMAL_PARA, $2); //already reverse
	};
opt_formal_param	
	: formal_param_list { 
		$$ = reverseBrother($1); 
		//reverse list 
	}
	| { $$ = NULL;};
formal_param_list	
	: param_dcl { }
	| formal_param_list ',' param_dcl { 
		//list
		$$ = linkBrother($1, $3, NULL);
	};
param_dcl
	: dcl_spec declarator { 
		$$ = makeTree(PARAM_DCL, linkBrother($2, $1, NULL));
	};
compound_st
	: '{' opt_dcl_list opt_stat_list '}' { 
		$$ = makeTree(COMPOUND_ST, linkBrother($3, $2, NULL));
	};
opt_dcl_list
	: declaration_list { 
		$$ = makeTree(DCL_LIST, reverseBrother($1));
	}
	| { 
		$$ = makeTree(DCL_LIST, NULL);
	};
declaration_list
	: declaration {
	}
	| declaration_list declaration { 
		$$ = linkBrother($1, $2, NULL);
	};
declaration
	: dcl_spec init_dcl_list ';' { 
		$$ = makeTree(DCL, linkBrother($2, $1, NULL));
	};
init_dcl_list
	: init_declarator {
	}
	| init_dcl_list ',' init_declarator { 
		$$ = linkBrother($1, $3, NULL);
	};
init_declarator
	: declarator { 
		$$ = makeTree(DCL_ITEM, $1);
	}
	| declarator '=' tnumber { 
		Node* number = makeNode(NUMBER, $3);
		$$ = makeTree(DCL_ITEM, linkBrother(number, $1));
	};
declarator
	: tident { 
		Node* ident = makeNode(IDENT, $1);
		$$ = makeTree(SIMPLE_VAR, ident);
	}
	| tident '[' opt_number ']' { 
		Node* ident = makeNode(NUMBER, $1);
		$$ = makeTree(ARRAY_VAR, linkBrother($3, ident));
	};
opt_number
	: tnumber { 
		$$ = makeNode(NUMBER, $1);
	}
	| { $$ = NULL;};
opt_stat_list
	: statement_list { 
		$$ = makeTree(STAT_LIST, reverseBrother($1));
	}
	| { $$ = NULL; };
statement_list
	: statement {  }
	| statement_list statement { 
		$$ = linkBrother($1, $2, NULL);
	};
statement
	: compound_st {  }
	| expression_st {  }
	| if_st {  }
	| while_st {  }
	| return_st {  };
expression_st
	: opt_expression ';' { 
		$$ = makeTree(EXP_ST, $1);
	};
opt_expression
	: expression {  }
	| { $$ = NULL };
if_st
	: tif '(' expression ')' statement { 
		$$ = makeTree(IF_ST, linkBrother($5, $3, NULL));
	}
	| tif '(' expression ')' statement telse statement { 
		$$ = makeTree(IF_ELSE_ST, linkBrother($7, $5, $3, NULL));
	};
while_st
	: twhile '(' expression ')' statement { 
		$$ = makeTree(WHILE_ST, linkBrother($5, $3));
	};
return_st
	: treturn opt_expression ';' { 
		$$ = makeTree(RETURN_ST, $2);
	};
expression
	: assignment_exp{ };
assignment_exp
	: logical_or_exp{ }
	| unary_exp '=' assignment_exp { 
		$$ = makeTree(ASSIGN_OP, linkBrother($3, $1, NULL));
	}
	| unary_exp taddAssign assignment_exp { 
		$$ = makeTree(ADD_ASSIGN, linkBrother($3, $1, NULL));
	}
	| unary_exp tsubAssign assignment_exp { 
		$$ = makeTree(SUB_ASSIGN, linkBrother($3, $1, NULL));
	}
	| unary_exp tmulAssign assignment_exp { 
		$$ = makeTree(MUL_ASSIGN, linkBrother($3, $1, NULL));
	}
	| unary_exp tdivAssign assignment_exp {
		$$ = makeTree(DIV_ASSIGN, linkBrother($3, $1, NULL));
	}
	| unary_exp tmodAssign assignment_exp { 
		$$ = makeTree(MOD_ASSIGN, linkBrother($3, $1, NULL));
	};
logical_or_exp
	: logical_and_exp { }
	| logical_or_exp tor logical_and_exp { 
		$$ = makeTree(LOGICAL_OR, linkBrother($3, $1, NULL));
	};
logical_and_exp		
	: equality_exp { }
	| logical_and_exp tand equality_exp { 
		$$ = makeTree(LOGICAL_AND, linkBrother($3, $1, NULL));
	};
equality_exp
	: relational_exp { }
	| equality_exp tequal relational_exp { 
		$$ = makeTree(EQ, linkBrother($3, $1, NULL));
	}
	| equality_exp tnotequ relational_exp { 
		$$ = makeTree(NE, linkBrother($3, $1, NULL));
	};
relational_exp		
	: additive_exp { }
	| relational_exp '>' additive_exp { 
		$$ = makeTree(GT, linkBrother($3, $1, NULL));
	}
	| relational_exp '<' additive_exp { 
		$$ = makeTree(LT, linkBrother($3, $1, NULL));
	}
	| relational_exp tgreate additive_exp { 
		$$ = makeTree(GE, linkBrother($3, $1, NULL));
	}
	| relational_exp tlesse additive_exp { 
		$$ = makeTree(LE, linkBrother($3, $1, NULL));
	};
additive_exp
	: multiplicative_exp { }
	| additive_exp '+' multiplicative_exp { 
		$$ = makeTree(ADD, linkBrother($3, $1, NULL));
	}
	| additive_exp '-' multiplicative_exp { 
		$$ = makeTree(SUB, linkBrother($3, $1, NULL));
	};
multiplicative_exp	
	: unary_exp { }
	| multiplicative_exp '*' unary_exp { 
		$$ = makeTree(MUL, linkBrother($3, $1, NULL));
	}
	| multiplicative_exp '/' unary_exp { 
		$$ = makeTree(DIV, linkBrother($3, $1, NULL));
	}
	| multiplicative_exp '%' unary_exp { 
		$$ = makeTree(MOD, linkBrother($3, $1, NULL));
	};
unary_exp
	: postfix_exp { }
	| '-' unary_exp { 
		$$ = makeTree(UNARY_MINUS, $2);
	}
	| '!' unary_exp { 
		$$ = makeTree(LOGICAL_NOT, $2);
	}
	| tinc unary_exp { 
		$$ = makeTree(PRE_INC, $2);
	}
	| tdec unary_exp { 
		$$ = makeTree(PRE_DEC, $2);
	};
postfix_exp	
	: primary_exp { }
	| postfix_exp '[' expression ']' { 
		$$ = makeTree(INDEX, linkBrother($3, $1, NULL));
	}
	| postfix_exp '(' opt_actual_param ')' { 
		$$ = makeTree(CALL, linkBrother($3, $1, NULL));
	}
	| postfix_exp tinc { 
		$$ = makeTree(POST_INC, $1);
	}
	| postfix_exp tdec { 
		$$ = makeTree(POST_DEC, $1);
	};
opt_actual_param	
	: actual_param { }
	| { $$ = NULL };
actual_param 
	: actual_param_list { 
		$$ = makeTree(ACTUAL_PARAM, reverseBrother($1));
	};
actual_param_list	
	: assignment_exp { }
	| actual_param_list ',' assignment_exp	{ 
		$$ = linkBrother($1, $3, NULL);
	};
primary_exp			
	: tident { 
		$$ = makeNode(IDENT, $1);
	}
	| tnumber { 
		$$ = makeNode(NUMBER, $1);
	}
	| '(' expression ')' { 
		$$ = $2;
	};

%%
void yyerror(char *s){
  printf("%s\n", s);
}

void main(int argc, char *argv[]){
  yyparse();
  printTree(root, 0);
  exportTree(root, NULL);
}

/*AST build function*/
Node* makeNode(int number, char* value){
	Node *node;
	Token token;
	token.number = number;
	token.value = value;

	node = (Node*) malloc(sizeof(Node));
	if(!node){
	    yyerror("malloc error in makeNode()");
		exit(1);
	}
	memset(node, 0, sizeof(Node));
	node->token = token;
	node->noderep = (number == NUMBER || number == IDENT) ? terminal : nonterm;
	return node;
}
Node* makeTree(int nodeNumber, Node* first){
	Node *node;
	node = makeNode(nodeNumber, NULL);
	node->child = first;
	return node;
}
Node* linkBrother(Node* node, ...){
	Node* temp = node;
	Node* arg = NULL;
	va_list ap;
	va_start(ap, node);
	for(;;){
		arg = va_arg(ap, Node*);
		if(arg == NULL){
			break;
		}
		arg->brother = temp;
		temp = arg;
	}
	va_end(ap);
	return temp;
}

/**
 * find end-brother and reverse node-list with stack
 */
Node* rebro(Node* node, int lv){
	static Node* first;
	first = NULL;
	if(node == NULL){
		return NULL;
	}
	if(node->brother != NULL){
		Node* before = rebro(node->brother, lv+1);
		before->brother = node;
		node->brother = NULL;
		if(lv == 0){
			return first;
		}
		return node;
	}else{
		//first brother
		first = node;
		return node;
	}

}

Node* reverseBrother(Node* node){
	return rebro(node, 0);
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
		printf(" Nonterminal: %s", nodeName[node->token.number]);
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

/* export AST to FILE */
void exportNode(Node* node){
    if(node == NULL){
        fprintf(fp, "-1 ");
        return;
    }
    if(ismeanfulNode(node)){
        fprintf(fp, "-%d %s ", node->token.number, node->token.value);
    }
    else{
        fprintf(fp, "%d ", node->token.number);
    }

    exportNode(node->child);
    exportNode(node->brother);
}

void exportTree(Node* root, char* filename){
    fp = NULL;
    fp = fopen(filename == NULL ? DEFAULT_OUT_FILE : filename, "w");
    if(fp == NULL){
        //error
        yyerror("invalid file to open");
        exit(1);
    }
    exportNode(root);
    fclose(fp);
}

int ismeanfulNode(Node* node){
    if(node->token.number == IDENT || node->token.number == NUMBER){
        return 1;
    }
    return 0;
}