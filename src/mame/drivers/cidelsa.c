#include "driver.h"
#include "cpu/cdp1802/cdp1802.h"
#include "cpu/cop400/cop400.h"
#include "video/cdp1869.h"
#include "sound/cdp1869.h"
#include "sound/ay8910.h"

/*

    TODO:

    - move set_cpu_mode timer call to MDRV
    - fix COP420 core to get sound in Draco

*/

#define DESTRYER_CHR1	3579000.0 // unverified
#define DESTRYER_CHR2	XTAL_5_7143MHz
#define ALTAIR_CHR1		3579000.0 // unverified
#define ALTAIR_CHR2		CDP1869_DOT_CLK_PAL // unverified
#define DRACO_CHR1		XTAL_4_43361MHz
#define DRACO_CHR2		CDP1869_DOT_CLK_PAL // unverified
#define DRACO_SND_CHR1	XTAL_2_01216MHz

/* CDP1802 Interface */

static int cdp1869_prd;
static int cdp1869_pcb;
static UINT8 *pcb_ram;  // 2048x1 bit PCB ram

static int cdp1802_mode = CDP1802_MODE_RESET;

static UINT8 cidelsa_mode_r(void)
{
	return cdp1802_mode;
}

static UINT8 cidelsa_ef_r(void)
{
	return readinputportbytag("EF");
}

static const CDP1802_CONFIG cidelsa_cdp1802_config =
{
	cidelsa_mode_r,	// MODE input
	cidelsa_ef_r,	// EF input
	NULL,			// SC output
	NULL,			// Q output
	NULL,			// DMA read
	NULL			// DMA write
};

/* Draco Sound Interface */

static int draco_sound;
static int draco_ay_latch;

static WRITE8_HANDLER ( draco_sound_bankswitch_w )
{
	memory_set_bank(1, BIT(data, 3));
}

static WRITE8_HANDLER ( draco_sound_g_w )
{
	/*

		 G1 G0  description

		  0  0  IAB     inactive
		  0  1  DWS     write to PSG
		  1  0  DTB     read from PSG
		  1  1  INTAK   latch address

    */

	switch (data)
	{
	case 0x01:
		AY8910_write_port_0_w(machine, 0, draco_ay_latch);
		break;

	case 0x02:
		draco_ay_latch = AY8910_read_port_0_r(machine, 0);
		break;

	case 0x03:
		AY8910_control_port_0_w(machine, 0, draco_ay_latch);
		break;
	}
}

static READ8_HANDLER ( draco_sound_in_r )
{
	return ~draco_sound & 0x07; // inverted
}

static READ8_HANDLER ( draco_sound_ay8910_r )
{
	return draco_ay_latch;
}

static WRITE8_HANDLER ( draco_sound_ay8910_w )
{
	draco_ay_latch = data;
}

static WRITE8_HANDLER ( draco_ay8910_port_a_w )
{
	/*

      bit   description

        0   N/C
        1   N/C
        2   N/C
        3   N/C
        4   N/C
        5   N/C
        6   N/C
        7   N/C

    */
}

static WRITE8_HANDLER ( draco_ay8910_port_b_w )
{
	/*

      bit   description

        0   RELE0
        1   RELE1
        2   sound output -> * -> 22K capacitor -> GND
        3   sound output -> * -> 220K capacitor -> GND
        4   5V -> 1K resistor -> * -> 10uF capacitor -> GND (volume pot voltage adjustment)
        5   N/C
        6   N/C
        7   N/C

    */
}

static const struct AY8910interface ay8910_interface =
{
	0,						// port A read
	0,						// port B read
	draco_ay8910_port_a_w,	// port A write
	draco_ay8910_port_b_w	// port B write
};

/* Read/Write Handlers */

static WRITE8_HANDLER ( destryer_out1_w )
{
	/*

      bit   description

        0
        1
        2
        3
        4
        5
        6
        7

    */
}

