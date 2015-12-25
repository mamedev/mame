// license:BSD-3-Clause
// copyright-holders:Philip Bennett, Anonymous
/******************************************************************************

    Bell-Fruit Cobra I/II and Viper Hardware

    driver by Phil Bennett and Anonymous

    Games supported:
        * A Question of Sport [2 sets]
        * Beeline (non-working - missing disk)
        * Every Second Counts
        * Inquizitor (Viper hardware, non-working - missing disk)
        * Quizvaders
        * Treble Top

    Other games on this hardware:
        * Brain Box
        * Quintoon alt. version (Cobra II/Cyclone hardware)

    Notes:

    The hardware is based on a chipset known as 'Flare One', developed
    by Flare Technology. It consists of a 16-bit DSP (intended for sound
    synthesis and 3D maths), an 8bpp blitter and a video controller, driven
    by a Z80.

    Flare One would evolve to become 'Slipstream', used by the unreleased
    Konix Multisystem console.

    The Flare One chipset is implemented as four Texas Instruments ASICs,
    each an 84 pin PLCC package:

    CF30204, CF30205, CF30206 (DSP) and CF30207.

    The hardware using this chipset is as follows:

    Viper
    =====

    A video expansion PCB for Scorpion I?
    On some PCB revisions there is audio output circuitry connected to the DSP.
    Viper uses a WD1772 type floppy disk controller.

    Cobra I
    =======

    A combination of Viper and Scorpion I hardware on a single PCB.
    Cobra uses an NEC '765 type FDC. Later revisions have no DSP.

    Cobra II (Cyclone)
    ==================

    A compact video expansion board for Scorpion II.
    The Z80 is replaced by a Z180 and there is no Flare DSP or FDC


    To do:

    * Complete blitter emulation
    * Cobra II support
    * Hook up additional inputs, EM meters, lamps etc

    Known issues:

    * All games bar qos: NVRAM not saved

    * Viper does not have a colour palette - the Flare chipset drives RGB direct.
      To fix this I set default values in the palette when the machine is initialised
    * CPU execution rate is wrong, the hardware adds 1 TCycle to each access which is unaccounted for.
    * Plane priority is probably wrong but it's only used in Treble Top.
    * Blitter loop counts and step are wrong - they are 9 bit counts, not 8.
    * Blitter emulation doesn't support hi-res mode (needed for Inquizitor)

******************************************************************************/

#include "emu.h"
#include "machine/clock.h"
#include "machine/6850acia.h"
#include "machine/meters.h"
#include "cpu/z80/z80.h"
#include "cpu/m6809/m6809.h"
#include "sound/upd7759.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"


/*
    Defines
*/
#define Z80_XTAL    5910000     /* Unconfirmed */
#define M6809_XTAL  1000000



/***************************************************************************

    Split into video\cobra.c !

***************************************************************************/

union ADDR_REG
{
#ifdef LSB_FIRST
	struct { UINT16 loword, hiword ; } as16bit;
	struct { UINT8 addr0, addr1, addr2; } as8bit;
#else
	struct { UINT16 hiword, loword ; } as16bit;
	struct { UINT8 addr2, addr1, addr0; } as8bit;
#endif
	UINT32 addr;
};

/* Blitter register flag bits */
#define CMD_RUN         0x01
#define CMD_COLST       0x02
#define CMD_PARRD       0x04        /* Never used? */
#define CMD_SRCUP       0x08
#define CMD_DSTUP       0x10
#define CMD_LT0         0x20
#define CMD_LT1         0x40
#define CMD_LINEDRAW    0x80


/* All unconfirmed */
//#define SRCDST_CMP    0x10
//#define SRCDST_WRAP   0x20
//#define SRCDST_SIGN   0x40
#define SRCDST_A_1      0x80        /* This might be correct for line drawing? */

/* These appear to be correct */
#define MODE_SSIGN      0x80
#define MODE_DSIGN      0x40
#define MODE_YFRAC      0x20
#define MODE_BITTOBYTE  0x04
#define MODE_PALREMAP   0x10

#define CMPFUNC_LT      0x01
#define CMPFUNC_EQ      0x02
#define CMPFUNC_GT      0x04
#define CMPFUNC_BEQ     0x08
#define CMPFUNC_LOG0    0x10
#define CMPFUNC_LOG1    0x20
#define CMPFUNC_LOG2    0x40
#define CMPFUNC_LOG3    0x80

/*
    Blitter state
*/
struct bf_blitter_t
{
	ADDR_REG    program;

	UINT8       control;
	UINT8       status;

	UINT8       command;
	ADDR_REG    source;
	ADDR_REG    dest;
	UINT8       modectl;
	UINT8       compfunc;
	UINT8       outercnt;

	UINT8       innercnt;
	UINT8       step;
	UINT8       pattern;
};

#define LOOPTYPE ( ( blitter.command&0x60 ) >> 5 )

/*
    MUSIC Semiconductor TR9C1710 RAMDAC or equivalent
*/
struct ramdac_t
{
	UINT8   addr_w;
	UINT8   addr_r;
	UINT8   mask;

	/* 18-bit colors */
	UINT8   color_r[3];
	UINT8   color_w[3];
	UINT32  table[256];

	/* Access counts */
	UINT8   count_r;
	UINT8   count_w;
};


struct fdc_t
{
	UINT8   MSR;

	int     side;
	int     track;
	int     sector;
	int     number;
	int     stop_track;
	int     setup_read;

	int     byte_pos;
	int     offset;

	int     phase;
	int     next_phase;
	int     cmd_len;
	int     cmd_cnt;
	int     res_len;
	int     res_cnt;
	UINT8   cmd[10];
	UINT8   results[8];
};


#define BLUE_0 0
#define BLUE_1 1
#define BLUE_2 2
#define BLUE_3 3
#define GREEN_0 ( 0 << 2 )
#define GREEN_1 ( 1 << 2 )
#define GREEN_2 ( 2 << 2 )
#define GREEN_3 ( 3 << 2 )
#define GREEN_4 ( 4 << 2 )
#define GREEN_5 ( 5 << 2 )
#define GREEN_6 ( 6 << 2 )
#define GREEN_7 ( 7 << 2 )
#define RED_0 ( 0 << 5 )
#define RED_1 ( 1 << 5 )
#define RED_2 ( 2 << 5 )
#define RED_3 ( 3 << 5 )
#define RED_4 ( 4 << 5 )
#define RED_5 ( 5 << 5 )
#define RED_6 ( 6 << 5 )
#define RED_7 ( 7 << 5 )

class bfcobra_state : public driver_device
{
public:
	bfcobra_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_acia6850_0(*this, "acia6850_0"),
		m_acia6850_1(*this, "acia6850_1"),
		m_acia6850_2(*this, "acia6850_2"),
		m_upd7759(*this, "upd"),
		m_palette(*this, "palette")
	{
	}

