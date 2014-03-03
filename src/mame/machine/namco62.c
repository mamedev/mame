/***************************************************************************

    Namco 62XX

    This custom chip is a Fujitsu MB8843 MCU programmed to act as an I/O
    device. It is used by just one game: Gaplus.

    TODO: Chip pin description/layout/notes

***************************************************************************/

#include "emu.h"
#include "machine/namco62.h"
#include "cpu/mb88xx/mb88xx.h"


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

static ADDRESS_MAP_START( namco_62xx_map_io, AS_IO, 8, namco_62xx_device )
//  AM_RANGE(MB88_PORTK,  MB88_PORTK)  AM_READ(namco_62xx_K_r)
//  AM_RANGE(MB88_PORTO,  MB88_PORTO)  AM_WRITE(namco_62xx_O_w)
//  AM_RANGE(MB88_PORTR0, MB88_PORTR0) AM_READ(namco_62xx_R0_r)
//  AM_RANGE(MB88_PORTR2, MB88_PORTR2) AM_READ(namco_62xx_R2_r)
ADDRESS_MAP_END


static MACHINE_CONFIG_FRAGMENT( namco_62xx )
	MCFG_CPU_ADD("mcu", MB8843, DERIVED_CLOCK(1,1))     /* parent clock, internally divided by 6 (TODO: Correct?) */
	MCFG_CPU_IO_MAP(namco_62xx_map_io)
	MCFG_DEVICE_DISABLE()
MACHINE_CONFIG_END

ROM_START( namco_62xx )
	ROM_REGION( 0x800, "mcu", 0 )
	ROM_LOAD( "62xx.bin", 0x0000, 0x0800, CRC(308dc115) SHA1(fe0a60fc339ac2eeed4879a64c1aab130f9d4cfe) )
ROM_END


const device_type NAMCO_62XX = &device_creator<namco_62xx_device>;

namco_62xx_device::namco_62xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NAMCO_62XX, "Namco 62xx", tag, owner, clock, "namco62", __FILE__)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void namco_62xx_device::device_config_complete()
{
	// inherit a copy of the static data
	const namco_62xx_interface *intf = reinterpret_cast<const namco_62xx_interface *>(static_config());
	if (intf != NULL)
		*static_cast<namco_62xx_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in[0], 0, sizeof(m_in[0]));
		memset(&m_in[1], 0, sizeof(m_in[1]));
		memset(&m_in[2], 0, sizeof(m_in[2]));
		memset(&m_in[3], 0, sizeof(m_in[3]));
		memset(&m_out[0], 0, sizeof(m_out[0]));
		memset(&m_out[1], 0, sizeof(m_out[1]));
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_62xx_device::device_start()
{
	/* find our CPU */
	m_cpu = subdevice("mcu");
	assert(m_cpu != NULL);

	/* resolve our read callbacks */
	m_in_func[0].resolve(m_in[0], *this);
	m_in_func[1].resolve(m_in[1], *this);
	m_in_func[2].resolve(m_in[2], *this);
	m_in_func[3].resolve(m_in[3], *this);

	/* resolve our write callbacks */
	m_out_func[0].resolve(m_out[0], *this);
	m_out_func[1].resolve(m_out[1], *this);
}

//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor namco_62xx_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( namco_62xx  );
}

//-------------------------------------------------
//  device_rom_region - return a pointer to the
//  the device's ROM definitions
//-------------------------------------------------

const rom_entry *namco_62xx_device::device_rom_region() const
{
	return ROM_NAME(namco_62xx );
}
