// license:BSD-3-Clause
// copyright-holders:

/*******************************************************************
Tong Zi Maque
ST (斯特), 2003
Hardware info By Guru
---------------------

No PCB number
"MADE IN SPIDER" on some ROMs and spider web logo on PCB
MADE IN TAIWAN 2003
|--------------------------------------|
|BATT    62256     Z80-PROG.U2         |
|           62256    15MHz  Z80        |-|
|                           DS12B887.U12 |
|GFX1.U6                       13.560MHz |
|                                        |
|     GFX2.U5                           M|
|           |--------|         TLP620-4 A|
|           |ALTERA  |         TLP620-4 H|
|           |EPM7256 |   |--------|     J|
|           |SQC208  |   |ALTERA  |     O|
|           |--------|   |EPM7192 |     N|
|                        |SQC160  |     G|
|                        |--------|      |
|        U6295                93C46.U11  |
|78L05  29F002.U9   BUTTON    TLP620-4   |
|uPC1241H      T518A   TLP620-4          |
| VOL                                  |-|
|-|  10WAY  |-----|       22WAY        |
  |---------|     |--------------------|
Notes:
         Z80 - Zilog Z84C0008 Z80 CPU. Clock 13.560MHz
       U6295 - OKI M6295. Clock 1.13MHz [13.560/12]. Pin 7 HIGH.
       62256 - 32kB x8-bit SRAM (the one nearest to the Z80 is battery-backed)
      BUTTON - Reset? Test?
    TLP620-4 - Toshiba TLP620-4 Photocoupler. Package contains 4 driver/receiver pairs.
     EMP7256 - Altera Max 7000-Series EPM7256SQC208 Programmable Logic Device
     EMP7192 - Altera Max 7000-Series EPM7192SQC160 Programmable Logic Device
    uPC1241H - NEC uPC1241H Mono Audio Power Amp
       T518A - PST518A System Reset IC
       78L05 - 5V Regulator in TO92 package
       U5/U6 - 27C160 EPROM (graphics)
   93C46.U11 - Atmel AT93C46 EEPROM (128 bytes) (for game settings or high scores?)
 Z80-PROG.U2 - Winbond W29EE011 EEPROM (Z80 program). The databus on this ROM connects to the EPM7192 and the Z80 databus is also connected
               to the EPM7192 on different pins. Additionally there's no valid Z80 code and no plain text in the ROM. This all leads to it
               being highly likely that the program is encrypted.
      29F002 - AMD AM29F002 PLCC32 (surface-mounted) EEPROM. This is connected to the OKI chip and holds the audio samples.
DS12B887.U12 - Dallas DS12B887 Real Time Clock Module. This looks suspiciously like it's used for protection data? The PCB contains a battery
               (for game settings or high scores?) and a 93C46 EEPROM (for game settings or high scores?). No other data needs to be stored
               anywhere. The DS12B887 is connected to the EPM7192 chip. It's possible it might just be for time/bookkeeping. The chip
               (dated 9614) is dumped but I'd be surprised if the battery inside was still alive and the data is good, although the data isn't
               just garbage and reads the same each time so by some miracle it might be ok.
               The first 14 bytes are clock and control registers. The remaining space is general-purpose battery-backed RAM. These same chips
               were used often in PCs around the 286/386/486 era to store the CMOS data. The RAM can technically hold anything, maybe even
               a decryption key....


---

The DS12B887 is used both as a clock and as part of the protection. The game seeds
it itself at $3569 (register $18 = $96, register $58 = $B6) and reads it back at $3583/$3591
to build the key of a second, software descrambling stage: that stage turns the block at
0x5000 of the program ROM into the code which then runs from $86F8 in RAM.  The CPLD forces
data bit 6 to the value of address bit 6 on reads, which is why $B6 comes back as $F6 - 126
of the 128 dumped bytes obey that rule.  Register $05 holds a "region" byte which
must agree with 93c46 byte $7a in a 2-out-of-3 vote against the battery backed RAM
at $8F07 ($36E2), otherwise the game stops with ERROR 5.  The clock registers are
used for real too: $2E3A sets date and time, and the bookkeeping screen has both
lifetime and "today" counters.

The battery backed RAM at 0x8000-0x8fff also holds the setup password at $81E0,
which is never stored in the EEPROM.  Password to enter settings screen is 51898.
Set it then press SERVICE 2 times to enter.

Bookkeeping totals live in the 93c46 in three copies at offsets $00, $10 and $20.
$3FD8 compares them at boot and silently repairs a single bad copy; if no two of
them agree it zeroes all three.  There is no operator command to clear them, which
is what you would expect on a gambling board.  The eight settings are at offset
$40, each one a 0-7 index into the lookup tables at $444D, decoded by $2D23 into
$83B0.  Note that $8396, the raw value of setting 6, also picks which of the two
background sets is drawn, at $0CE7.

The graphics ROMs look incomplete.  Parsing every picture the program draws (the
calls to $39C8) gives a catalogue stored back to back as linear runs of tiles.
For plane 1 the highest tile reached is 0x3cdd, which is exactly the last tile
holding data in 2.u5 - a perfect fit.  For plane 0 the highest tile reached is
0x3577, but 1.u6 only holds data up to 0x30dd and is erased from there on, so
roughly 150k of graphics seem to be missing and some backgrounds draw blank.

TODO:
- identify more inputs, if they exist
- don't see outputs, are there some? (i.e. lamps, counters, hopper)
- screen raw parameters
- missing GFX on some backgrounds
*******************************************************************/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/ds128x.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class sterz80_state : public driver_device
{
public:
	sterz80_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_rtc(*this, "rtc"),
		m_tileram(*this, "tileram%u", 0U),
		m_attrram(*this, "attrram%u", 0U),
		m_tilerom(*this, "tiles")
	{ }

	void tongzi(machine_config &config) ATTR_COLD;

	void init_tongzi() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<ds12885_device> m_rtc;

	required_shared_ptr_array<uint8_t, 4> m_tileram;
	required_shared_ptr_array<uint8_t, 4> m_attrram;
	required_region_ptr<uint8_t> m_tilerom;

	uint8_t m_nmi_enable = 0;
	uint8_t m_rtc_as = 0;
	uint8_t m_rtc_addr = 0;

	void draw_layer(bitmap_ind16 &bitmap, const rectangle &cliprect, int block);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void eeprom_w(uint8_t data);
	void nmi_enable_w(uint8_t data);
	uint8_t rtc_r();
	void rtc_ctrl_w(uint8_t data);
	void rtc_bus_w(uint8_t data);

	void vblank_w(int state);

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


/*
    There are two planes, each 128x32 tiles, split over the four 0x1000 blocks:

        C000  plane 0, tiles, columns   0-63     E000  plane 0, tiles, columns  64-127
        C800  plane 0, attr,  columns   0-63     E800  plane 0, attr,  columns  64-127
        D000  plane 1, tiles, columns   0-63     F000  plane 1, tiles, columns  64-127
        D800  plane 1, attr,  columns   0-63     F800  plane 1, attr,  columns  64-127

    Only columns 0-71 are displayed, so the screen is 576x256; columns 72-127 of
    E000 hold unrelated leftovers and are off screen.  The width was confirmed by
    the ROM image catalogue, which stores pictures back to back as linear runs:
    on one screen the top band is made of a 16 wide image at C000 columns 0-15
    (indices 2d08-2df7), an 8 wide one at E000 columns 0-7 (2df8-2e6f) and a 48
    wide one at C000 columns 16-63 (2e70-313f), chaining with no gap, 16+8+48 = 72.
    $0CF7 is the routine which draws that particular set.

    Plane 0 is the opaque background and fetches its tiles from 1.u6, plane 1 is
    the overlay with pen 0 transparent and fetches from 2.u5.  Each ROM holds
    16384 tiles, so the tile code is 14 bits wide and bits 14-15 of the attribute
    are unused.

    The tiles are 8x8 with 16 bits per pixel holding a direct colour, so there is
    no palette at all: 0xffff marks a transparent pixel and is the only value with
    bit 15 set.
*/

void sterz80_state::draw_layer(bitmap_ind16 &bitmap, const rectangle &cliprect, int block)
{
	bool const opaque = (block & 1) == 0;   // C000 / E000 are the background plane
	int const xoff = (block & 2) ? 64 * 8 : 0;

	// plane 0 fetches from 1.u6, plane 1 from 2.u5
	uint16_t const bank = opaque ? 0x0000 : 0x4000;

	for (int ty = 0; ty < 32; ty++)
	{
		for (int tx = 0; tx < 64; tx++)
		{
			int const offs = ty * 64 + tx;
			uint16_t const code = ((m_tileram[block][offs] | (m_attrram[block][offs] << 8)) & 0x3fff) | bank;
			uint8_t const *src = &m_tilerom[code * 128];

			for (int y = 0; y < 8; y++)
			{
				int const sy = ty * 8 + y;

				for (int x = 0; x < 8; x++, src += 2)
				{
					int const sx = tx * 8 + x + xoff;

					if (!cliprect.contains(sx, sy))
						continue;

					uint16_t const pix = src[0] | (src[1] << 8);

					if (pix == 0xffff || (!opaque && pix == 0x0000))
						continue;

					bitmap.pix(sy, sx) = pix & 0x7fff;
				}
			}
		}
	}
}

uint32_t sterz80_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	// background plane first (C000 + E000), then the overlay (D000 + F000)
	static const int order[4] = { 0, 2, 1, 3 };

	for (int i = 0; i < 4; i++)
		draw_layer(bitmap, cliprect, order[i]);

	return 0;
}


