// license:BSD-3-Clause
// copyright-holders:AJR

#include "emu.h"
#include "s1c33l27.h"

// device type definition
DEFINE_DEVICE_TYPE(S1C33L27, s1c33l27_device, "s1c33l27", "Epson S1C33L27")

// FIXME: based on C33 PE Core, not STD Core
s1c33l27_device::s1c33l27_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c33std_cpu_device_base(mconfig, S1C33L27, tag, owner, clock, address_map_constructor(FUNC(s1c33l27_device::internal_map), this))
{
}

void s1c33l27_device::device_reset()
{
	c33std_cpu_device_base::device_reset();

	// TODO: gate ROM depending on boot mode
	m_pc = m_data.read_dword(0x00c00000);
}

void s1c33l27_device::internal_map(address_map &map)
{
	map(0x00000000, 0x00004fff).ram();
	map(0x00008000, 0x0000ffff).ram();
	map(0x00084000, 0x000847ff).ram();
	// 0x00300000-0x003000ff Misc Registers (MISC)
	// 0x00300100-0x003001ff Clock Management Unit (CMU)
	// 0x00300200-0x003002ff Interrupt Controller (ITC)
	// 0x00300300-0x003003ff I/O Ports (GPIO)
	// 0x00300400-0x003005ff Universal Serial Interface Ch.0, Ch.1 (USI)
	// 0x00300600-0x003006ff Universal Serial Interface with LCD Interface (USIL)
	// 0x00300700-0x003007ff Universal Serial Interface Ch.2 (USI)
	// 0x00300800-0x003008ff Port MUX (PMUX)
	// 0x00300900-0x003009ff Host Interface (HIF)
	// 0x00300a00-0x00300aff Real-time Clock (RTC)
	// 0x00300b00-0x00300bff BBRAM
	// 0x00300c00-0x00300dff USB Function Controller
	// 0x00300e00-0x00300e0f Prescaler (PSC)
	// 0x00300e10-0x00300eff UART
	// 0x00300f00-0x00300fff Card Interface (CARD)
	// 0x00301000-0x003010ff Watchdog Timer (WDT)
	// 0x00301100-0x0030115f Fine Mode 8-bit Timer Ch.0-Ch.5 (T8F)
	// 0x00301160-0x003011ff 16-bit PWM Timer Ch.0-Ch.3 (T16A6)
	// 0x00301200-0x003012ff 16-bit Audio PWM Timer Ch.0, Ch.1 (T16P)
	// 0x00301300-0x003013ff A/D Converter
	// 0x00301500-0x003015ff Remote Controller (REMC)
	// 0x00302000-0x003020ff LCD Controller
	// 0x00302100-0x003021ff DMA Controller (DMAC)
	// 0x00302200-0x0030221f SDRAM Controller (SDRAMC)
	// 0x00302220-0x003022ff SRAM Controller (SRAMC)
	// 0x00302300-0x003023ff Cache Controller
	// 0x00302400-0x003024ff I²S Bus Interface
	// 0x00302600-0x003026ff SD/MMC Interface (SD_MMC)
}
