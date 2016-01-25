// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/***************************************************************************

                              -= Metro Games =-

                    driver by   Luca Elia (l.elia@tin.it)


Main  CPU    :  MC68000 Or H8/3007

Video Chips  :  Imagetek I4100 052  Or
                Imagetek I4220 071  Or
                Imagetek I4300 095

Sound CPU    :  NEC78C10 [Optional]

Sound Chips  :  OKIM6295 + YM2413  or
                 YMF278B + YRW801-M

Other        :  Memory Blitter

------------------------------------------------------------------------------------------------
Year + Game                       PCB           Video  Sub CPU  Sound         Issues / Notes
------------------------------------------------------------------------------------------------
92  Last Fortress - Toride        VG420         I4100  uPD7810  YM2413+M6295
92  Last Fortress - Toride (Ger)  VG460-(A)     I4100  uPD7810  YM2413+M6295
92  Pang Pom's                    VG420         I4100  uPD7810  YM2413+M6295
92  Sky Alert                     VG420         I4100  uPD7810  YM2413+M6295
92  The Karate Tournament         VG460-(A)     I4100  uPD7810  YM2413+M6295
92  The Karate Tournament (Japan) VG460-(A)     I4100  uPD7810  YM2413+M6295
93? Lady Killer / Moeyo Gonta!!   VG460-B       I4100  uPD7810  YM2413+M6295
93  Poitto!                       MTR5260-A     I4100  uPD7810  YM2413+M6295

94  Dharma Doujou                 MTR5260-A     I4220  uPD7810  YM2413+M6295
94  Dharma Doujou (Korea)         MTR527        I4220  uPD7810  YM2413+M6295
94  Toride II Adauchi Gaiden      MTR5260-A     I4220  uPD7810  YM2413+M6295
94  Toride II Adauchi Gaiden(Kr)  MTR5260-A     I4220  uPD7810  YM2413+M6295
94  Gun Master                    MTR5260-A     I4220  uPD7810  YM2151+M6295
95  Daitoride                     MTR5260-A     I4220  uPD7810  YM2151+M6295
95  Pururun                       MTR5260-A     I4220  uPD7810  YM2151+M6295
95  Puzzli                        MTR5260-A     I4220  uPD7810  YM2151+M6295
96  Sankokushi                    MTR5260-A     I4220  uPD7810  YM2413+M6295

95  Mouse Shooter GoGo            -             I4220   -       YMF278B       GFX ROM data lines swapped
96  Bal Cube                      -             I4220   -       YMF278B       GFX ROM data lines swapped
96  Daitoride (YMF278B)           -             I4220   -       YMF278B       GFX ROM data lines swapped
96  Bang Bang Ball                -             I4220   -       YMF278B       GFX ROM data lines swapped
99  Battle Bubble v2.00           LM2D-Y        I4220   -       YMF278B       GFX ROM data lines swapped

94  Blazing Tornado               HUM-002-A-(B) I4220  Z80      YM2610        Konami 053936 PSAC2
96  Grand Striker 2               HUM-003(A)    I4220  Z80      YM2610        Konami 053936 PSAC2

95  Varia Metal                   ES-9309B-B    I4220   -       ES8712+M6295+M6585

95  Mahjong Doukyuusei            VG330-B       I4300   -       YM2413+M6295
95  Mahjong Doukyuusei Special    VG340-A       I4300   -       YM2413+M6295
96  Mouja                         VG410-B       I4300   -       YM2413+M6295
97  Mahjong Gakuensai             VG340-A       I4300   -       YM2413+M6295
98  Mahjong Gakuensai 2           VG340-A       I4300   -       YM2413+M6295

00  Puzzlet                       VG2200-(B)    I4300  Z86E02   YM2413+M6295  H8/3007 CPU
------------------------------------------------------------------------------------------------

Mouse Shooter GoGo, Bal Cube, Bang Bang Ball & Daitoride (YMF278B) PCBs have
no PCB number but all look identical to each other.

To Do:

-   Tilemaps/sprites offsets may be emulated understanding what appear to be CRT registers
    at c78880 (sequence of 4 values), c78890 (sequence of 5 values) and c788a0 (start sequence).
-   Wrong color bars in service mode (e.g. balcube, toride2g).
    They use solid color tiles (80xx), but the right palette is not at 00-ff.
    Related to the unknown table in the RAM mapped just before the palette?
-   Most games, in service mode, seem to require that you press start1&2 *exactly at once*
    in order to advance to the next screen (e.g. holding 1 then pressing 2 doesn't work).
-   Coin lockout
-   Some gfx problems in ladykill, 3kokushi, puzzli, gakusai,
    seem related to how we handle windows and wrapping
-   Interrupt timing needs figuring out properly, having it incorrect
    causes scrolling glitches in some games.  Test cases Mouse Go Go
    title screen, GunMaster title screen.  Changing it can cause
    excessive slowdown in said games however.
-   Bang Bang Ball / Bubble Buster slow to a crawl when you press a
    button between levels, on a real PCB it speeds up instead (related
    to above?)
-   vmetal: ES8712 sound may not be quite right. Samples are currently looped, but
    whether they should and how, is unknown. Where does the M6585 hook up to?

Notes:

-   To enter service mode in ladykill, 3kokushi: toggle the dip switch and reset
    keeping start 2 pressed.
-   Sprite zoom in Mouja at the end of a match looks wrong, but it's been verified
    to be the same on the original board
-   vmetal: has Sega and Taito logos in the roms ?!


driver modified by Eisuke Watanabe
***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "cpu/upd7810/upd7810.h"
#include "cpu/h8/h83006.h"
#include "includes/metro.h"
#include "machine/eepromser.h"
#include "sound/2151intf.h"
#include "sound/2413intf.h"
#include "sound/2610intf.h"
#include "sound/okim6295.h"
#include "sound/ymf278b.h"



/***************************************************************************


                                Interrupts


***************************************************************************/

READ16_MEMBER(metro_state::metro_irq_cause_r)
{
	/* interrupt cause, used by

	int[0] vblank
	int[1] ?            DAITORIDE, BALCUBE, KARATOUR, MOUJA
	int[2] blitter
	int[3] ?            KARATOUR
	int[4] ?
	int[5] ?            KARATOUR, BLZNTRND
	int[6] unused
	int[7] unused

	*/

	UINT16 res = 0;
	for (int i = 0; i < 8; i++)
		res |= (m_requested_int[i] << i);

	return res;
}


/* Update the IRQ state based on all possible causes */
void metro_state::update_irq_state()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/*  Get the pending IRQs (only the enabled ones, e.g. where irq_enable is *0*)  */
	UINT16 irq = metro_irq_cause_r(space, 0, 0xffff) & ~*m_irq_enable;

	if (m_irq_line == -1)    /* mouja, gakusai, gakusai2, dokyusei, dokyusp */
	{
		/*  This is for games that supply an *IRQ Vector* on the data bus together with an IRQ level for each possible IRQ source */
		UINT8 irq_level[8] = { 0 };
		int i;

		for (i = 0; i < 8; i++)
			if (BIT(irq, i))
				irq_level[m_irq_levels[i] & 7] = 1;

		for (i = 0; i < 8; i++)
			m_maincpu->set_input_line(i, irq_level[i] ? ASSERT_LINE : CLEAR_LINE);
	}
	else
	{
		/*  This is for games where every IRQ source generates the same IRQ level. The interrupt service routine
		    then reads the actual source by peeking a register (metro_irq_cause_r) */

		int irq_state = (irq ? ASSERT_LINE : CLEAR_LINE);
		m_maincpu->set_input_line(m_irq_line, irq_state);
	}
}


/* For games that supply an *IRQ Vector* on the data bus */
IRQ_CALLBACK_MEMBER(metro_state::metro_irq_callback)
{
	// logerror("%s: irq callback returns %04X\n", device.machine().describe_context(), m_irq_vectors[int_level]);
	return m_irq_vectors[irqline] & 0xff;
}


WRITE16_MEMBER(metro_state::metro_irq_cause_w)
{
	//if (data & ~0x15) logerror("CPU #0 PC %06X : unknown bits of irqcause written: %04X\n", space.device().safe_pc(), data);

	if (ACCESSING_BITS_0_7)
	{
		data &= ~*m_irq_enable;

		for (int i = 0; i < 8; i++)
			if (BIT(data, i)) m_requested_int[i] = 0;
	}

	update_irq_state();
}

void metro_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_KARATOUR_IRQ:
		m_requested_int[5] = 0;
		break;
	case TIMER_MOUJA_IRQ:
		m_requested_int[0] = 1;
		update_irq_state();
		break;
	case TIMER_METRO_BLIT_DONE:
		metro_blit_done(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in metro_state::device_timer");
	}
}

INTERRUPT_GEN_MEMBER(metro_state::metro_vblank_interrupt)
{
	m_requested_int[m_vblank_bit] = 1;
	update_irq_state();
}

INTERRUPT_GEN_MEMBER(metro_state::metro_periodic_interrupt)
{
	m_requested_int[4] = 1;
	update_irq_state();
}

/* lev 2-7 (lev 1 seems sound related) */
INTERRUPT_GEN_MEMBER(metro_state::karatour_interrupt)
{
	m_requested_int[m_vblank_bit] = 1;

	/* write to scroll registers, the duration is a guess */
	timer_set(attotime::from_usec(2500), TIMER_KARATOUR_IRQ);
	m_requested_int[5] = 1;

	update_irq_state();
}

WRITE16_MEMBER(metro_state::mouja_irq_timer_ctrl_w)
{
	double freq = 58.0 + (0xff - (data & 0xff)) / 2.2; /* 0xff=58Hz, 0x80=116Hz? */

	m_mouja_irq_timer->adjust(attotime::zero, 0, attotime::from_hz(freq));
}

INTERRUPT_GEN_MEMBER(metro_state::puzzlet_interrupt)
{
	m_requested_int[m_vblank_bit] = 1;
	m_requested_int[5] = 1;

	update_irq_state();
}

/***************************************************************************


                            Sound Communication


***************************************************************************/

READ_LINE_MEMBER(metro_state::metro_rxd_r)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT8 data = soundlatch_byte_r(space, 0);

	soundlatch_byte_w(space, 0, data >> 1);

	return data & 1;

}

WRITE16_MEMBER(metro_state::metro_soundlatch_w)
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_byte_w(space, 0, data & 0xff);
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		space.device().execute().spin_until_interrupt();
		m_busy_sndcpu = 1;
	}
}


READ16_MEMBER(metro_state::metro_soundstatus_r)
{
	return (m_busy_sndcpu ? 0x00 : 0x01);
}

CUSTOM_INPUT_MEMBER(metro_state::custom_soundstatus_r)
{
	return (m_busy_sndcpu ? 0x01 : 0x00);
}

WRITE16_MEMBER(metro_state::metro_soundstatus_w)
{
	if (ACCESSING_BITS_0_7)
		m_soundstatus = data & 0x01;
}


WRITE8_MEMBER(metro_state::metro_sound_rombank_w)
{
	int bankaddress;
	UINT8 *ROM = memregion("audiocpu")->base();

	bankaddress = 0x10000-0x4000 + ((data >> 4) & 0x03) * 0x4000;
	if (bankaddress < 0x10000) bankaddress = 0x0000;

	membank("bank1")->set_base(&ROM[bankaddress]);
}

WRITE8_MEMBER(metro_state::daitorid_sound_rombank_w)
{
	int bankaddress;
	UINT8 *ROM = memregion("audiocpu")->base();

	bankaddress = 0x10000-0x4000 + ((data >> 4) & 0x07) * 0x4000;
	if (bankaddress < 0x10000) bankaddress = 0x10000;

	membank("bank1")->set_base(&ROM[bankaddress]);
}


READ8_MEMBER(metro_state::metro_porta_r)
{
	return m_porta;
}

WRITE8_MEMBER(metro_state::metro_porta_w)
{
	m_porta = data;
}

WRITE8_MEMBER(metro_state::metro_portb_w)
{
	/* port B layout:
	   7 !clock latch for message to main CPU
	   6
	   5 !clock YM2413 I/O
	   4 !clock MSM6295 I/O
	   3
	   2 !enable write to YM2413/6295
	   1 select YM2151 register or data port
	   0
	*/


	if (BIT(m_portb, 7) && !BIT(data, 7))   /* clock 1->0 */
	{
		m_busy_sndcpu = 0;
		m_portb = data;
		return;
	}

	if (BIT(m_portb, 5) && !BIT(data, 5))   /* clock 1->0 */
	{
		if (!BIT(data, 2))
		{
			downcast<ym2413_device *>(m_ymsnd.target())->write(space, BIT(data, 1), m_porta);
		}
		m_portb = data;
		return;
	}

	if (BIT(m_portb, 2) && !BIT(data, 2))   /* clock 1->0 */
	{
		/* write */
		if (!BIT(data, 4))
			m_oki->write(space, 0, m_porta);
	}

	m_portb = data;
}


WRITE8_MEMBER(metro_state::daitorid_portb_w)
{
	/* port B layout:
	   7 !clock latch for message to main CPU
	   6 !clock YM2151 I/O
	   5
	   4 !clock MSM6295 I/O
	   3 !enable read from YM2151/6295
	   2 !enable write to YM2151/6295
	   1 select YM2151 register or data port
	   0
	*/

	if (BIT(m_portb, 7) && !BIT(data, 7))   /* clock 1->0 */
	{
		m_busy_sndcpu = 0;
		m_portb = data;
		return;
	}

	if (BIT(m_portb, 6) && !BIT(data, 6))   /* clock 1->0 */
	{
		if (!BIT(data, 2))
		{
			/* write */
			downcast<ym2151_device *>(m_ymsnd.target())->write(space, BIT(data, 1), m_porta);
		}

		if (!BIT(data, 3))
		{
			/* read */
			m_porta = downcast<ym2151_device *>(m_ymsnd.target())->read(space, BIT(data, 1));
		}

		m_portb = data;
		return;
	}

	if (BIT(m_portb, 2) && !BIT(data, 2))   /* clock 1->0 */
	{
		/* write */
		if (!BIT(data, 4))
			m_oki->write(space, 0, m_porta);
	}

	if (BIT(m_portb, 3) && !BIT(data, 3))   /* clock 1->0 */
	{
		/* read */
		if (!BIT(data, 4))
			m_porta = m_oki->read(space, 0);
	}

	m_portb = data;
}


/***************************************************************************


                                Coin Lockout


***************************************************************************/

/* IT DOESN'T WORK PROPERLY */

WRITE16_MEMBER(metro_state::metro_coin_lockout_1word_w)
{
	if (ACCESSING_BITS_0_7)
	{
//      machine().bookkeeping().coin_lockout_w(0, data & 1);
//      machine().bookkeeping().coin_lockout_w(1, data & 2);
	}
	if (data & ~3)  logerror("CPU #0 PC %06X : unknown bits of coin lockout written: %04X\n", space.device().safe_pc(), data);
}


WRITE16_MEMBER(metro_state::metro_coin_lockout_4words_w)
{
//  machine().bookkeeping().coin_lockout_w((offset >> 1) & 1, offset & 1);
	if (data & ~1)  logerror("CPU #0 PC %06X : unknown bits of coin lockout written: %04X\n", space.device().safe_pc(), data);
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

READ16_MEMBER(metro_state::metro_bankedrom_r)
{
	UINT8 *ROM = memregion("gfx1")->base();
	size_t len = memregion("gfx1")->bytes();

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

TIMER_CALLBACK_MEMBER(metro_state::metro_blit_done)
{
	m_requested_int[m_blitter_bit] = 1;
	update_irq_state();
}

inline int metro_state::blt_read( const UINT8 *ROM, const int offs )
{
	return ROM[offs];
}

void metro_state::blt_write( address_space &space, const int tmap, const offs_t offs, const UINT16 data, const UINT16 mask )
{
	switch(tmap)
	{
		case 1: metro_vram_0_w(space, offs, data, mask);    break;
		case 2: metro_vram_1_w(space, offs, data, mask);    break;
		case 3: metro_vram_2_w(space, offs, data, mask);    break;
	}
//  logerror("%s : Blitter %X] %04X <- %04X & %04X\n", space.machine().describe_context(), tmap, offs, data, mask);
}


WRITE16_MEMBER(metro_state::metro_blitter_w)
{
	COMBINE_DATA(&m_blitter_regs[offset]);

	if (offset == 0x0c / 2)
	{
		UINT8 *src     = memregion("gfx1")->base();
		size_t src_len = memregion("gfx1")->bytes();

		UINT32 tmap     = (m_blitter_regs[0x00 / 2] << 16) + m_blitter_regs[0x02 / 2];
		UINT32 src_offs = (m_blitter_regs[0x04 / 2] << 16) + m_blitter_regs[0x06 / 2];
		UINT32 dst_offs = (m_blitter_regs[0x08 / 2] << 16) + m_blitter_regs[0x0a / 2];

		int shift   = (dst_offs & 0x80) ? 0 : 8;
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
					timer_set(attotime::from_usec(500), TIMER_METRO_BLIT_DONE);
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
				if (b1 == 0xc0)
				{
					dst_offs +=   0x100;
					dst_offs &= ~(0x100 - 1);
					dst_offs |=  (0x100 - 1) & (m_blitter_regs[0x0a / 2] >> (7 + 1));
				}
				else
				{
					dst_offs += count;
				}
				break;

			default:
				logerror("CPU #0 PC %06X : Blitter unknown opcode %02X at %06X\n",space.device().safe_pc(),b1,src_offs-1);
				return;
			}

		}
	}
}


/***************************************************************************


                                Memory Maps


***************************************************************************/

/*
 Lines starting with an empty comment in the following MemoryReadAddress
 arrays are there for debug (e.g. the game does not read from those ranges
 AFAIK)
*/


static ADDRESS_MAP_START( metro_sound_map, AS_PROGRAM, 8, metro_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM         /* External ROM */
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")    /* External ROM (Banked) */
	AM_RANGE(0x8000, 0x87ff) AM_RAM         /* External RAM */
	AM_RANGE(0xff00, 0xffff) AM_RAM         /* Internal RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( metro_sound_io_map, AS_IO, 8, metro_state )
	AM_RANGE(UPD7810_PORTA, UPD7810_PORTA) AM_READWRITE(metro_porta_r, metro_porta_w)
	AM_RANGE(UPD7810_PORTB, UPD7810_PORTB) AM_WRITE(metro_portb_w)
	AM_RANGE(UPD7810_PORTC, UPD7810_PORTC) AM_WRITE(metro_sound_rombank_w)
ADDRESS_MAP_END

/*****************/


static ADDRESS_MAP_START( daitorid_sound_io_map, AS_IO, 8, metro_state )
	AM_RANGE(UPD7810_PORTA, UPD7810_PORTA) AM_READWRITE(metro_porta_r, metro_porta_w)
	AM_RANGE(UPD7810_PORTB, UPD7810_PORTB) AM_WRITE(daitorid_portb_w)
	AM_RANGE(UPD7810_PORTC, UPD7810_PORTC) AM_WRITE(daitorid_sound_rombank_w)
ADDRESS_MAP_END

/*****************/


static ADDRESS_MAP_START( ymf278_map, AS_0, 8, metro_state )
	AM_RANGE(0x000000, 0x27ffff) AM_ROM
ADDRESS_MAP_END


/***************************************************************************
                                    Bal Cube
***************************************************************************/

/* Really weird way of mapping 3 DSWs */
READ16_MEMBER(metro_state::balcube_dsw_r)
{
	UINT16 dsw1 = ioport("DSW0")->read() >> 0;
	UINT16 dsw2 = ioport("DSW0")->read() >> 8;
	UINT16 dsw3 = ioport("IN2")->read();

	switch (offset * 2)
	{
		case 0x1FFFC:   return (BIT(dsw1, 0) ? 0x40 : 0) | (BIT(dsw3, 0) ? 0x80 : 0);
		case 0x1FFFA:   return (BIT(dsw1, 1) ? 0x40 : 0) | (BIT(dsw3, 1) ? 0x80 : 0);
		case 0x1FFF6:   return (BIT(dsw1, 2) ? 0x40 : 0) | (BIT(dsw3, 2) ? 0x80 : 0);
		case 0x1FFEE:   return (BIT(dsw1, 3) ? 0x40 : 0) | (BIT(dsw3, 3) ? 0x80 : 0);
		case 0x1FFDE:   return (BIT(dsw1, 4) ? 0x40 : 0) | (BIT(dsw3, 4) ? 0x80 : 0);
		case 0x1FFBE:   return (BIT(dsw1, 5) ? 0x40 : 0) | (BIT(dsw3, 5) ? 0x80 : 0);
		case 0x1FF7E:   return (BIT(dsw1, 6) ? 0x40 : 0) | (BIT(dsw3, 6) ? 0x80 : 0);
		case 0x1FEFE:   return (BIT(dsw1, 7) ? 0x40 : 0) | (BIT(dsw3, 7) ? 0x80 : 0);

		case 0x1FDFE:   return BIT(dsw2, 0) ? 0x40 : 0;
		case 0x1FBFE:   return BIT(dsw2, 1) ? 0x40 : 0;
		case 0x1F7FE:   return BIT(dsw2, 2) ? 0x40 : 0;
		case 0x1EFFE:   return BIT(dsw2, 3) ? 0x40 : 0;
		case 0x1DFFE:   return BIT(dsw2, 4) ? 0x40 : 0;
		case 0x1BFFE:   return BIT(dsw2, 5) ? 0x40 : 0;
		case 0x17FFE:   return BIT(dsw2, 6) ? 0x40 : 0;
		case 0x0FFFE:   return BIT(dsw2, 7) ? 0x40 : 0;
	}
	logerror("CPU #0 PC %06X : unknown dsw address read: %04X\n", space.device().safe_pc(), offset);
	return 0xffff;
}


static ADDRESS_MAP_START( balcube_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                             // ROM
	AM_RANGE(0x300000, 0x300001) AM_DEVREAD8("ymf", ymf278b_device, read, 0x00ff)   // Sound
	AM_RANGE(0x300000, 0x30000b) AM_DEVWRITE8("ymf", ymf278b_device, write, 0x00ff) // Sound
	AM_RANGE(0x400000, 0x41ffff) AM_READ(balcube_dsw_r)                             // DSW x 3
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("IN0")                                // Inputs
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("IN1")                                //
	AM_RANGE(0x500006, 0x500007) AM_READNOP                                         //
	AM_RANGE(0x500002, 0x500009) AM_WRITE(metro_coin_lockout_4words_w)              // Coin Lockout
	AM_RANGE(0x600000, 0x61ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0x620000, 0x63ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0x640000, 0x65ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0x660000, 0x66ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x670000, 0x671fff) AM_RAM                                             // ???
	AM_RANGE(0x672000, 0x673fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x674000, 0x674fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0x678000, 0x6787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x678840, 0x67884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0x678860, 0x67886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x678870, 0x67887b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll
	AM_RANGE(0x678880, 0x678881) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x678890, 0x678891) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x6788a2, 0x6788a3) AM_READWRITE(metro_irq_cause_r, metro_irq_cause_w) // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0x6788a4, 0x6788a5) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0x6788aa, 0x6788ab) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x6788ac, 0x6788ad) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
	AM_RANGE(0x679700, 0x679713) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
ADDRESS_MAP_END


/***************************************************************************
                            Daitoride (YMF278B version)
***************************************************************************/


static ADDRESS_MAP_START( daitoa_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                             // ROM
	AM_RANGE(0x100000, 0x11ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0x120000, 0x13ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0x140000, 0x15ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0x160000, 0x16ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x170000, 0x171fff) AM_RAM                                             // ???
	AM_RANGE(0x172000, 0x173fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x174000, 0x174fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0x178000, 0x1787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x178840, 0x17884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0x178860, 0x17886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x178870, 0x17887b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll
	AM_RANGE(0x178880, 0x178881) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x178890, 0x178891) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x1788a2, 0x1788a3) AM_READWRITE(metro_irq_cause_r, metro_irq_cause_w) // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0x1788a4, 0x1788a5) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0x1788aa, 0x1788ab) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x1788ac, 0x1788ad) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
	AM_RANGE(0x179700, 0x179713) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0x200000, 0x200001) AM_READ_PORT("IN0")                                // Inputs
	AM_RANGE(0x200002, 0x200003) AM_READ_PORT("IN1")                                //
	AM_RANGE(0x200006, 0x200007) AM_READNOP                                         //
	AM_RANGE(0x200002, 0x200009) AM_WRITE(metro_coin_lockout_4words_w)              // Coin Lockout
	AM_RANGE(0x300000, 0x31ffff) AM_READ(balcube_dsw_r)                             // DSW x 3
	AM_RANGE(0x400000, 0x400001) AM_DEVREAD8("ymf", ymf278b_device, read, 0x00ff)   // Sound
	AM_RANGE(0x400000, 0x40000b) AM_DEVWRITE8("ymf", ymf278b_device, write, 0x00ff) // Sound
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
ADDRESS_MAP_END


/***************************************************************************
                                Bang Bang Ball
***************************************************************************/

static ADDRESS_MAP_START( bangball_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                             // ROM
	AM_RANGE(0xb00000, 0xb00001) AM_DEVREAD8("ymf", ymf278b_device, read, 0x00ff)   // Sound
	AM_RANGE(0xb00000, 0xb0000b) AM_DEVWRITE8("ymf", ymf278b_device, write, 0x00ff) // Sound
	AM_RANGE(0xc00000, 0xc1ffff) AM_READ(balcube_dsw_r)                             // DSW x 3
	AM_RANGE(0xd00000, 0xd00001) AM_READ_PORT("IN0")                                // Inputs
	AM_RANGE(0xd00002, 0xd00003) AM_READ_PORT("IN1")                                //
	AM_RANGE(0xd00006, 0xd00007) AM_READNOP                                         //
	AM_RANGE(0xd00002, 0xd00009) AM_WRITE(metro_coin_lockout_4words_w)              // Coin Lockout
	AM_RANGE(0xe00000, 0xe1ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0xe20000, 0xe3ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0xe40000, 0xe5ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0xe60000, 0xe6ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0xe70000, 0xe71fff) AM_RAM                                             // ???
	AM_RANGE(0xe72000, 0xe73fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0xe74000, 0xe74fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0xe78000, 0xe787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0xe78840, 0xe7884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0xe78860, 0xe7886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0xe78870, 0xe7887b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll
	AM_RANGE(0xe78880, 0xe78881) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0xe78890, 0xe78891) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0xe788a2, 0xe788a3) AM_READWRITE(metro_irq_cause_r, metro_irq_cause_w) // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0xe788a4, 0xe788a5) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0xe788aa, 0xe788ab) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0xe788ac, 0xe788ad) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
	AM_RANGE(0xe79700, 0xe79713) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
ADDRESS_MAP_END


/***************************************************************************
                                Battle Bubble
***************************************************************************/

static ADDRESS_MAP_START( batlbubl_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM                                             // ROM
	AM_RANGE(0x100000, 0x11ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0x120000, 0x13ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0x140000, 0x15ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0x160000, 0x16ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x170000, 0x171fff) AM_RAM                                             // ???
	AM_RANGE(0x172000, 0x173fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x174000, 0x174fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0x178000, 0x1787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x178840, 0x17884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0x178860, 0x17886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x178870, 0x17887b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll
	AM_RANGE(0x178880, 0x178881) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x178890, 0x178891) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x1788a2, 0x1788a3) AM_READWRITE(metro_irq_cause_r,metro_irq_cause_w)  // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0x1788a4, 0x1788a5) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0x1788aa, 0x1788ab) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x1788ac, 0x1788ad) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
	AM_RANGE(0x179700, 0x179713) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0x200000, 0x200001) AM_READ_PORT("IN1")                                // Inputs
	AM_RANGE(0x200002, 0x200003) AM_READ_PORT("DSW0")                               //
	AM_RANGE(0x200004, 0x200005) AM_READ_PORT("IN0")                                //
	AM_RANGE(0x200006, 0x200007) AM_READ_PORT("IN2")                                //
	AM_RANGE(0x200002, 0x200009) AM_WRITE(metro_coin_lockout_4words_w)              // Coin Lockout
	AM_RANGE(0x300000, 0x31ffff) AM_READ(balcube_dsw_r)                             // read but ignored?
	AM_RANGE(0x400000, 0x400001) AM_DEVREAD8("ymf", ymf278b_device, read, 0x00ff)   // Sound
	AM_RANGE(0x400000, 0x40000b) AM_DEVWRITE8("ymf", ymf278b_device, write, 0x00ff) //
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
ADDRESS_MAP_END


/***************************************************************************
                             Mouse Shooter GoGo
***************************************************************************/

static ADDRESS_MAP_START( msgogo_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                             // ROM
	AM_RANGE(0x100000, 0x11ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0x120000, 0x13ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0x140000, 0x15ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0x160000, 0x16ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x170000, 0x171fff) AM_RAM                                             // ???
	AM_RANGE(0x172000, 0x173fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x174000, 0x174fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0x178000, 0x1787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x178840, 0x17884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0x178860, 0x17886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x178870, 0x17887b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll
	AM_RANGE(0x178880, 0x178881) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x178890, 0x178891) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x1788a2, 0x1788a3) AM_READWRITE(metro_irq_cause_r, metro_irq_cause_w) // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0x1788a4, 0x1788a5) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0x1788aa, 0x1788ab) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x1788ac, 0x1788ad) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
	AM_RANGE(0x179700, 0x179713) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0x200000, 0x200001) AM_READ_PORT("COINS")                              // Inputs
	AM_RANGE(0x200002, 0x200003) AM_READ_PORT("JOYS")                               //
	AM_RANGE(0x200006, 0x200007) AM_READNOP                                         //
	AM_RANGE(0x200002, 0x200009) AM_WRITE(metro_coin_lockout_4words_w)              // Coin Lockout
	AM_RANGE(0x300000, 0x31ffff) AM_READ(balcube_dsw_r)                             // 3 x DSW
	AM_RANGE(0x400000, 0x400001) AM_DEVREAD8("ymf", ymf278b_device, read, 0x00ff)   // Sound
	AM_RANGE(0x400000, 0x40000b) AM_DEVWRITE8("ymf", ymf278b_device, write, 0x00ff) //
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
ADDRESS_MAP_END

/***************************************************************************
                                Daitoride
***************************************************************************/

static ADDRESS_MAP_START( daitorid_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                             // ROM
	AM_RANGE(0x400000, 0x41ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0x420000, 0x43ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0x440000, 0x45ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0x460000, 0x46ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x470000, 0x471fff) AM_RAM                                             // ???
	AM_RANGE(0x472000, 0x473fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x474000, 0x474fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0x478000, 0x4787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x478840, 0x47884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0x478860, 0x47886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x478870, 0x47887b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll
	AM_RANGE(0x478880, 0x478881) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x478890, 0x478891) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x4788a2, 0x4788a3) AM_READWRITE(metro_irq_cause_r, metro_irq_cause_w) // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0x4788a4, 0x4788a5) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0x4788a8, 0x4788a9) AM_WRITE(metro_soundlatch_w)                       // To Sound CPU
	AM_RANGE(0x4788aa, 0x4788ab) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x4788ac, 0x4788ad) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
	AM_RANGE(0x479700, 0x479713) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0x800000, 0x80ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
	AM_RANGE(0xc00000, 0xc00001) AM_READ_PORT("IN0") AM_WRITE(metro_soundstatus_w)  // To Sound CPU
	AM_RANGE(0xc00002, 0xc00003) AM_READ_PORT("IN1")                                //
	AM_RANGE(0xc00004, 0xc00005) AM_READ_PORT("DSW0")                               //
	AM_RANGE(0xc00006, 0xc00007) AM_READ_PORT("IN2")                                //
	AM_RANGE(0xc00002, 0xc00009) AM_WRITE(metro_coin_lockout_4words_w)              // Coin Lockout
