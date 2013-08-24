#include "atapicdr.h"
#include "scsicd.h"

// device type definition
const device_type ATAPI_CDROM = &device_creator<atapi_cdrom_device>;

atapi_cdrom_device::atapi_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: atapi_hle_device(mconfig, ATAPI_CDROM, "ATAPI CDROM", tag, owner, clock, "cdrom", __FILE__)
{
}

static MACHINE_CONFIG_FRAGMENT( atapicdr )
	MCFG_DEVICE_ADD("device", SCSICD, 0)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor atapi_cdrom_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( atapicdr );
}

void atapi_cdrom_device::perform_diagnostic()
{
	m_error = IDE_ERROR_DIAGNOSTIC_PASSED;
}

void atapi_cdrom_device::identify_packet_device()
{
	m_buffer_size = 512;

	memset(m_buffer, 0, m_buffer_size);

	m_buffer[ 0 ^ 1 ] = 0x85; // ATAPI device, cmd set 5 compliant, DRQ within 3 ms of PACKET command
	m_buffer[ 1 ^ 1 ] = 0x80; // ATAPI device, removable media

	memset( &m_buffer[ 46 ], ' ', 8 );
	m_buffer[ 46 ^ 1 ] = '1';
	m_buffer[ 47 ^ 1 ] = '.';
	m_buffer[ 48 ^ 1 ] = '0';

	memset( &m_buffer[ 54 ], ' ', 40 );
	m_buffer[ 54 ^ 1 ] = 'M';
	m_buffer[ 55 ^ 1 ] = 'A';
	m_buffer[ 56 ^ 1 ] = 'M';
	m_buffer[ 57 ^ 1 ] = 'E';
	m_buffer[ 58 ^ 1 ] = ' ';
	m_buffer[ 59 ^ 1 ] = 'C';
	m_buffer[ 60 ^ 1 ] = 'o';
	m_buffer[ 61 ^ 1 ] = 'm';
	m_buffer[ 62 ^ 1 ] = 'p';
	m_buffer[ 63 ^ 1 ] = 'r';
	m_buffer[ 64 ^ 1 ] = 'e';
	m_buffer[ 65 ^ 1 ] = 's';
	m_buffer[ 66 ^ 1 ] = 's';
	m_buffer[ 67 ^ 1 ] = 'e';
	m_buffer[ 68 ^ 1 ] = 'd';
	m_buffer[ 69 ^ 1 ] = ' ';
	m_buffer[ 70 ^ 1 ] = 'C';
	m_buffer[ 71 ^ 1 ] = 'D';
	m_buffer[ 72 ^ 1 ] = '-';
	m_buffer[ 73 ^ 1 ] = 'R';
	m_buffer[ 74 ^ 1 ] = 'O';
	m_buffer[ 75 ^ 1 ] = 'M';

	m_buffer[ 98 ^ 1 ] = 0x06; // Word 49=Capabilities, IORDY may be disabled (bit_10), LBA Supported mandatory (bit_9)
	m_buffer[ 99 ^ 1 ] = 0x00;
}
