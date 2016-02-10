// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Fabio Priuli
/*****************************************************************************

    nes.c

    Nintendo Entertainment System (Famicom)

 ****************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "includes/nes.h"

/***************************************************************************
    FUNCTIONS
***************************************************************************/

// to be probably removed (it does nothing since a long time)
int nes_state::nes_ppu_vidaccess( int address, int data )
{
	return data;
}

//-------------------------------------------------
//  machine_reset
//-------------------------------------------------

void nes_state::machine_reset()
{
	// Reset the mapper variables. Will also mark the char-gen ram as dirty
	if (m_cartslot)
		m_cartslot->pcb_reset();

	m_maincpu->reset();
}

//-------------------------------------------------
//  machine_start
//-------------------------------------------------

void nes_state::machine_start()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	// CIRAM (Character Internal RAM)
	// NES has 2KB of internal RAM which can be used to fill the 4x1KB banks of PPU RAM at $2000-$2fff
	// Line A10 is exposed to the carts, so that games can change CIRAM mapping in PPU (we emulate this with the set_nt_mirroring
	// function). CIRAM can also be disabled by the game (if e.g. VROM or cart RAM has to be used in PPU...
	m_ciram = std::make_unique<UINT8[]>(0x800);
	// other pointers got set in the loading routine, because they 'belong' to the cart itself

	m_io_disksel = ioport("FLIPDISK");

	if (m_cartslot && m_cartslot->m_cart)
	{
		// Set up memory handlers
		space.install_read_handler(0x4100, 0x5fff, read8_delegate(FUNC(nes_cart_slot_device::read_l), (nes_cart_slot_device *)m_cartslot));
		space.install_write_handler(0x4100, 0x5fff, write8_delegate(FUNC(nes_cart_slot_device::write_l), (nes_cart_slot_device *)m_cartslot));
		space.install_read_handler(0x6000, 0x7fff, read8_delegate(FUNC(nes_cart_slot_device::read_m), (nes_cart_slot_device *)m_cartslot));
		space.install_write_handler(0x6000, 0x7fff, write8_delegate(FUNC(nes_cart_slot_device::write_m), (nes_cart_slot_device *)m_cartslot));
		space.install_read_bank(0x8000, 0x9fff, "prg0");
		space.install_read_bank(0xa000, 0xbfff, "prg1");
		space.install_read_bank(0xc000, 0xdfff, "prg2");
		space.install_read_bank(0xe000, 0xffff, "prg3");
		space.install_write_handler(0x8000, 0xffff, write8_delegate(FUNC(nes_cart_slot_device::write_h), (nes_cart_slot_device *)m_cartslot));

		m_ppu->space(AS_PROGRAM).install_readwrite_handler(0, 0x1fff, read8_delegate(FUNC(device_nes_cart_interface::chr_r),m_cartslot->m_cart), write8_delegate(FUNC(device_nes_cart_interface::chr_w),m_cartslot->m_cart));
		m_ppu->space(AS_PROGRAM).install_readwrite_handler(0x2000, 0x3eff, read8_delegate(FUNC(device_nes_cart_interface::nt_r),m_cartslot->m_cart), write8_delegate(FUNC(device_nes_cart_interface::nt_w),m_cartslot->m_cart));
		m_ppu->set_scanline_callback(ppu2c0x_scanline_delegate(FUNC(device_nes_cart_interface::scanline_irq),m_cartslot->m_cart));
		m_ppu->set_hblank_callback(ppu2c0x_hblank_delegate(FUNC(device_nes_cart_interface::hblank_irq),m_cartslot->m_cart));
		m_ppu->set_latch(ppu2c0x_latch_delegate(FUNC(device_nes_cart_interface::ppu_latch),m_cartslot->m_cart));

		// install additional handlers (read_h, read_ex, write_ex)
		if (m_cartslot->get_pcb_id() == STD_EXROM || m_cartslot->get_pcb_id() == STD_NROM368 || m_cartslot->get_pcb_id() == STD_DISKSYS
			|| m_cartslot->get_pcb_id() == GG_NROM || m_cartslot->get_pcb_id() == CAMERICA_ALADDIN || m_cartslot->get_pcb_id() == SUNSOFT_DCS
			|| m_cartslot->get_pcb_id() == BANDAI_DATACH || m_cartslot->get_pcb_id() == BANDAI_KARAOKE || m_cartslot->get_pcb_id() == BTL_2A03_PURITANS || m_cartslot->get_pcb_id() == AVE_MAXI15
			|| m_cartslot->get_pcb_id() == KAISER_KS7022 || m_cartslot->get_pcb_id() == KAISER_KS7031 || m_cartslot->get_pcb_id() == BMC_VT5201
			|| m_cartslot->get_pcb_id() == UNL_LH32 || m_cartslot->get_pcb_id() == UNL_LH10 || m_cartslot->get_pcb_id() == UNL_2708
			|| m_cartslot->get_pcb_id() == UNL_43272 || m_cartslot->get_pcb_id() == BMC_G63IN1 || m_cartslot->get_pcb_id() == BMC_8157
			|| m_cartslot->get_pcb_id() == BMC_GOLD150 || m_cartslot->get_pcb_id() == BMC_CH001
			|| m_cartslot->get_pcb_id() == BMC_70IN1 || m_cartslot->get_pcb_id() == BMC_800IN1)
		{
			logerror("read_h installed!\n");
			space.install_read_handler(0x8000, 0xffff, read8_delegate(FUNC(nes_cart_slot_device::read_h), (nes_cart_slot_device *)m_cartslot));
		}

		if (m_cartslot->get_pcb_id() == BTL_SMB2JB || m_cartslot->get_pcb_id() == UNL_AC08 || m_cartslot->get_pcb_id() == UNL_SMB2J || m_cartslot->get_pcb_id() == BTL_09034A)
		{
			logerror("write_ex installed!\n");
			space.install_write_handler(0x4020, 0x40ff, write8_delegate(FUNC(nes_cart_slot_device::write_ex), (nes_cart_slot_device *)m_cartslot));
		}

		if (m_cartslot->get_pcb_id() == KAISER_KS7017 || m_cartslot->get_pcb_id() == UNL_603_5052 || m_cartslot->get_pcb_id() == STD_DISKSYS)
		{
			logerror("write_ex & read_ex installed!\n");
			space.install_read_handler(0x4020, 0x40ff, read8_delegate(FUNC(nes_cart_slot_device::read_ex), (nes_cart_slot_device *)m_cartslot));
			space.install_write_handler(0x4020, 0x40ff, write8_delegate(FUNC(nes_cart_slot_device::write_ex), (nes_cart_slot_device *)m_cartslot));
		}

		m_cartslot->pcb_start(m_ciram.get());
		m_cartslot->m_cart->pcb_reg_postload(machine());
	}

	// register saves
	save_item(NAME(m_last_frame_flip));
	save_pointer(NAME(m_ciram.get()), 0x800);
}


