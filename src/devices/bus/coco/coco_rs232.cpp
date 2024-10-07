// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco_rs232.cpp

    Code for emulating the CoCo Deluxe RS-232 PAK

***************************************************************************/

#include "emu.h"
#include "coco_rs232.h"

#include "machine/mos6551.h"
#include "bus/rs232/rs232.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define UART_TAG        "uart"
#define PORT_TAG        "port"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> coco_rs232_device

namespace
{
	class coco_rs232_device :
			public device_t,
			public device_cococart_interface
	{
	public:
		// construction/destruction
		coco_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
			: device_t(mconfig, COCO_RS232, tag, owner, clock)
			, device_cococart_interface(mconfig, *this)
			, m_eprom(*this, "eprom")
			, m_uart(*this, UART_TAG)
		{
		}

		// optional information overrides
		virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

		void uart_irq_w(int state)
		{
			set_line_value(line::CART, state != 0);
		}

	protected:
		// device-level overrides
		virtual void device_start() override
		{
			install_readwrite_handler(0xFF68, 0xFF6B,
					read8sm_delegate(*m_uart, FUNC(mos6551_device::read)),
					write8sm_delegate(*m_uart, FUNC(mos6551_device::write)));
		}

		virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

		// CoCo cartridge level overrides
		virtual u8 *get_cart_base() override
		{
			return m_eprom->base();
		}

		virtual memory_region *get_cart_memregion() override
		{
			return m_eprom;
		}

		virtual u8 cts_read(offs_t offset) override;

	private:
		// internal state
		required_memory_region m_eprom;
		required_device<mos6551_device> m_uart;
	};
};


/***************************************************************************
IMPLEMENTATION
***************************************************************************/

void coco_rs232_device::device_add_mconfig(machine_config &config)
{
	MOS6551(config, m_uart, 0);
	m_uart->set_xtal(1.8432_MHz_XTAL);
	m_uart->irq_handler().set(FUNC(coco_rs232_device::uart_irq_w));
	m_uart->txd_handler().set(PORT_TAG, FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232(RS232_PORT(config, PORT_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_uart, FUNC(mos6551_device::write_rxd));
	rs232.dcd_handler().set(m_uart, FUNC(mos6551_device::write_dcd));
	rs232.dsr_handler().set(m_uart, FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set(m_uart, FUNC(mos6551_device::write_cts));
}

ROM_START(coco_rs232_device)
	ROM_REGION(0x1000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD("deluxe_rs-232_program_pak_1983_26-2226_tandy.rom", 0x0000, 0x1000, CRC(d990e1f9) SHA1(3fad25f3462a0b581b9c182ac11ad90c8fa08cb6))
ROM_END

//-------------------------------------------------
//  device_rom_region
//-------------------------------------------------

const tiny_rom_entry *coco_rs232_device::device_rom_region() const
{
	return ROM_NAME(coco_rs232_device);
}

//-------------------------------------------------
//  cts_read
//-------------------------------------------------

u8 coco_rs232_device::cts_read(offs_t offset)
{
	return m_eprom->base()[offset & 0x0fff];
}

//**************************************************************************
//  DEVICE DECLARATION
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(COCO_RS232, device_cococart_interface, coco_rs232_device, "coco_rs232", "CoCo Deluxe RS-232 PAK")
