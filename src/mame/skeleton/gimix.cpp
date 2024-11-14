// license:BSD-3-Clause
// copyright-holders:Barry Rodewald, 68bit
/*
    Gimix 6809-Based Computers

    Representative group of GIMIX users included:  Government Research and Scientific Organizations, Universities, Industrial, Computer Mainframe and
    Peripheral Manufacturers and Software Houses.

    This system is most notable for being the development base of the "Vid Kidz", a pair of programmers (Eugene Jarvis and Larry DeMar) who formerly
    worked for Williams Electronics on the game, Defender.  They left Willams and continued work on other games eventually making a deal with Williams
    to continue to design games producing the titles: Stargate, Robotron: 2084 and Blaster.

    Information Link:  http://www.backglass.org/duncan/gimix/

    TODO: Hard disk support

    Usage:
    System boots into GMXBUG-09
    To boot Flex, insert the Flex system disk (3.3 or later, must support the DMA disk controller), type U and press enter.
    To boot OS-9, select ROM version, insert the OS-9 system disk, type O, and press Enter.
*/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "imagedev/floppy.h"
#include "machine/input_merger.h"
#include "machine/mm58167.h"
#include "machine/6840ptm.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"
#include "machine/wd_fdc.h"
#include "machine/bankdev.h"
#include "machine/ram.h"
#include "formats/flex_dsk.h"
#include "formats/os9_dsk.h"
#include "softlist_dev.h"


namespace {

#define DMA_DRQ         (m_dma_status & 0x80)
#define DMA_INTRQ       (m_dma_status & 0x40)
#define DMA_MOTOR_DELAY (m_dma_status & 0x20)
#define DMA_ENABLED     (m_dma_status & 0x10)
#define DMA_FAULT       (m_dma_status & 0x08)
#define DMA_DRIVE_SIZE  (m_dma_status & 0x04)
#define DMA_DIP_SENSE   (m_dma_status & 0x01)

#define DMA_CONNECT_SEL (m_dma_drive_select & 0x40)
#define DMA_DENSITY     (m_dma_drive_select & 0x20)
#define DMA_WR_PROTECT  (m_dma_drive_select & 0x10)
#define DMA_SEL_DRV3    (m_dma_drive_select & 0x08)
#define DMA_SEL_DRV2    (m_dma_drive_select & 0x04)
#define DMA_SEL_DRV1    (m_dma_drive_select & 0x02)
#define DMA_SEL_DRV0    (m_dma_drive_select & 0x01)

#define DMA_IRQ_ENABLE  (m_dma_ctrl & 0x80)
#define DMA_SIDE_SEL    (m_dma_ctrl & 0x40)
#define DMA_DIRECTION   (m_dma_ctrl & 0x20)
#define DMA_ENABLE      (m_dma_ctrl & 0x10)
#define DMA_BANK        (m_dma_ctrl & 0x0f)

#define DMA_START_ADDR  (((m_dma_ctrl & 0x0f) << 16) | m_dma_start_addr)

class gimix_state : public driver_device
{
public:
	gimix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_irqs(*this, "irqs")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_ram(*this, RAM_TAG)
		, m_rom(*this, "roms")
		, m_acia1(*this, "acia1")
		, m_acia2(*this, "acia2")
		, m_acia3(*this, "acia3")
		, m_acia4(*this, "acia4")
		, m_bank(*this, "bank%u", 1U)
		, m_rombank1(*this, "rombank1")
		, m_rombank2(*this, "rombank2")
		, m_fixedrombank(*this, "fixedrombank")
		, m_lowerram(*this, "lower_ram")
		, m_upperram(*this, "upper_ram")
		, m_dma_dip(*this, "dma_s2")
	{}

	void gimix(machine_config &config);

private:
	void system_w(offs_t offset, uint8_t data);
	void fdc_irq_w(int state);
	void fdc_drq_w(int state);
	uint8_t dma_r(offs_t offset);
	void dma_w(offs_t offset, uint8_t data);
	uint8_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint8_t data);
	uint8_t pia_pa_r();
	void pia_pa_w(uint8_t data);
	uint8_t pia_pb_r();
	void pia_pb_w(uint8_t data);

	static void floppy_formats(format_registration &fr);

	void gimix_banked_mem(address_map &map) ATTR_COLD;
	void gimix_mem(address_map &map) ATTR_COLD;

