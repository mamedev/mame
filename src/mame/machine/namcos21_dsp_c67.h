// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_VIDEO_NAMCOS21_DSP_C67_H
#define MAME_VIDEO_NAMCOS21_DSP_C67_H

#pragma once

#include "machine/namco_c67.h"
#include "video/namcos21_3d.h"

#define PTRAM_SIZE 0x20000

#define ENABLE_LOGGING      0

class namcos21_dsp_c67_device : public device_t
{
public:
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

	DECLARE_READ16_MEMBER(dspram16_r);
	DECLARE_WRITE16_MEMBER(dspram16_hack_w);
	DECLARE_WRITE16_MEMBER(dspram16_w);
	DECLARE_WRITE16_MEMBER(pointram_control_w);
	DECLARE_READ16_MEMBER(pointram_data_r);
	DECLARE_WRITE16_MEMBER(pointram_data_w);
	DECLARE_READ16_MEMBER(namcos21_depthcue_r);
	DECLARE_WRITE16_MEMBER(namcos21_depthcue_w);

	void reset_dsps(int state);
	void reset_kickstart();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;

private:
	#define DSP_BUF_MAX (4096*12)
	struct dsp_state
	{
		unsigned masterSourceAddr;
		uint16_t slaveInputBuffer[DSP_BUF_MAX];
		unsigned slaveBytesAvailable;
		unsigned slaveBytesAdvertised;
		unsigned slaveInputStart;
		uint16_t slaveOutputBuffer[DSP_BUF_MAX];
		unsigned slaveOutputSize;
		uint16_t masterDirectDrawBuffer[256];
		unsigned masterDirectDrawSize;
		int masterFinished;
		int slaveActive;
	};

	required_device<namcos21_3d_device> m_renderer;
	required_device<cpu_device> m_c67master;
	required_device_array<cpu_device,4> m_c67slave;
	required_region_ptr<int32_t> m_ptrom24;
	std::vector<uint16_t> m_dspram16;

	required_shared_ptr<uint16_t> m_master_dsp_ram;

	int m_gametype; // hacks
	devcb_write_line m_yield_hack_cb;

	std::unique_ptr<dsp_state> m_mpDspState;

	std::unique_ptr<uint8_t[]> m_pointram;
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

	void namcos21_kickstart_hacks(int internal);

	DECLARE_WRITE16_MEMBER(dspcuskey_w);
	DECLARE_READ16_MEMBER(dspcuskey_r);
	DECLARE_READ16_MEMBER(dsp_port0_r);
	DECLARE_WRITE16_MEMBER(dsp_port0_w);
	DECLARE_READ16_MEMBER(dsp_port1_r);
	DECLARE_WRITE16_MEMBER(dsp_port1_w);
	DECLARE_READ16_MEMBER(dsp_port2_r);
	DECLARE_WRITE16_MEMBER(dsp_port2_w);
	DECLARE_READ16_MEMBER(dsp_port3_idc_rcv_enable_r);
	DECLARE_WRITE16_MEMBER(dsp_port3_w);
	DECLARE_WRITE16_MEMBER(dsp_port4_w);
	DECLARE_READ16_MEMBER(dsp_port8_r);
	DECLARE_WRITE16_MEMBER(dsp_port8_w);
	DECLARE_READ16_MEMBER(dsp_port9_r);
	DECLARE_READ16_MEMBER(dsp_porta_r);
	DECLARE_WRITE16_MEMBER(dsp_porta_w);
	DECLARE_READ16_MEMBER(dsp_portb_r);
	DECLARE_WRITE16_MEMBER(dsp_portb_w);
	DECLARE_WRITE16_MEMBER(dsp_portc_w);
	DECLARE_READ16_MEMBER(dsp_portf_r);
	DECLARE_WRITE16_MEMBER(dsp_xf_w);
	DECLARE_READ16_MEMBER(slave_port0_r);
	DECLARE_WRITE16_MEMBER(slave_port0_w);
	DECLARE_READ16_MEMBER(slave_port2_r);
	DECLARE_READ16_MEMBER(slave_port3_r);
	DECLARE_WRITE16_MEMBER(slave_port3_w);
	DECLARE_WRITE16_MEMBER(slave_XF_output_w);
	DECLARE_READ16_MEMBER(slave_portf_r);

	void master_dsp_data(address_map &map);
	void master_dsp_io(address_map &map);
	void master_dsp_program(address_map &map);

	void slave_dsp_data(address_map &map);
	void slave_dsp_io(address_map &map);
	void slave_dsp_program(address_map &map);
};

DECLARE_DEVICE_TYPE(NAMCOS21_DSP_C67, namcos21_dsp_c67_device)

#endif // MAME_VIDEO_NAMCOS21_DSP_C67_H
