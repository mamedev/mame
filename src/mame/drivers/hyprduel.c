// license:BSD-3-Clause
// copyright-holders:Luca Elia, Hau
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

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "sound/2413intf.h"
#include "includes/hyprduel.h"

/***************************************************************************
                                Interrupts
***************************************************************************/

void hyprduel_state::update_irq_state(  )
{
	int irq = m_requested_int & ~*m_irq_enable;

	m_maincpu->set_input_line(3, (irq & m_int_num) ? ASSERT_LINE : CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(hyprduel_state::vblank_end_callback)
{
	m_requested_int &= ~param;
}

TIMER_DEVICE_CALLBACK_MEMBER(hyprduel_state::hyprduel_interrupt)
{
	int line = param;

	if (line == 0) /* TODO: fix this! */
	{
		m_requested_int |= 0x01;        /* vblank */
		m_requested_int |= 0x20;
		m_maincpu->set_input_line(2, HOLD_LINE);
		/* the duration is a guess */
		machine().scheduler().timer_set(attotime::from_usec(2500), timer_expired_delegate(FUNC(hyprduel_state::vblank_end_callback),this), 0x20);
	}
	else
		m_requested_int |= 0x12;        /* hsync */

	update_irq_state();
}

READ16_MEMBER(hyprduel_state::hyprduel_irq_cause_r)
{
	return m_requested_int;
}

WRITE16_MEMBER(hyprduel_state::hyprduel_irq_cause_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (data == m_int_num)
			m_requested_int &= ~(m_int_num & ~*m_irq_enable);
		else
			m_requested_int &= ~(data & *m_irq_enable);

		update_irq_state();
	}
}


WRITE16_MEMBER(hyprduel_state::hyprduel_subcpu_control_w)
{
	switch (data)
	{
		case 0x0d:
		case 0x0f:
		case 0x01:
			if (!m_subcpu_resetline)
			{
				m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
				m_subcpu_resetline = 1;
			}
			break;

		case 0x00:
			if (m_subcpu_resetline)
			{
				m_subcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
				m_subcpu_resetline = 0;
			}
			space.device().execute().spin_until_interrupt();
			break;

		case 0x0c:
		case 0x80:
			m_subcpu->set_input_line(2, HOLD_LINE);
			break;
	}
}


READ16_MEMBER(hyprduel_state::hyprduel_cpusync_trigger1_r)
{
	if (m_cpu_trigger == 1001)
	{
		machine().scheduler().trigger(1001);
		m_cpu_trigger = 0;
	}

	return m_sharedram1[0x000408 / 2 + offset];
}

WRITE16_MEMBER(hyprduel_state::hyprduel_cpusync_trigger1_w)
{
	COMBINE_DATA(&m_sharedram1[0x00040e / 2 + offset]);

	if (((m_sharedram1[0x00040e / 2] << 16) + m_sharedram1[0x000410 / 2]) != 0x00)
	{
		if (!m_cpu_trigger && !m_subcpu_resetline)
		{
			space.device().execute().spin_until_trigger(1001);
			m_cpu_trigger = 1001;
		}
	}
}


READ16_MEMBER(hyprduel_state::hyprduel_cpusync_trigger2_r)
{
	if (m_cpu_trigger == 1002)
	{
		machine().scheduler().trigger(1002);
		m_cpu_trigger = 0;
	}

	return m_sharedram3[(0xfff34c - 0xfe4000) / 2 + offset];
}

WRITE16_MEMBER(hyprduel_state::hyprduel_cpusync_trigger2_w)
{
	COMBINE_DATA(&m_sharedram1[0x000408 / 2 + offset]);

	if (ACCESSING_BITS_8_15)
	{
		if (!m_cpu_trigger && !m_subcpu_resetline)
		{
			space.device().execute().spin_until_trigger(1002);
			m_cpu_trigger = 1002;
		}
	}
}


TIMER_CALLBACK_MEMBER(hyprduel_state::magerror_irq_callback)
{
	m_subcpu->set_input_line(1, HOLD_LINE);
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

READ16_MEMBER(hyprduel_state::hyprduel_bankedrom_r)
{
	UINT8 *ROM = memregion("gfx1")->base();
	size_t  len = memregion("gfx1")->bytes();

	offset = offset * 2 + 0x10000 * (*m_rombank);

	if (offset < len)
		return ((ROM[offset + 0] << 8) + ROM[offset + 1]);
	else
		return 0xffff;
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

TIMER_CALLBACK_MEMBER(hyprduel_state::hyprduel_blit_done)
{
	m_requested_int |= 1 << m_blitter_bit;
	update_irq_state();
}

inline int hyprduel_state::blt_read( const UINT8 *ROM, const int offs )
{
	return ROM[offs];
}

void hyprduel_state::blt_write( address_space &space, const int tmap, const offs_t offs, const UINT16 data, const UINT16 mask )
{
	switch( tmap )
	{
		case 1: hyprduel_vram_0_w(space, offs,data,mask);   break;
		case 2: hyprduel_vram_1_w(space, offs, data, mask); break;
		case 3: hyprduel_vram_2_w(space, offs, data, mask); break;
	}
//  logerror("%s : Blitter %X] %04X <- %04X & %04X\n", space.machine().describe_context(), tmap, offs, data, mask);
}


WRITE16_MEMBER(hyprduel_state::hyprduel_blitter_w)
{
	COMBINE_DATA(&m_blitter_regs[offset]);

	if (offset == 0xc / 2)
	{
		UINT8 *src = memregion("gfx1")->base();
		size_t  src_len = memregion("gfx1")->bytes();

		UINT32 tmap = (m_blitter_regs[0x00 / 2] << 16) + m_blitter_regs[0x02 / 2];
		UINT32 src_offs = (m_blitter_regs[0x04 / 2] << 16) + m_blitter_regs[0x06 / 2];
		UINT32 dst_offs = (m_blitter_regs[0x08 / 2] << 16) + m_blitter_regs[0x0a / 2];

		int shift = (dst_offs & 0x80) ? 0 : 8;
		UINT16 mask = (dst_offs & 0x80) ? 0x00ff : 0xff00;

//      logerror("CPU #0 PC %06X : Blitter regs %08X, %08X, %08X\n", space.device().safe_pc(), tmap, src_offs, dst_offs);

		dst_offs >>= 7 + 1;
		switch (tmap)
		{
			case 1:
			case 2:
			case 3:
				break;
			default:
				logerror("CPU #0 PC %06X : Blitter unknown destination: %08X\n", space.device().safe_pc(), tmap);
				return;
		}

		while (1)
		{
			UINT16 b1, b2, count;

			src_offs %= src_len;
			b1 = blt_read(src, src_offs);
//          logerror("CPU #0 PC %06X : Blitter opcode %02X at %06X\n", space.device().safe_pc(), b1, src_offs);
			src_offs++;

			count = ((~b1) & 0x3f) + 1;

			switch ((b1 & 0xc0) >> 6)
			{
				case 0:

					/* Stop and Generate an IRQ. We can't generate it now
					   both because it's unlikely that the blitter is so
					   fast and because some games (e.g. lastfort) need to
					   complete the blitter irq service routine before doing
					   another blit. */
					if (b1 == 0)
					{
						machine().scheduler().timer_set(attotime::from_usec(500), timer_expired_delegate(FUNC(hyprduel_state::hyprduel_blit_done),this));
						return;
					}

					/* Copy */
					while (count--)
					{
						src_offs %= src_len;
						b2 = blt_read(src, src_offs) << shift;
						src_offs++;

						dst_offs &= 0xffff;
						blt_write(space, tmap, dst_offs, b2, mask);
						dst_offs = ((dst_offs + 1) & (0x100 - 1)) | (dst_offs & (~(0x100 - 1)));
					}
					break;


				case 1:

					/* Fill with an increasing value */
					src_offs %= src_len;
					b2 = blt_read(src, src_offs);
					src_offs++;

					while (count--)
					{
						dst_offs &= 0xffff;
						blt_write(space, tmap, dst_offs, b2 << shift, mask);
						dst_offs = ((dst_offs + 1) & (0x100 - 1)) | (dst_offs & (~(0x100 - 1)));
						b2++;
					}
					break;


				case 2:

					/* Fill with a fixed value */
					src_offs %= src_len;
					b2 = blt_read(src, src_offs) << shift;
					src_offs++;

					while (count--)
					{
						dst_offs &= 0xffff;
						blt_write(space, tmap, dst_offs, b2, mask);
						dst_offs = ((dst_offs + 1) & (0x100 - 1)) | (dst_offs & (~(0x100 - 1)));
					}
					break;


				case 3:

					/* Skip to the next line ?? */
					if (b1 == 0xC0)
					{
						dst_offs += 0x100;
						dst_offs &= ~(0x100 - 1);
						dst_offs |= (0x100 - 1) & (m_blitter_regs[0x0a / 2] >> (7 + 1));
					}
					else
					{
						dst_offs += count;
					}
					break;


				default:
					logerror("CPU #0 PC %06X : Blitter unknown opcode %02X at %06X\n", space.device().safe_pc(), b1, src_offs - 1);
					return;
			}

		}
	}

}

/***************************************************************************
                                Memory Maps
***************************************************************************/

static ADDRESS_MAP_START( hyprduel_map, AS_PROGRAM, 16, hyprduel_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x400000, 0x41ffff) AM_RAM_WRITE(hyprduel_vram_0_w) AM_SHARE("vram_0")     /* Layer 0 */
	AM_RANGE(0x420000, 0x43ffff) AM_RAM_WRITE(hyprduel_vram_1_w) AM_SHARE("vram_1")     /* Layer 1 */
	AM_RANGE(0x440000, 0x45ffff) AM_RAM_WRITE(hyprduel_vram_2_w) AM_SHARE("vram_2")     /* Layer 2 */
	AM_RANGE(0x460000, 0x46ffff) AM_READ(hyprduel_bankedrom_r)      /* Banked ROM */
	AM_RANGE(0x470000, 0x473fff) AM_RAM_WRITE(hyprduel_paletteram_w) AM_SHARE("paletteram") /* Palette */
	AM_RANGE(0x474000, 0x474fff) AM_RAM AM_SHARE("spriteram")           /* Sprites */
	AM_RANGE(0x475000, 0x477fff) AM_RAM         /* only used memory test */
	AM_RANGE(0x478000, 0x4787ff) AM_RAM AM_SHARE("tiletable")   /* Tiles Set */
	AM_RANGE(0x478840, 0x47884d) AM_WRITE(hyprduel_blitter_w) AM_SHARE("blitter_regs")  /* Tiles Blitter */
	AM_RANGE(0x478860, 0x47886b) AM_WRITE(hyprduel_window_w) AM_SHARE("window")         /* Tilemap Window */
	AM_RANGE(0x478870, 0x47887b) AM_RAM_WRITE(hyprduel_scrollreg_w) AM_SHARE("scroll")      /* Scroll Regs */
	AM_RANGE(0x47887c, 0x47887d) AM_WRITE(hyprduel_scrollreg_init_w)
	AM_RANGE(0x478880, 0x478881) AM_WRITENOP
	AM_RANGE(0x478890, 0x478891) AM_WRITENOP
	AM_RANGE(0x4788a0, 0x4788a1) AM_WRITENOP
	AM_RANGE(0x4788a2, 0x4788a3) AM_READWRITE(hyprduel_irq_cause_r, hyprduel_irq_cause_w)   /* IRQ Cause,Acknowledge */
	AM_RANGE(0x4788a4, 0x4788a5) AM_RAM AM_SHARE("irq_enable")      /* IRQ Enable */
	AM_RANGE(0x4788aa, 0x4788ab) AM_RAM AM_SHARE("rombank")     /* Rom Bank */
	AM_RANGE(0x4788ac, 0x4788ad) AM_RAM AM_SHARE("screenctrl")  /* Screen Control */
	AM_RANGE(0x479700, 0x479713) AM_RAM AM_SHARE("videoregs")   /* Video Registers */
	AM_RANGE(0x800000, 0x800001) AM_WRITE(hyprduel_subcpu_control_w)
	AM_RANGE(0xc00000, 0xc07fff) AM_RAM AM_SHARE("sharedram1")
	AM_RANGE(0xe00000, 0xe00001) AM_READ_PORT("SERVICE") AM_WRITENOP
	AM_RANGE(0xe00002, 0xe00003) AM_READ_PORT("DSW")
	AM_RANGE(0xe00004, 0xe00005) AM_READ_PORT("P1_P2")
	AM_RANGE(0xe00006, 0xe00007) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xfe0000, 0xfe3fff) AM_RAM AM_SHARE("sharedram2")
	AM_RANGE(0xfe4000, 0xffffff) AM_RAM AM_SHARE("sharedram3")
ADDRESS_MAP_END

static ADDRESS_MAP_START( hyprduel_map2, AS_PROGRAM, 16, hyprduel_state )
	AM_RANGE(0x000000, 0x003fff) AM_RAM AM_SHARE("sharedram1")                      /* shadow ($c00000 - $c03fff : vector) */
	AM_RANGE(0x004000, 0x007fff) AM_READONLY AM_WRITENOP AM_SHARE("sharedram3")         /* shadow ($fe4000 - $fe7fff : read only) */
	AM_RANGE(0x400000, 0x400003) AM_DEVREADWRITE8("ymsnd", ym2151_device, read, write, 0x00ff )
	AM_RANGE(0x400004, 0x400005) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x800000, 0x800001) AM_NOP
	AM_RANGE(0xc00000, 0xc07fff) AM_RAM AM_SHARE("sharedram1")
	AM_RANGE(0xfe0000, 0xfe3fff) AM_RAM AM_SHARE("sharedram2")
	AM_RANGE(0xfe4000, 0xffffff) AM_RAM AM_SHARE("sharedram3")
ADDRESS_MAP_END


/* Magical Error - video is at 8x now */

static ADDRESS_MAP_START( magerror_map, AS_PROGRAM, 16, hyprduel_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x400000, 0x400001) AM_WRITE(hyprduel_subcpu_control_w)
	AM_RANGE(0x800000, 0x81ffff) AM_RAM_WRITE(hyprduel_vram_0_w) AM_SHARE("vram_0")     /* Layer 0 */
	AM_RANGE(0x820000, 0x83ffff) AM_RAM_WRITE(hyprduel_vram_1_w) AM_SHARE("vram_1")     /* Layer 1 */
	AM_RANGE(0x840000, 0x85ffff) AM_RAM_WRITE(hyprduel_vram_2_w) AM_SHARE("vram_2")     /* Layer 2 */
	AM_RANGE(0x860000, 0x86ffff) AM_READ(hyprduel_bankedrom_r)      /* Banked ROM */
	AM_RANGE(0x870000, 0x873fff) AM_RAM_WRITE(hyprduel_paletteram_w) AM_SHARE("paletteram") /* Palette */
	AM_RANGE(0x874000, 0x874fff) AM_RAM AM_SHARE("spriteram")       /* Sprites */
	AM_RANGE(0x875000, 0x877fff) AM_RAM         /* only used memory test */
	AM_RANGE(0x878000, 0x8787ff) AM_RAM AM_SHARE("tiletable")   /* Tiles Set */
	AM_RANGE(0x878840, 0x87884d) AM_WRITE(hyprduel_blitter_w) AM_SHARE("blitter_regs")  /* Tiles Blitter */
	AM_RANGE(0x878860, 0x87886b) AM_WRITE(hyprduel_window_w) AM_SHARE("window")         /* Tilemap Window */
	AM_RANGE(0x878870, 0x87887b) AM_RAM_WRITE(hyprduel_scrollreg_w) AM_SHARE("scroll")      /* Scroll Regs */
	AM_RANGE(0x87887c, 0x87887d) AM_WRITE(hyprduel_scrollreg_init_w)
	AM_RANGE(0x878880, 0x878881) AM_WRITENOP
	AM_RANGE(0x878890, 0x878891) AM_WRITENOP
	AM_RANGE(0x8788a0, 0x8788a1) AM_WRITENOP
	AM_RANGE(0x8788a2, 0x8788a3) AM_READWRITE(hyprduel_irq_cause_r, hyprduel_irq_cause_w)   /* IRQ Cause, Acknowledge */
	AM_RANGE(0x8788a4, 0x8788a5) AM_RAM AM_SHARE("irq_enable")      /* IRQ Enable */
	AM_RANGE(0x8788aa, 0x8788ab) AM_RAM AM_SHARE("rombank")     /* Rom Bank */
	AM_RANGE(0x8788ac, 0x8788ad) AM_RAM AM_SHARE("screenctrl")  /* Screen Control */
	AM_RANGE(0x879700, 0x879713) AM_RAM AM_SHARE("videoregs")   /* Video Registers */
	AM_RANGE(0xc00000, 0xc1ffff) AM_RAM AM_SHARE("sharedram1")
	AM_RANGE(0xe00000, 0xe00001) AM_READ_PORT("SERVICE") AM_WRITENOP
	AM_RANGE(0xe00002, 0xe00003) AM_READ_PORT("DSW")
	AM_RANGE(0xe00004, 0xe00005) AM_READ_PORT("P1_P2")
	AM_RANGE(0xe00006, 0xe00007) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xfe0000, 0xfe3fff) AM_RAM AM_SHARE("sharedram2")
	AM_RANGE(0xfe4000, 0xffffff) AM_RAM AM_SHARE("sharedram3")
