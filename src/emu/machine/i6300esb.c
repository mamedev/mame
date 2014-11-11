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
	command = 0x000f;
	command_mask = 0x0140;
	status = 0x0280;
}


DEVICE_ADDRESS_MAP_START(config_map, 32, i6300esb_lpc_device)
	AM_RANGE(0x40, 0x43) AM_READWRITE  (pmbase_r,               pmbase_w)
	AM_RANGE(0x44, 0x47) AM_READWRITE8 (acpi_cntl_r,            acpi_cntl_w,            0x000000ff)
	AM_RANGE(0x4c, 0x4f) AM_READWRITE16(bios_cntl_r,            bios_cntl_w,            0xffff0000)
	AM_RANGE(0x54, 0x57) AM_READWRITE8 (tco_cntl_r,             tco_cntl_w,             0x000000ff)
	AM_RANGE(0x58, 0x5b) AM_READWRITE  (gpio_base_r,            gpio_base_w)
	AM_RANGE(0x5c, 0x5f) AM_READWRITE8 (gpio_cntl_r,            gpio_cntl_w,            0x000000ff)
	AM_RANGE(0x60, 0x63) AM_READWRITE8 (pirq_rout_r,            pirq_rout_w,            0xffffffff)
	AM_RANGE(0x64, 0x67) AM_READWRITE8 (serirq_cntl_r,          serirq_cntl_w,          0x000000ff)
	AM_RANGE(0x68, 0x6b) AM_READWRITE8 (pirq2_rout_r,           pirq2_rout_w,           0xffffffff)
	AM_RANGE(0x88, 0x8b) AM_READWRITE8 (d31_err_cfg_r,          d31_err_cfg_w,          0x000000ff)
	AM_RANGE(0x88, 0x8b) AM_READWRITE8 (d31_err_sts_r,          d31_err_sts_w,          0x00ff0000)
	AM_RANGE(0x90, 0x93) AM_READWRITE16(pci_dma_cfg_r,          pci_dma_cfg_w,          0x0000ffff)
	AM_RANGE(0xd0, 0xd3) AM_READWRITE  (gen_cntl_r,             gen_cntl_w)
	AM_RANGE(0xd4, 0xd7) AM_READWRITE8 (gen_sta_r,              gen_sta_w,              0x000000ff)
	AM_RANGE(0xd4, 0xd7) AM_READWRITE8 (back_cntl_r,            back_cntl_w,            0x0000ff00)
	AM_RANGE(0xd8, 0xdb) AM_READWRITE8 (rtc_conf_r,             rtc_conf_w,             0x000000ff)
	AM_RANGE(0xe0, 0xe3) AM_READWRITE8 (lpc_if_com_range_r,     lpc_if_com_range_w,     0x000000ff)
	AM_RANGE(0xe0, 0xe3) AM_READWRITE8 (lpc_if_fdd_lpt_range_r, lpc_if_fdd_lpt_range_w, 0x0000ff00)
	AM_RANGE(0xe0, 0xe3) AM_READWRITE8 (lpc_if_sound_range_r,   lpc_if_sound_range_w,   0x00ff0000)
	AM_RANGE(0xe0, 0xe3) AM_READWRITE8 (fwh_dec_en1_r,          fwh_dec_en1_w,          0xff000000)
	AM_RANGE(0xe4, 0xe7) AM_READWRITE16(gen1_dec_r,             gen1_dec_w,             0x0000ffff)
	AM_RANGE(0xe4, 0xe7) AM_READWRITE16(lpc_en_r,               lpc_en_w,               0xffff0000)
	AM_RANGE(0xe8, 0xeb) AM_READWRITE  (fwh_sel1_r,             fwh_sel1_w)
	AM_RANGE(0xec, 0xef) AM_READWRITE16(gen2_dec_r,             gen2_dec_w,             0x0000ffff)
	AM_RANGE(0xec, 0xef) AM_READWRITE16(fwh_sel2_r,             fwh_sel2_w,             0xffff0000)
	AM_RANGE(0xf0, 0xf3) AM_READWRITE8 (fwh_dec_en2_r,          fwh_dec_en2_w,          0x000000ff)
	AM_RANGE(0xf0, 0xf3) AM_READWRITE16(func_dis_r,             func_dis_w,             0xffff0000)
	AM_RANGE(0xf4, 0xf7) AM_READWRITE  (etr1_r,                 etr1_w)
	AM_RANGE(0xf8, 0xfb) AM_READ       (mfid_r)
	AM_RANGE(0xfc, 0xff) AM_READWRITE  (unk_fc_r,               unk_fc_w)

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
	tco_cntl = 0x00;
	serirq_cntl = 0x10;
	memset(pirq_rout, 0x80, sizeof(pirq_rout));
	d31_err_cfg = 0x00;
	d31_err_sts = 0x00;
	pci_dma_cfg = 0x0000;
	rtc_conf = 0x00;
	func_dis = 0x0080;
	etr1 = 0x00000000;
	siu_config_port = 0;
	siu_config_state = 0;
}

