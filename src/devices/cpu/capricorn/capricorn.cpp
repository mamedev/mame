// license:BSD-3-Clause
// copyright-holders:F. Ulivi
//
// *****************************
// Emulator for HP Capricorn CPU
// *****************************
//
#include "emu.h"
#include "capricorn.h"
#include "capricorn_dasm.h"
#include "debugger.h"

// Register indexes
// GP registers are named "R" & the octal representation of the index (00-77)
enum {
	CAPRICORN_R00,
	CAPRICORN_R01,
	CAPRICORN_R02,
	CAPRICORN_R03,
	CAPRICORN_R04,
	CAPRICORN_R05,
	CAPRICORN_R06,
	CAPRICORN_R07,
	CAPRICORN_R10,
	CAPRICORN_R11,
	CAPRICORN_R12,
	CAPRICORN_R13,
	CAPRICORN_R14,
	CAPRICORN_R15,
	CAPRICORN_R16,
	CAPRICORN_R17,
	CAPRICORN_R20,
	CAPRICORN_R21,
	CAPRICORN_R22,
	CAPRICORN_R23,
	CAPRICORN_R24,
	CAPRICORN_R25,
	CAPRICORN_R26,
	CAPRICORN_R27,
	CAPRICORN_R30,
	CAPRICORN_R31,
	CAPRICORN_R32,
	CAPRICORN_R33,
	CAPRICORN_R34,
	CAPRICORN_R35,
	CAPRICORN_R36,
	CAPRICORN_R37,
	CAPRICORN_R40,
	CAPRICORN_R41,
	CAPRICORN_R42,
	CAPRICORN_R43,
	CAPRICORN_R44,
	CAPRICORN_R45,
	CAPRICORN_R46,
	CAPRICORN_R47,
	CAPRICORN_R50,
	CAPRICORN_R51,
	CAPRICORN_R52,
	CAPRICORN_R53,
	CAPRICORN_R54,
	CAPRICORN_R55,
	CAPRICORN_R56,
	CAPRICORN_R57,
	CAPRICORN_R60,
	CAPRICORN_R61,
	CAPRICORN_R62,
	CAPRICORN_R63,
	CAPRICORN_R64,
	CAPRICORN_R65,
	CAPRICORN_R66,
	CAPRICORN_R67,
	CAPRICORN_R70,
	CAPRICORN_R71,
	CAPRICORN_R72,
	CAPRICORN_R73,
	CAPRICORN_R74,
	CAPRICORN_R75,
	CAPRICORN_R76,
	CAPRICORN_R77,
	CAPRICORN_ARP,
	CAPRICORN_DRP,
	CAPRICORN_E
};

// Bit manipulation
namespace {
	static constexpr unsigned BIT_MASK(unsigned n)
	{
		return 1U << n;
	}

	template<typename T> void BIT_CLR(T& w , unsigned n)
	{
		w &= ~(T)BIT_MASK(n);
	}

	template<typename T> void BIT_SET(T& w , unsigned n)
	{
		w |= (T)BIT_MASK(n);
	}

	template<typename T> void COPY_BIT(bool bit , T& w , unsigned n)
	{
		if (bit) {
			BIT_SET(w , n);
		} else {
			BIT_CLR(w , n);
		}
	}
}

// Bits in m_flags
static constexpr unsigned FLAGS_DCM_BIT     = 0;    // Decimal/binary mode
static constexpr unsigned FLAGS_CY_BIT      = 1;    // Carry
static constexpr unsigned FLAGS_OVF_BIT     = 2;    // Overflow
static constexpr unsigned FLAGS_LSB_BIT     = 3;    // LSB
static constexpr unsigned FLAGS_MSB_BIT     = 4;    // MSB
static constexpr unsigned FLAGS_Z_BIT       = 5;    // Zero
static constexpr unsigned FLAGS_LDZ_BIT     = 6;    // Left digit zero
static constexpr unsigned FLAGS_RDZ_BIT     = 7;    // Right digit zero
static constexpr unsigned FLAGS_IRL_BIT     = 8;    // Interrupt request

// Special registers
static constexpr unsigned REG_BANK_PTR      = 0;    // R0: register bank pointer
static constexpr unsigned REG_INDEX_SCRATCH = 2;    // R2 & R3: index scratch registers
static constexpr unsigned REG_PC            = 4;    // R4 & R5: PC
static constexpr unsigned REG_SP            = 6;    // R6 & R7: return stack pointer

// Bit in address values that specifies external address (0) or internal register (1)
static constexpr unsigned GP_REG_BIT    = 17;
static constexpr unsigned GP_REG_MASK   = BIT_MASK(GP_REG_BIT);
static constexpr unsigned ADDR_MASK     = 0xffff;

// Mask of bits in ARP & DRP
static constexpr uint8_t ARP_DRP_MASK   = 0x3f;

// Mask of bits in E
static constexpr uint8_t E_MASK = 0xf;

DEFINE_DEVICE_TYPE(HP_CAPRICORN , capricorn_cpu_device , "capricorn" , "HP-Capricorn")

capricorn_cpu_device::capricorn_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, HP_CAPRICORN, tag, owner, clock),
	  m_program_config("program" , ENDIANNESS_LITTLE , 8 , 16)
{
}

uint8_t capricorn_cpu_device::flatten_burst()
{
	m_flatten = true;
	return (uint8_t)(m_curr_addr - m_start_addr);
}