ADDRESS_MAP_END

static ADDRESS_MAP_START( magerror_map2, AS_PROGRAM, 16, hyprduel_state )
	AM_RANGE(0x000000, 0x003fff) AM_RAM AM_SHARE("sharedram1")                      /* shadow ($c00000 - $c03fff : vector) */
	AM_RANGE(0x004000, 0x007fff) AM_READONLY AM_WRITENOP AM_SHARE("sharedram3")     /* shadow ($fe4000 - $fe7fff : read only) */
	AM_RANGE(0x400000, 0x400003) AM_NOP
	AM_RANGE(0x800000, 0x800003) AM_READNOP AM_DEVWRITE8("ymsnd", ym2413_device, write, 0x00ff)
	AM_RANGE(0x800004, 0x800005) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0xc00000, 0xc1ffff) AM_RAM AM_SHARE("sharedram1")
	AM_RANGE(0xfe0000, 0xfe3fff) AM_RAM AM_SHARE("sharedram2")
	AM_RANGE(0xfe4000, 0xffffff) AM_RAM AM_SHARE("sharedram3")
ADDRESS_MAP_END

/***************************************************************************
                                Input Ports
***************************************************************************/

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
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(  0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( magerror )
	PORT_INCLUDE( hyprduel )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0080, 0x0080, "Start Up Mode" )
	PORT_DIPSETTING(      0x0080, "Game Mode" )
	PORT_DIPSETTING(      0x0000, "Test Mode" )