	// disassembly override
	offs_t os9_dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params);
	offs_t dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params);

	bool m_fpla_sw_latch;
	uint8_t m_term_data;
	uint8_t m_dma_status;
	uint8_t m_dma_ctrl;
	uint8_t m_dma_drive_select;
	uint16_t m_dma_start_addr;
	uint32_t m_dma_current_addr;
	uint8_t m_task;
	uint8_t m_task_banks[16][16];
	uint8_t m_selected_drive;
	bool m_floppy_ready[4];

	uint8_t m_pia1_pa;
	uint8_t m_pia1_pb;

	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void driver_start() override;

	void refresh_memory();

	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_irqs;
	required_device<fd1797_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_device<acia6850_device> m_acia1;
	required_device<acia6850_device> m_acia2;
	required_device<acia6850_device> m_acia3;
	required_device<acia6850_device> m_acia4;

	required_device_array<address_map_bank_device, 16> m_bank;
	required_memory_bank m_rombank1;
	required_memory_bank m_rombank2;
	required_memory_bank m_fixedrombank;
	required_memory_bank m_lowerram;
	memory_bank_creator m_upperram;

	required_ioport m_dma_dip;
};

void gimix_state::gimix_banked_mem(address_map &map)
{
	map(0x00000, 0x0dfff).bankrw("lower_ram");
	map(0x0e000, 0x0e001).rw(m_acia1, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x0e004, 0x0e005).rw(m_acia2, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	//map(0x0e018, 0x0e01b).rw(FUNC(gimix_state::fdc_r), FUNC(gimix_state::fdc_w));  // FD1797 FDC (PIO)
	map(0x0e100, 0x0e1ff).ram();
	//map(0x0e200, 0x0e20f) // 9511A / 9512 Arithmetic Processor
	map(0x0e210, 0x0e21f).rw("timer", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0x0e220, 0x0e23f).rw("rtc", FUNC(mm58167_device::read), FUNC(mm58167_device::write));
	map(0x0e240, 0x0e3af).ram();
	map(0x0e3b0, 0x0e3b3).rw(FUNC(gimix_state::dma_r), FUNC(gimix_state::dma_w));  // DMA controller (custom?)
	map(0x0e3b4, 0x0e3b7).rw(FUNC(gimix_state::fdc_r), FUNC(gimix_state::fdc_w));  // FD1797 FDC
	map(0x0e400, 0x0e7ff).ram();  // scratchpad RAM
	map(0x0e800, 0x0efff).ram();
	map(0x0f000, 0x0f7ff).bankr("rombank2");
	map(0x0f800, 0x0ffff).bankr("rombank1");
	//map(0x10000, 0x1ffff).ram();
}

void gimix_state::gimix_mem(address_map &map)
{
	for (int bank = 0; bank < 16; bank++)
	{
		map(bank << 12, (bank << 12) | 0x0fff).rw(m_bank[bank], FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
	}
	map(0xff00, 0xffff).bankr("fixedrombank").w(FUNC(gimix_state::system_w));
}

static INPUT_PORTS_START( gimix )
	PORT_START("dma_s2")
	PORT_DIPNAME(0x00000100,0x00000000,"5.25\" / 8\" floppy drive 0") PORT_DIPLOCATION("S2:9")
	PORT_DIPSETTING(0x00000000,"5.25\"")
	PORT_DIPSETTING(0x00000100,"8\"")

INPUT_PORTS_END

void gimix_state::refresh_memory()
{
	for (int bank = 0; bank < 16; bank++)
	{
		m_bank[bank]->set_bank(m_task_banks[m_task][bank]);
	}
}

void gimix_state::system_w(offs_t offset, uint8_t data)
{
	if(offset == 0x7f)  // task register
	{
		if(data & 0x20)  // FPLA software latch
		{
			m_rombank1->set_entry(2);
			m_rombank2->set_entry(3);
			m_fixedrombank->set_entry(2);
			m_fpla_sw_latch = true;
			logerror("SYS: FPLA software latch set\n");
		}
		else
		{
			m_rombank1->set_entry(0);
			m_rombank2->set_entry(1);
			m_fixedrombank->set_entry(0);
			m_fpla_sw_latch = false;
			logerror("SYS: FPLA software latch reset\n");
		}
		m_task = data & 0x0f;
		refresh_memory();
		logerror("SYS: Task set to %02x\n",data & 0x0f);
	}
	if(offset >= 0xf0)  // Dynamic Address Translation RAM (write only)
	{
		m_bank[offset-0xf0]->set_bank(data & 0x0f);
		m_task_banks[m_task][offset-0xf0] = data & 0x0f;
		logerror("SYS: Bank %i set to physical bank %02x\n",offset-0xf0,data);
	}
}

uint8_t gimix_state::dma_r(offs_t offset)
{
	switch(offset)
	{
	case 0:
		if(m_dma_dip->read() & 0x00000100)
			m_dma_status |= 0x01;   // 8"
		else
			m_dma_status &= ~0x01;  // 5.25"
		return m_dma_status;
	case 1:
		return m_dma_ctrl;
	case 2:
		return (m_dma_start_addr & 0xff00) >> 8;
	case 3:
		return (m_dma_start_addr & 0x00ff);
	default:
		logerror("DMA: Unknown or invalid DMA register %02x read\n",offset);
	}
	return 0xff;
}

void gimix_state::dma_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
	case 0:
		logerror("DMA: Drive select %02x\n",data);
		m_dma_drive_select = data;
		m_fdc->dden_w(DMA_DENSITY ? 1 : 0);
		if(data & 0x40)  // 8" / 5.25" connector select
		{
			// 8 inch
			m_dma_status |= 0x04;
			m_fdc->set_unscaled_clock(8_MHz_XTAL / 4); // (2MHz)
		}
		else
		{
			// 5.25 inch
			m_dma_status &= ~0x04;
			m_fdc->set_unscaled_clock(8_MHz_XTAL / 8); // (1MHz)
		}

		if(data & 0x01)
		{
			m_selected_drive = 1;
			m_fdc->set_floppy(m_floppy[0]->get_device());
		}
		else if(data & 0x02)
		{
			m_selected_drive = 2;
			m_fdc->set_floppy(m_floppy[1]->get_device());
		}
		else if(data & 0x04)
		{
			m_selected_drive = 3;
			m_fdc->set_floppy(m_floppy[2]->get_device());
		}
		else if(data & 0x08)
		{
			m_selected_drive = 4;
			m_fdc->set_floppy(m_floppy[3]->get_device());
		}
		else
		{
			m_selected_drive = 0;
			m_fdc->set_floppy(nullptr);
		}

		for(int n = 0; n < 4; n++)
		{
			if(m_selected_drive != n + 1)
			{
				m_floppy[n]->get_device()->mon_w(1);
				m_floppy_ready[n] = false;
				logerror("FDC: Floppy drive %d motor off\n", n);
			}
		}
		break;
	case 1:
		logerror("DMA: DMA control %02x\n",data);
		m_dma_ctrl = data;
		if(data & 0x10)
			m_dma_status |= 0x12;
		else
			m_dma_status &= ~0x12;
		if(m_selected_drive != 0)
			m_floppy[m_selected_drive - 1]->get_device()->ss_w(BIT(data, 6));
		if((data & 0x80) == 0)
			m_irqs->in_w<6>(0);
		break;
	case 2:
		logerror("DMA: DMA start address MSB %02x\n",data);
		m_dma_start_addr = (m_dma_start_addr & 0x00ff) | (data << 8);
		m_dma_current_addr = DMA_START_ADDR;
		break;
	case 3:
		logerror("DMA: DMA start address LSB %02x\n",data);
		m_dma_start_addr = (m_dma_start_addr & 0xff00) | data;
		m_dma_current_addr = DMA_START_ADDR;
		break;
	default:
		logerror("DMA: Unknown or invalid DMA register %02x write %02x\n",offset,data);
	}
}

