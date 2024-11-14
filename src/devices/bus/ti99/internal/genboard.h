// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Geneve main board components.
    See genboard.c for documentation.

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/
#ifndef MAME_BUS_TI99_INTERNAL_GENBOARD_H
#define MAME_BUS_TI99_INTERNAL_GENBOARD_H

#pragma once

#include "bus/ti99/peb/peribox.h"
#include "video/v9938.h"
#include "cpu/tms9900/tms9995.h"
#include "sound/sn76496.h"
#include "machine/mm58274c.h"
#include "machine/at29x.h"
#include "machine/ram.h"

enum
{
	GENEVE_GM_TURBO = 1,
	GENEVE_GM_TIM = 2
};

enum
{
	GENEVE_EPROM = 0,
	GENEVE_PFM512,
	GENEVE_PFM512A
};

#define GENEVE_GATE_ARRAY_TAG     "gatearray"
#define GENMOD_DECODER_TAG     "genmod"
#define GENEVE_PAL_TAG     "pal"
#define GENEVE_MOUSE_TAG      "gmouse"
#define GENEVE_PFM512_TAG      "pfm512"
#define GENEVE_PFM512A_TAG     "pfm512a"

namespace bus::ti99::internal {

/*****************************************************************************/

class geneve_pal_device;
class genmod_decoder_device;

class geneve_gate_array_device : public device_t
{
	// friend class genmod_decoder_device;

public:
	geneve_gate_array_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Set the internal state and output lines according to the address
	void setaddress(offs_t offset, uint8_t busctrl);

	// Access to Gate Array-internal registers. Depends on the previous
	// call of setaddress
	// - Mapper
	// - Keyboard
	// - Grom address counter
	// - Cartridge bank switch
	void readz(uint8_t& value);
	void write(uint8_t data);

	// Inputs
	void clock_in(int state);
	void dbin_in(int state);
	void extready_in(int state);
	void sndready_in(int state);
	void cru_sstep_write(offs_t offset, uint8_t data);
	void cru_ctrl_write(offs_t offset, uint8_t data);

	// Outputs
	int csr_out();
	int csw_out();
	int rtcen_out();
	int romen_out();
	int ramen_out();
	int ramenx_out();
	int snden_out();
	int dben_out();
	int gaready_out();

	int get_prefix(int lines);
	bool accessing_dram();
	static bool accessing_dram_s(int function);
	static bool accessing_sram_s(int function);
	static bool accessing_box_s(int function, bool genmod);
	static bool accessing_devs_s(int function);

	bool accessing_grom();
	offs_t get_dram_address();

	// Keyboard support
	void set_keyboard_clock(int state);
	void enable_shift_register(int state);
	void kbdclk(int state);
	void kbddata(int state);
	auto kbdint_cb() { return m_keyint.bind(); }
	auto kbdclk_cb() { return m_keyb_clk.bind(); }
	auto kbddata_cb() { return m_keyb_data.bind(); }

	// Miscellaneous
	void set_debug(bool deb) { m_debug = deb; }
	bool geneve_mode() { return m_geneve_mode; }
	int  get_function() { return m_debug? m_decdebug.function : m_decoded.function; }

private:
	geneve_gate_array_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	void    common_reset();

	// Mapper function
	typedef struct
	{
		bool    read;           // Reading
		int     function;       // must be a fundamental type to be saveable
		offs_t  offset;         // Logical address
		int     physpage;       // Physical page
	} decdata;

	// Functions
	void    decode_logical(decdata* dec);
	void    get_page(decdata* dec);
	void    decode_physical(decdata* dec);

	/*
	Constants for mapper decoding. Naming scheme:
	M=Mapper, L=Logical space; P=Physical space
	*/
	typedef enum
	{
		MUNDEF=0,

		MLVIDEO,
		MLMAPPER,
		MLKEY,
		MLSOUND,
		MLCLOCK,
		MLEXT,
		MLGROM,
		MLGROMAD,
		MLCARTROM,
		MLCARTBANK,

		MPDRAM,
		MPEXP,
		MPEPROM,
		MPSRAM,
		MPSRAMX,
		MPSRAMF,
		MBOX,

		CARTPROT

	} decfunct_t;

	void    device_start() override;
	virtual void device_reset() override ATTR_COLD;

	// Wait state creation
	bool    m_have_waitstate;
	bool    m_have_extra_waitstate;
	bool    m_enable_extra_waitstates;
	bool    m_extready;
	bool    m_sndready;