INPUT_PORTS_END

/***************************************************************************
                            Graphics Layouts
***************************************************************************/

/* 8x8x4 tiles */
static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 4*1,4*0, 4*3,4*2, 4*5,4*4, 4*7,4*6 },
	{ STEP8(0,4*8) },
	4*8*8
};

/* 8x8x8 tiles for later games */
static GFXLAYOUT_RAW( layout_8x8x8h, 8, 8, 8*8, 32*8 )

static GFXDECODE_START( 14220 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x4,    0x0, 0x200 ) // [0] 4 Bit Tiles
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x8h,   0x0,  0x20 ) // [1] 8 Bit Tiles
GFXDECODE_END

/***************************************************************************
                                Machine Drivers
***************************************************************************/

void hyprduel_state::machine_reset()
{
	/* start with cpu2 halted */
	m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_subcpu_resetline = 1;
	m_cpu_trigger = 0;

	m_requested_int = 0x00;
	m_blitter_bit = 2;
	*m_irq_enable = 0xff;
}

MACHINE_START_MEMBER(hyprduel_state,hyprduel)
{
	save_item(NAME(m_blitter_bit));
	save_item(NAME(m_requested_int));
	save_item(NAME(m_subcpu_resetline));
	save_item(NAME(m_cpu_trigger));
}

