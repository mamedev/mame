// license:BSD-3-Clause
// copyright-holders: Mark McDougall

/*****************************          (by Mark McDougall)
 *** STREET FIGHT hardware ***  This has been adapted from the excellent
 *****************************  Psychic 5 description (by Roberto Ventura)


 Notes:  Lower Board - CPU board        S-0086-002-B0  (both games)
         Top Board   - GFX board        S-0086-002A-A0 (Street Fight)
         Top Board   - GFX board        S-0087-807     (Cross Shooter)

 for the single PCB version of Air Raid / Cross Shooter on the S-0087-011A-0 PCB,
 see airraid.cpp


---

Street Fight (c) Seibu Kaihatsu (1986)


0) GENERAL.

The game has two Z80s.
The second CPU controls the two YM2203 sound chips.
There is an OKI M5205 ADPCM chip fed directly from a ROM.
Screen resolution is 256x224 (horizontal CRT).
256 colors on screen.
128 sprites (16x16).


1) ROM CONTENTS.

SF01    Main program code (encrypted)
SF02    Main program code bank-switched? into main CPU space
SF03    Sound program code
SF04    ADPCM voice data
SF05-07 Foreground tile pixel data
SF09    Foreground map data
SF10    Foreground map/tile data
SF11-14 Background tile pixel data
SF15    Background map data
SF16    Background map/tile data
SF17    Character pixel data
SF18-21 Sprite pixel data

All ROMs are 32K except SF17 which is 8K.

Graphics format is a little messy, the 4 planes come from a bit in
the high and low nibble of each byte of a consecutive pair of ROMs.

All graphics are made of 16x16 (composite) tiles, each of which is composed
of 4 consecutive 8x8 tiles. In all there are 1024 composite (16x16) tiles
for each of the foreground, background and sprite layers. These can be
considered as four banks each of 256 tiles.

Text characters are defined as 8x8 tiles.


2) CPU.

The board has a single? crystal @12.000 MHz.
Both CPU clocks run at 3MHz (12/4).

The main Z80 runs in Interrupt mode 0 (IM0), the game program expects
execution of two different restart (RST) instructions.
RST 10,the main IRQ, is to be triggered each time the screen is refreshed.
RST 08 must be triggered in order to make the game work properly. I haven't
ascertained the exact frequency of this interrupt yet, though the game
appears to run at the correct speed with RST08 at 30Hz. Curiously a trace
on the interrupt pin shows two interrupts occurring at 60Hz, obviously the
VBlank interrupt followed by a second interrupt some 3.3ms later. At some
stage I'll get around to probing the data lines to find the interrupt
vector addresses.

Sound CPU runs in IM1.

The sound CPU lies idle waiting the external IRQ occurrence executing code
from $0100 to $010D.

Game code/data is directly accessible by the main CPU. There is what
appears to be some level data in the second half of sf02 that may
never be bank-switched in!?! Graphics data, ADPCM samples and level maps
are not accessed by the CPUs.

Text video RAM is situated at $D000-$D7FF.

The 68705 MCU handles the two coin inputs and drives the MSM5205 ADPCM chip.
It also appears to serve as a simple protection device. During initialization,
the main Z80 writes a special ADPCM code to the MCU and then spins in a loop.
The MCU responds by asserting the Z80's /NMI line, and thus the init process
continues. The protection ADPCM code varies between versions of each game.

3) MAIN CPU MEMORY MAP.

$0000-$7FFF R   ROM sf01 (encrypted)
$8000-$BFFF R   ROM sf02 (2 x 16k banks selected by D2 of ?)
$C000-$C1FF W   Palette RAM
$C200       R   player 1 controls hard value (negative logic)
                - MSB:x,x,B2,B1,RIGHT,LEFT,DOWN,UP
$C201       R   player 2 controls hard value (negative logic)
                - MSB:x,x,B2,B1,RIGHT,LEFT,DOWN,UP
$C202       R   start buttons (negative logic)
                - MSB:x,x,x,P2,P1,x,x,x:LSB
$C203       R   dipswitch #1 hard value (negative logic)
                - 76543210
                  xxxxx000  - coin A - 1 coin  / 1 credit
                  xxxxx001           - 2 coins / 1 credit
                  xxxxx010           - 1 coin  / 3 credits
                  xxxxx011           - 4 coins / 1 credit
                  xxxxx100           - 1 coin  / 2 credits
                  xxxxx101           - 3 coins / 1 credit
                  xxxxx110           - 1 coin  / 5 credits
                  xxxxx111           - 5 coins / 1 credit
                  xxx00xxx  - coin B - 1 coin  / 1 credit
                  xxx01xxx           - 2 coins / 1 credit
                  xxx10xxx           - 1 coin  / 2 credits
                  xxx11xxx           - 2 coins / 3 credits
                  xx1xxxxx  - test mode setting
                  x1xxxxxx  - continue setting
                  1xxxxxxx  - bullet colour setting
$C204       R   dipswitch #2 hard value (negative logic)
                - 76543210
                  xxxxxxx1  - cabinet style
                  xxxxx11x  - difficulty
                  xxx11xxx  - number of lives (-1)
                  x00xxxxx  - 10,000 & 30,000
                  x01xxxxx  - 20,000 & 40,000
                  x10xxxxx  - 30,000 & 60,000
                  x11xxxxx  - 40,000 & 80,000
                  1xxxxxxx  - demo sound on/off
$C205       R   read to determine coin circuit check status
$C500       W   play fm number
$C600       W   play voice number
$C700       W   coin mechanism control
$C804       W   coin counters
                - b4    Unknown
                - b1    Coin counter 2
                - b0    Coin counter 1
$C806       W   ???
$C807       W   current sprite bank h/w register
                - bank = b2,b0
$D000-$D3FF W   VRAM (Character RAM)
$D400-$D7FF W   VRAM (Attribute RAM)
                - b7    Character bank from SF17
                - b6    Flip Y
                - b5    Flip X
                - b4    ?
                - b3-0  ? Colour/Palette
$D800-$D801 W   foreground layer x coordinate h/w register
$D802-$D803 W   foreground layer y coordinate h/w register
$D804-$D805 W   background layer x coordinate h/w register
$D806,$D808 W   background layer y coordinate h/w register
$D807       W   layer control h/w register
                - b7 = text layer
                - b6 = sprite layer
                - b5 = background layer
                - b4 = foreground layer
                - b0 = video orientation (1=upside-down)
$E000-$FFFF RW  RAM SRM2064C (8k)
$Fxx0-$Fxx3 W   Sprite Ram (every 32 ($20) bytes)
                - xx0 sprite number
                - xx1 sprite attribute
                  - b7  sign extension of x coord
                  - b4  flip x
                - xx2 y coord of sprite
                - xx3 x coord of sprite

4) SOUND CPU MEMORY MAP.

$0000-$7FFF R   ROM (sf03)
$C000       W   YM2203 #1 address register
$C001       W   YM2203 #1 data register
$C800       W   YM2203 #2 address register
$C801       W   YM2203 #2 data register
$D000       R   ADPCM code port (unused)
$D800       W   MSM5205 control (unused)
$E800       W   Debug port? (mirrors FM code)
$F000       R   FM Voice number to play
                - b7    set for valid data latched
                - b6-b0 voice number
$F800-$FFFF RW  RAM

empcity contains an NMI routine which appears to control an MSM5205.
However, it does not work properly. The NMI routine is not present in stfight.

5) COLOR RAM

The palette system is dynamic, the game can show up to 256 different
colors on screen.

Each color component (RGB) depth is 4 bits, two bytes $100 apart are used
for each color code (12 bits).

I suspect that the colors are organized in sub-palettes, since the graphics
layers are all 4 bits (16 colors) each. Each of the text/graphics layers
have 'attribute' bytes associated with them that would define the palette
usage for each character/tile.

The 16 colours at offset $C0 appear to be the text palette. This group of
colours does not appear to change throughout the game, and the lower 192
colours fade in/out independently of these 16 - consistent with observations
of the real game.

There is a related mystery with the transparency colour. For the most part
colour 15 corresponds to the transparent colour, except in a few cases.
(for some Seibu PCB types transparency is handled by bit 0x40 in the CLUT
 PROMs, but not here, unless they've been dumped incorrectly)

-----

Notes below are for Street Fight video board only

6) TILE-BASED LAYERS

The foreground and background layers comprise static virtual layers which
are 8 screens wide and 16 screens deep. The hardware scrolls around the
layers by reading registers which are updated by software every VBlank.
The text layer is fixed and cannot scroll.

The maps that define the foreground and background layers are stored in
ROMs accessed directly by the hardware. They consist of 256 bytes for
each screen which define the tile number, and a corresponding byte in
a matching ROM which defines the tile bank and presumably palette info.

The top and bottom rows of the screen are not visible - resulting in a
256x224 viewport rather than 256 square. The layers can be individually
enabled/disabled. Inactive sprites are 'parked' at row 0.

The ROM layout for the foreground and sprite tiles are as you would expect,
with the four 8x8 tiles that make a single composite tile consecutive in
address. The background tiles are interleaved for presumably some good
reason, the first two 8x8 tiles from composite tile n are followed by two
8x8 tiles from the (n+512)'th composite tile.

The map ROMs are similarly interleaved for the background layer only.

7) SPRITES

The sprites are mapped into RAM locations $F000-$FFFF using only the first
4 bytes from each 32-byte slice. Intervening addresses appear to be
conventional RAM. See the memory map for sprite data format.

******************************************************************************

TODO:
- handle transparency in text layer properly (how?)
- is the second bank of sf02 used? (probably NOT)
- empcity/stfight never writes the YM2203s' divider registers but it expects
  0x2f, there's a workaround for it in the driver init
- if empcity turns out to really be a bootleg, maybe it doesn't have an MCU,
  and instead does the ADPCM with the audio CPU? (see the driver notes above
  mentioning an unused NMI handler)
- Each version of empcity/stfight has a different protection code stored in the
  MCU (at $1D2) so each 68705 will need to be dumped.
  We currently use hacked versions of the empcityu MCU for each different set.
  Currently, the following protection codes are known:
  06 USA
  08 Japan
  09 Benelux
  0d Italy
  0e France
  0f Germany

*****************************************************************************/

#include "emu.h"

#include "airraid_dev.h"
#include "stfight_dev.h"

#include "cpu/m6805/m68705.h"
#include "cpu/z80/z80.h"
#include "sound/msm5205.h"
#include "sound/ymopn.h"

#include "speaker.h"


