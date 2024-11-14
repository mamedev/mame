// license:<license>
// copyright-holders: Roberto Fresca, Grull Osgo.
/***************************************************************************************

  HOT CHILLI
  1995, Pacific Gaming Pty Ltd.

  Driver by Roberto Fresca & Grull Osgo.


  One of the most beautiful designs for 8bit slots machines. Very rare.

  The reverse-engineering was made seeing a few PCB pictures, and following the code.
  After all these exhaustive analysis we finally got the inputs working, but they are
  temporized and transformed in some way through dedicated hardware, so maybe they are
  not totally accurate.

  The game has dual coin sensor (upper/lower) to allow hook a modern coin comparator.


****************************************************************************************

  Technical Notes:
  ----------------

  MAIN PCB:
            J1100155A
            K1100355A

  1x Zilog Z0840006PSC (Z80 CPU)
  1x Yamaha YM2149F

  1x Allumer/Seta X1-001A
  1x Allumer/Seta X1-002A

  1x 6264 SRAM.

  1x 27C512 for program (IC32).
  4x 27C512 for graphics (IC1, IC2, IC25, IC26).

  2x MB7124 Bipolar PROMs (IC8, IC9).

  1x Maxim MAX232C (IC55)
  1x Taito TC0070RGB (RGB DAC)
  1x NEC D825

  1x Xtal 12.000 MHz.
  1x Xtal 2.4576 MHz.

  1x unpopulated DIP40 socket
     marked as JAE PS-20STS (*)

  (*) Here is plugged a UNIVERSAL RAM PCB:

  Etched:
           J9100140A
           K9100188A

  IC1: 8464A-10LL
  IC2: LS174
  IC3: LM393N
  IC4: MC14011BCP
  IC5: SN74LS273N
  IC6: AMPAL18P8APC
  IC7: SN74LS74AN

  1x unknown battery (not present)


***************************************************************************************

  How to play...

  Let the machine boots.
  Add credits with the COIN IN key (key 5).
  Choose the number of lines to play (1-3-5-7-9) using the PLAY LINE keys (keys A-S-D-F-G).
  Bet/Play de desired amount of credits (1-2-3-5-10) using the BET buttons (keys Z-X-C-V-B).


  To enter the Test Mode:

  1) Open the Door (be patient, it's a temporized input and could take more than 10 sec.)
  2) Use TEST key (key T) to switch the test type.
  3) Use PLAY 1 CREDIT key (key Z) to choose/set.
  3) Use the TEST END key (key Y) to exit to the game.

  To see the meters just press BOOKS key (key 0). Press it again to exit.
  If you want to use the clear/set function, just keep pressed key 9 during boot.


***************************************************************************************

  Message codes:
  --------------

  00 CLEAR MESSAGE LINE
  01 COPYRIGHT PACIFIC GAMES
  02 HOPPER EMPTY CALL ATT
  03 DOOR OPENED DO NOT PLAY
  04 DOOR CLOSED PLAY BOW
  05 MACHINE RESERVED
  06 POWER UP PLAY NOW
  07/08 CONGRATULATIONS LARGE WIN
  09 LOW VOLTAGE
  0A BATTERY
  0B COIN JAM
  0C COINS OVER DISPENSED
  0D HOPPER JAM
  0E HOPPER JAM
  0F SUB CLEAR
  10 dIP SW 8 ON
  11 GAME INVALID OFF LINE ERROR
  12 GAME INVALID COMM ERROR
  13 MACHINE STOP
  14 GAME INVALID METER FAULT
  15 PLAY OR PRESS COLLECT FOR HAND PAY
  16 CALL ATT. FOR HAND PAY
  17 A A A A A A A A
  18 OVER 50 CREDITS CALL ATT.
  19 OVER 100 CREDITS CALL ATT.
  1A OVER 150 CREDITS CALL ATT.
  1B OVER 250 CREDITS CALL ATT.
  1C OVER 300 CREDITS CALL ATT.
  1D OVER 500 CREDITS CALL ATT.
  1E OVER 600 CREDITS CALL ATT.
  1F OVER 1000 CREDITS CALL ATT.
  20 OVER 1500 CREDITS CALL ATT.
  21 OVER 2500 CREDITS CALL ATT.
  22 OVER 3000 CREDITS CALL ATT.
  23 OVER 5000 CREDITS CALL ATT.
  24 OVER 10000 CREDITS CALL ATT.
  25 NO HAY MENSAJE
  26 1ST GAMBLE OVER
  27 NO MESSAGE
  28 FOR HAND PAY PRESS COLLECT
  29 FREE GAME #1
  2A FREE GAME #2
  2B FREE GAME #3
  2C FREE GAME #4
  2D FREE GAME #5
  2E FREE GAME OVER
  2F FREE GANE OVER PLAY OR PRESS COLLECT FOR HAND PAY
  30 2ND GAMBLE OVER
  31 3RD GAMBLE OVER
  32 4TH GAMBLE OVER
  33 5TH GAMBLE OVER
  34 2ND GAMBLE OVER PLAY OR PRESS COLLECT FOR HAND PAY
  35 3RD GAMBLE OVER PLAY OR PRESS COLLECT FOR HAND PAY
  36 4TH GAMBLE OVER PLAY OR PRESS COLLECT FOR HAND PAY
  37 5TH GAMBLE OVER PLAY OR PRESS COLLECT FOR HAND PAY

***************************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "video/x1_001.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "hotchili.lh"


namespace {

// FIXME: this is the wrong frequency
#define MAIN_CLOCK XTAL(8'000'000)


class hotchili_state : public driver_device
{

public:
	hotchili_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_spritegen(*this, "spritegen")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_bank(*this, "bank")
		, m_ram(*this, RAM_TAG)
		, m_nvram(*this, "nvram")
		, m_inp0(*this, "IN0")
		, m_lamp(*this, "lamp%u", 0U)
	{
	}

	void hotchili(machine_config &config);
	void init_hc();


protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;

private:
	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void hc_palette(palette_device &palette) const;

	//external ram
	void extram_w(offs_t offset, uint8_t data);
	uint8_t extram_r(offs_t offset);
	uint8_t m_addr_high, m_addr_low;
	uint16_t m_addr_mask, m_addr_latch;

	void hc_map(address_map &map) ATTR_COLD;
	void bankswitch_w(uint8_t data);

	//inports
	uint8_t inport0_r();
	uint8_t inport3_r();

	//outports
	void outp1_w(offs_t offset, uint8_t data);
	void outp2_w(offs_t offset, uint8_t data);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<x1_001_device> m_spritegen;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_memory_bank m_bank;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;
	required_ioport m_inp0;
	output_finder<40> m_lamp;

	uint8_t m_meters = 0;
};


/*********************************************
*               Video Hardware               *
*********************************************/

