// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99/8 main board logic

    This component implements the address decoder and mapper logic from the
    TI-99/8 console.

    See 998board.c for documentation

    Michael Zapf

*****************************************************************************/

#ifndef MAME_BUS_TI99_INTERNAL_998BOARD_H
#define MAME_BUS_TI99_INTERNAL_998BOARD_H

#pragma once

#include "bus/ti99/gromport/gromport.h"
#include "bus/hexbus/hexbus.h"

#include "bus/ti99/internal/ioport.h"
#include "machine/ram.h"
#include "machine/tmc0430.h"
#include "machine/tms9901.h"
#include "machine/tms6100.h"
#include "sound/sn76496.h"
#include "sound/tms5220.h"
#include "video/tms9928a.h"

// -------------- Defines ------------------------------------

#define TI998_SRAM_TAG       "sram8"
#define TI998_DRAM_TAG       "dram8"
#define TI998_MAPPER_TAG     "mapper"
#define TI998_MAINBOARD_TAG  "mainboard8"
#define TI998_SPEECHSYN_TAG  "speech"
#define TI998_SOUNDCHIP_TAG  "soundchip"
#define TI998_TMS9901_TAG    "tms9901"
#define TI998_VDP_TAG        "vdp"
#define TI998_HEXBUS_TAG     "hexbus"

#define TI998_ROM0_REG        "rom0_region"
#define TI998_ROM1_REG        "rom1_region"
#define TI998_PASCAL_REG      "pascal_region"
#define TI998_SYSGROM_REG     "sysgrom_region"
#define TI998_GROMLIB1_REG    "gromlib1_region"
#define TI998_GROMLIB2_REG    "gromlib2_region"
#define TI998_GROMLIB3_REG    "gromlib3_region"
#define TI998_SPEECHROM_REG       "speech_region"

#define TI998_GROMLIB_TAG "gromlib"
#define TI998_SYSGROM_TAG TI998_GROMLIB_TAG "0"
#define TI998_SYSGROM0_TAG TI998_SYSGROM_TAG "_0"
#define TI998_SYSGROM1_TAG TI998_SYSGROM_TAG "_1"
#define TI998_SYSGROM2_TAG TI998_SYSGROM_TAG "_2"

#define TI998_GLIB1_TAG TI998_GROMLIB_TAG "1"
#define TI998_GLIB10_TAG TI998_GLIB1_TAG "_0"
#define TI998_GLIB11_TAG TI998_GLIB1_TAG "_1"
#define TI998_GLIB12_TAG TI998_GLIB1_TAG "_2"
#define TI998_GLIB13_TAG TI998_GLIB1_TAG "_3"
#define TI998_GLIB14_TAG TI998_GLIB1_TAG "_4"
#define TI998_GLIB15_TAG TI998_GLIB1_TAG "_5"
#define TI998_GLIB16_TAG TI998_GLIB1_TAG "_6"
#define TI998_GLIB17_TAG TI998_GLIB1_TAG "_7"

#define TI998_GLIB2_TAG TI998_GROMLIB_TAG "2"
#define TI998_GLIB20_TAG TI998_GLIB2_TAG "_0"
#define TI998_GLIB21_TAG TI998_GLIB2_TAG "_1"
#define TI998_GLIB22_TAG TI998_GLIB2_TAG "_2"
#define TI998_GLIB23_TAG TI998_GLIB2_TAG "_3"
#define TI998_GLIB24_TAG TI998_GLIB2_TAG "_4"
#define TI998_GLIB25_TAG TI998_GLIB2_TAG "_5"
#define TI998_GLIB26_TAG TI998_GLIB2_TAG "_6"
#define TI998_GLIB27_TAG TI998_GLIB2_TAG "_7"

#define TI998_GLIB3_TAG TI998_GROMLIB_TAG "3"
#define TI998_GLIB30_TAG TI998_GLIB3_TAG "_0"
#define TI998_GLIB31_TAG TI998_GLIB3_TAG "_1"
#define TI998_GLIB32_TAG TI998_GLIB3_TAG "_2"

#define TI998_VAQUERRO_TAG "vaquerro"
#define TI998_MOFETTA_TAG "mofetta"
#define TI998_AMIGO_TAG "amigo"
#define TI998_OSO_TAG "oso"