	UINT8 m_bank_data[4];
	std::unique_ptr<UINT8[]> m_work_ram;
	std::unique_ptr<UINT8[]> m_video_ram;
	UINT8 m_h_scroll;
	UINT8 m_v_scroll;
	UINT8 m_flip_8;
	UINT8 m_flip_22;
	UINT8 m_videomode;
	UINT8 m_data_r;
	UINT8 m_data_t;
	int m_irq_state;
	int m_acia_irq;
	int m_vblank_irq;
	int m_blitter_irq;
	UINT8 m_z80_int;
	UINT8 m_z80_inten;
	UINT32 m_meter_latch;
	UINT32 m_mux_input;
	UINT32 m_mux_outputlatch;
	UINT8 m_col4bit[16];
	UINT8 m_col3bit[16];
	UINT8 m_col8bit[256];
	UINT8 m_col7bit[256];
	UINT8 m_col6bit[256];
	struct bf_blitter_t m_blitter;
	struct ramdac_t m_ramdac;
	struct fdc_t m_fdc;
	DECLARE_READ8_MEMBER(ramdac_r);
	DECLARE_WRITE8_MEMBER(ramdac_w);
	DECLARE_READ8_MEMBER(chipset_r);
	DECLARE_WRITE8_MEMBER(chipset_w);
	DECLARE_WRITE8_MEMBER(rombank_w);
	DECLARE_READ8_MEMBER(fdctrl_r);
	DECLARE_READ8_MEMBER(fddata_r);
	DECLARE_WRITE8_MEMBER(fdctrl_w);
	DECLARE_READ8_MEMBER(int_latch_r);
	DECLARE_READ8_MEMBER(meter_r);
	DECLARE_WRITE8_MEMBER(meter_w);
	DECLARE_READ8_MEMBER(latch_r);
	DECLARE_WRITE8_MEMBER(latch_w);
	DECLARE_READ8_MEMBER(upd_r);
	DECLARE_WRITE8_MEMBER(upd_w);
	DECLARE_WRITE_LINE_MEMBER(z80_acia_irq);
	DECLARE_WRITE_LINE_MEMBER(m6809_data_irq);
	DECLARE_WRITE_LINE_MEMBER(data_acia_tx_w);
	DECLARE_WRITE_LINE_MEMBER(write_acia_clock);
	DECLARE_DRIVER_INIT(bfcobra);
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_bfcobra(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(timer_irq);
	INTERRUPT_GEN_MEMBER(vblank_gen);
	void RunBlit(address_space &space);
	void update_irqs();
	void reset_fdc();
	void exec_w_phase(UINT8 data);
	void init_ram();
	void command_phase(struct fdc_t &fdc, UINT8 data);
	inline UINT8* blitter_get_addr(UINT32 addr);
	inline void z80_bank(int num, int data);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<acia6850_device> m_acia6850_0;
	required_device<acia6850_device> m_acia6850_1;
	required_device<acia6850_device> m_acia6850_2;
	required_device<upd7759_device> m_upd7759;
	required_device<palette_device> m_palette;
};


static const UINT8 col4bit_default[16]=
{
	BLUE_0 | GREEN_0 | RED_0,
	BLUE_1,
	GREEN_2,
	BLUE_1 | GREEN_2,
	RED_2,
	RED_2 | BLUE_1,
	RED_2 | GREEN_2,
	RED_2 | GREEN_2 | BLUE_1,
	BLUE_2 | GREEN_5 | RED_5,
	BLUE_3,
	GREEN_7,
	BLUE_3 | GREEN_7,
	RED_7,
	RED_7 | BLUE_3,
	RED_7 | GREEN_7,
	RED_7 | GREEN_7 | BLUE_3
};

static const UINT8 col3bit_default[16]=
{
	0,
	BLUE_3,
	GREEN_7,
	BLUE_3 | GREEN_7,
	RED_7,
	RED_7 | BLUE_3,
	RED_7 | GREEN_7,
	RED_7 | GREEN_7 | BLUE_3,
	0,
	BLUE_3,
	GREEN_7,
	BLUE_3 | GREEN_7,
	RED_7,
	RED_7 | BLUE_3,
	RED_7 | GREEN_7,
	RED_7 | GREEN_7 | BLUE_3
};

static const UINT8 col76index[] = {0, 2, 4, 7};


void bfcobra_state::video_start()
{
	int i;

	memcpy(m_col4bit, col4bit_default, sizeof(m_col4bit));
	memcpy(m_col3bit, col3bit_default, sizeof(m_col3bit));
	for (i = 0; i < 256; ++i)
	{
		UINT8 col;

		m_col8bit[i] = i;
		col = i & 0x7f;
		col = (col & 0x1f) | (col76index[ ( (col & 0x60) >> 5 ) & 3] << 5);
		m_col7bit[i] = col;

		col = (col & 3) | (col76index[( (col & 0x0c) >> 2) & 3] << 2 ) |
				(col76index[( (col & 0x30) >> 4) & 3] << 5 );
		m_col6bit[i] = col;
	}
}

UINT32 bfcobra_state::screen_update_bfcobra(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x, y;
	UINT8  *src;
	UINT32 *dest;
	UINT32 offset;
	UINT8 *hirescol;
	UINT8 *lorescol;

	/* Select screen has to be programmed into two registers */
	/* No idea what happens if the registers are different */
	if (m_flip_8 & 0x40 && m_flip_22 & 0x40)
		offset = 0x10000;
	else
		offset = 0;

	if(m_videomode & 0x20)
	{
		hirescol = m_col3bit;
		lorescol = m_col7bit;
	}
	else if(m_videomode & 0x40)
	{
		hirescol = m_col4bit;
		lorescol = m_col6bit;
	}
	else
	{
		hirescol = m_col4bit;
		lorescol = m_col8bit;
	}

	for (y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		UINT16 y_offset = (y + m_v_scroll) * 256;
		src = &m_video_ram[offset + y_offset];
		dest = &bitmap.pix32(y);

		for (x = cliprect.min_x; x <= cliprect.max_x / 2; ++x)
		{
			UINT8 x_offset = x + m_h_scroll;
			UINT8 pen = *(src + x_offset);

			if ( ( m_videomode & 0x81 ) == 1 || (m_videomode & 0x80 && pen & 0x80) )
			{
				*dest++ = m_palette->pen(hirescol[pen & 0x0f]);
				*dest++ = m_palette->pen(hirescol[(pen >> 4) & 0x0f]);
			}
			else
			{
				*dest++ = m_palette->pen(lorescol[pen]);
				*dest++ = m_palette->pen(lorescol[pen]);
			}
		}
	}

	return 0;
}

UINT8* bfcobra_state::blitter_get_addr(UINT32 addr)
{
	if (addr < 0x10000)
	{
		/* Is this region fixed? */
		return (UINT8*)(memregion("user1")->base() + addr);
	}
	else if(addr < 0x20000)
	{
		addr &= 0xffff;
		addr += (m_bank_data[0] & 1) ? 0x10000 : 0;

		return (UINT8*)(memregion("user1")->base() + addr + ((m_bank_data[0] >> 1) * 0x20000));
	}
	else if (addr >= 0x20000 && addr < 0x40000)
	{
		return (UINT8*)&m_video_ram[addr - 0x20000];
	}
	else
	{
		return (UINT8*)&m_work_ram[addr - 0x40000];
	}
}


/*
    This is based this on the Slipstream technical reference manual.
    The Flare One blitter is a simpler design with slightly different parameters
    and will require hardware tests to figure everything out correctly.
*/
void bfcobra_state::RunBlit(address_space &space)
{
#define BLITPRG_READ(x)     blitter.x = *(blitter_get_addr(blitter.program.addr++))

	struct bf_blitter_t &blitter = m_blitter;
	int cycles_used = 0;


	do
	{
		UINT8 srcdata = 0;
		UINT8 dstdata = 0;

		/* Read the blitter command */
		BLITPRG_READ(source.as8bit.addr0);
		BLITPRG_READ(source.as8bit.addr1);
		BLITPRG_READ(source.as8bit.addr2);
		BLITPRG_READ(dest.as8bit.addr0);
		BLITPRG_READ(dest.as8bit.addr1);
		BLITPRG_READ(dest.as8bit.addr2);
		BLITPRG_READ(modectl);
		BLITPRG_READ(compfunc);
		BLITPRG_READ(outercnt);
		BLITPRG_READ(innercnt);
		BLITPRG_READ(step);
		BLITPRG_READ(pattern);

#if 0
		/* This debug is now wrong ! */
		if (DEBUG_BLITTER)
		{
			osd_printf_debug("\n%s:Blitter: Running command from 0x%.5x\n\n", device->machine().describe_context(), blitter.program.addr - 12);
			osd_printf_debug("Command Reg         %.2x",   blitter.command);
			osd_printf_debug("     %s %s %s %s %s %s %s\n",
				blitter.command & CMD_RUN ? "RUN" : "     ",
				blitter.command & CMD_COLST ? "COLST" : "     ",
				blitter.command & CMD_PARRD ? "PARRD" : "     ",
				blitter.command & CMD_SRCUP ? "SRCUP" : "     ",
				blitter.command & CMD_DSTUP ? "DSTUP" : "     ");

			osd_printf_debug("Src Address Byte 0  %.2x\n", blitter.source.as8bit.addr0);
			osd_printf_debug("Src Address Byte 1  %.2x\n", blitter.source.as8bit.addr1);
			osd_printf_debug("Src Control         %.2x\n", blitter.source.as8bit.addr2);
			osd_printf_debug("  Src Address       %.5x\n", blitter.source.addr & 0xfffff);
			osd_printf_debug("Dest Address Byte 0 %.2x\n", blitter.dest.as8bit.addr0);
			osd_printf_debug("Dest Address Byte 1 %.2x\n", blitter.dest.as8bit.addr1);
			osd_printf_debug("Dest Control        %.2x\n", blitter.dest.as8bit.addr2);
			osd_printf_debug("  Dst. Address      %.5x\n", blitter.dest.addr & 0xfffff);
			osd_printf_debug("Mode Control        %.2x",   blitter.modectl);
			osd_printf_debug("     %s\n", blitter.modectl & MODE_BITTOBYTE ? "BIT_TO_BYTE" : "");

			osd_printf_debug("Comp. and LFU       %.2x\n", blitter.compfunc);
			osd_printf_debug("Outer Loop Count    %.2x (%d)\n", blitter.outercnt, blitter.outercnt);
			osd_printf_debug("Inner Loop Count    %.2x (%d)\n", blitter.innercnt, blitter.innercnt);
			osd_printf_debug("Step Value          %.2x\n", blitter.step);
			osd_printf_debug("Pattern Byte        %.2x\n", blitter.pattern);
		}
#endif

		/* Ignore these writes */
		if (blitter.dest.addr == 0)
			return;

		/* Begin outer loop */
		for (;;)
		{
			UINT8 innercnt = blitter.innercnt;
			dstdata = blitter.pattern;

			if (blitter.command & CMD_LINEDRAW)
			{
				do
				{
					if (blitter.modectl & MODE_YFRAC)
					{
						if (blitter.modectl & MODE_SSIGN )
							blitter.dest.as8bit.addr0--;
						else
							blitter.dest.as8bit.addr0++;
					}
					else
					{
						if (blitter.modectl & MODE_DSIGN )
							blitter.dest.as8bit.addr1--;
						else
							blitter.dest.as8bit.addr1++;
					}
					if( blitter.source.as8bit.addr0 < blitter.step )
					{
						blitter.source.as8bit.addr0 -= blitter.step ;
						blitter.source.as8bit.addr0 += blitter.source.as8bit.addr1;

						if ( blitter.modectl & MODE_YFRAC )
						{
							if (blitter.modectl & MODE_DSIGN )
								blitter.dest.as8bit.addr1--;
							else
								blitter.dest.as8bit.addr1++;
						}
						else
						{
							if (blitter.modectl & MODE_SSIGN )
								blitter.dest.as8bit.addr0--;
							else
								blitter.dest.as8bit.addr0++;
						}
					}
					else
					{
						blitter.source.as8bit.addr0 -= blitter.step;
					}

					*blitter_get_addr( blitter.dest.addr) = blitter.pattern;
					cycles_used++;

				} while (--innercnt);
			}
			else do
			{
				UINT8   inhibit = 0;

				/* TODO: Set this correctly */
				UINT8   result = blitter.pattern;

				if (LOOPTYPE == 3 && innercnt == blitter.innercnt)
				{
					srcdata = *(blitter_get_addr( blitter.source.addr & 0xfffff));
					blitter.source.as16bit.loword++;
					cycles_used++;
				}

				/* Enable source address read and increment? */
				if (!(blitter.modectl & (MODE_BITTOBYTE | MODE_PALREMAP)))
				{
					if (LOOPTYPE == 0 || LOOPTYPE == 1)
					{
						srcdata = *(blitter_get_addr( blitter.source.addr & 0xfffff));
						cycles_used++;

						if (blitter.modectl & MODE_SSIGN)
							blitter.source.as16bit.loword-- ;
						else
							blitter.source.as16bit.loword++;

						result = srcdata;
					}
				}

				/* Read destination pixel? */
				if (LOOPTYPE == 0)
				{
					dstdata = *blitter_get_addr( blitter.dest.addr & 0xfffff);
					cycles_used++;
				}

				/* Inhibit depending on the bit selected by the inner count */

				/* Switch on comparator type? */
				if (blitter.modectl & MODE_BITTOBYTE)
				{
					inhibit = !(srcdata & (1 << (8 - innercnt)));
				}

				if (blitter.compfunc & CMPFUNC_BEQ)
				{
					if (srcdata == blitter.pattern)
					{
						inhibit = 1;

						/* TODO: Resume from inhibit? */
						if (blitter.command & CMD_COLST)
							return;
					}
				}
				if (blitter.compfunc & CMPFUNC_LT)
				{
					/* Might be wrong */
					if ((srcdata & 0xc0) < (dstdata & 0xc0))
					{
						inhibit = 1;

						/* TODO: Resume from inhibit? */
						if (blitter.command & CMD_COLST)
							return;
					}
				}
				if (blitter.compfunc & CMPFUNC_EQ)
				{
					if ((srcdata & 0xc0) == (dstdata & 0xc0))
					{
						inhibit = 1;

						/* TODO: Resume from inhibit? */
						if (blitter.command & CMD_COLST)
							return;
					}
				}
				if (blitter.compfunc & CMPFUNC_GT)
				{
					/* Might be wrong */
					if ((srcdata & 0xc0) > (dstdata & 0xc0))
					{
						inhibit = 1;

						/* TODO: Resume from inhibit? */
						if (blitter.command & CMD_COLST)
							return;
					}
				}

				/* Write the data if not inhibited */
				if (!inhibit)
				{
					if (blitter.modectl == MODE_PALREMAP)
					{
						/*
						    In this mode, the source points to a 256 entry lookup table.
						    The existing destination pixel is used as a lookup
						    into the table and the colours is replaced.
						*/
						UINT8 dest = *blitter_get_addr( blitter.dest.addr);
						UINT8 newcol = *(blitter_get_addr( (blitter.source.addr + dest) & 0xfffff));

						*blitter_get_addr( blitter.dest.addr) = newcol;
						cycles_used += 3;
					}
					else
					{
						UINT8 final_result = 0;

						if (blitter.compfunc & CMPFUNC_LOG3)
							final_result |= result & dstdata;

						if (blitter.compfunc & CMPFUNC_LOG2)
							final_result |= result & ~dstdata;

						if (blitter.compfunc & CMPFUNC_LOG1)
							final_result |= ~result & dstdata;

						if (blitter.compfunc & CMPFUNC_LOG0)
							final_result |= ~result & ~dstdata;

						*blitter_get_addr( blitter.dest.addr) = final_result;
						cycles_used++;
					}
				}

				/* Update destination address */
				if (blitter.modectl & MODE_DSIGN)
					blitter.dest.as16bit.loword--;
				else
					blitter.dest.as16bit.loword++;

			} while (--innercnt);

			if (!--blitter.outercnt)
			{
				break;
			}
			else
			{
				if (blitter.command & CMD_DSTUP)
					blitter.dest.as16bit.loword += blitter.step;

				if (blitter.command & CMD_SRCUP)
					blitter.source.as16bit.loword += blitter.step;

				if (blitter.command & CMD_PARRD)
				{
					BLITPRG_READ(innercnt);
					BLITPRG_READ(step);
					BLITPRG_READ(pattern);
				}
			}
		}

		/* Read next command header */
		BLITPRG_READ(command);

	} while (blitter.command  & CMD_RUN);

	/* Burn Z80 cycles while blitter is in operation */
	space.device().execute().spin_until_time(attotime::from_nsec( (1000000000 / Z80_XTAL)*cycles_used * 2 ) );
}


READ8_MEMBER(bfcobra_state::ramdac_r)
{
	struct ramdac_t &ramdac = m_ramdac;
	UINT8 val = 0xff;

	switch (offset & 3)
	{
		case 1:
		{
			UINT8 *count = &ramdac.count_r;

			if (*count == 0)
			{
				rgb_t color;
				color = m_palette->pen_color(ramdac.addr_r);

				ramdac.color_r[0] = color.r();
				ramdac.color_r[1] = color.g();
				ramdac.color_r[2] = color.b();
			}

			val = ramdac.color_r[*count];

			/* 8bpp -> 6bpp */
			val = ((val & 0xc0) >> 2) | ((val >>2) & 0xf);

			if (++*count == 3)
			{
				*count = 0;
				ramdac.addr_r++;
			}
			break;
		}
		default:
		{
			osd_printf_debug("Unhandled RAMDAC read (PC:%.4x)\n", space.device().safe_pcbase());
		}
	}

	return val;
}

WRITE8_MEMBER(bfcobra_state::ramdac_w)
{
	struct ramdac_t &ramdac = m_ramdac;

	switch (offset & 3)
	{
		case 0:
		{
			ramdac.addr_w = data;
			break;
		}
		case 1:
		{
			data &= 0x3f;
			ramdac.color_w[ramdac.count_w] = pal6bit(data);
			if (++ramdac.count_w == 3)
			{
				m_palette->set_pen_color(ramdac.addr_w, ramdac.color_w[0], ramdac.color_w[1], ramdac.color_w[2]);
				ramdac.count_w = 0;
				ramdac.addr_w++;
			}
			break;
		}
		case 2:
		{
			ramdac.mask = data;
			break;
		}
		case 3:
		{
			ramdac.addr_r = data;
			break;
		}
	}
}

/***************************************************************************

    Flare One Register Map

    01  Bank control for Z80 region 0x4000-0x7fff (16kB)    WR
    02  Bank control for Z80 region 0x8000-0xbfff (16kB)    WR
    03  Bank control for Z80 region 0xc000-0xffff (16kB)    WR

    06  Interrupt status....................................WR
    07  Interrupt ack.......................................WR
        Writing here sets the line number that vertical interrupt is generated at.
        cmd1, bit2 is the 9th bit of the line number
        ???? Written with 0x21
    08  cmd1                                                WR * bit 6 = screen select
        bit2 = 9th bit of vertical interrupt line number
        bit6 = 1 = select screen 1 else screen 0
    09  cmd2 Linked with c001...............................W * bit 0 = 1 = hires
        bit0=1=hi res else lo res (as long as bit7 is 0)
        bit5=mask msb of each pixel
        bit6=mask 2 msbits of each lores pixel
        bit7=1=variable resolution - resolution is set by bit 7 of each vram byte.  bit7=1=2 hires pixels
    0A  ???? Written with 0 and 1...........................W
        color of border

    0B  Horizontal frame buffer scroll .....................W
    0C  Vertical frame buffer scroll .......................W

    0D  Colour hold colour..................................W
    0E  Palette value for hi-res magenta....................W
    0F  Palette value for hi-res yellow?....................W
    14  ....................................................W

    18  Blitter program low byte............................WR
    19  Blitter program middle byte.........................W
    1A  Blitter program high byte...........................W
    1B  Blitter command?....................................W
    1C  Blitter status?......................................R
    20  Blitter control register............................W

    22  ???? Linked with C002...............................W
        Mask of 20

        Joystick: xxx1 11xx                                 R
        ? (polled on tight loop):  x1xx xxxx                R

    40: ROM bank select.....................................W

***************************************************************************/

void bfcobra_state::update_irqs()
{
	int newstate = m_blitter_irq || m_vblank_irq || m_acia_irq;

	if (newstate != m_irq_state)
	{
		m_irq_state = newstate;
		m_maincpu->set_input_line(0, m_irq_state ? ASSERT_LINE : CLEAR_LINE);
	}
}

READ8_MEMBER(bfcobra_state::chipset_r)
{
	UINT8 val = 0xff;

	switch(offset)
	{
		case 1:
		case 2:
		case 3:
		{
			val = m_bank_data[offset];
			break;
		}
		case 6:
		{
			/* TODO */
			val = m_vblank_irq << 4;
			break;
		}
		case 7:
		{
			m_vblank_irq = 0;
			val = 0x1;

			/* TODO */
			update_irqs();
			break;
		}
		case 0x1C:
		{
			/* Blitter status ? */
			val = 0;
			break;
		}
		case 0x20:
		{
			/* Seems correct - used during RLE pic decoding */
			val = m_blitter.dest.as8bit.addr0;
			break;
		}
		case 0x22:
		{
			val = 0x40 | ioport("JOYSTICK")->read();
			break;
		}
		default:
		{
			osd_printf_debug("Flare One unknown read: 0x%.2x (PC:0x%.4x)\n", offset, space.device().safe_pcbase());
		}
	}

	return val;
}

WRITE8_MEMBER(bfcobra_state::chipset_w)
{
	switch (offset)
	{
		case 0x01:
		case 0x02:
		case 0x03:
		{
			if (data > 0x3f)
				popmessage("%x: Unusual bank access (%x)\n", space.device().safe_pcbase(), data);

			data &= 0x3f;
			m_bank_data[offset] = data;
			z80_bank(offset, data);
			break;
		}

		case 0x08:
		{
			m_flip_8 = data;
			break;
		}
		case 9:
			m_videomode = data;
			break;

		case 0x0B:
		{
			m_h_scroll = data;
			break;
		}
		case 0x0C:
		{
			m_v_scroll = data;
			break;
		}
		case 0x0E:
		{
			m_col4bit[5] = data;
			m_col3bit[5] = data;
			m_col3bit[5 + 8] = data;
			break;
		}
		case 0x0f:
		{
			m_col4bit[6] = data;
			m_col3bit[6] = data;
			m_col3bit[6 + 8] = data;
			break;
		}
		case 0x18:
		{
			m_blitter.program.as8bit.addr0 = data;
			break;
		}
		case 0x19:
		{
			m_blitter.program.as8bit.addr1 = data;
			break;
		}
		case 0x1A:
		{
			m_blitter.program.as8bit.addr2 = data;
			break;
		}
		case 0x20:
		{
			m_blitter.command = data;

			if (data & CMD_RUN)
				RunBlit(space);
			else
				osd_printf_debug("Blitter stopped by IO.\n");

			break;
		}
		case 0x22:
		{
			m_flip_22 = data;
			break;
		}
		default:
		{
			osd_printf_debug("Flare One unknown write: 0x%.2x with 0x%.2x (PC:0x%.4x)\n", offset, data, space.device().safe_pcbase());
		}
	}
}

void bfcobra_state::z80_bank(int num, int data)
{
	static const char * const bank_names[] = { "bank1", "bank2", "bank3" };

	if (data < 0x08)
	{
		UINT32 offset = ((m_bank_data[0] >> 1) * 0x20000) + ((0x4000 * data) ^ ((m_bank_data[0] & 1) ? 0 : 0x10000));

		membank(bank_names[num - 1])->set_base(memregion("user1")->base() + offset);
	}
	else if (data < 0x10)
	{
		membank(bank_names[num - 1])->set_base(&m_video_ram[(data - 0x08) * 0x4000]);
	}
	else
	{
		membank(bank_names[num - 1])->set_base(&m_work_ram[(data - 0x10) * 0x4000]);
	}
}

WRITE8_MEMBER(bfcobra_state::rombank_w)
{
	m_bank_data[0] = data;
	z80_bank(1, m_bank_data[1]);
	z80_bank(2, m_bank_data[2]);
	z80_bank(3, m_bank_data[3]);
}



/***************************************************************************

    Split into machine\cobra.c !

    Alternatively chuck it all away and borrow the MESS implementation
    because it's a million times better.

***************************************************************************/


/*
    WD37C656C-PL (or equivalent) Floppy Disk Controller
*/

enum fdc_phase
{
	COMMAND,
	EXECUTION_R,
	EXECUTION_W,
	RESULTS
};

enum command
{
	SENSE_DRIVE_STATUS = 0,
	READ_A_TRACK = 2,
	SPECIFY = 3,
	WRITE_DATA = 5,
	READ_DATA = 6,
	RECALIBRATE = 7,
	SENSE_INTERRUPT_STATUS = 8,
	WRITE_DELETED_DATA = 9,
	READ_ID = 10,
	FORMAT_TRACK = 13,
	READ_DELETED_DATA = 14,
	SEEK = 15,
	SCAN_EQUAL = 17,
	SCAN_LOW_OR_EQUAL = 25,
	SCAN_HIGH_OR_EQUAL = 29
};

void bfcobra_state::reset_fdc()
{
	memset(&m_fdc, 0, sizeof(m_fdc));

	m_fdc.MSR = 0x80;
	m_fdc.phase = COMMAND;
}

READ8_MEMBER(bfcobra_state::fdctrl_r)
{
	UINT8 val = 0;

	val = m_fdc.MSR;

	return val;
}

READ8_MEMBER(bfcobra_state::fddata_r)
{
	struct fdc_t &fdc = m_fdc;
	#define BPS     1024
	#define SPT     10
	#define BPT     1024*10

	UINT8 val = 0;

	if (fdc.phase == EXECUTION_R)
	{
		switch (fdc.cmd[0] & 0x1f)
		{
			/* Specify */
			case READ_DATA:
			{
				if (fdc.setup_read)
				{
					fdc.track = fdc.cmd[2];
					fdc.side = fdc.cmd[3];
					fdc.sector = fdc.cmd[4];
					fdc.number = fdc.cmd[5];
					fdc.stop_track = fdc.cmd[6];
					//int GPL = fdc.cmd[7];
					//int DTL = fdc.cmd[8];

					fdc.setup_read = 0;
					fdc.byte_pos = 0;
				}

				fdc.offset = (BPT * fdc.track*2) + (fdc.side ? BPT : 0) + (BPS * (fdc.sector-1)) + fdc.byte_pos++;
				val = *(memregion("user2")->base() + fdc.offset);

				/* Move on to next sector? */
				if (fdc.byte_pos == 1024)
				{
					fdc.byte_pos = 0;

					if (fdc.sector == fdc.stop_track || ++fdc.sector == 11)
					{
						/* End of read operation */
						fdc.MSR = 0xd0;
						fdc.phase = RESULTS;

						fdc.results[0] = 0;
						fdc.results[1] = 0;
						fdc.results[2] = 0;

						fdc.results[3] = 0;
						fdc.results[4] = 0;
						fdc.results[5] = 0;
						fdc.results[6] = 0;
					}
				}
				break;
			}
		}
	}
	else if (fdc.phase == RESULTS)
	{
		val = fdc.results[fdc.res_cnt++];

		if (fdc.res_cnt == fdc.res_len)
		{
			fdc.phase = COMMAND;
			fdc.res_cnt = 0;
			fdc.MSR &= ~0x40;
		}
	}

	return val;
}

WRITE8_MEMBER(bfcobra_state::fdctrl_w)
{
	struct fdc_t &fdc = m_fdc;
	switch (fdc.phase)
	{
		case COMMAND:
		{
			command_phase(fdc, data);
			break;
		}
		case EXECUTION_W:
		{
			exec_w_phase(data);
			break;
		}
		default:
		{
			osd_printf_debug("Unknown FDC phase?!");
		}
	}
}

void bfcobra_state::command_phase(struct fdc_t &fdc, UINT8 data)
{
	if (fdc.cmd_cnt == 0)
	{
		fdc.cmd[0] = data;

		fdc.cmd_cnt = 1;

		switch (data & 0x1f)
		{
			/* Specify */
			case READ_DATA:
			{
//              osd_printf_debug("Read data\n");
				fdc.cmd_len = 9;
				fdc.res_len = 7;
				fdc.next_phase = EXECUTION_R;
				fdc.setup_read = 1;
				break;
			}
			case SPECIFY:
			{
//              osd_printf_debug("Specify\n");
				fdc.cmd_len = 3;
				fdc.res_len = 0;
				fdc.next_phase = COMMAND;
				break;
			}
			case RECALIBRATE:
			{
//              osd_printf_debug("Recalibrate\n");
				fdc.cmd_len = 2;
				fdc.res_len = 0;
				fdc.next_phase = COMMAND;
				//fdc.MSR |= 0x40;
				break;
			}
			case SENSE_INTERRUPT_STATUS:
			{
//              osd_printf_debug("Sense interrupt status\n");
				fdc.cmd_len = 1;
				fdc.res_len = 2;
				fdc.phase = RESULTS;

				fdc.results[0] = 0;
				fdc.results[1] = 0;

				fdc.cmd_cnt = 0;
				fdc.MSR |= 0x40;
				break;
			}
			case SEEK:
			{
//              osd_printf_debug("Seek\n");
				fdc.cmd_len = 3;
				fdc.res_len = 0;
				fdc.next_phase = COMMAND;
				break;
			}
			default:
			{
//              osd_printf_debug("%x\n",data & 0x1f);
			}
		}
	}
	else
	{
		fdc.cmd[fdc.cmd_cnt++] = data;
		//osd_printf_debug(" %x\n",data);
	}

	if (fdc.cmd_cnt == fdc.cmd_len)
	{
		fdc.phase = fdc.next_phase;
		fdc.cmd_cnt = 0;

		if ((fdc.cmd[0] & 0x1f) == READ_DATA)
			fdc.MSR = 0xf0;
	}
}

#ifdef UNUSED_FUNCTION
UINT8 bfcobra_state::exec_r_phase(void)
{
	return 0;
}
#endif

void bfcobra_state::exec_w_phase(UINT8 data)
{
}

#ifdef UNUSED_FUNCTION
UINT8 bfcobra_state::results_phase(void)
{
	return 0;
}

WRITE8_MEMBER(bfcobra_state::fd_op_w)
{
}

WRITE8_MEMBER(bfcobra_state::fd_ctrl_w)
{
}
#endif

void bfcobra_state::machine_reset()
{
	unsigned int pal;

	for (pal = 0; pal < 256; ++pal)
	{
		m_palette->set_pen_color(pal, pal3bit((pal>>5)&7), pal3bit((pal>>2)&7), pal2bit(pal&3));
	}

	m_bank_data[0] = 1;
	memset(&m_ramdac, 0, sizeof(m_ramdac));
	reset_fdc();

	m_irq_state = m_blitter_irq = m_vblank_irq = m_acia_irq = 0;
}

/***************************************************************************

    Cobra I/Viper Z80 Memory Map

***************************************************************************/

static ADDRESS_MAP_START( z80_prog_map, AS_PROGRAM, 8, bfcobra_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROMBANK("bank4")
	AM_RANGE(0x4000, 0x7fff) AM_RAMBANK("bank1")
	AM_RANGE(0x8000, 0xbfff) AM_RAMBANK("bank2")
	AM_RANGE(0xc000, 0xffff) AM_RAMBANK("bank3")
ADDRESS_MAP_END

static ADDRESS_MAP_START( z80_io_map, AS_IO, 8, bfcobra_state )
ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x23) AM_READWRITE(chipset_r, chipset_w)
	AM_RANGE(0x24, 0x24) AM_DEVWRITE("acia6850_0", acia6850_device, control_w)
	AM_RANGE(0x25, 0x25) AM_DEVWRITE("acia6850_0", acia6850_device, data_w)
	AM_RANGE(0x26, 0x26) AM_DEVREAD("acia6850_0", acia6850_device, status_r)
	AM_RANGE(0x27, 0x27) AM_DEVREAD("acia6850_0", acia6850_device, data_r)
	AM_RANGE(0x30, 0x30) AM_READ(fdctrl_r)
	AM_RANGE(0x31, 0x31) AM_READWRITE(fddata_r, fdctrl_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(rombank_w)
	AM_RANGE(0x50, 0x53) AM_READWRITE(ramdac_r, ramdac_w)
ADDRESS_MAP_END


/***************************************************************************

    Cobra I/Viper 6809 Memory Map

    Cobra I has these components:

    EF68B50P            - For communication with Z80
    EF68B50P            - For data retrieval
    AY38912A
    UPD7759C
    BFM1090 (CF30056)   - 'TEXAS' I/O chip, lamps and misc
    BFM1095 (CF30057)   - 'TEXAS' I/O chip, handles switches
    BFM1071             - 6809 Address decoding and bus control

    /IRQ provided by EF68850P at IC38
    /FIRQ connected to meter current sensing circuitry

    TODO: Calculate watchdog timer period.

***************************************************************************/

/* TODO */
READ8_MEMBER(bfcobra_state::int_latch_r)
{
	return 2 | 1;
}

/* TODO */
READ8_MEMBER(bfcobra_state::meter_r)
{
	return m_meter_latch;
}

/* TODO: This is borrowed from Scorpion 1 */
WRITE8_MEMBER(bfcobra_state::meter_w)
{
	int i;
	int  changed = m_meter_latch ^ data;

	m_meter_latch = data;

	/*
	    When a meter is triggered, the current drawn is sensed. If a meter
	    is connected, the /FIRQ line will be pulsed.
	*/
	for (i = 0; i < 8; i++)
	{
		if (changed & (1 << i))
		{
			MechMtr_update(i, data & (1 << i) );
			space.device().execute().set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
		}
	}
}

/* TODO */
READ8_MEMBER(bfcobra_state::latch_r)
{
	return m_mux_input;
}

WRITE8_MEMBER(bfcobra_state::latch_w)
{
	/* TODO: This is borrowed from Scorpion 1 */
	switch(offset)
	{
		case 0:
		{
			int changed = m_mux_outputlatch ^ data;
			static const char *const port[] = { "STROBE0", "STROBE1", "STROBE2", "STROBE3", "STROBE4", "STROBE5", "STROBE6", "STROBE7" };

			m_mux_outputlatch = data;

			/* Clock has changed */
			if (changed & 0x08)
			{
				int input_strobe = data & 0x7;

				/* Clock is low */
				if (!(data & 0x08))
					m_mux_input = ioport(port[input_strobe])->read();
			}
			break;
		}
		case 1:
		{
//          strobe_data_l = data;
			break;
		}
		case 2:
		{
//          strobe_data_h = data;
			break;
		}
	}
}

READ8_MEMBER(bfcobra_state::upd_r)
{
	return 2 | m_upd7759->busy_r();
}

WRITE8_MEMBER(bfcobra_state::upd_w)
{
	m_upd7759->reset_w(data & 0x80);
	m_upd7759->port_w(space, 0, data & 0x3f);
	m_upd7759->start_w(data & 0x40 ? 0 : 1);
}

static ADDRESS_MAP_START( m6809_prog_map, AS_PROGRAM, 8, bfcobra_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x2000, 0x2000) AM_RAM     // W 'B', 6F
	AM_RANGE(0x2200, 0x2200) AM_RAM     // W 'F'
	AM_RANGE(0x2600, 0x2600) AM_READWRITE(meter_r, meter_w)
	AM_RANGE(0x2800, 0x2800) AM_RAM     // W
	AM_RANGE(0x2A00, 0x2A02) AM_READWRITE(latch_r, latch_w)
	AM_RANGE(0x2E00, 0x2E00) AM_READ(int_latch_r)
	AM_RANGE(0x3001, 0x3001) AM_DEVWRITE("aysnd", ay8910_device, data_w)
	AM_RANGE(0x3201, 0x3201) AM_DEVWRITE("aysnd", ay8910_device, address_w)
	AM_RANGE(0x3404, 0x3404) AM_DEVREADWRITE("acia6850_1", acia6850_device, status_r, control_w)
	AM_RANGE(0x3405, 0x3405) AM_DEVREADWRITE("acia6850_1", acia6850_device, data_r, data_w)
	AM_RANGE(0x3406, 0x3406) AM_DEVREADWRITE("acia6850_2", acia6850_device, status_r, control_w)
	AM_RANGE(0x3407, 0x3407) AM_DEVREADWRITE("acia6850_2", acia6850_device, data_r, data_w)
//  AM_RANGE(0x3408, 0x3408) AM_NOP
//  AM_RANGE(0x340A, 0x340A) AM_NOP
//  AM_RANGE(0x3600, 0x3600) AM_NOP
	AM_RANGE(0x3801, 0x3801) AM_READWRITE(upd_r, upd_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP    /* Watchdog */
ADDRESS_MAP_END

static INPUT_PORTS_START( bfcobra )
	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Coin: 10p")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("Coin: 20p")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("Coin: 50p")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("Coin: 1 pound")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Green Test?") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Red Test?")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Pass") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Continue") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Collect") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Bonus") PORT_CODE(KEYCODE_F)
