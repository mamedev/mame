/*

   Igrosoft gambling hardware

   +--+ +-----+ +-----------------------------------+
+--+  +-+PRINT+-+                                   |
|            +--------------+                       |
|            |     Z80B     |                       |
|      VOL   +----+---------+                       |
+-+ +-----------+ | PRG ROM |                       |
  | |  KC89C72  | +---------+                       |
+-+ +-----------+ | M48T35Y |  24MHz                -+
|                 +---------+                ADM690 V|
|                |---Connector------------------|   G|
|E                                                  A|
|d                                                  -+
|g                                                  |
|e                                                  |
|                                                   |
|C                                                  |
|o                                                  |
|n                                                  |
|n                                                  |
|e                                                  |
|c                           Connector              |
|t               |---Connector------------------|   |
|o           +------+ +------+ +------+ +------+    |
|r           |ALTERA| |ALTERA| |ALTERA| |ALTERA| R  |
|            | EPM  | | EPM  | | EPM  | | EPM  | A  |
|            | 3032 | | 3032 | | 3032 | | 3032 | M  |
|            +------+ +------+ +------+ +------+    |
+-+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ |
  | |   | |   | |   | |   | |   | |   | |   | |   | |
+-+ | 8 | | 7 | | 6 | | 5 | | 4 | | 3 | | 2 | | 1 | |
|   |   | |   | |   | |   | |   | |   | |   | |   | |
|   |   | |   | |   | |   | |   | |   | |   | |   | |
|   +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ |
+---------------------------------------------------+

"Head" sub board:
+------------------------------+
||--Connector-----------------||
|            61256AK-15        |
|                              |
|          +---------+         |
|          | ALTERA  |         |
|          |   MAX   |         |
|          | EPM3256 |         |
|          |         |         |
|          +---------+         |
|                              |
|           Connector          |
||--Connector-----------------||
+------------------------------+

  CPU: Z80
Video: ALTERA EPM3032ALC44-10 (x4) + ALTERA MAX EPM3256AOC208-10 (on the HEAD sub board)
       later board revisions comes with EPM3064 and EPM3512 ALTERA's
Sound: File KC98C72 (compatible to YM2149 / AY3-9810)
  OSC: 24MHz
  RAM: UMC UM61256AK-15 (x2 on mainboard, one on the HEAD)
NVRAM: Timekeeper M48T35-70PC1 (used as main Z80 RAM)
Other: ADM ADM690AN (Microprocessor Supervisory Circuit AKA Watchdog timer)
       VGA connector for video output

Edge Connector is 36 count dual (IE: both sides) JAMMA-like connection (standard 8-liner??)

  RAM has E000 address

  To Init the games

  Turn Service Mode ON (press 'F2')
  Reset the game (press 'F3')
  Use 'C' to move pointer to INIT
  Press '1' (Start) to enter INIT menu
  Hold 'Z' (Bet/Double) for 5 seconds while counter counts down
  Turn Service Mode OFF (press 'F2')
  Reset the game (press 'F3')

  To Init Roll Fruit

  Turn Service Mode ON (press 'F2')
  Press and hold Service 1 ('9')
  Reset the game (press 'F3')
  Use Service 2 ('0') to move pointer to INIT
  Hold Service 1 ('9') for 5 seconds
  Turn Service Mode OFF (press 'F2')

  Todo:
  -------------------------------------------------------------------------
  Layouts

  NOTE:
  Revision information comes from Igrosoft's website, not all of them can be
  tested because some program rom revisions don't seem to be dumped.

  For sets where the program rom hasn't been verified, the SHA1 hash is given
  but not the CRC32 hash.

Banking addresses are likely controlled via a GAL/PAL and was added at some
point to try and prevent rom swaps and conversions. Many of the hacked sets
below are simply made to the banking address to run on other boards.

  Bank addresses
  ---------------------
  Island 2           E1
  Gnome              E5
  Sweet Life 2       E8
  Fruit Cocktail 2   EA
  Multi Fish         F8
  Crazy Monkey       F9
  Fruit Cocktail     F9
  Garage             F9
  Resident           F9
  Lucky Hunter       F9
  Rock Climber       F9
  Roll Fruit         F9
  Pirate             FA
  Sweet Life         FA
  Island             FB
  Keks               FC
  Pirate 2           FD


  Edge Connector pinout

+-------------------------+--------------------------+
|     COMPONENT SIDE      |     SOLDER SIDE          |
+---+-----------------+---+---+------------------+---+
|   |                 |1A |1B |                  |   |
|   |                 |2A |2B |                  |   |
|Out|SPEAKER          |3A |3B |GND               |   |
|In |HOLD 1 SW        |4A |4B |BILL ACCEPTOR 1   |In |
|In |HOLD 2 SW        |5A |5B |BILL ACCEPTOR 2   |In |
|In |HOLD 3 SW        |6A |6B |BILL ACCEPTOR 3   |In |
|In |HOLD 4 SW        |7A |7B |BILL ACCEPTOR 4   |In |
|In |HOLD 5 SW        |8A |8B |HOPPER INHIBIT*   |In |
|In |START SW         |9A |9B |                  |In |
|In |BET/DOUBLE SW    |10A|10B|                  |In |
|In |reserved         |11A|11B|                  |In |
|In |reserved         |12A|12B|                  |In |
|In |reserved         |13A|13B|                  |In |
|In |MAXBET SW        |14A|14B|                  |In |
|In |HELP SW          |15A|15B|                  |In |
|In |FRONT DOOR SW    |16A|16B|                  |In |
|In |BACK DOOR SW     |17A|17B|                  |In |
|In |COIN A           |18A|18B|COIN B            |In |
|In |COIN C           |19A|19B|COIN D            |In |
|In |STATISTIC SW     |20A|20B|SERVICE SW        |In |
|In |PAY OUT SW*      |21A|21B|KEY OUT SW        |In |
|Out|                 |22A|22B|HOPPER COIN SW    |In |
|Out|COIN+BILL COUNTER|23A|23B|COIN LOCK         |Out|
|Out|KEY IN COUNTER   |24A|24B|BILL ACCEPTOR LOCK|Out|
|Out|TOTAL IN COUNTER |25A|25B|UPPER LAMP GREEN  |Out|
|Out|TOTAL OUT COUNTER|26A|26B|UPPER LAMP RED    |Out|
|Out|KEY OUT COUNTER  |27A|27B|UPPER LAMP YELLOW |Out|
|Out|                 |28A|28B|TOTAL BET COUNTER |Out|
|Out|HOLD 1 LAMP      |29A|29B|BET / DOUBLE LAMP |Out|
|Out|HOLD 2 LAMP      |30A|30B|MAXBET LAMP       |Out|
|Out|HOLD 3 LAMP      |31A|31B|PAYOUT LAMP       |Out|
|Out|HOLD 4 LAMP      |32A|32B|                  |Out|
|Out|HOLD 5 LAMP      |33A|33B|HOPPER MOTOR      |Out|
|Out|START LAMP       |34A|34B|HELP LAMP         |Out|
|In |KEY IN SW        |35A|35B|                  |In |
|   |GND              |36A|36B|GND               |   |
+---+-----------------+---+---+------------------+---+
*/

#include "emu.h"
#include "sound/ay8910.h"
#include "cpu/z80/z80.h"
#include "machine/timekpr.h"

#define multfish_ROM_SIZE 0x80000
#define multfish_VIDRAM_SIZE (0x2000*0x04)

class multfish_state : public driver_device
{
public:
	multfish_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* Video related */

	int m_disp_enable;
	int m_xor_paltype;
	int m_xor_palette;

	tilemap_t *m_tilemap;
	tilemap_t *m_reel_tilemap;

	/* Misc related */

	UINT8 m_rambk;

	UINT8 m_hopper_motor;
	UINT8 m_hopper;

	UINT8 m_vid[multfish_VIDRAM_SIZE];
	DECLARE_WRITE8_MEMBER(multfish_vid_w);
	DECLARE_WRITE8_MEMBER(multfish_bank_w);
	DECLARE_READ8_MEMBER(bankedram_r);
	DECLARE_WRITE8_MEMBER(bankedram_w);
	DECLARE_WRITE8_MEMBER(multfish_rambank_w);
	DECLARE_READ8_MEMBER(ray_r);
	DECLARE_WRITE8_MEMBER(multfish_hopper_w);
	DECLARE_WRITE8_MEMBER(rollfr_hopper_w);
	DECLARE_WRITE8_MEMBER(multfish_lamps1_w);
	DECLARE_WRITE8_MEMBER(multfish_lamps2_w);
	DECLARE_WRITE8_MEMBER(multfish_lamps3_w);
	DECLARE_WRITE8_MEMBER(multfish_counters_w);
	DECLARE_WRITE8_MEMBER(multfish_f3_w);
	DECLARE_WRITE8_MEMBER(multfish_dispenable_w);
	DECLARE_CUSTOM_INPUT_MEMBER(multfish_hopper_r);
};

static TILE_GET_INFO( get_multfish_tile_info )
{
	multfish_state *state = machine.driver_data<multfish_state>();
	int code = state->m_vid[tile_index*2+0x0000] | (state->m_vid[tile_index*2+0x0001] << 8);
	int attr = state->m_vid[tile_index*2+0x1000] | (state->m_vid[tile_index*2+0x1001] << 8);

	tileinfo.category = (attr&0x100)>>8;

	SET_TILE_INFO(
			0,
			code&0x1fff,
			attr&0x7,
			0);
}

static TILE_GET_INFO( get_multfish_reel_tile_info )
{
	multfish_state *state = machine.driver_data<multfish_state>();
	int code = state->m_vid[tile_index*2+0x2000] | (state->m_vid[tile_index*2+0x2001] << 8);

	SET_TILE_INFO(
			0,
			(code&0x1fff)+0x2000,
			(code>>14)+0x8,
			0);
}

static VIDEO_START(multfish)
{
	multfish_state *state = machine.driver_data<multfish_state>();

	memset(state->m_vid,0x00,sizeof(state->m_vid));
	state->save_item(NAME(state->m_vid));

	state->m_tilemap = tilemap_create(machine,get_multfish_tile_info,tilemap_scan_rows,16,16, 64, 32);
	state->m_tilemap->set_transparent_pen(255);

	state->m_reel_tilemap = tilemap_create(machine,get_multfish_reel_tile_info,tilemap_scan_rows,16,16, 64, 64);
	state->m_reel_tilemap->set_transparent_pen(255);
	state->m_reel_tilemap->set_scroll_cols(64);
}

static SCREEN_UPDATE_IND16(multfish)
{
	multfish_state *state = screen.machine().driver_data<multfish_state>();
	int i;

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	if (!state->m_disp_enable) return 0;

	/* Draw lower part of static tilemap (low pri tiles) */
	state->m_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1),0);

	/* Setup the column scroll and draw the reels */
	for (i=0;i<64;i++)
	{
		int colscroll = (state->m_vid[i*2] | state->m_vid[i*2+1] << 8);
		state->m_reel_tilemap->set_scrolly(i, colscroll );
	}
	state->m_reel_tilemap->draw(bitmap, cliprect, 0,0);

	/* Draw upper part of static tilemap (high pri tiles) */
	state->m_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_CATEGORY(0),0);

	return 0;
}

WRITE8_MEMBER(multfish_state::multfish_vid_w)
{

	m_vid[offset]=data;

	// 0x0000 - 0x1fff is normal tilemap
	if (offset < 0x2000)
	{
		m_tilemap->mark_tile_dirty((offset&0xfff)/2);

	}
	// 0x2000 - 0x2fff is for the reels
	else if (offset < 0x4000)
	{
		m_reel_tilemap->mark_tile_dirty((offset&0x1fff)/2);
	}
	else if (offset < 0x6000)
	{
		int r,g,b;
		int coldat;

		coldat = m_vid[(offset&0xfffe)] | (m_vid[(offset&0xfffe)^1] << 8);

		/* xor and bitswap palette */
		switch (m_xor_paltype) {
			case 1:
				coldat ^= m_xor_palette;
				coldat ^= ((coldat&0x2) >>1) | ((coldat&0x80) >>3) ;
				coldat = BITSWAP16(coldat,10,15,5,13,8,12,11,2,0,4,7,14,9,3,1,6);
				break;
			case 2:
	                        coldat ^= m_xor_palette;
        	                coldat ^= ((coldat&0x0001) <<1) ^ ((coldat&0x0010) <<1) ^ ((coldat&0x0010) <<2) ^ ((coldat&0x0020) <<1) ^ ((coldat&0x0080) >>1);
                	        coldat = BITSWAP16(coldat,4,10,13,14,8,11,15,12,2,6,5,0,7,3,1,9);
				break;
		}
		r = ( (coldat &0x001f)>> 0);
		g = ( (coldat &0x1f00)>> 8);
		b = ( (coldat &0x00e0)>> (5));
		b|= ( (coldat &0xe000)>> (8+5-3));

		palette_set_color_rgb(machine(), (offset-0x4000)/2, r<<3, g<<3, b<<2);
	}
	else
	{
		// probably just work ram
	}
}

WRITE8_MEMBER(multfish_state::multfish_bank_w)
{
	memory_set_bank(machine(), "bank1", data & 0x0f);
}

static READ8_DEVICE_HANDLER( multfish_timekeeper_r )
{
	return timekeeper_r(device, offset + 0x6000);
}

static WRITE8_DEVICE_HANDLER( multfish_timekeeper_w )
{
	timekeeper_w(device, offset + 0x6000, data);
}

READ8_MEMBER(multfish_state::bankedram_r)
{

	if ((m_rambk & 0x80) == 0x00)
	{
		return timekeeper_r(machine().device("m48t35"), offset + 0x2000*(m_rambk & 0x03));
	}
	else
	{
		return m_vid[offset+0x2000*(m_rambk & 0x03)];
	}

}

WRITE8_MEMBER(multfish_state::bankedram_w)
{

	if ((m_rambk & 0x80) == 0x00)
	{
		timekeeper_w(machine().device("m48t35"), offset + 0x2000*(m_rambk & 0x03), data);
	}
	else
	{
		multfish_vid_w(space, offset+0x2000*(m_rambk & 0x03), data);
	}
}

WRITE8_MEMBER(multfish_state::multfish_rambank_w)
{

	m_rambk = data;
}


READ8_MEMBER(multfish_state::ray_r)
{
	// the games read the raster beam position as part of the hardware checks..
	// with a 6mhz clock and 640x480 resolution this seems to give the right results.
	return machine().primary_screen->vpos();
}

CUSTOM_INPUT_MEMBER(multfish_state::multfish_hopper_r)
{

	if ( m_hopper_motor != 0 )
	{
			m_hopper++;
			return m_hopper>>4;
	}
	else
	{
			return 0;
	}
}

WRITE8_MEMBER(multfish_state::multfish_hopper_w)
{
/*  Port 0x33

    7654 3210
    ---- ---X Coin Lock 23B
    ---- -X-- Bill Acceptor Lock 24B
    ---X ---- Hopper Motor 33B
*/


	m_hopper_motor = data & 0x10;
        coin_lockout_w(machine(), 0, data & 0x01);
        coin_lockout_w(machine(), 1, data & 0x01);
        coin_lockout_w(machine(), 2, data & 0x01);
        coin_lockout_w(machine(), 3, data & 0x01);
        coin_lockout_w(machine(), 4, data & 0x04);
        coin_lockout_w(machine(), 5, data & 0x04);
        coin_lockout_w(machine(), 6, data & 0x04);
        coin_lockout_w(machine(), 7, data & 0x04);
}

WRITE8_MEMBER(multfish_state::rollfr_hopper_w)
{
/*
    By default RollFruit use inverted coinlock bit.
*/


	m_hopper_motor = data & 0x10;
        coin_lockout_w(machine(), 0, !data & 0x01);
        coin_lockout_w(machine(), 1, !data & 0x01);
        coin_lockout_w(machine(), 2, !data & 0x01);
        coin_lockout_w(machine(), 3, !data & 0x01);
        coin_lockout_w(machine(), 4, data & 0x04);
        coin_lockout_w(machine(), 5, data & 0x04);
        coin_lockout_w(machine(), 6, data & 0x04);
        coin_lockout_w(machine(), 7, data & 0x04);
}

DRIVER_INIT( customl )
{
/*
rom 1
 swap msb/lsb
D0 = D0 xor A15
D1 = D1 xor A14
D2 = D2 xor A07
D3 = D3 xor A06

rom 3
 swap msb/lsb
D0 = D0 xor A16
D1 = D1 xor A17
D2 = D2 xor A08
D3 = D3 xor A09

rom 2,4
msb = msb xor lsb

All roms address lines swapped:
A06 <-> A15
A07 <-> A14
A08 <-> A16
A09 <-> A17
A10 <-> A18
A12 <-> A13
*/

	UINT32 i,j,jscr,romoffset;
	UINT8 *multfish_gfx = machine.region("gfx")->base();
	UINT8 *temprom = auto_alloc_array(machine, UINT8, multfish_ROM_SIZE);


	/* ROM 1 decode */
	romoffset = 0x000000;
	for (i = 0; i < multfish_ROM_SIZE; i++)
	{
		jscr = ((i & 0x8000)>>15) | ((i & 0x4000)>>13) | ((i & 0x0080)>>5) | ((i & 0x0040)>>3);
		multfish_gfx[romoffset+i] = (((0x0f & multfish_gfx[romoffset+i])<<4) | ((0xf0 & multfish_gfx[romoffset+i])>>4)) ^ jscr;
	}
        /* ROM 2 decode */
	romoffset = 0x100000;
	for (i = 0; i < multfish_ROM_SIZE; i++)
	{
		multfish_gfx[romoffset+i] ^=  (0x0f & multfish_gfx[romoffset+i])<<4;
	}
	/* ROM 3 decode */
	romoffset = 0x200000;
	for (i = 0; i < multfish_ROM_SIZE; i++)
	{
		jscr = ((i & 0x300)>>6) | ((i & 0x30000)>>16);
		multfish_gfx[romoffset+i] = (((0x0f & multfish_gfx[romoffset+i])<<4) | ((0xf0 & multfish_gfx[romoffset+i])>>4)) ^ jscr;
	}
	/* ROM 4 decode */
	romoffset = 0x300000;
	for (i = 0; i < multfish_ROM_SIZE; i++)
	{
		multfish_gfx[romoffset+i] ^= (0x0f & multfish_gfx[romoffset+i])<<4;
	}

	/* Deshuffle all roms*/
	for (i = 0;i < 8;i++)
	{
		romoffset = i * multfish_ROM_SIZE;

		for (j = 0; j < (multfish_ROM_SIZE/0x40); j++)
		{
		   jscr =  BITSWAP16(j,15,14,13,4,3,2,0,1,6,7,5,12,11,10,8,9);
		   memcpy(&temprom[j*0x40],&multfish_gfx[romoffset+(jscr*0x40)],0x40);

		}
		memcpy(&multfish_gfx[romoffset],temprom,multfish_ROM_SIZE);
          }
        auto_free(machine, temprom);
}

INLINE void rom_decodel(UINT8 *romptr, UINT8 *tmprom, UINT8 xor_data, UINT32 xor_add)
{
	UINT32 i, jscr;

	for (i = 0; i < multfish_ROM_SIZE; i++)
	{
	   jscr =  BITSWAP24(i,23,22,21,20,19,17,14,18,16,15,12,13,11,9,6,10,8,7,4,5,3,2,1,0) ^ xor_add ^ 8;
           tmprom[i] = romptr[jscr] ^ xor_data;
	}
	memcpy(romptr,tmprom,multfish_ROM_SIZE);
}
INLINE void rom_decodeh(UINT8 *romptr, UINT8 *tmprom, UINT8 xor_data, UINT32 xor_add)
{
	UINT32 i, jscr;

	for (i = 0; i < multfish_ROM_SIZE; i++)
	{
	   jscr =  BITSWAP24(i,23,22,21,20,19,17,14,18,16,15,12,13,11,9,6,10,8,7,4,5,2,3,1,0) ^ xor_add;
           tmprom[i] = romptr[jscr] ^ xor_data;
	}
	memcpy(romptr,tmprom,multfish_ROM_SIZE);
}

static void lottery_decode(running_machine &machine, UINT8 xor12, UINT8 xor34, UINT8 xor56, UINT8 xor78, UINT32 xor_addr)
{
	UINT8 *multfish_gfx = machine.region("gfx")->base();
	UINT8 *temprom = auto_alloc_array(machine, UINT8, multfish_ROM_SIZE);

	/* ROMs decode */
	rom_decodel(&multfish_gfx[0x000000], temprom, xor12, xor_addr);
	rom_decodel(&multfish_gfx[0x100000], temprom, xor12, xor_addr);
	rom_decodel(&multfish_gfx[0x200000], temprom, xor34, xor_addr);
	rom_decodel(&multfish_gfx[0x300000], temprom, xor34, xor_addr);
	rom_decodeh(&multfish_gfx[0x080000], temprom, xor56, xor_addr);
	rom_decodeh(&multfish_gfx[0x180000], temprom, xor56, xor_addr);
	rom_decodeh(&multfish_gfx[0x280000], temprom, xor78, xor_addr);
	rom_decodeh(&multfish_gfx[0x380000], temprom, xor78, xor_addr);

        auto_free(machine, temprom);
}

INLINE void roment_decodel(UINT8 *romptr, UINT8 *tmprom, UINT8 xor_data, UINT32 xor_add)
{
	UINT32 i, jscr;

	for (i = 0; i < multfish_ROM_SIZE; i++)
	{
	   jscr =  BITSWAP24(i,23,22,21,20,19,16,18,17,14,15,12,13,11,8,10,9,6,7,4,5,3,2,1,0) ^ xor_add ^ 8;
           tmprom[i] = romptr[jscr] ^ xor_data;
	}
	memcpy(romptr,tmprom,multfish_ROM_SIZE);
}
INLINE void roment_decodeh(UINT8 *romptr, UINT8 *tmprom, UINT8 xor_data, UINT32 xor_add)
{
	UINT32 i, jscr;

	for (i = 0; i < multfish_ROM_SIZE; i++)
	{
	   jscr =  BITSWAP24(i,23,22,21,20,19,16,18,17,14,15,12,13,11,8,10,9,6,7,4,5,2,3,1,0) ^ xor_add;
           tmprom[i] = romptr[jscr] ^ xor_data;
	}
	memcpy(romptr,tmprom,multfish_ROM_SIZE);
}

static void ent_decode(running_machine &machine, UINT8 xor12, UINT8 xor34, UINT8 xor56, UINT8 xor78, UINT32 xor_addr)
{
	UINT8 *multfish_gfx = machine.region("gfx")->base();
	UINT8 *temprom = auto_alloc_array(machine, UINT8, multfish_ROM_SIZE);

	/* ROMs decode */
	roment_decodel(&multfish_gfx[0x000000], temprom, xor12, xor_addr);
	roment_decodel(&multfish_gfx[0x100000], temprom, xor12, xor_addr);
	roment_decodel(&multfish_gfx[0x200000], temprom, xor34, xor_addr);
	roment_decodel(&multfish_gfx[0x300000], temprom, xor34, xor_addr);
	roment_decodeh(&multfish_gfx[0x080000], temprom, xor56, xor_addr);
	roment_decodeh(&multfish_gfx[0x180000], temprom, xor56, xor_addr);
	roment_decodeh(&multfish_gfx[0x280000], temprom, xor78, xor_addr);
	roment_decodeh(&multfish_gfx[0x380000], temprom, xor78, xor_addr);

        auto_free(machine, temprom);
}

DRIVER_INIT( island2l )
{
	multfish_state *state = machine.driver_data<multfish_state>();

        state->m_xor_palette = 0x8bf7;
        state->m_xor_paltype = 1;
	lottery_decode(machine, 0xff, 0x11, 0x77, 0xee, 0x44c40);
}
DRIVER_INIT( keksl )
{
	multfish_state *state = machine.driver_data<multfish_state>();

        state->m_xor_palette = 0x41f3;
        state->m_xor_paltype = 1;
	lottery_decode(machine, 0xdd, 0xaa, 0x22, 0x55, 0x2cac0);
}
DRIVER_INIT( pirate2l )
{
	multfish_state *state = machine.driver_data<multfish_state>();

        state->m_xor_palette = 0x8bfb;
        state->m_xor_paltype = 1;
	lottery_decode(machine, 0xaa, 0x11, 0x22, 0xee, 0x48480);
}
DRIVER_INIT( fcockt2l )
{
	multfish_state *state = machine.driver_data<multfish_state>();

        state->m_xor_palette = 0xedfb;
        state->m_xor_paltype = 1;
	lottery_decode(machine, 0x55, 0x11, 0xff, 0xee, 0x78780);
}
DRIVER_INIT( sweetl2l )
{
	multfish_state *state = machine.driver_data<multfish_state>();

        state->m_xor_palette = 0x4bf7;
        state->m_xor_paltype = 1;
	lottery_decode(machine, 0xdd, 0x33, 0x33, 0x77, 0x00800);
}
DRIVER_INIT( gnomel )
{
	multfish_state *state = machine.driver_data<multfish_state>();

        state->m_xor_palette = 0x49ff;
        state->m_xor_paltype = 1;
	lottery_decode(machine, 0xcc, 0x22, 0x33, 0x66, 0x14940);
}
DRIVER_INIT( crzmonent )
{
	multfish_state *state = machine.driver_data<multfish_state>();

        state->m_xor_palette = 0x1cdb;
        state->m_xor_paltype = 2;
}
DRIVER_INIT( resdntent )
{
	multfish_state *state = machine.driver_data<multfish_state>();

        state->m_xor_palette = 0x6edb;
        state->m_xor_paltype = 2;
}
DRIVER_INIT( gnomeent )
{
	multfish_state *state = machine.driver_data<multfish_state>();

        state->m_xor_palette = 0x9edb;
        state->m_xor_paltype = 2;
}
DRIVER_INIT( lhauntent )
{
	multfish_state *state = machine.driver_data<multfish_state>();

        state->m_xor_palette = 0x1adb;
        state->m_xor_paltype = 2;
	ent_decode(machine, 0x22, 0x44, 0x44, 0xbb, 0x24240);
}

static ADDRESS_MAP_START( multfish_map, AS_PROGRAM, 8, multfish_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_WRITE(multfish_vid_w)
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xdfff) AM_DEVREADWRITE_LEGACY("m48t35", multfish_timekeeper_r, multfish_timekeeper_w)
	AM_RANGE(0xe000, 0xffff) AM_READWRITE(bankedram_r, bankedram_w)
ADDRESS_MAP_END

// According to control panel the user buttons are arranged as
// Maxbet | Help | Payout |
// Bet/Cancel  |  1 Line  |  3 Lines  |  5 Lines  | 7 Lines  | 9 Lines  | Start |


