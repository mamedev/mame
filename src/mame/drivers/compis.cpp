// license:BSD-3-Clause
// copyright-holders:Curt Coder
// thanks-to:Per Ola Ingvarsson, Tomas Karlsson
/******************************************************************************

    drivers/compis.c
    machine driver

    Hardware:
        - Intel 80186 CPU 8MHz, integrated DMA(8237?), PIC(8259?), PIT(8253?)
                - Intel 80130 OSP Operating system processor (PIC 8259, PIT 8254)
        - Intel 8274 MPSC Multi-protocol serial communications controller (NEC 7201)
        - Intel 8255 PPI Programmable peripheral interface
        - Intel 8253 PIT Programmable interval timer
        - Intel 8251 USART Universal synchronous asynchronous receiver transmitter
        - National 58174 Real-time clock (compatible with 58274)
    Peripheral:
        - Intel 82720 GDC Graphic display processor (NEC uPD 7220)
        - Intel 8272 FDC Floppy disk controller (Intel iSBX-218A)
        - Western Digital WD1002-05 Winchester controller

    Memory map:

    00000-3FFFF RAM LMCS (Low Memory Chip Select)
    40000-4FFFF RAM MMCS 0 (Midrange Memory Chip Select)
    50000-5FFFF RAM MMCS 1 (Midrange Memory Chip Select)
    60000-6FFFF RAM MMCS 2 (Midrange Memory Chip Select)
    70000-7FFFF RAM MMCS 3 (Midrange Memory Chip Select)
    80000-EFFFF NOP
    F0000-FFFFF ROM UMCS (Upper Memory Chip Select)

18/08/2011 -[Robbbert]
- Modernised
- Removed F4 display, as the gfx is in different places per bios.
- Changed to monochrome, it usually had a greenscreen monitor, although some
  were amber.
- Still doesn't work.
- Added a nasty hack to get a display on compis2 (wait 20 seconds)


******************************************************************************/

#include "emu.h"
#include "softlist.h"
#include "bus/centronics/ctronics.h"
#include "bus/compis/graphics.h"
#include "bus/isbx/isbx.h"
#include "bus/rs232/rs232.h"
#include "cpu/i86/i186.h"
#include "cpu/mcs48/mcs48.h"
#include "imagedev/cassette.h"
#include "machine/compiskb.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/i80130.h"
#include "machine/mm58274c.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/z80dart.h"

#define I80186_TAG      "ic1"
#define I80130_TAG      "ic15"
#define I8251A_TAG      "ic59"
#define I8253_TAG       "ic60"
#define I8274_TAG       "ic65"
#define MM58174A_TAG    "ic66"
#define I8255_TAG       "ic69"
#define RS232_A_TAG     "rs232a"
#define RS232_B_TAG     "rs232b"
#define CASSETTE_TAG    "cassette"
#define CENTRONICS_TAG  "centronics"
#define ISBX_0_TAG      "isbx0"
#define ISBX_1_TAG      "isbx1"
#define GRAPHICS_TAG	"gfx"
#define COMPIS_KEYBOARD_TAG "compiskb"

