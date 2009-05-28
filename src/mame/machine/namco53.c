/***************************************************************************

Namco 53XX

This custom chip is a Fujitsu MB8843 MCU programmed to act as an I/O device.

        MB8843
       +------+
  EXTAL|1   42|Vcc
   XTAL|2   41|K3
 /RESET|3   40|K2
   /IRQ|4   39|K1
     SO|5   38|K0
     SI|6   37|R15
/SC /TO|7   36|R14
    /TC|8   35|R13
     P0|9   34|R12
     P1|10  33|R11
     P2|11  32|R10
     P3|12  31|R9
     O0|13  30|R8
     O1|14  29|R7
     O2|15  28|R6
     O3|16  27|R5
     O4|17  26|R4
     O5|18  25|R3
     O6|19  24|R2
     O7|20  23|R1
    GND|21  22|R0
       +------+

Bits K1-K3 select one of 8 modes in which the input data is interpreted.

Pole Position is hard-wired to use mode 0, which reads 4 steering inputs
and 4 DIP switches (only 1 of each is used).

Dig Dug can control which mode to use via the MOD bit latches. It sets
these values to mode 7 when running.

***************************************************************************/

#include "driver.h"
#include "namco53.h"
#include "cpu/mb88xx/mb88xx.h"


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


typedef struct _namco_53xx_state namco_53xx_state;
struct _namco_53xx_state
{
	const device_config *	cpu;
	UINT8					portO;
	devcb_resolved_read8 	k;
	devcb_resolved_read8 	in[4];
	devcb_resolved_write8 	p;
};

INLINE namco_53xx_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == NAMCO_53XX);

	return (namco_53xx_state *)device->token;
}



static READ8_HANDLER( namco_53xx_K_r )
{
	namco_53xx_state *state = get_safe_token(space->cpu->owner);
	return devcb_call_read8(&state->k, 0);
}

static READ8_HANDLER( namco_53xx_Rx_r )
{
	namco_53xx_state *state = get_safe_token(space->cpu->owner);
	return devcb_call_read8(&state->in[offset], 0);
}

static WRITE8_HANDLER( namco_53xx_O_w )
{
	namco_53xx_state *state = get_safe_token(space->cpu->owner);
	UINT8 out = (data & 0x0f);
	if (data & 0x10)
		state->portO = (state->portO & 0x0f) | (out << 4);
	else
		state->portO = (state->portO & 0xf0) | (out);
}

static WRITE8_HANDLER( namco_53xx_P_w )
{
	namco_53xx_state *state = get_safe_token(space->cpu->owner);
	devcb_call_write8(&state->p, 0, data);
}


static TIMER_CALLBACK( namco_53xx_irq_clear )
{
	namco_53xx_state *state = get_safe_token((const device_config *)ptr);
	cpu_set_input_line(state->cpu, 0, CLEAR_LINE);
}

void namco_53xx_read_request(const device_config *device)
{
	namco_53xx_state *state = get_safe_token(device);
	cpu_set_input_line(state->cpu, 0, ASSERT_LINE);

	// The execution time of one instruction is ~4us, so we must make sure to
	// give the cpu time to poll the /IRQ input before we clear it.
	// The input clock to the 06XX interface chip is 64H, that is
	// 18432000/6/64 = 48kHz, so it makes sense for the irq line to be
	// asserted for one clock cycle ~= 21us.
	timer_set(device->machine, ATTOTIME_IN_USEC(21), (void *)device, 0, namco_53xx_irq_clear);
}

READ8_DEVICE_HANDLER( namco_53xx_read )
{
	namco_53xx_state *state = get_safe_token(device);
	UINT8 res = state->portO;

	namco_53xx_read_request(device);

	return res;
}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

ADDRESS_MAP_START( namco_53xx_map_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(MB88_PORTK,  MB88_PORTK)  AM_READ(namco_53xx_K_r)
	AM_RANGE(MB88_PORTO,  MB88_PORTO)  AM_WRITE(namco_53xx_O_w)
	AM_RANGE(MB88_PORTP,  MB88_PORTP)  AM_WRITE(namco_53xx_P_w)
	AM_RANGE(MB88_PORTR0, MB88_PORTR3) AM_READ(namco_53xx_Rx_r)
ADDRESS_MAP_END


static MACHINE_DRIVER_START( namco_53xx )
	MDRV_CPU_ADD("mcu", MB8843, DERIVED_CLOCK(1,1))		/* parent clock, internally divided by 6 */
	MDRV_CPU_IO_MAP(namco_53xx_map_io)
MACHINE_DRIVER_END


ROM_START( namco_53xx )
	ROM_REGION( 0x400, "mcu", ROMREGION_LOADBYNAME )
	ROM_LOAD( "53xx.bin",     0x0000, 0x0400, CRC(b326fecb) SHA1(758d8583d658e4f1df93184009d86c3eb8713899) )
ROM_END


/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( namco_53xx )
{
	const namco_53xx_interface *config = (const namco_53xx_interface *)device->static_config;
	namco_53xx_state *state = get_safe_token(device);
	astring *tempstring = astring_alloc();

	assert(config != NULL);

	/* find our CPU */
	state->cpu = cputag_get_cpu(device->machine, device_build_tag(tempstring, device, "mcu"));
	assert(state->cpu != NULL);
	astring_free(tempstring);

	/* resolve our read/write callbacks */
	devcb_resolve_read8(&state->k, &config->k, device);
	devcb_resolve_read8(&state->in[0], &config->in[0], device);
	devcb_resolve_read8(&state->in[1], &config->in[1], device);
	devcb_resolve_read8(&state->in[2], &config->in[2], device);
	devcb_resolve_read8(&state->in[3], &config->in[3], device);
	devcb_resolve_write8(&state->p, &config->p, device);
}


/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( namco_53xx )
{
//  namco_53xx_state *state = get_safe_token(device);
}


/*-------------------------------------------------
    device get info callback
-------------------------------------------------*/

DEVICE_GET_INFO( namco_53xx )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(namco_53xx_state);				break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_PERIPHERAL;				break;

		/* --- the following bits of info are returned as pointers --- */
		case DEVINFO_PTR_ROM_REGION:			info->romregion = ROM_NAME(namco_53xx);			break;
		case DEVINFO_PTR_MACHINE_CONFIG:		info->machine_config = MACHINE_DRIVER_NAME(namco_53xx); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(namco_53xx); 	break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(namco_53xx); 	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Namco 53xx");					break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Namco I/O");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
