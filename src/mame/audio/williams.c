/***************************************************************************

    Midway/Williams Audio Boards
    ----------------------------

    6809 MEMORY MAP

    Function                                  Address     R/W  Data
    ---------------------------------------------------------------
    Program RAM                               0000-07FF   R/W  D0-D7

    Music (YM-2151)                           2000-2001   R/W  D0-D7

    6821 PIA                                  4000-4003   R/W  D0-D7

    HC55516 clock low, digit latch            6000        W    D0
    HC55516 clock high                        6800        W    xx

    Bank select                               7800        W    D0-D2

    Banked Program ROM                        8000-FFFF   R    D0-D7

****************************************************************************/

#include "emu.h"
#include "machine/6821pia.h"
#include "cpu/m6809/m6809.h"
#include "williams.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "sound/hc55516.h"
#include "sound/dac.h"


#define NARC_MASTER_CLOCK		XTAL_8MHz
#define NARC_FM_CLOCK			XTAL_3_579545MHz

#define CVSD_MASTER_CLOCK		XTAL_8MHz
#define CVSD_FM_CLOCK			XTAL_3_579545MHz

#define ADPCM_MASTER_CLOCK		XTAL_8MHz
#define ADPCM_FM_CLOCK			XTAL_3_579545MHz



/***************************************************************************
    STATIC GLOBALS
****************************************************************************/

static UINT8 williams_sound_int_state;

static UINT8 audio_talkback;
static UINT8 audio_sync;

static running_device *sound_cpu;
static running_device *soundalt_cpu;



/***************************************************************************
    FUNCTION PROTOTYPES
****************************************************************************/

static void init_audio_state(running_machine *machine);

static void cvsd_ym2151_irq(running_device *device, int state);
static void adpcm_ym2151_irq(running_device *device, int state);
static WRITE_LINE_DEVICE_HANDLER( cvsd_irqa );
static WRITE_LINE_DEVICE_HANDLER( cvsd_irqb );

static WRITE8_HANDLER( cvsd_bank_select_w );
static WRITE8_DEVICE_HANDLER( cvsd_talkback_w );
static WRITE8_DEVICE_HANDLER( cvsd_digit_clock_clear_w );
static WRITE8_DEVICE_HANDLER( cvsd_clock_set_w );

static READ8_HANDLER( adpcm_command_r );
static WRITE8_HANDLER( adpcm_bank_select_w );
static WRITE8_DEVICE_HANDLER( adpcm_6295_bank_select_w );
static WRITE8_HANDLER( adpcm_talkback_w );

static READ8_HANDLER( narc_command_r );
static READ8_HANDLER( narc_command2_r );
static WRITE8_HANDLER( narc_command2_w );
static WRITE8_HANDLER( narc_master_bank_select_w );
static WRITE8_HANDLER( narc_slave_bank_select_w );
static WRITE8_HANDLER( narc_master_talkback_w );
static WRITE8_HANDLER( narc_master_sync_w );
static WRITE8_HANDLER( narc_slave_talkback_w );
static WRITE8_HANDLER( narc_slave_sync_w );



/***************************************************************************
    PROCESSOR STRUCTURES
****************************************************************************/

/* CVSD readmem/writemem structures */
static ADDRESS_MAP_START( williams_cvsd_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x1800) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_MIRROR(0x1ffe) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x4000, 0x4003) AM_MIRROR(0x1ffc) AM_DEVREADWRITE("cvsdpia", pia6821_r, pia6821_w)
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x07ff) AM_DEVWRITE("cvsd", cvsd_digit_clock_clear_w)
	AM_RANGE(0x6800, 0x6800) AM_MIRROR(0x07ff) AM_DEVWRITE("cvsd", cvsd_clock_set_w)
	AM_RANGE(0x7800, 0x7800) AM_MIRROR(0x07ff) AM_WRITE(cvsd_bank_select_w)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank5")
ADDRESS_MAP_END


