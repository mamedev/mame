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

	// Fill main RAM with an arbitrary pattern (alternating 0x00/0xff) for software that depends on its contents at boot up (tsk tsk!)
	// The fill value is a compromise since certain games malfunction with zero-filled memory, others with one-filled memory
	// Examples: Minna no Taabou won't boot with all 0x00, Sachen's Dancing Block won't boot with all 0xff, Terminator 2 skips its copyright screen with all 0x00
	for (int i = 0; i < 0x800; i += 2)
	{
		m_mainram[i] = 0x00;
		m_mainram[i + 1] = 0xff;
	}

	// CIRAM (Character Internal RAM)
	// NES has 2KB of internal RAM which can be used to fill the 4x1KB banks of PPU RAM at $2000-$2fff
	// Line A10 is exposed to the carts, so that games can change CIRAM mapping in PPU (we emulate this with the set_nt_mirroring
	// function). CIRAM can also be disabled by the game (if e.g. VROM or cart RAM has to be used in PPU...
	m_ciram = std::make_unique<uint8_t[]>(0x800);
	// other pointers got set in the loading routine, because they 'belong' to the cart itself

	m_io_disksel = ioport("FLIPDISK");

	if (m_cartslot && m_cartslot->m_cart)
	{
		// Set up memory handlers
		space.install_read_handler(0x4100, 0x5fff, read8sm_delegate(*m_cartslot, FUNC(nes_cart_slot_device::read_l)));
		space.install_write_handler(0x4100, 0x5fff, write8sm_delegate(*m_cartslot, FUNC(nes_cart_slot_device::write_l)));
		space.install_read_handler(0x6000, 0x7fff, read8sm_delegate(*m_cartslot, FUNC(nes_cart_slot_device::read_m)));
		space.install_write_handler(0x6000, 0x7fff, write8sm_delegate(*m_cartslot, FUNC(nes_cart_slot_device::write_m)));
		for(int i = 0; i < 4; i++)
			space.install_read_bank(0x8000 + 0x2000*i, 0x9fff + 0x2000*i, m_prg_bank[i]);
		space.install_write_handler(0x8000, 0xffff, write8sm_delegate(*m_cartslot, FUNC(nes_cart_slot_device::write_h)));

		m_ppu->space(AS_PROGRAM).install_readwrite_handler(0, 0x1fff, read8sm_delegate(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::chr_r)), write8sm_delegate(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::chr_w)));
		m_ppu->space(AS_PROGRAM).install_readwrite_handler(0x2000, 0x3eff, read8sm_delegate(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::nt_r)), write8sm_delegate(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::nt_w)));
		m_ppu->set_scanline_callback(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::scanline_irq));
		m_ppu->set_hblank_callback(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::hblank_irq));
		m_ppu->set_latch(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::ppu_latch));

		// install additional handlers (read_h, read_ex, write_ex)
		static const int r_h_pcbs[] =
		{
			AVE_MAXI15,
			BANDAI_DATACH,
			BANDAI_KARAOKE,
			BATMAP_SRRX,
			BMC_70IN1,
			BMC_800IN1,
			BMC_8157,
			BMC_970630C,
			BMC_GOLD150,
			BMC_KC885,
			BMC_TELETUBBIES,
			BMC_VT5201,
			BTL_PALTHENA,
			CAMERICA_ALADDIN,
			GG_NROM,
			KAISER_KS7010,
			KAISER_KS7022,
			KAISER_KS7030,
			KAISER_KS7031,
			KAISER_KS7037,
			KAISER_KS7057,
			SACHEN_3013,
			SACHEN_3014,
			STD_DISKSYS,
			STD_EXROM,
			STD_NROM368,
			SUNSOFT_DCS,
			UNL_2708,
			UNL_2A03PURITANS,
			UNL_43272,
			UNL_EH8813A,
			UNL_LH10,
			UNL_LH32,
			UNL_RT01
		};

		static const int w_ex_pcbs[] =
		{
			BMC_N32_4IN1,
			BTL_SMB2JB,
			BTL_YUNG08,
			UNL_AC08,
			UNL_SMB2J
		};

		static const int rw_ex_pcbs[] =
		{
			BTL_09034A,
			KAISER_KS7017,
			STD_DISKSYS,
			UNL_603_5052
		};

		int pcb_id = m_cartslot->get_pcb_id();

		if (std::find(std::begin(r_h_pcbs), std::end(r_h_pcbs), pcb_id) != std::end(r_h_pcbs))
		{
			logerror("read_h installed!\n");
			space.install_read_handler(0x8000, 0xffff, read8sm_delegate(*m_cartslot, FUNC(nes_cart_slot_device::read_h)));
		}

		if (std::find(std::begin(w_ex_pcbs), std::end(w_ex_pcbs), pcb_id) != std::end(w_ex_pcbs))
		{
			logerror("write_ex installed!\n");
			space.install_write_handler(0x4020, 0x40ff, write8sm_delegate(*m_cartslot, FUNC(nes_cart_slot_device::write_ex)));
		}

		if (std::find(std::begin(rw_ex_pcbs), std::end(rw_ex_pcbs), pcb_id) != std::end(rw_ex_pcbs))
		{
			logerror("read_ex & write_ex installed!\n");
			space.install_read_handler(0x4020, 0x40ff, read8sm_delegate(*m_cartslot, FUNC(nes_cart_slot_device::read_ex)));
			space.install_write_handler(0x4020, 0x40ff, write8sm_delegate(*m_cartslot, FUNC(nes_cart_slot_device::write_ex)));
		}

		m_cartslot->pcb_start(m_ciram.get());
		m_cartslot->m_cart->pcb_reg_postload(machine());
	}

	// register saves
	save_item(NAME(m_last_frame_flip));
	save_pointer(NAME(m_ciram), 0x800);
}


