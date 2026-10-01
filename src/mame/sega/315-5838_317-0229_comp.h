// license:BSD-3-Clause
// copyright-holders:David Haywood, Samuel Neves, Peter Wilhelmsen, Morten Shearman Kirkegaard
#ifndef MAME_SEGA_315_5838_317_0229_COMP_H
#define MAME_SEGA_315_5838_317_0229_COMP_H

#pragma once

#include "devcb.h"

DECLARE_DEVICE_TYPE(SEGA315_5838_COMP, sega_315_5838_comp_device)

class sega_315_5838_comp_device : public device_t
{
public:
	enum class variant : u8
	{
		SEGA_315_5838,
		SEGA_317_0229,
		SEGA_317_0230,
		SEGA_317_0231
	};

	sega_315_5838_comp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// The callback takes a word address and returns a logical encrypted word.
	void set_variant(variant type) { m_variant = type; }
	auto source_callback() { return m_source_cb.bind(); }

	void source_w(u32 data, u32 mem_mask = ~0U);
	void table_w(offs_t offset, u16 data);
	u16 data_r();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	struct cipher_parameters
	{
		u8 routing[4];
		u8 sboxes[4][8];
		u8 affine[4];
	};

	variant m_variant = variant::SEGA_315_5838;
	devcb_read16 m_source_cb;

	u16 m_tree[24]{};
	u8 m_dictionary[256]{};
	u8 m_tree_words = 0;
	u16 m_dictionary_bytes = 0;
	bool m_upload_dictionary = false;

	u32 m_source = 0;
	u16 m_word = 0;
	u8 m_bits = 0;
	u16 m_output = 0xffff;
	bool m_abort = false;

	u16 decipher(u16 ciphertext) const;
	u8 decompress_byte();
};

#endif // MAME_SEGA_315_5838_317_0229_COMP_H
