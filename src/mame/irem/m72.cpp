// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

IREM M72 board

driver by Nicola Salmoria
protection information by Nao

board differences

M72 - 3 board stack, 2 known variants

      M72-B-D (bottom) / M72-A-C (middle) / M72-ROM-C (top)

      This is the original hardware used by R-type
      Z80 program uploaded to RAM rather than having a ROM
      each of the 2 tile layers uses it's own set of ROMs.
      Flip bits are with the tile num, so 0x3fff max tiles
      per layer

      M72-B-D (bottom) / M72-A-C (middle) / M72-C-A (top)

      This is used by all other M72 games, adds support
      for an I8751 MCU and sample playback


M81 - 2 PCB Stack

      M81-A-B (top board) (seen on Dragon Breed)
      CPUs, program roms etc.

      M81-B-B (bottom board) (seen on Dragon Breed)
      supports
      8 sprite ROMS
      4 tile roms for FG layer (A0-A3)
       (Jumper J3 also allows them to be used for BG)
      4 tile roms for BG layer (B0-B3)

      The Jumper at J3 seems to be an important difference
      from M72, it allows both the FG and BG layers to
      operate from a single set of ROMs.
      W - use both sets of ROMs
      S - use a single set of ROMs (A0-A3)


      revised hardware, Z80 uses a ROM, no MCU, same video
      system as M72 (some layer offsets - why?)


M82 - board made for Major Title, Z80 has a rom, no MCU
      has an extra sprite layer, rowscroll, and a larger
      tilemap.  Tile data from both tile layers now comes
      from a single set of ROMs, flip bits moved to 2nd
      word meaning max of 0xffff tiles.

      * Some games were converted to run on this board,
      leaving the extra sprite HW unused.

M84 -   2 PCB stack
        functionally same as M82 but without the extra sprite hw??

        M84-A-A (bottom board) (most games)
        supports
        4 program roms
        8 tile roms
        1 snd prg, 1 voice rom
        CPUs and some customs etc.

        M84-D-B (bottom board) (found on lightning swords / kengo)
        redesigned version of above but
        for V35 CPU? (seems to lack the UPD71059C interrupt
        controller which isn't needed when with the V35)

        M84-C-A (top board) (listed as for Hammering Harry)
        4 sprite roms (in a row)
        6 larger chips with detail removed
        etc.

        M84-B-A (top board) (found on rytpe 2)
        M84-B-B (top board) (lightning swords / kengo)
        these both look very similar, if not the same

        4 sprite roms (in a square)
        various NANAO marked customs
        KNA70H016(12)  NANAO 0201
        KNA65005 17 NANAO 9048KS
        KNA71H010(15) NANAO 0X2002
        KNA72H010(14) NANAO 0Z2001
        KNA71H009(13) NANAO 122001
        KNA70H015(11) NANAO 092002
        KNA91H014 NANAO 0Z2001V
        etc.



M85 - Pound for Pound uses this, possibly just M84 with
      a modified sound section?
      - most Jamma inputs not connected, trackball only


                                   Year Board                Protected?
R-Type                             1987  M72                 N
Battle Chopper / Mr. Heli          1987  M72                 Y
Ninja Spirit / Saigo no Nindou     1988  M72                 Y
Image Fight (World)                1988  M72                 Y
Image Fight (Japan)                1988  M72                 Y
Legend of Hero Tonma               1989  M72                 Y
X Multiply (World)                 1989  M81-A-B + M81-B-B   N
X Multiply (Japan)                 1989  M72                 Y
Dragon Breed                       1989  M81-A-B + M81-B-B   N
Dragon Breed (World)               1989  M72                 Y
Dragon Breed (Japan)               1989  M72                 Y
R-Type II                          1989  M84-A-A + M84-B-A   N
Major Title                        1990  M82-A-A + M82-B-A   N
Hammerin' Harry (World ver)        1990  M81-A-B + M81-B-B   N
Hammerin' H..(US)/ Daiku no Gensan 1990  M84-A-A + M84-C-A   N
                   Daiku no Gensan 1990  M72                 Y
                   Daiku no Gensan 1990  M82-A-A + M82-B-A   N
Pound for Pound                    1990  M85-A-B + M85-B     N
Air Duel (World)                   1990  M82-A-A + M82-B-A   N
Air Duel (World)                   1990  M72                 Y
Air Duel (Japan)                   1990  M72                 Y
Cosmic Cop /                       1991  M84-D-B + M84-B-B   N
  Gallop - Armed Police Unit       1991  M72                 Y (sample playback only)
Ken-Go / Lightning Swords          1991  M84-D-B + M84-B-B   Encrypted


Rtype / Rtype 2 are often misreported as being M82 games, this is mostly
due to the unofficial conversions that have become widespread, see:
http://www.paulswan.me/arcade/m82-m72.htm (Rtype on M82, extensive wiremods)
http://www.paulswan.me/arcade/m82-m84.htm (Rtype 2 on M82)
these are supported as 'rtypem82b' and 'rtype2m82b' although the former
still needs work because the wiremods change same of the behavior to be
more like an M72 PCB than an M82


TODO:
- m82_gfx_ctrl_w is unknown, it seems to be used to disable rowscroll,
  and maybe other things

- Maybe there is a layer enable register, e.g. nspirit shows (for an instant)
  incomplete screens with bad colors when you start a game.

- A lot of unknown I/O writes from the sound CPU in Pound for Pound.

- the sprite chip triggers IRQ1 when it has finished copying the sprite RAM to its
  private buffer. This isn't implemented (all games have an empty IRQ1 handler).
  The cpu board also has support for IRQ3 and IRQ4, coming from the external
  connectors, but I don't think they are used by any game.

- excessive transmask difference between m72 games, this must be user selectable somehow;

IRQ controller
--------------
The IRQ controller is a UPD71059C

The initialization consists of one write to port 0x40 and multiple writes
(2 or 3) to port 0x42. The first value written to 0x42 is the IRQ vector base.
Cosmic Cop and Ken-Go have a V35 CPU with its own IRQ controller built in.

Game      irqbase 0x40  0x42
----      ------- ----  ----------
rtype       0x20   17    20 0F
bchopper     "     "     "
nspirit      "     "     "
loht         "     "     "
rtype2       "     "     "
airduel      "     "     "
gallop       "     "     "
imgfight    0x20   17    20 0F 06
majtitle     "     "     "
poundfor    0x20   17    20 0F 0A
xmultipl    0x08   13    08 0F FA
dbreed       "     "     "
hharry       "     "     "
cosmccop    ---------------------
kengo       ---------------------


2008-08
Dip locations verified for the following games:
    - dbreed, hharry, loht (jpn), poundfor, rtype, rtype2 [manual]
    - airduel, gallop, imgfight [dip listing]
The other locations have been added assuming that the layout is the same
on all m72 boards. However, it would be nice to have them confirmed for
other supported games as well.


***************************************************************************/

#include "emu.h"
#include "m72.h"
#include "iremipt.h"

#include "cpu/nec/nec.h"
#include "cpu/nec/v25.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "irem_cpu.h"
#include "machine/rstbuf.h"
#include "sound/ymopm.h"
#include "speaker.h"


#define MASTER_CLOCK        XTAL(32'000'000)
#define SOUND_CLOCK         XTAL(3'579'545)



/***************************************************************************/

void m72_state::machine_start()
{
	m_scanline_timer = timer_alloc(FUNC(m72_state::scanline_interrupt), this);
}

MACHINE_START_MEMBER(m72_state,kengo)
{
	m_scanline_timer = timer_alloc(FUNC(m72_state::kengo_scanline_interrupt), this);
}

TIMER_CALLBACK_MEMBER(m72_state::synch_callback)
{
	//machine().scheduler().perfect_quantum(attotime::from_usec(8000000));
	machine().scheduler().add_quantum(attotime::from_hz(MASTER_CLOCK/4/12), attotime::from_seconds(25));
}

void m72_state::machine_reset()
{
	m_mcu_sample_addr = 0;
	//m_mcu_snd_cmd_latch = 0;

	m_scanline_timer->adjust(m_screen->time_until_pos(0));
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(m72_state::synch_callback),this));

	// Hold sound CPU in reset if main CPU has to upload the program into RAM
	if (m_soundram.found())
		m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

MACHINE_RESET_MEMBER(m72_state,kengo)
{
	m_scanline_timer->adjust(m_screen->time_until_pos(0));
}

TIMER_CALLBACK_MEMBER(m72_state::scanline_interrupt)
{
	int scanline = param;

	/* raster interrupt - visible area only? */
	if (scanline < 256 && scanline == m_raster_irq_position - 128)
	{
		m_upd71059c->ir2_w(1);
	}

	/* VBLANK interrupt */
	if (scanline == 256)
	{
		m_upd71059c->ir0_w(1);
		m_upd71059c->ir2_w(0);
	}
	else
	{
		m_upd71059c->ir0_w(0);
	}

	/* adjust for next scanline */
	if (++scanline >= m_screen->height())
		scanline = 0;
	m_scanline_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}

TIMER_CALLBACK_MEMBER(m72_state::kengo_scanline_interrupt)
{
	int scanline = param;

	/* raster interrupt - visible area only? */
	if (scanline < 256 && scanline == m_raster_irq_position - 128)
	{
		m_screen->update_partial(scanline);
		m_maincpu->set_input_line(NEC_INPUT_LINE_INTP2, ASSERT_LINE);
	}
	else
		m_maincpu->set_input_line(NEC_INPUT_LINE_INTP2, CLEAR_LINE);

	/* VBLANK interrupt */
	if (scanline == 256)
	{
		m_screen->update_partial(scanline);
		m_maincpu->set_input_line(NEC_INPUT_LINE_INTP0, ASSERT_LINE);
	}
	else
		m_maincpu->set_input_line(NEC_INPUT_LINE_INTP0, CLEAR_LINE);

	/* adjust for next scanline */
	if (++scanline >= m_screen->height())
		scanline = 0;
	m_scanline_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}

/***************************************************************************

Protection emulation

The protection device does

* provide startup code
* provide checksums
* feed samples to the sound cpu

***************************************************************************/

TIMER_CALLBACK_MEMBER(m72_state::delayed_ram16_w)
{
	const u16 val = param & 0xffff;
	const u16 offset = (param >> 16) & 0x07ff;
	const u16 mem_mask = (BIT(param, 28) ? 0xff00 : 0x0000) | (BIT(param, 27) ? 0x00ff : 0x0000);

	logerror("MB8421/MB8431 left_w(0x%03x, 0x%04x, 0x%04x)\n", offset, val, mem_mask);
	m_dpram->left_w(offset, val, mem_mask);
}

TIMER_CALLBACK_MEMBER(m72_state::delayed_ram8_w)
{
	const u8 val = param & 0xff;
	const u16 offset = (param >> 9) & 0x07ff;

	if (BIT(param, 8))
		m_dpram->right_w(offset, val << 8, 0xff00);
	else
		m_dpram->right_w(offset, val, 0x00ff);
}


void m72_state::main_mcu_w(offs_t offset, u16 data, u16 mem_mask)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(m72_state::delayed_ram16_w), this), offset << 16 | data | (mem_mask & 0x0180) << 20);
}

void m72_state::mcu_data_w(offs_t offset, u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(m72_state::delayed_ram8_w), this), offset << 8 | u32(data));
}

u8 m72_state::mcu_data_r(offs_t offset)
{
	return (m_dpram->right_r(offset >> 1) >> (BIT(offset, 0) ? 8 : 0)) & 0xff;
}

u8 m72_state::mcu_sample_r()
{
	const u8 sample = m_samples_region[m_mcu_sample_addr];
	if (!machine().side_effects_disabled())
		m_mcu_sample_addr++;
	return sample;
}

void m72_state::mcu_low_w(u8 data)
{
	m_mcu_sample_addr = (m_mcu_sample_addr & 0xffe000) | (data<<5);
	logerror("low: %02x %08x\n", data, m_mcu_sample_addr);
}

void m72_state::mcu_high_w(u8 data)
{
	m_mcu_sample_addr = (m_mcu_sample_addr & 0x1fff) | (data<<(8+5));
	logerror("high: %02x %08x\n", data, m_mcu_sample_addr);
}

void m72_state::init_m72_8751()
{
	save_item(NAME(m_mcu_sample_addr));

	/* lohtb2 */
#if 0
	/* running the mcu at twice the speed, the following
	 * timeouts have to be modified.
	 * At normal speed, the timing heavily depends on opcode
	 * prefetching on the V30.
	 */
	{
		u8 *rom=memregion("mcu")->base();

		rom[0x12d+5] += 1; printf(" 5: %d\n", rom[0x12d+5]);
		rom[0x12d+8] += 5;  printf(" 8: %d\n", rom[0x12d+8]);
		rom[0x12d+11] += 7; printf("11: %d\n", rom[0x12d+11]);
		rom[0x12d+14] += 9; printf("14: %d\n", rom[0x12d+14]);
		rom[0x12d+17] += 1; printf("17: %d\n", rom[0x12d+17]);
		rom[0x12d+20] += 10; printf("20: %d\n", rom[0x12d+20]);
		rom[0x12d+23] += 3; printf("23: %d\n", rom[0x12d+23]);
		rom[0x12d+26] += 2; printf("26: %d\n", rom[0x12d+26]);
		rom[0x12d+29] += 2; printf("29: %d\n", rom[0x12d+29]);
		rom[0x12d+32] += 16; printf("32: %d\n", rom[0x12d+32]);
	}
#endif
}


/***************************************************************************

Sample playback

In the later games, the sound CPU can program the start offset of the PCM
samples, but it seems the earlier games have them hardcoded somewhere (maybe
the protection MCU?).
So, here I provided some tables with the start offset precomputed.
They could be built automatically in most cases (00 marks the end of a
sample), but a couple of games (nspirit, loht) have holes in the numbering
so we would have to do them differently anyway.

Also, some games (dkgenm72, poundfor, airduel, gallop) have an empty NMI
handler, so the sample playback has to be handled entirely by external
hardware; we work around that by using (for all games, not just the ones
without a NMI handler) a NMI interrupt gen that mimics the behaviour of
the NMI handler in the other games.

***************************************************************************/

#if 0
int m72_state::find_sample(int num)
{
	int len = m_samples_region.length();
	int addr = 0;

	while (num--)
	{
		/* find end of sample */
		while (addr < len &&  m_samples_region[addr]) addr++;

		/* skip 0 filler between samples */
		while (addr < len && !m_samples_region[addr]) addr++;
	}

	return addr;
}
#endif

INTERRUPT_GEN_MEMBER(m72_state::fake_nmi)
{
	int sample = m_audio->sample_r();
	if (sample)
		m_audio->sample_w(sample);
}


void m72_state::nspirit_sample_trigger_w(offs_t offset, u16 data, u16 mem_mask)
{
	static const int a[9] = { 0x0000, 0x0020, 0x2020, 0, 0x5720, 0, 0x7b60, 0x9b60, 0xc360 };
	if (ACCESSING_BITS_0_7 && (data & 0xff) < 9) m_audio->set_sample_start(a[data & 0xff]);
}

void m72_state::dbreedm72_sample_trigger_w(offs_t offset, u16 data, u16 mem_mask)
{
	static const int a[9] = { 0x00000, 0x00020, 0x02c40, 0x08160, 0x0c8c0, 0x0ffe0, 0x13000, 0x15820, 0x15f40 };
	if (ACCESSING_BITS_0_7 && (data & 0xff) < 9) m_audio->set_sample_start(a[data & 0xff]);
}

void m72_state::dkgenm72_sample_trigger_w(offs_t offset, u16 data, u16 mem_mask)
{
	static const int a[28] = {
		0x00000, 0x00020, 0x01800, 0x02da0, 0x03be0, 0x05ae0, 0x06100, 0x06de0,
		0x07260, 0x07a60, 0x08720, 0x0a5c0, 0x0c3c0, 0x0c7a0, 0x0e140, 0x0fb00,
		0x10fa0, 0x10fc0, 0x10fe0, 0x11f40, 0x12b20, 0x130a0, 0x13c60, 0x14740,
		0x153c0, 0x197e0, 0x1af40, 0x1c080 };

	if (ACCESSING_BITS_0_7 && (data & 0xff) < 28) m_audio->set_sample_start(a[data & 0xff]);
}



/***************************************************************************

Protection simulation

Most of the games running on this board have an 8751 protection mcu.
It is not known how it works in detail, however it's pretty clear that it
shares RAM at b0000-b0fff.
On startup, the game writes a pattern to the whole RAM, then reads it back
expecting it to be INVERTED. If it isn't, it reports a RAM error.
If the RAM passes the test, the program increments every byte up to b0ffb,
then calls a subroutine at b0000, which has to be provided by the mcu.
It seems that this routine is not supposed to RET, but instead it should
jump directly to the game entry point. The routine should also write some
bytes here and there in RAM (different in every game); those bytes are
checked at various points during the game, causing a crash if they aren't
right.
Note that the program keeps incrementing b0ffe while the game is running,
maybe this is done to keep the 8751 alive. We don't bother with that.

Finally, to do the ROM test the program asks the mcu to provide the correct
values. This is done only in service, so doesn't seem to be much of a
protection. Here we have provided the correct crcs for the available dumps,
of course there is no guarantee that they are actually good.

All the protection routines below are entirely made up. They get the games
running, but they have not been derived from the real 8751 code.

***************************************************************************/

#define CODE_LEN 96
#define CRC_LEN 18


/* Ninja Spirit (World, M72 hardware) */
static const u8 nspirit_code[CODE_LEN] =
{
	0x68,0x00,0xa0,             // push 0a000h
	0x1f,                       // pop ds
	0xc6,0x06,0x38,0x38,0x4e,   // mov [3838h], byte 04eh
	0xc6,0x06,0x3a,0x38,0x49,   // mov [383ah], byte 049h
	0xc6,0x06,0x3c,0x38,0x4e,   // mov [383ch], byte 04eh
	0xc6,0x06,0x3e,0x38,0x44,   // mov [383eh], byte 044h
	0xc6,0x06,0x40,0x38,0x4f,   // mov [3840h], byte 04fh
	0xc6,0x06,0x42,0x38,0x55,   // mov [3842h], byte 055h
	0x68,0x00,0xb0,             // push 0b000h
	0x1f,                       // pop ds
	0xc6,0x06,0x00,0x09,0x49^0xff,  // mov [0900h], byte 049h
	0xc6,0x06,0x00,0x0a,0x49^0xff,  // mov [0a00h], byte 049h
	0xc6,0x06,0x00,0x0b,0x49^0xff,  // mov [0b00h], byte 049h
	0x68,0x00,0xd0,             // push 0d000h
	0x1f,                       // pop ds
	0xea,0x00,0x00,0x40,0x00    // jmp  0040:$0000
};
static const u8 nspirit_crc[CRC_LEN] =   {   0xfe,0x94,0x6e,0x4e, 0xc8,0x33,0xa7,0x2d,
												0xf2,0xa3,0xf9,0xe1, 0xa9,0x6c,0x02,0x95, 0x00,0x00 };

/* Dragon Breed (World, M72 hardware) */
static const u8 dbreedm72_code[CODE_LEN] =
{
	0xea,0x6c,0x00,0x00,0x00    // jmp  0000:$006c
};
static const u8 dbreedm72_crc[CRC_LEN] =   { 0xa4,0x96,0x5f,0xc0, 0xab,0x49,0x9f,0x19,
												0x84,0xe6,0xd6,0xca, 0x00,0x00 };

/* Daiku no Gensan */
static const u8 dkgenm72_code[CODE_LEN] =
{
	0xea,0x3d,0x00,0x00,0x10    // jmp  1000:$003d
};
static const u8 dkgenm72_crc[CRC_LEN] =  {   0xc8,0xb4,0xdc,0xf8, 0xd3,0xba,0x48,0xed,
												0x79,0x08,0x1c,0xb3, 0x00,0x00 };



void m72_state::copy_le(u16 *dest, const u8 *src, u8 bytes)
{
	int i;

	for (i = 0; i < bytes; i += 2)
		dest[i/2] = src[i+0] | (src[i+1] << 8);
}

u16 m72_state::protection_r(offs_t offset, u16 mem_mask)
{
	if (ACCESSING_BITS_8_15)
		copy_le(m_protection_ram.get(),m_protection_code,CODE_LEN);
	return m_protection_ram[0xffa/2+offset];
}

void m72_state::protection_w(offs_t offset, u16 data, u16 mem_mask)
{
	data ^= 0xffff;
	COMBINE_DATA(&m_protection_ram[offset]);
	data ^= 0xffff;

	if (offset == 0x0fff/2 && ACCESSING_BITS_8_15 && (data >> 8) == 0)
		copy_le(&m_protection_ram[0x0fe0],m_protection_crc,CRC_LEN);
}

void m72_state::install_protection_handler(const u8 *code,const u8 *crc)
{
	m_protection_ram = std::make_unique<u16[]>(0x1000/2);
	m_protection_code = code;
	m_protection_crc =  crc;
	m_maincpu->space(AS_PROGRAM).install_rom(0xb0000, 0xb0fff, m_protection_ram.get());
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xb0ffa, 0xb0ffb, read16s_delegate(*this, FUNC(m72_state::protection_r)));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xb0000, 0xb0fff, write16s_delegate(*this, FUNC(m72_state::protection_w)));

	save_pointer(NAME(m_protection_ram), 0x1000/2);
}

void m72_state::init_nspirit()
{
	install_protection_handler(nspirit_code,nspirit_crc);
	m_maincpu->space(AS_IO).install_write_handler(0xc0, 0xc1, write16s_delegate(*this, FUNC(m72_state::nspirit_sample_trigger_w)));
}

void m72_state::init_dbreedm72()
{
	install_protection_handler(dbreedm72_code,dbreedm72_crc);
	m_maincpu->space(AS_IO).install_write_handler(0xc0, 0xc1, write16s_delegate(*this, FUNC(m72_state::dbreedm72_sample_trigger_w)));
}

void m72_state::init_dkgenm72()
{
	install_protection_handler(dkgenm72_code,dkgenm72_crc);
	m_maincpu->space(AS_IO).install_write_handler(0xc0, 0xc1, write16s_delegate(*this, FUNC(m72_state::dkgenm72_sample_trigger_w)));
}


template<unsigned N>
u16 m72_state::palette_r(offs_t offset)
{
	/* A9 isn't connected, so 0x200-0x3ff mirrors 0x000-0x1ff etc. */
	offset &= ~0x100;

	return m_paletteram[N][offset] | 0xffe0;    /* only D0-D4 are connected */
}

inline void m72_state::changecolor(offs_t color, u8 r, u8 g, u8 b)
{
	m_palette->set_pen_color(color,pal5bit(r),pal5bit(g),pal5bit(b));
}

template<unsigned N>
void m72_state::palette_w(offs_t offset, u16 data, u16 mem_mask)
{
	/* A9 isn't connected, so 0x200-0x3ff mirrors 0x000-0x1ff etc. */
	offset &= ~0x100;

	COMBINE_DATA(&m_paletteram[N][offset]);
	offset &= 0x0ff;
	changecolor(offset + (N << 8),
			m_paletteram[N][offset + 0x000],
			m_paletteram[N][offset + 0x200],
			m_paletteram[N][offset + 0x400]);
}

template<unsigned N>
void m72_state::scrollx_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_screen->update_partial(m_screen->vpos());
	COMBINE_DATA(&m_scrollx[N]);
}

template<unsigned N>
void m72_state::scrolly_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_screen->update_partial(m_screen->vpos());
	COMBINE_DATA(&m_scrolly[N]);
}


u8 m72_state::soundram_r(offs_t offset)
{
	return m_soundram[offset];
}

void m72_state::soundram_w(offs_t offset, u8 data)
{
	m_soundram[offset] = data;
}

void m72_state::m72_cpu1_common_map(address_map &map)
{
	map(0xc0000, 0xc03ff).ram().share("spriteram");
	map(0xc8000, 0xc8bff).rw(FUNC(m72_state::palette_r<0>), FUNC(m72_state::palette_w<0>)).share("paletteram1");
	map(0xcc000, 0xccbff).rw(FUNC(m72_state::palette_r<1>), FUNC(m72_state::palette_w<1>)).share("paletteram2");
	map(0xd0000, 0xd3fff).ram().w(FUNC(m72_state::videoram1_w)).share("videoram1");
	map(0xd8000, 0xdbfff).ram().w(FUNC(m72_state::videoram2_w)).share("videoram2");
	map(0xe0000, 0xeffff).rw(FUNC(m72_state::soundram_r), FUNC(m72_state::soundram_w));
	map(0xffff0, 0xfffff).rom();
}

void m72_state::m72_map(address_map &map)
{
	m72_cpu1_common_map(map);
	map(0x00000, 0x7ffff).rom();
	map(0xa0000, 0xa3fff).ram();    /* work RAM */
}

void m72_state::rtype_map(address_map &map)
{
	m72_cpu1_common_map(map);
	map(0x00000, 0x3ffff).rom();
	map(0x40000, 0x43fff).ram();    /* work RAM */
}

void m72_state::m72_protected_map(address_map &map)
{
	m72_map(map);
	map(0xb0000, 0xb0fff).r(m_dpram, FUNC(mb8421_mb8431_16_device::left_r)).w(FUNC(m72_state::main_mcu_w));
}

void m72_state::xmultiplm72_map(address_map &map)
{
	m72_cpu1_common_map(map);
	map(0x00000, 0x7ffff).rom();
	map(0x80000, 0x83fff).ram();    /* work RAM */
	map(0xb0000, 0xb0fff).r(m_dpram, FUNC(mb8421_mb8431_16_device::left_r)).w(FUNC(m72_state::main_mcu_w));
}

void m72_state::dbreedwm72_map(address_map &map)
{
	m72_cpu1_common_map(map);
	map(0x00000, 0x7ffff).rom();
	map(0x90000, 0x93fff).ram();    /* work RAM */
}

void m72_state::dbreedm72_map(address_map &map)
{
	m72_cpu1_common_map(map);
	map(0x00000, 0x7ffff).rom();
	map(0x90000, 0x93fff).ram();    /* work RAM */
	map(0xb0000, 0xb0fff).r(m_dpram, FUNC(mb8421_mb8431_16_device::left_r)).w(FUNC(m72_state::main_mcu_w));
}

void m72_state::m81_cpu1_common_map(address_map &map)
{
	map(0x00000, 0x7ffff).rom();
	map(0xb0ffe, 0xb0fff).nopw(); /* leftover from protection?? */
	map(0xc0000, 0xc03ff).ram().share("spriteram");
	map(0xc8000, 0xc8bff).rw(FUNC(m72_state::palette_r<0>), FUNC(m72_state::palette_w<0>)).share("paletteram1");
	map(0xcc000, 0xccbff).rw(FUNC(m72_state::palette_r<1>), FUNC(m72_state::palette_w<1>)).share("paletteram2");
	map(0xd0000, 0xd3fff).ram().w(FUNC(m72_state::videoram1_w)).share("videoram1");
	map(0xd8000, 0xdbfff).ram().w(FUNC(m72_state::videoram2_w)).share("videoram2");
	map(0xffff0, 0xfffff).rom();
}

void m72_state::xmultipl_map(address_map &map)
{
	m81_cpu1_common_map(map);
	map(0x9c000, 0x9ffff).ram();    /* work RAM */
}

void m72_state::dbreed_map(address_map &map)
{
	m81_cpu1_common_map(map);
	map(0x88000, 0x8bfff).ram();    /* work RAM */
}

void m72_state::hharry_map(address_map &map)
{
	m81_cpu1_common_map(map);
	map(0xa0000, 0xa3fff).ram();    /* work RAM */
}

void m72_state::m84_cpu1_common_map(address_map &map)
{
	map(0x00000, 0x7ffff).rom();
	map(0xb0000, 0xb0001).w(FUNC(m72_state::irq_line_w));
	map(0xb4000, 0xb4001).nopw();  /* ??? */
	map(0xbc000, 0xbc000).w(FUNC(m72_state::dmaon_w));
	map(0xb0ffe, 0xb0fff).nopw(); /* leftover from protection?? */
	map(0xc0000, 0xc03ff).ram().share("spriteram");
	map(0xe0000, 0xe3fff).ram();   /* work RAM */
	map(0xffff0, 0xfffff).rom();
}

void m72_state::rtype2_map(address_map &map)
{
	m84_cpu1_common_map(map);
	map(0xd0000, 0xd3fff).ram().w(FUNC(m72_state::videoram1_w)).share("videoram1");
	map(0xd4000, 0xd7fff).ram().w(FUNC(m72_state::videoram2_w)).share("videoram2");
	map(0xc8000, 0xc8bff).rw(FUNC(m72_state::palette_r<0>), FUNC(m72_state::palette_w<0>)).share("paletteram1");
	map(0xd8000, 0xd8bff).rw(FUNC(m72_state::palette_r<1>), FUNC(m72_state::palette_w<1>)).share("paletteram2");
}

void m72_state::hharryu_map(address_map &map)
{
	m84_cpu1_common_map(map);
	map(0xd0000, 0xd3fff).ram().w(FUNC(m72_state::videoram1_w)).share("videoram1");
	map(0xd4000, 0xd7fff).ram().w(FUNC(m72_state::videoram2_w)).share("videoram2");
	map(0xa0000, 0xa0bff).rw(FUNC(m72_state::palette_r<0>), FUNC(m72_state::palette_w<0>)).share("paletteram1");
	map(0xa8000, 0xa8bff).rw(FUNC(m72_state::palette_r<1>), FUNC(m72_state::palette_w<1>)).share("paletteram2");
}

void m72_state::kengo_map(address_map &map)
{
	m84_cpu1_common_map(map);
	map(0x80000, 0x83fff).ram().w(FUNC(m72_state::videoram1_w)).share("videoram1");
	map(0x84000, 0x87fff).ram().w(FUNC(m72_state::videoram2_w)).share("videoram2");
	map(0xa0000, 0xa0bff).rw(FUNC(m72_state::palette_r<0>), FUNC(m72_state::palette_w<0>)).share("paletteram1");
	map(0xa8000, 0xa8bff).rw(FUNC(m72_state::palette_r<1>), FUNC(m72_state::palette_w<1>)).share("paletteram2");
}


void m72_state::m82_map(address_map &map)
{
	map(0x00000, 0x7ffff).rom();
	map(0xa0000, 0xa03ff).ram().share("majtitle_rowscr");
	map(0xa4000, 0xa4bff).rw(FUNC(m72_state::palette_r<1>), FUNC(m72_state::palette_w<1>)).share("paletteram2");
	map(0xac000, 0xaffff).ram().w(FUNC(m72_state::videoram1_w)).share("videoram1");
	map(0xb0000, 0xbffff).ram().w(FUNC(m72_state::videoram2_w)).share("videoram2");  /* larger than the other games */
	map(0xc0000, 0xc03ff).ram().share("spriteram");
	map(0xc8000, 0xc83ff).ram().share("spriteram2");
	map(0xcc000, 0xccbff).rw(FUNC(m72_state::palette_r<0>), FUNC(m72_state::palette_w<0>)).share("paletteram1");
	map(0xd0000, 0xd3fff).ram();   /* work RAM */
	map(0xe0000, 0xe0001).w(FUNC(m72_state::irq_line_w));
	map(0xe4000, 0xe4001).nopw(); /* playfield enable? 1 during screen transitions, 0 otherwise */
	map(0xec000, 0xec000).w(FUNC(m72_state::dmaon_w));
	map(0xffff0, 0xfffff).rom();
}

void m72_state::lohtb_map(address_map &map) // all to be checked
{
	map(0x00000, 0x7ffff).rom();
	map(0x80000, 0x803ff).ram().share("spriteram");
	map(0xa0000, 0xa3fff).ram();    /* work RAM */
	map(0xaa800, 0xaafff).ram();
	map(0xc8000, 0xc8bff).rw(FUNC(m72_state::palette_r<0>), FUNC(m72_state::palette_w<0>)).share("paletteram1");
	map(0xcc000, 0xccbff).rw(FUNC(m72_state::palette_r<1>), FUNC(m72_state::palette_w<1>)).share("paletteram2");
	map(0xd0000, 0xd3fff).ram().w(FUNC(m72_state::videoram1_w)).share("videoram1");
	map(0xd8000, 0xdbfff).ram().w(FUNC(m72_state::videoram2_w)).share("videoram2");
	map(0xffff0, 0xfffff).rom();
}

void m72_state::m72_portmap(address_map &map)
{
	map(0x00, 0x01).portr("IN0");
	map(0x02, 0x03).portr("IN1");
	map(0x04, 0x05).portr("DSW");
	map(0x00, 0x00).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x02, 0x02).w(FUNC(m72_state::port02_w)); /* coin counters, reset sound cpu, other stuff? */
	map(0x04, 0x04).w(FUNC(m72_state::dmaon_w));
	map(0x06, 0x07).w(FUNC(m72_state::irq_line_w));
	map(0x40, 0x43).rw(m_upd71059c, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x80, 0x81).w(FUNC(m72_state::scrolly_w<0>));
	map(0x82, 0x83).w(FUNC(m72_state::scrollx_w<0>));
	map(0x84, 0x85).w(FUNC(m72_state::scrolly_w<1>));
	map(0x86, 0x87).w(FUNC(m72_state::scrollx_w<1>));
/*  { 0xc0, 0xc0      trigger sample, filled by init_ function */
}

void m72_state::m72_protected_portmap(address_map &map)
{
	m72_portmap(map);
	map(0xc0, 0xc0).w("soundlatch2", FUNC(generic_latch_8_device::write));
}

void m72_state::m72_airduel_portmap(address_map &map)
{
	m72_portmap(map);
	map(0xc0, 0xc0).w("mculatch", FUNC(generic_latch_8_device::write));
}

void m72_state::m84_portmap(address_map &map)
{
	map(0x00, 0x01).portr("IN0");
	map(0x02, 0x03).portr("IN1");
	map(0x04, 0x05).portr("DSW");
	map(0x00, 0x00).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x02, 0x02).w(FUNC(m72_state::rtype2_port02_w));
	map(0x40, 0x43).rw(m_upd71059c, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x80, 0x81).w(FUNC(m72_state::scrolly_w<0>));
	map(0x82, 0x83).w(FUNC(m72_state::scrollx_w<0>));
	map(0x84, 0x85).w(FUNC(m72_state::scrolly_w<1>));
	map(0x86, 0x87).w(FUNC(m72_state::scrollx_w<1>));
}

void m72_state::m84_v33_portmap(address_map &map)
{
	map(0x00, 0x01).portr("IN0");
	map(0x02, 0x03).portr("IN1");
	map(0x04, 0x05).portr("DSW");
	map(0x00, 0x00).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x02, 0x02).w(FUNC(m72_state::rtype2_port02_w));
	map(0x80, 0x81).w(FUNC(m72_state::scrolly_w<0>));
	map(0x82, 0x83).w(FUNC(m72_state::scrollx_w<0>));
	map(0x84, 0x85).w(FUNC(m72_state::scrolly_w<1>));
	map(0x86, 0x87).w(FUNC(m72_state::scrollx_w<1>));
//  map(0x8c, 0x8f).nopw();    /* ??? */
}


void m72_state::poundfor_portmap(address_map &map)
{
	map(0x02, 0x03).portr("IN1");
	map(0x04, 0x05).portr("DSW");
	map(0x08, 0x0f).r("upd4701l", FUNC(upd4701_device::read_xy)).umask16(0x00ff);
	map(0x08, 0x0f).r("upd4701h", FUNC(upd4701_device::read_xy)).umask16(0xff00);
	map(0x00, 0x00).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x02, 0x02).w(FUNC(m72_state::poundfor_port02_w));
	map(0x40, 0x43).rw(m_upd71059c, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x80, 0x81).w(FUNC(m72_state::scrolly_w<0>));
	map(0x82, 0x83).w(FUNC(m72_state::scrollx_w<0>));
	map(0x84, 0x85).w(FUNC(m72_state::scrolly_w<1>));
	map(0x86, 0x87).w(FUNC(m72_state::scrollx_w<1>));
}

void m72_state::m82_portmap(address_map &map)
{
	map(0x00, 0x01).portr("IN0");
	map(0x02, 0x03).portr("IN1");
	map(0x04, 0x05).portr("DSW");
	map(0x00, 0x00).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x02, 0x02).w(FUNC(m72_state::rtype2_port02_w));
	map(0x40, 0x43).rw(m_upd71059c, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x80, 0x81).w(FUNC(m72_state::scrolly_w<0>));
	map(0x82, 0x83).w(FUNC(m72_state::scrollx_w<0>));
	map(0x84, 0x85).w(FUNC(m72_state::scrolly_w<1>));
	map(0x86, 0x87).w(FUNC(m72_state::scrollx_w<1>));

	// these ports control the tilemap sizes, rowscroll etc. that m82 has, exact bit usage not known (maybe one for each layer?)
	map(0x8c, 0x8d).w(FUNC(m72_state::m82_tm_ctrl_w));
	map(0x8e, 0x8f).w(FUNC(m72_state::m82_gfx_ctrl_w));
}

void m72_state::m81_portmap(address_map &map)
{
	map(0x00, 0x01).portr("IN0");
	map(0x02, 0x03).portr("IN1");
	map(0x04, 0x05).portr("DSW");
	map(0x00, 0x00).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x02, 0x02).w(FUNC(m72_state::rtype2_port02_w));  /* coin counters, reset sound cpu, other stuff? */
	map(0x04, 0x04).w(FUNC(m72_state::dmaon_w));
	map(0x06, 0x07).w(FUNC(m72_state::irq_line_w));
	map(0x40, 0x43).rw(m_upd71059c, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x80, 0x81).w(FUNC(m72_state::scrolly_w<0>));
	map(0x82, 0x83).w(FUNC(m72_state::scrollx_w<0>));
	map(0x84, 0x85).w(FUNC(m72_state::scrolly_w<1>));
	map(0x86, 0x87).w(FUNC(m72_state::scrollx_w<1>));
}

void m72_state::lohtb_portmap(address_map &map)
{
	map(0x20, 0x21).portr("IN0");
	map(0x22, 0x23).portr("IN1");
	map(0x24, 0x25).portr("DSW");
	//map(0x00, 0x00).w("soundlatch", FUNC(generic_latch_8_device::write));
	//map(0x02, 0x02).w(FUNC(m72_state::port02_w)); /* coin counters, reset sound cpu, other stuff? */
	//map(0x04, 0x04).w(FUNC(m72_state::dmaon_w));
	//map(0x06, 0x07).w(FUNC(m72_state::irq_line_w));
	//map(0x80, 0x81).w(FUNC(m72_state::scrolly_w<0>));
	//map(0x82, 0x83).w(FUNC(m72_state::scrollx_w<0>));
	//map(0x84, 0x85).w(FUNC(m72_state::scrolly_w<1>));
	//map(0x86, 0x87).w(FUNC(m72_state::scrollx_w<1>));
}

void m72_state::sound_ram_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("soundram");
}

void m72_state::sound_rom_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xffff).ram();
}

void m72_state::rtype_sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x02, 0x02).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x06, 0x06).w("soundlatch", FUNC(generic_latch_8_device::acknowledge_w));
}

void m72_state::sound_portmap(address_map &map)
{
	rtype_sound_portmap(map);
	map.global_mask(0xff);
	map(0x82, 0x82).w(m_audio, FUNC(m72_audio_device::sample_w));
	map(0x84, 0x84).r(m_audio, FUNC(m72_audio_device::sample_r));
}

void m72_state::sound_protected_portmap(address_map &map)
{
	rtype_sound_portmap(map);
	map.global_mask(0xff);
	map(0x82, 0x82).w("mculatch", FUNC(generic_latch_8_device::write));
	map(0x84, 0x84).r("soundlatch2", FUNC(generic_latch_8_device::read));
}

void m72_state::rtype2_sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x80, 0x80).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x80, 0x81).w(m_audio, FUNC(m72_audio_device::rtype2_sample_addr_w));
	map(0x82, 0x82).w(m_audio, FUNC(m72_audio_device::sample_w));
	map(0x83, 0x83).w("soundlatch", FUNC(generic_latch_8_device::acknowledge_w));
	map(0x84, 0x84).r(m_audio, FUNC(m72_audio_device::sample_r));
//  map(0x87, 0x87).nopw();    /* ??? */
}

void m72_state::poundfor_sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x13).w(m_audio, FUNC(m72_audio_device::poundfor_sample_addr_w));
	map(0x40, 0x41).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x42, 0x42).rw("soundlatch", FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::acknowledge_w));
}

void m72_state::i80c31_mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("mcu", 0);
}

void m72_state::mcu_io_map(address_map &map)
{
	/* External access */
	map(0x0000, 0x0000).rw(FUNC(m72_state::mcu_sample_r), FUNC(m72_state::mcu_low_w));
	map(0x0001, 0x0001).w(FUNC(m72_state::mcu_high_w));
	map(0x0002, 0x0002).rw("mculatch", FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::acknowledge_w));
	/* shared at b0000 - b0fff on the main cpu */
	map(0xc000, 0xcfff).rw(FUNC(m72_state::mcu_data_r), FUNC(m72_state::mcu_data_w));
}

#define COIN_MODE_1 \
	PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coinage ) ) PORT_CONDITION("DSW", 0x0400, NOTEQUALS, 0x0000) PORT_DIPLOCATION("SW1:5,6,7,8") \
	PORT_DIPSETTING(      0x00a0, DEF_STR( 6C_1C ) ) \
	PORT_DIPSETTING(      0x00b0, DEF_STR( 5C_1C ) ) \
	PORT_DIPSETTING(      0x00c0, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(      0x00d0, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(      0x0010, DEF_STR( 8C_3C ) ) \
	PORT_DIPSETTING(      0x00e0, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x0020, DEF_STR( 5C_3C ) ) \
	PORT_DIPSETTING(      0x0030, DEF_STR( 3C_2C ) ) \
	PORT_DIPSETTING(      0x00f0, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(      0x0090, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(      0x0070, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(      0x0050, DEF_STR( 1C_6C ) ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )

#define COIN_MODE_2_A \
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) ) PORT_CONDITION("DSW", 0x0400, EQUALS, 0x0000) PORT_DIPLOCATION("SW1:5,6") \
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) ) \
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) ) \
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) ) PORT_CONDITION("DSW", 0x0400, EQUALS, 0x0000) PORT_DIPLOCATION("SW1:7,8") \
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )

static INPUT_PORTS_START( common )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE ) /* 0x20 is another test mode */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM )  /* sprite DMA complete */
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( rtype )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, "50K 150K 250K 400K 600K" )
	PORT_DIPSETTING(      0x0008, "100K 200K 350K 500K 700K"  )
	/* Coin Mode 1 */
	COIN_MODE_1
	/* Coin Mode 2 */
	COIN_MODE_2_A
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x2000, 0x2000, "Stop Mode" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Invulnerability" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8")
INPUT_PORTS_END

/* identical but Demo Sounds is inverted */
static INPUT_PORTS_START( rtypep )
	PORT_INCLUDE( rtype )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bchopper )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, "80K 200K 350K" )
	PORT_DIPSETTING(      0x0000, "100K 250K 400K" )
	/* Coin Mode 1 */
	COIN_MODE_1
	/* Coin Mode 2 */
	COIN_MODE_2_A
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x2000, 0x2000, "Stop Mode" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Invulnerability" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8")
INPUT_PORTS_END

static INPUT_PORTS_START( nspirit )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	/* Coin Mode 1 */
	COIN_MODE_1
	/* Coin Mode 2 */
	COIN_MODE_2_A
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x2000, 0x2000, "Stop Mode" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Invulnerability" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( imgfight )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hardest ) )
	PORT_DIPSETTING(      0x0000, "Debug Mode 2 lap" ) /* Not used according to manual */
	/* Coin Mode 1 */
	COIN_MODE_1
	/* Coin Mode 2 */
	COIN_MODE_2_A
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW2:5" )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x2000, 0x2000, "Stop Mode" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW2:7" )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( loht )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0001, "5" )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW1:4" )
	/* Coin Mode 1 */
	COIN_MODE_1
	/* Coin Mode 2 */
	COIN_MODE_2_A
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hardest ) )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x2000, 0x2000, "Stop Mode" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Invulnerability" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( xmultipl )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, "1" )
	PORT_DIPSETTING(      0x0004, "2" )
	PORT_DIPSETTING(      0x000c, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	/* Coin Mode 1 */
	COIN_MODE_1
	/* Coin Mode 2 */
	COIN_MODE_2_A
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, "Upright (single)" )      PORT_CONDITION("DSW", 0x1000, NOTEQUALS, 0x0000)
	PORT_DIPSETTING(      0x0200, DEF_STR( Cocktail ) )     PORT_CONDITION("DSW", 0x1000, NOTEQUALS, 0x0000)
	PORT_DIPSETTING(      0x0000, "Upright (double) On" )   PORT_CONDITION("DSW", 0x1000, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0200, "Upright (double) Off" )  PORT_CONDITION("DSW", 0x1000, EQUALS, 0x0000)
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Upright (double) Mode" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW2:7" )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( m81_xmultipl )
	PORT_INCLUDE( xmultipl )
	M81_B_B_JUMPER_J3_W
INPUT_PORTS_END

static INPUT_PORTS_START( dbreed )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	/* Coin Mode 1 */
	COIN_MODE_1
	/* Coin Mode 2 */
	COIN_MODE_2_A
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x2000, 0x2000, "Stop Mode" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW2:7" )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8")
INPUT_PORTS_END

static INPUT_PORTS_START( m81_dbreed )
	PORT_INCLUDE( dbreed )
	M81_B_B_JUMPER_J3_S
INPUT_PORTS_END

static INPUT_PORTS_START( rtype2 )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	/* Coin Mode 1 */
	COIN_MODE_1
	/* Coin Mode 2 */
	COIN_MODE_2_A
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Mode" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x1800, 0x1000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, "Upright (2P)" )
	PORT_DIPSETTING(      0x1800, DEF_STR( Cocktail ) )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x2000, 0x2000, "Stop Mode" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW2:7" ) /* Always OFF according to the manual */
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW2:8")
INPUT_PORTS_END

static INPUT_PORTS_START( hharry )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Continue Limit" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0600, 0x0400, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, "Upright (2P)" )
	PORT_DIPSETTING(      0x0600, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	/* Coin Mode 1 */
	IREM_COIN_MODE_1_NEW_HIGH
	/* Coin mode 2 */
	IREM_COIN_MODE_2_HIGH
INPUT_PORTS_END

static INPUT_PORTS_START( m81_hharry )
	PORT_INCLUDE( hharry )
	M81_B_B_JUMPER_J3_S
INPUT_PORTS_END

static INPUT_PORTS_START( poundfor )
	PORT_START("IN0") // not read directly
	PORT_BIT( 0x9f9f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER("upd4701l", upd4701_device, right_w)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_WRITE_LINE_DEVICE_MEMBER("upd4701l", upd4701_device, left_w)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER("upd4701h", upd4701_device, right_w)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_WRITE_LINE_DEVICE_MEMBER("upd4701h", upd4701_device, left_w)

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE ) /* 0x0020 is another test mode */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM )  /* sprite DMA complete */
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, "Round Time" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "60" )
	PORT_DIPSETTING(      0x0003, "90" )
	PORT_DIPSETTING(      0x0001, "120" )
	PORT_DIPSETTING(      0x0000, "150" )
	PORT_DIPNAME( 0x0004, 0x0004, "Matches/Credit (2P)" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x0008, 0x0008, "Rounds/Match" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Trackball Size" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, "Small" )
	PORT_DIPSETTING(      0x0000, "Large" )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8")
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0600, 0x0400, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0200, "Upright (2P)" )
	PORT_DIPSETTING(      0x0600, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	/* Coin Mode 1 */
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coinage ) ) PORT_CONDITION("DSW", 0x0800, NOTEQUALS, 0x0000) PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(      0xa000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1000, "1 Coin/1 Credit, 1 Coin/Continue" )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	/* Coin Mode 2 */
	IREM_COIN_MODE_2_HIGH

	PORT_START("TRACK0_X")
	PORT_BIT( 0x0fff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_START("TRACK0_Y")
	PORT_BIT( 0x0fff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("TRACK1_X")
	PORT_BIT( 0x0fff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("TRACK1_Y")
	PORT_BIT( 0x0fff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( airduel )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW1:5" )     /* Manual says not used, must be OFF */
	PORT_DIPUNUSED_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW1:6" )     /* Manual says not used, must be OFF */
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0200, IP_ACTIVE_LOW, "SW1:2" )     /* Manual says not used, must be OFF */
	PORT_DIPUNUSED_DIPLOC( 0x0400, IP_ACTIVE_LOW, "SW1:3" )     /* Manual says not used, must be OFF */
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	/* Coin Mode 1 */
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coinage ) ) PORT_CONDITION("DSW", 0x0800, NOTEQUALS, 0x0000) PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(      0xa000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 3C_1C ) )
//  PORT_DIPSETTING(      0x1000, DEF_STR( Free-Play ) )  /* another free play */
	PORT_DIPSETTING(      0xe000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	/* Coin Mode 2 */
	IREM_COIN_MODE_2_HIGH
INPUT_PORTS_END

static INPUT_PORTS_START( gallop )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0600, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0200, "Upright (2P)" )
	PORT_DIPSETTING(      0x0600, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	/* Coin Mode 1 */
	IREM_COIN_MODE_1_NEW_HIGH
	/* Coin mode 2 */
	IREM_COIN_MODE_2_HIGH
INPUT_PORTS_END

static INPUT_PORTS_START( kengo )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "SW1:5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	/* Coin Mode 1 */
	IREM_COIN_MODE_1_NEW_HIGH
	/* Coin mode 2 */
	IREM_COIN_MODE_2_HIGH
INPUT_PORTS_END



static const gfx_layout tilelayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,4),  /* NUM characters */
	4,  /* 4 bits per pixel */
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,4),  /* NUM characters */
	4,  /* 4 bits per pixel */
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP8(0,1), STEP8(16*8,1) },
	{ STEP16(0,8) },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static GFXDECODE_START( gfx_m72 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,    0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,    256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout,    256, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_rtype2 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,     256, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_majtitle )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,     256, 16 )
	GFXDECODE_ENTRY( "sprites2", 0, spritelayout,     0, 16 )
GFXDECODE_END


void m72_state::m72_audio_chips(machine_config &config)
{
	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	generic_latch_8_device &soundlatch(GENERIC_LATCH_8(config, "soundlatch"));
	soundlatch.data_pending_callback().set("soundirq", FUNC(rst_neg_buffer_device::rst18_w));
	soundlatch.set_separate_acknowledge(true);

	RST_NEG_BUFFER(config, "soundirq", 0).int_callback().set_inputline(m_soundcpu, 0);

	m_soundcpu->set_irq_acknowledge_callback("soundirq", FUNC(rst_neg_buffer_device::inta_cb));

	IREM_M72_AUDIO(config, m_audio);
	m_audio->set_device_rom_tag("samples");
	m_audio->set_dac_tag("dac");

	ym2151_device &ymsnd(YM2151(config, "ymsnd", SOUND_CLOCK));
	ymsnd.irq_handler().set("soundirq", FUNC(rst_neg_buffer_device::rst28_w));
	ymsnd.add_route(ALL_OUTPUTS, "speaker", 0.33);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.33); // unknown DAC
}

void m72_state::m72_base(machine_config &config)
{
	/* basic machine hardware */
	V30(config, m_maincpu, MASTER_CLOCK/2/2);    /* 16 MHz external freq (8MHz internal) */
	m_maincpu->set_addrmap(AS_PROGRAM, &m72_state::m72_map);
	m_maincpu->set_addrmap(AS_IO, &m72_state::m72_portmap);
	m_maincpu->set_irq_acknowledge_callback("upd71059c", FUNC(pic8259_device::inta_cb));

	Z80(config, m_soundcpu, SOUND_CLOCK);
	m_soundcpu->set_addrmap(AS_PROGRAM, &m72_state::sound_ram_map);
	m_soundcpu->set_addrmap(AS_IO, &m72_state::sound_portmap);

	PIC8259(config, m_upd71059c, 0);
	m_upd71059c->out_int_callback().set_inputline(m_maincpu, 0);

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_m72);
	PALETTE(config, m_palette).set_entries(512);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK/4, 512, 64, 448, 284, 0, 256);
	m_screen->set_screen_update(FUNC(m72_state::screen_update));
	m_screen->set_palette(m_palette);

	MCFG_VIDEO_START_OVERRIDE(m72_state,m72)

	m72_audio_chips(config);
}

void m72_state::m72(machine_config &config)
{
	m72_base(config);
	/* Sample rate verified (Gallop : https://youtu.be/aozd0dbPzOw) */
	m_soundcpu->set_periodic_int(FUNC(m72_state::fake_nmi), attotime::from_hz(MASTER_CLOCK/8/512));
	/* IRQs are generated by main Z80 and YM2151 */
}

void m72_state::m72_8751(machine_config &config)
{
	m72_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &m72_state::m72_protected_map);
	m_maincpu->set_addrmap(AS_IO, &m72_state::m72_protected_portmap);

	m_soundcpu->set_addrmap(AS_IO, &m72_state::sound_protected_portmap);

	MB8421_MB8431_16BIT(config, m_dpram);
	//m_dpram->intl_callback().set(m_upd71059c, FUNC(pic8259_device::ir3_w)); // not actually used?
	m_dpram->intr_callback().set_inputline("mcu", MCS51_INT0_LINE);

	generic_latch_8_device &mculatch(GENERIC_LATCH_8(config, "mculatch"));
	mculatch.data_pending_callback().set_inputline(m_mcu, MCS51_INT1_LINE);
	mculatch.set_separate_acknowledge(true);

	GENERIC_LATCH_8(config, "soundlatch2").data_pending_callback().set_inputline(m_soundcpu, INPUT_LINE_NMI);

	i8751_device &mcu(I8751(config, m_mcu, XTAL(8'000'000))); /* Uses its own XTAL */
	mcu.set_addrmap(AS_IO, &m72_state::mcu_io_map);
	mcu.port_out_cb<1>().set(m_dac, FUNC(dac_byte_interface::write));
}

void m72_state::m72_airduel(machine_config &config)
{
	m72_8751(config);
	m_maincpu->set_addrmap(AS_IO, &m72_state::m72_airduel_portmap);
}

void m72_state::imgfightjb(machine_config &config)
{
	m72_8751(config);
	i80c31_device &mcu(I80C31(config.replace(), m_mcu, XTAL(32'000'000) / 4));
	mcu.set_addrmap(AS_PROGRAM, &m72_state::i80c31_mem_map);
	mcu.set_addrmap(AS_IO, &m72_state::mcu_io_map);
	mcu.port_out_cb<1>().set(m_dac, FUNC(dac_byte_interface::write));

	// TODO: uses 6116 type RAM instead of MB8421 and MB8431

	MCFG_VIDEO_START_OVERRIDE(m72_state, imgfight)
}

void m72_state::rtype(machine_config &config)
{
	m72_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &m72_state::rtype_map);
	m_soundcpu->set_addrmap(AS_IO, &m72_state::rtype_sound_portmap);

	config.device_remove("m72");
	config.device_remove("dac");
}

void m72_state::m72_xmultipl(machine_config &config)
{
	m72_8751(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &m72_state::xmultiplm72_map);

	/* Sample rate verified (Gallop : https://youtu.be/aozd0dbPzOw) */
	m_soundcpu->set_periodic_int(FUNC(m72_state::nmi_line_pulse), attotime::from_hz(MASTER_CLOCK/8/512));
	/* IRQs are generated by main Z80 and YM2151 */
}

void m72_state::m72_dbreedw(machine_config &config)
{
	m72_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &m72_state::dbreedwm72_map);

	/* Sample rate verified (Gallop : https://youtu.be/aozd0dbPzOw) */
	m_soundcpu->set_periodic_int(FUNC(m72_state::nmi_line_pulse), attotime::from_hz(MASTER_CLOCK/8/512));
	/* IRQs are generated by main Z80 and YM2151 */

	MCFG_VIDEO_START_OVERRIDE(m72_state,dbreedm72)
}

void m72_state::m72_dbreed(machine_config &config)
{
	m72_8751(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &m72_state::dbreedm72_map);

	/* Sample rate verified (Gallop : https://youtu.be/aozd0dbPzOw) */
	m_soundcpu->set_periodic_int(FUNC(m72_state::nmi_line_pulse), attotime::from_hz(MASTER_CLOCK/8/512));
	/* IRQs are generated by main Z80 and YM2151 */

	MCFG_VIDEO_START_OVERRIDE(m72_state,dbreedm72)
}



/****************************************** M81 ***********************************************/

// M81 is closest to M72
void m72_state::m81_hharry(machine_config &config)
{
	m72_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &m72_state::hharry_map);
	m_maincpu->set_addrmap(AS_IO, &m72_state::m81_portmap);

	m_soundcpu->set_addrmap(AS_PROGRAM, &m72_state::sound_rom_map);
	m_soundcpu->set_addrmap(AS_IO, &m72_state::rtype2_sound_portmap);
	m_soundcpu->set_periodic_int(FUNC(m72_state::nmi_line_pulse), attotime::from_hz(MASTER_CLOCK/8/512));

	MCFG_VIDEO_START_OVERRIDE(m72_state,hharry)

	m_screen->set_screen_update(FUNC(m72_state::screen_update_m81));
}

void m72_state::m81_xmultipl(machine_config &config)
{
	m81_hharry(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &m72_state::xmultipl_map);

	MCFG_VIDEO_START_OVERRIDE(m72_state,xmultipl) // different offsets
}

void m72_state::m81_dbreed(machine_config &config)
{
	m81_hharry(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &m72_state::dbreed_map);
}

/****************************************** M84 ***********************************************/

// M84
void m72_state::rtype2(machine_config &config)
{
	/* basic machine hardware */
	V30(config, m_maincpu, MASTER_CLOCK/2/2);   /* 16 MHz external freq (8MHz internal) */
	m_maincpu->set_addrmap(AS_PROGRAM, &m72_state::rtype2_map);
	m_maincpu->set_addrmap(AS_IO, &m72_state::m84_portmap);
	m_maincpu->set_irq_acknowledge_callback("upd71059c", FUNC(pic8259_device::inta_cb));

	Z80(config, m_soundcpu, SOUND_CLOCK);
	m_soundcpu->set_addrmap(AS_PROGRAM, &m72_state::sound_rom_map);
	m_soundcpu->set_addrmap(AS_IO, &m72_state::rtype2_sound_portmap);
	m_soundcpu->set_periodic_int(FUNC(m72_state::nmi_line_pulse), attotime::from_hz(MASTER_CLOCK/8/512)); /* verified (https://youtu.be/lUszf9Ong7U) */
								/* IRQs are generated by main Z80 and YM2151 */

	PIC8259(config, m_upd71059c, 0);
	m_upd71059c->out_int_callback().set_inputline(m_maincpu, 0);

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_rtype2);
	PALETTE(config, m_palette).set_entries(512);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK/4, 512, 64, 448, 284, 0, 256);
	m_screen->set_screen_update(FUNC(m72_state::screen_update));
	m_screen->set_palette(m_palette);

	MCFG_VIDEO_START_OVERRIDE(m72_state,rtype2)

	m72_audio_chips(config);
}

// not m72, different video system (less tiles regions?) (M84? M82?)
void m72_state::hharryu(machine_config &config)
{
	rtype2(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &m72_state::hharryu_map);

	MCFG_VIDEO_START_OVERRIDE(m72_state,hharryu)
}


// M84

void m72_state::cosmccop(machine_config &config)
{
	/* basic machine hardware */
	V35(config, m_maincpu, MASTER_CLOCK/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &m72_state::kengo_map);
	m_maincpu->set_addrmap(AS_IO, &m72_state::m84_v33_portmap);

	Z80(config, m_soundcpu, SOUND_CLOCK);
	m_soundcpu->set_addrmap(AS_PROGRAM, &m72_state::sound_rom_map);
	m_soundcpu->set_addrmap(AS_IO, &m72_state::rtype2_sound_portmap);
	m_soundcpu->set_periodic_int(FUNC(m72_state::nmi_line_pulse), attotime::from_hz(MASTER_CLOCK/8/512)); /* verified (https://youtu.be/Sol2Yq2S5hQ) */
								/* IRQs are generated by main Z80 and YM2151 */

	MCFG_MACHINE_START_OVERRIDE(m72_state,kengo)
	MCFG_MACHINE_RESET_OVERRIDE(m72_state,kengo)

	// upd71059c isn't needed because the V35 has its own IRQ controller

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_rtype2);
	PALETTE(config, m_palette).set_entries(512);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK/4, 512, 64, 448, 284, 0, 256);
	m_screen->set_screen_update(FUNC(m72_state::screen_update));
	m_screen->set_palette(m_palette);

	MCFG_VIDEO_START_OVERRIDE(m72_state,hharryu)

	m72_audio_chips(config);
}

void m72_state::kengo(machine_config &config)
{
	cosmccop(config);
	subdevice<v35_device>("maincpu")->set_decryption_table(gunforce_decryption_table);
}

void m72_state::imgfight(machine_config &config)
{
	m72_8751(config);
	MCFG_VIDEO_START_OVERRIDE(m72_state,imgfight)
}

void m72_state::nspiritj(machine_config &config)
{
	m72_8751(config);
	MCFG_VIDEO_START_OVERRIDE(m72_state,nspiritj)
}

void m72_state::mrheli(machine_config &config)
{
	m72_8751(config);
	MCFG_VIDEO_START_OVERRIDE(m72_state,mrheli)
}

/****************************************** M82 ***********************************************/

/* Major Title uses

M82-A-A as the top board
M82-B-A and as the bottom board

*/
void m72_state::m82(machine_config &config)
{
	/* basic machine hardware */
	V30(config, m_maincpu, MASTER_CLOCK/2/2);   /* 16 MHz external freq (8MHz internal) */
	m_maincpu->set_addrmap(AS_PROGRAM, &m72_state::m82_map);
	m_maincpu->set_addrmap(AS_IO, &m72_state::m82_portmap);
	m_maincpu->set_irq_acknowledge_callback("upd71059c", FUNC(pic8259_device::inta_cb));

	Z80(config, m_soundcpu, SOUND_CLOCK);
	m_soundcpu->set_addrmap(AS_PROGRAM, &m72_state::sound_rom_map);
	m_soundcpu->set_addrmap(AS_IO, &m72_state::rtype2_sound_portmap);
	m_soundcpu->set_periodic_int(FUNC(m72_state::nmi_line_pulse), attotime::from_hz(MASTER_CLOCK/8/512)); /* verified (https://youtu.be/lLQDPe-8Ha0) */
								/* IRQs are generated by main Z80 and YM2151 */

	PIC8259(config, m_upd71059c, 0);
	m_upd71059c->out_int_callback().set_inputline(m_maincpu, 0);

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_majtitle);
	PALETTE(config, m_palette).set_entries(512);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK/4, 512, 64, 448, 284, 0, 256);
	m_screen->set_screen_update(FUNC(m72_state::screen_update_m82));
	m_screen->set_palette(m_palette);

	MCFG_VIDEO_START_OVERRIDE(m72_state,m82)

	m72_audio_chips(config);
}


/* Pound for Pound uses
  M85-A-B / M85-B
*/

void m72_state::poundfor(machine_config &config)
{
	/* basic machine hardware */
	V30(config, m_maincpu, MASTER_CLOCK/2/2);   /* 16 MHz external freq (8MHz internal) */
	m_maincpu->set_addrmap(AS_PROGRAM, &m72_state::rtype2_map);
	m_maincpu->set_addrmap(AS_IO, &m72_state::poundfor_portmap);
	m_maincpu->set_irq_acknowledge_callback("upd71059c", FUNC(pic8259_device::inta_cb));

	Z80(config, m_soundcpu, SOUND_CLOCK);
	m_soundcpu->set_addrmap(AS_PROGRAM, &m72_state::sound_rom_map);
	m_soundcpu->set_addrmap(AS_IO, &m72_state::poundfor_sound_portmap);
	m_soundcpu->set_periodic_int(FUNC(m72_state::fake_nmi), attotime::from_hz(MASTER_CLOCK/8/512));   /* clocked by V1? (Vigilante) */
								/* IRQs are generated by main Z80 and YM2151 */

	PIC8259(config, m_upd71059c, 0);
	m_upd71059c->out_int_callback().set_inputline(m_maincpu, 0);

	UPD4701A(config, m_upd4701[0]);
	m_upd4701[0]->set_portx_tag("TRACK0_X");
	m_upd4701[0]->set_porty_tag("TRACK0_Y");

	UPD4701A(config, m_upd4701[1]);
	m_upd4701[1]->set_portx_tag("TRACK1_X");
	m_upd4701[1]->set_porty_tag("TRACK1_Y");

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_rtype2);
	PALETTE(config, m_palette).set_entries(512);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK/4, 512, 64, 448, 284, 0, 256);
	m_screen->set_screen_update(FUNC(m72_state::screen_update));
	m_screen->set_palette(m_palette);

	MCFG_VIDEO_START_OVERRIDE(m72_state,poundfor)

	m72_audio_chips(config);
}

void m72_state::lohtb(machine_config &config) // almost all to be verified
{
	/* basic machine hardware */
	V30(config, m_maincpu, MASTER_CLOCK/2/2);    /* 16 MHz external freq (8MHz internal) */
	m_maincpu->set_addrmap(AS_PROGRAM, &m72_state::lohtb_map);
	m_maincpu->set_addrmap(AS_IO, &m72_state::lohtb_portmap);

	Z80(config, m_soundcpu, SOUND_CLOCK);
	m_soundcpu->set_addrmap(AS_PROGRAM, &m72_state::sound_rom_map);
	m_soundcpu->set_addrmap(AS_IO, &m72_state::sound_portmap);

	MCFG_MACHINE_START_OVERRIDE(m72_state,kengo)
	MCFG_MACHINE_RESET_OVERRIDE(m72_state,kengo)

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_m72);
	PALETTE(config, m_palette).set_entries(512);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK/4, 512, 64, 448, 284, 0, 256);
	m_screen->set_screen_update(FUNC(m72_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, 0);

	MCFG_VIDEO_START_OVERRIDE(m72_state,m72)

	m72_audio_chips(config);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( rtype )
	ROM_REGION( 0x100000, "maincpu", 0 ) // Roms located on the M72-ROM-C rom board
	ROM_LOAD16_BYTE( "rt_r-h0-b.1b", 0x00001, 0x10000, CRC(591c7754) SHA1(0b9d5474bc5963224923126cf84d74a39b8270cc) )
	ROM_LOAD16_BYTE( "rt_r-l0-b.3b", 0x00000, 0x10000, CRC(a1928df0) SHA1(3001c1b87cd1d441ba1226fb5b9dd6268458c0e8) )
	ROM_LOAD16_BYTE( "rt_r-h1-b.1c", 0x20001, 0x10000, CRC(a9d71eca) SHA1(008d1dc289df2ae2ba8f93d319c2b2c108cb9b89) )
	ROM_RELOAD(                      0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "rt_r-l1-b.3c", 0x20000, 0x10000, CRC(0df3573d) SHA1(0144c846fd0bdb3e4d790f6cb7bb64829e931b76) )
	ROM_RELOAD(                      0xe0000, 0x10000 )

	ROM_REGION( 0x80000, "sprites", 0 ) // Roms located on the M72-ROM-C rom board
	ROM_LOAD( "rt_r-00.1h", 0x00000, 0x10000, CRC(dad53bc0) SHA1(1e3bc498861946278a0b1fe24259f5d224e265d7) )    // sprites
	ROM_LOAD( "rt_r-01.1j", 0x10000, 0x08000, CRC(5e441e7f) SHA1(6741eb7f2d9d985b5a89eefc73ea44c3e38de6f7) )
	ROM_RELOAD(             0x18000, 0x08000 )
	ROM_LOAD( "rt_r-10.1k", 0x20000, 0x10000, CRC(d6a66298) SHA1(d2873d05aa3b257e7699c188880ac3daad672fa5) )
	ROM_LOAD( "rt_r-11.1l", 0x30000, 0x08000, CRC(791df4f8) SHA1(5239a97222212ac9c019177771cb2b5096b7bc17) )
	ROM_RELOAD(             0x38000, 0x08000 )
	ROM_LOAD( "rt_r-20.3h", 0x40000, 0x10000, CRC(fc247c8a) SHA1(01cf0a60f47fa5e2ed430a3f075e69e6cb762a48) )
	ROM_LOAD( "rt_r-21.3j", 0x50000, 0x08000, CRC(ed793841) SHA1(7e55a9a11fcd989db39bce6be48821b747c7d97f) )
	ROM_RELOAD(             0x58000, 0x08000 )
	ROM_LOAD( "rt_r-30.3k", 0x60000, 0x10000, CRC(eb02a1cb) SHA1(60a394ab53afdcbbf9e88083b8dbe8c897170d77) )
	ROM_LOAD( "rt_r-31.3l", 0x70000, 0x08000, CRC(8558355d) SHA1(b5467d1f22f6e5f90c5d8a8ac2d55974f287d589) )
	ROM_RELOAD(             0x78000, 0x08000 )

	ROM_REGION( 0x20000, "gfx2", 0 ) // Roms located on the M72-B-D rom board
	ROM_LOAD( "rt_b-a0.ic20", 0x00000, 0x08000, CRC(4e212fb0) SHA1(687061ecade2ebd0bd1343c9c4a831791853f79c) )    // tiles #1
	ROM_LOAD( "rt_b-a1.ic22", 0x08000, 0x08000, CRC(8a65bdff) SHA1(130bf6af521f13247a739a95eab4bdaa24b2ac10) )
	ROM_LOAD( "rt_b-a2.ic20", 0x10000, 0x08000, CRC(5a4ae5b9) SHA1(95c3b64f50e6f673b2bf9b40642c152da5009d25) )
	ROM_LOAD( "rt_b-a3.ic23", 0x18000, 0x08000, CRC(73327606) SHA1(9529ecdedd30e2a0400fb1083117992cc18b5158) )

	ROM_REGION( 0x20000, "gfx3", 0 ) // Roms located on the M72-B-D rom board
	ROM_LOAD( "rt_b-b0.ic26", 0x00000, 0x08000, CRC(a7b17491) SHA1(5b390770e56ba2d35e108534d7eda8dca996fdf7) )    // tiles #2
	ROM_LOAD( "rt_b-b1.ic27", 0x08000, 0x08000, CRC(b9709686) SHA1(700905a3e9661e0874939f54da2909e1396ce596) )
	ROM_LOAD( "rt_b-b2.ic25", 0x10000, 0x08000, CRC(433b229a) SHA1(14222eaa3e67e5a7f80eafcf22bac4eb2d485a9a) )
	ROM_LOAD( "rt_b-b3.ic24", 0x18000, 0x08000, CRC(ad89b072) SHA1(e2683d0e7415f3abd147e518bf6c87e44744cd4f) )

	ROM_REGION( 0x0200, "proms", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0600, "plds", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-3d-.ic11", 0x0000, 0x0117, CRC(8a3732ff) SHA1(6e3039e7dc424cbef7156312fa1ce67d7b082d30) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_a-4d-.ic19", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_r-3a-.3a",   0x0400, 0x0117, CRC(055af779) SHA1(740d860df45109710e082d79c534ec0eeaa779f2) ) // PAL16L8 - bruteforced - located on M72-ROM-C rom board
ROM_END

ROM_START( rtypej )
	ROM_REGION( 0x100000, "maincpu", 0 ) // Roms located on the M72-ROM-C rom board
	ROM_LOAD16_BYTE( "rt_r-h0-.1b", 0x00001, 0x10000, CRC(c2940df2) SHA1(cbccd205ef81a0e39990a34d46e3f7d52b62e385) )
	ROM_LOAD16_BYTE( "rt_r-l0-.3b", 0x00000, 0x10000, CRC(858cc0f6) SHA1(7a256fe3aa3a96e161dd485a90b18c421b61458b) )
	ROM_LOAD16_BYTE( "rt_r-h1-.1c", 0x20001, 0x10000, CRC(5bcededa) SHA1(4ada3fd207fa57751f8e3d885bc91b374e27035d) )
	ROM_RELOAD(                     0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "rt_r-l1-.3c", 0x20000, 0x10000, CRC(4821141c) SHA1(df6cf04c3ecd04b6f27a96871848904575414dae) )
	ROM_RELOAD(                     0xe0000, 0x10000 )

	ROM_REGION( 0x80000, "sprites", 0 ) // Roms located on the M72-ROM-C rom board
	ROM_LOAD( "rt_r-00.1h", 0x00000, 0x10000, CRC(dad53bc0) SHA1(1e3bc498861946278a0b1fe24259f5d224e265d7) )    // sprites
	ROM_LOAD( "rt_r-01.1j", 0x10000, 0x08000, CRC(5e441e7f) SHA1(6741eb7f2d9d985b5a89eefc73ea44c3e38de6f7) )
	ROM_RELOAD(             0x18000, 0x08000 )
	ROM_LOAD( "rt_r-10.1k", 0x20000, 0x10000, CRC(d6a66298) SHA1(d2873d05aa3b257e7699c188880ac3daad672fa5) )
	ROM_LOAD( "rt_r-11.1l", 0x30000, 0x08000, CRC(791df4f8) SHA1(5239a97222212ac9c019177771cb2b5096b7bc17) )
	ROM_RELOAD(             0x38000, 0x08000 )
	ROM_LOAD( "rt_r-20.3h", 0x40000, 0x10000, CRC(fc247c8a) SHA1(01cf0a60f47fa5e2ed430a3f075e69e6cb762a48) )
	ROM_LOAD( "rt_r-21.3j", 0x50000, 0x08000, CRC(ed793841) SHA1(7e55a9a11fcd989db39bce6be48821b747c7d97f) )
	ROM_RELOAD(             0x58000, 0x08000 )
	ROM_LOAD( "rt_r-30.3k", 0x60000, 0x10000, CRC(eb02a1cb) SHA1(60a394ab53afdcbbf9e88083b8dbe8c897170d77) )
	ROM_LOAD( "rt_r-31.3l", 0x70000, 0x08000, CRC(8558355d) SHA1(b5467d1f22f6e5f90c5d8a8ac2d55974f287d589) )
	ROM_RELOAD(             0x78000, 0x08000 )

	ROM_REGION( 0x20000, "gfx2", 0 ) // Roms located on the M72-B-D rom board
	ROM_LOAD( "rt_b-a0.ic20", 0x00000, 0x08000, CRC(4e212fb0) SHA1(687061ecade2ebd0bd1343c9c4a831791853f79c) )    // tiles #1
	ROM_LOAD( "rt_b-a1.ic22", 0x08000, 0x08000, CRC(8a65bdff) SHA1(130bf6af521f13247a739a95eab4bdaa24b2ac10) )
	ROM_LOAD( "rt_b-a2.ic20", 0x10000, 0x08000, CRC(5a4ae5b9) SHA1(95c3b64f50e6f673b2bf9b40642c152da5009d25) )
	ROM_LOAD( "rt_b-a3.ic23", 0x18000, 0x08000, CRC(73327606) SHA1(9529ecdedd30e2a0400fb1083117992cc18b5158) )

	ROM_REGION( 0x20000, "gfx3", 0 ) // Roms located on the M72-B-D rom board
	ROM_LOAD( "rt_b-b0.ic26", 0x00000, 0x08000, CRC(a7b17491) SHA1(5b390770e56ba2d35e108534d7eda8dca996fdf7) )    // tiles #2
	ROM_LOAD( "rt_b-b1.ic27", 0x08000, 0x08000, CRC(b9709686) SHA1(700905a3e9661e0874939f54da2909e1396ce596) )
	ROM_LOAD( "rt_b-b2.ic25", 0x10000, 0x08000, CRC(433b229a) SHA1(14222eaa3e67e5a7f80eafcf22bac4eb2d485a9a) )
	ROM_LOAD( "rt_b-b3.ic24", 0x18000, 0x08000, CRC(ad89b072) SHA1(e2683d0e7415f3abd147e518bf6c87e44744cd4f) )

	ROM_REGION( 0x0200, "proms", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0600, "plds", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-3d-.ic11", 0x0000, 0x0117, CRC(8a3732ff) SHA1(6e3039e7dc424cbef7156312fa1ce67d7b082d30) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_a-4d-.ic19", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_r-3a-.3a",   0x0400, 0x0117, CRC(055af779) SHA1(740d860df45109710e082d79c534ec0eeaa779f2) ) // PAL16L8 - bruteforced - located on M72-ROM-C rom board
ROM_END

ROM_START( rtypejp )
	ROM_REGION( 0x100000, "maincpu", 0 ) // Roms located on the M72-ROM-C rom board
	ROM_LOAD16_BYTE( "db_b1.1b", 0x00001, 0x10000, CRC(c1865141) SHA1(3302b6529aa903d81eb2196d745eb4f7f8316857) )
	ROM_LOAD16_BYTE( "db_a1.3b", 0x00000, 0x10000, CRC(5ad2bd90) SHA1(0937dbbdf0cbce2e81cecf4d770bbd8c6bd82801) )
	ROM_LOAD16_BYTE( "db_b2.1c", 0x20001, 0x10000, CRC(b4f6407e) SHA1(4a00d8e104c580900b4feb318dd162b77b71d0a5) )
	ROM_RELOAD(                  0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "db_a2.3c", 0x20000, 0x10000, CRC(6098d86f) SHA1(c6c9c1c2c30d5f190c40e000004bd21606efb8b0) )
	ROM_RELOAD(                  0xe0000, 0x10000 )

	ROM_REGION( 0x80000, "sprites", 0 ) // Roms located on the M72-ROM-C rom board
	ROM_LOAD( "rt_r-00.1h", 0x00000, 0x10000, CRC(dad53bc0) SHA1(1e3bc498861946278a0b1fe24259f5d224e265d7) )    // sprites
	ROM_LOAD( "rt_r-01.1j", 0x10000, 0x08000, CRC(5e441e7f) SHA1(6741eb7f2d9d985b5a89eefc73ea44c3e38de6f7) )
	ROM_RELOAD(             0x18000, 0x08000 )
	ROM_LOAD( "rt_r-10.1k", 0x20000, 0x10000, CRC(d6a66298) SHA1(d2873d05aa3b257e7699c188880ac3daad672fa5) )
	ROM_LOAD( "rt_r-11.1l", 0x30000, 0x08000, CRC(791df4f8) SHA1(5239a97222212ac9c019177771cb2b5096b7bc17) )
	ROM_RELOAD(             0x38000, 0x08000 )
	ROM_LOAD( "rt_r-20.3h", 0x40000, 0x10000, CRC(fc247c8a) SHA1(01cf0a60f47fa5e2ed430a3f075e69e6cb762a48) )
	ROM_LOAD( "rt_r-21.3j", 0x50000, 0x08000, CRC(ed793841) SHA1(7e55a9a11fcd989db39bce6be48821b747c7d97f) )
	ROM_RELOAD(             0x58000, 0x08000 )
	ROM_LOAD( "rt_r-30.3k", 0x60000, 0x10000, CRC(eb02a1cb) SHA1(60a394ab53afdcbbf9e88083b8dbe8c897170d77) )
	ROM_LOAD( "rt_r-31.3l", 0x70000, 0x08000, CRC(8558355d) SHA1(b5467d1f22f6e5f90c5d8a8ac2d55974f287d589) )
	ROM_RELOAD(             0x78000, 0x08000 )

	ROM_REGION( 0x20000, "gfx2", 0 ) // Roms located on the M72-B-D rom board
	ROM_LOAD( "rt_b-a0.ic20", 0x00000, 0x08000, CRC(4e212fb0) SHA1(687061ecade2ebd0bd1343c9c4a831791853f79c) )    // tiles #1
	ROM_LOAD( "rt_b-a1.ic22", 0x08000, 0x08000, CRC(8a65bdff) SHA1(130bf6af521f13247a739a95eab4bdaa24b2ac10) )
	ROM_LOAD( "rt_b-a2.ic20", 0x10000, 0x08000, CRC(5a4ae5b9) SHA1(95c3b64f50e6f673b2bf9b40642c152da5009d25) )
	ROM_LOAD( "rt_b-a3.ic23", 0x18000, 0x08000, CRC(73327606) SHA1(9529ecdedd30e2a0400fb1083117992cc18b5158) )

	ROM_REGION( 0x20000, "gfx3", 0 ) // Roms located on the M72-B-D rom board
	ROM_LOAD( "rt_b-b0.ic26", 0x00000, 0x08000, CRC(a7b17491) SHA1(5b390770e56ba2d35e108534d7eda8dca996fdf7) )    // tiles #2
	ROM_LOAD( "rt_b-b1.ic27", 0x08000, 0x08000, CRC(b9709686) SHA1(700905a3e9661e0874939f54da2909e1396ce596) )
	ROM_LOAD( "rt_b-b2.ic25", 0x10000, 0x08000, CRC(433b229a) SHA1(14222eaa3e67e5a7f80eafcf22bac4eb2d485a9a) )
	ROM_LOAD( "rt_b-b3.ic24", 0x18000, 0x08000, CRC(ad89b072) SHA1(e2683d0e7415f3abd147e518bf6c87e44744cd4f) )

	ROM_REGION( 0x0200, "proms", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0600, "plds", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-3d-.ic11", 0x0000, 0x0117, CRC(8a3732ff) SHA1(6e3039e7dc424cbef7156312fa1ce67d7b082d30) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_a-4d-.ic19", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_r-3a-.3a",   0x0400, 0x0117, CRC(055af779) SHA1(740d860df45109710e082d79c534ec0eeaa779f2) ) // PAL16L8 - bruteforced - located on M72-ROM-C rom board
ROM_END

ROM_START( rtypeu )
	ROM_REGION( 0x100000, "maincpu", 0 ) // Roms located on the M72-ROM-C rom board
	ROM_LOAD16_BYTE( "rt_r-h0-a.1b", 0x00001, 0x10000, CRC(36008a4e) SHA1(832006cb14a34e1671e305cc8ae606c3c6185a6a) )
	ROM_LOAD16_BYTE( "rt_r-l0-a.3b", 0x00000, 0x10000, CRC(4aaa668e) SHA1(87059460b59f43f2ca8cd959d76f721facd9de96) )
	ROM_LOAD16_BYTE( "rt_r-h1-a.1c", 0x20001, 0x10000, CRC(7ebb2a53) SHA1(1466df19888c3374847eb77f702060647e49d6ad) )
	ROM_RELOAD(                      0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "rt_r-l1-a.3c", 0x20000, 0x10000, CRC(c28b103b) SHA1(f294a23c3917b97812eb4c7f3a99253fd0cbb7ea) )
	ROM_RELOAD(                      0xe0000, 0x10000 )

	ROM_REGION( 0x80000, "sprites", 0 ) // Roms located on the M72-ROM-C rom board
	ROM_LOAD( "rt_r-00.1h", 0x00000, 0x10000, CRC(dad53bc0) SHA1(1e3bc498861946278a0b1fe24259f5d224e265d7) )    // sprites
	ROM_LOAD( "rt_r-01.1j", 0x10000, 0x08000, CRC(5e441e7f) SHA1(6741eb7f2d9d985b5a89eefc73ea44c3e38de6f7) )
	ROM_RELOAD(             0x18000, 0x08000 )
	ROM_LOAD( "rt_r-10.1k", 0x20000, 0x10000, CRC(d6a66298) SHA1(d2873d05aa3b257e7699c188880ac3daad672fa5) )
	ROM_LOAD( "rt_r-11.1l", 0x30000, 0x08000, CRC(791df4f8) SHA1(5239a97222212ac9c019177771cb2b5096b7bc17) )
	ROM_RELOAD(             0x38000, 0x08000 )
	ROM_LOAD( "rt_r-20.3h", 0x40000, 0x10000, CRC(fc247c8a) SHA1(01cf0a60f47fa5e2ed430a3f075e69e6cb762a48) )
	ROM_LOAD( "rt_r-21.3j", 0x50000, 0x08000, CRC(ed793841) SHA1(7e55a9a11fcd989db39bce6be48821b747c7d97f) )
	ROM_RELOAD(             0x58000, 0x08000 )
	ROM_LOAD( "rt_r-30.3k", 0x60000, 0x10000, CRC(eb02a1cb) SHA1(60a394ab53afdcbbf9e88083b8dbe8c897170d77) )
	ROM_LOAD( "rt_r-31.3l", 0x70000, 0x08000, CRC(8558355d) SHA1(b5467d1f22f6e5f90c5d8a8ac2d55974f287d589) )
	ROM_RELOAD(             0x78000, 0x08000 )

	ROM_REGION( 0x20000, "gfx2", 0 ) // Roms located on the M72-B-D rom board
	ROM_LOAD( "rt_b-a0.ic20", 0x00000, 0x08000, CRC(4e212fb0) SHA1(687061ecade2ebd0bd1343c9c4a831791853f79c) )    // tiles #1
	ROM_LOAD( "rt_b-a1.ic22", 0x08000, 0x08000, CRC(8a65bdff) SHA1(130bf6af521f13247a739a95eab4bdaa24b2ac10) )
	ROM_LOAD( "rt_b-a2.ic20", 0x10000, 0x08000, CRC(5a4ae5b9) SHA1(95c3b64f50e6f673b2bf9b40642c152da5009d25) )
	ROM_LOAD( "rt_b-a3.ic23", 0x18000, 0x08000, CRC(73327606) SHA1(9529ecdedd30e2a0400fb1083117992cc18b5158) )

	ROM_REGION( 0x20000, "gfx3", 0 ) // Roms located on the M72-B-D rom board
	ROM_LOAD( "rt_b-b0.ic26", 0x00000, 0x08000, CRC(a7b17491) SHA1(5b390770e56ba2d35e108534d7eda8dca996fdf7) )    // tiles #2
	ROM_LOAD( "rt_b-b1.ic27", 0x08000, 0x08000, CRC(b9709686) SHA1(700905a3e9661e0874939f54da2909e1396ce596) )
	ROM_LOAD( "rt_b-b2.ic25", 0x10000, 0x08000, CRC(433b229a) SHA1(14222eaa3e67e5a7f80eafcf22bac4eb2d485a9a) )
	ROM_LOAD( "rt_b-b3.ic24", 0x18000, 0x08000, CRC(ad89b072) SHA1(e2683d0e7415f3abd147e518bf6c87e44744cd4f) )

	ROM_REGION( 0x0200, "proms", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0600, "plds", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-3d-.ic11", 0x0000, 0x0117, CRC(8a3732ff) SHA1(6e3039e7dc424cbef7156312fa1ce67d7b082d30) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_a-4d-.ic19", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_r-3a-.3a",   0x0400, 0x0117, CRC(055af779) SHA1(740d860df45109710e082d79c534ec0eeaa779f2) ) // PAL16L8 - bruteforced - located on M72-ROM-C rom board
ROM_END

ROM_START( rtypeb )
	ROM_REGION( 0x100000, "maincpu", 0 ) // Roms located on the M72-ROM-C rom board
	ROM_LOAD16_BYTE( "7.512", 0x00001, 0x10000, CRC(eacc8024) SHA1(6bcf1d4ea182b7341eac736d2a5d5f70deec0758) )
	ROM_LOAD16_BYTE( "1.512", 0x00000, 0x10000, CRC(2e5fe27b) SHA1(a3364be5ab9c67aaa2152baf39ea12c571eca3cc) )
	ROM_LOAD16_BYTE( "8.512", 0x20001, 0x10000, CRC(22cc4950) SHA1(ada5cffc13c38391a334411632237166a6be4938) )
	ROM_RELOAD(               0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "2.512", 0x20000, 0x10000, CRC(ada7b90e) SHA1(c9d2caed95b95d1c1718a10766bc88b2f8f51619) )
	ROM_RELOAD(               0xe0000, 0x10000 )

	ROM_REGION( 0x80000, "sprites", 0 ) // Roms located on the M72-ROM-C rom board
	ROM_LOAD( "rt_r-00.1h", 0x00000, 0x10000, CRC(dad53bc0) SHA1(1e3bc498861946278a0b1fe24259f5d224e265d7) )    // sprites
	ROM_LOAD( "rt_r-01.1j", 0x10000, 0x08000, CRC(5e441e7f) SHA1(6741eb7f2d9d985b5a89eefc73ea44c3e38de6f7) )
	ROM_RELOAD(             0x18000, 0x08000 )
	ROM_LOAD( "rt_r-10.1k", 0x20000, 0x10000, CRC(d6a66298) SHA1(d2873d05aa3b257e7699c188880ac3daad672fa5) )
	ROM_LOAD( "rt_r-11.1l", 0x30000, 0x08000, CRC(791df4f8) SHA1(5239a97222212ac9c019177771cb2b5096b7bc17) )
	ROM_RELOAD(             0x38000, 0x08000 )
	ROM_LOAD( "rt_r-20.3h", 0x40000, 0x10000, CRC(fc247c8a) SHA1(01cf0a60f47fa5e2ed430a3f075e69e6cb762a48) )
	ROM_LOAD( "rt_r-21.3j", 0x50000, 0x08000, CRC(ed793841) SHA1(7e55a9a11fcd989db39bce6be48821b747c7d97f) )
	ROM_RELOAD(             0x58000, 0x08000 )
	ROM_LOAD( "rt_r-30.3k", 0x60000, 0x10000, CRC(eb02a1cb) SHA1(60a394ab53afdcbbf9e88083b8dbe8c897170d77) )
	ROM_LOAD( "rt_r-31.3l", 0x70000, 0x08000, CRC(8558355d) SHA1(b5467d1f22f6e5f90c5d8a8ac2d55974f287d589) )
	ROM_RELOAD(             0x78000, 0x08000 )

	ROM_REGION( 0x20000, "gfx2", 0 ) // Roms located on the M72-B-D rom board
	ROM_LOAD( "rt_b-a0.ic20", 0x00000, 0x08000, CRC(4e212fb0) SHA1(687061ecade2ebd0bd1343c9c4a831791853f79c) )    // tiles #1
	ROM_LOAD( "rt_b-a1.ic22", 0x08000, 0x08000, CRC(8a65bdff) SHA1(130bf6af521f13247a739a95eab4bdaa24b2ac10) )
	ROM_LOAD( "rt_b-a2.ic20", 0x10000, 0x08000, CRC(5a4ae5b9) SHA1(95c3b64f50e6f673b2bf9b40642c152da5009d25) )
	ROM_LOAD( "rt_b-a3.ic23", 0x18000, 0x08000, CRC(73327606) SHA1(9529ecdedd30e2a0400fb1083117992cc18b5158) )

	ROM_REGION( 0x20000, "gfx3", 0 ) // Roms located on the M72-B-D rom board
	ROM_LOAD( "rt_b-b0.ic26", 0x00000, 0x08000, CRC(a7b17491) SHA1(5b390770e56ba2d35e108534d7eda8dca996fdf7) )    // tiles #2
	ROM_LOAD( "rt_b-b1.ic27", 0x08000, 0x08000, CRC(b9709686) SHA1(700905a3e9661e0874939f54da2909e1396ce596) )
	ROM_LOAD( "rt_b-b2.ic25", 0x10000, 0x08000, CRC(433b229a) SHA1(14222eaa3e67e5a7f80eafcf22bac4eb2d485a9a) )
	ROM_LOAD( "rt_b-b3.ic24", 0x18000, 0x08000, CRC(ad89b072) SHA1(e2683d0e7415f3abd147e518bf6c87e44744cd4f) )
ROM_END


ROM_START( bchopper )
	ROM_REGION( 0x100000, "maincpu", 0 ) // M72-C top board
	ROM_LOAD16_BYTE( "mh_c-h0-b.ic40", 0x00001, 0x10000, CRC(f2feab16) SHA1(03ee874658e0f59957f8425e1ebf9c938737cc19) )
	ROM_LOAD16_BYTE( "mh_c-l0-b.ic37", 0x00000, 0x10000, CRC(9f887096) SHA1(4f41ef29580fc026ea91d110ec6b2e6af83dbd9a) )
	ROM_LOAD16_BYTE( "mh_c-h1-b.ic41", 0x20001, 0x10000, CRC(a995d64f) SHA1(43eb2eb11e6875298a6ef2b18f0f5e587f1bba16) )
	ROM_LOAD16_BYTE( "mh_c-l1-b.ic36", 0x20000, 0x10000, CRC(41dda999) SHA1(4d07a399aaf16bc37b5488e3e4bb60e78811a099) )
	ROM_LOAD16_BYTE( "mh_c-h3-b.ic43", 0x60001, 0x10000, CRC(ab9451ca) SHA1(ec0e0ad592d8b21bb4e6927a452e3b7964cda015) )
	ROM_RELOAD(                        0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "mh_c-l3-b.ic34", 0x60000, 0x10000, CRC(11562221) SHA1(a2f136a487fb6f30350e8d1e26c0729eb0686c7d) )
	ROM_RELOAD(                        0xe0000, 0x10000 )

	ROM_REGION( 0x1000, "mcu", 0 )  // i8751 microcontroller
	ROM_LOAD( "mh_c-pr-b.ic1", 0x0000, 0x1000, CRC(9d201fea) SHA1(20fca55c46d756784b341bbe204388b9d836e76d) ) // i8751 MCU labeled  MH C-PR-B

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "mh_c-00-a.ic53", 0x00000, 0x10000, CRC(f6e6e660) SHA1(e066e5ed37719cf2b6fd36e0117f11325bb06f9c) )  // sprites
	ROM_LOAD( "mh_c-01-b.ic52", 0x10000, 0x10000, CRC(708cdd37) SHA1(24f3fcd381422f0d75410c2af7a56744e3b4a699) )
	ROM_LOAD( "mh_c-10-a.ic51", 0x20000, 0x10000, CRC(292c8520) SHA1(c552090d295ee1c1ca611b0cddee356e509e2045) )
	ROM_LOAD( "mh_c-11-b.ic50", 0x30000, 0x10000, CRC(20904cf3) SHA1(71fe505f2da53c2eb445b7b758d257d6af42e6f1) )
	ROM_LOAD( "mh_c-20-a.ic49", 0x40000, 0x10000, CRC(1ab50c23) SHA1(43e2f11e5bbf157c47764e04e372f40ed68bab59) )
	ROM_LOAD( "mh_c-21-b.ic48", 0x50000, 0x10000, CRC(c823d34c) SHA1(47383214b6a60e0b1b70208b00c291f8ffed36bc) )
	ROM_LOAD( "mh_c-30-a.ic47", 0x60000, 0x10000, CRC(11f6c56b) SHA1(39a2a674698b044c84fea65ae41a9e003a50b639) )
	ROM_LOAD( "mh_c-31-b.ic46", 0x70000, 0x10000, CRC(23134ec5) SHA1(43453f8a13b51310e04729dc828d391ca9c04da2) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "mh_b-a0-b.ic21", 0x00000, 0x10000, CRC(e46ed7bf) SHA1(75abb5f40629f7c40a610a44e068b6c4e3a5126e) )  // tiles #1
	ROM_LOAD( "mh_b-a1-b.ic22", 0x10000, 0x10000, CRC(590605ff) SHA1(fbb5c0cebd28b08d4ce39db4055d6343620e0f1c) )
	ROM_LOAD( "mh_b-a2-b.ic20", 0x20000, 0x10000, CRC(f8158226) SHA1(bb3a8686cd89bb8265b6b9e03682cc0bf6533793) )
	ROM_LOAD( "mh_b-a3-b.ic23", 0x30000, 0x10000, CRC(0f07b9b7) SHA1(63dbec17097f07eb39299372b736fbbc1b11b65e) )

	ROM_REGION( 0x040000, "gfx3", 0 )
	ROM_LOAD( "mh_b-b0-.ic26", 0x00000, 0x10000, CRC(b5b95776) SHA1(4685b56071b916ce712c45f24da8068dd7e40ed1) )  // tiles #2
	ROM_LOAD( "mh_b-b1-.ic27", 0x10000, 0x10000, CRC(74ca16ee) SHA1(7984bc9a0b46e1b4a8ecac7528d57606305aad73) )
	ROM_LOAD( "mh_b-b2-.ic25", 0x20000, 0x10000, CRC(b82cca04) SHA1(c12b95be311205181b01d15021bcf9f01ed3e0a3) )
	ROM_LOAD( "mh_b-b3-.ic24", 0x30000, 0x10000, CRC(a7afc920) SHA1(92c75463ada39184e731b82ef2883ae6f1f67482) )

	ROM_REGION( 0x10000, "samples", 0 ) // samples
	ROM_LOAD( "mh_c-v0-b.ic44", 0x00000, 0x10000, CRC(d0c27e58) SHA1(fec76217cc0c04c723989c3ec127a2bd33d64c60) )

	ROM_REGION( 0x0200, "proms", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0600, "pals", 0 )
	ROM_LOAD( "m72_a-3d-.ic11", 0x0000, 0x0117, CRC(8a3732ff) SHA1(6e3039e7dc424cbef7156312fa1ce67d7b082d30) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_a-4d-.ic19", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // PAL16L8 - bruteforced
	ROM_LOAD( "mh-c-3f-.ic13",  0x0400, 0x0117, CRC(2d774e1e) SHA1(373c3cbfaf983961c17ebe96b5aff850f36cb30c) ) // PAL16L8 - bruteforced - located on M72-C-A top board
ROM_END

ROM_START( mrheli )
	ROM_REGION( 0x100000, "maincpu", 0 ) // M72-C top board
	ROM_LOAD16_BYTE( "mh_c-h0-.ic40", 0x00001, 0x10000, CRC(e2ca5646) SHA1(9f4fe2f0a45233325bd9336cabb925a1f625453b) )
	ROM_LOAD16_BYTE( "mh_c-l0-.ic37", 0x00000, 0x10000, CRC(643e23cd) SHA1(66998a6dfc7ef538540986b61d2414a5ef250d0d) )
	ROM_LOAD16_BYTE( "mh_c-h1-.ic41", 0x20001, 0x10000, CRC(8974e84d) SHA1(39e05c80e805dde45f2fc5fc429b75f9b599089c) )
	ROM_LOAD16_BYTE( "mh_c-l1-.ic36", 0x20000, 0x10000, CRC(5f8bda69) SHA1(48629d617bd48c9de9c6a567fb203258a56fdbbd) )
	ROM_LOAD16_BYTE( "mh_c-h3-.ic43", 0x60001, 0x10000, CRC(143f596e) SHA1(f9d444eebcd53dac925d14b7a2858803b7fd9ce2) )
	ROM_RELOAD(                       0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "mh_c-l3-.ic34", 0x60000, 0x10000, CRC(c0982536) SHA1(45399f8d0577c6e2a277a69303954ce5d2de7c07) )
	ROM_RELOAD(                       0xe0000, 0x10000 )

	ROM_REGION( 0x1000, "mcu", 0 )  // i8751 microcontroller
	ROM_LOAD( "mh_c-pr-.ic1", 0x0000, 0x1000, CRC(897dc4ee) SHA1(05a24bf76e8fa9ca96ba9376cbf44d299df04138) ) // i8751 MCU labeled  MH C-PR-

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "mh_c-00.ic53", 0x00000, 0x20000, CRC(dec4e121) SHA1(92169b523f1600e994e016dc1959a52958e1d89d) )  // sprites
	ROM_LOAD( "mh_c-10.ic51", 0x20000, 0x20000, CRC(7aaa151e) SHA1(efd980bb2eed7084354b7a4aa2f733cd2f876741) )
	ROM_LOAD( "mh_c-20.ic49", 0x40000, 0x20000, CRC(eae0de74) SHA1(3a2469c0eeb18131f989807afb50228f57ccea30) )
	ROM_LOAD( "mh_c-30.ic47", 0x60000, 0x20000, CRC(01d5052f) SHA1(5d5e70913bb7af48193c70209595f27a64fa6cac) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "mh_b-a0.ic21", 0x00000, 0x10000, CRC(6a0db256) SHA1(fa3a2dc03da5bbe06a9c9b3d4ed4fddb47c469ac) )  // tiles #1
	ROM_LOAD( "mh_b-a1.ic22", 0x10000, 0x10000, CRC(14ec9795) SHA1(4842e076115efe9daf00dab8f61516d28c19baae) )
	ROM_LOAD( "mh_b-a2.ic20", 0x20000, 0x10000, CRC(dfcb510e) SHA1(2387cde4ec0bae176486e1f7541103fd557fe255) )
	ROM_LOAD( "mh_b-a3.ic23", 0x30000, 0x10000, CRC(957e329b) SHA1(9d48a0b84915e1cef0b0311a3581991dc83ee199) )

	ROM_REGION( 0x040000, "gfx3", 0 )
	ROM_LOAD( "mh_b-b0-.ic26", 0x00000, 0x10000, CRC(b5b95776) SHA1(4685b56071b916ce712c45f24da8068dd7e40ed1) )  // tiles #2
	ROM_LOAD( "mh_b-b1-.ic27", 0x10000, 0x10000, CRC(74ca16ee) SHA1(7984bc9a0b46e1b4a8ecac7528d57606305aad73) )
	ROM_LOAD( "mh_b-b2-.ic25", 0x20000, 0x10000, CRC(b82cca04) SHA1(c12b95be311205181b01d15021bcf9f01ed3e0a3) )
	ROM_LOAD( "mh_b-b3-.ic24", 0x30000, 0x10000, CRC(a7afc920) SHA1(92c75463ada39184e731b82ef2883ae6f1f67482) )

	ROM_REGION( 0x10000, "samples", 0 ) // samples
	ROM_LOAD( "mh_c-v0-b.ic44", 0x00000, 0x10000, CRC(d0c27e58) SHA1(fec76217cc0c04c723989c3ec127a2bd33d64c60) )

	ROM_REGION( 0x0200, "proms", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0600, "pals", 0 )
	ROM_LOAD( "m72_a-3d-.ic11", 0x0000, 0x0117, CRC(8a3732ff) SHA1(6e3039e7dc424cbef7156312fa1ce67d7b082d30) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_a-4d-.ic19", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // PAL16L8 - bruteforced
	ROM_LOAD( "mh-c-3f-.ic13",  0x0400, 0x0117, CRC(2d774e1e) SHA1(373c3cbfaf983961c17ebe96b5aff850f36cb30c) ) // PAL16L8 - bruteforced - located on M72-C-A top board
ROM_END

/*

Ninja Spirit
Irem, 1988

This game runs on Irem M72 hardware.


Top Board
---------

M72-C
|-------------------------------|
| 8MHz  J3 NIN_C-L3.6A   J4  J5 |
|     J1   NIN_C-L2.6B NIN-V0.7A|
|8751 J2   NIN_C-L1.6C   J12    |
|          NIN_C-L0.6D   J6     |
|                     NIN-R30.7D|
|      NIN_C-3F.3F              |
|            4364     NIN-R20.7F|
|            4364 J8          J9|
|                   J10         |
| LM358     NIN_C-H0.H6         |
|           NIN_C-H1.J6         |
|----|   MB8431     J11         |
     | CN3                  CN4 |
     |   MB8421       NIN-R10.7J|
     |     NIN_C-H2.6L NIN-R00.7M
     |     NIN_C-H3.6M   J7     |
     |--------------------------|
Notes:
      NIN_C-3F.3F - Ti TBP16L8 PAL
      All other NIN_C* - 27C512 EPROM
      NIN-R* - 28-pin 1Mb maskROM
      J1   - Jumper set to A
      J2   - Jumper set to A
      J3   - Jumper set to A
      J4   - Jumper set to A
      J5   - Jumper set to B
      J6   - Jumper set to A
      J7   - Jumper set to A
      J8   - Open
      J9   - Jumper set to A
      J10  - Jumper set to A
      J11  - Jumper set to B
      J12  - Jumper set to A
      8751 - MCU with label 'NIN C-PR' at location 1C. Clock input 8MHz
      4364 - 8kb x8-bit SRAM
      MB8431 - Fujitsu 2k x 8-bit CMOS Dual-Port SRAM
      MB8421 - Fujitsu 2k x 8-bit CMOS Dual-Port SRAM
      LM358  - Low power dual operational amplifier IC


Middle Board
------------

M72-A-C
|-----------------------------------------------------------|
|M51516L     YM2151                                         |
|VOL         Y3014B                3.579545KHz             |--|
|                                               43256      |  |
|         CN3                                              |  |
|           M72_A-3D.3D  M72_A-4D.4D   D780     43256      |  |
|                                                          |  |
|                                                          |  |
|J   DSW1(8)                                               |  |
|A                 2016                                    |  |
|M   DSW2(8)                               D71011   D71088 |--|
|M                 2016                                     |
|A                                       V30     D71059    |--|
|                  2018   KNA70H016(12)                    |  |
|                                          M72_A-8L.8L     |  |
|                  2018                         M72_A-9L.9L|  |
|                              2018                        |  |
|         KNA71H010(14)        2018           KNA70H015(11)|  |
|                              2018                        |  |
|         CN4                  2018         32MHz          |  |
|         KNA71H009(13)                                    |--|
|KNA71H010(15)                    KNA65005(17)  KNA91H014   |
|-----------------------------------------------------------|
Notes:
      KNA*     - NANAO custom chips
      M72*.*L  - Bipolar PROMs type TBP24S10 (==82S129)
      M72*.*D  - Ti TBP16L8 PALs
      2016     - 2kb x8-bit SRAM
      2018     - 2kb x8-bit SRAM
      43256    - 32kb x8-bit SRAM
      CN3/CN4  - Joining connectors for top board


Bottom Board
------------

M72-B-D
|-----------------------------------------------------------|
|                                                           |
|            NIN_B-A2.4B                                   |--|
|                                                          |  |
| KNA6034201             J2                                |  |
|            NIN_B-A0.4C J3                                |  |
|                                                          |  |
|                                                          |  |
|            NIN_B-A1.4D  4364                             |  |
|                                                          |  |
|            NIN_B-A3.4E  4364                             |--|
|                                                           |
|            B3.4F        4364                             |--|
|                                                          |  |
|            B2.4H        4364                             |  |
|                                                          |  |
| KNA6034201 B0.4J                                         |  |
|                                                          |  |
|                        J4                                |  |
|            B1.4K       J5                                |  |
| KNA91H014                                                |--|
|                                                           |
|-----------------------------------------------------------|
Notes:
      KNA* - NANAO custom chips
      NIN* - 27C512 EPROMs
      B*   - 512kb mask ROMs (no labels, just B and a number)
      4364 - 8kb x8-bit SRAM
      J*   - 3-pin jumpers. All set to the B position
*/

ROM_START( nspirit )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "nin_c-h0-b.ic40", 0x00001, 0x10000, CRC(035692fa) SHA1(d5ab54488344bf405063737ed55d68ff1e64b55f) )
	ROM_LOAD16_BYTE( "nin_c-l0-b.ic37", 0x00000, 0x10000, CRC(9a405898) SHA1(b28d71c1a6410720a37e6b6518b3cc66d4c32972) )
	ROM_LOAD16_BYTE( "nin_c-h1-.ic41",  0x20001, 0x10000, CRC(cbc10586) SHA1(9b1935ea9ebb21fe42ee3a57d6c10f1e8516f23c) )
	ROM_LOAD16_BYTE( "nin_c-l1-.ic36",  0x20000, 0x10000, CRC(b75c9a4d) SHA1(03c28896cbe0c9f778c259d59d2e69796902daa8) )
	ROM_LOAD16_BYTE( "nin_c-h2-.ic42",  0x40001, 0x10000, CRC(8ad818fa) SHA1(dd25e79b656b7fc6c31d1f8971fd0916295ccdb0) )
	ROM_LOAD16_BYTE( "nin_c-l2-.ic35",  0x40000, 0x10000, CRC(c52ca78c) SHA1(2b40cce5a1f5c588b49634e7fd4bc28c9160fe43) )
	ROM_LOAD16_BYTE( "nin_c-h3-b.ic43", 0x60001, 0x10000, CRC(501104ef) SHA1(e44e060c072affd359e52bf6606b1dd565368d44) )
	ROM_RELOAD(                         0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "nin_c-l3-b.ic34", 0x60000, 0x10000, CRC(fd7408b8) SHA1(3cbe72835a561c50265a047f0f5cd62db48378fd) )
	ROM_RELOAD(                         0xe0000, 0x10000 )

	ROM_REGION( 0x1000, "mcu", 0 )  // i8751 microcontroller
	ROM_LOAD( "nin_c-pr-b.ic1", 0x0000, 0x1000, NO_DUMP ) // i8751 MCU labeled  NIN C-PR-B  - read protected

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "nin-r00.ic53",  0x00000, 0x20000, CRC(5f61d30b) SHA1(7754697e43f6117fa604f50885b76014b1dc5760) )  // sprites
	ROM_LOAD( "nin-r10.ic51",  0x20000, 0x20000, CRC(0caad107) SHA1(c4eff00327313e05ac8f7c6dbee3a0de1c83fadd) )
	ROM_LOAD( "nin-r20.ic49",  0x40000, 0x20000, CRC(ef3617d3) SHA1(16c175cf45559aacdea6e4002dd8a87f16817cfb) )
	ROM_LOAD( "nin-r30.ic47",  0x60000, 0x20000, CRC(175d2a24) SHA1(d1887efd4d8e74c38c53dbbc541ca8d17f29eb59) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "nin_b-a0.ic21", 0x00000, 0x10000, CRC(63f8f658) SHA1(82c02d0f7a2d95dfd8d300c46312d511524775ce) )  // tiles #1
	ROM_LOAD( "nin_b-a1.ic22", 0x10000, 0x10000, CRC(75eb8306) SHA1(2abc359a0bb2863759a68ed60e730761b9751829) )
	ROM_LOAD( "nin_b-a2.ic20", 0x20000, 0x10000, CRC(df532172) SHA1(58b5a79a57e71405b3e1abd41d54cf6a4d12873a) )
	ROM_LOAD( "nin_b-a3.ic23", 0x30000, 0x10000, CRC(4dedd64c) SHA1(8a5c73a024d95e6fe3ab70daafcd5b235418ad36) )

	ROM_REGION( 0x040000, "gfx3", 0 )
	ROM_LOAD( "b0.ic26",       0x00000, 0x10000, CRC(1b0e08a6) SHA1(892686594970c264babbe8673c258929a5e480f6) )  // tiles #2
	ROM_LOAD( "b1.ic27",       0x10000, 0x10000, CRC(728727f0) SHA1(2f594c77a847ebee71c9da8a644f83ea2a1313d7) )
	ROM_LOAD( "b2.ic25",       0x20000, 0x10000, CRC(f87efd75) SHA1(16474c7ab57b4fbb5cb50799ea6a2326c66706b5) )
	ROM_LOAD( "b3.ic24",       0x30000, 0x10000, CRC(98856cb4) SHA1(aa4fbae972d2e827c75650a71ab4ef73a33cd018) )

	ROM_REGION( 0x10000, "samples", 0 ) // samples
	ROM_LOAD( "nin-v0.ic44",   0x00000, 0x10000, CRC(a32e8caf) SHA1(63d56ad3a63fb089056e4a170159120287594ea8) )

	ROM_REGION( 0x0200, "proms", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0600, "pals", 0 )
	ROM_LOAD( "m72_a-3d-.ic11", 0x0000, 0x0117, CRC(8a3732ff) SHA1(6e3039e7dc424cbef7156312fa1ce67d7b082d30) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_a-4d-.ic19", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // PAL16L8 - bruteforced
	ROM_LOAD( "nin-c-3f.ic13",  0x0400, 0x0117, CRC(d1f3a2e2) SHA1(b84d734d3cd6b4ab656fbfe686daf857d61aff31) ) // PAL16L8 - bruteforced - located on M72-C-A top board
ROM_END

ROM_START( nspiritj )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "nin_c-h0-.ic40", 0x00001, 0x10000, CRC(8603fab2) SHA1(2c5bc97b6c9648156969b4a9f139081dca19fa24) )
	ROM_LOAD16_BYTE( "nin_c-l0-.ic37", 0x00000, 0x10000, CRC(e520fa35) SHA1(05f7e5a1a5ada95809ffd941080fb2c2b54363b7) )
	ROM_LOAD16_BYTE( "nin_c-h1-.ic41", 0x20001, 0x10000, CRC(cbc10586) SHA1(9b1935ea9ebb21fe42ee3a57d6c10f1e8516f23c) )
	ROM_LOAD16_BYTE( "nin_c-l1-.ic36", 0x20000, 0x10000, CRC(b75c9a4d) SHA1(03c28896cbe0c9f778c259d59d2e69796902daa8) )
	ROM_LOAD16_BYTE( "nin_c-h2-.ic42", 0x40001, 0x10000, CRC(8ad818fa) SHA1(dd25e79b656b7fc6c31d1f8971fd0916295ccdb0) )
	ROM_LOAD16_BYTE( "nin_c-l2-.ic35", 0x40000, 0x10000, CRC(c52ca78c) SHA1(2b40cce5a1f5c588b49634e7fd4bc28c9160fe43) )
	ROM_LOAD16_BYTE( "nin_c-h3-.ic43", 0x60001, 0x10000, CRC(95b63a61) SHA1(bd5ec35fffe6d4898e6712eb6add7c51077b58d2) )
	ROM_RELOAD(                        0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "nin_c-l3-.ic34", 0x60000, 0x10000, CRC(e754a87a) SHA1(9951d972ed13a0415c827beff122bc7ddb078447) )
	ROM_RELOAD(                        0xe0000, 0x10000 )

	ROM_REGION( 0x1000, "mcu", 0 )  // i8751 microcontroller
	ROM_LOAD( "nin_c-pr-.ic1", 0x0000, 0x1000, CRC(802d440a) SHA1(45b844b831aa6d5d002e3960e17fb5a058b02a29) ) // i8751 MCU labeled  NIN C-PR-

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "nin-r00.ic53",  0x00000, 0x20000, CRC(5f61d30b) SHA1(7754697e43f6117fa604f50885b76014b1dc5760) )  // sprites
	ROM_LOAD( "nin-r10.ic51",  0x20000, 0x20000, CRC(0caad107) SHA1(c4eff00327313e05ac8f7c6dbee3a0de1c83fadd) )
	ROM_LOAD( "nin-r20.ic49",  0x40000, 0x20000, CRC(ef3617d3) SHA1(16c175cf45559aacdea6e4002dd8a87f16817cfb) )
	ROM_LOAD( "nin-r30.ic47",  0x60000, 0x20000, CRC(175d2a24) SHA1(d1887efd4d8e74c38c53dbbc541ca8d17f29eb59) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "nin_b-a0.ic21", 0x00000, 0x10000, CRC(63f8f658) SHA1(82c02d0f7a2d95dfd8d300c46312d511524775ce) )  // tiles #1
	ROM_LOAD( "nin_b-a1.ic22", 0x10000, 0x10000, CRC(75eb8306) SHA1(2abc359a0bb2863759a68ed60e730761b9751829) )
	ROM_LOAD( "nin_b-a2.ic20", 0x20000, 0x10000, CRC(df532172) SHA1(58b5a79a57e71405b3e1abd41d54cf6a4d12873a) )
	ROM_LOAD( "nin_b-a3.ic23", 0x30000, 0x10000, CRC(4dedd64c) SHA1(8a5c73a024d95e6fe3ab70daafcd5b235418ad36) )

	ROM_REGION( 0x040000, "gfx3", 0 )
	ROM_LOAD( "b0.ic26",       0x00000, 0x10000, CRC(1b0e08a6) SHA1(892686594970c264babbe8673c258929a5e480f6) )  // tiles #2
	ROM_LOAD( "b1.ic27",       0x10000, 0x10000, CRC(728727f0) SHA1(2f594c77a847ebee71c9da8a644f83ea2a1313d7) )
	ROM_LOAD( "b2.ic25",       0x20000, 0x10000, CRC(f87efd75) SHA1(16474c7ab57b4fbb5cb50799ea6a2326c66706b5) )
	ROM_LOAD( "b3.ic24",       0x30000, 0x10000, CRC(98856cb4) SHA1(aa4fbae972d2e827c75650a71ab4ef73a33cd018) )

	ROM_REGION( 0x10000, "samples", 0 ) // samples
	ROM_LOAD( "nin-v0.ic44",   0x00000, 0x10000, CRC(a32e8caf) SHA1(63d56ad3a63fb089056e4a170159120287594ea8) )

	ROM_REGION( 0x0200, "proms", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0600, "pals", 0 )
	ROM_LOAD( "m72_a-3d-.ic11", 0x0000, 0x0117, CRC(8a3732ff) SHA1(6e3039e7dc424cbef7156312fa1ce67d7b082d30) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_a-4d-.ic19", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // PAL16L8 - bruteforced
	ROM_LOAD( "nin-c-3f.ic13",  0x0400, 0x0117, CRC(d1f3a2e2) SHA1(b84d734d3cd6b4ab656fbfe686daf857d61aff31) ) // PAL16L8 - bruteforced - located on M72-C-A top board
ROM_END


ROM_START( imgfight )
	ROM_REGION( 0x100000, "maincpu", 0 ) // M72-C top board
	ROM_LOAD16_BYTE( "if_c-h0-a.ic40", 0x00001, 0x10000, CRC(f5c94464) SHA1(5964a00d21ebb358eecc0f10f6221fb684f284df) )
	ROM_LOAD16_BYTE( "if_c-l0-a.ic37", 0x00000, 0x10000, CRC(87c534fe) SHA1(10c231a2b3046a711a1fdcc6c1631a7378295f2f) )
	ROM_LOAD16_BYTE( "if_c-h3-.ic43",  0x40001, 0x20000, CRC(ea030541) SHA1(ee4c12773ecced2d755443ce0ca78fb2b2c04805) )
	ROM_RELOAD(                        0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "if_c-l3-.ic34",  0x40000, 0x20000, CRC(c66ae348) SHA1(eca5096ebd5bffc6e68f3fc9969cda9679bd921f) )
	ROM_RELOAD(                        0xc0000, 0x20000 )

	ROM_REGION( 0x1000, "mcu", 0 )  // i8751 microcontroller
	ROM_LOAD( "if_c-pr-a.ic1", 0x0000, 0x1000, CRC(55f10458) SHA1(d520ec2b075c94d76d97e0105644ff96384b378c) ) // i8751 MCU labeled  IF C-PR-A

	ROM_REGION( 0x080000, "sprites", 0 ) // mask ROMs
	ROM_LOAD( "if-c-00.ic53", 0x00000, 0x20000, CRC(745e6638) SHA1(43fb1f9da4190fea67eee3aee8caf4219becc21b) )  // sprites
	ROM_LOAD( "if-c-10.ic51", 0x20000, 0x20000, CRC(b7108449) SHA1(1f41ebe7164fab86958caaf6749b99425e682657) )
	ROM_LOAD( "if-c-20.ic49", 0x40000, 0x20000, CRC(aef33cba) SHA1(2d8a8458207d0c790c81b1285366463c8540d190) )
	ROM_LOAD( "if-c-30.ic47", 0x60000, 0x20000, CRC(1f98e695) SHA1(5fddcfb17523f8e96f4b85f0cb15d837b81f2bd4) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "if_b-a0.ic21", 0x00000, 0x10000, CRC(34ee2d77) SHA1(38826e0318aa8da893fa4c93f217288c015df606) )  // tiles #1
	ROM_LOAD( "if_b-a1.ic22", 0x10000, 0x10000, CRC(6bd2845b) SHA1(149cf14f919590da88b9a8e254690da010709862) )
	ROM_LOAD( "if_b-a2.ic20", 0x20000, 0x10000, CRC(090d50e5) SHA1(4f2a7c76320b3f8dafae90a246187e034fe7562b) )
	ROM_LOAD( "if_b-a3.ic23", 0x30000, 0x10000, CRC(3a8e3083) SHA1(8a75d556790b6bea41ead1a5f95589dd293bdf4e) )

	ROM_REGION( 0x040000, "gfx3", 0 )
	ROM_LOAD( "if_b-b0.ic26", 0x00000, 0x10000, CRC(b425c829) SHA1(0ccd487dba00bb7cb0ff5d1c67f8fee3e68df5d8) )  // tiles #2
	ROM_LOAD( "if_b-b1.ic27", 0x10000, 0x10000, CRC(e9bfe23e) SHA1(f97a68dbdce7e06d07faab19acf7625cdc8eeaa8) )
	ROM_LOAD( "if_b-b2.ic25", 0x20000, 0x10000, CRC(256e50f2) SHA1(9e9fda4f1f1449548942c0da4478f61fe0d263d1) )
	ROM_LOAD( "if_b-b3.ic24", 0x30000, 0x10000, CRC(4c682785) SHA1(f61f1227e0ad629fdfca106306b17a9f6a9959e3) )

	ROM_REGION( 0x20000, "samples", 0 ) // samples
	ROM_LOAD( "if_c-v0.ic44", 0x00000, 0x10000, CRC(cb64a194) SHA1(940fad6b9147bccc8290e112f5973f8ea062b52f) )
	ROM_LOAD( "if_c-v1.ic45", 0x10000, 0x10000, CRC(45b68bf5) SHA1(2fb28793019ca85b3b6d7c4c31eedff1d71f2d83) )

	ROM_REGION( 0x0200, "proms", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0600, "pals", 0 )
	ROM_LOAD( "m72_a-3d-.ic11", 0x0000, 0x0117, CRC(8a3732ff) SHA1(6e3039e7dc424cbef7156312fa1ce67d7b082d30) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_a-4d-.ic19", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // PAL16L8 - bruteforced
	ROM_LOAD( "if-c-3f-.ic13",  0x0400, 0x0117, CRC(2d774e1e) SHA1(373c3cbfaf983961c17ebe96b5aff850f36cb30c) ) // PAL16L8 - bruteforced - located on M72-C-A top board
ROM_END

ROM_START( imgfightj )
	ROM_REGION( 0x100000, "maincpu", 0 ) // M72-C top board
	ROM_LOAD16_BYTE( "if_c-h0-.ic40", 0x00001, 0x10000, CRC(592d2d80) SHA1(d54916a9bfe4b65a972b62202af706135e73518d) )
	ROM_LOAD16_BYTE( "if_c-l0-.ic37", 0x00000, 0x10000, CRC(61f89056) SHA1(3e0724dbc2b00a30193ea6cfac8b4331055d4fd4) )
	ROM_LOAD16_BYTE( "if_c-h3-.ic43", 0x40001, 0x20000, CRC(ea030541) SHA1(ee4c12773ecced2d755443ce0ca78fb2b2c04805) )
	ROM_RELOAD(                       0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "if_c-l3-.ic34", 0x40000, 0x20000, CRC(c66ae348) SHA1(eca5096ebd5bffc6e68f3fc9969cda9679bd921f) )
	ROM_RELOAD(                       0xc0000, 0x20000 )

	ROM_REGION( 0x1000, "mcu", 0 )  // i8751 microcontroller
	ROM_LOAD( "if_c-pr-.ic1", 0x0000, 0x1000, CRC(ef0d5098) SHA1(068b73937588e16a318a094dfe2fb1293b1a1711) ) // i8751 MCU labeled  IF C-PR-

	ROM_REGION( 0x080000, "sprites", 0 ) // mask ROMs
	ROM_LOAD( "if-c-00.ic53", 0x00000, 0x20000, CRC(745e6638) SHA1(43fb1f9da4190fea67eee3aee8caf4219becc21b) )  // sprites
	ROM_LOAD( "if-c-10.ic51", 0x20000, 0x20000, CRC(b7108449) SHA1(1f41ebe7164fab86958caaf6749b99425e682657) )
	ROM_LOAD( "if-c-20.ic49", 0x40000, 0x20000, CRC(aef33cba) SHA1(2d8a8458207d0c790c81b1285366463c8540d190) )
	ROM_LOAD( "if-c-30.ic47", 0x60000, 0x20000, CRC(1f98e695) SHA1(5fddcfb17523f8e96f4b85f0cb15d837b81f2bd4) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "if_b-a0.ic21", 0x00000, 0x10000, CRC(34ee2d77) SHA1(38826e0318aa8da893fa4c93f217288c015df606) )  // tiles #1
	ROM_LOAD( "if_b-a1.ic22", 0x10000, 0x10000, CRC(6bd2845b) SHA1(149cf14f919590da88b9a8e254690da010709862) )
	ROM_LOAD( "if_b-a2.ic20", 0x20000, 0x10000, CRC(090d50e5) SHA1(4f2a7c76320b3f8dafae90a246187e034fe7562b) )
	ROM_LOAD( "if_b-a3.ic23", 0x30000, 0x10000, CRC(3a8e3083) SHA1(8a75d556790b6bea41ead1a5f95589dd293bdf4e) )

	ROM_REGION( 0x040000, "gfx3", 0 )
	ROM_LOAD( "if_b-b0.ic26", 0x00000, 0x10000, CRC(b425c829) SHA1(0ccd487dba00bb7cb0ff5d1c67f8fee3e68df5d8) )  // tiles #2
	ROM_LOAD( "if_b-b1.ic27", 0x10000, 0x10000, CRC(e9bfe23e) SHA1(f97a68dbdce7e06d07faab19acf7625cdc8eeaa8) )
	ROM_LOAD( "if_b-b2.ic25", 0x20000, 0x10000, CRC(256e50f2) SHA1(9e9fda4f1f1449548942c0da4478f61fe0d263d1) )
	ROM_LOAD( "if_b-b3.ic24", 0x30000, 0x10000, CRC(4c682785) SHA1(f61f1227e0ad629fdfca106306b17a9f6a9959e3) )

	ROM_REGION( 0x20000, "samples", 0 ) // samples
	ROM_LOAD( "if_c-v0.ic44", 0x00000, 0x10000, CRC(cb64a194) SHA1(940fad6b9147bccc8290e112f5973f8ea062b52f) )
	ROM_LOAD( "if_c-v1.ic45", 0x10000, 0x10000, CRC(45b68bf5) SHA1(2fb28793019ca85b3b6d7c4c31eedff1d71f2d83) )

	ROM_REGION( 0x0200, "proms", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0600, "pals", 0 )
	ROM_LOAD( "m72_a-3d-.ic11", 0x0000, 0x0117, CRC(8a3732ff) SHA1(6e3039e7dc424cbef7156312fa1ce67d7b082d30) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_a-4d-.ic19", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // PAL16L8 - bruteforced
	ROM_LOAD( "if-c-3f-.ic13",  0x0400, 0x0117, CRC(2d774e1e) SHA1(373c3cbfaf983961c17ebe96b5aff850f36cb30c) ) // PAL16L8 - bruteforced - located on M72-C-A top board
ROM_END

ROM_START( imgfightjb ) // identical to imgfightj content-wise, it's a 4 PCB stack bootleg with flying wires
	ROM_REGION( 0x100000, "maincpu", 0 ) // identical
	ROM_LOAD16_BYTE( "ic108.9b", 0x00001, 0x10000, CRC(592d2d80) SHA1(d54916a9bfe4b65a972b62202af706135e73518d) )
	ROM_LOAD16_BYTE( "ic89.7b",  0x00000, 0x10000, CRC(61f89056) SHA1(3e0724dbc2b00a30193ea6cfac8b4331055d4fd4) )
	ROM_LOAD16_BYTE( "ic111.9e", 0x40001, 0x10000, CRC(6aae3a46) SHA1(87fe2e13b4dd98c6cbec03ec52dfae1980403125) )
	ROM_RELOAD(                  0xc0001, 0x10000 )
	ROM_LOAD16_BYTE( "ic110.9d", 0x60001, 0x10000, CRC(0e0aefcd) SHA1(f5056a2d0612d912aff1e0eccb1182de7ae16990) )
	ROM_RELOAD(                  0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "ic92.7e",  0x40000, 0x10000, CRC(38fce272) SHA1(4fe4d0838d21f3022b440a32ec69b25e936e62dd) )
	ROM_RELOAD(                  0xc0000, 0x10000 )
	ROM_LOAD16_BYTE( "ic91.7d",  0x60000, 0x10000, CRC(d69c0722) SHA1(ef18e7b7057f19caaa61d0b8c07d2d0c6e0a555e) )
	ROM_RELOAD(                  0xe0000, 0x10000 )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "25.ic27.2l", 0x00000, 0x2000, CRC(d83359a2) SHA1(2d486bf4a873abfe591e0d9383f9e230f47bc42a) ) // i80c31 instead of i8751, contents identical to imgfightj MCU, with second half padded with 0xff

	ROM_REGION( 0x080000, "sprites", 0 ) // half size ROMs, but identical content
	ROM_LOAD( "ic96.7k",  0x00000, 0x10000, CRC(d4febb03) SHA1(6fe53b198bdcef1708ff134c64af9c064e274e1b) )  // sprites
	ROM_LOAD( "ic97.7l",  0x10000, 0x10000, CRC(973d7bbc) SHA1(409242ddc7eb90564a15b222641e53d11ab1e04a) )
	ROM_LOAD( "ic115.9k", 0x20000, 0x10000, CRC(2328880b) SHA1(a35c77a7d04614dbfbca4c79bb3e729115129ee4) )
	ROM_LOAD( "ic116.9l", 0x30000, 0x10000, CRC(6da001ea) SHA1(473ed89b77809b3b76bfa9f5ca4008c9534cdbb4) )
	ROM_LOAD( "ic94.7h",  0x40000, 0x10000, CRC(92bc7fda) SHA1(521d4a29e06eb8790fdeeba968f99a389f50a24e) )
	ROM_LOAD( "ic95.7j",  0x50000, 0x10000, CRC(e63a5918) SHA1(fd3374866f922cef72c0678aa751ad1e6f95a12a) )
	ROM_LOAD( "ic113.9h", 0x60000, 0x10000, CRC(27caec8e) SHA1(cc1943ba9548715425e799f418750cd70c3f88da) )
	ROM_LOAD( "ic114.9j", 0x70000, 0x10000, CRC(1933eb65) SHA1(4c24cfd059c11875f53b57cc020fbdbac903bd4a) )

	ROM_REGION( 0x040000, "gfx2", 0 ) // identical
	ROM_LOAD( "ic30.3d",  0x00000, 0x10000, CRC(34ee2d77) SHA1(38826e0318aa8da893fa4c93f217288c015df606) )  // tiles #1
	ROM_LOAD( "ic31.3e",  0x10000, 0x10000, CRC(6bd2845b) SHA1(149cf14f919590da88b9a8e254690da010709862) )
	ROM_LOAD( "ic29.3c",  0x20000, 0x10000, CRC(090d50e5) SHA1(4f2a7c76320b3f8dafae90a246187e034fe7562b) )
	ROM_LOAD( "ic32.3f",  0x30000, 0x10000, CRC(3a8e3083) SHA1(8a75d556790b6bea41ead1a5f95589dd293bdf4e) )

	ROM_REGION( 0x040000, "gfx3", 0 ) // identical
	ROM_LOAD( "ic35.3k",  0x00000, 0x10000, CRC(b425c829) SHA1(0ccd487dba00bb7cb0ff5d1c67f8fee3e68df5d8) )  // tiles #2
	ROM_LOAD( "ic36.3l",  0x10000, 0x10000, CRC(e9bfe23e) SHA1(f97a68dbdce7e06d07faab19acf7625cdc8eeaa8) )
	ROM_LOAD( "ic34.3j",  0x20000, 0x10000, CRC(256e50f2) SHA1(9e9fda4f1f1449548942c0da4478f61fe0d263d1) )
	ROM_LOAD( "ic33.3h",  0x30000, 0x10000, CRC(4c682785) SHA1(f61f1227e0ad629fdfca106306b17a9f6a9959e3) )

	ROM_REGION( 0x20000, "samples", 0 ) // samples, identical
	ROM_LOAD( "ic28.lower.2n",  0x00000, 0x10000, CRC(cb64a194) SHA1(940fad6b9147bccc8290e112f5973f8ea062b52f) )
	ROM_LOAD( "ic28.upper.2n",  0x10000, 0x10000, CRC(45b68bf5) SHA1(2fb28793019ca85b3b6d7c4c31eedff1d71f2d83) )
ROM_END


ROM_START( loht )
	ROM_REGION( 0x100000, "maincpu", 0 ) // M72-C top board
	ROM_LOAD16_BYTE( "tom_c-h0-b.ic40", 0x00001, 0x20000, CRC(a63204b6) SHA1(d217bc70650a1a1bbe0cf536ec3bb678f670718d) )
	ROM_LOAD16_BYTE( "tom_c-l0-b.ic37", 0x00000, 0x20000, CRC(e788002f) SHA1(35f509976b342fd47e645453381faa3d86645876) )
	ROM_LOAD16_BYTE( "tom_c-h3-.ic43",  0x40001, 0x20000, CRC(714778b5) SHA1(e2eaa35d6b5fa5df5163fe0d7b45fa66667f9947) )
	ROM_RELOAD(                         0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "tom_c-l3-.ic34",  0x40000, 0x20000, CRC(2f049b03) SHA1(21047cb10912b1fc23795673af3ea7de249328b7) )
	ROM_RELOAD(                         0xc0000, 0x20000 )

	ROM_REGION( 0x1000, "mcu", 0 )  // i8751 microcontroller
	ROM_LOAD( "tom_c-pr-b.ic1", 0x0000, 0x1000, CRC(9c9545f1) SHA1(ca800ce7467efb877d0fff4c47d72478a991e2a9) ) // i8751 MCU labeled  TOM C-PR-B

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "tom_m53.ic53", 0x00000, 0x20000, CRC(0b83265f) SHA1(b31918d6442b79c9fe4f20410189788b050a994e) )  // sprites
	ROM_LOAD( "tom_m51.ic51", 0x20000, 0x20000, CRC(8ec5f6f3) SHA1(210f2753f5eeb06396758d21ab1778d459add247) )
	ROM_LOAD( "tom_m49.ic49", 0x40000, 0x20000, CRC(a41d3bfd) SHA1(536fb7c0321dbbc1a8b73e9647fba9c53a253fcc) )
	ROM_LOAD( "tom_m47.ic47", 0x60000, 0x20000, CRC(9d81a25b) SHA1(a354537c2fbba85f06485aa8487d7583a7133357) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "tom_m21.ic21", 0x00000, 0x10000, CRC(3ca3e771) SHA1(be052e01c5429ee89057c9d408794f2c7744047c) )  // tiles #1
	ROM_LOAD( "tom_m22.ic22", 0x10000, 0x10000, CRC(7a05ee2f) SHA1(7d1ca5db9a5a85610129e3bc6c640ade036fe7f9) )
	ROM_LOAD( "tom_m20.ic20", 0x20000, 0x10000, CRC(79aa2335) SHA1(6b70c79d800a7b755aa7c9a368c4ea74029aaa1e) )
	ROM_LOAD( "tom_m23.ic23", 0x30000, 0x10000, CRC(789e8b24) SHA1(e957cd25c3c155ca295ab1aea03d610f91562cfb) )

	ROM_REGION( 0x040000, "gfx3", 0 )
	ROM_LOAD( "tom_m26.ic26", 0x00000, 0x10000, CRC(44626bf6) SHA1(571ef74d42d30a272ff0fb33f830652b4a4bad29) )  // tiles #2
	ROM_LOAD( "tom_m27.ic27", 0x10000, 0x10000, CRC(464952cf) SHA1(6b99360b6ba1ed5a72c257f51291f9f7a1ddf363) )
	ROM_LOAD( "tom_m25.ic25", 0x20000, 0x10000, CRC(3db9b2c7) SHA1(02a318ffc459c494b7f40827eff5f89b41ac0426) )
	ROM_LOAD( "tom_m24.ic24", 0x30000, 0x10000, CRC(f01fe899) SHA1(c5ab967b7af55a757638bcdc9975f4b15064022d) )

	ROM_REGION( 0x10000, "samples", 0 ) // samples
	ROM_LOAD( "tom_m44.ic44", 0x00000, 0x10000, CRC(3ed51d1f) SHA1(84f3aa17d640df91387e5f1f5b5971cf8dcd4e17) )

	ROM_REGION( 0x0200, "proms", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0600, "pals", 0 )
	ROM_LOAD( "m72_a-3d-.ic11", 0x0000, 0x0117, CRC(8a3732ff) SHA1(6e3039e7dc424cbef7156312fa1ce67d7b082d30) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_a-4d-.ic19", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // PAL16L8 - bruteforced
	ROM_LOAD( "tom-c-3f.ic13",  0x0400, 0x0117, CRC(2d774e1e) SHA1(373c3cbfaf983961c17ebe96b5aff850f36cb30c) ) // PAL16L8 - bruteforced - located on M72-C-A top board
ROM_END

/*

Legend of Hero TONMA
(c)1989 Irem

M72 System
Horizontal Freq. = 15.625KHz
H.Period         = 64.0us
H.Blank          = 16.0us
H.Sync Pulse     = 5.0us
Vertical Freq.   = 55.02Hz
V.Period         = 18.176ms
V.Blank          = 1.792ms
V.Sync Pulse     = 384us

ROMs:
on M72-C mainboard
set jumper pins
J1:A
J2:A
J3:A
J4:A
J5:B
J6:A
J7:A
J9:A
J10:A
J11:B
J12:A

TOM_C-H0- (M5M27C101K, main programs)
TOM_C-L0-
TOM_C-H3-
TOM_C-H0-

R200 (28pin 1Mbit mask, read as 531000)
R210
R220
R230

082 - Samples

TOM_C-PR- (i8751H, read protected, not dumped) <-- Since decapped, deprotected and read
TOM_C-3F- (PAL, read protected, not dumped)


on M72-B-B or M72-B-C or M72-B-D
set jumper pins (J2, J3, J4, J5) to "B"
R2A0.A0 (27C512)
R2A1.A1
R2A2.A2
R2A3.A3

078.B0
079.B1
080.B2
081.B3

*/

ROM_START( lohtj )
	ROM_REGION( 0x100000, "maincpu", 0 ) // M72-B-C top board
	ROM_LOAD16_BYTE( "tom_c-h0-.ic40", 0x00001, 0x20000, CRC(2a752998) SHA1(a88c3c75a1106665c94ddd0945bfaa7696a21b75) )
	ROM_LOAD16_BYTE( "tom_c-l0-.ic37", 0x00000, 0x20000, CRC(a224d928) SHA1(c4744f6ca19ce60b0c03415be979f2a824235a1c) )
	ROM_LOAD16_BYTE( "tom_c-h3-.ic43", 0x40001, 0x20000, CRC(714778b5) SHA1(e2eaa35d6b5fa5df5163fe0d7b45fa66667f9947) )
	ROM_RELOAD(                        0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "tom_c-l3-.ic34", 0x40000, 0x20000, CRC(2f049b03) SHA1(21047cb10912b1fc23795673af3ea7de249328b7) )
	ROM_RELOAD(                        0xc0000, 0x20000 )

	ROM_REGION( 0x1000, "mcu", 0 )  // i8751 microcontroller
	ROM_LOAD( "tom_c-pr-.ic1", 0x0000, 0x1000, CRC(9fa9b496) SHA1(b529bcd7bf123894e11f2a8df8826932122e375a) ) // i8751 MCU labeled  TOM C-PR-

	ROM_REGION( 0x080000, "sprites", 0 ) // same data as loht above, just mask ROMs without labels
	ROM_LOAD( "r200.ic53",    0x00000, 0x20000, CRC(0b83265f) SHA1(b31918d6442b79c9fe4f20410189788b050a994e) )  // sprites
	ROM_LOAD( "r210.ic51",    0x20000, 0x20000, CRC(8ec5f6f3) SHA1(210f2753f5eeb06396758d21ab1778d459add247) )
	ROM_LOAD( "r220.ic49",    0x40000, 0x20000, CRC(a41d3bfd) SHA1(536fb7c0321dbbc1a8b73e9647fba9c53a253fcc) )
	ROM_LOAD( "r230.ic47",    0x60000, 0x20000, CRC(9d81a25b) SHA1(a354537c2fbba85f06485aa8487d7583a7133357) )

	ROM_REGION( 0x040000, "gfx2", 0 ) // same data as loht above, just mask ROMs without labels
	ROM_LOAD( "r2a0.a0.ic21", 0x00000, 0x10000, CRC(3ca3e771) SHA1(be052e01c5429ee89057c9d408794f2c7744047c) )  // tiles #1
	ROM_LOAD( "r2a1.a1.ic22", 0x10000, 0x10000, CRC(7a05ee2f) SHA1(7d1ca5db9a5a85610129e3bc6c640ade036fe7f9) )
	ROM_LOAD( "r2a2.a2.ic20", 0x20000, 0x10000, CRC(79aa2335) SHA1(6b70c79d800a7b755aa7c9a368c4ea74029aaa1e) )
	ROM_LOAD( "r2a3.a3.ic23", 0x30000, 0x10000, CRC(789e8b24) SHA1(e957cd25c3c155ca295ab1aea03d610f91562cfb) )

	ROM_REGION( 0x040000, "gfx3", 0 ) // same data as loht above, just mask ROMs without labels
	ROM_LOAD( "078.b0.ic26",  0x00000, 0x10000, CRC(44626bf6) SHA1(571ef74d42d30a272ff0fb33f830652b4a4bad29) )  // tiles #2
	ROM_LOAD( "079.b1.ic27",  0x10000, 0x10000, CRC(464952cf) SHA1(6b99360b6ba1ed5a72c257f51291f9f7a1ddf363) )
	ROM_LOAD( "080.b2.ic25",  0x20000, 0x10000, CRC(3db9b2c7) SHA1(02a318ffc459c494b7f40827eff5f89b41ac0426) )
	ROM_LOAD( "081.b3.ic24",  0x30000, 0x10000, CRC(f01fe899) SHA1(c5ab967b7af55a757638bcdc9975f4b15064022d) )

	ROM_REGION( 0x10000, "samples", 0 ) // same data as loht above, just mask ROM without label
	ROM_LOAD( "082.ic44",     0x00000, 0x10000, CRC(3ed51d1f) SHA1(84f3aa17d640df91387e5f1f5b5971cf8dcd4e17) ) // samples

	ROM_REGION( 0x0200, "proms", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0600, "pals", 0 )
	ROM_LOAD( "m72_a-3d-.ic11", 0x0000, 0x0117, CRC(8a3732ff) SHA1(6e3039e7dc424cbef7156312fa1ce67d7b082d30) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_a-4d-.ic19", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // PAL16L8 - bruteforced
	ROM_LOAD( "tom-c-3f.ic13",  0x0400, 0x0117, CRC(2d774e1e) SHA1(373c3cbfaf983961c17ebe96b5aff850f36cb30c) ) // PAL16L8 - bruteforced - located on M72-C-A top board
ROM_END

/*
nec v30
z80 (sharp LH0080B z80b-cpu)
2x YM2203C

Crystals: 16MHz near the v30 and 28MHz near the z80 and ym2203c
*/

ROM_START( lohtb )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lohtb03.b", 0x00001, 0x20000, CRC(8b845a70) SHA1(902c623310cd77b25592496745f9c6121149b516) )
	ROM_LOAD16_BYTE( "lohtb05.d", 0x00000, 0x20000, CRC(e90f7623) SHA1(e6e2f66a39286b2d7c03fc267beb2024913fb4ca) )
	ROM_LOAD16_BYTE( "lohtb02.a", 0x40001, 0x20000, CRC(714778b5) SHA1(e2eaa35d6b5fa5df5163fe0d7b45fa66667f9947) )
	ROM_RELOAD(                   0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "lohtb04.c", 0x40000, 0x20000, CRC(2f049b03) SHA1(21047cb10912b1fc23795673af3ea7de249328b7) )
	ROM_RELOAD(                   0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 ) // Sound CPU program (Z80) + Samples
	ROM_LOAD( "lohtb01.02",  0x00000, 0x10000, CRC(e4bd8f03) SHA1(69fe41a978db92daa912cb345c2c7bafd2a6eb93) )

	ROM_REGION( 0x080000, "sprites", 0 ) // Sprites
	ROM_LOAD( "lohtb14.11",  0x00000, 0x10000, CRC(df5ac5ee) SHA1(5b45417ada402047d97dfb6cee6545686ad26e37) )
	ROM_LOAD( "lohtb15.12",  0x20000, 0x10000, CRC(45220b01) SHA1(83715cf155f91c82067d69f14b3b01ed77777b7d) )
	ROM_LOAD( "lohtb16.13",  0x40000, 0x10000, CRC(25b85cfc) SHA1(c7a9962165379193dc6553ed1f977795a79e0f78) )
	ROM_LOAD( "lohtb17.14",  0x60000, 0x10000, CRC(763fa4ec) SHA1(2d72b1b41f24ae299fde23869942c0b6bbb82363) )
	ROM_LOAD( "lohtb18.15",  0x10000, 0x10000, CRC(d7ecf849) SHA1(ab86a88eae21e054d4e8a740a60c7c6c198232d4) )
	ROM_LOAD( "lohtb19.16",  0x30000, 0x10000, CRC(35d1a808) SHA1(9378ff000104ecfb842b3b884197be82c43a01b4) )
	ROM_LOAD( "lohtb20.17",  0x50000, 0x10000, CRC(464d8579) SHA1(b5981f4865ee5439f0e330091927e6d97d29933f) )
	ROM_LOAD( "lohtb21.18",  0x70000, 0x10000, CRC(a73568c7) SHA1(8fe1867256708cc1ed76d1bed5566b1852b47c40) )

	ROM_REGION( 0x040000, "gfx2", ROMREGION_INVERT )  // tiles #1
	ROM_LOAD( "lohtb13.10",  0x00000, 0x10000, CRC(359f17d4) SHA1(2875ba48395e7faa1a58404475be936dcca45ed1) )
	ROM_LOAD( "lohtb11.08",  0x10000, 0x10000, CRC(73391e8a) SHA1(53ca89b8a10895f817ecdb9fa5eef462edb94ae6) )
	ROM_LOAD( "lohtb09.06",  0x20000, 0x10000, CRC(7096d390) SHA1(f4a16bf8aef7a1a65619ab022cbdb67d2f191888) )
	ROM_LOAD( "lohtb07.04",  0x30000, 0x10000, CRC(71a27b81) SHA1(d8fe72d15bbcd5b170d1123d8f4c58874cefdca3) )

	ROM_REGION( 0x040000, "gfx3", ROMREGION_INVERT )  // tiles #2
	ROM_LOAD( "lohtb12.09",  0x00000, 0x10000, CRC(4d5e9b53) SHA1(3e3977bab7a66ed0171afcd555d181960e338749) )
	ROM_LOAD( "lohtb10.07",  0x10000, 0x10000, CRC(4f75a26a) SHA1(79c09a1ad3a6f9cfbd07cb527bbd89d2478ce582) )
	ROM_LOAD( "lohtb08.05",  0x20000, 0x10000, CRC(34854262) SHA1(37436c12579fb41d22a1596b495f065959c14a26) )
	ROM_LOAD( "lohtb06.03",  0x30000, 0x10000, CRC(f923183c) SHA1(a6b578191864aefa81e0cad3ba12a2ca491c91cf) )

	ROM_REGION( 0x10000, "samples", ROMREGION_ERASEFF ) // -- no sample roms on bootleg, included with z80 code

	ROM_REGION( 0x0117, "plds", 0 )
	ROM_LOAD( "gal16v8-25qp.ic3", 0x0000, 0x0117, CRC(6acdfafb) SHA1(2ffd6f5a846e49fd2a8c7c7dfc9cf015406a44df) )
ROM_END

ROM_START( lohtb2 ) // program ROMs identical to lohtj content-wise, just half sized ROMs
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "loht-a2.bin",  0x00001, 0x10000, CRC(ccc90e54) SHA1(860da001d9b0782adc25cfc3b453383225253d9e) ) // == tom_c-h0- 1/2
	ROM_LOAD16_BYTE( "loht-a3.bin",  0x20001, 0x10000, CRC(ff8a98de) SHA1(ccb8275241bea81abc01dc36e62557712c1b5a8c) ) // == tom_c-h0- 2/2
	ROM_LOAD16_BYTE( "loht-a10.bin", 0x00000, 0x10000, CRC(3aa06730) SHA1(483b135f8ee0fc54b1953c7c28e909a88aa2fa2e) ) // == tom_c-l0- 1/2
	ROM_LOAD16_BYTE( "loht-a11.bin", 0x20000, 0x10000, CRC(eab1d7bc) SHA1(ec50fe89f05ae46e91b9f2f3d4e4383aa764e71d) ) // == tom_c-l0- 2/2

	ROM_LOAD16_BYTE( "loht-a5.bin",  0x40001, 0x10000, CRC(79e007ec) SHA1(b2e4cc4a47f5f127ba9a1a00eaaf067464314ea0) ) // == tom_c-h3- 1/2
	ROM_RELOAD(                      0xc0001, 0x10000 )
	ROM_LOAD16_BYTE( "loht-a4.bin",  0x60001, 0x10000, CRC(254ea4d5) SHA1(07277bbe2ea6678f0de1f28e40be794880b3faff) ) // == tom_c-h3- 2/2
	ROM_RELOAD(                      0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "loht-a13.bin", 0x40000, 0x10000, CRC(b951346e) SHA1(82fa3c4a09a86b74b98c31aaea5c0629ddff83a0) ) // == tom_c-l3- 1/2
	ROM_RELOAD(                      0xc0000, 0x10000 )
	ROM_LOAD16_BYTE( "loht-a12.bin", 0x60000, 0x10000, CRC(cfb0390d) SHA1(4acc61a51a7ae681bd8d835e2644b44c4d6d7bcb) ) // == tom_c-l3- 2/2
	ROM_RELOAD(                      0xe0000, 0x10000 )

	ROM_REGION( 0x2000, "mcu", 0 ) // MCU running in external mode on daughtercard. Same data as lohtj just padded with 0xff
	ROM_LOAD( "loht-a26.bin",  0x00000, 0x02000, CRC(ac901e17) SHA1(70a73288d594c78ad2aca78ce55a699cb040bede) )

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "loht-a16.bin",  0x00000, 0x10000, CRC(df5ac5ee) SHA1(5b45417ada402047d97dfb6cee6545686ad26e37) ) // == r200 1/2
	ROM_LOAD( "loht-a17.bin",  0x10000, 0x10000, CRC(d7ecf849) SHA1(ab86a88eae21e054d4e8a740a60c7c6c198232d4) ) // == r200 2/2
	ROM_LOAD( "loht-a8.bin",   0x20000, 0x10000, CRC(45220b01) SHA1(83715cf155f91c82067d69f14b3b01ed77777b7d) ) // == r210 1/2
	ROM_LOAD( "loht-a9.bin",   0x30000, 0x10000, CRC(4af9bb3c) SHA1(04f66caae5b3ae985451002293ad8f609a8d9377) )
	ROM_LOAD( "loht-a14.bin",  0x40000, 0x10000, CRC(25b85cfc) SHA1(c7a9962165379193dc6553ed1f977795a79e0f78) ) // == r220 1/2
	ROM_LOAD( "loht-a15.bin",  0x50000, 0x10000, CRC(464d8579) SHA1(b5981f4865ee5439f0e330091927e6d97d29933f) ) // == r220 2/2
	ROM_LOAD( "loht-a6.bin",   0x60000, 0x10000, CRC(763fa4ec) SHA1(2d72b1b41f24ae299fde23869942c0b6bbb82363) ) // == r230 1/2
	ROM_LOAD( "loht-a7.bin",   0x70000, 0x10000, CRC(a73568c7) SHA1(8fe1867256708cc1ed76d1bed5566b1852b47c40) ) // == r230 1/2

	ROM_REGION( 0x040000, "gfx2", 0 ) // same data as loht/lohtj above
	ROM_LOAD( "loht-a19.bin",  0x00000, 0x10000, CRC(3ca3e771) SHA1(be052e01c5429ee89057c9d408794f2c7744047c) )  // tiles #1
	ROM_LOAD( "loht-a20.bin",  0x10000, 0x10000, CRC(7a05ee2f) SHA1(7d1ca5db9a5a85610129e3bc6c640ade036fe7f9) )
	ROM_LOAD( "loht-a18.bin",  0x20000, 0x10000, CRC(79aa2335) SHA1(6b70c79d800a7b755aa7c9a368c4ea74029aaa1e) )
	ROM_LOAD( "loht-a21.bin",  0x30000, 0x10000, CRC(789e8b24) SHA1(e957cd25c3c155ca295ab1aea03d610f91562cfb) )

	ROM_REGION( 0x040000, "gfx3", 0 ) // same data as loht/lohtj above
	ROM_LOAD( "loht-a24.bin",  0x00000, 0x10000, CRC(44626bf6) SHA1(571ef74d42d30a272ff0fb33f830652b4a4bad29) )  // tiles #2
	ROM_LOAD( "loht-a25.bin",  0x10000, 0x10000, CRC(464952cf) SHA1(6b99360b6ba1ed5a72c257f51291f9f7a1ddf363) )
	ROM_LOAD( "loht-a23.bin",  0x20000, 0x10000, CRC(3db9b2c7) SHA1(02a318ffc459c494b7f40827eff5f89b41ac0426) )
	ROM_LOAD( "loht-a22.bin",  0x30000, 0x10000, CRC(f01fe899) SHA1(c5ab967b7af55a757638bcdc9975f4b15064022d) )

	ROM_REGION( 0x10000, "samples", 0 ) // samples
	ROM_LOAD( "loht-a1.bin",   0x00000, 0x10000, CRC(3ed51d1f) SHA1(84f3aa17d640df91387e5f1f5b5971cf8dcd4e17) )
ROM_END

ROM_START( lohtb3 ) // extremely similar to the original. Copyright changed to 1997 and 'IRGM BORP.'
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "9.9", 0x00001, 0x20000, CRC(f1e4ebb7) SHA1(976ec822f57f36bc097deb5d3d86e7c2c1b247ed) )
	ROM_LOAD16_BYTE( "1.1", 0x00000, 0x20000, CRC(b9384e93) SHA1(5b882ecd68fdf7a6307af0e1aca2ab1782911227) )

	ROM_LOAD16_BYTE( "i-10.10",  0x40001, 0x10000, CRC(79e007ec) SHA1(b2e4cc4a47f5f127ba9a1a00eaaf067464314ea0) ) // == tom_c-h3- 1/2
	ROM_RELOAD(                  0xc0001, 0x10000 )
	ROM_LOAD16_BYTE( "i-11.11",  0x60001, 0x10000, CRC(254ea4d5) SHA1(07277bbe2ea6678f0de1f28e40be794880b3faff) ) // == tom_c-h3- 2/2
	ROM_RELOAD(                  0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "i-2.2",    0x40000, 0x10000, CRC(b951346e) SHA1(82fa3c4a09a86b74b98c31aaea5c0629ddff83a0) ) // == tom_c-l3- 1/2
	ROM_RELOAD(                  0xc0000, 0x10000 )
	ROM_LOAD16_BYTE( "i-3.3",    0x60000, 0x10000, CRC(cfb0390d) SHA1(4acc61a51a7ae681bd8d835e2644b44c4d6d7bcb) ) // == tom_c-l3- 2/2
	ROM_RELOAD(                  0xe0000, 0x10000 )

	ROM_REGION( 0x1000, "mcu", 0 )  // i8751 microcontroller
	ROM_LOAD( "c8751h.bin", 0x0000, 0x1000, CRC(9c9545f1) SHA1(ca800ce7467efb877d0fff4c47d72478a991e2a9) ) // unprotected?? == tom_c-pr-b.ic1 from loht (World) set

	ROM_REGION( 0x080000, "sprites", 0 ) // same data as loht/lohtj above, just half sized ROMs
	ROM_LOAD( "i-8.8",   0x00000, 0x10000, CRC(df5ac5ee) SHA1(5b45417ada402047d97dfb6cee6545686ad26e37) ) // == tom_m53.ic53 1/2
	ROM_LOAD( "i-7.7",   0x10000, 0x10000, CRC(d7ecf849) SHA1(ab86a88eae21e054d4e8a740a60c7c6c198232d4) ) // == tom_m53.ic53 2/2
	ROM_LOAD( "i-15.15", 0x20000, 0x10000, CRC(45220b01) SHA1(83715cf155f91c82067d69f14b3b01ed77777b7d) ) // == tom_m51.ic51 1/2
	ROM_LOAD( "i-14.14", 0x30000, 0x10000, CRC(35d1a808) SHA1(9378ff000104ecfb842b3b884197be82c43a01b4) ) // == tom_m51.ic51 2/2
	ROM_LOAD( "i-6.6",   0x40000, 0x10000, CRC(25b85cfc) SHA1(c7a9962165379193dc6553ed1f977795a79e0f78) ) // == tom_m49.ic49 1/2
	ROM_LOAD( "i-5.5",   0x50000, 0x10000, CRC(464d8579) SHA1(b5981f4865ee5439f0e330091927e6d97d29933f) ) // == tom_m49.ic49 2/2
	ROM_LOAD( "i-13.13", 0x60000, 0x10000, CRC(763fa4ec) SHA1(2d72b1b41f24ae299fde23869942c0b6bbb82363) ) // == tom_m47.ic47 1/2
	ROM_LOAD( "i-12.12", 0x70000, 0x10000, CRC(a73568c7) SHA1(8fe1867256708cc1ed76d1bed5566b1852b47c40) ) // == tom_m47.ic47 1/2

	ROM_REGION( 0x040000, "gfx2", 0 ) // same data as loht/lohtj above
	ROM_LOAD( "i-20.20",  0x00000, 0x10000, CRC(3ca3e771) SHA1(be052e01c5429ee89057c9d408794f2c7744047c) )  // tiles #1
	ROM_LOAD( "r-21.21",  0x10000, 0x10000, CRC(7a05ee2f) SHA1(7d1ca5db9a5a85610129e3bc6c640ade036fe7f9) )
	ROM_LOAD( "i-19.19",  0x20000, 0x10000, CRC(79aa2335) SHA1(6b70c79d800a7b755aa7c9a368c4ea74029aaa1e) )
	ROM_LOAD( "i-22.22",  0x30000, 0x10000, CRC(789e8b24) SHA1(e957cd25c3c155ca295ab1aea03d610f91562cfb) )

	ROM_REGION( 0x040000, "gfx3", 0 ) // 99.99% same data as loht/lohtj above - one byte difference in i-23.23
	ROM_LOAD( "r-25.25",  0x00000, 0x10000, CRC(44626bf6) SHA1(571ef74d42d30a272ff0fb33f830652b4a4bad29) )  // tiles #2
	ROM_LOAD( "r-26.26",  0x10000, 0x10000, CRC(464952cf) SHA1(6b99360b6ba1ed5a72c257f51291f9f7a1ddf363) )
	ROM_LOAD( "r-24.24",  0x20000, 0x10000, CRC(3db9b2c7) SHA1(02a318ffc459c494b7f40827eff5f89b41ac0426) )
	ROM_LOAD( "i-23.23",  0x30000, 0x10000, CRC(2e2a085d) SHA1(ed2eed6c30d44b176bd99eb386482b2a19e3b9cb) ) // byte 0x35C7==0x00 vs 0xFF in tom_m24.ic24

	ROM_REGION( 0x10000, "samples", 0 ) // samples
	ROM_LOAD( "i-4.4",   0x00000, 0x10000, CRC(3ed51d1f) SHA1(84f3aa17d640df91387e5f1f5b5971cf8dcd4e17) )
ROM_END

/*  Legend of Hero Tonma for Ervisa Modular System

 ROM Board (51)
 ___________________________________________________________
 |                      ________________  ________________  |
 |                      |               | |               | |
 |                      | LG 506        | | LG 508        | |
 |                      |_______________| |_______________| |
 |           ::::::::   ________________  ________________  |
 |                      |               | |               | |
 | __                   | LG 505        | | LG 507        | |
 | | |                  |_______________| |_______________| |
 | | |     __________   ________________  ________________  |
 | | |     |_GAL16V8|   |               | |               | |
 | | |                  | EMPTY         | | EMPTY         | |
 | | |                  |_______________| |_______________| |
 | | |     __________   ________________  ________________  |
 | | |     |_GAL16V8|   |               | |               | |
 | | |                  | EMPTY         | | EMPTY         | |
 | | |                  |_______________| |_______________| |
 | | |      _________   ________________  ________________  |
 | | |      |74LS138N   |               | |               | |
 | | |                  | EMPTY         | | EMPTY         | |
 | | |                  |_______________| |_______________| |
 | | |________________  ________________                    |
 | |_||               | |               |                   |
 |    | LG 502        | | LG 504        |                   |
 |    |_______________| |_______________|                   |
 |    ________________  ________________                    |
 |    |               | |               |                   |
 |    | LG 501        | | LG 503        |                   |
 |    |_______________| |_______________|                   |
 |    ________________  ________________                    |
 |    |               | |               |     MODULAR       |
 |    | EMPTY         | | EMPTY         |    SYSTEM 2       |
 |    |_______________| |_______________|                   |
 |    ________________  ________________                    |
 |    |               | |               |                   |
 |    | EMPTY         | | EMPTY         |                   |
 |    |_______________| |_______________|                   |
 |    ________________  ________________                    |
 |    |               | |               |                   |
 |    | EMPTY         | | EMPTY         |                   |
 |    |_______________| |_______________|                   |
 |    ________________  ________________                    |
 |    |               | |               |                   |
 |    | EMPTY         | | EMPTY         |                   |
 |    |_______________| |_______________|                   |
 |__________________________________________________________|

 ROM Board (8)
 __________________________________________________________________________________
 |           :::::::: <- Jumpers                                                   |
 |          ________________  ________________  ________________  ________________ |
 | _______  |               | |               | |               | |               ||
 | 74LS175N | EMPTY         | | EMPTY         | | EMPTY         | | EMPTY         ||
 |          |_______________| |_______________| |_______________| |_______________||
 | _______  ________________  ________________  ________________  ________________ |
 | 74LS175N |               | |               | |               | |               ||
 |          | EMPTY         | | EMPTY         | | EMPTY         | | EMPTY         ||
 | _______  |_______________| |_______________| |_______________| |_______________||
 | 74LS175N ________________  ________________  ________________  ________________ |
 |          |               | |               | |               | |               ||
 | _______  | EMPTY         | | EMPTY         | | EMPTY         | | EMPTY         ||
 | 74LS175N |_______________| |_______________| |_______________| |_______________||
 |          ________________  ________________  ________________  ________________ |
 | _______  |               | |               | |               | |               ||
 | 74LS175N | EMPTY         | | EMPTY         | | EMPTY         | | EMPTY         ||
 |          |_______________| |_______________| |_______________| |_______________||
 |          ________________  ________________  ________________  ________________ |
 | _______  |               | |               | |               | |               ||
 | 74LS175N | EMPTY         | | EMPTY         | | EMPTY         | | EMPTY         ||
 |          |_______________| |_______________| |_______________| |_______________||
 |          ________________  ________________  ________________  ________________ |
 | _______  |               | |               | |               | |               ||
 | 74LS175N | LG 802        | | LG 804        | | LG 806        | | LG 808        ||
 |          |_______________| |_______________| |_______________| |_______________||
 | _______  ________________  ________________  ________________  ________________ |
 | 74LS175N |               | |               | |               | |               ||
 |          | LG 801        | | LG 803        | | LG 805        | | LG 807        ||
 |          |_______________| |_______________| |_______________| |_______________||
 |  _____________________________      _______  _________________________________  |
 |__|                            |____74LS138N_|                                |__|
    |____________________________|             |________________________________|

 CPU Board (6)                                       Board 21/1
 _____________________________________________       ______________________________________________
 |             _______________              __|_     |                                             |
 |             |              |             |   |    |                                             |
 |             | KM62245AP-10 |             |   |    |  ________   _______  ________               |
 |             |______________|   _______   |   |    |  |_EMPTY_| |_EMPTY_| |_EMPTY_|              |
 |  _______   ________________    74LS138N  |   |    |                                             |
 | 74LS367AN  |               |             |   |    |                                             |
 |            | LG 606        |             |   |    |                                             |
 |            |_______________|             |   |    |  ________   _______  ________               |
 |            ________________    _______   |   |    |  |74S74N_| |74LS393N |74LS393N              |
 |  _______   |               |   74LS245N  |   |    |                                             |
 | 74LS367AN  | LG 605        |             |   |    |                                             |
 |            |_______________|             |   |    |                                             |
 |            ________________              |   |    |                                             |
 |  _______   |               |   _______   |   |    |  74LS7273N |82S129_| |82S129_|              |
 | 74LS138N   | LG 604        |   74LS374P  |   |    |               209       204                 |
 |            |_______________|             |   |    |                                             |
 |             _______________              |   |    |                                             |
 |             |              |   _______   |___|    |  ________   _______  ________               |
 |  _______    | KM62245AP-10 |   74LS245N    |      |  74LS74AN    XTAL    74LS367AN              |
 |  GAL16V8    |______________|               |      |            28.00000 MHz                     |
 |    686     ________________                |      |                                             |
 |  _______   |               |   _______     |      |  ________   _______  ________               |
 | 74LS174AN  | LG 603        |   74LS374P    |      |  74LS732B1 74LS368AN AM2148-55DC          __|_
 |            |_______________|               |      |                                           |   |
 |  _______   ________________                |      |                                           |   |
 |  GAL16V8   |               |               |      |  ________   _______  ________             |   |
 |   8638     | LG 602        |   _______     |      |  74LS157N  74S112AN  AM2148-55DC          |   |
 |  _______   |_______________|   74LS138N    |      |                                           |   |
 |  GAL16V8   ________________                |      |                                           |   |
 |   8600     |               |               |      |  ________   _______  ________             |   |
 |  _______   | LG 601        |               |      |  74LS148N  74LS368AN AM2148-55DC          |   |
 |  74LS373N  |_______________|   _______     |      |                                           |   |
 |___________________________     74LS32N     |      |                                           |   |
 ||_________________ _______ |                |      |  ________   _______  ________   ________  |   |
 |||                |74LS737N|    _______     |      |  74LS298P  74LS298P  74LS298P   74LS174N  |   |
 ||| NEC V30        |_______ |    74LS20N     |      |                                           |   |
 |||________________|74LS737N|                |      |                                           |   |
 ||__________________________|                |      |  ________   _______  ________   ________  |   |
 |  _______  ______  _______      _______     |      |  74LS245N  74LS245N  |74LS08P   74LS174N  |   |
 |  |74S74N   XTAL  74LS368AN     74LS132N    |      |                                           |___|
 |__________20.000MHz_________________________|      |_____________________________________________|

 Board 5                                             Sound Board 1/3
 _____________________________________________       ______________________________________________
 |                                            |      |                                             |
 | __________  ________ ________ ________     |      |  ________  ________  ________               |
 | |TO SUB 51| 74LS299N 74LS169N UM2149-1     |      |  74LS107AN |74LS3Z_| |_EMPTY_|              |
 | |_________| ________ ________ ________     |      |                                             |
 |             74LS169J 74LS169N UM2149-1     |      |  ________  __________________               |
 |                                            |      | 74LS368AB1 |                 |              |
 | __________  ________ ________ ________     |      |            | EMPTY           |              |
 | |TO SUB 51| 74LS158N 74LS169N |82S129N <- 502     |  ________  |_________________|              |
 | |_________| ________ ________ ________     |      |  74LS74AN  __________________               |
 |             74LS299N 74LS169N 74LS244N     |      |            |  Z8400BB1       |              |
 |                                            |      |  ________  |  Z80 B CPU      |  ________    |
 | __________  ________ ________ ________     |      | 74HCT157E  |_________________|  |CA324E_|   |
 | |TO SUB 51| 74LS299N 74LS169N 74LS244N     |      |            ________________                 |
 | |_________| ________ ________ ________     |      |  ________  |               |             __ |
 |             |74LS20N UM2149-1 74LS298P     |    105->N82S123N  | LG 1          |             |D||
 |                                            |      |  ________  |_______________|             |I||
 | __________  ________ ________ ________     |      | 74HCT157E  ________________              |P||     SOUND SUB
 | |TO SUB 51| 74LS299N UM2149-1 74LS298P     |      | __________ |               | _____ _____ |S|| _________________
 | |_________|                                |      | |         || SRM2064C-15   |Y3014B Y3014B|-|| | ______          |
 |   ________  ________ ________ ________  __ |      | | SOUND   ||_______________|             |D|| | LM358N     ___  |
 |   74LS273P  |74LS00N 74LS86B1 74LS244N  | ||      | |  SUB    |                              |I|| | .......... |  | |
 |                                         | ||      | |         |                              |P|| |            |OKI |
_|_  ________  ________ ________ ________  |T||    __|_|_________|__________________            |S|| | .......... |M5205
|  | 74LS08B1  74LS158N 74LS74AN |74LS20N  |O||    |   |          | YAMAHA          |           |_|| | ________   |__| |
|  |                                       | ||    |   |          | YM2203C         |              | | 74LS377B1       |
|  | ________  ________ ________ ________  |S||    |   |          |_________________|              | |_________________|
IC64->PAL16R6A 74LS393N 74LS368AN 74LS377N |U||    |   |                                           |
|  |                                       |B||    |   | _______  ________                         |
|  | ________  ________ ________ ________  | ||    |   | |EMPTY_| 74LS74AN                         |
|  | 74LS138N  74LS283N AM2148-55DC EMPTY  |5||    |   |                                           |
|  |                                       |1||    |   |                                           |
|  | ________  ________ ________ ________  | ||    |   |          __________________    _____      |
|  | 74LS175P  74LS283N AM2148-55DC 74LS273P ||    |   | _______  | YAMAHA          |  TDA2003     |
|  |                                       | ||    |   | |EMPTY|  | YM2203C         |              |
|  | ________  ________ ________ ________  |_||    |   |          |_________________|              |
|  | 74LS298P  74LS157N 74LS157N 74LS273P     |    |   |                                           |
|  |                                          |    |   |                                           |
|  | ________  ________ ________ ________     |    |   |                                           |
|  | 74LS158N  74LS169N 74LS169N 74LS245P     |    |   |                                           |
|__|                                          |    |___|        __________________________         |
 |____________________________________________|      |__________| |_|_|_|_|_|_|_|_|_|_|_| |________|
                                                                          PRE-JAMMA
 Board 7/4
 __________________________________________________________
 | _________  _________ __________  _________  _________ __|_
 | 74LS163AP| 74LS163AP||_GAL20V8_| |74S174N_| |74S189N_||   |
 |                         7636                          |   |
 | _________  _________  _________  _________  _________ |   |
 | |74LS157N| |74LS288N| |74LS273P| 74LS290B1| |74S189N_||   |
 |                                                       |   |
 | _________  _________  _________  _________  _________ |   |
 | |74LS393N| 74LS283N_|74HCTLS373N 74LS298B1| |74S189N_||   |
 |                                                       |   |
 | _________  __________  _____________  _________       |   |
 | |74LS157N| |_GAL16V8_| |KM62256AP-10| |74S174N_|      |   |
 |              7536      |____________| _________       |   |
 | _________  __________  _____________  |74S174N_|      |   |
 | |74LS283N| |74LS245P_| |KM62256AP-10| __________      |   |
 | _________  __________  |____________| |74LS245P_|     |   |
 | |74LS20B1| |74LS245P_|                                |   |
 | _________  __________    __________   _________       |   |
 | |74LS20B1| |74LS374N_|   |74LS273P_|  |74LS74AP|      |   |
 | _________  __________    __________   _________       |___|
 | |74LS04N_| |_GAL16V8_|   |74LS374N_|  |74LS157N|        |
 |               7336                                      |
 | _________  __________    __________   _________         |
 | |74LS00N_| |_GAL16V8_|   |74LS273P_|  74LS367AN       __|_
 |               7236                                    |   |
 | _________  __________    __________   _________       |   |
 | |74LS08B1| |74LS273P_|   |74LS374N_|  |GAL16V8_|      |   |
 |                                          7436         |   |
 | _________  __________    __________   _________       |   |
 | |74LS74AP| |74LS273P_|   74LS367AB1|  74HCTLS373N     |   |
 |                                                       |   |
 | _________  __________    __________   _________       |   |
 | |74LS32P_| |74LS374N_|   74LS367AB1| GAL20V8-25LP     |   |
 |                                          7136         |   |
 | _________  _________  _________  _________  _________ |   |
 | 74LS139AN  |_74S74N_| |74LS157N| |74LS20B1| 74HCTLS373N   |
 | _________  _________  _________  _________  _________ |   |
 | |74LS32N_| |74LS157N|74HCTLS597N 74HCTLS597N 74HCTLS597N  |
 | _________  _________  _________  _________  _________ |   |
 | |74LS748P| |74LS86N_|74HCTLS597N 74HCTLS597N 74HCTLS597N  |
 | _________  _________  _________  _________  _________ |   |
 | |74LS748P| |74LS86N_|74HCTLS597N 74HCTLS597N 74HCTLS597N  |
 |            _________  _________  _________  _________ |   |
 |            |74LS377N|74HCTLS597N 74HCTLS597N 74HCTLS597N  |
 |            _________  _________  _________  _________ |___|
 |            |74LS273P| |74LS169N| |74LS169N| |74LS169N|  |
 |_________________________________________________________|

*/
ROM_START( loht_ms ) // really similar to lohtb, even if it runs on 'Modular Hardware'
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "6_lg_604.ic17", 0x00001, 0x10000, CRC(7e84b6ce) SHA1(a9a54f045917a191b6d6bbe061fc7a3344efb3bb) )
	ROM_LOAD16_BYTE( "6_lg_605.ic20", 0x20001, 0x10000, CRC(abdcd211) SHA1(6ad79c4ef14d908032e4dbded7696a66fe5d31da) )
	ROM_LOAD16_BYTE( "6_lg_601.ic8",  0x00000, 0x10000, CRC(7e080cb8) SHA1(598bcd8ecebe1922735b61cc1305e8839d9d054e) )
	ROM_LOAD16_BYTE( "6_lg_602.ic11", 0x20000, 0x10000, CRC(150d1178) SHA1(71df3c9a49eb74cf40790e9b4cf7c6260fbd07d6) )
	ROM_LOAD16_BYTE( "6_lg_606.ic26", 0x40001, 0x20000, CRC(714778b5) SHA1(e2eaa35d6b5fa5df5163fe0d7b45fa66667f9947) )
	ROM_RELOAD(                       0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "6_lg_603.ic25", 0x40000, 0x20000, CRC(2f049b03) SHA1(21047cb10912b1fc23795673af3ea7de249328b7) )
	ROM_RELOAD(                       0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 ) // Sound CPU program (Z80) + Samples
	ROM_LOAD( "1_lg_1.ic12",  0x00000, 0x10000, CRC(7aa26b54) SHA1(01df7ea6443bbc775d5592edc32816853c857189) )

	ROM_REGION( 0x080000, "sprites", 0 ) // Sprites
	ROM_LOAD( "5_lg_501.ic5",   0x00000, 0x10000, CRC(df5ac5ee) SHA1(5b45417ada402047d97dfb6cee6545686ad26e37) )
	ROM_LOAD( "5_lg_503.ic14",  0x20000, 0x10000, CRC(45220b01) SHA1(83715cf155f91c82067d69f14b3b01ed77777b7d) )
	ROM_LOAD( "5_lg_505.ic20",  0x40000, 0x10000, CRC(25b85cfc) SHA1(c7a9962165379193dc6553ed1f977795a79e0f78) )
	ROM_LOAD( "5_lg_507.ic26",  0x60000, 0x10000, CRC(763fa4ec) SHA1(2d72b1b41f24ae299fde23869942c0b6bbb82363) )
	ROM_LOAD( "5_lg_502.ic6",   0x10000, 0x10000, CRC(d7ecf849) SHA1(ab86a88eae21e054d4e8a740a60c7c6c198232d4) )
	ROM_LOAD( "5_lg_504.ic15",  0x30000, 0x10000, CRC(35d1a808) SHA1(9378ff000104ecfb842b3b884197be82c43a01b4) )
	ROM_LOAD( "5_lg_506.ic21",  0x50000, 0x10000, CRC(464d8579) SHA1(b5981f4865ee5439f0e330091927e6d97d29933f) )
	ROM_LOAD( "5_lg_508.ic27",  0x70000, 0x10000, CRC(a73568c7) SHA1(8fe1867256708cc1ed76d1bed5566b1852b47c40) )

	ROM_REGION( 0x040000, "gfx2", ROMREGION_INVERT ) // tiles #1
	ROM_LOAD( "8_lg_801.ic15",  0x00000, 0x10000, CRC(359f17d4) SHA1(2875ba48395e7faa1a58404475be936dcca45ed1) )
	ROM_LOAD( "8_lg_803.ic22",  0x10000, 0x10000, CRC(73391e8a) SHA1(53ca89b8a10895f817ecdb9fa5eef462edb94ae6) )
	ROM_LOAD( "8_lg_805.ic30",  0x20000, 0x10000, CRC(7096d390) SHA1(f4a16bf8aef7a1a65619ab022cbdb67d2f191888) )
	ROM_LOAD( "8_lg_807.ic37",  0x30000, 0x10000, CRC(1c113901) SHA1(3d4ce2ac6cdad0e1b0a21ffb062f9c92700adcf4) )

	ROM_REGION( 0x040000, "gfx3", ROMREGION_INVERT ) // tiles #2
	ROM_LOAD( "8_lg_802.ic14",  0x00000, 0x10000, CRC(4d5e9b53) SHA1(3e3977bab7a66ed0171afcd555d181960e338749) )
	ROM_LOAD( "8_lg_804.ic21",  0x10000, 0x10000, CRC(4f75a26a) SHA1(79c09a1ad3a6f9cfbd07cb527bbd89d2478ce582) )
	ROM_LOAD( "8_lg_806.ic29",  0x20000, 0x10000, CRC(34854262) SHA1(37436c12579fb41d22a1596b495f065959c14a26) )
	ROM_LOAD( "8_lg_808.ic36",  0x30000, 0x10000, CRC(f923183c) SHA1(a6b578191864aefa81e0cad3ba12a2ca491c91cf) )

	ROM_REGION( 0x10000, "samples", ROMREGION_ERASEFF ) // -- no sample roms on bootleg, included with z80 code

	ROM_REGION( 0x320, "prom", ROMREGION_ERASEFF )
	ROM_LOAD( "51_502_82s129n.ic10",      0x000, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) )
	ROM_LOAD( "21_204_82s129an.ic4",      0x100, 0x100, CRC(74470450) SHA1(40b0e0991090733f8190ad7efcb500bd109c2a7e) )
	ROM_LOAD( "21_209_82s129an.ic12",     0x200, 0x100, CRC(7922a7ab) SHA1(e7ec2b1fc61bd7d2cf921bfc50d975f91b938e8a) )
	ROM_LOAD( "1_105_82s123n.ic20",       0x300, 0x020, CRC(14d72781) SHA1(372dc021d8aaf4aa6fd46e69a3d8f1c68113426f) )

	ROM_REGION( 0x104, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "51_p0503_pal16r6.ic46",       0x000, 0x104, CRC(07eb86d2) SHA1(482eb325df5bc60353bac85412cf45429cd03c6d) )
	// these were read protected
	ROM_LOAD( "5_5136_gal16v8-25lp.ic9",   0x0, 0x1, NO_DUMP )
	ROM_LOAD( "5_5236_gal16v8-25lp.ic8",   0x0, 0x1, NO_DUMP )
	ROM_LOAD( "6_686_gal16v8.ic13",        0x0, 0x1, NO_DUMP )
	ROM_LOAD( "6_8638_gal16v8.ic7",        0x0, 0x1, NO_DUMP )
	ROM_LOAD( "6_subcpu_8600_gal16v8.ic3", 0x0, 0x1, NO_DUMP )
	ROM_LOAD( "7_7136_gal20v8-25lp.ic7",   0x0, 0x1, NO_DUMP )
	ROM_LOAD( "7_7236_gal20v8-25lp.ic54",  0x0, 0x1, NO_DUMP )
	ROM_LOAD( "7_7336_gal16v8.ic55",       0x0, 0x1, NO_DUMP )
	ROM_LOAD( "7_7436_gal16v8.ic9",        0x0, 0x1, NO_DUMP )
	ROM_LOAD( "7_7536_gal16v8.ic59",       0x0, 0x1, NO_DUMP )
	ROM_LOAD( "7_7636_gal20v8-25lp.ic44",  0x0, 0x1, NO_DUMP )
ROM_END


ROM_START( xmultiplm72 )
	ROM_REGION( 0x100000, "maincpu", 0 ) // M72-B-C top board
	ROM_LOAD16_BYTE( "xm_c-h3-.ic43", 0x00001, 0x20000, CRC(20685021) SHA1(92f4216320bf525045223b9454fb5bb224c536d8) )
	ROM_LOAD16_BYTE( "xm_c-l3-.ic34", 0x00000, 0x20000, CRC(93fdd200) SHA1(dd4244ba0ce6c621136b0648374179da44363c01) )
	ROM_LOAD16_BYTE( "xm_c-h0-.ic40", 0x40001, 0x10000, CRC(9438dd8a) SHA1(dc85789c47d31a96300b4236dc43f065e1b01e4a) )
	ROM_RELOAD(                       0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "xm_c-l0-.ic37", 0x40000, 0x10000, CRC(06a9e213) SHA1(9831c110814642703d6e71d49848d854095b7d3a) )
	ROM_RELOAD(                       0xe0000, 0x10000 )

	ROM_REGION( 0x1000, "mcu", 0 )  // i8751 microcontroller
	ROM_LOAD( "xm_c-pr-.ic1", 0x0000, 0x1000, CRC(c8ceb3cd) SHA1(e5d20a3a9d7f0919604543c97643a03434d80130) ) // i8751 MCU labeled  XM C-PR-

	ROM_REGION( 0x100000, "sprites", 0 ) // mask ROMs
	ROM_LOAD( "t44.00.ic53", 0x00000, 0x20000, CRC(db45186e) SHA1(8c8edeb4b7e6b0516f2597823dc27eba9c5d9528) )  // sprites
	ROM_LOAD( "t45.01.ic52", 0x20000, 0x20000, CRC(4d0764d4) SHA1(4942333336a110b033f16ac1afa06ffef7b2dad6) )
	ROM_LOAD( "t46.10.ic51", 0x40000, 0x20000, CRC(f0c465a4) SHA1(69c107c860d4e8736431fd86b6821b70a8367eb3) )
	ROM_LOAD( "t47.11.ic50", 0x60000, 0x20000, CRC(1263b24b) SHA1(0445a5381df3a868bed6967c8e5de7169e4be6a3) )
	ROM_LOAD( "t48.20.ic49", 0x80000, 0x20000, CRC(4129944f) SHA1(988b072032d1667c3ac0731fada32fb6978505dc) )
	ROM_LOAD( "t49.21.ic48", 0xa0000, 0x20000, CRC(2346e6f9) SHA1(b3de017dd0353e04d279f57e151c47f5fcc70e9c) )
	ROM_LOAD( "t50.30.ic47", 0xc0000, 0x20000, CRC(e322543e) SHA1(b4c3a7f202d81485d5f0a7b7668ee89fc1edb215) )
	ROM_LOAD( "t51.31.ic46", 0xe0000, 0x20000, CRC(229bf7b1) SHA1(ae42c7efbb6278dd3fa56842361138391f2d49ca) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // mask ROMs
	ROM_LOAD( "t53.a0.ic21", 0x00000, 0x20000, CRC(1a082494) SHA1(63a3a84a262833d2cafab41e35df8f10a5e317b1) )  // tiles #1
	ROM_LOAD( "t54.a1.ic22", 0x20000, 0x20000, CRC(076c16c5) SHA1(4be858806b916953d59aceee550e721eaf3996a6) )
	ROM_LOAD( "t55.a2.ic20", 0x40000, 0x20000, CRC(25d877a5) SHA1(48c948bf714c432f534c098123c8f50d5561756f) )
	ROM_LOAD( "t56.a3.ic23", 0x60000, 0x20000, CRC(5b1213f5) SHA1(87782aa0bd04d4378c4ba78b63028ae2709da2f1) )

	ROM_REGION( 0x080000, "gfx3", 0 ) // mask ROMs
	ROM_LOAD( "t57.b0.ic26", 0x00000, 0x20000, CRC(0a84e0c7) SHA1(67ad181a7d2c431cb4bf45955e09754549a03576) )  // tiles #2
	ROM_LOAD( "t58.b1.ic27", 0x20000, 0x20000, CRC(a874121d) SHA1(1351d5901d55059c6472a4588a2e560396903861) )
	ROM_LOAD( "t59.b2.ic25", 0x40000, 0x20000, CRC(69deb990) SHA1(1eed3183efbe576376661b45152a0a21240ecfc8) )
	ROM_LOAD( "t60.b3.ic24", 0x60000, 0x20000, CRC(14c69f99) SHA1(4bea72f8bd421ef3ca559363f7473ce2e7038699) )

	ROM_REGION( 0x20000, "samples", 0 ) // mask ROM
	ROM_LOAD( "t52.v0.ic44", 0x00000, 0x20000, CRC(2db1bd80) SHA1(657006d0642ec7fb949bb52821d78fe51a599415) )  // samples

	ROM_REGION( 0x0200, "proms", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0600, "pals", 0 )
	ROM_LOAD( "m72_a-3d-.ic11", 0x0000, 0x0117, CRC(8a3732ff) SHA1(6e3039e7dc424cbef7156312fa1ce67d7b082d30) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_a-4d-.ic19", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // PAL16L8 - bruteforced
	ROM_LOAD( "xm_c-3f-.ic13",  0x0400, 0x0117, CRC(f43e91e4) SHA1(37dd9b9436ad57232da7fd57040311ac140c1841) ) // PAL16L8 - bruteforced - located on M72-C-A top board
ROM_END


ROM_START( dbreedm72 )
	ROM_REGION( 0x100000, "maincpu", 0 ) // M72-C top board
	ROM_LOAD16_BYTE( "db_c-h3-b.ic43", 0x00001, 0x20000, CRC(4bf3063c) SHA1(3f970c9ece2ac700738e217e0b31b3aba2848ab2) ) // need to verify label, with MCU DB C-PR- below shows ROM NG  1  2
	ROM_LOAD16_BYTE( "db_c-l3-b.ic34", 0x00000, 0x20000, CRC(e4b89b79) SHA1(c312925940633e60fb5d0f05044c6e73e4f7fd54) ) // need to verify label
	ROM_LOAD16_BYTE( "db_c-h0-.ic40",  0x60001, 0x10000, CRC(5aa79fb2) SHA1(b7b862699ddccf90cf18d3822703078668aa1dc7) )
	ROM_RELOAD(                        0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "db_c-l0-.ic37",  0x60000, 0x10000, CRC(ed0f5e06) SHA1(9030840b15e83c18d59c884ed08c93c05fa70c5b) )
	ROM_RELOAD(                        0xe0000, 0x10000 )

	ROM_REGION( 0x1000, "mcu", 0 )  // i8751 microcontroller
	ROM_LOAD( "db_c-pr-b.ic1", 0x0000, 0x1000, NO_DUMP ) // Requires different currently undumped MCU code - i8751 MCU labeled  DB C-PR-B??

	ROM_REGION( 0x080000, "sprites", 0 ) // mask ROMs
	ROM_LOAD( "db_k800m.00.ic53", 0x00000, 0x20000, CRC(c027a8cf) SHA1(534dc416b8f5587168c7f644d3f9438c8a190491) )   // sprites
	ROM_LOAD( "db_k801m.10.ic51", 0x20000, 0x20000, CRC(093faf33) SHA1(2704f644cdce87daf975984f143b1d55ba731c3f) )
	ROM_LOAD( "db_k802m.20.ic49", 0x40000, 0x20000, CRC(055b4c59) SHA1(71315dd7476612f138cb64b905648791d44eb7da) )
	ROM_LOAD( "db_k803m.30.ic47", 0x60000, 0x20000, CRC(8ed63922) SHA1(51daa8a23e637f6b4394598ff4a1d26f65b59c8b) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // same roms are duplicated at a0-a3 and b0-b3, confirmed
	ROM_LOAD( "db_k804m.a0.ic21", 0x00000, 0x20000, CRC(4c83e92e) SHA1(6dade027435c48ab48bd4516d16a9961d4dd6fad) )   // tiles #1
	ROM_LOAD( "db_k805m.a1.ic22", 0x20000, 0x20000, CRC(835ef268) SHA1(89d0bb15201440dffad3ef745970f95505d7ab03) )
	ROM_LOAD( "db_k806m.a2.ic20", 0x40000, 0x20000, CRC(5117f114) SHA1(a401a3e638209b32d4101a5c2e2a8b4612eaa21b) )
	ROM_LOAD( "db_k807m.a3.ic23", 0x60000, 0x20000, CRC(8eb0c978) SHA1(7fc55bbe4d0923db88492bb7160a89de34e11cd6) )

	ROM_REGION( 0x080000, "gfx3", 0 ) // mask ROMs
	ROM_LOAD( "db_k804m.b0.ic26", 0x00000, 0x20000, CRC(4c83e92e) SHA1(6dade027435c48ab48bd4516d16a9961d4dd6fad) )   // tiles #2
	ROM_LOAD( "db_k805m.b1.ic27", 0x20000, 0x20000, CRC(835ef268) SHA1(89d0bb15201440dffad3ef745970f95505d7ab03) )
	ROM_LOAD( "db_k806m.b2.ic25", 0x40000, 0x20000, CRC(5117f114) SHA1(a401a3e638209b32d4101a5c2e2a8b4612eaa21b) )
	ROM_LOAD( "db_k807m.b3.ic24", 0x60000, 0x20000, CRC(8eb0c978) SHA1(7fc55bbe4d0923db88492bb7160a89de34e11cd6) )

	ROM_REGION( 0x20000, "samples", 0 )
	ROM_LOAD( "db_c-v0.ic44",  0x00000, 0x20000, CRC(312f7282) SHA1(742d56980b4618180e9a0e02051c5aec4d5cdae4) )  // samples

	ROM_REGION( 0x0200, "proms", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0600, "pals", 0 )
	ROM_LOAD( "m72_a-3d-.ic11", 0x0000, 0x0117, CRC(8a3732ff) SHA1(6e3039e7dc424cbef7156312fa1ce67d7b082d30) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_a-4d-.ic19", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // PAL16L8 - bruteforced
	ROM_LOAD( "db-c-3f-.ic13",  0x0400, 0x0117, CRC(7184d421) SHA1(3120ac2ca32e89d2f2abdb278a8c8cfa298aad75) ) // PAL16L8 - bruteforced - located on M72-C-A top board
ROM_END

ROM_START( dbreedjm72 )
	ROM_REGION( 0x100000, "maincpu", 0 ) // M72-C top board
	ROM_LOAD16_BYTE( "db_c-h3-.ic43", 0x00001, 0x20000, CRC(43425d67) SHA1(a87339bf299f7e84b9a181f3827278f64a6a29ea) )
	ROM_LOAD16_BYTE( "db_c-l3-.ic34", 0x00000, 0x20000, CRC(9c1abc85) SHA1(6c73fbec12a7795e327381d886a87bca09a7dff0) )
	ROM_LOAD16_BYTE( "db_c-h0-.ic40", 0x60001, 0x10000, CRC(5aa79fb2) SHA1(b7b862699ddccf90cf18d3822703078668aa1dc7) )
	ROM_RELOAD(                       0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "db_c-l0-.ic37", 0x60000, 0x10000, CRC(ed0f5e06) SHA1(9030840b15e83c18d59c884ed08c93c05fa70c5b) )
	ROM_RELOAD(                       0xe0000, 0x10000 )

	ROM_REGION( 0x1000, "mcu", 0 )  // i8751 microcontroller
	ROM_LOAD( "db_c-pr-.ic1", 0x0000, 0x1000, CRC(8bf2910c) SHA1(65842c928626077f613e0ab56074e83301a64a2e) ) // i8751 MCU labeled  DB C-PR-

	ROM_REGION( 0x080000, "sprites", 0 ) // mask ROMs
	ROM_LOAD( "db_k800m.00.ic53", 0x00000, 0x20000, CRC(c027a8cf) SHA1(534dc416b8f5587168c7f644d3f9438c8a190491) )   // sprites
	ROM_LOAD( "db_k801m.10.ic51", 0x20000, 0x20000, CRC(093faf33) SHA1(2704f644cdce87daf975984f143b1d55ba731c3f) )
	ROM_LOAD( "db_k802m.20.ic49", 0x40000, 0x20000, CRC(055b4c59) SHA1(71315dd7476612f138cb64b905648791d44eb7da) )
	ROM_LOAD( "db_k803m.30.ic47", 0x60000, 0x20000, CRC(8ed63922) SHA1(51daa8a23e637f6b4394598ff4a1d26f65b59c8b) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // same roms are duplicated at a0-a3 and b0-b3, confirmed
	ROM_LOAD( "db_k804m.a0.ic21", 0x00000, 0x20000, CRC(4c83e92e) SHA1(6dade027435c48ab48bd4516d16a9961d4dd6fad) )   // tiles #1
	ROM_LOAD( "db_k805m.a1.ic22", 0x20000, 0x20000, CRC(835ef268) SHA1(89d0bb15201440dffad3ef745970f95505d7ab03) )
	ROM_LOAD( "db_k806m.a2.ic20", 0x40000, 0x20000, CRC(5117f114) SHA1(a401a3e638209b32d4101a5c2e2a8b4612eaa21b) )
	ROM_LOAD( "db_k807m.a3.ic23", 0x60000, 0x20000, CRC(8eb0c978) SHA1(7fc55bbe4d0923db88492bb7160a89de34e11cd6) )

	ROM_REGION( 0x080000, "gfx3", 0 ) // mask ROMs
	ROM_LOAD( "db_k804m.b0.ic26", 0x00000, 0x20000, CRC(4c83e92e) SHA1(6dade027435c48ab48bd4516d16a9961d4dd6fad) )   // tiles #2
	ROM_LOAD( "db_k805m.b1.ic27", 0x20000, 0x20000, CRC(835ef268) SHA1(89d0bb15201440dffad3ef745970f95505d7ab03) )
	ROM_LOAD( "db_k806m.b2.ic25", 0x40000, 0x20000, CRC(5117f114) SHA1(a401a3e638209b32d4101a5c2e2a8b4612eaa21b) )
	ROM_LOAD( "db_k807m.b3.ic24", 0x60000, 0x20000, CRC(8eb0c978) SHA1(7fc55bbe4d0923db88492bb7160a89de34e11cd6) )

	ROM_REGION( 0x20000, "samples", 0 )
	ROM_LOAD( "db_c-v0.ic44",  0x00000, 0x20000, CRC(312f7282) SHA1(742d56980b4618180e9a0e02051c5aec4d5cdae4) )  // samples

	ROM_REGION( 0x0200, "proms", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0600, "pals", 0 )
	ROM_LOAD( "m72_a-3d-.ic11", 0x0000, 0x0117, CRC(8a3732ff) SHA1(6e3039e7dc424cbef7156312fa1ce67d7b082d30) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_a-4d-.ic19", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // PAL16L8 - bruteforced
	ROM_LOAD( "db-c-3f-.ic13",  0x0400, 0x0117, CRC(7184d421) SHA1(3120ac2ca32e89d2f2abdb278a8c8cfa298aad75) ) // PAL16L8 - bruteforced - located on M72-C-A top board
ROM_END


ROM_START( dkgensanm72 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ge72_h0-.ic40", 0x00001, 0x20000, CRC(a0ad992c) SHA1(6de4105d8454c4e4e62762fdd7e22829acc2442b) )
	ROM_LOAD16_BYTE( "ge72_l0-.ic37", 0x00000, 0x20000, CRC(996396f0) SHA1(1a2501ba46bcbc607f772765e8614bc442154a18) )
	ROM_LOAD16_BYTE( "ge72_h3-.ic43", 0x60001, 0x10000, CRC(d8b86005) SHA1(dd626cfe50a823066c54cc24d9fdaaf03d61d1e7) )
	ROM_RELOAD(                       0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "ge72_l3-.ic34", 0x60000, 0x10000, CRC(23d303a5) SHA1(b62010f34d71afb590deae458493454f9af38f7c) )
	ROM_RELOAD(                       0xe0000, 0x10000 )

	ROM_REGION( 0x1000, "mcu", 0 )  // i8751 microcontroller
	ROM_LOAD( "dkgenm72_i8751.ic1", 0x0000, 0x1000, NO_DUMP ) // read protected

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "hh_00.ic53",    0x00000, 0x20000, CRC(ec5127ef) SHA1(014ac8ad7b19cd9b475b72a0f42a4991119501c4) )  // sprites
	ROM_LOAD( "hh_10.ic51",    0x20000, 0x20000, CRC(def65294) SHA1(23f5d99fa9f604fde37cb52113bff233d9be1d25) )
	ROM_LOAD( "hh_20.ic49",    0x40000, 0x20000, CRC(bb0d6ad4) SHA1(4ab617fadfc32efad90ed7f0555513f167b0c43a) )
	ROM_LOAD( "hh_30.ic47",    0x60000, 0x20000, CRC(4351044e) SHA1(0d3ce3f4f1473fd997e70de91e7b5b5a5ec60ad4) )

	ROM_REGION( 0x040000, "gfx2", 0 )
	ROM_LOAD( "ge72b-a0.ic21", 0x00000, 0x10000, CRC(f5f56b2a) SHA1(4ef6602052fa70e765d6d7747e672b7108b44f59) )  // tiles #1
	ROM_LOAD( "ge72-a1.ic22",  0x10000, 0x10000, CRC(d194ea08) SHA1(0270897049cd256472df42f3dda856ee707535cd) )
	ROM_LOAD( "ge72-a2.ic20",  0x20000, 0x10000, CRC(2b06bcc3) SHA1(36378a4a69f3c3da96d2dc8df48916af8de50009) )
	ROM_LOAD( "ge72-a3.ic23",  0x30000, 0x10000, CRC(94b96bfa) SHA1(33c1e9045e7a984097f3fe4954b20d954cffbafa) )

	ROM_REGION( 0x040000, "gfx3", 0 )
	ROM_LOAD( "ge72-b0.ic26",  0x00000, 0x10000, CRC(208796b3) SHA1(38b90732c8d5c77ee84053364a8a7e3daaaabe66) )  // tiles #2
	ROM_LOAD( "ge72-b1.ic27",  0x10000, 0x10000, CRC(b4a7f490) SHA1(851b40650fc8920b49f43f9cc6f19e845a25e945) )
	ROM_LOAD( "ge72b-b2.ic25", 0x20000, 0x10000, CRC(34fe8f7f) SHA1(fbf8839b26be55ad83ad4db538ba3e196c1ab945) )
	ROM_LOAD( "ge72b-b3.ic24", 0x30000, 0x10000, CRC(4b0e92f4) SHA1(16ad9220ca6708028cea18c1c4b57e2b6eb425b4) )

	ROM_REGION( 0x20000, "samples", 0 ) // samples
	ROM_LOAD( "gen-v0.ic44",   0x00000, 0x20000, CRC(d8595c66) SHA1(97920c9947fbac609fb901415e5471c6e4ca066c) )

	ROM_REGION( 0x0200, "proms", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0600, "pals", 0 ) // where you see gen=72= actual label reads GEN(72)
	ROM_LOAD( "m72_a-3d-.ic11",      0x0000, 0x0117, CRC(8a3732ff) SHA1(6e3039e7dc424cbef7156312fa1ce67d7b082d30) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_a-4d-.ic19",      0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // PAL16L8 - bruteforced
	ROM_LOAD( "gen=m72=c-3f-b.ic13", 0x0400, 0x0117, CRC(028932a4) SHA1(04ec1fc874e7edb646777b301421953b26f49587) ) // PAL16L8 - bruteforced - located on M72-C-A top board
ROM_END

/*

A version of Air Duel is known to exist on an M72-C top board with the following ROM configuration:

AD C-H0-A through AD C-H3-A & AD L0-A through AD L3-A
AD C-00-A, AD C-01-A, AD C-10-A, AD C-11-A, AD C-20-A, AD C-21-A, AD C-30-A, AD C-31-A
AD C-V0-A & AD C-V1-A
AD C-PR-A MCU at IC1

unknown whether this version is just a half-sized ROM configuration of a current set or a set meant for a different region.

*/
ROM_START( airduelm72 )
	ROM_REGION( 0x100000, "maincpu", 0 ) // M72-C-A top board
	ROM_LOAD16_BYTE( "ad_c-h0-c.ic40", 0x00001, 0x20000, CRC(6467ed0f) SHA1(3b6db463168edec0e42247e4d913d9d7615061c1) )
	ROM_LOAD16_BYTE( "ad_c-l0-c.ic37", 0x00000, 0x20000, CRC(b90c4ffd) SHA1(c95998244c52e95f6612013c23051b293423946f) )
	ROM_LOAD16_BYTE( "ad_c-h3-.ic43",  0x40001, 0x20000, CRC(9f7cfca3) SHA1(becf827aa7749c54f1c435ea224e1fd9c8b3f5f9) )
	ROM_RELOAD(                        0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "ad_c-l3-.ic34",  0x40000, 0x20000, CRC(9dd343f7) SHA1(9f499936b6d3807aa5b5c18e9811c73c9a2c99f9) )
	ROM_RELOAD(                        0xc0000, 0x20000 )

	ROM_REGION( 0x1000, "mcu", 0 )  // i8751 microcontroller
	ROM_LOAD( "ad_c-pr-c.ic1", 0x0000, 0x1000, CRC(8785e4e2) SHA1(491f5646e1c5d68453086f05b9d79722eb5282ea) ) // i8751 MCU labeled  AD C-PR-C

	ROM_REGION( 0x080000, "sprites", 0 ) // mask ROMs
	ROM_LOAD( "ad-00.ic53", 0x00000, 0x20000, CRC(2f0d599b) SHA1(a966f806b5e25bb98cc63c46c49e0e676a62afcf) )
	ROM_LOAD( "ad-10.ic51", 0x20000, 0x20000, CRC(9865856b) SHA1(b18a06899ae29d45e2351594df544220f3f4485a) )
	ROM_LOAD( "ad-20.ic49", 0x40000, 0x20000, CRC(d392aef2) SHA1(0f639a07066cadddc3884eb490885a8745571567) )
	ROM_LOAD( "ad-30.ic47", 0x60000, 0x20000, CRC(923240c3) SHA1(f587a83329087a715a3e42110f74f104e8c8ef1f) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // mask ROMs
	ROM_LOAD( "ad-a0.ic21", 0x00000, 0x20000, CRC(ce134b47) SHA1(841358cc222c81b8a91edc262f355310d50b4dbb) )  // tiles #1
	ROM_LOAD( "ad-a1.ic22", 0x20000, 0x20000, CRC(097fd853) SHA1(8e08f4f4a747c899bb8e21b347635e26af9edc2d) )
	ROM_LOAD( "ad-a2.ic20", 0x40000, 0x20000, CRC(6a94c1b9) SHA1(55174acbac54236e5fc1b80d120cd6da9fe5524c) )
	ROM_LOAD( "ad-a3.ic23", 0x60000, 0x20000, CRC(6637c349) SHA1(27cb7c89ab73292b43f8ae3c0d803a01ef3d3936) )

	ROM_REGION( 0x080000, "gfx3", 0 ) // mask ROMs
	ROM_LOAD( "ad-b0.ic26", 0x00000, 0x20000, CRC(ce134b47) SHA1(841358cc222c81b8a91edc262f355310d50b4dbb) )  // tiles #2
	ROM_LOAD( "ad-b1.ic27", 0x20000, 0x20000, CRC(097fd853) SHA1(8e08f4f4a747c899bb8e21b347635e26af9edc2d) )
	ROM_LOAD( "ad-b2.ic25", 0x40000, 0x20000, CRC(6a94c1b9) SHA1(55174acbac54236e5fc1b80d120cd6da9fe5524c) )
	ROM_LOAD( "ad-b3.ic24", 0x60000, 0x20000, CRC(6637c349) SHA1(27cb7c89ab73292b43f8ae3c0d803a01ef3d3936) )

	ROM_REGION( 0x20000, "samples", 0 ) // mask ROM
	ROM_LOAD( "ad-v0.ic44", 0x00000, 0x20000, CRC(339f474d) SHA1(a81bb52598a0e31b2ed6a538755237c5d14d1844) )

	ROM_REGION( 0x0200, "proms", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0600, "pals", 0 )
	ROM_LOAD( "m72_a-3d-.ic11", 0x0000, 0x0117, CRC(8a3732ff) SHA1(6e3039e7dc424cbef7156312fa1ce67d7b082d30) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_a-4d-.ic19", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // PAL16L8 - bruteforced
	ROM_LOAD( "ad_c-3f-.ic13",  0x0400, 0x0117, CRC(9748fa38) SHA1(cc883dd801be03f2559c0dcd77580fe7d9546ed3) ) // PAL16L8 - bruteforced - located on M72-C-A top board
ROM_END


ROM_START( airdueljm72 )
	ROM_REGION( 0x100000, "maincpu", 0 ) // M72-C-A top board
	ROM_LOAD16_BYTE( "ad_c-h0-.ic40", 0x00001, 0x20000, CRC(12140276) SHA1(f218c5f2e6795b6295dea064817d7d6b1a7762b6) )
	ROM_LOAD16_BYTE( "ad_c-l0-.ic37", 0x00000, 0x20000, CRC(4ac0b91d) SHA1(97e2f633181cd5c25927fd0e2988af2acdb3f388) )
	ROM_LOAD16_BYTE( "ad_c-h3-.ic43", 0x40001, 0x20000, CRC(9f7cfca3) SHA1(becf827aa7749c54f1c435ea224e1fd9c8b3f5f9) )
	ROM_RELOAD(                       0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "ad_c-l3-.ic34", 0x40000, 0x20000, CRC(9dd343f7) SHA1(9f499936b6d3807aa5b5c18e9811c73c9a2c99f9) )
	ROM_RELOAD(                       0xc0000, 0x20000 )

	ROM_REGION( 0x1000, "mcu", 0 )  // i8751 microcontroller
	ROM_LOAD( "ad_c-pr-.ic1", 0x0000, 0x1000, CRC(45584e52) SHA1(647826f1bec9b39ae971ecb58d921d363ee4cf45) ) // i8751 MCU labeled  AD C-PR-

	ROM_REGION( 0x080000, "sprites", 0 ) // mask ROMs
	ROM_LOAD( "ad-00.ic53", 0x00000, 0x20000, CRC(2f0d599b) SHA1(a966f806b5e25bb98cc63c46c49e0e676a62afcf) )
	ROM_LOAD( "ad-10.ic51", 0x20000, 0x20000, CRC(9865856b) SHA1(b18a06899ae29d45e2351594df544220f3f4485a) )
	ROM_LOAD( "ad-20.ic49", 0x40000, 0x20000, CRC(d392aef2) SHA1(0f639a07066cadddc3884eb490885a8745571567) )
	ROM_LOAD( "ad-30.ic47", 0x60000, 0x20000, CRC(923240c3) SHA1(f587a83329087a715a3e42110f74f104e8c8ef1f) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // mask ROMs
	ROM_LOAD( "ad-a0.ic21", 0x00000, 0x20000, CRC(ce134b47) SHA1(841358cc222c81b8a91edc262f355310d50b4dbb) )  // tiles #1
	ROM_LOAD( "ad-a1.ic22", 0x20000, 0x20000, CRC(097fd853) SHA1(8e08f4f4a747c899bb8e21b347635e26af9edc2d) )
	ROM_LOAD( "ad-a2.ic20", 0x40000, 0x20000, CRC(6a94c1b9) SHA1(55174acbac54236e5fc1b80d120cd6da9fe5524c) )
	ROM_LOAD( "ad-a3.ic23", 0x60000, 0x20000, CRC(6637c349) SHA1(27cb7c89ab73292b43f8ae3c0d803a01ef3d3936) )

	ROM_REGION( 0x080000, "gfx3", 0 ) // mask ROMs
	ROM_LOAD( "ad-b0.ic26", 0x00000, 0x20000, CRC(ce134b47) SHA1(841358cc222c81b8a91edc262f355310d50b4dbb) )  // tiles #2
	ROM_LOAD( "ad-b1.ic27", 0x20000, 0x20000, CRC(097fd853) SHA1(8e08f4f4a747c899bb8e21b347635e26af9edc2d) )
	ROM_LOAD( "ad-b2.ic25", 0x40000, 0x20000, CRC(6a94c1b9) SHA1(55174acbac54236e5fc1b80d120cd6da9fe5524c) )
	ROM_LOAD( "ad-b3.ic24", 0x60000, 0x20000, CRC(6637c349) SHA1(27cb7c89ab73292b43f8ae3c0d803a01ef3d3936) )

	ROM_REGION( 0x20000, "samples", 0 ) // mask ROM
	ROM_LOAD( "ad-v0.ic44", 0x00000, 0x20000, CRC(339f474d) SHA1(a81bb52598a0e31b2ed6a538755237c5d14d1844) )

	ROM_REGION( 0x0200, "proms", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0600, "pals", 0 )
	ROM_LOAD( "m72_a-3d-.ic11", 0x0000, 0x0117, CRC(8a3732ff) SHA1(6e3039e7dc424cbef7156312fa1ce67d7b082d30) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_a-4d-.ic19", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // PAL16L8 - bruteforced
	ROM_LOAD( "ad_c-3f-.ic13",  0x0400, 0x0117, CRC(9748fa38) SHA1(cc883dd801be03f2559c0dcd77580fe7d9546ed3) ) // PAL16L8 - bruteforced - located on M72-C-A top board
ROM_END


ROM_START( gallopm72 )
	ROM_REGION( 0x100000, "maincpu", 0 ) // M72-C-A top board
	ROM_LOAD16_BYTE( "cc_c-h0-.ic40", 0x00001, 0x20000, CRC(2217dcd0) SHA1(9485b6c3eec99e720439e69dcbe0e55798bbff1c) )
	ROM_LOAD16_BYTE( "cc_c-l0-.ic37", 0x00000, 0x20000, CRC(ff39d7fb) SHA1(fad95f76050fce04464268b5edff6622b2cb798f) )
	ROM_LOAD16_BYTE( "cc_c-h3-.ic43", 0x40001, 0x20000, CRC(9b2bbab9) SHA1(255d4dda55be667f5f1f4324e9e66111738e79b3) )
	ROM_RELOAD(                       0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "cc_c-l3-.ic34", 0x40000, 0x20000, CRC(acd3278e) SHA1(83d7ddfbdb4bc9548a179b728351a21b3b0ac134) )
	ROM_RELOAD(                       0xc0000, 0x20000 )

	ROM_REGION( 0x1000, "mcu", 0 )  // i8751 microcontroller
	ROM_LOAD( "cc_c-pr-.ic1", 0x0000, 0x1000, CRC(ac4421b1) SHA1(4614acbc4efb26b27f3871b4d22879d74df5e2e0) ) // i8751 MCU labeled  CC C-PR-

	ROM_REGION( 0x080000, "sprites", 0 )  // sprites - same data as the cosmccop/gallop sets
	ROM_LOAD( "cc_c-00.ic53", 0x00000, 0x20000, CRC(9d99deaa) SHA1(acf16bea0f482306107d2a305c568406b6c21e9a) )   // == cc-b-n0.ic31
	ROM_LOAD( "cc_c-10.ic51", 0x20000, 0x20000, CRC(7eb083ed) SHA1(31fa7d532fd46e861c3d19d5b08661653f685a49) )   // == cc-b-n1.ic21
	ROM_LOAD( "cc_c-20.ic49", 0x40000, 0x20000, CRC(9421489e) SHA1(e43d042bf8b4ebed93558d74ec479ec60a01ca5c) )   // == cc-b-n2.ic32
	ROM_LOAD( "cc_c-30.ic47", 0x60000, 0x20000, CRC(920ec735) SHA1(2d0949b43dddce7317c45910d6e4868ddf010806) )   // == cc-b-n3.ic22

	ROM_REGION( 0x040000, "gfx2", 0 )  // tiles #1 - same data as the cosmccop/gallop sets split between the 2 tile banks
	ROM_LOAD( "cc_b-a0.ic21", 0x00000, 0x10000, CRC(a33472bd) SHA1(962047fe3dd1fb996285ecef615a8ebdb529adef) )   // == cc-d-g00.ic51 [1/2]
	ROM_LOAD( "cc_b-a1.ic22", 0x10000, 0x10000, CRC(118b1f2d) SHA1(7413ccc67a8aa9dae156e6ee122b1ca5beeb9a76) )   // == cc-d-g10.ic57 [1/2]
	ROM_LOAD( "cc_b-a2.ic20", 0x20000, 0x10000, CRC(83cebf48) SHA1(12847827ecbf6b493eb9dbddd0a469729d87a451) )   // == cc-d-g20.ic66 [1/2]
	ROM_LOAD( "cc_b-a3.ic23", 0x30000, 0x10000, CRC(572903fc) SHA1(03305301bcf939e97044e746594736b1ca1d7c0a) )   // == cc-d-g30.ic64 [1/2]

	ROM_REGION( 0x040000, "gfx3", 0 )  // tiles #2 - same data as the cosmccop/gallop sets split between the 2 tile banks
	ROM_LOAD( "cc_b-b0.ic26", 0x00000, 0x10000, CRC(0df5b439) SHA1(0775cf92139a111542c8b5f940da0f7f43020982) )   // == cc-d-g00.ic51 [2/2]
	ROM_LOAD( "cc_b-b1.ic27", 0x10000, 0x10000, CRC(010b778f) SHA1(cc5bfeb0fbe0ed2fe513458c5785ec0ce5b02f53) )   // == cc-d-g10.ic57 [2/2]
	ROM_LOAD( "cc_b-b2.ic25", 0x20000, 0x10000, CRC(bda9f6fb) SHA1(a6b655ae5bff0568c1fb56ee8a3874fc6524052c) )   // == cc-d-g20.ic66 [2/2]
	ROM_LOAD( "cc_b-b3.ic24", 0x30000, 0x10000, CRC(d361ba3f) SHA1(7348fdae03e997e05187a2726eb221edb92553df) )   // == cc-d-g30.ic64 [2/2]

	ROM_REGION( 0x20000, "samples", 0 ) // samples - same data as the cosmccop/gallop sets
	ROM_LOAD( "cc_c-v0.ic44", 0x00000, 0x20000, CRC(6247bade) SHA1(4bc9f86acd09908c74b1ab0e7817c4ff1cad6f0b) )   // == cc-d-v0.ic14

	ROM_REGION( 0x0200, "proms", 0 ) // Located on M72-A-C CPU/Sound board
	ROM_LOAD( "m72_a-8l-.ic66", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m72_a-9l-.ic75", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0600, "pals", 0 )
	ROM_LOAD( "m72_a-3d-.ic11", 0x0000, 0x0117, CRC(8a3732ff) SHA1(6e3039e7dc424cbef7156312fa1ce67d7b082d30) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m72_a-4d-.ic19", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // PAL16L8 - bruteforced
	ROM_LOAD( "cc_c-3f-.ic13",  0x0400, 0x0117, CRC(16ca7c50) SHA1(8291bb2fc12c374a970cdb7da6315b32a01cf3b8) ) // PAL16L8 - bruteforced - located on M72-C-A top board
ROM_END


/*****************************
  M81 sets
******************************/

ROM_START( xmultipl )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "xm_a-h1-.ic58", 0x00001, 0x20000, CRC(449048cf) SHA1(871b588177fb018937d143f76eda18aa53b0f6c4) )
	ROM_LOAD16_BYTE( "xm_a-l1-.ic67", 0x00000, 0x20000, CRC(26ce39b0) SHA1(18ae2e8c2c826c6ecfa66f7af5afdeeac3936543) )
	ROM_LOAD16_BYTE( "xm_a-h0-.ic59", 0x40001, 0x10000, CRC(509bc970) SHA1(44bb4ecedf8f127792e9a8da70b3a42c8ff30ad2) )
	ROM_RELOAD(                       0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "xm_a-l0-.ic68", 0x40000, 0x10000, CRC(490a9ebc) SHA1(55d9d3a4f82f120faabca78c2e47922831f62a5d) )
	ROM_RELOAD(                       0xe0000, 0x10000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "xm_a-sp-.ic14", 0x00000, 0x10000, CRC(006eef56) SHA1(917b26b200fa4c1692d4c7ca0ea0f7897e3e3b7b) )

	ROM_REGION( 0x100000, "sprites", 0 ) // mask ROMs
	ROM_LOAD( "t44.00.ic3",  0x00000, 0x20000, CRC(db45186e) SHA1(8c8edeb4b7e6b0516f2597823dc27eba9c5d9528) )  // sprites
	ROM_LOAD( "t45.01.ic2",  0x20000, 0x20000, CRC(4d0764d4) SHA1(4942333336a110b033f16ac1afa06ffef7b2dad6) )
	ROM_LOAD( "t46.10.ic5",  0x40000, 0x20000, CRC(f0c465a4) SHA1(69c107c860d4e8736431fd86b6821b70a8367eb3) )
	ROM_LOAD( "t47.11.ic4",  0x60000, 0x20000, CRC(1263b24b) SHA1(0445a5381df3a868bed6967c8e5de7169e4be6a3) )
	ROM_LOAD( "t48.20.ic12", 0x80000, 0x20000, CRC(4129944f) SHA1(988b072032d1667c3ac0731fada32fb6978505dc) )
	ROM_LOAD( "t49.21.ic11", 0xa0000, 0x20000, CRC(2346e6f9) SHA1(b3de017dd0353e04d279f57e151c47f5fcc70e9c) )
	ROM_LOAD( "t50.30.ic14", 0xc0000, 0x20000, CRC(e322543e) SHA1(b4c3a7f202d81485d5f0a7b7668ee89fc1edb215) )
	ROM_LOAD( "t51.31.ic13", 0xe0000, 0x20000, CRC(229bf7b1) SHA1(ae42c7efbb6278dd3fa56842361138391f2d49ca) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // mask ROMs
	ROM_LOAD( "t53.a0.ic50", 0x00000, 0x20000, CRC(1a082494) SHA1(63a3a84a262833d2cafab41e35df8f10a5e317b1) )  // tiles #1
	ROM_LOAD( "t54.a1.ic49", 0x20000, 0x20000, CRC(076c16c5) SHA1(4be858806b916953d59aceee550e721eaf3996a6) )
	ROM_LOAD( "t55.a2.ic51", 0x40000, 0x20000, CRC(25d877a5) SHA1(48c948bf714c432f534c098123c8f50d5561756f) )
	ROM_LOAD( "t56.a3.ic52", 0x60000, 0x20000, CRC(5b1213f5) SHA1(87782aa0bd04d4378c4ba78b63028ae2709da2f1) )

	ROM_REGION( 0x080000, "gfx3", 0 ) // mask ROMs
	// b0-b3 are populated, Jumper J3 on the M81-B-B board is set to 'W' meaning use the ROMs from the b0-b3 positions
	ROM_LOAD( "t57.b0.ic47", 0x00000, 0x20000, CRC(0a84e0c7) SHA1(67ad181a7d2c431cb4bf45955e09754549a03576) )  // tiles #2
	ROM_LOAD( "t58.b1.ic48", 0x20000, 0x20000, CRC(a874121d) SHA1(1351d5901d55059c6472a4588a2e560396903861) )
	ROM_LOAD( "t59.b2.ic46", 0x40000, 0x20000, CRC(69deb990) SHA1(1eed3183efbe576376661b45152a0a21240ecfc8) )
	ROM_LOAD( "t60.b3.ic45", 0x60000, 0x20000, CRC(14c69f99) SHA1(4bea72f8bd421ef3ca559363f7473ce2e7038699) )

	ROM_REGION( 0x20000, "samples", 0 ) // mask ROM
	ROM_LOAD( "t52.v0.ic11", 0x00000, 0x20000, CRC(2db1bd80) SHA1(657006d0642ec7fb949bb52821d78fe51a599415) )  // samples

	ROM_REGION( 0x0200, "proms", 0 ) // proms
	ROM_LOAD( "m81_a-9l-.ic72", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m81_a-9p-.ic74", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "m81_b-9h-.ic94", 0x0000, 0x0117, CRC(4bc1d393) SHA1(d9358a409b4568839ee6cea7241e37f557c3d3a9) ) // TIBPAL-16L8-25 - bruteforced - located on M81-B-B daughterboard
	ROM_LOAD( "m81_a-5l-.ic28", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "m81_a-2h-.ic2",  0x0400, 0x0117, CRC(21ede612) SHA1(5d05d3088f3d248db8948da175551ea29d7478b5) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "xm_a-7d-.ic48",  0x0600, 0x0117, CRC(2ef7ad4e) SHA1(a7fae2f34f60de11362cd712c63b681012452e96) ) // TIBPAL-16L8-25 - bruteforced
ROM_END


ROM_START( hharry ) // where you see gen=81= actual label reads GEN(81)
	ROM_REGION( 0x100000, "maincpu", 0 ) // located on M81-A-B mainboard
	ROM_LOAD16_BYTE( "gen=m81=_a-h0-v.ic59", 0x00001, 0x20000, CRC(c52802a5) SHA1(7180189c886aebe8d3e7fd38922916cecfddae32) ) // labeled  GEN(M81)  A-H0-V
	ROM_LOAD16_BYTE( "gen=m81=_a-l0-v.ic68", 0x00000, 0x20000, CRC(f463074c) SHA1(aca86345610e65848c276ab278092d35ba215916) ) // labeled  GEN(M81)  A-L0-V
	ROM_LOAD16_BYTE( "gen=m81=_a-h1-0.ic58", 0x60001, 0x10000, CRC(3ae21335) SHA1(780d7a0c5bebe4b914ea5b3741e30630f8c29a4f) ) // labeled  GEN(M81)  A-H1-0
	ROM_RELOAD(                              0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "gen=m81=_a-l1-0.ic67", 0x60000, 0x10000, CRC(bc6ac5f9) SHA1(c6afba4967a8055f6b63827697425eac743f5a75) ) // labeled  GEN(M81)  A-L1-0
	ROM_RELOAD(                              0xe0000, 0x10000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "gen=m81=_a-sp-0.ic14",   0x00000, 0x10000, CRC(80e210e7) SHA1(66cff58fb37c52e1d8e0567e13b774253e862585) ) // labeled  GEN(M81)  A-SP-0

	ROM_REGION( 0x080000, "sprites", 0 ) // sprites - located on M81-B-B daughterboard
	ROM_LOAD( "gen=m81=_00.ic3",  0x00000, 0x20000, CRC(ec5127ef) SHA1(014ac8ad7b19cd9b475b72a0f42a4991119501c4) ) // mask ROMs with no labels
	ROM_LOAD( "gen=m81=_10.ic5",  0x20000, 0x20000, CRC(def65294) SHA1(23f5d99fa9f604fde37cb52113bff233d9be1d25) )
	ROM_LOAD( "gen=m81=_20.ic12", 0x40000, 0x20000, CRC(bb0d6ad4) SHA1(4ab617fadfc32efad90ed7f0555513f167b0c43a) )
	ROM_LOAD( "gen=m81=_30.ic14", 0x60000, 0x20000, CRC(4351044e) SHA1(0d3ce3f4f1473fd997e70de91e7b5b5a5ec60ad4) )

	ROM_REGION( 0x080000, "gfx2", ROMREGION_ERASEFF ) // tiles - located on M81-B-B daughterboard
	ROM_LOAD( "gen=m81=_a0.ic50", 0x00000, 0x20000, CRC(c577ba5f) SHA1(c882e58cf64deca8eee6f14f3df43ecc932488fc) ) // mask ROMs with no labels
	ROM_LOAD( "gen=m81=_a1.ic49", 0x20000, 0x20000, CRC(429d12ab) SHA1(ccba25eab981fc4e664f76e06a2964066f2ae2e8) )
	ROM_LOAD( "gen=m81=_a2.ic51", 0x40000, 0x20000, CRC(b5b163b0) SHA1(82a708fea4953a7c4dcd1d4a1b07f302221ba30b) )
	ROM_LOAD( "gen=m81=_a3.ic52", 0x60000, 0x20000, CRC(8ef566a1) SHA1(3afb020a7317efe89c18b2a7773894ce28499d49) )

	ROM_REGION( 0x080000, "gfx3", ROMREGION_ERASEFF )
	// b0-b3 are unpopulated, Jumper J3 on the M81-B-B board is set to 'S' meaning use the ROMs from the a0-a3 positions

	ROM_REGION( 0x20000, "samples", 0 ) // samples
	ROM_LOAD( "gen=m81=_a-v0-0.ic11",   0x00000, 0x20000, CRC(faaacaff) SHA1(ea3a3920255c07aa9c0a7e0191eae257a9f7f558) ) // labeled  GEN(M81)  A-V0-0

	ROM_REGION( 0x0200, "proms", 0 ) // proms - located on M81-A-B mainboard
	ROM_LOAD( "m81_a-9l-.ic72", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m81_a-9p-.ic74", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "m81_b-9h-.ic94",      0x0000, 0x0117, CRC(4bc1d393) SHA1(d9358a409b4568839ee6cea7241e37f557c3d3a9) ) // TIBPAL-16L8-25 - bruteforced - located on M81-B-B daughterboard
	ROM_LOAD( "m81_a-5l-.ic28",      0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "m81_a-2h-.ic2",       0x0400, 0x0117, CRC(21ede612) SHA1(5d05d3088f3d248db8948da175551ea29d7478b5) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "gen=81=_a-7d-a.ic48", 0x0600, 0x0117, CRC(92fe9eff) SHA1(970f9ed9b2fc3289cded9cde2b68cce2106fae7d) ) // TIBPAL-16L8-25 - bruteforced
ROM_END


ROM_START( dbreed )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "db_a-h0-.ic59", 0x00001, 0x10000, CRC(e1177267) SHA1(f226f34ce85305870e659dd4f519bee30936af9a) )
	ROM_CONTINUE(                     0x60001, 0x10000 )
	ROM_RELOAD(                       0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "db_a-l0-.ic68", 0x00000, 0x10000, CRC(d82b167e) SHA1(f9ccb152feb31971230f61371a906bd900ef34e8) )
	ROM_CONTINUE(                     0x60000, 0x10000 )
	ROM_RELOAD(                       0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "db_a-sp-.ic14", 0x00000, 0x10000, CRC(54a61560) SHA1(e5fccfcedcadbab1667900f98370043c1907dd89) )

	ROM_REGION( 0x080000, "sprites", 0 ) // mask ROMs
	ROM_LOAD( "db_k800m.00.ic2",  0x00000, 0x20000, CRC(c027a8cf) SHA1(534dc416b8f5587168c7f644d3f9438c8a190491) )   // sprites
	ROM_LOAD( "db_k801m.10.ic5",  0x20000, 0x20000, CRC(093faf33) SHA1(2704f644cdce87daf975984f143b1d55ba731c3f) )
	ROM_LOAD( "db_k802m.20.ic12", 0x40000, 0x20000, CRC(055b4c59) SHA1(71315dd7476612f138cb64b905648791d44eb7da) )
	ROM_LOAD( "db_k803m.30.ic14", 0x60000, 0x20000, CRC(8ed63922) SHA1(51daa8a23e637f6b4394598ff4a1d26f65b59c8b) )

	ROM_REGION( 0x080000, "gfx2", ROMREGION_ERASEFF ) // mask ROMs
	ROM_LOAD( "db_k804m.a0.ic50", 0x00000, 0x20000, CRC(4c83e92e) SHA1(6dade027435c48ab48bd4516d16a9961d4dd6fad) )   // tiles
	ROM_LOAD( "db_k805m.a1.ic49", 0x20000, 0x20000, CRC(835ef268) SHA1(89d0bb15201440dffad3ef745970f95505d7ab03) )
	ROM_LOAD( "db_k806m.a2.ic51", 0x40000, 0x20000, CRC(5117f114) SHA1(a401a3e638209b32d4101a5c2e2a8b4612eaa21b) )
	ROM_LOAD( "db_k807m.a3.ic52", 0x60000, 0x20000, CRC(8eb0c978) SHA1(7fc55bbe4d0923db88492bb7160a89de34e11cd6) )

	ROM_REGION( 0x080000, "gfx3", ROMREGION_ERASEFF )
	// b0-b3 are unpopulated, Jumper J3 on the M81-B-B board is set to 'S' meaning use the ROMs from the a0-a3 positions

	ROM_REGION( 0x20000, "samples", 0 )
	ROM_LOAD( "db_a-v0-.ic11", 0x00000, 0x20000, CRC(312f7282) SHA1(742d56980b4618180e9a0e02051c5aec4d5cdae4) )  // samples

	ROM_REGION( 0x0200, "proms", 0 ) // proms - located on M81-A-B mainboard
	ROM_LOAD( "m81_a-9l-.ic72", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "m81_a-9p-.ic74", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "m81_b-9h-.ic94", 0x0000, 0x0117, CRC(4bc1d393) SHA1(d9358a409b4568839ee6cea7241e37f557c3d3a9) ) // TIBPAL-16L8-25 - bruteforced - located on M81-B-B daughterboard
	ROM_LOAD( "m81_a-5l-.ic28", 0x0200, 0x0117, CRC(56c29834) SHA1(a66c589845f9995c673325f1161c687eb90d68c1) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "m81_a-2h-.ic2",  0x0400, 0x0117, CRC(21ede612) SHA1(5d05d3088f3d248db8948da175551ea29d7478b5) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "db_a-7d-.ic48",  0x0600, 0x0117, CRC(421cecd6) SHA1(621304f8161ec22457b6a2cb83c69ca663dd703e) ) // TIBPAL-16L8-25 - bruteforced
ROM_END


/*****************************
  M82 sets
******************************/

ROM_START( majtitle )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mt_a-h0-a.ic52", 0x00001, 0x20000, CRC(36aadb67) SHA1(11cb9f190431ef7b68bcad691191c810b00452be) )
	ROM_LOAD16_BYTE( "mt_a-l0-a.ic60", 0x00000, 0x20000, CRC(2e1b6242) SHA1(a1d36c1bad7eb874e6b37a372d0ff95452b09315) )
	ROM_LOAD16_BYTE( "mt_a-h1-a.ic51", 0x40001, 0x20000, CRC(e1402a22) SHA1(97ecf3cf5438dd3aad65554b672a4e401917dc34) )
	ROM_RELOAD(                        0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "mt_a-l1-a.ic59", 0x40000, 0x20000, CRC(0efa409a) SHA1(2b4fd62705398c7ac1647f6ea821f722b4f7496f) )
	ROM_RELOAD(                        0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "mt_a-sp.ic15", 0x00000, 0x10000, CRC(e44260a9) SHA1(a2512033c8cca9a8064eae1ada721202edf06e8e) )

	ROM_REGION( 0x100000, "sprites", 0 ) // mask ROMs
	ROM_LOAD( "mt_n0.ic44",   0x00000, 0x40000, CRC(5618cddc) SHA1(16d34b431ab9b72067fa669d694e635c88aeb261) )  // sprites #1
	ROM_LOAD( "mt_n1.ic45",   0x40000, 0x40000, CRC(483b873b) SHA1(654efd67b2102521e8c46cd57cefa2cc64cf4fd3) )
	ROM_LOAD( "mt_n2.ic46",   0x80000, 0x40000, CRC(4f5d665b) SHA1(f539d0f5c738ffabfac16121706abe3bb3b2a1fa) )
	ROM_LOAD( "mt_n3.ic36",   0xc0000, 0x40000, CRC(83571549) SHA1(ce0b89aa4b3e3e1cf6ec6136f956577267cdd9d3) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // mask ROMs
	ROM_LOAD( "mt_c0.ic49",   0x00000, 0x20000, CRC(780e7a02) SHA1(9776ecb8b5d86636061f8360464001a63bec0842) )  // tiles
	ROM_LOAD( "mt_c1.ic48",   0x20000, 0x20000, CRC(45ad1381) SHA1(de281398dcd1c547bde9fa86f8ca409dd8d4aa6c) )
	ROM_LOAD( "mt_c2.ic57",   0x40000, 0x20000, CRC(5df5856d) SHA1(f16163f672de6701b411315c9956ddb74c8464ce) )
	ROM_LOAD( "mt_c3.ic56",   0x60000, 0x20000, CRC(f5316cc8) SHA1(123892d4a7e8d98582ea736afe659afdba8c5f87) )

	ROM_REGION( 0x080000, "sprites2", 0 ) // mask ROMs
	ROM_LOAD( "mt_f0.ic38",   0x00000, 0x20000, CRC(2d5e05d5) SHA1(18bdc9c561dbf0f91642161ca985d2154bd58b5d) )  // sprites #2
	ROM_LOAD( "mt_f1.ic39",   0x20000, 0x20000, CRC(c68cd65f) SHA1(8999b558b4af0f453ada9e4ef705163df96844e6) )
	ROM_LOAD( "mt_f2.ic40",   0x40000, 0x20000, CRC(a71feb2d) SHA1(47e366b422772bed08ee4d1c338970687d6c3b4c) )
	ROM_LOAD( "mt_f3.ic41",   0x60000, 0x20000, CRC(179f7562) SHA1(6d28b199daffc62e8fa9009878ac0bb976ccbb2a) )

	ROM_REGION( 0x20000, "samples", 0 ) // mask ROM
	ROM_LOAD( "mt_vo.ic12",   0x00000, 0x20000, CRC(eb24bb2c) SHA1(9fca04fba0249e8213dd164eb6829e1a5acbee65) )  // samples

	ROM_REGION( 0x0200, "proms", 0 ) // proms - located on M82-B-B daughterboard
	ROM_LOAD( "mt_b-6a-.ic37", 0x0000, 0x0100, NO_DUMP ) // TBP24S10
	ROM_LOAD( "mt_b-7c-.ic47", 0x0100, 0x0100, NO_DUMP ) // TBP24S10

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "mt_b-3q-.ic23", 0x0000, 0x0117, CRC(8c864543) SHA1(1167524b72a4d72898dcdc2561a9706f8683c888) ) // TIBPAL-16L8-25 - bruteforced - located on M82-B-B daughterboard
	ROM_LOAD( "mt_a-2h-.ic5",  0x0200, 0x0117, CRC(21ede612) SHA1(5d05d3088f3d248db8948da175551ea29d7478b5) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "mt_a-5l-.ic31", 0x0400, 0x0117, CRC(e3064bfe) SHA1(c25b0d734be332042f86661d8743008c2a9d3a4e) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "mt_a-7d-.ic43", 0x0600, 0x0117, CRC(d7ec1cc6) SHA1(94afb0098206777d68de5e6122114fdc76618931) ) // TIBPAL-16L8-25 - bruteforced
ROM_END

ROM_START( majtitlej )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mt_a-h0-.ic52", 0x00001, 0x20000, CRC(b9682c70) SHA1(b979c0a630397f2a2eb73709cf12c5262c973782) )
	ROM_LOAD16_BYTE( "mt_a-l0-.ic60", 0x00000, 0x20000, CRC(702c9fd6) SHA1(84a5e9e64f4bf235d115f5648b4a108f710ade1d) )
	ROM_LOAD16_BYTE( "mt_a-h1-.ic51", 0x40001, 0x20000, CRC(d9e97c30) SHA1(97f59b614eeeced0a414f8a1693590525a58f788) )
	ROM_RELOAD(                       0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "mt_a-l1-.ic59", 0x40000, 0x20000, CRC(8dbd91b5) SHA1(2bd01f3fba0fa1ca4b6f8ff57e7dc4434c42ce48) )
	ROM_RELOAD(                       0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "mt_a-sp.ic15", 0x00000, 0x10000, CRC(e44260a9) SHA1(a2512033c8cca9a8064eae1ada721202edf06e8e) )

	ROM_REGION( 0x100000, "sprites", 0 ) // mask ROMs
	ROM_LOAD( "mt_n0.ic44",   0x00000, 0x40000, CRC(5618cddc) SHA1(16d34b431ab9b72067fa669d694e635c88aeb261) )  // sprites #1
	ROM_LOAD( "mt_n1.ic45",   0x40000, 0x40000, CRC(483b873b) SHA1(654efd67b2102521e8c46cd57cefa2cc64cf4fd3) )
	ROM_LOAD( "mt_n2.ic46",   0x80000, 0x40000, CRC(4f5d665b) SHA1(f539d0f5c738ffabfac16121706abe3bb3b2a1fa) )
	ROM_LOAD( "mt_n3.ic36",   0xc0000, 0x40000, CRC(83571549) SHA1(ce0b89aa4b3e3e1cf6ec6136f956577267cdd9d3) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // mask ROMs
	ROM_LOAD( "mt_c0.ic49",   0x00000, 0x20000, CRC(780e7a02) SHA1(9776ecb8b5d86636061f8360464001a63bec0842) )  // tiles
	ROM_LOAD( "mt_c1.ic48",   0x20000, 0x20000, CRC(45ad1381) SHA1(de281398dcd1c547bde9fa86f8ca409dd8d4aa6c) )
	ROM_LOAD( "mt_c2.ic57",   0x40000, 0x20000, CRC(5df5856d) SHA1(f16163f672de6701b411315c9956ddb74c8464ce) )
	ROM_LOAD( "mt_c3.ic56",   0x60000, 0x20000, CRC(f5316cc8) SHA1(123892d4a7e8d98582ea736afe659afdba8c5f87) )

	ROM_REGION( 0x080000, "sprites2", 0 ) // mask ROMs
	ROM_LOAD( "mt_f0.ic38",   0x00000, 0x20000, CRC(2d5e05d5) SHA1(18bdc9c561dbf0f91642161ca985d2154bd58b5d) )  // sprites #2
	ROM_LOAD( "mt_f1.ic39",   0x20000, 0x20000, CRC(c68cd65f) SHA1(8999b558b4af0f453ada9e4ef705163df96844e6) )
	ROM_LOAD( "mt_f2.ic40",   0x40000, 0x20000, CRC(a71feb2d) SHA1(47e366b422772bed08ee4d1c338970687d6c3b4c) )
	ROM_LOAD( "mt_f3.ic41",   0x60000, 0x20000, CRC(179f7562) SHA1(6d28b199daffc62e8fa9009878ac0bb976ccbb2a) )

	ROM_REGION( 0x20000, "samples", 0 ) // mask ROM
	ROM_LOAD( "mt_vo.ic12",   0x00000, 0x20000, CRC(eb24bb2c) SHA1(9fca04fba0249e8213dd164eb6829e1a5acbee65) )  // samples

	ROM_REGION( 0x0200, "proms", 0 ) // proms - located on M82-B-B daughterboard
	ROM_LOAD( "mt_b-6a-.ic37", 0x0000, 0x0100, NO_DUMP ) // TBP24S10
	ROM_LOAD( "mt_b-7c-.ic47", 0x0100, 0x0100, NO_DUMP ) // TBP24S10

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "mt_a-2h-.ic5",  0x0000, 0x0117, CRC(21ede612) SHA1(5d05d3088f3d248db8948da175551ea29d7478b5) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "mt_a-5l-.ic31", 0x0200, 0x0117, CRC(e3064bfe) SHA1(c25b0d734be332042f86661d8743008c2a9d3a4e) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "mt_a-7d-.ic43", 0x0400, 0x0117, CRC(d7ec1cc6) SHA1(94afb0098206777d68de5e6122114fdc76618931) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "mt_b-3q-.ic23", 0x0600, 0x0117, CRC(8c864543) SHA1(1167524b72a4d72898dcdc2561a9706f8683c888) ) // TIBPAL-16L8-25 - bruteforced - located on M82-B-B daughterboard
ROM_END


ROM_START( airduel )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ad_=m82=_a-h0-d.ic52", 0x00001, 0x20000, CRC(dbecc726) SHA1(526e9fdf0ca3af3eae462524df71af8e6bfa85d0) )
	ROM_LOAD16_BYTE( "ad_=m82=_a-l0-d.ic60", 0x00000, 0x20000, CRC(6a9fcf59) SHA1(a2ae64d290137036c350f84c38054cf6681473a5) )
	ROM_LOAD16_BYTE( "ad_=m82=_a-h1-d.ic51", 0x40001, 0x20000, CRC(bafc152a) SHA1(e20fa8b832ebfb7a4407fc162f28388858686d61) )
	ROM_RELOAD(                              0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "ad_=m82=_a-l1-d.ic59", 0x40000, 0x20000, CRC(9e2b1ae7) SHA1(838ccffb760b464d7d7e108e033a09e6295e5fc8) )
	ROM_RELOAD(                              0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ad_=m82=_a-sp-d.ic15", 0x00000, 0x10000, CRC(16a858a3) SHA1(51dbac5b37ecb30b46072f5a300a29dc7f7b8542) )

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "ad_=m82=_b-n0-d.ic44", 0x00000, 0x20000, CRC(2f0d599b) SHA1(a966f806b5e25bb98cc63c46c49e0e676a62afcf) )  // sprites
	ROM_LOAD( "ad_=m82=_b-n1-d.ic45", 0x20000, 0x20000, CRC(9865856b) SHA1(b18a06899ae29d45e2351594df544220f3f4485a) )
	ROM_LOAD( "ad_=m82=_b-n2-d.ic46", 0x40000, 0x20000, CRC(d392aef2) SHA1(0f639a07066cadddc3884eb490885a8745571567) )
	ROM_LOAD( "ad_=m82=_b-n3-d.ic36", 0x60000, 0x20000, CRC(923240c3) SHA1(f587a83329087a715a3e42110f74f104e8c8ef1f) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "ad_=m82=_a-c0-d.ic49", 0x00000, 0x20000, CRC(ce134b47) SHA1(841358cc222c81b8a91edc262f355310d50b4dbb) )  // tiles
	ROM_LOAD( "ad_=m82=_a-c1-d.ic48", 0x20000, 0x20000, CRC(097fd853) SHA1(8e08f4f4a747c899bb8e21b347635e26af9edc2d) )
	ROM_LOAD( "ad_=m82=_a-c2-d.ic57", 0x40000, 0x20000, CRC(6a94c1b9) SHA1(55174acbac54236e5fc1b80d120cd6da9fe5524c) )
	ROM_LOAD( "ad_=m82=_a-c3-d.ic56", 0x60000, 0x20000, CRC(6637c349) SHA1(27cb7c89ab73292b43f8ae3c0d803a01ef3d3936) )

	ROM_REGION( 0x080000, "sprites2", 0 ) // still had these leftover from Major Title, probably needed to avoid displaying garbage?
	ROM_LOAD( "mt_f0.ic38",    0x00000, 0x20000, CRC(2d5e05d5) SHA1(18bdc9c561dbf0f91642161ca985d2154bd58b5d) )  // sprites #2
	ROM_LOAD( "mt_f1.ic39",    0x20000, 0x20000, CRC(c68cd65f) SHA1(8999b558b4af0f453ada9e4ef705163df96844e6) )
	ROM_LOAD( "mt_f2.ic40",    0x40000, 0x20000, CRC(a71feb2d) SHA1(47e366b422772bed08ee4d1c338970687d6c3b4c) )
	ROM_LOAD( "mt_f3.ic41",    0x60000, 0x20000, CRC(179f7562) SHA1(6d28b199daffc62e8fa9009878ac0bb976ccbb2a) )

	ROM_REGION( 0x20000, "samples", 0 ) // samples
	ROM_LOAD( "ad_=m82=_a-vo-d.ic12", 0x00000, 0x20000, CRC(339f474d) SHA1(a81bb52598a0e31b2ed6a538755237c5d14d1844) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "mt_a-2h-.ic5",  0x0000, 0x0117, CRC(21ede612) SHA1(5d05d3088f3d248db8948da175551ea29d7478b5) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "mt_a-5l-.ic31", 0x0200, 0x0117, CRC(e3064bfe) SHA1(c25b0d734be332042f86661d8743008c2a9d3a4e) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "mt_a-7d-.ic43", 0x0400, 0x0117, CRC(d7ec1cc6) SHA1(94afb0098206777d68de5e6122114fdc76618931) ) // TIBPAL-16L8-25 - bruteforced
ROM_END

ROM_START( airduelu )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "r10-m82a-h0.ic52", 0x00001, 0x20000, CRC(17f19965) SHA1(51f84903eba9b0daa6f91f3578e6fdb9b286ef1c) )
	ROM_LOAD16_BYTE( "r10-m82a-l0.ic60", 0x00000, 0x20000, CRC(f8b54d6c) SHA1(9796daa6728736258b45e3cf8ead6d6c6481f36b) )
	ROM_LOAD16_BYTE( "r10-m82-h1.ic51",  0x40001, 0x20000, CRC(bafc152a) SHA1(e20fa8b832ebfb7a4407fc162f28388858686d61) )
	ROM_RELOAD(                          0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "r10-m82-l1.ic59",  0x40000, 0x20000, CRC(9e2b1ae7) SHA1(838ccffb760b464d7d7e108e033a09e6295e5fc8) )
	ROM_RELOAD(                          0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "r10-bgm.ic15", 0x00000, 0x10000, CRC(16a858a3) SHA1(51dbac5b37ecb30b46072f5a300a29dc7f7b8542) )

	ROM_REGION( 0x080000, "sprites", 0 ) // same data as airduel
	ROM_LOAD( "r10-obj0.ic44", 0x00000, 0x20000, CRC(2f0d599b) SHA1(a966f806b5e25bb98cc63c46c49e0e676a62afcf) )  // sprites
	ROM_LOAD( "r10-obj1.ic45", 0x20000, 0x20000, CRC(9865856b) SHA1(b18a06899ae29d45e2351594df544220f3f4485a) )
	ROM_LOAD( "r10-obj2.ic46", 0x40000, 0x20000, CRC(d392aef2) SHA1(0f639a07066cadddc3884eb490885a8745571567) )
	ROM_LOAD( "r10-obj3.ic36", 0x60000, 0x20000, CRC(923240c3) SHA1(f587a83329087a715a3e42110f74f104e8c8ef1f) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // same data as airduel
	ROM_LOAD( "r10-chr0.ic49", 0x00000, 0x20000, CRC(ce134b47) SHA1(841358cc222c81b8a91edc262f355310d50b4dbb) )  // tiles
	ROM_LOAD( "r10-chr1.ic48", 0x20000, 0x20000, CRC(097fd853) SHA1(8e08f4f4a747c899bb8e21b347635e26af9edc2d) )
	ROM_LOAD( "r10-chr2.ic57", 0x40000, 0x20000, CRC(6a94c1b9) SHA1(55174acbac54236e5fc1b80d120cd6da9fe5524c) )
	ROM_LOAD( "r10-chr3.ic56", 0x60000, 0x20000, CRC(6637c349) SHA1(27cb7c89ab73292b43f8ae3c0d803a01ef3d3936) )

	ROM_REGION( 0x080000, "sprites2", 0 ) // leftover from Major Title (mask roms, soldered on pcb)
	ROM_LOAD( "mt_f0.ic38",    0x00000, 0x20000, CRC(2d5e05d5) SHA1(18bdc9c561dbf0f91642161ca985d2154bd58b5d) )  // sprites #2
	ROM_LOAD( "mt_f1.ic39",    0x20000, 0x20000, CRC(c68cd65f) SHA1(8999b558b4af0f453ada9e4ef705163df96844e6) )
	ROM_LOAD( "mt_f2.ic40",    0x40000, 0x20000, CRC(a71feb2d) SHA1(47e366b422772bed08ee4d1c338970687d6c3b4c) )
	ROM_LOAD( "mt_f3.ic41",    0x60000, 0x20000, CRC(179f7562) SHA1(6d28b199daffc62e8fa9009878ac0bb976ccbb2a) )

	ROM_REGION( 0x20000, "samples", 0 ) // same data as airduel
	ROM_LOAD( "r10-vo.ic12",   0x00000, 0x20000, CRC(339f474d) SHA1(a81bb52598a0e31b2ed6a538755237c5d14d1844) )  // samples

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "mt_a-2h-.ic5",  0x0000, 0x0117, CRC(21ede612) SHA1(5d05d3088f3d248db8948da175551ea29d7478b5) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "mt_a-5l-.ic31", 0x0200, 0x0117, CRC(e3064bfe) SHA1(c25b0d734be332042f86661d8743008c2a9d3a4e) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "mt_a-7d-.ic43", 0x0400, 0x0117, CRC(d7ec1cc6) SHA1(94afb0098206777d68de5e6122114fdc76618931) ) // TIBPAL-16L8-25 - bruteforced
ROM_END


ROM_START( rtypem82b )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rt_h0.ic52", 0x00001, 0x20000, CRC(5fa5068b) SHA1(b33891d2e0ca7e52226c1318ad657ac6bc7d6df4) )
	ROM_LOAD16_BYTE( "rt_l0.ic60", 0x00000, 0x20000, CRC(aee6fae8) SHA1(22645da3bfeb3a7517bdbf0829fd9d689ddc5368) )
	ROM_LOAD16_BYTE( "rt_h1.ic51", 0x40001, 0x20000, CRC(76389df4) SHA1(20004dd1d058589bdd8ea93a66a6cdf93382c7cc) )
	ROM_RELOAD(                    0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "rt_l1.ic59", 0x40000, 0x20000, CRC(6af66a05) SHA1(3c686b4559e4e223e3b03533fe2b2fc4758f4e02) )
	ROM_RELOAD(                    0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "rt_sp.ic15", 0x00000, 0x10000, CRC(24fded65) SHA1(34e085ebfc6415a60b7440ac53c8ae7130b5e9d4) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "rt_n0.ic44", 0x00000, 0x20000, CRC(236e93ad) SHA1(a168c2f007a7469d8c1d834dc5247d99d13fd36d) )  // sprites #1
	ROM_LOAD( "rt_n1.ic45", 0x20000, 0x20000, CRC(94e0da50) SHA1(0e8aef07b2a4a60bb6faa9ea3d02869d30dff84c) )
	ROM_LOAD( "rt_n2.ic46", 0x40000, 0x20000, CRC(6310dd0e) SHA1(4e4a50ef64cdfddea10d415a4b2d2490c1364074) )
	ROM_LOAD( "rt_n3.ic36", 0x60000, 0x20000, CRC(dd9674fb) SHA1(925bbd64015ec9109a74ce80747bea2bfdb0cde6) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "rt_c0.ic38", 0x00000, 0x40000, CRC(c2511272) SHA1(138dd131f827215f13ba0761cacc0f383b5e5a48) )  // tiles
	ROM_LOAD( "rt_c1.ic39", 0x40000, 0x40000, CRC(6da33dae) SHA1(2b5f686c5c8e45a896ab115818066d03af767cb5) )
	ROM_LOAD( "rt_c2.ic40", 0x80000, 0x40000, CRC(29322d6e) SHA1(b553d46f1270dcc4754800e65c21b5e418994fcd) )
	ROM_LOAD( "rt_c3.ic41", 0xc0000, 0x40000, CRC(0ab3a8db) SHA1(7f4f5c18b5df0f5fdcb471db4e87c1be393aca92) )

	ROM_REGION( 0x080000, "sprites2", 0 ) // leftover from Major Title
	ROM_LOAD( "mt_f0.ic38", 0x00000, 0x20000, CRC(2d5e05d5) SHA1(18bdc9c561dbf0f91642161ca985d2154bd58b5d) )  // sprites #2
	ROM_LOAD( "mt_f1.ic39", 0x20000, 0x20000, CRC(c68cd65f) SHA1(8999b558b4af0f453ada9e4ef705163df96844e6) )
	ROM_LOAD( "mt_f2.ic40", 0x40000, 0x20000, CRC(a71feb2d) SHA1(47e366b422772bed08ee4d1c338970687d6c3b4c) )
	ROM_LOAD( "mt_f3.ic41", 0x60000, 0x20000, CRC(179f7562) SHA1(6d28b199daffc62e8fa9009878ac0bb976ccbb2a) )

	ROM_REGION( 0x20000, "samples", 0 ) // samples - leftover from Major Title
	ROM_LOAD( "mt_vo.ic12", 0x00000, 0x20000, CRC(eb24bb2c) SHA1(9fca04fba0249e8213dd164eb6829e1a5acbee65) )
ROM_END

ROM_START( rtype2m82b )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rt2_h0.ic52", 0x00001, 0x20000, CRC(47639a78) SHA1(d7dd851fed96d46c850e5c8f24d9d1a081f6b297) )
	ROM_LOAD16_BYTE( "rt2_l0.ic60", 0x00000, 0x20000, CRC(a1661cdf) SHA1(d209328d678fc2fc405bd20f5134bd85b4cd4802) )
	ROM_LOAD16_BYTE( "rt2_h1.ic51", 0x40001, 0x20000, CRC(4b79840c) SHA1(6cf8c8cf4bcf5e2acdaa05b8dca2f2a969edc2c5) )
	ROM_RELOAD(                     0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "rt2_l1.ic59", 0x40000, 0x20000, CRC(6ab3ae42) SHA1(d3d7c35e1583b55cc668aa011471c3bf04a541af) )
	ROM_RELOAD(                     0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "rt2_sp.ic15", 0x00000, 0x10000, CRC(73ffecb4) SHA1(4795bf0d6263060c3d3759b659bdb189a4087600) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "rt2_n0.ic44", 0x00000, 0x20000, CRC(2cd8f913) SHA1(a53752b35da95b420dd29a09176d265d292b3938) )  // sprites #1
	ROM_LOAD( "rt2_n1.ic45", 0x20000, 0x20000, CRC(5033066d) SHA1(e125127f0610c63f9e59a585db547be5d49ed863) )
	ROM_LOAD( "rt2_n2.ic46", 0x40000, 0x20000, CRC(ec3a0450) SHA1(632bdd397f1bc67f6970faf7d09ab8d911e105fe) )
	ROM_LOAD( "rt2_n3.ic36", 0x60000, 0x20000, CRC(db6176fc) SHA1(1eaf72af0322490c98461aded202288e387caac1) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "rt2_c0.ic38", 0x00000, 0x40000, CRC(f5bad5f2) SHA1(dc86b93f62e8947e3551f07e393e740e5dc43f5e) )  // tiles
	ROM_LOAD( "rt2_c1.ic39", 0x40000, 0x40000, CRC(71451778) SHA1(52ca7aa8522b988a19556313041450c767dad054) )
	ROM_LOAD( "rt2_c2.ic40", 0x80000, 0x40000, CRC(c6b0c352) SHA1(eec4fa88c27815960106881e7ccb23e62556bf1c) )
	ROM_LOAD( "rt2_c3.ic41", 0xc0000, 0x40000, CRC(6d530a32) SHA1(4e4100e5e5d88e65fb5494474d3692ecd8f44343) )

	ROM_REGION( 0x080000, "sprites2", 0 ) // leftover from Major Title
	ROM_LOAD( "mt_f0.ic38",  0x00000, 0x20000, CRC(2d5e05d5) SHA1(18bdc9c561dbf0f91642161ca985d2154bd58b5d) )  // sprites #2
	ROM_LOAD( "mt_f1.ic39",  0x20000, 0x20000, CRC(c68cd65f) SHA1(8999b558b4af0f453ada9e4ef705163df96844e6) )
	ROM_LOAD( "mt_f2.ic40",  0x40000, 0x20000, CRC(a71feb2d) SHA1(47e366b422772bed08ee4d1c338970687d6c3b4c) )
	ROM_LOAD( "mt_f3.ic41",  0x60000, 0x20000, CRC(179f7562) SHA1(6d28b199daffc62e8fa9009878ac0bb976ccbb2a) )

	ROM_REGION( 0x20000, "samples", 0 ) // samples
	ROM_LOAD( "rt2_vo.ic12", 0x00000, 0x20000, CRC(637172d5) SHA1(9dd0dc409306287238826bf301e2a7a12d6cd9ce) )
ROM_END


ROM_START( dkgensanm82 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gen_=m84=_a-h0-d.ic52", 0x00001, 0x20000, CRC(a1ca8855) SHA1(19b0ec1a03af114aa4f02630ace67c277ebce60f) )
	ROM_LOAD16_BYTE( "gen_=m84=_a-l0-d.ic60", 0x00000, 0x20000, CRC(247117b0) SHA1(e7674b9d0ae80afb52b47094f95ed5c250c7e303) )
	ROM_LOAD16_BYTE( "gen_=m84=_a-h1-d.ic51", 0x60001, 0x10000, CRC(54e5b73c) SHA1(5664f6e0a931b1c139e82dc98fcc9e38acd14616) )
	ROM_RELOAD(                               0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "gen_=m84=_a-l1-d.ic59", 0x60000, 0x10000, CRC(894f8a9f) SHA1(57a0885c52a094def03b129a450cc891e6c075c6) )
	ROM_RELOAD(                               0xe0000, 0x10000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "gen_=m84=_a-sp-d.ic15", 0x00000, 0x10000, CRC(e83cfc2c) SHA1(3193bdd06a9712fc499e6fc90a33140463ef59fe) )

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "gen_=m72=_c-l0-b.ic44", 0x00000, 0x20000, CRC(ec5127ef) SHA1(014ac8ad7b19cd9b475b72a0f42a4991119501c4) )  // sprites
	ROM_LOAD( "gen_=m72=_c-l3-b.ic45", 0x20000, 0x20000, CRC(def65294) SHA1(23f5d99fa9f604fde37cb52113bff233d9be1d25) )
	ROM_LOAD( "gen_=m72=_c-h0-b.ic46", 0x40000, 0x20000, CRC(bb0d6ad4) SHA1(4ab617fadfc32efad90ed7f0555513f167b0c43a) )
	ROM_LOAD( "gen_=m72=_c-h3-b.ic36", 0x60000, 0x20000, CRC(4351044e) SHA1(0d3ce3f4f1473fd997e70de91e7b5b5a5ec60ad4) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "gen_=m81=_a-l0-a.ic49", 0x00000, 0x20000, CRC(c577ba5f) SHA1(c882e58cf64deca8eee6f14f3df43ecc932488fc) )  // tiles
	ROM_LOAD( "gen_=m81=_a-l1-a.ic48", 0x20000, 0x20000, CRC(429d12ab) SHA1(ccba25eab981fc4e664f76e06a2964066f2ae2e8) )
	ROM_LOAD( "gen_=m81=_a-h0-a.ic57", 0x40000, 0x20000, CRC(b5b163b0) SHA1(82a708fea4953a7c4dcd1d4a1b07f302221ba30b) )
	ROM_LOAD( "gen_=m81=_a-h1-a.ic56", 0x60000, 0x20000, CRC(8ef566a1) SHA1(3afb020a7317efe89c18b2a7773894ce28499d49) )

	ROM_REGION( 0x080000, "sprites2", 0 ) // leftover from Major Title (mask roms, soldered on pcb)
	ROM_LOAD( "mt_f0.ic38", 0x00000, 0x20000, CRC(2d5e05d5) SHA1(18bdc9c561dbf0f91642161ca985d2154bd58b5d) )  // sprites #2
	ROM_LOAD( "mt_f1.ic39", 0x20000, 0x20000, CRC(c68cd65f) SHA1(8999b558b4af0f453ada9e4ef705163df96844e6) )
	ROM_LOAD( "mt_f2.ic40", 0x40000, 0x20000, CRC(a71feb2d) SHA1(47e366b422772bed08ee4d1c338970687d6c3b4c) )
	ROM_LOAD( "mt_f3.ic41", 0x60000, 0x20000, CRC(179f7562) SHA1(6d28b199daffc62e8fa9009878ac0bb976ccbb2a) )

	ROM_REGION( 0x20000, "samples", 0 ) // samples
	ROM_LOAD( "gen_=m84=_a-vo-d.ic12", 0x00000, 0x20000, CRC(d8595c66) SHA1(97920c9947fbac609fb901415e5471c6e4ca066c) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "mt_a-2h-.ic5",        0x0000, 0x0117, CRC(21ede612) SHA1(5d05d3088f3d248db8948da175551ea29d7478b5) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "mt_a-5l-.ic31",       0x0200, 0x0117, CRC(e3064bfe) SHA1(c25b0d734be332042f86661d8743008c2a9d3a4e) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "gen=m84=a-7d-d.ic43", 0x0400, 0x0117, CRC(a3ee35fe) SHA1(c6d448c245d7c7dde981c970ce70f7d0f43723ed) ) // TIBPAL-16L8-25 labeled GEN(M84)-A-7D-D - bruteforced
	ROM_LOAD( "gen=m72=c-3f-b.ic23", 0x0600, 0x0117, CRC(028932a4) SHA1(04ec1fc874e7edb646777b301421953b26f49587) ) // TIBPAL-16L8-25 labeled GEN(M72)-C-3F-B - bruteforced - located on M82-B-B daughterboard
ROM_END


/*****************************
  M84 sets
******************************/

ROM_START( rtype2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rt2_a-h0-d.ic54", 0x00001, 0x20000, CRC(d8ece6f4) SHA1(f7bb246fe8b75af24716d419bb3c6e7d9cd0971e) )
	ROM_LOAD16_BYTE( "rt2_a-l0-d.ic60", 0x00000, 0x20000, CRC(32cfb2e4) SHA1(d4b44a40e2933040eddb2b09de7bfe28d76c5f25) )
	ROM_LOAD16_BYTE( "rt2_a-h1-d.ic53", 0x40001, 0x20000, CRC(4f6e9b15) SHA1(ef733c2615951f54691877ad3e84d08107723324) )
	ROM_RELOAD(                         0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "rt2_a-l1-d.ic59", 0x40000, 0x20000, CRC(0fd123bf) SHA1(1133163f6716e9a4bbb437b3a471477d0bd97051) )
	ROM_RELOAD(                         0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "rt2_a-sp-.ic17", 0x00000, 0x10000, CRC(73ffecb4) SHA1(4795bf0d6263060c3d3759b659bdb189a4087600) )

	ROM_REGION( 0x080000, "sprites", 0 ) // tiles - located on M84-B-A daughterboard
	ROM_LOAD( "rt2_b-n0.ic31", 0x00000, 0x20000, CRC(2cd8f913) SHA1(a53752b35da95b420dd29a09176d265d292b3938) )  // mask ROMs with no labels
	ROM_LOAD( "rt2_b-n1.ic21", 0x20000, 0x20000, CRC(5033066d) SHA1(e125127f0610c63f9e59a585db547be5d49ed863) )
	ROM_LOAD( "rt2_b-n2.ic32", 0x40000, 0x20000, CRC(ec3a0450) SHA1(632bdd397f1bc67f6970faf7d09ab8d911e105fe) )
	ROM_LOAD( "rt2_b-n3.ic22", 0x60000, 0x20000, CRC(db6176fc) SHA1(1eaf72af0322490c98461aded202288e387caac1) )

	ROM_REGION( 0x100000, "gfx2", 0 ) // tiles - located on M84-A-A mainboard
	ROM_LOAD( "rt2_a-g00.ic50", 0x00000, 0x20000, CRC(f3f8736e) SHA1(37872b30459ad05b2981d4ac84983f3b52d0d2d6) )  // mask ROMs with no labels
	ROM_LOAD( "rt2_a-g01.ic51", 0x20000, 0x20000, CRC(b4c543af) SHA1(56042eba711160fc701021c8787414dcaddcdecb) )
	ROM_LOAD( "rt2_a-g10.ic56", 0x40000, 0x20000, CRC(4cb80d66) SHA1(31c5496c14b277e428a2f22195fe1742d6a577d4) )
	ROM_LOAD( "rt2_a-g11.ic57", 0x60000, 0x20000, CRC(bee128e0) SHA1(b149dae5f8f67a329d6df033fadf50ad75c0a57a) )
	ROM_LOAD( "rt2_a-g20.ic65", 0x80000, 0x20000, CRC(2dc9c71a) SHA1(124e89c17f3af034d5a387ff3eab906d289c27f7) )
	ROM_LOAD( "rt2_a-g21.ic66", 0xa0000, 0x20000, CRC(7533c428) SHA1(ba435cfb6c3c49fcc4d716dcecf8f17545b8eec6) )
	ROM_LOAD( "rt2_a-g30.ic63", 0xc0000, 0x20000, CRC(a6ad67f2) SHA1(b005b037ce8b3c932089982ecfbccdc922278fe3) )
	ROM_LOAD( "rt2_a-g31.ic64", 0xe0000, 0x20000, CRC(3686d555) SHA1(d03754d9b8a6a3bfd4a85eeddacc35a36af197bd) )

	ROM_REGION( 0x20000, "samples", 0 )
	ROM_LOAD( "rt2_a-vo-.ic14", 0x00000, 0x20000, CRC(637172d5) SHA1(9dd0dc409306287238826bf301e2a7a12d6cd9ce) )  // samples

	ROM_REGION( 0x0200, "proms", 0 ) // located on M84-B-A daughterboard
	ROM_LOAD( "rt2_b-4n-.ic23", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "rt2_b-4p-.ic24", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "rt2_a-2h-.ic5",  0x0000, 0x0117, CRC(21ede612) SHA1(5d05d3088f3d248db8948da175551ea29d7478b5) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "rt2_a-5l-.ic33", 0x0200, 0x0117, CRC(86e87e50) SHA1(5d4090aa76f8adfd389fbf53fb000d0bbfa1b5f6) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "rt2_a-7d-.ic45", 0x0400, 0x0104, CRC(53c1e087) SHA1(b214ba4e7cc3d582ee85616923f38fd4873dacb1) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "rt2_b-3a-.ic9",  0x0600, 0x0117, CRC(8ad303aa) SHA1(9c7b426f3aea9ce45a02b18ee65723d51ddfcc0d) ) // TIBPAL-16L8-25 - bruteforced
ROM_END

ROM_START( rtype2j )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rt2_a-h0-.ic54", 0x00001, 0x20000, CRC(7857ccf6) SHA1(9f6774a8128ee2dbb5b6c42289095275337bc73e) )
	ROM_LOAD16_BYTE( "rt2_a-l0-.ic60", 0x00000, 0x20000, CRC(cb22cd6e) SHA1(a877cffbac9f55bca8932b12540a4686ba975684) )
	ROM_LOAD16_BYTE( "rt2_a-h1-.ic53", 0x40001, 0x20000, CRC(49e75d28) SHA1(956bafaaa6711a8a13f2bffe43e8d05d51d8a3c9) )
	ROM_RELOAD(                        0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "rt2_a-l1-.ic59", 0x40000, 0x20000, CRC(12ec1676) SHA1(10cee9a87dd954444b0e64fad7f15a5ae529890d) )
	ROM_RELOAD(                        0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "rt2_a-sp-.ic17", 0x00000, 0x10000, CRC(73ffecb4) SHA1(4795bf0d6263060c3d3759b659bdb189a4087600) )

	ROM_REGION( 0x080000, "sprites", 0 ) // tiles - located on M84-B-A daughterboard
	ROM_LOAD( "rt2_b-n0.ic31", 0x00000, 0x20000, CRC(2cd8f913) SHA1(a53752b35da95b420dd29a09176d265d292b3938) )  // mask ROMs with no labels
	ROM_LOAD( "rt2_b-n1.ic21", 0x20000, 0x20000, CRC(5033066d) SHA1(e125127f0610c63f9e59a585db547be5d49ed863) )
	ROM_LOAD( "rt2_b-n2.ic32", 0x40000, 0x20000, CRC(ec3a0450) SHA1(632bdd397f1bc67f6970faf7d09ab8d911e105fe) )
	ROM_LOAD( "rt2_b-n3.ic22", 0x60000, 0x20000, CRC(db6176fc) SHA1(1eaf72af0322490c98461aded202288e387caac1) )

	ROM_REGION( 0x100000, "gfx2", 0 ) // tiles - located on M84-A-A mainboard
	ROM_LOAD( "rt2_a-g00.ic50", 0x00000, 0x20000, CRC(f3f8736e) SHA1(37872b30459ad05b2981d4ac84983f3b52d0d2d6) )  // mask ROMs with no labels
	ROM_LOAD( "rt2_a-g01.ic51", 0x20000, 0x20000, CRC(b4c543af) SHA1(56042eba711160fc701021c8787414dcaddcdecb) )
	ROM_LOAD( "rt2_a-g10.ic56", 0x40000, 0x20000, CRC(4cb80d66) SHA1(31c5496c14b277e428a2f22195fe1742d6a577d4) )
	ROM_LOAD( "rt2_a-g11.ic57", 0x60000, 0x20000, CRC(bee128e0) SHA1(b149dae5f8f67a329d6df033fadf50ad75c0a57a) )
	ROM_LOAD( "rt2_a-g20.ic65", 0x80000, 0x20000, CRC(2dc9c71a) SHA1(124e89c17f3af034d5a387ff3eab906d289c27f7) )
	ROM_LOAD( "rt2_a-g21.ic66", 0xa0000, 0x20000, CRC(7533c428) SHA1(ba435cfb6c3c49fcc4d716dcecf8f17545b8eec6) )
	ROM_LOAD( "rt2_a-g30.ic63", 0xc0000, 0x20000, CRC(a6ad67f2) SHA1(b005b037ce8b3c932089982ecfbccdc922278fe3) )
	ROM_LOAD( "rt2_a-g31.ic64", 0xe0000, 0x20000, CRC(3686d555) SHA1(d03754d9b8a6a3bfd4a85eeddacc35a36af197bd) )

	ROM_REGION( 0x20000, "samples", 0 ) // samples
	ROM_LOAD( "rt2_a-vo-.ic14", 0x00000, 0x20000, CRC(637172d5) SHA1(9dd0dc409306287238826bf301e2a7a12d6cd9ce) )

	ROM_REGION( 0x0200, "proms", 0 ) // located on M84-B-A daughterboard
	ROM_LOAD( "rt2_b-4n-.ic23", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "rt2_b-4p-.ic24", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "rt2_a-2h-.ic5",  0x0000, 0x0117, CRC(21ede612) SHA1(5d05d3088f3d248db8948da175551ea29d7478b5) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "rt2_a-5l-.ic33", 0x0200, 0x0117, CRC(86e87e50) SHA1(5d4090aa76f8adfd389fbf53fb000d0bbfa1b5f6) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "rt2_a-7d-.ic45", 0x0400, 0x0104, CRC(53c1e087) SHA1(b214ba4e7cc3d582ee85616923f38fd4873dacb1) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "rt2_b-3a-.ic9",  0x0600, 0x0117, CRC(8ad303aa) SHA1(9c7b426f3aea9ce45a02b18ee65723d51ddfcc0d) ) // TIBPAL-16L8-25 - bruteforced
ROM_END

ROM_START( rtype2jc )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rt2_a-h0-c.ic54", 0x00001, 0x20000, CRC(ef9a9990) SHA1(fdf8fb7cb2b16f3cc58592da5c696ccff85f87e0) )
	ROM_LOAD16_BYTE( "rt2_a-l0-c.ic60", 0x00000, 0x20000, CRC(d8b9da64) SHA1(ce19fc95d0adefa57c2161d7b0d756ecf799c707) )
	ROM_LOAD16_BYTE( "rt2_a-h1-c.ic53", 0x40001, 0x20000, CRC(1b1870f4) SHA1(9a98b146980a87d088b7157da7a64c99902ddd54) )
	ROM_RELOAD(                         0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "rt2_a-l1-c.ic59", 0x40000, 0x20000, CRC(60fdff35) SHA1(9c89682deebfa88864b5af9cf0f05944d5c2212f) )
	ROM_RELOAD(                         0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "rt2_a-sp-.ic17", 0x00000, 0x10000, CRC(73ffecb4) SHA1(4795bf0d6263060c3d3759b659bdb189a4087600) )

	ROM_REGION( 0x080000, "sprites", 0 ) // tiles - located on M84-B-A daughterboard
	ROM_LOAD( "rt2_b-n0.ic31", 0x00000, 0x20000, CRC(2cd8f913) SHA1(a53752b35da95b420dd29a09176d265d292b3938) )  // mask ROMs with no labels
	ROM_LOAD( "rt2_b-n1.ic21", 0x20000, 0x20000, CRC(5033066d) SHA1(e125127f0610c63f9e59a585db547be5d49ed863) )
	ROM_LOAD( "rt2_b-n2.ic32", 0x40000, 0x20000, CRC(ec3a0450) SHA1(632bdd397f1bc67f6970faf7d09ab8d911e105fe) )
	ROM_LOAD( "rt2_b-n3.ic22", 0x60000, 0x20000, CRC(db6176fc) SHA1(1eaf72af0322490c98461aded202288e387caac1) )

	ROM_REGION( 0x100000, "gfx2", 0 ) // tiles - located on M84-A-A mainboard
	ROM_LOAD( "rt2_a-g00.ic50", 0x00000, 0x20000, CRC(f3f8736e) SHA1(37872b30459ad05b2981d4ac84983f3b52d0d2d6) )  // mask ROMs with no labels
	ROM_LOAD( "rt2_a-g01.ic51", 0x20000, 0x20000, CRC(b4c543af) SHA1(56042eba711160fc701021c8787414dcaddcdecb) )
	ROM_LOAD( "rt2_a-g10.ic56", 0x40000, 0x20000, CRC(4cb80d66) SHA1(31c5496c14b277e428a2f22195fe1742d6a577d4) )
	ROM_LOAD( "rt2_a-g11.ic57", 0x60000, 0x20000, CRC(bee128e0) SHA1(b149dae5f8f67a329d6df033fadf50ad75c0a57a) )
	ROM_LOAD( "rt2_a-g20.ic65", 0x80000, 0x20000, CRC(2dc9c71a) SHA1(124e89c17f3af034d5a387ff3eab906d289c27f7) )
	ROM_LOAD( "rt2_a-g21.ic66", 0xa0000, 0x20000, CRC(7533c428) SHA1(ba435cfb6c3c49fcc4d716dcecf8f17545b8eec6) )
	ROM_LOAD( "rt2_a-g30.ic63", 0xc0000, 0x20000, CRC(a6ad67f2) SHA1(b005b037ce8b3c932089982ecfbccdc922278fe3) )
	ROM_LOAD( "rt2_a-g31.ic64", 0xe0000, 0x20000, CRC(3686d555) SHA1(d03754d9b8a6a3bfd4a85eeddacc35a36af197bd) )

	ROM_REGION( 0x20000, "samples", 0 ) // samples
	ROM_LOAD( "rt2_a-vo-.ic14", 0x00000, 0x20000, CRC(637172d5) SHA1(9dd0dc409306287238826bf301e2a7a12d6cd9ce) )

	ROM_REGION( 0x0200, "proms", 0 ) // located on M84-B-A daughterboard
	ROM_LOAD( "rt2_b-4n-.ic23", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "rt2_b-4p-.ic24", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "rt2_a-2h-.ic5",  0x0000, 0x0117, CRC(21ede612) SHA1(5d05d3088f3d248db8948da175551ea29d7478b5) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "rt2_a-5l-.ic33", 0x0200, 0x0117, CRC(86e87e50) SHA1(5d4090aa76f8adfd389fbf53fb000d0bbfa1b5f6) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "rt2_a-7d-.ic45", 0x0400, 0x0104, CRC(53c1e087) SHA1(b214ba4e7cc3d582ee85616923f38fd4873dacb1) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "rt2_b-3a-.ic9",  0x0600, 0x0117, CRC(8ad303aa) SHA1(9c7b426f3aea9ce45a02b18ee65723d51ddfcc0d) ) // TIBPAL-16L8-25 - bruteforced
ROM_END


ROM_START( hharryu ) // where you see gen=84= actual label reads GEN(84)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gen_a-h0-u.ic55", 0x00001, 0x20000, CRC(ede7f755) SHA1(adcec83d6b936ab1a14d039792b9375e9f803a08) )
	ROM_LOAD16_BYTE( "gen_a-l0-u.ic61", 0x00000, 0x20000, CRC(df0726ae) SHA1(7ef163d2e8c14a14328d4365705bb31540bdc7cb) )
	ROM_LOAD16_BYTE( "gen_a-h1-f.ic54", 0x60001, 0x10000, CRC(31b741c5) SHA1(46c1c4cea09477cc4989f3e06e08851d02743e62) )
	ROM_RELOAD(                         0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "gen_a-l1-f.ic60", 0x60000, 0x10000, CRC(b23e966c) SHA1(f506f6d1f4f7874070e91d1df8f141cca031ce29) )
	ROM_RELOAD(                         0xe0000, 0x10000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "gen=84=_a-sp-0-f.ic14", 0x00000, 0x10000, CRC(80e210e7) SHA1(66cff58fb37c52e1d8e0567e13b774253e862585) )

	ROM_REGION( 0x080000, "sprites", 0 ) // sprites - located on M84-C-A top board
	ROM_LOAD( "hh_n0.ic33", 0x00000, 0x20000, CRC(ec5127ef) SHA1(014ac8ad7b19cd9b475b72a0f42a4991119501c4) )  // mask ROMs with no labels
	ROM_LOAD( "hh_n1.ic34", 0x20000, 0x20000, CRC(def65294) SHA1(23f5d99fa9f604fde37cb52113bff233d9be1d25) )
	ROM_LOAD( "hh_n2.ic35", 0x40000, 0x20000, CRC(bb0d6ad4) SHA1(4ab617fadfc32efad90ed7f0555513f167b0c43a) )
	ROM_LOAD( "hh_n3.ic36", 0x60000, 0x20000, CRC(4351044e) SHA1(0d3ce3f4f1473fd997e70de91e7b5b5a5ec60ad4) )

	ROM_REGION( 0x080000, "gfx2", 0 )  // tiles
	ROM_LOAD( "hh_a0.ic51", 0x00000, 0x20000, CRC(c577ba5f) SHA1(c882e58cf64deca8eee6f14f3df43ecc932488fc) )  // mask ROMs with no labels
	ROM_LOAD( "hh_a1.ic57", 0x20000, 0x20000, CRC(429d12ab) SHA1(ccba25eab981fc4e664f76e06a2964066f2ae2e8) )
	ROM_LOAD( "hh_a2.ic66", 0x40000, 0x20000, CRC(b5b163b0) SHA1(82a708fea4953a7c4dcd1d4a1b07f302221ba30b) )
	ROM_LOAD( "hh_a3.ic64", 0x60000, 0x20000, CRC(8ef566a1) SHA1(3afb020a7317efe89c18b2a7773894ce28499d49) )

	ROM_REGION( 0x20000, "samples", 0 ) // samples
	ROM_LOAD( "gen=84=_a-vo-f.ic17", 0x00000, 0x20000, CRC(faaacaff) SHA1(ea3a3920255c07aa9c0a7e0191eae257a9f7f558) )

	ROM_REGION( 0x200, "proms", 0 ) // located on M84-C-A top board
	ROM_LOAD( "gen=84=_c-4n-.ic21", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "gen=84=_c-4p-.ic22", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "gen=84=_a-2h.ic5",  0x0000, 0x0117, CRC(21ede612) SHA1(5d05d3088f3d248db8948da175551ea29d7478b5) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "gen=84=_a-5l.ic33", 0x0200, 0x0117, CRC(579e257d) SHA1(bea2da60dc068fe16f469695f66786fe5406a823) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "gen=84=_a-7d.ic45", 0x0400, 0x0117, CRC(79ef86f2) SHA1(69d3ead62e2c70f5831ec6915920da356c922dfb) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "gen=84=_c-3a.ic8",  0x0600, 0x0117, CRC(c1e19913) SHA1(7292ea25df818fe25e00dc4f37b3338abf2caaa2) ) // TIBPAL-16L8-25 - bruteforced - located on M84-C-A top board
ROM_END

ROM_START( dkgensan ) // where you see gen=84= actual label reads GEN(84)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gen_a-h0-.ic55", 0x00001, 0x20000, CRC(07a45f6d) SHA1(8ffbd395aad244747d9f87062d2b062f41a4829c) )
	ROM_LOAD16_BYTE( "gen_a-l0-.ic61", 0x00000, 0x20000, CRC(46478fea) SHA1(fd4ff544588535333c1b98fbc08446ef49b11212) )
	ROM_LOAD16_BYTE( "gen_a-h1-.ic54", 0x60001, 0x10000, CRC(54e5b73c) SHA1(5664f6e0a931b1c139e82dc98fcc9e38acd14616) )
	ROM_RELOAD(                        0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "gen_a-l1-.ic60", 0x60000, 0x10000, CRC(894f8a9f) SHA1(57a0885c52a094def03b129a450cc891e6c075c6) )
	ROM_RELOAD(                        0xe0000, 0x10000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "gen_a-sp-.ic17", 0x00000, 0x10000, CRC(e83cfc2c) SHA1(3193bdd06a9712fc499e6fc90a33140463ef59fe) )

	ROM_REGION( 0x080000, "sprites", 0 ) // sprites - located on M84-C-A top board
	ROM_LOAD( "hh_n0.ic33", 0x00000, 0x20000, CRC(ec5127ef) SHA1(014ac8ad7b19cd9b475b72a0f42a4991119501c4) )  // mask ROMs with no labels
	ROM_LOAD( "hh_n1.ic34", 0x20000, 0x20000, CRC(def65294) SHA1(23f5d99fa9f604fde37cb52113bff233d9be1d25) )
	ROM_LOAD( "hh_n2.ic35", 0x40000, 0x20000, CRC(bb0d6ad4) SHA1(4ab617fadfc32efad90ed7f0555513f167b0c43a) )
	ROM_LOAD( "hh_n3.ic36", 0x60000, 0x20000, CRC(4351044e) SHA1(0d3ce3f4f1473fd997e70de91e7b5b5a5ec60ad4) )

	ROM_REGION( 0x080000, "gfx2", 0 )  // tiles
	ROM_LOAD( "hh_a0.ic51", 0x00000, 0x20000, CRC(c577ba5f) SHA1(c882e58cf64deca8eee6f14f3df43ecc932488fc) )  // mask ROMs with no labels
	ROM_LOAD( "hh_a1.ic57", 0x20000, 0x20000, CRC(429d12ab) SHA1(ccba25eab981fc4e664f76e06a2964066f2ae2e8) )
	ROM_LOAD( "hh_a2.ic66", 0x40000, 0x20000, CRC(b5b163b0) SHA1(82a708fea4953a7c4dcd1d4a1b07f302221ba30b) )
	ROM_LOAD( "hh_a3.ic64", 0x60000, 0x20000, CRC(8ef566a1) SHA1(3afb020a7317efe89c18b2a7773894ce28499d49) )

	ROM_REGION( 0x20000, "samples", 0 ) // samples
	ROM_LOAD( "gen_a-vo-.ic14", 0x00000, 0x20000, CRC(d8595c66) SHA1(97920c9947fbac609fb901415e5471c6e4ca066c) )

	ROM_REGION( 0x200, "proms", 0 ) // located on M84-C-A top board
	ROM_LOAD( "gen=84=_c-4n-.ic21", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10
	ROM_LOAD( "gen=84=_c-4p-.ic22", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) ) // TBP24S10

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "gen=84=_a-2h.ic5",  0x0000, 0x0117, CRC(21ede612) SHA1(5d05d3088f3d248db8948da175551ea29d7478b5) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "gen=84=_a-5l.ic33", 0x0200, 0x0117, CRC(579e257d) SHA1(bea2da60dc068fe16f469695f66786fe5406a823) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "gen=84=_a-7d.ic45", 0x0400, 0x0117, CRC(79ef86f2) SHA1(69d3ead62e2c70f5831ec6915920da356c922dfb) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "gen=84=_c-3a.ic8",  0x0600, 0x0117, CRC(c1e19913) SHA1(7292ea25df818fe25e00dc4f37b3338abf2caaa2) ) // TIBPAL-16L8-25 - bruteforced - located on M84-C-A top board
ROM_END

ROM_START( hharryb )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "4-a-27c010a.bin", 0x00001, 0x20000, CRC(755c0874) SHA1(28ea0f7d700cc0119d8cb5693d0bcfef3da78c95) )
	ROM_LOAD16_BYTE( "6-a-27c010a.bin", 0x00000, 0x20000, CRC(f10fb55c) SHA1(1bb0d56a29ca34b003c57faa9693b96413718608) )
	ROM_LOAD16_BYTE( "3-a-27c512.bin",  0x60001, 0x10000, CRC(31b741c5) SHA1(46c1c4cea09477cc4989f3e06e08851d02743e62) )
	ROM_RELOAD(                         0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "5-a-27c512.bin",  0x60000, 0x10000, CRC(b23e966c) SHA1(f506f6d1f4f7874070e91d1df8f141cca031ce29) )
	ROM_RELOAD(                         0xe0000, 0x10000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "2-a-27c512.bin",   0x00000, 0x10000, CRC(80e210e7) SHA1(66cff58fb37c52e1d8e0567e13b774253e862585) )

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "17-c-27c010a.bin",    0x00000, 0x20000, CRC(ec5127ef) SHA1(014ac8ad7b19cd9b475b72a0f42a4991119501c4) ) // Same data as other M84 Hammerin' Harry sets
	ROM_LOAD( "16-c-27c010a.bin",    0x20000, 0x20000, CRC(def65294) SHA1(23f5d99fa9f604fde37cb52113bff233d9be1d25) )
	ROM_LOAD( "14-c-27c010a.bin",    0x40000, 0x20000, CRC(bb0d6ad4) SHA1(4ab617fadfc32efad90ed7f0555513f167b0c43a) )
	ROM_LOAD( "15-c-27c010a.bin",    0x60000, 0x20000, CRC(4351044e) SHA1(0d3ce3f4f1473fd997e70de91e7b5b5a5ec60ad4) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "13-b-27c010a.bin",   0x00000, 0x20000, CRC(c577ba5f) SHA1(c882e58cf64deca8eee6f14f3df43ecc932488fc) ) // Same data as other M84 Hammerin' Harry sets
	ROM_LOAD( "11-b-27c010a.bin",   0x20000, 0x20000, CRC(429d12ab) SHA1(ccba25eab981fc4e664f76e06a2964066f2ae2e8) )
	ROM_LOAD( "9-b-27c010a.bin",    0x40000, 0x20000, CRC(b5b163b0) SHA1(82a708fea4953a7c4dcd1d4a1b07f302221ba30b) )
	ROM_LOAD( "7-b-27c010a.bin",    0x60000, 0x20000, CRC(8ef566a1) SHA1(3afb020a7317efe89c18b2a7773894ce28499d49) )

	ROM_REGION( 0x20000, "samples", 0 )
	ROM_LOAD( "1-a-27c010a.bin",   0x00000, 0x20000, CRC(faaacaff) SHA1(ea3a3920255c07aa9c0a7e0191eae257a9f7f558) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "19-c-82s129.bin", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) )
	ROM_LOAD( "18-c-82s129.bin", 0x0100, 0x0100, CRC(a4f2c4bc) SHA1(f13b0a4b52dcc6704063b676f09d83dcba170133) )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "a-pal16l8.bin", 0x000, 0x104, CRC(1358c513) SHA1(7c8f44e4d63867d54e16fc29d168a27be5f4babf) )
ROM_END

ROM_START( hharryb2 ) // 2-PCB set marked TON/A and TON/B (same as the lohtb set). Audio is simplified (Z80 + YM2151)
	ROM_REGION( 0x100000, "maincpu", 0 ) // on bottom PCB, code is most similar to the M84 version
	ROM_LOAD16_BYTE( "3.ic30", 0x00001, 0x20000, CRC(026001f0) SHA1(bcbc6b44085336836885005dfe9d40c444e17b7b) )
	ROM_LOAD16_BYTE( "1.ic33", 0x00000, 0x20000, CRC(4e0e6eff) SHA1(51d422d95acec37a2249e9137ad6b92909747373) )
	ROM_LOAD16_BYTE( "4.ic29", 0x60001, 0x10000, CRC(31b741c5) SHA1(46c1c4cea09477cc4989f3e06e08851d02743e62) )
	ROM_RELOAD(                0xe0001, 0x10000 )
	ROM_LOAD16_BYTE( "2.ic32", 0x60000, 0x10000, CRC(b23e966c) SHA1(f506f6d1f4f7874070e91d1df8f141cca031ce29) )
	ROM_RELOAD(                0xe0000, 0x10000 )

	ROM_REGION( 0x20000, "soundcpu", 0 ) // on bottom PCB
	ROM_LOAD( "5.ic23", 0x00000, 0x10000, CRC(6857913a) SHA1(4143b80a59414e77bedf5d7230d32207300e7301) )
	ROM_LOAD( "6.ic22", 0x10000, 0x10000, CRC(812f94a2) SHA1(dc8be0e2db82d2b39c66dae68c28725371dd96c4) )

	ROM_REGION( 0x080000, "sprites", 0 ) // on top PCB, no labels
	ROM_LOAD( "ic1",  0x00000, 0x10000, CRC(a033002b) SHA1(d08a4b5ff704e8bb5c37c1285203265bdae8a0ec) )
	ROM_LOAD( "ic30", 0x10000, 0x10000, CRC(2f8438e9) SHA1(73a04303d9713cb92600b98180f4eb158021a426) )
	ROM_LOAD( "ic2",  0x20000, 0x10000, CRC(17d25f87) SHA1(b88ef27498993b596d587ea9bcdc149e0305be14) )
	ROM_LOAD( "ic31", 0x30000, 0x10000, CRC(f548c48f) SHA1(c782c73017dc5bd9b3ce41bec985a4ccc7b33275) )
	ROM_LOAD( "ic3",  0x40000, 0x10000, CRC(aa0256a0) SHA1(588a871ace051ac9c4e02c6777503eb8e040c587) )
	ROM_LOAD( "ic32", 0x50000, 0x10000, CRC(85143b72) SHA1(08c6247604e937c58785194a9d77b40af4827795) )
	ROM_LOAD( "ic4",  0x60000, 0x10000, CRC(2be70871) SHA1(2f3ba46cad67916d1067522fd1779119b71246c9) )
	ROM_LOAD( "ic33", 0x70000, 0x10000, CRC(eb0ae4a1) SHA1(66da41ad424498c3f58865dbc1de2256a45fd05a) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // on bottom PCB
	ROM_LOAD( "8.ic139",  0x00000, 0x10000, CRC(208796b3) SHA1(38b90732c8d5c77ee84053364a8a7e3daaaabe66) )
	ROM_LOAD( "7.ic140",  0x10000, 0x10000, CRC(f5f56b2a) SHA1(4ef6602052fa70e765d6d7747e672b7108b44f59) )
	ROM_LOAD( "10.ic137", 0x20000, 0x10000, CRC(b4a7f490) SHA1(851b40650fc8920b49f43f9cc6f19e845a25e945) )
	ROM_LOAD( "9.ic138",  0x30000, 0x10000, CRC(d194ea08) SHA1(0270897049cd256472df42f3dda856ee707535cd) )
	ROM_LOAD( "12.ic135", 0x40000, 0x10000, CRC(34fe8f7f) SHA1(fbf8839b26be55ad83ad4db538ba3e196c1ab945) )
	ROM_LOAD( "11.ic136", 0x50000, 0x10000, CRC(2b06bcc3) SHA1(36378a4a69f3c3da96d2dc8df48916af8de50009) )
	ROM_LOAD( "14.ic133", 0x60000, 0x10000, CRC(4b0e92f4) SHA1(16ad9220ca6708028cea18c1c4b57e2b6eb425b4) )
	ROM_LOAD( "13.ic134", 0x70000, 0x10000, CRC(94b96bfa) SHA1(33c1e9045e7a984097f3fe4954b20d954cffbafa) )

	ROM_REGION( 0x20000, "samples", ROMREGION_ERASEFF ) // -- no sample ROMs on bootleg, included with Z80 code
ROM_END

ROM_START( cosmccop )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cc_d-h0-b.ic55", 0x00001, 0x40000, CRC(38958b01) SHA1(7d7e217742e33a1fe096adf5bbc93d63ddcfb375) )
	ROM_RELOAD(                        0x80001, 0x40000 )
	ROM_LOAD16_BYTE( "cc_d-l0-b.ic61", 0x00000, 0x40000, CRC(eff87f70) SHA1(61f49b8738cf31546d4182680b761705274b01bf) )
	ROM_RELOAD(                        0x80000, 0x40000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "cc_d-sp-.ic17", 0x00000, 0x10000, CRC(3e3ace60) SHA1(d89b1b84de2887598bb7bcb17b1df1ec8d1862a9) )

	ROM_REGION( 0x080000, "sprites", 0 ) // sprites - located on M84-B-B top board
	ROM_LOAD( "cc_b-n0-.ic31", 0x00000, 0x20000, CRC(9d99deaa) SHA1(acf16bea0f482306107d2a305c568406b6c21e9a) )   // == cc-c-00.ic53
	ROM_LOAD( "cc_b-n1-.ic21", 0x20000, 0x20000, CRC(7eb083ed) SHA1(31fa7d532fd46e861c3d19d5b08661653f685a49) )   // == cc-c-10.ic51
	ROM_LOAD( "cc_b-n2-.ic32", 0x40000, 0x20000, CRC(9421489e) SHA1(e43d042bf8b4ebed93558d74ec479ec60a01ca5c) )   // == cc-c-20.ic49
	ROM_LOAD( "cc_b-n3-.ic22", 0x60000, 0x20000, CRC(920ec735) SHA1(2d0949b43dddce7317c45910d6e4868ddf010806) )   // == cc-c-30.ic47

	ROM_REGION( 0x080000, "gfx2", 0 )  // tiles - same data, different format as the gallopa set
	ROM_LOAD( "cc_d-g00-.ic51", 0x00000, 0x20000, CRC(e7f3d772) SHA1(c7f0bc42e8dde7bae334c7974c3d0ddba3856144) )   // == cc-b-a0.ic21 + cc-b-b0.ic26
	ROM_LOAD( "cc_d-g10-.ic57", 0x20000, 0x20000, CRC(418b4e4c) SHA1(1191f12741ee7a360240f706534c9c83be8d5c2d) )   // == cc-b-a1.ic22 + cc-b-b1.ic27
	ROM_LOAD( "cc_d-g20-.ic66", 0x40000, 0x20000, CRC(a4b558eb) SHA1(0babf725de0065dbeca73fa170bd33565305d129) )   // == cc-b-a2.ic20 + cc-b-b2.ic25
	ROM_LOAD( "cc_d-g30-.ic64", 0x60000, 0x20000, CRC(f64a3166) SHA1(1661db2a37c76e6b4552e48c04966dbbccab8926) )   // == cc-b-a3.ic23 + cc-b-b3.ic24

	ROM_REGION( 0x20000, "samples", 0 ) // samples
	ROM_LOAD( "cc_d-v0-.ic14", 0x00000, 0x20000, CRC(6247bade) SHA1(4bc9f86acd09908c74b1ab0e7817c4ff1cad6f0b) )   // == cc-c-v0.ic44

	ROM_REGION( 0x0200, "proms", 0 ) // located on M84-B-B top board
	ROM_LOAD( "ken_b-4n-.ic23", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10N
	ROM_LOAD( "ken_b-4p-.ic24", 0x0100, 0x0100, CRC(526f10ca) SHA1(e0ecd4db0720a4a37489e4d725843a2fbf266ebf) ) // TBP24S10N

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "ken_d-2h-.ic5",  0x0000, 0x0117, CRC(a83807e9) SHA1(3875d9881789756870721cab41a198c5af67a446) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "ken_d-5l-.ic33", 0x0200, 0x0117, CRC(c719b8a3) SHA1(dba75220c002ca4c75c578ed6c8c56dcfbb781ca) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "cc_d-7b-.ic45",  0x0400, 0x0117, CRC(75ff4517) SHA1(87ea8f44eaa7e317a066484b3cb5f20de1d80844) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "ken_b-3a-.ic9",  0x0600, 0x0117, CRC(ad1a7942) SHA1(72c699de17e3d65081a5951581af90aadb4ba65b) ) // TIBPAL-16L8-25 - bruteforced - located on M84-B-B top board
ROM_END


ROM_START( gallop )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cc_d-h0-.ic55", 0x00001, 0x40000, CRC(dac9eec3) SHA1(003762fb69e2766de5dab0690f7ecb289d667c8c) )
	ROM_RELOAD(                       0x80001, 0x40000 )
	ROM_LOAD16_BYTE( "cc_d-l0-.ic61", 0x00000, 0x40000, CRC(10e37ee1) SHA1(439d98b095249e4f5adcfef73e2d78189c94b739) )
	ROM_RELOAD(                       0x80000, 0x40000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "cc_d-sp-.ic17", 0x00000, 0x10000, CRC(3e3ace60) SHA1(d89b1b84de2887598bb7bcb17b1df1ec8d1862a9) )

	ROM_REGION( 0x080000, "sprites", 0 ) // sprites - located on M84-B-B top board
	ROM_LOAD( "cc_b-n0-.ic31", 0x00000, 0x20000, CRC(9d99deaa) SHA1(acf16bea0f482306107d2a305c568406b6c21e9a) )   // == cc-c-00.ic53
	ROM_LOAD( "cc_b-n1-.ic21", 0x20000, 0x20000, CRC(7eb083ed) SHA1(31fa7d532fd46e861c3d19d5b08661653f685a49) )   // == cc-c-10.ic51
	ROM_LOAD( "cc_b-n2-.ic32", 0x40000, 0x20000, CRC(9421489e) SHA1(e43d042bf8b4ebed93558d74ec479ec60a01ca5c) )   // == cc-c-20.ic49
	ROM_LOAD( "cc_b-n3-.ic22", 0x60000, 0x20000, CRC(920ec735) SHA1(2d0949b43dddce7317c45910d6e4868ddf010806) )   // == cc-c-30.ic47

	ROM_REGION( 0x080000, "gfx2", 0 )  // tiles - same data, different format as the gallopa set
	ROM_LOAD( "cc_d-g00-.ic51", 0x00000, 0x20000, CRC(e7f3d772) SHA1(c7f0bc42e8dde7bae334c7974c3d0ddba3856144) )   // == cc-b-a0.ic21 + cc-b-b0.ic26
	ROM_LOAD( "cc_d-g10-.ic57", 0x20000, 0x20000, CRC(418b4e4c) SHA1(1191f12741ee7a360240f706534c9c83be8d5c2d) )   // == cc-b-a1.ic22 + cc-b-b1.ic27
	ROM_LOAD( "cc_d-g20-.ic66", 0x40000, 0x20000, CRC(a4b558eb) SHA1(0babf725de0065dbeca73fa170bd33565305d129) )   // == cc-b-a2.ic20 + cc-b-b2.ic25
	ROM_LOAD( "cc_d-g30-.ic64", 0x60000, 0x20000, CRC(f64a3166) SHA1(1661db2a37c76e6b4552e48c04966dbbccab8926) )   // == cc-b-a3.ic23 + cc-b-b3.ic24

	ROM_REGION( 0x20000, "samples", 0 ) // samples
	ROM_LOAD( "cc_d-v0-.ic14", 0x00000, 0x20000, CRC(6247bade) SHA1(4bc9f86acd09908c74b1ab0e7817c4ff1cad6f0b) )   // == cc-c-v0.ic44

	ROM_REGION( 0x0200, "proms", 0 ) // located on M84-B-B top board
	ROM_LOAD( "ken_b-4n-.ic23", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10N
	ROM_LOAD( "ken_b-4p-.ic24", 0x0100, 0x0100, CRC(526f10ca) SHA1(e0ecd4db0720a4a37489e4d725843a2fbf266ebf) ) // TBP24S10N

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "ken_d-2h-.ic5",  0x0000, 0x0117, CRC(a83807e9) SHA1(3875d9881789756870721cab41a198c5af67a446) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "ken_d-5l-.ic33", 0x0200, 0x0117, CRC(c719b8a3) SHA1(dba75220c002ca4c75c578ed6c8c56dcfbb781ca) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "cc_d-7b-.ic45",  0x0400, 0x0117, CRC(75ff4517) SHA1(87ea8f44eaa7e317a066484b3cb5f20de1d80844) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "ken_b-3a-.ic9",  0x0600, 0x0117, CRC(ad1a7942) SHA1(72c699de17e3d65081a5951581af90aadb4ba65b) ) // TIBPAL-16L8-25 - bruteforced - located on M84-B-B top board
ROM_END


ROM_START( ltswords )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "h0.ic55", 0x00001, 0x20000, CRC(22f342b2) SHA1(8a0954eed7ad5a231a0e3884da28556c0f64f7f6) ) // EPROM with no label
	ROM_RELOAD(                 0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "l0.ic61", 0x00000, 0x20000, CRC(0210d592) SHA1(8500dd6da56c007878287d468b3ebc1686e59ef7) ) // EPROM with no label
	ROM_RELOAD(                 0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ken_d-sp-.ic17", 0x00000, 0x10000, CRC(233ca1cf) SHA1(4ebb6162773bd586a10016ccd77998a9b880f474) )

	ROM_REGION( 0x080000, "sprites", 0 ) // mask ROMs
	ROM_LOAD( "ken_m31.ic31",  0x00000, 0x20000, CRC(e00b95a6) SHA1(6efcd8d58f8ebe3a42c60a0aa790b42c0e132777) )  // sprites
	ROM_LOAD( "ken_m21.ic21",  0x20000, 0x20000, CRC(d7722f87) SHA1(8606a53b8630934d2b5dfc986bd92ac4142f67e2) )
	ROM_LOAD( "ken_m32.ic32",  0x40000, 0x20000, CRC(30a844c4) SHA1(72b2caba3ee7a229ca56f004516dea8d3f0a7ba6) )
	ROM_LOAD( "ken_m22.ic22",  0x60000, 0x20000, CRC(a00dac85) SHA1(0c1ed852795046926f62843f6b256cbeecf9ebcf) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // mask ROMs
	ROM_LOAD( "ken_m51.ic51",  0x00000, 0x20000, CRC(1646cf4f) SHA1(d240cb2bad3e766128e8e40aa7b1bf4f3b9a5559) )  // tiles
	ROM_LOAD( "ken_m57.ic57",  0x20000, 0x20000, CRC(a9f88d90) SHA1(c8d4a96fe55fed4b7499550f3c74b03d10306757) )
	ROM_LOAD( "ken_m66.ic66",  0x40000, 0x20000, CRC(e9d17645) SHA1(fbe18d6691686a1c458d4a91169c9850698b5ca7) )
	ROM_LOAD( "ken_m64.ic64",  0x60000, 0x20000, CRC(df46709b) SHA1(e7c2cd752e765bf7b8ff24637305d61031ce0baa) )

	ROM_REGION( 0x20000, "samples", 0 ) // mask ROM
	ROM_LOAD( "ken_m14.ic14",  0x00000, 0x20000, CRC(6651e9b7) SHA1(c42009f986c9a9f35732d5cd717d548536469b1c) )  // samples

	ROM_REGION( 0x0200, "proms", 0 ) // located on M84-B-B top board
	ROM_LOAD( "ken_b-4n-.ic23", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10N
	ROM_LOAD( "ken_b-4p-.ic24", 0x0100, 0x0100, CRC(526f10ca) SHA1(e0ecd4db0720a4a37489e4d725843a2fbf266ebf) ) // TBP24S10N

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "ken_d-2h-.ic5",  0x0000, 0x0117, CRC(a83807e9) SHA1(3875d9881789756870721cab41a198c5af67a446) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "ken_d-5l-.ic33", 0x0200, 0x0117, CRC(c719b8a3) SHA1(dba75220c002ca4c75c578ed6c8c56dcfbb781ca) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "ken_d-7b-.ic45", 0x0400, 0x0117, CRC(b0558dc5) SHA1(e95d9a8ddc49f99062439803c3359fca6fecf703) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "ken_b-3a-.ic9",  0x0600, 0x0117, CRC(ad1a7942) SHA1(72c699de17e3d65081a5951581af90aadb4ba65b) ) // TIBPAL-16L8-25 - bruteforced - located on M84-B-B top board
ROM_END

ROM_START( kengo )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ken_d-h0-c.ic55", 0x00001, 0x20000, CRC(f4ddeea5) SHA1(bcf016e40886e11c171f2f50de39ac0d8cabcdd1) ) // no regional 'For use in ...' message
	ROM_RELOAD(                         0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "ken_d-l0-c.ic61", 0x00000, 0x20000, CRC(04dc0f81) SHA1(b296529f0bc26d53b344449dfa5a08eca70f30d8) )
	ROM_RELOAD(                         0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ken_d-sp-.ic17", 0x00000, 0x10000, CRC(233ca1cf) SHA1(4ebb6162773bd586a10016ccd77998a9b880f474) )

	ROM_REGION( 0x080000, "sprites", 0 ) // mask ROMs
	ROM_LOAD( "ken_m31.ic31",  0x00000, 0x20000, CRC(e00b95a6) SHA1(6efcd8d58f8ebe3a42c60a0aa790b42c0e132777) )  // sprites
	ROM_LOAD( "ken_m21.ic21",  0x20000, 0x20000, CRC(d7722f87) SHA1(8606a53b8630934d2b5dfc986bd92ac4142f67e2) )
	ROM_LOAD( "ken_m32.ic32",  0x40000, 0x20000, CRC(30a844c4) SHA1(72b2caba3ee7a229ca56f004516dea8d3f0a7ba6) )
	ROM_LOAD( "ken_m22.ic22",  0x60000, 0x20000, CRC(a00dac85) SHA1(0c1ed852795046926f62843f6b256cbeecf9ebcf) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // mask ROMs
	ROM_LOAD( "ken_m51.ic51",  0x00000, 0x20000, CRC(1646cf4f) SHA1(d240cb2bad3e766128e8e40aa7b1bf4f3b9a5559) )  // tiles
	ROM_LOAD( "ken_m57.ic57",  0x20000, 0x20000, CRC(a9f88d90) SHA1(c8d4a96fe55fed4b7499550f3c74b03d10306757) )
	ROM_LOAD( "ken_m66.ic66",  0x40000, 0x20000, CRC(e9d17645) SHA1(fbe18d6691686a1c458d4a91169c9850698b5ca7) )
	ROM_LOAD( "ken_m64.ic64",  0x60000, 0x20000, CRC(df46709b) SHA1(e7c2cd752e765bf7b8ff24637305d61031ce0baa) )

	ROM_REGION( 0x20000, "samples", 0 ) // mask ROM
	ROM_LOAD( "ken_m14.ic14",  0x00000, 0x20000, CRC(6651e9b7) SHA1(c42009f986c9a9f35732d5cd717d548536469b1c) )  // samples

	ROM_REGION( 0x0200, "proms", 0 ) // located on M84-B-B top board
	ROM_LOAD( "ken_b-4n-.ic23", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10N
	ROM_LOAD( "ken_b-4p-.ic24", 0x0100, 0x0100, CRC(526f10ca) SHA1(e0ecd4db0720a4a37489e4d725843a2fbf266ebf) ) // TBP24S10N

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "ken_d-2h-.ic5",  0x0000, 0x0117, CRC(a83807e9) SHA1(3875d9881789756870721cab41a198c5af67a446) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "ken_d-5l-.ic33", 0x0200, 0x0117, CRC(c719b8a3) SHA1(dba75220c002ca4c75c578ed6c8c56dcfbb781ca) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "ken_d-7b-.ic45", 0x0400, 0x0117, CRC(b0558dc5) SHA1(e95d9a8ddc49f99062439803c3359fca6fecf703) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "ken_b-3a-.ic9",  0x0600, 0x0117, CRC(ad1a7942) SHA1(72c699de17e3d65081a5951581af90aadb4ba65b) ) // TIBPAL-16L8-25 - bruteforced - located on M84-B-B top board
ROM_END

ROM_START( kengoj )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ken_d-h0-.ic55", 0x00001, 0x20000, CRC(ed3da88c) SHA1(536824eb3347eade2d3aad927e83eae51ee852b3) ) // shows 'For use in Japan' message
	ROM_RELOAD(                        0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "ken_d-l0-.ic61", 0x00000, 0x20000, CRC(92c57d8e) SHA1(eb078a7b261e13cfb0a920b5115beee917b8d89c) )
	ROM_RELOAD(                        0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ken_d-sp.ic17", 0x00000, 0x10000, CRC(233ca1cf) SHA1(4ebb6162773bd586a10016ccd77998a9b880f474) )

	ROM_REGION( 0x080000, "sprites", 0 ) // mask ROMs
	ROM_LOAD( "ken_m31.ic31",  0x00000, 0x20000, CRC(e00b95a6) SHA1(6efcd8d58f8ebe3a42c60a0aa790b42c0e132777) )  // sprites
	ROM_LOAD( "ken_m21.ic21",  0x20000, 0x20000, CRC(d7722f87) SHA1(8606a53b8630934d2b5dfc986bd92ac4142f67e2) )
	ROM_LOAD( "ken_m32.ic32",  0x40000, 0x20000, CRC(30a844c4) SHA1(72b2caba3ee7a229ca56f004516dea8d3f0a7ba6) )
	ROM_LOAD( "ken_m22.ic22",  0x60000, 0x20000, CRC(a00dac85) SHA1(0c1ed852795046926f62843f6b256cbeecf9ebcf) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // mask ROMs
	ROM_LOAD( "ken_m51.ic51",  0x00000, 0x20000, CRC(1646cf4f) SHA1(d240cb2bad3e766128e8e40aa7b1bf4f3b9a5559) )  // tiles
	ROM_LOAD( "ken_m57.ic57",  0x20000, 0x20000, CRC(a9f88d90) SHA1(c8d4a96fe55fed4b7499550f3c74b03d10306757) )
	ROM_LOAD( "ken_m66.ic66",  0x40000, 0x20000, CRC(e9d17645) SHA1(fbe18d6691686a1c458d4a91169c9850698b5ca7) )
	ROM_LOAD( "ken_m64.ic64",  0x60000, 0x20000, CRC(df46709b) SHA1(e7c2cd752e765bf7b8ff24637305d61031ce0baa) )

	ROM_REGION( 0x20000, "samples", 0 ) // mask ROMs
	ROM_LOAD( "ken_m14.ic14",  0x00000, 0x20000, CRC(6651e9b7) SHA1(c42009f986c9a9f35732d5cd717d548536469b1c) )  // samples

	ROM_REGION( 0x0200, "proms", 0 ) // located on M84-B-B top board
	ROM_LOAD( "ken_b-4n-.ic23", 0x0000, 0x0100, CRC(b460c438) SHA1(00e20cf754b6fd5138ee4d2f6ec28dff9e292fe6) ) // TBP24S10N
	ROM_LOAD( "ken_b-4p-.ic24", 0x0100, 0x0100, CRC(526f10ca) SHA1(e0ecd4db0720a4a37489e4d725843a2fbf266ebf) ) // TBP24S10N

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "ken_d-2h-.ic5",  0x0000, 0x0117, CRC(a83807e9) SHA1(3875d9881789756870721cab41a198c5af67a446) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "ken_d-5l-.ic33", 0x0200, 0x0117, CRC(c719b8a3) SHA1(dba75220c002ca4c75c578ed6c8c56dcfbb781ca) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "ken_d-7b-.ic45", 0x0400, 0x0117, CRC(b0558dc5) SHA1(e95d9a8ddc49f99062439803c3359fca6fecf703) ) // TIBPAL-16L8-25 - bruteforced
	ROM_LOAD( "ken_b-3a-.ic9",  0x0600, 0x0117, CRC(ad1a7942) SHA1(72c699de17e3d65081a5951581af90aadb4ba65b) ) // TIBPAL-16L8-25 - bruteforced - located on M84-B-B top board
ROM_END


/*****************************
  M85 sets
******************************/

ROM_START( poundfor )
	ROM_REGION( 0x100000, "maincpu", 0 ) // Located on M85-A-B CPU/Sound board
	ROM_LOAD16_BYTE( "pp-a-h0-b.9e", 0x00001, 0x20000, CRC(50d4a2d8) SHA1(7fd62c6613cb58b512c6c3670fa66a5b9906e6a1) )
	ROM_LOAD16_BYTE( "pp-a-l0-b.9d", 0x00000, 0x20000, CRC(bd997942) SHA1(da484afe3b79e09e323c768a0b2165e6283971a7) )
	ROM_LOAD16_BYTE( "pp-a-h1-.9f",  0x40001, 0x20000, CRC(f6c82f48) SHA1(b38a2f9f0f6439b2cf453fec87ca11d959777ee6) )
	ROM_RELOAD(                      0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "pp-a-l1-.9c",  0x40000, 0x20000, CRC(5b07b087) SHA1(04a2403eb8c443cb92b880edc612542acdbcafa4) )
	ROM_RELOAD(                      0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 ) // Located on M85-A-B CPU/Sound board
	ROM_LOAD( "pp-a-sp.4j",    0x00000, 0x10000, CRC(3f458a5b) SHA1(d73740b2a548bf8a895909da0841f18d9ed32668) )

	ROM_REGION( 0x100000, "sprites", 0 ) // located on M85-B top board
	ROM_LOAD( "ppb-n0.ic21", 0x00000, 0x40000, CRC(951a41f8) SHA1(59b64f63ea2452c2b42ff7ebf1ff6fc4e7879ce3) )  // sprites
	ROM_LOAD( "ppb-n1.ic22", 0x40000, 0x40000, CRC(c609b7f2) SHA1(1da3550c7e4d2a26d75d143934680d9177ba5c35) )
	ROM_LOAD( "ppb-n2.ic35", 0x80000, 0x40000, CRC(318c0b5f) SHA1(1d4cd17dc2f8fc4e523eaf679f21d83e1bfade4e) )
	ROM_LOAD( "ppb-n3.ic10", 0xc0000, 0x40000, CRC(93dc9490) SHA1(3df4d57a7bf19443f5aa6a416bcee968f81d9059) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // Located on M85-A-B CPU/Sound board
	ROM_LOAD( "ppa-g00.ic19", 0x00000, 0x20000, CRC(8a88a174) SHA1(d360b9014aec31960538ee488894496248a820dc) )  // tiles
	ROM_LOAD( "ppa-g10.ic11", 0x20000, 0x20000, CRC(e48a66ac) SHA1(49b33db6a922d6f1d1417e28714a67431b7c0217) )
	ROM_LOAD( "ppa-g20.ic18", 0x40000, 0x20000, CRC(12b93e79) SHA1(f3d2b76a30874827c8998c1d13a55a3990b699b7) )
	ROM_LOAD( "ppa-g30.ic10", 0x60000, 0x20000, CRC(faa39aee) SHA1(9cc1a468b304437766c04189054d3b8f7ff1f958) )

	ROM_REGION( 0x40000, "samples", 0 ) // Located on M85-A-B CPU/Sound board
	ROM_LOAD( "ppa-v0.ic6",   0x00000, 0x40000, CRC(03321664) SHA1(51f2b2b712385c1cd55fd069829efac01838d603) )  // samples

	ROM_REGION( 0x0200, "proms", 0 ) // proms - located on M85-B top board
	ROM_LOAD( "m85_b-1f-.ic5",  0x0000, 0x0100, NO_DUMP ) // TBP24S10
	ROM_LOAD( "m85_b-3f-.ic12", 0x0100, 0x0100, NO_DUMP ) // TBP24S10

	ROM_REGION( 0x0600, "pals", 0 )
	ROM_LOAD( "m85_a-5h-.5h",  0x0000, 0x0117, CRC(a7ce2e57) SHA1(7af157d0ffb3001c3066d4abb1e3f731744243cb) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m85_a-6j-.6j",  0x0200, 0x0117, CRC(733ed0f9) SHA1(863055a2b13825a900095b18aad3829faef0f79e) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m85_b-1a-.ic1", 0x0400, 0x0117, CRC(3cf26744) SHA1(b367e142b24695aa42d2a6634d66072aad925614) ) // PAL16L8 - bruteforced - located on M85-B  top board
ROM_END

ROM_START( poundforj )
	ROM_REGION( 0x100000, "maincpu", 0 ) // Located on M85-A-B CPU/Sound board
	ROM_LOAD16_BYTE( "pp-a-h0-.9e", 0x00001, 0x20000, CRC(f0165e3b) SHA1(a0482b34c0d05d8f48d1b16f2bc2d5d9ec465dc8) )
	ROM_LOAD16_BYTE( "pp-a-l0-.9d", 0x00000, 0x20000, CRC(f954f99f) SHA1(6e7a9718dc63e595403bfc0f1ceae4a71dc75133) )
	ROM_LOAD16_BYTE( "pp-a-h1-.9f", 0x40001, 0x20000, CRC(f6c82f48) SHA1(b38a2f9f0f6439b2cf453fec87ca11d959777ee6) )
	ROM_RELOAD(                     0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "pp-a-l1-.9c", 0x40000, 0x20000, CRC(5b07b087) SHA1(04a2403eb8c443cb92b880edc612542acdbcafa4) )
	ROM_RELOAD(                     0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 ) // Located on M85-A-B CPU/Sound board
	ROM_LOAD( "pp-a-sp.4j",    0x00000, 0x10000, CRC(3f458a5b) SHA1(d73740b2a548bf8a895909da0841f18d9ed32668) )

	ROM_REGION( 0x100000, "sprites", 0 ) // located on M85-B top board
	ROM_LOAD( "ppb-n0.ic21", 0x00000, 0x40000, CRC(951a41f8) SHA1(59b64f63ea2452c2b42ff7ebf1ff6fc4e7879ce3) )  // sprites
	ROM_LOAD( "ppb-n1.ic22", 0x40000, 0x40000, CRC(c609b7f2) SHA1(1da3550c7e4d2a26d75d143934680d9177ba5c35) )
	ROM_LOAD( "ppb-n2.ic35", 0x80000, 0x40000, CRC(318c0b5f) SHA1(1d4cd17dc2f8fc4e523eaf679f21d83e1bfade4e) )
	ROM_LOAD( "ppb-n3.ic10", 0xc0000, 0x40000, CRC(93dc9490) SHA1(3df4d57a7bf19443f5aa6a416bcee968f81d9059) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // Located on M85-A-B CPU/Sound board
	ROM_LOAD( "ppa-g00.ic19", 0x00000, 0x20000, CRC(8a88a174) SHA1(d360b9014aec31960538ee488894496248a820dc) )  // tiles
	ROM_LOAD( "ppa-g10.ic11", 0x20000, 0x20000, CRC(e48a66ac) SHA1(49b33db6a922d6f1d1417e28714a67431b7c0217) )
	ROM_LOAD( "ppa-g20.ic18", 0x40000, 0x20000, CRC(12b93e79) SHA1(f3d2b76a30874827c8998c1d13a55a3990b699b7) )
	ROM_LOAD( "ppa-g30.ic10", 0x60000, 0x20000, CRC(faa39aee) SHA1(9cc1a468b304437766c04189054d3b8f7ff1f958) )

	ROM_REGION( 0x40000, "samples", 0 ) // Located on M85-A-B CPU/Sound board
	ROM_LOAD( "ppa-v0.ic6",   0x00000, 0x40000, CRC(03321664) SHA1(51f2b2b712385c1cd55fd069829efac01838d603) )  // samples

	ROM_REGION( 0x0200, "proms", 0 ) // proms - located on M85-B  top board
	ROM_LOAD( "m85_b-1f-.ic5",  0x0000, 0x0100, NO_DUMP ) // TBP24S10
	ROM_LOAD( "m85_b-3f-.ic12", 0x0100, 0x0100, NO_DUMP ) // TBP24S10

	ROM_REGION( 0x0600, "pals", 0 )
	ROM_LOAD( "m85_a-5h-.5h",  0x0000, 0x0117, CRC(a7ce2e57) SHA1(7af157d0ffb3001c3066d4abb1e3f731744243cb) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m85_a-6j-.6j",  0x0200, 0x0117, CRC(733ed0f9) SHA1(863055a2b13825a900095b18aad3829faef0f79e) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m85_b-1a-.ic1", 0x0400, 0x0117, CRC(3cf26744) SHA1(b367e142b24695aa42d2a6634d66072aad925614) ) // PAL16L8 - bruteforced - located on M85-B  top board
ROM_END

ROM_START( poundforu )
	ROM_REGION( 0x100000, "maincpu", 0 ) // Located on M85-A-B CPU/Sound board
	ROM_LOAD16_BYTE( "pp-a-h0-a.9e", 0x00001, 0x20000, CRC(ff4c83a4) SHA1(1b7791c784bf7c4774e3200b76d65ab0bf0ff93b) )
	ROM_LOAD16_BYTE( "pp-a-l0-a.9d", 0x00000, 0x20000, CRC(3374ce8f) SHA1(7455f8339aeed0ef3d0567baa804b62ca3615283) )
	ROM_LOAD16_BYTE( "pp-a-h1-.9f",  0x40001, 0x20000, CRC(f6c82f48) SHA1(b38a2f9f0f6439b2cf453fec87ca11d959777ee6) )
	ROM_RELOAD(                      0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "pp-a-l1-.9c",  0x40000, 0x20000, CRC(5b07b087) SHA1(04a2403eb8c443cb92b880edc612542acdbcafa4) )
	ROM_RELOAD(                      0xc0000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 ) // Located on M85-A-B CPU/Sound board
	ROM_LOAD( "pp-a-sp.4j",    0x00000, 0x10000, CRC(3f458a5b) SHA1(d73740b2a548bf8a895909da0841f18d9ed32668) )

	ROM_REGION( 0x100000, "sprites", 0 ) // located on M85-B top board
	ROM_LOAD( "ppb-n0.ic21", 0x00000, 0x40000, CRC(951a41f8) SHA1(59b64f63ea2452c2b42ff7ebf1ff6fc4e7879ce3) )  // sprites
	ROM_LOAD( "ppb-n1.ic22", 0x40000, 0x40000, CRC(c609b7f2) SHA1(1da3550c7e4d2a26d75d143934680d9177ba5c35) )
	ROM_LOAD( "ppb-n2.ic35", 0x80000, 0x40000, CRC(318c0b5f) SHA1(1d4cd17dc2f8fc4e523eaf679f21d83e1bfade4e) )
	ROM_LOAD( "ppb-n3.ic10", 0xc0000, 0x40000, CRC(93dc9490) SHA1(3df4d57a7bf19443f5aa6a416bcee968f81d9059) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // Located on M85-A-B CPU/Sound board
	ROM_LOAD( "ppa-g00.ic19", 0x00000, 0x20000, CRC(8a88a174) SHA1(d360b9014aec31960538ee488894496248a820dc) )  // tiles
	ROM_LOAD( "ppa-g10.ic11", 0x20000, 0x20000, CRC(e48a66ac) SHA1(49b33db6a922d6f1d1417e28714a67431b7c0217) )
	ROM_LOAD( "ppa-g20.ic18", 0x40000, 0x20000, CRC(12b93e79) SHA1(f3d2b76a30874827c8998c1d13a55a3990b699b7) )
	ROM_LOAD( "ppa-g30.ic10", 0x60000, 0x20000, CRC(faa39aee) SHA1(9cc1a468b304437766c04189054d3b8f7ff1f958) )

	ROM_REGION( 0x40000, "samples", 0 ) // Located on M85-A-B CPU/Sound board
	ROM_LOAD( "ppa-v0.ic6",   0x00000, 0x40000, CRC(03321664) SHA1(51f2b2b712385c1cd55fd069829efac01838d603) )  // samples

	ROM_REGION( 0x0200, "proms", 0 ) // proms - located on M85-B daughterboard
	ROM_LOAD( "m85_b-1f-.ic5",  0x0000, 0x0100, NO_DUMP ) // TBP24S10
	ROM_LOAD( "m85_b-3f-.ic12", 0x0100, 0x0100, NO_DUMP ) // TBP24S10

	ROM_REGION( 0x0600, "pals", 0 )
	ROM_LOAD( "m85_a-5h-.5h",  0x0000, 0x0117, CRC(a7ce2e57) SHA1(7af157d0ffb3001c3066d4abb1e3f731744243cb) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m85_a-6j-.6j",  0x0200, 0x0117, CRC(733ed0f9) SHA1(863055a2b13825a900095b18aad3829faef0f79e) ) // PAL16L8 - bruteforced
	ROM_LOAD( "m85_b-1a-.ic1", 0x0400, 0x0117, CRC(3cf26744) SHA1(b367e142b24695aa42d2a6634d66072aad925614) ) // PAL16L8 - bruteforced - located on M85-B daughterboard
ROM_END

// For i8751 protected games, each region uses unique internal MCU code. The MCU code provides checksum information
// needed for each specific set. Using the wrong MCU code will result in the program ROMs failing the checksum tests.
// Simulation code is used for the few remaining games without a corresponding MCU dump.  See notes next to the sets

/* M72 */
GAME( 1987, rtype,       0,        rtype,        rtype,        m72_state, empty_init,      ROT0,   "Irem", "R-Type (World)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1987, rtypej,      rtype,    rtype,        rtype,        m72_state, empty_init,      ROT0,   "Irem", "R-Type (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1987, rtypejp,     rtype,    rtype,        rtypep,       m72_state, empty_init,      ROT0,   "Irem", "R-Type (Japan prototype)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1987, rtypeu,      rtype,    rtype,        rtype,        m72_state, empty_init,      ROT0,   "Irem (Nintendo of America license)", "R-Type (US)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1987, rtypeb,      rtype,    rtype,        rtype,        m72_state, empty_init,      ROT0,   "bootleg", "R-Type (World bootleg)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1987, bchopper,    0,        mrheli,       bchopper,     m72_state, init_m72_8751,   ROT0,   "Irem", "Battle Chopper (World)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1987, mrheli,      bchopper, mrheli,       bchopper,     m72_state, init_m72_8751,   ROT0,   "Irem", "Mr. HELI no Daibouken (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1988, nspirit,     0,        m72,          nspirit,      m72_state, init_nspirit,    ROT0,   "Irem", "Ninja Spirit (World)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // missing i8751 MCU code
GAME( 1988, nspiritj,    nspirit,  nspiritj,     nspirit,      m72_state, init_m72_8751,   ROT0,   "Irem", "Saigo no Nindou (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1988, imgfight,    0,        imgfight,     imgfight,     m72_state, init_m72_8751,   ROT270, "Irem", "Image Fight (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, imgfightj,   imgfight, imgfight,     imgfight,     m72_state, init_m72_8751,   ROT270, "Irem", "Image Fight (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, imgfightjb,  imgfight, imgfightjb,   imgfight,     m72_state, init_m72_8751,   ROT270, "Irem", "Image Fight (Japan, bootleg)", MACHINE_SUPPORTS_SAVE ) // uses an 80c31 MCU

GAME( 1989, loht,        0,        m72_8751,     loht,         m72_state, init_m72_8751,   ROT0,   "Irem", "Legend of Hero Tonma (World)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1989, lohtj,       loht,     m72_8751,     loht,         m72_state, init_m72_8751,   ROT0,   "Irem", "Legend of Hero Tonma (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1989, lohtb2,      loht,     m72_8751,     loht,         m72_state, init_m72_8751,   ROT0,   "bootleg", "Legend of Hero Tonma (Japan, bootleg with i8751)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // works like above, mcu code is the same as the real code, probably just an alt revision on a bootleg board
GAME( 1997, lohtb3,      loht,     m72_8751,     loht,         m72_state, init_m72_8751,   ROT0,   "bootleg", "Legend of Hero Tonma (World, bootleg with i8751)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1989, xmultiplm72, xmultipl, m72_xmultipl, xmultipl,     m72_state, init_m72_8751,   ROT0,   "Irem", "X Multiply (Japan, M72 hardware)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1989, dbreedm72,   dbreed,   m72_dbreedw,  dbreed,       m72_state, init_dbreedm72,  ROT0,   "Irem", "Dragon Breed (World, M72 hardware)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // missing i8751 MCU code
GAME( 1989, dbreedjm72,  dbreed,   m72_dbreed,   dbreed,       m72_state, init_m72_8751,   ROT0,   "Irem", "Dragon Breed (Japan, M72 hardware)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1991, gallopm72,   cosmccop, m72_airduel,  gallop,       m72_state, init_m72_8751,   ROT0,   "Irem", "Gallop - Armed Police Unit (Japan, M72 hardware)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1990, airduelm72,  airduel,  m72_airduel,  airduel,      m72_state, init_m72_8751,   ROT270, "Irem", "Air Duel (World, M72 hardware)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, airdueljm72, airduel,  m72_airduel,  airduel,      m72_state, init_m72_8751,   ROT270, "Irem", "Air Duel (Japan, M72 hardware)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, dkgensanm72, hharry,   m72,          hharry,       m72_state, init_dkgenm72,   ROT0,   "Irem", "Daiku no Gensan (Japan, M72 hardware)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // missing i8751 MCU code

/* M81 */
GAME( 1989, xmultipl,    0,        m81_xmultipl, m81_xmultipl, m72_state, empty_init,      ROT0,   "Irem", "X Multiply (World, M81 hardware)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1989, dbreed,      0,        m81_dbreed,   m81_dbreed,   m72_state, empty_init,      ROT0,   "Irem", "Dragon Breed (World, M81 hardware)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, hharry,      0,        m81_hharry,   m81_hharry,   m72_state, empty_init,      ROT0,   "Irem", "Hammerin' Harry (World, M81 hardware)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

/* M82 */
GAME( 1990, majtitle,    0,        m82,          rtype2,       m72_state, empty_init,      ROT0,   "Irem", "Major Title (World)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // M82-A-A + M82-B-A
GAME( 1990, majtitlej,   majtitle, m82,          rtype2,       m72_state, empty_init,      ROT0,   "Irem", "Major Title (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // ^

GAME( 1990, airduel,     0,        m82,          airduel,      m72_state, empty_init,      ROT270, "Irem", "Air Duel (World, M82 hardware)", MACHINE_SUPPORTS_SAVE ) // Major Title conversion
GAME( 1990, airduelu,    airduel,  m82,          airduel,      m72_state, empty_init,      ROT270, "Irem America", "Air Duel (US location test, M82 hardware)", MACHINE_SUPPORTS_SAVE ) // ^

GAME( 1990, dkgensanm82, hharry,   hharryu,      hharry,       m72_state, empty_init,      ROT0,   "Irem", "Daiku no Gensan (Japan, M82 hardware)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // ^

GAME( 2009, rtypem82b,   rtype,    m82,          rtype,        m72_state, empty_init,      ROT0,   "bootleg", "R-Type (Japan, bootleg M82 conversion)", MACHINE_NOT_WORKING | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // unofficial conversion of Major Title, extensive wiremods, made in 2009 by Paul Swan
GAME( 1997, rtype2m82b,  rtype2,   m82,          rtype2,       m72_state, empty_init,      ROT0,   "bootleg", "R-Type II (Japan, bootleg M82 conversion)", MACHINE_NOT_WORKING | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // made in 1997 by Chris Hardy

/* M84 */
GAME( 1990, hharryu,     hharry,   hharryu,      hharry,       m72_state, empty_init,      ROT0,   "Irem America", "Hammerin' Harry (US, M84 hardware)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, dkgensan,    hharry,   hharryu,      hharry,       m72_state, empty_init,      ROT0,   "Irem", "Daiku no Gensan (Japan, M84 hardware)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, hharryb,     hharry,   hharryu,      hharry,       m72_state, empty_init,      ROT0,   "bootleg", "Hammerin' Harry (World, M84 hardware bootleg)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1989, rtype2,      0,        rtype2,       rtype2,       m72_state, empty_init,      ROT0,   "Irem", "R-Type II (World)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1989, rtype2j,     rtype2,   rtype2,       rtype2,       m72_state, empty_init,      ROT0,   "Irem", "R-Type II (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1989, rtype2jc,    rtype2,   rtype2,       rtype2,       m72_state, empty_init,      ROT0,   "Irem", "R-Type II (Japan, revision C)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1991, cosmccop,    0,        cosmccop,     gallop,       m72_state, empty_init,      ROT0,   "Irem", "Cosmic Cop (World)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1991, gallop,      cosmccop, cosmccop,     gallop,       m72_state, empty_init,      ROT0,   "Irem", "Gallop - Armed Police Unit (Japan, M84 hardware)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1991, ltswords,    0,        kengo,        kengo,        m72_state, empty_init,      ROT0,   "Irem", "Lightning Swords (World)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1991, kengo,       ltswords, kengo,        kengo,        m72_state, empty_init,      ROT0,   "Irem", "Ken-Go (World)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1991, kengoj,      ltswords, kengo,        kengo,        m72_state, empty_init,      ROT0,   "Irem", "Ken-Go (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // has 'for use in Japan' message, above set doesn't

/* M85 */
GAME( 1990, poundfor,    0,        poundfor,     poundfor,     m72_state, empty_init,      ROT270, "Irem", "Pound for Pound (World)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )      // M85-A-B / M85-B
GAME( 1990, poundforj,   poundfor, poundfor,     poundfor,     m72_state, empty_init,      ROT270, "Irem", "Pound for Pound (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )      // ^
GAME( 1990, poundforu,   poundfor, poundfor,     poundfor,     m72_state, empty_init,      ROT270, "Irem America", "Pound for Pound (US)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // ^

/* bootlegs, unique hw */
GAME( 1989, lohtb,       loht,     lohtb,        loht,         m72_state, empty_init,      ROT0,   "bootleg (Playmark)", "Legend of Hero Tonma (Playmark unprotected bootleg)", MACHINE_NOT_WORKING | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1989, loht_ms,     loht,     lohtb,        loht,         m72_state, empty_init,      ROT0,   "bootleg (Gaelco / Ervisa)", "Legend of Hero Tonma (Gaelco bootleg, Modular System)", MACHINE_NOT_WORKING | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, hharryb2,    hharry,   hharryu,      hharry,       m72_state, empty_init,      ROT0,   "bootleg (Playmark)", "Hammerin' Harry (Playmark bootleg)", MACHINE_NOT_WORKING | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
