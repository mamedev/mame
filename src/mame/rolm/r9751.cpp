// license:GPL-2.0+
// copyright-holders:Brandon Munger, Stephen Stair
/******************************************************************************
*
* Rolm CBX 9751 Driver
*
* This driver attempts to emulate the following models:
* * Model 10
* * Model 20
* * Model 40
* * Model 50
* * Model 70
*
* The following software releases are known:
* * 9004
* * 9005
*
* The basis of this driver was influenced by the zexall.c driver by
* Jonathan Gevaryahu.
*
*
* Special Thanks to:
* * Stephen Stair (sgstair)   - for help with reverse engineering,
*               programming, and emulation
* * Felipe Sanches (FSanches) - for help with MESS and emulation
* * Michael Zapf (mizapf)     - for building the HDC9234/HDC9224
*               driver that makes this driver possible
*
* Memory map:
* * 0x00000000 - 0x00ffffff : RAM 12MB to 16MB known, up to 128MB?
* * 0x08000000 - 0x0800ffff : PROM Region
* * 0x5ff00000 - 0x5fffffff : System boards
* * 0xff010000 - 0xfff8ffff : CPU board registers
*
* Working:
* * Floppy Disk IO (PDC device)
* * SMIOC terminal (preliminary)
*
* TODO:
* * Identify registers required for OS booting
* * Hard disk support
* * SMIOC ports (1-8)
* * Identify various LED registers for system boards
* * ROLMLink (RLI) board support
* * Analog Telephone Interface (ATI) board support
* * T1 (T1DN) board support
*
******************************************************************************/

/* Core includes */
#include "emu.h"
#include "cpu/m68000/m68030.h"
#include "machine/terminal.h"

#include "machine/nscsi_bus.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"

#include "machine/wd33c9x.h"

#include "machine/pdc.h"
#include "machine/smioc.h"
#include "softlist.h"


namespace {

/* Log defines */
#define TRACE_FDC 0
#define TRACE_HDC 0
#define TRACE_LED 0
#define TRACE_DMA 0

/* Trace accesses to other boards in the system */
#define ENABLE_TRACE_ALL_DEVICES 0

/* Trace accesses to 68k system board registers */
#define ENABLE_TRACE_SYSTEM 0
#define SYSTEM_TRACE_LEVEL 5





class r9751_state : public driver_device
{
public:
	r9751_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pdc(*this, "pdc"),
		m_smioc(*this, "smioc"),
		m_wd33c93(*this, "scsi:7:wd33c93"),
		m_main_ram(*this, "main_ram")
	{
		device_trace_init();
		system_trace_init();
	}
	~r9751_state()
	{
		device_trace_teardown();
		system_trace_teardown();
	}

	void r9751(machine_config &config);

	void init_r9751();

private:

	uint32_t r9751_mmio_5ff_r(offs_t offset);
	void r9751_mmio_5ff_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t r9751_mmio_ff01_r(offs_t offset);
	void r9751_mmio_ff01_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t r9751_mmio_ff05_r(offs_t offset);
	void r9751_mmio_ff05_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t r9751_mmio_fff8_r(offs_t offset);
	void r9751_mmio_fff8_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint8_t pdc_dma_r(offs_t offset);
	void pdc_dma_w(uint8_t data);
	uint8_t smioc_dma_r(offs_t offset);
	void smioc_dma_w(offs_t offset, uint8_t data);

	void r9751_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<pdc_device> m_pdc;
	required_device<smioc_device> m_smioc;
	required_device<wd33c93_device> m_wd33c93;
	required_shared_ptr<uint32_t> m_main_ram;

	m68000_base_device* ptr_m68000 = nullptr;

	// Begin registers
	uint32_t reg_ff050004 = 0;
	uint32_t reg_fff80040 = 0;
	uint32_t fdd_dest_address = 0; // 5FF080B0
	uint32_t fdd_cmd_complete = 0;
	uint32_t smioc_dma_bank = 0;
	uint32_t fdd_dma_bank = 0;
	attotime timer_32khz_last;
	// End registers

	address_space *m_mem = nullptr;

	// functions
	uint32_t swap_uint32( uint32_t val );
	uint32_t debug_a6();
	uint32_t debug_a5();
	uint32_t debug_a5_20();
	[[maybe_unused]] void UnifiedTrace(u32 address, u32 data, const char* operation="Read", const char* Device="SMIOC", const char* RegisterName=nullptr, const char* extraText=nullptr);

	virtual void machine_reset() override ATTR_COLD;
	void trace_device(int address, int data, const char* direction);

	void system_trace_init();
	void system_trace_teardown();
	void trace_system(int table, int offset, int data, const char* direction);

	uint32_t m_device_trace_enable[2];
	//int m_trace_last_address[64];
	//int m_trace_last_data[64];
	//int m_trace_repeat_count[64];
	//char const * m_trace_last_direction[64];
	//char const * m_generic_command_names[256];
	//char const ** m_specific_command_tables[64];
	void device_trace_init();
	void device_trace_teardown();
	void device_trace_enable_all();
	void device_trace_disable(int device);

	void* system_trace_context;

