// license:BSD-3-Clause
// copyright-holders:ElSemi, Angelo Salese
/*
    CRYSTAL SYSTEM by Brezzasoft (2001)
    using VRender0 System on a Chip

    The VRender0 (info at archive.org for http://www.mesdigital.com) chip contains:
        - CPU Core SE3208 (info at www.adc.co.kr) @ 43Mhz
        - 2 DMA chans
        - 4 Timers
        - 32 PIO pins
        - PWM output
        - 32 channels wavetable synth (8bit linear, 16bit linear and 8bit ulaw sample format)
        - Custom 2D video rendering device (texture mapping, alphablend, roz)

    The SMART-IC protection PIC authenticates over a bit-banged serial link and returns
    encrypted address/data records. The main CPU decrypts these records and repairs
    deliberately damaged instructions after copying the program from flash to RAM.
    Patching flash alone leaves failed authentication results behind: Donghaer and
    Office Yeoin Cheonha contain timer handlers that then corrupt arbitrary work RAM.
    Dumped PICs are executed; undumped PICs use a simulation of their serial protocol.

    MAME driver by ElSemi
    Additional work and refactoring by Angelo Salese

    TODO:
    - Move flash memory implementation into machine/intelfsh.cpp
    - provide NVRAM defaults where applicable;
    - add an actual reset button (helps with inp record/playback);
    - wulybuly: inputs, not extensively tested;
    - maldaiza: verify alternate prize dispensers and mechanical timing against hardware;
    - protection HLE: verify timing, EEPROM defaults and Crystal of Kings nonce generation
      against original PICs when dumps or hardware traces become available;
    - urachamu: some animation timings seems off, like bat hit animation before starting a given game.
      They were actually too fast before adding 30 Hz vblank for interlace mode, even if the game don't
      really read crtc blanking reg or use any other interrupt but the coin ones;
    - urachamu: investigate what CDMA in test mode really do, assuming it's not a dud;

========================================================================================================

The Crystal of Kings
Brezza Soft Corporation (Japan), 2001

This game runs on a small cartridge-based PCB known as the 'Crystal System'
The main PCB is small (approx 6" square) and contains only a few components. All of the processing
work is done by the large IC in the middle of the PCB. The system looks a bit like IGS's PGM System, in
that it's housed in a plastic case and has a single slot for insertion of a game cart. However this
system and the game carts are approx. half the size of the PGM carts.
On bootup, the screen is black and the system outputs a vertical white line on the right side of the screen.
The HSync is approx 20kHz, the screen is out of sync on a standard 15kHz arcade monitor.
After approx. 15-20 seconds, the screen changes to white and has some vertical stripes on it and the HSync
changes to 15kHz. After a few more seconds the game boots to a white screen and a blue 'Brezza Soft' logo.
Without a cart plugged in, the screen stays at the first vertical line screen.

Main PCB Layout
---------------

Brezza Soft MAGIC EYES AMG0110B
  |----------------------------------------------------|
  |TDA1519   VOL     3.6V_BATT  SW1 SW2 SW3            |
|-|                                                    |
|                              DS1233                  |
|                                                |---| |
|                                       GM76256  |   | |
|        DA1133A                        |-----|  |   | |
|                HY57V651620  32.768kHz |MX27L|  |   | |
|                               DS1302  |1000 |  |   | |
|J                  |-------------|     |-----|  |   | |
|A                  |             |LED1          |   | |
|M                  |             |LED2          |CN1| |
|M      HY57V651620 | VRENDERZERO |              |   | |
|A                  |             |              |   | |
|                   |             |              |   | |
|                   |-------------| HY57V651620  |   | |
|                                                |   | |
|              14.31818MHz                       |   | |
|                              PAL               |---| |
|                                    TD62003           |
|-|                                                    |
  | |---------------| DSW(8)                           |
  |-|     CN2       |----------------------------------|
    |---------------|

Notes:
      GM76C256   : Hyundai GM76C256 32k x8 SRAM (SOP28)
      MX27L1000  : Macronix MX27L1000QC-12 128k x8 EEPROM (BIOS, PLCC32)
      PAL        : Atmel ATF16V8-10PC PAL (DIP20)
      HY57V651620: Hyundai HY57V651620 4M x16 SDRAM (SSOP54)
      DA1311A    : Philips DA1311A DAC (SOIC8)
      DS1233     : Dallas DS1233 master reset IC (SOIC4)
      DS1302     : Dallas DS1302 real time clock IC (DIP8)
      VRENDERZERO: MESGraphics VRenderZERO (all-in-one main CPU/graphics/sound, QFP240)
      SW1        : Push button reset switch
      SW2        : Push button service switch
      SW3        : Push button test switch
      TDA1519    : Philips TDA1519 dual 6W stereo power amplifier (SIP9)
      VOL        : Master volume potentiometer
      3.6V_BATT  : 3.6 Volt NiCad battery 65mAh (for RTC)
      TD62003    : Toshiba TD62003 PNP 50V 0.5A quad darlington switch, for driving coin meters (DIP16)
      CN1        : PCI-type slot for extension riser board (for game cart connection at 90 degrees to the main PCB)
      CN2        : IDC 34-way flat cable connector (purpose unknown)
      VSync      : 60Hz


Game cart PCB
-------------
Cart sticker: 'THE CRYSTAL OF KINGS      BCSV0000'
PCB printing: Brezza Soft MAGIC EYES AMG0111B
|------------------------------|
|                              |
|PIC16xxx?                     |
|           U8   U7   U6   U5  |
|3.57945Mhz                    |
|                              |
|                              |
|                              |
|                              |
|74HC138                       |
|           U4   U3   U2   U1  |
|                              |
|                              |
|                              |
|-|                          |-|
  |--------------------------|

Notes:
      The cart PCB is single sided and contains only...
      1x 3.579545MHz crystal
      1x 74HC138 logic chip
      1x 18 pin unknown chip (DIP18, surface scratched but it's probably a PIC16xxx, labelled 'dgSMART-PR3 MAGIC EYES')
      3x Intel E28F128J3A 128Mbit surface mounted Flash ROMs (TSOP56, labelled 'BREZZASOFT BCSV0004Fxx', xx=01, 02, 03)
         Note: there are 8 spaces total for Flash ROMs. Only U1, U2 & U3 are populated in this cart.


*/

#include "emu.h"

#include "cpu/pic16_mid/pic16_mid.h"
#include "cpu/se3208/se3208.h"
#include "machine/ds1302.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/vrender0.h"

#include "speaker.h"

#include "endianness.h"
#include "multibyte.h"

#include <algorithm>
#include <array>
#include <cassert>

#define LOG_PROTECTION (1U << 1)
#include "logmacro.h"


