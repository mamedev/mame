/***************************************************************************

Hyper Duel
(c)1993 Technosoft
TEC442-A(Japan)

TMP68000N-10 x2
YM2151,YM3012
OKI M6295
OSC:  4.0000MHz, 20.0000MHz, 26.6660MHz
Imagetek Inc 14220 071


Magical Error wo Sagase
(c)1994 Technosoft / Jaleco
TEC5000(Japan)

TMP68000N-10 x2
YM2413
OKI M6295
OSC:  3.579545MHz, 4.0000MHz, 20.0000MHz, 26.6660MHz
Imagetek Inc 14220 071

--
Written by Hau
03/29/2009
based on driver from drivers/metro.c by Luca Elia
spthx to kikur,Cha,teioh,kokkyu,teruchu,aya,sgo
---

Magical Error
different sized sound / shared region (or the mem map needs more alterations?)
fix comms so it boots, it's a bit of a hack for hyperduel at the moment ;-)

***************************************************************************/

#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "deprecat.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "sound/2413intf.h"
#include "includes/hyprduel.h"


static int blitter_bit;
static int requested_int;
static UINT16 *hyprduel_irq_enable;
static int subcpu_resetline;
static int cpu_trigger;
static int int_num;

static UINT16 *sharedram1;
static UINT16 *sharedram3;

/***************************************************************************
                                Interrupts
***************************************************************************/

static void update_irq_state(running_machine *machine)
{
	int irq = requested_int & ~*hyprduel_irq_enable;

	cputag_set_input_line(machine, "maincpu", 3, (irq & int_num) ? ASSERT_LINE : CLEAR_LINE);
}

static TIMER_CALLBACK( vblank_end_callback )
{
	requested_int &= ~param;
}

static INTERRUPT_GEN( hyprduel_interrupt )
{
	int line = RASTER_LINES - cpu_getiloops(device);

	if (line == RASTER_LINES)
	{
		requested_int |= 0x01;		/* vblank */
		requested_int |= 0x20;
		cpu_set_input_line(device, 2, HOLD_LINE);
		/* the duration is a guess */
		timer_set(device->machine, ATTOTIME_IN_USEC(2500), NULL, 0x20, vblank_end_callback);
	} else {
		requested_int |= 0x12;		/* hsync */
	}

	update_irq_state(device->machine);
}

static READ16_HANDLER( hyprduel_irq_cause_r )
{
	return requested_int;
}

static WRITE16_HANDLER( hyprduel_irq_cause_w )
{
	if (ACCESSING_BITS_0_7)
	{
		if (data == int_num)
			requested_int &= ~(int_num & ~*hyprduel_irq_enable);
		else
			requested_int &= ~(data & *hyprduel_irq_enable);

		update_irq_state(space->machine);
	}
}


static WRITE16_HANDLER( hyprduel_subcpu_control_w )
{
	switch (data)
	{
		case 0x0d:
		case 0x0f:
		case 0x01:
			if (!subcpu_resetline)
			{
				cputag_set_input_line(space->machine, "sub", INPUT_LINE_RESET, ASSERT_LINE);
				subcpu_resetline = 1;
			}
			break;

		case 0x00:
			if (subcpu_resetline)
			{
				cputag_set_input_line(space->machine, "sub", INPUT_LINE_RESET, CLEAR_LINE);
				subcpu_resetline = 0;
			}
			cpu_spinuntil_int(space->cpu);
			break;

		case 0x0c:
		case 0x80:
			cputag_set_input_line(space->machine, "sub", 2, HOLD_LINE);
			break;
	}
}


static READ16_HANDLER( hyprduel_cpusync_trigger1_r )
{
	if (cpu_trigger == 1001)
	{
		cpuexec_trigger(space->machine, 1001);
		cpu_trigger = 0;
	}

	return sharedram1[0x000408/2 + offset];
}

static WRITE16_HANDLER( hyprduel_cpusync_trigger1_w )
{
	COMBINE_DATA(&sharedram1[0x00040e/2 + offset]);

	if (((sharedram1[0x00040e/2]<<16) + sharedram1[0x000410/2]) != 0x00)
	{
		if (!cpu_trigger && !subcpu_resetline)
		{
			cpu_spinuntil_trigger(space->cpu, 1001);
			cpu_trigger = 1001;
		}
	}
}


