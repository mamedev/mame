// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Midway V-Unit games

    driver by Aaron Giles

    Games supported:
        * Cruis'n USA (1994)        [3 sets]
        * Cruis'n World (1996)      [7 sets]
        * War Gods (1996)           [3 sets]
        * Off Road Challenge (1997) [5 sets]

    Known bugs:
        * textures for automatic/manual selection get overwritten in Cruis'n World
        * rendering needs to be looked at a little more closely to fix some holes
        * in Cruis'n World attract mode, right side of sky looks like it has wrapped
        * Off Road Challenge has polygon sorting issues, among other problems
        * Issues for the Wargods sets:
           Sound D/RAM      ERROR EE (during boot diag)
           Listen for Tone  ERROR E1 (during boot diag)
           All sets report as Game Type: 452 (12/11/1995) [which is wrong for newer sets]

Known to exist but not dumped:
    Off Road Challenge v1.00 (Mon 07-28-97)

**************************************************************************/

#include "emu.h"
#include "cpu/tms32031/tms32031.h"
#include "cpu/adsp2100/adsp2100.h"
#include "audio/dcs.h"
#include "machine/ataintf.h"
#include "machine/nvram.h"
#include "includes/midvunit.h"


#define CPU_CLOCK       50000000


/*************************************
 *
 *  Machine init
 *
 *************************************/

void midvunit_state::machine_start()
{
	save_item(NAME(m_cmos_protected));
	save_item(NAME(m_control_data));
	save_item(NAME(m_adc_data));
	save_item(NAME(m_adc_shift));
	save_item(NAME(m_last_port0));
	save_item(NAME(m_shifter_state));
	save_item(NAME(m_timer_rate));
}


void midvunit_state::machine_reset()
{
	m_dcs->reset_w(1);
	m_dcs->reset_w(0);

	memcpy(m_ram_base, memregion("user1")->base(), 0x20000*4);
	m_maincpu->reset();

	m_timer[0] = machine().device<timer_device>("timer0");
	m_timer[1] = machine().device<timer_device>("timer1");
}