void capricorn_cpu_device::device_start()
{
	// ARP
	state_add(CAPRICORN_ARP , "ARP" , m_arp).formatstr("%2s");
	// DRP
	state_add(CAPRICORN_DRP , "DRP" , m_drp).formatstr("%2s");
	// E
	state_add(CAPRICORN_E , "E" , m_reg_E).formatstr("%1X");
	// R00 .. R77
	for (unsigned i = 0; i < 64; i++) {
		state_add(CAPRICORN_R00 + i , util::string_format("R%02o" , i).c_str() , m_reg[ i ]);
	}
	// PC
	state_add(STATE_GENPC, "GENPC", m_genpc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_genpc).noshow();
	// Flags
	state_add(STATE_GENFLAGS , "GENFLAGS" , m_flags).noshow().formatstr("%9s");

	m_program = &space(AS_PROGRAM);
	m_cache = m_program->cache<0, 0, ENDIANNESS_LITTLE>();

	save_item(NAME(m_reg));
	save_item(NAME(m_arp));
	save_item(NAME(m_drp));
	save_item(NAME(m_reg_E));
	save_item(NAME(m_flags));

	set_icountptr(m_icount);
}

void capricorn_cpu_device::device_reset()
{
	for (auto& reg : m_reg) {
		reg = 0;
	}
	m_arp = 0;
	m_drp = 0;
	m_reg_E = 0;
	m_flags = 0;
	// Reset vector @0
	vector_to_pc(0);
}

void capricorn_cpu_device::execute_run()
{
	do {
		if (BIT(m_flags , FLAGS_IRL_BIT)) {
			// Handle interrupt
			take_interrupt();
		} else {
			debugger_instruction_hook(m_genpc);

			uint8_t opcode = fetch();
			execute_one(opcode);
			offset_pc(1);
		}
	} while (m_icount > 0);
}

void capricorn_cpu_device::execute_set_input(int linenum, int state)
{
	if (linenum == 0) {
		COPY_BIT(state != 0 , m_flags, FLAGS_IRL_BIT);
	}
}

device_memory_interface::space_config_vector capricorn_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

void capricorn_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	if (entry.index() == CAPRICORN_ARP) {
		str = string_format("%02o" , m_arp);
	} else if (entry.index() == CAPRICORN_DRP) {
		str = string_format("%02o" , m_drp);
	} else if (entry.index() == STATE_GENFLAGS) {
		str = string_format("%s %c %c %c" , BIT(m_flags , FLAGS_DCM_BIT) ? "BCD" : "BIN",
							BIT(m_flags , FLAGS_CY_BIT) ? 'C' : ' ',
							BIT(m_flags , FLAGS_OVF_BIT) ? 'O' : ' ',
							BIT(m_flags , FLAGS_Z_BIT) ? 'Z' : ' ');
	}
}

std::unique_ptr<util::disasm_interface> capricorn_cpu_device::create_disassembler()
{
	return std::make_unique<capricorn_disassembler>();
}

void capricorn_cpu_device::start_mem_burst(ea_addr_t addr)
{
	m_flatten = false;
	// Only relevant for memory access (not for internal registers)
	m_start_addr = (uint16_t)(addr & ADDR_MASK);
}

uint16_t capricorn_cpu_device::read_u16(ea_addr_t addr)
{
	PAIR16 tmp;

	start_mem_burst(addr);
	tmp.b.l = RM(addr);
	tmp.b.h = RM(addr);
	return tmp.w;
}

void capricorn_cpu_device::write_u16(ea_addr_t addr , uint16_t v)
{
	PAIR16 tmp;

	tmp.w = v;

	start_mem_burst(addr);
	WM(addr , tmp.b.l);
	WM(addr , tmp.b.h);
}

uint8_t capricorn_cpu_device::RM(ea_addr_t& addr)
{
	uint8_t res;

	if (BIT(addr , GP_REG_BIT)) {
		res = m_reg[ addr & ARP_DRP_MASK ];
	} else {
		m_curr_addr = (uint16_t)(addr & ADDR_MASK);
		res = m_program->read_byte(m_flatten ? m_start_addr : m_curr_addr);
	}
	addr++;
	return res;
}

void capricorn_cpu_device::WM(ea_addr_t& addr , uint8_t v)
{
	if (BIT(addr , GP_REG_BIT)) {
		m_reg[ addr & ARP_DRP_MASK ] = v;
	} else {
		m_curr_addr = (uint16_t)(addr & ADDR_MASK);
		m_program->write_byte(m_flatten ? m_start_addr : m_curr_addr , v);
	}
	addr++;
}

uint8_t capricorn_cpu_device::fetch()
{
	m_genpc = read_u16(REG_PC | GP_REG_MASK);
	return m_cache->read_byte(m_genpc);
}

void capricorn_cpu_device::offset_pc(uint16_t offset)
{
	m_genpc = read_u16(REG_PC | GP_REG_MASK);
	m_genpc += offset;
	write_u16(REG_PC | GP_REG_MASK, m_genpc);
}

void capricorn_cpu_device::vector_to_pc(uint8_t vector)
{
	m_genpc = read_u16(vector);
	write_u16(REG_PC | GP_REG_MASK, m_genpc);
}

void capricorn_cpu_device::do_jump(bool condition)
{
	m_icount -= 4;
	offset_pc(1);
	uint16_t disp = fetch();
	if (condition) {
		m_icount--;
		if (BIT(disp , 7)) {
			disp -= 0x100;
		}
		offset_pc(disp);
	}
}

uint8_t capricorn_cpu_device::get_lower_boundary() const
{
	if (BIT(m_drp , 5)) {
		return m_drp & ~7U;
	} else {
		return m_drp & ~1U;
	}
}

uint8_t capricorn_cpu_device::get_upper_boundary() const
{
	if (BIT(m_drp , 5)) {
		return m_drp | 7;
	} else {
		return m_drp | 1;
	}
}

