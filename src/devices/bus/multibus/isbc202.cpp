// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    isbc202.cpp

    Intel iSBC-202 SSDD 8" floppy disk controller

    This controller interfaces a standard 8/16-bit Multibus system
    with up to 4 8" SSDD floppy drives. It was typically used to
    expand a MDS-II system with double-density drives.
    The ISIS-II OS identifies the drives as :F0: :F1: :F2: :F3:
    The iSBC-202 controller is based on two boards: a "channel"
    board with the actual controller and an "interface" board.
    The latter handles the low-level aspects of the disks, especially
    the encoding and decoding of MMFM modulated bits. This board
    interfaces directly with standard Shugart SA-800-1 drives.
    The drives are housed, two at time, in a (big) external box
    that also holds their power supply.
    The channel board is implemented with a 3000-series bit slice
    processor. Microcode is stored on 4 512x8 bipolar PROMs.
    The channel board is bus mastering, i.e. it can issue memory
    read/write cycles to the main processor RAM.
    Format of data on disk is entirely Intel proprietary.
    Intel also designed a similar controller (iSBC-201) for FM
    IBM-standard disks. AFAIK, this board shares the channel board
    with iSBC-202 (but not the microcode) and has a different
    interface board.
    This table summarizes the main characteristics of the disks.

    | Bit cell size  | 2 µs     |
    | Modulation     | MMFM     |
    | Bit order      | MS first |
    | Sides          | 1        |
    | Tracks         | 77       |
    | Sectors/track  | 52       |
    | Sector size    | 128 bytes|
    | Formatted size | 500.5 kB |
    | Rotation speed | 360 RPM  |

    Special thanks to Eric Smith for dumping the microcode PROMs.
    Without his work this driver wouldn't exist at all.

    Reference manuals.
    - Intellec series II MDS double-density diskette subsystem
      (schematic drawings) - Intel 1980 - 9800425-02 Rev. B
    - Intellec double-density diskette operating system hardware
      reference manual - Intel 1977 - 9800422A
    - SBC 202 double-density diskette controller hardware reference
      manual - Intel 1977 - 9800420A

    What follows is a list of things that I left out. They could be
    implemented at a later time just for completeness sake as ISIS-II
    doesn't rely on them at all.
    - The STOP signal (it seems to be used in iSBC-201 only)
    - Interrupt to CPU
    - Head load/unload commands (MAME doesn't emulate head loading)

*********************************************************************/

#include "emu.h"
#include "isbc202.h"
#include "formats/img_dsk.h"

// Debugging
#include "logmacro.h"
#define LOG_BUS_MASK (LOG_GENERAL << 1)
#define LOG_BUS(...) LOGMASKED(LOG_BUS_MASK, __VA_ARGS__)
#define LOG_RD_MASK (LOG_BUS_MASK << 1)
#define LOG_RD(...) LOGMASKED(LOG_RD_MASK, __VA_ARGS__)
#define LOG_WR_MASK (LOG_RD_MASK << 1)
#define LOG_WR(...) LOGMASKED(LOG_WR_MASK, __VA_ARGS__)
#define LOG_DR_MASK (LOG_WR_MASK << 1)
#define LOG_DR(...) LOGMASKED(LOG_DR_MASK, __VA_ARGS__)
#undef VERBOSE
//#define VERBOSE (LOG_GENERAL|LOG_BUS_MASK|LOG_RD_MASK|LOG_WR_MASK|LOG_DR_MASK)
#define VERBOSE LOG_GENERAL

// Bit manipulation
namespace {
	template<typename T> constexpr T BIT_MASK(unsigned n)
	{
		return (T)1U << n;
	}

	template<typename T> void BIT_CLR(T& w , unsigned n)
	{
		w &= ~BIT_MASK<T>(n);
	}

	template<typename T> void BIT_SET(T& w , unsigned n)
	{
		w |= BIT_MASK<T>(n);
	}
}

// Constants
constexpr unsigned TIMEOUT_MS = 10;     // "timeout" timer: 10 ms
constexpr unsigned HALF_BIT_CELL_US = 1;// Half bit cell duration in µs
constexpr unsigned BIT_FREQUENCY = 500000;  // Frequency of bit cells in Hz
constexpr uint16_t CRC_POLY = 0x1021;   // CRC-CCITT

// Timers
enum {
	  TIMEOUT_TMR_ID,
	  BYTE_TMR_ID,
	  F_TMR_ID
};

// device type definition
DEFINE_DEVICE_TYPE(ISBC202, isbc202_device, "isbc202", "iSBC-202 floppy controller")

// Microcode disassembler
class isbc202_disassembler : public util::disasm_interface
{
public:
	isbc202_disassembler();
	virtual ~isbc202_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

isbc202_disassembler::isbc202_disassembler()
{
}

u32 isbc202_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t isbc202_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint32_t microcode = opcodes.r32(pc);

	// Decode address control instruction
	uint8_t ac = uint8_t(microcode >> 25);

	if ((ac & 0b1100000) == 0b0000000) {
		// JCC
		util::stream_format(stream , "JCC $%03x" , ((uint16_t(ac) & 0b11111) << 4) | (pc & 0xf));
	} else if ((ac & 0b1110000) == 0b0100000) {
		// JZR
		util::stream_format(stream , "JZR $00%x" , ac & 0b1111);
	} else if ((ac & 0b1110000) == 0b0110000) {
		// JCR
		util::stream_format(stream , "JCR $%03x" , (pc & 0b111110000) | (ac & 0b1111));
	} else if ((ac & 0b1111000) == 0b1110000) {
		// JCE
		util::stream_format(stream , "JCE $%03x" , (pc & 0b110001111) | ((ac & 0b111) << 4));
	} else if ((ac & 0b1110000) == 0b1000000) {
		// JFL
		util::stream_format(stream , "JFL $%03x" , (pc & 0b100001000) | ((ac & 0b1111) << 4) | 0b10);
	} else if ((ac & 0b1111000) == 0b1010000) {
		// JCF
		util::stream_format(stream , "JCF $%03x" , (pc & 0b110001000) | ((ac & 0b111) << 4) | 0b10);
	} else if ((ac & 0b1111000) == 0b1011000) {
		// JZF
		util::stream_format(stream , "JZF $%03x" , (pc & 0b110001000) | ((ac & 0b111) << 4) | 0b10);
	} else if ((ac & 0b1111000) == 0b1100000) {
		// JPR
		util::stream_format(stream , "JPR $%02xx" , ((pc >> 4) & 0b11000) | (ac & 0b111));
	} else if ((ac & 0b1111000) == 0b1101000) {
		// JLL
		util::stream_format(stream , "JLL $%03x" , (pc & 0b110000000) | ((ac & 0b111) << 4) | 0b100);
	} else if ((ac & 0b1111100) == 0b1111100) {
		// JRL
		util::stream_format(stream , "JRL $%03x" , (pc & 0b110000000) | ((ac & 0b11) << 4) | 0b001001100);
	} else {
		// JPX
		util::stream_format(stream , "JPX $%02xx" , ((pc >> 4) & 0b11100) | (ac & 0b11));
	}

	// Decode input multiplexer
	uint8_t in = uint8_t((microcode >> 13) & 7);
	util::stream_format(stream , " I=%u" , in);

	// Decode function code
	uint8_t fc = uint8_t((microcode >> 18) & 0x7f);
	uint8_t fg;
	uint8_t rg;
	unsigned reg;
	i3002_device::decode_fc(fc , fg , rg , reg);
	util::stream_format(stream , " CPE=%u%u/%2s" , fg , rg + 1 , i3002_device::reg_name(reg));

	// Decode flag control
	stream << (BIT(microcode , 17) ? " FF1" : " FF0");
	stream << (BIT(microcode , 16) ? " HCZ" : " SCZ");

	// Decode K
	uint8_t slk = uint8_t((microcode >> 8) & 3);
	uint8_t mask = uint8_t(microcode);
	uint8_t kbus;
	if (!BIT(slk , 0)) {
		kbus = mask;
	} else if (slk == 1) {
		kbus = 0xff;
	} else {
		kbus = 0;
	}
	util::stream_format(stream , " K=$%02x" , kbus);

	// Decode OUT
	util::stream_format(stream , " S=%u M=$%02x" , slk , mask);
	if (BIT(slk , 0) && !BIT(mask , 7)) {
		uint8_t out = uint8_t((microcode >> 10) & 7);
		util::stream_format(stream , " O=%u" , out);
	}

	return 1 | SUPPORTED;
}

// isbc202_device
isbc202_device::isbc202_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig , ISBC202 , tag , owner , DERIVED_CLOCK(1, 4))
	, device_multibus_interface(mconfig , *this)
	, m_mcu(*this , "mcu")
	, m_cpes(*this , "cpe%u" , 0)
	, m_drives(*this , "floppy%u" , 0)
	, m_program_config("microprogram" , ENDIANNESS_BIG , 32 , 9 , -2)
	, m_mem_space(nullptr)
{
}

