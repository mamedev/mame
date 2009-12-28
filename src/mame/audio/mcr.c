/***************************************************************************

    audio/mcr.c

    Functions to emulate general the various MCR sound cards.

***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "sound/tms5220.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "audio/williams.h"
#include "includes/mcr.h"
#include "audio/mcr.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define SSIO_CLOCK			XTAL_16MHz
#define CSDELUXE_CLOCK		XTAL_16MHz
#define SOUNDSGOOD_CLOCK	XTAL_16MHz
#define TURBOCS_CLOCK		XTAL_8MHz
#define SQUAWKTALK_CLOCK	XTAL_3_579545MHz



/*************************************
 *
 *  Global variables
 *
 *************************************/

static UINT8 mcr_sound_config;



/*************************************
 *
 *  Statics
 *
 *************************************/

static UINT16 dacval;

/* SSIO-specific globals */
static const device_config *ssio_sound_cpu;
static UINT8 ssio_data[4];
static UINT8 ssio_status;
static UINT8 ssio_14024_count;
static UINT8 ssio_mute;
static UINT8 ssio_overall[2];
static UINT8 ssio_duty_cycle[2][3];
static UINT8 ssio_ayvolume_lookup[16];
static UINT8 ssio_custom_input_mask[5];
static UINT8 ssio_custom_output_mask[2];
static read8_space_func ssio_custom_input[5];
static write8_space_func ssio_custom_output[2];

/* Chip Squeak Deluxe-specific globals */
static const device_config *csdeluxe_sound_cpu;
static UINT8 csdeluxe_status;

/* Turbo Chip Squeak-specific globals */
static const device_config *turbocs_sound_cpu;
static UINT8 turbocs_status;

/* Sounds Good-specific globals */
static const device_config *soundsgood_sound_cpu;
static UINT8 soundsgood_status;

/* Squawk n' Talk-specific globals */
static const device_config *squawkntalk_sound_cpu;
static UINT8 squawkntalk_tms_command;
static UINT8 squawkntalk_tms_strobes;



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static void ssio_compute_ay8910_modulation(running_machine *machine);



/*************************************
 *
 *  Generic MCR sound initialization
 *
 *************************************/

void mcr_sound_init(running_machine *machine, UINT8 config)
{
	int sound_cpu = 1;

	mcr_sound_config = config;

	/* SSIO */
	if (mcr_sound_config & MCR_SSIO)
	{
		ssio_sound_cpu = cputag_get_cpu(machine, "ssiocpu");
		ssio_compute_ay8910_modulation(machine);
		state_save_register_global_array(machine, ssio_data);
		state_save_register_global(machine, ssio_status);
		state_save_register_global(machine, ssio_14024_count);
		state_save_register_global(machine, ssio_mute);
		state_save_register_global_array(machine, ssio_overall);
		state_save_register_global_2d_array(machine, ssio_duty_cycle);
	}

	/* Turbo Chip Squeak */
	if (mcr_sound_config & MCR_TURBO_CHIP_SQUEAK)
	{
		turbocs_sound_cpu = cputag_get_cpu(machine, "tcscpu");
		state_save_register_global(machine, turbocs_status);
	}

	/* Chip Squeak Deluxe */
	if (mcr_sound_config & MCR_CHIP_SQUEAK_DELUXE)
	{
		csdeluxe_sound_cpu = cputag_get_cpu(machine, "csdcpu");
		state_save_register_global(machine, csdeluxe_status);
	}

	/* Sounds Good */
	if (mcr_sound_config & MCR_SOUNDS_GOOD)
	{
		soundsgood_sound_cpu = cputag_get_cpu(machine, "sgcpu");
		state_save_register_global(machine, soundsgood_status);
	}

	/* Squawk n Talk */
	if (mcr_sound_config & MCR_SQUAWK_N_TALK)
	{
		squawkntalk_sound_cpu = cputag_get_cpu(machine, "sntcpu");
		state_save_register_global(machine, squawkntalk_tms_command);
		state_save_register_global(machine, squawkntalk_tms_strobes);
	}

	/* Advanced Audio */
	if (mcr_sound_config & MCR_WILLIAMS_SOUND)
	{
		williams_cvsd_init(machine);
		sound_cpu++;
	}
}