	static void scsi_devices(device_slot_interface &device);
	void wd33c93(device_t *device);
};

#if ENABLE_TRACE_ALL_DEVICES
// Device tracing
static char const *const DeviceNames[] = {
	nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,"??", // 0x00-0x07
	"??","HDD",nullptr,nullptr,nullptr,nullptr,nullptr,nullptr, // 0x08-0x0F
	nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr, // 0x10-0x17
	nullptr,nullptr,nullptr,nullptr,"SMIOCb","SMIOCb?","SMIOCb?","SMIOCb?", // 0x18-0x1F - Unclear how wide the range is for the second SMIOC range.
	nullptr,nullptr,nullptr,nullptr,"SMIOCa","SMIOCa?","SMIOCa","SMIOCa", // 0x20-0x27 - 0x25 SMIOC we have not observed being used yet.
	"??",nullptr,nullptr,nullptr,"FDC",nullptr,nullptr,nullptr, // 0x28-0x2F
	"??",nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr, // 0x30-0x37
	"SMIOCc?",nullptr,"SMIOCd?",nullptr,nullptr,nullptr,nullptr,"??" // 0x38-0x3F
};

struct GenericCommandRecord {
	u32 AddressPattern;
	const char * Name;
};

static const GenericCommandRecord GenericCommands[] = {
	{ 0x200, "Clear Status" },
	{ 0x800, "Status" },
	{ 0x1000, "Word Count / Result" },
	{ 0x2800, "DMA Busy" },
	{ 0x3000, "Command Result" },
	{ 0x4000, "Command" },  // Also a write length.
	//{ 0x8000, "various" },  // Read address,
	//{ 0xC000, "various" },  // Write address, Read length
};

static const GenericCommandRecord SpecificCommands[] = {
	{ 0x5FF00198, "Special/reinit?" },
	{ 0x5FF08098, "Command Parameter" },
	{ 0x5FF0C098, "DMA Transmit Address" },
	{ 0x5FF0409C, "DMA Transmit Length" },
	{ 0x5FF0809C, "DMA Receive Address" },
	{ 0x5FF0C09C, "DMA Receive Length" },
	{ 0x5FF004B0, "Reset PDC" },
	{ 0x5FF010B0, "Clear result?" },
	{ 0x5FF041B0, "Old style Command" },
	{ 0x5FF080B0, "SCSI Command Address" },
	{ 0x5FF0C1B0, "DMA Receive Address" },
};


void r9751_state::device_trace_init()
{
	for (int i = 0; i < 64; i++)
	{
		m_trace_last_address[i] = -1;
		m_trace_last_data[i] = -1;
		m_trace_repeat_count[i] = 0;
		m_trace_last_direction[i] = nullptr;
		m_specific_command_tables[i] = nullptr;
	}
	for (int i = 0; i < 256; i++)
	{
		m_generic_command_names[i] = "?";
	}

	for (int i = 0; i < sizeof(GenericCommands) / sizeof(GenericCommands[0]); i++)
	{
		int index = (GenericCommands[i].AddressPattern & 0xFF00) >> 8;
		m_generic_command_names[index] = GenericCommands[i].Name;
	}

	for (int i = 0; i < sizeof(SpecificCommands) / sizeof(SpecificCommands[0]); i++)
	{
		int deviceIndex = (SpecificCommands[i].AddressPattern & 0xFC) >> 2;
		char const ** deviceTable = m_specific_command_tables[deviceIndex];
		if (deviceTable == nullptr)
		{
			deviceTable = new char const *[256];
			memset(deviceTable, 0, sizeof(*deviceTable) * 256);
			m_specific_command_tables[deviceIndex] = deviceTable;
		}

		int index = (SpecificCommands[i].AddressPattern & 0xFF00) >> 8;

		deviceTable[index] = SpecificCommands[i].Name;
	}
}
void r9751_state::device_trace_teardown()
{
	for (int i = 0; i < 64; i++)
	{
		delete[] m_specific_command_tables[i];
	}
}

void r9751_state::trace_device(int address, int data, const char* direction)
{
	int dev = (address & 0xFF) / 4;
	char const * devName = "?";
	char const * regName = "?";

	if (!(m_device_trace_enable[(dev >> 5)] & (1 << (dev & 31))))
	{
		// Device is not enabled for tracing.
		return;
	}

	if ((address == m_trace_last_address[dev]) && (data == m_trace_last_data[dev]) && (direction == m_trace_last_direction[dev]))
	{
		m_trace_repeat_count[dev]++;
		return;
	}
	// Compute Device and Register name
	if (DeviceNames[dev] != nullptr)
	{
		devName = DeviceNames[dev];
	}

	if (m_trace_repeat_count[dev] > 0)
	{
		regName = m_generic_command_names[(m_trace_last_address[dev] & 0xFF00) >> 8];
		logerror("Previous access on Device 0x%x (%s) to [%08x] (%s) repeated %d times\n", dev, devName, m_trace_last_address[dev], regName, m_trace_repeat_count[dev]);
	}
	m_trace_repeat_count[dev] = 0;
	m_trace_last_address[dev] = address;
	m_trace_last_data[dev] = data;
	m_trace_last_direction[dev] = direction;

	regName = m_generic_command_names[(address & 0xFF00) >> 8];
	if (m_specific_command_tables[dev] != nullptr)
	{
		int index = (address & 0xFF00) >> 8;
		if (m_specific_command_tables[dev][index] != nullptr)
		{
			regName = m_specific_command_tables[dev][index];
		}
	}


	u32 stacktrace[4];
	u32 basepointer[2];
	u32 reg_a6 = ptr_m68000->state_int(M68K_A6);

	for (int i = 0; i<4; i++)
		stacktrace[i] = 0;
	for (int i = 0; i<2; i++)
		basepointer[i] = 0;

	stacktrace[0] = m_maincpu->pc();
	if (reg_a6 + 4 < 0xFFFFFF) stacktrace[1] = m_maincpu->space(AS_PROGRAM).read_dword(reg_a6 + 4);
	if (reg_a6 < 0xFFFFFF && reg_a6 != 0) basepointer[0] = m_maincpu->space(AS_PROGRAM).read_dword(reg_a6);
	if (basepointer[0] + 4 < 0xFFFFFF && basepointer[0] != 0) stacktrace[2] = m_maincpu->space(AS_PROGRAM).read_dword(basepointer[0] + 4);
	if (basepointer[0] < 0xFFFFFF && basepointer[0] != 0) basepointer[1] = m_maincpu->space(AS_PROGRAM).read_dword(basepointer[0]);
	if (basepointer[1] + 4 < 0xFFFFFF && basepointer[1] != 0) stacktrace[3] = m_maincpu->space(AS_PROGRAM).read_dword(basepointer[1] + 4);

	logerror("%s Device 0x%x (%s) Register [%08X] (%s) %s %08X (%08X, %08X, %08X, %08X)\n", machine().time().as_string(), dev, devName, address, regName, direction, data, stacktrace[0], stacktrace[1], stacktrace[2], stacktrace[3]);

}

#else
void r9751_state::device_trace_init() {}
void r9751_state::device_trace_teardown() {}
void r9751_state::trace_device(int address, int data, const char* direction) {}
#endif

void r9751_state::device_trace_enable_all()
{
	for (int i = 0; i < sizeof(m_device_trace_enable) / sizeof(m_device_trace_enable[0]); i++)
	{
		m_device_trace_enable[i] = 0xFFFFFFFF;
	}
}
void r9751_state::device_trace_disable(int device)
{
	device = device & 0x3F;
	m_device_trace_enable[device >> 5] &= ~(1 << (device & 0x1F));
}


// System register tracing
struct SystemRegisterInfo {
	int VerboseLevel;
	u32 RegisterAddress;
	const char * RegisterName;
	const char * Description;
};

static const SystemRegisterInfo SystemRegisters[] = {
	{ 5, 0xff010000, "DB00" },{ 5, 0xff010004, "DB01" },{ 5, 0xff010008, "DB02" },{ 5, 0xff01000c, "DB03" },
	{ 5, 0xff010010, "DB04" },{ 5, 0xff010014, "DB05" },{ 5, 0xff010018, "DB06" },{ 5, 0xff01001c, "DB07" },
	{ 5, 0xff010020, "DB08" },{ 5, 0xff010024, "DB09" },{ 5, 0xff010028, "DB10" },{ 5, 0xff01002c, "DB11" },
	{ 5, 0xff010040, "DBVR" },{ 5, 0xff010044, "DTAR" },{ 5, 0xff010048, "DESR" },{ 5, 0xff01004c, "DVR" },
	{ 5, 0xff010050, "DRR" },{ 5, 0xff010054, "DDR" },{ 5, 0xff010060, "DMR" },{ 5, 0xff010064, "RCR" },
	{ 5, 0xff010068, "RESR" },{ 5, 0xff01006c, "RBSR" },{ 5, 0xff010070, "RBDR" },{ 5, 0xff010074, "RBAR" },
	{ 5, 0xff010078, "RIOB" },{ 5, 0xff01007c, "RPSR" },{ 5, 0xff010080, "PDR" },

	{ 5, 0xff020004, "ARSP" },{ 5, 0xff020008, "MEXC" },{ 5, 0xff02000c, "MADD" },{ 5, 0xff020010, "MDAT" },
	{ 5, 0xff020014, "ADIA" },{ 5, 0xff020018, "ECCD" },{ 5, 0xff02001c, "VPDS" },{ 5, 0xff020020, "HBRK" },
	{ 5, 0xff020024, "LBRK" },{ 5, 0xff020028, "REFS" },{ 5, 0xff02002c, "RCNT" },{ 5, 0xff020030, "RTMR" },

	{ 5, 0xff050004, "ROSR" },{ 5, 0xff050008, "BIVR" },{ 5, 0xff05000c, "RDDR", "LED - RAS Digit Display Register" },
	{ 5, 0xff050104, "AMR" },
	{ 5, 0xff050108, "MIVR" },{ 5, 0xff050200, "APCR" },{ 5, 0xff050204, "PESR" },{ 5, 0xff050208, "PEAR" },
	{ 5, 0xff05020c, "PEDR" },{ 5, 0xff050210, "PEST" },{ 5, 0xff050300, "RCID" },
	{ 2, 0xff050310, "APLT" }, // APLT is accessed heavily in background/timing processes.
	{ 2, 0xff050320, "WDT" }, // WDT use is particularly noisy
	{ 5, 0xff050400, "APT" },{ 5, 0xff050404, "AIVR" },{ 5, 0xff050408, "ASSR" },
	{ 5, 0xff050410, "STRU" },{ 5, 0xff050414, "STRL" },
	{ 5, 0xff050500, "EV00" },{ 5, 0xff050504, "EV01" },{ 5, 0xff050508, "EV02" },{ 5, 0xff05050c, "EV03" },
	{ 5, 0xff050510, "EV04" },{ 5, 0xff050514, "EV05" },{ 5, 0xff050518, "EV06" },{ 5, 0xff05051c, "EV07" },
	{ 5, 0xff050520, "EV08" },{ 5, 0xff050524, "EV09" },{ 5, 0xff050528, "EV10" },{ 5, 0xff05052c, "EV11" },
	{ 5, 0xff050530, "EV12" },{ 5, 0xff050534, "EV13" },{ 5, 0xff050538, "EV14" },{ 5, 0xff05053c, "EV15" },
	{ 5, 0xff050540, "EV16" },{ 5, 0xff050544, "EV17" },{ 5, 0xff050548, "EV18" },{ 5, 0xff05054c, "EV19" },
	{ 5, 0xff050550, "EV20" },{ 5, 0xff050554, "EV21" },{ 5, 0xff050558, "EV22" },{ 5, 0xff05055c, "EV23" },
	{ 5, 0xff050560, "EV24" },{ 5, 0xff050564, "EV25" },
	{ 5, 0xff050580, "PISR" },
	{ 2, 0xff050584, "PIMR" }, // PIMR use in the bios is noisy
	{ 5, 0xff050600, "CDMR" },{ 5, 0xff050604, "CDER" },
	{ 5, 0xff050610, "CIV0" },{ 5, 0xff050614, "CIV1" },{ 5, 0xff050618, "CIV2" },{ 5, 0xff05061c, "CIV3" },
	{ 5, 0xff050620, "CIV4" },{ 5, 0xff050624, "CIV5" },{ 5, 0xff050628, "CIV6" },{ 5, 0xff05062c, "CIV7" },
	{ 5, 0xff050630, "CCV0" },{ 5, 0xff050634, "CCV1" },{ 5, 0xff050638, "CCV2" },{ 5, 0xff05063c, "CCV3" },
	{ 5, 0xff050640, "CCV4" },{ 5, 0xff050644, "CCV5" },{ 5, 0xff050648, "CCV6" },{ 5, 0xff05064c, "CCV7" },

	{ 5, 0xff060014, "SCAS" },{ 5, 0xff060018, "SCRG" },{ 5, 0xff060020, "SDER" },{ 5, 0xff060024, "SDCR" },
	{ 5, 0xff060028, "SESR" },{ 5, 0xff06002c, "SBSR" },{ 5, 0xff060030, "SBDR" },{ 5, 0xff060034, "SBAR" },
	{ 5, 0xff060038, "SDAR" },{ 5, 0xff06003c, "SINT" },{ 5, 0xff060040, "SDXC" },{ 5, 0xff060044, "SOSR" },
	{ 5, 0xff060048, "SISR" },{ 5, 0xff06004c, "SIDR" },{ 5, 0xff060050, "SDSD" },

};


static constexpr int MaxSystemTables = 256; // FF00xxxx through FFFFxxxx
static constexpr int MaxSystemRegisters = 0x400; // FF0x0000 through FF0x0FFC
void r9751_state::system_trace_init()
{
	const SystemRegisterInfo*** trace_context;

	// Allocate tables for FF00 through FFFF
	trace_context = new const SystemRegisterInfo**[MaxSystemTables];
	memset(trace_context, 0, sizeof(void*)*MaxSystemTables);

	// Iterate over the system register info
	for (int i = 0; i < (sizeof(SystemRegisters) / sizeof(*SystemRegisters)); i++)
	{
		const SystemRegisterInfo* current = SystemRegisters + i;

		int table = (current->RegisterAddress & 0x00FF0000) >> 16;
		int regindex = (current->RegisterAddress & 0xFFFC) >> 2;

		if (table >= MaxSystemTables || regindex >= MaxSystemRegisters)
		{
			logerror("system_trace_init: Register out of range and cannot be traced: 0x%x %s\n", current->RegisterAddress, current->RegisterName);
			continue;
		}

		// Lookup the first tier table and create it if it doesn't exist
		const SystemRegisterInfo** firstTier = trace_context[table];
		if (firstTier == nullptr)
		{
			firstTier = new const SystemRegisterInfo*[MaxSystemRegisters];
			memset(firstTier, 0, sizeof(void*) * MaxSystemRegisters);
			trace_context[table] = firstTier;
		}

		// Set the content in the table
		firstTier[regindex] = current;

	}

	system_trace_context = trace_context;
}

void r9751_state::system_trace_teardown()
{
	for (int i = 0; i < MaxSystemTables; i++)
	{
		delete[] ((const SystemRegisterInfo***)system_trace_context)[i];
	}
	delete[] (const SystemRegisterInfo***)system_trace_context;
}

#if ENABLE_TRACE_SYSTEM
void r9751_state::trace_system(int table, int offset, int data, const char* direction)
{
	const SystemRegisterInfo*** trace_context = (const SystemRegisterInfo***)system_trace_context;

	const char* regName = "?";
	const char* regDescription = "?";
	int level = 10; // Undefined registers are level 10

	if (table < MaxSystemTables && offset < MaxSystemRegisters)
	{
		const SystemRegisterInfo** firstTier = trace_context[table];
		if (firstTier != nullptr)
		{
			const SystemRegisterInfo* current = firstTier[offset];
			if (current != nullptr)
			{
				regName = current->RegisterName;
				if (current->Description != nullptr) regDescription = current->Description;
				level = current->VerboseLevel;
			}
		}
	}

	if (level >= SYSTEM_TRACE_LEVEL)
	{
		u32 address = 0xFF000000 | (table << 16) | (offset << 2);
		logerror("%s System Register [%08X] (%s %s) %s %08X (PC=%08X)\n", machine().time().as_string(), address, regName, regDescription, direction, data, m_maincpu->pc());
	}

}
#else
void r9751_state::trace_system(int table, int offset, int data, const char* direction)
{
}
#endif


uint32_t r9751_state::swap_uint32( uint32_t val )
{
	val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF );
	return (val << 16) | (val >> 16);
}

