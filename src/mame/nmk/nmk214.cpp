// license:BSD-3-Clause
// copyright-holders:Sergio Galiano
/*
    NMK214 GFX Descrambler emulation

    This device is used for descrambling the GFX data on some game PCBs from NMK (nmk16).
    It works in tandem with NMK215, that's a Toshiba MCU which sends initialization data to NMK214 in order to do the
    descrambling process.
    Every game PCB using it has two NMK214 chips, one for sprites and another for background tiles.
    It can work in two different modes: word and byte:
    For sprites it always works in word mode; for backgrounds it always works in byte mode.
    There are 8 hard-wired internal configurations.  The data received from NMK215 selects one of those them at startup.
    That init data is stored in the device when bit 3 matches with the operation mode wired directly on the PCB.
    The descrambling process is essentially a dynamic bitswap of the incoming word/byte data, doing a different bitswap
    based on the address of the data to be descrambled.
    The input address bus on the device is used to determine which bitswap do for each word/byte, and it's usually
    hooked differently for sprites and background tiles, so an 'input_address_bitswap' is included to get the effective
    address the device will use.
*/


#include "emu.h"
#include "nmk214.h"


namespace {

const std::array<u8, 13> default_input_address_bitswap = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

// Static byte values for each of the 8 different hardwired configurations.
// One bit will be taken from each byte in the selected config (row), and those
// 3 bits will be used to select the output bitswap down below:
const u8 init_configs[8][3] =
{
	{0xaa, 0xcc, 0xf0},  // Predefined config 0
	{0x55, 0x39, 0x1e},  // Predefined config 1
	{0xc5, 0x69, 0x5c},  // Predefined config 2
	{0x35, 0x5c, 0xc5},  // Predefined config 3
	{0x78, 0x1d, 0x2e},  // Predefined config 4
	{0x55, 0x33, 0x0f},  // Predefined config 5
	{0xa5, 0xb8, 0x36},  // Predefined config 6
	{0x8b, 0x69, 0x2e}   // Predefined config 7
};

// 3 values for each configuration to determine which lines from input address
// bus are used to select a bit from hardwired config byte values above:
const u8 selection_address_bits[8][3] =
{
	{0x8, 0x9, 0xa},  // A8, A9 and A10 for predefined config 0
	{0x6, 0x8, 0xb},  // A6, A8 and A11 for predefined config 1
	{0x3, 0x9, 0xc},  // A3, A9 and A12 for predefined config 2
	{0x3, 0x7, 0xa},  // A3, A7 and A10 for predefined config 3
	{0x2, 0x5, 0xb},  // A2, A5 and A11 for predefined config 4
	{0x1, 0x4, 0xa},  // A1, A4 and A10 for predefined config 5
	{0x2, 0x4, 0xa},  // A2, A4 and A10 for predefined config 6
	{0x0, 0x4, 0xc}   // A0, A4 and A12 for predefined config 7
};

// Output word data bitswaps hardwired inside the chip.
// Each value in the same column represents which bit from input data word is
// used for current bit position in the output word.
const u8 output_word_bitswaps[8][16] =
{
//   D0   D1   D2   D3   D4   D5   D6   D7   D8   D9   D10  D11  D12  D13  D14  D15
	{0x2, 0x3, 0x7, 0x8, 0xc, 0x4, 0xb, 0x9, 0x1, 0xf, 0xa, 0x5, 0xe, 0x6, 0xd, 0x0},  // word bitswap 0
	{0x0, 0x3, 0x8, 0x7, 0xa, 0xc, 0x4, 0x1, 0xf, 0x9, 0x6, 0xd, 0xe, 0xb, 0x5, 0x2},  // word bitswap 1
	{0x9, 0x8, 0x2, 0x3, 0x6, 0x5, 0xd, 0xf, 0x7, 0x0, 0xc, 0xb, 0xa, 0x4, 0xe, 0x1},  // word bitswap 2
	{0x0, 0x3, 0x9, 0xf, 0xd, 0xc, 0xb, 0x1, 0x2, 0x7, 0xe, 0x6, 0x4, 0xa, 0x5, 0x8},  // word bitswap 3
	{0x1, 0x3, 0xf, 0x7, 0xd, 0xa, 0xe, 0x9, 0x0, 0x8, 0xc, 0x4, 0x6, 0x5, 0xb, 0x2},  // word bitswap 4
	{0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf},  // word bitswap 5
	{0x0, 0xf, 0x3, 0x2, 0xe, 0x4, 0x6, 0x7, 0x8, 0x9, 0x5, 0xd, 0xc, 0xb, 0xa, 0x1},  // word bitswap 6
	{0xf, 0x2, 0x3, 0x1, 0xb, 0xe, 0xd, 0x8, 0x7, 0x0, 0x4, 0xc, 0x6, 0xa, 0x5, 0x9}   // word bitswap 7
};

// Output byte data bitswaps hardwired inside the chip.
// Values on the table are calculated from the above word-bitswaps based on the
// following input data lines correlation (that's how the data lines are hooked
// up in real hardware):
/*
   BYTE mode | WORD mode
   ----------|----------
   D0        | D13
   D1        | D10
   D2        | D4
   D3        | D12
   D4        | D6
   D5        | D14
   D6        | D11
   D7        | D5
*/
const u8 output_byte_bitswaps[8][8] =
{
//   D0   D1   D2   D3   D4   D5   D6   D7
	{0x4, 0x1, 0x3, 0x5, 0x6, 0x0, 0x7, 0x2},  // byte bitswap 0
	{0x6, 0x4, 0x1, 0x5, 0x2, 0x7, 0x0, 0x3},  // byte bitswap 1
	{0x2, 0x3, 0x4, 0x1, 0x0, 0x5, 0x6, 0x7},  // byte bitswap 2
	{0x1, 0x5, 0x0, 0x2, 0x6, 0x7, 0x4, 0x3},  // byte bitswap 3
	{0x7, 0x3, 0x0, 0x4, 0x5, 0x6, 0x2, 0x1},  // byte bitswap 4
	{0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7},  // byte bitswap 5
	{0x6, 0x7, 0x5, 0x3, 0x4, 0x1, 0x0, 0x2},  // byte bitswap 6
	{0x1, 0x2, 0x6, 0x4, 0x0, 0x7, 0x3, 0x5}   // byte bitswap 7
};

} // anonymous namespace


