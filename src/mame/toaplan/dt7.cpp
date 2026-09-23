// license:BSD-3-Clause
// copyright-holders:David Haywood, Andrea Bogazzi

/*  DT7 / Dynamic Trial 7 - Toaplan 1993 (prototype)

	Notes:

	As this is a prototype game a number of features are incomplete or broken,
	some of these have been verified against the code.  There are also some
	debug features left in.

-------------------------------------------------------------------------------------

	- The coin support is broken on the prototype

	The four coinage rate lookups at 0x2b09e / 0x2b0a8 / 0x2b0c6 / 0x2b0d0
	encode displacements to the rate tables at 0x2b13c / 0x2b15c

	These displacements no longer fit in the signed 8-bit (d8,PC,Xn) field
	so instead invalid coinage values are fetched from surrounding code bytes.

    Batsugun ships the identical routine with its tables still in reach
	Every dt7 lookup misses its table by exactly 0x100.

	The following ROM patch can be used to fix the coinage behavior.

	u16 *rom = reinterpret_cast<u16 *>(memregion("maincpu")->base());
	for (int i = 0; i < 16; i++)
		rom[0x2b11c / 2 + i] += 0x100; // 0x2b13c / 0x2b15c rate tables
	rom[0x2abdc / 2] += 0x100; // cmpi.w #$10 -> #$110 (Europe entry)

    For the USA regions the 'Discount Coin' setting bypasses the coinage
	table entirely and hardcodes the game to 2 coins / 1 credit


	- Game hangs at title if "Handle, 2 Pedals & Buttons" mode is selected

	Does this prototype support the alt control method? there's code for
	reading these devices in the ROM.

	- Game at hangs car selection if "Goal De Shi-Nu" mode is selected

	This might require linked cabinet support, it is unknown if the link
	feature is complete in the prototype code.

	- If Invincibility mode is turned on the game also shows debug markers
	  on the track, and has a very crude level selection (intentional
	  behavior)

	- The Palette Bar screen tiles sometimes get corrupted when entering
	  service mode (TODO: check if this is a problem with the CPU comms)

	- The arrow for the Service menu is shown on screen 2, but the text
	  is not (TODO: check if this is just a problem in the prototype code)

-------------------------------------------------------------------------------------

	Hidden Region Selection:

	Much like Fixeight, The region (title, notice screen and license text) come
	from byte 2 of the EEPROM and can be rewritten on real hardware with a hidden
	operator combo.

	in the CONFIGURATION page of service mode

	hold 2P START plus the 1P buttons encoding the region value
	SHOT1 = +1
	SHOT2 = +2
	SHOT3 = +4
	1P START = +8

	then toggle the test switch to save.
	Values:
	0/1 Korea (Car Fighting title)
	2/3 Hong Kong
	4/5 Taiwan
	6/7 Southeast Asia
	8/9 Europe
	a/b USA
	c/d invalid
	e/f Japan

	Even values give a Taito license string, except for Japan where
	the odd value does

-------------------------------------------------------------------------------------

   TODO:
    - serial comms (needs support in V25 core?) for linked units (if supported)
    - verify frequencies on chips
    - identify the hardware meant to feed the 0x58008 / 0x5800a latches
    - merge tilemap emulation into toaplan/toaplan_txtilemap.cpp?
*/


#include "emu.h"

#include "toaplipt.h"
#include "toaplan_v25_tables.h"
#include "gp9001.h"

