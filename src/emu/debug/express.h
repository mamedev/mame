/***************************************************************************

    express.h
    Generic expressions engine.
    Written by Aaron Giles
    Copyright Aaron Giles

***************************************************************************/

#ifndef __EXPRESS_H__
#define __EXPRESS_H__

#include "mamecore.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* maximum number of parameters in a function call */
#define MAX_FUNCTION_PARAMS					(16)

/* values for symbol_entry.type */
#define SMT_REGISTER						(0)
#define SMT_FUNCTION						(1)
#define SMT_VALUE							(2)

/* values for the error code in an expression error */
#define EXPRERR_NONE						(0)
#define EXPRERR_NOT_LVAL					(1)
#define EXPRERR_NOT_RVAL					(2)
#define EXPRERR_SYNTAX						(3)
#define EXPRERR_UNKNOWN_SYMBOL				(4)
#define EXPRERR_INVALID_NUMBER				(5)
#define EXPRERR_INVALID_TOKEN				(6)
#define EXPRERR_STACK_OVERFLOW				(7)
#define EXPRERR_STACK_UNDERFLOW				(8)
#define EXPRERR_UNBALANCED_PARENS			(9)
#define EXPRERR_DIVIDE_BY_ZERO				(10)
#define EXPRERR_OUT_OF_MEMORY				(11)
#define EXPRERR_INVALID_PARAM_COUNT			(12)
#define EXPRERR_UNBALANCED_QUOTES			(13)
#define EXPRERR_TOO_MANY_STRINGS			(14)

/* values for the address space passed to external_read/write_memory */
#define EXPSPACE_PROGRAM					(0)
#define EXPSPACE_DATA						(1)
#define EXPSPACE_IO							(2)
#define EXPSPACE_OPCODE						(3)
#define EXPSPACE_RAMWRITE					(4)
#define EXPSPACE_EEPROM						(5)
#define EXPSPACE_CPU						(6)
#define EXPSPACE_USER						(7)
#define EXPSPACE_GFX						(8)
#define EXPSPACE_SOUND						(9)



/***************************************************************************
    MACROS
***************************************************************************/

/* expression error assembly/disassembly macros */
#define EXPRERR_ERROR_CLASS(x)				((x) >> 16)
#define EXPRERR_ERROR_OFFSET(x)				((x) & 0xffff)
#define MAKE_EXPRERR(a,b)					(((a) << 16) | ((b) & 0xffff))

/* macros to assemble specific error conditions */
#define MAKE_EXPRERR_NOT_LVAL(x)			MAKE_EXPRERR(EXPRERR_NOT_LVAL, (x))
#define MAKE_EXPRERR_NOT_RVAL(x)			MAKE_EXPRERR(EXPRERR_NOT_RVAL, (x))
#define MAKE_EXPRERR_SYNTAX(x)				MAKE_EXPRERR(EXPRERR_SYNTAX, (x))
#define MAKE_EXPRERR_UNKNOWN_SYMBOL(x)		MAKE_EXPRERR(EXPRERR_UNKNOWN_SYMBOL, (x))
#define MAKE_EXPRERR_INVALID_NUMBER(x)		MAKE_EXPRERR(EXPRERR_INVALID_NUMBER, (x))
#define MAKE_EXPRERR_INVALID_TOKEN(x)		MAKE_EXPRERR(EXPRERR_INVALID_TOKEN, (x))
#define MAKE_EXPRERR_STACK_OVERFLOW(x)		MAKE_EXPRERR(EXPRERR_STACK_OVERFLOW, (x))
#define MAKE_EXPRERR_STACK_UNDERFLOW(x)		MAKE_EXPRERR(EXPRERR_STACK_UNDERFLOW, (x))
#define MAKE_EXPRERR_UNBALANCED_PARENS(x)	MAKE_EXPRERR(EXPRERR_UNBALANCED_PARENS, (x))
#define MAKE_EXPRERR_DIVIDE_BY_ZERO(x)		MAKE_EXPRERR(EXPRERR_DIVIDE_BY_ZERO, (x))
#define MAKE_EXPRERR_OUT_OF_MEMORY(x)		MAKE_EXPRERR(EXPRERR_OUT_OF_MEMORY, (x))
#define MAKE_EXPRERR_INVALID_PARAM_COUNT(x)	MAKE_EXPRERR(EXPRERR_INVALID_PARAM_COUNT, (x))
#define MAKE_EXPRERR_UNBALANCED_QUOTES(x)	MAKE_EXPRERR(EXPRERR_UNBALANCED_QUOTES, (x))
#define MAKE_EXPRERR_TOO_MANY_STRINGS(x)	MAKE_EXPRERR(EXPRERR_TOO_MANY_STRINGS, (x))



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* callback functions for getting/setting a symbol value */
typedef UINT64 (*symbol_getter_func)(UINT32 ref);
typedef void (*symbol_setter_func)(UINT32 ref, UINT64 value);

