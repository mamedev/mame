// license:BSD-3-Clause
// copyright-holders:Paul Daniels
/**********************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrups,
  I/O ports)

**********************************************************************/

#include "emu.h"
#include "includes/p2000t.h"

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
    80/40 character port 0x00

    Set bit 0 of port 0 will switch the 80 character mode on
    Clearing bit 0 of port 0 will switch the 80 character mode off and 
    thus back to 40 characters per line

*/
void p2000t_state::p2000t_port_00_w(uint8_t data)
{
    if (BIT(data, 0))
	{
        /* Switch  to 80 chars/line */
        m_screen->set_visarea(0, 80 * 12 - 1, 0, 24 * 20 - 1);
        // Set bit 0 on 0x70 indicating we are in 80 char mode
        m_port_707f |= 0x1;
    } 
    else
    {
        /* Switch back to 40 chars/line */
        m_screen->set_visarea(0, 40 * 12 - 1, 0, 24 * 20 - 1);
        // Clear bit 0 on 0x70 indicating we are in 40 char mode
        m_port_707f &= 0xFE;
    }
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
void p2000t_state::p2000t_port_303f_w(uint8_t data) 
{ 
    m_port_303f = data; 
}

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
void p2000t_state::p2000t_port_505f_w(uint8_t data) 
{ 
    m_speaker->level_w(BIT(data, 0)); 
}

/*
    DISAS 0x7x (P2000M only) 40/80 (P2000T only)

    bit 0 - 0=40 char/line mode 1=80 char/line mode  (0x70)
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
void p2000t_state::p2000t_port_707f_w(uint8_t data) 
{ 
    m_port_707f = data; 
}
uint8_t p2000t_state::p2000t_port_707f_r() 
{ 
    return m_port_707f; 
}

void p2000t_state::p2000t_port_888b_w(uint8_t data) {}
void p2000t_state::p2000t_port_8c90_w(uint8_t data) {}


void p2000t_state::p2000t_port_9494_w(uint8_t data)
{
	//  The memory region E000-FFFF (8k) is bank switched
	int available_banks = (m_ram->size() - 0xe000) / 0x2000;
	if (data < available_banks)
		m_bank->set_entry(data);
}

void p2000t_state::machine_start()
{
    auto program = &m_maincpu->space(AS_PROGRAM);
    auto ramsize = m_ram->size();
    switch(ramsize) 
    {
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

void p2000h_state::machine_start()
{
    p2000t_state::machine_start();
    m_hiresrom = memregion("hirescpu")->base();
}

/** -------------------- Hires CPU output/input ports ---------------------- **/
/* 
   HiRES channels - P2000T (main) CPU side
   0x2c  reset Hires
  
   Status channel
   0x68  channel A data address  bits 012367 (0xCF) input bits 45 (0x30) output
   0x6A  channel A control address

   Data channel
   0x69  channel B data address     (output)
   0x6B  channel B control address  (output)

 */
uint8_t p2000h_state::mainpio_pa_r_cb() 
{
    return (m_channel_a_data & 0xfe) | (m_hirespio->rdy_b() ? 0x1 : 0x0);
}
void p2000h_state::mainpio_pa_w_cb(uint8_t data) 
{
    // 00 11 00 00 (0x30) only bits 4+5 to write by P2000T rest remain as is
    m_channel_a_data = (data & 0x30) | (m_channel_a_data & ~0x30);  
    // main-PIO output is connected to hires-PIO input
    m_hirespio->strobe_a(1);
    // Clock data into Hires CPU PIO
    m_hirespio->strobe_a(0);
}
uint8_t p2000h_state::mainpio_pb_r_cb() 
{
    return m_channel_b_data;
}
void p2000h_state::mainpio_pb_w_cb(uint8_t data) 
{
    // main-PIO output is connected to hires-PIO input
    m_channel_b_data = data;
    m_hirespio->strobe_b(1);
    // Clock data into Hires CPU PIO
    m_hirespio->strobe_b(0);
}

void p2000h_state::hirespio_emulate_sync() 
{
    /* toggle bit 2 & 3 [00 00 11 00 =0xc] to emulate image syncs */
    m_channel_a_data = (m_channel_a_data & 0x0c) ? (m_channel_a_data & ~0x0c) : (m_channel_a_data | 0x0c);
}

void p2000h_state::p2000t_port_2c_w(uint8_t data)
{
    m_hires_LutRedCnt = 0;
    m_hires_LutBlueCnt = 0;
    m_hires_LutGreenCnt = 0;
    m_hirescpu->reset();
}

/* 
   HiRES channels - Hires CPU side
   
        80-8f       Red color table
		90-9f       Green color table
		a0-af       Red color table
		b0-bf       RGB-P2000T image switch
		c0-cf       Memory map
		d0-df       Scroll register
		e0-ef       Mode register

    Status channel
        0xf0  channel A data address  bits 012367 (0xCF) output bits 45 (0x30) input
        0xf2  channel A control address

    Data channel
        0xf1  channel B data address    
        0xf3  channel B control address 
  */
void p2000h_state::p2000h_port_808f_w(uint8_t data) 
{
    m_hires_LutRed[m_hires_LutRedCnt] = data << 4 | data; // Converting 4 bits colors to 8 bit colors
    m_hires_LutRedCnt = (m_hires_LutRedCnt + 1) % LUT_TABLE_SIZE;
}
void p2000h_state::p2000h_port_909f_w(uint8_t data) 
{
    m_hires_LutGreen[m_hires_LutGreenCnt] = data << 4 | data; // Converting 4 bits colors to 8 bit colors
    m_hires_LutGreenCnt = (m_hires_LutGreenCnt + 1) % LUT_TABLE_SIZE;
}
void p2000h_state::p2000h_port_a0af_w(uint8_t data) 
{
    m_hires_LutBlue[m_hires_LutBlueCnt] = data << 4 | data; // Converting 4 bits colors to 8 bit colors
    m_hires_LutBlueCnt = (m_hires_LutBlueCnt + 1) % LUT_TABLE_SIZE;
}
void p2000h_state::p2000h_port_b0bf_w(uint8_t data) 
{
    m_hires_image_select = data & 0x0F; 
}
void p2000h_state::p2000h_port_c0cf_w(uint8_t data) 
{
    m_hiresmem_bank0_ROM = (data & 0x1) ? false : true; 
}
void p2000h_state::p2000h_port_d0df_w(uint8_t data) 
{
    m_hires_scroll_reg = data; 
}
void p2000h_state::p2000h_port_e0ef_w(uint8_t data) 
{
    m_hires_image_mode = data; 
}

 /*
    Status channel
        0xf0  channel A data address  bits 012367 (0xCF) output bits 45 (0x30) input
        0xf2  channel A control address

    Data channel
        0xf1  channel B data address    
        0xf3  channel B control address 
  */
 uint8_t p2000h_state::hirespio_pa_r_cb() 
{
    return m_channel_a_data;
}
void p2000h_state::hirespio_pa_w_cb(uint8_t data) 
{
    // 11 00 00 11 (0xc3) only bits 0,1,6,7 to write by hires rest remain as is
    m_channel_a_data = (data & 0xC3) | (m_channel_a_data & ~0xC3);  
    // hires-PIO output is connected to main-PIO input
    m_mainpio->strobe_a(1);
    // Clock data into MAIN CPU PIO
    m_mainpio->strobe_a(0);
}
uint8_t p2000h_state::hirespio_pb_r_cb() 
{
    return m_channel_b_data;
}
void p2000h_state::hirespio_pb_w_cb(uint8_t data) 
{
    // hires-PIO output is connected to main-PIO input
    m_channel_b_data = data;
    m_mainpio->strobe_b(1);
    // Clock data into MAIN CPU PIO
    m_mainpio->strobe_b(0);
}
