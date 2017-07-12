// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
High-speed paper tape reader

The monitor PROM has support for loading BNPF or Intel HEX from this
device (use J command to select it), but it doesn't appear in any
catalogues or manuals I've seen.
*/
#ifndef MAME_BUS_INTELLEC4_TAPEREADER_H
#define MAME_BUS_INTELLEC4_TAPEREADER_H

#pragma once

#include "intellec4.h"

namespace bus { namespace intellec4 {

class tape_reader_device : public device_t, public device_univ_card_interface, public device_image_interface
{
public:
	tape_reader_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual image_init_result call_load() override;
	virtual void call_unload() override;

	virtual iodevice_t  image_type()       const override { return IO_PUNCHTAPE; }
	virtual bool        is_readable()      const override { return true; }
	virtual bool        is_writeable()     const override { return false; }
	virtual bool        is_creatable()     const override { return false; }
	virtual bool        must_be_loaded()   const override { return false; }
	virtual bool        is_reset_on_load() const override { return false; }
	virtual char const *file_extensions()  const override { return "bnpf,hex,lst,txt"; }

protected:
	virtual void device_start() override;

private:
	DECLARE_READ8_MEMBER(rom4_in) { return m_ready ? 0x07U : 0x0fU; }
	DECLARE_READ8_MEMBER(rom6_in) { return ~m_data & 0x0fU; }
	DECLARE_READ8_MEMBER(rom7_in) { return (~m_data >> 4) & 0x0fU; }
	DECLARE_WRITE8_MEMBER(rom4_out) { advance(BIT(data, 3)); }
	DECLARE_WRITE_LINE_MEMBER(advance);

	u8      m_data;
	bool    m_ready;
	bool    m_advance;
};

} } // namespace bus::intellec4

DECLARE_DEVICE_TYPE_NS(INTELLEC4_TAPE_READER, bus::intellec4, tape_reader_device)

#endif // MAME_BUS_INTELLEC4_TAPEREADER_H