/* callback function for execution a function */
typedef UINT64 (*function_execute_func)(UINT32 ref, UINT32 numparams, const UINT64 *paramlist);

/* callback function for memory reads/writes */
typedef UINT64 (*express_read_func)(const char *name, int space, UINT32 offset, int size);
typedef void (*express_write_func)(const char *name, int space, UINT32 offset, int size, UINT64 value);


/* callback parameter for executing expressions */
typedef struct _express_callbacks express_callbacks;
struct _express_callbacks
{
	express_read_func	read;					/* read callback */
	express_write_func	write;					/* write callback */
};


/* symbol_entry describes a symbol in a symbol table */
typedef struct _symbol_entry symbol_entry;
struct _symbol_entry
{
	UINT32			ref;						/* internal reference */
	UINT32			type;						/* type of symbol */
	union
	{
		/* register info */
		struct
		{
			symbol_getter_func 		getter;		/* value getter */
			symbol_setter_func 		setter;		/* value setter */
		} reg;

		/* function info */
		struct
		{
			UINT16					minparams;	/* minimum expected parameters */
			UINT16					maxparams;	/* maximum expected parameters */
			function_execute_func 	execute;	/* execute callback */
		} func;

		/* generic info */
		struct generic_info
		{
			void *					ptr;		/* generic pointer */
			UINT64					value;		/* generic value */
		} gen;
	} info;
};


/* symbol_table is an opaque structure for holding a collection of symbols */
typedef struct _symbol_table symbol_table;


/* parsed_expression is an opaque structure for holding a pre-parsed expression */
typedef struct _parsed_expression parsed_expression;


/* EXPRERR is an error code for expression evaluation */
typedef UINT32 EXPRERR;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* expression evaluation */
EXPRERR 					expression_evaluate(const char *expression, const symbol_table *table, const express_callbacks *callbacks, UINT64 *result);
EXPRERR 					expression_parse(const char *expression, const symbol_table *table, const express_callbacks *callbacks, parsed_expression **result);
EXPRERR 					expression_execute(parsed_expression *expr, UINT64 *result);
void 						expression_free(parsed_expression *expr);
const char *				expression_original_string(parsed_expression *expr);
const char *				exprerr_to_string(EXPRERR error);

/* symbol table manipulation */
symbol_table *				symtable_alloc(symbol_table *parent);
int 						symtable_add(symbol_table *table, const char *name, const symbol_entry *entry);
int 						symtable_add_register(symbol_table *table, const char *name, UINT32 ref, symbol_getter_func getter, symbol_setter_func setter);
int 						symtable_add_function(symbol_table *table, const char *name, UINT32 ref, UINT16 minparams, UINT16 maxparams, function_execute_func execute);
int							symtable_add_value(symbol_table *table, const char *name, UINT64 value);
const symbol_entry *		symtable_find(const symbol_table *table, const char *name);
const char *				symtable_find_indexed(const symbol_table *table, int index, const symbol_entry **entry);
void 						symtable_free(symbol_table *table);

#endif
