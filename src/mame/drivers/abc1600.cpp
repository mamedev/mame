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

    - starting from MAME 0.151, the Z80 DMA reads 0x08 as the 257th byte to transfer from disk t0s14 thus failing a comparison @ 37cfa, leading to a watchdog reset
      changing z80dma.cpp:480 to "done = (m_count == 0);" fixes this but isn't the real reason
    - segment/page RAM addresses are not correctly decoded, "sas/format/format" after abcenix is booted can't find the SASI interface because of this
        [:mac] ':3f' (08A98) MAC 7e4a2:0004a2 (SEGA 02f SEGD 09 PGA 09c PGD 8000 NONX 1 WP 0)
            should be
        [:mac] ':3f' (089A8) MAC 7e4a2:1fe4a2 (SEGA 00f SEGD 0f PGA 0fc PGD 43fc NONX 0 WP 1)

        [:mac] ':3f' (100082) SEGMENT 80eeb:27 (SEGA 000 SEGD 27)
        [:mac] ':3f' (100084) PAGE 80ee8:02 (SEGA 000 SEGD 27 PGA 271 PGD 0283)
        [:mac] ':3f' (100084) PAGE 80ee9:83 (SEGA 000 SEGD 27 PGA 271 PGD 0283)
        [:mac] ':3f' (100082) SEGMENT 806eb:27 (SEGA 000 SEGD 27)
        [:mac] ':3f' (100084) PAGE 806e8:02 (SEGA 000 SEGD 27 PGA 270 PGD 0282)
        [:mac] ':3f' (100084) PAGE 806e9:82 (SEGA 000 SEGD 27 PGA 270 PGD 0282)
        [:mac] ':3f' (100082) MAC 7feea:1b8eea (SEGA 00f SEGD 36 PGA 36f PGD 0371 NONX 0 WP 0)
        [:mac] ':3f' (100082): unmapped program memory write to 1B8EEA = 00 & FF
        [:mac] ':3f' (100082) MAC 7feeb:1b8eeb (SEGA 00f SEGD 36 PGA 36f PGD 0371 NONX 0 WP 0)
        [:mac] ':3f' (100082): unmapped program memory write to 1B8EEB = 27 & FF
        [:mac] ':3f' (100084) MAC 7fee8:1b8ee8 (SEGA 00f SEGD 36 PGA 36f PGD 0371 NONX 0 WP 0)
        [:mac] ':3f' (100084): unmapped program memory write to 1B8EE8 = 02 & FF
        [:mac] ':3f' (100084) MAC 7fee9:1b8ee9 (SEGA 00f SEGD 36 PGA 36f PGD 0371 NONX 0 WP 0)
        [:mac] ':3f' (100084): unmapped program memory write to 1B8EE9 = 81 & FF
        [:mac] ':3f' (100082) MAC 7f6ea:1b86ea (SEGA 00f SEGD 36 PGA 36e PGD 0370 NONX 0 WP 0)
        [:mac] ':3f' (100082): unmapped program memory write to 1B86EA = 00 & FF
        [:mac] ':3f' (100082) MAC 7f6eb:1b86eb (SEGA 00f SEGD 36 PGA 36e PGD 0370 NONX 0 WP 0)
        [:mac] ':3f' (100082): unmapped program memory write to 1B86EB = 27 & FF
        [:mac] ':3f' (100084) MAC 7f6e8:1b86e8 (SEGA 00f SEGD 36 PGA 36e PGD 0370 NONX 0 WP 0)
        [:mac] ':3f' (100084): unmapped program memory write to 1B86E8 = 02 & FF
        [:mac] ':3f' (100084) MAC 7f6e9:1b86e9 (SEGA 00f SEGD 36 PGA 36e PGD 0370 NONX 0 WP 0)
        [:mac] ':3f' (100084): unmapped program memory write to 1B86E9 = 80 & FF

    - short/long reset (RSTBUT)
    - CIO
        - optimize timers!
        - port C, open drain output bit PC1 (RTC/NVRAM data)
    - hard disk
        - 4105 SASI interface card
        - SASI interface (scsibus.cpp)
    - connect RS-232 port A

*/

