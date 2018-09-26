// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    IMI 5000H 5.25" Winchester Hard Disk Controller emulation

    Used in Corvus Systems H-Series drives (Model 6/11/20)

**********************************************************************/

#include "emu.h"
#include "imi5000h.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80_TAG         "u70"
#define Z80CTC_TAG      "u45"
#define Z80PIO_0_TAG    "u25"
#define Z80PIO_2_TAG    "u64"
#define Z80PIO_3_TAG    "u73"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(IMI5000H, imi5000h_device, "imi5000h", "IMI 5000H")


//-------------------------------------------------
//  ROM( imi5000h )
//-------------------------------------------------

ROM_START( imi5000h )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_LOAD( "c 7.63.u62", 0x0000, 0x1000, CRC(822aac68) SHA1(ab3ad7726ab20dd1041cb754d266e2f191fa3ec3) )

	ROM_REGION( 0x320, "proms", 0 )
	ROM_LOAD( "8152323-2a.u52", 0x000, 0x100, CRC(b36bc7e1) SHA1(de00b5bc17ff86b66af3e974dfd9b53245de12bd) )
	ROM_LOAD( "8152323-4a.u53", 0x100, 0x100, CRC(016fe2f7) SHA1(909f815a61e759fdf998674ee383512ecd8fee65) )
	ROM_LOAD( "8152323-1a.u54", 0x200, 0x100, CRC(512f1f39) SHA1(50c68289a19fdfca3665dbb0e98373608458c5d8) )
	ROM_LOAD( "8152323-3a.u71", 0x300, 0x020, CRC(b1092f02) SHA1(646c5a3e951535a80d24d9ce8764a3f373c508db) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *imi5000h_device::device_rom_region() const
{
	return ROM_NAME( imi5000h );
}


//-------------------------------------------------
//  ADDRESS_MAP( imi5000h_mem )
//-------------------------------------------------

void imi5000h_device::imi5000h_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).mirror(0x1000).rom().region(Z80_TAG, 0);
	map(0x4000, 0x47ff).mirror(0x1800).ram();
	map(0x6000, 0x63ff).mirror(0x1c00).ram();
	map(0x8000, 0x83ff).mirror(0x1c00).ram();
	map(0xa000, 0xa3ff).mirror(0x1c00).ram();
}


//-------------------------------------------------
//  ADDRESS_MAP( imi5000h_io )
//-------------------------------------------------

void imi5000h_device::imi5000h_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x9f);
	map(0x00, 0x03).rw(Z80PIO_0_TAG, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x08, 0x0b).rw(Z80PIO_2_TAG, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x0c, 0x0f).rw(Z80PIO_3_TAG, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x10, 0x10).mirror(0x03); // BEGRDY
	map(0x14, 0x14).mirror(0x03); // HSXCLR
	map(0x18, 0x18).mirror(0x03); // XFERSTB
	map(0x1c, 0x1f).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}


//-------------------------------------------------
//  z80_daisy_config z80_daisy_chain
//-------------------------------------------------

static const z80_daisy_config z80_daisy_chain[] =
{
	{ Z80PIO_0_TAG },
	{ Z80CTC_TAG },
	{ Z80PIO_2_TAG },
	{ nullptr }
};


//-------------------------------------------------
//  Z80CTC
//-------------------------------------------------

WRITE_LINE_MEMBER( imi5000h_device::ctc_z0_w )
{
	m_ctc->trg1(state);
}

WRITE_LINE_MEMBER( imi5000h_device::ctc_z1_w )
{
	m_ctc->trg2(state);
	m_ctc->trg3(state);
}

WRITE_LINE_MEMBER( imi5000h_device::ctc_z2_w )
{
	//m_memory_enable = state;
	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}

//-------------------------------------------------
//  Z80PIO 1
//-------------------------------------------------

