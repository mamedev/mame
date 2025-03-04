// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Roberto Fresca
/**************************************************************************************************************

  Challenger Draw Poker (c) 198? Videotronics.
  Driver by Angelo Salese & Roberto Fresca.


  Notes:

  I found two companies that sold the same game with different name:

    1) "Challenger Draw Poker", from Bend Electronics Co. Inc.
    2) "VHI Draw Poker", from Video Horizons, Inc.

    Both companies shared the same address and phone number:
     63353 Nels Anderson Road. Bend, Oregon 97701.
     Tel: 503-389-7626.

    Bend Electronics Co. Inc. claims that they are worldwide distributors for Videotronics, Inc.

    There are some legal issues between all these companies...
    https://scholar.google.com/scholar_case?case=7993095852400122011
    http://www.plainsite.org/dockets/201rtodjb/nevada-district-court/videotronics-inc-v-bend-electronics/


===============================================================================================================

  Bought as "old poker game by videotronics early 80's"

  Scratched on the CPU board  SN1069
  Scratched on the CPU board  SN1069

  CPU board

  .0  2716    stickered   DRAWPKR2  8-F  REV A    located top left
  .1  2716    stickered   DRAWPKR2  0-7  REV A    located next to .0

  ROM board

  Top of board left to right

  .R0 2716    stickered   RA  0-7
  .R1 2716    stickered   RA  8-F
  .R2 2716    stickered   BA  0-7
  .R3 2716    stickered   BA  8-F
  .R4 2716    stickered   GA  0-7
  .R5 2716    stickered   GA  8-F

  Below top row left to right

  .R6  2716    stickered   RB  0-7
  .R7  2716    stickered   RB  8-F
  .R8  2716    stickered   BB  0-7
  .R9  2716    stickered   BB  8-F
  .R10 2716    stickered   GB  0-7
  .R11 2716    stickered   GB  8-F

  ROM data showed cards

  MC6809 CPU
  4.000 Mhz crystal
  MC6840P
  mm74c920J   x2
  mmc6551j-9  x2


===============================================================================================================

 - Added 5-Aces Poker (Roberto Fresca)

  .1 is closest to the connector
  .7 is closest to the cpu

  Etched in copper on top by .1 eprom 6000

  .1  2732    handwritten sticker GJOK1
  .2  2732    handwritten sticker GJOK2
  .3  2732    handwritten sticker BJOK1
  .4  2732    handwritten sticker BJOK2
  .5  2732    handwritten sticker RJOK1
  .6  2732    handwritten sticker RJOK2
  .7  2764    handwritten sticker 688C

  4 MHz crystal
  HD68A09P
  MC6840P
  nmc6514-9   x2
  nm23114     x2

  16 pin chip marked  74166F 7745
                      SA2889-0697
             stamped     ETC


===============================================================================================================

  Driver notes:

  meters offsets:

  Aces:           16a-16b
  2 Pair:         16c-16d
  3 of a Kind:    16e-16f
  Straight:       170-171
  Flush:          172-173
  Full House:     173-174
  4 of a Kind:    175-176
  Straight Flush: 177-178

  Coins In:       188-189-18a-18b
  Credits Played: 18c-18d-18e-18f
  Credits Won:    190-191-192-193
  Cleared:        180-181-182-183
  Games Played:   194-195-196-197
  Games Won:      184-185-186-187


  registers:
  
  Credits:        da-db
  Coins in:       dd


===============================================================================================================

  Updates [2025-03-04]:

  - Change vpoker description to Challenger Draw Poker.
  - Inputs from the scratch.
  - Split machine drivers for each game.
  - Hook clocks to the 6840 PTM.
  - Reworked interrupts handling.
  - Lamps support for both sets.
  - Adjusted screen visible area per game.
  - Clickable button-lamps layout for vpoker.
  - Clickable button-lamps layout for 5acespkr.
  - Mech counters support.
  - NVRAM support.
  - Added sigma-delta DAC sound support.
  - Promoted vpoker to working.
  - Promoted 5acespkr to working.
  - Added technical notes.


  TODO:

  - Check NVRAM issues.
  - Find why vectors are changed in 5acespkr.
  - Investigate about what seems to be a custom processor
    due to the weird routines related to interrupts, and the
    complete lack of SWI triggers.	

  
**************************************************************************************************************/


