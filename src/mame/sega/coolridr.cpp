// license:LGPL-2.1+
// copyright-holders:Angelo Salese, David Haywood
/******************************************************************************************************

    System H1 (c) 1994 Sega

    Driver by David Haywood, Angelo Salese and Tomasz Slanina
    special thanks to Guru for references and HW advices

    Games on this system include Cool Riders and Aqua Stage.

    This system is interesting in that it makes use of an RLE compression scheme, which
    is not something common for Sega hardware, there is a patent for it ( 6,141,122 )
    http://www.google.com/patents/US6141122

    TODO:
    - Understand what the 0x400000c reads on SH-2 really do.
    - improve sound emulation
    - i8237 purpose is unknown (missing ROM for comms?).
    - verify zooming etc. our current algorithm is a bit ugly for text

=======================================================================================================

Cool Riders
Sega 1994

This game runs on SYSTEM-H1 hardware. The hardware seems overly complex for a
2D bike racing game? The design of the PCB is very similar to vanilla Model 2
(i.e. Daytona etc). However instead of fully custom-badged chips, many of the
custom chips are off-the-shelf Hitachi/Toshiba gate-arrays.


PCB Layouts
-----------

SYSTEM-H1 ROM BD
171-6516C
837-9623
834-11482 (sticker)
|--------------------------------------------------------------|
|  IC17            IC18              IC5               IC10    |
|                                                              |
|  IC15            IC16              IC4               IC9     |
|                                                              |
|  IC13            IC14              IC3               IC8     |
|                                                              |
|  IC11            IC12              IC2               IC7     |
|                                                              |
|  IC31            IC32              IC1               IC6     |
|                                                              |
|  IC29            IC30                                        |
|JP1-JP8                                                       |
|  LED    32.500MHz   CN1         JP9-JP12    CN2              |
|--------------------------------------------------------------|
Notes:
      CN1/2   - Connectors joining to CPU board
      JP1-8   - Jumpers to set ROM sizes
                JP1-4 set to 1-2
                JP5-8 set to 2-3
      JP9-12  - Jumpers to set ROM sizes
                JP9 set to 1-2
                JP10-12 open (no jumpers) but JP12 pin 2 tied to JP10 pin 1
      IC*     - IC29-IC32 are 27C4002 EPROM
                IC1-IC10 are DIP42 32M mask ROM
                IC11-IC18 are DIP42 16M mask ROM


SYSTEM-H1 COMMUNICATION-BD
171-6849B
837-10942
|----------------------------------|
|                 CN2              |
|       74F74   74F245  MB84256    |
|       74F373  74F245  MB84256    |
|MB89237A         CN1              |
|                                  |
|       74F138 74F04   74F125      |
|                                  |
|       74F157 74F161  74F02       |
|                                  |
|       74F04  74F74   74F86     TX|
|MB89374                           |
|       74F02  74F160            RX|
|                         SN75179  |
|                     JP1 JP2 JP3  |
|       LED               CN3      |
|----------------------------------|
Notes: (All IC's shown)
      CN1/2   - Connectors joining to CPU board
      CN3     - Connector joining to Filter board
      RX/TX   - Optical cable connections for network (not used)
      JP*     - 3x 2-pin jumpers. JP1 shorted, other jumpers open
      MB84256 - Fujitsu MB84256 32k x8 SRAM (NDIP28)
      MB89374 - Fujitsu MB89374 Data Link Controller (SDIP42)
      MB89237A- Fujitsu MB89237A 8-Bit Proprietary DMAC (?) (DIP40)
      SN75179 - Texas Instruments SN75179 Differential Driver and Receiver Pair (DIP8)


SYSTEM-H1 CPU BD
171-6651A
837-10389
837-11481 (sticker)
833-11483 COOL RIDERS (sticker)
|--------------------------------------------------------------|
|                                                EPR-17662.IC12|
|                                                              |
|   |--------|    |--------|                                   |
|   |SEGA    |    |SEGA    |           FM1208S      SEC_CONN   |
|   |315-5758|    |315-5757|       CN2         CN1             |
|   |        |    |        |           |------|            CN10|
|   |--------|    |--------|           |SH7032|                |
|CN8                                   |      |                |
|                                      |------|                |
|                                                              |
|                   PAL1  JP1 JP4 JP6 JP2                      |
|                   PAL2   JP3 JP5     MB3771                  |
|     28MHz   32MHz              PC910                 A1603C  |
|                                               DSW1   DAN803  |
|HM5241605                       |--------| |--------| DAP803  |
|HM5241605      PAL3             |SEGA    | |SEGA    |         |
|                                |315-5687| |315-5687| 315-5649|
|                                |        | |        |         |
|   |-----|        TMP68HC000N-16|--------| |--------|         |
|   |SH2  |                       514270      514270           |
|   |     |                         CN12               A1603C  |
|CN7|-----|         MB84256            TDA1386   TL062     CN11|
|                           22.579MHz   CN14                   |
|                   MB84256                                    |
|                                                              |
|                                      TDA1386   TL062         |
|--------------------------------------------------------------|
Notes:
      22.579MHz   - This OSC is tied to pin 1 of a 74AC04 logic chip. The output from that (pin 2) is tied
                    directly to both 315-5687 chips on pin 14. Therefore the clock input of the YMF292's is 22.579MHz
      514270      - Hitachi HM514270AJ-7 256k x16 DRAM (SOJ40)
      68000       - Clock 16.00MHz [32/2]
      A1603C      - NEC uPA1603C Monolithic N-Channel Power MOS FET Array (DIP16)
      CN7/8       - Connectors joining to ROM board (above)
      CN10/11     - Connectors joining to Filter board
      CN12/14     - Connectors for (possible) extra sound board (not used)
      DAN803      - Diotec Semiconductor DAN803 Small Signal Diode Array with common anodes (SIL9)
      DAP803      - Diotec Semiconductor DAP803 Small Signal Diode Array with common cathodes (SIL9)
      DSW1        - 4-position DIP switch. All OFF
      EPR-17662   - Toshiba TC57H1025 1M EPROM (DIP40)
      FM1208S     - RAMTRON FM1208S 4k (512 bytes x8) Nonvolatile Ferroelectric RAM (SOIC24)
      HM5241605   - Hitachi HM5241605 4M (256k x 16 x 2 banks) SDRAM (SSOP50)
      JP1-6       - Jumpers. JP2 open. JP5 1-2. All others 2-3
      MB3771      - Fujitsu MB3771 Master Reset IC (SOIC8)
      MB84256     - Fujitsu MB84256 32k x8 SRAM (SOP28)
      PAL1        - GAL16V8B also marked '315-5800' (DIP20)
      PAL2        - GAL16V8B also marked '315-5802' (DIP20)
      PAL3        - GAL16V8B also marked '315-5801' (DIP20)
      PC910       - Sharp PC910 opto-isolator (DIP8)
      SEC_CONN    - Sega security-board connector (not used)
      SH7032      - Hitachi HD6417032F20 ROMless SH1 CPU (QFP112). Clock input 16.00MHz on pin 71
      SH2         - Hitachi HD6417095 SH2 CPU (QFP144). Clock input 28.00MHz on pin 118
      TDA1386     - Philips TDA1386T Noise Shaping Filter DAC (SOP24)
      TL062       - Texas Instruments TL062 Low Power JFET Input Operational Amplifier (SOIC8)
      Sega Custom - 315-5757 (QFP160)
                    315-5758 (QFP168) also marked 'HG62G035R26F'
                    315-5649 (QFP100) custom I/O chip (also used on Model 2A/2B/2C, but NOT vanilla Model 2)
                    315-5687 (QFP128 x2) also marked 'YMF292-F' (also used on Model 2A/2B/2C and ST-V)
      Syncs       - Horizontal 24.24506kHz
                    Vertical 57.0426Hz


SYSTEM-H1 VIDEO BD
171-6514F
837-9621
|--------------------------------------------------------------|
|         CN2                                                  |
|                                                           JP4|
|                                   |--------|  TC55328 D431008|
|      |--------|          HM514270 |SEGA    |  TC55328 D431008|
|      |SEGA    |          HM514270 |315-5697|              JP3|
|CN1   |315-5691|  TC55328 HM514270 |        |  D431008 D431008|
|      |        |  TC55328 HM514270 |--------|  D431008 D431008|
|      |--------|                                           JP2|
|                                                           JP1|
|                                     315-5698   315-5648      |
|        |------------|                                        |
|        |  SEGA      |      50MHz                             |
|        |  315-5692  |               315-5696   315-5648      |
|        |            |                                     CN9|
|LED     |------------|                                        |
|40MHz     |--------|                 315-5696   315-5648      |
|          |SEGA    |                                          |
|   PAL1   |315-5693|                                          |
|CN4       |        |       315-5695  315-5695   315-5648      |
||--------||--------|                                          |
||SEGA    |                                                    |
||315-5694||--------| M5M411860 M5M411860 M5M411860 M5M411860  |
||        ||SEGA    | M5M411860 M5M411860 M5M411860 M5M411860  |
||--------||315-5693| M5M411860 M5M411860 M5M411860 M5M411860  |
|   PAL2   |        | M5M411860 M5M411860 M5M411860 M5M411860  |
|          |--------|                                          |
|--------------------------------------------------------------|
Notes:
      CN9         - Connector joining to Filter board
      CN1/2/4     - Connectors joining to CPU board
      JP1/2/3/4   - 4x 3-pin jumpers. All set to 1-2
      D431008     - NEC D431008 128k x8 SRAM (SOJ32)
      HM514270    - Hitachi HM514270AJ7 256k x16 DRAM (SOJ40)
      M5M411860   - Mitsubishi M5M411860TP435SF00-7 DRAM with fast page mode, 64k-words x 18 bits per word (maybe?) (TSOP42)
      TC55328     - Toshiba TC55328AJ-15 32k x8 SRAM (SOJ24)
      PAL1        - GAL16V8B also marked '315-5803' (DIP20)
      PAL2        - GAL16V8B also marked '315-5864' (DIP20)
      Sega Custom - 315-5648 (QFP64, x4)
                    315-5691 also marked 'HG62S0791R17F' (QFP208)
                    315-5692 also marked 'HG51B152FD' (QFP256)
                    315-5693 also marked 'HG62G019R16F' (QFP168, x3)
                    315-5694 (QFP208)
                    315-5695 (QFP100, x2)
                    315-5696 (QFP120, x2)
                    315-5697 (QFP208)
                    315-5698 (QFP144)

*******************************************************************************************************

Note: This hardware appears to have been designed as a test-bed for a new RLE based compression system
      used by the zooming sprites.  It is possible that Sega planned on using this for ST-V, but
      decided against it. Video/CPU part numbers give an interesting insight, since video hardware #
      sits between Model 1 & Model 2.

                 Year on
      System      PCB     PCB #      PALs
      ---------------------------------------------------------
      System32    1990    837-7428   315-5441 315-5442
      SysMulti32  1992    837-8676   315-5596
      Model 1     1992    837-8886   315-5546 315-5483 315-5484
      Model 2     1994    837-10071  315-5737 315-5741
      Model 2A    1994    837-10848  315-5737 315-5815
      STV         1994    837-10934  315-5833

      H1 (CPU)    1994    837-10389  315-5800 315-5801 315-5802
      H1 (Video)  1994    837-9621   315-5803 315-5864


   NOTE:  While the hardware and title screen might list 1994 as a copyright, MAME uses 1995 due to the
   abudance of evidence in trade journals and even it's own service manuals showing the year as 1995.

   References:
   Arcade game magazine called 'Gamest' show released on 04.28.1995
   VGL (Ultimate Video Game List published by AMP group) - year is printed as '94(4.95)'
   Sega Arcade History (published by Enterbrain) is '1995/4'.


******************************************************************************************************/

/*

some Sprite compression notes from Charles and Andrew

OK so here's what we know. Andrew and I were playing with the ROMs and
Guru traced out connections on the board at the time:

The ten graphics ROMs are IC1-IC10. Each is 16 bits, and all have a
common enable and common A20 input. All data lines are unique; nothing
is shared.

There are two independent address buses:
- One drives A0-A19 of IC1-IC5
- The other drives A0-A19 of IC6-IC10

So the ROMs are arranged as 4,194,304 x 160-bit words (very wide), and
are divided into two banks of 2,097,152 x 160-bit words per bank,
where the common A20 input selects which bank is currently being used.
This bank bit may not come from the graphics hardware and could just
be a spare I/O pin on some other chip, we don't know.

Then, within the current bank selected, the video chip can generate
independent addresses for IC1-5 and IC6-10 in parallel, so that it can
read 80 bits from IC1-5 at one address, and 80 bits from IC6-10 at
another address simultaneously. Or it can drive the same address and
read a 160-bit word at once. Regardless both addresses are restricted
to the same bank as defined through A20.

*/

// http://www.nicozon.net/watch/sm7834644  (first part is Aqua Stage video?)
// http://www.system16.com/hardware.php?id=841 has a picture of Aqua Stage showing the wide aspect


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/sh/sh7032.h"
#include "cpu/sh/sh7604.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "315_5649.h"
#include "sound/scsp.h"
#include "emupal.h"
#include "layout/generic.h"
#include "screen.h"
#include "speaker.h"

#include "aquastge.lh"


namespace {

#define CLIPMAXX_FULL (496-1)
#define CLIPMAXY_FULL (384-1)
#define CLIPMINX_FULL (0)
#define CLIPMINY_FULL (0)


class coolridr_state : public driver_device
{
public:
	coolridr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_textBytesToWrite(0x00),
		m_blitterSerialCount(0x00),
		m_blitterMode(0x00),
		m_textOffset(0x0000),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this,"sub"),
		m_soundcpu(*this,"soundcpu"),
		//m_dmac(*this, "i8237"),
		m_framebuffer_vram(*this, "fb_vram"),
		m_txt_vram(*this, "txt_vram"),
		m_txt_blit(*this, "txt_blit"),
		m_workram_h(*this, "workrah"),
		m_sound_dma(*this, "sound_dma"),
		m_soundram(*this, "soundram%u", 1U),
		m_rom(*this, "maincpu"),
		m_compressedgfx(*this, "compressedgfx"),
		m_io_config(*this, "CONFIG"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen")
	{
	}

	// Blitter state
	uint16_t m_textBytesToWrite;
	int16_t  m_blitterSerialCount;
	uint8_t  m_blitterMode;
	uint8_t  m_blittype;
	uint16_t m_blitterAddr;
	uint16_t m_textOffset;
	uint32_t m_blitterClearMode;
	int16_t  m_blitterClearCount;
	std::unique_ptr<pen_t[]> m_fadedpals;

	// store the blit params here
	uint32_t m_spriteblit[12];
	uint32_t m_vregs_address;

	uint32_t m_clipvals[2][3];
	uint8_t  m_clipblitterMode[2]; // hack

	required_device<sh7604_device> m_maincpu;
	required_device<sh7032_device> m_subcpu;
	required_device<cpu_device> m_soundcpu;
	//required_device<am9517a_device> m_dmac;

	required_shared_ptr<uint32_t> m_framebuffer_vram;
	required_shared_ptr<uint32_t> m_txt_vram;
	required_shared_ptr<uint32_t> m_txt_blit;
	required_shared_ptr<uint32_t> m_workram_h;
	required_shared_ptr<uint32_t> m_sound_dma;
	required_shared_ptr_array<uint16_t, 2> m_soundram;
	required_region_ptr<uint32_t> m_rom;
	required_region_ptr<uint8_t> m_compressedgfx;
	required_ioport m_io_config;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	bitmap_ind16 m_temp_bitmap_sprites[2];
	//bitmap_ind16 m_zbuffer_bitmap[2];

	bitmap_ind16 m_screen_bitmap[2];
	uint8_t m_an_mux_data;
	uint8_t m_sound_data, m_sound_fifo;

	std::unique_ptr<uint16_t[]> m_expanded_10bit_gfx;
	std::unique_ptr<uint16_t[]> m_rearranged_16bit_gfx;

	uint32_t get_20bit_data(uint32_t romoffset, int _20bitwordnum);
	uint16_t get_10bit_data(uint32_t romoffset, int _10bitwordnum);

	uint32_t sound_dma_r(offs_t offset);
	void sound_dma_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t unk_blit_r(offs_t offset);
	void unk_blit_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void blit_mode_w(uint32_t data);
	void blit_data_w(address_space &space, uint32_t data);
	void fb_mode_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void fb_data_w(offs_t offset, uint32_t data);

	void dma_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template<int Chip> uint16_t soundram_r(offs_t offset);
	template<int Chip> void soundram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void lamps_w(uint8_t data);
	void scsp1_to_sh1_irq(int state);
	void scsp2_to_sh1_irq(int state);
	void sound_to_sh1_w(uint8_t data);
	void init_coolridr();
	void init_aquastge();
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	uint32_t m_colbase;

	void coolriders_drawgfx_opaque(bitmap_ind16 &dest, const rectangle &cliprect, gfx_element *gfx,
		uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty);

	void coolriders_drawgfx_transpen(bitmap_ind16 &dest, const rectangle &cliprect, gfx_element *gfx,
		uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty,
		uint32_t transpen);

	void draw_bg_coolridr(bitmap_ind16 &bitmap, const rectangle &cliprect, int which);
	template<int Screen> uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void blit_current_sprite(address_space &space);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt_main);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt_sub);
	void scsp_irq(offs_t offset, uint8_t data);

	void dma_transfer( address_space &space, uint16_t dma_index );

	int m_debug_randompal;

	std::unique_ptr<uint16_t[]> m_vram;
	std::unique_ptr<uint8_t[]> m_pcgram;
	std::unique_ptr<uint16_t[]> m_palram;
	int m_gfx_index;
	int m_color_bank;
	struct {
		uint32_t setting;
		uint8_t gradient;
	}m_rgb_ctrl[2];
	uint32_t m_pen_fill[2];

	osd_work_queue *    m_work_queue[2]; // work queue, one per screen
	static void *draw_object_threaded(void *param, int threadid);
	int m_usethreads;

	struct cool_render_object
	{
		cool_render_object(coolridr_state &s) : state(s), colbase(s.m_colbase)
		{
			std::copy(std::begin(s.m_spriteblit), std::end(s.m_spriteblit), std::begin(spriteblit));
		}

		std::unique_ptr<uint8_t []> indirect_tiles;
		std::unique_ptr<uint32_t []> indirect_zoom;
		uint32_t spriteblit[12];
		bitmap_ind16* drawbitmap = nullptr;
		//bitmap_ind16* zbitmap = nullptr;
		uint16_t zpri = 0;
		uint8_t blittype = 0;
		coolridr_state& state;
		uint32_t clipvals[3];
		int screen = 0;
		int colbase;
	};

	std::unique_ptr<std::unique_ptr<cool_render_object> []> m_cool_render_object_list[2];

	int m_listcount[2];

	// the decode cache mechansim is an optimization
	// we know all gfx are in ROM, and that calling the RLE decompression every time they're used is slow, so we cache the decoded tiles
	// and objects after they're used, for future re-use, quite handy with a driving game.

