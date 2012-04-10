/***************************************************************************

    Raster Elite Tickee Tickats hardware
     + variations

    driver by Aaron Giles

    Games supported:
        * Tickee Tickats
        * Ghost Hunter
        * Tuts Tomb
        * Mouse Attack
        * Rapid Fire
        * Mallet Madness

    Known bugs:
        * (Tickee) gun sometimes misfires
        * Mouse Attack dips and inputs need fixing
        * Mallet Madness ticket dispenser isn't working

***************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "machine/ticket.h"
#include "video/tlc34076.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "machine/nvram.h"


class tickee_state : public driver_device
{
public:
	tickee_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_control;
	UINT16 *m_vram;
	emu_timer *m_setup_gun_timer;
	int m_beamxadd;
	int m_beamyadd;
	int m_palette_bank;
	UINT8 m_gunx[2];
	DECLARE_WRITE16_MEMBER(rapidfir_transparent_w);
	DECLARE_READ16_MEMBER(rapidfir_transparent_r);
	DECLARE_WRITE16_MEMBER(tickee_control_w);
	DECLARE_READ16_MEMBER(ffff_r);
	DECLARE_READ16_MEMBER(rapidfir_gun1_r);
	DECLARE_READ16_MEMBER(rapidfir_gun2_r);
	DECLARE_READ16_MEMBER(ff7f_r);
	DECLARE_WRITE16_MEMBER(ff7f_w);
	DECLARE_WRITE16_MEMBER(rapidfir_control_w);
};


#define CPU_CLOCK			XTAL_40MHz
#define VIDEO_CLOCK			XTAL_14_31818MHz
#define OKI_CLOCK			XTAL_1MHz


/*************************************
 *
 *  Compute X/Y coordinates
 *
 *************************************/

INLINE void get_crosshair_xy(running_machine &machine, int player, int *x, int *y)
{
	const rectangle &visarea = machine.primary_screen->visible_area();

	*x = (((input_port_read(machine, player ? "GUNX2" : "GUNX1") & 0xff) * visarea.width()) >> 8) + visarea.min_x;
	*y = (((input_port_read(machine, player ? "GUNY2" : "GUNY1") & 0xff) * visarea.height()) >> 8) + visarea.min_y;
}



/*************************************
 *
 *  Light gun interrupts
 *
 *************************************/

static TIMER_CALLBACK( trigger_gun_interrupt )
{
	tickee_state *state = machine.driver_data<tickee_state>();
	int which = param & 1;
	int beamx = (machine.primary_screen->hpos()/2)-58;

	/* once we're ready to fire, set the X coordinate and assert the line */
	state->m_gunx[which] = beamx;

	/* fire the IRQ at the correct moment */
	cputag_set_input_line(machine, "maincpu", param, ASSERT_LINE);
}


static TIMER_CALLBACK( clear_gun_interrupt )
{
	/* clear the IRQ on the next scanline? */
	cputag_set_input_line(machine, "maincpu", param, CLEAR_LINE);
}


