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

#include "sound/sn76477.h"		/* for sn sound*/
#include "sound/wave.h"			/* for K7 sound*/
#include "sound/discrete.h"		/* for 1 Bit sound*/
#include "machine/upd765.h"		/* for floppy disc controller */
#include "formats/hect_dsk.h"
#include "cpu/z80/z80.h"

#include "includes/hec2hrp.h"

/* Disquette timer*/
static TIMER_CALLBACK( Callback_DMA_irq );
static TIMER_CALLBACK( Callback_INT_irq );

/* Callback uPD request */
//UPD765_DMA_REQUEST( hector_disc2_fdc_dma_irq );
static WRITE_LINE_DEVICE_HANDLER( disc2_fdc_interrupt );
static WRITE_LINE_DEVICE_HANDLER( hector_disc2_fdc_dma_irq );

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

// Define the hardware of the disk
 const floppy_interface hector_disc2_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(hector_disc2),
	NULL,
	NULL
};

/*****************************************************************************/
/******  Management of the uPD765 for interface with floppy images************/
/*****************************************************************************/
/* Hector Disc II uPD765 interface use interrupts and DMA! */
const upd765_interface hector_disc2_upd765_interface =
{
	DEVCB_LINE(disc2_fdc_interrupt),
	DEVCB_LINE(hector_disc2_fdc_dma_irq),
	NULL,
	UPD765_RDY_PIN_NOT_CONNECTED,
	{FLOPPY_0,FLOPPY_1, NULL, NULL}
};

/*****************************************************************************/
/****  Management of the interrupts (NMI and INT)between uPD765 and Z80 ******/
/*****************************************************************************/
static void valid_interrupt( running_machine &machine)
{
	hec2hrp_state *state = machine.driver_data<hec2hrp_state>();
	/* Called at each rising state of NMI or RNMI ! */

	/* Take NMI only if RNMI ok*/
	if ((state->m_hector_disc2_RNMI==1) &&  (state->m_NMI_current_state==1))
	{
		machine.device("disc2cpu")->execute().set_input_line(INPUT_LINE_NMI, CLEAR_LINE); // Clear NMI...
		state->m_DMA_timer->adjust(attotime::from_usec(6) );//a little time to let the Z80 terminate he's previous job !
		state->m_NMI_current_state=0;
	}
}

void hector_disc2_init( running_machine &machine)
{
	hec2hrp_state *state = machine.driver_data<hec2hrp_state>();
	state->m_DMA_timer = machine.scheduler().timer_alloc(FUNC(Callback_DMA_irq));
	state->m_INT_timer = machine.scheduler().timer_alloc(FUNC(Callback_INT_irq));
}

static TIMER_CALLBACK( Callback_DMA_irq )
{
	hec2hrp_state *state = machine.driver_data<hec2hrp_state>();
	/* To generate the NMI signal (late) when uPD DMA request*/
	machine.device("disc2cpu")->execute().set_input_line(INPUT_LINE_NMI, ASSERT_LINE);  //NMI...
	state->m_hector_nb_cde =0; // clear the cde length
}

static TIMER_CALLBACK( Callback_INT_irq )
{
	/* To generate the INT signal (late) when uPD DMA request*/
	/*device->*/machine.device("disc2cpu")->execute().set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
}

/* upd765 INT is connected to interrupt of Z80 within a RNMI hardware authorization*/
static WRITE_LINE_DEVICE_HANDLER( disc2_fdc_interrupt )
{
	hec2hrp_state *drvstate = device->machine().driver_data<hec2hrp_state>();
	device->machine().device("disc2cpu")->execute().set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	if (state)
		drvstate->m_INT_timer->adjust(attotime::from_usec(500) );//a little time to let the Z80 terminate he's previous job !
}

static WRITE_LINE_DEVICE_HANDLER( hector_disc2_fdc_dma_irq )
{
	hec2hrp_state *drvstate = device->machine().driver_data<hec2hrp_state>();
	/* upd765 DRQ is connected to NMI of Z80 within a RNMI hardware authorization*/
	/* Here the most difficult on this machine :
    The DMA request come with the uPD765 "soft" immediately,
    against the true hard uPD765. In the real life, the uPD had
    to seach for the sector before !
    So, we had to memorize the signal (the DMA is a pulse)
    until disc2's Z80 is ready to take the NMI interupt (when
    he had set the RNMI authorization) !      */

	if (state==1)
		drvstate->m_NMI_current_state = state;
	valid_interrupt(device->machine());
}