//  PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
//  PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )
//  PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("<A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("<B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("<C")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("A>")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("B>")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("C>")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_INTERLOCK) PORT_NAME("Cash box door") PORT_CODE(KEYCODE_Y) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Front Door? (resets)") PORT_CODE(KEYCODE_T) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE4")
	PORT_BIT( 0xFF, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE5")
	PORT_BIT( 0xFF, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x01, 0x00, "DIL09" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL11" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL12" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL13" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL14" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL15" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE7")
	PORT_DIPNAME( 0x01, 0x00, "DIL02" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL03" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL04" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL05" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL06" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL07" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL08" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("JOYSTICK")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
INPUT_PORTS_END

/*
    Allocate work RAM and video RAM shared by the Z80 and chipset.
*/
void bfcobra_state::init_ram()
{
	/* 768kB work RAM */
	m_work_ram = make_unique_clear<UINT8[]>(0xC0000);

	/* 128kB video RAM */
	m_video_ram = make_unique_clear<UINT8[]>(0x20000);
}


WRITE_LINE_MEMBER(bfcobra_state::z80_acia_irq)
{
	m_acia_irq = state;
	update_irqs();
}


WRITE_LINE_MEMBER(bfcobra_state::m6809_data_irq)
{
	m_audiocpu->set_input_line(M6809_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}


WRITE_LINE_MEMBER(bfcobra_state::data_acia_tx_w)
{
	m_data_t = state;
}


WRITE_LINE_MEMBER(bfcobra_state::write_acia_clock)
{
	m_acia6850_0->write_txc(state);
	m_acia6850_0->write_rxc(state);
	m_acia6850_1->write_txc(state);
	m_acia6850_1->write_rxc(state);
	m_acia6850_2->write_txc(state);
	m_acia6850_2->write_rxc(state);
}


/* TODO: Driver vs Machine Init */
DRIVER_INIT_MEMBER(bfcobra_state,bfcobra)
{
	/*
	    6809 ROM address and data lines are scrambled.
	    This is the same scrambling as Scorpion 2.
	*/
	static const UINT8 datalookup[] = { 1, 3, 5, 6, 4, 2, 0, 7 };
	static const UINT8 addrlookup[] = { 11, 12, 0, 2, 3, 5, 7, 9, 8, 6, 1, 4, 10, 13, 14 };

	UINT32 i;
	UINT8 *rom;

	dynamic_buffer tmp(0x8000);
	rom = memregion("audiocpu")->base() + 0x8000;
	memcpy(&tmp[0], rom, 0x8000);

	for (i = 0; i < 0x8000; i++)
	{
		UINT16 addr = 0;
		UINT8 x;
		UINT8 data = 0;
		UINT8 val = tmp[i];

		for (x = 0; x < 8; x ++)
			data |= ((val >> x) & 1) << datalookup[x];

		for (x = 0; x < 15; x ++)
			addr |= ((i >> x) & 1)  << addrlookup[x];

		rom[addr] = data;
	}

	init_ram();

	m_bank_data[0] = 1;
	m_bank_data[1] = 0;
	m_bank_data[2] = 0;
	m_bank_data[3] = 0;

	/* Fixed 16kB ROM region */
	membank("bank4")->set_base(memregion("user1")->base());

	/* TODO: Properly sort out the data ACIA */
	m_acia6850_2->write_rxd(1);

	/* Finish this */
	save_item(NAME(m_data_r));
	save_item(NAME(m_data_t));
	save_item(NAME(m_h_scroll));
	save_item(NAME(m_v_scroll));
	save_item(NAME(m_flip_8));
	save_item(NAME(m_flip_22));
	save_item(NAME(m_z80_int));
	save_item(NAME(m_z80_inten));
	save_item(NAME(m_bank_data));
	save_pointer(NAME(m_work_ram.get()), 0xc0000);
	save_pointer(NAME(m_video_ram.get()), 0x20000);
}

/* TODO */
INTERRUPT_GEN_MEMBER(bfcobra_state::timer_irq)
{
	device.execute().set_input_line(M6809_IRQ_LINE, HOLD_LINE);
}

/* TODO */
INTERRUPT_GEN_MEMBER(bfcobra_state::vblank_gen)
{
	m_vblank_irq = 1;
	update_irqs();
}

static MACHINE_CONFIG_START( bfcobra, bfcobra_state )
	MCFG_CPU_ADD("maincpu", Z80, Z80_XTAL)
	MCFG_CPU_PROGRAM_MAP(z80_prog_map)
	MCFG_CPU_IO_MAP(z80_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", bfcobra_state,  vblank_gen)

	MCFG_CPU_ADD("audiocpu", M6809, M6809_XTAL)
	MCFG_CPU_PROGRAM_MAP(m6809_prog_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(bfcobra_state, timer_irq, 1000)

	MCFG_NVRAM_ADD_0FILL("nvram")


	/* TODO */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512 - 1, 0, 256 - 1)
	MCFG_SCREEN_UPDATE_DRIVER(bfcobra_state, screen_update_bfcobra)

	MCFG_PALETTE_ADD("palette", 256)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, M6809_XTAL)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MCFG_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	/* ACIAs */
	MCFG_DEVICE_ADD("acia6850_0", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("acia6850_1", acia6850_device, write_rxd))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(bfcobra_state, z80_acia_irq))

	MCFG_DEVICE_ADD("acia6850_1", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("acia6850_0", acia6850_device, write_rxd))

	MCFG_DEVICE_ADD("acia6850_2", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE(bfcobra_state, data_acia_tx_w))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(bfcobra_state, m6809_data_irq))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 31250*16) // What are the correct ACIA clocks ?
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(bfcobra_state, write_acia_clock))
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

  Note: Two different versions of each 6809 ROM exist: standard and protocol
  It appears sets can be a combination of disk images, 6809 and Z80 ROMs!

