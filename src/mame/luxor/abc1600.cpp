// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Luxor ABC 1600

    How to create HDD image:
    ------------------------
    ./chdman createhd -chs 615,4,17 -ss 512 -o necd5126a.chd
    ./chdman createhd -chs 1024,8,17 -ss 512 -o micr1325a.chd

    How to format HDD:
    ------------------
    mf(2,0)
    mf(2,0)
    sas/format/format
    sa(40,0)
    y
    5
    micr1325a

    How to install OS:
    ------------------
    mf(2,0)
    mf(2,0)
    abcenix
    loadsys1
    <enter>
    <enter>

    ABCenix <= D-NIX <= AT&T Unix System V

*/

/*

    TODO:

    - abcenix panics while booting after commit 78661e9aa92c7e43c9a96039e7dfb3dabc79a287
    - systest1600 failures
        - CIO timer
        - RTC (seconds advance too slowly)
        - DMA
    - bootpar writes console bauds without high order byte (9600=>128)
    - loadsys1 core dump (/etc/mkfs -b 1024 -v 69000 /dev/sa40)
    - crashes after reset
    - CIO
        - optimize timers!
    - connect RS-232 printer port
    - Z80 SCC/DART interrupt chain
    - [:2a:chb] - TX FIFO is full, discarding data
        [:] SCC write 000003
        [:2a:chb] void z80scc_channel::data_write(uint8_t): Data Register Write: 17 ' '

*/

#include "emu.h"
#include "abc1600.h"

#include "machine/74259.h"
#include "softlist_dev.h"


//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define VERBOSE 0
#include "logmacro.h"


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

uint8_t abc1600_state::bus_r(offs_t offset)
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

			LOG("%s EXP %02x: %02x\n", machine().describe_context(), cs, data);
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

			LOG("%s RCSB %02x\n", machine().describe_context(), data);
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

			LOG("%s INP %02x: %02x\n", machine().describe_context(), cs, data);
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

			LOG("%s STAT %02x: %02x\n", machine().describe_context(), cs, data);
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

			LOG("%s OPS %02x: %02x\n", machine().describe_context(), cs, data);
			break;

		default:
			LOG("%s Unmapped read from virtual I/O %06x\n", machine().describe_context(), offset);
		}
	}

	return data;
}


//-------------------------------------------------
//  bus_w -
//-------------------------------------------------

void abc1600_state::bus_w(offs_t offset, uint8_t data)
{
	uint8_t cs = (m_cs7 << 7) | ((offset >> 5) & 0x3f);

	m_bus0i->write_cs(cs);
	m_bus0x->write_cs(cs);
	m_bus1->write_cs(cs);
	m_bus2->write_cs(cs);

	switch ((offset >> 1) & 0x07)
	{
	case OUT:
		LOG("%s OUT %02x: %02x\n", machine().describe_context(), cs, data);

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
		LOG("%s C1 %02x: %02x\n", machine().describe_context(), cs, data);

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
		LOG("%s C2 %02x: %02x\n", machine().describe_context(), cs, data);

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
		LOG("%s C3 %02x: %02x\n", machine().describe_context(), cs, data);

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
		LOG("%s C4 %02x: %02x\n", machine().describe_context(), cs, data);

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
		LOG("%s Unmapped write %02x to virtual I/O %06x\n", machine().describe_context(), data, offset);
	}
}


//-------------------------------------------------
//  fw0_w -
//-------------------------------------------------

void abc1600_state::fw0_w(uint8_t data)
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

	LOG("%s FW0 %02x\n", machine().describe_context(), data);

	// drive select
	floppy_image_device *floppy = nullptr;

	for (int n = 0; n < 3; n++)
		if (BIT(data, n))
			floppy = m_floppy[n]->get_device();

	m_fdc->set_floppy(floppy);

	// floppy motor
	if (floppy) floppy->mon_w(!BIT(data, 3));
}


//-------------------------------------------------
//  fw1_w -
//-------------------------------------------------

void abc1600_state::fw1_w(uint8_t data)
{
	/*

	    bit     description

	    0       MR (FD1797)
	    1       DDEN (FD1797, 9229B)
	    2       HLT (FD1797)
	    3       MINI (9229B)
	    4       HLD (9229B)
	    5       P0 (9229B)
	    6       P1 (9229B)
	    7       P2 (9229B)

	*/

	LOG("%s FW1 %02x\n", machine().describe_context(), data);

	// FDC master reset
	m_fdc->mr_w(BIT(data, 0));

	// density select
	m_fdc->dden_w(BIT(data, 1));
}