MACHINE_RESET_MEMBER(midvunit_state,midvplus)
{
	m_dcs->reset_w(1);
	m_dcs->reset_w(0);

	memcpy(m_ram_base, memregion("user1")->base(), 0x20000*4);
	m_maincpu->reset();

	m_timer[0] = machine().device<timer_device>("timer0");
	m_timer[1] = machine().device<timer_device>("timer1");

	machine().device("ata")->reset();
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

READ32_MEMBER(midvunit_state::port0_r)
{
	UINT16 val = ioport("IN0")->read();
	UINT16 diff = val ^ m_last_port0;

	/* make sure the shift controls are mutually exclusive */
	if ((diff & 0x0400) && !(val & 0x0400))
		m_shifter_state = (m_shifter_state == 1) ? 0 : 1;
	if ((diff & 0x0800) && !(val & 0x0800))
		m_shifter_state = (m_shifter_state == 2) ? 0 : 2;
	if ((diff & 0x1000) && !(val & 0x1000))
		m_shifter_state = (m_shifter_state == 4) ? 0 : 4;
	if ((diff & 0x2000) && !(val & 0x2000))
		m_shifter_state = (m_shifter_state == 8) ? 0 : 8;
	m_last_port0 = val;

	val = (val | 0x3c00) ^ (m_shifter_state << 10);

	return (val << 16) | val;
}


/*************************************
 *
 *  ADC input ports
 *
 *************************************/

READ32_MEMBER(midvunit_state::midvunit_adc_r)
{
	if (!(m_control_data & 0x40))
	{
		m_maincpu->set_input_line(3, CLEAR_LINE);
		return m_adc_data << m_adc_shift;
	}
	else
		logerror("adc_r without enabling reads!\n");
	return 0xffffffff;
}


WRITE32_MEMBER(midvunit_state::midvunit_adc_w)
{
	static const char *const adcnames[] = { "WHEEL", "ACCEL", "BRAKE" };

	if (!(m_control_data & 0x20))
	{
		int which = (data >> m_adc_shift) - 4;
		if (which < 0 || which > 2)
			logerror("adc_w: unexpected which = %02X\n", which + 4);
		m_adc_data = ioport(adcnames[which])->read_safe(0);
		timer_set(attotime::from_msec(1), TIMER_ADC_READY);
	}
	else
		logerror("adc_w without enabling writes!\n");
}



/*************************************
 *
 *  CMOS access
 *
 *************************************/

WRITE32_MEMBER(midvunit_state::midvunit_cmos_protect_w)
{
	m_cmos_protected = ((data & 0xc00) != 0xc00);
}


WRITE32_MEMBER(midvunit_state::midvunit_cmos_w)
{
	if (!m_cmos_protected)
		COMBINE_DATA(m_nvram + offset);
}


READ32_MEMBER(midvunit_state::midvunit_cmos_r)
{
	return m_nvram[offset];
}



/*************************************
 *
 *  System controls
 *
 *************************************/

WRITE32_MEMBER(midvunit_state::midvunit_control_w)
{
	UINT16 olddata = m_control_data;
	COMBINE_DATA(&m_control_data);

	/* bit 7 is the LED */

	/* bit 3 is the watchdog */
	if ((olddata ^ m_control_data) & 0x0008)
		watchdog_reset_w(space, 0, 0);

	/* bit 1 is the DCS sound reset */
	m_dcs->reset_w((~m_control_data >> 1) & 1);

	/* log anything unusual */
	if ((olddata ^ m_control_data) & ~0x00e8)
		logerror("midvunit_control_w: old=%04X new=%04X diff=%04X\n", olddata, m_control_data, olddata ^ m_control_data);
}


WRITE32_MEMBER(midvunit_state::crusnwld_control_w)
{
	UINT16 olddata = m_control_data;
	COMBINE_DATA(&m_control_data);

	/* bit 11 is the DCS sound reset */
	m_dcs->reset_w((~m_control_data >> 11) & 1);

	/* bit 9 is the watchdog */
	if ((olddata ^ m_control_data) & 0x0200)
		watchdog_reset_w(space, 0, 0);

	/* bit 8 is the LED */

	/* log anything unusual */
	if ((olddata ^ m_control_data) & ~0xe800)
		logerror("crusnwld_control_w: old=%04X new=%04X diff=%04X\n", olddata, m_control_data, olddata ^ m_control_data);
}


WRITE32_MEMBER(midvunit_state::midvunit_sound_w)
{
	logerror("Sound W = %02X\n", data);
	m_dcs->data_w(data & 0xff);
}



/*************************************
 *
 *  TMS32031 I/O accesses
 *
 *************************************/

READ32_MEMBER(midvunit_state::tms32031_control_r)
{
	/* watch for accesses to the timers */
	if (offset == 0x24 || offset == 0x34)
	{
		/* timer is clocked at 100ns */
		int which = (offset >> 4) & 1;
		INT32 result = (m_timer[which]->time_elapsed() * m_timer_rate).as_double();
//      logerror("%06X:tms32031_control_r(%02X) = %08X\n", space.device().safe_pc(), offset, result);
		return result;
	}

	/* log anything else except the memory control register */
	if (offset != 0x64)
		logerror("%06X:tms32031_control_r(%02X)\n", space.device().safe_pc(), offset);

	return m_tms32031_control[offset];
}


WRITE32_MEMBER(midvunit_state::tms32031_control_w)
{
	COMBINE_DATA(&m_tms32031_control[offset]);

	/* ignore changes to the memory control register */
	if (offset == 0x64)
		;

	/* watch for accesses to the timers */
	else if (offset == 0x20 || offset == 0x30)
	{
		int which = (offset >> 4) & 1;
//  logerror("%06X:tms32031_control_w(%02X) = %08X\n", space.device().safe_pc(), offset, data);
		if (data & 0x40)
			m_timer[which]->reset();

		/* bit 0x200 selects internal clocking, which is 1/2 the main CPU clock rate */
		if (data & 0x200)
			m_timer_rate = (double)(m_maincpu->unscaled_clock() * 0.5);
		else
			m_timer_rate = 10000000.;
	}
	else
		logerror("%06X:tms32031_control_w(%02X) = %08X\n", space.device().safe_pc(), offset, data);
}



/*************************************
 *
 *  Serial number access
 *
 *************************************/

READ32_MEMBER(midvunit_state::crusnwld_serial_status_r)
{
	int status = m_midway_serial_pic->status_r(space,0);
	return (ioport("991030")->read() & 0x7fff7fff) | (status << 31) | (status << 15);
}


READ32_MEMBER(midvunit_state::crusnwld_serial_data_r)
{
	return m_midway_serial_pic->read(space,0) << 16;
}


WRITE32_MEMBER(midvunit_state::crusnwld_serial_data_w)
{
	if ((data & 0xf0000) == 0x10000)
	{
		m_midway_serial_pic->reset_w(1);
		m_midway_serial_pic->reset_w(0);
	}
	m_midway_serial_pic->write(space,0,data >> 16);
}


/*************************************
 *
 *  Some kind of protection-like
 *  device
 *
 *************************************/

/* values from offset 3, 6, and 10 must add up to 0x904752a2 */
static const UINT32 bit_data[0x10] =
{
	0x3017c636,0x3017c636,0x3017c636,0x3017c636,
	0x3017c636,0x3017c636,0x3017c636,0x3017c636,
	0x3017c636,0x3017c636,0x3017c636,0x3017c636,
	0x3017c636,0x3017c636,0x3017c636,0x3017c636
};


READ32_MEMBER(midvunit_state::bit_data_r)
{
	int bit = (bit_data[m_bit_index / 32] >> (31 - (m_bit_index % 32))) & 1;
	m_bit_index = (m_bit_index + 1) % 512;
	return bit ? m_nvram[offset] : ~m_nvram[offset];
}


WRITE32_MEMBER(midvunit_state::bit_reset_w)
{
	m_bit_index = 0;
}



/*************************************
 *
 *  Off Road Challenge PIC access
 *
 *************************************/

READ32_MEMBER(midvunit_state::offroadc_serial_status_r)
{
	int status = m_midway_serial_pic2->status_r(space,0);
	return (ioport("991030")->read()  & 0x7fff7fff) | (status << 31) | (status << 15);
}


READ32_MEMBER(midvunit_state::offroadc_serial_data_r)
{
	return m_midway_serial_pic2->read(space, 0) << 16;
}


WRITE32_MEMBER(midvunit_state::offroadc_serial_data_w)
{
	m_midway_serial_pic2->write(space, 0, data >> 16);
}



/*************************************
 *
 *  War Gods I/O ASICs
 *
 *************************************/

READ32_MEMBER(midvunit_state::midvplus_misc_r)
{
	UINT32 result = m_midvplus_misc[offset];

	switch (offset)
	{
		case 0:
			result = 0xb580;
			break;

		case 2:
			result = 0xf3ff;
			break;

		case 3:
			/* seems to want loopback */
			break;
	}

	if (offset != 0 && offset != 3)
		logerror("%06X:midvplus_misc_r(%d) = %08X\n", space.device().safe_pc(), offset, result);
	return result;
}


WRITE32_MEMBER(midvunit_state::midvplus_misc_w)
{
	UINT32 olddata = m_midvplus_misc[offset];
	int logit = 1;

	COMBINE_DATA(&m_midvplus_misc[offset]);

	switch (offset)
	{
		case 0:
			/* bit 0x10 resets watchdog */
			if ((olddata ^ m_midvplus_misc[offset]) & 0x0010)
			{
				watchdog_reset_w(space, 0, 0);
				logit = 0;
			}
			break;

		case 3:
			logit = 0;
			break;
	}

	if (logit)
		logerror("%06X:midvplus_misc_w(%d) = %08X\n", space.device().safe_pc(), offset, data);
}



/*************************************
 *
 *  War Gods RAM grossness
 *
 *************************************/

WRITE8_MEMBER(midvunit_state::midvplus_xf1_w)
{
//  osd_printf_debug("xf1_w = %d\n", data);

	if (m_lastval && !data)
		memcpy(m_ram_base, m_fastram_base, 0x20000*4);

	m_lastval = data;
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( midvunit_map, AS_PROGRAM, 32, midvunit_state )
	AM_RANGE(0x000000, 0x01ffff) AM_RAM AM_SHARE("ram_base")
	AM_RANGE(0x400000, 0x41ffff) AM_RAM
	AM_RANGE(0x600000, 0x600000) AM_WRITE(midvunit_dma_queue_w)
	AM_RANGE(0x808000, 0x80807f) AM_READWRITE(tms32031_control_r, tms32031_control_w) AM_SHARE("32031_control")
	AM_RANGE(0x809800, 0x809fff) AM_RAM
	AM_RANGE(0x900000, 0x97ffff) AM_READWRITE(midvunit_videoram_r, midvunit_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x980000, 0x980000) AM_READ(midvunit_dma_queue_entries_r)
	AM_RANGE(0x980020, 0x980020) AM_READ(midvunit_scanline_r)
	AM_RANGE(0x980020, 0x98002b) AM_WRITE(midvunit_video_control_w)
	AM_RANGE(0x980040, 0x980040) AM_READWRITE(midvunit_page_control_r, midvunit_page_control_w)
	AM_RANGE(0x980080, 0x980080) AM_NOP
	AM_RANGE(0x980082, 0x980083) AM_READ(midvunit_dma_trigger_r)
	AM_RANGE(0x990000, 0x990000) AM_READNOP // link PAL (low 4 bits must == 4)
	AM_RANGE(0x991030, 0x991030) AM_READ_PORT("991030")
//  AM_RANGE(0x991050, 0x991050) AM_READONLY // seems to be another port
	AM_RANGE(0x991060, 0x991060) AM_READ(port0_r)
	AM_RANGE(0x992000, 0x992000) AM_READ_PORT("992000")
	AM_RANGE(0x993000, 0x993000) AM_READWRITE(midvunit_adc_r, midvunit_adc_w)
	AM_RANGE(0x994000, 0x994000) AM_WRITE(midvunit_control_w)
	AM_RANGE(0x995000, 0x995000) AM_WRITENOP    // force feedback?
	AM_RANGE(0x995020, 0x995020) AM_WRITE(midvunit_cmos_protect_w)
	AM_RANGE(0x997000, 0x997000) AM_NOP // communications
	AM_RANGE(0x9a0000, 0x9a0000) AM_WRITE(midvunit_sound_w)
	AM_RANGE(0x9c0000, 0x9c1fff) AM_READWRITE(midvunit_cmos_r, midvunit_cmos_w) AM_SHARE("nvram")
	AM_RANGE(0x9e0000, 0x9e7fff) AM_RAM_WRITE(midvunit_paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0xa00000, 0xbfffff) AM_READWRITE(midvunit_textureram_r, midvunit_textureram_w) AM_SHARE("textureram")
	AM_RANGE(0xc00000, 0xffffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END


static ADDRESS_MAP_START( midvplus_map, AS_PROGRAM, 32, midvunit_state )
	AM_RANGE(0x000000, 0x01ffff) AM_RAM AM_SHARE("ram_base")
	AM_RANGE(0x400000, 0x41ffff) AM_RAM AM_SHARE("fastram_base")
	AM_RANGE(0x600000, 0x600000) AM_WRITE(midvunit_dma_queue_w)
	AM_RANGE(0x808000, 0x80807f) AM_READWRITE(tms32031_control_r, tms32031_control_w) AM_SHARE("32031_control")
	AM_RANGE(0x809800, 0x809fff) AM_RAM
	AM_RANGE(0x900000, 0x97ffff) AM_READWRITE(midvunit_videoram_r, midvunit_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x980000, 0x980000) AM_READ(midvunit_dma_queue_entries_r)
	AM_RANGE(0x980020, 0x980020) AM_READ(midvunit_scanline_r)
	AM_RANGE(0x980020, 0x98002b) AM_WRITE(midvunit_video_control_w)
	AM_RANGE(0x980040, 0x980040) AM_READWRITE(midvunit_page_control_r, midvunit_page_control_w)
	AM_RANGE(0x980080, 0x980080) AM_NOP
	AM_RANGE(0x980082, 0x980083) AM_READ(midvunit_dma_trigger_r)
	AM_RANGE(0x990000, 0x99000f) AM_DEVREADWRITE("ioasic", midway_ioasic_device, read, write)
	AM_RANGE(0x994000, 0x994000) AM_WRITE(midvunit_control_w)
	AM_RANGE(0x995020, 0x995020) AM_WRITE(midvunit_cmos_protect_w)
	AM_RANGE(0x9a0000, 0x9a0007) AM_DEVREADWRITE16("ata", ata_interface_device, read_cs0, write_cs0, 0x0000ffff)
	AM_RANGE(0x9c0000, 0x9c7fff) AM_RAM_WRITE(midvunit_paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0x9d0000, 0x9d000f) AM_READWRITE(midvplus_misc_r, midvplus_misc_w) AM_SHARE("midvplus_misc")
	AM_RANGE(0xa00000, 0xbfffff) AM_READWRITE(midvunit_textureram_r, midvunit_textureram_w) AM_SHARE("textureram")
	AM_RANGE(0xc00000, 0xcfffff) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( crusnusa )
	PORT_START("991030")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN1")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN1")

	PORT_START("992000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "DSW")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "DSW")

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) /* Slam Switch */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Enter") PORT_CODE(KEYCODE_F2) /* Test switch */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("4th Gear")    /* 4th */
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("3rd Gear")    /* 3rd */
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("2nd Gear")    /* 2nd */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("1st Gear")    /* 1st */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Radio")   /* radio */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("View 1")  /* view 1 */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("View 2")  /* view 2 */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("View 3") /* view 3 */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("View 4") /* view 4 */
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	/* DSW2 at U97 */
	PORT_DIPNAME( 0x0001, 0x0000, "Link Status" )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, "Master" )
	PORT_DIPSETTING(      0x0001, "Slave" )
	PORT_DIPNAME( 0x0002, 0x0002, "Link???" )       PORT_DIPLOCATION("SW2:7") /* Listed as Not Used in the manual */
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Linking" )       PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:5") /* Listed as Not Used in the manual */
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Freeze" )        PORT_DIPLOCATION("SW2:4") /* Listed as Not Used in the manual */
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, "Sitdown" )
	PORT_DIPNAME( 0x0040, 0x0040, "Enable Motion" )     PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:1" ) /* Listed as Not Used in the manual */

	/* DSW3 at U19 */
	PORT_DIPNAME( 0x0100, 0x0100, "Coin Counters" )     PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0xfe00, 0xf800, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW3:7,6,5,4,3,2,1")
	PORT_DIPSETTING(      0xfe00, "USA-1" )
	PORT_DIPSETTING(      0xfa00, "USA-3" )
	PORT_DIPSETTING(      0xfc00, "USA-7" )
	PORT_DIPSETTING(      0xf800, "USA-8" )
	PORT_DIPSETTING(      0xf600, "Norway-1" )
	PORT_DIPSETTING(      0xee00, "Australia-1" )
	PORT_DIPSETTING(      0xea00, "Australia-2" )
	PORT_DIPSETTING(      0xec00, "Australia-3" )
	PORT_DIPSETTING(      0xe800, "Australia-4" )
	PORT_DIPSETTING(      0xde00, "Swiss-1" )
	PORT_DIPSETTING(      0xda00, "Swiss-2" )
	PORT_DIPSETTING(      0xdc00, "Swiss-3" )
	PORT_DIPSETTING(      0xce00, "Belgium-1" )
	PORT_DIPSETTING(      0xca00, "Belgium-2" )
	PORT_DIPSETTING(      0xcc00, "Belgium-3" )
	PORT_DIPSETTING(      0xbe00, "French-1" )
	PORT_DIPSETTING(      0xba00, "French-2" )
	PORT_DIPSETTING(      0xbc00, "French-3" )
	PORT_DIPSETTING(      0xb800, "French-4" )
	PORT_DIPSETTING(      0xb600, "Hungary-1" )
	PORT_DIPSETTING(      0xae00, "Taiwan-1" )
	PORT_DIPSETTING(      0xaa00, "Taiwan-2" )
	PORT_DIPSETTING(      0xac00, "Taiwan-3" )
	PORT_DIPSETTING(      0x9e00, "UK-1" )
	PORT_DIPSETTING(      0x9a00, "UK-2" )
	PORT_DIPSETTING(      0x9c00, "UK-3" )
	PORT_DIPSETTING(      0x8e00, "Finland-1" )
	PORT_DIPSETTING(      0x7e00, "German-1" )
	PORT_DIPSETTING(      0x7a00, "German-2" )
	PORT_DIPSETTING(      0x7c00, "German-3" )
	PORT_DIPSETTING(      0x7800, "German-4" )
	PORT_DIPSETTING(      0x7600, "Denmark-1" )
	PORT_DIPSETTING(      0x6e00, "Japan-1" )
	PORT_DIPSETTING(      0x6a00, "Japan-2" )
	PORT_DIPSETTING(      0x6c00, "Japan-3" )
	PORT_DIPSETTING(      0x5e00, "Italy-1" )
	PORT_DIPSETTING(      0x5a00, "Italy-2" )
	PORT_DIPSETTING(      0x5c00, "Italy-3" )
	PORT_DIPSETTING(      0x4e00, "Sweden-1" )
	PORT_DIPSETTING(      0x3e00, "Canada-1" )
	PORT_DIPSETTING(      0x3a00, "Canada-2" )
	PORT_DIPSETTING(      0x3c00, "Canada-3" )
	PORT_DIPSETTING(      0x3600, "General-1" )
	PORT_DIPSETTING(      0x3200, "General-3" )
	PORT_DIPSETTING(      0x3400, "General-5" )
	PORT_DIPSETTING(      0x3000, "General-7" )
	PORT_DIPSETTING(      0x2e00, "Austria-1" )
	PORT_DIPSETTING(      0x2a00, "Austria-2" )
	PORT_DIPSETTING(      0x2c00, "Austria-3" )
	PORT_DIPSETTING(      0x2800, "Austria-4" )
	PORT_DIPSETTING(      0x1e00, "Spain-1" )
	PORT_DIPSETTING(      0x1a00, "Spain-2" )
	PORT_DIPSETTING(      0x1c00, "Spain-3" )
	PORT_DIPSETTING(      0x1800, "Spain-4" )
	PORT_DIPSETTING(      0x0e00, "Netherland-1" )

	PORT_START("WHEEL")     /* wheel */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("ACCEL")     /* gas pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("BRAKE")     /* brake pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)
INPUT_PORTS_END


static INPUT_PORTS_START( crusnwld )
	PORT_START("991030")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN1")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN1")

	PORT_START("992000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "DSW")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "DSW")

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) /* Slam Switch */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Enter") PORT_CODE(KEYCODE_F2) /* Test switch */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("4th Gear")    /* 4th */
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("3rd Gear")    /* 3rd */
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("2nd Gear")    /* 2nd */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("1st Gear")    /* 1st */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Radio")   /* radio */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("View 1")  /* view 1 */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("View 2")  /* view 2 */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("View 3") /* view 3 */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("View 4") /* view 4 */
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	/* DSW2 at U97 */
	PORT_DIPNAME( 0x0003, 0x0000, "Link Number" )       PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0001, "2" )
	PORT_DIPSETTING(      0x0002, "3" )
	PORT_DIPSETTING(      0x0003, "4" )
	PORT_DIPNAME( 0x0004, 0x0004, "Linking" )       PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0008, "Games Linked" )      PORT_DIPLOCATION("SW2:5,4")
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x0010, "3" )
	PORT_DIPSETTING(      0x0018, "4" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, "Sitdown" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:2" )        /* Manual shows Not Used */
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:1" )

	/* DSW3 at U19 */
	PORT_DIPNAME( 0x0100, 0x0100, "Coin Counters" )     PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0xfe00, 0xf800, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW3:7,6,5,4,3,2,1")
	PORT_DIPSETTING(      0xfe00, "USA-1" )
	PORT_DIPSETTING(      0xfa00, "USA-3" )
	PORT_DIPSETTING(      0xfc00, "USA-7" )
	PORT_DIPSETTING(      0xf800, "USA-8" )
	PORT_DIPSETTING(      0xf600, "Norway-1" )
	PORT_DIPSETTING(      0xee00, "Australia-1" )
	PORT_DIPSETTING(      0xea00, "Australia-2" )
	PORT_DIPSETTING(      0xec00, "Australia-3" )
	PORT_DIPSETTING(      0xe800, "Australia-4" )
	PORT_DIPSETTING(      0xde00, "Swiss-1" )
	PORT_DIPSETTING(      0xda00, "Swiss-2" )
	PORT_DIPSETTING(      0xdc00, "Swiss-3" )
	PORT_DIPSETTING(      0xce00, "Belgium-1" )
	PORT_DIPSETTING(      0xca00, "Belgium-2" )
	PORT_DIPSETTING(      0xcc00, "Belgium-3" )
	PORT_DIPSETTING(      0xbe00, "French-1" )
	PORT_DIPSETTING(      0xba00, "French-2" )
	PORT_DIPSETTING(      0xbc00, "French-3" )
	PORT_DIPSETTING(      0xb800, "French-4" )
	PORT_DIPSETTING(      0xb600, "Hungary-1" )
	PORT_DIPSETTING(      0xae00, "Taiwan-1" )
	PORT_DIPSETTING(      0xaa00, "Taiwan-2" )
	PORT_DIPSETTING(      0xac00, "Taiwan-3" )
	PORT_DIPSETTING(      0x9e00, "UK-1" )
	PORT_DIPSETTING(      0x9a00, "UK-2" )
	PORT_DIPSETTING(      0x9c00, "UK-3" )
	PORT_DIPSETTING(      0x8e00, "Finland-1" )
	PORT_DIPSETTING(      0x7e00, "German-1" )
	PORT_DIPSETTING(      0x7a00, "German-2" )
	PORT_DIPSETTING(      0x7c00, "German-3" )
	PORT_DIPSETTING(      0x7800, "German-4" )
	PORT_DIPSETTING(      0x7600, "Denmark-1" )
	PORT_DIPSETTING(      0x6e00, "Japan-1" )
	PORT_DIPSETTING(      0x6a00, "Japan-2" )
	PORT_DIPSETTING(      0x6c00, "Japan-3" )
	PORT_DIPSETTING(      0x5e00, "Italy-1" )
	PORT_DIPSETTING(      0x5a00, "Italy-2" )
	PORT_DIPSETTING(      0x5c00, "Italy-3" )
	PORT_DIPSETTING(      0x4e00, "Sweden-1" )
	PORT_DIPSETTING(      0x3e00, "Canada-1" )
	PORT_DIPSETTING(      0x3a00, "Canada-2" )
	PORT_DIPSETTING(      0x3c00, "Canada-3" )
	PORT_DIPSETTING(      0x3600, "General-1" )
	PORT_DIPSETTING(      0x3200, "General-3" )
	PORT_DIPSETTING(      0x3400, "General-5" )
	PORT_DIPSETTING(      0x3000, "General-7" )
	PORT_DIPSETTING(      0x2e00, "Austria-1" )
	PORT_DIPSETTING(      0x2a00, "Austria-2" )
	PORT_DIPSETTING(      0x2c00, "Austria-3" )
	PORT_DIPSETTING(      0x2800, "Austria-4" )
	PORT_DIPSETTING(      0x1e00, "Spain-1" )
	PORT_DIPSETTING(      0x1a00, "Spain-2" )
	PORT_DIPSETTING(      0x1c00, "Spain-3" )
	PORT_DIPSETTING(      0x1800, "Spain-4" )
	PORT_DIPSETTING(      0x0e00, "Netherland-1" )

	PORT_START("WHEEL")     /* wheel */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("ACCEL")     /* gas pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("BRAKE")     /* brake pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)
INPUT_PORTS_END


static INPUT_PORTS_START( offroadc )
	PORT_START("991030")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN1")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "IN1")

	PORT_START("992000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "DSW")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "DSW")

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) /* Slam Switch */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Enter") PORT_CODE(KEYCODE_F2) /* Test switch */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("4th Gear")    /* 4th */
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("3rd Gear")    /* 3rd */
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("2nd Gear")    /* 2nd */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("1st Gear")    /* 1st */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Radio")   /* radio */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("View 1")  /* view 1 */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("View 2")  /* view 2 */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("View 3") /* view 3 */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("View 4") /* view 4 */
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	/* DSW2 at U97 */
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SW2:8" )        /* Manual shows Not Used & "No Effect" for both On & Off */
	PORT_DIPNAME( 0x0002, 0x0000, "Gear Shifter Switch" )       PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0002, "Normally Closed" )
	PORT_DIPSETTING(      0x0000, "Normally Open" )
	PORT_DIPNAME( 0x0004, 0x0004, "Added Attractions" )     PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0004, "Girls Present" )
	PORT_DIPSETTING(      0x0000, "Girls Missing" )
	PORT_DIPNAME( 0x0008, 0x0008, "Graphic Effects" )       PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0008, "Roadkill Present" )
	PORT_DIPSETTING(      0x0000, "Roadkill Missing" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:4" )        /* Manual shows Not Used & "No Effect" for both On & Off */
	PORT_DIPNAME( 0x0020, 0x0020, "Link" )              PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0020, "Disabled" )
	PORT_DIPSETTING(      0x0000, "Enabled" )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Link Machine" )          PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(      0x00c0, "1" )
	PORT_DIPSETTING(      0x0080, "2" )
	PORT_DIPSETTING(      0x0040, "3" )
	PORT_DIPSETTING(      0x0000, "4" )

	/* DSW3 at U19 */
	PORT_DIPUNUSED_DIPLOC( 0x0100, 0x0100, "SW3:8" )    /* Manual states "Switches 6, 7 and 8 are not active. We recommend */
	PORT_DIPUNUSED_DIPLOC( 0x0200, 0x0200, "SW3:7" )    /* they be set to the facorty default (OFF) positions."            */
	PORT_DIPUNUSED_DIPLOC( 0x0400, 0x0400, "SW3:6" )
	PORT_DIPNAME( 0xf800, 0xf800, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW3:5,4,3,2,1")
	PORT_DIPSETTING(      0xf800, "USA 1" )
	PORT_DIPSETTING(      0xf000, "German 1" )
	PORT_DIPSETTING(      0xe800, "French 1" )
	PORT_DIPSETTING(      0xe000, "Canada 1" )
	PORT_DIPSETTING(      0xd800, "Swiss 1" )
	PORT_DIPSETTING(      0xd000, "Italy 1" )
	PORT_DIPSETTING(      0xc800, "UK 1" )
	PORT_DIPSETTING(      0xc000, "Spain 1" )
	PORT_DIPSETTING(      0xb800, "Australia 1" )
	PORT_DIPSETTING(      0xb000, "Japan 1" )
	PORT_DIPSETTING(      0xa800, "Taiwan 1" )
	PORT_DIPSETTING(      0xa000, "Austria 1" )
	PORT_DIPSETTING(      0x9800, "Belgium 1" )
	PORT_DIPSETTING(      0x9000, "Sweden 1" )
	PORT_DIPSETTING(      0x8800, "Finland 1" )
	PORT_DIPSETTING(      0x8000, "Netherlands 1" )
	PORT_DIPSETTING(      0x7800, "Norway 1" )
	PORT_DIPSETTING(      0x7000, "Denmark 1" )
	PORT_DIPSETTING(      0x6800, "Hungary 1" )

	PORT_START("WHEEL")     /* wheel */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("ACCEL")     /* gas pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("BRAKE")     /* brake pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)
INPUT_PORTS_END


static INPUT_PORTS_START( wargods )
	PORT_START("DIPS")
	PORT_DIPNAME( 0x0001, 0x0001, "CRT Type / Resolution" )     PORT_DIPLOCATION("SW1:1") /* This only works for the Dual Res version */
	PORT_DIPSETTING(      0x0001, "Medium Res (24Khz)" )
	PORT_DIPSETTING(      0x0000, "Standard Res (15Khz)" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:2") /* Manual shows Not Used (must be Off) */
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3") /* Manual shows Not Used (must be Off) */
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Blood" )             PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Graphics" )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Family" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:6") /* Manual shows Not Used */
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:7") /* Manual shows Not Used */
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8") /* Manual shows Not Used */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "Coinage Source" )        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPNAME( 0x3e00, 0x3e00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:2,3,4,5,6")
	PORT_DIPSETTING(      0x3e00, "USA-1" )
	PORT_DIPSETTING(      0x3c00, "USA-2" )
	PORT_DIPSETTING(      0x3a00, "USA-3" )
	PORT_DIPSETTING(      0x3800, "USA-4" )
	PORT_DIPSETTING(      0x3400, "USA-9" )
	PORT_DIPSETTING(      0x3200, "USA-10" )
	PORT_DIPSETTING(      0x3600, "USA-ECA" )
	PORT_DIPSETTING(      0x2e00, "German-1" )
	PORT_DIPSETTING(      0x2c00, "German-2" )
	PORT_DIPSETTING(      0x2a00, "German-3" )
	PORT_DIPSETTING(      0x2800, "German-4" )
	PORT_DIPSETTING(      0x2400, "German-5" )
	PORT_DIPSETTING(      0x2600, "German-ECA" )
	PORT_DIPSETTING(      0x1e00, "French-1" )
	PORT_DIPSETTING(      0x1c00, "French-2" )
	PORT_DIPSETTING(      0x1a00, "French-3" )
	PORT_DIPSETTING(      0x1800, "French-4" )
	PORT_DIPSETTING(      0x1400, "French-11" )
	PORT_DIPSETTING(      0x1200, "French-12" )
	PORT_DIPSETTING(      0x1600, "French-ECA" )
	PORT_DIPSETTING(      0x3000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7") /* Manual shows Not Used */
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Test Switch" )           PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT )     /* Slam Switch */
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BILL1 )    /* Bill */

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( wargodsa ) /* For Medium Res only versions */
	PORT_INCLUDE(wargods)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:1") /* Manual shows Not Used (must be Off) */
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( midvcommon, midvunit_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS32031, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(midvunit_map)

	MCFG_NVRAM_ADD_1FILL("nvram")

	MCFG_TIMER_ADD_NONE("timer0")
	MCFG_TIMER_ADD_NONE("timer1")

	/* video hardware */
	MCFG_PALETTE_ADD("palette", 32768)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MIDVUNIT_VIDEO_CLOCK/2, 666, 0, 512, 432, 0, 400)
	MCFG_SCREEN_UPDATE_DRIVER(midvunit_state, screen_update_midvunit)
	MCFG_SCREEN_PALETTE("palette")

MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( midvunit, midvcommon )

	/* sound hardware */
	MCFG_DEVICE_ADD("dcs", DCS_AUDIO_2K, 0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( crusnwld, midvunit )
	/* valid values are 450 or 460 */
	MCFG_DEVICE_ADD("serial_pic", MIDWAY_SERIAL_PIC, 0)
	MCFG_MIDWAY_SERIAL_PIC_UPPER(450)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( offroadc, midvunit )
	/* valid values are 230 or 234 */
	MCFG_DEVICE_ADD("serial_pic2", MIDWAY_SERIAL_PIC2, 0)
	MCFG_MIDWAY_SERIAL_PIC2_UPPER(230)
	MCFG_MIDWAY_SERIAL_PIC2_YEAR_OFFS(94)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( midvplus, midvcommon )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(midvplus_map)
	MCFG_TMS3203X_XF1_CB(WRITE8(midvunit_state, midvplus_xf1_w))

	MCFG_MACHINE_RESET_OVERRIDE(midvunit_state,midvplus)
	MCFG_DEVICE_REMOVE("nvram")

	MCFG_ATA_INTERFACE_ADD("ata", ata_devices, "hdd", NULL, true)

	MCFG_DEVICE_ADD("ioasic", MIDWAY_IOASIC, 0)
	MCFG_MIDWAY_IOASIC_SHUFFLE(0)
	MCFG_MIDWAY_IOASIC_UPPER(452) /* no alternates */
	MCFG_MIDWAY_IOASIC_YEAR_OFFS(94)

	/* sound hardware */
	MCFG_DEVICE_ADD("dcs", DCS2_AUDIO_2115, 0)
	MCFG_DCS2_AUDIO_DRAM_IN_MB(2)
	MCFG_DCS2_AUDIO_POLLING_OFFSET(0x3839)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

/*
Cruis'n USA & Offroad Challenge (Midway V-Unit)
Midway, 1994

PCB Layout
----------

5770-14365-02 (C) 1994 NINTENDO (for Cruisin USA)
5770-14365-05 WILLIAMS ELECTRONICS GAMES INC. (for Offroad Challenge)
|-------------------------------------------------------------------------------|
|  SOUND.U5  SOUND.U9   BATTERY  MAX691  GAME.U26  GAME.U27  GAME.U28  GAME.U29 |
|  SOUND.U4  SOUND.U8                    GAME.U22  GAME.U23  GAME.U24  GAME.U25 |
|  SOUND.U3  SOUND.U7   6264  RESET_SW   GAME.U18  GAME.U19  GAME.U20  GAME.U21 |
|  SOUND.U2  SOUND.U6         PIC16C57   GAME.U14  GAME.U15  GAME.U16  GAME.U17 |
|                                        GAME.U10  GAME.U11  GAME.U12  GAME.U13 |
|            6116            PAL2              IDT7204      IDT7204             |
|                                          LH521007 LH521007   LH521007 LH521007|
|            6116   6116     PAL3                                               |
|             PAL1           PAL4      |----------|    4C4001 4C4001  |-------| |
|AD1851                      33.3333MHz|LSI       |    4C4001 4C4001  |TMS    | |
|                                      |L1A7968   |    4C4001 4C4001  |320C31 | |
|                                 40MHz|5410-1346500   4C4001 4C4001  |DSP    | |
|                                      |MIDWAY    |                   |-------| |
|      10MHz   ADSP-2105   CY7C199     |----------|                   50MHz     |
|TDA2030                   CY7C199                                        DSW(8)|
|       TL084                             D482234  D482234                      |
|     TL084                               D482234  D482234                DSW(8)|
|TDA2030                        PAL5            SN75160   SN75160   PAL6        |
|          P5                             P7      SN75176  SN75176    P11       |
|P3      P4  |--|        JAMMA         |--|P8 P9  SN75176  SN75176 P10   ADC0844|
|------------|  |----------------------|  |-------------------------------------|
Notes:
      TMS320C31 - Texas Instruments TMS320C31 32-bit Floating-Point DSP, clock input 50.000MHz
      ADSP-2105 - Analog Devices ADSP-2105 16-bit Fixed-Point DSP Microprocessor with On-Chip Memory, clock input 10.000MHz
      IDT7204   - IDT7204 4k x9 Async. FIFO
      LH521007  - Sharp LH521007AK-17 128k x8 SRAM (SOJ32)
      D482234   - NEC D482234LE-70 (possibly 256k x8 ?) DRAM (SOJ40)
      4C4001    - Micron Technology 4C4001JDJ-6 1M x4 DRAM (SOJ24/20)
      CY7C199   - Cypress CY7C199-20PC 32k x8 SRAM
      6264      - 8k x8 SRAM (battery-backed)
      6116      - 2k x8 SRAM
      AD1851    - Analog Devices AD1851 16 bit PCM Audio DAC
      MAX691    - Maxim MAX691 Master Reset IC (DIP16)
      TDA2030   - ST TDA2030 Audio AMP
      TL084     - Texas Instruments TL084 JFET-Input Operational Amplifier
      ADC0844   - National Semiconductor ADC0844 8-Bit Microprocessor Compatible A/D Converter with Multiplexer Option
      PIC16C57  - Microchip PIC16C57
                    - not populated for Cruis'n USA
                    - labelled 'Offroad 25" U904' for Offroad Challenge
      PAL1      - GAL20V8 labelled 'A-19668'
      PAL2      - PALC22V10
                    - no label for Cruisin USA
                    - labelled 'A-21883 U38' for Offroad Challenge
      PAL3      - TIBPAL20L8
                    - labelled 'A-19670' for Cruis'n USA
                    - labelled 'A-21170 U43' for Offroad Challenge
      PAL4      - TIBPAL22V10
                    - labelled 'A-19671' for Cruis'n USA
                    - labelled 'A-21171 U54' for Offroad Challenge
      PAL5      - TIBPAL22V10
                    - labelled 'A-19672' for Cruis'n USA
                    - labelled 'A-21884 U114' for Offroad Challenge
      PAL6      - TIBPAL22V10
                    - labelled 'A-19673' for Cruis'n USA
                    - no label for Offroad Challenge
      P3 - P11  - various connectors for controls
      VSync     - 57.7090Hz  \
      HSync     - 24.807kHz  / measured via EL4583
      ROMs      - All ROMs 27C040 or 27C801
                  SOUND.Uxx - Sound ROMs
                  GAME.Uxx  - PROGRAM ROMs (including GFX)
*/

ROM_START( crusnusa ) /* Version 4.1, Mon Feb 13 1995 - 16:53:40 */
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "cusa.u2",  0x000000, 0x80000, CRC(b9338332) SHA1(e5c420e63c4eba0010a68c7e0a57ef210e2c83d2) )
	ROM_LOAD16_BYTE( "cusa.u3",  0x200000, 0x80000, CRC(cd8325d6) SHA1(d65d7263e056ca1d637adb44cafef523e0831a34) )
	ROM_LOAD16_BYTE( "cusa.u4",  0x400000, 0x80000, CRC(fab457f3) SHA1(2b4b647838b7a8100afc25ca1ffdc74ed67ae00a) )
	ROM_LOAD16_BYTE( "cusa.u5",  0x600000, 0x80000, CRC(becc92f4) SHA1(6dffa73ff5270155c44f295e443d5e77c03c0338) )
	ROM_LOAD16_BYTE( "cusa.u6",  0x800000, 0x80000, CRC(a9f915d3) SHA1(6a16a2d7a807a775673e7121b54f37c583581203) )
	ROM_LOAD16_BYTE( "cusa.u7",  0xa00000, 0x80000, CRC(424f0bbc) SHA1(f38a431fc0fb7102c51f2d5b6f716dd4669a9822) )
	ROM_LOAD16_BYTE( "cusa.u8",  0xc00000, 0x80000, CRC(03c28199) SHA1(393b009acd3eceb346b8fff45ae2bdf4f53d041f) )
	ROM_LOAD16_BYTE( "cusa.u9",  0xe00000, 0x80000, CRC(24ba6371) SHA1(f60a9ff73b3645e2c8bad67e2f6debc61b5e0653) )

	ROM_REGION32_LE( 0x1000000, "user1", 0 )
	ROM_LOAD32_BYTE( "cusa-l41.u10", 0x000000, 0x80000, CRC(eb9372d1) SHA1(ab1e489b23b4540c4e0d1d9a6c9a2c9317f5c099) )
	ROM_LOAD32_BYTE( "cusa-l41.u11", 0x000001, 0x80000, CRC(76f3cd40) SHA1(52276841944ada54d56ecd2da95998aabd699465) )
	ROM_LOAD32_BYTE( "cusa-l41.u12", 0x000002, 0x80000, CRC(9021a376) SHA1(6a838d49bec4201e8ead7491e3b6d4a3a52dcb12) )
	ROM_LOAD32_BYTE( "cusa-l41.u13", 0x000003, 0x80000, CRC(1687c932) SHA1(45947c0c22bd4e6640f792d0c7fd06a1f4483131) )
	ROM_LOAD32_BYTE( "cusa.u14",     0x200000, 0x80000, CRC(6a4ae622) SHA1(f488e7616371125d5aef2047b8e0fc954ca4b9b4) )
	ROM_LOAD32_BYTE( "cusa.u15",     0x200001, 0x80000, CRC(1a0ad3b7) SHA1(a5300f3c789a4d9d257fda3a280e882f17f4a99f) )
	ROM_LOAD32_BYTE( "cusa.u16",     0x200002, 0x80000, CRC(799d4dd6) SHA1(f1208967544477005924f2a553037e0ffbc668ab) )
	ROM_LOAD32_BYTE( "cusa.u17",     0x200003, 0x80000, CRC(3d68b660) SHA1(3f14e32c205a504ef39abf1e390bd8031d9d7b5b) )
	ROM_LOAD32_BYTE( "cusa.u18",     0x400000, 0x80000, CRC(9e8193fb) SHA1(ec88c2b51bb607d3181e467f8b255c13efebc73c) )
	ROM_LOAD32_BYTE( "cusa.u19",     0x400001, 0x80000, CRC(0bf60cde) SHA1(6c63b3eacaefeb405c8fdf641437786262bcb10d) )
	ROM_LOAD32_BYTE( "cusa.u20",     0x400002, 0x80000, CRC(c07f68f0) SHA1(444ccf8e49fd9c0f707ab32347984ca5628207f9) )
	ROM_LOAD32_BYTE( "cusa.u21",     0x400003, 0x80000, CRC(b0264aed) SHA1(d6a6eca4e4ecedfbc5590dbd06870761155ae8c5) )
	ROM_LOAD32_BYTE( "cusa.u22",     0x600000, 0x80000, CRC(ad137193) SHA1(642a7c37940cb3b2b190661da7b1d4848c7c513d) )
	ROM_LOAD32_BYTE( "cusa.u23",     0x600001, 0x80000, CRC(842449b0) SHA1(b23ebe28ff3c6a268ff9ae1242a4392d2305396b) )
	ROM_LOAD32_BYTE( "cusa.u24",     0x600002, 0x80000, CRC(0b2275be) SHA1(3dc79095064cc158d37218c9a038b5b7a777fc66) )
	ROM_LOAD32_BYTE( "cusa.u25",     0x600003, 0x80000, CRC(2b9fe68f) SHA1(2750613e61c1eaac629ef5b9e89fd88e99a262cc) )
	ROM_LOAD32_BYTE( "cusa.u26",     0x800000, 0x80000, CRC(ae56b871) SHA1(1e218426084123c6c2389d96ce92691010012aa4) )
	ROM_LOAD32_BYTE( "cusa.u27",     0x800001, 0x80000, CRC(2d977a8e) SHA1(8f4d511bfd6c3bee18daa7253be1a27d079aec8f) )
	ROM_LOAD32_BYTE( "cusa.u28",     0x800002, 0x80000, CRC(cffa5fb1) SHA1(fb73bc8f65b604c374f88d0ecf06c50ef52f0547) )
	ROM_LOAD32_BYTE( "cusa.u29",     0x800003, 0x80000, CRC(cbe52c60) SHA1(3f309ce8ef1784c830f4160cfe76dc3a0b438cac) )

	ROM_REGION( 0x0b33, "pals", 0 )
	ROM_LOAD("a-19993.u38.bin",  0x0000, 0x02dd, CRC(b6323e94) SHA1(a84e04db8838b35ad9d30416b86aba65a29dcd87) ) /* TIBPAL22V10-15BCNT */
	ROM_LOAD("a-19670.u43.bin",  0x0000, 0x0144, CRC(acafcc97) SHA1(b6f916838d08590a536fe925ec62d66e6ea3dcbc) ) /* TIBPAL20L8-10CNT */
	ROM_LOAD("a-19668.u52.bin",  0x0000, 0x0157, CRC(7915134e) SHA1(aeb22e46abdc14a9e9b34cfe3b77da3e29b789fe) ) /* GAL20V8B */
	ROM_LOAD("a-19671.u54.bin",  0x0000, 0x02dd, CRC(b9cce038) SHA1(8d1df026bdac66ea5493e9e51c23f8eb182b024e) ) /* TIBPAL22V10-15BCNT */
	ROM_LOAD("a-19673.u111.bin", 0x0000, 0x02dd, CRC(8552977d) SHA1(a1a53d797697682b3f18893a90b6bef39ebb069e) ) /* TIBPAL22V10-15BCNT */
	ROM_LOAD("a-19672.u114.bin", 0x0000, 0x0001, NO_DUMP ) /* TIBPAL22V10-15BCNT */
ROM_END


