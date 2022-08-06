// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS1000, TMS1070, TMS1040, TMS1200, TMS1700, TMS1730,
  and second source Motorola MC141000, MC141200.

  TODO:
  - add TMS1270 (10 O pins, how does that work?)

*/

#include "emu.h"
#include "tms1000.h"
#include "tms1k_dasm.h"

// TMS1000
// - 64x4bit RAM array at the bottom-left
// - 1024x8bit ROM array at the bottom-right
//   * FYI, the row-selector to the left of it is laid out as:
//     3,4,11,12,19,20,27,28,35,36,43,44,51,52,59,60,0,7,8,15,16,23,24,31,32,39,40,47,48,55,56,63,
//     2,5,10,13,18,21,26,29,34,37,42,45,50,53,58,61,1,6,9,14,17,22,25,30,33,38,41,46,49,54,57,62
// - 30-term microinstructions PLA(mpla) at the top half, to the right of the midline, supporting 16 microinstructions
// - 20-term output PLA(opla) at the top-left
// - the ALU is between the opla and mpla
DEFINE_DEVICE_TYPE(TMS1000,  tms1000_cpu_device,  "tms1000",  "Texas Instruments TMS1000") // 28-pin DIP, 11 R pins
DEFINE_DEVICE_TYPE(TMS1070,  tms1070_cpu_device,  "tms1070",  "Texas Instruments TMS1070") // high voltage version
DEFINE_DEVICE_TYPE(TMS1040,  tms1040_cpu_device,  "tms1040",  "Texas Instruments TMS1040") // same as TMS1070 with just a different pinout?
DEFINE_DEVICE_TYPE(TMS1200,  tms1200_cpu_device,  "tms1200",  "Texas Instruments TMS1200") // 40-pin DIP, 13 R pins
DEFINE_DEVICE_TYPE(TMS1700,  tms1700_cpu_device,  "tms1700",  "Texas Instruments TMS1700") // 28-pin DIP, RAM/ROM size halved, 9 R pins
DEFINE_DEVICE_TYPE(TMS1730,  tms1730_cpu_device,  "tms1730",  "Texas Instruments TMS1730") // 20-pin DIP, same die as TMS1700, package has less pins: 6 R pins, 5 O pins (output PLA is still 8-bit, O1,O3,O5 unused)

// 2nd source Motorola chips
DEFINE_DEVICE_TYPE(MC141000, mc141000_cpu_device, "mc141000", "Motorola MC141000") // CMOS, pin-compatible with TMS1000(reverse polarity)
DEFINE_DEVICE_TYPE(MC141200, mc141200_cpu_device, "mc141200", "Motorola MC141200") // CMOS, 40-pin DIP, 16 R pins


// internal memory maps
void tms1000_cpu_device::rom_9bitm(address_map &map)
{
	map(0x000, 0x1ff).mirror(0x200).rom();
}

void tms1000_cpu_device::ram_32x4(address_map &map)
{
	map(0x00, 0x3f).ram();
	map(0x08, 0x0f).mirror(0x30).noprw(); // override
}


// device definitions
tms1000_cpu_device::tms1000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms1000_cpu_device(mconfig, TMS1000, tag, owner, clock, 8 /* o pins */, 11 /* r pins */, 6 /* pc bits */, 8 /* byte width */, 2 /* x width */, 1 /* stack levels */, 10 /* rom width */, address_map_constructor(FUNC(tms1000_cpu_device::rom_10bit), this), 6 /* ram width */, address_map_constructor(FUNC(tms1000_cpu_device::ram_6bit), this))
{ }

tms1000_cpu_device::tms1000_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map) :
	tms1k_base_device(mconfig, type, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, stack_levels, rom_width, rom_map, ram_width, ram_map)
{ }

tms1070_cpu_device::tms1070_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms1000_cpu_device(mconfig, TMS1070, tag, owner, clock, 8, 11, 6, 8, 2, 1, 10, address_map_constructor(FUNC(tms1070_cpu_device::rom_10bit), this), 6, address_map_constructor(FUNC(tms1070_cpu_device::ram_6bit), this))
{ }

tms1040_cpu_device::tms1040_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms1000_cpu_device(mconfig, TMS1040, tag, owner, clock, 8, 11, 6, 8, 2, 1, 10, address_map_constructor(FUNC(tms1040_cpu_device::rom_10bit), this), 6, address_map_constructor(FUNC(tms1040_cpu_device::ram_6bit), this))
{ }