#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6840ptm.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "vpoker.lh"
#include "5acespkr.lh"


namespace {

class vpoker_state : public driver_device
{
public:
	vpoker_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_dac(*this, "dac"),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_lamps(*this, "lamp%u", 0U)

	{ }

	void vpoker(machine_config &config);
	void fiveaces(machine_config &config);
	void init_5aces();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	std::unique_ptr<uint8_t[]> m_videoram;
	uint8_t m_blit_ram[8];
	uint8_t blitter_r(offs_t offset);
	void blitter_w(offs_t offset, uint8_t data);
	void ptm_irq(int state);
	void swi_int(int state);
	uint32_t screen_update_vpoker(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<dac_bit_interface> m_dac;
	required_ioport m_in0;
	required_ioport m_in1;
	output_finder<9> m_lamps;
	void main_map(address_map &map) ATTR_COLD;
};


/***********************************
*          Video Hardware          *
***********************************/

void vpoker_state::video_start()
{
	m_videoram = std::make_unique<uint8_t[]>(0x200);
	std::fill(std::begin(m_blit_ram), std::end(m_blit_ram), 0);
}

uint32_t vpoker_state::screen_update_vpoker(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *videoram = m_videoram.get();
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int count = 0x0000;

	for (int y = 0; y < 0x10; y++)
	{
		for (int x = 0; x < 0x20; x++)
		{
			int tile = videoram[count];
			//int colour = tile >> 12;
			gfx->opaque(bitmap, cliprect, tile, 0, 0, 0, x*16, y*16);

			count++;
		}
	}

	return 0;
}

uint8_t vpoker_state::blitter_r(offs_t offset)
{
	if(offset == 6)
		return m_in0->read();

	if(offset == 7)
		return m_in1->read();

	return 0;
}

void vpoker_state::blitter_w(offs_t offset, uint8_t data)
{
	uint8_t *videoram = m_videoram.get();

	m_blit_ram[offset] = data;
	
	if(offset == 1)
	{
		machine().bookkeeping().coin_counter_w(0, BIT(data, 5));  // coin_in
		machine().bookkeeping().coin_counter_w(1, BIT(data, 4));  // coin_out

		m_lamps[7] = BIT(data, 7);  // bet lamp
	}
	if(offset == 2)
	{
		int blit_offs;

		blit_offs = (m_blit_ram[1] & 0x01) << 8 | (m_blit_ram[2] & 0xff);
		videoram[blit_offs] = m_blit_ram[0];
	}
	if(offset == 3)
	{
		m_lamps[0] = BIT(data, 0);  // hold 1 lamp
		m_lamps[1] = BIT(data, 1);  // hold 2 lamp
		m_lamps[2] = BIT(data, 2);  // hold 3 lamp
		m_lamps[3] = BIT(data, 3);  // hold 4 lamp
		m_lamps[4] = BIT(data, 4);  // hold 5 lamp
		m_lamps[5] = BIT(data, 5);  // deal lamp
		m_lamps[6] = BIT(data, 6);  // draw lamp
	}
	else
	{
		// logerror("blitter_w: offs:%02x - data:%02x\n", offset, data);
	}
}


/***********************************
*          Machine Start           *
***********************************/

void vpoker_state::machine_start()
{
	m_lamps.resolve();
}


/*****************************************
*         Memory Map Information         *
*****************************************/

void vpoker_state::main_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x01ff).ram().share("nvram");     // vpoker has 0x100, 5acespkr has 0x200
	map(0x0400, 0x0407).rw("6840ptm", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0x0800, 0x0807).r(FUNC(vpoker_state::blitter_r)).w(FUNC(vpoker_state::blitter_w));
	map(0x2000, 0x3fff).rom();
}


/*****************************************
*              Input Ports               *
*****************************************/

static INPUT_PORTS_START( vpoker )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_NAME("Coin In") PORT_IMPULSE(3) 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )   
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK )   // PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_Q) PORT_NAME("IN0-10")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_E) PORT_NAME("IN0-20")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_R) PORT_NAME("IN0-40")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_T) PORT_NAME("IN0-80")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )        PORT_NAME("Deal")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )   PORT_NAME("Draw")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
INPUT_PORTS_END