ROM_START( crusnusa40 ) /* Version 4.0, Wed Feb 08 1995 - 10:45:14 */
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "cusa.u2",  0x000000, 0x80000, CRC(b9338332) SHA1(e5c420e63c4eba0010a68c7e0a57ef210e2c83d2) )
	ROM_LOAD16_BYTE( "cusa.u3",  0x200000, 0x80000, CRC(cd8325d6) SHA1(d65d7263e056ca1d637adb44cafef523e0831a34) )
	ROM_LOAD16_BYTE( "cusa.u4",  0x400000, 0x80000, CRC(fab457f3) SHA1(2b4b647838b7a8100afc25ca1ffdc74ed67ae00a) )
	ROM_LOAD16_BYTE( "cusa.u5",  0x600000, 0x80000, CRC(becc92f4) SHA1(6dffa73ff5270155c44f295e443d5e77c03c0338) )
	ROM_LOAD16_BYTE( "cusa.u6",  0x800000, 0x80000, CRC(a9f915d3) SHA1(6a16a2d7a807a775673e7121b54f37c583581203) )
	ROM_LOAD16_BYTE( "cusa.u7",  0xa00000, 0x80000, CRC(424f0bbc) SHA1(f38a431fc0fb7102c51f2d5b6f716dd4669a9822) )
	ROM_LOAD16_BYTE( "cusa.u8",  0xc00000, 0x80000, CRC(03c28199) SHA1(393b009acd3eceb346b8fff45ae2bdf4f53d041f) )
	ROM_LOAD16_BYTE( "cusa.u9",  0xe00000, 0x80000, CRC(24ba6371) SHA1(f60a9ff73b3645e2c8bad67e2f6debc61b5e0653) )

	ROM_REGION32_LE( 0x1000000, "user1", 0 )
	ROM_LOAD32_BYTE( "cusa-l4.u10",  0x000000, 0x80000, CRC(7526d8bf) SHA1(ef00ea3b6e1923d3e4d10bf3601b080a009fb711) )
	ROM_LOAD32_BYTE( "cusa-l4.u11",  0x000001, 0x80000, CRC(bfc691b9) SHA1(41d1503c4290e396a49043fea7778851cdf11310) )
	ROM_LOAD32_BYTE( "cusa-l4.u12",  0x000002, 0x80000, CRC(059c2234) SHA1(145ec1ab3a46c3316f39bd731730dcb57b55b4ec) )
	ROM_LOAD32_BYTE( "cusa-l4.u13",  0x000003, 0x80000, CRC(39e0ff7d) SHA1(3b0f95bf2a6999b8ec8722e0bc0f3a60264469aa) )
	ROM_LOAD32_BYTE( "cusa.u14",     0x200000, 0x80000, CRC(6a4ae622) SHA1(f488e7616371125d5aef2047b8e0fc954ca4b9b4) )
	ROM_LOAD32_BYTE( "cusa.u15",     0x200001, 0x80000, CRC(1a0ad3b7) SHA1(a5300f3c789a4d9d257fda3a280e882f17f4a99f) )
	ROM_LOAD32_BYTE( "cusa.u16",     0x200002, 0x80000, CRC(799d4dd6) SHA1(f1208967544477005924f2a553037e0ffbc668ab) )
	ROM_LOAD32_BYTE( "cusa.u17",     0x200003, 0x80000, CRC(3d68b660) SHA1(3f14e32c205a504ef39abf1e390bd8031d9d7b5b) )
	ROM_LOAD32_BYTE( "cusa.u18",     0x400000, 0x80000, CRC(9e8193fb) SHA1(ec88c2b51bb607d3181e467f8b255c13efebc73c) )
	ROM_LOAD32_BYTE( "cusa.u19",     0x400001, 0x80000, CRC(0bf60cde) SHA1(6c63b3eacaefeb405c8fdf641437786262bcb10d) )
	ROM_LOAD32_BYTE( "cusa.u20",     0x400002, 0x80000, CRC(c07f68f0) SHA1(444ccf8e49fd9c0f707ab32347984ca5628207f9) )
	ROM_LOAD32_BYTE( "cusa.u21",     0x400003, 0x80000, CRC(b0264aed) SHA1(d6a6eca4e4ecedfbc5590dbd06870761155ae8c5) )
	ROM_LOAD32_BYTE( "cusa.u22",     0x600000, 0x80000, CRC(ad137193) SHA1(642a7c37940cb3b2b190661da7b1d4848c7c513d) )
	ROM_LOAD32_BYTE( "cusa.u23",     0x600001, 0x80000, CRC(842449b0) SHA1(b23ebe28ff3c6a268ff9ae1242a4392d2305396b) )
	ROM_LOAD32_BYTE( "cusa.u24",     0x600002, 0x80000, CRC(0b2275be) SHA1(3dc79095064cc158d37218c9a038b5b7a777fc66) )
	ROM_LOAD32_BYTE( "cusa.u25",     0x600003, 0x80000, CRC(2b9fe68f) SHA1(2750613e61c1eaac629ef5b9e89fd88e99a262cc) )
	ROM_LOAD32_BYTE( "cusa.u26",     0x800000, 0x80000, CRC(ae56b871) SHA1(1e218426084123c6c2389d96ce92691010012aa4) )
	ROM_LOAD32_BYTE( "cusa.u27",     0x800001, 0x80000, CRC(2d977a8e) SHA1(8f4d511bfd6c3bee18daa7253be1a27d079aec8f) )
	ROM_LOAD32_BYTE( "cusa.u28",     0x800002, 0x80000, CRC(cffa5fb1) SHA1(fb73bc8f65b604c374f88d0ecf06c50ef52f0547) )
	ROM_LOAD32_BYTE( "cusa.u29",     0x800003, 0x80000, CRC(cbe52c60) SHA1(3f309ce8ef1784c830f4160cfe76dc3a0b438cac) )
ROM_END


ROM_START( crusnusa21 ) /* Version 2.1, Wed Nov 09 1994 - 16:28:10 */
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "cusa.u2",  0x000000, 0x80000, CRC(b9338332) SHA1(e5c420e63c4eba0010a68c7e0a57ef210e2c83d2) )
	ROM_LOAD16_BYTE( "cusa.u3",  0x200000, 0x80000, CRC(cd8325d6) SHA1(d65d7263e056ca1d637adb44cafef523e0831a34) )
	ROM_LOAD16_BYTE( "cusa.u4",  0x400000, 0x80000, CRC(fab457f3) SHA1(2b4b647838b7a8100afc25ca1ffdc74ed67ae00a) )
	ROM_LOAD16_BYTE( "cusa.u5",  0x600000, 0x80000, CRC(becc92f4) SHA1(6dffa73ff5270155c44f295e443d5e77c03c0338) )
	ROM_LOAD16_BYTE( "cusa.u6",  0x800000, 0x80000, CRC(a9f915d3) SHA1(6a16a2d7a807a775673e7121b54f37c583581203) )
	ROM_LOAD16_BYTE( "cusa.u7",  0xa00000, 0x80000, CRC(424f0bbc) SHA1(f38a431fc0fb7102c51f2d5b6f716dd4669a9822) )
	ROM_LOAD16_BYTE( "cusa.u8",  0xc00000, 0x80000, CRC(03c28199) SHA1(393b009acd3eceb346b8fff45ae2bdf4f53d041f) )
	ROM_LOAD16_BYTE( "cusa.u9",  0xe00000, 0x80000, CRC(24ba6371) SHA1(f60a9ff73b3645e2c8bad67e2f6debc61b5e0653) )

	ROM_REGION32_LE( 0x1000000, "user1", 0 )
	ROM_LOAD32_BYTE( "cusa-l21.u10", 0x000000, 0x80000, CRC(bb759945) SHA1(dbf5270503cb58adb0abd34a8aece5933063ec66) )
	ROM_LOAD32_BYTE( "cusa-l21.u11", 0x000001, 0x80000, CRC(4d2da096) SHA1(6ccb9fee095580089f8d43a2e86e0f8a4407dda5) )
	ROM_LOAD32_BYTE( "cusa-l21.u12", 0x000002, 0x80000, CRC(4b66fe5e) SHA1(885d31c06b11209a1154789bc84e75d0ac9e1e8a) )
	ROM_LOAD32_BYTE( "cusa-l21.u13", 0x000003, 0x80000, CRC(a165359f) SHA1(eefbeaa67282b3826503f4edff84282ff5f45d35) )
	ROM_LOAD32_BYTE( "cusa.u14",     0x200000, 0x80000, CRC(6a4ae622) SHA1(f488e7616371125d5aef2047b8e0fc954ca4b9b4) )
	ROM_LOAD32_BYTE( "cusa.u15",     0x200001, 0x80000, CRC(1a0ad3b7) SHA1(a5300f3c789a4d9d257fda3a280e882f17f4a99f) )
	ROM_LOAD32_BYTE( "cusa.u16",     0x200002, 0x80000, CRC(799d4dd6) SHA1(f1208967544477005924f2a553037e0ffbc668ab) )
	ROM_LOAD32_BYTE( "cusa.u17",     0x200003, 0x80000, CRC(3d68b660) SHA1(3f14e32c205a504ef39abf1e390bd8031d9d7b5b) )
	ROM_LOAD32_BYTE( "cusa.u18",     0x400000, 0x80000, CRC(9e8193fb) SHA1(ec88c2b51bb607d3181e467f8b255c13efebc73c) )
	ROM_LOAD32_BYTE( "cusa.u19",     0x400001, 0x80000, CRC(0bf60cde) SHA1(6c63b3eacaefeb405c8fdf641437786262bcb10d) )
	ROM_LOAD32_BYTE( "cusa.u20",     0x400002, 0x80000, CRC(c07f68f0) SHA1(444ccf8e49fd9c0f707ab32347984ca5628207f9) )
	ROM_LOAD32_BYTE( "cusa.u21",     0x400003, 0x80000, CRC(b0264aed) SHA1(d6a6eca4e4ecedfbc5590dbd06870761155ae8c5) )
	ROM_LOAD32_BYTE( "cusa.u22",     0x600000, 0x80000, CRC(ad137193) SHA1(642a7c37940cb3b2b190661da7b1d4848c7c513d) )
	ROM_LOAD32_BYTE( "cusa.u23",     0x600001, 0x80000, CRC(842449b0) SHA1(b23ebe28ff3c6a268ff9ae1242a4392d2305396b) )
	ROM_LOAD32_BYTE( "cusa.u24",     0x600002, 0x80000, CRC(0b2275be) SHA1(3dc79095064cc158d37218c9a038b5b7a777fc66) )
	ROM_LOAD32_BYTE( "cusa.u25",     0x600003, 0x80000, CRC(2b9fe68f) SHA1(2750613e61c1eaac629ef5b9e89fd88e99a262cc) )
	ROM_LOAD32_BYTE( "cusa.u26",     0x800000, 0x80000, CRC(ae56b871) SHA1(1e218426084123c6c2389d96ce92691010012aa4) )
	ROM_LOAD32_BYTE( "cusa.u27",     0x800001, 0x80000, CRC(2d977a8e) SHA1(8f4d511bfd6c3bee18daa7253be1a27d079aec8f) )
	ROM_LOAD32_BYTE( "cusa.u28",     0x800002, 0x80000, CRC(cffa5fb1) SHA1(fb73bc8f65b604c374f88d0ecf06c50ef52f0547) )
	ROM_LOAD32_BYTE( "cusa.u29",     0x800003, 0x80000, CRC(cbe52c60) SHA1(3f309ce8ef1784c830f4160cfe76dc3a0b438cac) )
ROM_END


ROM_START( crusnwld ) /* Version 2.5, Wed Nov 04 1998 - 15:50:52 */
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "cwld.u2",  0x000000, 0x80000, CRC(7a233c89) SHA1(ecfad4bc48a69cd3399e3b3266c81574082e0169) )
	ROM_LOAD16_BYTE( "cwld.u3",  0x200000, 0x80000, CRC(be9a5ff0) SHA1(98d69dbfa6aa8462cdd46772e991ee418b79c653) )
	ROM_LOAD16_BYTE( "cwld.u4",  0x400000, 0x80000, CRC(69f02d84) SHA1(0fb4ff750de78505f241ae6cd18fccf3ddf4223f) )
	ROM_LOAD16_BYTE( "cwld.u5",  0x600000, 0x80000, CRC(9d0b9071) SHA1(05edf9073399a942a9d0b969274a7ebf4ca677da) )
	ROM_LOAD16_BYTE( "cwld.u6",  0x800000, 0x80000, CRC(df28f492) SHA1(c61f3870f59458b7bb5efbf93d697e3fa44a7830) )
	ROM_LOAD16_BYTE( "cwld.u7",  0xa00000, 0x80000, CRC(0128913e) SHA1(c11bc115877310c17f9b57f72b29d19b0ad71afa) )
	ROM_LOAD16_BYTE( "cwld.u8",  0xc00000, 0x80000, CRC(5127c08e) SHA1(4f0eae73817270fa156829100b66f0ff88fa422c) )
	ROM_LOAD16_BYTE( "cwld.u9",  0xe00000, 0x80000, CRC(84cdc781) SHA1(62287aa72903698d1890908adde53c39f8bd200c) )

	ROM_REGION32_LE( 0x1000000, "user1", 0 )
	ROM_LOAD32_BYTE( "crusnw25.u10", 0x0000000, 0x100000, CRC(fd776872) SHA1(90df230b58b1c60d1ca7545ef177e5df30b4ea9d) ) /* Labeled 2.5 Cruis'n World Automatic U10 */
	ROM_LOAD32_BYTE( "crusnw25.u11", 0x0000001, 0x100000, CRC(0c99a405) SHA1(14251187f00c198fbaa39817f8c95d1dbec80ec0) ) /* Labeled 2.5 Cruis'n World Automatic U11 */
	ROM_LOAD32_BYTE( "crusnw25.u12", 0x0000002, 0x100000, CRC(3ba9fad8) SHA1(ac8d0dd4df3c1f1c28d93d615d7e24aed4a4a9b5) ) /* Labeled 2.5 Cruis'n World Automatic U12 */
	ROM_LOAD32_BYTE( "crusnw25.u13", 0x0000003, 0x100000, CRC(21a79c9a) SHA1(e33f768c2309613e4936416ee5250d3b1690d11d) ) /* Labeled 2.5 Cruis'n World Automatic U13 */
	ROM_LOAD32_BYTE( "cwld.u14",     0x0400000, 0x100000, CRC(ee815091) SHA1(fb8a99bae07f42966f76a3bb073d7d8280d8efcb) )
	ROM_LOAD32_BYTE( "cwld.u15",     0x0400001, 0x100000, CRC(e2da7bf1) SHA1(9d9a80055ee62476f47c95e30ec9a989d5d0e25b) )
	ROM_LOAD32_BYTE( "cwld.u16",     0x0400002, 0x100000, CRC(05a7ad2f) SHA1(4bdfde671379ecefa3f8ceb6fc06e8df5d70fc22) )
	ROM_LOAD32_BYTE( "cwld.u17",     0x0400003, 0x100000, CRC(d6278c0c) SHA1(3e152d755d69903718a84d4154e442a31026f3d8) )
	ROM_LOAD32_BYTE( "cwld.u18",     0x0800000, 0x100000, CRC(e2dc2733) SHA1(c277643548c03d831a3b091f1a311accac9d106b) )
	ROM_LOAD32_BYTE( "cwld.u19",     0x0800001, 0x100000, CRC(5223a070) SHA1(90ce48b2308fa9e7cb636c4732b20b8e177aa9b1) )
	ROM_LOAD32_BYTE( "cwld.u20",     0x0800002, 0x100000, CRC(db535625) SHA1(599ccd6bcfb155eb68ac131de4af524510ab35b7) )
	ROM_LOAD32_BYTE( "cwld.u21",     0x0800003, 0x100000, CRC(92a080e8) SHA1(e5e0faf820b5870a81f121b6ad4c37a9081724e4) )
	ROM_LOAD32_BYTE( "cwld.u22",     0x0c00000, 0x100000, CRC(77c56318) SHA1(52344038942c83f3ce82f3169a345ceb86e43dcb) )
	ROM_LOAD32_BYTE( "cwld.u23",     0x0c00001, 0x100000, CRC(6b920fc7) SHA1(993da81181f24075e1aead7c4b374f36dd86a9c3) )
	ROM_LOAD32_BYTE( "cwld.u24",     0x0c00002, 0x100000, CRC(83485401) SHA1(58407818a82a7a3657530dcda7e373e678b58ab2) )
	ROM_LOAD32_BYTE( "cwld.u25",     0x0c00003, 0x100000, CRC(0dad97a9) SHA1(cdb0c02da35243b118e37ff1519aa6ee1a79d06d) )
ROM_END


