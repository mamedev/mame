// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1551 Single Disk Drive emulation

**********************************************************************/

#include "emu.h"
#include "c1551.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6510T_TAG      "u2"
#define M6523_0_TAG     "u3"
#define M6523_1_TAG     "ci_u2"
#define C64H156_TAG     "u6"
#define PLA_TAG         "u1"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C1551, c1551_device, "c1551", "Commodore 1551")


//-------------------------------------------------
//  ROM( c1551 )
//-------------------------------------------------

ROM_START( c1551 ) // schematic 251860
	ROM_REGION( 0x4000, M6510T_TAG, 0 )
	ROM_LOAD( "318001-01.u4", 0x0000, 0x4000, CRC(6d16d024) SHA1(fae3c788ad9a6cc2dbdfbcf6c0264b2ca921d55e) )

	ROM_REGION( 0xf5, PLA_TAG, 0 ) // schematic 251925
	ROM_LOAD( "251641-03.u1", 0x00, 0xf5, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1551_device::device_rom_region() const
{
	return ROM_NAME( c1551 );
}


//-------------------------------------------------
//  M6510_INTERFACE( cpu_intf )
//-------------------------------------------------

uint8_t c1551_device::port_r()
{
	/*

	    bit     description

	    P0
	    P1
	    P2
	    P3
	    P4      WPS
	    P5
	    P6
	    P7      BYTE LTCHED

	*/

	uint8_t data = 0;

	// write protect sense
	data |= !m_floppy->wpt_r() << 4;

	// byte latched
	data |= m_ga->atn_r() << 7;

	return data;
}

void c1551_device::port_w(uint8_t data)
{
	/*

	    bit     description

	    P0      STP0A
	    P1      STP0B
	    P2      MTR0
	    P3      ACT0
	    P4
	    P5      DS0
	    P6      DS1
	    P7

	*/

	// stepper motor
	m_ga->stp_w(data & 0x03);

	// spindle motor
	m_ga->mtr_w(BIT(data, 2));

	// activity LED
	m_leds[LED_ACT] = BIT(data, 3);

	// density select
	m_ga->ds_w((data >> 5) & 0x03);
}


//-------------------------------------------------
//  tpi6525_interface tpi0_intf
//-------------------------------------------------

uint8_t c1551_device::tcbm_data_r()
{
	/*

	    bit     description

	    PA0     TCBM PA0
	    PA1     TCBM PA1
	    PA2     TCBM PA2
	    PA3     TCBM PA3
	    PA4     TCBM PA4
	    PA5     TCBM PA5
	    PA6     TCBM PA6
	    PA7     TCBM PA7

	*/

	return m_tcbm_data;
}

void c1551_device::tcbm_data_w(uint8_t data)
{
	/*

	    bit     description

	    PA0     TCBM PA0
	    PA1     TCBM PA1
	    PA2     TCBM PA2
	    PA3     TCBM PA3
	    PA4     TCBM PA4
	    PA5     TCBM PA5
	    PA6     TCBM PA6
	    PA7     TCBM PA7

	*/

	m_tcbm_data = data;
}

uint8_t c1551_device::tpi0_r(offs_t offset)
{
	uint8_t data = m_tpi0->read(offset);

	m_ga->ted_w(0);
	m_ga->ted_w(1);

	return data;
}

void c1551_device::tpi0_w(offs_t offset, uint8_t data)
{
	m_tpi0->write(offset, data);

	m_ga->ted_w(0);
	m_ga->ted_w(1);
}

uint8_t c1551_device::tpi0_pc_r()
{
	/*

	    bit     description

	    PC0
	    PC1
	    PC2
	    PC3
	    PC4
	    PC5     JP1
	    PC6     _SYNC
	    PC7     TCBM DAV

	*/

	uint8_t data = 0;

	// JP1
	data |= m_jp1->read() << 5;

	// SYNC detect line
	data |= m_ga->sync_r() << 6;

	// TCBM data valid
	data |= m_dav << 7;

	return data;
}

void c1551_device::tpi0_pc_w(uint8_t data)
{
	/*

	    bit     description

	    PC0     TCBM STATUS0
	    PC1     TCBM STATUS1
	    PC2     TCBM DEV
	    PC3     TCBM ACK
	    PC4     MODE
	    PC5
	    PC6
	    PC7

	*/

	// TCBM status
	m_status = data & 0x03;

	// TCBM device number
	m_dev = BIT(data, 2);

	// TCBM acknowledge
	m_ack = BIT(data, 3);

	// read/write mode
	m_ga->oe_w(BIT(data, 4));
}

//-------------------------------------------------
//  tpi6525_interface tpi1_intf
//-------------------------------------------------

uint8_t c1551_device::tpi1_pb_r()
{
	/*

	    bit     description

	    PB0     STATUS0
	    PB1     STATUS1
	    PB2
	    PB3
	    PB4
	    PB5
	    PB6
	    PB7

	*/

	return m_status & 0x03;
}

uint8_t c1551_device::tpi1_pc_r()
{
	/*

	    bit     description

	    PC0
	    PC1
	    PC2
	    PC3
	    PC4
	    PC5
	    PC6
	    PC7     TCBM ACK

	*/

	uint8_t data = 0;

	// TCBM acknowledge
	data |= m_ack << 7;

	return data;
}

void c1551_device::tpi1_pc_w(uint8_t data)
{
	/*

	    bit     description

	    PC0
	    PC1
	    PC2
	    PC3
	    PC4
	    PC5
	    PC6     TCBM DAV
	    PC7

	*/

	// TCBM data valid
	m_dav = BIT(data, 6);
}

//-------------------------------------------------
//  ADDRESS_MAP( c1551_mem )
//-------------------------------------------------

void c1551_device::c1551_mem(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x0800).ram();
	map(0x4000, 0x4007).mirror(0x3ff8).rw(FUNC(c1551_device::tpi0_r), FUNC(c1551_device::tpi0_w));
	map(0xc000, 0xffff).rom().region(M6510T_TAG, 0);
}


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( c1551_device::floppy_formats )
	FLOPPY_D64_FORMAT,
	FLOPPY_G64_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c1551_device::device_add_mconfig(machine_config &config)
{
	M6510T(config, m_maincpu, XTAL(16'000'000)/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &c1551_device::c1551_mem);
	m_maincpu->read_callback().set(FUNC(c1551_device::port_r));
	m_maincpu->write_callback().set(FUNC(c1551_device::port_w));
	config.m_perfect_cpu_quantum = subtag(M6510T_TAG);

	PLS100(config, m_pla);

	TPI6525(config, m_tpi0, 0);
	m_tpi0->in_pa_cb().set(FUNC(c1551_device::tcbm_data_r));
	m_tpi0->out_pa_cb().set(FUNC(c1551_device::tcbm_data_w));
	m_tpi0->in_pb_cb().set(C64H156_TAG, FUNC(c64h156_device::yb_r));
	m_tpi0->out_pb_cb().set(C64H156_TAG, FUNC(c64h156_device::yb_w));
	m_tpi0->in_pc_cb().set(FUNC(c1551_device::tpi0_pc_r));
	m_tpi0->out_pc_cb().set(FUNC(c1551_device::tpi0_pc_w));

	TPI6525(config, m_tpi1, 0);
	m_tpi1->in_pa_cb().set(FUNC(c1551_device::tcbm_data_r));
	m_tpi1->out_pa_cb().set(FUNC(c1551_device::tcbm_data_w));
	m_tpi1->in_pb_cb().set(FUNC(c1551_device::tpi1_pb_r));
	m_tpi1->in_pc_cb().set(FUNC(c1551_device::tpi1_pc_r));
	m_tpi1->out_pc_cb().set(FUNC(c1551_device::tpi1_pc_w));

	C64H156(config, m_ga, XTAL(16'000'000));
	m_ga->byte_callback().set(C64H156_TAG, FUNC(c64h156_device::atni_w));

	floppy_connector &connector(FLOPPY_CONNECTOR(config, C64H156_TAG":0", 0));
	connector.option_add("525ssqd", FLOPPY_525_SSQD);
	connector.set_default_option("525ssqd");
	connector.set_fixed(true);
	connector.set_formats(c1551_device::floppy_formats);

	PLUS4_EXPANSION_SLOT(config, m_exp, 0);
	m_exp->irq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(plus4_expansion_slot_device::irq_w));
	m_exp->cd_rd_callback().set(DEVICE_SELF_OWNER, FUNC(plus4_expansion_slot_device::dma_cd_r));
	m_exp->cd_wr_callback().set(DEVICE_SELF_OWNER, FUNC(plus4_expansion_slot_device::dma_cd_w));
	m_exp->aec_wr_callback().set(DEVICE_SELF_OWNER, FUNC(plus4_expansion_slot_device::aec_w));
	plus4_expansion_cards(*m_exp);
}