namespace {

// HLE for the undumped PICs, reconstructed from the matching SE3208 code
// and the Office Yeoin Cheonha/Top Blade V firmware. The SE3208 performs
// authentication, decrypts the returned records and writes its own RAM.
// This device has no access to the host address space.
//
// The Crystal of Kings uses an older command assignment and authentication
// transform. The later games use the DES round function and rolling serial
// XOR masks seen in Top Blade V. Their host code reads counters at EEPROM
// 0x7c/0x7d, despite the tentative PIC16F84A labels in the ROM definitions.
// EEPROM defaults, instruction-level delays and the older nonce table are
// unknown; this models the host-visible protocol, not an undumped firmware.
class crystal_hle_device : public device_t, public device_nvram_interface
{
public:
	struct configuration
	{
		bool legacy = false;
		u32 key_offset = 0;
		u32 mask_offset = 0;
		char identifier[20]{};
		std::array<u32, 8> records{};
	};

	crystal_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	void set_config(const configuration &config) { m_config = &config; }
	void set_flash_region(const finder_base &finder) { m_flash.set_tag(finder); }
	auto data_callback() { return m_data_cb.bind(); }
	void lines_w(int reset, int data);
	int data_r() const { return m_host_data && m_output; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_nvram_interface implementation
	virtual void nvram_default() override { m_eeprom.fill(0); }
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	// The host and PIC use DES with reversed bit numbering within each byte.
	// Tables are decoded from the cartridge's host protection module.
	using block = std::array<u8, 8>;
	using schedule = std::array<u8, 128>;
	using tables = std::array<u8, 0x298>;

	enum : u16
	{
		PC1 = 0x000,
		PC2 = 0x038,
		KEY_BYTES = 0x068,
		KEY_BITS = 0x098,
		IP = 0x0c8,
		FP = 0x108,
		EXPAND = 0x148,
		SBOX = 0x178,
		PERMUTE = 0x278
	};

	static const char *invalid_table(const tables &table);
	static schedule make_schedule(const block &key, const tables &table);
	static block transform(block data, const schedule &keys, const tables &table);
	static block permutation(const block &data, const u8 *table);
	static block encrypt(const block &data, const schedule &keys, const tables &table);
	static block simple_transform(block state, const block &data);

	enum : u8 { BOOT_TX, BOOT_RX, COMMAND, AUTH_A, AUTH_B, AUTH_REPLY, ECHO, EE_ADDRESS, EE_COUNT, EE_WRITE };

	bool old_protocol() const { return m_config->legacy; }
	u8 eeprom_mask() const { return old_protocol() ? 63 : 127; }
	void output_w(int state);
	void receive(u8 data);
	void send(u8 data);
	void expect(u8 state, u16 count);
	TIMER_CALLBACK_MEMBER(rx_tick);
	TIMER_CALLBACK_MEMBER(tx_tick);

	required_region_ptr<u32> m_flash;
	devcb_write_line m_data_cb;
	const configuration *m_config;
	block m_key, m_auth, m_session;
	schedule m_keys;
	tables m_tables;
	std::array<u8, 32> m_patch;
	std::array<u8, 128> m_eeprom;
	std::array<u8, 256> m_input;
	std::array<u8, 512> m_output_queue;
	u16 m_in_count, m_expected, m_out_head, m_out_count;
	u8 m_state, m_mask, m_status, m_rx_key, m_tx_key, m_address, m_command;
	bool m_reset, m_receiving;
	bool m_host_data, m_output; // released levels of the two shared-line drivers
	u8 m_rx_bit, m_rx_byte, m_rx_parity;
	s32 m_tx_bit;
	u8 m_tx_byte, m_tx_parity;
	emu_timer *m_rx_timer, *m_tx_timer;
};

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(CRYSTAL_HLE, crystal_hle_device, crystal_hle_device, "crystal_hle", "Crystal System protection (HLE)")

namespace {

const char *crystal_hle_device::invalid_table(const tables &table)
{
	static constexpr struct
	{
		u16 offset;
		u8 count;
		u8 limit;
		const char *name;
	} LIMITS[] = {
		{ PC1,       56, 56, "PC1" },
		{ PC2,       48, 56, "PC2" },
		{ KEY_BYTES, 48,  6, "key byte" },
		{ KEY_BITS,  48,  8, "key bit" },
		{ IP,        64, 64, "initial permutation" },
		{ FP,        64, 64, "final permutation" },
		{ EXPAND,    48, 32, "expansion" },
		{ PERMUTE,   32, 32, "round permutation" }
	};
	for (const auto &range : LIMITS)
	{
		if (!std::all_of(table.begin() + range.offset, table.begin() + range.offset + range.count,
			[limit = range.limit] (u8 value) { return value < limit; }))
			return range.name;
	}
	return nullptr;
}

crystal_hle_device::schedule crystal_hle_device::make_schedule(const block &key, const tables &table)
{
	const u64 raw = get_u64le(key.data()) & 0x00ff'ffff'ffff'ffff;
	u64 cd = 0;
	for (u32 i = 0; i < 56; ++i)
		cd |= u64(BIT(raw, table[PC1 + i])) << i;
	u32 c = cd & 0x0fff'ffff;
	u32 d = cd >> 28;
	schedule result{};
	for (u32 r = 0; r < 16; ++r)
	{
		const u32 shift = (r == 0 || r == 1 || r == 8 || r == 15) ? 1 : 2;
		c = ((c >> shift) | (c << (28 - shift))) & 0x0fff'ffff;
		d = ((d >> shift) | (d << (28 - shift))) & 0x0fff'ffff;
		cd = c | (u64(d) << 28);
		u64 packed = 0;
		for (u32 i = 0; i < 48; ++i)
			packed |= u64(BIT(cd, table[PC2 + i])) << i;
		for (u32 i = 0; i < 48; ++i)
			result[8 * r + i / 6] |= BIT(packed, 8 * table[KEY_BYTES + i] + table[KEY_BITS + i]) << (i % 6);
	}
	return result;
}

crystal_hle_device::block crystal_hle_device::transform(block data, const schedule &keys, const tables &table)
{
	for (u32 r = 0; r < 16; ++r)
	{
		const u32 src = (r & 1) ? 4 : 0;
		const u32 dst = src ^ 4;
		u8 groups[8]{};
		u8 sub[4]{};
		for (u32 g = 0; g < 8; ++g)
		{
			for (u32 i = 0; i < 6; ++i)
			{
				const u8 bit = table[EXPAND + 6 * g + i];
				groups[g] |= BIT(data[src + bit / 8], bit & 7) << i;
			}
			groups[g] ^= keys[8 * r + g];
		}
		for (u32 i = 0; i < 4; ++i)
			sub[i] = (table[SBOX + 64 * i + groups[2 * i]] & 0x0f) | (table[SBOX + 64 * i + groups[2 * i + 1]] & 0xf0);
		for (u32 g = 0; g < 4; ++g)
		{
			for (u32 i = 0; i < 8; ++i)
			{
				const u8 bit = table[PERMUTE + 8 * g + i];
				data[dst + g] ^= BIT(sub[bit / 8], bit & 7) << i;
			}
		}
	}
	return data;
}

crystal_hle_device::block crystal_hle_device::permutation(const block &data, const u8 *table)
{
	const u64 raw = get_u64le(data.data());
	u64 result = 0;
	for (u32 i = 0; i < 64; ++i)
		result |= u64(BIT(raw, table[i])) << i;
	block bytes;
	put_u64le(bytes.data(), result);
	return bytes;
}

crystal_hle_device::block crystal_hle_device::encrypt(const block &data, const schedule &keys, const tables &table)
{
	block initial = permutation(data, &table[IP]);
	std::swap_ranges(initial.begin(), initial.begin() + 4, initial.begin() + 4);
	return permutation(transform(initial, keys, table), &table[FP]);
}

crystal_hle_device::block crystal_hle_device::simple_transform(block state, const block &data)
{
	for (u32 i = 0; i < 8; ++i)
	{
		u8 v = u8(state[i] + data[i]) ^ data[i];
		v = (v >> 1) | (v << 7);
		state[i] = ~u8((v >> 4) | (v << 4));
	}
	return state;
}

crystal_hle_device::crystal_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, CRYSTAL_HLE, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	m_flash(*this, finder_base::DUMMY_TAG),
	m_data_cb(*this),
	m_config(nullptr)
{
}

void crystal_hle_device::device_start()
{
	if (!m_config)
		fatalerror("%s: no configuration supplied\n", tag());

	// Decode the key and DES tables from the cartridge's host protection module.
	// Table offsets are relative to its XOR mask, and include linear bit indices
	// in place of the PIC's equivalent byte/mask pairs.
	if (u64(m_config->key_offset) + m_key.size() > m_flash.bytes()
		|| u64(m_config->mask_offset) + 0x100 + m_tables.size() > m_flash.bytes()
		|| (m_config->mask_offset & 3))
		fatalerror("%s: invalid protection data location in cartridge ROM\n", tag());
	const u32 mask = m_flash[m_config->mask_offset / 4];
	const auto read_byte = [this, mask] (u32 offset) -> u8
	{
		return BIT(m_flash[offset / 4] ^ mask, 8 * (offset & 3), 8);
	};
	// The Crystal of Kings key is unaligned. Its older authentication uses all
	// eight bytes; the DES key schedule uses only the first seven.
	for (u32 i = 0; i < m_key.size(); ++i)
		m_key[i] = read_byte(m_config->key_offset + i);
	for (u32 i = 0; i < m_tables.size(); ++i)
		m_tables[i] = read_byte(m_config->mask_offset + 0x100 + i);
	if (const char *const invalid = invalid_table(m_tables))
		fatalerror("%s: invalid %s protection table in cartridge ROM\n", tag(), invalid);
	m_keys = make_schedule(m_key, m_tables);
	for (u32 i = 0; i < 4; ++i)
	{
		block record;
		put_u32le(&record[0], m_config->records[2 * i]);
		put_u32le(&record[4], m_config->records[2 * i + 1]);
		const auto encrypted = encrypt(record, m_keys, m_tables);
		std::copy(encrypted.begin(), encrypted.end(), &m_patch[8 * i]);
	}
	m_reset = false;
	m_host_data = m_output = true;
	m_eeprom.fill(0);
	m_input.fill(0);
	m_output_queue.fill(0);
	m_rx_timer = timer_alloc(FUNC(crystal_hle_device::rx_tick), this);
	m_tx_timer = timer_alloc(FUNC(crystal_hle_device::tx_tick), this);
	save_item(NAME(m_auth));
	save_item(NAME(m_session));
	save_item(NAME(m_eeprom));
	save_item(NAME(m_input));
	save_item(NAME(m_output_queue));
	save_item(NAME(m_in_count));
	save_item(NAME(m_expected));
	save_item(NAME(m_out_head));
	save_item(NAME(m_out_count));
	save_item(NAME(m_state));
	save_item(NAME(m_mask));
	save_item(NAME(m_status));
	save_item(NAME(m_rx_key));
	save_item(NAME(m_tx_key));
	save_item(NAME(m_address));
	save_item(NAME(m_command));
	save_item(NAME(m_reset));
	save_item(NAME(m_host_data));
	save_item(NAME(m_output));
	save_item(NAME(m_receiving));
	save_item(NAME(m_rx_bit));
	save_item(NAME(m_rx_byte));
	save_item(NAME(m_rx_parity));
	save_item(NAME(m_tx_bit));
	save_item(NAME(m_tx_byte));
	save_item(NAME(m_tx_parity));
}

void crystal_hle_device::device_reset()
{
	m_rx_timer->adjust(attotime::never);
	m_tx_timer->adjust(attotime::never);
	m_state = old_protocol() ? COMMAND : BOOT_TX;
	m_mask = 0x11;
	m_status = m_rx_key = m_tx_key = 0;
	m_in_count = m_expected = m_out_head = m_out_count = 0;
	m_receiving = false;
	output_w(1);
	m_address = m_command = 0;
	m_rx_bit = m_rx_byte = m_rx_parity = 0;
	m_tx_byte = m_tx_parity = 0;
	m_tx_bit = -1;
	m_auth.fill(0);
	m_session.fill(0);
}

bool crystal_hle_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = util::read(file, m_eeprom.data(), eeprom_mask() + 1);
	return !err && actual == eeprom_mask() + 1;
}

bool crystal_hle_device::nvram_write(util::write_stream &file)
{
	return !util::write(file, m_eeprom.data(), eeprom_mask() + 1).first;
}

void crystal_hle_device::expect(u8 state, u16 count)
{
	m_state = state;
	m_in_count = 0;
	m_expected = count;
}

void crystal_hle_device::output_w(int state)
{
	const int old_line = data_r();
	m_output = state;
	if (old_line != data_r())
		m_data_cb(data_r());
}

void crystal_hle_device::lines_w(int reset, int data)
{
	const int old_line = data_r();
	const bool reset_asserted = reset && !m_reset;
	m_reset = reset;
	m_host_data = data;
	if (reset_asserted)
		device_reset();
	// The host may start its reply during the final stop bit.  At that point
	// the PIC has released the shared line and is ready to receive again.
	const bool listening = !m_out_count || (m_out_count == 1 && m_tx_bit >= 11);
	if (!reset && !m_receiving && listening && old_line && !data_r())
	{
		m_receiving = true;
		m_rx_bit = m_rx_byte = m_rx_parity = 0;
		m_rx_timer->adjust(attotime::from_hz(19200)); // validate the start bit halfway through
	}
}

TIMER_CALLBACK_MEMBER(crystal_hle_device::rx_tick)
{
	const u8 line = data_r();
	if (!m_rx_bit)
	{
		// Both dumped firmwares reject pulses shorter than half a bit.
		if (line)
		{
			m_receiving = false;
			return;
		}
	}
	else if (m_rx_bit <= 8)
	{
		m_rx_byte |= u32(line) << (m_rx_bit - 1);
		m_rx_parity ^= line;
	}
	else if (m_rx_bit == 9)
	{
		m_status = (m_status & ~1) | (m_rx_parity ^ line);
	}
	else
	{
		m_receiving = false;
		if (line)
			receive(m_rx_byte);
		return;
	}
	++m_rx_bit;
	m_rx_timer->adjust(attotime::from_hz(9600));
}

void crystal_hle_device::send(u8 data)
{
	assert(m_out_count < m_output_queue.size());
	m_output_queue[(m_out_head + m_out_count++) % m_output_queue.size()] = data;
	if (m_out_count == 1)
		m_tx_timer->adjust(attotime::from_usec(200));
}

TIMER_CALLBACK_MEMBER(crystal_hle_device::tx_tick)
{
	if (m_tx_bit == -1)
	{
		m_tx_byte = m_output_queue[m_out_head];
		if (!old_protocol())
			m_tx_byte ^= m_tx_key++;
		m_tx_parity = 0;
		for (u32 i = 0; i < 8; ++i)
			m_tx_parity ^= BIT(m_tx_byte, i);
		m_tx_bit = 0;
	}
	if (m_tx_bit == 0)
		output_w(0);
	else if (m_tx_bit <= 8)
		output_w(BIT(m_tx_byte, m_tx_bit - 1));
	else if (m_tx_bit == 9)
		output_w(m_tx_parity);
	else
		output_w(1);
	if (++m_tx_bit == 12)
	{
		m_tx_bit = -1;
		m_out_head = (m_out_head + 1) % m_output_queue.size();
		if (--m_out_count)
			m_tx_timer->adjust(attotime::from_usec(300));
	}
	else
		m_tx_timer->adjust(attotime::from_hz(9600));
}

void crystal_hle_device::receive(u8 data)
{
	if (!old_protocol())
		data ^= m_rx_key--;
	LOGMASKED(LOG_PROTECTION, "%s: RX %02x state %u\n", machine().describe_context(), data, m_state);
	if (m_state == BOOT_TX)
	{
		m_tx_key = data;
		m_state = BOOT_RX;
		return;
	}
	if (m_state == BOOT_RX)
	{
		m_rx_key = data;
		m_state = COMMAND;
		return;
	}
	if (m_state == COMMAND)
	{
		if (old_protocol())
		{
			switch (data)
			{
			case 1: data = 0x10; break;
			case 2: data = 1; break;
			case 4: data = 0x20; break;
			case 8: data = 4; break;
			case 0x10: data = 8; break;
			case 0x20: data = 0x40; break;
			default: data = 0; break;
			}
		}
		m_command = data & m_mask;
		switch (m_command)
		{
		case 0x10:
		{
			for (const char c : m_config->identifier)
				send(c);
			m_mask = 0x31;
			break;
		}
		case 0x20:
			send(m_status | 0x80);
			expect(AUTH_A, 8);
			break;
		case 0x40:
			for (u32 i = 0; i < 32; ++i)
				send(m_patch[i] ^ m_session[i & 7]);
			m_mask = 0xff;
			break;
		case 1:
			send(m_status | 0x80);
			expect(ECHO, 16);
			break;
		case 4:
		case 8:
			send(m_status | 0x80);
			expect(EE_ADDRESS, 1);
			break;
		default:
			m_status |= 4;
			send(m_status);
			break;
		}
		return;
	}
	m_input[m_in_count++] = data;
	if (m_in_count != m_expected)
		return;
	switch (m_state)
	{
	case AUTH_A:
		std::copy_n(m_input.begin(), 8, m_session.begin());
		if (old_protocol())
		{
			m_auth = simple_transform(m_key, m_session);
			// The undumped PIC's nonce table is unknown.  Any eight-byte nonce
			// is accepted by the host; keep it deterministic across save states.
			for (auto &v : m_session)
				v ^= ++m_eeprom[0x3f];
			m_auth = simple_transform(m_auth, m_session);
			for (const u8 v : m_session)
				send(v);
			expect(AUTH_B, 8);
		}
		else
		{
			m_session = transform(m_session, m_keys, m_tables);
			m_auth = m_session;
			for (u32 i = 0; i < 4; ++i)
				send(m_auth[i]);
			expect(AUTH_REPLY, 4);
		}
		break;
	case AUTH_B:
		std::copy_n(m_input.begin(), 8, m_session.begin());
		m_auth = simple_transform(m_auth, m_session);
		for (u32 i = 0; i < 4; ++i)
			send(m_auth[i]);
		expect(AUTH_REPLY, 4);
		break;
	case AUTH_REPLY:
		if (std::equal(m_input.begin(), m_input.begin() + 4, m_auth.begin() + 4))
		{
			++m_eeprom[eeprom_mask() - 2];
			m_mask = 0x7f;
			send(m_status | 0x80);
		}
		else
		{
			++m_eeprom[eeprom_mask() - 3];
			m_status |= 8;
			send(m_status);
		}
		m_state = COMMAND;
		break;
	case ECHO:
		for (u32 i = 0; i < 16; ++i)
			send(m_input[i]);
		m_state = COMMAND;
		break;
	case EE_ADDRESS:
		m_address = data;
		expect(EE_COUNT, 1);
		break;
	case EE_COUNT:
		if (m_command == 8)
		{
			expect(EE_WRITE, data ? data : 256);
		}
		else
		{
			send(m_status);
			const u16 count = data ? data : 256;
			for (u16 i = 0; i < count; ++i)
				send(m_eeprom[(m_address + i) & eeprom_mask()]);
			m_state = COMMAND;
		}
		break;
	case EE_WRITE:
		for (u32 i = 0; i < m_expected; ++i)
			m_eeprom[(m_address + i) & eeprom_mask()] = m_input[i];
		send(m_status);
		m_state = COMMAND;
		break;
	}
}


class crystal_state : public driver_device
{
public:
	crystal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_reset_patch(*this, "reset_patch"),
		m_flash(*this, "flash"),
		m_mainbank(*this, "mainbank"),
		m_maincpu(*this, "maincpu"),
		m_vr0soc(*this, "vr0soc"),
		m_ds1302(*this, "rtc"),
		m_pic(*this, "pic"),
		m_hle(*this, "protection"),
		m_dsw(*this, "DSW"),
		m_system(*this, "SYSTEM"),
		m_lamps(*this, "lamp%u", 1U)
	{ }

