// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

KiKi KaiKai - (c) 1987 Taito
    + Knight Boy (bootleg with 68705)

Kick and Run - (c) 1987 Taito
    + Mexico 86 (bootleg with 68705)

Ernesto Corvi
ernesto@imagina.com

Notes:
- master/slave 4 players mode is not emulated at all.
  To set it up, enable the Master/Slave Mode and set the two boards IDs with
  different settings. Do NOT enable the Single board 4p mode, I don't think
  that the main board is supposed to be still connected to the sub board.

- Single board 4 players mode actually works but I'm not sure how the reset /
  halt line is truly connected on the sub CPU.
  To set it up, just enable the single board 4p mode and keep the master/slave
  mode to off and the board ID to master.

- mexico86 does a PS4 STOP ERROR shortly after boot, but works afterwards. PS4 is
  the MC6801U4 MCU, the bootleggers replaced it with a custom programmed 68705 MCU.

- Kick and Run is a rom swap for Kiki KaiKai as the pal chips are all A85-0x
  A85 is the Taito rom code for Kiki KaiKai.  Even the MCU is socketed!

Note MCU labeling:

Bubble Bobble   KiKi KaiKai      Kick and Run
-------------   -------------    -------------
TAITO A78-01    TAITO A85-01     TAITO A87-01
JPH1011P        JPH1020P         JPH1021P
185             185              185
PS4  J8635      PS4  J8541       PS4  J8648


***************************************************************************
Kiki KaiKai, Taito, 1986
Hardware info by Guru
Last Update: 22nd August 2021


PCB Layout
----------

