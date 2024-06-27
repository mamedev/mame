// license:BSD-3-Clause
// copyright-holders:David Haywood

/* This is derived from toaplan2.cpp, but there are enough hardware
   differences to keep it separate

   TODO:
    - verify audio CPU opcodes
    - correctly hook up interrupts (especially V25)
    - correctly hook up inputs (there's a steering wheel test? is the game switchable)
    - serial comms (needs support in V25 core?) for linked units
    - verify frequencies on chips
    - verify alt titles, some regions have 'Car Fighting' as a subtitle, region comes from EEPROM?
    - verify text layer palettes
	- currently only coins up with service button
	- course selection cursor doesn't move?
	- sound dies after one stage?
	- game crashes after two stages?
*/


#include "emu.h"
#include "toaplipt.h"
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

class toaplan2_dt7_state : public driver_device
{
public:
	toaplan2_dt7_state(const machine_config &mconfig, device_type type, const char *tag)
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
	{ }

public:
	void dt7(machine_config &config);

protected:

private:
	void toaplan2_dt7_reset(int state);

	u32 screen_update_dt7_1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_dt7_2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_VIDEO_START(dt7);

	void tx_videoram_dt7_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_text_dt7_tile_info);

	void dt7_68k_0_mem(address_map &map);
	void dt7_68k_1_mem(address_map &map);
	void dt7_v25_mem(address_map &map);
	void dt7_shared_mem(address_map &map);

	void dt7_irq(int state);
	void dt7_unk_w(u8 data);

	uint8_t unmapped_v25_io1_r();
	uint8_t unmapped_v25_io2_r();

	uint8_t m_ioport_state = 0x00;

	uint8_t read_port_t();
	uint8_t read_port_2();
	void write_port_2(uint8_t data);

	u16 video_count_r();

	u8 dt7_shared_ram_hack_r(offs_t offset);
	void shared_ram_w(offs_t offset, u8 data);

	// We encode priority with colour in the tilemaps, so need a larger palette
	static constexpr unsigned T2PALETTE_LENGTH = 0x10000;

	void screen_vblank(int state);
	DECLARE_VIDEO_START(toaplan2);

	tilemap_t *m_tx_tilemap[2];    /* Tilemap for extra-text-layer */

	bitmap_ind8 m_custom_priority_bitmap;

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
};


// single byte default-opcode
#define UNKO 0xfc

// complete guess, wrong, just to get correct opcode size for DASM

#define G_62 0xa0  // very likely wrong (or is it?)

// some kind of branch, not sure which
// it's used after compares in blocks, sometimes with a 'be' then a 'br' straight after, so it must be a condition that could also fail a be and fall to the br
//#define G_B0  0x74
#define G_B0  0x79
//#define G_B0  0x75

//  6b  @ 73827
#define G_6B  0x34  // must be a 2 byte operation on al? after an AND, 2nd byte is 0x08

//  59  @ 73505 and 7379B  (maybe ret?, or di?)
//#define G_59  0xfa
#define G_59  0xc3

