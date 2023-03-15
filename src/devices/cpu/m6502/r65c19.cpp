// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Rockwell R65C19 Microcomputer (MCU)

    TODO: fully describe this MCU and its successors (C29, C39) and
    emulate their internal peripherals (only core emulation now)

**********************************************************************/

#include "emu.h"
#include "r65c19.h"
#include "r65c19d.h"

DEFINE_DEVICE_TYPE(R65C19, r65c19_device, "r65c19", "Rockwell R65C19 MCU")
DEFINE_DEVICE_TYPE(L2800, l2800_device, "l2800", "Rockwell L2800 MCU")

r65c19_device::r65c19_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal_map)
	: r65c02_device(mconfig, type, tag, owner, clock)
	, m_w(0)
	, m_i(0)
	, m_page1_ram(*this, "page1")
	, m_cir(0)
{
	program_config.m_internal_map = std::move(internal_map);
}

r65c19_device::r65c19_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: r65c19_device(mconfig, R65C19, tag, owner, clock, address_map_constructor())
{
}

c39_device::c39_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal_map)
	: r65c19_device(mconfig, type, tag, owner, clock, internal_map)
	, m_exp_config("expansion", ENDIANNESS_LITTLE, 8, 21, 0)
	, m_es4_config("es4", ENDIANNESS_LITTLE, 8, 9, 0)
{
}

l2800_device::l2800_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c39_device(mconfig, L2800, tag, owner, clock, address_map_constructor(FUNC(l2800_device::internal_map), this))
{
}

std::unique_ptr<util::disasm_interface> r65c19_device::create_disassembler()
{
	return std::make_unique<r65c19_disassembler>();
}

void r65c19_device::do_add(u8 v)
{
	P &= ~F_C;
	do_adc(v);
}

u16 r65c19_device::do_accumulate(u16 v, u16 w)
{
	// Compute the sum
	s32 result = s16(v) + s16(w);

	// Determine flags and saturate result upon overflow
	P &= ~(F_N | F_V);
	if (result > 32767)
	{
		P |= F_V;
		result = 32767;
	}
	else if (result < 0)
	{
		P |= F_N;
		if (result < -32768)
		{
			P |= F_V;
			result = -32768;
		}
	}

	// MPA and MPY always destroy the old value of Y, as does RND when it overflows
	Y = result & 0xff;

	// 16-bit result to W, or high byte to A
	return result & 0xffff;
}

u16 r65c19_device::get_irq_vector()
{
	// TODO: this is a stub
	return 0xfffc;
}

void r65c19_device::device_start()
{
	mintf = std::make_unique<mi_default>();

	c19_init();
}

device_memory_interface::space_config_vector c39_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &program_config),
		std::make_pair(AS_DATA, &m_exp_config),
		std::make_pair(AS_IO, &m_es4_config)
	};
}

void c39_device::device_start()
{
	std::unique_ptr<mi_banked> intf = std::make_unique<mi_banked>();
	space(AS_DATA).cache(intf->escache);
	space(AS_DATA).specific(intf->exp);
	space(AS_IO).specific(intf->es4);

	save_item(NAME(intf->bsr));
	save_item(NAME(intf->pbs));
	mintf = std::move(intf);

	c19_init();

	for (int i = 0; i < 8; i++)
		state_add(C39_BSR0 + i, string_format("BSR%d", i).c_str(), downcast<mi_banked &>(*mintf).bsr[i]);
}

void r65c19_device::c19_init()
{
	init();

	state_add(R65C19_W, "W", m_w);
	state_add<u8>(R65C19_WL, "WL",
		[this]() { return m_w & 0xff; },
		[this](u8 data) { m_w = set_l(m_w, data); }).noshow();
	state_add<u8>(R65C19_WH, "WH",
		[this]() { return m_w >> 8; },
		[this](u8 data) { m_w = set_h(m_w, data); }).noshow();
	state_add(R65C19_I, "I", m_i);

	save_item(NAME(m_w));
	save_item(NAME(m_i));
	save_item(NAME(m_cir));
}

void r65c19_device::device_reset()
{
	r65c02_device::device_reset();

	m_cir = 0x00;
}

