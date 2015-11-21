// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Luxor ABC 1600

    How to create HDD image:
    ------------------------
    chdman createhd -chs 615,4,17 -ss 512 -o necd5126a.chd

    How to format HDD:
    ------------------
    mf(2,0)
    mf(2,0)
    abcenix
    sas/format/format
    sa(40,0)
    y
    5
    necd5126a

    How to install OS:
    ------------------
    mf(2,0)
    mf(2,0)
    abcenix
    loadsys1
    <enter>
    <enter>

*/

/*

    TODO:

    - short/long reset (RSTBUT)
    - CIO
        - optimize timers!
        - port C, open drain output bit PC1 (RTC/NVRAM data)
    - hard disk
        - 4105 SASI interface card
        - SASI interface (scsibus.c)
    - connect RS-232 port A

*/

#include "includes/abc1600.h"
#include "softlist.h"


//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define LOG 0


#define A1          BIT(offset, 1)
#define A2          BIT(offset, 2)
#define A4          BIT(offset, 4)
#define X11         BIT(offset, 11)
#define A1_A2       ((A1 << 1) | A2)
#define A2_A1       ((offset >> 1) & 0x03)


// external I/O
enum
{
	INP = 0,
	STAT,
	OPS
};

enum
{
	OUT = 0,
	C1 = 2,
	C2,
	C3,
	C4
};




//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  bus_r -
//-------------------------------------------------

READ8_MEMBER( abc1600_state::bus_r )
{
	UINT8 data = 0;

	// card select pulse
	UINT8 cs = (m_cs7 << 7) | ((offset >> 5) & 0x3f);

	m_bus0i->cs_w(cs);
	m_bus0x->cs_w(cs);
	m_bus1->cs_w(cs);
	m_bus2->cs_w(cs);

	// card select b?
	m_csb = m_bus2->csb_r();
	m_csb |= m_bus1->csb_r() << 1;
	m_csb |= m_bus0x->xcsb2_r() << 2;
	m_csb |= m_bus0x->xcsb3_r() << 3;
	m_csb |= m_bus0x->xcsb4_r() << 4;
	m_csb |= m_bus0x->xcsb5_r() << 5;
	m_csb |= m_bus0x->csb_r() << 6;
	m_csb |= m_bus0i->csb_r() << 7;

	m_bus0 = !((m_csb & 0xfc) == 0xfc);

	if (X11)
	{
		if (A4)
		{
			// EXP
			data = m_bus0x->exp_r();

			if (LOG) logerror("%s EXP %02x: %02x\n", machine().describe_context(), cs, data);
		}
		else
		{
			// RCSB
			if (m_bus0)
			{
				/*

				    bit     description

				    0       1
				    1       1
				    2       LXCSB2*
				    3       LXCSB3*
				    4       LXCSB4*
				    5       LXCSB5*
				    6       LCSB*-0
				    7       LCSB*-0I

				*/

				data = (m_csb & 0xfc) | 0x03;
			}
			else
			{
				/*

				    bit     description

				    0       LCSB*-2
				    1       LCSB*-1
				    2       1
				    3       1
				    4       1
				    5       1
				    6       1
				    7       1

				*/

				data = 0xfc | (m_csb & 0x03);
			}

			if (LOG) logerror("%s RCSB %02x\n", machine().describe_context(), data);
		}
	}
	else
	{
		data = 0xff;

		switch ((offset >> 1) & 0x07)
		{
		case INP:
			if (m_bus0)
			{
				data &= m_bus0i->inp_r();
				data &= m_bus0x->inp_r();
			}
			else
			{
				data &= m_bus1->inp_r();
				data &= m_bus2->inp_r();
			}

			if (LOG) logerror("%s INP %02x: %02x\n", machine().describe_context(), cs, data);
			break;

		case STAT:
			if (m_bus0)
			{
				data &= m_bus0i->stat_r();
				data &= m_bus0x->stat_r();
			}
			else
			{
				data &= m_bus1->stat_r();
				data &= m_bus2->stat_r();
			}

			if (LOG) logerror("%s STAT %02x: %02x\n", machine().describe_context(), cs, data);
			break;

		case OPS:
			if (m_bus0)
			{
				data &= m_bus0i->ops_r();
				data &= m_bus0x->ops_r();
			}
			else
			{
				data &= m_bus1->ops_r();
				data &= m_bus2->ops_r();
			}

			if (LOG) logerror("%s OPS %02x: %02x\n", machine().describe_context(), cs, data);
			break;

		default:
			if (LOG) logerror("%s Unmapped read from virtual I/O %06x\n", machine().describe_context(), offset);
		}
	}

	return data;
}


