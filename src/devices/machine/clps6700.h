// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    CL-PS6700 - Low-Power PC Card Controller for the CL-PS7111

****************************************************************************/

#ifndef MAME_MACHINE_CLPS6700_H
#define MAME_MACHINE_CLPS6700_H

#pragma once

#include "bus/pccard/pccard.h"


class clps6700_device : public device_t
{
public:
	clps6700_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);

	// configuration helpers
	template <typename T> void set_pccard(T &&tag) { m_pccard.set_tag(std::forward<T>(tag)); }

	// callbacks
	auto pirq_handler() { return m_pirq_handler.bind(); }

	int pcm_rdy_r() { return BIT(m_int_input, PCM_RDY); }

	void write_pcm_bvd1(int state) { set_input_level(PCM_BVD1, state); }
	void write_pcm_bvd2(int state) { set_input_level(PCM_BVD2, state); }
	void write_pcm_cd1(int state)  { set_input_level(PCM_CD1, state); }
	void write_pcm_cd2(int state)  { set_input_level(PCM_CD2, state); }
	void write_pcm_wp(int state)   { set_input_level(PCM_WP, state); }

	void map(address_map &map) ATTR_COLD;

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint32_t register_r(offs_t offset);
	void register_w(offs_t offset, uint32_t data);

	void set_input_level(int bit, int state);

	required_device<pccard_slot_device> m_pccard;

	devcb_write_line m_pirq_handler;

	// interrupt sources
	static constexpr int PCM_BVD1 = 0;
	static constexpr int PCM_BVD2 = 1;
	static constexpr int PCM_CD1  = 2;
	static constexpr int PCM_CD2  = 3;
	static constexpr int PCM_VS1  = 4;
	static constexpr int PCM_VS2  = 5;
	static constexpr int PDREQ_L  = 6;
	static constexpr int PCTL     = 8;
	static constexpr int PCM_WP   = 9;
	static constexpr int PCM_RDY  = 10;
	static constexpr int FIFOTHLD = 11;
	static constexpr int IDLE     = 12;
	static constexpr int WR_FAIL  = 13;
	static constexpr int RD_FAIL  = 14;
	static constexpr int RESERVED = 15;

	uint16_t m_int_status;
	uint16_t m_int_mask;
	uint16_t m_int_output;
	uint16_t m_int_input;
	uint16_t m_sys_config;
	uint16_t m_pwr_manager;
	uint16_t m_pwr_control;
	uint16_t m_intf_config;
	uint16_t m_intf_timing_0a;
	uint16_t m_intf_timing_0b;
	uint16_t m_intf_timing_1a;
	uint16_t m_intf_timing_1b;
	uint16_t m_dma_control;
	uint16_t m_device_info;
};


DECLARE_DEVICE_TYPE(CLPS6700, clps6700_device)

#endif // MAME_MACHINE_CLPS6700_H