void i6300esb_lpc_device::reset_all_mappings()
{
	pci_device::reset_all_mappings();

	pmbase = 0;
	acpi_cntl = 0;
	gpio_base = 0;
	gpio_cntl = 0x00;
	back_cntl = 0x0f;
	lpc_if_com_range = 0x00;
	lpc_if_fdd_lpt_range = 0x00;
	lpc_if_sound_range = 0x00;
	fwh_dec_en1 = 0xff;
	fwh_dec_en2 = 0x0f;
	gen1_dec = 0x0000;
	lpc_en = 0x0000;
	fwh_sel1 = 0x00112233;
	gen_cntl = 0x00000080;
}

READ32_MEMBER (i6300esb_lpc_device::pmbase_r)
{
	return pmbase | 1;
}

WRITE32_MEMBER(i6300esb_lpc_device::pmbase_w)
{
	COMBINE_DATA(&pmbase);
	pmbase &= 0x0000ff80;
	logerror("%s: pmbase = %08x\n", tag(), pmbase);
	remap_cb();
}

READ8_MEMBER (i6300esb_lpc_device::acpi_cntl_r)
{
	return acpi_cntl;
}

WRITE8_MEMBER(i6300esb_lpc_device::acpi_cntl_w)
{
	acpi_cntl = data;
	logerror("%s: acpi_cntl = %08x\n", tag(), acpi_cntl);
	remap_cb();
}

READ16_MEMBER (i6300esb_lpc_device::bios_cntl_r)
{
	return pmbase | 1;
}

WRITE16_MEMBER(i6300esb_lpc_device::bios_cntl_w)
{
	COMBINE_DATA(&bios_cntl);
	logerror("%s: bios_cntl = %08x\n", tag(), bios_cntl);
	remap_cb();
}

READ8_MEMBER  (i6300esb_lpc_device::tco_cntl_r)
{
	return tco_cntl;
}

WRITE8_MEMBER (i6300esb_lpc_device::tco_cntl_w)
{
	tco_cntl = data;
	logerror("%s: tco_cntl = %02x\n", tag(), tco_cntl);
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
	remap_cb();
}

READ8_MEMBER  (i6300esb_lpc_device::gpio_cntl_r)
{
	return gpio_cntl;
}

WRITE8_MEMBER (i6300esb_lpc_device::gpio_cntl_w)
{
	gpio_cntl = data;
	logerror("%s: gpio_cntl = %02x\n", tag(), gpio_cntl);
	remap_cb();
}

READ8_MEMBER  (i6300esb_lpc_device::pirq_rout_r)
{
	return pirq_rout[offset];
}

WRITE8_MEMBER (i6300esb_lpc_device::pirq_rout_w)
{
	pirq_rout[offset] = data;
	logerror("%s: pirq_rout[%d] = %02x\n", tag(), offset, pirq_rout[offset]);
}

READ8_MEMBER  (i6300esb_lpc_device::serirq_cntl_r)
{
	return serirq_cntl;
}

WRITE8_MEMBER (i6300esb_lpc_device::serirq_cntl_w)
{
	serirq_cntl = data;
	logerror("%s: serirq_cntl = %02x\n", tag(), serirq_cntl);
}

READ8_MEMBER  (i6300esb_lpc_device::pirq2_rout_r)
{
	return pirq_rout_r(space, offset+4);
}

WRITE8_MEMBER (i6300esb_lpc_device::pirq2_rout_w)
{
	pirq_rout_w(space, offset+4, data);
}

READ8_MEMBER  (i6300esb_lpc_device::d31_err_cfg_r)
{
	return d31_err_cfg;
}

WRITE8_MEMBER (i6300esb_lpc_device::d31_err_cfg_w)
{
	d31_err_cfg = data;
	logerror("%s: d31_err_cfg = %02x\n", tag(), d31_err_cfg);
}

READ8_MEMBER  (i6300esb_lpc_device::d31_err_sts_r)
{
	return d31_err_sts;
}

