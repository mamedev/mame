/***************************************************************************

    audio/mcr.c

    Functions to emulate general the various MCR sound cards.

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "audio/mcr.h"
#include "audio/williams.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "sound/5220intf.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
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
static UINT8 ssio_sound_cpu;
static UINT8 ssio_data[4];
static UINT8 ssio_status;
static UINT8 ssio_14024_count;
static UINT8 ssio_mute;
static UINT8 ssio_overall[2];
static UINT8 ssio_duty_cycle[2][3];
static UINT8 ssio_ayvolume_lookup[16];
static UINT8 ssio_custom_input_mask[5];
static UINT8 ssio_custom_output_mask[2];
static read8_machine_func ssio_custom_input[5];
static write8_machine_func ssio_custom_output[2];

/* Chip Squeak Deluxe-specific globals */
static UINT8 csdeluxe_sound_cpu;
static UINT8 csdeluxe_dac_index;
static UINT8 csdeluxe_status;
static const pia6821_interface csdeluxe_pia_intf;

/* Turbo Chip Squeak-specific globals */
static UINT8 turbocs_sound_cpu;
static UINT8 turbocs_dac_index;
static UINT8 turbocs_status;
static const pia6821_interface turbocs_pia_intf;

/* Sounds Good-specific globals */
static UINT8 soundsgood_sound_cpu;
static UINT8 soundsgood_dac_index;
static UINT8 soundsgood_status;
static const pia6821_interface soundsgood_pia_intf;

/* Squawk n' Talk-specific globals */
static UINT8 squawkntalk_sound_cpu;
static UINT8 squawkntalk_tms_command;
static UINT8 squawkntalk_tms_strobes;
static const pia6821_interface squawkntalk_pia0_intf;
static const pia6821_interface squawkntalk_pia1_intf;



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static void ssio_compute_ay8910_modulation(void);



/*************************************
 *
 *  Generic MCR sound initialization
 *
 *************************************/

void mcr_sound_init(UINT8 config)
{
	int sound_cpu = 1;
	int dac_index = 0;

	mcr_sound_config = config;

	/* SSIO */
	if (mcr_sound_config & MCR_SSIO)
	{
		ssio_sound_cpu = sound_cpu++;
		ssio_compute_ay8910_modulation();
		state_save_register_global_array(ssio_data);
		state_save_register_global(ssio_status);
		state_save_register_global(ssio_14024_count);
		state_save_register_global(ssio_mute);
		state_save_register_global_array(ssio_overall);
		state_save_register_global_2d_array(ssio_duty_cycle);
	}

	/* Turbo Chip Squeak */
	if (mcr_sound_config & MCR_TURBO_CHIP_SQUEAK)
	{
		pia_config(0, &turbocs_pia_intf);
		turbocs_dac_index = dac_index++;
		turbocs_sound_cpu = sound_cpu++;
		state_save_register_global(turbocs_status);
	}

	/* Chip Squeak Deluxe */
	if (mcr_sound_config & MCR_CHIP_SQUEAK_DELUXE)
	{
		pia_config(0, &csdeluxe_pia_intf);
		csdeluxe_dac_index = dac_index++;
		csdeluxe_sound_cpu = sound_cpu++;
		state_save_register_global(csdeluxe_status);
	}

	/* Sounds Good */
	if (mcr_sound_config & MCR_SOUNDS_GOOD)
	{
		/* special case: Spy Hunter 2 has both Turbo CS and Sounds Good, so we use PIA slot 1 */
		pia_config(1, &soundsgood_pia_intf);
		soundsgood_dac_index = dac_index++;
		soundsgood_sound_cpu = sound_cpu++;
		state_save_register_global(soundsgood_status);
	}

	/* Squawk n Talk */
	if (mcr_sound_config & MCR_SQUAWK_N_TALK)
	{
		pia_config(0, &squawkntalk_pia0_intf);
		pia_config(1, &squawkntalk_pia1_intf);
		squawkntalk_sound_cpu = sound_cpu++;
		state_save_register_global(squawkntalk_tms_command);
		state_save_register_global(squawkntalk_tms_strobes);
	}

	/* Advanced Audio */
	if (mcr_sound_config & MCR_WILLIAMS_SOUND)
	{
		williams_cvsd_init(0);
		sound_cpu++;
		dac_index++;
	}
}