	void crysking(machine_config &config) ATTR_COLD;
	void evosocc(machine_config &config) ATTR_COLD;
	void donghaer(machine_config &config) ATTR_COLD;

	void crystal(machine_config &config) ATTR_COLD;
	void pic16f84a(machine_config &config) ATTR_COLD;
	void topbladv(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void lamps_w(offs_t offset, u8 data);
	void protection(machine_config &config, const crystal_hle_device::configuration &protection_config) ATTR_COLD;
	void set_soc_clock(u32 clock) ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<u32> m_workram;
	required_shared_ptr<u32> m_reset_patch;
	required_region_ptr<u32> m_flash;

	required_memory_bank m_mainbank;

	// devices
	required_device<se3208_device> m_maincpu;
	required_device<vrender0soc_device> m_vr0soc;
	required_device<ds1302_device> m_ds1302;
	optional_device<pic16_mid_device> m_pic;
	optional_device<crystal_hle_device> m_hle;

	required_ioport m_dsw;
	required_ioport m_system;

	output_finder<16> m_lamps;

	std::unique_ptr<u8[]> m_dummy_region;

	u32 m_bank = 0;
	u32 m_maxbank = 0;
	u32 m_flashcmd = 0xff;
	u32 m_pio = 0;

	bool m_pic_data = true; // pull-up

	u32 system_input_r();
	void banksw_w(u32 data);
	u32 flashcmd_r();
	void flashcmd_w(u32 data);
	void coin_counters_w(u8 data);

	void patchreset();
	void main_map(address_map &map) ATTR_COLD;

	// PIO
	u32 pioldat_r();
	void pioldat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pioedat_r();

	// PIC
	uint8_t pic_porta_r();
	void pic_porta_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	void pic_line_w(int state);
};


class maldaiza_state : public crystal_state
{
public:
	maldaiza_state(const machine_config &mconfig, device_type type, const char *tag) :
		crystal_state(mconfig, type, tag),
		m_prize(*this, "prize%u", 0U)
	{ }