void sterz80_state::machine_start()
{
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_rtc_as));
	save_item(NAME(m_rtc_addr));
}

void sterz80_state::eeprom_w(uint8_t data)
{
	// addressed latch: bit 0 is the value, bits 2-1 select the line it applies to
	switch ((data >> 1) & 0x03)
	{
		case 0: m_eeprom->cs_write(BIT(data, 0)); break;
		case 1: m_eeprom->clk_write(BIT(data, 0)); break;
		case 2: m_eeprom->di_write(BIT(data, 0)); break;
	}

	if (data & 0xf8)
		logerror("%s eeprom_w unknown bits written: %02x\n", machine().describe_context(), data);
}

void sterz80_state::nmi_enable_w(uint8_t data)
{
	m_nmi_enable = BIT(data, 7);

	if (data & 0x7f)
		logerror("%s nmi_enable_w unknown bits written: %02x\n", machine().describe_context(), data);
}

uint8_t sterz80_state::rtc_r()
{
	// the EPM7192 forces data bit 6 to the value of address bit 6 on reads
	return (m_rtc->data_r() & 0xbf) | (BIT(m_rtc_addr, 6) << 6);
}

void sterz80_state::rtc_ctrl_w(uint8_t data)
{
	// addressed latch, same encoding as the EEPROM port:
	// bit 0 is the value, bits 4-1 select the line
	int const line = (data >> 1) & 0x0f;
	int const val = BIT(data, 0);

	switch (line)
	{
		case 7:  break;             // CE#
		case 11: m_rtc_as = val; break; // AS
		case 2:  break;             // R/W
		case 3:  break;             // DS
		case 9:  break;             // write strobe
		default: logerror("%s rtc_ctrl_w unknown line %d = %d\n", machine().describe_context(), line, val);
	}
}

