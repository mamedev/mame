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

#include "driver.h"
#include "deprecat.h"
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

static INT8 sound_cpunum;
static INT8 soundalt_cpunum;
static UINT8 williams_pianum;



/***************************************************************************
    FUNCTION PROTOTYPES
****************************************************************************/

static void init_audio_state(running_machine *machine);

static void cvsd_ym2151_irq(running_machine *machine, int state);
static void adpcm_ym2151_irq(running_machine *machine, int state);
static void cvsd_irqa(running_machine *machine, int state);
static void cvsd_irqb(running_machine *machine, int state);

static WRITE8_HANDLER( cvsd_bank_select_w );
static READ8_HANDLER( cvsd_pia_r );
static WRITE8_HANDLER( cvsd_pia_w );
static WRITE8_HANDLER( cvsd_talkback_w );

static READ8_HANDLER( adpcm_command_r );
static WRITE8_HANDLER( adpcm_bank_select_w );
static WRITE8_HANDLER( adpcm_6295_bank_select_w );
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
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x1ffe) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0x2001, 0x2001) AM_MIRROR(0x1ffe) AM_READWRITE(YM2151_status_port_0_r, YM2151_data_port_0_w)
	AM_RANGE(0x4000, 0x4003) AM_MIRROR(0x1ffc) AM_READWRITE(cvsd_pia_r, cvsd_pia_w)
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x07ff) AM_WRITE(hc55516_0_digit_clock_clear_w)
	AM_RANGE(0x6800, 0x6800) AM_MIRROR(0x07ff) AM_WRITE(hc55516_0_clock_set_w)
	AM_RANGE(0x7800, 0x7800) AM_MIRROR(0x07ff) AM_WRITE(cvsd_bank_select_w)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK(5)
ADDRESS_MAP_END


/* NARC master readmem/writemem structures */
static ADDRESS_MAP_START( williams_narc_master_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x03fe) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0x2001, 0x2001) AM_MIRROR(0x03fe) AM_READWRITE(YM2151_status_port_0_r, YM2151_data_port_0_w)
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x03ff) AM_WRITE(narc_master_talkback_w)
	AM_RANGE(0x2c00, 0x2c00) AM_MIRROR(0x03ff) AM_WRITE(narc_command2_w)
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x03ff) AM_WRITE(DAC_0_data_w)
	AM_RANGE(0x3400, 0x3400) AM_MIRROR(0x03ff) AM_READ(narc_command_r)
	AM_RANGE(0x3800, 0x3800) AM_MIRROR(0x03ff) AM_WRITE(narc_master_bank_select_w)
	AM_RANGE(0x3c00, 0x3c00) AM_MIRROR(0x03ff) AM_WRITE(narc_master_sync_w)
	AM_RANGE(0x4000, 0xbfff) AM_ROMBANK(5)
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK(6)
ADDRESS_MAP_END

/* NARC slave readmem/writemem structures */
static ADDRESS_MAP_START( williams_narc_slave_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x03ff) AM_WRITE(hc55516_0_clock_set_w)
	AM_RANGE(0x2400, 0x2400) AM_MIRROR(0x03ff) AM_WRITE(hc55516_0_digit_clock_clear_w)
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x03ff) AM_WRITE(narc_slave_talkback_w)
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x03ff) AM_WRITE(DAC_1_data_w)
	AM_RANGE(0x3400, 0x3400) AM_MIRROR(0x03ff) AM_READ(narc_command2_r)
	AM_RANGE(0x3800, 0x3800) AM_MIRROR(0x03ff) AM_WRITE(narc_slave_bank_select_w)
	AM_RANGE(0x3c00, 0x3c00) AM_MIRROR(0x03ff) AM_WRITE(narc_slave_sync_w)
	AM_RANGE(0x4000, 0xbfff) AM_ROMBANK(7)
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK(8)
ADDRESS_MAP_END


/* ADPCM readmem/writemem structures */
static ADDRESS_MAP_START( williams_adpcm_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x03ff) AM_WRITE(adpcm_bank_select_w)
	AM_RANGE(0x2400, 0x2400) AM_MIRROR(0x03fe) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0x2401, 0x2401) AM_MIRROR(0x03fe) AM_READWRITE(YM2151_status_port_0_r, YM2151_data_port_0_w)
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x03ff) AM_WRITE(DAC_0_data_w)
	AM_RANGE(0x2c00, 0x2c00) AM_MIRROR(0x03ff) AM_READWRITE(OKIM6295_status_0_r, OKIM6295_data_0_w)
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x03ff) AM_READ(adpcm_command_r)
	AM_RANGE(0x3400, 0x3400) AM_MIRROR(0x03ff) AM_WRITE(adpcm_6295_bank_select_w)
	AM_RANGE(0x3c00, 0x3c00) AM_MIRROR(0x03ff) AM_WRITE(adpcm_talkback_w)
	AM_RANGE(0x4000, 0xbfff) AM_ROMBANK(5)
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK(6)
ADDRESS_MAP_END