// --------------------------------------------------

namespace bus::ti99::internal {

class mainboard8_device;

/*
    Custom chip: Vaquerro
*/
class vaquerro_device : public device_t
{
public:
	vaquerro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

	line_state ready();
	void treset();

	void set_address(offs_t offset, int state);

	int sprd_out();
	int spwt_out();
	int sccs_out();
	int sromcs_out();

	// Collective select line query
	int gromcs_out();

	int vdprd_out();
	int vdpwt_out();
	int lascsq_out();
	int ggrdy_out();
	void hold_cpu(int state);

	void crus_in(int state);
	void crusgl_in(int state);
	void clock_in(int state);
	void memen_in(int state);

	void sgmry(int state);
	void tsgry(int state);
	void p8gry(int state);
	void p3gry(int state);

private:
	/*
	    Wait state generator (part of Vaquerro)
	*/
	class waitstate_generator
	{
	public:
		waitstate_generator() :
			m_counting(false),
			m_generate(false),
			m_counter(0),
			m_addressed(true),
			m_ready(true)
		{
		}

		virtual ~waitstate_generator() { }
		void select_in(bool addressed);
		virtual void ready_in(line_state ready) = 0;
		virtual void clock_in(line_state clkout) = 0;
		void treset_in(line_state reset);

		int select_out();
		void init(int select_value) { m_selvalue = select_value; }

		line_state ready_out();

		bool is_counting();
		bool is_generating();
		bool is_ready();

	protected:
		// Two flipflops
		bool m_counting = false;
		bool m_generate = false;
		// Counter
		int  m_counter = 0;

		// Select value (indicates selected line)
		int  m_selvalue = 0;

		// Line state flags
		bool m_addressed = false;
		bool m_ready = false;
	};

	class grom_waitstate_generator : public waitstate_generator
	{
	public:
		void ready_in(line_state ready) override;
		void clock_in(line_state clkout) override;
	};

	class video_waitstate_generator : public waitstate_generator
	{
	public:
		void ready_in(line_state ready) override { }
		void clock_in(line_state clkout) override;
	};

	// Memory cycle state
	bool m_memen = false;

	// Waiting for video
	bool m_video_wait = false;

	// State of the CRUS line
	int m_crus = ASSERT_LINE;

	// Are the GROM libraries turned on?
	bool m_crugl = ASSERT_LINE;

	// Do we have a logical address space match?
	bool m_lasreq = false;

	// Keep the decoding result (opens the SRY gate)
	bool m_grom_or_video = false;

	// Select lines
	bool m_spwt = false;
	bool m_sccs = false;
	bool m_sromcs = false;
	bool m_sprd = false;
	bool m_vdprd = CLEAR_LINE;
	bool m_vdpwt = CLEAR_LINE;

	// Collective GROM select state
	int m_gromsel = 0;

	// Outgoing READY
	int m_ggrdy = ASSERT_LINE;

	// Outgoing READY latch (common flipflop driving SRY)
	bool m_sry = false;

	// Holds the A14 address line state. We need this for the clock_in method.
	int m_a14 = 0;

	// Keeps the recent DBIN level
	int m_dbin_level = 0;

	// Wait state logic components
	grom_waitstate_generator m_sgmws, m_tsgws, m_p8gws, m_p3gws;
	video_waitstate_generator m_vidws;

	// Pointer to mainboard
	mainboard8_device* m_mainboard;
};

/*
    Custom chip: Mofetta
*/
class mofetta_device : public device_t
{
public:
	mofetta_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

	void cruwrite(offs_t offset, uint8_t data);
	void set_address(offs_t offset, int state);

	// Debugger support
	bool hexbus_access_debug();
	bool intdsr_access_debug();

	void clock_in(int state);
	void msast_in(int state);
	void lascs_in(int state);
	void pmemen_in(int state);
	void skdrcs_in(int state);

	int gromclk_out();

	int alccs_out();
	int prcs_out();
	int cmas_out();
	int dbc_out();

	int rom1cs_out();
	int rom1am_out();
	int rom1al_out();

private:
	// Memory cycle state
	bool m_pmemen = false;