void mcr_sound_reset(void)
{
	/* SSIO */
	if (mcr_sound_config & MCR_SSIO)
	{
		ssio_reset_w(1);
		ssio_reset_w(0);
	}

	/* Turbo Chip Squeak */
	if (mcr_sound_config & MCR_TURBO_CHIP_SQUEAK)
	{
		turbocs_reset_w(1);
		turbocs_reset_w(0);
	}

	/* Chip Squeak Deluxe */
	if (mcr_sound_config & MCR_CHIP_SQUEAK_DELUXE)
	{
		csdeluxe_reset_w(1);
		csdeluxe_reset_w(0);
	}

	/* Sounds Good */
	if (mcr_sound_config & MCR_SOUNDS_GOOD)
	{
		soundsgood_reset_w(1);
		soundsgood_reset_w(0);
	}

	/* Squawk n Talk */
	if (mcr_sound_config & MCR_SQUAWK_N_TALK)
	{
		squawkntalk_reset_w(1);
		squawkntalk_reset_w(0);
	}

	/* Advanced Audio */
	if (mcr_sound_config & MCR_WILLIAMS_SOUND)
	{
		williams_cvsd_reset_w(1);
		williams_cvsd_reset_w(0);
	}

	/* reset any PIAs */
	pia_reset();
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
static void ssio_compute_ay8910_modulation(void)
{
	UINT8 *prom = memory_region(REGION_PROMS);
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
		cpunum_set_input_line(machine, ssio_sound_cpu, 0, (ssio_14024_count & 0x40) ? ASSERT_LINE : CLEAR_LINE);
}

static READ8_HANDLER( ssio_irq_clear )
{
	/* a read here asynchronously resets the 14024 count, clearing /SINT */
	ssio_14024_count = 0;
	cpunum_set_input_line(Machine, ssio_sound_cpu, 0, CLEAR_LINE);
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

static void ssio_update_volumes(void)
{
	AY8910_set_volume(0, 0, ssio_mute ? 0 : ssio_ayvolume_lookup[ssio_duty_cycle[0][0]]);
	AY8910_set_volume(0, 1, ssio_mute ? 0 : ssio_ayvolume_lookup[ssio_duty_cycle[0][1]]);
	AY8910_set_volume(0, 2, ssio_mute ? 0 : ssio_ayvolume_lookup[ssio_duty_cycle[0][2]]);
	AY8910_set_volume(1, 0, ssio_mute ? 0 : ssio_ayvolume_lookup[ssio_duty_cycle[1][0]]);
	AY8910_set_volume(1, 1, ssio_mute ? 0 : ssio_ayvolume_lookup[ssio_duty_cycle[1][1]]);
	AY8910_set_volume(1, 2, ssio_mute ? 0 : ssio_ayvolume_lookup[ssio_duty_cycle[1][2]]);
}

static WRITE8_HANDLER( ssio_porta0_w )
{
	ssio_duty_cycle[0][0] = data & 15;
	ssio_duty_cycle[0][1] = data >> 4;
	ssio_update_volumes();
}

static WRITE8_HANDLER( ssio_portb0_w )
{
	ssio_duty_cycle[0][2] = data & 15;
	ssio_overall[0] = (data >> 4) & 7;
	ssio_update_volumes();
}

static WRITE8_HANDLER( ssio_porta1_w )
{
	ssio_duty_cycle[1][0] = data & 15;
	ssio_duty_cycle[1][1] = data >> 4;
	ssio_update_volumes();
}

static WRITE8_HANDLER( ssio_portb1_w )
{
	ssio_duty_cycle[1][2] = data & 15;
	ssio_overall[1] = (data >> 4) & 7;
	ssio_mute = data & 0x80;
	ssio_update_volumes();
}

/********* external interfaces ***********/
WRITE8_HANDLER( ssio_data_w )
{
	timer_call_after_resynch(NULL, (offset << 8) | (data & 0xff), ssio_delayed_data_w);
}

READ8_HANDLER( ssio_status_r )
{
	return ssio_status;
}

void ssio_reset_w(int state)
{
	/* going high halts the CPU */
	if (state)
	{
		int i;

		cpunum_set_input_line(Machine, ssio_sound_cpu, INPUT_LINE_RESET, ASSERT_LINE);

		/* latches also get reset */
		for (i = 0; i < 4; i++)
			ssio_data[i] = 0;
		ssio_status = 0;
		ssio_14024_count = 0;
	}
	/* going low resets and reactivates the CPU */
	else
		cpunum_set_input_line(Machine, ssio_sound_cpu, INPUT_LINE_RESET, CLEAR_LINE);
}

READ8_HANDLER( ssio_input_port_r )
{
	static const char *const port[] = { "SSIO.IP0", "SSIO.IP1", "SSIO.IP2", "SSIO.IP3", "SSIO.IP4" };
	UINT8 result = readinputportbytag_safe(port[offset], 0xff);
	if (ssio_custom_input[offset])
		result = (result & ~ssio_custom_input_mask[offset]) |
		         ((*ssio_custom_input[offset])(machine, offset) & ssio_custom_input_mask[offset]);
	return result;
}

WRITE8_HANDLER( ssio_output_port_w )
{
	int which = offset >> 2;
	if (which == 0)
		mcr_control_port_w(machine, offset, data);
	if (ssio_custom_output[which])
		(*ssio_custom_output[which])(machine, offset, data & ssio_custom_output_mask[which]);
}

void ssio_set_custom_input(int which, int mask, read8_machine_func handler)
{
	ssio_custom_input[which] = handler;
	ssio_custom_input_mask[which] = mask;
}

void ssio_set_custom_output(int which, int mask, write8_machine_func handler)
{
	ssio_custom_output[which/4] = handler;
	ssio_custom_output_mask[which/4] = mask;
}


/********* sound interfaces ***********/
static const struct AY8910interface ssio_ay8910_interface_1 =
{
	0,
	0,
	ssio_porta0_w,
	ssio_portb0_w
};

static const struct AY8910interface ssio_ay8910_interface_2 =
{
	0,
	0,
	ssio_porta1_w,
	ssio_portb1_w
};


/********* memory interfaces ***********/

/* address map verified from schematics */
static ADDRESS_MAP_START( ssio_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0x9000, 0x9003) AM_MIRROR(0x0ffc) AM_READ(ssio_data_r)
	AM_RANGE(0xa000, 0xa000) AM_MIRROR(0x0ffc) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0xa001, 0xa001) AM_MIRROR(0x0ffc) AM_READ(AY8910_read_port_0_r)
	AM_RANGE(0xa002, 0xa002) AM_MIRROR(0x0ffc) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0xb000, 0xb000) AM_MIRROR(0x0ffc) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0xb001, 0xb001) AM_MIRROR(0x0ffc) AM_READ(AY8910_read_port_1_r)
	AM_RANGE(0xb002, 0xb002) AM_MIRROR(0x0ffc) AM_WRITE(AY8910_write_port_1_w)
	AM_RANGE(0xc000, 0xcfff) AM_READWRITE(SMH_NOP, ssio_status_w)
	AM_RANGE(0xd000, 0xdfff) AM_WRITENOP	/* low bit controls yellow LED */
	AM_RANGE(0xe000, 0xefff) AM_READ(ssio_irq_clear)
	AM_RANGE(0xf000, 0xffff) AM_READ_PORT("SSIO.DIP")	/* 6 DIP switches */
