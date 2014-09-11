/*****************************************************************************

    nes.c

    Nintendo Entertainment System (Famicom)

 ****************************************************************************/

#include "emu.h"
#include "crsshair.h"
#include "cpu/m6502/m6502.h"
#include "includes/nes.h"
#include "imagedev/cartslot.h"
#include "imagedev/flopdrv.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* Set to dump info about the inputs to the errorlog */
#define LOG_JOY     0

/* Set to generate prg & chr files when the cart is loaded */
#define SPLIT_PRG   0
#define SPLIT_CHR   0

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

	memset(m_pad_latch, 0, sizeof(m_pad_latch));
	memset(m_zapper_latch, 0, sizeof(m_zapper_latch));
	m_paddle_latch = 0;
	m_paddle_btn_latch = 0;
}

//-------------------------------------------------
//  machine_start
//-------------------------------------------------

void nes_state::state_register()
{
	save_item(NAME(m_last_frame_flip));

	save_pointer(NAME(m_ciram), 0x800);

	save_item(NAME(m_pad_latch));
	save_item(NAME(m_zapper_latch));
	save_item(NAME(m_paddle_latch));
	save_item(NAME(m_paddle_btn_latch));
	save_item(NAME(m_mjpanel_latch));
	save_item(NAME(m_fck_scan));
	save_item(NAME(m_fck_mode));
	save_item(NAME(m_mic_obstruct));
	save_item(NAME(m_powerpad_latch));
	save_item(NAME(m_ftrainer_scan));
}

void nes_state::setup_ioports()
{
	for (int i = 0; i < 9; i++)
	{
		char str[7];
		sprintf(str, "FCKEY%i", i);
		m_io_fckey[i] = ioport(str);
	}
	for (int i = 0; i < 13; i++)
	{
		char str[9];
		sprintf(str, "SUBKEY%i", i);
		m_io_subkey[i] = ioport(str);
	}
	for (int i = 0; i < 4; i++)
	{
		char str[8];
		sprintf(str, "PAD%i", i + 1);
		m_io_pad[i] = ioport(str);
		sprintf(str, "MAH%i", i);
		m_io_mahjong[i] = ioport(str);
		sprintf(str, "FT_COL%i", i);
		m_io_ftrainer[i] = ioport(str);
	}
	
	m_io_ctrlsel        = ioport("CTRLSEL");
	m_io_exp            = ioport("EXP");
	m_io_paddle         = ioport("PADDLE");
	m_io_paddle_btn     = ioport("PADDLE_BUTTON");
	m_io_cc_left        = ioport("CC_LEFT");
	m_io_cc_right       = ioport("CC_RIGHT");
	m_io_zapper1_t      = ioport("ZAPPER1_T");
	m_io_zapper1_x      = ioport("ZAPPER1_X");
	m_io_zapper1_y      = ioport("ZAPPER1_Y");
	m_io_zapper2_t      = ioport("ZAPPER2_T");
	m_io_zapper2_x      = ioport("ZAPPER2_X");
	m_io_zapper2_y      = ioport("ZAPPER2_Y");
	m_io_powerpad[0]    = ioport("POWERPAD1");
	m_io_powerpad[1]    = ioport("POWERPAD2");
	m_io_disksel        = ioport("FLIPDISK");
}

void nes_state::machine_start()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	// CIRAM (Character Internal RAM)
	// NES has 2KB of internal RAM which can be used to fill the 4x1KB banks of PPU RAM at $2000-$2fff
	// Line A10 is exposed to the carts, so that games can change CIRAM mapping in PPU (we emulate this with the set_nt_mirroring
	// function). CIRAM can also be disabled by the game (if e.g. VROM or cart RAM has to be used in PPU...
	m_ciram = auto_alloc_array(machine(), UINT8, 0x800);
	// other pointers got set in the loading routine, because they 'belong' to the cart itself

	setup_ioports();

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

		m_cartslot->pcb_start(m_ciram);
		m_cartslot->m_cart->pcb_reg_postload(machine());
	}

	state_register();
}


//-------------------------------------------------
//  INPUTS
//-------------------------------------------------