ROM_START( crusnwld24 ) /* Version 2.4, Thu Feb 19 1998 - 13:43:26 */
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "cwld.u2",  0x000000, 0x80000, CRC(7a233c89) SHA1(ecfad4bc48a69cd3399e3b3266c81574082e0169) )
	ROM_LOAD16_BYTE( "cwld.u3",  0x200000, 0x80000, CRC(be9a5ff0) SHA1(98d69dbfa6aa8462cdd46772e991ee418b79c653) )
	ROM_LOAD16_BYTE( "cwld.u4",  0x400000, 0x80000, CRC(69f02d84) SHA1(0fb4ff750de78505f241ae6cd18fccf3ddf4223f) )
	ROM_LOAD16_BYTE( "cwld.u5",  0x600000, 0x80000, CRC(9d0b9071) SHA1(05edf9073399a942a9d0b969274a7ebf4ca677da) )
	ROM_LOAD16_BYTE( "cwld.u6",  0x800000, 0x80000, CRC(df28f492) SHA1(c61f3870f59458b7bb5efbf93d697e3fa44a7830) )
	ROM_LOAD16_BYTE( "cwld.u7",  0xa00000, 0x80000, CRC(0128913e) SHA1(c11bc115877310c17f9b57f72b29d19b0ad71afa) )
	ROM_LOAD16_BYTE( "cwld.u8",  0xc00000, 0x80000, CRC(5127c08e) SHA1(4f0eae73817270fa156829100b66f0ff88fa422c) )
	ROM_LOAD16_BYTE( "cwld.u9",  0xe00000, 0x80000, CRC(84cdc781) SHA1(62287aa72903698d1890908adde53c39f8bd200c) )

	ROM_REGION32_LE( 0x1000000, "user1", 0 )
	ROM_LOAD32_BYTE( "crusnw24.u10", 0x0000000, 0x100000, CRC(551ec903) SHA1(f3d983ca5d9a90b2898fb2c3adf8859ab7b43917) )
	ROM_LOAD32_BYTE( "crusnw24.u11", 0x0000001, 0x100000, CRC(4c57faf2) SHA1(d5717e6222bb59c5aba782bce04aa52c1d148c49) )
	ROM_LOAD32_BYTE( "crusnw24.u12", 0x0000002, 0x100000, CRC(3a4d9a30) SHA1(ac944555340502e9324df8360c3efc538315e474) )
	ROM_LOAD32_BYTE( "crusnw24.u13", 0x0000003, 0x100000, CRC(ca6a0c94) SHA1(217659d2ce1b970265df258e330148fef327c6f1) )
	ROM_LOAD32_BYTE( "cwld.u14",     0x0400000, 0x100000, CRC(ee815091) SHA1(fb8a99bae07f42966f76a3bb073d7d8280d8efcb) )
	ROM_LOAD32_BYTE( "cwld.u15",     0x0400001, 0x100000, CRC(e2da7bf1) SHA1(9d9a80055ee62476f47c95e30ec9a989d5d0e25b) )
	ROM_LOAD32_BYTE( "cwld.u16",     0x0400002, 0x100000, CRC(05a7ad2f) SHA1(4bdfde671379ecefa3f8ceb6fc06e8df5d70fc22) )
	ROM_LOAD32_BYTE( "cwld.u17",     0x0400003, 0x100000, CRC(d6278c0c) SHA1(3e152d755d69903718a84d4154e442a31026f3d8) )
	ROM_LOAD32_BYTE( "cwld.u18",     0x0800000, 0x100000, CRC(e2dc2733) SHA1(c277643548c03d831a3b091f1a311accac9d106b) )
	ROM_LOAD32_BYTE( "cwld.u19",     0x0800001, 0x100000, CRC(5223a070) SHA1(90ce48b2308fa9e7cb636c4732b20b8e177aa9b1) )
	ROM_LOAD32_BYTE( "cwld.u20",     0x0800002, 0x100000, CRC(db535625) SHA1(599ccd6bcfb155eb68ac131de4af524510ab35b7) )
	ROM_LOAD32_BYTE( "cwld.u21",     0x0800003, 0x100000, CRC(92a080e8) SHA1(e5e0faf820b5870a81f121b6ad4c37a9081724e4) )
	ROM_LOAD32_BYTE( "cwld.u22",     0x0c00000, 0x100000, CRC(77c56318) SHA1(52344038942c83f3ce82f3169a345ceb86e43dcb) )
	ROM_LOAD32_BYTE( "cwld.u23",     0x0c00001, 0x100000, CRC(6b920fc7) SHA1(993da81181f24075e1aead7c4b374f36dd86a9c3) )
	ROM_LOAD32_BYTE( "cwld.u24",     0x0c00002, 0x100000, CRC(83485401) SHA1(58407818a82a7a3657530dcda7e373e678b58ab2) )
	ROM_LOAD32_BYTE( "cwld.u25",     0x0c00003, 0x100000, CRC(0dad97a9) SHA1(cdb0c02da35243b118e37ff1519aa6ee1a79d06d) )
ROM_END


ROM_START( crusnwld23 ) /* Version 2.3, Fri Jan 09 1998 - 10:25:49 */
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "cwld.u2",  0x000000, 0x80000, CRC(7a233c89) SHA1(ecfad4bc48a69cd3399e3b3266c81574082e0169) )
	ROM_LOAD16_BYTE( "cwld.u3",  0x200000, 0x80000, CRC(be9a5ff0) SHA1(98d69dbfa6aa8462cdd46772e991ee418b79c653) )
	ROM_LOAD16_BYTE( "cwld.u4",  0x400000, 0x80000, CRC(69f02d84) SHA1(0fb4ff750de78505f241ae6cd18fccf3ddf4223f) )
	ROM_LOAD16_BYTE( "cwld.u5",  0x600000, 0x80000, CRC(9d0b9071) SHA1(05edf9073399a942a9d0b969274a7ebf4ca677da) )
	ROM_LOAD16_BYTE( "cwld.u6",  0x800000, 0x80000, CRC(df28f492) SHA1(c61f3870f59458b7bb5efbf93d697e3fa44a7830) )
	ROM_LOAD16_BYTE( "cwld.u7",  0xa00000, 0x80000, CRC(0128913e) SHA1(c11bc115877310c17f9b57f72b29d19b0ad71afa) )
	ROM_LOAD16_BYTE( "cwld.u8",  0xc00000, 0x80000, CRC(5127c08e) SHA1(4f0eae73817270fa156829100b66f0ff88fa422c) )
	ROM_LOAD16_BYTE( "cwld.u9",  0xe00000, 0x80000, CRC(84cdc781) SHA1(62287aa72903698d1890908adde53c39f8bd200c) )

	ROM_REGION32_LE( 0x1000000, "user1", 0 )
	ROM_LOAD32_BYTE( "cwld_l23.u10", 0x0000000, 0x100000, CRC(956e0642) SHA1(c023d41159bac9b468d6fc411005f66b15b9dff6) )
	ROM_LOAD32_BYTE( "cwld_l23.u11", 0x0000001, 0x100000, CRC(b4ed2929) SHA1(22afc3c7bcc57b7b24b4156376df0b7fb8f0c9fb) )
	ROM_LOAD32_BYTE( "cwld_l23.u12", 0x0000002, 0x100000, CRC(cd12528e) SHA1(685e2280448be2cd90a875cca9ef2ab3d2f8d3e1) )
	ROM_LOAD32_BYTE( "cwld_l23.u13", 0x0000003, 0x100000, CRC(b096d211) SHA1(a2663b58e2f21bcfcba5317ff0ae91dd21a399f5) )
	ROM_LOAD32_BYTE( "cwld.u14",     0x0400000, 0x100000, CRC(ee815091) SHA1(fb8a99bae07f42966f76a3bb073d7d8280d8efcb) )
	ROM_LOAD32_BYTE( "cwld.u15",     0x0400001, 0x100000, CRC(e2da7bf1) SHA1(9d9a80055ee62476f47c95e30ec9a989d5d0e25b) )
	ROM_LOAD32_BYTE( "cwld.u16",     0x0400002, 0x100000, CRC(05a7ad2f) SHA1(4bdfde671379ecefa3f8ceb6fc06e8df5d70fc22) )
	ROM_LOAD32_BYTE( "cwld.u17",     0x0400003, 0x100000, CRC(d6278c0c) SHA1(3e152d755d69903718a84d4154e442a31026f3d8) )
	ROM_LOAD32_BYTE( "cwld.u18",     0x0800000, 0x100000, CRC(e2dc2733) SHA1(c277643548c03d831a3b091f1a311accac9d106b) )
	ROM_LOAD32_BYTE( "cwld.u19",     0x0800001, 0x100000, CRC(5223a070) SHA1(90ce48b2308fa9e7cb636c4732b20b8e177aa9b1) )
	ROM_LOAD32_BYTE( "cwld.u20",     0x0800002, 0x100000, CRC(db535625) SHA1(599ccd6bcfb155eb68ac131de4af524510ab35b7) )
	ROM_LOAD32_BYTE( "cwld.u21",     0x0800003, 0x100000, CRC(92a080e8) SHA1(e5e0faf820b5870a81f121b6ad4c37a9081724e4) )
	ROM_LOAD32_BYTE( "cwld.u22",     0x0c00000, 0x100000, CRC(77c56318) SHA1(52344038942c83f3ce82f3169a345ceb86e43dcb) )
	ROM_LOAD32_BYTE( "cwld.u23",     0x0c00001, 0x100000, CRC(6b920fc7) SHA1(993da81181f24075e1aead7c4b374f36dd86a9c3) )
	ROM_LOAD32_BYTE( "cwld.u24",     0x0c00002, 0x100000, CRC(83485401) SHA1(58407818a82a7a3657530dcda7e373e678b58ab2) )
	ROM_LOAD32_BYTE( "cwld.u25",     0x0c00003, 0x100000, CRC(0dad97a9) SHA1(cdb0c02da35243b118e37ff1519aa6ee1a79d06d) )
ROM_END


ROM_START( crusnwld20 ) /* Version 2.0, Tue Mar 18 1997 - 12:32:57 */
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "cwld.u2",  0x000000, 0x80000, CRC(7a233c89) SHA1(ecfad4bc48a69cd3399e3b3266c81574082e0169) )
	ROM_LOAD16_BYTE( "cwld.u3",  0x200000, 0x80000, CRC(be9a5ff0) SHA1(98d69dbfa6aa8462cdd46772e991ee418b79c653) )
	ROM_LOAD16_BYTE( "cwld.u4",  0x400000, 0x80000, CRC(69f02d84) SHA1(0fb4ff750de78505f241ae6cd18fccf3ddf4223f) )
	ROM_LOAD16_BYTE( "cwld.u5",  0x600000, 0x80000, CRC(9d0b9071) SHA1(05edf9073399a942a9d0b969274a7ebf4ca677da) )
	ROM_LOAD16_BYTE( "cwld.u6",  0x800000, 0x80000, CRC(df28f492) SHA1(c61f3870f59458b7bb5efbf93d697e3fa44a7830) )
	ROM_LOAD16_BYTE( "cwld.u7",  0xa00000, 0x80000, CRC(0128913e) SHA1(c11bc115877310c17f9b57f72b29d19b0ad71afa) )
	ROM_LOAD16_BYTE( "cwld.u8",  0xc00000, 0x80000, CRC(5127c08e) SHA1(4f0eae73817270fa156829100b66f0ff88fa422c) )
	ROM_LOAD16_BYTE( "cwld.u9",  0xe00000, 0x80000, CRC(84cdc781) SHA1(62287aa72903698d1890908adde53c39f8bd200c) )

	ROM_REGION32_LE( 0x1000000, "user1", 0 )
	ROM_LOAD32_BYTE( "u10_v20.u10",  0x0000000, 0x100000, CRC(2a04da6d) SHA1(0aab4f3dc4853de11234245ac14baa14cb3867f3) )
	ROM_LOAD32_BYTE( "u11_v20.u11",  0x0000001, 0x100000, CRC(26a8ad51) SHA1(522ef3499ba83fa808d7cdae71759e056df353bf) )
	ROM_LOAD32_BYTE( "u12_v20.u12",  0x0000002, 0x100000, CRC(236caec0) SHA1(f53df733943a52f94878bb1b7d6c877722b3fd82) )
	ROM_LOAD32_BYTE( "u13_v20.u13",  0x0000003, 0x100000, CRC(7e056e53) SHA1(62b593e093d06a8c0cca56e34f567f795bfc41fc) )
	ROM_LOAD32_BYTE( "cwld.u14",     0x0400000, 0x100000, CRC(ee815091) SHA1(fb8a99bae07f42966f76a3bb073d7d8280d8efcb) )
	ROM_LOAD32_BYTE( "cwld.u15",     0x0400001, 0x100000, CRC(e2da7bf1) SHA1(9d9a80055ee62476f47c95e30ec9a989d5d0e25b) )
	ROM_LOAD32_BYTE( "cwld.u16",     0x0400002, 0x100000, CRC(05a7ad2f) SHA1(4bdfde671379ecefa3f8ceb6fc06e8df5d70fc22) )
	ROM_LOAD32_BYTE( "cwld.u17",     0x0400003, 0x100000, CRC(d6278c0c) SHA1(3e152d755d69903718a84d4154e442a31026f3d8) )
	ROM_LOAD32_BYTE( "cwld.u18",     0x0800000, 0x100000, CRC(e2dc2733) SHA1(c277643548c03d831a3b091f1a311accac9d106b) )
	ROM_LOAD32_BYTE( "cwld.u19",     0x0800001, 0x100000, CRC(5223a070) SHA1(90ce48b2308fa9e7cb636c4732b20b8e177aa9b1) )
	ROM_LOAD32_BYTE( "cwld.u20",     0x0800002, 0x100000, CRC(db535625) SHA1(599ccd6bcfb155eb68ac131de4af524510ab35b7) )
	ROM_LOAD32_BYTE( "cwld.u21",     0x0800003, 0x100000, CRC(92a080e8) SHA1(e5e0faf820b5870a81f121b6ad4c37a9081724e4) )
	ROM_LOAD32_BYTE( "cwld.u22",     0x0c00000, 0x100000, CRC(77c56318) SHA1(52344038942c83f3ce82f3169a345ceb86e43dcb) )
	ROM_LOAD32_BYTE( "cwld.u23",     0x0c00001, 0x100000, CRC(6b920fc7) SHA1(993da81181f24075e1aead7c4b374f36dd86a9c3) )
	ROM_LOAD32_BYTE( "cwld.u24",     0x0c00002, 0x100000, CRC(83485401) SHA1(58407818a82a7a3657530dcda7e373e678b58ab2) )
	ROM_LOAD32_BYTE( "cwld.u25",     0x0c00003, 0x100000, CRC(0dad97a9) SHA1(cdb0c02da35243b118e37ff1519aa6ee1a79d06d) )
ROM_END


ROM_START( crusnwld19 ) /* Version 1.9, Sat Mar 08 1997 - 14:48:17 */
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "cwld.u2",  0x000000, 0x80000, CRC(7a233c89) SHA1(ecfad4bc48a69cd3399e3b3266c81574082e0169) )
	ROM_LOAD16_BYTE( "cwld.u3",  0x200000, 0x80000, CRC(be9a5ff0) SHA1(98d69dbfa6aa8462cdd46772e991ee418b79c653) )
	ROM_LOAD16_BYTE( "cwld.u4",  0x400000, 0x80000, CRC(69f02d84) SHA1(0fb4ff750de78505f241ae6cd18fccf3ddf4223f) )
	ROM_LOAD16_BYTE( "cwld.u5",  0x600000, 0x80000, CRC(9d0b9071) SHA1(05edf9073399a942a9d0b969274a7ebf4ca677da) )
	ROM_LOAD16_BYTE( "cwld.u6",  0x800000, 0x80000, CRC(df28f492) SHA1(c61f3870f59458b7bb5efbf93d697e3fa44a7830) )
	ROM_LOAD16_BYTE( "cwld.u7",  0xa00000, 0x80000, CRC(0128913e) SHA1(c11bc115877310c17f9b57f72b29d19b0ad71afa) )
	ROM_LOAD16_BYTE( "cwld.u8",  0xc00000, 0x80000, CRC(5127c08e) SHA1(4f0eae73817270fa156829100b66f0ff88fa422c) )
	ROM_LOAD16_BYTE( "cwld.u9",  0xe00000, 0x80000, CRC(84cdc781) SHA1(62287aa72903698d1890908adde53c39f8bd200c) )

	ROM_REGION32_LE( 0x1000000, "user1", 0 )
	ROM_LOAD32_BYTE( "crusnw19.u10", 0x0000000, 0x100000, CRC(c5cf5316) SHA1(f9900526314c1ea2903fd9a01fcdb839609b1858) )
	ROM_LOAD32_BYTE( "crusnw19.u11", 0x0000001, 0x100000, CRC(0b183a06) SHA1(06585662abebedc986b22c10fd4f489ed1d94e67) )
	ROM_LOAD32_BYTE( "crusnw19.u12", 0x0000002, 0x100000, CRC(e32d1a8d) SHA1(ba5bbcee4fe67194e5e0bd99898116350450e83f) )
	ROM_LOAD32_BYTE( "crusnw19.u13", 0x0000003, 0x100000, CRC(91fcae27) SHA1(a5c2942b01ed6c8a8f4187b4d08a5f1868cf3e2e) )
	ROM_LOAD32_BYTE( "cwld.u14",     0x0400000, 0x100000, CRC(ee815091) SHA1(fb8a99bae07f42966f76a3bb073d7d8280d8efcb) )
	ROM_LOAD32_BYTE( "cwld.u15",     0x0400001, 0x100000, CRC(e2da7bf1) SHA1(9d9a80055ee62476f47c95e30ec9a989d5d0e25b) )
	ROM_LOAD32_BYTE( "cwld.u16",     0x0400002, 0x100000, CRC(05a7ad2f) SHA1(4bdfde671379ecefa3f8ceb6fc06e8df5d70fc22) )
	ROM_LOAD32_BYTE( "cwld.u17",     0x0400003, 0x100000, CRC(d6278c0c) SHA1(3e152d755d69903718a84d4154e442a31026f3d8) )
	ROM_LOAD32_BYTE( "cwld.u18",     0x0800000, 0x100000, CRC(e2dc2733) SHA1(c277643548c03d831a3b091f1a311accac9d106b) )
	ROM_LOAD32_BYTE( "cwld.u19",     0x0800001, 0x100000, CRC(5223a070) SHA1(90ce48b2308fa9e7cb636c4732b20b8e177aa9b1) )
	ROM_LOAD32_BYTE( "cwld.u20",     0x0800002, 0x100000, CRC(db535625) SHA1(599ccd6bcfb155eb68ac131de4af524510ab35b7) )
	ROM_LOAD32_BYTE( "cwld.u21",     0x0800003, 0x100000, CRC(92a080e8) SHA1(e5e0faf820b5870a81f121b6ad4c37a9081724e4) )
	ROM_LOAD32_BYTE( "cwld.u22",     0x0c00000, 0x100000, CRC(77c56318) SHA1(52344038942c83f3ce82f3169a345ceb86e43dcb) )
	ROM_LOAD32_BYTE( "cwld.u23",     0x0c00001, 0x100000, CRC(6b920fc7) SHA1(993da81181f24075e1aead7c4b374f36dd86a9c3) )
	ROM_LOAD32_BYTE( "cwld.u24",     0x0c00002, 0x100000, CRC(83485401) SHA1(58407818a82a7a3657530dcda7e373e678b58ab2) )
	ROM_LOAD32_BYTE( "cwld.u25",     0x0c00003, 0x100000, CRC(0dad97a9) SHA1(cdb0c02da35243b118e37ff1519aa6ee1a79d06d) )