// RESET the disc2 Unit !
void hector_disc2_reset(running_machine &machine)
{
	hec2hrp_state *state = machine.driver_data<hec2hrp_state>();
	// Initialization Disc2 unit
	machine.device("disc2cpu" )->execute().set_input_line(INPUT_LINE_RESET, PULSE_LINE);
	//switch ON and OFF the reset line uPD
	upd765_reset_w(machine.device("upd765"), 1);
	upd765_reset_w(machine.device("upd765"), 0);
	// Select ROM memory to cold restart
	state->membank("bank3")->set_entry(DISCII_BANK_ROM);

	// Clear the Hardware's buffers
	state->m_hector_disc2_data_r_ready=0x0;	/* =ff when PC2 = true and data in read buffer (state->m_hector_disc2_data_read) */
	state->m_hector_disc2_data_w_ready=0x0;	/* =ff when Disc 2 Port 40 had send a data in write buffer (state->m_hector_disc2_data_write) */
	state->m_hector_disc2_data_read=0;		/* Data send by Hector to Disc 2 when PC2=true */
	state->m_hector_disc2_data_write=0;		/* Data send by Disc 2 to Hector when Write Port I/O 40 */
	state->m_hector_disc2_RNMI = 0;			/* State of I/O 50 D5 = authorization for INT / NMI */
	state->m_NMI_current_state=0;			/* Clear the DMA active request */
}

/*****************************************************************************/
/********************  Port handling of the Z80 Disc II unit *****************/
/*****************************************************************************/
READ8_HANDLER( hector_disc2_io00_port_r)
{
	hec2hrp_state *state = space.machine().driver_data<hec2hrp_state>();
	/* Switch Disc 2 to RAM to let full RAM acces */
	state->membank("bank3")->set_entry(DISCII_BANK_RAM);
	return 0;
}
WRITE8_HANDLER( hector_disc2_io00_port_w)
{
	hec2hrp_state *state = space.machine().driver_data<hec2hrp_state>();
	/* Switch Disc 2 to RAM to let full RAM acces */
	state->membank("bank3")->set_entry(DISCII_BANK_RAM);
}
READ8_HANDLER( hector_disc2_io20_port_r)
{
	// You can implemente the 8251 chip communication here !
	return 0;
}
WRITE8_HANDLER( hector_disc2_io20_port_w)
{
	// You can implemente the 8251 chip communication here !
}
READ8_HANDLER( hector_disc2_io30_port_r)
{
	hec2hrp_state *state = space.machine().driver_data<hec2hrp_state>();
	return state->m_hector_disc2_data_r_ready;
}
WRITE8_HANDLER( hector_disc2_io30_port_w)
{
	// Nothing here !
}

READ8_HANDLER( hector_disc2_io40_port_r)
{
	hec2hrp_state *state = space.machine().driver_data<hec2hrp_state>();
	/* Read data send by Hector, by Disc2*/
	state->m_hector_disc2_data_r_ready = 0x00;	/* Clear memory info read ready*/
	return state->m_hector_disc2_data_read;		/* send the data !*/
}

WRITE8_HANDLER( hector_disc2_io40_port_w)	/* Write data send by Disc2, to Hector*/
{
	hec2hrp_state *state = space.machine().driver_data<hec2hrp_state>();
	state->m_hector_disc2_data_write = data;		/* Memorization data*/
	state->m_hector_disc2_data_w_ready = 0x80;	/* Memorization data write ready in D7*/
}

READ8_HANDLER( hector_disc2_io50_port_r)	/*Read memory info write ready*/
{
	hec2hrp_state *state = space.machine().driver_data<hec2hrp_state>();
	return state->m_hector_disc2_data_w_ready;
}

WRITE8_HANDLER( hector_disc2_io50_port_w) /* I/O Port to the stuff of Disc2*/
{
	hec2hrp_state *state = space.machine().driver_data<hec2hrp_state>();
	device_t *fdc = space.machine().device("upd765");

	/* FDC Motor Control - Bit 0/1 defines the state of the FDD 0/1 motor */
	floppy_mon_w(floppy_get_device(space.machine(), 0), BIT(data, 0));	// Moteur floppy A:
	floppy_mon_w(floppy_get_device(space.machine(), 1), BIT(data, 1));	// Moteur floppy B:
	floppy_drive_set_ready_state(floppy_get_device(space.machine(), 0), FLOPPY_DRIVE_READY,!BIT(data, 0));
	floppy_drive_set_ready_state(floppy_get_device(space.machine(), 1), FLOPPY_DRIVE_READY,!BIT(data, 1));

	/* Write bit TC uPD765 on D4 of port I/O 50 */
	upd765_tc_w(fdc, BIT(data, 4));  // Seems not used...


	/* Authorization interrupt and NMI with RNMI signal*/
	state->m_hector_disc2_RNMI = BIT(data, 5);

	/* if RNMI is OK, try to lauch an NMI*/
	if (state->m_hector_disc2_RNMI)
		valid_interrupt(space.machine());
}