#define DECODECACHE_NUMOBJECTCACHES (128)

#define DECODECACHE_NUMSPRITETILES (16*16)

	// decode cache
	struct objectcache
	{
		// these needs to be all the elements actually going to affect the decode of an individual tile for any given object
		uint32_t lastromoffset;
		uint16_t lastused_flipx;
		uint16_t lastused_flipy;
		uint32_t lastblit_rotate;
		uint32_t lastb1mode;
		uint32_t lastb1colorNumber;
		uint32_t lastb2colorNumber;
		uint32_t lastb2altpenmask;
		uint16_t lastused_hCellCount;
		uint16_t lastused_vCellCount;
		int repeatcount;

		struct dectile
		{
			uint16_t tempshape_multi[16*16];
			bool tempshape_multi_decoded;
			bool is_blank;
		};

		dectile tiles[DECODECACHE_NUMSPRITETILES];

	};

	struct objcachemanager
	{
		int current_object;
		objectcache objcache[DECODECACHE_NUMOBJECTCACHES];

		// fallback decode buffer for certain cases (indirect sprites, sprites too big for our buffer..)
		uint16_t tempshape[16*16];
	};

	objcachemanager m_decode[2];
	void aquastge(machine_config &config);
	void coolridr(machine_config &config);
	void coolridr_h1_map(address_map &map) ATTR_COLD;
	void aquastge_h1_map(address_map &map) ATTR_COLD;
	void aquastge_submap(address_map &map) ATTR_COLD;
	void coolridr_submap(address_map &map) ATTR_COLD;
	void system_h1_map(address_map &map) ATTR_COLD;
	void system_h1_submap(address_map &map) ATTR_COLD;
	void system_h1_sound_map(address_map &map) ATTR_COLD;
	template<int Chip> void scsp_map(address_map &map) ATTR_COLD;
};

#define PRINT_BLIT_STUFF \
	printf("type blit %08x %08x(%d, %03x) %08x(%02x, %03x) %08x(%06x) %08x(%08x, %d, %d, %d) %08x(%d,%d) %04x %04x %04x %04x %08x %08x %d %d\n", blit0, blit1_unused,b1mode,b1colorNumber, blit2_unused,b2tpen,b2colorNumber, blit3_unused,b3romoffset, blit4_unused, blit4blendlevel, blit_flipy,blit_rotate, blit_flipx, blit5_unused, indirect_tile_enable, indirect_zoom_enable, vCellCount, hCellCount, vZoom, hZoom, blit10, textlookup, vPosition, hPosition);


/* video */

#define VRAM_SIZE 0x100000

static const gfx_layout h1_tile_layout =
{
	16,16,
	0x1000,
	8,
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	{ STEP16(0,8*16) },
	16*128
};


void coolridr_state::video_start()
{
	/* find first empty slot to decode gfx */
	for (m_gfx_index = 0; m_gfx_index < MAX_GFX_ELEMENTS; m_gfx_index++)
		if (m_gfxdecode->gfx(m_gfx_index) == nullptr)
			break;

	m_fadedpals = make_unique_clear<pen_t[]>(0x8000);
	m_vram = make_unique_clear<uint16_t[]>(VRAM_SIZE);
	m_pcgram = make_unique_clear<uint8_t[]>(VRAM_SIZE);
	m_palram = make_unique_clear<uint16_t[]>(VRAM_SIZE);

	m_cool_render_object_list[0] = std::make_unique<std::unique_ptr<cool_render_object> []>(1000000);
	m_listcount[0] = 0;

	m_cool_render_object_list[1] = std::make_unique<std::unique_ptr<cool_render_object> []>(1000000);
	m_listcount[1] = 0;

	m_screen->register_screen_bitmap(m_temp_bitmap_sprites[0]);
	m_screen->register_screen_bitmap(m_temp_bitmap_sprites[1]);
	//m_screen->register_screen_bitmap(m_zbuffer_bitmap[0]);
	//m_screen->register_screen_bitmap(m_zbuffer_bitmap[1]);

	m_screen->register_screen_bitmap(m_screen_bitmap[0]);
	m_screen->register_screen_bitmap(m_screen_bitmap[1]);

	m_gfxdecode->set_gfx(m_gfx_index, std::make_unique<gfx_element>(m_palette, h1_tile_layout, m_pcgram.get(), 0, 8, 0));

	m_debug_randompal = 9;

	save_pointer(NAME(m_fadedpals), 0x8000);
	save_pointer(NAME(m_vram), VRAM_SIZE);
	save_pointer(NAME(m_pcgram), VRAM_SIZE);
	save_pointer(NAME(m_palram), VRAM_SIZE);
}

/*
    vregs are setted up with one of DMA commands (see below)
    0x3e09b80 screen 1 base, 0x3e9bc0 screen 2 base
    [+0x1c] ---- ---- ---- ---- ---- ---- ---- -xxx tile offset start * 0x8000
    [+0x2c] ---- -xxx xxxx xxxx ---- --yy yyyy yyyy scrolling registers
    [+0x34] 1111 1111 2222 2222 3333 3333 4444 4444 - Almost surely Sega "map" registers
    [+0x38] 5555 5555 6666 6666 7777 7777 8888 8888 /
    [+0x3c] x--- ---- ---- ---- xxxx xxxx xxxx xxxx 0x11b8 on bike select, 0xffffffff otherwise, transparent pen value?
    (everything else is unknown at current time)
*/

#define COOLRIDERS_DRAWGFX_CORE(PIXEL_TYPE, COOL_PIXEL_OP)                               \
do {                                                                                    \
	do {                                                                                \
		const uint8_t *srcdata;                                                           \
		int32_t destendx, destendy;                                                       \
		int32_t srcx, srcy;                                                               \
		int32_t curx, cury;                                                               \
		int32_t dy;                                                                       \
																						\
		assert(dest.valid());                                                           \
		assert(gfx != nullptr);                                                            \
		assert(dest.cliprect().contains(cliprect));                                     \
		assert(code < gfx->elements());                                             \
																						\
		/* ignore empty/invalid cliprects */                                            \
		if (cliprect.empty())                                                           \
			break;                                                                      \
																						\
		/* compute final pixel in X and exit if we are entirely clipped */              \
		destendx = destx + gfx->width() - 1;                                                \
		if (destx > cliprect.max_x || destendx < cliprect.min_x)                        \
			break;                                                                      \
																						\
		/* apply left clip */                                                           \
		srcx = 0;                                                                       \
		if (destx < cliprect.min_x)                                                     \
		{                                                                               \
			srcx = cliprect.min_x - destx;                                              \
			destx = cliprect.min_x;                                                     \
		}                                                                               \
																						\
		/* apply right clip */                                                          \
		if (destendx > cliprect.max_x)                                                  \
			destendx = cliprect.max_x;                                                  \
																						\
		/* compute final pixel in Y and exit if we are entirely clipped */              \
		destendy = desty + gfx->height() - 1;                                               \
		if (desty > cliprect.max_y || destendy < cliprect.min_y)                        \
			break;                                                                      \
																						\
		/* apply top clip */                                                            \
		srcy = 0;                                                                       \
		if (desty < cliprect.min_y)                                                     \
		{                                                                               \
			srcy = cliprect.min_y - desty;                                              \
			desty = cliprect.min_y;                                                     \
		}                                                                               \
																						\
		/* apply bottom clip */                                                         \
		if (destendy > cliprect.max_y)                                                  \
			destendy = cliprect.max_y;                                                  \
																						\
		/* apply X flipping */                                                          \
		if (flipx)                                                                      \
			srcx = gfx->width() - 1 - srcx;                                             \
																						\
		/* apply Y flipping */                                                          \
		dy = gfx->rowbytes();                                                           \
		if (flipy)                                                                      \
		{                                                                               \
			srcy = gfx->height() - 1 - srcy;                                                \
			dy = -dy;                                                                   \
		}                                                                               \
																						\
		/* fetch the source data */                                                     \
		srcdata = gfx->get_data(code);                                      \
																						\
		/* compute how many blocks of 4 pixels we have */                           \
		uint32_t numblocks = (destendx + 1 - destx) / 4;                              \
		uint32_t leftovers = (destendx + 1 - destx) - 4 * numblocks;                  \
																					\
		/* adjust srcdata to point to the first source pixel of the row */          \
		srcdata += srcy * gfx->rowbytes() + srcx;                                   \
																					\
		/* non-flipped 8bpp case */                                                 \
		if (!flipx)                                                                 \
		{                                                                           \
			/* iterate over pixels in Y */                                          \
			for (cury = desty; cury <= destendy; cury++)                            \
			{                                                                       \
				PIXEL_TYPE *destptr = &dest.pix(cury, destx);                       \
				const uint8_t *srcptr = srcdata;                                    \
				srcdata += dy;                                                      \
																					\
				/* iterate over unrolled blocks of 4 */                             \
				for (curx = 0; curx < numblocks; curx++)                            \
				{                                                                   \
					COOL_PIXEL_OP(destptr[0], srcptr[0]);                           \
					COOL_PIXEL_OP(destptr[1], srcptr[1]);                           \
					COOL_PIXEL_OP(destptr[2], srcptr[2]);                           \
					COOL_PIXEL_OP(destptr[3], srcptr[3]);                           \
																					\
					srcptr += 4;                                                    \
					destptr += 4;                                                   \
				}                                                                   \
																					\
				/* iterate over leftover pixels */                                  \
				for (curx = 0; curx < leftovers; curx++)                            \
				{                                                                   \
					COOL_PIXEL_OP(destptr[0], srcptr[0]);                           \
					srcptr++;                                                       \
					destptr++;                                                      \
				}                                                                   \
			}                                                                       \
		}                                                                           \
																					\
		/* flipped 8bpp case */                                                     \
		else                                                                        \
		{                                                                           \
			/* iterate over pixels in Y */                                          \
			for (cury = desty; cury <= destendy; cury++)                            \
			{                                                                       \
				PIXEL_TYPE *destptr = &dest.pix(cury, destx);                       \
				const uint8_t *srcptr = srcdata;                                    \
				srcdata += dy;                                                      \
																					\
				/* iterate over unrolled blocks of 4 */                             \
				for (curx = 0; curx < numblocks; curx++)                            \
				{                                                                   \
					COOL_PIXEL_OP(destptr[0], srcptr[ 0]);                    \
					COOL_PIXEL_OP(destptr[1], srcptr[-1]);                    \
					COOL_PIXEL_OP(destptr[2], srcptr[-2]);                    \
					COOL_PIXEL_OP(destptr[3], srcptr[-3]);                    \
																					\
					srcptr -= 4;                                                    \
					destptr += 4;                                                   \
				}                                                                   \
																					\
				/* iterate over leftover pixels */                                  \
				for (curx = 0; curx < leftovers; curx++)                            \
				{                                                                   \
					COOL_PIXEL_OP(destptr[0], srcptr[0]);                     \
					srcptr--;                                                       \
					destptr++;                                                      \
				}                                                                   \
			}                                                                       \
		}                                                                           \
	} while (0);                                                                        \
} while (0)


#define COOLRIDERS_PIXEL_OP_REMAP_OPAQUE(DEST, SOURCE)                               \
do                                                                                  \
{                                                                                   \
	(DEST) = paldata[SOURCE];                                                       \
}                                                                                   \
while (0)

#define COOLRIDERS_PIXEL_OP_REMAP_TRANSPEN(DEST, SOURCE)                             \
do                                                                                  \
{                                                                                   \
	uint32_t srcdata = (SOURCE);                                                      \
	if (srcdata != transpen)                                                        \
		(DEST) = paldata[srcdata];                                                  \
}                                                                                   \
while (0)



void coolridr_state::coolriders_drawgfx_opaque(bitmap_ind16 &dest, const rectangle &cliprect, gfx_element *gfx,
		uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty)
{
	const uint16_t *paldata = &m_palram[gfx->colorbase() + gfx->granularity() * (color % gfx->colors())];
	code %= gfx->elements();
	COOLRIDERS_DRAWGFX_CORE(uint16_t, COOLRIDERS_PIXEL_OP_REMAP_OPAQUE);
}

void coolridr_state::coolriders_drawgfx_transpen(bitmap_ind16 &dest, const rectangle &cliprect, gfx_element *gfx,
		uint32_t code, uint32_t color, int flipx, int flipy, int32_t destx, int32_t desty,
		uint32_t transpen)
{
	// special case invalid pens to opaque
	if (transpen > 0xff)
		return coolriders_drawgfx_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty);

	// use pen usage to optimize
	code %= gfx->elements();
	if (gfx->has_pen_usage())
	{
		// fully transparent; do nothing
		uint32_t usage = gfx->pen_usage(code);
		if ((usage & ~(1 << transpen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << transpen)) == 0)
			return coolriders_drawgfx_opaque(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty);
	}

	// render
	const uint16_t *paldata = &m_palram[gfx->colorbase() + gfx->granularity() * (color % gfx->colors())] ;
	COOLRIDERS_DRAWGFX_CORE(uint16_t, COOLRIDERS_PIXEL_OP_REMAP_TRANSPEN);
}

