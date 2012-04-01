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

typedef struct _williams_audio_state williams_audio_state;
struct _williams_audio_state
{
	UINT8 sound_int_state;

	UINT8 audio_talkback;
	UINT8 audio_sync;

	device_t *sound_cpu;
	device_t *soundalt_cpu;
};

static williams_audio_state audio;

/***************************************************************************
    FUNCTION PROTOTYPES
****************************************************************************/

static void init_audio_state(running_machine &machine);

static void cvsd_ym2151_irq(device_t *device, int state);
static void adpcm_ym2151_irq(device_t *device, int state);
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
static ADDRESS_MAP_START( williams_cvsd_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x1800) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_MIRROR(0x1ffe) AM_DEVREADWRITE_LEGACY("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x4000, 0x4003) AM_MIRROR(0x1ffc) AM_DEVREADWRITE("cvsdpia", pia6821_device, read, write)
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x07ff) AM_DEVWRITE_LEGACY("cvsd", cvsd_digit_clock_clear_w)
	AM_RANGE(0x6800, 0x6800) AM_MIRROR(0x07ff) AM_DEVWRITE_LEGACY("cvsd", cvsd_clock_set_w)
	AM_RANGE(0x7800, 0x7800) AM_MIRROR(0x07ff) AM_WRITE_LEGACY(cvsd_bank_select_w)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank5")
ADDRESS_MAP_END


/* NARC master readmem/writemem structures */
static ADDRESS_MAP_START( williams_narc_master_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_MIRROR(0x03fe) AM_DEVREADWRITE_LEGACY("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x03ff) AM_WRITE_LEGACY(narc_master_talkback_w)
	AM_RANGE(0x2c00, 0x2c00) AM_MIRROR(0x03ff) AM_WRITE_LEGACY(narc_command2_w)
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x03ff) AM_DEVWRITE_LEGACY("dac1", dac_w)
	AM_RANGE(0x3400, 0x3400) AM_MIRROR(0x03ff) AM_READ_LEGACY(narc_command_r)
	AM_RANGE(0x3800, 0x3800) AM_MIRROR(0x03ff) AM_WRITE_LEGACY(narc_master_bank_select_w)
	AM_RANGE(0x3c00, 0x3c00) AM_MIRROR(0x03ff) AM_WRITE_LEGACY(narc_master_sync_w)
	AM_RANGE(0x4000, 0xbfff) AM_ROMBANK("bank5")
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("bank6")
ADDRESS_MAP_END

/* NARC slave readmem/writemem structures */
static ADDRESS_MAP_START( williams_narc_slave_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x03ff) AM_DEVWRITE_LEGACY("cvsd", cvsd_clock_set_w)
	AM_RANGE(0x2400, 0x2400) AM_MIRROR(0x03ff) AM_DEVWRITE_LEGACY("cvsd", cvsd_digit_clock_clear_w)
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x03ff) AM_WRITE_LEGACY(narc_slave_talkback_w)
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x03ff) AM_DEVWRITE_LEGACY("dac2", dac_w)
	AM_RANGE(0x3400, 0x3400) AM_MIRROR(0x03ff) AM_READ_LEGACY(narc_command2_r)
	AM_RANGE(0x3800, 0x3800) AM_MIRROR(0x03ff) AM_WRITE_LEGACY(narc_slave_bank_select_w)
	AM_RANGE(0x3c00, 0x3c00) AM_MIRROR(0x03ff) AM_WRITE_LEGACY(narc_slave_sync_w)
	AM_RANGE(0x4000, 0xbfff) AM_ROMBANK("bank7")
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("bank8")
ADDRESS_MAP_END


/* ADPCM readmem/writemem structures */
static ADDRESS_MAP_START( williams_adpcm_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x03ff) AM_WRITE_LEGACY(adpcm_bank_select_w)
	AM_RANGE(0x2400, 0x2401) AM_MIRROR(0x03fe) AM_DEVREADWRITE_LEGACY("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x03ff) AM_DEVWRITE_LEGACY("dac", dac_w)
	AM_RANGE(0x2c00, 0x2c00) AM_MIRROR(0x03ff) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x03ff) AM_READ_LEGACY(adpcm_command_r)
	AM_RANGE(0x3400, 0x3400) AM_MIRROR(0x03ff) AM_DEVWRITE_LEGACY("oki", adpcm_6295_bank_select_w)
	AM_RANGE(0x3c00, 0x3c00) AM_MIRROR(0x03ff) AM_WRITE_LEGACY(adpcm_talkback_w)
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