	void maldaiza(machine_config &config) ATTR_COLD;

protected:
	virtual void lamps_w(offs_t offset, u8 data) override;

private:
	required_device_array<ticket_dispenser_device, 2> m_prize;
};


u32 crystal_state::system_input_r()
{
	return (m_system->read() << 16) | (m_dsw->read()) | 0xff00ff00;
}

void crystal_state::banksw_w(u32 data)
{
	m_bank = (data >> 1) & 7;
	m_mainbank->set_entry(m_bank);
}

void crystal_state::lamps_w(offs_t offset, u8 data)
{
	for (unsigned i = 0; 8 > i; ++i)
		m_lamps[(offset << 3) | i] = BIT(data, i);
}

void maldaiza_state::lamps_w(offs_t offset, u8 data)
{
	crystal_state::lamps_w(offset, data);

	// Maldaliza's normal prize motors use output 3 and output 7 in its
	// packed twelve-output table (bits 3 and 1 of the two physical bytes).
	m_prize[offset]->motor_w(BIT(data, offset ? 1 : 3));
}

u32 crystal_state::pioldat_r()
{
	return m_pio;
}

// PIO Latched output DATa Register
void crystal_state::pioldat_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pio);

	const u32 rst = BIT(m_pio, 24);
	const u32 clk = BIT(m_pio, 25);
	const u32 dat = BIT(m_pio, 28);

