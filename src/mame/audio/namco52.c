/***************************************************************************

Namco 52XX

This instance of the Fujitsu MB8843 MCU is programmed to act as a sample player.
It is used by just two games: Bosconian and Pole Position.

A0-A15 = address to read from sample ROMs
D0-D7 = data freom sample ROMs
CMD = command from CPU (sample to play, 0 = none)
OUT = sound output

      +------+
 EXTAL|1   42|Vcc
  XTAL|2   41|CMD3
/RESET|3   40|CMD2
  /IRQ|4   39|CMD1
  n.c.|5   38|CMD0
  [2] |6   37|A7
  n.c.|7   36|A6
  [1] |8   35|A5
  OUT0|9   34|A4
  OUT1|10  33|A3
  OUT2|11  32|A2
  OUT3|12  31|A1
    A8|13  30|A0
    A9|14  29|D7
   A10|15  28|D6
   A11|16  27|D5
[3]A12|17  26|D4
[3]A13|18  25|D3
[3]A14|19  24|D2
[3]A15|20  23|D1
   GND|21  22|D0
      +------+

[1] in polepos, GND; in bosco, 4kHz output from a 555 timer
[2] in polepos, +5V; in bosco, GND
[3] in polepos, these are true address lines, in bosco they are chip select lines
    (each one select one of the four ROM chips). Behaviour related to [2]


CMD0-CMD3 -> K0-K3
D0-D3     -> R0-R3
D4-D7     -> R4-R7
A0-A3     -> R8-R11
A4-A7     -> R12-R15
A8-A11    -> O0-O3
A12-A15   -> O4-O7
OUT0-OUT3 -> P0-P3
/TC       -> [1]
SI        -> [2]

***************************************************************************/

#include "driver.h"
#include "namco52.h"
#include "cpu/mb88xx/mb88xx.h"

typedef struct _namco_52xx_state namco_52xx_state;
struct _namco_52xx_state
{
	const device_config *cpu;
	const device_config *discrete;
	int basenode;
	devcb_resolved_read8 romread;
	devcb_resolved_read8 si;
	UINT8 latched_cmd;
	UINT32 address;
};

INLINE namco_52xx_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == NAMCO_52XX);

	return (namco_52xx_state *)device->token;
}



static TIMER_CALLBACK( namco_52xx_latch_callback )
{
	namco_52xx_state *state = get_safe_token((const device_config *)ptr);
	state->latched_cmd = param;
}

static READ8_HANDLER( namco_52xx_K_r )
{
	namco_52xx_state *state = get_safe_token(space->cpu->owner);
	return state->latched_cmd & 0x0f;
}

static READ8_HANDLER( namco_52xx_SI_r )
{
	namco_52xx_state *state = get_safe_token(space->cpu->owner);
	return devcb_call_read8(&state->si, 0) ? 1 : 0;
}

static READ8_HANDLER( namco_52xx_R0_r )
{
	namco_52xx_state *state = get_safe_token(space->cpu->owner);
	return devcb_call_read8(&state->romread, state->address) & 0x0f;
}

static READ8_HANDLER( namco_52xx_R1_r )
{
	namco_52xx_state *state = get_safe_token(space->cpu->owner);
	return devcb_call_read8(&state->romread, state->address) >> 4;
}


static WRITE8_HANDLER( namco_52xx_P_w )
{
	namco_52xx_state *state = get_safe_token(space->cpu->owner);
	discrete_sound_w(state->discrete, NAMCO_52XX_P_DATA(state->basenode), data & 0x0f);
}

static WRITE8_HANDLER( namco_52xx_R2_w )
{
	namco_52xx_state *state = get_safe_token(space->cpu->owner);
	state->address = (state->address & 0xfff0) | ((data & 0xf) << 0);
}

static WRITE8_HANDLER( namco_52xx_R3_w )
{
	namco_52xx_state *state = get_safe_token(space->cpu->owner);
	state->address = (state->address & 0xff0f) | ((data & 0xf) << 4);
}

static WRITE8_HANDLER( namco_52xx_O_w )
{
	namco_52xx_state *state = get_safe_token(space->cpu->owner);
	if (data & 0x10)
		state->address = (state->address & 0x0fff) | ((data & 0xf) << 12);
	else
		state->address = (state->address & 0xf0ff) | ((data & 0xf) << 8);
}




static TIMER_CALLBACK( namco_52xx_irq_clear )
{
	namco_52xx_state *state = get_safe_token((const device_config *)ptr);
	cpu_set_input_line(state->cpu, 0, CLEAR_LINE);
}