static READ16_HANDLER( hyprduel_cpusync_trigger2_r )
{
	if (cpu_trigger == 1002)
	{
		cpuexec_trigger(space->machine, 1002);
		cpu_trigger = 0;
	}

	return sharedram3[(0xfff34c - 0xfe4000)/2 + offset];
}

static WRITE16_HANDLER( hyprduel_cpusync_trigger2_w )
{
	COMBINE_DATA(&sharedram1[0x000408/2 + offset]);

	if (ACCESSING_BITS_8_15)
	{
		if (!cpu_trigger && !subcpu_resetline)
		{
			cpu_spinuntil_trigger(space->cpu, 1002);
			cpu_trigger = 1002;
		}
	}
}


static emu_timer *magerror_irq_timer;

static TIMER_CALLBACK( magerror_irq_callback )
{
	cputag_set_input_line(machine, "sub", 1, HOLD_LINE);
}

/***************************************************************************
                                Banked ROM access
***************************************************************************/

/*
    The main CPU has access to the ROMs that hold the graphics through
    a banked window of 64k. Those ROMs also usually store the tables for
    the virtual tiles set. The tile codes to be written to the tilemap
    memory to render the backgrounds are also stored here, in a format
    that the blitter can readily use (which is a form of compression)
*/

static UINT16 *hyprduel_rombank;

static READ16_HANDLER( hyprduel_bankedrom_r )
{
	UINT8 *ROM = memory_region( space->machine, "gfx1" );
	size_t  len  = memory_region_length( space->machine, "gfx1" );

	offset = offset * 2 + 0x10000 * (*hyprduel_rombank);

	if ( offset < len )	return ((ROM[offset+0]<<8)+ROM[offset+1]);
	else				return 0xffff;
}

/***************************************************************************

                                    Blitter

    [ Registers ]

        Offset:     Value:

        0.l         Destination Tilemap      (1,2,3)
        4.l         Blitter Data Address     (byte offset into the gfx ROMs)
        8.l         Destination Address << 7 (byte offset into the tilemap)

        The Blitter reads a byte and looks at the most significative
        bits for the opcode, while the remaining bits define a value
        (usually how many bytes to write). The opcode byte may be
        followed by a number of other bytes:

            76------            Opcode
            --543210            N
            (at most N+1 bytes follow)


        The blitter is designed to write every other byte (e.g. it
        writes a byte and skips the next). Hence 2 blits are needed
        to fill a tilemap (first even, then odd addresses)

    [ Opcodes ]

            0       Copy the following N+1 bytes. If the whole byte
                    is $00: stop and generate an IRQ

            1       Fill N+1 bytes with a sequence, starting with
                    the  value in the following byte

            2       Fill N+1 bytes with the value in the following
                    byte

            3       Skip N+1 bytes. If the whole byte is $C0:
                    skip to the next row of the tilemap (+0x200 bytes)
                    but preserve the column passed at the start of the
                    blit (destination address % 0x200)

***************************************************************************/

static UINT16 *hyprduel_blitter_regs;

static TIMER_CALLBACK( hyprduel_blit_done )
{
	requested_int |= 1 << blitter_bit;
	update_irq_state(machine);
}

INLINE int blt_read(const UINT8 *ROM, const int offs)
{
	return ROM[offs];
}

INLINE void blt_write(const address_space *space, const int tmap, const offs_t offs, const UINT16 data, const UINT16 mask)
{
	switch( tmap )
	{
		case 1:	hyprduel_vram_0_w(space,offs,data,mask);	break;
		case 2:	hyprduel_vram_1_w(space,offs,data,mask);	break;
		case 3:	hyprduel_vram_2_w(space,offs,data,mask);	break;
	}
//  logerror("%s : Blitter %X] %04X <- %04X & %04X\n",cpuexec_describe_context(space->machine),tmap,offs,data,mask);
}


