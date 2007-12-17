/***************************************************************************

    state.h

    Save state management functions.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __STATE_H__
#define __STATE_H__

#include "mamecore.h"



/***************************************************************************
    MACROS
***************************************************************************/

#define IS_COMPATIBLE_TYPE(_valtype, _checktype)										\
	(sizeof(_valtype) == sizeof(_checktype) && TYPES_COMPATIBLE(typeof(_valtype), _checktype))

#define IS_VALID_SAVE_TYPE(_valtype)													\
	(IS_COMPATIBLE_TYPE(_valtype, double) || IS_COMPATIBLE_TYPE(_valtype, float)  ||	\
	 IS_COMPATIBLE_TYPE(_valtype, INT64)  || IS_COMPATIBLE_TYPE(_valtype, UINT64) ||	\
	 IS_COMPATIBLE_TYPE(_valtype, INT32)  || IS_COMPATIBLE_TYPE(_valtype, UINT32) ||	\
	 IS_COMPATIBLE_TYPE(_valtype, INT16)  || IS_COMPATIBLE_TYPE(_valtype, UINT16) ||	\
	 IS_COMPATIBLE_TYPE(_valtype, INT8)   || IS_COMPATIBLE_TYPE(_valtype, UINT8) ||		\
	 IS_COMPATIBLE_TYPE(_valtype, PAIR)   || IS_COMPATIBLE_TYPE(_valtype, PAIR64))

#define state_save_register_generic(_mod, _inst, _name, _val, _valsize, _count) 		\
do {																					\
	assert_always(IS_VALID_SAVE_TYPE(_valsize), "Invalid data type supplied for state saving.");\
	state_save_register_memory(_mod, _inst, _name, _val, sizeof(_valsize), _count);		\
} while (0)

#define state_save_register_item(_mod, _inst, _val)	\
	state_save_register_generic(_mod, _inst, #_val, &_val, _val, 1)

#define state_save_register_item_pointer(_mod, _inst, _val, _count) \
	state_save_register_generic(_mod, _inst, #_val, &_val[0], _val[0], _count)

#define state_save_register_item_array(_mod, _inst, _val) \
	state_save_register_item_pointer(_mod, _inst, _val, sizeof(_val)/sizeof(_val[0]))

#define state_save_register_item_2d_array(_mod, _inst, _val) \
	state_save_register_item_pointer(_mod, _inst, _val[0], sizeof(_val)/sizeof(_val[0][0]))

#define state_save_register_global(_val) \
	state_save_register_item("globals", 0, _val)

#define state_save_register_global_pointer(_val, _count) \
	state_save_register_item_pointer("globals", 0, _val, _count)

#define state_save_register_global_array(_val) \
	state_save_register_item_array("globals", 0, _val)

#define state_save_register_global_2d_array(_val) \
	state_save_register_item_2d_array("globals", 0, _val)

#define state_save_register_global_bitmap(_val) \
	state_save_register_bitmap("globals", 0, #_val, _val)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

void state_init(running_machine *machine);

/* Initializes the save state registrations */
void state_destructor(void *ptr, size_t size);
void state_save_allow_registration(int allowed);
int state_save_registration_allowed(void);

/* Registering functions */
int state_save_get_reg_count(void);

void state_save_register_memory(const char *module, UINT32 instance, const char *name, void *val, UINT32 valsize, UINT32 valcount);
void state_save_register_bitmap(const char *module, UINT32 instance, const char *name, mame_bitmap *val);

void state_save_register_func_presave(void (*func)(void));
void state_save_register_func_postload(void (*func)(void));

void state_save_register_func_presave_int(void (*func)(int), int param);
void state_save_register_func_postload_int(void (*func)(int), int param);

void state_save_register_func_presave_ptr(void (*func)(void *), void *param);
void state_save_register_func_postload_ptr(void (*func)(void *), void *param);

/* Save and load functions */
/* The tags are a hack around the current cpu structures */
int  state_save_save_begin(mame_file *file);
int  state_save_load_begin(mame_file *file);

void state_save_push_tag(int tag);
void state_save_pop_tag(void);

void state_save_save_continue(void);
void state_save_load_continue(void);

void state_save_save_finish(void);
void state_save_load_finish(void);

/* Display function */
void state_save_dump_registry(void);

const char *state_save_get_indexed_item(int index, void **base, UINT32 *valsize, UINT32 *valcount);

/* Verification function; can be called from front ends */
int state_save_check_file(mame_file *file, const char *gamename, int validate_signature, void (CLIB_DECL *errormsg)(const char *fmt, ...));

#endif	/* __STATE_H__ */
