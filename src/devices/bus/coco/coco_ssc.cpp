// license:BSD-3-Clause
// copyright-holders:tim lindner
/***************************************************************************

    coco_ssc.cpp

    Code for emulating the CoCo Speech / Sound Cartridge

    The SSC was a complex sound cartridge. A TMS-7040 microcontroller,
    SPO256-AL2 speech processor, AY-3-8913 programable sound generator, and
    2K of RAM.

	All four ports of the microcontroller are under software control.

	Port A is input from the host CPU.
	Port B is A0-A7 for the 2k of RAM.
	Port C is the internal bus controller:
		bit 7 6 5 4 3 2 1 0
			| | | | | | | |
			| | | | | | | + A8 for RAM and BC1 of AY3-8913
			| | | | | | +-- A9 for RAM
			| | | | | +---- A10 for RAM
			| | | | +------ R/W* for RAM and BDIR for AY3-8913
			| | | +-------- CS* for RAM
			| | +---------- ALD* for SP0256
			| +------------ CS* for AY3-8913
			+-------------- BUSY* for host CPU (connects to a latch)
		* – Active low
	Port D is the 8-bit data bus.

***************************************************************************/

#include "emu.h"
#include "coco_ssc.h"
#include "cpu/tms7000/tms7000.h"
#include "speaker.h"

#define LOG_SSC 0
#define PIC_TAG "pic7040"
#define AY_TAG "cocossc_ay"
#define SP0256_TAG "sp0256"

#define C_A8   0x01
#define C_BC1  0x01
#define C_A9   0x02
#define C_A10  0x04
#define C_RRW  0x08
#define C_BDR  0x08
#define C_RCS  0x10
#define C_ALD  0x20
#define C_ACS  0x40
#define C_BSY  0x80

#ifdef SAC_ON
//-------------------------------------------------
//  nl_ssc - Sound Activity Circuit net list
//-------------------------------------------------

static NETLIST_START(nl_sac)

	/* Standard stuff */

 	SOLVER(Solver, 48000)
	ANALOG_INPUT(V5, 5)
	ANALOG_INPUT(VN5, -5)
	// PARAM(Solver.ACCURACY, 1e-6)
// 	PARAM(Solver.GS_LOOPS, 6)
// 	PARAM(Solver.SOR_FACTOR, 1.0)

	RES(R20,    820)
	RES(R6,   10000)z
	RES(R7,    9100)
	RES(R5,  100000)
	RES(R2,   10000)
	RES(R1,  100000)
RES(R9, 56000)
RES(R8, 10000)
RES(R3, 2400000)
RES(R4, 4700)

CAP(C9, CAP_U(1.0))
CAP(C25, CAP_U(1.5))

DIODE(D2, "1N4148")
DIODE(D1, "1N4148")
DIODE(D3, "1N4148")

LM324_DIP(IC1)

NET_C(R20.2, GND)
NET_C(IC1.5, R20.1)
NET_C(IC1.7, IC1.6, C9.1)
NET_C(IC1.11, VN5)
NET_C(C9.2, R6.1)
NET_C(R6.2, IC1.2, R5.1)
NET_C(IC1.3, R7.1)
NET_C(R7.2, GND)
NET_C(IC1.1, R5.2, R2.1)
NET_C(R2.2, IC1.13, D2.A, R1.1)
NET_C(IC1.4, V5)
NET_C(IC1.12, GND)
NET_C(IC1.14, D2.K, D1.A)
NET_C(R1.2, D1.K, C25.1, IC1.9)
NET_C(C25.2, GND)
NET_C(V5, R9.1)
NET_C(R9.2, IC1.10, R3.1, R8.1)
NET_C(R8.2, GND)
NET_C(R3.2, IC1.8, D3.A)
NET_C(D3.K, R4.1)
NET_C(R4.2, GND)
NETLIST_END()

#endif