MAIN BOARD K1100195A J1100079A
Sticker: K1100195A KIKI KAIKAI
|-------------------------------------------------------------------|
|                 SWA   SWB             TMM2064            A85_15.A1|
| MB3731  YM3014  YM2203C                       A85-04.A2  A85_14.A3|
|         4556                          TMM2064            A85_13.A5|
|         VOL     Z0840006PSC                              A85_12.A6|
|     CN1            A85_11.F6                                      |
| PC050CM            CXK5816                           A85-03.B7    |
|A85_01.H8                                                         |--|
|                                                                  |  |
|                                                                  |  |
|J                                                                 |  |
|A                                 A85_05.D10                      |  |
|M                                                                 |  |E
|M             A85_07.G11                                  CXK5816 |  |
|A                                                                 |  |
|              A85-10.G12   MCM2016                                |  |
|              *   *   *                                           |  |
|              A85-09.G14                                          |--|
|              A85-08.G15   MCM2016                                 |
|                                                         A85-02.A13|
|              A85-06.G16                                           |
|                                                                   |
|   A85_17.H16                                                      |
|   A85_16.H18     Z0840006PSC      MB3771                  24MHz   |
|-------------------------------------------------------------------|
Notes:
           E - 50 pin flat cable connector labelled 'E' (unused)
   A85-01.H8 - Motorola MC6801U4 8-bit microcontroller with 192 Bytes internal RAM and 4kB internal mask ROM marked 'TAITO A85-01 JPH1020P 185 PS4 J8641'
               Clock input 3.000MHz on pin 3 [24/8]
         CN1 - 4-pin header labelled 'JAMMA' with a 4-pin plug marked 'H' connected on top.
               Pin 1 is tied to ground.
               Pin 2 is tied to the PC050CM on pin 10 (schematics show pin 10 is tied to ground).
               Each of the pins 3 and 4 connect to the JAMMA 12V pins separately (JAMMA 12V is not tied together on the PCB).
               On the 'H' plug, pins 1-2 are tied with a wire and pin 3-4 are tied with a wire.
      MB3731 - Fujitsu MB3731 18W BTL mono power amplifier
      MB3771 - Fujitsu MB3771 power supply monitor IC. This is used to provide the power-on reset.
     SWA/SWB - 8-position DIP switch
     TMM2064 - Toshiba TMM2064 8kx8-bit static RAM
     MCM2016 - Motorola MCM2016 2kx8-bit static RAM
     CXK5816 - Sony CXK5816 2kx8-bit static RAM
        4556 - 4556 dual operational amplifier
         VOL - 5k volume pot
     PC050CM - Taito PC050CM custom SIL28 ceramic module for coins, coin lockout and coin counters
      YM3014 - Yamaha YM3014 serial input floating D/A converter. Clock input 1.000MHz on pin 5 [24/24]. Other clocks: pin 3= 41.6666kHz, pin 4= 83.3332kHz
     YM2203C - Yamaha YM2203C OPN (FM Operator Type-N) 3-channel sound chip. Clock input 3.000MHz [24/8]
 Z0840006PSC - Zilog Z0840006PSC Z80 CPU. Clock input 6.000MHz [24/4] (for both Z80's)
       HSync - 15.1436kHz
       VSync - 59.1858Hz
  A85-02.A13 - MMI PAL16L8A marked 'A85-02'
   A85-03.B7 - MMI PAL16L8A marked 'A85-03'
   A85-04.A2 - MMI PAL16L8A marked 'A85-04'
  A85-05.D10 - MMI PAL16L8A marked 'A85-05'
  A85-06.G16 - MMI PAL16L8A marked 'A85-06'
  A85-07.G11 - MMI PAL16R4A marked 'A85-07'
      A85_11 - 27C256 EPROM
A85_12 to 17 - 27C512 EPROM
  A85-08.G15 - 63S141 bipolar PROM (Red)
  A85-09.G14 - 63S141 bipolar PROM (Blue)
  A85-10.G12 - 63S141 bipolar PROM (Green)
           * - 4 resistors connected to the color PROM outputs.
               They are connected as follows (duplicated 3 times for each color PROM).....

               O4-----R220-----|
                               |
               O3-----R470--+--|
      Final Output----------|  |
               02-----R1k------|
                               |
               01-----R2.2k----|

               The final output of each PROM connects to the JAMMA connector pins for the red, green or blue video output.

***************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "cpu/m6805/m68705.h"
#include "cpu/z80/z80.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class base_state : public driver_device
{
public:
	base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_inputs(*this, "IN%u", 0),
		m_rombank(*this, "rombank"),
		m_sharedram(*this, "sharedram"),
		m_subcpu(*this, "sub"),
		m_ymsnd(*this, "ymsnd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_mainram(*this, "mainram")
	{
	}

	void knightba(machine_config &config) ATTR_COLD;

protected:
	uint32_t screen_update_kicknrun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_kikikai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	IRQ_CALLBACK_MEMBER(mcram_vect_r);

	void base(machine_config &config) ATTR_COLD;
	void add_sub(machine_config &config) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<screen_device> m_screen;
	optional_ioport_array<8> m_inputs;
	required_memory_bank m_rombank;
	optional_shared_ptr<uint8_t> m_sharedram;

private:
	void kicknrun_sub_output_w(uint8_t data);
	virtual void main_f008_w(uint8_t data);
	void main_bankswitch_w(uint8_t data);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void kicknrun_sub_cpu_map(address_map &map) ATTR_COLD;
	void knightba_main_map(address_map &map) ATTR_COLD;

	optional_device<cpu_device> m_subcpu; // kicknrun / mexico86 only
	required_device<ym2203_device> m_ymsnd;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_mainram;

	/* video-related */
	uint8_t m_charbank = 0;
};


class kikikai_state : public base_state
{
public:
	kikikai_state(const machine_config &mconfig, device_type type, const char *tag) :
		base_state(mconfig, type, tag),
		m_mcu(*this, "mcu")
	{
	}

	void kicknrun(machine_config &config) ATTR_COLD;
	void kikikai(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	INTERRUPT_GEN_MEMBER(kikikai_interrupt);

	virtual void main_f008_w(uint8_t data) override;

	void mcu_port1_w(uint8_t data);
	void mcu_port2_w(uint8_t data);
	uint8_t mcu_port3_r();
	void mcu_port3_w(uint8_t data);
	void mcu_port4_w(uint8_t data);

	void add_mcu(machine_config &config) ATTR_COLD;

	required_device<m6801_cpu_device> m_mcu;

	// Kiki KaiKai / Kick and Run MCU
	uint8_t m_port3_in = 0U;
	uint8_t m_port1_out = 0U;
	uint8_t m_port2_out = 0U;
	uint8_t m_port3_out = 0U;
	uint8_t m_port4_out = 0U;
};


class mexico86_state : public base_state
{
public:
	mexico86_state(const machine_config &mconfig, device_type type, const char *tag) :
		base_state(mconfig, type, tag),
		m_mcu(*this, "mcu")
	{
	}

	void mexico86(machine_config &config) ATTR_COLD;
	void knightb(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void add_mcu(machine_config &config) ATTR_COLD;

	virtual void main_f008_w(uint8_t data) override;

	INTERRUPT_GEN_MEMBER(mexico86_68705_interrupt);
	void mexico86_68705_port_a_w(uint8_t data);
	void mexico86_68705_port_b_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);

	required_device<m68705p_device> m_mcu;

	// mexico86 68705 protection
	uint8_t m_port_a_out = 0U;
	uint8_t m_port_b_out = 0U;
	uint8_t m_address = 0;
	uint8_t m_latch = 0U;
};



/*************************************
 *
 *  Video
 *
 *************************************/

uint32_t base_state::screen_update_kicknrun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Similar to bublbobl.cpp, Video hardware generates sprites only.
	bitmap.fill(255, cliprect);
	int sx = 0;

	// the score display seems to be outside of the main objectram
	for (int offs = 0x1500; offs < 0x2000; offs += 4)
	{
		if (offs >= 0x1800 && offs < 0x1980)
			continue;

		if (offs >= 0x19c0)
			continue;

		// skip empty sprites
		// this is dword aligned so the uint32_t * cast shouldn't give problems on any architecture
		if (*(uint32_t *)(&m_mainram[offs]) == 0)
			continue;

		const int gfx_num = m_mainram[offs + 1];
		const int gfx_attr = m_mainram[offs + 3];

		int gfx_offs, height;
		if (!BIT(gfx_num, 7))  // 16x16 sprites
		{
			gfx_offs = ((gfx_num & 0x1f) * 0x80) + ((gfx_num & 0x60) >> 1) + 12;
			height = 2;
		}
		else    // tilemaps (each sprite is a 16x256 column)
		{
			gfx_offs = ((gfx_num & 0x3f) * 0x80);
			height = 32;
		}

		if ((gfx_num & 0xc0) == 0xc0)   // next column
			sx += 16;
		else
		{
			sx = m_mainram[offs + 2];
			//if (gfx_attr & 0x40) sx -= 256;
		}
		const int sy = 256 - height * 8 - (m_mainram[offs + 0]);

		for (int xc = 0; xc < 2; xc++)
		{
			for (int yc = 0; yc < height; yc++)
			{
				const int goffs = gfx_offs + xc * 0x40 + yc * 0x02;
				const int code = m_mainram[goffs] + ((m_mainram[goffs + 1] & 0x07) << 8)
						+ ((m_mainram[goffs + 1] & 0x80) << 4) + (m_charbank << 12);
				const int color = ((m_mainram[goffs + 1] & 0x38) >> 3) + ((gfx_attr & 0x02) << 2);
				const int flipx = m_mainram[goffs + 1] & 0x40;
				const int flipy = 0;

				//const int x = sx + xc * 8;
				const int x = (sx + xc * 8) & 0xff;
				const int y = (sy + yc * 8) & 0xff;

				m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
						code,
						color,
						flipx,flipy,
						x,y,15);
			}
		}
	}

	return 0;
}

uint32_t base_state::screen_update_kikikai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	int sx = 0;

	for (int offs = 0x1500; offs < 0x1800; offs += 4)
	{
		// skip empty sprites
		if (*(uint32_t *)(&m_mainram[offs]) == 0)
			continue;

		int ty = m_mainram[offs];
		int gfx_num = m_mainram[offs + 1];
		int tx = m_mainram[offs + 2];
		//int gfx_attr = m_mainram[offs + 3];

		int gfx_offs, height;
		if (gfx_num & 0x80)
		{
			gfx_offs = ((gfx_num & 0x3f) << 7);
			height = 32;
			if (gfx_num & 0x40) sx += 16;
			else sx = tx;
		}
		else
		{
			if (!(ty && tx))
				continue;
			gfx_offs = ((gfx_num & 0x1f) << 7) + ((gfx_num & 0x60) >> 1) + 12;
			height = 2;
			sx = tx;
		}

		int sy = 256 - (height << 3) - ty;

		height <<= 1;
		for (int yc = 0; yc < height; yc += 2)
		{
			int y = (sy + (yc << 2)) & 0xff;
			int goffs = gfx_offs + yc;
			int code = m_mainram[goffs] + ((m_mainram[goffs + 1] & 0x1f) << 8);
			int color = (m_mainram[goffs + 1] & 0xe0) >> 5;
			goffs += 0x40;

			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					0,0,
					sx&0xff,y,15);

			code = m_mainram[goffs] + ((m_mainram[goffs + 1] & 0x1f) << 8);
			color = (m_mainram[goffs + 1] & 0xe0) >> 5;

			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					0,0,
					(sx+8)&0xff,y,15);
		}
	}

	return 0;
}