//-------------------------------------------------
//  cs7_w - CS7 output handler
//-------------------------------------------------

void abc1600_state::cs7_w(int state)
{
	LOG("%s CS7 %d\n", machine().describe_context(), state);

	m_cs7 = state;
}


//-------------------------------------------------
//  btce_w - _BTCE output handler
//-------------------------------------------------

void abc1600_state::btce_w(int state)
{
	LOG("%s _BTCE %d\n", machine().describe_context(), state);

	m_btce = state;
}


//-------------------------------------------------
//  atce_w - _ATCE output handler
//-------------------------------------------------

void abc1600_state::atce_w(int state)
{
	LOG("%s _ATCE %d\n", machine().describe_context(), state);

	m_atce = state;
}


//-------------------------------------------------
//  dmadis_w - _DMADIS output handler
//-------------------------------------------------

void abc1600_state::dmadis_w(int state)
{
	LOG("%s _DMADIS %d\n", machine().describe_context(), state);

	m_dmadis = state;
}


//-------------------------------------------------
//  sysscc_w - SYSSCC output handler
//-------------------------------------------------

void abc1600_state::sysscc_w(int state)
{
	LOG("%s SYSSCC %d\n", machine().describe_context(), state);

	m_sysscc = state;

	m_cio->pb5_w(!state);

	update_drdy1(0);
}


//-------------------------------------------------
//  sysfs_w - SYSFS output handler
//-------------------------------------------------