isbc202_device::~isbc202_device()
{
}

uint8_t isbc202_device::io_r(address_space &space, offs_t offset)
{
	uint8_t res = 0;

	switch (offset) {
	case 0:
		// Read drive status & INT (auto XACK)
		// Bit      What
		// 7        0
		// 6        Drive 3 ready
		// 5        Drive 2 ready
		// 4        1
		// 3        1
		// 2        Int pending
		// 1        Drive 1 ready
		// 0        Drive 0 ready
		{
			uint8_t ready = m_ready_in & m_ready_ff;
			if (BIT(ready , 3)) {
				BIT_SET(res , 6);
			}
			if (BIT(ready , 2)) {
				BIT_SET(res , 5);
			}
			BIT_SET(res , 4);
			BIT_SET(res , 3);
			if (m_irq) {
				BIT_SET(res , 2);
			}
			if (BIT(ready , 1)) {
				BIT_SET(res , 1);
			}
			if (BIT(ready , 0)) {
				BIT_SET(res , 0);
			}
		}
		break;

	case 1:
		// Read result type (auto XACK)
		m_irq = false;
		res = m_data_low_out;
		break;

	case 3:
		// Read result byte (no auto XACK)
		if (m_cpu == nullptr) {
			m_cpu = dynamic_cast<cpu_device*>(&space.device());
			set_start(3 , true);
		} else {
			m_cpu = nullptr;
			res = m_data_low_out;
		}
		break;

	default:
		LOG("RD from unknown reg!\n");
		break;
	}

	LOG_BUS("IO R @%u=%02x\n" , offset , res);
	return res;
}

