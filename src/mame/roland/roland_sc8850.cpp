// license:BSD-3-Clause
// copyright-holders:superctr
/****************************************************************************

    Roland ED Sound Canvas SC-8850 and SC-8820.

    The last of the Sound Canvas line: 64 parts over two MIDI ports, USB,
    and on the SC-8850 two "XP" PCM chips splitting a 128 voice machine
    between them.

    SC-8850 main board (service notes and the control ROM):
    - IC1 HD64F7017F28 SH-2 at 28.224 MHz in on-chip ROM enabled mode.  The
      block diagram draws an SH7016 with a 64 kB mask ROM instead; the 64 kB
      dump is self contained, so the flash part is a later production change.
    - IC2 TC160G22AF gate array: LEDs, panel, reset and enable lines
    - IC3 SED1335F0B graphics LCD controller with 32 kB of VRAM of its own,
      driving a 160 x 64 panel
    - IC4 M37640M8 USB controller (undumped), reached through two byte wide
      mailboxes the firmware calls UIPC
    - IC5/IC6 RA09-002 (XP6) PCM chips, the slave at 0x00a00000 and the
      master at 0x00a80000, each with 512 kB of effect DRAM
    - IC7 MB87837 (LSP) insertion effect processor on the slave's serial out
    - IC9 1 MB program flash, IC10 2 MB tone parameter flash
    - IC53/IC54 two 16 MB wave mask ROMs
    - IC36/IC39 AK4324 DACs, 20 bit / 32 kHz, driven by the master's SDOC and
      SDOD lines as OUTPUT 1 and OUTPUT 2

    The two XPs split the machine: the master carries the reverb, the
    equalizer, the output stage and both DACs, the slave the chorus, the
    system delay and the link to the LSP, and fourteen words cross the
    SDOA/SDIA link between them every frame.

    SC-8820: the same firmware and the same sounds on one XP, with the
    program and the tone parameters in a single 2 MB flash and no graphics
    LCD.  Its own CPU is not dumped, and its USB controller is the one time
    PROM part M37640E8FP at IC2, undumped as well.

    Not emulated yet: the USB controller, which the firmware reports as a
    hardware error and then carries on without - and without it the machine
    cannot reach its second MIDI port, so half of its 64 parts are out of
    reach - the program flash's writes, which is where the settings are
    kept (see the memory map), and on the SC-8820 the panel and the
    display, which do not go through a gate array there.

****************************************************************************/

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/sh/sh7014.h"
#include "sound/roland_lsp.h"
#include "sound/roland_xp.h"
#include "video/sed1330.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include <algorithm>

#define LOG_GA      (1U << 1)
#define LOG_UIPC    (1U << 2)

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


namespace {

class roland_sc8850_state : public driver_device
{
public:
	roland_sc8850_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_master(*this, "xp1")
		, m_slave(*this, "xp2")
		, m_lsp(*this, "lsp")
		, m_lcdc(*this, "lcdc")
		, m_computer_sw(*this, "COMPUTER")
		, m_keys(*this, "KEY%u", 0U)
		, m_dial(*this, "VALUE")
		, m_leds(*this, "led%u", 0U)
	{
	}

	void sc8850(machine_config &config);
	void sc8820(machine_config &config);

	void init_sc8850() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void sc8850_map(address_map &map) ATTR_COLD;
	void sc8820_map(address_map &map) ATTR_COLD;
	void lcdc_map(address_map &map) ATTR_COLD;
	void xp_rom_map(address_map &map) ATTR_COLD;
	void sc8820_xp_rom_map(address_map &map) ATTR_COLD;

	void lcd_palette(palette_device &palette) const;

	u8 ga_r(offs_t offset);
	void ga_w(offs_t offset, u8 data);
	void ga_raise(u8 source);
	void ga_present();
	void ga_lower();
	TIMER_CALLBACK_MEMBER(ga_scan);
	TIMER_CALLBACK_MEMBER(ga_tick);
	TIMER_CALLBACK_MEMBER(ga_sequencer);
	u16 porta_r();

	template <int Channel> u8 uipc_r(offs_t offset);
	template <int Channel> void uipc_w(offs_t offset, u8 data);

	u16 computer_sw_r() { return m_computer_sw.read_safe(0); }