DEFINE_DEVICE_TYPE(NMK214, nmk214_device, "nmk214", "NMK214 Graphics Descrambler")

nmk214_device::nmk214_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NMK214, tag, owner, clock)
	, m_mode(0)
	, m_input_address_bitswap(default_input_address_bitswap)
	, m_init_config(0)
	, m_device_initialized(false)
{
}

u8 nmk214_device::get_bitswap_select_value(u32 addr) const noexcept
{
	// as the address lines could be hooked up in different order/positions on
	// the game PCB, reorder it and get the effective address to use:
	const u32 effective_address = bitswap<13>(addr,
			m_input_address_bitswap[12],
			m_input_address_bitswap[11],
			m_input_address_bitswap[10],
			m_input_address_bitswap[9],
			m_input_address_bitswap[8],
			m_input_address_bitswap[7],
			m_input_address_bitswap[6],
			m_input_address_bitswap[5],
			m_input_address_bitswap[4],
			m_input_address_bitswap[3],
			m_input_address_bitswap[2],
			m_input_address_bitswap[1],
			m_input_address_bitswap[0]);

	// get selection address using only 3 bits of the effective address, based
	// on the initial config selected while device initialization:
	const u8 selection_address = bitswap<3>(effective_address,
			selection_address_bits[m_init_config][2],
			selection_address_bits[m_init_config][1],
			selection_address_bits[m_init_config][0]);

	// get the bitswap selection value, using the previously computed selection
	// address and the selected internal config values:
	return (BIT(init_configs[m_init_config][0], selection_address) << 0)
			| (BIT(init_configs[m_init_config][1], selection_address) << 1)
			| (BIT(init_configs[m_init_config][2], selection_address) << 2);
}

u16 nmk214_device::decode_word(u32 addr, u16 data) const noexcept
{
	// compute the select value to choose which bitswap apply to the input word,
	// based on the address where the data is located
	const u8 bitswap_select = get_bitswap_select_value(addr);

	return bitswap<16>(data,
			output_word_bitswaps[bitswap_select][15],
			output_word_bitswaps[bitswap_select][14],
			output_word_bitswaps[bitswap_select][13],
			output_word_bitswaps[bitswap_select][12],
			output_word_bitswaps[bitswap_select][11],
			output_word_bitswaps[bitswap_select][10],
			output_word_bitswaps[bitswap_select][9],
			output_word_bitswaps[bitswap_select][8],
			output_word_bitswaps[bitswap_select][7],
			output_word_bitswaps[bitswap_select][6],
			output_word_bitswaps[bitswap_select][5],
			output_word_bitswaps[bitswap_select][4],
			output_word_bitswaps[bitswap_select][3],
			output_word_bitswaps[bitswap_select][2],
			output_word_bitswaps[bitswap_select][1],
			output_word_bitswaps[bitswap_select][0]);
}

u8 nmk214_device::decode_byte(u32 addr, u8 data) const noexcept
{
	// compute the select value to choose which bitswap apply to the input byte,
	// based on the address where the data is located
	u8 bitswap_select = get_bitswap_select_value(addr);

	return bitswap<8>(data,
			output_byte_bitswaps[bitswap_select][7],
			output_byte_bitswaps[bitswap_select][6],
			output_byte_bitswaps[bitswap_select][5],
			output_byte_bitswaps[bitswap_select][4],
			output_byte_bitswaps[bitswap_select][3],
			output_byte_bitswaps[bitswap_select][2],
			output_byte_bitswaps[bitswap_select][1],
			output_byte_bitswaps[bitswap_select][0]);
}

void nmk214_device::set_init_config(u8 init_config) noexcept
{
	// store config data only if Bit 3 matches with the operation mode of the device
	if (BIT(init_config, 3) == m_mode)
	{
		m_init_config = init_config & 0x7;
		m_device_initialized = true;
	}
}

void nmk214_device::device_start()
{
	save_item(NAME(m_init_config));
	save_item(NAME(m_device_initialized));
}

void nmk214_device::device_reset()
{
	m_init_config = 0;
	m_device_initialized = false;
}