void mcr_sound_reset(running_machine *machine)
{
	/* SSIO */
	if (mcr_sound_config & MCR_SSIO)
	{
		ssio_reset_w(machine, 1);
		ssio_reset_w(machine, 0);
	}

	/* Turbo Chip Squeak */
	if (mcr_sound_config & MCR_TURBO_CHIP_SQUEAK)
	{
		turbocs_reset_w(machine, 1);
		turbocs_reset_w(machine, 0);
	}

	/* Chip Squeak Deluxe */
	if (mcr_sound_config & MCR_CHIP_SQUEAK_DELUXE)
	{
		csdeluxe_reset_w(machine, 1);
		csdeluxe_reset_w(machine, 0);
	}

	/* Sounds Good */
	if (mcr_sound_config & MCR_SOUNDS_GOOD)
	{
		soundsgood_reset_w(machine, 1);
		soundsgood_reset_w(machine, 0);
	}

	/* Squawk n Talk */
	if (mcr_sound_config & MCR_SQUAWK_N_TALK)
	{
		squawkntalk_reset_w(machine, 1);
		squawkntalk_reset_w(machine, 0);
	}

	/* Advanced Audio */
	if (mcr_sound_config & MCR_WILLIAMS_SOUND)
	{
		williams_cvsd_reset_w(1);
		williams_cvsd_reset_w(0);
	}
}



/*************************************
 *
 *  MCR SSIO communications
 *
 *  Z80, 2 AY-8910
 *
 *************************************/

/*
    AY-8910 modulation:

        Starts with a 16MHz oscillator
            /2 via 7474 flip-flip @ F11

        This signal clocks the binary counter @ E11 which
        cascades into the decade counter @ D11. This combo
        effectively counts from 0-159 and then wraps. The
        value from these counters is input to an 82S123 PROM,
        which appears to be standard on all games.

        One bit at a time from this PROM is clocked at a time
        and the resulting inverted signal becomes a clock for
        the down counters at F3, F4, F5, F8, F9, and F10. The
        value in these down counters are reloaded after the 160
        counts from the binary/decade counter combination.

        When these down counters are loaded, the TC signal is
        clear, which mutes the voice. When the down counters
        cross through 0, the TC signal goes high and the 4016
        multiplexers allow the AY-8910 voice to go through.
        Thus, writing a 0 to the counters will enable the
        voice for the longest period of time, while writing
        a 15 enables it for the shortest period of time.
        This creates an effective duty cycle for the voice.

        Given that the down counters are reset 50000 times per
        second (SSIO_CLOCK/2/160), which is above the typical
        frequency of sound output. So we simply apply a volume
        adjustment to each voice according to the duty cycle.
*/
static void ssio_compute_ay8910_modulation(running_machine *machine)
{
	UINT8 *prom = memory_region(machine, "proms");
	int volval;

	/* loop over all possible values of the duty cycle */
	for (volval = 0; volval < 16; volval++)
	{
		int remaining_clocks = volval;
		int clock, cur = 0, prev = 1;

		/* loop over all the clocks until we run out; look up in the PROM */
		/* to find out when the next clock should fire */
		for (clock = 0; clock < 160 && remaining_clocks; clock++)
		{
			cur = prom[clock / 8] & (0x80 >> (clock % 8));

			/* check for a high -> low transition */
			if (cur == 0 && prev != 0)
				remaining_clocks--;

			prev = cur;
		}

		/* treat the duty cycle as a volume */
		ssio_ayvolume_lookup[15-volval] = clock * 100 / 160;
	}
}

/********* internal interfaces ***********/
static INTERRUPT_GEN( ssio_14024_clock )
{
	/*
        /SINT is generated as follows:

        Starts with a 16MHz oscillator
            /2 via 7474 flip-flop @ F11
            /16 via 74161 binary counter @ E11
            /10 via 74190 decade counter @ D11

        Bit 3 of the decade counter clocks a 14024 7-bit async counter @ C12.
        This routine is called to clock this 7-bit counter.
        Bit 6 of the output is inverted and connected to /SINT.
    */
	ssio_14024_count = (ssio_14024_count + 1) & 0x7f;

	/* if the low 5 bits clocked to 0, bit 6 has changed state */
	if ((ssio_14024_count & 0x3f) == 0)
		cpu_set_input_line(device, 0, (ssio_14024_count & 0x40) ? ASSERT_LINE : CLEAR_LINE);
}