#include "cpu/m68000/m68000.h"
#include "cpu/nec/v25.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class dt7_state : public driver_device
{
public:
	dt7_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_audiocpu(*this, "audiocpu")
		, m_vdp(*this, "gp9001_%u", 0U)
		, m_oki(*this, "oki%u", 1U)
		, m_eeprom(*this, "eeprom")
		, m_gfxdecode(*this, "gfxdecode%u", 0U)
		, m_screen(*this, "screen%u", 0U)
		, m_palette(*this, "palette%u", 0U)
		, m_shared_ram(*this, "shared_ram")
		, m_tx_videoram(*this, "tx_videoram")
		, m_lineram(*this, "lineram")
		, m_eepromport(*this, "EEPROM")
		, m_sysport(*this, "SYS")
		, m_sys2port(*this, "SYS2")
		, m_p1port(*this, "IN1")
		, m_p2port(*this, "IN2")
		, m_miscport(*this, "MISC%u", 0U)
		, m_anport(*this, "AN%u", 0U)
	{ }

public:
	void dt7(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void dt7_reset(int state);

	u32 screen_update_dt7_1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_dt7_2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void tx_videoram_dt7_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_tx_dt7_tile_info);

	void draw_tx_tilemap(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, int table);

	void dt7_68k_0_mem(address_map &map);
	void dt7_68k_1_mem(address_map &map);
	void dt7_v25_mem(address_map &map);
	void dt7_shared_mem(address_map &map);

	void dt7_irq(int state);
	void dt7_sndreset_coin_w(offs_t offset, u16 data, u16 mem_mask);

	u8 unmapped_v25_io1_r();
	u8 unmapped_v25_io2_r();

	u8 read_port_t();
	u8 read_port_2();
	void write_port_2(u8 data);
	void write_port_0(u8 data);

	u8 eeprom_r();
	void eeprom_w(u8 data);

	u8 dt7_shared_ram_r(offs_t offset);
	void shared_ram_w(offs_t offset, u8 data);

	void screen_vblank(int state);

	u8 m_ioport_state;
	u8 m_dac_value;
	u32 m_shift_chain[2];

	tilemap_t *m_tx_tilemap[2];    /* Tilemap for extra-text-layer */

	required_device<m68000_base_device> m_maincpu;
	required_device<m68000_base_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device_array<gp9001vdp_device, 2> m_vdp;
	required_device_array<okim6295_device, 2> m_oki;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device_array<gfxdecode_device, 2> m_gfxdecode;
	required_device_array<screen_device, 2> m_screen;
	required_device_array<palette_device, 2> m_palette;
	required_shared_ptr<u8> m_shared_ram; // 8 bit RAM shared between 68K and sound CPU
	required_shared_ptr<u16> m_tx_videoram;
	required_shared_ptr<u16> m_lineram;
	required_ioport m_eepromport;
	required_ioport m_sysport;
	required_ioport m_sys2port;
	required_ioport m_p1port;
	required_ioport m_p2port;
	required_ioport_array<2> m_miscport;
	required_ioport_array<4> m_anport;
};


void dt7_state::dt7_irq(int state)
{
	// only the first VDP gets IRQ acked, so does it trigger both CPUs, or does the main CPU then trigger sub?
	m_maincpu->set_input_line(4, state ? ASSERT_LINE : CLEAR_LINE);
	m_subcpu->set_input_line(4, state ? ASSERT_LINE : CLEAR_LINE);

	// INTP0 publishes the scanned inputs to the 68k through the ring buffer
	m_audiocpu->set_input_line(NEC_INPUT_LINE_INTP0, state ? ASSERT_LINE : CLEAR_LINE);
}


// SAR ADC comparators: bit n set when analog channel n >= the port 0 DAC value
u8 dt7_state::read_port_t()
{
	u8 ret = 0;
	for (int i = 0; i < 4; i++)
		if (m_anport[i]->read() >= m_dac_value)
			ret |= 1 << i;
	return ret;
}

void dt7_state::write_port_0(u8 data)
{
	m_dac_value = data;
}

u8 dt7_state::eeprom_r()
{
	return m_eepromport->read();
}

void dt7_state::eeprom_w(u8 data)
{
	m_eepromport->write(data);
}

// digital inputs: two serial chains on P2, bit 2 = parallel load, bit 3 = shift
// clock, bits 0/1 = data (LSB first); last byte of chain 1 must idle at 0xff
u8 dt7_state::read_port_2()
{
	return (m_ioport_state & 0xfc) | (m_shift_chain[0] & 1) | ((m_shift_chain[1] & 1) << 1);
}