uint8_t gimix_state::fdc_r(offs_t offset)
{
	// motors are switched on on FDC access
	if(m_selected_drive != 0 && !m_floppy_ready[m_selected_drive - 1] && !machine().side_effects_disabled())
	{
		m_floppy[m_selected_drive - 1]->get_device()->mon_w(0);
		m_floppy_ready[m_selected_drive - 1] = true;
		logerror("FDC: Floppy drive %d motor on\n", m_selected_drive - 1);
	}
	return m_fdc->read(offset);
}

void gimix_state::fdc_w(offs_t offset, uint8_t data)
{
	// motors are switched on on FDC access
	if(m_selected_drive != 0 && !m_floppy_ready[m_selected_drive - 1])
	{
		m_floppy[m_selected_drive - 1]->get_device()->mon_w(0);
		m_floppy_ready[m_selected_drive - 1] = true;
		logerror("FDC: Floppy drive %d motor on\n", m_selected_drive - 1);
	}
	m_fdc->write(offset,data);
}

uint8_t gimix_state::pia_pa_r()
{
	return m_pia1_pa;
}

void gimix_state::pia_pa_w(uint8_t data)
{
	m_pia1_pa = data;
	logerror("PIA: Port A write %02x\n",data);
}

uint8_t gimix_state::pia_pb_r()
{
	return m_pia1_pb;
}