READ8_MEMBER(nes_state::nes_in0_r)
{
	int cfg = m_io_ctrlsel->read();

	// Some games expect bit 6 to be set because the last entry on the data bus shows up
	// in the unused upper 3 bits, so typically a read from $4016 leaves 0x40 there.
	UINT8 ret = 0x40;
	ret |= (m_pad_latch[0] & 0x01);

	// shift
	m_pad_latch[0] >>= 1;

	// zapper
	if ((cfg & 0x000f) == 0x0002)
	{
		int x = m_zapper_latch[0][1];  // x-position
		int y = m_zapper_latch[0][2];  // y-position
		UINT32 pix, color_base;

		// get the pixel at the gun position
		pix = m_ppu->get_pixel(x, y);

		// get the color base from the ppu
		color_base = m_ppu->get_colorbase();

		// check if the cursor is over a bright pixel
		if ((pix == color_base + 0x20) || (pix == color_base + 0x30) ||
			(pix == color_base + 0x33) || (pix == color_base + 0x34))
			ret &= ~0x08; // sprite hit
		else
			ret |= 0x08;  // no sprite hit

		// light gun trigger
		ret |= (m_zapper_latch[0][0] << 4);
	}

	if (LOG_JOY)
		logerror("joy 0 read, val: %02x, pc: %04x\n", ret, space.device().safe_pc());

	return ret;
}

READ8_MEMBER(nes_state::nes_in1_r)
{
	int cfg = m_io_ctrlsel->read();

	// Some games expect bit 6 to be set because the last entry on the data bus shows up
	// in the unused upper 3 bits, so typically a read from $4017 leaves 0x40 there.
	UINT8 ret = 0x40;
	ret |= (m_pad_latch[1] & 0x01);

	// shift
	m_pad_latch[1] >>= 1;

	// zapper
	if ((cfg & 0x00f0) == 0x0020)
	{
		int x = m_zapper_latch[1][1];  // x-position
		int y = m_zapper_latch[1][2];  // y-position
		UINT32 pix, color_base;

		// get the pixel at the gun position
		pix = m_ppu->get_pixel(x, y);

		// get the color base from the ppu
		color_base = m_ppu->get_colorbase();

		// check if the cursor is over a bright pixel
		if ((pix == color_base + 0x20) || (pix == color_base + 0x30) ||
			(pix == color_base + 0x33) || (pix == color_base + 0x34))
			ret &= ~0x08; // sprite hit
		else
			ret |= 0x08;  // no sprite hit

		// light gun trigger
		ret |= (m_zapper_latch[1][0] << 4);
	}

	// arkanoid paddle
	if ((cfg & 0x00f0) == 0x0040)
	{
		ret |= (m_paddle_btn_latch << 3);   // button
		ret |= ((m_paddle_latch & 0x80) >> 3);      // paddle data
		m_paddle_latch <<= 1;
		m_paddle_latch &= 0xff;
	}

	// powerpad
	if ((cfg & 0x00f0) == 0x0050 || (cfg & 0x00f0) == 0x0060)
	{
		ret |= ((m_powerpad_latch[0] & 0x01) << 3);
		ret |= ((m_powerpad_latch[1] & 0x01) << 4);
		m_powerpad_latch[0] >>= 1;
		m_powerpad_latch[1] >>= 1;
	}

	if (LOG_JOY)
		logerror("joy 1 read, val: %02x, pc: %04x\n", ret, space.device().safe_pc());

	return ret;
}