//-------------------------------------------------
//  INPUTS
//-------------------------------------------------

READ8_MEMBER(nes_state::nes_in0_r)
{
	UINT8 ret = 0x40;
	ret |= m_ctrl1->read_bit0();
	ret |= m_ctrl1->read_bit34();
	return ret;
}

READ8_MEMBER(nes_state::nes_in1_r)
{
	UINT8 ret = 0x40;
	ret |= m_ctrl2->read_bit0();
	ret |= m_ctrl2->read_bit34();
	return ret;
}

WRITE8_MEMBER(nes_state::nes_in0_w)
{
	m_ctrl1->write(data);
	m_ctrl2->write(data);
}


READ8_MEMBER(nes_state::fc_in0_r)
{
	UINT8 ret = 0x40;
	// bit 0 to controller port
	ret |= m_ctrl1->read_bit0();

	// expansion port bits (in the original FC, P2 controller was hooked to these lines
	// too, so in principle some homebrew hardware modification could use the same
	// connection with P1 controller too)
	ret |= m_ctrl1->read_exp(0);
	ret |= m_ctrl2->read_exp(0);

	// at the same time, we might have a standard joypad connected to the expansion port which
	// shall be read as P3 (this is needed here to avoid implementing the expansion port as a
	// different device compared to the standard control port... it might be changed later)
	ret |= (m_exp->read_bit0() << 1);
	// finally, read the expansion port as expected
	ret |= m_exp->read_exp(0);
	return ret;
}

READ8_MEMBER(nes_state::fc_in1_r)
{
	UINT8 ret = 0x40;
	// bit 0 to controller port
	ret |= m_ctrl2->read_bit0();

	// expansion port bits (in the original FC, P2 controller was hooked to these lines
	// too, so in principle some homebrew hardware modification could use the same
	// connection with P1 controller too)
	ret |= m_ctrl1->read_exp(1);
	ret |= m_ctrl2->read_exp(1);

	// finally, read the expansion port as expected (standard pad cannot be hooked as P4, so
	// no read_bit0 here)
	ret |= m_exp->read_exp(1);
	return ret;
}

WRITE8_MEMBER(nes_state::fc_in0_w)
{
	m_ctrl1->write(data);
	m_ctrl2->write(data);
	m_exp->write(data);
}


DRIVER_INIT_MEMBER(nes_state,famicom)
{
	// setup alt input handlers for additional FC input devices
	address_space &space = machine().device<cpu_device>("maincpu")->space(AS_PROGRAM);
	space.install_read_handler(0x4016, 0x4016, read8_delegate(FUNC(nes_state::fc_in0_r), this));
	space.install_write_handler(0x4016, 0x4016, write8_delegate(FUNC(nes_state::fc_in0_w), this));
	space.install_read_handler(0x4017, 0x4017, read8_delegate(FUNC(nes_state::fc_in1_r), this));
}

NESCTRL_BRIGHTPIXEL_CB(nes_state::bright_pixel)
{
	// get the pixel at the gun position
	UINT32 pix = m_ppu->get_pixel(x, y);

	// get the color base from the ppu
	UINT32 color_base = m_ppu->get_colorbase();

	// check if the cursor is over a bright pixel
	if ((pix == color_base + 0x20) || (pix == color_base + 0x30) ||
		(pix == color_base + 0x33) || (pix == color_base + 0x34))
		return true;
	else
		return false;
}