static const u8 dt7_decryption_table[256] = {
//  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, 0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f, /* 00 */
	UNKO,0xea,0x8a,0x51,0x8b,UNKO,0x48,0x3c, 0x75,0x50,0x75,0x88,0x03,0x03,UNKO,0x36,

//  0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17, 0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f, /* 10 */
	0x8a,0x0f,0x8a,0x3c,0xe2,0xe8,0xc6,0xc7, 0x24,0x4d,0x68,0x3e,0x0c,0x33,0xbb,UNKO,

//  0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27, 0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f, /* 20 */
	0xbb,0xc6,0x1f,0x36,0x24,0xeb,0xe8,UNKO, 0x02,0x38,0x0f,0x45,0x8d,0x45,0x36,0xc6,

//  0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37, 0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f, /* 30 */
	0x53,0x8b,0x81,0x22,0xf9,0xbe,0x75,0x55, 0x45,0x51,0x5d,0x3e,0x0f,0x88,0x72,0x74,

//  0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47, 0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f, /* 40 */
	0x1e,0xb7,0x50,0xd0,0xe2,0xb1,0x0a,0xf3, 0xc7,0xff,0x8a,0x75,0x88,0xb5,UNKO,0xb3,

//  0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57, 0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f, /* 50 */
	0xc3,0x80,0x53,0x59,0x88,UNKO,0x87,0x45, 0x03,G_59,0x0c,0x36,0x5f,0x16,0x55,UNKO,

//  0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67, 0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f, /* 60 */
	0x0a,UNKO,G_62,0x89,0x88,0x57,0x2e,0xb1, 0x75,0x43,0x3a,G_6B,0x86,0x3a,0x03,0x58,

//  0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77, 0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f, /* 70 */
	0x46,0x33,0xe8,0x0f,0x0f,0xbb,0x59,0xc7, 0x2e,0xc6,0x53,0x3a,0xc0,0xfe,0x02,0x47,

//  0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87, 0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f, /* 80 */
	0xa0,0x2c,0xeb,0x24,UNKO,0xc3,0x8a,0x8e, 0x16,0x74,0x8a,0x33,0x4b,0x05,0x89,0x79,

//  0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97, 0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f, /* 90 */
	0xb4,0xd2,0x0f,0xbd,0xfb,0x3e,0x22,0x2a, 0x47,0xfe,0x8a,0xc3,0x03,0x5e,0xb3,0x07,

//  0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7, 0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf, /* a0 */
	0x86,0x1b,0x81,0xf3,0x86,0xe9,0x53,0x74, 0x80,0xab,0xb1,0xc3,0xd0,0x88,0x2e,0xa4,

//  0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7, 0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf, /* b0 */
	G_B0,0x5b,0x87,UNKO,0xc3,0x8c,0xff,0x8a, 0x50,0xeb,0x56,0x0c,UNKO,0xfc,0x83,0x74,

//  0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7, 0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf, /* c0 */
	0x26,UNKO,0xfe,0xbd,0x03,0xfe,0xb4,0xfe, 0x06,0xb8,0xc6,UNKO,0x45,0x73,0xb5,0x51,

//  0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7, 0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf, /* d0 */
	UNKO,0xa4,0xf9,0xc0,0x5b,0xab,0xf6,UNKO, 0x32,0xd3,0xeb,0xb9,0x73,0x89,0xbd,0x4d,

//  0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7, 0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef, /* e0 */
	0xb8,0xb9,0x74,0x07,0x0a,0xb0,0x4f,0x0f, 0xe8,0x47,0xeb,0x50,0xd1,0xd0,0x5d,0x72,

//  0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7, 0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff, /* f0 */
	0x2e,0xe2,0xc1,0xe8,0xa2,0x53,0x0f,0x73, 0x3a,0xbf,0xbb,0x46,0x1a,0x3c,0x1a,0xbc,
};

void toaplan2_dt7_state::dt7_irq(int state)
{
	// only the first VDP gets IRQ acked, so does it trigger both CPUs, or does the main CPU then trigger sub?
	m_maincpu->set_input_line(4, state ? ASSERT_LINE : CLEAR_LINE);
	m_subcpu->set_input_line(4, state ? ASSERT_LINE : CLEAR_LINE);

	// this reads the inputs (again what is the source?)
	// the audio CPU also has a 'serial' interrupt populated?
	// and the boards can be linked together

	// triggering this prevents our other timer? interrupt from working however?
	//m_audiocpu->set_input_line(NEC_INPUT_LINE_INTP0, state ? ASSERT_LINE : CLEAR_LINE);
}


// this is conditional on the unknown type of branch (see #define G_B0 in the table0
uint8_t toaplan2_dt7_state::read_port_t()
{
	logerror("%s: read port t\n", machine().describe_context()); return machine().rand();
}

uint8_t toaplan2_dt7_state::read_port_2()
{
	logerror("%s: read port 2\n", machine().describe_context());

	return 0xff;
}