ADDRESS_MAP_END


/***************************************************************************
                                Dharma Doujou
***************************************************************************/

static ADDRESS_MAP_START( dharma_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                             // ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
	AM_RANGE(0x800000, 0x81ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0x820000, 0x83ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0x840000, 0x85ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0x860000, 0x86ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x870000, 0x871fff) AM_RAM                                             // ???
	AM_RANGE(0x872000, 0x873fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x874000, 0x874fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0x878000, 0x8787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x878840, 0x87884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0x878860, 0x87886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x878870, 0x87887b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll Regs
	AM_RANGE(0x878880, 0x878881) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x878890, 0x878891) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x8788a2, 0x8788a3) AM_READWRITE(metro_irq_cause_r, metro_irq_cause_w) // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0x8788a4, 0x8788a5) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0x8788a8, 0x8788a9) AM_WRITE(metro_soundlatch_w)                       // To Sound CPU
	AM_RANGE(0x8788aa, 0x8788ab) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x8788ac, 0x8788ad) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
	AM_RANGE(0x879700, 0x879713) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0xc00000, 0xc00001) AM_READ_PORT("IN0") AM_WRITE(metro_soundstatus_w)  // To Sound CPU
	AM_RANGE(0xc00002, 0xc00003) AM_READ_PORT("IN1")                                //
	AM_RANGE(0xc00004, 0xc00005) AM_READ_PORT("DSW0")                               //
	AM_RANGE(0xc00006, 0xc00007) AM_READ_PORT("IN2")                                //
	AM_RANGE(0xc00002, 0xc00009) AM_WRITE(metro_coin_lockout_4words_w)              // Coin Lockout
ADDRESS_MAP_END


/***************************************************************************
                                Karate Tournament
***************************************************************************/

/* This game uses almost only the blitter to write to the tilemaps.
   The CPU can only access a "window" of 512x256 pixels in the upper
   left corner of the big tilemap */

#define KARATOUR_OFFS( _x_ ) ((_x_) & (0x3f)) + (((_x_) & ~(0x3f)) * (0x100 / 0x40))

READ16_MEMBER(metro_state::karatour_vram_0_r){ return m_vram_0[KARATOUR_OFFS(offset)]; }
READ16_MEMBER(metro_state::karatour_vram_1_r){ return m_vram_1[KARATOUR_OFFS(offset)]; }
READ16_MEMBER(metro_state::karatour_vram_2_r){ return m_vram_2[KARATOUR_OFFS(offset)]; }

WRITE16_MEMBER(metro_state::karatour_vram_0_w){ metro_vram_0_w(space, KARATOUR_OFFS(offset), data, mem_mask); }
WRITE16_MEMBER(metro_state::karatour_vram_1_w){ metro_vram_1_w(space, KARATOUR_OFFS(offset), data, mem_mask); }
WRITE16_MEMBER(metro_state::karatour_vram_2_w){ metro_vram_2_w(space, KARATOUR_OFFS(offset), data, mem_mask); }

static ADDRESS_MAP_START( karatour_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                             // ROM
	AM_RANGE(0x400000, 0x400001) AM_READWRITE(metro_soundstatus_r, metro_soundstatus_w) // From Sound CPU
	AM_RANGE(0x400002, 0x400003) AM_READ_PORT("IN0")                                // Inputs
	AM_RANGE(0x400002, 0x400003) AM_WRITE(metro_coin_lockout_1word_w)               // Coin Lockout
	AM_RANGE(0x400004, 0x400005) AM_READ_PORT("IN1")                                //
	AM_RANGE(0x400006, 0x400007) AM_READ_PORT("DSW0")                               //
	AM_RANGE(0x40000a, 0x40000b) AM_READ_PORT("DSW1")                               //
	AM_RANGE(0x40000c, 0x40000d) AM_READ_PORT("IN2")                                //
	AM_RANGE(0x860000, 0x86ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x870000, 0x871fff) AM_RAM                                             // ???
	AM_RANGE(0x872000, 0x873fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x874000, 0x874fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0x875000, 0x875fff) AM_READWRITE(karatour_vram_0_r, karatour_vram_0_w) // Layer 0 (Part of)
	AM_RANGE(0x876000, 0x876fff) AM_READWRITE(karatour_vram_1_r, karatour_vram_1_w) // Layer 1 (Part of)
	AM_RANGE(0x877000, 0x877fff) AM_READWRITE(karatour_vram_2_r, karatour_vram_2_w) // Layer 2 (Part of)
	AM_RANGE(0x878000, 0x8787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x878800, 0x878813) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0x878840, 0x87884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0x878860, 0x87886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x878870, 0x87887b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll
	AM_RANGE(0x878880, 0x878881) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x878890, 0x878891) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x8788a2, 0x8788a3) AM_READWRITE(metro_irq_cause_r, metro_irq_cause_w) // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0x8788a4, 0x8788a5) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0x8788a8, 0x8788a9) AM_WRITE(metro_soundlatch_w)                       // To Sound CPU
	AM_RANGE(0x8788aa, 0x8788ab) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x8788ac, 0x8788ad) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
ADDRESS_MAP_END


/***************************************************************************
                                Sankokushi
***************************************************************************/

/* same limited tilemap access as karatour */

static ADDRESS_MAP_START( kokushi_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                             // ROM
	AM_RANGE(0x700000, 0x70ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
	AM_RANGE(0x860000, 0x86ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x870000, 0x871fff) AM_RAM                                             // ???
	AM_RANGE(0x872000, 0x873fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x874000, 0x874fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0x875000, 0x875fff) AM_READWRITE(karatour_vram_0_r, karatour_vram_0_w) // Layer 0 (Part of)
	AM_RANGE(0x876000, 0x876fff) AM_READWRITE(karatour_vram_1_r, karatour_vram_1_w) // Layer 1 (Part of)
	AM_RANGE(0x877000, 0x877fff) AM_READWRITE(karatour_vram_2_r, karatour_vram_2_w) // Layer 2 (Part of)
	AM_RANGE(0x878000, 0x8787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x878840, 0x87884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0x878860, 0x87886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x878870, 0x87887b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll Regs - WRONG
//  AM_RANGE(0x878880, 0x878881) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x878890, 0x878891) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x8788a2, 0x8788a3) AM_READWRITE(metro_irq_cause_r, metro_irq_cause_w) // IRQ Cause /  IRQ Acknowledge
	AM_RANGE(0x8788a4, 0x8788a5) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0x8788a8, 0x8788a9) AM_WRITE(metro_soundlatch_w)                       // To Sound CPU
	AM_RANGE(0x8788aa, 0x8788ab) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x8788ac, 0x8788ad) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
	AM_RANGE(0x879700, 0x879713) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0xc00000, 0xc00001) AM_READ_PORT("IN0") AM_WRITE(metro_soundstatus_w)  // To Sound CPU
	AM_RANGE(0xc00002, 0xc00003) AM_READ_PORT("IN1")                                // Inputs
	AM_RANGE(0xc00004, 0xc00005) AM_READ_PORT("DSW0")                               //
	AM_RANGE(0xc00002, 0xc00009) AM_WRITE(metro_coin_lockout_4words_w   )           // Coin Lockout
ADDRESS_MAP_END


/***************************************************************************
                                Last Fortress
***************************************************************************/

static ADDRESS_MAP_START( lastfort_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                             // ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
	AM_RANGE(0x800000, 0x81ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0x820000, 0x83ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0x840000, 0x85ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0x860000, 0x86ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x870000, 0x871fff) AM_RAM                                             // ???
	AM_RANGE(0x872000, 0x873fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x874000, 0x874fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0x878000, 0x8787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x878800, 0x878813) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0x878840, 0x87884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0x878860, 0x87886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x878870, 0x87887b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll
	AM_RANGE(0x878880, 0x878881) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x878890, 0x878891) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x8788a2, 0x8788a3) AM_READWRITE(metro_irq_cause_r, metro_irq_cause_w) // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0x8788a4, 0x8788a5) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0x8788a8, 0x8788a9) AM_WRITE(metro_soundlatch_w)                       // To Sound CPU
	AM_RANGE(0x8788aa, 0x8788ab) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x8788ac, 0x8788ad) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
	AM_RANGE(0xc00000, 0xc00001) AM_READWRITE(metro_soundstatus_r, metro_soundstatus_w) // From / To Sound CPU
	AM_RANGE(0xc00002, 0xc00003) AM_WRITE(metro_coin_lockout_1word_w)               // Coin Lockout
	AM_RANGE(0xc00004, 0xc00005) AM_READ_PORT("IN0")                                // Inputs
	AM_RANGE(0xc00006, 0xc00007) AM_READ_PORT("IN1")                                //
	AM_RANGE(0xc00008, 0xc00009) AM_READ_PORT("IN2")                                //
	AM_RANGE(0xc0000a, 0xc0000b) AM_READ_PORT("DSW0")                               //
	AM_RANGE(0xc0000c, 0xc0000d) AM_READ_PORT("DSW1")                               //
	AM_RANGE(0xc0000e, 0xc0000f) AM_READ_PORT("IN3")                                //
ADDRESS_MAP_END

/* the German version is halfway between lastfort and ladykill (karatour) memory maps */

/* todo: clean up input reads etc. */
static ADDRESS_MAP_START( lastforg_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                             // ROM
	AM_RANGE(0x400000, 0x400001) AM_READWRITE(metro_soundstatus_r, metro_soundstatus_w) // From / To Sound CPU
	AM_RANGE(0x400002, 0x400003) AM_READ_PORT("IN0")                                // Inputs
	AM_RANGE(0x400002, 0x400003) AM_WRITE(metro_coin_lockout_1word_w)               // Coin Lockout
	AM_RANGE(0x400004, 0x400005) AM_READ_PORT("IN1")                                //
	AM_RANGE(0x400006, 0x400007) AM_READ_PORT("DSW0")                               //
	AM_RANGE(0x40000a, 0x40000b) AM_READ_PORT("DSW1")                               //
	AM_RANGE(0x40000c, 0x40000d) AM_READ_PORT("IN2")                                //
	AM_RANGE(0x880000, 0x89ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0x8a0000, 0x8bffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0x8c0000, 0x8dffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0x8e0000, 0x8effff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x8f0000, 0x8f1fff) AM_RAM                                             // ???
	AM_RANGE(0x8f2000, 0x8f3fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x8f4000, 0x8f4fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0x8f8000, 0x8f87ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x8f8800, 0x8f8813) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0x8f8840, 0x8f884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0x8f8860, 0x8f886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x8f8870, 0x8f887b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll
	AM_RANGE(0x8f8880, 0x8f8881) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x8f8890, 0x8f8891) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x8f88a2, 0x8f88a3) AM_READWRITE(metro_irq_cause_r, metro_irq_cause_w) // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0x8f88a4, 0x8f88a5) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0x8f88a8, 0x8f88a9) AM_WRITE(metro_soundlatch_w)                       // To Sound CPU
	AM_RANGE(0x8f88aa, 0x8f88ab) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x8f88ac, 0x8f88ad) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
	AM_RANGE(0xc00000, 0xc0ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
ADDRESS_MAP_END


/***************************************************************************
                                Mahjong Gakuensai
***************************************************************************/

void metro_state::gakusai_oki_bank_set()
{
	int bank = (m_gakusai_oki_bank_lo & 7) + (m_gakusai_oki_bank_hi & 1) * 8;
	m_oki->set_bank_base(bank * 0x40000);
}

WRITE16_MEMBER(metro_state::gakusai_oki_bank_hi_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_gakusai_oki_bank_hi = data & 0xff;
		gakusai_oki_bank_set();
	}
}

WRITE16_MEMBER(metro_state::gakusai_oki_bank_lo_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_gakusai_oki_bank_lo = data & 0xff;
		gakusai_oki_bank_set();
	}
}


READ16_MEMBER(metro_state::gakusai_input_r)
{
	UINT16 input_sel = (*m_input_sel) ^ 0x3e;
	// Bit 0 ??
	if (input_sel & 0x0002) return ioport("KEY0")->read();
	if (input_sel & 0x0004) return ioport("KEY1")->read();
	if (input_sel & 0x0008) return ioport("KEY2")->read();
	if (input_sel & 0x0010) return ioport("KEY3")->read();
	if (input_sel & 0x0020) return ioport("KEY4")->read();
	return 0xffff;
}

READ16_MEMBER(metro_state::gakusai_eeprom_r)
{
	return m_eeprom->do_read() & 1;
}

WRITE16_MEMBER(metro_state::gakusai_eeprom_w)
{
	if (ACCESSING_BITS_0_7)
	{
		// latch the bit
		m_eeprom->di_write(BIT(data, 0));

		// reset line asserted: reset.
		m_eeprom->cs_write(BIT(data, 2) ? ASSERT_LINE : CLEAR_LINE );

		// clock line asserted: write latch or select next bit to read
		m_eeprom->clk_write(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE );
	}
}

static ADDRESS_MAP_START( gakusai_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                             // ROM
	AM_RANGE(0x200000, 0x21ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0x220000, 0x23ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0x240000, 0x25ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0x260000, 0x26ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x270000, 0x271fff) AM_RAM                                             // ???
	AM_RANGE(0x272000, 0x273fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x274000, 0x274fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0x278000, 0x2787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x27880e, 0x27880f) AM_RAM AM_SHARE("screenctrl")                      // Screen Control
	AM_RANGE(0x278810, 0x27881f) AM_WRITEONLY AM_SHARE("irq_levels")                // IRQ Levels
	AM_RANGE(0x278820, 0x27882f) AM_WRITEONLY AM_SHARE("irq_vectors")               // IRQ Vectors
	AM_RANGE(0x278830, 0x278831) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0x278832, 0x278833) AM_READWRITE(metro_irq_cause_r, metro_irq_cause_w) // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0x278836, 0x278837) AM_WRITE(watchdog_reset16_w)                       // Watchdog
	AM_RANGE(0x278840, 0x27884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0x278850, 0x27885b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll Regs
	AM_RANGE(0x278860, 0x27886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x278870, 0x278871) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x278880, 0x278881) AM_READ(gakusai_input_r)                           // Inputs
	AM_RANGE(0x278882, 0x278883) AM_READ_PORT("IN0")                                //
	AM_RANGE(0x278888, 0x278889) AM_WRITEONLY AM_SHARE("input_sel")                 // Inputs
	AM_RANGE(0x279700, 0x279713) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0x400000, 0x400001) AM_WRITENOP                                        // ? 5
	AM_RANGE(0x500000, 0x500001) AM_WRITE(gakusai_oki_bank_lo_w)                    // Sound
	AM_RANGE(0x600000, 0x600003) AM_DEVWRITE8("ymsnd", ym2413_device, write, 0x00ff)
	AM_RANGE(0x700000, 0x700001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)  // Sound
	AM_RANGE(0xc00000, 0xc00001) AM_READWRITE(gakusai_eeprom_r, gakusai_eeprom_w)   // EEPROM
	AM_RANGE(0xd00000, 0xd00001) AM_WRITE(gakusai_oki_bank_hi_w)
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
ADDRESS_MAP_END


/***************************************************************************
                                Mahjong Gakuensai 2
***************************************************************************/

static ADDRESS_MAP_START( gakusai2_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                             // ROM
	AM_RANGE(0x600000, 0x61ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0x620000, 0x63ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0x640000, 0x65ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0x660000, 0x66ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x670000, 0x671fff) AM_RAM                                             // ???
	AM_RANGE(0x672000, 0x673fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x674000, 0x674fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0x675000, 0x675fff) AM_RAM                                             // Sprites?
	AM_RANGE(0x678000, 0x6787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x67880e, 0x67880f) AM_RAM AM_SHARE("screenctrl")                      // Screen Control
	AM_RANGE(0x678810, 0x67881f) AM_WRITEONLY AM_SHARE("irq_levels")                // IRQ Levels
	AM_RANGE(0x678820, 0x67882f) AM_WRITEONLY AM_SHARE("irq_vectors")               // IRQ Vectors
	AM_RANGE(0x678830, 0x678831) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0x678832, 0x678833) AM_READWRITE(metro_irq_cause_r,metro_irq_cause_w)  // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0x678836, 0x678837) AM_WRITE(watchdog_reset16_w)                       // Watchdog
	AM_RANGE(0x678840, 0x67884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0x678850, 0x67885b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll Regs
	AM_RANGE(0x678860, 0x67886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x678870, 0x678871) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x678880, 0x678881) AM_READ(gakusai_input_r)                           // Inputs
	AM_RANGE(0x678882, 0x678883) AM_READ_PORT("IN0")                                //
	AM_RANGE(0x678888, 0x678889) AM_WRITEONLY AM_SHARE("input_sel")                 // Inputs
	AM_RANGE(0x679700, 0x679713) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0x800000, 0x800001) AM_WRITENOP                                        // ? 5
	AM_RANGE(0x900000, 0x900001) AM_WRITE(gakusai_oki_bank_lo_w)                    // Sound bank
	AM_RANGE(0xa00000, 0xa00001) AM_WRITE(gakusai_oki_bank_hi_w)                    //
	AM_RANGE(0xb00000, 0xb00001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)  // Sound
	AM_RANGE(0xc00000, 0xc00003) AM_DEVWRITE8("ymsnd", ym2413_device, write, 0x00ff)
	AM_RANGE(0xe00000, 0xe00001) AM_READWRITE(gakusai_eeprom_r,gakusai_eeprom_w)    // EEPROM
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
ADDRESS_MAP_END


/***************************************************************************
                        Mahjong Doukyuusei Special
***************************************************************************/

READ16_MEMBER(metro_state::dokyusp_eeprom_r)
{
	// clock line asserted: write latch or select next bit to read
	m_eeprom->clk_write(CLEAR_LINE);
	m_eeprom->clk_write(ASSERT_LINE);

	return m_eeprom->do_read() & 1;
}

WRITE16_MEMBER(metro_state::dokyusp_eeprom_bit_w)
{
	if (ACCESSING_BITS_0_7)
	{
		// latch the bit
		m_eeprom->di_write(BIT(data, 0));

		// clock line asserted: write latch or select next bit to read
		m_eeprom->clk_write(CLEAR_LINE);
		m_eeprom->clk_write(ASSERT_LINE);
	}
}

WRITE16_MEMBER(metro_state::dokyusp_eeprom_reset_w)
{
	if (ACCESSING_BITS_0_7)
	{
		// reset line asserted: reset.
		m_eeprom->cs_write(BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);
	}
}

static ADDRESS_MAP_START( dokyusp_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                             // ROM
	AM_RANGE(0x200000, 0x21ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0x220000, 0x23ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0x240000, 0x25ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0x260000, 0x26ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x270000, 0x271fff) AM_RAM                                             // ???
	AM_RANGE(0x272000, 0x273fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x274000, 0x274fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0x278000, 0x2787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x27880e, 0x27880f) AM_RAM AM_SHARE("screenctrl")                      // Screen Control
	AM_RANGE(0x278810, 0x27881f) AM_WRITEONLY AM_SHARE("irq_levels")                // IRQ Levels
	AM_RANGE(0x278820, 0x27882f) AM_WRITEONLY AM_SHARE("irq_vectors")               // IRQ Vectors
	AM_RANGE(0x278830, 0x278831) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0x278832, 0x278833) AM_READWRITE(metro_irq_cause_r,metro_irq_cause_w)  // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0x278836, 0x278837) AM_WRITE(watchdog_reset16_w)                       // Watchdog
	AM_RANGE(0x278840, 0x27884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0x278850, 0x27885b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll Regs
	AM_RANGE(0x278860, 0x27886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x278870, 0x278871) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x278880, 0x278881) AM_READ(gakusai_input_r)                           // Inputs
	AM_RANGE(0x278882, 0x278883) AM_READ_PORT("IN0")                                //
	AM_RANGE(0x278888, 0x278889) AM_WRITEONLY AM_SHARE("input_sel")                 //
	AM_RANGE(0x279700, 0x279713) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0x400000, 0x400001) AM_WRITENOP                                        // ? 5
	AM_RANGE(0x500000, 0x500001) AM_WRITE(gakusai_oki_bank_lo_w)                    // Sound
	AM_RANGE(0x600000, 0x600003) AM_DEVWRITE8("ymsnd", ym2413_device, write, 0x00ff)
	AM_RANGE(0x700000, 0x700001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)  // Sound
	AM_RANGE(0xc00000, 0xc00001) AM_WRITE(dokyusp_eeprom_reset_w)                       // EEPROM
	AM_RANGE(0xd00000, 0xd00001) AM_READWRITE(dokyusp_eeprom_r, dokyusp_eeprom_bit_w)   // EEPROM
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
ADDRESS_MAP_END


/***************************************************************************
                            Mahjong Doukyuusei
***************************************************************************/

static ADDRESS_MAP_START( dokyusei_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                             // ROM
	AM_RANGE(0x400000, 0x41ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0x420000, 0x43ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0x440000, 0x45ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0x460000, 0x46ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x460000, 0x46ffff) AM_WRITENOP                                        // DSW Selection
	AM_RANGE(0x470000, 0x471fff) AM_RAM                                             // ???
	AM_RANGE(0x472000, 0x473fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x474000, 0x474fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0x478000, 0x4787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x47880e, 0x47880f) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
	AM_RANGE(0x478810, 0x47881f) AM_WRITEONLY AM_SHARE("irq_levels")                // IRQ Levels
	AM_RANGE(0x478820, 0x47882f) AM_WRITEONLY AM_SHARE("irq_vectors")               // IRQ Vectors
	AM_RANGE(0x478830, 0x478831) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
//  AM_RANGE(0x478832, 0x478833) AM_READ(metro_irq_cause_r)                         // IRQ Cause
	AM_RANGE(0x478832, 0x478833) AM_WRITE(metro_irq_cause_w)                        // IRQ Acknowledge
	AM_RANGE(0x478836, 0x478837) AM_WRITENOP                                        // ? watchdog ?
	AM_RANGE(0x478840, 0x47884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0x478850, 0x47885b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll Regs
	AM_RANGE(0x478860, 0x47886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x478870, 0x478871) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x478880, 0x478881) AM_READ(gakusai_input_r)                           // Inputs
	AM_RANGE(0x478882, 0x478883) AM_READ_PORT("IN0")                                //
	AM_RANGE(0x478884, 0x478885) AM_READ_PORT("DSW0")                               // 2 x DSW
	AM_RANGE(0x478886, 0x478887) AM_READ_PORT("DSW1")                               //
	AM_RANGE(0x478888, 0x478889) AM_WRITEONLY AM_SHARE("input_sel")                 // Inputs
	AM_RANGE(0x479700, 0x479713) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0x800000, 0x800001) AM_WRITE(gakusai_oki_bank_hi_w)                    // Samples Bank?
	AM_RANGE(0x900000, 0x900001) AM_WRITENOP                                        // ? 4
	AM_RANGE(0xa00000, 0xa00001) AM_WRITE(gakusai_oki_bank_lo_w)                    // Samples Bank
	AM_RANGE(0xc00000, 0xc00003) AM_DEVWRITE8("ymsnd", ym2413_device, write, 0x00ff)     //
	AM_RANGE(0xd00000, 0xd00001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)  // Sound
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
ADDRESS_MAP_END


/***************************************************************************
                                Pang Pom's
***************************************************************************/

static ADDRESS_MAP_START( pangpoms_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                             // ROM
	AM_RANGE(0x400000, 0x41ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0x420000, 0x43ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0x440000, 0x45ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0x460000, 0x46ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x470000, 0x471fff) AM_RAM                                             // ???
	AM_RANGE(0x472000, 0x473fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x474000, 0x474fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0x478000, 0x4787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x478800, 0x478813) AM_RAM AM_SHARE("videoregs")                       // Video Registers
	AM_RANGE(0x478840, 0x47884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0x478860, 0x47886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x478870, 0x47887b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll Regs
	AM_RANGE(0x478880, 0x478881) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x478890, 0x478891) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x4788a2, 0x4788a3) AM_READWRITE(metro_irq_cause_r,metro_irq_cause_w)  // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0x4788a4, 0x4788a5) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0x4788a8, 0x4788a9) AM_WRITE(metro_soundlatch_w)                       // To Sound CPU
	AM_RANGE(0x4788aa, 0x4788ab) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x4788ac, 0x4788ad) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
	AM_RANGE(0x800000, 0x800001) AM_READWRITE(metro_soundstatus_r,metro_soundstatus_w)  // From / To Sound CPU
	AM_RANGE(0x800002, 0x800003) AM_READNOP AM_WRITE(metro_coin_lockout_1word_w)    // Coin Lockout
	AM_RANGE(0x800004, 0x800005) AM_READ_PORT("IN0")                                // Inputs
	AM_RANGE(0x800006, 0x800007) AM_READ_PORT("IN1")                                //
	AM_RANGE(0x800008, 0x800009) AM_READ_PORT("IN2")                                //
	AM_RANGE(0x80000a, 0x80000b) AM_READ_PORT("DSW0")                               //
	AM_RANGE(0x80000c, 0x80000d) AM_READ_PORT("DSW1")                               //
	AM_RANGE(0x80000e, 0x80000f) AM_READ_PORT("IN3")                                //
	AM_RANGE(0xc00000, 0xc0ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
ADDRESS_MAP_END


/***************************************************************************
                                Poitto!
***************************************************************************/

static ADDRESS_MAP_START( poitto_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                             // ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("IN0") AM_WRITE(metro_soundstatus_w)  // To Sound CPU
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("IN1")                                //
	AM_RANGE(0x800004, 0x800005) AM_READ_PORT("DSW0")                               //
	AM_RANGE(0x800006, 0x800007) AM_READ_PORT("IN2")                                //
	AM_RANGE(0x800002, 0x800009) AM_WRITE(metro_coin_lockout_4words_w)              // Coin Lockout
	AM_RANGE(0xc00000, 0xc1ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0xc20000, 0xc3ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0xc40000, 0xc5ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0xc60000, 0xc6ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0xc70000, 0xc71fff) AM_RAM                                             // ???
	AM_RANGE(0xc72000, 0xc73fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0xc74000, 0xc74fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0xc78000, 0xc787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0xc78800, 0xc78813) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0xc78840, 0xc7884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0xc78860, 0xc7886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0xc78870, 0xc7887b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll Regs
	AM_RANGE(0xc78880, 0xc78881) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0xc78890, 0xc78891) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0xc788a2, 0xc788a3) AM_READWRITE(metro_irq_cause_r,metro_irq_cause_w)  // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0xc788a4, 0xc788a5) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0xc788a8, 0xc788a9) AM_WRITE(metro_soundlatch_w)                       // To Sound CPU
	AM_RANGE(0xc788aa, 0xc788ab) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0xc788ac, 0xc788ad) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
ADDRESS_MAP_END


/***************************************************************************
                                Sky Alert
***************************************************************************/

static ADDRESS_MAP_START( skyalert_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                             // ROM
	AM_RANGE(0x400000, 0x400001) AM_READWRITE(metro_soundstatus_r,metro_soundstatus_w)  // From / To Sound CPU
	AM_RANGE(0x400002, 0x400003) AM_READNOP AM_WRITE(metro_coin_lockout_1word_w)    // Coin Lockout
	AM_RANGE(0x400004, 0x400005) AM_READ_PORT("IN0")                                // Inputs
	AM_RANGE(0x400006, 0x400007) AM_READ_PORT("IN1")                                //
	AM_RANGE(0x400008, 0x400009) AM_READ_PORT("IN2")                                //
	AM_RANGE(0x40000a, 0x40000b) AM_READ_PORT("DSW0")                               //
	AM_RANGE(0x40000c, 0x40000d) AM_READ_PORT("DSW1")                               //
	AM_RANGE(0x40000e, 0x40000f) AM_READ_PORT("IN3")                                //
	AM_RANGE(0x800000, 0x81ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0x820000, 0x83ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0x840000, 0x85ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0x860000, 0x86ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x870000, 0x871fff) AM_RAM                                             // ???
	AM_RANGE(0x872000, 0x873fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x874000, 0x874fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0x878000, 0x8787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x878800, 0x878813) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0x878840, 0x87884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0x878860, 0x87886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x878870, 0x87887b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll
	AM_RANGE(0x878880, 0x878881) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x878890, 0x878891) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x8788a2, 0x8788a3) AM_READWRITE(metro_irq_cause_r,metro_irq_cause_w)  // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0x8788a4, 0x8788a5) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0x8788a8, 0x8788a9) AM_WRITE(metro_soundlatch_w)                       // To Sound CPU
	AM_RANGE(0x8788aa, 0x8788ab) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x8788ac, 0x8788ad) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
	AM_RANGE(0xc00000, 0xc0ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
ADDRESS_MAP_END


/***************************************************************************
                                Pururun
***************************************************************************/

static ADDRESS_MAP_START( pururun_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                             // ROM
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("IN0") AM_WRITE(metro_soundstatus_w)  // To Sound CPU
	AM_RANGE(0x400002, 0x400003) AM_READ_PORT("IN1")                                //
	AM_RANGE(0x400004, 0x400005) AM_READ_PORT("DSW0")                               //
	AM_RANGE(0x400006, 0x400007) AM_READ_PORT("IN2")                                //
	AM_RANGE(0x400002, 0x400009) AM_WRITE(metro_coin_lockout_4words_w)              // Coin Lockout
	AM_RANGE(0x800000, 0x80ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
	AM_RANGE(0xc00000, 0xc1ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0xc20000, 0xc3ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0xc40000, 0xc5ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0xc60000, 0xc6ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0xc70000, 0xc71fff) AM_RAM                                             // ???
	AM_RANGE(0xc72000, 0xc73fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0xc74000, 0xc74fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0xc78000, 0xc787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0xc78840, 0xc7884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0xc78860, 0xc7886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0xc78870, 0xc7887b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll Regs
	AM_RANGE(0xc78880, 0xc78881) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0xc78890, 0xc78891) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0xc788a2, 0xc788a3) AM_READWRITE(metro_irq_cause_r,metro_irq_cause_w)  // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0xc788a4, 0xc788a5) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0xc788a8, 0xc788a9) AM_WRITE(metro_soundlatch_w)                       // To Sound CPU
	AM_RANGE(0xc788aa, 0xc788ab) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0xc788ac, 0xc788ad) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
	AM_RANGE(0xc79700, 0xc79713) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
ADDRESS_MAP_END


/***************************************************************************
                            Toride II Adauchi Gaiden
***************************************************************************/

static ADDRESS_MAP_START( toride2g_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                             // ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("IN0") AM_WRITE(metro_soundstatus_w)  // Watchdog (R)? / To Sound CPU (W)
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("IN1")                                //
	AM_RANGE(0x800004, 0x800005) AM_READ_PORT("DSW0")                               //
	AM_RANGE(0x800006, 0x800007) AM_READ_PORT("IN2")                                //
	AM_RANGE(0x800002, 0x800009) AM_WRITE(metro_coin_lockout_4words_w)              // Coin Lockout
	AM_RANGE(0xc00000, 0xc1ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0xc20000, 0xc3ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0xc40000, 0xc5ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0xc60000, 0xc6ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0xc70000, 0xc71fff) AM_RAM                                             // ???
	AM_RANGE(0xc72000, 0xc73fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0xc74000, 0xc74fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0xc78000, 0xc787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0xc78840, 0xc7884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0xc78860, 0xc7886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0xc78870, 0xc7887b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll Regs
	AM_RANGE(0xc78880, 0xc78881) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0xc78890, 0xc78891) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0xc788a2, 0xc788a3) AM_READWRITE(metro_irq_cause_r, metro_irq_cause_w) // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0xc788a4, 0xc788a5) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0xc788a8, 0xc788a9) AM_WRITE(metro_soundlatch_w)                       // To Sound CPU
	AM_RANGE(0xc788aa, 0xc788ab) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0xc788ac, 0xc788ad) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
	AM_RANGE(0xc79700, 0xc79713) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