void sterz80_state::rtc_bus_w(uint8_t data)
{
	// the address is written while AS is high, the data while it is low
	if (m_rtc_as)
	{
		m_rtc_addr = data & 0x7f;
		m_rtc->address_w(m_rtc_addr);
	}
	else
	{
		m_rtc->data_w(data);
	}
}


void sterz80_state::program_map(address_map &map)
{
	map(0x0000, 0x5fff).rom().nopw(); // some spurious writes
	map(0x8000, 0x8fff).ram().share("nvram");
	map(0xc000, 0xc7ff).ram().share(m_tileram[0]);
	map(0xc800, 0xcfff).ram().share(m_attrram[0]);
	map(0xd000, 0xd7ff).ram().share(m_tileram[1]);
	map(0xd800, 0xdfff).ram().share(m_attrram[1]);
	map(0xe000, 0xe7ff).ram().share(m_tileram[2]);
	map(0xe800, 0xefff).ram().share(m_attrram[2]);
	map(0xf000, 0xf7ff).ram().share(m_tileram[3]);
	map(0xf800, 0xffff).ram().share(m_attrram[3]);

}

void sterz80_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();

	map(0x00, 0x00).portr("IN0");
	map(0x01, 0x01).portr("IN1");
	map(0x02, 0x02).portr("IN2");
	map(0x03, 0x03).r(FUNC(sterz80_state::rtc_r)).w(FUNC(sterz80_state::eeprom_w));
	map(0x10, 0x10).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x11, 0x11).w(FUNC(sterz80_state::rtc_bus_w));
	map(0x12, 0x12).w(FUNC(sterz80_state::rtc_ctrl_w));
	// high byte of the descrambler key seed: needs to read $8e-$91 so that
	// (value + 2) & $fc gives $90 at $359a.  Source unknown
	map(0x13, 0x13).lr8(NAME([] () { return 0x90; }));
	map(0x70, 0x70).w(FUNC(sterz80_state::nmi_enable_w));
}