void coolridr_state::draw_bg_coolridr(bitmap_ind16 &bitmap, const rectangle &cliprect, int which)
{
	int bg_r,bg_g,bg_b;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	if(m_pen_fill[which])
	{
#if 0
		/* logic here is a bit of a guess. - should probably be 555 like everything else.. or we are going to have to have a rgb32 bitmap? */
		bg_r = (((m_pen_fill[which] >> 16) & 0x7f) << 1) | (((m_pen_fill[which] >> 16) & 0x80) >> 7);
		bg_g = (((m_pen_fill[which] >> 8) & 0x7f) << 1) | (((m_pen_fill[which] >> 8) & 0x80) >> 7);
		bg_b = (((m_pen_fill[which] >> 0) & 0x7f) << 1) | (((m_pen_fill[which] >> 0) & 0x80) >> 7);
		bitmap.fill(rgb_t(0xff,bg_r,bg_g,bg_b),cliprect);
#endif

		bg_r = (((m_pen_fill[which] >> 16) & 0x78) >> 2) | (((m_pen_fill[which] >> 16) & 0x80) >> 7);
		bg_g = (((m_pen_fill[which] >> 8) & 0x78) >> 2) | (((m_pen_fill[which] >> 8) & 0x80) >> 7);
		bg_b = (((m_pen_fill[which] >> 0) & 0x78) >> 2) | (((m_pen_fill[which] >> 0) & 0x80) >> 7);
		bitmap.fill( (bg_r<<10) | (bg_g << 5) | bg_b  ,cliprect);
	}
	else
	{
		uint8_t transpen_setting;
		gfx_element *gfx = m_gfxdecode->gfx(m_gfx_index);
		#define VREG(_offs) \
			space.read_dword(m_vregs_address+_offs+which*0x40)

		uint16_t const scrollx = (VREG(0x2c) >> 16) & 0x7ff;
		uint16_t const scrolly = VREG(0x2c) & 0x3ff;

		uint32_t const base_offset = (VREG(0x1c) * 0x8000)/2;
		m_color_bank = which * 2;
		/* TODO: the whole transpen logic might be incorrect */
		transpen_setting = (VREG(0x3c) & 0x80000000) >> 31;
		bg_r = (VREG(0x3c) >> 10) & 0x1f;
		bg_g = (VREG(0x3c) >> 5) & 0x1f;
		bg_b = (VREG(0x3c) >> 0) & 0x1f;

		bitmap.fill(VREG(0x3c),cliprect);


		uint16_t basey = ((scrolly + cliprect.top()) & 0x3ff) >> 4;
		for (int y = cliprect.top() >> 4; y <= (cliprect.bottom() + 15) >> 4; y++)
		{
			uint16_t basex = ((scrollx + cliprect.left()) & 0x7ff) >> 4;
			for (int x = cliprect.left() >> 4; x <= (cliprect.right() + 15) >> 4; x++)
			{
				uint16_t const vram_data = (m_vram[((basex&0x7f)+((basey&0x3f)*0x80)+base_offset)&0x07ffff] & 0xffff);
				uint16_t const color = m_color_bank + ((vram_data & 0x800) >> 11) * 4;
				/* bike select enables bits 15-12, pretty sure one of these is tile bank (because there's a solid pen on 0x3ff / 0x7ff). */
				uint16_t const tile = (vram_data & 0x7ff) | ((vram_data & 0x8000) >> 4);

				coolriders_drawgfx_transpen(bitmap,cliprect,gfx,tile,color,0,0,(x*16)-(scrollx&0xf),(y*16)-(scrolly&0xf),transpen_setting ? -1 : 0);

				basex++;
			}
			basey++;
		}
	}


}

template<int Screen>
uint32_t coolridr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if(m_rgb_ctrl[Screen].gradient)
	{
		if( (m_rgb_ctrl[Screen].setting == 0x1240) || (m_rgb_ctrl[Screen].setting == 0x920) || (m_rgb_ctrl[Screen].setting == 0x800) )
		{
		}
		else
		{
			popmessage("%08x %08x",m_rgb_ctrl[Screen].setting,m_rgb_ctrl[Screen].gradient);
		}
	}

#if 0
	if (machine().input().code_pressed_once(KEYCODE_W))
	{
		m_debug_randompal++;
		popmessage("%02x",m_debug_randompal);
	}
	if (machine().input().code_pressed_once(KEYCODE_Q))
	{
		m_debug_randompal--;
		popmessage("%02x",m_debug_randompal);
	}
#endif

	// there are probably better ways to do this
	for (int i = 0; i < 0x8000; i++)
	{
		int r = (i >> 10)&0x1f;
		int g = (i >> 5)&0x1f;
		int b = (i >> 0)&0x1f;

		if(m_rgb_ctrl[Screen].gradient)
		{
			/* fade-in / outs */
			if(m_rgb_ctrl[Screen].setting == 0x1240)
			{
				r -= m_rgb_ctrl[Screen].gradient;
				g -= m_rgb_ctrl[Screen].gradient;
				b -= m_rgb_ctrl[Screen].gradient;
				if(r < 0) { r = 0; }
				if(g < 0) { g = 0; }
				if(b < 0) { b = 0; }
			}
			else if(m_rgb_ctrl[Screen].setting == 0x920) /* at bike select / outside tunnels, addition */
			{
				r += m_rgb_ctrl[Screen].gradient;
				g += m_rgb_ctrl[Screen].gradient;
				b += m_rgb_ctrl[Screen].gradient;
				if(r > 0x1f) { r = 0x1f; }
				if(g > 0x1f) { g = 0x1f; }
				if(b > 0x1f) { b = 0x1f; }
			}
			else if(m_rgb_ctrl[Screen].setting == 0x800) /* when you get hit TODO: algo might be different. */
			{
				r += m_rgb_ctrl[Screen].gradient;
				g -= m_rgb_ctrl[Screen].gradient;
				b -= m_rgb_ctrl[Screen].gradient;
				if(r > 0x1f) { r = 0x1f; }
				if(g < 0) { g = 0; }
				if(b < 0) { b = 0; }
			}
		}
		m_fadedpals[i] = (r<<10|g<<5|b);
	}

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint16_t const *const linesrc = &m_screen_bitmap[Screen].pix(y);
		uint16_t *const linedest = &bitmap.pix(y);

		for (int x = cliprect.left(); x<= cliprect.right(); x++)
		{
			linedest[x] = m_fadedpals[linesrc[x]];
		}
	}

	return 0;
}

/* end video */


#define RLE_BLOCK(writeaddrxor) \
	/* skip the decoding if it's the same tile as last time! */ \
	if (!current_decoded) \
	{ \
		lastSpriteNumber = 0xffffffff; /* this optimization is currently broken, so hack it to be disabled here */ \
		if (spriteNumber != lastSpriteNumber) \
		{ \
			blankcount = 256;\
			lastSpriteNumber = spriteNumber; \
			\
			int i = 1;/* skip first 10 bits for now */ \
			int data_written = 0; \
		\
			while (data_written<256) \
			{ \
		\
				const uint16_t compdata = expanded_10bit_gfx[ (b3romoffset) + spriteNumber + i]; \
		\
				if (((compdata & 0x300) == 0x000) || ((compdata & 0x300) == 0x100)) /* 3bpp */ \
				{ \
					/* mm ccrr rrr0 */ \
					int encodelength = (compdata & 0x03e)>>1; \
					const uint16_t rledata =  rearranged_16bit_gfx[color_offs + ((compdata & 0x1c0) >> 6)]; \
					/* guess, blank tiles have the following form */ \
					/* 00120 (00000024,0) | 010 03f */ \
					if (compdata&1) encodelength = 255; \
		\
					while (data_written<256 && encodelength >=0) \
					{ \
						tempshape[data_written^writeaddrxor] = rledata; \
						if (tempshape[data_written^writeaddrxor]==0x8000) blankcount--; \
						encodelength--; \
						data_written++; \
					} \
				} \
				else if ((compdata & 0x300) == 0x200) /* 6bpp */ \
				{ \
					/* mm cccc ccrr */ \
					int encodelength = (compdata & 0x003);           \
					const uint16_t rledata = rearranged_16bit_gfx[color_offs + ((compdata & 0x0fc) >> 2) + 8]; \
					while (data_written<256 && encodelength >=0) \
					{ \
						tempshape[data_written^writeaddrxor] = rledata; /* + 0x8 crt test, most of red, green, start of blue */ \
						if (tempshape[data_written^writeaddrxor]==0x8000) blankcount--; \
						encodelength--; \
						data_written++; \
					} \
				} \
				else /* 8bpp */ \
				{ \
					/* mm cccc cccc */ \
					uint16_t rawdat = (compdata & 0x0ff); \
					if (b1mode && (rawdat > (b2altpenmask + 0x48))) /* does this have to be turned on by b1mode? road ends up with some bad pixels otherwise but maybe the calc is wrong... does it affect the other colour depths too? */ \
						tempshape[data_written^writeaddrxor] = rearranged_16bit_gfx[color_offs2 + (rawdat )+0x48]; /* bike wheels + brake light */ \
					else \
						tempshape[data_written^writeaddrxor] = rearranged_16bit_gfx[color_offs + (rawdat )+0x48]; /* +0x48 crt test end of blue, start of white */ \
					if (tempshape[data_written^writeaddrxor]==0x8000) blankcount--; \
					data_written++; \
				} \
		\
				i++; \
			} \
			if (!indirect_tile_enable && size < DECODECACHE_NUMSPRITETILES) \
			{ \
				object->state.m_decode[screen].objcache[use_object].tiles[v*used_hCellCount + h].tempshape_multi_decoded = true; \
				if (blankcount==0) \
					object->state.m_decode[screen].objcache[use_object].tiles[v*used_hCellCount + h].is_blank = true; \
				else \
					object->state.m_decode[screen].objcache[use_object].tiles[v*used_hCellCount + h].is_blank = false; \
				/* if (object->screen==0) printf("marking offset %04x as decoded (sprite number %08x ptr %08x)\n", v*used_hCellCount + h, spriteNumber, ((uint64_t)(void*)tempshape)&0xffffffff);*/ \
			} \
		} \
	}


#define CHECK_DECODE \
	if (used_flipy) \
	{ \
		if (used_flipx) \
		{ \
			RLE_BLOCK(0xff) \
		} \
		else \
		{ \
			RLE_BLOCK(0xf0) \
		} \
	} \
	else \
	{   if (used_flipx) \
		{ \
			RLE_BLOCK(0x0f) \
		} \
		else \
		{ \
			RLE_BLOCK(0x00) \
		} \
	} \
	if (!indirect_tile_enable && size < DECODECACHE_NUMSPRITETILES) \
	{ \
		if (object->state.m_decode[screen].objcache[use_object].tiles[v*used_hCellCount + h].is_blank == true) \
				continue; \
	} \
	else \
		if (blankcount==0) continue;


#define GET_SPRITE_NUMBER \
	int lookupnum; \
	/* with this bit enabled the tile numbers gets looked up using 'data' (which would be blit11) (eg 03f40000 for startup text) */ \
	/* this allows text strings to be written as 8-bit ascii in one area (using command 0x10), and drawn using multi-width sprites */ \
	if (indirect_tile_enable) \
	{ \
		/* this doesn't handle the various flip modes.. */ \
		lookupnum = object->indirect_tiles[h + (v*used_hCellCount)]; \
	} \
	else \
	{ \
		if (!blit_rotate) \
		{ \
			if (!used_flipy) \
			{ \
				if (!used_flipx) \
					lookupnum = h + (v*used_hCellCount); \
				else \
					lookupnum = (used_hCellCount-h-1) + (v*used_hCellCount); \
			} \
			else \
			{ \
				if (!used_flipx) \
					lookupnum = h + ((used_vCellCount-v-1)*used_hCellCount); \
				else \
					lookupnum = (used_hCellCount-h-1) + ((used_vCellCount-v-1)*used_hCellCount); \
			} \
		} \
		else \
		{ \
			if (!used_flipy) \
			{ \
				if (!used_flipx) \
					lookupnum = v + (h*used_vCellCount); \
				else \
					lookupnum = (used_vCellCount-v-1) + (h*used_vCellCount); \
			} \
			else \
			{ \
				if (!used_flipx) \
					lookupnum = v + ((used_hCellCount-h-1)*used_vCellCount); \
				else \
					lookupnum = (used_vCellCount-v-1) + ((used_hCellCount-h-1)*used_vCellCount); \
			} \
		} \
	} \
	uint32_t spriteNumber = (expanded_10bit_gfx[ (b3romoffset) + (lookupnum<<1) +0 ] << 10) | (expanded_10bit_gfx[ (b3romoffset) + (lookupnum<<1) + 1 ]);

#define DO_XCLIP_REAL \
	if (drawx>clipmaxX) { break; } \
	if (drawx<clipminX) { drawx++; continue; }

#define DO_XCLIP_NONE \
	{ \
	}


#define GET_CURRENT_LINESCROLLZOOM \
	uint32_t dword = object->indirect_zoom[v*16+realy]; \
	uint16_t hZoomHere = hZoom + (dword>>16); \
	if (!hZoomHere) { drawy++; continue; } \
	/* bit 0x8000 does get set too, but only on some lines, might have another meaning? */ \
	int linescroll = dword&0x7fff; \
	if (linescroll & 0x4000) linescroll -= 0x8000; \
	int hPositionTable = linescroll + hPositionx; \
	/* DON'T use the table hZoom in this calc? (road..) */ \
	int sizex = used_hCellCount * 16 * hZoom; \
	hPositionTable *= 0x40; \
	switch (hOrigin & 3) \
	{ \
	case 0: \
		/* left */ \
		break; \
	case 1: \
		hPositionTable -= sizex / 2; \
		/* middle? */ \
		break; \
	case 2: \
		hPositionTable -= sizex-1; \
		/* right? */ \
		break; \
	case 3: \
		/* invalid? */ \
		break; \
	}



#define YXLOOP \
	int drawy = pixelOffsetY; \
	for (int y = 0; y < blockhigh; y++) \
	{ \
		int realy = ((y*incy)>>21); \
		GET_CURRENT_LINESCROLLZOOM \
		const int pixelOffsetX = ((hPositionTable) + (h* 16 * hZoomHere)) / 0x40; \
		const int pixelOffsetnextX = ((hPositionTable) + ((h+1)* 16 * hZoomHere)) / 0x40; \
		if (drawy>clipmaxY) { break; }; \
		if (drawy<clipminY) { drawy++; continue; }; \
		uint16_t *const line = &drawbitmap->pix(drawy); \
		/* uint16_t *const zline = &object->zbitmap->pix(drawy); */ \
		int blockwide = pixelOffsetnextX-pixelOffsetX; \
		if (pixelOffsetX+blockwide <clipminX) { drawy++; continue; } \
		if (pixelOffsetX>clipmaxX)  { drawy++; continue; } \
		if (pixelOffsetX>=clipminX && pixelOffsetX+blockwide<clipmaxX) \
		{ \
			uint32_t incx = 0x8000000 / hZoomHere; \
			int drawx = pixelOffsetX; \
			for (int x = 0; x < blockwide; x++) \
			{ \
				DO_XCLIP_NONE \
				int realx = ((x*incx)>>21); \
				GET_PIX; \
				DRAW_PIX; \
			} \
		} \
		else \
		{ \
			uint32_t incx = 0x8000000 / hZoomHere; \
			int drawx = pixelOffsetX; \
			for (int x = 0; x < blockwide; x++) \
			{ \
				DO_XCLIP_REAL \
				int realx = ((x*incx)>>21); \
				GET_PIX; \
				DRAW_PIX; \
			} \
		} \
		drawy++; \
	}

#define YXLOOP_NO_LINEZOOM \
	for (int y = 0; y < blockhigh; y++) \
	{ \
		int realy = ((y*incy)>>21); \
		const int drawy = pixelOffsetY+y; \
		if ((drawy>clipmaxY) || (drawy<clipminY)) continue; \
		uint16_t *const line = &drawbitmap->pix(drawy); \
		/* uint16_t *const zline = &object->zbitmap->pix(drawy); */ \
		int drawx = pixelOffsetX; \
		for (int x = 0; x < blockwide; x++) \
		{ \
			DO_XCLIP \
			int realx = ((x*incx)>>21); \
			GET_PIX; \
			DRAW_PIX \
		} \
	}


#define YXLOOP_NO_ZOOM \
	for (int realy = 0; realy < 16; realy++) \
	{ \
		const int drawy = pixelOffsetY+realy; \
		if ((drawy>clipmaxY) || (drawy<clipminY)) continue; \
		uint16_t *const line = &drawbitmap->pix(drawy); \
		/* uint16_t *const zline = &object->zbitmap->pix(drawy); */ \
		int drawx = pixelOffsetX; \
		for (int realx = 0; realx < 16; realx++) \
		{ \
			DO_XCLIP \
			GET_PIX; \
			DRAW_PIX \
		} \
	}


/* the two tables that the patent claims are located at:
    0x1ec800
    0x1f0000
    0x3ec800
    0x3f0000
of each rom ... ROM 1 + 2 gives the full palette data for each pixel, in even/odd order.
TODO: fix anything that isn't text.
*/