ADDRESS_MAP_END


/********* machine driver ***********/
MACHINE_DRIVER_START(mcr_ssio)
	MDRV_CPU_ADD_TAG("ssio", Z80, SSIO_CLOCK/2/4)
	MDRV_CPU_PROGRAM_MAP(ssio_map,0)
	MDRV_CPU_PERIODIC_INT(ssio_14024_clock, SSIO_CLOCK/2/16/10)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")
	MDRV_SOUND_ADD_TAG("ssio.1", AY8910, SSIO_CLOCK/2/4)
	MDRV_SOUND_CONFIG(ssio_ay8910_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.33)

	MDRV_SOUND_ADD_TAG("ssio.2", AY8910, SSIO_CLOCK/2/4)
	MDRV_SOUND_CONFIG(ssio_ay8910_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.33)
MACHINE_DRIVER_END



/*************************************
 *
 *  Chip Squeak Deluxe communications
 *
 *  MC68000, 1 PIA, 10-bit DAC
 *
 *************************************/

/********* internal interfaces ***********/
static WRITE8_HANDLER( csdeluxe_porta_w )
{
	dacval = (dacval & ~0x3fc) | (data << 2);
	DAC_signed_data_16_w(csdeluxe_dac_index, dacval << 6);
}

static WRITE8_HANDLER( csdeluxe_portb_w )
{
	UINT8 z_mask = pia_get_port_b_z_mask(0);

	dacval = (dacval & ~0x003) | (data >> 6);
	DAC_signed_data_16_w(csdeluxe_dac_index, dacval << 6);

	if (~z_mask & 0x10)  csdeluxe_status = (csdeluxe_status & ~1) | ((data >> 4) & 1);
	if (~z_mask & 0x20)  csdeluxe_status = (csdeluxe_status & ~2) | ((data >> 4) & 2);
}