uint32_t r9751_state::debug_a6()
{
	return m_maincpu->space(AS_PROGRAM).read_dword(ptr_m68000->state_int(M68K_A6) + 4);
}

uint32_t r9751_state::debug_a5()
{
	return m_maincpu->space(AS_PROGRAM).read_dword(ptr_m68000->state_int(M68K_A5));
}

uint32_t r9751_state::debug_a5_20()
{
	return m_maincpu->space(AS_PROGRAM).read_dword(ptr_m68000->state_int(M68K_A5) + 0x20);
}

void r9751_state::UnifiedTrace(u32 address, u32 data, const char* operation, const char* Device, const char* RegisterName, const char* extraText)
{
	u32 stacktrace[4];
	u32 basepointer[2];
	u32 reg_a6 = ptr_m68000->state_int(M68K_A6);

	for(int i=0; i<4; i++)
		stacktrace[i] = 0;
	for(int i=0; i<2; i++)
		basepointer[i] = 0;

	stacktrace[0] = m_maincpu->pc();
	if(reg_a6 + 4 < 0xFFFFFF) stacktrace[1] = m_maincpu->space(AS_PROGRAM).read_dword(reg_a6 + 4);
	if(reg_a6 < 0xFFFFFF && reg_a6 != 0) basepointer[0] = m_maincpu->space(AS_PROGRAM).read_dword(reg_a6);
	if(basepointer[0] + 4 < 0xFFFFFF && basepointer[0] != 0) stacktrace[2] = m_maincpu->space(AS_PROGRAM).read_dword(basepointer[0] + 4);
	if(basepointer[0] < 0xFFFFFF && basepointer[0] != 0) basepointer[1] = m_maincpu->space(AS_PROGRAM).read_dword(basepointer[0]);
	if(basepointer[1] + 4 < 0xFFFFFF && basepointer[1] != 0) stacktrace[3] = m_maincpu->space(AS_PROGRAM).read_dword(basepointer[1] + 4);

	logerror("%s[%08X] => %08X (%s:%s) %s (%08X, %08X, %08X, %08X)\n", operation, address, data, Device, RegisterName==nullptr?"":RegisterName, extraText==nullptr?"":extraText, stacktrace[0], stacktrace[1], stacktrace[2], stacktrace[3]);
}

