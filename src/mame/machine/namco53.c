/***************************************************************************

Namco 53XX

This custom chip is a Fujitsu MB8843 MCU programmed to act as an I/O device.

The chip reads/writes the I/O ports when the /IRQ is pulled down. Pin 21
determines whether a read or write should happen (1=R, 0=W).

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


commands:
00: nop
01 + 4 arguments: set coinage (xevious, possibly because of a bug, is different)
02: go in "credit" mode and enable start buttons
03: disable joystick remapping
04: enable joystick remapping
05: go in "switch" mode
06: nop
07: nop

***************************************************************************/

#include "driver.h"
#include "namco53.h"
#include "cpu/mb88xx/mb88xx.h"


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


#define READ_PORT(st,num) 			devcb_call_read8(&(st)->in[num], 0)
#define WRITE_PORT(st,num,data) 	devcb_call_write8(&(st)->out[num], 0, data)


typedef struct _namco_53xx_state namco_53xx_state;
struct _namco_53xx_state
{
	const device_config *	cpu;
	int in_count;
	devcb_resolved_read8 in[4];
	devcb_resolved_write8 out[2];
};

INLINE namco_53xx_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == NAMCO_53XX);

	return (namco_53xx_state *)device->token;
}



READ8_DEVICE_HANDLER( namco_53xx_read )
{
	namco_53xx_state *state = get_safe_token(device);

	LOG(("%s: custom 53XX read\n",cpuexec_describe_context(device->machine)));

// digdug: ((state->in_count++) % 2)
	switch ((state->in_count++) % 8)
	{
		case 0: return READ_PORT(state,0) | (READ_PORT(state,1) << 4);	// steering
// digdug: case 1:
		case 4: return READ_PORT(state,2) | (READ_PORT(state,3) << 4);	// dip switches
		default: return 0xff;	// polepos2 hangs if 0 is returned
	}
}




/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

ADDRESS_MAP_START( namco_53xx_map_io, ADDRESS_SPACE_IO, 8 )
//	AM_RANGE(MB88_PORTK,  MB88_PORTK)  AM_READ(namco_53xx_K_r)
//	AM_RANGE(MB88_PORTO,  MB88_PORTO)  AM_WRITE(namco_53xx_O_w)
//	AM_RANGE(MB88_PORTR0, MB88_PORTR0) AM_READ(namco_53xx_R0_r)
//	AM_RANGE(MB88_PORTR2, MB88_PORTR2) AM_READ(namco_53xx_R2_r)
ADDRESS_MAP_END


static MACHINE_DRIVER_START( namco_53xx )
	MDRV_CPU_ADD("mcu", MB8843, DERIVED_CLOCK(1,1))		/* parent clock, internally divided by 6 */
	MDRV_CPU_IO_MAP(namco_53xx_map_io)
	MDRV_CPU_FLAGS(CPU_DISABLE)
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

	/* resolve our read callbacks */
	devcb_resolve_read8(&state->in[0], &config->in[0], device);
	devcb_resolve_read8(&state->in[1], &config->in[1], device);
	devcb_resolve_read8(&state->in[2], &config->in[2], device);
	devcb_resolve_read8(&state->in[3], &config->in[3], device);

	/* resolve our write callbacks */
	devcb_resolve_write8(&state->out[0], &config->out[0], device);
	devcb_resolve_write8(&state->out[1], &config->out[1], device);
	
	state_save_register_device_item(device, 0, state->in_count);
}


/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( namco_53xx )
{
	namco_53xx_state *state = get_safe_token(device);
	state->in_count = 0;
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
