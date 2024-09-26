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

******************************************************************************/

/*

    TODO:

    - booting system2 with disk in drive b: fails
    - uhrg graphics are drawn wrong (upd7220 bugs?)
    - compis2
        - color graphics
        - 8087
        - programmable keyboard
    - hard disk

*/

#include "emu.h"
#include "bus/centronics/ctronics.h"
#include "bus/compis/graphics.h"
#include "bus/isbx/isbx.h"
#include "bus/rs232/rs232.h"
#include "cpu/i86/i186.h"
#include "cpu/mcs48/mcs48.h"
#include "imagedev/cassette.h"
#include "compiskb.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/i80130.h"
#include "machine/mm58174.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "machine/z80sio.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

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
#define GRAPHICS_TAG    "gfx"
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
	required_device<mm58174_device> m_rtc;
	required_device<cassette_image_device> m_cassette;
	required_device<compis_graphics_slot_device> m_graphics;
	required_device<isbx_slot_device> m_isbx0;
	required_device<isbx_slot_device> m_isbx1;
	required_device<ram_device> m_ram;
	required_ioport m_s8;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint16_t pcs6_0_1_r(offs_t offset, uint16_t mem_mask = ~0);
	void pcs6_0_1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pcs6_2_3_r(offs_t offset, uint16_t mem_mask = ~0);
	void pcs6_2_3_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pcs6_4_5_r(offs_t offset, uint16_t mem_mask = ~0);
	void pcs6_4_5_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pcs6_6_7_r(offs_t offset, uint16_t mem_mask = ~0);
	void pcs6_6_7_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pcs6_8_9_r(offs_t offset, uint16_t mem_mask = ~0);
	void pcs6_8_9_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pcs6_10_11_r(offs_t offset, uint16_t mem_mask = ~0);
	void pcs6_10_11_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pcs6_12_13_r(offs_t offset, uint16_t mem_mask = ~0);
	void pcs6_12_13_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pcs6_14_15_r(offs_t offset, uint16_t mem_mask = ~0);
	void pcs6_14_15_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint8_t compis_irq_callback();

	uint8_t ppi_pb_r();
	void ppi_pc_w(uint8_t data);

	void tmr0_w(int state);
	void tmr1_w(int state);
	void tmr2_w(int state);
	void tmr5_w(int state);

	TIMER_DEVICE_CALLBACK_MEMBER( tape_tick );

	int m_centronics_busy;
	int m_centronics_select;

	void write_centronics_busy(int state);
	void write_centronics_select(int state);

	int m_tmr0;
	void compis(machine_config &config);
	void compis2(machine_config &config);
	void compis2_mem(address_map &map) ATTR_COLD;
	void compis_io(address_map &map) ATTR_COLD;
	void compis_mem(address_map &map) ATTR_COLD;
};



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

uint16_t compis_state::pcs6_0_1_r(offs_t offset, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		return 0xff;
	}
	else
	{
		return m_graphics->dma_ack_r(offset);
	}
}

void compis_state::pcs6_0_1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		cassette_state state = BIT(data, 0) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED;

		m_cassette->change_state(state, CASSETTE_MASK_MOTOR);
	}
	else
	{
		m_graphics->dma_ack_w(offset, data);
	}
}

uint16_t compis_state::pcs6_2_3_r(offs_t offset, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		return m_mpsc->inta_r();
	}
	else
	{
		return m_uart->read(offset & 1) << 8;
	}
}

void compis_state::pcs6_2_3_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
	}
	else
	{
		m_uart->write(offset & 1, data >> 8);
	}
}

uint16_t compis_state::pcs6_4_5_r(offs_t offset, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		return m_mpsc->cd_ba_r(offset & 0x03);
	}
	else
	{
		m_isbx0->tdma_w(0);
		m_isbx0->tdma_w(1);

		return 0xff;
	}
}

void compis_state::pcs6_4_5_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_mpsc->cd_ba_w(offset & 0x03, data);
	}
	else
	{
		m_isbx0->tdma_w(0);
		m_isbx0->tdma_w(1);
	}
}

uint16_t compis_state::pcs6_6_7_r(offs_t offset, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		return m_graphics->pcs6_6_r(offset);
	}
	else
	{
		m_isbx1->tdma_w(0);
		m_isbx1->tdma_w(1);

		return 0xff;
	}
}

void compis_state::pcs6_6_7_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_graphics->pcs6_6_w(offset, data);
	}
	else
	{
		m_isbx1->tdma_w(0);
		m_isbx1->tdma_w(1);
	}
}