void capricorn_cpu_device::update_flags_right(uint8_t res)
{
	// Update RDZ & LSB flags on least significant byte of single/multi byte operations
	COPY_BIT((res & 0xf) == 0 , m_flags, FLAGS_RDZ_BIT);
	COPY_BIT(BIT(res , 0) , m_flags, FLAGS_LSB_BIT);
}

void capricorn_cpu_device::update_flags_left(uint8_t res)
{
	// Update LDZ, MSB flags on most significant byte of single/multi byte operations
	COPY_BIT((res & 0xf0) == 0 , m_flags, FLAGS_LDZ_BIT);
	COPY_BIT(BIT(res , 7) , m_flags, FLAGS_MSB_BIT);
}

void capricorn_cpu_device::update_flags_every(uint8_t res)
{
	// Update Z flag on every byte of single/multi byte operations
	// It's assumed that Z=1 at start of each operation
	if (res) {
		BIT_CLR(m_flags, FLAGS_Z_BIT);
	}
}

capricorn_cpu_device::ea_addr_t capricorn_cpu_device::get_ea_reg_imm()
{
	// Register immediate addressing mode
	return (ea_addr_t)m_arp | GP_REG_MASK;
}

capricorn_cpu_device::ea_addr_t capricorn_cpu_device::get_ea_lit_imm(bool multibyte)
{
	// Literal immediate addressing mode
	offset_pc(1);
	ea_addr_t res = m_genpc;
	if (multibyte) {
		offset_pc(get_upper_boundary() - m_drp);
	}
	return res;
}

capricorn_cpu_device::ea_addr_t capricorn_cpu_device::get_ea_reg_dir()
{
	// Register direct addressing mode
	m_icount--;
	return read_u16(m_arp | GP_REG_MASK);
}

capricorn_cpu_device::ea_addr_t capricorn_cpu_device::get_ea_lit_dir()
{
	// Literal direct addressing mode
	m_icount--;
	offset_pc(1);
	ea_addr_t res = read_u16(m_genpc);
	offset_pc(1);
	return res;
}

capricorn_cpu_device::ea_addr_t capricorn_cpu_device::get_ea_reg_indir()
{
	// Register indirect addressing mode
	m_icount -= 3;
	return read_u16(read_u16(m_arp | GP_REG_MASK));
}

capricorn_cpu_device::ea_addr_t capricorn_cpu_device::get_ea_idx_dir()
{
	// Indexed direct addressing mode
	m_icount -= 3;
	offset_pc(1);
	uint16_t res = read_u16(m_genpc) + read_u16(m_arp | GP_REG_MASK);
	offset_pc(1);
	write_u16(REG_INDEX_SCRATCH | GP_REG_MASK, res);
	return res;
}

capricorn_cpu_device::ea_addr_t capricorn_cpu_device::get_ea_lit_indir()
{
	// Literal indirect addressing mode
	m_icount -= 3;
	offset_pc(1);
	ea_addr_t res = read_u16(read_u16(m_genpc));
	offset_pc(1);
	return res;
}

capricorn_cpu_device::ea_addr_t capricorn_cpu_device::get_ea_idx_indir()
{
	// Indexed indirect addressing mode
	m_icount -= 5;
	offset_pc(1);
	uint16_t res = read_u16(m_genpc) + read_u16(m_arp | GP_REG_MASK);
	offset_pc(1);
	write_u16(REG_INDEX_SCRATCH | GP_REG_MASK, res);
	return read_u16(res);
}

uint8_t capricorn_cpu_device::add_bcd_digits(uint8_t first , uint8_t second , bool& carry)
{
	if (first > 9) {
		first -= 8;
	}
	uint8_t res = first + second + carry;
	carry = res > 9;
	if (carry) {
		res -= 10;
	}
	return res;
}

uint8_t capricorn_cpu_device::add_bcd_bytes(uint8_t first , uint8_t second , bool& carry)
{
	uint8_t rd = add_bcd_digits(first & 0xf, second & 0xf, carry);
	uint8_t ld = add_bcd_digits(first >> 4, second >> 4, carry);
	return (ld << 4) | rd;
}

uint8_t capricorn_cpu_device::sub_bcd_digits(uint8_t first , uint8_t second , bool& carry)
{
	int res = first + (9 - second) + carry;
	carry = false;
	if (res < 0) {
		res += 8;
	} else if (res >= 10) {
		res -= 10;
		carry = true;
	}
	return (uint8_t)res;
}

uint8_t capricorn_cpu_device::sub_bcd_bytes(uint8_t first , uint8_t second , bool& carry)
{
	uint8_t rd = sub_bcd_digits(first & 0xf, second & 0xf, carry);
	uint8_t ld = sub_bcd_digits(first >> 4, second >> 4, carry);
	return (ld << 4) | rd;
}

#define OP_ITERATION_START_FWD(idx , multi)                     \
	unsigned boundary = multi ? get_upper_boundary() : m_drp;   \
	BIT_SET(m_flags , FLAGS_Z_BIT);                             \
	bool first = true;                                          \
	for (unsigned idx = m_drp; idx <= boundary; idx++)

#define OP_ITERATION_START_REV(idx , multi)                 \
	int boundary = multi ? get_lower_boundary() : m_drp;    \
	BIT_SET(m_flags , FLAGS_Z_BIT);                         \
	bool first = true;                                      \
	for (int idx = m_drp; idx >= boundary; idx--)

#define OP1_GET(idx , op1)                      \
	m_icount--;                                 \
	uint8_t op1 = m_reg[ idx ];

#define OP2_GET(idx , ea , op1 , op2)           \
	m_icount--;                                 \
	uint8_t op1 = m_reg[ idx ];                 \
	uint8_t op2 = RM(ea);

#define RES_SET(idx , res)                      \
	m_reg[ idx ] = res;