WRITE8_MEMBER (i6300esb_lpc_device::d31_err_sts_w)
{
	d31_err_sts &= ~data;
	logerror("%s: d31_err_sts = %02x\n", tag(), d31_err_sts);
}

READ16_MEMBER (i6300esb_lpc_device::pci_dma_cfg_r)
{
	return pci_dma_cfg;
}

WRITE16_MEMBER(i6300esb_lpc_device::pci_dma_cfg_w)
{
	COMBINE_DATA(&pci_dma_cfg);
	logerror("%s: pci_dma_cfg = %04x\n", tag(), pci_dma_cfg);
}

READ32_MEMBER (i6300esb_lpc_device::gen_cntl_r)
{
	return gen_cntl;
}

WRITE32_MEMBER(i6300esb_lpc_device::gen_cntl_w)
{
	COMBINE_DATA(&gen_cntl);
	logerror("%s: gen_cntl = %08x\n", tag(), gen_cntl);
}

READ8_MEMBER  (i6300esb_lpc_device::gen_sta_r)
{
	return gen_sta;
}

WRITE8_MEMBER (i6300esb_lpc_device::gen_sta_w)
{
	gen_sta = data;
	logerror("%s: gen_sta = %02x\n", tag(), gen_sta);
}

READ8_MEMBER  (i6300esb_lpc_device::back_cntl_r)
{
	return back_cntl;
}

WRITE8_MEMBER (i6300esb_lpc_device::back_cntl_w)
{
	back_cntl = data;
	logerror("%s: back_cntl = %02x\n", tag(), back_cntl);
	remap_cb();
}

READ8_MEMBER  (i6300esb_lpc_device::rtc_conf_r)
{
	return rtc_conf;
}

WRITE8_MEMBER (i6300esb_lpc_device::rtc_conf_w)
{
	rtc_conf = data;
	logerror("%s: rtc_conf = %02x\n", tag(), rtc_conf);
}

READ8_MEMBER  (i6300esb_lpc_device::lpc_if_com_range_r)
{
	return lpc_if_com_range;
}

WRITE8_MEMBER (i6300esb_lpc_device::lpc_if_com_range_w)
{
	lpc_if_com_range = data;
	logerror("%s: lpc_if_com_range = %02x\n", tag(), lpc_if_com_range);
	remap_cb();
}

READ8_MEMBER  (i6300esb_lpc_device::lpc_if_fdd_lpt_range_r)
{
	return lpc_if_fdd_lpt_range;
}

WRITE8_MEMBER (i6300esb_lpc_device::lpc_if_fdd_lpt_range_w)
{
	COMBINE_DATA(&lpc_if_fdd_lpt_range);
	logerror("%s: lpc_if_fdd_lpt_range  = %02x\n", tag(), lpc_if_fdd_lpt_range);
	remap_cb();
}

READ8_MEMBER  (i6300esb_lpc_device::lpc_if_sound_range_r)
{
	return lpc_if_sound_range;
}

WRITE8_MEMBER (i6300esb_lpc_device::lpc_if_sound_range_w)
{
	COMBINE_DATA(&lpc_if_sound_range);
	logerror("%s: lpc_if_sound_range  = %02x\n", tag(), lpc_if_sound_range);
	remap_cb();
}

READ8_MEMBER  (i6300esb_lpc_device::fwh_dec_en1_r)
{
	return fwh_dec_en1;
}

WRITE8_MEMBER (i6300esb_lpc_device::fwh_dec_en1_w)
{
	fwh_dec_en1 = data | 0x80;
	logerror("%s: fwh_dec_en1  = %02x\n", tag(), fwh_dec_en1);
	remap_cb();
}

READ16_MEMBER (i6300esb_lpc_device::gen1_dec_r)
{
	return gen1_dec;
}

WRITE16_MEMBER(i6300esb_lpc_device::gen1_dec_w)
{
	COMBINE_DATA(&gen1_dec);
	logerror("%s: gen1_dec = %04x\n", tag(), gen1_dec);
	remap_cb();
}

READ16_MEMBER (i6300esb_lpc_device::lpc_en_r)
{
	return lpc_en;
}

WRITE16_MEMBER(i6300esb_lpc_device::lpc_en_w)
{
	COMBINE_DATA(&lpc_en);
	logerror("%s: lpc_en = %04x\n", tag(), lpc_en);
	remap_cb();
}

READ32_MEMBER (i6300esb_lpc_device::fwh_sel1_r)
{
	return fwh_sel1;
}