uint8_t r9751_state::pdc_dma_r(offs_t offset)
{
	/* This callback function takes the value written to 0xFF01000C as the bank offset */
	uint32_t address = (fdd_dma_bank & 0x7FFFF800) + (offset & 0x3FFFF);
	if (TRACE_DMA) logerror("DMA READ: %08X DATA: %08X\n", address, m_maincpu->space(AS_PROGRAM).read_byte(address));
	return m_maincpu->space(AS_PROGRAM).read_byte(address);
}

void r9751_state::pdc_dma_w(uint8_t data)
{
	/* This callback function takes the value written to 0xFF01000C as the bank offset */
	uint32_t address = (fdd_dma_bank & 0x7FFFF800) + (m_pdc->fdd_68k_dma_address & 0x3FFFF);
	m_maincpu->space(AS_PROGRAM).write_byte(address, data);
	if (TRACE_DMA) logerror("DMA WRITE: %08X DATA: %08X\n", address, data);
}

uint8_t r9751_state::smioc_dma_r(offs_t offset)
{
	/* This callback function takes the value written to 0xFF01000C as the bank offset */
	uint32_t address = (smioc_dma_bank & 0x7FFFF800) + (offset*2 & 0x3FFFF);
	u16 word = m_maincpu->space(AS_PROGRAM).read_word(address);
	char c = word & 0xFF;
	if (c < 32 || c > 127) c = ' ';
	if (TRACE_DMA) logerror("%s SMIOC DMA READ: %08X DATA: %04X (%c)\n", machine().time().as_string(), address, word, c);
	return m_maincpu->space(AS_PROGRAM).read_word(address) & 0x00FF;
}