#define OP_ITERATION_END_FWD(res)               \
	if (first) {                                \
		update_flags_right(res);                \
		first = false;                          \
	}                                           \
	update_flags_left(res);                     \
	update_flags_every(res);

#define OP_ITERATION_END_REV(res)               \
	if (first) {                                \
		update_flags_left(res);                 \
		first = false;                          \
	}                                           \
	update_flags_right(res);                    \
	update_flags_every(res);

void capricorn_cpu_device::do_AN_op(ea_addr_t ea)
{
	m_icount -= 4;
	BIT_CLR(m_flags , FLAGS_CY_BIT);
	BIT_CLR(m_flags , FLAGS_OVF_BIT);

	start_mem_burst(ea);

	OP_ITERATION_START_FWD(i , true) {
		OP2_GET(i , ea , op1 , op2);

		uint8_t res = op1 & op2;
		RES_SET(i , res);

		OP_ITERATION_END_FWD(res);
	}
}

void capricorn_cpu_device::do_LD_op(ea_addr_t ea , bool multibyte)
{
	m_icount -= 4;
	BIT_CLR(m_flags , FLAGS_CY_BIT);
	BIT_CLR(m_flags , FLAGS_OVF_BIT);

	start_mem_burst(ea);

	OP_ITERATION_START_FWD(i , multibyte) {
		OP2_GET(i , ea , op1 , op2);

		// op1 is unused (it's overwritten by op2)
		(void)op1;
		uint8_t res = op2;
		RES_SET(i , res);

		OP_ITERATION_END_FWD(res);
	}
}

void capricorn_cpu_device::do_ST_op(ea_addr_t ea , bool multibyte)
{
	m_icount -= 4;
	BIT_CLR(m_flags , FLAGS_CY_BIT);
	BIT_CLR(m_flags , FLAGS_OVF_BIT);

	start_mem_burst(ea);

	OP_ITERATION_START_FWD(i , multibyte) {
		OP1_GET(i , op1);

		WM(ea , op1);
		OP_ITERATION_END_FWD(op1);
	}
}

void capricorn_cpu_device::do_AD_op(ea_addr_t ea , bool multibyte)
{
	m_icount -= 4;
	bool carry = false;
	bool ovf = false;

	start_mem_burst(ea);

	OP_ITERATION_START_FWD(i , multibyte) {
		OP2_GET(i , ea , op1 , op2);

		uint8_t res;

		if (BIT(m_flags , FLAGS_DCM_BIT)) {
			res = add_bcd_bytes(op1 , op2 , carry);
		} else {
			uint16_t tmp = (uint16_t)op1 + (uint16_t)op2 + carry;
			carry = BIT(tmp , 8);
			ovf = BIT((tmp ^ op1) & (tmp ^ op2) , 7);
			res = (uint8_t)tmp;
		}
		RES_SET(i , res);

		OP_ITERATION_END_FWD(res);
	}
	COPY_BIT(carry , m_flags , FLAGS_CY_BIT);
	COPY_BIT(ovf , m_flags , FLAGS_OVF_BIT);
}

void capricorn_cpu_device::do_SB_op(ea_addr_t ea , bool multibyte)
{
	m_icount -= 4;
	bool carry = true;
	bool ovf = false;

	start_mem_burst(ea);

	OP_ITERATION_START_FWD(i , multibyte) {
		OP2_GET(i , ea , op1 , op2);

		uint8_t res;

		if (BIT(m_flags , FLAGS_DCM_BIT)) {
			res = sub_bcd_bytes(op1 , op2 , carry);
		} else {
			op2 = ~op2;
			uint16_t tmp = (uint16_t)op1 + (uint16_t)op2 + carry;
			carry = BIT(tmp , 8);
			ovf = BIT((tmp ^ op1) & (tmp ^ op2) , 7);
			res = (uint8_t)tmp;
		}
		RES_SET(i , res);

		OP_ITERATION_END_FWD(res);
	}
	COPY_BIT(carry , m_flags , FLAGS_CY_BIT);
	COPY_BIT(ovf , m_flags , FLAGS_OVF_BIT);
}

void capricorn_cpu_device::do_CM_op(ea_addr_t ea , bool multibyte)
{
	m_icount -= 4;
	bool carry = true;
	bool ovf = false;

	start_mem_burst(ea);

	OP_ITERATION_START_FWD(i , multibyte) {
		OP2_GET(i , ea , op1 , op2);

		uint8_t res;

		if (BIT(m_flags , FLAGS_DCM_BIT)) {
			res = sub_bcd_bytes(op1 , op2 , carry);
		} else {
			op2 = ~op2;
			uint16_t tmp = (uint16_t)op1 + (uint16_t)op2 + carry;
			carry = BIT(tmp , 8);
			ovf = BIT((tmp ^ op1) & (tmp ^ op2) , 7);
			res = (uint8_t)tmp;
		}

		OP_ITERATION_END_FWD(res);
	}
	COPY_BIT(carry , m_flags , FLAGS_CY_BIT);
	COPY_BIT(ovf , m_flags , FLAGS_OVF_BIT);
}

void capricorn_cpu_device::do_OR_op(bool multibyte)
{
	m_icount -= 4;
	BIT_CLR(m_flags, FLAGS_CY_BIT);
	BIT_CLR(m_flags, FLAGS_OVF_BIT);

	ea_addr_t ea = get_ea_reg_imm();

	start_mem_burst(ea);

	OP_ITERATION_START_FWD(i , multibyte) {
		OP2_GET(i , ea , op1 , op2);

		uint8_t res = op1 | op2;
		RES_SET(i , res);

		OP_ITERATION_END_FWD(res);
	}
}