WRITE32_MEMBER(i6300esb_lpc_device::fwh_sel1_w)
{
	COMBINE_DATA(&fwh_sel1);
	logerror("%s: fwh_sel1 = %08x\n", tag(), fwh_sel1);
	remap_cb();
}

READ16_MEMBER (i6300esb_lpc_device::gen2_dec_r)
{
	return gen2_dec;
}

WRITE16_MEMBER(i6300esb_lpc_device::gen2_dec_w)
{
	COMBINE_DATA(&gen2_dec);
	logerror("%s: gen2_dec = %04x\n", tag(), gen2_dec);
	remap_cb();
}

READ16_MEMBER (i6300esb_lpc_device::fwh_sel2_r)
{
	return fwh_sel2;
}

WRITE16_MEMBER(i6300esb_lpc_device::fwh_sel2_w)
{
	COMBINE_DATA(&fwh_sel2);
	logerror("%s: fwh_sel2 = %04x\n", tag(), fwh_sel2);
	remap_cb();
}

READ8_MEMBER  (i6300esb_lpc_device::fwh_dec_en2_r)
{
	return fwh_dec_en2;
}

WRITE8_MEMBER (i6300esb_lpc_device::fwh_dec_en2_w)
{
	fwh_dec_en2 = data;
	logerror("%s: fwh_dec_en2  = %02x\n", tag(), fwh_dec_en2);
	remap_cb();
}

READ16_MEMBER (i6300esb_lpc_device::func_dis_r)
{
	return func_dis;
}

WRITE16_MEMBER(i6300esb_lpc_device::func_dis_w)
{
	COMBINE_DATA(&func_dis);
	logerror("%s: func_dis = %04x\n", tag(), func_dis);
}

READ32_MEMBER (i6300esb_lpc_device::etr1_r)
{
	return etr1;
}

WRITE32_MEMBER(i6300esb_lpc_device::etr1_w)
{
	logerror("%s: etr1 = %08x\n", tag(), data);
}

READ32_MEMBER (i6300esb_lpc_device::mfid_r)
{
	return 0xf66;
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

	if(acpi_cntl & 0x10)
		logerror("%s: Warning: acpi range enabled at %04x-%04x\n", tag(), pmbase, pmbase+127);
	if(gpio_cntl & 0x10)
		logerror("%s: Warning: gpio range enabled at %04x-%04x\n", tag(), gpio_base, gpio_base+63);

	UINT32 hpet = 0xfed00000 + ((gen_cntl & 0x00018000) >> 3);
	logerror("%s: Warning: hpet at %08x-%08x\n", tag(), hpet, hpet+0x3ff);

	if(lpc_en & 0x1000)
		logerror("%s: Warning: superio at 2e-2f\n", tag());
	if(lpc_en & 0x0800)
		logerror("%s: Warning: mcu at 62/66\n", tag());
	if(lpc_en & 0x0400)
		logerror("%s: Warning: mcu at 60/64\n", tag());
	if(lpc_en & 0x0200)
		logerror("%s: Warning: gameport at 208-20f\n", tag());
	if(lpc_en & 0x0100)
		logerror("%s: Warning: gameport at 200-207\n", tag());

	if(lpc_en & 0x0008) {
		UINT16 fdc = lpc_if_fdd_lpt_range & 0x10 ? 0x370 : 0x3f0;
		logerror("%s: Warning: floppy at %04x-%04x\n", tag(), fdc, fdc+7);
	}

	if(lpc_en & 0x0004) {
		static const UINT16 lpt_pos[4] = { 0x378, 0x278, 0x3bc, 0x000 };
		UINT16 lpt = lpt_pos[lpc_if_fdd_lpt_range & 3];
		if(lpt)
			logerror("%s: Warning: lpt at %04x-%04x %04x-%04x\n", tag(), lpt, lpt+7, lpt+0x400, lpt+0x407);
	}

	static const UINT16 com_pos[8] = { 0x3f8, 0x2f8, 0x220, 0x228, 0x238, 0x2e8, 0x338, 0x3e8 };

	if(lpc_en & 0x0002) {
		UINT16 comb = com_pos[(lpc_if_com_range >> 4) & 7];
		logerror("%s: Warning: comb at %04x-%04x\n", tag(), comb, comb+7);
	}

	if(lpc_en & 0x0001) {
		UINT16 coma = com_pos[lpc_if_com_range & 7];
		logerror("%s: Warning: coma at %04x-%04x\n", tag(), coma, coma+7);
	}
}


