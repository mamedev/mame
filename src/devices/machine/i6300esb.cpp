// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "i6300esb.h"

DEFINE_DEVICE_TYPE(I6300ESB_WATCHDOG, i6300esb_watchdog_device, "i6300esb_watchdog", "i6300ESB southbridge watchdog")
DEFINE_DEVICE_TYPE(I6300ESB_LPC,      i6300esb_lpc_device,      "i6300esb_lpc",      "i6300ESB southbridge ISA/LPC bridge")

void i6300esb_watchdog_device::map(address_map &map)
{
}

i6300esb_watchdog_device::i6300esb_watchdog_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, I6300ESB_WATCHDOG, tag, owner, clock)
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


void i6300esb_lpc_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x40, 0x43).rw(FUNC(i6300esb_lpc_device::pmbase_r), FUNC(i6300esb_lpc_device::pmbase_w));
	map(0x44, 0x44).rw(FUNC(i6300esb_lpc_device::acpi_cntl_r), FUNC(i6300esb_lpc_device::acpi_cntl_w));
	map(0x4e, 0x4f).rw(FUNC(i6300esb_lpc_device::bios_cntl_r), FUNC(i6300esb_lpc_device::bios_cntl_w));
	map(0x54, 0x54).rw(FUNC(i6300esb_lpc_device::tco_cntl_r), FUNC(i6300esb_lpc_device::tco_cntl_w));
	map(0x58, 0x5b).rw(FUNC(i6300esb_lpc_device::gpio_base_r), FUNC(i6300esb_lpc_device::gpio_base_w));
	map(0x5c, 0x5c).rw(FUNC(i6300esb_lpc_device::gpio_cntl_r), FUNC(i6300esb_lpc_device::gpio_cntl_w));
	map(0x60, 0x63).rw(FUNC(i6300esb_lpc_device::pirq_rout_r), FUNC(i6300esb_lpc_device::pirq_rout_w));
	map(0x64, 0x64).rw(FUNC(i6300esb_lpc_device::serirq_cntl_r), FUNC(i6300esb_lpc_device::serirq_cntl_w));
	map(0x68, 0x6b).rw(FUNC(i6300esb_lpc_device::pirq2_rout_r), FUNC(i6300esb_lpc_device::pirq2_rout_w));
	map(0x88, 0x88).rw(FUNC(i6300esb_lpc_device::d31_err_cfg_r), FUNC(i6300esb_lpc_device::d31_err_cfg_w));
	map(0x8a, 0x8a).rw(FUNC(i6300esb_lpc_device::d31_err_sts_r), FUNC(i6300esb_lpc_device::d31_err_sts_w));
	map(0x90, 0x91).rw(FUNC(i6300esb_lpc_device::pci_dma_cfg_r), FUNC(i6300esb_lpc_device::pci_dma_cfg_w));
	map(0xa0, 0xa1).rw(FUNC(i6300esb_lpc_device::gen_pmcon_1_r), FUNC(i6300esb_lpc_device::gen_pmcon_1_w));
	map(0xa2, 0xa2).rw(FUNC(i6300esb_lpc_device::gen_pmcon_2_r), FUNC(i6300esb_lpc_device::gen_pmcon_2_w));
	map(0xa4, 0xa4).rw(FUNC(i6300esb_lpc_device::gen_pmcon_3_r), FUNC(i6300esb_lpc_device::gen_pmcon_3_w));
	map(0xac, 0xaf).rw(FUNC(i6300esb_lpc_device::rst_cnt2_r), FUNC(i6300esb_lpc_device::rst_cnt2_w));
	map(0xb2, 0xb2).rw(FUNC(i6300esb_lpc_device::apm_cnt_r), FUNC(i6300esb_lpc_device::apm_cnt_w));
	map(0xb3, 0xb3).rw(FUNC(i6300esb_lpc_device::apm_sts_r), FUNC(i6300esb_lpc_device::apm_sts_w));
	map(0xb8, 0xbb).rw(FUNC(i6300esb_lpc_device::gpi_rout_r), FUNC(i6300esb_lpc_device::gpi_rout_w));
	map(0xc0, 0xc0).rw(FUNC(i6300esb_lpc_device::mon_fwd_en_r), FUNC(i6300esb_lpc_device::mon_fwd_en_w));
	map(0xc4, 0xcb).rw(FUNC(i6300esb_lpc_device::mon_trp_rng_r), FUNC(i6300esb_lpc_device::mon_trp_rng_w));
	map(0xcc, 0xcd).rw(FUNC(i6300esb_lpc_device::mon_trp_msk_r), FUNC(i6300esb_lpc_device::mon_trp_msk_w));
	map(0xd0, 0xd3).rw(FUNC(i6300esb_lpc_device::gen_cntl_r), FUNC(i6300esb_lpc_device::gen_cntl_w));
	map(0xd4, 0xd4).rw(FUNC(i6300esb_lpc_device::gen_sta_r), FUNC(i6300esb_lpc_device::gen_sta_w));
	map(0xd5, 0xd5).rw(FUNC(i6300esb_lpc_device::back_cntl_r), FUNC(i6300esb_lpc_device::back_cntl_w));
	map(0xd8, 0xd8).rw(FUNC(i6300esb_lpc_device::rtc_conf_r), FUNC(i6300esb_lpc_device::rtc_conf_w));
	map(0xe0, 0xe0).rw(FUNC(i6300esb_lpc_device::lpc_if_com_range_r), FUNC(i6300esb_lpc_device::lpc_if_com_range_w));
	map(0xe1, 0xe1).rw(FUNC(i6300esb_lpc_device::lpc_if_fdd_lpt_range_r), FUNC(i6300esb_lpc_device::lpc_if_fdd_lpt_range_w));
	map(0xe2, 0xe2).rw(FUNC(i6300esb_lpc_device::lpc_if_sound_range_r), FUNC(i6300esb_lpc_device::lpc_if_sound_range_w));
	map(0xe3, 0xe3).rw(FUNC(i6300esb_lpc_device::fwh_dec_en1_r), FUNC(i6300esb_lpc_device::fwh_dec_en1_w));
	map(0xe4, 0xe5).rw(FUNC(i6300esb_lpc_device::gen1_dec_r), FUNC(i6300esb_lpc_device::gen1_dec_w));
	map(0xe6, 0xe7).rw(FUNC(i6300esb_lpc_device::lpc_en_r), FUNC(i6300esb_lpc_device::lpc_en_w));
	map(0xe8, 0xeb).rw(FUNC(i6300esb_lpc_device::fwh_sel1_r), FUNC(i6300esb_lpc_device::fwh_sel1_w));
	map(0xec, 0xed).rw(FUNC(i6300esb_lpc_device::gen2_dec_r), FUNC(i6300esb_lpc_device::gen2_dec_w));
	map(0xee, 0xef).rw(FUNC(i6300esb_lpc_device::fwh_sel2_r), FUNC(i6300esb_lpc_device::fwh_sel2_w));
	map(0xf0, 0xf0).rw(FUNC(i6300esb_lpc_device::fwh_dec_en2_r), FUNC(i6300esb_lpc_device::fwh_dec_en2_w));
	map(0xf2, 0xf3).rw(FUNC(i6300esb_lpc_device::func_dis_r), FUNC(i6300esb_lpc_device::func_dis_w));
	map(0xf4, 0xf7).rw(FUNC(i6300esb_lpc_device::etr1_r), FUNC(i6300esb_lpc_device::etr1_w));
	map(0xf8, 0xfb).r(FUNC(i6300esb_lpc_device::mfid_r));
	map(0xfc, 0xff).rw(FUNC(i6300esb_lpc_device::unk_fc_r), FUNC(i6300esb_lpc_device::unk_fc_w));
}

