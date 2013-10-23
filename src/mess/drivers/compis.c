/******************************************************************************

    drivers/compis.c
    machine driver

    Per Ola Ingvarsson
    Tomas Karlsson

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

#include "includes/compis.h"

static UPD7220_DISPLAY_PIXELS( hgdc_display_pixels )
{
	compis_state *state = device->machine().driver_data<compis_state>();
	UINT8 i,gfx = state->m_video_ram[address];

	for(i=0; i<8; i++)
		bitmap.pix32(y, x + i) = RGB_MONOCHROME_GREEN_HIGHLIGHT[BIT(gfx,i )];
}

static UPD7220_INTERFACE( hgdc_intf )
{
	hgdc_display_pixels,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


WRITE8_MEMBER( compis_state::tape_mon_w )
{
	cassette_state state = BIT(data, 0) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED;

	m_cassette->change_state(state, CASSETTE_MASK_MOTOR);
}

READ16_MEMBER( compis_state::isbx0_tdma_r )
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

WRITE16_MEMBER( compis_state::isbx0_tdma_w )
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

READ16_MEMBER( compis_state::isbx1_tdma_r )
{
	if (ACCESSING_BITS_0_7)
	{
		if (offset < 2) 
			return m_crtc->read(space, offset & 0x01);
		else
			return 0;
	}
	else
	{
		m_isbx1->tdma_w(0);
		m_isbx1->tdma_w(1);

		return 0xff;
	}
}

WRITE16_MEMBER( compis_state::isbx1_tdma_w )
{
	if (ACCESSING_BITS_0_7)
	{
		if (offset < 2) m_crtc->write(space, offset & 0x01, data);
	}
	else
	{
		m_isbx1->tdma_w(0);
		m_isbx1->tdma_w(1);
	}
}

READ16_MEMBER( compis_state::isbx0_cs_r )
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

WRITE16_MEMBER( compis_state::isbx0_cs_w )
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

READ16_MEMBER( compis_state::isbx0_dack_r )
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

WRITE16_MEMBER( compis_state::isbx0_dack_w )
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

READ16_MEMBER( compis_state::isbx1_cs_r )
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

WRITE16_MEMBER( compis_state::isbx1_cs_w )
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

READ16_MEMBER( compis_state::isbx1_dack_r )
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

WRITE16_MEMBER( compis_state::isbx1_dack_w )
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


READ8_MEMBER( compis_state::vram_r )
{
	return m_video_ram[offset];
}


WRITE8_MEMBER( compis_state::vram_w )
{
	m_video_ram[offset] = data;
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
	AM_RANGE(0x40000, 0x5ffff) AM_RAM AM_READWRITE8(vram_r, vram_w, 0xffff)
	AM_RANGE(0xe8000, 0xeffff) AM_ROM AM_REGION(I80186_TAG, 0)
	AM_RANGE(0xf0000, 0xfffff) AM_ROM AM_REGION(I80186_TAG, 0)
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
	AM_RANGE(0x0280, 0x0283) /* PCS5 */ AM_MIRROR(0x70) AM_DEVREADWRITE8("pic8259_master", pic8259_device, read, write, 0x00ff) /* 80150/80130 */
	AM_RANGE(0x0288, 0x028f) /* PCS5 */ AM_MIRROR(0x70) AM_DEVREADWRITE8("pit8254", pit8254_device, read, write, 0x00ff) /* 80150/80130 */
	AM_RANGE(0x0300, 0x0301) /* PCS6:0 */ AM_MIRROR(0xe) AM_WRITE8(tape_mon_w, 0x00ff)
	AM_RANGE(0x0310, 0x0311) /* PCS6:3 */ AM_MIRROR(0xc) AM_DEVREADWRITE8(I8251A_TAG, i8251_device, data_r, data_w, 0xff00)
	AM_RANGE(0x0312, 0x0313) /* PCS6:3 */ AM_MIRROR(0xc) AM_DEVREADWRITE8(I8251A_TAG, i8251_device, status_r, control_w, 0xff00)
	AM_RANGE(0x0320, 0x032f) AM_READWRITE(isbx0_tdma_r, isbx0_tdma_w)
	AM_RANGE(0x0330, 0x033f) AM_READWRITE(isbx1_tdma_r, isbx1_tdma_w)
	AM_RANGE(0x0340, 0x034f) AM_READWRITE(isbx0_cs_r, isbx0_cs_w)
	AM_RANGE(0x0350, 0x035f) AM_READWRITE(isbx0_dack_r, isbx0_dack_w)
	AM_RANGE(0x0360, 0x036f) AM_READWRITE(isbx1_cs_r, isbx1_cs_w)
	AM_RANGE(0x0370, 0x037f) AM_READWRITE(isbx1_dack_r, isbx1_dack_w)