MACHINE_START_MEMBER(hyprduel_state,magerror)
{
	MACHINE_START_CALL_MEMBER(hyprduel);
	m_magerror_irq_timer->adjust(attotime::zero, 0, attotime::from_hz(968));        /* tempo? */
}

static MACHINE_CONFIG_START( hyprduel, hyprduel_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,20000000/2)      /* 10MHz */
	MCFG_CPU_PROGRAM_MAP(hyprduel_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", hyprduel_state, hyprduel_interrupt, "screen", 0, 1)

	MCFG_CPU_ADD("sub", M68000,20000000/2)      /* 10MHz */
	MCFG_CPU_PROGRAM_MAP(hyprduel_map2)

	MCFG_MACHINE_START_OVERRIDE(hyprduel_state,hyprduel)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_SCANLINE)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 224)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, FIRST_VISIBLE_LINE, LAST_VISIBLE_LINE)
	MCFG_SCREEN_UPDATE_DRIVER(hyprduel_state, screen_update_hyprduel)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 14220)
	MCFG_PALETTE_ADD("palette", 8192)

	MCFG_VIDEO_START_OVERRIDE(hyprduel_state,hyprduel_14220)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", 4000000)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("sub", 1))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.80)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.80)

	MCFG_OKIM6295_ADD("oki", 4000000/16/16*132, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.57)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.57)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( magerror, hyprduel_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,20000000/2)      /* 10MHz */
	MCFG_CPU_PROGRAM_MAP(magerror_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", hyprduel_state, hyprduel_interrupt, "screen", 0, 1)

	MCFG_CPU_ADD("sub", M68000,20000000/2)      /* 10MHz */
	MCFG_CPU_PROGRAM_MAP(magerror_map2)

	MCFG_MACHINE_START_OVERRIDE(hyprduel_state,magerror)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_SCANLINE)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 224)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, FIRST_VISIBLE_LINE, LAST_VISIBLE_LINE)
	MCFG_SCREEN_UPDATE_DRIVER(hyprduel_state, screen_update_hyprduel)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 14220)
	MCFG_PALETTE_ADD("palette", 8192)

	MCFG_VIDEO_START_OVERRIDE(hyprduel_state,magerror_14220)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2413, 3579545)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.00)

	MCFG_OKIM6295_ADD("oki", 4000000/16/16*132, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.57)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.57)