/* NARC master readmem/writemem structures */
static ADDRESS_MAP_START( williams_narc_master_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_MIRROR(0x03fe) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x03ff) AM_WRITE(narc_master_talkback_w)
	AM_RANGE(0x2c00, 0x2c00) AM_MIRROR(0x03ff) AM_WRITE(narc_command2_w)
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x03ff) AM_DEVWRITE("dac1", dac_w)
	AM_RANGE(0x3400, 0x3400) AM_MIRROR(0x03ff) AM_READ(narc_command_r)
	AM_RANGE(0x3800, 0x3800) AM_MIRROR(0x03ff) AM_WRITE(narc_master_bank_select_w)
	AM_RANGE(0x3c00, 0x3c00) AM_MIRROR(0x03ff) AM_WRITE(narc_master_sync_w)
	AM_RANGE(0x4000, 0xbfff) AM_ROMBANK("bank5")
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("bank6")
ADDRESS_MAP_END

/* NARC slave readmem/writemem structures */
static ADDRESS_MAP_START( williams_narc_slave_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x03ff) AM_DEVWRITE("cvsd", cvsd_clock_set_w)
	AM_RANGE(0x2400, 0x2400) AM_MIRROR(0x03ff) AM_DEVWRITE("cvsd", cvsd_digit_clock_clear_w)
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x03ff) AM_WRITE(narc_slave_talkback_w)
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x03ff) AM_DEVWRITE("dac2", dac_w)
	AM_RANGE(0x3400, 0x3400) AM_MIRROR(0x03ff) AM_READ(narc_command2_r)
	AM_RANGE(0x3800, 0x3800) AM_MIRROR(0x03ff) AM_WRITE(narc_slave_bank_select_w)
	AM_RANGE(0x3c00, 0x3c00) AM_MIRROR(0x03ff) AM_WRITE(narc_slave_sync_w)
	AM_RANGE(0x4000, 0xbfff) AM_ROMBANK("bank7")
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("bank8")
ADDRESS_MAP_END


/* ADPCM readmem/writemem structures */
static ADDRESS_MAP_START( williams_adpcm_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x03ff) AM_WRITE(adpcm_bank_select_w)
	AM_RANGE(0x2400, 0x2401) AM_MIRROR(0x03fe) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x03ff) AM_DEVWRITE("dac", dac_w)
	AM_RANGE(0x2c00, 0x2c00) AM_MIRROR(0x03ff) AM_DEVREADWRITE("oki", okim6295_r, okim6295_w)
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x03ff) AM_READ(adpcm_command_r)
	AM_RANGE(0x3400, 0x3400) AM_MIRROR(0x03ff) AM_DEVWRITE("oki", adpcm_6295_bank_select_w)
	AM_RANGE(0x3c00, 0x3c00) AM_MIRROR(0x03ff) AM_WRITE(adpcm_talkback_w)
	AM_RANGE(0x4000, 0xbfff) AM_ROMBANK("bank5")
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("bank6")
ADDRESS_MAP_END



/* PIA structure */
static const pia6821_interface cvsd_pia_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DEVICE_HANDLER("dac", dac_w),		/* port A out */
	DEVCB_HANDLER(cvsd_talkback_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_LINE(cvsd_irqa),		/* IRQA */
	DEVCB_LINE(cvsd_irqb)		/* IRQB */
};



/***************************************************************************
    AUDIO STRUCTURES
****************************************************************************/

/* YM2151 structure (CVSD variant) */
static const ym2151_interface cvsd_ym2151_interface =
{
	cvsd_ym2151_irq
};


/* YM2151 structure (ADPCM variant) */
static const ym2151_interface adpcm_ym2151_interface =
{
	adpcm_ym2151_irq
};



/***************************************************************************
    MACHINE DRIVERS
****************************************************************************/

