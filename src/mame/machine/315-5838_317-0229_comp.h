// license:BSD-3-Clause
// copyright-holders:David Haywood

#pragma once

#ifndef __SEGA315_5838_COMP__
#define __SEGA315_5838_COMP__

#define CHANNELS 2

typedef device_delegate<UINT16 (UINT32)> sega_dec_read_delegate;

extern const device_type SEGA315_5838_COMP;

#define MCFG_SET_5838_READ_CALLBACK_CH1( _class, _method) \
	sega_315_5838_comp_device::set_read_cb_ch1(*device, sega_m2_read_delegate(&_class::_method, #_class "::" #_method, NULL, (_class *)0));

#define MCFG_SET_5838_READ_CALLBACK_CH2( _class, _method) \
	sega_315_5838_comp_device::set_read_cb_ch2(*device, sega_m2_read_delegate(&_class::_method, #_class "::" #_method, NULL, (_class *)0));

class sega_315_5838_comp_device :  public device_t
{
public:
	// construction/destruction
	sega_315_5838_comp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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

	DECLARE_READ32_MEMBER(decathlt_prot1_r);
	DECLARE_READ32_MEMBER(decathlt_prot2_r);
	UINT32 genericdecathlt_prot_r(UINT32 mem_mask, int channel);

	void write_prot_data(UINT32 data, UINT32 mem_mask, int channel, int rev_words);

	void upload_table_data(UINT16 data, int channel);
	void set_upload_mode(UINT16 data, int channel);
	void set_prot_addr(UINT32 data, UINT32 mem_mask, int channel);

	DECLARE_WRITE32_MEMBER(decathlt_prot1_w_doa);
	DECLARE_WRITE32_MEMBER(decathlt_prot1_w);
	DECLARE_WRITE32_MEMBER(decathlt_prot2_w);
	DECLARE_WRITE32_MEMBER(decathlt_prot1_srcaddr_w);
	DECLARE_WRITE32_MEMBER(decathlt_prot2_srcaddr_w);

	void install_decathlt_protection();
	void install_doa_protection();

	DECLARE_READ32_MEMBER(doa_prot_r);
	DECLARE_WRITE32_MEMBER(doa_prot_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	UINT16 m_decathlt_prottable1[24];
	UINT16 m_decathlt_dictionaryy[128];

	UINT32 m_srcoffset;

	UINT32 m_decathlt_lastcount;
	UINT32 m_decathlt_prot_uploadmode;
	UINT32 m_decathlt_prot_uploadoffset;


	// Decathlete specific variables and functions (see machine/decathlt.c)
	struct channel_type
	{
		sega_dec_read_delegate m_read_ch;

	};

	channel_type m_channel[2];



	// Doa
	int m_protstate;
	int m_prot_a;
	UINT8 m_protram[256];
};

#endif
