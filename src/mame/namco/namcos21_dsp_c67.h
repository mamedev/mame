// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, David Haywood
#ifndef MAME_NAMCO_NAMCOS21_DSP_C67_H
#define MAME_NAMCO_NAMCOS21_DSP_C67_H

#pragma once

#include "namco_dsp.h"
#include "namcos21_3d.h"

#include <algorithm>
#include <memory>


class namcos21_dsp_c67_device : public device_t
{
public:
	static constexpr unsigned PTRAM_SIZE = 0x20000;

	enum
	{   /* Namco System21 */
		NAMCOS21_AIRCOMBAT = 0x4000,
		NAMCOS21_STARBLADE,
		NAMCOS21_CYBERSLED,
		NAMCOS21_SOLVALOU,
	};

	namcos21_dsp_c67_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// config
	template <typename T> void set_renderer_tag(T &&tag) { m_renderer.set_tag(std::forward<T>(tag)); }
	void set_gametype(int gametype) { m_gametype = gametype; }

	u16 dspram16_r(offs_t offset);
	void dspram16_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void pointram_control_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 pointram_data_r();
	void pointram_data_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 namcos21_depthcue_r(offs_t offset);
	void namcos21_depthcue_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void reset_dsps(int state);
	void reset_kickstart();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	static constexpr unsigned DSP_BUF_MAX = 4096*12;
	struct dsp_state
	{
		dsp_state()
		{
			std::fill(std::begin(slaveInputBuffer), std::end(slaveInputBuffer), 0);
			std::fill(std::begin(slaveOutputBuffer), std::end(slaveOutputBuffer), 0);
			std::fill(std::begin(masterDirectDrawBuffer), std::end(masterDirectDrawBuffer), 0);
		}

		u32 masterSourceAddr = 0;
		u16 slaveInputBuffer[DSP_BUF_MAX];
		u32 slaveBytesAvailable = 0;
		u32 slaveBytesAdvertised = 0;
		u32 slaveInputStart = 0;
		u16 slaveOutputBuffer[DSP_BUF_MAX];
		u32 slaveOutputSize = 0;
		u16 masterDirectDrawBuffer[256];
		u32 masterDirectDrawSize = 0;
		int masterFinished = 0;
		int slaveActive = 0;
	};

	required_device<namcos21_3d_device> m_renderer;
	required_device<namco_c67_device> m_c67master;
	required_device_array<namco_c67_device, 4> m_c67slave;
	required_region_ptr<s32> m_ptrom24;
	std::unique_ptr<u16 []> m_dspram16;

	required_shared_ptr<u16> m_master_dsp_ram;

	int m_gametype; // hacks
	std::unique_ptr<dsp_state> m_mpDspState;

	std::unique_ptr<u8 []> m_pointram;
	int m_pointram_idx;
	u16 m_pointram_control;
	u32 m_pointrom_idx;
	u8 m_mPointRomMSB;
	int m_mbPointRomDataAvailable;
	u8 m_depthcue[2][0x400];
	int m_irq_enable;
	int m_mbNeedsKickstart;

	s32 read_pointrom_data(u32 offset);
	void transmit_word_to_slave(u16 data);
	void transfer_dsp_data(bool first);
	u16 read_word_from_slave_input();
	u16 get_input_bytes_advertised_for_slave();
	void render_slave_output(u16 data);

	void namcos21_kickstart();

	void dspcuskey_w(u16 data);
	u16 dspcuskey_r();
	u16 dsp_port0_r();
	void dsp_port0_w(u16 data);
	u16 dsp_port1_r();
	void dsp_port1_w(u16 data);
	u16 dsp_port2_r();
	void dsp_port2_w(u16 data);
	u16 dsp_port3_idc_rcv_enable_r();
	void dsp_port3_w(u16 data);
	void dsp_port4_w(u16 data);
	u16 dsp_port8_r();
	void dsp_port8_w(u16 data);
	u16 dsp_port9_r();
	u16 dsp_porta_r();
	void dsp_porta_w(u16 data);
	u16 dsp_portb_r();
	void dsp_portb_w(u16 data);
	void dsp_portc_w(u16 data);
	u16 dsp_portf_r();
	void dsp_xf_w(u16 data);
	u16 slave_port0_r();
	void slave_port0_w(u16 data);
	u16 slave_port2_r();
	u16 slave_port3_r();
	void slave_port3_w(u16 data);
	void slave_XF_output_w(u16 data);
	u16 slave_portf_r();

	void master_dsp_data(address_map &map) ATTR_COLD;
	void master_dsp_io(address_map &map) ATTR_COLD;
	void master_dsp_program(address_map &map) ATTR_COLD;

	void slave_dsp_data(address_map &map) ATTR_COLD;
	void slave_dsp_io(address_map &map) ATTR_COLD;
	void slave_dsp_program(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(NAMCOS21_DSP_C67, namcos21_dsp_c67_device)

#endif // MAME_NAMCO_NAMCOS21_DSP_C67_H