namespace {

class stfight_state : public driver_device
{
public:
	stfight_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_mcu(*this, "mcu")
		, m_msm(*this, "msm")
		, m_ym(*this, "ym%u", 0)
		, m_main_bank(*this, "mainbank")
		, m_samples(*this, "adpcm")
		, m_decrypted_opcodes(*this, "decrypted_opcodes")
		, m_coin_mech(*this, "COIN")
		, m_coin_state(0)
		, m_fm_data(0)
		, m_cpu_to_mcu_empty(true)
		, m_cpu_to_mcu_data(0x0f)
		, m_port_a_out(0xff)
		, m_port_c_out(0xff)
		, m_vck2(false)
		, m_adpcm_reset(true)
		, m_adpcm_data_offs(0x0000)
	{
	}

	void stfight_base(machine_config &config);
	void stfight(machine_config &config);
	void cshooter(machine_config &config);

	void init_stfight();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<m68705p5_device> m_mcu;
	required_device<msm5205_device> m_msm;
	required_device_array<ym2203_device, 2> m_ym;

	required_memory_bank m_main_bank;

	required_region_ptr<uint8_t> m_samples;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	required_ioport m_coin_mech;

	uint8_t m_coin_state = 0;

	uint8_t m_fm_data = 0;

	bool m_cpu_to_mcu_empty = false;
	uint8_t m_cpu_to_mcu_data = 0;
	uint8_t m_port_a_out = 0;
	uint8_t m_port_c_out = 0;

	bool m_vck2 = false;
	bool m_adpcm_reset = false;
	uint16_t m_adpcm_data_offs = 0;

	emu_timer *m_int1_timer = nullptr;

	void adpcm_int(int state);

	void io_w(uint8_t data);
	uint8_t coin_r();
	void coin_w(uint8_t data);
	void fm_w(uint8_t data);
	void mcu_w(uint8_t data);

	void cshooter_bank_w(uint8_t data);

	uint8_t fm_r();

	INTERRUPT_GEN_MEMBER(vb_interrupt);
	TIMER_CALLBACK_MEMBER(rst08_tick);

	// MCU specifics
	uint8_t _68705_port_b_r();
	void _68705_port_a_w(uint8_t data);
	void _68705_port_b_w(uint8_t data);
	void _68705_port_c_w(uint8_t data);

	void cpu1_map(address_map &map) ATTR_COLD;
	void cpu2_map(address_map &map) ATTR_COLD;
	void cshooter_cpu1_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void stfight_cpu1_map(address_map &map) ATTR_COLD;
};


void stfight_state::machine_start()
{
	m_main_bank->configure_entries(0, 4, memregion("maincpu")->base() + 0x10000, 0x4000);
	m_main_bank->set_entry(0);

	m_int1_timer = timer_alloc(FUNC(stfight_state::rst08_tick), this);

	save_item(NAME(m_coin_state));
	save_item(NAME(m_fm_data));

	save_item(NAME(m_cpu_to_mcu_empty));
	save_item(NAME(m_cpu_to_mcu_data));
	save_item(NAME(m_port_a_out));
	save_item(NAME(m_port_c_out));

	save_item(NAME(m_vck2));
	save_item(NAME(m_adpcm_reset));
	save_item(NAME(m_adpcm_data_offs));
}


void stfight_state::machine_reset()
{
	m_fm_data = 0;
	m_cpu_to_mcu_empty = true;
	m_adpcm_reset = true;

	// Coin signals are active low
	m_coin_state = 3;
}

// It's entirely possible that this bank is never switched out
// - in fact I don't even know how/where it's switched in!
void stfight_state::cshooter_bank_w(uint8_t data)
{
	m_main_bank->set_entry(bitswap(data, 7, 2));
}

/*
 *      CPU 1 timed interrupt - 60Hz???
 */

TIMER_CALLBACK_MEMBER(stfight_state::rst08_tick)
{
	m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7); // Z80
}

INTERRUPT_GEN_MEMBER(stfight_state::vb_interrupt)
{
	// Do a RST10
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0xcf); // Z80
	m_int1_timer->adjust(attotime::from_hz(120));
}

/*
 *      Hardware handlers
 */

void stfight_state::io_w(uint8_t data)
{
	// TODO: What is bit 4?
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);
}

uint8_t stfight_state::coin_r()
{
	return m_coin_state;
}

void stfight_state::coin_w(uint8_t data)
{
	// Acknowledge coin signals (active low)
	if (!BIT(data, 0))
		m_coin_state |= 1;

	if (!BIT(data, 1))
		m_coin_state |= 2;
}

/*
 *      Machine hardware for MSM5205 ADPCM sound control
 */

void stfight_state::adpcm_int(int state)
{
	if (!state)
		return;

	// Falling edge triggered interrupt at half the rate of /VCK?
	m_mcu->set_input_line(M68705_IRQ_LINE, m_vck2 ? ASSERT_LINE : CLEAR_LINE);
	m_vck2 = !m_vck2;

	if (!m_adpcm_reset)
	{
		uint8_t adpcm_data = m_samples[(m_adpcm_data_offs >> 1) & 0x7fff];

		if (!BIT(m_adpcm_data_offs, 0))
			adpcm_data >>= 4;
		++m_adpcm_data_offs;

		m_msm->data_w(adpcm_data & 0x0f);
	}
}


/*
 *      Machine hardware for YM2303 FM sound control
 */

void stfight_state::fm_w(uint8_t data)
{
	// The sound CPU ignores any FM data without bit 7 set
	m_fm_data = 0x80 | data;
}

uint8_t stfight_state::fm_r()
{
	uint8_t const data = m_fm_data;

	// Acknowledge the command
	if (!machine().side_effects_disabled())
		m_fm_data &= ~0x80;

	return data;
}


/*
 *  MCU communications
 */

void stfight_state::mcu_w(uint8_t data)
{
	m_cpu_to_mcu_data = data & 0x0f;
	m_cpu_to_mcu_empty = false;
}

void stfight_state::_68705_port_a_w(uint8_t data)
{
	m_port_a_out = data;
}

uint8_t stfight_state::_68705_port_b_r()
{
	return
			(m_coin_mech->read() << 6) |
			(m_cpu_to_mcu_empty ? 0x10 : 0x00) |
			(m_cpu_to_mcu_data & 0x0f);
}

void stfight_state::_68705_port_b_w(uint8_t data)
{
	// Acknowledge Z80 command
	if (!BIT(data, 5))
		m_cpu_to_mcu_empty = true;
}

void stfight_state::_68705_port_c_w(uint8_t data)
{
	// Signal a valid coin on the falling edge
	if (BIT(m_port_c_out, 0) && !BIT(data, 0))
		m_coin_state &= ~1;
	if (BIT(m_port_c_out, 1) && !BIT(data, 1))
		m_coin_state &= ~2;

	// Latch ADPCM data address when dropping the reset line
	m_adpcm_reset = BIT(data, 2);
	if (!m_adpcm_reset && BIT(m_port_c_out, 2))
		m_adpcm_data_offs = m_port_a_out << 9;
	m_msm->reset_w(m_adpcm_reset ? ASSERT_LINE : CLEAR_LINE);

	// Generate NMI on host CPU (used on handshake error or stuck coin)
	m_maincpu->set_input_line(INPUT_LINE_NMI, BIT(data, 3) ? CLEAR_LINE : ASSERT_LINE);

	m_port_c_out = data;
}


void stfight_state::cpu1_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_main_bank); // sf02.bin
	map(0xc000, 0xc0ff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0xc100, 0xc1ff).ram().w("palette", FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xc200, 0xc200).portr("P1");
	map(0xc201, 0xc201).portr("P2");
	map(0xc202, 0xc202).portr("START");
	map(0xc203, 0xc203).portr("DSW0");
	map(0xc204, 0xc204).portr("DSW1");
	map(0xc205, 0xc205).r(FUNC(stfight_state::coin_r));
	map(0xc500, 0xc500).w(FUNC(stfight_state::fm_w));
	map(0xc600, 0xc600).w(FUNC(stfight_state::mcu_w));
	map(0xc700, 0xc700).w(FUNC(stfight_state::coin_w));
	map(0xc804, 0xc804).w(FUNC(stfight_state::io_w));
	map(0xc806, 0xc806).nopw(); // TBD
	map(0xe000, 0xefff).ram();
}

void stfight_state::stfight_cpu1_map(address_map &map)
{
	cpu1_map(map);
	map(0xc807, 0xc807).w("stfight_vid", FUNC(stfight_video_device::stfight_sprite_bank_w));
	map(0xd000, 0xd7ff).ram().w("stfight_vid", FUNC(stfight_video_device::stfight_text_char_w)).share("txram");
	map(0xd800, 0xd808).ram().w("stfight_vid", FUNC(stfight_video_device::stfight_vh_latch_w)).share("vregs");
	map(0xf000, 0xffff).ram().share("sprite_ram");
}


void stfight_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().share(m_decrypted_opcodes);
}


void stfight_state::cshooter_cpu1_map(address_map &map)
{
	cpu1_map(map);
	map(0xc801, 0xc801).w(FUNC(stfight_state::cshooter_bank_w));
	map(0xd000, 0xd7ff).ram().w("airraid_vid", FUNC(airraid_video_device::txram_w)).share("txram");
	map(0xd800, 0xd80f).ram().w("airraid_vid", FUNC(airraid_video_device::vregs_w)).share("vregs"); // wrong?
	map(0xf000, 0xfdff).ram();
	map(0xfe00, 0xffff).ram().share("sprite_ram");
}



void stfight_state::cpu2_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc001).rw(m_ym[0], FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xc800, 0xc801).rw(m_ym[1], FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xd000, 0xd000).nopr();
	map(0xd800, 0xd800).nopw();
	map(0xe800, 0xe800).nopw();
	map(0xf000, 0xf000).r(FUNC(stfight_state::fm_r));
	map(0xf800, 0xffff).ram();
}


