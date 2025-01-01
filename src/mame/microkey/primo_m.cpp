// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/*******************************************************************************

    Functions to emulate general aspects of Microkey Primo computers
    (RAM, ROM, interrupts, I/O ports)

    Krzysztof Strzecha

*******************************************************************************/

/* Core includes */
#include "emu.h"
#include "primo.h"

/* Components */
#include "cpu/z80/z80.h"
#include "screen.h"


/*******************************************************************************

    Interrupt callback

*******************************************************************************/

void primo_state::vblank_irq(int state)
{
	if (state && m_nmi)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


/*******************************************************************************

    Memory banking

*******************************************************************************/

void primo_state::update_memory()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	switch (m_port_fd & 0x03)
	{
		case 0x00:  /* Original ROM */
			space.unmap_write(0x0000, 0x3fff);
			membank("bank1")->set_base(memregion("maincpu")->base());
			break;
		case 0x01:  /* EPROM extension 1 */
			space.unmap_write(0x0000, 0x3fff);
			membank("bank1")->set_base(m_cart2_rom->base());
			break;
		case 0x02:  /* RAM */
			space.install_write_bank(0x0000, 0x3fff, membank("bank1"));
			membank("bank1")->set_base(memregion("maincpu")->base());
			break;
		case 0x03:  /* EPROM extension 2 */
			space.unmap_write(0x0000, 0x3fff);
			membank("bank1")->set_base(m_cart1_rom->base());
			break;
	}
	logerror("Memory update: %02x\n", m_port_fd);
}


/*******************************************************************************

    IO read/write handlers

*******************************************************************************/

uint8_t primo_state::be_1_r(offs_t offset)
{
	uint8_t data = 0x00;

	// bit 7, 6 - not used

	// bit 5 - VBLANK
	data |= m_screen->vblank() ? 0x20 : 0x00;

	// bit 4 - I4 (external bus)

	// bit 3 - I3 (external bus)

	// bit 2 - cassette
	data |= (m_cassette->input() < 0.1) ? 0x04 : 0x00;

	// bit 1 - reset button
	data |= (m_reset_port->read()) ? 0x02 : 0x00;

	// bit 0 - keyboard
	data |= (m_keyboard_port[(offset & 0x0030) >> 4]->read() >> (offset&0x000f)) & 0x0001 ? 0x01 : 0x00;

	return data;
}

uint8_t primo_state::be_2_r()
{
	uint8_t data = 0xff;

	// bit 7, 6 - not used

	// bit 5 - SCLK
	if (!m_iec->clk_r())
		data &= ~0x20;

	// bit 4 - SDATA
	if (!m_iec->data_r())
		data &= ~0x10;

	// bit 3 - SRQ
	if (!m_iec->srq_r())
		data &= ~0x08;

	// bit 2 - joystic 2 (not implemeted yet)

	// bit 1 - ATN
	if (!m_iec->atn_r())
		data &= ~0x02;

	// bit 0 - joystic 1 (not implemeted yet)

	logerror("IOR BE-2 data:%02x\n", data);
	return data;
}

void primo_state::ki_1_w(uint8_t data)
{
	// bit 7 - NMI generator enable/disable
	m_nmi = BIT(data, 7);

	// bit 6 - joystick register shift (not emulated)

	// bit 5 - V.24 (2) / tape control (not emulated)

	// bit 4 - speaker
	m_speaker->level_w(BIT(data, 4));

	// bit 3 - display buffer
	if (BIT(data, 3))
		m_video_memory_base |= 0x2000;
	else
		m_video_memory_base &= 0xdfff;

	// bit 2 - V.24 (1) / tape control (not emulated)

	// bit 1, 0 - cassette output
	switch (data & 0x03)
	{
		case 0:
			m_cassette->output(-1.0);
			break;
		case 1:
		case 2:
			m_cassette->output(0.0);
			break;
		case 3:
			m_cassette->output(1.0);
			break;
	}
}

void primo_state::ki_2_w(uint8_t data)
{
	// bit 7, 6 - not used

	// bit 5 - SCLK
	m_iec->host_clk_w(!BIT(data, 5));

	// bit 4 - SDATA
	m_iec->host_data_w(!BIT(data, 4));

	// bit 3 - not used

	// bit 2 - SRQ
	m_iec->host_srq_w(!BIT(data, 2));

	// bit 1 - ATN
	m_iec->host_atn_w(!BIT(data, 1));

	// bit 0 - not used

	//logerror("IOW KI-2 data:%02x\n", data);
}

void primo_state::fd_w(uint8_t data)
{
	if (!m_mem_exp_port->read())
	{
		m_port_fd = data;
		update_memory();
	}
}


/*******************************************************************************

    Machine initialization

*******************************************************************************/

