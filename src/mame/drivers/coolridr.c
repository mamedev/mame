/******************************************************************************************************

    System H1 (c) 1994 Sega

    preliminary driver by David Haywood, Angelo Salese and Tomasz Slanina
    special thanks to Guru for references and HW advices

    TODO:
    - decode compressed GFX ROMs for "sprite" blitter (6,141,122 is the patent number)
	  http://www.google.com/patents/US6141122
    - DMA is still a bit of a mystery;
    - video emulation is pratically non-existant;
    - SCSP;
    - Many SH-1 ports needs investigations;
    - i8237 purpose is unknown, might even not be at the right place ...
    - IRQ generation
    - Understand & remove the hacks at the bottom;
    - IC1/IC10 are currently unused, might contain sprite data / music data for the SCSP / chars for the
      text tilemap/blitter;

=======================================================================================================

Cool Riders
Sega 1994

This game runs on SYSTEM-H1 hardware. Only one known game exists on this
PCB and this is it. The hardware seems overly complex for a 2D bike
racing game? The design of the PCB is very similar to vanilla Model 2
(i.e. Daytona etc). However instead of fully custom-badged chips,
many of the custom chips are off-the-shelf Hitachi/Toshiba gate-arrays.


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

From looking at the ROMs there seem to be structures that may be
tables. A table would be needed to associate a tile number with RLE
data in ROM, so that would be a likely function of tables in the ROM.
It may be that the split address design allows the chip to read table
data from one 80-bit port while reading in graphics data from the
other.

Now this next bit is just an interpretation of what's in the patent,
so you probably already know it:

The primitive unit of graphics data is a 10-bit word (hence all the
multiples of 10 listed above, e.g. 80 and 160-bit words) The top two
bits define the type:

0,x : $000 : Type A (3-bit color code, 5-bit run length, 1 bit "cell" flag)
1,x : $800 : Type B (7-bit color code, 2-bit run length)
1,1 : $C00 : Type C (8-bit color code)

Because the top two bits (which identify the type) are shared with the
color code, this limits the range of colors somewhat. E.g. one of the
patent diagrams shows the upper 4 bits of a Type A pixel moved into
the bottom four bits of the color RAM index, but for type A the most
significant bit is always zero. So that 4-bit quantity is always 0-7
which is why type A has a limit of 8 colors.

This is why one of the later diagrams shows the Type B pixel ANDed
with 7F and the Type C pixel ANDed with 3FF, to indicate the two
indicator bits are being stripped out and the remaining value is
somewhat truncated.

We figured due to the colorful graphics in the game there would be a
large groups of Type C pixels, but we could never find an
rearrangement of data and address bits that yielded a large number of
words with bits 9,8 set. Some of the data looks like it's linear, and
some of it looks like bitplanes. Perhaps tables are linear and
graphics are planar.

We tried the usual stuff; assuming the 16-bit words from each ROM was
a 160-pixel wide array of 10-bit elements (linear), or assuming each
bit from each ROM was a single bit of a 10-bit word so 16 bits defined
16 10-bit words in parallel (planar), and never got useful output.

There may be some weird data/address scrambling done to put data in an
order easiest for the hardware to process, which would be
non-intuitive.

Also the two indicator bits may have different polarities, maybe 0,0
is Type C instead of 1,1.

--- CORRECTION from Charles 20/02/13

I was just playing around with decoding this morning, and now I feel
certain the ordering is to take the 16-bit output of each of the 10
ROMs at a given address and divide that 160-bit word into sixteen
10-bit words.

When you do this, you get some kind of animation at 0x3898E0 which is
eight sequential frames of an object with a solid color fill in the
background. Because the size is constant it may be all Type C pixels
(e.g. no run-length) so that could be a good place to start with
shuffling bits/addresses to see how to get a lot of $3xx pixels.

There's a similar animation at 0x73CEC0, smaller object, 32 frames.

I think the 'background' data behind the object in both cases might be
a pen number that continuously increases, maybe for flashing effect as
you cycle through the animation. Otherwise you'd expect the background
pixels to be the same in each frame.

I also wonder if a pixel value of 0000 (type A with run length=0)
indicates a single dot since there are a lot of 0000 words.

There may be some bit/byte/address swapping needed to still get valid
output. And the tables may be encoded differently, there's a large one
at 0xDE60.



*/


#include "emu.h"
#include "debugger.h"
#include "cpu/sh2/sh2.h"
#include "cpu/m68000/m68000.h"
#include "sound/scsp.h"
#include "machine/am9517a.h"
#include "machine/nvram.h"
#include "rendlay.h"

#define CLIPMAXX_FULL (496-1)
#define CLIPMAXY_FULL (384-1)
#define CLIPMINX_FULL (0)
#define CLIPMINY_FULL (0)

class coolridr_state : public driver_device
{
public:
	coolridr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
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
		m_sysh1_txt_blit(*this, "sysh1_txt_blit"),
		m_sysh1_workram_h(*this, "sysh1_workrah"),
		m_sound_dma(*this, "sound_dma"),
		m_soundram(*this, "soundram"),
		m_soundram2(*this, "soundram2")
	{
		m_work_queue[0] = osd_work_queue_alloc(WORK_QUEUE_FLAG_HIGH_FREQ);
		m_work_queue[1] = osd_work_queue_alloc(WORK_QUEUE_FLAG_HIGH_FREQ);
	}

	// Blitter state
	UINT16 m_textBytesToWrite;
	INT16  m_blitterSerialCount;
	UINT8  m_blitterMode;
	UINT8  m_blittype;
	UINT16 m_blitterAddr;
	UINT16 m_textOffset;
	UINT32 m_blitterClearMode;
	INT16 m_blitterClearCount;

	// store the blit params here
	UINT32 m_spriteblit[12];

	UINT32 m_clipvals[2][3];



	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_soundcpu;
	//required_device<am9517a_device> m_dmac;

	required_shared_ptr<UINT32> m_framebuffer_vram;
	required_shared_ptr<UINT32> m_txt_vram;
	required_shared_ptr<UINT32> m_sysh1_txt_blit;
	required_shared_ptr<UINT32> m_sysh1_workram_h;
	required_shared_ptr<UINT32> m_sound_dma;
	required_shared_ptr<UINT16> m_soundram;
	required_shared_ptr<UINT16> m_soundram2;
	bitmap_rgb32 m_temp_bitmap_sprites[4];
	bitmap_rgb32 m_temp_bitmap_sprites2[4];
	bitmap_ind16 m_zbuffer_bitmap;
	bitmap_ind16 m_zbuffer_bitmap2;

	bitmap_rgb32 m_screen1_bitmap;
	bitmap_rgb32 m_screen2_bitmap;
	int m_scsp_last_line;
	UINT8 an_mux_data;
	UINT8 sound_data, sound_fifo;

	UINT8* m_compressedgfx;
	UINT16* m_expanded_10bit_gfx;
	UINT16* m_rearranged_16bit_gfx;

	UINT32 get_20bit_data(UINT32 romoffset, int _20bitwordnum);
	UINT16 get_10bit_data(UINT32 romoffset, int _10bitwordnum);

	DECLARE_READ32_MEMBER(sysh1_sound_dma_r);
	DECLARE_WRITE32_MEMBER(sysh1_sound_dma_w);
	DECLARE_READ32_MEMBER(sysh1_ioga_r);
	DECLARE_WRITE32_MEMBER(sysh1_ioga_w);
	DECLARE_READ32_MEMBER(sysh1_unk_blit_r);
	DECLARE_WRITE32_MEMBER(sysh1_unk_blit_w);
	DECLARE_WRITE32_MEMBER(sysh1_blit_mode_w);
	DECLARE_WRITE32_MEMBER(sysh1_blit_data_w);
	DECLARE_WRITE32_MEMBER(sysh1_fb_mode_w);
	DECLARE_WRITE32_MEMBER(sysh1_fb_data_w);

	DECLARE_WRITE32_MEMBER(sysh1_pal_w);
	DECLARE_WRITE32_MEMBER(sysh1_dma_w);
	DECLARE_WRITE32_MEMBER(sysh1_char_w);
	DECLARE_READ32_MEMBER(coolridr_hack2_r);
	DECLARE_READ16_MEMBER(h1_soundram_r);
	DECLARE_READ16_MEMBER(h1_soundram2_r);
	DECLARE_WRITE16_MEMBER(h1_soundram_w);
	DECLARE_WRITE16_MEMBER(h1_soundram2_w);
	DECLARE_READ8_MEMBER(analog_mux_r);
	DECLARE_WRITE8_MEMBER(analog_mux_w);
	DECLARE_WRITE8_MEMBER(lamps_w);
	DECLARE_WRITE_LINE_MEMBER(scsp1_to_sh1_irq);
	DECLARE_WRITE_LINE_MEMBER(scsp2_to_sh1_irq);
	DECLARE_WRITE8_MEMBER(sound_to_sh1_w);
	DECLARE_DRIVER_INIT(coolridr);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_coolridr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int which);
	UINT32 screen_update_coolridr1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_coolridr2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void blit_current_sprite(address_space &space);
	INTERRUPT_GEN_MEMBER(system_h1);
	TIMER_DEVICE_CALLBACK_MEMBER(system_h1_main);
	TIMER_DEVICE_CALLBACK_MEMBER(system_h1_sub);

	void sysh1_dma_transfer( address_space &space, UINT16 dma_index );

	UINT16 *m_h1_vram;
	UINT8 *m_h1_pcg;
	UINT16 *m_h1_pal;
	void flush_pal_data( UINT16 offset );
	void apply_rgb_control(int screen_num,int *r, int *g, int *b);
	int m_gfx_index;
	int m_color_bank;
	struct {
		UINT32 setting;
		UINT8 gradient;
	}m_rgb_ctrl[2];
	UINT32 m_pen_fill[2];

	osd_work_queue *    m_work_queue[2]; // work queue, one per screen
	static void *draw_object_threaded(void *param, int threadid);
};