//-------------------------------------------------
//  INPUT_PORTS( c1551 )
//-------------------------------------------------

static INPUT_PORTS_START( c1551 )
	PORT_START("JP1")
	PORT_DIPNAME( 0x01, 0x00, "Device Number" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x01, "9" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c1551_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c1551 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1551_device - constructor
//-------------------------------------------------

c1551_device::c1551_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, C1551, tag, owner, clock)
	, device_plus4_expansion_card_interface(mconfig, *this)
	, m_maincpu(*this, M6510T_TAG)
	, m_tpi0(*this, M6523_0_TAG)
	, m_tpi1(*this, M6523_1_TAG)
	, m_ga(*this, C64H156_TAG)
	, m_pla(*this, PLA_TAG)
	, m_floppy(*this, C64H156_TAG":0:525ssqd")
	, m_exp(*this, PLUS4_EXPANSION_SLOT_TAG)
	, m_jp1(*this, "JP1")
	, m_leds(*this, "led%u", 0U)
	, m_tcbm_data(0xff)
	, m_status(1)
	, m_dav(1)
	, m_ack(1)
	, m_dev(0)
	, m_irq_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c1551_device::device_start()
{
	m_leds.resolve();

	// allocate timers
	m_irq_timer = timer_alloc();
	m_irq_timer->adjust(attotime::zero, CLEAR_LINE);

	// install image callbacks
	m_ga->set_floppy(m_floppy);

	// register for state saving
	save_item(NAME(m_tcbm_data));
	save_item(NAME(m_status));
	save_item(NAME(m_dav));
	save_item(NAME(m_ack));
	save_item(NAME(m_dev));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c1551_device::device_reset()
{
	m_maincpu->reset();

	m_tpi0->reset();

	m_exp->reset();

	// initialize gate array
	m_ga->test_w(1);
	m_ga->soe_w(1);
	m_ga->accl_w(1);
	m_ga->atna_w(1);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void c1551_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, param);

	if (param == ASSERT_LINE)
	{
		// Ts = 0.7*R2*C1 = 0.7*100R*0.1uF = 7us
		m_irq_timer->adjust(attotime::from_usec(7), CLEAR_LINE);
	}
	else
	{
		// Tm = 0.7*(R1+R2)*C1 = 0.7*(120K+100R)*0.1uF = 0.008407s
		m_irq_timer->adjust(attotime::from_usec(8407), ASSERT_LINE);
	}
}