// it seems to attempt to read inputs (including the tilt switch?) here on startup
// strangely all the EEPROM access code (which is otherwise very similar to FixEight
// also has accesses to this port added, maybe something is sitting in the middle?
void toaplan2_dt7_state::write_port_2(uint8_t data)
{
	if ((m_ioport_state & 0x01) != (data & 0x01))
	{
		if (data & 0x01)
			logerror("%s: bit 0x01 low to high\n", machine().describe_context());
		else
			logerror("%s: bit 0x01 high to low\n", machine().describe_context());
	}

	if ((m_ioport_state & 0x02) != (data & 0x02))
	{
		if (data & 0x02)
			logerror("%s: bit 0x02 low to high\n", machine().describe_context());
		else
			logerror("%s: bit 0x02 high to low\n", machine().describe_context());
	}

	if ((m_ioport_state & 0x04) != (data & 0x04))
	{
		if (data & 0x04)
			logerror("%s: bit 0x04 low to high\n", machine().describe_context());
		else
			logerror("%s: bit 0x04 high to low\n", machine().describe_context());
	}

	if ((m_ioport_state & 0x08) != (data & 0x08))
	{
		if (data & 0x08)
			logerror("%s: bit 0x08 low to high\n", machine().describe_context());
		else
			logerror("%s: bit 0x08 high to low\n", machine().describe_context());
	}

	if ((m_ioport_state & 0x10) != (data & 0x10))
	{
		if (data & 0x10)
			logerror("%s: bit 0x10 low to high\n", machine().describe_context());
		else
			logerror("%s: bit 0x10 high to low\n", machine().describe_context());
	}

	if ((m_ioport_state & 0x20) != (data & 0x20))
	{
		if (data & 0x20)
			logerror("%s: bit 0x20 low to high\n", machine().describe_context());
		else
			logerror("%s: bit 0x20 high to low\n", machine().describe_context());
	}

	if ((m_ioport_state & 0x40) != (data & 0x40))
	{
		if (data & 0x40)
			logerror("%s: bit 0x40 low to high\n", machine().describe_context());
		else
			logerror("%s: bit 0x40 high to low\n", machine().describe_context());
	}

	if ((m_ioport_state & 0x80) != (data & 0x80))
	{
		if (data & 0x80)
			logerror("%s: bit 0x80 low to high\n", machine().describe_context());
		else
			logerror("%s: bit 0x80 high to low\n", machine().describe_context());
	}

	m_ioport_state = data;
}


// hacks because the sound CPU isn't running properly
u8 toaplan2_dt7_state::dt7_shared_ram_hack_r(offs_t offset)
{
    u16 ret = m_shared_ram[offset];

    u32 addr = (offset * 2) + 0x610000;

    if (addr == 0x061f00c)
      return ioport("SYS")->read();// machine().rand();

    //return ret;


    u32 pc = m_maincpu->pc();
    if (pc == 0x7d84)
        return 0xff;
    if (addr == 0x061d000) // settings (from EEPROM?) including flipscreen
        return 0x00;
    if (addr == 0x061d002) // settings (from EEPROM?) dipswitch?
        return 0x00;
    if (addr == 0x061d004) // settings (from EEPROM?) region
        return 0xff;
    if (addr == 0x061f004)
        return ioport("IN1")->read(); ;// machine().rand(); // p1 inputs
    if (addr == 0x061f006)
        return ioport("IN2")->read();// machine().rand(); // P2 inputs
//  logerror("%08x: dt7_shared_ram_hack_r address %08x ret %02x\n", pc, addr, ret);
    return ret;
}

void toaplan2_dt7_state::shared_ram_w(offs_t offset, u8 data)
{
	m_shared_ram[offset] = data;
}


void toaplan2_dt7_state::dt7_unk_w(u8 data)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
}


void toaplan2_dt7_state::dt7_68k_0_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();

	map(0x380000, 0x380001).r(FUNC(toaplan2_dt7_state::video_count_r));

	map(0x200000, 0x200fff).ram().w(m_palette[0], FUNC(palette_device::write16)).share("palette0");
	map(0x280000, 0x280fff).ram().w(m_palette[1], FUNC(palette_device::write16)).share("palette1");

	map(0x300000, 0x300000).w(FUNC(toaplan2_dt7_state::dt7_unk_w));

	map(0x400000, 0x40000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x480000, 0x48000d).rw(m_vdp[1], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));

	dt7_shared_mem(map);

	map(0x610000, 0x61ffff).rw(FUNC(toaplan2_dt7_state::dt7_shared_ram_hack_r), FUNC(toaplan2_dt7_state::shared_ram_w)).umask16(0x00ff);
//  map(0x620000, 0x62ffff).rw(FUNC(toaplan2_dt7_state::dt7_shared_ram_hack_r), FUNC(toaplan2_dt7_state::shared_ram_w)).umask16(0x00ff);
}

void toaplan2_dt7_state::dt7_shared_mem(address_map &map)
{
	map(0x500000, 0x50ffff).ram().share("shared_ram2");
	// is this really in the middle of shared RAM, or is there a DMA to get it out?
	map(0x509000, 0x50afff).ram().w(FUNC(toaplan2_dt7_state::tx_videoram_dt7_w)).share("tx_videoram");

}
void toaplan2_dt7_state::dt7_68k_1_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom().mirror(0x80000); // mirror needed or road doesn't draw
	dt7_shared_mem(map);
}