void primo_state::common_machine_init()
{
	if (m_mem_exp_port->read())
		m_port_fd = 0x00;
	update_memory();
	m_maincpu->set_clock_scale(m_clock_port->read() ? 1.5 : 1.0);
}

void primo_state::machine_start()
{
	save_item(NAME(m_video_memory_base));
	save_item(NAME(m_port_fd));
	save_item(NAME(m_nmi));

	std::string region_tag;
	m_cart1_rom = memregion(region_tag.assign(m_cart1->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	m_cart2_rom = memregion(region_tag.assign(m_cart2->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
}

void primo_state::machine_reset()
{
	common_machine_init();
}


/*******************************************************************************

    Snapshot files (.pss)

*******************************************************************************/

void primo_state::setup_pss(uint8_t* snapshot_data, uint32_t snapshot_size)
{
	/* Z80 registers */
	m_maincpu->set_state_int(Z80_BC, snapshot_data[4] + snapshot_data[5]*256);
	m_maincpu->set_state_int(Z80_DE, snapshot_data[6] + snapshot_data[7]*256);
	m_maincpu->set_state_int(Z80_HL, snapshot_data[8] + snapshot_data[9]*256);
	m_maincpu->set_state_int(Z80_AF, snapshot_data[10] + snapshot_data[11]*256);
	m_maincpu->set_state_int(Z80_BC2, snapshot_data[12] + snapshot_data[13]*256);
	m_maincpu->set_state_int(Z80_DE2, snapshot_data[14] + snapshot_data[15]*256);
	m_maincpu->set_state_int(Z80_HL2, snapshot_data[16] + snapshot_data[17]*256);
	m_maincpu->set_state_int(Z80_AF2, snapshot_data[18] + snapshot_data[19]*256);
	m_maincpu->set_state_int(Z80_PC, snapshot_data[20] + snapshot_data[21]*256);
	m_maincpu->set_state_int(Z80_SP, snapshot_data[22] + snapshot_data[23]*256);
	m_maincpu->set_state_int(Z80_I, snapshot_data[24]);
	m_maincpu->set_state_int(Z80_R, snapshot_data[25]);
	m_maincpu->set_state_int(Z80_IX, snapshot_data[26] + snapshot_data[27]*256);
	m_maincpu->set_state_int(Z80_IY, snapshot_data[28] + snapshot_data[29]*256);

	/* IO ports */

	// KI-1 bit 7 - NMI generator enable/disable
	m_nmi = (snapshot_data[30] & 0x80) ? 1 : 0;

	// KI-1 bit 4 - speaker
	m_speaker->level_w(BIT(snapshot_data[30], 4));

	/* memory */
	for (u16 i = 0; i < 0xc000; i++)
		m_maincpu->space(AS_PROGRAM).write_byte(i + 0x4000, snapshot_data[i + 38]);
}

SNAPSHOT_LOAD_MEMBER(primo_state::snapshot_cb)
{
	std::vector<uint8_t> snapshot_data(image.length());

	if (image.fread(&snapshot_data[0], image.length()) != image.length())
		return std::make_pair(image_error::UNSPECIFIED, std::string());

	if (strncmp((char *)&snapshot_data[0], "PS01", 4))
		return std::make_pair(image_error::INVALIDIMAGE, std::string());

	setup_pss(&snapshot_data[0], image.length());

	return std::make_pair(std::error_condition(), std::string());
}


/*******************************************************************************

    Quicload files (.pp)

*******************************************************************************/

void primo_state::setup_pp(uint8_t* quickload_data, uint32_t quickload_size)
{
	u16 load_addr = quickload_data[0] + quickload_data[1]*256;
	u16 start_addr = quickload_data[2] + quickload_data[3]*256;

	for (u32 i = 4; i < quickload_size; i++)
		m_maincpu->space(AS_PROGRAM).write_byte(start_addr+i-4, quickload_data[i]);

	m_maincpu->set_state_int(Z80_PC, start_addr);

	logerror("Quickload .pp l: %04x r: %04x s: %04x\n", load_addr, start_addr, quickload_size-4);
}

QUICKLOAD_LOAD_MEMBER(primo_state::quickload_cb)
{
	size_t const quickload_size = image.length();
	std::vector<uint8_t> quickload_data(quickload_size);

	if (image.fread(&quickload_data[0], quickload_size) != quickload_size)
		return std::make_pair(image_error::UNSPECIFIED, std::string());

	setup_pp(&quickload_data[0], quickload_size);

	return std::make_pair(std::error_condition(), std::string());
}

u32 primo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u32 ma = m_video_memory_base;

	for (u8 y = 0; y < 192; y++)
	{
		u16 *p = &bitmap.pix(y);
		for (u16 x = 0; x < 32; x++)
		{
			u8 data = m_vram[ma+x];
			for (u8 i = 0; i < 8; i++)
				*p++ = BIT(data, i^7);
		}
		ma += 32;
	}
	return 0;
}