static INPUT_PORTS_START( multfish )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_TOGGLE // Key In ( 35 A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) // COIN B (18 B)
	PORT_DIPNAME(     0x04, 0x04, "S Reserve (14 B)" ) // S Reserve ( 14 B )
	PORT_DIPSETTING(  0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x08, 0x08, "Hopper 3 ( 10 B )" ) // Hooper 3 ( 10 B )
	PORT_DIPSETTING(  0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP4 ) PORT_NAME("7 Lines")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // unused?

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, multfish_state,multfish_hopper_r, NULL )// Hopper SW (22 B)
	PORT_DIPNAME(     0x02, 0x02, "BK Door (17 A)"  )
	PORT_DIPSETTING(  0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x04, 0x04, "P Reserve (13 A)" )
	PORT_DIPSETTING(  0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start / Take")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN8 ) PORT_IMPULSE(2) // BILL 4 (07 A)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // unused?

	PORT_START("IN2")
	PORT_DIPNAME(     0x01, 0x01, "Unused??" ) // unused?
	PORT_DIPSETTING(  0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x02, 0x02, "Call Att (17 A)" )
	PORT_DIPSETTING(  0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x04, 0x04, "S Reserve (13 B)" )
	PORT_DIPSETTING(  0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x08, 0x08, "Hopper 2 (09 B)" )
	PORT_DIPSETTING(  0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("5 Lines")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // unused?

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_DIPNAME(     0x02, 0x02, "S Reserve (16 B)" )
	PORT_DIPSETTING(  0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x04, 0x04, "Ticket (12 B)" )
	PORT_DIPSETTING(  0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x08, 0x08, "Hopper 1 (08 B)" )
	PORT_DIPSETTING(  0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN5 ) PORT_IMPULSE(2) // BILL 1 (04 B)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // unused?

	PORT_START("IN4")
	PORT_DIPNAME(     0x01, 0x01, "S Reserve (35 B)" )
	PORT_DIPSETTING(  0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2) // COIN C (19 A)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Help")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("9 Lines") // must be IPT_SLOT_STOP5
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("1 Line")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // unused?

	PORT_START("IN5")
	PORT_SERVICE(     0x01, IP_ACTIVE_LOW )
	PORT_BIT(         0x02, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2) // COIN D (19 B)
	PORT_DIPNAME(     0x04, 0x04, "S Reserve (16 B)" )
	PORT_DIPSETTING(  0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Bet / Double / Cancel")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN6 ) PORT_IMPULSE(2) // BILL 2 (05 A)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // unused?

	PORT_START("IN6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Short Statistic") // Short St (20 A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) // COIN A (18 A)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Max Bet")
	PORT_DIPNAME(     0x08, 0x08, "Hopper 4 (11 A)" )
	PORT_DIPSETTING(  0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("3 Lines")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // unused?

	PORT_START("IN7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) // Key Out (21 B)
	PORT_DIPNAME(     0x02, 0x02, "Fr Door (16 A)" )
	PORT_DIPSETTING(  0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x04, 0x04, "P Reserve (12 A)" )
	PORT_DIPSETTING(  0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_DIPNAME(     0x08, 0x08, "P Reserve (11 A)" )
	PORT_DIPSETTING(  0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN7 ) PORT_IMPULSE(2) // BILL 3 (06 A)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) // unused?
INPUT_PORTS_END

static INPUT_PORTS_START( rollfr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) // COIN B (18 B)
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, multfish_state,multfish_hopper_r, NULL )// Hopper SW (22 B)
	PORT_DIPNAME(     0x02, 0x02, "BK Door (17 A)"  )
	PORT_DIPSETTING(  0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_DIPNAME(     0x08, 0x08, "Hopper Inhibit (08 B)" )
	PORT_DIPSETTING(  0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00, DEF_STR( On ) )
	PORT_BIT( 0xf7, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) //Service SW (20 B)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) //Statistic SW (20 A)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW ) // Fr Door (16 A)
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


WRITE8_MEMBER(multfish_state::multfish_lamps1_w)
{
/*  Port 0x30

    7654 3210
    ---- ---X Hold 1 Lamp 29A
    ---- --X- Hold 2 Lamp 30A
    ---- -X-- Hold 3 Lamp 31A
    ---- X--- Hold 4 Lamp 32A
    ---X ---- Hold 5 Lamp 33A
    --X- ---- Help Lamp 34B
    -X-- ---- Start Lamp 34A
    X--- ---- Bet/Double Lamp 29B
*/
	output_set_lamp_value(1, ((data) & 1));      /* Hold 1 Lamp */
	output_set_lamp_value(2, ((data >> 1) & 1)); /* Hold 2 Lamp */
	output_set_lamp_value(3, ((data >> 2) & 1)); /* Hold 3 Lamp */
	output_set_lamp_value(4, ((data >> 3) & 1)); /* Hold 4 Lamp */
	output_set_lamp_value(5, ((data >> 4) & 1)); /* Hold 5 Lamp */
	output_set_lamp_value(8, ((data >> 5) & 1)); /* Help Lamp */
	output_set_lamp_value(6, ((data >> 6) & 1)); /* Start Lamp */
	output_set_lamp_value(0, ((data >> 7) & 1)); /* Bet/Double Lamp */
}

WRITE8_MEMBER(multfish_state::multfish_lamps2_w)
{
/*  Port 0x34

    7654 3210
    ---- ---X Payout Lamp 31B
    ---- --X- Upper Lamp Yellow 27B (Hopper Error)
    ---- -X-- Maxbet Lamp 30B
    ---X ---- Upper Lamp Green 25B  (Demo Mode)
*/
	output_set_lamp_value(9, ((data) & 1));       /* Payout Lamp */
	output_set_lamp_value(12, ((data >> 1) & 1)); /* Upper Lamp Yellow */
	output_set_lamp_value(7, ((data >> 2) & 1));  /* Maxbet Lamp */
	output_set_lamp_value(10, ((data >> 4) & 1)); /* Upper Lamp Green */
}

WRITE8_MEMBER(multfish_state::multfish_lamps3_w)
{
/*  Port 0x35

    7654 3210
    ---- --X- Upper Lamp Red 26B (Service Mode)
*/
	output_set_lamp_value(11, ((data >> 1) & 1)); /* Upper Lamp Red */
}

WRITE8_MEMBER(multfish_state::multfish_counters_w)
{
/*  Port 0x31

    7654 3210
    ---- ---X Total In Counter 25A
    ---- --X- Coin+Bill Counter 23A
    ---- -X-- Key In Counter 24A
    ---X ---- Total Out Counter 26A
    -X-- ---- Key Out Counter 27A
    X--- ---- Total Bet Counter 28B
*/
        coin_counter_w(machine(), 0, data & 0x01);
        coin_counter_w(machine(), 1, data & 0x02);
        coin_counter_w(machine(), 2, data & 0x04);
        coin_counter_w(machine(), 3, data & 0x10);
        coin_counter_w(machine(), 4, data & 0x40);
        coin_counter_w(machine(), 5, data & 0x80);
}

WRITE8_MEMBER(multfish_state::multfish_f3_w)
{
	//popmessage("multfish_f3_w %02x",data);
}

WRITE8_MEMBER(multfish_state::multfish_dispenable_w)
{
	//popmessage("multfish_f4_w %02x",data); // display enable?
	m_disp_enable = data;
}

static ADDRESS_MAP_START( multfish_portmap, AS_IO, 8, multfish_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_READ_PORT("IN0")
	AM_RANGE(0x11, 0x11) AM_READ_PORT("IN1")
	AM_RANGE(0x12, 0x12) AM_READ_PORT("IN2")
	AM_RANGE(0x13, 0x13) AM_READ_PORT("IN3")
	AM_RANGE(0x14, 0x14) AM_READ_PORT("IN4")
	AM_RANGE(0x15, 0x15) AM_READ_PORT("IN5")
	AM_RANGE(0x16, 0x16) AM_READ_PORT("IN6")
	AM_RANGE(0x17, 0x17) AM_READ_PORT("IN7")

	/* Write ports not hooked up yet */
	AM_RANGE(0x30, 0x30) AM_WRITE(multfish_lamps1_w)
        AM_RANGE(0x31, 0x31) AM_WRITE(multfish_counters_w)
//  AM_RANGE(0x32, 0x32) AM_WRITE_LEGACY(multfish_port32_w)
	AM_RANGE(0x33, 0x33) AM_WRITE(multfish_hopper_w)
	AM_RANGE(0x34, 0x34) AM_WRITE(multfish_lamps2_w)
	AM_RANGE(0x35, 0x35) AM_WRITE(multfish_lamps3_w)
//  AM_RANGE(0x36, 0x36) AM_WRITE_LEGACY(multfish_port36_w)
	AM_RANGE(0x37, 0x37) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x38, 0x38) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_w)
	AM_RANGE(0x39, 0x39) AM_DEVWRITE_LEGACY("aysnd", ay8910_data_w)
	AM_RANGE(0x3a, 0x3a) AM_DEVREAD_LEGACY("aysnd", ay8910_r)

	AM_RANGE(0x60, 0x60) AM_WRITE(multfish_dispenable_w) // display enable mirror for lottery sets

	AM_RANGE(0x90, 0x90) AM_READ(ray_r)

	AM_RANGE(0xa0, 0xa0)  AM_WRITE(multfish_bank_w) // Crazy Monkey 2 banking
	AM_RANGE(0xb0, 0xb0)  AM_WRITE(multfish_bank_w) // Fruit Cocktail 2 lottery banking
	AM_RANGE(0xb1, 0xb1)  AM_WRITE(multfish_bank_w) // Crazy Monkey Ent banking
	AM_RANGE(0xb2, 0xb2)  AM_WRITE(multfish_bank_w) // Lacky Haunter Ent banking
	AM_RANGE(0xb6, 0xb6)  AM_WRITE(multfish_bank_w) // Resident Ent banking
	AM_RANGE(0xbf, 0xbf)  AM_WRITE(multfish_bank_w) // Gnome Ent banking
	AM_RANGE(0xc7, 0xc7)  AM_WRITE(multfish_bank_w) // Resident lottery banking
	AM_RANGE(0xca, 0xca)  AM_WRITE(multfish_bank_w) // Gnome lottery banking
	AM_RANGE(0xcb, 0xcb)  AM_WRITE(multfish_bank_w) // Keks lottery banking
	AM_RANGE(0xcc, 0xcc)  AM_WRITE(multfish_bank_w) // Sweet Life 2 lottery banking
	AM_RANGE(0xcd, 0xcd)  AM_WRITE(multfish_bank_w) // Island 2 lottery banking
	AM_RANGE(0xce, 0xce)  AM_WRITE(multfish_bank_w) // Pirate 2 lottery banking
	AM_RANGE(0xd0, 0xd0)  AM_WRITE(multfish_bank_w) // rollfr_4 banking
	AM_RANGE(0xe1, 0xe1)  AM_WRITE(multfish_bank_w) // Island 2 banking
	AM_RANGE(0xe5, 0xe5)  AM_WRITE(multfish_bank_w) // Gnome banking
	AM_RANGE(0xe8, 0xe8)  AM_WRITE(multfish_bank_w) // Sweet Life 2 banking
	AM_RANGE(0xea, 0xea)  AM_WRITE(multfish_bank_w) // Fruit Cocktail 2 banking
	AM_RANGE(0xec, 0xec)  AM_WRITE(multfish_bank_w) // Crazy Monkey lottery banking

	AM_RANGE(0xf0, 0xf0)  AM_WRITE(multfish_bank_w) // Gold Fish banking
	AM_RANGE(0xf1, 0xf1)  AM_WRITE(multfish_rambank_w)
	AM_RANGE(0xf3, 0xf3)  AM_WRITE(multfish_f3_w) // from 00->01 at startup, irq enable maybe?
	AM_RANGE(0xf4, 0xf4)  AM_WRITE(multfish_dispenable_w) // display enable

	/* mirrors of the rom banking */
	AM_RANGE(0xf8, 0xfd)  AM_WRITE(multfish_bank_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( rollfr_portmap, AS_IO, 8, multfish_state )
	AM_RANGE(0x33, 0x33) AM_WRITE(rollfr_hopper_w)
	AM_IMPORT_FROM(multfish_portmap)
ADDRESS_MAP_END

static const gfx_layout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,4),
	8,
	{ RGN_FRAC(2,4)+0, RGN_FRAC(2,4)+1,RGN_FRAC(2,4)+2, RGN_FRAC(2,4)+3,0,1,2,3 },
	{ 0,4,
	  RGN_FRAC(1,4)+0, RGN_FRAC(1,4)+4,
	  8, 12,
	  RGN_FRAC(1,4)+8, RGN_FRAC(1,4)+12,
	  16, 20,
	  RGN_FRAC(1,4)+16, RGN_FRAC(1,4)+20,
	  24, 28,
	  RGN_FRAC(1,4)+24,RGN_FRAC(1,4)+28 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32 },
	8*64
};


static GFXDECODE_START( multfish )
	GFXDECODE_ENTRY( "gfx", 0, tiles16x16_layout, 0, 16 )
GFXDECODE_END

static MACHINE_START( multfish )
{
	multfish_state *state = machine.driver_data<multfish_state>();

	state->save_item(NAME(state->m_disp_enable));
	state->save_item(NAME(state->m_rambk));
	state->save_item(NAME(state->m_hopper_motor));
	state->save_item(NAME(state->m_hopper));
}

static MACHINE_RESET( multfish )
{
	multfish_state *state = machine.driver_data<multfish_state>();

	memory_configure_bank(machine, "bank1", 0, 16, machine.region("maincpu")->base(), 0x4000);
	memory_set_bank(machine, "bank1", 0);

	state->m_disp_enable = 0;
	state->m_rambk = 0;
	state->m_hopper_motor = 0;
	state->m_hopper = 0;
}

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


static MACHINE_CONFIG_START( multfish, multfish_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_24MHz/4)
	MCFG_CPU_PROGRAM_MAP(multfish_map)
	MCFG_CPU_IO_MAP(multfish_portmap)
	MCFG_CPU_VBLANK_INT("screen",irq0_line_hold)

	MCFG_MACHINE_START( multfish )
	MCFG_MACHINE_RESET( multfish )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*16, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(17*16, 1024-16*7-1, 1*16, 32*16-1*16-1)
	MCFG_SCREEN_UPDATE_STATIC(multfish)
	MCFG_GFXDECODE(multfish)
	MCFG_PALETTE_LENGTH(0x1000)

	MCFG_VIDEO_START(multfish)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, 6000000/4)
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_M48T35_ADD( "m48t35" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( rollfr, multfish )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(rollfr_portmap)
MACHINE_CONFIG_END

static MACHINE_RESET( island2a )
{
	MACHINE_RESET_CALL(multfish);

	// this set needs preprogrammed data in timekeeper
	timekeeper_w(machine.device("m48t35"), 0x2003 , 0x01);
	timekeeper_w(machine.device("m48t35"), 0x4003 , 0x02);
}
static MACHINE_CONFIG_DERIVED( island2a, multfish )

	/* basic machine hardware */
	MCFG_MACHINE_RESET( island2a )
MACHINE_CONFIG_END



/* Rom Naming note:

The GFX ROMs do not have labels, for clarity we name
them as on Igrosoft web-site hash tables.

code roms:
xx_m_xxxxxx.rom     - world relase
xx_xxxxxx.rom       - release for Russia (or for all, if the game does not have different roms for different regions)
xx_l_xxxxxx.rom     - lottery game
xx_e_xxxxxx.rom     - entertainment game

graphics roms:
xxxxxxxx_m.00x      - roms for world relase
xxxxxxxx.00x        - release for Russia (or for all, if the game does not have special gfx-sets for different regions)
xxxxxxxx_loto.00x   - lottery sets
xxxxxxxx_ent.00x    - entertainment sets


*/

/*********************************************************
   Multifish
**********************************************************/

ROM_START( goldfish ) // Gold Fish 020903 prototype of Multi Fish
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gf_020903.rom", 0x00000, 0x40000, CRC(705304fc) SHA1(f02336066ba2ff394ac153107e308d5356e99eca) )

	ROM_REGION( 0x400000, "gfx", 0 )
	/* did it really use these graphic ROMs? they include the screens used by the games not included in 'Gold Fish' */
	ROM_LOAD( "multi_fish.001", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "multi_fish.002", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "multi_fish.003", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "multi_fish.004", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "multi_fish.005", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "multi_fish.006", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "multi_fish.007", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "multi_fish.008", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish ) // 021120
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf021120.rom", 0x00000, 0x40000, SHA1(eb7eb5aae00a77edcf328f460970eb180d86d058) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "multi_fish.001", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "multi_fish.002", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "multi_fish.003", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "multi_fish.004", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "multi_fish.005", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "multi_fish.006", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "multi_fish.007", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "multi_fish.008", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_2 ) // 021121
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf021121.rom", 0x00000, 0x40000, SHA1(87a1fb81330cf4b66e17702c22fda694ebff58eb) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "multi_fish.001", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "multi_fish.002", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "multi_fish.003", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "multi_fish.004", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "multi_fish.005", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "multi_fish.006", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "multi_fish.007", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "multi_fish.008", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_3 ) // 021124
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf021124.rom", 0x00000, 0x40000, CRC(59fd16f5) SHA1(ea132f68e9c09c40369d4cc02c670ee6e26bdcbe) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "multi_fish.001", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "multi_fish.002", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "multi_fish.003", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "multi_fish.004", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "multi_fish.005", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "multi_fish.006", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "multi_fish.007", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "multi_fish.008", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END


ROM_START( mfish_4 ) // 021219
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf021219.rom", 0x00000, 0x40000, SHA1(887d456b2ba89560329457d9eaea26fb72223a38) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "multi_fish.001", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "multi_fish.002", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "multi_fish.003", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "multi_fish.004", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "multi_fish.005", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "multi_fish.006", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "multi_fish.007", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "multi_fish.008", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_5 ) // 021227
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf021227.rom", 0x00000, 0x40000, SHA1(58b74c41a88a781da01dba52744dc74e41deae70) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "multi_fish.001", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "multi_fish.002", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "multi_fish.003", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "multi_fish.004", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "multi_fish.005", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "multi_fish.006", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "multi_fish.007", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "multi_fish.008", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_6 ) // 030124
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf030124.rom", 0x00000, 0x40000, CRC(554c9cda) SHA1(b119b086bad3f6f8acc64a5809ce449800615406) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "multi_fish.001", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "multi_fish.002", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "multi_fish.003", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "multi_fish.004", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "multi_fish.005", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "multi_fish.006", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "multi_fish.007", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "multi_fish.008", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_7 ) // 030511
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf030511.rom", 0x00000, 0x40000, SHA1(06b3e3875f036782983e29e305f67a36f78a4f06) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "multi_fish.001", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "multi_fish.002", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "multi_fish.003", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "multi_fish.004", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "multi_fish.005", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "multi_fish.006", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "multi_fish.007", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "multi_fish.008", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_8 ) // 030522
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf030522.rom", 0x00000, 0x40000, CRC(bff97c25) SHA1(fa80e12275b960374c84518bcaa1e32d0a4ff437) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "multi_fish.001", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "multi_fish.002", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "multi_fish.003", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "multi_fish.004", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "multi_fish.005", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "multi_fish.006", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "multi_fish.007", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "multi_fish.008", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_9 ) // 031026
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf031026.rom", 0x00000, 0x40000, SHA1(451b390793f89188afe2b6e82fc02b474fb97a7c) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "multi_fish.001", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "multi_fish.002", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "multi_fish.003", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "multi_fish.004", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "multi_fish.005", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "multi_fish.006", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "multi_fish.007", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "multi_fish.008", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_10 ) // 031117
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf031117.rom", 0x00000, 0x40000, SHA1(1d244a332af0fb6aa593a246211ff2b6d2c48a59) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "multi_fish.001", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "multi_fish.002", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "multi_fish.003", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "multi_fish.004", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "multi_fish.005", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "multi_fish.006", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "multi_fish.007", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "multi_fish.008", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_11 ) // 031124
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf031124.rom", 0x00000, 0x40000, CRC(1d60d37a) SHA1(c0d1b541c4b076bbc810ad637acb4a2663a919ba) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "multi_fish.001", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "multi_fish.002", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "multi_fish.003", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "multi_fish.004", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "multi_fish.005", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "multi_fish.006", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "multi_fish.007", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "multi_fish.008", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_12 ) // 040308
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf040308.rom", 0x00000, 0x40000, CRC(adb9c1d9) SHA1(88c69f48766dc7c98a6f03c1a0a4aa63b76560b6) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "multi_fish.001", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "multi_fish.002", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "multi_fish.003", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "multi_fish.004", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "multi_fish.005", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "multi_fish.006", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "multi_fish.007", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "multi_fish.008", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_13 ) // 040316
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "mf040316.rom", 0x00000, 0x40000, CRC(1acf9f4f) SHA1(c1f4d1c51632a45b533d19c8b6f63d337d84d9cd) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "multi_fish.001", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "multi_fish.002", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "multi_fish.003", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "multi_fish.004", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "multi_fish.005", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "multi_fish.006", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "multi_fish.007", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "multi_fish.008", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END


/*********************************************************
   Crazy Monkey

    "Russia" sets use different gfx roms 1-4.
        The official list of hashes shows the both roms.

**********************************************************/

ROM_START( crzmon ) // 030217
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm_m_030217.rom", 0x00000, 0x40000, SHA1(75787f32aa4c8e8ff7bc11c57a37ad5a65f71c52) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_m.001", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "crazymonkey_m.002", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "crazymonkey_m.003", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "crazymonkey_m.004", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_2 ) // 030225
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm_m_030225.rom", 0x00000, 0x40000, SHA1(3627a3d6a4a50ed8544456d53ab5a489af389a19) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_m.001", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "crazymonkey_m.002", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "crazymonkey_m.003", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "crazymonkey_m.004", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_3 ) // 030227
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm_m_030227.rom", 0x00000, 0x40000, SHA1(4f8cd68dd2b6abeaabc9b45da18469cc6e7ac74d) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_m.001", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "crazymonkey_m.002", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "crazymonkey_m.003", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "crazymonkey_m.004", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_4 ) // 030404
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm_m_030404.rom", 0x00000, 0x40000, SHA1(fd99caa2b6ef7218563db4f3b755e34dd551e05f) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_m.001", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "crazymonkey_m.002", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "crazymonkey_m.003", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "crazymonkey_m.004", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_5 ) // 030421
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm_m_030421.rom", 0x00000, 0x40000, CRC(6826564E) SHA1(6559e45e3ec39c1d201ed54a10fdb5c6aeff6582) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_m.001", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "crazymonkey_m.002", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "crazymonkey_m.003", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "crazymonkey_m.004", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_6 ) // 031016
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm_m_031016.rom", 0x00000, 0x40000, SHA1(2f2a5ecbb311ade75f8fdc322c6e63836d4119c3) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_m.001", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "crazymonkey_m.002", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "crazymonkey_m.003", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "crazymonkey_m.004", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_7 ) // 031110
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm_m_031110.rom", 0x00000, 0x40000, CRC(d3e67980) SHA1(f0daa91abdde211a2ff61414d84386b763c30949) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_m.001", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "crazymonkey_m.002", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "crazymonkey_m.003", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "crazymonkey_m.004", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_8 ) // 050120
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm_m_050120.rom", 0x00000, 0x40000, CRC(9af1e03f) SHA1(caadf48a36da48f4e126b286f6f5498005d8182a) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_m.001", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "crazymonkey_m.002", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "crazymonkey_m.003", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "crazymonkey_m.004", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_9 ) // 070315
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm_070315.rom", 0x00000, 0x40000, CRC(5b2310b0) SHA1(b9bcb45bd97cbf1546c938512709bae44501447d) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey.001",   0x000000, 0x80000, CRC(665ae6a1) SHA1(2ef2d657918d66d303d45d2d82430d13108f3fad) )
	ROM_LOAD( "crazymonkey.002",   0x100000, 0x80000, CRC(1a8e235a) SHA1(6d562cc5250283fc0c8ca7e103231a2e5bab4c69) )
	ROM_LOAD( "crazymonkey.003",   0x200000, 0x80000, CRC(415133eb) SHA1(227f7c8858fd5b928fdde691017104d3bd69910a) )
	ROM_LOAD( "crazymonkey.004",   0x300000, 0x80000, CRC(ec45fe14) SHA1(4a0fc87e2f19ea05c9a5746bb4ca7cafe5592d33) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_10 ) // 081027 lottery
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm_l_081027.rom", 0x00000, 0x40000, SHA1(11a1523bc0ce5cf43534b34201f59784283693f0) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_loto.001", 0x000000, 0x80000, SHA1(bf953dc53ec85f4841fe7ada7e480520b3bce1d7) )
	ROM_LOAD( "crazymonkey_loto.002", 0x100000, 0x80000, SHA1(57db2212b690a8a92034ba4993526c34cbf48c09) )
	ROM_LOAD( "crazymonkey_loto.003", 0x200000, 0x80000, SHA1(4551494b883f8076931bb22fd0541b193039dfdc) )
	ROM_LOAD( "crazymonkey_loto.004", 0x300000, 0x80000, SHA1(b33d5007b649661a3d811139c2b40d036863343d) )
	ROM_LOAD( "crazymonkey_m.005",    0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006",    0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007",    0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008",    0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_11 ) // 081113 lottery
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "cm_l_081113.rom", 0x00000, 0x40000, SHA1(7196c301691b47a572cefc090888db550f10998c) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_loto.001", 0x000000, 0x80000, SHA1(bf953dc53ec85f4841fe7ada7e480520b3bce1d7) )
	ROM_LOAD( "crazymonkey_loto.002", 0x100000, 0x80000, SHA1(57db2212b690a8a92034ba4993526c34cbf48c09) )
	ROM_LOAD( "crazymonkey_loto.003", 0x200000, 0x80000, SHA1(4551494b883f8076931bb22fd0541b193039dfdc) )
	ROM_LOAD( "crazymonkey_loto.004", 0x300000, 0x80000, SHA1(b33d5007b649661a3d811139c2b40d036863343d) )
	ROM_LOAD( "crazymonkey_m.005",    0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006",    0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007",    0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008",    0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

/*********************************************************
   Fruit Cocktail

    "Russia" sets use different gfx roms.
        The official list of hashes shows only updated roms.

**********************************************************/

ROM_START( fcockt ) // 030505
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc_m_030505.rom", 0x00000, 0x40000, SHA1(cc65334e8dfae5ffef1d73bd5085e3555905e259) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_2 ) // 030512
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc_m_030512.rom", 0x00000, 0x40000, SHA1(c23ebcf64609a56a029f05101185f3adf73cdadd) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_3 ) // 030623
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc_m_030623.rom", 0x00000, 0x40000, CRC(00A267B2) SHA1(b95c5e06cf41762802199e1b55a5eda2243c9af7) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_4 ) // 031028
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc_m_031028.rom", 0x00000, 0x40000, SHA1(18a0ac6e3c6f1d6ae7aeae5322e6b6617923cfdf) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_5 ) // 031111
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc_m_031111.rom", 0x00000, 0x40000, CRC(feef74bf) SHA1(7cc9aeb88a2923f6c5c176abcd6c6b241b353eab) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_6 ) // 040216
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc_m_040216.rom", 0x00000, 0x40000, CRC(d12b0201) SHA1(09f4b0b5239609ebf13e643782d1881920a1203d) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_7 ) // 050118
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc_m_050118.rom", 0x00000, 0x40000, CRC(356b140a) SHA1(d6e671b5c7fa6592f80b90b289cce0afe1a9cea3) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_8 ) // 060111
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc_m_060111.rom", 0x00000, 0x40000, CRC(a4af79e3) SHA1(28f40573d6c61e1937b8d05da94e197da5236f57) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_9 ) // 070305
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc_070305.rom", 0x00000, 0x40000, CRC(4eb835d9) SHA1(406b2fcad0ca587eacee123ac4b040cb6f6db18c) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail.001",   0x000000, 0x80000, CRC(735FBE79) SHA1(6ab590e00043dcb6648fd942e11747486d497df1) ) /* Only this set is listed as official hashes */
	ROM_LOAD( "fruitcocktail.002",   0x100000, 0x80000, CRC(28fc888e) SHA1(2b8c55675bf61203682d560c9b3f29568719113f) )
	ROM_LOAD( "fruitcocktail.003",   0x200000, 0x80000, CRC(01fc1a18) SHA1(4f73c6cde6ed741cc8c1bc32442f572ee7ba208a) )
	ROM_LOAD( "fruitcocktail.004",   0x300000, 0x80000, CRC(68daa864) SHA1(b05c455e23ace80e102699616b75f3a0946c04bc) )
	ROM_LOAD( "fruitcocktail.005",   0x080000, 0x80000, CRC(64b547e3) SHA1(285421fa3aa67a16cf6a9dadb20d74e6a8471dc0) )
	ROM_LOAD( "fruitcocktail.006",   0x180000, 0x80000, CRC(965d6363) SHA1(5c229238a09ec54147d492e9843595962ce79952) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) ) // this remains the same
	ROM_LOAD( "fruitcocktail.008",   0x380000, 0x80000, CRC(8384e4d4) SHA1(83d0bbbd7cca7328a66a69cf802632fd8d22d5b8) )
ROM_END

ROM_START( fcockt_10 ) // 070517
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc_070517.rom", 0x00000, 0x40000, CRC(8b43f765) SHA1(86412c37252cf1f12a3acd9359bbf1cdcf52da9f) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail.001",   0x000000, 0x80000, CRC(735FBE79) SHA1(6ab590e00043dcb6648fd942e11747486d497df1) ) /* Only this set is listed as official hashes */
	ROM_LOAD( "fruitcocktail.002",   0x100000, 0x80000, CRC(28fc888e) SHA1(2b8c55675bf61203682d560c9b3f29568719113f) )
	ROM_LOAD( "fruitcocktail.003",   0x200000, 0x80000, CRC(01fc1a18) SHA1(4f73c6cde6ed741cc8c1bc32442f572ee7ba208a) )
	ROM_LOAD( "fruitcocktail.004",   0x300000, 0x80000, CRC(68daa864) SHA1(b05c455e23ace80e102699616b75f3a0946c04bc) )
	ROM_LOAD( "fruitcocktail.005",   0x080000, 0x80000, CRC(64b547e3) SHA1(285421fa3aa67a16cf6a9dadb20d74e6a8471dc0) )
	ROM_LOAD( "fruitcocktail.006",   0x180000, 0x80000, CRC(965d6363) SHA1(5c229238a09ec54147d492e9843595962ce79952) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) ) // this remains the same
	ROM_LOAD( "fruitcocktail.008",   0x380000, 0x80000, CRC(8384e4d4) SHA1(83d0bbbd7cca7328a66a69cf802632fd8d22d5b8) )
ROM_END

ROM_START( fcockt_11 ) // 070822
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc_070822.rom", 0x00000, 0x40000, CRC(f156657d) SHA1(bd538e714a87461bdf84df18ae3f8caeee876747) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail.001",   0x000000, 0x80000, CRC(735FBE79) SHA1(6ab590e00043dcb6648fd942e11747486d497df1) ) /* Only this set is listed as official hashes */
	ROM_LOAD( "fruitcocktail.002",   0x100000, 0x80000, CRC(28fc888e) SHA1(2b8c55675bf61203682d560c9b3f29568719113f) )
	ROM_LOAD( "fruitcocktail.003",   0x200000, 0x80000, CRC(01fc1a18) SHA1(4f73c6cde6ed741cc8c1bc32442f572ee7ba208a) )
	ROM_LOAD( "fruitcocktail.004",   0x300000, 0x80000, CRC(68daa864) SHA1(b05c455e23ace80e102699616b75f3a0946c04bc) )
	ROM_LOAD( "fruitcocktail.005",   0x080000, 0x80000, CRC(64b547e3) SHA1(285421fa3aa67a16cf6a9dadb20d74e6a8471dc0) )
	ROM_LOAD( "fruitcocktail.006",   0x180000, 0x80000, CRC(965d6363) SHA1(5c229238a09ec54147d492e9843595962ce79952) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) ) // this remains the same
	ROM_LOAD( "fruitcocktail.008",   0x380000, 0x80000, CRC(8384e4d4) SHA1(83d0bbbd7cca7328a66a69cf802632fd8d22d5b8) )
ROM_END

ROM_START( fcockt_12 ) // 070911
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc_070911.rom", 0x00000, 0x40000, CRC(17c015bb) SHA1(5369549853f1c463b999bb4ff9d06c5d8e467c5b) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail.001",   0x000000, 0x80000, CRC(735FBE79) SHA1(6ab590e00043dcb6648fd942e11747486d497df1) ) /* Only this set is listed as official hashes */
	ROM_LOAD( "fruitcocktail.002",   0x100000, 0x80000, CRC(28fc888e) SHA1(2b8c55675bf61203682d560c9b3f29568719113f) )
	ROM_LOAD( "fruitcocktail.003",   0x200000, 0x80000, CRC(01fc1a18) SHA1(4f73c6cde6ed741cc8c1bc32442f572ee7ba208a) )
	ROM_LOAD( "fruitcocktail.004",   0x300000, 0x80000, CRC(68daa864) SHA1(b05c455e23ace80e102699616b75f3a0946c04bc) )
	ROM_LOAD( "fruitcocktail.005",   0x080000, 0x80000, CRC(64b547e3) SHA1(285421fa3aa67a16cf6a9dadb20d74e6a8471dc0) )
	ROM_LOAD( "fruitcocktail.006",   0x180000, 0x80000, CRC(965d6363) SHA1(5c229238a09ec54147d492e9843595962ce79952) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) ) // this remains the same
	ROM_LOAD( "fruitcocktail.008",   0x380000, 0x80000, CRC(8384e4d4) SHA1(83d0bbbd7cca7328a66a69cf802632fd8d22d5b8) )
ROM_END

ROM_START( fcockt_13 ) // 081124 lottery
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc_l_081124.rom", 0x00000, 0x40000, SHA1(896252194f32842f784463668e6416cbfe9687a0) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_loto.001", 0x000000, 0x80000, SHA1(bb2b518dc166836f7cedd4ec443b50687e8927e1) ) /* Only this set is listed as official hashes */
	ROM_LOAD( "fruitcocktail_loto.002", 0x100000, 0x80000, SHA1(2621e2644ebec3959c49905c54eb20a83d5a7bd6) )
	ROM_LOAD( "fruitcocktail_loto.003", 0x200000, 0x80000, SHA1(ffe11deef6b3b86b4b78e2e4d96c30f820e77971) )
	ROM_LOAD( "fruitcocktail_loto.004", 0x300000, 0x80000, SHA1(fa88d113721ce7c0b3418614cd6bb974c20df644) )
	ROM_LOAD( "fruitcocktail_m.005",    0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006",    0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007",    0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008",    0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

/*********************************************************
   Lucky Haunter

    "Russia" sets use different gfx roms 1-4.
        The official list of hashes shows only updated roms.

**********************************************************/

ROM_START( lhaunt ) // 030707
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "lh_m_030707.rom", 0x00000, 0x40000, SHA1(c7b8e1b98cd0aa665d62c1652716993539c9f3ef) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "luckyhaunter_m.001", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "luckyhaunter_m.002", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "luckyhaunter_m.003", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "luckyhaunter_m.004", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "luckyhaunter_m.005", 0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006", 0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007", 0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008", 0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_2 ) // 030804
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "lh_m_030804.rom", 0x00000, 0x40000, CRC(2A9A7267) SHA1(b75702a678d716cd0ccb1f2d1e58c1d3e9f7ca98) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "luckyhaunter_m.001", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "luckyhaunter_m.002", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "luckyhaunter_m.003", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "luckyhaunter_m.004", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "luckyhaunter_m.005", 0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006", 0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007", 0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008", 0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_3 ) // 031027
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "lh_m_031027.rom", 0x00000, 0x40000, SHA1(caec736dde2878588ab197ba37801cf7a9ed975b) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "luckyhaunter_m.001", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "luckyhaunter_m.002", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "luckyhaunter_m.003", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "luckyhaunter_m.004", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "luckyhaunter_m.005", 0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006", 0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007", 0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008", 0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_4 ) // 031111
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "lh_m_031111.rom", 0x00000, 0x40000, CRC(fc357b75) SHA1(512e4f57612851284bb93ba97c276cbc7cb758d9) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "luckyhaunter_m.001", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "luckyhaunter_m.002", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "luckyhaunter_m.003", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "luckyhaunter_m.004", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "luckyhaunter_m.005", 0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006", 0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007", 0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008", 0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_5 ) // 040216
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "lh_m_040216.rom", 0x00000, 0x40000, CRC(558d8345) SHA1(30a87902b291413b1e6eaad6bf4964c54e391e23) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "luckyhaunter_m.001", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "luckyhaunter_m.002", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "luckyhaunter_m.003", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "luckyhaunter_m.004", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "luckyhaunter_m.005", 0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006", 0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007", 0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008", 0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_6 ) // 040825
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "lh_m_040825.rom", 0x00000, 0x40000, CRC(f9924fa1) SHA1(57a1730fef4963d30f3991f27021647a8c681952) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "luckyhaunter_m.001", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "luckyhaunter_m.002", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "luckyhaunter_m.003", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "luckyhaunter_m.004", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "luckyhaunter_m.005", 0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006", 0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007", 0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008", 0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_7 ) // 070402
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "lh_070402.rom", 0x00000, 0x40000, CRC(16e921d9) SHA1(66849020a451d0c1dc4ac310e98fa7ef6a962548) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "luckyhaunter.001",   0x000000, 0x80000, CRC(c541ce0f) SHA1(5b5ad171168f204e1a4892a4be06a4983dcfc5e5) )
	ROM_LOAD( "luckyhaunter.002",   0x100000, 0x80000, CRC(ea49e52f) SHA1(7c6b0da1c4ebca5439dda7d38882055b47d00169) )
	ROM_LOAD( "luckyhaunter.003",   0x200000, 0x80000, CRC(f601cd05) SHA1(b983d703ff5f5795ed60248cd4affc91f983072c) )
	ROM_LOAD( "luckyhaunter.004",   0x300000, 0x80000, CRC(10040104) SHA1(966abcccb67949b73f84ffc85ead70b10856fa5b) )
	ROM_LOAD( "luckyhaunter_m.005", 0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006", 0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007", 0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008", 0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_8 ) // 070604
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "lh_070604.rom", 0x00000, 0x40000, CRC(a9e80888) SHA1(57fdfc1149db1555808040a7a902dec5f1389943) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "luckyhaunter.001",   0x000000, 0x80000, CRC(c541ce0f) SHA1(5b5ad171168f204e1a4892a4be06a4983dcfc5e5) )
	ROM_LOAD( "luckyhaunter.002",   0x100000, 0x80000, CRC(ea49e52f) SHA1(7c6b0da1c4ebca5439dda7d38882055b47d00169) )
	ROM_LOAD( "luckyhaunter.003",   0x200000, 0x80000, CRC(f601cd05) SHA1(b983d703ff5f5795ed60248cd4affc91f983072c) )
	ROM_LOAD( "luckyhaunter.004",   0x300000, 0x80000, CRC(10040104) SHA1(966abcccb67949b73f84ffc85ead70b10856fa5b) )
	ROM_LOAD( "luckyhaunter_m.005", 0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006", 0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007", 0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008", 0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_9 ) // 081208 lottery
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "lh_l_081208.rom", 0x00000, 0x40000, SHA1(4962bfc9c3aadd45fdb30bb159aaaed463e4d06b) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "luckyhaunter_loto.001",   0x000000, 0x80000, SHA1(961f832654f2cdb844e36a1a9034b87b5e3750f5) )
	ROM_LOAD( "luckyhaunter_loto.002",   0x100000, 0x80000, SHA1(20ac980f4f8b502773845d2e1350b960ea707d83) )
	ROM_LOAD( "luckyhaunter_loto.003",   0x200000, 0x80000, SHA1(e4cf08104e7717d9706105ad52ade6bf9a782d76) )
	ROM_LOAD( "luckyhaunter_loto.004",   0x300000, 0x80000, SHA1(ceb4a5e9912f5d98483cb75e871c925dffbb8e72) )
	ROM_LOAD( "luckyhaunter_m.005",      0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006",      0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007",      0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008",      0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

/*********************************************************
   Garage

    "Russia" sets use different gfx roms 1-4.
        The official list of hashes shows only updated roms.

**********************************************************/

ROM_START( garage ) // 040122
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gg_m_040122.rom", 0x00000, 0x40000, SHA1(327e55d1f4bdc0ad0556faa2fbdaa05b9a5f1c16) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "garage_m.001", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "garage_m.002", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "garage_m.003", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "garage_m.004", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "garage_m.005", 0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "garage_m.006", 0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "garage_m.007", 0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "garage_m.008", 0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_2 ) // 040123
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gg_m_040123.rom", 0x00000, 0x40000, SHA1(3051c99d22cfe46b532fcc59a0b98eec186f4a76) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "garage_m.001", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "garage_m.002", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "garage_m.003", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "garage_m.004", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "garage_m.005", 0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "garage_m.006", 0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "garage_m.007", 0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "garage_m.008", 0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_3 ) // 040216
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gg_m_040216.rom", 0x00000, 0x40000, SHA1(321c4106ce07e195a05eacdef6387d61d5e58bb9) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "garage_m.001", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "garage_m.002", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "garage_m.003", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "garage_m.004", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "garage_m.005", 0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "garage_m.006", 0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "garage_m.007", 0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "garage_m.008", 0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_4 ) // 040219
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gg_m_040219.rom", 0x00000, 0x40000, CRC(49fe4a55) SHA1(df55df0065b4718d2b0c7ff3da85f5d66c2dd95f) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "garage_m.001", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "garage_m.002", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "garage_m.003", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "garage_m.004", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "garage_m.005", 0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "garage_m.006", 0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "garage_m.007", 0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "garage_m.008", 0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_5 ) // 050311
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gg_m_050311.rom", 0x00000, 0x40000, CRC(405aee88) SHA1(356a8c309434ae4ad6b6fab97aeaece8aa60a730) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "garage_m.001", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "garage_m.002", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "garage_m.003", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "garage_m.004", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "garage_m.005", 0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "garage_m.006", 0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "garage_m.007", 0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "garage_m.008", 0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_6 ) // 070213
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gg_070213.rom", 0x00000, 0x40000, CRC(2ae974c3) SHA1(61076d4154befffd3f90ae32b9bfa0e8d003ae72) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "garage.001",   0x000000, 0x80000, CRC(15349b8c) SHA1(4f59e67114d7440dab82655787a6df9404bcb4b2) )
	ROM_LOAD( "garage.002",   0x100000, 0x80000, CRC(634d1f94) SHA1(d979b0c22c7722eea8789296e9bd3f65b27a2b50) )
	ROM_LOAD( "garage.003",   0x200000, 0x80000, CRC(3fb65c28) SHA1(0b3cacc8907267be84051c562bce1fc82701a5d5) )
	ROM_LOAD( "garage.004",   0x300000, 0x80000, CRC(fc489452) SHA1(ac421f99c3ac3c9b7adf274881c00cbee65b6df5) )
	ROM_LOAD( "garage_m.005", 0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "garage_m.006", 0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "garage_m.007", 0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "garage_m.008", 0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_7 ) // 070329
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gg_070329.rom", 0x00000, 0x40000, CRC(2bd50a8f) SHA1(b0c66eb0ae40b87f81ee5bb66dc4ff7d7acd14bd) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "garage.001",   0x000000, 0x80000, CRC(15349b8c) SHA1(4f59e67114d7440dab82655787a6df9404bcb4b2) )
	ROM_LOAD( "garage.002",   0x100000, 0x80000, CRC(634d1f94) SHA1(d979b0c22c7722eea8789296e9bd3f65b27a2b50) )
	ROM_LOAD( "garage.003",   0x200000, 0x80000, CRC(3fb65c28) SHA1(0b3cacc8907267be84051c562bce1fc82701a5d5) )
	ROM_LOAD( "garage.004",   0x300000, 0x80000, CRC(fc489452) SHA1(ac421f99c3ac3c9b7adf274881c00cbee65b6df5) )
	ROM_LOAD( "garage_m.005", 0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "garage_m.006", 0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "garage_m.007", 0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "garage_m.008", 0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_8 ) // 081229 lottery
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gg_l_081229.rom", 0x00000, 0x40000, SHA1(6bc22aeb6d8d5ffbc556d9056a25e6506bb8f118) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "garage_loto.001", 0x000000, 0x80000, SHA1(50de46c4ae28f70c96da03391446cca0cb91f43b) )
	ROM_LOAD( "garage_loto.002", 0x100000, 0x80000, SHA1(df5c1684f1a29f77a3fa79b35a9a0b9371c1b8a3) )
	ROM_LOAD( "garage_loto.003", 0x200000, 0x80000, SHA1(47e81aa034c1ac717b1715c521cbaa8ff4d336c5) )
	ROM_LOAD( "garage_loto.004", 0x300000, 0x80000, SHA1(71dbcd71ee5bddeda77a012c243981e0960e0c9e) )
	ROM_LOAD( "garage_m.005",    0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "garage_m.006",    0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "garage_m.007",    0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "garage_m.008",    0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

/*********************************************************
   Rock Climber

    "Russia" sets use different gfx roms 1-4.
        The official list of hashes shows only updated roms.

**********************************************************/

ROM_START( rclimb ) // 040815
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rc_m_040815.rom", 0x00000, 0x40000, CRC(5efd5c86) SHA1(593e64bfe57ba271c04bdd2a35c9484c4efaaa00))

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "rockclimber_m.001", 0x000000, 0x80000, CRC(934f18c7) SHA1(da3a7cddc68e104d415d947e89c0e7f0d067c056) )
	ROM_LOAD( "rockclimber_m.002", 0x100000, 0x80000, CRC(7364bd2b) SHA1(c0edfd3b8de813c95fe5d6072662fa0e39fec89e) )
	ROM_LOAD( "rockclimber_m.003", 0x200000, 0x80000, CRC(e7befb17) SHA1(8a214680142cd657784a667ab3f6422165fea224) )
	ROM_LOAD( "rockclimber_m.004", 0x300000, 0x80000, CRC(dc6d43a0) SHA1(62fc47136775f3fa9369857ec91fe897a1f1ebd6) )
	ROM_LOAD( "rockclimber_m.005", 0x080000, 0x80000, CRC(ea127c3d) SHA1(a6391eed69a4723b68d727f59b6baebe51633e66) )
	ROM_LOAD( "rockclimber_m.006", 0x180000, 0x80000, CRC(277fa273) SHA1(6320e6c5b5e48dc451cc48189054c42d85e8ccc1) )
	ROM_LOAD( "rockclimber_m.007", 0x280000, 0x80000, CRC(3ca7f69a) SHA1(878cca181d915dc3548d5285a4bbb51aef31a64e) )
	ROM_LOAD( "rockclimber_m.008", 0x380000, 0x80000, CRC(8cf6b4c2) SHA1(4c36c217b83c82acfdd615f5547bf597af7b8833) )
ROM_END

ROM_START( rclimb_2 ) // 040823
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rc_m_040823.rom", 0x00000, 0x40000, SHA1(31cf4d7f50102d35556817273893182e30c9a70c) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "rockclimber_m.001", 0x000000, 0x80000, CRC(934f18c7) SHA1(da3a7cddc68e104d415d947e89c0e7f0d067c056) )
	ROM_LOAD( "rockclimber_m.002", 0x100000, 0x80000, CRC(7364bd2b) SHA1(c0edfd3b8de813c95fe5d6072662fa0e39fec89e) )
	ROM_LOAD( "rockclimber_m.003", 0x200000, 0x80000, CRC(e7befb17) SHA1(8a214680142cd657784a667ab3f6422165fea224) )
	ROM_LOAD( "rockclimber_m.004", 0x300000, 0x80000, CRC(dc6d43a0) SHA1(62fc47136775f3fa9369857ec91fe897a1f1ebd6) )
	ROM_LOAD( "rockclimber_m.005", 0x080000, 0x80000, CRC(ea127c3d) SHA1(a6391eed69a4723b68d727f59b6baebe51633e66) )
	ROM_LOAD( "rockclimber_m.006", 0x180000, 0x80000, CRC(277fa273) SHA1(6320e6c5b5e48dc451cc48189054c42d85e8ccc1) )
	ROM_LOAD( "rockclimber_m.007", 0x280000, 0x80000, CRC(3ca7f69a) SHA1(878cca181d915dc3548d5285a4bbb51aef31a64e) )
	ROM_LOAD( "rockclimber_m.008", 0x380000, 0x80000, CRC(8cf6b4c2) SHA1(4c36c217b83c82acfdd615f5547bf597af7b8833) )
ROM_END

ROM_START( rclimb_3 ) // 040827
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rc_m_040827.rom", 0x00000, 0x40000, CRC(3ba55647) SHA1(56e96be0d9782da4b3d5d911ea67962257626ae0) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "rockclimber_m.001", 0x000000, 0x80000, CRC(934f18c7) SHA1(da3a7cddc68e104d415d947e89c0e7f0d067c056) )
	ROM_LOAD( "rockclimber_m.002", 0x100000, 0x80000, CRC(7364bd2b) SHA1(c0edfd3b8de813c95fe5d6072662fa0e39fec89e) )
	ROM_LOAD( "rockclimber_m.003", 0x200000, 0x80000, CRC(e7befb17) SHA1(8a214680142cd657784a667ab3f6422165fea224) )
	ROM_LOAD( "rockclimber_m.004", 0x300000, 0x80000, CRC(dc6d43a0) SHA1(62fc47136775f3fa9369857ec91fe897a1f1ebd6) )
	ROM_LOAD( "rockclimber_m.005", 0x080000, 0x80000, CRC(ea127c3d) SHA1(a6391eed69a4723b68d727f59b6baebe51633e66) )
	ROM_LOAD( "rockclimber_m.006", 0x180000, 0x80000, CRC(277fa273) SHA1(6320e6c5b5e48dc451cc48189054c42d85e8ccc1) )
	ROM_LOAD( "rockclimber_m.007", 0x280000, 0x80000, CRC(3ca7f69a) SHA1(878cca181d915dc3548d5285a4bbb51aef31a64e) )
	ROM_LOAD( "rockclimber_m.008", 0x380000, 0x80000, CRC(8cf6b4c2) SHA1(4c36c217b83c82acfdd615f5547bf597af7b8833) )
ROM_END

ROM_START( rclimb_4 ) // 070322
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rc_070322.rom", 0x00000, 0x40000, CRC(200fcc35) SHA1(a9f71c5a3cb328f6522436693364d85586cca796) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "rockclimber.001",   0x000000, 0x80000, CRC(2c6c10b6) SHA1(b2f96fb90a5f2150d84fe5012414c74fcbde81d1) )
	ROM_LOAD( "rockclimber.002",   0x100000, 0x80000, CRC(7def062a) SHA1(d418f5e8b80680ecdc10159322c83d298c96cf4d) )
	ROM_LOAD( "rockclimber.003",   0x200000, 0x80000, CRC(313a6490) SHA1(96430117f45cc0a34a7dbc51c0df032595451390) )
	ROM_LOAD( "rockclimber.004",   0x300000, 0x80000, CRC(0cfaa33f) SHA1(df289b816fdb094228d518e823a3e660f5b8a0b6) )
	ROM_LOAD( "rockclimber_m.005", 0x080000, 0x80000, CRC(ea127c3d) SHA1(a6391eed69a4723b68d727f59b6baebe51633e66) )
	ROM_LOAD( "rockclimber_m.006", 0x180000, 0x80000, CRC(277fa273) SHA1(6320e6c5b5e48dc451cc48189054c42d85e8ccc1) )
	ROM_LOAD( "rockclimber_m.007", 0x280000, 0x80000, CRC(3ca7f69a) SHA1(878cca181d915dc3548d5285a4bbb51aef31a64e) )
	ROM_LOAD( "rockclimber_m.008", 0x380000, 0x80000, CRC(8cf6b4c2) SHA1(4c36c217b83c82acfdd615f5547bf597af7b8833) )
ROM_END

ROM_START( rclimb_5 ) // 070621
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rc_070621.rom", 0x00000, 0x40000, CRC(7f5223a6) SHA1(9aaed66ce6cd12faad09e2b74f8ae764a9707be7) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "rockclimber.001",   0x000000, 0x80000, CRC(2c6c10b6) SHA1(b2f96fb90a5f2150d84fe5012414c74fcbde81d1) )
	ROM_LOAD( "rockclimber.002",   0x100000, 0x80000, CRC(7def062a) SHA1(d418f5e8b80680ecdc10159322c83d298c96cf4d) )
	ROM_LOAD( "rockclimber.003",   0x200000, 0x80000, CRC(313a6490) SHA1(96430117f45cc0a34a7dbc51c0df032595451390) )
	ROM_LOAD( "rockclimber.004",   0x300000, 0x80000, CRC(0cfaa33f) SHA1(df289b816fdb094228d518e823a3e660f5b8a0b6) )
	ROM_LOAD( "rockclimber_m.005", 0x080000, 0x80000, CRC(ea127c3d) SHA1(a6391eed69a4723b68d727f59b6baebe51633e66) )
	ROM_LOAD( "rockclimber_m.006", 0x180000, 0x80000, CRC(277fa273) SHA1(6320e6c5b5e48dc451cc48189054c42d85e8ccc1) )
	ROM_LOAD( "rockclimber_m.007", 0x280000, 0x80000, CRC(3ca7f69a) SHA1(878cca181d915dc3548d5285a4bbb51aef31a64e) )
	ROM_LOAD( "rockclimber_m.008", 0x380000, 0x80000, CRC(8cf6b4c2) SHA1(4c36c217b83c82acfdd615f5547bf597af7b8833) )
ROM_END



/*********************************************************
   Sweet Life
**********************************************************/

ROM_START( sweetl ) // 041220
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "sl_m_041220.rom", 0x00000, 0x40000, CRC(851b85c6) SHA1(a5db94d94fe82d06f3fac1c16aed5358fcb92f29) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "sweetlife.001", 0x000000, 0x80000, CRC(a096c786) SHA1(81f6b083cb089e9412a8506889196354c670d945) )
	ROM_LOAD( "sweetlife.002", 0x100000, 0x80000, CRC(c5e1e22c) SHA1(973ad27681a0f3beee7084b1b85fc9deb79d638e) )
	ROM_LOAD( "sweetlife.003", 0x200000, 0x80000, CRC(af335323) SHA1(b8afdce231a8ec0f313cc47e00a27f05461bbbc4) )
	ROM_LOAD( "sweetlife.004", 0x300000, 0x80000, CRC(a35c7503) SHA1(78f7a868660bbaa066e8e9e341db52018aaf3af1) )
	ROM_LOAD( "sweetlife.005", 0x080000, 0x80000, CRC(e2d6b632) SHA1(65d05e55671b8c335cae2dfbf6a6f5bd8cc90e2c) )
	ROM_LOAD( "sweetlife.006", 0x180000, 0x80000, CRC(d34e0905) SHA1(cc4afe64fb9052a31f759be41ff07a727e0a9093) )
	ROM_LOAD( "sweetlife.007", 0x280000, 0x80000, CRC(978b67bb) SHA1(87357d5832588f00272bd76df736c06c599f3853) )
	ROM_LOAD( "sweetlife.008", 0x380000, 0x80000, CRC(75954355) SHA1(e6ef2b70d859b61e8e3d1751de8558b8778e502d) )
