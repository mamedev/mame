// license:BSD-3-Clause
// copyright-holders:68bit
/**********************************************************************

    PIA IDE Hard Disk Interface

    Note this interface does appear to have used an 8 bit IDE data path, and
    does appear to have discarded every second byte of storage - this does not
    appear to be mis-implementation here. FLEX uses 256 byte sectors so half
    of the IDE 512 byte sector give the required fit.

    Some emulators appear to have stripped out that second unused byte when
    writing their disk images. So their disk images will need to be filled
    when importing these images for use with this device. Also some emulators
    appear tolerant of trailing unused data and their images might need to be
    truncated to the length defined by the disk geomentry.

    The FLEX drivers for the PIA IDE appear to translate FLEX virtual track
    and sector ID's into a target geometry. The disk geometry will likely need
    to be defined and that can be done by using the MAME CHD disk format. The
    range of FLEX tracks and sectors is limited to 0 to 255 so it needs to use
    that full virtual range get the largest usable partitions. The drivers
    support multiple partitions that appears as multiple disks to FLEX to make
    use of more storage.

    A similar (compatible) interface appears to have been used by the 'PT69'
    SBC and drivers and disks targeting the PT69 have also run with this
    device. The 'PT69' appears to place the PIA at 0xe010 to 0xe013 and it's
    supporting ROM based at 0xf000, in contrast to the MAME swtpc09i machine
    which places the PIA IDE in IO6 at 0xe060 (so that the DC5 FDC can remain
    at IO1) and places the supporting ROM at $e800. The MAME swtpc09i machine
    has booted some PT69 disk images with some minor software changes.

**********************************************************************/

#include "emu.h"
#include "piaide.h"

#include "machine/6821pia.h"
#include "machine/idectrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ss50_piaide_device

class ss50_piaide_device : public device_t, public ss50_card_interface
{
public:
	// construction/destruction
	ss50_piaide_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, SS50_PIAIDE, tag, owner, clock)
		, ss50_card_interface(mconfig, *this)
		, m_pia(*this, "pia")
		, m_ide(*this, "ide")
	{
	}

protected:
	// device-specific overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// interface-specific overrides
	virtual uint8_t register_read(offs_t offset) override;
	virtual void register_write(offs_t offset, uint8_t data) override;

private:
	optional_device<pia6821_device> m_pia;
	optional_device<ide_controller_device> m_ide;

	uint8_t m_pia_porta;
	uint8_t m_pia_portb;

	uint8_t pia_a_r();
	uint8_t pia_b_r();
	void pia_a_w(uint8_t data);
	void pia_b_w(uint8_t data);

};

void ss50_piaide_device::device_start()
{
	save_item(NAME(m_pia_porta));
	save_item(NAME(m_pia_portb));
}

//-------------------------------------------------
//  device_add_mconfig - add device-specific
//  machine configuration
//-------------------------------------------------

void ss50_piaide_device::device_add_mconfig(machine_config &config)
{
	PIA6821(config, m_pia);
	m_pia->readpa_handler().set(FUNC(ss50_piaide_device::pia_a_r));
	m_pia->readpb_handler().set(FUNC(ss50_piaide_device::pia_b_r));
	m_pia->writepa_handler().set(FUNC(ss50_piaide_device::pia_a_w));
	m_pia->writepb_handler().set(FUNC(ss50_piaide_device::pia_b_w));

	IDE_CONTROLLER(config, m_ide).options(ata_devices, "hdd", nullptr, false);
}

//-------------------------------------------------
//  register_read - read from a port register
//-------------------------------------------------

uint8_t ss50_piaide_device::register_read(offs_t offset)
{
	return m_pia->read(offset & 3);
}

//-------------------------------------------------
//  register_write - write to a port register
//-------------------------------------------------

void ss50_piaide_device::register_write(offs_t offset, uint8_t data)
{
	m_pia->write(offset & 3, data);
}

/******* MC6821 PIA on IDE Board *******/
/* Read/Write handlers for pia ide */

uint8_t ss50_piaide_device::pia_a_r()
{
	return m_pia_porta;
}

uint8_t ss50_piaide_device::pia_b_r()
{
	return m_pia_portb;
}

void ss50_piaide_device::pia_a_w(uint8_t data)
{
	m_pia_porta = data;
}

void ss50_piaide_device::pia_b_w(uint8_t data)
{
	uint16_t tempidedata;

	m_pia_portb = data;

	// Pass through a 16 bit accessor to the IDE bus.
	uint8_t low = (data & 0x04) >> 2;
	uint8_t high = (data & 0x18) >> 3;
	uint16_t ide_mem_mask = low ? 0xff00 : 0x00ff;

	if ((data & 0x40)&&(!(data&0x20)))  //cs0=0 cs1=1 bit 5&6
	{
		if (!(data & 0x02))  //rd line bit 1
		{
			tempidedata = m_ide->read_cs0(high, ide_mem_mask);
			logerror("ide_bus_r: offset $%02X data %04X\n", (data&0x1c)>>2, tempidedata);
			m_pia_porta = low ? tempidedata >> 8 : tempidedata & 0x00ff;
		}
		else if (!(data & 0x01))  //wr line bit 0
		{
			uint16_t ide_data = low ? m_pia_porta << 8: m_pia_porta;
			m_ide->write_cs0(high, ide_data, ide_mem_mask);
			logerror("ide_bus_w: offset $%02X data %04X\n", (data&0x1c)>>2, m_pia_porta);
		}
	}
	else if ((data & 0x20)&&(!(data&0x40)))  //cs0=1 cs1=0 bit 5&6
	{
		if (!(data & 0x02))  //rd line bit 1
		{
			tempidedata = m_ide->read_cs1(high, ide_mem_mask);
			logerror("ide_bus_r: offset $%02X data %04X\n", (data&0x1c)>>2, tempidedata);
			m_pia_porta = low ? tempidedata >> 8 : tempidedata & 0x00ff;
		}
		else if (!(data & 0x01))  //wr line bit 0
		{
			uint16_t ide_data = low ? m_pia_porta << 8: m_pia_porta;
			m_ide->write_cs1(high, ide_data, ide_mem_mask);
			logerror("ide_bus_w: offset $%02X data %04X\n", (data&0x1c)>>2, m_pia_porta);
		}
	}
}

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(SS50_PIAIDE, ss50_card_interface, ss50_piaide_device, "ss50_piaide", "PIA IDE Hard Disk Interface")