***************************************************************************/

ROM_START( inquiztr )
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "inq6809", 0x08000, 0x08000, CRC(ae996600) SHA1(f360399e77b81399d910770fa8106c196f04363c) )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD( "9576002.bin", 0x00000, 0x10000, CRC(5b8c8a04) SHA1(af5328fee79c370f45bff36f534aaf50964b6900) )

	ROM_REGION( 0x20000, "altuser1", 0 )
	ROM_LOAD( "9576028.bin", 0x10000, 0x10000, CRC(2d85682c) SHA1(baec47bff4b8beef5afbb737dc57b22bf93ebcf8) )

		// these look quite different.. (like they belong together) but booting with these gives a checksum error (banking?)
	ROM_LOAD( "inqvypp1", 0x00000, 0x010000, CRC(9bac8c6e) SHA1(15e24d60c2f3997e637694f60daa552b22628766) )
	ROM_LOAD( "inqvypp2", 0x10000, 0x010000, CRC(f9cd196c) SHA1(0ac31d87462cbee6f41e19aefe740d876910bdf5) )

	ROM_REGION( 0x1c2000, "user2", 0 )
	ROM_LOAD( "inqdisk.img", 0x000000, 0x1c2000, NO_DUMP )
ROM_END




ROM_START( escounts )
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "esc12int", 0x08000, 0x08000, CRC(741a1fe6) SHA1(e741d0ae0d2f11036a358120381e4b0df4a560a1) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "bfm_esc.bin", 0x08000, 0x08000, CRC(27acb5a5) SHA1(da50d650ab6456d61d0fb7f89247f2040b4bb9a8) )
	ROM_LOAD( "escint1b",    0x08000, 0x08000, CRC(fde413c9) SHA1(18724a1b771ba4c72e571c356591c5be32948d7a) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "esccobpa", 0x00000, 0x10000, CRC(d8eadeb7) SHA1(9b94f1454e6a17bf8321b0ef4ddd0ed1a56150f7) )

	/* 95-100-207 */
	ROM_REGION( 0x190000, "user2", 0 )
	ROM_LOAD( "escdisk4.img", 0x000000, 0x190000, CRC(24156e0f) SHA1(e163daa8effadd93ace2bc2ba1af0f4a190abd7f) )