uint16_t compis_state::pcs6_8_9_r(offs_t offset, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		return m_isbx0->mcs0_r(offset);
	}
	else
	{
		return m_isbx0->mcs1_r(offset) << 8;
	}
}

void compis_state::pcs6_8_9_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_isbx0->mcs0_w(offset, data);
	}
	else
	{
		m_isbx0->mcs1_w(offset, data >> 8);
	}
}

uint16_t compis_state::pcs6_10_11_r(offs_t offset, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		return m_isbx0->mcs1_r(offset);
	}
	else
	{
		return m_isbx0->mdack_r(offset) << 8;
	}
}

void compis_state::pcs6_10_11_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_isbx0->mcs1_w(offset, data);
	}
	else
	{
		m_isbx0->mdack_w(offset, data >> 8);
	}
}

uint16_t compis_state::pcs6_12_13_r(offs_t offset, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		return m_isbx1->mcs0_r(offset);
	}
	else
	{
		return m_isbx1->mcs1_r(offset) << 8;
	}
}

void compis_state::pcs6_12_13_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_isbx1->mcs0_w(offset, data);
	}
	else
	{
		m_isbx1->mcs1_w(offset, data >> 8);
	}
}

uint16_t compis_state::pcs6_14_15_r(offs_t offset, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		return m_isbx1->mcs1_r(offset);
	}
	else
	{
		return m_isbx1->mdack_r(offset) << 8;
	}
}

void compis_state::pcs6_14_15_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_isbx1->mcs1_w(offset, data);
	}
	else
	{
		m_isbx1->mdack_w(offset, data >> 8);
	}
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( compis_mem )
//-------------------------------------------------

void compis_state::compis_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x1ffff).ram();
	map(0x60000, 0x63fff).mirror(0x1c000).m(m_osp, FUNC(i80130_device::rom_map));
	map(0xe0000, 0xeffff).mirror(0x10000).rom().region(I80186_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( compis2_mem )
//-------------------------------------------------

void compis_state::compis2_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x3ffff).ram();
	map(0xe0000, 0xeffff).mirror(0x10000).rom().region(I80186_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( compis_io )
//-------------------------------------------------

void compis_state::compis_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0007) /* PCS0 */ .mirror(0x78).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0xff00);
	map(0x0080, 0x0087) /* PCS1 */ .mirror(0x78).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0x0100, 0x011f) /* PCS2 */ .mirror(0x60).rw(m_rtc, FUNC(mm58174_device::read), FUNC(mm58174_device::write)).umask16(0x00ff);
	map(0x0180, 0x01ff) /* PCS3 */ .rw(m_graphics, FUNC(compis_graphics_slot_device::pcs3_r), FUNC(compis_graphics_slot_device::pcs3_w));
	//map(0x0200, 0x0201) /* PCS4 */ .mirror(0x7e);
	map(0x0280, 0x028f) /* PCS5 */ .mirror(0x70).m(m_osp, FUNC(i80130_device::io_map));
	map(0x0300, 0x030f).rw(FUNC(compis_state::pcs6_0_1_r), FUNC(compis_state::pcs6_0_1_w));
	map(0x0310, 0x031f).rw(FUNC(compis_state::pcs6_2_3_r), FUNC(compis_state::pcs6_2_3_w));
	map(0x0320, 0x032f).rw(FUNC(compis_state::pcs6_4_5_r), FUNC(compis_state::pcs6_4_5_w));
	map(0x0330, 0x033f).rw(FUNC(compis_state::pcs6_6_7_r), FUNC(compis_state::pcs6_6_7_w));
	map(0x0340, 0x034f).rw(FUNC(compis_state::pcs6_8_9_r), FUNC(compis_state::pcs6_8_9_w));
	map(0x0350, 0x035f).rw(FUNC(compis_state::pcs6_10_11_r), FUNC(compis_state::pcs6_10_11_w));
	map(0x0360, 0x036f).rw(FUNC(compis_state::pcs6_12_13_r), FUNC(compis_state::pcs6_12_13_w));
	map(0x0370, 0x037f).rw(FUNC(compis_state::pcs6_14_15_r), FUNC(compis_state::pcs6_14_15_w));
}



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

uint8_t compis_state::compis_irq_callback()
{
	return m_osp->inta_r();
}

void compis_state::tmr0_w(int state)
{
	m_tmr0 = state;

	m_cassette->output(m_tmr0 ? -1 : 1);

	m_maincpu->tmrin0_w(state);
}

void compis_state::tmr1_w(int state)
{
	m_isbx0->mclk_w(state);
	m_isbx1->mclk_w(state);

	m_maincpu->tmrin1_w(state);
}