	void pump_slave(int state);
	u32 master_link_r(offs_t strobe);
	u32 slave_link_r(offs_t strobe);
	void lsp_serial_w(offs_t port, u32 data);
	u32 lsp_serial_r(offs_t group);

	required_device<sh7014_device> m_maincpu;
	required_device<roland_xp_device> m_master;
	optional_device<roland_xp_device> m_slave;
	required_device<roland_lsp_device> m_lsp;
	optional_device<sed1330_device> m_lcdc;
	optional_ioport m_computer_sw;
	optional_ioport_array<4> m_keys;
	optional_ioport m_dial;
	output_finder<16> m_leds;

	// the voice service tick, timed against the machine's own portamento
	static constexpr int TICK_HZ = 100;
	// the sequencer clock, one millisecond
	static constexpr int SEQUENCER_HZ = 1000;

	emu_timer *m_ga_timer = nullptr;
	emu_timer *m_ga_tick_timer = nullptr;
	emu_timer *m_ga_sequencer_timer = nullptr;
	u16 m_ga_requests = 0;
	u8 m_ga_regs[0x100]{};
	u8 m_ga_event = 0;
	u8 m_ga_source = 0;
	bool m_ga_pending = false;
	u8 m_key_state[4]{};
	u8 m_scan_position = 0;
	bool m_scan_encoder = false;
	u8 m_dial_position = 0;
	s32 m_encoder = 0;
	u32 m_uipc_step = 0;
};


//-------------------------------------------------
//  the wave ROMs are stored as dumped: the board permutes the address lines
//  within each 256 KB block and the data lines.  The permutation is the pair
//  the SC-88 family uses, but this generation wires it the other way round.
//-------------------------------------------------

void roland_sc8850_state::init_sc8850()
{
	memory_region *region = memregion("waverom");
	u8 *rom = region->base();
	const u32 size = region->bytes();

	static const u8 address_lines[18] = { 0, 4, 2, 3, 1, 8, 12, 6, 13, 11, 9, 16, 7, 5, 14, 17, 10, 15 };
	static const u8 data_lines[8] = { 2, 0, 4, 5, 7, 6, 3, 1 };

	std::vector<u8> scrambled(rom, rom + size);
	for (u32 i = 0; i < size; i++)
	{
		u32 address = i & ~0x3ffff;
		for (int bit = 0; bit < 18; bit++)
			if (BIT(i, bit))
				address |= 1 << address_lines[bit];

		const u8 source = scrambled[address];
		u8 data = 0;
		for (int bit = 0; bit < 8; bit++)
			if (BIT(source, data_lines[bit]))
				data |= 1 << bit;
		rom[i] = data;
	}
}

void roland_sc8850_state::machine_start()
{
	m_ga_timer = timer_alloc(FUNC(roland_sc8850_state::ga_scan), this);
	m_ga_tick_timer = timer_alloc(FUNC(roland_sc8850_state::ga_tick), this);
	m_ga_sequencer_timer = timer_alloc(FUNC(roland_sc8850_state::ga_sequencer), this);

	save_item(NAME(m_ga_regs));
	save_item(NAME(m_ga_event));
	save_item(NAME(m_ga_requests));
	save_item(NAME(m_ga_source));
	save_item(NAME(m_ga_pending));
	save_item(NAME(m_key_state));
	save_item(NAME(m_scan_position));
	save_item(NAME(m_scan_encoder));
	save_item(NAME(m_dial_position));
	save_item(NAME(m_encoder));
	save_item(NAME(m_uipc_step));
}

void roland_sc8850_state::machine_reset()
{
	std::fill(std::begin(m_ga_regs), std::end(m_ga_regs), 0);
	std::fill(std::begin(m_key_state), std::end(m_key_state), 0);
	m_ga_event = 0;
	m_ga_source = 0;
	m_ga_pending = false;
	m_ga_requests = 0;
	m_scan_position = 0;
	m_scan_encoder = false;
	m_dial_position = 0;
	m_encoder = 0;
	m_uipc_step = 0;

	if (m_keys[0].found())
	{
		m_ga_timer->adjust(attotime::from_usec(250), 0, attotime::from_usec(250));
		m_ga_tick_timer->adjust(attotime::from_hz(TICK_HZ), 0, attotime::from_hz(TICK_HZ));
		m_ga_sequencer_timer->adjust(attotime::from_hz(SEQUENCER_HZ), 0, attotime::from_hz(SEQUENCER_HZ));
	}
}


//-------------------------------------------------
//  the TC160G22AF gate array
//
//  Sixteen sources share IRQ2 and the line the CPU sees on PA8.  0x40 names
//  the one that is waiting; the handler the firmware picks out of a table in
//  DRAM reads that source's own data register, which is what takes the event
//  and drops the line.  Two of the sixteen are the front panel.
//
//  Source 0 is the switches, a four by eight matrix read at 0x43: the row in
//  bits 6-3, the column in bits 2-0, and bit 7 set when the switch has come
//  up.  The two column seven positions are not in the matrix and read the
//  other way round, which the firmware undoes for exactly those two codes.
//
//  Source 1 is the value encoder, read at 0x42 as the movement since the
//  last read, a signed byte.
//
//  Source 10 is the voice service tick, and nothing else in the machine wakes
//  the task that owns the chip's ramps: its handler counts the tick and posts
//  that task's only event, so without it a note starts and never moves again.
//  A source with no data register of its own is taken by the read of 0x40.
//  It steps a portamento by one pitch step, which is what times it: a glide
//  takes the same number of steps whatever the rate, and three of them against
//  the machine make it a hundredth of a second.
//
//  Source 11 is the sequencer clock, and it is the machine's millisecond: the
//  phrase player registers a handler on it only while it has something to
//  play, and steps the phrase by the microseconds its tempo gives each step.
//  Without it the preview and the demo start and then stand still.
//
//  0x3e and 0x3f carry the sixteen interrupt enables, one bit a source, and
//  the firmware keeps them shadowed at 0x01052cd9 and 0x01052cda; a source
//  whose bit is clear waits.  0x38 and 0x39 are the sixteen LED drives, active
//  low and rewritten whenever the panel is redrawn; five are fitted, SOLO and
//  MUTE in the first byte and EDIT, DRUM and EFFECT in the second.  0x3a
//  carries two reset or enable bits.  The rest the boot ROM writes once and
//  neither it nor the firmware reads.
//-------------------------------------------------

void roland_sc8850_state::ga_raise(u8 source)
{
	m_ga_requests |= 1 << source;
	ga_present();
}

void roland_sc8850_state::ga_present()
{
	const u16 enabled = m_ga_requests & (m_ga_regs[0x3e] | (m_ga_regs[0x3f] << 8));
	if (m_ga_pending || !enabled)
		return;

	for (m_ga_source = 0; !BIT(enabled, m_ga_source); m_ga_source++)
		;

	m_ga_pending = true;
	m_maincpu->set_input_line(2, ASSERT_LINE);
}

void roland_sc8850_state::ga_lower()
{
	m_ga_requests &= ~(1 << m_ga_source);
	m_ga_pending = false;
	m_maincpu->set_input_line(2, CLEAR_LINE);
	ga_present();
}

TIMER_CALLBACK_MEMBER(roland_sc8850_state::ga_tick)
{
	ga_raise(10);
}

TIMER_CALLBACK_MEMBER(roland_sc8850_state::ga_sequencer)
{
	ga_raise(11);
}

TIMER_CALLBACK_MEMBER(roland_sc8850_state::ga_scan)
{
	if (BIT(m_ga_requests, 0) || BIT(m_ga_requests, 1))
		return;

	const u8 dial = m_dial.read_safe(0);
	m_encoder += s8(dial - m_dial_position);
	m_dial_position = dial;

	m_scan_encoder = !m_scan_encoder;
	if (m_scan_encoder && m_encoder != 0)
	{
		ga_raise(1);
		return;
	}

	for (int i = 0; i < 32; i++)
	{
		m_scan_position = (m_scan_position + 1) & 0x1f;
		const int row = m_scan_position >> 3, column = m_scan_position & 7;
		const u8 mask = 1 << column;
		if (BIT(m_keys[row].read_safe(0), column) == bool(m_key_state[row] & mask))
			continue;

		m_key_state[row] ^= mask;
		const bool up = bool(m_key_state[row] & mask) == (column == 7);
		m_ga_event = (up ? 0x80 : 0x00) | (row << 3) | column;
		ga_raise(0);
		return;
	}
}

u8 roland_sc8850_state::ga_r(offs_t offset)
{
	u8 data = m_ga_regs[offset];
	switch (offset)
	{
	case 0x40:
		data = m_ga_source;
		if (!machine().side_effects_disabled() && m_ga_source > 1)
			ga_lower();
		break;

	case 0x42:
		data = u8(std::clamp<s32>(m_encoder, -128, 127));
		if (!machine().side_effects_disabled())
		{
			m_encoder -= s8(data);
			ga_lower();
		}
		break;

	case 0x43:
		data = m_ga_event;
		if (!machine().side_effects_disabled())
			ga_lower();
		break;

	default:
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_GA, "%s: gate array read %02x = %02x\n", machine().describe_context(), offset, data);
		break;
	}
	return data;
}

