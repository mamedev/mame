// license:BSD-3-Clause
// copyright-holders:David Haywood
/************************************************************************************

    Bellfruit dotmatrix driver, (under heavy construction !!!)

*************************************************************************************/
#ifndef MAME_BFM_BFM_DM01
#define MAME_BFM_BFM_DM01

#pragma once

#include "emupal.h"
#include "screen.h"

class bfm_dm01_device : public device_t
{
public:
	bfm_dm01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto busy_callback() { return m_busy_cb.bind(); }

	uint8_t control_r();
	void control_w(uint8_t data);
	uint8_t mux_r();
	void mux_w(uint8_t data);
	uint8_t comm_r();
	void comm_w(uint8_t data);
	uint8_t unknown_r();
	void unknown_w(uint8_t data);

	void writedata(uint8_t data);
	int busy(void);

	void bfm_dm01_memmap(address_map &map) ATTR_COLD;
protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr unsigned BYTES_PER_ROW = 9;

	required_device<cpu_device> m_matrixcpu;

	// internal state
	int m_data_avail;
	int m_control;
	int m_xcounter;
	int m_segbuffer[65];
	int m_busy;

	uint8_t m_scanline[BYTES_PER_ROW];
	uint8_t m_comdata;

	output_finder<65 * 21> m_dotmatrix;
	devcb_write_line m_busy_cb;

	int read_data();

	INTERRUPT_GEN_MEMBER(nmi_line_assert);
};

DECLARE_DEVICE_TYPE(BFM_DM01, bfm_dm01_device)

#endif // MAME_BFM_BFM_DM01
