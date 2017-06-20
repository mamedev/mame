// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_INCLUDES_INTERPRO_H
#define MAME_INCLUDES_INTERPRO_H

#pragma once

#include "cpu/clipper/clipper.h"
#include "machine/cammu.h"

#include "machine/interpro_ioga.h"
#include "machine/interpro_mcga.h"
#include "machine/interpro_sga.h"
#include "machine/interpro_srarb.h"

#include "machine/z80scc.h"
#include "machine/mc146818.h"
#include "machine/upd765.h"
#if NEW_SCSI
#include "machine/ncr5390.h"

#include "machine/nscsi_bus.h"
#include "machine/nscsi_cd.h"
#include "machine/nscsi_hd.h"
#else
#include "machine/ncr539x.h"

#include "bus/scsi/scsi.h"
#include "bus/scsi/scsicd.h"
#include "bus/scsi/scsihd.h"
#endif
#include "bus/rs232/rs232.h"

#include "formats/pc_dsk.h"

#define INTERPRO_CPU_TAG        "cpu"
#define INTERPRO_MMU_TAG        "mmu"

#define INTERPRO_RTC_TAG        "rtc"
#define INTERPRO_SCC1_TAG       "scc1"
#define INTERPRO_SCC2_TAG       "scc2"
#define INTERPRO_EPROM_TAG      "eprom"
#define INTERPRO_FLASH_TAG      "flash"
#define INTERPRO_TERMINAL_TAG   "terminal"
#define INTERPRO_FDC_TAG        "fdc"
#define INTERPRO_SCSI_TAG       "scsi"
#define INTERPRO_SCSI_ADAPTER_TAG    "adapter"

#define INTERPRO_IOGA_TAG       "ioga"
#define INTERPRO_MCGA_TAG       "mcga"
#define INTERPRO_SGA_TAG        "sga"
#define INTERPRO_SRARB_TAG      "srarb"

// system board register offsets
#define SREG_LED    0
#define SREG_ERROR  0
#define SREG_STATUS 1
#define SREG_CTRL1  2
#define SREG_CTRL2  3

// control register 1
#define CTRL1_FLOPLOW    0x0001
#define CTRL1_FLOPRDY    0x0002
#define CTRL1_LEDENA     0x0004
#define CTRL1_LEDDP      0x0008
#define CTRL1_ETHLOOP    0x0010
#define CTRL1_ETHDTR     0x0020
#define CTRL1_ETHRMOD    0x0040
#define CTRL1_CLIPRESET  0x0040
#define CTRL1_FIFOACTIVE 0x0080

// control register 2
#define CTRL2_PWRUP     0x0001
#define CTRL2_PWRENA    0x0002
#define CTRL2_HOLDOFF   0x0004
#define CTRL2_EXTNMIENA 0x0008
#define CTRL2_COLDSTART 0x0010
#define CTRL2_RESET     0x0020
#define CTRL2_BUSENA    0x0040
#define CTRL2_FRCPARITY 0x0080
#define CTRL2_FLASHEN   0x0080
#define CTRL2_WMASK     0x000f

class interpro_state : public driver_device
{
public:
	interpro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, INTERPRO_CPU_TAG),
		m_mmu(*this, INTERPRO_MMU_TAG),
		m_scc1(*this, INTERPRO_SCC1_TAG),
		m_scc2(*this, INTERPRO_SCC2_TAG),
		m_rtc(*this, INTERPRO_RTC_TAG),
		m_fdc(*this, INTERPRO_FDC_TAG),
#if NEW_SCSI
		m_scsibus(*this, INTERPRO_SCSI_TAG),
		m_scsi(*this, INTERPRO_SCSI_TAG ":7:" INTERPRO_SCSI_ADAPTER_TAG),
#else
		m_scsi(*this, INTERPRO_SCSI_ADAPTER_TAG),
#endif
		m_ioga(*this, INTERPRO_IOGA_TAG),
		m_mcga(*this, INTERPRO_MCGA_TAG),
		m_sga(*this, INTERPRO_SGA_TAG),
		m_srarb(*this, INTERPRO_SRARB_TAG)
		{ }

	required_device<clipper_device> m_maincpu;
	required_device<cammu_device> m_mmu;

	// FIXME: not sure which one is the escc
	required_device<z80scc_device> m_scc1;
	required_device<z80scc_device> m_scc2;
	required_device<mc146818_device> m_rtc;
	required_device<n82077aa_device> m_fdc;
#if NEW_SCSI
	required_device<nscsi_bus_device> m_scsibus;
	required_device<ncr53c94_device> m_scsi;
#else
	required_device<ncr539x_device> m_scsi;
#endif

	required_device<interpro_ioga_device> m_ioga;
	required_device<interpro_fmcc_device> m_mcga;
	required_device<interpro_sga_device> m_sga;
	required_device<interpro_srarb_device> m_srarb;

	DECLARE_DRIVER_INIT(ip2800);

	DECLARE_WRITE16_MEMBER(system_w);
	DECLARE_READ16_MEMBER(system_r);

	DECLARE_WRITE8_MEMBER(rtc_w);
	DECLARE_READ8_MEMBER(rtc_r);

	DECLARE_READ32_MEMBER(idprom_r);
	DECLARE_READ32_MEMBER(slot0_r);

	DECLARE_READ8_MEMBER(scsi_r);
	DECLARE_WRITE8_MEMBER(scsi_w);
	DECLARE_READ8_MEMBER(scsi_dma_r);
	DECLARE_WRITE8_MEMBER(scsi_dma_w);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	u16 m_system_reg[4];
};

#endif // MAME_INCLUDES_INTERPRO_H