void isbc202_device::io_w(address_space &space, offs_t offset, uint8_t data)
{
	LOG_BUS("IO W @%u=%02x\n" , offset , data);

	switch (offset) {
	case 0:
	case 1:
		// Write LSB address
	case 2:
		// Write MSB address & start op
	case 4:
	case 5:
	case 6:
		if (m_cpu != nullptr) {
			LOG("CPU != NULL!\n");
		}
		m_cpu = dynamic_cast<cpu_device*>(&space.device());
		m_cpu_data = data;
		set_start(offset , false);
		break;

	case 7:
		// Reset
		pulse_input_line(INPUT_LINE_RESET , attotime::zero);
		break;

	default:
		LOG("WR to unknown reg!\n");
		break;
	}
}

WRITE_LINE_MEMBER(isbc202_device::co_w)
{
	m_inputs[ IN_SEL_CO ] = state;
	m_mcu->fi_w(state);
	m_cpes[ 3 ]->li_w(state);
}

uint8_t isbc202_device::px_r()
{
	if (BIT(m_px_s1s0 , 0)) {
		return m_cmd & 7;
	} else if (BIT(m_px_s1s0 , 1)) {
		return (m_op_us & 7) | 8;
	} else {
		return 0;
	}
}

void isbc202_device::device_start()
{
	state_add(STATE_GENPC     , "GENPC" , m_microcode_addr).noshow();
	state_add(STATE_GENPCBASE , "CURPC" , m_microcode_addr).noshow();
	state_add(STATE_GENFLAGS  , "FLAGS" , m_flags).noshow().callimport().callexport().formatstr("%9s");

	for (int i = 0; i < i3002_device::REG_COUNT; ++i) {
		state_add(i , i3002_device::reg_name(i) , m_regs[ i ]).callimport().callexport();
	}

	save_item(NAME(m_flags));
	save_item(NAME(m_regs));
	save_item(NAME(m_microcode_addr));
	save_item(NAME(m_code_word));
	save_item(NAME(m_ac));
	save_item(NAME(m_fc));
	save_item(NAME(m_fc32));
	save_item(NAME(m_fc10));
	save_item(NAME(m_in_sel));
	save_item(NAME(m_out_sel));
	save_item(NAME(m_slk));
	save_item(NAME(m_mask));
	save_item(NAME(m_kbus));
	save_item(NAME(m_inputs));
	save_item(NAME(m_op_us));
	save_item(NAME(m_px_s1s0));
	save_item(NAME(m_cmd));
	save_item(NAME(m_cpu_rd));
	save_item(NAME(m_ready_in));
	save_item(NAME(m_ready_ff));
	save_item(NAME(m_gate_lower));
	save_item(NAME(m_irq));
	save_item(NAME(m_data_low_out));
	save_item(NAME(m_data_low_in));
	save_item(NAME(m_cpu_data));
	save_item(NAME(m_addr_low_out));
	save_item(NAME(m_mem_wrt));
	save_item(NAME(m_wrt_inh));
	save_item(NAME(m_direction));
	save_item(NAME(m_ibus_cached));
	save_item(NAME(m_ibus));
	save_item(NAME(m_crc));
	save_item(NAME(m_crc_enabled));
	save_item(NAME(m_crc_out));
	save_item(NAME(m_reading));
	save_item(NAME(m_writing));
	save_item(NAME(m_data_sr));
	save_item(NAME(m_last_data_bit));
	save_item(NAME(m_clock_sr));
	save_item(NAME(m_last_f_time));
	save_item(NAME(m_clock_gate));
	save_item(NAME(m_amwrt));
	save_item(NAME(m_dlyd_amwrt));

	space(AS_PROGRAM).cache(m_cache);
	set_icountptr(m_icount);
	space(AS_PROGRAM).install_rom(0 , 0x1ff , memregion("microcode")->base());

	for (auto& d : m_drives) {
		d->get_device()->setup_ready_cb(floppy_image_device::ready_cb(&isbc202_device::floppy_ready_cb , this));
		d->get_device()->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&isbc202_device::floppy_index_cb , this));
	}

	m_timeout_timer = timer_alloc(TIMEOUT_TMR_ID);
	m_byte_timer = timer_alloc(BYTE_TMR_ID);
	m_f_timer = timer_alloc(F_TMR_ID);

	m_mem_space = &m_bus->space(AS_PROGRAM);
	m_bus->space(AS_IO).install_readwrite_handler(0x78, 0x7f, read8m_delegate(*this, FUNC(isbc202_device::io_r)), write8m_delegate(*this, FUNC(isbc202_device::io_w)));
}

void isbc202_device::device_reset()
{
	// Set start address
	m_mcu->addr_w(0);
	// Select drive #0
	m_op_us = 0;
	m_current_drive = m_drives[ 0 ]->get_device();

	// XFERREQ is always 1 because R/W in CPU memory is instantaneous
	m_inputs[ IN_SEL_XFERQ ] = true;

	m_inputs[ IN_SEL_TIMEOUT ] = true;
	m_inputs[ IN_SEL_F ] = false;

	m_cpu = nullptr;

	m_irq = false;

	m_reading = false;
	m_writing = false;

	m_timeout_timer->reset();
	m_byte_timer->reset();
	m_f_timer->reset();
}