//Here we must take the exchange with uPD against AM_DEVREADWRITE
// Because we had to add D6 = 1 when write is done with 0x28 in ST0 back

//  AM_RANGE(0x061,0x061) AM_DEVREADWRITE("upd765",upd765_data_r,upd765_data_w)
READ8_HANDLER( hector_disc2_io61_port_r)
{
	hec2hrp_state *state = space.machine().driver_data<hec2hrp_state>();
	UINT8 data;
	device_t *fdc = space.machine().device("upd765");
	data = upd765_data_r(fdc,space, 0); //Get the result

// if ST0 == 0x28 (drive A:) or 0x29 (drive B:) => add 0x40
// and correct the ST1 and ST2 (patch)
	if ((state->m_hector_flag_result == 3) & ((data==0x28) | (data==0x29)) ) // are we in the problem?
	{
		data=data + 0x40;
		state->m_hector_flag_result--;
	}
	// Nothing to do in over case!
	if (state->m_hector_flag_result == 3)
		state->m_hector_flag_result = 0;

	if ((state->m_hector_flag_result == 2) & (data==0x00) )
	{
		data=/*data +*/ 0x04;
		state->m_hector_flag_result--;
	}
	if ((state->m_hector_flag_result == 1) & (data==0x00) )
	{
		data=/*data + */0x10;
		state->m_hector_flag_result=0; // End !
	}
	#ifdef hector_trace
	if (state->m_print==1)
		printf(" _%x",data);
	#endif
	state->m_hector_nb_cde =0; // clear the cde length
	return data;
}
WRITE8_HANDLER( hector_disc2_io61_port_w)
{
	hec2hrp_state *state = space.machine().driver_data<hec2hrp_state>();
	/* Data useful to patch the RESULT in case of write command */
	state->m_hector_cmd[9]=state->m_hector_cmd[8];  //hector_cmd_8 = Cde number when state->m_hector_nb_cde = 9
	state->m_hector_cmd[8]=state->m_hector_cmd[7];  //hector_cmd_7 = Drive
	state->m_hector_cmd[7]=state->m_hector_cmd[6];  //hector_cmd_6 = C
	state->m_hector_cmd[6]=state->m_hector_cmd[5];  //hector_cmd_5 = H
	state->m_hector_cmd[5]=state->m_hector_cmd[4];  //hector_cmd_4 = R
	state->m_hector_cmd[4]=state->m_hector_cmd[3];  //hector_cmd_3 = N
	state->m_hector_cmd[3]=state->m_hector_cmd[2];  //hector_cmd_2 = EOT
	state->m_hector_cmd[2]=state->m_hector_cmd[1];  //hector_cmd_1 = GPL
	state->m_hector_cmd[1]=state->m_hector_cmd[0];  //hector_cmd_0 = DTL
	state->m_hector_cmd[0] = data;
	// Increase the length cde!
	state->m_hector_nb_cde++;

	// check if current commande is write cmde.
	if (((state->m_hector_cmd[8] & 0x1f)== 0x05)  & (state->m_hector_nb_cde==9) ) /*Detect wrtie commande*/
		state->m_hector_flag_result = 3; // here we are!
#ifdef hector_trace
	if (state->m_hector_nb_cde==6 ) /*Detect 1 octet command*/
	{
		printf("\n commande = %x, %x, %x, %x, %x, %x Result = ", state->m_hector_cmd[5], state->m_hector_cmd[4], state->m_hector_cmd[3], state->m_hector_cmd[2], state->m_hector_cmd[1], data );
		state->m_print=1;
	}
	else
		state->m_print=0;
#endif

	device_t *fdc = space.machine().device("upd765");
	upd765_data_w(fdc,space, 0, data);
}

//  AM_RANGE(0x070,0x07f) AM_DEVREADWRITE("upd765",upd765_dack_r,upd765_dack_w)
READ8_HANDLER( hector_disc2_io70_port_r) // Gestion du DMA
{
	UINT8 data;
	device_t *fdc = space.machine().device("upd765");
	data = upd765_dack_r(fdc,space, 0);
	return data;
}
WRITE8_HANDLER( hector_disc2_io70_port_w)
{
	device_t *fdc = space.machine().device("upd765");
	upd765_dack_w(fdc,space, 0, data);
}
