// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1570/1571/1571CR Single Disk Drive emulation

**********************************************************************/

/*

    TODO:

    - WD1770 set_floppy
    - 1571CR
        - MOS5710
    - ICT Mini Chief MC-20
        - WD1002A-WX1 ISA controller card
        - Seagate ST225 (-chs 615,4,17 -ss 512)

*/

#include "emu.h"
#include "c1571.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "u1"
#define M6522_0_TAG     "u9"
#define M6522_1_TAG     "u4"
#define M6526_TAG       "u20"
#define M5710_TAG       "u107"
#define WD1770_TAG      "u11"
#define C64H156_TAG     "u6"
#define C64H157_TAG     "u5"
#define ISA_BUS_TAG     "isabus"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C1570,      c1570_device,      "c1570",    "Commodore 1570 Disk Drive")
DEFINE_DEVICE_TYPE(C1571,      c1571_device,      "c1571",    "Commodore 1571 Disk Drive")
DEFINE_DEVICE_TYPE(C1571CR,    c1571cr_device,    "c1571cr",  "Commodore 1571CR Disk Drive")
DEFINE_DEVICE_TYPE(MINI_CHIEF, mini_chief_device, "minichif", "ICT Mini Chief Disk Drive")


//-------------------------------------------------
//  ROM( c1570 )
//-------------------------------------------------