	m_ds1302->ce_w(rst);
	m_ds1302->io_w(dat);
	m_ds1302->sclk_w(clk);

	if (m_pic)
	{
		m_pic->set_input_line(INPUT_LINE_RESET, BIT(m_pio, 30) ? ASSERT_LINE : CLEAR_LINE);
		m_pic_data = !BIT(m_pio, 29);
	}

	if (m_hle)
	{
		m_hle->lines_w(BIT(m_pio, 30), !BIT(m_pio, 29));
		// Host writes update readback without raising the PIC output interrupt.
		m_pic_data = m_hle->data_r();
	}
}

// PIO External DATa Register
u32 crystal_state::pioedat_r()
{
	u32 data = 0;

	data |= m_ds1302->io_r() << 28;

	if (m_pic)
		data |= m_pic_data << 29;
	else if (m_hle)
		data |= m_hle->data_r() << 29;

	return data;
}

uint8_t crystal_state::pic_porta_r()
{
	return m_pic_data ? 1 : 0;
}

void crystal_state::pic_porta_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if (BIT(mem_mask, 0))
		pic_line_w(BIT(data, 0));
}

void crystal_state::pic_line_w(int state)
{
	// The Crystal of Kings uses IRQ 20 to detect the serial start bit,
	// then samples the frame with timer 2.  Later games poll PIOEDAT.
	if (m_pic_data && !state)
		m_vr0soc->int_req(20);
	m_pic_data = state;
}

u32 crystal_state::flashcmd_r()
{
	if ((m_flashcmd & 0xff) == 0xff)
	{
		if (m_bank < m_maxbank)
		{
			u32 *ptr = (u32*)(m_mainbank->base());
			return ptr[0];
		}
		else
			return 0xffffffff;
	}
	if ((m_flashcmd & 0xff) == 0x90)
	{
		if (m_bank < m_maxbank)
			return 0x00180089;  //Intel 128MBit
		else
			return 0xffffffff;
	}
	return 0;
}

void crystal_state::flashcmd_w(u32 data)
{
	m_flashcmd = data;
}

void crystal_state::coin_counters_w(u8 data)
{
	// Both signals are sent when setting is "1 shooter"
	// Only evosocc and crysking allow the user to change this setting.
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
}

void crystal_state::main_map(address_map &map)
{
	map(0x00000000, 0x0001ffff).rom().nopw();

	map(0x01200000, 0x01200003).portr("P1_P2");
	map(0x01200000, 0x01200000).w(FUNC(crystal_state::coin_counters_w));
	map(0x01200004, 0x01200007).portr("P3_P4");
	map(0x01200008, 0x0120000b).r(FUNC(crystal_state::system_input_r));

	map(0x01280000, 0x01280003).w(FUNC(crystal_state::banksw_w));
	map(0x01320000, 0x01320003).umask32(0x00ff00ff).w(FUNC(crystal_state::lamps_w));
	map(0x01400000, 0x0140ffff).ram().share("nvram");

	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));
	map(0x01802004, 0x01802007).rw(FUNC(crystal_state::pioldat_r), FUNC(crystal_state::pioldat_w));
	map(0x01802008, 0x0180200b).r(FUNC(crystal_state::pioedat_r));

	// mirror is accessed by donghaer on later levels
	map(0x02000000, 0x027fffff).mirror(0x00800000).ram().share(m_workram);

	map(0x03000000, 0x04ffffff).m(m_vr0soc, FUNC(vrender0soc_device::audiovideo_map));

	map(0x05000000, 0x05ffffff).bankr(m_mainbank);
	map(0x05000000, 0x05000003).rw(FUNC(crystal_state::flashcmd_r), FUNC(crystal_state::flashcmd_w));

	map(0x44414f4c, 0x44414f7f).ram().share(m_reset_patch);
}