MACHINE_CONFIG_END

/***************************************************************************
                                ROMs Loading
***************************************************************************/

ROM_START( hyprduel )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "24.u24", 0x000000, 0x40000, CRC(c7402722) SHA1(e385676cdcee65a3ddf07791d82a1fe83ba1b3e2) ) /* Also silk screened as position 10 */
	ROM_LOAD16_BYTE( "23.u23", 0x000001, 0x40000, CRC(d8297c2b) SHA1(2e23c5b1784d0a465c0c0dc3ca28505689a8b16c) ) /* Also silk screened as position  9 */

	ROM_REGION( 0x400000, "gfx1", 0 )   /* Gfx + Prg + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "ts_hyper-1.u74", 0x000000, 0x100000, CRC(4b3b2d3c) SHA1(5e9e8ec853f71aeff3910b93dadbaeae2b61717b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ts_hyper-2.u75", 0x000002, 0x100000, CRC(dc230116) SHA1(a3c447657d8499764f52c81382961f425c56037b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ts_hyper-3.u76", 0x000004, 0x100000, CRC(2d770dd0) SHA1(27f9e7f67e96210d3710ab4f940c5d7ae13f8bbf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ts_hyper-4.u77", 0x000006, 0x100000, CRC(f88c6d33) SHA1(277b56df40a17d7dd9f1071b0d498635a5b783cd) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "97.u97", 0x00000, 0x40000, CRC(bf3f8574) SHA1(9e743f05e53256c886d43e1f0c43d7417134b9b3) ) /* Also silk screened as position 11 */