static WRITE8_HANDLER ( altair_out1_w )
{
	/*

      bit   description

        0   S1 (CARTUCHO)
        1   S2 (CARTUCHO)
        2   S3 (CARTUCHO)
        3   LG1
        4   LG2
        5   LGF
        6   CONT. M2
        7   CONT. M1

    */

	set_led_status(0, BIT(data, 3)); // 1P
	set_led_status(1, BIT(data, 4)); // 2P
	set_led_status(2, BIT(data, 5)); // FIRE
}

static WRITE8_HANDLER ( draco_out1_w )
{
	/*

      bit   description

        0   3K9 -> Green signal
        1   820R -> Blue signal
        2   510R -> Red signal
        3   1K -> N/C
        4   N/C
        5   SONIDO A -> COP402 IN0
        6   SONIDO B -> COP402 IN1
        7   SONIDO C -> COP402 IN2

    */

    draco_sound = (data >> 5) & 0x07;
}

static WRITE8_HANDLER ( cidelsa_charram_w )
{
	int addr = cdp1869_get_cma(offset);

	pcb_ram[addr] = activecpu_get_reg(CDP1802_Q);

	cdp1869_charram_w(machine, offset, data);
}

static READ8_HANDLER ( cidelsa_charram_r )
{
	int addr = cdp1869_get_cma(offset);

	cdp1869_pcb = pcb_ram[addr];

	return cdp1869_charram_r(machine, offset);
}

/* Memory Maps */

// Destroyer

