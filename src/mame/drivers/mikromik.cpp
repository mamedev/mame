// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Nokia Elektroniikka pj

    Controller ILC 9534
    FDC-Interface ILC 9530

    Parts:

    6,144 MHz xtal (CPU clock)
    18,720 MHz xtal (pixel clock)
    16 MHz xtal (FDC clock)
    Intel I8085AP (CPU)
    Intel 8253-5P (PIT)
    Intel 8275P (CRTC)
    Intel 8212P (I/OP)
    Intel 8237A-5P (DMAC)
    NEC uPD7220C (GDC)
    NEC uPD7201P (MPSC=uart)
    NEC uPD765 (FDC)
    TMS4116-15 (16Kx4 DRAM)*4 = 32KB Video RAM for 7220
    2164-6P (64Kx1 DRAM)*8 = 64KB Work RAM

    DMA channels:

    0   CRT
    1   MPSC transmit
    2   MPSC receive
    3   FDC

    Interrupts:

    INTR    MPSC INT
    RST5.5  FDC IRQ
    RST6.5  8212 INT
    RST7.5  DMA EOP

*/

/*

    TODO:

    - NEC uPD7220 GDC
    - accurate video timing
    - floppy DRQ during RECALL = 0
    - PCB layout
    - NEC uPD7201 MPSC
    - model M7 5MB hard disk

*/

#include "includes/mikromik.h"
#include "softlist.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define MMU_IOEN    0x01
#define MMU_RAMEN   0x02
#define MMU_CE4     0x08
#define MMU_CE0     0x10
#define MMU_CE1     0x20
#define MMU_CE2     0x40
#define MMU_CE3     0x80



//**************************************************************************
//  MEMORY MANAGEMENT UNIT
//**************************************************************************

//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( mm1_state::read )
{
	UINT8 data = 0;
	UINT8 mmu = m_mmu_rom->base()[(m_a8 << 8) | (offset >> 8)];

	if (mmu & MMU_IOEN)
	{
		switch ((offset >> 4) & 0x07)
		{
		case 0:
			data = m_dmac->read(space, offset & 0x0f);
			break;

		case 1:
			data = m_mpsc->cd_ba_r(space, offset & 0x03);
			break;

		case 2:
			data = m_crtc->read(space, offset & 0x01);
			break;

		case 3:
			data = m_pit->read(space, offset & 0x03);
			break;

		case 4:
			data = m_iop->read(space, 0);
			break;

		case 5:
			if (BIT(offset, 0))
			{
				data = m_fdc->fifo_r(space, 0, 0xff);
			}
			else
			{
				data = m_fdc->msr_r(space, 0, 0xff);
			}
			break;

		case 7:
			data = m_hgdc->read(space, offset & 0x01);
			break;
		}
	}
	else
	{
		if (mmu & MMU_RAMEN)
		{
			data = m_ram->pointer()[offset];
		}
		else if (!(mmu & MMU_CE0))
		{
			data = m_rom->base()[offset & 0x1fff];
		}
		else if (!(mmu & MMU_CE1))
		{
			data = m_rom->base()[0x2000 + (offset & 0x1fff)];
		}
	}

	return data;
}



//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( mm1_state::write )
{
	UINT8 mmu = m_mmu_rom->base()[(m_a8 << 8) | (offset >> 8)];

	if (mmu & MMU_IOEN)
	{
		switch ((offset >> 4) & 0x07)
		{
		case 0:
			m_dmac->write(space, offset & 0x0f, data);
			break;

		case 1:
			m_mpsc->cd_ba_w(space, offset & 0x03, data);
			break;

		case 2:
			m_crtc->write(space, offset & 0x01, data);
			break;

		case 3:
			m_pit->write(space, offset & 0x03, data);
			break;

		case 4:
			m_iop->write(space, 0, data);
			break;

		case 5:
			if (BIT(offset, 0))
			{
				m_fdc->fifo_w(space, 0, data, 0xff);
			}
			break;

		case 6:
			ls259_w(space, offset & 0x07, data);
			break;

		case 7:
			m_hgdc->write(space, offset & 0x01, data);
			break;
		}
	}
	else
	{
		if (mmu & MMU_RAMEN)
		{
			m_ram->pointer()[offset] = data;
		}
	}
}


//-------------------------------------------------
//  ls259_w -
//-------------------------------------------------