//-------------------------------------------------
//  I80130_INTERFACE( osp_intf )
//-------------------------------------------------

void compis_state::tmr2_w(int state)
{
	m_uart->write_rxc(state);
	m_uart->write_txc(state);
}


void compis_state::tmr5_w(int state)
{
	m_mpsc->rxca_w(state);
	m_mpsc->txca_w(state);
}

//-------------------------------------------------
//  I8255A interface
//-------------------------------------------------

void compis_state::write_centronics_busy(int state)
{
	m_centronics_busy = state;
}

void compis_state::write_centronics_select(int state)
{
	m_centronics_select = state;
}

uint8_t compis_state::ppi_pb_r()
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

	uint8_t data = 0;

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

void compis_state::ppi_pc_w(uint8_t data)
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
	// RAM size
	switch (m_ram->size())
	{
	case 256*1024:
		m_maincpu->space(AS_PROGRAM).install_ram(0x20000, 0x3ffff, m_ram->pointer());
		break;

	case 512*1024:
		m_maincpu->space(AS_PROGRAM).install_ram(0x20000, 0x7ffff, m_ram->pointer());
		break;

	case 768*1024:
		m_maincpu->space(AS_PROGRAM).install_ram(0x20000, 0xbffff, m_ram->pointer());
		break;
	}

	// state saving
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_select));
	save_item(NAME(m_tmr0));
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
//  machine_config( compis )
//-------------------------------------------------

void compis_state::compis(machine_config &config)
{
	// basic machine hardware
	I80186(config, m_maincpu, 15.36_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &compis_state::compis_mem);
	m_maincpu->set_addrmap(AS_IO, &compis_state::compis_io);
	m_maincpu->read_slave_ack_callback().set(FUNC(compis_state::compis_irq_callback));
	m_maincpu->tmrout0_handler().set(FUNC(compis_state::tmr0_w));
	m_maincpu->tmrout1_handler().set(FUNC(compis_state::tmr1_w));

	// devices
	I80130(config, m_osp, 15.36_MHz_XTAL/2);
	m_osp->irq().set(m_maincpu, FUNC(i80186_cpu_device::int0_w));
	m_osp->systick().set(m_osp, FUNC(i80130_device::ir3_w));
	m_osp->delay().set(m_osp, FUNC(i80130_device::ir7_w));
	m_osp->baud().set(FUNC(compis_state::tmr2_w));

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(15.36_MHz_XTAL/8);
	m_pit->out_handler<0>().set(m_mpsc, FUNC(i8274_device::rxtxcb_w));
	m_pit->set_clk<1>(15.36_MHz_XTAL/8);
	m_pit->set_clk<2>(15.36_MHz_XTAL/8);
	m_pit->out_handler<2>().set(FUNC(compis_state::tmr5_w));

	I8255(config, m_ppi);
	m_ppi->out_pa_callback().set("cent_data_out", FUNC(output_latch_device::write));
	m_ppi->in_pb_callback().set(FUNC(compis_state::ppi_pb_r));
	m_ppi->out_pc_callback().set(FUNC(compis_state::ppi_pc_w));

	I8251(config, m_uart, 0);
	m_uart->txd_handler().set(COMPIS_KEYBOARD_TAG, FUNC(compis_keyboard_device::si_w));
	m_uart->rxrdy_handler().set(m_osp, FUNC(i80130_device::ir2_w));
	m_uart->txrdy_handler().set(m_maincpu, FUNC(i80186_cpu_device::int1_w));

	compis_keyboard_device &kb(COMPIS_KEYBOARD(config, COMPIS_KEYBOARD_TAG, 0));
	kb.out_tx_handler().set(m_uart, FUNC(i8251_device::write_rxd));

	I8274(config, m_mpsc, 15.36_MHz_XTAL/4);
	m_mpsc->out_txda_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_txd));
	m_mpsc->out_dtra_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_dtr));
	m_mpsc->out_rtsa_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_rts));
	m_mpsc->out_txdb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	m_mpsc->out_dtrb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_dtr));
	m_mpsc->out_rtsb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_rts));
	m_mpsc->out_int_callback().set(m_maincpu, FUNC(i80186_cpu_device::int3_w));

	MM58174(config, m_rtc, 32.768_kHz_XTAL);

	SPEAKER(config, "cass_snd").front_center();
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "cass_snd", 0.05);

	TIMER(config, "tape").configure_periodic(FUNC(compis_state::tape_tick), attotime::from_hz(44100));

	rs232_port_device &rs232a(RS232_PORT(config, RS232_A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_mpsc, FUNC(i8274_device::rxa_w));
	rs232a.dcd_handler().set(m_mpsc, FUNC(i8274_device::dcda_w));
	rs232a.cts_handler().set(m_mpsc, FUNC(i8274_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_mpsc, FUNC(i8274_device::rxb_w));
	rs232b.dcd_handler().set(m_mpsc, FUNC(i8274_device::dcdb_w));
	rs232b.cts_handler().set(m_mpsc, FUNC(i8274_device::ctsb_w));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(compis_state::write_centronics_busy));
	m_centronics->select_handler().set(FUNC(compis_state::write_centronics_select));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	COMPIS_GRAPHICS_SLOT(config, m_graphics, 15.36_MHz_XTAL/2, compis_graphics_cards, "hrg");

	ISBX_SLOT(config, m_isbx0, 0, isbx_cards, "fdc");
	m_isbx0->mintr0().set(m_osp, FUNC(i80130_device::ir1_w));
	m_isbx0->mintr1().set(m_osp, FUNC(i80130_device::ir0_w));
	m_isbx0->mdrqt().set(m_maincpu, FUNC(i80186_cpu_device::drq0_w));
	ISBX_SLOT(config, m_isbx1, 0, isbx_cards, nullptr);
	m_isbx1->mintr0().set(m_osp, FUNC(i80130_device::ir6_w));
	m_isbx1->mintr1().set(m_osp, FUNC(i80130_device::ir5_w));
	m_isbx1->mdrqt().set(m_maincpu, FUNC(i80186_cpu_device::drq1_w));

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("compis");

	// internal ram
	RAM(config, m_ram).set_default_size("128K").set_extra_options("256K");
}