uint8_t toaplan2_dt7_state::unmapped_v25_io1_r()
{
	logerror("%s: 0x58008 unknown read\n", machine().describe_context());
	return machine().rand();
}

uint8_t toaplan2_dt7_state::unmapped_v25_io2_r()
{
	logerror("%s: 0x5800a unknown read\n", machine().describe_context());
	return machine().rand();
}


void toaplan2_dt7_state::dt7_v25_mem(address_map &map)
{
	// exact mirroring unknown, don't cover up where the inputs/sound maps
	map(0x00000, 0x07fff).ram().share("shared_ram");
	map(0x20000, 0x27fff).ram().share("shared_ram");
	map(0x28000, 0x2ffff).ram().share("shared_ram");
	map(0x60000, 0x67fff).ram().share("shared_ram");
	map(0x68000, 0x6ffff).ram().share("shared_ram");
	map(0x70000, 0x77fff).ram().share("shared_ram");
	map(0xf8000, 0xfffff).ram().share("shared_ram");

	map(0x58000, 0x58001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x58002, 0x58002).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x58004, 0x58005).rw("ymsnd2", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x58006, 0x58006).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x58008, 0x58008).r(FUNC(toaplan2_dt7_state::unmapped_v25_io1_r));
	map(0x5800a, 0x5800a).r(FUNC(toaplan2_dt7_state::unmapped_v25_io2_r));
}

void toaplan2_dt7_state::toaplan2_dt7_reset(int state)
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

void toaplan2_dt7_state::dt7(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &toaplan2_dt7_state::dt7_68k_0_mem);
	m_maincpu->reset_cb().set(FUNC(toaplan2_dt7_state::toaplan2_dt7_reset));

	M68000(config, m_subcpu, 32_MHz_XTAL/2);
	m_subcpu->set_addrmap(AS_PROGRAM, &toaplan2_dt7_state::dt7_68k_1_mem);

	v25_device &audiocpu(V25(config, m_audiocpu, 32_MHz_XTAL/2));
	audiocpu.set_addrmap(AS_PROGRAM, &toaplan2_dt7_state::dt7_v25_mem);
	audiocpu.set_decryption_table(dt7_decryption_table);
	audiocpu.pt_in_cb().set(FUNC(toaplan2_dt7_state::read_port_t));
	audiocpu.p2_in_cb().set(FUNC(toaplan2_dt7_state::read_port_2));
	audiocpu.p2_out_cb().set(FUNC(toaplan2_dt7_state::write_port_2));
	//audiocpu.p1_in_cb().set_ioport("EEPROM");
	//audiocpu.p1_out_cb().set_ioport("EEPROM");

	// eeprom type confirmed, and gets inited after first boot, but then game won't boot again?
	EEPROM_93C66_16BIT(config, m_eeprom);

	config.set_maximum_quantum(attotime::from_hz(6000));

	/* video hardware */
	SCREEN(config, m_screen[0], SCREEN_TYPE_RASTER);
	m_screen[0]->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen[0]->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen[0]->set_screen_update(FUNC(toaplan2_dt7_state::screen_update_dt7_1));
	m_screen[0]->screen_vblank().set(FUNC(toaplan2_dt7_state::screen_vblank));
	m_screen[0]->set_palette(m_palette[0]);

	SCREEN(config, m_screen[1], SCREEN_TYPE_RASTER);
	m_screen[1]->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen[1]->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen[1]->set_screen_update(FUNC(toaplan2_dt7_state::screen_update_dt7_2));
	//m_screen[1]->screen_vblank().set(FUNC(toaplan2_dt7_state::screen_vblank));
	m_screen[1]->set_palette(m_palette[1]);

	PALETTE(config, m_palette[0]).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	PALETTE(config, m_palette[1]).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette[0]);
	m_vdp[0]->vint_out_cb().set(FUNC(toaplan2_dt7_state::dt7_irq));
	m_vdp[0]->set_screen(m_screen[0]);

	GP9001_VDP(config, m_vdp[1], 27_MHz_XTAL);
	m_vdp[1]->set_palette(m_palette[1]);
	m_vdp[1]->set_screen(m_screen[1]);

	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_textrom_double);

	GFXDECODE(config, m_gfxdecode[1], m_palette[1], gfx_textrom_double_1);

	MCFG_VIDEO_START_OVERRIDE(toaplan2_dt7_state,dt7)

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "lspeaker", 0.5);

	OKIM6295(config, m_oki[0], 27_MHz_XTAL / 24, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "lspeaker", 0.5);

	YM2151(config, "ymsnd2", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "rspeaker", 0.5);

	OKIM6295(config, m_oki[1], 27_MHz_XTAL/24, okim6295_device::PIN7_HIGH);
	m_oki[1]->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
}