WRITE8_MEMBER(nes_state::nes_in0_w)
{
	int cfg = m_io_ctrlsel->read();

	if (LOG_JOY)
		logerror("joy write, val: %02x, pc: %04x\n", data, space.device().safe_pc());

	// Check if lightgun has been chosen as input: if so, enable crosshair
	timer_set(attotime::zero, TIMER_ZAPPER_TICK);

	if (data & 0x01)
		return;

	// Toggling bit 0 high then low resets controllers
	m_pad_latch[0] = 0;
	m_pad_latch[1] = 0;
	m_zapper_latch[0][0] = 0;
	m_zapper_latch[0][1] = 0;
	m_zapper_latch[0][2] = 0;
	m_zapper_latch[1][0] = 0;
	m_zapper_latch[1][1] = 0;
	m_zapper_latch[1][2] = 0;
	m_paddle_btn_latch = 0;
	m_paddle_latch = 0;
	m_powerpad_latch[0] = 0;
	m_powerpad_latch[1] = 0;

	// P1 inputs
	switch (cfg & 0x000f)
	{
		case 0x01:  // pad 1
			m_pad_latch[0] = m_io_pad[0]->read();
			break;

		case 0x02:  // zapper (secondary)
			m_zapper_latch[0][0] = m_io_zapper1_t->read();
			m_zapper_latch[0][1] = m_io_zapper1_x->read();
			m_zapper_latch[0][2] = m_io_zapper1_y->read();
			break;
	}

	// P2 inputs
	switch ((cfg & 0x00f0) >> 4)
	{
		case 0x01:  // pad 2
			m_pad_latch[1] = m_io_pad[1]->read();
			break;

		case 0x02:  // zapper (primary) - most games expect pad in port1 & zapper in port2
			m_zapper_latch[1][0] = m_io_zapper2_t->read();
			m_zapper_latch[1][1] = m_io_zapper2_x->read();
			m_zapper_latch[1][2] = m_io_zapper2_y->read();
			break;

		case 0x04:  // arkanoid paddle
			m_paddle_btn_latch = m_io_paddle_btn->read();
			m_paddle_latch = (UINT8) (m_io_paddle->read() ^ 0xff);
			break;

		case 0x05:  // power pad
		case 0x06:  // power pad
			m_powerpad_latch[0] = m_io_powerpad[0]->read();
			m_powerpad_latch[1] = m_io_powerpad[1]->read() | 0xf0;
			break;
	}

	// P3 & P4 inputs in NES Four Score are read serially with P1 & P2
	// P3 inputs
	if ((cfg & 0x0f00))
		m_pad_latch[0] |= ((m_io_pad[2]->read() << 8) | (0x08 << 16));    // pad 3 + signature

	// P4 inputs
	if ((cfg & 0xf000))
		m_pad_latch[1] |= ((m_io_pad[3]->read() << 8) | (0x04 << 16));    // pad 4 + signature
}


WRITE8_MEMBER(nes_state::nes_in1_w)
{
}


READ8_MEMBER(nes_state::fc_in0_r)
{
	int cfg = m_io_ctrlsel->read();
	int exp = m_io_exp->read();

	// Some games expect bit 6 to be set because the last entry on the data bus shows up
	// in the unused upper 3 bits, so typically a read from $4016 leaves 0x40 there.
	UINT8 ret = 0x40;
	ret |= (m_pad_latch[0] & 0x01);

	// shift
	m_pad_latch[0] >>= 1;

	// microphone bit
	if ((cfg & 0x00f0) == 0x00f0)
		ret |= m_mic_obstruct;  //bit2!

	// EXP input
	switch (exp & 0x0f)
	{
		case 0x02:  // FC Keyboard: tape input
			if ((m_cassette->get_state() & CASSETTE_MASK_UISTATE) == CASSETTE_PLAY)
			{
				double level = m_cassette->input();
				if (level < 0)
					ret |= 0x00;
				else
					ret |= 0x02;
			}
			break;

		case 0x04:  // Arkanoid paddle
			ret |= (m_paddle_btn_latch << 1);   // button
			break;

		case 0x07:  // Mahjong Panel
			ret |= ((m_mjpanel_latch & 0x01) << 1);
			m_mjpanel_latch >>= 1;
			break;

		case 0x08:  // 'multitap' p3
			ret |= ((m_pad_latch[2] & 0x01) << 1);
			m_pad_latch[2] >>= 1;
			break;
	}

	if (LOG_JOY)
		logerror("joy 0 read, val: %02x, pc: %04x\n", ret, space.device().safe_pc());

	return ret;
}