static ADDRESS_MAP_START( destryer_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x20ff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0xf400, 0xf7ff) AM_READWRITE(cidelsa_charram_r, cidelsa_charram_w)
	AM_RANGE(0xf800, 0xffff) AM_READWRITE(cdp1869_pageram_r, cdp1869_pageram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( destryea_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x3000, 0x30ff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0xf400, 0xf7ff) AM_READWRITE(cidelsa_charram_r, cidelsa_charram_w)
	AM_RANGE(0xf800, 0xffff) AM_READWRITE(cdp1869_pageram_r, cdp1869_pageram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( destryer_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN0") AM_WRITE(destryer_out1_w)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN1")
	AM_RANGE(0x03, 0x03) AM_WRITE(cdp1869_out3_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(cdp1869_out4_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(cdp1869_out5_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(cdp1869_out6_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(cdp1869_out7_w)
ADDRESS_MAP_END

// Altair

static ADDRESS_MAP_START( altair_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x3000, 0x30ff) AM_RAM
	AM_RANGE(0xf400, 0xf7ff) AM_READWRITE(cidelsa_charram_r, cidelsa_charram_w)
	AM_RANGE(0xf800, 0xffff) AM_READWRITE(cdp1869_pageram_r, cdp1869_pageram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( altair_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN0") AM_WRITE(altair_out1_w)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN1")
	AM_RANGE(0x03, 0x03) AM_WRITE(cdp1869_out3_w)
	AM_RANGE(0x04, 0x04) AM_READ_PORT("IN2") AM_WRITE(cdp1869_out4_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(cdp1869_out5_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(cdp1869_out6_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(cdp1869_out7_w)
ADDRESS_MAP_END

// Draco

static ADDRESS_MAP_START( draco_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0xf400, 0xf7ff) AM_READWRITE(cidelsa_charram_r, cidelsa_charram_w)
	AM_RANGE(0xf800, 0xffff) AM_READWRITE(cdp1869_pageram_r, cdp1869_pageram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( draco_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN0") AM_WRITE(draco_out1_w)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN1")
	AM_RANGE(0x03, 0x03) AM_WRITE(cdp1869_out3_w)
	AM_RANGE(0x04, 0x04) AM_READ_PORT("IN2") AM_WRITE(cdp1869_out4_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(cdp1869_out5_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(cdp1869_out6_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(cdp1869_out7_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( draco_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x000, 0x3ff) AM_ROMBANK(1)
ADDRESS_MAP_END

static ADDRESS_MAP_START( draco_sound_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(COP400_PORT_D, COP400_PORT_D) AM_WRITE(draco_sound_bankswitch_w)
	AM_RANGE(COP400_PORT_G, COP400_PORT_G) AM_WRITE(draco_sound_g_w)
	AM_RANGE(COP400_PORT_L, COP400_PORT_L) AM_READWRITE(draco_sound_ay8910_r, draco_sound_ay8910_w)
	AM_RANGE(COP400_PORT_IN, COP400_PORT_IN) AM_READ(draco_sound_in_r)
ADDRESS_MAP_END

/* Input Ports */

static CUSTOM_INPUT( cdp1869_pcb_r )
{
	return cdp1869_pcb;
}

static CUSTOM_INPUT( cdp1869_predisplay_r )
{
	return cdp1869_prd;
}

static INPUT_PORTS_START( destryer )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // CARTUCHO
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 ) // 1P
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 ) // 2P
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) // RG
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) // LF
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) // FR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(cdp1869_pcb_r, 0)

	PORT_START_TAG("IN1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, "Very Conserv" )
	PORT_DIPSETTING(    0x01, "Conserv" )
	PORT_DIPSETTING(    0x02, "Liberal" )
	PORT_DIPSETTING(    0x03, "Very Liberal" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "5000" )
	PORT_DIPSETTING(    0x08, "7000" )
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x00, "14000" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR(Coinage) )
	PORT_DIPSETTING(    0xc0, "Slot A: 1  Slot B: 2" )
	PORT_DIPSETTING(    0x80, "Slot A: 1.5  Slot B: 3" )
	PORT_DIPSETTING(    0x40, "Slot A: 2  Slot B: 4" )
	PORT_DIPSETTING(    0x00, "Slot A: 2.5  Slot B: 5" )

	PORT_START_TAG("EF")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(cdp1869_predisplay_r, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) // ST
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) // M2
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) // M1
INPUT_PORTS_END

static INPUT_PORTS_START( altair )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // CARTUCHO
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 ) // 1P
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 ) // 2P
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) // RG
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) // LF
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) // FR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(cdp1869_pcb_r, 0)

	PORT_START_TAG("IN1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, "Very Conserv" )
	PORT_DIPSETTING(    0x01, "Conserv" )
	PORT_DIPSETTING(    0x02, "Liberal" )
	PORT_DIPSETTING(    0x03, "Very Liberal" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "5000" )
	PORT_DIPSETTING(    0x08, "7000" )
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x00, "14000" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR(Coinage) )
	PORT_DIPSETTING(    0xc0, "Slot A: 1  Slot B: 2" )
	PORT_DIPSETTING(    0x80, "Slot A: 1.5  Slot B: 3" )
	PORT_DIPSETTING(    0x40, "Slot A: 2  Slot B: 4" )
	PORT_DIPSETTING(    0x00, "Slot A: 2.5  Slot B: 5" )

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) // UP
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) // DN
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) // IN
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("EF")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(cdp1869_predisplay_r, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) // ST
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) // M2
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) // M1
INPUT_PORTS_END

static INPUT_PORTS_START( draco )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(cdp1869_pcb_r, 0)

	PORT_START_TAG("IN1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, "Very Conserv" )
	PORT_DIPSETTING(    0x01, "Conserv" )
	PORT_DIPSETTING(    0x02, "Liberal" )
	PORT_DIPSETTING(    0x03, "Very Liberal" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "5000" )
	PORT_DIPSETTING(    0x08, "7000" )
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x00, "14000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0xe0, 0xc0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, "Slot A: 0.5  Slot B: 0.5" )
	PORT_DIPSETTING(    0xe0, "Slot A: 0.6  Slot B: 3" )
	PORT_DIPSETTING(    0xc0, "Slot A: 1  Slot B: 2" )
	PORT_DIPSETTING(    0xa0, "Slot A: 1.2  Slot B: 6" )
	PORT_DIPSETTING(    0x80, "Slot A: 1.5  Slot B: 3" )
	PORT_DIPSETTING(    0x60, "Slot A: 1.5  Slot B: 7.5" )
	PORT_DIPSETTING(    0x20, "Slot A: 2.5  Slot B: 3" )
	PORT_DIPSETTING(    0x00, "Slot A: 3  Slot B: 6" )

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT )

	PORT_START_TAG("EF")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(cdp1869_predisplay_r, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) // ST
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) // M2
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) // M1
INPUT_PORTS_END