void i6300esb_lpc_device::internal_io_map(address_map &map)
{
	if(lpc_en & 0x2000) {
		map(0x004e, 0x004e).rw(FUNC(i6300esb_lpc_device::siu_config_port_r), FUNC(i6300esb_lpc_device::siu_config_port_w));
		map(0x004f, 0x004f).rw(FUNC(i6300esb_lpc_device::siu_data_port_r), FUNC(i6300esb_lpc_device::siu_data_port_w));
	}

	map(0x0061, 0x0061).rw(FUNC(i6300esb_lpc_device::nmi_sc_r), FUNC(i6300esb_lpc_device::nmi_sc_w));

	map(0x0080, 0x0080).w(FUNC(i6300esb_lpc_device::nop_w)); // POST/non-existing, used for delays by the bios/os
	map(0x00ed, 0x00ed).w(FUNC(i6300esb_lpc_device::nop_w)); // Non-existing, used for delays by the bios/os
}


i6300esb_lpc_device::i6300esb_lpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, I6300ESB_LPC, tag, owner, clock)
	, acpi(*this, "acpi")
	, rtc (*this, "rtc")
	, pit (*this, "pit")
	, m_region(*this, DEVICE_SELF)
{
	set_ids(0x808625a1, 0x02, 0x060100, 0x00000000);
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
	func_dis = 0x0080;
	etr1 = 0x00000000;
	siu_config_port = 0;
	siu_config_state = 0;
	gen_pmcon_1 = 0;
	gen_pmcon_2 = 0;
	gen_pmcon_3 = 0;
	rst_cnt2 = 0;
	apm_cnt = 0;
	apm_sts = 0;
	gpi_rout = 0;
	mon_fwd_en = 0;
	memset(mon_trp_rng, 0, sizeof(mon_trp_rng));
	mon_trp_msk = 0;
	nmi_sc = 0;
	gen_sta = 0x00;
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
	rtc_conf = 0x00;
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

READ16_MEMBER (i6300esb_lpc_device::gen_pmcon_1_r)
{
	return gen_pmcon_1;
}

WRITE16_MEMBER(i6300esb_lpc_device::gen_pmcon_1_w)
{
	COMBINE_DATA(&gen_pmcon_1);
	logerror("%s: gen_pmcon_1 = %04x\n", tag(), gen_pmcon_1);
}

READ8_MEMBER  (i6300esb_lpc_device::gen_pmcon_2_r)
{
	return gen_pmcon_2;
}

WRITE8_MEMBER (i6300esb_lpc_device::gen_pmcon_2_w)
{
	gen_pmcon_2 = data;
	logerror("%s: gen_pmcon_2 = %02x\n", tag(), gen_pmcon_2);
}

READ8_MEMBER  (i6300esb_lpc_device::gen_pmcon_3_r)
{
	return gen_pmcon_3;
}

WRITE8_MEMBER (i6300esb_lpc_device::gen_pmcon_3_w)
{
	gen_pmcon_3 = data;
	logerror("%s: gen_pmcon_3 = %02x\n", tag(), gen_pmcon_3);
}

READ32_MEMBER (i6300esb_lpc_device::rst_cnt2_r)
{
	return rst_cnt2;
}

WRITE32_MEMBER(i6300esb_lpc_device::rst_cnt2_w)
{
	COMBINE_DATA(&rst_cnt2);
	logerror("%s: rst_cnt2 = %08x\n", tag(), rst_cnt2);
}

READ8_MEMBER  (i6300esb_lpc_device::apm_cnt_r)
{
	return apm_cnt;
}

WRITE8_MEMBER (i6300esb_lpc_device::apm_cnt_w)
{
	apm_cnt = data;
	logerror("%s: apm_cnt = %02x\n", tag(), apm_cnt);
}

READ8_MEMBER  (i6300esb_lpc_device::apm_sts_r)
{
	return apm_sts;
}

WRITE8_MEMBER (i6300esb_lpc_device::apm_sts_w)
{
	apm_sts = data;
	logerror("%s: apm_sts = %02x\n", tag(), apm_sts);
}

READ32_MEMBER (i6300esb_lpc_device::gpi_rout_r)
{
	return gpi_rout;
}

WRITE32_MEMBER(i6300esb_lpc_device::gpi_rout_w)
{
	COMBINE_DATA(&gpi_rout);
	logerror("%s: gpi_rout = %08x\n", tag(), gpi_rout);
}

READ8_MEMBER  (i6300esb_lpc_device::mon_fwd_en_r)
{
	return mon_fwd_en;
}

WRITE8_MEMBER (i6300esb_lpc_device::mon_fwd_en_w)
{
	mon_fwd_en = data;
	logerror("%s: mon_fwd_en = %02x\n", tag(), mon_fwd_en);
}

READ16_MEMBER (i6300esb_lpc_device::mon_trp_rng_r)
{
	return mon_trp_rng[offset];
}

WRITE16_MEMBER(i6300esb_lpc_device::mon_trp_rng_w)
{
	COMBINE_DATA(&mon_trp_rng[offset]);
	logerror("%s: mon_trp_rng[%d] = %04x\n", tag(), 4+offset, mon_trp_rng[offset]);
}

READ16_MEMBER (i6300esb_lpc_device::mon_trp_msk_r)
{
	return mon_trp_msk;
}

WRITE16_MEMBER(i6300esb_lpc_device::mon_trp_msk_w)
{
	COMBINE_DATA(&mon_trp_msk);
	logerror("%s: mon_trp_msk = %04x\n", tag(), mon_trp_msk);
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
	remap_cb();
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
	COMBINE_DATA(&etr1);
	logerror("%s: etr1 = %08x\n", tag(), etr1);
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

READ8_MEMBER  (i6300esb_lpc_device::nmi_sc_r)
{
	nmi_sc ^= 0x10;
	return nmi_sc;
}

WRITE8_MEMBER (i6300esb_lpc_device::nmi_sc_w)
{
	nmi_sc = data;
	logerror("%s: nmi_sc = %02x\n", tag(), nmi_sc);
}


WRITE8_MEMBER (i6300esb_lpc_device::nop_w)
{
}

void i6300esb_lpc_device::map_bios(address_space *memory_space, uint32_t start, uint32_t end, int idsel)
{
	// Ignore idsel, a16 inversion for now
	uint32_t mask = m_region->bytes() - 1;
	memory_space->install_rom(start, end, m_region->base() + (start & mask));
}

void i6300esb_lpc_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
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
		acpi->map_device(memory_window_start, memory_window_end, 0, memory_space, io_window_start, io_window_end, pmbase, io_space);
	if(gpio_cntl & 0x10)
		logerror("%s: Warning: gpio range enabled at %04x-%04x\n", tag(), gpio_base, gpio_base+63);

	uint32_t hpet = 0xfed00000 + ((gen_cntl & 0x00018000) >> 3);
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
		uint16_t fdc = lpc_if_fdd_lpt_range & 0x10 ? 0x370 : 0x3f0;
		logerror("%s: Warning: floppy at %04x-%04x\n", tag(), fdc, fdc+7);
	}

	if(lpc_en & 0x0004) {
		static const uint16_t lpt_pos[4] = { 0x378, 0x278, 0x3bc, 0x000 };
		uint16_t lpt = lpt_pos[lpc_if_fdd_lpt_range & 3];
		if(lpt)
			logerror("%s: Warning: lpt at %04x-%04x %04x-%04x\n", tag(), lpt, lpt+7, lpt+0x400, lpt+0x407);
	}

	static const uint16_t com_pos[8] = { 0x3f8, 0x2f8, 0x220, 0x228, 0x238, 0x2e8, 0x338, 0x3e8 };

	if(lpc_en & 0x0002) {
		uint16_t comb = com_pos[(lpc_if_com_range >> 4) & 7];
		logerror("%s: Warning: comb at %04x-%04x\n", tag(), comb, comb+7);
	}

	if(lpc_en & 0x0001) {
		uint16_t coma = com_pos[lpc_if_com_range & 7];
		logerror("%s: Warning: coma at %04x-%04x\n", tag(), coma, coma+7);
	}

	rtc->map_device(memory_window_start, memory_window_end, 0, memory_space, io_window_start, io_window_end, 0, io_space);
	if(rtc_conf & 4)
		rtc->map_extdevice(memory_window_start, memory_window_end, 0, memory_space, io_window_start, io_window_end, 0, io_space);
	pit->map_device(memory_window_start, memory_window_end, 0, memory_space, io_window_start, io_window_end, 0, io_space);
}
