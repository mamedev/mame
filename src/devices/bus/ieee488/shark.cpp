// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Mator Systems SHARK Intelligent Winchester Disc Subsystem emulation

**********************************************************************/

#include "emu.h"
#include "shark.h"

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/harddriv.h"
#include "machine/i8255.h"

#include <algorithm>
#include <cstdlib>



namespace {

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8085_TAG       "i8085"
#define RS232_TAG       "rs232"


//-------------------------------------------------
//  ROM( mshark )
//-------------------------------------------------

ROM_START( mshark )
	ROM_REGION( 0x5000, I8085_TAG, 0 )
	ROM_LOAD( "pch488 3450 v22.1 @1", 0x0000, 0x1000, CRC(03bff9d7) SHA1(ac506df6509e1b2185a69f9f8f44b8b456aa9834) )
	ROM_LOAD( "pch488 3450 v22.1 @2", 0x1000, 0x1000, CRC(c14fa5fe) SHA1(bcfd1dd65d692c76b90e6134b85f22c39c049430) )
	ROM_LOAD( "pch488 3450 v22.1 @3", 0x2000, 0x1000, CRC(4dfaa482) SHA1(fe2c44bb650572616c8bdad6358032fe64b1e363) )
	ROM_LOAD( "pch488 3450 v22.1 @4", 0x3000, 0x1000, CRC(aef665e9) SHA1(80a4c00b717100b4e22fa3704e34060fffce2bc3) )
	ROM_LOAD( "pch488 3450 v22.1 @5", 0x4000, 0x1000, CRC(f30adf60) SHA1(96c15264d5a9b52e1d238921880c48a797a6da1e) )

	ROM_REGION( 0x800, "micro", 0 ) // address decoder
	ROM_LOAD( "micro p3450 v1.3", 0x000, 0x800, CRC(0e69202e) SHA1(3b384951ff54c4b45a3a778a88966d13e2c9d57a) )
ROM_END


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mshark_device

class mshark_device :  public device_t,
						public device_ieee488_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::DISK; }

	// construction/destruction
	mshark_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_ieee488_interface implementation
	virtual void ieee488_atn(int state) override;
	virtual void ieee488_ifc(int state) override;

private:
	static constexpr unsigned CYLINDERS = 525;
	static constexpr unsigned HEADS = 5;
	static constexpr unsigned SECTORS = 44;
	static constexpr unsigned SECTOR_SIZE = 256;

	required_device<i8085a_cpu_device> m_maincpu;
	required_address_space m_program;
	required_device<i8255_device> m_disk_ppi;
	required_device<i8255_device> m_ieee_ppi;
	required_device<harddisk_image_device> m_harddisk;
	required_region_ptr<uint8_t> m_micro;
	emu_timer *m_drive_timer;
	emu_timer *m_sector_timer;

	uint8_t m_ieee_pc;
	uint8_t m_disk_pc;
	bool m_atn;
	bool m_ifc;
	uint8_t m_drive_select;
	uint8_t m_drive_status;
	uint8_t m_drive_command;
	uint16_t m_target_cylinder;
	uint16_t m_current_cylinder;
	uint16_t m_drive_parameter;
	bool m_spinning;
	uint8_t m_record_id[3];
	uint16_t m_dma_address;
	uint8_t m_controller_status;
	uint16_t m_sequence;
	uint16_t m_transfer_count;
	uint32_t m_sector_lba;
	uint8_t m_sector_buffer[SECTOR_SIZE + 3];
	uint8_t m_transfer_mode;

	bool drive_selected() const;
	uint8_t drive_status_r();
	uint8_t current_cylinder_r(offs_t offset);
	void target_cylinder_w(offs_t offset, uint8_t data);
	void drive_command_w(uint8_t data);
	TIMER_CALLBACK_MEMBER(drive_complete);
	uint8_t disk_pb_r();
	void disk_pa_w(uint8_t data);
	void record_id_w(offs_t offset, uint8_t data);
	void dma_address_w(offs_t offset, uint8_t data);
	void start_sector();
	TIMER_CALLBACK_MEMBER(sector_tick);
	void disk_pc_w(uint8_t data);
	void update_ieee_signals();
	uint8_t ieee_pc_r();
	void ieee_pc_w(uint8_t data);
	uint8_t switches_r();
	void update_interrupt();

	void mshark_io(address_map &map) ATTR_COLD;
	void mshark_mem(address_map &map) ATTR_COLD;
};


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mshark_device::device_rom_region() const
{
	return ROM_NAME( mshark );
}


//-------------------------------------------------
//  ADDRESS_MAP( mshark_mem )
//-------------------------------------------------

