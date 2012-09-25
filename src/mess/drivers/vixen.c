/*

Osborne 4 Vixen

Main PCB Layout
---------------

TODO

Notes:
    Relevant IC's shown.

    CPU     - Zilog Z8400APS Z80A CPU
    FDC     - SMC FDC1797
    8155    - Intel P8155H
    ROM0    -
    ROM1,2  - AMD AM2732-1DC 4Kx8 EPROM
    CN1     - keyboard connector
    CN2     -
    CN3     -
    CN4     - floppy connector
    CN5     - power connector
    CN6     - composite video connector
    SW1     - reset switch
    SW2     -


I/O PCB Layout
--------------

TODO

Notes:
    Relevant IC's shown.

    8155    - Intel P8155H
    8251    - AMD P8251A
    CN1     - IEEE488 connector
    CN2     - RS232 connector
    CN3     -

*/

/*

    TODO:

    - video line buffer
    - floppy
    - keyboard
    - RS232 RI interrupt
    - PCB layouts

*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/flopdrv.h"
#include "machine/ram.h"
#include "machine/i8155.h"
#include "machine/ieee488.h"
#include "machine/i8251.h"
#include "machine/wd17xx.h"
#include "sound/discrete.h"
#include "includes/vixen.h"



//**************************************************************************
//  INTERRUPTS
//**************************************************************************

//-------------------------------------------------
//  update_interrupt -
//-------------------------------------------------

void vixen_state::update_interrupt()
{
	int state = (m_cmd_d1 && m_fdint) || m_vsync || (!m_enb_srq_int && !m_srq) || (!m_enb_atn_int && !m_atn) || (!m_enb_xmt_int && m_txrdy) || (!m_enb_rcv_int && m_rxrdy);

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  ctl_w - command write
//-------------------------------------------------

WRITE8_MEMBER( vixen_state::ctl_w )
{
	logerror("CTL %u\n", data);

	membank("bank3")->set_entry(BIT(data, 0));
}


//-------------------------------------------------
//  status_r - status read
//-------------------------------------------------

READ8_MEMBER( vixen_state::status_r )
{
	/*

        bit     description

        0       VSYNC enable
        1       FDINT enable
        2       VSYNC
        3       1
        4       1
        5       1
        6       1
        7       1

    */

	UINT8 data = 0xf8;

	// vertical sync interrupt enable
	data |= m_cmd_d0;

	// floppy interrupt enable
	data |= m_cmd_d1 << 1;

	// vertical sync
	data |= m_vsync << 2;

	return data;
}


//-------------------------------------------------
//  cmd_w - command write
//-------------------------------------------------

WRITE8_MEMBER( vixen_state::cmd_w )
{
	/*

        bit     description

        0       VSYNC enable
        1       FDINT enable
        2
        3
        4
        5
        6
        7

    */

//  logerror("CMD %u\n", data);

	// vertical sync interrupt enable
	m_cmd_d0 = BIT(data, 0);

	if (!m_cmd_d0)
	{
		// clear vertical sync
		m_vsync = 0;
	}

	// floppy interrupt enable
	m_cmd_d1 = BIT(data, 1);

	update_interrupt();
}


//-------------------------------------------------
//  ieee488_r - IEEE488 bus read
//-------------------------------------------------

READ8_MEMBER( vixen_state::ieee488_r )
{
	/*

        bit     description

        0       ATN
        1       DAV
        2       NDAC
        3       NRFD
        4       EOI
        5       SRQ
        6       IFC
        7       REN

    */

	UINT8 data = 0;

	/* attention */
	data |= m_ieee488->atn_r();

	/* data valid */
	data |= m_ieee488->dav_r() << 1;

	/* data not accepted */
	data |= m_ieee488->ndac_r() << 2;

	/* not ready for data */
	data |= m_ieee488->nrfd_r() << 3;

	/* end or identify */
	data |= m_ieee488->eoi_r() << 4;

	/* service request */
	data |= m_ieee488->srq_r() << 5;

	/* interface clear */
	data |= m_ieee488->ifc_r() << 6;

	/* remote enable */
	data |= m_ieee488->ren_r() << 7;

	return data;
}


//-------------------------------------------------
//  port3_r - serial status read
//-------------------------------------------------