//-------------------------------------------------
//  tpi1_selected -
//-------------------------------------------------

bool c1551_device::tpi1_selected(offs_t offset)
{
#ifdef PLA_DUMPED
	int mux = 0, ras = 0, phi0 = 0, f7 = 0;
	uint16_t input = A5 << 15 | A6 << 14 | A7 << 13 | A8 << 12 | A9 << 11 | mux << 10 | A10 << 9 | m_dev << 8 | ras << 7 | phi0 << 6 | A15 << 5 | A14 << 4 | A13 << 3 | A12 << 2 | A11 << 1 | f7;
	uint8_t data = m_pla->read(input);
	return BIT(data, 0) ? true : false;
#endif

	offs_t start_address = m_dev ? 0xfee0 : 0xfec0;

	if (offset >= start_address && offset < (start_address + 0x20))
	{
		return true;
	}

	return false;
}


//-------------------------------------------------
//  plus4_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c1551_device::plus4_cd_r(offs_t offset, uint8_t data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h)
{
	data = m_exp->cd_r(offset, data, ba, cs0, c1l, c2l, cs1, c1h, c2h);

	if (tpi1_selected(offset))
	{
		data = m_tpi1->read(offset & 0x07);
	}

	return data;
}


//-------------------------------------------------
//  plus4_cd_w - cartridge data write
//-------------------------------------------------

void c1551_device::plus4_cd_w(offs_t offset, uint8_t data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h)
{
	if (tpi1_selected(offset))
	{
		m_tpi1->write(offset & 0x07, data);
	}

	m_exp->cd_w(offset, data, ba, cs0, c1l, c2l, cs1, c1h, c2h);
}