void roland_sc8850_state::ga_w(offs_t offset, u8 data)
{
	m_ga_regs[offset] = data;
	switch (offset)
	{
	case 0x38:
	case 0x39:
		for (int i = 0; i < 8; i++)
			m_leds[(offset & 1) * 8 + i] = !BIT(data, i);
		break;

	case 0x3e:
	case 0x3f:
		ga_present();
		break;
	default:
		LOGMASKED(LOG_GA, "%s: gate array write %02x = %02x\n", machine().describe_context(), offset, data);
		break;
	}
}

// the gate array pulls PA8 down while it has an event waiting.  The boot
// ROM's own panel pass takes the pin off IRQ2, polls it and puts it back.
u16 roland_sc8850_state::porta_r()
{
	return m_ga_pending ? 0x0000 : 0x0100;
}


//-------------------------------------------------
//  the two host channels of the M37640 USB controller, "UIPC" to the
//  firmware: +0 data, +1 status, with bit 0 of the status saying a byte has
//  arrived on channel 1 and bit 1 that channel 0 will take one.  A byte
//  whose status has any of bits 4-7 set is a tagged one and the firmware's
//  plain reader discards it.
//
//  The controller runs a 32 KB program that the SC-8850's own flash carries
//  at 0x00250674 and pushes across this mailbox at power-on: it waits for
//  fa, answers a tagged byte, then fb, sends the two bytes of a constant,
//  waits for fc, and then either fe (send the program) or fd followed by ff
//  (the controller is already running).  Nothing here emulates the
//  controller, so this walks the short branch of that handshake and then
//  goes quiet, which is what gets the machine past its power-on wait.
//-------------------------------------------------