class compis_state : public driver_device
{
public:
	compis_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, I80186_TAG),
		m_osp(*this, I80130_TAG),
		m_pit(*this, I8253_TAG),
		m_ppi(*this, I8255_TAG),
		m_mpsc(*this, I8274_TAG),
		m_centronics(*this, CENTRONICS_TAG),
		m_uart(*this, I8251A_TAG),
		m_rtc(*this, MM58174A_TAG),
		m_cassette(*this, CASSETTE_TAG),
		m_graphics(*this, GRAPHICS_TAG),
		m_isbx0(*this, ISBX_0_TAG),
		m_isbx1(*this, ISBX_1_TAG),
		m_ram(*this, RAM_TAG),
		m_s8(*this, "S8")
	{ }

	required_device<i80186_cpu_device> m_maincpu;
	required_device<i80130_device> m_osp;
	required_device<pit8253_device> m_pit;
	required_device<i8255_device> m_ppi;
	required_device<i8274_device> m_mpsc;
	required_device<centronics_device> m_centronics;
	required_device<i8251_device> m_uart;
	required_device<mm58274c_device> m_rtc;
	required_device<cassette_image_device> m_cassette;
	required_device<compis_graphics_slot_t> m_graphics;
	required_device<isbx_slot_device> m_isbx0;
	required_device<isbx_slot_device> m_isbx1;
	required_device<ram_device> m_ram;
	required_ioport m_s8;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ16_MEMBER( pcs6_0_1_r );
	DECLARE_WRITE16_MEMBER( pcs6_0_1_w );
	DECLARE_READ16_MEMBER( pcs6_2_3_r );
	DECLARE_WRITE16_MEMBER( pcs6_2_3_w );
	DECLARE_READ16_MEMBER( pcs6_4_5_r );
	DECLARE_WRITE16_MEMBER( pcs6_4_5_w );
	DECLARE_READ16_MEMBER( pcs6_6_7_r );
	DECLARE_WRITE16_MEMBER( pcs6_6_7_w );
	DECLARE_READ16_MEMBER( pcs6_8_9_r );
	DECLARE_WRITE16_MEMBER( pcs6_8_9_w );
	DECLARE_READ16_MEMBER( pcs6_10_11_r );
	DECLARE_WRITE16_MEMBER( pcs6_10_11_w );
	DECLARE_READ16_MEMBER( pcs6_12_13_r );
	DECLARE_WRITE16_MEMBER( pcs6_12_13_w );
	DECLARE_READ16_MEMBER( pcs6_14_15_r );
	DECLARE_WRITE16_MEMBER( pcs6_14_15_w );

	DECLARE_READ8_MEMBER( compis_irq_callback );

	DECLARE_READ8_MEMBER( ppi_pb_r );
	DECLARE_WRITE8_MEMBER( ppi_pc_w );

	DECLARE_WRITE_LINE_MEMBER( tmr0_w );
	DECLARE_WRITE_LINE_MEMBER( tmr1_w );
	DECLARE_WRITE_LINE_MEMBER( tmr2_w );
	DECLARE_WRITE_LINE_MEMBER( tmr5_w );

	TIMER_DEVICE_CALLBACK_MEMBER( tape_tick );

	int m_centronics_busy;
	int m_centronics_select;

	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_select);

	int m_tmr0;
};



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ16_MEMBER( compis_state::pcs6_0_1_r )
{
	if (ACCESSING_BITS_0_7)
	{
		return 0xff;
	}
	else
	{
		return m_graphics->dma_ack_r(space, offset);
	}
}

WRITE16_MEMBER( compis_state::pcs6_0_1_w )
{
	if (ACCESSING_BITS_0_7)
	{
		cassette_state state = BIT(data, 0) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED;

		m_cassette->change_state(state, CASSETTE_MASK_MOTOR);
	}
	else
	{
		m_graphics->dma_ack_w(space, offset, data);
	}
}

READ16_MEMBER( compis_state::pcs6_2_3_r )
{
	if (ACCESSING_BITS_0_7)
	{
		return m_mpsc->inta_r(space, 0);
	}
	else
	{
		if (BIT(offset, 0))
		{
			return m_uart->status_r(space, 0) << 8;
		}	
		else
		{
			return m_uart->data_r(space, 0) << 8;
		}
	}
}

WRITE16_MEMBER( compis_state::pcs6_2_3_w )
{
	if (ACCESSING_BITS_0_7)
	{
	}
	else
	{
		if (BIT(offset, 0))
		{
			m_uart->control_w(space, 0, data >> 8);
		}	
		else
		{
			m_uart->data_w(space, 0, data >> 8);
		}
	}
}

READ16_MEMBER( compis_state::pcs6_4_5_r )
{
	if (ACCESSING_BITS_0_7)
	{
		return m_mpsc->cd_ba_r(space, offset & 0x03);
	}
	else
	{
		m_isbx0->tdma_w(0);
		m_isbx0->tdma_w(1);

		return 0xff;
	}
}