/*************************************
 *
 *  I/O handlers
 *
 *************************************/

void base_state::main_bankswitch_w(uint8_t data)
{
	if ((data & 7) > 5)
		logerror("Switching to invalid bank %d\n", data & 7);

	m_rombank->set_entry(data & 7);

	m_charbank = BIT(data, 5);
}

void base_state::kicknrun_sub_output_w(uint8_t data)
{
	// --x- ---- coin lockout 2
	// ---x ---- coin lockout 1
	// ---- -x-- coin counter
	// ---- --x- <unknown, always high, irq ack?>
}


/*
$f008 - write
bit 7 = ? (unused?)
bit 6 = ? (unused?)
bit 5 = ? (unused?)
bit 4 = ? (usually set in game)
bit 3 = ? (usually set in game)
bit 2 = sound CPU reset line
bit 1 = microcontroller reset line
bit 0 = ? (unused?)
*/
void base_state::main_f008_w(uint8_t data)
{
	// knightba doesn't use this, but virtual member function needs to be implemented
	throw emu_fatalerror("unused f008_w");
}

void kikikai_state::main_f008_w(uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 4) ? CLEAR_LINE : ASSERT_LINE);
	m_mcu->set_input_line(INPUT_LINE_RESET, (data & 2) ? CLEAR_LINE : ASSERT_LINE);

	// TODO: bit 0 is flipscreen
}

void mexico86_state::main_f008_w(uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 4) ? CLEAR_LINE : ASSERT_LINE);
	m_mcu->set_input_line(INPUT_LINE_RESET, (data & 2) ? CLEAR_LINE : ASSERT_LINE);
}

INTERRUPT_GEN_MEMBER(kikikai_state::kikikai_interrupt)
{
	device.execute().set_input_line(0, ASSERT_LINE);
}

IRQ_CALLBACK_MEMBER(base_state::mcram_vect_r)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	return m_sharedram[0];
}



/*************************************
 *
 *  Kiki KaiKai / Kick and Run MCU
 *
 *************************************/

void kikikai_state::mcu_port1_w(uint8_t data)
{
	//logerror("%04x: 6801U4 port 1 write %02x\n", m_mcu->pc(), data);

	// bit 0, 1: ?

	// bit 4, 5: coin lockouts
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 4));
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 5));

	// bit 7: ? (set briefly while MCU boots)
	m_port1_out = data;
}

void kikikai_state::mcu_port2_w(uint8_t data)
{
	//logerror("%04x: 6801U4 port 2 write %02x\n", m_mcu->pc(), data);

	// bit 2: clock
	// latch on high->low transition
	if ((m_port2_out & 0x04) && (~data & 0x04))
	{
		int address = m_port4_out;

		if (data & 0x10)
		{
			// read
			if (data & 0x01)
			{
				m_port3_in = m_sharedram[address];
			}
			else
			{
				m_port3_in = m_inputs[(address & 1) + 1]->read();
			}
			m_mcu->pulse_input_line(M6801_IS3_LINE, attotime::from_usec(1));
		}
		else
		{
			// write
			m_sharedram[address] = m_port3_out;
		}
	}

	m_port2_out = data;
}

uint8_t kikikai_state::mcu_port3_r()
{
	//logerror("%04x: 6801U4 port 3 read\n", m_mcu->pc());
	return m_port3_in;
}

void kikikai_state::mcu_port3_w(uint8_t data)
{
	//logerror("%04x: 6801U4 port 3 write %02x\n", m_mcu->pc(), data);
	m_port3_out = data;
}

void kikikai_state::mcu_port4_w(uint8_t data)
{
	//logerror("%04x: 6801U4 port 4 write %02x\n", m_mcu->pc(), data);
	// bits 0-7 of shared RAM address
	m_port4_out = data;
}



/*************************************
 *
 *  Mexico 86 68705 protection interface
 *
 *  The following is ENTIRELY GUESSWORK!!!
 *
 *************************************/

INTERRUPT_GEN_MEMBER(mexico86_state::mexico86_68705_interrupt)
{
	device.execute().set_input_line(M68705_IRQ_LINE, ASSERT_LINE);
}


void mexico86_state::mexico86_68705_port_a_w(uint8_t data)
{
	//logerror("%s: 68705 port A write %02x\n", machine().describe_context(), data);
	m_port_a_out = data;
}


/*
 *  Port B connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  0   W  enables latch which holds data from main Z80 memory
 *  1   W  loads the latch which holds the low 8 bits of the address of
 *               the main Z80 memory location to access
 *  2   W  0 = read input ports, 1 = access Z80 memory
 *  3   W  clocks main Z80 memory access
 *  4   W  selects Z80 memory access direction (0 = write 1 = read)
 *  5   W  clocks a flip-flop which causes IRQ on the main Z80
 *  6   W  not used?
 *  7   W  not used?
 */

void mexico86_state::mexico86_68705_port_b_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	//logerror("%s: 68705 port B write %02x\n", machine().describe_context(), data);

	uint8_t const port_a_value(m_port_a_out & (BIT(m_port_b_out, 0) ? 0xff : m_latch));

	if (BIT(mem_mask, 3) && !BIT(data, 3) && BIT(m_port_b_out, 3))
	{
		if (BIT(m_port_b_out, 4)) // read
		{
			if (BIT(m_port_b_out, 2))
			{
				//logerror("%s: 68705 read %02x from address %04x\n", machine().describe_context(), m_sharedram[m_address], m_address);
				m_latch = m_sharedram[m_address];
			}
			else
			{
				//logerror("%s: 68705 read input port %04x\n", machine().describe_context(), m_address);
				m_latch = m_inputs[(m_address & 1) + 1]->read();
			}
		}
		else // write
		{
			//logerror("%s: 68705 write %02x to address %04x\n",machine().describe_context(), port_a_value, m_address);
			m_sharedram[m_address] = port_a_value;
		}
	}

	m_mcu->pa_w((BIT(mem_mask, 0) && !BIT(data, 0)) ? m_latch : 0xff);

	if (BIT(mem_mask, 1) && !BIT(data, 1) && BIT(m_port_b_out, 1))
	{
		m_address = port_a_value;
		//if (m_address >= 0x80) logerror("%s: 68705 address %02x\n", machine().describe_context(), port_a_value);
	}

	if (BIT(mem_mask, 5) && BIT(data, 5) && !BIT(m_port_b_out, 5))
	{
		m_maincpu->set_input_line(0, ASSERT_LINE);
		m_mcu->set_input_line(M68705_IRQ_LINE, CLEAR_LINE);
	}

	if (BIT(mem_mask, 6) && !BIT(data, 6) && BIT(m_port_b_out, 6))
		logerror("%s: 68705 unknown port B bit %02x\n", machine().describe_context(), data);

	if (BIT(mem_mask, 7) && !BIT(data, 7) && BIT(m_port_b_out, 7))
		logerror("%s: 68705 unknown port B bit %02x\n", machine().describe_context(), data);

	m_port_b_out = data;
}



