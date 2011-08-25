/***************************************************************************

    driver.h

    Driver enumeration helpers.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DRIVER_H__
#define __DRIVER_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* maxima */
#define MAX_DRIVER_NAME_CHARS	8



/* ----- flags for game drivers ----- */

#define ORIENTATION_MASK        		0x00000007
#define GAME_NOT_WORKING				0x00000008
#define GAME_UNEMULATED_PROTECTION		0x00000010	/* game's protection not fully emulated */
#define GAME_WRONG_COLORS				0x00000020	/* colors are totally wrong */
#define GAME_IMPERFECT_COLORS			0x00000040	/* colors are not 100% accurate, but close */
#define GAME_IMPERFECT_GRAPHICS			0x00000080	/* graphics are wrong/incomplete */
#define GAME_NO_COCKTAIL				0x00000100	/* screen flip support is missing */
#define GAME_NO_SOUND					0x00000200	/* sound is missing */
#define GAME_IMPERFECT_SOUND			0x00000400	/* sound is known to be wrong */
#define GAME_SUPPORTS_SAVE				0x00000800	/* game supports save states */
#define GAME_IS_BIOS_ROOT				0x00001000	/* this driver entry is a BIOS root */
#define GAME_NO_STANDALONE				0x00002000	/* this driver cannot stand alone */
#define GAME_REQUIRES_ARTWORK			0x00004000	/* the driver requires external artwork for key elements of the game */
#define GAME_UNOFFICIAL     			0x00008000	/* unofficial hardware change */
#define GAME_NO_SOUND_HW				0x00010000	/* sound hardware not available */
#define GAME_MECHANICAL					0x00020000	/* contains mechanical parts (pinball, redemption games,...) */
#define GAME_TYPE_ARCADE				0x00040000	/* arcade machine (coin operated machines) */
#define GAME_TYPE_CONSOLE				0x00080000	/* console system */
#define GAME_TYPE_COMPUTER				0x00100000	/* any kind of computer including home computers, minis, calcs,... */
#define GAME_TYPE_OTHER					0x00200000	/* any other emulated system that doesn't fit above (ex. clock, satelite receiver,...) */

/* ----- flags to return from video_update ----- */
#define UPDATE_HAS_NOT_CHANGED			0x0001	/* the video has not changed */



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void   (*driver_init_func)(running_machine &machine);


struct game_driver
{
	const char *		source_file;				/* set this to __FILE__ */
	const char *		parent;						/* if this is a clone, the name of the parent */
	const char *		name;						/* short (8-character) name of the game */
	const char *		description;				/* full name of the game */
	const char *		year;						/* year the game was released */
	const char *		manufacturer;				/* manufacturer of the game */
	machine_config_constructor machine_config;		/* machine driver tokens */
	ioport_constructor	ipt;					/* pointer to array of input port tokens */
	void				(*driver_init)(running_machine &machine); /* DRIVER_INIT callback */
	const rom_entry *	rom;						/* pointer to list of ROMs for the game */
	const char *		compatible_with;
	UINT32				flags;						/* orientation and other flags; see defines below */
	const char *		default_layout;				/* default internally defined layout */
};


// driver_list is a purely static class that wraps the global driver list
class driver_list
{
	DISABLE_COPYING(driver_list);

protected:
	// construction/destruction
	driver_list();

public:
	// getters
	static int total() { return s_driver_count; }

	// any item by index
	static const game_driver &driver(int index) { assert(index >= 0 && index < s_driver_count); return *s_drivers_sorted[index]; }
	static int clone(int index) { return find(driver(index).parent); }
	static int non_bios_clone(int index) { int result = find(driver(index).parent); return (result != -1 && (driver(result).flags & GAME_IS_BIOS_ROOT) == 0) ? result : -1; }
	static int compatible_with(int index) { return find(driver(index).compatible_with); }

	// any item by driver
	static int clone(const game_driver &driver) { int index = find(driver); assert(index != -1); return clone(index); }
	static int non_bios_clone(const game_driver &driver) { int index = find(driver); assert(index != -1); return non_bios_clone(index); }
	static int compatible_with(const game_driver &driver) { int index = find(driver); assert(index != -1); return compatible_with(index); }

	// general helpers
	static int find(const char *name);
	static int find(const game_driver &driver) { return find(driver.name); }

	// static helpers
	static bool matches(const char *wildstring, const char *string);

protected:
	// internal helpers
	static int driver_sort_callback(const void *elem1, const void *elem2);
	static int penalty_compare(const char *source, const char *target);

	// internal state
	static int							s_driver_count;
	static const game_driver * const	s_drivers_sorted[];
};


// driver_enumerator enables efficient iteration through the driver list
class driver_enumerator : public driver_list
{
	DISABLE_COPYING(driver_enumerator);

public:
	// construction/destruction
	driver_enumerator(emu_options &options);
	driver_enumerator(emu_options &options, const char *filter);
	driver_enumerator(emu_options &options, const game_driver &filter);
	~driver_enumerator();

	// getters
	int count() const { return m_filtered_count; }
	int current() const { return m_current; }
	emu_options &options() const { return m_options; }

	// current item
	const game_driver &driver() const { return driver_list::driver(m_current); }
	machine_config &config() const { return config(m_current); }
	int clone() { return driver_list::clone(m_current); }
	int non_bios_clone() { return driver_list::non_bios_clone(m_current); }
	int compatible_with() { return driver_list::compatible_with(m_current); }
	void include() { include(m_current); }
	void exclude() { exclude(m_current); }