void isbc202_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id) {
	case TIMEOUT_TMR_ID:
		m_inputs[ IN_SEL_TIMEOUT ] = true;
		break;

	case BYTE_TMR_ID:
		m_inputs[ IN_SEL_F ] = true;
		m_f_timer->adjust(attotime::from_usec(HALF_BIT_CELL_US * 2));
		m_dlyd_amwrt = m_amwrt;
		if (m_reading) {
			m_last_f_time = machine().time();
			rd_bits(8);
			m_byte_timer->adjust(m_pll.ctime - machine().time());
			// Updating of AZ flag actually happens when F goes low
			m_inputs[ IN_SEL_AZ ] = m_crc == 0;
		}
		break;

	case F_TMR_ID:
		m_inputs[ IN_SEL_F ] = false;
		if (m_writing) {
			write_byte();
			m_data_sr = dbus_r();
			m_byte_timer->adjust(attotime::from_usec(HALF_BIT_CELL_US * 14));
		}
		break;

	default:
		break;
	}
}

ROM_START(isbc202)
	ROM_REGION(0x800 , "microcode" , ROMREGION_32BIT | ROMREGION_BE)
	ROM_LOAD32_BYTE("sbc202-a10-0230.bin" , 0x000 , 0x200 , CRC(e8fa3893) SHA1(88fab74b0466e8aac36eee46cd7536ed1b32a2c9))
	ROM_LOAD32_BYTE("sbc202-a11-0261.bin" , 0x001 , 0x200 , CRC(3ad01769) SHA1(4c22b8fc3ea599dd49684ff4dcafc29ec3425c4c))
	ROM_LOAD32_BYTE("sbc202-a12-0233.bin" , 0x002 , 0x200 , CRC(61496232) SHA1(b0473217944b2f6e966d97e97cf5ad8d883a09e4))
	ROM_LOAD32_BYTE("sbc202-a13-0232.bin" , 0x003 , 0x200 , CRC(c369ab86) SHA1(fc3b7f9c3e71ea1442827c51247a9944c6d40b37))
ROM_END

const tiny_rom_entry *isbc202_device::device_rom_region() const
{
	return ROM_NAME(isbc202);
}

static void isbc202_floppies(device_slot_interface &device)
{
	device.option_add("8ssdd" , FLOPPY_8_SSDD);
}

static void isbc202_floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_IMG_FORMAT);
};

void isbc202_device::device_add_mconfig(machine_config &config)
{
	I3001(config , m_mcu , 0);

	// Allocation of the bit-slices:
	// m_cpes[ 0 ]  Bits 0..1
	// m_cpes[ 1 ]  Bits 2..3
	// m_cpes[ 2 ]  Bits 4..5
	// m_cpes[ 3 ]  Bits 6..7
	for (auto& finder : m_cpes) {
		I3002(config , finder , 0);
	}

	// Connect CO/CI signals
	m_mcu->fo_w().set(m_cpes[ 0 ] , FUNC(i3002_device::ci_w));
	m_cpes[ 0 ]->co_w().set(m_cpes[ 1 ] , FUNC(i3002_device::ci_w));
	m_cpes[ 1 ]->co_w().set(m_cpes[ 2 ] , FUNC(i3002_device::ci_w));
	m_cpes[ 2 ]->co_w().set(m_cpes[ 3 ] , FUNC(i3002_device::ci_w));
	m_cpes[ 3 ]->co_w().set(FUNC(isbc202_device::co_w));

	// Connect RO/LI signals
	m_cpes[ 0 ]->ro_w().set(FUNC(isbc202_device::co_w));
	m_cpes[ 1 ]->ro_w().set(m_cpes[ 0 ] , FUNC(i3002_device::li_w));
	m_cpes[ 2 ]->ro_w().set(m_cpes[ 1 ] , FUNC(i3002_device::li_w));
	m_cpes[ 3 ]->ro_w().set(m_cpes[ 2 ] , FUNC(i3002_device::li_w));

	// Connect M-bus
	m_cpes[ 0 ]->mbus_r().set([this]() { return mbus_r(); });
	m_cpes[ 1 ]->mbus_r().set([this]() { return mbus_r() >> 2; });
	m_cpes[ 2 ]->mbus_r().set([this]() { return mbus_r() >> 4; });
	m_cpes[ 3 ]->mbus_r().set([this]() { return mbus_r() >> 6; });

	// Connect I-bus
	m_cpes[ 0 ]->ibus_r().set([this]() { return ibus_r(); });
	m_cpes[ 1 ]->ibus_r().set([this]() { return ibus_r() >> 2; });
	m_cpes[ 2 ]->ibus_r().set([this]() { return ibus_r() >> 4; });
	m_cpes[ 3 ]->ibus_r().set([this]() { return ibus_r() >> 6; });

	// Connect SX input
	m_mcu->sx_r().set([this]() { return m_microcode_addr & 0xf; });
	// Connect PX input
	m_mcu->px_r().set(FUNC(isbc202_device::px_r));

	// Drives
	for (auto& finder : m_drives) {
		FLOPPY_CONNECTOR(config , finder , isbc202_floppies , "8ssdd" , isbc202_floppy_formats).set_fixed(true);
	}
}