ADDRESS_MAP_END


/***************************************************************************
                            Blazing Tornado
***************************************************************************/

WRITE16_MEMBER(metro_state::blzntrnd_sound_w)
{
	soundlatch_byte_w(space, offset, data >> 8);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER(metro_state::blzntrnd_sh_bankswitch_w)
{
	UINT8 *RAM = memregion("audiocpu")->base();
	int bankaddress;

	bankaddress = 0x10000 + (data & 0x03) * 0x4000;
	membank("bank1")->set_base(&RAM[bankaddress]);
}

static ADDRESS_MAP_START( blzntrnd_sound_map, AS_PROGRAM, 8, metro_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( blzntrnd_sound_io_map, AS_IO, 8, metro_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(blzntrnd_sh_bankswitch_w)
	AM_RANGE(0x40, 0x40) AM_READ(soundlatch_byte_r) AM_WRITENOP
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("ymsnd", ym2610_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( blzntrnd_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM                                             // ROM
	AM_RANGE(0x200000, 0x21ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0x220000, 0x23ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0x240000, 0x25ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0x260000, 0x26ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x260000, 0x26ffff) AM_WRITENOP                                        // ??????
	AM_RANGE(0x270000, 0x271fff) AM_RAM                                             // ???
	AM_RANGE(0x272000, 0x273fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x274000, 0x274fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0x278000, 0x2787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x278860, 0x27886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x278870, 0x27887b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll
	AM_RANGE(0x278890, 0x278891) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x2788a2, 0x2788a3) AM_READWRITE(metro_irq_cause_r,metro_irq_cause_w)  // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0x2788a4, 0x2788a5) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0x2788aa, 0x2788ab) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x2788ac, 0x2788ad) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
	AM_RANGE(0x279700, 0x279713) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
//  AM_RANGE(0x300000, 0x300001) AM_READNOP                                         // Sound

	AM_RANGE(0x400000, 0x43ffff) AM_RAM_WRITE(metro_k053936_w) AM_SHARE("k053936_ram")  // 053936
	AM_RANGE(0x500000, 0x500fff) AM_DEVWRITE("k053936", k053936_device, linectrl_w)      // 053936 line control
	AM_RANGE(0x600000, 0x60001f) AM_DEVWRITE("k053936", k053936_device, ctrl_w)          // 053936 control

	AM_RANGE(0xe00000, 0xe00001) AM_READ_PORT("DSW0") AM_WRITENOP                   // Inputs
	AM_RANGE(0xe00002, 0xe00003) AM_READ_PORT("DSW1") AM_WRITE(blzntrnd_sound_w)    //
	AM_RANGE(0xe00004, 0xe00005) AM_READ_PORT("IN0")                                //
	AM_RANGE(0xe00006, 0xe00007) AM_READ_PORT("IN1")                                //
	AM_RANGE(0xe00008, 0xe00009) AM_READ_PORT("IN2")                                //
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
ADDRESS_MAP_END


/***************************************************************************
                                    Mouja
***************************************************************************/

WRITE8_MEMBER(metro_state::mouja_sound_rombank_w)
{
	membank("okibank")->set_entry((data >> 3) & 0x07);
}

static ADDRESS_MAP_START( mouja_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                             // ROM
	AM_RANGE(0x400000, 0x41ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0x420000, 0x43ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0x440000, 0x45ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0x470000, 0x471fff) AM_RAM                                             // ???
	AM_RANGE(0x472000, 0x473fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x474000, 0x474fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0x478000, 0x4787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x47880e, 0x47880f) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
	AM_RANGE(0x478810, 0x47881f) AM_WRITEONLY AM_SHARE("irq_levels")                // IRQ Levels
	AM_RANGE(0x478820, 0x47882f) AM_WRITEONLY AM_SHARE("irq_vectors")               // IRQ Vectors
	AM_RANGE(0x478830, 0x478831) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0x478832, 0x478833) AM_READWRITE(metro_irq_cause_r,metro_irq_cause_w)  // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0x478834, 0x478835) AM_WRITE(mouja_irq_timer_ctrl_w)                   // IRQ set timer count
	AM_RANGE(0x478836, 0x478837) AM_WRITE(watchdog_reset16_w)                       // Watchdog
	AM_RANGE(0x478850, 0x47885b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll Regs
	AM_RANGE(0x478860, 0x47886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x478880, 0x478881) AM_READ_PORT("IN0")                                // Inputs
	AM_RANGE(0x478882, 0x478883) AM_READ_PORT("IN1")                                //
	AM_RANGE(0x478884, 0x478885) AM_READ_PORT("DSW0")                               //
	AM_RANGE(0x478886, 0x478887) AM_READ_PORT("IN2")                                //
	AM_RANGE(0x478888, 0x478889) AM_WRITENOP                                        // ??
	AM_RANGE(0x479700, 0x479713) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0x800000, 0x800001) AM_WRITE8(mouja_sound_rombank_w, 0x00ff)
	AM_RANGE(0xc00000, 0xc00003) AM_DEVWRITE8("ymsnd", ym2413_device, write, 0x00ff)
	AM_RANGE(0xd00000, 0xd00001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0xffff)
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
#if 0
	AM_RANGE(0x460000, 0x46ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x478840, 0x47884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0x47883a, 0x47883b) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x800002, 0x800009) AM_WRITE(metro_coin_lockout_4words_w)              // Coin Lockout
#endif
ADDRESS_MAP_END

static ADDRESS_MAP_START( mouja_okimap, AS_0, 8, metro_state )
	AM_RANGE(0x00000, 0x1ffff) AM_ROM
	AM_RANGE(0x20000, 0x3ffff) AM_ROMBANK("okibank")
ADDRESS_MAP_END


/***************************************************************************
                                Puzzlet
***************************************************************************/

#define MCFG_PUZZLET_IO_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PUZZLET_IO, 0)

#define MCFG_PUZZLET_IO_DATA_CALLBACK(_devcb) \
	devcb = &puzzlet_io_device::set_data_cb(*device, DEVCB_##_devcb);

class puzzlet_io_device : public device_t {
public:
	puzzlet_io_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER( ce_w );
	DECLARE_WRITE_LINE_MEMBER( clk_w );

	template<class _Object> static devcb_base &set_data_cb(device_t &device, _Object object) { return downcast<puzzlet_io_device &>(device).data_cb.set_callback(object); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_write_line data_cb;
	required_ioport port;
	int ce, clk;
	int cur_bit;
	UINT8 value;
};

const device_type PUZZLET_IO = &device_creator<puzzlet_io_device>;


puzzlet_io_device::puzzlet_io_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PUZZLET_IO, "Puzzlet Coin/Start I/O", tag, owner, clock, "puzzlet_io", __FILE__),
		data_cb(*this),
		port(*this, ":IN0")
{
}

void puzzlet_io_device::device_start()
{
	data_cb.resolve_safe();
	save_item(NAME(ce));
	save_item(NAME(clk));
	save_item(NAME(cur_bit));
	save_item(NAME(value));
	ce = 1;
	clk = 1;
}

void puzzlet_io_device::device_reset()
{
	cur_bit = 0;
	value = 0xff;
}

WRITE_LINE_MEMBER(puzzlet_io_device::ce_w)
{
	if(ce && !state) {
		value = port->read();
		cur_bit = 0;
	} else if(!ce && state)
		data_cb(1);

	ce = state;
}

WRITE_LINE_MEMBER(puzzlet_io_device::clk_w)
{
	if(clk && !state) {
		if(cur_bit == 8)
			data_cb(1);
		else {
			data_cb((value >> cur_bit) & 1);
			cur_bit++;
		}
	}
	clk = state;
}


WRITE16_MEMBER(metro_state::puzzlet_irq_enable_w)
{
	if (ACCESSING_BITS_0_7)
		*m_irq_enable = data ^ 0xffff;
}

/* FIXME: algorithm not yet understood. */
WRITE16_MEMBER(metro_state::vram_0_clr_w)
{
	static int i;

//  printf("0 %04x %04x\n",offset,data);
	for(i=0;i<0x20/2;i++)
		m_vram_0[(offset*0x10+i)/2] = 0xffff;
}

WRITE16_MEMBER(metro_state::vram_1_clr_w)
{
	static int i;

//  printf("0 %04x %04x\n",offset,data);
	for(i=0;i<0x20/2;i++)
		m_vram_1[(offset*0x10+i)/2] = 0xffff;
}

WRITE16_MEMBER(metro_state::vram_2_clr_w)
{
	static int i;

//  printf("0 %04x %04x\n",offset,data);
	for(i=0;i<0x20/2;i++)
		m_vram_2[(offset*0x10+i)/2] = 0xffff;
}


// H8/3007 CPU
static ADDRESS_MAP_START( puzzlet_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x430000, 0x433fff) AM_RAM
	AM_RANGE(0x470000, 0x47dfff) AM_RAM

	AM_RANGE(0x500000, 0x500001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0xff00)
	AM_RANGE(0x580000, 0x580003) AM_DEVWRITE8("ymsnd", ym2413_device, write, 0xff00)

	AM_RANGE(0x700000, 0x71ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0x720000, 0x73ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0x740000, 0x75ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0x760000, 0x76ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x770000, 0x771fff) AM_RAM                                             // ???
	AM_RANGE(0x772000, 0x773fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x774000, 0x774fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x775000, 0x775fff) AM_RAM_WRITE(vram_0_clr_w)
	AM_RANGE(0x776000, 0x776fff) AM_RAM_WRITE(vram_1_clr_w)
	AM_RANGE(0x777000, 0x777fff) AM_RAM_WRITE(vram_2_clr_w)
	AM_RANGE(0x778000, 0x7787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x778800, 0x778813) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0x778840, 0x77884f) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0x778860, 0x77886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x778870, 0x77887b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll
	AM_RANGE(0x778890, 0x778891) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x7788a2, 0x7788a3) AM_WRITE(metro_irq_cause_w)                            // IRQ Cause
	AM_RANGE(0x7788a4, 0x7788a5) AM_WRITE(puzzlet_irq_enable_w) AM_SHARE("irq_enable")  // IRQ Enable
	AM_RANGE(0x7788aa, 0x7788ab) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x7788ac, 0x7788ad) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control

	AM_RANGE(0x7f2000, 0x7f3fff) AM_RAM

	AM_RANGE(0x7f8880, 0x7f8881) AM_READ_PORT("IN1")
	AM_RANGE(0x7f8884, 0x7f8885) AM_READ_PORT("DSW0")
	AM_RANGE(0x7f8886, 0x7f8887) AM_READ_PORT("DSW0")

	AM_RANGE(0x7f88a2, 0x7f88a3) AM_READ(metro_irq_cause_r)                         // IRQ Cause
ADDRESS_MAP_END


WRITE16_MEMBER(metro_state::puzzlet_portb_w)
{
//  popmessage("PORTB %02x", data);
}

static ADDRESS_MAP_START( puzzlet_io_map, AS_IO, 16, metro_state )
	AM_RANGE(h8_device::PORT_7,   h8_device::PORT_7) AM_READ_PORT("IN2")
	AM_RANGE(h8_device::PORT_B,   h8_device::PORT_B) AM_READ_PORT("DSW0") AM_WRITE(puzzlet_portb_w)
ADDRESS_MAP_END


/***************************************************************************
                                Varia Metal
***************************************************************************/

WRITE8_MEMBER(metro_state::vmetal_control_w)
{
	/* Lower nibble is the coin control bits shown in
	   service mode, but in game mode they're different */
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x08);  /* 2nd coin schute activates coin 0 counter in game mode?? */
//  machine().bookkeeping().coin_lockout_w(0, data & 0x01);  /* always on in game mode?? */
	machine().bookkeeping().coin_lockout_w(1, data & 0x02);  /* never activated in game mode?? */

	if ((data & 0x40) == 0)
		m_essnd->reset();
	else
		m_essnd->play();

	if (data & 0x10)
		m_essnd->set_bank_base(0x100000);
	else
		m_essnd->set_bank_base(0x000000);

	if (data & 0xa0)
		logerror("%s: Writing unknown bits %04x to $200000\n",machine().describe_context(),data);
}

WRITE8_MEMBER(metro_state::vmetal_es8712_w)
{
	/* Many samples in the ADPCM ROM are actually not used.

	Snd         Offset Writes                 Sample Range
	     0000 0004 0002 0006 000a 0008 000c
	--   ----------------------------------   -------------
	00   006e 0001 00ab 003c 0002 003a 003a   01ab6e-023a3c
	01   003d 0002 003a 001d 0002 007e 007e   023a3d-027e1d
	02   00e2 0003 0005 002e 0003 00f3 00f3   0305e2-03f32e
	03   000a 0005 001e 00f6 0005 00ec 00ec   051e0a-05ecf6
	04   00f7 0005 00ec 008d 0006 0060 0060   05ecf7-06608d
	05   0016 0008 002e 0014 0009 0019 0019   082e16-091914
	06   0015 0009 0019 0094 000b 0015 0015   091915-0b1594
	07   0010 000d 0012 00bf 000d 0035 0035   0d1210-0d35bf
	08   00ce 000e 002f 0074 000f 0032 0032   0e2fce-0f3274
	09   0000 0000 0000 003a 0000 007d 007d   000000-007d3a
	0a   0077 0000 00fa 008d 0001 00b6 00b6   00fa77-01b68d
	0b   008e 0001 00b6 00b3 0002 0021 0021   01b68e-0221b3
	0c   0062 0002 00f7 0038 0003 00de 00de   02f762-03de38
	0d   00b9 0005 00ab 00ef 0006 0016 0016   05abb9-0616ef
	0e   00dd 0007 0058 00db 0008 001a 001a   0758dd-081adb
	0f   00dc 0008 001a 002e 0008 008a 008a   081adc-088a2e
	10   00db 0009 00d7 00ff 000a 0046 0046   09d7db-0a46ff
	11   0077 000c 0003 006d 000c 0080 0080   0c0377-0c806d
	12   006e 000c 0080 006c 000d 0002 0002   0c806e-0d026c
	13   006d 000d 0002 002b 000d 0041 0041   0d026d-0d412b
	14   002c 000d 0041 002a 000d 00be 00be   0d412c-0dbe2a
	15   002b 000d 00be 0029 000e 0083 0083   0dbe2b-0e8329
	16   002a 000e 0083 00ee 000f 0069 0069   0e832a-0f69ee
	*/

	m_essnd->es8712_w(space, offset, data);
	logerror("%s: Writing %04x to ES8712 offset %02x\n", machine().describe_context(), data, offset);
}

static ADDRESS_MAP_START( vmetal_map, AS_PROGRAM, 16, metro_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM                                             // ROM
	AM_RANGE(0x100000, 0x11ffff) AM_RAM_WRITE(metro_vram_0_w) AM_SHARE("vram_0")    // Layer 0
	AM_RANGE(0x120000, 0x13ffff) AM_RAM_WRITE(metro_vram_1_w) AM_SHARE("vram_1")    // Layer 1
	AM_RANGE(0x140000, 0x15ffff) AM_RAM_WRITE(metro_vram_2_w) AM_SHARE("vram_2")    // Layer 2
	AM_RANGE(0x160000, 0x16ffff) AM_READ(metro_bankedrom_r)                         // Banked ROM
	AM_RANGE(0x170000, 0x171fff) AM_RAM                                             // ???
	AM_RANGE(0x172000, 0x173fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    // Palette
	AM_RANGE(0x174000, 0x174fff) AM_RAM AM_SHARE("spriteram")                       // Sprites
	AM_RANGE(0x178000, 0x1787ff) AM_RAM AM_SHARE("tiletable")                       // Tiles Set
	AM_RANGE(0x178840, 0x17884d) AM_WRITE(metro_blitter_w) AM_SHARE("blitter_regs") // Tiles Blitter
	AM_RANGE(0x178860, 0x17886b) AM_WRITE(metro_window_w) AM_SHARE("window")        // Tilemap Window
	AM_RANGE(0x178870, 0x17887b) AM_WRITEONLY AM_SHARE("scroll")                    // Scroll
	AM_RANGE(0x178880, 0x178881) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x178890, 0x178891) AM_WRITENOP                                        // ? CRT
	AM_RANGE(0x1788a2, 0x1788a3) AM_READWRITE(metro_irq_cause_r, metro_irq_cause_w) // IRQ Cause / IRQ Acknowledge
	AM_RANGE(0x1788a4, 0x1788a5) AM_WRITEONLY AM_SHARE("irq_enable")                // IRQ Enable
	AM_RANGE(0x1788aa, 0x1788ab) AM_WRITEONLY AM_SHARE("rombank")                   // Rom Bank
	AM_RANGE(0x1788ac, 0x1788ad) AM_WRITEONLY AM_SHARE("screenctrl")                // Screen Control
	AM_RANGE(0x179700, 0x179713) AM_WRITEONLY AM_SHARE("videoregs")                 // Video Registers
	AM_RANGE(0x200000, 0x200001) AM_READ_PORT("P1_P2") AM_WRITE8(vmetal_control_w, 0x00ff)
	AM_RANGE(0x200002, 0x200003) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x300000, 0x31ffff) AM_READ(balcube_dsw_r)                             // DSW x 3
	AM_RANGE(0x400000, 0x400001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff )
	AM_RANGE(0x400002, 0x400003) AM_DEVWRITE8("oki", okim6295_device, write, 0x00ff)
	AM_RANGE(0x500000, 0x50000d) AM_WRITE8(vmetal_es8712_w, 0x00ff)
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM AM_MIRROR(0x0f0000)                         // RAM (mirrored)
ADDRESS_MAP_END


/***************************************************************************


                                Input Ports


***************************************************************************/


#define JOY_LSB(_n_, _b1_, _b2_, _b3_, _b4_) \
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_##_b1_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_##_b2_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_##_b3_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_##_b4_         ) PORT_PLAYER(_n_)

#define JOY_MSB(_n_, _b1_, _b2_, _b3_, _b4_) \
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_##_b1_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_##_b2_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_##_b3_         ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_##_b4_         ) PORT_PLAYER(_n_)

#define COINS \
	PORT_BIT(  0x0001, IP_ACTIVE_LOW,  IPT_SERVICE1 ) \
	PORT_BIT(  0x0002, IP_ACTIVE_LOW,  IPT_TILT     ) \
	PORT_BIT(  0x0004, IP_ACTIVE_LOW,  IPT_COIN1 ) PORT_IMPULSE(2) \
	PORT_BIT(  0x0008, IP_ACTIVE_LOW,  IPT_COIN2 ) PORT_IMPULSE(2) \
	PORT_BIT(  0x0010, IP_ACTIVE_LOW,  IPT_START1   ) \
	PORT_BIT(  0x0020, IP_ACTIVE_LOW,  IPT_START2   ) \
	PORT_BIT(  0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN  ) \
	PORT_BIT(  0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN  )

#define COINS_SOUND \
	PORT_BIT(  0x0001, IP_ACTIVE_LOW,  IPT_SERVICE1 ) \
	PORT_BIT(  0x0002, IP_ACTIVE_LOW,  IPT_TILT     ) \
	PORT_BIT(  0x0004, IP_ACTIVE_LOW,  IPT_COIN1 ) PORT_IMPULSE(2) \
	PORT_BIT(  0x0008, IP_ACTIVE_LOW,  IPT_COIN2 ) PORT_IMPULSE(2) \
	PORT_BIT(  0x0010, IP_ACTIVE_LOW,  IPT_START1   ) \
	PORT_BIT(  0x0020, IP_ACTIVE_LOW,  IPT_START2   ) \
	PORT_BIT(  0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN  ) \
	PORT_BIT(  0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL  ) PORT_CUSTOM_MEMBER(DEVICE_SELF, metro_state,custom_soundstatus_r, NULL)   /* From Sound CPU */


#define COINAGE_SERVICE_LOC(DIPBANK) \
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION(#DIPBANK":1,2,3") \
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) ) \
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION(#DIPBANK":4,5,6") \
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) ) \
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION(#DIPBANK":7") \
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) ) \
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, #DIPBANK":8" )


#define COINAGE_FLIP_LOC(DIPBANK) \
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION(#DIPBANK":1,2,3") \
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) ) \
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION(#DIPBANK":4,5,6") \
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) ) \
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION(#DIPBANK":7") \
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) ) \
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )


/***************************************************************************
                                    Bal Cube
***************************************************************************/

static INPUT_PORTS_START( balcube )
	PORT_START("IN0")   // $500000
	COINS

	PORT_START("IN1")   // $500002
	JOY_LSB(1, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)
	JOY_MSB(2, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)

	PORT_START("DSW0")  // Strangely mapped in the 0x400000-0x41ffff range
	COINAGE_SERVICE_LOC(SW1)
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0400, 0x0400, "2 Players Game" )        PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, "1 Credit" )
	PORT_DIPSETTING(      0x0400, "2 Credits" )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("IN2")   // Strangely mapped in the 0x400000-0x41ffff range
	PORT_BIT(  0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused
INPUT_PORTS_END


/***************************************************************************
                                Bang Bang Ball
***************************************************************************/

static INPUT_PORTS_START( bangball )
	PORT_START("IN0")   // $d00000
	COINS

	PORT_START("IN1")   // $d00002
	JOY_LSB(1, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)
	JOY_MSB(2, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)

	PORT_START("DSW0")  // Strangely mapped in the 0xc00000-0xc1ffff range
	COINAGE_SERVICE_LOC(SW1)
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPSETTING(      0x0c00, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Language ) )     PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )

	PORT_START("IN2")   // Strangely mapped in the 0xc00000-0xc1ffff range
	PORT_BIT(  0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN ) // used for debug
INPUT_PORTS_END

/***************************************************************************
                                Battle Bubble
***************************************************************************/

static INPUT_PORTS_START( batlbubl )
	PORT_START("IN1")
	JOY_LSB(1, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)
	JOY_MSB(2, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)

	PORT_START("DSW0")  // Strangely mapped in the 0x300000-0x31ffff range
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x0004, "3" )
	PORT_DIPSETTING(      0x000c, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0800, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("IN0")   // $200004
	COINS

	PORT_START("IN2")   // Strangely mapped in the 0x300000-0x31ffff range
	// DSW3 is used for debug (it's not soldered on the PCB)
	PORT_DIPNAME( 0x0001, 0x0001, "0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Debug Mode?" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************
                             Mouse Shooter GoGo
***************************************************************************/

static INPUT_PORTS_START( msgogo )
	PORT_START("COINS") // $200000
	COINS

	PORT_START("JOYS")  // $200002
	JOY_LSB(1, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)
	JOY_MSB(2, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)

	PORT_START("DSW0")  // Strangely mapped in the 0x300000-0x31ffff range
	COINAGE_SERVICE_LOC(SW1)
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy )    )  // 0
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal )  )  // 1
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard )    )  // 2
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )  // 3
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Allow P2 to Join Game" )     PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Language ) )     PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )

	PORT_START("IN2")   // Strangely mapped in the 0x300000-0x31ffff range
	// DSW3 is used for debug (it's not soldered on the PCB)
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Debug: Offset" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Debug: Menu" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************
                            Blazing Tornado
***************************************************************************/

static INPUT_PORTS_START( blzntrnd )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x0007, 0x0004, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(      0x0007, "Beginner" )
	PORT_DIPSETTING(      0x0006, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hardest ) )
	PORT_DIPSETTING(      0x0001, "Expert" )
	PORT_DIPSETTING(      0x0000, "Master" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x00c0, 0x0000, "Control Panel" )         PORT_DIPLOCATION("SW3:7,8")
	PORT_DIPSETTING(      0x0000, "4 Players" )
//  PORT_DIPSETTING(      0x0040, "4 Players" )
	PORT_DIPSETTING(      0x0080, "1P & 2P Tag only" )
	PORT_DIPSETTING(      0x00c0, "1P & 2P vs only" )
	PORT_DIPNAME( 0x0300, 0x0300, "Half Continue" )         PORT_DIPLOCATION("SW4:1,2")
	PORT_DIPSETTING(      0x0000, "6C to start, 3C to continue" )
	PORT_DIPSETTING(      0x0100, "4C to start, 2C to continue" )
	PORT_DIPSETTING(      0x0200, "2C to start, 1C to continue" )
	PORT_DIPSETTING(      0x0300, "Disabled" )
	PORT_DIPUNUSED_DIPLOC( 0x0400, 0x0400, "SW4:3" ) /* Not read in Service Mode */
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW4:4" ) /* Not read in Service Mode */
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SW4:5" ) /* Not read in Service Mode */
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW4:6" ) /* Not read in Service Mode */
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW4:7" ) /* Not read in Service Mode */
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW4:8" ) /* Not read in Service Mode */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0300, 0x0300, "CP Single" )         PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, "2:00" )
	PORT_DIPSETTING(      0x0200, "2:30" )
	PORT_DIPSETTING(      0x0100, "3:00" )
	PORT_DIPSETTING(      0x0000, "3:30" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "CP Tag" )            PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "2:00" )
	PORT_DIPSETTING(      0x0800, "2:30" )
	PORT_DIPSETTING(      0x0400, "3:00" )
	PORT_DIPSETTING(      0x0000, "3:30" )
	PORT_DIPNAME( 0x3000, 0x3000, "Vs Single" )         PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x3000, "2:30" )
	PORT_DIPSETTING(      0x2000, "3:00" )
	PORT_DIPSETTING(      0x1000, "4:00" )
	PORT_DIPSETTING(      0x0000, "5:00" )
	PORT_DIPNAME( 0xc000, 0xc000, "Vs Tag" )            PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0xc000, "2:30" )
	PORT_DIPSETTING(      0x8000, "3:00" )
	PORT_DIPSETTING(      0x4000, "4:00" )
	PORT_DIPSETTING(      0x0000, "5:00" )

	PORT_START("IN0")
	JOY_LSB(1, BUTTON1, BUTTON2, BUTTON3, BUTTON4)
	JOY_MSB(2, BUTTON1, BUTTON2, BUTTON3, BUTTON4)

	PORT_START("IN1")
	JOY_LSB(3, BUTTON1, BUTTON2, BUTTON3, BUTTON4)
	JOY_MSB(4, BUTTON1, BUTTON2, BUTTON3, BUTTON4)

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x0002, IP_ACTIVE_LOW)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START4 )
INPUT_PORTS_END


/***************************************************************************
                            Grand Striker 2
***************************************************************************/

static INPUT_PORTS_START( gstrik2 )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x0003, 0x0003, "Player Vs Com" )         PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(      0x0003, "1:00" )
	PORT_DIPSETTING(      0x0002, "1:30" )
	PORT_DIPSETTING(      0x0001, "2:00" )
	PORT_DIPSETTING(      0x0000, "2:30" )
	PORT_DIPNAME( 0x000c, 0x000c, "1P Vs 2P" )          PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(      0x000c, "0:45" )
	PORT_DIPSETTING(      0x0008, "1:00" )
	PORT_DIPSETTING(      0x0004, "1:30" )
	PORT_DIPSETTING(      0x0000, "2:00" )
	PORT_DIPNAME( 0x0030, 0x0030, "Extra Time" )            PORT_DIPLOCATION("SW3:5,6")
	PORT_DIPSETTING(      0x0030, "0:30" )
	PORT_DIPSETTING(      0x0020, "0:45" )
	PORT_DIPSETTING(      0x0010, "1:00" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:7") /* Does not in Service Mode */
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Time Period" )           PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x0080, "Sudden Death" )
	PORT_DIPSETTING(      0x0000, "Full" )
	PORT_DIPNAME( 0x0700, 0x0400, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(      0x0700, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(      0x0600, DEF_STR( Easier ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hardest ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:5") /* Does not in Service Mode */
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:6") /* Does not in Service Mode */
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW4:8" )      /* Does not in Service Mode */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x001f, 0x001f, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3,4,5")
	PORT_DIPSETTING(      0x001c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x001d, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(      0x001e, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0019, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(      0x0015, DEF_STR( 3C_3C ) )
	PORT_DIPSETTING(      0x001a, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(      0x001f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(      0x0011, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0008, "4 Coins/6 Credits" )
	PORT_DIPSETTING(      0x0016, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000d, "3 Coins/5 Credits" )
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_7C ) )
	PORT_DIPSETTING(      0x0000, "4 Coins/8 Credits" )
	PORT_DIPSETTING(      0x0009, "3 Coins/6 Credits" )
	PORT_DIPSETTING(      0x0012, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(      0x001b, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, "3 Coins/7 Credits" )
	PORT_DIPSETTING(      0x000e, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x0001, "3 Coins/8 Credits" )
	PORT_DIPSETTING(      0x000a, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(      0x0017, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_8C ) )
	PORT_DIPSETTING(      0x0013, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x00e0, "Same as Coin A" )
	PORT_DIPNAME( 0x0300, 0x0300, "Credits to Start" )      PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, "1" )
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Credits to Continue" )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "1" )
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x1000, 0x1000, "Continue" )          PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:6") /* Does not in Service Mode */
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Playmode" )          PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, "1 Credit for 1 Player" )
	PORT_DIPSETTING(      0x0000, "1 Credit for 2 Players" )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN0")
	JOY_LSB(1, BUTTON1, BUTTON2, BUTTON3, UNUSED)
	JOY_MSB(2, BUTTON1, BUTTON2, BUTTON3, UNUSED)

	PORT_START("IN1")
	/* Not Used */

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x0002, IP_ACTIVE_LOW )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************
                                Daitoride
***************************************************************************/

/* If only ONE of the "Coinage" is set to "Free Play", it is in fact "5C_1C".

   IN2 bits 12 and 13 are in fact "merged" :

     12  13    effect
     Off Off   Continue, Retry level
     On  Off   Continue, Ask player for retry
     Off On    No continue
     On  On    Continue, Retry level

*/
static INPUT_PORTS_START( daitorid )
	PORT_START("IN0") // $c00000
	COINS_SOUND

	PORT_START("IN1") // $c00002
	JOY_LSB(1, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)      // BUTTON2 and BUTTON3 in "test mode" only
	JOY_MSB(2, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)      // BUTTON2 and BUTTON3 in "test mode" only

	PORT_START("DSW0") // $c00004
	COINAGE_SERVICE_LOC(SW1)

	PORT_DIPNAME( 0x0300, 0x0300, "Timer Speed" )               PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, "Slower" )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, "Fast" )
	PORT_DIPSETTING(      0x0000, "Fastest" )
	PORT_DIPUNUSED_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPNAME( 0x0800, 0x0800, "Winning Rounds (Player VS Player)" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, "1/1" )
	PORT_DIPSETTING(      0x0800, "2/3" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x3000, "Retry Level" )
	PORT_DIPSETTING(      0x2000, "Ask Player" )
	PORT_DIPSETTING(      0x1000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, "Retry Level" )   /* Dulicate setting */
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )

	PORT_START("IN2") // $c00006
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Dharma Doujou
***************************************************************************/

/* Difficulty refers to how difficult the stack is solve in the given time.
   The manual calls it "Placement Difficulty" or block placement in the
   stack when you start the level. */

static INPUT_PORTS_START( dharma )
	PORT_START("IN0") //$c00000
	COINS_SOUND

	PORT_START("IN1") //$c00002
	JOY_LSB(1, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)      // BUTTON2 and BUTTON3 in "test mode" only
	JOY_MSB(2, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)      // BUTTON2 and BUTTON3 in "test mode" only

	PORT_START("DSW0") //$c00004
	COINAGE_SERVICE_LOC(SW1)

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2") // Check code at 0x00da0a and see notes
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )     //   Table offset : 0x00e718
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )   //   Table offset : 0x00e770
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )     //   Table offset : 0x00e6c0
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )  //   Table offset : 0x00e668
	PORT_DIPNAME( 0x0c00, 0x0c00, "Timer" )         PORT_DIPLOCATION("SW2:3,4") // Timer (crab) speed
	PORT_DIPSETTING(      0x0800, "Slow" )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, "Fast" )
	PORT_DIPSETTING(      0x0000, "Fastest" )
	PORT_DIPNAME( 0x1000, 0x1000, "2 Players Game" )    PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, "2 Credits" )
	PORT_DIPSETTING(      0x0000, "1 Credit" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Freeze (Cheat)")     PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2") // $c00006
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************
                                Gun Master
