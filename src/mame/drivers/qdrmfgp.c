/***************************************************************************

Quiz Do Re Mi Fa Grand Prix (Japan)     (GQ460) (c)1994 Konami
Quiz Do Re Mi Fa Grand Prix 2 (Japan)   (GE557) (c)1995 Konami


CPU  :MC68HC000FN16
OSC  :18.43200MHz/32.00000MHz
Other(GQ460):Konami 053252,054156,056832,054539
Other(GE557):Konami 056832,058141,058143

TODO:
- enabling irq acks breaks both games, why?

--
driver by Eisuke Watanabe

Note:
GP1 HDD data contents:
    0x000-0x52D intro quiz musics
    0x52E-0x535 not used quiz (system music or invalid data)
***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/idectrl.h"
#include "sound/k054539.h"
#include "video/konicdev.h"
#include "machine/k053252.h"
#include "machine/nvram.h"
#include "includes/qdrmfgp.h"


/*************************************
 *
 *  68k CPU memory handlers
 *
 *************************************/

static CUSTOM_INPUT( inputs_r )
{
	qdrmfgp_state *state = field.machine().driver_data<qdrmfgp_state>();
	const char *tag1 = (const char *)param;
	const char *tag2 = tag1 + strlen(tag1) + 1;
	return input_port_read(field.machine(), (state->m_control & 0x0080) ? tag1 : tag2);
}

static CUSTOM_INPUT( battery_sensor_r )
{
	/* bit 0-1  battery power sensor: 3=good, 2=low, other=bad */
	return 0x0003;
}


static WRITE16_HANDLER( gp_control_w )
{
	qdrmfgp_state *state = space->machine().driver_data<qdrmfgp_state>();

	/* bit 0        enable irq 1 (sound) */
	/* bit 1        enable irq 2 (not used) */
	/* bit 2        enable irq 3 (vblank) */
	/* bit 3        enable irq 4 (hdd) */
	/* bit 4-6      palette (tilemap) */
	/* bit 7        inputports bankswitch */
	/* bit 8        enable volume control */
	/* bit 9        volume: 1=up, 0=down (low5,mid90,high180) */
	/* bit 10       enable headphone volume control */
	/* bit 11       headphone volume: 1=up, 0=down */
	/* bit 15       gfxrom bankswitch */

	COMBINE_DATA(&state->m_control);
	state->m_pal = state->m_control & 0x70;

	if (state->m_control & 0x0100)
	{
		qdrmfgp_state *state = space->machine().driver_data<qdrmfgp_state>();
		int vol = state->m_nvram[0x10] & 0xff;
		if (vol)
		{
			device_t *k054539 = space->machine().device("konami");
			int i;
			double gain = vol / 90.0;

			for (i=0; i<8; i++)
				k054539_set_gain(k054539, i, gain);
		}
	}
}

static WRITE16_HANDLER( gp2_control_w )
{
	qdrmfgp_state *state = space->machine().driver_data<qdrmfgp_state>();

	/* bit 2        enable irq 3 (sound) */
	/* bit 3        enable irq 4 (vblank) */
	/* bit 4        enable irq 5 (hdd) */
	/* bit 7        inputports bankswitch */
	/* bit 8        enable volume control */
	/* bit 9        volume: 1=up, 0=down (low0,mid90,high255) */
	/* bit 10       enable headphone volume control */
	/* bit 11       headphone volume: 1=up, 0=down */
	/* bit 15       gfxrom bankswitch */

	COMBINE_DATA(&state->m_control);
	state->m_pal = 0;

	if (state->m_control & 0x0100)
	{
		qdrmfgp_state *state = space->machine().driver_data<qdrmfgp_state>();
		int vol = state->m_nvram[0x8] & 0xff;
		if (vol)
		{
			device_t *k054539 = space->machine().device("konami");
			int i;
			double gain = vol / 90.0;

			for (i=0; i<8; i++)
				k054539_set_gain(k054539, i, gain);
		}
	}
}


static READ16_HANDLER( v_rom_r )
{
	qdrmfgp_state *state = space->machine().driver_data<qdrmfgp_state>();
	device_t *k056832 = space->machine().device("k056832");
	UINT8 *mem8 = space->machine().region("gfx1")->base();
	int bank = k056832_word_r(k056832, 0x34/2, 0xffff);

	offset += bank * 0x800 * 4;

	if (state->m_control & 0x8000)
		offset += 0x800 * 2;

	return (mem8[offset + 1] << 8) + mem8[offset];
}