/*************************************
 *
 *  Address maps
 *
 *************************************/

void base_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_rombank);         // banked ROMs
	map(0xc000, 0xe7ff).ram().share(m_mainram);   // shared with sound CPU
	map(0xe800, 0xe8ff).ram().share(m_sharedram); // shared with MCU
	map(0xe900, 0xefff).ram();
	map(0xf000, 0xf000).w(FUNC(base_state::main_bankswitch_w)); // program and gfx ROM banks
	map(0xf008, 0xf008).w(FUNC(base_state::main_f008_w));       // CPU reset lines + other unknown stuff
	map(0xf010, 0xf010).portr("IN3");
	map(0xf018, 0xf018).nopw();                   // watchdog?
	map(0xf800, 0xffff).ram().share("subram");    // communication ram - to connect 4 players' subboard
}

void base_state::knightba_main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_rombank);       // banked ROMs
	map(0xc000, 0xe7ff).ram().share(m_mainram); // shared with sound CPU
	map(0xe800, 0xefff).ram();
	map(0xf000, 0xf000).w(FUNC(base_state::main_bankswitch_w)); // program and gfx ROM banks
	map(0xf008, 0xf008).nopw();
	map(0xf010, 0xf010).portr("IN3");
	map(0xf018, 0xf018).nopw();                 // watchdog?
	map(0xf019, 0xf019).portr("IN1");
}

void base_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xa7ff).ram().share(m_mainram); // shared with main
	map(0xa800, 0xbfff).ram();
	map(0xc000, 0xc001).rw(m_ymsnd, FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}

void base_state::kicknrun_sub_cpu_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram(); // sub CPU ram
	map(0x8000, 0x87ff).ram().share("subram"); // shared with main
	map(0xc000, 0xc000).portr("IN4");
	map(0xc001, 0xc001).portr("IN5");
	map(0xc002, 0xc002).portr("IN6");
	map(0xc003, 0xc003).portr("IN7");
	map(0xc004, 0xc004).w(FUNC(base_state::kicknrun_sub_output_w));
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( kicknrun )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE )    /* service 2 */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	/* When Bit 1 is On, the machine waits a signal from another one */
	/* Seems like if you can join two cabinets, one as master */
	/* and the other as slave, probably to play four players. */
	PORT_DIPNAME( 0x01, 0x01, "Master/Slave Mode" ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SWA:2") // Screen ?
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWA:4") // Demo Sounds only play every 8th Demo
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWA:5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x08, "Playing Time" ) PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x00, "40 Seconds" )
	PORT_DIPSETTING(    0x0c, "One Minute" )
	PORT_DIPSETTING(    0x08, "One Minute and 20 Sec." )
	PORT_DIPSETTING(    0x04, "One Minute and 40 Sec." )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* The following dip seems to be related with the first one */
	PORT_DIPNAME( 0x20, 0x20, "Board ID" ) PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, "Master" )
	PORT_DIPSETTING(    0x00, "Slave" )
	PORT_DIPNAME( 0x40, 0x40, "Number of Matches" ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x40, "6" )
	PORT_DIPNAME( 0x80, 0x80, "Single board 4 Players Mode" ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) //p3 service

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) //p4 service

	PORT_START("IN6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,IPT_COIN4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( kikikai )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // Ofuda in service mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) // Oharai
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SWA:3")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

#if 0 // old coinage settings
	PORT_DIPNAME( 0x30, 0x30, "Coin 1" )
	PORT_DIPSETTING(    0x30, "A:1C/1C B:1C/1C" )
	PORT_DIPSETTING(    0x20, "A:1C/2C B:2C/1C" )
	PORT_DIPSETTING(    0x10, "A:2C/1C B:3C/1C" )
	PORT_DIPSETTING(    0x00, "A:2C/3C B:4C/1C" )
	PORT_DIPNAME( 0xc0, 0xc0, "Coin 2" )
	PORT_DIPSETTING(    0xc0, "A:1C/1C B:1C/2C" )
	PORT_DIPSETTING(    0x80, "A:1C/2C B:1C/3C" )
	PORT_DIPSETTING(    0x40, "A:2C/1C B:1C/4C" )
	PORT_DIPSETTING(    0x00, "A:2C/3C B:1C/6C" )
#endif

	// coinage copied from Japanese manual but Type 2 doesn't work so when Type 2 is selected, the Type 1 coinage is still active.
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWA:5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x00)
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x00)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x00)
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x00)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_4C ) )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x00)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )  PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x00, "50000 100000" )
	PORT_DIPSETTING(    0x0c, "70000 150000" )
	PORT_DIPSETTING(    0x08, "70000 200000" )
	PORT_DIPSETTING(    0x04, "100000 300000" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )  PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, "Type 1" )
	PORT_DIPSETTING(    0x00, "Type 2" )
	PORT_DIPNAME( 0x80, 0x00, "Number Match" )  PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( knightba )
	PORT_INCLUDE(kikikai)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	4*2048,
	4,
	{ 0x20000*8, 0x20000*8+4, 0, 4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static GFXDECODE_START( gfx_mexico86 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void base_state::machine_start()
{
	uint8_t *const ROM = memregion("maincpu")->base();
	m_rombank->configure_entries(0, 6, &ROM[0x08000], 0x4000);

	save_item(NAME(m_charbank));
}

void kikikai_state::machine_start()
{
	base_state::machine_start();

	save_item(NAME(m_port3_in));
	save_item(NAME(m_port1_out));
	save_item(NAME(m_port2_out));
	save_item(NAME(m_port3_out));
	save_item(NAME(m_port4_out));
}

void mexico86_state::machine_start()
{
	base_state::machine_start();

	save_item(NAME(m_port_a_out));
	save_item(NAME(m_port_b_out));
	save_item(NAME(m_address));
	save_item(NAME(m_latch));

	m_port_a_out = 0xff;
	m_port_b_out = 0xff;
}



void base_state::machine_reset()
{
	// TODO: check the PCB and see how the halt / reset lines are connected.
	if (m_subcpu != nullptr)
		m_subcpu->set_input_line(INPUT_LINE_RESET, (ioport("DSW1")->read() & 0x80) ? ASSERT_LINE : CLEAR_LINE);

	m_charbank = 0;
}

void mexico86_state::machine_reset()
{
	base_state::machine_reset();

	m_address = 0;
	m_latch = 0;
}


void base_state::base(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 24_MHz_XTAL / 4); // 6 MHz, Uses clock divided 24 MHz OSC
	m_maincpu->set_addrmap(AS_PROGRAM, &base_state::main_map);

	Z80(config, m_audiocpu, 24_MHz_XTAL / 4); // 6 MHz, Uses clock divided 24 MHz OSC
	m_audiocpu->set_addrmap(AS_PROGRAM, &base_state::sound_map);
	m_audiocpu->set_vblank_int("screen", FUNC(base_state::irq0_line_hold));

	/* 100 CPU slices per frame - high value to ensure proper synchronization of the CPUs */
	config.set_maximum_quantum(attotime::from_hz(6000));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(24_MHz_XTAL / 4, 384, 0, 256, 264, 16, 240);
	m_screen->set_screen_update(FUNC(base_state::screen_update_kicknrun));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mexico86);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2203(config, m_ymsnd, 24_MHz_XTAL / 8);
	m_ymsnd->port_a_read_callback().set_ioport("DSW0");
	m_ymsnd->port_b_read_callback().set_ioport("DSW1");
	m_ymsnd->add_route(0, "mono", 0.30);
	m_ymsnd->add_route(1, "mono", 0.30);
	m_ymsnd->add_route(2, "mono", 0.30);
	m_ymsnd->add_route(3, "mono", 1.00);
}