***************************************************************************/

static INPUT_PORTS_START( gunmast )
	PORT_START("IN0") //$400000
	COINS_SOUND

	PORT_START("IN1") //$400002
	JOY_LSB(1, BUTTON1, BUTTON2, BUTTON3, UNKNOWN)
	JOY_MSB(2, BUTTON1, BUTTON2, BUTTON3, UNKNOWN)

	PORT_START("DSW0") //$400004
	COINAGE_SERVICE_LOC(SW1)

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Allow P2 to Join Game" )     PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" ) /* Listed as "Unused" */

	PORT_START("IN2")   // IN3 - $400006
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/***************************************************************************
                                Karate Tournament
***************************************************************************/

static INPUT_PORTS_START( karatour )
	PORT_START("IN0") // $400002
	JOY_LSB(2, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)

	PORT_START("IN1") //$400004
	COINS

	PORT_START("DSW0") // $400006
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0001, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Time" )              PORT_DIPLOCATION("SW2:5") /* Listed as "Timer" */
	PORT_DIPSETTING(      0x0010, "60" ) /* Listed as "Normal" */
	PORT_DIPSETTING(      0x0000, "40" ) /* Listed as "Short" */
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW2:6") /* Listed as "Unused" */
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW1") // $40000a
	COINAGE_FLIP_LOC(SW1)
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("IN2") // $40000c
	JOY_LSB(1, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)
INPUT_PORTS_END


/***************************************************************************
                                Lady Killer
***************************************************************************/

static INPUT_PORTS_START( ladykill )
	PORT_START("IN0")   /*$400002*/
	JOY_LSB(2, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)

	PORT_START("IN1")   /*$400004*/
	COINS

	PORT_START("DSW0")  // $400006
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0001, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0010, 0x0000, "Nudity" )                    PORT_DIPLOCATION("SW2:5") // Manual calls this "Sexy Version"
	PORT_DIPSETTING(      0x0010, "Partial" )
	PORT_DIPSETTING(      0x0000, "Full" )
	PORT_DIPNAME( 0x0020, 0x0020, "Service Mode / Free Play" )  PORT_DIPLOCATION("SW2:6")   // Keep Start2 pressed during boot - Manual states "Don't Change"
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW1") /*$40000a*/
	COINAGE_FLIP_LOC(SW1)
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" )        /* Manual states "Don't Change" */

	PORT_START("IN2") /*$40000c*/
	JOY_LSB(1, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)
INPUT_PORTS_END

static INPUT_PORTS_START( moegonta )
	PORT_INCLUDE( ladykill )

	PORT_MODIFY("DSW0") // $400006
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" )        /* Same as 'ladykill' but NO "Nudity" Dip Switch */
INPUT_PORTS_END


/***************************************************************************
                                Last Fortress
***************************************************************************/

/* The code which tests IN4 bit 7 is the SAME as the one for 'lastfero'.
   So WHY can't the game display cards instead of mahjong tiles ?
   Is it due to different GFX ROMS or to an emulation bug ?
*/

static INPUT_PORTS_START( lastfort )
	PORT_START("IN0")   /*$c00004*/
	COINS

	PORT_START("IN1")   /*$c00006*/
	JOY_LSB(1, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)      /* BUTTON2 and BUTTON3 in "test mode" only*/

	PORT_START("IN2")   /*$c00008*/
	JOY_LSB(2, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)      /*BUTTON2 and BUTTON3 in "test mode" only*/

	PORT_START("DSW0")  /*$c0000a*/
	COINAGE_SERVICE_LOC(SW1)

	PORT_START("DSW1")  // $c0000c
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2") // Timer speed
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )     //   Slow
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )   //   Normal
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )     //   Fast
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )  //   Fastest
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Retry Level On Continue" )   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0008, "Ask Player" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0010, 0x0010, "2 Players Game" )        PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0010, "2 Credits" )
	PORT_DIPSETTING(      0x0000, "1 Credit" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Tiles" )             PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, "Mahjong" )
//  PORT_DIPSETTING(      0x0000, "Cards" )             // Not working - See notes

	PORT_START("IN3")   // $c0000e
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                            Last Fortress (Erotic)
***************************************************************************/

/* Same as 'lastfort' but WORKING "Tiles" Dip Switch */
static INPUT_PORTS_START( lastfero )
	PORT_INCLUDE( lastfort )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2") // Timer speed
	PORT_DIPSETTING(      0x0000, DEF_STR( Easiest ) )  //   Slowest
	PORT_DIPSETTING(      0x0001, DEF_STR( Easy ) )     //   Slow
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )   //   Normal
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )     //   Fast
	PORT_DIPNAME( 0x0080, 0x0080, "Tiles" )             PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, "Mahjong" )
	PORT_DIPSETTING(      0x0000, "Cards" )
INPUT_PORTS_END


/***************************************************************************
                            Mahjong Doukyuusei
***************************************************************************/

static INPUT_PORTS_START( mj_panel )
	PORT_START("KEY0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( dokyusei )
	PORT_INCLUDE( mj_panel )

	PORT_START("DSW0")  // $478884.w
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0300, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Game Sound" )            PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Auto TSUMO after REACH" )    PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )

	PORT_START("DSW1")  // $478886.w
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "CPU wears clothes on RON" )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0400, 0x0400, "CPU clothes on continue play" )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Return to default" )
	PORT_DIPSETTING(      0x0000, "Keep current status" )
	PORT_SERVICE_DIPLOC(  0x0800, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPNAME( 0x1000, 0x0000, "Self Test" )         PORT_DIPLOCATION("SW2:5") //!
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Unknown 2-6" )           PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Unknown 2-7" )           PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Unknown 2-8" )           PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************
                        Mahjong Gakuensai 1 & 2
***************************************************************************/

/* Same as dokyusei, without the DSWs (these games have an eeprom) */

static INPUT_PORTS_START( gakusai )

	PORT_INCLUDE( mj_panel )

INPUT_PORTS_END


/***************************************************************************
                                    Mouja
***************************************************************************/

static INPUT_PORTS_START( mouja )
	PORT_START("IN0") //$478880
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN1") //$478882
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x0080, IP_ACTIVE_LOW)

	PORT_START("DSW0") //$478884
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Free_Play ) )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0004, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0800, 0x0000, "Winning Rounds (Player VS Computer)" )   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "1/1" )
	PORT_DIPSETTING(      0x0000, "2/3" )
	PORT_DIPNAME( 0x1000, 0x1000, "Winning Rounds (Player VS Player)" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, "1/1" )
	PORT_DIPSETTING(      0x0000, "2/3" )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )

	PORT_START("IN2") //$478886
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Pang Pom's
***************************************************************************/

static INPUT_PORTS_START( pangpoms )
	PORT_START("IN0") //$800004
	COINS

	PORT_START("IN1") //$800006
	JOY_LSB(1, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)

	PORT_START("IN2") //$800008
	JOY_LSB(2, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)

	PORT_START("DSW0") //$80000a
	COINAGE_SERVICE_LOC(SW1)

	PORT_START("DSW1") //$80000c
	PORT_DIPNAME( 0x0003, 0x0003, "Time Speed" )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, "Slowest" )           // 60 (1 game sec. lasts x/60 real sec.)
	PORT_DIPSETTING(      0x0001, "Slow"    )           // 90
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal )  )      // 120
	PORT_DIPSETTING(      0x0002, "Fast"    )           // 150
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, "1" )
	PORT_DIPSETTING(      0x0004, "2" )
	PORT_DIPSETTING(      0x000c, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0030, 0x0020, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0020, "400k and 800k" )
	PORT_DIPSETTING(      0x0030, "400k" )
	PORT_DIPSETTING(      0x0010, "800k" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_START("IN3") //$80000e
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Poitto!
***************************************************************************/

static INPUT_PORTS_START( poitto )
	PORT_START("IN0") //$800000
	COINS_SOUND

	PORT_START("IN1") //$800002
	JOY_LSB(1, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)      // BUTTON2 and BUTTON3 in "test mode" only
	JOY_MSB(2, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)      // BUTTON2 and BUTTON3 in "test mode" only

	PORT_START("DSW0") //$800004
	COINAGE_SERVICE_LOC(SW1)

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SW2:5" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )

	PORT_START("IN2") //$800006
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Puzzlet
***************************************************************************/

static INPUT_PORTS_START( puzzlet )
	PORT_START("IN0")       // IN0 - ser B
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")       // IN1 - 7f8880.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)   // Next
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)   // Rotate CW
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)   // Push
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")       // IN2 - port 7
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")      // IN3 - dsw?
	PORT_DIPUNKNOWN( 0x0001, 0x0001 )
	PORT_DIPUNKNOWN( 0x0002, 0x0002 )
	PORT_DIPUNKNOWN( 0x0004, 0x0004 )
	PORT_DIPUNKNOWN( 0x0008, 0x0008 )
	PORT_DIPUNKNOWN( 0x0010, 0x0010 )
	PORT_DIPUNKNOWN( 0x0020, 0x0020 )
	PORT_DIPUNKNOWN( 0x0040, 0x0040 )
	PORT_DIPUNKNOWN( 0x0080, 0x0080 )

	PORT_DIPUNKNOWN( 0x0100, 0x0100 )
	PORT_DIPUNKNOWN( 0x0200, 0x0200 )
	PORT_DIPUNKNOWN( 0x0400, 0x0400 )
	PORT_DIPUNKNOWN( 0x0800, 0x0800 )
	PORT_DIPUNKNOWN( 0x1000, 0x1000 )
	PORT_DIPUNKNOWN( 0x2000, 0x2000 )
	PORT_DIPUNKNOWN( 0x4000, 0x4000 )
	PORT_DIPUNKNOWN( 0x8000, 0x8000 )
INPUT_PORTS_END


/***************************************************************************
                                Puzzli
***************************************************************************/

static INPUT_PORTS_START( puzzli )
	PORT_START("IN0") //$c00000
	COINS_SOUND

	PORT_START("IN1") //$c00002
	JOY_LSB(1, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)      // BUTTON3 in "test mode" only
	JOY_MSB(2, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)      // BUTTON3 in "test mode" only

	PORT_START("DSW0") //$c00004
	COINAGE_SERVICE_LOC(SW1)

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
//  PORT_DIPSETTING(      0x0100, DEF_STR( Normal ) )           // Duplicated setting
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Join In" )               PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0800, 0x0800, "2 Players Game" )            PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, "1 Credit" )
	PORT_DIPSETTING(      0x0800, "2 Credits" )
	PORT_DIPNAME( 0x1000, 0x1000, "Winning Rounds (Player VS Player)" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, "1/1" )
	PORT_DIPSETTING(      0x1000, "2/3" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )

	PORT_START("IN2") //$c00006
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Sankokushi
***************************************************************************/

static INPUT_PORTS_START( 3kokushi )
	PORT_START("IN0") //$c00000
	COINS_SOUND

	PORT_START("IN1") //$c00002
	JOY_LSB(1, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)
	JOY_MSB(2, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)

	PORT_START("DSW0") //$c00004
	COINAGE_FLIP_LOC(SW1)
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2") // Timer speed
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )         //   Slow
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )       //   Normal
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )         //   Fast
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )      //   Fastest
	PORT_DIPUNUSED_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW2:4" )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Service Mode / Free Play" )  PORT_DIPLOCATION("SW2:6")   // Keep Start2 pressed during boot
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0xc000, "Helps" )                     PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x4000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x8000, "4" )
INPUT_PORTS_END


/***************************************************************************
                                Pururun
***************************************************************************/

static INPUT_PORTS_START( pururun )
	PORT_START("IN0") //$400000
	COINS_SOUND

	PORT_START("IN1") //$400002
	JOY_LSB(1, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)      // BUTTON3 in "test mode" only
	JOY_MSB(2, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)      // BUTTON3 in "test mode" only

	PORT_START("DSW0") //$400004
	COINAGE_SERVICE_LOC(SW1)

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2") // Distance to goal
	PORT_DIPSETTING(      0x0200, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Join In" )           PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0800, 0x0800, "2 Players Game" )        PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, "1 Credit" )
	PORT_DIPSETTING(      0x0800, "2 Credits" )
	PORT_DIPNAME( 0x1000, 0x1000, "Bombs" )             PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )

	PORT_START("IN2")   // IN3 - $400006
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Sky Alert
***************************************************************************/

/* The game shows wrong values on screen for the "Bonus Life" Dip Switch !
   The wrong values are text which is stored at 0x02671a, and to determine
   which text to display, the routine at 0x0022f2 is called.
   The REAL "Bonus Life" table is stored at 0x0097f6, and to determine what
   are the values, the routine at 0x00974e is called.

   Here is the correspondance between real and fake values :

        Real         Fake
     100K, 400K   100K, 400K
     200K, 400K    50K, 300K
     200K         150K, 500K
       "none"       "none"

*/
static INPUT_PORTS_START( skyalert )
	PORT_START("IN0") //$400004
	COINS

	PORT_START("IN1") //$400006
	JOY_LSB(1, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)      // BUTTON3 in "test mode" only

	PORT_START("IN2") //$400008
	JOY_LSB(2, BUTTON1, BUTTON2, UNKNOWN, UNKNOWN)      // BUTTON3 in "test mode" only

	PORT_START("DSW0") //$40000a
	COINAGE_SERVICE_LOC(SW1)

	PORT_START("DSW1") //$40000c
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, "1" )
	PORT_DIPSETTING(      0x0004, "2" )
	PORT_DIPSETTING(      0x000c, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:5,6") // See notes
	PORT_DIPSETTING(      0x0030, "100K, every 400K" )
	PORT_DIPSETTING(      0x0020, "200K, every 400K" )
	PORT_DIPSETTING(      0x0010, "200K" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_START("IN3") //$40000e
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                            Toride II Adauchi Gaiden
***************************************************************************/

/* I don't really know HOW to describe the effect of IN2 bit 10.
   All I can tell is that is that it affects the levels which are
   proposed, but there is no evidence that one "table" is harder
   than another. */
static INPUT_PORTS_START( toride2g )
	PORT_START("IN0") //$800000
	COINS_SOUND

	PORT_START("IN1") //$800002
	JOY_LSB(1, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)      // BUTTON2 and BUTTON3 in "test mode" only
	JOY_MSB(2, BUTTON1, UNKNOWN, UNKNOWN, UNKNOWN)      // BUTTON2 and BUTTON3 in "test mode" only

	PORT_START("DSW0") //$800004
	COINAGE_SERVICE_LOC(SW1)

	PORT_DIPNAME( 0x0300, 0x0300, "Timer Speed" )           PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, "Slower" )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, "Fast" )
	PORT_DIPSETTING(      0x0000, "Fastest" )
	PORT_DIPNAME( 0x0400, 0x0400, "Tile Arrangement" )      PORT_DIPLOCATION("SW2:3") /* As listed by the manual */
	PORT_DIPSETTING(      0x0400, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0800, 0x0000, "Retry Level On Continue" )   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, "Ask Player" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1000, 0x1000, "2 Players Game" )        PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, "2 Credits" )
	PORT_DIPSETTING(      0x0000, "1 Credit" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )

	PORT_START("IN2") //$800006
	PORT_BIT(  0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN ) // BIT 6 !?
INPUT_PORTS_END

/***************************************************************************
                                Varia Metal
***************************************************************************/

/* verified from M68000 code */
static INPUT_PORTS_START( vmetal )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_TILT )             /* 'Tilt' only in "test mode" - no effect ingame */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )         /* same coinage as COIN1 and COIN2 */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE2 )         /* 'Test' only in "test mode" - no effect ingame */
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")
	// DSW1, stored at 0xff0085.b (cpl'ed)
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C )  )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C )  )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C )  )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_2C )  )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_3C )  )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_4C )  )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_5C )  )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C )  )
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW1:4" ) /* 0x01 (OFF) or 0x02 (ON) written to 0xff0112.b but NEVER read back - old credits for 2 players game ? */
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:5")  /* 0x07c1 written to 0x1788ac.w (screen control ?) at first (code at 0x0001b8) */
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )          /* 0x07c1 written to 0xff0114.w (then 0x1788ac.w) during initialisation (code at 0x000436) */
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )           /* 0x07c0 written to 0xff0114.w (then 0x1788ac.w) during initialisation (code at 0x000436) */
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("IN2")
	// DSW2, stored at 0xff0084.b (cpl'ed)
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, "1"  )
	PORT_DIPSETTING(      0x0004, "2"  )
	PORT_DIPSETTING(      0x000c, "3"  )
	PORT_DIPSETTING(      0x0000, "4"  )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:5")   /* code at 0x0004a4 */
	PORT_DIPSETTING(      0x0010, "Every 30000" )
	PORT_DIPSETTING(      0x0000, "Every 60000" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW2:7" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )
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
static GFXLAYOUT_RAW( layout_8x8x8, 8, 8, 8*8, 32*8 )

/* 16x16x4 tiles for later games */
static const gfx_layout layout_16x16x4 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 4*1,4*0, 4*3,4*2, 4*5,4*4, 4*7,4*6, 4*9,4*8, 4*11,4*10, 4*13,4*12, 4*15,4*14 },
	{ STEP16(0,4*16) },
	4*8*8
};

/* 16x16x8 tiles for later games */
static GFXLAYOUT_RAW( layout_16x16x8, 16, 16, 16*8, 32*8 )

static const gfx_layout layout_053936 =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*8*8, 1*8*8, 2*8*8, 3*8*8, 4*8*8, 5*8*8, 6*8*8, 7*8*8 },
	8*8*8
};

static const gfx_layout layout_053936_16 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	8*8*8+0*8, 8*8*8+1*8, 8*8*8+2*8, 8*8*8+3*8, 8*8*8+4*8, 8*8*8+5*8, 8*8*8+6*8, 8*8*8+7*8,
	},

	{ 0*8*8, 1*8*8, 2*8*8, 3*8*8, 4*8*8, 5*8*8, 6*8*8, 7*8*8,
	8*8*8*2+0*8*8,  8*8*8*2+1*8*8,  8*8*8*2+2*8*8,  8*8*8*2+3*8*8,  8*8*8*2+4*8*8,  8*8*8*2+5*8*8,  8*8*8*2+6*8*8,  8*8*8*2+7*8*8,
	},
	8*8*8*4
};

static GFXDECODE_START( i4100 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x4,    0x0, 0x100 ) // [0] 4 Bit Tiles
GFXDECODE_END

static GFXDECODE_START( i4220 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x4,    0x0, 0x100 ) // [0] 4 Bit Tiles
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x8,    0x0,  0x10 ) // [1] 8 Bit Tiles
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x4,  0x0, 0x100 ) // [2] 4 Bit Tiles 16x16
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x8,  0x0, 0x100 ) // [3] 8 Bit Tiles 16x16
GFXDECODE_END

static GFXDECODE_START( blzntrnd )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x4,    0x0, 0x100 ) // [0] 4 Bit Tiles
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x8,    0x0,  0x10 ) // [1] 8 Bit Tiles
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x4,  0x0, 0x100 ) // [2] 4 Bit Tiles 16x16
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x8,  0x0, 0x100 ) // [3] 8 Bit Tiles 16x16
	GFXDECODE_ENTRY( "gfx2", 0, layout_053936,   0x0,  0x10 ) // [4] 053936 Tiles
GFXDECODE_END

static GFXDECODE_START( gstrik2 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x4,    0x0, 0x100 ) // [0] 4 Bit Tiles
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x8,    0x0,  0x10 ) // [1] 8 Bit Tiles
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x4,  0x0, 0x100 ) // [2] 4 Bit Tiles 16x16
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x8,  0x0, 0x100 ) // [3] 8 Bit Tiles 16x16
	GFXDECODE_ENTRY( "gfx2", 0, layout_053936_16,0x0,  0x10 ) // [4] 053936 Tiles
GFXDECODE_END

// same as 14220:
static GFXDECODE_START( i4300 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x4,    0x0, 0x100 ) // [0] 4 Bit Tiles
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x8,    0x0,  0x10 ) // [1] 8 Bit Tiles
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x4,  0x0, 0x100 ) // [2] 4 Bit Tiles 16x16
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x8,  0x0, 0x100 ) // [3] 8 Bit Tiles 16x16
GFXDECODE_END


/***************************************************************************


                                Machine Drivers


***************************************************************************/

void metro_state::machine_start()
{
	save_item(NAME(m_blitter_bit));
	save_item(NAME(m_irq_line));
	save_item(NAME(m_requested_int));
	save_item(NAME(m_soundstatus));
	save_item(NAME(m_porta));
	save_item(NAME(m_portb));
	save_item(NAME(m_busy_sndcpu));
	save_item(NAME(m_gakusai_oki_bank_lo));
	save_item(NAME(m_gakusai_oki_bank_hi));
	save_item(NAME(m_sprite_xoffs));
	save_item(NAME(m_sprite_yoffs));
}