static READ16_HANDLER( gp2_vram_r )
{
	device_t *k056832 = space->machine().device("k056832");

	if (offset < 0x1000 / 2)
		return k056832_ram_word_r(k056832, offset * 2 + 1, mem_mask);
	else
		return k056832_ram_word_r(k056832, (offset - 0x1000 / 2) * 2, mem_mask);
}

static READ16_HANDLER( gp2_vram_mirror_r )
{
	device_t *k056832 = space->machine().device("k056832");

	if (offset < 0x1000 / 2)
		return k056832_ram_word_r(k056832, offset * 2, mem_mask);
	else
		return k056832_ram_word_r(k056832, (offset - 0x1000 / 2) * 2 + 1, mem_mask);
}

static WRITE16_HANDLER( gp2_vram_w )
{
	device_t *k056832 = space->machine().device("k056832");

	if (offset < 0x1000 / 2)
		k056832_ram_word_w(k056832, offset * 2 + 1, data, mem_mask);
	else
		k056832_ram_word_w(k056832, (offset - 0x1000 / 2) * 2, data, mem_mask);
}

static WRITE16_HANDLER( gp2_vram_mirror_w )
{
	device_t *k056832 = space->machine().device("k056832");

	if (offset < 0x1000 / 2)
		k056832_ram_word_w(k056832, offset * 2, data, mem_mask);
	else
		k056832_ram_word_w(k056832, (offset - 0x1000 / 2) * 2 + 1, data, mem_mask);
}


/*************/

static READ16_HANDLER( sndram_r )
{
	qdrmfgp_state *state = space->machine().driver_data<qdrmfgp_state>();
	if (ACCESSING_BITS_0_7)
		return state->m_sndram[offset];

	return 0;
}

static WRITE16_HANDLER( sndram_w )
{
	qdrmfgp_state *state = space->machine().driver_data<qdrmfgp_state>();
	if (ACCESSING_BITS_0_7)
	{
		state->m_sndram[offset] = data & 0xff;
		if (offset >= 0x40000)
			state->m_sndram[offset+0xc00000-0x900000] = data & 0xff;
	}
}

/*************/

#define IDE_STD_OFFSET	(0x1f0/2)
#define IDE_ALT_OFFSET	(0x3f6/2)

static READ16_DEVICE_HANDLER( ide_std_r )
{
	if (offset & 0x01)
		return ide_controller16_r(device, IDE_STD_OFFSET + offset/2, 0xff00) >> 8;
	else
		return ide_controller16_r(device, IDE_STD_OFFSET + offset/2, 0xffff);
}

static WRITE16_DEVICE_HANDLER( ide_std_w )
{
	if (offset & 0x01)
		ide_controller16_w(device, IDE_STD_OFFSET + offset/2, data << 8, 0xff00);
	else
		ide_controller16_w(device, IDE_STD_OFFSET + offset/2, data, 0xffff);
}

static READ16_DEVICE_HANDLER( ide_alt_r )
{
	if (offset == 0)
		return ide_controller16_r(device, IDE_ALT_OFFSET, 0x00ff);

	return 0;
}

static WRITE16_DEVICE_HANDLER( ide_alt_w )
{
	if (offset == 0)
		ide_controller16_w(device, IDE_ALT_OFFSET, data, 0x00ff);
}


static READ16_HANDLER( gp2_ide_std_r )
{
	qdrmfgp_state *state = space->machine().driver_data<qdrmfgp_state>();
	device_t *device = space->machine().device("ide");
	if (offset & 0x01)
	{
		if (offset == 0x07)
		{
			switch (cpu_get_previouspc(&space->device()))
			{
				case 0xdb4c:
					if ((state->m_workram[0x5fa4/2] - cpu_get_reg(&space->device(), M68K_D0)) <= 0x10)
						state->m_gp2_irq_control = 1;
					break;
				case 0xdec2:
					state->m_gp2_irq_control = 1;
				default:
					break;
			}
		}
		return ide_controller16_r(device, IDE_STD_OFFSET + offset/2, 0xff00) >> 8;
	} else {
		return ide_controller16_r(device, IDE_STD_OFFSET + offset/2, 0xffff);
	}
}


/*************************************
 *
 *  Interrupt handlers
 *
 *************************************/