WRITE8_MEMBER( mm1_state::ls259_w )
{
	int d = BIT(data, 0);

	switch (offset)
	{
	case 0: // IC24 A8
		if (LOG) logerror("IC24 A8 %u\n", d);
		m_a8 = d;
		break;

	case 1: // RECALL
		if (LOG) logerror("RECALL %u\n", d);
		m_recall = d;
		if (d) m_fdc->soft_reset();
		break;

	case 2: // _RV28/RX21
		m_rx21 = d;
		break;

	case 3: // _TX21
		m_tx21 = d;
		break;

	case 4: // _RCL
		m_rcl = d;
		break;

	case 5: // _INTC
		m_intc = d;
		break;

	case 6: // LLEN
		if (LOG) logerror("LLEN %u\n", d);
		m_llen = d;
		break;

	case 7: // MOTOR ON
		if (LOG) logerror("MOTOR %u\n", d);
		m_floppy0->mon_w(!d);
		m_floppy1->mon_w(!d);
		break;
	}
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( mm1_map )
//-------------------------------------------------

static ADDRESS_MAP_START( mm1_map, AS_PROGRAM, 8, mm1_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(read, write)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( mm1 )
//-------------------------------------------------

static INPUT_PORTS_START( mm1 )
	// defined in machine/mm1kb.h

	PORT_START("T5")
	PORT_CONFNAME( 0x01, 0x00, "Floppy Drive Type")
	PORT_CONFSETTING( 0x00, "640 KB" )
	PORT_CONFSETTING( 0x01, "160/320 KB" )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  I8237_INTERFACE( dmac_intf )
//-------------------------------------------------

void mm1_state::update_tc()
{
	int fdc_tc = m_tc && !m_dack3;

	if (m_fdc_tc != fdc_tc)
	{
		m_fdc_tc = fdc_tc;
		m_fdc->tc_w(m_fdc_tc);
	}
}

WRITE_LINE_MEMBER( mm1_state::dma_hrq_w )
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	// Assert HLDA
	m_dmac->hack_w(state);
}

READ8_MEMBER( mm1_state::mpsc_dack_r )
{
	// clear data request
	m_dmac->dreq2_w(CLEAR_LINE);

	return 1;//m_mpsc->dtra_r();
}

WRITE8_MEMBER( mm1_state::mpsc_dack_w )
{
	//m_mpsc->hai_w(data);

	// clear data request
	m_dmac->dreq1_w(CLEAR_LINE);
}

WRITE_LINE_MEMBER( mm1_state::dma_eop_w )
{
	m_maincpu->set_input_line(I8085_RST75_LINE, state);

	m_tc = state;
	update_tc();
}

WRITE_LINE_MEMBER( mm1_state::dack3_w )
{
	m_dack3 = state;
	update_tc();
}

WRITE_LINE_MEMBER( mm1_state::itxc_w )
{
	if (!m_intc)
	{
		m_mpsc->txca_w(state);
	}
}

WRITE_LINE_MEMBER( mm1_state::irxc_w )
{
	if (!m_intc)
	{
		m_mpsc->rxca_w(state);
	}
}

WRITE_LINE_MEMBER( mm1_state::auxc_w )
{
	m_mpsc->txcb_w(state);
	m_mpsc->rxcb_w(state);
}

//-------------------------------------------------
//  UPD7201
//-------------------------------------------------

WRITE_LINE_MEMBER( mm1_state::drq2_w )
{
	if (state)
	{
		m_dmac->dreq2_w(ASSERT_LINE);
	}
}

WRITE_LINE_MEMBER( mm1_state::drq1_w )
{
	if (state)
	{
		m_dmac->dreq1_w(ASSERT_LINE);
	}
}

READ_LINE_MEMBER( mm1_state::dsra_r )
{
	return 1;
}


//-------------------------------------------------
//  upd765_interface fdc_intf
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( mm1_state::floppy_formats )
	FLOPPY_MM1_FORMAT
FLOPPY_FORMATS_END
/*
FLOPPY_FORMATS_MEMBER( mm2_state::floppy_formats )
    FLOPPY_MM2_FORMAT
FLOPPY_FORMATS_END
*/
static SLOT_INTERFACE_START( mm1_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END


//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( mm1 )
//-------------------------------------------------

void mm1_state::machine_start()
{
	// register for state saving
	save_item(NAME(m_llen));
	save_item(NAME(m_intc));
	save_item(NAME(m_rx21));
	save_item(NAME(m_tx21));
	save_item(NAME(m_rcl));
	save_item(NAME(m_recall));
}


void mm1_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	// reset LS259
	for (int i = 0; i < 8; i++)
	{
		ls259_w(program, i, 0);
	}

	// reset FDC
	m_fdc->reset();
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( mm1 )
//-------------------------------------------------

static MACHINE_CONFIG_START( mm1, mm1_state )
	// basic system hardware
	MCFG_CPU_ADD(I8085A_TAG, I8085A, XTAL_6_144MHz)
	MCFG_CPU_PROGRAM_MAP(mm1_map)
	MCFG_I8085A_SID(READLINE(mm1_state, dsra_r))
	MCFG_I8085A_SOD(DEVWRITELINE(KB_TAG, mm1_keyboard_t, bell_w))
	MCFG_QUANTUM_PERFECT_CPU(I8085A_TAG)

	// peripheral hardware
	MCFG_DEVICE_ADD(I8212_TAG, I8212, 0)
	MCFG_I8212_IRQ_CALLBACK(INPUTLINE(I8085A_TAG, I8085_RST65_LINE))
	MCFG_I8212_DI_CALLBACK(DEVREAD8(KB_TAG, mm1_keyboard_t, read))

	MCFG_DEVICE_ADD(I8237_TAG, AM9517A, XTAL_6_144MHz/2)
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(mm1_state, dma_hrq_w))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(mm1_state, dma_eop_w))
	MCFG_I8237_IN_MEMR_CB(READ8(mm1_state, read))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(mm1_state, write))
	MCFG_I8237_IN_IOR_2_CB(READ8(mm1_state, mpsc_dack_r))
	MCFG_I8237_IN_IOR_3_CB(DEVREAD8(UPD765_TAG, upd765_family_device, mdma_r))
	MCFG_I8237_OUT_IOW_0_CB(DEVWRITE8(I8275_TAG, i8275_device, dack_w))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(mm1_state, mpsc_dack_w))
	MCFG_I8237_OUT_IOW_3_CB(DEVWRITE8(UPD765_TAG, upd765_family_device, mdma_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(mm1_state, dack3_w))

	MCFG_DEVICE_ADD(I8253_TAG, PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_6_144MHz/2/2)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(mm1_state, itxc_w))
	MCFG_PIT8253_CLK1(XTAL_6_144MHz/2/2)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(mm1_state, irxc_w))
	MCFG_PIT8253_CLK2(XTAL_6_144MHz/2/2)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(mm1_state, auxc_w))

	MCFG_UPD765A_ADD(UPD765_TAG, /* XTAL_16MHz/2/2 */ true, true)
	MCFG_UPD765_INTRQ_CALLBACK(INPUTLINE(I8085A_TAG, I8085_RST55_LINE))
	MCFG_UPD765_DRQ_CALLBACK(DEVWRITELINE(I8237_TAG, am9517a_device, dreq3_w))
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":0", mm1_floppies, "525qd", mm1_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":1", mm1_floppies, "525qd", mm1_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	MCFG_UPD7201_ADD(UPD7201_TAG, XTAL_6_144MHz/2, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_RXDRQA_CB(WRITELINE(mm1_state, drq2_w))
	MCFG_Z80DART_OUT_TXDRQA_CB(WRITELINE(mm1_state, drq1_w))

	MCFG_RS232_PORT_ADD(RS232_A_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(UPD7201_TAG, z80dart_device, rxa_w))
	MCFG_RS232_PORT_ADD(RS232_B_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_PORT_ADD(RS232_C_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(UPD7201_TAG, z80dart_device, ctsb_w))

	MCFG_DEVICE_ADD(KB_TAG, MM1_KEYBOARD, 2500) // actual KBCLK is XTAL_6_144MHz/2/16
	MCFG_MM1_KEYBOARD_KBST_CALLBACK(DEVWRITELINE(I8212_TAG, i8212_device, stb_w))

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")

	// software lists
	MCFG_SOFTWARE_LIST_ADD("flop_list", "mm1_flop")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( mm1m6 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( mm1m6, mm1 )
	// video hardware
	MCFG_FRAGMENT_ADD(mm1m6_video)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( mm1m7 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( mm1m7, mm1 )
	// video hardware
	MCFG_FRAGMENT_ADD(mm1m6_video)

	// TODO hard disk
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( mm1m6 )
//-------------------------------------------------

ROM_START( mm1m6 )
	ROM_REGION( 0x4000, I8085A_TAG, 0 ) // BIOS
	ROM_LOAD( "9081b.ic2", 0x0000, 0x2000, CRC(2955feb3) SHA1(946a6b0b8fb898be3f480c04da33d7aaa781152b) )

	ROM_REGION( 0x200, "address", 0 ) // address decoder
	ROM_LOAD( "720793a.ic24", 0x0000, 0x0200, CRC(deea87a6) SHA1(8f19e43252c9a0b1befd02fc9d34fe1437477f3a) )

	ROM_REGION( 0x1000, "chargen", 0 ) // character generator
	ROM_LOAD( "6807b.ic61", 0x0000, 0x1000, CRC(32b36220) SHA1(8fe7a181badea3f7e656dfaea21ee9e4c9baf0f1) )
ROM_END


//-------------------------------------------------
//  ROM( mm1m7 )
//-------------------------------------------------

#define rom_mm1m7 rom_mm1m6



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT    COMPANY           FULLNAME                FLAGS
COMP( 1981, mm1m6,      0,      0,      mm1m6,      mm1, driver_device,     0,      "Nokia Data",       "MikroMikko 1 M6",      MACHINE_SUPPORTS_SAVE )
COMP( 1981, mm1m7,      mm1m6,  0,      mm1m7,      mm1, driver_device,     0,      "Nokia Data",       "MikroMikko 1 M7",      MACHINE_SUPPORTS_SAVE )