#define PRINT_BLIT_STUFF \
	printf("type blit %08x %08x(%d, %03x) %08x(%02x, %03x) %08x(%06x) %08x(%08x, %d, %d, %d) %08x(%d,%d) %04x %04x %04x %04x %08x %08x %d %d\n", blit0, blit1_unused,b1mode,b1colorNumber, blit2_unused,b2tpen,b2colorNumber, blit3_unused,b3romoffset, blit4_unused, blit4, blit_flipy,blit_rotate, blit_flipx, blit5_unused, indirect_tile_enable, indirect_zoom_enable, vCellCount, hCellCount, vZoom, hZoom, blit10, textlookup, vPosition, hPosition); \



/* video */

#define VRAM_SIZE 0x100000

static const gfx_layout h1_tile_layout =
{
	16,16,
	0x1000,
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	16*128
};


void coolridr_state::video_start()
{
	/* find first empty slot to decode gfx */
	for (m_gfx_index = 0; m_gfx_index < MAX_GFX_ELEMENTS; m_gfx_index++)
		if (machine().gfx[m_gfx_index] == 0)
			break;

	machine().primary_screen->register_screen_bitmap(m_temp_bitmap_sprites[0]);
	machine().primary_screen->register_screen_bitmap(m_temp_bitmap_sprites2[0]);
	machine().primary_screen->register_screen_bitmap(m_zbuffer_bitmap);
	machine().primary_screen->register_screen_bitmap(m_zbuffer_bitmap2);




	// testing, not right
	machine().primary_screen->register_screen_bitmap(m_temp_bitmap_sprites[1]);
	machine().primary_screen->register_screen_bitmap(m_temp_bitmap_sprites2[1]);
	machine().primary_screen->register_screen_bitmap(m_temp_bitmap_sprites[2]);
	machine().primary_screen->register_screen_bitmap(m_temp_bitmap_sprites2[2]);
	machine().primary_screen->register_screen_bitmap(m_temp_bitmap_sprites[3]);
	machine().primary_screen->register_screen_bitmap(m_temp_bitmap_sprites2[3]);

	machine().primary_screen->register_screen_bitmap(m_screen1_bitmap);
	machine().primary_screen->register_screen_bitmap(m_screen2_bitmap);

	machine().gfx[m_gfx_index] = auto_alloc(machine(), gfx_element(machine(), h1_tile_layout, m_h1_pcg, 8, 0));
}

// might be a page 'map / base' setup somewhere, but it's just used for ingame backgrounds
/* 0x00000 - 0x1ffff = screen 1 */
/* 0x20000 - 0x3ffff = screen 2 */
/* 0x40000 - 0xfffff = ? */
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

UINT32 coolridr_state::screen_update_coolridr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int which)
{
	int bg_r,bg_g,bg_b;

	if(m_pen_fill[which])
	{
		/* logic here is a bit of a guess. */
		bg_r = (((m_pen_fill[which] >> 16) & 0x7f) << 1) | (((m_pen_fill[which] >> 16) & 0x80) >> 7);
		bg_g = (((m_pen_fill[which] >> 8) & 0x7f) << 1) | (((m_pen_fill[which] >> 8) & 0x80) >> 7);
		bg_b = (((m_pen_fill[which] >> 0) & 0x7f) << 1) | (((m_pen_fill[which] >> 0) & 0x80) >> 7);
		bitmap.fill(MAKE_ARGB(0xff,bg_r,bg_g,bg_b),cliprect);
	}
	else
	{
		UINT32 base_offset;
		int tile,vram_data,color;
		int scrollx;
		int scrolly;
		UINT8 transpen_setting;
		gfx_element *gfx = machine().gfx[m_gfx_index];
		#define VREG(_offs) \
			m_framebuffer_vram[(0x9b80+_offs+which*0x40)/4]

		scrollx = (VREG(0x2c) >> 16) & 0x7ff;
		scrolly = VREG(0x2c) & 0x3ff;

		base_offset = (VREG(0x1c) * 0x8000)/2;
		m_color_bank = which * 2;
		/* TODO: the whole transpen logic might be incorrect */
		transpen_setting = (VREG(0x3c) & 0x80000000) >> 31;
		bg_r = (VREG(0x3c) >> 10) & 0x1f;
		bg_g = (VREG(0x3c) >> 5) & 0x1f;
		bg_b = (VREG(0x3c) >> 0) & 0x1f;
		apply_rgb_control(which,&bg_r,&bg_g,&bg_b);

		bitmap.fill(MAKE_ARGB(0xff,pal5bit(bg_r),pal5bit(bg_g),pal5bit(bg_b)),cliprect);

 		for (int y=0;y<64;y++)
 		{
			for (int x=0;x<128;x++)
 			{
 				int res_x,res_y;

				res_x = (x*16)-scrollx;
 				res_y = (y*16)-scrolly;

				vram_data = (m_h1_vram[x+y*128+base_offset] & 0xffff);
				color = m_color_bank + ((vram_data & 0x800) >> 11) * 4;
				/* bike select enables bits 15-12, pretty sure one of these is tile bank (because there's a solid pen on 0x3ff / 0x7ff). */
				tile = (vram_data & 0x7ff) | ((vram_data & 0x8000) >> 4);

				drawgfx_transpen(bitmap,cliprect,gfx,tile,color,0,0,res_x,res_y,transpen_setting ? -1 : 0);
				drawgfx_transpen(bitmap,cliprect,gfx,tile,color,0,0,res_x+2048,res_y,transpen_setting ? -1 : 0);
				drawgfx_transpen(bitmap,cliprect,gfx,tile,color,0,0,res_x,res_y+1024,transpen_setting ? -1 : 0);
				drawgfx_transpen(bitmap,cliprect,gfx,tile,color,0,0,res_x+2048,res_y+1024,transpen_setting ? -1 : 0);
			}
		}
	}

	if (which==0)
	{
		// will probably need a custom function
		copybitmap_trans(bitmap, m_screen1_bitmap, 0, 0, 0, 0, cliprect, 0xff000000);
	}
	else
	{
		// will probably need a custom function
		copybitmap_trans(bitmap, m_screen2_bitmap, 0, 0, 0, 0, cliprect, 0xff000000);
	}

	return 0;
}

UINT32 coolridr_state::screen_update_coolridr1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return screen_update_coolridr(screen,bitmap,cliprect,0);
}

UINT32 coolridr_state::screen_update_coolridr2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return screen_update_coolridr(screen,bitmap,cliprect,1);
}

/* end video */




struct cool_render_object
{
	UINT8* indirect_tiles;
	UINT32* indirect_zoom;
	UINT32 spriteblit[12];
	bitmap_rgb32* drawbitmap;
	bitmap_ind16* zbitmap;
	UINT16 zpri;
	UINT8 blittype;
	coolridr_state* state;
	UINT32 clipvals[3];
};

#define RLE_BLOCK(writeaddrxor) \
	/* skip the decoding if it's the same tile as last time! */ \
	if (spriteNumber != lastSpriteNumber) \
	{ \
		lastSpriteNumber = spriteNumber; \
       \
		int i = 1;/* skip first 10 bits for now */ \
		int data_written = 0; \
 \
		while (data_written<256) \
		{ \
 \
			const UINT16 compdata = expanded_10bit_gfx[ (b3romoffset) + spriteNumber + i]; \
 \
			if (((compdata & 0x300) == 0x000) || ((compdata & 0x300) == 0x100)) /* 3bpp */ \
			{ \
				/* mm ccrr rrr0 */ \
				int encodelength = (compdata & 0x03e)>>1; \
				const UINT16 rledata =  rearranged_16bit_gfx[color_offs + ((compdata & 0x1c0) >> 6)]; \
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
				int encodelength = (compdata & 0x003);			 \
				const UINT16 rledata = rearranged_16bit_gfx[color_offs + ((compdata & 0x0fc) >> 2) + 8]; \
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
				tempshape[data_written^writeaddrxor] = rearranged_16bit_gfx[color_offs + (compdata & 0x0ff) + 0x48]; /* +0x48 crt test end of blue, start of white */ \
				if (tempshape[data_written^writeaddrxor]==0x8000) blankcount--; \
				data_written++; \
			} \
 \
			i++; \
		} \
	} \


