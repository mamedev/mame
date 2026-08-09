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

    Models:

    M1: 1x 160KB floppy
    M2: 2x 160KB floppy
    M3: 1x 320KB floppy (96 tpi, single sided)
    M4: 2x 320KB floppy
    M5: 1x 640KB floppy (96 tpi, double sided)
    M6: 2x 640KB floppy
    M7: 1x 640KB floppy + 5MB hard disk
    M4G: 2x 320KB floppy + GDC
    M6G: 2x 640KB floppy + GDC
    M7G: 1x 640KB floppy + 5MB hard disk + GDC


    ./chdman createhd -chs 306,2,32 -ss 256 -o st406.chd

*/

/*

PCB Layout
----------

NOKIA 9924570 VDU200A MIKROMIKKO

|-------------------------------------------------------|
|       RAM     ROM1    LS393   LS08    LS14    PIT     |-
|       RAM             LS164                           ||
|       RAM                     MPSC            IOP     ||CON3
|       RAM             LS132                           ||
|       RAM             LS244   LS32    LS157   75188   |-
|       RAM                                             |-
|       RAM             PROM                    MC1489A ||CON4
|       RAM     LS373   LS373   LS138   LS125   MC1489A |-
|                                                       |-
|       LS157               LS259   LS125               ||
|       LS157   CPU      18.72MHz   LS11    LS74        ||CON5
|       LS04                                            ||
|F      6.144MHz DMA        LS04    LS163   LS32 75188  |-
|-      LS163   LS374   LS374   LS02    LS175   LS08    |-
||CON1  LS74    CRTC    LS374   ROM2    LS374           ||CON6
|-             |-CON2-|                                 |-
|-------------------------------------------------------|

Notes:
    All IC's shown.

    RAM     - TMM4164P-3
    ROM1    - NEC D2764D "9057C"
    ROM2    - NEC D2732D "6807B"
    PROM    - "726972B"
    CPU     - Intel P8085AH
    DMA     - NEC D8237AC-5
    CRTC    - Intel P8275
    PIT     - NEC D8253C-5
    MPSC    - NEC D7201-C
    IOP     - NEC uPB7212C
    CON1    - 1x5 power connector
    CON2    - 1x20 PCB header for floppy/SASI interface
    CON3    - DB15 keyboard connector
    CON4    - DB9 printer connector
    CON5    - DB25 RS232 connector
    CON6    - DB9 video connector
    F       - 2.5A fuse


NOKIA 9924494 C

|-----------------------|
|   |-----CON1------|   |
| LS14  7406    7445    |
|       LS123           |
|                       |
| 74368 7474    7445    |
| LS132 LS08    LS04    |
|       LS74    74221   |
| LS112                 |
| LS157                 |
| LS393                 |
| LS629         FDC     |
| LS393                 |
| 16MHz                 |
|-----------------------|

Notes:
    All IC's shown.

    CON1    - 2x22 PCB header for floppy
    FDC     - NEC D765AC


NOKIA 9924913 Z

|---------------------------------------|
| 7438  -          |-----CON1------|    |
| 7438  |       LS14    7406    7445    |
|       |               LS123           |
| DIP   |CON2                           |
|       |       74368   7474    7445    |
| 7438  |       LS132   LS08    LS04    |
| 7438  -               LS74    74221   |
| LS240         LS112                   |
| LS240         LS157                   |
| LS374         LS393                   |
| LS374         LS629           FDC     |
| LS373         LS393                   |
| LS74  LS04    16MHz                   |
| LS74  LS74    LS139   LS32    LS257   |
| LS08  LS257                           |
|---------------------------------------|

Notes:
    All IC's shown.

    CON1    - 2x22 PCB header for floppy
    CON2    - 2x25 PCB header for SASI interface
    FDC     - NEC D765AC


NOKIA 9923204 C

|-----------------------------------------------------------------------------------|
|       PROM    LS14    XTAL                LS163   LS163           LS123   7438    |
|                                           LS42    LS151           LS162           |
|-----------------------------------------------------------------------------------|

Notes:
    All IC's shown.

    PROM    - 6349-1J
    XTAL    - unknown value

*/