void isbc202_device::execute_run()
{
	do {
		m_microcode_addr = m_mcu->addr_r();
		debugger_instruction_hook(m_microcode_addr);
		m_code_word = m_cache.read_dword(m_microcode_addr);

		// Unpack microcode into fields
		// Bits     Field
		//================
		// 31..25   Address Control
		// 24..18   Function Code
		// 17       Flag Control bits 3 & 2
		// 16       Flag Control bits 1 & 0
		// 15..13   Input multiplexer selection
		// 12..10   Output control
		//  9..8    SLK field
		//  7..0    Mask field
		m_ac = uint8_t(m_code_word >> 25);
		m_fc = uint8_t((m_code_word >> 18) & 0x7f);
		m_fc32 = BIT(m_code_word , 17);
		m_fc10 = BIT(m_code_word , 16);
		m_in_sel = uint8_t((m_code_word >> 13) & 7);
		m_out_sel = uint8_t((m_code_word >> 10) & 7);
		m_slk = uint8_t((m_code_word >> 8) & 3);
		m_mask = uint8_t(m_code_word);

		m_mcu->fc_w((m_fc32 ? 0b1100 : 0b0000) | (m_fc10 ? 0b0011 : 0b0000));

		// Set outputs
		if (BIT(m_slk , 0) && !BIT(m_mask , 7)) {
			set_output();
		}

		// Compute K-bus
		if (!BIT(m_slk , 0)) {
			m_kbus = m_mask;
		} else if (m_slk == 1) {
			m_kbus = 0xff;
		} else {
			m_kbus = 0;
		}

		m_ibus_cached = false;

		// Update CPEs
		m_cpes[ 0 ]->fc_kbus_w(m_fc , m_kbus);
		m_cpes[ 1 ]->fc_kbus_w(m_fc , m_kbus >> 2);
		m_cpes[ 2 ]->fc_kbus_w(m_fc , m_kbus >> 4);
		m_cpes[ 3 ]->fc_kbus_w(m_fc , m_kbus >> 6);
		if (m_cpes[ 0 ]->update_ro()) {
			// Data propagate to right (right-shift op)
			m_cpes[ 3 ]->clk_w(1);
			m_cpes[ 2 ]->clk_w(1);
			m_cpes[ 1 ]->clk_w(1);
			m_cpes[ 0 ]->clk_w(1);
		} else {
			// Data propagate to left (every op but right-shift)
			m_cpes[ 0 ]->clk_w(1);
			m_cpes[ 1 ]->clk_w(1);
			m_cpes[ 2 ]->clk_w(1);
			m_cpes[ 3 ]->clk_w(1);
		}

		// Update MCU
		if (m_in_sel == IN_SEL_AC0) {
			m_mcu->ac_w(m_ac);
		} else {
			m_mcu->ac_w((m_ac & 0b1111110) | m_inputs[ m_in_sel ]);
		}
		m_mcu->clk_w(1);

		m_icount--;
	} while (m_icount > 0);
}

device_memory_interface::space_config_vector isbc202_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM , &m_program_config)
	};
}

std::unique_ptr<util::disasm_interface> isbc202_device::create_disassembler()
{
	return std::make_unique<isbc202_disassembler>();
}

void isbc202_device::state_import(const device_state_entry &entry)
{
	switch (entry.index()) {
	case i3002_device::REG_R0:
	case i3002_device::REG_R1:
	case i3002_device::REG_R2:
	case i3002_device::REG_R3:
	case i3002_device::REG_R4:
	case i3002_device::REG_R5:
	case i3002_device::REG_R6:
	case i3002_device::REG_R7:
	case i3002_device::REG_R8:
	case i3002_device::REG_R9:
	case i3002_device::REG_T:
	case i3002_device::REG_AC:
	case i3002_device::REG_MAR:
		m_cpes[ 0 ]->get_reg(entry.index()) = m_regs[ entry.index() ] & i3002_device::WORD_MASK;
		m_cpes[ 1 ]->get_reg(entry.index()) = (m_regs[ entry.index() ] >> 2) & i3002_device::WORD_MASK;
		m_cpes[ 2 ]->get_reg(entry.index()) = (m_regs[ entry.index() ] >> 4) & i3002_device::WORD_MASK;
		m_cpes[ 3 ]->get_reg(entry.index()) = (m_regs[ entry.index() ] >> 6) & i3002_device::WORD_MASK;
		break;

	default:
		break;
	}
}

void isbc202_device::state_export(const device_state_entry &entry)
{
	switch (entry.index()) {
	case STATE_GENFLAGS:
		m_flags = 0;
		if (m_inputs[ IN_SEL_CO ]) {
			BIT_SET(m_flags , 0);
		}
		if (m_mcu->fo_r()) {
			BIT_SET(m_flags , 1);
		}
		if (m_mcu->carry_r()) {
			BIT_SET(m_flags , 2);
		}
		if (m_mcu->zero_r()) {
			BIT_SET(m_flags , 3);
		}
		break;

	case i3002_device::REG_R0:
	case i3002_device::REG_R1:
	case i3002_device::REG_R2:
	case i3002_device::REG_R3:
	case i3002_device::REG_R4:
	case i3002_device::REG_R5:
	case i3002_device::REG_R6:
	case i3002_device::REG_R7:
	case i3002_device::REG_R8:
	case i3002_device::REG_R9:
	case i3002_device::REG_T:
	case i3002_device::REG_AC:
	case i3002_device::REG_MAR:
		m_regs[ entry.index() ] = m_cpes[ 3 ]->get_reg(entry.index());
		m_regs[ entry.index() ] <<= 2;
		m_regs[ entry.index() ] |= m_cpes[ 2 ]->get_reg(entry.index());
		m_regs[ entry.index() ] <<= 2;
		m_regs[ entry.index() ] |= m_cpes[ 1 ]->get_reg(entry.index());
		m_regs[ entry.index() ] <<= 2;
		m_regs[ entry.index() ] |= m_cpes[ 0 ]->get_reg(entry.index());
		break;

	default:
		break;
	}
}

