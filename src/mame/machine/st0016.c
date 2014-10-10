/* ST0016 - CPU (z80) + Sound + Video */

#include "st0016.h"

const device_type ST0016_CPU = &device_creator<st0016_cpu_device>;


static ADDRESS_MAP_START(st0016_cpu_internal_map, AS_PROGRAM, 8, st0016_cpu_device)
	AM_RANGE(0xe900, 0xe9ff) AM_DEVREADWRITE("stsnd", st0016_device, st0016_snd_r, st0016_snd_w) /* sound regs 8 x $20 bytes, see notes */
ADDRESS_MAP_END


static ADDRESS_MAP_START(st0016_cpu_internal_io_map, AS_IO, 8, st0016_cpu_device)
ADDRESS_MAP_END


st0016_cpu_device::st0016_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80_device(mconfig, ST0016_CPU, "ST0016", tag, owner, clock, "st0016_cpu", __FILE__),
  	  m_io_space_config("io", ENDIANNESS_LITTLE, 8, 16, 0, ADDRESS_MAP_NAME(st0016_cpu_internal_io_map)),
	  m_space_config("regs", ENDIANNESS_LITTLE, 8, 16, 0, ADDRESS_MAP_NAME(st0016_cpu_internal_map))
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void st0016_cpu_device::device_start()
{
	z80_device::device_start();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void st0016_cpu_device::device_reset()
{
	z80_device::device_reset();
}

static const st0016_interface st0016_config =
{
	&st0016_charram
};

/* CPU interface */
static MACHINE_CONFIG_FRAGMENT( st0016_cpu )
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_ST0016_ADD("stsnd", 0)
	MCFG_SOUND_CONFIG(st0016_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

MACHINE_CONFIG_END

machine_config_constructor st0016_cpu_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( st0016_cpu );
}


