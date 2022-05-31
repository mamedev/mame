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
#include "cpu/rx01/rx01.h"
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


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(RX01, rx01_device, "rx01", "RX01 Floppy Disk Controller")

//-------------------------------------------------
//  rx01_device - constructor
//-------------------------------------------------

rx01_device::rx01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, RX01, tag, owner, clock)
	, m_image(*this, "floppy%u", 0U)
{
}

void rx01_device::firmware_map(address_map &map)
{
	map(00000, 02777).rom().region("firmware", 0);
}

void rx01_device::secbuf_map(address_map &map)
{
	map(00000, 01777).ram(); // FIXME: 1-bit
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void rx01_device::device_add_mconfig(machine_config &config)
{
	rx01_cpu_device &cpu(RX01_CPU(config, "rx01cpu", 20_MHz_XTAL));
	cpu.set_addrmap(AS_PROGRAM, &rx01_device::firmware_map);
	cpu.set_addrmap(AS_DATA, &rx01_device::secbuf_map);

	for (auto &floppy : m_image)
		LEGACY_FLOPPY(config, floppy, 0, &rx01_floppy_interface);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rx01_device::device_start()
{
	m_command_timer = timer_alloc(FUNC(rx01_device::service_command), this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void rx01_device::device_reset()
{
	m_command_timer->adjust(attotime::never);

	for (auto & elem : m_image)
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

uint16_t rx01_device::read(offs_t offset)
{
	switch (offset & 1)
	{
		case 0: return status_read();
		default: return data_read();
	}

}


//-------------------------------------------------
//  write
//-------------------------------------------------

void rx01_device::write(offs_t offset, uint16_t data)
{
	switch (offset & 1)
	{
		case 0: command_write(data); break;
		case 1: data_write(data); break;
	}
}

void rx01_device::command_write(uint16_t data)
{
	printf("command_write %04x\n",data);
	m_unit = BIT(data, 4);
	m_interrupt = BIT(data, 6);

	m_image[m_unit]->floppy_drive_set_ready_state(1, 0);


	if (BIT(data, 14)) // Initialize
	{
		printf("initialize\n");
		m_state = RX01_INIT;
	}
	else if (BIT(data,1)) // If GO bit is selected
	{
		m_rxcs &= ~(1 << 5); // Clear done bit
		m_buf_pos = 0; // Point to start of buffer
		switch ((data >> 1) & 7)
		{
			case 0: // Fill Buffer
				m_rxcs |= (1 << 7); // Set TR Bit
				m_state = RX01_FILL;
				break;
			case 1: // Empty Buffer
				m_state = RX01_EMPTY;
				break;
			case 2: // Write Sector
			case 3: // Read Sector
			case 6: // Write Deleted Data Sector
				m_rxes &= ~(1 << 6 | 1 << 1 | 1 << 0);// Reset bits 0, 1 and 6
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
	m_command_timer->adjust(attotime::from_msec(100));
}

uint16_t rx01_device::status_read()
{
	//printf("status_read %04x\n",m_rxcs);
	return m_rxcs;
}

void rx01_device::data_write(uint16_t data)
{
//  printf("data_write %04x\n",data);
	// data can be written only if TR is set
	if (BIT(m_rxcs,7))
		m_rxdb = data;
	machine().scheduler().timer_set(attotime::from_msec(100), timer_expired_delegate(FUNC(rx01_device::service_command), this));
}

uint16_t rx01_device::data_read()
{
	if (m_state == RX01_EMPTY && BIT(m_rxcs, 7))
		m_rxcs &= (1 << 7); // clear TR bit;
//  printf("data_read %04x\n",m_rxdb);
	return m_rxdb;
}

TIMER_CALLBACK_MEMBER(rx01_device::service_command)
{
	printf("service_command %d\n",m_state);
	m_rxes |= m_image[m_unit]->floppy_drive_get_flag_state(FLOPPY_DRIVE_READY) << 7;
	switch (m_state)
	{
		case RX01_FILL:
			m_buffer[m_buf_pos] = m_rxdb & 0xff;
			m_buf_pos++;
			if (m_buf_pos < 128)
			{
				m_rxcs |= (1 << 7); // Set TR Bit
			}
			else
			{
				//finished
			}
			break;
		case RX01_EMPTY:
			if (m_buf_pos >= 128)
			{
				//finished
			}
			else
			{
				m_buffer[m_buf_pos] = m_rxdb & 0xff;
				m_buf_pos++;
				m_rxcs |= (1 << 7); // Set TR Bit
			}
			break;
		case RX01_SET_SECTOR:
			m_rxsa = m_rxdb & 0x1f;
			m_rxcs |= (1 << 7); // Set TR Bit
			m_state = RX01_SET_TRACK;
			break;
		case RX01_SET_TRACK:
			m_rxta = m_rxdb & 0x7f;
			m_rxcs |= (1 << 7); // Set TR Bit
			m_state = RX01_TRANSFER;
			position_head();
			break;
		case RX01_TRANSFER:
			break;
		case RX01_COMPLETE:
			break;
		case RX01_INIT:
			m_rxes |= (1 << 2); // Set init done flag
			m_rxcs |= (1 << 5); // Set operation done
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

ROM_START(rx01)
	ROM_REGION(0x600, "firmware", 0) // Harris M1-7610-5 (82S126 equivalent) PROMs
	ROM_LOAD_NIB_LOW( "23-111a2.e13", 0x000, 0x100, CRC(67ada159) SHA1(7cdc31e4aa64491c6212cb3ec1e00e6ae41eff1e))
	ROM_LOAD_NIB_HIGH("23-421a2.e3",  0x000, 0x100, CRC(d95473f1) SHA1(0fe73ffc3fb9ace480b7100e8f6921cb925f9702))
	ROM_LOAD_NIB_LOW( "23-257a2.e14", 0x100, 0x100, CRC(226f5f48) SHA1(b458bcd6d48158448967bd9fe30ea4a7df3b44d0))
	ROM_LOAD_NIB_HIGH("23-258a2.e4",  0x100, 0x100, CRC(4c0efd41) SHA1(c3cc76fffdd626c1c07e3a38f5c0e5f1481aabfe))
	ROM_LOAD_NIB_LOW( "23-115a2.e15", 0x200, 0x100, CRC(99fc60a3) SHA1(172c9ecfc705a6df404aeec1de210aa992995c40))
	ROM_LOAD_NIB_HIGH("23-116a2.e5",  0x200, 0x100, CRC(35677163) SHA1(0298130686e87416eddff2346e858f38804222fd))
	ROM_LOAD_NIB_LOW( "23-117a2.e16", 0x300, 0x100, CRC(3cec12ee) SHA1(eb2289144bace85b4df04e06b2df3ea4b40c1c63))
	ROM_LOAD_NIB_HIGH("23-118a2.e6",  0x300, 0x100, CRC(d64aaffe) SHA1(06c4729e6d04a24ae0be0022e62389169706715a))
	ROM_LOAD_NIB_LOW( "23-259a2.e17", 0x400, 0x100, CRC(0a382bb3) SHA1(960d9a995ec67b85080765377b1a546bcfe80883))
	ROM_LOAD_NIB_HIGH("23-260a2.e7",  0x400, 0x100, CRC(3643c2e4) SHA1(3e11ae926ca746fcfb849adbea7a78329dbc73a7))
	ROM_LOAD_NIB_LOW( "23-121a2.e18", 0x500, 0x100, CRC(ebc0ced0) SHA1(ef09ba7df66af5afd355ef9d4fef0efb97b3d2f9))
	ROM_LOAD_NIB_HIGH("23-122a2.e8",  0x500, 0x100, CRC(04ab3bbe) SHA1(abf87d731213e51413c6ef3172f14353da36e791))
ROM_END

const tiny_rom_entry *rx01_device::device_rom_region() const
{
	return ROM_NAME(rx01);
}