void isbc202_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index()) {
	case STATE_GENFLAGS:
		str = string_format("%c %c %s %s" ,
							BIT(m_flags , 3) ? 'Z' : '-' ,
							BIT(m_flags , 2) ? 'C' : '-' ,
							BIT(m_flags , 1) ? "FO" : "--" ,
							BIT(m_flags , 0) ? "FI" : "--");
		break;

	default:
		break;
	}
}

void isbc202_device::set_output()
{
	switch (m_out_sel) {
	case 0:
		// Bit      What
		// 6..3     -
		// 2        Head load
		// 1        Start timeout
		// 0        Step pulse
		if (BIT(m_mask , 1)) {
			m_inputs[ IN_SEL_TIMEOUT ] = false;
			m_timeout_timer->adjust(attotime::from_msec(TIMEOUT_MS));
		}
		if (BIT(m_mask , 0) && (!m_direction || m_current_drive->trk00_r())) {
			LOG_DR("Step %s\n" , m_direction ? "OUT" : "IN");
			m_current_drive->dir_w(m_direction);
			m_current_drive->stp_w(0);
			m_current_drive->stp_w(1);
		}
		break;

	case 1:
		// Bit      What
		// 6        Reset data overrun
		// 5        Set XACK
		// 4        Set write inhibit
		// 3        Clear write inhibit
		// 2        -
		// 1        Reset RDY latches (0)
		// 0        -
		if (BIT(m_mask , 5)) {
			if (m_cpu != nullptr) {
				// Release CPU from wait state
				LOG_BUS("CPU out of wait state\n");
				m_cpu->trigger(1);
				if (!m_cpu_rd) {
					m_cpu = nullptr;
				}
				// Ensure the MCU executes a few instruction before the CPU
				machine().scheduler().boost_interleave(attotime::from_usec(1) , attotime::from_usec(5));
			} else {
				LOG("No CPU to wake up?\n");
			}
			m_inputs[ IN_SEL_START ] = false;
		}
		if (BIT(m_mask , 4)) {
			m_wrt_inh = true;
		}
		if (BIT(m_mask , 3)) {
			m_wrt_inh = false;
		}
		if (!BIT(m_mask , 1)) {
			m_ready_ff = 0xf;
		}
		break;

	case 2:
		// Bit      What
		// 6..2     -
		// 1..0     s1:s0 for PX input selection
		m_px_s1s0 = m_mask & 3;
		break;

	case 3:
		// Bit      What
		// 6        Z2
		// 5        SR OUT (0 = DATA, 1 = CRC)
		// 4        Write gate (0)
		// 3        -
		// 2        Z1
		// 1..0     -
		m_crc_out = BIT(m_mask , 5);
		set_rd_wr(m_reading, !BIT(m_mask , 4));
		break;

	case 4:
		// Bit      What
		// 6        Stepping direction (1 = out, 0 = in)
		// 5        CRC enable (0)
		// 4..0     -
		m_direction = BIT(m_mask , 6);
		m_crc_enabled = !BIT(m_mask , 5);
		break;

	case 5:
		// Bit      What
		// 6..5     -
		// 4        Mem write (0)
		// 3        GATE LOWER (0)
		// 2        -
		// 1        INOP RESET (0)
		// 0        AMWRT
		m_mem_wrt = !BIT(m_mask , 4);
		m_gate_lower = BIT(m_mask , 3);
		m_amwrt = BIT(m_mask , 0);
		break;

	case 6:
		// Bit      What
		// 6        Latch data bus into A24/A25
		// 5        Set INT FF
		// 4        Set Track > 43
		// 3        Latch D into A[7..0]
		// 2        Latch D into D[7..0]
		// 1        Latch D into D[F..8]
		// 0        Set XFER REQ
		if (BIT(m_mask , 6)) {
			m_data_low_in = m_cpu_data;
		}
		if (BIT(m_mask , 5)) {
			m_irq = true;
		}
		if (BIT(m_mask , 3)) {
			m_addr_low_out = dbus_r();
		}
		if (BIT(m_mask , 2)) {
			m_data_low_out = dbus_r();
		}
		if (BIT(m_mask , 0)) {
			if (m_mem_wrt) {
				if (!m_wrt_inh) {
					// CPU memory write
					uint16_t addr = m_addr_low_out | (uint16_t(abus_r()) << 8);
					if (m_mem_space) {
						LOG_BUS("MEM W %04x=%02x\n" , addr , m_data_low_out);
						m_mem_space->write_byte(addr , m_data_low_out);
					} else {
						LOG("CPU AS not set!\n");
					}
				}
			} else {
				// CPU memory read
				uint16_t addr = m_addr_low_out | (uint16_t(abus_r()) << 8);
				if (m_mem_space) {
					m_data_low_in = m_mem_space->read_byte(addr);
					LOG_BUS("MEM R %04x=%02x\n" , addr , m_data_low_in);
				} else {
					LOG("CPU AS not set!\n");
				}
			}
		}
		break;

	case 7:
		// Bit      What
		// 6        Latch OP & US
		// 5        Reset INDEX
		// 4        Set Track <= 43
		// 3        Clear START/STOP
		// 2        Unload head
		// 1        Set RESET READ
		// 0        Clear RESET READ
		if (BIT(m_mask , 6)) {
			m_op_us = dbus_r() & 0x3f;
			m_current_drive = m_drives[ selected_drive() ]->get_device();
		}
		if (BIT(m_mask , 5)) {
			m_inputs[ IN_SEL_INDEX ] = false;
		}
		if (BIT(m_mask , 3)) {
			m_cmd = 0;
			LOG_BUS("CLR ST\n");
			// TODO: more
		}
		if (BIT(m_mask , 1)) {
			set_rd_wr(false, m_writing);
		}
		if (BIT(m_mask , 0)) {
			set_rd_wr(true, m_writing);
		}
		break;
	}
}