ROM_END

ROM_START( sweetl_2 ) // 070412
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "sl_070412.rom", 0x00000, 0x40000, CRC(2d2cd8ec) SHA1(40f7778c6eb4681452635249a708e9dbc15c9045) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "sweetlife.001", 0x000000, 0x80000, CRC(a096c786) SHA1(81f6b083cb089e9412a8506889196354c670d945) )
	ROM_LOAD( "sweetlife.002", 0x100000, 0x80000, CRC(c5e1e22c) SHA1(973ad27681a0f3beee7084b1b85fc9deb79d638e) )
	ROM_LOAD( "sweetlife.003", 0x200000, 0x80000, CRC(af335323) SHA1(b8afdce231a8ec0f313cc47e00a27f05461bbbc4) )
	ROM_LOAD( "sweetlife.004", 0x300000, 0x80000, CRC(a35c7503) SHA1(78f7a868660bbaa066e8e9e341db52018aaf3af1) )
	ROM_LOAD( "sweetlife.005", 0x080000, 0x80000, CRC(e2d6b632) SHA1(65d05e55671b8c335cae2dfbf6a6f5bd8cc90e2c) )
	ROM_LOAD( "sweetlife.006", 0x180000, 0x80000, CRC(d34e0905) SHA1(cc4afe64fb9052a31f759be41ff07a727e0a9093) )
	ROM_LOAD( "sweetlife.007", 0x280000, 0x80000, CRC(978b67bb) SHA1(87357d5832588f00272bd76df736c06c599f3853) )
	ROM_LOAD( "sweetlife.008", 0x380000, 0x80000, CRC(75954355) SHA1(e6ef2b70d859b61e8e3d1751de8558b8778e502d) )
ROM_END


/*********************************************************
   Sweet Life 2
**********************************************************/

ROM_START( sweetl2 ) // 071217
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "sl2_071217.rom", 0x00000, 0x40000, CRC(b6299b02) SHA1(7051f9034bf3181d6f3dc66c0048c4a57685d2a8) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "sweetlife2.001", 0x000000, 0x80000, CRC(b1e6157c) SHA1(e042aaaf85b13865d56d1709be280f5a3d5c95e3) )
	ROM_LOAD( "sweetlife2.002", 0x100000, 0x80000, CRC(8bc54047) SHA1(9179cf02376fec77ae50b5603de66754237698f8) )
	ROM_LOAD( "sweetlife2.003", 0x200000, 0x80000, CRC(709c5d34) SHA1(d702576d70ebef667c77376effecc02abbaaa8cd) )
	ROM_LOAD( "sweetlife2.004", 0x300000, 0x80000, CRC(7bcfa2ce) SHA1(8df894e9b7afa52e47e4dabd1802e878bbbeaa80) )
	ROM_LOAD( "sweetlife2.005", 0x080000, 0x80000, CRC(5c385d43) SHA1(ddb362e5894d146ce90acbaafe9dd7aad2a7c242) )
	ROM_LOAD( "sweetlife2.006", 0x180000, 0x80000, CRC(868fe1cb) SHA1(692679f8242950e009c30cc084c4ddc5d1963502) )
	ROM_LOAD( "sweetlife2.007", 0x280000, 0x80000, CRC(6ce87282) SHA1(586e08994db4ca2b967d47b16ba5b458e240d30f) )
	ROM_LOAD( "sweetlife2.008", 0x380000, 0x80000, CRC(c2ad2b74) SHA1(c78e3ca5d15acb17ee671d2205405f287ad9c464) )
ROM_END

ROM_START( sweetl2_2 ) // 080320
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "sl2_m_080320.rom", 0x00000, 0x40000, CRC(78669405) SHA1(149990bee56d0f94ce1bea17883cbf5545925eb7) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "sweetlife2.001", 0x000000, 0x80000, CRC(b1e6157c) SHA1(e042aaaf85b13865d56d1709be280f5a3d5c95e3) )
	ROM_LOAD( "sweetlife2.002", 0x100000, 0x80000, CRC(8bc54047) SHA1(9179cf02376fec77ae50b5603de66754237698f8) )
	ROM_LOAD( "sweetlife2.003", 0x200000, 0x80000, CRC(709c5d34) SHA1(d702576d70ebef667c77376effecc02abbaaa8cd) )
	ROM_LOAD( "sweetlife2.004", 0x300000, 0x80000, CRC(7bcfa2ce) SHA1(8df894e9b7afa52e47e4dabd1802e878bbbeaa80) )
	ROM_LOAD( "sweetlife2.005", 0x080000, 0x80000, CRC(5c385d43) SHA1(ddb362e5894d146ce90acbaafe9dd7aad2a7c242) )
	ROM_LOAD( "sweetlife2.006", 0x180000, 0x80000, CRC(868fe1cb) SHA1(692679f8242950e009c30cc084c4ddc5d1963502) )
	ROM_LOAD( "sweetlife2.007", 0x280000, 0x80000, CRC(6ce87282) SHA1(586e08994db4ca2b967d47b16ba5b458e240d30f) )
	ROM_LOAD( "sweetlife2.008", 0x380000, 0x80000, CRC(c2ad2b74) SHA1(c78e3ca5d15acb17ee671d2205405f287ad9c464) )
ROM_END


/*********************************************************
   Resident

    "Russia" sets use different gfx roms 1-4.
        The official list of hashes shows both roms.

**********************************************************/

ROM_START( resdnt ) // 040415
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rs_m_040415.rom", 0x00000, 0x40000, CRC(2E06F70C) SHA1(b9f07bc2765d4f366e548007e51b9f605c884ba1) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "resident_m.001", 0x000000, 0x80000, CRC(e0645da6) SHA1(dd72f4830d8011f603aa6d430f34ac2598005281) )
	ROM_LOAD( "resident_m.002", 0x100000, 0x80000, CRC(dd8de247) SHA1(498c5b931ce65e289f52d8864b603166f81e3dc4) )
	ROM_LOAD( "resident_m.003", 0x200000, 0x80000, CRC(0d346ec2) SHA1(e2456b28825c54c5e16829525627c40611c0083d) )
	ROM_LOAD( "resident_m.004", 0x300000, 0x80000, CRC(1f95aad9) SHA1(51d003288d5ff23b3c981fbaa99d29b66dd2c101) )
	ROM_LOAD( "resident_m.005", 0x080000, 0x80000, CRC(0cfe7d44) SHA1(9f0e4925e815ff9f79188f18e78c0a7b377daa3f) )
	ROM_LOAD( "resident_m.006", 0x180000, 0x80000, CRC(7437904f) SHA1(630c79cd6a990ce7658a1ffabba5a27efba985a1) )
	ROM_LOAD( "resident_m.007", 0x280000, 0x80000, CRC(6e94728a) SHA1(ab414879cb957d9bc8d653b5e3bb2bbf91139ec0) )
	ROM_LOAD( "resident_m.008", 0x380000, 0x80000, CRC(a9f55043) SHA1(4771df3d45bdc0a21b1c479f45e09ac5bab6c94f) )
ROM_END

ROM_START( resdnt_2 ) // 040513
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rs_m_040513.rom", 0x00000, 0x40000, CRC(95f74cb3) SHA1(2e4862ac0ad86899b8ce12580ebd217dfb74f6a2) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "resident_m.001", 0x000000, 0x80000, CRC(e0645da6) SHA1(dd72f4830d8011f603aa6d430f34ac2598005281) )
	ROM_LOAD( "resident_m.002", 0x100000, 0x80000, CRC(dd8de247) SHA1(498c5b931ce65e289f52d8864b603166f81e3dc4) )
	ROM_LOAD( "resident_m.003", 0x200000, 0x80000, CRC(0d346ec2) SHA1(e2456b28825c54c5e16829525627c40611c0083d) )
	ROM_LOAD( "resident_m.004", 0x300000, 0x80000, CRC(1f95aad9) SHA1(51d003288d5ff23b3c981fbaa99d29b66dd2c101) )
	ROM_LOAD( "resident_m.005", 0x080000, 0x80000, CRC(0cfe7d44) SHA1(9f0e4925e815ff9f79188f18e78c0a7b377daa3f) )
	ROM_LOAD( "resident_m.006", 0x180000, 0x80000, CRC(7437904f) SHA1(630c79cd6a990ce7658a1ffabba5a27efba985a1) )
	ROM_LOAD( "resident_m.007", 0x280000, 0x80000, CRC(6e94728a) SHA1(ab414879cb957d9bc8d653b5e3bb2bbf91139ec0) )
	ROM_LOAD( "resident_m.008", 0x380000, 0x80000, CRC(a9f55043) SHA1(4771df3d45bdc0a21b1c479f45e09ac5bab6c94f) )
ROM_END

ROM_START( resdnt_3 ) // 070222
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rs_070222.rom", 0x00000, 0x40000, CRC(8aec434e) SHA1(e83c57bbde3379b4d09aa615f2f3fc82d5c5a3cb) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "resident.001",   0x000000, 0x80000, CRC(5a1a24f1) SHA1(da601ed84603b880d2ce4b98784067531d5294b2) )
	ROM_LOAD( "resident.002",   0x100000, 0x80000, CRC(9a8bfa95) SHA1(392690b7031ce2c8ff87843befeefa0e68ae8fce) )
	ROM_LOAD( "resident.003",   0x200000, 0x80000, CRC(0d0db860) SHA1(93bc2b0bb51d754e19f58d8388c837ba2c7ba5ee) )
	ROM_LOAD( "resident.004",   0x300000, 0x80000, CRC(1d3a5b92) SHA1(c8bebca6beb225911c26e30e090d66b724733d59) )
	ROM_LOAD( "resident_m.005", 0x080000, 0x80000, CRC(0cfe7d44) SHA1(9f0e4925e815ff9f79188f18e78c0a7b377daa3f) )
	ROM_LOAD( "resident_m.006", 0x180000, 0x80000, CRC(7437904f) SHA1(630c79cd6a990ce7658a1ffabba5a27efba985a1) )
	ROM_LOAD( "resident_m.007", 0x280000, 0x80000, CRC(6e94728a) SHA1(ab414879cb957d9bc8d653b5e3bb2bbf91139ec0) )
	ROM_LOAD( "resident_m.008", 0x380000, 0x80000, CRC(a9f55043) SHA1(4771df3d45bdc0a21b1c479f45e09ac5bab6c94f) )
ROM_END



/*********************************************************
   Roll Fruit

    Roms 4-8 were changed after the 080327 update.
        The official list of hashes shows both set of roms.

**********************************************************/

ROM_START( rollfr ) // 030821
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rf5-030821.rom", 0x00000, 0x40000, SHA1(5e9c2235ea4207086db23870993d8e28356c9eb8) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "roll_fruit5.001",  0x000000, 0x80000, CRC(caeb1fc3) SHA1(14b9f99f892849faecb3327e572dc134e1065463) )
	ROM_LOAD( "roll_fruit5.002",  0x100000, 0x80000, CRC(f017c200) SHA1(a247bbbd1c4ca99978dcc705bd62590815a891f2) )
	ROM_LOAD( "roll_fruit5.003",  0x200000, 0x80000, CRC(a2d6df11) SHA1(c2553136252aebe3b3ce0b5c33e740d0e27fb7b2) )
	ROM_LOAD( "roll_fruit5.004",  0x300000, 0x80000, CRC(cd3c928a) SHA1(4c50ce17bd5714149eae91279a0133059397b776) )
	ROM_LOAD( "roll_fruit5.005",  0x080000, 0x80000, CRC(24c7362a) SHA1(684b7b370fcad07bf74bddffaf432bd52e5d29e2) )
	ROM_LOAD( "roll_fruit5.006",  0x180000, 0x80000, CRC(d6a61904) SHA1(73700e88358ed9bccbb63643b7daaff416737e43) )
	ROM_LOAD( "roll_fruit5.007",  0x280000, 0x80000, CRC(81e3480b) SHA1(c0f006cf2a4747359cb79f14976ac3411951af1c) )
	ROM_LOAD( "roll_fruit5.008",  0x380000, 0x80000, CRC(ed3558b8) SHA1(8ec808069053f0c07d81c45090b2ba22ef8e9c32) )
ROM_END

ROM_START( rollfr_2 ) // 040318
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rf5-040318.rom", 0x00000, 0x40000, CRC(d8efd395) SHA1(71edd1541df400fef97abacabb10d882ace4c8b0) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "roll_fruit5.001",  0x000000, 0x80000, CRC(caeb1fc3) SHA1(14b9f99f892849faecb3327e572dc134e1065463) )
	ROM_LOAD( "roll_fruit5.002",  0x100000, 0x80000, CRC(f017c200) SHA1(a247bbbd1c4ca99978dcc705bd62590815a891f2) )
	ROM_LOAD( "roll_fruit5.003",  0x200000, 0x80000, CRC(a2d6df11) SHA1(c2553136252aebe3b3ce0b5c33e740d0e27fb7b2) )
	ROM_LOAD( "roll_fruit5.004",  0x300000, 0x80000, CRC(cd3c928a) SHA1(4c50ce17bd5714149eae91279a0133059397b776) )
	ROM_LOAD( "roll_fruit5.005",  0x080000, 0x80000, CRC(24c7362a) SHA1(684b7b370fcad07bf74bddffaf432bd52e5d29e2) )
	ROM_LOAD( "roll_fruit5.006",  0x180000, 0x80000, CRC(d6a61904) SHA1(73700e88358ed9bccbb63643b7daaff416737e43) )
	ROM_LOAD( "roll_fruit5.007",  0x280000, 0x80000, CRC(81e3480b) SHA1(c0f006cf2a4747359cb79f14976ac3411951af1c) )
	ROM_LOAD( "roll_fruit5.008",  0x380000, 0x80000, CRC(ed3558b8) SHA1(8ec808069053f0c07d81c45090b2ba22ef8e9c32) )
ROM_END

ROM_START( rollfr_3 ) // 080327
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rf1-080327.rom", 0x00000, 0x40000, CRC(ea0b402d) SHA1(cf1ae74c90ca1609cf911a2127130a9d56625db0) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "roll_fruit1.001", 0x000000, 0x80000, CRC(caeb1fc3) SHA1(14b9f99f892849faecb3327e572dc134e1065463) )
	ROM_LOAD( "roll_fruit1.002", 0x100000, 0x80000, CRC(f017c200) SHA1(a247bbbd1c4ca99978dcc705bd62590815a891f2) )
	ROM_LOAD( "roll_fruit1.003", 0x200000, 0x80000, CRC(a2d6df11) SHA1(c2553136252aebe3b3ce0b5c33e740d0e27fb7b2) )
	ROM_LOAD( "roll_fruit1.004", 0x300000, 0x80000, CRC(cd3c928a) SHA1(4c50ce17bd5714149eae91279a0133059397b776) )
	ROM_LOAD( "roll_fruit1.005", 0x080000, 0x80000, CRC(1711ab4d) SHA1(902854770bf5ae3de6c9e3100a31399c7ab40cbb) )
	ROM_LOAD( "roll_fruit1.006", 0x180000, 0x80000, CRC(ef2639fb) SHA1(90c67916380f9c49e284aac42f899cae86982829) )
	ROM_LOAD( "roll_fruit1.007", 0x280000, 0x80000, CRC(0305c231) SHA1(f5b25e8716f163eee1f40c737240a8db7059268f) )
	ROM_LOAD( "roll_fruit1.008", 0x380000, 0x80000, CRC(437fe141) SHA1(96783757a0b1af560954f2658b940bec1a4610b2) )
ROM_END

ROM_START( rollfr_4 ) // 080331, bank D0
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "rf1-080331.rom", 0x00000, 0x40000, CRC(19b84292) SHA1(f01d907cc8b1716eecb5c28b93ee9712073e1e8d) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "roll_fruit1.001", 0x000000, 0x80000, CRC(caeb1fc3) SHA1(14b9f99f892849faecb3327e572dc134e1065463) )
	ROM_LOAD( "roll_fruit1.002", 0x100000, 0x80000, CRC(f017c200) SHA1(a247bbbd1c4ca99978dcc705bd62590815a891f2) )
	ROM_LOAD( "roll_fruit1.003", 0x200000, 0x80000, CRC(a2d6df11) SHA1(c2553136252aebe3b3ce0b5c33e740d0e27fb7b2) )
	ROM_LOAD( "roll_fruit1.004", 0x300000, 0x80000, CRC(cd3c928a) SHA1(4c50ce17bd5714149eae91279a0133059397b776) )
	ROM_LOAD( "roll_fruit1.005", 0x080000, 0x80000, CRC(1711ab4d) SHA1(902854770bf5ae3de6c9e3100a31399c7ab40cbb) )
	ROM_LOAD( "roll_fruit1.006", 0x180000, 0x80000, CRC(ef2639fb) SHA1(90c67916380f9c49e284aac42f899cae86982829) )
	ROM_LOAD( "roll_fruit1.007", 0x280000, 0x80000, CRC(0305c231) SHA1(f5b25e8716f163eee1f40c737240a8db7059268f) )
	ROM_LOAD( "roll_fruit1.008", 0x380000, 0x80000, CRC(437fe141) SHA1(96783757a0b1af560954f2658b940bec1a4610b2) )
ROM_END



/*********************************************************
   Island
**********************************************************/

ROM_START( island ) // 050713
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "is_m_050713.rom", 0x00000, 0x40000, CRC(26c7013e) SHA1(5d604f5b4859e9e82830424a1e21f32a9e49bf34) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "island.001", 0x000000, 0x80000, CRC(dbe8cdda) SHA1(4747cf0d85afdef22d3ba9fa5e75b39548725745) )
	ROM_LOAD( "island.002", 0x100000, 0x80000, CRC(64064745) SHA1(91a7bc7204a8f7a7512eeaf4906da20a9f587565) )
	ROM_LOAD( "island.003", 0x200000, 0x80000, CRC(1d993f68) SHA1(b0459d3941d50668f7533909e3f3da91453d3efd) )
	ROM_LOAD( "island.004", 0x300000, 0x80000, CRC(a4739404) SHA1(8f7ffcc13dcb35adfa8060ab1930d07195b6110c) )
	ROM_LOAD( "island.005", 0x080000, 0x80000, CRC(d016eb31) SHA1(a84f18af470f72730b241b9031cd6131c8a03db2) )
	ROM_LOAD( "island.006", 0x180000, 0x80000, CRC(0faaa968) SHA1(0f05546e6e0559e24c6afdde65b3feeb66b6adff) )
	ROM_LOAD( "island.007", 0x280000, 0x80000, CRC(d7277a6c) SHA1(d96a0befc965ad22087381982305d68208978a7e) )
	ROM_LOAD( "island.008", 0x380000, 0x80000, CRC(ac6fba48) SHA1(64dd03d624f16da52bc7fa0702246e91ae39a806) )
ROM_END

ROM_START( island_2 ) // 070409
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "is_070409.rom", 0x00000, 0x40000, CRC(b6790aeb) SHA1(b80d0e4ae003473ac623183439737d6159f59274) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "island.001", 0x000000, 0x80000, CRC(dbe8cdda) SHA1(4747cf0d85afdef22d3ba9fa5e75b39548725745) )
	ROM_LOAD( "island.002", 0x100000, 0x80000, CRC(64064745) SHA1(91a7bc7204a8f7a7512eeaf4906da20a9f587565) )
	ROM_LOAD( "island.003", 0x200000, 0x80000, CRC(1d993f68) SHA1(b0459d3941d50668f7533909e3f3da91453d3efd) )
	ROM_LOAD( "island.004", 0x300000, 0x80000, CRC(a4739404) SHA1(8f7ffcc13dcb35adfa8060ab1930d07195b6110c) )
	ROM_LOAD( "island.005", 0x080000, 0x80000, CRC(d016eb31) SHA1(a84f18af470f72730b241b9031cd6131c8a03db2) )
	ROM_LOAD( "island.006", 0x180000, 0x80000, CRC(0faaa968) SHA1(0f05546e6e0559e24c6afdde65b3feeb66b6adff) )
	ROM_LOAD( "island.007", 0x280000, 0x80000, CRC(d7277a6c) SHA1(d96a0befc965ad22087381982305d68208978a7e) )
	ROM_LOAD( "island.008", 0x380000, 0x80000, CRC(ac6fba48) SHA1(64dd03d624f16da52bc7fa0702246e91ae39a806) )
ROM_END


/*********************************************************
   Island 2
**********************************************************/

ROM_START( island2 ) // 060529
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "is2_m_060529.rom", 0x00000, 0x40000, CRC(4ccddabd) SHA1(ae5902734488b7ddfa0f7bbf9b800d25b2b657b5) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "island2.001", 0x000000, 0x80000, CRC(f8dd9fe9) SHA1(0cf67fbca107b255011fded6390507d12cbac514) )
	ROM_LOAD( "island2.002", 0x100000, 0x80000, CRC(4f9607c0) SHA1(cd3e7b4a88f46231a115c9d18a26b5e30fea74e4) )
	ROM_LOAD( "island2.003", 0x200000, 0x80000, CRC(bceccdba) SHA1(5cf3b51ccfe317ca57d770bff0204b0ee83d1173) )
	ROM_LOAD( "island2.004", 0x300000, 0x80000, CRC(15fdecc7) SHA1(6882ea10f117c85544df51d9abd67ef52db91d95) )
	ROM_LOAD( "island2.005", 0x080000, 0x80000, CRC(2d4905aa) SHA1(c4a1e4db61e8af6cf0fb70aabe5e3896ab5227ca) )
	ROM_LOAD( "island2.006", 0x180000, 0x80000, CRC(55e285d9) SHA1(ba58963441c65220700cd8057e6afe3f5f8faa4f) )
	ROM_LOAD( "island2.007", 0x280000, 0x80000, CRC(edd72be6) SHA1(fb1e63f59e8565c23ae43630fa572fbc022c878f) )
	ROM_LOAD( "island2.008", 0x380000, 0x80000, CRC(c336d608) SHA1(55391183c6d95ecea81354efa70641350860d1f5) )
ROM_END

ROM_START( island2_2 ) // 061214
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "is2_m_061214.rom", 0x00000, 0x40000, SHA1(dd416d831c3773f044b355efcb1121f7eb81932b) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "island2.001", 0x000000, 0x80000, CRC(f8dd9fe9) SHA1(0cf67fbca107b255011fded6390507d12cbac514) )
	ROM_LOAD( "island2.002", 0x100000, 0x80000, CRC(4f9607c0) SHA1(cd3e7b4a88f46231a115c9d18a26b5e30fea74e4) )
	ROM_LOAD( "island2.003", 0x200000, 0x80000, CRC(bceccdba) SHA1(5cf3b51ccfe317ca57d770bff0204b0ee83d1173) )
	ROM_LOAD( "island2.004", 0x300000, 0x80000, CRC(15fdecc7) SHA1(6882ea10f117c85544df51d9abd67ef52db91d95) )
	ROM_LOAD( "island2.005", 0x080000, 0x80000, CRC(2d4905aa) SHA1(c4a1e4db61e8af6cf0fb70aabe5e3896ab5227ca) )
	ROM_LOAD( "island2.006", 0x180000, 0x80000, CRC(55e285d9) SHA1(ba58963441c65220700cd8057e6afe3f5f8faa4f) )
	ROM_LOAD( "island2.007", 0x280000, 0x80000, CRC(edd72be6) SHA1(fb1e63f59e8565c23ae43630fa572fbc022c878f) )
	ROM_LOAD( "island2.008", 0x380000, 0x80000, CRC(c336d608) SHA1(55391183c6d95ecea81354efa70641350860d1f5) )
ROM_END

ROM_START( island2_3 ) // 061218
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "is2_m_061218.rom", 0x00000, 0x40000, CRC(f16cecc5) SHA1(c9caa265295837deb83d1454daf586f3d0be0ee6) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "island2.001", 0x000000, 0x80000, CRC(f8dd9fe9) SHA1(0cf67fbca107b255011fded6390507d12cbac514) )
	ROM_LOAD( "island2.002", 0x100000, 0x80000, CRC(4f9607c0) SHA1(cd3e7b4a88f46231a115c9d18a26b5e30fea74e4) )
	ROM_LOAD( "island2.003", 0x200000, 0x80000, CRC(bceccdba) SHA1(5cf3b51ccfe317ca57d770bff0204b0ee83d1173) )
	ROM_LOAD( "island2.004", 0x300000, 0x80000, CRC(15fdecc7) SHA1(6882ea10f117c85544df51d9abd67ef52db91d95) )
	ROM_LOAD( "island2.005", 0x080000, 0x80000, CRC(2d4905aa) SHA1(c4a1e4db61e8af6cf0fb70aabe5e3896ab5227ca) )
	ROM_LOAD( "island2.006", 0x180000, 0x80000, CRC(55e285d9) SHA1(ba58963441c65220700cd8057e6afe3f5f8faa4f) )
	ROM_LOAD( "island2.007", 0x280000, 0x80000, CRC(edd72be6) SHA1(fb1e63f59e8565c23ae43630fa572fbc022c878f) )
	ROM_LOAD( "island2.008", 0x380000, 0x80000, CRC(c336d608) SHA1(55391183c6d95ecea81354efa70641350860d1f5) )
ROM_END

ROM_START( island2_4 ) // 070205
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "is2_070205.rom", 0x00000, 0x40000, CRC(bc3e619b) SHA1(4a8e01d60f73e58b2c6cfa739a0a8fce60e565cc) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "island2.001", 0x000000, 0x80000, CRC(f8dd9fe9) SHA1(0cf67fbca107b255011fded6390507d12cbac514) )
	ROM_LOAD( "island2.002", 0x100000, 0x80000, CRC(4f9607c0) SHA1(cd3e7b4a88f46231a115c9d18a26b5e30fea74e4) )
	ROM_LOAD( "island2.003", 0x200000, 0x80000, CRC(bceccdba) SHA1(5cf3b51ccfe317ca57d770bff0204b0ee83d1173) )
	ROM_LOAD( "island2.004", 0x300000, 0x80000, CRC(15fdecc7) SHA1(6882ea10f117c85544df51d9abd67ef52db91d95) )
	ROM_LOAD( "island2.005", 0x080000, 0x80000, CRC(2d4905aa) SHA1(c4a1e4db61e8af6cf0fb70aabe5e3896ab5227ca) )
	ROM_LOAD( "island2.006", 0x180000, 0x80000, CRC(55e285d9) SHA1(ba58963441c65220700cd8057e6afe3f5f8faa4f) )
	ROM_LOAD( "island2.007", 0x280000, 0x80000, CRC(edd72be6) SHA1(fb1e63f59e8565c23ae43630fa572fbc022c878f) )
	ROM_LOAD( "island2.008", 0x380000, 0x80000, CRC(c336d608) SHA1(55391183c6d95ecea81354efa70641350860d1f5) )
ROM_END


/*********************************************************
   Pirate
**********************************************************/

ROM_START( pirate ) // 051229
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "pr_m_051229.rom", 0x00000, 0x40000, SHA1(d1286cba474ccbbff8358ba2fd6917d43d101674) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "pirate.001", 0x000000, 0x80000, CRC(d2199619) SHA1(8c67ef7d0e305ae0783302ba9a1cb56cdcf4bc09) )
	ROM_LOAD( "pirate.002", 0x100000, 0x80000, CRC(ce5c6548) SHA1(ef1cd6ae36cc1abcf010762dc89a255cd817d016) )
	ROM_LOAD( "pirate.003", 0x200000, 0x80000, CRC(d6a8338d) SHA1(6a0e41309dc909decf8bd49cf13cbeca95f0314a) )
	ROM_LOAD( "pirate.004", 0x300000, 0x80000, CRC(590b8cf6) SHA1(b2778f6e1b7bcf7f33ced43f999eff983e5a6af4) )
	ROM_LOAD( "pirate.005", 0x080000, 0x80000, CRC(bf9f1267) SHA1(b0947bd7d31301ffbe80cbaf1e96c3476f6f9ca3) )
	ROM_LOAD( "pirate.006", 0x180000, 0x80000, CRC(b0cdf7eb) SHA1(cf6bd20fb40cf0d87eeb6f1502fb73d9760c9140) )
	ROM_LOAD( "pirate.007", 0x280000, 0x80000, CRC(6c4a9510) SHA1(e10bf8475ff7c73ba90b904b9214b285a5b2669f) )
	ROM_LOAD( "pirate.008", 0x380000, 0x80000, CRC(cc2edac2) SHA1(24bacd9e092a83945a8def3a254ec66758d71ff5) )
ROM_END

ROM_START( pirate_2 ) // 060210
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "pr_m_060210.rom", 0x00000, 0x40000, CRC(5684d67d) SHA1(4cbd103bcd071df26830d56760ef477b9a652857) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "pirate.001", 0x000000, 0x80000, CRC(d2199619) SHA1(8c67ef7d0e305ae0783302ba9a1cb56cdcf4bc09) )
	ROM_LOAD( "pirate.002", 0x100000, 0x80000, CRC(ce5c6548) SHA1(ef1cd6ae36cc1abcf010762dc89a255cd817d016) )
	ROM_LOAD( "pirate.003", 0x200000, 0x80000, CRC(d6a8338d) SHA1(6a0e41309dc909decf8bd49cf13cbeca95f0314a) )
	ROM_LOAD( "pirate.004", 0x300000, 0x80000, CRC(590b8cf6) SHA1(b2778f6e1b7bcf7f33ced43f999eff983e5a6af4) )
	ROM_LOAD( "pirate.005", 0x080000, 0x80000, CRC(bf9f1267) SHA1(b0947bd7d31301ffbe80cbaf1e96c3476f6f9ca3) )
	ROM_LOAD( "pirate.006", 0x180000, 0x80000, CRC(b0cdf7eb) SHA1(cf6bd20fb40cf0d87eeb6f1502fb73d9760c9140) )
	ROM_LOAD( "pirate.007", 0x280000, 0x80000, CRC(6c4a9510) SHA1(e10bf8475ff7c73ba90b904b9214b285a5b2669f) )
	ROM_LOAD( "pirate.008", 0x380000, 0x80000, CRC(cc2edac2) SHA1(24bacd9e092a83945a8def3a254ec66758d71ff5) )
ROM_END

ROM_START( pirate_3 ) // 060803
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "pr_m_060803.rom", 0x00000, 0x40000, CRC(1de68707) SHA1(99dd3c5186ed8ba6c17e9f5479df93173da527e0) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "pirate.001", 0x000000, 0x80000, CRC(d2199619) SHA1(8c67ef7d0e305ae0783302ba9a1cb56cdcf4bc09) )
	ROM_LOAD( "pirate.002", 0x100000, 0x80000, CRC(ce5c6548) SHA1(ef1cd6ae36cc1abcf010762dc89a255cd817d016) )
	ROM_LOAD( "pirate.003", 0x200000, 0x80000, CRC(d6a8338d) SHA1(6a0e41309dc909decf8bd49cf13cbeca95f0314a) )
	ROM_LOAD( "pirate.004", 0x300000, 0x80000, CRC(590b8cf6) SHA1(b2778f6e1b7bcf7f33ced43f999eff983e5a6af4) )
	ROM_LOAD( "pirate.005", 0x080000, 0x80000, CRC(bf9f1267) SHA1(b0947bd7d31301ffbe80cbaf1e96c3476f6f9ca3) )
	ROM_LOAD( "pirate.006", 0x180000, 0x80000, CRC(b0cdf7eb) SHA1(cf6bd20fb40cf0d87eeb6f1502fb73d9760c9140) )
	ROM_LOAD( "pirate.007", 0x280000, 0x80000, CRC(6c4a9510) SHA1(e10bf8475ff7c73ba90b904b9214b285a5b2669f) )
	ROM_LOAD( "pirate.008", 0x380000, 0x80000, CRC(cc2edac2) SHA1(24bacd9e092a83945a8def3a254ec66758d71ff5) )
ROM_END

ROM_START( pirate_4 ) // 070412
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "pr_070412.rom", 0x00000, 0x40000, CRC(31909fb2) SHA1(f9ebad0de53645f7ed854147d8227a3b5b9224f4) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "pirate.001", 0x000000, 0x80000, CRC(d2199619) SHA1(8c67ef7d0e305ae0783302ba9a1cb56cdcf4bc09) )
	ROM_LOAD( "pirate.002", 0x100000, 0x80000, CRC(ce5c6548) SHA1(ef1cd6ae36cc1abcf010762dc89a255cd817d016) )
	ROM_LOAD( "pirate.003", 0x200000, 0x80000, CRC(d6a8338d) SHA1(6a0e41309dc909decf8bd49cf13cbeca95f0314a) )
	ROM_LOAD( "pirate.004", 0x300000, 0x80000, CRC(590b8cf6) SHA1(b2778f6e1b7bcf7f33ced43f999eff983e5a6af4) )
	ROM_LOAD( "pirate.005", 0x080000, 0x80000, CRC(bf9f1267) SHA1(b0947bd7d31301ffbe80cbaf1e96c3476f6f9ca3) )
	ROM_LOAD( "pirate.006", 0x180000, 0x80000, CRC(b0cdf7eb) SHA1(cf6bd20fb40cf0d87eeb6f1502fb73d9760c9140) )
	ROM_LOAD( "pirate.007", 0x280000, 0x80000, CRC(6c4a9510) SHA1(e10bf8475ff7c73ba90b904b9214b285a5b2669f) )
	ROM_LOAD( "pirate.008", 0x380000, 0x80000, CRC(cc2edac2) SHA1(24bacd9e092a83945a8def3a254ec66758d71ff5) )
ROM_END


/*********************************************************
   Pirate 2
**********************************************************/

ROM_START( pirate2 ) // 061005
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "pr2_m_061005.rom", 0x00000, 0x40000, CRC(4ad0a29a) SHA1(72950da5f201a393a4761a8696cfc210725df23f) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "pirate2.001", 0x000000, 0x80000, CRC(106e7cba) SHA1(289a3ae38b895c83600c920bee0c2dd46e941eac) )
	ROM_LOAD( "pirate2.002", 0x100000, 0x80000, CRC(076a290f) SHA1(2f9bb74e081262e535c8ed9a31589d6a919f5d15) )
	ROM_LOAD( "pirate2.003", 0x200000, 0x80000, CRC(13a91fe7) SHA1(6e127b3827a9271ad19986714747be9367125f62) )
	ROM_LOAD( "pirate2.004", 0x300000, 0x80000, CRC(5ac8c531) SHA1(1da91b9a71a9a8681577342660bfa85e5bbc99bc) )
	ROM_LOAD( "pirate2.005", 0x080000, 0x80000, CRC(98012c74) SHA1(2a5b466353eef3a5cfc9f98eceb7523b00d0204a) )
	ROM_LOAD( "pirate2.006", 0x180000, 0x80000, CRC(366e1465) SHA1(440230d5306c4b424f27839b7fb9c8a5bb922dcc) )
	ROM_LOAD( "pirate2.007", 0x280000, 0x80000, CRC(21fb963e) SHA1(e3f7fb13f326699e34aebcc3ee07016f7cfe6e46) )
	ROM_LOAD( "pirate2.008", 0x380000, 0x80000, CRC(40c59448) SHA1(774af0f376864ec5948904df338bc7493eaed392) )
ROM_END