/*

    TODO

    - floppy does not work in mm1m7

*/

#include "emu.h"
#include "mikromik.h"
#include "softlist_dev.h"

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

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

uint8_t mm1_state::read(offs_t offset)
{
	uint8_t data = 0;
	uint8_t mmu = m_mmu_rom->base()[(m_a8 << 8) | (offset >> 8)];

	if (mmu & MMU_IOEN)
	{
		data = m_io->read8(offset & 0x7f);
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

void mm1_state::write(offs_t offset, uint8_t data)
{
	uint8_t mmu = m_mmu_rom->base()[(m_a8 << 8) | (offset >> 8)];

	if (mmu & MMU_IOEN)
	{
		m_io->write8(offset & 0x7f, data);
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
//  recall_w -
//-------------------------------------------------

void mm1_state::recall_w(int state)
{
	LOG("RECALL %u\n", state);

	m_recall = state;
	m_fdc->reset_w(state);

	if (m_recall)
	{
		m_dmac->dreq3_w(false);
	}
}


//-------------------------------------------------
//  motor_on_w -
//-------------------------------------------------

void mm1_state::motor_on_w(int state)
{
	LOG("MOTOR %u\n", state);

	m_floppy[0]->mon_w(!state);

	if (m_floppy[1])
	{
		m_floppy[1]->mon_w(!state);
	}
}


//-------------------------------------------------
//  switch_w -
//-------------------------------------------------

void mm1_state::switch_w(int state)
{
	LOG("SWITCH %u\n", state);

	m_switch = state;

	m_fdc_view.select(state);
	m_floppy[0]->mon_w(state);
}

uint8_t mm1_state::sasi_status_r(offs_t offset)
{
	uint8_t data = 0;

	data |= m_sasi->bsy_r();
	data |= m_sasi->rst_r() << 1;
	data |= m_sasi->msg_r() << 2;
	data |= m_sasi->cd_r() << 3;
	data |= m_sasi->req_r() << 4;
	data |= m_sasi->io_r() << 5;
	data |= m_sasi->ack_r() << 7;

	//LOG("%s SASI STATUS %02x\n",machine().describe_context(),data);

	return data;
}

void mm1_state::sasi_cmd_w(offs_t offset, uint8_t data)
{
	LOG("%s SASI CMD %02x\n", machine().describe_context(), data);

	m_sasi->sel_w(BIT(data, 0));
	m_sasi->rst_w(BIT(data, 1));
	m_sasi->atn_w(BIT(data, 2));
}

uint8_t mm1_state::sasi_data_r(offs_t offset)
{
	uint8_t data = m_sasi->read();

	LOG("%s SASI DATA R %02x\n", machine().describe_context(), data);

	if (m_sasi->req_r())
	{
		m_sasi->ack_w(1);
	}

	return data;
}

void mm1_state::sasi_data_w(offs_t offset, uint8_t data)
{
	m_sasi_data = data;

	if (!m_sasi->io_r())
	{
		m_sasi->write(data);
	}

	LOG("%s SASI DATA W %02x\n", machine().describe_context(), data);

	if (m_sasi->req_r())
	{
		m_sasi->ack_w(1);
	}
}

uint8_t mm1_state::sasi_ior3_r(offs_t offset)
{
	uint8_t data = 0;

	if (m_switch)
	{
		data = sasi_data_r(0);
	}
	else
	{
		data = m_fdc->dma_r();
	}

	return data;
}

void mm1_state::sasi_iow3_w(offs_t offset, uint8_t data)
{
	if (m_switch)
	{
		sasi_data_w(0, data);
	}
	else
	{
		m_fdc->dma_w(data);
	}
}

void mm1_state::sasi_bsy_w(int state)
{
	if (state)
	{
		m_sasi->sel_w(0);
	}
}

void mm1_state::sasi_req_w(int state)
{
	if (!state)
	{
		m_sasi->ack_w(0);
	}

	m_dmac->dreq3_w(state);
}

void mm1_state::sasi_io_w(int state)
{
	if (state)
	{
		m_sasi->write(0);
	}
	else
	{
		m_sasi->write(m_sasi_data);
	}
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void mm1_state::mm1_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(mm1_state::read), FUNC(mm1_state::write));
}

void mm1_state::mmu_io_map(address_map &map)
{
	map(0x00, 0x0f).rw(m_dmac, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x10, 0x13).mirror(0x0c).rw(m_mpsc, FUNC(upd7201_device::cd_ba_r), FUNC(upd7201_device::cd_ba_w));
	map(0x20, 0x21).mirror(0x0e).rw(m_crtc, FUNC(i8275_device::read), FUNC(i8275_device::write));
	map(0x30, 0x33).mirror(0x0c).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x40, 0x40).mirror(0x0f).rw(m_iop, FUNC(i8212_device::read), FUNC(i8212_device::write));
	map(0x50, 0x51).mirror(0x0e).view(m_fdc_view);
	m_fdc_view[0](0x50, 0x51).m(m_fdc, FUNC(upd765a_device::map));
	m_fdc_view[1](0x50, 0x50).rw(FUNC(mm1_state::sasi_status_r), FUNC(mm1_state::sasi_cmd_w));
	m_fdc_view[1](0x51, 0x51).rw(FUNC(mm1_state::sasi_data_r), FUNC(mm1_state::sasi_data_w));
	map(0x60, 0x67).mirror(0x08).w(m_outlatch, FUNC(ls259_device::write_d0));
}

void mm1_state::mm1g_mmu_io_map(address_map &map)
{
	mmu_io_map(map);
	map(0x70, 0x71).mirror(0x0e).rw(m_hgdc, FUNC(upd7220_device::read), FUNC(upd7220_device::write));
}



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

void mm1_state::dma_hrq_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	// Assert HLDA
	m_dmac->hack_w(state);
}


void mm1_state::dma_eop_w(int state)
{
	m_maincpu->set_input_line(I8085_RST75_LINE, state);

	m_tc = state;
	update_tc();
}


//-------------------------------------------------
//  upd765_interface fdc_intf
//-------------------------------------------------

void mm1_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_MM1_FORMAT);
}