WRITE16_MEMBER( compis_state::pcs6_4_5_w )
{
	if (ACCESSING_BITS_0_7)
	{
		m_mpsc->cd_ba_w(space, offset & 0x03, data);
	}
	else
	{
		m_isbx0->tdma_w(0);
		m_isbx0->tdma_w(1);
	}
}

READ16_MEMBER( compis_state::pcs6_6_7_r )
{
	if (ACCESSING_BITS_0_7)
	{
		return m_graphics->pcs6_6_r(space, offset);
	}
	else
	{
		m_isbx1->tdma_w(0);
		m_isbx1->tdma_w(1);

		return 0xff;
	}
}

WRITE16_MEMBER( compis_state::pcs6_6_7_w )
{
	if (ACCESSING_BITS_0_7)
	{
		m_graphics->pcs6_6_w(space, offset, data);
	}
	else
	{
		m_isbx1->tdma_w(0);
		m_isbx1->tdma_w(1);
	}
}

READ16_MEMBER( compis_state::pcs6_8_9_r )
{
	if (ACCESSING_BITS_0_7)
	{
		return m_isbx0->mcs0_r(space, offset);
	}
	else
	{
		return m_isbx0->mcs1_r(space, offset) << 8;
	}
}

WRITE16_MEMBER( compis_state::pcs6_8_9_w )
{
	if (ACCESSING_BITS_0_7)
	{
		m_isbx0->mcs0_w(space, offset, data);
	}
	else
	{
		m_isbx0->mcs1_w(space, offset, data >> 8);
	}
}

READ16_MEMBER( compis_state::pcs6_10_11_r )
{
	if (ACCESSING_BITS_0_7)
	{
		return m_isbx0->mcs1_r(space, offset);
	}
	else
	{
		return m_isbx0->mdack_r(space, offset) << 8;
	}
}

WRITE16_MEMBER( compis_state::pcs6_10_11_w )
{
	if (ACCESSING_BITS_0_7)
	{
		m_isbx0->mcs1_w(space, offset, data);
	}
	else
	{
		m_isbx0->mdack_w(space, offset, data >> 8);
	}
}

READ16_MEMBER( compis_state::pcs6_12_13_r )
{
	if (ACCESSING_BITS_0_7)
	{
		return m_isbx1->mcs0_r(space, offset);
	}
	else
	{
		return m_isbx1->mcs1_r(space, offset) << 8;
	}
}

WRITE16_MEMBER( compis_state::pcs6_12_13_w )
{
	if (ACCESSING_BITS_0_7)
	{
		m_isbx1->mcs0_w(space, offset, data);
	}
	else
	{
		m_isbx1->mcs1_w(space, offset, data >> 8);
	}
}

READ16_MEMBER( compis_state::pcs6_14_15_r )
{
	if (ACCESSING_BITS_0_7)
	{
		return m_isbx1->mcs1_r(space, offset);
	}
	else
	{
		return m_isbx1->mdack_r(space, offset) << 8;
	}
}