READ8_MEMBER(nes_state::fc_in1_r)
{
	int exp = m_io_exp->read();

	// Some games expect bit 6 to be set because the last entry on the data bus shows up
	// in the unused upper 3 bits, so typically a read from $4017 leaves 0x40 there.
	UINT8 ret = 0x40;
	ret |= (m_pad_latch[1] & 0x01);

	// shift
	m_pad_latch[1] >>= 1;


	// EXP input
	switch (exp & 0x0f)
	{
		case 0x01:  // Lightgun
			{
				int x = m_zapper_latch[0][1];  // x-position
				int y = m_zapper_latch[0][2];  // y-position
				UINT32 pix, color_base;

				// get the pixel at the gun position
				pix = m_ppu->get_pixel(x, y);

				// get the color base from the ppu
				color_base = m_ppu->get_colorbase();

				// check if the cursor is over a bright pixel
				if ((pix == color_base + 0x20) || (pix == color_base + 0x30) ||
					(pix == color_base + 0x33) || (pix == color_base + 0x34))
					ret &= ~0x08; // sprite hit
				else
					ret |= 0x08;  // no sprite hit

				// light gun trigger
				ret |= (m_zapper_latch[0][0] << 4);
			}
			break;

		case 0x02:  // FC Keyboard: rows of the keyboard matrix are read 4-bits at time and returned as bit1->bit4
			if (m_fck_scan < 9)
				ret |= ~(((m_io_fckey[m_fck_scan]->read() >> (m_fck_mode * 4)) & 0x0f) << 1) & 0x1e;
			else
				ret |= 0x1e;
			break;

		case 0x03:  // Subor Keyboard: rows of the keyboard matrix are read 4-bits at time and returned as bit1->bit4
			if (m_fck_scan < 12)
				ret |= ~(((m_io_subkey[m_fck_scan]->read() >> (m_fck_mode * 4)) & 0x0f) << 1) & 0x1e;
			else
				ret |= 0x1e;
			break;

		case 0x04:  // Arkanoid paddle
			ret |= ((m_paddle_latch & 0x80) >> 6);      // paddle data
			m_paddle_latch <<= 1;
			m_paddle_latch &= 0xff;
			break;

		case 0x05:  // family trainer
		case 0x06:  // family trainer
			if (!BIT(m_ftrainer_scan, 0))
			{
				// read low line: buttons 9,10,11,12
				for (int i = 0; i < 4; i++)
					ret |= ((m_io_ftrainer[i]->read() & 0x01) << (1 + i));
			}
			else if (!BIT(m_ftrainer_scan, 1))
			{
				// read mid line: buttons 5,6,7,8
				for (int i = 0; i < 4; i++)
					ret |= ((m_io_ftrainer[i]->read() & 0x02) << (1 + i));
			}
			else if (!BIT(m_ftrainer_scan, 2))
			{
				// read high line: buttons 1,2,3,4
				for (int i = 0; i < 4; i++)
					ret |= ((m_io_ftrainer[i]->read() & 0x04) << (1 + i));
			}
			break;

		case 0x07:  // Mahjong Panel
			ret |= ((m_mjpanel_latch & 0x01) << 1);
			m_mjpanel_latch >>= 1;
			break;

		case 0x08:  // 'multitap' p4
			ret |= ((m_pad_latch[3] & 0x01) << 1);
			m_pad_latch[3] >>= 1;
			break;
	}

	if (LOG_JOY)
		logerror("joy 1 read, val: %02x, pc: %04x\n", ret, space.device().safe_pc());

	return ret;
}