void base_state::add_sub(machine_config &config)
{
	Z80(config, m_subcpu, 8_MHz_XTAL / 2); // 4 MHz, Uses 8 MHz OSC
	m_subcpu->set_addrmap(AS_PROGRAM, &base_state::kicknrun_sub_cpu_map);
	m_subcpu->set_vblank_int("screen", FUNC(base_state::irq0_line_hold));
}


void kikikai_state::add_mcu(machine_config &config)
{
	// Not too sure IRQs are triggered by MCU..
	m_maincpu->set_vblank_int("screen", FUNC(kikikai_state::kikikai_interrupt));
	m_maincpu->set_irq_acknowledge_callback(FUNC(kikikai_state::mcram_vect_r));

	M6801U4(config, m_mcu, 24_MHz_XTAL / 8);
	m_mcu->in_p1_cb().set_ioport("IN0").invert();
	m_mcu->out_p1_cb().set(FUNC(kikikai_state::mcu_port1_w));
	m_mcu->out_p2_cb().set(FUNC(kikikai_state::mcu_port2_w));
	m_mcu->out_p3_cb().set(FUNC(kikikai_state::mcu_port3_w));
	m_mcu->in_p3_cb().set(FUNC(kikikai_state::mcu_port3_r));
	m_mcu->out_p4_cb().set(FUNC(kikikai_state::mcu_port4_w));

	config.set_perfect_quantum(m_maincpu);

	m_screen->screen_vblank().set_inputline(m_mcu, M6801_IRQ1_LINE); // same clock latches the INT pin on the second Z80
}

void kikikai_state::kicknrun(machine_config &config)
{
	base(config);
	add_sub(config);
	add_mcu(config);
}

void kikikai_state::kikikai(machine_config &config)
{
	base(config);
	add_mcu(config);

	m_screen->set_screen_update(FUNC(kikikai_state::screen_update_kikikai));
}


void mexico86_state::add_mcu(machine_config &config)
{
	m_maincpu->set_irq_acknowledge_callback(FUNC(mexico86_state::mcram_vect_r));

	M68705P3(config, m_mcu, 4_MHz_XTAL); // crystal is 4 MHz, divided by 4 internally
	m_mcu->portc_r().set_ioport("IN0");
	m_mcu->porta_w().set(FUNC(mexico86_state::mexico86_68705_port_a_w));
	m_mcu->portb_w().set(FUNC(mexico86_state::mexico86_68705_port_b_w));
	m_mcu->set_vblank_int("screen", FUNC(mexico86_state::mexico86_68705_interrupt));
}

void mexico86_state::mexico86(machine_config &config)
{
	base(config);
	add_sub(config);
	add_mcu(config);
}

void mexico86_state::knightb(machine_config &config)
{
	base(config);
	add_mcu(config);

	m_screen->set_screen_update(FUNC(mexico86_state::screen_update_kikikai));
}