static WRITE16_HANDLER( hyprduel_blitter_w )
{
	COMBINE_DATA( &hyprduel_blitter_regs[offset] );

	if (offset == 0xC/2)
	{
		UINT8 *src	=	memory_region(space->machine, "gfx1");
		size_t  src_len	=	memory_region_length(space->machine, "gfx1");

		UINT32 tmap		=	(hyprduel_blitter_regs[ 0x00 / 2 ] << 16 ) +
							 hyprduel_blitter_regs[ 0x02 / 2 ];
		UINT32 src_offs	=	(hyprduel_blitter_regs[ 0x04 / 2 ] << 16 ) +
							 hyprduel_blitter_regs[ 0x06 / 2 ];
		UINT32 dst_offs	=	(hyprduel_blitter_regs[ 0x08 / 2 ] << 16 ) +
							 hyprduel_blitter_regs[ 0x0a / 2 ];

		int shift			=	(dst_offs & 0x80) ? 0 : 8;
		UINT16 mask		=	(dst_offs & 0x80) ? 0x00ff : 0xff00;

//      logerror("CPU #0 PC %06X : Blitter regs %08X, %08X, %08X\n",cpu_get_pc(space->cpu),tmap,src_offs,dst_offs);

		dst_offs >>= 7+1;
		switch( tmap )
		{
			case 1:
			case 2:
			case 3:
				break;
			default:
				logerror("CPU #0 PC %06X : Blitter unknown destination: %08X\n",cpu_get_pc(space->cpu),tmap);
				return;
		}

		while (1)
		{
			UINT16 b1,b2,count;

			src_offs %= src_len;
			b1 = blt_read(src,src_offs);
//          logerror("CPU #0 PC %06X : Blitter opcode %02X at %06X\n",cpu_get_pc(space->cpu),b1,src_offs);
			src_offs++;

			count = ((~b1) & 0x3f) + 1;

			switch( (b1 & 0xc0) >> 6 )
			{
				case 0:

					/* Stop and Generate an IRQ. We can't generate it now
                       both because it's unlikely that the blitter is so
                       fast and because some games (e.g. lastfort) need to
                       complete the blitter irq service routine before doing
                       another blit. */
					if (b1 == 0)
					{
						timer_set(space->machine, ATTOTIME_IN_USEC(500), NULL,0,hyprduel_blit_done);
						return;
					}

					/* Copy */
					while (count--)
					{
						src_offs %= src_len;
						b2 = blt_read(src,src_offs) << shift;
						src_offs++;

						dst_offs &= 0xffff;
						blt_write(space,tmap,dst_offs,b2,mask);
						dst_offs = ((dst_offs+1) & (0x100-1)) | (dst_offs & (~(0x100-1)));
					}
					break;


				case 1:

					/* Fill with an increasing value */
					src_offs %= src_len;
					b2 = blt_read(src,src_offs);
					src_offs++;

					while (count--)
					{
						dst_offs &= 0xffff;
						blt_write(space,tmap,dst_offs,b2<<shift,mask);
						dst_offs = ((dst_offs+1) & (0x100-1)) | (dst_offs & (~(0x100-1)));
						b2++;
					}
					break;


				case 2:

					/* Fill with a fixed value */
					src_offs %= src_len;
					b2 = blt_read(src,src_offs) << shift;
					src_offs++;

					while (count--)
					{
						dst_offs &= 0xffff;
						blt_write(space,tmap,dst_offs,b2,mask);
						dst_offs = ((dst_offs+1) & (0x100-1)) | (dst_offs & (~(0x100-1)));
					}
					break;


				case 3:

					/* Skip to the next line ?? */
					if (b1 == 0xC0)
					{
						dst_offs +=   0x100;
						dst_offs &= ~(0x100-1);
						dst_offs |=  (0x100-1) & (hyprduel_blitter_regs[ 0x0a / 2 ] >> (7+1));
					}
					else
					{
						dst_offs += count;
					}
					break;


				default:
					logerror("CPU #0 PC %06X : Blitter unknown opcode %02X at %06X\n",cpu_get_pc(space->cpu),b1,src_offs-1);
					return;
			}

		}
	}

}

/***************************************************************************
                                Memory Maps
***************************************************************************/