void r9751_state::smioc_dma_w(offs_t offset, uint8_t data)
{
	/* This callback function takes the value written to 0xFF01000C as the bank offset */
	uint32_t address = (smioc_dma_bank & 0x7FFFF800) + (offset*2 & 0x3FFFF);
	m_maincpu->space(AS_PROGRAM).write_word(address, data);
	char c = data & 0xFF;
	if (c < 32 || c > 127) c = ' ';
	if (TRACE_DMA) logerror("%s SMIOC DMA WRITE: %08X DATA: %08X (%c)\n", machine().time().as_string(), address, data, c);
}

void r9751_state::init_r9751()
{
	reg_ff050004 = 0;
	reg_fff80040 = 0;
	fdd_dest_address = 0;
	fdd_cmd_complete = 0;
	fdd_dma_bank = 0;
	smioc_dma_bank = 0;

	m_mem = &m_maincpu->space(AS_PROGRAM);

	m_maincpu->interface<m68000_base_device>(ptr_m68000);

	device_trace_enable_all();
	device_trace_disable(0x07);
	device_trace_disable(0x09);

	/* Save states */
	save_item(NAME(reg_ff050004));
	save_item(NAME(reg_fff80040));
	save_item(NAME(fdd_dest_address));
	save_item(NAME(fdd_cmd_complete));
	save_item(NAME(smioc_dma_bank));
	save_item(NAME(fdd_dma_bank));
	save_item(NAME(timer_32khz_last));

}

void r9751_state::machine_reset()
{
	uint8_t *rom = memregion("prom")->base();
	uint32_t *ram = m_main_ram;

	memcpy(ram, rom, 8);
}