WRITE8_DEVICE_HANDLER( namco_52xx_write )
{
	namco_52xx_state *state = get_safe_token(device);

	timer_call_after_resynch(device->machine, (void *)device, data, namco_52xx_latch_callback);

	cpu_set_input_line(state->cpu, 0, ASSERT_LINE);

	// The execution time of one instruction is ~4us, so we must make sure to
	// give the cpu time to poll the /IRQ input before we clear it.
	// The input clock to the 06XX interface chip is 64H, that is
	// 18432000/6/64 = 48kHz, so it makes sense for the irq line to be
	// asserted for one clock cycle ~= 21us.

	/* the 52xx uses TSTI to check for an interrupt; it also may be handling
       a timer interrupt, so we need to ensure the IRQ line is held long enough */
	timer_set(device->machine, ATTOTIME_IN_USEC(5*21), (void *)device, 0, namco_52xx_irq_clear);
}


static TIMER_CALLBACK( external_clock_pulse )
{
	namco_52xx_state *state = get_safe_token((const device_config *)ptr);
	mb88_external_clock_w(state->cpu, 1);
	mb88_external_clock_w(state->cpu, 0);
}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

ADDRESS_MAP_START( namco_52xx_map_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(MB88_PORTK,  MB88_PORTK)  AM_READ(namco_52xx_K_r)
	AM_RANGE(MB88_PORTO,  MB88_PORTO)  AM_WRITE(namco_52xx_O_w)
	AM_RANGE(MB88_PORTP,  MB88_PORTP)  AM_WRITE(namco_52xx_P_w)
	AM_RANGE(MB88_PORTSI, MB88_PORTSI) AM_READ(namco_52xx_SI_r)
	AM_RANGE(MB88_PORTR0, MB88_PORTR0) AM_READ(namco_52xx_R0_r)
	AM_RANGE(MB88_PORTR1, MB88_PORTR1) AM_READ(namco_52xx_R1_r)
	AM_RANGE(MB88_PORTR2, MB88_PORTR2) AM_WRITE(namco_52xx_R2_w)
	AM_RANGE(MB88_PORTR3, MB88_PORTR3) AM_WRITE(namco_52xx_R3_w)
ADDRESS_MAP_END


static MACHINE_DRIVER_START( namco_52xx )
	MDRV_CPU_ADD("mcu", MB8843, DERIVED_CLOCK(1,1))		/* parent clock, internally divided by 6 */
	MDRV_CPU_IO_MAP(namco_52xx_map_io)
MACHINE_DRIVER_END


ROM_START( namco_52xx )
	ROM_REGION( 0x400, "mcu", ROMREGION_LOADBYNAME )
	ROM_LOAD( "52xx.bin",     0x0000, 0x0400, CRC(3257d11e) SHA1(4883b2fdbc99eb7b9906357fcc53915842c2c186) )
ROM_END


/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( namco_52xx )
{
	namco_52xx_interface *intf = (namco_52xx_interface *)device->static_config;
	namco_52xx_state *state = get_safe_token(device);
	astring *tempstring = astring_alloc();

	/* find our CPU */
	state->cpu = cputag_get_cpu(device->machine, device_build_tag(tempstring, device, "mcu"));
	assert(state->cpu != NULL);
	astring_free(tempstring);

	/* find the attached discrete sound device */
	assert(intf->discrete != NULL);
	state->discrete = devtag_get_device(device->machine, intf->discrete);
	assert(state->discrete != NULL);
	state->basenode = intf->firstnode;

	/* resolve our read/write callbacks */
	devcb_resolve_read8(&state->romread, &intf->romread, device);
	devcb_resolve_read8(&state->si, &intf->si, device);

	/* start the external clock */
	if (intf->extclock != 0)
		timer_pulse(device->machine, attotime_make(0, intf->extclock), (void *)device, 0, external_clock_pulse);
}


/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( namco_52xx )
{
//  namco_52xx_state *state = get_safe_token(device);
}


/*-------------------------------------------------
    device get info callback
-------------------------------------------------*/

DEVICE_GET_INFO( namco_52xx )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(namco_52xx_state);				break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_PERIPHERAL;				break;

		/* --- the following bits of info are returned as pointers --- */
		case DEVINFO_PTR_ROM_REGION:			info->romregion = ROM_NAME(namco_52xx);			break;
		case DEVINFO_PTR_MACHINE_CONFIG:		info->machine_config = MACHINE_DRIVER_NAME(namco_52xx); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(namco_52xx); 	break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(namco_52xx); 	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Namco 52xx");					break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Namco I/O");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