static void csdeluxe_irq(int state)
{
	int combined_state = pia_get_irq_a(0) | pia_get_irq_b(0);

  	cpunum_set_input_line(Machine, csdeluxe_sound_cpu, 4, combined_state ? ASSERT_LINE : CLEAR_LINE);
}

static TIMER_CALLBACK( csdeluxe_delayed_data_w )
{
	pia_0_portb_w(machine, 0, param & 0x0f);
	pia_0_ca1_w(machine, 0, ~param & 0x10);

	/* oftentimes games will write one nibble at a time; the sync on this is very */
	/* important, so we boost the interleave briefly while this happens */
	cpu_boost_interleave(attotime_zero, ATTOTIME_IN_USEC(100));
}

static READ16_HANDLER( csdeluxe_pia_r )
{
	/* Spy Hunter accesses the MSB; Turbo Tag access via the LSB */
	/* My guess is that Turbo Tag works through a fluke, whereby the 68000 */
	/* using the MOVEP instruction outputs the same value on the high and */
	/* low bytes. */
	if (ACCESSING_BYTE_1)
		return pia_0_alt_r(machine, offset) << 8;
	else
		return pia_0_alt_r(machine, offset);
}

static WRITE16_HANDLER( csdeluxe_pia_w )
{
	if (ACCESSING_BYTE_1)
		pia_0_alt_w(machine, offset, data >> 8);
	else
		pia_0_alt_w(machine, offset, data);
}


/********* external interfaces ***********/
WRITE8_HANDLER( csdeluxe_data_w )
{
	timer_call_after_resynch(NULL, data, csdeluxe_delayed_data_w);
}

READ8_HANDLER( csdeluxe_status_r )
{
	return csdeluxe_status;
}

void csdeluxe_reset_w(int state)
{
	cpunum_set_input_line(Machine, csdeluxe_sound_cpu, INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}


/********* memory interfaces ***********/

/* address map determined by PAL; not verified */
static ADDRESS_MAP_START( csdeluxe_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x1ffff)
	AM_RANGE(0x000000, 0x007fff) AM_ROM
	AM_RANGE(0x018000, 0x018007) AM_READWRITE(csdeluxe_pia_r, csdeluxe_pia_w)
	AM_RANGE(0x01c000, 0x01cfff) AM_RAM
ADDRESS_MAP_END


/********* PIA interfaces ***********/
static const pia6821_interface csdeluxe_pia_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ csdeluxe_porta_w, csdeluxe_portb_w, 0, 0,
	/*irqs   : A/B             */ csdeluxe_irq, csdeluxe_irq
};