template <int Channel>
u8 roland_sc8850_state::uipc_r(offs_t offset)
{
	if (Channel == 0)
		return offset ? 0x02 : 0x00;

	static const u8 boot[][2] = {
		{ 0x01, 0xfa }, { 0xf1, 0x00 }, { 0x01, 0xfb }, { 0x01, 0xfc }, { 0x01, 0xfd }, { 0x01, 0xff }
	};

	if (m_uipc_step >= std::size(boot))
		return 0x00;

	if (offset == 0)
	{
		const u8 data = boot[m_uipc_step][1];
		if (!machine().side_effects_disabled())
		{
			LOGMASKED(LOG_UIPC, "%s: uipc handshake byte %02x\n", machine().describe_context(), data);
			m_uipc_step++;
		}
		return data;
	}

	return boot[m_uipc_step][0];
}

template <int Channel>
void roland_sc8850_state::uipc_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_UIPC, "%s: uipc%d write %d = %02x\n", machine().describe_context(), Channel, offset, data);
}




//-------------------------------------------------
//  the link between the two XPs: six SDOA and six SDIA lines in both
//  directions, carrying fourteen words a frame.  Both programs strobe port A
//  on the same eight slots, so each chip reads what the other has at that
//  strobe; and because every word the master sends is written before its
//  strobe and every word the slave sends after it, running the master's frame
//  and then the slave's gives each side exactly what the wire would.
//-------------------------------------------------

void roland_sc8850_state::pump_slave(int state)
{
	m_slave->run_frame();
}

u32 roland_sc8850_state::master_link_r(offs_t strobe)
{
	return m_master->port_a_out_r(strobe);
}