	// Logical access
	bool m_lasreq = false;

	// DRAM access
	bool m_skdrcs = false;

	// Indicates the UP level of the GROMCLK
	bool m_gromclk_up = false;

	// Have we got the upper word of the address?
	bool m_gotfirstword = false;

	// Address latch
	int m_address_latch = 0;

	// Most significant byte of the 24-bit address
	int m_prefix = 0;

	// CRU select of the 1700 device
	bool m_alcpg = false;

	// CRU select of the 2700 device
	bool m_txspg = false;

	// ROM1 select lines
	bool m_rom1cs = false;
	bool m_rom1am = false;
	bool m_rom1al = false;

	// OSO select
	bool m_alccs = false;

	// Pascal ROM select line
	bool m_prcs = false;

	// Cartridge port select line
	bool m_cmas = false;

	// GROM clock count (as frequency divider)
	int m_gromclock_count = 0;

	// Remember last msast state for edge detection
	int m_msast = 0;

	// Pointer to mainboard
	mainboard8_device* m_mainboard;
};

/*
    Custom chip: Amigo
*/
class amigo_device : public device_t
{
public:
	amigo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

	uint8_t read();
	void write(uint8_t data);
	uint8_t set_address(offs_t offset);

	// Debugger support
	int get_physical_address_debug(offs_t offset);
	void mapper_access_debug(int data);

	void srdy_in(int state);
	void clock_in(int state);
	void crus_in(int state);
	void lascs_in(int state);
	void memen_in(int state);

	void holda_in(int state);

	int cpury_out();
	int sramcs_out();
	int skdrcs_out();

	void connect_sram(uint8_t* sram) { m_sram = sram; }
	bool mapper_accessed() { return m_mapper_accessed; }

private:
	// Memory cycle state
	bool m_memen = false;

	// DMA methods for loading/saving maps
	void mapper_load();
	void mapper_save();

	// Address mapper registers. Each offset is selected by the first 4 bits
	// of the logical address.
	uint32_t  m_base_register[16];

	// Indicates a logical space access
	bool m_logical_space = true;

	// Physical address
	uint32_t  m_physical_address = 0;

	// Pointer to SRAM where AMIGO needs to upload/download its map values
	uint8_t* m_sram = nullptr;

	// Pointer to mainboard
	mainboard8_device* m_mainboard;

	// Keep the system ready state
	int m_srdy = 0;

	// Outgoing READY level
	int m_ready_out = 0;

	// Keep the CRUS setting
	int m_crus = 0;

	// State of the address creation
	int m_amstate = 0;

	// Protection flags
	int m_protflag = 0;

	// Accessing SRAM
	bool m_sram_accessed = false;

	// Accessing DRAM
	bool m_dram_accessed = false;

	// Accessing the mapper
	bool m_mapper_accessed = false;

	// HOLDA flag
	bool m_hold_acknowledged = false;

	// Address in SRAM during DMA
	uint32_t  m_sram_address = 0;

	// Number of the currently loaded/save base register
	int m_basereg = 0;

	// Latched value for mapper DMA transfer
	uint32_t m_mapvalue = 0;
};

/*
    Custom chip: OSO
*/

/* Status register bits */
typedef enum
{
	HSKWT = 0x80,
	HSKRD = 0x40,
	BAVIAS = 0x20,
	BAVAIS = 0x10,
	SBAV = 0x08,
	WBUSY = 0x04,
	RBUSY = 0x02,
	SHSK = 0x01
} oso_status;

/* Control register bits */
typedef enum
{
	WIEN = 0x80,
	RIEN = 0x40,
	BAVIAEN = 0x20,
	BAVAIEN = 0x10,
	BAVC = 0x08,
	WEN = 0x04,
	REN = 0x02,
	CR7 = 0x01
} oso_control;

class oso_device : public bus::hexbus::hexbus_chained_device
{
public:
	oso_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	void device_start() override ATTR_COLD;
	void hexbus_value_changed(uint8_t data) override;

	void clock_in(int state);

	// INT line
	devcb_write_line m_int;

private:
	bool control_bit(oso_control bit) { return (m_control & bit)!=0; }
	bool status_bit(oso_status bit) { return (m_status & bit)!=0; }
	void clear_int_status();
	void set_status(int bit, bool set) { m_status = (set)? (m_status | bit) : (m_status & ~bit); }
	void update_hexbus();