static ADDRESS_MAP_START(ssc_io_map, AS_IO, 8, coco_ssc_device)
AM_RANGE(TMS7000_PORTA, TMS7000_PORTA) AM_READ(ssc_port_a_r)
AM_RANGE(TMS7000_PORTB, TMS7000_PORTB) AM_WRITE(ssc_port_b_w)
AM_RANGE(TMS7000_PORTC, TMS7000_PORTC) AM_READWRITE(ssc_port_c_r, ssc_port_c_w)
AM_RANGE(TMS7000_PORTD, TMS7000_PORTD) AM_READWRITE(ssc_port_d_r, ssc_port_d_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(ssc_rom, AS_PROGRAM, 8, coco_ssc_device)
AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION(PIC_TAG, 0)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT(coco_ssc)
MCFG_CPU_ADD(PIC_TAG, TMS7040, XTAL_3_579545MHz / 2)
MCFG_CPU_PROGRAM_MAP(ssc_rom)
MCFG_CPU_IO_MAP(ssc_io_map)

MCFG_RAM_ADD("staticram")
MCFG_RAM_DEFAULT_SIZE("2K")
MCFG_RAM_DEFAULT_VALUE(0x00)

MCFG_SPEAKER_STANDARD_MONO("sscmono")

MCFG_SOUND_ADD(SP0256_TAG, SP0256, XTAL_3_12MHz)
MCFG_SOUND_ROUTE(ALL_OUTPUTS, "sscmono", 1.75)
MCFG_SP0256_DATA_REQUEST_CB(INPUTLINE(PIC_TAG, TMS7000_INT1_LINE))

#ifdef SAC_ON
MCFG_SOUND_ADD(AY_TAG, AY8913, XTAL_3_579545MHz / 4)
MCFG_AY8910_OUTPUT_TYPE(AY8910_RESISTOR_OUTPUT)
MCFG_AY8910_RES_LOADS(820, 820, 820)
MCFG_SOUND_ROUTE_EX(0, "snd_nl", 1.0, 0)
MCFG_SOUND_ROUTE_EX(1, "snd_nl", 1.0, 1)
MCFG_SOUND_ROUTE_EX(2, "snd_nl", 1.0, 2)

MCFG_SOUND_ADD("snd_nl", NETLIST_SOUND, XTAL_3_579545MHz / 4)
MCFG_NETLIST_SETUP(nl_sac)
MCFG_SOUND_ROUTE(ALL_OUTPUTS, "sscmono", 1.0)
MCFG_NETLIST_STREAM_INPUT("snd_nl", 0, "R20.R")
MCFG_NETLIST_STREAM_INPUT("snd_nl", 1, "R20.R")
MCFG_NETLIST_STREAM_INPUT("snd_nl", 2, "R20.R")

MCFG_NETLIST_STREAM_OUTPUT("snd_nl", 0, "R20.1")
MCFG_NETLIST_LOGIC_OUTPUT("", "cocossc", "sac", coco_ssc_device, sac_cb, "cocossc_tag")
#else
MCFG_SOUND_ADD(AY_TAG, AY8913, XTAL_3_579545MHz / 4)
MCFG_SOUND_ROUTE(ALL_OUTPUTS, "sscmono", 2.0)
#endif

MACHINE_CONFIG_END

ROM_START(coco_ssc)
ROM_REGION(0x1000, PIC_TAG, 0)
ROM_LOAD("pic-7040-510.bin", 0x0000, 0x1000, CRC(a8e2eb98) SHA1(7c17dcbc21757535ce0b3a9e1ce5ca61319d3606)) // pic7040 cpu rom
ROM_REGION(0x10000, SP0256_TAG, 0)
ROM_LOAD("sp0256-al2.bin", 0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc))
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(COCO_SSC, coco_ssc_device, "coco_ssc", "CoCo S/SC PAK");


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  coco_ssc_device - constructor
//-------------------------------------------------

