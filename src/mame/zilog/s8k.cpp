// license:BSD-3-Clause
// copyright-holders:A. Lenard
/***************************************************************************

	Zilog System 8000 driver

	Based on docs and code from Oliver Lehmann's site (http://www.pofo.de/S8000/)

	The following models are known:

	               CPU  Segmented  Disk type
	Model 11        A       N        5.25"
	Model 11 Plus   A       Y        5.25"
	Model 12        H       Y        5.25"
	Model 20        A       N         8"
	Model 21        A       N         8"
	Model 21 Plus   A       Y         8"
	Model 22        H       Y        5.25"
	Model 30        A       N         8"
	Model 31        A       N        SMD
	Model 31 Plus   A       Y        SMD
	Model 32        H       Y        SMD

	(CPU A = 5.5 Mhz; CPU H = 11 Mhz + 32 kB cache)

***************************************************************************/

#include "emu.h"
#include "ioport.h"

#include "bus/rs232/rs232.h"
#include "bus/centronics/ctronics.h"
#include "cpu/z8000/z8000.h"
#include "machine/z8010.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/z80pio.h"
#include "machine/ram.h"

#include "s8k.lh"

//#define VERBOSE 1
#include "logmacro.h"

class s8k_16_daisy_device : public device_t, public z80_daisy_chain_interface
{
public:
	s8k_16_daisy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t viack_r()
	{
		//logerror("Daisy interrupt asserted.\n");
		device_z80daisy_interface *intf = daisy_get_irq_device();
		return intf ? intf->z80daisy_irq_ack() : 0;
	}

	void reti_w(uint8_t data)
		{ if(data == 0x4d) daisy_call_reti_device(); }

protected:
	void device_start() override
		{ }
};

DEFINE_DEVICE_TYPE(S8K_16_DAISY, s8k_16_daisy_device, "s8k_16_daisy", "S8000 16-bit daisy chain device")

s8k_16_daisy_device::s8k_16_daisy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, S8K_16_DAISY, tag, owner, clock),
	z80_daisy_chain_interface(mconfig, *this)
	{ }

namespace {

class s8k_state : public driver_device
{
public:
	s8k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_local_ram(*this, "local_ram", 0x800, ENDIANNESS_BIG),
		m_view_code(*this, "memview_code"),
		m_view_data(*this, "memview_data"),
		m_view_stck(*this, "memview_stck"),
		m_mmu_code(*this, "mmu0_code"),
		m_mmu_data(*this, "mmu1_data"),
		m_mmu_stck(*this, "mmu2_stck"),
		m_daisy(*this, "s8k_16_daisy"),
		m_sio(*this, "sio%u", 0U),
		m_ctc(*this, "ctc%u", 0U),
		m_pio(*this, "pio0"),
		m_dsw(*this, "DSW"),
		m_jp_seg(*this, "JP_SEG"),
		m_start_btn(*this),
		m_normal_led(*this, "normal_led"),
		m_busack_led(*this, "busack_led")
	{
	}

	void s8k(machine_config &config);

	void start_btn_w(int state);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<z8001_device> m_maincpu;
	required_device<ram_device> m_ram;
	memory_share_creator<uint16_t> m_local_ram;
	memory_view m_view_code, m_view_data, m_view_stck;
	required_device<z8010_device> m_mmu_code, m_mmu_data, m_mmu_stck;
	required_device<s8k_16_daisy_device> m_daisy;
	required_device_array<z80sio_device, 4> m_sio;
	required_device_array<z80ctc_device, 3> m_ctc;
	required_device<z80pio_device> m_pio;

	required_ioport m_dsw;	// DIP switch
	required_ioport m_jp_seg;
	devcb_write_line m_start_btn;

	output_finder<> m_normal_led;
	output_finder<> m_busack_led;

private:
	uint8_t reg_scr_r();
	void reg_scr_w(uint8_t data);
	uint8_t reg_sbr_r();
	void reg_sbr_w(uint8_t data);
	uint8_t reg_nbr_r();
	void reg_nbr_w(uint8_t data);
	uint8_t reg_snvr_r();
	uint8_t reg_trpl_r();
	uint8_t reg_if1l_r();

	z8010_device *select_code_mmu(offs_t offset);
	z8010_device *select_data_mmu(offs_t offset);

	bool translate_addr(int spacenum, bool write, offs_t &offset);
	uint16_t ram_r(address_space &space, offs_t offset, uint16_t mask);
	void ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mask);

	uint8_t comms_r(offs_t offset);
	void comms_w(offs_t offset, uint8_t data);

	uint8_t mmu_cmd_r(offs_t offset);
	void mmu_cmd_w(offs_t offset, uint8_t data);

	uint8_t pa_data_r();

	void centronics_busy_w(uint8_t data);
	void centronics_select_w(uint8_t data);
	void centronics_fault_w(uint8_t data);
	void centronics_ack_w(uint8_t data);

	uint16_t segtack_r();
	uint16_t nmiack_r();

	void normal_led_w(int state);

	void install_memory();