unsigned isbc202_device::selected_drive() const
{
	return (m_op_us >> 4) & 3;
}

unsigned isbc202_device::drive_idx(floppy_image_device *drive)
{
	for (unsigned i = 0; i < 4; ++i) {
		if (drive == m_drives[ i ]->get_device()) {
			return i;
		}
	}
	LOG("Unknown drive!\n");
	return 0;
}

void isbc202_device::floppy_ready_cb(floppy_image_device *floppy , int state)
{
	unsigned idx = drive_idx(floppy);

	uint8_t old_state = m_ready_in;

	if (!state) {
		BIT_SET(m_ready_in , idx);
	} else {
		BIT_CLR(m_ready_in , idx);
	}

	uint8_t gone_not_ready = ~m_ready_in & old_state;

	BIT_CLR(gone_not_ready, selected_drive());

	m_ready_ff &= ~gone_not_ready;
}

void isbc202_device::floppy_index_cb(floppy_image_device *floppy , int state)
{
	if (state && floppy == m_current_drive) {
		LOG_DR("Index @%.6f\n" , machine().time().as_double());
		m_inputs[ IN_SEL_INDEX ] = true;
	}
}

uint8_t isbc202_device::dbus_r() const
{
	return m_cpes[ 0 ]->dbus_r() |
		(m_cpes[ 1 ]->dbus_r() << 2) |
		(m_cpes[ 2 ]->dbus_r() << 4) |
		(m_cpes[ 3 ]->dbus_r() << 6);
}

uint8_t isbc202_device::mbus_r() const
{
	return m_gate_lower ? 0 : m_data_low_in;
}

uint8_t isbc202_device::ibus_r()
{
	if (!m_ibus_cached) {
		m_ibus_cached = true;
		m_ibus = 0xff;
		if (BIT(m_slk , 0) && BIT(m_mask , 7)) {
			if (BIT(m_mask , 5)) {
				// Bit      What
				// 7        Drive 3/1 ready
				// 6        Drive 2/0 ready
				// 5        Track 0
				// 4        STOP
				// 3..0     1
				m_ibus = 0x0f;
				uint8_t ready = m_ready_in & m_ready_ff;
				if (m_gate_lower) {
					ready >>= 2;
				}
				if (BIT(ready , 1)) {
					BIT_SET(m_ibus, 7);
				}
				if (BIT(ready , 0)) {
					BIT_SET(m_ibus, 6);
				}
				if (!m_current_drive->trk00_r()) {
					BIT_SET(m_ibus, 5);
				}
				// TODO: STOP bit
			} else if (BIT(m_mask , 6)) {
				// Bit      What
				// 7        Selected drive not ready
				// 6        Write fault
				// 5        Write protection
				// 4        Data overrun
				// 3..0     1
				m_ibus = 0x0f;
				if (!BIT(m_ready_in , selected_drive())) {
					BIT_SET(m_ibus, 7);
				}
				if (m_current_drive->wpt_r()) {
					BIT_SET(m_ibus, 5);
				}
			} else if (BIT(m_mask , 3)) {
				// Read clock SR
				m_ibus = aligned_rd_data(m_clock_sr);
			} else if (BIT(m_mask , 4)) {
				// Read data SR
				m_ibus = aligned_rd_data(m_data_sr);
			}
		}
	}

	return m_ibus;
}

uint8_t isbc202_device::abus_r() const
{
	return m_cpes[ 0 ]->abus_r() |
		(m_cpes[ 1 ]->abus_r() << 2) |
		(m_cpes[ 2 ]->abus_r() << 4) |
		(m_cpes[ 3 ]->abus_r() << 6);
}

void isbc202_device::set_start(uint8_t off , bool read)
{
	m_cmd = off;
	m_inputs[ IN_SEL_START ] = true;
	// Put CPU in wait state
	m_cpu->spin_until_trigger(1);
	m_cpu_rd = read;
	LOG_BUS("CPU in wait state (rd=%d)\n" , read);
	if (read) {
		// If CPU is suspended when reading, rewind PC so that the
		// "IN" instruction is repeated when CPU is released
		m_cpu->set_pc(m_cpu->pc() - 2);
	}
}