ROM_END

ROM_START( hyprduel2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "24a.u24", 0x000000, 0x40000, CRC(2458f91d) SHA1(c75c7bccc84738e29b35667793491a1213aea1da) ) /* Also silk screened as position 10 */
	ROM_LOAD16_BYTE( "23a.u23", 0x000001, 0x40000, CRC(98aedfca) SHA1(42028e57ac79473cde683be2100b953ff3b2b345) ) /* Also silk screened as position  9 */

	ROM_REGION( 0x400000, "gfx1", 0 )   /* Gfx + Prg + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "ts_hyper-1.u74", 0x000000, 0x100000, CRC(4b3b2d3c) SHA1(5e9e8ec853f71aeff3910b93dadbaeae2b61717b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ts_hyper-2.u75", 0x000002, 0x100000, CRC(dc230116) SHA1(a3c447657d8499764f52c81382961f425c56037b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ts_hyper-3.u76", 0x000004, 0x100000, CRC(2d770dd0) SHA1(27f9e7f67e96210d3710ab4f940c5d7ae13f8bbf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ts_hyper-4.u77", 0x000006, 0x100000, CRC(f88c6d33) SHA1(277b56df40a17d7dd9f1071b0d498635a5b783cd) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "97.u97", 0x00000, 0x40000, CRC(bf3f8574) SHA1(9e743f05e53256c886d43e1f0c43d7417134b9b3) ) /* Also silk screened as position 11 */