static READ8_HANDLER( ssio_irq_clear )
{
	/* a read here asynchronously resets the 14024 count, clearing /SINT */
	ssio_14024_count = 0;
	cpu_set_input_line(ssio_sound_cpu, 0, CLEAR_LINE);
	return 0xff;
}

static WRITE8_HANDLER( ssio_status_w )
{
	ssio_status = data;
}

static READ8_HANDLER( ssio_data_r )
{
	return ssio_data[offset];
}

static TIMER_CALLBACK( ssio_delayed_data_w )
{
	ssio_data[param >> 8] = param & 0xff;
}

static void ssio_update_volumes(running_machine *machine)
{
	const device_config *ay0 = devtag_get_device(machine, "ssio.1");
	const device_config *ay1 = devtag_get_device(machine, "ssio.2");
	ay8910_set_volume(ay0, 0, ssio_mute ? 0 : ssio_ayvolume_lookup[ssio_duty_cycle[0][0]]);
	ay8910_set_volume(ay0, 1, ssio_mute ? 0 : ssio_ayvolume_lookup[ssio_duty_cycle[0][1]]);
	ay8910_set_volume(ay0, 2, ssio_mute ? 0 : ssio_ayvolume_lookup[ssio_duty_cycle[0][2]]);
	ay8910_set_volume(ay1, 0, ssio_mute ? 0 : ssio_ayvolume_lookup[ssio_duty_cycle[1][0]]);
	ay8910_set_volume(ay1, 1, ssio_mute ? 0 : ssio_ayvolume_lookup[ssio_duty_cycle[1][1]]);
	ay8910_set_volume(ay1, 2, ssio_mute ? 0 : ssio_ayvolume_lookup[ssio_duty_cycle[1][2]]);
}

static WRITE8_DEVICE_HANDLER( ssio_porta0_w )
{
	ssio_duty_cycle[0][0] = data & 15;
	ssio_duty_cycle[0][1] = data >> 4;
	ssio_update_volumes(device->machine);
}

static WRITE8_DEVICE_HANDLER( ssio_portb0_w )
{
	ssio_duty_cycle[0][2] = data & 15;
	ssio_overall[0] = (data >> 4) & 7;
	ssio_update_volumes(device->machine);
}

static WRITE8_DEVICE_HANDLER( ssio_porta1_w )
{
	ssio_duty_cycle[1][0] = data & 15;
	ssio_duty_cycle[1][1] = data >> 4;
	ssio_update_volumes(device->machine);
}

static WRITE8_DEVICE_HANDLER( ssio_portb1_w )
{
	ssio_duty_cycle[1][2] = data & 15;
	ssio_overall[1] = (data >> 4) & 7;
	ssio_mute = data & 0x80;
	ssio_update_volumes(device->machine);
}

/********* external interfaces ***********/
WRITE8_HANDLER( ssio_data_w )
{
	timer_call_after_resynch(space->machine, NULL, (offset << 8) | (data & 0xff), ssio_delayed_data_w);
}

READ8_HANDLER( ssio_status_r )
{
	return ssio_status;
}

void ssio_reset_w(running_machine *machine, int state)
{
	/* going high halts the CPU */
	if (state)
	{
		int i;

		cpu_set_input_line(ssio_sound_cpu, INPUT_LINE_RESET, ASSERT_LINE);

		/* latches also get reset */
		for (i = 0; i < 4; i++)
			ssio_data[i] = 0;
		ssio_status = 0;
		ssio_14024_count = 0;
	}
	/* going low resets and reactivates the CPU */
	else
		cpu_set_input_line(ssio_sound_cpu, INPUT_LINE_RESET, CLEAR_LINE);
}

