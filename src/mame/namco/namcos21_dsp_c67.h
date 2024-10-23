// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_NAMCO_NAMCOS21_DSP_C67_H
#define MAME_NAMCO_NAMCOS21_DSP_C67_H

#pragma once

#include "namco_c67.h"
#include "namcos21_3d.h"

#include <algorithm>
#include <memory>


#define ENABLE_LOGGING      0

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

	namcos21_dsp_c67_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// config
	template <typename T> void set_renderer_tag(T &&tag) { m_renderer.set_tag(std::forward<T>(tag)); }
	auto yield_hack_callback() { return m_yield_hack_cb.bind(); }

	void set_gametype(int gametype) { m_gametype = gametype; }

	uint16_t dspram16_r(offs_t offset);
	void dspram16_hack_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void dspram16_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void pointram_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pointram_data_r();
	void pointram_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t namcos21_depthcue_r(offs_t offset);
	void namcos21_depthcue_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

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

		unsigned masterSourceAddr = 0;
		uint16_t slaveInputBuffer[DSP_BUF_MAX];
		unsigned slaveBytesAvailable = 0;
		unsigned slaveBytesAdvertised = 0;
		unsigned slaveInputStart = 0;
		uint16_t slaveOutputBuffer[DSP_BUF_MAX];
		unsigned slaveOutputSize = 0;
		uint16_t masterDirectDrawBuffer[256];
		unsigned masterDirectDrawSize = 0;
		int masterFinished = 0;
		int slaveActive = 0;
	};

	required_device<namcos21_3d_device> m_renderer;
	required_device<cpu_device> m_c67master;
	required_device_array<cpu_device,4> m_c67slave;
	required_region_ptr<int32_t> m_ptrom24;
	std::unique_ptr<uint16_t []> m_dspram16;

	required_shared_ptr<uint16_t> m_master_dsp_ram;

	int m_gametype; // hacks
	devcb_write_line m_yield_hack_cb;

	std::unique_ptr<dsp_state> m_mpDspState;

	std::unique_ptr<uint8_t []> m_pointram;
	int m_pointram_idx;
	uint16_t m_pointram_control;
	uint32_t m_pointrom_idx;
	uint8_t m_mPointRomMSB;
	int m_mbPointRomDataAvailable;
	uint8_t m_depthcue[2][0x400];
	int m_irq_enable;

	int m_mbNeedsKickstart;

	int m_poly_frame_width;
	int m_poly_frame_height;

	int32_t read_pointrom_data(unsigned offset);
	void transmit_word_to_slave(uint16_t data);
	void transfer_dsp_data();
	uint16_t read_word_from_slave_input();
	uint16_t get_input_bytes_advertised_for_slave();
	void render_slave_output(uint16_t data);

	void namcos21_kickstart_hacks(bool internal);

	void dspcuskey_w(uint16_t data);
	uint16_t dspcuskey_r();
	uint16_t dsp_port0_r();
	void dsp_port0_w(uint16_t data);
	uint16_t dsp_port1_r();
	void dsp_port1_w(uint16_t data);
	uint16_t dsp_port2_r();
	void dsp_port2_w(uint16_t data);
	uint16_t dsp_port3_idc_rcv_enable_r();
	void dsp_port3_w(uint16_t data);
	void dsp_port4_w(uint16_t data);
	uint16_t dsp_port8_r();
	void dsp_port8_w(uint16_t data);
	uint16_t dsp_port9_r();
	uint16_t dsp_porta_r();
	void dsp_porta_w(uint16_t data);
	uint16_t dsp_portb_r();
	void dsp_portb_w(uint16_t data);
	void dsp_portc_w(uint16_t data);
	uint16_t dsp_portf_r();
	void dsp_xf_w(uint16_t data);
	uint16_t slave_port0_r();
	void slave_port0_w(uint16_t data);
	uint16_t slave_port2_r();
	uint16_t slave_port3_r();
	void slave_port3_w(uint16_t data);
	void slave_XF_output_w(uint16_t data);
	uint16_t slave_portf_r();

	void master_dsp_data(address_map &map) ATTR_COLD;
	void master_dsp_io(address_map &map) ATTR_COLD;
	void master_dsp_program(address_map &map) ATTR_COLD;

	void slave_dsp_data(address_map &map) ATTR_COLD;
	void slave_dsp_io(address_map &map) ATTR_COLD;
	void slave_dsp_program(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(NAMCOS21_DSP_C67, namcos21_dsp_c67_device)

#endif // MAME_NAMCO_NAMCOS21_DSP_C67_H