ROM_END

ROM_START( trebltop )
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "95740078.bin", 0x08000, 0x08000, CRC(aca1980b) SHA1(3d4ed1dc545cc80f56d7daa13028fb10a12a718b) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "ttopint", 0x08000, 0x08000, CRC(cc798f90) SHA1(c3f541ebbb3a2c29cc4f00bbd47e289bcff50ecb) )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD( "95760031.bin", 0x00000, 0x10000, CRC(8e75edb8) SHA1(0aaa3834d5ac20f92bdf1f2b8f1eb71854469cbe) )
	ROM_LOAD( "95760021.bin", 0x10000, 0x10000, CRC(f42016c0) SHA1(7067d018cb4bdcfba777267fb01cddf44e4216c3) )

	ROM_REGION( 0x1c2000, "user2", 0 )
	ROM_LOAD( "ttdisk1.img", 0x000000, 0x190000, CRC(b2003228) SHA1(5eb49f05137cdd404f22948d39aa79c1518c06eb) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "95000172.bin", 0x00000, 0x10000, CRC(e85367a5) SHA1(695fd95ddeecdb16602f7b0f075cf5128a2fb808) )
	ROM_LOAD( "95000173.bin", 0x10000, 0x10000, CRC(8bda2c5e) SHA1(79aab5a2af7a5add5fe9132dc13bcc3705c6faf3) )