void mshark_device::mshark_mem(address_map &map)
{
	map(0x0000, 0x4fff).rom().region(I8085_TAG, 0);
	map(0x8000, 0xbfff).ram();
}


//-------------------------------------------------
//  ADDRESS_MAP( mshark_io )
//-------------------------------------------------

void mshark_device::mshark_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x00).rw(FUNC(mshark_device::drive_status_r), FUNC(mshark_device::drive_command_w));
	map(0x01, 0x02).rw(FUNC(mshark_device::current_cylinder_r), FUNC(mshark_device::target_cylinder_w));
	map(0x04, 0x07).rw(m_disk_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x0a).w(FUNC(mshark_device::record_id_w));
	map(0x0c, 0x0d).w(FUNC(mshark_device::dma_address_w));
	map(0x0e, 0x0e).nopw();
	map(0x10, 0x13).rw(m_ieee_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x14, 0x14).r(FUNC(mshark_device::switches_r));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mshark_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	I8085A(config, m_maincpu, 1000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mshark_device::mshark_mem);
	m_maincpu->set_addrmap(AS_IO, &mshark_device::mshark_io);

	I8255(config, m_disk_ppi);
	m_disk_ppi->out_pa_callback().set(FUNC(mshark_device::disk_pa_w));
	m_disk_ppi->in_pb_callback().set(FUNC(mshark_device::disk_pb_r));
	m_disk_ppi->out_pc_callback().set(FUNC(mshark_device::disk_pc_w));

	I8255(config, m_ieee_ppi);
	m_ieee_ppi->out_pa_callback().set([this] (uint8_t data) { m_bus->dio_w(this, data); });
	m_ieee_ppi->in_pb_callback().set([this] () -> uint8_t { return m_bus->dio_r(); });
	m_ieee_ppi->in_pc_callback().set(FUNC(mshark_device::ieee_pc_r));
	m_ieee_ppi->out_pc_callback().set(FUNC(mshark_device::ieee_pc_w));

	// devices
	HARDDISK(config, m_harddisk, "mator_shark_hdd");
	RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr);
}


//-------------------------------------------------
//  INPUT_PORTS( mshark )
//-------------------------------------------------

INPUT_PORTS_START( mshark )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor mshark_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( mshark );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mshark_device - constructor
//-------------------------------------------------

mshark_device::mshark_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, GPIB_MSHARK, tag, owner, clock)
	, device_ieee488_interface(mconfig, *this)
	, m_maincpu(*this, I8085_TAG)
	, m_program(*this, I8085_TAG, AS_PROGRAM)
	, m_disk_ppi(*this, "disk_ppi")
	, m_ieee_ppi(*this, "ieee_ppi")
	, m_harddisk(*this, "harddisk1")
	, m_micro(*this, "micro")
	, m_ieee_pc(0xff)
	, m_disk_pc(0xff)
	, m_atn(true)
	, m_ifc(true)
	, m_drive_select(0xff)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mshark_device::device_start()
{
	m_drive_timer = timer_alloc(FUNC(mshark_device::drive_complete), this);
	m_sector_timer = timer_alloc(FUNC(mshark_device::sector_tick), this);

	save_item(NAME(m_drive_select));
	save_item(NAME(m_drive_status));
	save_item(NAME(m_drive_command));
	save_item(NAME(m_target_cylinder));
	save_item(NAME(m_current_cylinder));
	save_item(NAME(m_drive_parameter));
	save_item(NAME(m_spinning));
	save_item(NAME(m_record_id));
	save_item(NAME(m_dma_address));
	save_item(NAME(m_controller_status));
	save_item(NAME(m_sequence));
	save_item(NAME(m_transfer_count));
	save_item(NAME(m_sector_lba));
	save_item(NAME(m_sector_buffer));
	save_item(NAME(m_transfer_mode));
	save_item(NAME(m_ieee_pc));
	save_item(NAME(m_disk_pc));
	save_item(NAME(m_atn));
	save_item(NAME(m_ifc));
}

void mshark_device::device_reset()
{
	m_ieee_pc = 0xff;
	m_disk_pc = 0xff;
	m_drive_status = 0x48;
	m_drive_command = 0;
	m_target_cylinder = 0;
	m_current_cylinder = 0;
	m_drive_parameter = 0;
	m_spinning = false;
	std::fill(std::begin(m_record_id), std::end(m_record_id), 0);
	m_dma_address = 0;
	m_controller_status = 0;
	m_sequence = 0;
	m_transfer_count = 0;
	m_sector_lba = 0;
	std::fill(std::begin(m_sector_buffer), std::end(m_sector_buffer), 0);
	m_transfer_mode = 0;
	m_drive_timer->adjust(attotime::never);
	m_sector_timer->adjust(attotime::never);
	update_ieee_signals();
	update_interrupt();
}