/******************************************************************************
 External board communication registers [0x5FF00000 - 0x5FFFFFFF]
******************************************************************************/
uint32_t r9751_state::r9751_mmio_5ff_r(offs_t offset)
{
	uint32_t data;
	switch(offset << 2)
	{
		/* PDC HDD region (0x24, device 9) */
		case 0x0824: /* HDD Command result code */
			data = 0x10;
			break;

		case 0x3024: /* HDD SCSI command completed successfully */
			data = 0x1;
			if(TRACE_HDC) logerror("SCSI HDD command completion status - Read: %08X, From: %08X, Register: %08X\n", data, m_maincpu->pc(), offset << 2 | 0x5FF00000);
			break;

		/* SMIOC region (0x98, device 0x26) */
		case 0x0898: /* Serial status or DMA status */
			data = m_smioc->GetStatus();
			break;

		case 0x0870: /* Serial status or DMA status 2 (0x70, device 0x24) */
			data = m_smioc->GetStatus2();
			break;
		/* SMIOC region (0x9C, device 0x27) */

		case 0x1098: /* Serial word count register */
			data = m_smioc->m_wordcount;
			m_smioc->ClearParameter();
			break;

		case 0x1070: /* Serial word count register (alternate) */
			data = m_smioc->m_wordcount2;
			m_smioc->ClearParameter2();
			break;

		case 0x2898: /* SMIOC DMA Busy register - nonzero = busy */
			data = m_smioc->m_deviceBusy;
			break;

		/* PDC FDD region (0xB0, device 44 */
		case 0x08B0: /* FDD Command result code */
			data = 0x10;
			break;

		case 0x10B0: /* Clear 5FF030B0 ?? */
			if(TRACE_FDC) logerror("--- FDD 0x5FF010B0 READ (0)\n");
			data = 0;
			break;

		case 0x1890: /* Device offset 0x90 (Bit 7 needs to be set) */
			data = 0x80;
			break;

		case 0x30B0: /* FDD command completion status */
			data = (m_pdc->reg_p5 << 8) + m_pdc->reg_p4;
			if(TRACE_FDC && data != 0) logerror("--- SCSI FDD command completion status - Read: %08X, From: %08X, Register: %08X\n", data, m_maincpu->pc(), offset << 2 | 0x5FF00000);
			break;

		default:
			data = 0;
			break;
	}
	trace_device(offset << 2 | 0x5FF00000, data, ">>");
	return data;
}

void r9751_state::r9751_mmio_5ff_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint8_t data_b0, data_b1;

	trace_device(offset << 2 | 0x5FF00000, data, "<<");

	/* Unknown mask */
	if (mem_mask != 0xFFFFFFFF)
		logerror("Mask found: %08X Register: %08X PC: %08X\n", mem_mask, offset << 2 | 0x5FF00000, m_maincpu->pc());

	switch(offset << 2)
	{
		/* PDC HDD region (0x24, device 9) */

		/* SMIOC region (0x98, device 26) - Output */
		case 0x0298:
			m_smioc->ClearStatus();
			break;
		case 0x0270:
			m_smioc->ClearStatus2();
			break;
		case 0x4070: // SMIOC upper port command (ports 4-7)
			m_smioc->SendCommand2(data);
			break;
		case 0x8070: // SMIOC upper port parameter (ports 4-7)
			m_smioc->SetCommandParameter2(data);
			break;

		case 0x0198: // SMIOC soft reset?
			// It's not clear what exactly this register write does.
			//   It isn't a soft reset of the SMIOC because the 68k does not wait long enough for the SMIOC to finish rebooting.
			//   Also the 68k does this twice in succession, with a status clear in between.
			// The theory now is that a write to this register causes the status registers on the SMIOC to be
			//   set to the magic value 0x0100 (which is set after initialization is complete) - which serves as a trigger
			//   for the disktool software to reinitialize the SMIOC and proceed into a working state.
			// This probably isn't correct, but hopefully we will determine the correct approach in the future.

			m_smioc->m_status = 0x0140;
			m_smioc->m_status2 = 0x0140;
			break;

		case 0x4098: /* Serial DMA Command */
			m_smioc->SendCommand(data);
			break;
		case 0x8098: /* Command Parameter */
			m_smioc->SetCommandParameter(data);
			break;
		case 0xC098: /* Serial DMA Transmit data address */
			m_smioc->SetDmaParameter(smiocdma_sendaddress, data);
			break;

		/* SMIOC region (0x9C, device 27) - Input */
		case 0x409C: /* Serial DMA write length */
			m_smioc->SetDmaParameter(smiocdma_sendlength, data);
			break;
		case 0x809C: /* Serial DMA Receive data address */
			m_smioc->SetDmaParameter(smiocdma_recvaddress, data);
			break;
		case 0xC09C: /* Serial DMA read length */
			m_smioc->SetDmaParameter(smiocdma_recvlength, data);
			break;

		/* PDC FDD region (0xB0, device 44) */
		case 0x04B0: /* FDD RESET PDC */
			if(TRACE_FDC) logerror("PDC RESET, PC: %08X DATA: %08X\n", m_maincpu->pc(), data);
			m_pdc->reset();
			break;
		case 0x41B0: /* Unknown - Probably old style commands */
			if(TRACE_FDC) logerror("--- FDD Command: %08X, From: %08X, Register: %08X\n", data, m_maincpu->pc(), offset << 2 | 0x5FF00000);

			/* Clear FDD Command completion status 0x5FF030B0 (PDC 0x4, 0x5) */
			m_pdc->reg_p4 = 0;
			m_pdc->reg_p5 = 0;

			data_b0 = data & 0xFF;
			data_b1 = (data & 0xFF00) >> 8;
			m_pdc->reg_p0 = data_b0;
			m_pdc->reg_p1 = data_b1;
			m_pdc->reg_p38 |= 0x2; /* Set bit 1 on port 38 register, PDC polls this port looking for a command */
			if(TRACE_FDC) logerror("--- FDD Old Command: %02X and %02X\n", data_b0, data_b1);
			break;
		case 0xC0B0:
		case 0xC1B0: /* fdd_dest_address register */
			fdd_dest_address = data << 1;
			if(TRACE_FDC) logerror("--- FDD destination address: %08X PC: %08X Register: %08X (A6+4): %08X\n", (fdd_dma_bank & 0x7FFFF800) + (fdd_dest_address&0x3FFFF), m_maincpu->pc(), offset << 2 | 0x5FF00000, debug_a6());
			data_b0 = data & 0xFF;
			data_b1 = (data & 0xFF00) >> 8;
			m_pdc->reg_p6 = data_b0;
			m_pdc->reg_p7 = data_b1;
			m_pdc->reg_p38 |= 0x2; // Set bit 1 on port 38 register, PDC polls this port looking for a command
			if(TRACE_FDC)logerror("--- FDD SET PDC Port 38: %X\n",m_pdc->reg_p38);
			break;
		case 0x80B0: /* FDD command address register */
			uint32_t fdd_scsi_command;
			uint32_t fdd_scsi_command2;
			unsigned char c_fdd_scsi_command[8]; // Array for SCSI command
			int scsi_lba; // FDD LBA location here, extracted from command

			/* Clear FDD Command completion status 0x5FF030B0 (PDC 0x4, 0x5) */
			m_pdc->reg_p4 = 0;
			m_pdc->reg_p5 = 0;

			/* Send FDD SCSI command location address to PDC 0x2, 0x3 */
			if(TRACE_FDC) logerror("--- FDD command address: %08X PC: %08X Register: %08X (A6+4): %08X A4: %08X (A5): %08X (A5+20): %08X\n", (fdd_dma_bank & 0x7FFFF800) + ((data << 1)&0x3FFFF), m_maincpu->pc(), offset << 2 | 0x5FF00000, debug_a6(), ptr_m68000->state_int(M68K_A4), debug_a5(), debug_a5_20());
			data_b0 = data & 0xFF;
			data_b1 = (data & 0xFF00) >> 8;
			m_pdc->reg_p2 = data_b0;
			m_pdc->reg_p3 = data_b1;

			fdd_scsi_command = swap_uint32(m_mem->read_dword((fdd_dma_bank & 0x7FFFF800) + ((data << 1)&0x3FFFF)));
			fdd_scsi_command2 = swap_uint32(m_mem->read_dword(((fdd_dma_bank & 0x7FFFF800) + ((data << 1)&0x3FFFF))+4));

			memcpy(c_fdd_scsi_command,&fdd_scsi_command,4);
			memcpy(c_fdd_scsi_command+4,&fdd_scsi_command2,4);

			if(TRACE_FDC)
			{
				logerror("--- FDD SCSI Command: ");
				for(int i = 0; i < 8; i++)
					logerror("%02X ", c_fdd_scsi_command[i]);
				logerror("\n");
			}

			scsi_lba = c_fdd_scsi_command[3] | (c_fdd_scsi_command[2]<<8) | ((c_fdd_scsi_command[1]&0x1F)<<16);
			if(TRACE_FDC) logerror("--- FDD SCSI LBA: %i\n", scsi_lba);

			break;

		default:
			break;
	}
}