READ8_MEMBER( vixen_state::port3_r )
{
	/*

        bit     description

        0       RI
        1       DCD
        2       1
        3       1
        4       1
        5       1
        6       1
        7       1

    */

	UINT8 data = 0xff; //0xfc;

	// TODO ring indicator
	//data |= rs232_ri_r(m_rs232);

	// TODO data carrier detect
	//data |= rs232_dcd_r(m_rs232) << 1;

	return data;
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( vixen_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( vixen_mem, AS_PROGRAM, 8, vixen_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xefff) AM_READ_BANK("bank1") AM_WRITE_BANK("bank2")
	AM_RANGE(0xf000, 0xffff) AM_READ_BANK("bank3") AM_WRITE_BANK("bank4") AM_SHARE("video_ram")
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vixen_io )
//-------------------------------------------------

static ADDRESS_MAP_START( vixen_io, AS_IO, 8, vixen_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE_LEGACY(FDC1797_TAG, wd17xx_r, wd17xx_w)
	AM_RANGE(0x04, 0x04) AM_MIRROR(0x03) AM_READWRITE(status_r, cmd_w)
	AM_RANGE(0x08, 0x08) AM_MIRROR(0x01) AM_DEVREADWRITE(P8155H_TAG, i8155_device, read, write)
	AM_RANGE(0x0c, 0x0d) AM_DEVWRITE(P8155H_TAG, i8155_device, ale_w)
	AM_RANGE(0x10, 0x10) AM_MIRROR(0x07) AM_DEVREAD(IEEE488_TAG, ieee488_device, dio_r)
	AM_RANGE(0x18, 0x18) AM_MIRROR(0x07) AM_READ(ieee488_r)
	AM_RANGE(0x20, 0x21) AM_MIRROR(0x04) AM_DEVWRITE(P8155H_IO_TAG, i8155_device, ale_w)
	AM_RANGE(0x28, 0x28) AM_MIRROR(0x05) AM_DEVREADWRITE(P8155H_IO_TAG, i8155_device, read, write)
	AM_RANGE(0x30, 0x30) AM_MIRROR(0x06) AM_DEVREADWRITE(P8251A_TAG, i8251_device, data_r, data_w)
	AM_RANGE(0x31, 0x31) AM_MIRROR(0x06) AM_DEVREADWRITE(P8251A_TAG, i8251_device, status_r, control_w)
	AM_RANGE(0x38, 0x38) AM_MIRROR(0x07) AM_READ(port3_r)
//  AM_RANGE(0xf0, 0xff) Hard Disk?
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( vixen )
//-------------------------------------------------

INPUT_PORTS_START( vixen )
	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("ROW4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("ROW5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("ROW6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("ROW7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
INPUT_PORTS_END



//**************************************************************************
//  VIDEO
//**************************************************************************

//-------------------------------------------------
//  TIMER_DEVICE_CALLBACK( vsync_tick )
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER(vixen_state::vsync_tick)
{
	if (m_cmd_d0)
	{
		m_vsync = 1;
		update_interrupt();
	}
}


//-------------------------------------------------
//  VIDEO_START( vixen )
//-------------------------------------------------

void vixen_state::video_start()
{
	// find memory regions
	m_sync_rom = memregion("video")->base();
	m_char_rom = memregion("chargen")->base();

	// register for state saving
	save_item(NAME(m_alt));
	save_item(NAME(m_256));
	save_item(NAME(m_vsync));
	save_pointer(NAME(m_video_ram.target()), 0x1000);
}


//-------------------------------------------------
//  SCREEN_UPDATE_IND16( vixen )
//-------------------------------------------------

UINT32 vixen_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int txadr = 0; txadr < 26; txadr++)
	{
		for (int scan = 0; scan < 10; scan++)
		{
			for (int chadr = 0; chadr < 128; chadr++)
			{
				UINT16 sync_addr = (txadr << 7) | chadr;
				UINT8 sync_data = m_sync_rom[sync_addr];
				int blank = BIT(sync_data, 4);
				/*
                int clrchadr = BIT(sync_data, 7);
                int hsync = BIT(sync_data, 6);
                int clrtxadr = BIT(sync_data, 5);
                int vsync = BIT(sync_data, 3);
                int comp_sync = BIT(sync_data, 2);

                logerror("SYNC %03x:%02x TXADR %u SCAN %u CHADR %u : COMPSYNC %u VSYNC %u BLANK %u CLRTXADR %u HSYNC %u CLRCHADR %u\n",
                    sync_addr,sync_data,txadr,scan,chadr,comp_sync,vsync,blank,clrtxadr,hsync,clrchadr);
                */

				int reverse = 0;

				UINT16 video_addr = (txadr << 7) | chadr;
				UINT8 video_data = m_video_ram[video_addr];
				UINT16 char_addr = 0;

				if (m_256)
				{
					char_addr = (BIT(video_data, 7) << 11) | (scan << 7) | (video_data & 0x7f);
					reverse = m_alt;
				}
				else
				{
					char_addr = (scan << 7) | (video_data & 0x7f);
					reverse = BIT(video_data, 7);
				}

				UINT8 char_data = m_char_rom[char_addr];

				for (int x = 0; x < 8; x++)
				{
					int color = (BIT(char_data, 7 - x) ^ reverse) & !blank;

					bitmap.pix16((txadr * 10) + scan, (chadr * 8) + x) = color;
				}
			}
		}
	}

	return 0;
}



//**************************************************************************
//  SOUND
//**************************************************************************

//-------------------------------------------------
//  DISCRETE_SOUND( vixen )
//-------------------------------------------------

static DISCRETE_SOUND_START( vixen )
	DISCRETE_INPUT_LOGIC(NODE_01)
	DISCRETE_SQUAREWAVE(NODE_02, NODE_01, XTAL_23_9616MHz/15360, 100, 50, 0, 90)
	DISCRETE_OUTPUT(NODE_02, 2000)
DISCRETE_SOUND_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  I8155_INTERFACE( i8155_intf )
//-------------------------------------------------

READ8_MEMBER( vixen_state::i8155_pa_r )
{
	UINT8 data = 0xff;

	if (!BIT(m_col, 0)) data &= ioport("ROW0")->read();
	if (!BIT(m_col, 1)) data &= ioport("ROW1")->read();
	if (!BIT(m_col, 2)) data &= ioport("ROW2")->read();
	if (!BIT(m_col, 3)) data &= ioport("ROW3")->read();
	if (!BIT(m_col, 4)) data &= ioport("ROW4")->read();
	if (!BIT(m_col, 5)) data &= ioport("ROW5")->read();
	if (!BIT(m_col, 6)) data &= ioport("ROW6")->read();
	if (!BIT(m_col, 7)) data &= ioport("ROW7")->read();

	return data;
}

WRITE8_MEMBER( vixen_state::i8155_pb_w )
{
	m_col = data;
}

WRITE8_MEMBER( vixen_state::i8155_pc_w )
{
	/*

        bit     description

        0       DSEL1
        1       DSEL2
        2       DDEN
        3       ALT CHARSET
        4       256 CHARS
        5       BEEP ENB
        6
        7

    */

	// drive select
	if (!BIT(data, 0)) wd17xx_set_drive(m_fdc, 0);
	if (!BIT(data, 1)) wd17xx_set_drive(m_fdc, 1);

	// density select
	wd17xx_dden_w(m_fdc, BIT(data, 2));

	// charset
	m_alt = BIT(data, 3);
	m_256 = BIT(data, 4);

	// beep enable
	discrete_sound_w(m_discrete, space, NODE_01, BIT(data, 5));
}

static I8155_INTERFACE( i8155_intf )
{
	DEVCB_DRIVER_MEMBER(vixen_state, i8155_pa_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(vixen_state, i8155_pb_w),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(vixen_state, i8155_pc_w),
	DEVCB_NULL
};


//-------------------------------------------------
//  I8155_INTERFACE( io_i8155_intf )
//-------------------------------------------------

WRITE8_MEMBER( vixen_state::io_i8155_pb_w )
{
	/*

        bit     description

        PB0     ATN
        PB1     DAV
        PB2     NDAC
        PB3     NRFD
        PB4     EOI
        PB5     SRQ
        PB6     IFC
        PB7     REN

    */

	/* data valid */
	m_ieee488->atn_w(BIT(data, 0));

	/* end or identify */
	m_ieee488->dav_w(BIT(data, 1));

	/* remote enable */
	m_ieee488->ndac_w(BIT(data, 2));

	/* attention */
	m_ieee488->nrfd_w(BIT(data, 3));

	/* interface clear */
	m_ieee488->eoi_w(BIT(data, 4));

	/* service request */
	m_ieee488->srq_w(BIT(data, 5));

	/* not ready for data */
	m_ieee488->ifc_w(BIT(data, 6));

	/* data not accepted */
	m_ieee488->ren_w(BIT(data, 7));
}

WRITE8_MEMBER( vixen_state::io_i8155_pc_w )
{
	/*

        bit     description

        PC0     select internal clock
        PC1     ENB RING INT
        PC2     ENB RCV INT
        PC3     ENB XMT INT
        PC4     ENB ATN INT
        PC5     ENB SRQ INT
        PC6
        PC7

    */

	m_int_clk = BIT(data, 0);
	m_enb_ring_int = BIT(data, 1);
	m_enb_rcv_int = BIT(data, 2);
	m_enb_xmt_int = BIT(data, 3);
	m_enb_atn_int = BIT(data, 4);
	m_enb_srq_int = BIT(data, 5);
}

WRITE_LINE_MEMBER( vixen_state::io_i8155_to_w )
{
	if (m_int_clk && !state)
	{
		m_usart->transmit_clock();
		m_usart->receive_clock();
	}
}

static I8155_INTERFACE( io_i8155_intf )
{
    DEVCB_NULL,
	DEVCB_DEVICE_MEMBER(IEEE488_TAG, ieee488_device, dio_w),
    DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(vixen_state, io_i8155_pb_w),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(vixen_state, io_i8155_pc_w),
	DEVCB_DRIVER_LINE_MEMBER(vixen_state, io_i8155_to_w)
};


//-------------------------------------------------
//  i8251_interface usart_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( vixen_state::rxrdy_w )
{
	m_rxrdy = state;
	update_interrupt();
}

WRITE_LINE_MEMBER( vixen_state::txrdy_w )
{
	m_txrdy = state;
	update_interrupt();
}

static const i8251_interface usart_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(vixen_state, rxrdy_w),
	DEVCB_DRIVER_LINE_MEMBER(vixen_state, txrdy_w),
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  IEEE488_INTERFACE( ieee488_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( vixen_state::srq_w )
{
	m_srq = state;
	update_interrupt();
}

WRITE_LINE_MEMBER( vixen_state::atn_w )
{
	m_atn = state;
	update_interrupt();
}

static IEEE488_INTERFACE( ieee488_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(vixen_state, srq_w),
	DEVCB_DRIVER_LINE_MEMBER(vixen_state, atn_w),
	DEVCB_NULL
};


//-------------------------------------------------
//  wd17xx_interface fdc_intf
//-------------------------------------------------

static const floppy_interface vixen_floppy_interface =
{
    DEVCB_NULL,
	DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    FLOPPY_STANDARD_5_25_SSDD_40,
    LEGACY_FLOPPY_OPTIONS_NAME(default),
    "floppy_5_25",
	NULL
};

WRITE_LINE_MEMBER( vixen_state::fdint_w )
{
	m_fdint = state;
	update_interrupt();
}

static const wd17xx_interface fdc_intf =
{
	DEVCB_NULL,
    DEVCB_DRIVER_LINE_MEMBER(vixen_state, fdint_w),
	DEVCB_NULL,
	{ FLOPPY_0, FLOPPY_1, NULL, NULL }
};



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  IRQ_CALLBACK( vixen )
//-------------------------------------------------

static IRQ_CALLBACK( vixen_int_ack )
{
	// D0 is pulled low
	return 0xfe;
}


//-------------------------------------------------
//  MACHINE_START( vixen )
//-------------------------------------------------

void vixen_state::machine_start()
{
	// interrupt callback
	m_maincpu->set_irq_acknowledge_callback(vixen_int_ack);

	// configure memory banking
	UINT8 *ram = m_ram->pointer();

	membank("bank1")->configure_entry(0, ram);
	membank("bank1")->configure_entry(1, memregion(Z8400A_TAG)->base());

	membank("bank2")->configure_entry(0, ram);
	membank("bank2")->configure_entry(1, m_video_ram);

	membank("bank3")->configure_entry(0, m_video_ram);
	membank("bank3")->configure_entry(1, memregion(Z8400A_TAG)->base());

	membank("bank4")->configure_entry(0, m_video_ram);

	// register for state saving
	save_item(NAME(m_reset));
	save_item(NAME(m_col));
	save_item(NAME(m_cmd_d0));
	save_item(NAME(m_cmd_d1));
	save_item(NAME(m_fdint));
}


//-------------------------------------------------
//  MACHINE_RESET( vixen )
//-------------------------------------------------

void vixen_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	program.install_read_bank(0x0000, 0xefff, 0xfff, 0, "bank1");
	program.install_write_bank(0x0000, 0xefff, 0xfff, 0, "bank2");

	membank("bank1")->set_entry(1);
	membank("bank2")->set_entry(1);
	membank("bank3")->set_entry(1);

	m_reset = 1;

	m_vsync = 0;
	m_cmd_d0 = 0;
	m_cmd_d1 = 0;
	update_interrupt();
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( vixen )
//-------------------------------------------------

static MACHINE_CONFIG_START( vixen, vixen_state )
    // basic machine hardware
    MCFG_CPU_ADD(Z8400A_TAG, Z80, XTAL_23_9616MHz/6)
    MCFG_CPU_PROGRAM_MAP(vixen_mem)
    MCFG_CPU_IO_MAP(vixen_io)

    // video hardware
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(vixen_state, screen_update)
	MCFG_SCREEN_RAW_PARAMS(XTAL_23_9616MHz/2, 96*8, 0*8, 81*8, 27*10, 0*10, 26*10)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("vsync", vixen_state, vsync_tick, SCREEN_TAG, 26*10, 27*10)

	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(monochrome_amber)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(DISCRETE_TAG, DISCRETE, 0)
	MCFG_SOUND_CONFIG_DISCRETE(vixen)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	// devices
	MCFG_I8155_ADD(P8155H_TAG, XTAL_23_9616MHz/6, i8155_intf)
	MCFG_I8155_ADD(P8155H_IO_TAG, XTAL_23_9616MHz/6, io_i8155_intf)
	MCFG_I8251_ADD(P8251A_TAG, usart_intf)
	MCFG_FD1797_ADD(FDC1797_TAG, fdc_intf)
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(vixen_floppy_interface)
	MCFG_IEEE488_BUS_ADD(ieee488_intf)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list","vixen")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( vixen )
//-------------------------------------------------

ROM_START( vixen )
    ROM_REGION( 0x1000, Z8400A_TAG, 0 )
	ROM_LOAD( "osborne 4 mon rom v1.04 3p40082-03 a0a9.4c", 0x0000, 0x1000, CRC(5f1038ce) SHA1(e6809fac23650bbb4689e58edc768d917d80a2df) ) // OSBORNE 4 MON ROM / V1.04  3P40082-03 / A0A9 (c) OCC 1985

    ROM_REGION( 0x1000, "video", 0 )
	ROM_LOAD( "v1.10.3j", 0x0000, 0x1000, CRC(1f93e2d7) SHA1(0c479bfd3ac8d9959c285c020d0096930a9c6867) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "v1.00 l.1j", 0x0000, 0x1000, CRC(f97c50d9) SHA1(39f73afad68508c4b8a4d241c064f9978098d8f2) )
ROM_END



//**************************************************************************
//  DRIVER INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  DRIVER_INIT( vixen )
//-------------------------------------------------

DIRECT_UPDATE_MEMBER(vixen_state::vixen_direct_update_handler)
{
	if (address >= 0xf000)
	{
		if (m_reset)
		{
			address_space &program = m_maincpu->space(AS_PROGRAM);

			program.install_read_bank(0x0000, 0xefff, "bank1");
			program.install_write_bank(0x0000, 0xefff, "bank2");

			membank("bank1")->set_entry(0);
			membank("bank2")->set_entry(0);

			m_reset = 0;
		}

		direct.explicit_configure(0xf000, 0xffff, 0xfff, machine().root_device().memregion(Z8400A_TAG)->base());

		return ~0;
	}

	return address;
}

DRIVER_INIT_MEMBER(vixen_state,vixen)
{
	address_space &program = machine().device<cpu_device>(Z8400A_TAG)->space(AS_PROGRAM);
	program.set_direct_update_handler(direct_update_delegate(FUNC(vixen_state::vixen_direct_update_handler), this));
}



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS
COMP( 1984, vixen,  0,       0, 	vixen,	vixen, vixen_state,	 vixen,  "Osborne",   "Vixen",		GAME_NOT_WORKING )