void hotchili_state::hc_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int const col = (color_prom[i] << 8) | color_prom[i + 512];
		palette.set_pen_color(i, pal5bit(col >> 10), pal5bit(col >> 5), pal5bit(col >> 0));
	}
}

uint32_t hotchili_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	bitmap.fill(0x1f0, cliprect);

	m_spritegen->draw_sprites(screen, bitmap, cliprect, 0x1000);
	return 0;
}


/*********************************************
*               Bankswitching                *
*********************************************/

void hotchili_state::bankswitch_w(uint8_t data)
{
	m_bank->set_entry((data & 0x80) ? (data & 0x03) : 0);
}


/*********************************************
*           Memory Map Information           *
*********************************************/

void hotchili_state::hc_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).bankr("bank");
	map(0xa000, 0xafff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecodelow_r8), FUNC(x1_001_device::spritecodelow_w8));
	map(0xb000, 0xbfff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecodehigh_r8), FUNC(x1_001_device::spritecodehigh_w8));
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe2ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r8), FUNC(x1_001_device::spriteylow_w8));
	map(0xe300, 0xe303).ram().w(m_spritegen, FUNC(x1_001_device::spritectrl_w8));
	map(0xe800, 0xe800).w(m_spritegen, FUNC(x1_001_device::spritebgflag_w8));
	map(0xf000, 0xf00c).rw( FUNC(hotchili_state::extram_r), FUNC(hotchili_state::extram_w));
	map(0xf200, 0xf200).w(FUNC(hotchili_state::outp2_w));       // outport (mem img ca85h)
	map(0xf300, 0xf300).w(FUNC(hotchili_state::bankswitch_w));
	map(0xf400, 0xf40c).w(FUNC(hotchili_state::outp1_w));       // 4 outports 0,4,8,c (mem img ca81h - ca84h)
	map(0xf500, 0xf500).r(FUNC(hotchili_state::inport0_r));
	map(0xf501, 0xf501).portr("IN1");
	map(0xf600, 0xf600).portr("IN2");
	map(0xf601, 0xf601).r(FUNC(hotchili_state::inport3_r));
	map(0xf700, 0xf700).noprw();  // watchdog???
	map(0xf800, 0xf801).rw("ay8910", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
}