ROM_END

ROM_START( magerror )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "24.u24", 0x000000, 0x40000, CRC(5e78027f) SHA1(053374942bc545a92cc6f6ab6784c4626e4ec9e1) ) /* Also silk screened as position 10 */
	ROM_LOAD16_BYTE( "23.u23", 0x000001, 0x40000, CRC(7271ec70) SHA1(bd7666390b70821f90ba976a3afe3194fb119478) ) /* Also silk screened as position  9 */

	ROM_REGION( 0x400000, "gfx1", 0 )   /* Gfx + Prg + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "mr93046-02.u74", 0x000000, 0x100000, CRC(f7ba06fb) SHA1(e1407b0d03863f434b68183c01e8547612e5c5fd) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mr93046-04.u75", 0x000002, 0x100000, CRC(8c114d15) SHA1(4eb1f82e7992deb126633287cb4fd2a6d215346c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mr93046-01.u76", 0x000004, 0x100000, CRC(6cc3b928) SHA1(f19d0add314867bfb7dcefe8e7a2d50a84530df7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mr93046-03.u77", 0x000006, 0x100000, CRC(6b1eb0ea) SHA1(6167a61562ef28147a7917c692f181f3fc2d5be6) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "97.u97", 0x00000, 0x40000, CRC(2e62bca8) SHA1(191fff11186dbbc1d9d9f3ba1b6e17c38a7d2d1d) ) /* Also silk screened as position 11 */
ROM_END


DRIVER_INIT_MEMBER(hyprduel_state,hyprduel)
{
	m_int_num = 0x02;

	/* cpu synchronization (severe timings) */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xc0040e, 0xc00411, write16_delegate(FUNC(hyprduel_state::hyprduel_cpusync_trigger1_w),this));
	m_subcpu->space(AS_PROGRAM).install_read_handler(0xc00408, 0xc00409, read16_delegate(FUNC(hyprduel_state::hyprduel_cpusync_trigger1_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xc00408, 0xc00409, write16_delegate(FUNC(hyprduel_state::hyprduel_cpusync_trigger2_w),this));
	m_subcpu->space(AS_PROGRAM).install_read_handler(0xfff34c, 0xfff34d, read16_delegate(FUNC(hyprduel_state::hyprduel_cpusync_trigger2_r),this));
}

DRIVER_INIT_MEMBER(hyprduel_state,magerror)
{
	m_int_num = 0x01;
	m_magerror_irq_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(hyprduel_state::magerror_irq_callback),this));
}


GAME( 1993, hyprduel, 0,        hyprduel, hyprduel, hyprduel_state, hyprduel, ROT0, "Technosoft", "Hyper Duel (Japan set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, hyprduel2,hyprduel, hyprduel, hyprduel, hyprduel_state, hyprduel, ROT0, "Technosoft", "Hyper Duel (Japan set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, magerror, 0,        magerror, magerror, hyprduel_state, magerror, ROT0, "Technosoft / Jaleco", "Magical Error wo Sagase", MACHINE_SUPPORTS_SAVE )