	// Cartridge and GROM support
	int     m_grom_address;
	bool    m_cartridge_banked;
	bool    m_cartridge_secondpage;
	bool    m_cartridge6_writable;
	bool    m_cartridge7_writable;
	bool    m_load_lsb;
	void    increase_grom_address();

	// Global modes
	bool    m_geneve_mode;
	bool    m_direct_mode;

	// Mapper file
	int     m_map[8];

	// The result of decoding
	decdata m_decoded;
	decdata m_decdebug;

	// ====== Decoding ============

	// Static decoder entry for the logical space
	// Not all entries apply for native mode, e.g. there is no GROM in native
	// mode. In that case the base and mask are 0000, and the entry must be
	// skipped. Speech is accessible in the physical space in native mode.
	// All entries have a wait state count of 1.
	typedef struct
	{
		offs_t      genbase;       // Base address in native mode
		int         genmask;       // Bits that also match this entry
		offs_t      tibase;        // Base address in TI mode
		int         timask;        // Bits that also match this entry
		int         writeoff;      // Additional offset in TI mode for writing
		decfunct_t  function;      // Decoded function
		const char* description;   // Good for logging
	} logentry_t;

	// Static decoder entry for the physical space
	// There are no differences between native mode and TI mode.
	typedef struct
	{
		offs_t      base;           // Base page
		int         mask;           // Bits that also match this entry
		decfunct_t  function;       // Decoded function
		const char* description;    // Good for logging
	} physentry_t;

	// Tables
	static const geneve_gate_array_device::logentry_t s_logmap[];
	static const geneve_gate_array_device::physentry_t s_physmap[];

	// Keyboard support
	devcb_write_line    m_keyint;
	devcb_write_line    m_keyb_clk;
	devcb_write_line    m_keyb_data;
	uint16_t            m_keyboard_shift_reg;
	line_state          m_keyboard_last_clock;
	line_state          m_keyboard_data_in;
	bool                m_shift_reg_enabled;
	void                shift_reg_changed();

	// Devices
	required_device<geneve_pal_device>                  m_pal;
	required_device<bus::ti99::peb::peribox_device>     m_peribox;

	// Emulation-specific: Is the debugger active?
	bool    m_debug;
};

/*****************************************************************************/

/*
   PAL circuit, controlling the READY line to the CPU and MEMEN/WE
   to the peribox
*/

class geneve_pal_device : public device_t
{
public:
	geneve_pal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void gaready_in(int state);
	void csw_in(int state);
	void csr_in(int state);
	void memen(int state);
	void vwaiten(int state);   // pin 19
	void sysspeed(int state);  // pin 5
	void clock_in(int state);

	auto ready_cb() { return m_ready.bind(); }

private:
	void device_start() override ATTR_COLD;
	void set_ready();

	// Pins
	bool m_pin3;
	bool m_pin4;
	bool m_pin5;
	bool m_pin9;
	bool m_pin19;
	bool m_pin14d,m_pin14q;
	bool m_pin15d,m_pin15q;
	bool m_pin16d,m_pin16q;
	bool m_pin17d,m_pin17q;

	// Debugging
	int  m_prev_ready;

	required_device<bus::ti99::peb::peribox_device> m_peribox;

	// Ready line to the CPU
	devcb_write_line m_ready;
};

class genmod_decoder_device : public device_t
{
public:
	genmod_decoder_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Set the internal state and output lines according to the address
	void set_function(int func, int page);
	void set_debug(bool deb) { m_debug = deb; }
	void set_turbo(bool turbo) { m_turbo = turbo; }
	void set_timode(bool timode) { m_timode = timode; }

	int gaready_out();
	int dben_out();
	void gaready_in(int state);
	void extready_in(int state);
	void sndready_in(int state);

private:
	void device_start() override ATTR_COLD;

	// Emulation-specific: Is the debugger active?
	bool    m_debug;
	bool    m_turbo;
	bool    m_timode;
	int     m_function;
	int     m_function_debug;
	int     m_page;
	int     m_page_debug;
	int     m_gaready;
	int     m_extready;
	int     m_sndready;
};

} // end namespace bus::ti99::internal

DECLARE_DEVICE_TYPE_NS(GENEVE_GATE_ARRAY,   bus::ti99::internal, geneve_gate_array_device)
DECLARE_DEVICE_TYPE_NS(GENEVE_PAL,          bus::ti99::internal, geneve_pal_device)
DECLARE_DEVICE_TYPE_NS(GENMOD_DECODER,      bus::ti99::internal, genmod_decoder_device)

#endif // MAME_BUS_TI99_INTERNAL_GENBOARD_H