static INPUT_PORTS_START( tongzi )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void sterz80_state::vblank_w(int state)
{
	if (state && m_nmi_enable)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void sterz80_state::tongzi(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 13.560_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sterz80_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &sterz80_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	EEPROM_93C46_16BIT(config, "eeprom");

	DS12885(config, "rtc", 32.768_kHz_XTAL); // TODO: should be DS12B887

	// video hardware
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(128*8, 32*8);
	screen.set_visarea(0, 72*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(sterz80_state::screen_update));
	screen.screen_vblank().set(FUNC(sterz80_state::vblank_w));
	screen.set_palette("palette");

	// the tiles hold GRB555 values directly, so palette entry n decodes to n
	PALETTE(config, "palette", palette_device::GRB_555);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 13.560_MHz_XTAL / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( tongzi )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "3.u2", 0x00000, 0x20000, CRC(8f394f91) SHA1(e49c682cb819885334c1f25d5221e3f59c21a7e2) ) // encrypted

	// plane 0 fetches from 1.u6, plane 1 from 2.u5
	ROM_REGION( 0x400000, "tiles", 0 )
	// only holds data up to tile 0x30dd (byte 0x186f00) and is erased from there
	// on, while the program draws pictures reaching tile 0x3577: graphics missing?
	ROM_LOAD( "1.u6", 0x000000, 0x200000, CRC(e885dcc9) SHA1(ea4a72eec7b65cb668c5abb64347426192bd0f86) )
	ROM_LOAD( "2.u5", 0x200000, 0x200000, CRC(a7c11185) SHA1(96c9479802db0594845b2b282553c73406b84bb0) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c46.u11", 0x00, 0x80, CRC(d20d6865) SHA1(054d6ac609f9f5b19f9c8e27ab7096d942a2b39e) ) // hand reset

	ROM_REGION( 0x80, "rtc", 0 )
	// register $05 holds the region byte, which must match 93c46 byte $7a ($12) for the 2-out-of-3 check at $36e2 to pass,
	// otherwise the game stops with ERROR 5
	ROM_LOAD( "ds12887.u12", 0x00, 0x80, CRC(4e70ae43) SHA1(cb9809ce25d9889d66f47f5e693fbde66b0ed37a) ) // hand repaired

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "am29f002.u9", 0x00000, 0x40000, CRC(4bbd8cfa) SHA1(690d7a98764162b0771629c02fd3c488761d8ec0) ) // encrypted
ROM_END


/*
    Program ROM encryption.

    The EPM7192 rewires the program ROM data bus and XORs the data with a value
    which depends on address lines A0, A1, A4, A5, A6 and A7 only (A2, A3 and
    everything from A8 upwards are not used - confirmed by index of coincidence,
    which peaks at delta 4/8/12 and at 256/512/768/1024).  The rewiring is not
    fixed: there are four different bit permutations, selected by the same six
    address lines.

    Only 0x00000-0x045af of the device is the actual encrypted program (RAM starts
    at 0x8000, so the Z80 ROM space is 0x0000-0x7fff).  There is a second encrypted
    block at 0x05000-0x053ff; everything else in the 128K device is unrelated
    leftover 8086/DOS code from the machine the ROM image was built on ("Lattice
    C 2.1", "Z80 & CP/M 2.2 Emulator v3.10 3/14/86", "*** Error opening file %s"),
    plus 0x18000-0x1ffff erased to 0xff.

    The block at 0x05000 is not junk either: it is the source of the code which the
    game descrambles into RAM at $86F8 and then calls.  $33C1 and $3360 run two
    passes over it, the first swapping bit n with bit 6 of each byte and the second
    complementing bit n, where n is the low three bits of a 16 bit key rotated left
    once per byte ($355E).  The key is ($8177), built from the DS12B887 as described
    at the top of this file, plus the region byte as an offset; for this ROM it
    works out to $90f6 with an offset of $12, which are also the values hardcoded
    at $3646 and $3661 for the paths this build never takes.
*/

void sterz80_state::init_tongzi()
{
	// decrypt main CPU ROM

	// XOR value, indexed by (A7,A6,A5,A4,A1,A0)
	static const uint8_t xor_table[64] =
	{
		0x9e, 0x1c, 0x96, 0x82,   // A7-A4 = 0
		0x05, 0x6b, 0xb0, 0xb0,   // A7-A4 = 1
		0x9f, 0x18, 0x97, 0xc2,   // A7-A4 = 2
		0x01, 0x6f, 0xf0, 0xb1,   // A7-A4 = 3
		0x13, 0x03, 0x36, 0x02,   // A7-A4 = 4
		0x31, 0x4b, 0x27, 0x30,   // A7-A4 = 5
		0x53, 0x07, 0x32, 0x06,   // A7-A4 = 6
		0x71, 0x4f, 0x23, 0x31,   // A7-A4 = 7
		0x94, 0x0a, 0x30, 0xbc,   // A7-A4 = 8
		0xfa, 0x28, 0xa5, 0x7a,   // A7-A4 = 9
		0x90, 0x4a, 0x34, 0xb8,   // A7-A4 = A
		0xfb, 0x68, 0xa1, 0x7e,   // A7-A4 = B
		0x1b, 0x9e, 0x10, 0x54,   // A7-A4 = C
		0x7a, 0x29, 0x58, 0x70,   // A7-A4 = D
		0x5b, 0x9a, 0x14, 0x55,   // A7-A4 = E
		0x7b, 0x69, 0x5c, 0x71    // A7-A4 = F
	};

	// which of the four data bus permutations is used, same index
	static const uint8_t swap_table[64] =
	{
		1, 2, 1, 3,   // A7-A4 = 0
		2, 0, 3, 1,   // A7-A4 = 1
		1, 2, 1, 3,   // A7-A4 = 2
		2, 0, 3, 1,   // A7-A4 = 3
		3, 0, 2, 0,   // A7-A4 = 4
		3, 0, 2, 1,   // A7-A4 = 5
		3, 0, 2, 0,   // A7-A4 = 6
		3, 0, 2, 1,   // A7-A4 = 7
		2, 3, 0, 2,   // A7-A4 = 8
		1, 3, 2, 0,   // A7-A4 = 9
		2, 3, 0, 2,   // A7-A4 = A
		1, 3, 2, 0,   // A7-A4 = B
		3, 2, 0, 1,   // A7-A4 = C
		1, 3, 0, 1,   // A7-A4 = D
		3, 2, 0, 1,   // A7-A4 = E
		1, 3, 0, 1    // A7-A4 = F
	};

	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x20000; i++)
	{
		uint8_t const k = (i & 0x03) | ((i & 0xf0) >> 2);
		uint8_t const v = rom[i] ^ xor_table[k];

		switch (swap_table[k])
		{
			case 0: rom[i] = bitswap<8>(v, 5, 6, 7, 4, 3, 1, 2, 0); break;
			case 1: rom[i] = bitswap<8>(v, 7, 5, 4, 6, 2, 1, 0, 3); break;
			case 2: rom[i] = bitswap<8>(v, 1, 4, 6, 7, 0, 3, 2, 5); break;
			case 3: rom[i] = bitswap<8>(v, 0, 1, 2, 3, 5, 4, 6, 7); break;
		}
	}

	// descramble the tile ROM data bus: the pixels hold a direct 15 bit colour, but
	// one channel is split between bits 14-13 and bits 2-0.
	uint8_t *const tiles = memregion("tiles")->base();

	for (int i = 0; i < 0x400000; i += 2)
	{
		uint16_t const pix = tiles[i] | (tiles[i + 1] << 8);
		uint16_t const out = bitswap<15>(pix, 14, 13, 2, 1, 0, 7, 6, 5, 4, 3, 12, 11, 10, 9, 8) | (pix & 0x8000); // keep the transparency flag

		tiles[i] = out & 0xff;
		tiles[i + 1] = out >> 8;
	}

	// decrypt M6295 ROM
	uint8_t *okirom = memregion("oki")->base();
	std::vector<uint8_t> buffer(0x40000);

	memcpy(&buffer[0], okirom, 0x40000);

	for (int i = 0; i < 0x40000; i++)
	{
		okirom[i] = buffer[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 4, 3, 1, 2, 5, 0, 10, 13, 8, 6, 15, 17, 7, 9, 12, 14, 11, 16)];
		okirom[i] = bitswap<8>(okirom[i], 0, 1, 2, 3, 4, 5, 6, 7);
	}
}

} // anonymous namespace


GAME( 2003, tongzi, 0, tongzi, tongzi, sterz80_state, init_tongzi, ROT0, "ST", "Tong Zi Maque", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