	required_device<bus::hexbus::hexbus_device> m_hexbusout;

	// Registers
	uint8_t m_data = 0;
	uint8_t m_status = 0;
	uint8_t m_control = 0;
	uint8_t m_xmit = 0;

	bool m_bav = false;         // Bus available; when true, a communication is about to happen
	bool m_sbav = false;        // Stable BAV; the BAV signal is true for two clock cycles
	bool m_sbavold = false;     // Old SBAV state
	bool m_bavold = false;

	bool m_hsk = false;         // Handshake line; when true, a bus member needs more time to process the message
	bool m_hsklocal = false;    // Local level of HSK
	bool m_shsk = false;        // Stable HSK
	bool m_hskold = false;

	// Page 3 in OSO schematics: Write timing
	bool m_wq1 = false;         // Flipflop 1
	bool m_wq1old = false;      // Previous state
	bool m_wq2 = false;         // Flipflop 2
	bool m_wq2old = false;      // Previous state
	bool m_wnp = false;         // Write nibble selection; true means upper 4 bits
	bool m_wbusy = false;       // When true, direct the transmit register towards the output
	bool m_wbusyold = false;    // Old state of the WBUSY line
	bool m_sendbyte = false;    // Byte has been loaded into the XMIT register
	bool m_wrset = false;       // Start sending
	bool m_counting = false;    // Counter running
	int m_clkcount = 0;     // Counter for 30 cycles

	// Page 4 in OSO schematics: Read timing
	bool m_rq1 = false;         // Flipflop 1
	bool m_rq2 = false;         // Flipflop 2
	bool m_rq2old = false;      // Previous state
	bool m_rnib = false;        // Read nibble, true means upper 4 bits
	bool m_rnibcold = false;    // Needed to detect the raising edge
	bool m_rdset = false;       // Start reading
	bool m_rdsetold = false;    // Old state
	bool m_msns = false;        // Upper 4 bits
	bool m_lsns = false;        // Lower 4 bits

	// Page 6 (RHSUS*)
	bool m_rhsus = false;       // Needed to assert the HSK line until the CPU has read the byte

	bool m_rbusy = false;

	bool m_phi3 = false;

	// Debugging help
	uint8_t m_oldvalue = 0;
/*
    // This is a buffer to enqueue changes on the Hexbus
    // This is not part of the real implementation, but in the emulation
    // there is no guarantee that two subsequent bus changes will be sensed
    // on the other side

    void enqueue(uint8_t val);
    uint8_t dequeue();

    uint8_t m_queue[8];
    int m_qhead;
    int m_qtail; */
};

class mainboard8_device : public device_t
{
public:
	mainboard8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Memory space
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	void setaddress(offs_t offset, uint8_t busctrl);

	// Memory space for debugger access
	uint8_t debugger_read(offs_t offset);
	void debugger_write(offs_t offset, uint8_t data);

	// I/O space
	void crureadz(offs_t offset, uint8_t *value);
	void cruwrite(offs_t offset, uint8_t data);

	// Control lines
	void clock_in(int state);
	void dbin_in(int state);
	void msast_in(int state);
	void crus_in(int state);
	void ptgen_in(int state);
	void reset_console(int state);
	void hold_cpu(int state);
	void ggrdy_in(int state);

	void holda_line(int state);

	auto ready_cb() { return m_ready.bind(); }
	auto reset_cb() { return m_console_reset.bind(); }
	auto hold_cb() { return m_hold_line.bind(); }

	void set_paddress(int address);

	// Ready lines from GROMs
	void system_grom_ready(int state);
	void ptts_grom_ready(int state);
	void p8_grom_ready(int state);
	void p3_grom_ready(int state);
	void sound_ready(int state);
	void speech_ready(int state);
	void pbox_ready(int state);

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// Holds the state of the A14 line
	bool    m_A14_set = false;

	// Propagates the end of the memory cycle
	void cycle_end();

	// Original logical address.
	int     m_logical_address = 0;

	// Mapped physical address.
	int     m_physical_address = 0;

