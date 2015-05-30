// license:BSD-3-Clause
// copyright-holders:JJ Stacino
/*******************************************************/
/************ HECDISK2.C  in machine  ******************/
/*******************************************************/

/* Lecteur de disquette DISK II pour les machines :
        Hector HRX
        Hector MX40c
        Hector MX80c

        JJStacino  jj.stacino@aliceadsl.fr

    15/02/2010 : Start of the disc2 project! JJStacino
    26/09/2010 : first sending with bug2 (the first "dir" command finih with a crash of the Z80 disc II proc) JJStacino
    01/11/2010 : first time ending boot sequence, probleme on the CP/M lauch JJStacino
    20/11/2010 : synchronization between uPD765 and Z80 are now OK, CP/M runnig! JJStacino
    28/11/2010 : Found at Bratislava that the disk writing with TRANS X: is NOT WORKING (the exchange Hector=>Disc2 ok)
*/

#include "emu.h"

#include "sound/wave.h"         /* for K7 sound*/
#include "sound/discrete.h"     /* for 1 Bit sound*/
#include "machine/upd765.h"     /* for floppy disc controller */
#include "formats/hect_dsk.h"
#include "cpu/z80/z80.h"

#include "includes/hec2hrp.h"

/* Callback uPD request */

/* How uPD765 works:
        * First we send at uPD the string of command (p.e. 9 bytes for read starting by 0x46) on port 60h
                between each byte, check the authorization of the uPD by reading the status register
        * When the command is finish, the data arrive with DMA interrupt, then:
                If read: in port 70 to retrieve the data,
                If write: in port 70 send the data
        * When all data had been send the uPD launch an INT
        * The Z80 Disc2 writes in FF12 a flag
        * if the flag is set, end of DMA function,
        * At this point the Z80 can read the RESULT in port 61h
*/

/*****************************************************************************/
/****  Management of the interrupts (NMI and INT)between uPD765 and Z80 ******/
/*****************************************************************************/

/* upd765 INT is connected to interrupt of Z80 within a RNMI hardware authorization */
WRITE_LINE_MEMBER( hec2hrp_state::disc2_fdc_interrupt )
{
	m_IRQ_current_state = state;
	m_disc2cpu->set_input_line(INPUT_LINE_IRQ0, state && m_hector_disc2_RNMI ? ASSERT_LINE : CLEAR_LINE);
}

/* upd765 DRQ is connected to NMI of Z80 within a RNMI hardware authorization */
WRITE_LINE_MEMBER( hec2hrp_state::disc2_fdc_dma_irq )
{
	m_NMI_current_state = state;
	m_disc2cpu->set_input_line(INPUT_LINE_NMI,  state && m_hector_disc2_RNMI ? ASSERT_LINE : CLEAR_LINE);
}

// RESET the disc2 Unit !
void hec2hrp_state::hector_disc2_reset()
{
	// Initialization Disc2 unit
	m_disc2cpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
	machine().device<upd765a_device>("upd765")->reset();
	// Select ROM memory to cold restart
	membank("bank3")->set_entry(DISCII_BANK_ROM);

	// Clear the Hardware's buffers
	m_hector_disc2_data_r_ready=0x0; /* =ff when PC2 = true and data in read buffer (state->m_hector_disc2_data_read) */
	m_hector_disc2_data_w_ready=0x0; /* =ff when Disc 2 Port 40 had send a data in write buffer (state->m_hector_disc2_data_write) */
	m_hector_disc2_data_read=0;      /* Data send by Hector to Disc 2 when PC2=true */
	m_hector_disc2_data_write=0;     /* Data send by Disc 2 to Hector when Write Port I/O 40 */
	m_hector_disc2_RNMI = 0;         /* State of I/O 50 D5 = authorization for INT / NMI */
	m_IRQ_current_state=0;           /* Clear the IRQ active request */
	m_NMI_current_state=0;           /* Clear the DMA active request */
}

/*****************************************************************************/
/********************  Port handling of the Z80 Disc II unit *****************/
/*****************************************************************************/
READ8_MEMBER( hec2hrp_state::hector_disc2_io00_port_r)
{
	/* Switch Disc 2 to RAM to let full RAM acces */
	membank("bank3")->set_entry(DISCII_BANK_RAM);
	return 0;
}
WRITE8_MEMBER( hec2hrp_state::hector_disc2_io00_port_w)
{
	/* Switch Disc 2 to RAM to let full RAM acces */
	membank("bank3")->set_entry(DISCII_BANK_RAM);
}
READ8_MEMBER( hec2hrp_state::hector_disc2_io20_port_r)
{
	// You can implemente the 8251 chip communication here !
	return 0;
}
WRITE8_MEMBER( hec2hrp_state::hector_disc2_io20_port_w)
{
	// You can implemente the 8251 chip communication here !
}
READ8_MEMBER( hec2hrp_state::hector_disc2_io30_port_r)
{
	return m_hector_disc2_data_r_ready;
}
WRITE8_MEMBER( hec2hrp_state::hector_disc2_io30_port_w)
{
	// Nothing here !
}

READ8_MEMBER( hec2hrp_state::hector_disc2_io40_port_r)
{
	/* Read data send by Hector, by Disc2*/
	m_hector_disc2_data_r_ready = 0x00;  /* Clear memory info read ready*/
	return m_hector_disc2_data_read;     /* send the data !*/
}

WRITE8_MEMBER( hec2hrp_state::hector_disc2_io40_port_w)   /* Write data send by Disc2, to Hector*/
{
	m_hector_disc2_data_write = data;        /* Memorization data*/
	m_hector_disc2_data_w_ready = 0x80;  /* Memorization data write ready in D7*/
}

READ8_MEMBER( hec2hrp_state::hector_disc2_io50_port_r)    /*Read memory info write ready*/
{
	return m_hector_disc2_data_w_ready;
}

WRITE8_MEMBER( hec2hrp_state::hector_disc2_io50_port_w) /* I/O Port to the stuff of Disc2*/
{
	upd765a_device *fdc = machine().device<upd765a_device>("upd765");

	/* FDC Motor Control - Bit 0/1 defines the state of the FDD 0/1 motor */
	machine().device<floppy_connector>("upd765:0")->get_device()->mon_w(BIT(data, 0));    // Moteur floppy A:
	machine().device<floppy_connector>("upd765:1")->get_device()->mon_w(BIT(data, 1));    // Moteur floppy B:

	/* Write bit TC uPD765 on D4 of port I/O 50 */
	fdc->tc_w(BIT(data, 4));


	/* Authorization interrupt and NMI with RNMI signal*/
	m_hector_disc2_RNMI = BIT(data, 5);
	m_disc2cpu->set_input_line(INPUT_LINE_IRQ0, m_IRQ_current_state && m_hector_disc2_RNMI ? ASSERT_LINE : CLEAR_LINE);
	m_disc2cpu->set_input_line(INPUT_LINE_NMI,  m_NMI_current_state && m_hector_disc2_RNMI ? ASSERT_LINE : CLEAR_LINE);
}