void gimix_state::pia_pb_w(uint8_t data)
{
	m_pia1_pb = data;
	logerror("PIA: Port B write %02x\n",data);
}


void gimix_state::fdc_irq_w(int state)
{
	if(state)
		m_dma_status |= 0x40;
	else
		m_dma_status &= ~0x40;

	if (DMA_IRQ_ENABLE)
		m_irqs->in_w<6>(state);
	else
		m_irqs->in_w<6>(0);
}

void gimix_state::fdc_drq_w(int state)
{
	if(state && DMA_ENABLED)
	{
		m_dma_status |= 0x80;
		// do a DMA transfer
		if(DMA_DIRECTION)
		{
			// write to disk
			m_fdc->data_w(m_ram->read(m_dma_current_addr));
//          logerror("DMA: read from RAM %05x\n",m_dma_current_addr);
		}
		else
		{
			// read from disk
			m_ram->write(m_dma_current_addr,m_fdc->data_r());
//          logerror("DMA: write to RAM %05x\n",m_dma_current_addr);
		}
		m_dma_current_addr++;
	}
	else
		m_dma_status &= ~0x80;
}

void gimix_state::machine_reset()
{
	m_term_data = 0;
	m_rombank1->set_entry(0);  // RAM banks are undefined on startup
	m_rombank2->set_entry(1);
	m_fixedrombank->set_entry(0);
	m_fpla_sw_latch = false;
	m_dma_status = 0x00;
	m_dma_ctrl = 0x00;
	m_irqs->in_w<6>(0);
	m_task = 0x00;
	m_selected_drive = 0;
	std::fill(std::begin(m_floppy_ready), std::end(m_floppy_ready), false);
	m_lowerram->set_base(m_ram->pointer());
	if(m_ram->size() > 65536)
		m_upperram->set_base(m_ram->pointer()+0x10000);

	// initialise FDC clock based on DIP Switch S2-9 (5.25"/8" drive select)
	if(m_dma_dip->read() & 0x00000100)
		m_fdc->set_unscaled_clock(8_MHz_XTAL / 4); // 8 inch (2MHz)
	else
		m_fdc->set_unscaled_clock(8_MHz_XTAL / 8); // 5.25 inch (1MHz)
}

void gimix_state::machine_start()
{
	uint8_t* ROM = m_rom->base();
	m_rombank1->configure_entries(0,4,ROM,0x800);
	m_rombank2->configure_entries(0,4,ROM,0x800);
	m_fixedrombank->configure_entries(0,4,ROM+0x700,0x800);
	m_rombank1->set_entry(0);  // RAM banks are undefined on startup
	m_rombank2->set_entry(1);
	m_fixedrombank->set_entry(0);
	m_fpla_sw_latch = false;
	// install any extra RAM
	if(m_ram->size() > 65536)
	{
		for (int bank = 0; bank < 16; bank++)
		{
			m_bank[bank]->space(AS_PROGRAM).install_readwrite_bank(0x10000,m_ram->size()-1,m_upperram);
		}
	}
	m_floppy[0]->get_device()->set_rpm(300);
	m_floppy[1]->get_device()->set_rpm(300);
}

void gimix_state::driver_start()
{
}

void gimix_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_FLEX_FORMAT);
	fr.add(FLOPPY_OS9_FORMAT);
}

static void gimix_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("8dd", FLOPPY_8_DSDD);
}

/***************************************************************************
  DISASSEMBLY OVERRIDE (OS9 syscalls)
 ***************************************************************************/