#define DRAW_PIX \
	/* I think 0x8000 is ALWAYS transparent */ \
	/* values < 0x8000 have no alpha effect to them */ \
	if (pix < 0x8000) \
	{ \
		/*if (object->zpri < zline[drawx])*/ \
		{ \
			{ \
				line[drawx] = pix&0x7fff; \
				/*zline[drawx] = object->zpri;*/ \
			} \
		} \
	} \
	/* values > 0x8000 have blending */ \
	else if (pix > 0x8000) \
	{ \
		/* a blend level of 0x8 (real register value 0x7 but we added one so we can shift instead of divide in code below) seems to be the same as solid, it is set on most parts of the road and during the 'lovemachine' animation in attract when the heart should be hidden */ \
		/* if (object->zpri < zline[drawx]) */ \
		{ \
			if (blit4blendlevelinv==0x0) \
			{ \
				line[drawx] = pix&0x7fff; \
				/* zline[drawx] = object->zpri; */ \
			} \
			else \
			{ \
				uint16_t source = line[drawx]; \
				int src_r = ((source>>10)&0x1f) * blit4blendlevelinv; \
				int src_g = ((source>>5)&0x1f)  * blit4blendlevelinv; \
				int src_b = ((source>>0)&0x1f)  * blit4blendlevelinv; \
				int dest_r = ((pix>>10)&0x1f) * blit4blendlevel; \
				int dest_g = ((pix>>5)&0x1f)  * blit4blendlevel; \
				int dest_b = ((pix>>0)&0x1f)  * blit4blendlevel; \
				line[drawx] = (((src_r+dest_r)>>3)<<10) | (((src_g+dest_g)>>3)<<5) | (((src_b+dest_b)>>3)<<0); \
				/* zline[drawx] = object->zpri; */ \
			} \
		} \
	} \
	drawx++;


//object->rearranged_16bit_gfx
//object->expanded_10bit_gfx

#define GET_PIX_ROTATED \
		uint16_t pix = tempshape[realx*16+realy];
#define GET_PIX_NORMAL \
		uint16_t pix = tempshape[realy*16+realx];

void *coolridr_state::draw_object_threaded(void *param, int threadid)
{
	const std::unique_ptr<cool_render_object> object(reinterpret_cast<cool_render_object *>(param));
	bitmap_ind16* drawbitmap = object->drawbitmap;

	/************* object->spriteblit[3] *************/

	uint32_t blit3_unused = object->spriteblit[3] & 0xffe00000;
	uint32_t b3romoffset = (object->spriteblit[3] & 0x001fffff)*16;

	if (blit3_unused) printf("unknown bits in blit word %d -  %08x\n", 3, blit3_unused);

	/************* object->spriteblit[5] *************/

	uint32_t blit5_unused = object->spriteblit[5]&0xfffefffe;
	// this might enable the text indirection thing?
	int indirect_tile_enable = (object->spriteblit[5] & 0x00010000)>>16;
	int indirect_zoom_enable = (object->spriteblit[5] & 0x00000001);


	if (blit5_unused) printf("unknown bits in blit word %d -  %08x\n", 5, blit5_unused);
	// 00010000 (text)
	// 00000001 (other)




	uint16_t* rearranged_16bit_gfx = object->state.m_rearranged_16bit_gfx.get();
	uint16_t* expanded_10bit_gfx = object->state.m_expanded_10bit_gfx.get();

	int16_t clipminX = CLIPMINX_FULL;
	int16_t clipmaxX = CLIPMAXX_FULL;
	int16_t clipminY = CLIPMINY_FULL;
	int16_t clipmaxY = CLIPMAXY_FULL;


	/************* object->spriteblit[1] *************/

	// 000u0ccc  - c = colour? u = 0/1
	uint32_t blit1_unused = object->spriteblit[1] & 0xfffef800;
	uint32_t b1mode = (object->spriteblit[1] & 0x00010000)>>16;
	uint32_t b1colorNumber = (object->spriteblit[1] & 0x000007ff);    // Probably more bits

	if (blit1_unused!=0) printf("blit1 unknown bits set %08x\n", object->spriteblit[1]);


	if (b1mode)
	{
	//  b1colorNumber = object->state.machine().rand()&0xfff;
	}

	/************* object->spriteblit[3] *************/

	// seems to be more complex than just transparency
	uint32_t blit2_unused = object->spriteblit[2]&0xff80f800;
	uint32_t b2altpenmask = (object->spriteblit[2] & 0x007f0000)>>16;
	uint32_t b2colorNumber = (object->spriteblit[2] & 0x000007ff);

	if (b2colorNumber != b1colorNumber)
	{
	//  b1colorNumber = machine().rand()&0xfff;
	}

//  if(b1colorNumber > 0x60 || b2colorNumber)
//      printf("%08x %08x\n",b1colorNumber,b2colorNumber);


	if (blit2_unused!=0) printf("blit1 unknown bits set %08x\n", object->spriteblit[2]);
	if (b1mode)
	{
		if (b2altpenmask != 0x7f) printf("b1mode 1, b2altpenmask!=0x7f\n");
	}
	else
	{
		// 0x01/0x02 trips in rare cases (start of one of the attract levels) maybe this is some kind of alpha instead?
		if ((b2altpenmask != 0x00) && (b2altpenmask != 0x01) && (b2altpenmask != 0x02)) printf("b1mode 0, b2altpenmask!=0x00,0x01 or 0x02 (is %02x)\n", b2altpenmask);
	}
		// 00??0uuu
		// ?? seems to be 00 or 7f, set depending on b1mode
		// uuu, at least 11 bits used, maybe 12 usually the same as blit1_unused? leftover?



	/************* object->spriteblit[4] *************/

	uint32_t blit4_unused = object->spriteblit[4] & 0xf8fefeee;
	uint16_t blit4blendlevel = (object->spriteblit[4] & 0x07000000)>>24;
	uint16_t blit4blendlevelinv = blit4blendlevel^0x7;
	blit4blendlevel+=1; // make our maths easier later (not sure about accuracy)
	//object->zpri = 7-blit4;
	// unknown bits in blit word 4 -  00000010 - australia (and various other times)

	uint32_t blit_flipx = object->spriteblit[4] & 0x00000001;
	uint32_t blit_flipy = (object->spriteblit[4] & 0x00000100)>>8;
	uint32_t blit_rotate = (object->spriteblit[4] & 0x00010000)>>16;
	//uint32_t b4_unk = object->spriteblit[4] & 0x00000010;

	if (blit4_unused) printf("unknown bits in blit word %d -  %08x\n", 4, blit4_unused);

	// ---- -111 ---- ---r ---- ---y ---z ---x
	// 1 = used bits? (unknown purpose.. might be object colour mode)
	// x = x-flip
	// y = y-flip
	// r = rotate 90 degrees
	// z = ??? set very occasionally, often at the start of stages, might relate to the clipping done at the same time?

	// this might affect blending logic / amount when 0x8000 palette bit is set, and maybe even z-behavior
	// 7 = road, player bike etc.? possibly a 'do not blend' (solid) flag of sorts because the road has 0x8000 palette bit set...
	// 6 = smoke frome player bike, waves on west indies, waterfalls on niagra, occasional bits of road?
	// 5 = clouds
	// 4 = shadows and some HUD elements, occassional bit of road, some firework effects, all bg elements in the coolriders of coolriders (including reaper)
	// 3 = front coloured jets
	// 2 = middle coloured jets
	// 1 = last few coloured trails from the jets at the start, occassional bit of road
	// 0 = some HUD elements, title screen, tunnels? road during coolriders of.. (this doesn't have 0x8000 set)


	// note the road always has 0x8000 bit set in the palette.  I *think* this is because they do a gradual blend of some kind between the road types
	//  see the number of transitional road bits which have various values above set

	if (blit4blendlevel==object->state.m_debug_randompal)
	{
		b1colorNumber = object->state.machine().rand()&0xfff;
	}


	/************* object->spriteblit[6] *************/

	uint16_t vCellCount = (object->spriteblit[6] & 0x03ff0000) >> 16;
	uint16_t hCellCount = (object->spriteblit[6] & 0x000003ff);

	/************* object->spriteblit[7] *************/

	uint16_t vOrigin = (object->spriteblit[7] & 0x00030000) >> 16;
	uint16_t hOrigin = (object->spriteblit[7] & 0x00000003);
	uint16_t OriginUnused = (object->spriteblit[7] & 0xfffcfffc);

	if (blit5_unused) printf("unknown bits in blit word %d -  %08x\n", 7, OriginUnused);

	//printf("%04x %04x\n", vOrigin, hOrigin);

	/************* object->spriteblit[8] *************/

	uint16_t vZoom = (object->spriteblit[8] & 0xffff0000) >> 16;
	uint16_t hZoom = (object->spriteblit[8] & 0x0000ffff);

	// if we have no vertical zoom value there's no point in going any further
	// because there are no known vertical indirect modes
	if (!vZoom)
		return nullptr;

	/************* object->spriteblit[9] *************/

	int vPosition = (object->spriteblit[9] & 0xffff0000) >> 16;
	int hPositionx = (object->spriteblit[9] & 0x0000ffff);

	if (hPositionx & 0x8000) hPositionx -= 0x10000;
	if (vPosition & 0x8000) vPosition -= 0x10000;

	/************* object->spriteblit[10] *************/

	// pointer to per-line zoom and scroll data for sprites
	//uint32_t blit10 = 0; // we've cached the data here already

	/************* object->spriteblit[11] *************/

	//uint32_t textlookup = 0; // we've cached the data here already

	/*
	    sample data from attract mode 'filmstrip'

	    you can see it's screen regions at least, gets enabled in certain game situations too
	    interestingly there is a bit to determine the screen number this applies to, even if that should already be implied from m_blitterMode

	    screen 1 clipping(?)

	    unknown sprite list type 1 - 00000001 003f00f0 027801f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 03e001f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 000700e3 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 010c01f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000

	    unknown sprite list type 1 - 00000001 003f00f0 027401f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 03dc01f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 000700df 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 010801f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000

	    unknown sprite list type 1 - 00000001 003f00f0 027001f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 03d801f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 000700db 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 010401f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000

	    unknown sprite list type 1 - 00000001 003f00f0 019c01f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 030401f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 00070007 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 0030016f 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000

	    screen 2 clipping

	    unknown sprite list type 1 - 00000001 003f00f0 039803f7 00000207 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 050003f7 00000207 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 02070203 00000207 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 022c036b 00000207 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000

	    unknown sprite list type 1 - 00000001 003f00f0 039403f7 00000207 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 04fc03f7 00000207 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 020701ff 00000207 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 02280367 00000207 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000

	    unknown sprite list type 1 - 00000001 003f00f0 039003f7 00000207 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 04f803f7 00000207 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 020701fb 00000207 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 003f00f0 02240363 00000207 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000

	  NOTE, we only copy across [1] [2] and [3]   as [0] [1] and [2]
	    the 3rd dword seems to be some kind of offset, it's 0x207 on the 2nd screen, and the actual x-clip rects are also higher on that screen?

	    note this is practically the same format as the sysh1_fb_data_w commands..



	    between stages (screen 1)
	    unknown sprite list type 1 - 00000001 000000a4 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000a4 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000a3 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000a4 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000a4 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000a5 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000a6 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000a8 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000a9 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000ab 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000ac 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000ae 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000af 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000b1 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000b2 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000b3 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000b5 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000b6 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000b8 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000b9 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000ba 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000bc 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000bd 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000be 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000c0 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000c1 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000c2 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000c4 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000c5 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000c6 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000c8 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000c9 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000cb 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000cc 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000ce 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000cf 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000d1 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000d1 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000d2 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000d3 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000d4 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000d4 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000d5 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000d5 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000d6 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000d7 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000d7 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000d8 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000d9 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000d9 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000db 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000db 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000dc 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000dd 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000dd 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000de 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000e0 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000e1 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000e1 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000e2 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000e3 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000e3 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000e4 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000e6 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000e6 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000e7 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000e8 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000ea 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000eb 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000ed 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000ee 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000f0 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000f1 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000f3 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000f4 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000f6 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000f7 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000f9 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000fa 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000fc 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000fe 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000fe 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000fd 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000fd 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000fd 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000fc 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000fd 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000fd 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000fd 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000fd 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000fc 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000fa 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000fa 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000f8 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000f6 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000f4 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000f3 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000f1 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000ee 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000eb 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000e9 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000e7 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000e4 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000e1 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000dd 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000da 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000d7 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 000000d5 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000

	ending

	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	    unknown sprite list type 1 - 00000001 0000017f 000701f7 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000

	romania
	    unknown sprite list type 1 - 00000001 00000001 00000001 00000007 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	*/

	// how does this really work? there's more to it
	// used in film strip attract mode, and between stages
	if (object->clipvals[2] & 0x0000007) // clearly this isn't the enable flag, probably just enabled until next frame / it gets disabled
	{
		// these go negative when things scroll off the left in attract mode
		int16_t minx = ((object->clipvals[1]&0xffff0000)>>16) - (((object->clipvals[2]&0x0000ffff)>>0));
		int16_t maxx = ((object->clipvals[1]&0x0000ffff)>>0)  - (((object->clipvals[2]&0x0000ffff)>>0));
		int16_t miny = ((object->clipvals[0]&0xffff0000)>>16); // maybe subtract the top 16 bits of clipvals[2], but not used?
		int16_t maxy = ((object->clipvals[0]&0x0000ffff)>>0);


		clipminX = minx -1;
		if (clipminX<CLIPMINX_FULL) clipminX = CLIPMINX_FULL;
		if (clipminX>CLIPMAXX_FULL) clipminX = CLIPMAXX_FULL;
		clipmaxX = maxx;
		if (clipmaxX<CLIPMINX_FULL) clipmaxX = CLIPMINX_FULL;
		if (clipmaxX>CLIPMAXX_FULL) clipmaxX = CLIPMAXX_FULL;
		if (clipminX>clipmaxX) clipminX = clipmaxX;

		clipminY = miny -1;
		if (clipminY<CLIPMINY_FULL) clipminY = CLIPMINY_FULL;
		if (clipminY>CLIPMAXY_FULL) clipminY = CLIPMAXY_FULL;
		clipmaxY = maxy;
		if (clipmaxY<CLIPMINY_FULL) clipmaxY = CLIPMINY_FULL;
		if (clipmaxY>CLIPMAXY_FULL) clipmaxY = CLIPMAXY_FULL;
		if (clipminY>clipmaxY) clipminY = clipmaxY;


		//b1colorNumber = object->state.machine().rand()&0xfff;
	}

	/* DRAW */
	uint16_t used_hCellCount = hCellCount;
	uint16_t used_vCellCount = vCellCount;
	uint16_t used_flipx = blit_flipx;
	uint16_t used_flipy = blit_flipy;

	if (blit_rotate)
	{
		used_hCellCount = vCellCount;
		used_vCellCount = hCellCount;
		used_flipx = blit_flipy;
		used_flipy = blit_flipx;
		// do the zoom params rotate?
	}

	// SPRITES / BLITS

	// for text objects this is an address containing the 8-bit tile numbers to use for ASCII text
	// I guess the tiles are decoded by a DMA operation earlier, from the compressed ROM?

	// we also use this to trigger the actual draw operation



	int size = used_hCellCount * used_vCellCount;

	uint16_t* tempshape;
	int screen = object->screen;
	int use_object = 0;

	if (!indirect_tile_enable && size < DECODECACHE_NUMSPRITETILES)
	{
		int found = -1;

		for (int k=0;k<DECODECACHE_NUMOBJECTCACHES;k++)
		{
			if(((object->state.m_decode[screen].objcache[k].lastromoffset == b3romoffset)) &&
				((object->state.m_decode[screen].objcache[k].lastused_flipx == used_flipx)) &&
				((object->state.m_decode[screen].objcache[k].lastused_flipy == used_flipy)) &&
				((object->state.m_decode[screen].objcache[k].lastblit_rotate == blit_rotate)) &&
				((object->state.m_decode[screen].objcache[k].lastb1mode == b1mode)) &&
				((object->state.m_decode[screen].objcache[k].lastb1colorNumber == b1colorNumber)) &&
				((object->state.m_decode[screen].objcache[k].lastb2colorNumber == b2colorNumber)) &&
				((object->state.m_decode[screen].objcache[k].lastused_hCellCount == used_hCellCount)) &&
				((object->state.m_decode[screen].objcache[k].lastused_vCellCount == used_vCellCount)) &&
				((object->state.m_decode[screen].objcache[k].lastb2altpenmask == b2altpenmask)))
			{
				found = k;
				break;
			}
		}

		if (found != -1)
		{
			object->state.m_decode[screen].objcache[found].repeatcount++;
			use_object = found;
		}
		else
		{
			use_object = object->state.m_decode[screen].current_object;

			// dirty the cache
			for (int i=0;i<DECODECACHE_NUMSPRITETILES;i++)
				object->state.m_decode[screen].objcache[use_object].tiles[i].tempshape_multi_decoded = false;

			object->state.m_decode[screen].objcache[use_object].lastromoffset = b3romoffset;
			object->state.m_decode[screen].objcache[use_object].lastused_flipx = used_flipx;
			object->state.m_decode[screen].objcache[use_object].lastused_flipy = used_flipy;
			object->state.m_decode[screen].objcache[use_object].lastblit_rotate = blit_rotate;
			object->state.m_decode[screen].objcache[use_object].lastb1mode = b1mode;
			object->state.m_decode[screen].objcache[use_object].lastb1colorNumber = b1colorNumber;
			object->state.m_decode[screen].objcache[use_object].lastb2colorNumber = b2colorNumber;
			object->state.m_decode[screen].objcache[use_object].lastused_hCellCount = used_hCellCount;
			object->state.m_decode[screen].objcache[use_object].lastused_vCellCount = used_vCellCount;
			object->state.m_decode[screen].objcache[use_object].lastb2altpenmask = b2altpenmask;
			object->state.m_decode[screen].objcache[use_object].repeatcount = 0;

			object->state.m_decode[screen].current_object++;
			if (object->state.m_decode[screen].current_object >= DECODECACHE_NUMOBJECTCACHES)
				object->state.m_decode[screen].current_object = 0;
		}
	}




	int sizey = used_vCellCount * 16 * vZoom;

	vPosition *= 0x40;

	switch (vOrigin & 3)
	{
	case 0:
		// top
		break;
	case 1:
		vPosition -= sizey / 2 ;
		// middle?
		break;
	case 2:
		vPosition -= sizey;
		// bottom?
		break;
	case 3:
		// invalid?
		break;
	}



	// Splat some sprites
	for (int v = 0; v < used_vCellCount; v++)
	{
		const int pixelOffsetY = ((vPosition) + (v* 16 * vZoom)) / 0x40;
		const int pixelOffsetnextY = ((vPosition) + ((v+1)* 16 * vZoom)) / 0x40;

		int blockhigh = pixelOffsetnextY - pixelOffsetY;

		if (pixelOffsetY+blockhigh<0)
			continue;



		if (pixelOffsetY>clipmaxY)
		{
			v = used_vCellCount;
			continue;
		}

		// I don't know, the Rainbow is a non-zoomed sprite and won't link unless you move one half
		//  (fixed in the hOrigin handling instead)
		//if (used_flipx)
		//  hPositionx -= 1;

		int hPosition = 0;



		if (!indirect_zoom_enable)
		{
			int sizex = used_hCellCount * 16 * hZoom;

			hPosition = hPositionx * 0x40;

			switch (hOrigin & 3)
			{
			case 0:
				// left
				break;
			case 1:
				hPosition -= sizex / 2;
				// middle?
				break;
			case 2:
				hPosition -= sizex-1;
				// right?
				break;
			case 3:
				// invalid?
				break;
			}
		}

		uint32_t lastSpriteNumber = 0xffffffff;
		uint16_t blankcount = 0;
		int color_offs = (object->colbase + (b1colorNumber & 0x7ff))*0x40 * 5; /* yes, * 5 */
		int color_offs2 = (object->colbase + (b2colorNumber & 0x7ff))*0x40 * 5;
		for (int h = 0; h < used_hCellCount; h++)
		{
			int current_decoded = false;

			if (!indirect_tile_enable && size < DECODECACHE_NUMSPRITETILES)
			{
				tempshape = object->state.m_decode[screen].objcache[use_object].tiles[v*used_hCellCount + h].tempshape_multi;
				current_decoded = object->state.m_decode[screen].objcache[use_object].tiles[v*used_hCellCount + h].tempshape_multi_decoded;
				/*
				if (object->screen==0)
				{
				    if (current_decoded) printf("setting temp shape to %04x tile is marked as decoded %08x \n", v*used_hCellCount + h, ((uint64_t)(void*)tempshape)&0xffffffff);
				    else printf("setting temp shape to %04x tile is marked as NOT decoded %08x \n", v*used_hCellCount + h, ((uint64_t)(void*)tempshape)&0xffffffff);
				}
				*/
			}
			else
			{
				//if (object->screen==0) printf("using base tempshape\n");
				tempshape = object->state.m_decode[screen].tempshape;
			}




			uint32_t incy = 0x8000000 / vZoom;

			// DEBUG: Draw 16x16 block

			if (indirect_zoom_enable)
			{
				GET_SPRITE_NUMBER
				CHECK_DECODE


				if (blit_rotate)
				{
					#define GET_PIX GET_PIX_ROTATED
					YXLOOP
					#undef GET_PIX
				}
				else // no rotate
				{
					#define GET_PIX GET_PIX_NORMAL
					YXLOOP
					#undef GET_PIX

				}
			}
			else  // no indirect zoom
			{
				if (!hZoom)
				{
					// abort, but make sure we clean up
					goto end;
				}

				if ((hZoom==0x40) && (vZoom==0x40)) // non-zoomed
				{
					const int pixelOffsetX = ((hPosition/0x40) + (h* 16));

					if (pixelOffsetX+16 < clipminX)
						continue;

					if (pixelOffsetX>clipmaxX)
						continue;

					GET_SPRITE_NUMBER
					CHECK_DECODE

					if (pixelOffsetX>=clipminX && pixelOffsetX+16<clipmaxX)
					{
						if (blit_rotate)
						{
							#define DO_XCLIP DO_XCLIP_NONE
							#define GET_PIX GET_PIX_ROTATED
							YXLOOP_NO_ZOOM
							#undef GET_PIX
							#undef DO_XCLIP
						}
						else // no rotate
						{
							#define DO_XCLIP DO_XCLIP_NONE
							#define GET_PIX GET_PIX_NORMAL
							YXLOOP_NO_ZOOM
							#undef GET_PIX
							#undef DO_XCLIP
						}
					}
					else
					{
						if (blit_rotate)
						{
							#define DO_XCLIP DO_XCLIP_REAL
							#define GET_PIX GET_PIX_ROTATED
							YXLOOP_NO_ZOOM
							#undef GET_PIX
							#undef DO_XCLIP
						}
						else // no rotate
						{
							#define DO_XCLIP DO_XCLIP_REAL
							#define GET_PIX GET_PIX_NORMAL
							YXLOOP_NO_ZOOM
							#undef GET_PIX
							#undef DO_XCLIP
						}
					}
				}
				else // zoomed
				{
					const int pixelOffsetX = ((hPosition) + (h* 16 * hZoom)) / 0x40;
					const int pixelOffsetnextX = ((hPosition) + ((h+1)* 16 * hZoom)) / 0x40;

					int blockwide = pixelOffsetnextX-pixelOffsetX;
					uint32_t incx = 0x8000000 / (object->spriteblit[8] & 0x0000ffff);

					if (pixelOffsetX+blockwide < clipminX)
						continue;

					if (pixelOffsetX>clipmaxX)
						continue;

					GET_SPRITE_NUMBER
					CHECK_DECODE

					if (pixelOffsetX>=clipminX && pixelOffsetX+blockwide<clipmaxX)
					{
						if (blit_rotate)
						{
							#define DO_XCLIP DO_XCLIP_NONE
							#define GET_PIX GET_PIX_ROTATED
							YXLOOP_NO_LINEZOOM
							#undef GET_PIX
							#undef DO_XCLIP
						}
						else // no rotate
						{
							#define DO_XCLIP DO_XCLIP_NONE
							#define GET_PIX GET_PIX_NORMAL
							YXLOOP_NO_LINEZOOM
							#undef GET_PIX
							#undef DO_XCLIP
						}
					}
					else
					{
						if (blit_rotate)
						{
							#define DO_XCLIP DO_XCLIP_REAL
							#define GET_PIX GET_PIX_ROTATED
							YXLOOP_NO_LINEZOOM
							#undef GET_PIX
							#undef DO_XCLIP
						}
						else // no rotate
						{
							#define DO_XCLIP DO_XCLIP_REAL
							#define GET_PIX GET_PIX_NORMAL
							YXLOOP_NO_LINEZOOM
							#undef GET_PIX
							#undef DO_XCLIP
						}
					}
				} // end zoomed
			} // end no indirect zoom
		}
	}



	end:

	return nullptr;

}