void dt7_state::write_port_2(u8 data)
{
	// rising edge on bit 2: parallel load
	if (!(m_ioport_state & 0x04) && (data & 0x04))
	{
		const u8 p1 = m_p1port->read();
		const u8 p2 = m_p2port->read();

		// chain 0: 1P controls, system byte, 2P start echo, gate (idle high)
		m_shift_chain[0] = ~u32(p1 | (m_sysport->read() << 8) | ((p2 & 0x80) << 16));

		// chain 1: 2P side, mirroring chain 0 (controls, system byte, 1P start echo, gate)
		m_shift_chain[1] = ~u32(p2 | (m_sys2port->read() << 8) | ((p1 & 0x80) << 16));
	}

	// rising edge on bit 3: shift both chains one bit
	if (!(m_ioport_state & 0x08) && (data & 0x08))
	{
		m_shift_chain[0] = (m_shift_chain[0] >> 1) | 0x80000000;
		m_shift_chain[1] = (m_shift_chain[1] >> 1) | 0x80000000;
	}

	m_ioport_state = data;
}

u8 dt7_state::dt7_shared_ram_r(offs_t offset)
{
	return m_shared_ram[offset];
}

void dt7_state::shared_ram_w(offs_t offset, u8 data)
{
	m_shared_ram[offset] = data;
}

void dt7_state::dt7_sndreset_coin_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x8000) ? CLEAR_LINE : ASSERT_LINE);
	logerror("%s: dt7_sndreset_coin_w %04x %04x\n", machine().describe_context(), data, mem_mask);
	// TODO: coin counters in lower byte?
}

void dt7_state::dt7_68k_0_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();

	map(0x380000, 0x380001).r(m_vdp[0], FUNC(gp9001vdp_device::vdpcount_r));

	map(0x200000, 0x200fff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette0");
	map(0x280000, 0x280fff).ram().w(m_palette[1], FUNC(palette_device::write16)).share("palette1");

	map(0x300000, 0x300001).w(FUNC(dt7_state::dt7_sndreset_coin_w));

	map(0x400000, 0x40000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x480000, 0x48000d).rw(m_vdp[1], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));

	dt7_shared_mem(map);

	map(0x610000, 0x61ffff).rw(FUNC(dt7_state::dt7_shared_ram_r), FUNC(dt7_state::shared_ram_w)).umask16(0x00ff);
}

void dt7_state::dt7_shared_mem(address_map &map)
{
	map(0x500000, 0x50ffff).ram().share("shared_ram2");
	// is this really in the middle of shared RAM, or is there a DMA to get it out?
	map(0x509000, 0x50afff).ram().w(FUNC(dt7_state::tx_videoram_dt7_w)).share("tx_videoram");
	map(0x50f000, 0x50ffff).ram().share("lineram");
}

void dt7_state::dt7_68k_1_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom().mirror(0x080000); // mirror needed or road doesn't draw
	dt7_shared_mem(map);
}


u8 dt7_state::unmapped_v25_io1_r()
{
	return m_miscport[0]->read();
}

u8 dt7_state::unmapped_v25_io2_r()
{
	return m_miscport[1]->read();
}

void dt7_state::machine_start()
{
	save_item(NAME(m_ioport_state));
	save_item(NAME(m_dac_value));
	save_item(NAME(m_shift_chain));
}

void dt7_state::machine_reset()
{
	m_ioport_state = 0x00;
	m_dac_value = 0x00;
	m_shift_chain[0] = m_shift_chain[1] = 0xffffffff;
}

void dt7_state::dt7_v25_mem(address_map &map)
{
	// the mirroring is unclear, these are the mirrors needed by the game
	map(0x00000, 0x07fff).ram().share("shared_ram");
	map(0x08000, 0x0ffff).ram().share("shared_ram");
	map(0x40000, 0x47fff).ram().share("shared_ram");
	map(0x60000, 0x67fff).ram().share("shared_ram");

	map(0x58000, 0x58001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x58002, 0x58002).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x58004, 0x58005).rw("ymsnd2", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x58006, 0x58006).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x58008, 0x58008).r(FUNC(dt7_state::unmapped_v25_io1_r));
	map(0x5800a, 0x5800a).r(FUNC(dt7_state::unmapped_v25_io2_r));

	map(0x70000, 0x77fff).ram().share("shared_ram");
	map(0xf8000, 0xfffff).ram().share("shared_ram");
}