static const char *const os9syscalls[] =
{
	"F$Link",          // Link to Module
	"F$Load",          // Load Module from File
	"F$UnLink",        // Unlink Module
	"F$Fork",          // Start New Process
	"F$Wait",          // Wait for Child Process to Die
	"F$Chain",         // Chain Process to New Module
	"F$Exit",          // Terminate Process
	"F$Mem",           // Set Memory Size
	"F$Send",          // Send Signal to Process
	"F$Icpt",          // Set Signal Intercept
	"F$Sleep",         // Suspend Process
	"F$SSpd",          // Suspend Process
	"F$ID",            // Return Process ID
	"F$SPrior",        // Set Process Priority
	"F$SSWI",          // Set Software Interrupt
	"F$PErr",          // Print Error
	"F$PrsNam",        // Parse Pathlist Name
	"F$CmpNam",        // Compare Two Names
	"F$SchBit",        // Search Bit Map
	"F$AllBit",        // Allocate in Bit Map
	"F$DelBit",        // Deallocate in Bit Map
	"F$Time",          // Get Current Time
	"F$STime",         // Set Current Time
	"F$CRC",           // Generate CRC
	"F$GPrDsc",        // get Process Descriptor copy
	"F$GBlkMp",        // get System Block Map copy
	"F$GModDr",        // get Module Directory copy
	"F$CpyMem",        // Copy External Memory
	"F$SUser",         // Set User ID number
	"F$UnLoad",        // Unlink Module by name
	"F$Alarm",         // Color Computer Alarm Call (system wide)
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	"F$TPS",           // Return System's Ticks Per Second
	nullptr,
	"F$VIRQ",          // Install/Delete Virtual IRQ
	"F$SRqMem",        // System Memory Request
	"F$SRtMem",        // System Memory Return
	"F$IRQ",           // Enter IRQ Polling Table
	"F$IOQu",          // Enter I/O Queue
	"F$AProc",         // Enter Active Process Queue
	"F$NProc",         // Start Next Process
	"F$VModul",        // Validate Module
	"F$Find64",        // Find Process/Path Descriptor
	"F$All64",         // Allocate Process/Path Descriptor
	"F$Ret64",         // Return Process/Path Descriptor
	"F$SSvc",          // Service Request Table Initialization
	"F$IODel",         // Delete I/O Module
	"F$SLink",         // System Link
	"F$Boot",          // Bootstrap System
	"F$BtMem",         // Bootstrap Memory Request
	"F$GProcP",        // Get Process ptr
	"F$Move",          // Move Data (low bound first)
	"F$AllRAM",        // Allocate RAM blocks
	"F$AllImg",        // Allocate Image RAM blocks
	"F$DelImg",        // Deallocate Image RAM blocks
	"F$SetImg",        // Set Process DAT Image
	"F$FreeLB",        // Get Free Low Block
	"F$FreeHB",        // Get Free High Block
	"F$AllTsk",        // Allocate Process Task number
	"F$DelTsk",        // Deallocate Process Task number
	"F$SetTsk",        // Set Process Task DAT registers
	"F$ResTsk",        // Reserve Task number
	"F$RelTsk",        // Release Task number
	"F$DATLog",        // Convert DAT Block/Offset to Logical
	"F$DATTmp",        // Make temporary DAT image (Obsolete)
	"F$LDAXY",         // Load A [X,[Y]]
	"F$LDAXYP",        // Load A [X+,[Y]]
	"F$LDDDXY",        // Load D [D+X,[Y]]
	"F$LDABX",         // Load A from 0,X in task B
	"F$STABX",         // Store A at 0,X in task B
	"F$AllPrc",        // Allocate Process Descriptor
	"F$DelPrc",        // Deallocate Process Descriptor
	"F$ELink",         // Link using Module Directory Entry
	"F$FModul",        // Find Module Directory Entry
	"F$MapBlk",        // Map Specific Block
	"F$ClrBlk",        // Clear Specific Block
	"F$DelRAM",        // Deallocate RAM blocks
	"F$GCMDir",        // Pack module directory
	"F$AlHRam",        // Allocate HIGH RAM Blocks
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	"F$RegDmp",        // Ron Lammardo's debugging register dump call
	"F$NVRAM",         // Non Volatile RAM (RTC battery backed static) read/write
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	"I$Attach",        // Attach I/O Device
	"I$Detach",        // Detach I/O Device
	"I$Dup",           // Duplicate Path
	"I$Create",        // Create New File
	"I$Open",          // Open Existing File
	"I$MakDir",        // Make Directory File
	"I$ChgDir",        // Change Default Directory
	"I$Delete",        // Delete File
	"I$Seek",          // Change Current Position
	"I$Read",          // Read Data
	"I$Write",         // Write Data
	"I$ReadLn",        // Read Line of ASCII Data
	"I$WritLn",        // Write Line of ASCII Data
	"I$GetStt",        // Get Path Status
	"I$SetStt",        // Set Path Status
	"I$Close",         // Close Path
	"I$DeletX"         // Delete from current exec dir
};


