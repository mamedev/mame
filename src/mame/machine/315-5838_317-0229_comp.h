// license:BSD-3-Clause
// copyright-holders:David Haywood

#pragma once

#ifndef __SEGA315_5838_COMP__
#define __SEGA315_5838_COMP__

#define CHANNELS 2

typedef device_delegate<uint16_t (uint32_t)> sega_dec_read_delegate;

extern const device_type SEGA315_5838_COMP;

#define MCFG_SET_5838_READ_CALLBACK_CH1( _class, _method) \
	sega_315_5838_comp_device::set_read_cb_ch1(*device, sega_m2_read_delegate(&_class::_method, #_class "::" #_method, nullptr, (_class *)nullptr));

#define MCFG_SET_5838_READ_CALLBACK_CH2( _class, _method) \
	sega_315_5838_comp_device::set_read_cb_ch2(*device, sega_m2_read_delegate(&_class::_method, #_class "::" #_method, nullptr, (_class *)nullptr));

class sega_315_5838_comp_device :  public device_t
{
public:
	// construction/destruction
	sega_315_5838_comp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	sega_dec_read_delegate m_read_ch2;

	static void set_read_cb_ch1(device_t &device,sega_dec_read_delegate readcb)
	{
		sega_315_5838_comp_device &dev = downcast<sega_315_5838_comp_device &>(device);
		dev.m_channel[0].m_read_ch = readcb;
	}

	static void set_read_cb_ch2(device_t &device,sega_dec_read_delegate readcb)
	{
		sega_315_5838_comp_device &dev = downcast<sega_315_5838_comp_device &>(device);
		dev.m_channel[1].m_read_ch = readcb;
	}

	uint32_t decathlt_prot1_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t decathlt_prot2_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t genericdecathlt_prot_r(uint32_t mem_mask, int channel);

	void write_prot_data(uint32_t data, uint32_t mem_mask, int channel, int rev_words);

	void upload_table_data(uint16_t data, int channel);
	void set_upload_mode(uint16_t data, int channel);
	void set_prot_addr(uint32_t data, uint32_t mem_mask, int channel);

	void decathlt_prot1_w_doa(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void decathlt_prot1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void decathlt_prot2_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void decathlt_prot1_srcaddr_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void decathlt_prot2_srcaddr_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	void install_decathlt_protection();
	void install_doa_protection();

	uint32_t doa_prot_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void doa_prot_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint16_t m_decathlt_prottable1[24];
	uint16_t m_decathlt_dictionaryy[128];

	uint32_t m_srcoffset;

	uint32_t m_decathlt_lastcount;
	uint32_t m_decathlt_prot_uploadmode;
	uint32_t m_decathlt_prot_uploadoffset;


	// Decathlete specific variables and functions (see machine/decathlt.c)
	struct channel_type
	{
		sega_dec_read_delegate m_read_ch;

	};

	channel_type m_channel[2];



	// Doa
	int m_protstate;
	int m_prot_a;
	uint8_t m_protram[256];
};

#endif