void dt7_state::dt7_reset(int state)
{
	//if (m_audiocpu != nullptr)
	//  m_audiocpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);

	// needed when exiting service menu
	m_subcpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

static GFXDECODE_START( gfx_textrom_double )
	GFXDECODE_ENTRY( "text_0", 0, gfx_8x8x4_packed_msb, 0, 128 )
GFXDECODE_END

static GFXDECODE_START( gfx_textrom_double_1 )
	GFXDECODE_ENTRY( "text_1", 0, gfx_8x8x4_packed_msb, 0, 128 )
GFXDECODE_END

void dt7_state::dt7(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &dt7_state::dt7_68k_0_mem);
	m_maincpu->reset_cb().set(FUNC(dt7_state::dt7_reset));

	M68000(config, m_subcpu, 32_MHz_XTAL/2);
	m_subcpu->set_addrmap(AS_PROGRAM, &dt7_state::dt7_68k_1_mem);

	v25_device &audiocpu(V25(config, m_audiocpu, 32_MHz_XTAL/2));
	audiocpu.set_addrmap(AS_PROGRAM, &dt7_state::dt7_v25_mem);
	audiocpu.set_decryption_table(toaplan_v25_tables::dt7_decryption_table);
	audiocpu.pt_in_cb().set(FUNC(dt7_state::read_port_t));
	audiocpu.p0_out_cb().set(FUNC(dt7_state::write_port_0));
	audiocpu.p2_in_cb().set(FUNC(dt7_state::read_port_2));
	audiocpu.p2_out_cb().set(FUNC(dt7_state::write_port_2));
	audiocpu.p1_in_cb().set(FUNC(dt7_state::eeprom_r));
	audiocpu.p1_out_cb().set(FUNC(dt7_state::eeprom_w));

	EEPROM_93C66_16BIT(config, m_eeprom);

	config.set_maximum_quantum(attotime::from_hz(6000));

	/* video hardware */
	SCREEN(config, m_screen[0]);
	m_screen[0]->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen[0]->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen[0]->set_screen_update(FUNC(dt7_state::screen_update_dt7_1));
	m_screen[0]->screen_vblank().set(FUNC(dt7_state::screen_vblank));
	m_screen[0]->set_palette(m_palette[0]);

	SCREEN(config, m_screen[1]);
	m_screen[1]->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen[1]->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen[1]->set_screen_update(FUNC(dt7_state::screen_update_dt7_2));
	//m_screen[1]->screen_vblank().set(FUNC(dt7_state::screen_vblank));
	m_screen[1]->set_palette(m_palette[1]);

	PALETTE(config, m_palette[0]).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	PALETTE(config, m_palette[1]).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette[0]);
	m_vdp[0]->vint_out_cb().set(FUNC(dt7_state::dt7_irq));
	m_vdp[0]->set_screen(m_screen[0]);

	GP9001_VDP(config, m_vdp[1], 27_MHz_XTAL);
	m_vdp[1]->set_palette(m_palette[1]);
	m_vdp[1]->set_screen(m_screen[1]);

	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_textrom_double);

	GFXDECODE(config, m_gfxdecode[1], m_palette[1], gfx_textrom_double_1);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "speaker", 0.5, 0);

	OKIM6295(config, m_oki[0], 27_MHz_XTAL / 24, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "speaker", 0.5, 0);

	YM2151(config, "ymsnd2", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "speaker", 0.5, 1);

	OKIM6295(config, m_oki[1], 27_MHz_XTAL/24, okim6295_device::PIN7_HIGH);
	m_oki[1]->add_route(ALL_OUTPUTS, "speaker", 0.5, 1);
}


