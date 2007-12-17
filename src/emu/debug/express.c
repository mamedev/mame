/***************************************************************************

    express.c
    Generic expressions engine.
    Written by Aaron Giles
    Copyright (c) 2005, Aaron Giles

***************************************************************************/

#include "express.h"
#include <ctype.h>



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define DEBUG_TOKENS			0



/***************************************************************************
    CONSTANTS
***************************************************************************/

#ifndef DEFAULT_BASE
#define DEFAULT_BASE			16			/* hex unless otherwise specified */
#endif

#define MAX_TOKENS				128
#define MAX_STACK_DEPTH			128
#define MAX_STRING_LENGTH		256
#define MAX_EXPRESSION_STRINGS	64
#define MAX_SYMBOL_LENGTH		64

#define SYM_TABLE_HASH_SIZE		97

/* token.type values */
enum
{
	TOK_INVALID = 0,
	TOK_END,
	TOK_NUMBER,
	TOK_STRING,
	TOK_MEMORY,
	TOK_SYMBOL,
	TOK_OPERATOR
};


/* token.info values */
enum
{
	TIN_PRECEDENCE_MASK		= 0x001f,
	TIN_PRECEDENCE_0		= 0x0000,		/* ( ) */
	TIN_PRECEDENCE_1		= 0x0001,		/* ++ (postfix), -- (postfix) */
	TIN_PRECEDENCE_2		= 0x0002,		/* ++ (prefix), -- (prefix), ~, !, - (unary), + (unary), b@, w@, d@, q@ */
	TIN_PRECEDENCE_3		= 0x0003,		/* *, /, % */
	TIN_PRECEDENCE_4		= 0x0004,		/* + - */
	TIN_PRECEDENCE_5		= 0x0005,		/* << >> */
	TIN_PRECEDENCE_6		= 0x0006,		/* < <= > >= */
	TIN_PRECEDENCE_7		= 0x0007,		/* == != */
	TIN_PRECEDENCE_8		= 0x0008,		/* & */
	TIN_PRECEDENCE_9		= 0x0009,		/* ^ */
	TIN_PRECEDENCE_10		= 0x000a,		/* | */
	TIN_PRECEDENCE_11		= 0x000b,		/* && */
	TIN_PRECEDENCE_12		= 0x000c,		/* || */
	TIN_PRECEDENCE_13		= 0x000d,		/* = *= /= %= += -= <<= >>= &= |= ^= */
	TIN_PRECEDENCE_14		= 0x000e,		/* , */
	TIN_PRECEDENCE_15		= 0x000f,		/* func() */

	TIN_RIGHT_TO_LEFT		= 0x0040,
	TIN_FUNCTION			= 0x0080,

	TIN_MEMORY_SIZE_SHIFT	= 8,
	TIN_MEMORY_SIZE_MASK	= (3 << TIN_MEMORY_SIZE_SHIFT),
	TIN_MEMORY_BYTE			= (0 << TIN_MEMORY_SIZE_SHIFT),
	TIN_MEMORY_WORD			= (1 << TIN_MEMORY_SIZE_SHIFT),
	TIN_MEMORY_DWORD		= (2 << TIN_MEMORY_SIZE_SHIFT),
	TIN_MEMORY_QWORD		= (3 << TIN_MEMORY_SIZE_SHIFT),

	TIN_MEMORY_SPACE_SHIFT	= 10,
	TIN_MEMORY_SPACE_MASK	= (3 << TIN_MEMORY_SPACE_SHIFT),
	TIN_MEMORY_PROGRAM		= (EXPSPACE_PROGRAM << TIN_MEMORY_SPACE_SHIFT),
	TIN_MEMORY_DATA			= (EXPSPACE_DATA    << TIN_MEMORY_SPACE_SHIFT),
	TIN_MEMORY_IO			= (EXPSPACE_IO      << TIN_MEMORY_SPACE_SHIFT),
};