/*********************************************
*                Input Ports                 *
*********************************************/

static INPUT_PORTS_START( hotchili )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("All Clear / Configuration")  // pressed on startup
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unknown
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )  // active: enables RNG and read inputs on secondary buffer (inputs with special timing)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Meter Reading Key") PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_U) PORT_NAME("Jackpot Reset Key")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_6) PORT_NAME("Clear Att. Key")    //Next Page into Input or Output Test
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Collect")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("Reserve / Next (Met.Read)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )    // coin upper sensor - currently not used
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )    // coin lower sensor
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_CODE(KEYCODE_E) PORT_NAME("CoinToCashBox") // coin to cashbox
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE ) PORT_CODE(KEYCODE_R) PORT_NAME("CoinTest")      // coin test
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE ) PORT_CODE(KEYCODE_T) PORT_NAME("Test Select")   // test select
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_SERVICE ) PORT_CODE(KEYCODE_Y) PORT_NAME("Test End")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR ) PORT_TOGGLE

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Play 1 Credits / Change (Setup)") // 1st. Start
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Play 2 Credits")                  // 2nd. Start
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Play 3 Credits")                  // 3rd. Start
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Play 5 Credits")                  // 4th. Start
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Play 10 Credits / Next (Setup)")  // 5th. Start
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )     // unknown
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )     PORT_CODE(KEYCODE_J) PORT_NAME("Hopper Empty Reset Key")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE )     PORT_CODE(KEYCODE_K) PORT_NAME("Hopper CountSW")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_A) PORT_NAME("Play 1 Line")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_S) PORT_NAME("Play 3 Lines")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_D) PORT_NAME("Play 5 Lines")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_F) PORT_NAME("Play 7 Lines")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_G) PORT_NAME("Play 9 Lines")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*******************************************
*                   Inputs                 *
*******************************************/

uint8_t hotchili_state::inport0_r()
{
	if( (m_screen->frame_number() & 7) == 0)
	{
		return m_inp0->read() & 0xf9;  // enables alternate inputs read
	}
	else
	{
		return m_inp0->read();
	}
}


uint8_t hotchili_state::inport3_r()
{
	return m_meters;  // meters signal feedback
}


/*******************************************
*            Lamps and Outputs             *
*******************************************/

