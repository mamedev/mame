/*********************************************************************

    debugvw.h

    Debugger view engine.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#ifndef __DEBUGVIEW_H__
#define __DEBUGVIEW_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* types passed to debug_view_alloc */
#define DVT_CONSOLE							(1)
#define DVT_REGISTERS						(2)
#define DVT_DISASSEMBLY						(3)
#define DVT_MEMORY							(4)
#define DVT_LOG								(5)
#define DVT_TIMERS							(6)
#define DVT_ALLOCS							(7)

/* properties available for all views */
#define DVP_VISIBLE_ROWS					(1)		/* r/w - UINT32 */
#define DVP_VISIBLE_COLS					(2)		/* r/w - UINT32 */
#define DVP_TOTAL_ROWS						(3)		/* r/w - UINT32 */
#define DVP_TOTAL_COLS						(4)		/* r/w - UINT32 */
#define DVP_TOP_ROW							(5)		/* r/w - UINT32 */
#define DVP_LEFT_COL						(6)		/* r/w - UINT32 */
#define DVP_UPDATE_CALLBACK					(7)		/* r/w - void (*update)(debug_view *) */
#define DVP_VIEW_DATA						(8)		/* r/o - debug_view_char * */
#define DVP_SUPPORTS_CURSOR					(9)		/* r/o - UINT32 */
#define DVP_CURSOR_VISIBLE					(10)	/* r/w - UINT32 */
#define DVP_CURSOR_ROW						(11)	/* r/w - UINT32 */
#define DVP_CURSOR_COL						(12)	/* r/w - UINT32 */
#define DVP_CHARACTER						(13)	/* w/o - UINT32 */
#define DVP_OSD_PRIVATE						(14)	/* r/w - void * */

/* properties available for register views */
#define DVP_REGS_CPUNUM						(100)	/* r/w - UINT32 */

/* properties available for disassembly views */
#define DVP_DASM_CPUNUM						(100)	/* r/w - UINT32 */
#define DVP_DASM_EXPRESSION					(101)	/* r/w - const char * */
#define DVP_DASM_TRACK_LIVE					(102)	/* r/w - UINT32 */
#define DVP_DASM_RIGHT_COLUMN				(103)	/* r/w - UINT32 */
#define   DVP_DASM_RIGHTCOL_NONE			(0)
#define   DVP_DASM_RIGHTCOL_RAW				(1)
#define   DVP_DASM_RIGHTCOL_ENCRYPTED		(2)
#define   DVP_DASM_RIGHTCOL_COMMENTS		(3)
#define DVP_DASM_BACKWARD_STEPS				(104)	/* r/w - UINT32 */
#define DVP_DASM_WIDTH						(105)	/* r/w - UINT32 */
#define DVP_DASM_ACTIVE_ADDRESS				(112)	/* r/w - UINT32 */

/* properties available for memory views */
#define DVP_MEM_CPUNUM						(100)	/* r/w - UINT32 */
#define DVP_MEM_EXPRESSION					(101)	/* r/w - const char * */
#define DVP_MEM_TRACK_LIVE					(102)	/* r/w - UINT32 */
#define DVP_MEM_SPACENUM					(103)	/* r/w - UINT32 */
#define DVP_MEM_BYTES_PER_CHUNK				(104)	/* r/w - UINT32 */
#define DVP_MEM_REVERSE_VIEW				(105)	/* r/w - UINT32 */
#define DVP_MEM_ASCII_VIEW					(106)	/* r/w - UINT32 */
#define DVP_MEM_RAW_BASE					(107)	/* r/w - void * */
#define DVP_MEM_RAW_LENGTH					(108)	/* r/w - UINT32 */
#define DVP_MEM_RAW_OFFSET_XOR				(109)	/* r/w - UINT32 */
#define DVP_MEM_RAW_LITTLE_ENDIAN			(110)	/* r/w - UINT32 */
#define DVP_MEM_WIDTH						(111)	/* r/w - UINT32 */
#define DVP_MEM_NO_TRANSLATION				(113)	/* r/w - UINT32 */

/* properties available for textbuffer views */
#define DVP_TEXTBUF_LINE_LOCK				(100)	/* r/w - UINT32 */

