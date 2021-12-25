// license:BSD-3-Clause
// copyright-holders:kmg
/**********************************************************************

    Nintendo Family Computer ASCII Turbofile SRAM external storage device

    The original 8K Turbofile, emulated here, has been seen with serials
    AS-TF02 through AS-TF04, differences unknown. We do not support the
    Turbo File II (yet?), but it appears to simply be a 32K version with
    a manual switch to select between four 8K banks. Both devices also
    sport a write protect switch.

**********************************************************************/

#include "emu.h"
#include "turbofile.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_TURBOFILE, nes_turbofile_device, "nes_turbofile", "ASCII Turbofile Backup RAM")


INPUT_CHANGED_MEMBER( nes_turbofile_device::lock_changed )
{
	m_locked = newval;
	popmessage("Turbofile Protect Switch %s\n", m_locked ? "ON" : "OFF");
}


static INPUT_PORTS_START( nes_turbofile )
	PORT_START("LOCK")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Protect Switch") PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, nes_turbofile_device, lock_changed, 0)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_turbofile_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_turbofile );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nes_turbofile_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_turbofile_device - constructor
//-------------------------------------------------

nes_turbofile_device::nes_turbofile_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NES_TURBOFILE, tag, owner, clock)
	, device_nes_control_port_interface(mconfig, *this)
	, m_nvram(*this, "nvram")
	, m_lock(*this, "LOCK")
	, m_addr(0)
	, m_bit(0)
	, m_latch(0)
	, m_locked(0)
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_turbofile_device::device_start()
{
	// 8K RAM
	m_ram = std::make_unique<u8[]>(0x2000);
	m_nvram->set_base(m_ram.get(), 0x2000);

	save_pointer(NAME(m_ram), 0x2000);
	save_item(NAME(m_addr));
	save_item(NAME(m_bit));
	save_item(NAME(m_latch));
	save_item(NAME(m_locked));
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void nes_turbofile_device::device_reset()
{
	m_locked = m_lock->read();
}


//-------------------------------------------------
//  read
//-------------------------------------------------

u8 nes_turbofile_device::read_exp(offs_t offset)
{
	return (offset == 1) ? BIT(m_ram[m_addr], m_bit) << 2 : 0;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_turbofile_device::write(u8 data)
{
	if (!BIT(data, 1))    // reset position to start of RAM
	{
		m_addr = 0;
		m_bit = 0;
	}

	if (!BIT(data, 2) && BIT(m_latch, 2))
	{
		if (!m_locked)
		{
			m_ram[m_addr] &= ~(1 << m_bit);
			m_ram[m_addr] |= (data & 1) << m_bit;
		}
		m_bit = (m_bit + 1) & 0x07;
		if (!m_bit)
			m_addr = (m_addr + 1) & 0x1fff;
	}

	m_latch = data;
}
