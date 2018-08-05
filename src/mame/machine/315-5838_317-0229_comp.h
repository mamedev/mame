// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_315_5838_371_0229_COMP_H
#define MAME_MACHINE_315_5838_371_0229_COMP_H

#pragma once

typedef device_delegate<uint16_t (uint32_t)> sega_dec_read_delegate;

DECLARE_DEVICE_TYPE(SEGA315_5838_COMP, sega_315_5838_comp_device)

#define MCFG_SET_5838_READ_CALLBACK_CH( _class, _method) \
	downcast<sega_315_5838_comp_device &>(*device).set_read_cb_ch1(sega_m2_read_delegate(&_class::_method, #_class "::" #_method, nullptr, (_class *)nullptr));

class sega_315_5838_comp_device :  public device_t
{
public:
	// construction/destruction
	sega_315_5838_comp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename Object> void set_read_cb_ch1(Object &&readcb) { m_read_ch = std::forward<Object>(readcb); }

	DECLARE_READ32_MEMBER(data_r);
	uint32_t genericdecathlt_prot_r(uint32_t mem_mask);

	void write_prot_data(uint32_t data, uint32_t mem_mask, int rev_words);

	void upload_table_data(uint16_t data);
	void set_upload_mode(uint16_t data);
	void set_prot_addr(uint32_t data, uint32_t mem_mask);

	DECLARE_WRITE32_MEMBER(data_w_doa);
	DECLARE_WRITE32_MEMBER(data_w);
	DECLARE_WRITE32_MEMBER(srcaddr_w);

	void install_doa_protection();

	DECLARE_READ32_MEMBER(data_r_doa);
	DECLARE_WRITE32_MEMBER(doa_prot_w);

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
	sega_dec_read_delegate m_read_ch;

	// Doa
	int m_protstate;
	int m_prot_a;
	uint8_t m_protram[256];
};

#endif // MAME_MACHINE_315_5838_371_0229_COMP_H