void base_state::knightba(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &base_state::knightba_main_map);
	m_maincpu->set_vblank_int("screen", FUNC(base_state::irq0_line_hold));

	m_screen->set_screen_update(FUNC(base_state::screen_update_kikikai));
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( kikikai )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "a85-17.h16", 0x00000, 0x08000, CRC(c141d5ab) SHA1(fe3622ba283e514416c43a44f83f922a958b27cd) ) /* 1st half, main code        */
	ROM_CONTINUE(           0x18000, 0x08000 )             /* 2nd half, banked at 0x8000 */
	ROM_LOAD( "a85-16.h18", 0x08000, 0x10000, CRC(4094d750) SHA1(05e0ad177a3eb144b203784ecb6242a0fc5c4d4d) ) /* banked at 0x8000           */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a85-11.f6", 0x0000, 0x8000, CRC(cc3539db) SHA1(4239a40fdee65cba613e4b4ec54cf7899480e366) )

	ROM_REGION( 0x1000, "mcu", 0 )    /* 4k for the microcontroller (MC6801U4 type MCU) */
	/* MCU labeled TAITO A85 01,  JPH1020P, 185, PS4 */
	ROM_LOAD( "a85-01_jph1020p.h8", 0x0000, 0x1000, CRC(01771197) SHA1(84430a56c66ff2781fe1ff35d4f15b332cd0af37) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a85-15.a1", 0x00000, 0x10000, CRC(aebc8c32) SHA1(77347cf5780f084a77123eb636cd0bad672a39e8) )
	ROM_LOAD( "a85-14.a3", 0x10000, 0x10000, CRC(a9df0453) SHA1(a5e9cd6266ab3ae46cd1b35a4603e13a2ca023fb) )
	ROM_LOAD( "a85-13.a4", 0x20000, 0x10000, CRC(3eeaf878) SHA1(f8ae8938a8358d1222e9fdf7bc0094ac13faf404) )
	ROM_LOAD( "a85-12.a6", 0x30000, 0x10000, CRC(91e58067) SHA1(c7eb9bf650039254fb7664758938b1012eacc597) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "a85-08.g15", 0x0000, 0x0100, CRC(d15f61a8) SHA1(945c8aa26c85269c10373218bef13e04e25eb1e4) ) /* all proms are 63S141AN or compatible type */
	ROM_LOAD( "a85-10.g12", 0x0100, 0x0100, CRC(8fc3fa86) SHA1(d4d86f8e147bbf2a370de428ac20a28b0f146782) )
	ROM_LOAD( "a85-09.g14", 0x0200, 0x0100, CRC(b931c94d) SHA1(fb554084f34c602d1ff7806fb945a06cf14332af) )
ROM_END

ROM_START( knightb )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "a85-17.h16", 0x00000, 0x08000, CRC(c141d5ab) SHA1(fe3622ba283e514416c43a44f83f922a958b27cd) ) /* 1st half, main code        */
	ROM_CONTINUE(           0x18000, 0x08000 )             /* 2nd half, banked at 0x8000 */
	ROM_LOAD( "a85-16.h18", 0x08000, 0x10000, CRC(4094d750) SHA1(05e0ad177a3eb144b203784ecb6242a0fc5c4d4d) ) /* banked at 0x8000           */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a85-11.f6", 0x0000, 0x8000, CRC(cc3539db) SHA1(4239a40fdee65cba613e4b4ec54cf7899480e366) )

	ROM_REGION( 0x0800, "mcu", 0 )    /* 2k for the microcontroller */
	ROM_LOAD( "knightb.uc", 0x0000, 0x0800, CRC(3cc2bbe4) SHA1(af018a1e0655b66fd859617a3bd0c01a4967c0e6) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "knightb.d",  0x00000, 0x10000, CRC(53ecdb3f) SHA1(f8b4822926f3712a426c014759b1cf382a7ad9d1) )
	ROM_LOAD( "a85-14.a3",  0x10000, 0x10000, CRC(a9df0453) SHA1(a5e9cd6266ab3ae46cd1b35a4603e13a2ca023fb) )
	ROM_LOAD( "knightb.b",  0x20000, 0x10000, CRC(63ad7df3) SHA1(8ce149b63032bcdd596a3fa52baba2f2c154e84e) )
	ROM_LOAD( "a85-12.a6", 0x30000, 0x10000, CRC(91e58067) SHA1(c7eb9bf650039254fb7664758938b1012eacc597) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "a85-08.g15", 0x0000, 0x0100, CRC(d15f61a8) SHA1(945c8aa26c85269c10373218bef13e04e25eb1e4) ) /* all proms are 63S141AN or compatible type */
	ROM_LOAD( "a85-10.g12", 0x0100, 0x0100, CRC(8fc3fa86) SHA1(d4d86f8e147bbf2a370de428ac20a28b0f146782) )
	ROM_LOAD( "a85-09.g14", 0x0200, 0x0100, CRC(b931c94d) SHA1(fb554084f34c602d1ff7806fb945a06cf14332af) )
ROM_END

ROM_START( knightba ) // similar to the bootleg above but without MCU. Only different ROM is 7, where the comms with the MCU have been patched out.
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "7.bin", 0x00000, 0x08000, CRC(f1d90e89) SHA1(d2e4a036aca9b84189d74b09e53fea9fa0abe959) ) /* 1st half, main code        */
	ROM_CONTINUE(      0x18000, 0x08000 )               /* 2nd half, banked at 0x8000 */
	ROM_LOAD( "6.bin", 0x08000, 0x10000, CRC(4094d750) SHA1(05e0ad177a3eb144b203784ecb6242a0fc5c4d4d) ) /* banked at 0x8000           */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "5.bin", 0x0000, 0x8000, CRC(9ba79d96) SHA1(33b570f0baad6bd417b4dd6389e6263b691906e2) )
	ROM_IGNORE(                0x8000 ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "4.bin", 0x00000, 0x10000, CRC(53ecdb3f) SHA1(f8b4822926f3712a426c014759b1cf382a7ad9d1) )
	ROM_LOAD( "3.bin", 0x10000, 0x10000, CRC(a9df0453) SHA1(a5e9cd6266ab3ae46cd1b35a4603e13a2ca023fb) )
	ROM_LOAD( "2.bin", 0x20000, 0x10000, CRC(63ad7df3) SHA1(8ce149b63032bcdd596a3fa52baba2f2c154e84e) )
	ROM_LOAD( "1.bin", 0x30000, 0x10000,  CRC(91e58067) SHA1(c7eb9bf650039254fb7664758938b1012eacc597) )

	ROM_REGION( 0x0300, "proms", 0 ) // not dumped for this set, but they are almost surely correct
	ROM_LOAD( "a85-08.g15", 0x0000, 0x0100, CRC(d15f61a8) SHA1(945c8aa26c85269c10373218bef13e04e25eb1e4) )
	ROM_LOAD( "a85-10.g12", 0x0100, 0x0100, CRC(8fc3fa86) SHA1(d4d86f8e147bbf2a370de428ac20a28b0f146782) )
	ROM_LOAD( "a85-09.g14", 0x0200, 0x0100, CRC(b931c94d) SHA1(fb554084f34c602d1ff7806fb945a06cf14332af) )
ROM_END

ROM_START( kicknrun )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "a87-08.h16", 0x00000, 0x08000, CRC(715e1b04) SHA1(60b7259758ec73f1cc945556e9c2b25766b745a8) ) /* 1st half, main code        */
	ROM_CONTINUE(           0x18000, 0x08000 )             /* 2nd half, banked at 0x8000 */
	ROM_LOAD( "a87-07.h18", 0x08000, 0x10000, CRC(6cb6ebfe) SHA1(fca61fc2ad8fadc1e15b9ff84c7469b68d16e885) ) /* banked at 0x8000           */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a87-06.f6", 0x0000, 0x8000, CRC(1625b587) SHA1(7336384e13c114915de5e439df5731ce3fc2054a) )

	ROM_REGION( 0x1000, "mcu", 0 )    /* 4k for the microcontroller (MC6801U4 type MCU) */
	ROM_LOAD( "a87-01_jph1021p.h8", 0x0000, 0x1000, CRC(9451e880) SHA1(e9a505296108645f99449d391d0ebe9ac1b9984e) ) /* MCU labeled TAITO A87-01,  JPH1021P, 185, PS4 */

	ROM_REGION( 0x10000, "sub", 0 )    /* 64k for the cpu on the sub board */
	ROM_LOAD( "a87-09-1",  0x0000, 0x4000, CRC(6a2ad32f) SHA1(42d4b97b25d219902ad215793f1d2c006ffe94dc) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a87-05.a1", 0x08000, 0x08000, CRC(4eee3a8a) SHA1(2f0e4c2fb6cba48d0e2b95927fc14f0038557371) )
	ROM_CONTINUE(          0x00000, 0x08000 )
	ROM_LOAD( "a87-04.a3", 0x10000, 0x08000, CRC(8b438d20) SHA1(12e615f34b7e732157f893b97c9b7e99e9ef7d62) )
	ROM_RELOAD(            0x18000, 0x08000 )
	ROM_LOAD( "a87-03.a4", 0x28000, 0x08000, CRC(f42e8a88) SHA1(db2702141981ba368bdc665443a8a0662266e6d9) )
	ROM_CONTINUE(          0x20000, 0x08000 )
	ROM_LOAD( "a87-02.a6", 0x30000, 0x08000, CRC(64f1a85f) SHA1(04fb9824450812b08f7e6fc57e0af828be9bd575) )
	ROM_RELOAD(            0x38000, 0x08000 )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "a87-10.g15", 0x0000, 0x0100, CRC(be6eb1f0) SHA1(f4d00e9b12bf116bf84edb2ff6caab158094b668) ) /* all proms are 63S141AN or compatible type */
	ROM_LOAD( "a87-12.g12", 0x0100, 0x0100, CRC(3e953444) SHA1(e9c84ca9390fd7c73738a8b681a02e87fbd51bb4) )
	ROM_LOAD( "a87-11.g14", 0x0200, 0x0100, CRC(14f6c28d) SHA1(8c60974e4607906a3f77260bdd0704af60d596fc) )
