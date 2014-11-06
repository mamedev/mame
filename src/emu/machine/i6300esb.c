#include "i6300esb.h"

const device_type I6300ESB_WATCHDOG = &device_creator<i6300esb_watchdog_device>;
const device_type I6300ESB_LPC      = &device_creator<i6300esb_lpc_device>;

DEVICE_ADDRESS_MAP_START(map, 32, i6300esb_watchdog_device)
ADDRESS_MAP_END

i6300esb_watchdog_device::i6300esb_watchdog_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, I6300ESB_WATCHDOG, "i6300ESB southbridge watchdog", tag, owner, clock, "i6300esb_watchdog", __FILE__)
{
}

void i6300esb_watchdog_device::device_start()
{
	pci_device::device_start();
	add_map(16, M_MEM, FUNC(i6300esb_watchdog_device::map));
}

void i6300esb_watchdog_device::device_reset()
{
	pci_device::device_reset();
}


DEVICE_ADDRESS_MAP_START(config_map, 32, i6300esb_lpc_device)
	AM_RANGE(0x58, 0x5b) AM_READWRITE  (gpio_base_r, gpio_base_w)
	AM_RANGE(0x5c, 0x5f) AM_READWRITE8 (gpio_cntl_r, gpio_cntl_w, 0x000000ff)
	AM_RANGE(0xe4, 0xe7) AM_READWRITE16(gen1_dec_r,  gen1_dec_w,  0x0000ffff)
	AM_RANGE(0xe4, 0xe7) AM_READWRITE16(lpc_en_r,    lpc_en_w,    0xffff0000)
	AM_RANGE(0xe8, 0xeb) AM_READWRITE  (fwh_sel1_r,  fwh_sel1_w)

	AM_INHERIT_FROM(pci_device::config_map)
ADDRESS_MAP_END


i6300esb_lpc_device::i6300esb_lpc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, I6300ESB_LPC, "i6300ESB southbridge ISA/LPC bridge", tag, owner, clock, "i6300esb_lpc", __FILE__)
{
}

void i6300esb_lpc_device::device_start()
{
	pci_device::device_start();
}

void i6300esb_lpc_device::device_reset()
{
	pci_device::device_reset();
	gpio_base = 0;
	gpio_cntl = 0x00;
	gen1_dec = 0x0000;
	lpc_en = 0x0000;
	fwh_sel1 = 0x00112233;
}

READ32_MEMBER (i6300esb_lpc_device::gpio_base_r)
{
	return gpio_base | 1;
}

WRITE32_MEMBER(i6300esb_lpc_device::gpio_base_w)
{
	COMBINE_DATA(&gpio_base);
	gpio_base &= 0x0000ffc0;
	logerror("%s: gpio_base = %08x\n", tag(), gpio_base);
}

READ8_MEMBER  (i6300esb_lpc_device::gpio_cntl_r)
{
	return gpio_cntl;
}

WRITE8_MEMBER (i6300esb_lpc_device::gpio_cntl_w)
{
	COMBINE_DATA(&gpio_cntl);
	logerror("%s: gpio_cntl = %02x\n", tag(), gpio_cntl);
}

READ16_MEMBER (i6300esb_lpc_device::gen1_dec_r)
{
	return gen1_dec;
}

WRITE16_MEMBER(i6300esb_lpc_device::gen1_dec_w)
{
	COMBINE_DATA(&gen1_dec);
	logerror("%s: gen1_dec = %04x\n", tag(), gen1_dec);
}

READ16_MEMBER (i6300esb_lpc_device::lpc_en_r)
{
	return lpc_en;
}

WRITE16_MEMBER(i6300esb_lpc_device::lpc_en_w)
{
	COMBINE_DATA(&lpc_en);
	logerror("%s: lpc_en = %04x\n", tag(), lpc_en);
}

READ32_MEMBER (i6300esb_lpc_device::fwh_sel1_r)
{
	return fwh_sel1;
}

WRITE32_MEMBER(i6300esb_lpc_device::fwh_sel1_w)
{
	COMBINE_DATA(&fwh_sel1);
	logerror("%s: fwh_sel1 = %08x\n", tag(), fwh_sel1);
}

void i6300esb_lpc_device::map_extra(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
									UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space)
{
	memory_space->install_rom(0xfff00000, 0xffffffff, m_region->base());
	memory_space->install_rom(0x000f0000, 0x000fffff, m_region->base()+0xf0000);
}

