// license:BSD-3-Clause
// copyright-holders:F. Ulivi
//
// *******************************
// Driver for HP series 80 systems
// *******************************
//
// This driver currently emulates the HP85A & HP86B machines.
//
// What's in HP85A emulation:
// - Capricorn CPU @613 kHz
// - 32K of system ROMs
// - Optional ROMs
// - 16K of RAM
// - Alpha/graphic video
// - Internal timers
// - DC100 tape drive
// - Integrated thermal printer
// - Beeper & 1-bit bitbanged sound
// - I/O slots
//
// What's in HP86B emulation:
// - Capricorn CPU @613 kHz
// - 56K of system ROMs
// - Optional ROMs
// - 128K of RAM (through Extended Memory Controller)
// - Alpha/graphic video (with correct aspect ratio for 82913A 12" monitor)
// - Run light
// - Internal timers
// - Integrated HPIB interface (which is basically a built-in 82937 module)
// - Beeper & 1-bit bitbanged sound
// - I/O slots
//
// The HP86B was also produced with support for various non-English European languages.
// There were 3 major differences between standard (i.e. English only) and international models:
// - The keyboard controller IC translated from the matrix position to key code in the
//   standard model whereas it only reported the row/column position in the international models.
//   In the latter case the decoding was done in software.
// - The international models had an extra built-in ROM (the "Language" ROM) that handled the
//   decoding of various keyboard layouts and also provided some extra BASIC instructions to
//   deal with non-English text.
// - There were 3 video controllers that displayed different character shapes in the 00..1b range.
//
// This table summarizes the differences between hp86b models.
//
// | Model   | Kb layout        | Kb controller | Has           | Video controller | Emulator  |
// |         |                  | decodes key?  | Language ROM? | version          |           |
// |---------+------------------+---------------+---------------+------------------+-----------|
// | Std     | English          | Yes           | No            | 1st              | hp86b     |
// | Opt 001 | Swedish/Finnish  | No            | Yes           | 2nd              | hp86b_001 |
// | Opt 002 | Danish/Norwegian | No            | Yes           | 2nd              | hp86b_001 |
// | Opt 004 | German           | No            | Yes           | 3rd              | hp86b_004 |
// | Opt 006 | Spanish          | No            | Yes           | 2nd              | hp86b_001 |
// | Opt 008 | French           | No            | Yes           | 3rd              | hp86b_004 |
// | Opt 009 | Italian          | No            | Yes           | 3rd              | hp86b_004 |
// | Opt 010 | Dutch            | No            | Yes           | 3rd              | hp86b_004 |
// | Opt 020 | Swiss German     | No            | Yes           | 3rd              | hp86b_004 |
// | Opt 021 | Swiss French     | No            | Yes           | 3rd              | hp86b_004 |
//
// Special thanks to Everett Kaser for his excellent reverse engineering of Language ROM and for
// all his support.
//
// Thanks to all the people who made docs available & dumped the various ROMs.
//
// References for these systems:
// https://groups.io/g/hpseries80 - Site with tons of info on HP80 systems
// http://www.kaser.com/hp85.html - A Windows-based emulator of HP80 systems
// https://sites.google.com/site/olivier2smet2/hpseries80 - Another Windows-based emulator
// http://www.series80.org/ - *The* reference site for these machines
// http://www.akso.de/index.php?id=hp_series_80&L=-1%27 - Another interesting site
// http://www.hpmuseum.net/exhibit.php?class=1&cat=9 - Last but not least: HP museum pages for HP80

#include "emu.h"
#include "emupal.h"
#include "screen.h"
#include "cpu/capricorn/capricorn.h"
#include "speaker.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "sound/dac.h"
#include "machine/1ma6.h"
#include "hp80_optrom.h"
#include "machine/ram.h"
#include "softlist_dev.h"
#include "machine/bankdev.h"
#include "bus/hp80_io/hp80_io.h"
#include "bus/hp80_io/82937.h"
#include "imagedev/bitbngr.h"
#include "hp86b.lh"

// Debugging
#include "logmacro.h"
#define LOG_EMC_MASK (LOG_GENERAL << 1)
#define LOG_EMC(...) LOGMASKED(LOG_EMC_MASK, __VA_ARGS__)
#define LOG_IRQ_MASK (LOG_EMC_MASK << 1)
#define LOG_IRQ(...) LOGMASKED(LOG_IRQ_MASK, __VA_ARGS__)
#undef VERBOSE
//#define VERBOSE (LOG_GENERAL|LOG_EMC_MASK|LOG_IRQ_MASK)
#define VERBOSE LOG_GENERAL

// Bit manipulation
namespace {
	template<typename T> constexpr T BIT_MASK(unsigned n)
	{
		return (T)1U << n;
	}

	template<typename T> void BIT_CLR(T& w , unsigned n)
	{
		w &= ~BIT_MASK<T>(n);
	}

	template<typename T> void BIT_SET(T& w , unsigned n)
	{
		w |= BIT_MASK<T>(n);
	}

	template<typename T> void COPY_BIT(bool bit , T& w , unsigned n)
	{
		if (bit) {
			BIT_SET(w , n);
		} else {
			BIT_CLR(w , n);
		}
	}
}


namespace {

// **** Constants ****
static constexpr unsigned CPU_CLOCK = 613000;
// Time taken by hw timer updating (semi-made up) (in µsec)
static constexpr unsigned TIMER_BUSY_USEC   = 128;
static constexpr unsigned IRQ_KEYBOARD_BIT  = 0;
static constexpr unsigned IRQ_INTKEYB_BIT   = 1;
static constexpr unsigned IRQ_TIMER0_BIT    = 2;
static constexpr unsigned TIMER_COUNT       = 4;
static constexpr unsigned IRQ_IOP0_BIT      = IRQ_TIMER0_BIT + TIMER_COUNT;
// Maximum count of I/O processors (the same thing as count of I/O slots)
static constexpr unsigned IOP_COUNT         = 4;
static constexpr unsigned IRQ_BIT_COUNT     = IRQ_IOP0_BIT + IOP_COUNT;
static constexpr unsigned NO_IRQ            = IRQ_BIT_COUNT;

// *****************
//  hp80_base_state
// *****************
class hp80_base_state : public driver_device
{
public:
	hp80_base_state(const machine_config &mconfig, device_type type, const char *tag, bool has_int_keyb = false);

protected:
	void hp80_base(machine_config &config);

	virtual void cpu_mem_map(address_map &map) ATTR_COLD;
	virtual void rombank_mem_map(address_map &map) ATTR_COLD;
	virtual void unmap_optroms(address_space &space);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t intack_r();

	void ginten_w(uint8_t data);
	void gintdis_w(uint8_t data);
	uint8_t keysts_r();
	void keysts_w(uint8_t data);
	uint8_t keycod_r();
	void keycod_w(uint8_t data);
	uint8_t clksts_r();
	void clksts_w(uint8_t data);
	uint8_t clkdat_r();
	void clkdat_w(uint8_t data);
	void rselec_w(uint8_t data);
	uint8_t intrsc_r();
	void intrsc_w(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(kb_scan);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_update);
	TIMER_DEVICE_CALLBACK_MEMBER(clk_busy_timer);

	void irl_w(offs_t offset, uint8_t data);
	void halt_w(offs_t offset, uint8_t data);

	required_device<capricorn_cpu_device> m_cpu;
	required_device<timer_device> m_clk_busy_timer;
	required_device<beep_device> m_beep;
	required_device<dac_1bit_device> m_dac;
	required_ioport m_io_key0;
	required_ioport m_io_key1;
	required_ioport m_io_key2;
	required_ioport m_io_modkeys;
	optional_ioport m_io_language;
	required_device_array<hp80_optrom_device , 6> m_rom_drawers;
	required_device<address_map_bank_device> m_rombank;
	required_device_array<hp80_io_slot_device , IOP_COUNT> m_io_slots;

	bool m_global_int_en;
	uint16_t m_int_serv;
	uint16_t m_int_acked;
	unsigned m_top_acked;
	unsigned m_top_pending;
	uint16_t m_int_en;
	uint8_t m_halt_lines;

	// State of keyboard
	ioport_value m_kb_state[ 3 ];
	bool m_int_kb_enabled;
	bool m_kb_enable;
	bool m_kb_pressed;
	bool m_kb_flipped;
	bool m_kb_lang_readout;
	bool m_kb_raw_readout;
	uint8_t m_kb_keycode;
	uint8_t m_raw_keycode;
	const bool m_has_int_keyb;

	// Timers
	typedef struct {
		uint8_t m_timer_cnt[ 4 ];
		uint8_t m_timer_reg[ 4 ];
		bool m_timer_en;
		bool m_timer_clr;
		uint8_t m_digit_to_match;
	} hw_timer_t;
	hw_timer_t m_hw_timer[ TIMER_COUNT ];
	uint8_t m_timer_idx;
	bool m_clk_busy;

	bool kb_scan_ioport(ioport_value pressed , unsigned idx_base , uint8_t& row , uint8_t& col);
	unsigned get_kb_irq() const;

	void irq_w(unsigned n_irq , bool state);
	void irq_en_w(unsigned n_irq , bool state);
	void release_irq(unsigned n_irq);
	static unsigned get_top_irq(uint16_t irqs);
	void update_int_bits();
	void update_irl();
};

hp80_base_state::hp80_base_state(const machine_config &mconfig, device_type type, const char *tag, bool has_int_keyb)
	: driver_device(mconfig , type , tag)
	, m_cpu(*this , "cpu")
	, m_clk_busy_timer(*this , "clk_busy_timer")
	, m_beep(*this , "beeper")
	, m_dac(*this , "dac")
	, m_io_key0(*this , "KEY0")
	, m_io_key1(*this , "KEY1")
	, m_io_key2(*this , "KEY2")
	, m_io_modkeys(*this, "MODKEYS")
	, m_io_language(*this , "LANGUAGE")
	, m_rom_drawers(*this , "drawer%u" , 1)
	, m_rombank(*this , "rombank")
	, m_io_slots(*this , "slot%u" , 1)
	, m_has_int_keyb(has_int_keyb)
{
}

void hp80_base_state::hp80_base(machine_config &config)
{
	HP_CAPRICORN(config, m_cpu, CPU_CLOCK);
	m_cpu->set_addrmap(AS_PROGRAM, &hp80_base_state::cpu_mem_map);
	m_cpu->intack_cb().set(FUNC(hp80_base_state::intack_r));
	config.set_perfect_quantum(m_cpu);

	ADDRESS_MAP_BANK(config, "rombank").set_map(&hp80_base_state::rombank_mem_map).set_options(ENDIANNESS_LITTLE, 8, 21, HP80_OPTROM_SIZE);

	// No idea at all about the actual keyboard scan frequency
	TIMER(config, "kb_timer").configure_periodic(FUNC(hp80_base_state::kb_scan), attotime::from_hz(100));

	// Hw timers are updated at 1 kHz rate
	TIMER(config, "hw_timer").configure_periodic(FUNC(hp80_base_state::timer_update), attotime::from_hz(1000));
	TIMER(config, m_clk_busy_timer).configure_generic(FUNC(hp80_base_state::clk_busy_timer));

	// Beeper
	SPEAKER(config, "mono").front_center();
	DAC_1BIT(config, m_dac , 0).add_route(ALL_OUTPUTS, "mono", 0.5, AUTO_ALLOC_INPUT, 0);
	BEEP(config, m_beep, CPU_CLOCK / 512).add_route(ALL_OUTPUTS, "mono", 0.5, AUTO_ALLOC_INPUT, 0);

	// Optional ROMs
	for (auto& finder : m_rom_drawers) {
		HP80_OPTROM(config, finder);
	}

	// I/O slots
	for (unsigned slot = 0; slot < 4; slot++) {
		auto& finder = m_io_slots[ slot ];
		HP80_IO_SLOT(config, finder).set_slot_no(slot);
		finder->irl_cb().set(FUNC(hp80_base_state::irl_w));
		finder->halt_cb().set(FUNC(hp80_base_state::halt_w));
	}
}

void hp80_base_state::cpu_mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x7fff).m(m_rombank, FUNC(address_map_bank_device::amap8));
	map(0xff00, 0xff00).w(FUNC(hp80_base_state::ginten_w));
	map(0xff01, 0xff01).w(FUNC(hp80_base_state::gintdis_w));
	map(0xff02, 0xff02).rw(FUNC(hp80_base_state::keysts_r), FUNC(hp80_base_state::keysts_w));
	map(0xff03, 0xff03).rw(FUNC(hp80_base_state::keycod_r), FUNC(hp80_base_state::keycod_w));
	map(0xff0a, 0xff0a).rw(FUNC(hp80_base_state::clksts_r), FUNC(hp80_base_state::clksts_w));
	map(0xff0b, 0xff0b).rw(FUNC(hp80_base_state::clkdat_r), FUNC(hp80_base_state::clkdat_w));
	map(0xff18, 0xff18).w(FUNC(hp80_base_state::rselec_w));
	map(0xff40, 0xff40).rw(FUNC(hp80_base_state::intrsc_r), FUNC(hp80_base_state::intrsc_w));
}

void hp80_base_state::rombank_mem_map(address_map &map)
{
	map.unmap_value_high();
	// ROM in bank 0 is always present (it's part of system ROMs)
	map(0x0000, 0x1fff).rom();
}