/********* machine driver ***********/
MACHINE_DRIVER_START(chip_squeak_deluxe)
	MDRV_CPU_ADD_TAG("csd", M68000, CSDELUXE_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(csdeluxe_map,0)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD_TAG("csd", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

MACHINE_DRIVER_START(chip_squeak_deluxe_stereo)
	MDRV_CPU_ADD_TAG("csd", M68000, CSDELUXE_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(csdeluxe_map,0)

	MDRV_SOUND_ADD_TAG("csd", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.0)
MACHINE_DRIVER_END



/*************************************
 *
 *  MCR Sounds Good communications
 *
 *  MC68000, 1 PIA, 10-bit DAC
 *
 *************************************/

/********* internal interfaces ***********/
static WRITE8_HANDLER( soundsgood_porta_w )
{
	dacval = (dacval & ~0x3fc) | (data << 2);
	DAC_signed_data_16_w(soundsgood_dac_index, dacval << 6);
}

static WRITE8_HANDLER( soundsgood_portb_w )
{
	UINT8 z_mask = pia_get_port_b_z_mask(1);

	dacval = (dacval & ~0x003) | (data >> 6);
	DAC_signed_data_16_w(soundsgood_dac_index, dacval << 6);

	if (~z_mask & 0x10)  soundsgood_status = (soundsgood_status & ~1) | ((data >> 4) & 1);
	if (~z_mask & 0x20)  soundsgood_status = (soundsgood_status & ~2) | ((data >> 4) & 2);
}

static void soundsgood_irq(int state)
{
	int combined_state = pia_get_irq_a(1) | pia_get_irq_b(1);

  	cpunum_set_input_line(Machine, soundsgood_sound_cpu, 4, combined_state ? ASSERT_LINE : CLEAR_LINE);
}

static TIMER_CALLBACK( soundsgood_delayed_data_w )
{
	pia_1_portb_w(machine, 0, (param >> 1) & 0x0f);
	pia_1_ca1_w(machine, 0, ~param & 0x01);

	/* oftentimes games will write one nibble at a time; the sync on this is very */
	/* important, so we boost the interleave briefly while this happens */
	cpu_boost_interleave(attotime_zero, ATTOTIME_IN_USEC(250));
}


/********* external interfaces ***********/
WRITE8_HANDLER( soundsgood_data_w )
{
	timer_call_after_resynch(NULL, data, soundsgood_delayed_data_w);
}

READ8_HANDLER( soundsgood_status_r )
{
	return soundsgood_status;
}

void soundsgood_reset_w(int state)
{
//if (state) mame_printf_debug("SG Reset\n");
	cpunum_set_input_line(Machine, soundsgood_sound_cpu, INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}


/********* memory interfaces ***********/

/* address map determined by PAL; not verified */
static ADDRESS_MAP_START( soundsgood_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x7ffff)
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x060000, 0x060007) AM_READWRITE(pia_1_msb_alt_r, pia_1_msb_alt_w)
	AM_RANGE(0x070000, 0x070fff) AM_RAM
ADDRESS_MAP_END


/********* PIA interfaces ***********/
/* Note: we map this board to PIA #1. It is only used in Spy Hunter and Spy Hunter 2 */
/* For Spy Hunter 2, we also have a Turbo Chip Squeak in PIA slot 0, so we don't want */
/* to interfere */
static const pia6821_interface soundsgood_pia_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ soundsgood_porta_w, soundsgood_portb_w, 0, 0,
	/*irqs   : A/B             */ soundsgood_irq, soundsgood_irq
};


/********* machine driver ***********/
MACHINE_DRIVER_START(sounds_good)
	MDRV_CPU_ADD_TAG("sg", M68000, SOUNDSGOOD_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(soundsgood_map,0)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD_TAG("sg", DAC, 0)
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
static WRITE8_HANDLER( turbocs_porta_w )
{
	dacval = (dacval & ~0x3fc) | (data << 2);
	DAC_signed_data_16_w(turbocs_dac_index, dacval << 6);
}

static WRITE8_HANDLER( turbocs_portb_w )
{
	dacval = (dacval & ~0x003) | (data >> 6);
	DAC_signed_data_16_w(turbocs_dac_index, dacval << 6);
	turbocs_status = (data >> 4) & 3;
}

static void turbocs_irq(int state)
{
	int combined_state = pia_get_irq_a(0) | pia_get_irq_b(0);

	cpunum_set_input_line(Machine, turbocs_sound_cpu, M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}

static TIMER_CALLBACK( turbocs_delayed_data_w )
{
	pia_0_portb_w(machine, 0, (param >> 1) & 0x0f);
	pia_0_ca1_w(machine, 0, ~param & 0x01);

	/* oftentimes games will write one nibble at a time; the sync on this is very */
	/* important, so we boost the interleave briefly while this happens */
	cpu_boost_interleave(attotime_zero, ATTOTIME_IN_USEC(100));
}


/********* external interfaces ***********/
WRITE8_HANDLER( turbocs_data_w )
{
	timer_call_after_resynch(NULL, data, turbocs_delayed_data_w);
}

READ8_HANDLER( turbocs_status_r )
{
	return turbocs_status;
}

void turbocs_reset_w(int state)
{
	cpunum_set_input_line(Machine, turbocs_sound_cpu, INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}


/********* memory interfaces ***********/

/* address map verified from schematics */
static ADDRESS_MAP_START( turbocs_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x3800) AM_RAM
	AM_RANGE(0x4000, 0x4003) AM_MIRROR(0x3ffc) AM_READWRITE(pia_0_alt_r, pia_0_alt_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/********* PIA interfaces ***********/
static const pia6821_interface turbocs_pia_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ turbocs_porta_w, turbocs_portb_w, 0, 0,
	/*irqs   : A/B             */ turbocs_irq, turbocs_irq
};


/********* machine driver ***********/
MACHINE_DRIVER_START(turbo_chip_squeak)
	MDRV_CPU_ADD_TAG("tcs", M6809E, TURBOCS_CLOCK)
	MDRV_CPU_PROGRAM_MAP(turbocs_map,0)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD_TAG("tcs", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


MACHINE_DRIVER_START(turbo_chip_squeak_plus_sounds_good)
	MDRV_IMPORT_FROM(turbo_chip_squeak)
	MDRV_SPEAKER_REMOVE("mono")
	MDRV_IMPORT_FROM(sounds_good)

	MDRV_SOUND_REPLACE("tcs", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_REPLACE("sg", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END



/*************************************
 *
 *  MCR Squawk n Talk communications
 *
 *  MC6802, 2 PIAs, TMS5200, AY8912 (not used), 8-bit DAC (not used)
 *
 *************************************/

/********* internal interfaces ***********/
static WRITE8_HANDLER( squawkntalk_porta1_w )
{
	logerror("Write to AY-8912 = %02X\n", data);
}

static WRITE8_HANDLER( squawkntalk_dac_w )
{
	logerror("Write to DAC = %02X\n", data);
}

static WRITE8_HANDLER( squawkntalk_porta2_w )
{
	squawkntalk_tms_command = data;
}

static WRITE8_HANDLER( squawkntalk_portb2_w )
{
	/* bits 0-1 select read/write strobes on the TMS5200 */
	data &= 0x03;

	/* write strobe -- pass the current command to the TMS5200 */
	if (((data ^ squawkntalk_tms_strobes) & 0x02) && !(data & 0x02))
	{
		tms5220_data_w(machine, offset, squawkntalk_tms_command);

		/* DoT expects the ready line to transition on a command/write here, so we oblige */
		pia_1_ca2_w(machine, 0, 1);
		pia_1_ca2_w(machine, 0, 0);
	}

	/* read strobe -- read the current status from the TMS5200 */
	else if (((data ^ squawkntalk_tms_strobes) & 0x01) && !(data & 0x01))
	{
		pia_1_porta_w(machine, 0, tms5220_status_r(machine, offset));

		/* DoT expects the ready line to transition on a command/write here, so we oblige */
		pia_1_ca2_w(machine, 0, 1);
		pia_1_ca2_w(machine, 0, 0);
	}

	/* remember the state */
	squawkntalk_tms_strobes = data;
}

static void squawkntalk_irq(int state)
{
	int combined_state = pia_get_irq_a(0) | pia_get_irq_b(0) | pia_get_irq_a(1) | pia_get_irq_b(1);

	cpunum_set_input_line(Machine, squawkntalk_sound_cpu, M6808_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}

static TIMER_CALLBACK( squawkntalk_delayed_data_w )
{
	pia_0_porta_w(machine, 0, ~param & 0x0f);
	pia_0_cb1_w(machine, 0, ~param & 0x10);
}


/********* external interfaces ***********/
WRITE8_HANDLER( squawkntalk_data_w )
{
	timer_call_after_resynch(NULL, data, squawkntalk_delayed_data_w);
}

void squawkntalk_reset_w(int state)
{
	cpunum_set_input_line(Machine, squawkntalk_sound_cpu, INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
}


/********* memory interfaces ***********/

/* address map verified from schematics */
/* note that jumpers control the ROM sizes; if these are changed, use the alternate */
/* address map below */
static ADDRESS_MAP_START( squawkntalk_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x007f) AM_RAM		/* internal RAM */
	AM_RANGE(0x0080, 0x0083) AM_MIRROR(0x4f6c) AM_READWRITE(pia_0_r, pia_0_w)
	AM_RANGE(0x0090, 0x0093) AM_MIRROR(0x4f6c) AM_READWRITE(pia_1_r, pia_1_w)
	AM_RANGE(0x1000, 0x1fff) AM_MIRROR(0x4000) AM_WRITE(squawkntalk_dac_w)
	AM_RANGE(0x8000, 0xbfff) AM_MIRROR(0x4000) AM_ROM
ADDRESS_MAP_END

/* alternate address map if the ROM jumpers are changed to support a smaller */
/* ROM size of 2k */
#ifdef UNUSED_FUNCTION
ADDRESS_MAP_START( squawkntalk_alt_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x007f) AM_RAM		/* internal RAM */
	AM_RANGE(0x0080, 0x0083) AM_MIRROR(0x676c) AM_READWRITE(pia_0_r, pia_0_w)
	AM_RANGE(0x0090, 0x0093) AM_MIRROR(0x676c) AM_READWRITE(pia_1_r, pia_1_w)
	AM_RANGE(0x0800, 0x0fff) AM_MIRROR(0x6000) AM_WRITE(squawkntalk_dac_w)
	AM_RANGE(0x8000, 0x9fff) AM_MIRROR(0x6000) AM_ROM
ADDRESS_MAP_END
#endif


/********* PIA interfaces ***********/
static const pia6821_interface squawkntalk_pia0_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ squawkntalk_porta1_w, 0, 0, 0,
	/*irqs   : A/B             */ squawkntalk_irq, squawkntalk_irq
};

static const pia6821_interface squawkntalk_pia1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ squawkntalk_porta2_w, squawkntalk_portb2_w, 0, 0,
	/*irqs   : A/B             */ squawkntalk_irq, squawkntalk_irq
};


/********* machine driver ***********/
MACHINE_DRIVER_START(squawk_n_talk)
	MDRV_CPU_ADD_TAG("snt", M6802, SQUAWKTALK_CLOCK)
	MDRV_CPU_PROGRAM_MAP(squawkntalk_map,0)

	/* only used on Discs of Tron, which is stereo */
	MDRV_SOUND_ADD_TAG("snt", TMS5200, 640000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.60)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.60)

	/* the board also supports an AY-8912 and/or an 8-bit DAC, neither of */
	/* which are populated on the Discs of Tron board */
MACHINE_DRIVER_END