u32 roland_sc8850_state::slave_link_r(offs_t strobe)
{
	return m_slave->port_a_out_r(strobe);
}


//-------------------------------------------------
//  the slave clocks the insertion effect's send out on SDOC and takes its
//  return on port B, which the program reads with `col 0x1c`.  The chip
//  refreshes that node at each group's first strobe, after the strobe's own
//  instruction, so the program's read on the strobe sees the other half's
//  word: the half-frame rotation is the chip's, on both machines.
//-------------------------------------------------

void roland_sc8850_state::lsp_serial_w(offs_t port, u32 data)
{
	if ((port >> 1) != roland_xp_device::PORT_C)
		return;
	m_lsp->ser_w(port & 1, s32(data));
	if (port & 1)
		m_lsp->run_once();
}

u32 roland_sc8850_state::lsp_serial_r(offs_t group)
{
	return u32(-(m_lsp->ser_r(group & 1) >> 8));
}


//-------------------------------------------------
//  memory maps
//
//  The SH7010 series decodes A23/A22 into CS0-CS3.  The boot ROM sets CS1
//  eight bits wide and everything else sixteen; CS1's five devices are
//  decoded again on a 256 KB grid.
//
//  The two flashes on CS0 and CS3 are mapped as plain ROM, and that is
//  where the machine's settings would go: it has no battery SRAM, and every
//  user setting lives in block 0xe of the program flash, 0x2e0000-0x2effff.
//  A save - the Utility page's ENTER, a user instrument's write - runs the
//  boot ROM's flash driver over it: read the chip's ID at 0x200000, clear
//  the block's lock bits, erase it, program 14340 words (the system
//  parameters first, then the user instruments and drum sets) and set
//  block 0's lock bit again.  That driver knows two Sharp parts, 00b0/0050
//  for the program flash, sixteen 64 KB blocks with its write enable on
//  port B bit 9, and 00b0/00d0 for the tone flash, thirty-two blocks on
//  bit 8, which no user action ever writes.  Against plain ROM the ID read
//  comes back as data, so the firmware takes its unknown-chip path, skips
//  the erase and aims its lock commands at address 4, and the settings are
//  the dump's again at the next power-on.  Emulating the chip - word
//  programming, block erase, status and ID in place of data while it is
//  out of read-array mode - is what an NVRAM would take; it is not done.
//-------------------------------------------------

void roland_sc8850_state::sc8850_map(address_map &map)
{
	map(0x00000000, 0x0000ffff).rom().region("cpurom", 0);
	map(0x00200000, 0x002fffff).rom().region("progrom", 0);
	map(0x00500000, 0x00500000).rw(m_lcdc, FUNC(sed1330_device::data_r), FUNC(sed1330_device::data_w));
	map(0x00500001, 0x00500001).rw(m_lcdc, FUNC(sed1330_device::status_r), FUNC(sed1330_device::command_w));
	map(0x00540000, 0x00540001).rw(FUNC(roland_sc8850_state::uipc_r<0>), FUNC(roland_sc8850_state::uipc_w<0>));
	map(0x00580000, 0x00580001).rw(FUNC(roland_sc8850_state::uipc_r<1>), FUNC(roland_sc8850_state::uipc_w<1>));
	map(0x005c0000, 0x005c000f).rw(m_lsp, FUNC(roland_lsp_device::host_r), FUNC(roland_lsp_device::host_w));
	map(0x006c0000, 0x006c00ff).rw(FUNC(roland_sc8850_state::ga_r), FUNC(roland_sc8850_state::ga_w));
	map(0x00a00000, 0x00a03fff).rw(m_slave, FUNC(roland_xp_device::read), FUNC(roland_xp_device::write));
	map(0x00a80000, 0x00a83fff).rw(m_master, FUNC(roland_xp_device::read), FUNC(roland_xp_device::write));
	map(0x00d00000, 0x00efffff).rom().region("toneprm", 0);
	map(0x01000000, 0x0107ffff).mirror(0x00f80000).ram();
}