static INPUT_PORTS_START( 5acespkr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_NAME("Coin In") PORT_IMPULSE(2) 
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )                                         // setu change value (up/off)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )   // PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_Q) PORT_NAME("IN0-10")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )                     PORT_NAME("Settings") // setup
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_R) PORT_NAME("IN0-40")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_T) PORT_NAME("IN0-80")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )        PORT_NAME("Deal")  // setup change value (down/up-1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )   PORT_NAME("Draw")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_CANCEL )                     // setup netx item
INPUT_PORTS_END


/*****************************************
*            Graphics Layouts            *
*****************************************/

static const gfx_layout charlayout =
{
	16,16,
	RGN_FRAC(1,4),
	3,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4),RGN_FRAC(2,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 , 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16  },
	16*16
};


/*****************************************
*      Graphics Decode Information       *
*****************************************/

static GFXDECODE_START( gfx_vpoker )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,  0, 1 )
GFXDECODE_END


/*****************************************
*     PTM 6840 interrupts handling       *
*****************************************/

void vpoker_state::ptm_irq(int state)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

void vpoker_state::swi_int(int state)
{
	if(m_in0->read() == 0xfe) 
		m_maincpu->set_input_line(M6809_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}



/*****************************************
*             Machine Driver             *
*****************************************/

void vpoker_state::vpoker(machine_config &config)
{
	// basic machine hardware
	MC6809(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &vpoker_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(vpoker_state::irq0_line_assert));
	
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(512, 256);
	screen.set_visarea(0, 464-1, 0, 240-1);  // 512x256 total
	screen.set_screen_update(FUNC(vpoker_state::screen_update_vpoker));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_vpoker);

	PALETTE(config, m_palette, palette_device::GBR_3BIT);

	// 6840 PTM
	ptm6840_device &ptm(PTM6840(config, "6840ptm", XTAL(4'000'000) / 4));
	ptm.set_external_clocks(500, 0000, 3000);
	ptm.irq_callback().set(FUNC(vpoker_state::ptm_irq));
	ptm.o2_callback().set("dac", FUNC(dac_1bit_device::data_w));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5);
}


void vpoker_state::fiveaces(machine_config &config)
{
	// basic machine hardware
	MC6809(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &vpoker_state::main_map);
	
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(30);
	screen.screen_vblank().set(FUNC(vpoker_state::swi_int));
	screen.set_size(512, 256);
	screen.set_visarea(48, 448-1, 0, 240-1);  // 512x256 total
	screen.set_screen_update(FUNC(vpoker_state::screen_update_vpoker));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_vpoker);

	PALETTE(config, m_palette, palette_device::GBR_3BIT);

	// 6840 PTM
	ptm6840_device &ptm(PTM6840(config, "6840ptm", XTAL(4'000'000) / 4));
	ptm.set_external_clocks(500, 0, 1000000);
	ptm.irq_callback().set(FUNC(vpoker_state::ptm_irq));
	ptm.o2_callback().set("dac", FUNC(dac_1bit_device::data_w));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5);
}


/*****************************************
*                Rom Load                *
*****************************************/