READ8_HANDLER( ssio_input_port_r )
{
	static const char *const port[] = { "SSIO.IP0", "SSIO.IP1", "SSIO.IP2", "SSIO.IP3", "SSIO.IP4" };
	UINT8 result = input_port_read_safe(space->machine, port[offset], 0xff);
	if (ssio_custom_input[offset])
		result = (result & ~ssio_custom_input_mask[offset]) |
		         ((*ssio_custom_input[offset])(space, offset) & ssio_custom_input_mask[offset]);
	return result;
}

WRITE8_HANDLER( ssio_output_port_w )
{
	int which = offset >> 2;
	if (which == 0)
		mcr_control_port_w(space, offset, data);
	if (ssio_custom_output[which])
		(*ssio_custom_output[which])(space, offset, data & ssio_custom_output_mask[which]);
}

void ssio_set_custom_input(int which, int mask, read8_space_func handler)
{
	ssio_custom_input[which] = handler;
	ssio_custom_input_mask[which] = mask;
}

void ssio_set_custom_output(int which, int mask, write8_space_func handler)
{
	ssio_custom_output[which/4] = handler;
	ssio_custom_output_mask[which/4] = mask;
}


/********* sound interfaces ***********/
static const ay8910_interface ssio_ay8910_interface_1 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(ssio_porta0_w),
	DEVCB_HANDLER(ssio_portb0_w)
};

static const ay8910_interface ssio_ay8910_interface_2 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(ssio_porta1_w),
	DEVCB_HANDLER(ssio_portb1_w)
};


/********* memory interfaces ***********/

/* address map verified from schematics */
static ADDRESS_MAP_START( ssio_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0x9000, 0x9003) AM_MIRROR(0x0ffc) AM_READ(ssio_data_r)
	AM_RANGE(0xa000, 0xa000) AM_MIRROR(0x0ffc) AM_DEVWRITE("ssio.1", ay8910_address_w)
	AM_RANGE(0xa001, 0xa001) AM_MIRROR(0x0ffc) AM_DEVREAD("ssio.1", ay8910_r)
	AM_RANGE(0xa002, 0xa002) AM_MIRROR(0x0ffc) AM_DEVWRITE("ssio.1", ay8910_data_w)
	AM_RANGE(0xb000, 0xb000) AM_MIRROR(0x0ffc) AM_DEVWRITE("ssio.2", ay8910_address_w)
	AM_RANGE(0xb001, 0xb001) AM_MIRROR(0x0ffc) AM_DEVREAD("ssio.2", ay8910_r)
	AM_RANGE(0xb002, 0xb002) AM_MIRROR(0x0ffc) AM_DEVWRITE("ssio.2", ay8910_data_w)
	AM_RANGE(0xc000, 0xcfff) AM_READNOP AM_WRITE(ssio_status_w)
	AM_RANGE(0xd000, 0xdfff) AM_WRITENOP	/* low bit controls yellow LED */
	AM_RANGE(0xe000, 0xefff) AM_READ(ssio_irq_clear)
	AM_RANGE(0xf000, 0xffff) AM_READ_PORT("SSIO.DIP")	/* 6 DIP switches */
ADDRESS_MAP_END


/********* machine driver ***********/
MACHINE_DRIVER_START(mcr_ssio)
	MDRV_CPU_ADD("ssiocpu", Z80, SSIO_CLOCK/2/4)
	MDRV_CPU_PROGRAM_MAP(ssio_map)
	MDRV_CPU_PERIODIC_INT(ssio_14024_clock, SSIO_CLOCK/2/16/10)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MDRV_SOUND_ADD("ssio.1", AY8910, SSIO_CLOCK/2/4)
	MDRV_SOUND_CONFIG(ssio_ay8910_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.33)

	MDRV_SOUND_ADD("ssio.2", AY8910, SSIO_CLOCK/2/4)
	MDRV_SOUND_CONFIG(ssio_ay8910_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.33)
MACHINE_DRIVER_END



/*************************************
 *
 *  Chip Squeak Deluxe communications
 *
 *  MC68000, 1 PIA, 10-bit DAC
 *
 *************************************/

/********* internal interfaces ***********/
static WRITE8_DEVICE_HANDLER( csdeluxe_porta_w )
{
	dacval = (dacval & ~0x3fc) | (data << 2);
	dac_signed_data_16_w(devtag_get_device(device->machine, "csddac"), dacval << 6);
}