static INPUT_PORTS_START( dt7 )
	// bit layouts follow the serial shift chains (chain 0 byte 0, byte 1 and
	// chain 1 byte 0)
	PORT_START("IN1")
	TOAPLAN_GENERIC_JOY_MONO_UDLR( 1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("IN2")
	TOAPLAN_GENERIC_JOY_MONO_UDLR( 2 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Test Switch")

	PORT_START("SYS2") // second seat's coin unit
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_COIN4 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_SERVICE2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	// per-seat auxiliary input latches; the sound CPU forwards them into the
	// ring buffer half that is exchanged over the cabinet link, so they are
	// local inputs (possibly handle-mode steering), not the linked players -
	// those arrive over the serial link and read as idle without one
	PORT_START("MISC0") // latched at 0x58008
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("MISC1") // latched at 0x5800a
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	// The following should be the analog inputs, but the game hangs at the title screen if analog
	// controls are enabled in the service mode, so they might not be functional in the supported prototype
	PORT_START("AN0") // digitized against the port 0 DAC, calibrated via EEPROM words 0xfc/0xfe
	//PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("AN1")
	//PORT_BIT( 0xff, 0x80, IPT_PEDAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("AN2")
	//PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("AN3")
	//PORT_BIT( 0xff, 0x80, IPT_PEDAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("EEPROM")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
INPUT_PORTS_END

// as the coin code is broken for most regions, just hook it up to the service coin
static INPUT_PORTS_START( dt7_altcoin )
	PORT_INCLUDE( dt7 )

	PORT_MODIFY("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // should be COIN1, but doesn't work
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // should be COIN2, but doesn't work
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_MODIFY("SYS2") // second seat's coin unit
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // should be COIN3, but doesn't work
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // should be COIN4, but doesn't work
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
INPUT_PORTS_END

TILE_GET_INFO_MEMBER(dt7_state::get_tx_dt7_tile_info)
{
	const u16 attrib = m_tx_videoram[tile_index];
	const u32 tile_number = attrib & 0x3ff;

	u32 color = (attrib & 0xf800) >> 11;

	color |= 0x60;

	tileinfo.set(0,
			tile_number,
			color,
			0);
}

void dt7_state::video_start()
{
	// a different part of this tilemap is displayed on each screen
	// each screen has a different palette and uses a ROM in a different location on the PCB
	m_tx_tilemap[0] = &machine().tilemap().create(*m_gfxdecode[0], tilemap_get_info_delegate(*this, FUNC(dt7_state::get_tx_dt7_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tx_tilemap[1] = &machine().tilemap().create(*m_gfxdecode[1], tilemap_get_info_delegate(*this, FUNC(dt7_state::get_tx_dt7_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tx_tilemap[0]->set_transparent_pen(0);
	m_tx_tilemap[1]->set_transparent_pen(0);
}

void dt7_state::tx_videoram_dt7_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tx_videoram[offset]);
	m_tx_tilemap[0]->mark_tile_dirty(offset);
	m_tx_tilemap[1]->mark_tile_dirty(offset);
}


void dt7_state::draw_tx_tilemap(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, int table)
{
	// there seems to be RAM for another tx tilemap
	// but there were 2 empty sockets / sockets with blank tx ROMs, so
	// it's likely only one of them is used

	// per-line entries: word 0 = line select (bit 15 clear = flipped), word 1 =
	// xscroll; the game's init writes 0x8900+y / 0x1e0+2*table when the screen is
	// normal and 0x9ef-y / 0x15f-2*table when it is flipped (screens flip
	// independently, service menu items 2 and 3), so the y mirroring comes from
	// the line select values themselves and only x needs the flipped origin
	int flipx = m_lineram[(table * 2)] & 0x8000;
	m_tx_tilemap[table / 2]->set_flip(flipx ? 0 : TILEMAP_FLIPX);

	rectangle clip = cliprect;
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		clip.min_y = clip.max_y = y;

		u16 scroll1 = m_lineram[((y * 8) + (table * 2) + 0) & 0x7ff]; // lineselect
		u16 scroll2 = m_lineram[((y * 8) + (table * 2) + 1) & 0x7ff]; // xscroll

		scroll1 &= 0x7fff; // 0x8000 is per-line flip
		scroll1 -= 0x0900;

		int xscroll;
		if (flipx)
			xscroll = scroll2 - (0x1e0 + table * 2);
		else
			xscroll = -(scroll2 - (0x15f - table * 2));

		m_tx_tilemap[table / 2]->set_scrolly(0, scroll1 - y);
		m_tx_tilemap[table / 2]->set_scrollx(0, xscroll);
		m_tx_tilemap[table / 2]->draw(screen, bitmap, clip, 0);
	}
}

u32 dt7_state::screen_update_dt7_1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	screen.priority().fill(0, cliprect);
	m_vdp[0]->render_vdp(bitmap, cliprect, screen.priority());
	draw_tx_tilemap(screen, bitmap, cliprect, 0);
	return 0;
}


u32 dt7_state::screen_update_dt7_2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	screen.priority().fill(0, cliprect);
	m_vdp[1]->render_vdp(bitmap, cliprect, screen.priority());
	draw_tx_tilemap(screen, bitmap, cliprect, 2);
	return 0;
}

void dt7_state::screen_vblank(int state)
{
	m_vdp[0]->screen_eof();
	m_vdp[1]->screen_eof();
}



// All sets share the single dumped ROM set; the region (and with it the title,
// notice screen and license text) comes from EEPROM settings byte 2, so each set
// below only provides a different EEPROM default.
#define ROMS_DT7 \
	ROM_REGION( 0x080000, "maincpu", 0 ) \
	ROM_LOAD16_WORD_SWAP( "main.11", 0x000000, 0x080000, CRC(01646c22) SHA1(4b87f00dc99e1206b3b9eaee425fc05e1a033bee) ) \
	ROM_REGION( 0x080000, "subcpu", 0 ) \
	ROM_LOAD16_WORD_SWAP( "2.21", 0x000000, 0x080000, CRC(a08e25ed) SHA1(db10c64ce305477442b35e7624052aae9fb6e412) ) \
	ROM_REGION( 0x400000, "gp9001_0", 0 ) \
	ROM_LOAD( "3a.49", 0x000000, 0x080000, CRC(ba8e378c) SHA1(d5eb4a839d6b3c2b9bf0bd87f06859a01a2c0cbf) ) \
	ROM_LOAD( "3b.50", 0x080000, 0x080000, CRC(a9e4c6c7) SHA1(4058b1b887f41494a70b0b09e581ef5e3a444a1c) ) \
	ROM_LOAD( "3c.51", 0x100000, 0x080000, CRC(ffc6fa95) SHA1(87d18520fae7eec9336fc8cfb1adc2923ea10f8d) ) \
	ROM_LOAD( "3d.52", 0x180000, 0x080000, CRC(3faaa3e7) SHA1(ec3e6e8d16a8095c857ff270d2bd48c04664b62f) ) \
	ROM_LOAD( "4a.30", 0x200000, 0x080000, CRC(53627ea6) SHA1(02f9cc223427a2b78e60bc866fd6c73df07b438d) ) \
	ROM_LOAD( "4b.31", 0x280000, 0x080000, CRC(a7e20eb4) SHA1(73da86764a93350224ada21b3178dde0a34cc657) ) \
	ROM_LOAD( "4c.32", 0x300000, 0x080000, CRC(ad0fc76a) SHA1(112e934a2cab13f994d1873aaaec40d38d2c2deb) ) \
	ROM_LOAD( "4d.33", 0x380000, 0x080000, CRC(280f97af) SHA1(9fe74c67440d7c952f091fb77905b7515852e0fb) ) \
	ROM_REGION( 0x08000, "text_0", 0 ) \
	ROM_LOAD( "7text.115", 0x000000, 0x08000,  CRC(7fb47a44) SHA1(1b5401967f33dc232187bf9f2a402b71286c5fc2) ) \
	ROM_REGION( 0x400000, "gp9001_1", 0 ) \
	ROM_LOAD( "3a.68", 0x000000, 0x080000, CRC(ba8e378c) SHA1(d5eb4a839d6b3c2b9bf0bd87f06859a01a2c0cbf) ) \
	ROM_LOAD( "3b.69", 0x080000, 0x080000, CRC(a9e4c6c7) SHA1(4058b1b887f41494a70b0b09e581ef5e3a444a1c) ) \
	ROM_LOAD( "3c.70", 0x100000, 0x080000, CRC(ffc6fa95) SHA1(87d18520fae7eec9336fc8cfb1adc2923ea10f8d) ) \
	ROM_LOAD( "3d.71", 0x180000, 0x080000, CRC(3faaa3e7) SHA1(ec3e6e8d16a8095c857ff270d2bd48c04664b62f) ) \
	ROM_LOAD( "4a.87", 0x200000, 0x080000, CRC(53627ea6) SHA1(02f9cc223427a2b78e60bc866fd6c73df07b438d) ) \
	ROM_LOAD( "4b.88", 0x280000, 0x080000, CRC(a7e20eb4) SHA1(73da86764a93350224ada21b3178dde0a34cc657) ) \
	ROM_LOAD( "4c.89", 0x300000, 0x080000, CRC(ad0fc76a) SHA1(112e934a2cab13f994d1873aaaec40d38d2c2deb) ) \
	ROM_LOAD( "4d.90", 0x380000, 0x080000, CRC(280f97af) SHA1(9fe74c67440d7c952f091fb77905b7515852e0fb) ) \
	ROM_REGION( 0x08000, "text_1", 0 ) \
	ROM_LOAD( "7text.152", 0x000000, 0x08000,  CRC(7fb47a44) SHA1(1b5401967f33dc232187bf9f2a402b71286c5fc2) ) \
	ROM_REGION( 0x40000, "oki1", 0 ) \
	ROM_LOAD( "7adpcm.37", 0x00000, 0x40000, CRC(aefce555) SHA1(0d47190287957122fefdae17ccf6bcfaef8cd430) ) \
	ROM_REGION( 0x40000, "oki2", 0 ) \
	ROM_LOAD( "7adpcm.43", 0x00000, 0x40000, CRC(aefce555) SHA1(0d47190287957122fefdae17ccf6bcfaef8cd430) ) \

ROM_START( dt7 )
	ROMS_DT7

	ROM_REGION16_BE( 0x200, "eeprom", ROMREGION_ERASE00 )
	// defaulted to 'Discount Continue' so that the regular coin slots work
	ROM_LOAD16_WORD_SWAP( "eeprom_usa", 0x000, 0x200, CRC(4c546ede) SHA1(7892b54dafe5782d4900063377b956e3bf7bca07) )
ROM_END

ROM_START( dt7j )
	ROMS_DT7

	ROM_REGION16_BE( 0x200, "eeprom", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "eeprom_japan", 0x000, 0x200, CRC(e8af8958) SHA1(4284ba45323bbd3679b8e4e7b96fa8cf37b939c0) )
ROM_END

ROM_START( dt7k )
	ROMS_DT7

	ROM_REGION16_BE( 0x200, "eeprom", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "eeprom_korea", 0x000, 0x200, CRC(910b4058) SHA1(ca52306ed5a6b12b5c8fc92f9cc36e7639e5570a) )
ROM_END

} // anonymous namespace

// The flyer shows "Survival Battle Dynamic Trial 7"; the Korean sets title as "Car Fighting"

// While the EEPROM can provide more regions than this, these ones have been selected for
// the following resaons
// The USA region can have working coin slots and English langauge
// The Japanese region has Japanese language
// The Korean region has a unique title screen
GAME( 1993, dt7,    0,   dt7, dt7,         dt7_state, empty_init, ROT270, "Toaplan", "DT7 (USA) (prototype)",            MACHINE_NODEVICE_LAN | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, dt7j,   dt7, dt7, dt7_altcoin, dt7_state, empty_init, ROT270, "Toaplan", "DT7 (Japan) (prototype)",          MACHINE_NODEVICE_LAN | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, dt7k,   dt7, dt7, dt7_altcoin, dt7_state, empty_init, ROT270, "Toaplan", "Car Fighting (Korea) (prototype)", MACHINE_NODEVICE_LAN | MACHINE_IMPERFECT_GRAPHICS )