static void mm1m4_floppies(device_slot_interface &device)
{
	device.option_add("525", FLOPPY_525_QD);
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( mm1 )
//-------------------------------------------------

void mm1_state::machine_start()
{
	// state saving
	save_item(NAME(m_a8));
	save_item(NAME(m_leen));
	save_item(NAME(m_intc));
	save_item(NAME(m_rx21));
	save_item(NAME(m_tx21));
	save_item(NAME(m_rcl));
	save_item(NAME(m_recall));
	save_item(NAME(m_dack3));
	save_item(NAME(m_tc));
	save_item(NAME(m_fdc_tc));
	save_item(NAME(m_switch));
	save_item(NAME(m_sasi_data));
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

void mm1_state::common(machine_config &config)
{
	// basic system hardware
	I8085A(config, m_maincpu, XTAL(6'144'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &mm1_state::mm1_map);
	m_maincpu->in_inta_func().set(m_mpsc, FUNC(upd7201_device::inta_r));
	m_maincpu->in_sid_func().set(m_rs232a, FUNC(rs232_port_device::dsr_r));
	m_maincpu->out_sod_func().set(KB_TAG, FUNC(mm1_keyboard_device::bell_w)).invert();

	config.set_perfect_quantum(m_maincpu);

	// peripheral hardware
	ADDRESS_MAP_BANK(config, m_io);
	m_io->set_addrmap(0, &mm1_state::mmu_io_map);
	m_io->set_data_width(8);
	m_io->set_addr_width(7);

	I8212(config, m_iop);
	m_iop->int_wr_callback().set_inputline(m_maincpu, I8085_RST65_LINE);
	m_iop->di_rd_callback().set(KB_TAG, FUNC(mm1_keyboard_device::read));

	LS259(config, m_outlatch);
	m_outlatch->q_out_cb<0>().set(FUNC(mm1_state::a8_w));
	m_outlatch->q_out_cb<1>().set(FUNC(mm1_state::recall_w));
	m_outlatch->q_out_cb<2>().set(FUNC(mm1_state::rx21_w));
	m_outlatch->q_out_cb<3>().set(FUNC(mm1_state::tx21_w));
	m_outlatch->q_out_cb<4>().set(FUNC(mm1_state::rcl_w));
	m_outlatch->q_out_cb<5>().set(FUNC(mm1_state::intc_w));
	m_outlatch->q_out_cb<6>().set(FUNC(mm1_state::leen_w));
	m_outlatch->q_out_cb<7>().set(FUNC(mm1_state::motor_on_w));

	AM9517A(config, m_dmac, XTAL(6'144'000)/2);
	m_dmac->out_hreq_callback().set(FUNC(mm1_state::dma_hrq_w));
	m_dmac->out_eop_callback().set(FUNC(mm1_state::dma_eop_w));
	m_dmac->in_memr_callback().set(FUNC(mm1_state::read));
	m_dmac->out_memw_callback().set(FUNC(mm1_state::write));
	m_dmac->in_ior_callback<2>().set(m_mpsc, FUNC(upd7201_device::da_r));
	m_dmac->in_ior_callback<3>().set(m_fdc, FUNC(upd765_family_device::dma_r));
	m_dmac->out_iow_callback<0>().set(m_crtc, FUNC(i8275_device::dack_w));
	m_dmac->out_iow_callback<1>().set(m_mpsc, FUNC(upd7201_device::da_w));
	m_dmac->out_iow_callback<3>().set(m_fdc, FUNC(upd765_family_device::dma_w));
	m_dmac->out_dack_callback<1>().set(FUNC(mm1_state::dack1_w));
	m_dmac->out_dack_callback<2>().set(FUNC(mm1_state::dack2_w));
	m_dmac->out_dack_callback<3>().set(FUNC(mm1_state::dack3_w));

	PIT8253(config, m_pit);
	m_pit->set_clk<0>(XTAL(6'144'000)/2/2);
	m_pit->out_handler<0>().set(FUNC(mm1_state::itxc_w));
	m_pit->set_clk<1>(XTAL(6'144'000)/2/2);
	m_pit->out_handler<1>().set(FUNC(mm1_state::irxc_w));
	m_pit->set_clk<2>(XTAL(6'144'000)/2/2);
	m_pit->out_handler<2>().set(FUNC(mm1_state::auxc_w));

	UPD765A(config, m_fdc, 16_MHz_XTAL/2, true, true);
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, I8085_RST55_LINE);
	m_fdc->drq_wr_callback().set(m_dmac, FUNC(am9517a_device::dreq3_w));

	UPD7201(config, m_mpsc, XTAL(6'144'000)/2);
	m_mpsc->out_int_callback().set_inputline(m_maincpu, I8085_INTR_LINE);
	m_mpsc->out_rxdrqa_callback().set(FUNC(mm1_state::drq2_w));
	m_mpsc->out_txdrqa_callback().set(FUNC(mm1_state::drq1_w));
	m_mpsc->out_txda_callback().set(m_rs232a, FUNC(rs232_port_device::write_txd));
	m_mpsc->out_rtsa_callback().set(m_rs232a, FUNC(rs232_port_device::write_rts));
	m_mpsc->out_txdb_callback().set(m_rs232b, FUNC(rs232_port_device::write_txd));
	m_mpsc->out_rtsb_callback().set(m_rs232b, FUNC(rs232_port_device::write_rts));

	RS232_PORT(config, m_rs232a, default_rs232_devices, nullptr);
	m_rs232a->rxd_handler().set(m_mpsc, FUNC(upd7201_device::rxa_w));
	m_rs232a->dcd_handler().set(m_mpsc, FUNC(upd7201_device::dcda_w));
	m_rs232a->cts_handler().set(m_mpsc, FUNC(upd7201_device::ctsa_w));
	m_rs232a->rxc_handler().set(FUNC(mm1_state::erxc_w));
	m_rs232a->txc_handler().set(FUNC(mm1_state::etxc_w));

	RS232_PORT(config, m_rs232b, default_rs232_devices, nullptr);
	m_rs232b->rxd_handler().set(m_mpsc, FUNC(upd7201_device::rxb_w));
	m_rs232b->dcd_handler().set(m_mpsc, FUNC(upd7201_device::dcdb_w));
	m_rs232b->cts_handler().set(m_mpsc, FUNC(upd7201_device::ctsb_w));

	mm1_keyboard_device &kb(MM1_KEYBOARD(config, KB_TAG, 2500)); // actual KBCLK is XTAL(6'144'000)/2/16
	kb.kbst_wr_callback().set(m_iop, FUNC(i8212_device::stb_w));

	// internal ram
	RAM(config, RAM_TAG).set_default_size("64K");

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("mm1_flop");
}

void mm1_state::mm1(machine_config &config)
{
	common(config);
	mm1_video(config);
}

void mm1_state::mm1g(machine_config &config)
{
	common(config);
	mm1g_video(config);

	m_io->set_addrmap(0, &mm1_state::mm1g_mmu_io_map);
}

void mm1_state::mm1_320k_dual(machine_config &config)
{
	FLOPPY_CONNECTOR(config, UPD765_TAG ":0", mm1m4_floppies, "525", mm1_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, UPD765_TAG ":1", mm1m4_floppies, "525", mm1_state::floppy_formats).enable_sound(true);
}

void mm1_state::mm1m4(machine_config &config)
{
	mm1(config);
	mm1_320k_dual(config);
}

void mm1_state::mm1m4g(machine_config &config)
{
	mm1g(config);
	mm1_320k_dual(config);
}

void mm1_state::mm1_640k_dual(machine_config &config)
{
	FLOPPY_CONNECTOR(config, UPD765_TAG ":0", mm1m4_floppies, "525", mm1_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, UPD765_TAG ":1", mm1m4_floppies, "525", mm1_state::floppy_formats).enable_sound(true);
}

void mm1_state::mm1m6(machine_config &config)
{
	mm1(config);
	mm1_640k_dual(config);
}

void mm1_state::mm1m6g(machine_config &config)
{
	mm1g(config);
	mm1_640k_dual(config);
}

void mm1_state::mm1_640k_winchester(machine_config &config)
{
	FLOPPY_CONNECTOR(config, UPD765_TAG ":0", mm1m4_floppies, "525", mm1_state::floppy_formats).enable_sound(true);

	NSCSI_BUS(config, "sasi");
	NSCSI_CONNECTOR(config, "sasi:0", default_scsi_devices, "s1410");
	NSCSI_CONNECTOR(config, "sasi:7", default_scsi_devices, "scsicb", true)
		.option_add_internal("scsicb", NSCSI_CB)
		.machine_config([this](device_t* device) {
			downcast<nscsi_callback_device&>(*device).bsy_callback().set(*this, FUNC(mm1_state::sasi_bsy_w));
			downcast<nscsi_callback_device&>(*device).req_callback().set(*this, FUNC(mm1_state::sasi_req_w));
			downcast<nscsi_callback_device&>(*device).io_callback().set(*this, FUNC(mm1_state::sasi_io_w));
		});

	m_outlatch->q_out_cb<7>().set(FUNC(mm1_state::switch_w));

	m_dmac->in_ior_callback<3>().set(*this, FUNC(mm1_state::sasi_ior3_r));
	m_dmac->out_iow_callback<3>().set(*this, FUNC(mm1_state::sasi_iow3_w));
}

void mm1_state::mm1m7(machine_config &config)
{
	mm1(config);
	mm1_640k_winchester(config);
}

void mm1_state::mm1m7g(machine_config &config)
{
	mm1g(config);
	mm1_640k_winchester(config);
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( mm1m4 )
//-------------------------------------------------

ROM_START( mm1m4 )
	ROM_REGION( 0x4000, I8085A_TAG, 0 ) // BIOS
	ROM_LOAD( "9081b.ic43", 0x0000, 0x2000, CRC(60841940) SHA1(d755e9be53f27f41d1e93b4c1793f9ea6a3e1229) )

	ROM_REGION( 0x200, "address", 0 ) // address decoder
	ROM_LOAD( "720793a.ic24", 0x000, 0x200, CRC(deea87a6) SHA1(8f19e43252c9a0b1befd02fc9d34fe1437477f3a) )

	ROM_REGION( 0x1000, "chargen", 0 ) // character generator
	ROM_LOAD( "6807b.ic61", 0x0000, 0x1000, CRC(32b36220) SHA1(8fe7a181badea3f7e656dfaea21ee9e4c9baf0f1) )
ROM_END

#define rom_mm1m4g rom_mm1m4


//-------------------------------------------------
//  ROM( mm1m6 )
//-------------------------------------------------

ROM_START( mm1m6 )
	ROM_REGION( 0x4000, I8085A_TAG, 0 ) // BIOS
	ROM_LOAD( "9081b.ic2", 0x0000, 0x2000, CRC(2955feb3) SHA1(946a6b0b8fb898be3f480c04da33d7aaa781152b) )

	ROM_REGION( 0x200, "address", 0 ) // address decoder
	ROM_LOAD( "720793a.ic24", 0x000, 0x200, CRC(deea87a6) SHA1(8f19e43252c9a0b1befd02fc9d34fe1437477f3a) )

	ROM_REGION( 0x1000, "chargen", 0 ) // character generator
	ROM_LOAD( "6807b.ic61", 0x0000, 0x1000, CRC(32b36220) SHA1(8fe7a181badea3f7e656dfaea21ee9e4c9baf0f1) )
ROM_END

#define rom_mm1m6g rom_mm1m6


//-------------------------------------------------
//  ROM( mm1m7 )
//-------------------------------------------------

ROM_START( mm1m7 )
	ROM_REGION( 0x4000, I8085A_TAG, 0 ) // BIOS
	ROM_LOAD( "9057c.ic2", 0x0000, 0x2000, CRC(89bbc042) SHA1(7e8800c94934b81ce08b7af862e1159e0517684d) )

	ROM_REGION( 0x200, "address", 0 ) // address decoder
	ROM_LOAD( "726972b.ic24", 0x000, 0x200, CRC(2487d4ca) SHA1(e883a2e9540c31abba3d7f3bc23a48941f655ea0) )

	ROM_REGION( 0x1000, "chargen", 0 ) // character generator
	ROM_LOAD( "6807b.ic61", 0x0000, 0x1000, CRC(32b36220) SHA1(8fe7a181badea3f7e656dfaea21ee9e4c9baf0f1) )
ROM_END

#define rom_mm1m7g rom_mm1m7



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY       FULLNAME            FLAGS
COMP( 1981, mm1m4,  0,     0,      mm1m4,   mm1,   mm1_state, empty_init, "Nokia Data", "MikroMikko 1 M4",  MACHINE_SUPPORTS_SAVE )
COMP( 1981, mm1m4g, mm1m4, 0,      mm1m4g,  mm1,   mm1_state, empty_init, "Nokia Data", "MikroMikko 1 M4G", MACHINE_SUPPORTS_SAVE )
COMP( 1981, mm1m6,  0,     0,      mm1m6,   mm1,   mm1_state, empty_init, "Nokia Data", "MikroMikko 1 M6",  MACHINE_SUPPORTS_SAVE )
COMP( 1981, mm1m6g, mm1m6, 0,      mm1m6g,  mm1,   mm1_state, empty_init, "Nokia Data", "MikroMikko 1 M6G", MACHINE_SUPPORTS_SAVE )
COMP( 1981, mm1m7,  0,     0,      mm1m7,   mm1,   mm1_state, empty_init, "Nokia Data", "MikroMikko 1 M7",  MACHINE_SUPPORTS_SAVE )
COMP( 1981, mm1m7g, mm1m7, 0,      mm1m7g,  mm1,   mm1_state, empty_init, "Nokia Data", "MikroMikko 1 M7G", MACHINE_SUPPORTS_SAVE )