void hotchili_state::outp1_w(offs_t offset, uint8_t data)
{
	offset = (offset >> 2) & 0x03;
	switch( offset )
	{
		case 0:
			m_lamp[0] = BIT(data,0);  // 1st Start
			m_lamp[1] = BIT(data,1);  // 2nd Start
			m_lamp[2] = BIT(data,2);  // 3rd Start
			m_lamp[3] = BIT(data,3);  // 4th Start
			m_lamp[4] = BIT(data,4);  // 5th Start
			m_lamp[5] = BIT(data,5);
			m_lamp[6] = BIT(data,6);
			m_lamp[7] = BIT(data,7);  // Motor Hopper
			break;

		case 1:
			m_lamp[8]  = BIT(data,0);  // 1st Start - Test Mode
			m_lamp[9]  = BIT(data,1);  // 2nd Start
			m_lamp[10] = BIT(data,2);  // 3rd Start
			m_lamp[11] = BIT(data,3);  // 4th Start
			m_lamp[12] = BIT(data,4);  // 5th Start
			m_lamp[13] = BIT(data,5);  // Coin Lock Out Coil
			m_lamp[14] = BIT(data,6);  // Divert Solenoid
			m_lamp[15] = BIT(data,7);  // Divert Solenoid
		break;

		case 2:
			m_lamp[16] = BIT(data,0);  // alt door lamp?
			m_lamp[17] = BIT(data,1);  // Animation Lamp D
			m_lamp[18] = BIT(data,2);  // Animation Lamp C
			m_lamp[19] = BIT(data,3);  // Animation Lamp B
			m_lamp[20] = BIT(data,4);  // Animation Lamp A
			m_lamp[21] = BIT(data,5);  // Door Oper Tower
			m_lamp[22] = BIT(data,6);  // Call Attendat Tower
			m_lamp[23] = BIT(data,7);  // Jackpot Tower
		break;

		case 3:
			m_meters = data;
			m_lamp[24] = BIT(data,0); machine().bookkeeping().coin_counter_w(0, BIT(data, 0));  // Meter 1
			m_lamp[25] = BIT(data,1); machine().bookkeeping().coin_counter_w(1, BIT(data, 1));  // Meter 2
			m_lamp[26] = BIT(data,2); machine().bookkeeping().coin_counter_w(2, BIT(data, 2));  // Meter 3 - Credits Played
			m_lamp[27] = BIT(data,3); machine().bookkeeping().coin_counter_w(3, BIT(data, 3));  // Meter 4 - Credits Won
			m_lamp[28] = BIT(data,4); machine().bookkeeping().coin_counter_w(4, BIT(data, 4));  // Meter 5 - Credits to Cash Box
			m_lamp[29] = BIT(data,5); machine().bookkeeping().coin_counter_w(5, BIT(data, 5));  // Meter 6 - Cancelled Credits (Hand Pay)
			m_lamp[30] = BIT(data,6); machine().bookkeeping().coin_counter_w(6, BIT(data, 6));  // Meter 7 - Games
			m_lamp[31] = BIT(data,7); machine().bookkeeping().coin_counter_w(7, BIT(data, 7));  // Meter 8
		break;
	}
}

void hotchili_state::outp2_w(offs_t offset, uint8_t data)
{
	m_lamp[32] = BIT(data,0);  // 1st Line
	m_lamp[33] = BIT(data,1);  // 2nd Line
	m_lamp[34] = BIT(data,2);  // 3rd Line
	m_lamp[35] = BIT(data,3);  // 4th Line
	m_lamp[36] = BIT(data,4);  // 5th Line
	m_lamp[37] = BIT(data,5);
	m_lamp[38] = BIT(data,6);
	m_lamp[39] = BIT(data,7);
}


/*********************************************
*              Graphics Layouts              *
*********************************************/

static const gfx_layout charlayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};


/**************************************************
*           Graphics Decode Information           *
**************************************************/

static GFXDECODE_START( gfx_hotchili )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 32 )
GFXDECODE_END


/**************************************************
*          Universal External RAM Module          *
**************************************************/

void hotchili_state::extram_w(offs_t offset, uint8_t data)
{
	//logerror("external_memory_w:%x: data:%x\n", offset, data);
	offset = (offset >> 2) & 0x03;
	switch( offset )
	{
		case 0: m_addr_low = data;
		break;

		case 1: m_addr_high = data & 0x07;
		break;

		case 2: m_addr_mask = data; m_addr_latch = (m_addr_high * 0x100) + m_addr_low;
		break;

		case 3: m_ram->pointer()[m_addr_latch & 0x0fff] = data;
		break;
	}
}

uint8_t hotchili_state::extram_r(offs_t offset)
{
	//logerror("external_memory_r:%x\n", offset);
	offset = (offset >> 2) & 0x07;
	switch( offset )
	{
		case 0: return m_addr_low;

		case 1: return m_addr_high;

		case 2: return 0x3f; // mask

		case 3: return m_ram->pointer()[m_addr_latch & 0x0fff];

		default: return 0;
	}
}


/******************************************
*              Machine Start              *
******************************************/

void hotchili_state::machine_start()
{
	m_lamp.resolve();
	m_nvram->set_base(m_ram->pointer(), m_ram->size());
}