void crystal_state::patchreset()
{
	if (!m_reset_patch)
		return;

	//The test menu reset routine seems buggy
	//it reads the reset vector from 0x02000000 but it should be
	//read from 0x00000000. At 0x2000000 there is the BIOS signature
	//"LOADED VER....", so it jumps to "LOAD" in hex (0x44414F4C)
	//I'll add some code there that makes the game stay in a loop
	//reading the flip register so the idle skip works

/*
Loop1:
    LDI 1,%R2
    LDI     0x30000a6,%R1
    STS %R2,(%R1,0x0)

loop:
    LDI 0x30000a6,%R1
    LDSU    (%R1,0x0),%R2
    CMP     %R2,0x0000
    JNZ loop

    JMP Loop1
*/


#if 1
	static const u32 patch[] =
	{
		0x40c0ea01,
		0xe906400a,
		0x40c02a20,
		0xe906400a,
		0xa1d03a20,
		0xdef4d4fa
	};

	memcpy(m_reset_patch, patch, sizeof(patch));
#else
	static const u8 patch[] =
	{
		0x01,0xea,0xc0,0x40,0x0a,0x40,0x06,0xe9,
		0x20,0x2a,0xc0,0x40,0x0a,0x40,0x06,0xe9,
		0x20,0x3a,0xd0,0xa1,0xfa,0xd4,0xf4,0xde
	};

	memcpy(m_reset_patch, patch, sizeof(patch));
#endif
}

void crystal_state::machine_start()
{
	patchreset();

	if (m_mainbank)
	{
		m_maxbank = m_flash ? (m_flash.bytes() / 0x1000000) : 0;
		m_dummy_region = std::make_unique<u8 []>(0x1000000);
		std::fill_n(&m_dummy_region[0], 0x1000000, 0xff); // 0xff filled at unmapped area
		u8 *rom = (m_flash) ? (u8 *)&m_flash[0] : &m_dummy_region[0];
		for (int i = 0; i < 8; i++)
		{
			if (i < m_maxbank)
				m_mainbank->configure_entry(i, rom + i * 0x1000000);
			else
				m_mainbank->configure_entry(i, m_dummy_region.get());
		}
	}

	save_item(NAME(m_bank));
	save_item(NAME(m_flashcmd));
	save_item(NAME(m_pio));
	save_item(NAME(m_pic_data));
}

void crystal_state::machine_reset()
{
	m_bank = 0;
	m_mainbank->set_entry(m_bank);
	m_flashcmd = 0xff;

	patchreset();
}

INPUT_CHANGED_MEMBER(crystal_state::coin_inserted)
{
	if (oldval)
	{
		const u8 coin_chute = (u8)(uintptr_t)param & 1;
		m_vr0soc->int_req(coin_chute ? 19 : 12);
	}
}

static INPUT_PORTS_START( crystal )
	PORT_START("P1_P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3_P4")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(crystal_state::coin_inserted), 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(crystal_state::coin_inserted), 1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Pause ) )       PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Free_Play ) )   PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )     PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )     PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )     PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )     PORT_DIPLOCATION("DSW:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )     PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Test ) )        PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( topbladv )
	PORT_INCLUDE( crystal )

	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x000000f0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P3_P4")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNUSED )

	// TODO: coin 2 insertion is fuzzy, may be BTANB
	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(crystal_state::coin_inserted), 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(crystal_state::coin_inserted), 1)
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END


static INPUT_PORTS_START( officeye )
	PORT_INCLUDE( crystal )

	// TODO: player 2 start doesn't work ingame, it only skips attract mode items,
	// or be valid in test mode when prompted to press start
	// lamps test also gives only p1 and p3, and after crediting up only p1 and p3 starts are lighted on ($01320000 address writes)
	// I guess game is currently in 2 players mode, is there anything that enables 3 players mode?
	// notice that this is valid for both urachamu and officeye
	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Red")
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Green")
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Blue")
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Red")
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("P3 Red")
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Green")
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("P3 Green")
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Blue")
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3) PORT_NAME("P3 Blue")
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("P3_P4")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0000000f, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( maldaiza )
	// The three buttons also join/select; there are no separate start buttons.
	// In setup, P1 button 2 moves down and P1 buttons 1/3 change/select.
	PORT_START("P1_P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("prize1", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("prize0", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0xffaaffaa, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3_P4")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0000000f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(crystal_state::coin_inserted), 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(crystal_state::coin_inserted), 1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )

	// Coinage, demo sound, race length and prize probability are set in the
	// service menu. The generic Crystal DIP meanings are not established here.
	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "DSW:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSW:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSW:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DSW:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSW:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( urachamu )
	PORT_INCLUDE( officeye )

	// uses blue/yellow/red rather than red/green/blue
	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Blue")
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Yellow")
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Red")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Blue")
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("P3 Blue")
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Yellow")
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("P3 Yellow")
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Red")
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3) PORT_NAME("P3 Red")
INPUT_PORTS_END

static INPUT_PORTS_START( wulybuly )
	PORT_INCLUDE( crystal )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Test ) )        PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )     PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void crystal_state::crystal(machine_config &config)
{
	SE3208(config, m_maincpu, 14'318'180 * 3); // TODO : dynamic via PLL
	m_maincpu->set_addrmap(AS_PROGRAM, &crystal_state::main_map);
	m_maincpu->iackx_cb().set(m_vr0soc, FUNC(vrender0soc_device::irq_callback));

	// DEFAULT_ALL_1 seems to be correct
	// maldaiza and donghaer require it or they don't create valid defaults
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	VRENDER0_SOC(config, m_vr0soc, 14'318'180 * 6); // TODO : dynamic via PLL
	m_vr0soc->set_host_space_tag(m_maincpu, AS_PROGRAM);
	m_vr0soc->int_callback().set_inputline(m_maincpu, se3208_device::SE3208_INT);

	DS1302(config, m_ds1302, 32.768_kHz_XTAL);

	SPEAKER(config, "speaker", 2).front();
	m_vr0soc->add_route(0, "speaker", 1.0, 0);
	m_vr0soc->add_route(1, "speaker", 1.0, 1);
}

void crystal_state::protection(machine_config &config, const crystal_hle_device::configuration &protection_config)
{
	CRYSTAL_HLE(config, m_hle).set_config(protection_config);
	m_hle->set_flash_region(m_flash);
	m_hle->data_callback().set(FUNC(crystal_state::pic_line_w));
}

void crystal_state::set_soc_clock(u32 clock)
{
	m_maincpu->set_clock(clock / 2);
	m_vr0soc->set_clock(clock);
}

void crystal_state::crysking(machine_config &config)
{
	crystal(config);
	// Reconstructed PIC response records; the host applies them to its own RAM.
	static constexpr crystal_hle_device::configuration PROTECTION = {
		true, 0x000078ea, 0x00007534, "Brezza Soft Corp.  ",
		{ 0x02012bfa, 0x9c00df01, 0x020147ae, 0x9001901c, 0x020130da, 0x900190fc, 0x02013a96, 0x403c4000 }
	};
	protection(config, PROTECTION);
}