ROM_END

ROM_START( beeline )
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bln12int.a", 0x08000, 0x08000, CRC(cb97905e) SHA1(9725156bf64e53a56bc0f90795d4b07db41d059e) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "beeint12", 0x8000, 0x008000, CRC(0a77d0be) SHA1(8e55e7b4eb85cc2521d8fdf7ede02131ed80372e) )


	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD( "blncob.pa", 0x00000, 0x10000, CRC(8abc0017) SHA1(ecf6e7a4021b35295eb9bb9aed1b88fff27ffbd1) )
	ROM_LOAD( "blncob.pb", 0x10000, 0x10000, CRC(feb121fe) SHA1(e83bfd6db00a3264e5076f257e261a1cb4605a83) )

	ROM_REGION( 0x1c2000, "user2", 0 )
	ROM_LOAD( "beedisk.img", 0x000000, 0x1c2000, NO_DUMP )
ROM_END

ROM_START( quizvadr )
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "q6809.bin", 0x08000, 0x8000, CRC(a74dff10) SHA1(87578694a022dc3d7ade9cc76d387c1ae5fc74d9) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "qvadrint", 0x08000, 0x08000, CRC(4c729943) SHA1(ab4e0fb6cfc66540ea1e9e36eebdf85f65c5fd2a) )

	ROM_REGION( 0x200000, "user1", 0 )
	ROM_LOAD( "5947011r.0", 0x000000, 0x80000, CRC(cac43c97) SHA1(3af529cd0f8ec57dd3596f5bca7b9c74cff171e4) )
	ROM_LOAD( "5947011r.1", 0x080000, 0x80000, CRC(120018dc) SHA1(cd153d2b7ed535b04dbcaf189d2fc96fe3c5b466) )
	ROM_LOAD( "5947011r.2", 0x100000, 0x80000, CRC(689b3b5a) SHA1(ea32d18acfd380de822efff4f2c95ce9873a33a2) )
	ROM_LOAD( "5947011r.3", 0x180000, 0x80000, CRC(c38dafeb) SHA1(d693387a5c3cde34c9d581f81a08a5fbc6f753f2) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "185snd2.bin", 0x00000, 0x10000, CRC(e36eccc2) SHA1(cfd8ca4c71528ea4e229074016240681b6de37cd) )
	ROM_LOAD( "184snd1.bin", 0x10000, 0x10000, CRC(aac058e8) SHA1(39e59ad9524130fc3bd8d46e1aa78bc4daf04e39) )
