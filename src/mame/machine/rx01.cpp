// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/**********************************************************************

    DEC RX01 floppy drive controller

**********************************************************************/

/*

    TODO:
    - Create also unibus and qbus devices that contain this controller on them
*/

#include "emu.h"
#include "machine/rx01.h"
#include "formats/basicdsk.h"

static LEGACY_FLOPPY_OPTIONS_START( rx01 )
	LEGACY_FLOPPY_OPTION(rx01, "img", "RX01 image", basicdsk_identify_default, basicdsk_construct_default, nullptr,
		HEADS([1])
		TRACKS([77])
		SECTORS([26])
		SECTOR_LENGTH([128])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END


static const floppy_interface rx01_floppy_interface =
{
	FLOPPY_STANDARD_8_SSSD,
	LEGACY_FLOPPY_OPTIONS_NAME(rx01),
	"floppy_8"
};


MACHINE_CONFIG_FRAGMENT( rx01 )
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(rx01_floppy_interface)
MACHINE_CONFIG_END

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type RX01 = &device_creator<rx01_device>;

//-------------------------------------------------
//  rx01_device - constructor
//-------------------------------------------------

rx01_device::rx01_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, RX01, "RX01", tag, owner, clock, "rx01", __FILE__)
{
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor rx01_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( rx01 );
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rx01_device::device_start()
{
	m_image[0] = subdevice<legacy_floppy_image_device>(FLOPPY_0);
	m_image[1] = subdevice<legacy_floppy_image_device>(FLOPPY_1);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void rx01_device::device_reset()
{
	for(auto & elem : m_image)
	{
		elem->floppy_mon_w(0); // turn it on
		elem->floppy_drive_set_controller(this);
		elem->floppy_drive_set_rpm(360.);
	}
	m_rxes = 0;
	m_rxcs = 0;
	m_rxdb = 0;
}

//-------------------------------------------------
//  read
//-------------------------------------------------

READ16_MEMBER( rx01_device::read )
{
	switch(offset & 1) {
		case 0: return status_read();
		default: return data_read();
	}

}


//-------------------------------------------------
//  write
//-------------------------------------------------

WRITE16_MEMBER( rx01_device::write )
{
	switch(offset & 1) {
		case 0: command_write(data); break;
		case 1: data_write(data); break;
	}
}

void rx01_device::command_write(UINT16 data)
{
	printf("command_write %04x\n",data);
	m_unit = BIT(data,4);
	m_interrupt = BIT(data,6);

	m_image[m_unit]->floppy_drive_set_ready_state(1,0);


	if (BIT(data,14)) // Initialize
	{
		printf("initialize\n");
		m_state = RX01_INIT;
	}
	else if (BIT(data,1)) // If GO bit is selected
	{
		m_rxcs &= ~(1<<5); // Clear done bit
		m_buf_pos = 0; // Point to start of buffer
		switch((data >> 1) & 7) {
			case 0: // Fill Buffer
					m_rxcs |= (1<<7); // Set TR Bit
					m_state = RX01_FILL;
					break;
			case 1: // Empty Buffer
					m_state = RX01_EMPTY;
					break;
			case 2: // Write Sector
			case 3: // Read Sector
			case 6: // Write Deleted Data Sector
					m_rxes &= ~(1<<6 | 1<<1 | 1<<0);// Reset bits 0, 1 and 6
					m_state = RX01_SET_SECTOR;
					break;
			case 4: // Not Used
					m_state = RX01_COMPLETE;
					break;
			case 5: // Read Status
					m_state = RX01_COMPLETE;
					m_rxdb = m_rxes;
					break;
			case 7: // Read Error Register
					m_state = RX01_COMPLETE;
					// set ready signal according to current drive status
					m_rxes |= m_image[m_unit]->floppy_drive_get_flag_state(FLOPPY_DRIVE_READY) << 7;
					break;
		}
	}
	machine().scheduler().timer_set(attotime::from_msec(100), FUNC(command_execution_callback), 0, this);
}

UINT16 rx01_device::status_read()
{
	//printf("status_read %04x\n",m_rxcs);
	return m_rxcs;
}

void rx01_device::data_write(UINT16 data)
{
//  printf("data_write %04x\n",data);
	// data can be written only if TR is set
	if (BIT(m_rxcs,7)) m_rxdb = data;
	machine().scheduler().timer_set(attotime::from_msec(100), FUNC(command_execution_callback), 0, this);
}

UINT16 rx01_device::data_read()
{
	if (m_state==RX01_EMPTY && BIT(m_rxcs,7)) m_rxcs &= (1<<7); // clear TR bit;
//  printf("data_read %04x\n",m_rxdb);
	return m_rxdb;
}

void rx01_device::service_command()
{
	printf("service_command %d\n",m_state);
	m_rxes |= m_image[m_unit]->floppy_drive_get_flag_state(FLOPPY_DRIVE_READY) << 7;
	switch(m_state) {
		case RX01_FILL :
						m_buffer[m_buf_pos] = m_rxdb & 0xff;
						m_buf_pos++;
						if (m_buf_pos<128)
						{
							m_rxcs |= (1<<7); // Set TR Bit
						} else {
							//finished
						}
						break;
		case RX01_EMPTY :
						if (m_buf_pos>=128)
						{
							//finished
						} else {
							m_buffer[m_buf_pos] = m_rxdb & 0xff;
							m_buf_pos++;
							m_rxcs |= (1<<7); // Set TR Bit
						}
						break;
		case RX01_SET_SECTOR:
						m_rxsa = m_rxdb & 0x1f;
						m_rxcs |= (1<<7); // Set TR Bit
						m_state = RX01_SET_TRACK;
						break;
		case RX01_SET_TRACK:
						m_rxta = m_rxdb & 0x7f;
						m_rxcs |= (1<<7); // Set TR Bit
						m_state = RX01_TRANSFER;
						position_head();
						break;
		case RX01_TRANSFER:
						break;
		case RX01_COMPLETE:
						break;
		case RX01_INIT:
						m_rxes |= (1<<2); // Set init done flag
						m_rxcs |= (1<<5); // Set operation done
						break;
	}
}

void rx01_device::position_head()
{
	int cur_track = m_image[m_unit]->floppy_drive_get_current_track();
	int dir = (cur_track < m_rxta) ? +1 : -1;

	while (m_rxta != cur_track)
	{
		cur_track += dir;

		m_image[m_unit]->floppy_drive_seek(dir);
	}
}

void rx01_device::read_sector()
{
	/* read data */
	m_image[m_unit]->floppy_drive_read_sector_data(0, m_rxsa, (char *)m_buffer, 128);
}

void rx01_device::write_sector(int ddam)
{
	/* write data */
	m_image[m_unit]->floppy_drive_write_sector_data(0, m_rxsa, (char *)m_buffer, 128, ddam);
}