/* attribute bits for debug_view_char.attrib */
#define DCA_NORMAL							(0x00)	/* in Windows: black on white */
#define DCA_CHANGED							(0x01)	/* in Windows: red foreground */
#define DCA_SELECTED						(0x02)	/* in Windows: light red background */
#define DCA_INVALID							(0x04)	/* in Windows: dark blue foreground */
#define DCA_DISABLED						(0x08)	/* in Windows: darker foreground */
#define DCA_ANCILLARY						(0x10)	/* in Windows: grey background */
#define DCA_CURRENT							(0x20)	/* in Windows: yellow background */
#define DCA_COMMENT							(0x40)	/* in Windows: green foreground */

/* special characters that can be passed as a DVP_CHARACTER */
#define DCH_UP								(1)		/* up arrow */
#define DCH_DOWN							(2)		/* down arrow */
#define DCH_LEFT							(3)		/* left arrow */
#define DCH_RIGHT							(4)		/* right arrow */
#define DCH_PUP								(5)		/* page up */
#define DCH_PDOWN							(6)		/* page down */
#define DCH_HOME							(7)		/* home */
#define DCH_CTRLHOME						(8)		/* ctrl+home */
#define DCH_END								(9)		/* end */
#define DCH_CTRLEND							(10)	/* ctrl+end */
#define DCH_CTRLRIGHT						(11)	/* ctrl+right */
#define DCH_CTRLLEFT						(12)	/* ctrl+left */


/***************************************************************************
    MACROS
***************************************************************************/



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* opaque structure representing a debug view */
typedef struct _debug_view debug_view;


/* a single "character" in the debug view has an ASCII value and an attribute byte */
struct _debug_view_char
{
	UINT8		byte;
	UINT8		attrib;
};
typedef struct _debug_view_char debug_view_char;


union _debug_property_info
{
	UINT32 i;
	const char *s;
	void *p;
	genf *f;
};
typedef union _debug_property_info debug_property_info;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* initialization */
void				debug_view_init(running_machine *machine);
void				debug_view_exit(running_machine *machine);

/* view creation/deletion */
debug_view *		debug_view_alloc(running_machine *machine, int type);
void				debug_view_free(debug_view *view);

/* property management */
void				debug_view_get_property(debug_view *view, int property, debug_property_info *value);
void				debug_view_set_property(debug_view *view, int property, debug_property_info value);

/* update management */
void				debug_view_begin_update(debug_view *view);
void				debug_view_end_update(debug_view *view);
void				debug_view_update_all(void);
void				debug_view_update_type(int type);

/* misc stuff */
void				debug_disasm_update_all(void);



/***************************************************************************
    INLINE HELPERS
***************************************************************************/

INLINE UINT32 debug_view_get_property_UINT32(debug_view *view, int property)
{
	debug_property_info value;
	debug_view_get_property(view, property, &value);
	return value.i;
}

INLINE void debug_view_set_property_UINT32(debug_view *view, int property, UINT32 value)
{
	debug_property_info info;
	info.i = value;
	debug_view_set_property(view, property, info);
}


INLINE const char *debug_view_get_property_string(debug_view *view, int property)
{
	debug_property_info value;
	debug_view_get_property(view, property, &value);
	return value.s;
}

INLINE void debug_view_set_property_string(debug_view *view, int property, const char *value)
{
	debug_property_info info;
	info.s = value;
	debug_view_set_property(view, property, info);
}


INLINE void *debug_view_get_property_ptr(debug_view *view, int property)
{
	debug_property_info value;
	debug_view_get_property(view, property, &value);
	return value.p;
}

INLINE void debug_view_set_property_ptr(debug_view *view, int property, void *value)
{
	debug_property_info info;
	info.s = value;
	debug_view_set_property(view, property, info);
}


INLINE genf *debug_view_get_property_fct(debug_view *view, int property)
{
	debug_property_info value;
	debug_view_get_property(view, property, &value);
	return value.f;
}

INLINE void debug_view_set_property_fct(debug_view *view, int property, genf *value)
{
	debug_property_info info;
	info.f = value;
	debug_view_set_property(view, property, info);
}

#endif