ROM_END

ROM_START( qos )
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "39360107.bin", 0x08000, 0x8000, CRC(20844655) SHA1(b67c7f7bbabf6d5139b8ad8cbb5f8cc3f28e9cc7) )

	ROM_REGION( 0x200000, "user1", 0 )
	ROM_LOAD( "95000338.rm0", 0x000000, 0x80000, CRC(96918aae) SHA1(849ce7b8eccc89c45aacc840a73935f95788a141) )
	ROM_LOAD( "95000339.rm1", 0x080000, 0x80000, CRC(b4c6dcc0) SHA1(56d8761766dfbd5b0e71f8c3ca575e88f1bc9929) )
	ROM_LOAD( "95000340.rm2", 0x100000, 0x80000, CRC(66d121fd) SHA1(ac65cc0ac6b0a41e78a3159c21ee44f765bdb5c8) )
	ROM_LOAD( "95000341.rm3", 0x180000, 0x80000, CRC(ef13658d) SHA1(240bc589900214eac79c91a531f254a9ac2f4ef6) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "snd1_218.ic7", 0x00000, 0x10000, CRC(061f496d) SHA1(653d16454d909c034191813b37d14010da7258c6) )
	ROM_LOAD( "snd2_219.ic8", 0x10000, 0x10000, CRC(d7874a47) SHA1(5bbd4040c7c0299e8cc135e6c6cd05370b260e9b) )