/*********************************************
*              Machine Drivers               *
*********************************************/

void hotchili_state::hotchili(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, MAIN_CLOCK / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &hotchili_state::hc_map);

	RAM(config, m_ram).set_default_size("2K").set_default_value(0);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw( MAIN_CLOCK / 2, 260, 0, 256, 256, 16, 239);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_screen_update(FUNC(hotchili_state::screen_update));
	m_screen->screen_vblank().set_inputline(m_maincpu, 0, HOLD_LINE);
	m_screen->set_palette(m_palette);

	X1_001(config, m_spritegen, 16'000'000, m_palette, gfx_hotchili);
	m_spritegen->set_fg_yoffsets( -0x12, 0x0e );
	m_spritegen->set_bg_yoffsets( 0x1, -0x1 );

	PALETTE(config, m_palette, FUNC(hotchili_state::hc_palette), 512);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	ay8910_device &ay8910(AY8910(config, "ay8910", MAIN_CLOCK / 4));
	ay8910.port_a_read_callback().set_ioport("DSW1");
	ay8910.port_b_read_callback().set_ioport("IN4");
	ay8910.add_route(ALL_OUTPUTS, "mono", 1.00);
}


/*********************************************
*                  Rom Load                  *
*********************************************/

ROM_START( hotchili )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "h.chilli_95103_v0191__27c512.ic32", 0x0000, 0x10000, CRC(54b7c675) SHA1(7ce0348ae9561784519cdb99e33a57922520941c) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "9510301_chr0_h.c.nsw.ic2",   0x00000, 0x10000, CRC(52faf9ae) SHA1(72050a3168f7326f39743e8424a0795b14b00d69) )
	ROM_LOAD( "9510301_chr1_h.c.nsw.ic1",   0x10000, 0x10000, CRC(f0ac0f98) SHA1(45a2f304758f2b1d701feb8db58b6137e58fed4c) )
	ROM_LOAD( "9510301_chr2_h.c.nsw.ic25",  0x20000, 0x10000, CRC(a946c2e4) SHA1(7530aebd5c5204bb7aa091acd108b0cd00ac272b) )
	ROM_LOAD( "9510301_chr3_h.c.nsw.ic26",  0x30000, 0x10000, CRC(69dc0f95) SHA1(7b613a5d1f2d431b178fc105c2023ec8bbf3a873) )

	ROM_REGION( 0x0800, "nvram", 0 )
	ROM_LOAD( "hotchili_nvram.bin",  0x0000, 0x0800, CRC(e2b463e5) SHA1(a94ae3888858173aed53e54cceb951a9fe8b7a20) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "vn55-1.ic8",  0x0000, 0x0200, CRC(3acb6539) SHA1(f202b2403acf1c6e7abc61e860a75aef318ab03c) )
	ROM_LOAD( "vn55-2.ic9",  0x0200, 0x0200, CRC(e1d2897e) SHA1(2e8ff5041dfd4b69488f0d580645564bd523fc10) )
ROM_END


/*********************************************
*                Driver Init                 *
*********************************************/

void hotchili_state::init_hc()
{
	uint8_t *ROM = memregion("maincpu")->base();
	m_bank->configure_entries(0, 4, &ROM[0x8000], 0x2000);

	ROM[0x05bc] = 0x00;  // Avoids ram error flag setup
	ROM[0x06c1] = 0x20;  // Skip Rom Error
	ROM[0x06c4] = 0xc6;  // Skip Ram Error
	ROM[0x06d1] = 0xd3;  // Skip Ram Error
	ROM[0x06d2] = 0x06;  // Skip Ram Error
	ROM[0x1c54] = 0x84;  // Avoids meter error
	ROM[0x1c5b] = 0x84;  // Avoids meter error
}

} // anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//     YEAR  NAME      PARENT  MACHINE   INPUT     STATE           INIT        ROT    COMPANY                    FULLNAME                    FLAGS   LAYOUT
GAMEL( 1995, hotchili, 0,      hotchili, hotchili, hotchili_state, init_hc,    ROT0, "Pacific Gaming Pty Ltd.", "Hot Chilli (95103, v0104)", 0     , layout_hotchili )