MACHINE_DRIVER_START( williams_cvsd_sound )
	MDRV_CPU_ADD("cvsdcpu", M6809E, CVSD_MASTER_CLOCK)
	MDRV_CPU_PROGRAM_MAP(williams_cvsd_map)

	MDRV_PIA6821_ADD("cvsdpia", cvsd_pia_intf)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2151, CVSD_FM_CLOCK)
	MDRV_SOUND_CONFIG(cvsd_ym2151_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("cvsd", HC55516, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( williams_narc_sound )
	MDRV_CPU_ADD("narc1cpu", M6809E, NARC_MASTER_CLOCK)
	MDRV_CPU_PROGRAM_MAP(williams_narc_master_map)

	MDRV_CPU_ADD("narc2cpu", M6809E, NARC_MASTER_CLOCK)
	MDRV_CPU_PROGRAM_MAP(williams_narc_slave_map)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, NARC_FM_CLOCK)
	MDRV_SOUND_CONFIG(adpcm_ym2151_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.10)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.10)

	MDRV_SOUND_ADD("dac1", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MDRV_SOUND_ADD("dac2", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MDRV_SOUND_ADD("cvsd", HC55516, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.60)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.60)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( williams_adpcm_sound )
	MDRV_CPU_ADD("adpcm", M6809E, ADPCM_MASTER_CLOCK)
	MDRV_CPU_PROGRAM_MAP(williams_adpcm_map)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2151, ADPCM_FM_CLOCK)
	MDRV_SOUND_CONFIG(adpcm_ym2151_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("oki", OKIM6295, ADPCM_MASTER_CLOCK/8)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END



/***************************************************************************
    INITIALIZATION
****************************************************************************/

void williams_cvsd_init(running_machine *machine)
{
	UINT8 *ROM;
	int bank;

	/* configure the CPU */
	sound_cpu = devtag_get_device(machine, "cvsdcpu");
	soundalt_cpu = NULL;

	/* configure master CPU banks */
	ROM = memory_region(machine, "cvsdcpu");
	for (bank = 0; bank < 16; bank++)
	{
		/*
            D0/D1 -> selects: 0=U4 1=U19 2=U20 3=n/c
            D2 -> A15
            D3 -> A16
         */
		offs_t offset = 0x8000 * ((bank >> 2) & 3) + 0x20000 * (bank & 3);
		memory_configure_bank(machine, "bank5", bank, 1, &ROM[0x10000 + offset], 0);
	}
	memory_set_bank(machine, "bank5", 0);

	/* reset the IRQ state */
	pia6821_ca1_w(devtag_get_device(machine, "cvsdpia"), 0, 1);

	/* register for save states */
	state_save_register_global(machine, williams_sound_int_state);
	state_save_register_global(machine, audio_talkback);
}


void williams_narc_init(running_machine *machine)
{
	UINT8 *ROM;
	int bank;

	/* configure the CPU */
	sound_cpu = devtag_get_device(machine, "narc1cpu");
	soundalt_cpu = devtag_get_device(machine, "narc2cpu");

	/* configure master CPU banks */
	ROM = memory_region(machine, "narc1cpu");
	for (bank = 0; bank < 16; bank++)
	{
		/*
            D0 -> A15
            D1/D2 -> selects: 0=n/c 1=U3 2=U4 3=U5
            D3 -> A16
         */
		offs_t offset = 0x8000 * (bank & 1) + 0x10000 * ((bank >> 3) & 1) + 0x20000 * ((bank >> 1) & 3);
		memory_configure_bank(machine, "bank5", bank, 1, &ROM[0x10000 + offset], 0);
	}
	memory_set_bankptr(machine, "bank6", &ROM[0x10000 + 0x4000 + 0x8000 + 0x10000 + 0x20000 * 3]);

	/* configure slave CPU banks */
	ROM = memory_region(machine, "narc2cpu");
	for (bank = 0; bank < 16; bank++)
	{
		/*
            D0 -> A15
            D1/D2 -> selects: 0=U35 1=U36 2=U37 3=U38
            D3 -> A16
        */
		offs_t offset = 0x8000 * (bank & 1) + 0x10000 * ((bank >> 3) & 1) + 0x20000 * ((bank >> 1) & 3);
		memory_configure_bank(machine, "bank7", bank, 1, &ROM[0x10000 + offset], 0);
	}
	memory_set_bankptr(machine, "bank8", &ROM[0x10000 + 0x4000 + 0x8000 + 0x10000 + 0x20000 * 3]);

	/* register for save states */
	state_save_register_global(machine, williams_sound_int_state);
	state_save_register_global(machine, audio_talkback);
	state_save_register_global(machine, audio_sync);
}


void williams_adpcm_init(running_machine *machine)
{
	UINT8 *ROM;

	/* configure the CPU */
	sound_cpu = devtag_get_device(machine, "adpcm");
	soundalt_cpu = NULL;

	/* configure banks */
	ROM = memory_region(machine, "adpcm");
	memory_configure_bank(machine, "bank5", 0, 8, &ROM[0x10000], 0x8000);
	memory_set_bankptr(machine, "bank6", &ROM[0x10000 + 0x4000 + 7 * 0x8000]);

	/* expand ADPCM data */
	/* it is assumed that U12 is loaded @ 0x00000 and U13 is loaded @ 0x40000 */
	ROM = memory_region(machine, "oki");
	memcpy(ROM + 0x1c0000, ROM + 0x080000, 0x20000);	/* expand individual banks */
	memcpy(ROM + 0x180000, ROM + 0x0a0000, 0x20000);
	memcpy(ROM + 0x140000, ROM + 0x0c0000, 0x20000);
	memcpy(ROM + 0x100000, ROM + 0x0e0000, 0x20000);
	memcpy(ROM + 0x0c0000, ROM + 0x000000, 0x20000);
	memcpy(ROM + 0x000000, ROM + 0x040000, 0x20000);
	memcpy(ROM + 0x080000, ROM + 0x020000, 0x20000);

	memcpy(ROM + 0x1e0000, ROM + 0x060000, 0x20000);	/* copy common bank */
	memcpy(ROM + 0x1a0000, ROM + 0x060000, 0x20000);
	memcpy(ROM + 0x160000, ROM + 0x060000, 0x20000);
	memcpy(ROM + 0x120000, ROM + 0x060000, 0x20000);
	memcpy(ROM + 0x0e0000, ROM + 0x060000, 0x20000);
	memcpy(ROM + 0x0a0000, ROM + 0x060000, 0x20000);
	memcpy(ROM + 0x020000, ROM + 0x060000, 0x20000);

	/* register for save states */
	state_save_register_global(machine, williams_sound_int_state);
	state_save_register_global(machine, audio_talkback);
}


static void init_audio_state(running_machine *machine)
{
	/* reset the YM2151 state */
	devtag_reset(machine, "ymsnd");

	/* clear all the interrupts */
	williams_sound_int_state = 0;
	if (sound_cpu != NULL)
	{
		cpu_set_input_line(sound_cpu, M6809_FIRQ_LINE, CLEAR_LINE);
		cpu_set_input_line(sound_cpu, M6809_IRQ_LINE, CLEAR_LINE);
		cpu_set_input_line(sound_cpu, INPUT_LINE_NMI, CLEAR_LINE);
	}
	if (soundalt_cpu != NULL)
	{
		cpu_set_input_line(soundalt_cpu, M6809_FIRQ_LINE, CLEAR_LINE);
		cpu_set_input_line(soundalt_cpu, M6809_IRQ_LINE, CLEAR_LINE);
		cpu_set_input_line(soundalt_cpu, INPUT_LINE_NMI, CLEAR_LINE);
	}
}



/***************************************************************************
    CVSD IRQ GENERATION CALLBACKS
****************************************************************************/

static void cvsd_ym2151_irq(running_device *device, int state)
{
	pia6821_ca1_w(devtag_get_device(device->machine, "cvsdpia"), 0, !state);
}


static WRITE_LINE_DEVICE_HANDLER( cvsd_irqa )
{
	cpu_set_input_line(sound_cpu, M6809_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}


static WRITE_LINE_DEVICE_HANDLER( cvsd_irqb )
{
	cpu_set_input_line(sound_cpu, INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}



/***************************************************************************
    ADPCM IRQ GENERATION CALLBACKS
****************************************************************************/

static void adpcm_ym2151_irq(running_device *device, int state)
{
	cpu_set_input_line(sound_cpu, M6809_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}



/***************************************************************************
    CVSD READ/WRITE HANDLERS
****************************************************************************/

static WRITE8_HANDLER( cvsd_bank_select_w )
{
	memory_set_bank(space->machine, "bank5", data & 0x0f);
}


static WRITE8_DEVICE_HANDLER( cvsd_talkback_w )
{
	audio_talkback = data;
	logerror("CVSD Talkback = %02X\n", data);
}


static WRITE8_DEVICE_HANDLER( cvsd_digit_clock_clear_w )
{
	hc55516_digit_w(device, data);
	hc55516_clock_w(device, 0);
}


static WRITE8_DEVICE_HANDLER( cvsd_clock_set_w )
{
	hc55516_clock_w(device, 1);
}


/***************************************************************************
    CVSD COMMUNICATIONS
****************************************************************************/

static TIMER_CALLBACK( williams_cvsd_delayed_data_w )
{
	running_device *pia = devtag_get_device(machine, "cvsdpia");
	pia6821_portb_w(pia, 0, param & 0xff);
	pia6821_cb1_w(pia, 0, (param >> 8) & 1);
	pia6821_cb2_w(pia, 0, (param >> 9) & 1);
}


void williams_cvsd_data_w(running_machine *machine, int data)
{
	timer_call_after_resynch(machine, NULL, data, williams_cvsd_delayed_data_w);
}


void williams_cvsd_reset_w(int state)
{
	const address_space *space = cpu_get_address_space(sound_cpu, ADDRESS_SPACE_PROGRAM);

	/* going high halts the CPU */
	if (state)
	{
		cvsd_bank_select_w(space, 0, 0);
		init_audio_state(space->machine);
		cpu_set_input_line(space->cpu, INPUT_LINE_RESET, ASSERT_LINE);
	}
	/* going low resets and reactivates the CPU */
	else
		cpu_set_input_line(space->cpu, INPUT_LINE_RESET, CLEAR_LINE);
}



/***************************************************************************
    NARC READ/WRITE HANDLERS
****************************************************************************/

static WRITE8_HANDLER( narc_master_bank_select_w )
{
	memory_set_bank(space->machine, "bank5", data & 0x0f);
}


static WRITE8_HANDLER( narc_slave_bank_select_w )
{
	memory_set_bank(space->machine, "bank7", data & 0x0f);
}


static READ8_HANDLER( narc_command_r )
{
	cpu_set_input_line(sound_cpu, M6809_IRQ_LINE, CLEAR_LINE);
	williams_sound_int_state = 0;
	return soundlatch_r(space, 0);
}


static WRITE8_HANDLER( narc_command2_w )
{
	soundlatch2_w(space, 0, data & 0xff);
	cpu_set_input_line(soundalt_cpu, M6809_FIRQ_LINE, ASSERT_LINE);
}


static READ8_HANDLER( narc_command2_r )
{
	cpu_set_input_line(soundalt_cpu, M6809_FIRQ_LINE, CLEAR_LINE);
	return soundlatch2_r(space, 0);
}


static WRITE8_HANDLER( narc_master_talkback_w )
{
	audio_talkback = data;
	logerror("Master Talkback = %02X\n", data);
}


static TIMER_CALLBACK( narc_sync_clear )
{
	audio_sync &= ~param;
}

static WRITE8_HANDLER( narc_master_sync_w )
{
	timer_set(space->machine, double_to_attotime(TIME_OF_74LS123(180000, 0.000001)), NULL, 0x01, narc_sync_clear);
	audio_sync |= 0x01;
	logerror("Master sync = %02X\n", data);
}


static WRITE8_HANDLER( narc_slave_talkback_w )
{
	logerror("Slave Talkback = %02X\n", data);
}


static WRITE8_HANDLER( narc_slave_sync_w )
{
	timer_set(space->machine, double_to_attotime(TIME_OF_74LS123(180000, 0.000001)), NULL, 0x02, narc_sync_clear);
	audio_sync |= 0x02;
	logerror("Slave sync = %02X\n", data);
}



/***************************************************************************
    NARC COMMUNICATIONS
****************************************************************************/

void williams_narc_data_w(int data)
{
	const address_space *space = cpu_get_address_space(sound_cpu, ADDRESS_SPACE_PROGRAM);

	soundlatch_w(space, 0, data & 0xff);
	cpu_set_input_line(sound_cpu, INPUT_LINE_NMI, (data & 0x100) ? CLEAR_LINE : ASSERT_LINE);
	if (!(data & 0x200))
	{
		cpu_set_input_line(sound_cpu, M6809_IRQ_LINE, ASSERT_LINE);
		williams_sound_int_state = 1;
	}
}


void williams_narc_reset_w(int state)
{
	/* going high halts the CPU */
	if (state)
	{
		const address_space *space = cpu_get_address_space(sound_cpu, ADDRESS_SPACE_PROGRAM);
		narc_master_bank_select_w(space, 0, 0);
		narc_slave_bank_select_w(space, 0, 0);
		init_audio_state(space->machine);
		cpu_set_input_line(sound_cpu, INPUT_LINE_RESET, ASSERT_LINE);
		cpu_set_input_line(soundalt_cpu, INPUT_LINE_RESET, ASSERT_LINE);
	}
	/* going low resets and reactivates the CPU */
	else
	{
		cpu_set_input_line(sound_cpu, INPUT_LINE_RESET, CLEAR_LINE);
		cpu_set_input_line(soundalt_cpu, INPUT_LINE_RESET, CLEAR_LINE);
	}
}


int williams_narc_talkback_r(void)
{
	return audio_talkback | (audio_sync << 8);
}



/***************************************************************************
    ADPCM READ/WRITE HANDLERS
****************************************************************************/

static WRITE8_HANDLER( adpcm_bank_select_w )
{
	memory_set_bank(space->machine, "bank5", data & 0x07);
}


static WRITE8_DEVICE_HANDLER( adpcm_6295_bank_select_w )
{
	okim6295_set_bank_base(device, (data & 7) * 0x40000);
}


static TIMER_CALLBACK( clear_irq_state )
{
	williams_sound_int_state = 0;
}


static READ8_HANDLER( adpcm_command_r )
{
	cpu_set_input_line(sound_cpu, M6809_IRQ_LINE, CLEAR_LINE);

	/* don't clear the external IRQ state for a short while; this allows the
       self-tests to pass */
	timer_set(space->machine, ATTOTIME_IN_USEC(10), NULL, 0, clear_irq_state);
	return soundlatch_r(space, 0);
}


static WRITE8_HANDLER( adpcm_talkback_w )
{
	audio_talkback = data;
	logerror("ADPCM Talkback = %02X\n", data);
}



/***************************************************************************
    ADPCM COMMUNICATIONS
****************************************************************************/

void williams_adpcm_data_w(int data)
{
	const address_space *space = cpu_get_address_space(sound_cpu, ADDRESS_SPACE_PROGRAM);
	soundlatch_w(space, 0, data & 0xff);
	if (!(data & 0x200))
	{
		cpu_set_input_line(sound_cpu, M6809_IRQ_LINE, ASSERT_LINE);
		williams_sound_int_state = 1;
		cpuexec_boost_interleave(space->machine, attotime_zero, ATTOTIME_IN_USEC(100));
	}
}


void williams_adpcm_reset_w(int state)
{
	/* going high halts the CPU */
	if (state)
	{
		const address_space *space = cpu_get_address_space(sound_cpu, ADDRESS_SPACE_PROGRAM);
		adpcm_bank_select_w(space, 0, 0);
		init_audio_state(space->machine);
		cpu_set_input_line(sound_cpu, INPUT_LINE_RESET, ASSERT_LINE);
	}
	/* going low resets and reactivates the CPU */
	else
		cpu_set_input_line(sound_cpu, INPUT_LINE_RESET, CLEAR_LINE);
}


int williams_adpcm_sound_irq_r(void)
{
	return williams_sound_int_state;
}
