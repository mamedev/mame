// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

PC-8801 S[oftware]DIP interface

93C06 EEPROM hooked up in a way to replace reading of physical dip-switches in BIOS software.
Clearly an ancestor of PC-98 SDIP device.

SRAM contents:
[0]
--x- ---- Duplex Half/Full (DSW2 bit 5)
---x ---- Enable X parameter (DSW2 bit 4)
---- x--- Stop bit (DSW2 bit 3)
---- -x-- Serial char length (DSW2 bit 2)
---- --x- Parity Type even/odd (DSW2 bit 1)
---- ---x Parity Generate (DSW2 bit 0)

[1]
---- xxxx Baud rate (set by BIOS at startup to port $6f)

[2]
-x-- ---- Memory weight (DSW1 bit 6)

[3]
x--- ---- Built-in FDD i/f (?, the other setting is <prohibited> anyway)
-x-- ---- Auto-boot floppy (CTRL bit 3)
--x- ---- Enable DEL code (DSW1 bit 5)
---x ---- Enable S parameter (DSW1 bit 4)
---- x--- Text Height (DSW1 bit 3)
---- -x-- Text Width (DSW1 bit 2)
---- --x- Terminal Mode '1' Basic '0' (same as DSW1 bit 1)

TODO:
- define aliases for MA2/MC (CPU clock switch and BASIC mode);
- "SDIP" as name is unconfirmed;

**************************************************************************************************/

#include "emu.h"
#include "pc88_sdip.h"

DEFINE_DEVICE_TYPE(PC88_SDIP, pc88_sdip_device, "pc88_sdip", "NEC PC-88 SDIP device (93C06 serial EEPROM)")

pc88_sdip_device::pc88_sdip_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: eeprom_serial_93c06_16bit_device(mconfig, PC88_SDIP, tag, owner, clock)
{
}

ioport_value pc88_sdip_device::dsw1_r() { return (m_data[3] & 0x3e) >> 1; }
ioport_value pc88_sdip_device::dsw2_r() { return m_data[0] & 0x3f; }
ioport_value pc88_sdip_device::auto_boot_floppy_r() { return BIT(m_data[3], 6); }
//ioport_value pc88_sdip_device::built_in_fdd_r() { return BIT(m_data[3], 7); }
ioport_value pc88_sdip_device::memory_weight_r() { return BIT(m_data[2], 6); }