void crystal_state::evosocc(machine_config &config)
{
	crystal(config);
	// Reconstructed PIC response records; the host applies them to its own RAM.
	static constexpr crystal_hle_device::configuration PROTECTION = {
		false, 0x028f7fe8, 0x028f7c18, "Brezza Soft Soccer.",
		{ 0x0208e2d6, 0x900190fc, 0x0208baa0, 0x9001907c, 0x02092a7e, 0x8303900c, 0x0208f918, 0x900190fc }
	};
	protection(config, PROTECTION);
}

void crystal_state::donghaer(machine_config &config)
{
	crystal(config);
	// Reconstructed PIC response records; the host applies them to its own RAM.
	static constexpr crystal_hle_device::configuration PROTECTION = {
		false, 0x00003590, 0x000031c0, "Danbi System HERONG",
		{ 0x02012c42, 0x82029004, 0x02012cd4, 0x92009001, 0x0201a13e, 0x40819004, 0x02029110, 0x9001900c }
	};
	protection(config, PROTECTION);
}

void maldaiza_state::maldaiza(machine_config &config)
{
	crystal(config);

	// Feedback wiring follows the game's prize-output/input handlers. The
	// half-second mechanical delay is estimated, pending cabinet measurements.
	TICKET_DISPENSER(config, m_prize[0], attotime::from_msec(500));
	TICKET_DISPENSER(config, m_prize[1], attotime::from_msec(500));
	// Reconstructed PIC response records; the host applies them to its own RAM.
	static constexpr crystal_hle_device::configuration PROTECTION = {
		false, 0x00002e4c, 0x00002a7c, "Danbi System RUN H.",
		{ 0x020196f6, 0x82029004, 0x0201976c, 0x401a4000, 0x0201bf54, 0x4080900c, 0x020194a2, 0x90019004 }
	};
	protection(config, PROTECTION);
	// The game programs the same 0x573c PLL setting as Top Blade V.
	set_soc_clock(14'318'180 * (87 + 8) / (15 + 2));
}

void crystal_state::pic16f84a(machine_config &config)
{
	crystal(config);

	PIC16F84A(config, m_pic, 3.579545_MHz_XTAL);
	m_pic->read_a().set(FUNC(crystal_state::pic_porta_r));
	m_pic->write_a().set(FUNC(crystal_state::pic_porta_w));
}