#include "emu.h"
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
	uint8_t data = 0;

	// card select pulse
	uint8_t cs = (m_cs7 << 7) | ((offset >> 5) & 0x3f);

	m_bus0i->write_cs(cs);
	m_bus0x->write_cs(cs);
	m_bus1->write_cs(cs);
	m_bus2->write_cs(cs);

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
				data &= m_bus0i->read_inp();
				data &= m_bus0x->read_inp();
			}
			else
			{
				data &= m_bus1->read_inp();
				data &= m_bus2->read_inp();
			}

			if (LOG) logerror("%s INP %02x: %02x\n", machine().describe_context(), cs, data);
			break;

		case STAT:
			if (m_bus0)
			{
				data &= m_bus0i->read_stat();
				data &= m_bus0x->read_stat();
			}
			else
			{
				data &= m_bus1->read_stat();
				data &= m_bus2->read_stat();
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
	uint8_t cs = (m_cs7 << 7) | ((offset >> 5) & 0x3f);

	m_bus0i->write_cs(cs);
	m_bus0x->write_cs(cs);
	m_bus1->write_cs(cs);
	m_bus2->write_cs(cs);

	switch ((offset >> 1) & 0x07)
	{
	case OUT:
		if (LOG) logerror("%s OUT %02x: %02x\n", machine().describe_context(), cs, data);

		if (m_bus0)
		{
			m_bus0i->write_out(data);
			m_bus0x->write_out(data);
		}
		else
		{
			m_bus1->write_out(data);
			m_bus2->write_out(data);
		}
		break;

	case C1:
		if (LOG) logerror("%s C1 %02x: %02x\n", machine().describe_context(), cs, data);

		if (m_bus0)
		{
			m_bus0i->write_c1(data);
			m_bus0x->write_c1(data);
		}
		else
		{
			m_bus1->write_c1(data);
			m_bus2->write_c1(data);
		}
		break;

	case C2:
		if (LOG) logerror("%s C2 %02x: %02x\n", machine().describe_context(), cs, data);

		if (m_bus0)
		{
			m_bus0i->write_c2(data);
			m_bus0x->write_c2(data);
		}
		else
		{
			m_bus1->write_c2(data);
			m_bus2->write_c2(data);
		}
		break;

	case C3:
		if (LOG) logerror("%s C3 %02x: %02x\n", machine().describe_context(), cs, data);

		if (m_bus0)
		{
			m_bus0i->write_c3(data);
			m_bus0x->write_c3(data);
		}
		else
		{
			m_bus1->write_c3(data);
			m_bus2->write_c3(data);
		}
		break;

	case C4:
		if (LOG) logerror("%s C4 %02x: %02x\n", machine().describe_context(), cs, data);

		if (m_bus0)
		{
			m_bus0i->write_c4(data);
			m_bus0x->write_c4(data);
		}
		else
		{
			m_bus1->write_c4(data);
			m_bus2->write_c4(data);
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
	floppy_image_device *floppy = nullptr;

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

	if (LOG) logerror("%s SPEC CONTR REG %u:%u\n", machine().describe_context(), data & 0x07, state);

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

void abc1600_state::abc1600_mem(address_map &map)
{
	map(0x00000, 0xfffff).m(ABC1600_MAC_TAG, FUNC(abc1600_mac_device::map));
}


//-------------------------------------------------
//  ADDRESS_MAP( mac_mem )
//-------------------------------------------------

void abc1600_state::mac_mem(address_map &map)
{
	map(0x000000, 0x0fffff).ram();
	map(0x100000, 0x17ffff).m(ABC1600_MOVER_TAG, FUNC(abc1600_mover_device::vram_map));
	map(0x1fe000, 0x1fefff).rw(FUNC(abc1600_state::bus_r), FUNC(abc1600_state::bus_w));
	map(0x1ff000, 0x1ff000).mirror(0xf9).rw(m_fdc, FUNC(fd1797_device::status_r), FUNC(fd1797_device::cmd_w));
	map(0x1ff002, 0x1ff002).mirror(0xf9).rw(m_fdc, FUNC(fd1797_device::track_r), FUNC(fd1797_device::track_w));
	map(0x1ff004, 0x1ff004).mirror(0xf9).rw(m_fdc, FUNC(fd1797_device::sector_r), FUNC(fd1797_device::sector_w));
	map(0x1ff006, 0x1ff006).mirror(0xf9).rw(m_fdc, FUNC(fd1797_device::data_r), FUNC(fd1797_device::data_w));
	map(0x1ff100, 0x1ff101).mirror(0xfe).m(ABC1600_MOVER_TAG, FUNC(abc1600_mover_device::crtc_map));
	map(0x1ff200, 0x1ff207).mirror(0xf8).rw(FUNC(abc1600_state::dart_r), FUNC(abc1600_state::dart_w));
	map(0x1ff300, 0x1ff300).mirror(0xff).rw(m_dma0, FUNC(z80dma_device::bus_r), FUNC(z80dma_device::bus_w));
	map(0x1ff400, 0x1ff400).mirror(0xff).rw(m_dma1, FUNC(z80dma_device::bus_r), FUNC(z80dma_device::bus_w));
	map(0x1ff500, 0x1ff500).mirror(0xff).rw(m_dma2, FUNC(z80dma_device::bus_r), FUNC(z80dma_device::bus_w));
	map(0x1ff600, 0x1ff607).mirror(0xf8).rw(FUNC(abc1600_state::scc_r), FUNC(abc1600_state::scc_w));
	map(0x1ff700, 0x1ff707).mirror(0xf8).rw(FUNC(abc1600_state::cio_r), FUNC(abc1600_state::cio_w));
	map(0x1ff800, 0x1ff8ff).m(ABC1600_MOVER_TAG, FUNC(abc1600_mover_device::iowr0_map));
	map(0x1ff900, 0x1ff9ff).m(ABC1600_MOVER_TAG, FUNC(abc1600_mover_device::iowr1_map));
	map(0x1ffa00, 0x1ffaff).m(ABC1600_MOVER_TAG, FUNC(abc1600_mover_device::iowr2_map));
	map(0x1ffb00, 0x1ffb00).mirror(0x7e).w(FUNC(abc1600_state::fw0_w));
	map(0x1ffb01, 0x1ffb01).mirror(0x7e).w(FUNC(abc1600_state::fw1_w));
	map(0x1ffd00, 0x1ffd07).mirror(0xf8).w(ABC1600_MAC_TAG, FUNC(abc1600_mac_device::dmamap_w));
	map(0x1ffe00, 0x1ffe00).mirror(0xff).w(FUNC(abc1600_state::spec_contr_reg_w));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( abc1600 )
//-------------------------------------------------

static INPUT_PORTS_START( abc1600 )
	// inputs defined in machine/abc99.cpp
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
	return m_dart->ba_cd_r(A2_A1 ^ 0x03);
}

WRITE8_MEMBER( abc1600_state::dart_w )
{
	m_dart->ba_cd_w(A2_A1 ^ 0x03, data);
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

	uint8_t data = 0;

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

	uint8_t data = 0;

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

	uint8_t data = 0x0d;

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

static void abc1600_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

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

void abc1600_state::cpu_space_map(address_map &map)
{
	map(0xffff0, 0xfffff).m(m_maincpu, FUNC(m68008_device::autovectors_map));
	map(0xffff5, 0xffff5).lr8("cio int", [this]() -> u8 { return m_cio->intack_r(); });
	map(0xffffb, 0xffffb).lr8("dart int", [this]() -> u8 { return m_dart->m1_r(); });
	map(0xfffff, 0xfffff).lr8("nmi int", [this]() -> u8 { m_maincpu->set_input_line(M68K_IRQ_7, CLEAR_LINE); return m68008_device::autovector(7); });
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
//  machine_config( abc1600 )
//-------------------------------------------------

void abc1600_state::abc1600(machine_config &config)
{
	// basic machine hardware
	M68008(config, m_maincpu, 64_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &abc1600_state::abc1600_mem);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &abc1600_state::cpu_space_map);

	// video hardware
	ABC1600_MOVER(config, ABC1600_MOVER_TAG, 0);

	// devices
	abc1600_mac_device &mac(ABC1600_MAC(config, "mac", 0));
	mac.set_addrmap(AS_PROGRAM, &abc1600_state::mac_mem);
	mac.set_cpu_tag(m_maincpu);

	Z80DMA(config, m_dma0, 64_MHz_XTAL / 16);
	m_dma0->out_busreq_callback().set(FUNC(abc1600_state::dbrq_w));
	m_dma0->out_bao_callback().set(m_dma1, FUNC(z80dma_device::bai_w));
	m_dma0->in_mreq_callback().set(ABC1600_MAC_TAG, FUNC(abc1600_mac_device::dma0_mreq_r));
	m_dma0->out_mreq_callback().set(ABC1600_MAC_TAG, FUNC(abc1600_mac_device::dma0_mreq_w));
	m_dma0->in_iorq_callback().set(ABC1600_MAC_TAG, FUNC(abc1600_mac_device::dma0_iorq_r));
	m_dma0->out_iorq_callback().set(ABC1600_MAC_TAG, FUNC(abc1600_mac_device::dma0_iorq_w));

	Z80DMA(config, m_dma1, 64_MHz_XTAL / 16);
	m_dma1->out_busreq_callback().set(FUNC(abc1600_state::dbrq_w));
	m_dma1->out_bao_callback().set(m_dma2, FUNC(z80dma_device::bai_w));
	m_dma1->in_mreq_callback().set(ABC1600_MAC_TAG, FUNC(abc1600_mac_device::dma1_mreq_r));
	m_dma1->out_mreq_callback().set(ABC1600_MAC_TAG, FUNC(abc1600_mac_device::dma1_mreq_w));
	m_dma1->in_iorq_callback().set(ABC1600_MAC_TAG, FUNC(abc1600_mac_device::dma1_iorq_r));
	m_dma1->out_iorq_callback().set(ABC1600_MAC_TAG, FUNC(abc1600_mac_device::dma1_iorq_w));

	Z80DMA(config, m_dma2, 64_MHz_XTAL / 16);
	m_dma2->out_busreq_callback().set(FUNC(abc1600_state::dbrq_w));
	m_dma2->in_mreq_callback().set(ABC1600_MAC_TAG, FUNC(abc1600_mac_device::dma2_mreq_r));
	m_dma2->out_mreq_callback().set(ABC1600_MAC_TAG, FUNC(abc1600_mac_device::dma2_mreq_w));
	m_dma2->in_iorq_callback().set(ABC1600_MAC_TAG, FUNC(abc1600_mac_device::dma2_iorq_r));
	m_dma2->out_iorq_callback().set(ABC1600_MAC_TAG, FUNC(abc1600_mac_device::dma2_iorq_w));

	Z80DART(config, m_dart, 64_MHz_XTAL / 16);
	m_dart->out_txda_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	m_dart->out_dtra_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_dtr));
	m_dart->out_rtsa_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_rts));
	m_dart->out_txdb_callback().set(ABC_KEYBOARD_PORT_TAG, FUNC(abc_keyboard_port_device::txd_w));
	m_dart->out_int_callback().set_inputline(m_maincpu, M68K_IRQ_5);    // shared with SCC

	SCC8530(config, m_scc, 64_MHz_XTAL / 16);
	m_scc->intrq_callback().set_inputline(MC68008P8_TAG, M68K_IRQ_5);

	Z8536(config, m_cio, 64_MHz_XTAL / 16);
	m_cio->irq_wr_cb().set_inputline(MC68008P8_TAG, M68K_IRQ_2);
	m_cio->pa_rd_cb().set(FUNC(abc1600_state::cio_pa_r));
	m_cio->pb_rd_cb().set(FUNC(abc1600_state::cio_pb_r));
	m_cio->pb_wr_cb().set(FUNC(abc1600_state::cio_pb_w));
	m_cio->pc_rd_cb().set(FUNC(abc1600_state::cio_pc_r));
	m_cio->pc_wr_cb().set(FUNC(abc1600_state::cio_pc_w));

	NMC9306(config, m_nvram, 0);

	E0516(config, E050_C16PC_TAG, 32.768_kHz_XTAL);

	FD1797(config, m_fdc, 64_MHz_XTAL / 64);
	m_fdc->intrq_wr_callback().set(m_cio, FUNC(z8536_device::pb7_w));
	m_fdc->drq_wr_callback().set(FUNC(abc1600_state::fdc_drq_w));

	FLOPPY_CONNECTOR(config, SAB1797_02P_TAG":0", abc1600_floppies, nullptr, floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, SAB1797_02P_TAG":1", abc1600_floppies, nullptr, floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, SAB1797_02P_TAG":2", abc1600_floppies, "525qd", floppy_image_device::default_floppy_formats);

	RS232_PORT(config, RS232_A_TAG, default_rs232_devices, nullptr);

	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_dart, FUNC(z80dart_device::rxa_w));

	abc_keyboard_port_device &kb(ABC_KEYBOARD_PORT(config, ABC_KEYBOARD_PORT_TAG, abc_keyboard_devices, "abc99"));
	kb.out_rx_handler().set(m_dart, FUNC(z80dart_device::rxb_w));
	kb.out_trxc_handler().set(m_dart, FUNC(z80dart_device::rxtxcb_w));
	kb.out_keydown_handler().set(m_dart, FUNC(z80dart_device::dcdb_w));

	abcbus_slot_device &bus0i(ABCBUS_SLOT(config, "bus0i", 64_MHz_XTAL / 16, abc1600bus_cards, nullptr));
	bus0i.irq_callback().set(m_cio, FUNC(z8536_device::pa7_w));
	abcbus_slot_device &bus0x(ABCBUS_SLOT(config, "bus0x", 64_MHz_XTAL / 16, abc1600bus_cards, nullptr));
	bus0x.irq_callback().set(m_cio, FUNC(z8536_device::pa6_w));
	bus0x.nmi_callback().set(FUNC(abc1600_state::nmi_w));
	bus0x.xint2_callback().set(m_cio, FUNC(z8536_device::pa2_w));
	bus0x.xint3_callback().set(m_cio, FUNC(z8536_device::pa3_w));
	bus0x.xint4_callback().set(m_cio, FUNC(z8536_device::pa4_w));
	bus0x.xint5_callback().set(m_cio, FUNC(z8536_device::pa5_w));
	abcbus_slot_device &bus1(ABCBUS_SLOT(config, "bus1", 64_MHz_XTAL / 16, abc1600bus_cards, nullptr));
	bus1.irq_callback().set(m_cio, FUNC(z8536_device::pa1_w));
	abcbus_slot_device &bus2(ABCBUS_SLOT(config, "bus2", 64_MHz_XTAL / 16, abc1600bus_cards, "4105"));
	bus2.irq_callback().set(m_cio, FUNC(z8536_device::pa0_w));
	//bus2.pren_callback().set(Z8410AB1_2_TAG, FUNC(z80dma_device::iei_w));
	bus2.trrq_callback().set(Z8410AB1_2_TAG, FUNC(z80dma_device::rdy_w));

	// internal ram
	RAM(config, RAM_TAG).set_default_size("1M");

	// software list
	SOFTWARE_LIST(config, "flop_list").set_original("abc1600");
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( abc1600 )
//-------------------------------------------------