ROM_END

ROM_START( kicknrunu )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "a87-23.h16", 0x00000, 0x08000, CRC(37182560) SHA1(8db393131f50af88b2e7489d6aae65bad0a5a65b) ) /* 1st half, main code        */
	ROM_CONTINUE(           0x18000, 0x08000 )             /* 2nd half, banked at 0x8000 */
	ROM_LOAD( "a87-22.h18", 0x08000, 0x10000, CRC(3b5a8354) SHA1(e0db4cb0657989d5a21f9a8d4e8f842adba636ad) ) /* banked at 0x8000           */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a87-06.f6", 0x0000, 0x8000, CRC(1625b587) SHA1(7336384e13c114915de5e439df5731ce3fc2054a) )

	ROM_REGION( 0x1000, "mcu", 0 )    /* 4k for the microcontroller (MC6801U4 type MCU) */
	ROM_LOAD( "a87-01_jph1021p.h8", 0x0000, 0x1000, CRC(9451e880) SHA1(e9a505296108645f99449d391d0ebe9ac1b9984e) ) /* MCU labeled TAITO A87-01,  JPH1021P, 185, PS4 */

	ROM_REGION( 0x10000, "sub", 0 )    /* 64k for the cpu on the sub board */
	ROM_LOAD( "a87-09-1",  0x0000, 0x4000, CRC(6a2ad32f) SHA1(42d4b97b25d219902ad215793f1d2c006ffe94dc) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a87-05.a1", 0x08000, 0x08000, CRC(4eee3a8a) SHA1(2f0e4c2fb6cba48d0e2b95927fc14f0038557371) )
	ROM_CONTINUE(          0x00000, 0x08000 )
	ROM_LOAD( "a87-04.a3", 0x10000, 0x08000, CRC(8b438d20) SHA1(12e615f34b7e732157f893b97c9b7e99e9ef7d62) )
	ROM_RELOAD(            0x18000, 0x08000 )
	ROM_LOAD( "a87-03.a4", 0x28000, 0x08000, CRC(f42e8a88) SHA1(db2702141981ba368bdc665443a8a0662266e6d9) )
	ROM_CONTINUE(          0x20000, 0x08000 )
	ROM_LOAD( "a87-02.a6", 0x30000, 0x08000, CRC(64f1a85f) SHA1(04fb9824450812b08f7e6fc57e0af828be9bd575) )
	ROM_RELOAD(            0x38000, 0x08000 )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "a87-10.g15", 0x0000, 0x0100, CRC(be6eb1f0) SHA1(f4d00e9b12bf116bf84edb2ff6caab158094b668) ) /* all proms are 63S141AN or compatible type */
	ROM_LOAD( "a87-12.g12", 0x0100, 0x0100, CRC(3e953444) SHA1(e9c84ca9390fd7c73738a8b681a02e87fbd51bb4) )
	ROM_LOAD( "a87-11.g14", 0x0200, 0x0100, CRC(14f6c28d) SHA1(8c60974e4607906a3f77260bdd0704af60d596fc) )
ROM_END

ROM_START( mexico86 )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "2_g.bin",    0x00000, 0x08000, CRC(2bbfe0fb) SHA1(8f047e001ea8e49d28f73e546c82812af1c2533c) ) /* 1st half, main code        */
	ROM_CONTINUE(           0x18000, 0x08000 )             /* 2nd half, banked at 0x8000 */
	ROM_LOAD( "1_f.bin",    0x08000, 0x10000, CRC(0b93e68e) SHA1(c6fbcce83103e3e71a7a1ef9f18a10622ed6b951) ) /* banked at 0x8000           */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a87-06.f6", 0x0000, 0x8000, CRC(1625b587) SHA1(7336384e13c114915de5e439df5731ce3fc2054a) )

	ROM_REGION( 0x0800, "mcu", 0 )    /* 2k for the microcontroller */
	ROM_LOAD( "68_h.bin",   0x0000, 0x0800, CRC(ff92f816) SHA1(0015c3f2ed014052b3fa376409e3a7cca36fac72) )

	ROM_REGION( 0x10000, "sub", 0 )    /* 64k for the cpu on the sub board */
	ROM_LOAD( "a87-09-1",  0x0000, 0x4000, CRC(6a2ad32f) SHA1(42d4b97b25d219902ad215793f1d2c006ffe94dc) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "4_d.bin",    0x08000, 0x08000, CRC(57cfdbca) SHA1(89c305c380c3de14a956ee4bc85d3a0d343b638e) )
	ROM_CONTINUE(           0x00000, 0x08000 )
	ROM_LOAD( "5_c.bin",    0x10000, 0x08000, CRC(e42fa143) SHA1(02d7e0e01af1cecc3952f6355987118098d346c3) )
	ROM_RELOAD(             0x18000, 0x08000 )
	ROM_LOAD( "6_b.bin",    0x28000, 0x08000, CRC(a4607989) SHA1(6832147603a146c34cc1809e839c8e034d0dacc5) )
	ROM_CONTINUE(           0x20000, 0x08000 )
	ROM_LOAD( "7_a.bin",    0x30000, 0x08000, CRC(245036b1) SHA1(108d9959de869b4fdf766abeade1486acec13bf2) )
	ROM_RELOAD(             0x38000, 0x08000 )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "a87-10.g15", 0x0000, 0x0100, CRC(be6eb1f0) SHA1(f4d00e9b12bf116bf84edb2ff6caab158094b668) ) /* all proms are 63S141AN or compatible type */
	ROM_LOAD( "a87-12.g12", 0x0100, 0x0100, CRC(3e953444) SHA1(e9c84ca9390fd7c73738a8b681a02e87fbd51bb4) )
	ROM_LOAD( "a87-11.g14", 0x0200, 0x0100, CRC(14f6c28d) SHA1(8c60974e4607906a3f77260bdd0704af60d596fc) )
