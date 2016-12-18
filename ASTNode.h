#ifndef NODE_HEADER
#define NODE_HEADER

enum astNodeNumber{
	ACTUAL_PARAM, 	ADD, 			ADD_ASSIGN, 	ARRAY_VAR, 
	ASSIGN_OP, 	CALL, 			COMPOUND_ST, 	CONST_NODE,
	DCL, 			DCL_ITEM, 		DCL_LIST, 		DCL_SPEC, 
	DIV, 			DIV_ASSIGN, 	EQ, 			ERROR_NODE, 
	EXP_ST, 		FORMAL_PARA, 	FUNC_DEF, 		FUNC_HEAD, 
	GE, 			GT, 			IDENT, 			IF_ELSE_ST, 
	IF_ST, 			INDEX, 			INT_NODE, 		LE, 
	LOGICAL_AND, 	LOGICAL_NOT, 	LOGICAL_OR,	LT,
	MOD, 			MOD_ASSIGN, 	MUL, 			MUL_ASSIGN, 
	NE, 			NUMBER, 		PARAM_DCL, 	POST_DEC,
	POST_INC, 		PRE_DEC, 		PRE_INC, 		PROGRAM, 
	RETURN_ST, 	SIMPLE_VAR, 	STAT_LIST, 	SUB,
	SUB_ASSIGN, 	UNARY_MINUS, 	VOID_NODE, 	WHILE_ST };

enum astNodeRep{terminal, nonterm};
typedef struct TokenType{
	int number;
	char* value;
} Token;

typedef struct ASTNodeType{
	Token token;
	struct ASTNodeType *child;
	struct ASTNodeType *brother;
	enum astNodeRep noderep;
} ASTNode;


#endif 