void crystal_state::topbladv(machine_config &config)
{
	crystal(config);

	// the PLL is programmed with 0x573c
	// assuming a reference clock of 14'318'180 this gives ~40 MHz
	// needs to be correct for CPU <-> PIC communication
	set_soc_clock(14'318'180 * (87 + 8) / (15 + 2));

	PIC16F628A(config, m_pic, 3.579545_MHz_XTAL);
	m_pic->read_a().set(FUNC(crystal_state::pic_porta_r));
	m_pic->write_a().set(FUNC(crystal_state::pic_porta_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

#define CRYSBIOS \
	ROM_REGION( 0x20000, "maincpu", 0 )  \
	ROM_SYSTEM_BIOS( 0, "amg0110b", "AMG0110B PCB" ) \
	ROMX_LOAD("mx27l1000.u14",  0x000000, 0x020000, CRC(beff39a9) SHA1(b6f6dda58d9c82273f9422c1bd623411e58982cb), ROM_BIOS(0)) \
	ROM_SYSTEM_BIOS( 1, "amg0110d", "AMG0110D PCB" ) /* newer? */ \
	ROMX_LOAD("mx27l1000-alt.u14",  0x000000, 0x020000, CRC(1e8175c8) SHA1(f60c016be2ff11e47b2192acddb92676043af501), ROM_BIOS(1))

ROM_START( crysbios )
	CRYSBIOS

	ROM_REGION32_LE( 0x1000000, "flash", ROMREGION_ERASEFF )
ROM_END

ROM_START( crysking )
	CRYSBIOS

	ROM_REGION16_LE(0x4280, "pic", 0) // PIC16F84A? - not dumped
	// Label: "dgSMART-PR3  MAGIC EYES"
	ROM_LOAD("crysking_pic16f84a.u14", 0x0000, 0x4280, NO_DUMP)

	ROM_REGION32_LE( 0x3000000, "flash", 0 )
	ROM_LOAD("bcsv0004f01.u1",  0x0000000, 0x1000000, CRC(8feff120) SHA1(2ea42fa893bff845b5b855e2556789f8354e9066) )
	ROM_LOAD("bcsv0004f02.u2",  0x1000000, 0x1000000, CRC(0e799845) SHA1(419674ce043cb1efb18303f4cb7fdbbae642ee39) )
	ROM_LOAD("bcsv0004f03.u3",  0x2000000, 0x1000000, CRC(659e2d17) SHA1(342c98f3f695ef4dea8b533612451c4d2fb58809) )
ROM_END

ROM_START( evosocc )
	CRYSBIOS

	ROM_REGION16_LE(0x4280, "pic", 0) // PIC16F84A? - not dumped
	// Label: "MAGICEYES  dgSMART-PR2  0134HAH"
	ROM_LOAD("evosocc_pic16f84a.u14", 0x0000, 0x4280, NO_DUMP)

	ROM_REGION32_LE( 0x3000000, "flash", 0 )
	ROM_LOAD("bcsv0001u01",  0x0000000, 0x1000000, CRC(2581a0ea) SHA1(ee483ac60a3ed00a21cb515974cec4af19916a7d) )
	ROM_LOAD("bcsv0001u02",  0x1000000, 0x1000000, CRC(47ef1794) SHA1(f573706c17d1342b9b7aed9b40b8b648f0bf58db) )
	ROM_LOAD("bcsv0001u03",  0x2000000, 0x1000000, CRC(f396a2ec) SHA1(f305eb10856fb5d4c229a6b09d6a2fb21b24ce66) )
ROM_END

ROM_START( topbladv )
	CRYSBIOS

	ROM_REGION16_LE(0x4300, "pic", 0) // PIC16F628A
	// Label: "MAGICEYES  dgSMART-PR2  0134HAG"
	ROM_LOAD("top_blade_v_pic16f628a.u14", 0x0000, 0x4300, CRC(9cdea57b) SHA1(884156085f9e780cdf719aedc2e8a0fd5983613b))

	ROM_REGION32_LE( 0x1000000, "flash", 0 )
	ROM_LOAD("flash.u1",  0x0000000, 0x1000000, CRC(bd23f640) SHA1(1d22aa2c828642bb7c1dfea4e13f777f95acc701) )
ROM_END

ROM_START( officeye )
	ROM_REGION( 0x20000, "maincpu", 0 ) // bios (not the standard one)
	ROM_LOAD("bios.u14",  0x000000, 0x020000, CRC(ffc57e90) SHA1(6b6a17fd4798dea9c7b880f3063be8494e7db302) )

	ROM_REGION16_LE( 0x4280, "pic", 0) // PIC16F84A
	ROM_LOAD("office_yeo_in_cheon_ha_pic16f84a.u14", 0x0000, 0x4280, CRC(7561cdf5) SHA1(eade592823a110019b4af81a7dc56d01f7d6589f))

	ROM_REGION32_LE( 0x2000000, "flash", 0 )
	ROM_LOAD("flash.u1",  0x0000000, 0x1000000, CRC(d3f3eec4) SHA1(ea728415bd4906964b7d37f4379a8a3bd42a1c2d) )
	ROM_LOAD("flash.u2",  0x1000000, 0x1000000, CRC(e4f85d0a) SHA1(2ddfa6b3a30e69754aa9d96434ff3d37784bfa57) )
ROM_END

ROM_START( donghaer )
	CRYSBIOS

	ROM_REGION16_LE(0x4280, "pic", 0) // PIC16F84A? - not dumped
	// Label: "MAGICEYES  dgSMART-PR2  0134HAF"
	ROM_LOAD("donghaer_pic16f84a.u14", 0x0000, 0x4280, NO_DUMP)

	ROM_REGION32_LE( 0x2000000, "flash", 0 )
	ROM_LOAD( "u1",           0x0000000, 0x1000000, CRC(61217ad7) SHA1(2593f1356aa850f4f9aa5d00bec822aa59c59224) )
	ROM_LOAD( "u2",           0x1000000, 0x1000000, CRC(6d82f1a5) SHA1(036bd45f0daac1ffeaa5ad9774fc1b56e3c75ff9) )
ROM_END

ROM_START( wulybuly )
	CRYSBIOS

	ROM_REGION32_LE( 0x1000000, "flash", 0 )
	ROM_LOAD( "u1",           0x0000000, 0x1000000,  CRC(7406f5db) SHA1(dd53afb08d0567241d08d2422c672d429ef9b78f) )

	ROM_REGION( 0x10000, "nvram", 0 ) // otherwise it defaults to completely invalid coinage
	ROM_LOAD( "nvram",  0x0000000, 0x10000, CRC(5908b829) SHA1(c23180f1b3a80e29c61e25584ffa929e41479b56) )
ROM_END

ROM_START( urachamu )
	CRYSBIOS

	ROM_REGION32_LE( 0x4000000, "flash", 0 )
	ROM_LOAD( "u1",           0x0000000, 0x1000000,  CRC(f341d6fc) SHA1(23ecd9f3e5e20fc2a293cc735c8c4d60d01b68c0) )
	ROM_LOAD( "u2",           0x1000000, 0x1000000,  CRC(ad81d61f) SHA1(5872c1d96e25d1b2a1b8a35c8ff163d67cfdabe4) )
	ROM_LOAD( "u3",           0x2000000, 0x1000000,  CRC(3c7e32a4) SHA1(8a26e745eccc00a2c622062c8ad027781dfc1969) )
	ROM_LOAD( "u4",           0x3000000, 0x1000000,  CRC(c5505627) SHA1(5229f435b4cf218d50ae4a4ae65a6ae13d7b7080) )

	// without this game defaults in free play mode, with no demo sound and 0 lives count.
	ROM_REGION( 0x10000, "nvram", 0 )
	ROM_LOAD( "nvram",        0x00000, 0x10000, CRC(66df826d) SHA1(09260b76cb17082c841e7e37b89cb076e54ffb8d) )
ROM_END

ROM_START( maldaiza )
	CRYSBIOS

	ROM_REGION16_LE(0x4280, "pic", 0) // PIC16F84A? - not dumped
	ROM_LOAD("maldaliza_pic16f84a.bin", 0x0000, 0x4280, NO_DUMP)

	ROM_REGION32_LE( 0x2000000, "flash", 0 )
	ROM_LOAD( "u1",           0x0000000, 0x1000000,  CRC(f484d12b) SHA1(29641cda9138b5bf02c2ece34f8289385fd2ba29) )
	ROM_LOAD( "u2",           0x1000000, 0x1000000,  CRC(86175ebf) SHA1(c1800f7339dafd3ec6c0302eebc8406582a46b04) ) // $ff filled
ROM_END

/* Original note on the PIC serial connection from ElSemi:

The PIC uses a software UART bit banged on a single output pin of the main CPU:
the data port is bit 0x20000000 on the PIO register, the same register where the
EEPROM control lines are.  The serial data is transmitted at 8 data bits, even
parity, 1 stop bit. It's probably tricky to get it working properly because it
doesn't rely on a clock signal, and so, the PIC and main CPU must run in
parallel, and the bit lengths must match.  The PIC bit delay routine is just a
loop.  Also it seems that bit 0x40000000 is the PIC reset.

*/

} // anonymous namespace


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT    MACHINE    INPUT     CLASS          INIT           ROT   COMPANY                FULLNAME                              FLAGS
GAME( 2001, crysbios, 0,        crystal,   crystal,  crystal_state, empty_init,    ROT0, "BrezzaSoft",          "Crystal System BIOS",                MACHINE_IS_BIOS_ROOT )
GAME( 2001, crysking, crysbios, crysking,  crystal,  crystal_state, empty_init,    ROT0, "BrezzaSoft",          "The Crystal of Kings",               MACHINE_UNEMULATED_PROTECTION )
GAME( 2001, evosocc,  crysbios, evosocc,   crystal,  crystal_state, empty_init,    ROT0, "Evoga / BrezzaSoft",  "Evolution Soccer",                   0 )
GAME( 2001, officeye, 0,        pic16f84a, officeye, crystal_state, empty_init,    ROT0, "Danbi",               "Office Yeoin Cheonha (version 1.2)", 0 )
GAME( 2001, donghaer, crysbios, donghaer,  crystal,  crystal_state, empty_init,    ROT0, "Danbi",               "Donggul Donggul Haerong",            0 )
GAME( 2002, urachamu, crysbios, crystal,   urachamu, crystal_state, empty_init,    ROT0, "GamToU",              "Urachacha Mudaeri (Korea)",          0 ) // lamps, verify game timings
GAME( 2003, topbladv, crysbios, topbladv,  topbladv, crystal_state, empty_init,    ROT0, "Sonokong / Expotato", "Top Blade V",                        0 )
GAME( 200?, wulybuly, crysbios, crystal,   wulybuly, crystal_state, empty_init,    ROT0, "<unknown>",           "Wully Bully",                        MACHINE_NOT_WORKING )
GAME( 2002, maldaiza, crysbios, maldaiza,  maldaiza, maldaiza_state, empty_init,    ROT0, "GamToU",              "Maldaliza!",                        0 )
