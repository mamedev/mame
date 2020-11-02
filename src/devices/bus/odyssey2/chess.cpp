// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

Videopac C7010 Chess Module emulation
The chess engine is "Gambiet", written by Wim Rens

Hardware notes:
- NSC800 (Z80-compatible) @ 4.43MHz
- 8KB ROM, 2KB RAM

******************************************************************************/

#include "emu.h"
#include "chess.h"

DEFINE_DEVICE_TYPE(O2_ROM_CHESS, o2_chess_device, "o2_chess", "Odyssey 2 Videopac C7010")


//-------------------------------------------------
//  o2_chess_device - constructor
//-------------------------------------------------

o2_chess_device::o2_chess_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, O2_ROM_CHESS, tag, owner, clock),
	device_o2_cart_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_latch(*this, "latch%u", 0)
{ }

void o2_chess_device::device_start()
{
	save_item(NAME(m_control));
}

void o2_chess_device::cart_init()
{
	if (m_rom_size != 0x800 || m_exrom_size != 0x2000)
		fatalerror("o2_chess_device: Wrong ROM region size\n");
}


//-------------------------------------------------
//  address maps
//-------------------------------------------------

void o2_chess_device::chess_mem(address_map &map)
{
	map(0x0000, 0x1fff).r(FUNC(o2_chess_device::internal_rom_r));
	map(0xe000, 0xe7ff).mirror(0x1800).ram();
}

void o2_chess_device::chess_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).mirror(0xff).r(m_latch[1], FUNC(generic_latch_8_device::read)).w(m_latch[0], FUNC(generic_latch_8_device::write));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void o2_chess_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	NSC800(config, m_maincpu, 4.433619_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &o2_chess_device::chess_mem);
	m_maincpu->set_addrmap(AS_IO, &o2_chess_device::chess_io);

	GENERIC_LATCH_8(config, m_latch[0]);
	GENERIC_LATCH_8(config, m_latch[1]);
}


//-------------------------------------------------
//  mapper specific handlers
//-------------------------------------------------

void o2_chess_device::write_p1(u8 data)
{
	// P11: reset
	m_maincpu->set_input_line(INPUT_LINE_RESET, (data & 2) ? CLEAR_LINE : ASSERT_LINE);

	// P10,P14: must be low to access latches
	m_control = data;
}

u8 o2_chess_device::io_read(offs_t offset)
{
	if ((offset & 0xa0) == 0xa0 && (m_control & 0x11) == 0)
		return m_latch[0]->read();
	else
		return 0xff;
}

void o2_chess_device::io_write(offs_t offset, u8 data)
{
	if (offset & 0x80 && (m_control & 0x11) == 0)
		m_latch[1]->write(data);
}
