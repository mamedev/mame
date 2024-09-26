// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco_dcmodem.cpp

    Code for emulating the CoCo Direct Connect Modem PAK

***************************************************************************/

#include "emu.h"
#include "coco_dcmodem.h"

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

// ======================> coco_dc_modem_device

namespace
{
	class coco_dc_modem_device :
		public device_t,
		public device_cococart_interface
	{
	public:
		// construction/destruction
		coco_dc_modem_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
			: device_t(mconfig, COCO_DCMODEM, tag, owner, clock)
			, device_cococart_interface(mconfig, *this)
			, m_uart(*this, UART_TAG)
			, m_eprom(*this, "eprom")
		{
		}

		// optional information overrides
		virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

		// callbacks
		void uart_irq_w(int state)
		{
			set_line_value(line::CART, state != 0);
		}

	protected:
		// device-level overrides
		virtual void device_start() override
		{
			install_readwrite_handler(0xFF6C, 0xFF6F,
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
		required_device<mos6551_device> m_uart;
		required_memory_region m_eprom;
	};
};


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

void coco_dc_modem_device::device_add_mconfig(machine_config &config)
{
	MOS6551(config, m_uart, 0);
	m_uart->set_xtal(1.8432_MHz_XTAL);
	m_uart->irq_handler().set(FUNC(coco_dc_modem_device::uart_irq_w));
	m_uart->txd_handler().set(PORT_TAG, FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232(RS232_PORT(config, PORT_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_uart, FUNC(mos6551_device::write_rxd));
	rs232.dcd_handler().set(m_uart, FUNC(mos6551_device::write_dcd));
	rs232.dsr_handler().set(m_uart, FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set(m_uart, FUNC(mos6551_device::write_cts));
}


ROM_START(coco_dcmodem)
	ROM_REGION(0x2000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD("direct connect modem pak,1985,26-2228,tandy.rom", 0x0000, 0x2000, CRC(667bc55d) SHA1(703fe0aba4a603591078cb675ffd26a67c02df88))
ROM_END

DEFINE_DEVICE_TYPE_PRIVATE(COCO_DCMODEM, device_cococart_interface, coco_dc_modem_device, "coco_dcmodem", "CoCo Direct Connect Modem PAK")


//-------------------------------------------------
//  device_rom_region
//-------------------------------------------------

const tiny_rom_entry *coco_dc_modem_device::device_rom_region() const
{
	return ROM_NAME(coco_dcmodem);
}

//-------------------------------------------------
//  cts_read
//-------------------------------------------------

u8 coco_dc_modem_device::cts_read(offs_t offset)
{
	return m_eprom->base()[offset & 0x1fff];
}