static INPUT_PORTS_START( stfight )
	PORT_START("P1")    // PLAYER 1
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")    // PLAYER 2
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("START") // START BUTTONS
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xe7, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")  // DSW1
	// Manual refers to these as Dip-Switch Bank B, but TEST mode shows them as DIP-SW 1
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_SERVICE_DIPLOC(  0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Bullet Colour" )     PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Red" )
	PORT_DIPSETTING(    0x80, "Blue" )

	PORT_START("DSW1")  // DSWA
	// Manual refers to these as Dip-Switch Bank A, but TEST mode shows them as DIP-SW 2
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x04, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, "10000 30000" )
	PORT_DIPSETTING(    0x40, "20000 40000" )
	PORT_DIPSETTING(    0x20, "30000 60000" )
	PORT_DIPSETTING(    0x00, "40000 80000" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COIN")  // COIN MECH
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END

static INPUT_PORTS_START( cshooter )
	PORT_INCLUDE(stfight)

	PORT_MODIFY("DSW0")  // DSW2 (0xc203)
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "2k 10k 20k" )
	PORT_DIPSETTING(    0x08, "5k 20k 40k" )
	PORT_DIPSETTING(    0x04, "6k 30k 60k" )
	PORT_DIPSETTING(    0x00, "7k 40k 80k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_MODIFY("DSW1")  // DSW1 (0xc204)
	PORT_DIPNAME( 0x01, 0x01, "Coin Slots" )        PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_SERVICE_DIPLOC(  0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )
INPUT_PORTS_END



void stfight_state::stfight_base(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 12_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &stfight_state::cpu1_map);

	Z80(config, m_audiocpu, 12_MHz_XTAL / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &stfight_state::cpu2_map);
	m_audiocpu->set_periodic_int(FUNC(stfight_state::irq0_line_hold), attotime::from_hz(120));

	M68705P5(config, m_mcu, 12_MHz_XTAL / 4);
	m_mcu->portb_r().set(FUNC(stfight_state::_68705_port_b_r));
	m_mcu->porta_w().set(FUNC(stfight_state::_68705_port_a_w));
	m_mcu->portb_w().set(FUNC(stfight_state::_68705_port_b_w));
	m_mcu->portc_w().set(FUNC(stfight_state::_68705_port_c_w));

	config.set_maximum_quantum(attotime::from_hz(600));

	PALETTE(config, "palette").set_format(palette_device::xBRG_444, 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2203(config, m_ym[0], 12_MHz_XTAL / 8);
	m_ym[0]->add_route(0, "mono", 0.15);
	m_ym[0]->add_route(1, "mono", 0.15);
	m_ym[0]->add_route(2, "mono", 0.15);
	m_ym[0]->add_route(3, "mono", 0.10);

	YM2203(config, m_ym[1], 12_MHz_XTAL / 8);
	m_ym[1]->add_route(0, "mono", 0.15);
	m_ym[1]->add_route(1, "mono", 0.15);
	m_ym[1]->add_route(2, "mono", 0.15);
	m_ym[1]->add_route(3, "mono", 0.10);

	MSM5205(config, m_msm, 384_kHz_XTAL);
	m_msm->vck_callback().set(FUNC(stfight_state::adpcm_int)); // Interrupt function
	m_msm->set_prescaler_selector(msm5205_device::S48_4B); // 8KHz, 4-bit
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void stfight_state::stfight(machine_config &config)
{
	stfight_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &stfight_state::stfight_cpu1_map);
	m_maincpu->set_addrmap(AS_OPCODES, &stfight_state::decrypted_opcodes_map);
	m_maincpu->set_vblank_int("stfight_vid:screen", FUNC(stfight_state::vb_interrupt));

	STFIGHT_VIDEO(config, "stfight_vid", 0);
}

void stfight_state::cshooter(machine_config &config)
{
	stfight_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &stfight_state::cshooter_cpu1_map);
	m_maincpu->set_vblank_int("airraid_vid:screen", FUNC(stfight_state::vb_interrupt));

	AIRRAID_VIDEO(config, "airraid_vid", 0);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

// Is this a bootleg? The MCU protection check at $00A7 has been disabled
ROM_START( empcity )
	ROM_REGION( 2*0x18000, "maincpu", 0 )
	ROM_LOAD( "ec_01.rom",  0x00000, 0x8000, CRC(fe01d9b1) SHA1(c4b62d1b7e3a062f6a7a75f49cce5712f9016f98) )
	ROM_LOAD( "ec_02.rom",  0x10000, 0x8000, CRC(b3cf1ef7) SHA1(91bc92293cbb47c38a2552c5beea53894b87d446) ) // bank switched

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ec_04.rom",  0x0000,  0x8000, CRC(aa3e7d1e) SHA1(da350384d55f011253d19ce17fc327cd2604257f) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "empcityu_68705.3j",  0x0000,  0x0800, CRC(182f7616) SHA1(38b4f23a559ae13f8ca1b974407a2a40fc52879f) )

	ROM_REGION( 0x02000, "stfight_vid:tx_gfx", 0 )
	ROM_LOAD( "17.2n",   0x0000, 0x2000, CRC(1b3706b5) SHA1(61f069329a7a836523ffc8cce915b0d0129fd896) )

	ROM_REGION( 0x20000, "stfight_vid:fg_gfx", 0 )
	ROM_LOAD( "7.4c",   0x10000, 0x8000, CRC(2c6caa5f) SHA1(f6893cb87004979ead331897c684f995f850447e) )
	ROM_LOAD( "8.5c",   0x18000, 0x8000, CRC(e11ded31) SHA1(e3e634ad324d51e52d79dd79e5e6e5697cb8d21f) )
	ROM_LOAD( "5.2c",   0x00000, 0x8000, CRC(0c099a31) SHA1(dabaf8edc59e4954941cd8176031a358f45a1956) )
	ROM_LOAD( "6.3c",   0x08000, 0x8000, CRC(3cc77c31) SHA1(13d2324df5a322d499c9959a6bb3a844edaefb45) )

	ROM_REGION( 0x20000, "stfight_vid:bg_gfx", 0 )
	ROM_LOAD( "13.4c",   0x10000, 0x8000, CRC(0ae48dd3) SHA1(ca3d9aeb9f4343c379cef9282e408fbf8aa67d99) )
	ROM_LOAD( "14.5j",   0x18000, 0x8000, CRC(debf5d76) SHA1(eb18c35166eb5f93be98b3c30c7d909c0a68eada) )
	ROM_LOAD( "11.2j",   0x00000, 0x8000, CRC(8261ecfe) SHA1(5817f4a0458a949298414fe09c86bbcf50be52f3) )
	ROM_LOAD( "12.3j",   0x08000, 0x8000, CRC(71137301) SHA1(087a9f401939bc30f1dafa9916e8d8c564595a57) )

	ROM_REGION( 0x20000, "stfight_vid:spr_gfx", 0 )
	ROM_LOAD( "20.8w",   0x10000, 0x8000, CRC(8299f247) SHA1(71891f7b1fbfaed14c3854b7f6e10a3ddb4bd479) )
	ROM_LOAD( "21.9w",   0x18000, 0x8000, CRC(b57dc037) SHA1(69ac79a95ba9ace7c9ca7af480a4a10176be5ace) )
	ROM_LOAD( "18.6w",   0x00000, 0x8000, CRC(68acd627) SHA1(f98ff9ccb0913711079a2988e8dd08695fb5e107) )
	ROM_LOAD( "19.7w",   0x08000, 0x8000, CRC(5170a057) SHA1(9222f9febc222fa0c2eead258ad77c857f6d40c8) )

	ROM_REGION( 0x10000, "stfight_vid:fg_map", 0 )
	ROM_LOAD( "9.7c",   0x00000, 0x8000, CRC(8ceaf4fe) SHA1(5698f2ff44c109825b8d9d0b6dd2426624df668b) )
	ROM_LOAD( "10.8c",   0x08000, 0x8000, CRC(5a1a227a) SHA1(24928ab218824ae1f5380398ceb90dcad525cc08) )

	ROM_REGION( 0x10000, "stfight_vid:bg_map", 0 )
	ROM_LOAD( "15.7j",   0x00000, 0x8000, CRC(27a310bc) SHA1(dd30d72bc33b0bf7ddaf3ab730e028f51b20152a) )
	ROM_LOAD( "16.8j",   0x08000, 0x8000, CRC(3d19ce18) SHA1(38f691a23c96ef672637965c1a13f6d1595f9d51) )

	ROM_REGION( 0x0100, "stfight_vid:tx_clut", 0 )
	ROM_LOAD( "82s129.006", 0x0000, 0x0100, CRC(f9424b5b) SHA1(e3bc23213406d35d54f1221f17f25d433df273a2) )

	ROM_REGION( 0x0100, "stfight_vid:fg_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.002", 0x0000, 0x0100, CRC(c883d49b) SHA1(e84900ccf6f27e5043e43c0d85ea1e4eee7e52d3) )
	ROM_LOAD_NIB_LOW(  "82s129.003", 0x0000, 0x0100, CRC(af81882a) SHA1(b1008c991bd8d1157b3479e465ab286c70418b58) )

	ROM_REGION( 0x0100, "stfight_vid:bg_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.004", 0x0000, 0x0100, CRC(1831ce7c) SHA1(57afbee9225f0efd63895a5f522e96dc87ca2616) )
	ROM_LOAD_NIB_LOW(  "82s129.005", 0x0000, 0x0100, CRC(96cb6293) SHA1(1dcdeaa995e6ffa3753b742842c5ffe0f68ef8cd) )

	ROM_REGION( 0x0100, "stfight_vid:spr_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.052", 0x0000, 0x0100, CRC(3d915ffc) SHA1(921be6d5e5fc0fdee9c9f545c1c4a0c334e9844c) )
	ROM_LOAD_NIB_LOW(  "82s129.066", 0x0000, 0x0100, CRC(51e8832f) SHA1(ed8c00559e7a02bb8c11861d747c8c64c01b7437) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "82s129.015", 0x0700, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )  // timing? (not used)

	ROM_REGION( 0x08000, "adpcm", 0 )
	ROM_LOAD( "5j",   0x00000, 0x8000, CRC(1b8d0c07) SHA1(c163ccd2b7ed6c84facc075eb1564ca399f3ba17) )
ROM_END

// set just contained the 3 ROMs cpu.4u, cpu.2u and vid.2p and the PROM 82s123.a7
ROM_START( empcityu )
	ROM_REGION( 2*0x18000, "maincpu", 0 )
	ROM_LOAD( "cpu.4u",  0x00000, 0x8000, CRC(e2c40ea3) SHA1(fd3c21fe3b5faf323a16be54ad2eed23b12c977e) )
	ROM_LOAD( "cpu.2u",  0x10000, 0x8000, CRC(96ee8b81) SHA1(95b516c023766fae79241d4422814e39e268ae7d) )    // bank switched

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ec_04.rom",  0x0000,  0x8000, CRC(aa3e7d1e) SHA1(da350384d55f011253d19ce17fc327cd2604257f) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "empcityu_68705.3j",  0x0000,  0x0800, CRC(182f7616) SHA1(38b4f23a559ae13f8ca1b974407a2a40fc52879f) )

	ROM_REGION( 0x02000, "stfight_vid:tx_gfx", 0 )
	ROM_LOAD( "vid.2p",   0x0000, 0x2000, CRC(15593793) SHA1(ac9ca8a0aa0ce3810f45aa41e74d4946ecced245) )

	ROM_REGION( 0x20000, "stfight_vid:fg_gfx", 0 )
	ROM_LOAD( "7.4c",   0x10000, 0x8000, CRC(2c6caa5f) SHA1(f6893cb87004979ead331897c684f995f850447e) )
	ROM_LOAD( "8.5c",   0x18000, 0x8000, CRC(e11ded31) SHA1(e3e634ad324d51e52d79dd79e5e6e5697cb8d21f) )
	ROM_LOAD( "5.2c",   0x00000, 0x8000, CRC(0c099a31) SHA1(dabaf8edc59e4954941cd8176031a358f45a1956) )
	ROM_LOAD( "6.3c",   0x08000, 0x8000, CRC(3cc77c31) SHA1(13d2324df5a322d499c9959a6bb3a844edaefb45) )

	ROM_REGION( 0x20000, "stfight_vid:bg_gfx", 0 )
	ROM_LOAD( "13.4c",   0x10000, 0x8000, CRC(0ae48dd3) SHA1(ca3d9aeb9f4343c379cef9282e408fbf8aa67d99) )
	ROM_LOAD( "14.5j",   0x18000, 0x8000, CRC(debf5d76) SHA1(eb18c35166eb5f93be98b3c30c7d909c0a68eada) )
	ROM_LOAD( "11.2j",   0x00000, 0x8000, CRC(8261ecfe) SHA1(5817f4a0458a949298414fe09c86bbcf50be52f3) )
	ROM_LOAD( "12.3j",   0x08000, 0x8000, CRC(71137301) SHA1(087a9f401939bc30f1dafa9916e8d8c564595a57) )

	ROM_REGION( 0x20000, "stfight_vid:spr_gfx", 0 )
	ROM_LOAD( "20.8w",   0x10000, 0x8000, CRC(8299f247) SHA1(71891f7b1fbfaed14c3854b7f6e10a3ddb4bd479) )
	ROM_LOAD( "21.9w",   0x18000, 0x8000, CRC(b57dc037) SHA1(69ac79a95ba9ace7c9ca7af480a4a10176be5ace) )
	ROM_LOAD( "18.6w",   0x00000, 0x8000, CRC(68acd627) SHA1(f98ff9ccb0913711079a2988e8dd08695fb5e107) )
	ROM_LOAD( "19.7w",   0x08000, 0x8000, CRC(5170a057) SHA1(9222f9febc222fa0c2eead258ad77c857f6d40c8) )

	ROM_REGION( 0x10000, "stfight_vid:fg_map", 0 )
	ROM_LOAD( "9.7c",   0x00000, 0x8000, CRC(8ceaf4fe) SHA1(5698f2ff44c109825b8d9d0b6dd2426624df668b) )
	ROM_LOAD( "10.8c",   0x08000, 0x8000, CRC(5a1a227a) SHA1(24928ab218824ae1f5380398ceb90dcad525cc08) )

	ROM_REGION( 0x10000, "stfight_vid:bg_map", 0 )
	ROM_LOAD( "15.7j",   0x00000, 0x8000, CRC(27a310bc) SHA1(dd30d72bc33b0bf7ddaf3ab730e028f51b20152a) )
	ROM_LOAD( "16.8j",   0x08000, 0x8000, CRC(3d19ce18) SHA1(38f691a23c96ef672637965c1a13f6d1595f9d51) )

	ROM_REGION( 0x0100, "stfight_vid:tx_clut", 0 )
	ROM_LOAD( "82s129.006", 0x0000, 0x0100, CRC(f9424b5b) SHA1(e3bc23213406d35d54f1221f17f25d433df273a2) )

	ROM_REGION( 0x0100, "stfight_vid:fg_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.002", 0x0000, 0x0100, CRC(c883d49b) SHA1(e84900ccf6f27e5043e43c0d85ea1e4eee7e52d3) )
	ROM_LOAD_NIB_LOW(  "82s129.003", 0x0000, 0x0100, CRC(af81882a) SHA1(b1008c991bd8d1157b3479e465ab286c70418b58) )

	ROM_REGION( 0x0100, "stfight_vid:bg_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.004", 0x0000, 0x0100, CRC(1831ce7c) SHA1(57afbee9225f0efd63895a5f522e96dc87ca2616) )
	ROM_LOAD_NIB_LOW(  "82s129.005", 0x0000, 0x0100, CRC(96cb6293) SHA1(1dcdeaa995e6ffa3753b742842c5ffe0f68ef8cd) )

	ROM_REGION( 0x0100, "stfight_vid:spr_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.052", 0x0000, 0x0100, CRC(3d915ffc) SHA1(921be6d5e5fc0fdee9c9f545c1c4a0c334e9844c) )
	ROM_LOAD_NIB_LOW(  "82s129.066", 0x0000, 0x0100, CRC(51e8832f) SHA1(ed8c00559e7a02bb8c11861d747c8c64c01b7437) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "82s129.015", 0x0700, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )  // timing? (not used)

	ROM_REGION( 0x08000, "adpcm", 0 )
	ROM_LOAD( "5j",   0x00000, 0x8000, CRC(1b8d0c07) SHA1(c163ccd2b7ed6c84facc075eb1564ca399f3ba17) )

	ROM_REGION( 0x020, "user1", 0 )
	ROM_LOAD( "82s123.a7", 0x0000, 0x0020, CRC(93e2d292) SHA1(af8edd0cfe85f28ede9604cfaf4516d54e5277c9) )   // ?
ROM_END

ROM_START( empcityj )
	ROM_REGION( 2*0x18000, "maincpu", 0 )
	ROM_LOAD( "1.bin",   0x00000, 0x8000, CRC(8162331c) SHA1(f2fdf5fbc52d4ea692fb87fa049c48935a73d67b) ) // sldh
	ROM_LOAD( "2.bin",   0x10000, 0x8000, CRC(960edea6) SHA1(fd19475e841defe42625a94c40c6390b7e6e7682) ) // bank switched // sldh

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ec_04.rom",  0x0000,  0x8000, CRC(aa3e7d1e) SHA1(da350384d55f011253d19ce17fc327cd2604257f) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "empcityj_68705.3j",  0x0000,  0x0800, BAD_DUMP CRC(19bdb0a9) SHA1(6baba9a46d64ae8349c7e9713419141f76a7af96) )

	ROM_REGION( 0x02000, "stfight_vid:tx_gfx", 0 )
	ROM_LOAD( "17.2n",   0x0000, 0x2000, CRC(1b3706b5) SHA1(61f069329a7a836523ffc8cce915b0d0129fd896) )

	ROM_REGION( 0x20000, "stfight_vid:fg_gfx", 0 )
	ROM_LOAD( "7.4c",   0x10000, 0x8000, CRC(2c6caa5f) SHA1(f6893cb87004979ead331897c684f995f850447e) )
	ROM_LOAD( "8.5c",   0x18000, 0x8000, CRC(e11ded31) SHA1(e3e634ad324d51e52d79dd79e5e6e5697cb8d21f) )
	ROM_LOAD( "5.2c",   0x00000, 0x8000, CRC(0c099a31) SHA1(dabaf8edc59e4954941cd8176031a358f45a1956) )
	ROM_LOAD( "6.3c",   0x08000, 0x8000, CRC(3cc77c31) SHA1(13d2324df5a322d499c9959a6bb3a844edaefb45) )

	ROM_REGION( 0x20000, "stfight_vid:bg_gfx", 0 )
	ROM_LOAD( "13.4c",   0x10000, 0x8000, CRC(0ae48dd3) SHA1(ca3d9aeb9f4343c379cef9282e408fbf8aa67d99) )
	ROM_LOAD( "14.5j",   0x18000, 0x8000, CRC(debf5d76) SHA1(eb18c35166eb5f93be98b3c30c7d909c0a68eada) )
	ROM_LOAD( "11.2j",   0x00000, 0x8000, CRC(8261ecfe) SHA1(5817f4a0458a949298414fe09c86bbcf50be52f3) )
	ROM_LOAD( "12.3j",   0x08000, 0x8000, CRC(71137301) SHA1(087a9f401939bc30f1dafa9916e8d8c564595a57) )

	ROM_REGION( 0x20000, "stfight_vid:spr_gfx", 0 )
	ROM_LOAD( "20.8w",   0x10000, 0x8000, CRC(8299f247) SHA1(71891f7b1fbfaed14c3854b7f6e10a3ddb4bd479) )
	ROM_LOAD( "21.9w",   0x18000, 0x8000, CRC(b57dc037) SHA1(69ac79a95ba9ace7c9ca7af480a4a10176be5ace) )
	ROM_LOAD( "18.6w",   0x00000, 0x8000, CRC(68acd627) SHA1(f98ff9ccb0913711079a2988e8dd08695fb5e107) )
	ROM_LOAD( "19.7w",   0x08000, 0x8000, CRC(5170a057) SHA1(9222f9febc222fa0c2eead258ad77c857f6d40c8) )

	ROM_REGION( 0x10000, "stfight_vid:fg_map", 0 )
	ROM_LOAD( "9.7c",   0x00000, 0x8000, CRC(8ceaf4fe) SHA1(5698f2ff44c109825b8d9d0b6dd2426624df668b) )
	ROM_LOAD( "10.8c",   0x08000, 0x8000, CRC(5a1a227a) SHA1(24928ab218824ae1f5380398ceb90dcad525cc08) )

	ROM_REGION( 0x10000, "stfight_vid:bg_map", 0 )
	ROM_LOAD( "15.7j",   0x00000, 0x8000, CRC(27a310bc) SHA1(dd30d72bc33b0bf7ddaf3ab730e028f51b20152a) )
	ROM_LOAD( "16.8j",   0x08000, 0x8000, CRC(3d19ce18) SHA1(38f691a23c96ef672637965c1a13f6d1595f9d51) )

	ROM_REGION( 0x0100, "stfight_vid:tx_clut", 0 )
	ROM_LOAD( "82s129.006", 0x0000, 0x0100, CRC(f9424b5b) SHA1(e3bc23213406d35d54f1221f17f25d433df273a2) )

	ROM_REGION( 0x0100, "stfight_vid:fg_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.002", 0x0000, 0x0100, CRC(c883d49b) SHA1(e84900ccf6f27e5043e43c0d85ea1e4eee7e52d3) )
	ROM_LOAD_NIB_LOW(  "82s129.003", 0x0000, 0x0100, CRC(af81882a) SHA1(b1008c991bd8d1157b3479e465ab286c70418b58) )

	ROM_REGION( 0x0100, "stfight_vid:bg_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.004", 0x0000, 0x0100, CRC(1831ce7c) SHA1(57afbee9225f0efd63895a5f522e96dc87ca2616) )
	ROM_LOAD_NIB_LOW(  "82s129.005", 0x0000, 0x0100, CRC(96cb6293) SHA1(1dcdeaa995e6ffa3753b742842c5ffe0f68ef8cd) )

	ROM_REGION( 0x0100, "stfight_vid:spr_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.052", 0x0000, 0x0100, CRC(3d915ffc) SHA1(921be6d5e5fc0fdee9c9f545c1c4a0c334e9844c) )
	ROM_LOAD_NIB_LOW(  "82s129.066", 0x0000, 0x0100, CRC(51e8832f) SHA1(ed8c00559e7a02bb8c11861d747c8c64c01b7437) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "82s129.015", 0x0700, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )  // timing? (not used)

	ROM_REGION( 0x08000, "adpcm", 0 )
	ROM_LOAD( "5j",   0x00000, 0x8000, CRC(1b8d0c07) SHA1(c163ccd2b7ed6c84facc075eb1564ca399f3ba17) )
ROM_END

ROM_START( stfight )
	ROM_REGION( 2*0x18000, "maincpu", 0 )
	ROM_LOAD( "a-1.4q",     0x00000, 0x8000, CRC(ff83f316) SHA1(84553ebd96ddbf59a1bcb221d53781980a006925) )
	ROM_LOAD( "sf02.bin",   0x10000, 0x8000, CRC(e626ce9e) SHA1(2c6c5a5cf15cc50217c9864a4d861af8a1b1b5ad) ) // bank switched

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sf03.bin",   0x0000,  0x8000, CRC(6a8cb7a6) SHA1(dc123cc48d3623752b78e7c23dd8d2f5adf84f92) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "_68705.3j",  0x0000,  0x0800, BAD_DUMP  CRC(f4cc50d6) SHA1(2ff62a349b74fa965b5d19615e52b867c04988dc) )

	ROM_REGION( 0x02000, "stfight_vid:tx_gfx", 0 )
	ROM_LOAD( "17.2n",   0x0000, 0x2000, CRC(1b3706b5) SHA1(61f069329a7a836523ffc8cce915b0d0129fd896) )

	ROM_REGION( 0x20000, "stfight_vid:fg_gfx", 0 )
	ROM_LOAD( "7.4c",   0x10000, 0x8000, CRC(2c6caa5f) SHA1(f6893cb87004979ead331897c684f995f850447e) )
	ROM_LOAD( "8.5c",   0x18000, 0x8000, CRC(e11ded31) SHA1(e3e634ad324d51e52d79dd79e5e6e5697cb8d21f) )
	ROM_LOAD( "5.2c",   0x00000, 0x8000, CRC(0c099a31) SHA1(dabaf8edc59e4954941cd8176031a358f45a1956) )
	ROM_LOAD( "6.3c",   0x08000, 0x8000, CRC(3cc77c31) SHA1(13d2324df5a322d499c9959a6bb3a844edaefb45) )

	ROM_REGION( 0x20000, "stfight_vid:bg_gfx", 0 )
	ROM_LOAD( "13.4c",   0x10000, 0x8000, CRC(0ae48dd3) SHA1(ca3d9aeb9f4343c379cef9282e408fbf8aa67d99) )
	ROM_LOAD( "14.5j",   0x18000, 0x8000, CRC(debf5d76) SHA1(eb18c35166eb5f93be98b3c30c7d909c0a68eada) )
	ROM_LOAD( "11.2j",   0x00000, 0x8000, CRC(8261ecfe) SHA1(5817f4a0458a949298414fe09c86bbcf50be52f3) )
	ROM_LOAD( "12.3j",   0x08000, 0x8000, CRC(71137301) SHA1(087a9f401939bc30f1dafa9916e8d8c564595a57) )

	ROM_REGION( 0x20000, "stfight_vid:spr_gfx", 0 )
	ROM_LOAD( "20.8w",   0x10000, 0x8000, CRC(8299f247) SHA1(71891f7b1fbfaed14c3854b7f6e10a3ddb4bd479) )
	ROM_LOAD( "21.9w",   0x18000, 0x8000, CRC(b57dc037) SHA1(69ac79a95ba9ace7c9ca7af480a4a10176be5ace) )
	ROM_LOAD( "18.6w",   0x00000, 0x8000, CRC(68acd627) SHA1(f98ff9ccb0913711079a2988e8dd08695fb5e107) )
	ROM_LOAD( "19.7w",   0x08000, 0x8000, CRC(5170a057) SHA1(9222f9febc222fa0c2eead258ad77c857f6d40c8) )

	ROM_REGION( 0x10000, "stfight_vid:fg_map", 0 )
	ROM_LOAD( "9.7c",   0x00000, 0x8000, CRC(8ceaf4fe) SHA1(5698f2ff44c109825b8d9d0b6dd2426624df668b) )
	ROM_LOAD( "10.8c",   0x08000, 0x8000, CRC(5a1a227a) SHA1(24928ab218824ae1f5380398ceb90dcad525cc08) )

	ROM_REGION( 0x10000, "stfight_vid:bg_map", 0 )
	ROM_LOAD( "15.7j",   0x00000, 0x8000, CRC(27a310bc) SHA1(dd30d72bc33b0bf7ddaf3ab730e028f51b20152a) )
	ROM_LOAD( "16.8j",   0x08000, 0x8000, CRC(3d19ce18) SHA1(38f691a23c96ef672637965c1a13f6d1595f9d51) )

	ROM_REGION( 0x0100, "stfight_vid:tx_clut", 0 )
	ROM_LOAD( "82s129.006", 0x0000, 0x0100, CRC(f9424b5b) SHA1(e3bc23213406d35d54f1221f17f25d433df273a2) )

	ROM_REGION( 0x0100, "stfight_vid:fg_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.002", 0x0000, 0x0100, CRC(c883d49b) SHA1(e84900ccf6f27e5043e43c0d85ea1e4eee7e52d3) )
	ROM_LOAD_NIB_LOW(  "82s129.003", 0x0000, 0x0100, CRC(af81882a) SHA1(b1008c991bd8d1157b3479e465ab286c70418b58) )

	ROM_REGION( 0x0100, "stfight_vid:bg_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.004", 0x0000, 0x0100, CRC(1831ce7c) SHA1(57afbee9225f0efd63895a5f522e96dc87ca2616) )
	ROM_LOAD_NIB_LOW(  "82s129.005", 0x0000, 0x0100, CRC(96cb6293) SHA1(1dcdeaa995e6ffa3753b742842c5ffe0f68ef8cd) )

	ROM_REGION( 0x0100, "stfight_vid:spr_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.052", 0x0000, 0x0100, CRC(3d915ffc) SHA1(921be6d5e5fc0fdee9c9f545c1c4a0c334e9844c) )
	ROM_LOAD_NIB_LOW(  "82s129.066", 0x0000, 0x0100, CRC(51e8832f) SHA1(ed8c00559e7a02bb8c11861d747c8c64c01b7437) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "82s129.015", 0x0700, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )  // timing? (not used)

	ROM_REGION( 0x08000, "adpcm", 0 )
	ROM_LOAD( "5j",   0x00000, 0x8000, CRC(1b8d0c07) SHA1(c163ccd2b7ed6c84facc075eb1564ca399f3ba17) )
ROM_END

/* not sure if this is a bootleg or not, it still displays the Seibu copyright on a screen during the attract mode
   but not during the initial startup, must investigate this set more later */
ROM_START( stfighta )
	ROM_REGION( 2*0x18000, "maincpu", 0 )
	ROM_LOAD( "sfight2.bin",     0x00000, 0x8000, CRC(8fb4dfc9) SHA1(0350f4a8749883a4e2e9c4aed2447a64a078f9ce) ) //  2.bin 58.532715%
	ROM_LOAD( "sfight1.bin",     0x10000, 0x8000, CRC(983ce746) SHA1(3c7b9498f1adf253ba651558ee40641ec3dbc5eb) ) // bank switched //  a-1.4q 99.737549%

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sf03.bin",   0x0000,  0x8000, CRC(6a8cb7a6) SHA1(dc123cc48d3623752b78e7c23dd8d2f5adf84f92) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "_68705.3j",  0x0000,  0x0800, BAD_DUMP CRC(f4cc50d6) SHA1(2ff62a349b74fa965b5d19615e52b867c04988dc) )

	ROM_REGION( 0x02000, "stfight_vid:tx_gfx", 0 )
	ROM_LOAD( "17.2n",   0x0000, 0x2000, CRC(1b3706b5) SHA1(61f069329a7a836523ffc8cce915b0d0129fd896) )

	ROM_REGION( 0x20000, "stfight_vid:fg_gfx", 0 )
	ROM_LOAD( "7.4c",   0x10000, 0x8000, CRC(2c6caa5f) SHA1(f6893cb87004979ead331897c684f995f850447e) )
	ROM_LOAD( "8.5c",   0x18000, 0x8000, CRC(e11ded31) SHA1(e3e634ad324d51e52d79dd79e5e6e5697cb8d21f) )
	ROM_LOAD( "5.2c",   0x00000, 0x8000, CRC(0c099a31) SHA1(dabaf8edc59e4954941cd8176031a358f45a1956) )
	ROM_LOAD( "6.3c",   0x08000, 0x8000, CRC(3cc77c31) SHA1(13d2324df5a322d499c9959a6bb3a844edaefb45) )

	ROM_REGION( 0x20000, "stfight_vid:bg_gfx", 0 )
	ROM_LOAD( "13.4c",   0x10000, 0x8000, CRC(0ae48dd3) SHA1(ca3d9aeb9f4343c379cef9282e408fbf8aa67d99) )
	ROM_LOAD( "14.5j",   0x18000, 0x8000, CRC(debf5d76) SHA1(eb18c35166eb5f93be98b3c30c7d909c0a68eada) )
	ROM_LOAD( "11.2j",   0x00000, 0x8000, CRC(8261ecfe) SHA1(5817f4a0458a949298414fe09c86bbcf50be52f3) )
	ROM_LOAD( "12.3j",   0x08000, 0x8000, CRC(71137301) SHA1(087a9f401939bc30f1dafa9916e8d8c564595a57) )

	ROM_REGION( 0x20000, "stfight_vid:spr_gfx", 0 )
	ROM_LOAD( "20.8w",   0x10000, 0x8000, CRC(8299f247) SHA1(71891f7b1fbfaed14c3854b7f6e10a3ddb4bd479) )
	ROM_LOAD( "21.9w",   0x18000, 0x8000, CRC(b57dc037) SHA1(69ac79a95ba9ace7c9ca7af480a4a10176be5ace) )
	ROM_LOAD( "18.6w",   0x00000, 0x8000, CRC(68acd627) SHA1(f98ff9ccb0913711079a2988e8dd08695fb5e107) )
	ROM_LOAD( "19.7w",   0x08000, 0x8000, CRC(5170a057) SHA1(9222f9febc222fa0c2eead258ad77c857f6d40c8) )

	ROM_REGION( 0x10000, "stfight_vid:fg_map", 0 )
	ROM_LOAD( "9.7c",   0x00000, 0x8000, CRC(8ceaf4fe) SHA1(5698f2ff44c109825b8d9d0b6dd2426624df668b) )
	ROM_LOAD( "10.8c",   0x08000, 0x8000, CRC(5a1a227a) SHA1(24928ab218824ae1f5380398ceb90dcad525cc08) )

	ROM_REGION( 0x10000, "stfight_vid:bg_map", 0 )
	ROM_LOAD( "15.7j",   0x00000, 0x8000, CRC(27a310bc) SHA1(dd30d72bc33b0bf7ddaf3ab730e028f51b20152a) )
	ROM_LOAD( "16.8j",   0x08000, 0x8000, CRC(3d19ce18) SHA1(38f691a23c96ef672637965c1a13f6d1595f9d51) )

	ROM_REGION( 0x0100, "stfight_vid:tx_clut", 0 )
	ROM_LOAD( "82s129.006", 0x0000, 0x0100, CRC(f9424b5b) SHA1(e3bc23213406d35d54f1221f17f25d433df273a2) )

	ROM_REGION( 0x0100, "stfight_vid:fg_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.002", 0x0000, 0x0100, CRC(c883d49b) SHA1(e84900ccf6f27e5043e43c0d85ea1e4eee7e52d3) )
	ROM_LOAD_NIB_LOW(  "82s129.003", 0x0000, 0x0100, CRC(af81882a) SHA1(b1008c991bd8d1157b3479e465ab286c70418b58) )

	ROM_REGION( 0x0100, "stfight_vid:bg_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.004", 0x0000, 0x0100, CRC(1831ce7c) SHA1(57afbee9225f0efd63895a5f522e96dc87ca2616) )
	ROM_LOAD_NIB_LOW(  "82s129.005", 0x0000, 0x0100, CRC(96cb6293) SHA1(1dcdeaa995e6ffa3753b742842c5ffe0f68ef8cd) )

	ROM_REGION( 0x0100, "stfight_vid:spr_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.052", 0x0000, 0x0100, CRC(3d915ffc) SHA1(921be6d5e5fc0fdee9c9f545c1c4a0c334e9844c) )
	ROM_LOAD_NIB_LOW(  "82s129.066", 0x0000, 0x0100, CRC(51e8832f) SHA1(ed8c00559e7a02bb8c11861d747c8c64c01b7437) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "82s129.015", 0x0700, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )  // timing? (not used)

	ROM_REGION( 0x08000, "adpcm", 0 )
	ROM_LOAD( "5j",   0x00000, 0x8000, CRC(1b8d0c07) SHA1(c163ccd2b7ed6c84facc075eb1564ca399f3ba17) )
ROM_END

ROM_START( empcityi ) // very similar to above set
	ROM_REGION( 2*0x18000, "maincpu", 0 )
	ROM_LOAD( "1.bin",     0x00000, 0x8000, CRC(32378e47) SHA1(1194e5a6b77ee754450ce532e048a55cf48d416c) )
	ROM_LOAD( "2.bin",     0x10000, 0x8000, CRC(d20010c6) SHA1(8f30b385cbe733a4256461ab6f4aa82bc6694a6e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sf03.bin",   0x0000,  0x8000, CRC(6a8cb7a6) SHA1(dc123cc48d3623752b78e7c23dd8d2f5adf84f92) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "empcityi_68705.3j",  0x0000,  0x0800, BAD_DUMP CRC(b1817d44) SHA1(395aad763eb054514f658a14c12b92c1b90c02ce) )

	ROM_REGION( 0x02000, "stfight_vid:tx_gfx", 0 )
	ROM_LOAD( "17.2n",   0x0000, 0x2000, CRC(1b3706b5) SHA1(61f069329a7a836523ffc8cce915b0d0129fd896) )

	ROM_REGION( 0x20000, "stfight_vid:fg_gfx", 0 )
	ROM_LOAD( "7.4c",   0x10000, 0x8000, CRC(2c6caa5f) SHA1(f6893cb87004979ead331897c684f995f850447e) )
	ROM_LOAD( "8.5c",   0x18000, 0x8000, CRC(e11ded31) SHA1(e3e634ad324d51e52d79dd79e5e6e5697cb8d21f) )
	ROM_LOAD( "5.2c",   0x00000, 0x8000, CRC(0c099a31) SHA1(dabaf8edc59e4954941cd8176031a358f45a1956) )
	ROM_LOAD( "6.3c",   0x08000, 0x8000, CRC(3cc77c31) SHA1(13d2324df5a322d499c9959a6bb3a844edaefb45) )

	ROM_REGION( 0x20000, "stfight_vid:bg_gfx", 0 )
	ROM_LOAD( "13.4c",   0x10000, 0x8000, CRC(0ae48dd3) SHA1(ca3d9aeb9f4343c379cef9282e408fbf8aa67d99) )
	ROM_LOAD( "14.5j",   0x18000, 0x8000, CRC(debf5d76) SHA1(eb18c35166eb5f93be98b3c30c7d909c0a68eada) )
	ROM_LOAD( "11.2j",   0x00000, 0x8000, CRC(8261ecfe) SHA1(5817f4a0458a949298414fe09c86bbcf50be52f3) )
	ROM_LOAD( "12.3j",   0x08000, 0x8000, CRC(71137301) SHA1(087a9f401939bc30f1dafa9916e8d8c564595a57) )

	ROM_REGION( 0x20000, "stfight_vid:spr_gfx", 0 )
	ROM_LOAD( "20.8w",   0x10000, 0x8000, CRC(8299f247) SHA1(71891f7b1fbfaed14c3854b7f6e10a3ddb4bd479) )
	ROM_LOAD( "21.9w",   0x18000, 0x8000, CRC(b57dc037) SHA1(69ac79a95ba9ace7c9ca7af480a4a10176be5ace) )
	ROM_LOAD( "18.6w",   0x00000, 0x8000, CRC(68acd627) SHA1(f98ff9ccb0913711079a2988e8dd08695fb5e107) )
	ROM_LOAD( "19.7w",   0x08000, 0x8000, CRC(5170a057) SHA1(9222f9febc222fa0c2eead258ad77c857f6d40c8) )

	ROM_REGION( 0x10000, "stfight_vid:fg_map", 0 )
	ROM_LOAD( "9.7c",    0x00000, 0x8000, CRC(8ceaf4fe) SHA1(5698f2ff44c109825b8d9d0b6dd2426624df668b) )
	ROM_LOAD( "10.8c",   0x08000, 0x8000, CRC(5a1a227a) SHA1(24928ab218824ae1f5380398ceb90dcad525cc08) )

	ROM_REGION( 0x10000, "stfight_vid:bg_map", 0 )
	ROM_LOAD( "15.7j",   0x00000, 0x8000, CRC(27a310bc) SHA1(dd30d72bc33b0bf7ddaf3ab730e028f51b20152a) )
	ROM_LOAD( "16.8j",   0x08000, 0x8000, CRC(3d19ce18) SHA1(38f691a23c96ef672637965c1a13f6d1595f9d51) )

	ROM_REGION( 0x0100, "stfight_vid:tx_clut", 0 )
	ROM_LOAD( "82s129.006", 0x0000, 0x0100, CRC(f9424b5b) SHA1(e3bc23213406d35d54f1221f17f25d433df273a2) )

	ROM_REGION( 0x0100, "stfight_vid:fg_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.002", 0x0000, 0x0100, CRC(c883d49b) SHA1(e84900ccf6f27e5043e43c0d85ea1e4eee7e52d3) )
	ROM_LOAD_NIB_LOW(  "82s129.003", 0x0000, 0x0100, CRC(af81882a) SHA1(b1008c991bd8d1157b3479e465ab286c70418b58) )

	ROM_REGION( 0x0100, "stfight_vid:bg_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.004", 0x0000, 0x0100, CRC(1831ce7c) SHA1(57afbee9225f0efd63895a5f522e96dc87ca2616) )
	ROM_LOAD_NIB_LOW(  "82s129.005", 0x0000, 0x0100, CRC(96cb6293) SHA1(1dcdeaa995e6ffa3753b742842c5ffe0f68ef8cd) )

	ROM_REGION( 0x0100, "stfight_vid:spr_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.052", 0x0000, 0x0100, CRC(3d915ffc) SHA1(921be6d5e5fc0fdee9c9f545c1c4a0c334e9844c) )
	ROM_LOAD_NIB_LOW(  "82s129.066", 0x0000, 0x0100, CRC(51e8832f) SHA1(ed8c00559e7a02bb8c11861d747c8c64c01b7437) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "82s129.015", 0x0700, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )  // timing? (not used)

	ROM_REGION( 0x08000, "adpcm", 0 )
	ROM_LOAD( "5j",   0x00000, 0x8000, CRC(1b8d0c07) SHA1(c163ccd2b7ed6c84facc075eb1564ca399f3ba17) )
ROM_END

ROM_START( empcityfr )
	ROM_REGION( 2*0x18000, "maincpu", 0 )
	ROM_LOAD( "pr.4t",     0x00000, 0x8000, CRC(aa1f84ac) SHA1(b484b85270091511860f5b4041099c5335ff1204) )
	ROM_LOAD( "pr.2t",     0x10000, 0x8000, CRC(af381247) SHA1(93812d6b6a2dead07670b789597d23f29b8f0c5d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "092-5c",   0x0000,  0x8000, CRC(6a8cb7a6) SHA1(dc123cc48d3623752b78e7c23dd8d2f5adf84f92) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "empcityfr_68705.3j",  0x0000,  0x0800, BAD_DUMP CRC(d66ac61f) SHA1(5f44d69886d4db46f2e4c07ebdf01e337ee4fd35) )

	ROM_REGION( 0x02000, "stfight_vid:tx_gfx", 0 )
	ROM_LOAD( "17.2n",   0x0000, 0x2000, CRC(1b3706b5) SHA1(61f069329a7a836523ffc8cce915b0d0129fd896) )

	ROM_REGION( 0x20000, "stfight_vid:fg_gfx", 0 )
	ROM_LOAD( "115.4c",   0x10000, 0x8000, CRC(2c6caa5f) SHA1(f6893cb87004979ead331897c684f995f850447e) )
	ROM_LOAD( "116.5c",   0x18000, 0x8000, CRC(e11ded31) SHA1(e3e634ad324d51e52d79dd79e5e6e5697cb8d21f) )
	ROM_LOAD( "113.2c",   0x00000, 0x8000, CRC(0c099a31) SHA1(dabaf8edc59e4954941cd8176031a358f45a1956) )
	ROM_LOAD( "114.3c",   0x08000, 0x8000, CRC(3cc77c31) SHA1(13d2324df5a322d499c9959a6bb3a844edaefb45) )

	ROM_REGION( 0x20000, "stfight_vid:bg_gfx", 0 )
	ROM_LOAD( "109.4j",   0x10000, 0x8000, CRC(0ae48dd3) SHA1(ca3d9aeb9f4343c379cef9282e408fbf8aa67d99) )
	ROM_LOAD( "110.5j",   0x18000, 0x8000, CRC(debf5d76) SHA1(eb18c35166eb5f93be98b3c30c7d909c0a68eada) )
	ROM_LOAD( "097.2j",   0x00000, 0x8000, CRC(8261ecfe) SHA1(5817f4a0458a949298414fe09c86bbcf50be52f3) )
	ROM_LOAD( "108.3j",   0x08000, 0x8000, CRC(71137301) SHA1(087a9f401939bc30f1dafa9916e8d8c564595a57) )

	ROM_REGION( 0x20000, "stfight_vid:spr_gfx", 0 )
	ROM_LOAD( "095.8w",   0x10000, 0x8000, CRC(8299f247) SHA1(71891f7b1fbfaed14c3854b7f6e10a3ddb4bd479) )
	ROM_LOAD( "096.9w",   0x18000, 0x8000, CRC(b57dc037) SHA1(69ac79a95ba9ace7c9ca7af480a4a10176be5ace) )
	ROM_LOAD( "093.6w",   0x00000, 0x8000, CRC(68acd627) SHA1(f98ff9ccb0913711079a2988e8dd08695fb5e107) )
	ROM_LOAD( "094.7w",   0x08000, 0x8000, CRC(5170a057) SHA1(9222f9febc222fa0c2eead258ad77c857f6d40c8) )

	ROM_REGION( 0x10000, "stfight_vid:fg_map", 0 )
	ROM_LOAD( "117.7c",   0x00000, 0x8000, CRC(8ceaf4fe) SHA1(5698f2ff44c109825b8d9d0b6dd2426624df668b) )
	ROM_LOAD( "118.8c",   0x08000, 0x8000, CRC(5a1a227a) SHA1(24928ab218824ae1f5380398ceb90dcad525cc08) )

	ROM_REGION( 0x10000, "stfight_vid:bg_map", 0 )
	ROM_LOAD( "111.7j",   0x00000, 0x8000, CRC(27a310bc) SHA1(dd30d72bc33b0bf7ddaf3ab730e028f51b20152a) )
	ROM_LOAD( "112.8j",   0x08000, 0x8000, CRC(3d19ce18) SHA1(38f691a23c96ef672637965c1a13f6d1595f9d51) )

	ROM_REGION( 0x0100, "stfight_vid:tx_clut", 0 )
	ROM_LOAD( "82s129.006", 0x0000, 0x0100, CRC(f9424b5b) SHA1(e3bc23213406d35d54f1221f17f25d433df273a2) )

	ROM_REGION( 0x0100, "stfight_vid:fg_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.002", 0x0000, 0x0100, CRC(c883d49b) SHA1(e84900ccf6f27e5043e43c0d85ea1e4eee7e52d3) )
	ROM_LOAD_NIB_LOW(  "82s129.003", 0x0000, 0x0100, CRC(af81882a) SHA1(b1008c991bd8d1157b3479e465ab286c70418b58) )

	ROM_REGION( 0x0100, "stfight_vid:bg_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.004", 0x0000, 0x0100, CRC(1831ce7c) SHA1(57afbee9225f0efd63895a5f522e96dc87ca2616) )
	ROM_LOAD_NIB_LOW(  "82s129.005", 0x0000, 0x0100, CRC(96cb6293) SHA1(1dcdeaa995e6ffa3753b742842c5ffe0f68ef8cd) )

	ROM_REGION( 0x0100, "stfight_vid:spr_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.052", 0x0000, 0x0100, CRC(3d915ffc) SHA1(921be6d5e5fc0fdee9c9f545c1c4a0c334e9844c) )
	ROM_LOAD_NIB_LOW(  "82s129.066", 0x0000, 0x0100, CRC(51e8832f) SHA1(ed8c00559e7a02bb8c11861d747c8c64c01b7437) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "82s129.015", 0x0700, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )  // timing? (not used)

	ROM_REGION( 0x08000, "adpcm", 0 )
	ROM_LOAD( "091.5j",   0x00000, 0x8000, CRC(1b8d0c07) SHA1(c163ccd2b7ed6c84facc075eb1564ca399f3ba17) )
ROM_END

ROM_START( stfightgb )
	ROM_REGION( 2*0x18000, "maincpu", 0 )
	ROM_LOAD( "1.4t",  0x00000, 0x8000, CRC(0ce5ca11) SHA1(f753107a0c4ce52fe761ea2edce4c5e96169dfbd) )
	ROM_LOAD( "2.2t",  0x10000, 0x8000, CRC(936ba873) SHA1(bab519c587692a44a9cd1e9af2aeb7e3347c3f1b) ) // bank switched

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c5",  0x0000,  0x8000, CRC(6a8cb7a6) SHA1(dc123cc48d3623752b78e7c23dd8d2f5adf84f92) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "stfightgb_68705.3j",  0x0000,  0x0800, BAD_DUMP  CRC(3b1b2660) SHA1(8d5d853a0861ff9cdea27eb3588586b441cc77b1) ) //hand-crafted, to be dumped

	ROM_REGION( 0x02000, "stfight_vid:tx_gfx", 0 )
	ROM_LOAD( "17.2n",  0x0000, 0x2000, CRC(1b3706b5) SHA1(61f069329a7a836523ffc8cce915b0d0129fd896) )

	ROM_REGION( 0x20000, "stfight_vid:fg_gfx", 0 )
	ROM_LOAD( "7.4c",   0x10000, 0x8000, CRC(2c6caa5f) SHA1(f6893cb87004979ead331897c684f995f850447e) )
	ROM_LOAD( "8.5c",   0x18000, 0x8000, CRC(e11ded31) SHA1(e3e634ad324d51e52d79dd79e5e6e5697cb8d21f) )
	ROM_LOAD( "5.2c",   0x00000, 0x8000, CRC(0c099a31) SHA1(dabaf8edc59e4954941cd8176031a358f45a1956) )
	ROM_LOAD( "6.3c",   0x08000, 0x8000, CRC(3cc77c31) SHA1(13d2324df5a322d499c9959a6bb3a844edaefb45) )

	ROM_REGION( 0x20000, "stfight_vid:bg_gfx", 0 )
	ROM_LOAD( "13.4c",   0x10000, 0x8000, CRC(0ae48dd3) SHA1(ca3d9aeb9f4343c379cef9282e408fbf8aa67d99) )
	ROM_LOAD( "14.5j",   0x18000, 0x8000, CRC(debf5d76) SHA1(eb18c35166eb5f93be98b3c30c7d909c0a68eada) )
	ROM_LOAD( "11.2j",   0x00000, 0x8000, CRC(8261ecfe) SHA1(5817f4a0458a949298414fe09c86bbcf50be52f3) )
	ROM_LOAD( "12.3j",   0x08000, 0x8000, CRC(71137301) SHA1(087a9f401939bc30f1dafa9916e8d8c564595a57) )

	ROM_REGION( 0x20000, "stfight_vid:spr_gfx", 0 )
	ROM_LOAD( "20.8w",   0x10000, 0x8000, CRC(8299f247) SHA1(71891f7b1fbfaed14c3854b7f6e10a3ddb4bd479) )
	ROM_LOAD( "21.9w",   0x18000, 0x8000, CRC(b57dc037) SHA1(69ac79a95ba9ace7c9ca7af480a4a10176be5ace) )
	ROM_LOAD( "18.6w",   0x00000, 0x8000, CRC(68acd627) SHA1(f98ff9ccb0913711079a2988e8dd08695fb5e107) )
	ROM_LOAD( "19.7w",   0x08000, 0x8000, CRC(5170a057) SHA1(9222f9febc222fa0c2eead258ad77c857f6d40c8) )

	ROM_REGION( 0x10000, "stfight_vid:fg_map", 0 )
	ROM_LOAD( "9.7c",    0x00000, 0x8000, CRC(8ceaf4fe) SHA1(5698f2ff44c109825b8d9d0b6dd2426624df668b) )
	ROM_LOAD( "10.8c",   0x08000, 0x8000, CRC(5a1a227a) SHA1(24928ab218824ae1f5380398ceb90dcad525cc08) )

	ROM_REGION( 0x10000, "stfight_vid:bg_map", 0 )
	ROM_LOAD( "15.7j",   0x00000, 0x8000, CRC(27a310bc) SHA1(dd30d72bc33b0bf7ddaf3ab730e028f51b20152a) )
	ROM_LOAD( "16.8j",   0x08000, 0x8000, CRC(3d19ce18) SHA1(38f691a23c96ef672637965c1a13f6d1595f9d51) )

	ROM_REGION( 0x0100, "stfight_vid:tx_clut", 0 )
	ROM_LOAD( "82s129.006", 0x0000, 0x0100, CRC(f9424b5b) SHA1(e3bc23213406d35d54f1221f17f25d433df273a2) )

	ROM_REGION( 0x0100, "stfight_vid:fg_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.002", 0x0000, 0x0100, CRC(c883d49b) SHA1(e84900ccf6f27e5043e43c0d85ea1e4eee7e52d3) )
	ROM_LOAD_NIB_LOW(  "82s129.003", 0x0000, 0x0100, CRC(af81882a) SHA1(b1008c991bd8d1157b3479e465ab286c70418b58) )

	ROM_REGION( 0x0100, "stfight_vid:bg_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.004", 0x0000, 0x0100, CRC(1831ce7c) SHA1(57afbee9225f0efd63895a5f522e96dc87ca2616) )
	ROM_LOAD_NIB_LOW(  "82s129.005", 0x0000, 0x0100, CRC(96cb6293) SHA1(1dcdeaa995e6ffa3753b742842c5ffe0f68ef8cd) )

	ROM_REGION( 0x0100, "stfight_vid:spr_clut", 0 )
	ROM_LOAD_NIB_HIGH( "82s129.052", 0x0000, 0x0100, CRC(3d915ffc) SHA1(921be6d5e5fc0fdee9c9f545c1c4a0c334e9844c) )
	ROM_LOAD_NIB_LOW(  "82s129.066", 0x0000, 0x0100, CRC(51e8832f) SHA1(ed8c00559e7a02bb8c11861d747c8c64c01b7437) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "82s129.015", 0x0700, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )  // timing? (not used)

	ROM_REGION( 0x08000, "adpcm", 0 )
	ROM_LOAD( "5j",   0x00000, 0x8000, CRC(1b8d0c07) SHA1(c163ccd2b7ed6c84facc075eb1564ca399f3ba17) )
ROM_END

/*

-----------------------------
Cross Shooter by TAITO (1987)
-----------------------------
malcor


Location    Type     File ID   Checksum
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LB 4U       27256      R1        C2F0   [  main program  ]
LB 2U       27512      R2        74EA   [    tilemaps    ]
TB 11A      2764       R3        DFA7   [      fix       ]
LB 5C       27256      R4        D7B8   [ sound program  ]
LB 7A       82S123     0.BPR     00A1   [  foregrounds   ]
LB 9S       82S129     1.BPR     0194   [ motion objects ]
TB 4E       82S129     2.BPR     00DC   [ motion objects ]
TB 16A      63S281     x         x      [     clut       ] NOTE: dumped much later
LB 3J       68705

         The 0/1/2 bipolar PROMs are not used for colour.

         However, this contradicts Guru's findings: "If I short some of the pins(of 0.bpr at 7A)
         the sprite colors change, and the chip is connected to the color RAM."


Brief hardware overview:
------------------------

Main processor  - Z80 6MHz
                - 68705

GFX             - custom TC15G008AP-0048  SEI0040BU    - Toshiba CMOS Gate Array
            3 x - custom TC17G008AN-0015  SEI0020BU    - Toshiba CMOS Gate Array
                - custom TC17G005AN-0028  SEI0030BU    - Toshiba CMOS Gate Array
            3 x - custom SIPs. No ID, unusually large. - covered in epoxy, probably sprite/tile gfx data is in here

Sound processor - Z80 6MHz (5.897MHz)
            2 x - YM2203C


See airraid.cpp for notes about custom modules

*/

ROM_START( cshootert )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "r1.4u",   0x00000, 0x08000, CRC(fbe8c518) SHA1(bff8319f4892e6d06f1c7a679f67dc8407279cfa) )
	ROM_LOAD( "r2.2u",   0x10000, 0x10000, CRC(5ddf9f4e) SHA1(69e4d422ca272bf2e9f00edbe7d23760485fdfe6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "r4.5c",   0x00000, 0x08000, CRC(84fed017) SHA1(9a564c9379eb48569cfba48562889277991864d8) )

	ROM_REGION( 0x08000, "adpcm", ROMREGION_ERASEFF )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "crshooter.3j", 0x0000, 0x0800, CRC(aae61ce7) SHA1(bb2b9887ec73a5b82604b9b64c533c2242d20d0f) )

	ROM_REGION( 0x820, "proms", 0 )
	ROM_LOAD( "82s129.9s",  0x0500, 0x0100, CRC(cf14ba30) SHA1(3284b6809075756b3c8e07d9705fc7eacb7556f1) ) // timing? (not used)
	ROM_LOAD( "82s129.4e",  0x0600, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) ) // timing? (not used)
	ROM_LOAD( "82s123.7a",  0x0800, 0x0020, CRC(93e2d292) SHA1(af8edd0cfe85f28ede9604cfaf4516d54e5277c9) ) // ? (not used)

	// below are from the video board

	ROM_REGION( 0x02000, "airraid_vid:tx_gfx", 0 )
	ROM_LOAD( "r3.11a",  0x00000, 0x02000, CRC(67b50a47) SHA1(b1f4aefc9437edbeefba5371149cc08c0b55c741) )

	ROM_REGION( 0x100, "airraid_vid:tx_clut", 0 )
	ROM_LOAD( "63s281.16a", 0x0000, 0x0100, CRC(0b8b914b) SHA1(8cf4910b846de79661cc187887171ed8ebfd6719) )

	// ### MODULE 1 ### Background generation / graphics
	ROM_REGION( 0x40000, "airraid_vid:bg_map", 0 )
	ROM_LOAD16_BYTE( "bg_layouts_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "bg_layouts_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x40000, "airraid_vid:bg_gfx", 0 )
	ROM_LOAD16_BYTE( "bg_tiles_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "bg_tiles_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x100, "airraid_vid:bg_clut", 0 )
	ROM_LOAD( "bg_clut",   0x000, 0x100, NO_DUMP )

	// ### MODULE 2 ### Foreground generation / graphics
	ROM_REGION( 0x40000, "airraid_vid:fg_map", 0 )
	ROM_LOAD16_BYTE( "fg_layouts_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "fg_layouts_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x40000, "airraid_vid:fg_gfx", 0 )
	ROM_LOAD16_BYTE( "fg_tiles_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "fg_tiles_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x100, "airraid_vid:fg_clut", 0 )
	ROM_LOAD( "fg_clut",   0x000, 0x100, NO_DUMP )

	// ### MODULE 3 ### Sprite graphics
	ROM_REGION( 0x40000, "airraid_vid:spr_gfx", 0 )
	ROM_LOAD16_BYTE( "sprite_tiles_even",   0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "sprite_tiles_odd",    0x00001, 0x20000, NO_DUMP )
	ROM_REGION( 0x100, "airraid_vid:spr_clut", 0 )
	ROM_LOAD( "spr_clut",   0x000, 0x100, NO_DUMP )
ROM_END


/*

Encryption PAL 16R4 on CPU board

          +---U---+
     CP --|       |-- VCC
 ROM D1 --|       |-- ROM D0          M1 = 0                M1 = 1
 ROM D3 --|       |-- (NC)
 ROM D4 --|       |-- D6         D6 = D1 ^^ D3          D6 = / ( D1 ^^ D0 )
 ROM D6 --|       |-- D4         D4 = / ( D6 ^^ A7 )    D4 = D3 ^^ A0
     A0 --|       |-- D3         D3 = / ( D0 ^^ A1 )    D3 = D4 ^^ A4
     A1 --|       |-- D0         D0 = D1 ^^ D4          D0 = / ( D6 ^^ A0 )
     A4 --|       |-- (NC)
     A7 --|       |-- /M1
    GND --|       |-- /OE
          +-------+

*/


void stfight_state::init_stfight()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (uint32_t A = 0; A < 0x8000; ++A)
	{
		uint8_t src = rom[A];

		// decode opcode
		m_decrypted_opcodes[A] =
				( src & 0xa6 ) |
				( ( ( ( src << 2 ) ^ src ) << 3 ) & 0x40 ) |
				( ~( ( src ^ ( A >> 1 ) ) >> 2 ) & 0x10 ) |
				( ~( ( ( src << 1 ) ^ A ) << 2 ) & 0x08 ) |
				( ( ( src ^ ( src >> 3 ) ) >> 1 ) & 0x01 );

		// decode operand
		rom[A] =
				( src & 0xa6 ) |
				( ~( ( src ^ ( src << 1 ) ) << 5 ) & 0x40 ) |
				( ( ( src ^ ( A << 3 ) ) << 1 ) & 0x10 ) |
				( ( ( src ^ A ) >> 1 ) & 0x08 ) |
				( ~( ( src >> 6 ) ^ A ) & 0x01 );
	}

	// Set clock prescaler FM:1/2 PSG:1/1
	m_ym[0]->write(0, 0x2f);
	m_ym[1]->write(0, 0x2f);
}

} // anonymous namespace