bool mshark_device::drive_selected() const
{
	return !BIT(m_drive_select, 0) && m_harddisk->exists();
}

uint8_t mshark_device::drive_status_r()
{
	if (!drive_selected())
		return 0xff;
	return m_drive_status | (m_harddisk->is_readonly() ? 0x40 : 0x00);
}

uint8_t mshark_device::current_cylinder_r(offs_t offset)
{
	if (!drive_selected())
		return 0xff;
	uint16_t const value = BIT(m_drive_status, 0) ? m_current_cylinder : m_drive_parameter;
	return offset ? uint8_t(value) : uint8_t(value >> 8);
}

void mshark_device::target_cylinder_w(offs_t offset, uint8_t data)
{
	if (!drive_selected())
		return;
	if (!BIT(m_drive_status, 0))
	{
		m_drive_status |= 0x80;
		return;
	}
	if (offset)
		m_target_cylinder = (m_target_cylinder & 0x0700) | data;
	else
		m_target_cylinder = (m_target_cylinder & 0x00ff) | (uint16_t(data & 7) << 8);
}

void mshark_device::drive_command_w(uint8_t data)
{
	if (!drive_selected())
		return;
	if (BIT(m_drive_status, 4))
	{
		m_drive_status |= 0x80;
		return;
	}

	attotime delay = attotime::from_usec(20);
	switch (data)
	{
	case 0x01:
	case 0x03:
		delay = m_spinning ? attotime::from_msec(150) : attotime::from_seconds(30);
		break;
	case 0x02:
		delay = attotime::from_seconds(1);
		break;
	case 0x04:
		if (!BIT(m_drive_status, 0))
		{
			m_drive_status |= 0x80;
			return;
		}
		delay = attotime::from_usec(8'000 + 200 * std::abs(int(m_target_cylinder) - int(m_current_cylinder)));
		break;
	case 0x05:
	case 0x10:
	case 0x11:
		break;
	default:
		m_drive_status |= 0x80;
		return;
	}
	m_drive_command = data;
	m_drive_status = (m_drive_status & 0x68) | 0x10;
	m_drive_timer->adjust(delay);
}

TIMER_CALLBACK_MEMBER(mshark_device::drive_complete)
{
	if (!m_harddisk->exists())
	{
		m_drive_status = 0x48;
		m_spinning = false;
		return;
	}

	switch (m_drive_command)
	{
	case 0x01:
	case 0x03:
		m_current_cylinder = 0;
		m_spinning = true;
		m_drive_status = 0x0b;
		break;
	case 0x02:
		m_spinning = false;
		m_drive_status = 0x40;
		break;
	case 0x04:
		if (m_target_cylinder >= m_harddisk->get_info().cylinders)
		{
			m_current_cylinder = 0;
			m_drive_status = 0x0d;
		}
		else
		{
			m_current_cylinder = m_target_cylinder;
			m_drive_status = m_current_cylinder ? 0x03 : 0x0b;
		}
		break;
	case 0x05:
		m_drive_status = m_spinning ? (m_current_cylinder ? 0x03 : 0x0b) : 0x40;
		break;
	case 0x10:
		m_drive_parameter = 0x04;
		m_drive_status &= 0x68;
		break;
	case 0x11:
		m_drive_parameter = 304;
		m_drive_status &= 0x68;
		break;
	}
}

uint8_t mshark_device::disk_pb_r()
{
	return 0xc0 | m_controller_status | (drive_selected() ? 0x00 : 0x01);
}

void mshark_device::disk_pa_w(uint8_t data)
{
	if (data != m_drive_select && m_sector_timer->enabled())
	{
		m_sector_timer->adjust(attotime::never);
		m_controller_status = 0x20;
	}
	m_drive_select = data;
}

void mshark_device::record_id_w(offs_t offset, uint8_t data)
{
	m_record_id[offset] = data;
}

void mshark_device::dma_address_w(offs_t offset, uint8_t data)
{
	if (offset)
		m_dma_address = (m_dma_address & 0x00ff) | (uint16_t(data) << 8);
	else
		m_dma_address = (m_dma_address & 0xff00) | data;
}

void mshark_device::start_sector()
{
	m_controller_status = 0;
	m_sequence = 0;
	m_transfer_count = 0;
	m_transfer_mode = BIT(m_disk_pc, 2, 2);
	if (!drive_selected() || !BIT(m_drive_status, 0))
		return;

	uint8_t const head = BIT(~m_drive_select, 4, 3);
	uint16_t const cylinder = (uint16_t(m_record_id[0] & 7) << 8) | m_record_id[1];
	if (head >= HEADS || m_record_id[2] >= SECTORS || cylinder != m_current_cylinder || BIT(m_record_id[0], 4, 3) != head)
	{
		m_controller_status = 0x20;
		return;
	}
	m_sector_lba = (m_current_cylinder * HEADS + head) * SECTORS + m_record_id[2];
	std::fill(std::begin(m_sector_buffer), std::end(m_sector_buffer), 0);
	if (m_transfer_mode == 0 && !m_harddisk->read(m_sector_lba, m_sector_buffer))
	{
		m_controller_status = 0x22;
		return;
	}

	attotime const now = machine().time();
	uint64_t const revolution = now.as_ticks(60);
	attotime sector = attotime::from_ticks(revolution * SECTORS + m_record_id[2], 60 * SECTORS);
	if (sector < now)
		sector += attotime::from_ticks(SECTORS, 60 * SECTORS);
	m_sector_timer->adjust(sector - now);
}

TIMER_CALLBACK_MEMBER(mshark_device::sector_tick)
{
	if (!drive_selected() || !BIT(m_drive_status, 0))
	{
		m_controller_status = 0x20;
		return;
	}
	uint8_t const control = m_micro[(uint16_t(m_transfer_mode) << 9) | m_sequence];
	if (!control || m_sequence == 0x1ff)
	{
		m_controller_status = 0x28;
		if (m_transfer_mode == 1 || m_transfer_mode == 2)
		{
			unsigned const offset = (m_transfer_mode == 2) ? 3 : 0;
			if (m_transfer_count != (SECTOR_SIZE + offset) || (offset && !std::equal(std::begin(m_record_id), std::end(m_record_id), m_sector_buffer)))
				m_controller_status = 0x20;
			else if (m_harddisk->is_readonly() || !m_harddisk->write(m_sector_lba, m_sector_buffer + offset))
			{
				m_drive_status = (m_drive_status & ~0x01) | 0x20;
				m_controller_status = 0x30;
			}
		}
		return;
	}
	if (BIT(control, 3) && !BIT(control, 1))
	{
		if (m_transfer_count >= std::size(m_sector_buffer))
		{
			m_controller_status = 0x20;
			return;
		}
		if (BIT(control, 6))
			m_sector_buffer[m_transfer_count] = m_program->read_byte(m_dma_address++);
		else
			m_program->write_byte(m_dma_address++, m_sector_buffer[m_transfer_count]);
		m_transfer_count++;
	}
	m_sequence++;
	m_sector_timer->adjust(attotime::from_nsec(1'240));
}

void mshark_device::disk_pc_w(uint8_t data)
{
	uint8_t const previous = m_disk_pc;
	m_disk_pc = data;
	update_ieee_signals();
	if (!BIT(data, 0) && BIT(previous, 0) && drive_selected())
	{
		m_drive_timer->adjust(attotime::never);
		m_drive_status &= 0x49;
	}
	if (BIT(data, 4))
		m_sector_timer->adjust(attotime::never);
	else if (BIT(previous, 4))
		start_sector();
	if (BIT(data, 5) && !BIT(previous, 5))
		m_controller_status = 0;
}

void mshark_device::update_ieee_signals()
{
	bool const atna = m_atn == bool(BIT(m_disk_pc, 7));
	m_bus->nrfd_w(this, BIT(m_ieee_pc, 1) && atna);
	m_bus->ndac_w(this, BIT(m_ieee_pc, 2) && atna);
}

uint8_t mshark_device::ieee_pc_r()
{
	return (m_bus->dav_r() << 4) | (m_bus->nrfd_r() << 5) | (m_bus->ndac_r() << 6) | (m_bus->eoi_r() << 7);
}

void mshark_device::ieee_pc_w(uint8_t data)
{
	m_ieee_pc = data;
	m_bus->dav_w(this, BIT(data, 0));
	m_bus->eoi_w(this, BIT(data, 3));
	update_ieee_signals();
}

uint8_t mshark_device::switches_r()
{
	return (~m_slot->get_address() & 0x0f) | (m_bus->ifc_r() << 4) | (m_bus->atn_r() << 5) | (m_bus->ren_r() << 6) | 0x80;
}

void mshark_device::update_interrupt()
{
	m_maincpu->set_input_line(I8085_RST75_LINE, !m_atn || !m_ifc);
}

void mshark_device::ieee488_atn(int state)
{
	m_atn = state;
	update_ieee_signals();
	update_interrupt();
}

void mshark_device::ieee488_ifc(int state)
{
	m_ifc = state;
	update_interrupt();
}

} // anonymous namespace



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(GPIB_MSHARK, device_ieee488_interface, mshark_device, "gpib_mshark", "Mator SHARK")