void c39_device::device_reset()
{
	r65c02_device::device_reset();

	mi_banked &intf = downcast<mi_banked &>(*mintf);
	intf.bsr[0] = 0xe0;
	intf.bsr[1] = 0xd1;
	intf.bsr[2] = 0xb2;
	intf.bsr[3] = 0xb3;
	intf.bsr[4] = 0x74;
	intf.bsr[5] = 0x75;
	intf.bsr[6] = 0x76;
	intf.bsr[7] = 0x77;
	intf.pbs = 0xff;
}

u8 r65c19_device::page1_seg_r(offs_t offset)
{
	return m_page1_ram[(m_cir & 0x03) << 6 | offset];
}

void r65c19_device::page1_seg_w(offs_t offset, u8 data)
{
	m_page1_ram[(m_cir & 0x03) << 6 | offset] = data;
}

u8 r65c19_device::cir_r()
{
	return m_cir;
}

void r65c19_device::cir_w(u8 data)
{
	// TODO: clear interrupts
	m_cir = data & 0x07;
}

u8 c39_device::mi_banked::exp_read(u16 adr)
{
	return exp.read_byte(u32(bsr[(adr & 0xe000) >> 13]) << 13 | (adr & 0x1fff));
}

u8 c39_device::mi_banked::exp_read_cached(u16 adr)
{
	return escache.read_byte(u32(bsr[(adr & 0xe000) >> 13]) << 13 | (adr & 0x1fff));
}

void c39_device::mi_banked::exp_write(u16 adr, u8 val)
{
	exp.write_byte(u32(bsr[(adr & 0xe000) >> 13]) << 13 | (adr & 0x1fff), val);
}

u8 c39_device::mi_banked::es4_read(u16 adr)
{
	return es4.read_byte(adr & 0x1ff);
}

void c39_device::mi_banked::es4_write(u16 adr, u8 val)
{
	es4.write_byte(adr & 0x1ff, val);
}

u8 c39_device::mi_banked::read(u16 adr)
{
	return program.read_byte(adr);
}

u8 c39_device::mi_banked::read_sync(u16 adr)
{
	if (adr < 0x0600)
		return cprogram.read_byte(adr);
	else if (adr >= 0x0800 || BIT(pbs, 1))
		return exp_read_cached(adr);
	else
		return es4_read(adr);
}

u8 c39_device::mi_banked::read_arg(u16 adr)
{
	if (adr < 0x0600)
		return cprogram.read_byte(adr);
	else if (adr >= 0x0800 || BIT(pbs, 1))
		return exp_read_cached(adr);
	else
		return es4_read(adr);
}

void c39_device::mi_banked::write(u16 adr, u8 val)
{
	program.write_byte(adr, val);
}

u8 c39_device::pbs_r()
{
	return downcast<mi_banked &>(*mintf).pbs;
}

void c39_device::pbs_w(u8 data)
{
	downcast<mi_banked &>(*mintf).pbs = data;
}

u8 c39_device::bsr_r(offs_t offset)
{
	return downcast<mi_banked &>(*mintf).bsr[offset];
}

void c39_device::bsr_w(offs_t offset, u8 data)
{
	downcast<mi_banked &>(*mintf).bsr[offset] = data;
}

u8 c39_device::expansion_r(offs_t offset)
{
	mi_banked &intf = downcast<mi_banked &>(*mintf);
	if (offset >= 0x0200 || BIT(intf.pbs, 1))
		return intf.exp_read(offset + 0x0600);
	else
		return intf.es4_read(offset + 0x0600);
}

void c39_device::expansion_w(offs_t offset, u8 data)
{
	mi_banked &intf = downcast<mi_banked &>(*mintf);
	if (offset >= 0x0200 || BIT(intf.pbs, 1))
		intf.exp_write(offset + 0x0600, data);
	else
		intf.es4_write(offset + 0x0600, data);
}

void l2800_device::internal_map(address_map &map)
{
	// TODO: most registers still unimplemented
	map(0x0005, 0x0005).rw(FUNC(l2800_device::pbs_r), FUNC(l2800_device::pbs_w));
	map(0x000b, 0x000b).rw(FUNC(l2800_device::cir_r), FUNC(l2800_device::cir_w));
	map(0x0018, 0x001f).rw(FUNC(l2800_device::bsr_r), FUNC(l2800_device::bsr_w));
	map(0x0040, 0x05fd).ram(); // Page 0 has 192 dedicated bytes here
	map(0x0600, 0xffff).rw(FUNC(l2800_device::expansion_r), FUNC(l2800_device::expansion_w));
}

#include "cpu/m6502/r65c19.hxx"