#define YXLOOP_START \
	for (int y = 0; y < blockhigh; y++) \
	{ \
		int realy = ((y*incy)>>21); \
		if (!hZoomTable[realy]) \
			continue;	 \
		const int pixelOffsetX = ((hPositionTable[realy]) + (h* 16 * hZoomTable[realy])) / 0x40; \
		const int pixelOffsetnextX = ((hPositionTable[realy]) + ((h+1)* 16 * hZoomTable[realy])) / 0x40; \
		const int drawy = pixelOffsetY+y; \
		if ((drawy>clipmaxY) || (drawy<clipminY)) continue; \
		line = &drawbitmap->pix32(drawy); \
		zline = &object->zbitmap->pix16(drawy); \
		int blockwide = pixelOffsetnextX-pixelOffsetX; \
		if (pixelOffsetX+blockwide <clipminX) \
			continue; \
		if (pixelOffsetX>clipmaxX) \
			continue; \
		UINT32 incx = 0x8000000 / hZoomTable[realy]; \
		for (int x = 0; x < blockwide; x++) \
		{ \
			const int drawx = pixelOffsetX+x; \
			if ((drawx>clipmaxX || drawx<clipminX)) continue; \
			int realx = ((x*incx)>>21); \

#define YXLOOP_END \
		} \
	} \