/* token.value values if token.type == TOK_OPERATOR */
enum
{
	TVL_LPAREN,
	TVL_RPAREN,
	TVL_PLUSPLUS,
	TVL_MINUSMINUS,
	TVL_PREINCREMENT,
	TVL_PREDECREMENT,
	TVL_POSTINCREMENT,
	TVL_POSTDECREMENT,
	TVL_COMPLEMENT,
	TVL_NOT,
	TVL_UPLUS,
	TVL_UMINUS,
	TVL_MULTIPLY,
	TVL_DIVIDE,
	TVL_MODULO,
	TVL_ADD,
	TVL_SUBTRACT,
	TVL_LSHIFT,
	TVL_RSHIFT,
	TVL_LESS,
	TVL_LESSOREQUAL,
	TVL_GREATER,
	TVL_GREATEROREQUAL,
	TVL_EQUAL,
	TVL_NOTEQUAL,
	TVL_BAND,
	TVL_BXOR,
	TVL_BOR,
	TVL_LAND,
	TVL_LOR,
	TVL_ASSIGN,
	TVL_ASSIGNMULTIPLY,
	TVL_ASSIGNDIVIDE,
	TVL_ASSIGNMODULO,
	TVL_ASSIGNADD,
	TVL_ASSIGNSUBTRACT,
	TVL_ASSIGNLSHIFT,
	TVL_ASSIGNRSHIFT,
	TVL_ASSIGNBAND,
	TVL_ASSIGNBXOR,
	TVL_ASSIGNBOR,
	TVL_COMMA,
	TVL_MEMORYAT,
	TVL_EXECUTEFUNC
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef union _int_ptr int_ptr;
union _int_ptr
{
	void *			p;				/* pointer value */
	UINT64			i;				/* integer value */
};


typedef struct _parse_token parse_token;
struct _parse_token
{
	UINT16			type;			/* type of token */
	UINT16			info;			/* info for token */
	UINT32			offset;			/* offset within the string */
	int_ptr			value;			/* value of token */
};


typedef struct _internal_symbol_entry internal_symbol_entry;
struct _internal_symbol_entry
{
	internal_symbol_entry *next;	/* pointer to the next entry */
	const char *	name;			/* name of the symbol */
	symbol_entry	entry;		/* actual entry data */
};


/* typedef struct _symbol_table symbol_table -- defined in express.h */
struct _symbol_table
{
	symbol_table *parent;	/* pointer to the parent symbol table */
	internal_symbol_entry *hash[SYM_TABLE_HASH_SIZE]; /* hash table */
};


/* typedef struct _parsed_expression parsed_expression -- defined in express.h */
struct _parsed_expression
{
	const symbol_table *table; /* symbol table */
	char *			original_string;/* original string (prior to parsing) */
	char *			string[MAX_EXPRESSION_STRINGS]; /* string table */
	parse_token		token[MAX_TOKENS];/* array of tokens */
	int				token_stack_ptr;/* stack poointer */
	parse_token		token_stack[MAX_STACK_DEPTH];/* token stack */
};



/***************************************************************************
    STACK MANIPULATION
***************************************************************************/

/*-------------------------------------------------
    init_token_stack - reset the token stack
-------------------------------------------------*/

INLINE void init_token_stack(parsed_expression *expr)
{
	expr->token_stack_ptr = 0;
}


/*-------------------------------------------------
    push_token - push a token onto the stack
-------------------------------------------------*/

INLINE EXPRERR push_token(parsed_expression *expr, parse_token *token)
{
	/* check for overflow */
	if (expr->token_stack_ptr >= MAX_STACK_DEPTH)
		return MAKE_EXPRERR_STACK_OVERFLOW(token->offset);

	/* push */
	expr->token_stack[expr->token_stack_ptr++] = *token;
	return EXPRERR_NONE;
}


/*-------------------------------------------------
    pop_token - pop a token off the stack
-------------------------------------------------*/

INLINE EXPRERR pop_token(parsed_expression *expr, parse_token *token)
{
	/* check for underflow */
	if (expr->token_stack_ptr == 0)
		return MAKE_EXPRERR_STACK_UNDERFLOW(token->offset);

	/* push */
	*token = expr->token_stack[--expr->token_stack_ptr];
	return EXPRERR_NONE;
}


/*-------------------------------------------------
    peek_token - look at a token some number of
    entries up the stack
-------------------------------------------------*/

INLINE parse_token *peek_token(parsed_expression *expr, int count)
{
	if (expr->token_stack_ptr <= count)
		return NULL;
	return &expr->token_stack[expr->token_stack_ptr - count - 1];
}



/***************************************************************************
    LVAL/RVAL EVALUATION
***************************************************************************/

/*-------------------------------------------------
    pop_token_lval - pop a token off the stack
    and ensure that it is a proper lval
-------------------------------------------------*/

INLINE EXPRERR pop_token_lval(parsed_expression *expr, parse_token *token, const symbol_table *table)
{
	/* check for underflow */
	if (expr->token_stack_ptr == 0)
		return MAKE_EXPRERR_STACK_UNDERFLOW(token->offset);

	/* pop */
	*token = expr->token_stack[--expr->token_stack_ptr];

	/* to be an lval, the token must be a valid read/write symbol or a memory token */
	if (token->type == TOK_SYMBOL)
	{
		symbol_entry *symbol = token->value.p;
		if (symbol == NULL || symbol->type != SMT_REGISTER || symbol->info.reg.setter == NULL)
			return MAKE_EXPRERR_NOT_LVAL(token->offset);
	}
	else if (token->type != TOK_MEMORY)
		return MAKE_EXPRERR_NOT_LVAL(token->offset);

	return 0;
}


/*-------------------------------------------------
    pop_token_rval - pop a token off the stack
    and ensure that it is a proper rval
-------------------------------------------------*/

INLINE EXPRERR pop_token_rval(parsed_expression *expr, parse_token *token, const symbol_table *table)
{
	/* check for underflow */
	if (expr->token_stack_ptr == 0)
		return MAKE_EXPRERR_STACK_UNDERFLOW(token->offset);

	/* pop */
	*token = expr->token_stack[--expr->token_stack_ptr];

	/* symbol tokens get resolved down to number tokens */
	if (token->type == TOK_SYMBOL)
	{
		symbol_entry *symbol = token->value.p;
		if (symbol == NULL || (symbol->type != SMT_REGISTER && symbol->type != SMT_VALUE))
			return MAKE_EXPRERR_NOT_RVAL(token->offset);
		token->type = TOK_NUMBER;
		if (symbol->type == SMT_REGISTER)
			token->value.i = (*symbol->info.reg.getter)(symbol->ref);
		else
			token->value.i = symbol->info.gen.value;
	}

	/* memory tokens get resolved down to number tokens */
	else if (token->type == TOK_MEMORY)
	{
		int space = (token->info & TIN_MEMORY_SPACE_MASK) >> TIN_MEMORY_SPACE_SHIFT;
		int size = (token->info & TIN_MEMORY_SIZE_MASK) >> TIN_MEMORY_SIZE_SHIFT;
		token->type = TOK_NUMBER;
		token->value.i = external_read_memory(space, token->value.i, 1 << size);
	}

	/* to be an rval, the token must be a number */
	if (token->type != TOK_NUMBER)
		return MAKE_EXPRERR_NOT_RVAL(token->offset);
	return 0;
}


/*-------------------------------------------------
    get_lval_value - call the getter function
    for a SYMBOL token
-------------------------------------------------*/

INLINE UINT64 get_lval_value(parse_token *token, const symbol_table *table)
{
	if (token->type == TOK_SYMBOL)
	{
		symbol_entry *symbol = token->value.p;
		if (symbol != NULL && symbol->type == SMT_REGISTER)
			return (*symbol->info.reg.getter)(symbol->ref);
	}
	else if (token->type == TOK_MEMORY)
	{
		int space = (token->info & TIN_MEMORY_SPACE_MASK) >> TIN_MEMORY_SPACE_SHIFT;
		int size = (token->info & TIN_MEMORY_SIZE_MASK) >> TIN_MEMORY_SIZE_SHIFT;
		return external_read_memory(space, token->value.i, 1 << size);
	}
	return 0;
}


/*-------------------------------------------------
    set_lval_value - call the setter function
    for a SYMBOL token
-------------------------------------------------*/

INLINE void set_lval_value(parse_token *token, const symbol_table *table, UINT64 value)
{
	if (token->type == TOK_SYMBOL)
	{
		symbol_entry *symbol = token->value.p;
		if (symbol != NULL && symbol->type == SMT_REGISTER && symbol->info.reg.setter)
			(*symbol->info.reg.setter)(symbol->ref, value);
	}
	else if (token->type == TOK_MEMORY)
	{
		int space = (token->info & TIN_MEMORY_SPACE_MASK) >> TIN_MEMORY_SPACE_SHIFT;
		int size = (token->info & TIN_MEMORY_SIZE_MASK) >> TIN_MEMORY_SIZE_SHIFT;
		external_write_memory(space, token->value.i, 1 << size, value);
	}
}



/***************************************************************************
    DEBUGGING
***************************************************************************/

/*-------------------------------------------------
    print_token - debugging took to print a
    human readable token representation
-------------------------------------------------*/

static void print_tokens(FILE *out, parsed_expression *expr)
{
#if DEBUG_TOKENS
	parse_token *token = expr->token;

	mame_printf_debug("----\n");
	while (token->type != TOK_END)
	{
		switch (token->type)
		{
			default:
			case TOK_INVALID:
				fprintf(out, "INVALID\n");
				break;

			case TOK_END:
				fprintf(out, "END\n");
				break;

			case TOK_NUMBER:
				fprintf(out, "NUMBER: %08X%08X\n", (UINT32)(token->value.i >> 32), (UINT32)token->value.i);
				break;

			case TOK_STRING:
				fprintf(out, "STRING: ""%s""\n", (char *)token->value.p);
				break;

			case TOK_SYMBOL:
				fprintf(out, "SYMBOL: %08X%08X\n", (UINT32)(token->value.i >> 32), (UINT32)token->value.i);
				break;

			case TOK_OPERATOR:
				switch (token->value.i)
				{
					case TVL_LPAREN:		fprintf(out, "(\n");					break;
					case TVL_RPAREN:		fprintf(out, ")\n");					break;
					case TVL_PLUSPLUS:		fprintf(out, "++ (unspecified)\n");		break;
					case TVL_MINUSMINUS:	fprintf(out, "-- (unspecified)\n");		break;
					case TVL_PREINCREMENT:	fprintf(out, "++ (prefix)\n");			break;
					case TVL_PREDECREMENT:	fprintf(out, "-- (prefix)\n");			break;
					case TVL_POSTINCREMENT:	fprintf(out, "++ (postfix)\n");			break;
					case TVL_POSTDECREMENT:	fprintf(out, "-- (postfix)\n");			break;
					case TVL_COMPLEMENT:	fprintf(out, "!\n");					break;
					case TVL_NOT:			fprintf(out, "~\n");					break;
					case TVL_UPLUS:			fprintf(out, "+ (unary)\n");			break;
					case TVL_UMINUS:		fprintf(out, "- (unary)\n");			break;
					case TVL_MULTIPLY:		fprintf(out, "*\n");					break;
					case TVL_DIVIDE:		fprintf(out, "/\n");					break;
					case TVL_MODULO:		fprintf(out, "%%\n");					break;
					case TVL_ADD:			fprintf(out, "+\n");					break;
					case TVL_SUBTRACT:		fprintf(out, "-\n");					break;
					case TVL_LSHIFT:		fprintf(out, "<<\n");					break;
					case TVL_RSHIFT:		fprintf(out, ">>\n");					break;
					case TVL_LESS:			fprintf(out, "<\n");					break;
					case TVL_LESSOREQUAL:	fprintf(out, "<=\n");					break;
					case TVL_GREATER:		fprintf(out, ">\n");					break;
					case TVL_GREATEROREQUAL:fprintf(out, ">=\n");					break;
					case TVL_EQUAL:			fprintf(out, "==\n");					break;
					case TVL_NOTEQUAL:		fprintf(out, "!=\n");					break;
					case TVL_BAND:			fprintf(out, "&\n");					break;
					case TVL_BXOR:			fprintf(out, "^\n");					break;
					case TVL_BOR:			fprintf(out, "|\n");					break;
					case TVL_LAND:			fprintf(out, "&&\n");					break;
					case TVL_LOR:			fprintf(out, "||\n");					break;
					case TVL_ASSIGN:		fprintf(out, "=\n");					break;
					case TVL_ASSIGNMULTIPLY:fprintf(out, "*=\n");					break;
					case TVL_ASSIGNDIVIDE:	fprintf(out, "/=\n");					break;
					case TVL_ASSIGNMODULO:	fprintf(out, "%%=\n");					break;
					case TVL_ASSIGNADD:		fprintf(out, "+=\n");					break;
					case TVL_ASSIGNSUBTRACT:fprintf(out, "-=\n");					break;
					case TVL_ASSIGNLSHIFT:	fprintf(out, "<<=\n");					break;
					case TVL_ASSIGNRSHIFT:	fprintf(out, ">>=\n");					break;
					case TVL_ASSIGNBAND:	fprintf(out, "&=\n");					break;
					case TVL_ASSIGNBXOR:	fprintf(out, "^=\n");					break;
					case TVL_ASSIGNBOR:		fprintf(out, "|=\n");					break;
					case TVL_COMMA:			fprintf(out, ",\n");					break;
					case TVL_MEMORYAT:		fprintf(out, "mem@\n");					break;
					case TVL_EXECUTEFUNC:	fprintf(out, "execute\n");				break;
					default:				fprintf(out, "INVALID OPERATOR\n");		break;
				}
				break;
		}
		token++;
	}
	mame_printf_debug("----\n");
#endif
}



/***************************************************************************
    TOKENIZING
***************************************************************************/

/*-------------------------------------------------
    parse_memory_operator - parse the several
    forms of memory operators
-------------------------------------------------*/

static int parse_memory_operator(const char *buffer, UINT16 *flags)
{
	int length = (int)strlen(buffer);
	int space = 'p', size;

	*flags = 0;

	/* length 2 means space, then size */
	if (length == 2)
	{
		space = buffer[0];
		size = buffer[1];
	}

	/* length 1 means size */
	else if (length == 1)
		size = buffer[0];

	/* anything else is invalid */
	else
		return 0;

	/* convert the space to flags */
	switch (space)
	{
		case 'p':	*flags |= TIN_MEMORY_PROGRAM;	break;
		case 'd':	*flags |= TIN_MEMORY_DATA;		break;
		case 'i':	*flags |= TIN_MEMORY_IO;		break;
		default:	return 0;
	}

	/* convert the size to flags */
	switch (size)
	{
		case 'b':	*flags |= TIN_MEMORY_BYTE;		break;
		case 'w':	*flags |= TIN_MEMORY_WORD;		break;
		case 'd':	*flags |= TIN_MEMORY_DWORD;		break;
		case 'q':	*flags |= TIN_MEMORY_QWORD;		break;
		default:	return 0;
	}
	return 1;
}


/*-------------------------------------------------
    parse_string_into_tokens - take an expression
    string and break it into a sequence of tokens
-------------------------------------------------*/

static EXPRERR parse_string_into_tokens(const char *stringstart, parsed_expression *expr, const symbol_table *table)
{
	#define SET_TOKEN_INFO(_length, _type, _value, _info) \
	do { \
		token->type = _type; \
		token->value.i = _value; \
		token->info = _info; \
		string += _length; \
	} while (0)

	parse_token *token = expr->token;
	const char *string = stringstart;

	/* zap expression object */
	memset(expr, 0, sizeof(*expr));

	/* stash the symbol table pointer */
	expr->table = table;

	/* make a copy of the original string */
	expr->original_string = malloc(strlen(stringstart) + 1);
	if (!expr->original_string)
		return MAKE_EXPRERR_OUT_OF_MEMORY(0);
	strcpy(expr->original_string, stringstart);

	/* loop until done */
	while (string[0] != 0 && token - expr->token < MAX_TOKENS)
	{
		/* ignore any whitespace */
		while (string[0] != 0 && isspace(string[0]))
			string++;
		if (string[0] == 0)
			break;

		/* initialize the current token object */
		SET_TOKEN_INFO(0, TOK_INVALID, 0, 0);
		token->offset = string - stringstart;

		/* switch off the first character */
		switch (tolower(string[0]))
		{
			case '(':
				SET_TOKEN_INFO(1, TOK_OPERATOR, TVL_LPAREN, TIN_PRECEDENCE_0);
				break;

			case ')':
				SET_TOKEN_INFO(1, TOK_OPERATOR, TVL_RPAREN, TIN_PRECEDENCE_0);
				break;

			case '~':
				SET_TOKEN_INFO(1, TOK_OPERATOR, TVL_NOT, TIN_PRECEDENCE_2);
				break;

			case ',':
				SET_TOKEN_INFO(1, TOK_OPERATOR, TVL_COMMA, TIN_PRECEDENCE_14);
				break;

			case '+':
				if (string[1] == '+')
					SET_TOKEN_INFO(2, TOK_OPERATOR, TVL_PLUSPLUS, TIN_PRECEDENCE_1);
				else if (string[1] == '=')
					SET_TOKEN_INFO(2, TOK_OPERATOR, TVL_ASSIGNADD, TIN_PRECEDENCE_13 | TIN_RIGHT_TO_LEFT);
				else
					SET_TOKEN_INFO(1, TOK_OPERATOR, TVL_ADD, TIN_PRECEDENCE_4);
				break;

			case '-':
				if (string[1] == '-')
					SET_TOKEN_INFO(2, TOK_OPERATOR, TVL_MINUSMINUS, TIN_PRECEDENCE_1);
				else if (string[1] == '=')
					SET_TOKEN_INFO(2, TOK_OPERATOR, TVL_ASSIGNSUBTRACT, TIN_PRECEDENCE_13 | TIN_RIGHT_TO_LEFT);
				else
					SET_TOKEN_INFO(1, TOK_OPERATOR, TVL_SUBTRACT, TIN_PRECEDENCE_4);
				break;

			case '*':
				if (string[1] == '=')
					SET_TOKEN_INFO(2, TOK_OPERATOR, TVL_ASSIGNMULTIPLY, TIN_PRECEDENCE_13 | TIN_RIGHT_TO_LEFT);
				else
					SET_TOKEN_INFO(1, TOK_OPERATOR, TVL_MULTIPLY, TIN_PRECEDENCE_3);
				break;

			case '/':
				if (string[1] == '=')
					SET_TOKEN_INFO(2, TOK_OPERATOR, TVL_ASSIGNDIVIDE, TIN_PRECEDENCE_13 | TIN_RIGHT_TO_LEFT);
				else
					SET_TOKEN_INFO(1, TOK_OPERATOR, TVL_DIVIDE, TIN_PRECEDENCE_3);
				break;

			case '%':
				if (string[1] == '=')
					SET_TOKEN_INFO(2, TOK_OPERATOR, TVL_ASSIGNMODULO, TIN_PRECEDENCE_13 | TIN_RIGHT_TO_LEFT);
				else
					SET_TOKEN_INFO(1, TOK_OPERATOR, TVL_MODULO, TIN_PRECEDENCE_3);
				break;

			case '<':
				if (string[1] == '<' && string[2] == '=')
					SET_TOKEN_INFO(3, TOK_OPERATOR, TVL_ASSIGNLSHIFT, TIN_PRECEDENCE_13 | TIN_RIGHT_TO_LEFT);
				else if (string[1] == '<')
					SET_TOKEN_INFO(2, TOK_OPERATOR, TVL_LSHIFT, TIN_PRECEDENCE_5);
				else if (string[1] == '=')
					SET_TOKEN_INFO(2, TOK_OPERATOR, TVL_LESSOREQUAL, TIN_PRECEDENCE_6);
				else
					SET_TOKEN_INFO(1, TOK_OPERATOR, TVL_LESS, TIN_PRECEDENCE_6);
				break;

			case '>':
				if (string[1] == '>' && string[2] == '=')
					SET_TOKEN_INFO(3, TOK_OPERATOR, TVL_ASSIGNRSHIFT, TIN_PRECEDENCE_13 | TIN_RIGHT_TO_LEFT);
				else if (string[1] == '>')
					SET_TOKEN_INFO(2, TOK_OPERATOR, TVL_RSHIFT, TIN_PRECEDENCE_5);
				else if (string[1] == '=')
					SET_TOKEN_INFO(2, TOK_OPERATOR, TVL_GREATEROREQUAL, TIN_PRECEDENCE_6);
				else
					SET_TOKEN_INFO(1, TOK_OPERATOR, TVL_GREATER, TIN_PRECEDENCE_6);
				break;

			case '=':
				if (string[1] == '=')
					SET_TOKEN_INFO(2, TOK_OPERATOR, TVL_EQUAL, TIN_PRECEDENCE_7);
				else
					SET_TOKEN_INFO(1, TOK_OPERATOR, TVL_ASSIGN, TIN_PRECEDENCE_13 | TIN_RIGHT_TO_LEFT);
				break;

			case '!':
				if (string[1] == '=')
					SET_TOKEN_INFO(2, TOK_OPERATOR, TVL_NOTEQUAL, TIN_PRECEDENCE_7);
				else
					SET_TOKEN_INFO(2, TOK_OPERATOR, TVL_COMPLEMENT, TIN_PRECEDENCE_2);
				break;

			case '&':
				if (string[1] == '&')
					SET_TOKEN_INFO(2, TOK_OPERATOR, TVL_LAND, TIN_PRECEDENCE_11);
				else if (string[1] == '=')
					SET_TOKEN_INFO(2, TOK_OPERATOR, TVL_ASSIGNBAND, TIN_PRECEDENCE_13 | TIN_RIGHT_TO_LEFT);
				else
					SET_TOKEN_INFO(1, TOK_OPERATOR, TVL_BAND, TIN_PRECEDENCE_8);
				break;

			case '|':
				if (string[1] == '|')
					SET_TOKEN_INFO(2, TOK_OPERATOR, TVL_LOR, TIN_PRECEDENCE_12);
				else if (string[1] == '=')
					SET_TOKEN_INFO(2, TOK_OPERATOR, TVL_ASSIGNBOR, TIN_PRECEDENCE_13 | TIN_RIGHT_TO_LEFT);
				else
					SET_TOKEN_INFO(1, TOK_OPERATOR, TVL_BOR, TIN_PRECEDENCE_10);
				break;

			case '^':
				if (string[1] == '=')
					SET_TOKEN_INFO(2, TOK_OPERATOR, TVL_ASSIGNBXOR, TIN_PRECEDENCE_13 | TIN_RIGHT_TO_LEFT);
				else
					SET_TOKEN_INFO(1, TOK_OPERATOR, TVL_BXOR, TIN_PRECEDENCE_9);
				break;

			case '"':
			{
				char buffer[MAX_STRING_LENGTH];
				int bufindex = 0, strindex;

				/* find a free string entry */
				for (strindex = 0; strindex < MAX_EXPRESSION_STRINGS; strindex++)
					if (expr->string[strindex] == NULL)
						break;
				if (strindex == MAX_EXPRESSION_STRINGS)
					return MAKE_EXPRERR_TOO_MANY_STRINGS(token->offset);

				/* accumulate a copy of the string */
				string++;
				while (string[0] != 0 && bufindex < MAX_STRING_LENGTH)
				{
					/* allow "" to mean a nested double-quote */
					if (string[0] == '"')
					{
						if (string[1] != '"')
							break;
						string++;
					}
					buffer[bufindex++] = *string++;
				}

				/* if we didn't find the ending quote, report an error */
				if (string[0] != '"')
					return MAKE_EXPRERR_UNBALANCED_QUOTES(token->offset);
				string++;

				/* terminate the string and allocate memory */
				buffer[bufindex++] = 0;
				expr->string[strindex] = malloc(bufindex);
				if (!expr->string[strindex])
					return MAKE_EXPRERR_OUT_OF_MEMORY(token->offset);

				/* copy the string in and make the token */
				strcpy(expr->string[strindex], buffer);
				token->type = TOK_STRING;
				token->value.p = expr->string[strindex];
				break;
			}

			case '\'':
			{
				UINT64 value = 0;

				/* accumulate the value of the character token */
				string++;
				while (string[0] != 0)
				{
					/* allow '' to mean a nested single quote */
					if (string[0] == '\'')
					{
						if (string[1] != '\'')
							break;
						string++;
					}
					value = (value << 8) | (UINT8)*string++;
				}

				/* if we didn't find the ending quote, report an error */
				if (string[0] != '\'')
					return MAKE_EXPRERR_UNBALANCED_QUOTES(token->offset);
				string++;

				/* make it a number token */
				token->type = TOK_NUMBER;
				token->value.i = value;
				break;
			}

			default:
			{
				static const char valid[] = "abcdefghijklmnopqrstuvwxyz0123456789_$#.";
				static const char numbers[] = "0123456789abcdef";
				int bufindex = 0, must_be_number = 0, numbase = DEFAULT_BASE;
				char buffer[MAX_SYMBOL_LENGTH];
				UINT64 value;

				/* accumulate a lower-case version of the symbol */
				while (1)
				{
					char val = tolower(string[0]);
					if (val == 0 || strchr(valid, val) == NULL)
						break;
					buffer[bufindex++] = val;
					string++;
				}
				buffer[bufindex] = 0;

				/* check for memory @ operators */
				if (string[0] == '@')
				{
					UINT16 info;
					if (parse_memory_operator(buffer, &info))
					{
						SET_TOKEN_INFO(1, TOK_OPERATOR, TVL_MEMORYAT, TIN_PRECEDENCE_2 | info);
						break;
					}
				}

				/* empty string is automatically invalid */
				if (buffer[0] == 0)
					return MAKE_EXPRERR_INVALID_TOKEN(token->offset);
				else
				{
					/* if we have a number prefix, assume it is a number */
					bufindex = 0;
					if (buffer[0] == '$') { numbase = 16; bufindex++; must_be_number = 1; }
					else if (buffer[0] == '0' && buffer[1] == 'x') { numbase = 16; bufindex += 2; must_be_number = 1; }
					else if (buffer[0] == '#') { numbase = 10; bufindex++; must_be_number = 1; }

					/* if we're not forced to be a number, check for a symbol match first */
					if (!must_be_number)
					{
						const symbol_entry *symbol = symtable_find(expr->table, buffer);
						if (symbol != NULL)
						{
							token->type = TOK_SYMBOL;
							token->value.p = (void *)symbol;

							/* if this is a function symbol, synthesize an execute function operator */
							if (symbol->type == SMT_FUNCTION)
							{
								token++;
								SET_TOKEN_INFO(0, TOK_OPERATOR, TVL_EXECUTEFUNC, TIN_PRECEDENCE_0);
							}
						}
					}

					/* see if we parse as a number */
					if (token->type == TOK_INVALID)
					{
						value = 0;
						while (buffer[bufindex] != 0)
						{
							const char *ptr = strchr(numbers, tolower(buffer[bufindex]));
							int digit;

							if (ptr == NULL)
								break;
							digit = ptr - numbers;
							if (digit >= numbase)
								break;
							value = (value * (UINT64)numbase) + digit;
							bufindex++;
						}

						/* if we succeeded as a number, make it so */
						if (buffer[bufindex] == 0)
						{
							token->type = TOK_NUMBER;
							token->value.i = value;
						}
					}

					/* report errors */
					if (token->type == TOK_INVALID)
					{
						if (must_be_number)
							return MAKE_EXPRERR_INVALID_NUMBER(token->offset);
						else
							return MAKE_EXPRERR_UNKNOWN_SYMBOL(token->offset);
					}
				}
				break;
			}
		}

		/* advance to the next token */
		token++;
	}

	/* add an ending token */
	token->type = TOK_END;
	token->value.i = 0;
	return EXPRERR_NONE;
}



/***************************************************************************
    INFIX TO POSTFIX CONVERSION
***************************************************************************/

/*-------------------------------------------------
    normalize_operator - resolve operator
    ambiguities based on neighboring tokens
-------------------------------------------------*/

static EXPRERR normalize_operator(parsed_expression *expr, int tokindex)
{
	parse_token *thistoken = &expr->token[tokindex];
	parse_token *nexttoken = thistoken + 1;
	parse_token *prevtoken = (tokindex == 0) ? NULL : (thistoken - 1);

	switch (thistoken->value.i)
	{
		/* Determine if an open paren is part of a function or not */
		case TVL_LPAREN:
			if (prevtoken != NULL && prevtoken->type == TOK_OPERATOR && prevtoken->value.i == TVL_EXECUTEFUNC)
				thistoken->info |= TIN_FUNCTION;
			break;

		/* Determine if ++ is a pre or post increment */
		case TVL_PLUSPLUS:
			if (nexttoken->type == TOK_SYMBOL || (nexttoken->type == TOK_OPERATOR && nexttoken->value.i == TVL_MEMORYAT))
			{
				thistoken->value.i = TVL_PREINCREMENT;
				thistoken->info = TIN_PRECEDENCE_2;
			}
			else if (prevtoken != NULL && (prevtoken->type == TOK_SYMBOL || (prevtoken->type == TOK_OPERATOR && prevtoken->value.i == TVL_MEMORYAT)))
			{
				thistoken->value.i = TVL_POSTINCREMENT;
				thistoken->info = TIN_PRECEDENCE_1;
			}
			else
				return MAKE_EXPRERR_SYNTAX(thistoken->offset);
			break;

		/* Determine if -- is a pre or post decrement */
		case TVL_MINUSMINUS:
			if (nexttoken->type == TOK_SYMBOL || (nexttoken->type == TOK_OPERATOR && nexttoken->value.i == TVL_MEMORYAT))
			{
				thistoken->value.i = TVL_PREDECREMENT;
				thistoken->info = TIN_PRECEDENCE_2;
			}
			else if (prevtoken != NULL && (prevtoken->type == TOK_SYMBOL || (prevtoken->type == TOK_OPERATOR && prevtoken->value.i == TVL_MEMORYAT)))
			{
				thistoken->value.i = TVL_POSTDECREMENT;
				thistoken->info = TIN_PRECEDENCE_1;
			}
			else
				return MAKE_EXPRERR_SYNTAX(thistoken->offset);
			break;

		/* Determine if +/- is a unary or binary */
		case TVL_ADD:
		case TVL_SUBTRACT:
			/* Assume we're unary if we are the first token, or if the previous token is not
                a symbol, a number, or a right parenthesis */
			if (prevtoken == NULL ||
				(prevtoken->type != TOK_SYMBOL && prevtoken->type != TOK_NUMBER &&
				 (prevtoken->type != TOK_OPERATOR || prevtoken->value.i != TVL_RPAREN)))
			{
				thistoken->value.i = (thistoken->value.i == TVL_ADD) ? TVL_UPLUS : TVL_UMINUS;
				thistoken->info = TIN_PRECEDENCE_2;
			}
			break;

		/* Determine if , refers to a function parameter */
		case TVL_COMMA:
		{
			int lookback;

			for (lookback = 0; lookback < MAX_STACK_DEPTH; lookback++)
			{
				parse_token *peek = peek_token(expr, lookback);
				if (!peek)
					break;
				if (peek->value.i == TVL_LPAREN)
				{
					if (peek->info & TIN_FUNCTION)
						thistoken->info |= TIN_FUNCTION;
					break;
				}
				if (peek->value.i == TVL_EXECUTEFUNC)
				{
					thistoken->info |= TIN_FUNCTION;
					break;
				}
			}
			break;
		}
	}
	return EXPRERR_NONE;
}


/*-------------------------------------------------
    infix_to_postfix - convert an infix sequence
    of tokens to a postfix sequence for processing
-------------------------------------------------*/

static EXPRERR infix_to_postfix(parsed_expression *expr)
{
	parse_token *dest = expr->token;
	parse_token dummy;
	parse_token *peek;
	int tokindex = 0;
	EXPRERR exprerr;

	/* start with an empty stack */
	init_token_stack(expr);

	/* loop over all the original tokens */
	for ( ; expr->token[tokindex].type != TOK_END; tokindex++)
	{
		parse_token *token = &expr->token[tokindex];

		/* If the character is an operand, append it to the result string */
		if (token->type == TOK_NUMBER || token->type == TOK_SYMBOL || token->type == TOK_STRING)
			*dest++ = *token;

		/* If this is an operator, process it */
		else if (token->type == TOK_OPERATOR)
		{
			int exprerr = normalize_operator(expr, tokindex);
			if (exprerr != 0)
				return exprerr;

			/* If the token is an opening parenthesis, push it onto the stack. */
			if (token->value.i == TVL_LPAREN)
			{
				exprerr = push_token(expr, token);
				if (exprerr != 0)
					return exprerr;
			}

			/* If the token is a closing parenthesis, pop all operators until we
               reach an opening parenthesis and append them to the result string. */
			else if (token->value.i == TVL_RPAREN)
			{
				/* loop until we can't peek at the stack anymore */
				while ((peek = peek_token(expr, 0)) != NULL)
				{
					if (peek->value.i == TVL_LPAREN)
						break;
					exprerr = pop_token(expr, dest++);
					if (exprerr != 0)
						return exprerr;
				}

				/* if we didn't find an open paren, it's an error */
				if (peek == NULL)
					return MAKE_EXPRERR_UNBALANCED_PARENS(token->offset);

				/* pop the open paren off the stack */
				exprerr = pop_token(expr, &dummy);
				if (exprerr != 0)
					return exprerr;
			}

			/* If the token is an operator, pop operators until we reach an opening parenthesis,
               an operator of lower precedence, or a right associative symbol of equal precedence.
               Push the operator onto the stack. */
			else
			{
				int our_precedence = token->info & TIN_PRECEDENCE_MASK;

				/* loop until we can't peek at the stack anymore */
				while ((peek = peek_token(expr, 0)) != NULL)
				{
					int stack_precedence = peek->info & TIN_PRECEDENCE_MASK;

					/* break if any of the above conditions are true */
					if (peek->value.i == TVL_LPAREN)
						break;
					if (stack_precedence > our_precedence)
						break;
					if (stack_precedence == our_precedence && (peek->info & TIN_RIGHT_TO_LEFT))
						break;

					/* pop this token */
					exprerr = pop_token(expr, dest++);
					if (exprerr != 0)
						return exprerr;
				}

				/* push the new operator */
				exprerr = push_token(expr, token);
				if (exprerr != 0)
					return exprerr;
			}
		}
	}

	/* finish popping the stack */
	while ((peek = peek_token(expr, 0)) != NULL)
	{
		/* it is an error to have a left parenthesis still on the stack */
		if (peek->value.i == TVL_LPAREN)
			return MAKE_EXPRERR_UNBALANCED_PARENS(peek->offset);

		/* pop this token */
		exprerr = pop_token(expr, dest++);
		if (exprerr != 0)
			return exprerr;
	}

	/* copy the end token to the final stream */
	*dest++ = expr->token[tokindex];
	return EXPRERR_NONE;
}



/***************************************************************************
    EXPRESSION EVALUATION
***************************************************************************/

/*-------------------------------------------------
    execute_function - handle an execute function
    operator
-------------------------------------------------*/

static EXPRERR execute_function(parsed_expression *expr, parse_token *token)
{
	UINT64 funcparams[MAX_FUNCTION_PARAMS];
	symbol_entry *symbol = NULL;
	int paramcount = 0;
	parse_token t1;
	EXPRERR exprerr;

	/* pop off all pushed parameters */
	while (paramcount < MAX_FUNCTION_PARAMS)
	{
		/* peek at the next token on the stack */
		parse_token *peek = peek_token(expr, 0);
		if (!peek)
			return MAKE_EXPRERR_INVALID_PARAM_COUNT(token->offset);

		/* if it is a function symbol, break out of the loop */
		if (peek->type == TOK_SYMBOL)
		{
			symbol = peek->value.p;
			if (symbol != NULL && symbol->type == SMT_FUNCTION)
			{
				pop_token(expr, &t1);
				break;
			}
		}

		/* otherwise, pop as a standard rval */
		exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
		funcparams[MAX_FUNCTION_PARAMS - (++paramcount)] = t1.value.i;
	}

	/* if we didn't find the symbol, fail */
	if (paramcount == MAX_FUNCTION_PARAMS)
		return MAKE_EXPRERR_INVALID_PARAM_COUNT(token->offset);

	/* validate the number of parameters */
	if (paramcount < symbol->info.func.minparams || paramcount > symbol->info.func.maxparams)
		return MAKE_EXPRERR_INVALID_PARAM_COUNT(token->offset);

	/* execute the function and push the result */
	t1.type = TOK_NUMBER;
	t1.offset = token->offset;
	t1.value.i = (*symbol->info.func.execute)(symbol->ref, paramcount, &funcparams[MAX_FUNCTION_PARAMS - paramcount]);
	push_token(expr, &t1);

	return EXPRERR_NONE;
}


/*-------------------------------------------------
    execute_tokens - execute a postfix sequence
    of tokens
-------------------------------------------------*/

static EXPRERR execute_tokens(parsed_expression *expr, UINT64 *result)
{
	parse_token t1, t2, tempnum, tempmem;
	EXPRERR exprerr;
	int tokindex;

	/* reset the token stack */
	init_token_stack(expr);

	/* create a temporary token for holding intermediate number and memory values */
	tempnum.type = TOK_NUMBER;
	tempnum.offset = 0;
	tempnum.info = 0;
	tempmem.type = TOK_MEMORY;
	tempmem.offset = 0;
	tempmem.info = 0;

	/* loop over the entire sequence */
	for (tokindex = 0; expr->token[tokindex].type != TOK_END; tokindex++)
	{
		parse_token *token = &expr->token[tokindex];

		switch (token->type)
		{
			default:
			case TOK_INVALID:
				return MAKE_EXPRERR_INVALID_TOKEN(token->offset);

			case TOK_SYMBOL:
			case TOK_NUMBER:
			case TOK_STRING:
				exprerr = push_token(expr, token);
				if (exprerr != 0)
					return exprerr;
				break;

			case TOK_OPERATOR:
				switch (token->value.i)
				{
					case TVL_PREINCREMENT:
						exprerr = pop_token_lval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = get_lval_value(&t1, expr->table) + 1;
						tempnum.offset = t1.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						set_lval_value(&t1, expr->table, tempnum.value.i);
						break;

					case TVL_PREDECREMENT:
						exprerr = pop_token_lval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = get_lval_value(&t1, expr->table) - 1;
						tempnum.offset = t1.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						set_lval_value(&t1, expr->table, tempnum.value.i);
						break;

					case TVL_POSTINCREMENT:
						exprerr = pop_token_lval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = get_lval_value(&t1, expr->table);
						tempnum.offset = t1.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						set_lval_value(&t1, expr->table, tempnum.value.i + 1);
						break;

					case TVL_POSTDECREMENT:
						exprerr = pop_token_lval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = get_lval_value(&t1, expr->table);
						tempnum.offset = t1.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						set_lval_value(&t1, expr->table, tempnum.value.i - 1);
						break;

					case TVL_COMPLEMENT:
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = !t1.value.i;
						tempnum.offset = t1.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_NOT:
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = ~t1.value.i;
						tempnum.offset = t1.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_UPLUS:
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = t1.value.i;
						tempnum.offset = t1.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_UMINUS:
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = -t1.value.i;
						tempnum.offset = t1.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_MULTIPLY:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = t1.value.i * t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_DIVIDE:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						if (t2.value.i == 0) return MAKE_EXPRERR_DIVIDE_BY_ZERO(t2.offset);
						tempnum.value.i = t1.value.i / t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_MODULO:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						if (t2.value.i == 0) return MAKE_EXPRERR_DIVIDE_BY_ZERO(t2.offset);
						tempnum.value.i = t1.value.i % t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_ADD:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = t1.value.i + t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_SUBTRACT:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = t1.value.i - t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_LSHIFT:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = t1.value.i << t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_RSHIFT:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = t1.value.i >> t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_LESS:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = t1.value.i < t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_LESSOREQUAL:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = t1.value.i <= t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_GREATER:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = t1.value.i > t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_GREATEROREQUAL:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = t1.value.i >= t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_EQUAL:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = t1.value.i == t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_NOTEQUAL:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = t1.value.i != t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_BAND:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = t1.value.i & t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_BXOR:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = t1.value.i ^ t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_BOR:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = t1.value.i | t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_LAND:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = t1.value.i && t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_LOR:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = t1.value.i || t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						break;

					case TVL_ASSIGN:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_lval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						push_token(expr, &t2);
						set_lval_value(&t1, expr->table, t2.value.i);
						break;

					case TVL_ASSIGNMULTIPLY:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_lval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = get_lval_value(&t1, expr->table);
						tempnum.value.i *= t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						set_lval_value(&t1, expr->table, tempnum.value.i);
						break;

					case TVL_ASSIGNDIVIDE:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_lval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = get_lval_value(&t1, expr->table);
						if (t2.value.i == 0) return MAKE_EXPRERR_DIVIDE_BY_ZERO(t2.offset);
						tempnum.value.i /= t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						set_lval_value(&t1, expr->table, tempnum.value.i);
						break;

					case TVL_ASSIGNMODULO:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_lval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = get_lval_value(&t1, expr->table);
						if (t2.value.i == 0) return MAKE_EXPRERR_DIVIDE_BY_ZERO(t2.offset);
						tempnum.value.i %= t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						set_lval_value(&t1, expr->table, tempnum.value.i);
						break;

					case TVL_ASSIGNADD:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_lval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = get_lval_value(&t1, expr->table);
						tempnum.value.i += t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						set_lval_value(&t1, expr->table, tempnum.value.i);
						break;

					case TVL_ASSIGNSUBTRACT:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_lval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = get_lval_value(&t1, expr->table);
						tempnum.value.i -= t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						set_lval_value(&t1, expr->table, tempnum.value.i);
						break;

					case TVL_ASSIGNLSHIFT:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_lval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = get_lval_value(&t1, expr->table);
						tempnum.value.i <<= t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						set_lval_value(&t1, expr->table, tempnum.value.i);
						break;

					case TVL_ASSIGNRSHIFT:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_lval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = get_lval_value(&t1, expr->table);
						tempnum.value.i >>= t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						set_lval_value(&t1, expr->table, tempnum.value.i);
						break;

					case TVL_ASSIGNBAND:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_lval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = get_lval_value(&t1, expr->table);
						tempnum.value.i &= t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						set_lval_value(&t1, expr->table, tempnum.value.i);
						break;

					case TVL_ASSIGNBXOR:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_lval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = get_lval_value(&t1, expr->table);
						tempnum.value.i ^= t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						set_lval_value(&t1, expr->table, tempnum.value.i);
						break;

					case TVL_ASSIGNBOR:
						exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
						exprerr = pop_token_lval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempnum.value.i = get_lval_value(&t1, expr->table);
						tempnum.value.i |= t2.value.i;
						tempnum.offset = (t1.offset < t2.offset) ? t1.offset : t2.offset;
						exprerr = push_token(expr, &tempnum); if (exprerr != 0) return exprerr;
						set_lval_value(&t1, expr->table, tempnum.value.i);
						break;

					case TVL_COMMA:
						if (!(token->info & TIN_FUNCTION))
						{
							exprerr = pop_token_rval(expr, &t2, expr->table); if (exprerr != 0) return exprerr;
							exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
							exprerr = push_token(expr, &t2); if (exprerr != 0) return exprerr;
						}
						break;

					case TVL_MEMORYAT:
						exprerr = pop_token_rval(expr, &t1, expr->table); if (exprerr != 0) return exprerr;
						tempmem.value.i = t1.value.i;
						tempmem.info = token->info;
						exprerr = push_token(expr, &tempmem); if (exprerr != 0) return exprerr;
						break;

					case TVL_EXECUTEFUNC:
						exprerr = execute_function(expr, token); if (exprerr != 0) return exprerr;
						break;

					default:
						return MAKE_EXPRERR_SYNTAX(token->offset);
				}
				break;
		}
	}

	/* pop the final result */
	exprerr = pop_token_rval(expr, &tempnum, expr->table);
	if (exprerr != 0)
		return exprerr;

	/* error if our stack isn't empty */
	if (peek_token(expr, 0) != NULL)
		return MAKE_EXPRERR_SYNTAX(0);

	*result = tempnum.value.i;
	return EXPRERR_NONE;
}



/***************************************************************************
    MISC HELPERS
***************************************************************************/

/*-------------------------------------------------
    free_expression_strings - free all strings
    allocated to an expression
-------------------------------------------------*/

static void free_expression_strings(parsed_expression *expr)
{
	int strindex;

	/* free the original expression */
	if (expr->original_string != NULL)
		free(expr->original_string);
	expr->original_string = NULL;

	/* free all strings */
	for (strindex = 0; strindex < MAX_EXPRESSION_STRINGS; strindex++)
		if (expr->string[strindex] != NULL)
		{
			free(expr->string[strindex]);
			expr->string[strindex] = NULL;
		}
}



/***************************************************************************
    CORE WRAPPER FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    expression_evaluate - evaluate a string
    expression using the passed symbol table
-------------------------------------------------*/

EXPRERR expression_evaluate(const char *expression, const symbol_table *table, UINT64 *result)
{
	parsed_expression temp_expression;
	EXPRERR exprerr;

	/* first parse the tokens into the token array in order */
	exprerr = parse_string_into_tokens(expression, &temp_expression, table);
	if (exprerr != EXPRERR_NONE)
		goto cleanup;

	/* debugging */
	print_tokens(stdout, &temp_expression);

	/* convert the infix order to postfix order */
	exprerr = infix_to_postfix(&temp_expression);
	if (exprerr != EXPRERR_NONE)
		goto cleanup;

	/* debugging */
	print_tokens(stdout, &temp_expression);

	/* execute the expression to get the result */
	exprerr = execute_tokens(&temp_expression, result);

cleanup:
	free_expression_strings(&temp_expression);
	return exprerr;
}


/*-------------------------------------------------
    expression_parse - parse an expression and
    return an allocated token array
-------------------------------------------------*/

EXPRERR expression_parse(const char *expression, const symbol_table *table, parsed_expression **result)
{
	parsed_expression temp_expression;
	EXPRERR exprerr;

	/* first parse the tokens into the token array in order */
	exprerr = parse_string_into_tokens(expression, &temp_expression, table);
	if (exprerr != EXPRERR_NONE)
		goto cleanup;

	/* convert the infix order to postfix order */
	exprerr = infix_to_postfix(&temp_expression);
	if (exprerr != EXPRERR_NONE)
		goto cleanup;

	/* allocate memory for the result */
	*result = malloc(sizeof(temp_expression));
	if (!*result)
	{
		exprerr = MAKE_EXPRERR_OUT_OF_MEMORY(0);
		goto cleanup;
	}

	/* copy the final expression and return */
	**result = temp_expression;
	return EXPRERR_NONE;

cleanup:
	free_expression_strings(&temp_expression);
	return exprerr;
}


/*-------------------------------------------------
    expression_execute - execute a
    previously-parsed expression
-------------------------------------------------*/

EXPRERR expression_execute(parsed_expression *expr, UINT64 *result)
{
	/* execute the expression to get the result */
	return execute_tokens(expr, result);
}


/*-------------------------------------------------
    expression_free - free a previously
    allocated parsed expression
-------------------------------------------------*/

void expression_free(parsed_expression *expr)
{
	if (expr)
	{
		free_expression_strings(expr);
		free(expr);
	}
}


/*-------------------------------------------------
    expression_original_string - return a
    pointer to the original expression string
-------------------------------------------------*/

const char *expression_original_string(parsed_expression *expr)
{
	return expr->original_string;
}



/***************************************************************************
    ERROR HANDLING
***************************************************************************/

/*-------------------------------------------------
    exprerr_to_string - return a friendly
    string for a given expression error
-------------------------------------------------*/

const char *exprerr_to_string(EXPRERR error)
{
	switch (EXPRERR_ERROR_CLASS(error))
	{
		case EXPRERR_NOT_LVAL:				return "not an lvalue";
		case EXPRERR_NOT_RVAL:				return "not an rvalue";
		case EXPRERR_SYNTAX:				return "syntax error";
		case EXPRERR_UNKNOWN_SYMBOL:		return "unknown symbol";
		case EXPRERR_INVALID_NUMBER:		return "invalid number";
		case EXPRERR_INVALID_TOKEN:			return "invalid token";
		case EXPRERR_STACK_OVERFLOW:		return "stack overflow";
		case EXPRERR_STACK_UNDERFLOW:		return "stack underflow";
		case EXPRERR_UNBALANCED_PARENS:		return "unbalanced parentheses";
		case EXPRERR_DIVIDE_BY_ZERO:		return "divide by zero";
		case EXPRERR_OUT_OF_MEMORY:			return "out of memory";
		case EXPRERR_INVALID_PARAM_COUNT:	return "invalid number of parameters";
		case EXPRERR_UNBALANCED_QUOTES:		return "unbalanced quotes";
		case EXPRERR_TOO_MANY_STRINGS:		return "too many strings";
		default:							return "unknown error";
	}
}



/***************************************************************************
    SYMBOL TABLES
***************************************************************************/

/*-------------------------------------------------
    hash_string - simple string hash
-------------------------------------------------*/

INLINE UINT32 hash_string(const char *string)
{
	UINT32 hash = 0;
	while (*string)
		hash = (hash * 31) + *string++;
	return hash;
}


/*-------------------------------------------------
    symtable_alloc - allocate a symbol table
-------------------------------------------------*/

symbol_table *symtable_alloc(symbol_table *parent)
{
	symbol_table *table;

	/* allocate memory for the table */
	table = malloc(sizeof(*table));
	if (!table)
		return NULL;

	/* initialize the data */
	memset(table, 0, sizeof(*table));
	table->parent = parent;
	return table;
}


/*-------------------------------------------------
    symtable_add - add a new symbol to a
    symbol table
-------------------------------------------------*/

int symtable_add(symbol_table *table, const char *name, const symbol_entry *entry)
{
	internal_symbol_entry *symbol;
	symbol_entry *oldentry;
	char *newstring;
	UINT32 hash_index;
	int strindex;
	int all_digits, i;

	/* we cannot add numeric symbols */
	all_digits = TRUE;
	for (i = 0; name[i]; i++)
	{
		if (!isdigit(name[i]))
		{
			all_digits = FALSE;
			break;
		}
	}
	assert_always(!all_digits, "All-digit symbols are not allowed");

	/* see if we already have an entry and just overwrite it if we do */
	oldentry = (symbol_entry *)symtable_find(table, name);
	if (oldentry)
	{
		*oldentry = *entry;
		return 1;
	}

	/* otherwise, allocate a new entry */
	symbol = malloc(sizeof(*symbol));
	if (!symbol)
		return 0;
	memset(symbol, 0, sizeof(*symbol));

	/* allocate space for a copy of the string */
	newstring = malloc(strlen(name) + 1);
	if (!newstring)
	{
		free(symbol);
		return 0;
	}

	/* copy the string, converting to lowercase */
	for (strindex = 0; name[strindex] != 0; strindex++)
		newstring[strindex] = tolower(name[strindex]);
	newstring[strindex] = 0;

	/* fill in the details */
	symbol->name = newstring;
	symbol->entry = *entry;

	/* add the entry to the hash table */
	hash_index = hash_string(newstring) % SYM_TABLE_HASH_SIZE;
	symbol->next = table->hash[hash_index];
	table->hash[hash_index] = symbol;
	return 1;
}


/*-------------------------------------------------
    symtable_add_register - add a new
    register symbol to a symbol table
-------------------------------------------------*/

int	symtable_add_register(symbol_table *table, const char *name, UINT32 ref, UINT64 (*getter)(UINT32), void (*setter)(UINT32, UINT64))
{
	symbol_entry symbol;

	symbol.ref = ref;
	symbol.type = SMT_REGISTER;
	symbol.info.reg.getter = getter;
	symbol.info.reg.setter = setter;
	return symtable_add(table, name, &symbol);
}


/*-------------------------------------------------
    symtable_add_function - add a new
    function symbol to a symbol table
-------------------------------------------------*/

int symtable_add_function(symbol_table *table, const char *name, UINT32 ref, UINT16 minparams, UINT16 maxparams, UINT64 (*execute)(UINT32, UINT32, UINT64 *))
{
	symbol_entry symbol;

	symbol.ref = ref;
	symbol.type = SMT_FUNCTION;
	symbol.info.func.minparams = minparams;
	symbol.info.func.maxparams = maxparams;
	symbol.info.func.execute = execute;
	return symtable_add(table, name, &symbol);
}


/*-------------------------------------------------
    symtable_add_value - add a new
    value symbol to a symbol table
-------------------------------------------------*/

int symtable_add_value(symbol_table *table, const char *name, UINT64 value)
{
	symbol_entry symbol;

	symbol.ref = 0;
	symbol.type = SMT_VALUE;
	symbol.info.gen.value = value;
	return symtable_add(table, name, &symbol);
}


/*-------------------------------------------------
    symtable_find - find a symbol in a symbol
    table
-------------------------------------------------*/

const symbol_entry *symtable_find(const symbol_table *table, const char *name)
{
	UINT32 hash_index = hash_string(name) % SYM_TABLE_HASH_SIZE;
	const internal_symbol_entry *symbol;

	/* loop until we run out of tables */
	while (table)
	{
		/* search linearly within this hash entry */
		for (symbol = table->hash[hash_index]; symbol; symbol = symbol->next)
			if (!strcmp(symbol->name, name))
				return &symbol->entry;

		/* look in the parent */
		table = table->parent;
	}

	return NULL;
}


/*-------------------------------------------------
    symtable_find_indexed - find an indexed symbol
    in a symbol table
-------------------------------------------------*/

const char *symtable_find_indexed(const symbol_table *table, int index, const symbol_entry **entry)
{
	const internal_symbol_entry *symbol;
	int hash_index;

	/* loop over hash entries, then over entries within each bucket */
	for (hash_index = 0; hash_index < SYM_TABLE_HASH_SIZE; hash_index++)
		for (symbol = table->hash[hash_index]; symbol; symbol = symbol->next)
			if (index-- == 0)
			{
				if (entry)
					*entry = &symbol->entry;
				return symbol->name;
			}

	return NULL;
}


/*-------------------------------------------------
    symtable_free - free a symbol table
-------------------------------------------------*/

void symtable_free(symbol_table *table)
{
	internal_symbol_entry *entry, *next;
	int hash_index;

	/* free all the entries in the hash table */
	for (hash_index = 0; hash_index < SYM_TABLE_HASH_SIZE; hash_index++)
		for (entry = table->hash[hash_index]; entry; entry = next)
		{
			/* free the allocated name */
			if (entry->name)
				free((void *)entry->name);

			/* remove from this list and put on the free list */
			next = entry->next;
			free(entry);
		}

	/* free the structure */
	free(table);
}