//-------------------------------------------------
//  bus_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_state::bus_w )
{
	UINT8 cs = (m_cs7 << 7) | ((offset >> 5) & 0x3f);

	m_bus0i->cs_w(cs);
	m_bus0x->cs_w(cs);
	m_bus1->cs_w(cs);
	m_bus2->cs_w(cs);

	switch ((offset >> 1) & 0x07)
	{
	case OUT:
		if (LOG) logerror("%s OUT %02x: %02x\n", machine().describe_context(), cs, data);

		if (m_bus0)
		{
			m_bus0i->out_w(data);
			m_bus0x->out_w(data);
		}
		else
		{
			m_bus1->out_w(data);
			m_bus2->out_w(data);
		}
		break;

	case C1:
		if (LOG) logerror("%s C1 %02x: %02x\n", machine().describe_context(), cs, data);

		if (m_bus0)
		{
			m_bus0i->c1_w(data);
			m_bus0x->c1_w(data);
		}
		else
		{
			m_bus1->c1_w(data);
			m_bus2->c1_w(data);
		}
		break;

	case C2:
		if (LOG) logerror("%s C2 %02x: %02x\n", machine().describe_context(), cs, data);

		if (m_bus0)
		{
			m_bus0i->c2_w(data);
			m_bus0x->c2_w(data);
		}
		else
		{
			m_bus1->c2_w(data);
			m_bus2->c2_w(data);
		}
		break;

	case C3:
		if (LOG) logerror("%s C3 %02x: %02x\n", machine().describe_context(), cs, data);

		if (m_bus0)
		{
			m_bus0i->c3_w(data);
			m_bus0x->c3_w(data);
		}
		else
		{
			m_bus1->c3_w(data);
			m_bus2->c3_w(data);
		}
		break;

	case C4:
		if (LOG) logerror("%s C4 %02x: %02x\n", machine().describe_context(), cs, data);

		if (m_bus0)
		{
			m_bus0i->c4_w(data);
			m_bus0x->c4_w(data);
		}
		else
		{
			m_bus1->c4_w(data);
			m_bus2->c4_w(data);
		}
		break;

	default:
		if (LOG) logerror("%s Unmapped write %02x to virtual I/O %06x\n", machine().describe_context(), data, offset);
	}
}


//-------------------------------------------------
//  fw0_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_state::fw0_w )
{
	/*

	    bit     description

	    0       SEL1
	    1       SEL2
	    2       SEL3
	    3       MOTOR
	    4       LC/PC
	    5       LC/PC
	    6
	    7

	*/

	if (LOG) logerror("FW0 %02x\n", data);

	// drive select
	floppy_image_device *floppy = NULL;

	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	if (BIT(data, 1)) floppy = m_floppy1->get_device();
	if (BIT(data, 2)) floppy = m_floppy2->get_device();

	m_fdc->set_floppy(floppy);

	// floppy motor
	if (floppy) floppy->mon_w(!BIT(data, 3));
}


//-------------------------------------------------
//  fw1_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_state::fw1_w )
{
	/*

	    bit     description

	    0       MR
	    1       DDEN
	    2       HLT
	    3       MINI
	    4       HLD
	    5       P0
	    6       P1
	    7       P2

	*/

	if (LOG) logerror("FW1 %02x\n", data);

	// FDC master reset
	if (!BIT(data, 0)) m_fdc->reset();

	// density select
	m_fdc->dden_w(BIT(data, 1));
}