static WRITE8_DEVICE_HANDLER( csdeluxe_portb_w )
{
	UINT8 z_mask = pia6821_get_port_b_z_mask(device);

	dacval = (dacval & ~0x003) | (data >> 6);
	dac_signed_data_16_w(devtag_get_device(device->machine, "csddac"), dacval << 6);

	if (~z_mask & 0x10)  csdeluxe_status = (csdeluxe_status & ~1) | ((data >> 4) & 1);
	if (~z_mask & 0x20)  csdeluxe_status = (csdeluxe_status & ~2) | ((data >> 4) & 2);
}

static WRITE_LINE_DEVICE_HANDLER( csdeluxe_irq )
{
	int combined_state = pia6821_get_irq_a(device) | pia6821_get_irq_b(device);

	cpu_set_input_line(csdeluxe_sound_cpu, 4, combined_state ? ASSERT_LINE : CLEAR_LINE);
}

static TIMER_CALLBACK( csdeluxe_delayed_data_w )
{
	const device_config *pia = devtag_get_device(machine, "csdpia");

	pia6821_portb_w(pia, 0, param & 0x0f);
	pia6821_ca1_w(pia, 0, ~param & 0x10);

	/* oftentimes games will write one nibble at a time; the sync on this is very */
	/* important, so we boost the interleave briefly while this happens */
	cpuexec_boost_interleave(machine, attotime_zero, ATTOTIME_IN_USEC(100));
}

static READ16_DEVICE_HANDLER( csdeluxe_pia_r )
{
	/* Spy Hunter accesses the MSB; Turbo Tag access via the LSB */
	/* My guess is that Turbo Tag works through a fluke, whereby the 68000 */
	/* using the MOVEP instruction outputs the same value on the high and */
	/* low bytes. */
	if (ACCESSING_BITS_8_15)
		return pia6821_alt_r(device, offset) << 8;
	else
		return pia6821_alt_r(device, offset);
}

static WRITE16_DEVICE_HANDLER( csdeluxe_pia_w )
{
	if (ACCESSING_BITS_8_15)
		pia6821_alt_w(device, offset, data >> 8);
	else
		pia6821_alt_w(device, offset, data);
}


/********* external interfaces ***********/
WRITE8_HANDLER( csdeluxe_data_w )
{
	timer_call_after_resynch(space->machine, NULL, data, csdeluxe_delayed_data_w);
}

READ8_HANDLER( csdeluxe_status_r )
{
	return csdeluxe_status;
}

