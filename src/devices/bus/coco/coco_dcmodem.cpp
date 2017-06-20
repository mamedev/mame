// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

	coco_dcmodem.cpp

	Code for emulating the CoCo Direct Connect Modem PAK

	This is just a "skeleton device"; the UART is emulated but pretty much
	nothing else

***************************************************************************/

#include "emu.h"
#include "cococart.h"
#include "machine/mos6551.h"


/***************************************************************************
	CONSTANTS
***************************************************************************/

#define UART_TAG        "uart"


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
		coco_dc_modem_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
			: device_t(mconfig, COCO_DCMODEM, tag, owner, clock)
			, device_cococart_interface(mconfig, *this)
			, m_uart(*this, UART_TAG)
		{
		}

		// optional information overrides
		virtual void device_add_mconfig(machine_config &config) override;

		// callbacks
		WRITE_LINE_MEMBER(uart_irq_w)
		{
			set_line_value(line::CART, state != 0);
		}

	protected:
		// device-level overrides
		virtual void device_start() override
		{
			install_readwrite_handler(0xFF6C, 0xFF6F,
				read8_delegate(FUNC(mos6551_device::read), (mos6551_device *)m_uart),
				write8_delegate(FUNC(mos6551_device::write), (mos6551_device *)m_uart));
		}

		virtual const tiny_rom_entry *device_rom_region() const override;

		// CoCo cartridge level overrides
		virtual uint8_t *get_cart_base() override
		{
			return memregion("eprom")->base();
		}

	private:
		// internal state
		required_device<mos6551_device> m_uart;
	};
};


/***************************************************************************
	IMPLEMENTATION
***************************************************************************/

MACHINE_CONFIG_MEMBER(coco_dc_modem_device::device_add_mconfig)
	MCFG_DEVICE_ADD(UART_TAG, MOS6551, 0)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)
	MCFG_MOS6551_IRQ_HANDLER(WRITELINE(coco_dc_modem_device, uart_irq_w))
MACHINE_CONFIG_END


ROM_START(coco_dcmodem)
	ROM_REGION(0x2000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD("Direct Connect Modem Pak (1985) (26-2228) (Tandy).rom", 0x0000, 0x2000, CRC(667bc55d) SHA1(703fe0aba4a603591078cb675ffd26a67c02df88))
ROM_END

DEFINE_DEVICE_TYPE(COCO_DCMODEM, coco_dc_modem_device, "coco_dcmodem", "CoCo Direct Connect Modem PAK")


//-------------------------------------------------
//  device_rom_region
//-------------------------------------------------

const tiny_rom_entry *coco_dc_modem_device::device_rom_region() const
{
	return ROM_NAME(coco_dcmodem);
}