static TIMER_DEVICE_CALLBACK(qdrmfgp_interrupt)
{
	qdrmfgp_state *state = timer.machine().driver_data<qdrmfgp_state>();
	int scanline = param;

	if(scanline == 0)
		if (state->m_control & 0x0001)
			device_set_input_line(state->m_maincpu, 1, HOLD_LINE);

	/* trigger V-blank interrupt */
	if(scanline == 240)
		if (state->m_control & 0x0004)
			device_set_input_line(state->m_maincpu, 3, HOLD_LINE);
}

static void ide_interrupt(device_t *device, int state)
{
	qdrmfgp_state *drvstate = device->machine().driver_data<qdrmfgp_state>();
	if (drvstate->m_control & 0x0008)
	{
		if (state != CLEAR_LINE)
			cputag_set_input_line(device->machine(), "maincpu", 4, HOLD_LINE);
		else
			cputag_set_input_line(device->machine(), "maincpu", 4, CLEAR_LINE);
	}
}

/*************/

static TIMER_CALLBACK( gp2_timer_callback )
{
	qdrmfgp_state *state = machine.driver_data<qdrmfgp_state>();
	if (state->m_control & 0x0004)
		cputag_set_input_line(machine, "maincpu", 3, HOLD_LINE);
}

static INTERRUPT_GEN(qdrmfgp2_interrupt)
{
	qdrmfgp_state *state = device->machine().driver_data<qdrmfgp_state>();
	/* trigger V-blank interrupt */
	if (state->m_control & 0x0008)
		device_set_input_line(device, 4, HOLD_LINE);
}

static void gp2_ide_interrupt(device_t *device, int state)
{
	qdrmfgp_state *drvstate = device->machine().driver_data<qdrmfgp_state>();
	if (drvstate->m_control & 0x0010)
	{
		if (state != CLEAR_LINE)
		{
			if (drvstate->m_gp2_irq_control)
				drvstate->m_gp2_irq_control = 0;
			else
				cputag_set_input_line(device->machine(), "maincpu", 5, HOLD_LINE);
		}
		else
		{
			cputag_set_input_line(device->machine(), "maincpu", 5, CLEAR_LINE);
		}
	}
}


/*************************************
 *
 *  Memory definitions
 *
 *************************************/