static MACHINE_CONFIG_START( msgogo, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(msgogo_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  metro_vblank_interrupt) // timing is off, shaking sprites in intro
	MCFG_CPU_PERIODIC_INT_DRIVER(metro_state, metro_periodic_interrupt,  60) // ?

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 224)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i4220)
	MCFG_VIDEO_START_OVERRIDE(metro_state,metro_i4220_dx_tmap)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymf", YMF278B, YMF278B_STD_CLOCK)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, ymf278_map)
	MCFG_YMF278B_IRQ_HANDLER(INPUTLINE("maincpu", 2))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( balcube, msgogo )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(balcube_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  metro_vblank_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(metro_state, metro_periodic_interrupt,  8*60) // ?
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( daitoa, msgogo )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(daitoa_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  metro_vblank_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(metro_state, metro_periodic_interrupt,  8*60) // ?
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bangball, msgogo )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(bangball_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  metro_vblank_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(metro_state, metro_periodic_interrupt,  60) // ?
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( batlbubl, msgogo )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(batlbubl_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  metro_vblank_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(metro_state, metro_periodic_interrupt,  60) // ?
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( daitorid, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_32MHz/2)
	MCFG_CPU_PROGRAM_MAP(daitorid_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  metro_vblank_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(metro_state, metro_periodic_interrupt,  8*60) // ?

	MCFG_CPU_ADD("audiocpu", UPD7810, XTAL_12MHz)
	MCFG_UPD7810_RXD(READLINE(metro_state,metro_rxd_r))
	MCFG_CPU_PROGRAM_MAP(metro_sound_map)
	MCFG_CPU_IO_MAP(daitorid_sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 224)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i4220)
	MCFG_VIDEO_START_OVERRIDE(metro_state,metro_i4220_dx_tmap)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", XTAL_3_579545MHz)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", UPD7810_INTF2))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.80)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.80)

	MCFG_OKIM6295_ADD("oki", 1200000, OKIM6295_PIN7_HIGH) // sample rate =  M6295 clock / 132
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.40)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.40)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( dharma, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)
	MCFG_CPU_PROGRAM_MAP(dharma_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  metro_vblank_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(metro_state, metro_periodic_interrupt,  8*60) // ?

	MCFG_CPU_ADD("audiocpu", UPD7810, XTAL_24MHz/2)
	MCFG_UPD7810_RXD(READLINE(metro_state,metro_rxd_r))
	MCFG_CPU_PROGRAM_MAP(metro_sound_map)
	MCFG_CPU_IO_MAP(metro_sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 224)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i4220)
	MCFG_VIDEO_START_OVERRIDE(metro_state,metro_i4220)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", XTAL_24MHz/20, OKIM6295_PIN7_HIGH) // sample rate =  M6295 clock / 132
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.10)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.10)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.90)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.90)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( karatour, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)
	MCFG_CPU_PROGRAM_MAP(karatour_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  karatour_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(metro_state, metro_periodic_interrupt,  8*60) // ?

	MCFG_CPU_ADD("audiocpu", UPD7810, XTAL_24MHz/2)
	MCFG_UPD7810_RXD(READLINE(metro_state,metro_rxd_r))
	MCFG_CPU_PROGRAM_MAP(metro_sound_map)
	MCFG_CPU_IO_MAP(metro_sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i4100)
	MCFG_VIDEO_START_OVERRIDE(metro_state,metro_i4100)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", XTAL_24MHz/20, OKIM6295_PIN7_HIGH) // was /128.. so pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.10)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.10)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.90)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.90)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( 3kokushi, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)
	MCFG_CPU_PROGRAM_MAP(kokushi_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  karatour_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(metro_state, metro_periodic_interrupt,  8*60) // ?

	MCFG_CPU_ADD("audiocpu", UPD7810, XTAL_24MHz/2)
	MCFG_UPD7810_RXD(READLINE(metro_state,metro_rxd_r))
	MCFG_CPU_PROGRAM_MAP(metro_sound_map)
	MCFG_CPU_IO_MAP(metro_sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i4220)
	MCFG_VIDEO_START_OVERRIDE(metro_state,metro_i4220)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", XTAL_24MHz/20, OKIM6295_PIN7_HIGH) // was /128.. so pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.10)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.10)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.90)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.90)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( lastfort, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)
	MCFG_CPU_PROGRAM_MAP(lastfort_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  metro_vblank_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(metro_state, metro_periodic_interrupt,  8*60) // ?

	MCFG_CPU_ADD("audiocpu", UPD7810, XTAL_24MHz/2)
	MCFG_UPD7810_RXD(READLINE(metro_state,metro_rxd_r))
	MCFG_CPU_PROGRAM_MAP(metro_sound_map)
	MCFG_CPU_IO_MAP(metro_sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(360, 224)
	MCFG_SCREEN_VISIBLE_AREA(0, 360-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i4100)
	MCFG_VIDEO_START_OVERRIDE(metro_state,metro_i4100)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", XTAL_24MHz/20, OKIM6295_PIN7_LOW) // sample rate =  M6295 clock / 165
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.10)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.10)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.90)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.90)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( lastforg, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)
	MCFG_CPU_PROGRAM_MAP(lastforg_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  karatour_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(metro_state, metro_periodic_interrupt,  8*60) // ?

	MCFG_CPU_ADD("audiocpu", UPD7810, XTAL_24MHz/2)
	MCFG_UPD7810_RXD(READLINE(metro_state,metro_rxd_r))
	MCFG_CPU_PROGRAM_MAP(metro_sound_map)
	MCFG_CPU_IO_MAP(metro_sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(360, 224)
	MCFG_SCREEN_VISIBLE_AREA(0, 360-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i4100)
	MCFG_VIDEO_START_OVERRIDE(metro_state,metro_i4100)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", XTAL_24MHz/20, OKIM6295_PIN7_HIGH) // was /128.. so pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.10)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.10)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.90)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.90)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( dokyusei, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(dokyusei_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  metro_vblank_interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(metro_state,metro_irq_callback)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 224)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i4300)
	MCFG_VIDEO_START_OVERRIDE(metro_state,metro_i4300)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.90)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.90)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( dokyusp, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_32MHz/2)
	MCFG_CPU_PROGRAM_MAP(dokyusp_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  metro_vblank_interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(metro_state,metro_irq_callback)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(384, 224)
	MCFG_SCREEN_VISIBLE_AREA(0, 384-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i4300)
	MCFG_VIDEO_START_OVERRIDE(metro_state,metro_i4300)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", 2112000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.90)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.90)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( gakusai, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 16000000) /* 26.6660MHz/2?, OSCs listed are 26.6660MHz & 3.579545MHz */
	MCFG_CPU_PROGRAM_MAP(gakusai_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  metro_vblank_interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(metro_state,metro_irq_callback)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i4300)
	MCFG_VIDEO_START_OVERRIDE(metro_state,metro_i4300)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", 2112000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.90)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.90)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( gakusai2, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 16000000) /* 26.6660MHz/2?, OSCs listed are 26.6660MHz & 3.579545MHz */
	MCFG_CPU_PROGRAM_MAP(gakusai2_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  metro_vblank_interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(metro_state,metro_irq_callback)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i4300)
	MCFG_VIDEO_START_OVERRIDE(metro_state,metro_i4300)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", 2112000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.90)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.90)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( pangpoms, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)
	MCFG_CPU_PROGRAM_MAP(pangpoms_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  metro_vblank_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(metro_state, metro_periodic_interrupt,  8*60) // ?

	MCFG_CPU_ADD("audiocpu", UPD7810, XTAL_24MHz/2)
	MCFG_UPD7810_RXD(READLINE(metro_state,metro_rxd_r))
	MCFG_CPU_PROGRAM_MAP(metro_sound_map)
	MCFG_CPU_IO_MAP(metro_sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(360, 224)
	MCFG_SCREEN_VISIBLE_AREA(0, 360-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i4100)
	MCFG_VIDEO_START_OVERRIDE(metro_state,metro_i4100)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", XTAL_24MHz/20, OKIM6295_PIN7_HIGH) // was /128.. so pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.10)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.10)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.90)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.90)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( poitto, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)
	MCFG_CPU_PROGRAM_MAP(poitto_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  metro_vblank_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(metro_state, metro_periodic_interrupt,  8*60) // ?

	MCFG_CPU_ADD("audiocpu", UPD7810, XTAL_24MHz/2)
	MCFG_UPD7810_RXD(READLINE(metro_state,metro_rxd_r))
	MCFG_CPU_PROGRAM_MAP(metro_sound_map)
	MCFG_CPU_IO_MAP(metro_sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(360, 224)
	MCFG_SCREEN_VISIBLE_AREA(0, 360-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i4100)
	MCFG_VIDEO_START_OVERRIDE(metro_state,metro_i4100)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", XTAL_24MHz/20, OKIM6295_PIN7_HIGH) // was /128.. so pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.10)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.10)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.90)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.90)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( pururun, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)       /* Not confirmed */
	MCFG_CPU_PROGRAM_MAP(pururun_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  metro_vblank_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(metro_state, metro_periodic_interrupt,  8*60) // ?

	MCFG_CPU_ADD("audiocpu", UPD7810, XTAL_24MHz/2)     /* Not confiremd */
	MCFG_UPD7810_RXD(READLINE(metro_state,metro_rxd_r))
	MCFG_CPU_PROGRAM_MAP(metro_sound_map)
	MCFG_CPU_IO_MAP(daitorid_sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 224)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i4220)
	MCFG_VIDEO_START_OVERRIDE(metro_state,metro_i4220)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", XTAL_3_579545MHz)  /* Confirmed match to reference video */
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", UPD7810_INTF2))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.80)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.80)

	MCFG_OKIM6295_ADD("oki", XTAL_3_579545MHz/3, OKIM6295_PIN7_HIGH) // sample rate =  M6295 clock / 132
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.40)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.40)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( skyalert, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)
	MCFG_CPU_PROGRAM_MAP(skyalert_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  metro_vblank_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(metro_state, metro_periodic_interrupt,  8*60) // ?

	MCFG_CPU_ADD("audiocpu", UPD7810, XTAL_24MHz/2)
	MCFG_UPD7810_RXD(READLINE(metro_state,metro_rxd_r))
	MCFG_CPU_PROGRAM_MAP(metro_sound_map)
	MCFG_CPU_IO_MAP(metro_sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(360, 224)
	MCFG_SCREEN_VISIBLE_AREA(0, 360-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i4100)
	MCFG_VIDEO_START_OVERRIDE(metro_state,metro_i4100)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", XTAL_24MHz/20, OKIM6295_PIN7_LOW) // sample rate =  M6295 clock / 165
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.10)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.10)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.90)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.90)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( toride2g, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)
	MCFG_CPU_PROGRAM_MAP(toride2g_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  metro_vblank_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(metro_state, metro_periodic_interrupt,  8*60) // ?

	MCFG_CPU_ADD("audiocpu", UPD7810, XTAL_24MHz/2)
	MCFG_UPD7810_RXD(READLINE(metro_state,metro_rxd_r))
	MCFG_CPU_PROGRAM_MAP(metro_sound_map)
	MCFG_CPU_IO_MAP(metro_sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 224)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i4220)
	MCFG_VIDEO_START_OVERRIDE(metro_state,metro_i4220)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", XTAL_24MHz/20, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.10)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.10)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.90)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.90)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( mouja, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(mouja_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  metro_vblank_interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(metro_state,metro_irq_callback)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i4300)
	MCFG_VIDEO_START_OVERRIDE(metro_state,metro_i4300)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_OKIM6295_ADD("oki", XTAL_16MHz/1024*132, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_DEVICE_ADDRESS_MAP(AS_0, mouja_okimap)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.25)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.00)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( vmetal, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(vmetal_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  metro_vblank_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(metro_state, metro_periodic_interrupt,  8*60) // ?

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58.2328) // VSync 58.2328Hz, HSync 15.32kHz
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 224)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i4220)
	MCFG_VIDEO_START_OVERRIDE(metro_state,metro_i4220_dx_sprite)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", XTAL_1MHz, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.75)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.75)

	MCFG_ES8712_ADD("essnd", 12000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	// OKI M6585 not hooked up...
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( blzntrnd, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(blzntrnd_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  karatour_interrupt)
	MCFG_CPU_PERIODIC_INT_DRIVER(metro_state, metro_periodic_interrupt,  8*60) // ?

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_16MHz/2)
	MCFG_CPU_PROGRAM_MAP(blzntrnd_sound_map)
	MCFG_CPU_IO_MAP(blzntrnd_sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 224)
	MCFG_SCREEN_VISIBLE_AREA(8, 320-8-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", blzntrnd)
	MCFG_VIDEO_START_OVERRIDE(metro_state,blzntrnd)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	MCFG_DEVICE_ADD("k053936", K053936, 0)
	MCFG_K053936_OFFSETS(-69, -21)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2610, XTAL_16MHz/2)
	MCFG_YM2610_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "lspeaker",  0.25)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(1, "lspeaker",  1.0)
	MCFG_SOUND_ROUTE(2, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gstrik2, blzntrnd )
	MCFG_GFXDECODE_MODIFY("gfxdecode", gstrik2)
	MCFG_VIDEO_START_OVERRIDE(metro_state,gstrik2)

	MCFG_DEVICE_MODIFY("k053936")
	MCFG_K053936_OFFSETS(-69, -19)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( puzzlet, metro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", H83007, XTAL_20MHz) // H8/3007 - Hitachi HD6413007F20 CPU. Clock 20MHz
	MCFG_CPU_PROGRAM_MAP(puzzlet_map)
	MCFG_CPU_IO_MAP(puzzlet_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", metro_state,  puzzlet_interrupt)

	/* Coins/service */
	MCFG_PUZZLET_IO_ADD("coins")
	MCFG_PUZZLET_IO_DATA_CALLBACK(DEVWRITELINE("maincpu:sci1", h8_sci_device, rx_w))
	MCFG_DEVICE_MODIFY("maincpu:sci1")
	MCFG_H8_SCI_TX_CALLBACK(DEVWRITELINE(":coins", puzzlet_io_device, ce_w))
	MCFG_H8_SCI_CLK_CALLBACK(DEVWRITELINE(":coins", puzzlet_io_device, clk_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 224)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(metro_state, screen_update_metro)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", i4300)
	MCFG_VIDEO_START_OVERRIDE(metro_state,metro_i4300)
	MCFG_PALETTE_ADD("palette", 0x1000)
	MCFG_PALETTE_FORMAT(GGGGGRRRRRBBBBBx)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki", XTAL_20MHz/5, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_20MHz/5)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.90)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.90)
MACHINE_CONFIG_END


/***************************************************************************

                                ROMs Loading

***************************************************************************/

/***************************************************************************

Bal Cube
Metro 1996

+--------------------------------------------+
|               BAL-CUBE_07      BAL-CUBE_01 |
|                 YRW801-M       BAL-CUBE_02 |
|                                BAL-CUBE_03 |
|J      33.369MHz YMF278B        BAL-CUBE_04 |
|A                                           |
|M                                           |
|M                                           |
|A           ALTERA 16MHz     Imagetek       |
|     BAL-CUBE_06 BAL-CUBE_05 I4220          |
|SW1      CY7C199 CY7C199                    |
|SW2        68000-16          CY7C199 61C64  |
|SW3                26.666MHz CY7C199        |
+--------------------------------------------+

CPU  : TMP68HC000P-16
Sound: YAMAHA OPL YMF278B-F + YRW801-M
OSC  : 16.0000MHz (OSC1) 26.6660MHz (OSC2) 33.869MHz (OSC3)
PLD  : ALTERA EPM7032LC44-15T
Video: Imagetek I4220

SW3 - Not Populated
***************************************************************************/

ROM_START( balcube )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "bal-cube_06.6", 0x000000, 0x040000, CRC(c400f84d) SHA1(416eb82ec1201d24d9d964191a5a1792c9445923) ) /* Silkscreened 6 and U18 */
	ROM_LOAD16_BYTE( "bal-cube_05.5", 0x000001, 0x040000, CRC(15313e3f) SHA1(10a8702016f223194dc91875b4736253fd47dbb8) ) /* Silkscreened 5 and U19 */

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "bal-cube_02.2", 0x000000, 0x080000, CRC(492ca8f0) SHA1(478336a462a2bfc288cf91262314f5767f8c707d) , ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened 2 and U30 */
	ROMX_LOAD( "bal-cube_04.4", 0x000002, 0x080000, CRC(d1acda2c) SHA1(f58015302af6c864523d48bdf8f8a4383b69fa9d) , ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened 4 and U29 */
	ROMX_LOAD( "bal-cube_01.1", 0x000004, 0x080000, CRC(0ea3d161) SHA1(63ae430a19e777ce82b41ab02baef3bb224c7557) , ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened 1 and U28 */
	ROMX_LOAD( "bal-cube_03.3", 0x000006, 0x080000, CRC(eef1d3b4) SHA1(be535963c00390e34a2305586397a16325f3c3c0) , ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened 3 and U27 */

	ROM_REGION( 0x280000, "ymf", 0 )
	ROM_LOAD( "yrw801-m",      0x000000, 0x200000, CRC(2a9d8d43) SHA1(32760893ce06dbe3930627755ba065cc3d8ec6ca) ) /* Silkscreened U52 */        // Yamaha YRW801 2MB ROM with samples for the OPL4.
	ROM_LOAD( "bal-cube_07.7", 0x200000, 0x080000, CRC(f769287d) SHA1(dd0f781b4a1a1fd6bf0a50048b4996f3cf41e155) ) /* Silkscreened 7 and U49 */  // PCM 16 Bit (Signed)
ROM_END


/***************************************************************************

Bang Bang Ball
(c)1996 Banpresto/Kunihiko Tashiro/Goodhouse

+--------------------------------------------+
|                 rom#007         BP963A_U28 |
|                 YRW801-M        BP963A_U30 |
|                                 BP963A_U27 |
|J      33.369MHz YMF278B         BP963A_U29 |
|A                                           |
|M                                           |
|M                                           |
|A           ALTERA 16MHz     Imagetek       |
|         rom#006 rom#005     I4220          |
|SW1      CY7C199 CY7C199                    |
|SW2        68000-16          CY7C199 61C64  |
|SW3                26.666MHz CY7C199        |
+--------------------------------------------+

CPU  : TMP68HC000P-16
Sound: YAMAHA OPL YMF278B-F + YRW801-M
OSC  : 16.0000MHz (OSC1) 26.6660MHz (OSC2) 33.869MHz (OSC3)
PLD  : ALTERA EPM7032LC44-15T D9522
Video: Imagetek I4220 071 9403EK701

SW3 - Not Populated

ROMs:
B-BALL/J rom #005.u19 - Main programs (27c020)
B-BALL/J rom #006.u18 /

B-BALL/J rom #007.u49 - Sound samples (27c040)
yrw801-m.u52          - Yamaha wave data ROM (44pin SOP 16M mask (LH537019))

BP963A U27 - Graphics (MASK, read as 27c800)
BP963A U28 |
BP963A U29 |
BP963A U30 /

**********************************

Battle Bubble
(c)1999 Limenko

  Listed on Limenko's Web site as kit LM2DY00

PCB -
 REV: LM2D-Y
 SEL: 00-200-004

Same basic componets as those listed for Bang Bang Ball, except
PCB uses a Xlinix XC9536 istead of the Altera EMP7032LC44 PLD.

Did Limenko license this or bootleg it?  The board doesn't look like a
bootleg and has all original parts on it..

Limenko's web site states:

 1998  6 Developed LM2D-Y00-LM
      10 Contract the technology and products in cooperation with Metro Ltd.
 1999 11 Begin to sell Battle Bubble internally
      12 Received an overseas order for Battle Bubble

***************************************************************************/

ROM_START( bangball )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "b-ball_j_rom#006.u18", 0x000000, 0x040000, CRC(0e4124bc) SHA1(f5cd762df4e822ab5c8dba6f276b3366895235d1) ) /* Silkscreened 6 and U18 */
	ROM_LOAD16_BYTE( "b-ball_j_rom#005.u19", 0x000001, 0x040000, CRC(3fa08587) SHA1(8fdafdde5e77d077b5cd8f94f97b5430fe062936) ) /* Silkscreened 5 and U19 */

	ROM_REGION( 0x400000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "bp963a_u30.u30", 0x000000, 0x100000, CRC(b0ca8e39) SHA1(f2eb1d07cd10050c234f0b418146c742b496f196) , ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened 2 and U30 */
	ROMX_LOAD( "bp963a_u29.u29", 0x000002, 0x100000, CRC(d934468f) SHA1(b93353bf2302b68a297d71fc9d91dc55c1cccce4) , ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened 4 and U29 */
	ROMX_LOAD( "bp963a_u28.u28", 0x000004, 0x100000, CRC(96d03c6a) SHA1(6257585721291e5a5ce311c2873c9e1e1dac2fc6) , ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened 1 and U28 */
	ROMX_LOAD( "bp963a_u27.u27", 0x000006, 0x100000, CRC(5e3c7732) SHA1(e8c442a8038921ae3de48ce52923d25cb97e36ea) , ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened 3 and U27 */

	ROM_REGION( 0x280000, "ymf", 0 )
	ROM_LOAD( "yrw801-m",             0x000000, 0x200000, CRC(2a9d8d43) SHA1(32760893ce06dbe3930627755ba065cc3d8ec6ca) ) /* Silkscreened U52 */
	ROM_LOAD( "b-ball_j_rom#007.u49", 0x200000, 0x080000, CRC(04cc91a9) SHA1(e5cf6055a0803f4ad44919090cd147702e805d88) ) /* Silkscreened 7 and U49 */
ROM_END

ROM_START( batlbubl )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "lm-01.u11", 0x000000, 0x080000, CRC(1d562807) SHA1(3e5dbe6f4b04aa9e01b7b8938d0b46d4862054bf) )
	ROM_LOAD16_WORD_SWAP( "lm-02.u12", 0x080000, 0x080000, CRC(852e4750) SHA1(d8b703ba65d0f267eba07f160b13dbe0f5ac40c2) )

	ROM_REGION( 0x800000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "lm-07.u30", 0x000000, 0x200000, CRC(03d9dfd8) SHA1(33c96f1b0fa28c6e46b2d2c0a62dfe0306139e09) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "lm-06.u29", 0x000002, 0x200000, CRC(5efb905b) SHA1(c3f5d781941225c17d37473e2e0ed84875cebace) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "lm-05.u28", 0x000004, 0x200000, CRC(e53ba59f) SHA1(d82749c04d776fbf9e5cc44a23d2bfafe073fafa) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "lm-04.u27", 0x000006, 0x200000, CRC(2e687cfb) SHA1(4766ddc882c3e330e948b64e4e44a08846bf2046) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x280000, "ymf", 0 )
	ROM_LOAD( "lm-08.u40", 0x000000, 0x200000, CRC(2a9d8d43) SHA1(32760893ce06dbe3930627755ba065cc3d8ec6ca) ) /* PCB labeled YRM801, Sticker says lm-08 */
	ROM_LOAD( "lm-03.u42", 0x200000, 0x080000, CRC(04cc91a9) SHA1(e5cf6055a0803f4ad44919090cd147702e805d88) )
ROM_END

/***************************************************************************

Blazing Tornado
(c)1994 Human

CPU:    68000-16
Sound:  Z80-8
    YMF286K (YM2610 compatible)
OSC:    16.0000MHz
    26.666MHz
Chips:  Imagetek I4220 071
    Konami 053936 (PSAC2)

***************************************************************************/

ROM_START( blzntrnd )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 68000 */
	ROM_LOAD16_BYTE( "1k.bin", 0x000000, 0x80000, CRC(b007893b) SHA1(609363449c0218b8a38de72d37c66e6f3bb4f8cd) )
	ROM_LOAD16_BYTE( "2k.bin", 0x000001, 0x80000, CRC(ec173252) SHA1(652d70055d2799442beede1ae68e54551931068f) )
	ROM_LOAD16_BYTE( "3k.bin", 0x100000, 0x80000, CRC(1e230ba2) SHA1(ca96c82d57a6b5bacc1bfd2f7965503c2a6e162f) )
	ROM_LOAD16_BYTE( "4k.bin", 0x100001, 0x80000, CRC(e98ca99e) SHA1(9346fc0d419add23eaceb5843c505f3ffa69e495) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 */
	ROM_LOAD( "rom5.bin", 0x0000, 0x20000, CRC(7e90b774) SHA1(abd0eda9eababa1f7ab17a2f60534dcebda33c9c) )

	ROM_REGION( 0x1800000, "gfx1", 0 )  /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "rom142.bin", 0x0000000, 0x200000, CRC(a7200598) SHA1(f8168a94abc380308901303a69cbd15097019797) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "rom186.bin", 0x0000002, 0x200000, CRC(6ee28ea7) SHA1(b33bcbf16423999135d96a62bf25c6ff23031f2a) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "rom131.bin", 0x0000004, 0x200000, CRC(c77e75d3) SHA1(8ad716d4e37d6efe478a8e49feb4e68283310890) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "rom175.bin", 0x0000006, 0x200000, CRC(04a84f9b) SHA1(83aabbc1c7ab06b351168153335f3c2f91fba0e9) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "rom242.bin", 0x0800000, 0x200000, CRC(1182463f) SHA1(6fa2a0b3186a3542b43926e3f37714b78a890542) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "rom286.bin", 0x0800002, 0x200000, CRC(384424fc) SHA1(f89d43756bd38515a223fe4ffbed3a44c673ae28) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "rom231.bin", 0x0800004, 0x200000, CRC(f0812362) SHA1(9f8be51f60f7baf72f9de8352e4e13d730f85903) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "rom275.bin", 0x0800006, 0x200000, CRC(184cb129) SHA1(8ffb3cdc7e0d227b6f0a7962bc6d853c6b84c8d2) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "rom342.bin", 0x1000000, 0x200000, CRC(e527fee5) SHA1(e5de1e134d95aa7a48695183189924061482e3a3) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "rom386.bin", 0x1000002, 0x200000, CRC(d10b1401) SHA1(0eb75a283000a8b19a14177461b6f335c9d9dec2) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "rom331.bin", 0x1000004, 0x200000, CRC(4d909c28) SHA1(fb9bb824e518f67713799ed2c0159a7bd70f35c4) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "rom375.bin", 0x1000006, 0x200000, CRC(6eb4f97c) SHA1(c7f006230cbf10e706b0362eeed34655a3aef1a5) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x200000, "gfx2", 0 )   /* 053936 gfx data */
	ROM_LOAD( "rom9.bin", 0x000000, 0x200000, CRC(37ca3570) SHA1(3374c586bf84583fa33f2793c4e8f2f61a0cab1c) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )   /* Samples */
	ROM_LOAD( "rom8.bin", 0x000000, 0x080000, CRC(565a4086) SHA1(bd5780acfa5affa8705acbfccb0af16bac8ed298) )

	ROM_REGION( 0x400000, "ymsnd", 0 )  /* ? YRW801-M ? */
	ROM_LOAD( "rom6.bin", 0x000000, 0x200000, CRC(8b8819fc) SHA1(5fd9d2b5088cb676c11d32cac7ba8c5c18e31b64) )
	ROM_LOAD( "rom7.bin", 0x200000, 0x200000, CRC(0089a52b) SHA1(d643ac122d62557de27f06ba1413ef757a45a927) )
ROM_END

/*

Grand Striker 2
Human Entertainment, 1996

PCB Layout
----------

HUM-003(A)
|-----------------------------------------------------------------------|
|           YM3016 ROM8.22  ROM342.88  ROM386.87  ROM331.86  ROM375.85  |
|                                                                       |
| 6264  YM2610         ROM142.80  ROM186.79  ROM131.78  ROM175.77       |
|                                                                       |
|                  ROM7.27  ROM442.92  ROM486.91  ROM431.90  ROM475.89  |
|                                                                       |
|          PAL         ROM242.84  ROM286.83  ROM231.82  ROM275.81       |
|  SPRG.30                                                              |
|  PAL     Z80     ROM6.23                                              |
|                                                                       |
|J                                                                      |
|A                                                                      |
|M                                               |--------|             |
|M                   PRG2  PRG3                  |IMAGETEK|   6264      |
|A                                               |I4220   |             |
|                    PRG0  PRG1                  |--------|             |
|     16MHz  68000   62256  62256    26.666MHz                          |
|                                                                       |
|     DSW1                                                              |
|     DSW2   EPM7032         |------|  62256  62256                     |
|     DSW3            6116   |053936|  62256  62256                     |
|     DSW4            6116   |PSAC2 |                   PAL             |
|                            |------|                          ROM9.60  |
|-----------------------------------------------------------------------|

Notes:
       68000 clock: 16.000MHz
         Z80 clock: 8.000MHz
      YM2610 clock: 8.000MHz
             VSync: 58Hz
             HSync: 15.11kHz

*/


/* The MASK roms weren't dumped from this set, but it's safe to assume they're the same in this case */
ROM_START( gstrik2 )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 68000 */
	ROM_LOAD16_BYTE( "hum_003_g2f.rom1.u107", 0x000000, 0x80000, CRC(2712d9ca) SHA1(efa967de931728534a663fa1529e92003afbb3e9) )
	ROM_LOAD16_BYTE( "hum_003_g2f.rom2.u108", 0x000001, 0x80000, CRC(86785c64) SHA1(ef172d6e859a68eb80f7c127b61883d50eefb0fe) )
	ROM_LOAD16_BYTE( "prg2.109", 0x100000, 0x80000, CRC(ead86919) SHA1(eb9b68dff4e08d90ac90043c7f3021914caa007d) )
	ROM_LOAD16_BYTE( "prg3.110", 0x100001, 0x80000, CRC(e0b026e3) SHA1(05f75c0432efda3dec0372199382e310bb268fba) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 */
	ROM_LOAD( "sprg.30", 0x0000, 0x20000, CRC(aeef6045) SHA1(61b8c89ca495d3aac79e53413a85dd203db816f3) )

	ROM_REGION( 0x1000000, "gfx1", 0 )  /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "chr0.80", 0x0000000, 0x200000, CRC(f63a52a9) SHA1(1ad52bb3a051eaffe8fb6ba49d4fc1d0b6144156) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "chr1.79", 0x0000002, 0x200000, CRC(4110c184) SHA1(90ccb3d50eff7a655336cfa9c072f7213589e64c) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "chr2.78", 0x0000004, 0x200000, CRC(ddb4b9ee) SHA1(0e2c151c3690b9c3d298dda8842e283660d37386) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "chr3.77", 0x0000006, 0x200000, CRC(5ab367db) SHA1(adf8749451f4583f8e9e00ab61f3408d804a7265) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "chr4.84", 0x0800000, 0x200000, CRC(77d7ef99) SHA1(8f5cf72f5919fe9363e7549e0bb1b3ee633cec3b) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "chr5.83", 0x0800002, 0x200000, CRC(a4d49e95) SHA1(9789bacba7876100e0f0293f54c81def545ed068) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "chr6.82", 0x0800004, 0x200000, CRC(32eb33b0) SHA1(2ea06484ca326b44a35ee470343147a9d91d5626) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "chr7.81", 0x0800006, 0x200000, CRC(2d30a21e) SHA1(749e86b7935ef71556eaee4caf6f954634e9bcbf) , ROM_GROUPWORD | ROM_SKIP(6))
	/* not populated */
//  ROMX_LOAD( "chr8.88", 0x1000000, 0x200000, CRC() SHA1() , ROM_GROUPWORD | ROM_SKIP(6))
//  ROMX_LOAD( "chr9.87", 0x1000002, 0x200000, CRC() SHA1() , ROM_GROUPWORD | ROM_SKIP(6))
//  ROMX_LOAD( "chr10.86", 0x1000004, 0x200000, CRC() SHA1() , ROM_GROUPWORD | ROM_SKIP(6))
//  ROMX_LOAD( "chr11.85", 0x1000006, 0x200000, CRC() SHA1() , ROM_GROUPWORD | ROM_SKIP(6))
//  ROMX_LOAD( "chr12.92", 0x1800000, 0x200000, CRC() SHA1() , ROM_GROUPWORD | ROM_SKIP(6))
//  ROMX_LOAD( "chr13.91", 0x1800002, 0x200000, CRC() SHA1() , ROM_GROUPWORD | ROM_SKIP(6))
//  ROMX_LOAD( "chr14.90", 0x1800004, 0x200000, CRC() SHA1() , ROM_GROUPWORD | ROM_SKIP(6))
//  ROMX_LOAD( "chr15.89", 0x1800006, 0x200000, CRC() SHA1() , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x200000, "gfx2", 0 )   /* 053936 gfx data */
	ROM_LOAD( "psacrom.60", 0x000000, 0x200000,  CRC(73f1f279) SHA1(1135b2b1eb4c52249bc12ee178340bbb202a94c8) )

	ROM_REGION( 0x200000, "ymsnd.deltat", 0 )   /* Samples */
	ROM_LOAD( "sndpcm-b.22", 0x000000, 0x200000, CRC(a5d844d2) SHA1(18d644545f0844e66aa53775b67b0a29c7b7c31b) )

	ROM_REGION( 0x400000, "ymsnd", 0 )  /* ? YRW801-M ? */
	ROM_LOAD( "sndpcm-a.23", 0x000000, 0x200000, CRC(e6d32373) SHA1(8a79d4ea8b27d785fffd80e38d5ae73b7cea7304) )
	/* ROM7.27 not populated?  */
ROM_END

ROM_START( gstrik2j )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 68000 */
	ROM_LOAD16_BYTE( "prg0.107", 0x000000, 0x80000, CRC(e60a8c19) SHA1(19be6cfcb60ede6fd4eb2e14914b174107c4b52d) )
	ROM_LOAD16_BYTE( "prg1.108", 0x000001, 0x80000, CRC(853f6f7c) SHA1(8fb9d7cd0390f620560a1669bb13f2033eed7c81) )
	ROM_LOAD16_BYTE( "prg2.109", 0x100000, 0x80000, CRC(ead86919) SHA1(eb9b68dff4e08d90ac90043c7f3021914caa007d) )
	ROM_LOAD16_BYTE( "prg3.110", 0x100001, 0x80000, CRC(e0b026e3) SHA1(05f75c0432efda3dec0372199382e310bb268fba) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 */
	ROM_LOAD( "sprg.30", 0x0000, 0x20000, CRC(aeef6045) SHA1(61b8c89ca495d3aac79e53413a85dd203db816f3) )

	ROM_REGION( 0x1000000, "gfx1", 0 )  /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "chr0.80", 0x0000000, 0x200000, CRC(f63a52a9) SHA1(1ad52bb3a051eaffe8fb6ba49d4fc1d0b6144156) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "chr1.79", 0x0000002, 0x200000, CRC(4110c184) SHA1(90ccb3d50eff7a655336cfa9c072f7213589e64c) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "chr2.78", 0x0000004, 0x200000, CRC(ddb4b9ee) SHA1(0e2c151c3690b9c3d298dda8842e283660d37386) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "chr3.77", 0x0000006, 0x200000, CRC(5ab367db) SHA1(adf8749451f4583f8e9e00ab61f3408d804a7265) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "chr4.84", 0x0800000, 0x200000, CRC(77d7ef99) SHA1(8f5cf72f5919fe9363e7549e0bb1b3ee633cec3b) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "chr5.83", 0x0800002, 0x200000, CRC(a4d49e95) SHA1(9789bacba7876100e0f0293f54c81def545ed068) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "chr6.82", 0x0800004, 0x200000, CRC(32eb33b0) SHA1(2ea06484ca326b44a35ee470343147a9d91d5626) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "chr7.81", 0x0800006, 0x200000, CRC(2d30a21e) SHA1(749e86b7935ef71556eaee4caf6f954634e9bcbf) , ROM_GROUPWORD | ROM_SKIP(6))
	/* not populated */
//  ROMX_LOAD( "chr8.88", 0x1000000, 0x200000, CRC() SHA1() , ROM_GROUPWORD | ROM_SKIP(6))
//  ROMX_LOAD( "chr9.87", 0x1000002, 0x200000, CRC() SHA1() , ROM_GROUPWORD | ROM_SKIP(6))
//  ROMX_LOAD( "chr10.86", 0x1000004, 0x200000, CRC() SHA1() , ROM_GROUPWORD | ROM_SKIP(6))
//  ROMX_LOAD( "chr11.85", 0x1000006, 0x200000, CRC() SHA1() , ROM_GROUPWORD | ROM_SKIP(6))
//  ROMX_LOAD( "chr12.92", 0x1800000, 0x200000, CRC() SHA1() , ROM_GROUPWORD | ROM_SKIP(6))
//  ROMX_LOAD( "chr13.91", 0x1800002, 0x200000, CRC() SHA1() , ROM_GROUPWORD | ROM_SKIP(6))
//  ROMX_LOAD( "chr14.90", 0x1800004, 0x200000, CRC() SHA1() , ROM_GROUPWORD | ROM_SKIP(6))
//  ROMX_LOAD( "chr15.89", 0x1800006, 0x200000, CRC() SHA1() , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x200000, "gfx2", 0 )   /* 053936 gfx data */
	ROM_LOAD( "psacrom.60", 0x000000, 0x200000,  CRC(73f1f279) SHA1(1135b2b1eb4c52249bc12ee178340bbb202a94c8) )

	ROM_REGION( 0x200000, "ymsnd.deltat", 0 )   /* Samples */
	ROM_LOAD( "sndpcm-b.22", 0x000000, 0x200000, CRC(a5d844d2) SHA1(18d644545f0844e66aa53775b67b0a29c7b7c31b) )

	ROM_REGION( 0x400000, "ymsnd", 0 )  /* ? YRW801-M ? */
	ROM_LOAD( "sndpcm-a.23", 0x000000, 0x200000, CRC(e6d32373) SHA1(8a79d4ea8b27d785fffd80e38d5ae73b7cea7304) )
	/* ROM7.27 not populated?  */
ROM_END


/***************************************************************************

Daitoride
Metro 1995

MTR5260-A

                      3.5759MHz  12MHz  6116
   26.666MHz        YM2151          DT7  DT8
                            M6295
     7C199                             78C10
     7C199       Imagetek I4220
     61C64

                  68000-16             DT1
                  32MHz    52258       DT2
   SW1                     52258       DT3
   SW2            DT6  DT5             DT4

********************************************************

Daitoride (YMF278B version)
Metro 1996

+--------------------------------------------+
|                 DT_JA-7            DT_JA-1 |
|                 YRW801-M           DT_JA-2 |
|                                    DT_JA-3 |
|J      33.369MHz YMF278B            DT_JA-4 |
|A                                           |
|M                                           |
|M                                           |
|A           ALTERA 16MHz     Imagetek       |
|         DT_JA-6 DT_JA-5     I4220          |
|SW1      CY7C199 CY7C199                    |
|SW2        68000-16          CY7C199 61C64  |
|SW3                26.666MHz CY7C199        |
+--------------------------------------------+

CPU  : TMP68HC000P-16
Sound: YAMAHA OPL YMF278B-F + YRW801-M
OSC  : 16.0000MHz (OSC1) 26.6660MHz (OSC2) 33.869MHz (OSC3)
PLD  : ALTERA EPM7032LC44-15T D9519
Video: Imagetek I4220 071 9338EK709

SW3 - Not Populated
***************************************************************************/

ROM_START( daitorid )
	ROM_REGION( 0x040000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "dt-ja-5.19e", 0x000000, 0x020000, CRC(441efd77) SHA1(18b255f42ba7a180535f0897aaeebe5d2a33df46) )
	ROM_LOAD16_BYTE( "dt-ja-6.19c", 0x000001, 0x020000, CRC(494f9cc3) SHA1(b88af581fee9e2d94a12a5c1fed0797614bb738e) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "dt-ja-8.3h", 0x000000, 0x004000, CRC(0351ad5b) SHA1(942c1cbb52bf2933aea4209335c1bc4cdd1cc3dd) )
	ROM_CONTINUE(           0x010000, 0x01c000 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "dt-ja-2.14h", 0x000000, 0x080000, CRC(56881062) SHA1(150a8f043e61b28c22d0f898aea61853d1accddc) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "dt-ja-4.18h", 0x000002, 0x080000, CRC(85522e3b) SHA1(2c6e7c8ad01d39843669ef1afe7a0843ea6c107c) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "dt-ja-1.12h", 0x000004, 0x080000, CRC(2a220bf2) SHA1(553dea2ab42d845b2e91930219fe8df026748642) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "dt-ja-3.16h", 0x000006, 0x080000, CRC(fd1f58e0) SHA1(b4bbe94127ae59d4c899d09862703c374c8f4746) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
	ROM_LOAD( "dt-ja-7.3f", 0x000000, 0x040000, CRC(0d888cde) SHA1(fa871fc34f8b8ff0eebe47f338733e4f9fe65b76) )
ROM_END

ROM_START( daitorida )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "dt_ja-6.6", 0x000000, 0x040000, CRC(c753954e) SHA1(f895c776ec6e2da063d3fbf9630f4812ba7bc455) ) /* Silkscreened 6 and U18 */
	ROM_LOAD16_BYTE( "dt_ja-5.5", 0x000001, 0x040000, CRC(c4340290) SHA1(6748572a8733d88a1dd03604628e3d0e90171cf0) ) /* Silkscreened 5 and U19 */

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "dt_ja-2.2", 0x000000, 0x080000, CRC(6a262249) SHA1(93b58825a454403d568e7d9a3b4d998322d0baef) , ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened 2 and U30 */
	ROMX_LOAD( "dt_ja-4.4", 0x000002, 0x080000, CRC(cdcef57a) SHA1(4b386f5ebde1ab6866bbbe528e43b813eba99237) , ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened 4 and U29 */
	ROMX_LOAD( "dt_ja-1.1", 0x000004, 0x080000, CRC(a6ccb1d2) SHA1(87570b8d82af0529c054b3038b3d3e9aa550ce6a) , ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened 1 and U28 */
	ROMX_LOAD( "dt_ja-3.3", 0x000006, 0x080000, CRC(32353e04) SHA1(16ac82de9e6e43eabef3adab2d3a006bb50100fb) , ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened 3 and U27 */

	ROM_REGION( 0x280000, "ymf", 0 )
	ROM_LOAD( "yrw801-m",  0x000000, 0x200000, CRC(2a9d8d43) SHA1(32760893ce06dbe3930627755ba065cc3d8ec6ca) ) /* Silkscreened U52 */    // Yamaha YRW801 2MB ROM with samples for the OPL4.
	ROM_LOAD( "dt_ja-7.7", 0x200000, 0x080000, CRC(7a2d3222) SHA1(1a16bf483a5a086ad48029dd23dd16ad47c3740e) ) /* Silkscreened 7 and U49 */  // PCM 16 Bit (Signed)
ROM_END


/***************************************************************************

Dharma Doujou
Metro 1994

MTR5260-A
|-----------------------------------------------|
|TA7222            3.579545MHz                  |
|            YM3012                      6116   |
|C3403  C3403      YM2413  M6295                |
|       26.666MHz             DD_JA-7  DD_JA-8  |
|            7C199                              |
|J           7C199          |--------|  D78C10  |
|A           7C199          |IMAGETEK|          |
|M                          |I4220   |          |
|M               MM1035     |        |          |
|A         |------------|   |--------|          |
|          |  68000-12  |              DD_JB-1  |
|          |------------|                       |
|                                      DD_JB-2  |
|              24MHz                            |
|       MACH110                        DD_JB-3  |
|                  6264                         |
|DSW1              6264                DD_JB-4  |
|DSW2  DD_JC-6   DD_JC-5                        |
|-----------------------------------------------|
Notes:
      68000 clock     - 12.000MHz [24/2]
      D78C10 clock    - 12.000MHz [24/2]
      YM2413 clock    - 3.579545MHz
      Oki M6295 clock - 1.200MHz [24/20], sample rate = 1200000 / 132
      VSync - 60Hz
      HSync - 15.55kHz


Korean version & international version of Dharma run on Metro hardware PCB Number - METRO CORP. MTR527


***************************************************************************/

ROM_START( dharma )
	ROM_REGION( 0x040000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "dd__wea5.u39", 0x000000, 0x020000, CRC(960319d7) SHA1(f76783fcbb5e5a027889620c783f053d372346a8) )
	ROM_LOAD16_BYTE( "dd__wea6.u42", 0x000001, 0x020000, CRC(386eb6b3) SHA1(e353ea70bae521c4cc362cf2f5ce643c98c61681) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "dd__wa-8.u9", 0x000000, 0x004000, CRC(af7ebc4c) SHA1(6abf0036346da10be56932f9674f8c250a3ea592) ) // (c)1992 Imagetek (11xxxxxxxxxxxxxxx = 0xFF) // == dd_ja-8
	ROM_CONTINUE(        0x010000, 0x01c000 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "dd__wa-2.u4",  0x000000, 0x080000, CRC(2c67a5c8) SHA1(777d5f64446004bbb6dafee610ad9a1ff262349d) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "dd__wa-4.u5",  0x000002, 0x080000, CRC(36ca7848) SHA1(278788727193ae65ed012d230a4e5966c07afe9e) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "dd__wa-1.u10", 0x000004, 0x080000, CRC(d8034574) SHA1(a9bf29ae980033dfaae43b6ab46f850744020d92) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "dd__wa-3.u11", 0x000006, 0x080000, CRC(fe320fa3) SHA1(80532cc38bd21608e4cff1254d993e0df72eaccf) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
	ROM_LOAD( "dd__wa-7.u3", 0x000000, 0x040000, CRC(7ce817eb) SHA1(9dfb79021a552877fbc26049cca853c0b93735b5) ) // == dd_ja-7
ROM_END

ROM_START( dharmaj )
	ROM_REGION( 0x040000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "dd_jc-5", 0x000000, 0x020000, CRC(b5d44426) SHA1(d68aaf6b9976ccf5cb665d7ec0afa44e2453094d) )
	ROM_LOAD16_BYTE( "dd_jc-6", 0x000001, 0x020000, CRC(bc5a202e) SHA1(c2b6d2e44e3605e0525bde4030c5162badad4d4b) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "dd_ja-8", 0x000000, 0x004000, CRC(af7ebc4c) SHA1(6abf0036346da10be56932f9674f8c250a3ea592) ) // (c)1992 Imagetek (11xxxxxxxxxxxxxxx = 0xFF)
	ROM_CONTINUE(        0x010000, 0x01c000 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "dd_jb-2", 0x000000, 0x080000, CRC(2c07c29b) SHA1(26244145139df1ffe2b6ec25a32e5009da6a5aba) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "dd_jb-4", 0x000002, 0x080000, CRC(fe15538e) SHA1(a52ac04656783611ec5d5af01b18e22254decc0c) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "dd_jb-1", 0x000004, 0x080000, CRC(e6ca9bf6) SHA1(0379250303eb6895a4dda080da8bf031d055ce8e) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "dd_jb-3", 0x000006, 0x080000, CRC(6ecbe193) SHA1(33b799699d5d17705df36591cdc40032278388d1) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
	ROM_LOAD( "dd_ja-7", 0x000000, 0x040000, CRC(7ce817eb) SHA1(9dfb79021a552877fbc26049cca853c0b93735b5) )
ROM_END

ROM_START( dharmak )
	ROM_REGION( 0x040000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "5.bin", 0x000000, 0x020000, CRC(7dec1f77) SHA1(86cda990392e738f1bacec9d7a232d27887c1135) )
	ROM_LOAD16_BYTE( "6.bin", 0x000001, 0x020000, CRC(a194edbe) SHA1(676a4c0d4ee842a1b9d1c86ecd89417ebd6b5927) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "8.bin", 0x000000, 0x004000, CRC(d0e0a8e2) SHA1(99a3142589a1763ba162ed5b1b6c44961a5aaabc) )   // (c)1992 Imagetek (11xxxxxxxxxxxxxxx = 0xFF)
	ROM_CONTINUE(      0x010000, 0x01c000 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */ /* note, these are bitswapped, see init */
	ROMX_LOAD( "2.bin", 0x000000, 0x080000, CRC(3cc0bb6c) SHA1(aaa063fa748e0f6fe3c07f2dfb510c1b69ea92af) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "4.bin", 0x000002, 0x080000, CRC(2cdcdf91) SHA1(44da8eac822a89e9c07bfd28720ec0b566d19b44) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "1.bin", 0x000004, 0x080000, CRC(312ee2ec) SHA1(73ea401e4615eb9ad5f42be9c75ca4550c3a4668) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "3.bin", 0x000006, 0x080000, CRC(b81aede8) SHA1(fe11e1523a9bcd59397b5866bc03c9d24049a5f5) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
	ROM_LOAD( "7.bin", 0x000000, 0x040000, CRC(8af698d7) SHA1(9f8b2ecc07c19f38088cd4be05a498ae4f5af6f5) )
ROM_END



/*

Gun Master
Metro Corp. 1994

PCB Layout
----------

MTR5260-A
|-----------------------------------------------|
|TA7222            3.579545MHz                  |
|            YM3012                      6116   |
|C3403  C3403      YM2151  M6295                |
|       26.666MHz              GMJA-7   GMJA-8  |
|            6264                               |
|J           6264           |--------|  D78C10  |
|A           6264           |IMAGETEK|          |
|M                          |I4220   |          |
|M               MM1035     |        |          |
|A         |------------|   |--------|          |
|          |    68000   |               GMJA-1  |
|          |------------|                       |
|                                       GMJA-2  |
|              24MHz                            |
|       MACH110                         GMJA-3  |
|                  6264                         |
|DSW1              6264                 GMJA-4  |
|DSW2    GMJA-6    GMJA-5                       |
|-----------------------------------------------|
Notes:
      68000 clock     - 12.000MHz [24/2]
      D78C10 clock    - 12.000MHz [24/2]
      YM2151 clock    - 3.579545MHz
      Oki M6295 clock - 1.200MHz [24/20], sample rate = 1200000 / 132
      VSync - 60Hz
      HSync - 15.55kHz

RAM - CY7C199 (x2), 6164 (x2), LH5168 (x2), 6116 (x1)
ROMs 5+6 = Main Prg
ROMs 7+8 = Sound Data
ROMs 1-4 = GFX Data

*/

ROM_START( gunmast )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "gmja-5.20e", 0x000000, 0x040000, CRC(7334b2a3) SHA1(23f0a00b7539329f23eb564bc2823383997f83a9) )
	ROM_LOAD16_BYTE( "gmja-6.20c", 0x000001, 0x040000, CRC(c38d185e) SHA1(fdbc16a6ffc791778cb7ac2dafd15f4eb72c4cf9) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "gmja-8.3i", 0x000000, 0x004000, CRC(ab4bcc56) SHA1(9ef91e14d0974f30c874a12370ddd04ee8ab6d5d) )   // (c)1992 Imagetek (11xxxxxxxxxxxxxxx = 0xFF)
	ROM_CONTINUE(     0x010000, 0x01c000 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "gmja-2.14i", 0x000000, 0x080000, CRC(bc9acd54) SHA1(e6154cc5e8e33b38f56a0055dd0a51aa6adc4f9c) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "gmja-4.18i", 0x000002, 0x080000, CRC(f2d72d90) SHA1(575a01999e4608d1503904ba22310413b680b2b9) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "gmja-1.12i", 0x000004, 0x080000, CRC(336d0a90) SHA1(39ff59ba13e21f2a8488e5dc2d44cf2c50f7c4fb) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "gmja-3.16i", 0x000006, 0x080000, CRC(a6651297) SHA1(cdfb8a176cced552a9e72d39980c7fb005edf4f9) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
	ROM_LOAD( "gmja-7.3g", 0x000000, 0x040000, CRC(3a342312) SHA1(5c31bc9ec5159e1a0c9a931c7b702a31d3a1af10) )
ROM_END



/***************************************************************************

Karate Tournament
Mitchell 1992

Note: This identical PCB with ROM and PAL swap is used by Moeyo Gonta!! (Lady Killer)

VG460-(A)
|----------------------------------------------|
|TA7222       YM2413  KT008  D78C10      KT001 |
|VOLUME UPC3403 3.579545MHz M6295         6116 |
|       UPC3403 *YM2151                        |
|*UPC3403       *YM3012              24MHz     |
|                                              |
|M54532          62256 20MHz             MM1035|
|                62256         460A24  460A21  |
|J  HE-2         |--------|    460A23  460A22  |
|A  HE-2  6264   |IMAGETEK|                    |
|M  HE-2         |I4100   |                    |
|M               |        |  6264 KT002  |---| |
|A               |--------|  6264 KT003  | 6 | |
|                                        | 8 | |
|                                        | 0 | |
|                                        | 0 | |
|SW1                                     | 0 | |
|SW2          361A06    361A04           |---| |
|             361A07    361A05                 |
|----------------------------------------------|
Notes:
           * - Not populated
       68000 - Clock input 12.000MHz [24/2]
      D78C10 - NEC D78C10 8-bit CPU with A/D Converter. Clock input 12.000MHz [24/2]
       M6295 - OKI M6295 4-Channel Mixing ADPCM Voice Synthesis LSI. Clock input 1.200MHz [24/20]. Pin 7 HIGH
      YM2413 - Yamaha YM2413 FM Operator TYPE-LL sound IC. Clock input 3.579545MHz
       I4100 - ImageTek Inc. I4100 052 9227KK702 graphics chip
       KT001 - 27C010 EPROM at location 1I
       KT002 - 27C2001 EPROM at location 8G
       KT003 - 27C2001 EPROM at location 10G
       KT008 - 27C2001 EPROM at location 1D
      361A0* - 42 pin 1M x8-bit (8Mbit) mask ROM
460A24/23/22 - AMI PAL18CV8
      460A21 - AMI PAL22CV10
       SW1/2 - 8-position DIP switch
        HE-2 - Resistor array
     UPC3403 - NEC uPC3403C Quad Operational Amplifier
        6116 - 2k x8-bit SRAM
       62256 - 32k x8-bit SRAM
        6264 - 8k x8-bit SRAM
      M54532 - Mitsubishi M54532P Quad 1.5A Darlington Transistor Array with Clamp Diode
      MM1035 - Mitsumi System Reset IC with Built-in Watchdog Timer (==Fujitsu MB3773)
      TA7222 - Toshiba TA7222 5.8w Audio Power Amplifier
       HSync - 14.9505kHz
       VSync - 57.1556Hz

***************************************************************************/

ROM_START( karatour )
  ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
  ROM_LOAD16_BYTE( "2.2FAB.8G",  0x000000, 0x040000, CRC(199a28d4) SHA1(ae880b5d5a1703c54e0ef27015039c7bb05eb185) )  // Hand-written label "(2) 2FAB"
  ROM_LOAD16_BYTE( "3.0560.10G", 0x000001, 0x040000, CRC(b054e683) SHA1(51e28a99f87684f3e56c7a168523f94717903d79) )  // Hand-written label "(3) 0560"

  ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
  ROM_LOAD( "KT001.1I", 0x000000, 0x004000, CRC(1dd2008c) SHA1(488b6f5d15bdbc069ee2cd6d7a0980a228d2f790) )    // 11xxxxxxxxxxxxxxx = 0xFF
  ROM_CONTINUE(         0x010000, 0x01c000 )

  ROM_REGION( 0x400000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
  ROMX_LOAD( "361A04.15F", 0x000000, 0x100000, CRC(f6bf20a5) SHA1(cb4cb249eb1c106fe7ef0ace735c0cc3106f1ab7) , ROM_GROUPWORD | ROM_SKIP(6))
  ROMX_LOAD( "361A07.17D", 0x000002, 0x100000, CRC(794cc1c0) SHA1(ecfdec5874a95846c0fb7966fdd1da625d85531f) , ROM_GROUPWORD | ROM_SKIP(6))
  ROMX_LOAD( "361A05.17F", 0x000004, 0x100000, CRC(ea9c11fc) SHA1(176c4419cfe13ff019654a93cd7b0befa238bbc3) , ROM_GROUPWORD | ROM_SKIP(6))
  ROMX_LOAD( "361A06.15D", 0x000006, 0x100000, CRC(7e15f058) SHA1(267f0a5acb874d4fff3556ffa405e24724174667) , ROM_GROUPWORD | ROM_SKIP(6))

  ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
  ROM_LOAD( "8.4A06.1D", 0x000000, 0x040000, CRC(8d208179) SHA1(54a27ef155828435bc5eba60790a8584274c8b4a) )  // Hand-written label "(8) 4A06"
ROM_END

ROM_START( karatourj )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "kt002.8g",  0x000000, 0x040000, CRC(316a97ec) SHA1(4b099d2fa91822c9c85d647aab3d6779fc400250) )
	ROM_LOAD16_BYTE( "kt003.10g", 0x000001, 0x040000, CRC(abe1b991) SHA1(9b6327169d66717dd9dd74816bc33eb208c3763c) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "kt001.1i", 0x000000, 0x004000, CRC(1dd2008c) SHA1(488b6f5d15bdbc069ee2cd6d7a0980a228d2f790) )    // 11xxxxxxxxxxxxxxx = 0xFF
	ROM_CONTINUE(         0x010000, 0x01c000 )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "361a04.15f", 0x000000, 0x100000, CRC(f6bf20a5) SHA1(cb4cb249eb1c106fe7ef0ace735c0cc3106f1ab7) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "361a07.17d", 0x000002, 0x100000, CRC(794cc1c0) SHA1(ecfdec5874a95846c0fb7966fdd1da625d85531f) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "361a05.17f", 0x000004, 0x100000, CRC(ea9c11fc) SHA1(176c4419cfe13ff019654a93cd7b0befa238bbc3) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "361a06.15d", 0x000006, 0x100000, CRC(7e15f058) SHA1(267f0a5acb874d4fff3556ffa405e24724174667) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
	ROM_LOAD( "kt008.1d", 0x000000, 0x040000, CRC(47cf9fa1) SHA1(88923ace550154c58c066f859cadfa7864c5344c) )
ROM_END

/***************************************************************************

Moeyo Gonta!! (Lady Killer)
(c)1993 Yanyaka
VG460-(B)

CPU  : TMP68HC000P-16
Sound: D78C10ACW YM2413 M6295
OSC  : 3.579545MHz(XTAL1) 20.0000MHz(XTAL2) 24.0000MHz(XTAL3)

ROMs:
e1.1i - Sound program (27c010)

j2.8g  - Main programs (27c020)
j3.10g /

ladyj-4.15f - Graphics (mask, read as 27c800)
ladyj-5.17f |
ladyj-6.15d |
ladyj-7.17d /

e8j.1d - Samples (27c020)

Others:
Imagetek I4100 052 9330EK712

***************************************************************************/

ROM_START( ladykill )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "e2.8g",  0x000000, 0x040000, CRC(211a4865) SHA1(4315c0a708383d357d8dd89a1820fe6cf7652adb) )
	ROM_LOAD16_BYTE( "e3.10g", 0x000001, 0x040000, CRC(581a55ea) SHA1(41bfcaae84e583bf185948ab53ec39c05180a7a4) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "e1.1i", 0x000000, 0x004000, CRC(a4d95cfb) SHA1(2fd8a5cbb0dc289bd5294519dbd5369bfb4c2d4d) )   // 11xxxxxxxxxxxxxxx = 0xFF
	ROM_CONTINUE(      0x010000, 0x01c000 )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "ladyj-4.15f", 0x000000, 0x100000, CRC(65e5906c) SHA1(cc3918c2094ca819ec4043055564e1dbff4a4750) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "ladyj-7.17d", 0x000002, 0x100000, CRC(56bd64a5) SHA1(911272078b0fd375111f5d1463945c2075c19e40) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "ladyj-5.17f", 0x000004, 0x100000, CRC(a81ffaa3) SHA1(5c161b0ef33f1bab077e9a2eb2d3432825729e83) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "ladyj-6.15d", 0x000006, 0x100000, CRC(3a34913a) SHA1(a55624ede7c368e61555ca7b9cd9e6948265b784) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
	ROM_LOAD( "e8.1d", 0x000000, 0x040000, CRC(da88244d) SHA1(90c0cc275b69afffd9a0126985fd3fe16d44dced) )
ROM_END

/* an 'Electronic Devices' manufactured board has been seen with the following roms. The data is 100% identical to the above set
   but due to lazy manufacturing larger ROMs were used and the first half filled with 0xff

    ROM_LOAD16_BYTE( "ladyki_3.h9",   0x000000, 0x080000, CRC(c658f954) SHA1(d50043457e67a94feff1328fe9bf522aa3c124b6) ) // == e2.8g
    ROM_LOAD16_BYTE( "ladyki_2.h10",  0x000001, 0x080000, CRC(bf58e4db) SHA1(9d7f74dc348b0ccb3bcf1b618d6092292b6945b8) ) // == e3.10g
    ROM_LOAD( "ladyki_1.d1", 0x000000, 0x080000, CRC(3dca957c) SHA1(4b815b7cb124a38c639a4b425ed6e8b1f0946451) ) // == e8.1d
*/

ROM_START( moegonta )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "j2.8g",  0x000000, 0x040000, CRC(aa18d130) SHA1(6e0fd3b95d8589665b418bcae4fe64b288289c78) )
	ROM_LOAD16_BYTE( "j3.10g", 0x000001, 0x040000, CRC(b555e6ab) SHA1(adfc6eafec612c8770b9f832a0a2574c53c3d047) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "e1.1i", 0x000000, 0x004000, CRC(a4d95cfb) SHA1(2fd8a5cbb0dc289bd5294519dbd5369bfb4c2d4d) )   // 11xxxxxxxxxxxxxxx = 0xFF
	ROM_CONTINUE(      0x010000, 0x01c000 )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "ladyj-4.15f", 0x000000, 0x100000, CRC(65e5906c) SHA1(cc3918c2094ca819ec4043055564e1dbff4a4750) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "ladyj-7.17d", 0x000002, 0x100000, CRC(56bd64a5) SHA1(911272078b0fd375111f5d1463945c2075c19e40) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "ladyj-5.17f", 0x000004, 0x100000, CRC(a81ffaa3) SHA1(5c161b0ef33f1bab077e9a2eb2d3432825729e83) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "ladyj-6.15d", 0x000006, 0x100000, CRC(3a34913a) SHA1(a55624ede7c368e61555ca7b9cd9e6948265b784) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
	ROM_LOAD( "e8j.1d", 0x000000, 0x040000, CRC(f66c2a80) SHA1(d95ddc8fe4144a6ad4a92385ff962d0b9391d53b) )
ROM_END


/***************************************************************************

Last Fortress - Toride
Metro 1992
VG420

                                     TR_JB12 5216
                     SW2 SW1           NEC78C10   3.579MHz

                                                          6269
                                                          TR_JB11
  55328 55328 55328       24MHz

                           4064   4064   TR_   TR_          68000-12
       Imagetek                          JC10  JC09
       I4100

    TR_  TR_  TR_  TR_  TR_  TR_  TR_  TR_
    JC08 JC07 JC06 JC05 JC04 JC03 JC02 JC01

CPU     :MC68000P12
Sound   :Yamaha YM2413, OKI M6295
OSC     :24.0000MHz, 3.579545MHz
other   :D78C10ACW, Imagetek Inc I4100 052

Clock measurements by the Guru:
Master clock: 24.00MHz
 D7810 clock: 12.00MHz (24 / 2)
 M6295 clock: 1.200MHz (24 / 20), sample rate =  M6295 clock /165
YM2413 clock: 3.579545MHz

Vsync: 58Hz
HSync: 15.16kHz

***************************************************************************/

ROM_START( lastfort )
	ROM_REGION( 0x040000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "tr_jc09", 0x000000, 0x020000, CRC(8b98a49a) SHA1(15adca78d54973820d04f8b308dc58d0784eb900) )
	ROM_LOAD16_BYTE( "tr_jc10", 0x000001, 0x020000, CRC(8d04da04) SHA1(5c7e65a39929e94d1fa99aeb5fed7030b110451f) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "tr_jb12", 0x000000, 0x004000, CRC(8a8f5fef) SHA1(530b4966ec058cd80a2fc5f9e961239ce59d0b89) ) // (c)1992 Imagetek (11xxxxxxxxxxxxxxx = 0xFF)
	ROM_CONTINUE(        0x010000, 0x01c000 )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "tr_jc02", 0x000000, 0x020000, CRC(db3c5b79) SHA1(337f4c547a6267f317415cbc78cdac41574b1024) , ROM_SKIP(7))
	ROMX_LOAD( "tr_jc04", 0x000001, 0x020000, CRC(f8ab2f9b) SHA1(bfbbd5ec2bc039b8eaef92467c2e7fd3b425b477) , ROM_SKIP(7))
	ROMX_LOAD( "tr_jc06", 0x000002, 0x020000, CRC(47a7f397) SHA1(1d2b11b95ce81ca66713457283464d6d85753e4b) , ROM_SKIP(7))
	ROMX_LOAD( "tr_jc08", 0x000003, 0x020000, CRC(d7ba5e26) SHA1(294fd9b68eebd28ca64627f0d6e64b325cab18a0) , ROM_SKIP(7))
	ROMX_LOAD( "tr_jc01", 0x000004, 0x020000, CRC(3e3dab03) SHA1(e3c6eb73467f0ed207657084e51ee87d85152c3f) , ROM_SKIP(7))
	ROMX_LOAD( "tr_jc03", 0x000005, 0x020000, CRC(87ac046f) SHA1(6555a55642383990bc7a8282ab5ea8fc0ba6cd14) , ROM_SKIP(7))
	ROMX_LOAD( "tr_jc05", 0x000006, 0x020000, CRC(3fbbe49c) SHA1(642631e69d78898403013884cf0fb711ea000541) , ROM_SKIP(7))
	ROMX_LOAD( "tr_jc07", 0x000007, 0x020000, CRC(05e1456b) SHA1(51cd3ad2aa9c0adc7b9d63a337b247b4b65701ca) , ROM_SKIP(7))

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "tr_jb11", 0x000000, 0x020000, CRC(83786a09) SHA1(910cf0ccf4493f2a80062149f6364dbb6a1c2a5d) )
ROM_END

ROM_START( lastfortk )
	ROM_REGION( 0x040000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "7f-9",  0x000000, 0x020000, CRC(d2894c1f) SHA1(4f4ab6d8ce69999cd7c4a9ddabec8d1e8fefc6fc) )
	ROM_LOAD16_BYTE( "8f-10", 0x000001, 0x020000, CRC(9696ea39) SHA1(27af0c6399cd7be40aa8a1c1b58e0db8408aff11) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "tr_jb12", 0x000000, 0x004000, CRC(8a8f5fef) SHA1(530b4966ec058cd80a2fc5f9e961239ce59d0b89) ) // (c)1992 Imagetek (11xxxxxxxxxxxxxxx = 0xFF)
	ROM_CONTINUE(        0x010000, 0x01c000 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "7i-2",  0x000000, 0x040000, CRC(d1fe8d7b) SHA1(88b1973ebb47b91a49f6b4f722c9cc33e5330694) , ROM_SKIP(7))
	ROMX_LOAD( "10i-4", 0x000001, 0x040000, CRC(058126d4) SHA1(985177556c8545e6a65a41083246b31509de7130) , ROM_SKIP(7))
	ROMX_LOAD( "13i-6", 0x000002, 0x040000, CRC(39a9dea2) SHA1(9f8067cff15be93771d42b3776ee7ca1b7c61798) , ROM_SKIP(7))
	ROMX_LOAD( "16i-8", 0x000003, 0x040000, CRC(4c050baa) SHA1(3e0b2029d7c0b6cd32b22f147663cd22975ce8c3) , ROM_SKIP(7))
	ROMX_LOAD( "5i-1",  0x000004, 0x040000, CRC(0d503f05) SHA1(0b1ce22630bb2326930f0f3b5710c6c191730c45) , ROM_SKIP(7))
	ROMX_LOAD( "8i-3",  0x000005, 0x040000, CRC(b6d4f753) SHA1(2864ad5fe4186e4e15bb7d5dafa6a9b8c803d7d0) , ROM_SKIP(7))
	ROMX_LOAD( "12i-5", 0x000006, 0x040000, CRC(ce69c805) SHA1(88debdbd8e73da54c1c25a1a60f27a05dac3f104) , ROM_SKIP(7))
	ROMX_LOAD( "14i-7", 0x000007, 0x040000, CRC(0cb38317) SHA1(6e18096f6616aa0d9c4f3a2394561ed3f636731e) , ROM_SKIP(7))

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "tr_jb11", 0x000000, 0x020000, CRC(83786a09) SHA1(910cf0ccf4493f2a80062149f6364dbb6a1c2a5d) )
ROM_END

ROM_START( lastfortg ) /* German version on PCB VG460-(A) */
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "tr_ma02.8g",  0x000000, 0x020000, CRC(e6f40918) SHA1(c8c9369103530b2214c779c8a643ba9349b3eac5) )
	ROM_LOAD16_BYTE( "tr_ma03.10g", 0x000001, 0x020000, CRC(b00fb126) SHA1(7dd4b7a2d1c5401fde2275ef76fac1ccc586a0bd) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "tr_ma01.1i",  0x000000, 0x004000,  CRC(8a8f5fef) SHA1(530b4966ec058cd80a2fc5f9e961239ce59d0b89) ) /* Same as parent set, but different label */
	ROM_CONTINUE(            0x010000, 0x01c000 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "tr_ma04.15f", 0x000000, 0x080000, CRC(5feafc6f) SHA1(eb50905eb0d25eb342e08d591907f79b5eadff43) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "tr_ma07.17d", 0x000002, 0x080000, CRC(7519d569) SHA1(c88932a19a48d45a19b777113a4719b18f42a297) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "tr_ma05.17f", 0x000004, 0x080000, CRC(5d917ba5) SHA1(34fc72924fa2877c1038d7f61b22f7667af01e9f) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "tr_ma06.15d", 0x000006, 0x080000, CRC(d366c04e) SHA1(e0a67688043cb45916860d32ff1076d9257e6ad9) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "tr_ma08.1d", 0x000000, 0x020000, CRC(83786a09) SHA1(910cf0ccf4493f2a80062149f6364dbb6a1c2a5d) ) /* Same as parent set, but different label */
ROM_END

/***************************************************************************

Last Fortress - Toride (Erotic)
Metro Corporation.

Board number VG420

CPU: MC68000P12
SND: OKI M6295+ YM2413 + NEC D78C10ACW + NEC D4016 (ram?)
DSW: see manual (scanned in sub-directory Manual)
OSC: 24.000 MHz, 3.579545MHz

***************************************************************************/

ROM_START( lastforte )
	ROM_REGION( 0x040000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "tr_hc09", 0x000000, 0x020000, CRC(32f43390) SHA1(b5bad9d80f2155f277265fe487a59f0f4ec6575d) )
	ROM_LOAD16_BYTE( "tr_hc10", 0x000001, 0x020000, CRC(9536369c) SHA1(39291e92c107be35d130ff29533b42581efc308b) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "tr_jb12", 0x000000, 0x004000, CRC(8a8f5fef) SHA1(530b4966ec058cd80a2fc5f9e961239ce59d0b89) ) // (c)1992 Imagetek (11xxxxxxxxxxxxxxx = 0xFF)
	ROM_CONTINUE(        0x010000, 0x01c000 )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "tr_ha02", 0x000000, 0x020000, CRC(11cfbc84) SHA1(fb7005be7678564713b5480569f2cdab6c36f029) , ROM_SKIP(7))
	ROMX_LOAD( "tr_ha04", 0x000001, 0x020000, CRC(32bf9c26) SHA1(9d16eca8810d1823726dc9c047504bd24f2a55f7) , ROM_SKIP(7))
	ROMX_LOAD( "tr_ha06", 0x000002, 0x020000, CRC(16937977) SHA1(768bb6b1c9b90b2eedc9dbb19c8e9fa8f4265f17) , ROM_SKIP(7))
	ROMX_LOAD( "tr_ha08", 0x000003, 0x020000, CRC(6dd96a9b) SHA1(fe8214d57dc83157eff53f2d83bd3a4e2da91555) , ROM_SKIP(7))
	ROMX_LOAD( "tr_ha01", 0x000004, 0x020000, CRC(aceb44b3) SHA1(9a236eddbc916c206bfa694b576d971d788e8eb1) , ROM_SKIP(7))
	ROMX_LOAD( "tr_ha03", 0x000005, 0x020000, CRC(f18f1248) SHA1(30e39d904368c61a46719a0f21a6acb7fa55593f) , ROM_SKIP(7))
	ROMX_LOAD( "tr_ha05", 0x000006, 0x020000, CRC(79f769dd) SHA1(7a9ff8e961ae09fdf36a0a751befc141f47c9fd8) , ROM_SKIP(7))
	ROMX_LOAD( "tr_ha07", 0x000007, 0x020000, CRC(b6feacb2) SHA1(85df28d5ff6601753a435e31bcaf45702c7489ea) , ROM_SKIP(7))

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "tr_jb11", 0x000000, 0x020000, CRC(83786a09) SHA1(910cf0ccf4493f2a80062149f6364dbb6a1c2a5d) )
ROM_END

ROM_START( lastfortea )
	ROM_REGION( 0x040000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "tr_ha09", 0x000000, 0x020000, CRC(61fe8fb2) SHA1(d3f33bbc5326f89407fe1f4e389af7510ce134a0) )
	ROM_LOAD16_BYTE( "tr_ha10", 0x000001, 0x020000, CRC(14a9fba2) SHA1(984247397f204b9e1bdf69e68299b2e061fba5b1) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "tr_jb12", 0x000000, 0x004000, CRC(8a8f5fef) SHA1(530b4966ec058cd80a2fc5f9e961239ce59d0b89) ) // (c)1992 Imagetek (11xxxxxxxxxxxxxxx = 0xFF)
	ROM_CONTINUE(        0x010000, 0x01c000 )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "tr_ha02", 0x000000, 0x020000, CRC(11cfbc84) SHA1(fb7005be7678564713b5480569f2cdab6c36f029) , ROM_SKIP(7))
	ROMX_LOAD( "tr_ha04", 0x000001, 0x020000, CRC(32bf9c26) SHA1(9d16eca8810d1823726dc9c047504bd24f2a55f7) , ROM_SKIP(7))
	ROMX_LOAD( "tr_ha06", 0x000002, 0x020000, CRC(16937977) SHA1(768bb6b1c9b90b2eedc9dbb19c8e9fa8f4265f17) , ROM_SKIP(7))
	ROMX_LOAD( "tr_ha08", 0x000003, 0x020000, CRC(6dd96a9b) SHA1(fe8214d57dc83157eff53f2d83bd3a4e2da91555) , ROM_SKIP(7))
	ROMX_LOAD( "tr_ha01", 0x000004, 0x020000, CRC(aceb44b3) SHA1(9a236eddbc916c206bfa694b576d971d788e8eb1) , ROM_SKIP(7))
	ROMX_LOAD( "tr_ha03", 0x000005, 0x020000, CRC(f18f1248) SHA1(30e39d904368c61a46719a0f21a6acb7fa55593f) , ROM_SKIP(7))
	ROMX_LOAD( "tr_ha05", 0x000006, 0x020000, CRC(79f769dd) SHA1(7a9ff8e961ae09fdf36a0a751befc141f47c9fd8) , ROM_SKIP(7))
	ROMX_LOAD( "tr_ha07", 0x000007, 0x020000, CRC(b6feacb2) SHA1(85df28d5ff6601753a435e31bcaf45702c7489ea) , ROM_SKIP(7))

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "tr_jb11", 0x000000, 0x020000, CRC(83786a09) SHA1(910cf0ccf4493f2a80062149f6364dbb6a1c2a5d) )
ROM_END


/***************************************************************************

Mahjong Doukyuusei (JPN Ver.)

(c)1995 make software/elf/media trading corp.

Board: VG330-B

CPU   : 68000 16MHz
Sound : YM2413, M6295
OSC   : 16.0000MHz 3.579545MHz 26.666MHz
Custom: Imagetek Inc I4300

***************************************************************************/

ROM_START( dokyusei )
	ROM_REGION( 0x040000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "330_a06.bin", 0x000000, 0x020000, CRC(36157c2e) SHA1(f855175143caf476dcbee5a8aaec802a8fdb64fa) )
	ROM_LOAD16_BYTE( "330_a05.bin", 0x000001, 0x020000, CRC(177f50d2) SHA1(2298411152553041b907d9243aaa7983ca21c946) )

	ROM_REGION( 0x800000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "2.bin", 0x000000, 0x200000, CRC(075bface) SHA1(7f0e47ebdc37a1fc09b072cb8e0f38258a702a3d) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "4.bin", 0x000002, 0x200000, CRC(bc631438) SHA1(da3ef24d94e69197e3c69e4fd2b716162c275278) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "1.bin", 0x000004, 0x200000, CRC(4566c29b) SHA1(3216e21d898855cbb0ad328e6d45f3726d95b099) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "3.bin", 0x000006, 0x200000, CRC(5f6d7969) SHA1(bcb48c5808f268ca35a28f162d4e9da9df65b843) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x100000, "oki", 0 )    /* Samples */
	ROM_LOAD( "7.bin", 0x000000, 0x100000, CRC(c572aee1) SHA1(2a3baf962617577f8ac3f9e58fb4e5a0dae4f0e8) )   // 4 x 0x40000
ROM_END


/***************************************************************************

Mahjong Doukyuusei Special
(c)1995 Make Software / Elf / Media Trading

Board:  VG340-A

CPU:    68000-16
Sound:  M6295
        YM2413
OSC:    32.0000MHz
        3.579545MHz
EEPROM: 93C46
Custom: Imagetek Inc I4300 095

***************************************************************************/

ROM_START( dokyusp )
	ROM_REGION( 0x040000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "6.bin", 0x000000, 0x020000, CRC(01729b7f) SHA1(42a60f034ee5d5c2a42856b97d0d4c499b24627b) )
	ROM_LOAD16_BYTE( "5.bin", 0x000001, 0x020000, CRC(57770776) SHA1(15093886f2fe49443e8d7541903714de0a14aa0b) )

	ROM_REGION( 0x1000000, "gfx1", 0 )  /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "2l.bin", 0x0000000, 0x400000, CRC(4bed184d) SHA1(12bdb00030d19c2c9fb2120ed6b267a7982c213a) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "4l.bin", 0x0000002, 0x400000, CRC(2ee468e6) SHA1(ced58fdd8b5c99ce3f09cece2e05d7fcf4c7f786) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "1l.bin", 0x0000004, 0x400000, CRC(510ace14) SHA1(f5f1f46f4d8d150dd9e17083f32e9b45938c1dad) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "3l.bin", 0x0000006, 0x400000, CRC(82ea562e) SHA1(42839de9f346ccd0736bdbd3eead61ad66fcb666) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x200000, "oki", 0 )    /* Samples */
	ROM_LOAD( "7.bin", 0x000000, 0x200000, CRC(763985e1) SHA1(395d925b79922de5060a3f59de99fbcc9bd40fad) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-dokyusp.bin", 0x0000, 0x0080, CRC(cf159485) SHA1(f8e9c89e1b7c8bcd77ae5f55e334f79285f602a8) )
ROM_END


/***************************************************************************

Mahjong Gakuensai (JPN Ver.)
(c)1997 Make Software

Board:  VG340-A

CPU:    68000-16
Sound:  M6295
        YM2413
OSC:    26.6660MHz
        3.5795MHz

Custom: I4300 095

***************************************************************************/

ROM_START( gakusai )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "6.bin", 0x000000, 0x040000, CRC(6f8ab082) SHA1(18caf49a0c65f831d375f089f27b8570b094f029) )
	ROM_LOAD16_BYTE( "5.bin", 0x000001, 0x040000, CRC(010176c4) SHA1(48fcea18c02c1426a699a636f44b21cf7625e8a0) )

	ROM_REGION( 0x2000000, "gfx1", 0 )  /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "2l.bin", 0x0000000, 0x400000, CRC(45dfb5c7) SHA1(04338d695bd6973fd7d7286a8da563250ae4f71b) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "4l.bin", 0x0000002, 0x400000, CRC(7ab64f49) SHA1(e4d9a7bf97635b41fe632b3542eee1f609db080a) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "1l.bin", 0x0000004, 0x400000, CRC(75093421) SHA1(cfe549e24abfedd740ead30cab235df494e9f45d) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "3l.bin", 0x0000006, 0x400000, CRC(4dcfcd98) SHA1(bfb882d99c854e68e86f4e8f8aa7d02dcf5e9cfc) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "2u.bin", 0x1000000, 0x400000, CRC(8d4f912b) SHA1(1fcf1dd50fd678cc908ab47bcccaa4ed7b2b6938) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "4u.bin", 0x1000002, 0x400000, CRC(1f83e98a) SHA1(10b2d3ceb4bda6a2ecf795b865c948563c2fb84d) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "1u.bin", 0x1000004, 0x400000, CRC(28b386d9) SHA1(d1e151fa112c86d2cb97b7a5439a1e549359055d) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "3u.bin", 0x1000006, 0x400000, CRC(87f3c5e6) SHA1(097c0a53b040399d928f17fe3e9f42755b1d72f3) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x400000, "oki", 0 )    /* Samples */
	ROM_LOAD( "7.bin", 0x000000, 0x400000, CRC(34575a14) SHA1(53d458513f208f07844e1727d5889e85dcd4f0ed) )
ROM_END


/***************************************************************************

Mahjong Gakuensai 2 (JPN Ver.)
(c)1998 Make Software

Board:  VG340-A

CPU:    68000-16
Sound:  M6295
        YM2413
OSC:    26.6660MHz
        3.579545MHz

Custom: I4300 095

***************************************************************************/

ROM_START( gakusai2 )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "mg2a06.bin", 0x000000, 0x020000, CRC(8b006dd4) SHA1(893ec0e7c367d79bc99e65ab8abd0d290f2ede58) )
	ROM_LOAD16_BYTE( "mg2a05.bin", 0x000001, 0x020000, CRC(7702b9ac) SHA1(09d0c11fa2c9ed9cde365cb1ff215d55e39b7734) )

	ROM_REGION( 0x2000000, "gfx1", 0 )  /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "mg22l.bin", 0x0000000, 0x400000, CRC(28366708) SHA1(56fccee126916cc301678a205dfe629efefb79db) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "mg24l.bin", 0x0000002, 0x400000, CRC(9e003bb0) SHA1(aa73cc0e79732fd6826c89671b179cb3189571e0) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "mg21l.bin", 0x0000004, 0x400000, CRC(3827098d) SHA1(dda9fb6c56c4408802d54c5975fb9470ca2e1d34) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "mg23l.bin", 0x0000006, 0x400000, CRC(a6f96961) SHA1(dd2578da5d091991580a2c7a979ba8dbfa0cceb3) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "mg22u.bin", 0x1000000, 0x400000, CRC(53ffa68a) SHA1(3d8d69c2063c78bd79cdbd7457bca1af9700bf3c) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "mg24u.bin", 0x1000002, 0x400000, CRC(c218e9ab) SHA1(3b6ee4cc828198b284ac9020e2da911efc90725a) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "mg21u.bin", 0x1000004, 0x400000, CRC(385495e5) SHA1(5181e279fef23780d07ab5a124618e4d0e5cb821) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "mg23u.bin", 0x1000006, 0x400000, CRC(d8315923) SHA1(6bb5cad317f7efa6a384f6c257c5faeb789a8eed) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x400000, "oki", 0 )    /* Samples */
	ROM_LOAD( "mg2-7.bin", 0x000000, 0x400000, CRC(2f1c041e) SHA1(a72720b3d7f816e23452775f2fd4223cf2d02985) )