ROM_START( c1570 )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_LOAD( "315090-01.u2", 0x0000, 0x8000, CRC(5a0c7937) SHA1(5fc06dc82ff6840f183bd43a4d9b8a16956b2f56) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1570_device::device_rom_region() const
{
	return ROM_NAME( c1570 );
}


//-------------------------------------------------
//  ROM( c1571 )
//-------------------------------------------------

ROM_START( c1571 )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS("r5")
	ROM_SYSTEM_BIOS( 0, "r3", "Revision 3" )
	ROMX_LOAD( "310654-03.u2", 0x0000, 0x8000, CRC(3889b8b8) SHA1(e649ef4419d65829d2afd65e07d31f3ce147d6eb), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r5", "Revision 5" )
	ROMX_LOAD( "310654-05.u2", 0x0000, 0x8000, CRC(5755bae3) SHA1(f1be619c106641a685f6609e4d43d6fc9eac1e70), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos 1571.u2", 0x0000, 0x8000, CRC(fe6cac6d) SHA1(d4b79b60cf1eaa399d0932200eb7811e00455249), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1571_device::device_rom_region() const
{
	return ROM_NAME( c1571 );
}


//-------------------------------------------------
//  ROM( c1571cr )
//-------------------------------------------------

ROM_START( c1571cr )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS("cbm")
	ROM_SYSTEM_BIOS( 0, "cbm", "Commodore" )
	ROMX_LOAD( "318047-01.u102", 0x0000, 0x8000, CRC(f24efcc4) SHA1(14ee7a0fb7e1c59c51fbf781f944387037daa3ee), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos 1571d.u102", 0x0000, 0x8000, CRC(9cba146d) SHA1(823b178561302b403e6bfd8dd741d757efef3958), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1571cr_device::device_rom_region() const
{
	return ROM_NAME( c1571cr );
}


//-------------------------------------------------
//  ROM( minichief )
//-------------------------------------------------

ROM_START( minichief )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_LOAD( "ictdos710.u2", 0x0000, 0x8000, CRC(aaacf7e9) SHA1(c1296995238ef23f18e7fec70a144a0566a25a27) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mini_chief_device::device_rom_region() const
{
	return ROM_NAME( minichief );
}


//-------------------------------------------------
//  ADDRESS_MAP( c1571_mem )
//-------------------------------------------------

void c1571_device::c1571_mem(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x1800, 0x180f).mirror(0x03f0).m(M6522_0_TAG, FUNC(via6522_device::map));
	map(0x1c00, 0x1c0f).mirror(0x03f0).rw(FUNC(c1571_device::via1_r), FUNC(c1571_device::via1_w));
	map(0x2000, 0x2003).mirror(0x1ffc).rw(WD1770_TAG, FUNC(wd1770_device::read), FUNC(wd1770_device::write));
	map(0x4000, 0x400f).mirror(0x3ff0).rw(M6526_TAG, FUNC(mos6526_device::read), FUNC(mos6526_device::write));
	map(0x8000, 0xffff).rom().region(M6502_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( mini_chief_mem )
//-------------------------------------------------

void mini_chief_device::mini_chief_mem(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x1800, 0x180f).mirror(0x03f0).m(M6522_0_TAG, FUNC(via6522_device::map));
	map(0x1c00, 0x1c0f).mirror(0x03f0).rw(FUNC(mini_chief_device::via1_r), FUNC(mini_chief_device::via1_w));
	map(0x2000, 0x2003).mirror(0x1ffc).rw(WD1770_TAG, FUNC(wd1770_device::read), FUNC(wd1770_device::write));
	map(0x4000, 0x400f).mirror(0xff0).rw(M6526_TAG, FUNC(mos6526_device::read), FUNC(mos6526_device::write));
	map(0x5000, 0x5fff).mirror(0x2000).ram();
	map(0x6000, 0x6fff).ram();
	map(0x8000, 0xffff).rom().region(M6502_TAG, 0);
}


WRITE_LINE_MEMBER( c1571_device::via0_irq_w )
{
	m_via0_irq = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, (m_via0_irq || m_via1_irq || m_cia_irq) ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t c1571_device::via0_pa_r()
{
	/*

	    bit     description

	    PA0     TRK0 SNS
	    PA1
	    PA2
	    PA3
	    PA4
	    PA5
	    PA6
	    PA7     BYTE RDY

	*/

	uint8_t data = 0;

	// track 0 sense
	data |= (m_floppy->trk00_r() ? 0x01 : 0x00);

	// byte ready
	data |= m_ga->byte_r() << 7;

	return data;
}

void c1571_device::via0_pa_w(uint8_t data)
{
	/*

	    bit     description

	    PA0
	    PA1     SER DIR
	    PA2     SIDE
	    PA3
	    PA4
	    PA5     _1/2 MHZ
	    PA6     ATN OUT
	    PA7

	*/

	// fast serial direction
	m_ser_dir = BIT(data, 1);

	// side select
	m_floppy->ss_w(BIT(data, 2));

	// 1/2 MHz
	int clock_1_2 = BIT(data, 5);

	if (m_1_2mhz != clock_1_2)
	{
		const XTAL clock = 16_MHz_XTAL / (clock_1_2 ? 8 : 16);

		m_maincpu->set_unscaled_clock(clock);
		m_cia->set_unscaled_clock(clock);
		m_via0->set_unscaled_clock(clock);
		m_via1->set_unscaled_clock(clock);
		m_ga->accl_w(clock_1_2);

		m_1_2mhz = clock_1_2;
	}

	// attention out
	m_bus->atn_w(this, !BIT(data, 6));

	update_iec();
}

void c1571cr_device::via0_pa_w(uint8_t data)
{
	/*

	    bit     description

	    PA0
	    PA1
	    PA2     SIDE
	    PA3
	    PA4
	    PA5     _1/2 MHZ
	    PA6
	    PA7

	*/

	// side select
	m_floppy->ss_w(BIT(data, 2));

	// 1/2 MHz
	int clock_1_2 = BIT(data, 5);

	if (m_1_2mhz != clock_1_2)
	{
		const XTAL clock = 16_MHz_XTAL / (clock_1_2 ? 8 : 16);

		m_maincpu->set_unscaled_clock(clock);
		m_cia->set_unscaled_clock(clock);
		m_via0->set_unscaled_clock(clock);
		m_via1->set_unscaled_clock(clock);
		m_ga->accl_w(clock_1_2);

		m_1_2mhz = clock_1_2;
	}
}

uint8_t c1571_device::via0_pb_r()
{
	/*

	    bit     description

	    PB0     DATA IN
	    PB1
	    PB2     CLK IN
	    PB3
	    PB4
	    PB5     DEV# SEL
	    PB6     DEV# SEL
	    PB7     ATN IN

	*/

	uint8_t data;

	// data in
	data = !m_bus->data_r();

	// clock in
	data |= !m_bus->clk_r() << 2;

	// serial bus address
	data |= ((m_slot->get_address() - 8) & 0x03) << 5;

	// attention in
	data |= !m_bus->atn_r() << 7;

	return data;
}

void c1571_device::via0_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0
	    PB1     DATA OUT
	    PB2
	    PB3     CLK OUT
	    PB4     ATN ACK
	    PB5
	    PB6
	    PB7

	*/

	// data out
	m_data_out = BIT(data, 1);

	// clock out
	m_bus->clk_w(this, !BIT(data, 3));

	// attention acknowledge
	m_ga->atna_w(BIT(data, 4));

	update_iec();
}

void c1571cr_device::via0_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0
	    PB1     DATA OUT
	    PB2
	    PB3     CLK OUT
	    PB4     ATNI
	    PB5
	    PB6
	    PB7

	*/

	// data out
	m_data_out = BIT(data, 1);

	// clock out
	m_bus->clk_w(this, !BIT(data, 3));

	// attention in
	m_ga->atni_w(BIT(data, 4));

	update_iec();
}


uint8_t c1571_device::via1_r(offs_t offset)
{
	uint8_t data = m_via1->read(offset);

	m_ga->ted_w(!m_1_2mhz);
	m_ga->ted_w(1);

	return data;
}

void c1571_device::via1_w(offs_t offset, uint8_t data)
{
	m_via1->write(offset, data);

	m_ga->ted_w(!m_1_2mhz);
	m_ga->ted_w(1);
}

WRITE_LINE_MEMBER( c1571_device::via1_irq_w )
{
	m_via1_irq = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, (m_via0_irq || m_via1_irq || m_cia_irq) ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t c1571_device::via1_pb_r()
{
	/*

	    bit     signal      description

	    PB0
	    PB1
	    PB2
	    PB3
	    PB4     _WPRT       write protect sense
	    PB5
	    PB6
	    PB7     _SYNC       SYNC detect line

	*/

	uint8_t data = 0;

	// write protect sense
	data |= !m_floppy->wpt_r() << 4;

	// SYNC detect line
	data |= m_ga->sync_r() << 7;

	return data;
}

void c1571_device::via1_pb_w(uint8_t data)
{
	/*

	    bit     signal      description

	    PB0     STP0        stepping motor bit 0
	    PB1     STP1        stepping motor bit 1
	    PB2     MTR         motor ON/OFF
	    PB3     ACT         drive 0 LED
	    PB4
	    PB5     DS0         density select 0
	    PB6     DS1         density select 1
	    PB7

	*/

	// spindle motor
	m_ga->mtr_w(BIT(data, 2));

	// stepper motor
	m_ga->stp_w(data & 0x03); // TODO actually STP1=0, STP0=!(PB0^PB1), Y0=PB1, Y2=!PB1

	// activity LED
	m_leds[LED_ACT] = BIT(data, 3);

	// density select
	m_ga->ds_w((data >> 5) & 0x03);
}


//-------------------------------------------------
//  MOS6526_INTERFACE( cia_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( c1571_device::cia_irq_w )
{
	m_cia_irq = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, (m_via0_irq || m_via1_irq || m_cia_irq) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER( c1571_device::cia_pc_w )
{
	if (m_other != nullptr)
	{
		m_other->parallel_strobe_w(state);
	}
}

WRITE_LINE_MEMBER( c1571_device::cia_cnt_w )
{
	m_cnt_out = state;

	update_iec();
}

WRITE_LINE_MEMBER( c1571_device::cia_sp_w )
{
	m_sp_out = state;

	update_iec();
}

uint8_t c1571_device::cia_pb_r()
{
	return m_parallel_data;
}

void c1571_device::cia_pb_w(uint8_t data)
{
	if (m_other != nullptr)
	{
		m_other->parallel_data_w(data);
	}
}


//-------------------------------------------------
//  MOS6526_INTERFACE( mini_chief_cia_intf )
//-------------------------------------------------

uint8_t mini_chief_device::cia_pa_r()
{
	// TODO read from ISA bus @ 0x320 | A2 A1 A0

	return 0;
}

void mini_chief_device::cia_pa_w(uint8_t data)
{
	// TODO write to ISA bus @ 0x320 | A2 A1 A0
}

void mini_chief_device::cia_pb_w(uint8_t data)
{
	/*

	    bit     description

	    0       ISA A0
	    1       ISA A1
	    2       ISA A2
	    3       ISA /SMEMR
	    4       ISA /SMEMW
	    5       ISA RESET
	    6
	    7

	*/
}


//-------------------------------------------------
//  C64H156_INTERFACE( ga_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( c1571_device::byte_w )
{
	m_via1->write_ca1(state);

	m_maincpu->set_input_line(M6502_SET_OVERFLOW, state);
}


//-------------------------------------------------
//  floppy_interface c1571_floppy_interface
//-------------------------------------------------

void c1571_device::wpt_callback(floppy_image_device *floppy, int state)
{
	m_via0->write_ca2(!state);
}


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

void c1571_device::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_D64_FORMAT);
	fr.add(FLOPPY_G64_FORMAT);
	fr.add(FLOPPY_D71_FORMAT);
}


//-------------------------------------------------
//  isa8bus_interface isabus_intf
//-------------------------------------------------

static void mini_chief_isa8_cards(device_slot_interface &device)
{
	device.option_add("wd1002a_wx1", ISA8_WD1002A_WX1);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c1571_device::add_base_mconfig(machine_config &config)
{
	M6502(config, m_maincpu, 16_MHz_XTAL / 16);
	m_maincpu->set_addrmap(AS_PROGRAM, &c1571_device::c1571_mem);
	//config.set_perfect_quantum(m_maincpu); FIXME: not safe in a slot device - add barriers

	MOS6522(config, m_via0, 16_MHz_XTAL / 16);
	m_via0->readpa_handler().set(FUNC(c1571_device::via0_pa_r));
	m_via0->readpb_handler().set(FUNC(c1571_device::via0_pb_r));
	m_via0->writepa_handler().set(FUNC(c1571_device::via0_pa_w));
	m_via0->writepb_handler().set(FUNC(c1571_device::via0_pb_w));
	m_via0->irq_handler().set(FUNC(c1571_device::via0_irq_w));

	MOS6522(config, m_via1, 16_MHz_XTAL / 16);
	m_via1->readpa_handler().set(C64H156_TAG, FUNC(c64h156_device::yb_r));
	m_via1->readpb_handler().set(FUNC(c1571_device::via1_pb_r));
	m_via1->writepa_handler().set(C64H156_TAG, FUNC(c64h156_device::yb_w));
	m_via1->writepb_handler().set(FUNC(c1571_device::via1_pb_w));
	m_via1->ca2_handler().set(C64H156_TAG, FUNC(c64h156_device::soe_w));
	m_via1->cb2_handler().set(C64H156_TAG, FUNC(c64h156_device::oe_w));
	m_via1->irq_handler().set(FUNC(c1571_device::via1_irq_w));

	WD1770(config, m_fdc, 16_MHz_XTAL / 2);

	C64H156(config, m_ga, 16_MHz_XTAL);
	m_ga->byte_callback().set(FUNC(c1571_device::byte_w));

	floppy_connector &connector(FLOPPY_CONNECTOR(config, C64H156_TAG":0", 0));
	connector.option_add("525qd", FLOPPY_525_QD);
	connector.set_default_option("525qd");
	connector.set_fixed(true);
	connector.set_formats(c1571_device::floppy_formats);
	connector.enable_sound(true);
}

void c1571_device::add_cia_mconfig(machine_config &config)
{
	MOS6526(config, m_cia, 16_MHz_XTAL / 16);
	m_cia->irq_wr_callback().set(FUNC(c1571_device::cia_irq_w));
	m_cia->cnt_wr_callback().set(FUNC(c1571_device::cia_cnt_w));
	m_cia->sp_wr_callback().set(FUNC(c1571_device::cia_sp_w));
	m_cia->pb_rd_callback().set(FUNC(c1571_device::cia_pb_r));
	m_cia->pb_wr_callback().set(FUNC(c1571_device::cia_pb_w));
	m_cia->pc_wr_callback().set(FUNC(c1571_device::cia_pc_w));
}

void c1570_device::device_add_mconfig(machine_config &config)
{
	add_base_mconfig(config);
	add_cia_mconfig(config);
}


void c1571_device::device_add_mconfig(machine_config &config)
{
	add_base_mconfig(config);
	add_cia_mconfig(config);
}


void c1571cr_device::device_add_mconfig(machine_config &config)
{
	add_base_mconfig(config);

	m_via0->writepa_handler().set(FUNC(c1571cr_device::via0_pa_w));
	m_via0->writepb_handler().set(FUNC(c1571cr_device::via0_pb_w));

	//MOS5710(config, M5710_TAG, 16_MHz_XTAL / 16);
}


void mini_chief_device::device_add_mconfig(machine_config &config)
{
	add_base_mconfig(config);
	add_cia_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mini_chief_device::mini_chief_mem);

	isa8_device &isa8(ISA8(config, ISA_BUS_TAG, 0));
	isa8.set_memspace(m_maincpu, AS_PROGRAM);
	isa8.set_iospace(m_maincpu, AS_PROGRAM);
	ISA8_SLOT(config, "isa1", 0, ISA_BUS_TAG, mini_chief_isa8_cards, "wd1002a_wx1", false);
}


//-------------------------------------------------
//  INPUT_PORTS( c1571 )
//-------------------------------------------------

static INPUT_PORTS_START( c1571 )
	PORT_START("ADDRESS")
	PORT_DIPNAME( 0x03, 0x00, "Device Address" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x01, "9" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x03, "11" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c1571_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c1571 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1571_device - constructor
//-------------------------------------------------

c1571_device::c1571_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
		device_cbm_iec_interface(mconfig, *this),
		device_c64_floppy_parallel_interface(mconfig, *this),
		m_maincpu(*this, M6502_TAG),
		m_via0(*this, M6522_0_TAG),
		m_via1(*this, M6522_1_TAG),
		m_cia(*this, M6526_TAG),
		m_fdc(*this, WD1770_TAG),
		m_ga(*this, C64H156_TAG),
		m_floppy(*this, C64H156_TAG":0:525qd"),
		m_address(*this, "ADDRESS"),
		m_leds(*this, "led%u", 0U),
		m_1_2mhz(0),
		m_data_out(1),
		m_ser_dir(0),
		m_sp_out(1),
		m_cnt_out(1),
		m_via0_irq(CLEAR_LINE),
		m_via1_irq(CLEAR_LINE),
		m_cia_irq(CLEAR_LINE)
{
}

c1571_device::c1571_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c1571_device(mconfig, C1571, tag, owner, clock)
{
}


//-------------------------------------------------
//  c1570_device - constructor
//-------------------------------------------------

c1570_device::c1570_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c1571_device(mconfig, C1570, tag, owner, clock)
{
}


//-------------------------------------------------
//  c1571cr_device - constructor
//-------------------------------------------------

c1571cr_device::c1571cr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c1571_device(mconfig, C1571CR, tag, owner, clock)
{
}


//-------------------------------------------------
//  mini_chief_device - constructor
//-------------------------------------------------

mini_chief_device::mini_chief_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c1571_device(mconfig, MINI_CHIEF, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c1571_device::device_start()
{
	m_leds.resolve();

	// install image callbacks
	m_ga->set_floppy(m_floppy);
	//m_fdc->set_floppy(m_floppy);
	m_floppy->setup_wpt_cb(floppy_image_device::wpt_cb(&c1571_device::wpt_callback, this));

	// register for state saving
	save_item(NAME(m_1_2mhz));
	save_item(NAME(m_data_out));
	save_item(NAME(m_ser_dir));
	save_item(NAME(m_sp_out));
	save_item(NAME(m_cnt_out));
	save_item(NAME(m_via0_irq));
	save_item(NAME(m_via1_irq));
	save_item(NAME(m_cia_irq));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c1571_device::device_reset()
{
	m_maincpu->reset();

	m_via0->reset();
	m_via1->reset();
	m_cia->reset();
	m_fdc->reset();

	m_fdc->dden_w(0);

	m_sp_out = 1;
	m_cnt_out = 1;

	update_iec();
}


//-------------------------------------------------
//  cbm_iec_srq -
//-------------------------------------------------

void c1571_device::cbm_iec_srq(int state)
{
	update_iec();
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void c1571_device::cbm_iec_atn(int state)
{
	update_iec();
}


//-------------------------------------------------
//  cbm_iec_data -
//-------------------------------------------------

void c1571_device::cbm_iec_data(int state)
{
	update_iec();
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void c1571_device::cbm_iec_reset(int state)
{
	if (!state)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  parallel_data_w -
//-------------------------------------------------

void c1571_device::parallel_data_w(uint8_t data)
{
	m_parallel_data = data;
}


//-------------------------------------------------
//  parallel_strobe_w -
//-------------------------------------------------

void c1571_device::parallel_strobe_w(int state)
{
	m_cia->flag_w(state);
}


//-------------------------------------------------
//  update_iec -
//-------------------------------------------------

void c1571_device::update_iec()
{
	m_cia->cnt_w(m_ser_dir || m_bus->srq_r());
	m_cia->sp_w(m_ser_dir || m_bus->data_r());

	int atn = m_bus->atn_r();
	m_via0->write_ca1(!atn);
	m_ga->atni_w(!atn);

	// serial data
	int data = !m_data_out && !m_ga->atn_r();
	if (m_ser_dir) data &= m_sp_out;
	m_bus->data_w(this, data);

	// fast clock
	int srq = 1;
	if (m_ser_dir) srq &= m_cnt_out;
	m_bus->srq_w(this, srq);
}