WRITE16_MEMBER( compis_state::pcs6_14_15_w )
{
	if (ACCESSING_BITS_0_7)
	{
		m_isbx1->mcs1_w(space, offset, data);
	}
	else
	{
		m_isbx1->mdack_w(space, offset, data >> 8);
	}
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( compis_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( compis_mem, AS_PROGRAM, 16, compis_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x1ffff) AM_RAM
	AM_RANGE(0x60000, 0x63fff) AM_MIRROR(0x1c000) AM_DEVICE(I80130_TAG, i80130_device, rom_map)
	AM_RANGE(0xe0000, 0xeffff) AM_MIRROR(0x10000) AM_ROM AM_REGION(I80186_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( compis2_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( compis2_mem, AS_PROGRAM, 16, compis_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0xbffff) AM_RAM
	AM_RANGE(0xe0000, 0xeffff) AM_MIRROR(0x10000) AM_ROM AM_REGION(I80186_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( compis_io )
//-------------------------------------------------

static ADDRESS_MAP_START( compis_io, AS_IO, 16, compis_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0007) /* PCS0 */ AM_MIRROR(0x78) AM_DEVREADWRITE8(I8255_TAG, i8255_device, read, write, 0xff00)
	AM_RANGE(0x0080, 0x0087) /* PCS1 */ AM_MIRROR(0x78) AM_DEVREADWRITE8(I8253_TAG, pit8253_device, read, write, 0x00ff)
	AM_RANGE(0x0100, 0x011f) /* PCS2 */ AM_MIRROR(0x60) AM_DEVREADWRITE8(MM58174A_TAG, mm58274c_device, read, write, 0x00ff)
	//AM_RANGE(0x0180, 0x0181) /* PCS3 */ AM_MIRROR(0x7e)
	//AM_RANGE(0x0200, 0x0201) /* PCS4 */ AM_MIRROR(0x7e)
	AM_RANGE(0x0280, 0x028f) /* PCS5 */ AM_MIRROR(0x70) AM_DEVICE(I80130_TAG, i80130_device, io_map)
//	AM_RANGE(0x0300, 0x0301) /* PCS6:0 */ AM_MIRROR(0xe) AM_WRITE8(tape_mon_w, 0x00ff)
//	AM_RANGE(0x0310, 0x0311) /* PCS6:3 */ AM_MIRROR(0xc) AM_DEVREADWRITE8(I8251A_TAG, i8251_device, data_r, data_w, 0xff00)
//	AM_RANGE(0x0312, 0x0313) /* PCS6:3 */ AM_MIRROR(0xc) AM_DEVREADWRITE8(I8251A_TAG, i8251_device, status_r, control_w, 0xff00)
	AM_RANGE(0x0300, 0x030f) AM_READWRITE(pcs6_0_1_r, pcs6_0_1_w)
	AM_RANGE(0x0310, 0x031f) AM_READWRITE(pcs6_2_3_r, pcs6_2_3_w)
	AM_RANGE(0x0320, 0x032f) AM_READWRITE(pcs6_4_5_r, pcs6_4_5_w)
	AM_RANGE(0x0330, 0x033f) AM_READWRITE(pcs6_6_7_r, pcs6_6_7_w)
	AM_RANGE(0x0340, 0x034f) AM_READWRITE(pcs6_8_9_r, pcs6_8_9_w)
	AM_RANGE(0x0350, 0x035f) AM_READWRITE(pcs6_10_11_r, pcs6_10_11_w)
	AM_RANGE(0x0360, 0x036f) AM_READWRITE(pcs6_12_13_r, pcs6_12_13_w)
	AM_RANGE(0x0370, 0x037f) AM_READWRITE(pcs6_14_15_r, pcs6_14_15_w)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( compis )
//-------------------------------------------------

static INPUT_PORTS_START( compis )
	PORT_START("S1")
	PORT_CONFNAME( 0x01, 0x00, "S1 ROM Type")
	PORT_CONFSETTING(    0x00, "27128" )
	PORT_CONFSETTING(    0x01, "27256" )

	PORT_START("S2")
	PORT_CONFNAME( 0x01, 0x00, "S2 IC36/IC40")
	PORT_CONFSETTING(    0x00, "ROM" )
	PORT_CONFSETTING(    0x01, "RAM" )

	PORT_START("S3")
	PORT_CONFNAME( 0x03, 0x00, "S3 J4 RxC")
	PORT_CONFSETTING(    0x00, "DCE" )
	PORT_CONFSETTING(    0x01, "Tmr3" )
	PORT_CONFSETTING(    0x02, "Tmr4" )

	PORT_START("S4")
	PORT_CONFNAME( 0x01, 0x01, "S4 iSBX0 Bus Width")
	PORT_CONFSETTING(    0x00, "8 Bit" )
	PORT_CONFSETTING(    0x01, "16 Bit" )

	PORT_START("S5")
	PORT_CONFNAME( 0x01, 0x01, "S5 iSBX1 Bus Width")
	PORT_CONFSETTING(    0x00, "8 Bit" )
	PORT_CONFSETTING(    0x01, "16 Bit" )

	PORT_START("S6")
	PORT_CONFNAME( 0x001, 0x001, "S6 INT 8274")
	PORT_CONFSETTING(     0x000, DEF_STR( Off ) )
	PORT_CONFSETTING(     0x001, DEF_STR( On ) )
	PORT_CONFNAME( 0x002, 0x000, "S6 TxRDY 8251")
	PORT_CONFSETTING(     0x000, DEF_STR( Off ) )
	PORT_CONFSETTING(     0x002, DEF_STR( On ) )
	PORT_CONFNAME( 0x004, 0x000, "S6 INT KB")
	PORT_CONFSETTING(     0x000, DEF_STR( Off ) )
	PORT_CONFSETTING(     0x004, DEF_STR( On ) )
	PORT_CONFNAME( 0x008, 0x008, "S6 DELAY 80150")
	PORT_CONFSETTING(     0x000, DEF_STR( Off ) )
	PORT_CONFSETTING(     0x008, DEF_STR( On ) )
	PORT_CONFNAME( 0x010, 0x000, "S6 INT0 iSBX1 (J9)")
	PORT_CONFSETTING(     0x000, DEF_STR( Off ) )
	PORT_CONFSETTING(     0x010, DEF_STR( On ) )
	PORT_CONFNAME( 0x020, 0x000, "S6 INT1 iSBX1 (J9)")
	PORT_CONFSETTING(     0x000, DEF_STR( Off ) )
	PORT_CONFSETTING(     0x020, DEF_STR( On ) )
	PORT_CONFNAME( 0x040, 0x040, "S6 ACK J7")
	PORT_CONFSETTING(     0x000, DEF_STR( Off ) )
	PORT_CONFSETTING(     0x040, DEF_STR( On ) )
	PORT_CONFNAME( 0x080, 0x000, "S6 SYSTICK 80150")
	PORT_CONFSETTING(     0x000, DEF_STR( Off ) )
	PORT_CONFSETTING(     0x080, DEF_STR( On ) )
	PORT_CONFNAME( 0x100, 0x100, "S6 RxRDY 8251")
	PORT_CONFSETTING(     0x000, DEF_STR( Off ) )
	PORT_CONFSETTING(     0x100, DEF_STR( On ) )
	PORT_CONFNAME( 0x200, 0x000, "S6 INT0 iSBX0 (J8)")
	PORT_CONFSETTING(     0x000, DEF_STR( Off ) )
	PORT_CONFSETTING(     0x200, DEF_STR( On ) )
	PORT_CONFNAME( 0x400, 0x400, "S6 INT1 iSBX0 (J8)")
	PORT_CONFSETTING(     0x000, DEF_STR( Off ) )
	PORT_CONFSETTING(     0x400, DEF_STR( On ) )

	PORT_START("S7")
	PORT_CONFNAME( 0x01, 0x00, "S7 ROM Type")
	PORT_CONFSETTING(    0x00, "27128" )
	PORT_CONFSETTING(    0x01, "27256" )

	PORT_START("S8")
	PORT_CONFNAME( 0x18, 0x00, "S8 Test Mode")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x08, "Remote Test" )
	PORT_CONFSETTING(    0x10, "Standalone Test" )
	PORT_CONFSETTING(    0x18, "Reserved" )

	PORT_START("S9")
	PORT_CONFNAME( 0x03, 0x00, "S9 8274 TxCB")
	PORT_CONFSETTING(    0x00, "DCE-Rxc (J4-11)" )
	PORT_CONFSETTING(    0x01, "DCE-Txc (J4-13)" )
	PORT_CONFSETTING(    0x02, "Tmr3" )

	PORT_START("S10")
	PORT_CONFNAME( 0x01, 0x01, "S10 8274 RxCA")
	PORT_CONFSETTING(    0x00, "DCE (J2-11)" )
	PORT_CONFSETTING(    0x01, "Tmr5" )

	PORT_START("S11")
	PORT_CONFNAME( 0x03, 0x01, "S11 8274 TxCA")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, "DCE (J2-13)" )
	PORT_CONFSETTING(    0x02, "Tmr5" )

	PORT_START("S12")
	PORT_CONFNAME( 0x01, 0x01, "S12 8274 TxDA")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, "V24 (J2)" )

	PORT_START("S13")
	PORT_CONFNAME( 0x01, 0x01, "S13 8274 RxDA")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, "V24 (J2)" )

	PORT_START("S14")
	PORT_CONFNAME( 0x01, 0x01, "S14 8274 TxCA")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )

	PORT_START("S15")
	PORT_CONFNAME( 0x01, 0x00, "S15 Network")
	PORT_CONFSETTING(    0x00, "Server" )
	PORT_CONFSETTING(    0x01, "Client" )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  I80186_INTERFACE( cpu_intf )