void capricorn_cpu_device::do_XR_op(bool multibyte)
{
	m_icount -= 4;
	BIT_CLR(m_flags, FLAGS_CY_BIT);
	BIT_CLR(m_flags, FLAGS_OVF_BIT);

	ea_addr_t ea = get_ea_reg_imm();

	start_mem_burst(ea);

	OP_ITERATION_START_FWD(i , multibyte) {
		OP2_GET(i , ea , op1 , op2);

		uint8_t res = op1 ^ op2;
		RES_SET(i , res);

		OP_ITERATION_END_FWD(res);
	}
}

void capricorn_cpu_device::do_IC_op(bool multibyte)
{
	m_icount -= 4;
	bool carry = true;
	bool ovf = false;

	OP_ITERATION_START_FWD(i , multibyte) {
		OP1_GET(i, op1);

		uint8_t res;

		if (BIT(m_flags , FLAGS_DCM_BIT)) {
			res = add_bcd_bytes(op1 , 0 , carry);
		} else {
			uint16_t tmp = (uint16_t)op1 + carry;
			carry = BIT(tmp , 8);
			// Overflow = 1 when a positive number is incremented into a negative one
			ovf = BIT(tmp & ~op1 , 7);
			res = (uint8_t)tmp;
		}
		RES_SET(i , res);

		OP_ITERATION_END_FWD(res);
	}
	COPY_BIT(carry , m_flags , FLAGS_CY_BIT);
	COPY_BIT(ovf , m_flags , FLAGS_OVF_BIT);
}

void capricorn_cpu_device::do_DC_op(bool multibyte)
{
	m_icount -= 4;
	bool carry = false;
	bool ovf = false;

	OP_ITERATION_START_FWD(i , multibyte) {
		OP1_GET(i, op1);

		uint8_t res;

		if (BIT(m_flags , FLAGS_DCM_BIT)) {
			res = sub_bcd_bytes(op1 , 0 , carry);
		} else {
			uint16_t tmp = (uint16_t)op1 + 0xff + carry;
			carry = BIT(tmp , 8);
			// Overflow = 1 when a negative number is decremented into a positive one
			ovf = BIT(~tmp & op1 , 7);
			res = (uint8_t)tmp;
		}
		RES_SET(i , res);

		OP_ITERATION_END_FWD(res);
	}
	COPY_BIT(carry , m_flags , FLAGS_CY_BIT);
	COPY_BIT(ovf , m_flags , FLAGS_OVF_BIT);
}

void capricorn_cpu_device::do_TC_op(bool multibyte)
{
	m_icount -= 4;
	bool carry = true;
	bool ovf = false;

	OP_ITERATION_START_FWD(i , multibyte) {
		OP1_GET(i, op1);

		uint8_t res;

		if (BIT(m_flags , FLAGS_DCM_BIT)) {
			res = sub_bcd_bytes(0 , op1 , carry);
		} else {
			op1 = ~op1;
			uint16_t tmp = (uint16_t)op1 + carry;
			carry = BIT(tmp , 8);
			ovf = BIT(tmp & ~op1 , 7);
			res = (uint8_t)tmp;
		}
		RES_SET(i , res);

		OP_ITERATION_END_FWD(res);
	}
	COPY_BIT(carry , m_flags , FLAGS_CY_BIT);
	COPY_BIT(ovf , m_flags , FLAGS_OVF_BIT);
}

void capricorn_cpu_device::do_NC_op(bool multibyte)
{
	m_icount -= 4;
	BIT_CLR(m_flags, FLAGS_CY_BIT);
	BIT_CLR(m_flags, FLAGS_OVF_BIT);

	OP_ITERATION_START_FWD(i , multibyte) {
		OP1_GET(i, op1);

		uint8_t res;

		if (BIT(m_flags , FLAGS_DCM_BIT)) {
			bool carry = false;
			res = sub_bcd_bytes(0 , op1 , carry);
		} else {
			res = ~op1;
		}
		RES_SET(i , res);

		OP_ITERATION_END_FWD(res);
	}
}

void capricorn_cpu_device::do_TS_op(bool multibyte)
{
	m_icount -= 4;
	BIT_CLR(m_flags, FLAGS_CY_BIT);
	BIT_CLR(m_flags, FLAGS_OVF_BIT);

	OP_ITERATION_START_FWD(i , multibyte) {
		OP1_GET(i, op1);

		OP_ITERATION_END_FWD(op1);
	}
}

void capricorn_cpu_device::do_CL_op(bool multibyte)
{
	m_icount -= 4;
	BIT_CLR(m_flags, FLAGS_CY_BIT);
	BIT_CLR(m_flags, FLAGS_OVF_BIT);

	OP_ITERATION_START_FWD(i , multibyte) {
		OP1_GET(i, op1);

		(void)op1;
		RES_SET(i , 0);
		OP_ITERATION_END_FWD(0);
	}
}

void capricorn_cpu_device::do_EL_op(bool multibyte)
{
	m_icount -= 4;
	bool carry = BIT(m_flags , FLAGS_CY_BIT);
	bool ovf = false;

	OP_ITERATION_START_FWD(i , multibyte) {
		OP1_GET(i, op1);

		uint8_t res;

		if (BIT(m_flags , FLAGS_DCM_BIT)) {
			res = (op1 << 4) | m_reg_E;
			m_reg_E = (op1 >> 4) & E_MASK;
			carry = false;
		} else {
			uint16_t tmp = (uint16_t)op1 << 1;
			res = (uint8_t)tmp + carry;
			ovf = BIT(res ^ op1 , 7);
			carry = BIT(tmp , 8);
		}
		RES_SET(i , res);
		OP_ITERATION_END_FWD(res);
	}
	COPY_BIT(carry , m_flags , FLAGS_CY_BIT);
	COPY_BIT(ovf , m_flags , FLAGS_OVF_BIT);
}