/* This is a RLE-based sprite blitter (US Patent #6,141,122), very unusual from Sega... */
void coolridr_state::blit_current_sprite(address_space &space)
{
//  const pen_t *clut = &m_palette->pen(0);

	// Serialized 32-bit words in order of appearance:
	//  0: 00000000 - unknown, 0x00000000 or 0x00000001, 0 seems to be regular sprite, 1 seems to change meaning of below, possible clip area?
	//  1: 00010000 - unknown, color mode? (7bpp select?) set on player bike object
	//  1: 00000xxx - "Color Number" (all bits or just lower 16/8?)
	//  2: 007f0000 - unknown, transpen? set to 0x7f whenever the 'color mode' bit in (1) is set, otherwise 0
	//  2: 00000xxx - unknown, usually a copy of color number, leftover?
	//  3: 001fffff - offset to compressed data? (it's 0 on text objects tho, but maybe the ascii tiles are a special decode to go with the indirect mode)
	//  4: 07000000 - unknown (draw mode?)
	//  4: 00010000 - unknown (set on a few object)
	//  4: 00000100 - y-flip?
	//  4: 00000001 - x-flip?
	//  5: 00010000 - enable indirect text tile lookup
	//  5: 00000001 - enable line-zoom(?) lookup (road)
	//  6: vvvv---- - "Vertical Cell Count"
	//  6: ----hhhh - "Horizontal Cell Count"
	//  7: 00030003 - "Vertical|Horizontal Origin point"
	//  8: 00ff00ff - "Vertical|Horizontal Zoom Ratios"
	//  9: xxxx---- - "Display Vertical Position"
	//  9: ----yyyy - "Display Horizontal Position"
	// 10: 00000000 - unknown : always seems to be zero - NO, for some things (not text) it's also a reference to 3f40000 region like #11
	// 11: ........ - indirect tile mode ram address (used for text)


	// first parse the bits

	/************* m_spriteblit[0] *************/

	// set to 0x00000001 on some objects during the 'film strip' part of attract, otherwise 0
	// those objects don't seem visible anyway so might have some special meaning
	// this is also set at times during the game
	//
	// the sprites with 1 set appear to have 0x00000000 in everything after the 4th write (blit4 and above)
	// so likely have some other meaning and are NOT regular sprite data
	uint32_t blit0 = m_spriteblit[0];

	if (blit0==0)
	{
		// normal
	}
	else if (blit0==1)
	{
		if (m_blitterMode&0x80)
		{
			// HACK...
			// the end sequences do this.. 4f / af cliprects
			//(mode 4f) unknown sprite list type 1 - 0054 0105 0059 0198 (00540105 0060019f 00000007)
			//(mode 50) unknown sprite list type 1 - 0000 017f 0000 01f0 (0000017f 000701f7 00000007)
			// THEN writes the sprite that needs to be clipped(!)
			// this ends up disabling the cliprect before the sprite that actually needs it
			//
			// all other cliprect blits seem to be written after mode 50, so this hack just prevents
			// writes with a higher value than the last one from taking effect, fixing the ending...
			// especially noticeable with 'LoveMachine'
			// (mode 4f) unknown sprite list type 1 - 0054 0105 0059 0198 (00540105 0060019f 00000007)
			// (mode 50) unknown sprite list type 1 - 0000 017f 0000 01f0 (0000017f 000701f7 00000007)
			// said ending also ends up drawing a red bar under the image, which isn't clipped out and
			// is sent before the clip window anyway(?)...

			// lightning also needs clipping (doesn't have the red bar problem)
			// (mode 4f) unknown sprite list type 1 - 0054 0105 0059 0198 (00540105 0060019f 00000007)

			if (m_clipblitterMode[1] >= m_blitterMode)
			{
				m_clipvals[1][0] = m_spriteblit[1];
				m_clipvals[1][1] = m_spriteblit[2];
				m_clipvals[1][2] = m_spriteblit[3];
				m_clipblitterMode[1] = m_blitterMode;
			}

		}
		else
		{
			if (m_clipblitterMode[0] >= m_blitterMode)
			{
				m_clipvals[0][0] = m_spriteblit[1];
				m_clipvals[0][1] = m_spriteblit[2];
				m_clipvals[0][2] = m_spriteblit[3];
				m_clipblitterMode[0] = m_blitterMode;
			}
			//printf("(mode %02x) unknown sprite list type 1 - %04x %04x %04x %04x (%08x %08x %08x)\n", m_blitterMode, (m_spriteblit[1]&0xffff0000)>>16,(m_spriteblit[1]&0x0000ffff)>>0, ((m_spriteblit[2]&0xffff0000)>>16)-((m_spriteblit[3]&0x0000ffff)>>0),((m_spriteblit[2]&0x0000ffff)>>0)-((m_spriteblit[3]&0x0000ffff)>>0),    m_spriteblit[1],m_spriteblit[2],m_spriteblit[3]);

		}

		// abort early
		return;
	}
	else
	{
		printf("unknown blit0 value %08x\n", blit0);
		// abort early
		return;
	}

	std::unique_ptr<cool_render_object> testobject(new cool_render_object(*this));

	// cache some values that are looked up from RAM to be safe.. alternatively we could stall the rendering if they get written to, but they're a direct memory pointer..
	int test_indirect_tile_enable = (m_spriteblit[5] & 0x00010000)>>16;

	if (test_indirect_tile_enable)
	{
		uint32_t test_textlookup =  m_spriteblit[11];
		uint16_t test_hCellCount = (m_spriteblit[6] & 0x00003ff);
		uint16_t test_vCellCount = (m_spriteblit[6] & 0x03ff0000) >> 16;
		int bytes = test_vCellCount*test_hCellCount;
		testobject->indirect_tiles = std::make_unique<uint8_t []>(bytes);
		for (int i=0;i<bytes;i++)
		{
			testobject->indirect_tiles[i] = space.read_byte(test_textlookup + i);
		}
	}
	else
	{
		testobject->indirect_tiles = nullptr;
	}

	int test_indirect_zoom_enable = (m_spriteblit[5] & 0x00000001);
	if (test_indirect_zoom_enable)
	{
		uint32_t test_blit10 =  m_spriteblit[10];
		uint16_t test_vCellCount = (m_spriteblit[6] & 0x03ff0000) >> 16;
		int bytes = test_vCellCount * 4 * 16;
		testobject->indirect_zoom = std::make_unique<uint32_t []>(bytes/4);
		for (int i=0;i<bytes/4;i++)
		{
			testobject->indirect_zoom[i] = space.read_dword(test_blit10 + i*4);
		}
	}
	else
	{
		testobject->indirect_zoom = nullptr;
	}

	testobject->zpri = m_blitterAddr | m_blittype<<12;
	testobject->blittype = m_blittype;
#if 0
	osd_work_queue *queue;
#endif
	// which queue, which bitmap
	if (m_blitterMode == 0x30 || m_blitterMode == 0x40 || m_blitterMode == 0x4f || m_blitterMode == 0x50 || m_blitterMode == 0x60)
	{
		testobject->drawbitmap = &m_temp_bitmap_sprites[0];
		/* testobject->zbitmap = &m_zbuffer_bitmap[0]; */
		// pass these from the type 1 writes
		testobject->clipvals[0] = m_clipvals[0][0];
		testobject->clipvals[1] = m_clipvals[0][1];
		testobject->clipvals[2] = m_clipvals[0][2];
		testobject->screen = 0;
#if 0
		queue = m_work_queue[0];
#endif
	}
	else // 0x90, 0xa0, 0xaf, 0xb0, 0xc0
	{
		testobject->drawbitmap = &m_temp_bitmap_sprites[1];
		/* testobject->zbitmap = &m_zbuffer_bitmap[1]; */
		// pass these from the type 1 writes
		testobject->clipvals[0] = m_clipvals[1][0];
		testobject->clipvals[1] = m_clipvals[1][1];
		testobject->clipvals[2] = m_clipvals[1][2];
		testobject->screen = 1;
#if 0
		queue = m_work_queue[1];
#endif
	}

#if 0
	if (m_usethreads)
	{
		osd_work_item_queue(queue, draw_object_threaded, testobject.release(), WORK_ITEM_FLAG_AUTO_RELEASE);
	}
	else
	{
		draw_object_threaded(testobject.release(), 0);
	}
#else

	if (m_blitterMode == 0x30 || m_blitterMode == 0x40 || m_blitterMode == 0x4f || m_blitterMode == 0x50 || m_blitterMode == 0x60)
	{
		if (m_listcount[0]<1000000)
		{
			m_cool_render_object_list[0][m_listcount[0]] =  std::move(testobject);
			m_listcount[0]++;
		}
		else
		{
			popmessage("m_listcount[0] overflow!\n");
		}
	}
	else
	{
		if (m_listcount[1]<1000000)
		{
			m_cool_render_object_list[1][m_listcount[1]] =  std::move(testobject);
			m_listcount[1]++;
		}
		else
		{
			popmessage("m_listcount[1] overflow!\n");
		}
	}
#endif
}