	// any item by index
	bool included(int index) const { assert(index >= 0 && index < s_driver_count); return m_included[index]; }
	bool excluded(int index) const { assert(index >= 0 && index < s_driver_count); return !m_included[index]; }
	machine_config &config(int index) const;
	void include(int index) { assert(index >= 0 && index < s_driver_count); if (!m_included[index]) { m_included[index] = true; m_filtered_count++; }  }
	void exclude(int index) { assert(index >= 0 && index < s_driver_count); if (m_included[index]) { m_included[index] = false; m_filtered_count--; } }
	using driver_list::driver;
	using driver_list::clone;
	using driver_list::non_bios_clone;
	using driver_list::compatible_with;

	// filtering/iterating
	int filter(const char *string = NULL);
	int filter(const game_driver &driver);
	void include_all();
	void exclude_all() { memset(m_included, 0, sizeof(m_included[0]) * s_driver_count); m_filtered_count = 0; }
	void reset() { m_current = -1; }
	bool next();
	bool next_excluded();

	// general helpers
	void set_current(int index) { assert(index >= -1 && index <= s_driver_count); m_current = index; }
	void find_approximate_matches(const char *string, int count, int *results);

private:
	// entry in the config cache
	struct config_entry
	{
		friend class simple_list<config_entry>;

	public:
		// construction/destruction
		config_entry(machine_config &config, int index);
		~config_entry();

		// getters
		config_entry *next() const { return m_next; }
		int index() const { return m_index; }
		machine_config *config() const { return m_config; }

	private:
		// internal state
		config_entry *		m_next;
		machine_config *	m_config;
		int					m_index;
	};

	static const int CONFIG_CACHE_COUNT = 100;

	// internal state
	int					m_current;
	int					m_filtered_count;
	emu_options &		m_options;
	UINT8 *				m_included;
	machine_config **	m_config;
	mutable simple_list<config_entry> m_config_cache;
};



/***************************************************************************
    MACROS FOR BUILDING GAME DRIVERS
***************************************************************************/


#define DRIVER_INIT_NAME(name)		driver_init_##name
#define DRIVER_INIT(name)			void DRIVER_INIT_NAME(name)(running_machine &machine)
#define DRIVER_INIT_CALL(name)		DRIVER_INIT_NAME(name)(machine)

#define driver_init_0				NULL


#define GAME_NAME(name) driver_##name
#define GAME_EXTERN(name) extern const game_driver GAME_NAME(name)

#define GAME(YEAR,NAME,PARENT,MACHINE,INPUT,INIT,MONITOR,COMPANY,FULLNAME,FLAGS)	\
	GAMEL(YEAR,NAME,PARENT,MACHINE,INPUT,INIT,MONITOR,COMPANY,FULLNAME,FLAGS,((const char *)0))

#define GAMEL(YEAR,NAME,PARENT,MACHINE,INPUT,INIT,MONITOR,COMPANY,FULLNAME,FLAGS,LAYOUT)	\
extern const game_driver GAME_NAME(NAME) =	\
{											\
	__FILE__,								\
	#PARENT,								\
	#NAME,									\
	FULLNAME,								\
	#YEAR,									\
	COMPANY,								\
	MACHINE_CONFIG_NAME(MACHINE),			\
	INPUT_PORTS_NAME(INPUT),				\
	DRIVER_INIT_NAME(INIT),					\
	ROM_NAME(NAME),							\
	NULL,									\
	(MONITOR)|(FLAGS)|GAME_TYPE_ARCADE,		\
	&LAYOUT[0]								\
};

#define CONS(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,INIT,COMPANY,FULLNAME,FLAGS)	\
extern const game_driver GAME_NAME(NAME) =	\
{											\
	__FILE__,								\
	#PARENT,								\
	#NAME,									\
	FULLNAME,								\
	#YEAR,									\
	COMPANY,								\
	MACHINE_CONFIG_NAME(MACHINE),			\
	INPUT_PORTS_NAME(INPUT),				\
	DRIVER_INIT_NAME(INIT),					\
	ROM_NAME(NAME),							\
	#COMPAT,								\
	ROT0|(FLAGS)|GAME_TYPE_CONSOLE,			\
	NULL									\
};

#define COMP(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,INIT,COMPANY,FULLNAME,FLAGS)	\
extern const game_driver GAME_NAME(NAME) =	\
{											\
	__FILE__,								\
	#PARENT,								\
	#NAME,									\
	FULLNAME,								\
	#YEAR,									\
	COMPANY,								\
	MACHINE_CONFIG_NAME(MACHINE),			\
	INPUT_PORTS_NAME(INPUT),				\
	DRIVER_INIT_NAME(INIT),					\
	ROM_NAME(NAME),							\
	#COMPAT,								\
	ROT0|(FLAGS)|GAME_TYPE_COMPUTER,		\
	NULL									\
};

#define SYST(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,INIT,COMPANY,FULLNAME,FLAGS)	\
extern const game_driver GAME_NAME(NAME) =	\
{											\
	__FILE__,								\
	#PARENT,								\
	#NAME,									\
	FULLNAME,								\
	#YEAR,									\
	COMPANY,								\
	MACHINE_CONFIG_NAME(MACHINE),			\
	INPUT_PORTS_NAME(INPUT),				\
	DRIVER_INIT_NAME(INIT),					\
	ROM_NAME(NAME),							\
	#COMPAT,								\
	ROT0|(FLAGS)|GAME_TYPE_OTHER,		\
	NULL									\
};

/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

GAME_EXTERN(___empty);



#endif