ROM_START( abc1600 )
	ROM_REGION( 0x71c, "plds", 0 )
	ROM_LOAD( "1020 6490349-01.8b",  0x104, 0x104, CRC(1fa065eb) SHA1(20a95940e39fa98e97e59ea1e548ac2e0c9a3444) ) // PAL16L8A expansion bus strobes
	ROM_LOAD( "1021 6490350-01.5d",  0x208, 0x104, CRC(96f6f44b) SHA1(12d1cd153dcc99d1c4a6c834122f370d49723674) ) // PAL16L8 interrupt encoder and ROM/RAM control
	ROM_LOAD( "1023 6490352-01.11e", 0x410, 0x104, CRC(a2f350ac) SHA1(77e08654a197080fa2111bc3031cd2c7699bf82b) ) // PAL16R6 interrupt acknowledge
	ROM_LOAD( "1024 6490353-01.12e", 0x514, 0x104, CRC(67f1328a) SHA1(b585495fe14a7ae2fbb29f722dca106d59325002) ) // PAL16R4 expansion bus timing and control
	ROM_LOAD( "1025 6490354-01.6e",  0x618, 0x104, CRC(9bda0468) SHA1(ad373995dcc18532274efad76fa80bd13c23df25) ) // PAL16R4 DMA transfer
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY  FULLNAME    FLAGS
COMP( 1985, abc1600, 0,      0,      abc1600, abc1600, abc1600_state, empty_init, "Luxor", "ABC 1600", MACHINE_NOT_WORKING )