//-------------------------------------------------

READ8_MEMBER( compis_state::compis_irq_callback )
{
	return m_osp->inta_r();
}

WRITE_LINE_MEMBER( compis_state::tmr0_w )
{
	m_tmr0 = state;

	m_cassette->output(m_tmr0 ? -1 : 1);

	m_maincpu->tmrin0_w(state);
}

WRITE_LINE_MEMBER( compis_state::tmr1_w )
{
	m_isbx0->mclk_w(state);
	m_isbx1->mclk_w(state);

	m_maincpu->tmrin1_w(state);
}


//-------------------------------------------------
//  I80130_INTERFACE( osp_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( compis_state::tmr2_w )
{
	m_uart->write_rxc(state);
	m_uart->write_txc(state);
}


WRITE_LINE_MEMBER( compis_state::tmr5_w )
{
	m_mpsc->rxca_w(state);
	m_mpsc->txca_w(state);
}

//-------------------------------------------------
//  I8255A interface
//-------------------------------------------------

WRITE_LINE_MEMBER(compis_state::write_centronics_busy)
{
	m_centronics_busy = state;
}

WRITE_LINE_MEMBER(compis_state::write_centronics_select)
{
	m_centronics_select = state;
}

READ8_MEMBER( compis_state::ppi_pb_r )
{
	/*

	    bit     description

	    0       J5-4
	    1       J5-5
	    2       J6-3 Cassette read
	    3       J2-6 DSR / S8-4 Test
	    4       J4-6 DSR / S8-3 Test
	    5       J7-11 Centronics BUSY
	    6       J7-13 Centronics SELECT
	    7       Tmr0

	*/

	UINT8 data = 0;

	/* DIP switch - Test mode */
	data = m_s8->read();

	// cassette
	data |= (m_cassette->input() > 0.0) << 2;

	/* Centronics busy */
	data |= m_centronics_busy << 5;
	data |= m_centronics_select << 6;

	// TMR0
	data |= m_tmr0 << 7;

	return data;
}