void coolridr_state::blit_mode_w(uint32_t data)
{
	m_blitterMode = (data & 0x00ff0000) >> 16;


	if (m_blitterMode == 0xf4)
	{
		// Some sort of addressing state.
		// In the case of text, simply writes 4 characters per 32-bit word.
		// These values may be loaded into RAM somewhere as they are written.
		// The number of characters is determined by the upper-most 8 bits.
		m_textBytesToWrite = (data & 0xff000000) >> 24;
		m_textOffset = (data & 0x0000ffff);
		m_blitterSerialCount = 0;

		// this is ONLY used when there is text on the screen

		//printf("set mode %08x\n", data);


	}
	else if (m_blitterMode == 0x30 || m_blitterMode == 0x40 || m_blitterMode == 0x4f || m_blitterMode == 0x50 || m_blitterMode == 0x60
			|| m_blitterMode == 0x90 || m_blitterMode == 0xa0 || m_blitterMode == 0xaf || m_blitterMode == 0xb0 || m_blitterMode == 0xc0)
	{
		// 4f / af are used to send the clipping window during the end sequence, I don't know if the 'f' bit has any special meaning
		// we need a hack for the blit window to set by them to work at all

		// The blitter function(s).
		// After this is set a fixed count of 11 32-bit words are sent to the data register.
		// The lower word always seems to be 0x0001 and the upper byte always 0xac.
		m_blitterSerialCount = 0;

		if (m_blitterMode>=0x80)
			m_blittype = m_blitterMode - 0x90;
		else
			m_blittype = m_blitterMode - 0x30;

		m_blittype>>=4;


		m_blitterAddr = data & 0x00000fff;

		// maybe it's actually treated like RAM, and blittermode determines the nature of the write (forward inc, backward inc, no inc etc.)
		// the m_blitterAddr when used does increase by 6 each time

		// not seen this triggered
		if ((data & 0xff000000) != 0xac000000)
			printf("blitter mode set without upper bits equal 0xac000000\n");

		// i've seen this triggered once or twice
		// might be there are more z-bits, or it's just overflowing when there are too many sprites
		// also set on the 4f/af cliprect during the ending
		//if (data & 0x0000f000)
		//  printf("blitter mode with mask 0x0000f000 (%08x)\n", data & 0x0000f000);



		//if (m_blitterMode<0x80)  printf("blitter set screen %d mode %02x addr? %04x\n", (data&0x00800000)>>23, ((data & 0x00ff0000)>>16)-0x30, data & 0x00000fff);

		// form 0xacMM-xxx   ac = fixed value for this mode?  MM = modes above.  -xxx = some kind of offset? but it doesn't increment for each blit like the textOffset / paletteOffset stuff, investigate

	}
	else if (m_blitterMode == 0x10)
	{
		// Could be a full clear of VRAM?
		for(uint32_t vramAddr = 0x3f40000; vramAddr < 0x3f4ffff; vramAddr+=4)
			m_maincpu->space(AS_PROGRAM).write_dword(vramAddr, 0x00000000);

		m_blitterSerialCount = 0;
	}
	else if (m_blitterMode == 0xe0)
	{
		// uploads palettes...
		// does NOT upload the palette for the WDUD screen when set to US mode this way..
		m_blitterSerialCount = 0;
		m_textOffset = (data & 0x0000ffff);

	//  printf("set e0 %08x\n", data);

	}
	else
	{
		printf("set unknown blit mode %02x\n", m_blitterMode);
	}
}

void coolridr_state::blit_data_w(address_space &space, uint32_t data)
{
	if (m_blitterMode == 0xf4)
	{
		// Uploads a series of bytes that index into the encoded sprite table
		const size_t memOffset = 0x03f40000 + m_textOffset + m_blitterSerialCount;
		space.write_dword(memOffset, data);
		m_blitterSerialCount += 0x04;
	}
	else if (m_blitterMode == 0x30 || m_blitterMode == 0x40 || m_blitterMode == 0x4f || m_blitterMode == 0x50 || m_blitterMode == 0x60
			|| m_blitterMode == 0x90 || m_blitterMode == 0xa0 || m_blitterMode == 0xaf || m_blitterMode == 0xb0 || m_blitterMode == 0xc0)
	{
		// Serialized counts
		if (m_blitterSerialCount < 12)
		{
			m_spriteblit[m_blitterSerialCount] = data;
			m_blitterSerialCount++;
		}
		else
		{
			printf("more than 11 dwords (%d) in blit?\n", m_blitterSerialCount);
		}

		// use the 11th blit write also as the trigger
		if (m_blitterSerialCount == 12)
		{
			blit_current_sprite(space);
		}

	}
	// ??
	else if (m_blitterMode == 0x10) // at startup
	{
		//printf("blit mode %02x %02x %08x\n", m_blitterMode, m_blitterSerialCount,  data);
		m_blitterSerialCount++;

		//blit mode 10 00 00000003
		//blit mode 10 01 00000002
		//blit mode 10 02 00007b20  << this is the palette base for the sprites (but not in aquastage where it gets set to 0x210 and colbase is 0?) ( m_colbase )
		//blit mode 10 00 00000002
		//blit mode 10 01 00000001
		//blit mode 10 02 00040204
		//blit mode 10 03 00000000

	}
	else if (m_blitterMode == 0xe0) // when going into game (in units of 0x10 writes)
	{
		// it writes the palette for the bgs here, with fade effects?
		//  is this the only way for the tile colours to be actually used, or does this just go to memory somewhere too?
		//printf("blit mode %02x %02x %08x\n", m_blitterMode, m_blitterSerialCount,  data);

		// maybe should write to a different address, see dma hack in other code
		const size_t memOffset = 0x3c00000 + m_textOffset + m_blitterSerialCount;
		space.write_dword(memOffset, data);
		m_blitterSerialCount += 0x04;

	}
	else
	{
		printf("unk blit mode %02x\n", m_blitterMode);
	}
}

void coolridr_state::fb_mode_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	/*
	This does the fb display/clear phases of blitter data processed in the previous frame.
	And yes, game effectively runs at 30 Hz (because data processing happens on even frames, actual display transfer happens on odd frames).
	screen 1
	8c200000 06
	00000001 07
	0000017f 07 Y range (upper start, lower end)
	000701f7 07 X range (upper start, lower end)
	00000007 07 enable?
	screen 2
	8c800000 06
	00000001 07
	0000017f 07
	020703f7 07
	00000207 07 enable plus clear?
	*/

	COMBINE_DATA(&m_blitterClearMode);

/*
    if(m_blitterClearMode != 0x8c200000 && m_blitterClearMode != 0x8c800000)
        printf("Blitter Clear used with param %08x\n",m_blitterClearMode);
*/

	m_blitterClearCount = 0;
}




void coolridr_state::fb_data_w(offs_t offset, uint32_t data)
{
	if(m_blitterClearCount == 0)
	{
		if(data != 1)
			printf("Blitter Clear Count == 0 used with param %08x\n",data);
	}
	else if(m_blitterClearCount == 1)
	{
		if(data != 0x17f)
			printf("Blitter Clear Count == 1 used with param %08x\n",data);
	}
	else if(m_blitterClearCount == 2)
	{
		/*
		if(data != 0x000701f7 && m_txt_blit[offset] != 0x020703f7)
		    printf("Blitter Clear Count == 2 used with param %08x\n",data);
		*/
	}
	else if(m_blitterClearCount == 3)
	{
		if(data != 0x00000007 && data != 0x00000207)
			printf("Blitter Clear Count == 3 used with param %08x\n",data);

		{
			const rectangle& visarea = m_screen->visible_area();

			if(m_blitterClearMode == 0x8c200000)
			{
				// wait for our sprite rendering to finish
				osd_work_queue_wait(m_work_queue[0], osd_ticks_per_second() * 100);

				// copy our old buffer to the actual screen
				copybitmap(m_screen_bitmap[0], m_temp_bitmap_sprites[0], 0, 0, 0, 0, visarea);




				//m_temp_bitmap_sprites[1].fill(0xff000000, visarea);
				// render the tilemap to the backbuffer, ready for having sprites drawn on it
				draw_bg_coolridr(m_temp_bitmap_sprites[0], visarea, 0);
				// wipe the z-buffer ready for the sprites
				/* m_zbuffer_bitmap[0].fill(0xffff, visarea); */
				// almost certainly wrong
				m_clipvals[0][0] = 0;
				m_clipvals[0][1] = 0;
				m_clipvals[0][2] = 0;
				m_clipblitterMode[0] = 0xff;

				/* bubble sort, might be something better to use instead */
				for (int pass = 0 ; pass < (m_listcount[0] - 1); pass++)
				{
					for (int elem2 = 0 ; elem2 < m_listcount[0] - pass - 1; elem2++)
					{
						if (m_cool_render_object_list[0][elem2]->zpri > m_cool_render_object_list[0][elem2+1]->zpri)
							std::swap(m_cool_render_object_list[0][elem2], m_cool_render_object_list[0][elem2+1]);
					}
				}

				for (int i=m_listcount[0]-1;i>=0;i--)
				{
					if (m_usethreads)
					{
						osd_work_item_queue(m_work_queue[0], draw_object_threaded, m_cool_render_object_list[0][i].release(), WORK_ITEM_FLAG_AUTO_RELEASE);
					}
					else
					{
						draw_object_threaded((void*)m_cool_render_object_list[0][i].release(), 0);
					}
				}

				m_listcount[0] = 0;


			}
			else if(m_blitterClearMode == 0x8c800000)
			{
				// wait for our sprite rendering to finish
				osd_work_queue_wait(m_work_queue[1], osd_ticks_per_second() * 100);

				// copy our old buffer to the actual screen
				copybitmap(m_screen_bitmap[1], m_temp_bitmap_sprites[1], 0, 0, 0, 0, visarea);




				//m_temp_bitmap_sprites[1].fill(0xff000000, visarea);
				// render the tilemap to the backbuffer, ready for having sprites drawn on it
				draw_bg_coolridr(m_temp_bitmap_sprites[1], visarea, 1);
				// wipe the z-buffer ready for the sprites
				/* m_zbuffer_bitmap[1].fill(0xffff, visarea); */
				// almost certainly wrong
				m_clipvals[1][0] = 0;
				m_clipvals[1][1] = 0;
				m_clipvals[1][2] = 0;
				m_clipblitterMode[1] = 0xff;

				/* bubble sort, might be something better to use instead */
				for (int pass = 0 ; pass < (m_listcount[1] - 1); pass++)
				{
					for (int elem2 = 0 ; elem2 < m_listcount[1] - pass - 1; elem2++)
					{
						if (m_cool_render_object_list[1][elem2]->zpri > m_cool_render_object_list[1][elem2+1]->zpri)
							std::swap(m_cool_render_object_list[1][elem2], m_cool_render_object_list[1][elem2+1]);
					}
				}

				for (int i=m_listcount[1]-1;i>=0;i--)
				{
					if (m_usethreads)
					{
						osd_work_item_queue(m_work_queue[1], draw_object_threaded, m_cool_render_object_list[1][i].release(), WORK_ITEM_FLAG_AUTO_RELEASE);
					}
					else
					{
						draw_object_threaded((void*)m_cool_render_object_list[1][i].release(), 0);
					}
				}

				m_listcount[1] = 0;

			}

			//printf("frame\n");
		}
	}
	else
	{
		printf("Blitter Clear Count == %02x used with param %08x\n",m_blitterClearCount,m_txt_blit[offset]);
	}

	m_blitterClearCount++;
}

uint32_t coolridr_state::unk_blit_r(offs_t offset)
{
//  if(offset == 0x0c/4) // TODO

	return m_txt_blit[offset];
}


void coolridr_state::unk_blit_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_txt_blit[offset]);

	switch(offset)
	{
		default:
		{
			printf("sysh1_unk_blit_w unhandled offset %04x %08x %08x\n", offset, data, mem_mask);
		}
		break;

		case 0x01:
		{
			// writes on startup
			// sysh1_unk_blit_w unhandled offset 0001 01010101 ffffffff
		}
		break;

		case 0x02:
		{
			// writes 3d0dxxxx / 3d0exxxx before a level start.. offset for a transfer read at 0x400000c, stored in work RAM H

			//printf("sysh1_unk_blit_w unhandled offset %04x %08x %08x\n", offset, data, mem_mask);


		}
		break;

	}
}



void coolridr_state::dma_transfer( address_space &space, uint16_t dma_index )
{
	uint32_t src = 0,dst = 0,size = 0;
	uint8_t end_dma_mark;
	uint8_t cmd;

	end_dma_mark = 0;

	do{
		cmd = (m_framebuffer_vram[(0+dma_index)/4] & 0xfc000000) >> 24;

		switch(cmd)
		{
			case 0x00: /* end of list marker */
				//printf("end of list reached\n");
				end_dma_mark = 1;
				break;
			case 0xc0: /* to internal buffer VRAM */
				src = (m_framebuffer_vram[(0+dma_index)/4] & 0x03ffffff);
				dst = (m_framebuffer_vram[(4+dma_index)/4]);
				size = m_framebuffer_vram[(8+dma_index)/4];
				printf("%08x %08x %04x\n",src,dst,size);

				if(dst & 0xfff00001)
					printf("unk values to %02x dst %08x\n",cmd,dst);
				dst &= 0x000ffffe;
				dst >>= 1;

				for(int i=0;i<size;i+=2)
				{
					m_vram[dst] = space.read_word(src);
					dst++;
					src+=2;
				}

				dma_index+=0xc;
				break;

			case 0xd0: /* to internal buffer PCG */
				src = (m_framebuffer_vram[(0+dma_index)/4] & 0x03ffffff);
				dst = (m_framebuffer_vram[(4+dma_index)/4]);
				size = m_framebuffer_vram[(8+dma_index)/4];
				if(dst & 0xfff00000)
					printf("unk values to %02x dst %08x\n",cmd,dst);
				dst &= 0x000fffff;

				for(int i=0;i<size;i++)
				{
					m_pcgram[dst] = space.read_byte(src);
					m_gfxdecode->gfx(m_gfx_index)->mark_dirty(dst/256);
					dst++;
					src++;
				}

				dma_index+=0xc;
				break;

			case 0xe0: /* to palette RAM */
				src = (m_framebuffer_vram[(0+dma_index)/4] & 0x03ffffff);
				dst = (m_framebuffer_vram[(4+dma_index)/4]);
				size = m_framebuffer_vram[(8+dma_index)/4];
				/* Note: there are also some reads at 0x3e00000. This tells us that the DMA thing actually mirrors at 0x3c00000 too. */
				if(dst & 0xfff00001)
					printf("unk values to %02x dst %08x\n",cmd,dst);
				dst &= 0x000ffffe;
				dst >>= 1;

				for(int i=0;i<size;i+=2)
				{
					m_palram[dst] = space.read_word(src);
					dst++;
					src+=2;
				}

				dma_index+=0xc;
				break;

				break;
			case 0x10: /* sets up look-up for tilemap video registers */
				m_vregs_address = (m_framebuffer_vram[(0+dma_index)/4] & 0x03ffffff);
				dma_index+=4;
				break;
			case 0x04: /* init - value 0x040c80d2 (unknown purpose, slave mode?) */
			case 0x20: /* screen 1 - linescroll/zoom table? (default values) */
			case 0x24: /* screen 2 / */
			case 0x50: /* screen 1 - unknown */
			case 0x54: /* screen 2 / */
				//printf("%02x %08x\n",cmd,m_framebuffer_vram[(0+dma_index)/4]);
				dma_index+=4;
				break;
			case 0x30: /* screen 1 - 0x80 at boot, then 0x808080  */
			case 0x34: /* screen 2 / */
				m_pen_fill[(cmd & 4) >> 2] = m_framebuffer_vram[(0+dma_index)/4] & 0xffffff;
				dma_index+=4;
				break;
			case 0x40: /* screen 1 - almost certainly RGB brightness (at least bits 4 - 0) */
			case 0x44: /* screen 2 / */
				m_rgb_ctrl[(cmd & 4) >> 2].setting = m_framebuffer_vram[(0+dma_index)/4] & 0xffffe0;
				m_rgb_ctrl[(cmd & 4) >> 2].gradient = m_framebuffer_vram[(0+dma_index)/4] & 0x1f;
				dma_index+=4;
				break;
			default:
				printf("%02x %08x\n",cmd,m_framebuffer_vram[(0+dma_index)/4]);
				dma_index+=4;
				break;
		}

	}while(!end_dma_mark );
}