static INPUT_PORTS_START( dt7 )
	PORT_START("IN1")
	TOAPLAN_JOY_UDLR_3_BUTTONS( 1 )

	PORT_START("IN2")
	TOAPLAN_JOY_UDLR_3_BUTTONS( 2 )

	PORT_START("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_TILT )
	TOAPLAN_TEST_SWITCH( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("EEPROM")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

INPUT_PORTS_END



// This is taken from toaplan2.cpp, it might not be the same here, needs verifying!
u16 toaplan2_dt7_state::video_count_r()
{
	int vpos = m_screen[0]->vpos();

	u16 video_status = 0xff00;    // Set signals inactive

	vpos = (vpos + 15) % 262;

	if (!m_vdp[0]->hsync_r())
		video_status &= ~0x8000;
	if (!m_vdp[0]->vsync_r())
		video_status &= ~0x4000;
	if (!m_vdp[0]->fblank_r())
		video_status &= ~0x0100;
	if (vpos < 256)
		video_status |= (vpos & 0xff);
	else
		video_status |= 0xff;

	return video_status;
}


TILE_GET_INFO_MEMBER(toaplan2_dt7_state::get_text_dt7_tile_info)
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

VIDEO_START_MEMBER(toaplan2_dt7_state,dt7)
{
	/* our current VDP implementation needs this bitmap to work with */
	m_screen[0]->register_screen_bitmap(m_custom_priority_bitmap);
	m_vdp[0]->custom_priority_bitmap = &m_custom_priority_bitmap;
	m_vdp[1]->custom_priority_bitmap = &m_custom_priority_bitmap;

	// a different part of this tilemap is displayed on each screen
	// each screen has a different palette and uses a ROM in a different location on the PCB
	m_tx_tilemap[0] = &machine().tilemap().create(*m_gfxdecode[0], tilemap_get_info_delegate(*this, FUNC(toaplan2_dt7_state::get_text_dt7_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tx_tilemap[1] = &machine().tilemap().create(*m_gfxdecode[1], tilemap_get_info_delegate(*this, FUNC(toaplan2_dt7_state::get_text_dt7_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tx_tilemap[0]->set_transparent_pen(0);
	m_tx_tilemap[1]->set_transparent_pen(0);
}

void toaplan2_dt7_state::tx_videoram_dt7_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tx_videoram[offset]);
	if (offset < 64 * 64)
	{
		m_tx_tilemap[0]->mark_tile_dirty(offset);
		m_tx_tilemap[1]->mark_tile_dirty(offset);
	}
}

u32 toaplan2_dt7_state::screen_update_dt7_1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_custom_priority_bitmap.fill(0, cliprect);
	m_vdp[0]->render_vdp(bitmap, cliprect);

	m_tx_tilemap[0]->set_scrolldy(0, 0);
	m_tx_tilemap[0]->draw(screen, bitmap, cliprect, 0);

	return 0;
}

u32 toaplan2_dt7_state::screen_update_dt7_2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_custom_priority_bitmap.fill(0, cliprect);
	m_vdp[1]->render_vdp(bitmap, cliprect);

	m_tx_tilemap[1]->set_scrolldy(256 + 16, 256 + 16);
	m_tx_tilemap[1]->draw(screen, bitmap, cliprect, 0);

	return 0;
}

void toaplan2_dt7_state::screen_vblank(int state)
{
	m_vdp[0]->screen_eof();
	m_vdp[1]->screen_eof();
}



ROM_START( dt7 )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "main.11", 0x000000, 0x080000, CRC(01646c22) SHA1(4b87f00dc99e1206b3b9eaee425fc05e1a033bee) )

	ROM_REGION( 0x080000, "subcpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "2.21", 0x000000, 0x080000, CRC(a08e25ed) SHA1(db10c64ce305477442b35e7624052aae9fb6e412) )

	/* Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted program uploaded by main CPU) */
	/* Note, same markings as other games found in toaplan2.cpp, but table is different! */

	ROM_REGION( 0x400000, "gp9001_0", 0 )
	ROM_LOAD( "3a.49", 0x000000, 0x080000, CRC(ba8e378c) SHA1(d5eb4a839d6b3c2b9bf0bd87f06859a01a2c0cbf) )
	ROM_LOAD( "3b.50", 0x080000, 0x080000, CRC(a9e4c6c7) SHA1(4058b1b887f41494a70b0b09e581ef5e3a444a1c) )
	ROM_LOAD( "3c.51", 0x100000, 0x080000, CRC(ffc6fa95) SHA1(87d18520fae7eec9336fc8cfb1adc2923ea10f8d) )
	ROM_LOAD( "3d.52", 0x180000, 0x080000, CRC(3faaa3e7) SHA1(ec3e6e8d16a8095c857ff270d2bd48c04664b62f) )
	ROM_LOAD( "4a.30", 0x200000, 0x080000, CRC(53627ea6) SHA1(02f9cc223427a2b78e60bc866fd6c73df07b438d) )
	ROM_LOAD( "4b.31", 0x280000, 0x080000, CRC(a7e20eb4) SHA1(73da86764a93350224ada21b3178dde0a34cc657) )
	ROM_LOAD( "4c.32", 0x300000, 0x080000, CRC(ad0fc76a) SHA1(112e934a2cab13f994d1873aaaec40d38d2c2deb) )
	ROM_LOAD( "4d.33", 0x380000, 0x080000, CRC(280f97af) SHA1(9fe74c67440d7c952f091fb77905b7515852e0fb) )

	ROM_REGION( 0x08000, "text_0", 0 )
	ROM_LOAD( "7text.115", 0x000000, 0x08000,  CRC(7fb47a44) SHA1(1b5401967f33dc232187bf9f2a402b71286c5fc2) )
	// some dumps contain an empty '1M' ROM located next to each 'text' ROM on the PCB?

	ROM_REGION( 0x400000, "gp9001_1", 0 )
	ROM_LOAD( "3a.68", 0x000000, 0x080000, CRC(ba8e378c) SHA1(d5eb4a839d6b3c2b9bf0bd87f06859a01a2c0cbf) )
	ROM_LOAD( "3b.69", 0x080000, 0x080000, CRC(a9e4c6c7) SHA1(4058b1b887f41494a70b0b09e581ef5e3a444a1c) )
	ROM_LOAD( "3c.70", 0x100000, 0x080000, CRC(ffc6fa95) SHA1(87d18520fae7eec9336fc8cfb1adc2923ea10f8d) )
	ROM_LOAD( "3d.71", 0x180000, 0x080000, CRC(3faaa3e7) SHA1(ec3e6e8d16a8095c857ff270d2bd48c04664b62f) )
	ROM_LOAD( "4a.87", 0x200000, 0x080000, CRC(53627ea6) SHA1(02f9cc223427a2b78e60bc866fd6c73df07b438d) )
	ROM_LOAD( "4b.88", 0x280000, 0x080000, CRC(a7e20eb4) SHA1(73da86764a93350224ada21b3178dde0a34cc657) )
	ROM_LOAD( "4c.89", 0x300000, 0x080000, CRC(ad0fc76a) SHA1(112e934a2cab13f994d1873aaaec40d38d2c2deb) )
	ROM_LOAD( "4d.90", 0x380000, 0x080000, CRC(280f97af) SHA1(9fe74c67440d7c952f091fb77905b7515852e0fb) )

	ROM_REGION( 0x08000, "text_1", 0 )
	ROM_LOAD( "7text.152", 0x000000, 0x08000,  CRC(7fb47a44) SHA1(1b5401967f33dc232187bf9f2a402b71286c5fc2) )
	// some dumps contain an empty '1M' ROM located next to each 'text' ROM on the PCB?

	ROM_REGION( 0x40000, "oki1", 0 )     /* ADPCM Samples */
	ROM_LOAD( "7adpcm.37", 0x00000, 0x40000, CRC(aefce555) SHA1(0d47190287957122fefdae17ccf6bcfaef8cd430) )

	ROM_REGION( 0x40000, "oki2", 0 )     /* ADPCM Samples */
	ROM_LOAD( "7adpcm.43", 0x00000, 0x40000, CRC(aefce555) SHA1(0d47190287957122fefdae17ccf6bcfaef8cd430) )
ROM_END

// The region comes from the EEPROM? so will need clones like FixEight
GAME( 1993, dt7,         0,        dt7,          dt7,        toaplan2_dt7_state,empty_init, ROT270, "Toaplan",         "DT7 (prototype)",              MACHINE_NOT_WORKING )