/******************************************************************************
 CPU board registers [0xFF010000 - 0xFF06FFFF]
******************************************************************************/
uint32_t r9751_state::r9751_mmio_ff01_r(offs_t offset)
{
	u32 data;
	switch(offset << 2)
	{
		default:
			data = 0;
	}
	trace_system(1, offset, data, ">>");
	return data;
}

void  r9751_state::r9751_mmio_ff01_w (offs_t offset, uint32_t data, uint32_t mem_mask)
{
	/* Unknown mask */
	if (mem_mask != 0xFFFFFFFF)
		logerror("Mask found: %08X Register: %08X PC: %08X\n", mem_mask, offset << 2 | 0xFF010000, m_maincpu->pc());

	trace_system(1, offset, data, "<<");

	switch(offset << 2)
	{
		case 0x000C: /* FDD DMA Offset */
			fdd_dma_bank = data;
			if(TRACE_DMA) logerror("Banking register(FDD): %08X PC: %08X Data: %08X\n", offset << 2 | 0xFF010000, m_maincpu->pc(), data);
			return;
		case 0x0010: /* SMIOC DMA Offset */
			smioc_dma_bank = data;
			if(TRACE_DMA) logerror("Banking register(SMIOC): %08X PC: %08X Data: %08X\n", offset << 2 | 0xFF010000, m_maincpu->pc(), data);
			return;
		default:
			if(TRACE_DMA) logerror("Banking register(Unknown): %08X PC: %08X Data: %08X\n", offset << 2 | 0xFF010000, m_maincpu->pc(), data);
			return;
	}
}

uint32_t r9751_state::r9751_mmio_ff05_r(offs_t offset)
{
	uint32_t data;

	switch(offset << 2)
	{
		case 0x0004:
			data = reg_ff050004;
			break;
		case 0x0300:
			data = 0x1B | (1<<0x14);
			break;
		case 0x0320: /* Some type of counter */
			data = (machine().time() - timer_32khz_last).as_ticks(32768) & 0xFFFF;
			break;
		case 0x0584:
			data = 0;
			break;
		case 0x0610:
			data = 0xabacabac;
			break;
		case 0x0014:
			data = 0x80;
			break;
		default:
			data = 0;
			break;
	}
	trace_system(5, offset, data, "<<");
	return data;
}