ROM_END


ROM_START( crusnwld17 ) /* Version 1.7, Fri Jan 24 1997 - 16:23:59 */
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "cwld.u2",  0x000000, 0x80000, CRC(7a233c89) SHA1(ecfad4bc48a69cd3399e3b3266c81574082e0169) )
	ROM_LOAD16_BYTE( "cwld.u3",  0x200000, 0x80000, CRC(be9a5ff0) SHA1(98d69dbfa6aa8462cdd46772e991ee418b79c653) )
	ROM_LOAD16_BYTE( "cwld.u4",  0x400000, 0x80000, CRC(69f02d84) SHA1(0fb4ff750de78505f241ae6cd18fccf3ddf4223f) )
	ROM_LOAD16_BYTE( "cwld.u5",  0x600000, 0x80000, CRC(9d0b9071) SHA1(05edf9073399a942a9d0b969274a7ebf4ca677da) )
	ROM_LOAD16_BYTE( "cwld.u6",  0x800000, 0x80000, CRC(df28f492) SHA1(c61f3870f59458b7bb5efbf93d697e3fa44a7830) )
	ROM_LOAD16_BYTE( "cwld.u7",  0xa00000, 0x80000, CRC(0128913e) SHA1(c11bc115877310c17f9b57f72b29d19b0ad71afa) )
	ROM_LOAD16_BYTE( "cwld.u8",  0xc00000, 0x80000, CRC(5127c08e) SHA1(4f0eae73817270fa156829100b66f0ff88fa422c) )
	ROM_LOAD16_BYTE( "cwld.u9",  0xe00000, 0x80000, CRC(84cdc781) SHA1(62287aa72903698d1890908adde53c39f8bd200c) )

	ROM_REGION32_LE( 0x1000000, "user1", 0 )
	ROM_LOAD32_BYTE( "crusnw17.u10", 0x0000000, 0x100000, CRC(afca0f15) SHA1(52ed51e31ba7f8ac1a71a7bdb64733b6e95b0669) )
	ROM_LOAD32_BYTE( "crusnw17.u11", 0x0000001, 0x100000, CRC(6610af52) SHA1(c6ab7f369bd0b05e0ce28c7829b870f5b6ddf12f) )
	ROM_LOAD32_BYTE( "crusnw17.u12", 0x0000002, 0x100000, CRC(ef0107b1) SHA1(350017ab56c220516dda53c8323eaf82d7dee8dd) )
	ROM_LOAD32_BYTE( "crusnw17.u13", 0x0000003, 0x100000, CRC(c1d68aa0) SHA1(07d5ec75d921935474a9de738b1b7e9cb0748483) )
	ROM_LOAD32_BYTE( "cwld.u14",     0x0400000, 0x100000, CRC(ee815091) SHA1(fb8a99bae07f42966f76a3bb073d7d8280d8efcb) )
	ROM_LOAD32_BYTE( "cwld.u15",     0x0400001, 0x100000, CRC(e2da7bf1) SHA1(9d9a80055ee62476f47c95e30ec9a989d5d0e25b) )
	ROM_LOAD32_BYTE( "cwld.u16",     0x0400002, 0x100000, CRC(05a7ad2f) SHA1(4bdfde671379ecefa3f8ceb6fc06e8df5d70fc22) )
	ROM_LOAD32_BYTE( "cwld.u17",     0x0400003, 0x100000, CRC(d6278c0c) SHA1(3e152d755d69903718a84d4154e442a31026f3d8) )
	ROM_LOAD32_BYTE( "cwld.u18",     0x0800000, 0x100000, CRC(e2dc2733) SHA1(c277643548c03d831a3b091f1a311accac9d106b) )
	ROM_LOAD32_BYTE( "cwld.u19",     0x0800001, 0x100000, CRC(5223a070) SHA1(90ce48b2308fa9e7cb636c4732b20b8e177aa9b1) )
	ROM_LOAD32_BYTE( "cwld.u20",     0x0800002, 0x100000, CRC(db535625) SHA1(599ccd6bcfb155eb68ac131de4af524510ab35b7) )
	ROM_LOAD32_BYTE( "cwld.u21",     0x0800003, 0x100000, CRC(92a080e8) SHA1(e5e0faf820b5870a81f121b6ad4c37a9081724e4) )
	ROM_LOAD32_BYTE( "cwld.u22",     0x0c00000, 0x100000, CRC(77c56318) SHA1(52344038942c83f3ce82f3169a345ceb86e43dcb) )
	ROM_LOAD32_BYTE( "cwld.u23",     0x0c00001, 0x100000, CRC(6b920fc7) SHA1(993da81181f24075e1aead7c4b374f36dd86a9c3) )
	ROM_LOAD32_BYTE( "cwld.u24",     0x0c00002, 0x100000, CRC(83485401) SHA1(58407818a82a7a3657530dcda7e373e678b58ab2) )
	ROM_LOAD32_BYTE( "cwld.u25",     0x0c00003, 0x100000, CRC(0dad97a9) SHA1(cdb0c02da35243b118e37ff1519aa6ee1a79d06d) )
ROM_END


ROM_START( crusnwld13 ) /* Version 1.3, Mon Nov 25 1996 - 23:22:45 */
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "cwld.u2",  0x000000, 0x80000, CRC(7a233c89) SHA1(ecfad4bc48a69cd3399e3b3266c81574082e0169) )
	ROM_LOAD16_BYTE( "cwld.u3",  0x200000, 0x80000, CRC(be9a5ff0) SHA1(98d69dbfa6aa8462cdd46772e991ee418b79c653) )
	ROM_LOAD16_BYTE( "cwld.u4",  0x400000, 0x80000, CRC(69f02d84) SHA1(0fb4ff750de78505f241ae6cd18fccf3ddf4223f) )
	ROM_LOAD16_BYTE( "cwld.u5",  0x600000, 0x80000, CRC(9d0b9071) SHA1(05edf9073399a942a9d0b969274a7ebf4ca677da) )
	ROM_LOAD16_BYTE( "cwld.u6",  0x800000, 0x80000, CRC(df28f492) SHA1(c61f3870f59458b7bb5efbf93d697e3fa44a7830) )
	ROM_LOAD16_BYTE( "cwld.u7",  0xa00000, 0x80000, CRC(0128913e) SHA1(c11bc115877310c17f9b57f72b29d19b0ad71afa) )
	ROM_LOAD16_BYTE( "cwld.u8",  0xc00000, 0x80000, CRC(5127c08e) SHA1(4f0eae73817270fa156829100b66f0ff88fa422c) )
	ROM_LOAD16_BYTE( "cwld.u9",  0xe00000, 0x80000, CRC(84cdc781) SHA1(62287aa72903698d1890908adde53c39f8bd200c) )

	ROM_REGION32_LE( 0x1000000, "user1", 0 )
	ROM_LOAD32_BYTE( "cwld_l13.u10", 0x0000000, 0x100000, CRC(d361d17d) SHA1(7f42baec5492c4040e030e6233e500eb54bd9cba) )
	ROM_LOAD32_BYTE( "cwld_l13.u11", 0x0000001, 0x100000, CRC(b0c0a462) SHA1(22ae081c3eb9f298aea73e99a0124becd540f0df) )
	ROM_LOAD32_BYTE( "cwld_l13.u12", 0x0000002, 0x100000, CRC(5e7c566b) SHA1(81e6f21309bd3ba8589bc591a9ba5729f301539e) )
	ROM_LOAD32_BYTE( "cwld_l13.u13", 0x0000003, 0x100000, CRC(46886e9c) SHA1(a2400f42fef9838fd8a347e8a249ba977d9fbcfe) )
	ROM_LOAD32_BYTE( "cwld.u14",     0x0400000, 0x100000, CRC(ee815091) SHA1(fb8a99bae07f42966f76a3bb073d7d8280d8efcb) )
	ROM_LOAD32_BYTE( "cwld.u15",     0x0400001, 0x100000, CRC(e2da7bf1) SHA1(9d9a80055ee62476f47c95e30ec9a989d5d0e25b) )
	ROM_LOAD32_BYTE( "cwld.u16",     0x0400002, 0x100000, CRC(05a7ad2f) SHA1(4bdfde671379ecefa3f8ceb6fc06e8df5d70fc22) )
	ROM_LOAD32_BYTE( "cwld.u17",     0x0400003, 0x100000, CRC(d6278c0c) SHA1(3e152d755d69903718a84d4154e442a31026f3d8) )
	ROM_LOAD32_BYTE( "cwld.u18",     0x0800000, 0x100000, CRC(e2dc2733) SHA1(c277643548c03d831a3b091f1a311accac9d106b) )
	ROM_LOAD32_BYTE( "cwld.u19",     0x0800001, 0x100000, CRC(5223a070) SHA1(90ce48b2308fa9e7cb636c4732b20b8e177aa9b1) )
	ROM_LOAD32_BYTE( "cwld.u20",     0x0800002, 0x100000, CRC(db535625) SHA1(599ccd6bcfb155eb68ac131de4af524510ab35b7) )
	ROM_LOAD32_BYTE( "cwld.u21",     0x0800003, 0x100000, CRC(92a080e8) SHA1(e5e0faf820b5870a81f121b6ad4c37a9081724e4) )
	ROM_LOAD32_BYTE( "cwld.u22",     0x0c00000, 0x100000, CRC(77c56318) SHA1(52344038942c83f3ce82f3169a345ceb86e43dcb) )
	ROM_LOAD32_BYTE( "cwld.u23",     0x0c00001, 0x100000, CRC(6b920fc7) SHA1(993da81181f24075e1aead7c4b374f36dd86a9c3) )
	ROM_LOAD32_BYTE( "cwld.u24",     0x0c00002, 0x100000, CRC(83485401) SHA1(58407818a82a7a3657530dcda7e373e678b58ab2) )
	ROM_LOAD32_BYTE( "cwld.u25",     0x0c00003, 0x100000, CRC(0dad97a9) SHA1(cdb0c02da35243b118e37ff1519aa6ee1a79d06d) )
ROM_END


ROM_START( offroadc ) /* Version 1.63, Tue 03-03-98 */
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "offroadc.u2",  0x000000, 0x80000, CRC(69976e9d) SHA1(63c886ac2563c43a10840f49f929f8613cd94de2) )
	ROM_LOAD16_BYTE( "offroadc.u3",  0x200000, 0x80000, CRC(2db9b548) SHA1(4f454a3e6a8851b0ef5d325dd28102d57ea11a11) )
	ROM_LOAD16_BYTE( "offroadc.u4",  0x400000, 0x80000, CRC(42bdf9d0) SHA1(04add0f0ee7fa61de1913cc0b988345d3d430cde) )
	ROM_LOAD16_BYTE( "offroadc.u5",  0x600000, 0x80000, CRC(569cc84b) SHA1(08b917cc41fae6b6a3e9d9461a783d3d2865e72a) )
	ROM_LOAD16_BYTE( "offroadc.u6",  0x800000, 0x80000, CRC(0896f679) SHA1(dde39ef17834256909ef2c9fcd5b5fb9939d5178) )
	ROM_LOAD16_BYTE( "offroadc.u7",  0xa00000, 0x80000, CRC(fe242d6a) SHA1(8fbac22ed23044841f309ce58c5b1affcdd5d114) )
	ROM_LOAD16_BYTE( "offroadc.u8",  0xc00000, 0x80000, CRC(5da13f12) SHA1(2bb5e929e8bc6c70cb4475024a6b0bb07ac25244) )
	ROM_LOAD16_BYTE( "offroadc.u9",  0xe00000, 0x80000, CRC(7ad27f69) SHA1(b33665d0593a95b58d529720aae49e90449bf714) )

	ROM_REGION32_LE( 0x1000000, "user1", 0 )
	ROM_LOAD32_BYTE( "orc-1_63.u10", 0x0000000, 0x100000, CRC(faaf81b8) SHA1(d0bd40b2cf5d07db9f668826cc7f0ed84c4e84bf) ) /* Version 1.63 program roms */
	ROM_LOAD32_BYTE( "orc-1_63.u11", 0x0000001, 0x100000, CRC(f68e9655) SHA1(e29926ea24cfbd228a2136d04a63a92eba0098d7) )
	ROM_LOAD32_BYTE( "orc-1_63.u12", 0x0000002, 0x100000, CRC(6a5295b3) SHA1(ac72fe205ffb306598400e8b1d9c98ae67b0bab9) )
	ROM_LOAD32_BYTE( "orc-1_63.u13", 0x0000003, 0x100000, CRC(cb9233b5) SHA1(2d23b6a2312a75dbaa44de3224512c844aaac7b5) )
	ROM_LOAD32_BYTE( "offroadc.u14", 0x0400000, 0x100000, CRC(1e41d14b) SHA1(3f7c5fae1f8b82ddd811720837fa298785a8dd27) )
	ROM_LOAD32_BYTE( "offroadc.u15", 0x0400001, 0x100000, CRC(654d623d) SHA1(a944b8f8d71b099d7b5bbd7df6effb90afc3aec8) )
	ROM_LOAD32_BYTE( "offroadc.u16", 0x0400002, 0x100000, CRC(259774d8) SHA1(90cdf659324b84b3c2c59497cc5611e8f12629a6) )
	ROM_LOAD32_BYTE( "offroadc.u17", 0x0400003, 0x100000, CRC(50c61434) SHA1(52bc603101b4f88b7d892af683b7c8358cabbf4a) )
	ROM_LOAD32_BYTE( "offroadc.u18", 0x0800000, 0x100000, CRC(015be91c) SHA1(1624537068c6bc5fa6235bf0b0343347c337e8d8) )
	ROM_LOAD32_BYTE( "offroadc.u19", 0x0800001, 0x100000, CRC(cfc6b70e) SHA1(8c5ad84c50ca142726db0595153cf04caaabec9c) )
	ROM_LOAD32_BYTE( "offroadc.u20", 0x0800002, 0x100000, CRC(f48d6e33) SHA1(8b9c205e24f217ac110cdd82388c056ebbbb09b0) )
	ROM_LOAD32_BYTE( "offroadc.u21", 0x0800003, 0x100000, CRC(17794b56) SHA1(8bfd8f5b43056bfe7f62524bb8c3a8564a3a9413) )
	ROM_LOAD32_BYTE( "offroadc.u22", 0x0c00000, 0x100000, CRC(f2a6e622) SHA1(a7d7004e95b058124cc02e8073dab8fbed8813c5) )
	ROM_LOAD32_BYTE( "offroadc.u23", 0x0c00001, 0x100000, CRC(1cba6e20) SHA1(a7c9c58bfc4d26decb08979d83cccedb27528eb6) )
	ROM_LOAD32_BYTE( "offroadc.u24", 0x0c00002, 0x100000, CRC(fd3ce11f) SHA1(78c65267712488784bc6dc14eef98a90494a9553) )
	ROM_LOAD32_BYTE( "offroadc.u25", 0x0c00003, 0x100000, CRC(78f8e5db) SHA1(7ec2a5add27d66c43ba5cb7182554321007f5798) )