READ8_MEMBER( imi5000h_device::pio0_pa_r )
{
	/*

	    bit     description

	    0       -SEEK COMPLETE
	    1       -SECTOR SIZE 2 (UB4:4)
	    2       -SECTOR SIZE 1 (UB4:1)
	    3       -SECTOR SEL
	    4       CRC ERROR
	    5       WRITE FAULT
	    6       -INDEX SEL
	    7

	*/

	return 0;
}

WRITE8_MEMBER( imi5000h_device::pio0_pa_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7       ACTIVITY LED

	*/
}

READ8_MEMBER( imi5000h_device::pio0_pb_r )
{
	/*

	    bit     description

	    0       -READY
	    1
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	return 0;
}

WRITE8_MEMBER( imi5000h_device::pio0_pb_w )
{
	/*

	    bit     description

	    0
	    1       DIRECTION IN
	    2       -HSXSTB
	    3       STEP
	    4       HEAD SEL 2^0
	    5       HEAD SEL 2^1
	    6       HEAD SEL 2^2
	    7       REDUCE WR CURRENT

	*/
}

//-------------------------------------------------
//  Z80PIO 2
//-------------------------------------------------

READ8_MEMBER( imi5000h_device::pio2_pa_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6       -SYNC
	    7       -DRV.ACK

	*/

	return 0;
}

WRITE8_MEMBER( imi5000h_device::pio2_pa_w )
{
	/*

	    bit     description

	    0       BUS DIR
	    1       -DRV.ACK
	    2       -ALT SEL
	    3       -HSXFER
	    4       PIO RDY
	    5       -COMPL
	    6
	    7

	*/
}

READ8_MEMBER( imi5000h_device::pio2_pb_r )
{
	// command bus
	return 0;
}

WRITE8_MEMBER( imi5000h_device::pio2_pb_w )
{
	// command bus
}

//-------------------------------------------------
//  Z80PIO 3
//-------------------------------------------------

READ8_MEMBER( imi5000h_device::pio3_pa_r )
{
	/*

	    bit     description

	    0       -TIMEOUT DISABLE (UB4:8)
	    1       -UNIT SELECT 1 (UB4:7)
	    2       -UNIT SELECT 2 (UB4:6)
	    3       SYSTEM/-DIAG (UB4:5)
	    4       -RXD
	    5
	    6       -TRACK 00
	    7

	*/

	return 0;
}

WRITE8_MEMBER( imi5000h_device::pio3_pa_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5       TXD
	    6
	    7       -WRITE DISABLE

	*/
}

READ8_MEMBER( imi5000h_device::pio3_pb_r )
{
	/*

	    bit     description

	    0
	    1
	    2       6MB1
	    3       -WRITE PROTECT (W2)
	    4       -FORMAT ENABLE
	    5       6MB2
	    6       12MB1
	    7       12MB2

	*/

	return 0;
}