ROM_START( pirate2_2 ) // 070126
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "pr2_070126.rom", 0x00000, 0x40000, CRC(5abf1580) SHA1(0f5c2ed4f52070dcbeb3adf0d209088e2822696d) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "pirate2.001", 0x000000, 0x80000, CRC(106e7cba) SHA1(289a3ae38b895c83600c920bee0c2dd46e941eac) )
	ROM_LOAD( "pirate2.002", 0x100000, 0x80000, CRC(076a290f) SHA1(2f9bb74e081262e535c8ed9a31589d6a919f5d15) )
	ROM_LOAD( "pirate2.003", 0x200000, 0x80000, CRC(13a91fe7) SHA1(6e127b3827a9271ad19986714747be9367125f62) )
	ROM_LOAD( "pirate2.004", 0x300000, 0x80000, CRC(5ac8c531) SHA1(1da91b9a71a9a8681577342660bfa85e5bbc99bc) )
	ROM_LOAD( "pirate2.005", 0x080000, 0x80000, CRC(98012c74) SHA1(2a5b466353eef3a5cfc9f98eceb7523b00d0204a) )
	ROM_LOAD( "pirate2.006", 0x180000, 0x80000, CRC(366e1465) SHA1(440230d5306c4b424f27839b7fb9c8a5bb922dcc) )
	ROM_LOAD( "pirate2.007", 0x280000, 0x80000, CRC(21fb963e) SHA1(e3f7fb13f326699e34aebcc3ee07016f7cfe6e46) )
	ROM_LOAD( "pirate2.008", 0x380000, 0x80000, CRC(40c59448) SHA1(774af0f376864ec5948904df338bc7493eaed392) )
ROM_END



/*********************************************************
   Keks

    "Russia" sets use different gfx roms 1-4.
        The official list of hashes shows only updated roms.

**********************************************************/

ROM_START( keks ) // 060328
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "ks_m_060328.rom", 0x00000, 0x40000, CRC(bcf77f77) SHA1(26b09994907c41be957a0b7442cfb1807b27d7be) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "keks_m.001", 0x000000, 0x80000, CRC(f4c20f66) SHA1(bed42ef01dfaa9d5d6ebb703e44ce7c11b8a373c) )
	ROM_LOAD( "keks_m.002", 0x100000, 0x80000, CRC(b7ec3fac) SHA1(c3c62690487a6056415c46888bde8254efca836f) )
	ROM_LOAD( "keks_m.003", 0x200000, 0x80000, CRC(5b6e8568) SHA1(003297e9cd080d91fe6751286eabd3a2f37ceb76) )
	ROM_LOAD( "keks_m.004", 0x300000, 0x80000, CRC(9dc32736) SHA1(7b2091ae802431d1c959b859a58e0076d32abef0) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END


ROM_START( keks_2 ) // 060403
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "ks_m_060403.rom", 0x00000, 0x40000, CRC(7abb9392) SHA1(f7a0ba5bcc7566f706e911486fa9cf3e62b86b8b) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "keks_m.001", 0x000000, 0x80000, CRC(f4c20f66) SHA1(bed42ef01dfaa9d5d6ebb703e44ce7c11b8a373c) )
	ROM_LOAD( "keks_m.002", 0x100000, 0x80000, CRC(b7ec3fac) SHA1(c3c62690487a6056415c46888bde8254efca836f) )
	ROM_LOAD( "keks_m.003", 0x200000, 0x80000, CRC(5b6e8568) SHA1(003297e9cd080d91fe6751286eabd3a2f37ceb76) )
	ROM_LOAD( "keks_m.004", 0x300000, 0x80000, CRC(9dc32736) SHA1(7b2091ae802431d1c959b859a58e0076d32abef0) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keks_3 ) // 070119
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "ks_070119.rom", 0x00000, 0x40000, CRC(1cf8fdaa) SHA1(a7f1242d19c5fb5a6fccacc06dd9d27f3352fe24) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "keks.001",   0x000000, 0x80000, CRC(fc399595) SHA1(037afd4a613cd58d4a28627b9e395d48c3fa866a) )
	ROM_LOAD( "keks.002",   0x100000, 0x80000, CRC(474b36e7) SHA1(e1e62acd4a706b2654fc1249850806b612fc1419) )
	ROM_LOAD( "keks.003",   0x200000, 0x80000, CRC(7f885e3d) SHA1(09bb4690e86ed4a29eef75ee4e5753ce40a164dd) )
	ROM_LOAD( "keks.004",   0x300000, 0x80000, CRC(a0fc654b) SHA1(3354bdb7aa372816a766b0d36408543de7d3482f) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END



/*********************************************************
   Gnome
**********************************************************/

ROM_START( gnome ) // 070906
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gn_070906.rom", 0x00000, 0x40000, CRC(8fe48f72) SHA1(cc3c74be120359ecfc42a5e96eff95c2da1e8f4c) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gnome.001", 0x000000, 0x80000, CRC(6ed866d7) SHA1(68d75d24d98e6d533cb26ceac0a680203cb26069) )
	ROM_LOAD( "gnome.002", 0x100000, 0x80000, CRC(f6e5e6f0) SHA1(9751e8df87f14a252595547d24b8dd865ee4f08d) )
	ROM_LOAD( "gnome.003", 0x200000, 0x80000, CRC(f8beb972) SHA1(3afbca8ce7e69d2dadae05f69205a6fd9036cf6a) )
	ROM_LOAD( "gnome.004", 0x300000, 0x80000, CRC(83357c38) SHA1(45cd31c4f02f9d7b1888701c2146d1e7229b6cb5) )
	ROM_LOAD( "gnome.005", 0x080000, 0x80000, CRC(687ad3e3) SHA1(23941a4f40c45029b9a43451f78b04c03c3cd7da) )
	ROM_LOAD( "gnome.006", 0x180000, 0x80000, CRC(7ef2b88a) SHA1(7e7de60fc6791731d7cfd6a50e2bc5af1bf5e4b2) )
	ROM_LOAD( "gnome.007", 0x280000, 0x80000, CRC(71976bdf) SHA1(c44dbfa75a0f12893b3177907fc93b3d5e8ad390) )
	ROM_LOAD( "gnome.008", 0x380000, 0x80000, CRC(c86a1586) SHA1(e622bca8dc618ca8edc1a7daa9c8286383caebef) )
ROM_END

ROM_START( gnome_2 ) // 071115
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gn_071115.rom", 0x00000, 0x40000, CRC(2fb768b8) SHA1(b0aadf057bd8a5b6cdcef5baed190c3d222256ca) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gnome.001", 0x000000, 0x80000, CRC(6ed866d7) SHA1(68d75d24d98e6d533cb26ceac0a680203cb26069) )
	ROM_LOAD( "gnome.002", 0x100000, 0x80000, CRC(f6e5e6f0) SHA1(9751e8df87f14a252595547d24b8dd865ee4f08d) )
	ROM_LOAD( "gnome.003", 0x200000, 0x80000, CRC(f8beb972) SHA1(3afbca8ce7e69d2dadae05f69205a6fd9036cf6a) )
	ROM_LOAD( "gnome.004", 0x300000, 0x80000, CRC(83357c38) SHA1(45cd31c4f02f9d7b1888701c2146d1e7229b6cb5) )
	ROM_LOAD( "gnome.005", 0x080000, 0x80000, CRC(687ad3e3) SHA1(23941a4f40c45029b9a43451f78b04c03c3cd7da) )
	ROM_LOAD( "gnome.006", 0x180000, 0x80000, CRC(7ef2b88a) SHA1(7e7de60fc6791731d7cfd6a50e2bc5af1bf5e4b2) )
	ROM_LOAD( "gnome.007", 0x280000, 0x80000, CRC(71976bdf) SHA1(c44dbfa75a0f12893b3177907fc93b3d5e8ad390) )
	ROM_LOAD( "gnome.008", 0x380000, 0x80000, CRC(c86a1586) SHA1(e622bca8dc618ca8edc1a7daa9c8286383caebef) )
ROM_END

ROM_START( gnome_3 ) // 080303
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "gn_m_080303.rom", 0x00000, 0x40000, CRC(d0aec288) SHA1(5870fd0fe27b049b7945369b23b0dd00e0ccf3e1) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gnome.001", 0x000000, 0x80000, CRC(6ed866d7) SHA1(68d75d24d98e6d533cb26ceac0a680203cb26069) )
	ROM_LOAD( "gnome.002", 0x100000, 0x80000, CRC(f6e5e6f0) SHA1(9751e8df87f14a252595547d24b8dd865ee4f08d) )
	ROM_LOAD( "gnome.003", 0x200000, 0x80000, CRC(f8beb972) SHA1(3afbca8ce7e69d2dadae05f69205a6fd9036cf6a) )
	ROM_LOAD( "gnome.004", 0x300000, 0x80000, CRC(83357c38) SHA1(45cd31c4f02f9d7b1888701c2146d1e7229b6cb5) )
	ROM_LOAD( "gnome.005", 0x080000, 0x80000, CRC(687ad3e3) SHA1(23941a4f40c45029b9a43451f78b04c03c3cd7da) )
	ROM_LOAD( "gnome.006", 0x180000, 0x80000, CRC(7ef2b88a) SHA1(7e7de60fc6791731d7cfd6a50e2bc5af1bf5e4b2) )
	ROM_LOAD( "gnome.007", 0x280000, 0x80000, CRC(71976bdf) SHA1(c44dbfa75a0f12893b3177907fc93b3d5e8ad390) )
	ROM_LOAD( "gnome.008", 0x380000, 0x80000, CRC(c86a1586) SHA1(e622bca8dc618ca8edc1a7daa9c8286383caebef) )
ROM_END



/*********************************************************
   Fruit Cocktail 2

    Roms 4-8 were changed 11/19/2008 (listed below as non "old" roms)

        The Igrosoft web site explains:
         The wrong representation of the number of free games
         on the page 6 of help was corrected

**********************************************************/

ROM_START( fcockt2 ) // 080707
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc2_080707.rom", 0x00000, 0x40000, CRC(3a42f27d) SHA1(7ba91f52b1b0ac4513caebc2989f3b9d6f9dfde4) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail2.001",  0x000000, 0x80000, CRC(d1b9416d) SHA1(4d3cb0a6dbcf02bbd20d5c43df358882b2ad794d) )
	ROM_LOAD( "fruitcocktail2.002",  0x100000, 0x80000, CRC(69236be1) SHA1(9a2e6c8f279714f79a606c0b118b6bf1d8442cda) )
	ROM_LOAD( "fruitcocktail2.003",  0x200000, 0x80000, CRC(29aade8c) SHA1(bef42f8a25c90e3a1cccd13872a10eb8b2b2e276) )
	ROM_LOAD( "fruitcocktail2.004",  0x300000, 0x80000, CRC(4b9646e7) SHA1(26548a018401f4e07383eb145f8f0847677f3272) )
	ROM_LOAD( "fruitcocktail2_old.005", 0x080000, 0x80000, CRC(6b9e6b43) SHA1(c7fb17e91ec62b22da42f110d68b4f37e39de3ce) )
	ROM_LOAD( "fruitcocktail2_old.006", 0x180000, 0x80000, CRC(2c9f712e) SHA1(c3118154eafca74b66b3325a2e07c85f86f3544d) )
	ROM_LOAD( "fruitcocktail2_old.007", 0x280000, 0x80000, CRC(85ba9a86) SHA1(aa9b6170135e9e420509e8f7c1702c9896bc5d8e) )
	ROM_LOAD( "fruitcocktail2_old.008", 0x380000, 0x80000, CRC(a27c49a2) SHA1(7c9ee0e01f76ca3ab6716579f5dde7036050970b) )
ROM_END

ROM_START( fcockt2_2 ) // 080904
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc2_080904.rom", 0x00000, 0x40000, SHA1(d8f1034753274aed874ee552ab78e660ddaba939) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail2.001",  0x000000, 0x80000, CRC(d1b9416d) SHA1(4d3cb0a6dbcf02bbd20d5c43df358882b2ad794d) )
	ROM_LOAD( "fruitcocktail2.002",  0x100000, 0x80000, CRC(69236be1) SHA1(9a2e6c8f279714f79a606c0b118b6bf1d8442cda) )
	ROM_LOAD( "fruitcocktail2.003",  0x200000, 0x80000, CRC(29aade8c) SHA1(bef42f8a25c90e3a1cccd13872a10eb8b2b2e276) )
	ROM_LOAD( "fruitcocktail2.004",  0x300000, 0x80000, CRC(4b9646e7) SHA1(26548a018401f4e07383eb145f8f0847677f3272) )
	ROM_LOAD( "fruitcocktail2_old.005", 0x080000, 0x80000, CRC(6b9e6b43) SHA1(c7fb17e91ec62b22da42f110d68b4f37e39de3ce) )
	ROM_LOAD( "fruitcocktail2_old.006", 0x180000, 0x80000, CRC(2c9f712e) SHA1(c3118154eafca74b66b3325a2e07c85f86f3544d) )
	ROM_LOAD( "fruitcocktail2_old.007", 0x280000, 0x80000, CRC(85ba9a86) SHA1(aa9b6170135e9e420509e8f7c1702c9896bc5d8e) )
	ROM_LOAD( "fruitcocktail2_old.008", 0x380000, 0x80000, CRC(a27c49a2) SHA1(7c9ee0e01f76ca3ab6716579f5dde7036050970b) )
ROM_END

ROM_START( fcockt2_3 ) // 080909
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc2_m_080909.rom", 0x00000, 0x40000, CRC(6de0353c) SHA1(7a827a172cdd593f8b37a7737304a5a2e145d52d) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail2.001",  0x000000, 0x80000, CRC(d1b9416d) SHA1(4d3cb0a6dbcf02bbd20d5c43df358882b2ad794d) )
	ROM_LOAD( "fruitcocktail2.002",  0x100000, 0x80000, CRC(69236be1) SHA1(9a2e6c8f279714f79a606c0b118b6bf1d8442cda) )
	ROM_LOAD( "fruitcocktail2.003",  0x200000, 0x80000, CRC(29aade8c) SHA1(bef42f8a25c90e3a1cccd13872a10eb8b2b2e276) )
	ROM_LOAD( "fruitcocktail2.004",  0x300000, 0x80000, CRC(4b9646e7) SHA1(26548a018401f4e07383eb145f8f0847677f3272) )
	ROM_LOAD( "fruitcocktail2_old.005", 0x080000, 0x80000, CRC(6b9e6b43) SHA1(c7fb17e91ec62b22da42f110d68b4f37e39de3ce) )
	ROM_LOAD( "fruitcocktail2_old.006", 0x180000, 0x80000, CRC(2c9f712e) SHA1(c3118154eafca74b66b3325a2e07c85f86f3544d) )
	ROM_LOAD( "fruitcocktail2_old.007", 0x280000, 0x80000, CRC(85ba9a86) SHA1(aa9b6170135e9e420509e8f7c1702c9896bc5d8e) )
	ROM_LOAD( "fruitcocktail2_old.008", 0x380000, 0x80000, CRC(a27c49a2) SHA1(7c9ee0e01f76ca3ab6716579f5dde7036050970b) )
ROM_END

ROM_START( fcockt2_4 ) // 081105
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc2_m_081105.rom", 0x00000, 0x40000,  CRC(2cc5313d) SHA1(b5e55acbb5936f49130758947ae22d1847800333) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail2.001", 0x000000, 0x80000, CRC(d1b9416d) SHA1(4d3cb0a6dbcf02bbd20d5c43df358882b2ad794d) )
	ROM_LOAD( "fruitcocktail2.002", 0x100000, 0x80000, CRC(69236be1) SHA1(9a2e6c8f279714f79a606c0b118b6bf1d8442cda) )
	ROM_LOAD( "fruitcocktail2.003", 0x200000, 0x80000, CRC(29aade8c) SHA1(bef42f8a25c90e3a1cccd13872a10eb8b2b2e276) )
	ROM_LOAD( "fruitcocktail2.004", 0x300000, 0x80000, CRC(4b9646e7) SHA1(26548a018401f4e07383eb145f8f0847677f3272) )
	ROM_LOAD( "fruitcocktail2.005", 0x080000, 0x80000, CRC(3fc13d72) SHA1(ba0727138ef03d576d190cbce04b9eb0bba88a9a) )
	ROM_LOAD( "fruitcocktail2.006", 0x180000, 0x80000, CRC(8fc75fd7) SHA1(3b7cd8a3e04ca9d4494b37c801e21d1293f094e8) )
	ROM_LOAD( "fruitcocktail2.007", 0x280000, 0x80000, CRC(d37fcc0f) SHA1(57c2ea5dc747f16e2233305f2c73cb4b632aae2c) )
	ROM_LOAD( "fruitcocktail2.008", 0x380000, 0x80000, CRC(e3a9442c) SHA1(cbaba182e858b0f158756118e5da873e3ddfc0b9) )
ROM_END

ROM_START( fcockt2_5 ) // 081106
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "fc2_081106.rom", 0x00000, 0x40000, CRC(d23347b3) SHA1(2d7d00af182c61fa166a8f3fd6fd830cf5eb78c6) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail2.001", 0x000000, 0x80000, CRC(d1b9416d) SHA1(4d3cb0a6dbcf02bbd20d5c43df358882b2ad794d) )
	ROM_LOAD( "fruitcocktail2.002", 0x100000, 0x80000, CRC(69236be1) SHA1(9a2e6c8f279714f79a606c0b118b6bf1d8442cda) )
	ROM_LOAD( "fruitcocktail2.003", 0x200000, 0x80000, CRC(29aade8c) SHA1(bef42f8a25c90e3a1cccd13872a10eb8b2b2e276) )
	ROM_LOAD( "fruitcocktail2.004", 0x300000, 0x80000, CRC(4b9646e7) SHA1(26548a018401f4e07383eb145f8f0847677f3272) )
	ROM_LOAD( "fruitcocktail2.005", 0x080000, 0x80000, CRC(3fc13d72) SHA1(ba0727138ef03d576d190cbce04b9eb0bba88a9a) )
	ROM_LOAD( "fruitcocktail2.006", 0x180000, 0x80000, CRC(8fc75fd7) SHA1(3b7cd8a3e04ca9d4494b37c801e21d1293f094e8) )
	ROM_LOAD( "fruitcocktail2.007", 0x280000, 0x80000, CRC(d37fcc0f) SHA1(57c2ea5dc747f16e2233305f2c73cb4b632aae2c) )
	ROM_LOAD( "fruitcocktail2.008", 0x380000, 0x80000, CRC(e3a9442c) SHA1(cbaba182e858b0f158756118e5da873e3ddfc0b9) )
ROM_END

/*********************************************************
   Crazy Monkey 2
**********************************************************/

/* no supported sets */


/*

Note:

   Only the first set of a given revision is listed in Igrosoft's official hashes list.

   The sets which differs from the originals by 5-6 bytes are bootlegs that simply change
     the banking address. Usually to convert a Crazy Monkey PCB which use the address "F9".
     Software exists to automatic modify any program rom's banking address for any PCB.
     This has resulted in dozens of different bootleg versions floating around the net.

   Some sets simply changed the version text to show a newer version.  This was likely done
     so the opperator appears to be running a version that meets the standards of the changed
     gambling law (see below).

   Sets marked as "backdoor" are identical to originals, but have added code.  This code is
     activated by a secret sequence of actions that leads to a guaranteed win, or dramatically
     increases odds for winning. These backdoor version were commonly used by administrators
     or PCB sellers to steal money from the slots owners.
   Software does exist to automatically add the backdoor code and allows for custom key
     sequences.  As a result of this, there is also software to dectect the backdoor code
     in program roms.
   How to activate the backdoor (each combination is: No of lines, Bet value then Start):
     Enter the first five combinations - you'll become very lucky from this moment.
     Note: backdoor state stored in NVRAM.
   How to deactivate the backdoor:
     Enter the sixth combination twice.
crzmon_7a 1,1 1,3 1,5 1,7  3,3  3,4
crzmon_7b 1,5 5,5 1,7 3,2  3,3  3,4
crzmon_8a 1,1 1,3 1,5 1,7  3,3  3,4
fcockt_6b 1,1 1,3 1,5 1,7  3,3  3,4
fcockt_7a 1,1 1,3 1,5 1,7  3,3  3,4
lhaunt_4a 1,1 1,3 1,5 1,7  3,3  3,4
lhaunt_5a 1,1 1,3 1,5 1,7  3,3  3,4
lhaunt_6a 1,5 9,1 5,1 1,5  3,3  3,4
garage_4a 1,1 1,3 1,5 1,7  3,3  3,4
garage_5a 1,1 1,3 1,5 1,7  3,3  3,4
rclimb_3a 1,5 9,1 5,1 1,5  3,3  3,4
sweetla   1,5 9,1 5,3 1,5  3,3  3,4
resdnt_2a 1,5 9,1 5,1 1,5  3,3  3,4
islanda   1,1 1,3 1,5 1,7  3,3  3,4
keksb     1,1 1,3 1,5 1,7  3,3  3,4

Most games had a revision in early 2007 to meet the standards of the "Government gambling control"
   law of The Russian Federation No 244-03 of Dec 29, 2006

   From Igrosoft's web site about version types (IE: some version have "M" in them):

   * Two software versions are shown, one of them corresponds to Russian legislation,
     the other one (with the letter m) is for the countries without such restrictions.

*/


#define mfish_parent mfish_13
  GAME( 2002, goldfish,    mfish_parent,    multfish, multfish,  0,        ROT0, "Igrosoft", "Gold Fish (020903, prototype)", GAME_SUPPORTS_SAVE ) /* World */
//GAME( 2002, mfish,       mfish_parent,    multfish, multfish,  0,        ROT0, "Igrosoft", "Multi Fish (021120)", GAME_SUPPORTS_SAVE ) /* World */
//GAME( 2002, mfish_2,     mfish_parent,    multfish, multfish,  0,        ROT0, "Igrosoft", "Multi Fish (021121)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2002, mfish_3,     mfish_parent,    multfish, multfish,  0,        ROT0, "Igrosoft", "Multi Fish (021124)", GAME_SUPPORTS_SAVE ) /* World */
//GAME( 2002, mfish_4,     mfish_parent,    multfish, multfish,  0,        ROT0, "Igrosoft", "Multi Fish (021219)", GAME_SUPPORTS_SAVE ) /* World */
//GAME( 2002, mfish_5,     mfish_parent,    multfish, multfish,  0,        ROT0, "Igrosoft", "Multi Fish (021227)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2002, mfish_6,     mfish_parent,    multfish, multfish,  0,        ROT0, "Igrosoft", "Multi Fish (030124)", GAME_SUPPORTS_SAVE ) /* World */
//GAME( 2002, mfish_7,     mfish_parent,    multfish, multfish,  0,        ROT0, "Igrosoft", "Multi Fish (030511)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2002, mfish_8,     mfish_parent,    multfish, multfish,  0,        ROT0, "Igrosoft", "Multi Fish (030522)", GAME_SUPPORTS_SAVE ) /* World */
//GAME( 2002, mfish_9,     mfish_parent,    multfish, multfish,  0,        ROT0, "Igrosoft", "Multi Fish (031026)", GAME_SUPPORTS_SAVE ) /* World */
//GAME( 2002, mfish_10,    mfish_parent,    multfish, multfish,  0,        ROT0, "Igrosoft", "Multi Fish (031117)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2002, mfish_11,    mfish_parent,    multfish, multfish,  0,        ROT0, "Igrosoft", "Multi Fish (031124)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2002, mfish_12,    mfish_parent,    multfish, multfish,  0,        ROT0, "Igrosoft", "Multi Fish (040308)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2002, mfish_13,    0,               multfish, multfish,  0,        ROT0, "Igrosoft", "Multi Fish (040316)", GAME_SUPPORTS_SAVE ) /* World */

#define crzmon_parent crzmon_8
//GAME( 2003, crzmon,      crzmon_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Crazy Monkey (030217 World)", GAME_SUPPORTS_SAVE ) /* World */
//GAME( 2003, crzmon_2,    crzmon_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Crazy Monkey (030225 World)", GAME_SUPPORTS_SAVE ) /* World */
//GAME( 2003, crzmon_3,    crzmon_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Crazy Monkey (030227 World)", GAME_SUPPORTS_SAVE ) /* World */
//GAME( 2003, crzmon_4,    crzmon_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Crazy Monkey (030404 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2003, crzmon_5,    crzmon_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Crazy Monkey (030421 World)", GAME_SUPPORTS_SAVE ) /* World */
//GAME( 2003, crzmon_6,    crzmon_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Crazy Monkey (031016 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2003, crzmon_7,    crzmon_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Crazy Monkey (031110 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2003, crzmon_8,    0,               multfish, multfish,  0,        ROT0, "Igrosoft", "Crazy Monkey (050120 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2003, crzmon_9,    crzmon_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Crazy Monkey (070315 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */
//GAME( 2003, crzmon_10,   crzmon_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Crazy Monkey (081027 Lottery)", GAME_SUPPORTS_SAVE ) /* Lottery */
//GAME( 2003, crzmon_11,   crzmon_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Crazy Monkey (081113 Lottery)", GAME_SUPPORTS_SAVE ) /* Lottery */


