// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
imm4-90 High-Speed Paper Tape Reader

The monitor PROM has support for loading BNPF or Intel HEX from this
device (use J command to select it), but it doesn't appear in any
catalogues or manuals I've seen.  Apparently it was announced in
Computerworld.

In practice you needed a GPIO card (e.g. an imm4-60 or imm4-22) to talk
to the paper taper reader.  To simplify configuration we emulate the I/O
interface and paper tape reader as a single device.
*/
#ifndef MAME_BUS_INTELLEC4_TAPEREADER_H
#define MAME_BUS_INTELLEC4_TAPEREADER_H

#pragma once

#include "intellec4.h"

namespace bus { namespace intellec4 {

class imm4_90_device : public device_t, public device_univ_card_interface, public device_image_interface
{
public:
	imm4_90_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual image_init_result call_load() override;
	virtual void call_unload() override;

	virtual iodevice_t  image_type()       const noexcept override { return IO_PUNCHTAPE; }
	virtual bool        is_readable()      const noexcept override { return true; }
	virtual bool        is_writeable()     const noexcept override { return false; }
	virtual bool        is_creatable()     const noexcept override { return false; }
	virtual bool        must_be_loaded()   const noexcept override { return false; }
	virtual bool        is_reset_on_load() const noexcept override { return false; }
	virtual char const *file_extensions()  const noexcept override { return "bnpf,hex,lst,txt"; }

protected:
	virtual void device_start() override;

private:
	DECLARE_READ8_MEMBER(rom4_in) { return m_ready ? 0x07U : 0x0fU; }
	DECLARE_READ8_MEMBER(rom6_in) { return ~m_data & 0x0fU; }
	DECLARE_READ8_MEMBER(rom7_in) { return (~m_data >> 4) & 0x0fU; }
	DECLARE_WRITE8_MEMBER(rom4_out) { advance(BIT(data, 3)); }
	DECLARE_WRITE_LINE_MEMBER(advance);
	TIMER_CALLBACK_MEMBER(step);

	emu_timer   *m_step_timer;

	u8      m_data;
	bool    m_ready;
	bool    m_advance;
	bool    m_stepping;
};

} } // namespace bus::intellec4

DECLARE_DEVICE_TYPE_NS(INTELLEC4_TAPE_READER, bus::intellec4, imm4_90_device)

#endif // MAME_BUS_INTELLEC4_TAPEREADER_H