tms1200_cpu_device::tms1200_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms1000_cpu_device(mconfig, TMS1200, tag, owner, clock, 8, 13, 6, 8, 2, 1, 10, address_map_constructor(FUNC(tms1200_cpu_device::rom_10bit), this), 6, address_map_constructor(FUNC(tms1200_cpu_device::ram_6bit), this))
{ }

tms1700_cpu_device::tms1700_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms1000_cpu_device(mconfig, TMS1700, tag, owner, clock, 8, 9, 6, 8, 2, 1, 10, address_map_constructor(FUNC(tms1700_cpu_device::rom_9bitm), this), 6, address_map_constructor(FUNC(tms1700_cpu_device::ram_32x4), this))
{ }

tms1730_cpu_device::tms1730_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms1000_cpu_device(mconfig, TMS1730, tag, owner, clock, 8, 9, 6, 8, 2, 1, 10, address_map_constructor(FUNC(tms1730_cpu_device::rom_9bitm), this), 6, address_map_constructor(FUNC(tms1730_cpu_device::ram_32x4), this))
{ }

mc141000_cpu_device::mc141000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms1000_cpu_device(mconfig, MC141000, tag, owner, clock, 8, 11, 6, 8, 2, 1, 10, address_map_constructor(FUNC(mc141000_cpu_device::rom_10bit), this), 6, address_map_constructor(FUNC(mc141000_cpu_device::ram_6bit), this))
{ }

mc141200_cpu_device::mc141200_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms1000_cpu_device(mconfig, MC141200, tag, owner, clock, 8, 16, 6, 8, 2, 1, 10, address_map_constructor(FUNC(mc141200_cpu_device::rom_10bit), this), 6, address_map_constructor(FUNC(mc141200_cpu_device::ram_6bit), this))
{ }


// machine configs
void tms1000_cpu_device::device_add_mconfig(machine_config &config)
{
	// microinstructions PLA, output PLA
	PLA(config, "mpla", 8, 16, 30).set_format(pla_device::FMT::BERKELEY);
	PLA(config, "opla", 5, 8, 20).set_format(pla_device::FMT::BERKELEY);
}


// disasm
std::unique_ptr<util::disasm_interface> tms1000_cpu_device::create_disassembler()
{
	return std::make_unique<tms1000_disassembler>();
}


// device_reset
u32 tms1000_cpu_device::decode_micro(u8 sel)
{
	//                                           _____              _____  ______  _____  ______  _____  _____  _____  _____
	const u32 md[16] = { M_STSL, M_AUTY, M_AUTA, M_CIN, M_C8, M_NE, M_CKN, M_15TN, M_MTN, M_NATN, M_ATN, M_MTP, M_YTP, M_CKP, M_CKM, M_STO };
	u16 mask = m_mpla->read(sel);
	mask ^= 0x3fc8; // invert active-negative
	u32 decode = 0;

	for (int bit = 0; bit < 16; bit++)
		if (mask & (1 << bit))
			decode |= md[bit];

	return decode;
}

void tms1000_cpu_device::device_reset()
{
	// common reset
	tms1k_base_device::device_reset();

	// pre-decode instructionset
	m_fixed_decode.resize(0x100);
	memset(&m_fixed_decode[0], 0, 0x100*sizeof(u32));
	m_micro_decode.resize(0x100);
	memset(&m_micro_decode[0], 0, 0x100*sizeof(u32));

	// decode microinstructions
	for (int op = 0; op < 0x100; op++)
		m_micro_decode[op] = m_decode_micro.isnull() ? decode_micro(op) : m_decode_micro(op);

	// the fixed instruction set is not programmable
	m_fixed_decode[0x00] = F_COMX;
	m_fixed_decode[0x0a] = F_TDO;
	m_fixed_decode[0x0b] = F_CLO;
	m_fixed_decode[0x0c] = F_RSTR;
	m_fixed_decode[0x0d] = F_SETR;
	m_fixed_decode[0x0f] = F_RETN;

	for (int i = 0x10; i < 0x20; i++) m_fixed_decode[i] = F_LDP;
	for (int i = 0x30; i < 0x34; i++) m_fixed_decode[i] = F_SBIT;
	for (int i = 0x34; i < 0x38; i++) m_fixed_decode[i] = F_RBIT;
	for (int i = 0x3c; i < 0x40; i++) m_fixed_decode[i] = F_LDX;

	for (int i = 0x80; i < 0xc0; i++) m_fixed_decode[i] = F_BR;
	for (int i = 0xc0; i < 0x100; i++) m_fixed_decode[i] = F_CALL;
}
