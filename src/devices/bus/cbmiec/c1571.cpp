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

const device_type C1570 = &device_creator<c1570_t>;
const device_type C1571 = &device_creator<c1571_t>;
const device_type C1571CR = &device_creator<c1571cr_t>;
const device_type MINI_CHIEF = &device_creator<mini_chief_t>;


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

const rom_entry *c1570_t::device_rom_region() const
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
	ROMX_LOAD( "310654-03.u2", 0x0000, 0x8000, CRC(3889b8b8) SHA1(e649ef4419d65829d2afd65e07d31f3ce147d6eb), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "r5", "Revision 5" )
	ROMX_LOAD( "310654-05.u2", 0x0000, 0x8000, CRC(5755bae3) SHA1(f1be619c106641a685f6609e4d43d6fc9eac1e70), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos 1571.u2", 0x0000, 0x8000, CRC(fe6cac6d) SHA1(d4b79b60cf1eaa399d0932200eb7811e00455249), ROM_BIOS(3) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c1571_t::device_rom_region() const
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
	ROMX_LOAD( "318047-01.u102", 0x0000, 0x8000, CRC(f24efcc4) SHA1(14ee7a0fb7e1c59c51fbf781f944387037daa3ee), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos 1571d.u102", 0x0000, 0x8000, CRC(9cba146d) SHA1(823b178561302b403e6bfd8dd741d757efef3958), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c1571cr_t::device_rom_region() const
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

const rom_entry *mini_chief_t::device_rom_region() const
{
	return ROM_NAME( minichief );
}


//-------------------------------------------------
//  ADDRESS_MAP( c1571_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c1571_mem, AS_PROGRAM, 8, c1571_t )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x1800, 0x180f) AM_MIRROR(0x03f0) AM_DEVREADWRITE(M6522_0_TAG, via6522_device, read, write)
	AM_RANGE(0x1c00, 0x1c0f) AM_MIRROR(0x03f0) AM_READWRITE(via1_r, via1_w)
	AM_RANGE(0x2000, 0x2003) AM_MIRROR(0x1ffc) AM_DEVREADWRITE(WD1770_TAG, wd1770_t, read, write)
	AM_RANGE(0x4000, 0x400f) AM_MIRROR(0x3ff0) AM_DEVREADWRITE(M6526_TAG, mos6526_device, read, write)
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION(M6502_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( mini_chief_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( mini_chief_mem, AS_PROGRAM, 8, mini_chief_t )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x1800, 0x180f) AM_MIRROR(0x03f0) AM_DEVREADWRITE(M6522_0_TAG, via6522_device, read, write)
	AM_RANGE(0x1c00, 0x1c0f) AM_MIRROR(0x03f0) AM_READWRITE(via1_r, via1_w)
	AM_RANGE(0x2000, 0x2003) AM_MIRROR(0x1ffc) AM_DEVREADWRITE(WD1770_TAG, wd1770_t, read, write)
	AM_RANGE(0x4000, 0x400f) AM_MIRROR(0xff0) AM_DEVREADWRITE(M6526_TAG, mos6526_device, read, write)
	AM_RANGE(0x5000, 0x5fff) AM_MIRROR(0x2000) AM_RAM
	AM_RANGE(0x6000, 0x6fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION(M6502_TAG, 0)
ADDRESS_MAP_END


WRITE_LINE_MEMBER( c1571_t::via0_irq_w )
{
	m_via0_irq = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, (m_via0_irq || m_via1_irq || m_cia_irq) ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER( c1571_t::via0_pa_r )
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

	UINT8 data = 0;

	// track 0 sense
	data |= (m_floppy->trk00_r() ? 0x01 : 0x00);

	// byte ready
	data |= m_ga->byte_r() << 7;

	return data;
}

WRITE8_MEMBER( c1571_t::via0_pa_w )
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
		UINT32 clock = clock_1_2 ? XTAL_16MHz/8 : XTAL_16MHz/16;

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

WRITE8_MEMBER( c1571cr_t::via0_pa_w )
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
		UINT32 clock = clock_1_2 ? XTAL_16MHz/8 : XTAL_16MHz/16;

		m_maincpu->set_unscaled_clock(clock);
		m_cia->set_unscaled_clock(clock);
		m_via0->set_unscaled_clock(clock);
		m_via1->set_unscaled_clock(clock);
		m_ga->accl_w(clock_1_2);

		m_1_2mhz = clock_1_2;
	}
}

READ8_MEMBER( c1571_t::via0_pb_r )
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

	UINT8 data = 0;

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

WRITE8_MEMBER( c1571_t::via0_pb_w )
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

WRITE8_MEMBER( c1571cr_t::via0_pb_w )
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


READ8_MEMBER( c1571_t::via1_r )
{
	UINT8 data = m_via1->read(space, offset);

	m_ga->ted_w(!m_1_2mhz);
	m_ga->ted_w(1);

	return data;
}

WRITE8_MEMBER( c1571_t::via1_w )
{
	m_via1->write(space, offset, data);

	m_ga->ted_w(!m_1_2mhz);
	m_ga->ted_w(1);
}

WRITE_LINE_MEMBER( c1571_t::via1_irq_w )
{
	m_via1_irq = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, (m_via0_irq || m_via1_irq || m_cia_irq) ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER( c1571_t::via1_pb_r )
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

	UINT8 data = 0;

	// write protect sense
	data |= !m_floppy->wpt_r() << 4;

	// SYNC detect line
	data |= m_ga->sync_r() << 7;

	return data;
}

WRITE8_MEMBER( c1571_t::via1_pb_w )
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
	output_set_led_value(LED_ACT, BIT(data, 3));

	// density select
	m_ga->ds_w((data >> 5) & 0x03);
}


//-------------------------------------------------
//  MOS6526_INTERFACE( cia_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( c1571_t::cia_irq_w )
{
	m_cia_irq = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, (m_via0_irq || m_via1_irq || m_cia_irq) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER( c1571_t::cia_pc_w )
{
	if (m_other != nullptr)
	{
		m_other->parallel_strobe_w(state);
	}
}

WRITE_LINE_MEMBER( c1571_t::cia_cnt_w )
{
	m_cnt_out = state;

	update_iec();
}

WRITE_LINE_MEMBER( c1571_t::cia_sp_w )
{
	m_sp_out = state;

	update_iec();
}

READ8_MEMBER( c1571_t::cia_pb_r )
{
	return m_parallel_data;
}

WRITE8_MEMBER( c1571_t::cia_pb_w )
{
	if (m_other != nullptr)
	{
		m_other->parallel_data_w(data);
	}
}


//-------------------------------------------------
//  MOS6526_INTERFACE( mini_chief_cia_intf )
//-------------------------------------------------

READ8_MEMBER( mini_chief_t::cia_pa_r )
{
	// TODO read from ISA bus @ 0x320 | A2 A1 A0

	return 0;
}

WRITE8_MEMBER( mini_chief_t::cia_pa_w )
{
	// TODO write to ISA bus @ 0x320 | A2 A1 A0
}

WRITE8_MEMBER( mini_chief_t::cia_pb_w )
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

WRITE_LINE_MEMBER( c1571_t::byte_w )
{
	m_via1->write_ca1(state);

	m_maincpu->set_input_line(M6502_SET_OVERFLOW, state);
}


//-------------------------------------------------
//  SLOT_INTERFACE( c1571_floppies )
//-------------------------------------------------

static SLOT_INTERFACE_START( c1571_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END


//-------------------------------------------------
//  floppy_interface c1571_floppy_interface
//-------------------------------------------------

void c1571_t::wpt_callback(floppy_image_device *floppy, int state)
{
	m_via0->write_ca2(!state);
}


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( c1571_t::floppy_formats )
	FLOPPY_D64_FORMAT,
	FLOPPY_G64_FORMAT,
	FLOPPY_D71_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  isa8bus_interface isabus_intf
//-------------------------------------------------

static SLOT_INTERFACE_START( mini_chief_isa8_cards )
	SLOT_INTERFACE("wd1002a_wx1", ISA8_WD1002A_WX1)
SLOT_INTERFACE_END

//-------------------------------------------------
//  MACHINE_DRIVER( c1570 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c1570 )
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_16MHz/16)
	MCFG_CPU_PROGRAM_MAP(c1571_mem)
	MCFG_QUANTUM_PERFECT_CPU(M6502_TAG)

	MCFG_DEVICE_ADD(M6522_0_TAG, VIA6522, XTAL_16MHz/16)
	MCFG_VIA6522_READPA_HANDLER(READ8(c1571_t, via0_pa_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(c1571_t, via0_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(c1571_t, via0_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(c1571_t, via0_pb_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(c1571_t, via0_irq_w))

	MCFG_DEVICE_ADD(M6522_1_TAG, VIA6522, XTAL_16MHz/16)
	MCFG_VIA6522_READPA_HANDLER(DEVREAD8(C64H156_TAG, c64h156_device, yb_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(c1571_t, via1_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(DEVWRITE8(C64H156_TAG, c64h156_device, yb_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(c1571_t, via1_pb_w))
	MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE(C64H156_TAG, c64h156_device, soe_w))
	MCFG_VIA6522_CB2_HANDLER(DEVWRITELINE(C64H156_TAG, c64h156_device, oe_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(c1571_t, via1_irq_w))

	MCFG_DEVICE_ADD(M6526_TAG, MOS6526, XTAL_16MHz/16)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(c1571_t, cia_irq_w))
	MCFG_MOS6526_CNT_CALLBACK(WRITELINE(c1571_t, cia_cnt_w))
	MCFG_MOS6526_SP_CALLBACK(WRITELINE(c1571_t, cia_sp_w))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(c1571_t, cia_pb_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(WRITE8(c1571_t, cia_pb_w))
	MCFG_MOS6526_PC_CALLBACK(WRITELINE(c1571_t, cia_pc_w))

	MCFG_WD1770_ADD(WD1770_TAG, XTAL_16MHz/2)
	MCFG_DEVICE_ADD(C64H156_TAG, C64H156, XTAL_16MHz)
	MCFG_64H156_BYTE_CALLBACK(WRITELINE(c1571_t, byte_w))
	MCFG_FLOPPY_DRIVE_ADD(C64H156_TAG":0", c1571_floppies, "525qd", c1571_t::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c1570_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c1570 );
}


//-------------------------------------------------
//  MACHINE_DRIVER( c1571 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c1571 )
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_16MHz/16)
	MCFG_CPU_PROGRAM_MAP(c1571_mem)
	MCFG_QUANTUM_PERFECT_CPU(M6502_TAG)

	MCFG_DEVICE_ADD(M6522_0_TAG, VIA6522, XTAL_16MHz/16)
	MCFG_VIA6522_READPA_HANDLER(READ8(c1571_t, via0_pa_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(c1571_t, via0_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(c1571_t, via0_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(c1571_t, via0_pb_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(c1571_t, via0_irq_w))

	MCFG_DEVICE_ADD(M6522_1_TAG, VIA6522, XTAL_16MHz/16)
	MCFG_VIA6522_READPA_HANDLER(DEVREAD8(C64H156_TAG, c64h156_device, yb_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(c1571_t, via1_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(DEVWRITE8(C64H156_TAG, c64h156_device, yb_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(c1571_t, via1_pb_w))
	MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE(C64H156_TAG, c64h156_device, soe_w))
	MCFG_VIA6522_CB2_HANDLER(DEVWRITELINE(C64H156_TAG, c64h156_device, oe_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(c1571_t, via1_irq_w))

	MCFG_DEVICE_ADD(M6526_TAG, MOS6526, XTAL_16MHz/16)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(c1571_t, cia_irq_w))
	MCFG_MOS6526_CNT_CALLBACK(WRITELINE(c1571_t, cia_cnt_w))
	MCFG_MOS6526_SP_CALLBACK(WRITELINE(c1571_t, cia_sp_w))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(c1571_t, cia_pb_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(WRITE8(c1571_t, cia_pb_w))
	MCFG_MOS6526_PC_CALLBACK(WRITELINE(c1571_t, cia_pc_w))

	MCFG_WD1770_ADD(WD1770_TAG, XTAL_16MHz/2)
	MCFG_DEVICE_ADD(C64H156_TAG, C64H156, XTAL_16MHz)
	MCFG_64H156_BYTE_CALLBACK(WRITELINE(c1571_t, byte_w))
	MCFG_FLOPPY_DRIVE_ADD(C64H156_TAG":0", c1571_floppies, "525qd", c1571_t::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c1571_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c1571 );
}


//-------------------------------------------------
//  MACHINE_DRIVER( c1571cr )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c1571cr )
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_16MHz/16)
	MCFG_CPU_PROGRAM_MAP(c1571_mem)
	MCFG_QUANTUM_PERFECT_CPU(M6502_TAG)

	MCFG_DEVICE_ADD(M6522_0_TAG, VIA6522, XTAL_16MHz/16)
	MCFG_VIA6522_READPA_HANDLER(READ8(c1571_t, via0_pa_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(c1571_t, via0_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(c1571cr_t, via0_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(c1571cr_t, via0_pb_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(c1571_t, via0_irq_w))

	MCFG_DEVICE_ADD(M6522_1_TAG, VIA6522, XTAL_16MHz/16)
	MCFG_VIA6522_READPA_HANDLER(DEVREAD8(C64H156_TAG, c64h156_device, yb_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(c1571_t, via1_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(DEVWRITE8(C64H156_TAG, c64h156_device, yb_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(c1571_t, via1_pb_w))
	MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE(C64H156_TAG, c64h156_device, soe_w))
	MCFG_VIA6522_CB2_HANDLER(DEVWRITELINE(C64H156_TAG, c64h156_device, oe_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(c1571_t, via1_irq_w))

	//MCFG_MOS5710_ADD(M5710_TAG, XTAL_16MHz/16, 0)

	MCFG_WD1770_ADD(WD1770_TAG, XTAL_16MHz/2)
	MCFG_DEVICE_ADD(C64H156_TAG, C64H156, XTAL_16MHz)
	MCFG_64H156_BYTE_CALLBACK(WRITELINE(c1571_t, byte_w))
	MCFG_FLOPPY_DRIVE_ADD(C64H156_TAG":0", c1571_floppies, "525qd", c1571_t::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c1571cr_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c1571cr );
}


//-------------------------------------------------
//  MACHINE_DRIVER( mini_chief )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( mini_chief )
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_16MHz/16)
	MCFG_CPU_PROGRAM_MAP(mini_chief_mem)
	MCFG_QUANTUM_PERFECT_CPU(M6502_TAG)

	MCFG_DEVICE_ADD(M6522_0_TAG, VIA6522, XTAL_16MHz/16)
	MCFG_VIA6522_READPA_HANDLER(READ8(c1571_t, via0_pa_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(c1571_t, via0_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(c1571_t, via0_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(c1571_t, via0_pb_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(c1571_t, via0_irq_w))

	MCFG_DEVICE_ADD(M6522_1_TAG, VIA6522, XTAL_16MHz/16)
	MCFG_VIA6522_READPA_HANDLER(DEVREAD8(C64H156_TAG, c64h156_device, yb_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(c1571_t, via1_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(DEVWRITE8(C64H156_TAG, c64h156_device, yb_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(c1571_t, via1_pb_w))
	MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE(C64H156_TAG, c64h156_device, soe_w))
	MCFG_VIA6522_CB2_HANDLER(DEVWRITELINE(C64H156_TAG, c64h156_device, oe_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(c1571_t, via1_irq_w))

	MCFG_DEVICE_ADD(M6526_TAG, MOS6526, XTAL_16MHz/16)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(c1571_t, cia_irq_w))
	MCFG_MOS6526_CNT_CALLBACK(WRITELINE(c1571_t, cia_cnt_w))
	MCFG_MOS6526_SP_CALLBACK(WRITELINE(c1571_t, cia_sp_w))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(c1571_t, cia_pb_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(WRITE8(c1571_t, cia_pb_w))
	MCFG_MOS6526_PC_CALLBACK(WRITELINE(c1571_t, cia_pc_w))

	MCFG_WD1770_ADD(WD1770_TAG, XTAL_16MHz/2)
	MCFG_DEVICE_ADD(C64H156_TAG, C64H156, XTAL_16MHz)
	MCFG_64H156_BYTE_CALLBACK(WRITELINE(c1571_t, byte_w))
	MCFG_FLOPPY_DRIVE_ADD(C64H156_TAG":0", c1571_floppies, "525qd", c1571_t::floppy_formats)

	MCFG_DEVICE_ADD(ISA_BUS_TAG, ISA8, 0)
	MCFG_ISA8_CPU(M6502_TAG)
	MCFG_ISA8_SLOT_ADD(ISA_BUS_TAG, "isa1", mini_chief_isa8_cards, "wd1002a_wx1", false)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor mini_chief_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mini_chief );
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

ioport_constructor c1571_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( c1571 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1571_t - constructor
//-------------------------------------------------

c1571_t::c1571_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
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

c1571_t::c1571_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, C1571, "C1571", tag, owner, clock, "c1571", __FILE__),
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


//-------------------------------------------------
//  c1570_t - constructor
//-------------------------------------------------

c1570_t::c1570_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: c1571_t(mconfig, C1570, "C1570", tag, owner, clock, "c1570", __FILE__)
{
}


//-------------------------------------------------
//  c1571cr_t - constructor
//-------------------------------------------------

c1571cr_t::c1571cr_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: c1571_t(mconfig, C1571CR, "C1571CR", tag, owner, clock, "c1571cr", __FILE__)
{
}


//-------------------------------------------------
//  mini_chief_t - constructor
//-------------------------------------------------

mini_chief_t::mini_chief_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: c1571_t(mconfig, MINI_CHIEF, "ICT Mini Chief", tag, owner, clock, "minichif", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c1571_t::device_start()
{
	// install image callbacks
	m_ga->set_floppy(m_floppy);
	//m_fdc->set_floppy(m_floppy);
	m_floppy->setup_wpt_cb(floppy_image_device::wpt_cb(FUNC(c1571_t::wpt_callback), this));

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

void c1571_t::device_reset()
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

void c1571_t::cbm_iec_srq(int state)
{
	update_iec();
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void c1571_t::cbm_iec_atn(int state)
{
	update_iec();
}


//-------------------------------------------------
//  cbm_iec_data -
//-------------------------------------------------

void c1571_t::cbm_iec_data(int state)
{
	update_iec();
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void c1571_t::cbm_iec_reset(int state)
{
	if (!state)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  parallel_data_w -
//-------------------------------------------------

void c1571_t::parallel_data_w(UINT8 data)
{
	m_parallel_data = data;
}


//-------------------------------------------------
//  parallel_strobe_w -
//-------------------------------------------------

void c1571_t::parallel_strobe_w(int state)
{
	m_cia->flag_w(state);
}


//-------------------------------------------------
//  update_iec -
//-------------------------------------------------

void c1571_t::update_iec()
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