// the SC-8820 keeps the two mailboxes where they are, moves the LSP down to
// where the SC-8850 puts its LCD controller, has no gate array, and carries
// its program and its tone parameters in one flash on CS3
void roland_sc8850_state::sc8820_map(address_map &map)
{
	map(0x00000000, 0x0000ffff).rom().region("cpurom", 0);
	map(0x00500000, 0x0050000f).rw(m_lsp, FUNC(roland_lsp_device::host_r), FUNC(roland_lsp_device::host_w));
	map(0x00540000, 0x00540001).rw(FUNC(roland_sc8850_state::uipc_r<0>), FUNC(roland_sc8850_state::uipc_w<0>));
	map(0x00580000, 0x00580001).rw(FUNC(roland_sc8850_state::uipc_r<1>), FUNC(roland_sc8850_state::uipc_w<1>));
	map(0x00900000, 0x00903fff).rw(m_master, FUNC(roland_xp_device::read), FUNC(roland_xp_device::write));
	map(0x00d00000, 0x00efffff).rom().region("progrom", 0);
	map(0x01000000, 0x0107ffff).mirror(0x00f80000).ram();
}

void roland_sc8850_state::lcdc_map(address_map &map)
{
	map(0x0000, 0x7fff).ram(); // TC55257 32 KB
}

// two 16 MB mask ROMs, on wave chip selects 5 and 6
// the XP asks for a wave byte as a chip select in bits 27-24 and a 1 MB bank
// in bits 23-20, and it is the board that decides what each of the eight
// selects reaches.  This one gives each of them four banks, in order, so the
// pair of 16 MB mask ROMs is a flat 32 MB: the machine's own wave table uses
// all eight selects and never a bank above three, and every one of its
// descriptors below 20 MB is then the same wave, at the same offset, as the
// SC-8820's.
void roland_sc8850_state::xp_rom_map(address_map &map)
{
	for (int select = 0; select < 8; select++)
		map(select << 24, (select << 24) | 0x3fffff).rom().region("waverom", select << 22);
}

// the SC-8820 has 16 MB and 8 MB; it cannot boot far enough to say which
// selects it uses, so it follows its sibling's
// the SC-8820 numbers its banks straight through instead, 0 to 23 with no
// chip select at all, so its two ROMs are one window.
void roland_sc8850_state::sc8820_xp_rom_map(address_map &map)
{
	map(0x0000000, 0x17fffff).rom().region("waverom", 0x0000000);
}


//-------------------------------------------------
//  the 160 x 64 graphic panel
//-------------------------------------------------

void roland_sc8850_state::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0xf8, 0xc8, 0x40)); // backlight
	palette.set_pen_color(1, rgb_t(0x20, 0x10, 0x00)); // dot on
}


//-------------------------------------------------
//  machine configuration
//-------------------------------------------------

// the rear panel COMPUTER switch, a four position resistor ladder on AN0: the boot ROM sorts the ten bit
// result, left aligned in ADDRA, into four bands at 0x3ff0, 0x7fe0 and 0xbfd0, and the firmware routes
// MIDI by the band.  Only the MIDI position gives the two MIDI IN ports a handler table that reaches the
// part assigner; the others hand the machine to a USB or serial host that nothing here answers.
static INPUT_PORTS_START(sc8850)
	// the switch matrix as the firmware reads it: the row and column of each
	// code taken from the table at 0x00250410, its name from the one the
	// switch and LED test prints at 0x00286410
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Map")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F4")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F2")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F1")
	PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Value")

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Edit")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Drum")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Effect")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Shift")
	PORT_BIT(0x70, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Preview")

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Part <")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Down")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Exit")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Solo")
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Part >")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Up")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Enter")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Mute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Dec / -")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Inc / +")
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("VALUE")
	PORT_BIT(0xff, 0x00, IPT_DIAL) PORT_NAME("Value Dial") PORT_SENSITIVITY(25) PORT_KEYDELTA(2) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT)

	PORT_START("COMPUTER")
	PORT_CONFNAME(0x3ff, 0x000, "Computer Switch")
	PORT_CONFSETTING(0x000, "MIDI")
	PORT_CONFSETTING(0x180, "PC-1")
	PORT_CONFSETTING(0x280, "PC-2")
	PORT_CONFSETTING(0x3ff, "USB")
INPUT_PORTS_END