static ADDRESS_MAP_START( qdrmfgp_map, AS_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM AM_BASE_MEMBER(qdrmfgp_state, m_workram)										/* work ram */
	AM_RANGE(0x180000, 0x183fff) AM_RAM AM_SHARE("nvram")	/* backup ram */
	AM_RANGE(0x280000, 0x280fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x300000, 0x30003f) AM_DEVWRITE("k056832", k056832_word_w)										/* video reg */
	AM_RANGE(0x320000, 0x32001f) AM_DEVREADWRITE8("k053252", k053252_r, k053252_w,0x00ff)					/* ccu */
	AM_RANGE(0x330000, 0x330001) AM_READ_PORT("SENSOR")											/* battery power & service sw */
	AM_RANGE(0x340000, 0x340001) AM_READ_PORT("340000")											/* inputport */
	AM_RANGE(0x350000, 0x350001) AM_WRITENOP													/* unknown */
	AM_RANGE(0x360000, 0x360001) AM_WRITENOP													/* unknown */
	AM_RANGE(0x370000, 0x370001) AM_WRITE(gp_control_w)											/* control reg */
	AM_RANGE(0x380000, 0x380001) AM_WRITENOP													/* Watchdog */
	AM_RANGE(0x800000, 0x80045f) AM_DEVREADWRITE8("konami", k054539_r, k054539_w, 0x00ff)		/* sound regs */
	AM_RANGE(0x880000, 0x881fff) AM_DEVREADWRITE("k056832", k056832_ram_word_r, k056832_ram_word_w)			/* vram */
	AM_RANGE(0x882000, 0x883fff) AM_DEVREADWRITE("k056832", k056832_ram_word_r, k056832_ram_word_w)			/* vram (mirror) */
	AM_RANGE(0x900000, 0x901fff) AM_READ(v_rom_r)												/* gfxrom through */
	AM_RANGE(0xa00000, 0xa0000f) AM_DEVREADWRITE("ide", ide_std_r,ide_std_w)					/* IDE control regs */
	AM_RANGE(0xa4000c, 0xa4000f) AM_DEVREADWRITE("ide", ide_alt_r,ide_alt_w)					/* IDE status control reg */
	AM_RANGE(0xc00000, 0xcbffff) AM_READWRITE(sndram_r, sndram_w)								/* sound ram */
ADDRESS_MAP_END


static ADDRESS_MAP_START( qdrmfgp2_map, AS_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x110fff) AM_RAM AM_BASE_MEMBER(qdrmfgp_state, m_workram)										/* work ram */
	AM_RANGE(0x180000, 0x183fff) AM_RAM AM_SHARE("nvram")	/* backup ram */
	AM_RANGE(0x280000, 0x280fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x300000, 0x30003f) AM_DEVWRITE("k056832", k056832_word_w)										/* video reg */
	AM_RANGE(0x320000, 0x32001f) AM_DEVREADWRITE8("k053252", k053252_r, k053252_w,0xff00)					/* ccu */
	AM_RANGE(0x330000, 0x330001) AM_READ_PORT("SENSOR")											/* battery power & service */
	AM_RANGE(0x340000, 0x340001) AM_READ_PORT("340000")											/* inputport */
	AM_RANGE(0x350000, 0x350001) AM_WRITENOP													/* unknown */
	AM_RANGE(0x360000, 0x360001) AM_WRITENOP													/* unknown */
	AM_RANGE(0x370000, 0x370001) AM_WRITE(gp2_control_w)										/* control reg */
	AM_RANGE(0x380000, 0x380001) AM_WRITENOP													/* Watchdog */
	AM_RANGE(0x800000, 0x80045f) AM_DEVREADWRITE8("konami", k054539_r,k054539_w, 0x00ff)		/* sound regs */
	AM_RANGE(0x880000, 0x881fff) AM_READWRITE(gp2_vram_r, gp2_vram_w)							/* vram */
	AM_RANGE(0x89f000, 0x8a0fff) AM_READWRITE(gp2_vram_mirror_r, gp2_vram_mirror_w)				/* vram (mirror) */
	AM_RANGE(0x900000, 0x901fff) AM_READ(v_rom_r)												/* gfxrom through */
	AM_RANGE(0xa00000, 0xa0000f) AM_READ(gp2_ide_std_r) AM_DEVWRITE("ide", ide_std_w)			/* IDE control regs */
	AM_RANGE(0xa4000c, 0xa4000f) AM_DEVREADWRITE("ide", ide_alt_r,ide_alt_w)					/* IDE status control reg */
	AM_RANGE(0xc00000, 0xcbffff) AM_READWRITE(sndram_r,sndram_w)								/* sound ram */
ADDRESS_MAP_END


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( qdrmfgp )
	PORT_START("340000")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(inputs_r, "INPUTS\0DSW")

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)	/* 1P STOP */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)	/* 2P STOP */

	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE1 )	/* SERVICE */
	PORT_SERVICE_NO_TOGGLE( 0x2000, IP_ACTIVE_LOW )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0004, 0x0004, "Extended Service Menu" )		/* and skipped initial checks. */
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Skip HDD Check" )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "Initialize Backup RAM" )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )

	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x3000, "All The Time" )
	PORT_DIPSETTING(      0x1000, "Once Every 2Cycles" )
	PORT_DIPSETTING(      0x2000, "Once Every 4Cycles" )
	PORT_DIPSETTING(      0x0000, "Completely Off" )
	PORT_DIPNAME( 0xc000, 0x4000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )

	PORT_START("SENSOR")
	PORT_BIT( 0x0003, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(battery_sensor_r, NULL)	/* battery power sensor */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( qdrmfgp2 )
	PORT_START("340000")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(inputs_r, "INPUTS\0DSW")

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)	/* 1P STOP */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)	/* 2P STOP */

	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE1 )	/* SERVICE */
	PORT_SERVICE_NO_TOGGLE( 0x2000, IP_ACTIVE_LOW )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0004, 0x0004, "Extended Service Menu & None Sounds Mode" )		/* and skipped initial checks. */
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "Initialize Backup RAM" )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )

	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x3000, "All The Time" )
	PORT_DIPSETTING(      0x1000, "Once Every 2Cycles" )
	PORT_DIPSETTING(      0x2000, "Once Every 4Cycles" )
	PORT_DIPSETTING(      0x0000, "Completely Off" )
	PORT_DIPNAME( 0xc000, 0x4000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )

	PORT_START("SENSOR")
	PORT_BIT( 0x0003, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(battery_sensor_r, NULL)	/* battery power sensor */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics layouts
 *
 *************************************/


/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

static void sound_irq(device_t *device)
{
	qdrmfgp_state *state = device->machine().driver_data<qdrmfgp_state>();
	if (state->m_control & 0x0001)
		cputag_set_input_line(device->machine(), "maincpu", 1, HOLD_LINE);
}

static const k054539_interface k054539_config =
{
	NULL,
	NULL,
	sound_irq
};


/*************************************
 *
 *  Machine-specific init
 *
 *************************************/

static const k056832_interface qdrmfgp_k056832_intf =
{
	"gfx1", 0,
	K056832_BPP_4dj,
	1, 0,
	KONAMI_ROM_DEINTERLEAVE_NONE,
	qdrmfgp_tile_callback, "none"
};

static const k056832_interface qdrmfgp2_k056832_intf =
{
	"gfx1", 0,
	K056832_BPP_4dj,
	1, 0,
	KONAMI_ROM_DEINTERLEAVE_NONE,
	qdrmfgp2_tile_callback, "none"
};

static WRITE_LINE_DEVICE_HANDLER( qdrmfgp_irq3_ack_w )
{
//  cputag_set_input_line(device->machine(), "maincpu", M68K_IRQ_3, CLEAR_LINE);
}

static WRITE_LINE_DEVICE_HANDLER( qdrmfgp_irq4_ack_w )
{
//  cputag_set_input_line(device->machine(), "maincpu", M68K_IRQ_4, CLEAR_LINE);
}

static const k053252_interface qdrmfgp_k053252_intf =
{
	"screen",
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_LINE(qdrmfgp_irq3_ack_w),
	DEVCB_NULL,
	40, 16
};

static const k053252_interface qdrmfgp2_k053252_intf =
{
	"screen",
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_LINE(qdrmfgp_irq4_ack_w),
	DEVCB_LINE(qdrmfgp_irq3_ack_w),
	40, 16
};

static MACHINE_START( qdrmfgp )
{
	qdrmfgp_state *state = machine.driver_data<qdrmfgp_state>();
	state_save_register_global(machine, state->m_control);
	state_save_register_global(machine, state->m_pal);
	state_save_register_global(machine, state->m_gp2_irq_control);
}

static MACHINE_START( qdrmfgp2 )
{
	/* sound irq (CCU? 240Hz) */
	machine.scheduler().timer_pulse(attotime::from_hz(18432000/76800), FUNC(gp2_timer_callback));

	MACHINE_START_CALL( qdrmfgp );
}

static MACHINE_RESET( qdrmfgp )
{
	qdrmfgp_state *state = machine.driver_data<qdrmfgp_state>();
	state->m_sndram = machine.region("konami")->base() + 0x100000;

	/* reset the IDE controller */
	state->m_gp2_irq_control = 0;
	devtag_reset(machine, "ide");
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( qdrmfgp, qdrmfgp_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 32000000/2)	/*  16.000 MHz */
	MCFG_CPU_PROGRAM_MAP(qdrmfgp_map)
	MCFG_TIMER_ADD_SCANLINE("scantimer", qdrmfgp_interrupt, "screen", 0, 1)

	MCFG_MACHINE_START(qdrmfgp)
	MCFG_MACHINE_RESET(qdrmfgp)
	MCFG_NVRAM_ADD_1FILL("nvram")

	MCFG_IDE_CONTROLLER_ADD("ide", ide_interrupt)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(40, 40+384-1, 16, 16+224-1)
	MCFG_SCREEN_UPDATE(qdrmfgp)

	MCFG_PALETTE_LENGTH(2048)

	MCFG_VIDEO_START(qdrmfgp)

	MCFG_K056832_ADD("k056832", qdrmfgp_k056832_intf)
	MCFG_K053252_ADD("k053252", 32000000/4, qdrmfgp_k053252_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("konami", K054539, 18432000/384)
	MCFG_SOUND_CONFIG(k054539_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( qdrmfgp2, qdrmfgp_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 32000000/2)	/*  16.000 MHz */
	MCFG_CPU_PROGRAM_MAP(qdrmfgp2_map)
	MCFG_CPU_VBLANK_INT("screen", qdrmfgp2_interrupt)

	MCFG_MACHINE_START(qdrmfgp2)
	MCFG_MACHINE_RESET(qdrmfgp)
	MCFG_NVRAM_ADD_1FILL("nvram")

	MCFG_IDE_CONTROLLER_ADD("ide", gp2_ide_interrupt)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(40, 40+384-1, 16, 16+224-1)
	MCFG_SCREEN_UPDATE(qdrmfgp)

	MCFG_PALETTE_LENGTH(2048)

	MCFG_VIDEO_START(qdrmfgp2)

	MCFG_K056832_ADD("k056832", qdrmfgp2_k056832_intf)
	MCFG_K053252_ADD("k053252", 32000000/4, qdrmfgp2_k053252_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("konami", K054539, 18432000/384)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( qdrmfgp )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "gq_460_b04.20e", 0x000000, 0x80000, CRC(293d8174) SHA1(cf507d0b29dab161190f0160c05c640f16306bae) )
	ROM_LOAD16_WORD_SWAP( "gq_460_a05.22e", 0x080000, 0x80000, CRC(4128cb3c) SHA1(4a16d85a66934a20afd074546de362c40a1ea785) )

	ROM_REGION( 0x100000, "gfx1", 0 )		/* TILEMAP */
	ROM_LOAD( "gq_460_a01.15e", 0x000000, 0x80000, CRC(6536b700) SHA1(47ffe0cfbf80810179560150b23d825fe1a5c5ca) )
	ROM_LOAD( "gq_460_a02.17e", 0x080000, 0x80000, CRC(ac01d675) SHA1(bf66433ace95f4ef14699d03add7cbc2e5d90eea) )

	ROM_REGION( 0x460000, "konami", 0)		/* SE SAMPLES + space for additional RAM */
	ROM_LOAD( "gq_460_a07.14h", 0x000000, 0x80000, CRC(67d8ea6b) SHA1(11af1b5a33de2a6e24823964d210bef193ecefe4) )
	ROM_LOAD( "gq_460_a06.12h", 0x080000, 0x80000, CRC(97ed5a77) SHA1(68600fd8d914451284cf181fb4bd5872860fb9ad) )

	DISK_REGION( "ide" )			/* IDE HARD DRIVE */
	DISK_IMAGE( "gq460a08", 0, SHA1(2f142f986fa3c79d5c4102e800980d1706c35f75) )
ROM_END

ROM_START( qdrmfgp2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "ge_557_c05.20e", 0x000000, 0x80000, CRC(336df99f) SHA1(46fb36d40371761be0cfa17b34f28cc893a44a22) )
	ROM_LOAD16_WORD_SWAP( "ge_557_a06.22e", 0x080000, 0x80000, CRC(ad77e10f) SHA1(4a762a59fe3096d48e3cbf0da3bb0d75c5087e78) )

	ROM_REGION( 0x100000, "gfx1", 0 )		/* TILEMAP */
	ROM_LOAD( "ge_557_a01.13e", 0x000000, 0x80000, CRC(c301d406) SHA1(5fad8cc611edd83380972abf37ec80561b9317a6) )
	ROM_LOAD( "ge_557_a02.15e", 0x080000, 0x80000, CRC(3bfe1e56) SHA1(9e4df512a804a96fcb545d4e0eb58b5421d65ea4) )

	ROM_REGION( 0x460000, "konami", 0)		/* SE SAMPLES + space for additional RAM */
	ROM_LOAD( "ge_557_a07.19h", 0x000000, 0x80000, CRC(7491e0c8) SHA1(6459ab5e7af052ef7a1c4ce01cd844c0f4319f2e) )
	ROM_LOAD( "ge_557_a08.19k", 0x080000, 0x80000, CRC(3da2b20c) SHA1(fdc2cdc27f3299f541944a78ce36ed33a7926056) )

	DISK_REGION( "ide" )			/* IDE HARD DRIVE */
	DISK_IMAGE( "ge557a09", 0, SHA1(1ef8093b542fe0bf8240a5fd64e5af3839b6a04c) )
ROM_END


/*************************************
 *
 *  Game drivers
 *
 *************************************/

/*     year  rom       clone     machine   inputs    init */
GAME(  1994, qdrmfgp,  0,        qdrmfgp,  qdrmfgp,  0,        ROT0, "Konami", "Quiz Do Re Mi Fa Grand Prix (Japan)", 0 )
GAME(  1995, qdrmfgp2, 0,        qdrmfgp2, qdrmfgp2, 0,        ROT0, "Konami", "Quiz Do Re Mi Fa Grand Prix 2 - Shin-Kyoku Nyuukadayo (Japan)", 0 )