#define fcockt_parent fcockt_8
//GAME( 2003, fcockt,      fcockt_parent    multfish, multfish,  0,        ROT0, "Igrosoft", "Fruit Cocktail (030505 World)", GAME_SUPPORTS_SAVE ) /* World */
//GAME( 2003, fcockt_2,    fcockt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Fruit Cocktail (030512 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2003, fcockt_3,    fcockt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Fruit Cocktail (030623 World)", GAME_SUPPORTS_SAVE ) /* World */
//GAME( 2003, fcockt_4,    fcockt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Fruit Cocktail (031028 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2003, fcockt_5,    fcockt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Fruit Cocktail (031111 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2003, fcockt_6,    fcockt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Fruit Cocktail (040216 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2003, fcockt_7,    fcockt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Fruit Cocktail (050118 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2003, fcockt_8,    0,               multfish, multfish,  0,        ROT0, "Igrosoft", "Fruit Cocktail (060111 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2003, fcockt_9,    fcockt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Fruit Cocktail (070305 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */
  GAME( 2003, fcockt_10,   fcockt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Fruit Cocktail (070517 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */
  GAME( 2003, fcockt_11,   fcockt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Fruit Cocktail (070822 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */
  GAME( 2003, fcockt_12,   fcockt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Fruit Cocktail (070911 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */
//GAME( 2003, fcockt_13,   fcockt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Fruit Cocktail (081124 Lottery)", GAME_SUPPORTS_SAVE ) /* Lottery */

#define lhaunt_parent lhaunt_6
//GAME( 2003, lhaunt,      lhaunt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Lucky Haunter (030707 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2003, lhaunt_2,    lhaunt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Lucky Haunter (030804 World)", GAME_SUPPORTS_SAVE ) /* World */
//GAME( 2003, lhaunt_3,    lhaunt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Lucky Haunter (031027 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2003, lhaunt_4,    lhaunt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Lucky Haunter (031111 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2003, lhaunt_5,    lhaunt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Lucky Haunter (040216 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2003, lhaunt_6,    0,               multfish, multfish,  0,        ROT0, "Igrosoft", "Lucky Haunter (040825 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2003, lhaunt_7,    lhaunt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Lucky Haunter (070402 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */
  GAME( 2003, lhaunt_8,    lhaunt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Lucky Haunter (070604 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */
//GAME( 2003, lhaunt_9,    lhaunt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Lucky Haunter (081208 Lottery)", GAME_SUPPORTS_SAVE ) /* Lottery */


#define rollfr_parent rollfr_4
//GAME( 2003, rollfr,      rollfr_parent,   rollfr,   rollfr,    0,        ROT0, "Igrosoft", "Roll Fruit (030821)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2003, rollfr_2,    rollfr_parent,   rollfr,   rollfr,    0,        ROT0, "Igrosoft", "Roll Fruit (040318)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2003, rollfr_3,    rollfr_parent,   rollfr,   rollfr,    0,        ROT0, "Igrosoft", "Roll Fruit (080327)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2003, rollfr_4,    0,               rollfr,   rollfr,    0,        ROT0, "Igrosoft", "Roll Fruit (080331)", GAME_SUPPORTS_SAVE ) /* World */

#define garage_parent garage_5
//GAME( 2004, garage,      garage_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Garage (040122 World)", GAME_SUPPORTS_SAVE ) /* World */
//GAME( 2004, garage_2,    garage_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Garage (040123 World)", GAME_SUPPORTS_SAVE ) /* World */
//GAME( 2004, garage_3,    garage_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Garage (040216 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2004, garage_4,    garage_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Garage (040219 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2004, garage_5,    0,               multfish, multfish,  0,        ROT0, "Igrosoft", "Garage (050311 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2004, garage_6,    garage_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Garage (070213 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */
  GAME( 2004, garage_7,    garage_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Garage (070329 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */
//GAME( 2004, garage_8,    garage_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Garage (081229 Lottery)", GAME_SUPPORTS_SAVE ) /* Lottery */


#define rclimb_parent rclimb_3
  GAME( 2004, rclimb,      rclimb_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Rock Climber (040815 World)", GAME_SUPPORTS_SAVE ) /* World */
//GAME( 2004, rclimb_2,    rclimb_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Rock Climber (040823 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2004, rclimb_3,    0,               multfish, multfish,  0,        ROT0, "Igrosoft", "Rock Climber (040827 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2004, rclimb_4,    rclimb_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Rock Climber (070322 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */
  GAME( 2004, rclimb_5,    rclimb_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Rock Climber (070621 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */


#define sweetl_parent sweetl
  GAME( 2004, sweetl,      0,               multfish, multfish,  0,        ROT0, "Igrosoft", "Sweet Life (041220 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2004, sweetl_2,    sweetl_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Sweet Life (070412 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */

#define resdnt_parent resdnt_2
  GAME( 2004, resdnt,      resdnt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Resident (040415 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2004, resdnt_2,    0,               multfish, multfish,  0,        ROT0, "Igrosoft", "Resident (040513 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2004, resdnt_3,    resdnt_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Resident (070222 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */


#define island_parent island
  GAME( 2005, island,      0,               multfish, multfish,  0,        ROT0, "Igrosoft", "Island (050713 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2005, island_2,    island_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Island (070409 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */

#define pirate_parent pirate_3
//GAME( 2005, pirate,      pirate_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Pirate (051229 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2005, pirate_2,    pirate_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Pirate (060210 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2005, pirate_3,    0,               multfish, multfish,  0,        ROT0, "Igrosoft", "Pirate (060803 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2005, pirate_4,    pirate_parent,   multfish, multfish,  0,        ROT0, "Igrosoft", "Pirate (070412 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */

#define island2_parent island2
  GAME( 2006, island2,     0,               multfish, multfish,  0,        ROT0, "Igrosoft", "Island 2 (060529 World)", GAME_SUPPORTS_SAVE ) /* World */
//GAME( 2006, island2_2,   island2_parent,  multfish, multfish,  0,        ROT0, "Igrosoft", "Island 2 (061214 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2006, island2_3,   island2_parent,  multfish, multfish,  0,        ROT0, "Igrosoft", "Island 2 (061218 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2006, island2_4,   island2_parent,  multfish, multfish,  0,        ROT0, "Igrosoft", "Island 2 (070205 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */


#define pirate2_parent pirate2
  GAME( 2006, pirate2,     0,               multfish, multfish,  0,        ROT0, "Igrosoft", "Pirate 2 (061005 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2006, pirate2_2,   pirate2_parent,  multfish, multfish,  0,        ROT0, "Igrosoft", "Pirate 2 (070126 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */


#define keks_parent keks_2
  GAME( 2006, keks,        keks_parent,     multfish, multfish,  0,        ROT0, "Igrosoft", "Keks (060328 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2006, keks_2,      0,               multfish, multfish,  0,        ROT0, "Igrosoft", "Keks (060403 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2006, keks_3,      keks_parent,     multfish, multfish,  0,        ROT0, "Igrosoft", "Keks (070119 Russia)", GAME_SUPPORTS_SAVE )  /* Russia */


#define gnome_parent gnome_3
  GAME( 2007, gnome,       gnome_parent,    multfish, multfish,  0,        ROT0, "Igrosoft", "Gnome (070906 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */
  GAME( 2007, gnome_2,     gnome_parent,    multfish, multfish,  0,        ROT0, "Igrosoft", "Gnome (071115 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */
  GAME( 2007, gnome_3,     0,               multfish, multfish,  0,        ROT0, "Igrosoft", "Gnome (080303 World)", GAME_SUPPORTS_SAVE ) /* World */


#define sweetl2_parent sweetl2
  GAME( 2007, sweetl2,     0,               multfish, multfish,  0,        ROT0, "Igrosoft", "Sweet Life 2 (071217 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */
  GAME( 2007, sweetl2_2,   sweetl2_parent,  multfish, multfish,  0,        ROT0, "Igrosoft", "Sweet Life 2 (080320 World)", GAME_SUPPORTS_SAVE ) /* World */

#define fcockt2_parent fcockt2
  GAME( 2008, fcockt2,     0,               multfish, multfish,  0,        ROT0, "Igrosoft", "Fruit Cocktail 2 (080707 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */
//GAME( 2008, fcockt2_2,   fcockt2_parent,  multfish, multfish,  0,        ROT0, "Igrosoft", "Fruit Cocktail 2 (080904 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */
  GAME( 2008, fcockt2_3,   fcockt2_parent,  multfish, multfish,  0,        ROT0, "Igrosoft", "Fruit Cocktail 2 (080909 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2008, fcockt2_4,   fcockt2_parent,  multfish, multfish,  0,        ROT0, "Igrosoft", "Fruit Cocktail 2 (081105 World)", GAME_SUPPORTS_SAVE ) /* World */
  GAME( 2008, fcockt2_5,   fcockt2_parent,  multfish, multfish,  0,        ROT0, "Igrosoft", "Fruit Cocktail 2 (081106 Russia)", GAME_SUPPORTS_SAVE ) /* Russia */



/*************************************************************************************************************************************************************/
/*************************************************************************************************************************************************************/
/*************************************************************************************************************************************************************/
/*************************************************************************************************************************************************************/
/**************************************************************************************************************************************************************

  Sets below are bootleg / hacked sets, descriptions of what has been modified are provided where known
 -- use of these sets may present a risk to the operator, you should always ensure you're using original Igrosoft program roms on your PCB!

**************************************************************************************************************************************************************/
/*************************************************************************************************************************************************************/
/*************************************************************************************************************************************************************/
/*************************************************************************************************************************************************************/
/*************************************************************************************************************************************************************/

ROM_START( mfish_3a ) // 021124 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "bootleg_mf021124a.rom", 0x00000, 0x40000, CRC(31344b4e) SHA1(33c7f30b55d22c087a02e840456d475177df8bf1) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "multi_fish.001", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "multi_fish.002", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "multi_fish.003", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "multi_fish.004", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "multi_fish.005", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "multi_fish.006", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "multi_fish.007", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "multi_fish.008", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END

ROM_START( mfish_12a ) // 040308 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "bootleg_mf040308a.rom", 0x00000, 0x40000, CRC(44537648) SHA1(7bce6085778ff0b21c052ae91703de3b78b8eed0) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "multi_fish.001", 0x000000, 0x80000, CRC(2f2a7367) SHA1(ce7ee9ca4f374ec61edc3b89d4752f0edb64a910) )
	ROM_LOAD( "multi_fish.002", 0x100000, 0x80000, CRC(606acd73) SHA1(ce5f7b1366dbb16d57fe4b7f395f08725e3cf756) )
	ROM_LOAD( "multi_fish.003", 0x200000, 0x80000, CRC(33759c2a) SHA1(6afcee2e00a27542fc9751702abcc84cd7d3a2a8) )
	ROM_LOAD( "multi_fish.004", 0x300000, 0x80000, CRC(d0053546) SHA1(01c69be0c594947d57648f491904a3b6938a5570) )
	ROM_LOAD( "multi_fish.005", 0x080000, 0x80000, CRC(6f632872) SHA1(949661cb234855a9c86403e9893c5d9f465ddd79) )
	ROM_LOAD( "multi_fish.006", 0x180000, 0x80000, CRC(023c1193) SHA1(98cf2732f9542b0bb3bee324611f6d3143ef1dc4) )
	ROM_LOAD( "multi_fish.007", 0x280000, 0x80000, CRC(9afdc2d3) SHA1(b112fd2005354c9f97d77030bdb6f99d7b5c8050) )
	ROM_LOAD( "multi_fish.008", 0x380000, 0x80000, CRC(29f1a326) SHA1(5e268411cab888c0727aaf8ae7d0b435d2efd189) )
ROM_END



ROM_START( crzmon_7a ) // 031110 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_m_031110a.rom", 0x00000, 0x40000, CRC(80666246) SHA1(e15a210b11ba769ca4fd637c962932417555dc0e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_m.001", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "crazymonkey_m.002", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "crazymonkey_m.003", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "crazymonkey_m.004", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_7b ) // 031110 backdoor 1,5 5,5 1,7 3,2  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_m_031110b.rom", 0x00000, 0x40000, CRC(bb6f4f85) SHA1(a2f44632f857392eb422412b55a19decae4c8620) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_m.001", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "crazymonkey_m.002", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "crazymonkey_m.003", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "crazymonkey_m.004", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_8a ) // 050120 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_m_050120a.rom", 0x00000, 0x40000, CRC(e20a6997) SHA1(50e0f0f354dd6db2be64d42e36b4043915c4276b) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_m.001", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "crazymonkey_m.002", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "crazymonkey_m.003", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "crazymonkey_m.004", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_8b ) // 050120 changed version text to 070315
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_m_050120b.rom", 0x00000, 0x40000, CRC(9350d184) SHA1(ccb79bb6c5e9025d64fe07c02334c43d75ee3334) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey_m.001", 0x000000, 0x80000, CRC(683f2be3) SHA1(6fdba4ec07752bf049787a11638895352e9d5f10) )
	ROM_LOAD( "crazymonkey_m.002", 0x100000, 0x80000, CRC(e21ce6a4) SHA1(942ffe323ddbcaaad887cb5bc9f356550926083b) )
	ROM_LOAD( "crazymonkey_m.003", 0x200000, 0x80000, CRC(c3d0e3d5) SHA1(5b0cb436c6b0bac1213c1df56702fa7f16856106) )
	ROM_LOAD( "crazymonkey_m.004", 0x300000, 0x80000, CRC(f79df52c) SHA1(b99fa9f61849b62668bf9edff1c80212a9108b15) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_8c ) // 050120 custom alteras, modified graphics, changed version text to "VIDEO GAME-1 CM01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_m_050120c.rom", 0x00000, 0x40000, CRC(231008a7) SHA1(81f77644ba971946cfdf40e6f886652550b10bae) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(6595a293) SHA1(87b6adc837ca7540dd1ae10b544bd753abc95c38) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(5a0ce771) SHA1(15c7364eba1316128508acb5407d68dc06b3f506) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(12b85ca4) SHA1(22228b0283487b0c01412944030d8e83cd20529d) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(8eabc9d8) SHA1(4ade17740f1c584c417c2a404220249168f350ac) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(032fb030) SHA1(392941cd0538bf776cae8422056e8f67cbf4dcb8) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(4f8a61aa) SHA1(cb266f31ad573f627c8b8d54af88fa9e5a3676ea) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(a055fea1) SHA1(69498f7410acd3d66b391f9c8a1d541d3400674c) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(aa4b0eb3) SHA1(39a98e89a8137c9c986932398b748e48d7f21d9d) )
ROM_END

ROM_START( crzmon_8d ) // 050120 modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_m_050120d.rom", 0x00000, 0x40000, CRC(ad37f261) SHA1(0e03bf6134b59340cb43b7d3bdd0e746ebfb112a) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1e", 0x000000, 0x80000, CRC(0a087fce) SHA1(32bb353a45ea39e16fafb78bb10dbd3d330365a6) )
	ROM_LOAD( "bootleg_2e", 0x100000, 0x80000, CRC(210c9cf0) SHA1(ede3cdb9be7dddd4619cd91e1eb51656ba49e250) )
	ROM_LOAD( "bootleg_3e", 0x200000, 0x80000, CRC(08f5933a) SHA1(16a232c634778e4a5f39c6890606ad402b2a52ca) )
	ROM_LOAD( "bootleg_4e", 0x300000, 0x80000, CRC(e12abf21) SHA1(cc015cdda4e1693eb1452c9d11996dbd8b906610) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_8e ) // 050120 modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-CM2"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_m_050120e.rom", 0x00000, 0x40000, CRC(b4b79f13) SHA1(7efb67eb66e18885760422a239f2f56052aa7aa2) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1f", 0x000000, 0x80000, CRC(a2825411) SHA1(ad2d8bdd7b8031e69d3357911b4be7913cdde668) )
	ROM_LOAD( "bootleg_2f", 0x100000, 0x80000, CRC(2a446252) SHA1(4d4daa22421a9ac5a577afdbd257e38b034270c9) )
	ROM_LOAD( "bootleg_3f", 0x200000, 0x80000, CRC(12fa2d2c) SHA1(b3e91cf796e177204c1fde64b83ab389c8b71724) )
	ROM_LOAD( "bootleg_4f", 0x300000, 0x80000, CRC(12e0981f) SHA1(e69e8c635573aedfa177aa6fb2b3188c6fa265b5) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_8f ) // 050120 custom_alteras, modified graphics, many texts changed, changed version text to "LOTOS CM01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_m_050120f.rom", 0x00000, 0x40000, CRC(1ea2e07e) SHA1(964b7381573918880b1b537ab5f8006702d2eedb) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1g", 0x000000, 0x80000, CRC(88ef6dfb) SHA1(a8e323b363f30f8b3ec1f57dbffbedd0a868dc91) )
	ROM_LOAD( "bootleg_2g", 0x100000, 0x80000, CRC(5ae3c4ab) SHA1(7e2d5c4c7900222920f1a6babd7b655aa4c3e849) )
	ROM_LOAD( "bootleg_3g", 0x200000, 0x80000, CRC(70c12376) SHA1(fbb92baed3102975125aa36d377b2f07d66e3a4a) )
	ROM_LOAD( "bootleg_4g", 0x300000, 0x80000, CRC(65da15c4) SHA1(e853cf8aad61a26bdfa864fa16fcf82fd99522b3) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(032fb030) SHA1(392941cd0538bf776cae8422056e8f67cbf4dcb8) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(4f8a61aa) SHA1(cb266f31ad573f627c8b8d54af88fa9e5a3676ea) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(a055fea1) SHA1(69498f7410acd3d66b391f9c8a1d541d3400674c) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(aa4b0eb3) SHA1(39a98e89a8137c9c986932398b748e48d7f21d9d) )
ROM_END


ROM_START( crzmon_9a ) // 070315 custom alteras, modified graphics, changed version text to "VIDEO GAME-1 O01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_070315a.rom", 0x00000, 0x40000, CRC(1c4ea4ac) SHA1(ffbc5345170afdca9154a8a53c2887439b04b489) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1c", 0x000000, 0x80000, CRC(449253a7) SHA1(766e3b9b9585ee8c3b952033e5fb78c256075925) )
	ROM_LOAD( "bootleg_2c", 0x100000, 0x80000, CRC(9475f993) SHA1(a69326bcf5b98510ab996dee8aecee62ef3f2319) )
	ROM_LOAD( "bootleg_3c", 0x200000, 0x80000, CRC(45376e79) SHA1(097b779b5aeea952e4b900123909b441e2f32003) )
	ROM_LOAD( "bootleg_4c", 0x300000, 0x80000, CRC(4d2dbb3d) SHA1(da1d9b1901d1dc63643529f1e60d67fc3635b966) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(032fb030) SHA1(392941cd0538bf776cae8422056e8f67cbf4dcb8) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(4f8a61aa) SHA1(cb266f31ad573f627c8b8d54af88fa9e5a3676ea) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(a055fea1) SHA1(69498f7410acd3d66b391f9c8a1d541d3400674c) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(aa4b0eb3) SHA1(39a98e89a8137c9c986932398b748e48d7f21d9d) )
ROM_END

ROM_START( crzmon_9b ) // 070315 modified graphics, changed version text to "VIDEO GAME-1 O01" (crzmon_9a, decoded gfx)
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_070315a.rom", 0x00000, 0x40000, CRC(1c4ea4ac) SHA1(ffbc5345170afdca9154a8a53c2887439b04b489) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1d", 0x000000, 0x80000, CRC(19e03366) SHA1(0050d54f816a10f03b3e76b8472038d39ebe6fc7) )
	ROM_LOAD( "bootleg_2d", 0x100000, 0x80000, CRC(57a08a6a) SHA1(f8691f35efe61ad2e0e22f60a626dec38f6da49c) )
	ROM_LOAD( "bootleg_3d", 0x200000, 0x80000, CRC(2c3bd0c9) SHA1(181f8c84478ad06af941710142bd3c3ab808455b) )
	ROM_LOAD( "bootleg_4d", 0x300000, 0x80000, CRC(d054838f) SHA1(21c2466d45706a80563fc5873d64a7b1abe8da94) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END

ROM_START( crzmon_9c ) // 070315 payout percentage 70%
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_cm_070315b.rom", 0x00000, 0x40000, CRC(d49d3d36) SHA1(00c1ddac53ada6905fe1a8c450158c87c87f33d6) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "crazymonkey.001", 0x000000, 0x80000, CRC(665ae6a1) SHA1(2ef2d657918d66d303d45d2d82430d13108f3fad) )
	ROM_LOAD( "crazymonkey.002", 0x100000, 0x80000, CRC(1a8e235a) SHA1(6d562cc5250283fc0c8ca7e103231a2e5bab4c69) )
	ROM_LOAD( "crazymonkey.003", 0x200000, 0x80000, CRC(415133eb) SHA1(227f7c8858fd5b928fdde691017104d3bd69910a) )
	ROM_LOAD( "crazymonkey.004", 0x300000, 0x80000, CRC(ec45fe14) SHA1(4a0fc87e2f19ea05c9a5746bb4ca7cafe5592d33) )
	ROM_LOAD( "crazymonkey_m.005", 0x080000, 0x80000, CRC(9d4d2a94) SHA1(c714e110de628b343dfc7fff23befaa1276056a9) )
	ROM_LOAD( "crazymonkey_m.006", 0x180000, 0x80000, CRC(a15f0fee) SHA1(3f06d5a1a41e1335bcc7586a5ea95b9b734155c0) )
	ROM_LOAD( "crazymonkey_m.007", 0x280000, 0x80000, CRC(715a2528) SHA1(6c4c72592568ecbaa9518fb7271d2714dd22dbbb) )
	ROM_LOAD( "crazymonkey_m.008", 0x380000, 0x80000, CRC(6fdb6fd5) SHA1(f40916112365de258956ec033aff79aae1f58690) )
ROM_END



ROM_START( fcockt_6a ) // 040216 bank F8
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_040216a.rom", 0x00000, 0x40000, CRC(58e7a0c6) SHA1(8022f92af05e9ff6999ff936bad6048d6c264086) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_6b ) // 040216 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_040216b.rom", 0x00000, 0x40000, CRC(0f6bcf03) SHA1(6c8765f836f1d899aec3be9c842d5064fd70a435) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_6c ) // 040216 modified graphics, some code changes, description says "for Lat-02 terminals", older set
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_040216c.rom", 0x00000, 0x40000, CRC(890940db) SHA1(5180d711fb3222e3d86c691cb8a23eed6ea0030e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1c", 0x000000, 0x80000, CRC(3ac27b83) SHA1(72a06f217158367e97d226b3479896319aa3e089) )
	ROM_LOAD( "bootleg_2c", 0x100000, 0x80000, CRC(d00183b8) SHA1(cb4a6f8fdf3b4f1540e53b956e676a9c2c2e1de0) )
	ROM_LOAD( "bootleg_3c", 0x200000, 0x80000, CRC(c509f3c6) SHA1(4a0553e971d7e17971e0d66ff0ed04f29bc42ca7) )
	ROM_LOAD( "bootleg_4c", 0x300000, 0x80000, CRC(5baa91fa) SHA1(dd0ef9972f6f372a6ed8280c887384b0cfa9bce6) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_6d ) // 040216 modified graphics, some code changes, changed version text to "VIDEO GAME-1 FR01", description says "for Lat-02 terminals", newer set
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_040216d.rom", 0x00000, 0x40000, CRC(c1db9659) SHA1(ff01794b7007b5b0c5e77f9a40d87fb6b125143c) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1d", 0x000000, 0x80000, CRC(72ce6a0c) SHA1(e9a6adc9e077fd2a6242282af3d2edc2d3db2a68) )
	ROM_LOAD( "bootleg_2d", 0x100000, 0x80000, CRC(ca1b02c7) SHA1(bc0f2fdf6f6a21d53150642ee155197e94f159aa) )
	ROM_LOAD( "bootleg_3d", 0x200000, 0x80000, CRC(1f1af38e) SHA1(37877bd13c628f3664f6fd9b2597134735e5397c) )
	ROM_LOAD( "bootleg_4d", 0x300000, 0x80000, CRC(e3696471) SHA1(1065dbca6ffb962329dd8faf41a40ac75358216b) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_7a ) // 050118 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_050118a.rom", 0x00000, 0x40000, CRC(eb2bd908) SHA1(b8e9ef469767fb9e95ff181876ffeaee4b7b9361) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_7b ) // 050118 custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 FR01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_050118b.rom", 0x00000, 0x40000, CRC(5f3067f9) SHA1(db5eb951e481ac169b1b0c2439e6553cd7845383) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1e", 0x000000, 0x80000, CRC(8d8d246a) SHA1(c422a7fb06bf4e94c0fec9de5e6f07ad756c9f02) )
	ROM_LOAD( "bootleg_2e", 0x100000, 0x80000, CRC(a9b1496b) SHA1(09c2316dfce4793b2c3fe8b75b79ebf7cc9dfe3d) )
	ROM_LOAD( "bootleg_3e", 0x200000, 0x80000, CRC(fa755764) SHA1(1e057ede7f3b5739548d45b513f324b83a94a9ce) )
	ROM_LOAD( "bootleg_4e", 0x300000, 0x80000, CRC(83b83f0d) SHA1(a739b3bad3f092eb8fd54662281f0cd7cfd77c62) )
	ROM_LOAD( "bootleg_5e", 0x080000, 0x80000, CRC(a43d6253) SHA1(7d58230efdb89a090b456345a44a363a039cdee8) )
	ROM_LOAD( "bootleg_6e", 0x180000, 0x80000, CRC(32ede872) SHA1(b2f2c8c9ad00ee98d19fcd16efff8e4b28ce44d0) )
	ROM_LOAD( "bootleg_7e", 0x280000, 0x80000, CRC(391b0471) SHA1(5d20ff6a00425b66f6e53766e41afbed34a6aa42) )
	ROM_LOAD( "bootleg_8e", 0x380000, 0x80000, CRC(a634af6b) SHA1(91eeaa7932368b1240a3c0180841b010f5c52362) )
ROM_END

ROM_START( fcockt_7c ) // 050118 payout percentage 40%
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_050118c.rom", 0x00000, 0x40000, CRC(65ce8d61) SHA1(f8b63d0cc5ad314013dad4d566a08c87430817d7) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_7d ) // 050118 payout percentage 60%
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_050118d.rom", 0x00000, 0x40000, CRC(f13165ce) SHA1(5340be60a1f1d530b164eddf9d2717ac752253b9) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_7e ) // 050118 payout percentage 70%
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_050118e.rom", 0x00000, 0x40000, CRC(0f25d1d3) SHA1(2b16a06f6126d110f8664d0874476b687265bb91) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_7f ) // 050118 changed version text to 070305
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_050118f.rom", 0x00000, 0x40000, CRC(a6be3c68) SHA1(53cd6d4d2860ba75c95fa1609efa9ff8ea5e2048) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail_m.001", 0x000000, 0x80000, CRC(da72c0bb) SHA1(41c9eebccce82520dfe41d6a3a574b2890945ffa) )
	ROM_LOAD( "fruitcocktail_m.002", 0x100000, 0x80000, CRC(6239ba9d) SHA1(22486ad30c28341784e7e490255247b82782b72e) )
	ROM_LOAD( "fruitcocktail_m.003", 0x200000, 0x80000, CRC(2c14a464) SHA1(5fce2f4ef95c5054b055db94399946257bc7321f) )
	ROM_LOAD( "fruitcocktail_m.004", 0x300000, 0x80000, CRC(115898f4) SHA1(55b93bddaeede1c2f6b18083a6a2e6329af087cc) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_7g ) // 050118 modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-FC2"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_050118g.rom", 0x00000, 0x40000, CRC(e9d1b425) SHA1(caf76dda7bf2383ce44b81c560d91dbde4d7421e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1h", 0x000000, 0x80000, CRC(5144f9a3) SHA1(e530bb6e1e7c44f7378d6923286f78a74ac34ad4) )
	ROM_LOAD( "bootleg_2h", 0x100000, 0x80000, CRC(c9e57914) SHA1(44e2c3e8739b8943babb18fd7483a84d170cb85e) )
	ROM_LOAD( "bootleg_3h", 0x200000, 0x80000, CRC(f18c6fce) SHA1(632fc7b3b9927071081b66ac0fd01f85e1290266) )
	ROM_LOAD( "bootleg_4h", 0x300000, 0x80000, CRC(9ac3e603) SHA1(f2fab6122d7304543e804b56878f067952f5963d) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_7h ) // 050118 modified graphics, many texts changed, changed version text to "LOTOS FR01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_050118h.rom", 0x00000, 0x40000, CRC(fb191121) SHA1(e08587ac260bc57b16fbb7c7022ae7849946565b) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1i", 0x000000, 0x80000, CRC(5186689f) SHA1(afaacaaff8b4ba37bba4c0fb800c953fa1a37b04) )
	ROM_LOAD( "bootleg_2i", 0x100000, 0x80000, CRC(7263bb27) SHA1(38e46d40b76e83311472192aadcc7daf65a407b1) )
	ROM_LOAD( "bootleg_3i", 0x200000, 0x80000, CRC(02a6e05e) SHA1(dd1f4d50bb46fd3e3b3d8dcb14c5f5084ecf2e35) )
	ROM_LOAD( "bootleg_4i", 0x300000, 0x80000, CRC(99b28f09) SHA1(85eb3788fd413c5d2a131982e158e5e552a930b4) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END

ROM_START( fcockt_8a ) // 060111 modified graphics, many texts changed, changed version text to "LOTO COCKTAIL V01-0001"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_060111a.rom", 0x00000, 0x40000, CRC(d594f40a) SHA1(291c4a84ba90df5a50205c67874f69efb3346194) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1f", 0x000000, 0x80000, CRC(4e905563) SHA1(63f2e74947fd4918d0fa744db85ca646686bd459) )
	ROM_LOAD( "bootleg_2f", 0x100000, 0x80000, CRC(25049350) SHA1(51dae347f4da5dad6bc83fd26a855527fe8cb8ef) )
	ROM_LOAD( "bootleg_3f", 0x200000, 0x80000, CRC(ca9bf738) SHA1(b5f82b9f082ada0299c4585c22fcbf785b62320f) )
	ROM_LOAD( "bootleg_4f", 0x300000, 0x80000, CRC(b65f200c) SHA1(16a3c612a6c6ed1d2956ebba55a933aa2c905974) )
	ROM_LOAD( "bootleg_5",  0x080000, 0x80000, CRC(64b547e3) SHA1(285421fa3aa67a16cf6a9dadb20d74e6a8471dc0) )
	ROM_LOAD( "bootleg_6",  0x180000, 0x80000, CRC(965d6363) SHA1(5c229238a09ec54147d492e9843595962ce79952) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "bootleg_8",  0x380000, 0x80000, CRC(8384e4d4) SHA1(83d0bbbd7cca7328a66a69cf802632fd8d22d5b8) )
ROM_END

ROM_START( fcockt_8b ) // 060111 modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc_m_060111b.rom", 0x00000, 0x40000, CRC(71796ce6) SHA1(e4f4140cce3dbe7735c13b23682e23c9e1331f3a) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1g", 0x000000, 0x80000, CRC(5b658094) SHA1(13801c3267772cecb22578fa6f01c20fe5da687d) )
	ROM_LOAD( "bootleg_2g", 0x100000, 0x80000, CRC(4bbad46c) SHA1(d44d224c18922a7837a47894d2e679a64be7f3cb) )
	ROM_LOAD( "bootleg_3g", 0x200000, 0x80000, CRC(38ad449a) SHA1(eebb826a702cf7a6373a8ef51b4d40ceac758204) )
	ROM_LOAD( "bootleg_4g", 0x300000, 0x80000, CRC(4e676629) SHA1(c85ea9322bdc375f26771b3996d8ca69c43b4d2c) )
	ROM_LOAD( "fruitcocktail_m.005", 0x080000, 0x80000, CRC(f0176b60) SHA1(f764aea00ed306a28cacc62f8d2db9cc42895db8) )
	ROM_LOAD( "fruitcocktail_m.006", 0x180000, 0x80000, CRC(ef24f255) SHA1(6ff924627c179868a25f180f79cd57182b72d9d4) )
	ROM_LOAD( "fruitcocktail_m.007", 0x280000, 0x80000, CRC(20f87a15) SHA1(cb60866a3543668f3592c270b445dee881d78128) )
	ROM_LOAD( "fruitcocktail_m.008", 0x380000, 0x80000, CRC(d282e42e) SHA1(eac9c3eaef39b1805f863ade5da47d6274d20a55) )
ROM_END



ROM_START( lhaunt_4a ) // 031111 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_lh_m_031111a.rom", 0x00000, 0x40000, CRC(83d487c9) SHA1(5b88745d06acba542e2d0660298c9058f2bdfa3f) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "luckyhaunter_m.001", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "luckyhaunter_m.002", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "luckyhaunter_m.003", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "luckyhaunter_m.004", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "luckyhaunter_m.005", 0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006", 0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007", 0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008", 0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_5a ) // 040216 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_lh_m_040216a.rom", 0x00000, 0x40000, CRC(2a6c7ff9) SHA1(4a0137c7df5003e8fd843d5489d416d15f001f46) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "luckyhaunter_m.001", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "luckyhaunter_m.002", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "luckyhaunter_m.003", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "luckyhaunter_m.004", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "luckyhaunter_m.005", 0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006", 0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007", 0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008", 0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_6a ) // 040825 backdoor 1,5 9,1 5,1 1,5  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_lh_m_040825a.rom", 0x00000, 0x40000, CRC(18ba5704) SHA1(3c77ed129db0e5181217167b76292f8e4ee78728) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "luckyhaunter_m.001", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "luckyhaunter_m.002", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "luckyhaunter_m.003", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "luckyhaunter_m.004", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "luckyhaunter_m.005", 0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006", 0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007", 0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008", 0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_6b ) // 040825 custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 PB01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_lh_m_040825b.rom", 0x00000, 0x40000, CRC(71bc9811) SHA1(64dc0d81aec657b8c722e530ad5858d831ab7d51) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(82e0dd1a) SHA1(7f99fe1e5d304fadae9f1f46ebe8680787c9175b) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(fa41dc9b) SHA1(3a2d46cc3bb1e5a8c9a42e28e0b195548d365865) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(761fcd19) SHA1(08c5eb77a8829f4cdb7508a1f3c2361f8bdefb17) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(be5f40c3) SHA1(62ac5b3c98db7352d241a115030aef0266c9e737) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(99fb98df) SHA1(66dd5a07ce72d88f5612c3dc8ec2bcd76229a586) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(208fb837) SHA1(857a615fa674cd55771cc4c12afd61fe8a891d3c) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(a4d609cd) SHA1(bcf193745012a59ec706f54c157dd413b49f0a35) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(6703695b) SHA1(7730aa1297cc1584cfe1236c16f1a90829727baa) )
ROM_END

ROM_START( lhaunt_6c ) // 040825 changed version text to 070604
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_lh_m_040825c.rom", 0x00000, 0x40000, CRC(b2b027c2) SHA1(18e5d7b396103a7301b2a6641ca3d85ab2091d2e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "luckyhaunter_m.001", 0x000000, 0x80000, CRC(5f1000c6) SHA1(05154d786caf7f5fb9ed90c7d3391cec42e871f8) )
	ROM_LOAD( "luckyhaunter_m.002", 0x100000, 0x80000, CRC(b18abebc) SHA1(66c28fd3f338360b9236dcf414548bfb53655951) )
	ROM_LOAD( "luckyhaunter_m.003", 0x200000, 0x80000, CRC(2e67a1d9) SHA1(a3eff78f25e4e4878706d7c9e4ca71f6914006c6) )
	ROM_LOAD( "luckyhaunter_m.004", 0x300000, 0x80000, CRC(90c963f6) SHA1(2a9c689315cdfb67425f0710511dc0e0241741a7) )
	ROM_LOAD( "luckyhaunter_m.005", 0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006", 0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007", 0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008", 0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_6d ) // 040825 modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_lh_m_040825d.rom", 0x00000, 0x40000, CRC(bf442051) SHA1(0c96b5cbf24dc8786feae658f23518bef6c0d589) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1c", 0x000000, 0x80000, CRC(9ff7618e) SHA1(1c191a211f2391f5aa5786c2071dc34d909bbe46) )
	ROM_LOAD( "bootleg_2c", 0x100000, 0x80000, CRC(094e040f) SHA1(254348b1690ce82bb2551006365d630fb82413c2) )
	ROM_LOAD( "bootleg_3c", 0x200000, 0x80000, CRC(47f7d726) SHA1(d43797c50fa0a8f25a5cd198e906f67d9f3e9a13) )
	ROM_LOAD( "bootleg_4c", 0x300000, 0x80000, CRC(1aa0998f) SHA1(078c09774ac3664a76d83b28596d0e07f3f7b9e2) )
	ROM_LOAD( "luckyhaunter_m.005", 0x080000, 0x80000, CRC(b50c90a3) SHA1(74749f4ffb5b0630631b511fc3230c6e7b50dc3b) )
	ROM_LOAD( "luckyhaunter_m.006", 0x180000, 0x80000, CRC(4eaaab64) SHA1(9fed16f8e0308200fd16c4b1e511e1bf6c22ae66) )
	ROM_LOAD( "luckyhaunter_m.007", 0x280000, 0x80000, CRC(64d16ba9) SHA1(3b897183d6e0f1256be7657441f234fc72077682) )
	ROM_LOAD( "luckyhaunter_m.008", 0x380000, 0x80000, CRC(1bdf6252) SHA1(7b5ae82a95a744b236e109024d47b526dccf9c14) )
ROM_END

ROM_START( lhaunt_6e ) // 040825 modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-LH2"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_lh_m_040825e.rom", 0x00000, 0x40000, CRC(087d4172) SHA1(ec9ca4192031d8b8e3cbaf809e8d1643978481f3) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1d", 0x000000, 0x80000, CRC(41e22133) SHA1(b3d88b62fed9aa4ea5bbdff3f92a4120a239cfb0) )
	ROM_LOAD( "bootleg_2d", 0x100000, 0x80000, CRC(ea948a43) SHA1(ccf0450ed095cf4d48a0775a50a630d7e2d50fcb) )
	ROM_LOAD( "bootleg_3d", 0x200000, 0x80000, CRC(df715e6b) SHA1(be8c143be66a638eeb971406e323c3e857a50e19) )
	ROM_LOAD( "bootleg_4d", 0x300000, 0x80000, CRC(e70b3a0e) SHA1(f3905c117467fb6817fd02ce08af6188439821bf) )
	ROM_LOAD( "bootleg_5d", 0x080000, 0x80000, CRC(666d1923) SHA1(5ef416c2c567c56d2188ba27d8f0f205551eca7f) )
	ROM_LOAD( "bootleg_6d", 0x180000, 0x80000, CRC(304475f2) SHA1(de532496c4534ff09c88d87d0ccadad542ff5637) )
	ROM_LOAD( "bootleg_7d", 0x280000, 0x80000, CRC(278aab12) SHA1(cb35e95babddcb126c10dfd6abf2986133a6fbf1) )
	ROM_LOAD( "bootleg_8d", 0x380000, 0x80000, CRC(79c110bc) SHA1(4198b6003af7e52720f84776f51fe4e1c1cc8713) )
ROM_END

ROM_START( lhaunt_6f ) // 040825 custom alteras, modified graphics, many texts changed, changed version text to "LOTOS PB01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_lh_m_040825f.rom", 0x00000, 0x40000, CRC(7033af62) SHA1(c6b859672816f650eb88bc363e6f6d3a197eb99c) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1e", 0x000000, 0x80000, CRC(c7b0191a) SHA1(d33ababbefcb39f4e6b063f448b8a08b015d6f24) )
	ROM_LOAD( "bootleg_2e", 0x100000, 0x80000, CRC(befb39e6) SHA1(6d6fdbb6e74d7418037dda5f76c8f007508df916) )
	ROM_LOAD( "bootleg_3e", 0x200000, 0x80000, CRC(a9cdb4b4) SHA1(7054e47913d8e14d3219d6780f912f730e3a67cb) )
	ROM_LOAD( "bootleg_4e", 0x300000, 0x80000, CRC(302cf08c) SHA1(eede494a0d47f5c911868a785145994f5e72829e) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(99fb98df) SHA1(66dd5a07ce72d88f5612c3dc8ec2bcd76229a586) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(208fb837) SHA1(857a615fa674cd55771cc4c12afd61fe8a891d3c) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(a4d609cd) SHA1(bcf193745012a59ec706f54c157dd413b49f0a35) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(6703695b) SHA1(7730aa1297cc1584cfe1236c16f1a90829727baa) )
ROM_END



ROM_START( garage_4a ) // 040219 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gg_m_040219a.rom", 0x00000, 0x40000, CRC(e16b213a) SHA1(af0d78116d985efe5f09eb86eb67df2535765527) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "garage_m.001", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "garage_m.002", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "garage_m.003", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "garage_m.004", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "garage_m.005", 0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "garage_m.006", 0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "garage_m.007", 0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "garage_m.008", 0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_4b ) // 040219 changed version text to 070329
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gg_m_040219b.rom", 0x00000, 0x40000, CRC(b8ebf3b6) SHA1(14c5263571123d251125723b32861bef578de67b) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "garage_m.001", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "garage_m.002", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "garage_m.003", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "garage_m.004", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "garage_m.005", 0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "garage_m.006", 0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "garage_m.007", 0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "garage_m.008", 0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_4c ) // 040219 modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-GG2"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gg_m_040219c.rom", 0x00000, 0x40000, CRC(2e0ee194) SHA1(2dab6f80cd81b34792c1d347e17c2410bf22a48b) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1d", 0x000000, 0x80000, CRC(63cf83f8) SHA1(ac96acd353049e77855cee618521fdba17e1b3f1) )
	ROM_LOAD( "bootleg_2d", 0x100000, 0x80000, CRC(be09cf31) SHA1(067abe9a58cc1ec42524cda33d9c899625ef9e12) )
	ROM_LOAD( "bootleg_3d", 0x200000, 0x80000, CRC(7da936ed) SHA1(f13cb3b3c3e3a3aa39bed72601ca7b6d5f517579) )
	ROM_LOAD( "bootleg_4d", 0x300000, 0x80000, CRC(8f12f84e) SHA1(10f774a36382b52e35738f6060cbe8bbd990dc66) )
	ROM_LOAD( "bootleg_5d", 0x080000, 0x80000, CRC(39ca3b8a) SHA1(e456c2965f1d4519ee1b8b9e7689004b467c4957) )
	ROM_LOAD( "bootleg_6d", 0x180000, 0x80000, CRC(df4ff9ff) SHA1(89cc80923544e44367ba27e42f4f49744de9782c) )
	ROM_LOAD( "bootleg_7d", 0x280000, 0x80000, CRC(e65de46e) SHA1(ffcc48449e764329864800befcb31b884bd7ff20) )
	ROM_LOAD( "bootleg_8d", 0x380000, 0x80000, CRC(15fcac6d) SHA1(3c1f44225686b42eba5229fb0ad03d2592c09203) )
ROM_END

ROM_START( garage_5a ) // 050311 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gg_m_050311a.rom", 0x00000, 0x40000, CRC(874a2c27) SHA1(9bf586314f375c2c6f7d79557cc777ac3559cb64) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "garage_m.001", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "garage_m.002", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "garage_m.003", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "garage_m.004", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "garage_m.005", 0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "garage_m.006", 0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "garage_m.007", 0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "garage_m.008", 0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_5b ) // 050311 custom alteras, modified graphics, changed version text to "VIDEO GAME-1 GA01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gg_m_050311b.rom", 0x00000, 0x40000, CRC(79f9a8e7) SHA1(f6ddd586b72ca004ac18284a0394d5a9d56fbc19) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(df71e54b) SHA1(e5899151aea13b527e4f0819609be6d9ee8cc1b2) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(72067f6a) SHA1(be1d4e72b03d32dce8f0f86f860b1e976ab6d459) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(a432329a) SHA1(ab526e4c1909d205f69f934a3a808337d087b1c7) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(44bcf56b) SHA1(19fd698d8d712e00735e2a70836ef3135de0e351) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(9d210b24) SHA1(0bbf2a855880b9f82e3f356adb7b151db3daaed6) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(45b039a0) SHA1(1166815fa1271e44d08bad1f5a3a786c93cb5030) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(b4e1ecdb) SHA1(7cd5ffbe21ff48434f3be995388e6bc398fe2aa4) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(06c5149f) SHA1(012573a144532bb6d697c0390b97136e47aaca99) )
ROM_END

ROM_START( garage_5c ) // 050311 payout percentage 70%
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gg_m_050311c.rom", 0x00000, 0x40000, CRC(039af644) SHA1(548e2990f15d6f7f68758a91aae5bf297a220698) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "garage_m.001", 0x000000, 0x80000, CRC(57acd4bc) SHA1(8796f463787c359cce6ac09c1b6895f871bbb7c9) )
	ROM_LOAD( "garage_m.002", 0x100000, 0x80000, CRC(6d591fa3) SHA1(ddbdf87e0e88dc848b963fbfcb6e14d7b3b9efdc) )
	ROM_LOAD( "garage_m.003", 0x200000, 0x80000, CRC(6a15eeda) SHA1(ac35a20893b0518a159207401f6b7f58e3de45fa) )
	ROM_LOAD( "garage_m.004", 0x300000, 0x80000, CRC(38f2cd3c) SHA1(5a4463ac352e4e340c6aaa61102841541e9f4c48) )
	ROM_LOAD( "garage_m.005", 0x080000, 0x80000, CRC(5bf85bc5) SHA1(ff9d2b9cbcd2af6f5fda972e387820d830c196a9) )
	ROM_LOAD( "garage_m.006", 0x180000, 0x80000, CRC(e5082b26) SHA1(6547409d39dd51498ce8e3f82ff813a8ac3c6522) )
	ROM_LOAD( "garage_m.007", 0x280000, 0x80000, CRC(dfa2ceb1) SHA1(ec7de8a8f6e7785a563df973841cc1f1603f79fc) )
	ROM_LOAD( "garage_m.008", 0x380000, 0x80000, CRC(90c5416b) SHA1(583bfb517bc2e30d7b7903aa19fc3b4b5188d7d2) )
ROM_END

ROM_START( garage_5d ) // 050311 modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gg_m_050311d.rom", 0x00000, 0x40000, CRC(00db1210) SHA1(90eb11d6055b464546657233b44a6783c34161ae) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1c", 0x000000, 0x80000, CRC(e3deb6f3) SHA1(a1fe01040bd8f4b08268e2b99cfa240e50dce00b) )
	ROM_LOAD( "bootleg_2c", 0x100000, 0x80000, CRC(540abed4) SHA1(c3f7c9c9c71da2092b13cafd51017751b65eea44) )
	ROM_LOAD( "bootleg_3c", 0x200000, 0x80000, CRC(c8ff1697) SHA1(6edc15793ce28abcd85b4bfcdb8dec3b6b9fc86b) )
	ROM_LOAD( "bootleg_4c", 0x300000, 0x80000, CRC(5cf6c32a) SHA1(4f3baf788a7a868bee3343ac623e0d73d11833dd) )
	ROM_LOAD( "bootleg_5c", 0x080000, 0x80000, CRC(5670c51f) SHA1(823d2152342df0129dbe91dceb58282919a30bd1) )
	ROM_LOAD( "bootleg_6c", 0x180000, 0x80000, CRC(b8a6a574) SHA1(157d68736960f351b148be0b81bcdba472437508) )
	ROM_LOAD( "bootleg_7c", 0x280000, 0x80000, CRC(94f2b464) SHA1(879c435a80eb8770fe47d4120078c6a447025f08) )
	ROM_LOAD( "bootleg_8c", 0x380000, 0x80000, CRC(0cf53598) SHA1(c456add6b56169a5116643a27aace1ee19a68340) )
ROM_END

ROM_START( garage_5e ) // 050311 custom alteras, modified graphics, many texts changed, changed version text to "LOTOS GA01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gg_m_050311e.rom", 0x00000, 0x40000, CRC(2c7971ba) SHA1(512c6a76d22ba46c9bbb6cc1255f156a8d20befe) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1e", 0x000000, 0x80000, CRC(c89cc739) SHA1(31e62dd6ef841dc3f2a3da3e89649b7250cc3a6d) )
	ROM_LOAD( "bootleg_2e", 0x100000, 0x80000, CRC(c224a356) SHA1(c684b69ca8998f74d13edc56f3c3201c6cce6e01) )
	ROM_LOAD( "bootleg_3e", 0x200000, 0x80000, CRC(a83cfda9) SHA1(c754b47da998cf9aa11a2332ba66756599d79340) )
	ROM_LOAD( "bootleg_4e", 0x300000, 0x80000, CRC(e262b94c) SHA1(f6e22a38226a55b99ddd9fcf714599acbd3ee27c) )
	ROM_LOAD( "bootleg_5e", 0x080000, 0x80000, CRC(ad6c6919) SHA1(ff152adf63372a203d2a2f629766706f482d8065) )
	ROM_LOAD( "bootleg_6e", 0x180000, 0x80000, CRC(ddbd373e) SHA1(cebb6b666cc1ee6e1f0a27c6b989f3818d67419b) )
	ROM_LOAD( "bootleg_7e", 0x280000, 0x80000, CRC(13880471) SHA1(037b5e331da282aba310bc72346d66a0b4a08e6d) )
	ROM_LOAD( "bootleg_8e", 0x380000, 0x80000, CRC(10cc710b) SHA1(2bcf01b9d4730d8377edfd563c1c508ea128fd46) )
ROM_END



ROM_START( rclimb_3a ) // 040827 backdoor 1,5 9,1 5,1 1,5  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rc_m_040827a.rom", 0x00000, 0x40000, CRC(6420f8b4) SHA1(64e4018dbea245ddc06a65fb2f8cf38e77f60999) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "rockclimber_m.001", 0x000000, 0x80000, CRC(934f18c7) SHA1(da3a7cddc68e104d415d947e89c0e7f0d067c056) )
	ROM_LOAD( "rockclimber_m.002", 0x100000, 0x80000, CRC(7364bd2b) SHA1(c0edfd3b8de813c95fe5d6072662fa0e39fec89e) )
	ROM_LOAD( "rockclimber_m.003", 0x200000, 0x80000, CRC(e7befb17) SHA1(8a214680142cd657784a667ab3f6422165fea224) )
	ROM_LOAD( "rockclimber_m.004", 0x300000, 0x80000, CRC(dc6d43a0) SHA1(62fc47136775f3fa9369857ec91fe897a1f1ebd6) )
	ROM_LOAD( "rockclimber_m.005", 0x080000, 0x80000, CRC(ea127c3d) SHA1(a6391eed69a4723b68d727f59b6baebe51633e66) )
	ROM_LOAD( "rockclimber_m.006", 0x180000, 0x80000, CRC(277fa273) SHA1(6320e6c5b5e48dc451cc48189054c42d85e8ccc1) )
	ROM_LOAD( "rockclimber_m.007", 0x280000, 0x80000, CRC(3ca7f69a) SHA1(878cca181d915dc3548d5285a4bbb51aef31a64e) )
	ROM_LOAD( "rockclimber_m.008", 0x380000, 0x80000, CRC(8cf6b4c2) SHA1(4c36c217b83c82acfdd615f5547bf597af7b8833) )
ROM_END

ROM_START( rclimb_3b ) // 040827 - new service menu
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rc_m_040827.rom", 0x00000, 0x40000, CRC(3ba55647) SHA1(56e96be0d9782da4b3d5d911ea67962257626ae0) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_rc1_.bin", 0x000000, 0x80000, CRC(4bc00c66) SHA1(e4d89c2b188f253b642ae341a1a4c04af33024c8) )
	ROM_LOAD( "bootleg_rc2_.bin", 0x100000, 0x80000, CRC(89237f10) SHA1(a493a03f79656332089f2794872be44e62d7e306) )
	ROM_LOAD( "bootleg_rc3_.bin", 0x200000, 0x80000, CRC(c4147d05) SHA1(8497fca6e64896cf8f03877c3455ee7bf9965b60) )
	ROM_LOAD( "bootleg_rc4_.bin", 0x300000, 0x80000, CRC(06176cb1) SHA1(0644861042ca4be2d459b31870369a2e46d80aa4) )
	ROM_LOAD( "bootleg_rc5_.bin", 0x080000, 0x80000, CRC(fb5f2036) SHA1(5f8885332a2d9249d34a0a50ac464a5637b9bc95) )
	ROM_LOAD( "bootleg_rc6_.bin", 0x180000, 0x80000, CRC(36a8148f) SHA1(57d84e44c77f2bb1e97ec8a1acd3ef71246e3274) )
	ROM_LOAD( "bootleg_rc7_.bin", 0x280000, 0x80000, CRC(26c620e3) SHA1(b14f2cfc0e5b740360e6ecca97f6bf941136141f) )
	ROM_LOAD( "bootleg_rc8_.bin", 0x380000, 0x80000, CRC(8cf6b4c2) SHA1(4c36c217b83c82acfdd615f5547bf597af7b8833) )
ROM_END

ROM_START( rclimb_3c ) // 040827 custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 SK01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rc_m_040827c.rom", 0x00000, 0x40000, CRC(4b8b3e0e) SHA1(6554073d589de67807c3b3779c1f2fa610aa03df) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(a66e718a) SHA1(84493efde5a70032e709ebde5635ed50e151d2ab) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(3486c691) SHA1(658e0f7e1e7c874c6e8f9f1908d242ad57f66cf1) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(4600b9c7) SHA1(ce8e8474705592b246933662b24c579bee45d6b1) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(610826f6) SHA1(52ad56da44bf3803ea1a30fbeaf7c7cab3c347dc) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(77413756) SHA1(41eac1ee200d9d850e494f79555a82bb2a591e1f) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(6f8f3678) SHA1(b9c0e5093f93a3e9ebc715a24588a68105ca15c7) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(72d3cba8) SHA1(2f14bc23574276cb1ceef4306653e79f7bf19ac9) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(4210c5e2) SHA1(053cee86e3fa27e43ef74615cd4bb1f28c900ec4) )
ROM_END

ROM_START( rclimb_3d ) // 040827 modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rc_m_040827d.rom", 0x00000, 0x40000, CRC(b7796cff) SHA1(a180397d8827adbc161e282a4ee6d064d4837199) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1c", 0x000000, 0x80000, CRC(b846d5c8) SHA1(235ebe08ef89b308dd9642ebb8ce0eb102ffaf7d) )
	ROM_LOAD( "bootleg_2c", 0x100000, 0x80000, CRC(600cfee9) SHA1(b0f251cd9e6bff195f44bd70a961ccc489688574) )
	ROM_LOAD( "bootleg_3c", 0x200000, 0x80000, CRC(d8394c53) SHA1(a20862c0753cba6b5f4db03c9a208381c769a3e5) )
	ROM_LOAD( "bootleg_4c", 0x300000, 0x80000, CRC(7b0911b8) SHA1(25fa9b021cc041eccbbf0af1ec613895cc79a973) )
	ROM_LOAD( "rockclimber_m.005", 0x080000, 0x80000, CRC(ea127c3d) SHA1(a6391eed69a4723b68d727f59b6baebe51633e66) )
	ROM_LOAD( "rockclimber_m.006", 0x180000, 0x80000, CRC(277fa273) SHA1(6320e6c5b5e48dc451cc48189054c42d85e8ccc1) )
	ROM_LOAD( "rockclimber_m.007", 0x280000, 0x80000, CRC(3ca7f69a) SHA1(878cca181d915dc3548d5285a4bbb51aef31a64e) )
	ROM_LOAD( "rockclimber_m.008", 0x380000, 0x80000, CRC(8cf6b4c2) SHA1(4c36c217b83c82acfdd615f5547bf597af7b8833) )
ROM_END

ROM_START( rclimb_3e ) // 040827 custom alteras, modified graphics, many texts changed, changed version text to "LOTOS SK01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rc_m_040827e.rom", 0x00000, 0x40000, CRC(0ae53856) SHA1(af6b744addc8fb429464e5dbe28c8ebcd2925dec) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1d", 0x000000, 0x80000, CRC(085caab6) SHA1(32a6a9160f9a175ba74ef60d36e1715efe7599b4) )
	ROM_LOAD( "bootleg_2d", 0x100000, 0x80000, CRC(29af4561) SHA1(dcc9f8b910a8b55abb79d47ce6b3e3c0fc37f2e8) )
	ROM_LOAD( "bootleg_3d", 0x200000, 0x80000, CRC(f0576bfd) SHA1(3b51893589843f52c6d83328c8f0cdb80e094e07) )
	ROM_LOAD( "bootleg_4d", 0x300000, 0x80000, CRC(66e10e16) SHA1(3ed013ec287347d817345815c7e8d2a6a4e5563a) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(77413756) SHA1(41eac1ee200d9d850e494f79555a82bb2a591e1f) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(6f8f3678) SHA1(b9c0e5093f93a3e9ebc715a24588a68105ca15c7) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(72d3cba8) SHA1(2f14bc23574276cb1ceef4306653e79f7bf19ac9) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(4210c5e2) SHA1(053cee86e3fa27e43ef74615cd4bb1f28c900ec4) )
ROM_END



ROM_START( sweetla ) // 041220 backdoor 1,5 9,1 5,3 1,5  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_sl_m_041220a.rom", 0x00000, 0x40000, CRC(920fd9fe) SHA1(0b5ad099ae4c8e3ba0f99baf8fc9322cae24e9d2) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "sweetlife.001", 0x000000, 0x80000, CRC(a096c786) SHA1(81f6b083cb089e9412a8506889196354c670d945) )
	ROM_LOAD( "sweetlife.002", 0x100000, 0x80000, CRC(c5e1e22c) SHA1(973ad27681a0f3beee7084b1b85fc9deb79d638e) )
	ROM_LOAD( "sweetlife.003", 0x200000, 0x80000, CRC(af335323) SHA1(b8afdce231a8ec0f313cc47e00a27f05461bbbc4) )
	ROM_LOAD( "sweetlife.004", 0x300000, 0x80000, CRC(a35c7503) SHA1(78f7a868660bbaa066e8e9e341db52018aaf3af1) )
	ROM_LOAD( "sweetlife.005", 0x080000, 0x80000, CRC(e2d6b632) SHA1(65d05e55671b8c335cae2dfbf6a6f5bd8cc90e2c) )
	ROM_LOAD( "sweetlife.006", 0x180000, 0x80000, CRC(d34e0905) SHA1(cc4afe64fb9052a31f759be41ff07a727e0a9093) )
	ROM_LOAD( "sweetlife.007", 0x280000, 0x80000, CRC(978b67bb) SHA1(87357d5832588f00272bd76df736c06c599f3853) )
	ROM_LOAD( "sweetlife.008", 0x380000, 0x80000, CRC(75954355) SHA1(e6ef2b70d859b61e8e3d1751de8558b8778e502d) )
ROM_END

ROM_START( sweetlb ) // 041220 bank F9, changed version text to 070412
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_sl_m_041220b.rom", 0x00000, 0x40000, CRC(53f1a0e1) SHA1(a00a8e8d95bd2be27313c1421d40120ee79ff29d) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "sweetlife.001", 0x000000, 0x80000, CRC(a096c786) SHA1(81f6b083cb089e9412a8506889196354c670d945) )
	ROM_LOAD( "sweetlife.002", 0x100000, 0x80000, CRC(c5e1e22c) SHA1(973ad27681a0f3beee7084b1b85fc9deb79d638e) )
	ROM_LOAD( "sweetlife.003", 0x200000, 0x80000, CRC(af335323) SHA1(b8afdce231a8ec0f313cc47e00a27f05461bbbc4) )
	ROM_LOAD( "sweetlife.004", 0x300000, 0x80000, CRC(a35c7503) SHA1(78f7a868660bbaa066e8e9e341db52018aaf3af1) )
	ROM_LOAD( "sweetlife.005", 0x080000, 0x80000, CRC(e2d6b632) SHA1(65d05e55671b8c335cae2dfbf6a6f5bd8cc90e2c) )
	ROM_LOAD( "sweetlife.006", 0x180000, 0x80000, CRC(d34e0905) SHA1(cc4afe64fb9052a31f759be41ff07a727e0a9093) )
	ROM_LOAD( "sweetlife.007", 0x280000, 0x80000, CRC(978b67bb) SHA1(87357d5832588f00272bd76df736c06c599f3853) )
	ROM_LOAD( "sweetlife.008", 0x380000, 0x80000, CRC(75954355) SHA1(e6ef2b70d859b61e8e3d1751de8558b8778e502d) )
ROM_END


ROM_START( sweetl2_2a ) // 080320 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_sl2_m_080320a.rom", 0x00000, 0x40000, CRC(425066e9) SHA1(f2b11c3c6e08cb59c7c6841ee3575503ed4abb80) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "sweetlife2.001", 0x000000, 0x80000, CRC(b1e6157c) SHA1(e042aaaf85b13865d56d1709be280f5a3d5c95e3) )
	ROM_LOAD( "sweetlife2.002", 0x100000, 0x80000, CRC(8bc54047) SHA1(9179cf02376fec77ae50b5603de66754237698f8) )
	ROM_LOAD( "sweetlife2.003", 0x200000, 0x80000, CRC(709c5d34) SHA1(d702576d70ebef667c77376effecc02abbaaa8cd) )
	ROM_LOAD( "sweetlife2.004", 0x300000, 0x80000, CRC(7bcfa2ce) SHA1(8df894e9b7afa52e47e4dabd1802e878bbbeaa80) )
	ROM_LOAD( "sweetlife2.005", 0x080000, 0x80000, CRC(5c385d43) SHA1(ddb362e5894d146ce90acbaafe9dd7aad2a7c242) )
	ROM_LOAD( "sweetlife2.006", 0x180000, 0x80000, CRC(868fe1cb) SHA1(692679f8242950e009c30cc084c4ddc5d1963502) )
	ROM_LOAD( "sweetlife2.007", 0x280000, 0x80000, CRC(6ce87282) SHA1(586e08994db4ca2b967d47b16ba5b458e240d30f) )
	ROM_LOAD( "sweetlife2.008", 0x380000, 0x80000, CRC(c2ad2b74) SHA1(c78e3ca5d15acb17ee671d2205405f287ad9c464) )
ROM_END

ROM_START( sweetl2_2b ) // 080320 bank F9, some fixes
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_sl2_m_080320b.rom", 0x00000, 0x40000, CRC(faffe8c3) SHA1(66a8247fc4bc6d7fb2eefcec37d56eabb1fbca57) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "sweetlife2.001", 0x000000, 0x80000, CRC(b1e6157c) SHA1(e042aaaf85b13865d56d1709be280f5a3d5c95e3) )
	ROM_LOAD( "sweetlife2.002", 0x100000, 0x80000, CRC(8bc54047) SHA1(9179cf02376fec77ae50b5603de66754237698f8) )
	ROM_LOAD( "sweetlife2.003", 0x200000, 0x80000, CRC(709c5d34) SHA1(d702576d70ebef667c77376effecc02abbaaa8cd) )
	ROM_LOAD( "sweetlife2.004", 0x300000, 0x80000, CRC(7bcfa2ce) SHA1(8df894e9b7afa52e47e4dabd1802e878bbbeaa80) )
	ROM_LOAD( "sweetlife2.005", 0x080000, 0x80000, CRC(5c385d43) SHA1(ddb362e5894d146ce90acbaafe9dd7aad2a7c242) )
	ROM_LOAD( "sweetlife2.006", 0x180000, 0x80000, CRC(868fe1cb) SHA1(692679f8242950e009c30cc084c4ddc5d1963502) )
	ROM_LOAD( "sweetlife2.007", 0x280000, 0x80000, CRC(6ce87282) SHA1(586e08994db4ca2b967d47b16ba5b458e240d30f) )
	ROM_LOAD( "sweetlife2.008", 0x380000, 0x80000, CRC(c2ad2b74) SHA1(c78e3ca5d15acb17ee671d2205405f287ad9c464) )
ROM_END

ROM_START( sweetl2_2c ) // 080320 modified graphics, bank F9, changed version text to "VIDEO GAME-1 MD01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_sl2_m_080320c.rom", 0x00000, 0x40000, CRC(d31d682d) SHA1(a8addbed8373f06d90648c01dc6fda584789f25c) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1a", 0x000000, 0x80000, CRC(aa9e41f4) SHA1(50c4078581e902aab0dfd902c5ae82ba50f03e39) )
	ROM_LOAD( "bootleg_2a", 0x100000, 0x80000, CRC(35a92e6b) SHA1(48a1856a8970383619dd5acd3026630ce8c2dfe9) )
	ROM_LOAD( "bootleg_3a", 0x200000, 0x80000, CRC(b8bd46b4) SHA1(c4c9c6c94b7d6e2e6b02f61177407464b75c76bb) )
	ROM_LOAD( "bootleg_4a", 0x300000, 0x80000, CRC(e4a8f35d) SHA1(f45dbdc5757bd86218e2c975f9c127c2b3077aef) )
	ROM_LOAD( "bootleg_5a", 0x080000, 0x80000, CRC(7bc3d048) SHA1(4485c028757b4ade722f7a4d72ce87bc94c08a08) )
	ROM_LOAD( "bootleg_6a", 0x180000, 0x80000, CRC(c334b279) SHA1(55bace43fe2aee54ab9cc787a955f382f05cb666) )
	ROM_LOAD( "bootleg_7a", 0x280000, 0x80000, CRC(9e8cfe95) SHA1(2594c66d65ec4b23fe7760ed7f05761b95ec1bde) )
	ROM_LOAD( "bootleg_8a", 0x380000, 0x80000, CRC(f4b48eee) SHA1(02af6ea807d003504a37c4dc7f9111b716cf78a5) )
ROM_END

ROM_START( sweetl2_2d ) // 080320 bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_sl2_m_080320d.rom", 0x00000, 0x40000, CRC(03b26c85) SHA1(9c05e48f8d2e467da8b267021560b121538b8c3e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(a5119aca) SHA1(ed59477438ba2b5f17f7c3ab9fa769f5aa28ed69) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(689f09f5) SHA1(8bf13f453e51a0d1abbc00a2fc92e8f544cb176f) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(925abdcb) SHA1(93ffea087f754614e3ceaf43c00a94a3525f4ef8) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(94c525a3) SHA1(0c67fd937c85774c10d6671c7ea4b9733d760cdc) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(98d76993) SHA1(39dc0b650d5437f5fae70651ed67dc3f80ea45a4) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(f85255d9) SHA1(eb9e8de3775c9a1ddc1134c7b96e556ec2e87b1d) )
	ROM_LOAD( "sweetlife2.007",  0x280000, 0x80000, CRC(6ce87282) SHA1(586e08994db4ca2b967d47b16ba5b458e240d30f) )
	ROM_LOAD( "sweetlife2.008",  0x380000, 0x80000, CRC(c2ad2b74) SHA1(c78e3ca5d15acb17ee671d2205405f287ad9c464) )
ROM_END



ROM_START( resdnt_2a ) // 040513 backdoor 1,5 9,1 5,1 1,5  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rs_m_040513a.rom", 0x00000, 0x40000, CRC(5b6480d7) SHA1(e54ddc822819136687d613ce4f38dd98f3e01bb5) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "resident_m.001", 0x000000, 0x80000, CRC(e0645da6) SHA1(dd72f4830d8011f603aa6d430f34ac2598005281) )
	ROM_LOAD( "resident_m.002", 0x100000, 0x80000, CRC(dd8de247) SHA1(498c5b931ce65e289f52d8864b603166f81e3dc4) )
	ROM_LOAD( "resident_m.003", 0x200000, 0x80000, CRC(0d346ec2) SHA1(e2456b28825c54c5e16829525627c40611c0083d) )
	ROM_LOAD( "resident_m.004", 0x300000, 0x80000, CRC(1f95aad9) SHA1(51d003288d5ff23b3c981fbaa99d29b66dd2c101) )
	ROM_LOAD( "resident_m.005", 0x080000, 0x80000, CRC(0cfe7d44) SHA1(9f0e4925e815ff9f79188f18e78c0a7b377daa3f) )
	ROM_LOAD( "resident_m.006", 0x180000, 0x80000, CRC(7437904f) SHA1(630c79cd6a990ce7658a1ffabba5a27efba985a1) )
	ROM_LOAD( "resident_m.007", 0x280000, 0x80000, CRC(6e94728a) SHA1(ab414879cb957d9bc8d653b5e3bb2bbf91139ec0) )
	ROM_LOAD( "resident_m.008", 0x380000, 0x80000, CRC(a9f55043) SHA1(4771df3d45bdc0a21b1c479f45e09ac5bab6c94f) )
ROM_END

ROM_START( resdnt_2b ) // 040513 custom alteras, modified graphics, changed version text to "VIDEO GAME-1 SE01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rs_m_040513b.rom", 0x00000, 0x40000, CRC(2f8b2249) SHA1(4fb34888a99946937791fdba4659d256e862598b) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(529ac635) SHA1(76cb5b04d61a1e5cb358b55a1b8a5602edd2841a) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(84033d15) SHA1(632a18b059ec94d195e311080baf1f5f7d9c8e6c) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(2b456ab0) SHA1(bf7971bd6ad6fcf7bc964b5c30cf9ead828e9fd3) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(4e02adf4) SHA1(63d2e40577171be0f8c2591d7cd3500c83f3ab1f) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(9613a21c) SHA1(d514cd8e6d7695dc8be52bc5acb04991ebaf3499) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(2a4364c6) SHA1(f2760d7416585123a46ffb47ab9da008532f64bb) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(a34d91d0) SHA1(9616952b1e8b6c06c8bca3c776bdfa3526ef60cf) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(a0be7be4) SHA1(d6f4a1ba22be638fa801923960da8f5f72fc16c0) )
ROM_END

ROM_START( resdnt_2c ) // 040513 custom alteras, modified graphics, changed version text to "VIDEO GAME-1 SE01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rs_m_040513b.rom", 0x00000, 0x40000, CRC(2f8b2249) SHA1(4fb34888a99946937791fdba4659d256e862598b) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1c", 0x000000, 0x80000, CRC(31bdf1c6) SHA1(b4c1555cd1cd1eaec5e39ed5879a99633fca5c46) )
	ROM_LOAD( "bootleg_2c", 0x100000, 0x80000, CRC(861fbf05) SHA1(6348236124792719afbd0117dea35a6af059355c) )
	ROM_LOAD( "bootleg_3c", 0x200000, 0x80000, CRC(7de96e4a) SHA1(1641655b95b35825de237b07142f4097a757cf6c) )
	ROM_LOAD( "bootleg_4c", 0x300000, 0x80000, CRC(179cf46c) SHA1(1585a6e54b393154c1e7aa6e5e116d45f34da07b) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(9613a21c) SHA1(d514cd8e6d7695dc8be52bc5acb04991ebaf3499) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(2a4364c6) SHA1(f2760d7416585123a46ffb47ab9da008532f64bb) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(a34d91d0) SHA1(9616952b1e8b6c06c8bca3c776bdfa3526ef60cf) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(a0be7be4) SHA1(d6f4a1ba22be638fa801923960da8f5f72fc16c0) )
ROM_END

ROM_START( resdnt_2d ) // 040513 custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 SE01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rs_m_040513d.rom", 0x00000, 0x40000, CRC(3481a7b0) SHA1(fb9fa5aea082c80a232d09a938c8b0007e582cc7) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1d", 0x000000, 0x80000, CRC(e13336d0) SHA1(a427a3eeecf29d7760ffb0955719f282b6688f79) )
	ROM_LOAD( "bootleg_2d", 0x100000, 0x80000, CRC(16ed4ed0) SHA1(a9d012fcd53aa912594ba42e677c0061a5daeb89) )
	ROM_LOAD( "bootleg_3d", 0x200000, 0x80000, CRC(bddd68bd) SHA1(97a7899ba0386249a1b40eee32db7ef96734bea6) )
	ROM_LOAD( "bootleg_4d", 0x300000, 0x80000, CRC(78c509f9) SHA1(171ed587d124b90fd68bd747e02322d16f7d08be) )
	ROM_LOAD( "bootleg_5d", 0x080000, 0x80000, CRC(8d4205df) SHA1(669b95bb06d235a9cc977a2e714c2e722ca385f1) )
	ROM_LOAD( "bootleg_6d", 0x180000, 0x80000, CRC(1e06508e) SHA1(df64f3849945f0303e55de7061d3fc3370a27299) )
	ROM_LOAD( "bootleg_7d", 0x280000, 0x80000, CRC(e896b62f) SHA1(fb97ff11b49345fda04d5b1f03ec96fdf4a71419) )
	ROM_LOAD( "bootleg_8d", 0x380000, 0x80000, CRC(a8de4e0a) SHA1(92264e99a7c90967bf71ca440b5cd4df4baeb553) )
ROM_END

ROM_START( resdnt_2e ) // 040513 modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rs_m_040513e.rom", 0x00000, 0x40000, CRC(83fb15e2) SHA1(59b2fbc218802787bc4c7a6324248d1c4bea459b) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1e", 0x000000, 0x80000, CRC(7e9cb8a1) SHA1(5d089b41897dbb90b8209e60c0746196033b8945) )
	ROM_LOAD( "bootleg_2e", 0x100000, 0x80000, CRC(474c2a5d) SHA1(54e317706d1b4328ff006dcace11a112b9533b8f) )
	ROM_LOAD( "bootleg_3e", 0x200000, 0x80000, CRC(8407a51d) SHA1(d790efa0cd4c38c26e95bf10c63a751694840c52) )
	ROM_LOAD( "bootleg_4e", 0x300000, 0x80000, CRC(65f51297) SHA1(799762a7833510a4c513bd3ffb5bd4e541608cca) )
	ROM_LOAD( "resident_m.005", 0x080000, 0x80000, CRC(0cfe7d44) SHA1(9f0e4925e815ff9f79188f18e78c0a7b377daa3f) )
	ROM_LOAD( "resident_m.006", 0x180000, 0x80000, CRC(7437904f) SHA1(630c79cd6a990ce7658a1ffabba5a27efba985a1) )
	ROM_LOAD( "resident_m.007", 0x280000, 0x80000, CRC(6e94728a) SHA1(ab414879cb957d9bc8d653b5e3bb2bbf91139ec0) )
	ROM_LOAD( "resident_m.008", 0x380000, 0x80000, CRC(a9f55043) SHA1(4771df3d45bdc0a21b1c479f45e09ac5bab6c94f) )
ROM_END

ROM_START( resdnt_2f ) // 040513 modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-RS2"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rs_m_040513f.rom", 0x00000, 0x40000, CRC(2f711769) SHA1(07333d5837484107681561a2434c8668ec77a7f4) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1f", 0x000000, 0x80000, CRC(8cefec30) SHA1(3bf400bdff230e2af83c377617e162818a8c70a4) )
	ROM_LOAD( "bootleg_2f", 0x100000, 0x80000, CRC(5318fc74) SHA1(8c6f6f0de426228ca24d552b246f5d0f5feb8611) )
	ROM_LOAD( "bootleg_3f", 0x200000, 0x80000, CRC(5ff2bf17) SHA1(c0e23e02cd4e39fb2a99af64a002474f1bc48131) )
	ROM_LOAD( "bootleg_4f", 0x300000, 0x80000, CRC(9e9a90fa) SHA1(dadacfded904401a1f1f0e31c8218763fd01cff8) )
	ROM_LOAD( "resident_m.005", 0x080000, 0x80000, CRC(0cfe7d44) SHA1(9f0e4925e815ff9f79188f18e78c0a7b377daa3f) )
	ROM_LOAD( "resident_m.006", 0x180000, 0x80000, CRC(7437904f) SHA1(630c79cd6a990ce7658a1ffabba5a27efba985a1) )
	ROM_LOAD( "resident_m.007", 0x280000, 0x80000, CRC(6e94728a) SHA1(ab414879cb957d9bc8d653b5e3bb2bbf91139ec0) )
	ROM_LOAD( "resident_m.008", 0x380000, 0x80000, CRC(a9f55043) SHA1(4771df3d45bdc0a21b1c479f45e09ac5bab6c94f) )
ROM_END

ROM_START( resdnt_2g ) // 040513 custom alteras, modified graphics, many texts changed, changed version text to "LOTOS SE01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_rs_m_040513g.rom", 0x00000, 0x40000, CRC(3c061c32) SHA1(c71376da261f1f69cfa78a243444ea9963f9000e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1g", 0x000000, 0x80000, CRC(f98cda01) SHA1(c8bb11230d4659ae57429ed3ec56dd26af2a6332) )
	ROM_LOAD( "bootleg_2g", 0x100000, 0x80000, CRC(c3ccf6d0) SHA1(adbe8772c731c000309e7e56acdb92823cae7971) )
	ROM_LOAD( "bootleg_3g", 0x200000, 0x80000, CRC(564d2255) SHA1(d589e6c12d62c9c5a316e4cd0512576f5b8ec916) )
	ROM_LOAD( "bootleg_4g", 0x300000, 0x80000, CRC(66b98fc5) SHA1(6010053fd0f4b5b062bbfceb3ce417d6cc618ad1) )
	ROM_LOAD( "bootleg_5d", 0x080000, 0x80000, CRC(8d4205df) SHA1(669b95bb06d235a9cc977a2e714c2e722ca385f1) )
	ROM_LOAD( "bootleg_6d", 0x180000, 0x80000, CRC(1e06508e) SHA1(df64f3849945f0303e55de7061d3fc3370a27299) )
	ROM_LOAD( "bootleg_7d", 0x280000, 0x80000, CRC(e896b62f) SHA1(fb97ff11b49345fda04d5b1f03ec96fdf4a71419) )
	ROM_LOAD( "bootleg_8d", 0x380000, 0x80000, CRC(a8de4e0a) SHA1(92264e99a7c90967bf71ca440b5cd4df4baeb553) )
ROM_END



ROM_START( islanda ) // 050713 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_is_m_050713a.rom", 0x00000, 0x40000, CRC(d3d62cb3) SHA1(2ceb83ac9d59a570435220f06e8317057bb46608) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "island.001", 0x000000, 0x80000, CRC(dbe8cdda) SHA1(4747cf0d85afdef22d3ba9fa5e75b39548725745) )
	ROM_LOAD( "island.002", 0x100000, 0x80000, CRC(64064745) SHA1(91a7bc7204a8f7a7512eeaf4906da20a9f587565) )
	ROM_LOAD( "island.003", 0x200000, 0x80000, CRC(1d993f68) SHA1(b0459d3941d50668f7533909e3f3da91453d3efd) )
	ROM_LOAD( "island.004", 0x300000, 0x80000, CRC(a4739404) SHA1(8f7ffcc13dcb35adfa8060ab1930d07195b6110c) )
	ROM_LOAD( "island.005", 0x080000, 0x80000, CRC(d016eb31) SHA1(a84f18af470f72730b241b9031cd6131c8a03db2) )
	ROM_LOAD( "island.006", 0x180000, 0x80000, CRC(0faaa968) SHA1(0f05546e6e0559e24c6afdde65b3feeb66b6adff) )
	ROM_LOAD( "island.007", 0x280000, 0x80000, CRC(d7277a6c) SHA1(d96a0befc965ad22087381982305d68208978a7e) )
	ROM_LOAD( "island.008", 0x380000, 0x80000, CRC(ac6fba48) SHA1(64dd03d624f16da52bc7fa0702246e91ae39a806) )
ROM_END

ROM_START( islandb ) // 050713 custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 OS01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_is_m_050713b.rom", 0x00000, 0x40000, CRC(535d4283) SHA1(d4823b1487aa9c7f07e84968de563a8c18f3e770) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1a", 0x000000, 0x80000, CRC(95195fdc) SHA1(4ef22f9d6e539555303f042e53c42acf658a6027) )
	ROM_LOAD( "bootleg_2a", 0x100000, 0x80000, CRC(0d6d3104) SHA1(2b9f1ee3854a6e0645d698757e89aebd4831e31e) )
	ROM_LOAD( "bootleg_3a", 0x200000, 0x80000, CRC(981a067b) SHA1(3a75caa39c13b7fdb4ce56a6144c7d715d455a3b) )
	ROM_LOAD( "bootleg_4a", 0x300000, 0x80000, CRC(2e235348) SHA1(148d898271feb03dbef5b1efb65571f813c58be7) )
	ROM_LOAD( "bootleg_5a", 0x080000, 0x80000, CRC(feba83ab) SHA1(0609f2d4b98fb5dcdf80102c32a26f29fd30f50d) )
	ROM_LOAD( "bootleg_6a", 0x180000, 0x80000, CRC(f59eaafa) SHA1(c324ff461b997c79572644b2974db33a156a00e8) )
	ROM_LOAD( "bootleg_7a", 0x280000, 0x80000, CRC(5248d6b8) SHA1(1ef6fbc56c81b641003b16eceffb8d0b3d385546) )
	ROM_LOAD( "bootleg_8a", 0x380000, 0x80000, CRC(47d37fe9) SHA1(1c12cadfe265bd18539af8c94deef674e179eea4) )
ROM_END

ROM_START( islandc ) // 050713 custom alteras, modified graphics, many texts changed, changed version text to "LOTOS OS01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_is_m_050713c.rom", 0x00000, 0x40000, CRC(c879a78e) SHA1(61bf87f50cbf4105330e57e1af7802100286d888) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(eccf5648) SHA1(7b9dc15e8fac9dcc8a14bda52df4d03462a242d0) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(9d85dcfb) SHA1(3752e9e4c1b8485a37e7c68175dc42aa7927c485) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(dd208792) SHA1(ee7cf68b40719e783c70584d7da87875280283dd) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(9d8de4db) SHA1(9935569285f456decc2b851bef07d60a4e440229) )
	ROM_LOAD( "bootleg_5a", 0x080000, 0x80000, CRC(feba83ab) SHA1(0609f2d4b98fb5dcdf80102c32a26f29fd30f50d) )
	ROM_LOAD( "bootleg_6a", 0x180000, 0x80000, CRC(f59eaafa) SHA1(c324ff461b997c79572644b2974db33a156a00e8) )
	ROM_LOAD( "bootleg_7a", 0x280000, 0x80000, CRC(5248d6b8) SHA1(1ef6fbc56c81b641003b16eceffb8d0b3d385546) )
	ROM_LOAD( "bootleg_8a", 0x380000, 0x80000, CRC(47d37fe9) SHA1(1c12cadfe265bd18539af8c94deef674e179eea4) )
ROM_END



ROM_START( island2a ) // 060529 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_is2_m_060529a.rom", 0x00000, 0x40000, CRC(4341d65c) SHA1(e7120c805d7dbf0fee5d18243ddf2cfa19a0d88c) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "island2.001", 0x000000, 0x80000, CRC(f8dd9fe9) SHA1(0cf67fbca107b255011fded6390507d12cbac514) )
	ROM_LOAD( "island2.002", 0x100000, 0x80000, CRC(4f9607c0) SHA1(cd3e7b4a88f46231a115c9d18a26b5e30fea74e4) )
	ROM_LOAD( "island2.003", 0x200000, 0x80000, CRC(bceccdba) SHA1(5cf3b51ccfe317ca57d770bff0204b0ee83d1173) )
	ROM_LOAD( "island2.004", 0x300000, 0x80000, CRC(15fdecc7) SHA1(6882ea10f117c85544df51d9abd67ef52db91d95) )
	ROM_LOAD( "island2.005", 0x080000, 0x80000, CRC(2d4905aa) SHA1(c4a1e4db61e8af6cf0fb70aabe5e3896ab5227ca) )
	ROM_LOAD( "island2.006", 0x180000, 0x80000, CRC(55e285d9) SHA1(ba58963441c65220700cd8057e6afe3f5f8faa4f) )
	ROM_LOAD( "island2.007", 0x280000, 0x80000, CRC(edd72be6) SHA1(fb1e63f59e8565c23ae43630fa572fbc022c878f) )
	ROM_LOAD( "island2.008", 0x380000, 0x80000, CRC(c336d608) SHA1(55391183c6d95ecea81354efa70641350860d1f5) )
ROM_END

ROM_START( island2b ) // 060529 bank F9, changed version text to 070205, skip some start tests
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_is2_m_060529b.rom", 0x00000, 0x40000, CRC(617d9a35) SHA1(96947342216872984e13e9997552f8411b268467) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "island2.001", 0x000000, 0x80000, CRC(f8dd9fe9) SHA1(0cf67fbca107b255011fded6390507d12cbac514) )
	ROM_LOAD( "island2.002", 0x100000, 0x80000, CRC(4f9607c0) SHA1(cd3e7b4a88f46231a115c9d18a26b5e30fea74e4) )
	ROM_LOAD( "island2.003", 0x200000, 0x80000, CRC(bceccdba) SHA1(5cf3b51ccfe317ca57d770bff0204b0ee83d1173) )
	ROM_LOAD( "island2.004", 0x300000, 0x80000, CRC(15fdecc7) SHA1(6882ea10f117c85544df51d9abd67ef52db91d95) )
	ROM_LOAD( "island2.005", 0x080000, 0x80000, CRC(2d4905aa) SHA1(c4a1e4db61e8af6cf0fb70aabe5e3896ab5227ca) )
	ROM_LOAD( "island2.006", 0x180000, 0x80000, CRC(55e285d9) SHA1(ba58963441c65220700cd8057e6afe3f5f8faa4f) )
	ROM_LOAD( "island2.007", 0x280000, 0x80000, CRC(edd72be6) SHA1(fb1e63f59e8565c23ae43630fa572fbc022c878f) )
	ROM_LOAD( "island2.008", 0x380000, 0x80000, CRC(c336d608) SHA1(55391183c6d95ecea81354efa70641350860d1f5) )
ROM_END

ROM_START( island2c ) // 060529 bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_is2_m_060529c.rom", 0x00000, 0x40000, CRC(cbf1ec14) SHA1(ec6dc40deacc1a1e0bbe2778d834fb5eb392ca04) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(4ee179af) SHA1(77e48ff2a45bb4eb8f68b92044791f6f4fad09f5) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(764a7e33) SHA1(7a0e32c2fb86296cd316e23311f95cc3d183c3b8) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(9680c9cb) SHA1(2ca8d02ce8aa168e3561c47ca140397857096811) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(8443f48d) SHA1(568fe79ecef3a120cde4d807a2e63d8e8125e859) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(8aa21c72) SHA1(5f1b95af245c81df699ef346ab790b971ec5b8ab) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(b4b7dc6e) SHA1(8b1b195e55d402e298a45b9b5d1b8bff39f6aecd) )
	ROM_LOAD( "bootleg_7",  0x280000, 0x80000, CRC(edd72be6) SHA1(fb1e63f59e8565c23ae43630fa572fbc022c878f) )
	ROM_LOAD( "bootleg_8",  0x380000, 0x80000, CRC(c336d608) SHA1(55391183c6d95ecea81354efa70641350860d1f5) )
ROM_END

ROM_START( island2_3a ) // 061218 bank F9, modified graphics, changed version text to "VIDEO GAME-1 OS2-01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_is2_m_061218a.rom", 0x00000, 0x40000, CRC(f190c856) SHA1(c3b762c8b2e24a75b91e93a6c058d8a4a86de60e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1a", 0x000000, 0x80000, CRC(ea6c4724) SHA1(f0054dc2e0d7c49e5ca96ce5aaf4c08ddb9d56d9) )
	ROM_LOAD( "bootleg_2a", 0x100000, 0x80000, CRC(f005e13b) SHA1(f16e3af752c35bac6aafa6bc14abbdb80ddb85d3) )
	ROM_LOAD( "bootleg_3a", 0x200000, 0x80000, CRC(df3a9181) SHA1(253a0dc28d027bb7573ae62a0631d7cedb29f344) )
	ROM_LOAD( "bootleg_4a", 0x300000, 0x80000, CRC(5d86af03) SHA1(d79053aa5468e70d102bfedaed3b030c75f32a0d) )
	ROM_LOAD( "bootleg_5a", 0x080000, 0x80000, CRC(8f2a5fa5) SHA1(5c84cf84e6e7141bd315763edc1d7904fb9b1776) )
	ROM_LOAD( "bootleg_6a", 0x180000, 0x80000, CRC(0072562b) SHA1(df6f24149a194d8654fd10bce4969bd9fc9d2165) )
	ROM_LOAD( "bootleg_7a", 0x280000, 0x80000, CRC(a325a291) SHA1(de3d4f1ab803faa08a50e4f5dc5a78cd67f5ec1f) )
	ROM_LOAD( "bootleg_8a", 0x380000, 0x80000, CRC(d6b228ca) SHA1(70c62c38291e7b2bae8f30e1b1488b1d8bc1435c) )
ROM_END

ROM_START( island2_4a ) // 070205 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_is2_070205a.rom", 0x00000, 0x40000, CRC(9bafe560) SHA1(c7238f822d26645207953798c87e67127de1b443) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "island2.001", 0x000000, 0x80000, CRC(f8dd9fe9) SHA1(0cf67fbca107b255011fded6390507d12cbac514) )
	ROM_LOAD( "island2.002", 0x100000, 0x80000, CRC(4f9607c0) SHA1(cd3e7b4a88f46231a115c9d18a26b5e30fea74e4) )
	ROM_LOAD( "island2.003", 0x200000, 0x80000, CRC(bceccdba) SHA1(5cf3b51ccfe317ca57d770bff0204b0ee83d1173) )
	ROM_LOAD( "island2.004", 0x300000, 0x80000, CRC(15fdecc7) SHA1(6882ea10f117c85544df51d9abd67ef52db91d95) )
	ROM_LOAD( "island2.005", 0x080000, 0x80000, CRC(2d4905aa) SHA1(c4a1e4db61e8af6cf0fb70aabe5e3896ab5227ca) )
	ROM_LOAD( "island2.006", 0x180000, 0x80000, CRC(55e285d9) SHA1(ba58963441c65220700cd8057e6afe3f5f8faa4f) )
	ROM_LOAD( "island2.007", 0x280000, 0x80000, CRC(edd72be6) SHA1(fb1e63f59e8565c23ae43630fa572fbc022c878f) )
	ROM_LOAD( "island2.008", 0x380000, 0x80000, CRC(c336d608) SHA1(55391183c6d95ecea81354efa70641350860d1f5) )
ROM_END



ROM_START( pirate2a ) // 061005 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_pr2_m_061005a.rom", 0x00000, 0x40000, CRC(2271eb23) SHA1(6a3e01b59df08a23e5f53ee9b9e473b9f5f5e3b6) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "pirate2.001", 0x000000, 0x80000, CRC(106e7cba) SHA1(289a3ae38b895c83600c920bee0c2dd46e941eac) )
	ROM_LOAD( "pirate2.002", 0x100000, 0x80000, CRC(076a290f) SHA1(2f9bb74e081262e535c8ed9a31589d6a919f5d15) )
	ROM_LOAD( "pirate2.003", 0x200000, 0x80000, CRC(13a91fe7) SHA1(6e127b3827a9271ad19986714747be9367125f62) )
	ROM_LOAD( "pirate2.004", 0x300000, 0x80000, CRC(5ac8c531) SHA1(1da91b9a71a9a8681577342660bfa85e5bbc99bc) )
	ROM_LOAD( "pirate2.005", 0x080000, 0x80000, CRC(98012c74) SHA1(2a5b466353eef3a5cfc9f98eceb7523b00d0204a) )
	ROM_LOAD( "pirate2.006", 0x180000, 0x80000, CRC(366e1465) SHA1(440230d5306c4b424f27839b7fb9c8a5bb922dcc) )
	ROM_LOAD( "pirate2.007", 0x280000, 0x80000, CRC(21fb963e) SHA1(e3f7fb13f326699e34aebcc3ee07016f7cfe6e46) )
	ROM_LOAD( "pirate2.008", 0x380000, 0x80000, CRC(40c59448) SHA1(774af0f376864ec5948904df338bc7493eaed392) )
ROM_END

ROM_START( pirate2b ) // 061005 bank F9, skip raster beam position check
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_pr2_m_061005b.rom", 0x00000, 0x40000, CRC(379f890e) SHA1(0fd05c1bf08fd3e6694f18ffe2ea9741cf465617) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "pirate2.001", 0x000000, 0x80000, CRC(106e7cba) SHA1(289a3ae38b895c83600c920bee0c2dd46e941eac) )
	ROM_LOAD( "pirate2.002", 0x100000, 0x80000, CRC(076a290f) SHA1(2f9bb74e081262e535c8ed9a31589d6a919f5d15) )
	ROM_LOAD( "pirate2.003", 0x200000, 0x80000, CRC(13a91fe7) SHA1(6e127b3827a9271ad19986714747be9367125f62) )
	ROM_LOAD( "pirate2.004", 0x300000, 0x80000, CRC(5ac8c531) SHA1(1da91b9a71a9a8681577342660bfa85e5bbc99bc) )
	ROM_LOAD( "pirate2.005", 0x080000, 0x80000, CRC(98012c74) SHA1(2a5b466353eef3a5cfc9f98eceb7523b00d0204a) )
	ROM_LOAD( "pirate2.006", 0x180000, 0x80000, CRC(366e1465) SHA1(440230d5306c4b424f27839b7fb9c8a5bb922dcc) )
	ROM_LOAD( "pirate2.007", 0x280000, 0x80000, CRC(21fb963e) SHA1(e3f7fb13f326699e34aebcc3ee07016f7cfe6e46) )
	ROM_LOAD( "pirate2.008", 0x380000, 0x80000, CRC(40c59448) SHA1(774af0f376864ec5948904df338bc7493eaed392) )
ROM_END

ROM_START( pirate2c ) // 061005 bank F9, changed version text to 070126
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_pr2_m_061005c.rom", 0x00000, 0x40000, CRC(7c4339df) SHA1(80e2f97629bc963f044c1697c65777a3fd038edc) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "pirate2.001", 0x000000, 0x80000, CRC(106e7cba) SHA1(289a3ae38b895c83600c920bee0c2dd46e941eac) )
	ROM_LOAD( "pirate2.002", 0x100000, 0x80000, CRC(076a290f) SHA1(2f9bb74e081262e535c8ed9a31589d6a919f5d15) )
	ROM_LOAD( "pirate2.003", 0x200000, 0x80000, CRC(13a91fe7) SHA1(6e127b3827a9271ad19986714747be9367125f62) )
	ROM_LOAD( "pirate2.004", 0x300000, 0x80000, CRC(5ac8c531) SHA1(1da91b9a71a9a8681577342660bfa85e5bbc99bc) )
	ROM_LOAD( "pirate2.005", 0x080000, 0x80000, CRC(98012c74) SHA1(2a5b466353eef3a5cfc9f98eceb7523b00d0204a) )
	ROM_LOAD( "pirate2.006", 0x180000, 0x80000, CRC(366e1465) SHA1(440230d5306c4b424f27839b7fb9c8a5bb922dcc) )
	ROM_LOAD( "pirate2.007", 0x280000, 0x80000, CRC(21fb963e) SHA1(e3f7fb13f326699e34aebcc3ee07016f7cfe6e46) )
	ROM_LOAD( "pirate2.008", 0x380000, 0x80000, CRC(40c59448) SHA1(774af0f376864ec5948904df338bc7493eaed392) )
ROM_END

ROM_START( pirate2d ) // 061005 bank F9, changed version text to 070126
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_pr2_m_061005d.rom", 0x00000, 0x40000, CRC(16620bb2) SHA1(bf3a26b06595e51fa37f2f8099a12e906a6ca439) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "pirate2.001", 0x000000, 0x80000, CRC(106e7cba) SHA1(289a3ae38b895c83600c920bee0c2dd46e941eac) )
	ROM_LOAD( "pirate2.002", 0x100000, 0x80000, CRC(076a290f) SHA1(2f9bb74e081262e535c8ed9a31589d6a919f5d15) )
	ROM_LOAD( "pirate2.003", 0x200000, 0x80000, CRC(13a91fe7) SHA1(6e127b3827a9271ad19986714747be9367125f62) )
	ROM_LOAD( "pirate2.004", 0x300000, 0x80000, CRC(5ac8c531) SHA1(1da91b9a71a9a8681577342660bfa85e5bbc99bc) )
	ROM_LOAD( "pirate2.005", 0x080000, 0x80000, CRC(98012c74) SHA1(2a5b466353eef3a5cfc9f98eceb7523b00d0204a) )
	ROM_LOAD( "pirate2.006", 0x180000, 0x80000, CRC(366e1465) SHA1(440230d5306c4b424f27839b7fb9c8a5bb922dcc) )
	ROM_LOAD( "pirate2.007", 0x280000, 0x80000, CRC(21fb963e) SHA1(e3f7fb13f326699e34aebcc3ee07016f7cfe6e46) )
	ROM_LOAD( "pirate2.008", 0x380000, 0x80000, CRC(40c59448) SHA1(774af0f376864ec5948904df338bc7493eaed392) )
ROM_END

ROM_START( pirate2e ) // 061005 bank F9, changed version text to 070126, skip some start tests
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_pr2_m_061005e.rom", 0x00000, 0x40000, CRC(f36edf02) SHA1(25ff61eb80b54298037b4a90d7fc93e8530e0815) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "pirate2.001", 0x000000, 0x80000, CRC(106e7cba) SHA1(289a3ae38b895c83600c920bee0c2dd46e941eac) )
	ROM_LOAD( "pirate2.002", 0x100000, 0x80000, CRC(076a290f) SHA1(2f9bb74e081262e535c8ed9a31589d6a919f5d15) )
	ROM_LOAD( "pirate2.003", 0x200000, 0x80000, CRC(13a91fe7) SHA1(6e127b3827a9271ad19986714747be9367125f62) )
	ROM_LOAD( "pirate2.004", 0x300000, 0x80000, CRC(5ac8c531) SHA1(1da91b9a71a9a8681577342660bfa85e5bbc99bc) )
	ROM_LOAD( "pirate2.005", 0x080000, 0x80000, CRC(98012c74) SHA1(2a5b466353eef3a5cfc9f98eceb7523b00d0204a) )
	ROM_LOAD( "pirate2.006", 0x180000, 0x80000, CRC(366e1465) SHA1(440230d5306c4b424f27839b7fb9c8a5bb922dcc) )
	ROM_LOAD( "pirate2.007", 0x280000, 0x80000, CRC(21fb963e) SHA1(e3f7fb13f326699e34aebcc3ee07016f7cfe6e46) )
	ROM_LOAD( "pirate2.008", 0x380000, 0x80000, CRC(40c59448) SHA1(774af0f376864ec5948904df338bc7493eaed392) )
ROM_END

ROM_START( pirate2f ) // 061005 custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 PR01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_pr2_m_061005f.rom", 0x00000, 0x40000, CRC(99960a9f) SHA1(0facd5d1f297d4433d6abefc61cfa2867826e90c) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1a", 0x000000, 0x80000, CRC(011522da) SHA1(c1ce09208b3144cbc38d4afa922a049043a93df6) )
	ROM_LOAD( "bootleg_2a", 0x100000, 0x80000, CRC(8f8a0ea3) SHA1(5b34b43d6aa843550a7aaff3306a4b68722e9966) )
	ROM_LOAD( "bootleg_3a", 0x200000, 0x80000, CRC(7bbfcda9) SHA1(7ebb1ac0b42f4aabcc4be65c44d01b51fd070ab4) )
	ROM_LOAD( "bootleg_4a", 0x300000, 0x80000, CRC(88b8b15d) SHA1(0f0f1c769eb1475e1f74f56a587efce13c910a4c) )
	ROM_LOAD( "bootleg_5a", 0x080000, 0x80000, CRC(85441649) SHA1(171e58eac7fdd5f8f9d0eb31535a824b19ea7342) )
	ROM_LOAD( "bootleg_6a", 0x180000, 0x80000, CRC(913d55e1) SHA1(030eb39a4e34e7c5791059ac2affcded914096ae) )
	ROM_LOAD( "bootleg_7a", 0x280000, 0x80000, CRC(02fb1604) SHA1(7fd9c6f256f22e437cdd7938b353a4ba9ceb006a) )
	ROM_LOAD( "bootleg_8a", 0x380000, 0x80000, CRC(48643b99) SHA1(1b0399df326835a6b442137d233cc741cf7863de) )
ROM_END

ROM_START( pirate2g ) // 061005 bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_pr2_m_061005g.rom", 0x00000, 0x40000, CRC(60170053) SHA1(4aca8fd8466199467a5c0020cdbb4379ae38cee4) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(76d3bc6f) SHA1(a748f5d1219fe3f332cebbf1dfb8014a15800135) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(e65b7e0a) SHA1(0ce76961b92f7153f3886965d909c522d32313dd) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(5d8270ac) SHA1(883435b08cea05dc1531e7871a6b3046e8365b55) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(ac0f1bb5) SHA1(d391b5be974a94aecba7fd55a4e2017b5b60ad06) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(48019c90) SHA1(f00effeb84d2c0f46faa40ea519b1d41efbc8e38) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(19155fa8) SHA1(81458b32432caa3effba778971840e330be33585) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(7fb69540) SHA1(595ede785962c8af84eee7ff47c4b3bc91c5537a) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(f8a97474) SHA1(b8c5be0f7bc3f4a08823a1bf8e2026afe11d052c) )
ROM_END

ROM_START( pirate2h ) // 061005 custom alteras, modified graphics, many texts changed, changed version text to "LOTOS PR01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_pr2_m_061005h.rom", 0x00000, 0x40000, CRC(6dadb485) SHA1(9b61ad0f117dc84af8b7b3523275e5cb48c521e5) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1d", 0x000000, 0x80000, CRC(d830ed6d) SHA1(4cdde127912d250201665534e6371dcfc500dc10) )
	ROM_LOAD( "bootleg_2d", 0x100000, 0x80000, CRC(b54df2b2) SHA1(1a08c4b345423741067a095fb21a6a91abb87dbc) )
	ROM_LOAD( "bootleg_3d", 0x200000, 0x80000, CRC(b87811cf) SHA1(3e9d8d06d7ac5ae5404d1002e4a2123c4f2c0328) )
	ROM_LOAD( "bootleg_4d", 0x300000, 0x80000, CRC(5aa8e647) SHA1(fa5ae55e1fd81068369f2bf9fc711fc78fa3fa24) )
	ROM_LOAD( "bootleg_5a", 0x080000, 0x80000, CRC(85441649) SHA1(171e58eac7fdd5f8f9d0eb31535a824b19ea7342) )
	ROM_LOAD( "bootleg_6a", 0x180000, 0x80000, CRC(913d55e1) SHA1(030eb39a4e34e7c5791059ac2affcded914096ae) )
	ROM_LOAD( "bootleg_7a", 0x280000, 0x80000, CRC(02fb1604) SHA1(7fd9c6f256f22e437cdd7938b353a4ba9ceb006a) )
	ROM_LOAD( "bootleg_8a", 0x380000, 0x80000, CRC(48643b99) SHA1(1b0399df326835a6b442137d233cc741cf7863de) )
ROM_END

ROM_START( pirate2_2a ) // 070126 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_pr2_070126a.rom", 0x00000, 0x40000, CRC(7131239b) SHA1(8210defd0987951a0dfe5b3ca6fcee580257475e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "pirate2.001", 0x000000, 0x80000, CRC(106e7cba) SHA1(289a3ae38b895c83600c920bee0c2dd46e941eac) )
	ROM_LOAD( "pirate2.002", 0x100000, 0x80000, CRC(076a290f) SHA1(2f9bb74e081262e535c8ed9a31589d6a919f5d15) )
	ROM_LOAD( "pirate2.003", 0x200000, 0x80000, CRC(13a91fe7) SHA1(6e127b3827a9271ad19986714747be9367125f62) )
	ROM_LOAD( "pirate2.004", 0x300000, 0x80000, CRC(5ac8c531) SHA1(1da91b9a71a9a8681577342660bfa85e5bbc99bc) )
	ROM_LOAD( "pirate2.005", 0x080000, 0x80000, CRC(98012c74) SHA1(2a5b466353eef3a5cfc9f98eceb7523b00d0204a) )
	ROM_LOAD( "pirate2.006", 0x180000, 0x80000, CRC(366e1465) SHA1(440230d5306c4b424f27839b7fb9c8a5bb922dcc) )
	ROM_LOAD( "pirate2.007", 0x280000, 0x80000, CRC(21fb963e) SHA1(e3f7fb13f326699e34aebcc3ee07016f7cfe6e46) )
	ROM_LOAD( "pirate2.008", 0x380000, 0x80000, CRC(40c59448) SHA1(774af0f376864ec5948904df338bc7493eaed392) )
ROM_END



ROM_START( keksa ) // 060328 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060328a.rom", 0x00000, 0x40000, CRC(7b387386) SHA1(d6bfc3b0d1f74723902d96dbcb69865cb5274cd0) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "keks_m.001", 0x000000, 0x80000, CRC(f4c20f66) SHA1(bed42ef01dfaa9d5d6ebb703e44ce7c11b8a373c) )
	ROM_LOAD( "keks_m.002", 0x100000, 0x80000, CRC(b7ec3fac) SHA1(c3c62690487a6056415c46888bde8254efca836f) )
	ROM_LOAD( "keks_m.003", 0x200000, 0x80000, CRC(5b6e8568) SHA1(003297e9cd080d91fe6751286eabd3a2f37ceb76) )
	ROM_LOAD( "keks_m.004", 0x300000, 0x80000, CRC(9dc32736) SHA1(7b2091ae802431d1c959b859a58e0076d32abef0) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keksb ) // 060328 backdoor 1,1 1,3 1,5 1,7  3,3  3,4
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060328b.rom", 0x00000, 0x40000, CRC(661c7ee9) SHA1(f07902c4a3ba5fce5bc7fe666d90deb852e40b4c) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "keks_m.001", 0x000000, 0x80000, CRC(f4c20f66) SHA1(bed42ef01dfaa9d5d6ebb703e44ce7c11b8a373c) )
	ROM_LOAD( "keks_m.002", 0x100000, 0x80000, CRC(b7ec3fac) SHA1(c3c62690487a6056415c46888bde8254efca836f) )
	ROM_LOAD( "keks_m.003", 0x200000, 0x80000, CRC(5b6e8568) SHA1(003297e9cd080d91fe6751286eabd3a2f37ceb76) )
	ROM_LOAD( "keks_m.004", 0x300000, 0x80000, CRC(9dc32736) SHA1(7b2091ae802431d1c959b859a58e0076d32abef0) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keksc ) // 060328 bank F9, changed version text to 070119
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060328c.rom", 0x00000, 0x40000, CRC(fc70d26e) SHA1(0430ad20b797412967c2e82779eb7f0e9170f77c) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "keks_m.001", 0x000000, 0x80000, CRC(f4c20f66) SHA1(bed42ef01dfaa9d5d6ebb703e44ce7c11b8a373c) )
	ROM_LOAD( "keks_m.002", 0x100000, 0x80000, CRC(b7ec3fac) SHA1(c3c62690487a6056415c46888bde8254efca836f) )
	ROM_LOAD( "keks_m.003", 0x200000, 0x80000, CRC(5b6e8568) SHA1(003297e9cd080d91fe6751286eabd3a2f37ceb76) )
	ROM_LOAD( "keks_m.004", 0x300000, 0x80000, CRC(9dc32736) SHA1(7b2091ae802431d1c959b859a58e0076d32abef0) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keks_2a ) // 060403 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060403a.rom", 0x00000, 0x40000, CRC(bd749f63) SHA1(dc3ba624b186370896d3ecf5968a82a17aa019d0) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "keks_m.001", 0x000000, 0x80000, CRC(f4c20f66) SHA1(bed42ef01dfaa9d5d6ebb703e44ce7c11b8a373c) )
	ROM_LOAD( "keks_m.002", 0x100000, 0x80000, CRC(b7ec3fac) SHA1(c3c62690487a6056415c46888bde8254efca836f) )
	ROM_LOAD( "keks_m.003", 0x200000, 0x80000, CRC(5b6e8568) SHA1(003297e9cd080d91fe6751286eabd3a2f37ceb76) )
	ROM_LOAD( "keks_m.004", 0x300000, 0x80000, CRC(9dc32736) SHA1(7b2091ae802431d1c959b859a58e0076d32abef0) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keks_2b ) // 060403 bank F9, changed version text to 070119
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060403b.rom", 0x00000, 0x40000, CRC(d089de90) SHA1(9208f411cd97fef89b52019f9655e7d006f91303) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "keks_m.001", 0x000000, 0x80000, CRC(f4c20f66) SHA1(bed42ef01dfaa9d5d6ebb703e44ce7c11b8a373c) )
	ROM_LOAD( "keks_m.002", 0x100000, 0x80000, CRC(b7ec3fac) SHA1(c3c62690487a6056415c46888bde8254efca836f) )
	ROM_LOAD( "keks_m.003", 0x200000, 0x80000, CRC(5b6e8568) SHA1(003297e9cd080d91fe6751286eabd3a2f37ceb76) )
	ROM_LOAD( "keks_m.004", 0x300000, 0x80000, CRC(9dc32736) SHA1(7b2091ae802431d1c959b859a58e0076d32abef0) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keks_2c ) // 060403 custom alteras, modified graphics, bank F9, changed version text to "VIDEO GAME-1 KS01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060403c.rom", 0x00000, 0x40000, CRC(ba7f429a) SHA1(6ad7a345b292f2081e49081374c74afc7430bd14) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(2844b89e) SHA1(75b59e6272ef42438a3734e45f131f449ed0aa46) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(ae8f47ad) SHA1(16504436c9291ffea7915547e5ad9dfbd53ebd5d) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(57ccb876) SHA1(e1f41cf09ac2a535b5dbc05364268b3bd913e8f5) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(05344f35) SHA1(ffc60a7581db82878fe625d590c01c7afdf8260c) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(45f9b0bd) SHA1(4b0f59461182753fd47f7fcc6a126faf53dd26b1) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(5ffbfa73) SHA1(0036bdba0ed6f907b8925b55adc33e96913297a3) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(ce3f6152) SHA1(6e142e0d448d7a2328b6f6bf014b8e6de6f18407) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(6e2e6a3d) SHA1(d4efad82b580c7cd53fca705a24aef51edede8ed) )
ROM_END

ROM_START( keks_2d ) // 060403 modified graphics, bank F9, changed version text to "VIDEO GAME-1 KS01" (keks_2c, decoded gfx)
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060403c.rom", 0x00000, 0x40000, CRC(ba7f429a) SHA1(6ad7a345b292f2081e49081374c74afc7430bd14) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1c", 0x000000, 0x80000, CRC(04ad2c2f) SHA1(ea0819dad0971629056ba0e6cb599415f1d34ad6) )
	ROM_LOAD( "bootleg_2c", 0x100000, 0x80000, CRC(8d0b747d) SHA1(6c40f3abe8bd20d6405829879fbe4718d85d4ba0) )
	ROM_LOAD( "bootleg_3c", 0x200000, 0x80000, CRC(10a43238) SHA1(9404eefad2c62730a771b2521e893bbc78c4ca71) )
	ROM_LOAD( "bootleg_4c", 0x300000, 0x80000, CRC(878b2553) SHA1(1b31c1b67496099c0b955865b63881e4a558c320) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keks_2e ) // 060403 bank F9, payout percentage 60%
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060403d.rom", 0x00000, 0x40000, CRC(358e5b48) SHA1(6a91d9ee515401ebe7a8dd7d95f43c4df1c13677) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "keks_m.001", 0x000000, 0x80000, CRC(f4c20f66) SHA1(bed42ef01dfaa9d5d6ebb703e44ce7c11b8a373c) )
	ROM_LOAD( "keks_m.002", 0x100000, 0x80000, CRC(b7ec3fac) SHA1(c3c62690487a6056415c46888bde8254efca836f) )
	ROM_LOAD( "keks_m.003", 0x200000, 0x80000, CRC(5b6e8568) SHA1(003297e9cd080d91fe6751286eabd3a2f37ceb76) )
	ROM_LOAD( "keks_m.004", 0x300000, 0x80000, CRC(9dc32736) SHA1(7b2091ae802431d1c959b859a58e0076d32abef0) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keks_2f ) // 060403 bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060403e.rom", 0x00000, 0x40000, CRC(80882337) SHA1(af942e27247b0d492799013a623080c02e7213cb) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1d", 0x000000, 0x80000, CRC(82c5f8e9) SHA1(0fbc1b314e15f93c62c3cd79ece453086c506878) )
	ROM_LOAD( "bootleg_2d", 0x100000, 0x80000, CRC(a174f52b) SHA1(45c225762f5885e33eb6848aeca1ae9e2ad918ed) )
	ROM_LOAD( "bootleg_3d", 0x200000, 0x80000, CRC(9486084d) SHA1(780cd6d7b85e9480c7c2a3f707228d37678fb684) )
	ROM_LOAD( "bootleg_4d", 0x300000, 0x80000, CRC(aa552b3c) SHA1(7b051b2cece5a5e84020466827576ca8272f4f96) )
	ROM_LOAD( "bootleg_5d", 0x080000, 0x80000, CRC(53fa2420) SHA1(2faf775c5bc1ff8683d4a70b236c77d6295167e1) )
	ROM_LOAD( "bootleg_6d", 0x180000, 0x80000, CRC(baa7cce5) SHA1(1aacc85498f026358085caa854c2caaad3de5abf) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keks_2g ) // 060403 custom alteras, modified graphics, bank F9, many texts changed, changed version text to "LOTOS KS01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_m_060403f.rom", 0x00000, 0x40000, CRC(9fd765ab) SHA1(ff061fe3d5d547c25d4ca830ec1f94025ab4e7f5) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1f", 0x000000, 0x80000, CRC(6c9bf198) SHA1(40155c3574f9968e272c6f1af90d663d2a686511) )
	ROM_LOAD( "bootleg_2f", 0x100000, 0x80000, CRC(daf71ad6) SHA1(2107d5eb617ff844eaeaabe3744fb8fad403390c) )
	ROM_LOAD( "bootleg_3f", 0x200000, 0x80000, CRC(ef9dfc94) SHA1(9b6d97dd6dc20caa11da4ced956ab5474e92e6d9) )
	ROM_LOAD( "bootleg_4f", 0x300000, 0x80000, CRC(6e88ac0e) SHA1(6dbc64794bb5afce3ab5e1dfe272b933bf66722d) )
	ROM_LOAD( "bootleg_5f", 0x080000, 0x80000, CRC(a79f6d46) SHA1(345bdf089529d90da861426fffc72bd2efb062f3) )
	ROM_LOAD( "bootleg_6f", 0x180000, 0x80000, CRC(7ecd7ed4) SHA1(e6320741f0def3915c91307cb4e3c277b347dee6) )
	ROM_LOAD( "bootleg_7f", 0x280000, 0x80000, CRC(e952996a) SHA1(3382158ced985c46fad56fb74766a4ed0e35b6c6) )
	ROM_LOAD( "bootleg_8f", 0x380000, 0x80000, CRC(4fefa4eb) SHA1(6a2b633e278a9b2a05dde2c6c37f15fc6b3fa7dc) )
ROM_END

ROM_START( keks_3a ) // 070119 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_070119a.rom", 0x00000, 0x40000, CRC(9325fea5) SHA1(1d396e63d51a81eae75df1ae2646cacac7f49f66) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "keks.001", 0x000000, 0x80000, CRC(fc399595) SHA1(037afd4a613cd58d4a28627b9e395d48c3fa866a) )
	ROM_LOAD( "keks.002", 0x100000, 0x80000, CRC(474b36e7) SHA1(e1e62acd4a706b2654fc1249850806b612fc1419) )
	ROM_LOAD( "keks.003", 0x200000, 0x80000, CRC(7f885e3d) SHA1(09bb4690e86ed4a29eef75ee4e5753ce40a164dd) )
	ROM_LOAD( "keks.004", 0x300000, 0x80000, CRC(a0fc654b) SHA1(3354bdb7aa372816a766b0d36408543de7d3482f) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END

ROM_START( keks_3b ) // 070119 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_ks_070119b.rom", 0x00000, 0x40000, CRC(13fcf022) SHA1(885e5097e5cb5805f2172adc41f2f20d109b2d10) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "keks.001", 0x000000, 0x80000, CRC(fc399595) SHA1(037afd4a613cd58d4a28627b9e395d48c3fa866a) )
	ROM_LOAD( "keks.002", 0x100000, 0x80000, CRC(474b36e7) SHA1(e1e62acd4a706b2654fc1249850806b612fc1419) )
	ROM_LOAD( "keks.003", 0x200000, 0x80000, CRC(7f885e3d) SHA1(09bb4690e86ed4a29eef75ee4e5753ce40a164dd) )
	ROM_LOAD( "keks.004", 0x300000, 0x80000, CRC(a0fc654b) SHA1(3354bdb7aa372816a766b0d36408543de7d3482f) )
	ROM_LOAD( "keks_m.005", 0x080000, 0x80000, CRC(c5b09267) SHA1(7fd0988e63752fdbb31fde60b4726cfd63149622) )
	ROM_LOAD( "keks_m.006", 0x180000, 0x80000, CRC(583da5fd) SHA1(645228db20cdaacb53bfc68731fd1a66a6a8cf56) )
	ROM_LOAD( "keks_m.007", 0x280000, 0x80000, CRC(311c166a) SHA1(5f0ad8d755a6141964d818b98b3f156cbda8fb0d) )
	ROM_LOAD( "keks_m.008", 0x380000, 0x80000, CRC(f69b0831) SHA1(75392349ef02a39cf883206938e2c615445065fc) )
ROM_END



ROM_START( gnomea ) // 070906 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_070906a.rom", 0x00000, 0x40000, CRC(5e7f1d53) SHA1(0548db91e8b1cd2f4f292095b4aad3b59cc25fb8) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gnome.001", 0x000000, 0x80000, CRC(6ed866d7) SHA1(68d75d24d98e6d533cb26ceac0a680203cb26069) )
	ROM_LOAD( "gnome.002", 0x100000, 0x80000, CRC(f6e5e6f0) SHA1(9751e8df87f14a252595547d24b8dd865ee4f08d) )
	ROM_LOAD( "gnome.003", 0x200000, 0x80000, CRC(f8beb972) SHA1(3afbca8ce7e69d2dadae05f69205a6fd9036cf6a) )
	ROM_LOAD( "gnome.004", 0x300000, 0x80000, CRC(83357c38) SHA1(45cd31c4f02f9d7b1888701c2146d1e7229b6cb5) )
	ROM_LOAD( "gnome.005", 0x080000, 0x80000, CRC(687ad3e3) SHA1(23941a4f40c45029b9a43451f78b04c03c3cd7da) )
	ROM_LOAD( "gnome.006", 0x180000, 0x80000, CRC(7ef2b88a) SHA1(7e7de60fc6791731d7cfd6a50e2bc5af1bf5e4b2) )
	ROM_LOAD( "gnome.007", 0x280000, 0x80000, CRC(71976bdf) SHA1(c44dbfa75a0f12893b3177907fc93b3d5e8ad390) )
	ROM_LOAD( "gnome.008", 0x380000, 0x80000, CRC(c86a1586) SHA1(e622bca8dc618ca8edc1a7daa9c8286383caebef) )
ROM_END

ROM_START( gnomeb ) // 070906 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_070906b.rom", 0x00000, 0x40000, CRC(0baeb2f7) SHA1(799576a1cf49681d81967678643c17dc0e09be78) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gnome.001", 0x000000, 0x80000, CRC(6ed866d7) SHA1(68d75d24d98e6d533cb26ceac0a680203cb26069) )
	ROM_LOAD( "gnome.002", 0x100000, 0x80000, CRC(f6e5e6f0) SHA1(9751e8df87f14a252595547d24b8dd865ee4f08d) )
	ROM_LOAD( "gnome.003", 0x200000, 0x80000, CRC(f8beb972) SHA1(3afbca8ce7e69d2dadae05f69205a6fd9036cf6a) )
	ROM_LOAD( "gnome.004", 0x300000, 0x80000, CRC(83357c38) SHA1(45cd31c4f02f9d7b1888701c2146d1e7229b6cb5) )
	ROM_LOAD( "gnome.005", 0x080000, 0x80000, CRC(687ad3e3) SHA1(23941a4f40c45029b9a43451f78b04c03c3cd7da) )
	ROM_LOAD( "gnome.006", 0x180000, 0x80000, CRC(7ef2b88a) SHA1(7e7de60fc6791731d7cfd6a50e2bc5af1bf5e4b2) )
	ROM_LOAD( "gnome.007", 0x280000, 0x80000, CRC(71976bdf) SHA1(c44dbfa75a0f12893b3177907fc93b3d5e8ad390) )
	ROM_LOAD( "gnome.008", 0x380000, 0x80000, CRC(c86a1586) SHA1(e622bca8dc618ca8edc1a7daa9c8286383caebef) )
ROM_END

ROM_START( gnomec ) // 070906 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_070906c.rom", 0x00000, 0x40000, CRC(37b4a5e2) SHA1(baffba15c5ba0f139b74e3dedd1d68e14fc7b370) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gnome.001", 0x000000, 0x80000, CRC(6ed866d7) SHA1(68d75d24d98e6d533cb26ceac0a680203cb26069) )
	ROM_LOAD( "gnome.002", 0x100000, 0x80000, CRC(f6e5e6f0) SHA1(9751e8df87f14a252595547d24b8dd865ee4f08d) )
	ROM_LOAD( "gnome.003", 0x200000, 0x80000, CRC(f8beb972) SHA1(3afbca8ce7e69d2dadae05f69205a6fd9036cf6a) )
	ROM_LOAD( "gnome.004", 0x300000, 0x80000, CRC(83357c38) SHA1(45cd31c4f02f9d7b1888701c2146d1e7229b6cb5) )
	ROM_LOAD( "gnome.005", 0x080000, 0x80000, CRC(687ad3e3) SHA1(23941a4f40c45029b9a43451f78b04c03c3cd7da) )
	ROM_LOAD( "gnome.006", 0x180000, 0x80000, CRC(7ef2b88a) SHA1(7e7de60fc6791731d7cfd6a50e2bc5af1bf5e4b2) )
	ROM_LOAD( "gnome.007", 0x280000, 0x80000, CRC(71976bdf) SHA1(c44dbfa75a0f12893b3177907fc93b3d5e8ad390) )
	ROM_LOAD( "gnome.008", 0x380000, 0x80000, CRC(c86a1586) SHA1(e622bca8dc618ca8edc1a7daa9c8286383caebef) )
ROM_END

ROM_START( gnomed ) // 070906 custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 GN01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_070906d.rom", 0x00000, 0x40000, CRC(12682805) SHA1(58a5c365e56ce3d679fe88f2930471ac63549d85) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1a", 0x000000, 0x80000, CRC(55a8b1ba) SHA1(0b6c906435e260178292014a9e160a364bbcd6ad) )
	ROM_LOAD( "bootleg_2a", 0x100000, 0x80000, CRC(2ee1cc82) SHA1(081bbbd7c4589ae62efd1ef06b9f593b90492ee7) )
	ROM_LOAD( "bootleg_3a", 0x200000, 0x80000, CRC(7011d0b8) SHA1(24987121c6a0e712ad355eee2d7be5cf9e24e908) )
	ROM_LOAD( "bootleg_4a", 0x300000, 0x80000, CRC(e6bb6057) SHA1(d214f7b4b235cf1c80cb1131f69244c25287938b) )
	ROM_LOAD( "bootleg_5a", 0x080000, 0x80000, CRC(668c21a8) SHA1(1fccab7363350f2a26af40e05b4e13369bcb5a49) )
	ROM_LOAD( "bootleg_6a", 0x180000, 0x80000, CRC(95c2d96f) SHA1(8ac7283b65940fc620e126ad7e8563d96378dfd4) )
	ROM_LOAD( "bootleg_7a", 0x280000, 0x80000, CRC(476d6b00) SHA1(106c4afdb4d4d5afe9ec8395ef26050c814f053a) )
	ROM_LOAD( "bootleg_8a", 0x380000, 0x80000, CRC(658becb6) SHA1(7280ce53796326a48da8e461de8f78ea78a2f8c2) )
ROM_END

ROM_START( gnomee ) // 070906 custom alteras, modified graphics, bank F9, many texts changed, changed version text to "LOTOS GN01"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_070906e.rom", 0x00000, 0x40000, CRC(33fbae84) SHA1(4b8fde65042de38b24c77665b76782db4efccf0f) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(414b537c) SHA1(65cc2a3bca8792387a83323f51b7345ee1be1125) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(3452bf73) SHA1(33e45d38bc7243cb4dbda483138fe529a693fad9) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(1e44b605) SHA1(c7e1018fb36cb1f1db006d06dbde94baeb609c46) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(19a7ff3e) SHA1(7a58c0db9615b0a4e6cb8480d56b9ecf2a4cec1d) )
	ROM_LOAD( "bootleg_5a", 0x080000, 0x80000, CRC(668c21a8) SHA1(1fccab7363350f2a26af40e05b4e13369bcb5a49) )
	ROM_LOAD( "bootleg_6a", 0x180000, 0x80000, CRC(95c2d96f) SHA1(8ac7283b65940fc620e126ad7e8563d96378dfd4) )
	ROM_LOAD( "bootleg_7a", 0x280000, 0x80000, CRC(476d6b00) SHA1(106c4afdb4d4d5afe9ec8395ef26050c814f053a) )
	ROM_LOAD( "bootleg_8a", 0x380000, 0x80000, CRC(658becb6) SHA1(7280ce53796326a48da8e461de8f78ea78a2f8c2) )
ROM_END

ROM_START( gnome_2a ) // 071115 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_071115a.rom", 0x00000, 0x40000, CRC(97e74228) SHA1(6b4d108c6c59ec335c14f0e5ab990cc8782ed236) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gnome.001", 0x000000, 0x80000, CRC(6ed866d7) SHA1(68d75d24d98e6d533cb26ceac0a680203cb26069) )
	ROM_LOAD( "gnome.002", 0x100000, 0x80000, CRC(f6e5e6f0) SHA1(9751e8df87f14a252595547d24b8dd865ee4f08d) )
	ROM_LOAD( "gnome.003", 0x200000, 0x80000, CRC(f8beb972) SHA1(3afbca8ce7e69d2dadae05f69205a6fd9036cf6a) )
	ROM_LOAD( "gnome.004", 0x300000, 0x80000, CRC(83357c38) SHA1(45cd31c4f02f9d7b1888701c2146d1e7229b6cb5) )
	ROM_LOAD( "gnome.005", 0x080000, 0x80000, CRC(687ad3e3) SHA1(23941a4f40c45029b9a43451f78b04c03c3cd7da) )
	ROM_LOAD( "gnome.006", 0x180000, 0x80000, CRC(7ef2b88a) SHA1(7e7de60fc6791731d7cfd6a50e2bc5af1bf5e4b2) )
	ROM_LOAD( "gnome.007", 0x280000, 0x80000, CRC(71976bdf) SHA1(c44dbfa75a0f12893b3177907fc93b3d5e8ad390) )
	ROM_LOAD( "gnome.008", 0x380000, 0x80000, CRC(c86a1586) SHA1(e622bca8dc618ca8edc1a7daa9c8286383caebef) )
ROM_END


ROM_START( gnome_3a ) // 080303 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_m_080303a.rom", 0x00000, 0x40000, CRC(86e3f741) SHA1(e5d9f897daa6cf58a7fb426c42810d71bf839294) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gnome.001", 0x000000, 0x80000, CRC(6ed866d7) SHA1(68d75d24d98e6d533cb26ceac0a680203cb26069) )
	ROM_LOAD( "gnome.002", 0x100000, 0x80000, CRC(f6e5e6f0) SHA1(9751e8df87f14a252595547d24b8dd865ee4f08d) )
	ROM_LOAD( "gnome.003", 0x200000, 0x80000, CRC(f8beb972) SHA1(3afbca8ce7e69d2dadae05f69205a6fd9036cf6a) )
	ROM_LOAD( "gnome.004", 0x300000, 0x80000, CRC(83357c38) SHA1(45cd31c4f02f9d7b1888701c2146d1e7229b6cb5) )
	ROM_LOAD( "gnome.005", 0x080000, 0x80000, CRC(687ad3e3) SHA1(23941a4f40c45029b9a43451f78b04c03c3cd7da) )
	ROM_LOAD( "gnome.006", 0x180000, 0x80000, CRC(7ef2b88a) SHA1(7e7de60fc6791731d7cfd6a50e2bc5af1bf5e4b2) )
	ROM_LOAD( "gnome.007", 0x280000, 0x80000, CRC(71976bdf) SHA1(c44dbfa75a0f12893b3177907fc93b3d5e8ad390) )
	ROM_LOAD( "gnome.008", 0x380000, 0x80000, CRC(c86a1586) SHA1(e622bca8dc618ca8edc1a7daa9c8286383caebef) )
ROM_END

ROM_START( gnome_3b ) // 080303 bank F9, payout percentage 45%
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_m_080303b.rom", 0x00000, 0x40000, CRC(c01d2d38) SHA1(7743d46e2ebce24d88961e52b67a928b29a3aa0d) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gnome.001", 0x000000, 0x80000, CRC(6ed866d7) SHA1(68d75d24d98e6d533cb26ceac0a680203cb26069) )
	ROM_LOAD( "gnome.002", 0x100000, 0x80000, CRC(f6e5e6f0) SHA1(9751e8df87f14a252595547d24b8dd865ee4f08d) )
	ROM_LOAD( "gnome.003", 0x200000, 0x80000, CRC(f8beb972) SHA1(3afbca8ce7e69d2dadae05f69205a6fd9036cf6a) )
	ROM_LOAD( "gnome.004", 0x300000, 0x80000, CRC(83357c38) SHA1(45cd31c4f02f9d7b1888701c2146d1e7229b6cb5) )
	ROM_LOAD( "gnome.005", 0x080000, 0x80000, CRC(687ad3e3) SHA1(23941a4f40c45029b9a43451f78b04c03c3cd7da) )
	ROM_LOAD( "gnome.006", 0x180000, 0x80000, CRC(7ef2b88a) SHA1(7e7de60fc6791731d7cfd6a50e2bc5af1bf5e4b2) )
	ROM_LOAD( "gnome.007", 0x280000, 0x80000, CRC(71976bdf) SHA1(c44dbfa75a0f12893b3177907fc93b3d5e8ad390) )
	ROM_LOAD( "gnome.008", 0x380000, 0x80000, CRC(c86a1586) SHA1(e622bca8dc618ca8edc1a7daa9c8286383caebef) )
ROM_END

ROM_START( gnome_3c ) // 080303 bank F9, payout percentage 60%
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_gn_m_080303c.rom", 0x00000, 0x40000, CRC(a7479ddb) SHA1(d64be81ff20548373e08ccae58d00f5cb2a335da) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gnome.001", 0x000000, 0x80000, CRC(6ed866d7) SHA1(68d75d24d98e6d533cb26ceac0a680203cb26069) )
	ROM_LOAD( "gnome.002", 0x100000, 0x80000, CRC(f6e5e6f0) SHA1(9751e8df87f14a252595547d24b8dd865ee4f08d) )
	ROM_LOAD( "gnome.003", 0x200000, 0x80000, CRC(f8beb972) SHA1(3afbca8ce7e69d2dadae05f69205a6fd9036cf6a) )
	ROM_LOAD( "gnome.004", 0x300000, 0x80000, CRC(83357c38) SHA1(45cd31c4f02f9d7b1888701c2146d1e7229b6cb5) )
	ROM_LOAD( "gnome.005", 0x080000, 0x80000, CRC(687ad3e3) SHA1(23941a4f40c45029b9a43451f78b04c03c3cd7da) )
	ROM_LOAD( "gnome.006", 0x180000, 0x80000, CRC(7ef2b88a) SHA1(7e7de60fc6791731d7cfd6a50e2bc5af1bf5e4b2) )
	ROM_LOAD( "gnome.007", 0x280000, 0x80000, CRC(71976bdf) SHA1(c44dbfa75a0f12893b3177907fc93b3d5e8ad390) )
	ROM_LOAD( "gnome.008", 0x380000, 0x80000, CRC(c86a1586) SHA1(e622bca8dc618ca8edc1a7daa9c8286383caebef) )
ROM_END

ROM_START( fcockt2a ) // 080707 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc2_080707a.rom", 0x00000, 0x40000, CRC(c4840b3c) SHA1(518079207bb3d13e492003b382d28a8cf3647f63) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail2.001",  0x000000, 0x80000, CRC(d1b9416d) SHA1(4d3cb0a6dbcf02bbd20d5c43df358882b2ad794d) )
	ROM_LOAD( "fruitcocktail2.002",  0x100000, 0x80000, CRC(69236be1) SHA1(9a2e6c8f279714f79a606c0b118b6bf1d8442cda) )
	ROM_LOAD( "fruitcocktail2.003",  0x200000, 0x80000, CRC(29aade8c) SHA1(bef42f8a25c90e3a1cccd13872a10eb8b2b2e276) )
	ROM_LOAD( "fruitcocktail2.004",  0x300000, 0x80000, CRC(4b9646e7) SHA1(26548a018401f4e07383eb145f8f0847677f3272) )
	ROM_LOAD( "fruitcocktail2_old.005", 0x080000, 0x80000, CRC(6b9e6b43) SHA1(c7fb17e91ec62b22da42f110d68b4f37e39de3ce) )
	ROM_LOAD( "fruitcocktail2_old.006", 0x180000, 0x80000, CRC(2c9f712e) SHA1(c3118154eafca74b66b3325a2e07c85f86f3544d) )
	ROM_LOAD( "fruitcocktail2_old.007", 0x280000, 0x80000, CRC(85ba9a86) SHA1(aa9b6170135e9e420509e8f7c1702c9896bc5d8e) )
	ROM_LOAD( "fruitcocktail2_old.008", 0x380000, 0x80000, CRC(a27c49a2) SHA1(7c9ee0e01f76ca3ab6716579f5dde7036050970b) )
ROM_END

ROM_START( fcockt2_4a ) // 081105 bank F9
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc2_m_081105a.rom", 0x00000, 0x40000,  CRC(411ad706) SHA1(4d29a6b1927241efdcdb3224b9ebe0eb6af92533) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail2.001", 0x000000, 0x80000, CRC(d1b9416d) SHA1(4d3cb0a6dbcf02bbd20d5c43df358882b2ad794d) )
	ROM_LOAD( "fruitcocktail2.002", 0x100000, 0x80000, CRC(69236be1) SHA1(9a2e6c8f279714f79a606c0b118b6bf1d8442cda) )
	ROM_LOAD( "fruitcocktail2.003", 0x200000, 0x80000, CRC(29aade8c) SHA1(bef42f8a25c90e3a1cccd13872a10eb8b2b2e276) )
	ROM_LOAD( "fruitcocktail2.004", 0x300000, 0x80000, CRC(4b9646e7) SHA1(26548a018401f4e07383eb145f8f0847677f3272) )
	ROM_LOAD( "fruitcocktail2.005", 0x080000, 0x80000, CRC(3fc13d72) SHA1(ba0727138ef03d576d190cbce04b9eb0bba88a9a) )
	ROM_LOAD( "fruitcocktail2.006", 0x180000, 0x80000, CRC(8fc75fd7) SHA1(3b7cd8a3e04ca9d4494b37c801e21d1293f094e8) )
	ROM_LOAD( "fruitcocktail2.007", 0x280000, 0x80000, CRC(d37fcc0f) SHA1(57c2ea5dc747f16e2233305f2c73cb4b632aae2c) )
	ROM_LOAD( "fruitcocktail2.008", 0x380000, 0x80000, CRC(e3a9442c) SHA1(cbaba182e858b0f158756118e5da873e3ddfc0b9) )
ROM_END

ROM_START( fcockt2_4b ) // 081105 bank F9, no credit limit, "MaxVin" signature
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc2_m_081105_cm.rom", 0x00000, 0x40000,  CRC(f0c702b9) SHA1(89121ddd325aac567ebd443eb443c4db0549236c) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail2.001", 0x000000, 0x80000, CRC(d1b9416d) SHA1(4d3cb0a6dbcf02bbd20d5c43df358882b2ad794d) )
	ROM_LOAD( "fruitcocktail2.002", 0x100000, 0x80000, CRC(69236be1) SHA1(9a2e6c8f279714f79a606c0b118b6bf1d8442cda) )
	ROM_LOAD( "fruitcocktail2.003", 0x200000, 0x80000, CRC(29aade8c) SHA1(bef42f8a25c90e3a1cccd13872a10eb8b2b2e276) )
	ROM_LOAD( "fruitcocktail2.004", 0x300000, 0x80000, CRC(4b9646e7) SHA1(26548a018401f4e07383eb145f8f0847677f3272) )
	ROM_LOAD( "fruitcocktail2.005", 0x080000, 0x80000, CRC(3fc13d72) SHA1(ba0727138ef03d576d190cbce04b9eb0bba88a9a) )
	ROM_LOAD( "fruitcocktail2.006", 0x180000, 0x80000, CRC(8fc75fd7) SHA1(3b7cd8a3e04ca9d4494b37c801e21d1293f094e8) )
	ROM_LOAD( "fruitcocktail2.007", 0x280000, 0x80000, CRC(d37fcc0f) SHA1(57c2ea5dc747f16e2233305f2c73cb4b632aae2c) )
	ROM_LOAD( "fruitcocktail2.008", 0x380000, 0x80000, CRC(e3a9442c) SHA1(cbaba182e858b0f158756118e5da873e3ddfc0b9) )
ROM_END

ROM_START( fcockt2_4c ) // 081105 custom alteras, modified graphics, bank F9, many texts changed, changed version text to "VIDEO GAME-1 FR02"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc2_m_081105b.rom", 0x00000, 0x40000,  CRC(b627f7d6) SHA1(0bcdb1cd808055d0a9e965f2c6ff522ac3fe6949) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1b", 0x000000, 0x80000, CRC(68b62355) SHA1(387da330fddaa685cb7aca32bf861e44d5cdbe58) )
	ROM_LOAD( "bootleg_2b", 0x100000, 0x80000, CRC(d36e8704) SHA1(80a84a24b3a43a0110a00ed3b01c7156218ccbf3) )
	ROM_LOAD( "bootleg_3b", 0x200000, 0x80000, CRC(57d87c5b) SHA1(6bc28e5c34a7e76d0e601b970ab922e5ad28ae85) )
	ROM_LOAD( "bootleg_4b", 0x300000, 0x80000, CRC(3de30bab) SHA1(c4c4c549ad980f562238054cd0e3ef6b0977f7b7) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(b47d65aa) SHA1(71c642ea7887ea35c6d7d985502082d2424c035a) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(aa982f4e) SHA1(edf75d4ddd60532ac0ca2e17e3fd9004aa768973) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(b9b1ead7) SHA1(111e505fce8fe75043ff7cc5dc68e81ce4f98d06) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(8e49d1c1) SHA1(4735503335cf499895d1b2d9a405be50f21a2d68) )
ROM_END

ROM_START( fcockt2_4d ) // 081105 bank F9, no credit limit, "MaxVin" signature, payout percentage 70%
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc2_m_081105c.rom", 0x00000, 0x40000,  CRC(b6d88fcc) SHA1(337a59accfbc229be05aff84ba70fc135fde09de) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "fruitcocktail2.001", 0x000000, 0x80000, CRC(d1b9416d) SHA1(4d3cb0a6dbcf02bbd20d5c43df358882b2ad794d) )
	ROM_LOAD( "fruitcocktail2.002", 0x100000, 0x80000, CRC(69236be1) SHA1(9a2e6c8f279714f79a606c0b118b6bf1d8442cda) )
	ROM_LOAD( "fruitcocktail2.003", 0x200000, 0x80000, CRC(29aade8c) SHA1(bef42f8a25c90e3a1cccd13872a10eb8b2b2e276) )
	ROM_LOAD( "fruitcocktail2.004", 0x300000, 0x80000, CRC(4b9646e7) SHA1(26548a018401f4e07383eb145f8f0847677f3272) )
	ROM_LOAD( "fruitcocktail2.005", 0x080000, 0x80000, CRC(3fc13d72) SHA1(ba0727138ef03d576d190cbce04b9eb0bba88a9a) )
	ROM_LOAD( "fruitcocktail2.006", 0x180000, 0x80000, CRC(8fc75fd7) SHA1(3b7cd8a3e04ca9d4494b37c801e21d1293f094e8) )
	ROM_LOAD( "fruitcocktail2.007", 0x280000, 0x80000, CRC(d37fcc0f) SHA1(57c2ea5dc747f16e2233305f2c73cb4b632aae2c) )
	ROM_LOAD( "fruitcocktail2.008", 0x380000, 0x80000, CRC(e3a9442c) SHA1(cbaba182e858b0f158756118e5da873e3ddfc0b9) )
ROM_END

ROM_START( fcockt2_4e ) // 081105 bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc2_m_081105d.rom", 0x00000, 0x40000,  CRC(37b1a159) SHA1(52c8e825ebb85a59803a6d14c766892e56a72c9e) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1c", 0x000000, 0x80000, CRC(9fe02e19) SHA1(dec48a0990d2341ae86a3a4acd5972844b93fccb) )
	ROM_LOAD( "bootleg_2c", 0x100000, 0x80000, CRC(734bd656) SHA1(e44124fd60675cfc8319bca5bbd90f99d73f4896) )
	ROM_LOAD( "bootleg_3c", 0x200000, 0x80000, CRC(36ff74dc) SHA1(7df93e4fa1c240adf33ef910efb8397abfa1a106) )
	ROM_LOAD( "bootleg_4c", 0x300000, 0x80000, CRC(b88356c9) SHA1(fbdcfc0d30f2a9b1364863d7a06b32f94113e865) )
	ROM_LOAD( "bootleg_5c", 0x080000, 0x80000, CRC(14053302) SHA1(adf0a1493074d16797e8d19dc98f2d5d2cb26e43) )
	ROM_LOAD( "bootleg_6c", 0x180000, 0x80000, CRC(21a91970) SHA1(6f4832c743469171cd0efe4554f243c4bed21e88) )
	ROM_LOAD( "bootleg_7c", 0x280000, 0x80000, CRC(3a8a7340) SHA1(9408791686926dd13216352ffcb04ad9f8954b3e) )
	ROM_LOAD( "bootleg_8c", 0x380000, 0x80000, CRC(fbf48ebd) SHA1(04917e613d9a3f0caeab71c78b9f77c4b5b84988) )
ROM_END

ROM_START( fcockt2_4f ) // 081105 custom alteras, modified graphics, bank F9, many texts changed, changed version text to "LOTOS FR02"
	ROM_REGION( 0x40000, "maincpu", 0 ) // z80 code, banked
	ROM_LOAD( "bootleg_fc2_m_081105f.rom", 0x00000, 0x40000,  CRC(b4fa0663) SHA1(aee3cb980fc7d9fbba1a4e9fe80e6be8df58bb3b) ) /* Not officially listed on Igrosoft's web site hash page */

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bootleg_1e", 0x000000, 0x80000, CRC(3c72a5e2) SHA1(15e07d64a0d633ccdadea9eb55556e6aa239c0e5) )
	ROM_LOAD( "bootleg_2e", 0x100000, 0x80000, CRC(017a61f7) SHA1(023cec2be1cfe885c562415d18fc32edf3c61346) )
	ROM_LOAD( "bootleg_3e", 0x200000, 0x80000, CRC(8ea5532f) SHA1(498991cfb7a92c3a6342e824a310c7edc5577cff) )
	ROM_LOAD( "bootleg_4e", 0x300000, 0x80000, CRC(c879025f) SHA1(93f77787ded2a4353320dcb9d4d1b1f5dd7a3b13) )
	ROM_LOAD( "bootleg_5b", 0x080000, 0x80000, CRC(b47d65aa) SHA1(71c642ea7887ea35c6d7d985502082d2424c035a) )
	ROM_LOAD( "bootleg_6b", 0x180000, 0x80000, CRC(aa982f4e) SHA1(edf75d4ddd60532ac0ca2e17e3fd9004aa768973) )
	ROM_LOAD( "bootleg_7b", 0x280000, 0x80000, CRC(b9b1ead7) SHA1(111e505fce8fe75043ff7cc5dc68e81ce4f98d06) )
	ROM_LOAD( "bootleg_8b", 0x380000, 0x80000, CRC(8e49d1c1) SHA1(4735503335cf499895d1b2d9a405be50f21a2d68) )
ROM_END


GAME( 2002, mfish_3a,    mfish_parent,    multfish, multfish,  0,             ROT0,  "bootleg", "Multi Fish (bootleg, 021124, banking address hack)", GAME_SUPPORTS_SAVE ) // bank F9
GAME( 2002, mfish_12a,   mfish_parent,    multfish, multfish,  0,             ROT0,  "bootleg", "Multi Fish (bootleg, 040308, banking address hack)", GAME_SUPPORTS_SAVE ) // bank F9

GAME( 2003, crzmon_7a,   crzmon_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Crazy Monkey (bootleg, 031110, backdoor set 1)", GAME_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2003, crzmon_7b,   crzmon_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Crazy Monkey (bootleg, 031110, backdoor set 2)", GAME_SUPPORTS_SAVE ) // backdoor 1,5 5,5 1,7 3,2  3,3  3,4
GAME( 2003, crzmon_8a,   crzmon_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Crazy Monkey (bootleg, 050120, backdoor)", GAME_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2003, crzmon_8b,   crzmon_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Crazy Monkey (bootleg, 050120, changed version text)", GAME_SUPPORTS_SAVE ) // changed version text to 070315
GAME( 2003, crzmon_8c,   crzmon_parent,   multfish, multfish,  customl,       ROT0,  "bootleg", "Crazy Monkey (bootleg, 050120, VIDEO GAME-1 CM01)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, changed version text to "VIDEO GAME-1 CM01"
GAME( 2003, crzmon_8d,   crzmon_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Crazy Monkey (bootleg, 050120, LOTTOGAME (I))", GAME_SUPPORTS_SAVE ) // modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
GAME( 2003, crzmon_8e,   crzmon_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Crazy Monkey (bootleg, 050120, LOTO PROGRAM V-CM2)", GAME_SUPPORTS_SAVE ) // modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-CM2"
GAME( 2003, crzmon_8f,   crzmon_parent,   multfish, multfish,  customl,       ROT0,  "bootleg", "Crazy Monkey (bootleg, 050120, LOTOS CM01)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "LOTOS CM01"
GAME( 2003, crzmon_9a,   crzmon_parent,   multfish, multfish,  customl,       ROT0,  "bootleg", "Crazy Monkey (bootleg, 070315, VIDEO GAME-1 O01 set 1)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, changed version text to "VIDEO GAME-1 O01"
GAME( 2003, crzmon_9b,   crzmon_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Crazy Monkey (bootleg, 070315, VIDEO GAME-1 O01 set 2)", GAME_SUPPORTS_SAVE ) // modified graphics, changed version text to "VIDEO GAME-1 O01" (crzmon_9a, decoded gfx)
GAME( 2003, crzmon_9c,   crzmon_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Crazy Monkey (bootleg, 070315, payout percentage 70)", GAME_SUPPORTS_SAVE ) // payout percentage 70%

GAME( 2003, fcockt_6a,   fcockt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 040216, banking address hack)", GAME_SUPPORTS_SAVE ) // bank F8
GAME( 2003, fcockt_6b,   fcockt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 040216, backdoor)", GAME_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2003, fcockt_6c,   fcockt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 040216, LotoRossy+)", GAME_SUPPORTS_SAVE ) // modified graphics, some code changes, description says "for Lat-02 terminals", older set
GAME( 2003, fcockt_6d,   fcockt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 040216, VIDEO GAME-1 FR01)", GAME_SUPPORTS_SAVE ) // modified graphics, some code changes, changed version text to "VIDEO GAME-1 FR01", description says "for Lat-02 terminals", newer set
GAME( 2003, fcockt_7a,   fcockt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 050118, backdoor)", GAME_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2003, fcockt_7b,   fcockt_parent,   multfish, multfish,  customl,       ROT0,  "bootleg", "Fruit Cocktail (bootleg, 050118, VIDEO GAME-1 FR01)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 FR01"
GAME( 2003, fcockt_7c,   fcockt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 050118, payout percentage 40)", GAME_SUPPORTS_SAVE ) // payout percentage 40%
GAME( 2003, fcockt_7d,   fcockt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 050118, payout percentage 60)", GAME_SUPPORTS_SAVE ) // payout percentage 60%
GAME( 2003, fcockt_7e,   fcockt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 050118, payout percentage 70)", GAME_SUPPORTS_SAVE ) // payout percentage 70%
GAME( 2003, fcockt_7f,   fcockt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 050118, changed version text)", GAME_SUPPORTS_SAVE ) // changed version text to 070305
GAME( 2003, fcockt_7g,   fcockt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 050118, LOTO PROGRAM V-FC2)", GAME_SUPPORTS_SAVE ) // modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-FC2"
GAME( 2003, fcockt_7h,   fcockt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 050118, LOTOS FR01)", GAME_SUPPORTS_SAVE ) // modified graphics, many texts changed, changed version text to "LOTOS FR01"
GAME( 2003, fcockt_8a,   fcockt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 060111, LOTO COCKTAIL V01-0001)", GAME_SUPPORTS_SAVE ) // modified graphics, many texts changed, changed version text to "LOTO COCKTAIL V01-0001"
GAME( 2003, fcockt_8b,   fcockt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Fruit Cocktail (bootleg, 060111, LOTTOGAME (I))", GAME_SUPPORTS_SAVE ) // modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"

GAME( 2003, lhaunt_4a,   lhaunt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Lucky Haunter (bootleg, 031111, backdoor)", GAME_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2003, lhaunt_5a,   lhaunt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Lucky Haunter (bootleg, 040216, backdoor)", GAME_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2003, lhaunt_6a,   lhaunt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Lucky Haunter (bootleg, 040825, backdoor)", GAME_SUPPORTS_SAVE ) // backdoor 1,5 9,1 5,1 1,5  3,3  3,4
GAME( 2003, lhaunt_6b,   lhaunt_parent,   multfish, multfish,  customl,       ROT0,  "bootleg", "Lucky Haunter (bootleg, 040825, VIDEO GAME-1 PB01)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 PB01"
GAME( 2003, lhaunt_6c,   lhaunt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Lucky Haunter (bootleg, 040825, changed version text)", GAME_SUPPORTS_SAVE ) // changed version text to 070604
GAME( 2003, lhaunt_6d,   lhaunt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Lucky Haunter (bootleg, 040825, LOTTOGAME (I))", GAME_SUPPORTS_SAVE ) // modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
GAME( 2003, lhaunt_6e,   lhaunt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Lucky Haunter (bootleg, 040825, LOTO PROGRAM V-LH2)", GAME_SUPPORTS_SAVE ) // modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-LH2"
GAME( 2003, lhaunt_6f,   lhaunt_parent,   multfish, multfish,  customl,       ROT0,  "bootleg", "Lucky Haunter (bootleg, 040825, LOTOS PB01)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "LOTOS PB01"

GAME( 2004, garage_4a,   garage_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Garage (bootleg, 040219, backdoor)", GAME_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2004, garage_4b,   garage_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Garage (bootleg, 040219, changed version text)", GAME_SUPPORTS_SAVE ) // changed version text to 070329
GAME( 2004, garage_4c,   garage_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Garage (bootleg, 040219, LOTO PROGRAM V-GG2)", GAME_SUPPORTS_SAVE ) // modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-GG2"
GAME( 2004, garage_5a,   garage_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Garage (bootleg, 050311, backdoor)", GAME_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2004, garage_5b,   garage_parent,   multfish, multfish,  customl,       ROT0,  "bootleg", "Garage (bootleg, 050311, VIDEO GAME-1 GA01)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, changed version text to "VIDEO GAME-1 GA01"
GAME( 2004, garage_5c,   garage_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Garage (bootleg, 050311, payout percentage 70)", GAME_SUPPORTS_SAVE ) // payout percentage 70%
GAME( 2004, garage_5d,   garage_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Garage (bootleg, 050311, LOTTOGAME (I))", GAME_SUPPORTS_SAVE ) // modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
GAME( 2004, garage_5e,   garage_parent,   multfish, multfish,  customl,       ROT0,  "bootleg", "Garage (bootleg, 050311, LOTOS GA01)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "LOTOS GA01"

GAME( 2004, rclimb_3a,   rclimb_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Rock Climber (bootleg, 040827, backdoor)", GAME_SUPPORTS_SAVE ) // backdoor 1,5 9,1 5,1 1,5  3,3  3,4
GAME( 2004, rclimb_3b,   rclimb_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Rock Climber (bootleg, 040827, new service menu)", GAME_SUPPORTS_SAVE ) // new service menu
GAME( 2004, rclimb_3c,   rclimb_parent,   multfish, multfish,  customl,       ROT0,  "bootleg", "Rock Climber (bootleg, 040827, VIDEO GAME-1 SK01)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 SK01"
GAME( 2004, rclimb_3d,   rclimb_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Rock Climber (bootleg, 040827, LOTTOGAME (I))", GAME_SUPPORTS_SAVE ) // modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
GAME( 2004, rclimb_3e,   rclimb_parent,   multfish, multfish,  customl,       ROT0,  "bootleg", "Rock Climber (bootleg, 040827, LOTOS SK01)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "LOTOS SK01"

GAME( 2004, sweetla,     sweetl_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Sweet Life (bootleg, 041220, backdoor)", GAME_SUPPORTS_SAVE ) // backdoor 1,5 9,1 5,3 1,5  3,3  3,4
GAME( 2004, sweetlb,     sweetl_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Sweet Life (bootleg, 041220, banking address hack, changed version text)", GAME_SUPPORTS_SAVE ) // bank F9, changed version text to 070412

GAME( 2004, resdnt_2a,   resdnt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Resident (bootleg, 040513, backdoor)", GAME_SUPPORTS_SAVE ) // backdoor 1,5 9,1 5,1 1,5  3,3  3,4
GAME( 2004, resdnt_2b,   resdnt_parent,   multfish, multfish,  customl,       ROT0,  "bootleg", "Resident (bootleg, 040513, VIDEO GAME-1 SE01 set 1)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, changed version text to "VIDEO GAME-1 SE01"
GAME( 2004, resdnt_2c,   resdnt_parent,   multfish, multfish,  customl,       ROT0,  "bootleg", "Resident (bootleg, 040513, VIDEO GAME-1 SE01 set 2)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, changed version text to "VIDEO GAME-1 SE01"
GAME( 2004, resdnt_2d,   resdnt_parent,   multfish, multfish,  customl,       ROT0,  "bootleg", "Resident (bootleg, 040513, VIDEO GAME-1 SE01 set 3)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 SE01"
GAME( 2004, resdnt_2e,   resdnt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Resident (bootleg, 040513, LOTTOGAME (I))", GAME_SUPPORTS_SAVE ) // modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
GAME( 2004, resdnt_2f,   resdnt_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Resident (bootleg, 040513, LOTO PROGRAM V-RS2)", GAME_SUPPORTS_SAVE ) // modified graphics, many texts changed, changed version text to "LOTO PROGRAM V-RS2"
GAME( 2004, resdnt_2g,   resdnt_parent,   multfish, multfish,  customl,       ROT0,  "bootleg", "Resident (bootleg, 040513, LOTOS SE01)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "LOTOS SE01"

GAME( 2005, islanda,     island_parent,   multfish, multfish,  0,             ROT0,  "bootleg", "Island (bootleg, 050713, backdoor)", GAME_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2005, islandb,     island_parent,   multfish, multfish,  customl,       ROT0,  "bootleg", "Island (bootleg, 050713, VIDEO GAME-1 OS01)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 OS01"
GAME( 2005, islandc,     island_parent,   multfish, multfish,  customl,       ROT0,  "bootleg", "Island (bootleg, 050713, LOTOS OS01)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "LOTOS OS01"

GAME( 2006, island2a,    island2_parent,  island2a, multfish,  0,             ROT0,  "bootleg", "Island 2 (bootleg, 060529, banking address hack)", GAME_SUPPORTS_SAVE ) // bank F9 (not standart, game not work)
GAME( 2006, island2b,    island2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Island 2 (bootleg, 060529, banking address hack, changed version text)", GAME_SUPPORTS_SAVE ) // bank F9, changed version text to 070205, skip some start tests
GAME( 2006, island2c,    island2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Island 2 (bootleg, 060529, LOTTOGAME (I))", GAME_SUPPORTS_SAVE ) // bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
GAME( 2006, island2_3a,  island2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Island 2 (bootleg, 061218, VIDEO GAME-1 OS2-01)", GAME_SUPPORTS_SAVE ) // bank F9, modified graphics, changed version text to "VIDEO GAME-1 OS2-01"
GAME( 2006, island2_4a,  island2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Island 2 (bootleg, 070205, banking address hack)", GAME_SUPPORTS_SAVE ) // bank F9

GAME( 2006, pirate2a,    pirate2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Pirate 2 (bootleg, 061005, banking address hack set 1)", GAME_SUPPORTS_SAVE ) // bank F9
GAME( 2006, pirate2b,    pirate2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Pirate 2 (bootleg, 061005, banking address hack set 2)", GAME_SUPPORTS_SAVE ) // bank F9, skip raster beam position check
GAME( 2006, pirate2c,    pirate2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Pirate 2 (bootleg, 061005, banking address hack, changed version text set 1)", GAME_SUPPORTS_SAVE ) // bank F9, changed version text to 070126
GAME( 2006, pirate2d,    pirate2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Pirate 2 (bootleg, 061005, banking address hack, changed version text set 2)", GAME_SUPPORTS_SAVE ) // bank F9, changed version text to 070126
GAME( 2006, pirate2e,    pirate2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Pirate 2 (bootleg, 061005, banking address hack, changed version text set 3)", GAME_SUPPORTS_SAVE ) // bank F9, changed version text to 070126, skip some start tests
GAME( 2006, pirate2f,    pirate2_parent,  multfish, multfish,  customl,       ROT0,  "bootleg", "Pirate 2 (bootleg, 061005, VIDEO GAME-1 PR01)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 PR01"
GAME( 2006, pirate2g,    pirate2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Pirate 2 (bootleg, 061005, LOTTOGAME (I))", GAME_SUPPORTS_SAVE ) // bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
GAME( 2006, pirate2h,    pirate2_parent,  multfish, multfish,  customl,       ROT0,  "bootleg", "Pirate 2 (bootleg, 061005, LOTOS PR01)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "LOTOS PR01"
GAME( 2006, pirate2_2a,  pirate2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Pirate 2 (bootleg, 070126, banking address hack)", GAME_SUPPORTS_SAVE ) // bank F9

GAME( 2006, keksa,       keks_parent,     multfish, multfish,  0,             ROT0,  "bootleg", "Keks (bootleg, 060328, banking address hack)", GAME_SUPPORTS_SAVE ) // bank F9
GAME( 2006, keksb,       keks_parent,     multfish, multfish,  0,             ROT0,  "bootleg", "Keks (bootleg, 060328, backdoor)", GAME_SUPPORTS_SAVE ) // backdoor 1,1 1,3 1,5 1,7  3,3  3,4
GAME( 2006, keksc,       keks_parent,     multfish, multfish,  0,             ROT0,  "bootleg", "Keks (bootleg, 060328, banking address hack, changed version text)", GAME_SUPPORTS_SAVE ) // bank F9, changed version text to 070119
GAME( 2006, keks_2a,     keks_parent,     multfish, multfish,  0,             ROT0,  "bootleg", "Keks (bootleg, 060403, banking address hack)", GAME_SUPPORTS_SAVE ) // bank F9
GAME( 2006, keks_2b,     keks_parent,     multfish, multfish,  0,             ROT0,  "bootleg", "Keks (bootleg, 060403, banking address hack, changed version text)", GAME_SUPPORTS_SAVE ) // bank F9, changed version text to 070119
GAME( 2006, keks_2c,     keks_parent,     multfish, multfish,  customl,       ROT0,  "bootleg", "Keks (bootleg, 060403, VIDEO GAME-1 KS01 set 1)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, bank F9, changed version text to "VIDEO GAME-1 KS01"
GAME( 2006, keks_2d,     keks_parent,     multfish, multfish,  0,             ROT0,  "bootleg", "Keks (bootleg, 060403, VIDEO GAME-1 KS01 set 2)", GAME_SUPPORTS_SAVE ) // modified graphics, bank F9, changed version text to "VIDEO GAME-1 KS01" (keks_2c, decoded gfx)
GAME( 2006, keks_2e,     keks_parent,     multfish, multfish,  0,             ROT0,  "bootleg", "Keks (bootleg, 060403, banking address hack, payout percentage 60)", GAME_SUPPORTS_SAVE ) // bank F9, payout percentage 60%
GAME( 2006, keks_2f,     keks_parent,     multfish, multfish,  0,             ROT0,  "bootleg", "Keks (bootleg, 060403, LOTTOGAME (I))", GAME_SUPPORTS_SAVE ) // bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
GAME( 2006, keks_2g,     keks_parent,     multfish, multfish,  customl,       ROT0,  "bootleg", "Keks (bootleg, 060403, LOTOS KS01)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, bank F9, many texts changed, changed version text to "LOTOS KS01"
GAME( 2006, keks_3a,     keks_parent,     multfish, multfish,  0,             ROT0,  "bootleg", "Keks (bootleg, 070119, banking address hack set 1)", GAME_SUPPORTS_SAVE ) // bank F9
GAME( 2006, keks_3b,     keks_parent,     multfish, multfish,  0,             ROT0,  "bootleg", "Keks (bootleg, 070119, banking address hack set 2)", GAME_SUPPORTS_SAVE ) // bank F9

GAME( 2007, gnomea,      gnome_parent,    multfish, multfish,  0,             ROT0, "bootleg",  "Gnome (bootleg, 070906, banking address hack set 1)", GAME_SUPPORTS_SAVE ) // bank F9
GAME( 2007, gnomeb,      gnome_parent,    multfish, multfish,  0,             ROT0, "bootleg",  "Gnome (bootleg, 070906, banking address hack set 2)", GAME_SUPPORTS_SAVE ) // bank F9
GAME( 2007, gnomec,      gnome_parent,    multfish, multfish,  0,             ROT0, "bootleg",  "Gnome (bootleg, 070906, banking address hack set 3)", GAME_SUPPORTS_SAVE ) // bank F9
GAME( 2007, gnomed,      gnome_parent,    multfish, multfish,  customl,       ROT0, "bootleg",  "Gnome (bootleg, 070906, VIDEO GAME-1 GN01)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, many texts changed, changed version text to "VIDEO GAME-1 GN01"
GAME( 2007, gnomee,      gnome_parent,    multfish, multfish,  customl,       ROT0, "bootleg",  "Gnome (bootleg, 070906, LOTOS GN01)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, bank F9, many texts changed, changed version text to "LOTOS GN01"
GAME( 2007, gnome_2a,    gnome_parent,    multfish, multfish,  0,             ROT0, "bootleg",  "Gnome (bootleg, 071115, banking address hack)", GAME_SUPPORTS_SAVE ) // bank F9
GAME( 2007, gnome_3a,    gnome_parent,    multfish, multfish,  0,             ROT0, "bootleg",  "Gnome (bootleg, 080303, banking address hack)", GAME_SUPPORTS_SAVE ) // bank F9
GAME( 2007, gnome_3b,    gnome_parent,    multfish, multfish,  0,             ROT0, "bootleg",  "Gnome (bootleg, 080303, banking address hack, payout percentage 45)", GAME_SUPPORTS_SAVE ) // bank F9 payout percentage 45%
GAME( 2007, gnome_3c,    gnome_parent,    multfish, multfish,  0,             ROT0, "bootleg",  "Gnome (bootleg, 080303, banking address hack, payout percentage 60)", GAME_SUPPORTS_SAVE ) // bank F9 payout percentage 60%

GAME( 2007, sweetl2_2a,  sweetl2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Sweet Life 2 (bootleg, 080320, banking address hack set 1)", GAME_SUPPORTS_SAVE ) // bank F9
GAME( 2007, sweetl2_2b,  sweetl2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Sweet Life 2 (bootleg, 080320, banking address hack set 2)", GAME_SUPPORTS_SAVE ) // bank F9, some fixes
GAME( 2007, sweetl2_2c,  sweetl2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Sweet Life 2 (bootleg, 080320, VIDEO GAME-1 MD01)", GAME_SUPPORTS_SAVE ) // modified graphics, bank F9, changed version text to "VIDEO GAME-1 MD01"
GAME( 2007, sweetl2_2d,  sweetl2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Sweet Life 2 (bootleg, 080320, LOTTOGAME (I))", GAME_SUPPORTS_SAVE ) // bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"

GAME( 2008, fcockt2a,    fcockt2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Fruit Cocktail 2 (bootleg, 080707, banking address hack)", GAME_SUPPORTS_SAVE ) // bank F9
GAME( 2008, fcockt2_4a,  fcockt2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Fruit Cocktail 2 (bootleg, 081105, banking address hack)", GAME_SUPPORTS_SAVE ) // bank F9
GAME( 2008, fcockt2_4b,  fcockt2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Fruit Cocktail 2 (bootleg, 081105, banking address hack, no credit limit)", GAME_SUPPORTS_SAVE ) // bank F9, no credit limit, "MaxVin" signature
GAME( 2008, fcockt2_4c,  fcockt2_parent,  multfish, multfish,  customl,       ROT0,  "bootleg", "Fruit Cocktail 2 (bootleg, 081105, VIDEO GAME-1 FR02)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, bank F9, many texts changed, changed version text to "VIDEO GAME-1 FR02"
GAME( 2008, fcockt2_4d,  fcockt2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Fruit Cocktail 2 (bootleg, 081105, banking address hack, payout percentage 70)", GAME_SUPPORTS_SAVE ) // bank F9, no credit limit, "MaxVin" signature, payout percentage 70%
GAME( 2008, fcockt2_4e,  fcockt2_parent,  multfish, multfish,  0,             ROT0,  "bootleg", "Fruit Cocktail 2 (bootleg, 081105, LOTTOGAME (I))", GAME_SUPPORTS_SAVE ) // bank F9, modified graphics, changed version text to "MDS_is_the_best_ LOTTOGAME (I)"
GAME( 2008, fcockt2_4f,  fcockt2_parent,  multfish, multfish,  customl,       ROT0,  "bootleg", "Fruit Cocktail 2 (bootleg, 081105, LOTOS FR02)", GAME_SUPPORTS_SAVE ) // custom alteras, modified graphics, bank F9, many texts changed, changed version text to "LOTOS FR02"


/* 0x000000 - 0x03ffff Crazy Monkey V03-1110
   0x040000 - 0x07ffff Garage V05-0311
   0x080000 - 0x0bffff Lucky Haunter V04-0825
   0x0c0000 - 0x0fffff Island 2  IS2-060529
   0x100000 - 0x13ffff Fruit Cocktail V05-0118
   0x140000 - 0x17ffff Resident  V04-0513
   0x180000 - 0x1bffff Rock Climber RC040827
   0x1c0000 - 0x1fffff xxxx MENU xxxx  ( Lucky Haunter V04-0825 service mode )
   0x200000 - 0x23ffff Pirate PR060210
   0x240000 - 0x27ffff Sweet Life SL041220
   0x280000 - 0x2bffff Keks KS060403
   0x2c0000 - 0x2fffff xxxx MENU xxxx  ( Lucky Haunter V04-0825 service mode )
   0x300000 - 0x33ffff NOTHING
   0x340000 - 0x37ffff NOTHING
   0x380000 - 0x3bffff NOTHING
   0x3c0000 - 0x3fffff NOTHING
*/
ROM_START( igromult )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "10games.320.bin", 0x0000, 0x400000, CRC(94e5b2e1) SHA1(62544a5b3de02b5f323478dbae19ef2409af2529) )

	ROM_REGION( 0x4000000, "gfx", 0 ) /* There should be 64MB worth of FLASH ROM for the gfx, the menu GFX are unique at least (not in any Igrosoft set) */
	ROM_LOAD( "10games.gfx", 0x000000, 0x4000000, NO_DUMP )
ROM_END

/* 0x000000 - 0x03ffff Crazy Monkey V03-1110
   0x040000 - 0x07ffff Garage V05-0311
   0x080000 - 0x0bffff Lucky Haunter V04-0825
   0x0c0000 - 0x0fffff Island 2  IS2-060529
   0x100000 - 0x13ffff Fruit Cocktail V05-0118
   0x140000 - 0x17ffff Resident  V04-0513
   0x180000 - 0x1bffff Rock Climber RC040827
   0x1c0000 - 0x1fffff Pirate PR060210
   0x200000 - 0x23ffff Sweet Life SL041220
   0x240000 - 0x27ffff Keks KS060403
   0x280000 - 0x2bffff Gnome M GNM-080303
   0x2c0000 - 0x2fffff Multifish 4Game V04-0308
   0x300000 - 0x33ffff Pirate 2 PR2-061005
   0x340000 - 0x37ffff F Cocktail 2 FC2M-081105
   0x380000 - 0x3bffff Sweet Life 2 - SL2M-080320
   0x3c0000 - 0x3fffff  xxxx MENU xxxx  ( Lucky Haunter V04-0825 service mode )
*/

ROM_START( igromula )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "15games.320.bin", 0x0000, 0x400000, CRC(1cfd520d) SHA1(7316b883a13ff98fc7c3ff05aa085f1fb8e0bdcd) )

	ROM_REGION( 0x4000000, "gfx", 0 ) /* There should be 64MB worth of FLASH ROM for the gfx, the menu GFX are unique at least (not in any Igrosoft set) */
	ROM_LOAD( "15games.gfx", 0x000000, 0x4000000, NO_DUMP )
ROM_END

GAME( 2003, igromult,   0,		  multfish, multfish,  0,             ROT0,  "bootleg", "Igrosoft Multigame Bootleg (10 Games)", GAME_NOT_WORKING ) // no GFX roms
GAME( 2003, igromula,   igromult, multfish, multfish,  0,             ROT0,  "bootleg", "Igrosoft Multigame Bootleg (15 Games)", GAME_NOT_WORKING ) // no GFX roms