// the SC-8820's panel does not go through a gate array and is not identified,
// so it has no switch matrix for the scanner to walk
static INPUT_PORTS_START(sc8820)
	PORT_START("COMPUTER")
	PORT_CONFNAME(0x3ff, 0x000, "Computer Switch")
	PORT_CONFSETTING(0x000, "MIDI")
	PORT_CONFSETTING(0x180, "PC-1")
	PORT_CONFSETTING(0x280, "PC-2")
	PORT_CONFSETTING(0x3ff, "USB")
INPUT_PORTS_END

void roland_sc8850_state::sc8850(machine_config &config)
{
	SH7014(config, m_maincpu, 28.224_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_sc8850_state::sc8850_map);
	m_maincpu->sci_tx_w<0>().set("mdout", FUNC(midi_port_device::write_txd));
	m_maincpu->read_adc<0>().set(FUNC(roland_sc8850_state::computer_sw_r));
	m_maincpu->read_porta().set(FUNC(roland_sc8850_state::porta_r));

	// MIDI IN A and B are the two on-chip serial ports, MIDI OUT/THRU is SCI0's transmitter
	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set(m_maincpu, FUNC(sh7014_device::sci_rx_w<0>));
	midi_port_device &mdin2(MIDI_PORT(config, "mdin2", midiin_slot, "midiin"));
	mdin2.rxd_handler().set(m_maincpu, FUNC(sh7014_device::sci_rx_w<1>));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_lcd();
	screen.set_refresh_hz(60);
	screen.set_screen_update("lcdc", FUNC(sed1330_device::screen_update));
	screen.set_size(160, 64);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(roland_sc8850_state::lcd_palette), 2);

	SED1330(config, m_lcdc, 10'000'000); // SED1335F0B
	m_lcdc->set_screen("screen");
	m_lcdc->set_addrmap(0, &roland_sc8850_state::lcdc_map);

	SPEAKER(config, "output1", 2).front();
	SPEAKER(config, "output2", 2).front();

	// the master's SDOC and SDOD lines, one AK4324 each, the frame half
	// picking left or right
	ROLAND_XP(config, m_master, 24.576_MHz_XTAL);
	m_master->set_addrmap(roland_xp_device::AS_WAVE, &roland_sc8850_state::xp_rom_map);
	// the runtime vector table sends IRQ1 to the master's service routine and IRQ0 to the slave's
	m_master->int_callback().set_inputline(m_maincpu, 1);
	m_master->add_route(2, "output1", 1.0, 0);
	m_master->add_route(3, "output1", 1.0, 1);
	m_master->add_route(4, "output2", 1.0, 0);
	m_master->add_route(5, "output2", 1.0, 1);

	m_master->port_a_in_callback().set(FUNC(roland_sc8850_state::slave_link_r));
	m_master->frame_callback().set(FUNC(roland_sc8850_state::pump_slave));

	// the slave reaches the DACs only through the master, over SDOA, and the master's frame clocks it
	ROLAND_XP(config, m_slave, 24.576_MHz_XTAL);
	m_slave->set_addrmap(roland_xp_device::AS_WAVE, &roland_sc8850_state::xp_rom_map);
	m_slave->int_callback().set_inputline(m_maincpu, 0);
	m_slave->set_pumped(true);
	m_slave->port_a_in_callback().set(FUNC(roland_sc8850_state::master_link_r));

	ROLAND_LSP(config, m_lsp, 24.576_MHz_XTAL);

	// the EFX send leaves the slave on SDOC; the return comes back on port B
	m_slave->port_out_callback().set(FUNC(roland_sc8850_state::lsp_serial_w));
	m_slave->port_b_in_callback().set(FUNC(roland_sc8850_state::lsp_serial_r));
}

void roland_sc8850_state::sc8820(machine_config &config)
{
	SH7014(config, m_maincpu, 28.224_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_sc8850_state::sc8820_map);

	SPEAKER(config, "output1", 2).front();
	SPEAKER(config, "output2", 2).front();

	ROLAND_XP(config, m_master, 24.576_MHz_XTAL);
	m_master->set_addrmap(roland_xp_device::AS_WAVE, &roland_sc8850_state::sc8820_xp_rom_map);
	m_master->int_callback().set_inputline(m_maincpu, 1);
	// its program puts the DAC pair on the third strobe of each group, SDOD under the rotation the
	// other machines follow, where the schematic runs the DAC from SDOC
	m_master->add_route(4, "output1", 1.0, 0);
	m_master->add_route(5, "output1", 1.0, 1);

	ROLAND_LSP(config, m_lsp, 24.576_MHz_XTAL);
}