void r9751_state::r9751_mmio_ff05_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	/* Unknown mask */
	if (mem_mask != 0xFFFFFFFF)
		logerror("Mask found: %08X Register: %08X PC: %08X\n", mem_mask, offset << 2 | 0xFF050000, m_maincpu->pc());

	trace_system(5, offset, data, ">>");

	switch(offset << 2)
	{
		case 0x0004:
			reg_ff050004 = data;
			return;
		case 0x000C: /* CPU LED hex display indicator */
			if(TRACE_LED) logerror("\n*** LED: %02x, Instruction: %08x ***\n\n", data, m_maincpu->pc());
			return;
		case 0x0320:
			timer_32khz_last = machine().time();
			return;
		default:
			return;
	}
}

uint32_t r9751_state::r9751_mmio_fff8_r(offs_t offset)
{
	uint32_t data;

	switch(offset << 2)
	{
		case 0x0040:
			data = reg_fff80040;
			break;
		default:
			data = 0;
			break;
	}
	trace_system(5, offset, data, "<<");
	return data;
}

void r9751_state::r9751_mmio_fff8_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	/* Unknown mask */
	if (mem_mask != 0xFFFFFFFF)
		logerror("Mask found: %08X Register: %08X PC: %08X\n", mem_mask, offset << 2 | 0xFFF80000, m_maincpu->pc());

	trace_system(5, offset, data, ">>");

	switch(offset << 2)
	{
		case 0x0040:
			reg_fff80040 = data;
			return;
		default:
			return;
	}
}

/******************************************************************************
 Address Maps
******************************************************************************/

void r9751_state::r9751_mem(address_map &map)
{
	//map.unmap_value_high();
	map(0x00000000, 0x00ffffff).ram().share("main_ram"); // 16MB
	map(0x08000000, 0x0800ffff).rom().region("prom", 0);
	map(0x5FF00000, 0x5FFFFFFF).rw(FUNC(r9751_state::r9751_mmio_5ff_r), FUNC(r9751_state::r9751_mmio_5ff_w));
	map(0xFF010000, 0xFF01FFFF).rw(FUNC(r9751_state::r9751_mmio_ff01_r), FUNC(r9751_state::r9751_mmio_ff01_w));
	map(0xFF050000, 0xFF06FFFF).rw(FUNC(r9751_state::r9751_mmio_ff05_r), FUNC(r9751_state::r9751_mmio_ff05_w));
	map(0xFFF80000, 0xFFF8FFFF).rw(FUNC(r9751_state::r9751_mmio_fff8_r), FUNC(r9751_state::r9751_mmio_fff8_w));
	//map(0xffffff00,0xffffffff).ram(); // Unknown area
}

/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( r9751 )
INPUT_PORTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/

void r9751_state::scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM);
	device.option_add("harddisk", NSCSI_HARDDISK);
}

void r9751_state::wd33c93(device_t *device)
{
	device->set_clock(10000000);
	//  downcast<wd33c93a_device *>(device)->irq_cb().set(*this, FUNC(r9751_state::scsi_irq_w));
	//  downcast<wd33c93a_device *>(device)->drq_cb().set(*this, FUNC(r9751_state::scsi_drq_w));
}

void r9751_state::r9751(machine_config &config)
{
	/* basic machine hardware */
	M68030(config, m_maincpu, 20000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &r9751_state::r9751_mem);
	config.set_maximum_quantum(attotime::from_hz(1000));

	/* i/o hardware */
	SMIOC(config, m_smioc, 0);
	m_smioc->m68k_r_callback().set(FUNC(r9751_state::smioc_dma_r));
	m_smioc->m68k_w_callback().set(FUNC(r9751_state::smioc_dma_w));

	/* disk hardware */
	PDC(config, m_pdc, 0);
	m_pdc->m68k_r_callback().set(FUNC(r9751_state::pdc_dma_r));
	m_pdc->m68k_w_callback().set(FUNC(r9751_state::pdc_dma_w));

	NSCSI_BUS(config, "scsi", 0);
	NSCSI_CONNECTOR(config, "scsi:0", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, "cdrom", false);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("wd33c93", WD33C93)
		.machine_config([this](device_t *device) { wd33c93(device); });

	/* software list */
	SOFTWARE_LIST(config, "flop_list").set_original("r9751");
}



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(r9751)
	ROM_REGION32_BE(0x00010000, "prom", 0)
	ROM_SYSTEM_BIOS(0, "prom34",  "PROM Version 3.4")
	ROMX_LOAD( "p-n_98d4643__abaco_v3.4__=49fe7a=__j221.27512.bin", 0x0000, 0x10000, CRC(9fb19a85) SHA1(c861e15a2fc9a4ef689c2034c53fbb36f17f7da6), ROM_GROUPWORD | ROM_BIOS(0) ) // Label: "P/N 98D4643 // ABACO V3.4 // (49FE7A) // J221" 27512 @Unknown

	ROM_SYSTEM_BIOS(1, "prom42", "PROM Version 4.2")
	ROMX_LOAD( "98d5731__zebra_v4.2__4cd79d.u5", 0x0000, 0x10000, CRC(e640f8df) SHA1(a9e4fa271d7f2f3a134e2120932ec088d5b8b007), ROM_GROUPWORD | ROM_BIOS(1) ) // Label: 98D5731 // ZEBRA V4.2 // 4CD79D 27512 @Unknown
ROM_END

} // anonymous namespace


/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY               FULLNAME              FLAGS
COMP( 1988, r9751, 0,      0,      r9751,   r9751, r9751_state, init_r9751, "ROLM Systems, Inc.", "ROLM 9751 Model 10", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