void hp80_base_state::unmap_optroms(address_space &space)
{
}

void hp80_base_state::machine_start()
{
	save_item(NAME(m_global_int_en));
	save_item(NAME(m_int_serv));
	save_item(NAME(m_int_acked));
	save_item(NAME(m_top_acked));
	save_item(NAME(m_top_pending));
	save_item(NAME(m_int_en));
	save_item(NAME(m_halt_lines));
	save_pointer(NAME(m_kb_state) , 3);
	save_item(NAME(m_kb_enable));
	save_item(NAME(m_int_kb_enabled));
	save_item(NAME(m_kb_pressed));
	save_item(NAME(m_kb_flipped));
	save_item(NAME(m_kb_lang_readout));
	save_item(NAME(m_kb_raw_readout));
	save_item(NAME(m_kb_keycode));
	save_item(NAME(m_raw_keycode));
}

void hp80_base_state::machine_reset()
{
	m_int_serv = 0;
	m_int_acked = 0;
	m_top_acked = NO_IRQ;
	m_top_pending = NO_IRQ;
	m_int_en = 0;
	m_global_int_en = false;
	m_kb_state[ 0 ] = 0;
	m_kb_state[ 1 ] = 0;
	m_kb_state[ 2 ] = 0;
	m_kb_keycode = 0xff;
	m_kb_enable = true;
	m_int_kb_enabled = false;
	m_kb_pressed = false;
	m_kb_flipped = false;
	m_kb_lang_readout = false;
	m_kb_raw_readout = false;
	for (auto& timer : m_hw_timer) {
		for (unsigned i = 0; i < 4; i++) {
			timer.m_timer_cnt[ i ] = 0;
			timer.m_timer_reg[ i ] = 0;
		}
		timer.m_timer_en = false;
		timer.m_timer_clr = false;
		timer.m_digit_to_match = 0;
	}
	m_timer_idx = 0;
	m_clk_busy = false;
	update_irl();
	m_halt_lines = 0;
	m_cpu->set_input_line(INPUT_LINE_HALT , CLEAR_LINE);

	// Load optional ROMs (if any)
	unmap_optroms(m_rombank->space(AS_PROGRAM));
	for (auto& draw : m_rom_drawers) {
		LOG("Loading opt ROM in drawer %s\n" , draw->tag());
		draw->install_read_handler(m_rombank->space(AS_PROGRAM));
	}

	// Clear RSELEC
	m_rombank->set_bank(0xff);

	// Mount I/O slots in address space
	m_cpu->space(AS_PROGRAM).unmap_readwrite(0xff50 , 0xff5f);
	for (auto& io : m_io_slots) {
		io->install_read_write_handlers(m_cpu->space(AS_PROGRAM));
	}
}

// Vector table (indexed by bit no. in m_int_serv)
static const uint8_t vector_table[] = {
	0x04,   // Keyboard
	0x12,   // International keyboard (or is it 0x14?)
	0x08,   // Timer 0
	0x0a,   // Timer 1
	0x0c,   // Timer 2
	0x0e,   // Timer 3
	0x10,   // Slot 1
	0x10,   // Slot 2
	0x10,   // Slot 3
	0x10,   // Slot 4
	0x00    // No IRQ
};

uint8_t hp80_base_state::intack_r()
{
	LOG_IRQ("INTACK %u %u\n" , m_top_pending , m_top_acked);
	BIT_SET(m_int_acked , m_top_pending);
	m_top_acked = m_top_pending;
	if (m_top_pending > IRQ_IOP0_BIT && m_top_pending < IRQ_BIT_COUNT) {
		// Interrupts are disabled in all I/O translators of higher priority than
		// the one being serviced
		for (unsigned i = m_top_pending - 1; i >= IRQ_IOP0_BIT; i--) {
			irq_en_w(i , false);
		}
	}
	update_irl();
	return vector_table[ m_top_pending ];
}

void hp80_base_state::ginten_w(uint8_t data)
{
	LOG_IRQ("GINTEN\n");
	m_global_int_en = true;
	update_irl();
}

void hp80_base_state::gintdis_w(uint8_t data)
{
	LOG_IRQ("GINTDIS\n");
	m_global_int_en = false;
	update_irl();
}

uint8_t hp80_base_state::keysts_r()
{
	uint8_t res = 0;
	if (BIT(m_int_en , get_kb_irq())) {
		BIT_SET(res , 0);
	}
	if (m_kb_pressed) {
		BIT_SET(res , 1);
	}
	if (m_has_int_keyb) {
		if (m_kb_flipped) {
			BIT_SET(res , 2);
		}
		if (BIT(m_io_modkeys->read() , 2)) {
			BIT_SET(res , 6);
		}
	}
	if (BIT(m_io_modkeys->read() , 0)) {
		BIT_SET(res , 3);
	}
	if (m_global_int_en) {
		BIT_SET(res , 7);
	}
	return res;
}

void hp80_base_state::keysts_w(uint8_t data)
{
	if (BIT(data , 0)) {
		irq_en_w(get_kb_irq() , true);
	} else if (BIT(data , 1)) {
		irq_en_w(get_kb_irq() , false);
	}
	if (m_has_int_keyb) {
		m_kb_lang_readout = BIT(data , 2);
		m_kb_raw_readout = BIT(data , 3);
		if (!m_int_kb_enabled && (data & 0x0f) == 1) {
			m_int_kb_enabled = true;
		}
	}
	m_dac->write(BIT(data , 5));
	m_beep->set_state(BIT(data , 6));
	if (BIT(data , 7)) {
		m_kb_flipped = !m_kb_flipped;
	}
}

uint8_t hp80_base_state::keycod_r()
{
	if (m_kb_lang_readout && m_io_language) {
		return m_io_language->read();
	} else if (m_kb_raw_readout) {
		return m_raw_keycode;
	} else {
		return m_kb_keycode;
	}
}

void hp80_base_state::keycod_w(uint8_t data)
{
	if (m_kb_raw_readout) {
		m_kb_keycode = data;
	} else if (data == 1) {
		unsigned irq = get_kb_irq();
		irq_w(irq , false);
		m_kb_enable = true;
		release_irq(irq);
	}
}

uint8_t hp80_base_state::clksts_r()
{
	uint8_t res = 0;
	for (unsigned i = 0; i < TIMER_COUNT; i++) {
		if (BIT(m_int_en , IRQ_TIMER0_BIT + i)) {
			BIT_SET(res , i);
		}
	}
	if (!m_clk_busy) {
		BIT_SET(res , 7);
	}
	return res;
}

void hp80_base_state::clksts_w(uint8_t data)
{
	if (data == 0x0c) {
		// Set test mode (see timer_update)
		auto& timer = m_hw_timer[ m_timer_idx ];
		timer.m_digit_to_match = 1;
		timer.m_timer_cnt[ 0 ] = timer.m_timer_reg[ 0 ];
		timer.m_timer_cnt[ 1 ] = timer.m_timer_reg[ 1 ];
		timer.m_timer_cnt[ 2 ] = timer.m_timer_reg[ 2 ];
		timer.m_timer_cnt[ 3 ] = timer.m_timer_reg[ 3 ];
		LOG("Test mode enabled for timer %u\n" , m_timer_idx);
	} else {
		m_timer_idx = (data >> 6) & 3;
		auto& timer = m_hw_timer[ m_timer_idx ];
		if (BIT(data , 0)) {
			// Disable timer irq
			irq_en_w(IRQ_TIMER0_BIT + m_timer_idx , false);
		} else if (BIT(data , 1)) {
			// Enable timer irq
			irq_en_w(IRQ_TIMER0_BIT + m_timer_idx , true);
		}
		if (BIT(data , 2)) {
			// Stop timer
			timer.m_timer_en = false;
		} else if (BIT(data , 3)) {
			// Start timer
			timer.m_timer_en = true;
		}
		if (BIT(data , 4) || (BIT(data , 3) && timer.m_digit_to_match)) {
			// Clear timer
			timer.m_timer_clr = true;
			// Disable test mode
			timer.m_digit_to_match = 0;
		}
		if (BIT(data , 5)) {
			// Clear timer irq
			unsigned irq_n = IRQ_TIMER0_BIT + m_timer_idx;
			irq_w(irq_n , false);
			release_irq(irq_n);
		}
	}
}

uint8_t hp80_base_state::clkdat_r()
{
	uint8_t res;
	unsigned burst_idx = m_cpu->flatten_burst();
	if (burst_idx < 4) {
		res = m_hw_timer[ m_timer_idx ].m_timer_cnt[ burst_idx ];
	} else {
		// What happens when loading more than 4 bytes from timers?
		LOG("Reading more than 4 bytes from timer %u\n" , m_timer_idx);
		res = 0;
	}
	return res;
}

void hp80_base_state::clkdat_w(uint8_t data)
{
	unsigned burst_idx = m_cpu->flatten_burst();
	if (burst_idx < 4) {
		m_hw_timer[ m_timer_idx ].m_timer_reg[ burst_idx ] = data;
	} else {
		// What happens when storing more than 4 bytes into timers?
		LOG("Writing more than 4 bytes into timer %u\n" , m_timer_idx);
	}
}

void hp80_base_state::rselec_w(uint8_t data)
{
	m_rombank->set_bank(data);
}

uint8_t hp80_base_state::intrsc_r()
{
	if (m_top_acked >= IRQ_IOP0_BIT && m_top_acked < IRQ_BIT_COUNT) {
		LOG_IRQ("INTRSC %u\n" , m_top_acked);
		// Clear interrupt request in the slot being serviced
		m_io_slots[ m_top_acked - IRQ_IOP0_BIT ]->clear_service();
		return (uint8_t)m_io_slots[ m_top_acked - IRQ_IOP0_BIT ]->get_base_addr();
	} else {
		// Probably..
		return 0xff;
	}
}

void hp80_base_state::intrsc_w(uint8_t data)
{
	LOG_IRQ("INTRSC W %u %03x %03x %03x\n" , m_top_acked , m_int_serv , m_int_en , m_int_acked);
	for (auto& iop: m_io_slots) {
		iop->inten();
	}
	for (unsigned i = IRQ_IOP0_BIT; i < (IRQ_IOP0_BIT + IOP_COUNT); i++) {
		irq_en_w(i , true);
	}
	m_int_acked &= ~(((1U << IOP_COUNT) - 1) << IRQ_IOP0_BIT);
	update_int_bits();
}

// Outer index: key position [0..79] = r * 8 + c
// Inner index: SHIFT state (0 = no SHIFT, 1 = SHIFT)
static const uint8_t keyboard_table[ 80 ][ 2 ] = {
	// --    SHIFT              HP85            HP86
	{ 0xa2 , 0xac },    // 0,0: Down / Auto     k6 / k13
	{ 0xa1 , 0xa5 },    // 0,1: Up / Home       k5 / k12
	{ 0x83 , 0x87 },    // 0,2: k4 / k8         k4 / k11
	{ 0x82 , 0x86 },    // 0,3: k3 / k7         k3 / k10
	{ 0x81 , 0x85 },    // 0,4: k2 / k6         k2 / k9
	{ 0x80 , 0x84 },    // 0,5: k1 / k5         k1 / k8
	{ 0x96 , 0x60 },    // 0,6: LABEL KEY
	{ 0xff , 0xff },    // 0,7: N/U
	{ 0x38 , 0x2a },    // 1,0: 8
	{ 0x37 , 0x26 },    // 1,1: 7
	{ 0x36 , 0x5e },    // 1,2: 6
	{ 0x35 , 0x25 },    // 1,3: 5
	{ 0x34 , 0x24 },    // 1,4: 4
	{ 0x33 , 0x23 },    // 1,5: 3
	{ 0x32 , 0x40 },    // 1,6: 2
	{ 0x31 , 0x21 },    // 1,7: 1
	{ 0x49 , 0x69 },    // 2,0: I
	{ 0x55 , 0x75 },    // 2,1: U
	{ 0x59 , 0x79 },    // 2,2: Y
	{ 0x54 , 0x74 },    // 2,3: T
	{ 0x52 , 0x72 },    // 2,4: R
	{ 0x45 , 0x65 },    // 2,5: E
	{ 0x57 , 0x77 },    // 2,6: W
	{ 0x51 , 0x71 },    // 2,7: Q
	{ 0x4b , 0x6b },    // 3,0: K
	{ 0x4a , 0x6a },    // 3,1: J
	{ 0x48 , 0x68 },    // 3,2: H
	{ 0x47 , 0x67 },    // 3,3: G
	{ 0x46 , 0x66 },    // 3,4: F
	{ 0x44 , 0x64 },    // 3,5: D
	{ 0x53 , 0x73 },    // 3,6: S
	{ 0x41 , 0x61 },    // 3,7: A
	{ 0x4d , 0x6d },    // 4,0: M
	{ 0x4e , 0x6e },    // 4,1: N
	{ 0x42 , 0x62 },    // 4,2: B
	{ 0x56 , 0x76 },    // 4,3: V
	{ 0x43 , 0x63 },    // 4,4: C
	{ 0x58 , 0x78 },    // 4,5: X
	{ 0x5a , 0x7a },    // 4,6: Z
	{ 0x20 , 0x20 },    // 4,7: Space
	{ 0x2c , 0x3c },    // 5,0: , <
	{ 0x2e , 0x3e },    // 5,1: . >
	{ 0x2f , 0x3f },    // 5,2: / ?
	{ 0x8e , 0x90 },    // 5,3: PAUSE / STEP
	{ 0x8d , 0x8d },    // 5,4: RUN
	{ 0x2b , 0x7f },    // 5,5: KP +
	{ 0x2d , 0x7d },    // 5,6: KP -
	{ 0x2a , 0x7e },    // 5,7: KP *            N/U
	{ 0x4c , 0x6c },    // 6,0: L
	{ 0x3b , 0x3a },    // 6,1: ; :
	{ 0x27 , 0x22 },    // 6,2: ' "
	{ 0x9a , 0x9a },    // 6,3: END LINE
	{ 0x94 , 0x95 },    // 6,4: LIST / P LST
	{ 0xff , 0xff },    // 6,5: N/U
	{ 0x2a , 0x7e },    // 6,6: N/U             KP *
	{ 0x2f , 0x7b },    // 6,7: KP /
	{ 0x4f , 0x6f },    // 7,0: O
	{ 0x50 , 0x70 },    // 7,1: P
	{ 0x28 , 0x5b },    // 7,2: ( [
	{ 0x29 , 0x5d },    // 7,3: ) ]
	{ 0x8f , 0xad },    // 7,4: CONT / SCRATCH  CONT / TR/NORM
	{ 0xa0 , 0x92 },    // 7,5: -LINE / CLEAR   E / TEST
	{ 0x29 , 0x8c },    // 7,6: ) INIT
	{ 0xff , 0xff },    // 7,7: N/U
	{ 0x39 , 0x28 },    // 8,0: 9
	{ 0x30 , 0x29 },    // 8,1: 0
	{ 0x2d , 0x5f },    // 8,2: - _
	{ 0x3d , 0x2b },    // 8,3: = +
	{ 0x5c , 0x7c },    // 8,4: \ |
	{ 0x99 , 0x9b },    // 8,5: BS
	{ 0x28 , 0x8b },    // 8,6: ( RESET
	{ 0x5e , 0xa6 },    // 8,7: ^ / RESLT
	{ 0x9c , 0x93 },    // 9,0: LEFT / GRAPH    k7 / k14
	{ 0x9d , 0x89 },    // 9,1: RIGHT / COPY    -LINE / CLEAR
	{ 0xa3 , 0xa3 },    // 9,2: RPL / INS       UP / HOME
	{ 0xa4 , 0xa8 },    // 9,3: -CHAR / DEL     DOWN / A/G
	{ 0x9f , 0x9e },    // 9,4: ROLL            LEFT / I/R
	{ 0xaa , 0x88 },    // 9,5: LOAD / REW      RIGHT / -CHAR
	{ 0xa9 , 0x91 },    // 9,6: STORE / TEST    ROLL
	{ 0x8a , 0x8a }     // 9,7: PAPER ADVANCE   N/U
};