ROM_END


/***************************************************************************

Mouja (JPN Ver.)
(c)1996 Etona / (c)1995 FPS/FWS

VG410-B
+------------------------+
|       SW2 SW1 68000-16 |
|       SW4 SW3   10   9 |
|             62256 62256|
|J YM2413 ALTERA         |
|A  3.579545MHz          |
|M   M6295 16MHz LH53882B|
|M LH538711              |
|A 26.666MHz     LH53882C|
|        I4300           |
|  61S256        LH53882D|
|  61S256 61C64          |
|                LH53882E|
+------------------------+

CPU     :TMP68H000P-16
Sound   :Yamaha YM2413, OKI M6295
OSC     :16000.00KHz, 3.579545MHz, 26.666MHz
other   :Imagetek Inc I4300 095, ALTERA EPM7032LC44-15T

* SW3 & SW4 are unpopulated

9, 10 Program roms, 27C020

LH53882B - LH53882E are MASK roms
LH53711 is a MASK rom

***************************************************************************/

ROM_START( mouja )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "10.u38", 0x000000, 0x040000, CRC(f9742b69) SHA1(f8c6508b227403a82413ceeb0651922759d7e0f4) ) /* Silkscreened U38 and 10 */
	ROM_LOAD16_BYTE( "9.u39",  0x000001, 0x040000, CRC(dc77216f) SHA1(3b73d29f4e8e385f45f2abfb38eaffc2d8406948) ) /* Silkscreened U39 and 9 */

	ROM_REGION( 0x400000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "lh53882c.u6", 0x000000, 0x100000, CRC(c4dd3194) SHA1(c9c88a8d2046224957b35de14763aa4bdf0d407f) , ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened U6 and 42 */
	ROMX_LOAD( "lh53882e.u5", 0x000002, 0x100000, CRC(09530f9d) SHA1(03f2ec5ea694266808d245abe7f688de0ef6d853) , ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened U5 and 86 */
	ROMX_LOAD( "lh53882b.u4", 0x000004, 0x100000, CRC(5dd7a7b2) SHA1(b0347e8951b29356a7d945b906d93c40b9abc19c) , ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened U4 and 31 */
	ROMX_LOAD( "lh53882d.u1", 0x000006, 0x100000, CRC(430c3925) SHA1(41e5bd02a665eee87ef8f4ae9f4bee374c25e00b) , ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened U1 and 75 */

	ROM_REGION( 0x100000, "oki", 0 )  /* Samples */
	ROM_LOAD( "lh538711.u53",     0x000000, 0x100000, CRC(fe3df432) SHA1(4fb7ad997ca6e91468d7516e5c4a94cde6e07104) ) /* Silkscreened U53 and 11 */
ROM_END


/***************************************************************************

Mouse Shooter GoGO
Metro 1995

+--------------------------------------------+
|                 MS_WA-7            MS_JA-1 |
|                 YRW801-M           MS_WA-2 |
|                                    MS_WA-3 |
|J      33.369MHz YMF278B            MS_WA-4 |
|A                                           |
|M                                           |
|M                                           |
|A           ALTERA 16MHz     Imagetek       |
|         MS_WA-6 MS_WA-5     I4220          |
|SW1      CY7C199 CY7C199                    |
|SW2        68000-16          CY7C199  61C64 |
|SW3                26.666MHz CY7C199        |
+--------------------------------------------+

CPU  : TMP68HC000P-16
Sound: YAMAHA OPL YMF278B-F + YRW801-M
OSC  : 16.0000MHz (OSC1) 26.6660MHz (OSC2) 33.869MHz (OSC3)
PLD  : ALTERA EPM7032LC44-15T D9443
Video: Imagetek I4220 071 9430WK440

SW3 - Not Populated

ms_ja-1.1    tms27c240 <-- Is there an undumped MS_WA1 World rom??
ms_wa-2.2    tms27c240
ms_wa-3.3    tms27c240
ms_wa-4.4    tms27c240
ms_wa-5.5    tms27c020
ms_wa-6.6    tms27c020
ms_wa-7.7    hn27c4001g

***************************************************************************/

ROM_START( msgogo )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "ms_wa-6.6", 0x000000, 0x040000, CRC(986acac8) SHA1(97c24f5b730aa811951db4c7e9c894c0701c58fd) ) /* Silkscreened 6 and U18 */
	ROM_LOAD16_BYTE( "ms_wa-5.5", 0x000001, 0x040000, CRC(746d9f99) SHA1(6e3e34dfb67fecc93213fe040465eccd88575822) ) /* Silkscreened 5 and U19 */

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "ms_wa-2.2", 0x000000, 0x080000, CRC(0d36c2b9) SHA1(3fd6631ad657c73e7e6bfdff9d9caf5ab044bdeb), ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened 2 and U30 */
	ROMX_LOAD( "ms_wa-4.4", 0x000002, 0x080000, CRC(fd387126) SHA1(a2f82a66b098a97d8f245e3c2f96c31c63642fec), ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened 4 and U29 */
	ROMX_LOAD( "ms_ja-1.1", 0x000004, 0x080000, CRC(8ec4e81d) SHA1(46947ad2941af154f91e47acee281302a12e3aa5), ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened 1 and U28 */
	ROMX_LOAD( "ms_wa-3.3", 0x000006, 0x080000, CRC(06cb6807) SHA1(d7303b4047983117cd33e057b1f4b98ed3f7dd32), ROM_GROUPWORD | ROM_SKIP(6)) /* Silkscreened 3 and U27 */

	ROM_REGION( 0x280000, "ymf", 0 )
	ROM_LOAD( "yrw801-m",  0x000000, 0x200000, CRC(2a9d8d43) SHA1(32760893ce06dbe3930627755ba065cc3d8ec6ca) ) /* Silkscreened U52 */
	ROM_LOAD( "ms_wa-7.7", 0x200000, 0x080000, CRC(e19941cb) SHA1(93777c9cd22ddd33d9584b6edad33b95c1e28bde) ) /* Silkscreened 7 and U49 */
ROM_END


/***************************************************************************

Pang Pom's (c) 1992 Metro

Pcb code:  VG420 (Same as Toride)

Cpus:  M68000, Z80
Clocks: 24 MHz, 3.579 MHz
Sound: M6295, YM2413, _unused_ slot for a YM2151

Custom graphics chip - Imagetek I4100 052 9227KK701 (same as Karate Tournament)

***************************************************************************/

ROM_START( pangpoms )
	ROM_REGION( 0x040000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "ppoms09.bin", 0x000000, 0x020000, CRC(0c292dbc) SHA1(8b09de2a560e804e0dea514c95b317c2e2b6501d) )
	ROM_LOAD16_BYTE( "ppoms10.bin", 0x000001, 0x020000, CRC(0bc18853) SHA1(68d50ad50caad34e72d32e7b9fea1d85af74b879) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "ppoms12.bin", 0x000000, 0x004000, CRC(a749357b) SHA1(1555f565c301c5be7c49fc44a004b5c0cb3777c6) )
	ROM_CONTINUE(            0x010000, 0x01c000 )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "ppoms02.bin", 0x000000, 0x020000, CRC(88f902f7) SHA1(12ea58d7c000b629ccdceec3dedc2747a63b84be) , ROM_SKIP(7))
	ROMX_LOAD( "ppoms04.bin", 0x000001, 0x020000, CRC(9190c2a0) SHA1(a7399cc2dea5a963e7c930e426915e8eb3552213) , ROM_SKIP(7))
	ROMX_LOAD( "ppoms06.bin", 0x000002, 0x020000, CRC(ed15c93d) SHA1(95072e7d1def0d8e97946a612b90ce078c64aed2) , ROM_SKIP(7))
	ROMX_LOAD( "ppoms08.bin", 0x000003, 0x020000, CRC(9a3408b9) SHA1(924b184d3a47bbe8aa5d41761ea5e94ba7e4f2e9) , ROM_SKIP(7))
	ROMX_LOAD( "ppoms01.bin", 0x000004, 0x020000, CRC(11ac3810) SHA1(6ada82a73d4383f99f5be67369b810a692d27ef9) , ROM_SKIP(7))
	ROMX_LOAD( "ppoms03.bin", 0x000005, 0x020000, CRC(e595529e) SHA1(91b4bd1f029ce09d7689815099b38916fe0d2686) , ROM_SKIP(7))
	ROMX_LOAD( "ppoms05.bin", 0x000006, 0x020000, CRC(02226214) SHA1(82302e7f1e7269c45e11dfba45ec7bbf522b47f1) , ROM_SKIP(7))
	ROMX_LOAD( "ppoms07.bin", 0x000007, 0x020000, CRC(48471c87) SHA1(025fa79993788a0091c4edb83423725abd3a47a2) , ROM_SKIP(7))

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ppoms11.bin", 0x000000, 0x020000, CRC(e89bd565) SHA1(6c7c1ad67ba708dbbe9654c1d290af290207d2be) )
ROM_END

ROM_START( pangpomsm )
	ROM_REGION( 0x040000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "pa.c09", 0x000000, 0x020000, CRC(e01a7a08) SHA1(1890b290dfb1521ab73b2392409aaf44b99d63bb) )
	ROM_LOAD16_BYTE( "pa.c10", 0x000001, 0x020000, CRC(5e509cee) SHA1(821cfbf5f65cc3091eb8008310266f9f2c838072) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "ppoms12.bin", 0x000000, 0x004000, CRC(a749357b) SHA1(1555f565c301c5be7c49fc44a004b5c0cb3777c6) )
	ROM_CONTINUE(            0x010000, 0x01c000 )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "ppoms02.bin", 0x000000, 0x020000, CRC(88f902f7) SHA1(12ea58d7c000b629ccdceec3dedc2747a63b84be) , ROM_SKIP(7))
	ROMX_LOAD( "pj.e04",      0x000001, 0x020000, CRC(54bf2f10) SHA1(2f0f18984e336f226457295d375a73bcf86cef31) , ROM_SKIP(7))
	ROMX_LOAD( "pj.e06",      0x000002, 0x020000, CRC(c8b6347d) SHA1(7090e44dc7032432795b6fb6bc166bf4de159685) , ROM_SKIP(7))
	ROMX_LOAD( "ppoms08.bin", 0x000003, 0x020000, CRC(9a3408b9) SHA1(924b184d3a47bbe8aa5d41761ea5e94ba7e4f2e9) , ROM_SKIP(7))
	ROMX_LOAD( "ppoms01.bin", 0x000004, 0x020000, CRC(11ac3810) SHA1(6ada82a73d4383f99f5be67369b810a692d27ef9) , ROM_SKIP(7))
	ROMX_LOAD( "pj.e03",      0x000005, 0x020000, CRC(d126e774) SHA1(f782d1e1277956f088dc91dec8f338f85b9af13a) , ROM_SKIP(7))
	ROMX_LOAD( "pj.e05",      0x000006, 0x020000, CRC(79c0ec1e) SHA1(b15582e89d859dda4f82908c62e9e07cb45229b9) , ROM_SKIP(7))
	ROMX_LOAD( "ppoms07.bin", 0x000007, 0x020000, CRC(48471c87) SHA1(025fa79993788a0091c4edb83423725abd3a47a2) , ROM_SKIP(7))

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ppoms11.bin", 0x000000, 0x020000, CRC(e89bd565) SHA1(6c7c1ad67ba708dbbe9654c1d290af290207d2be) )
ROM_END


/***************************************************************************

Poitto! (c)1993 Metro corp
MTR5260-A

CPU  : TMP68HC000P-16
Sound: D78C10ACW M6295 YM2413
OSC  : 24.0000MHz  (OSC1)
                   (OSC2)
                   (OSC3)
       3.579545MHz (OSC4)
                   (OSC5)

ROMs:
pt-1.13i - Graphics (23c4000)
pt-2.15i |
pt-3.17i |
pt-4.19i /

pt-jd05.20e - Main programs (27c010)
pt-jd06.20c /

pt-jc07.3g - Sound data (27c020)
pt-jc08.3i - Sound program (27c010)

Others:
Imagetek I4100 052 9309EK701 (208pin PQFP)
AMD MACH110-20 (CPLD)

***************************************************************************/

ROM_START( poitto )
	ROM_REGION( 0x040000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "pt-jd05.20e", 0x000000, 0x020000, CRC(6b1be034) SHA1(270c94f6017c5ce77f562bfe17273c79d4455053) )
	ROM_LOAD16_BYTE( "pt-jd06.20c", 0x000001, 0x020000, CRC(3092d9d4) SHA1(4ff95355fdf94eaa55c0ad46e6ce3b505e3ef790) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "pt-jc08.3i", 0x000000, 0x004000, CRC(f32d386a) SHA1(655c561aec1112d88c1b94725e932059e5d1d5a8) )  // 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_CONTINUE(           0x010000, 0x01c000 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "pt-2.15i", 0x000000, 0x080000, CRC(05d15d01) SHA1(24405908fb8207228cd3419657e0be49e413f152) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "pt-4.19i", 0x000002, 0x080000, CRC(8a39edb5) SHA1(1d860e0a1b975a93907d5bb0704e3bad383bbda7) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "pt-1.13i", 0x000004, 0x080000, CRC(ea6e2289) SHA1(2c939b32d2bf155bb5c8bd979dadcf4f75e178b0) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "pt-3.17i", 0x000006, 0x080000, CRC(522917c1) SHA1(cc2f5b574d31b0b93fe52c690f450b20b233dcad) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
	ROM_LOAD( "pt-jc07.3g", 0x000000, 0x040000, CRC(5ae28b8d) SHA1(5e5f80ebbc4e3726ac8dbbfbefb9217f2e3e3563) )
ROM_END


/***************************************************************************

Puzzlet
2000 Yunizu Corporation

Very small PCB using Metro-like hardware with
Imagetek GFX chip and H8/3007 CPU

PCB Layout
----------
VG2200-(B)
|--------------------------------------------|
|TA7222     YM2413B  SOUND4   PRG1           |
|  VOL  JRC3403              DSW1(8)         |
|             M6295          DSW2(8) CG2 CG3 |
|          20MHz                             |
|  MM1035             |---------|            |
|      H8/3007        |IMAGETEK |    CY7C199 |
|TD62307     HM6216255|I4300    |            |
|TD62064              |         | CY7C199    |
|                     |         |            |
|  Z86E02             |---------| CY7C199    |
|        JAMMA              26.666MHz        |
|--------------------------------------------|
Notes:
      H8/3007   - Hitachi HD6413007F20 CPU. Clock 20MHz
      M6295     - Clock 4MHz [20/5]. Pin7 LOW
      I4300     - Imagetek I4300 Graphics Generator IC
      VSync     - 58Hz
      HSync     - 15.26kHz
      Z86E02    - DIP18 surface scratched, decapping reveals Zilog Z8 MCU
      HM6216255 - Hitachi 4M high speed SRAM (256-kword x16-bit)
      CY7C199   - 32k x8 SRAM
      YM2413B   - Clock 4MHz [20/5]
      MM1035    - System Reset IC with Watchdog Timer
      TD62307   - 7 Channel Low Saturation Driver
      TD62064   - 4 Channel High Current Darlington Driver
      TA7222    - 40V 4.5A 12.5W 5.8W Audio Power Amplifier IC

      All ROMs 27C160

***************************************************************************/

ROM_START( puzzlet )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* H8/3007 Code */
	ROM_LOAD16_WORD_SWAP( "prg1_ver2.u9", 0x000000, 0x200000, CRC(592760da) SHA1(08f7493d2e50831438f53bbf0ae211ec40057da7) )

	ROM_REGION( 0x200, "z86e02", 0 )    /* Zilog Z8 family 8-bit MCU */
	ROM_LOAD( "z86e02.mcu", 0x000000, 0x200, CRC(399fa417) SHA1(f6c57020ea394c858742759050bf4f4b2f1e1fc5) )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "cg2.u2", 0x000000, 0x200000, CRC(7720f2d8) SHA1(8e0ccd1e8efe00df909327aefdb1e23e50487524), ROM_GROUPWORD | ROM_SKIP(2))
	ROMX_LOAD( "cg3.u1", 0x000002, 0x200000, CRC(77d39d12) SHA1(4bb339e479f0425931cff4eef3a6bc6ad1fac1f5), ROM_GROUPWORD | ROM_SKIP(2))

	ROM_REGION( 0x200000, "oki", 0 )    /* Samples */
	ROM_LOAD( "sound4.u23", 0x000000, 0x200000, CRC(9a611369) SHA1(97b9188354292b120a1bd0f01b4d884461bfa298) )
ROM_END


/***************************************************************************

Puzzli
Metro/Banpresto 1995

MTR5260-A                3.5759MHz  12MHz
               YM2151                         6116
   26.666MHz           M6295    PZ_JB7  PZ_JB8
                                     78C10
      7C199         Imagetek
      7C199           I4220
      61C64

                                          PZ_JB1
           68000-16                       PZ_JB2
               32MHz   6164               PZ_JB3
                       6164               PZ_JB4
    SW1     PZ_JB6 PZ_JB5
    SW2

***************************************************************************/

ROM_START( puzzli )
	ROM_REGION( 0x040000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "pz_jb5.20e", 0x000000, 0x020000, CRC(33bbbd28) SHA1(41a98cfbdd60a638e4aa08f15f1730a2436106f9) )
	ROM_LOAD16_BYTE( "pz_jb6.20c", 0x000001, 0x020000, CRC(e0bdea18) SHA1(9941a2cd88d7a3c1a640f837d9f34c39ba643ee5) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "pz_jb8.3i", 0x000000, 0x004000, CRC(c652da32) SHA1(907eba5103373ca6204f9d62c426ccdeef0a3791) )
	ROM_CONTINUE(          0x010000, 0x01c000 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "pz_jb2.14i", 0x000000, 0x080000, CRC(0c0997d4) SHA1(922d8553ef505f65238e5cc77b45861a80022d75) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "pz_jb4.18i", 0x000002, 0x080000, CRC(576bc5c2) SHA1(08c10e0a3356ee1f79b78eff92395d8b18e43485) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "pz_jb1.12i", 0x000004, 0x080000, CRC(29f01eb3) SHA1(1a56f0b8efb599ae4f3cd0a4f0b6a6152ea6b117) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "pz_jb3.16i", 0x000006, 0x080000, CRC(6753e282) SHA1(49d092543db34f2cb54697897790df12ca3eda74) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
	ROM_LOAD( "pz_jb7.3g", 0x000000, 0x040000, CRC(b3aab610) SHA1(9bcf1f98e19a7e26b22e152313dfbd43c882f008) )
ROM_END


/***************************************************************************

Sankokushi (JPN Ver.)
(c)1996 Mitchell

Board:  MTR5260-A

sound: YM2413 + M6295

***************************************************************************/

ROM_START( 3kokushi )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "5.20e", 0x000000, 0x040000, CRC(6104ea35) SHA1(efb4a9a98577894fac720028f18cb9877a00239a) )
	ROM_LOAD16_BYTE( "6.20c", 0x000001, 0x040000, CRC(aac25540) SHA1(811de761bb1b3cc47d811b00f4b5c960c8f061d0) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "8.3i", 0x000000, 0x004000, CRC(f56cca45) SHA1(4739b83b0b3a4235fac10def3d26b0bd190eb12a) )    // (c)1992 Imagetek (11xxxxxxxxxxxxxxx = 0xFF)
	ROM_CONTINUE(     0x010000, 0x01c000 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "2.14i", 0x000000, 0x080000, CRC(291f8149) SHA1(82f460517543ef544c21a81e51987fb2f5c6273d) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "4.18i", 0x000002, 0x080000, CRC(9317c359) SHA1(9756757fb5d2b298a2b1917a131f391ef0e31fb9) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "1.12i", 0x000004, 0x080000, CRC(d5495759) SHA1(9cbcb48915ec44a8026d88d96ab391e118e89df5) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "3.16i", 0x000006, 0x080000, CRC(3d76bdf3) SHA1(f621fcc8e6bde58077216b534c2e876ea9311e15) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
	ROM_LOAD( "7.3g", 0x000000, 0x040000, CRC(78fe9d44) SHA1(365a2d51daa24741957fa619bbbbf96e8f370701) )
ROM_END


/***************************************************************************

Pururun (c)1995 Metro/Banpresto
MTR5260-A

CPU  : TMP68HC000P-16
Sound: D78C10ACW M6295 YM2151 Y3012
OSC  : 24.000MHz   (OSC1)
                   (OSC2)
       26.6660MHz  (OSC3)
                   (OSC4)
       3.579545MHz (OSC5)

ROMs:
pu9-19-1.12i - Graphics (27c4096)
pu9-19-2.14i |
pu9-19-3.16i |
pu9-19-4.18i /

pu9-19-5.20e - Main programs (27c010)
pu9-19-6.20c /

pu9-19-7.3g - Sound data (27c020)
pu9-19-8.3i - Sound program (27c010)

Others:
Imagetek I4220 071 9338EK707 (208pin PQFP)
AMD MACH110-20 (CPLD)

***************************************************************************/

ROM_START( pururun )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "pu9-19-5.20e", 0x000000, 0x020000, CRC(5a466a1b) SHA1(032eeaf66ce1b601385a8e76d2efd9ea6fd34680) )
	ROM_LOAD16_BYTE( "pu9-19-6.20c", 0x000001, 0x020000, CRC(d155a53c) SHA1(6916a1bad82c624b8757f5124416dac50a8dd7f5) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "pu9-19-8.3i", 0x000000, 0x004000, CRC(edc3830b) SHA1(13ee759d10711218465f6d7155e9c443a82b323c) )
	ROM_CONTINUE(            0x010000, 0x01c000 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "pu9-19-2.14i", 0x000000, 0x080000, CRC(21550b26) SHA1(cb2a2f672cdca84def2fac8d325b7a80a1e9bfc0) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "pu9-19-4.18i", 0x000002, 0x080000, CRC(3f3e216d) SHA1(9881e07d5ee237b7134e2ddcf9a9887a1d7f3b4c) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "pu9-19-1.12i", 0x000004, 0x080000, CRC(7e83a75f) SHA1(9f516bbfc4ca8a8e857ebf7a19c37d7f026695a6) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "pu9-19-3.16i", 0x000006, 0x080000, CRC(d15485c5) SHA1(d37670b0d696f4ee9da7b8199da114fb4e45cd20) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
	ROM_LOAD( "pu9-19-7.3g", 0x000000, 0x040000, CRC(51ae4926) SHA1(1a69a00e960bda399aaf051b3dcc9e0a108c8047) )
ROM_END


/***************************************************************************

Sky Alert (JPN Ver.)
(c)1992 Metro
VG420
                                     SA
                     SW2 SW1         B12   4016
                                           NEC78C10  3.579MHz

                                                          6269
                                                          SA
                                                          A11
  55328 55328 55328       24MHz

                           4064   4064   SA   SA          68000-12
       Imagetek                          C10  C09
       I4100


    SA  SA  SA  SA  SA  SA  SA  SA
    A08 A07 A06 A05 A04 A03 A02 A01

CPU     :MC68000P12
Sound   :Yamaha YM2413, OKI M6295
OSC     :24.0000MHz, 3.579545MHz
other   :D78C10ACW, Imagetek Inc I4100 052

Master clock: 24.00MHz
 D7810 clock: 12.00MHz (24 / 2)
 M6295 clock: 1.200MHz (24 / 20), sample rate =  M6295 clock /165
YM2413 clock: 3.579545MHz


***************************************************************************/

ROM_START( skyalert )
	ROM_REGION( 0x040000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "sa_c_09.bin", 0x000000, 0x020000, CRC(6f14d9ae) SHA1(37e134af3d8461280dab971bc3ee9112f25de335) )
	ROM_LOAD16_BYTE( "sa_c_10.bin", 0x000001, 0x020000, CRC(f10bb216) SHA1(d904030fbb838d906ca69a77cffe286e903b273d) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "sa_b_12.bin", 0x000000, 0x004000, CRC(f358175d) SHA1(781d0f846217aa71e3c6d73c1d63bd87d1fa6b48) ) // (c)1992 Imagetek (1xxxxxxxxxxxxxxxx = 0xFF)
	ROM_CONTINUE(            0x010000, 0x01c000 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "sa_a_02.bin", 0x000000, 0x040000, CRC(f4f81d41) SHA1(85e587b4fda71fa5b944b0ac158d36c00e290f5f) , ROM_SKIP(7))
	ROMX_LOAD( "sa_a_04.bin", 0x000001, 0x040000, CRC(7d071e7e) SHA1(24b9b0cb7e9f719259b0444ee896bdc1ad79a28d) , ROM_SKIP(7))
	ROMX_LOAD( "sa_a_06.bin", 0x000002, 0x040000, CRC(77e4d5e1) SHA1(420e5aaf187e297b371830ebd5787675cff6177b) , ROM_SKIP(7))
	ROMX_LOAD( "sa_a_08.bin", 0x000003, 0x040000, CRC(f2a5a093) SHA1(66d482cc3f45ff7bf1363cf3c88e2dabc902a299) , ROM_SKIP(7))
	ROMX_LOAD( "sa_a_01.bin", 0x000004, 0x040000, CRC(41ec6491) SHA1(c0bd66409bc6ea969f4c45cc006fde891ba8b4d7) , ROM_SKIP(7))
	ROMX_LOAD( "sa_a_03.bin", 0x000005, 0x040000, CRC(e0dff10d) SHA1(3aa18b05f06b4b0a88ba4df86dfc0ca650c2684e) , ROM_SKIP(7))
	ROMX_LOAD( "sa_a_05.bin", 0x000006, 0x040000, CRC(62169d31) SHA1(294887b6ce0d56e053e7f7583b8a160afeef4ce5) , ROM_SKIP(7))
	ROMX_LOAD( "sa_a_07.bin", 0x000007, 0x040000, CRC(a6f5966f) SHA1(00319b96dacc4dcfd70935e1626da0ae6aa63e5a) , ROM_SKIP(7))

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sa_a_11.bin", 0x000000, 0x020000, CRC(04842a60) SHA1(ade016c85867dee7ac27efe3910b01f5f8e730a0) )
ROM_END


/***************************************************************************

Toride II Adauchi Gaiden (c)1994 Metro corp
MTR5260-A

CPU  : TMP68HC000P-16
Sound: D78C10ACW M6295 YM2413
OSC  : 24.0000MHz  (OSC1)
                   (OSC2)
       26.6660MHz  (OSC3)
       3.579545MHz (OSC4)
                   (OSC5)

ROMs:
tr2aja-1.12i - Graphics (27c4096)
tr2aja-2.14i |
tr2aja-3.16i |
tr2aja-4.18i /

tr2aja-5.20e - Main programs (27c020)
tr2aja-6.20c /

tr2aja-7.3g - Sound data (27c010)
tr2aja-8.3i - Sound program (27c010)

Others:
Imagetek I4220 071 9338EK700 (208pin PQFP)
AMD MACH110-20 (CPLD)

***************************************************************************/

ROM_START( toride2g )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "tr2aja-5.20e", 0x000000, 0x040000, CRC(b96a52f6) SHA1(353b5599d50d96b96bdd6352c046ad669cf8da44) )
	ROM_LOAD16_BYTE( "tr2aja-6.20c", 0x000001, 0x040000, CRC(2918b6b4) SHA1(86ebb884759dc9a8a701784d19845467aa1ce11b) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "tr2aja-8.3i", 0x000000, 0x004000, CRC(fdd29146) SHA1(8e996e1afd33f16d35ebf5a40829feb3e92f781f) )
	ROM_CONTINUE(            0x010000, 0x01c000 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "tr2aja-2.14i", 0x000000, 0x080000, CRC(5c73f629) SHA1(b38b7ee213bcc0dd5e4c339a8f9f2fdd81ede6ad) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "tr2aja-4.18i", 0x000002, 0x080000, CRC(67ebaf1b) SHA1(a0c5f253cc33620251fb58ef6f1647453d778462) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "tr2aja-1.12i", 0x000004, 0x080000, CRC(96245a5c) SHA1(524990c88a08648de6f330652fc5c02a27e1325c) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "tr2aja-3.16i", 0x000006, 0x080000, CRC(49013f5d) SHA1(8f29bd2606b30260e9b21886f2b257f7ae8fb2bf) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "tr2aja-7.3g", 0x000000, 0x020000, CRC(630c6193) SHA1(ddb63724e0b0f7264cb02904e49b24b87beb35a9) )
ROM_END

ROM_START( toride2gg )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "trii_ge_5.20e", 0x000000, 0x040000, CRC(5e0815a8) SHA1(574c1bf1149b7e98222876b402b20d824f207c79) )
	ROM_LOAD16_BYTE( "trii_ge_6.20c", 0x000001, 0x040000, CRC(55eba67d) SHA1(c12a11a98d49baf3643404a594d2b87b434acb01) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "tr2_jb-8.3i", 0x000000, 0x004000, CRC(0168f46f) SHA1(01bf4cc425d72936897c3c572f6c0b1366fe4041) )
	ROM_CONTINUE(            0x010000, 0x01c000 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "trii_gb_2.14i", 0x000000, 0x080000, CRC(5949e65f) SHA1(f51ff9590904e691b9ec91b22d3c52bf579deaff) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "trii_gb_4.18i", 0x000002, 0x080000, CRC(adc84c7b) SHA1(fe0f2b6e3c586c427701e43fdd4827c8b183b42a) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "trii_gb_1.12i", 0x000004, 0x080000, CRC(bcf30944) SHA1(c36fbffa6062a2443a47d8faf83baa903529ee97) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "trii_gb_3.16i", 0x000006, 0x080000, CRC(138e68d0) SHA1(5a9655f31e2f2e2f16a5bdc334efa78b2cfc37d2) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "tr2_ja_7.3g", 0x000000, 0x020000, CRC(6ee32315) SHA1(ef4d59576929deab0aa459a67be21d97c2803dea) )
ROM_END

/***************************************************************************

Toride II Bok Su Oi Jeon Adauchi Gaiden
(C)1994 Metro (Yu Jin?)

MTR5260-A
|----------------------------------------------|
| TA7222       YM2413  3.579545MHz  *12MHz     |
|VOLUME  *UPC3403 *YM3012                 6116 |
|UPC3403             *YM2151       ROM7   ROM8 |
|       26.824MHz          M6295               |
|                    |--------|          78C10 |
|         62256      |IMAGETEK|                |
|         62256      |I4220   |                |
|J                   |        |                |
|A        6264       |        |          ROM1  |
|M                   |--------|                |
|M                                             |
|A           MB3771                      ROM2  |
|          |---------|                         |
|          |68000-12 |                         |
|          |---------|                         |
|                     6264               ROM3  |
|                                              |
|     MACH110  24MHz  6264                     |
|SW1                                     ROM4  |
|                                              |
|SW2       ROM6       ROM5                     |
|----------------------------------------------|
* = Not populated on Toride II
MB3771 == MM1035 used on other MTR5260-A PCBs

info by Guru

***************************************************************************/

ROM_START( toride2gk )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "5", 0x00000, 0x40000, CRC(7e3f943a) SHA1(d9f36ee85ad8ae562433e0173562ededf6c6f3e4) )
	ROM_LOAD16_BYTE( "6", 0x00001, 0x40000, CRC(92726910) SHA1(529644fb8e4ea8df0dde617afd3e274821513ab4) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "8", 0x00000, 0x04000, CRC(fdd29146) SHA1(8e996e1afd33f16d35ebf5a40829feb3e92f781f) )
	ROM_CONTINUE(  0x10000, 0x1c000 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "2", 0x00000, 0x80000, CRC(5e7fb9db) SHA1(37094ea750be8605bd2130d0d5ce5f9c43b0cc77), ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "4", 0x00002, 0x80000, CRC(558c03e7) SHA1(f7fa5aa9eacd8953d998d9b05d5f03e65056bd78), ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "1", 0x00004, 0x80000, CRC(5e819ccd) SHA1(b1d4e800bac0f55286317d2a39c2b245d87a3e50), ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "3", 0x00006, 0x80000, CRC(24029583) SHA1(6e03db0a9835a8cf5c565d10794e8b01c919a679), ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "7", 0x00000, 0x20000, CRC(630c6193) SHA1(ddb63724e0b0f7264cb02904e49b24b87beb35a9) )