#ifdef NOT_SUPPORTED_BY_MAME_CORE
	AM_RANGE(0x0300, 0x0301) /* PCS6:1 */ AM_MIRROR(0xe) AM_DEVREADWRITE8("upd7220", upd7220_device, dack_r, dack_w, 0xff00) // DMA-ACK graphics
	AM_RANGE(0x0310, 0x0311) /* PCS6:2 */ AM_MIRROR(0xe) AM_DEVREAD8(I8274_TAG, i8274_device, inta_r, 0x00ff) // 8274 INTERRUPT ACKNOWLEDGE
	AM_RANGE(0x0320, 0x0323) /* PCS6:4 */ AM_MIRROR(0xc) AM_DEVREADWRITE8(I8274_TAG, i8274_device, cd_ba_r, cd_ba_w, 0x00ff)
	AM_RANGE(0x0320, 0x0321) /* PCS6:5 */ AM_MIRROR(0xe) AM_READWRITE8(isbx0_tdma_r, isbx0_tdma_w, 0xff00) // DMA-TERMINATE J8 (iSBX0)
	AM_RANGE(0x0330, 0x0333) /* PCS6:6 */ AM_DEVREADWRITE8("upd7220", upd7220_device, read, write, 0x00ff)
	AM_RANGE(0x0330, 0x0331) /* PCS6:7 */ AM_MIRROR(0xe) AM_READWRITE8(isbx1_tdma_r, isbx1_tdma_w, 0xff00) // DMA-TERMINATE J9 (iSBX1)
	AM_RANGE(0x0340, 0x034f) /* PCS6:8 */ AM_DEVREADWRITE8(ISBX_0_TAG, isbx_slot_device, mcs0_r, mcs0_w, 0x00ff) // 8272 CS0 (8/16-bit) J8 (iSBX0)
  	AM_RANGE(0x0340, 0x034f) /* PCS6:9 */ AM_DEVREADWRITE8(ISBX_0_TAG, isbx_slot_device, mcs1_r, mcs1_w, 0xff00) // CS1 (16-bit) J8 (iSBX0)
	AM_RANGE(0x0350, 0x035f) /* PCS6:10 */ AM_DEVREADWRITE8(ISBX_0_TAG, isbx_slot_device, mcs1_r, mcs1_w, 0x00ff) // CS1 (8-bit) J8 (iSBX0)
	AM_RANGE(0x0350, 0x035f) /* PCS6:11 */ AM_DEVREADWRITE8(ISBX_0_TAG, isbx_slot_device, mdack_r, mdack_w, 0xff00) // DMA-ACK J8 (iSBX0)
	AM_RANGE(0x0360, 0x036f) /* PCS6:13 */ AM_DEVREADWRITE8(ISBX_1_TAG, isbx_slot_device, mcs0_r, mcs0_w, 0x00ff) // CS0 (8/16-bit) J9 (iSBX1)
	AM_RANGE(0x0360, 0x036f) /* PCS6:13 */ AM_DEVREADWRITE8(ISBX_1_TAG, isbx_slot_device, mcs1_r, mcs1_w, 0xff00) // CS1 (16-bit) J9 (iSBX1)
	AM_RANGE(0x0370, 0x037f) /* PCS6:14 */ AM_DEVREADWRITE8(ISBX_1_TAG, isbx_slot_device, mcs1_r, mcs1_w, 0x00ff) // CS1 (8-bit) J9 (iSBX1)
	AM_RANGE(0x0370, 0x037f) /* PCS6:15 */ AM_DEVREADWRITE8(ISBX_1_TAG, isbx_slot_device, mdack_r, mdack_w, 0xff00) // DMA-ACK J9 (iSBX1)