ROM_END

ROM_START( mexico86a )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "2.bin",    0x00000, 0x08000, CRC(397c93ad) SHA1(6b28d284cafb86f3efd13033984caa1a221a8a14) ) /* 1st half, main code        */
	ROM_CONTINUE(         0x18000, 0x08000 )             /* 2nd half, banked at 0x8000 */
	ROM_LOAD( "1.bin",    0x08000, 0x10000, CRC(0b93e68e) SHA1(c6fbcce83103e3e71a7a1ef9f18a10622ed6b951) ) /* banked at 0x8000           */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "3x.bin", 0x0000, 0x8000, CRC(abbbf6c4) SHA1(27456d8607e0a246f0c2ad1bc57ee7e4ec37b278) ) // 0x1FEF is 0x2f instead of 0x0f, causes checksum failure, bad?
	ROM_LOAD( "3.bin",  0x0000, 0x8000, CRC(1625b587) SHA1(7336384e13c114915de5e439df5731ce3fc2054a) )

	ROM_REGION( 0x0800, "mcu", 0 )    /* 2k for the microcontroller */
	ROM_LOAD( "68_h.bin",   0x0000, 0x0800, CRC(ff92f816) SHA1(0015c3f2ed014052b3fa376409e3a7cca36fac72) ) // missing in this set, not dumped or never present??

	ROM_REGION( 0x10000, "sub", 0 )    /* 64k for the cpu on the sub board */
	ROM_LOAD( "8.bin",  0x0000, 0x4000, CRC(6a2ad32f) SHA1(42d4b97b25d219902ad215793f1d2c006ffe94dc) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "4.bin",    0x08000, 0x08000, CRC(57cfdbca) SHA1(89c305c380c3de14a956ee4bc85d3a0d343b638e) )
	ROM_CONTINUE(         0x00000, 0x08000 )
	ROM_LOAD( "5.bin",    0x10000, 0x08000, CRC(e42fa143) SHA1(02d7e0e01af1cecc3952f6355987118098d346c3) )
	ROM_RELOAD(           0x18000, 0x08000 )
	ROM_LOAD( "6.bin",    0x28000, 0x08000, CRC(a4607989) SHA1(6832147603a146c34cc1809e839c8e034d0dacc5) )
	ROM_CONTINUE(         0x20000, 0x08000 )
	ROM_LOAD( "7.bin",    0x30000, 0x08000, CRC(245036b1) SHA1(108d9959de869b4fdf766abeade1486acec13bf2) )
	ROM_RELOAD(           0x38000, 0x08000 )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "n82s129n.1.bin", 0x0000, 0x0100, CRC(be6eb1f0) SHA1(f4d00e9b12bf116bf84edb2ff6caab158094b668) ) /* all proms are 63S141AN or compatible type */
	ROM_LOAD( "n82s129n.3.bin", 0x0100, 0x0100, CRC(3e953444) SHA1(e9c84ca9390fd7c73738a8b681a02e87fbd51bb4) )
	ROM_LOAD( "n82s129n.2.bin", 0x0200, 0x0100, CRC(14f6c28d) SHA1(8c60974e4607906a3f77260bdd0704af60d596fc) )

	ROM_REGION( 0x0300, "plds", 0 )
	ROM_LOAD( "ampal16l8pc.1.bin", 0x0000, 0x0104, CRC(634f3a5b) SHA1(af895a10563e0011c9e6546de2bb61fb8c93bcf4) )
	ROM_LOAD( "ampal16l8pc.3.bin", 0x0000, 0x0104, CRC(f9ce900a) SHA1(c2b4626a4663a3dedd70e04833b9968e58ae372b) )
	ROM_LOAD( "ampal16l8pc.4.bin", 0x0000, 0x0104, CRC(39120b6f) SHA1(d44291f72566f2ad61ab6e612c2c6212076ef541) )
	ROM_LOAD( "ampal16l8pc.5.bin", 0x0000, 0x0104, CRC(1d27f7b9) SHA1(7fe3cb474c599acd7b5fe74bef8f2bae582f2ce9) )
	ROM_LOAD( "ampal16l8pc.6.bin", 0x0000, 0x0104, CRC(9f941c8e) SHA1(34728a572132c23bd2887452ec7ad38504d392d7) )
	ROM_LOAD( "ampal16l8pc.7.bin", 0x0000, 0x0104, CRC(9f941c8e) SHA1(34728a572132c23bd2887452ec7ad38504d392d7) )

	ROM_LOAD( "ampal16r4pc.2.bin", 0x0000, 0x0104, CRC(213a71d1) SHA1(a83b1c089fae72b8216533d0733491c3dc3630af) )
ROM_END

} // anonymous namespace



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1986, kikikai,   0,        kikikai,  kikikai,  kikikai_state,  empty_init, ROT90, "Taito Corporation",          "KiKi KaiKai",                                MACHINE_SUPPORTS_SAVE )
GAME( 1986, knightb,   kikikai,  knightb,  kikikai,  mexico86_state, empty_init, ROT90, "bootleg",                    "Knight Boy (bootleg of KiKi KaiKai, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, knightba,  kikikai,  knightba, knightba, base_state,     empty_init, ROT90, "bootleg (Game Corporation)", "Knight Boy (bootleg of KiKi KaiKai, set 2)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // missing coins/dsw, can be played using service to coin

GAME( 1986, kicknrun,  0,        kicknrun, kicknrun, kikikai_state,  empty_init, ROT0,  "Taito Corporation",          "Kick and Run (World)",                       MACHINE_SUPPORTS_SAVE )
GAME( 1986, kicknrunu, kicknrun, kicknrun, kicknrun, kikikai_state,  empty_init, ROT0,  "Taito America Corp",         "Kick and Run (US)",                          MACHINE_SUPPORTS_SAVE )
GAME( 1986, mexico86,  kicknrun, mexico86, kicknrun, mexico86_state, empty_init, ROT0,  "bootleg",                    "Mexico 86 (bootleg of Kick and Run, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, mexico86a, kicknrun, mexico86, kicknrun, mexico86_state, empty_init, ROT0,  "bootleg",                    "Mexico 86 (bootleg of Kick and Run, set 2)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
