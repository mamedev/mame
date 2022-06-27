// license:BSD-3-Clause
// copyright-holders:Paul Daniels
/**********************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrups,
  I/O ports)

**********************************************************************/

#include "emu.h"
#include "p2000t.h"

#define P2000M_101F_CASDAT 0x01
#define P2000M_101F_CASCMD 0x02
#define P2000M_101F_CASREW 0x04
#define P2000M_101F_CASFOR 0x08
#define P2000M_101F_KEYINT 0x40
#define P2000M_101F_PRNOUT 0x80

#define P2000M_202F_PINPUT 0x01
#define P2000M_202F_PREADY 0x02
#define P2000M_202F_STRAPN 0x04
#define P2000M_202F_CASENB 0x08
#define P2000M_202F_CASPOS 0x10
#define P2000M_202F_CASEND 0x20
#define P2000M_202F_CASCLK 0x40
#define P2000M_202F_CASDAT 0x80

#define P2000M_303F_VIDEO 0x01

#define P2000M_707F_DISA 0x01

/*
    Keyboard port 0x0x

    If the keyboard interrupt is enabled, all keyboard matrix rows are
    connected and reading from either of these ports will give the
    keyboard status (FF=no key pressed)

    If the keyboard interrupt is disabled, reading one of these ports
    will read the corresponding keyboard matrix row
*/
uint8_t p2000t_state::p2000t_port_000f_r(offs_t offset)
{
	if (m_port_101f & P2000M_101F_KEYINT)
	{
		return (m_keyboard[0]->read() & m_keyboard[1]->read() & m_keyboard[2]->read()
				& m_keyboard[3]->read() & m_keyboard[4]->read() & m_keyboard[5]->read()
				& m_keyboard[6]->read() & m_keyboard[7]->read() & m_keyboard[8]->read()
				& m_keyboard[9]->read());
	}
	else if (offset < 10)
	{
		return m_keyboard[offset]->read();
	}
	else
		return 0xff;
}

/*
    Input port 0x2x

    bit 0 - Printer input
    bit 1 - Printer ready
    bit 2 - Strap N (daisy/matrix)
    bit 3 - Cassette write enabled, 0 = Write enabled
    bit 4 - Cassette in position,   0 = Cassette in position
    bit 5 - Begin/end of tape       0 = Beginning or End of tap
    bit 6 - Cassette read clock     Flips when a bit is available.
    bit 7 - Cassette read data

    Note: bit 6 & 7 are swapped when the cassette is moving in reverse.
*/
uint8_t p2000t_state::p2000t_port_202f_r()
{
	uint8_t data = 0x00;
	data |= !m_mdcr->wen() << 3;
	data |= !m_mdcr->cip() << 4;
	data |= !m_mdcr->bet() << 5;
	data |= m_mdcr->rdc() << 6;
	data |= !m_mdcr->rda() << 7;
	return data;
}


/*
    Output Port 0x1x

    bit 0 - Cassette write data
    bit 1 - Cassette write command
    bit 2 - Cassette rewind
    bit 3 - Cassette forward
    bit 4 - Unused
    bit 5 - Unused
    bit 6 - Keyboard interrupt enable
    bit 7 - Printer output
*/
void p2000t_state::p2000t_port_101f_w(uint8_t data)
{
	m_port_101f = data;
	m_mdcr->wda(BIT(data, 0));
	m_mdcr->wdc(BIT(data, 1));
	m_mdcr->rev(BIT(data, 2));
	m_mdcr->fwd(BIT(data, 3));
}

/*
    Scroll Register 0x3x (P2000T only)

    bit 0 - /
    bit 1 - |
    bit 2 - | Index of the first character
    bit 3 - | to be displayed
    bit 4 - |
    bit 5 - |
    bit 6 - \
    bit 7 - Video disable (0 = enabled)
*/
void p2000t_state::p2000t_port_303f_w(uint8_t data) { m_port_303f = data; }

/*
    Beeper 0x5x

    bit 0 - Beeper
    bit 1 - Unused
    bit 2 - Unused
    bit 3 - Unused
    bit 4 - Unused
    bit 5 - Unused
    bit 6 - Unused
    bit 7 - Unused
*/
void p2000t_state::p2000t_port_505f_w(uint8_t data) { m_speaker->level_w(BIT(data, 0)); }

/*
    DISAS 0x7x (P2000M only)

    bit 0 - Unused
    bit 1 - DISAS enable
    bit 2 - Unused
    bit 3 - Unused
    bit 4 - Unused
    bit 5 - Unused
    bit 6 - Unused
    bit 7 - Unused

    When the DISAS is active, the CPU has the highest priority and
    video refresh is disabled when the CPU accesses video memory

*/
void p2000t_state::p2000t_port_707f_w(uint8_t data) { m_port_707f = data; }

void p2000t_state::p2000t_port_888b_w(uint8_t data) {}
void p2000t_state::p2000t_port_8c90_w(uint8_t data) {}


void p2000t_state::p2000t_port_9494_w(uint8_t data) {
	//  The memory region E000-FFFF (8k) is bank switched
	int available_banks = (m_ram->size() - 0xe000) / 0x2000;
	if (data < available_banks)
		m_bank->set_entry(data);
}

void p2000t_state::machine_start()
{
		auto program = &m_maincpu->space(AS_PROGRAM);
		auto ramsize = m_ram->size();
		switch(ramsize) {
			case 0x4000: // 16kb
				program->unmap_readwrite(0xa000, 0xffff);
				break;
			case 0x8000: // 32kb
				program->unmap_readwrite(0xe000, 0xffff);
				break;
			default: // more.. (48kb, 64kb, 102kb)
				// In this case we have a set of 8kb memory banks.
				uint8_t *ram = m_ram->pointer();
				auto available_banks = (ramsize - 0xe000) / 0x2000;
				for(int i = 0; i < available_banks; i++)
					m_bank->configure_entry(i, ram + (i * 0x2000));
				break;
		}
}