//-------------------------------------------------
//  os9_dasm_override
//-------------------------------------------------

offs_t gimix_state::os9_dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)
{
	unsigned call;
	offs_t result = 0;

	// Microware OS-9 (on the Gimix) and a number of other 6x09 based
	// systems used the SWI2 instruction for syscalls.  This checks for a
	// SWI2 and looks up the syscall as appropriate.
	//
	// But only apply this override if the OS9 ROMs are latched on.
	if (!m_fpla_sw_latch)
		return 0;

	if ((opcodes.r8(pc) == 0x10) && (opcodes.r8(pc+1) == 0x3F))
	{
		call = opcodes.r8(pc+2);
		if ((call < std::size(os9syscalls)) && (os9syscalls[call] != nullptr))
		{
			util::stream_format(stream, "OS9   %s", os9syscalls[call]);
			result = 3;
		}
	}
	return result;
}


offs_t gimix_state::dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)
{
	return os9_dasm_override(stream, pc, opcodes, params);
}

void gimix_state::gimix(machine_config &config)
{
	// basic machine hardware
	MC6809(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &gimix_state::gimix_mem);
	m_maincpu->set_dasm_override(FUNC(gimix_state::dasm_override));

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	/* rtc */
	mm58167_device &rtc(MM58167(config, "rtc", 32.768_kHz_XTAL));
	rtc.irq().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	/* timer */
	ptm6840_device &ptm(PTM6840(config, "timer", 2'000'000));  // clock is a guess
	// PCB pictures show both the RTC and timer set to generate IRQs (are jumper configurable)
	ptm.irq_callback().set(m_irqs, FUNC(input_merger_device::in_w<1>));

	/* floppy disks */
	FD1797(config, m_fdc, 8_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(gimix_state::fdc_irq_w));
	m_fdc->drq_wr_callback().set(FUNC(gimix_state::fdc_drq_w));
	m_fdc->set_force_ready(true);
	FLOPPY_CONNECTOR(config, "fdc:0", gimix_floppies, "525hd", gimix_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", gimix_floppies, "525hd", gimix_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", gimix_floppies, "525hd", gimix_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", gimix_floppies, "525hd", gimix_state::floppy_formats).enable_sound(true);

	/* parallel ports */
	pia6821_device &pia1(PIA6821(config, "pia1", 2'000'000));
	pia1.writepa_handler().set(FUNC(gimix_state::pia_pa_w));
	pia1.writepb_handler().set(FUNC(gimix_state::pia_pb_w));
	pia1.readpa_handler().set(FUNC(gimix_state::pia_pa_r));
	pia1.readpb_handler().set(FUNC(gimix_state::pia_pb_r));

	PIA6821(config, "pia2", 2'000'000);

	/* serial ports */
	ACIA6850(config, m_acia1, 2'000'000);
	m_acia1->txd_handler().set("serial1", FUNC(rs232_port_device::write_txd));
	m_acia1->rts_handler().set("serial1", FUNC(rs232_port_device::write_rts));
	m_acia1->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<2>));

	ACIA6850(config, m_acia2, 2'000'000);
	m_acia2->txd_handler().set("serial2", FUNC(rs232_port_device::write_txd));
	m_acia2->rts_handler().set("serial2", FUNC(rs232_port_device::write_rts));
	m_acia2->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<3>));

	ACIA6850(config, m_acia3, 2'000'000);
	m_acia3->txd_handler().set("serial3", FUNC(rs232_port_device::write_txd));
	m_acia3->rts_handler().set("serial3", FUNC(rs232_port_device::write_rts));
	m_acia3->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<4>));

	ACIA6850(config, m_acia4, 2'000'000);
	m_acia4->txd_handler().set("serial4", FUNC(rs232_port_device::write_txd));
	m_acia4->rts_handler().set("serial4", FUNC(rs232_port_device::write_rts));
	m_acia4->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<5>));

	rs232_port_device &serial1(RS232_PORT(config, "serial1", default_rs232_devices, nullptr));
	serial1.rxd_handler().set(m_acia1, FUNC(acia6850_device::write_rxd));
	serial1.cts_handler().set(m_acia1, FUNC(acia6850_device::write_cts));

	rs232_port_device &serial2(RS232_PORT(config, "serial2", default_rs232_devices, "terminal"));
	serial2.rxd_handler().set(m_acia2, FUNC(acia6850_device::write_rxd));
	serial2.cts_handler().set(m_acia2, FUNC(acia6850_device::write_cts));

	rs232_port_device &serial3(RS232_PORT(config, "serial3", default_rs232_devices, nullptr));
	serial3.rxd_handler().set(m_acia3, FUNC(acia6850_device::write_rxd));
	serial3.cts_handler().set(m_acia3, FUNC(acia6850_device::write_cts));

	rs232_port_device &serial4(RS232_PORT(config, "serial4", default_rs232_devices, nullptr));
	serial4.rxd_handler().set(m_acia4, FUNC(acia6850_device::write_rxd));
	serial4.cts_handler().set(m_acia4, FUNC(acia6850_device::write_cts));

	clock_device &acia_clock(CLOCK(config, "acia_clock", 9600 * 16));
	acia_clock.signal_handler().set(m_acia1, FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia1, FUNC(acia6850_device::write_rxc));
	acia_clock.signal_handler().append(m_acia2, FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia2, FUNC(acia6850_device::write_rxc));

	/* banking */
	for (int bank = 0; bank < 16; bank++)
	{
		ADDRESS_MAP_BANK(config, m_bank[bank]).set_map(&gimix_state::gimix_banked_mem).set_options(ENDIANNESS_LITTLE, 8, 32, 0x1000);
	}

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("128K").set_extra_options("56K,256K,512K");

	SOFTWARE_LIST(config, "flop_list").set_original("gimix");
}