WRITE8_MEMBER( imi5000h_device::pio3_pb_w )
{
	/*

	    bit     description

	    0       -DRV 1 SEL
	    1       -DRV 2 SEL
	    2
	    3
	    4
	    5
	    6
	    7

	*/
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void imi5000h_device::device_add_mconfig(machine_config & config)
{
	Z80(config, m_maincpu, XTAL(8'000'000)/2);
	m_maincpu->set_daisy_config(z80_daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &imi5000h_device::imi5000h_mem);
	m_maincpu->set_addrmap(AS_IO, &imi5000h_device::imi5000h_io);

	Z80CTC(config, m_ctc, XTAL(8'000'000) / 2);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(FUNC(imi5000h_device::ctc_z0_w));
	m_ctc->zc_callback<1>().set(FUNC(imi5000h_device::ctc_z1_w));
	m_ctc->zc_callback<2>().set(FUNC(imi5000h_device::ctc_z2_w));

	z80pio_device& pio0(Z80PIO(config, Z80PIO_0_TAG, XTAL(8'000'000)/2));
	pio0.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	pio0.in_pa_callback().set(FUNC(imi5000h_device::pio0_pa_r));
	pio0.out_pa_callback().set(FUNC(imi5000h_device::pio0_pa_w));
	pio0.out_ardy_callback().set(Z80PIO_0_TAG, FUNC(z80pio_device::strobe_a));
	pio0.in_pb_callback().set(FUNC(imi5000h_device::pio0_pb_r));
	pio0.out_pb_callback().set(FUNC(imi5000h_device::pio0_pb_w));
	pio0.out_brdy_callback().set(Z80PIO_0_TAG, FUNC(z80pio_device::strobe_b));

	z80pio_device& pio2(Z80PIO(config, Z80PIO_2_TAG, XTAL(8'000'000)/2));
	pio2.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	pio2.in_pa_callback().set(FUNC(imi5000h_device::pio2_pa_r));
	pio2.out_pa_callback().set(FUNC(imi5000h_device::pio2_pa_w));
	pio2.out_ardy_callback().set(Z80PIO_2_TAG, FUNC(z80pio_device::strobe_a));
	pio2.in_pb_callback().set(FUNC(imi5000h_device::pio2_pb_r));
	pio2.out_pb_callback().set(FUNC(imi5000h_device::pio2_pb_w));

	z80pio_device& pio3(Z80PIO(config, Z80PIO_3_TAG, XTAL(8'000'000)/2));
	pio3.in_pa_callback().set(FUNC(imi5000h_device::pio3_pa_r));
	pio3.out_pa_callback().set(FUNC(imi5000h_device::pio3_pa_w));
	pio3.out_ardy_callback().set(Z80PIO_3_TAG, FUNC(z80pio_device::strobe_a));
	pio3.in_pb_callback().set(FUNC(imi5000h_device::pio3_pb_r));
	pio3.out_pb_callback().set(FUNC(imi5000h_device::pio3_pb_w));
	pio3.out_brdy_callback().set(Z80PIO_3_TAG, FUNC(z80pio_device::strobe_b));

	//HARDDISK(config, "harddisk1", 0);
}


//-------------------------------------------------
//  INPUT_PORTS( imi5000h )
//-------------------------------------------------

static INPUT_PORTS_START( imi5000h )
	PORT_START("LSI-11")
	PORT_DIPNAME( 0x01, 0x00, "LSI-11" )
	PORT_DIPSETTING(    0x01, "Normal" )
	PORT_DIPSETTING(    0x00, "LSI-11" ) // emulate DEC RL01 and RL02

	PORT_START("MUX")
	PORT_DIPNAME( 0x01, 0x00, "MUX" )
	PORT_DIPSETTING(    0x01, "Single" )
	PORT_DIPSETTING(    0x00, "Multiplexer" ) // Corvus Multiplexer Network

	PORT_START("FORMAT")
	PORT_DIPNAME( 0x01, 0x00, "FORMAT" )
	PORT_DIPSETTING(    0x01, "Normal" ) // read controller firmware from cylinders 0 and 1
	PORT_DIPSETTING(    0x00, "Format" ) // drive ready after self-test, allow format

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("RESET")

	PORT_START("UB4")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "UB4:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "UB4:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "UB4:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "UB4:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "UB4:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "UB4:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "UB4:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "UB4:8" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor imi5000h_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( imi5000h );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  imi5000h_device - constructor
//-------------------------------------------------

imi5000h_device::imi5000h_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, IMI5000H, tag, owner, clock),
	device_imi7000_interface(mconfig, *this),
	m_maincpu(*this, Z80_TAG),
	m_ctc(*this, Z80CTC_TAG),
	m_lsi11(*this, "LSI-11"),
	m_mux(*this, "MUX"),
	m_format(*this, "FORMAT"),
	m_ub4(*this, "UB4")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void imi5000h_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void imi5000h_device::device_reset()
{
	m_maincpu->reset();
	m_ctc->reset();
}