void csdeluxe_reset_w(running_machine *machine, int state)
{
	cpu_set_input_line(csdeluxe_sound_cpu, INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}


/********* memory interfaces ***********/

/* address map determined by PAL; not verified */
static ADDRESS_MAP_START( csdeluxe_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x1ffff)
	AM_RANGE(0x000000, 0x007fff) AM_ROM
	AM_RANGE(0x018000, 0x018007) AM_DEVREADWRITE("csdpia", csdeluxe_pia_r, csdeluxe_pia_w)
	AM_RANGE(0x01c000, 0x01cfff) AM_RAM
ADDRESS_MAP_END


/********* PIA interfaces ***********/
static const pia6821_interface csdeluxe_pia_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(csdeluxe_porta_w),		/* port A out */
	DEVCB_HANDLER(csdeluxe_portb_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_LINE(csdeluxe_irq),		/* IRQA */
	DEVCB_LINE(csdeluxe_irq)		/* IRQB */
};


/********* machine driver ***********/
MACHINE_DRIVER_START(chip_squeak_deluxe)
	MDRV_CPU_ADD("csdcpu", M68000, CSDELUXE_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(csdeluxe_map)

	MDRV_PIA6821_ADD("csdpia", csdeluxe_pia_intf)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("csddac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

MACHINE_DRIVER_START(chip_squeak_deluxe_stereo)
	MDRV_CPU_ADD("csdcpu", M68000, CSDELUXE_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(csdeluxe_map)

	MDRV_PIA6821_ADD("csdpia", csdeluxe_pia_intf)

	MDRV_SOUND_ADD("csddac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END



/*************************************
 *
 *  MCR Sounds Good communications
 *
 *  MC68000, 1 PIA, 10-bit DAC
 *
 *************************************/

/********* internal interfaces ***********/
static WRITE8_DEVICE_HANDLER( soundsgood_porta_w )
{
	dacval = (dacval & ~0x3fc) | (data << 2);
	dac_signed_data_16_w(devtag_get_device(device->machine, "sgdac"), dacval << 6);
}

static WRITE8_DEVICE_HANDLER( soundsgood_portb_w )
{
	UINT8 z_mask = pia6821_get_port_b_z_mask(device);

	dacval = (dacval & ~0x003) | (data >> 6);
	dac_signed_data_16_w(devtag_get_device(device->machine, "sgdac"), dacval << 6);

	if (~z_mask & 0x10)  soundsgood_status = (soundsgood_status & ~1) | ((data >> 4) & 1);
	if (~z_mask & 0x20)  soundsgood_status = (soundsgood_status & ~2) | ((data >> 4) & 2);
}

static WRITE_LINE_DEVICE_HANDLER( soundsgood_irq )
{
	int combined_state = pia6821_get_irq_a(device) | pia6821_get_irq_b(device);

	cpu_set_input_line(soundsgood_sound_cpu, 4, combined_state ? ASSERT_LINE : CLEAR_LINE);
}

static TIMER_CALLBACK( soundsgood_delayed_data_w )
{
	const device_config *pia = devtag_get_device(machine, "sgpia");

	pia6821_portb_w(pia, 0, (param >> 1) & 0x0f);
	pia6821_ca1_w(pia, 0, ~param & 0x01);

	/* oftentimes games will write one nibble at a time; the sync on this is very */
	/* important, so we boost the interleave briefly while this happens */
	cpuexec_boost_interleave(machine, attotime_zero, ATTOTIME_IN_USEC(250));
}


/********* external interfaces ***********/
WRITE8_HANDLER( soundsgood_data_w )
{
	timer_call_after_resynch(space->machine, NULL, data, soundsgood_delayed_data_w);
}

READ8_HANDLER( soundsgood_status_r )
{
	return soundsgood_status;
}

void soundsgood_reset_w(running_machine *machine, int state)
{
//if (state) mame_printf_debug("SG Reset\n");
	cpu_set_input_line(soundsgood_sound_cpu, INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}


/********* memory interfaces ***********/

/* address map determined by PAL; not verified */
static ADDRESS_MAP_START( soundsgood_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x7ffff)
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x060000, 0x060007) AM_DEVREADWRITE8("sgpia", pia6821_alt_r, pia6821_alt_w, 0xff00)
	AM_RANGE(0x070000, 0x070fff) AM_RAM
ADDRESS_MAP_END


/********* PIA interfaces ***********/
static const pia6821_interface soundsgood_pia_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(soundsgood_porta_w),		/* port A out */
	DEVCB_HANDLER(soundsgood_portb_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_LINE(soundsgood_irq),		/* IRQA */
	DEVCB_LINE(soundsgood_irq)		/* IRQB */
};


/********* machine driver ***********/
MACHINE_DRIVER_START(sounds_good)
	MDRV_CPU_ADD("sgcpu", M68000, SOUNDSGOOD_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(soundsgood_map)

	MDRV_PIA6821_ADD("sgpia", soundsgood_pia_intf)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("sgdac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/*************************************
 *
 *  MCR Turbo Chip Squeak communications
 *
 *  MC6809, 1 PIA, 10-bit DAC
 *
 *************************************/

/********* internal interfaces ***********/
static WRITE8_DEVICE_HANDLER( turbocs_porta_w )
{
	dacval = (dacval & ~0x3fc) | (data << 2);
	dac_signed_data_16_w(devtag_get_device(device->machine, "tcsdac"), dacval << 6);
}

static WRITE8_DEVICE_HANDLER( turbocs_portb_w )
{
	dacval = (dacval & ~0x003) | (data >> 6);
	dac_signed_data_16_w(devtag_get_device(device->machine, "tcsdac"), dacval << 6);
	turbocs_status = (data >> 4) & 3;
}

static WRITE_LINE_DEVICE_HANDLER( turbocs_irq )
{
	int combined_state = pia6821_get_irq_a(device) | pia6821_get_irq_b(device);

	cpu_set_input_line(turbocs_sound_cpu, M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}

static TIMER_CALLBACK( turbocs_delayed_data_w )
{
	const device_config *pia = devtag_get_device(machine, "tcspia");

	pia6821_portb_w(pia, 0, (param >> 1) & 0x0f);
	pia6821_ca1_w(pia, 0, ~param & 0x01);

	/* oftentimes games will write one nibble at a time; the sync on this is very */
	/* important, so we boost the interleave briefly while this happens */
	cpuexec_boost_interleave(machine, attotime_zero, ATTOTIME_IN_USEC(100));
}


/********* external interfaces ***********/
WRITE8_HANDLER( turbocs_data_w )
{
	timer_call_after_resynch(space->machine, NULL, data, turbocs_delayed_data_w);
}

READ8_HANDLER( turbocs_status_r )
{
	return turbocs_status;
}

void turbocs_reset_w(running_machine *machine, int state)
{
	cpu_set_input_line(turbocs_sound_cpu, INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}


/********* memory interfaces ***********/

/* address map verified from schematics */
static ADDRESS_MAP_START( turbocs_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x3800) AM_RAM
	AM_RANGE(0x4000, 0x4003) AM_MIRROR(0x3ffc) AM_DEVREADWRITE("tcspia", pia6821_alt_r, pia6821_alt_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/********* PIA interfaces ***********/
static const pia6821_interface turbocs_pia_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(turbocs_porta_w),		/* port A out */
	DEVCB_HANDLER(turbocs_portb_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_LINE(turbocs_irq),		/* IRQA */
	DEVCB_LINE(turbocs_irq)		/* IRQB */
};


/********* machine driver ***********/
MACHINE_DRIVER_START(turbo_chip_squeak)
	MDRV_CPU_ADD("tcscpu", M6809E, TURBOCS_CLOCK)
	MDRV_CPU_PROGRAM_MAP(turbocs_map)

	MDRV_PIA6821_ADD("tcspia", turbocs_pia_intf)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("tcsdac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/*************************************
 *
 *  MCR Squawk n Talk communications
 *
 *  MC6802, 2 PIAs, TMS5200, AY8912 (not used), 8-bit DAC (not used)
 *
 *************************************/

/********* internal interfaces ***********/
static WRITE8_DEVICE_HANDLER( squawkntalk_porta1_w )
{
	logerror("Write to AY-8912 = %02X\n", data);
}

static WRITE8_HANDLER( squawkntalk_dac_w )
{
	logerror("Write to DAC = %02X\n", data);
}

static WRITE8_DEVICE_HANDLER( squawkntalk_porta2_w )
{
	squawkntalk_tms_command = data;
}

static WRITE8_DEVICE_HANDLER( squawkntalk_portb2_w )
{
	const device_config *tms = devtag_get_device(device->machine, "sntspeech");

	/* bits 0-1 select read/write strobes on the TMS5200 */
	data &= 0x03;

	/* write strobe -- pass the current command to the TMS5200 */
	if (((data ^ squawkntalk_tms_strobes) & 0x02) && !(data & 0x02))
	{
		tms5220_data_w(tms, offset, squawkntalk_tms_command);

		/* DoT expects the ready line to transition on a command/write here, so we oblige */
		pia6821_ca2_w(device, 0, 1);
		pia6821_ca2_w(device, 0, 0);
	}

	/* read strobe -- read the current status from the TMS5200 */
	else if (((data ^ squawkntalk_tms_strobes) & 0x01) && !(data & 0x01))
	{
		pia6821_porta_w(device, 0, tms5220_status_r(tms, offset));

		/* DoT expects the ready line to transition on a command/write here, so we oblige */
		pia6821_ca2_w(device, 0, 1);
		pia6821_ca2_w(device, 0, 0);
	}

	/* remember the state */
	squawkntalk_tms_strobes = data;
}

static WRITE_LINE_DEVICE_HANDLER( squawkntalk_irq )
{
	const device_config *pia0 = devtag_get_device(device->machine, "sntpia0");
	const device_config *pia1 = devtag_get_device(device->machine, "sntpia1");
	int combined_state = pia6821_get_irq_a(pia0) | pia6821_get_irq_b(pia0) | pia6821_get_irq_a(pia1) | pia6821_get_irq_b(pia1);

	cpu_set_input_line(squawkntalk_sound_cpu, M6800_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}

static TIMER_CALLBACK( squawkntalk_delayed_data_w )
{
	const device_config *pia0 = devtag_get_device(machine, "sntpia0");

	pia6821_porta_w(pia0, 0, ~param & 0x0f);
	pia6821_cb1_w(pia0, 0, ~param & 0x10);
}


/********* external interfaces ***********/
WRITE8_HANDLER( squawkntalk_data_w )
{
	timer_call_after_resynch(space->machine, NULL, data, squawkntalk_delayed_data_w);
}

void squawkntalk_reset_w(running_machine *machine, int state)
{
	cpu_set_input_line(squawkntalk_sound_cpu, INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}


/********* memory interfaces ***********/

/* address map verified from schematics */
/* note that jumpers control the ROM sizes; if these are changed, use the alternate */
/* address map below */
static ADDRESS_MAP_START( squawkntalk_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x007f) AM_RAM		/* internal RAM */
	AM_RANGE(0x0080, 0x0083) AM_MIRROR(0x4f6c) AM_DEVREADWRITE("sntpia0", pia6821_r, pia6821_w)
	AM_RANGE(0x0090, 0x0093) AM_MIRROR(0x4f6c) AM_DEVREADWRITE("sntpia1", pia6821_r, pia6821_w)
	AM_RANGE(0x1000, 0x1fff) AM_MIRROR(0x4000) AM_WRITE(squawkntalk_dac_w)
	AM_RANGE(0x8000, 0xbfff) AM_MIRROR(0x4000) AM_ROM
ADDRESS_MAP_END

/* alternate address map if the ROM jumpers are changed to support a smaller */
/* ROM size of 2k */
#ifdef UNUSED_FUNCTION
ADDRESS_MAP_START( squawkntalk_alt_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x007f) AM_RAM		/* internal RAM */
	AM_RANGE(0x0080, 0x0083) AM_MIRROR(0x676c) AM_DEVREADWRITE("sntpia0", pia6821_r, pia6821_w)
	AM_RANGE(0x0090, 0x0093) AM_MIRROR(0x676c) AM_DEVREADWRITE("sntpia1", pia6821_r, pia6821_w)
	AM_RANGE(0x0800, 0x0fff) AM_MIRROR(0x6000) AM_WRITE(squawkntalk_dac_w)
	AM_RANGE(0x8000, 0x9fff) AM_MIRROR(0x6000) AM_ROM
ADDRESS_MAP_END
#endif


/********* PIA interfaces ***********/
static const pia6821_interface squawkntalk_pia0_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(squawkntalk_porta1_w),		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_LINE(squawkntalk_irq),	/* IRQA */
	DEVCB_LINE(squawkntalk_irq)		/* IRQB */
};

static const pia6821_interface squawkntalk_pia1_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(squawkntalk_porta2_w),		/* port A out */
	DEVCB_HANDLER(squawkntalk_portb2_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_LINE(squawkntalk_irq),	/* IRQA */
	DEVCB_LINE(squawkntalk_irq)		/* IRQB */
};


/********* machine driver ***********/
MACHINE_DRIVER_START(squawk_n_talk)
	MDRV_CPU_ADD("sntcpu", M6802, SQUAWKTALK_CLOCK)
	MDRV_CPU_PROGRAM_MAP(squawkntalk_map)

	MDRV_PIA6821_ADD("sntpia0", squawkntalk_pia0_intf)
	MDRV_PIA6821_ADD("sntpia1", squawkntalk_pia1_intf)

	/* only used on Discs of Tron, which is stereo */
	MDRV_SOUND_ADD("sntspeech", TMS5200, 640000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.60)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.60)

	/* the board also supports an AY-8912 and/or an 8-bit DAC, neither of */
	/* which are populated on the Discs of Tron board */
MACHINE_DRIVER_END
