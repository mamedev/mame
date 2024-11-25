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

#include "emu.h"
#include "tapereader.h"

#include "imagedev/papertape.h"


namespace {

class imm4_90_device
		: public paper_tape_reader_device
		, public bus::intellec4::device_univ_card_interface
{
public:
	imm4_90_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual char const *file_extensions() const noexcept override { return "bnpf,hex,lst,txt"; }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	u8 rom4_in() { return m_ready ? 0x07U : 0x0fU; }
	u8 rom6_in() { return ~m_data & 0x0fU; }
	u8 rom7_in() { return (~m_data >> 4) & 0x0fU; }
	void rom4_out(u8 data) { advance(BIT(data, 3)); }
	void advance(int state);
	TIMER_CALLBACK_MEMBER(step);

	emu_timer   *m_step_timer;

	u8      m_data;
	bool    m_ready;
	bool    m_advance;
	bool    m_stepping;
};


imm4_90_device::imm4_90_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: paper_tape_reader_device(mconfig, INTELLEC4_TAPE_READER, tag, owner, clock)
	, bus::intellec4::device_univ_card_interface(mconfig, *this)
	, m_step_timer(nullptr)
	, m_data(0xffU)
	, m_ready(false)
	, m_advance(false)
	, m_stepping(false)
{
}


std::pair<std::error_condition, std::string> imm4_90_device::call_load()
{
	m_step_timer->reset();
	m_data = 0x00U;
	m_ready = false;
	m_stepping = false;
	return std::make_pair(std::error_condition(), std::string());
}

void imm4_90_device::call_unload()
{
	m_step_timer->reset();
	m_data = 0xffU;
	m_ready = false;
	m_stepping = false;
}


void imm4_90_device::device_start()
{
	m_step_timer = timer_alloc(FUNC(imm4_90_device::step), this);

	save_item(NAME(m_data));
	save_item(NAME(m_ready));
	save_item(NAME(m_advance));
	save_item(NAME(m_stepping));

	rom_ports_space().install_read_handler(0x0040U, 0x004fU, 0x0000U, 0x1f00U, 0x0000, read8smo_delegate(*this, FUNC(imm4_90_device::rom4_in)));
	rom_ports_space().install_read_handler(0x0060U, 0x006fU, 0x0000U, 0x1f00U, 0x0000, read8smo_delegate(*this, FUNC(imm4_90_device::rom6_in)));
	rom_ports_space().install_read_handler(0x0070U, 0x007fU, 0x0000U, 0x1f00U, 0x0000, read8smo_delegate(*this, FUNC(imm4_90_device::rom7_in)));
	rom_ports_space().install_write_handler(0x0040U, 0x004fU, 0x0000U, 0x1f00U, 0x0000, write8smo_delegate(*this, FUNC(imm4_90_device::rom4_out)));
}


void imm4_90_device::advance(int state)
{
	// this is edge-sensitive - CPU sends the narrowest pulse it can
	if (!m_advance && !bool(state) && !m_stepping)
	{
		m_ready = false;
		m_stepping = true;
		m_step_timer->adjust(attotime::from_msec(5)); // 200 characters/second
	}
	m_advance = !bool(state);
}

TIMER_CALLBACK_MEMBER(imm4_90_device::step)
{
	m_stepping = false;
	if (is_loaded() && fread(&m_data, 1U))
	{
		m_ready = true;
	}
	else
	{
		m_data = 0xffU;
		m_ready = false;
	}
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(INTELLEC4_TAPE_READER, bus::intellec4::device_univ_card_interface, imm4_90_device, "intlc4_imm4_90", "Intel imm4-90 High-Speed Paper Tape Reader")