ROM_START( vpoker )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vpoker.1",   0x2000, 0x0800, CRC(790f3c4e) SHA1(c60485cc44dd742a4a9398b98c2bde8a95f625f3) )
	ROM_RELOAD(             0x3000, 0x0800 )
	ROM_LOAD( "vpoker.0",   0x2800, 0x0800, CRC(8ad8ce66) SHA1(84b606ab9698b957b631070296a9e6e64fabdd8a) )
	ROM_RELOAD(             0x3800, 0x0800 )

	ROM_REGION( 0x8000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "vpoker.r0",  0x0000, 0x0800, CRC(1581202c) SHA1(7882fde76d0529fbfdd1235a39d04333e83f8a2f) )
	ROM_LOAD16_BYTE( "vpoker.r1",  0x1000, 0x0800, CRC(27695350) SHA1(09d1e0e6d5d823f091fa941a96f7c5f045501145) )
	ROM_LOAD16_BYTE( "vpoker.r2",  0x2000, 0x0800, CRC(1c0eab90) SHA1(19f3110088124f73de980502aab374888924d6a5) )
	ROM_LOAD16_BYTE( "vpoker.r3",  0x3000, 0x0800, CRC(7a8cb6f9) SHA1(d233f0f592c22dab6827e34c2cb22dd301a054e1) )
	ROM_LOAD16_BYTE( "vpoker.r4",  0x4000, 0x0800, CRC(755c4f02) SHA1(d19db1b1b2d41643cb69bb6eed46b1851de384c9) )
	ROM_LOAD16_BYTE( "vpoker.r5",  0x5000, 0x0800, CRC(ccd32805) SHA1(fdff53942f06b5fc7a292364afb98721369cc0f4) )
	ROM_LOAD16_BYTE( "vpoker.r6",  0x0001, 0x0800, CRC(77860770) SHA1(bffc8f38e9f63518706c093afd9254be8e15773d) )
	ROM_LOAD16_BYTE( "vpoker.r7",  0x1001, 0x0800, CRC(1ca9e74e) SHA1(3a2e71fb2f21acfa864dda4e459f7f150bddb988) )
	ROM_LOAD16_BYTE( "vpoker.r8",  0x2001, 0x0800, CRC(68022a42) SHA1(72a924a8ecf327821e444c5fb3ddd62510d4fc13) )
	ROM_LOAD16_BYTE( "vpoker.r9",  0x3001, 0x0800, CRC(5a71f01c) SHA1(e86a40e0533b24e66a2245e97670f131bd68be06) )
	ROM_LOAD16_BYTE( "vpoker.r10", 0x4001, 0x0800, CRC(5e0a7011) SHA1(9981f080581ef97f482e9a4b4ea0447c8bf89fc8) )
	ROM_LOAD16_BYTE( "vpoker.r11", 0x5001, 0x0800, CRC(960b1e05) SHA1(c692835f3cd0be6c221623c3955977ba6d8fd0cf) )
ROM_END


ROM_START( 5acespkr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "688c.bin",   0x2000, 0x2000, CRC(34ad0bcb) SHA1(d25e2d52896edaa8e9d2720685eb150fd0bcd9ee) )

	ROM_REGION( 0x8000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "rjok2.bin",  0x0000, 0x1000, CRC(88712bb4) SHA1(f36aba6f8184c7e01caed875696f01cd281b682b) )
	ROM_LOAD16_BYTE( "rjok1.bin",  0x0001, 0x1000, CRC(a88ba6a1) SHA1(61165f2afcd174878705dfc487935ee54f45a014) )
	ROM_LOAD16_BYTE( "gjok2.bin",  0x4000, 0x1000, CRC(1f9e25a0) SHA1(19756b0e99c052f0a87f042bfca1aca0b7aa78db) )
	ROM_LOAD16_BYTE( "gjok1.bin",  0x4001, 0x1000, CRC(da0c0a33) SHA1(d5c09965ea4f01082c87520a1fce5a39fef6e8e1) )
	ROM_LOAD16_BYTE( "bjok2.bin",  0x2000, 0x1000, CRC(d845f8a1) SHA1(fb050e72164662c2f5670f59a8ad43d19c0485ea) )
	ROM_LOAD16_BYTE( "bjok1.bin",  0x2001, 0x1000, CRC(20cdda67) SHA1(6c631b09e3da5f6660aa1c018fc0ff3004f7fe85) )
ROM_END


/*****************************************
*              Driver Init               *
*****************************************/

void vpoker_state::init_5aces()
{
//  Changing the vector temporarily for testing purposes,
//  otherwise the coin in routine will be tied to the
//  SW2 interrupt, and there is no code to trigger it.

	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0x3ffa] = 0x79;
}


} // Anonymous namespace


/*****************************************
*              Game Drivers              *
*****************************************/

//     YEAR  NAME      PARENT  MACHINE   INPUT     STATE         INIT        ROT   COMPANY               FULLNAME                  FLAGS                          LAYOUT
GAMEL( 198?, vpoker,   0,      vpoker,   vpoker,   vpoker_state, empty_init, ROT0, "Videotronics, Inc.", "Challenger Draw Poker",  0,                             layout_vpoker  )
GAMEL( 198?, 5acespkr, 0,      fiveaces, 5acespkr, vpoker_state, init_5aces, ROT0, "<unknown>",          "5-Aces Poker",           MACHINE_UNEMULATED_PROTECTION, layout_5acespkr  )
