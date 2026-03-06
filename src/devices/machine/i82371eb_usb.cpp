// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

PIIX4E USB interface (UHCI)

TODO:
- Actual USB ports;
- SMI traps for legacy support;
- devcb for RTCIREN in PIIX4 ISA, cfr. specification update;

**************************************************************************************************/

#include "emu.h"
#include "i82371eb_usb.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

DEFINE_DEVICE_TYPE(I82371EB_USB, i82371eb_usb_device, "i82371eb_usb", "Intel 82371EB PIIX4E USB Host Controller")

// rev PIIX4E A-0 / PIIX4M A-0 = 0x01
// subvendor ID not given (assume blank)
// PIRQD# (0x04)
i82371eb_usb_device::i82371eb_usb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: usb_uhci_device(mconfig, I82371EB_USB, tag, owner, clock)
{
	set_ids(0x80867112, 0x01, 0x0c0300, 0x00000000);
	intr_pin = 0x04;
}

void i82371eb_usb_device::device_add_mconfig(machine_config &config)
{
	// 2 USB ports
}

void i82371eb_usb_device::device_start()
{
	usb_uhci_device::device_start();

	save_item(NAME(m_legsup));
	save_item(NAME(m_rtciren));
}


void i82371eb_usb_device::device_reset()
{
	usb_uhci_device::device_reset();

	command = 0x0000;
	// Can be bus master
	command_mask = 5;
	status = 0x0280;

	m_legsup = 0x2000;
	// indeterminate, may as well disable
	m_rtciren = false;
}

void i82371eb_usb_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	// sbrnum - serial bus release number (1.0), 0x00 for "pre-release" 1.0
	map(0x60, 0x60).lr8(NAME([] () { return 0x10; }));
	// legsup - Legacy Support Register
	map(0xc0, 0xc1).lrw16(
		NAME([this] () { return m_legsup; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			LOG("LEGSUP: %04x & %04x\n", data, mem_mask);
			if (ACCESSING_BITS_8_15)
			{
				// A20PTS
				if (BIT(data, 15))
					m_legsup &= ~(1 << 15);
				// USBPIRQDEN
				m_legsup &= ~(1 << 13);
				m_legsup |= (BIT(data, 13) << 13);
				// bit 12 (r/o) USBIRQS
				// TBY64W
				if (BIT(data, 11))
					m_legsup &= ~(1 << 11);
				// TBY64R
				if (BIT(data, 10))
					m_legsup &= ~(1 << 10);
				// TBY60W
				if (BIT(data, 9))
					m_legsup &= ~(1 << 9);
				// TBY60R
				if (BIT(data, 8))
					m_legsup &= ~(1 << 8);

				LOG("USBPIRQDEN %d ", BIT(data, 13));
			}
			if (ACCESSING_BITS_0_7)
			{
				// bit 6 PSS (r/o)
				m_legsup &= ~(0xbf);
				m_legsup |= (data & 0xbf);

				LOG("SMIEPTE %d A20PTEN %d USBSMIEN %d 64WEN %d 64REN %d 60WEN %d 60REN %d"
					, BIT(data, 7)
					, BIT(data, 5)
					, BIT(data, 4)
					, BIT(data, 3)
					, BIT(data, 2)
					, BIT(data, 1)
					, BIT(data, 0)
				);
			}
			LOG("\n");
		})
	);
	// MISCSUP
	// NOTE: byte access only
	map(0xff, 0xff).lrw8(
		NAME([this] () { return m_rtciren << 4; }),
		NAME([this] (offs_t offset, u8 data) {
			m_rtciren = !!BIT(data, 4);
			LOG("MISCSUP: %02x\n", data);
		})
	);
}

void i82371eb_usb_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
//  io_space->install_device(0, 0x03ff, *this, &i82371eb_usb_device::io_map);
}