	void stack_mem(address_map &map) ATTR_COLD;
	void data_mem(address_map &map) ATTR_COLD;
	void program_mem(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void spec_io_map(address_map &map) ATTR_COLD;

	void daisy_interrupt(int state);
	void segt_interrupt(int state);

	offs_t m_memsize = 0;

	// Board registers
	uint8_t m_reg_scr	= 0; // System Configuration Register
	uint8_t m_reg_sbr	= 0; // System Break Register
	uint8_t m_reg_nbr	= 0; // Normal Break Register
	uint8_t m_reg_snvr	= 0; // Segment Violation Register
	uint8_t m_reg_trpl	= 0; // Segment trap memory address low-byte
	uint8_t m_reg_if1l	= 0; // Segment trap instruction low-byte

	bool m_is_seg_os = false;
	bool m_is_seg_user = false;

	// --- ZBI peripherals (TEMPORARY!!!) ---

	uint8_t m_ecc_reg = 0;

	uint8_t ecc_reg_r();
	void ecc_reg_w(uint8_t data);

	// --- ZBI peripherals ---

	uint16_t m_nmi_code = 0;

	int m_centronics_busy = 0;
	int m_centronics_select = 0;
	int m_centronics_fault = 0;
	int m_centronics_ack = 0;
};

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MAIN_CLOCK 		5528650 /* 5.5 Mhz (44.440 Mhz / 8) */
#define MASTER_CLOCK	(4 * MAIN_CLOCK) /* 22.1 Mhz */
#define CTC_CLOCK		(2.4576_MHz_XTAL / 2)

// System Configuration Register control bits 0-4
#define S8K_SCR_BD_MEMON	0x01	// 1 = use off-board memory
#define S8K_SCR_MMU_ONH		0x02	// 1 = enable MMUs
#define S8K_SCR_SEG_USER	0x04	// 1 = segmented user space
#define S8K_SCR_CLR_PARITY	0x08	// 1 = enable NMI from parity or ECC error

// Non-Maskable Interrupt reason codes
static constexpr uint16_t S8K_NMI_MANUAL 	= 0x0001;
static constexpr uint16_t S8K_NMI_PWRFAIL	= 0x0002;
static constexpr uint16_t S8K_NMI_ECCERR 	= 0x0004;

//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START( s8k )
	PORT_START("FP")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Start")  PORT_CODE(KEYCODE_PLUS_PAD) PORT_WRITE_LINE_MEMBER(FUNC(s8k_state::start_btn_w))

	PORT_START("JP_SEG")
	PORT_CONFNAME(0x01, 0x00, "Support segmented OS")
	PORT_CONFSETTING(	0x00, DEF_STR( No ) )
	PORT_CONFSETTING(	0x01, DEF_STR( Yes ) )