/* Video Timer Callbacks */

static TIMER_CALLBACK( altair_prd_start )
{
	cpunum_set_input_line(machine, 0, INPUT_LINE_IRQ0, ASSERT_LINE);
	cdp1869_prd = 1; // inverted
}

static TIMER_CALLBACK( altair_prd_end )
{
	cpunum_set_input_line(machine, 0, INPUT_LINE_IRQ0, CLEAR_LINE);
	cdp1869_prd = 0; // inverted
}

static TIMER_CALLBACK( draco_prd_start )
{
	cdp1869_prd = 0;
}

static TIMER_CALLBACK( draco_prd_end )
{
	cdp1869_prd = 1;
}

/* Machine Start */

static TIMER_CALLBACK( set_cpu_mode )
{
	cdp1802_mode = CDP1802_MODE_RUN;
}

static UINT8 cidelsa_get_color_bits(UINT8 cramdata, UINT16 cramaddr, UINT16 pramaddr)
{
	int ccb0 = BIT(cramdata, 6);
	int ccb1 = BIT(cramdata, 7);
	int pcb = BIT(pcb_ram[cramaddr & 0x7ff], 0);

	return (ccb0 << 2) + (ccb1 << 1) + pcb;
}

static const CDP1869_interface destryer_CDP1869_interface =
{
	CDP1869_PAL,			// display format
	REGION_INVALID,			// character RAM region
	0x800,					// character RAM size
	0x400,					// page RAM size
	cidelsa_get_color_bits	// color callback function
};

static const CDP1869_interface draco_CDP1869_interface =
{
	CDP1869_PAL,			// display format
	REGION_INVALID,			// character RAM region
	0x800,					// character RAM size
	0x800,					// page RAM size
	cidelsa_get_color_bits	// color callback function
};

static MACHINE_START( destryer )
{
	// allocate PCB RAM

	pcb_ram = auto_malloc(0x800);

	// configure CDP1869

	cdp1869_configure(&destryer_CDP1869_interface);

	// allocate one-shot CPU mode set timer

	timer_set(ATTOTIME_IN_MSEC(200), NULL, 0, set_cpu_mode);

	// save state support

	state_save_register_global(cdp1802_mode);
	state_save_register_global_pointer(pcb_ram, 0x800);
	state_save_register_global(cdp1869_prd);
	state_save_register_global(cdp1869_pcb);
}

static MACHINE_START( draco )
{
	// configure COP402 ROM banking

	UINT8 *ROM = memory_region(REGION_CPU2);
	memory_configure_bank(1, 0, 2, &ROM[0x000], 0x400);

	// allocate PCB RAM

	pcb_ram = auto_malloc(0x800);

	// configure CDP1869

	cdp1869_configure(&draco_CDP1869_interface);

	// allocate one-shot CPU mode set timer

	timer_set(ATTOTIME_IN_MSEC(200), NULL, 0, set_cpu_mode);

	// save state support

	state_save_register_global(cdp1802_mode);
	state_save_register_global_pointer(pcb_ram, 0x800);
	state_save_register_global(cdp1869_prd);
	state_save_register_global(cdp1869_pcb);
	state_save_register_global(draco_sound);
	state_save_register_global(draco_ay_latch);
}

/* Machine Reset */

static MACHINE_RESET( destryer )
{
	cpunum_set_input_line(machine, 0, INPUT_LINE_RESET, PULSE_LINE);
}

/* Machine Drivers */