void capricorn_cpu_device::do_LL_op(bool multibyte)
{
	if (BIT(m_flags , FLAGS_DCM_BIT)) {
		m_reg_E = 0;
	} else {
		BIT_CLR(m_flags , FLAGS_CY_BIT);
	}
	do_EL_op(multibyte);
}

void capricorn_cpu_device::do_ER_op(bool multibyte)
{
	m_icount -= 4;
	bool carry = BIT(m_flags , FLAGS_CY_BIT);
	BIT_CLR(m_flags, FLAGS_OVF_BIT);

	OP_ITERATION_START_REV(i , multibyte) {
		OP1_GET(i, op1);

		uint8_t res;

		if (BIT(m_flags , FLAGS_DCM_BIT)) {
			res = (op1 >> 4) | (m_reg_E << 4);
			m_reg_E = op1 & E_MASK;
			carry = false;
		} else {
			res = op1 >> 1;
			if (carry) {
				BIT_SET(res, 7);
			}
			carry = BIT(op1 , 0);
		}
		RES_SET(i , res);
		OP_ITERATION_END_REV(res);
	}
	COPY_BIT(carry , m_flags , FLAGS_CY_BIT);
}

void capricorn_cpu_device::do_LR_op(bool multibyte)
{
	if (BIT(m_flags , FLAGS_DCM_BIT)) {
		m_reg_E = 0;
	} else {
		BIT_CLR(m_flags , FLAGS_CY_BIT);
	}
	do_ER_op(multibyte);
}

void capricorn_cpu_device::do_SAD_op()
{
	m_icount -= 8;
	uint16_t tmp = read_u16(REG_SP | GP_REG_MASK);
	ea_addr_t ea = tmp;
	write_u16(REG_SP | GP_REG_MASK, tmp + 3);

	start_mem_burst(ea);

	uint8_t byte = m_arp;
	if (BIT(m_flags , FLAGS_CY_BIT)) {
		BIT_SET(byte, 6);
	}
	if (BIT(m_flags , FLAGS_OVF_BIT)) {
		BIT_SET(byte, 7);
	}
	WM(ea , byte);

	byte = m_drp;
	if (BIT(m_flags , FLAGS_DCM_BIT)) {
		BIT_SET(byte, 6);
	}
	if (BIT(m_flags , FLAGS_OVF_BIT)) {
		BIT_SET(byte, 7);
	}
	WM(ea , byte);

	byte = 0;
	if (BIT(m_flags , FLAGS_LSB_BIT)) {
		BIT_SET(byte, 0);
	}
	if (!BIT(m_flags , FLAGS_RDZ_BIT)) {
		BIT_SET(byte, 1);
	}
	if (!BIT(m_flags , FLAGS_Z_BIT)) {
		BIT_SET(byte, 2);
	}
	if (!BIT(m_flags , FLAGS_LDZ_BIT)) {
		BIT_SET(byte, 6);
	}
	if (BIT(m_flags , FLAGS_MSB_BIT)) {
		BIT_SET(byte, 7);
	}
	WM(ea , byte);
}

void capricorn_cpu_device::do_PAD_op()
{
	m_icount -= 8;
	uint16_t tmp = read_u16(REG_SP | GP_REG_MASK) - 3;
	ea_addr_t ea = tmp;
	write_u16(REG_SP | GP_REG_MASK, tmp);

	uint8_t byte;
	start_mem_burst(ea);

	byte = RM(ea);
	m_arp = byte & ARP_DRP_MASK;
	COPY_BIT(BIT(byte , 6), m_flags, FLAGS_CY_BIT);
	COPY_BIT(BIT(byte , 7), m_flags, FLAGS_OVF_BIT);

	byte = RM(ea);
	m_drp = byte & ARP_DRP_MASK;
	COPY_BIT(BIT(byte , 6), m_flags, FLAGS_DCM_BIT);

	byte = RM(ea);
	COPY_BIT(BIT(byte , 0), m_flags, FLAGS_LSB_BIT);
	COPY_BIT(!BIT(byte , 1), m_flags, FLAGS_RDZ_BIT);
	COPY_BIT(!BIT(byte , 2), m_flags, FLAGS_Z_BIT);
	COPY_BIT(!BIT(byte , 6), m_flags, FLAGS_LDZ_BIT);
	COPY_BIT(BIT(byte , 7), m_flags, FLAGS_MSB_BIT);
}

void capricorn_cpu_device::do_RTN_op()
{
	m_icount -= 5;
	uint16_t tmp = read_u16(REG_SP | GP_REG_MASK) - 2;
	write_u16(REG_PC | GP_REG_MASK, read_u16(tmp) - 1);
	write_u16(REG_SP | GP_REG_MASK, tmp);
}

void capricorn_cpu_device::push_pc()
{
	uint16_t tmp = read_u16(REG_SP | GP_REG_MASK);
	write_u16(tmp, m_genpc);
	write_u16(REG_SP | GP_REG_MASK, tmp + 2);
}

void capricorn_cpu_device::do_JSB_op(ea_addr_t ea)
{
	m_icount -= 8;
	offset_pc(1);
	push_pc();
	write_u16(REG_PC | GP_REG_MASK, (uint16_t)(ea - 1));
}