static TIMER_CALLBACK( setup_gun_interrupts )
{
	tickee_state *state = machine.driver_data<tickee_state>();
	int beamx, beamy;

	/* set a timer to do this again next frame */
	state->m_setup_gun_timer->adjust(machine.primary_screen->time_until_pos(0));

	/* only do work if the palette is flashed */
	if (state->m_control)
		if (!state->m_control[2])
			return;

	/* generate interrupts for player 1's gun */
	get_crosshair_xy(machine, 0, &beamx, &beamy);
	machine.scheduler().timer_set(machine.primary_screen->time_until_pos(beamy + state->m_beamyadd, beamx + state->m_beamxadd), FUNC(trigger_gun_interrupt), 0);
	machine.scheduler().timer_set(machine.primary_screen->time_until_pos(beamy + state->m_beamyadd + 1, beamx + state->m_beamxadd), FUNC(clear_gun_interrupt), 0);

	/* generate interrupts for player 2's gun */
	get_crosshair_xy(machine, 1, &beamx, &beamy);
	machine.scheduler().timer_set(machine.primary_screen->time_until_pos(beamy + state->m_beamyadd, beamx + state->m_beamxadd), FUNC(trigger_gun_interrupt), 1);
	machine.scheduler().timer_set(machine.primary_screen->time_until_pos(beamy + state->m_beamyadd + 1, beamx + state->m_beamxadd), FUNC(clear_gun_interrupt), 1);
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

static VIDEO_START( tickee )
{
	tickee_state *state = machine.driver_data<tickee_state>();
	/* start a timer going on the first scanline of every frame */
	state->m_setup_gun_timer = machine.scheduler().timer_alloc(FUNC(setup_gun_interrupts));
	state->m_setup_gun_timer->adjust(machine.primary_screen->time_until_pos(0));
}



/*************************************
 *
 *  Video update
 *
 *************************************/

static void scanline_update(screen_device &screen, bitmap_rgb32 &bitmap, int scanline, const tms34010_display_params *params)
{
	tickee_state *state = screen.machine().driver_data<tickee_state>();
	UINT16 *src = &state->m_vram[(params->rowaddr << 8) & 0x3ff00];
	UINT32 *dest = &bitmap.pix32(scanline);
	const rgb_t *pens = tlc34076_get_pens(screen.machine().device("tlc34076"));
	int coladdr = params->coladdr << 1;
	int x;

	/* blank palette: fill with pen 255 */
	if (state->m_control[2])
	{
		for (x = params->heblnk; x < params->hsblnk; x++)
			dest[x] = pens[0xff];
	}
	else
		/* copy the non-blanked portions of this scanline */
		for (x = params->heblnk; x < params->hsblnk; x += 2)
		{
			UINT16 pixels = src[coladdr++ & 0xff];
			dest[x + 0] = pens[pixels & 0xff];
			dest[x + 1] = pens[pixels >> 8];
		}
}


static void rapidfir_scanline_update(screen_device &screen, bitmap_rgb32 &bitmap, int scanline, const tms34010_display_params *params)
{
	tickee_state *state = screen.machine().driver_data<tickee_state>();
	UINT16 *src = &state->m_vram[(params->rowaddr << 8) & 0x3ff00];
	UINT32 *dest = &bitmap.pix32(scanline);
	const rgb_t *pens = tlc34076_get_pens(screen.machine().device("tlc34076"));
	int coladdr = params->coladdr << 1;
	int x;

	if (state->m_palette_bank)
	{
		/* blank palette: fill with pen 255 */
		for (x = params->heblnk; x < params->hsblnk; x += 2)
		{
			dest[x + 0] = pens[0xff];
			dest[x + 1] = pens[0xff];
		}
	}
	else
	{
		/* copy the non-blanked portions of this scanline */
		for (x = params->heblnk; x < params->hsblnk; x += 2)
		{
			UINT16 pixels = src[coladdr++ & 0xff];
			dest[x + 0] = pens[pixels & 0xff];
			dest[x + 1] = pens[pixels >> 8];
		}
	}
}


/*************************************
 *
 *  Machine init
 *
 *************************************/

static MACHINE_RESET( tickee )
{
	tickee_state *state = machine.driver_data<tickee_state>();
	state->m_beamxadd = 50;
	state->m_beamyadd = 0;
}

static MACHINE_RESET( rapidfir )
{
	tickee_state *state = machine.driver_data<tickee_state>();
	state->m_beamxadd = 0;
	state->m_beamyadd = -5;
}


/*************************************
 *
 *  Video related
 *
 *************************************/

WRITE16_MEMBER(tickee_state::rapidfir_transparent_w)
{
	if (!(data & 0xff00)) mem_mask &= 0x00ff;
	if (!(data & 0x00ff)) mem_mask &= 0xff00;
	COMBINE_DATA(&m_vram[offset]);
}


READ16_MEMBER(tickee_state::rapidfir_transparent_r)
{
	return m_vram[offset];
}


static void rapidfir_to_shiftreg(address_space *space, UINT32 address, UINT16 *shiftreg)
{
	tickee_state *state = space->machine().driver_data<tickee_state>();
	if (address < 0x800000)
		memcpy(shiftreg, &state->m_vram[TOWORD(address)], TOBYTE(0x2000));
}


static void rapidfir_from_shiftreg(address_space *space, UINT32 address, UINT16 *shiftreg)
{
	tickee_state *state = space->machine().driver_data<tickee_state>();
	if (address < 0x800000)
		memcpy(&state->m_vram[TOWORD(address)], shiftreg, TOBYTE(0x2000));
}



/*************************************
 *
 *  Miscellaneous control bits
 *
 *************************************/

WRITE16_MEMBER(tickee_state::tickee_control_w)
{
	UINT16 olddata = m_control[offset];

	/* offsets:

        2 = palette flash (0 normally, 1 when trigger is pressed)
        3 = ticket motor (bit 3 = 0 for left motor, bit 2 = 0 for right motor)
        6 = lamps? (changing all the time)
    */

	COMBINE_DATA(&m_control[offset]);

	if (offset == 3)
	{
		machine().device<ticket_dispenser_device>("ticket1")->write(space, 0, (data & 8) << 4);
		machine().device<ticket_dispenser_device>("ticket2")->write(space, 0, (data & 4) << 5);
	}

	if (olddata != m_control[offset])
		logerror("%08X:tickee_control_w(%d) = %04X (was %04X)\n", cpu_get_pc(&space.device()), offset, m_control[offset], olddata);
}



/*************************************
 *
 *  Unknowns
 *
 *************************************/

READ16_MEMBER(tickee_state::ffff_r)
{
	return 0xffff;
}


READ16_MEMBER(tickee_state::rapidfir_gun1_r)
{
	return m_gunx[0];
}


READ16_MEMBER(tickee_state::rapidfir_gun2_r)
{
	return m_gunx[1];
}


READ16_MEMBER(tickee_state::ff7f_r)
{
	/* Ticket dispenser status? */
	return 0xff7f;
}

WRITE16_MEMBER(tickee_state::ff7f_w)
{
	/* Ticket dispenser output? */
}

WRITE16_MEMBER(tickee_state::rapidfir_control_w)
{
	/* other bits like control on tickee? */
	if (ACCESSING_BITS_0_7)
		m_palette_bank = data & 1;
}



/*************************************
 *
 *  Sound
 *
 *************************************/

static WRITE16_DEVICE_HANDLER( sound_bank_w )
{
	switch (data & 0xff)
	{
		case 0x2c:
			downcast<okim6295_device *>(device)->set_bank_base(0x00000);
			break;

		case 0x2d:
			downcast<okim6295_device *>(device)->set_bank_base(0x40000);
			break;

		case 0x1c:
			downcast<okim6295_device *>(device)->set_bank_base(0x80000);
			break;

		case 0x1d:
			downcast<okim6295_device *>(device)->set_bank_base(0xc0000);
			break;

		default:
			logerror("sound_bank_w %04X %04X\n", data, mem_mask);
			break;
	}
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( tickee_map, AS_PROGRAM, 16, tickee_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_BASE(m_vram)
	AM_RANGE(0x02000000, 0x02ffffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x04000000, 0x04003fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x04100000, 0x041000ff) AM_DEVREADWRITE8_LEGACY("tlc34076", tlc34076_r, tlc34076_w, 0x00ff)
	AM_RANGE(0x04200000, 0x0420000f) AM_DEVREAD8_LEGACY("ym1", ay8910_r, 0x00ff)
	AM_RANGE(0x04200000, 0x0420001f) AM_DEVWRITE8_LEGACY("ym1", ay8910_address_data_w, 0x00ff)
	AM_RANGE(0x04200100, 0x0420010f) AM_DEVREAD8_LEGACY("ym2", ay8910_r, 0x00ff)
	AM_RANGE(0x04200100, 0x0420011f) AM_DEVWRITE8_LEGACY("ym2", ay8910_address_data_w, 0x00ff)
	AM_RANGE(0x04400000, 0x0440007f) AM_WRITE(tickee_control_w) AM_BASE(m_control)
	AM_RANGE(0x04400040, 0x0440004f) AM_READ_PORT("IN2")
	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE_LEGACY(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0xc0000240, 0xc000025f) AM_WRITENOP		/* seems to be a bug in their code */
	AM_RANGE(0xff000000, 0xffffffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END


/* addreses in the 04x range shifted slightly...*/
static ADDRESS_MAP_START( ghoshunt_map, AS_PROGRAM, 16, tickee_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_BASE(m_vram)
	AM_RANGE(0x02000000, 0x02ffffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x04100000, 0x04103fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x04200000, 0x042000ff) AM_DEVREADWRITE8_LEGACY("tlc34076", tlc34076_r, tlc34076_w, 0x00ff)
	AM_RANGE(0x04300000, 0x0430000f) AM_DEVREAD8_LEGACY("ym1", ay8910_r, 0x00ff)
	AM_RANGE(0x04300000, 0x0430001f) AM_DEVWRITE8_LEGACY("ym1", ay8910_address_data_w, 0x00ff)
	AM_RANGE(0x04300100, 0x0430010f) AM_DEVREAD8_LEGACY("ym2", ay8910_r, 0x00ff)
	AM_RANGE(0x04300100, 0x0430011f) AM_DEVWRITE8_LEGACY("ym2", ay8910_address_data_w, 0x00ff)
	AM_RANGE(0x04500000, 0x0450007f) AM_WRITE(tickee_control_w) AM_BASE(m_control)
	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE_LEGACY(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0xc0000240, 0xc000025f) AM_WRITENOP		/* seems to be a bug in their code */
	AM_RANGE(0xff000000, 0xffffffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END


static ADDRESS_MAP_START( mouseatk_map, AS_PROGRAM, 16, tickee_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_BASE(m_vram)
	AM_RANGE(0x02000000, 0x02ffffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x04000000, 0x04003fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x04100000, 0x041000ff) AM_DEVREADWRITE8_LEGACY("tlc34076", tlc34076_r, tlc34076_w, 0x00ff)
	AM_RANGE(0x04200000, 0x0420000f) AM_DEVREAD8_LEGACY("ym", ay8910_r, 0x00ff)
	AM_RANGE(0x04200000, 0x0420000f) AM_DEVWRITE8_LEGACY("ym", ay8910_address_data_w, 0x00ff)
	AM_RANGE(0x04200100, 0x0420010f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x04400000, 0x0440007f) AM_WRITE(tickee_control_w) AM_BASE(m_control)
	AM_RANGE(0x04400040, 0x0440004f) AM_READ_PORT("IN2") // ?
	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE_LEGACY(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0xc0000240, 0xc000025f) AM_WRITENOP		/* seems to be a bug in their code */
	AM_RANGE(0xff000000, 0xffffffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END


/* newer hardware */
static ADDRESS_MAP_START( rapidfir_map, AS_PROGRAM, 16, tickee_state )
	AM_RANGE(0x00000000, 0x007fffff) AM_RAM AM_BASE(m_vram)
	AM_RANGE(0x02000000, 0x027fffff) AM_READWRITE(rapidfir_transparent_r, rapidfir_transparent_w)
	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE_LEGACY(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0xfc000000, 0xfc00000f) AM_READ(rapidfir_gun1_r)
	AM_RANGE(0xfc000100, 0xfc00010f) AM_READ(rapidfir_gun2_r)
	AM_RANGE(0xfc000400, 0xfc00040f) AM_READ(ffff_r)
	AM_RANGE(0xfc000500, 0xfc00050f) AM_NOP
	AM_RANGE(0xfc000600, 0xfc00060f) AM_WRITE(rapidfir_control_w)
	AM_RANGE(0xfc000700, 0xfc00070f) AM_DEVWRITE_LEGACY("oki", sound_bank_w)
	AM_RANGE(0xfc000800, 0xfc00080f) AM_READ_PORT("IN0")
	AM_RANGE(0xfc000900, 0xfc00090f) AM_READ_PORT("IN1")
	AM_RANGE(0xfc000a00, 0xfc000a0f) AM_READ_PORT("IN2")
	AM_RANGE(0xfc000b00, 0xfc000b0f) AM_READ_PORT("DSW0")
	AM_RANGE(0xfc000c00, 0xfc000c1f) AM_READ_PORT("DSW1")
	AM_RANGE(0xfc000e00, 0xfc000e1f) AM_READ(watchdog_reset16_r)
	AM_RANGE(0xfc100000, 0xfc1000ff) AM_MIRROR(0x80000) AM_DEVREADWRITE8_LEGACY("tlc34076", tlc34076_r, tlc34076_w, 0x00ff)
	AM_RANGE(0xfc200000, 0xfc207fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xfc300000, 0xfc30000f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0xfc400010, 0xfc40001f) AM_READWRITE(ff7f_r, ff7f_w)
	AM_RANGE(0xfe000000, 0xffffffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( tickee )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, "Game Time/Diff" )
	PORT_DIPSETTING(    0x03, "Very Fast/Very Easy" )
	PORT_DIPSETTING(    0x02, "Fast/Easy" )
	PORT_DIPSETTING(    0x01, "Average/Hard" )
	PORT_DIPSETTING(    0x00, "Slow/Very Hard" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "Last Box Tickets" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "25" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x30, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ))

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("ticket2", ticket_dispenser_device, line_r) /* right ticket status */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("ticket1", ticket_dispenser_device, line_r)	/* left ticket status */
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_SERVICE( 0x0001, IP_ACTIVE_LOW )
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("GUNX1")			/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("GUNY1")			/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START("GUNX2")			/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("GUNY2")			/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( ghoshunt )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Messages in Play")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x02, "Fixed Ticketing")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x04, "Setting")
	PORT_DIPSETTING(    0x04, "Custom")
	PORT_DIPSETTING(    0x00, DEF_STR( Standard ))
	PORT_DIPNAME( 0x08, 0x08, "Messages")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x30, 0x00, "Tickets")
	PORT_DIPSETTING(    0x30, "5")
	PORT_DIPSETTING(    0x20, "10")
	PORT_DIPSETTING(    0x10, "15")
	PORT_DIPSETTING(    0x00, "20")
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ))

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("ticket2", ticket_dispenser_device, line_r)	/* right ticket status */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("ticket1", ticket_dispenser_device, line_r)	/* left ticket status */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xd8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("ticket2", ticket_dispenser_device, line_r)	/* right ticket status */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("ticket1", ticket_dispenser_device, line_r)	/* left ticket status */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xd8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("GUNX1")			/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("GUNY1")			/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START("GUNX2")			/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("GUNY2")			/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( mouseatk )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, "Game Time/Diff" )
	PORT_DIPSETTING(    0x03, "Very Fast/Very Easy" )
	PORT_DIPSETTING(    0x02, "Fast/Easy" )
	PORT_DIPSETTING(    0x01, "Average/Hard" )
	PORT_DIPSETTING(    0x00, "Slow/Very Hard" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "Last Box Tickets" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "25" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x30, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ))

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("ticket2", ticket_dispenser_device, line_r) /* right ticket status */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("ticket1", ticket_dispenser_device, line_r)	/* left ticket status */
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_SERVICE( 0x0001, IP_ACTIVE_LOW )
	PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( rapidfir )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_SERVICE_NO_TOGGLE( 0x0020, IP_ACTIVE_LOW )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ))
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0001, DEF_STR( On ))
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ))
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0002, DEF_STR( On ))
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ))
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0004, DEF_STR( On ))
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ))
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0008, DEF_STR( On ))
	PORT_BIT( 0x0070, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ))
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0080, DEF_STR( On ))
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_BIT( 0x003f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0040, 0x0000, "Reset NVRAM" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ))
	PORT_DIPSETTING(      0x0040, DEF_STR( Yes ))
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("GUNX1")			/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("GUNY1")			/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START("GUNX2")			/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("GUNY2")			/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END