void isbc202_device::set_rd_wr(bool new_rd , bool new_wr)
{
	if (!m_reading && new_rd) {
		// Start reading
		LOG_RD("Start RD @%.6f\n" , machine().time().as_double());
		m_pll.set_clock(attotime::from_usec(HALF_BIT_CELL_US));
		m_pll.read_reset(machine().time());

		// Search for next SYNC (16x 1 and a 0)
		m_byte_timer->reset();
		m_f_timer->reset();
		unsigned cnt_trans = 0;
		attotime rot_period = attotime::from_hz(6);
		while ((m_pll.ctime - machine().time()) < rot_period) {
			attotime edge = m_current_drive->get_next_transition(m_pll.ctime);
			if (edge.is_never()) {
				break;
			}
			attotime tm;
			bool bit = m_pll.feed_read_data(tm , edge , attotime::never);
			if (cnt_trans < 32) {
				if (!(BIT(cnt_trans , 0) ^ bit)) {
					cnt_trans++;
				} else {
					cnt_trans = 0;
				}
			} else if (cnt_trans == 32) {
				if (!bit) {
					cnt_trans++;
				} else {
					cnt_trans = 0;
				}
			} else {
				if (!bit) {
					LOG_RD("SYNC found @%.6f\n" , m_pll.ctime.as_double());
					// GOT SYNC!
					if (m_crc_enabled) {
						// CRC shouldn't be enabled here or register won't get cleared
						LOG("Huh? CRC enabled during SYNC scan?\n");
					}
					m_crc = 0;
					// Load the "0" bit into data/clock SR
					m_data_sr = 0;
					m_clock_sr = 0;
					// Read 7 more bits to make a full byte
					rd_bits(7);
					// Timer to go off at end of 8th bit of AM byte (when F signal goes high)
					m_byte_timer->adjust(m_pll.ctime - machine().time());
					break;
				} else {
					cnt_trans = 32;
				}
			}
		}
	} else if (m_reading && !new_rd) {
		// Stop reading
		LOG_RD("Stop RD\n");
		m_byte_timer->reset();
		m_f_timer->reset();
		m_inputs[ IN_SEL_F ] = false;
	}
	m_reading = new_rd;

	if (!m_writing && new_wr) {
		// Start writing
		LOG_WR("Start WR\n");
		m_pll.set_clock(attotime::from_usec(HALF_BIT_CELL_US));
		m_pll.start_writing(machine().time());
		m_pll.ctime = machine().time();
		m_last_data_bit = false;
		m_byte_timer->adjust(attotime::from_usec(HALF_BIT_CELL_US * 14));
	} else if (m_writing && !new_wr) {
		// Stop writing
		LOG_WR("Stop WR\n");
		m_pll.stop_writing(m_current_drive , machine().time());
		m_byte_timer->reset();
		m_f_timer->reset();
		m_inputs[ IN_SEL_F ] = false;
	}
	m_writing = new_wr;
}

uint8_t isbc202_device::aligned_rd_data(uint16_t sr)
{
	attotime tmp{ machine().time() - m_last_f_time };

	// Compute how many bit cells have gone by since the last time F went high
	unsigned bits = tmp.as_ticks(BIT_FREQUENCY);
	if (bits) {
		LOG_RD("Aligning by %u bits\n" , bits);
		sr <<= bits;
	}
	return uint8_t(sr >> 8);
}

void isbc202_device::rd_bits(unsigned n)
{
	while (n--) {
		attotime edge = m_current_drive->get_next_transition(m_pll.ctime);
		if (edge.is_never()) {
			break;
		}
		attotime tm;
		bool clock_bit = m_pll.feed_read_data(tm , edge , attotime::never);
		edge = m_current_drive->get_next_transition(m_pll.ctime);
		if (edge.is_never()) {
			break;
		}
		bool data_bit = m_pll.feed_read_data(tm , edge , attotime::never);

		m_clock_sr = (m_clock_sr << 1) | clock_bit;
		bool crc_bit = BIT(m_data_sr , 15);
		m_data_sr = (m_data_sr << 1) | data_bit;
		update_crc(crc_bit);
	}
	LOG_RD("CLK %04x DT %04x CRC %04x\n" , m_clock_sr , m_data_sr , m_crc);
}

void isbc202_device::write_byte()
{
	LOG_WR("WR DT %02x CRC %04x CE %d CO %d AW %d @%.6f\n" , m_data_sr & 0xff , m_crc , m_crc_enabled , m_crc_out , m_dlyd_amwrt , machine().time().as_double());

	for (unsigned i = 0; i < 8; i++) {
		bool sr_bit = BIT(m_data_sr , 7);
		bool crc_bit = update_crc(sr_bit);
		bool data_bit = m_crc_out ? crc_bit : sr_bit;
		bool clock_bit = m_clock_gate && !data_bit && !m_last_data_bit;
		if (i <= 3 && m_dlyd_amwrt) {
			m_clock_gate = true;
		} else {
			m_clock_gate = !data_bit && !clock_bit;
		}
		attotime dummy;

		m_pll.write_next_bit(clock_bit , dummy , nullptr , attotime::never);
		m_pll.write_next_bit(data_bit , dummy , nullptr , attotime::never);
		m_data_sr <<= 1;
		m_last_data_bit = data_bit;
	}
	m_pll.commit(m_current_drive , machine().time());
	m_pll.ctime = machine().time();
}

bool isbc202_device::update_crc(bool bit)
{
	bool out = BIT(m_crc , 15);

	if (m_crc_enabled && (out ^ bit)) {
		m_crc = (m_crc << 1) ^ CRC_POLY;
	} else {
		m_crc <<= 1;
	}

	return out;
}