void abc1600_state::sysfs_w(int state)
{
	LOG("%s SYSFS %d\n", machine().describe_context(), state);

	m_sysfs = state;

	m_cio->pb6_w(!state);

	update_drdy0(0);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( abc1600_mem )
//-------------------------------------------------

void abc1600_state::abc1600_mem(address_map &map)
{
	map(0x00000, 0xfffff).rw(ABC1600_MAC_TAG, FUNC(abc1600_mac_device::read), FUNC(abc1600_mac_device::write));
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
	map(0x1ff300, 0x1ff300).mirror(0xff).rw(m_dma0, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x1ff400, 0x1ff400).mirror(0xff).rw(m_dma1, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x1ff500, 0x1ff500).mirror(0xff).rw(m_dma2, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x1ff600, 0x1ff607).mirror(0xf8).rw(FUNC(abc1600_state::scc_r), FUNC(abc1600_state::scc_w));
	map(0x1ff700, 0x1ff707).mirror(0xf8).rw(FUNC(abc1600_state::cio_r), FUNC(abc1600_state::cio_w));
	map(0x1ff800, 0x1ff8ff).m(ABC1600_MOVER_TAG, FUNC(abc1600_mover_device::iowr0_map));
	map(0x1ff900, 0x1ff9ff).m(ABC1600_MOVER_TAG, FUNC(abc1600_mover_device::iowr1_map));
	map(0x1ffa00, 0x1ffaff).m(ABC1600_MOVER_TAG, FUNC(abc1600_mover_device::iowr2_map));
	map(0x1ffb00, 0x1ffb00).mirror(0x7e).w(FUNC(abc1600_state::fw0_w));
	map(0x1ffb01, 0x1ffb01).mirror(0x7e).w(FUNC(abc1600_state::fw1_w));
	map(0x1ffd00, 0x1ffd07).mirror(0xf8).w(ABC1600_MAC_TAG, FUNC(abc1600_mac_device::dmamap_w));
	map(0x1ffe00, 0x1ffe00).mirror(0xff).w("spec_contr_reg", FUNC(ls259_device::write_nibble_d3));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( reset )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( abc1600_state::reset )
{
	if (!oldval && newval)
	{
		machine_reset();
	}

	m_mac->rstbut_w(newval);
}


//-------------------------------------------------
//  INPUT_PORTS( abc1600 )
//-------------------------------------------------

static INPUT_PORTS_START( abc1600 )
	// keyboard inputs defined in machine/abc99.cpp

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(abc1600_state::reset), 0)
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  Z80DMA 0
//-------------------------------------------------

void abc1600_state::update_br()
{
	// _BR = !_DMADIS || (_DBRQ0 || _DBRQ1 || _DBRQ2)
	// _IOC = IORQ delayed by 1 clock, or IORQ preceded by 1 clock on MINT2 or MINT5 
	// _BGACK = !_BR && !_BG && _IOC
	// DMA0.BAI = _BGACK

	// workaround for floppy DMA, this should use the 68000 BR line instead
	m_dma0->bai_w(!m_dmadis || m_dbrq0 || m_dbrq1 || m_dbrq2);
}

void abc1600_state::update_pren0(int state)
{
	if (m_sysfs)
	{
		// floppy
		m_dma0->iei_w(0);
	}
	else
	{
		// BUS0I/BUS0X
		bool pren0 = m_bus0i->pren_r() && m_bus0x->pren_r();

		m_dma0->iei_w(!pren0);
	}
}

void abc1600_state::update_drdy0(int state)
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


//-------------------------------------------------
//  Z80DMA 1
//-------------------------------------------------

void abc1600_state::update_pren1(int state)
{
	if (m_sysscc)
	{
		// SCC
		m_dma1->iei_w(1);
	}
	else
	{
		// BUS1
		m_dma1->iei_w(!m_bus1->pren_r());
	}
}

void abc1600_state::update_drdy1(int state)
{
	if (m_sysscc)
	{
		// SCC
		m_dma1->rdy_w(m_sccrq_a && m_sccrq_b);
	}
	else
	{
		// BUS1
		m_dma1->rdy_w(m_bus1->trrq_r());
	}
}

//-------------------------------------------------
//  Z80DART
//-------------------------------------------------

uint8_t abc1600_state::dart_r(offs_t offset)
{
	return m_dart->ba_cd_r(A2_A1 ^ 0x03);
}

void abc1600_state::dart_w(offs_t offset, uint8_t data)
{
	m_dart->ba_cd_w(A2_A1 ^ 0x03, data);
}

//-------------------------------------------------
//  SCC8530
//-------------------------------------------------

uint8_t abc1600_state::scc_r(offs_t offset)
{
	return m_scc->ab_dc_r(A2_A1);
}

void abc1600_state::scc_w(offs_t offset, uint8_t data)
{
	m_scc->ab_dc_w(A2_A1, data);
}


//-------------------------------------------------
//  Z8536
//-------------------------------------------------

uint8_t abc1600_state::cio_r(offs_t offset)
{
	return m_cio->read(A2_A1);
}

void abc1600_state::cio_w(offs_t offset, uint8_t data)
{
	m_cio->write(A2_A1, data);
}

uint8_t abc1600_state::cio_pa_r()
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

uint8_t abc1600_state::cio_pb_r()
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

void abc1600_state::cio_pb_w(uint8_t data)
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

uint8_t abc1600_state::cio_pc_r()
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

void abc1600_state::cio_pc_w(uint8_t data)
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

	LOG("CLK %u DATA %u RTC %u NVRAM %u\n", clock, data_out, rtc_cs, nvram_cs);

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

void abc1600_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ABC1600_FORMAT);
}



//-------------------------------------------------
//  ABC1600BUS_INTERFACE( abcbus_intf )
//-------------------------------------------------

void abc1600_state::nmi_w(int state)
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
	map(0xffff5, 0xffff5).lr8(NAME([this]() -> u8 { return m_cio->intack_r(); }));
	map(0xffffb, 0xffffb).lr8(NAME([this]() -> u8 { return m_dart->m1_r(); }));
	map(0xfffff, 0xfffff).lr8(NAME([this]() -> u8 { m_maincpu->set_input_line(M68K_IRQ_7, CLEAR_LINE); return m68008_device::autovector(7); }));
}

void abc1600_state::machine_start()
{
	// state saving
	save_item(NAME(m_dmadis));
	save_item(NAME(m_sysscc));
	save_item(NAME(m_sysfs));
	save_item(NAME(m_dbrq0));
	save_item(NAME(m_dbrq1));
	save_item(NAME(m_dbrq2));
	save_item(NAME(m_cs7));
	save_item(NAME(m_bus0));
	save_item(NAME(m_csb));
	save_item(NAME(m_atce));
	save_item(NAME(m_btce));
	save_item(NAME(m_sccrq_a));
	save_item(NAME(m_sccrq_b));
	save_item(NAME(m_scc_irq));
	save_item(NAME(m_dart_irq));
}