coco_ssc_device::coco_ssc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, COCO_SSC, tag, owner, clock),
		device_cococart_interface(mconfig, *this ),
		m_tms7040(*this, PIC_TAG),
		m_staticram(*this, "staticram"),
		m_ay(*this, AY_TAG),
		m_spo(*this, SP0256_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_ssc_device::device_start()
{
	// install $FF7D-E handler
	write8_delegate wh = write8_delegate(FUNC(coco_ssc_device::ff7d_write), this);
	read8_delegate rh = read8_delegate(FUNC(coco_ssc_device::ff7d_read), this);
	install_readwrite_handler(0xFF7D, 0xFF7E, rh, wh);

	save_item(NAME(reset_line));
	save_item(NAME(tms7000_porta));
	save_item(NAME(tms7000_portb));
	save_item(NAME(tms7000_portc));
	save_item(NAME(tms7000_portd));
}

void coco_ssc_device::device_reset()
{
	reset_line = 0;
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor coco_ssc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( coco_ssc );
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *coco_ssc_device::device_rom_region() const
{
	return ROM_NAME( coco_ssc );
}


//-------------------------------------------------
//  coco_cartridge_set_line
//-------------------------------------------------

void coco_ssc_device::cart_set_line(cococart_slot_device::line which, cococart_slot_device::line_value value)
{

	switch (which)
	{
		case cococart_slot_device::line::SOUND_ENABLE:
			if( value == cococart_slot_device::line_value::ASSERT )
			{
				logerror( "coco_ssc_device::cart_set_line sound enable assert\n" );
				m_ay->set_volume(ALL_8910_CHANNELS,100);
			}
			else
			{
				logerror( "coco_ssc_device::cart_set_line sound enable clear\n" );
				m_ay->set_volume(ALL_8910_CHANNELS,0);
			}
			break;
		default:
			logerror( "coco_ssc_device::cart_set_line something else\n" );
			break;
	}
}


//-------------------------------------------------
//	ff7d_read
//-------------------------------------------------

READ8_MEMBER(coco_ssc_device::ff7d_read)
{
	uint8_t data = 0xff;

	switch(offset)
	{
		case 0x00:
			data = 0xff;
			break;

		case 0x01:
			data = 0x3f;

			if( tms7000_portc & C_BSY )
			{
				data |= 0x80;
			}

			if( m_spo->sby_r() )
			{
				data |= 0x40;
			}

			break;
	}

	return data;
}


//-------------------------------------------------
//	ff7d_write
//-------------------------------------------------

WRITE8_MEMBER(coco_ssc_device::ff7d_write)
{
	switch(offset)
	{
		case 0x00:
			if( (reset_line & 1) == 1 )
			{
				if( (data & 1) == 0 )
				{
					m_tms7040->reset();
					m_ay->reset();
 					m_spo->reset();
				}
			}

			reset_line = data;
			break;

		case 0x01:
			tms7000_porta = data;
			m_tms7040->set_input_line(TMS7000_INT3_LINE, ASSERT_LINE);
			break;
	}
}

#ifdef SAC_ON
//-------------------------------------------------
//  Callback for Sound Activity Circuit
//-------------------------------------------------

NETDEV_LOGIC_CALLBACK_MEMBER(coco_ssc_device::sac_cb)
{
	logerror( "sac_cb: %d\n", data );
}
#endif

//-------------------------------------------------
//  Handlers for secondary CPU ports
//-------------------------------------------------

READ8_MEMBER(coco_ssc_device::ssc_port_a_r)
{
	if (LOG_SSC)
	{
		logerror( "[0x%04x] port a read: %02x\n", space.device().safe_pc(), tms7000_porta );
	}

 	m_tms7040->set_input_line(TMS7000_INT3_LINE, CLEAR_LINE);

	return tms7000_porta;
}

WRITE8_MEMBER(coco_ssc_device::ssc_port_b_w)
{
	if (LOG_SSC)
	{
		logerror( "[0x%04x] port b write: %02x\n", space.device().safe_pc(), data );
	}

	tms7000_portb = data;
}

READ8_MEMBER(coco_ssc_device::ssc_port_c_r)
{
	if (LOG_SSC)
	{
		logerror( "[0x%04x] port c read: %02x\n", space.device().safe_pc(), tms7000_portc );
	}

	return tms7000_portc;
}

WRITE8_MEMBER(coco_ssc_device::ssc_port_c_w)
{
	if( (data & C_RCS) == 0 && (data & C_RRW) == 0) /* static RAM write */
	{
		uint16_t address = (uint16_t)data << 8;
		address += tms7000_portb;
		address &= 0x7ff;

		m_staticram->write(address, tms7000_portd);
	}

	if( (data & C_ACS) == 0 ) /* chip select for AY-3-8913 */
	{
		if( (data & (C_BDR|C_BC1)) == (C_BDR|C_BC1) ) /* BDIR = 1, BC1 = 1: latch address */
		{
			m_ay->address_w(space, 0, tms7000_portd);
		}

		if( ((data & C_BDR) == C_BDR) && ((data & C_BC1) == 0) ) /* BDIR = 1, BC1 = 0: write data */
		{
			m_ay->data_w(space, 0, tms7000_portd);
		}
	}

	if( (data & C_ALD) == 0 )
	{
		m_spo->ald_w(space, 0, tms7000_portd);
	}

	if (LOG_SSC)
	{
		logerror( "[0x%04x] pord c write: %c%c%c%c %c%c%c%c (%02x)\n",
			space.device().safe_pc(),
			data & 0x80 ? '.' : 'B',
			data & 0x40 ? '.' : 'P',
			data & 0x20 ? '.' : 'V',
			data & 0x10 ? '.' : 'R',
			data & 0x40 ? (data & 0x08 ? 'R' : 'W') : (data & 0x08 ? 'D' : '.'),
			data & 0x04 ? '1' : '0',
			data & 0x02 ? '1' : '0',
			data & 0x40 ? (data & 0x01 ? '1' : '0') : (data & 0x01 ? 'C' : '.'),
			data );
	}

	tms7000_portc = data;
}

READ8_MEMBER(coco_ssc_device::ssc_port_d_r)
{
	if( ((tms7000_portc & C_RCS) == 0) && ((tms7000_portc & C_ACS) == 0))
		logerror( "[0x%04x] Warning: Reading RAM and PSG at the same time!\n", space.device().safe_pc() );

	if( ((tms7000_portc & C_RCS) == 0)  && ((tms7000_portc & C_RRW) == C_RRW)) /* static ram chip select (low) and static ram chip read (high) */
	{
		uint16_t address = (uint16_t)tms7000_portc << 8;
		address += tms7000_portb;
		address &= 0x7ff;

		tms7000_portd = m_staticram->read(address);
	}

	if( (tms7000_portc & C_ACS) == 0 ) /* chip select for AY-3-8913 */
	{
		if( ((tms7000_portc & C_BDR) == 0) && ((tms7000_portc & C_BC1) == C_BC1) ) /* psg read data */
		{
			tms7000_portd = m_ay->data_r(space, 0);
		}
	}

	if (LOG_SSC)
	{
		logerror( "[0x%04x] port d read: %02x\n", space.device().safe_pc(), tms7000_portd );
	}

	return tms7000_portd;
}

WRITE8_MEMBER(coco_ssc_device::ssc_port_d_w)
{
	if (LOG_SSC)
	{
		logerror( "[0x%04x] port d write: %02x\n", space.device().safe_pc(), data );
	}

	tms7000_portd = data;
}