/* PIA structure */
static const pia6821_interface cvsd_pia_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ DAC_0_data_w, cvsd_talkback_w, 0, 0,
	/*irqs   : A/B             */ cvsd_irqa, cvsd_irqb
};



/***************************************************************************
    AUDIO STRUCTURES
****************************************************************************/

/* YM2151 structure (CVSD variant) */
static const struct YM2151interface cvsd_ym2151_interface =
{
	cvsd_ym2151_irq
};


/* YM2151 structure (ADPCM variant) */
static const struct YM2151interface adpcm_ym2151_interface =
{
	adpcm_ym2151_irq
};



/***************************************************************************
    MACHINE DRIVERS
****************************************************************************/

MACHINE_DRIVER_START( williams_cvsd_sound )
	MDRV_CPU_ADD_TAG("cvsd", M6809E, CVSD_MASTER_CLOCK)
	MDRV_CPU_PROGRAM_MAP(williams_cvsd_map,0)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(YM2151, CVSD_FM_CLOCK)
	MDRV_SOUND_CONFIG(cvsd_ym2151_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD(HC55516, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( williams_narc_sound )
	MDRV_CPU_ADD_TAG("narc1", M6809E, NARC_MASTER_CLOCK)
	MDRV_CPU_PROGRAM_MAP(williams_narc_master_map,0)

	MDRV_CPU_ADD_TAG("narc2", M6809E, NARC_MASTER_CLOCK)
	MDRV_CPU_PROGRAM_MAP(williams_narc_slave_map,0)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YM2151, NARC_FM_CLOCK)
	MDRV_SOUND_CONFIG(adpcm_ym2151_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.10)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.10)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.50)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.50)

	MDRV_SOUND_ADD(HC55516, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.60)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.60)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( williams_adpcm_sound )
	MDRV_CPU_ADD_TAG("adpcm", M6809E, ADPCM_MASTER_CLOCK)
	MDRV_CPU_PROGRAM_MAP(williams_adpcm_map,0)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(YM2151, ADPCM_FM_CLOCK)
	MDRV_SOUND_CONFIG(adpcm_ym2151_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD(OKIM6295, ADPCM_MASTER_CLOCK/8)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END



/***************************************************************************
    INITIALIZATION
****************************************************************************/

void williams_cvsd_init(int pianum)
{
	UINT8 *ROM;
	int bank;

	/* configure the CPU */
	sound_cpunum = mame_find_cpu_index(Machine, "cvsd");
	soundalt_cpunum = -1;

	/* configure the PIA */
	williams_pianum = pianum;
	pia_config(pianum, &cvsd_pia_intf);

	/* configure master CPU banks */
	ROM = memory_region(REGION_CPU1 + sound_cpunum);
	for (bank = 0; bank < 16; bank++)
	{
		/*
            D0/D1 -> selects: 0=U4 1=U19 2=U20 3=n/c
            D2 -> A15
            D3 -> A16
         */
		offs_t offset = 0x8000 * ((bank >> 2) & 3) + 0x20000 * (bank & 3);
		memory_configure_bank(5, bank, 1, &ROM[0x10000 + offset], 0);
	}
	memory_set_bank(5, 0);

	/* reset the IRQ state */
	pia_set_input_ca1(williams_pianum, 1);

	/* register for save states */
	state_save_register_global(williams_sound_int_state);
	state_save_register_global(audio_talkback);
}


void williams_narc_init(void)
{
	UINT8 *ROM;
	int bank;

	/* configure the CPU */
	sound_cpunum = mame_find_cpu_index(Machine, "narc1");
	soundalt_cpunum = mame_find_cpu_index(Machine, "narc2");

	/* configure master CPU banks */
	ROM = memory_region(REGION_CPU1 + sound_cpunum);
	for (bank = 0; bank < 16; bank++)
	{
		/*
            D0 -> A15
            D1/D2 -> selects: 0=n/c 1=U3 2=U4 3=U5
            D3 -> A16
         */
		offs_t offset = 0x8000 * (bank & 1) + 0x10000 * ((bank >> 3) & 1) + 0x20000 * ((bank >> 1) & 3);
		memory_configure_bank(5, bank, 1, &ROM[0x10000 + offset], 0);
	}
	memory_set_bankptr(6, &ROM[0x10000 + 0x4000 + 0x8000 + 0x10000 + 0x20000 * 3]);

	/* configure slave CPU banks */
	ROM = memory_region(REGION_CPU1 + soundalt_cpunum);
	for (bank = 0; bank < 16; bank++)
	{
		/*
            D0 -> A15
            D1/D2 -> selects: 0=U35 1=U36 2=U37 3=U38
            D3 -> A16
        */
		offs_t offset = 0x8000 * (bank & 1) + 0x10000 * ((bank >> 3) & 1) + 0x20000 * ((bank >> 1) & 3);
		memory_configure_bank(7, bank, 1, &ROM[0x10000 + offset], 0);
	}
	memory_set_bankptr(8, &ROM[0x10000 + 0x4000 + 0x8000 + 0x10000 + 0x20000 * 3]);

	/* register for save states */
	state_save_register_global(williams_sound_int_state);
	state_save_register_global(audio_talkback);
	state_save_register_global(audio_sync);
}


void williams_adpcm_init(void)
{
	UINT8 *ROM;

	/* configure the CPU */
	sound_cpunum = mame_find_cpu_index(Machine, "adpcm");
	soundalt_cpunum = -1;

	/* configure banks */
	ROM = memory_region(REGION_CPU1 + sound_cpunum);
	memory_configure_bank(5, 0, 8, &ROM[0x10000], 0x8000);
	memory_set_bankptr(6, &ROM[0x10000 + 0x4000 + 7 * 0x8000]);

	/* expand ADPCM data */
	/* it is assumed that U12 is loaded @ 0x00000 and U13 is loaded @ 0x40000 */
	ROM = memory_region(REGION_SOUND1);
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
	state_save_register_global(williams_sound_int_state);
	state_save_register_global(audio_talkback);
}


static void init_audio_state(running_machine *machine)
{
	/* reset the YM2151 state */
	sndti_reset(SOUND_YM2151, 0);

	/* clear all the interrupts */
	williams_sound_int_state = 0;
	if (sound_cpunum != -1)
	{
		cpunum_set_input_line(machine, sound_cpunum, M6809_FIRQ_LINE, CLEAR_LINE);
		cpunum_set_input_line(machine, sound_cpunum, M6809_IRQ_LINE, CLEAR_LINE);
		cpunum_set_input_line(machine, sound_cpunum, INPUT_LINE_NMI, CLEAR_LINE);
	}
	if (soundalt_cpunum != -1)
	{
		cpunum_set_input_line(machine, soundalt_cpunum, M6809_FIRQ_LINE, CLEAR_LINE);
		cpunum_set_input_line(machine, soundalt_cpunum, M6809_IRQ_LINE, CLEAR_LINE);
		cpunum_set_input_line(machine, soundalt_cpunum, INPUT_LINE_NMI, CLEAR_LINE);
	}
}



/***************************************************************************
    CVSD IRQ GENERATION CALLBACKS
****************************************************************************/

static void cvsd_ym2151_irq(running_machine *machine, int state)
{
	pia_set_input_ca1(williams_pianum, !state);
}


static void cvsd_irqa(running_machine *machine, int state)
{
	cpunum_set_input_line(machine, sound_cpunum, M6809_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}


static void cvsd_irqb(running_machine *machine, int state)
{
	cpunum_set_input_line(machine, sound_cpunum, INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}



/***************************************************************************
    ADPCM IRQ GENERATION CALLBACKS
****************************************************************************/

static void adpcm_ym2151_irq(running_machine *machine, int state)
{
	cpunum_set_input_line(machine, sound_cpunum, M6809_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}



/***************************************************************************
    CVSD READ/WRITE HANDLERS
****************************************************************************/

static WRITE8_HANDLER( cvsd_bank_select_w )
{
	memory_set_bank(5, data & 0x0f);
}


static READ8_HANDLER( cvsd_pia_r )
{
	return pia_read(williams_pianum, offset);
}


static WRITE8_HANDLER( cvsd_pia_w )
{
	pia_write(williams_pianum, offset, data);
}


static WRITE8_HANDLER( cvsd_talkback_w )
{
	audio_talkback = data;
	logerror("CVSD Talkback = %02X\n", data);
}



/***************************************************************************
    CVSD COMMUNICATIONS
****************************************************************************/

static TIMER_CALLBACK( williams_cvsd_delayed_data_w )
{
	pia_set_input_b(williams_pianum, param & 0xff);
	pia_set_input_cb1(williams_pianum, param & 0x100);
	pia_set_input_cb2(williams_pianum, param & 0x200);
}


void williams_cvsd_data_w(int data)
{
	timer_call_after_resynch(NULL, data, williams_cvsd_delayed_data_w);
}


void williams_cvsd_reset_w(int state)
{
	/* going high halts the CPU */
	if (state)
	{
		cvsd_bank_select_w(Machine, 0, 0);
		init_audio_state(Machine);
		cpunum_set_input_line(Machine, sound_cpunum, INPUT_LINE_RESET, ASSERT_LINE);
	}
	/* going low resets and reactivates the CPU */
	else
		cpunum_set_input_line(Machine, sound_cpunum, INPUT_LINE_RESET, CLEAR_LINE);
}



/***************************************************************************
    NARC READ/WRITE HANDLERS
****************************************************************************/

static WRITE8_HANDLER( narc_master_bank_select_w )
{
	memory_set_bank(5, data & 0x0f);
}


static WRITE8_HANDLER( narc_slave_bank_select_w )
{
	memory_set_bank(7, data & 0x0f);
}


static READ8_HANDLER( narc_command_r )
{
	cpunum_set_input_line(machine, sound_cpunum, M6809_IRQ_LINE, CLEAR_LINE);
	williams_sound_int_state = 0;
	return soundlatch_r(machine, 0);
}


static WRITE8_HANDLER( narc_command2_w )
{
	soundlatch2_w(machine, 0, data & 0xff);
	cpunum_set_input_line(machine, soundalt_cpunum, M6809_FIRQ_LINE, ASSERT_LINE);
}


static READ8_HANDLER( narc_command2_r )
{
	cpunum_set_input_line(machine, soundalt_cpunum, M6809_FIRQ_LINE, CLEAR_LINE);
	return soundlatch2_r(machine, 0);
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
	timer_set(double_to_attotime(TIME_OF_74LS123(180000, 0.000001)), NULL, 0x01, narc_sync_clear);
	audio_sync |= 0x01;
	logerror("Master sync = %02X\n", data);
}


static WRITE8_HANDLER( narc_slave_talkback_w )
{
	logerror("Slave Talkback = %02X\n", data);
}


static WRITE8_HANDLER( narc_slave_sync_w )
{
	timer_set(double_to_attotime(TIME_OF_74LS123(180000, 0.000001)), NULL, 0x02, narc_sync_clear);
	audio_sync |= 0x02;
	logerror("Slave sync = %02X\n", data);
}



/***************************************************************************
    NARC COMMUNICATIONS
****************************************************************************/

void williams_narc_data_w(int data)
{
	soundlatch_w(Machine, 0, data & 0xff);
	cpunum_set_input_line(Machine, sound_cpunum, INPUT_LINE_NMI, (data & 0x100) ? CLEAR_LINE : ASSERT_LINE);
	if (!(data & 0x200))
	{
		cpunum_set_input_line(Machine, sound_cpunum, M6809_IRQ_LINE, ASSERT_LINE);
		williams_sound_int_state = 1;
	}
}


void williams_narc_reset_w(int state)
{
	/* going high halts the CPU */
	if (state)
	{
		narc_master_bank_select_w(Machine, 0, 0);
		narc_slave_bank_select_w(Machine, 0, 0);
		init_audio_state(Machine);
		cpunum_set_input_line(Machine, sound_cpunum, INPUT_LINE_RESET, ASSERT_LINE);
		cpunum_set_input_line(Machine, soundalt_cpunum, INPUT_LINE_RESET, ASSERT_LINE);
	}
	/* going low resets and reactivates the CPU */
	else
	{
		cpunum_set_input_line(Machine, sound_cpunum, INPUT_LINE_RESET, CLEAR_LINE);
		cpunum_set_input_line(Machine, soundalt_cpunum, INPUT_LINE_RESET, CLEAR_LINE);
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
	memory_set_bank(5, data & 0x07);
}


static WRITE8_HANDLER( adpcm_6295_bank_select_w )
{
	OKIM6295_set_bank_base(0, (data & 7) * 0x40000);
}


static TIMER_CALLBACK( clear_irq_state )
{
	williams_sound_int_state = 0;
}


static READ8_HANDLER( adpcm_command_r )
{
	cpunum_set_input_line(machine, sound_cpunum, M6809_IRQ_LINE, CLEAR_LINE);

	/* don't clear the external IRQ state for a short while; this allows the
       self-tests to pass */
	timer_set(ATTOTIME_IN_USEC(10), NULL, 0, clear_irq_state);
	return soundlatch_r(machine, 0);
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
	soundlatch_w(Machine, 0, data & 0xff);
	if (!(data & 0x200))
	{
		cpunum_set_input_line(Machine, sound_cpunum, M6809_IRQ_LINE, ASSERT_LINE);
		williams_sound_int_state = 1;
		cpu_boost_interleave(attotime_zero, ATTOTIME_IN_USEC(100));
	}
}


void williams_adpcm_reset_w(int state)
{
	/* going high halts the CPU */
	if (state)
	{
		adpcm_bank_select_w(Machine, 0, 0);
		init_audio_state(Machine);
		cpunum_set_input_line(Machine, sound_cpunum, INPUT_LINE_RESET, ASSERT_LINE);
	}
	/* going low resets and reactivates the CPU */
	else
		cpunum_set_input_line(Machine, sound_cpunum, INPUT_LINE_RESET, CLEAR_LINE);
}


int williams_adpcm_sound_irq_r(void)
{
	return williams_sound_int_state;
}
