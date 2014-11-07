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
	AM_RANGE(0xfc, 0xff) AM_READWRITE  (unk_fc_r,    unk_fc_w)

	AM_INHERIT_FROM(pci_device::config_map)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(internal_io_map, 32, i6300esb_lpc_device)
	if(lpc_en & 0x2000) {
		AM_RANGE(0x004c, 0x004f) AM_READWRITE8(siu_config_port_r, siu_config_port_w, 0x00ff0000)
		AM_RANGE(0x004c, 0x004f) AM_READWRITE8(siu_data_port_r,   siu_data_port_w,   0xff000000)
	}

	AM_RANGE(0x80, 0x83) AM_WRITE8(                  nop_w,       0x000000ff) // POST/non-existing, used for delays by the bios/os
	AM_RANGE(0xec, 0xef) AM_WRITE8(                  nop_w,       0x0000ff00) // Non-existing, used for delays by the bios/os
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
	siu_config_port = 0;
	siu_config_state = 0;
}

void i6300esb_lpc_device::reset_all_mappings()
{
	gpio_base = 0;
	gpio_cntl = 0x00;
	lpc_if_com_range = 0x00;
	lpc_if_fdd_lpt_range = 0x00;
	lpc_if_sound_range = 0x00;
	fwh_dec_en1 = 0xff;
	gen1_dec = 0x0000;
	lpc_en = 0x2000;
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

READ8_MEMBER  (i6300esb_lpc_device::lpc_if_com_range_r)
{
	return lpc_if_com_range;
}

WRITE8_MEMBER (i6300esb_lpc_device::lpc_if_com_range_w)
{
	COMBINE_DATA(&lpc_if_com_range);
	logerror("%s: lpc_if_com_range  = %02x\n", tag(), lpc_if_com_range);
}

READ8_MEMBER  (i6300esb_lpc_device::lpc_if_fdd_lpt_range_r)
{
	return lpc_if_fdd_lpt_range;
}

WRITE8_MEMBER (i6300esb_lpc_device::lpc_if_fdd_lpt_range_w)
{
	COMBINE_DATA(&lpc_if_fdd_lpt_range);
	logerror("%s: lpc_if_fdd_lpt_range  = %02x\n", tag(), lpc_if_fdd_lpt_range);
}

READ8_MEMBER  (i6300esb_lpc_device::lpc_if_sound_range_r)
{
	return lpc_if_sound_range;
}

WRITE8_MEMBER (i6300esb_lpc_device::lpc_if_sound_range_w)
{
	COMBINE_DATA(&lpc_if_sound_range);
	logerror("%s: lpc_if_sound_range  = %02x\n", tag(), lpc_if_sound_range);
}

READ8_MEMBER  (i6300esb_lpc_device::fwh_dec_en1_r)
{
	return fwh_dec_en1;
}

WRITE8_MEMBER (i6300esb_lpc_device::fwh_dec_en1_w)
{
	fwh_dec_en1 = data | 0x80;
	logerror("%s: fwh_dec_en1  = %02x\n", tag(), fwh_dec_en1);
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

READ32_MEMBER (i6300esb_lpc_device::unk_fc_r)
{
	logerror("%s: read undocumented config reg fc\n", tag());
	return 0;
}

WRITE32_MEMBER(i6300esb_lpc_device::unk_fc_w)
{
	logerror("%s: write undocumented config reg fc (%08x)\n", tag(), data);
}



READ8_MEMBER  (i6300esb_lpc_device::siu_config_port_r)
{
	return siu_config_port;
}

WRITE8_MEMBER (i6300esb_lpc_device::siu_config_port_w)
{
	siu_config_port = data;
	switch(siu_config_state) {
	case 0:
		siu_config_state = data == 0x80 ? 1 : 0;
		break;
	case 1:
		siu_config_state = data == 0x86 ? 2 : data == 0x80 ? 1 : 0;
		if(siu_config_state == 2)
			logerror("%s: siu configuration active\n", tag());
		break;
	case 2:
		siu_config_state = data == 0x68 ? 3 : 2;
		break;
	case 3:
		siu_config_state = data == 0x08 ? 0 : data == 0x68 ? 3 : 2;
		if(!siu_config_state)
			logerror("%s: siu configuration disabled\n", tag());
		break;
	}
}

READ8_MEMBER  (i6300esb_lpc_device::siu_data_port_r)
{
	logerror("%s: siu config read port %02x\n", tag(), siu_config_port);
	return 0xff;
}

WRITE8_MEMBER (i6300esb_lpc_device::siu_data_port_w)
{
	if(siu_config_state < 2) {
		logerror("%s: siu config write port with config disabled (port=%02x, data=%02x)\n", tag(), siu_config_port, data);
		return;
	}
	logerror("%s: siu config write port %02x, %02x\n", tag(), siu_config_port, data);
}

WRITE8_MEMBER (i6300esb_lpc_device::nop_w)
{
}

void i6300esb_lpc_device::map_bios(address_space *memory_space, UINT32 start, UINT32 end, int idsel)
{
	// Ignore idsel, a16 inversion for now
	UINT32 mask = m_region->bytes() - 1;
	memory_space->install_rom(start, end, m_region->base() + (start & mask));	
}

void i6300esb_lpc_device::map_extra(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
									UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space)
{
	if(fwh_dec_en1 & 0x80) {
		map_bios(memory_space, 0xfff80000, 0xffffffff, 7);
		map_bios(memory_space, 0xffb80000, 0xffbfffff, 7);
		map_bios(memory_space, 0x000e0000, 0x000fffff, 7);
	}
	if(fwh_dec_en1 & 0x40) {
		map_bios(memory_space, 0xfff00000, 0xfff7ffff, 6);
		map_bios(memory_space, 0xffb00000, 0xffb7ffff, 6);
	}
	if(fwh_dec_en1 & 0x20) {
		map_bios(memory_space, 0xffe80000, 0xffefffff, 5);
		map_bios(memory_space, 0xffa80000, 0xffafffff, 5);
	}
	if(fwh_dec_en1 & 0x10) {
		map_bios(memory_space, 0xffe00000, 0xffe7ffff, 4);
		map_bios(memory_space, 0xffa00000, 0xffa7ffff, 4);
	}
	if(fwh_dec_en1 & 0x08) {
		map_bios(memory_space, 0xffd80000, 0xffdfffff, 3);
		map_bios(memory_space, 0xff980000, 0xff9fffff, 3);
	}
	if(fwh_dec_en1 & 0x04) {
		map_bios(memory_space, 0xffd00000, 0xffd7ffff, 2);
		map_bios(memory_space, 0xff900000, 0xff97ffff, 2);
	}
	if(fwh_dec_en1 & 0x02) {
		map_bios(memory_space, 0xffc80000, 0xffcfffff, 1);
		map_bios(memory_space, 0xff880000, 0xff8fffff, 1);
	}
	if(fwh_dec_en1 & 0x01) {
		map_bios(memory_space, 0xffc00000, 0xffc7ffff, 0);
		map_bios(memory_space, 0xff800000, 0xff87ffff, 0);
	}

	io_space->install_device(0, 0xffff, *this, &i6300esb_lpc_device::internal_io_map);
}