ROM_END


ROM_START( offroadc5 ) /* Version 1.50, Tue 10-21-97 */
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "offroadc.u2",  0x000000, 0x80000, CRC(69976e9d) SHA1(63c886ac2563c43a10840f49f929f8613cd94de2) )
	ROM_LOAD16_BYTE( "offroadc.u3",  0x200000, 0x80000, CRC(2db9b548) SHA1(4f454a3e6a8851b0ef5d325dd28102d57ea11a11) )
	ROM_LOAD16_BYTE( "offroadc.u4",  0x400000, 0x80000, CRC(42bdf9d0) SHA1(04add0f0ee7fa61de1913cc0b988345d3d430cde) )
	ROM_LOAD16_BYTE( "offroadc.u5",  0x600000, 0x80000, CRC(569cc84b) SHA1(08b917cc41fae6b6a3e9d9461a783d3d2865e72a) )
	ROM_LOAD16_BYTE( "offroadc.u6",  0x800000, 0x80000, CRC(0896f679) SHA1(dde39ef17834256909ef2c9fcd5b5fb9939d5178) )
	ROM_LOAD16_BYTE( "offroadc.u7",  0xa00000, 0x80000, CRC(fe242d6a) SHA1(8fbac22ed23044841f309ce58c5b1affcdd5d114) )
	ROM_LOAD16_BYTE( "offroadc.u8",  0xc00000, 0x80000, CRC(5da13f12) SHA1(2bb5e929e8bc6c70cb4475024a6b0bb07ac25244) )
	ROM_LOAD16_BYTE( "offroadc.u9",  0xe00000, 0x80000, CRC(7ad27f69) SHA1(b33665d0593a95b58d529720aae49e90449bf714) )

	ROM_REGION32_LE( 0x1000000, "user1", 0 )
	ROM_LOAD32_BYTE( "orc-1_5.u10",  0x0000000, 0x100000, CRC(f464be4f) SHA1(da6c04ae49d033f92cdd62f997841365c4a08616) ) /* Version 1.50 program roms */
	ROM_LOAD32_BYTE( "orc-1_5.u11",  0x0000001, 0x100000, CRC(eaddc9ac) SHA1(a6b810bf7460e3257bf6acdc3b79c532fb71ad68) )
	ROM_LOAD32_BYTE( "orc-1_5.u12",  0x0000002, 0x100000, CRC(a2da68da) SHA1(b8dcc042b9926055bff9020599c1c218f08b1727) )
	ROM_LOAD32_BYTE( "orc-1_5.u13",  0x0000003, 0x100000, CRC(b4755ee2) SHA1(1c4cde7ca60a6e8bff12aed348e7148e20a8caba) )
	ROM_LOAD32_BYTE( "offroadc.u14", 0x0400000, 0x100000, CRC(1e41d14b) SHA1(3f7c5fae1f8b82ddd811720837fa298785a8dd27) )
	ROM_LOAD32_BYTE( "offroadc.u15", 0x0400001, 0x100000, CRC(654d623d) SHA1(a944b8f8d71b099d7b5bbd7df6effb90afc3aec8) )
	ROM_LOAD32_BYTE( "offroadc.u16", 0x0400002, 0x100000, CRC(259774d8) SHA1(90cdf659324b84b3c2c59497cc5611e8f12629a6) )
	ROM_LOAD32_BYTE( "offroadc.u17", 0x0400003, 0x100000, CRC(50c61434) SHA1(52bc603101b4f88b7d892af683b7c8358cabbf4a) )
	ROM_LOAD32_BYTE( "offroadc.u18", 0x0800000, 0x100000, CRC(015be91c) SHA1(1624537068c6bc5fa6235bf0b0343347c337e8d8) )
	ROM_LOAD32_BYTE( "offroadc.u19", 0x0800001, 0x100000, CRC(cfc6b70e) SHA1(8c5ad84c50ca142726db0595153cf04caaabec9c) )
	ROM_LOAD32_BYTE( "offroadc.u20", 0x0800002, 0x100000, CRC(f48d6e33) SHA1(8b9c205e24f217ac110cdd82388c056ebbbb09b0) )
	ROM_LOAD32_BYTE( "offroadc.u21", 0x0800003, 0x100000, CRC(17794b56) SHA1(8bfd8f5b43056bfe7f62524bb8c3a8564a3a9413) )
	ROM_LOAD32_BYTE( "offroadc.u22", 0x0c00000, 0x100000, CRC(f2a6e622) SHA1(a7d7004e95b058124cc02e8073dab8fbed8813c5) )
	ROM_LOAD32_BYTE( "offroadc.u23", 0x0c00001, 0x100000, CRC(1cba6e20) SHA1(a7c9c58bfc4d26decb08979d83cccedb27528eb6) )
	ROM_LOAD32_BYTE( "offroadc.u24", 0x0c00002, 0x100000, CRC(fd3ce11f) SHA1(78c65267712488784bc6dc14eef98a90494a9553) )
	ROM_LOAD32_BYTE( "offroadc.u25", 0x0c00003, 0x100000, CRC(78f8e5db) SHA1(7ec2a5add27d66c43ba5cb7182554321007f5798) )
ROM_END


ROM_START( offroadc4 ) /* Version 1.40, Mon 10-06-97 */
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "offroadc.u2",  0x000000, 0x80000, CRC(69976e9d) SHA1(63c886ac2563c43a10840f49f929f8613cd94de2) )
	ROM_LOAD16_BYTE( "offroadc.u3",  0x200000, 0x80000, CRC(2db9b548) SHA1(4f454a3e6a8851b0ef5d325dd28102d57ea11a11) )
	ROM_LOAD16_BYTE( "offroadc.u4",  0x400000, 0x80000, CRC(42bdf9d0) SHA1(04add0f0ee7fa61de1913cc0b988345d3d430cde) )
	ROM_LOAD16_BYTE( "offroadc.u5",  0x600000, 0x80000, CRC(569cc84b) SHA1(08b917cc41fae6b6a3e9d9461a783d3d2865e72a) )
	ROM_LOAD16_BYTE( "offroadc.u6",  0x800000, 0x80000, CRC(0896f679) SHA1(dde39ef17834256909ef2c9fcd5b5fb9939d5178) )
	ROM_LOAD16_BYTE( "offroadc.u7",  0xa00000, 0x80000, CRC(fe242d6a) SHA1(8fbac22ed23044841f309ce58c5b1affcdd5d114) )
	ROM_LOAD16_BYTE( "offroadc.u8",  0xc00000, 0x80000, CRC(5da13f12) SHA1(2bb5e929e8bc6c70cb4475024a6b0bb07ac25244) )
	ROM_LOAD16_BYTE( "offroadc.u9",  0xe00000, 0x80000, CRC(7ad27f69) SHA1(b33665d0593a95b58d529720aae49e90449bf714) )

	ROM_REGION32_LE( 0x1000000, "user1", 0 )
	ROM_LOAD32_BYTE( "orc-1_4.u10",  0x0000000, 0x100000, CRC(d263b078) SHA1(d376e120e05cf8526b002300db345fd0b9775702) ) /* Version 1.40 program roms */
	ROM_LOAD32_BYTE( "orc-1_4.u11",  0x0000001, 0x100000, CRC(1b443a72) SHA1(0e16d923f0e97f21e92c8d5b431fcaa0815b2c87) )
	ROM_LOAD32_BYTE( "orc-1_4.u12",  0x0000002, 0x100000, CRC(4e82a34b) SHA1(c22a3f638b7e226add511147982339b1f59821e9) )
	ROM_LOAD32_BYTE( "orc-1_4.u13",  0x0000003, 0x100000, CRC(558b859c) SHA1(b7946a4b44976b08a691622000e1457021267d1a) )
	ROM_LOAD32_BYTE( "offroadc.u14", 0x0400000, 0x100000, CRC(1e41d14b) SHA1(3f7c5fae1f8b82ddd811720837fa298785a8dd27) )
	ROM_LOAD32_BYTE( "offroadc.u15", 0x0400001, 0x100000, CRC(654d623d) SHA1(a944b8f8d71b099d7b5bbd7df6effb90afc3aec8) )
	ROM_LOAD32_BYTE( "offroadc.u16", 0x0400002, 0x100000, CRC(259774d8) SHA1(90cdf659324b84b3c2c59497cc5611e8f12629a6) )
	ROM_LOAD32_BYTE( "offroadc.u17", 0x0400003, 0x100000, CRC(50c61434) SHA1(52bc603101b4f88b7d892af683b7c8358cabbf4a) )
	ROM_LOAD32_BYTE( "offroadc.u18", 0x0800000, 0x100000, CRC(015be91c) SHA1(1624537068c6bc5fa6235bf0b0343347c337e8d8) )
	ROM_LOAD32_BYTE( "offroadc.u19", 0x0800001, 0x100000, CRC(cfc6b70e) SHA1(8c5ad84c50ca142726db0595153cf04caaabec9c) )
	ROM_LOAD32_BYTE( "offroadc.u20", 0x0800002, 0x100000, CRC(f48d6e33) SHA1(8b9c205e24f217ac110cdd82388c056ebbbb09b0) )
	ROM_LOAD32_BYTE( "offroadc.u21", 0x0800003, 0x100000, CRC(17794b56) SHA1(8bfd8f5b43056bfe7f62524bb8c3a8564a3a9413) )
	ROM_LOAD32_BYTE( "offroadc.u22", 0x0c00000, 0x100000, CRC(f2a6e622) SHA1(a7d7004e95b058124cc02e8073dab8fbed8813c5) )
	ROM_LOAD32_BYTE( "offroadc.u23", 0x0c00001, 0x100000, CRC(1cba6e20) SHA1(a7c9c58bfc4d26decb08979d83cccedb27528eb6) )
	ROM_LOAD32_BYTE( "offroadc.u24", 0x0c00002, 0x100000, CRC(fd3ce11f) SHA1(78c65267712488784bc6dc14eef98a90494a9553) )
	ROM_LOAD32_BYTE( "offroadc.u25", 0x0c00003, 0x100000, CRC(78f8e5db) SHA1(7ec2a5add27d66c43ba5cb7182554321007f5798) )
ROM_END


ROM_START( offroadc3 ) /* Version 1.30, Mon 09-15-97 */
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "offroadc.u2",  0x000000, 0x80000, CRC(69976e9d) SHA1(63c886ac2563c43a10840f49f929f8613cd94de2) )
	ROM_LOAD16_BYTE( "offroadc.u3",  0x200000, 0x80000, CRC(2db9b548) SHA1(4f454a3e6a8851b0ef5d325dd28102d57ea11a11) )
	ROM_LOAD16_BYTE( "offroadc.u4",  0x400000, 0x80000, CRC(42bdf9d0) SHA1(04add0f0ee7fa61de1913cc0b988345d3d430cde) )
	ROM_LOAD16_BYTE( "offroadc.u5",  0x600000, 0x80000, CRC(569cc84b) SHA1(08b917cc41fae6b6a3e9d9461a783d3d2865e72a) )
	ROM_LOAD16_BYTE( "offroadc.u6",  0x800000, 0x80000, CRC(0896f679) SHA1(dde39ef17834256909ef2c9fcd5b5fb9939d5178) )
	ROM_LOAD16_BYTE( "offroadc.u7",  0xa00000, 0x80000, CRC(fe242d6a) SHA1(8fbac22ed23044841f309ce58c5b1affcdd5d114) )
	ROM_LOAD16_BYTE( "offroadc.u8",  0xc00000, 0x80000, CRC(5da13f12) SHA1(2bb5e929e8bc6c70cb4475024a6b0bb07ac25244) )
	ROM_LOAD16_BYTE( "offroadc.u9",  0xe00000, 0x80000, CRC(7ad27f69) SHA1(b33665d0593a95b58d529720aae49e90449bf714) )

	ROM_REGION32_LE( 0x1000000, "user1", 0 )
	ROM_LOAD32_BYTE( "orc-1_3.u10",  0x0000000, 0x100000, CRC(71c62ce2) SHA1(e6bdbf3df4795f4cf29a08641cc59d90aed73b57) ) /* Version 1.30 program roms */
	ROM_LOAD32_BYTE( "orc-1_3.u11",  0x0000001, 0x100000, CRC(9e362dbb) SHA1(2480710f1081679ff87239a8e28a9a3f235bd3dc) )
	ROM_LOAD32_BYTE( "orc-1_3.u12",  0x0000002, 0x100000, CRC(9e0a5b06) SHA1(63bbe427713fc966c65dab575dd42cdce6b00874) )
	ROM_LOAD32_BYTE( "orc-1_3.u13",  0x0000003, 0x100000, CRC(d602db7e) SHA1(48bc762a83baeb382476619f54631ccbe12d1b2c) )
	ROM_LOAD32_BYTE( "offroadc.u14", 0x0400000, 0x100000, CRC(1e41d14b) SHA1(3f7c5fae1f8b82ddd811720837fa298785a8dd27) )
	ROM_LOAD32_BYTE( "offroadc.u15", 0x0400001, 0x100000, CRC(654d623d) SHA1(a944b8f8d71b099d7b5bbd7df6effb90afc3aec8) )
	ROM_LOAD32_BYTE( "offroadc.u16", 0x0400002, 0x100000, CRC(259774d8) SHA1(90cdf659324b84b3c2c59497cc5611e8f12629a6) )
	ROM_LOAD32_BYTE( "offroadc.u17", 0x0400003, 0x100000, CRC(50c61434) SHA1(52bc603101b4f88b7d892af683b7c8358cabbf4a) )
	ROM_LOAD32_BYTE( "offroadc.u18", 0x0800000, 0x100000, CRC(015be91c) SHA1(1624537068c6bc5fa6235bf0b0343347c337e8d8) )
	ROM_LOAD32_BYTE( "offroadc.u19", 0x0800001, 0x100000, CRC(cfc6b70e) SHA1(8c5ad84c50ca142726db0595153cf04caaabec9c) )
	ROM_LOAD32_BYTE( "offroadc.u20", 0x0800002, 0x100000, CRC(f48d6e33) SHA1(8b9c205e24f217ac110cdd82388c056ebbbb09b0) )
	ROM_LOAD32_BYTE( "offroadc.u21", 0x0800003, 0x100000, CRC(17794b56) SHA1(8bfd8f5b43056bfe7f62524bb8c3a8564a3a9413) )
	ROM_LOAD32_BYTE( "offroadc.u22", 0x0c00000, 0x100000, CRC(f2a6e622) SHA1(a7d7004e95b058124cc02e8073dab8fbed8813c5) )
	ROM_LOAD32_BYTE( "offroadc.u23", 0x0c00001, 0x100000, CRC(1cba6e20) SHA1(a7c9c58bfc4d26decb08979d83cccedb27528eb6) )
	ROM_LOAD32_BYTE( "offroadc.u24", 0x0c00002, 0x100000, CRC(fd3ce11f) SHA1(78c65267712488784bc6dc14eef98a90494a9553) )
	ROM_LOAD32_BYTE( "offroadc.u25", 0x0c00003, 0x100000, CRC(78f8e5db) SHA1(7ec2a5add27d66c43ba5cb7182554321007f5798) )
ROM_END