void capricorn_cpu_device::do_PU_op(bool multibyte , bool direct , bool increment)
{
	m_icount -= 5;
	BIT_CLR(m_flags , FLAGS_CY_BIT);
	BIT_CLR(m_flags , FLAGS_OVF_BIT);

	uint16_t ar = read_u16(m_arp | GP_REG_MASK);
	ea_addr_t ea;
	if (direct) {
		unsigned n_regs = multibyte ? (get_upper_boundary() - m_drp + 1) : 1;
		if (increment) {
			ea = ar;
			ar += n_regs;
		} else {
			ar -= n_regs;
			ea = ar;
		}
	} else {
		m_icount -= 2;
		if (increment) {
			ea = read_u16(ar);
			ar += 2;
		} else {
			ar -= 2;
			ea = read_u16(ar);
		}
	}
	write_u16(m_arp | GP_REG_MASK, ar);

	start_mem_burst(ea);

	OP_ITERATION_START_FWD(i , multibyte) {
		OP1_GET(i, op1);

		WM(ea , op1);
		OP_ITERATION_END_FWD(op1);
	}
}

void capricorn_cpu_device::do_PO_op(bool multibyte , bool direct , bool increment)
{
	m_icount -= 5;
	BIT_CLR(m_flags , FLAGS_CY_BIT);
	BIT_CLR(m_flags , FLAGS_OVF_BIT);

	uint16_t ar = read_u16(m_arp | GP_REG_MASK);
	ea_addr_t ea;
	if (direct) {
		unsigned n_regs = multibyte ? (get_upper_boundary() - m_drp + 1) : 1;
		if (increment) {
			ea = ar;
			ar += n_regs;
		} else {
			ar -= n_regs;
			ea = ar;
		}
	} else {
		m_icount -= 2;
		if (increment) {
			ea = read_u16(ar);
			ar += 2;
		} else {
			ar -= 2;
			ea = read_u16(ar);
		}
	}
	write_u16(m_arp | GP_REG_MASK, ar);

	start_mem_burst(ea);

	OP_ITERATION_START_FWD(i , multibyte) {
		OP1_GET(i, op1);

		(void)op1;
		uint8_t res = RM(ea);
		RES_SET(i, res);
		OP_ITERATION_END_FWD(res);
	}
}