	// Indicates that a byte is waiting on the data bus (see m_latched_data)
	bool    m_pending_write = false;

	// Hold the value of the data bus. In a real machine, the data bus continues
	// to show that value, but in this emulation we have a push mechanism.
	uint8_t   m_latched_data = 0;

	// Hold the level of the GROMCLK line
	int m_gromclk = 0;

	// Selecting GROM libraries
	void select_groms();

	// Previous select state
	int m_prev_grom = 0;

	// Ready states
	bool m_speech_ready = true;
	bool m_sound_ready = true;
	bool m_pbox_ready = true;

	// Keeps the recent DBIN level
	int m_dbin_level = 0;

	// Ready line to the CPU
	devcb_write_line m_ready;

	// Reset line to the main system
	devcb_write_line m_console_reset;

	// Hold line to the main system
	devcb_write_line m_hold_line;

	// Custom chips
	required_device<vaquerro_device> m_vaquerro;
	required_device<mofetta_device>  m_mofetta;
	required_device<amigo_device>    m_amigo;
	required_device<oso_device>       m_oso;

	// Link to main cpu
	required_device<cpu_device>             m_maincpu;

	// More devices
	required_device<tms9928a_device>        m_video;
	required_device<sn76496_base_device>    m_sound;
	required_device<cd2501ecd_device>       m_speech;
	required_device<bus::ti99::gromport::gromport_device>   m_gromport;
	required_device<bus::ti99::internal::ioport_device>     m_ioport;

	required_device<ram_device>             m_sram;
	required_device<ram_device>             m_dram;

	// Debugging
	int m_last_ready = CLEAR_LINE;
	line_state m_crus_debug;

	// System GROM library
	required_device<tmc0430_device> m_sgrom0;
	required_device<tmc0430_device> m_sgrom1;
	required_device<tmc0430_device> m_sgrom2;

	// Text-to-speech GROM library
	required_device<tmc0430_device> m_tsgrom0;
	required_device<tmc0430_device> m_tsgrom1;
	required_device<tmc0430_device> m_tsgrom2;
	required_device<tmc0430_device> m_tsgrom3;
	required_device<tmc0430_device> m_tsgrom4;
	required_device<tmc0430_device> m_tsgrom5;
	required_device<tmc0430_device> m_tsgrom6;
	required_device<tmc0430_device> m_tsgrom7;

	// Pascal 8 GROM library
	required_device<tmc0430_device> m_p8grom0;
	required_device<tmc0430_device> m_p8grom1;
	required_device<tmc0430_device> m_p8grom2;
	required_device<tmc0430_device> m_p8grom3;
	required_device<tmc0430_device> m_p8grom4;
	required_device<tmc0430_device> m_p8grom5;
	required_device<tmc0430_device> m_p8grom6;
	required_device<tmc0430_device> m_p8grom7;

	// Pascal 3 GROM library
	required_device<tmc0430_device> m_p3grom0;
	required_device<tmc0430_device> m_p3grom1;
	required_device<tmc0430_device> m_p3grom2;

	// Link to the 9901
	required_device<tms9901_device> m_tms9901;

	// Idle flags for GROMs
	bool m_sgrom_idle = true;
	bool m_tsgrom_idle = true;
	bool m_p8grom_idle = true;
	bool m_p3grom_idle = true;

	// ROM area of the system.
	uint8_t*   m_rom0 = nullptr;
	uint8_t*   m_rom1 = nullptr;
	uint8_t*   m_pascalrom = nullptr;
};

} // end namespace bus::ti99::internal

DECLARE_DEVICE_TYPE_NS(TI99_MAINBOARD8, bus::ti99::internal, mainboard8_device)
DECLARE_DEVICE_TYPE_NS(TI99_VAQUERRO, bus::ti99::internal, vaquerro_device)
DECLARE_DEVICE_TYPE_NS(TI99_MOFETTA,  bus::ti99::internal, mofetta_device)
DECLARE_DEVICE_TYPE_NS(TI99_AMIGO,    bus::ti99::internal, amigo_device)
DECLARE_DEVICE_TYPE_NS(TI99_OSO,      bus::ti99::internal, oso_device)

#endif // MAME_BUS_TI99_INTERNAL_998BOARD_H