#endif
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( upd7220_map )
//-------------------------------------------------

static ADDRESS_MAP_START( upd7220_map, AS_0, 8, compis_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1ffff)
	AM_RANGE(0x00000, 0x1ffff) AM_RAM AM_SHARE("video_ram")
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( compis )
//-------------------------------------------------

static INPUT_PORTS_START( compis )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x18, 0x00, "S8 Test mode")
	PORT_DIPSETTING( 0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING( 0x08, "Remote" )
	PORT_DIPSETTING( 0x10, "Stand alone" )
	PORT_DIPSETTING( 0x18, "Reserved" )
INPUT_PORTS_END


//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  I80186_INTERFACE( cpu_intf )
//-------------------------------------------------

READ8_MEMBER( compis_state::compis_irq_callback )
{
	return m_8259m->inta_r();
}

WRITE_LINE_MEMBER( compis_state::tmr0_w )
{
	m_tmr0 = state;
	
	m_cassette->output(m_tmr0 ? -1 : 1);
}


//-------------------------------------------------
//  I80130_INTERFACE( osp_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( compis_state::tmr2_w )
{
	m_uart->rxc_w(state);
	m_uart->txc_w(state);
}

static const struct pit8253_interface osp_pit_intf =
{
	{
		{ XTAL_16MHz/2, DEVCB_LINE_VCC, DEVCB_NULL /*DEVCB_DEVICE_LINE_MEMBER("pic8259_master", pic8259_device, ir3_w)*/ }, // SYSTICK
		{ XTAL_16MHz/2, DEVCB_LINE_VCC, DEVCB_DEVICE_LINE_MEMBER("pic8259_master", pic8259_device, ir7_w) }, // DELAY
		{ 7932659, DEVCB_LINE_VCC, DEVCB_DRIVER_LINE_MEMBER(compis_state, tmr2_w) } // BAUD
	}
};


//-------------------------------------------------
//  pit8253_interface pit_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( compis_state::tmr3_w )
{
	m_mpsc->rxtxcb_w(state);
}

WRITE_LINE_MEMBER( compis_state::tmr4_w )
{
}

WRITE_LINE_MEMBER( compis_state::tmr5_w )
{
	m_mpsc->rxca_w(state);
	m_mpsc->txca_w(state);
}

static const struct pit8253_interface pit_intf =
{
	{
		{ XTAL_16MHz/8, DEVCB_LINE_VCC, DEVCB_DRIVER_LINE_MEMBER(compis_state, tmr3_w) },
		{ XTAL_16MHz/8, DEVCB_LINE_VCC, DEVCB_DRIVER_LINE_MEMBER(compis_state, tmr4_w) },
		{ XTAL_16MHz/8, DEVCB_LINE_VCC, DEVCB_DRIVER_LINE_MEMBER(compis_state, tmr5_w) }
	}
};


//-------------------------------------------------
//  i8251_interface usart_intf
//-------------------------------------------------

static const i8251_interface usart_intf =
{
	DEVCB_DEVICE_LINE_MEMBER(COMPIS_KEYBOARD_TAG, compis_keyboard_device, so_r),
	DEVCB_DEVICE_LINE_MEMBER(COMPIS_KEYBOARD_TAG, compis_keyboard_device, si_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER("pic8259_master", pic8259_device, ir2_w),
	DEVCB_NULL, //DEVCB_DEVICE_LINE_MEMBER(I80186_TAG, i80186_cpu_device, int1_w),
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  I8255A_INTERFACE( ppi_intf )
//-------------------------------------------------

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
	data = ioport("DSW0")->read();

	// cassette
	data |= (m_cassette->input() > 0.0) << 2;

	/* Centronics busy */
	data |= m_centronics->busy_r() << 5;
	data |= m_centronics->vcc_r() << 6;

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

	m_centronics->strobe_w(BIT(data, 5));

	if (BIT(data, 6))
	{
		m_isbx0->reset();
	}

	m_isbx0->opt0_w(BIT(data, 7));
}

static I8255A_INTERFACE( ppi_intf )
{
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER(CENTRONICS_TAG, centronics_device, write),
	DEVCB_DRIVER_MEMBER(compis_state, ppi_pb_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(compis_state, ppi_pc_w)
};


//-------------------------------------------------
//  I8274_INTERFACE( mpsc_intf )
//-------------------------------------------------

static I8274_INTERFACE( mpsc_intf )
{
	0, 0, 0, 0,

	DEVCB_DEVICE_LINE_MEMBER(RS232_A_TAG, serial_port_device, rx),
	DEVCB_DEVICE_LINE_MEMBER(RS232_A_TAG, serial_port_device, tx),
	DEVCB_DEVICE_LINE_MEMBER(RS232_A_TAG, rs232_port_device, dtr_w),
	DEVCB_DEVICE_LINE_MEMBER(RS232_A_TAG, rs232_port_device, rts_w),
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_DEVICE_LINE_MEMBER(RS232_B_TAG, serial_port_device, rx),
	DEVCB_DEVICE_LINE_MEMBER(RS232_B_TAG, serial_port_device, tx),
	DEVCB_DEVICE_LINE_MEMBER(RS232_B_TAG, rs232_port_device, dtr_w),
	DEVCB_DEVICE_LINE_MEMBER(RS232_B_TAG, rs232_port_device, rts_w),
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_DEVICE_LINE_MEMBER(I80186_TAG, i80186_cpu_device, int3_w)
};


//-------------------------------------------------
//  mm58274c_interface rtc_intf
//-------------------------------------------------

static const mm58274c_interface rtc_intf =
{
	0,  /*  mode 24*/
	1   /*  first day of week */
};


//-------------------------------------------------
//  rs232_port_interface rs232a_intf
//-------------------------------------------------

static const rs232_port_interface rs232a_intf =
{
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(I8274_TAG, z80dart_device, dcda_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(I8274_TAG, z80dart_device, ctsa_w)
};


//-------------------------------------------------
//  rs232_port_interface rs232b_intf
//-------------------------------------------------

static const rs232_port_interface rs232b_intf =
{
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(I8274_TAG, z80dart_device, dcdb_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(I8274_TAG, z80dart_device, ctsb_w)
};


//-------------------------------------------------
//  cassette_interface compis_cassette_interface
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER( compis_state::tape_tick )
{
	m_maincpu->tmrin0_w(m_cassette->input() > 0.0);
}

static const cassette_interface compis_cassette_interface =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED),
	NULL,
	NULL
};



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
		m_maincpu->space(AS_PROGRAM).install_ram(0x20000, 0x3ffff, NULL);
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

	// video hardware
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 400-1)
	MCFG_SCREEN_UPDATE_DEVICE("upd7220", upd7220_device, screen_update)
	MCFG_UPD7220_ADD("upd7220", XTAL_4_433619MHz/2, hgdc_intf, upd7220_map) //unknown clock

	// devices
	MCFG_PIC8259_ADD("pic8259_master", DEVWRITELINE(I80186_TAG, i80186_cpu_device, int0_w), VCC, NULL ) // inside 80130
	MCFG_PIT8254_ADD("pit8254", osp_pit_intf ) // inside 80130
	MCFG_PIT8253_ADD(I8253_TAG, pit_intf )
	MCFG_I8255_ADD(I8255_TAG, ppi_intf )
	MCFG_I8251_ADD(I8251A_TAG, usart_intf)
	MCFG_I8274_ADD(I8274_TAG, XTAL_16MHz/4, mpsc_intf)
	MCFG_MM58274C_ADD(MM58174A_TAG, rtc_intf)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, compis_cassette_interface)
	MCFG_RS232_PORT_ADD(RS232_A_TAG, rs232a_intf, default_rs232_devices, NULL)
	MCFG_RS232_PORT_ADD(RS232_B_TAG, rs232b_intf, default_rs232_devices, NULL)
	MCFG_CENTRONICS_PRINTER_ADD(CENTRONICS_TAG, standard_centronics)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("tape", compis_state, tape_tick, attotime::from_hz(44100))
	MCFG_ISBX_SLOT_ADD(ISBX_0_TAG, 0, isbx_cards, "fdc")
	MCFG_ISBX_SLOT_MINTR0_CALLBACK(DEVWRITELINE("pic8259_master", pic8259_device, ir1_w))
	MCFG_ISBX_SLOT_MINTR1_CALLBACK(DEVWRITELINE("pic8259_master", pic8259_device, ir0_w))
	MCFG_ISBX_SLOT_MDRQT_CALLBACK(DEVWRITELINE(I80186_TAG, i80186_cpu_device, drq0_w))
	MCFG_ISBX_SLOT_ADD(ISBX_1_TAG, 0, isbx_cards, NULL)
	MCFG_ISBX_SLOT_MINTR0_CALLBACK(DEVWRITELINE("pic8259_master", pic8259_device, ir6_w))
	MCFG_ISBX_SLOT_MINTR1_CALLBACK(DEVWRITELINE("pic8259_master", pic8259_device, ir5_w))
	MCFG_ISBX_SLOT_MDRQT_CALLBACK(DEVWRITELINE(I80186_TAG, i80186_cpu_device, drq1_w))
	MCFG_COMPIS_KEYBOARD_ADD(NULL)

	// software lists
	MCFG_SOFTWARE_LIST_ADD("flop_list", "compis")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_EXTRA_OPTIONS("256K")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( compis )
//-------------------------------------------------

ROM_START( compis )
	ROM_REGION16_LE( 0x10000, I80186_TAG, 0 )
	ROM_LOAD16_BYTE( "sa883003.u40", 0x0000, 0x4000, CRC(195ef6bf) SHA1(eaf8ae897e1a4b62d3038ff23777ce8741b766ef) )
	ROM_LOAD16_BYTE( "sa883003.u36", 0x0001, 0x4000, CRC(7c918f56) SHA1(8ba33d206351c52f44f1aa76cc4d7f292dcef761) )
	ROM_LOAD16_BYTE( "sa883003.u39", 0x8000, 0x4000, CRC(3cca66db) SHA1(cac36c9caa2f5bb42d7a6d5b84f419318628935f) )
	ROM_LOAD16_BYTE( "sa883003.u35", 0x8001, 0x4000, CRC(43c38e76) SHA1(f32e43604107def2c2259898926d090f2ed62104) )
ROM_END


//-------------------------------------------------
//  ROM( compis2 )
//-------------------------------------------------

ROM_START( compis2 )
	ROM_REGION16_LE( 0x10000, I80186_TAG, 0 )
	ROM_DEFAULT_BIOS( "v303" )

	ROM_SYSTEM_BIOS( 0, "v302", "Compis II v3.02 (1986-09-09)" )
	ROMX_LOAD( "comp302.u39", 0x0000, 0x8000, CRC(16a7651e) SHA1(4cbd4ba6c6c915c04dfc913ec49f87c1dd7344e3), ROM_BIOS(1) | ROM_SKIP(1) )
	ROMX_LOAD( "comp302.u35", 0x0001, 0x8000, CRC(ae546bef) SHA1(572e45030de552bb1949a7facbc885b8bf033fc6), ROM_BIOS(1) | ROM_SKIP(1) )

	ROM_SYSTEM_BIOS( 1, "v303", "Compis II v3.03 (1987-03-09)" )
	ROMX_LOAD( "rysa094.u39", 0x0000, 0x8000, CRC(e7302bff) SHA1(44ea20ef4008849af036c1a945bc4f27431048fb), ROM_BIOS(2) | ROM_SKIP(1) )
	ROMX_LOAD( "rysa094.u35", 0x0001, 0x8000, CRC(b0694026) SHA1(eb6b2e3cb0f42fd5ffdf44f70e652ecb9714ce30), ROM_BIOS(2) | ROM_SKIP(1) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   INIT                         COMPANY             FULLNAME        FLAGS
COMP(1985,  compis,     0,      0,     compis,  compis, driver_device, 0, "Telenova", "Compis" , GAME_NOT_WORKING )
COMP(1986,  compis2,    compis, 0,     compis, compis, driver_device, 0, "Telenova", "Compis II" , GAME_NOT_WORKING )