static ADDRESS_MAP_START( hyprduel_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x400000, 0x41ffff) AM_RAM_WRITE(hyprduel_vram_0_w) AM_BASE(&hyprduel_vram_0)		/* Layer 0 */
	AM_RANGE(0x420000, 0x43ffff) AM_RAM_WRITE(hyprduel_vram_1_w) AM_BASE(&hyprduel_vram_1)		/* Layer 1 */
	AM_RANGE(0x440000, 0x45ffff) AM_RAM_WRITE(hyprduel_vram_2_w) AM_BASE(&hyprduel_vram_2)		/* Layer 2 */
	AM_RANGE(0x460000, 0x46ffff) AM_READ(hyprduel_bankedrom_r)		/* Banked ROM */
	AM_RANGE(0x470000, 0x473fff) AM_RAM AM_WRITE(hyprduel_paletteram_w) AM_BASE(&paletteram16)	/* Palette */
	AM_RANGE(0x474000, 0x474fff) AM_RAM AM_BASE(&spriteram16) AM_SIZE(&spriteram_size)			/* Sprites */
	AM_RANGE(0x475000, 0x477fff) AM_RAM			/* only used memory test */
	AM_RANGE(0x478000, 0x4787ff) AM_RAM AM_BASE(&hyprduel_tiletable) AM_SIZE(&hyprduel_tiletable_size)	/* Tiles Set */
	AM_RANGE(0x478840, 0x47884d) AM_WRITE(hyprduel_blitter_w) AM_BASE(&hyprduel_blitter_regs)	/* Tiles Blitter */
	AM_RANGE(0x478860, 0x47886b) AM_WRITE(hyprduel_window_w) AM_BASE(&hyprduel_window)			/* Tilemap Window */
	AM_RANGE(0x478870, 0x47887b) AM_RAM_WRITE(hyprduel_scrollreg_w) AM_BASE(&hyprduel_scroll)		/* Scroll Regs */
	AM_RANGE(0x47887c, 0x47887d) AM_WRITE(hyprduel_scrollreg_init_w)
	AM_RANGE(0x478880, 0x478881) AM_WRITENOP
	AM_RANGE(0x478890, 0x478891) AM_WRITENOP
	AM_RANGE(0x4788a0, 0x4788a1) AM_WRITENOP
	AM_RANGE(0x4788a2, 0x4788a3) AM_READWRITE(hyprduel_irq_cause_r, hyprduel_irq_cause_w)	/* IRQ Cause,Acknowledge */
	AM_RANGE(0x4788a4, 0x4788a5) AM_RAM AM_BASE(&hyprduel_irq_enable)		/* IRQ Enable */
	AM_RANGE(0x4788aa, 0x4788ab) AM_RAM AM_BASE(&hyprduel_rombank)		/* Rom Bank */
	AM_RANGE(0x4788ac, 0x4788ad) AM_RAM AM_BASE(&hyprduel_screenctrl)	/* Screen Control */
	AM_RANGE(0x479700, 0x479713) AM_RAM AM_BASE(&hyprduel_videoregs)	/* Video Registers */
	AM_RANGE(0x800000, 0x800001) AM_WRITE(hyprduel_subcpu_control_w)
	AM_RANGE(0xc00000, 0xc07fff) AM_RAM AM_SHARE(1) AM_BASE(&sharedram1)
	AM_RANGE(0xe00000, 0xe00001) AM_READ_PORT("SERVICE") AM_WRITENOP
	AM_RANGE(0xe00002, 0xe00003) AM_READ_PORT("DSW")
	AM_RANGE(0xe00004, 0xe00005) AM_READ_PORT("P1_P2")
	AM_RANGE(0xe00006, 0xe00007) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xfe0000, 0xfe3fff) AM_RAM AM_SHARE(2)
	AM_RANGE(0xfe4000, 0xffffff) AM_RAM AM_SHARE(3) AM_BASE(&sharedram3)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hyprduel_map2, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x003fff) AM_RAM AM_SHARE(1)						/* shadow ($c00000 - $c03fff : vector) */
	AM_RANGE(0x004000, 0x007fff) AM_RAM AM_WRITENOP AM_SHARE(3)			/* shadow ($fe4000 - $fe7fff : read only) */
	AM_RANGE(0x400000, 0x400003) AM_DEVREADWRITE8("ymsnd", ym2151_r, ym2151_w, 0x00ff )
	AM_RANGE(0x400004, 0x400005) AM_DEVREADWRITE8("oki", okim6295_r, okim6295_w, 0x00ff)
	AM_RANGE(0x800000, 0x800001) AM_NOP
	AM_RANGE(0xc00000, 0xc07fff) AM_RAM AM_SHARE(1)
	AM_RANGE(0xfe0000, 0xfe3fff) AM_RAM AM_SHARE(2)
	AM_RANGE(0xfe4000, 0xffffff) AM_RAM AM_SHARE(3)