void capricorn_cpu_device::execute_one(uint8_t opcode)
{
	// DRP & ARP instructions
	if ((opcode & 0xc0) == 0x00) {
		// ARP
		if (opcode == 0x01) {
			m_icount -= 3;
			m_arp = m_reg[ REG_BANK_PTR ] & ARP_DRP_MASK;
		} else {
			m_icount -= 2;
			m_arp = opcode & ARP_DRP_MASK;
		}
	} else if ((opcode & 0xc0) == 0x40) {
		// DRP
		if (opcode == 0x41) {
			m_icount -= 3;
			m_drp = m_reg[ REG_BANK_PTR ] & ARP_DRP_MASK;
		} else {
			m_icount -= 2;
			m_drp = opcode & ARP_DRP_MASK;
		}
	} else {
		// Opcodes without mask
		switch (opcode) {
		case 0x98:
			// BIN
			m_icount -= 4;
			BIT_CLR(m_flags , FLAGS_DCM_BIT);
			break;

		case 0x99:
			// BCD
			m_icount -= 4;
			BIT_SET(m_flags , FLAGS_DCM_BIT);
			break;

		case 0x9a:
			// SAD
			do_SAD_op();
			break;

		case 0x9b:
			// DCE
			m_icount -= 2;
			m_reg_E = (m_reg_E - 1) & E_MASK;
			break;

		case 0x9c:
			// ICE
			m_icount -= 2;
			m_reg_E = (m_reg_E + 1) & E_MASK;
			break;

		case 0x9d:
			// CLE
			m_icount -= 2;
			m_reg_E = 0;
			break;

		case 0x9e:
			// RTN
			do_RTN_op();
			break;

		case 0x9f:
			// PAD
			do_PAD_op();
			break;

		case 0xc6:
			// JSB
			do_JSB_op(get_ea_idx_dir());
			break;

		case 0xc7:
			// ANM
			do_AN_op(get_ea_reg_imm());
			break;

		case 0xce:
			// JSB
			do_JSB_op(get_ea_lit_dir());
			break;

		case 0xcf:
			// ANM
			do_AN_op(get_ea_lit_imm(true));
			break;

		case 0xd7:
			// ANM
			do_AN_op(get_ea_lit_dir());
			break;

		case 0xdf:
			// ANM
			do_AN_op(get_ea_reg_dir());
			break;

		case 0xf0:
			// JMP
			do_jump(true);
			break;

		case 0xf1:
			// JNO
			do_jump(!BIT(m_flags , FLAGS_OVF_BIT));
			break;

		case 0xf2:
			// JOD
			do_jump(BIT(m_flags , FLAGS_LSB_BIT));
			break;

		case 0xf3:
			// JEV
			do_jump(!BIT(m_flags , FLAGS_LSB_BIT));
			break;

		case 0xf4:
			// JNG
			do_jump(BIT(m_flags , FLAGS_OVF_BIT) != BIT(m_flags , FLAGS_MSB_BIT));
			break;

		case 0xf5:
			// JPS
			do_jump(BIT(m_flags , FLAGS_OVF_BIT) == BIT(m_flags , FLAGS_MSB_BIT));
			break;

		case 0xf6:
			// JNZ
			do_jump(!BIT(m_flags , FLAGS_Z_BIT));
			break;

		case 0xf7:
			// JZR
			do_jump(BIT(m_flags , FLAGS_Z_BIT));
			break;

		case 0xf8:
			// JEN
			do_jump(m_reg_E != 0);
			break;

		case 0xf9:
			// JEZ
			do_jump(m_reg_E == 0);
			break;

		case 0xfa:
			// JNC
			do_jump(!BIT(m_flags , FLAGS_CY_BIT));
			break;

		case 0xfb:
			// JCY
			do_jump(BIT(m_flags , FLAGS_CY_BIT));
			break;

		case 0xfc:
			// JLZ
			do_jump(BIT(m_flags , FLAGS_LDZ_BIT));
			break;

		case 0xfd:
			// JLN
			do_jump(!BIT(m_flags , FLAGS_LDZ_BIT));
			break;

		case 0xfe:
			// JRZ
			do_jump(BIT(m_flags , FLAGS_RDZ_BIT));
			break;

		case 0xff:
			// JRN
			do_jump(!BIT(m_flags , FLAGS_RDZ_BIT));
			break;

		default:
			// Opcodes with 0xfe mask (M/B bit is in b0)
			bool multibyte = BIT(opcode , 0);
			switch (opcode & 0xfe) {
			case 0x80:
				// EL
				do_EL_op(multibyte);
				break;

			case 0x82:
				// ER
				do_ER_op(multibyte);
				break;

			case 0x84:
				// LL
				do_LL_op(multibyte);
				break;

			case 0x86:
				// LR
				do_LR_op(multibyte);
				break;

			case 0x88:
				// IC
				do_IC_op(multibyte);
				break;

			case 0x8a:
				// DC
				do_DC_op(multibyte);
				break;

			case 0x8c:
				// TC
				do_TC_op(multibyte);
				break;

			case 0x8e:
				// NC
				do_NC_op(multibyte);
				break;

			case 0x90:
				// TS
				do_TS_op(multibyte);
				break;

			case 0x92:
				// CL
				do_CL_op(multibyte);
				break;

			case 0x94:
				// OR
				do_OR_op(multibyte);
				break;

			case 0x96:
				// XR
				do_XR_op(multibyte);
				break;

			case 0xa0:
				// LD
				do_LD_op(get_ea_reg_imm() , multibyte);
				break;

			case 0xa2:
				// ST
				do_ST_op(get_ea_reg_imm() , multibyte);
				break;

			case 0xa4:
				// LD
				do_LD_op(get_ea_reg_dir() , multibyte);
				break;

			case 0xa6:
				// ST
				do_ST_op(get_ea_reg_dir() , multibyte);
				break;

			case 0xa8:
				// LD
				do_LD_op(get_ea_lit_imm(multibyte) , multibyte);
				break;

			case 0xaa:
				// ST
				do_ST_op(get_ea_lit_imm(multibyte) , multibyte);
				break;

			case 0xac:
				// LD
				do_LD_op(get_ea_reg_indir() , multibyte);
				break;

			case 0xae:
				// ST
				do_ST_op(get_ea_reg_indir() , multibyte);
				break;

			case 0xb0:
				// LD
				do_LD_op(get_ea_lit_dir() , multibyte);
				break;

			case 0xb2:
				// ST
				do_ST_op(get_ea_lit_dir() , multibyte);
				break;

			case 0xb4:
				// LD
				do_LD_op(get_ea_idx_dir() , multibyte);
				break;

			case 0xb6:
				// ST
				do_ST_op(get_ea_idx_dir() , multibyte);
				break;

			case 0xb8:
				// LD
				do_LD_op(get_ea_lit_indir() , multibyte);
				break;

			case 0xba:
				// ST
				do_ST_op(get_ea_lit_indir() , multibyte);
				break;

			case 0xbc:
				// LD
				do_LD_op(get_ea_idx_indir() , multibyte);
				break;

			case 0xbe:
				// ST
				do_ST_op(get_ea_idx_indir() , multibyte);
				break;

			case 0xc0:
				// CM
				do_CM_op(get_ea_reg_imm() , multibyte);
				break;

			case 0xc2:
				// AD
				do_AD_op(get_ea_reg_imm() , multibyte);
				break;

			case 0xc4:
				// SB
				do_SB_op(get_ea_reg_imm() , multibyte);
				break;

			case 0xc8:
				// CM
				do_CM_op(get_ea_lit_imm(multibyte) , multibyte);
				break;

			case 0xca:
				// AD
				do_AD_op(get_ea_lit_imm(multibyte) , multibyte);
				break;

			case 0xcc:
				// SB
				do_SB_op(get_ea_lit_imm(multibyte) , multibyte);
				break;

			case 0xd0:
				// CM
				do_CM_op(get_ea_lit_dir() , multibyte);
				break;

			case 0xd2:
				// AD
				do_AD_op(get_ea_lit_dir() , multibyte);
				break;

			case 0xd4:
				// SB
				do_SB_op(get_ea_lit_dir() , multibyte);
				break;

			case 0xd8:
				// CM
				do_CM_op(get_ea_reg_dir() , multibyte);
				break;

			case 0xda:
				// AD
				do_AD_op(get_ea_reg_dir() , multibyte);
				break;

			case 0xdc:
				// SB
				do_SB_op(get_ea_reg_dir() , multibyte);
				break;

			default:
				// Opcodes with 0xfc mask
				switch (opcode & 0xfc) {
				case 0xe0:
					// PO
					do_PO_op(multibyte, true, !BIT(opcode , 1));
					break;

				case 0xe4:
					// PU
					do_PU_op(multibyte, true, !BIT(opcode , 1));
					break;

				case 0xe8:
					// PO
					do_PO_op(multibyte, false, !BIT(opcode , 1));
					break;

				case 0xec:
					// PU
					do_PU_op(multibyte, false, !BIT(opcode , 1));
					break;

				default:
					logerror("Unknown opcode %02x\n" , opcode);
					break;
				}
				break;
			}
			break;
		}
	}
}

void capricorn_cpu_device::take_interrupt()
{
	// According to O. De Smet emulator interrupt handling takes 15 cycles
	m_icount -= 15;
	push_pc();
	uint8_t vector = (uint8_t)standard_irq_callback(0);
	vector_to_pc(vector);
}
