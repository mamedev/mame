/*****************************************************************************
 *
 * includes/msx_slot.h
 *
 ****************************************************************************/

#ifndef MSX_SLOT_H_
#define MSX_SLOT_H_


enum {
	MSX_LAYOUT_SLOT_ENTRY,
	MSX_LAYOUT_KANJI_ENTRY,
	MSX_LAYOUT_RAMIO_SET_BITS_ENTRY,
	MSX_LAYOUT_LAST
};

typedef struct {
	int entry;
	int type;
	int slot_primary, slot_secondary, slot_page, page_extent;
	int size, option;
} msx_slot_layout;

#define MSX_LAYOUT_INIT(msx) \
static const msx_slot_layout msx_slot_layout_##msx[] = {

#define MSX_LAYOUT_SLOT(prim, sec, page, extend, type, size, option) \
	{								\
		MSX_LAYOUT_SLOT_ENTRY,		\
		SLOT_##type,				\
		prim,						\
		sec,						\
		page,						\
		extend,						\
		size,						\
		option						\
	},

#define MSX_LAYOUT_KANJI(offset) \
	{								\
		MSX_LAYOUT_KANJI_ENTRY,		\
		SLOT_EMPTY,					\
		0,							\
		0,							\
		0,							\
		0,							\
		0,							\
		offset						\
	},

#define MSX_LAYOUT_RAMIO_SET_BITS(offset) \
	{								\
		MSX_LAYOUT_RAMIO_SET_BITS_ENTRY,		\
		SLOT_EMPTY,					\
		0,							\
		0,							\
		0,							\
		0,							\
		0,							\
		offset						\
	},

#define MSX_LAYOUT_END \
	{								\
		MSX_LAYOUT_LAST,			\
		SLOT_END,					\
		0,							\
		0,							\
		0,							\
		0,							\
		0,							\
		0							\
	}								\
};


enum msx_slot_type {
	SLOT_EMPTY = 0,
	SLOT_MSXDOS2,
	SLOT_KONAMI_SCC,
	SLOT_KONAMI,
	SLOT_ASCII8,
	SLOT_ASCII16,
	SLOT_GAMEMASTER2,
	SLOT_ASCII8_SRAM,
	SLOT_ASCII16_SRAM,
	SLOT_RTYPE,
	SLOT_MAJUTSUSHI,
	SLOT_FMPAC,
	SLOT_SUPERLOADRUNNER,
	SLOT_SYNTHESIZER,
	SLOT_CROSS_BLAIM,
	SLOT_DISK_ROM,
	SLOT_KOREAN_80IN1,
	SLOT_KOREAN_126IN1,
	SLOT_KOREAN_90IN1,
	SLOT_LAST_CARTRIDGE_TYPE = SLOT_KOREAN_90IN1,
	SLOT_SOUNDCARTRIDGE,
	SLOT_ROM,
	SLOT_RAM,
	SLOT_RAM_MM,
	SLOT_CARTRIDGE1,
	SLOT_CARTRIDGE2,
        SLOT_DISK_ROM2,
	SLOT_END
};

enum msx_mem_type {
	MSX_MEM_ROM,
	MSX_MEM_RAM,
	MSX_MEM_HANDLER
};

typedef struct {
	int m_type;
	int m_start_page;
	int m_bank_mask;
	int m_banks[4];
	int m_size;
	UINT8 *m_mem;
	const char *m_sramfile;
	union {
		struct {
			UINT8 *mem;
			int sram_support;
			int sram_active;
			int opll_active;
		} fmpac;
		struct {
			int active;
		} scc;
		struct {
			UINT8 *mem;
			int sram_mask;
			int empty_mask;
		} sram;
		struct {
			int scc_active;
			int sccp_active;
			int ram_mode[4];
			int banks_saved[4];
			int mode;
		} sccp;
	} m_cart;
} slot_state;

typedef struct {
	int slot_type;
	int mem_type;
	char name[32];
	int (*init)(running_machine &machine, slot_state*, int page, UINT8 *mem, int size);
	void (*reset)(running_machine &machine, slot_state*);
	void (*map)(running_machine &machine, slot_state*, int page);
	void (*write)(running_machine &machine, slot_state*, UINT16, UINT8);
	int (*loadsram)(running_machine &machine, slot_state*);
	int (*savesram)(running_machine &machine, slot_state*);
} msx_slot;

extern const msx_slot msx_slot_list[];

#define MSX_SLOT_START \
const msx_slot msx_slot_list[] = {

#define MSX_SLOT_ROM(type, ent) { \
	type,							\
	MSX_MEM_ROM,					\
	#type,							\
	slot_##ent##_init,				\
	slot_##ent##_reset,				\
	slot_##ent##_map,				\
	NULL,							\
	NULL,							\
	NULL							\
},

#define MSX_SLOT_RAM(type, ent) { \
	type,							\
	MSX_MEM_RAM,					\
	#type,							\
	slot_##ent##_init,				\
	slot_##ent##_reset,				\
	slot_##ent##_map,				\
	NULL,							\
	NULL,							\
	NULL							\
},

#define MSX_SLOT(type, ent) { \
	type,							\
	MSX_MEM_HANDLER,				\
	#type,							\
	slot_##ent##_init,				\
	slot_##ent##_reset,				\
	slot_##ent##_map,				\
	slot_##ent##_write,				\
	NULL,							\
	NULL							\
},

#define MSX_SLOT_SRAM(type, ent) { \
	type,							\
	MSX_MEM_HANDLER,				\
	#type,							\
	slot_##ent##_init,				\
	slot_##ent##_reset,				\
	slot_##ent##_map,				\
	slot_##ent##_write,				\
	slot_##ent##_loadsram,			\
	slot_##ent##_savesram			\
},

#define MSX_SLOT_NULL(type) { \
	type,							\
	MSX_MEM_ROM,					\
	#type,							\
	NULL,							\
	NULL,							\
	NULL,							\
	NULL,							\
	NULL,							\
	NULL							\
},

#define MSX_SLOT_END \
	{ SLOT_END, 0, "", NULL, NULL, NULL, NULL, NULL } \
};

#define MSX_SLOT_INIT(nm)			 static int \
	slot_##nm##_init (running_machine &machine, slot_state *state, int page, UINT8 *mem, int size)
#define MSX_SLOT_MAP(nm)			\
	static void slot_##nm##_map (running_machine &machine, slot_state *state, int page)
#define MSX_SLOT_WRITE(nm)			\
	static void slot_##nm##_write (running_machine &machine, slot_state *state, UINT16 addr, UINT8 val)
#define MSX_SLOT_RESET(nm)			\
	static void slot_##nm##_reset (running_machine &machine, slot_state *state)
#define MSX_SLOT_LOADSRAM(nm)		\
	static int slot_##nm##_loadsram (running_machine &machine, slot_state *state)
#define MSX_SLOT_SAVESRAM(nm)		\
	static int slot_##nm##_savesram (running_machine &machine, slot_state *state)

typedef struct {
	char name[9];
	const msx_slot_layout *layout;
} msx_driver_struct;

extern const msx_driver_struct msx_driver_list[];

#define MSX_DRIVER_LIST		\
const msx_driver_struct msx_driver_list[] = {
#define MSX_DRIVER(foo)		\
		{ #foo, msx_slot_layout_##foo },
#define MSX_DRIVER_END		\
		{ "", NULL }		\
};


#endif /* MSX_SLOT_H_ */