static MACHINE_DRIVER_START( destryer )
	// basic system hardware

	MDRV_CPU_ADD(CDP1802, DESTRYER_CHR1)
	MDRV_CPU_PROGRAM_MAP(destryer_map, 0)
	MDRV_CPU_IO_MAP(destryer_io_map, 0)
	MDRV_CPU_CONFIG(cidelsa_cdp1802_config)
	MDRV_NVRAM_HANDLER(generic_0fill)
	MDRV_MACHINE_START(destryer)
	MDRV_MACHINE_RESET(destryer)

	// video hardware

	MDRV_PALETTE_LENGTH(8+64)
	MDRV_PALETTE_INIT(cdp1869)
	MDRV_VIDEO_START(cdp1869)
	MDRV_VIDEO_UPDATE(cdp1869)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(DESTRYER_CHR2, CDP1869_SCREEN_WIDTH, CDP1869_HBLANK_END, CDP1869_HBLANK_START, CDP1869_TOTAL_SCANLINES_PAL, CDP1869_SCANLINE_VBLANK_END_PAL, CDP1869_SCANLINE_VBLANK_START_PAL)
	MDRV_SCREEN_DEFAULT_POSITION(1.226, 0.012, 1.4, 0.044)

	MDRV_TIMER_ADD_SCANLINE("prd_start", altair_prd_start, "main", CDP1869_SCANLINE_PREDISPLAY_START_PAL, CDP1869_TOTAL_SCANLINES_PAL)
	MDRV_TIMER_ADD_SCANLINE("prd_end", altair_prd_end, "main", CDP1869_SCANLINE_PREDISPLAY_END_PAL, CDP1869_TOTAL_SCANLINES_PAL)

	// sound hardware

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(CDP1869, DESTRYER_CHR2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( destryea )
	// basic system hardware

	MDRV_CPU_ADD(CDP1802, DESTRYER_CHR1)
	MDRV_CPU_PROGRAM_MAP(destryea_map, 0)
	MDRV_CPU_IO_MAP(destryer_io_map, 0)
	MDRV_CPU_CONFIG(cidelsa_cdp1802_config)
	MDRV_NVRAM_HANDLER(generic_0fill)
	MDRV_MACHINE_START(destryer)
	MDRV_MACHINE_RESET(destryer)

	// video hardware

	MDRV_PALETTE_LENGTH(8+64)
	MDRV_PALETTE_INIT(cdp1869)
	MDRV_VIDEO_START(cdp1869)
	MDRV_VIDEO_UPDATE(cdp1869)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(DESTRYER_CHR2, CDP1869_SCREEN_WIDTH, CDP1869_HBLANK_END, CDP1869_HBLANK_START, CDP1869_TOTAL_SCANLINES_PAL, CDP1869_SCANLINE_VBLANK_END_PAL, CDP1869_SCANLINE_VBLANK_START_PAL)
	MDRV_SCREEN_DEFAULT_POSITION(1.226, 0.012, 1.4, 0.044)

	MDRV_TIMER_ADD_SCANLINE("prd_start", altair_prd_start, "main", CDP1869_SCANLINE_PREDISPLAY_START_PAL, CDP1869_TOTAL_SCANLINES_PAL)
	MDRV_TIMER_ADD_SCANLINE("prd_end", altair_prd_end, "main", CDP1869_SCANLINE_PREDISPLAY_END_PAL, CDP1869_TOTAL_SCANLINES_PAL)

	// sound hardware

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(CDP1869, DESTRYER_CHR2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( altair )
	// basic system hardware

	MDRV_CPU_ADD(CDP1802, ALTAIR_CHR1)
	MDRV_CPU_PROGRAM_MAP(altair_map, 0)
	MDRV_CPU_IO_MAP(altair_io_map, 0)
	MDRV_CPU_CONFIG(cidelsa_cdp1802_config)
	MDRV_MACHINE_START(destryer)
	MDRV_MACHINE_RESET(destryer)

	// video hardware

	MDRV_PALETTE_LENGTH(8+64)
	MDRV_PALETTE_INIT(cdp1869)
	MDRV_VIDEO_START(cdp1869)
	MDRV_VIDEO_UPDATE(cdp1869)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(ALTAIR_CHR2, CDP1869_SCREEN_WIDTH, CDP1869_HBLANK_END, CDP1869_HBLANK_START, CDP1869_TOTAL_SCANLINES_PAL, CDP1869_SCANLINE_VBLANK_END_PAL, CDP1869_SCANLINE_VBLANK_START_PAL)
	MDRV_SCREEN_DEFAULT_POSITION(1.226, 0.012, 1.4, 0.044)

	MDRV_TIMER_ADD_SCANLINE("prd_start", altair_prd_start, "main", CDP1869_SCANLINE_PREDISPLAY_START_PAL, CDP1869_TOTAL_SCANLINES_PAL)
	MDRV_TIMER_ADD_SCANLINE("prd_end", altair_prd_end, "main", CDP1869_SCANLINE_PREDISPLAY_END_PAL, CDP1869_TOTAL_SCANLINES_PAL)

	// sound hardware

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(CDP1869, ALTAIR_CHR2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( draco )
	// basic system hardware

	MDRV_CPU_ADD(CDP1802, DRACO_CHR1)
	MDRV_CPU_PROGRAM_MAP(draco_map, 0)
	MDRV_CPU_IO_MAP(draco_io_map, 0)
	MDRV_CPU_CONFIG(cidelsa_cdp1802_config)
	MDRV_NVRAM_HANDLER(generic_0fill)
	MDRV_MACHINE_START(draco)
	MDRV_MACHINE_RESET(destryer)

	MDRV_CPU_ADD(COP420, DRACO_SND_CHR1) // COP402N
	MDRV_CPU_PROGRAM_MAP(draco_sound_map, 0)
	MDRV_CPU_IO_MAP(draco_sound_io_map, 0)

	// video hardware

	MDRV_PALETTE_LENGTH(8+64)
	MDRV_PALETTE_INIT(cdp1869)
	MDRV_VIDEO_START(cdp1869)
	MDRV_VIDEO_UPDATE(cdp1869)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(DRACO_CHR2, CDP1869_SCREEN_WIDTH, CDP1869_HBLANK_END, CDP1869_HBLANK_START, CDP1869_TOTAL_SCANLINES_PAL, CDP1869_SCANLINE_VBLANK_END_PAL, CDP1869_SCANLINE_VBLANK_START_PAL)
	MDRV_SCREEN_DEFAULT_POSITION(1.226, 0.012, 1.360, 0.024)

	MDRV_TIMER_ADD_SCANLINE("prd_start", draco_prd_start, "main", CDP1869_SCANLINE_PREDISPLAY_START_PAL, CDP1869_TOTAL_SCANLINES_PAL)
	MDRV_TIMER_ADD_SCANLINE("prd_end", draco_prd_end, "main", CDP1869_SCANLINE_PREDISPLAY_END_PAL, CDP1869_TOTAL_SCANLINES_PAL)

	// sound hardware

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(CDP1869, DRACO_CHR2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD(AY8910, DRACO_SND_CHR1)
	MDRV_SOUND_CONFIG(ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END

/* ROMs */

ROM_START( destryer )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "des a 2.ic4", 0x0000, 0x0800, CRC(63749870) SHA1(a8eee4509d7a52dcf33049de221d928da3632174) )
	ROM_LOAD( "des b 2.ic5", 0x0800, 0x0800, CRC(60604f40) SHA1(32ca95c5b38b0f4992e04d77123d217f143ae084) )
	ROM_LOAD( "des c 2.ic6", 0x1000, 0x0800, CRC(a7cdeb7b) SHA1(a5a7748967d4ca89fb09632e1f0130ef050dbd68) )
	ROM_LOAD( "des d 2.ic7", 0x1800, 0x0800, CRC(dbec0aea) SHA1(1d9d49009a45612ee79763781a004499313b823b) )
ROM_END

// this was destroyer2.rom in standalone emu..
ROM_START( destryea )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "destryea_1", 0x0000, 0x0800, CRC(421428e9) SHA1(0ac3a1e7f61125a1cd82145fa28cbc4b93505dc9) )
	ROM_LOAD( "destryea_2", 0x0800, 0x0800, CRC(55dc8145) SHA1(a0066d3f3ac0ae56273485b74af90eeffea5e64e) )
	ROM_LOAD( "destryea_3", 0x1000, 0x0800, CRC(5557bdf8) SHA1(37a9cbc5d25051d3bed7535c58aac937cd7c64e1) )
	ROM_LOAD( "destryea_4", 0x1800, 0x0800, CRC(608b779c) SHA1(8fd6cc376c507680777553090329cc66be42a934) )
ROM_END

ROM_START( altair )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "alt a 1.ic7",  0x0000, 0x0800, CRC(37c26c4e) SHA1(30df7efcf5bd12dafc1cb6e894fc18e7b76d3e61) )
	ROM_LOAD( "alt b 1.ic8",  0x0800, 0x0800, CRC(76b814a4) SHA1(e8ab1d1cbcef974d929ef8edd10008f60052a607) )
	ROM_LOAD( "alt c 1.ic9",  0x1000, 0x0800, CRC(2569ce44) SHA1(a09597d2f8f50fab9a09ed9a59c50a2bdcba47bb) )
	ROM_LOAD( "alt d 1.ic10", 0x1800, 0x0800, CRC(a25e6d11) SHA1(c197ff91bb9bdd04e88908259e4cde11b990e31d) )
	ROM_LOAD( "alt e 1.ic11", 0x2000, 0x0800, CRC(e497f23b) SHA1(6094e9873df7bd88c521ddc3fd63961024687243) )
	ROM_LOAD( "alt f 1.ic12", 0x2800, 0x0800, CRC(a06dd905) SHA1(c24ad9ff6d4e3b4e57fd75f946e8832fa00c2ea0) )
ROM_END

ROM_START( draco )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "dra a 1.ic10", 0x0000, 0x0800, CRC(ca127984) SHA1(46721cf42b1c891f7c88bc063a2149dd3cefea74) )
	ROM_LOAD( "dra b 1.ic11", 0x0800, 0x0800, CRC(e4936e28) SHA1(ddbbf769994d32a6bce75312306468a89033f0aa) )
	ROM_LOAD( "dra c 1.ic12", 0x1000, 0x0800, CRC(94480f5d) SHA1(8f49ce0f086259371e999d097a502482c83c6e9e) )
	ROM_LOAD( "dra d 1.ic13", 0x1800, 0x0800, CRC(32075277) SHA1(2afaa92c91f554e3bdcfec6d94ef82df63032afb) )
	ROM_LOAD( "dra e 1.ic14", 0x2000, 0x0800, CRC(cce7872e) SHA1(c956eb994452bd8a27bbc6d0e6d103e87a4a3e6e) )
	ROM_LOAD( "dra f 1.ic15", 0x2800, 0x0800, CRC(e5927ec7) SHA1(42e0aabb6187bbb189648859fd5dddda43814526) )
	ROM_LOAD( "dra g 1.ic16", 0x3000, 0x0800, CRC(f28546c0) SHA1(daedf1d64f94358b15580d697dd77d3c977aa22c) )
	ROM_LOAD( "dra h 1.ic17", 0x3800, 0x0800, CRC(dce782ea) SHA1(f558096f43fb30337bc4a527169718326c265c2c) )

	ROM_REGION( 0x800, REGION_CPU2, 0 )
	ROM_LOAD( "dra s 1.ic4",  0x0000, 0x0800, CRC(292a57f8) SHA1(b34a189394746d77c3ee669db24109ee945c3be7) )
ROM_END

/* Game Drivers */

GAME( 1980, destryer, 0, 		destryer, destryer, 0, ROT90, "Cidelsa", "Destroyer (Cidelsa) (set 1)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1980, destryea, destryer, destryea, destryer, 0, ROT90, "Cidelsa", "Destroyer (Cidelsa) (set 2)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1981, altair,   0, 		altair,   altair,   0, ROT90, "Cidelsa", "Altair", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1981, draco,    0, 		draco,    draco,    0, ROT90, "Cidelsa", "Draco", GAME_IMPERFECT_COLORS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