bool hp80_base_state::kb_scan_ioport(ioport_value pressed , unsigned idx_base , uint8_t& row , uint8_t& col)
{
	if (pressed) {
		unsigned bit_no = 31 - count_leading_zeros_32(pressed);
		row = (idx_base + bit_no) / 8;
		col = (idx_base + bit_no) % 8;
		return true;
	} else {
		return false;
	}
}

unsigned hp80_base_state::get_kb_irq() const
{
	return m_has_int_keyb ? IRQ_INTKEYB_BIT : IRQ_KEYBOARD_BIT;
}

TIMER_DEVICE_CALLBACK_MEMBER(hp80_base_state::kb_scan)
{
	ioport_value input[ 3 ];
	input[ 0 ] = m_io_key0->read();
	input[ 1 ] = m_io_key1->read();
	input[ 2 ] = m_io_key2->read();

	if (m_kb_enable && (!m_has_int_keyb || m_int_kb_enabled)) {
		uint8_t row;
		uint8_t col;

		bool got_key = kb_scan_ioport(input[ 0 ] & ~m_kb_state[ 0 ] , 0 , row , col) ||
			kb_scan_ioport(input[ 1 ] & ~m_kb_state[ 1 ] , 32 , row , col) ||
			kb_scan_ioport(input[ 2 ] & ~m_kb_state[ 2 ] , 64 , row , col);

		if (got_key) {
			if (m_has_int_keyb) {
				m_raw_keycode = (row << 4) + col;
			} else {
				uint8_t keycode = (row << 3) + col;
				uint8_t unshifted = keyboard_table[ keycode ][ 0 ];
				bool isalpha = unshifted >= 'A' && unshifted <= 'Z';
				ioport_value modifiers = m_io_modkeys->read();
				bool shift = BIT(modifiers , 0);
				bool caps_lock = BIT(modifiers , 1);
				bool control = BIT(modifiers , 2);
				if (isalpha) {
					shift = shift ^ caps_lock ^ m_kb_flipped;
				}
				keycode = keyboard_table[ keycode ][ shift ];
				uint8_t tmp = isalpha ? unshifted : keycode;
				if (control && (tmp & 0xe0) == 0x40) {
					keycode &= ~0xe0;
				}
				m_kb_keycode = keycode;
			}
			irq_w(get_kb_irq() , true);
			m_kb_enable = false;
		}
	}
	m_kb_pressed = input[ 0 ] != 0 ||
		input[ 1 ] != 0 ||
		input[ 2 ] != 0;

	m_kb_state[ 0 ] = input[ 0 ];
	m_kb_state[ 1 ] = input[ 1 ];
	m_kb_state[ 2 ] = input[ 2 ];
}

TIMER_DEVICE_CALLBACK_MEMBER(hp80_base_state::timer_update)
{
	for (unsigned i = 0; i < TIMER_COUNT; i++) {
		auto& timer = m_hw_timer[ i ];
		if (timer.m_timer_clr) {
			timer.m_timer_clr = false;
			timer.m_timer_cnt[ 0 ] = 0;
			timer.m_timer_cnt[ 1 ] = 0;
			timer.m_timer_cnt[ 2 ] = 0;
			timer.m_timer_cnt[ 3 ] = 0;
		} else if (timer.m_timer_en) {
			if (timer.m_digit_to_match) {
				// Timers have an undocumented mode (used by test "J" of service ROM)
				// where the counter has to match in sequence all digits of register
				// in order to raise an interrupt. In other words interrupt is generated
				// after a number of updates that's equal to the sum of all digits in
				// register + 1. My opinion is that people at HP designed this mode to
				// allow all digits in a timer to be tested quickly. Without this special
				// mode it takes more than 27 hours to check that all digits increment
				// correctly and that there are no stuck bits.
				// From an operative point of view, we copy register into counter when
				// this special mode is activated (see clksts_w). Then, at each update,
				// we decrement the digit of counter pointed to by m_digit_to_match (1 =
				// least significant digit). Each time a digit "borrows" (i.e. it decrements
				// from 0 to 9), we move on to digit at left. When m_digit_to_match reaches
				// 9, interrupt is raised and the timer stops.
				// At this point counter is always "99999999".
				if (timer.m_digit_to_match < 9) {
					while (true) {
						bool borrow = false;
						uint8_t b = timer.m_timer_cnt[ (timer.m_digit_to_match - 1) / 2 ];
						if (BIT(timer.m_digit_to_match , 0)) {
							// Least significant digit in b
							if (b & 0x0f) {
								b--;
							} else {
								b = (b & 0xf0) | 9;
								borrow = true;
							}
						} else {
							// Most significant digit in b
							if (b & 0xf0) {
								b -= 0x10;
							} else {
								b = 0x99;
								borrow = true;
							}
						}
						timer.m_timer_cnt[ (timer.m_digit_to_match - 1) / 2 ] = b;
						if (borrow) {
							timer.m_digit_to_match++;
							if (timer.m_digit_to_match == 9) {
								irq_w(IRQ_TIMER0_BIT + i , true);
								break;
							}
						} else {
							break;
						}
					}
				}
			} else {
				// Standard timer mode
				// Increment all active timers by 1
				bool carry = true;
				for (unsigned idx = 0; idx < 4 && carry; idx++) {
					carry = false;
					uint8_t b = timer.m_timer_cnt[ idx ];
					b++;
					if ((b & 0xf) > 9) {
						b += 6;
						if (b >= 0xa0) {
							b += 0x60;
							carry = true;
						}
					}
					timer.m_timer_cnt[ idx ] = b;
				}
				if (timer.m_timer_cnt[ 0 ] == timer.m_timer_reg[ 0 ] &&
					timer.m_timer_cnt[ 1 ] == timer.m_timer_reg[ 1 ] &&
					timer.m_timer_cnt[ 2 ] == timer.m_timer_reg[ 2 ] &&
					timer.m_timer_cnt[ 3 ] == timer.m_timer_reg[ 3 ]) {
					timer.m_timer_cnt[ 0 ] = 0;
					timer.m_timer_cnt[ 1 ] = 0;
					timer.m_timer_cnt[ 2 ] = 0;
					timer.m_timer_cnt[ 3 ] = 0;
					irq_w(IRQ_TIMER0_BIT + i , true);
				}
			}
		}
	}
	m_clk_busy = true;
	m_clk_busy_timer->adjust(attotime::from_usec(TIMER_BUSY_USEC));
}

TIMER_DEVICE_CALLBACK_MEMBER(hp80_base_state::clk_busy_timer)
{
	m_clk_busy = false;
}

void hp80_base_state::irl_w(offs_t offset, uint8_t data)
{
	irq_w(offset + IRQ_IOP0_BIT , data != 0);
}

void hp80_base_state::halt_w(offs_t offset, uint8_t data)
{
	bool prev_halt = m_halt_lines != 0;
	COPY_BIT(data != 0 , m_halt_lines , offset);
	bool new_halt = m_halt_lines != 0;
	if (prev_halt != new_halt) {
		LOG_IRQ("halt=%d hl=%x\n" , new_halt , m_halt_lines);
		m_cpu->set_input_line(INPUT_LINE_HALT , new_halt);
	}
}

void hp80_base_state::irq_w(unsigned n_irq , bool state)
{
	LOG_IRQ("IRQ_W %u %d\n" , n_irq , state);
	COPY_BIT(state , m_int_serv , n_irq);
	update_int_bits();
}

void hp80_base_state::irq_en_w(unsigned n_irq , bool state)
{
	LOG_IRQ("IRQ_EN_W %u %d\n" , n_irq , state);
	COPY_BIT(state , m_int_en , n_irq);
	update_int_bits();
}

void hp80_base_state::release_irq(unsigned n_irq)
{
	if (BIT(m_int_acked , n_irq)) {
		BIT_CLR(m_int_acked , n_irq);
		update_int_bits();
	}
}

unsigned hp80_base_state::get_top_irq(uint16_t irqs)
{
	unsigned top;
	for (top = 0; top < IRQ_BIT_COUNT && !BIT(irqs , 0); top++ , irqs >>= 1) {
	}
	return top;
}

void hp80_base_state::update_int_bits()
{
	m_top_pending = get_top_irq(m_int_en & m_int_serv);
	m_top_acked = get_top_irq(m_int_acked);
	update_irl();
}

void hp80_base_state::update_irl()
{
	m_cpu->set_input_line(0 , m_global_int_en && m_top_pending < m_top_acked);
}

// ************
//  hp85_state
// ************
class hp85_state : public hp80_base_state
{
public:
	hp85_state(const machine_config &mconfig, device_type type, const char *tag);

	// **** Constants of HP85 ****
	static constexpr unsigned MASTER_CLOCK  = 9808000;
	// Video memory is actually made of 16384 4-bit nibbles
	static constexpr unsigned VIDEO_MEM_SIZE    = 8192;
	static constexpr unsigned ALPHA_MEM_SIZE    = 4096;
	static constexpr unsigned GRAPH_MEM_SIZE    = 16384;
	static constexpr unsigned CRT_STS_READY_BIT     = 0;
	static constexpr unsigned CRT_STS_DISPLAY_BIT   = 1;
	static constexpr unsigned CRT_STS_BUSY_BIT      = 7;
	static constexpr unsigned CRT_CTL_RD_RQ_BIT     = 0;
	static constexpr unsigned CRT_CTL_WIPEOUT_BIT   = 1;
	static constexpr unsigned CRT_CTL_POWERDN_BIT   = 2;
	static constexpr unsigned CRT_CTL_GRAPHICS_BIT  = 7;
	// Time to read/write a byte in video memory (in master clock cycles)
	static constexpr unsigned CRT_RW_TIME           = 96;
	// Internal printer has a moving printhead with 8 vertically-arranged resistors that print dots
	// by heating thermal paper. The horizontal span of the printhead covers 224 columns.
	// In alpha mode, each sweep prints up to 32 characters. Each character has a 8x7 cell.
	// 8 pixels of cell height are covered by the printhead height, whereas 7 pixels of width
	// allow for 32 characters on a row (224 = 32 * 7).
	// After an alpha line is printed the paper advances by 10 pixel lines, so that a space of
	// 2 lines is left between alpha lines.
	// In graphic mode, printing starts at column 16 and covers 192 columns. So on each side of
	// the printed area there's a 16-column wide margin (224 = 192 + 2 * 16).
	// Once a graphic line is printed, paper advances by 8 pixel lines so that no space is inserted
	// between successive sweeps.
	// A full image of the graphic screen (256 x 192) is printed rotated 90 degrees clockwise.
	// The printer controller chip (1MA9) has an embedded character generator ROM that is used
	// when printing alpha lines. This ROM is also read by the CPU when drawing text on the graphic
	// screen (BASIC "LABEL" instruction).
	static constexpr unsigned PRT_BUFFER_SIZE      = 192;
	static constexpr unsigned PRTSTS_PAPER_OK_BIT  = 7;
	static constexpr unsigned PRTSTS_DATARDY_BIT   = 6;
	static constexpr unsigned PRTSTS_PRTRDY_BIT    = 0;
	static constexpr unsigned PRTCTL_GRAPHIC_BIT   = 7;
	//constexpr unsigned PRTCTL_POWERUP_BIT = 6;
	static constexpr unsigned PRTCTL_READGEN_BIT   = 5;
	// Time to print a line (nominal speed is 2 lines/s)
	static constexpr unsigned PRT_BUSY_MSEC        = 500;
	// Horizontal start position of graphic print (16 columns from left-hand side)
	static constexpr unsigned PRT_GRAPH_OFFSET     = 16;
	// Height of printhead
	static constexpr unsigned PRT_PH_HEIGHT        = 8;
	// Height of alpha rows
	static constexpr unsigned PRT_ALPHA_HEIGHT     = 10;
	// Width of character cells
	static constexpr unsigned PRT_CELL_WIDTH       = 7;
	// Height of graphic rows
	//constexpr unsigned PRT_GRAPH_HEIGHT   = 8;
	// Width of graphic sweeps
	static constexpr unsigned PRT_GRAPH_WIDTH      = 192;
	// Width of printhead sweeps
	static constexpr unsigned PRT_WIDTH            = 224;