	PORT_START("DSW")
	PORT_DIPNAME( 0x30, 0x20, "Serial console baud rate" )	PORT_DIPLOCATION("U70:1,4")
	PORT_DIPSETTING(	0x00, "300 baud" )
	PORT_DIPSETTING(	0x10, "1200 baud" )
	PORT_DIPSETTING(	0x20, "9600 baud" )
	PORT_DIPSETTING(	0x30, "19200 baud" )
	PORT_DIPNAME( 0xC0, 0xC0, "Primary boot device" )		PORT_DIPLOCATION("U70:2,3")
	PORT_DIPSETTING(	0xC0, "8 inch disk")
	PORT_DIPSETTING(	0x80, "5.25 inch disk")
	PORT_DIPSETTING(	0x40, "SMD disk")
INPUT_PORTS_END

void s8k_state::start_btn_w(int state)
{
	if(state & (m_nmi_code == 0))
	{
		m_nmi_code = S8K_NMI_MANUAL;
		m_maincpu->set_input_line(z8001_device::NMI_LINE, ASSERT_LINE);
	}
}

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

uint8_t s8k_state::ecc_reg_r()
{
	LOG("%s -> ECC REGISTER READ! \n");
	return m_ecc_reg;
}

void s8k_state::ecc_reg_w(uint8_t data)
{
	LOG("%s -> ECC REGISTER WRITTEN: %02x\n", machine().describe_context(), data);
	m_ecc_reg = data;
}

uint8_t s8k_state::reg_scr_r()
{
	return m_reg_scr;
}

void s8k_state::reg_scr_w(uint8_t data)
{
	uint8_t diff = data ^ m_reg_scr;

	LOG("%s reg_scr_w: %02x\n", machine().describe_context(), data);

	if(diff & S8K_SCR_BD_MEMON)
	{
		if(data & S8K_SCR_BD_MEMON)
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

	if(diff & S8K_SCR_MMU_ONH)
	{
		m_maincpu->space(AS_PROGRAM).invalidate_caches(read_or_write::READWRITE);
		m_maincpu->space(AS_DATA).invalidate_caches(read_or_write::READWRITE);
		m_maincpu->space(z8001_device::AS_STACK).invalidate_caches(read_or_write::READWRITE);
	}

	if(diff & S8K_SCR_SEG_USER)
	{
		m_is_seg_user = data & S8K_SCR_SEG_USER;
	}

	m_reg_scr = (m_reg_scr & 0xf0) | ( data & 0x0f );	// Mask off read-only nibble
}

uint8_t s8k_state::reg_sbr_r()
{
	return m_reg_sbr;
}

void s8k_state::reg_sbr_w(uint8_t data)
{
	LOG("%s reg_SBR_w: %02x\n", machine().describe_context(), data);
	m_reg_sbr = data;
}

uint8_t s8k_state::reg_nbr_r()
{
	return m_reg_nbr;
}

void s8k_state::reg_nbr_w(uint8_t data)
{
	LOG("%s reg_NBR_w: %02x\n", machine().describe_context(), data);
	m_reg_nbr = data;
}

uint8_t s8k_state::reg_snvr_r()
{
	return m_reg_snvr;
}

uint8_t s8k_state::reg_trpl_r()
{
	return m_reg_trpl;
}

uint8_t s8k_state::reg_if1l_r()
{
	return m_reg_if1l;
}

uint8_t s8k_state::comms_r(offs_t offset)
{
	if((offset & 1) == 0)
	{
		offset >>= 1;

		switch(offset & 0xfc)
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

void s8k_state::comms_w(offs_t offset, uint8_t data)
{
	if((offset & 1) == 0)
	{
		offset >>= 1;

		switch(offset & 0xfc)
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

uint8_t s8k_state::mmu_cmd_r(offs_t offset)
{
	if((offset & 1) == 0)
	{
		uint8_t reg = (uint8_t)(offset >> 8);

		if((offset & 0x2) == 0)
			return m_mmu_code->read(reg);
		if((offset & 0x4) == 0)
			return m_mmu_data->read(reg);
		if((offset & 0x8) == 0)
			return m_mmu_stck->read(reg);
	}

	return 0xff;
}

void s8k_state::mmu_cmd_w(offs_t offset, uint8_t data)
{
	if((offset & 1) == 0)
	{
		uint8_t reg = (uint8_t)(offset >> 8);

		if((offset & 0x2) == 0)
			m_mmu_code->write(reg, data);
		if((offset & 0x4) == 0)
			m_mmu_data->write(reg, data);
		if((offset & 0x8) == 0)
			m_mmu_stck->write(reg, data);

		m_maincpu->space(AS_PROGRAM).invalidate_caches(read_or_write::READWRITE);
		m_maincpu->space(AS_DATA).invalidate_caches(read_or_write::READWRITE);
		m_maincpu->space(z8001_device::AS_STACK).invalidate_caches(read_or_write::READWRITE);
	}
}

void s8k_state::io_map(address_map &map)
{
	/*

	=========================================================
					System I/O Space Memory Map
	=========================================================
	I/O Space      Device Class            Current use
	=========================================================
	0000 - 003F    Memory Devices          ECC 0000 - 0003
	0040 - 007F    Tape                    TCU 0040 - 004F
	0080 - 0FFF    Other Memory Devices    None
	1000 - 1FFF    Tape Devices            9-Track 1000 - 101F
	2000 - 6FFF    Reserved                FPP 2000 - 200F
	7000 - 8FFF    Disk Devices
					- SMD                  7FF0 - 7FFF
					- WDC/mWDC-II          8000 - 80FF
	9000 - 9FFF    Available to User
	A000 - DFFF    Reserved
	E000 - FFFF    Communication Devices
					- CPU                  FFC1 - FFFF
					- ICP                  EF01 - EF0F
					- SSB                  FF41 - FF7F
					- SSB                  FF01 - FF3F
	=========================================================

	*/

	map.unmap_value_high();
	map(0xff81, 0xffbf).rw(FUNC(s8k_state::comms_r), FUNC(s8k_state::comms_w));

	map(0xffc1, 0xffc1).rw(FUNC(s8k_state::reg_scr_r), FUNC(s8k_state::reg_scr_w));
	map(0xffc9, 0xffc9).rw(FUNC(s8k_state::reg_sbr_r), FUNC(s8k_state::reg_sbr_w));
	map(0xffd1, 0xffd1).rw(FUNC(s8k_state::reg_nbr_r), FUNC(s8k_state::reg_nbr_w));
	map(0xffd9, 0xffd9).r(FUNC(s8k_state::reg_snvr_r));
	map(0xffe1, 0xffe1).w("s8k_16_daisy", FUNC(s8k_16_daisy_device::reti_w));
	//map(0xffe9, 0xffe9).w(FUNC(s8k_state::reset());
	map(0xfff1, 0xfff1).r(FUNC(s8k_state::reg_trpl_r));
	map(0xfff9, 0xfff9).r(FUNC(s8k_state::reg_if1l_r));

	// ------------ ZBI peripherals (TESTING!!!) ----------
	map(0x0000, 0x0000).rw(FUNC(s8k_state::ecc_reg_r), FUNC(s8k_state::ecc_reg_w));
}

void s8k_state::spec_io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00f0, 0x20fc).rw(FUNC(s8k_state::mmu_cmd_r), FUNC(s8k_state::mmu_cmd_w));
}

void s8k_state::program_mem(address_map &map)
{
	map.unmap_value_low();
}

void s8k_state::data_mem(address_map &map)
{
	map.unmap_value_low();
}

void s8k_state::stack_mem(address_map &map)
{
	map.unmap_value_low();
}

z8010_device *s8k_state::select_code_mmu(offs_t offset)
{
	z8010_device *mmu = m_mmu_code;
	uint8_t seg = (uint8_t)(offset >> 16);

	LOG("%s CODE MMU SELECT, offset: %06x\n", machine().describe_context(), offset);

	if(m_is_seg_os)
	{
		if(m_is_seg_user && m_normal_led)	//Use normal LED as N/S indicator
		{
			if(seg < 64)
				mmu = m_mmu_data;
			else
				mmu = m_mmu_stck;
		}
	}
	else	// Non-seg OS
	{
		if((seg & 0x3f) < 2)	// OS segs 0,1,64,65
		{
			if(m_normal_led)	// Trying to access in normal mode?
			{
				// SEGTRAP!
				m_reg_snvr = seg;
				m_reg_if1l = offset;
				mmu = nullptr;
				segt_interrupt(1);
			}
		}
		else if(m_is_seg_user)
		{
			if(seg < 64)
				mmu = m_mmu_data;
			else
				mmu = m_mmu_stck;
		}
	}

	return mmu;
}

z8010_device *s8k_state::select_data_mmu(offs_t offset)
{
	z8010_device *mmu = m_mmu_stck;
	uint8_t seg = (uint8_t)(offset >> 16);
	uint8_t seg_offs = (uint8_t)(offset >> 8);

	LOG("%s DATA MMU SELECT, offset: %06x\n", machine().describe_context(), offset);

	if(m_is_seg_os)
	{
		if(!m_normal_led)	// System mode
		{
			mmu = m_mmu_code;
		}
		else if(m_is_seg_user)
		{
			if(seg < 64)
			{
				mmu = m_mmu_data;
			}
		}
		else if(seg_offs < m_reg_nbr)
		{
			mmu = m_mmu_data;
		}
	}
	else	// Non-seg OS
	{
		if((seg & 0x3f) < 2)	// OS segs 0,1,64,65
		{
			if(m_normal_led)	// Trying to access in normal mode?
			{
				// SEGTRAP!
				m_reg_snvr = seg;
				m_reg_trpl = offset;
				mmu = nullptr;
				segt_interrupt(1);
			}
			else if(seg_offs < m_reg_sbr)
			{
				mmu = m_mmu_data;
			}
		}
		else if(m_is_seg_user)
		{
			if(seg < 64)
			{
				mmu = m_mmu_data;
			}
		}
		else if(seg_offs < m_reg_nbr)
		{
			mmu = m_mmu_data;
		}
	}

	return mmu;
}

bool s8k_state::translate_addr(int spacenum, bool write, offs_t &offset)
{
	bool stack_access = (spacenum == z8001_device::AS_STACK);

	offset <<= 1;

	if(stack_access)
	{
		m_ctc[0]->trg3(1);
		m_ctc[0]->trg3(0);
	}

	if(m_reg_scr & S8K_SCR_MMU_ONH)
	{
		bool code_access = (spacenum == AS_PROGRAM);
		z8010_device *mmu = code_access ?
							select_code_mmu(offset) : select_data_mmu(offset);

		if(mmu)
		{
			int st = code_access ?
						(m_maincpu->is_ifetch1() ?
							z8002_device::ST_IFETCH_1 :
							z8002_device::ST_IFETCH_N) :
						(stack_access ?
							z8002_device::ST_REQ_STACK :
							z8002_device::ST_REQ_DATA);

			LOG("%s MMU MEM REQ (space %d): %06x\n", machine().describe_context(), spacenum, offset);

			offset &= 0x3fffff;	// Mask off seg bit 7 to disable URS checking in MMUs

			return !mmu->translate(offset, write, true, m_busack_led, st);
		}
	}
	else if(offset < m_ram->size())
	{
		return true;
	}

	return false;
}

uint16_t s8k_state::ram_r(address_space &space, offs_t offset, uint16_t mask)
{
	if(translate_addr(space.spacenum(), false, offset))
	{
		const uint8_t *memptr = m_ram->pointer() + offset;

		return (((mask & 0xff00) & ((uint16_t)(memptr[0]) << 8)) |
				((mask & 0x00ff) & ((uint16_t)(memptr[1]))));
	}
	else if(space.spacenum() == AS_PROGRAM)
		return 0x8d07;	// NOP instruction
	else
		return 0;
}

void s8k_state::ram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mask)
{
	if(translate_addr(space.spacenum(), true, offset))
	{
		uint8_t *memptr = m_ram->pointer() + offset;

		if(mask & 0xff00)
			memptr[0] = (uint8_t)(data >> 8);
		if(mask & 0x00ff)
			memptr[1] = (uint8_t)(data);
	}
}

// NOTE: Stack address space is only used for m_ctc[0]->trg3(). Is this needed?
void s8k_state::install_memory()
{
	void *LROM, *LRAM;

	m_memsize = m_ram->size();

	address_space& pspace = m_maincpu->space(AS_PROGRAM);
	address_space& dspace = m_maincpu->space(AS_DATA);
	address_space& sspace = m_maincpu->space(z8001_device::AS_STACK);

	// Install handlers over entire address space
	pspace.install_readwrite_handler(0x000000, 0x7fffff,
		read16_delegate(*this, FUNC(s8k_state::ram_r)), write16_delegate(*this, FUNC(s8k_state::ram_w)));
	dspace.install_readwrite_handler(0x000000, 0x7fffff,
		read16_delegate(*this, FUNC(s8k_state::ram_r)), write16_delegate(*this, FUNC(s8k_state::ram_w)));
	sspace.install_readwrite_handler(0x000000, 0x7fffff,
		read16_delegate(*this, FUNC(s8k_state::ram_r)), write16_delegate(*this, FUNC(s8k_state::ram_w)));

	pspace.install_view(0x0000, 0x7fff, m_view_code);
	dspace.install_view(0x0000, 0x7fff, m_view_data);
	sspace.install_view(0x0000, 0x7fff, m_view_stck);

	LROM = memregion("maincpu")->base();
	LRAM = memshare("local_ram")->ptr();
	m_view_code[0].install_rom(0x0000, 0x3fff, LROM);
	m_view_data[0].install_rom(0x0000, 0x3fff, LROM);
	m_view_data[0].install_ram(0x4000, 0x47ff, LRAM);
	m_view_stck[0].install_ram(0x4000, 0x47ff, LRAM);
	//m_view_data[0].install_ram(0x7800, 0x7fff, LRAM);
}

//**************************************************************************
//  INTERRUPT HANDLING
//**************************************************************************

void s8k_state::segt_interrupt(int state)
{
	m_maincpu->set_input_line(z8001_device::SEGT_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

uint16_t s8k_state::segtack_r()
{
	uint16_t code = (m_mmu_code->segtack_r() |
					 m_mmu_data->segtack_r() |
					 m_mmu_stck->segtack_r());

	// Locally triggered?
	if(code == 0)
	{
		segt_interrupt(0);
	}

	LOG("%s SEGTRAP: %d\n", machine().describe_context(), code);

	return code;
}

uint16_t s8k_state::nmiack_r()
{
	uint16_t code = m_nmi_code;

	m_nmi_code = 0;
	//if(m_start_btn.read())
	//	code = S8K_NMI_MANUAL;
	//else if(/* power failure */)
	//	code = NMI_SRC_PWRFAIL;
	//else if(/* ECC error */)
	//	code = NMI_SRC_ECCERR;
	//else
	//{
	//	// Get code from ZBI bus
	//	code = m_zbi->read();
	//}

	m_maincpu->set_input_line(z8001_device::NMI_LINE, CLEAR_LINE);

	return code;
}

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

void s8k_state::normal_led_w(int state)
{
	m_normal_led = !state;

	LOG("%s NORMAL/SYSTEM MODE CHANGE: %d\n", machine().describe_context(), state);
}

//-------------------------------------------------
//  Z80PIO
//-------------------------------------------------

uint8_t s8k_state::pa_data_r()
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
	6       FAULT       printer fault (in(
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

void s8k_state::centronics_busy_w(uint8_t data)
{
	m_centronics_busy = data;
}
void s8k_state::centronics_select_w(uint8_t data)
{
	m_centronics_select = data;
}

void s8k_state::centronics_fault_w(uint8_t data)
{
	m_centronics_fault = data;
}

void s8k_state::centronics_ack_w(uint8_t data)
{
	m_centronics_ack = data;
}

//-------------------------------------------------
//  Z80 Daisy Chain
//-------------------------------------------------

void s8k_state::daisy_interrupt(int state)
{
	m_maincpu->set_input_line(z8001_device::VI_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

static const z80_daisy_config s8k_16_daisy_chain[] =
{
	{ "ctc0" },
	{ "ctc1" },
	{ "ctc2" },
	{ "sio0" },
	{ "sio1" },
	{ "sio2" },
	{ "sio3" },
	{ "pio0" },
	{ nullptr }
};

//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

void s8k_state::machine_start()
{
	install_memory();

	m_normal_led.resolve();
	m_normal_led = 0;
	m_busack_led.resolve();
	m_busack_led = 0;

	save_item(NAME(m_reg_scr));
	save_item(NAME(m_reg_sbr));
	save_item(NAME(m_reg_nbr));
	save_item(NAME(m_reg_snvr));
	save_item(NAME(m_reg_trpl));
	save_item(NAME(m_reg_if1l));
}

void s8k_state::machine_reset()
{
	m_reg_scr = (uint8_t)(m_dsw->read());
	m_is_seg_os = !!(m_jp_seg->read());
	m_is_seg_user = false;

	m_reg_sbr = 0;
	m_reg_nbr = 0;
	m_reg_snvr = 0;
	m_reg_trpl = 0;
	m_reg_if1l = 0;

	m_view_code.select(0);
	m_view_data.select(0);
	m_view_stck.select(0);
}

//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

void s8k_state::s8k(machine_config &config)
{
	/* basic machine hardware */
	Z8001(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &s8k_state::program_mem);
	m_maincpu->set_addrmap(AS_DATA, &s8k_state::data_mem);
	m_maincpu->set_addrmap(z8001_device::AS_STACK, &s8k_state::stack_mem);
	m_maincpu->set_addrmap(AS_IO, &s8k_state::io_map);
	m_maincpu->set_addrmap(z8001_device::AS_SIO, &s8k_state::spec_io_map);
	m_maincpu->segtack().set(FUNC(s8k_state::segtack_r));
	m_maincpu->nmiack().set(FUNC(s8k_state::nmiack_r));
	m_maincpu->viack().set("s8k_16_daisy", FUNC(s8k_16_daisy_device::viack_r));
	m_maincpu->ns().set(FUNC(s8k_state::normal_led_w));

	RAM(config, RAM_TAG).set_default_size("512K").set_default_value(0).set_extra_options("256K,512K,1M,2M,4M");

	S8K_16_DAISY(config, m_daisy, 0);
	m_daisy->set_daisy_config(s8k_16_daisy_chain);

	Z8010(config, m_mmu_code, MAIN_CLOCK);
	m_mmu_code->out_segt_cb().set(FUNC(s8k_state::segt_interrupt));
	Z8010(config, m_mmu_data, MAIN_CLOCK);
	m_mmu_data->out_segt_cb().set(FUNC(s8k_state::segt_interrupt));
	Z8010(config, m_mmu_stck, MAIN_CLOCK);
	m_mmu_stck->out_segt_cb().set(FUNC(s8k_state::segt_interrupt));

	config.set_default_layout(layout_s8k);

	/* peripheral hardware */
	Z80SIO(config, m_sio[0], MAIN_CLOCK);
	m_sio[0]->out_txda_callback().set("sio0:cha:tty0", FUNC(rs232_port_device::write_txd));
	m_sio[0]->out_rtsa_callback().set("sio0:cha:tty0", FUNC(rs232_port_device::write_rts));
	m_sio[0]->out_dtra_callback().set("sio0:cha:tty0", FUNC(rs232_port_device::write_dtr));
	m_sio[0]->out_txdb_callback().set("sio0:chb:console", FUNC(rs232_port_device::write_txd));
	m_sio[0]->out_rtsb_callback().set("sio0:chb:console", FUNC(rs232_port_device::write_rts));
	m_sio[0]->out_dtrb_callback().set("sio0:chb:console", FUNC(rs232_port_device::write_dtr));
	m_sio[0]->out_int_callback().set(FUNC(s8k_state::daisy_interrupt));

	rs232_port_device &rs232_0(RS232_PORT(config, "sio0:cha:tty0", default_rs232_devices, nullptr));
	rs232_0.rxd_handler().set(m_sio[0], FUNC(z80sio_device::rxa_w));
	rs232_0.cts_handler().set(m_sio[0], FUNC(z80sio_device::ctsa_w));
	rs232_port_device &rs232_1(RS232_PORT(config, "sio0:chb:console", default_rs232_devices, "terminal"));
	rs232_1.rxd_handler().set(m_sio[0], FUNC(z80sio_device::rxb_w));
	rs232_1.cts_handler().set(m_sio[0], FUNC(z80sio_device::ctsb_w));
	//rs232_1.dcd_handler().set(m_sio0, FUNC(z80sio_device::dcdb_w));
	//rs232_1.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	Z80SIO(config, m_sio[1], MAIN_CLOCK);
	m_sio[1]->out_txda_callback().set("sio1:cha:tty2", FUNC(rs232_port_device::write_txd));
	m_sio[1]->out_rtsa_callback().set("sio1:cha:tty2", FUNC(rs232_port_device::write_rts));
	m_sio[1]->out_dtra_callback().set("sio1:cha:tty2", FUNC(rs232_port_device::write_dtr));
	m_sio[1]->out_txdb_callback().set("sio1:chb:tty3", FUNC(rs232_port_device::write_txd));
	m_sio[1]->out_rtsb_callback().set("sio1:chb:tty3", FUNC(rs232_port_device::write_rts));
	m_sio[1]->out_dtrb_callback().set("sio1:chb:tty3", FUNC(rs232_port_device::write_dtr));
	m_sio[1]->out_int_callback().set(FUNC(s8k_state::daisy_interrupt));

	rs232_port_device &rs232_2(RS232_PORT(config, "sio1:cha:tty2", default_rs232_devices, nullptr));
	rs232_2.rxd_handler().set(m_sio[1], FUNC(z80sio_device::rxa_w));
	rs232_2.cts_handler().set(m_sio[1], FUNC(z80sio_device::ctsa_w));
	rs232_port_device &rs232_3(RS232_PORT(config, "sio1:chb:tty3", default_rs232_devices, nullptr));
	rs232_3.rxd_handler().set(m_sio[1], FUNC(z80sio_device::rxb_w));
	rs232_3.cts_handler().set(m_sio[1], FUNC(z80sio_device::ctsb_w));

	Z80SIO(config, m_sio[2], MAIN_CLOCK);
	m_sio[2]->out_txda_callback().set("sio2:cha:tty4", FUNC(rs232_port_device::write_txd));
	m_sio[2]->out_rtsa_callback().set("sio2:cha:tty4", FUNC(rs232_port_device::write_rts));
	m_sio[2]->out_dtra_callback().set("sio2:cha:tty4", FUNC(rs232_port_device::write_dtr));
	m_sio[2]->out_txdb_callback().set("sio2:chb:tty5", FUNC(rs232_port_device::write_txd));
	m_sio[2]->out_rtsb_callback().set("sio2:chb:tty5", FUNC(rs232_port_device::write_rts));
	m_sio[2]->out_dtrb_callback().set("sio2:chb:tty5", FUNC(rs232_port_device::write_dtr));
	m_sio[2]->out_int_callback().set(FUNC(s8k_state::daisy_interrupt));

	rs232_port_device &rs232_4(RS232_PORT(config, "sio2:cha:tty4", default_rs232_devices, nullptr));
	rs232_4.rxd_handler().set(m_sio[2], FUNC(z80sio_device::rxa_w));
	rs232_4.cts_handler().set(m_sio[2], FUNC(z80sio_device::ctsa_w));
	rs232_port_device &rs232_5(RS232_PORT(config, "sio2:chb:tty5", default_rs232_devices, nullptr));
	rs232_5.rxd_handler().set(m_sio[2], FUNC(z80sio_device::rxb_w));
	rs232_5.cts_handler().set(m_sio[2], FUNC(z80sio_device::ctsb_w));

	Z80SIO(config, m_sio[3], MAIN_CLOCK);
	m_sio[3]->out_txda_callback().set("sio3:cha:tty6", FUNC(rs232_port_device::write_txd));
	m_sio[3]->out_rtsa_callback().set("sio3:cha:tty6", FUNC(rs232_port_device::write_rts));
	m_sio[3]->out_dtra_callback().set("sio3:cha:tty6", FUNC(rs232_port_device::write_dtr));
	m_sio[3]->out_txdb_callback().set("sio3:chb:tty7", FUNC(rs232_port_device::write_txd));
	m_sio[3]->out_rtsb_callback().set("sio3:chb:tty7", FUNC(rs232_port_device::write_rts));
	m_sio[3]->out_dtrb_callback().set("sio3:chb:tty7", FUNC(rs232_port_device::write_dtr));
	m_sio[3]->out_int_callback().set(FUNC(s8k_state::daisy_interrupt));

	rs232_port_device &rs232_6(RS232_PORT(config, "sio3:cha:tty6", default_rs232_devices, nullptr));
	rs232_6.rxd_handler().set(m_sio[3], FUNC(z80sio_device::rxa_w));
	rs232_6.cts_handler().set(m_sio[3], FUNC(z80sio_device::ctsa_w));
	rs232_port_device &rs232_7(RS232_PORT(config, "sio3:chb:tty7", default_rs232_devices, nullptr));
	rs232_7.rxd_handler().set(m_sio[3], FUNC(z80sio_device::rxb_w));
	rs232_7.cts_handler().set(m_sio[3], FUNC(z80sio_device::ctsb_w));

	Z80CTC(config, m_ctc[0], MAIN_CLOCK);
	m_ctc[0]->set_clk<0>(CTC_CLOCK);
	m_ctc[0]->set_clk<1>(CTC_CLOCK);
	m_ctc[0]->set_clk<2>(CTC_CLOCK);
	m_ctc[0]->zc_callback<0>().set(m_sio[0], FUNC(z80sio_device::rxca_w));
	m_ctc[0]->zc_callback<0>().append(m_sio[0], FUNC(z80sio_device::txca_w));
	m_ctc[0]->zc_callback<1>().set(m_sio[0], FUNC(z80sio_device::rxcb_w));
	m_ctc[0]->zc_callback<1>().append(m_sio[0], FUNC(z80sio_device::txcb_w));
	m_ctc[0]->zc_callback<2>().set(m_sio[1], FUNC(z80sio_device::rxca_w));
	m_ctc[0]->zc_callback<2>().append(m_sio[1], FUNC(z80sio_device::txca_w));
	m_ctc[0]->intr_callback().set(FUNC(s8k_state::daisy_interrupt));

	Z80CTC(config, m_ctc[1], MAIN_CLOCK);
	m_ctc[1]->set_clk<0>(CTC_CLOCK);
	m_ctc[1]->set_clk<1>(CTC_CLOCK);
	m_ctc[1]->set_clk<2>(CTC_CLOCK);
	m_ctc[1]->set_clk<3>(CTC_CLOCK);
	m_ctc[1]->zc_callback<0>().set(m_sio[1], FUNC(z80sio_device::rxcb_w));
	m_ctc[1]->zc_callback<0>().append(m_sio[1], FUNC(z80sio_device::txcb_w));
	m_ctc[1]->zc_callback<1>().set(m_sio[2], FUNC(z80sio_device::rxca_w));
	m_ctc[1]->zc_callback<1>().append(m_sio[2], FUNC(z80sio_device::txca_w));
	m_ctc[1]->zc_callback<2>().set(m_sio[2], FUNC(z80sio_device::rxcb_w));
	m_ctc[1]->zc_callback<2>().append(m_sio[2], FUNC(z80sio_device::txcb_w));
	m_ctc[1]->intr_callback().set(FUNC(s8k_state::daisy_interrupt));

	Z80CTC(config, m_ctc[2], MAIN_CLOCK);
	m_ctc[2]->set_clk<0>(CTC_CLOCK);
	m_ctc[2]->set_clk<1>(CTC_CLOCK);
	m_ctc[2]->set_clk<2>(CTC_CLOCK);
	m_ctc[2]->zc_callback<0>().set(m_sio[3], FUNC(z80sio_device::rxca_w));
	m_ctc[2]->zc_callback<0>().append(m_sio[3], FUNC(z80sio_device::txca_w));
	m_ctc[2]->zc_callback<1>().set(m_sio[3], FUNC(z80sio_device::rxcb_w));
	m_ctc[2]->zc_callback<1>().append(m_sio[3], FUNC(z80sio_device::txcb_w));
	m_ctc[2]->intr_callback().set(FUNC(s8k_state::daisy_interrupt));

	Z80PIO(config, m_pio, MAIN_CLOCK);
	m_pio->in_pa_callback().set(FUNC(s8k_state::pa_data_r));
	m_pio->out_pb_callback().set("pio0:printer1:data_out", FUNC(output_latch_device::write));
	//m_pio->out_brdy_callback().set("pio0:printer1:data_out", FUNC(output_latch_device::write));
	m_pio->out_int_callback().set(FUNC(s8k_state::daisy_interrupt));

	centronics_device &centronics(CENTRONICS(config, "pio0:printer1", centronics_devices, nullptr));
	centronics.strobe_handler().set(m_pio, FUNC(z80pio_device::strobe_a));
	centronics.busy_handler().set(FUNC(s8k_state::centronics_busy_w));
	centronics.select_handler().set(FUNC(s8k_state::centronics_select_w));
	centronics.fault_handler().set(FUNC(s8k_state::centronics_fault_w));
	centronics.ack_handler().set(FUNC(s8k_state::centronics_ack_w));

	output_latch_device &latch(OUTPUT_LATCH(config, "pio0:printer1:data_out"));
	centronics.set_output_latch(latch);
}

//**************************************************************************
//  ROMS
//**************************************************************************

ROM_START( s8000 )
	ROM_REGION16_BE(0x4000,"maincpu", 0)

	ROM_SYSTEM_BIOS( 0, "v12", "Version 1.2" )
	ROMX_LOAD("cpu-12.u74", 0x0001, 0x1000, CRC(ab2ca534) SHA1(25857479801397f1f18c55b40f81cb5ba7a01a55), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("cpu-12.u75", 0x2001, 0x1000, CRC(c8d3be3b) SHA1(efbca6fcbf53565075b67a14096d0f725839494a), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("cpu-12.u76", 0x0000, 0x1000, CRC(81a4cac6) SHA1(9e74883a365f1034610b1c4681ca3611362d62ea), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("cpu-12.u77", 0x2000, 0x1000, CRC(6b8b4536) SHA1(85ff7c9be0f51e299d4f9406064cd32df08b8f16), ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS( 1, "v22", "Version 2.2" )
	ROMX_LOAD("cpu-22.u74", 0x0001, 0x1000, CRC(8a3ea482) SHA1(0572b21ac5aeb24cec01d7682f1ad7eef08cb070), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("cpu-22.u75", 0x2001, 0x1000, CRC(8ddb6479) SHA1(93eec5a59a7856d19e32f526dddb4f21c1864373), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("cpu-22.u76", 0x0000, 0x1000, CRC(198ce8ee) SHA1(743d75dab6f4ea85b2f95ec1b620134f4416a351), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("cpu-22.u77", 0x2000, 0x1000, CRC(43660a81) SHA1(8398d1998384ea0a95fcad58f791d9657e023b83), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

//TODO: Series Two (HPCPU) Models
/*
ROM_START( s8000_s2 )
	ROM_REGION16_BE(0x4000,"maincpu", 0)

	ROM_SYSTEM_BIOS( 0, "v101", "Version 10.1" )
	ROMX_LOAD("cpu-101.19e", 0x0000, 0x2000, CRC(a77055c8) SHA1(f1268c6b163f9e4151d425ccc2f5cb4c9c0af8c8), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("cpu-101.21e", 0x0001, 0x2000, CRC(610e8b0c) SHA1(bc804e09cf6905c9f0ccc502a15c98841ee4b087), ROM_SKIP(1) | ROM_BIOS(0))
ROM_END
*/

} // anonymous namespace

//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY  FULLNAME       FLAGS
COMP( 1982, s8000, 0,      0,      s8k,     s8k,   s8k_state, empty_init, "Zilog", "System 8000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