ROM_START( offroadc1 ) /* Version 1.10, Mon 08-18-97 */
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "offroadc.u2",  0x000000, 0x80000, CRC(69976e9d) SHA1(63c886ac2563c43a10840f49f929f8613cd94de2) )
	ROM_LOAD16_BYTE( "offroadc.u3",  0x200000, 0x80000, CRC(2db9b548) SHA1(4f454a3e6a8851b0ef5d325dd28102d57ea11a11) )
	ROM_LOAD16_BYTE( "offroadc.u4",  0x400000, 0x80000, CRC(42bdf9d0) SHA1(04add0f0ee7fa61de1913cc0b988345d3d430cde) )
	ROM_LOAD16_BYTE( "offroadc.u5",  0x600000, 0x80000, CRC(569cc84b) SHA1(08b917cc41fae6b6a3e9d9461a783d3d2865e72a) )
	ROM_LOAD16_BYTE( "offroadc.u6",  0x800000, 0x80000, CRC(0896f679) SHA1(dde39ef17834256909ef2c9fcd5b5fb9939d5178) )
	ROM_LOAD16_BYTE( "offroadc.u7",  0xa00000, 0x80000, CRC(fe242d6a) SHA1(8fbac22ed23044841f309ce58c5b1affcdd5d114) )
	ROM_LOAD16_BYTE( "offroadc.u8",  0xc00000, 0x80000, CRC(5da13f12) SHA1(2bb5e929e8bc6c70cb4475024a6b0bb07ac25244) )
	ROM_LOAD16_BYTE( "offroadc.u9",  0xe00000, 0x80000, CRC(7ad27f69) SHA1(b33665d0593a95b58d529720aae49e90449bf714) )

	ROM_REGION32_LE( 0x1000000, "user1", 0 )
	ROM_LOAD32_BYTE( "orc-1_1.u10",  0x0000000, 0x100000, CRC(4729660c) SHA1(0baff6a27015f4eb3fe0a981ecbac33d140e872a) ) /* Version 1.10 program roms */
	ROM_LOAD32_BYTE( "orc-1_1.u11",  0x0000001, 0x100000, CRC(6272d013) SHA1(860121184282627ed692e56a0dafee8b64562811) )
	ROM_LOAD32_BYTE( "orc-1_1.u12",  0x0000002, 0x100000, CRC(9c8326be) SHA1(55f16d14379f57d08ed84d82f9db1a582bc223a1) )
	ROM_LOAD32_BYTE( "orc-1_1.u13",  0x0000003, 0x100000, CRC(53bbc181) SHA1(1ab29a27a216eb09d69a9f3d681865de1a904717) )
	ROM_LOAD32_BYTE( "offroadc.u14", 0x0400000, 0x100000, CRC(1e41d14b) SHA1(3f7c5fae1f8b82ddd811720837fa298785a8dd27) )
	ROM_LOAD32_BYTE( "offroadc.u15", 0x0400001, 0x100000, CRC(654d623d) SHA1(a944b8f8d71b099d7b5bbd7df6effb90afc3aec8) )
	ROM_LOAD32_BYTE( "offroadc.u16", 0x0400002, 0x100000, CRC(259774d8) SHA1(90cdf659324b84b3c2c59497cc5611e8f12629a6) )
	ROM_LOAD32_BYTE( "offroadc.u17", 0x0400003, 0x100000, CRC(50c61434) SHA1(52bc603101b4f88b7d892af683b7c8358cabbf4a) )
	ROM_LOAD32_BYTE( "offroadc.u18", 0x0800000, 0x100000, CRC(015be91c) SHA1(1624537068c6bc5fa6235bf0b0343347c337e8d8) )
	ROM_LOAD32_BYTE( "offroadc.u19", 0x0800001, 0x100000, CRC(cfc6b70e) SHA1(8c5ad84c50ca142726db0595153cf04caaabec9c) )
	ROM_LOAD32_BYTE( "offroadc.u20", 0x0800002, 0x100000, CRC(f48d6e33) SHA1(8b9c205e24f217ac110cdd82388c056ebbbb09b0) )
	ROM_LOAD32_BYTE( "offroadc.u21", 0x0800003, 0x100000, CRC(17794b56) SHA1(8bfd8f5b43056bfe7f62524bb8c3a8564a3a9413) )
	ROM_LOAD32_BYTE( "offroadc.u22", 0x0c00000, 0x100000, CRC(f2a6e622) SHA1(a7d7004e95b058124cc02e8073dab8fbed8813c5) )
	ROM_LOAD32_BYTE( "offroadc.u23", 0x0c00001, 0x100000, CRC(1cba6e20) SHA1(a7c9c58bfc4d26decb08979d83cccedb27528eb6) )
	ROM_LOAD32_BYTE( "offroadc.u24", 0x0c00002, 0x100000, CRC(fd3ce11f) SHA1(78c65267712488784bc6dc14eef98a90494a9553) )
	ROM_LOAD32_BYTE( "offroadc.u25", 0x0c00003, 0x100000, CRC(78f8e5db) SHA1(7ec2a5add27d66c43ba5cb7182554321007f5798) )
ROM_END


/*
War Gods
Midway, 1996

This game runs on hardware that appears to be similar to Cruisin' USA but using
a 420M 2.5" IDE hard drive. Only about 100M of the hard drive is used.
There are only 2 ROMs located at U12 and U41.

PCB LAYOUT
|-------------------------------------------------------------------------------------------------------------------|
|                                                                         SEAGATE ST9420AG                          |
|                                                                                                                   |
|                                                                                                                   |
|                                                                                                                   |
|                                                 |-------------|                                                   |
|NEC431008LE-15 x4          LH540204U-20 x2       |MIDWAY       |                U12              16.000MHz         |
|                                                 |5410-14591-00|                |-------------|                    |
| |---------|                      |-------------||(C)1995      |                |MIDWAY       |                    |
| |TMS320C31|         16.6667MHz   |LSI LIA7968  ||             |                |5410-14590-00|   ADSP-2115        |
| |PQL60    |         40.0000MHz   |5410-1346500 ||             |                |(C)1995      |                    |
| |         |                      |MIDWAY MFG CO||-------------|                |-------------|   NEC4218160 x1    |
| |         |                      |             |                                   M628032-15E x3                 |
| |---------|                      |             |                        ALTERA                                    |
|  60.00MHz                        |-------------|  NEC4218160-60 x2      EPM7032LC44-15T                           |
|                                                                                                                   |
|                                                                                                                   |
|                                                                            U41                                    |
|                                                                                                                   |
|                         TMS55165DGH-70 x2          NEC424260-70 x4                                                |
|                                                                                                                   |
|                                                                                  4MHz                  BATT+3V    |
|TL084 x2                                                                          PIC16C57(U69)  MAX232            |
|                         M628032-20E x2                                    |-------------|                         |-|
|AD1866                                                                     |MIDWAY       |     SW2   ULN2064B      | |9 PIN
|                                                                           |5410-14589-00|                         | |SERIAL
|                                                                           |(C) 1995     |     SW1                 |-|LINK
|                                                                           |             |                         |
|                                                                           |             |                         |
|                                                                           |-------------|                         |
|                                                                                                                   |
|                             |--|           J   A   M   M   A           |--|                                       |
|-----------------------------|  |---------------------------------------|  |---------------------------------------|
*/

ROM_START( wargods ) /* Boot EPROM Version 1.0, Game Type: 452 (10/09/1996) */
	ROM_REGION16_LE( 0x10000, "dcs", 0 )    /* sound data */
	ROM_LOAD16_BYTE( "u2.rom",   0x000000, 0x8000, CRC(bec7d3ae) SHA1(db80aa4a645804a4574b07b9f34dec6b6b64190d) )

	ROM_REGION32_LE( 0x1000000, "user1", 0 )
	ROM_LOAD( "u41.rom", 0x000000, 0x20000, CRC(398c54cc) SHA1(6c4b5d6ec5c844dcbf181f9d86a9196a088ed2db) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "wargods_10-09-1996", 0, SHA1(7585bc65b1038589cb59d3e7c56e08ca9d7015b8) )
ROM_END

ROM_START( wargodsa ) /* Boot EPROM Version 1.0, Game Type: 452 (08/15/1996) */
	ROM_REGION16_LE( 0x10000, "dcs", 0 )    /* sound data */
	ROM_LOAD16_BYTE( "u2.rom",   0x000000, 0x8000, CRC(bec7d3ae) SHA1(db80aa4a645804a4574b07b9f34dec6b6b64190d) )

	ROM_REGION32_LE( 0x1000000, "user1", 0 )
	ROM_LOAD( "u41.rom", 0x000000, 0x20000, CRC(398c54cc) SHA1(6c4b5d6ec5c844dcbf181f9d86a9196a088ed2db) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "wargods_08-15-1996", 0, SHA1(5dee00be40c315fbb1d6e3994dae8e498ab87fb2) )
ROM_END

ROM_START( wargodsb ) /* Boot EPROM Version 1.0, Game Type: 452 (12/11/1995) */
	ROM_REGION16_LE( 0x10000, "dcs", 0 )    /* sound data */
	ROM_LOAD16_BYTE( "u2.rom",   0x000000, 0x8000, CRC(bec7d3ae) SHA1(db80aa4a645804a4574b07b9f34dec6b6b64190d) )

	ROM_REGION32_LE( 0x1000000, "user1", 0 )
	ROM_LOAD( "u41.rom", 0x000000, 0x20000, CRC(398c54cc) SHA1(6c4b5d6ec5c844dcbf181f9d86a9196a088ed2db) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "wargods_12-11-1995", 0, SHA1(141063f95867fdcc4b15c844e510696604a70c6a) )
ROM_END



/*************************************
 *
 *  Driver init
 *
 *************************************/

READ32_MEMBER(midvunit_state::generic_speedup_r)
{
	space.device().execute().eat_cycles(100);
	return m_generic_speedup[offset];
}


void midvunit_state::init_crusnusa_common(offs_t speedup)
{
	m_adc_shift = 24;

	/* speedups */
	m_generic_speedup = m_maincpu->space(AS_PROGRAM).install_read_handler(speedup, speedup + 1, read32_delegate(FUNC(midvunit_state::generic_speedup_r),this));
}
DRIVER_INIT_MEMBER(midvunit_state,crusnusa)  { init_crusnusa_common(0xc93e); }
DRIVER_INIT_MEMBER(midvunit_state,crusnu40)  { init_crusnusa_common(0xc957); }
DRIVER_INIT_MEMBER(midvunit_state,crusnu21)  { init_crusnusa_common(0xc051); }


void midvunit_state::init_crusnwld_common(offs_t speedup)
{
	m_adc_shift = 16;

	/* control register is different */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x994000, 0x994000, write32_delegate(FUNC(midvunit_state::crusnwld_control_w),this));

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x991030, 0x991030, read32_delegate(FUNC(midvunit_state::crusnwld_serial_status_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x996000, 0x996000, read32_delegate(FUNC(midvunit_state::crusnwld_serial_data_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x996000, 0x996000, write32_delegate(FUNC(midvunit_state::crusnwld_serial_data_w),this));

	/* install strange protection device */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x9d0000, 0x9d1fff, read32_delegate(FUNC(midvunit_state::bit_data_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x9d0000, 0x9d0000, write32_delegate(FUNC(midvunit_state::bit_reset_w),this));

	/* speedups */
	if (speedup)
		m_generic_speedup = m_maincpu->space(AS_PROGRAM).install_read_handler(speedup, speedup + 1, read32_delegate(FUNC(midvunit_state::generic_speedup_r),this));
}
DRIVER_INIT_MEMBER(midvunit_state,crusnwld)  { init_crusnwld_common(0xd4c0); }
#if 0
DRIVER_INIT_MEMBER(midvunit_state,crusnw20)  { init_crusnwld_common(0xd49c); }
DRIVER_INIT_MEMBER(midvunit_state,crusnw13)  { init_crusnwld_common(0); }
#endif

DRIVER_INIT_MEMBER(midvunit_state,offroadc)
{
	m_adc_shift = 16;

	/* control register is different */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x994000, 0x994000, write32_delegate(FUNC(midvunit_state::crusnwld_control_w),this));

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x991030, 0x991030, read32_delegate(FUNC(midvunit_state::offroadc_serial_status_r),this));
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x996000, 0x996000, read32_delegate(FUNC(midvunit_state::offroadc_serial_data_r),this), write32_delegate(FUNC(midvunit_state::offroadc_serial_data_w),this));

	/* speedups */
	m_generic_speedup = m_maincpu->space(AS_PROGRAM).install_read_handler(0x195aa, 0x195aa, read32_delegate(FUNC(midvunit_state::generic_speedup_r),this));
}


DRIVER_INIT_MEMBER(midvunit_state,wargods)
{
	UINT8 default_nvram[256];

	/* initialize the subsystems */
	m_adc_shift = 16;

	/* we need proper VRAM */
	memset(default_nvram, 0xff, sizeof(default_nvram));
	default_nvram[0x0e] = default_nvram[0x2e] = 0x67;
	default_nvram[0x0f] = default_nvram[0x2f] = 0x32;
	default_nvram[0x10] = default_nvram[0x30] = 0x0a;
	default_nvram[0x11] = default_nvram[0x31] = 0x00;
	default_nvram[0x12] = default_nvram[0x32] = 0xaf;
	default_nvram[0x17] = default_nvram[0x37] = 0xd8;
	default_nvram[0x18] = default_nvram[0x38] = 0xe7;
	machine().device<midway_ioasic_device>("ioasic")->set_default_nvram(default_nvram);

	/* speedups */
	m_generic_speedup = m_maincpu->space(AS_PROGRAM).install_read_handler(0x2f4c, 0x2f4c, read32_delegate(FUNC(midvunit_state::generic_speedup_r),this));
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1994, crusnusa,   0,        midvunit, crusnusa, midvunit_state, crusnusa, ROT0, "Midway", "Cruis'n USA (rev L4.1)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, crusnusa40, crusnusa, midvunit, crusnusa, midvunit_state, crusnu40, ROT0, "Midway", "Cruis'n USA (rev L4.0)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, crusnusa21, crusnusa, midvunit, crusnusa, midvunit_state, crusnu21, ROT0, "Midway", "Cruis'n USA (rev L2.1)", MACHINE_SUPPORTS_SAVE )

GAME( 1996, crusnwld,   0,        crusnwld, crusnwld, midvunit_state, crusnwld, ROT0, "Midway", "Cruis'n World (rev L2.5)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, crusnwld24, crusnwld, crusnwld, crusnwld, midvunit_state, crusnwld, ROT0, "Midway", "Cruis'n World (rev L2.4)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, crusnwld23, crusnwld, crusnwld, crusnwld, midvunit_state, crusnwld, ROT0, "Midway", "Cruis'n World (rev L2.3)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, crusnwld20, crusnwld, crusnwld, crusnwld, midvunit_state, crusnwld, ROT0, "Midway", "Cruis'n World (rev L2.0)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, crusnwld19, crusnwld, crusnwld, crusnwld, midvunit_state, crusnwld, ROT0, "Midway", "Cruis'n World (rev L1.9)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, crusnwld17, crusnwld, crusnwld, crusnwld, midvunit_state, crusnwld, ROT0, "Midway", "Cruis'n World (rev L1.7)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, crusnwld13, crusnwld, crusnwld, crusnwld, midvunit_state, crusnwld, ROT0, "Midway", "Cruis'n World (rev L1.3)", MACHINE_SUPPORTS_SAVE )

GAME( 1997, offroadc,  0,        offroadc, offroadc, midvunit_state, offroadc, ROT0, "Midway", "Off Road Challenge (v1.63)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, offroadc5, offroadc, offroadc, offroadc, midvunit_state, offroadc, ROT0, "Midway", "Off Road Challenge (v1.50)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, offroadc4, offroadc, offroadc, offroadc, midvunit_state, offroadc, ROT0, "Midway", "Off Road Challenge (v1.40)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, offroadc3, offroadc, offroadc, offroadc, midvunit_state, offroadc, ROT0, "Midway", "Off Road Challenge (v1.30)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, offroadc1, offroadc, offroadc, offroadc, midvunit_state, offroadc, ROT0, "Midway", "Off Road Challenge (v1.10)", MACHINE_SUPPORTS_SAVE )

GAME( 1995, wargods,   0,        midvplus, wargods, midvunit_state,  wargods,  ROT0, "Midway", "War Gods (HD 10/09/1996 - Dual Resolution)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, wargodsa,  wargods,  midvplus, wargodsa, midvunit_state, wargods,  ROT0, "Midway", "War Gods (HD 08/15/1996)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, wargodsb,  wargods,  midvplus, wargodsa, midvunit_state, wargods,  ROT0, "Midway", "War Gods (HD 12/11/1995)", MACHINE_SUPPORTS_SAVE )