//-------------------------------------------------
//  machine_config( compis2 )
//-------------------------------------------------

void compis_state::compis2(machine_config &config)
{
	compis(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &compis_state::compis2_mem);
	// TODO 8087

	// internal ram
	m_ram->set_default_size("256K").set_extra_options("512K,768K");
}



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
	ROMX_LOAD( "sa883003.u40", 0x0000, 0x4000, CRC(195ef6bf) SHA1(eaf8ae897e1a4b62d3038ff23777ce8741b766ef), ROM_BIOS(0) | ROM_SKIP(1) )
	ROMX_LOAD( "sa883003.u36", 0x0001, 0x4000, CRC(7c918f56) SHA1(8ba33d206351c52f44f1aa76cc4d7f292dcef761), ROM_BIOS(0) | ROM_SKIP(1) )
	ROMX_LOAD( "sa883003.u39", 0x8000, 0x4000, CRC(3cca66db) SHA1(cac36c9caa2f5bb42d7a6d5b84f419318628935f), ROM_BIOS(0) | ROM_SKIP(1) )
	ROMX_LOAD( "sa883003.u35", 0x8001, 0x4000, CRC(43c38e76) SHA1(f32e43604107def2c2259898926d090f2ed62104), ROM_BIOS(0) | ROM_SKIP(1) )

	ROM_SYSTEM_BIOS( 1, "v302", "Compis II v3.02 (1986-09-09)" )
	ROMX_LOAD( "comp302.u39", 0x0000, 0x8000, CRC(16a7651e) SHA1(4cbd4ba6c6c915c04dfc913ec49f87c1dd7344e3), ROM_BIOS(1) | ROM_SKIP(1) )
	ROMX_LOAD( "comp302.u35", 0x0001, 0x8000, CRC(ae546bef) SHA1(572e45030de552bb1949a7facbc885b8bf033fc6), ROM_BIOS(1) | ROM_SKIP(1) )

	ROM_SYSTEM_BIOS( 2, "v303", "Compis II v3.03 (1987-03-09)" )
	ROMX_LOAD( "rysa094.u39", 0x0000, 0x8000, CRC(e7302bff) SHA1(44ea20ef4008849af036c1a945bc4f27431048fb), ROM_BIOS(2) | ROM_SKIP(1) )
	ROMX_LOAD( "rysa094.u35", 0x0001, 0x8000, CRC(b0694026) SHA1(eb6b2e3cb0f42fd5ffdf44f70e652ecb9714ce30), ROM_BIOS(2) | ROM_SKIP(1) )
ROM_END

#define rom_compis2 rom_compis

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY     FULLNAME     FLAGS
COMP( 1985, compis,  0,      0,      compis,  compis, compis_state, empty_init, "Telenova", "Compis",    MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, compis2, compis, 0,      compis2, compis, compis_state, empty_init, "Telenova", "Compis II", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