ADDRESS_MAP_END


/* Magical Error - video is at 8x now */

static ADDRESS_MAP_START( magerror_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x400000, 0x400001) AM_WRITE(hyprduel_subcpu_control_w)
	AM_RANGE(0x800000, 0x81ffff) AM_RAM_WRITE(hyprduel_vram_0_w) AM_BASE(&hyprduel_vram_0)		/* Layer 0 */
	AM_RANGE(0x820000, 0x83ffff) AM_RAM_WRITE(hyprduel_vram_1_w) AM_BASE(&hyprduel_vram_1)		/* Layer 1 */
	AM_RANGE(0x840000, 0x85ffff) AM_RAM_WRITE(hyprduel_vram_2_w) AM_BASE(&hyprduel_vram_2)		/* Layer 2 */
	AM_RANGE(0x860000, 0x86ffff) AM_READ(hyprduel_bankedrom_r)		/* Banked ROM */
	AM_RANGE(0x870000, 0x873fff) AM_RAM AM_WRITE(hyprduel_paletteram_w) AM_BASE(&paletteram16)	/* Palette */
	AM_RANGE(0x874000, 0x874fff) AM_RAM AM_BASE(&spriteram16) AM_SIZE(&spriteram_size	)		/* Sprites */
	AM_RANGE(0x875000, 0x877fff) AM_RAM			/* only used memory test */
	AM_RANGE(0x878000, 0x8787ff) AM_RAM AM_BASE(&hyprduel_tiletable) AM_SIZE(&hyprduel_tiletable_size)	/* Tiles Set */
	AM_RANGE(0x878840, 0x87884d) AM_WRITE(hyprduel_blitter_w) AM_BASE(&hyprduel_blitter_regs)	/* Tiles Blitter */
	AM_RANGE(0x878860, 0x87886b) AM_WRITE(hyprduel_window_w) AM_BASE(&hyprduel_window)			/* Tilemap Window */
	AM_RANGE(0x878870, 0x87887b) AM_RAM_WRITE(hyprduel_scrollreg_w) AM_BASE(&hyprduel_scroll)		/* Scroll Regs */
	AM_RANGE(0x87887c, 0x87887d) AM_WRITE(hyprduel_scrollreg_init_w)
	AM_RANGE(0x878880, 0x878881) AM_WRITENOP
	AM_RANGE(0x878890, 0x878891) AM_WRITENOP
	AM_RANGE(0x8788a0, 0x8788a1) AM_WRITENOP
	AM_RANGE(0x8788a2, 0x8788a3) AM_READWRITE(hyprduel_irq_cause_r, hyprduel_irq_cause_w)	/* IRQ Cause, Acknowledge */
	AM_RANGE(0x8788a4, 0x8788a5) AM_RAM AM_BASE(&hyprduel_irq_enable)		/* IRQ Enable */
	AM_RANGE(0x8788aa, 0x8788ab) AM_RAM AM_BASE(&hyprduel_rombank)		/* Rom Bank */
	AM_RANGE(0x8788ac, 0x8788ad) AM_RAM AM_BASE(&hyprduel_screenctrl)	/* Screen Control */
	AM_RANGE(0x879700, 0x879713) AM_RAM AM_BASE(&hyprduel_videoregs)	/* Video Registers */
	AM_RANGE(0xc00000, 0xc1ffff) AM_RAM AM_SHARE(1) AM_BASE(&sharedram1)
	AM_RANGE(0xe00000, 0xe00001) AM_READ_PORT("SERVICE") AM_WRITENOP
	AM_RANGE(0xe00002, 0xe00003) AM_READ_PORT("DSW")
	AM_RANGE(0xe00004, 0xe00005) AM_READ_PORT("P1_P2")
	AM_RANGE(0xe00006, 0xe00007) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xfe0000, 0xfe3fff) AM_RAM AM_SHARE(2)
	AM_RANGE(0xfe4000, 0xffffff) AM_RAM AM_SHARE(3) AM_BASE(&sharedram3)