ROM_END

ROM_START( toride2j )
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "tr2_jk-5.20e", 0x000000, 0x040000, CRC(f2668578) SHA1(1dd18a5597efb25c937697b50fb1262f50580a63) )
	ROM_LOAD16_BYTE( "tr2_jk-6.20c", 0x000001, 0x040000, CRC(4c87629f) SHA1(5fde8580bedb783491ee87ecfe4b1c22d0c9f716) )

	ROM_REGION( 0x02c000, "audiocpu", 0 )       /* NEC78C10 Code */
	ROM_LOAD( "tr2_jb-8.3i", 0x000000, 0x004000, CRC(0168f46f) SHA1(01bf4cc425d72936897c3c572f6c0b1366fe4041) )
	ROM_CONTINUE(            0x010000, 0x01c000 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Gfx + Data (Addressable by CPU & Blitter) */
	ROMX_LOAD( "tr2_jb-2.14i", 0x000000, 0x080000, CRC(b31754dc) SHA1(be2423bafbf07c93c3d222e907190b44616014f0) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "tr2_jb-4.18i", 0x000002, 0x080000, CRC(a855c3fa) SHA1(eca3e235256df7e6ae66ecbe43bc0edb974af503) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "tr2_jb-1.12i", 0x000004, 0x080000, CRC(856f40b7) SHA1(99aca5472b991cd08e9c2128ffdd40675a3b968d) , ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "tr2_jb-3.16i", 0x000006, 0x080000, CRC(78ba205f) SHA1(1069a362e60747aaa284c0d9bb7718013df347f3) , ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "tr2_ja_7.3g", 0x000000, 0x020000, CRC(6ee32315) SHA1(ef4d59576929deab0aa459a67be21d97c2803dea) )
ROM_END

/***************************************************************************

Varia Metal
Excellent System Ltd, 1995

PCB Layout
----------
This game runs on Metro hardware.

ES-9309B-B
|--------------------------------------------|
| TA7222   8.U9     DSW1(8)  DSW2(8)         |
|VOL    M6295  1.000MHz                      |
|                   |------------|           |
|          7.U12    |   68000    |           |
|  uPC3403          |------------|           |
|J 640kHz  ES-8712                           |
|A M6585           EPM7032    6B.U18  5B.U19 |
|M       MM1035                              |
|M        26.666MHz  16MHz    62256   62256  |
|A                                           |
|                 |--------|          1.U29  |
|         62256   |Imagetek|                 |
|         62256   |I4220   |          2.U31  |
|                 |        |                 |
|                 |--------|          3.U28  |
|                                            |
|                  6264               4.U30  |
|--------------------------------------------|
Notes:
      68000   - clock 16.000MHz
      ES-8712 - ES-8712 Single Channel ADPCM Samples Player. Clock ?? seems to be 16kHz?
                This chip is branded 'EXCELLENT', may be (or not??) manufactured by Ensonic (SDIP48)
      M6295   - clock 1.000MHz. Sample rate = 1000000/132
      M6585   - Oki M6585 ADPCM Voice Synthesizer IC (DIP18). Clock 640kHz.
                Sample rate = 16kHz (selection - pin 1 LOW, pin 2 HIGH = 16kHz)
                This is a version-up to the previous M5205 with some additional
                capabilies and improvements.
      MM1035  - Mitsumi Monolithic IC MM1035 System Reset and Watchdog Timer (DIP8)
      uPC3403 - NEC uPC3403 High Performance Quad Operational Amplifier (DIP14)
      62256   - 32k x8 SRAM (DIP28)
      6264    - 8k x8 SRAM (DIP28)
      TA7222  - Toshiba TA7222 5.8 Watt Audio Power Amplifier (SIP10)
      EPM7032 - Altera EPM7032LC44-15T High Performance EEPROM-based Programmable Logic Device (PLCC44)
      Custom  - Imagetek I4220 Graphics Controller (QFP208)
      VSync   - 58.2328Hz
      HSync   - 15.32kHz
      ROMs    -
                6B & 5B are 27C040 EPROM (DIP32)
                8 is 4M MaskROM (DIP32)
                All other ROMs are 16M MaskROM (DIP42)

***************************************************************************/

ROM_START( vmetal )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "6b.u18", 0x00000, 0x80000, CRC(4eb939d5) SHA1(741ab05043fc3bd886162d878630e45da9359718) )
	ROM_LOAD16_BYTE( "5b.u19", 0x00001, 0x80000, CRC(4933ac6c) SHA1(1a3303e32fcb08854d4d6e13f36ca99d92aed4cc) )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROMX_LOAD( "2.u31", 0x000000, 0x200000, CRC(b36f8d60) SHA1(1676859d0fee4eb9897ce1601a2c9fd9a6dc4a43), ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "4.u30", 0x000002, 0x200000, CRC(5a25a49c) SHA1(c30781202ec882e1ec6adfb560b0a1075b3cce55), ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "1.u29", 0x000004, 0x200000, CRC(b470c168) SHA1(c30462dc134da1e71a94b36ef96ecd65c325b07e), ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "3.u28", 0x000006, 0x200000, CRC(00fca765) SHA1(ca9010bd7f59367e483868018db9a9abf871386e), ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x080000, "oki", 0 ) /* OKI6295 Samples */
	/* Second half is junk */
	ROM_LOAD( "8.u9", 0x00000, 0x80000, CRC(c14c001c) SHA1(bad96b5cd40d1c34ef8b702262168ecab8192fb6) )

	ROM_REGION( 0x200000, "essnd", 0 ) /* Samples */
	ROM_LOAD( "7.u12", 0x00000, 0x200000, CRC(a88c52f1) SHA1(d74a5a11f84ba6b1042b33a2c156a1071b6fbfe1) )