//-------------------------------------------------
//  INPUTS
//-------------------------------------------------

uint8_t nes_base_state::nes_in0_r()
{
	uint8_t ret = 0x40;
	ret |= m_ctrl1->read_bit0();
	ret |= m_ctrl1->read_bit34();
	return ret;
}

uint8_t nes_base_state::nes_in1_r()
{
	uint8_t ret = 0x40;
	ret |= m_ctrl2->read_bit0();
	ret |= m_ctrl2->read_bit34();
	return ret;
}

void nes_base_state::nes_in0_w(uint8_t data)
{
	m_ctrl1->write(data);
	m_ctrl2->write(data);
}


uint8_t nes_state::fc_in0_r()
{
	uint8_t ret = 0x40;
	// bit 0 from controller port
	ret |= m_ctrl1->read_bit0();

	// bit 2 from P2 controller microphone
	ret |= m_ctrl2->read_bit2();

	// and bit 1 comes from expansion port
	ret |= m_exp->read_exp(0);
	return ret;
}

uint8_t nes_state::fc_in1_r()
{
	uint8_t ret = 0x40;
	// bit 0 from controller port
	ret |= m_ctrl2->read_bit0();

	// bits 1-4 from expansion port (in theory bit 0 also can be read on AV Famicom when controller is unplugged)
	ret |= m_exp->read_exp(1);
	return ret;
}

void nes_state::fc_in0_w(uint8_t data)
{
	m_ctrl1->write(data);
	m_ctrl2->write(data);
	m_exp->write(data);
}


void nes_state::init_famicom()
{
	// setup alt input handlers for additional FC input devices
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_read_handler(0x4016, 0x4016, read8smo_delegate(*this, FUNC(nes_state::fc_in0_r)));
	space.install_write_handler(0x4016, 0x4016, write8smo_delegate(*this, FUNC(nes_state::fc_in0_w)));
	space.install_read_handler(0x4017, 0x4017, read8smo_delegate(*this, FUNC(nes_state::fc_in1_r)));
}