ADDRESS_MAP_END

static ADDRESS_MAP_START( magerror_map2, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x003fff) AM_RAM AM_SHARE(1)						/* shadow ($c00000 - $c03fff : vector) */
	AM_RANGE(0x004000, 0x007fff) AM_RAM AM_WRITENOP AM_SHARE(3)			/* shadow ($fe4000 - $fe7fff : read only) */
	AM_RANGE(0x400000, 0x400003) AM_NOP
	AM_RANGE(0x800000, 0x800003) AM_READNOP AM_DEVWRITE8("ymsnd", ym2413_w, 0x00ff)
	AM_RANGE(0x800004, 0x800005) AM_DEVREADWRITE8("oki", okim6295_r, okim6295_w, 0x00ff)
	AM_RANGE(0xc00000, 0xc1ffff) AM_RAM AM_SHARE(1)
	AM_RANGE(0xfe0000, 0xfe3fff) AM_RAM AM_SHARE(2)
	AM_RANGE(0xfe4000, 0xffffff) AM_RAM AM_SHARE(3)
ADDRESS_MAP_END

/***************************************************************************
                                Input Ports
***************************************************************************/

#define JOY_LSB(_n_, _b1_, _b2_, _b3_, _b4_) \
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	 ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_##_b1_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_##_b2_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_##_b3_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_##_b4_         ) PORT_PLAYER(_n_) \

#define JOY_MSB(_n_, _b1_, _b2_, _b3_, _b4_) \
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	 ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_##_b1_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_##_b2_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_##_b3_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_##_b4_         ) PORT_PLAYER(_n_) \


static INPUT_PORTS_START( hyprduel )
	PORT_START("SERVICE")
	PORT_SERVICE_NO_TOGGLE( 0x8000, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x4000, 0x0000, "Show Warning" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x3fff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Start Up Mode" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(     0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x2000, "2" )
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x1000, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_BIT(     0xc000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_P2")
	JOY_LSB(1, BUTTON1, BUTTON2, BUTTON3, UNKNOWN)
	JOY_MSB(2, BUTTON1, BUTTON2, BUTTON3, UNKNOWN)

	PORT_START("SYSTEM")
	PORT_BIT(  0x0001, IP_ACTIVE_LOW,  IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW,  IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_START1   )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_START2   )
	PORT_BIT(  0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( magerror )
	PORT_INCLUDE( hyprduel )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Start Up Mode" )
	PORT_DIPSETTING(      0x0080, "Game Mode" )
	PORT_DIPSETTING(      0x0000, "Test Mode" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(     0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x2000, "2" )
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x1000, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_BIT(     0xc000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************
                            Graphics Layouts
***************************************************************************/

/* 8x8x4 tiles */
static GFXLAYOUT_RAW( layout_8x8x4, 4, 8, 8, 4*8, 32*8 )

/* 8x8x8 tiles for later games */
static GFXLAYOUT_RAW( layout_8x8x8h, 8, 8, 8, 8*8, 32*8 )

static GFXDECODE_START( 14220 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x4,    0x0, 0x200 ) // [0] 4 Bit Tiles
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x8h,   0x0,  0x20 ) // [1] 8 Bit Tiles
GFXDECODE_END

/***************************************************************************
                            Sound Communication
***************************************************************************/

static void sound_irq(const device_config *device, int state)
{
	cputag_set_input_line(device->machine, "sub", 1, HOLD_LINE);
}

static const ym2151_interface ym2151_config =
{
	sound_irq
};

/***************************************************************************
                                Machine Drivers
***************************************************************************/

static MACHINE_RESET( hyprduel )
{
	/* start with cpu2 halted */
	cputag_set_input_line(machine, "sub", INPUT_LINE_RESET, ASSERT_LINE);
	subcpu_resetline = 1;
	cpu_trigger = 0;

	requested_int = 0x00;
	blitter_bit = 2;
	*hyprduel_irq_enable = 0xff;
}

static MACHINE_START( magerror )
{
	timer_adjust_periodic(magerror_irq_timer, attotime_zero, 0, ATTOTIME_IN_HZ(968));		/* tempo? */
}

static MACHINE_DRIVER_START( hyprduel )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000,20000000/2)		/* 10MHz */
	MDRV_CPU_PROGRAM_MAP(hyprduel_map)
	MDRV_CPU_VBLANK_INT_HACK(hyprduel_interrupt,RASTER_LINES)

	MDRV_CPU_ADD("sub", M68000,20000000/2)		/* 10MHz */
	MDRV_CPU_PROGRAM_MAP(hyprduel_map2)

	MDRV_MACHINE_RESET(hyprduel)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_SCANLINE)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 224)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, FIRST_VISIBLE_LINE, LAST_VISIBLE_LINE)

	MDRV_GFXDECODE(14220)
	MDRV_PALETTE_LENGTH(8192)

	MDRV_VIDEO_START(hyprduel_14220)
	MDRV_VIDEO_UPDATE(hyprduel)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 4000000)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.80)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.80)

	MDRV_SOUND_ADD("oki", OKIM6295, 4000000/16/16*132)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.57)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.57)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( magerror )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000,20000000/2)		/* 10MHz */
	MDRV_CPU_PROGRAM_MAP(magerror_map)
	MDRV_CPU_VBLANK_INT_HACK(hyprduel_interrupt,RASTER_LINES)

	MDRV_CPU_ADD("sub", M68000,20000000/2)		/* 10MHz */
	MDRV_CPU_PROGRAM_MAP(magerror_map2)

	MDRV_MACHINE_START(magerror)
	MDRV_MACHINE_RESET(hyprduel)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_SCANLINE)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 224)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, FIRST_VISIBLE_LINE, LAST_VISIBLE_LINE)

	MDRV_GFXDECODE(14220)
	MDRV_PALETTE_LENGTH(8192)

	MDRV_VIDEO_START(hyprduel_14220)
	MDRV_VIDEO_UPDATE(hyprduel)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2413, 3579545)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.00)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.00)

	MDRV_SOUND_ADD("oki", OKIM6295, 4000000/16/16*132)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.57)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.57)