WRITE8_MEMBER( compis_state::ppi_pc_w )
{
	/*

	    bit     description

	    0       J5-1
	    1       J5-2
	    2       Select: 1=time measure, DSR from J2/J4 pin 6. 0=read cassette
	    3       Datex: Tristate datex output (low)
	    4       V2-5 Floppy motor on/off
	    5       J7-1 Centronics STROBE
	    6       V2-4 Floppy Soft reset
	    7       V2-3 Floppy Terminal count

	*/

	m_isbx0->opt1_w(BIT(data, 4));

	m_centronics->write_strobe(BIT(data, 5));

	if (BIT(data, 6))
	{
		m_isbx0->reset();
	}

	m_isbx0->opt0_w(BIT(data, 7));
}

TIMER_DEVICE_CALLBACK_MEMBER( compis_state::tape_tick )
{
	m_maincpu->tmrin0_w(m_cassette->input() > 0.0);
}

//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  machine_start
//-------------------------------------------------

void compis_state::machine_start()
{
	if (m_ram->size() == 256*1024)
	{
		m_maincpu->space(AS_PROGRAM).install_ram(0x20000, 0x3ffff, nullptr);
	}
}


//-------------------------------------------------
//  machine_reset
//-------------------------------------------------

void compis_state::machine_reset()
{
	m_uart->reset();
	m_mpsc->reset();
	m_ppi->reset();
	m_graphics->reset();
	m_isbx0->reset();
	m_isbx1->reset();
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( compis )
//-------------------------------------------------

static MACHINE_CONFIG_START( compis, compis_state )
	// basic machine hardware
	MCFG_CPU_ADD(I80186_TAG, I80186, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(compis_mem)
	MCFG_CPU_IO_MAP(compis_io)
	MCFG_80186_IRQ_SLAVE_ACK(DEVREAD8(DEVICE_SELF, compis_state, compis_irq_callback))
	MCFG_80186_TMROUT0_HANDLER(DEVWRITELINE(DEVICE_SELF, compis_state, tmr0_w))
	MCFG_80186_TMROUT1_HANDLER(DEVWRITELINE(DEVICE_SELF, compis_state, tmr1_w))

	// devices
	MCFG_DEVICE_ADD(I80130_TAG, I80130, XTAL_16MHz/2)
	MCFG_I80130_IRQ_CALLBACK(DEVWRITELINE(I80186_TAG, i80186_cpu_device, int0_w))
	MCFG_I80130_SYSTICK_CALLBACK(DEVWRITELINE(I80130_TAG, i80130_device, ir3_w))
	MCFG_I80130_DELAY_CALLBACK(DEVWRITELINE(I80130_TAG, i80130_device, ir7_w))
	MCFG_I80130_BAUD_CALLBACK(WRITELINE(compis_state, tmr2_w))

	MCFG_DEVICE_ADD(I8253_TAG, PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_16MHz/8)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE(I8274_TAG, i8274_device, rxtxcb_w))
	MCFG_PIT8253_CLK1(XTAL_16MHz/8)
	MCFG_PIT8253_CLK2(XTAL_16MHz/8)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(compis_state, tmr5_w))

	MCFG_DEVICE_ADD(I8255_TAG, I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_I8255_IN_PORTB_CB(READ8(compis_state, ppi_pb_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(compis_state, ppi_pc_w))

	MCFG_DEVICE_ADD(I8251A_TAG, I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE(COMPIS_KEYBOARD_TAG, compis_keyboard_device, si_w))
	MCFG_I8251_RXRDY_HANDLER(DEVWRITELINE(I80130_TAG, i80130_device, ir2_w))
	MCFG_I8251_TXRDY_HANDLER(DEVWRITELINE(I80186_TAG, i80186_cpu_device, int1_w))

	MCFG_DEVICE_ADD(COMPIS_KEYBOARD_TAG, COMPIS_KEYBOARD, 0)
	MCFG_COMPIS_KEYBOARD_OUT_TX_HANDLER(DEVWRITELINE(I8251A_TAG, i8251_device, write_rxd))

	MCFG_I8274_ADD(I8274_TAG, XTAL_16MHz/4, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_INT_CB(DEVWRITELINE(I80186_TAG, i80186_cpu_device, int3_w))

	MCFG_DEVICE_ADD(MM58174A_TAG, MM58274C, 0)
	MCFG_MM58274C_MODE24(0) // 12 hour
	MCFG_MM58274C_DAY1(1)   // monday

	MCFG_CASSETTE_ADD(CASSETTE_TAG)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("tape", compis_state, tape_tick, attotime::from_hz(44100))

	MCFG_RS232_PORT_ADD(RS232_A_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(I8274_TAG, z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(I8274_TAG, z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(I8274_TAG, z80dart_device, ctsa_w))

	MCFG_RS232_PORT_ADD(RS232_B_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(I8274_TAG, z80dart_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(I8274_TAG, z80dart_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(I8274_TAG, z80dart_device, ctsb_w))

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(compis_state, write_centronics_busy))
	MCFG_CENTRONICS_SELECT_HANDLER(WRITELINE(compis_state, write_centronics_select))
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	MCFG_COMPIS_GRAPHICS_SLOT_ADD(GRAPHICS_TAG, XTAL_16MHz, compis_graphics_cards, "hrg")

	MCFG_ISBX_SLOT_ADD(ISBX_0_TAG, 0, isbx_cards, "fdc")
	MCFG_ISBX_SLOT_MINTR0_CALLBACK(DEVWRITELINE(I80130_TAG, i80130_device, ir1_w))
	MCFG_ISBX_SLOT_MINTR1_CALLBACK(DEVWRITELINE(I80130_TAG, i80130_device, ir0_w))
	MCFG_ISBX_SLOT_MDRQT_CALLBACK(DEVWRITELINE(I80186_TAG, i80186_cpu_device, drq0_w))
	MCFG_ISBX_SLOT_ADD(ISBX_1_TAG, 0, isbx_cards, nullptr)
	MCFG_ISBX_SLOT_MINTR0_CALLBACK(DEVWRITELINE(I80130_TAG, i80130_device, ir6_w))
	MCFG_ISBX_SLOT_MINTR1_CALLBACK(DEVWRITELINE(I80130_TAG, i80130_device, ir5_w))
	MCFG_ISBX_SLOT_MDRQT_CALLBACK(DEVWRITELINE(I80186_TAG, i80186_cpu_device, drq1_w))

	// software lists
	MCFG_SOFTWARE_LIST_ADD("flop_list", "compis")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_EXTRA_OPTIONS("256K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( compis2 )
//-------------------------------------------------

static MACHINE_CONFIG_DERIVED( compis2, compis )
	// basic machine hardware
	MCFG_CPU_MODIFY(I80186_TAG)
	MCFG_CPU_PROGRAM_MAP(compis2_mem)
	// TODO 8087

	// devices
	// TODO 525hd drives

	// internal ram
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("768K")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( compis )
//-------------------------------------------------

ROM_START( compis )
	ROM_REGION16_LE( 0x10000, I80186_TAG, 0 )
	ROM_DEFAULT_BIOS( "v303" )

	ROM_SYSTEM_BIOS( 0, "v20", "Compis v2.0 (1985-05-15)" )
	ROMX_LOAD( "sa883003.u40", 0x0000, 0x4000, CRC(195ef6bf) SHA1(eaf8ae897e1a4b62d3038ff23777ce8741b766ef), ROM_BIOS(1) | ROM_SKIP(1) )
	ROMX_LOAD( "sa883003.u36", 0x0001, 0x4000, CRC(7c918f56) SHA1(8ba33d206351c52f44f1aa76cc4d7f292dcef761), ROM_BIOS(1) | ROM_SKIP(1) )
	ROMX_LOAD( "sa883003.u39", 0x8000, 0x4000, CRC(3cca66db) SHA1(cac36c9caa2f5bb42d7a6d5b84f419318628935f), ROM_BIOS(1) | ROM_SKIP(1) )
	ROMX_LOAD( "sa883003.u35", 0x8001, 0x4000, CRC(43c38e76) SHA1(f32e43604107def2c2259898926d090f2ed62104), ROM_BIOS(1) | ROM_SKIP(1) )

	ROM_SYSTEM_BIOS( 1, "v302", "Compis II v3.02 (1986-09-09)" )
	ROMX_LOAD( "comp302.u39", 0x0000, 0x8000, CRC(16a7651e) SHA1(4cbd4ba6c6c915c04dfc913ec49f87c1dd7344e3), ROM_BIOS(2) | ROM_SKIP(1) )
	ROMX_LOAD( "comp302.u35", 0x0001, 0x8000, CRC(ae546bef) SHA1(572e45030de552bb1949a7facbc885b8bf033fc6), ROM_BIOS(2) | ROM_SKIP(1) )

	ROM_SYSTEM_BIOS( 2, "v303", "Compis II v3.03 (1987-03-09)" )
	ROMX_LOAD( "rysa094.u39", 0x0000, 0x8000, CRC(e7302bff) SHA1(44ea20ef4008849af036c1a945bc4f27431048fb), ROM_BIOS(3) | ROM_SKIP(1) )
	ROMX_LOAD( "rysa094.u35", 0x0001, 0x8000, CRC(b0694026) SHA1(eb6b2e3cb0f42fd5ffdf44f70e652ecb9714ce30), ROM_BIOS(3) | ROM_SKIP(1) )
ROM_END

#define rom_compis2 rom_compis



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   INIT                         COMPANY             FULLNAME        FLAGS
COMP(1985,  compis,     0,      0,     compis,  compis, driver_device, 0, "Telenova", "Compis" , MACHINE_NOT_WORKING )
COMP(1986,  compis2,    compis, 0,     compis2, compis, driver_device, 0, "Telenova", "Compis II" , MACHINE_NOT_WORKING )