/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

static const ay8910_interface ay8910_interface_1 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW"),
	DEVCB_INPUT_PORT("IN1"),
	DEVCB_NULL,
	DEVCB_NULL
};

static const ay8910_interface ay8910_interface_2 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("IN0"),
	DEVCB_INPUT_PORT("IN2"),
	DEVCB_NULL,
	DEVCB_NULL
};



/*************************************
 *
 *  34010 configuration
 *
 *************************************/

static const tms34010_config tms_config =
{
	FALSE,							/* halt on reset */
	"screen",						/* the screen operated on */
	VIDEO_CLOCK/2,					/* pixel clock */
	1,								/* pixels per clock */
	NULL,							/* scanline callback (indexed16) */
	scanline_update,				/* scanline callback (rgb32) */
	NULL,							/* generate interrupt */
	NULL,							/* write to shiftreg function */
	NULL							/* read from shiftreg function */
};


static const tms34010_config rapidfir_tms_config =
{
	FALSE,							/* halt on reset */
	"screen",						/* the screen operated on */
	VIDEO_CLOCK/2,					/* pixel clock */
	1,								/* pixels per clock */
	NULL,							/* scanline callback (indexed16) */
	rapidfir_scanline_update,		/* scanline callback (rgb32) */
	NULL,							/* generate interrupt */
	rapidfir_to_shiftreg,			/* write to shiftreg function */
	rapidfir_from_shiftreg			/* read from shiftreg function */
};



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( tickee, tickee_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS34010, XTAL_40MHz)
	MCFG_CPU_CONFIG(tms_config)
	MCFG_CPU_PROGRAM_MAP(tickee_map)

	MCFG_MACHINE_RESET(tickee)
	MCFG_NVRAM_ADD_1FILL("nvram")

	MCFG_TICKET_DISPENSER_ADD("ticket1", attotime::from_msec(100), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_HIGH)
	MCFG_TICKET_DISPENSER_ADD("ticket2", attotime::from_msec(100), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_HIGH)

	/* video hardware */
	MCFG_TLC34076_ADD("tlc34076", TLC34076_6_BIT)

	MCFG_VIDEO_START(tickee)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(VIDEO_CLOCK/2, 444, 0, 320, 233, 0, 200)
	MCFG_SCREEN_UPDATE_STATIC(tms340x0_rgb32)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2149, VIDEO_CLOCK/8)
	MCFG_SOUND_CONFIG(ay8910_interface_1)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("ym2", YM2149, VIDEO_CLOCK/8)
	MCFG_SOUND_CONFIG(ay8910_interface_2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( ghoshunt, tickee )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ghoshunt_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( rapidfir, tickee_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS34010, XTAL_50MHz)
	MCFG_CPU_CONFIG(rapidfir_tms_config)
	MCFG_CPU_PROGRAM_MAP(rapidfir_map)

	MCFG_MACHINE_RESET(rapidfir)
	MCFG_NVRAM_ADD_1FILL("nvram")

	/* video hardware */
	MCFG_TLC34076_ADD("tlc34076", TLC34076_6_BIT)

	MCFG_VIDEO_START(tickee)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(VIDEO_CLOCK/2, 444, 0, 320, 233, 0, 200)
	MCFG_SCREEN_UPDATE_STATIC(tms340x0_rgb32)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", OKI_CLOCK, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( mouseatk, tickee_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS34010, XTAL_40MHz)
	MCFG_CPU_CONFIG(tms_config)
	MCFG_CPU_PROGRAM_MAP(mouseatk_map)

	MCFG_MACHINE_RESET(tickee)
	MCFG_NVRAM_ADD_1FILL("nvram")

	MCFG_TICKET_DISPENSER_ADD("ticket1", attotime::from_msec(100), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_HIGH)
	MCFG_TICKET_DISPENSER_ADD("ticket2", attotime::from_msec(100), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_HIGH)

	/* video hardware */
	MCFG_TLC34076_ADD("tlc34076", TLC34076_6_BIT)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(VIDEO_CLOCK/2, 444, 0, 320, 233, 0, 200)
	MCFG_SCREEN_UPDATE_STATIC(tms340x0_rgb32)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym", YM2149, OKI_CLOCK)
	MCFG_SOUND_CONFIG(ay8910_interface_1)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_OKIM6295_ADD("oki", OKI_CLOCK, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

/*
Tickee Tickats
Raster Elite, 1994

This is a gun shooting game similar to Point Blank, with ticket redemption on game completion.
The PCB is around 6" square and contains only a few conponents.

CPU  : TMS34010FNL-40
SOUND: AY-3-8910 (x2)
OSC  : 40.000MHz, 14.31818MHz
RAM  : TOSHIBA TC524258BZ-80 (x4)
DIPSW: 8 position (x1)
PROMs: None
PALs : None
OTHER: ADV476KN50E (DIP28)
       MACH110 (CPLD, PLCC44)
       DALLAS DS1220Y-150 (NVRAM)
       4-pin header for standard light gun (x2)

ROMS :
-----------------------------------------
ds1220y.ic1  NVRAM       located near ic2
1.ic2        27C040  \
2.ic3        27C040   |
3.ic4        27C040   |  main program
4.ic5        27C040   /
*/

ROM_START( tickee )
	ROM_REGION16_LE( 0x200000, "user1", 0 )	/* 34010 code */
	ROM_LOAD16_BYTE( "3.ic4",  0x000000, 0x80000, CRC(5b1e399c) SHA1(681608f06bbaf3d258e9f4768a8a6c5047ad08ec) )
	ROM_LOAD16_BYTE( "2.ic3",  0x000001, 0x80000, CRC(1b26d4bb) SHA1(40266ec0fe5897eba85072e5bb39973d34f97546) )
	ROM_LOAD16_BYTE( "1.ic2",  0x100000, 0x80000, CRC(f7f0309e) SHA1(4a93e0e203f5a340a56b770a40b9ab00e131644d) )
	ROM_LOAD16_BYTE( "4.ic5",  0x100001, 0x80000, CRC(ceb0f559) SHA1(61923fe09e1dfde1eaae297ccbc672bc74a70397) )
ROM_END


/*

Ghost Hunter
Hanaho Games, 1996

  and

Tut's Tomb
Island Design Inc., 1996

PCB Layout
----------

|-----------------------------------------------|
|VOL                              CN106  CN103  |
|        NTE1423      ADV476KN50E    74LS175    |
|                                               |
|   YM2149            V52C4258Z80    74LS138    |
|                                               |
|   YM2149            V52C4258Z80    74LS373    |
|                                               |
|         DIPSW(8)    V52C4258Z80    74LS373    |
|J                                              |
|A         74LS161    V52C4258Z80    GHOSTHUN.7K|
|M  74LS74                                      |
|M         74LS273    |------|       GHOSTHUN.7J|
|A                    |TMS   |                  |
|          74LS74     |34010 |       GHOSTHUN.7H|
|                     |------|                  |
|          74LS14                    GHOSTHUN.7G|
|   ULN2803           |---|                     |
| TIP122   TIP122     |110|          DS1220Y    |
| TIP122   TIP122     |---|    40MHz 14.31818MHz|
| TIP122   TIP122    74LS273                    |
|                              74LS245  74LS374 |
|-------------------------------------|--CN2--|-|
(All IC's shown)                      |-------|

Notes:
      34010 clocks - INCLK: 40.000MHz, VCLK: 7.15909MHz, HSYNC: 16.1kHz, VSYNC: 69Hz, BLANK: 69Hz
      YM2149 clock - 1.7897725MHz [14.31818/8]
      VSync        - 69Hz
      HSync        - 15.78kHz
      110          - AMD MACH110 High Density Electrically-Erasable CMOS Programmable Logic (PLCC44)
      ADV476KN50E  - Analog Devices ADV476KN50E CMOS Monolithic 256x18 Color Palette RAM-DAC (DIP28)
      DS1220Y      - Dallas Semiconductor DS1220Y 16K Nonvolatile SRAM (DIP24)
      V52C4258Z80  - Vitelic V52C4258Z80 ?? possibly 256K x8 SRAM (ZIP28)
      NTE1423      - NTE1423 5.7W Power Amplifier (SIP8)
      CN2          - DB25 connector
      CN103/106    - 4-pin connector for gun hookup
      ULN2803      - Motorola ULN2803 Octal High Voltage, High Current Darlington Transistor Arrays (DIP18)
      TIP122       - Motorola TIP122 General-Purpose NPN Darlington Transistor (TO-220)
*/

ROM_START( ghoshunt )
	ROM_REGION16_LE( 0x200000, "user1", 0 )	/* 34010 code */
	ROM_LOAD16_BYTE( "ghosthun.7g",  0x000001, 0x80000, CRC(d59716c2) SHA1(717a1a1c5c559569f9e7bc4ae4356d112f0cf4eb) )
	ROM_LOAD16_BYTE( "ghosthun.7h",  0x000000, 0x80000, CRC(ef38bfc8) SHA1(12b8f29f4da120f14126cbcdf4019bedd97063c3) )
	ROM_LOAD16_BYTE( "ghosthun.7j",  0x100001, 0x80000, CRC(763d7c79) SHA1(f0dec99feeeefeddda6a88276dc306a30a58f4e4) )
	ROM_LOAD16_BYTE( "ghosthun.7k",  0x100000, 0x80000, CRC(71e6099e) SHA1(2af6f1aa304eed849c90d95d17643cb12b05baab) )
ROM_END


ROM_START( tutstomb )
	ROM_REGION16_LE( 0x200000, "user1", 0 )	/* 34010 code */
	ROM_LOAD16_BYTE( "tutstomb.7g",  0x000001, 0x80000, CRC(b74d3cf2) SHA1(2221b565362183a97a959389e8a0a026ca89e0ce) )
	ROM_LOAD16_BYTE( "tutstomb.7h",  0x000000, 0x80000, CRC(177f3afb) SHA1(845f982a66a8b69b0ea0045399102e8bb33f7fbf) )
	ROM_LOAD16_BYTE( "tutstomb.7j",  0x100001, 0x80000, CRC(69094f31) SHA1(eadae8847d0ff1568e63f71bf09a84dc443fdc1c))
	ROM_LOAD16_BYTE( "tutstomb.7k",  0x100000, 0x80000, CRC(bc362df8) SHA1(7b15c646e99c916d850629e4e758b1dbb329639a) )
ROM_END


/*

Mouse Attack
ICE, 1996

MA Video Board C ICE Inc. 1996
+--------------------------------------+
|                                 VOL  |
|      U202                            |
|   M6295             ADV476           |
|   1MHz               RAM             |
|J                     RAM             |
|A  YM2149             RAM        U10  |
|M                     RAM         U9  |
|M     DSW8                        U8  |
|A                  TMS34010-50    U7  |
|                                      |
|                  MACH110       40MHz |
|   +--------+ +------+                |
|   |  DB25  | | DB15 |     14.31818MHz|
+---+--------+-+------+----------------+

DB25 and DB15 are not populated.

*/

ROM_START( mouseatk )
	ROM_REGION16_LE( 0x200000, "user1", 0 )	/* 34010 code */
	ROM_LOAD16_BYTE( "ms-attk2-rev.a1.u8",  0x000000, 0x80000, CRC(a150525c) SHA1(a6be8524ae435502eeeeeaafa856dc812121d4e3) )
	ROM_LOAD16_BYTE( "ms-attk3-rev.a1.u9",  0x000001, 0x80000, CRC(f060091d) SHA1(b3a2099d5ca5a658a7a87bb11a20c27a6a2f11f2) )
	ROM_LOAD16_BYTE( "ms-attk4-rev.a1.u10", 0x100000, 0x80000, CRC(19806349) SHA1(3431dc70897f50e1be7578dd4ef99fa9be4450cf) )
	ROM_LOAD16_BYTE( "ms-attk1-rev.a1.u7",  0x100001, 0x80000, CRC(b936194b) SHA1(33a2038a56fb4a4301ee04ca7a32a70ab5870fad) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "ms-snd-rev.a.u202", 0x000000, 0x80000, CRC(ce4af0f2) SHA1(f054ac27faf52ae5ec6910ecfca164a21eefe4ca) )
ROM_END


/*

Rapid Fire (v1.1)
Hanaho Games, 1998

BE 1V-0 VER3
+-----------------------------------------------+
|         U510 U507    M6295    MACH210         |
|   VOL DS1232                          RAM RAM |
|                               MACH210         |
|GUN1     YM3012 18MHz                  RAM RAM |
|GUN2                                           |
+-+         YM2151 8MHz  TMS34010               |
  |                3.5795MHz                    |
+-+                      MACH110                |
|                                    ADV476     |
|J            DSW2               50MHz          |
|A            DSW1                              |
|M                                  U3 U5 U7 U9 |
|M                                              |
|A     +------------+        DS1220 U2 U4 U6 U8 |
|      | CN110 DB25 |                           |
+------+------------+---------------------------+


PCB No: BE 1V-0 VER3: 7/30/98 (Sticker says PVG/SEMCO)
CPU   : TMS34010FNL-50
SND   : BS901 + CA5102 (YM2151 + YM3012), AD-65 (OKI M6295)
OSC   : 50.000MHz, 3.5795MHz, 8.000MHz, 18.000MHz
RAM   : Toshiba TC528267J-70 (x 4, surface mounted)
DIPs  : 8 position (x2)
OTHER : MACH110 (x1, CPLD)
        MACH210 (x2, CPLD)
        DS1232 (Dallas Semi, MicroMonitor Chip / Watchdog timer)
        ADV476KN50 (28 Pin DIP, Analog Devices CMOS Monolithic 256x18 color palette RAM-DAC @ 50MHz)
        DALLAS DS1220Y-200 (NONVOLATILE SRAM)

There is also a standard DB25 female connector (looks like a Parallel connector)

ROMs  : RF11.U2     27C040      \
        RF11.U3       "         |
        RF10.U4       "         |
        RF10.U5       "         |
        RF10.U6       "         |   Grouped together, PRG/GFX
        RF10.U7       "         |
        RF11.U8       "         |
        RF11.U9       "         |
        DS1220Y.U504  *1        /

        RF11.U507    27C040     \
        RF11.U510      "        /   Grouped together, SND/OKI SAMPLES

*1: Contents of Dallas DS1220 NVRAM (2K)

*/

ROM_START( rapidfir ) /* Version 1.1, test menu shows "Build 239" */
	ROM_REGION16_LE( 0x400000, "user1", 0 )	/* 34010 code */
	ROM_LOAD16_BYTE( "rf11.u8",  0x000000, 0x80000, CRC(f7d8df33) SHA1(0abd54bbccfa90d830cbbdbcf2058197af980981) )
	ROM_LOAD16_BYTE( "rf11.u9",  0x000001, 0x80000, CRC(a72af935) SHA1(ed0deb6f51681f70e07ad7c05a92f6a0f2063f7a) )
	ROM_LOAD16_BYTE( "rf10.u6",  0x100000, 0x80000, CRC(e8d2e5d2) SHA1(db93014598f7b76785e0fd5c0ac8808a3be06435) ) /* Can be labeled V1.0 or V1.1 */
	ROM_LOAD16_BYTE( "rf10.u7",  0x100001, 0x80000, CRC(0e33f2ed) SHA1(9b2533e001b94ccc97b95e31762186f59d5a3b9c) ) /* Can be labeled V1.0 or V1.1 */
	ROM_LOAD16_BYTE( "rf10.u4",  0x200000, 0x80000, CRC(8a088468) SHA1(f94dc78158e5656657d3b26c5b0ca88f39eb5ff4) ) /* Can be labeled V1.0 or V1.1 */
	ROM_LOAD16_BYTE( "rf10.u5",  0x200001, 0x80000, CRC(593b3df2) SHA1(301fa06031eff54fb2d9e08f80fc3c26e5c51da9) ) /* Can be labeled V1.0 or V1.1 */
	ROM_LOAD16_BYTE( "rf11.u2",  0x300000, 0x80000, CRC(ffa0c695) SHA1(bccdefe7cee15999c416fdcb16a65b1bc6e12d13) )
	ROM_LOAD16_BYTE( "rf11.u3",  0x300001, 0x80000, CRC(ac63b863) SHA1(c9160aec6179d1f550279b80fd4c2a14ce94fdab) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "rf11.u507", 0x000000, 0x80000, CRC(899d1e15) SHA1(ca22b4ad714a5212bc9347eb3a5b660c02bad7e5) )
	ROM_LOAD( "rf11.u510", 0x080000, 0x80000, CRC(6209c8fe) SHA1(bfbd63445b4ac2d4253c4b5354e1058070290084) )
ROM_END

ROM_START( rapidfira ) /* Version 1.1, test menu shows "Build 238" */
	ROM_REGION16_LE( 0x400000, "user1", 0 )	/* 34010 code */
	ROM_LOAD16_BYTE( "rf11.u8",  0x000000, 0x80000, CRC(f7d8df33) SHA1(0abd54bbccfa90d830cbbdbcf2058197af980981) )
	ROM_LOAD16_BYTE( "rf11.u9",  0x000001, 0x80000, CRC(a72af935) SHA1(ed0deb6f51681f70e07ad7c05a92f6a0f2063f7a) )
	ROM_LOAD16_BYTE( "rf10.u6",  0x100000, 0x80000, CRC(e8d2e5d2) SHA1(db93014598f7b76785e0fd5c0ac8808a3be06435) ) /* Can be labeled V1.0 or V1.1 */
	ROM_LOAD16_BYTE( "rf10.u7",  0x100001, 0x80000, CRC(0e33f2ed) SHA1(9b2533e001b94ccc97b95e31762186f59d5a3b9c) ) /* Can be labeled V1.0 or V1.1 */
	ROM_LOAD16_BYTE( "rf10.u4",  0x200000, 0x80000, CRC(8a088468) SHA1(f94dc78158e5656657d3b26c5b0ca88f39eb5ff4) ) /* Can be labeled V1.0 or V1.1 */
	ROM_LOAD16_BYTE( "rf10.u5",  0x200001, 0x80000, CRC(593b3df2) SHA1(301fa06031eff54fb2d9e08f80fc3c26e5c51da9) ) /* Can be labeled V1.0 or V1.1 */
	ROM_LOAD16_BYTE( "rf11-u2",  0x300000, 0x80000, CRC(bcb7195a) SHA1(04b14facfe84973737f94b5278fb59668af0a47c) ) /* Only U2 & U3 differ from Build 239, labeled the same */
	ROM_LOAD16_BYTE( "rf11-u3",  0x300001, 0x80000, CRC(e455a0b5) SHA1(5568f55ccfd6cba373f4b4f3e61f0a4f391b01c0) ) /* Only U2 & U3 differ from Build 239, labeled the same */

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "rf11.u507", 0x000000, 0x80000, CRC(899d1e15) SHA1(ca22b4ad714a5212bc9347eb3a5b660c02bad7e5) )
	ROM_LOAD( "rf11.u510", 0x080000, 0x80000, CRC(6209c8fe) SHA1(bfbd63445b4ac2d4253c4b5354e1058070290084) )
ROM_END


ROM_START( rapidfire ) /* Version 1.0, test menu shows "Build 236" */
	ROM_REGION16_LE( 0x400000, "user1", 0 )	/* 34010 code */
	ROM_LOAD16_BYTE( "rf10.u8",  0x000000, 0x80000, CRC(71d42125) SHA1(e34bdb08aa1b85ba4dd219b7f6de6f99a1ed8758) )
	ROM_LOAD16_BYTE( "rf10.u9",  0x000001, 0x80000, CRC(d70e67c2) SHA1(e0876027f58584dae949a4f3c9391bd013912ee8) )
	ROM_LOAD16_BYTE( "rf10.u6",  0x100000, 0x80000, CRC(e8d2e5d2) SHA1(db93014598f7b76785e0fd5c0ac8808a3be06435) )
	ROM_LOAD16_BYTE( "rf10.u7",  0x100001, 0x80000, CRC(0e33f2ed) SHA1(9b2533e001b94ccc97b95e31762186f59d5a3b9c) )
	ROM_LOAD16_BYTE( "rf10.u4",  0x200000, 0x80000, CRC(8a088468) SHA1(f94dc78158e5656657d3b26c5b0ca88f39eb5ff4) )
	ROM_LOAD16_BYTE( "rf10.u5",  0x200001, 0x80000, CRC(593b3df2) SHA1(301fa06031eff54fb2d9e08f80fc3c26e5c51da9) )
	ROM_LOAD16_BYTE( "rf10.u2",  0x300000, 0x80000, CRC(5ef404dd) SHA1(cf060567822c4c02baf4b1948d6e50b480bfb7da) )
	ROM_LOAD16_BYTE( "rf10.u3",  0x300001, 0x80000, CRC(d8d664db) SHA1(cd63fdc6fe4beb68ced57a2547f8302c1d2544dc) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "rf10.u507", 0x000000, 0x80000, CRC(7eab2af4) SHA1(bbb4b2b9f96add56c26f334a7242cdc81a64ce2d) )
	ROM_LOAD( "rf10.u510", 0x080000, 0x80000, CRC(ecd70be6) SHA1(5a26703e822776fecb3f6d729bf91e76db7a7141) )
ROM_END


/*

Mallet Madness (v2.1)
Hanaho Games, 1999 licensed to Capcom

Same exact PCB as Rapid Fire

*/


ROM_START( maletmad ) /* Version 2.1 */
	ROM_REGION16_LE( 0x400000, "user1", 0 )	/* 34010 code */
	/* U8 & U9 not populated */
	ROM_LOAD16_BYTE( "malletmadness_v2.1.u6",  0x100000, 0x80000, CRC(83309174) SHA1(d387c8bc4d3c640f16525241892cc8d5d5da7f60) )
	ROM_LOAD16_BYTE( "malletmadness_v2.1.u7",  0x100001, 0x80000, CRC(4642587e) SHA1(076eda538d570074028e9b4394f1a8a459678137) )
	ROM_LOAD16_BYTE( "malletmadness_v2.1.u4",  0x200000, 0x80000, CRC(70ca968c) SHA1(74c66a67568b428ae5e20377038c7ea0cd33b25e) )
	ROM_LOAD16_BYTE( "malletmadness_v2.1.u5",  0x200001, 0x80000, CRC(c771418a) SHA1(cab360103a4d4195f5a9920746ae5df2866e24dc) )
	ROM_LOAD16_BYTE( "malletmadness_v2.1.u2",  0x300000, 0x80000, CRC(08825aee) SHA1(d7c2098ce5d73e0ca4b02a654d5c958d999337e3) )
	ROM_LOAD16_BYTE( "malletmadness_v2.1.u3",  0x300001, 0x80000, CRC(49a6bd62) SHA1(40d67fd7a3dded708366246d897682ffee88a2e1) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "malletmadness_v2.1.u507", 0x000000, 0x80000, CRC(6a2c9021) SHA1(bff61ef696a2104a32aab6fdc51f504385d0c769) )
	/* U510 not populated */
ROM_END


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1994, tickee,    0,        tickee,   tickee,   0, ROT0, "Raster Elite",  "Tickee Tickats", 0 )
GAME( 1996, ghoshunt,  0,        ghoshunt, ghoshunt, 0, ROT0, "Hanaho Games",  "Ghost Hunter", 0 )
GAME( 1996, tutstomb,  0,        ghoshunt, ghoshunt, 0, ROT0, "Island Design", "Tut's Tomb", 0 )
GAME( 1996, mouseatk,  0,        mouseatk, mouseatk, 0, ROT0, "ICE",           "Mouse Attack", 0 )
GAME( 1998, rapidfir,  0,        rapidfir, rapidfir, 0, ROT0, "Hanaho Games",  "Rapid Fire v1.1 (Build 239)", 0 )
GAME( 1998, rapidfira, rapidfir, rapidfir, rapidfir, 0, ROT0, "Hanaho Games",  "Rapid Fire v1.1 (Build 238)", 0 )
GAME( 1998, rapidfire, rapidfir, rapidfir, rapidfir, 0, ROT0, "Hanaho Games",  "Rapid Fire v1.0 (Build 236)", 0 )
GAME( 1999, maletmad,  0,        rapidfir, rapidfir, 0, ROT0, "Hanaho Games",  "Mallet Madness v2.1", 0 )