void coolridr_state::dma_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_framebuffer_vram[offset]);

	if(offset*4 == 0x000)
	{
		/* enable */
		if((m_framebuffer_vram[offset] & 0xff000000) == 0x0f000000)
			dma_transfer(space, m_framebuffer_vram[offset] & 0xffff);
	}
}


void coolridr_state::system_h1_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rom().nopw();
	map(0x01000000, 0x01ffffff).rom().region("gfx_data", 0x0000000);

	map(0x03f40000, 0x03f4ffff).ram().share("txt_vram");//text tilemap + "lineram"
	map(0x04000000, 0x0400000f).rw(FUNC(coolridr_state::unk_blit_r), FUNC(coolridr_state::unk_blit_w)).share("txt_blit");
	map(0x04000010, 0x04000013).w(FUNC(coolridr_state::blit_mode_w));
	map(0x04000014, 0x04000017).w(FUNC(coolridr_state::blit_data_w));
	map(0x04000018, 0x0400001b).w(FUNC(coolridr_state::fb_mode_w));
	map(0x0400001c, 0x0400001f).w(FUNC(coolridr_state::fb_data_w));

	map(0x06000000, 0x060fffff).ram().share("workrah");
	map(0x20000000, 0x201fffff).rom().region("maincpu", 0);

	map(0x60000000, 0x600003ff).nopw();
}

void coolridr_state::coolridr_h1_map(address_map &map)
{
	system_h1_map(map);
	map(0x03c00000, 0x03c1ffff).mirror(0x00200000).ram().w(FUNC(coolridr_state::dma_w)).share("fb_vram"); /* mostly mapped at 0x03e00000 */

	map(0x03f00000, 0x03f0ffff).ram().share("share3"); /*Communication area RAM*/
}

void coolridr_state::aquastge_h1_map(address_map &map)
{
	system_h1_map(map);
	map(0x03c00000, 0x03c0ffff).mirror(0x00200000).ram().w(FUNC(coolridr_state::dma_w)).share("fb_vram"); /* mostly mapped at 0x03e00000 */
	map(0x03f50000, 0x03f5ffff).ram(); // video registers
	map(0x03e10000, 0x03e1ffff).ram().share("share3"); /*Communication area RAM*/
	map(0x03f00000, 0x03f0ffff).ram();  /*Communication area RAM*/
}

template<int Chip>
uint16_t coolridr_state::soundram_r(offs_t offset)
{
	return m_soundram[Chip][offset];
}

template<int Chip>
void coolridr_state::soundram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_soundram[Chip][offset]);
}


void coolridr_state::lamps_w(uint8_t data)
{
	/*
	x--- ---- P2 Music select Lamp
	-x-- ---- P1 Music select Lamp
	--x- ---- P2 Race Leader Lamp
	---x ---- P1 Race Leader Lamp
	---- x--- P2 Start Lamp
	---- -x-- P1 Start Lamp
	---- ---x (used in game?)
	*/
}


uint32_t coolridr_state::sound_dma_r(offs_t offset)
{
	if(offset == 8)
	{
		//popmessage("%02x",m_sound_data);
		/*
		Checked in irq routine
		--x- ---- second SCSP
		---x ---- first SCSP
		*/
		return m_sound_data;
	}

	if(offset == 2 || offset == 6) // DMA status
		return 0;

	printf("%08x\n",offset);

	return m_sound_dma[offset];
}

void coolridr_state::sound_dma_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	address_space &main_space = m_maincpu->space(AS_PROGRAM);
	address_space &sound_space = m_soundcpu->space(AS_PROGRAM);

	//printf("%08x %08x\n",offset*4,m_h1_unk[offset]);

	if(offset == 8)
	{
		//probably writing to upper word disables m68k, to lower word enables it
		m_soundcpu->set_input_line(INPUT_LINE_RESET, (data) ? ASSERT_LINE : CLEAR_LINE);
		return;
	}

	if(offset == 2)
	{
		if(data & 1 && (!(m_sound_dma[2] & 1))) // 0 -> 1 transition enables DMA
		{
			uint32_t src = m_sound_dma[0];
			uint32_t dst = m_sound_dma[1];
			uint32_t size = (m_sound_dma[2]>>16)*0x40;

			//printf("%08x %08x %08x %02x\n",src,dst,size,m_sound_fifo);

			for(int i = 0;i < size; i+=2)
			{
				sound_space.write_word(dst,main_space.read_word(src));
				src+=2;
				dst+=2;
			}
		}
	}

	if(offset == 6)
	{
		if(data & 1 && (!(m_sound_dma[6] & 1))) // 0 -> 1 transition enables DMA
		{
			uint32_t src = m_sound_dma[4];
			uint32_t dst = m_sound_dma[5];
			uint32_t size = (m_sound_dma[6]>>16)*0x40;

			//printf("%08x %08x %08x %02x\n",src,dst,size,m_sound_fifo);

			for(int i = 0;i < size; i+=2)
			{
				sound_space.write_word(dst,main_space.read_word(src));
				src+=2;
				dst+=2;
			}
		}
	}

	COMBINE_DATA(&m_sound_dma[offset]);
}



void coolridr_state::system_h1_submap(address_map &map)
{
	map(0x00000000, 0x0001ffff).rom(); // note: SH7032 only supports 64KB

	map(0x01000000, 0x0100ffff).ram(); //communication RAM

	map(0x03000000, 0x0307ffff).rw(FUNC(coolridr_state::soundram_r<0>), FUNC(coolridr_state::soundram_w<0>)); //.share("soundram1");
	map(0x03100000, 0x03100fff).rw("scsp1", FUNC(scsp_device::read), FUNC(scsp_device::write));
	map(0x03200000, 0x0327ffff).rw(FUNC(coolridr_state::soundram_r<1>), FUNC(coolridr_state::soundram_w<1>)); //.share("soundram2");
	map(0x03300000, 0x03300fff).rw("scsp2", FUNC(scsp_device::read), FUNC(scsp_device::write));

	map(0x04000000, 0x0400003f).rw(FUNC(coolridr_state::sound_dma_r), FUNC(coolridr_state::sound_dma_w)).share("sound_dma");
//  map(0x04200000, 0x0420003f).ram(); /* unknown */

	map(0x05000000, 0x05000fff).ram();
//  map(0x05fffe00, 0x05ffffff).rw(FUNC(coolridr_state::sh7032_r), FUNC(coolridr_state::sh7032_w)); // SH-7032H internal i/o
	map(0x06000000, 0x060001ff).ram().share("nvram"); // backup RAM
	map(0x06100000, 0x0610001f).rw("io", FUNC(sega_315_5649_device::read), FUNC(sega_315_5649_device::write)).umask32(0x00ff00ff);
	map(0x06200000, 0x06200fff).ram(); //network related?
	map(0x07ffe000, 0x07ffffff).ram(); // On-Chip RAM (actually mapped at 0x0fffe000-0x0fffffff)
}

void coolridr_state::coolridr_submap(address_map &map)
{
	system_h1_submap(map);
	map(0x05200000, 0x052001ff).ram();
	map(0x05300000, 0x0530ffff).ram().share("share3"); /*Communication area RAM*/
}

void coolridr_state::aquastge_submap(address_map &map)
{
	system_h1_submap(map);
	map(0x05200000, 0x0520ffff).ram();
	map(0x05210000, 0x0521ffff).ram().share("share3"); /*Communication area RAM*/
	map(0x05220000, 0x0537ffff).ram();
	map(0x06000200, 0x06000207).nopw(); // program bug?
}

/* TODO: what is this for, volume mixing? MIDI? */
void coolridr_state::sound_to_sh1_w(uint8_t data)
{
	m_sound_fifo = data;
}

void coolridr_state::system_h1_sound_map(address_map &map)
{
	map(0x000000, 0x07ffff).ram().share("soundram1");
	map(0x100000, 0x100fff).rw("scsp1", FUNC(scsp_device::read), FUNC(scsp_device::write));
	map(0x200000, 0x27ffff).ram().share("soundram2");
	map(0x300000, 0x300fff).rw("scsp2", FUNC(scsp_device::read), FUNC(scsp_device::write));
	map(0x800000, 0x80ffff).mirror(0x200000).ram();
	map(0x900001, 0x900001).w(FUNC(coolridr_state::sound_to_sh1_w));
}

template<int Chip>
void coolridr_state::scsp_map(address_map &map)
{
	map(0x000000, 0x07ffff).ram().share(m_soundram[Chip]);
}


static GFXDECODE_START( gfx_coolridr )
//  GFXDECODE_ENTRY( nullptr, 0, tiles16x16_layout, 0, 0x100 )
GFXDECODE_END


static INPUT_PORTS_START( coolridr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )    PORT_NAME("P1 Coin")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )    PORT_NAME("P2 Coin")
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P1 Service Switch")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )   PORT_NAME("P1 Start")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )   PORT_NAME("P2 Start")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("P2 Service Switch")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Music <<") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Music >>") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Shift Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Shift Down")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Music <<") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Music >>") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Shift Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Shift Down")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(60) PORT_PLAYER(1) PORT_NAME("P1 Handle Bar")

	PORT_START("AN1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL )  PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(60) PORT_PLAYER(1) PORT_NAME("P1 Throttle")   PORT_REVERSE

	PORT_START("AN2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(60) PORT_PLAYER(1) PORT_NAME("P1 Brake")      PORT_REVERSE

	PORT_START("AN4")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(60) PORT_PLAYER(2) PORT_NAME("P2 Handle Bar")

	PORT_START("AN5")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL )  PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(60) PORT_PLAYER(2) PORT_NAME("P2 Throttle")   PORT_REVERSE

	PORT_START("AN6")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(60) PORT_PLAYER(2) PORT_NAME("P2 Brake")      PORT_REVERSE

	// driver debug
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "Use Threading Code" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( aquastge )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )    PORT_NAME("P1 Coin")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )    PORT_NAME("P2 Coin")
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P1 Service Switch")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )   PORT_NAME("P1 Start")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )   PORT_NAME("P2 Start")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("P2 Service Switch")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )   PORT_PLAYER(1)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	// driver debug
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "Use Threading Code" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END