MACHINE_CONFIG_FRAGMENT( williams_cvsd_sound )
	MCFG_CPU_ADD("cvsdcpu", M6809E, CVSD_MASTER_CLOCK)
	MCFG_CPU_PROGRAM_MAP(williams_cvsd_map)

	MCFG_PIA6821_ADD("cvsdpia", cvsd_pia_intf)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, CVSD_FM_CLOCK)
	MCFG_SOUND_CONFIG(cvsd_ym2151_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("cvsd", HC55516, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( williams_narc_sound )
	MCFG_CPU_ADD("narc1cpu", M6809E, NARC_MASTER_CLOCK)
	MCFG_CPU_PROGRAM_MAP(williams_narc_master_map)

	MCFG_CPU_ADD("narc2cpu", M6809E, NARC_MASTER_CLOCK)
	MCFG_CPU_PROGRAM_MAP(williams_narc_slave_map)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2151, NARC_FM_CLOCK)
	MCFG_SOUND_CONFIG(adpcm_ym2151_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.10)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.10)

	MCFG_SOUND_ADD("dac1", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MCFG_SOUND_ADD("dac2", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MCFG_SOUND_ADD("cvsd", HC55516, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.60)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.60)
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( williams_adpcm_sound )
	MCFG_CPU_ADD("adpcm", M6809E, ADPCM_MASTER_CLOCK)
	MCFG_CPU_PROGRAM_MAP(williams_adpcm_map)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, ADPCM_FM_CLOCK)
	MCFG_SOUND_CONFIG(adpcm_ym2151_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_OKIM6295_ADD("oki", ADPCM_MASTER_CLOCK/8, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/***************************************************************************
    INITIALIZATION
****************************************************************************/

void williams_cvsd_init(running_machine &machine)
{
	williams_audio_state *state = &audio;
	UINT8 *ROM;
	int bank;

	/* configure the CPU */
	state->sound_cpu = machine.device("cvsdcpu");
	state->soundalt_cpu = NULL;

	/* configure master CPU banks */
	ROM = machine.region("cvsdcpu")->base();
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
	machine.device<pia6821_device>("cvsdpia")->ca1_w(1);

	/* register for save states */
	state_save_register_global(machine, state->sound_int_state);
	state_save_register_global(machine, state->audio_talkback);
}


void williams_narc_init(running_machine &machine)
{
	williams_audio_state *state = &audio;
	UINT8 *ROM;
	int bank;

	/* configure the CPU */
	state->sound_cpu = machine.device("narc1cpu");
	state->soundalt_cpu = machine.device("narc2cpu");

	/* configure master CPU banks */
	ROM = machine.region("narc1cpu")->base();
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
	ROM = machine.region("narc2cpu")->base();
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
	state_save_register_global(machine, state->sound_int_state);
	state_save_register_global(machine, state->audio_talkback);
	state_save_register_global(machine, state->audio_sync);
}


void williams_adpcm_init(running_machine &machine)
{
	williams_audio_state *state = &audio;
	UINT8 *ROM;

	/* configure the CPU */
	state->sound_cpu = machine.device("adpcm");
	state->soundalt_cpu = NULL;

	/* configure banks */
	ROM = machine.region("adpcm")->base();
	memory_configure_bank(machine, "bank5", 0, 8, &ROM[0x10000], 0x8000);
	memory_set_bankptr(machine, "bank6", &ROM[0x10000 + 0x4000 + 7 * 0x8000]);

	/* expand ADPCM data */
	/* it is assumed that U12 is loaded @ 0x00000 and U13 is loaded @ 0x40000 */
	ROM = machine.region("oki")->base();
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
	state_save_register_global(machine, state->sound_int_state);
	state_save_register_global(machine, state->audio_talkback);
}


static void init_audio_state(running_machine &machine)
{
	williams_audio_state *state = &audio;
	device_t *sound_cpu = state->sound_cpu;
	device_t *soundalt_cpu = state->soundalt_cpu;

	/* reset the YM2151 state */
	devtag_reset(machine, "ymsnd");

	/* clear all the interrupts */
	state->sound_int_state = 0;
	if (sound_cpu != NULL)
	{
		device_set_input_line(sound_cpu, M6809_FIRQ_LINE, CLEAR_LINE);
		device_set_input_line(sound_cpu, M6809_IRQ_LINE, CLEAR_LINE);
		device_set_input_line(sound_cpu, INPUT_LINE_NMI, CLEAR_LINE);
	}
	if (soundalt_cpu != NULL)
	{
		device_set_input_line(soundalt_cpu, M6809_FIRQ_LINE, CLEAR_LINE);
		device_set_input_line(soundalt_cpu, M6809_IRQ_LINE, CLEAR_LINE);
		device_set_input_line(soundalt_cpu, INPUT_LINE_NMI, CLEAR_LINE);
	}
}



/***************************************************************************
    CVSD IRQ GENERATION CALLBACKS
****************************************************************************/

static void cvsd_ym2151_irq(device_t *device, int state)
{
	device->machine().device<pia6821_device>("cvsdpia")->ca1_w(!state);
}


static WRITE_LINE_DEVICE_HANDLER( cvsd_irqa )
{
	williams_audio_state *sndstate = &audio;

	device_set_input_line(sndstate->sound_cpu, M6809_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}


static WRITE_LINE_DEVICE_HANDLER( cvsd_irqb )
{
	williams_audio_state *sndstate = &audio;

	device_set_input_line(sndstate->sound_cpu, INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}



/***************************************************************************
    ADPCM IRQ GENERATION CALLBACKS
****************************************************************************/

static void adpcm_ym2151_irq(device_t *device, int state)
{
	williams_audio_state *sndstate = &audio;

	device_set_input_line(sndstate->sound_cpu, M6809_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}



/***************************************************************************
    CVSD READ/WRITE HANDLERS
****************************************************************************/

static WRITE8_HANDLER( cvsd_bank_select_w )
{
	memory_set_bank(space->machine(), "bank5", data & 0x0f);
}


static WRITE8_DEVICE_HANDLER( cvsd_talkback_w )
{
	williams_audio_state *state = &audio;

	state->audio_talkback = data;
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
	pia6821_device *pia = machine.device<pia6821_device>("cvsdpia");
	pia->portb_w(param & 0xff);
	pia->cb1_w((param >> 8) & 1);
	pia->cb2_w((param >> 9) & 1);
}


void williams_cvsd_data_w(running_machine &machine, int data)
{
	machine.scheduler().synchronize(FUNC(williams_cvsd_delayed_data_w), data);
}


void williams_cvsd_reset_w(running_machine &machine, int state)
{
	williams_audio_state *sndstate = &audio;
	address_space *space = sndstate->sound_cpu->memory().space(AS_PROGRAM);

	/* going high halts the CPU */
	if (state)
	{
		cvsd_bank_select_w(space, 0, 0);
		init_audio_state(space->machine());
		device_set_input_line(&space->device(), INPUT_LINE_RESET, ASSERT_LINE);
	}
	/* going low resets and reactivates the CPU */
	else
		device_set_input_line(&space->device(), INPUT_LINE_RESET, CLEAR_LINE);
}



/***************************************************************************
    NARC READ/WRITE HANDLERS
****************************************************************************/

static WRITE8_HANDLER( narc_master_bank_select_w )
{
	memory_set_bank(space->machine(), "bank5", data & 0x0f);
}


static WRITE8_HANDLER( narc_slave_bank_select_w )
{
	memory_set_bank(space->machine(), "bank7", data & 0x0f);
}


static READ8_HANDLER( narc_command_r )
{
	williams_audio_state *state = &audio;

	device_set_input_line(state->sound_cpu, M6809_IRQ_LINE, CLEAR_LINE);
	state->sound_int_state = 0;
	return soundlatch_r(space, 0);
}


static WRITE8_HANDLER( narc_command2_w )
{
	williams_audio_state *state = &audio;

	soundlatch2_w(space, 0, data & 0xff);
	device_set_input_line(state->soundalt_cpu, M6809_FIRQ_LINE, ASSERT_LINE);
}


static READ8_HANDLER( narc_command2_r )
{
	williams_audio_state *state = &audio;

	device_set_input_line(state->soundalt_cpu, M6809_FIRQ_LINE, CLEAR_LINE);
	return soundlatch2_r(space, 0);
}


static WRITE8_HANDLER( narc_master_talkback_w )
{
	williams_audio_state *state = &audio;

	state->audio_talkback = data;
	logerror("Master Talkback = %02X\n", data);
}


static TIMER_CALLBACK( narc_sync_clear )
{
	williams_audio_state *state = &audio;

	state->audio_sync &= ~param;
}

static WRITE8_HANDLER( narc_master_sync_w )
{
	williams_audio_state *state = &audio;

	space->machine().scheduler().timer_set(attotime::from_double(TIME_OF_74LS123(180000, 0.000001)), FUNC(narc_sync_clear), 0x01);
	state->audio_sync |= 0x01;
	logerror("Master sync = %02X\n", data);
}


static WRITE8_HANDLER( narc_slave_talkback_w )
{
	logerror("Slave Talkback = %02X\n", data);
}


static WRITE8_HANDLER( narc_slave_sync_w )
{
	williams_audio_state *state = &audio;

	space->machine().scheduler().timer_set(attotime::from_double(TIME_OF_74LS123(180000, 0.000001)), FUNC(narc_sync_clear), 0x02);
	state->audio_sync |= 0x02;
	logerror("Slave sync = %02X\n", data);
}



/***************************************************************************
    NARC COMMUNICATIONS
****************************************************************************/

void williams_narc_data_w(running_machine &machine, int data)
{
	williams_audio_state *state = &audio;
	device_t *sound_cpu = state->sound_cpu;
	address_space *space = sound_cpu->memory().space(AS_PROGRAM);

	soundlatch_w(space, 0, data & 0xff);
	device_set_input_line(sound_cpu, INPUT_LINE_NMI, (data & 0x100) ? CLEAR_LINE : ASSERT_LINE);
	if (!(data & 0x200))
	{
		device_set_input_line(sound_cpu, M6809_IRQ_LINE, ASSERT_LINE);
		state->sound_int_state = 1;
	}
}


void williams_narc_reset_w(running_machine &machine, int state)
{
	williams_audio_state *sndstate = &audio;
	device_t *sound_cpu = sndstate->sound_cpu;
	device_t *soundalt_cpu = sndstate->soundalt_cpu;

	/* going high halts the CPU */
	if (state)
	{
		address_space *space = sound_cpu->memory().space(AS_PROGRAM);
		narc_master_bank_select_w(space, 0, 0);
		narc_slave_bank_select_w(space, 0, 0);
		init_audio_state(space->machine());
		device_set_input_line(sound_cpu, INPUT_LINE_RESET, ASSERT_LINE);
		device_set_input_line(soundalt_cpu, INPUT_LINE_RESET, ASSERT_LINE);
	}
	/* going low resets and reactivates the CPU */
	else
	{
		device_set_input_line(sound_cpu, INPUT_LINE_RESET, CLEAR_LINE);
		device_set_input_line(soundalt_cpu, INPUT_LINE_RESET, CLEAR_LINE);
	}
}


int williams_narc_talkback_r(running_machine &machine)
{
	williams_audio_state *state = &audio;

	return state->audio_talkback | (state->audio_sync << 8);
}



/***************************************************************************
    ADPCM READ/WRITE HANDLERS
****************************************************************************/

static WRITE8_HANDLER( adpcm_bank_select_w )
{
	memory_set_bank(space->machine(), "bank5", data & 0x07);
}


static WRITE8_DEVICE_HANDLER( adpcm_6295_bank_select_w )
{
	downcast<okim6295_device *>(device)->set_bank_base((data & 7) * 0x40000);
}


static TIMER_CALLBACK( clear_irq_state )
{
	williams_audio_state *state = &audio;

	state->sound_int_state = 0;
}


static READ8_HANDLER( adpcm_command_r )
{
	williams_audio_state *state = &audio;
	device_t *sound_cpu = state->sound_cpu;
	device_set_input_line(sound_cpu, M6809_IRQ_LINE, CLEAR_LINE);

	/* don't clear the external IRQ state for a short while; this allows the
       self-tests to pass */
	space->machine().scheduler().timer_set(attotime::from_usec(10), FUNC(clear_irq_state));
	return soundlatch_r(space, 0);
}


static WRITE8_HANDLER( adpcm_talkback_w )
{
	williams_audio_state *state = &audio;
	state->audio_talkback = data;
	logerror("ADPCM Talkback = %02X\n", data);
}



/***************************************************************************
    ADPCM COMMUNICATIONS
****************************************************************************/

void williams_adpcm_data_w(running_machine &machine, int data)
{
	williams_audio_state *state = &audio;
	device_t *sound_cpu = state->sound_cpu;
	address_space *space = sound_cpu->memory().space(AS_PROGRAM);
	soundlatch_w(space, 0, data & 0xff);
	if (!(data & 0x200))
	{
		device_set_input_line(sound_cpu, M6809_IRQ_LINE, ASSERT_LINE);
		state->sound_int_state = 1;
		space->machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
	}
}


void williams_adpcm_reset_w(running_machine &machine, int state)
{
	williams_audio_state *sndstate = &audio;
	device_t *sound_cpu = sndstate->sound_cpu;

	/* going high halts the CPU */
	if (state)
	{
		address_space *space = sound_cpu->memory().space(AS_PROGRAM);
		adpcm_bank_select_w(space, 0, 0);
		init_audio_state(space->machine());
		device_set_input_line(sound_cpu, INPUT_LINE_RESET, ASSERT_LINE);
	}
	/* going low resets and reactivates the CPU */
	else
		device_set_input_line(sound_cpu, INPUT_LINE_RESET, CLEAR_LINE);
}


int williams_adpcm_sound_irq_r(running_machine &machine)
{
	williams_audio_state *state = &audio;
	return state->sound_int_state;
}
