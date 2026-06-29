// license:BSD-3-Clause
// copyright-holders:A. Lenard
/***************************************************************************

    System 8000 CPU-A & HPCPU boards

=========================================================
				System I/O Space Memory Map
=========================================================
I/O Space      Device Class            Current use
=========================================================
0000 - 003F    Memory Devices          ECC 0000 - 0003
0040 - 007F    Tape                    TCC 0040 - 004F
0080 - 0FFF    Other Memory Devices    None
1000 - 1FFF    Tape Devices            9-Track 1000 - 101F
2000 - 6FFF    Reserved                FPP 2000 - 200F
7000 - 8FFF    Disk Devices
				- SMDC                 7F00 - 7F0F
				- WDC/mWDC             8000 - 80FF
9000 - 9FFF    Available to User
A000 - DFFF    Reserved
E000 - FFFF    Communication Devices
				- CPU                  FFC1 - FFFF
				- ICP                  EF01 - EF0F
				- SSB                  FF41 - FF7F
				- SSB                  FF01 - FF3F
=========================================================

***************************************************************************/

#include "emu.h"

#include "s8k_cpu.h"

#include "bus/rs232/rs232.h"
#include "bus/centronics/ctronics.h"

//#define VERBOSE 1
#include "logmacro.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CLK_CPU		(44.444_MHz_XTAL / 8)	/* 5.5555 Mhz */
#define CLK_HPCPU	(CLK_CPU * 2)			/* 11.111 Mhz */
#define CLK_BUS		CLK_CPU
#define CLK_MASTER	(CLK_CPU * 4)
#define CLK_CTC		(2.4576_MHz_XTAL / 2)
#define CLK_SCC		((2.4576_MHz_XTAL).value())

// System Configuration Register control bits 0-3
#define SCR_BD_MEMON	0x01	// Use off-board memory (disable monitor ROM and local RAM)
#define SCR_MMU_ONH		0x02	// Enable MMUs (disable linear memory)
#define SCR_SEG_USER	0x04	// Segmented user space
#define SCR_CLR_PARITY	0x08	// Enable NMI from parity or ECC error

// Non-Maskable Interrupt reason codes
static constexpr uint16_t NMI_MANUAL 	= 0x0001;
static constexpr uint16_t NMI_PWRFAIL	= 0x0002;
static constexpr uint16_t NMI_ECCERR 	= 0x0004;

s8k_cpu_base::s8k_cpu_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_zbi_card_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_local_ram(*this, "local_ram", 0x800, ENDIANNESS_BIG)	// 2 KB
	, m_view_code(*this, "memview_code")
	, m_view_data(*this, "memview_data")
	, m_view_stck(*this, "memview_stck")
	, m_mmu_code(*this, "mmu0_code")
	, m_mmu_data(*this, "mmu1_data")
	, m_mmu_stck(*this, "mmu2_stck")
	, m_dipsw(*this, "DIPSW")
	, m_ns_cb(*this)
	, m_busack_cb(*this)
{
}

void s8k_cpu_base::base_device_start()
{
	install_memory();

	save_item(NAME(m_reg_snvr));
	save_item(NAME(m_reg_trpl));
	save_item(NAME(m_reg_if1l));
}

void s8k_cpu_base::base_device_reset()
{
	m_is_seg_user = false;

	m_reg_snvr = 0;
	m_reg_trpl = 0;
	m_reg_if1l = 0;

	m_view_code.select(0);
	m_view_data.select(0);
	m_view_stck.select(0);
}

void s8k_cpu_base::base_device_resolve_objects()
{
	m_bus->set_bus_clock(CLK_BUS);
	m_bus->assign_iospace_installer(&m_maincpu->space(AS_IO));

	m_maincpu->ns().append(*m_bus, FUNC(zbi_bus_device::ns_w));
	m_maincpu->busack().append(*m_bus, FUNC(zbi_bus_device::busack_w));
	m_maincpu->viack().set(*m_bus, FUNC(zbi_bus_device::viack_r));
}

//**************************************************************************
//  INTERRUPT HANDLING
//**************************************************************************