MACHINE_DRIVER_END

/***************************************************************************
                                ROMs Loading
***************************************************************************/

ROM_START( hyprduel )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "24.u24", 0x000000, 0x40000, CRC(c7402722) SHA1(e385676cdcee65a3ddf07791d82a1fe83ba1b3e2) ) /* Also silk screened as position 10 */
	ROM_LOAD16_BYTE( "23.u23", 0x000001, 0x40000, CRC(d8297c2b) SHA1(2e23c5b1784d0a465c0c0dc3ca28505689a8b16c) ) /* Also silk screened as position  9 */

	ROM_REGION( 0x400000, "gfx1", 0 )	/* Gfx + Prg + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "ts_hyper-1.u74", 0x000000, 0x100000, CRC(4b3b2d3c) SHA1(5e9e8ec853f71aeff3910b93dadbaeae2b61717b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ts_hyper-2.u75", 0x000002, 0x100000, CRC(dc230116) SHA1(a3c447657d8499764f52c81382961f425c56037b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ts_hyper-3.u76", 0x000004, 0x100000, CRC(2d770dd0) SHA1(27f9e7f67e96210d3710ab4f940c5d7ae13f8bbf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ts_hyper-4.u77", 0x000006, 0x100000, CRC(f88c6d33) SHA1(277b56df40a17d7dd9f1071b0d498635a5b783cd) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "97.u97", 0x00000, 0x40000, CRC(bf3f8574) SHA1(9e743f05e53256c886d43e1f0c43d7417134b9b3) ) /* Also silk screened as position 11 */
ROM_END