// Note: Marked MACHINE_IMPERFECT_SOUND due to YM2203 clock issue
GAME( 1986, empcity,   0,       stfight,  stfight,  stfight_state, init_stfight,  ROT0,   "Seibu Kaihatsu",                           "Empire City: 1931 (bootleg?)",     MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1986, empcityu,  empcity, stfight,  stfight,  stfight_state, init_stfight,  ROT0,   "Seibu Kaihatsu (Taito / Romstar license)", "Empire City: 1931 (US)",           MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // different title logo
GAME( 1986, empcityj,  empcity, stfight,  stfight,  stfight_state, init_stfight,  ROT0,   "Seibu Kaihatsu (Taito license)",           "Empire City: 1931 (Japan)",        MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1986, empcityi,  empcity, stfight,  stfight,  stfight_state, init_stfight,  ROT0,   "Seibu Kaihatsu (Eurobed license)",         "Empire City: 1931 (Italy)",        MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1986, empcityfr, empcity, stfight,  stfight,  stfight_state, init_stfight,  ROT0,   "Seibu Kaihatsu (Norad license)",           "Empire City: 1931 (France)",       MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1986, stfight,   empcity, stfight,  stfight,  stfight_state, init_stfight,  ROT0,   "Seibu Kaihatsu (Tuning license)",          "Street Fight (Germany)",           MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1986, stfighta,  empcity, stfight,  stfight,  stfight_state, init_stfight,  ROT0,   "Seibu Kaihatsu",                           "Street Fight (bootleg?)",          MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1986, stfightgb, empcity, stfight,  stfight,  stfight_state, init_stfight,  ROT0,   "Seibu Kaihatsu (Tuning license)",          "Street Fight (Germany - Benelux)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

// Cross Shooter uses the same base board, but different video board
GAME( 1987, cshootert, airraid, cshooter, cshooter, stfight_state, empty_init,    ROT270, "Seibu Kaihatsu (Taito license)",           "Cross Shooter (2 PCB Stack)",      MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