// IRQs 4 & 6 are valid on SH-2
TIMER_DEVICE_CALLBACK_MEMBER(coolridr_state::interrupt_main)
{
	int scanline = param;

	if(scanline == 384)
		m_maincpu->set_input_line(4, HOLD_LINE);

	if(scanline == 0)
		m_maincpu->set_input_line(6, HOLD_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(coolridr_state::interrupt_sub)
{
	int scanline = param;

	/* 0xa: reads from 0x4000000 (sound irq) */
	/* 0xc: reads from inputs (so presumably V-Blank) */
	/* 0xe: tries to r/w to 0x62***** area (network irq?) */

	if(scanline == 384)
		m_subcpu->set_input_line(0xc, HOLD_LINE);
}




#define READ_COMPRESSED_ROM(chip) \
	m_compressedgfx[(chip)*0x400000 + romoffset] << 8 | m_compressedgfx[(chip)*0x0400000 + romoffset +1];
// this helps you feth the 20bit words from an address in the compressed data
uint32_t coolridr_state::get_20bit_data(uint32_t romoffset, int _20bitwordnum)
{
	uint16_t testvalue, testvalue2;

	int temp = _20bitwordnum & 3;
	int inc = 0;
	if (_20bitwordnum&4) inc = 5;

	romoffset += (_20bitwordnum>>3)*2;

	if (temp==0)
	{
		testvalue  = READ_COMPRESSED_ROM(0+inc);
		testvalue2 = READ_COMPRESSED_ROM(1+inc);
		return (testvalue << 4) | (testvalue2 & 0xf000) >> 12;
	}
	else if (temp==1)
	{
		testvalue  = READ_COMPRESSED_ROM(1+inc);
		testvalue2 = READ_COMPRESSED_ROM(2+inc);
		return ((testvalue & 0x0fff) << 8) | (testvalue2 & 0xff00) >> 8;
	}
	else if (temp==2)
	{
		testvalue  = READ_COMPRESSED_ROM(2+inc);
		testvalue2 = READ_COMPRESSED_ROM(3+inc);
		return ((testvalue & 0x00ff) << 12) | (testvalue2 & 0xfff0) >> 4;
	}
	else // temp == 3
	{
		testvalue  = READ_COMPRESSED_ROM(3+inc);
		testvalue2 = READ_COMPRESSED_ROM(4+inc);
		return ((testvalue & 0x000f) << 16) | (testvalue2);
	}

}

uint16_t coolridr_state::get_10bit_data(uint32_t romoffset, int _10bitwordnum)
{
	uint32_t data = get_20bit_data(romoffset, _10bitwordnum>>1);
	if (_10bitwordnum&1) return data & 0x3ff;
	else return (data>>10) & 0x3ff;
}

void coolridr_state::machine_start()
{
	size_t  size    = m_compressedgfx.length();

	// we're expanding 10bit packed data to 16bits(10 used)
	m_expanded_10bit_gfx = std::make_unique<uint16_t[]>(((size/10)*16)/2);

	for (int i=0;i<(0x800000*8)/2;i++)
	{
		m_expanded_10bit_gfx[i] = get_10bit_data( 0, i);
	}

	// do a rearranged version too with just the 16-bit words in a different order, palettes seem to
	// be referenced this way?!
	m_rearranged_16bit_gfx = std::make_unique<uint16_t[]>(size/2);

	uint16_t* compressed = (uint16_t*)&m_compressedgfx[0];
	int count = 0;
	for (int i=0;i<size/2/10;i++)
	{
		m_rearranged_16bit_gfx[count+0] = swapendian_int16(compressed[i+((0x0400000/2)*0)]);
		m_rearranged_16bit_gfx[count+1] = swapendian_int16(compressed[i+((0x0400000/2)*1)]);
		m_rearranged_16bit_gfx[count+2] = swapendian_int16(compressed[i+((0x0400000/2)*2)]);
		m_rearranged_16bit_gfx[count+3] = swapendian_int16(compressed[i+((0x0400000/2)*3)]);
		m_rearranged_16bit_gfx[count+4] = swapendian_int16(compressed[i+((0x0400000/2)*4)]);
		m_rearranged_16bit_gfx[count+5] = swapendian_int16(compressed[i+((0x0400000/2)*5)]);
		m_rearranged_16bit_gfx[count+6] = swapendian_int16(compressed[i+((0x0400000/2)*6)]);
		m_rearranged_16bit_gfx[count+7] = swapendian_int16(compressed[i+((0x0400000/2)*7)]);
		m_rearranged_16bit_gfx[count+8] = swapendian_int16(compressed[i+((0x0400000/2)*8)]);
		m_rearranged_16bit_gfx[count+9] = swapendian_int16(compressed[i+((0x0400000/2)*9)]);
		count+=10;
	}


	if (0)
	{
		auto filename = "expanded_" + std::string(machine().system().name) + "_gfx";
		auto fp = fopen(filename.c_str(), "w+b");
		if (fp)
		{
			for (int i=0;i<(0x800000*8);i++)
			{
				fwrite((uint8_t*)m_expanded_10bit_gfx.get()+(i^1), 1, 1, fp);
			}
			fclose(fp);
		}
	}

	m_work_queue[0] = osd_work_queue_alloc(WORK_QUEUE_FLAG_HIGH_FREQ);
	m_work_queue[1] = osd_work_queue_alloc(WORK_QUEUE_FLAG_HIGH_FREQ);
	m_decode[0].current_object = 0;
	m_decode[1].current_object = 0;

	save_item(NAME(m_sound_data));
	save_item(NAME(m_sound_fifo));
}

void coolridr_state::machine_reset()
{
	m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_usethreads = m_io_config->read()&1;
}

void coolridr_state::scsp_irq(offs_t offset, uint8_t data)
{
	m_soundcpu->set_input_line(offset, data);
}

void coolridr_state::scsp1_to_sh1_irq(int state)
{
	m_subcpu->set_input_line(0xe, (state) ? ASSERT_LINE : CLEAR_LINE);
	if(state)
		m_sound_data |= 0x10;
	else
		m_sound_data &= ~0x10;
}

void coolridr_state::scsp2_to_sh1_irq(int state)
{
	m_subcpu->set_input_line(0xe, (state) ? ASSERT_LINE : CLEAR_LINE);
	if(state)
		m_sound_data |= 0x20;
	else
		m_sound_data &= ~0x20;
}


void coolridr_state::coolridr(machine_config &config)
{
	SH7604(config, m_maincpu, XTAL(28'000'000)); // 28 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &coolridr_state::coolridr_h1_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(coolridr_state::interrupt_main), "screen", 0, 1);

	M68000(config, m_soundcpu, XTAL(32'000'000)/2); // 16 MHz
	m_soundcpu->set_addrmap(AS_PROGRAM, &coolridr_state::system_h1_sound_map);

	SH7032(config, m_subcpu, XTAL(32'000'000)/2); // SH7032 HD6417032F20!! 16 MHz
	m_subcpu->set_addrmap(AS_PROGRAM, &coolridr_state::coolridr_submap);
	TIMER(config, "scantimer2").configure_scanline(FUNC(coolridr_state::interrupt_sub), "screen", 0, 1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	sega_315_5649_device &io(SEGA_315_5649(config, "io", 0));
	io.out_pb_callback().set(FUNC(coolridr_state::lamps_w));
	io.in_pc_callback().set_ioport("IN0");
	io.in_pd_callback().set_ioport("P1");
	io.in_pe_callback().set_ioport("P2");
	io.an_port_callback<0>().set_ioport("AN0");
	io.an_port_callback<1>().set_ioport("AN1");
	io.an_port_callback<2>().set_ioport("AN2");
	io.an_port_callback<4>().set_ioport("AN4");
	io.an_port_callback<5>().set_ioport("AN5");
	io.an_port_callback<6>().set_ioport("AN6");

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_coolridr);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(57); // measured at 57.0426Hz
	m_screen->set_size(640, 512);
	m_screen->set_visarea(CLIPMINX_FULL,CLIPMAXX_FULL, CLIPMINY_FULL, CLIPMAXY_FULL);
	m_screen->set_screen_update(FUNC(coolridr_state::screen_update<0>));
	m_screen->set_palette(m_palette);

	screen_device &screen2(SCREEN(config, "screen2", SCREEN_TYPE_RASTER));
	screen2.set_refresh_hz(57); // measured at 57.0426Hz
	screen2.set_size(640, 512);
	screen2.set_visarea(CLIPMINX_FULL,CLIPMAXX_FULL, CLIPMINY_FULL, CLIPMAXY_FULL);
	screen2.set_screen_update(FUNC(coolridr_state::screen_update<1>));
	screen2.set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::RGB_555);

	config.set_default_layout(layout_dualhsxs);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	scsp_device &scsp1(SCSP(config, "scsp1", XTAL(22'579'000))); // 22.579 MHz
	scsp1.set_addrmap(0, &coolridr_state::scsp_map<0>);
	scsp1.irq_cb().set(FUNC(coolridr_state::scsp_irq));
	scsp1.main_irq_cb().set(FUNC(coolridr_state::scsp1_to_sh1_irq));
	scsp1.add_route(0, "lspeaker", 1.0);
	scsp1.add_route(1, "rspeaker", 1.0);

	scsp_device &scsp2(SCSP(config, "scsp2", XTAL(22'579'000))); // 22.579 MHz
	scsp2.set_addrmap(0, &coolridr_state::scsp_map<1>);
	scsp2.main_irq_cb().set(FUNC(coolridr_state::scsp2_to_sh1_irq));
	scsp2.add_route(0, "lspeaker", 1.0);
	scsp2.add_route(1, "rspeaker", 1.0);
}

void coolridr_state::aquastge(machine_config &config)
{
	coolridr(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &coolridr_state::aquastge_h1_map);

	m_subcpu->set_addrmap(AS_PROGRAM, &coolridr_state::aquastge_submap);

	sega_315_5649_device &io(SEGA_315_5649(config.replace(), "io", 0));
	io.in_pc_callback().set_ioport("IN0");
	io.in_pd_callback().set_ioport("IN1");
}

ROM_START( coolridr )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* SH2 code */
	ROM_LOAD32_WORD_SWAP( "ep17659.30", 0x0000000, 0x080000, CRC(473027b0) SHA1(acaa212869dd79550235171b9f054e82750f74c3) )
	ROM_LOAD32_WORD_SWAP( "ep17658.29", 0x0000002, 0x080000, CRC(7ecfdfcc) SHA1(97cb3e6cf9764c8db06de12e4e958148818ef737) )
	ROM_LOAD32_WORD_SWAP( "ep17661.32", 0x0100000, 0x080000, CRC(81a7d90b) SHA1(99f8c3e75b94dd1b60455c26dc38ce08db82fe32) )
	ROM_LOAD32_WORD_SWAP( "ep17660.31", 0x0100002, 0x080000, CRC(27b7a507) SHA1(4c28b1d18d75630a73194b5d4fd166f3b647c595) )

	/* Page 12 of the service manual states that these 4 regions are tested, so I believe that they are read by the SH-2 */
	ROM_REGION32_BE( 0x1000000, "gfx_data", 0 ) /* SH2 code */
	ROM_LOAD32_WORD_SWAP( "mp17650.11", 0x0000002, 0x0200000, CRC(0ccc84a1) SHA1(65951685b0c8073f6bd1cf9959e1b4d0fc6031d8) )
	ROM_LOAD32_WORD_SWAP( "mp17651.12", 0x0000000, 0x0200000, CRC(25fd7dde) SHA1(a1c3f3d947ce20fbf61ea7ab235259be9b7d35a8) )
	ROM_LOAD32_WORD_SWAP( "mp17652.13", 0x0400002, 0x0200000, CRC(be9b4d05) SHA1(0252ba647434f69d6eacb4efc6f55e6af534c7c5) )
	ROM_LOAD32_WORD_SWAP( "mp17653.14", 0x0400000, 0x0200000, CRC(64d1406d) SHA1(779dbbf42a14a6be1de9afbae5bbb18f8f36ceb3) )
	ROM_LOAD32_WORD_SWAP( "mp17654.15", 0x0800002, 0x0200000, CRC(5dee5cba) SHA1(6e6ec8574bdd35cc27903fc45f0d4a36ce9df103) )
	ROM_LOAD32_WORD_SWAP( "mp17655.16", 0x0800000, 0x0200000, CRC(02903cf2) SHA1(16d555fda144e0f1b62b428e9158a0e8ebf7084e) )
	ROM_LOAD32_WORD_SWAP( "mp17656.17", 0x0c00002, 0x0200000, CRC(945c89e3) SHA1(8776d74f73898d948aae3c446d7c710ad0407603) )
	ROM_LOAD32_WORD_SWAP( "mp17657.18", 0x0c00000, 0x0200000, CRC(74676b1f) SHA1(b4a9003a052bde93bebfa4bef9e8dff65003c3b2) )

	ROM_REGION( 0x100000, "sub", 0 ) /* SH1 */
	ROM_LOAD16_WORD_SWAP( "ep17662.12", 0x000000, 0x020000,  CRC(50d66b1f) SHA1(f7b7f2f5b403a13b162f941c338a3e1207762a0b) )

	/* these are compressed sprite data */
	ROM_REGION( 0x2800000, "compressedgfx", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "mpr-17644.ic5", 0x0000000, 0x0400000, CRC(80199c79) SHA1(e525d8ee9f9176101629853e50cca73b02b16a38) ) // 0004
	ROM_LOAD16_WORD_SWAP( "mpr-17643.ic4", 0x0400000, 0x0400000, CRC(5100f23b) SHA1(659c2300399ff1cbd24fb1eb18cfd6c26e06fd96) ) // 9000
	ROM_LOAD16_WORD_SWAP( "mpr-17642.ic3", 0x0800000, 0x0400000, CRC(1a5bcc73) SHA1(a7df04c0a326323ea185db5f55b3e0449d76c535) ) // 4900
	ROM_LOAD16_WORD_SWAP( "mpr-17641.ic2", 0x0c00000, 0x0400000, CRC(fccc3dae) SHA1(0df7fd8b1110ba9063dc4dc40301267229cb9a35) ) // 0490
	ROM_LOAD16_WORD_SWAP( "mpr-17640.ic1", 0x1000000, 0x0400000, CRC(981e3e69) SHA1(d242055e0359ec4b5fac4676b2f974fbc974cc68) ) // 0049
	ROM_LOAD16_WORD_SWAP( "mpr-17649.ic10",0x1400000, 0x0400000, CRC(618c47ae) SHA1(5b69ad36fcf8e70d34c3b2fc71412ce953c5ceb3) ) // 0004
	ROM_LOAD16_WORD_SWAP( "mpr-17648.ic9", 0x1800000, 0x0400000, CRC(bf184cce) SHA1(62c004ea279f9a649d21426369336c2e1f9d24da) ) // 9000
	ROM_LOAD16_WORD_SWAP( "mpr-17647.ic8", 0x1c00000, 0x0400000, CRC(9dd9330c) SHA1(c91a7f497c1f4bd283bd683b06dff88893724d51) ) // 4900
	ROM_LOAD16_WORD_SWAP( "mpr-17646.ic7", 0x2000000, 0x0400000, CRC(b77eb2ad) SHA1(b832c0f1798aca39adba840d56ae96a75346670a) ) // 0490
	ROM_LOAD16_WORD_SWAP( "mpr-17645.ic6", 0x2400000, 0x0400000, CRC(56968d07) SHA1(e88c3d66ea05affb4681a25d155f097bd1b5a84b) ) // 0049
ROM_END

/*
Aqua Stage (Coin pusher)
PCB: 833-12000 AQUA STAGE
*/

ROM_START( aquastge )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* SH2 code */
	ROM_LOAD32_WORD_SWAP( "epr-18280.ic30", 0x0000000, 0x080000, CRC(4038352a) SHA1(fe951592e2462c701c740da4ec77435ae3026eab) )
	ROM_LOAD32_WORD_SWAP( "epr-18279.ic29", 0x0000002, 0x080000, CRC(9697cbcd) SHA1(4afa9a3f85b9d4483dafae8a98526ae32abd7103) )
	ROM_LOAD32_WORD_SWAP( "epr-18282.ic32", 0x0100000, 0x080000, CRC(53684dd8) SHA1(59759f6e3f815280a2406cc2133fc46e673cc76a))
	ROM_LOAD32_WORD_SWAP( "epr-18281.ic31", 0x0100002, 0x080000, CRC(f1233190) SHA1(471578c9343ac7198d73bee73975656c52e0bc5d) )

	/* Page 12 of the service manual states that these 4 regions are tested, so I believe that they are read by the SH-2 */
	ROM_REGION32_BE( 0x1000000, "gfx_data", ROMREGION_ERASEFF ) /* SH2 code */
	ROM_LOAD32_WORD_SWAP( "mpr-18283.ic17", 0x0c00002, 0x0200000, CRC(f42e2e72) SHA1(caf8733b6888ee718032c55f64da14590353517e) )
	ROM_RELOAD(0x0000002, 0x0200000)
	ROM_LOAD32_WORD_SWAP( "mpr-18284.ic18", 0x0c00000, 0x0200000, CRC(5fdf3c1f) SHA1(9976fe4afc3234eecbaf47a2e0f951b6fe1cb5f5) )
	ROM_RELOAD(0x0000000, 0x0200000)

	ROM_REGION( 0x100000, "sub", 0 ) /* SH1 */
	ROM_LOAD16_WORD_SWAP( "epr-18278.ic12", 0x000000, 0x020000,  CRC(e601132a) SHA1(bed103ef2e0dfa8bb485d93d661142b82c23088b) )

	/* these are compressed sprite data */
	ROM_REGION( 0x2800000, "compressedgfx", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "mpr-18289.ic5", 0x0000000, 0x0200000, CRC(fb212692) SHA1(da2f77564e718276c66b0f02e72a97d7b3653c35) ) // 0004
	ROM_LOAD16_WORD_SWAP( "mpr-18288.ic4", 0x0400000, 0x0200000, CRC(558c82ee) SHA1(e2ae4f2e81c7360eedd1b18e36d1f6ca6c015fee) ) // 9000
	ROM_LOAD16_WORD_SWAP( "mpr-18287.ic3", 0x0800000, 0x0200000, CRC(bf8743d8) SHA1(8d0e0691ec062f1939db8f6b75edb92d34417f81) ) // 4900
	ROM_LOAD16_WORD_SWAP( "mpr-18286.ic2", 0x0c00000, 0x0200000, CRC(3eb95e9a) SHA1(d565e5f353327e16f7ead2251fd9a503bf46e210) ) // 0490
	ROM_LOAD16_WORD_SWAP( "mpr-18285.ic1", 0x1000000, 0x0200000, CRC(8390453c) SHA1(e7d3a6579d9805a71b954bb248a2da749e9d9d38) ) // 0049
	ROM_LOAD16_WORD_SWAP( "mpr-18294.ic10",0x1400000, 0x0200000, CRC(341ebd4a) SHA1(9d9a1c09d81a50132edcc18a99266416683f4ff6) ) // 0004
	ROM_LOAD16_WORD_SWAP( "mpr-18293.ic9", 0x1800000, 0x0200000, CRC(f76bc076) SHA1(f5a8f9bd26b2e8533a1fbf8da6938373df971749)) // 9000
	ROM_LOAD16_WORD_SWAP( "mpr-18292.ic8", 0x1c00000, 0x0200000, CRC(59a713f9) SHA1(388b833fa6fb930f26c80674606505ec80668a16) ) // 4900
	ROM_LOAD16_WORD_SWAP( "mpr-18291.ic7", 0x2000000, 0x0200000, CRC(b6c167bd) SHA1(4990bae50e8804b2e1048aa5c64b086e8427073f) ) // 0490
	ROM_LOAD16_WORD_SWAP( "mpr-18290.ic6", 0x2400000, 0x0200000, CRC(11f7adb0) SHA1(a72f9892f93506456edc7ffc66224446a58ca38b) ) // 0049
ROM_END


void coolridr_state::init_coolridr()
{
	m_maincpu->sh2drc_set_options(SH2DRC_FASTEST_OPTIONS);
	m_subcpu->sh2drc_set_options(SH2DRC_FASTEST_OPTIONS);

	m_colbase = 0x7b20;

	// work around the hack when mapping the workram directly
	m_maincpu->sh2drc_add_fastram(0x06000000, 0x060fffff, 0, &m_workram_h[0]);
	m_maincpu->sh2drc_add_fastram(0x00000000, 0x001fffff, 1, &m_rom[0]);
	m_maincpu->sh2drc_add_fastram(0x20000000, 0x201fffff, 1, &m_rom[0]);
}

void coolridr_state::init_aquastge()
{
	m_maincpu->sh2drc_set_options(SH2DRC_FASTEST_OPTIONS);
	m_subcpu->sh2drc_set_options(SH2DRC_FASTEST_OPTIONS);

	m_colbase = 0;
}

} // anonymous namespace


GAME(  1995, coolridr, 0, coolridr, coolridr, coolridr_state, init_coolridr, ROT0, "Sega", "Cool Riders", MACHINE_IMPERFECT_SOUND | MACHINE_NODEVICE_LAN ) // region is set in test mode, this set is for Japan, USA and Export (all regions)
GAMEL( 1995, aquastge, 0, aquastge, aquastge, coolridr_state, init_aquastge, ROT0, "Sega", "Aqua Stage",  MACHINE_NOT_WORKING, layout_aquastge)