ROM_END

ROM_START( qosa )
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "qos_nondata_68f4.bin", 0x08000, 0x8000, CRC(5f40005a) SHA1(180017acf6b432bc135d1090099fdf99f1e3583a) )

	ROM_REGION( 0x200000, "user1", 0 )
	ROM_LOAD( "95000338.rm0", 0x000000, 0x80000, CRC(96918aae) SHA1(849ce7b8eccc89c45aacc840a73935f95788a141) )
	ROM_LOAD( "95000339.rm1", 0x080000, 0x80000, CRC(b4c6dcc0) SHA1(56d8761766dfbd5b0e71f8c3ca575e88f1bc9929) )
	ROM_LOAD( "95000340.rm2", 0x100000, 0x80000, CRC(66d121fd) SHA1(ac65cc0ac6b0a41e78a3159c21ee44f765bdb5c8) )
	ROM_LOAD( "95000341.rm3", 0x180000, 0x80000, CRC(ef13658d) SHA1(240bc589900214eac79c91a531f254a9ac2f4ef6) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "snd1_218.ic7", 0x00000, 0x10000, CRC(061f496d) SHA1(653d16454d909c034191813b37d14010da7258c6) )
	ROM_LOAD( "snd2_219.ic8", 0x10000, 0x10000, CRC(d7874a47) SHA1(5bbd4040c7c0299e8cc135e6c6cd05370b260e9b) )
ROM_END

ROM_START( qosb )
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "95740599.bin", 0x08000, 0x8000, CRC(bf1e321f) SHA1(51f18620f22ba2a1b110954284ddf00614d51a0e) )

	ROM_REGION( 0x200000, "user1", 0 )
	ROM_LOAD( "rom0_306.bin", 0x000000, 0x80000, CRC(c26c8f83) SHA1(6949027e1fe241cbb2e1cbbce18e47bcb0d84550) )
	ROM_LOAD( "rom1_307.bin", 0x080000, 0x80000, CRC(94611c03) SHA1(81f545ff96ff3d44285315400da94d870c89f896) )
	ROM_LOAD( "rom2_308.bin", 0x100000, 0x80000, CRC(f5572726) SHA1(e109265c5571d21213a6f405a13459e7bc6699bc) )
	ROM_LOAD( "rom3_309.bin", 0x180000, 0x80000, CRC(1b5edfa8) SHA1(348488debd4aa52f064e351ed0c082274da1db2b) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "snd1_218.ic7", 0x00000, 0x10000, CRC(061f496d) SHA1(653d16454d909c034191813b37d14010da7258c6) )
	ROM_LOAD( "snd2_219.ic8", 0x10000, 0x10000, CRC(d7874a47) SHA1(5bbd4040c7c0299e8cc135e6c6cd05370b260e9b) )

	/* there is a set close to this with

	    ROM_LOAD( "qosrom0", 0x0000, 0x080000, CRC(4f150634) SHA1(beb1d3c212b189f3baa08fe454e83f30108be08e) )
	    qosrom1 = rom1_307.bin          qosb       A Question of Sport (39-960-089)
	    ROM_LOAD( "qosrom2", 0x0000, 0x080000, CRC(c39737db) SHA1(ccfdb19dab3af064db44e6022248aef98749bc97) )
	    ROM_LOAD( "qosrom3", 0x0000, 0x080000, CRC(785b8ff9) SHA1(61b31e0e60c31ecb4b179bfe008a96155d939709) )
	    ROM_LOAD( "qossnd1", 0x0000, 0x010000, CRC(888a29f8) SHA1(0e5aa9db54e783708ece1e8c7bffb10d994ab384) )

	   but it simply looks like a bad dump, rom3 is an alt 'rom 2' and the others are mostly the same as qosb
	*/

ROM_END


GAME( 1989, inquiztr, 0         ,   bfcobra, bfcobra, bfcobra_state, bfcobra, ROT0, "BFM", "Inquizitor",                       MACHINE_NOT_WORKING )
GAME( 1990, escounts, 0         ,   bfcobra, bfcobra, bfcobra_state, bfcobra, ROT0, "BFM", "Every Second Counts (39-360-053)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1991, trebltop, 0         ,   bfcobra, bfcobra, bfcobra_state, bfcobra, ROT0, "BFM", "Treble Top (39-360-070)",          MACHINE_IMPERFECT_GRAPHICS )
GAME( 1991, beeline,  0         ,   bfcobra, bfcobra, bfcobra_state, bfcobra, ROT0, "BFM", "Beeline (39-360-075)",             MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1991, quizvadr, 0         ,   bfcobra, bfcobra, bfcobra_state, bfcobra, ROT0, "BFM", "Quizvaders (39-360-078)",          MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, qos,      0         ,   bfcobra, bfcobra, bfcobra_state, bfcobra, ROT0, "BFM", "A Question of Sport (set 1, 39-960-107)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, qosa,     qos       ,   bfcobra, bfcobra, bfcobra_state, bfcobra, ROT0, "BFM", "A Question of Sport (set 2, 39-960-099)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, qosb,     qos       ,   bfcobra, bfcobra, bfcobra_state, bfcobra, ROT0, "BFM", "A Question of Sport (set 3, 39-960-089)", MACHINE_IMPERFECT_GRAPHICS )