	void hp85(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank_w(int state);

	uint8_t crtc_r(offs_t offset);
	void crtc_w(offs_t offset, uint8_t data);
	void prtlen_w(uint8_t data);
	uint8_t prchar_r();
	void prchar_w(uint8_t data);
	uint8_t prtsts_r();
	void prtctl_w(uint8_t data);
	void prtdat_w(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(vm_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(prt_busy_timer);

	virtual void cpu_mem_map(address_map &map) override ATTR_COLD;
	virtual void unmap_optroms(address_space &space) override;

	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<timer_device> m_vm_timer;
	required_device<timer_device> m_prt_busy_timer;
	required_device<bitbanger_device> m_prt_graph_out;
	required_device<bitbanger_device> m_prt_alpha_out;

	// Character generators
	required_region_ptr<uint8_t> m_chargen;
	required_region_ptr<uint8_t> m_prt_chargen;

	bitmap_rgb32 m_bitmap;
	std::vector<uint8_t> m_video_mem;
	uint16_t m_crt_sad = 0;
	uint16_t m_crt_bad = 0;
	uint8_t m_crt_sts = 0;
	uint8_t m_crt_ctl = 0;
	uint8_t m_crt_read_byte = 0;
	uint8_t m_crt_write_byte = 0;

	// Printer
	uint8_t m_prtlen = 0;
	uint8_t m_prt_idx = 0;
	uint8_t m_prchar_r = 0;
	uint8_t m_prchar_w = 0;
	uint8_t m_prtsts = 0;
	uint8_t m_prtctl = 0;
	uint8_t m_prt_buffer[ PRT_BUFFER_SIZE ]{};

	attotime time_to_video_mem_availability() const;
	static void get_video_addr(uint16_t addr , uint16_t& byte_addr , bool& lsb_nibble);
	uint8_t video_mem_r(uint16_t addr , uint16_t addr_mask) const;
	void video_mem_w(uint16_t addr , uint16_t addr_mask , uint8_t data);
	void video_mem_read();
	void video_mem_write();

	uint8_t get_prt_font(uint8_t ch , unsigned col) const;
	void prt_format_alpha(unsigned row , uint8_t *pixel_row) const;
	void prt_format_graphic(unsigned row , uint8_t *pixel_row) const;
	void prt_output_row(const uint8_t *pixel_row);
	void prt_do_printing();
};

hp85_state::hp85_state(const machine_config &mconfig, device_type type, const char *tag)
	: hp80_base_state(mconfig , type , tag),
	  m_screen(*this , "screen"),
	  m_palette(*this , "palette"),
	  m_vm_timer(*this , "vm_timer"),
	  m_prt_busy_timer(*this , "prt_busy_timer"),
	  m_prt_graph_out(*this , "prt_graphic"),
	  m_prt_alpha_out(*this , "prt_alpha"),
	  m_chargen(*this , "chargen"),
	  m_prt_chargen(*this , "prt_chargen")
{
}

void hp85_state::machine_start()
{
	hp80_base_state::machine_start();
	m_screen->register_screen_bitmap(m_bitmap);
	m_video_mem.resize(VIDEO_MEM_SIZE);
}

void hp85_state::machine_reset()
{
	hp80_base_state::machine_reset();

	m_crt_sad = 0;
	m_crt_bad = 0;
	m_crt_sts = 0x7c;
	m_crt_ctl = BIT_MASK<uint8_t>(CRT_CTL_POWERDN_BIT) | BIT_MASK<uint8_t>(CRT_CTL_WIPEOUT_BIT);
	m_crt_read_byte = 0;
	m_crt_write_byte = 0;
	m_prtlen = 0;
	m_prt_idx = PRT_BUFFER_SIZE;
	m_prchar_r = 0;
	m_prchar_w = 0;
	m_prtsts = BIT_MASK<uint8_t>(PRTSTS_PAPER_OK_BIT) | BIT_MASK<uint8_t>(PRTSTS_PRTRDY_BIT);
	m_prtctl = 0;
}

uint32_t hp85_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

void hp85_state::vblank_w(int state)
{
	COPY_BIT(!state , m_crt_sts , CRT_STS_DISPLAY_BIT);
	if (state) {
		if (BIT(m_crt_ctl , CRT_CTL_WIPEOUT_BIT) || BIT(m_crt_ctl , CRT_CTL_POWERDN_BIT)) {
			// Blank video
			m_bitmap.fill(rgb_t::black());
		} else if (BIT(m_crt_ctl , CRT_CTL_GRAPHICS_BIT)) {
			// Render graphic video
			uint16_t video_start = m_crt_sad;
			for (unsigned y = 0; y < 192; y++) {
				for (unsigned x = 0; x < 256; x += 8) {
					uint8_t pixels = video_mem_r(video_start , GRAPH_MEM_SIZE / 2 - 1);
					video_start += 2;
					for (unsigned sub_x = 0; sub_x < 8; sub_x++) {
						m_bitmap.pix(y , x + sub_x) = m_palette->pen(BIT(pixels , 7));
						pixels <<= 1;
					}
				}
			}
		} else {
			// Render alpha video
			uint16_t video_start = m_crt_sad;
			for (unsigned row = 0; row < 192; row += 12) {
				for (unsigned col = 0; col < 256; col += 8) {
					uint8_t ch = video_mem_r(video_start , ALPHA_MEM_SIZE / 2 - 1);
					video_start += 2;
					for (unsigned sub_row = 0; sub_row < 12; sub_row++) {
						uint8_t pixels;
						if (sub_row < 8) {
							pixels = m_chargen[ (ch & 0x7f) * 8 + sub_row ];
						} else if (BIT(ch , 7) && (sub_row == 9 || sub_row == 10)) {
							// Underline
							pixels = 0xfe;
						} else {
							pixels = 0;
						}
						for (unsigned sub_x = 0; sub_x < 8; sub_x++) {
							m_bitmap.pix(row + sub_row , col + sub_x) = m_palette->pen(BIT(pixels , 7));
							pixels <<= 1;
						}
					}
				}
			}
		}
	}
}

uint8_t hp85_state::crtc_r(offs_t offset)
{
	uint8_t res = 0xff;

	// Read from CRT controller (1MA5)
	switch (offset) {
	case 0:
		// CRTSAD: write-only
		break;

	case 1:
		// CRTBAD: write-only
		break;

	case 2:
		// CRTSTS
		res = m_crt_sts;
		break;

	case 3:
		// CRTDAT
		res = m_crt_read_byte;
		break;
	}
	return res;
}

void hp85_state::crtc_w(offs_t offset, uint8_t data)
{
	// Write to CRT controller (1MA5)
	uint8_t burst_idx = m_cpu->flatten_burst();
	switch (offset) {
	case 0:
		// CRTSAD
		if (burst_idx == 1) {
			m_crt_sad = ((uint16_t)data << 8) | (m_crt_sad & 0xff);
		} else if (burst_idx == 0) {
			m_crt_sad = (m_crt_sad & 0xff00) | data;
		}
		break;

	case 1:
		// CRTBAD
		if (burst_idx == 1) {
			m_crt_bad = ((uint16_t)data << 8) | (m_crt_bad & 0xff);
		} else if (burst_idx == 0) {
			m_crt_bad = (m_crt_bad & 0xff00) | data;
		}
		break;

	case 2:
		// CRTCTL
		m_crt_ctl = data;
		if (BIT(m_crt_ctl , CRT_CTL_RD_RQ_BIT)) {
			BIT_CLR(m_crt_sts , CRT_STS_READY_BIT);
			BIT_SET(m_crt_sts , CRT_STS_BUSY_BIT);
			attotime vm_av = time_to_video_mem_availability();
			m_vm_timer->adjust(vm_av + attotime::from_ticks(CRT_RW_TIME , MASTER_CLOCK));
		}
		break;

	case 3:
		// CRTDAT
		{
			m_crt_write_byte = data;
			BIT_CLR(m_crt_sts , CRT_STS_READY_BIT);
			BIT_SET(m_crt_sts , CRT_STS_BUSY_BIT);
			attotime vm_av = time_to_video_mem_availability();
			m_vm_timer->adjust(vm_av + attotime::from_ticks(CRT_RW_TIME , MASTER_CLOCK));
		}
		break;
	}
}

void hp85_state::prtlen_w(uint8_t data)
{
	if (data == 0) {
		// Advance paper
		memset(m_prt_buffer , 0 , sizeof(m_prt_buffer));
		m_prt_idx = 0;
		prt_do_printing();
	} else {
		m_prtlen = data;
		if (!BIT(m_prtctl , PRTCTL_GRAPHIC_BIT)) {
			m_prt_idx = 0;
		}
	}
}

uint8_t hp85_state::prchar_r()
{
	return m_prchar_r;
}

void hp85_state::prchar_w(uint8_t data)
{
	m_prchar_w = data;
}

uint8_t hp85_state::prtsts_r()
{
	return m_prtsts;
}

void hp85_state::prtctl_w(uint8_t data)
{
	m_prtctl = data;
	BIT_SET(m_prtsts , PRTSTS_PRTRDY_BIT);
	if (BIT(m_prtctl , PRTCTL_READGEN_BIT)) {
		// Reading printer char. gen.
		m_prchar_r = get_prt_font(m_prchar_w , m_prtctl & 7);
		BIT_SET(m_prtsts , PRTSTS_DATARDY_BIT);
	} else {
		BIT_CLR(m_prtsts , PRTSTS_DATARDY_BIT);
	}
	if (BIT(m_prtctl , PRTCTL_GRAPHIC_BIT)) {
		m_prt_idx = 0;
	}
}

void hp85_state::prtdat_w(uint8_t data)
{
	m_cpu->flatten_burst();
	if (m_prt_idx < PRT_BUFFER_SIZE) {
		m_prt_buffer[ m_prt_idx++ ] = data;
		if (m_prt_idx == PRT_BUFFER_SIZE || (!BIT(m_prtctl , PRTCTL_GRAPHIC_BIT) && m_prt_idx >= m_prtlen)) {
			prt_do_printing();
			m_prt_idx = PRT_BUFFER_SIZE;
		}
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(hp85_state::prt_busy_timer)
{
	BIT_SET(m_prtsts , PRTSTS_PRTRDY_BIT);
}

TIMER_DEVICE_CALLBACK_MEMBER(hp85_state::vm_timer)
{
	if (BIT(m_crt_ctl , CRT_CTL_RD_RQ_BIT)) {
		video_mem_read();
	} else {
		video_mem_write();
	}
	BIT_CLR(m_crt_sts , CRT_STS_BUSY_BIT);
}

attotime hp85_state::time_to_video_mem_availability() const
{
	if (BIT(m_crt_ctl , CRT_CTL_WIPEOUT_BIT) || BIT(m_crt_ctl , CRT_CTL_POWERDN_BIT)) {
		// Blank video, immediate access
		return attotime::zero;
	} else if (m_screen->vblank()) {
		// Vertical blanking, immediate access
		return attotime::zero;
	} else {
		// In the active part, wait until vertical blanking
		return m_screen->time_until_vblank_start();
	}
}

void hp85_state::get_video_addr(uint16_t addr , uint16_t& byte_addr , bool& lsb_nibble)
{
	byte_addr = (addr / 2) & (VIDEO_MEM_SIZE - 1);
	lsb_nibble = BIT(addr , 0);
}

uint8_t hp85_state::video_mem_r(uint16_t addr , uint16_t addr_mask) const
{
	uint16_t byte_addr;
	bool lsb_nibble;

	get_video_addr(addr , byte_addr , lsb_nibble);

	byte_addr &= addr_mask;

	uint8_t res;

	if (lsb_nibble) {
		res = (m_video_mem[ byte_addr ] & 0x0f) << 4;
		byte_addr = (byte_addr + 1) & addr_mask;
		res |= (m_video_mem[ byte_addr ] & 0xf0) >> 4;
	} else {
		res = m_video_mem[ byte_addr ];
	}

	return res;
}

void hp85_state::video_mem_w(uint16_t addr , uint16_t addr_mask , uint8_t data)
{
	uint16_t byte_addr;
	bool lsb_nibble;

	get_video_addr(addr , byte_addr , lsb_nibble);

	byte_addr &= addr_mask;

	if (lsb_nibble) {
		m_video_mem[ byte_addr ] = (m_video_mem[ byte_addr ] & 0xf0) | (data >> 4);
		byte_addr = (byte_addr + 1) & addr_mask;
		m_video_mem[ byte_addr ] = (m_video_mem[ byte_addr ] & 0x0f) | (data << 4);
	} else {
		m_video_mem[ byte_addr ] = data;
	}
}

void hp85_state::video_mem_read()
{
	uint16_t mask;

	if (BIT(m_crt_ctl , CRT_CTL_GRAPHICS_BIT)) {
		mask = GRAPH_MEM_SIZE / 2 - 1;
	} else {
		mask = ALPHA_MEM_SIZE / 2 - 1;
	}
	m_crt_read_byte = video_mem_r(m_crt_bad , mask);
	m_crt_bad += 2;
	BIT_CLR(m_crt_ctl , CRT_CTL_RD_RQ_BIT);
	BIT_SET(m_crt_sts , CRT_STS_READY_BIT);
}

void hp85_state::video_mem_write()
{
	uint16_t mask;

	if (BIT(m_crt_ctl , CRT_CTL_GRAPHICS_BIT)) {
		mask = GRAPH_MEM_SIZE / 2 - 1;
	} else {
		mask = ALPHA_MEM_SIZE / 2 - 1;
	}
	video_mem_w(m_crt_bad , mask , m_crt_write_byte);
	m_crt_bad += 2;
}

uint8_t hp85_state::get_prt_font(uint8_t ch , unsigned col) const
{
	// Bit 7: pixel @ top
	// Bit 0: pixel @ bottom
	uint8_t column = m_prt_chargen[ (((unsigned)ch & 0x7f) << 3) | col ];
	if (BIT(ch , 7)) {
		// Underline
		BIT_SET(column , 0);
	}
	return column;
}

void hp85_state::prt_format_alpha(unsigned row , uint8_t *pixel_row) const
{
	memset(pixel_row , 0 , PRT_WIDTH);
	for (unsigned i = 0; i < m_prt_idx; i++) {
		for (unsigned j = 0; j < PRT_CELL_WIDTH; j++) {
			uint8_t pixel_col = get_prt_font(m_prt_buffer[ i ] , j);
			*pixel_row++ = BIT(pixel_col , 7 - row);
		}
	}
}

void hp85_state::prt_format_graphic(unsigned row , uint8_t *pixel_row) const
{
	memset(pixel_row , 0 , PRT_WIDTH);
	pixel_row += PRT_GRAPH_OFFSET;
	for (unsigned i = 0; i < PRT_GRAPH_WIDTH; i++) {
		*pixel_row++ = BIT(m_prt_buffer[ i ] , 7 - row);
	}
}

void hp85_state::prt_output_row(const uint8_t *pixel_row)
{
	for (unsigned i = 0; i < PRT_WIDTH; i++) {
		m_prt_graph_out->output(*pixel_row++ != 0 ? '*' : ' ');
	}
	m_prt_graph_out->output('\n');
}

void hp85_state::prt_do_printing()
{
	uint8_t pixel_row[ PRT_WIDTH ];
	for (unsigned row = 0; row < PRT_PH_HEIGHT; row++) {
		if (BIT(m_prtctl , PRTCTL_GRAPHIC_BIT)) {
			prt_format_graphic(row , pixel_row);
		} else {
			prt_format_alpha(row , pixel_row);
		}
		prt_output_row(pixel_row);
	}
	if (!BIT(m_prtctl , PRTCTL_GRAPHIC_BIT)) {
		// Dump the text line to alpha bitbanger
		for (unsigned i = 0; i < m_prt_idx; i++) {
			m_prt_alpha_out->output(m_prt_buffer[ i ]);
		}
		m_prt_alpha_out->output('\n');
		// Add 2 empty lines
		memset(pixel_row , 0 , PRT_WIDTH);
		for (unsigned i = 0; i < (PRT_ALPHA_HEIGHT - PRT_PH_HEIGHT); i++) {
			prt_output_row(pixel_row);
		}
	}
	// Start busy timer
	BIT_CLR(m_prtsts , PRTSTS_PRTRDY_BIT);
	m_prt_busy_timer->adjust(attotime::from_msec(PRT_BUSY_MSEC));
}

#define IOP_MASK(x) BIT_MASK<ioport_value>((x))

static INPUT_PORTS_START(hp85)
	// Keyboard is arranged in a matrix of 10 rows and 8 columns. In addition there are 3 keys with
	// dedicated input lines: SHIFT, SHIFT LOCK & CONTROL.
	// A key on row "r"=[0..9] and column "c"=[0..7] is mapped to bit "b" of KEY"n" input, where
	// n = r / 4
	// b = (r % 4) * 8 + c
	PORT_START("KEY0")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_NAME("Down AUTO") // 0,0: Down / Auto
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_NAME("Up Home")       // 0,1: Up / Home
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_NAME("k4 k8")         // 0,2: k4 / k8
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_NAME("k3 k7")         // 0,3: k3 / k7
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_NAME("k2 k6")         // 0,4: k2 / k6
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_NAME("k1 k5")         // 0,5: k1 / k5
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("LABEL KEY")                                                        // 0,6: LABEL KEY
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                 // 0,7: N/U
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')                            // 1,0: 8
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')                            // 1,1: 7
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')                           // 1,2: 6
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')                           // 1,3: 5
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')                           // 1,4: 4
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')                           // 1,5: 3
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')                           // 1,6: 2
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')                           // 1,7: 1
	PORT_BIT(IOP_MASK(16) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')                           // 2,0: I
	PORT_BIT(IOP_MASK(17) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')                           // 2,1: U
	PORT_BIT(IOP_MASK(18) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')                           // 2,2: Y
	PORT_BIT(IOP_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')                           // 2,3: T
	PORT_BIT(IOP_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')                           // 2,4: R
	PORT_BIT(IOP_MASK(21) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')                           // 2,5: E
	PORT_BIT(IOP_MASK(22) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')                           // 2,6: W
	PORT_BIT(IOP_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')                           // 2,7: Q
	PORT_BIT(IOP_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')                           // 3,0: K
	PORT_BIT(IOP_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')                           // 3,1: J
	PORT_BIT(IOP_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')                           // 3,2: H
	PORT_BIT(IOP_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')                           // 3,3: G
	PORT_BIT(IOP_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')                           // 3,4: F
	PORT_BIT(IOP_MASK(29) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')                           // 3,5: D
	PORT_BIT(IOP_MASK(30) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')                           // 3,6: S
	PORT_BIT(IOP_MASK(31) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')                           // 3,7: A

	PORT_START("KEY1")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')                            // 4,0: M
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')                            // 4,1: N
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')                            // 4,2: B
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')                            // 4,3: V
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')                            // 4,4: C
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')                            // 4,5: X
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')                            // 4,6: Z
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')                                       // 4,7: Space
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')                        // 5,0: ,
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')                         // 5,1: .
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')                       // 5,2: / ?
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("PAUSE STEP")                                                      // 5,3: PAUSE / STEP
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("RUN")                                                             // 5,4: RUN
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD)) PORT_NAME("KP +") // 5,5: KP +
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_NAME("KP -") // 5,6: KP -
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK)) PORT_NAME("KP *") // 5,7: KP * (not sure)
	PORT_BIT(IOP_MASK(16) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')                           // 6,0: L
	PORT_BIT(IOP_MASK(17) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')                       // 6,1: ;
	PORT_BIT(IOP_MASK(18) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')                      // 6,2: ' "
	PORT_BIT(IOP_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("END LINE")                 // 6,3: END LINE
	PORT_BIT(IOP_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("LIST P LST")                                                      // 6,4: LIST / P LST
	PORT_BIT(IOP_MASK(21) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                // 6,5: N/U
	PORT_BIT(IOP_MASK(22) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                // 6,6: N/U
	PORT_BIT(IOP_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) PORT_NAME("KP /") // 6,7: KP /
	PORT_BIT(IOP_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')                           // 7,0: O
	PORT_BIT(IOP_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')                           // 7,1: P
	PORT_BIT(IOP_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('(') PORT_CHAR('[')                   // 7,2: ( [
	PORT_BIT(IOP_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(')') PORT_CHAR(']')                  // 7,3: ) ]
	PORT_BIT(IOP_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("CONT SCRATCH")                                                    // 7,4: CONT / SCRATCH
	PORT_BIT(IOP_MASK(29) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("-LINE CLEAR")                                                     // 7,5: -LINE / CLEAR
	PORT_BIT(IOP_MASK(30) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME(") INIT")                                                          // 7,6: ) INIT
	PORT_BIT(IOP_MASK(31) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                // 7,7: N/U

	PORT_START("KEY2")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')                            // 8,0: 9
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')                            // 8,1: 0
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')                        // 8,2: - _
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')                       // 8,3: = +
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('\\') PORT_CHAR('|')                       // 8,4: \ |
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)                                     // 8,5: BS
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("( RESET")                                                          // 8,6: ( RESET
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("^ RESLT")                                                          // 8,7: ^ / RESLT
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))    PORT_NAME("Left GRAPH") // 9,0: LEFT / GRAPH
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME("Right COPY")  // 9,1: RIGHT / COPY
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT) PORT_NAME("RPL INS")                               // 9,2: RPL / INS
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_NAME("-CHAR DEL")                                // 9,3: -CHAR / DEL
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("ROLL")                                    // 9,4: ROLL
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("LOAD REW")                                                        // 9,5: LOAD / REW
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("STORE TEST")                                                      // 9,6: STORE / TEST
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("PAPER ADVANCE")                                                   // 9,7: PAPER ADVANCE

	PORT_START("MODKEYS")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)                // Shift
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_NAME("Shift lock")   // Shift lock
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)              // Control

INPUT_PORTS_END

void hp85_state::cpu_mem_map(address_map &map)
{
	hp80_base_state::cpu_mem_map(map);
	map(0x8000, 0xbfff).ram();
	map(0xff04, 0xff07).rw(FUNC(hp85_state::crtc_r), FUNC(hp85_state::crtc_w));
	map(0xff08, 0xff09).rw("tape", FUNC(hp_1ma6_device::reg_r), FUNC(hp_1ma6_device::reg_w));
	map(0xff0c, 0xff0c).w(FUNC(hp85_state::prtlen_w));
	map(0xff0d, 0xff0d).rw(FUNC(hp85_state::prchar_r), FUNC(hp85_state::prchar_w));
	map(0xff0e, 0xff0e).rw(FUNC(hp85_state::prtsts_r), FUNC(hp85_state::prtctl_w));
	map(0xff0f, 0xff0f).w(FUNC(hp85_state::prtdat_w));
}

void hp85_state::unmap_optroms(address_space &space)
{
	// OptROMs are in rombanks [01..FF]
	space.unmap_read(HP80_OPTROM_SIZE * 1 , HP80_OPTROM_SIZE * 0x100 - 1);
}

void hp85_state::hp85(machine_config &config)
{
	hp80_base(config);

	m_cpu->set_addrmap(AS_PROGRAM, &hp85_state::cpu_mem_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK / 2 , 312 , 0 , 256 , 256 , 0 , 192);
	m_screen->set_screen_update(FUNC(hp85_state::screen_update));
	m_screen->screen_vblank().set(FUNC(hp85_state::vblank_w));
	PALETTE(config, m_palette, palette_device::MONOCHROME);
	TIMER(config, m_vm_timer).configure_generic(FUNC(hp85_state::vm_timer));

	TIMER(config, m_prt_busy_timer).configure_generic(FUNC(hp85_state::prt_busy_timer));

	// Tape drive
	HP_1MA6(config, "tape", 0);

	// Printer output
	BITBANGER(config, m_prt_graph_out, 0);
	BITBANGER(config, m_prt_alpha_out, 0);

	SOFTWARE_LIST(config, "optrom_list").set_original("hp85_rom");
}

ROM_START(hp85)
	ROM_REGION(0x6000 , "cpu" , 0)
	ROM_LOAD("romsys1.bin" , 0x0000 , 0x2000 , CRC(7724b1e9) SHA1(7836195389de2ac0eab7199835f5dc8f7dc41729))
	ROM_LOAD("romsys2.bin" , 0x2000 , 0x2000 , CRC(50a85263) SHA1(3cf1d08749103ee245d572550ba1b053ffc7ef57))
	ROM_LOAD("romsys3.bin" , 0x4000 , 0x2000 , CRC(0df385f0) SHA1(4c5ce5afd28f6d776f16cabbbbcc09769ff306b7))

	ROM_REGION(0x2000 , "rombank" , 0)
	ROM_LOAD("rom000.bin" , 0 , 0x2000 , CRC(e13b8ae3) SHA1(2374618d25d1a000ddb534ae4f55ebd98ce0fff3))

	ROM_REGION(0x400 , "chargen" , 0)
	ROM_LOAD("chrgen.bin" , 0 , 0x400 , CRC(9c402544) SHA1(32634fc73c1544aeeefda62ebb10349c5b40729f))

	ROM_REGION(0x400 , "prt_chargen" , 0)
	ROM_LOAD("prt_chrgen.bin" , 0 , 0x400 , CRC(abeaba27) SHA1(fbf6bdd5d96df6aa5963f8cdfdeb180402b1cc85))
ROM_END

// ************
//  hp86_state
// ************
class hp86_state : public hp80_base_state
{
public:
	hp86_state(const machine_config &mconfig, device_type type, const char *tag, bool has_int_keyb = false);

	// **** Constants of HP86 ****
	static constexpr unsigned MASTER_CLOCK  = 12260000;
	static constexpr unsigned VIDEO_MEM_SIZE    = 16384;
	static constexpr uint16_t VIDEO_ADDR_MASK   = VIDEO_MEM_SIZE - 1;
	static constexpr uint16_t VIDEO_ALPHA_N_END = 0x10e0;
	static constexpr uint16_t VIDEO_ALPHA_A_END = 0x3fc0;
	static constexpr uint16_t VIDEO_GRAPH_START = 0x10e0;
	// Time to read/write a byte in video memory (in master clock cycles) TBC
	static constexpr unsigned CRT_RW_TIME       = 24;
	// Duration of on/off states of run light (ms)
	static constexpr unsigned RULITE_ON_MS      = 373;
	static constexpr unsigned RULITE_OFF_MS     = 187;

	void hp86(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void cpu_mem_map(address_map &map) override ATTR_COLD;
	virtual void rombank_mem_map(address_map &map) override ATTR_COLD;
	virtual void unmap_optroms(address_space &space) override;

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank_w(int state);
	attotime time_to_video_mem_availability() const;

	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<timer_device> m_vm_timer;
	required_device<ram_device> m_ram;
	output_finder<> m_run_light;
	required_device<timer_device> m_rulite_timer;

	// Character generator
	required_region_ptr<uint8_t> m_chargen;

	// Video
	bitmap_rgb32 m_bitmap;
	std::unique_ptr<uint8_t []> m_video_mem;
	uint16_t m_crt_sad = 0;
	uint16_t m_crt_bad = 0;
	uint8_t m_crt_sts = 0;
	uint8_t m_crt_byte = 0;
	bool m_crt_rdrq = false;

	// Extended RAM access
	uint32_t m_emc_ptr1 = 0;    // PTR1 (24 bits)
	uint32_t m_emc_ptr2 = 0;    // PTR2 (24 bits)
	uint8_t m_emc_disp = 0;     // Displacement (3 bits)
	bool m_emc_mult = false;        // Multibyte access
	uint8_t m_emc_mode = 0;     // Mode (3 bits)
	enum {
		  EMC_IDLE,
		  EMC_INDIRECT_1,
		  EMC_INDIRECT_2
	};
	int m_emc_state = 0;        // EMC indirect access state
	bool m_lmard = false;           // LMARD cycles in progress

	// Run light
	bool m_rulite = false;

	void crtsad_w(uint8_t data);
	void crtbad_w(uint8_t data);
	uint8_t crtsts_r();
	void crtsts_w(uint8_t data);
	uint8_t crtdat_r();
	void crtdat_w(uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(vm_timer);
	uint16_t get_video_limit() const;
	void rulite_w(uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(rulite_timer);
	uint8_t direct_ram_r(offs_t offset);
	void direct_ram_w(offs_t offset, uint8_t data);
	uint8_t emc_r(offs_t offset);
	void emc_w(offs_t offset, uint8_t data);
	uint32_t& get_ptr();
	void ptr12_decrement();
	void lma_cycle(int state);
	void opcode_cb(uint8_t opcode);
};

hp86_state::hp86_state(const machine_config &mconfig, device_type type, const char *tag, bool has_int_keyb)
	: hp80_base_state(mconfig , type , tag , has_int_keyb)
	, m_screen(*this , "screen")
	, m_palette(*this , "palette")
	, m_vm_timer(*this , "vm_timer")
	, m_ram(*this , "ram")
	, m_run_light(*this , "run_light")
	, m_rulite_timer(*this , "rulite_timer")
	, m_chargen(*this , "chargen")
{
}

void hp86_state::cpu_mem_map(address_map &map)
{
	hp80_base_state::cpu_mem_map(map);
	map(0x8000 , 0xfeff).rw(FUNC(hp86_state::direct_ram_r) , FUNC(hp86_state::direct_ram_w));
	map(0xffc0 , 0xffc0).w(FUNC(hp86_state::crtsad_w));
	map(0xffc1 , 0xffc1).w(FUNC(hp86_state::crtbad_w));
	map(0xffc2 , 0xffc2).rw(FUNC(hp86_state::crtsts_r) , FUNC(hp86_state::crtsts_w));
	map(0xffc3 , 0xffc3).rw(FUNC(hp86_state::crtdat_r) , FUNC(hp86_state::crtdat_w));
	map(0xffc4 , 0xffc4).w(FUNC(hp86_state::rulite_w));
	map(0xffc8 , 0xffcf).rw(FUNC(hp86_state::emc_r) , FUNC(hp86_state::emc_w));
}

void hp86_state::rombank_mem_map(address_map &map)
{
	hp80_base_state::rombank_mem_map(map);
	// rom001 (graphics)
	map(0x2000, 0x3fff).rom();
	// rom320 (mass memory)
	// rom321 (electronic disk)
	map(0x1a0000 , 0x1a3fff).rom();
}

void hp86_state::unmap_optroms(address_space &space)
{
	// OptROMs are in rombanks [02..CF] & [D2..FF]
	space.unmap_read(HP80_OPTROM_SIZE * 2 , HP80_OPTROM_SIZE * 0xd0 - 1);
	space.unmap_read(HP80_OPTROM_SIZE * 0xd2 , HP80_OPTROM_SIZE * 0x100 - 1);
}

void hp86_state::hp86(machine_config &config)
{
	hp80_base(config);

	m_cpu->opcode_cb().set(FUNC(hp86_state::opcode_cb));
	m_cpu->lma_cb().set(FUNC(hp86_state::lma_cycle));

	RAM(config , m_ram).set_default_size("128K");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK , 784 , 0 , 640 , 261 , 0 , 240);
	m_screen->set_screen_update(FUNC(hp86_state::screen_update));
	m_screen->screen_vblank().set(FUNC(hp86_state::vblank_w));
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	config.set_default_layout(layout_hp86b);

	TIMER(config, m_vm_timer).configure_generic(FUNC(hp86_state::vm_timer));
	TIMER(config, m_rulite_timer).configure_generic(FUNC(hp86_state::rulite_timer));

	m_io_slots[ 0 ]->option_set("hpib" , HP82937_IO_CARD);

	SOFTWARE_LIST(config, "optrom_list").set_original("hp86_rom");
}

void hp86_state::machine_start()
{
	hp80_base_state::machine_start();

	m_run_light.resolve();

	m_screen->register_screen_bitmap(m_bitmap);
	m_video_mem = std::make_unique<uint8_t[]>(VIDEO_MEM_SIZE);

	save_pointer(NAME(m_video_mem) , VIDEO_MEM_SIZE);
	save_item(NAME(m_crt_sad));
	save_item(NAME(m_crt_bad));
	save_item(NAME(m_emc_ptr1));
	save_item(NAME(m_emc_ptr2));
	save_item(NAME(m_emc_disp));
	save_item(NAME(m_emc_mult));
	save_item(NAME(m_emc_mode));
	save_item(NAME(m_rulite));

	m_emc_ptr1 = 0;
	m_emc_ptr2 = 0;
}

void hp86_state::machine_reset()
{
	hp80_base_state::machine_reset();

	m_crt_sad = 0;
	m_crt_sts = 0x06;
	m_crt_rdrq = false;
	m_emc_state = EMC_IDLE;
	m_rulite = true;
	m_run_light = true;
	m_rulite_timer->reset();
}

uint32_t hp86_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

void hp86_state::vblank_w(int state)
{
	COPY_BIT(state , m_crt_sts , 4);
	if (state) {
		if (m_crt_sts & 0x06) {
			// Blank display
			m_bitmap.fill(rgb_t::black());
		} else {
			// Load palette for normal or inverse video
			if (BIT(m_crt_sts , 5)) {
				m_palette->set_pen_color(0 , rgb_t::green());
				m_palette->set_pen_color(1 , rgb_t::black());
			} else {
				m_palette->set_pen_color(1 , rgb_t::green());
				m_palette->set_pen_color(0 , rgb_t::black());
			}
			uint16_t limit = get_video_limit();
			if (BIT(m_crt_sts , 7)) {
				uint16_t video_ptr = VIDEO_GRAPH_START;
				unsigned dots_per_line;
				unsigned offset;
				if (BIT(m_crt_sts , 6)) {
					// GRAPH ALL mode
					dots_per_line = 544;
					offset = 48;
				} else {
					// GRAPH NORMAL mode
					dots_per_line = 400;
					offset = 120;
				}
				// Fill black bars on either side of the display
				m_bitmap.fill(m_palette->pen(0) , rectangle(0 , offset - 1 , 0 , 239));
				m_bitmap.fill(m_palette->pen(0) , rectangle(640 - offset , 639 , 0 , 239));

				for (unsigned y = 0; y < 240; y++) {
					for (unsigned x = offset; x < (dots_per_line + offset); x += 8) {
						uint8_t pixels = m_video_mem[ video_ptr ];
						if (++video_ptr >= limit) {
							video_ptr = 0;
						}
						m_bitmap.pix(y , x) = m_palette->pen(BIT(pixels , 7));
						m_bitmap.pix(y , x + 1) = m_palette->pen(BIT(pixels , 6));
						m_bitmap.pix(y , x + 2) = m_palette->pen(BIT(pixels , 5));
						m_bitmap.pix(y , x + 3) = m_palette->pen(BIT(pixels , 4));
						m_bitmap.pix(y , x + 4) = m_palette->pen(BIT(pixels , 3));
						m_bitmap.pix(y , x + 5) = m_palette->pen(BIT(pixels , 2));
						m_bitmap.pix(y , x + 6) = m_palette->pen(BIT(pixels , 1));
						m_bitmap.pix(y , x + 7) = m_palette->pen(BIT(pixels , 0));
					}
				}
			} else {
				unsigned rows;
				unsigned lines_per_row;
				if (BIT(m_crt_sts , 3)) {
					// 24 rows
					rows = 24;
					lines_per_row = 10;
				} else {
					// 16 rows
					rows = 16;
					lines_per_row = 15;
				}
				uint16_t video_ptr = m_crt_sad;
				for (unsigned row = 0; row < rows; row++) {
					for (unsigned col = 0; col < 640; col += 8) {
						uint8_t ch = m_video_mem[ video_ptr ];
						if (++video_ptr >= limit) {
							video_ptr = 0;
						}
						for (unsigned sub_row = 0; sub_row < lines_per_row; sub_row++) {
							uint8_t pixels;
							if (sub_row < 10) {
								pixels = m_chargen[ (ch & 0x7f) * 10 + sub_row ];
							} else {
								pixels = 0;
							}
							if (BIT(ch , 7)) {
								pixels = ~pixels;
							}
							unsigned y = row * lines_per_row + sub_row;
							m_bitmap.pix(y , col) = m_palette->pen(BIT(pixels , 7));
							m_bitmap.pix(y , col + 1) = m_palette->pen(BIT(pixels , 6));
							m_bitmap.pix(y , col + 2) = m_palette->pen(BIT(pixels , 5));
							m_bitmap.pix(y , col + 3) = m_palette->pen(BIT(pixels , 4));
							m_bitmap.pix(y , col + 4) = m_palette->pen(BIT(pixels , 3));
							m_bitmap.pix(y , col + 5) = m_palette->pen(BIT(pixels , 2));
							m_bitmap.pix(y , col + 6) = m_palette->pen(BIT(pixels , 1));
							m_bitmap.pix(y , col + 7) = m_palette->pen(BIT(pixels , 0));
						}
					}
				}
			}
		}
	}
}

attotime hp86_state::time_to_video_mem_availability() const
{
	if ((m_crt_sts & 0x06) != 0 || m_screen->vblank() || m_screen->hblank()) {
		// Blank video or vertical/horizontal retrace: immediate access
		return attotime::zero;
	} else {
		// In the active part, wait until next retrace
		return m_screen->time_until_pos(m_screen->vpos() , 640);
	}
}

void hp86_state::crtsad_w(uint8_t data)
{
	auto burst_idx = m_cpu->flatten_burst();
	if (burst_idx == 0) {
		m_crt_sad = (m_crt_sad & 0xff00) | data;
	} else if (burst_idx == 1) {
		m_crt_sad = (uint16_t(data) << 8) | (m_crt_sad & 0xff);
		m_crt_sad &= VIDEO_ADDR_MASK;
	}
}

void hp86_state::crtbad_w(uint8_t data)
{
	auto burst_idx = m_cpu->flatten_burst();
	if (burst_idx == 0) {
		m_crt_bad = (m_crt_bad & 0xff00) | data;
	} else if (burst_idx == 1) {
		m_crt_bad = (uint16_t(data) << 8) | (m_crt_bad & 0xff);
		m_crt_bad &= VIDEO_ADDR_MASK;
	}
}

uint8_t hp86_state::crtsts_r()
{
	return m_crt_sts;
}

void hp86_state::crtsts_w(uint8_t data)
{
	m_crt_sts = (m_crt_sts & 0x11) | (data & ~0x11);
	if (BIT(data , 0)) {
		// Read request
		BIT_SET(m_crt_sts , 0);
		m_crt_rdrq = true;
		attotime vm_av = time_to_video_mem_availability();
		m_vm_timer->adjust(vm_av + attotime::from_ticks(CRT_RW_TIME , MASTER_CLOCK));
	}
}

uint8_t hp86_state::crtdat_r()
{
	return m_crt_byte;
}

void hp86_state::crtdat_w(uint8_t data)
{
	m_crt_byte = data;
	BIT_SET(m_crt_sts , 0);
	m_crt_rdrq = false;
	attotime vm_av = time_to_video_mem_availability();
	m_vm_timer->adjust(vm_av + attotime::from_ticks(CRT_RW_TIME , MASTER_CLOCK));
}

TIMER_DEVICE_CALLBACK_MEMBER(hp86_state::vm_timer)
{
	if (m_crt_rdrq) {
		m_crt_rdrq = false;
		m_crt_byte = m_video_mem[ m_crt_bad ];
	} else {
		m_video_mem[ m_crt_bad ] = m_crt_byte;
	}
	BIT_CLR(m_crt_sts , 0);
	if (++m_crt_bad >= get_video_limit()) {
		m_crt_bad = 0;
	}
}

uint16_t hp86_state::get_video_limit() const
{
	if (BIT(m_crt_sts , 7)) {
		// Graphic mode
		return VIDEO_MEM_SIZE;
	} else if (BIT(m_crt_sts , 6)) {
		// ALPHA ALL mode
		return VIDEO_ALPHA_A_END;
	} else {
		// ALPHA NORMAL mode
		return VIDEO_ALPHA_N_END;
	}
}

void hp86_state::rulite_w(uint8_t data)
{
	bool new_rulite = !BIT(data , 0);

	if (m_rulite && !new_rulite) {
		m_run_light = false;
		m_rulite_timer->adjust(attotime::from_msec(RULITE_OFF_MS));
	} else if (!m_rulite && new_rulite) {
		m_run_light = true;
		m_rulite_timer->reset();
	}
	m_rulite = new_rulite;
}

TIMER_DEVICE_CALLBACK_MEMBER(hp86_state::rulite_timer)
{
	m_run_light = !m_run_light;
	m_rulite_timer->adjust(attotime::from_msec(m_run_light ? RULITE_ON_MS : RULITE_OFF_MS));
}

uint8_t hp86_state::direct_ram_r(offs_t offset)
{
	return m_ram->read(offset);
}

void hp86_state::direct_ram_w(offs_t offset, uint8_t data)
{
	m_ram->write(offset , data);
}

uint8_t hp86_state::emc_r(offs_t offset)
{
	auto idx = m_cpu->flatten_burst();
	uint8_t res = 0xff;

	if (m_emc_state == EMC_INDIRECT_2) {
		uint32_t& ptr = get_ptr();
		if (ptr >= 0x8000 && (ptr - 0x8000) < m_ram->size()) {
			res = m_ram->read(ptr - 0x8000);
		}
		LOG_EMC("EMC r @%06x=%02x\n" , ptr , res);
		ptr++;
	} else if (m_lmard) {
		m_emc_mode = uint8_t(offset);
		// During a LMARD pair, address 0xffc8 is returned to CPU and indirect mode is activated
		if (idx == 0) {
			res = 0xc8;
		} else if (idx == 1) {
			LOG_EMC("EMC access %u %06x\n" , m_emc_mode & 7 , get_ptr());
			m_emc_state = EMC_INDIRECT_1;
			if (BIT(m_emc_mode , 0)) {
				// Pre-decrement
				ptr12_decrement();
			}
		}
	} else {
		m_emc_mode = uint8_t(offset);
		// Read PTRx
		if (idx < 3) {
			res = uint8_t(get_ptr() >> (8 * idx));
		}
	}
	return res;
}

void hp86_state::emc_w(offs_t offset, uint8_t data)
{
	auto idx = m_cpu->flatten_burst();

	if (m_emc_state == EMC_INDIRECT_2) {
		uint32_t& ptr = get_ptr();
		LOG_EMC("EMC w @%06x=%02x\n" , ptr , data);
		if (ptr >= 0x8000 && (ptr - 0x8000) < m_ram->size()) {
			m_ram->write(ptr - 0x8000 , data);
		}
		ptr++;
	} else {
		m_emc_mode = uint8_t(offset);
		// Write PTRx
		if (idx < 3) {
			uint32_t& ptr = get_ptr();
			uint32_t mask = 0xffU << (8 * idx);
			ptr = (ptr & ~mask) | (uint32_t(data) << (8 * idx));
		}
	}
}

uint32_t& hp86_state::get_ptr()
{
	return BIT(m_emc_mode , 2) ? m_emc_ptr2 : m_emc_ptr1;
}

void hp86_state::ptr12_decrement()
{
	if (m_emc_mult) {
		get_ptr() -= m_emc_disp;
	} else {
		get_ptr()--;
	}
}

void hp86_state::lma_cycle(int state)
{
	m_lmard = state;
	if (m_emc_state == EMC_INDIRECT_1) {
		m_emc_state = EMC_INDIRECT_2;
	} else if (m_emc_state == EMC_INDIRECT_2) {
		LOG_EMC("EMC close %u %06x\n" , m_emc_mode & 7 , get_ptr());
		if (!BIT(m_emc_mode , 1)) {
			// In PTRx & PTRx- cases, bring the PTR back to start
			ptr12_decrement();
		}
		m_emc_state = EMC_IDLE;
	}
}

void hp86_state::opcode_cb(uint8_t opcode)
{
	// Intercept DRP instructions & load displacement
	if ((opcode & 0xc0) == 0x40) {
		if (BIT(opcode , 5)) {
			m_emc_disp = 8 - (opcode & 7);
		} else {
			m_emc_disp = 2 - (opcode & 1);
		}
	}

	m_emc_mult = BIT(opcode , 0);
}

static INPUT_PORTS_START(hp86)
	// Keyboard is arranged in a matrix of 10 rows and 8 columns. In addition there are 3 keys with
	// dedicated input lines: SHIFT, SHIFT LOCK & CONTROL.
	// A key on row "r"=[0..9] and column "c"=[0..7] is mapped to bit "b" of KEY"n" input, where
	// n = r / 4
	// b = (r % 4) * 8 + c
	PORT_START("KEY0")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_NAME("k6 k13")        // 0,0: k6 / k13
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_NAME("k5 k12")        // 0,1: k5 / k12
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_NAME("k4 k11")        // 0,2: k4 / k11
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_NAME("k3 k10")        // 0,3: k3 / k10
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_NAME("k2 k9")         // 0,4: k2 / k9
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_NAME("k1 k8")         // 0,5: k1 / k8
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("LABEL KEY")                                                        // 0,6: LABEL KEY
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                 // 0,7: N/U
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')                            // 1,0: 8
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')                            // 1,1: 7
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')                           // 1,2: 6
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')                           // 1,3: 5
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')                           // 1,4: 4
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')                           // 1,5: 3
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')                           // 1,6: 2
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')                           // 1,7: 1
	PORT_BIT(IOP_MASK(16) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')                           // 2,0: I
	PORT_BIT(IOP_MASK(17) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')                           // 2,1: U
	PORT_BIT(IOP_MASK(18) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')                           // 2,2: Y
	PORT_BIT(IOP_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')                           // 2,3: T
	PORT_BIT(IOP_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')                           // 2,4: R
	PORT_BIT(IOP_MASK(21) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')                           // 2,5: E
	PORT_BIT(IOP_MASK(22) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')                           // 2,6: W
	PORT_BIT(IOP_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')                           // 2,7: Q
	PORT_BIT(IOP_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')                           // 3,0: K
	PORT_BIT(IOP_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')                           // 3,1: J
	PORT_BIT(IOP_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')                           // 3,2: H
	PORT_BIT(IOP_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')                           // 3,3: G
	PORT_BIT(IOP_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')                           // 3,4: F
	PORT_BIT(IOP_MASK(29) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')                           // 3,5: D
	PORT_BIT(IOP_MASK(30) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')                           // 3,6: S
	PORT_BIT(IOP_MASK(31) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')                           // 3,7: A

	PORT_START("KEY1")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')                            // 4,0: M
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')                            // 4,1: N
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')                            // 4,2: B
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')                            // 4,3: V
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')                            // 4,4: C
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')                            // 4,5: X
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')                            // 4,6: Z
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')                                       // 4,7: Space
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')                        // 5,0: ,
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')                         // 5,1: .
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')                       // 5,2: / ?
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("PAUSE STEP")                                                      // 5,3: PAUSE / STEP
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("RUN")                                                             // 5,4: RUN
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD)) PORT_NAME("KP +") // 5,5: KP +
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_NAME("KP -") // 5,6: KP -
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                // 5,7: N/U
	PORT_BIT(IOP_MASK(16) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')                           // 6,0: L
	PORT_BIT(IOP_MASK(17) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')                       // 6,1: ;
	PORT_BIT(IOP_MASK(18) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')                      // 6,2: ' "
	PORT_BIT(IOP_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("END LINE")                 // 6,3: END LINE
	PORT_BIT(IOP_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("LIST P LST")                                                      // 6,4: LIST / P LST
	PORT_BIT(IOP_MASK(21) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                // 6,5: N/U
	PORT_BIT(IOP_MASK(22) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK)) PORT_NAME("KP *") // 6,6: KP *
	PORT_BIT(IOP_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) PORT_NAME("KP /") // 6,7: KP /
	PORT_BIT(IOP_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')                           // 7,0: O
	PORT_BIT(IOP_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')                           // 7,1: P
	PORT_BIT(IOP_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('(') PORT_CHAR('[')                   // 7,2: ( [
	PORT_BIT(IOP_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(')') PORT_CHAR(']')                  // 7,3: ) ]
	PORT_BIT(IOP_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("CONT TR/NORM")                                                    // 7,4: CONT / TR/NORM
	PORT_BIT(IOP_MASK(29) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("E TEST")                                                          // 7,5: KP E / TEST
	PORT_BIT(IOP_MASK(30) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME(") INIT")                                                          // 7,6: KP ) / INIT
	PORT_BIT(IOP_MASK(31) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                // 7,7: N/U

	PORT_START("KEY2")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')                            // 8,0: 9
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')                            // 8,1: 0
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')                        // 8,2: - _
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')                       // 8,3: = +
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('\\') PORT_CHAR('|')                       // 8,4: \ |
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)                                     // 8,5: BS
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("( RESET")                                                          // 8,6: KP ( / RESET
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("^ RESLT")                                                          // 8,7: KP ^ / RESLT
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_NAME("k7 k14")        // 9,0: k7 / k14
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("-LINE CLEAR")                                                      // 9,1: -LINE / CLEAR
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_NAME("Up Home")      // 9,2: Up / Home
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_NAME("Down A/G") // 9,3: Down / A/G
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME("Left I/R") // 9,4: LEFT / I/R
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME("Right -CHAR") // 9,5: RIGHT / -CHAR
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("ROLL")                                    // 9,6: ROLL
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                // 9,7: n/u

	PORT_START("MODKEYS")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)                // Shift
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_NAME("Shift lock")   // Shift lock
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)              // Control
INPUT_PORTS_END

ROM_START(hp86b)
	ROM_REGION(0x6000 , "cpu" , 0)
	ROM_LOAD("romsys1.bin" , 0x0000 , 0x2000 , CRC(bfa473b8) SHA1(cc420742a5f03c466484a5063e0abcbc084bf298))
	ROM_LOAD("romsys2.bin" , 0x2000 , 0x2000 , CRC(2bc3ba4b) SHA1(760bef9c482f562677f80b18d6163a19ee7aea1c))
	ROM_LOAD("romsys3.bin" , 0x4000 , 0x2000 , CRC(86bf3b8b) SHA1(209c91b9b972ab514c600752e2e4af68f984612e))

	ROM_REGION(0x1a4000 , "rombank" , 0)
	ROM_LOAD("rom000.bin" , 0x0000 , 0x2000 , CRC(c3ca5c54) SHA1(2b291607de101c7206bfae9520a18f1009929e9b))
	ROM_LOAD("rom001.bin" , 0x2000 , 0x2000 , CRC(59a1616c) SHA1(e0fe840f9740bdb455fe1872869671f8712b7cff))
	ROM_LOAD("rom320.bin" , 0x1a0000 , 0x2000 , CRC(c921e2e4) SHA1(e37ac61364830cfa214e6d1b9942cc1cde6ad01f))
	ROM_LOAD("rom321.bin" , 0x1a2000 , 0x2000 , CRC(e6e5cc91) SHA1(67711de228cc48a78d04b13f0a1c91dc26f7e87c))

	ROM_REGION(0x500 , "chargen" , 0)
	ROM_LOAD("chrgen.bin" , 0 , 0x500 , CRC(e90fad22) SHA1(6b2ecef96906ead99cd688e54c507611747c8687))
ROM_END

// ****************
//  hp86_int_state
// ****************
class hp86_int_state : public hp86_state
{
public:
	hp86_int_state(const machine_config &mconfig, device_type type, const char *tag);

protected:
	virtual void rombank_mem_map(address_map &map) override ATTR_COLD;
	virtual void unmap_optroms(address_space &space) override;
};

hp86_int_state::hp86_int_state(const machine_config &mconfig, device_type type, const char *tag)
	: hp86_state(mconfig , type , tag , true)
{
}

void hp86_int_state::rombank_mem_map(address_map &map)
{
	hp86_state::rombank_mem_map(map);
	// rom030 (language)
	map(0x30000, 0x31fff).rom();
}

void hp86_int_state::unmap_optroms(address_space &space)
{
	LOG("hp86_int_state::unmap_optroms\n");
	// OptROMs are in rombanks [02..17] & [19..CF] & [D2..FF]
	space.unmap_read(HP80_OPTROM_SIZE * 2 , HP80_OPTROM_SIZE * 0x18 - 1);
	space.unmap_read(HP80_OPTROM_SIZE * 0x19 , HP80_OPTROM_SIZE * 0xd0 - 1);
	space.unmap_read(HP80_OPTROM_SIZE * 0xd2 , HP80_OPTROM_SIZE * 0x100 - 1);
}

static INPUT_PORTS_START(hp86_int)
	// Keyboard is arranged in a matrix of 11 rows and 8 columns. In addition there are 2 keys with
	// dedicated input lines: SHIFT & CONTROL.
	// A key on row "r"=[0..10] and column "c"=[0..7] is mapped to bit "b" of KEY"n" input, where
	// n = r / 4
	// b = (r % 4) * 8 + c
	PORT_START("KEY0")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')                            // 0,0: K
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')                            // 0,1: M
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')                            // 0,2: N
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')                            // 0,3: B
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')                            // 0,4: V
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')                            // 0,5: Z
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')                            // 0,6: X
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')                            // 0,7: C
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')                            // 1,0: 9
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_NAME("k5 k12")        // 1,1: k5 / k12
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_NAME("k4 k11")       // 1,2: k4 / k11
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_NAME("k3 k10")       // 1,3: k3 / k10
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_NAME("k2 k9")        // 1,4: k2 / k9
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_NAME("Caps")                                // 1,5: Caps
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("LABEL KEY")                                                       // 1,6: LABEL KEY
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_NAME("k1 k8")        // 1,7: k1 / k8
	PORT_BIT(IOP_MASK(16) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')                           // 2,0: I
	PORT_BIT(IOP_MASK(17) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')                           // 2,1: J
	PORT_BIT(IOP_MASK(18) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')                           // 2,2: H
	PORT_BIT(IOP_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')                           // 2,3: G
	PORT_BIT(IOP_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')                           // 2,4: F
	PORT_BIT(IOP_MASK(21) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')                           // 2,5: A
	PORT_BIT(IOP_MASK(22) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')                           // 2,6: S
	PORT_BIT(IOP_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')                           // 2,7: D
	PORT_BIT(IOP_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')                           // 3,0: 8
	PORT_BIT(IOP_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')                           // 3,1: U
	PORT_BIT(IOP_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')                           // 3,2: Y
	PORT_BIT(IOP_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')                           // 3,3: T
	PORT_BIT(IOP_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')                           // 3,4: R
	PORT_BIT(IOP_MASK(29) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')                           // 3,5: Q
	PORT_BIT(IOP_MASK(30) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')                           // 3,6: W
	PORT_BIT(IOP_MASK(31) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')                           // 3,7: E

	PORT_START("KEY1")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')                            // 4,0: 0
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')                       // 4,1: = +
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('\\') PORT_CHAR('|')                       // 4,2: \ |
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)                                     // 4,3: BS
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("E TEST")                                                           // 4,4: KP E / TEST
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("^ RESLT")                                                          // 4,5: KP ^ / RESLT
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME(") INIT")                                                           // 4,6: KP ) / INIT
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("( RESET")                                                          // 4,7: KP ( / RESET
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')                            // 5,0: P
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')                        // 5,1: ; :
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')                      // 5,2: ' "
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("END LINE")                 // 5,3: END LINE
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))                     // 5,4: KP 4
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK)) PORT_NAME("KP *") // 5,5: KP *
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))                     // 5,6: KP 6
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))                     // 5,7: KP 5
	PORT_BIT(IOP_MASK(16) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')                           // 6,0: L
	PORT_BIT(IOP_MASK(17) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')                        // 6,1: .
	PORT_BIT(IOP_MASK(18) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')                       // 6,2: / ?
	PORT_BIT(IOP_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("LIST P LST")                                                      // 6,3: LIST / P LST
	PORT_BIT(IOP_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))                     // 6,4: KP 1
	PORT_BIT(IOP_MASK(21) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_NAME("KP -") // 6,5: KP -
	PORT_BIT(IOP_MASK(22) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))                     // 6,6: KP 3
	PORT_BIT(IOP_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))                     // 6,7: KP 2
	PORT_BIT(IOP_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')                           // 7,0: O
	PORT_BIT(IOP_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')                           // 7,1: 7
	PORT_BIT(IOP_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')                           // 7,2: 6
	PORT_BIT(IOP_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')                           // 7,3: 5
	PORT_BIT(IOP_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')                           // 7,4: 4
	PORT_BIT(IOP_MASK(29) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')                           // 7,5: 1
	PORT_BIT(IOP_MASK(30) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')                           // 7,6: 2
	PORT_BIT(IOP_MASK(31) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')                           // 7,7: 3

	PORT_START("KEY2")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')                        // 8,0: - _
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('(') PORT_CHAR('[')                    // 8,1: ( [
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(')') PORT_CHAR(']')                   // 8,2: ) ]
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("CONT TR/NORM")                                                     // 8,3: CONT / TR/NORM
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))                      // 8,4: KP 7
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) PORT_NAME("KP /") // 8,5: KP /
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))                      // 8,6: KP 9
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))                      // 8,7: KP 8
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')                        // 9,0: ,
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')                                       // 9,1: Space
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("PAUSE STEP")                                                      // 9,2: PAUSE / STEP
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("RUN")                                                             // 9,3: RUN
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))                     // 9,4: KP 0
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD)) PORT_NAME("KP +") // 9,5: KP +
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA_PAD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD)) PORT_CHAR('<') PORT_NAME("KP ,") // 9,6: KP ,
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) PORT_CHAR('>') PORT_NAME("KP .") // 9,7: KP .
	PORT_BIT(IOP_MASK(16) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_NAME("k6 k13")       // 10,0: k6 / k13
	PORT_BIT(IOP_MASK(17) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_NAME("k7 k14")       // 10,1: k7 / k14
	PORT_BIT(IOP_MASK(18) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("-LINE CLEAR")                                                     // 10,2: -LINE / CLEAR
	PORT_BIT(IOP_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_NAME("Up Home")      // 10,3: Up / Home
	PORT_BIT(IOP_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_NAME("Down A/G") // 10,4: Down / A/G
	PORT_BIT(IOP_MASK(21) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("ROLL")                                    // 10,5: ROLL
	PORT_BIT(IOP_MASK(22) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME("Right -CHAR") // 10,6: RIGHT / -CHAR
	PORT_BIT(IOP_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME("Left I/R") // 10,7: LEFT / I/R

	PORT_START("MODKEYS")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)                // Shift
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)              // Control
INPUT_PORTS_END

static INPUT_PORTS_START(hp86_001)
	PORT_INCLUDE(hp86_int)

	PORT_START("LANGUAGE")
	PORT_DIPNAME(0x3f , 0 , "Language")
	PORT_DIPLOCATION("S2:7,6,5,4,3,2")
	PORT_DIPSETTING(0x00 , "English")
	PORT_DIPSETTING(0x01 , "Swedish/Finnish")
	PORT_DIPSETTING(0x02 , "Norwegian/Danish")
	PORT_DIPSETTING(0x06 , "Spanish")
INPUT_PORTS_END

static INPUT_PORTS_START(hp86_004)
	PORT_INCLUDE(hp86_int)

	PORT_START("LANGUAGE")
	PORT_DIPNAME(0x3f , 0 , "Language")
	PORT_DIPLOCATION("S2:7,6,5,4,3,2")
	PORT_DIPSETTING(0x00 , "English")
	PORT_DIPSETTING(0x04 , "German")
	PORT_DIPSETTING(0x08 , "French")
	PORT_DIPSETTING(0x09 , "Italian")
	PORT_DIPSETTING(0x0a , "Dutch")
	PORT_DIPSETTING(0x14 , "Swiss German")
	PORT_DIPSETTING(0x15 , "Swiss French")
INPUT_PORTS_END

ROM_START(hp86b_001)
	ROM_REGION(0x6000 , "cpu" , 0)
	ROM_LOAD("romsys1.bin" , 0x0000 , 0x2000 , CRC(bfa473b8) SHA1(cc420742a5f03c466484a5063e0abcbc084bf298))
	ROM_LOAD("romsys2.bin" , 0x2000 , 0x2000 , CRC(2bc3ba4b) SHA1(760bef9c482f562677f80b18d6163a19ee7aea1c))
	ROM_LOAD("romsys3.bin" , 0x4000 , 0x2000 , CRC(86bf3b8b) SHA1(209c91b9b972ab514c600752e2e4af68f984612e))

	ROM_REGION(0x1a4000 , "rombank" , 0)
	ROM_LOAD("rom000.bin" , 0x0000 , 0x2000 , CRC(c3ca5c54) SHA1(2b291607de101c7206bfae9520a18f1009929e9b))
	ROM_LOAD("rom001.bin" , 0x2000 , 0x2000 , CRC(59a1616c) SHA1(e0fe840f9740bdb455fe1872869671f8712b7cff))
	ROM_LOAD("rom030.bin" , 0x30000 , 0x2000 , CRC(14507bc0) SHA1(67ca5a15019bd7b2bfbf53bb8cfbe2ca20a6239c))
	ROM_LOAD("rom320.bin" , 0x1a0000 , 0x2000 , CRC(c921e2e4) SHA1(e37ac61364830cfa214e6d1b9942cc1cde6ad01f))
	ROM_LOAD("rom321.bin" , 0x1a2000 , 0x2000 , CRC(e6e5cc91) SHA1(67711de228cc48a78d04b13f0a1c91dc26f7e87c))

	ROM_REGION(0x500 , "chargen" , 0)
	ROM_LOAD("chrgen.bin" , 0 , 0x500 , CRC(a3d891c2) SHA1(df7c262b585e9394251640d0775474a76c199905))
ROM_END

ROM_START(hp86b_004)
	ROM_REGION(0x6000 , "cpu" , 0)
	ROM_LOAD("romsys1.bin" , 0x0000 , 0x2000 , CRC(bfa473b8) SHA1(cc420742a5f03c466484a5063e0abcbc084bf298))
	ROM_LOAD("romsys2.bin" , 0x2000 , 0x2000 , CRC(2bc3ba4b) SHA1(760bef9c482f562677f80b18d6163a19ee7aea1c))
	ROM_LOAD("romsys3.bin" , 0x4000 , 0x2000 , CRC(86bf3b8b) SHA1(209c91b9b972ab514c600752e2e4af68f984612e))

	ROM_REGION(0x1a4000 , "rombank" , 0)
	ROM_LOAD("rom000.bin" , 0x0000 , 0x2000 , CRC(c3ca5c54) SHA1(2b291607de101c7206bfae9520a18f1009929e9b))
	ROM_LOAD("rom001.bin" , 0x2000 , 0x2000 , CRC(59a1616c) SHA1(e0fe840f9740bdb455fe1872869671f8712b7cff))
	ROM_LOAD("rom030.bin" , 0x30000 , 0x2000 , CRC(14507bc0) SHA1(67ca5a15019bd7b2bfbf53bb8cfbe2ca20a6239c))
	ROM_LOAD("rom320.bin" , 0x1a0000 , 0x2000 , CRC(c921e2e4) SHA1(e37ac61364830cfa214e6d1b9942cc1cde6ad01f))
	ROM_LOAD("rom321.bin" , 0x1a2000 , 0x2000 , CRC(e6e5cc91) SHA1(67711de228cc48a78d04b13f0a1c91dc26f7e87c))

	ROM_REGION(0x500 , "chargen" , 0)
	ROM_LOAD("chrgen.bin" , 0 , 0x500 , CRC(c7d04292) SHA1(b86ed801ee9f7a57b259374b8a9810572cb03230))
ROM_END

} // anonymous namespace


COMP( 1980, hp85,      0,     0, hp85, hp85,     hp85_state,     empty_init, "HP", "HP 85", 0)
COMP( 1983, hp86b,     0,     0, hp86, hp86,     hp86_state,     empty_init, "HP", "HP 86B",0)
COMP( 1983, hp86b_001, hp86b, 0, hp86, hp86_001, hp86_int_state, empty_init, "HP", "HP 86B Opt 001",0)
COMP( 1983, hp86b_004, hp86b, 0, hp86, hp86_004, hp86_int_state, empty_init, "HP", "HP 86B Opt 004",0)
