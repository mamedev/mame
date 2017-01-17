// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#pragma once

#ifndef INTERPRO_H_
#define INTERPRO_H_

#include "emu.h"

#include "cpu/clipper/clipper.h"
#include "machine/cammu.h"

#include "machine/interpro_ioga.h"

#include "machine/z80scc.h"
#include "machine/mc146818.h"
#include "machine/upd765.h"
#include "machine/ncr539x.h"

#include "bus/rs232/rs232.h"

#include "formats/pc_dsk.h"

#define INTERPRO_CPU_TAG        "cpu"
#define INTERPRO_CAMMU_TAG      "cammu"

#define INTERPRO_RTC_TAG        "rtc"
#define INTERPRO_SCC1_TAG       "scc1"
#define INTERPRO_SCC2_TAG       "scc2"
#define INTERPRO_ROM_TAG        "rom"
#define INTERPRO_EEPROM_TAG     "eeprom"
#define INTERPRO_TERMINAL_TAG   "terminal"
#define INTERPRO_FDC_TAG        "fdc"
#define INTERPRO_SCSI_TAG       "scsi"
#define INTERPRO_IOGA_TAG       "ioga"

// TODO: RTC is actually a DS12887, but the only difference is the 128 byte NVRAM, same as the DS12885

class interpro_state : public driver_device
{
public:
	interpro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, INTERPRO_CPU_TAG),
		m_cammu(*this, INTERPRO_CAMMU_TAG),
		m_scc1(*this, INTERPRO_SCC1_TAG),
		m_scc2(*this, INTERPRO_SCC2_TAG),
		m_rtc(*this, INTERPRO_RTC_TAG),
		m_fdc(*this, INTERPRO_FDC_TAG),
		m_scsi(*this, INTERPRO_SCSI_TAG),
		m_ioga(*this, INTERPRO_IOGA_TAG)
		{ }

	required_device<clipper_device> m_maincpu;
	required_device<cammu_device> m_cammu;

	// FIXME: not sure which one is the escc
	required_device<z80scc_device> m_scc1;
	required_device<z80scc_device> m_scc2;

	required_device<mc146818_device> m_rtc;
	required_device<n82077aa_device> m_fdc;
	required_device<ncr539x_device> m_scsi;

	required_device<interpro_ioga_device> m_ioga;

	DECLARE_DRIVER_INIT(ip2800);

	DECLARE_WRITE16_MEMBER(emerald_w);
	DECLARE_READ16_MEMBER(emerald_r);

	DECLARE_WRITE16_MEMBER(mcga_w);
	DECLARE_READ16_MEMBER(mcga_r);

	DECLARE_WRITE8_MEMBER(rtc_w);
	DECLARE_READ8_MEMBER(rtc_r);

	DECLARE_READ32_MEMBER(idprom_r);
	DECLARE_READ32_MEMBER(slot0_r);

	DECLARE_READ8_MEMBER(scsi_r);
	DECLARE_WRITE8_MEMBER(scsi_w);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	DECLARE_READ32_MEMBER(sga_gcs_r) { return m_sga_gcs; }
	DECLARE_WRITE32_MEMBER(sga_gcs_w) { m_sga_gcs = data; }
	DECLARE_READ32_MEMBER(sga_ipoll_r) { return m_sga_ipoll; }
	DECLARE_WRITE32_MEMBER(sga_ipoll_w) { m_sga_ipoll = data; }
	DECLARE_READ32_MEMBER(sga_imask_r) { return m_sga_imask; }
	DECLARE_WRITE32_MEMBER(sga_imask_w) { m_sga_imask = data; }
	DECLARE_READ32_MEMBER(sga_range_base_r) { return m_sga_range_base; }
	DECLARE_WRITE32_MEMBER(sga_range_base_w) { m_sga_range_base = data; }
	DECLARE_READ32_MEMBER(sga_range_end_r) { return m_sga_range_end; }
	DECLARE_WRITE32_MEMBER(sga_range_end_w) { m_sga_range_end = data; }
	DECLARE_READ32_MEMBER(sga_cttag_r) { return m_sga_cttag; }
	DECLARE_WRITE32_MEMBER(sga_cttag_w) { m_sga_cttag = data; }
	DECLARE_READ32_MEMBER(sga_address_r) { return m_sga_address; }
	DECLARE_WRITE32_MEMBER(sga_address_w) { m_sga_address = data; }
	DECLARE_READ32_MEMBER(sga_dmacs_r) { return m_sga_dmacs; }
	DECLARE_WRITE32_MEMBER(sga_dmacs_w) { m_sga_dmacs = data; }
	DECLARE_READ32_MEMBER(sga_edmacs_r) { return m_sga_edmacs; }
	DECLARE_WRITE32_MEMBER(sga_edmacs_w) { m_sga_edmacs = data; }
	DECLARE_READ32_MEMBER(sga_dspad1_r) { return m_sga_dspad1; }
	DECLARE_WRITE32_MEMBER(sga_dspad1_w) { m_sga_dspad1 = data; }
	DECLARE_READ32_MEMBER(sga_dsoff1_r) { return m_sga_dsoff1; }
	DECLARE_WRITE32_MEMBER(sga_dsoff1_w) { m_sga_dsoff1 = data; }
	DECLARE_READ32_MEMBER(sga_unknown1_r) { return m_sga_unknown1; }
	DECLARE_WRITE32_MEMBER(sga_unknown1_w) { m_sga_unknown1 = data; }
	DECLARE_READ32_MEMBER(sga_unknown2_r) { return m_sga_unknown2; }
	DECLARE_WRITE32_MEMBER(sga_unknown2_w) { m_sga_unknown2 = data; }
	DECLARE_READ32_MEMBER(sga_ddtc1_r) { return m_sga_ddtc1; }
	DECLARE_WRITE32_MEMBER(sga_ddtc1_w);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	uint16_t m_emerald_reg[4];
	uint16_t m_mcga[32];

	uint32_t m_sga_gcs;         // general control/status
	uint32_t m_sga_ipoll;       // interrupt poll
	uint32_t m_sga_imask;       // interrupt mask
	uint32_t m_sga_range_base;
	uint32_t m_sga_range_end;
	uint32_t m_sga_cttag;       // error cycletype/tag
	uint32_t m_sga_address;
	uint32_t m_sga_dmacs;       // dma control/status
	uint32_t m_sga_edmacs;      // extended dma control/status
	uint32_t m_sga_dspad1;
	uint32_t m_sga_dsoff1;
	uint32_t m_sga_unknown1;
	uint32_t m_sga_unknown2;
	uint32_t m_sga_ddtc1;
};

#endif