WRITE8_MEMBER(nes_state::fc_in0_w)
{
	int cfg = m_io_ctrlsel->read();
	int exp = m_io_exp->read();

	if (LOG_JOY)
		logerror("joy write, val: %02x, pc: %04x\n", data, space.device().safe_pc());

	// Check if lightgun has been chosen as input: if so, enable crosshair
	timer_set(attotime::zero, TIMER_LIGHTGUN_TICK);

	// keyboards
	if ((exp & 0x0f) == 0x02 || (exp & 0x0f) == 0x03)
	{
		// tape output (not fully tested)
		if ((m_cassette->get_state() & CASSETTE_MASK_UISTATE) == CASSETTE_RECORD)
			m_cassette->output(((data & 0x07) == 0x07) ? +1.0 : -1.0);

		if (BIT(data, 2))   // keyboard active
		{
			int lines = ((exp & 0x0f) == 0x02) ? 9 : 12;
			UINT8 out = BIT(data, 1);   // scan

			if (m_fck_mode && !out && ++m_fck_scan > lines)
				m_fck_scan = 0;

			m_fck_mode = out;   // access lower or upper 4 bits

			if (BIT(data, 0))   // reset
				m_fck_scan = 0;
		}
	}

	// family trainer
	if ((exp & 0x0f) == 0x05 || (exp & 0x0f) == 0x06)
	{
		// select raw to scan
		m_ftrainer_scan = data & 0x07;
	}

	if (data & 0x01)
		return;

	// Toggling bit 0 high then low resets controllers
	m_pad_latch[0] = 0;
	m_pad_latch[1] = 0;
	m_pad_latch[2] = 0;
	m_pad_latch[3] = 0;
	m_zapper_latch[0][0] = 0;
	m_zapper_latch[0][1] = 0;
	m_zapper_latch[0][2] = 0;
	m_paddle_btn_latch = 0;
	m_paddle_latch = 0;
	m_mjpanel_latch = 0;
	m_mic_obstruct = 0;

	// P1 inputs
	switch (cfg & 0x000f)
	{
		case 0x01:  // pad 1
			m_pad_latch[0] = m_io_pad[0]->read();
			break;

		case 0x02:  // crazy climber (left stick)
			m_pad_latch[0] = m_io_cc_left->read();
			break;
	}

	// P2 inputs
	switch ((cfg & 0x00f0) >> 4)
	{
		case 0x01:  // pad 2
			m_pad_latch[1] = m_io_pad[1]->read();
			break;

		case 0x02:  // crazy climber (right stick)
			m_pad_latch[1] = m_io_cc_right->read();
			break;

		case 0x0f:  // pad 2 old style with microphone instead of START/SELECT keys
			// we only emulate obstruction of mic (when you blow or talk into it)
			m_mic_obstruct = m_io_pad[1]->read() & 0x04;
			m_pad_latch[1] = (m_io_pad[1]->read() & ~0x04);
			break;
	}

	// P3 & P4 inputs in Famicom (e.g. through Hori Twin Adapter or Hori 4 Players Adapter)
	// are read in parallel with P1 & P2 (just using diff bits)
	// P3 inputs
	if ((exp & 0x0f) == 8 && (cfg & 0x0f00) == 0x0100)
		m_pad_latch[2] = m_io_pad[2]->read();     // pad 3

	// P4 inputs
	if ((exp & 0x0f) == 8 && (cfg & 0xf000) == 0x1000)
		m_pad_latch[3] = m_io_pad[3]->read();     // pad 4


	// EXP input
	switch (exp & 0x0f)
	{
		case 0x01:  // Lightgun
			m_zapper_latch[0][0] = m_io_zapper2_t->read();
			m_zapper_latch[0][1] = m_io_zapper2_x->read();
			m_zapper_latch[0][2] = m_io_zapper2_y->read();
			break;

		case 0x02:  // FC Keyboard
		case 0x03:  // Subor Keyboard
			// these are scanned differently than other devices:
			// writes to $4016 with bit2 set always update the
			// line counter and writing bit0 set resets the counter
			break;

		case 0x04:  // Arkanoid paddle
			m_paddle_btn_latch = m_io_paddle_btn->read();
			m_paddle_latch = (UINT8) (m_io_paddle->read() ^ 0xff);
			break;

		case 0x05:  // family trainer
		case 0x06:  // family trainer
			// these are scanned differently than other devices:
			// bit0-bit2 of writes to $4016 select the row to read
			// from from the mat input "columns"
			break;


		case 0x07:  // Mahjong Panel
			if (data & 0xf8)
				logerror("Error: Mahjong panel read with mux data %02x\n", (data & 0xfe));
			else
				m_mjpanel_latch = m_io_mahjong[(data & 0xfe) >> 1]->read();
			break;
	}

}


void nes_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_ZAPPER_TICK:
			if ((m_io_ctrlsel->read() & 0x000f) == 0x0002)
			{
				/* enable lightpen crosshair */
				crosshair_set_screen(machine(), 0, CROSSHAIR_SCREEN_ALL);
			}
			else
			{
				/* disable lightpen crosshair */
				crosshair_set_screen(machine(), 0, CROSSHAIR_SCREEN_NONE);
			}

			if ((m_io_ctrlsel->read() & 0x00f0) == 0x0020)
			{
				/* enable lightpen crosshair */
				crosshair_set_screen(machine(), 1, CROSSHAIR_SCREEN_ALL);
			}
			else
			{
				/* disable lightpen crosshair */
				crosshair_set_screen(machine(), 1, CROSSHAIR_SCREEN_NONE);
			}
			break;
		case TIMER_LIGHTGUN_TICK:
			if ((m_io_exp->read() & 0x0f) == 0x01)
			{
				/* enable lightpen crosshair */
				crosshair_set_screen(machine(), 0, CROSSHAIR_SCREEN_ALL);
			}
			else
			{
				/* disable lightpen crosshair */
				crosshair_set_screen(machine(), 0, CROSSHAIR_SCREEN_NONE);
			}
			break;
		default:
			assert_always(FALSE, "Unknown id in nes_state::device_timer");
	}
}


DRIVER_INIT_MEMBER(nes_state,famicom)
{
	// setup alt input handlers for additional FC input devices
	address_space &space = machine().device<cpu_device>("maincpu")->space(AS_PROGRAM);
	space.install_read_handler(0x4016, 0x4016, read8_delegate(FUNC(nes_state::fc_in0_r), this));
	space.install_write_handler(0x4016, 0x4016, write8_delegate(FUNC(nes_state::fc_in0_w), this));
	space.install_read_handler(0x4017, 0x4017, read8_delegate(FUNC(nes_state::fc_in1_r), this));
}