//-------------------------------------------------
//  spec_contr_reg_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_state::spec_contr_reg_w )
{
	int state = BIT(data, 3);

	switch (data & 0x07)
	{
	case 0: // CS7
		m_cs7 = state;
		break;

	case 1:
		break;

	case 2: // _BTCE
		m_btce = state;
		break;

	case 3: // _ATCE
		m_atce = state;
		break;

	case 4: // PARTST
		m_partst = state;
		break;

	case 5: // _DMADIS
		m_dmadis = state;
		break;

	case 6: // SYSSCC
		m_sysscc = state;

		m_cio->pb5_w(!state);
		m_bus1->pren_w(!state);

		update_drdy1();
		break;

	case 7: // SYSFS
		m_sysfs = state;

		m_cio->pb6_w(!state);
		m_bus0i->pren_w(!state);
		m_bus0x->pren_w(!state);

		update_drdy0();
		break;
	}
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( abc1600_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( abc1600_mem, AS_PROGRAM, 8, abc1600_state )
	AM_RANGE(0x00000, 0xfffff) AM_DEVICE(ABC1600_MAC_TAG, abc1600_mac_device, map)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( mac_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( mac_mem, AS_PROGRAM, 8, abc1600_state )
	AM_RANGE(0x000000, 0x0fffff) AM_RAM
	AM_RANGE(0x100000, 0x17ffff) AM_DEVICE(ABC1600_MOVER_TAG, abc1600_mover_device, vram_map)
	AM_RANGE(0x1fe000, 0x1fefff) AM_READWRITE(bus_r, bus_w)
	AM_RANGE(0x1ff000, 0x1ff000) AM_MIRROR(0xf9) AM_DEVREADWRITE(SAB1797_02P_TAG, fd1797_t, status_r, cmd_w)
	AM_RANGE(0x1ff002, 0x1ff002) AM_MIRROR(0xf9) AM_DEVREADWRITE(SAB1797_02P_TAG, fd1797_t, track_r, track_w)
	AM_RANGE(0x1ff004, 0x1ff004) AM_MIRROR(0xf9) AM_DEVREADWRITE(SAB1797_02P_TAG, fd1797_t, sector_r, sector_w)
	AM_RANGE(0x1ff006, 0x1ff006) AM_MIRROR(0xf9) AM_DEVREADWRITE(SAB1797_02P_TAG, fd1797_t, data_r, data_w)
	AM_RANGE(0x1ff100, 0x1ff101) AM_MIRROR(0xfe) AM_DEVICE(ABC1600_MOVER_TAG, abc1600_mover_device, crtc_map)
	AM_RANGE(0x1ff200, 0x1ff207) AM_MIRROR(0xf8) AM_READWRITE(dart_r, dart_w)
	AM_RANGE(0x1ff300, 0x1ff300) AM_MIRROR(0xff) AM_DEVREADWRITE(Z8410AB1_0_TAG, z80dma_device, read, write)
	AM_RANGE(0x1ff400, 0x1ff400) AM_MIRROR(0xff) AM_DEVREADWRITE(Z8410AB1_1_TAG, z80dma_device, read, write)
	AM_RANGE(0x1ff500, 0x1ff500) AM_MIRROR(0xff) AM_DEVREADWRITE(Z8410AB1_2_TAG, z80dma_device, read, write)
	AM_RANGE(0x1ff600, 0x1ff607) AM_MIRROR(0xf8) AM_READWRITE(scc_r, scc_w)
	AM_RANGE(0x1ff700, 0x1ff707) AM_MIRROR(0xf8) AM_READWRITE(cio_r, cio_w)
	AM_RANGE(0x1ff800, 0x1ffaff) AM_DEVICE(ABC1600_MOVER_TAG, abc1600_mover_device, io_map)
	AM_RANGE(0x1ffb00, 0x1ffb00) AM_MIRROR(0x7e) AM_WRITE(fw0_w)
	AM_RANGE(0x1ffb01, 0x1ffb01) AM_MIRROR(0x7e) AM_WRITE(fw1_w)
	AM_RANGE(0x1ffd00, 0x1ffd07) AM_MIRROR(0xf8) AM_DEVWRITE(ABC1600_MAC_TAG, abc1600_mac_device, dmamap_w)
	AM_RANGE(0x1ffe00, 0x1ffe00) AM_MIRROR(0xff) AM_WRITE(spec_contr_reg_w)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( abc1600 )
//-------------------------------------------------

static INPUT_PORTS_START( abc1600 )
	// inputs defined in machine/abc99.c
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  Z80DMA 0
//-------------------------------------------------

void abc1600_state::update_drdy0()
{
	if (m_sysfs)
	{
		// floppy
		m_dma0->rdy_w(!m_fdc->drq_r());
	}
	else
	{
		// BUS0I/BUS0X
		int trrq0 = m_bus0i->trrq_r() && m_bus0x->trrq_r();

		m_dma0->rdy_w(trrq0);
	}
}

WRITE_LINE_MEMBER( abc1600_state::dbrq_w )
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state && m_dmadis);
}

//-------------------------------------------------
//  Z80DMA 1
//-------------------------------------------------

void abc1600_state::update_drdy1()
{
	if (m_sysscc)
	{
		// SCC
		m_dma1->rdy_w(1);
	}
	else
	{
		// BUS1
		m_dma1->rdy_w(m_bus1->trrq_r());
	}
}

//-------------------------------------------------
//  Z80DMA 2
//-------------------------------------------------

void abc1600_state::update_drdy2()
{
	// Winchester
	m_dma2->rdy_w(1);
}

//-------------------------------------------------
//  Z80DART
//-------------------------------------------------

READ8_MEMBER( abc1600_state::dart_r )
{
	return m_dart->ba_cd_r(space, A2_A1 ^ 0x03);
}

WRITE8_MEMBER( abc1600_state::dart_w )
{
	m_dart->ba_cd_w(space, A2_A1 ^ 0x03, data);
}

//-------------------------------------------------
//  SCC8530_INTERFACE( sc_intf )
//-------------------------------------------------

READ8_MEMBER( abc1600_state::scc_r )
{
	return m_scc->reg_r(space, A1_A2);
}

WRITE8_MEMBER( abc1600_state::scc_w )
{
	m_scc->reg_w(space, A1_A2, data);
}


//-------------------------------------------------
//  Z8536_INTERFACE( cio_intf )
//-------------------------------------------------

READ8_MEMBER( abc1600_state::cio_r )
{
	return m_cio->read(space, A2_A1);
}

WRITE8_MEMBER( abc1600_state::cio_w )
{
	m_cio->write(space, A2_A1, data);
}

READ8_MEMBER( abc1600_state::cio_pa_r )
{
	/*

	    bit     description

	    PA0     BUS2
	    PA1     BUS1
	    PA2     BUS0X*2
	    PA3     BUS0X*3
	    PA4     BUS0X*4
	    PA5     BUS0X*5
	    PA6     BUS0X
	    PA7     BUS0I

	*/

	UINT8 data = 0;

	data |= m_bus2->irq_r();
	data |= m_bus1->irq_r() << 1;
	data |= m_bus0x->xint2_r() << 2;
	data |= m_bus0x->xint3_r() << 3;
	data |= m_bus0x->xint4_r() << 4;
	data |= m_bus0x->xint5_r() << 5;
	data |= m_bus0x->irq_r() << 6;
	data |= m_bus0i->irq_r() << 7;

	return data;
}

READ8_MEMBER( abc1600_state::cio_pb_r )
{
	/*

	    bit     description

	    PB0
	    PB1     POWERFAIL
	    PB2
	    PB3
	    PB4     MINT
	    PB5     _PREN-1
	    PB6     _PREN-0
	    PB7     FINT

	*/

	UINT8 data = 0;

	data |= !m_sysscc << 5;
	data |= !m_sysfs << 6;

	// floppy interrupt
	data |= m_fdc->intrq_r() << 7;

	return data;
}

WRITE8_MEMBER( abc1600_state::cio_pb_w )
{
	/*

	    bit     description

	    PB0     PRBR
	    PB1
	    PB2
	    PB3
	    PB4
	    PB5
	    PB6
	    PB7

	*/

	// printer baudrate
	int prbr = BIT(data, 0);

	m_dart->txca_w(prbr);
	m_dart->rxca_w(prbr);
}

READ8_MEMBER( abc1600_state::cio_pc_r )
{
	/*

	    bit     description

	    PC0     1
	    PC1     DATA IN
	    PC2     1
	    PC3     1

	*/

	UINT8 data = 0x0d;

	// data in
	data |= (m_rtc->dio_r() || m_nvram->do_r()) << 1;

	return data;
}

WRITE8_MEMBER( abc1600_state::cio_pc_w )
{
	/*

	    bit     description

	    PC0     CLOCK
	    PC1     DATA OUT
	    PC2     RTC CS
	    PC3     NVRAM CS

	*/

	int clock = BIT(data, 0);
	int data_out = BIT(data, 1);
	int rtc_cs = BIT(data, 2);
	int nvram_cs = BIT(data, 3);

	if (LOG) logerror("CLK %u DATA %u RTC %u NVRAM %u\n", clock, data_out, rtc_cs, nvram_cs);

	m_rtc->cs_w(rtc_cs);
	m_rtc->dio_w(data_out);
	m_rtc->clk_w(clock);

	m_nvram->cs_w(nvram_cs);
	m_nvram->di_w(data_out);
	m_nvram->sk_w(clock);
}

static SLOT_INTERFACE_START( abc1600_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END

WRITE_LINE_MEMBER( abc1600_state::fdc_drq_w )
{
	update_drdy0();
}


//-------------------------------------------------
//  ABC1600BUS_INTERFACE( abcbus_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( abc1600_state::nmi_w )
{
	if (state == ASSERT_LINE)
	{
		m_maincpu->set_input_line(M68K_IRQ_7, ASSERT_LINE);
	}
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  IRQ_CALLBACK_MEMBER( abc1600_int_ack )
//-------------------------------------------------

IRQ_CALLBACK_MEMBER( abc1600_state::abc1600_int_ack )
{
	int data = 0;

	switch (irqline)
	{
	case M68K_IRQ_2:
		data = m_cio->intack_r();
		break;

	case M68K_IRQ_5:
		data = m_dart->m1_r();
		break;

	case M68K_IRQ_7:
		m_maincpu->set_input_line(M68K_IRQ_7, CLEAR_LINE);

		data = M68K_INT_ACK_AUTOVECTOR;
		break;
	}

	return data;
}


void abc1600_state::machine_start()
{
	// state saving
	save_item(NAME(m_dmadis));
	save_item(NAME(m_sysscc));
	save_item(NAME(m_sysfs));
	save_item(NAME(m_partst));
	save_item(NAME(m_cs7));
	save_item(NAME(m_bus0));
	save_item(NAME(m_csb));
	save_item(NAME(m_atce));
	save_item(NAME(m_btce));
}


void abc1600_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	// clear special control register
	for (int i = 0; i < 8; i++)
	{
		spec_contr_reg_w(program, 0, i);
	}

	// clear floppy registers
	fw0_w(program, 0, 0);
	fw1_w(program, 0, 0);

	// clear NMI
	m_maincpu->set_input_line(M68K_IRQ_7, CLEAR_LINE);
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( abc1600 )
//-------------------------------------------------

static MACHINE_CONFIG_START( abc1600, abc1600_state )
	// basic machine hardware
	MCFG_CPU_ADD(MC68008P8_TAG, M68008, XTAL_64MHz/8)
	MCFG_CPU_PROGRAM_MAP(abc1600_mem)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(abc1600_state,abc1600_int_ack)

	MCFG_WATCHDOG_TIME_INIT(attotime::from_msec(1600)) // XTAL_64MHz/8/10/20000/8/8

	// video hardware
	MCFG_ABC1600_MOVER_ADD()

	// devices
	MCFG_ABC1600_MAC_ADD(MC68008P8_TAG, mac_mem)

	MCFG_DEVICE_ADD(Z8410AB1_0_TAG, Z80DMA, XTAL_64MHz/16)
	MCFG_Z80DMA_OUT_BUSREQ_CB(WRITELINE(abc1600_state, dbrq_w))
	MCFG_Z80DMA_OUT_BAO_CB(DEVWRITELINE(Z8410AB1_1_TAG, z80dma_device, bai_w))
	MCFG_Z80DMA_IN_MREQ_CB(DEVREAD8(ABC1600_MAC_TAG, abc1600_mac_device, dma0_mreq_r))
	MCFG_Z80DMA_OUT_MREQ_CB(DEVWRITE8(ABC1600_MAC_TAG, abc1600_mac_device, dma0_mreq_w))
	MCFG_Z80DMA_IN_IORQ_CB(DEVREAD8(ABC1600_MAC_TAG, abc1600_mac_device, dma0_iorq_r))
	MCFG_Z80DMA_OUT_IORQ_CB(DEVWRITE8(ABC1600_MAC_TAG, abc1600_mac_device, dma0_iorq_w))

	MCFG_DEVICE_ADD(Z8410AB1_1_TAG, Z80DMA, XTAL_64MHz/16)
	MCFG_Z80DMA_OUT_BUSREQ_CB(WRITELINE(abc1600_state, dbrq_w))
	MCFG_Z80DMA_OUT_BAO_CB(DEVWRITELINE(Z8410AB1_2_TAG, z80dma_device, bai_w))
	MCFG_Z80DMA_IN_MREQ_CB(DEVREAD8(ABC1600_MAC_TAG, abc1600_mac_device, dma1_mreq_r))
	MCFG_Z80DMA_OUT_MREQ_CB(DEVWRITE8(ABC1600_MAC_TAG, abc1600_mac_device, dma1_mreq_w))
	MCFG_Z80DMA_IN_IORQ_CB(DEVREAD8(ABC1600_MAC_TAG, abc1600_mac_device, dma1_iorq_r))
	MCFG_Z80DMA_OUT_IORQ_CB(DEVWRITE8(ABC1600_MAC_TAG, abc1600_mac_device, dma1_iorq_w))

	MCFG_DEVICE_ADD(Z8410AB1_2_TAG, Z80DMA, XTAL_64MHz/16)
	MCFG_Z80DMA_OUT_BUSREQ_CB(WRITELINE(abc1600_state, dbrq_w))
	MCFG_Z80DMA_IN_MREQ_CB(DEVREAD8(ABC1600_MAC_TAG, abc1600_mac_device, dma2_mreq_r))
	MCFG_Z80DMA_OUT_MREQ_CB(DEVWRITE8(ABC1600_MAC_TAG, abc1600_mac_device, dma2_mreq_w))
	MCFG_Z80DMA_IN_IORQ_CB(DEVREAD8(ABC1600_MAC_TAG, abc1600_mac_device, dma2_iorq_r))
	MCFG_Z80DMA_OUT_IORQ_CB(DEVWRITE8(ABC1600_MAC_TAG, abc1600_mac_device, dma2_iorq_w))

	MCFG_Z80DART_ADD(Z8470AB1_TAG, XTAL_64MHz/16, 0, 0, 0, 0 )
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE(ABC_KEYBOARD_PORT_TAG, abc_keyboard_port_device, txd_w))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(MC68008P8_TAG, M68K_IRQ_5))    // shared with SCC

	MCFG_DEVICE_ADD(Z8530B1_TAG, SCC8530, XTAL_64MHz/16)
	MCFG_Z8530_INTRQ_CALLBACK(INPUTLINE(MC68008P8_TAG, M68K_IRQ_5))

	MCFG_DEVICE_ADD(Z8536B1_TAG, Z8536, XTAL_64MHz/16)
	MCFG_Z8536_IRQ_CALLBACK(INPUTLINE(MC68008P8_TAG, M68K_IRQ_2))
	MCFG_Z8536_PA_IN_CALLBACK(READ8(abc1600_state, cio_pa_r))
	MCFG_Z8536_PB_IN_CALLBACK(READ8(abc1600_state, cio_pb_r))
	MCFG_Z8536_PB_OUT_CALLBACK(WRITE8(abc1600_state, cio_pb_w))
	MCFG_Z8536_PC_IN_CALLBACK(READ8(abc1600_state, cio_pc_r))
	MCFG_Z8536_PC_OUT_CALLBACK(WRITE8(abc1600_state, cio_pc_w))

	MCFG_NMC9306_ADD(NMC9306_TAG)
	MCFG_E0516_ADD(E050_C16PC_TAG, XTAL_32_768kHz)
	MCFG_FD1797_ADD(SAB1797_02P_TAG, XTAL_64MHz/64)
	MCFG_WD_FDC_INTRQ_CALLBACK(DEVWRITELINE(Z8536B1_TAG, z8536_device, pb7_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(abc1600_state, fdc_drq_w))

	MCFG_FLOPPY_DRIVE_ADD(SAB1797_02P_TAG":0", abc1600_floppies, NULL,    floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(SAB1797_02P_TAG":1", abc1600_floppies, NULL,    floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(SAB1797_02P_TAG":2", abc1600_floppies, "525qd", floppy_image_device::default_floppy_formats)

	MCFG_RS232_PORT_ADD(RS232_A_TAG, default_rs232_devices, NULL)

	MCFG_RS232_PORT_ADD(RS232_B_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z8470AB1_TAG, z80dart_device, rxa_w))

	MCFG_ABC_KEYBOARD_PORT_ADD(ABC_KEYBOARD_PORT_TAG, "abc99")
	MCFG_ABC_KEYBOARD_OUT_RX_HANDLER(DEVWRITELINE(Z8470AB1_TAG, z80dart_device, rxb_w))
	MCFG_ABC_KEYBOARD_OUT_TRXC_HANDLER(DEVWRITELINE(Z8470AB1_TAG, z80dart_device, rxtxcb_w))
	MCFG_ABC_KEYBOARD_OUT_KEYDOWN_HANDLER(DEVWRITELINE(Z8470AB1_TAG, z80dart_device, dcdb_w))

	MCFG_ABCBUS_SLOT_ADD("bus0i", abc1600bus_cards, NULL)
	MCFG_ABCBUS_SLOT_IRQ_CALLBACK(DEVWRITELINE(Z8536B1_TAG, z8536_device, pa7_w))
	MCFG_ABCBUS_SLOT_ADD("bus0x", abc1600bus_cards, NULL)
	MCFG_ABCBUS_SLOT_IRQ_CALLBACK(DEVWRITELINE(Z8536B1_TAG, z8536_device, pa6_w))
	MCFG_ABCBUS_SLOT_NMI_CALLBACK(WRITELINE(abc1600_state, nmi_w))
	MCFG_ABCBUS_SLOT_XINT2_CALLBACK(DEVWRITELINE(Z8536B1_TAG, z8536_device, pa2_w))
	MCFG_ABCBUS_SLOT_XINT3_CALLBACK(DEVWRITELINE(Z8536B1_TAG, z8536_device, pa3_w))
	MCFG_ABCBUS_SLOT_XINT4_CALLBACK(DEVWRITELINE(Z8536B1_TAG, z8536_device, pa4_w))
	MCFG_ABCBUS_SLOT_XINT5_CALLBACK(DEVWRITELINE(Z8536B1_TAG, z8536_device, pa5_w))
	MCFG_ABCBUS_SLOT_ADD("bus1", abc1600bus_cards, NULL)
	MCFG_ABCBUS_SLOT_IRQ_CALLBACK(DEVWRITELINE(Z8536B1_TAG, z8536_device, pa1_w))
	MCFG_ABCBUS_SLOT_ADD("bus2", abc1600bus_cards, "4105")
	MCFG_ABCBUS_SLOT_IRQ_CALLBACK(DEVWRITELINE(Z8536B1_TAG, z8536_device, pa0_w))
	//MCFG_ABCBUS_SLOT_PREN_CALLBACK(DEVWRITELINE(Z8410AB1_2_TAG, z80dma_device, iei_w))
	MCFG_ABCBUS_SLOT_TRRQ_CALLBACK(DEVWRITELINE(Z8410AB1_2_TAG, z80dma_device, rdy_w))

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1M")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "abc1600")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( abc1600 )
//-------------------------------------------------

ROM_START( abc1600 )
	ROM_REGION( 0x71c, "plds", 0 )
	ROM_LOAD( "1020 6490349-01.8b",  0x104, 0x104, CRC(1fa065eb) SHA1(20a95940e39fa98e97e59ea1e548ac2e0c9a3444) ) // expansion bus strobes
	ROM_LOAD( "1021 6490350-01.5d",  0x208, 0x104, CRC(96f6f44b) SHA1(12d1cd153dcc99d1c4a6c834122f370d49723674) ) // interrupt encoder and ROM/RAM control
	ROM_LOAD( "1023 6490352-01.11e", 0x410, 0x104, CRC(a2f350ac) SHA1(77e08654a197080fa2111bc3031cd2c7699bf82b) ) // interrupt acknowledge
	ROM_LOAD( "1024 6490353-01.12e", 0x514, 0x104, CRC(67f1328a) SHA1(b585495fe14a7ae2fbb29f722dca106d59325002) ) // expansion bus timing and control
	ROM_LOAD( "1025 6490354-01.6e",  0x618, 0x104, CRC(9bda0468) SHA1(ad373995dcc18532274efad76fa80bd13c23df25) ) // DMA transfer
	//ROM_LOAD( "pal16r4.10c", 0x71c, 0x104, NO_DUMP ) // SCC read/write, mentioned in the preliminary service manual, but not present on the PCB
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     INIT  COMPANY     FULLNAME     FLAGS
COMP( 1985, abc1600, 0,      0,      abc1600, abc1600, driver_device, 0,    "Luxor", "ABC 1600", MACHINE_NOT_WORKING )