ROM_END

ROM_START( vmetaln )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "vm6.bin", 0x00000, 0x80000, CRC(cb292ab1) SHA1(41fdfe67e6cb848542fd5aa0dfde3b1936bb3a28) )
	ROM_LOAD16_BYTE( "vm5.bin", 0x00001, 0x80000, CRC(43ef844e) SHA1(c673f34fcc9e406282c9008795b52d01a240099a) )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROMX_LOAD( "2.u31", 0x000000, 0x200000, CRC(b36f8d60) SHA1(1676859d0fee4eb9897ce1601a2c9fd9a6dc4a43), ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "4.u30", 0x000002, 0x200000, CRC(5a25a49c) SHA1(c30781202ec882e1ec6adfb560b0a1075b3cce55), ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "1.u29", 0x000004, 0x200000, CRC(b470c168) SHA1(c30462dc134da1e71a94b36ef96ecd65c325b07e), ROM_GROUPWORD | ROM_SKIP(6))
	ROMX_LOAD( "3.u28", 0x000006, 0x200000, CRC(00fca765) SHA1(ca9010bd7f59367e483868018db9a9abf871386e), ROM_GROUPWORD | ROM_SKIP(6))

	ROM_REGION( 0x080000, "oki", 0 ) /* OKI6295 Samples */
	/* Second half is junk */
	ROM_LOAD( "8.u9", 0x00000, 0x80000, CRC(c14c001c) SHA1(bad96b5cd40d1c34ef8b702262168ecab8192fb6) )

	ROM_REGION( 0x200000, "essnd", 0 ) /* Samples */
	ROM_LOAD( "7.u12", 0x00000, 0x200000, CRC(a88c52f1) SHA1(d74a5a11f84ba6b1042b33a2c156a1071b6fbfe1) )
ROM_END


/***************************************************************************


                                Driver Inits


***************************************************************************/

void metro_state::metro_common(  )
{
	memset(m_requested_int, 0, ARRAY_LENGTH(m_requested_int));
	m_vblank_bit = 0;
	m_blitter_bit = 2;
	m_irq_line = 2;

	*m_irq_enable = 0;
}


DRIVER_INIT_MEMBER(metro_state,metro)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	metro_common();

	m_porta = 0x00;
	m_portb = 0x00;
	m_busy_sndcpu = 0;
	metro_sound_rombank_w(space, 0, 0x00);
}

DRIVER_INIT_MEMBER(metro_state,karatour)
{
	m_vram_0.allocate(0x20000/2);
	m_vram_1.allocate(0x20000/2);
	m_vram_2.allocate(0x20000/2);
	for (int i = 0; i < 0x20000 / 2; i++)
	{
		m_vram_0[i] = machine().rand();
		m_vram_1[i] = machine().rand();
		m_vram_2[i] = machine().rand();
	}

	DRIVER_INIT_CALL(metro);
}

DRIVER_INIT_MEMBER(metro_state,daitorid)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	metro_common();

	m_porta = 0x00;
	m_portb = 0x00;
	m_busy_sndcpu = 0;
	daitorid_sound_rombank_w(space, 0, 0x00);
}


/* Unscramble the GFX ROMs */
DRIVER_INIT_MEMBER(metro_state,balcube)
{
	UINT8 *ROM         = memregion("gfx1")->base();
	const unsigned len = memregion("gfx1")->bytes();

	for (unsigned i = 0; i < len; i+=2)
	{
		ROM[i]  = BITSWAP8(ROM[i],0,1,2,3,4,5,6,7);
	}

	metro_common();
	m_irq_line = 1;
}


DRIVER_INIT_MEMBER(metro_state,dharmak)
{
	UINT8 *src = memregion( "gfx1" )->base();
	int i;

	for (i = 0; i < 0x200000; i += 4)
	{
		UINT8 dat;
		dat = src[i + 1];
		dat = BITSWAP8(dat, 7,3,2,4, 5,6,1,0);
		src[i + 1] = dat;

		dat = src[i + 3];
		dat = BITSWAP8(dat, 7,2,5,4, 3,6,1,0);
		src[i + 3] = dat;
	}

	DRIVER_INIT_CALL(metro);
}

DRIVER_INIT_MEMBER(metro_state,blzntrnd)
{
	metro_common();
	m_irq_line = 1;
}

DRIVER_INIT_MEMBER(metro_state,mouja)
{
	metro_common();
	m_irq_line = -1;    /* split interrupt handlers */
	m_vblank_bit = 1;
	m_mouja_irq_timer = timer_alloc(TIMER_MOUJA_IRQ);
	membank("okibank")->configure_entries(0, 8, memregion("oki")->base(), 0x20000);
}

DRIVER_INIT_MEMBER(metro_state,gakusai)
{
	metro_common();
	m_irq_line = -1;
	m_vblank_bit = 1;
	m_blitter_bit = 3;
}

DRIVER_INIT_MEMBER(metro_state,puzzlet)
{
	metro_common();
	m_irq_line = 0;
	m_vblank_bit = 1;
	m_blitter_bit = 3;
}

/***************************************************************************


                                Game Drivers


***************************************************************************/

GAME( 1992, karatour,  0,        karatour, karatour, metro_state, karatour, ROT0,   "Mitchell",                                        "The Karate Tournament",                  MACHINE_SUPPORTS_SAVE )
GAME( 1992, karatourj, karatour, karatour, karatour, metro_state, karatour, ROT0,   "Mitchell",                                        "The Karate Tournament (Japan)",          MACHINE_SUPPORTS_SAVE )
GAME( 1992, pangpoms,  0,        pangpoms, pangpoms, metro_state, metro,    ROT0,   "Metro",                                           "Pang Pom's",                             MACHINE_SUPPORTS_SAVE )
GAME( 1992, pangpomsm, pangpoms, pangpoms, pangpoms, metro_state, metro,    ROT0,   "Metro (Mitchell license)",                        "Pang Pom's (Mitchell)",                  MACHINE_SUPPORTS_SAVE )
GAME( 1992, skyalert,  0,        skyalert, skyalert, metro_state, metro,    ROT270, "Metro",                                           "Sky Alert",                              MACHINE_SUPPORTS_SAVE )
GAME( 1993, ladykill,  0,        karatour, ladykill, metro_state, karatour, ROT90,  "Yanyaka (Mitchell license)",                      "Lady Killer",                            MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1993, moegonta,  ladykill, karatour, moegonta, metro_state, karatour, ROT90,  "Yanyaka",                                         "Moeyo Gonta!! (Japan)",                  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1993, poitto,    0,        poitto,   poitto,   metro_state, metro,    ROT0,   "Metro / Able Corp.",                              "Poitto!",                                MACHINE_SUPPORTS_SAVE )
GAME( 1994, blzntrnd,  0,        blzntrnd, blzntrnd, metro_state, blzntrnd, ROT0,   "Human Amusement",                                 "Blazing Tornado",                        MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, dharma,    0,        dharma,   dharma,   metro_state, dharmak,  ROT0,   "Metro",                                           "Dharma Doujou",                          MACHINE_SUPPORTS_SAVE )
GAME( 1994, dharmaj,   dharma,   dharma,   dharma,   metro_state, metro,    ROT0,   "Metro",                                           "Dharma Doujou (Japan)",                  MACHINE_SUPPORTS_SAVE )
GAME( 1994, dharmak,   dharma,   dharma,   dharma,   metro_state, dharmak,  ROT0,   "Metro",                                           "Dharma Doujou (Korea)",                  MACHINE_SUPPORTS_SAVE )
GAME( 1994, lastfort,  0,        lastfort, lastfort, metro_state, metro,    ROT0,   "Metro",                                           "Last Fortress - Toride",                 MACHINE_SUPPORTS_SAVE )
GAME( 1994, lastforte, lastfort, lastfort, lastfero, metro_state, metro,    ROT0,   "Metro",                                           "Last Fortress - Toride (Erotic, Rev C)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, lastfortea,lastfort, lastfort, lastfero, metro_state, metro,    ROT0,   "Metro",                                           "Last Fortress - Toride (Erotic, Rev A)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, lastfortk, lastfort, lastfort, lastfero, metro_state, metro,    ROT0,   "Metro",                                           "Last Fortress - Toride (Korea)",         MACHINE_SUPPORTS_SAVE )
GAME( 1994, lastfortg, lastfort, lastforg, ladykill, metro_state, metro,    ROT0,   "Metro",                                           "Last Fortress - Toride (German)",        MACHINE_SUPPORTS_SAVE )
GAME( 1994, toride2g,  0,        toride2g, toride2g, metro_state, metro,    ROT0,   "Metro",                                           "Toride II Adauchi Gaiden",               MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, toride2gg, toride2g, toride2g, toride2g, metro_state, metro,    ROT0,   "Metro",                                           "Toride II Adauchi Gaiden (German)",      MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, toride2gk, toride2g, toride2g, toride2g, metro_state, metro,    ROT0,   "Metro",                                           "Toride II Bok Su Oi Jeon Adauchi Gaiden (Korea)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, toride2j,  toride2g, toride2g, toride2g, metro_state, metro,    ROT0,   "Metro",                                           "Toride II (Japan)",                      MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, gunmast,   0,        pururun,  gunmast,  metro_state, daitorid, ROT0,   "Metro",                                           "Gun Master",                             MACHINE_SUPPORTS_SAVE )
GAME( 1995, daitorid,  0,        daitorid, daitorid, metro_state, daitorid, ROT0,   "Metro",                                           "Daitoride",                              MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1996, daitorida, daitorid, daitoa,   daitorid, metro_state, balcube,  ROT0,   "Metro",                                           "Daitoride (YMF278B version)",            MACHINE_SUPPORTS_SAVE )
GAME( 1995, dokyusei,  0,        dokyusei, dokyusei, metro_state, gakusai,  ROT0,   "Make Software / Elf / Media Trading",             "Mahjong Doukyuusei",                     MACHINE_SUPPORTS_SAVE )
GAME( 1995, dokyusp,   0,        dokyusp,  gakusai,  metro_state, gakusai,  ROT0,   "Make Software / Elf / Media Trading",             "Mahjong Doukyuusei Special",             MACHINE_SUPPORTS_SAVE )
GAME( 1995, msgogo,    0,        msgogo,   msgogo,   metro_state, balcube,  ROT0,   "Metro",                                           "Mouse Shooter GoGo",                     MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1995, pururun,   0,        pururun,  pururun,  metro_state, daitorid, ROT0,   "Metro / Banpresto",                               "Pururun",                                MACHINE_SUPPORTS_SAVE )
GAME( 1995, puzzli,    0,        daitorid, puzzli,   metro_state, daitorid, ROT0,   "Metro / Banpresto",                               "Puzzli",                                 MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1996, 3kokushi,  0,        3kokushi, 3kokushi, metro_state, karatour, ROT0,   "Mitchell",                                        "Sankokushi (Japan)",                     MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1996, balcube,   0,        balcube,  balcube,  metro_state, balcube,  ROT0,   "Metro",                                           "Bal Cube",                               MACHINE_SUPPORTS_SAVE )
GAME( 1996, bangball,  0,        bangball, bangball, metro_state, balcube,  ROT0,   "Banpresto / Kunihiko Tashiro+Goodhouse",          "Bang Bang Ball (v1.05)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1996, gstrik2,   0,        gstrik2,  gstrik2,  metro_state, blzntrnd, ROT0,   "Human Amusement",                                 "Grand Striker 2 (Europe and Oceania)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1996, gstrik2j,  gstrik2,  gstrik2,  gstrik2,  metro_state, blzntrnd, ROT0,   "Human Amusement",                                 "Grand Striker 2 (Japan)",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // priority between rounds
GAME( 1999, batlbubl,  bangball, batlbubl, batlbubl, metro_state, balcube,  ROT0,   "Banpresto (Limenko license?)",                    "Battle Bubble (v2.00)",                  MACHINE_SUPPORTS_SAVE ) // or bootleg?
GAME( 1996, mouja,     0,        mouja,    mouja,    metro_state, mouja,    ROT0,   "Etona",                                           "Mouja (Japan)",                          MACHINE_SUPPORTS_SAVE )
GAME( 1997, gakusai,   0,        gakusai,  gakusai,  metro_state, gakusai,  ROT0,   "MakeSoft",                                        "Mahjong Gakuensai (Japan)",              MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1998, gakusai2,  0,        gakusai2, gakusai,  metro_state, gakusai,  ROT0,   "MakeSoft",                                        "Mahjong Gakuensai 2 (Japan)",            MACHINE_SUPPORTS_SAVE )
GAME( 2000, puzzlet,   0,        puzzlet,  puzzlet,  metro_state, puzzlet,  ROT0,   "Unies Corporation",                               "Puzzlet (Japan)",                        MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1995, vmetal,    0,        vmetal,   vmetal,   metro_state, blzntrnd, ROT90,  "Excellent System",                                "Varia Metal",                            MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1995, vmetaln,   vmetal,   vmetal,   vmetal,   metro_state, blzntrnd, ROT90,  "Excellent System (New Ways Trading Co. license)", "Varia Metal (New Ways Trading Co.)",     MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
