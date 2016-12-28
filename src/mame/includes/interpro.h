// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#pragma once

#ifndef INTERPRO_H_
#define INTERPRO_H_

#include "emu.h"
#include "cpu/clipper/clipper.h"

#include "machine/z80scc.h"
#include "machine/mc146818.h"
#include "bus/rs232/rs232.h"
#include "video/dm9368.h"
#include "machine/upd765.h"
#include "machine/interpro_ioga.h"
#include "machine/ncr539x.h"

#include "formats/pc_dsk.h"

#define INTERPRO_CPU_TAG "cpu"
#define INTERPRO_RTC_TAG "rtc"
#define INTERPRO_SCC1_TAG "scc1"
#define INTERPRO_SCC2_TAG "scc2"
#define INTERPRO_ROM_TAG "rom"
#define INTERPRO_EEPROM_TAG "eeprom"
#define INTERPRO_TERMINAL_TAG "terminal"
#define INTERPRO_LED_TAG "led"
#define INTERPRO_FDC_TAG "fdc"
#define INTERPRO_SCSI_TAG "scsi"
#define INTERPRO_IOGA_TAG "ioga"

// TODO: RTC is actually a DS12887, but the only difference is the 128 byte NVRAM, same as the DS12885
// TODO: not sure what the LED is, but the DM9368 seems close enough

class interpro_state : public driver_device
{
public:
	interpro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, INTERPRO_CPU_TAG),
		m_scc1(*this, INTERPRO_SCC1_TAG),
		m_scc2(*this, INTERPRO_SCC2_TAG),
		m_rtc(*this, INTERPRO_RTC_TAG),
		m_led(*this, INTERPRO_LED_TAG),
		m_fdc(*this, INTERPRO_FDC_TAG),
		m_scsi(*this, INTERPRO_SCSI_TAG),
		m_ioga(*this, INTERPRO_IOGA_TAG)
		{ }

	required_device<clipper_device> m_maincpu;

	// FIXME: not sure which one is the escc
	required_device<z80scc_device> m_scc1;
	required_device<z80scc_device> m_scc2;

	required_device<mc146818_device> m_rtc;
	required_device<dm9368_device> m_led;
	required_device<n82077aa_device> m_fdc;
	required_device<ncr539x_device> m_scsi;

	required_device<interpro_ioga_device> m_ioga;

	DECLARE_DRIVER_INIT(ip2800);

	DECLARE_WRITE8_MEMBER(emerald_w);
	DECLARE_READ8_MEMBER(emerald_r);

	DECLARE_WRITE16_MEMBER(mcga_w);
	DECLARE_READ16_MEMBER(mcga_r);

	DECLARE_WRITE8_MEMBER(interpro_rtc_w);
	DECLARE_READ8_MEMBER(interpro_rtc_r);

	DECLARE_READ32_MEMBER(idprom_r);
	DECLARE_READ32_MEMBER(slot0_r);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	uint8_t m_emerald_reg[4];
	uint16_t m_mcga[32];
};

#endif