ROM_START( gimix )
	ROM_REGION( 0x10000, "roms", 0)
/* CPU board U4: gimixf8.bin  - checksum 68DB - 2716 - GMXBUG09 V2.1 | (c)1981 GIMIX | $F800 I2716 */
	ROM_LOAD( "gimixf8.u4",  0x000000, 0x000800, CRC(7d60f838) SHA1(eb7546e8bbf50d33e181f3e86c3e4c5c9032cab2) )
/* CPU board U5: gimixv14.bin - checksum 97E2 - 2716 - GIMIX 6809 | AUTOBOOT | V1.4 I2716 */
	ROM_LOAD( "gimixv14.u5", 0x000800, 0x000800, CRC(f795b8b9) SHA1(eda2de51cc298d94b36605437d900ce971b3b276) )

	ROM_SYSTEM_BIOS(0, "os9l1v11", "OS9 Level 1 version 1.1")
/* CPU board U6: os9p1-l1v11.bin - checksum 2C84 - 2716 - OS-9tmL1 V1 | GIMIX P1 " (c)1982 MSC
   CPU board U7: os9p2-l1v11.bin - checksum 7694 - 2716 - OS-9tmL1 V1 | GIMIX P2-68 | (c)1982 MSC */
	ROMX_LOAD( "os9p1-l1v11.u6", 0x001000, 0x000800, CRC(0d6527a0) SHA1(1435a22581c6e9e0ae338071a72eed646f429530), ROM_BIOS(0))
	ROMX_LOAD( "os9p2-l1v11.u7", 0x001800, 0x000800, CRC(b3c65feb) SHA1(19d1ea1e84473b25c95cbb8449e6b9828567e998), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "os9l1v12", "OS9 Level 1 version 1.2")
	ROMX_LOAD( "os9p1-l1v12.u6", 0x001000, 0x000800, CRC(4de6e313) SHA1(b32cbc07418a147fd33a4404a5c2f68c25616c0d), ROM_BIOS(1))
	ROMX_LOAD( "os9p2-l1v12.u7", 0x001800, 0x000800, CRC(22f5f128) SHA1(8abf5cd2a52c0b8286f717f9ddf7feca61d1f46d), ROM_BIOS(1))

/* Hard drive controller board 2 (XEBEC board) 11H: gimixhd.bin - checksum 2436 - 2732 - 104521D */
	ROM_REGION( 0x10000, "xebec", 0)
	ROM_LOAD( "gimixhd.h11",  0x000000, 0x001000, CRC(35c12201) SHA1(51ac9052f9757d79c7f5bd3aa5d8421e98cfcc37) )
ROM_END

} // anonymous namespace


COMP( 1980, gimix, 0, 0, gimix, gimix, gimix_state, empty_init, "Gimix", "Gimix 6809 System", MACHINE_NO_SOUND_HW )