ROM_START(sc8850)
	ROM_REGION32_BE(0x10000, "cpurom", 0)
	// dated 08/25/1999 in its own string table; carries the flash updater
	ROM_LOAD("roland-r01783490.ic1", 0x00000, 0x10000, CRC(4b2f36e3) SHA1(99b414c5129960e3e58af1fb147a1474ccccca84))

	// M37640M8-104FP, the mask ROM member of the 7640 group; the firmware
	// downloads its application into the SRAM beside it, so this is the
	// USB stack and the loader that takes it
	ROM_REGION(0x8000, "usbmcu", 0)
	ROM_LOAD("roland-r02010212.ic4", 0x0000, 0x8000, NO_DUMP)

	ROM_REGION32_BE(0x100000, "progrom", 0)
	// "Roland XP-GS Ver.1.01" / "Roland SC-GS Version 1.00", built 11/28/2000
	ROM_LOAD("roland-r01678145.ic9", 0x00000, 0x100000, CRC(3ef69f93) SHA1(e594de1f5be17c11ef4f4d7efd142053bbf44085))

	ROM_REGION32_BE(0x200000, "toneprm", 0)
	ROM_LOAD("roland-r01561945.ic10", 0x000000, 0x200000, CRC(390faa62) SHA1(d9af1c75b277de2258ed74982d7a754f60c0826e))

	ROM_REGION(0x2000000, "waverom", ROMREGION_ERASE00)
	ROM_LOAD("roland-r01891445-823.ic53", 0x0000000, 0x1000000, CRC(2cfe5aa2) SHA1(68655acdea37bfa73a30db64807902f525bf2215))
	ROM_LOAD("roland-r01891456-824.ic54", 0x1000000, 0x1000000, CRC(623015b6) SHA1(b98166a1222ad32f82911fd72e7d0eb76380f80d))
ROM_END

ROM_START(sc8820)
	ROM_REGION32_BE(0x10000, "cpurom", 0)
	// the SC-8820's own CPU (Roland R02015367) is undumped; the SC-8850's
	// on-chip ROM stands in for it and jumps to the SC-8850's flash base
	ROM_LOAD("roland-r01783490.ic1", 0x00000, 0x10000, BAD_DUMP CRC(4b2f36e3) SHA1(99b414c5129960e3e58af1fb147a1474ccccca84))

	// M37640E8FP here, the one time PROM member of the group
	ROM_REGION(0x8000, "usbmcu", 0)
	ROM_LOAD("roland-r02010623.ic2", 0x0000, 0x8000, NO_DUMP)

	ROM_REGION32_BE(0x200000, "progrom", 0)
	// program and tone parameters in one flash, built 11/28/2000
	ROM_LOAD("roland-r01561945.ic5", 0x000000, 0x200000, CRC(352ad418) SHA1(4b23624d6317eb43dc63bf07d04bcce5c415e202))

	ROM_REGION(0x1800000, "waverom", ROMREGION_ERASEFF)
	ROM_LOAD("roland-r01891445-823.ic7", 0x0000000, 0x1000000, CRC(2cfe5aa2) SHA1(68655acdea37bfa73a30db64807902f525bf2215))
	// reconstructed from Sound Canvas VA's embedded image, which is this
	// machine's wave set descrambled; the chip is also stocked as IC39,
	// Roland R02016156, a Macronix MX23C6410
	ROM_LOAD("roland-r02121512-541.ic8", 0x1000000, 0x800000, BAD_DUMP CRC(38908222) SHA1(c89c88bba5290b6184142da27d7b6949a0ad94b8))
ROM_END

} // anonymous namespace


SYST(1999, sc8850, 0,      0, sc8850, sc8850, roland_sc8850_state, init_sc8850, "Roland", "Sound Canvas SC-8850", MACHINE_NOT_WORKING)
SYST(1999, sc8820, sc8850, 0, sc8820, sc8820, roland_sc8850_state, init_sc8850, "Roland", "Sound Canvas SC-8820", MACHINE_NOT_WORKING)