void s8k_cpu_base::segt_interrupt(int state)
{
	m_maincpu->set_input_line(z8001_device::SEGT_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

uint16_t s8k_cpu_base::segtack_r()
{
	uint16_t code = (m_mmu_code->segtack_r() |
					 m_mmu_data->segtack_r() |
					 m_mmu_stck->segtack_r());

	// Locally triggered?
	if (code == 0)
	{
		segt_interrupt(0);
	}

	LOG("%s SEGTRAP: %d\n", machine().describe_context(), code);

	return code;
}

uint16_t s8k_cpu_base::nmiack_r()
{
	uint16_t code = m_nmi_code;

	m_nmi_code = 0;

	if (code == 0)
	{
		code = m_bus->nmiack_r();
	}

	m_maincpu->set_input_line(z8001_device::NMI_LINE, CLEAR_LINE);

	return code;
}

void s8k_cpu_base::nmi_switch_w(int state)
{
	if (state & (m_nmi_code == 0))
	{
		m_nmi_code = NMI_MANUAL;
		card_nmi_w(ASSERT_LINE);
	}
}

void s8k_cpu_base::soft_reset_w(uint8_t dummy)
{
	machine().schedule_soft_reset();
}

z8010_device *s8k_cpu_base::select_code_mmu(offs_t offset)
{
	z8010_device *mmu = m_mmu_code;
	uint8_t seg = (uint8_t)(offset >> 16);

	LOG("%s CODE MMU SELECT, offset: %06x\n", machine().describe_context(), offset);

	if (m_is_seg_os)
	{
		if (m_is_seg_user && m_normal_mode)
		{
			if (seg < 64)
				mmu = m_mmu_data;
			else
				mmu = m_mmu_stck;
		}
	}
	else	// Non-seg OS
	{
		if ((seg & 0x3f) < 2)	// OS segs 0,1,64,65
		{
			if (m_normal_mode)	// Trying to access in normal mode?
			{
				// SEGTRAP!
				m_reg_snvr = seg;
				m_reg_if1l = offset;
				mmu = nullptr;
				segt_interrupt(1);
			}
		}
		else if (m_is_seg_user)
		{
			if (seg < 64)
				mmu = m_mmu_data;
			else
				mmu = m_mmu_stck;
		}
	}

	return mmu;
}

z8010_device *s8k_cpu_base::select_data_mmu(offs_t offset, uint8_t sbr, uint8_t nbr)
{
	z8010_device *mmu = m_mmu_stck;
	uint8_t seg = (uint8_t)(offset >> 16);
	uint8_t seg_offs = (uint8_t)(offset >> 8);

	LOG("%s DATA MMU SELECT, offset: %06x\n", machine().describe_context(), offset);

	if (m_is_seg_os)
	{
		if (!m_normal_mode)	// System mode
		{
			mmu = m_mmu_code;
		}
		else if (m_is_seg_user)
		{
			if (seg < 64)
			{
				mmu = m_mmu_data;
			}
		}
		else if (seg_offs < nbr)
		{
			mmu = m_mmu_data;
		}
	}
	else	// Non-seg OS
	{
		if ((seg & 0x3f) < 2)	// OS segs 0,1,64,65
		{
			if (m_normal_mode)	// Trying to access in normal mode?
			{
				// SEGTRAP!
				m_reg_snvr = seg;
				m_reg_trpl = offset;
				mmu = nullptr;
				segt_interrupt(1);
			}
			else if (seg_offs < sbr)
			{
				mmu = m_mmu_data;
			}
		}
		else if (m_is_seg_user)
		{
			if (seg < 64)
			{
				mmu = m_mmu_data;
			}
		}
		else if (seg_offs < nbr)
		{
			mmu = m_mmu_data;
		}
	}

	return mmu;
}

uint16_t s8k_cpu_base::ram_r(address_space &space, offs_t offset, uint16_t mask)
{
	uint16_t data = 0;
	if (translate_addr(space.spacenum(), false, offset))
		data = m_bus->ram16_r(offset, mask);
	else if (space.spacenum() == AS_PROGRAM)
		data = 0x8d07;	// NOP instruction

	return data;
}

void s8k_cpu_base::ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mask)
{
	if (translate_addr(space.spacenum(), true, offset))
		m_bus->ram16_w(offset, data, mask);
}

// NOTE: Stack address space is only used for calling m_ctc[0]->trg3()
void s8k_cpu_base::install_memory(offs_t lrom_end, offs_t lram_start, offs_t lmem_end)
{
	void *LROM, *LRAM;

	address_space& pspace = m_maincpu->space(AS_PROGRAM);
	address_space& dspace = m_maincpu->space(AS_DATA);
	address_space& sspace = m_maincpu->space(z8001_device::AS_STACK);

	// Install handlers over entire address space
	pspace.install_readwrite_handler(0x00'0000, 0x7f'ffff,
		read16_delegate(*this, FUNC(s8k_cpu_base::ram_r)), write16_delegate(*this, FUNC(s8k_cpu_base::ram_w)));
	dspace.install_readwrite_handler(0x00'0000, 0x7f'ffff,
		read16_delegate(*this, FUNC(s8k_cpu_base::ram_r)), write16_delegate(*this, FUNC(s8k_cpu_base::ram_w)));
	sspace.install_readwrite_handler(0x00'0000, 0x7f'ffff,
		read16_delegate(*this, FUNC(s8k_cpu_base::ram_r)), write16_delegate(*this, FUNC(s8k_cpu_base::ram_w)));

	pspace.install_view(0x0000, lmem_end, m_view_code);
	dspace.install_view(0x0000, lmem_end, m_view_data);
	sspace.install_view(0x0000, lmem_end, m_view_stck);

	LROM = memregion("maincpu")->base();
	LRAM = memshare("local_ram")->ptr();
	m_view_code[0].install_rom(0x0000, lrom_end, LROM);
	m_view_data[0].install_rom(0x0000, lrom_end, LROM);
	m_view_data[0].install_ram(lram_start, lram_start + 0x7ff, LRAM);
	m_view_stck[0].install_ram(lram_start, lram_start + 0x7ff, LRAM);
}

uint8_t s8k_cpu_base::reg_snvr_r()
{
	return m_reg_snvr;
}

uint8_t s8k_cpu_base::reg_trpl_r()
{
	return m_reg_trpl;
}

uint8_t s8k_cpu_base::reg_if1l_r()
{
	return m_reg_if1l;
}

void s8k_cpu_base::out_ns_cb(int state)
{
	LOG("%s NORMAL/SYSTEM MODE CHANGE: %d\n", machine().describe_context(), state);

	m_normal_mode = state;
	m_ns_cb(state);
}

void s8k_cpu_base::out_busack_cb(int state)
{
	m_busack_cb(state);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  zbi_s8k_cpu10_card_device - constructor
//-------------------------------------------------

zbi_s8k_cpu10_card_device::zbi_s8k_cpu10_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: zbi_s8k_cpu10_card_device(mconfig, ZBI_S8K_CPU10, tag, owner, clock)
{
}

zbi_s8k_cpu10_card_device::zbi_s8k_cpu10_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: s8k_cpu_base(mconfig, type, tag, owner, clock)
	, m_sio(*this, "sio%u", 0U)
	, m_ctc(*this, "ctc%u", 0U)
	, m_pio(*this, "pio0")
	, m_segjp(*this, "SEGJP")
{
}

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************


uint16_t zbi_s8k_cpu10_card_device::reg_scr_r(offs_t offset, uint16_t mask)
{
	return (mask == 0xffff) ? swapendian_int16((uint16_t)(m_reg_scr)) : (uint16_t)(m_reg_scr);
}

void zbi_s8k_cpu10_card_device::reg_scr_w(uint16_t data)
{
	data >>= 8;
	uint8_t diff = data ^ m_reg_scr;

	LOG("%s reg_scr_w: %02x\n", machine().describe_context(), data);

	if (diff & SCR_BD_MEMON)
	{
		if (data & SCR_BD_MEMON)
		{
			m_view_code.disable();
			m_view_data.disable();
			m_view_stck.disable();
		}
		else
		{
			m_view_code.select(0);
			m_view_data.select(0);
			m_view_stck.select(0);
		}
	}

	if (diff & SCR_MMU_ONH)
	{
		m_maincpu->space(AS_PROGRAM).invalidate_caches(read_or_write::READWRITE);
		m_maincpu->space(AS_DATA).invalidate_caches(read_or_write::READWRITE);
		m_maincpu->space(z8001_device::AS_STACK).invalidate_caches(read_or_write::READWRITE);
	}

	if (diff & SCR_SEG_USER)
	{
		m_is_seg_user = !!(data & SCR_SEG_USER);
	}

	m_reg_scr = (m_reg_scr & 0xf0) | ( data & 0x0f );	// Mask off read-only nibble
}

uint8_t zbi_s8k_cpu10_card_device::reg_sbr_r()
{
	return m_reg_sbr;
}

void zbi_s8k_cpu10_card_device::reg_sbr_w(uint8_t data)
{
	LOG("%s reg_SBR_w: %02x\n", machine().describe_context(), data);
	m_reg_sbr = data;
}

uint8_t zbi_s8k_cpu10_card_device::reg_nbr_r()
{
	return m_reg_nbr;
}

void zbi_s8k_cpu10_card_device::reg_nbr_w(uint8_t data)
{
	LOG("%s reg_NBR_w: %02x\n", machine().describe_context(), data);
	m_reg_nbr = data;
}

uint8_t zbi_s8k_cpu10_card_device::comms_r(offs_t offset)
{
	if (!BIT(offset, 0))
	{
		offset >>= 1;

		switch (offset & 0xfc)
		{
		case 0x00: return m_sio[0]->cd_ba_r(offset);
		case 0x04: return m_sio[1]->cd_ba_r(offset);
		case 0x08: return m_sio[2]->cd_ba_r(offset);
		case 0x0c: return m_sio[3]->cd_ba_r(offset);
		case 0x10: return m_ctc[0]->read(offset);
		case 0x14: return m_ctc[1]->read(offset);
		case 0x18: return m_ctc[2]->read(offset);
		case 0x1c: return m_pio->read(offset);
		default:
			break;
		}
	}

	return 0xff;
}

void zbi_s8k_cpu10_card_device::comms_w(offs_t offset, uint8_t data)
{
	if (!BIT(offset, 0))
	{
		offset >>= 1;

		switch (offset & 0xfc)
		{
		case 0x00: m_sio[0]->cd_ba_w(offset, data); break;
		case 0x04: m_sio[1]->cd_ba_w(offset, data); break;
		case 0x08: m_sio[2]->cd_ba_w(offset, data); break;
		case 0x0c: m_sio[3]->cd_ba_w(offset, data); break;
		case 0x10: m_ctc[0]->write(offset, data); break;
		case 0x14: m_ctc[1]->write(offset, data); break;
		case 0x18: m_ctc[2]->write(offset, data); break;
		case 0x1c: m_pio->write(offset, data); break;
		default:
			break;
		}
	}
}

uint8_t zbi_s8k_cpu10_card_device::mmu_cmd_r(offs_t offset)
{
	if (!BIT(offset, 0))
	{
		uint8_t reg = (uint8_t)(offset >> 8);

		if (!BIT(offset, 1)) return m_mmu_code->read(reg);
		if (!BIT(offset, 2)) return m_mmu_data->read(reg);
		if (!BIT(offset, 3)) return m_mmu_stck->read(reg);
	}

	return 0xff;
}

void zbi_s8k_cpu10_card_device::mmu_cmd_w(offs_t offset, uint8_t data)
{
	if (!BIT(offset, 0))
	{
		uint8_t reg = (uint8_t)(offset >> 8);

		if (!BIT(offset, 1)) m_mmu_code->write(reg, data);
		if (!BIT(offset, 2)) m_mmu_data->write(reg, data);
		if (!BIT(offset, 3)) m_mmu_stck->write(reg, data);

		m_maincpu->space(AS_PROGRAM).invalidate_caches(read_or_write::READWRITE);
		m_maincpu->space(AS_DATA).invalidate_caches(read_or_write::READWRITE);
		m_maincpu->space(z8001_device::AS_STACK).invalidate_caches(read_or_write::READWRITE);
	}
}

void zbi_s8k_cpu10_card_device::addrmap_sio(address_map &map)
{
	s8k_cpu_base::addrmap_sio(map);
	map(0x00f0, 0x20fc).rw(FUNC(zbi_s8k_cpu10_card_device::mmu_cmd_r), FUNC(zbi_s8k_cpu10_card_device::mmu_cmd_w));
}

bool zbi_s8k_cpu10_card_device::translate_addr(int spacenum, bool write, offs_t &offset)
{
	bool stack_access = (spacenum == z8001_device::AS_STACK);

	offset <<= 1;

	if (stack_access)
	{
		m_ctc[0]->trg3(1);
		m_ctc[0]->trg3(0);
	}

	if (m_reg_scr & SCR_MMU_ONH)
	{
		bool code_access = (spacenum == AS_PROGRAM);
		z8010_device *mmu = code_access ?
							select_code_mmu(offset) : select_data_mmu(offset, m_reg_sbr, m_reg_nbr);

		if (mmu)
		{
			int st = code_access ?
						(m_maincpu->is_ifetch1() ?
							z8002_device::ST_IFETCH_1 :
							z8002_device::ST_IFETCH_N) :
						(stack_access ?
							z8002_device::ST_REQ_STACK :
							z8002_device::ST_REQ_DATA);

			LOG("%s MMU MEM REQ (space %d): %06x\n", machine().describe_context(), spacenum, offset);

			offset &= 0x3f'ffff;	// Mask off seg bit 7 to disable URS checking in MMUs

			return mmu->translate(offset, write, true, m_dma_on, st);
		}
	}

	return true;
}

void zbi_s8k_cpu10_card_device::install_memory()
{
	s8k_cpu_base::install_memory(0x1fff, 0x2000, 0x3fff);
}


//-------------------------------------------------
//  Z80PIO
//-------------------------------------------------

uint8_t zbi_s8k_cpu10_card_device::pa_data_r()
{
	/*

	Parallel Printer Connector (Centronics interface), Port A

	bit     signal      description

	0       STROBE      data strobe (out)
	1       -           not used
	2       -           not used
	3       -           not used
	4       BUSY        printer busy (in)
	5       PSEL        printer select (in)
	6       FAULT       printer fault (in)
	7       ACK         data acknowledge (in)

	*/

	uint8_t data = 0;

	data |= m_centronics_busy << 4;
	data |= m_centronics_select << 5;
	data |= m_centronics_fault << 6;
	data |= m_centronics_ack << 7;

	//logerror("%s pa_data_r: %02x\n", machine().describe_context(), data);
	return data;
}

void zbi_s8k_cpu10_card_device::centronics_busy_w(uint8_t data)
{
	m_centronics_busy = data;
}
void zbi_s8k_cpu10_card_device::centronics_select_w(uint8_t data)
{
	m_centronics_select = data;
}

void zbi_s8k_cpu10_card_device::centronics_fault_w(uint8_t data)
{
	m_centronics_fault = data;
}

void zbi_s8k_cpu10_card_device::centronics_ack_w(uint8_t data)
{
	m_centronics_ack = data;
}

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void zbi_s8k_cpu10_card_device::device_add_mconfig(machine_config &config)
{
	Z8001(config, m_maincpu, CLK_CPU);
	m_maincpu->set_m20_hack(false);
	m_maincpu->set_addrmap(AS_PROGRAM, &zbi_s8k_cpu10_card_device::addrmap_program);
	m_maincpu->set_addrmap(AS_DATA, &zbi_s8k_cpu10_card_device::addrmap_data);
	m_maincpu->set_addrmap(z8001_device::AS_STACK, &zbi_s8k_cpu10_card_device::addrmap_stack);
	m_maincpu->set_addrmap(AS_IO, &zbi_s8k_cpu10_card_device::addrmap_io);
	m_maincpu->set_addrmap(z8001_device::AS_SIO, &zbi_s8k_cpu10_card_device::addrmap_sio);
	m_maincpu->segtack().set(FUNC(zbi_s8k_cpu10_card_device::segtack_r));
	m_maincpu->nmiack().set(FUNC(zbi_s8k_cpu10_card_device::nmiack_r));
	m_maincpu->ns().set(FUNC(zbi_s8k_cpu10_card_device::out_ns_cb));
	m_maincpu->busack().set(FUNC(zbi_s8k_cpu10_card_device::out_busack_cb));

	Z8010(config, m_mmu_code, CLK_CPU);
	m_mmu_code->out_segt_cb().set(FUNC(zbi_s8k_cpu10_card_device::segt_interrupt));
	Z8010(config, m_mmu_data, CLK_CPU);
	m_mmu_data->out_segt_cb().set(FUNC(zbi_s8k_cpu10_card_device::segt_interrupt));
	Z8010(config, m_mmu_stck, CLK_CPU);
	m_mmu_stck->out_segt_cb().set(FUNC(zbi_s8k_cpu10_card_device::segt_interrupt));

	Z80SIO(config, m_sio[0], CLK_CPU);
	m_sio[0]->out_txda_callback().set("sio0:cha:tty0", FUNC(rs232_port_device::write_txd));
	m_sio[0]->out_rtsa_callback().set("sio0:cha:tty0", FUNC(rs232_port_device::write_rts));
	m_sio[0]->out_dtra_callback().set("sio0:cha:tty0", FUNC(rs232_port_device::write_dtr));
	m_sio[0]->out_txdb_callback().set("sio0:chb:console", FUNC(rs232_port_device::write_txd));
	m_sio[0]->out_rtsb_callback().set("sio0:chb:console", FUNC(rs232_port_device::write_rts));
	m_sio[0]->out_dtrb_callback().set("sio0:chb:console", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232_0(RS232_PORT(config, "sio0:cha:tty0", default_rs232_devices, nullptr));
	rs232_0.rxd_handler().set(m_sio[0], FUNC(z80sio_device::rxa_w));
	rs232_0.cts_handler().set(m_sio[0], FUNC(z80sio_device::ctsa_w));
	rs232_0.dcd_handler().set(m_sio[0], FUNC(z80sio_device::dcda_w));
	rs232_port_device &rs232_1(RS232_PORT(config, "sio0:chb:console", default_rs232_devices, "terminal"));
	rs232_1.rxd_handler().set(m_sio[0], FUNC(z80sio_device::rxb_w));
	rs232_1.cts_handler().set(m_sio[0], FUNC(z80sio_device::ctsb_w));
	rs232_1.dcd_handler().set(m_sio[0], FUNC(z80sio_device::dcdb_w));

	Z80SIO(config, m_sio[1], CLK_CPU);
	m_sio[1]->out_txda_callback().set("sio1:cha:tty2", FUNC(rs232_port_device::write_txd));
	m_sio[1]->out_rtsa_callback().set("sio1:cha:tty2", FUNC(rs232_port_device::write_rts));
	m_sio[1]->out_dtra_callback().set("sio1:cha:tty2", FUNC(rs232_port_device::write_dtr));
	m_sio[1]->out_txdb_callback().set("sio1:chb:tty3", FUNC(rs232_port_device::write_txd));
	m_sio[1]->out_rtsb_callback().set("sio1:chb:tty3", FUNC(rs232_port_device::write_rts));
	m_sio[1]->out_dtrb_callback().set("sio1:chb:tty3", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232_2(RS232_PORT(config, "sio1:cha:tty2", default_rs232_devices, nullptr));
	rs232_2.rxd_handler().set(m_sio[1], FUNC(z80sio_device::rxa_w));
	rs232_2.cts_handler().set(m_sio[1], FUNC(z80sio_device::ctsa_w));
	rs232_2.dcd_handler().set(m_sio[1], FUNC(z80sio_device::dcda_w));
	rs232_port_device &rs232_3(RS232_PORT(config, "sio1:chb:tty3", default_rs232_devices, nullptr));
	rs232_3.rxd_handler().set(m_sio[1], FUNC(z80sio_device::rxb_w));
	rs232_3.cts_handler().set(m_sio[1], FUNC(z80sio_device::ctsb_w));
	rs232_3.dcd_handler().set(m_sio[1], FUNC(z80sio_device::dcdb_w));

	Z80SIO(config, m_sio[2], CLK_CPU);
	m_sio[2]->out_txda_callback().set("sio2:cha:tty4", FUNC(rs232_port_device::write_txd));
	m_sio[2]->out_rtsa_callback().set("sio2:cha:tty4", FUNC(rs232_port_device::write_rts));
	m_sio[2]->out_dtra_callback().set("sio2:cha:tty4", FUNC(rs232_port_device::write_dtr));
	m_sio[2]->out_txdb_callback().set("sio2:chb:tty5", FUNC(rs232_port_device::write_txd));
	m_sio[2]->out_rtsb_callback().set("sio2:chb:tty5", FUNC(rs232_port_device::write_rts));
	m_sio[2]->out_dtrb_callback().set("sio2:chb:tty5", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232_4(RS232_PORT(config, "sio2:cha:tty4", default_rs232_devices, nullptr));
	rs232_4.rxd_handler().set(m_sio[2], FUNC(z80sio_device::rxa_w));
	rs232_4.cts_handler().set(m_sio[2], FUNC(z80sio_device::ctsa_w));
	rs232_4.dcd_handler().set(m_sio[2], FUNC(z80sio_device::dcda_w));
	rs232_port_device &rs232_5(RS232_PORT(config, "sio2:chb:tty5", default_rs232_devices, nullptr));
	rs232_5.rxd_handler().set(m_sio[2], FUNC(z80sio_device::rxb_w));
	rs232_5.cts_handler().set(m_sio[2], FUNC(z80sio_device::ctsb_w));
	rs232_5.dcd_handler().set(m_sio[2], FUNC(z80sio_device::dcdb_w));

	Z80SIO(config, m_sio[3], CLK_CPU);
	m_sio[3]->out_txda_callback().set("sio3:cha:tty6", FUNC(rs232_port_device::write_txd));
	m_sio[3]->out_rtsa_callback().set("sio3:cha:tty6", FUNC(rs232_port_device::write_rts));
	m_sio[3]->out_dtra_callback().set("sio3:cha:tty6", FUNC(rs232_port_device::write_dtr));
	m_sio[3]->out_txdb_callback().set("sio3:chb:tty7", FUNC(rs232_port_device::write_txd));
	m_sio[3]->out_rtsb_callback().set("sio3:chb:tty7", FUNC(rs232_port_device::write_rts));
	m_sio[3]->out_dtrb_callback().set("sio3:chb:tty7", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232_6(RS232_PORT(config, "sio3:cha:tty6", default_rs232_devices, nullptr));
	rs232_6.rxd_handler().set(m_sio[3], FUNC(z80sio_device::rxa_w));
	rs232_6.cts_handler().set(m_sio[3], FUNC(z80sio_device::ctsa_w));
	rs232_6.dcd_handler().set(m_sio[3], FUNC(z80sio_device::dcda_w));
	rs232_port_device &rs232_7(RS232_PORT(config, "sio3:chb:tty7", default_rs232_devices, nullptr));
	rs232_7.rxd_handler().set(m_sio[3], FUNC(z80sio_device::rxb_w));
	rs232_7.cts_handler().set(m_sio[3], FUNC(z80sio_device::ctsb_w));
	rs232_7.dcd_handler().set(m_sio[3], FUNC(z80sio_device::dcdb_w));

	Z80CTC(config, m_ctc[0], CLK_CPU);
	m_ctc[0]->set_clk<0>(CLK_CTC);
	m_ctc[0]->set_clk<1>(CLK_CTC);
	m_ctc[0]->set_clk<2>(CLK_CTC);
	m_ctc[0]->zc_callback<0>().set(m_sio[0], FUNC(z80sio_device::rxca_w));
	m_ctc[0]->zc_callback<0>().append(m_sio[0], FUNC(z80sio_device::txca_w));
	m_ctc[0]->zc_callback<1>().set(m_sio[0], FUNC(z80sio_device::rxcb_w));
	m_ctc[0]->zc_callback<1>().append(m_sio[0], FUNC(z80sio_device::txcb_w));
	m_ctc[0]->zc_callback<2>().set(m_sio[1], FUNC(z80sio_device::rxca_w));
	m_ctc[0]->zc_callback<2>().append(m_sio[1], FUNC(z80sio_device::txca_w));

	Z80CTC(config, m_ctc[1], CLK_CPU);
	m_ctc[1]->set_clk<0>(CLK_CTC);
	m_ctc[1]->set_clk<1>(CLK_CTC);
	m_ctc[1]->set_clk<2>(CLK_CTC);
	m_ctc[1]->set_clk<3>(CLK_CTC);
	m_ctc[1]->zc_callback<0>().set(m_sio[1], FUNC(z80sio_device::rxcb_w));
	m_ctc[1]->zc_callback<0>().append(m_sio[1], FUNC(z80sio_device::txcb_w));
	m_ctc[1]->zc_callback<1>().set(m_sio[2], FUNC(z80sio_device::rxca_w));
	m_ctc[1]->zc_callback<1>().append(m_sio[2], FUNC(z80sio_device::txca_w));
	m_ctc[1]->zc_callback<2>().set(m_sio[2], FUNC(z80sio_device::rxcb_w));
	m_ctc[1]->zc_callback<2>().append(m_sio[2], FUNC(z80sio_device::txcb_w));

	Z80CTC(config, m_ctc[2], CLK_CPU);
	m_ctc[2]->set_clk<0>(CLK_CTC);
	m_ctc[2]->set_clk<1>(CLK_CTC);
	m_ctc[2]->set_clk<2>(CLK_CTC);
	m_ctc[2]->zc_callback<0>().set(m_sio[3], FUNC(z80sio_device::rxca_w));
	m_ctc[2]->zc_callback<0>().append(m_sio[3], FUNC(z80sio_device::txca_w));
	m_ctc[2]->zc_callback<1>().set(m_sio[3], FUNC(z80sio_device::rxcb_w));
	m_ctc[2]->zc_callback<1>().append(m_sio[3], FUNC(z80sio_device::txcb_w));
	m_ctc[2]->zc_callback<2>().set(m_ctc[2], FUNC(z80ctc_device::trg3));

	Z80PIO(config, m_pio, CLK_CPU);
	m_pio->in_pa_callback().set(FUNC(zbi_s8k_cpu10_card_device::pa_data_r));
	m_pio->out_pb_callback().set("pio0:printer1:data_out", FUNC(output_latch_device::write));

	centronics_device &centronics(CENTRONICS(config, "pio0:printer1", centronics_devices, nullptr));
	centronics.strobe_handler().set(m_pio, FUNC(z80pio_device::strobe_a));
	centronics.busy_handler().set(FUNC(zbi_s8k_cpu10_card_device::centronics_busy_w));
	centronics.select_handler().set(FUNC(zbi_s8k_cpu10_card_device::centronics_select_w));
	centronics.fault_handler().set(FUNC(zbi_s8k_cpu10_card_device::centronics_fault_w));
	centronics.ack_handler().set(FUNC(zbi_s8k_cpu10_card_device::centronics_ack_w));

	output_latch_device &latch(OUTPUT_LATCH(config, "pio0:printer1:data_out"));
	centronics.set_output_latch(latch);
}

void zbi_s8k_cpu10_card_device::device_resolve_objects()
{
	s8k_cpu_base::base_device_resolve_objects();

	// Z80 daisy chain for CPU-A board components
	m_bus->add_to_daisy_chain(subtag(m_ctc[0].finder_tag()));
	m_bus->add_to_daisy_chain(subtag(m_ctc[1].finder_tag()));
	m_bus->add_to_daisy_chain(subtag(m_ctc[2].finder_tag()));
	m_bus->add_to_daisy_chain(subtag(m_sio[0].finder_tag()));
	m_bus->add_to_daisy_chain(subtag(m_sio[1].finder_tag()));
	m_bus->add_to_daisy_chain(subtag(m_sio[2].finder_tag()));
	m_bus->add_to_daisy_chain(subtag(m_sio[3].finder_tag()));
	m_bus->add_to_daisy_chain(subtag(m_pio.finder_tag()));

	m_sio[0]->out_int_callback().set(*m_bus, FUNC(zbi_bus_device::vi_w));
	m_sio[1]->out_int_callback().set(*m_bus, FUNC(zbi_bus_device::vi_w));
	m_sio[2]->out_int_callback().set(*m_bus, FUNC(zbi_bus_device::vi_w));
	m_sio[3]->out_int_callback().set(*m_bus, FUNC(zbi_bus_device::vi_w));
	m_ctc[0]->intr_callback().set(*m_bus, FUNC(zbi_bus_device::vi_w));
	m_ctc[1]->intr_callback().set(*m_bus, FUNC(zbi_bus_device::vi_w));
	m_ctc[2]->intr_callback().set(*m_bus, FUNC(zbi_bus_device::vi_w));
	m_pio->out_int_callback().set(*m_bus, FUNC(zbi_bus_device::vi_w));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void zbi_s8k_cpu10_card_device::device_start()
{
	s8k_cpu_base::base_device_start();

	m_bus->set_bus_clock(CLK_BUS);

	save_item(NAME(m_reg_scr));
	save_item(NAME(m_reg_sbr));
	save_item(NAME(m_reg_nbr));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void zbi_s8k_cpu10_card_device::device_reset()
{
	s8k_cpu_base::base_device_reset();

	m_is_seg_os = !!(m_segjp->read());
	m_reg_scr = (uint8_t)(m_dipsw->read());
	m_reg_sbr = 0;
	m_reg_nbr = 0;

	m_bus->iospace()->install_readwrite_handler(0xff81, 0xffbf,
		read8sm_delegate(*this, FUNC(zbi_s8k_cpu10_card_device::comms_r)), write8sm_delegate(*this, FUNC(zbi_s8k_cpu10_card_device::comms_w)));

	m_bus->iospace()->install_readwrite_handler(0xffc0, 0xffc1,
		read16s_delegate(*this, FUNC(zbi_s8k_cpu10_card_device::reg_scr_r)), write16smo_delegate(*this, FUNC(zbi_s8k_cpu10_card_device::reg_scr_w)));
	m_bus->iospace()->install_readwrite_handler(0xffc9, 0xffc9,
		read8smo_delegate(*this, FUNC(zbi_s8k_cpu10_card_device::reg_sbr_r)), write8smo_delegate(*this, FUNC(zbi_s8k_cpu10_card_device::reg_sbr_w)));
	m_bus->iospace()->install_readwrite_handler(0xffd1, 0xffd1,
		read8smo_delegate(*this, FUNC(zbi_s8k_cpu10_card_device::reg_nbr_r)), write8smo_delegate(*this, FUNC(zbi_s8k_cpu10_card_device::reg_nbr_w)));
	m_bus->iospace()->install_read_handler(0xffd9, 0xffd9, read8smo_delegate(*this, FUNC(zbi_s8k_cpu10_card_device::reg_snvr_r)));
	m_bus->iospace()->install_write_handler(0xffe1, 0xffe1, write8smo_delegate(*m_bus, FUNC(zbi_bus_device::reti_w)));
	m_bus->iospace()->install_write_handler(0xffe9, 0xffe9, write8smo_delegate(*this, FUNC(zbi_s8k_cpu10_card_device::soft_reset_w)));
	m_bus->iospace()->install_read_handler(0xfff1, 0xfff1, read8smo_delegate(*this, FUNC(zbi_s8k_cpu10_card_device::reg_trpl_r)));
	m_bus->iospace()->install_read_handler(0xfff9, 0xfff9, read8smo_delegate(*this, FUNC(zbi_s8k_cpu10_card_device::reg_if1l_r)));
}

// ROM size: 4x2KB
ROM_START( s8k_cpu10 )
	ROM_REGION16_BE(0x2000, "maincpu", 0)

	ROM_LOAD16_BYTE("cpu_34-0601-00a.u76", 0x0000, 0x0800, CRC(fe8e0de5) SHA1(9d28f6ecbf4f077c80cf987c9ea7adf99bf3429c))
	ROM_LOAD16_BYTE("cpu_34-0602-00a.u74", 0x0001, 0x0800, CRC(0b634c89) SHA1(1b81f56151038812441a7ceaf28f2bcb7d58b6d4))
	ROM_LOAD16_BYTE("cpu_34-0605-00a.u77", 0x1000, 0x0800, CRC(ae5ebab8) SHA1(e082107914da9acc1a71fa5c2dc9d1d464222fe5))
	ROM_LOAD16_BYTE("cpu_34-0606-00a.u75", 0x1001, 0x0800, CRC(a46db483) SHA1(86569a6c8649f691acbf7d42a825bde105460f55))
ROM_END

static INPUT_PORTS_START( s8k_cpu10 )
	PORT_START("SEGJP")
	PORT_CONFNAME(0x01, 0x00, "Support Segmented OS (Must Be No)")
	PORT_CONFSETTING(	0x00, DEF_STR( No ) )
	PORT_CONFSETTING(	0x01, DEF_STR( Yes ) )

	PORT_START("DIPSW")
	PORT_DIPNAME( 0x30, 0x20, "Serial Console Baud Rate" )	PORT_DIPLOCATION("U70:1,4")
	PORT_DIPSETTING(	0x00, "300 baud" )
	PORT_DIPSETTING(	0x10, "1200 baud" )
	PORT_DIPSETTING(	0x20, "9600 baud" )
	PORT_DIPSETTING(	0x30, "19200 baud" )
	PORT_DIPUNUSED_DIPLOC( 0xC0, 0xC0, "U70:2,3" )
INPUT_PORTS_END

const tiny_rom_entry *zbi_s8k_cpu10_card_device::device_rom_region() const
{
	return ROM_NAME(s8k_cpu10);
}

ioport_constructor zbi_s8k_cpu10_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(s8k_cpu10);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  zbi_s8k_cpu_card_device - constructor
//-------------------------------------------------

zbi_s8k_cpu_card_device::zbi_s8k_cpu_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: zbi_s8k_cpu10_card_device(mconfig, ZBI_S8K_CPU, tag, owner, clock)
{
}

void zbi_s8k_cpu_card_device::install_memory()
{
	s8k_cpu_base::install_memory(0x3fff, 0x4000, 0x7fff);
}

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

static INPUT_PORTS_START( s8k_cpu )
	PORT_START("SEGJP")
	PORT_CONFNAME(0x01, 0x00, "Support Segmented OS (v2.2+ Only)")
	PORT_CONFSETTING(	0x00, DEF_STR( No ) )
	PORT_CONFSETTING(	0x01, DEF_STR( Yes ) )

	PORT_START("DIPSW")
	PORT_DIPNAME( 0x30, 0x20, "Serial Console Baud Rate" )	PORT_DIPLOCATION("U70:1,4")
	PORT_DIPSETTING(	0x00, "300 baud" )
	PORT_DIPSETTING(	0x10, "1200 baud" )
	PORT_DIPSETTING(	0x20, "9600 baud" )
	PORT_DIPSETTING(	0x30, "19200 baud" )
	PORT_DIPNAME( 0xC0, 0x40, "Primary Boot Device" )		PORT_DIPLOCATION("U70:2,3")
	PORT_DIPSETTING(	0xC0, "8-inch Disk")
	PORT_DIPSETTING(	0x80, "5.25-inch Disk")
	PORT_DIPSETTING(	0x40, "SMD Disk")
INPUT_PORTS_END

// ROM size: 4x4KB
ROM_START( s8k_cpu )
	ROM_REGION16_BE(0x4000, "maincpu", 0)
	ROM_DEFAULT_BIOS("v30")

	ROM_SYSTEM_BIOS(0, "v12", "Version 1.2")
	ROMX_LOAD("cpu_34-0715-01a.u76", 0x0000, 0x1000, CRC(81a4cac6) SHA1(9e74883a365f1034610b1c4681ca3611362d62ea), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("cpu_34-0716-01a.u74", 0x0001, 0x1000, CRC(ab2ca534) SHA1(25857479801397f1f18c55b40f81cb5ba7a01a55), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("cpu_34-0718-01a.u77", 0x2000, 0x1000, CRC(6b8b4536) SHA1(85ff7c9be0f51e299d4f9406064cd32df08b8f16), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("cpu_34-0717-01a.u75", 0x2001, 0x1000, CRC(c8d3be3b) SHA1(efbca6fcbf53565075b67a14096d0f725839494a), ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "v22", "Version 2.2")
	ROMX_LOAD("cpu_34-0715-02_v_2.2.u76", 0x0000, 0x1000, CRC(198ce8ee) SHA1(743d75dab6f4ea85b2f95ec1b620134f4416a351), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("cpu_34-0716-02_v_2.2.u74", 0x0001, 0x1000, CRC(8a3ea482) SHA1(0572b21ac5aeb24cec01d7682f1ad7eef08cb070), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("cpu_34-0718-02_v_2.2.u77", 0x2000, 0x1000, CRC(43660a81) SHA1(8398d1998384ea0a95fcad58f791d9657e023b83), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("cpu_34-0717-02_v_2.2.u75", 0x2001, 0x1000, CRC(8ddb6479) SHA1(93eec5a59a7856d19e32f526dddb4f21c1864373), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "v30_8u", "Version 3.0 (8 users)")
	ROMX_LOAD("cpu_34-0715-03a.u76",       0x0000, 0x1000, CRC(addc3e4f) SHA1(86e013450d23ab7a39b50bffa6113a0a060f1650), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("cpu_34-0716-03a_8user.u74", 0x0001, 0x1000, CRC(645dd24b) SHA1(f123684c604a971ade5ed229538403989be7cc2a), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("cpu_34-0718-03a.u77",       0x2000, 0x1000, CRC(f2341d8e) SHA1(574e6d1dd4e5211c0c83cad21e6e1b53810c699b), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("cpu_34-0717-03a.u75",       0x2001, 0x1000, CRC(3a0370b3) SHA1(4660c1c58202ebaaf1200f92de719e1811860fd9), ROM_SKIP(1) | ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "v30", "Version 3.0")
	ROMX_LOAD("cpu_34-0715-03a.u76", 0x0000, 0x1000, CRC(addc3e4f) SHA1(86e013450d23ab7a39b50bffa6113a0a060f1650), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD("cpu_34-0716-03a.u74", 0x0001, 0x1000, CRC(9a315ba4) SHA1(c2ab2bfaf21d60f69ea7ede01a929ac46511061b), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD("cpu_34-0718-03a.u77", 0x2000, 0x1000, CRC(f2341d8e) SHA1(574e6d1dd4e5211c0c83cad21e6e1b53810c699b), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD("cpu_34-0717-03a.u75", 0x2001, 0x1000, CRC(3a0370b3) SHA1(4660c1c58202ebaaf1200f92de719e1811860fd9), ROM_SKIP(1) | ROM_BIOS(3))

	// TODO
	//ROM_SYSTEM_BIOS(4, "v31", "Version 3.1")
	//ROMX_LOAD("cpu_34-0715-04a_8usr.u76", 0x0000, 0x1000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(4))
	//ROMX_LOAD("cpu_34-0716-04a.u74", 0x0001, 0x1000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(4))
	//ROMX_LOAD("cpu_34-0718-04a.u77", 0x2000, 0x1000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(4))
	//ROMX_LOAD("cpu_34-0717-04a.u75", 0x2001, 0x1000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(4))
ROM_END

const tiny_rom_entry *zbi_s8k_cpu_card_device::device_rom_region() const
{
	return ROM_NAME(s8k_cpu);
}

ioport_constructor zbi_s8k_cpu_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(s8k_cpu);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  zbi_s8k_hpcpu_card_device - constructor
//-------------------------------------------------

zbi_s8k_hpcpu_card_device::zbi_s8k_hpcpu_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: s8k_cpu_base(mconfig, ZBI_S8K_HPCPU, tag, owner, clock)
	, m_cache(*this, "cache", 0x8000, ENDIANNESS_BIG)	// 32 KB
	, m_scc(*this, "scc")
	, m_cio(*this, "cio")
{
	m_is_seg_os = true;
}

bool zbi_s8k_hpcpu_card_device::translate_addr(int spacenum, bool write, offs_t &offset)
{
	bool stack_access = (spacenum == z8001_device::AS_STACK);

	offset <<= 1;

	if (m_reg_scr & SCR_MMU_ONH)
	{
		bool code_access = (spacenum == AS_PROGRAM);
		z8010_device *mmu = code_access ?
							select_code_mmu(offset) : select_data_mmu(offset, 0, m_reg_ubr);

		if (mmu)
		{
			int st = code_access ?
						(m_maincpu->is_ifetch1() ?
							z8002_device::ST_IFETCH_1 :
							z8002_device::ST_IFETCH_N) :
						(stack_access ?
							z8002_device::ST_REQ_STACK :
							z8002_device::ST_REQ_DATA);

			LOG("%s MMU MEM REQ (space %d): %06x\n", machine().describe_context(), spacenum, offset);

			offset &= 0x3f'ffff;	// Mask off seg bit 7 to disable URS checking in MMUs

			return mmu->translate(offset, write, true, m_dma_on, st);
		}
	}

	return true;
}

void zbi_s8k_hpcpu_card_device::install_memory()
{
	s8k_cpu_base::install_memory(0x3fff, 0x7800, 0x7fff);
}

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

uint16_t zbi_s8k_hpcpu_card_device::reg_scr_r()
{
	// SCR is on odd address so need to swap
	return swapendian_int16(m_reg_scr);
}

void zbi_s8k_hpcpu_card_device::reg_scr_w(uint16_t data)
{
	data = swapendian_int16(data);

	uint16_t diff = data ^ m_reg_scr;

	LOG("%s reg_scr_w: %04x\n", machine().describe_context(), data);

	if (diff & SCR_BD_MEMON)
	{
		if (data & SCR_BD_MEMON)
		{
			m_view_code.disable();
			m_view_data.disable();
			m_view_stck.disable();
		}
		else
		{
			m_view_code.select(0);
			m_view_data.select(0);
			m_view_stck.select(0);
		}
	}

	if (diff & SCR_MMU_ONH)
	{
		m_maincpu->space(AS_PROGRAM).invalidate_caches(read_or_write::READWRITE);
		m_maincpu->space(AS_DATA).invalidate_caches(read_or_write::READWRITE);
		m_maincpu->space(z8001_device::AS_STACK).invalidate_caches(read_or_write::READWRITE);
	}

	if (diff & SCR_SEG_USER)
	{
		m_is_seg_user = !!(data & SCR_SEG_USER);
	}

	m_reg_scr = (m_reg_scr & 0x0ff0) | ( data & 0xf00f );	// Mask off read-only parts
}

uint8_t zbi_s8k_hpcpu_card_device::reg_ubr_r()
{
	return m_reg_ubr;
}

void zbi_s8k_hpcpu_card_device::reg_ubr_w(uint8_t data)
{
	LOG("%s reg_UBR_w: %02x\n", machine().describe_context(), data);
	m_reg_ubr = data;
}

uint8_t zbi_s8k_hpcpu_card_device::spec_io_r(offs_t offset)
{
	if (!BIT(offset, 0))
	{
		uint8_t reg = (uint8_t)(offset >> 8);

		if (!BIT(offset, 1)) return m_mmu_code->read(reg);
		if (!BIT(offset, 2)) return m_mmu_data->read(reg);
		if (!BIT(offset, 3)) return m_mmu_stck->read(reg);
	}
	else
		return m_cache[offset >> 1];

	return 0xff;
}

void zbi_s8k_hpcpu_card_device::spec_io_w(offs_t offset, uint8_t data)
{
	if (!BIT(offset, 0))
	{
		uint8_t reg = (uint8_t)(offset >> 8);

		if (!BIT(offset, 1)) m_mmu_code->write(reg, data);
		if (!BIT(offset, 2)) m_mmu_data->write(reg, data);
		if (!BIT(offset, 3)) m_mmu_stck->write(reg, data);

		m_maincpu->space(AS_PROGRAM).invalidate_caches(read_or_write::READWRITE);
		m_maincpu->space(AS_DATA).invalidate_caches(read_or_write::READWRITE);
		m_maincpu->space(z8001_device::AS_STACK).invalidate_caches(read_or_write::READWRITE);
	}
	else
		m_cache[offset >> 1] = data;
}

void zbi_s8k_hpcpu_card_device::addrmap_sio(address_map &map)
{
	s8k_cpu_base::addrmap_sio(map);
	map(0x0000, 0xffff).rw(FUNC(zbi_s8k_hpcpu_card_device::spec_io_r), FUNC(zbi_s8k_hpcpu_card_device::spec_io_w));
}

//**************************************************************************
//  DEVICE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void zbi_s8k_hpcpu_card_device::device_start()
{
	s8k_cpu_base::base_device_start();

	save_item(NAME(m_reg_scr));
	save_item(NAME(m_reg_ubr));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void zbi_s8k_hpcpu_card_device::device_reset()
{
	s8k_cpu_base::base_device_reset();

	m_reg_scr = (uint16_t)(m_dipsw->read());
	m_reg_ubr = 0;

	m_bus->iospace()->install_readwrite_handler(0xff81, 0xff81,
		read8sm_delegate(m_scc, FUNC(scc8530_device::da_r)),  write8sm_delegate(m_scc, FUNC(scc8530_device::da_w)));
	m_bus->iospace()->install_readwrite_handler(0xff83, 0xff83,
		read8sm_delegate(m_scc, FUNC(scc8530_device::db_r)),  write8sm_delegate(m_scc, FUNC(scc8530_device::db_w)));
	m_bus->iospace()->install_readwrite_handler(0xff85, 0xff85,
		read8sm_delegate(m_scc, FUNC(scc8530_device::ca_r)),  write8sm_delegate(m_scc, FUNC(scc8530_device::ca_w)));
	m_bus->iospace()->install_readwrite_handler(0xff87, 0xff87,
		read8sm_delegate(m_scc, FUNC(scc8530_device::cb_r)),  write8sm_delegate(m_scc, FUNC(scc8530_device::cb_w)));

	m_bus->iospace()->install_readwrite_handler(0xffa1, 0xffa7,
		read8sm_delegate(*this, FUNC(zbi_s8k_hpcpu_card_device::cio_r)), write8sm_delegate(*this, FUNC(zbi_s8k_hpcpu_card_device::cio_w)));

	m_bus->iospace()->install_readwrite_handler(0xffc0, 0xffc1,
		read16smo_delegate(*this, FUNC(zbi_s8k_hpcpu_card_device::reg_scr_r)), write16smo_delegate(*this, FUNC(zbi_s8k_hpcpu_card_device::reg_scr_w)));
	m_bus->iospace()->install_readwrite_handler(0xffd1, 0xffd1,
		read8smo_delegate(*this, FUNC(zbi_s8k_hpcpu_card_device::reg_ubr_r)), write8smo_delegate(*this, FUNC(zbi_s8k_hpcpu_card_device::reg_ubr_w)));
	m_bus->iospace()->install_read_handler(0xffd9, 0xffd9, read8smo_delegate(*this, FUNC(zbi_s8k_hpcpu_card_device::reg_snvr_r)));
	m_bus->iospace()->install_write_handler(0xffe1, 0xffe1, write8smo_delegate(*m_bus, FUNC(zbi_bus_device::reti_w)));
	m_bus->iospace()->install_write_handler(0xffe9, 0xffe9, write8smo_delegate(*this, FUNC(zbi_s8k_hpcpu_card_device::soft_reset_w)));
	m_bus->iospace()->install_read_handler(0xfff1, 0xfff1, read8smo_delegate(*this, FUNC(zbi_s8k_hpcpu_card_device::reg_trpl_r)));
	m_bus->iospace()->install_read_handler(0xfff9, 0xfff9, read8smo_delegate(*this, FUNC(zbi_s8k_hpcpu_card_device::reg_if1l_r)));
}

void zbi_s8k_hpcpu_card_device::device_resolve_objects()
{
	s8k_cpu_base::base_device_resolve_objects();

	// Z80 daisy chain for HPCPU board components
	m_bus->add_to_daisy_chain(subtag(m_cio.finder_tag()));
	m_bus->add_to_daisy_chain(subtag(m_scc.finder_tag()));

	m_scc->out_int_callback().set(*m_bus, FUNC(zbi_bus_device::vi_w));
	m_cio->irq_wr_cb().set(*m_bus, FUNC(zbi_bus_device::vi_w));
}

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void zbi_s8k_hpcpu_card_device::device_add_mconfig(machine_config &config)
{
	Z8001(config, m_maincpu, CLK_HPCPU);
	m_maincpu->set_m20_hack(false);
	m_maincpu->set_addrmap(AS_PROGRAM, &zbi_s8k_hpcpu_card_device::addrmap_program);
	m_maincpu->set_addrmap(AS_DATA, &zbi_s8k_hpcpu_card_device::addrmap_data);
	m_maincpu->set_addrmap(z8001_device::AS_STACK, &zbi_s8k_hpcpu_card_device::addrmap_stack);
	m_maincpu->set_addrmap(AS_IO, &zbi_s8k_hpcpu_card_device::addrmap_io);
	m_maincpu->set_addrmap(z8001_device::AS_SIO, &zbi_s8k_hpcpu_card_device::addrmap_sio);
	m_maincpu->segtack().set(FUNC(zbi_s8k_hpcpu_card_device::segtack_r));
	m_maincpu->nmiack().set(FUNC(zbi_s8k_hpcpu_card_device::nmiack_r));
	m_maincpu->ns().set(FUNC(zbi_s8k_hpcpu_card_device::out_ns_cb));
	m_maincpu->busack().set(FUNC(zbi_s8k_hpcpu_card_device::out_busack_cb));

	Z8010(config, m_mmu_code, CLK_HPCPU);
	m_mmu_code->out_segt_cb().set(FUNC(zbi_s8k_hpcpu_card_device::segt_interrupt));
	Z8010(config, m_mmu_data, CLK_HPCPU);
	m_mmu_data->out_segt_cb().set(FUNC(zbi_s8k_hpcpu_card_device::segt_interrupt));
	Z8010(config, m_mmu_stck, CLK_HPCPU);
	m_mmu_stck->out_segt_cb().set(FUNC(zbi_s8k_hpcpu_card_device::segt_interrupt));

	SCC8530(config, m_scc, CLK_HPCPU);
	m_scc->configure_channels(CLK_SCC, CLK_SCC, CLK_SCC, CLK_SCC);
	m_scc->out_txda_callback().set("scc:cha:tty0", FUNC(rs232_port_device::write_txd));
	m_scc->out_rtsa_callback().set("scc:cha:tty0", FUNC(rs232_port_device::write_rts));
	m_scc->out_dtra_callback().set("scc:cha:tty0", FUNC(rs232_port_device::write_dtr));
	m_scc->out_txdb_callback().set("scc:chb:console", FUNC(rs232_port_device::write_txd));
	m_scc->out_rtsb_callback().set("scc:chb:console", FUNC(rs232_port_device::write_rts));
	m_scc->out_dtrb_callback().set("scc:chb:console", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232a(RS232_PORT(config, "scc:cha:tty0", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc, FUNC(scc8530_device::rxa_w));
	rs232a.cts_handler().set(m_scc, FUNC(scc8530_device::ctsa_w));
	rs232a.dcd_handler().set(m_scc, FUNC(scc8530_device::dcda_w));
	rs232_port_device &rs232b(RS232_PORT(config, "scc:chb:console", default_rs232_devices, "terminal"));
	rs232b.rxd_handler().set(m_scc, FUNC(scc8530_device::rxb_w));
	rs232b.cts_handler().set(m_scc, FUNC(scc8530_device::ctsb_w));
	rs232b.dcd_handler().set(m_scc, FUNC(scc8530_device::dcdb_w));

	Z8536(config, m_cio, CLK_HPCPU);
}

static INPUT_PORTS_START( s8k_hpcpu )
	PORT_START("DIPSW")
	PORT_DIPNAME( 0x0030, 0x0010, "Serial Console Baud Rate" )	PORT_DIPLOCATION("F4:7,8")
	PORT_DIPSETTING(	  0x0030, "300 baud" )
	PORT_DIPSETTING(	  0x0020, "1200 baud" )
	PORT_DIPSETTING(	  0x0010, "9600 baud" )
	PORT_DIPSETTING(	  0x0000, "19200 baud" )
	PORT_DIPNAME( 0x00C0, 0x0040, "Primary Boot Device" )		PORT_DIPLOCATION("F4:5,6")
	PORT_DIPSETTING(	  0x00C0, "8-inch Disk")
	PORT_DIPSETTING(	  0x0080, "5.25-inch Disk")
	PORT_DIPSETTING(	  0x0040, "SMD Disk")
	PORT_DIPUNKNOWN_DIPLOC(0x0100, 0x0100, "F4:4")	// "INVALID BOOT DEVICE CODE" when enabled
	PORT_DIPUNKNOWN_DIPLOC(0x0200, 0x0200, "F4:3")	// Lots of errors (non-seg mode?)
	PORT_DIPUNKNOWN_DIPLOC(0x0400, 0x0000, "F4:2")	// Enables baud 300 when disabled, but generates "ERROR #0030" (off-board tty?)
	PORT_DIPUNKNOWN_DIPLOC(0x0800, 0x0800, "F4:1")	// Nothing (test skip?)
INPUT_PORTS_END

// ROM size: 2x8KB
ROM_START( s8k_hpcpu )
	ROM_REGION16_BE(0x4000, "maincpu", 0)

	ROM_LOAD16_BYTE("hpcpu_34-1117-00_v10.1_common.19e", 0x0000, 0x2000, CRC(a77055c8) SHA1(f1268c6b163f9e4151d425ccc2f5cb4c9c0af8c8))
	ROM_LOAD16_BYTE("hpcpu_34-1119-00_v10.1_16user.21e", 0x0001, 0x2000, CRC(610e8b0c) SHA1(bc804e09cf6905c9f0ccc502a15c98841ee4b087))
ROM_END

ioport_constructor zbi_s8k_hpcpu_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(s8k_hpcpu);
}

const tiny_rom_entry *zbi_s8k_hpcpu_card_device::device_rom_region() const
{
	return ROM_NAME(s8k_hpcpu);
}

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ZBI_S8K_CPU10, zbi_s8k_cpu10_card_device, "s8k_cpu10", "System 8000 CPU-A 1.0")
DEFINE_DEVICE_TYPE(ZBI_S8K_CPU, zbi_s8k_cpu_card_device, "s8k_cpu", "System 8000 CPU-A")
DEFINE_DEVICE_TYPE(ZBI_S8K_HPCPU, zbi_s8k_hpcpu_card_device, "s8k_hpcpu", "System 8000 HPCPU")