#define YXLOOP_START_NO_LINEZOOM \
	for (int y = 0; y < blockhigh; y++) \
	{ \
		int realy = ((y*incy)>>21); \
		const int drawy = pixelOffsetY+y; \
		if ((drawy>clipmaxY) || (drawy<clipminY)) continue; \
		line = &drawbitmap->pix32(drawy); \
		zline = &object->zbitmap->pix16(drawy); \
		for (int x = 0; x < blockwide; x++) \
		{ \
			const int drawx = pixelOffsetX+x; \
			if ((drawx>clipmaxX || drawx<clipminX)) continue; \
			int realx = ((x*incx)>>21); \

#define YXLOOP_START_NO_LINEZOOM_NO_XCLIP \
	for (int y = 0; y < blockhigh; y++) \
	{ \
		int realy = ((y*incy)>>21); \
		const int drawy = pixelOffsetY+y; \
		if ((drawy>clipmaxY) || (drawy<clipminY)) continue; \
		line = &drawbitmap->pix32(drawy); \
		zline = &object->zbitmap->pix16(drawy); \
		for (int x = 0; x < blockwide; x++) \
		{ \
			const int drawx = pixelOffsetX+x; \
			int realx = ((x*incx)>>21); \


#define YXLOOP_START_NO_ZOOM \
	for (int y = 0; y < 16; y++) \
	{ \
		const int drawy = pixelOffsetY+y; \
		if ((drawy>clipmaxY) || (drawy<clipminY)) continue; \
		line = &drawbitmap->pix32(drawy); \
		zline = &object->zbitmap->pix16(drawy); \
		for (int x = 0; x < 16; x++) \
		{ \
			const int drawx = pixelOffsetX+x; \
			if ((drawx>clipmaxX || drawx<clipminX)) continue; \

#define YXLOOP_START_NO_ZOOM_NO_XCLIP \
	for (int y = 0; y < 16; y++) \
	{ \
		const int drawy = pixelOffsetY+y; \
		if ((drawy>clipmaxY) || (drawy<clipminY)) continue; \
		line = &drawbitmap->pix32(drawy); \
		zline = &object->zbitmap->pix16(drawy); \
		for (int x = 0; x < 16; x++) \
		{ \
			const int drawx = pixelOffsetX+x; \



/* the two tables that the patent claims are located at:
	0x1ec800
	0x1f0000
	0x3ec800
	0x3f0000
of each rom ... ROM 1 + 2 gives the full palette data for each pixel, in even/odd order.
TODO: fix anything that isn't text.
*/


#define DRAW_PIX \
	if (pix != 0x8000) \
	{ \
		if (object->zpri < zline[drawx]) \
		{ \
			{ \
				int r,g,b; \
				r = pal5bit((pix >> 10) & 0x1f); \
				g = pal5bit((pix >> 5) & 0x1f); \
				 b = pal5bit((pix >> 0) & 0x1f); \
				line[drawx] = r<<16 | g<<8 | b; \
				zline[drawx] = object->zpri; \
			} \
		} \
	} \
	else \
	{ \
		/* some alpha sprites have 0x8000 set */ \
		/* but some regular ones text do too? */ \
		/* how do we tell the difference between them? */ \
		/* how would you have a black 0x0000 in an alpha sprite?? */ \
	} \


//object->rearranged_16bit_gfx
//object->expanded_10bit_gfx





void *coolridr_state::draw_object_threaded(void *param, int threadid)
{
	cool_render_object *object = reinterpret_cast<cool_render_object *>(param);
	bitmap_rgb32* drawbitmap = object->drawbitmap;

	UINT16* rearranged_16bit_gfx = object->state->m_rearranged_16bit_gfx;
	UINT16* expanded_10bit_gfx = object->state->m_expanded_10bit_gfx;

	UINT16 clipminX = CLIPMINX_FULL;
	UINT16 clipmaxX = CLIPMAXX_FULL;
	UINT16 clipminY = CLIPMINY_FULL;
	UINT16 clipmaxY = CLIPMAXY_FULL;


	/************* object->spriteblit[1] *************/

	// 000u0ccc  - c = colour? u = 0/1
	UINT32 blit1_unused = object->spriteblit[1] & 0xfffef800;
	UINT32 b1mode = (object->spriteblit[1] & 0x00010000)>>16;
	UINT32 b1colorNumber = (object->spriteblit[1] & 0x000007ff);    // Probably more bits

	if (blit1_unused!=0) printf("blit1 unknown bits set %08x\n", object->spriteblit[1]);

	/************* object->spriteblit[3] *************/

	// seems to be more complex than just transparency
	UINT32 blit2_unused = object->spriteblit[2]&0xff80f800;
	UINT32 b2tpen = (object->spriteblit[2] & 0x007f0000)>>16;
	UINT32 b2colorNumber = (object->spriteblit[2] & 0x000007ff);

	if (b2colorNumber != b1colorNumber)
	{
	//	b1colorNumber = space.machine().rand()&0xfff;
	}




//	if(b1colorNumber > 0x60 || b2colorNumber)
//		printf("%08x %08x\n",b1colorNumber,b2colorNumber);


	if (blit2_unused!=0) printf("blit1 unknown bits set %08x\n", object->spriteblit[2]);
	if (b1mode)
	{
		if (b2tpen != 0x7f) printf("b1mode 1, b2tpen!=0x7f\n");
	}
	else
	{
		// 0x01/0x02 trips in rare cases (start of one of the attract levels) maybe this is some kind of alpha instead?
		if ((b2tpen != 0x00) && (b2tpen != 0x01) && (b2tpen != 0x02)) printf("b1mode 0, b2tpen!=0x00,0x01 or 0x02 (is %02x)\n", b2tpen);
	}
	 // 00??0uuu
	 // ?? seems to be 00 or 7f, set depending on b1mode
	 // uuu, at least 11 bits used, maybe 12 usually the same as blit1_unused? leftover?

	/************* object->spriteblit[3] *************/

	UINT32 blit3_unused = object->spriteblit[3] & 0xffe00000;
	UINT32 b3romoffset = (object->spriteblit[3] & 0x001fffff)*16;

	if (blit3_unused) printf("unknown bits in blit word %d -  %08x\n", 3, blit3_unused);


	/************* object->spriteblit[4] *************/

	UINT32 blit4_unused = object->spriteblit[4] & 0xf8fefefe;
	//UINT32 blit4 = (object->spriteblit[4] & 0x07000000)>>24;
	//object->zpri = 7-blit4;
	// unknown bits in blit word 4 -  00000010 - australia
	UINT32 blit_flipx = object->spriteblit[4] & 0x00000001;
	UINT32 blit_flipy = (object->spriteblit[4] & 0x00000100)>>8;
	UINT32 blit_rotate = (object->spriteblit[4] & 0x00010000)>>16;
	if (blit4_unused) printf("unknown bits in blit word %d -  %08x\n", 4, blit4_unused);

	// ---- -111 ---- ---r ---- ---y ---- ---x
	// 1 = used bits? (unknown purpose.. might be object colour mode)
	// x = x-flip
	// y = y-flip
	// r = unknown, not used much, occasional object - rotate

	/************* object->spriteblit[5] *************/

	UINT32 blit5_unused = object->spriteblit[5]&0xfffefffe;
	// this might enable the text indirection thing?
	int indirect_tile_enable = (object->spriteblit[5] & 0x00010000)>>16;
	int indirect_zoom_enable = (object->spriteblit[5] & 0x00000001);


	if (blit5_unused) printf("unknown bits in blit word %d -  %08x\n", 5, blit5_unused);
	// 00010000 (text)
	// 00000001 (other)




	/************* object->spriteblit[6] *************/

	UINT16 vCellCount = (object->spriteblit[6] & 0x03ff0000) >> 16;
	UINT16 hCellCount = (object->spriteblit[6] & 0x000003ff);

	/************* object->spriteblit[7] *************/

	UINT16 vOrigin = (object->spriteblit[7] & 0x00030000) >> 16;
	UINT16 hOrigin = (object->spriteblit[7] & 0x00000003);
	UINT16 OriginUnused = (object->spriteblit[7] & 0xfffcfffc);

	if (blit5_unused) printf("unknown bits in blit word %d -  %08x\n", 7, OriginUnused);

	//printf("%04x %04x\n", vOrigin, hOrigin);

	/************* object->spriteblit[8] *************/

	UINT16 vZoom = (object->spriteblit[8] & 0xffff0000) >> 16;
	UINT16 hZoom = (object->spriteblit[8] & 0x0000ffff);

	// if we have no vertical zoom value there's no point in going any further
	// because there are no known vertical indirect modes
	if (!vZoom)
	{
		// abort, but make sure we clean up
		if (object->indirect_tiles)
			free(object->indirect_tiles);

		if (object->indirect_zoom)
			free(object->indirect_zoom);

		free (object);

		return NULL;
	}

	/************* object->spriteblit[9] *************/

	int vPosition = (object->spriteblit[9] & 0xffff0000) >> 16;
	int hPositionx = (object->spriteblit[9] & 0x0000ffff);

	if (hPositionx & 0x8000) hPositionx -= 0x10000;
	if (vPosition & 0x8000) vPosition -= 0x10000;

	/************* object->spriteblit[10] *************/

	// pointer to per-line zoom and scroll data for sprites
	UINT32 blit10 = 0; // we've cached the data here already

	/************* object->spriteblit[11] *************/

	//UINT32 textlookup = 0; // we've cached the data here already

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

		UINT16 minx = ((object->clipvals[1]&0xffff0000)>>16) - (((object->clipvals[2]&0x0000ffff)>>0));
		UINT16 maxx = ((object->clipvals[1]&0x0000ffff)>>0)  - (((object->clipvals[2]&0x0000ffff)>>0));
		UINT16 miny = ((object->clipvals[0]&0xffff0000)>>16); // maybe subtract the top 16 bits of clipvals[2], but not used?
		UINT16 maxy = ((object->clipvals[0]&0x0000ffff)>>0);


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
	


//		b1colorNumber = object->state->machine().rand()&0xfff;
	}

	/* DRAW */
	UINT16 used_hCellCount = hCellCount;
	UINT16 used_vCellCount = vCellCount;
	UINT16 used_flipx = blit_flipx;
	UINT16 used_flipy = blit_flipy;

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
		//	hPositionx -= 1;

		UINT16 hZoomTable[16];
		int hPositionTable[16];
		int hPosition = 0;

		if (indirect_zoom_enable)
		{
			for (int idx=0;idx<16;idx++)
			{
				UINT32 dword = object->indirect_zoom[blit10];

				hZoomTable[idx] = hZoom + (dword>>16); // add original value?

				// bit 0x8000 does get set too, but only on some lines, might have another meaning?
				int linescroll = dword&0x7fff;
				if (linescroll & 0x4000) linescroll -= 0x8000;

				hPositionTable[idx] = linescroll + hPositionx;
				blit10++;

				// DON'T use the table hZoom in this calc? (road..)
				int sizex = used_hCellCount * 16 * hZoom;

				hPositionTable[idx] *= 0x40;

				switch (hOrigin & 3)
				{
				case 0:
					// left
					break;
				case 1:
					hPositionTable[idx] -= sizex / 2;
					// middle?
					break;
				case 2:
					hPositionTable[idx] -= sizex-1;
					// right?
					break;
				case 3:
					// invalid?
					break;
				}
			}
		}
		else
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



		for (int h = 0; h < used_hCellCount; h++)
		{
			if (!indirect_zoom_enable)
			{
			//	int offs = ((hPosition) + (h* 16 * hZoom)) / 0x40;
			//	if (offs>clipmaxX) continue;
			}

			UINT32 lastSpriteNumber = 0xffffffff;

			int lookupnum;

			// with this bit enabled the tile numbers gets looked up using 'data' (which would be blit11) (eg 03f40000 for startup text)
			// this allows text strings to be written as 8-bit ascii in one area (using command 0x10), and drawn using multi-width sprites
			if (indirect_tile_enable)
			{
				// this doesn't handle the various flip modes..
				lookupnum = object->indirect_tiles[h + (v*used_hCellCount)];
			}
			else
			{
				if (!blit_rotate)
				{
					if (!used_flipy)
					{
						if (!used_flipx)
							lookupnum = h + (v*used_hCellCount);
						else
							lookupnum = (used_hCellCount-h-1) + (v*used_hCellCount);
					}
					else
					{
						if (!used_flipx)
							lookupnum = h + ((used_vCellCount-v-1)*used_hCellCount);
						else
							lookupnum = (used_hCellCount-h-1) + ((used_vCellCount-v-1)*used_hCellCount);

					}
				}
				else
				{
					if (!used_flipy)
					{
						if (!used_flipx)
							lookupnum = v + (h*used_vCellCount);
						else
							lookupnum = (used_vCellCount-v-1) + (h*used_vCellCount);
					}
					else
					{
						if (!used_flipx)
							lookupnum = v + ((used_hCellCount-h-1)*used_vCellCount);
						else
							lookupnum = (used_vCellCount-v-1) + ((used_hCellCount-h-1)*used_vCellCount);

					}
				}
			}

			// these should be 'cell numbers' (tile numbers) which look up RLE data?
			UINT32 spriteNumber = (expanded_10bit_gfx[ (b3romoffset) + (lookupnum<<1) +0 ] << 10) | (expanded_10bit_gfx[ (b3romoffset) + (lookupnum<<1) + 1 ]);
			UINT16 tempshape[16*16];

			int color_offs = (0x7b20 + (b1colorNumber & 0x7ff))*0x40 * 5; /* yes, * 5 */
			UINT16 blankcount = 256;

			if (used_flipy)
			{
				if (used_flipx)
				{
					RLE_BLOCK(0xff)
				}
				else
				{
					RLE_BLOCK(0xf0)
				}
			}
			else
			{	if (used_flipx)
				{
					RLE_BLOCK(0x0f)
				}
				else
				{
					RLE_BLOCK(0x00)
				}
			}


			if (blankcount==0)
				continue;






			UINT32 incy = 0x8000000 / vZoom;

			// DEBUG: Draw 16x16 block
			UINT32* line;
			UINT16* zline;


			if (indirect_zoom_enable)
			{
				if (blit_rotate)
				{
					YXLOOP_START
					UINT16 pix = tempshape[realx*16+realy];
					DRAW_PIX
					YXLOOP_END
				}
				else // no rotate
				{
					YXLOOP_START
					UINT16 pix = tempshape[realy*16+realx];
					DRAW_PIX
					YXLOOP_END
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

					if (pixelOffsetX>=clipminX && pixelOffsetX+16<clipmaxX)
					{
						if (blit_rotate)
						{
							YXLOOP_START_NO_ZOOM_NO_XCLIP
							UINT16 pix = tempshape[x*16+y];
							DRAW_PIX
							YXLOOP_END
						}
						else // no rotate
						{
							YXLOOP_START_NO_ZOOM_NO_XCLIP
							UINT16 pix = tempshape[y*16+x];
							DRAW_PIX
							YXLOOP_END
						}
					}
					else
					{
						if (blit_rotate)
						{
							YXLOOP_START_NO_ZOOM
							UINT16 pix = tempshape[x*16+y];
							DRAW_PIX
							YXLOOP_END
						}
						else // no rotate
						{
							YXLOOP_START_NO_ZOOM
							UINT16 pix = tempshape[y*16+x];
							DRAW_PIX
							YXLOOP_END
						}
					}
				}
				else // zoomed
				{
					const int pixelOffsetX = ((hPosition) + (h* 16 * hZoom)) / 0x40;
					const int pixelOffsetnextX = ((hPosition) + ((h+1)* 16 * hZoom)) / 0x40;

					int blockwide = pixelOffsetnextX-pixelOffsetX;
					UINT32 incx = 0x8000000 / hZoom;

					if (pixelOffsetX+blockwide < clipminX)
						continue;

					if (pixelOffsetX>clipmaxX)
						continue;

					if (pixelOffsetX>=clipminX && pixelOffsetX+blockwide<clipmaxX)
					{
						if (blit_rotate)
						{
							YXLOOP_START_NO_LINEZOOM_NO_XCLIP
							UINT16 pix = tempshape[realx*16+realy];
							DRAW_PIX
							YXLOOP_END
						}
						else // no rotate
						{
							YXLOOP_START_NO_LINEZOOM_NO_XCLIP
							UINT16 pix = tempshape[realy*16+realx];
							DRAW_PIX
							YXLOOP_END
						}
					}
					else
					{
						if (blit_rotate)
						{
							YXLOOP_START_NO_LINEZOOM
							UINT16 pix = tempshape[realx*16+realy];
							DRAW_PIX
							YXLOOP_END
						}
						else // no rotate
						{
							YXLOOP_START_NO_LINEZOOM
							UINT16 pix = tempshape[realy*16+realx];
							DRAW_PIX
							YXLOOP_END
						}
					}
				} // end zoomed
			} // end no indirect zoom
		}
	}



	end:

	if (object->indirect_tiles)
		free(object->indirect_tiles);

	if (object->indirect_zoom)
		free(object->indirect_zoom);

	free (object);

	return NULL;

}



/* This is a RLE-based sprite blitter (US Patent #6,141,122), very unusual from Sega... */
void coolridr_state::blit_current_sprite(address_space &space)
{
//	const pen_t *clut = &machine().pens[0];

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
	UINT32 blit0 = m_spriteblit[0];

	if (blit0==0)
	{
		// normal
	}
	else if (blit0==1)
	{

	

		if (m_blitterMode&0x80)
		{
			m_clipvals[1][0] = m_spriteblit[1];
			m_clipvals[1][1] = m_spriteblit[2];
			m_clipvals[1][2] = m_spriteblit[3];

		}
		else
		{
			m_clipvals[0][0] = m_spriteblit[1];
			m_clipvals[0][1] = m_spriteblit[2];
			m_clipvals[0][2] = m_spriteblit[3];
			//printf("unknown sprite list type 1 - %04x %04x %04x %04x (%08x %08x %08x)\n", (m_spriteblit[1]&0xffff0000)>>16,(m_spriteblit[1]&0x0000ffff)>>0, ((m_spriteblit[2]&0xffff0000)>>16)-((m_spriteblit[3]&0x0000ffff)>>0),((m_spriteblit[2]&0x0000ffff)>>0)-((m_spriteblit[3]&0x0000ffff)>>0),    m_spriteblit[1],m_spriteblit[2],m_spriteblit[3]);
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

	cool_render_object* testobject = (cool_render_object *)malloc(sizeof(cool_render_object));

	testobject->state = this;

	for (int i=0;i<12;i++)
		testobject->spriteblit[i] = m_spriteblit[i];

	// cache some values that are looked up from RAM to be safe.. alternatively we could stall the rendering if they get written to, but they're a direct memory pointer..
	int test_indirect_tile_enable = (m_spriteblit[5] & 0x00010000)>>16;

	if (test_indirect_tile_enable)
	{
		UINT32 test_textlookup =  m_spriteblit[11];
		UINT16 test_hCellCount = (m_spriteblit[6] & 0x00003ff);
		UINT16 test_vCellCount = (m_spriteblit[6] & 0x03ff0000) >> 16;
		int bytes = test_vCellCount*test_hCellCount;
		testobject->indirect_tiles = (UINT8*)malloc(bytes);
		for (int i=0;i<bytes;i++)
		{
			testobject->indirect_tiles[i] = space.read_byte(test_textlookup + i);
		}
	}
	else
	{
		testobject->indirect_tiles = NULL;
	}

	int test_indirect_zoom_enable = (m_spriteblit[5] & 0x00000001);
	if (test_indirect_zoom_enable)
	{
		UINT32 test_blit10 =  m_spriteblit[10];
		UINT16 test_vCellCount = (m_spriteblit[6] & 0x03ff0000) >> 16;
		int bytes = test_vCellCount * 4 * 16;
		testobject->indirect_zoom = (UINT32*)malloc(bytes);
		for (int i=0;i<bytes/4;i++)
		{
			testobject->indirect_zoom[i] = space.read_dword(test_blit10 + i*4);
		}
	}
	else
	{
		testobject->indirect_zoom = NULL;
	}

	testobject->zpri = m_blitterAddr | m_blittype<<12;
	testobject->blittype = m_blittype;
	osd_work_queue *queue;
	// which queue, which bitmap
	if (m_blitterMode == 0x30 || m_blitterMode == 0x40 || m_blitterMode == 0x50 || m_blitterMode == 0x60)
	{
		testobject->drawbitmap = &m_temp_bitmap_sprites[0];
		testobject->zbitmap = &m_zbuffer_bitmap;
		// pass these from the type 1 writes
		testobject->clipvals[0] = m_clipvals[0][0];
		testobject->clipvals[1] = m_clipvals[0][1];
		testobject->clipvals[2] = m_clipvals[0][2];

		queue = m_work_queue[0];
	}
	else // 0x90, 0xa0, 0xb0, 0xc0
	{
		testobject->drawbitmap = &m_temp_bitmap_sprites2[0];
		testobject->zbitmap = &m_zbuffer_bitmap2;
		// pass these from the type 1 writes
		testobject->clipvals[0] = m_clipvals[1][0];
		testobject->clipvals[1] = m_clipvals[1][1];
		testobject->clipvals[2] = m_clipvals[1][2];

		queue = m_work_queue[1];
	}

	osd_work_item_queue(queue, draw_object_threaded, testobject, WORK_ITEM_FLAG_AUTO_RELEASE);
}


WRITE32_MEMBER(coolridr_state::sysh1_blit_mode_w)
{
	m_blitterMode = (data & 0x00ff0000) >> 16;

	if (m_blitterMode == 0x4f)
	{
		/* used during end sequence, purpose unknown */
	}
	else if (m_blitterMode == 0xf4)
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
	else if (m_blitterMode == 0x30 || m_blitterMode == 0x40 || m_blitterMode == 0x50 || m_blitterMode == 0x60
		  || m_blitterMode == 0x90 || m_blitterMode == 0xa0 || m_blitterMode == 0xb0 || m_blitterMode == 0xc0)
	{
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
		if (data & 0x0000f000)
			printf("blitter mode with mask 0x0000f000\n");



		//if (m_blitterMode<0x80)  printf("blitter set screen %d mode %02x addr? %04x\n", (data&0x00800000)>>23, ((data & 0x00ff0000)>>16)-0x30, data & 0x00000fff);

		// form 0xacMM-xxx   ac = fixed value for this mode?  MM = modes above.  -xxx = some kind of offset? but it doesn't increment for each blit like the textOffset / paletteOffset stuff, investigate

	}
	else if (m_blitterMode == 0x10)
	{
		// Could be a full clear of VRAM?
		for(UINT32 vramAddr = 0x3f40000; vramAddr < 0x3f4ffff; vramAddr+=4)
			space.write_dword(vramAddr, 0x00000000);

		m_blitterSerialCount = 0;
	}
	else if (m_blitterMode == 0xe0)
	{
		// uploads palettes...
		// does NOT upload the palette for the WDUD screen when set to US mode this way..
		m_blitterSerialCount = 0;
		m_textOffset = (data & 0x0000ffff);

	//	printf("set e0 %08x\n", data);

	}
	else
	{
		printf("set unknown blit mode %02x\n", m_blitterMode);
	}
}

WRITE32_MEMBER(coolridr_state::sysh1_blit_data_w)
{
	if (m_blitterMode == 0xf4)
	{
		// Uploads a series of bytes that index into the encoded sprite table
		const size_t memOffset = 0x03f40000 + m_textOffset + m_blitterSerialCount;
		space.write_dword(memOffset, data);
		m_blitterSerialCount += 0x04;
	}
	else if (m_blitterMode == 0x30 || m_blitterMode == 0x40 || m_blitterMode == 0x50 || m_blitterMode == 0x60
		  || m_blitterMode == 0x90 || m_blitterMode == 0xa0 || m_blitterMode == 0xb0 || m_blitterMode == 0xc0)
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
		//blit mode 10 02 00007b20  << this is the palette base for the sprites
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
		logerror("unk blit mode %02x\n", m_blitterMode);
	}
}

WRITE32_MEMBER(coolridr_state::sysh1_fb_mode_w)
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

WRITE32_MEMBER(coolridr_state::sysh1_fb_data_w)
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
		if(data != 0x000701f7 && m_sysh1_txt_blit[offset] != 0x020703f7)
			printf("Blitter Clear Count == 2 used with param %08x\n",data);
		*/
	}
	else if(m_blitterClearCount == 3)
	{
		if(data != 0x00000007 && data != 0x00000207)
			printf("Blitter Clear Count == 3 used with param %08x\n",data);

		{
			const rectangle& visarea = machine().primary_screen->visible_area();

			// test code
			int i = 0;

			if(m_blitterClearMode == 0x8c200000)
			{
				osd_work_queue_wait(m_work_queue[0], osd_ticks_per_second() * 100);
				copybitmap(m_screen1_bitmap, m_temp_bitmap_sprites[i], 0, 0, 0, 0, visarea);
				m_temp_bitmap_sprites[i].fill(0xff000000, visarea);
				m_zbuffer_bitmap.fill(0xffff, visarea);
				// almost certainly wrong
				m_clipvals[0][0] = 0;
				m_clipvals[0][1] = 0;
				m_clipvals[0][2] = 0;
			}
			else if(m_blitterClearMode == 0x8c800000)
			{
				osd_work_queue_wait(m_work_queue[1], osd_ticks_per_second() * 100);
				copybitmap(m_screen2_bitmap, m_temp_bitmap_sprites2[i], 0, 0, 0, 0, visarea);
				m_temp_bitmap_sprites2[i].fill(0xff000000, visarea);
				m_zbuffer_bitmap2.fill(0xffff, visarea);
				// almost certainly wrong
				m_clipvals[1][0] = 0;
				m_clipvals[1][1] = 0;
				m_clipvals[1][2] = 0;
			}

			//printf("frame\n");
		}
	}
	else
	{
		printf("Blitter Clear Count == %02x used with param %08x\n",m_blitterClearCount,m_sysh1_txt_blit[offset]);
	}

	m_blitterClearCount++;
}

READ32_MEMBER(coolridr_state::sysh1_unk_blit_r)
{
//	if(offset == 0x0c/4) reads

	return m_sysh1_txt_blit[offset];
}


WRITE32_MEMBER(coolridr_state::sysh1_unk_blit_w)
{
	COMBINE_DATA(&m_sysh1_txt_blit[offset]);

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

void coolridr_state::apply_rgb_control(int screen_num,int *r, int *g, int *b)
{
	if(!m_rgb_ctrl[screen_num].gradient)
		return;

	if(m_rgb_ctrl[screen_num].setting == 0x1240) /* fade-in / outs */
	{
		*r -= m_rgb_ctrl[screen_num].gradient;
		*g -= m_rgb_ctrl[screen_num].gradient;
		*b -= m_rgb_ctrl[screen_num].gradient;
		if(*r < 0) { *r = 0; }
		if(*g < 0) { *g = 0; }
		if(*b < 0) { *b = 0; }
	}
	else if(m_rgb_ctrl[screen_num].setting == 0x920) /* at bike select / outside tunnels, addition */
	{
		*r += m_rgb_ctrl[screen_num].gradient;
		*g += m_rgb_ctrl[screen_num].gradient;
		*b += m_rgb_ctrl[screen_num].gradient;
		if(*r > 0x1f) { *r = 0x1f; }
		if(*g > 0x1f) { *g = 0x1f; }
		if(*b > 0x1f) { *b = 0x1f; }
	}
	else if(m_rgb_ctrl[screen_num].setting == 0x800) /* when you get hit TODO: algo might be different. */
	{
		*g &= (0x1f - m_rgb_ctrl[screen_num].gradient);
		*b &= (0x1f - m_rgb_ctrl[screen_num].gradient);
	}
	else
	{
		popmessage("%08x %08x",m_rgb_ctrl[screen_num].setting,m_rgb_ctrl[screen_num].gradient);
	}
}

void coolridr_state::flush_pal_data( UINT16 offset )
{
	int r,g,b;
	int screen_type = (offset & 0x400) >> 10;

	r = ((m_h1_pal[offset] & 0x7c00) >> 10);
	g = ((m_h1_pal[offset] & 0x03e0) >> 5);
	b = ((m_h1_pal[offset] & 0x001f) >> 0);
	apply_rgb_control(screen_type,&r,&g,&b);

	palette_set_color_rgb(machine(),offset,pal5bit(r),pal5bit(g),pal5bit(b));
}

void coolridr_state::sysh1_dma_transfer( address_space &space, UINT16 dma_index )
{
	UINT32 src = 0,dst = 0,size = 0;
	UINT8 end_dma_mark;
	UINT8 cmd;

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
				if(dst & 0xfff00001)
					printf("unk values to %02x dst %08x\n",cmd,dst);
				dst &= 0x000ffffe;
				dst >>= 1;

				for(int i=0;i<size;i+=2)
				{
					m_h1_vram[dst] = space.read_word(src);
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
					m_h1_pcg[dst] = space.read_byte(src);
					machine().gfx[m_gfx_index]->mark_dirty(dst/256);
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
					m_h1_pal[dst] = space.read_word(src);
					flush_pal_data(dst);
					dst++;
					src+=2;
				}

				dma_index+=0xc;
				break;

			case 0x04: /* init - value 0x040c80d2 (unknown purpose, slave mode?) */
			case 0x10: /* sets up look-up for tilemap video registers */
			case 0x20: /* screen 1 - linescroll/zoom table? (default values) */
			case 0x24: /* screen 2 / */
			case 0x50: /* screen 1 - unknown */
			case 0x54: /* screen 2 / */
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
				for(int i=((cmd & 4) * 0x100);i<((cmd & 4) * 0x100)+0x400;i++)
					flush_pal_data( i );
				dma_index+=4;
				break;
			default:
				printf("%02x %08x\n",cmd,m_framebuffer_vram[(0+dma_index)/4]);
				dma_index+=4;
				break;
		}

	}while(!end_dma_mark );
}

WRITE32_MEMBER(coolridr_state::sysh1_dma_w)
{
	COMBINE_DATA(&m_framebuffer_vram[offset]);

	if(offset*4 == 0x000)
	{
		/* enable */
		if((m_framebuffer_vram[offset] & 0xff000000) == 0x0f000000)
			sysh1_dma_transfer(space, m_framebuffer_vram[offset] & 0xffff);
	}
}


static ADDRESS_MAP_START( system_h1_map, AS_PROGRAM, 32, coolridr_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM AM_SHARE("share1") AM_WRITENOP
	AM_RANGE(0x01000000, 0x01ffffff) AM_ROM AM_REGION("gfx_data",0x0000000)

	AM_RANGE(0x03c00000, 0x03c1ffff) AM_MIRROR(0x00200000) AM_RAM_WRITE(sysh1_dma_w) AM_SHARE("fb_vram") /* mostly mapped at 0x03e00000 */

	AM_RANGE(0x03f00000, 0x03f0ffff) AM_RAM AM_SHARE("share3") /*Communication area RAM*/
	AM_RANGE(0x03f40000, 0x03f4ffff) AM_RAM AM_SHARE("txt_vram")//text tilemap + "lineram"
	AM_RANGE(0x04000000, 0x0400000f) AM_READWRITE(sysh1_unk_blit_r,sysh1_unk_blit_w) AM_SHARE("sysh1_txt_blit")
	AM_RANGE(0x04000010, 0x04000013) AM_WRITE(sysh1_blit_mode_w)
	AM_RANGE(0x04000014, 0x04000017) AM_WRITE(sysh1_blit_data_w)
	AM_RANGE(0x04000018, 0x0400001b) AM_WRITE(sysh1_fb_mode_w)
	AM_RANGE(0x0400001c, 0x0400001f) AM_WRITE(sysh1_fb_data_w)

	AM_RANGE(0x06000000, 0x060fffff) AM_RAM AM_SHARE("sysh1_workrah")
	AM_RANGE(0x20000000, 0x201fffff) AM_ROM AM_SHARE("share1")

	AM_RANGE(0x60000000, 0x600003ff) AM_WRITENOP
ADDRESS_MAP_END


READ16_MEMBER( coolridr_state::h1_soundram_r)
{
	return m_soundram[offset];
}

READ16_MEMBER( coolridr_state::h1_soundram2_r)
{
	return m_soundram2[offset];
}

WRITE16_MEMBER( coolridr_state::h1_soundram_w)
{
	COMBINE_DATA(&m_soundram[offset]);
}

WRITE16_MEMBER( coolridr_state::h1_soundram2_w)
{
	COMBINE_DATA(&m_soundram2[offset]);
}

READ8_MEMBER( coolridr_state::analog_mux_r )
{
	static const char *const adcnames[] = { "AN0", "AN1", "AN2", "AN3", "AN4", "AN5", "AN6", "AN7" };
	UINT8 adc_data = ioport(adcnames[an_mux_data])->read_safe(0);
	an_mux_data++;
	an_mux_data &= 0x7;
	return adc_data;
}

WRITE8_MEMBER( coolridr_state::analog_mux_w )
{
	an_mux_data = data;
}

WRITE8_MEMBER( coolridr_state::lamps_w )
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


READ32_MEMBER(coolridr_state::sysh1_sound_dma_r)
{
	if(offset == 8)
	{
		//popmessage("%02x",sound_data);
		/*
		Checked in irq routine
		--x- ---- second SCSP
		---x ---- first SCSP
		*/
		return sound_data;
	}

	if(offset == 2 || offset == 6) // DMA status
		return 0;

	printf("%08x\n",offset);

	return m_sound_dma[offset];
}

WRITE32_MEMBER(coolridr_state::sysh1_sound_dma_w)
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
			UINT32 src = m_sound_dma[0];
			UINT32 dst = m_sound_dma[1];
			UINT32 size = (m_sound_dma[2]>>16)*0x40;

			//printf("%08x %08x %08x %02x\n",src,dst,size,sound_fifo);

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
			UINT32 src = m_sound_dma[4];
			UINT32 dst = m_sound_dma[5];
			UINT32 size = (m_sound_dma[6]>>16)*0x40;

			//printf("%08x %08x %08x %02x\n",src,dst,size,sound_fifo);

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


static ADDRESS_MAP_START( coolridr_submap, AS_PROGRAM, 32, coolridr_state )
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM AM_SHARE("share2")

	AM_RANGE(0x01000000, 0x0100ffff) AM_RAM //communication RAM

	AM_RANGE(0x03000000, 0x0307ffff) AM_READWRITE16(h1_soundram_r, h1_soundram_w,0xffffffff) //AM_SHARE("soundram")
	AM_RANGE(0x03100000, 0x03100fff) AM_DEVREADWRITE16_LEGACY("scsp1", scsp_r, scsp_w, 0xffffffff)
	AM_RANGE(0x03200000, 0x0327ffff) AM_READWRITE16(h1_soundram2_r, h1_soundram2_w,0xffffffff) //AM_SHARE("soundram2")
	AM_RANGE(0x03300000, 0x03300fff) AM_DEVREADWRITE16_LEGACY("scsp2", scsp_r, scsp_w, 0xffffffff)

//	AM_RANGE(0x04000000, 0x0400001f) AM_DEVREADWRITE8("i8237", am9517a_device, read, write, 0xffffffff)
	AM_RANGE(0x04000000, 0x0400003f) AM_READWRITE(sysh1_sound_dma_r,sysh1_sound_dma_w) AM_SHARE("sound_dma")
//	AM_RANGE(0x04200000, 0x0420003f) AM_RAM /* hi-word for DMA? */

	AM_RANGE(0x05000000, 0x05000fff) AM_RAM
	AM_RANGE(0x05200000, 0x052001ff) AM_RAM
	AM_RANGE(0x05300000, 0x0530ffff) AM_RAM AM_SHARE("share3") /*Communication area RAM*/
	AM_RANGE(0x05ff0000, 0x05ffffff) AM_RAM /*???*/
	AM_RANGE(0x06000000, 0x060001ff) AM_RAM AM_SHARE("nvram") // backup RAM
	AM_RANGE(0x06100000, 0x06100003) AM_READ_PORT("IN0") AM_WRITE8(lamps_w,0x000000ff)
	AM_RANGE(0x06100004, 0x06100007) AM_READ_PORT("IN1")
	AM_RANGE(0x06100008, 0x0610000b) AM_READ_PORT("IN5")
	AM_RANGE(0x0610000c, 0x0610000f) AM_READ_PORT("IN6")
	AM_RANGE(0x06100010, 0x06100013) AM_READ_PORT("IN2") AM_WRITENOP
	AM_RANGE(0x06100014, 0x06100017) AM_READ_PORT("IN3")
	AM_RANGE(0x0610001c, 0x0610001f) AM_READWRITE8(analog_mux_r,analog_mux_w,0x000000ff) //AM_WRITENOP
	AM_RANGE(0x06200000, 0x06200fff) AM_RAM //network related?
	AM_RANGE(0x07fff000, 0x07ffffff) AM_RAM
	AM_RANGE(0x20000000, 0x2001ffff) AM_ROM AM_SHARE("share2")

	AM_RANGE(0x60000000, 0x600003ff) AM_WRITENOP
ADDRESS_MAP_END

/* TODO: what is this for, volume mixing? MIDI? */
WRITE8_MEMBER(coolridr_state::sound_to_sh1_w)
{
	sound_fifo = data;
//	sound_data = data;
//	printf("%02x sound\n",data);
}

static ADDRESS_MAP_START( system_h1_sound_map, AS_PROGRAM, 16, coolridr_state )
	AM_RANGE(0x000000, 0x07ffff) AM_RAM AM_REGION("scsp1",0) AM_SHARE("soundram")
	AM_RANGE(0x100000, 0x100fff) AM_DEVREADWRITE_LEGACY("scsp1", scsp_r, scsp_w)
	AM_RANGE(0x200000, 0x27ffff) AM_RAM AM_REGION("scsp2",0) AM_SHARE("soundram2")
	AM_RANGE(0x300000, 0x300fff) AM_DEVREADWRITE_LEGACY("scsp2", scsp_r, scsp_w)
	AM_RANGE(0x800000, 0x80ffff) AM_MIRROR(0x200000) AM_RAM
	AM_RANGE(0x900000, 0x900001) AM_WRITE8(sound_to_sh1_w,0x00ff)
ADDRESS_MAP_END





static GFXDECODE_START( coolridr )
//	GFXDECODE_ENTRY( NULL, 0, tiles16x16_layout, 0, 0x100 )
GFXDECODE_END

static INPUT_PORTS_START( coolridr )
	PORT_START("IN0")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN0-0" )
	PORT_DIPSETTING(    0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "IN0-1" )
	PORT_DIPSETTING(    0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x00000003, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Music <<")
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Music >>")
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Shift Up")
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Shift Down")
	PORT_BIT( 0x000000c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("P1 Coin")
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("P2 Coin")
	PORT_SERVICE_NO_TOGGLE( 0x00040000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P1 Service Switch")
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 Start")
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("P2 Start")
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("P2 Service Switch")
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN2-0" )
	PORT_DIPSETTING(    0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "IN2-1" )
	PORT_DIPSETTING(    0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN3-0" )
	PORT_DIPSETTING(    0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "IN3-1" )
	PORT_DIPSETTING(    0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN4-0" )
	PORT_DIPSETTING(    0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "IN4-1" )
	PORT_DIPSETTING(    0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN5")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN5-0" )
	PORT_DIPSETTING(    0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_BIT( 0x00030000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Music <<")
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Music >>")
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Shift Up")
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Shift Down")
	PORT_BIT( 0x00c00000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN6")
	PORT_DIPNAME( 0x00000001, 0x00000001, "IN6-0" )
	PORT_DIPSETTING(    0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00010000, 0x00010000, "IN6-1" )
	PORT_DIPSETTING(    0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(1) PORT_NAME("P1 Handle Bar")

	PORT_START("AN1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(25) PORT_PLAYER(1) PORT_REVERSE PORT_NAME("P1 Throttle")

	PORT_START("AN2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(25) PORT_PLAYER(1) PORT_REVERSE PORT_NAME("P1 Brake")

	PORT_START("AN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN4")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_PLAYER(2) PORT_NAME("P2 Handle Bar")

	PORT_START("AN5")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(25) PORT_PLAYER(2) PORT_REVERSE PORT_NAME("P2 Throttle")

	PORT_START("AN6")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(25) PORT_PLAYER(2) PORT_REVERSE PORT_NAME("P2 Brake")

	PORT_START("AN7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


// IRQs 4 & 6 are valid on SH-2
TIMER_DEVICE_CALLBACK_MEMBER(coolridr_state::system_h1_main)
{
	int scanline = param;

	if(scanline == 384)
		m_maincpu->set_input_line(4, HOLD_LINE);

	if(scanline == 0)
		m_maincpu->set_input_line(6, HOLD_LINE);

}

TIMER_DEVICE_CALLBACK_MEMBER(coolridr_state::system_h1_sub)
{
	int scanline = param;

	/* 10: reads from 0x4000000 (sound irq) */
	/* 12: reads from inputs (so presumably V-Blank) */
	/* 14: tries to r/w to 0x62***** area (network irq?) */

	if(scanline == 384)
		m_subcpu->set_input_line(0xc, HOLD_LINE);
}




#define READ_COMPRESSED_ROM(chip) \
	m_compressedgfx[(chip)*0x400000 + romoffset] << 8 | m_compressedgfx[(chip)*0x0400000 + romoffset +1]; \

// this helps you feth the 20bit words from an address in the compressed data
UINT32 coolridr_state::get_20bit_data(UINT32 romoffset, int _20bitwordnum)
{
	UINT16 testvalue, testvalue2;

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

UINT16 coolridr_state::get_10bit_data(UINT32 romoffset, int _10bitwordnum)
{
	UINT32 data = get_20bit_data(romoffset, _10bitwordnum>>1);
	if (_10bitwordnum&1) return data & 0x3ff;
	else return (data>>10) & 0x3ff;
}

void coolridr_state::machine_start()
{
	m_compressedgfx = memregion( "compressedgfx" )->base();
	size_t  size    = memregion( "compressedgfx" )->bytes();

	// we're expanding 10bit packed data to 16bits(10 used)
	m_expanded_10bit_gfx = auto_alloc_array(machine(), UINT16, ((size/10)*16)/2);

	for (int i=0;i<(0x800000*8)/2;i++)
	{
		m_expanded_10bit_gfx[i] = get_10bit_data( 0, i);
	}

	// do a rearranged version too with just the 16-bit words in a different order, palettes seem to
	// be referenced this way?!
	m_rearranged_16bit_gfx = auto_alloc_array(machine(), UINT16, size/2);

	UINT16* compressed = (UINT16*)memregion( "compressedgfx" )->base();
	int count = 0;
	for (int i=0;i<size/2/10;i++)
	{
		m_rearranged_16bit_gfx[count+0] = ((compressed[i+((0x0400000/2)*0)]&0x00ff) << 8) | ((compressed[i+((0x0400000/2)*0)]&0xff00) >> 8);
		m_rearranged_16bit_gfx[count+1] = ((compressed[i+((0x0400000/2)*1)]&0x00ff) << 8) | ((compressed[i+((0x0400000/2)*1)]&0xff00) >> 8);
		m_rearranged_16bit_gfx[count+2] = ((compressed[i+((0x0400000/2)*2)]&0x00ff) << 8) | ((compressed[i+((0x0400000/2)*2)]&0xff00) >> 8);
		m_rearranged_16bit_gfx[count+3] = ((compressed[i+((0x0400000/2)*3)]&0x00ff) << 8) | ((compressed[i+((0x0400000/2)*3)]&0xff00) >> 8);
		m_rearranged_16bit_gfx[count+4] = ((compressed[i+((0x0400000/2)*4)]&0x00ff) << 8) | ((compressed[i+((0x0400000/2)*4)]&0xff00) >> 8);
		m_rearranged_16bit_gfx[count+5] = ((compressed[i+((0x0400000/2)*5)]&0x00ff) << 8) | ((compressed[i+((0x0400000/2)*5)]&0xff00) >> 8);
		m_rearranged_16bit_gfx[count+6] = ((compressed[i+((0x0400000/2)*6)]&0x00ff) << 8) | ((compressed[i+((0x0400000/2)*6)]&0xff00) >> 8);
		m_rearranged_16bit_gfx[count+7] = ((compressed[i+((0x0400000/2)*7)]&0x00ff) << 8) | ((compressed[i+((0x0400000/2)*7)]&0xff00) >> 8);
		m_rearranged_16bit_gfx[count+8] = ((compressed[i+((0x0400000/2)*8)]&0x00ff) << 8) | ((compressed[i+((0x0400000/2)*8)]&0xff00) >> 8);
		m_rearranged_16bit_gfx[count+9] = ((compressed[i+((0x0400000/2)*9)]&0x00ff) << 8) | ((compressed[i+((0x0400000/2)*9)]&0xff00) >> 8);
		count+=10;
	}


	if (0)
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"expanded_%s_gfx", machine().system().name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			for (int i=0;i<(0x800000*8);i++)
			{
				fwrite((UINT8*)m_expanded_10bit_gfx+(i^1), 1, 1, fp);
			}
			fclose(fp);

		}
	}

	m_h1_vram = auto_alloc_array_clear(machine(), UINT16, VRAM_SIZE);
	m_h1_pcg = auto_alloc_array_clear(machine(), UINT8, VRAM_SIZE);
	m_h1_pal = auto_alloc_array_clear(machine(), UINT16, VRAM_SIZE);

	save_pointer(NAME(m_h1_vram), VRAM_SIZE);
	save_pointer(NAME(m_h1_pcg), VRAM_SIZE);
	save_pointer(NAME(m_h1_pal), VRAM_SIZE);
}

void coolridr_state::machine_reset()
{
//  machine().device("maincpu")->execute().set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);



//	memcpy(m_soundram, memregion("soundcpu")->base()+0x80000, 0x80000);
//  m_soundcpu->reset();


}

#if 0
static I8237_INTERFACE( dmac_intf )
{
	DEVCB_NULL, //DEVCB_DRIVER_LINE_MEMBER(coolridr_state, coolridr_dma_hrq_changed),
	DEVCB_NULL, //DEVCB_DRIVER_LINE_MEMBER(coolridr_state, coolridr_tc_w),
	DEVCB_NULL, //DEVCB_DRIVER_MEMBER(coolridr_state, coolridr_dma_read_byte),
	DEVCB_NULL,//DEVCB_DRIVER_MEMBER(coolridr_state, coolridr_dma_write_byte),
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL /*DEVCB_DRIVER_LINE_MEMBER(coolridr_state, coolridr_dack0_w)*/,
		DEVCB_NULL/*DEVCB_DRIVER_LINE_MEMBER(coolridr_state, coolridr_dack1_w)*/,
		DEVCB_NULL/*DEVCB_DRIVER_LINE_MEMBER(coolridr_state, coolridr_dack2_w)*/,
		DEVCB_NULL/*DEVCB_DRIVER_LINE_MEMBER(coolridr_state, coolridr_dack3_w)*/ }
};
#endif

static void scsp_irq(device_t *device, int irq)
{
	coolridr_state *state = device->machine().driver_data<coolridr_state>();
	if (irq > 0)
	{
		state->m_scsp_last_line = irq;
		state->m_soundcpu->set_input_line(irq, ASSERT_LINE);
	}
	else
		state->m_soundcpu->set_input_line(-irq, CLEAR_LINE);
}

WRITE_LINE_MEMBER(coolridr_state::scsp1_to_sh1_irq)
{
	m_subcpu->set_input_line(0xe, (state) ? ASSERT_LINE : CLEAR_LINE);
	if(state)
		sound_data |= 0x10;
	else
		sound_data &= ~0x10;
}

WRITE_LINE_MEMBER(coolridr_state::scsp2_to_sh1_irq)
{
	m_subcpu->set_input_line(0xe, (state) ? ASSERT_LINE : CLEAR_LINE);
	if(state)
		sound_data |= 0x20;
	else
		sound_data &= ~0x20;
}

static const scsp_interface scsp_config =
{
	0,
	scsp_irq,
	DEVCB_DRIVER_LINE_MEMBER(coolridr_state, scsp1_to_sh1_irq)
};

static const scsp_interface scsp2_interface =
{
	0,
	NULL,
	DEVCB_DRIVER_LINE_MEMBER(coolridr_state, scsp2_to_sh1_irq)
};

#define MAIN_CLOCK XTAL_28_63636MHz

static MACHINE_CONFIG_START( coolridr, coolridr_state )
	MCFG_CPU_ADD("maincpu", SH2, MAIN_CLOCK)  // 28 mhz
	MCFG_CPU_PROGRAM_MAP(system_h1_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", coolridr_state, system_h1_main, "lscreen", 0, 1)

	MCFG_CPU_ADD("soundcpu", M68000, 11289600) //256 x 44100 Hz = 11.2896 MHz
	MCFG_CPU_PROGRAM_MAP(system_h1_sound_map)

	MCFG_CPU_ADD("sub", SH1, 16000000)  // SH7032 HD6417032F20!! 16 mhz
	MCFG_CPU_PROGRAM_MAP(coolridr_submap)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer2", coolridr_state, system_h1_sub, "lscreen", 0, 1)

//	MCFG_I8237_ADD("i8237", 16000000, dmac_intf)
	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_GFXDECODE(coolridr)

	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(640, 512)
	MCFG_SCREEN_VISIBLE_AREA(CLIPMINX_FULL,CLIPMAXX_FULL, CLIPMINY_FULL, CLIPMAXY_FULL)
	MCFG_SCREEN_UPDATE_DRIVER(coolridr_state, screen_update_coolridr1)

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(640, 512)
	MCFG_SCREEN_VISIBLE_AREA(CLIPMINX_FULL,CLIPMAXX_FULL, CLIPMINY_FULL, CLIPMAXY_FULL)
	MCFG_SCREEN_UPDATE_DRIVER(coolridr_state, screen_update_coolridr2)

	MCFG_PALETTE_LENGTH(0x10000)
	MCFG_DEFAULT_LAYOUT(layout_dualhsxs)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("scsp1", SCSP, 0)
	MCFG_SOUND_CONFIG(scsp_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(0, "rspeaker", 1.0)

	MCFG_SOUND_ADD("scsp2", SCSP, 0)
	MCFG_SOUND_CONFIG(scsp2_interface)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(0, "rspeaker", 1.0)
MACHINE_CONFIG_END

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

	ROM_REGION( 0x100000, "soundcpu", ROMREGION_ERASE00 )   /* 68000 */

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

	ROM_REGION( 0x80000, "scsp1", 0 )   /* first SCSP's RAM */
	ROM_FILL( 0x000000, 0x80000, 0 )

	ROM_REGION( 0x80000, "scsp2", 0 )   /* second SCSP's RAM */
	ROM_FILL( 0x000000, 0x80000, 0 )
ROM_END


/*
TODO: both irq routines writes 1 to 0x60d8894, sets up the Watchdog timer then expect that this buffer goes low IN the irq routines.
	  The Watchdog Timer is setted up with these params:
	  0xee for wtcnt
	  0x39 for wtcsr (enable irq (bit 5), enable timer (bit 4), clock select divider / 64 (bits 2-0))
	  vector is 0x7f (so VBR+0x1fc)
	  level is 0xf
... and indeed the Watchdog irq routine effectively clears this RAM buffer. What the manual doesn't say is that the Watchdog timer irq
    presumably is treated as an NMI by the SH-2 CPU and not really a "normal" irq exception.
    For the record, here's the ITI code snippet:
    06002DE4: 2F36   MOV.L   R3,@-SP
	06002DE6: E300   MOV     #$00,R3
	06002DE8: 2F26   MOV.L   R2,@-SP
	06002DEA: D20B   MOV.L   @($2C,PC),R2
	06002DEC: 2230   MOV.B   R3,@R2 ;writes 0 to the RAM buffer 0x60d8896
	06002DEE: 9305   MOV.W   @($000A,PC),R3
	06002DF0: 9205   MOV.W   @($000A,PC),R2
	06002DF2: 2231   MOV.W   R3,@R2 ;writes 0x19, disables the watchdog timer
	06002DF4: 62F6   MOV.L   @SP+,R2
	06002DF6: 63F6   MOV.L   @SP+,R3
	06002DF8: 002B   RTE
	06002DFA: 0009   NOP

*/
READ32_MEMBER(coolridr_state::coolridr_hack2_r)
{
	offs_t pc = downcast<cpu_device *>(&space.device())->pc();


	if(pc == 0x6002cba || pc == 0x6002d42)
		return 0;

	// with the non-recompiler pc returns +2
	if(pc == 0x06002cbc || pc == 0x06002d44)
		return 0;

	return m_sysh1_workram_h[0xd8894/4];
}



DRIVER_INIT_MEMBER(coolridr_state,coolridr)
{
	machine().device("maincpu")->memory().space(AS_PROGRAM).install_read_handler(0x60d8894, 0x060d8897, read32_delegate(FUNC(coolridr_state::coolridr_hack2_r), this));


	sh2drc_set_options(machine().device("maincpu"), SH2DRC_FASTEST_OPTIONS);
	sh2drc_set_options(machine().device("sub"), SH2DRC_FASTEST_OPTIONS);
}

GAME( 1995, coolridr,    0, coolridr,    coolridr, coolridr_state,    coolridr, ROT0,  "Sega", "Cool Riders",GAME_NOT_WORKING|GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS ) // region is set in test mode, this set is for Japan, USA and Export (all regions)