ROM_START( hyprdelj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "24a.u24", 0x000000, 0x40000, CRC(2458f91d) SHA1(c75c7bccc84738e29b35667793491a1213aea1da) ) /* Also silk screened as position 10 */
	ROM_LOAD16_BYTE( "23a.u23", 0x000001, 0x40000, CRC(98aedfca) SHA1(42028e57ac79473cde683be2100b953ff3b2b345) ) /* Also silk screened as position  9 */

	ROM_REGION( 0x400000, "gfx1", 0 )	/* Gfx + Prg + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "ts_hyper-1.u74", 0x000000, 0x100000, CRC(4b3b2d3c) SHA1(5e9e8ec853f71aeff3910b93dadbaeae2b61717b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ts_hyper-2.u75", 0x000002, 0x100000, CRC(dc230116) SHA1(a3c447657d8499764f52c81382961f425c56037b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ts_hyper-3.u76", 0x000004, 0x100000, CRC(2d770dd0) SHA1(27f9e7f67e96210d3710ab4f940c5d7ae13f8bbf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ts_hyper-4.u77", 0x000006, 0x100000, CRC(f88c6d33) SHA1(277b56df40a17d7dd9f1071b0d498635a5b783cd) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "97.u97", 0x00000, 0x40000, CRC(bf3f8574) SHA1(9e743f05e53256c886d43e1f0c43d7417134b9b3) ) /* Also silk screened as position 11 */
ROM_END

ROM_START( magerror )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "24.u24", 0x000000, 0x40000, CRC(5e78027f) SHA1(053374942bc545a92cc6f6ab6784c4626e4ec9e1) ) /* Also silk screened as position 10 */
	ROM_LOAD16_BYTE( "23.u23", 0x000001, 0x40000, CRC(7271ec70) SHA1(bd7666390b70821f90ba976a3afe3194fb119478) ) /* Also silk screened as position  9 */

	ROM_REGION( 0x400000, "gfx1", 0 )	/* Gfx + Prg + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "mr93046-02.u74", 0x000000, 0x100000, CRC(f7ba06fb) SHA1(e1407b0d03863f434b68183c01e8547612e5c5fd) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mr93046-04.u75", 0x000002, 0x100000, CRC(8c114d15) SHA1(4eb1f82e7992deb126633287cb4fd2a6d215346c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mr93046-01.u76", 0x000004, 0x100000, CRC(6cc3b928) SHA1(f19d0add314867bfb7dcefe8e7a2d50a84530df7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mr93046-03.u77", 0x000006, 0x100000, CRC(6b1eb0ea) SHA1(6167a61562ef28147a7917c692f181f3fc2d5be6) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "97.u97", 0x00000, 0x40000, CRC(2e62bca8) SHA1(191fff11186dbbc1d9d9f3ba1b6e17c38a7d2d1d) ) /* Also silk screened as position 11 */
ROM_END


static void savestate(running_machine *machine)
{
	/* Set up save state */
	state_save_register_global(machine, blitter_bit);
	state_save_register_global(machine, requested_int);
	state_save_register_global(machine, subcpu_resetline);
	state_save_register_global(machine, cpu_trigger);
	state_save_register_global(machine, int_num);
}

static DRIVER_INIT( hyprduel )
{
	int_num = 0x02;
	savestate(machine);

	/* cpu synchronization (severe timings) */
	memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc0040e, 0xc00411, 0, 0, hyprduel_cpusync_trigger1_w);
	memory_install_read16_handler(cputag_get_address_space(machine, "sub", ADDRESS_SPACE_PROGRAM), 0xc00408, 0xc00409, 0, 0, hyprduel_cpusync_trigger1_r);
	memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00408, 0xc00409, 0, 0, hyprduel_cpusync_trigger2_w);
	memory_install_read16_handler(cputag_get_address_space(machine, "sub", ADDRESS_SPACE_PROGRAM), 0xfff34c, 0xfff34d, 0, 0, hyprduel_cpusync_trigger2_r);
}

static DRIVER_INIT( magerror )
{
	int_num = 0x01;
	savestate(machine);

	magerror_irq_timer = timer_alloc(machine, magerror_irq_callback, NULL);
}


GAME( 1993, hyprduel, 0,        hyprduel, hyprduel, hyprduel, ROT0, "Technosoft", "Hyper Duel (Japan set 1)", GAME_SUPPORTS_SAVE )
GAME( 1993, hyprdelj, hyprduel, hyprduel, hyprduel, hyprduel, ROT0, "Technosoft", "Hyper Duel (Japan set 2)", GAME_SUPPORTS_SAVE )
GAME( 1994, magerror, 0,        magerror, magerror, magerror, ROT0, "Technosoft / Jaleco", "Magical Error wo Sagase", GAME_SUPPORTS_SAVE )