void abc1600_state::machine_reset()
{
	// clear floppy registers
	fw0_w(0);
	fw1_w(0);

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
	ABC1600_MAC(config, m_mac, 0);
	m_mac->set_addrmap(AS_PROGRAM, &abc1600_state::mac_mem);
	m_mac->set_cpu(m_maincpu);
	m_mac->in_tren0_cb().set(m_bus0i, FUNC(abcbus_slot_device::read_tren)); // TODO bus0x
	m_mac->out_tren0_cb().set(m_bus0i, FUNC(abcbus_slot_device::write_tren)); // TODO bus0x
	m_mac->in_tren1_cb().set(m_bus1, FUNC(abcbus_slot_device::read_tren));
	m_mac->out_tren1_cb().set(m_bus1, FUNC(abcbus_slot_device::write_tren));
	m_mac->in_tren2_cb().set(m_bus2, FUNC(abcbus_slot_device::read_tren));
	m_mac->out_tren2_cb().set(m_bus2, FUNC(abcbus_slot_device::write_tren));

	ls259_device &spec_contr_reg(LS259(config, "spec_contr_reg")); // Special Control Register @ 13E
	spec_contr_reg.q_out_cb<0>().set(FUNC(abc1600_state::cs7_w));
	spec_contr_reg.q_out_cb<2>().set(FUNC(abc1600_state::btce_w));
	spec_contr_reg.q_out_cb<3>().set(FUNC(abc1600_state::atce_w));
	spec_contr_reg.q_out_cb<4>().set(m_mac, FUNC(abc1600_mac_device::partst_w));
	spec_contr_reg.q_out_cb<5>().set(FUNC(abc1600_state::dmadis_w));
	spec_contr_reg.q_out_cb<6>().set(FUNC(abc1600_state::sysscc_w));
	spec_contr_reg.q_out_cb<7>().set(FUNC(abc1600_state::sysfs_w));

	Z80DMA(config, m_dma0, 64_MHz_XTAL / 16);
	m_dma0->out_busreq_callback().set(FUNC(abc1600_state::dbrq0_w));
	m_dma0->out_bao_callback().set(m_dma1, FUNC(z80dma_device::bai_w));
	m_dma0->in_mreq_callback().set(m_mac, FUNC(abc1600_mac_device::dma0_mreq_r));
	m_dma0->out_mreq_callback().set(m_mac, FUNC(abc1600_mac_device::dma0_mreq_w));
	m_dma0->out_ieo_callback().set([this](int state) { m_bus0i->prac_w(state); m_bus0x->prac_w(state); }).exor(1);
	m_dma0->in_iorq_callback().set(m_mac, FUNC(abc1600_mac_device::dma0_iorq_r));
	m_dma0->out_iorq_callback().set(m_mac, FUNC(abc1600_mac_device::dma0_iorq_w));

	Z80DMA(config, m_dma1, 64_MHz_XTAL / 16);
	m_dma1->out_busreq_callback().set(FUNC(abc1600_state::dbrq1_w));
	m_dma1->out_bao_callback().set(m_dma2, FUNC(z80dma_device::bai_w));
	m_dma1->in_mreq_callback().set(m_mac, FUNC(abc1600_mac_device::dma1_mreq_r));
	m_dma1->out_mreq_callback().set(m_mac, FUNC(abc1600_mac_device::dma1_mreq_w));
	m_dma1->out_ieo_callback().set(m_bus1, FUNC(abcbus_slot_device::prac_w)).exor(1);
	m_dma1->in_iorq_callback().set(m_mac, FUNC(abc1600_mac_device::dma1_iorq_r));
	m_dma1->out_iorq_callback().set(m_mac, FUNC(abc1600_mac_device::dma1_iorq_w));

	Z80DMA(config, m_dma2, 64_MHz_XTAL / 16);
	m_dma2->out_busreq_callback().set(FUNC(abc1600_state::dbrq2_w));
	m_dma2->in_mreq_callback().set(m_mac, FUNC(abc1600_mac_device::dma2_mreq_r));
	m_dma2->out_mreq_callback().set(m_mac, FUNC(abc1600_mac_device::dma2_mreq_w));
	m_dma2->out_ieo_callback().set(m_bus2, FUNC(abcbus_slot_device::prac_w)).exor(1);
	m_dma2->in_iorq_callback().set(m_mac, FUNC(abc1600_mac_device::dma2_iorq_r));
	m_dma2->out_iorq_callback().set(m_mac, FUNC(abc1600_mac_device::dma2_iorq_w));

	Z80DART(config, m_dart, 64_MHz_XTAL / 16);
	m_dart->out_int_callback().set(FUNC(abc1600_state::dart_irq_w));
	m_dart->out_txda_callback().set(RS232_PR_TAG, FUNC(rs232_port_device::write_txd));
	//m_dart->out_dtra_callback().set(RS232_PR_TAG, FUNC(rs232_port_device::write_dcd));
	//m_dart->out_rtsa_callback().set(RS232_PR_TAG, FUNC(rs232_port_device::write_cts));
	m_dart->out_txdb_callback().set(ABC_KEYBOARD_PORT_TAG, FUNC(abc_keyboard_port_device::txd_w));

	abc_keyboard_port_device &kb(ABC_KEYBOARD_PORT(config, ABC_KEYBOARD_PORT_TAG, abc_keyboard_devices, "abc99"));
	kb.out_rx_handler().set(m_dart, FUNC(z80dart_device::rxb_w));
	kb.out_trxc_handler().set(m_dart, FUNC(z80dart_device::rxtxcb_w));
	kb.out_keydown_handler().set(m_dart, FUNC(z80dart_device::dcdb_w));

	rs232_port_device &rs232pr(RS232_PORT(config, RS232_PR_TAG, default_rs232_devices, nullptr));
	rs232pr.rxd_handler().set(m_dart, FUNC(z80dart_device::rxa_w));
	//rs232pr.rts_handler().set(m_dart, FUNC(z80dart_device::ctsa_w));
	//rs232pr.dtr_handler().set(m_dart, FUNC(z80dart_device::dcda_w));

	SCC8530(config, m_scc, 64_MHz_XTAL / 16);
	m_scc->out_int_callback().set(FUNC(abc1600_state::scc_irq_w));
	m_scc->out_wreqa_callback().set(FUNC(abc1600_state::sccrq_a_w));
	m_scc->out_wreqb_callback().set(FUNC(abc1600_state::sccrq_b_w));
	m_scc->out_txda_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_txd));
	m_scc->out_dtra_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_dtr));
	m_scc->out_rtsa_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_rts));
	m_scc->out_txdb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	m_scc->out_dtrb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_dtr));
	m_scc->out_rtsb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232a(RS232_PORT(config, RS232_A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc, FUNC(scc8530_device::rxa_w));
	rs232a.cts_handler().set(m_scc, FUNC(scc8530_device::ctsa_w));
	rs232a.dcd_handler().set(m_scc, FUNC(scc8530_device::dcda_w));
	rs232a.ri_handler().set(m_scc, FUNC(scc8530_device::synca_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_scc, FUNC(scc8530_device::rxb_w));
	rs232b.cts_handler().set(m_scc, FUNC(scc8530_device::ctsb_w));
	rs232b.dcd_handler().set(m_scc, FUNC(scc8530_device::dcdb_w));
	rs232b.ri_handler().set(m_scc, FUNC(scc8530_device::syncb_w));

	Z8536(config, m_cio, 64_MHz_XTAL / 16);
	m_cio->irq_wr_cb().set_inputline(MC68008P8_TAG, M68K_IRQ_2);
	m_cio->pa_rd_cb().set(FUNC(abc1600_state::cio_pa_r));
	m_cio->pb_rd_cb().set(FUNC(abc1600_state::cio_pb_r));
	m_cio->pb_wr_cb().set(FUNC(abc1600_state::cio_pb_w));
	m_cio->pc_rd_cb().set(FUNC(abc1600_state::cio_pc_r));
	m_cio->pc_wr_cb().set(FUNC(abc1600_state::cio_pc_w));

	NMC9306(config, m_nvram, 0);

	E0516(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->outsel_rd_cb().set_constant(0);

	FD1797(config, m_fdc, 64_MHz_XTAL / 64); // clocked by 9229B
	m_fdc->intrq_wr_callback().set(m_cio, FUNC(z8536_device::pb7_w));
	m_fdc->drq_wr_callback().set(FUNC(abc1600_state::update_drdy0));

	FLOPPY_CONNECTOR(config, m_floppy[0], abc1600_floppies, nullptr, abc1600_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], abc1600_floppies, nullptr, abc1600_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[2], abc1600_floppies, "525qd", abc1600_state::floppy_formats).enable_sound(true);

	ABCBUS_SLOT(config, m_bus0i, 64_MHz_XTAL / 16, abc1600bus_cards, nullptr);
	m_bus0i->irq_callback().set(m_cio, FUNC(z8536_device::pa7_w));
	m_bus0i->pren_callback().set(FUNC(abc1600_state::update_pren0));
	m_bus0i->trrq_callback().set(FUNC(abc1600_state::update_drdy0));

	ABCBUS_SLOT(config, m_bus0x, 64_MHz_XTAL / 16, abc1600bus_cards, nullptr);
	m_bus0x->irq_callback().set(m_cio, FUNC(z8536_device::pa6_w));
	m_bus0x->nmi_callback().set(FUNC(abc1600_state::nmi_w));
	m_bus0x->xint2_callback().set(m_cio, FUNC(z8536_device::pa2_w));
	m_bus0x->xint3_callback().set(m_cio, FUNC(z8536_device::pa3_w));
	m_bus0x->xint4_callback().set(m_cio, FUNC(z8536_device::pa4_w));
	m_bus0x->xint5_callback().set(m_cio, FUNC(z8536_device::pa5_w));
	m_bus0x->pren_callback().set(FUNC(abc1600_state::update_pren0));
	m_bus0x->trrq_callback().set(FUNC(abc1600_state::update_drdy0));

	ABCBUS_SLOT(config, m_bus1, 64_MHz_XTAL / 16, abc1600bus_cards, nullptr);
	m_bus1->irq_callback().set(m_cio, FUNC(z8536_device::pa1_w));
	m_bus1->pren_callback().set(FUNC(abc1600_state::update_pren1));
	m_bus1->trrq_callback().set(FUNC(abc1600_state::update_drdy1));

	ABCBUS_SLOT(config, m_bus2, 64_MHz_XTAL / 16, abc1600bus_cards, "4105");
	m_bus2->irq_callback().set(m_cio, FUNC(z8536_device::pa0_w));
	m_bus2->pren_callback().set(m_dma2, FUNC(z80dma_device::iei_w)).exor(1);
	m_bus2->trrq_callback().set(m_dma2, FUNC(z80dma_device::rdy_w));

	// internal ram
	RAM(config, RAM_TAG).set_default_size("1M");

	// software list
	SOFTWARE_LIST(config, "flop_list").set_original("abc1600_flop");
	SOFTWARE_LIST(config, "hdd_list").set_original("abc1600_hdd");
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( abc1600 )
//-------------------------------------------------

ROM_START( abc1600 )
	ROM_REGION( 0x860, "plds", 0 )
	ROM_LOAD( "1020 6490349-01.8b",  0x104, 0x104, CRC(1fa065eb) SHA1(20a95940e39fa98e97e59ea1e548ac2e0c9a3444) ) // BUS INTERFACE X35
	ROM_LOAD( "1021 6490350-01.5d",  0x208, 0x104, CRC(96f6f44b) SHA1(12d1cd153dcc99d1c4a6c834122f370d49723674) ) // X36 BOOT DECODE, MAP ACK PLUS INT PRIO
	ROM_LOAD( "1023 6490352-01.11e", 0x410, 0x104, CRC(a2f350ac) SHA1(77e08654a197080fa2111bc3031cd2c7699bf82b) ) // X36 IO INTERFACE STROBE HANDLER
	ROM_LOAD( "1024 6490353-01.12e", 0x514, 0x104, CRC(67f1328a) SHA1(b585495fe14a7ae2fbb29f722dca106d59325002) ) // X36 CHANNEL SELECT TIMER
	ROM_LOAD( "1025 6490354-01.6e",  0x618, 0x104, CRC(9bda0468) SHA1(ad373995dcc18532274efad76fa80bd13c23df25) ) // X36 Z80A-DMA INTERFACER
	ROM_LOAD( "1031", 0x71c, 0x144, CRC(0aedc9fc) SHA1(2cbbc7d5cb16b410d296062feb77ed26ff01af24) ) // NS32081 IN ABC1600

	ROM_REGION( 0x20, NMC9306_TAG, 0 )
	ROM_LOAD( "nmc9306.14c", 0x00, 0x20, CRC(1cb59b6e) SHA1(3c955a667034db86fa1b848f0c0317157a3a48f6) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY  FULLNAME    FLAGS
COMP( 1985, abc1600, 0,      0,      abc1600, abc1600, abc1600_state, empty_init, "Luxor", "ABC 1600", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
