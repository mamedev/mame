// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Luca Elia
/************************************************************************************************************

                                        -= IGS017 / IGS031 Based Hardware =-

                             driver by   Pierpaolo Prazzoli, Luca Elia (l.elia@tin.it)

CPU:   Z180 or 68000
Video: IGS017 or IGS031 (2 tilemaps, variable size sprites, protection)
Other: IGS025 (8255), IGS022 (protection, MCU), IGS029 (protection)
Sound: M6295(K668/AR17961) + [YM2413(U3567)]

-------------------------------------------------------------------------------------------------------------
Year + Game                                   PCB        CPU    Sound           Custom                Other
-------------------------------------------------------------------------------------------------------------
96  Shuzi Leyuan (V127M)                      NO-0131-4  Z180   AR17961 U3567   IGS017 8255           Battery
97  Chaoji Damanguan II (V754C)               NO-0147-6  68000  K668            IGS031 8255           Battery
97  Tian Jiang Shen Bing (V137C)              NO-0157-2  Z180   AR17961 U3567   IGS017 IGS025         Battery
97  Manguan Daheng (V123T1)                   NO-0252    68000  M6295           IGS031 IGS025 IGS???* Battery
98  Genius 6 (V110F)                          NO-0131-4  Z180   K668    U3567   IGS017 IGS003c        Battery
98  Long Hu Zhengba 2 (set 1)                 NO-0206    68000  K668            IGS031 IGS025 IGS022* Battery
98  Shuang Long Qiang Zhu 2 VS (VS203J)       NO-0207    68000  K668            IGS031 IGS025 IGS022  Battery
98  Manguan Caishen (V103CS)                  NO-0192-1  68000  K668            IGS017 IGS025 IGS029  Battery
98  Manguan Caishen (V106CS)                  NO-0208    68000  M6295           IGS031 IGS025 IGS029  Battery
99  Tarzan (V107)                             NO-0228?   Z180   U6295           IGS031 IGS025 IGS029  Battery
99  Tarzan (V109C)                            NO-0248-1  Z180   U6295           IGS031 IGS025         Battery
00  Chaoji Damanguan 2 - Jiaqiang Ban (V100C) NO-0271    68000  K668            IGS031 IGS025         Battery
00? Super Tarzan (V100I)                      NO-0230-1  Z180   K668            IGS031 IGS025         Battery
00? Happy Skill (V611IT)                      NO-0281    Z180   K668            IGS031 IGS025         Battery
00? Champion Poker 2 (V100A)                  unreadable Z180   M6295           IGS031 IGS025         Battery
00? Super Poker (V100xD03) / Formosa          NO-0187    Z180   K668    U3567   IGS017 IGS025         Battery
-------------------------------------------------------------------------------------------------------------
                                                                         not present in another set *
To Do:

- Protection emulation in some games, instead of patching the roms.
- NVRAM.
- mgcs: Finish IGS029 protection simulation.

Notes:

- Test mode is usually accessed by keeping test (F2) pressed during boot.
- iqblocka: keep start (1) pressed during boot for DSWs & input test. Keep test (F2) pressed for book-keeping / setup [pass: press deal (2)].
- iqblockf/genius6: press service1 (9) then press deal (2) eight times to switch to gambling. Then test (F2) enters book-keeping / setup.
- lhzb2, mgcs, slqz2, tjsb: press test (F2) + book (0) during inputs test for sound test.
- mgdh, sdmg2: press keys A + B during test mode for sound test (B1 + B2 + B3 when using a joystick in mgdh).
- spkrform: to switch from poker to Formosa press service1 (9). To switch back, press in sequence:
            service3 (right of 0) then Bet (M) then press "Hold 1".."Hold 5" (Z, X, C, V, B)

************************************************************************************************************/

#include "emu.h"

#include "igs017_igs031.h"
#include "igs022.h"
#include "mahjong.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z180/z180.h"
#include "machine/i8255.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "multibyte.h"

#include "igspoker.lh"
#include "igsslot.lh"

#define LOG_PROT_STRING     (1U << 1)
#define LOG_PROT_BITSWAP    (1U << 2)
#define LOG_PROT_INCDEC     (1U << 3)
#define LOG_PROT_INC        (1U << 4)
#define LOG_PROT_SCRAMBLE   (1U << 5)
#define LOG_PROT_REMAP      (1U << 6)
#define LOG_PROT_IGS022     (1U << 7)
#define LOG_PROT_IGS029     (1U << 8)

//#define VERBOSE (LOG_GENERAL | LOG_PROT_STRING | LOG_PROT_BITSWAP | LOG_PROT_INCDEC | LOG_PROT_INC | LOG_PROT_SCRAMBLE | LOG_PROT_REMAP | LOG_PROT_IGS022 | LOG_PROT_IGS029)
#define VERBOSE (0)
#include "logmacro.h"

/***************************************************************************

    ---- IGS Multiplexer ----

***************************************************************************/

class igs_mux_device :
	public device_t,
	public device_memory_interface
{
public:
	igs_mux_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void address_w(u8 data);
	void data_w(u8 data);
	u8 data_r();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

private:
	address_space_config m_space_config;
	u8 m_address = 0;
};

device_memory_interface::space_config_vector igs_mux_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

DEFINE_DEVICE_TYPE(IGS_MUX, igs_mux_device, "igs_mux", "IGS I/O Multiplexer")

igs_mux_device::igs_mux_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, IGS_MUX, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_config("igs_mux", ENDIANNESS_LITTLE, 8, 8, 0)
{ }

void igs_mux_device::device_start()
{
	save_item(NAME(m_address));
}

void igs_mux_device::device_reset()
{
}

void igs_mux_device::address_w(u8 data)
{
	m_address = data;
}

void igs_mux_device::data_w(u8 data)
{
	space().write_byte(m_address, data);
}

u8 igs_mux_device::data_r()
{
	return space().read_byte(m_address);
}

/***************************************************************************

    ---- IGS String Protection ----

***************************************************************************/

class igs_string_device : public device_t
{
public:
	igs_string_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u8 result_r();                                  // 0x05:        result_r
	void do_bitswap_w(offs_t offset, u8 data);      // 0x20-0x27:   do_bitswap_w
	u8 advance_string_offs_r(address_space &space); // 0x40:        advance_string_offs_r

	// Call after decryption, e.g. at the end of init_<gamename>
	void dump(const char *filename, u32 string_addr, u32 xor_addr, bool is_16bits) const;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_memory_region m_region_key;

	u16 m_string_word = 0, m_word = 0;
	u8 m_string_offs = 0;
};

DEFINE_DEVICE_TYPE(IGS_STRING, igs_string_device, "igs_string", "IGS String Protection")

igs_string_device::igs_string_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, IGS_STRING, tag, owner, clock)
	, m_region_key(*this, DEVICE_SELF)
{ }

void igs_string_device::dump(const char *filename, u32 string_addr, u32 xor_addr, bool is_16bits) const
{
	FILE *f = fopen(filename, "wb");
	if (!f)
	{
		printf("Error opening igs_string dump file %s\n", filename);
		fclose(f);
		return;
	}

	printf("igs_string addr = %x, %x (%d bits)\n", string_addr, xor_addr, is_16bits ? 16 : 8);

	const u8 * const string_base    =   (const u8 *)memregion(":maincpu")->base() + string_addr;
	const u8 * const xor_base       =   (const u8 *)memregion(":maincpu")->base() + xor_addr;
	for (u32 i = 0; i < 0xec; ++i)
	{
		const u32 addr = is_16bits ? BYTE_XOR_BE(i) : i;
		const u8 data  = string_base[addr] ^ xor_base[addr];
		fwrite(&data, 1, 1, f);
	}

	fclose(f);
}

void igs_string_device::device_start()
{
	save_item(NAME(m_string_offs));
	save_item(NAME(m_string_word));
	save_item(NAME(m_word));
}

void igs_string_device::device_reset()
{
	m_string_word = m_word = 0;
	m_string_offs = 0;
}

u8 igs_string_device::result_r()
{
	const u8 res = bitswap<8>(m_word, 5, 2, 9, 7, 10, 13, 12, 15);
	LOGMASKED(LOG_PROT_STRING, "%s: read bitswap - word %04x -> %02x\n", machine().describe_context(), m_word, res);
	return res;
}

void igs_string_device::do_bitswap_w(offs_t offset, u8 data)
{
	const u16 x = m_word;

	m_word = 0;

	m_word |= (BIT( x,  0) ^ BIT(m_string_word,  0)) << 1;  // bit 1
	m_word |= (BIT(~x,  1) ^ BIT(m_string_word,  1)) << 2;  // bit 2
	m_word |= (BIT(~x,  2) ^ BIT(m_string_word,  2)) << 3;  // bit 3
	m_word |= (BIT(~x, 13) ^ BIT(            x,  3)) << 4;  // bit 4
	m_word |= (BIT(~x,  4) ^ BIT(m_string_word,  4)) << 5;  // bit 5
	m_word |= (BIT( x,  5) ^ BIT(m_string_word,  5)) << 6;  // bit 6
	m_word |= (BIT(~x,  6) ^ BIT(m_string_word,  6)) << 7;  // bit 7
	m_word |= (BIT(~x,  7) ^ BIT(m_string_word,  7)) << 8;  // bit 8
	m_word |= (BIT(~x,  8) ^ BIT(m_string_word,  8)) << 9;  // bit 9
	m_word |= (BIT( x,  9) ^ BIT(m_string_word,  9)) << 10; // bit 10
	m_word |= (BIT(~x, 10) ^ BIT(            x,  3)) << 11; // bit 11
	m_word |= (BIT( x, 11) ^ BIT(m_string_word, 11)) << 12; // bit 12
	m_word |= (BIT(~x, 12) ^ BIT(m_string_word, 12)) << 13; // bit 13
	m_word |= (BIT( x, 13) ^ BIT(m_string_word, 13)) << 14; // bit 14
	m_word |= (BIT( x, 14) ^ BIT(m_string_word, 14)) << 15; // bit 15

	// bit 0
	const u16 bit0 = BIT(~x, 15) ^ BIT(x, 7);

	// bit 0 is further xor'd with bit x of the data written to this address
	const u16 xor0 = BIT(data, offset & 7);
	m_word |= bit0 ^ xor0;

	LOGMASKED(LOG_PROT_STRING, "%s: exec bitswap on port %02x = %02x - bit0 %x, xor0 %x, word %04x -> %04x\n", machine().describe_context(),
		offset, data, bit0, xor0, x, m_word);
}

u8 igs_string_device::advance_string_offs_r(address_space &space)
{
	const u8 next_string_offs   =   ((m_string_offs + 1) < 0xec ? (m_string_offs + 1) : 0);

	const u8 shift              =   (next_string_offs & 1) ? 8 : 0;

	const u8 next_string_byte   =   m_region_key ? m_region_key->base()[next_string_offs] : 0;

	const u16 next_string_word  =   (m_string_word & (0xff << (8 - shift))) | (next_string_byte << shift);

	LOGMASKED(LOG_PROT_STRING, "%s: advance string offs %02x -> %02x, string data = %02x, string word %04x -> %04x\n", machine().describe_context(),
		m_string_offs, next_string_offs, next_string_byte, m_string_word, next_string_word);

	m_string_offs = next_string_offs;
	m_string_word = next_string_word;

	return space.unmap();
}

// Similar protection to that found in igs011.cpp:

/***************************************************************************

    ---- IGS Parametric Bitswaps Protection ----

    The protection involves an internal 16-bit value (val), two mode registers
    (mode_f = 0..f, mode_3 = 0..3) and 8 x 16-bit registers (word).

    The two modes affect the bitswap, and are set by loading the (same) mode-specific value
    to all the word registers, and then writing to the mode_f or mode_3 trigger register.

    The bitswap of the internal value is then performed writing to one of 8 trigger registers,
    according to the modes, trigger register and value written.

    The result is read through a fixed bitswap of the internal value.

***************************************************************************/

class igs_bitswap_device : public device_t
{
public:
	igs_bitswap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u8 result_r();                              // 0x03:        result_r
	void word_w(u8 data);                       // 0x40-0x47:   word_w
	void mode_f_w(u8 data);                     // 0x48:        mode_f_w
	void mode_3_w(u8 data);                     // 0x50:        mode_3_w
	void do_bitswap_w(offs_t offset, u8 data);  // 0x80-0x87:   do_bitswap_w
	void reset_w(u8 data);                      // 0xa0:        reset_w

	template <unsigned N> void set_m3_bits(u8 b0, u8 b1, u8 b2, u8 b3);
	void set_mf_bits(u8 b0, u8 b1, u8 b2, u8 b3);
	void set_val_xor(u16 val_xor);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 m_m3 = 0, m_mf = 0;
	u16 m_val = 0, m_word = 0;

	u8 m_m3_bits[4][4] = { {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0} };
	u8 m_mf_bits[4] = {0,0,0,0};
	u16 m_val_xor = 0;
};

template <unsigned N> void igs_bitswap_device::set_m3_bits(u8 b0, u8 b1, u8 b2, u8 b3)
{
	m_m3_bits[N][0] = b0;
	m_m3_bits[N][1] = b1;
	m_m3_bits[N][2] = b2;
	m_m3_bits[N][3] = b3;

#if 0
	printf("igs_bitswap: INIT m3_bits[%x] =", N);
	for (int i = 0; i < 4; ++i)
	{
		u8 bit = m_m3_bits[N][i];
		printf(" %c%d", BIT(bit, 7) ? '~' : ' ', BIT(bit, 7) ? (bit ^ 0xff) : bit);
	}
	printf("\n");
#endif
}

void igs_bitswap_device::set_mf_bits(u8 b0, u8 b1, u8 b2, u8 b3)
{
	m_mf_bits[0] = b0;
	m_mf_bits[1] = b1;
	m_mf_bits[2] = b2;
	m_mf_bits[3] = b3;

#if 0
	printf("igs_bitswap: INIT mf_bits = %d %d %d %d\n", m_mf_bits[0], m_mf_bits[1], m_mf_bits[2], m_mf_bits[3]);
#endif
}

void igs_bitswap_device::set_val_xor(u16 val_xor)
{
	m_val_xor = val_xor;

#if 0
	printf("igs_bitswap: INIT val_xor = %04x\n", m_val_xor);
#endif
}

DEFINE_DEVICE_TYPE(IGS_BITSWAP, igs_bitswap_device, "igs_bitswap", "IGS Bitswap Protection")

igs_bitswap_device::igs_bitswap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, IGS_BITSWAP, tag, owner, clock)
{ }

void igs_bitswap_device::device_start()
{
	save_item(NAME(m_m3));
	save_item(NAME(m_mf));
	save_item(NAME(m_val));
	save_item(NAME(m_word));
}

void igs_bitswap_device::device_reset()
{
	m_m3 = m_mf = 0;
	m_val = m_word = 0;
}

u8 igs_bitswap_device::result_r()
{
	u8 res = bitswap<8>(m_val, 5, 2, 9, 7, 10, 13, 12, 15);
	LOGMASKED(LOG_PROT_BITSWAP, "%s: read bitswap - val %04x -> %02x\n", machine().describe_context(), m_val, res);
	return res;
}

void igs_bitswap_device::word_w(u8 data)
{
	m_word = (m_word << 8) | data;
	LOGMASKED(LOG_PROT_BITSWAP, "%s: word = %02x\n", machine().describe_context(), m_word);
}

void igs_bitswap_device::mode_f_w(u8 data)
{
	m_mf = 0;
	if ((m_word & 0x0f00) != 0x0a00)    m_mf |= 0x08;
	if ((m_word & 0xf000) != 0x9000)    m_mf |= 0x04;
	if ((m_word & 0x000f) != 0x0006)    m_mf |= 0x02;
	if ((m_word & 0x00f0) != 0x0090)    m_mf |= 0x01;

	LOGMASKED(LOG_PROT_BITSWAP, "%s: mode_f = %x (word = %04x)\n", machine().describe_context(), m_mf, m_word);
}

void igs_bitswap_device::mode_3_w(u8 data)
{
	m_m3 = 0;
	if ((m_word & 0x000f) != 0x0003)    m_m3 |= 0x02;
	if ((m_word & 0x00f0) != 0x0050)    m_m3 |= 0x01;

	LOGMASKED(LOG_PROT_BITSWAP, "%s: mode_3 = %x (word = %04x)\n", machine().describe_context(), m_m3, m_word);
}

void igs_bitswap_device::do_bitswap_w(offs_t offset, u8 data)
{
	u16 x = m_val;

	// bits 1-15 come from inverting some bits, xor-ing 4 bits (they change per game) with the mf bits, then shifting to the left
	m_val ^= m_val_xor;
	for (int i = 0; i < 4; ++i)
		m_val ^= BIT(m_mf, i) << m_mf_bits[i];
	m_val <<= 1;

	// bit 0 is the xor of 4 bits from val (some inverted). The 4 bits are picked based on m3 (and they change per game)
	u16 bit0 = 0;
	for (int i = 0; i < 4; ++i)
	{
		u8 bit = m_m3_bits[m_m3][i];
		bit0 ^= BIT(bit, 7) ? (BIT(x, bit ^ 0xff) ^ 1) : BIT(x, bit);
	}

	// bit 0 is further xor'd with bit x of the data written to this address (8x)
	u16 xor0 = BIT(data, offset & 7);
	m_val |= bit0 ^ xor0;

	LOGMASKED(LOG_PROT_BITSWAP, "%s: exec bitswap on port %02x = %02x - mode_3 %02x, mode_f %02x, bit0 %x, xor0 %x, val %04x -> %04x\n", machine().describe_context(), offset, data, m_m3, m_mf, bit0, xor0, x, m_val);
	return;
}

void igs_bitswap_device::reset_w(u8 data)
{
	LOGMASKED(LOG_PROT_BITSWAP, "%s: reset bitswap - val %04x -> 0\n", machine().describe_context(), m_val);
	m_val = 0;
}

/***************************************************************************

    ---- IGS Inc/Dec Protection ----

    The chip holds an internal 4-bit value. It is manipulated by issuing commands,
    where each command is assigned a specific address range, and is triggered
    by writing to that range. Possible commands:

    - [offset 0] RESET: value = 0
    - [offset 1] DEC:   decrement value
    - [offset 3] INC:   increment value

    The protection value is read from an additional address range:

    - [offset 5] READ:  read bitswap(value). Only 4 bits are checked (0-32-1--, i.e. mask $B4)

***************************************************************************/

class igs_incdec_device : public device_t
{
public:
	igs_incdec_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u8 result_r();
	void reset_w(u8 data);
	void inc_w(u8 data);
	void dec_w(u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 m_val = 0;
};

u8 igs_incdec_device::result_r()
{
	u8 res =    (BIT(m_val, 0) << 7) |
				(BIT(m_val, 3) << 5) |
				(BIT(m_val, 2) << 4) |
				(BIT(m_val, 1) << 2) ;

	LOGMASKED(LOG_PROT_INCDEC, "%s: value read, %02x -> %02x\n", machine().describe_context(), m_val, res);
	return res;
}

void igs_incdec_device::reset_w(u8 data)
{
	m_val = 0x00;
	LOGMASKED(LOG_PROT_INCDEC, "%s: reset -> %02x\n", machine().describe_context(), m_val);
}

void igs_incdec_device::inc_w(u8 data)
{
	m_val++;
	LOGMASKED(LOG_PROT_INCDEC, "%s: inc -> %02x\n", machine().describe_context(), m_val);
}

void igs_incdec_device::dec_w(u8 data)
{
	m_val--;
	LOGMASKED(LOG_PROT_INCDEC, "%s: dec -> %02x\n", machine().describe_context(), m_val);
}

DEFINE_DEVICE_TYPE(IGS_INCDEC, igs_incdec_device, "igs_incdec", "IGS Inc/Dec Protection")

igs_incdec_device::igs_incdec_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, IGS_INCDEC, tag, owner, clock)
{ }

void igs_incdec_device::device_start()
{
	save_item(NAME(m_val));
}

void igs_incdec_device::device_reset()
{
	m_val = 0;
}

/***************************************************************************

    ---- IGS Inc Protection ----

***************************************************************************/

class igs_inc_device : public device_t
{
public:
	igs_inc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u8 result_r();
	void reset_w(u8 data);
	void inc_w(u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 m_val = 0;
};

u8 igs_inc_device::result_r()
{
	const u8 res = (BIT(~m_val, 0) | (BIT(m_val, 1) & BIT(m_val, 2))) << 5;

	LOGMASKED(LOG_PROT_INC, "%s: value read, %02x -> %02x\n", machine().describe_context(), m_val, res);
	return res;
}

void igs_inc_device::reset_w(u8 data)
{
	m_val = 0x00;
	LOGMASKED(LOG_PROT_INC, "%s: reset -> %02x\n", machine().describe_context(), m_val);
}

void igs_inc_device::inc_w(u8 data)
{
	m_val++;
	LOGMASKED(LOG_PROT_INC, "%s: inc -> %02x\n", machine().describe_context(), m_val);
}

DEFINE_DEVICE_TYPE(IGS_INC, igs_inc_device, "igs_inc", "IGS Inc Protection")

igs_inc_device::igs_inc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, IGS_INC, tag, owner, clock)
{ }

void igs_inc_device::device_start()
{
	save_item(NAME(m_val));
}

void igs_inc_device::device_reset()
{
	m_val = 0;
}


namespace {

// igs017_state

class igs017_state : public driver_device
{
public:
	igs017_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_remap_addr(-1)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_igs017_igs031(*this, "igs017_igs031")
		, m_oki(*this, "oki")
		, m_igs_mux(*this, "igs_mux")
		, m_ppi(*this, "ppi8255")
		// Optional shared pointers
		, m_decrypted_opcodes(*this, "decrypted_opcodes")
		// Optional devices
		, m_hopper(*this, "hopper")
		, m_igs_bitswap(*this, "igs_bitswap")
		, m_igs_string(*this, "igs_string")
		, m_igs_incdec(*this, "igs_incdec")
		, m_igs_inc(*this, "igs_inc")
		, m_igs022(*this,"igs022")
		// Optional I/O
		, m_io_coins(*this, "COINS")
		, m_io_joy(*this, "JOY")
		, m_io_key(*this, "KEY%u", 0U)
		, m_io_dsw(*this, "DSW%u", 1U)
		, m_lamps(*this, "lamp%u", 1U)
	{ }

	// Construct
	void base_machine_oki(machine_config &config, const XTAL &xtal_oki);
	// Z180
	void cpoker2(machine_config &config);
	void genius6(machine_config &config);
	void happyskl(machine_config &config);
	void iqblocka(machine_config &config);
	void iqblockf(machine_config &config);
	void spkrform(machine_config &config);
	void starzan(machine_config &config);
	void tarzan(machine_config &config);
	void tjsb(machine_config &config);
	// 68000
	void lhzb2(machine_config &config);
	void lhzb2a(machine_config &config);
	void mgcs(machine_config &config);
	void mgcsa(machine_config &config);
	void mgdh(machine_config &config);
	void mgdha(machine_config &config);
	void sdmg2(machine_config &config);
	void sdmg2p(machine_config &config);
	void slqz2(machine_config &config);

	// Init
	void init_cpoker2();
	void init_happyskl();
	void init_iqblocka();
	void init_lhzb2();
	void init_lhzb2a();
	void init_mgcs();
	void init_mgcsa();
	void init_mgdh();
	void init_mgdha();
	void init_sdmg2();
	void init_sdmg2p();
	void init_slqz2();
	void init_spkrform();
	void init_starzan();
	void init_tarzan();
	void init_tarzana();
	void init_tarzanc();
	void init_tjsb();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	int m_remap_addr;
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<igs017_igs031_device> m_igs017_igs031;
	required_device<okim6295_device> m_oki;
	required_device<igs_mux_device> m_igs_mux;
	required_device<i8255_device> m_ppi;

	// Optional shared pointers
	optional_shared_ptr<u8> m_decrypted_opcodes;

	// Optional devices
	optional_device<hopper_device> m_hopper;
	optional_device<igs_bitswap_device> m_igs_bitswap;
	optional_device<igs_string_device> m_igs_string;
	optional_device<igs_incdec_device> m_igs_incdec;
	optional_device<igs_inc_device> m_igs_inc;
	optional_device<igs022_device> m_igs022;

	// Optional I/O
	optional_ioport m_io_coins;
	optional_ioport m_io_joy;
	optional_ioport_array<5> m_io_key;
	optional_ioport_array<3> m_io_dsw;

	output_finder<6> m_lamps;

	u8 m_input_select;
	u8 m_dsw_select;
	u8 m_scramble_data;
	u8 m_igs022_latch;

	// IGS029 protection (communication)
	u8 m_igs029_send_data, m_igs029_recv_data;
	u8 m_igs029_send_buf[256], m_igs029_recv_buf[256];
	int m_igs029_send_len, m_igs029_recv_len;
	// IGS029 protection (mgcs)
	u32 m_igs029_mgcs_long;

	void dsw_select_w(u8 data);
	u8 dsw_r();

	template<u8 WarnMask>
	void input_select_w(u8 data)
	{
		m_input_select = data;

		if (data & WarnMask)
			logerror("%s: warning, unknown bits written in input_select_w = %02x\n", machine().describe_context(), data);
	}

	template<u8 Bit, u8 WarnMask>
	void oki_sound_bank_w(u8 data)
	{
		m_oki->set_rom_bank(BIT(data, Bit));

		if (data & WarnMask)
			logerror("%s: warning, unknown bits written in oki_sound_bank_w = %02x\n", machine().describe_context(), data);
	}

	template<u8 Bit, u8 WarnMask>
	void hopper_motor_w(u8 data)
	{
		m_hopper->motor_w(BIT(data, Bit));

		if (data & WarnMask)
			logerror("%s: warning, unknown bits written in hopper_motor_w = %02x\n", machine().describe_context(), data);
	}

	template<u8 Bit, u8 WarnMask, int Counter>
	void counter_w(u8 data)
	{
		machine().bookkeeping().coin_counter_w(Counter, BIT(data, Bit));
//      popmessage("COUNTER %d: %02X", Counter, data);
		if (data & WarnMask)
			logerror("%s: warning, unknown bits written in counter_w = %02x\n", machine().describe_context(), data);
	}

	template<u8 Bit, u8 WarnMask>
	void igs022_execute_w(u8 data)
	{
		if (!BIT(m_igs022_latch, Bit) && BIT(data, Bit)) // 0 -> 1 executes
		{
			m_igs022->handle_command();

			const u8 new_scramble_data = (m_scramble_data + 1) & 0x7f;
			LOGMASKED(LOG_PROT_IGS022, "%s: IGS022 scrambled data %02x -> %02x\n", machine().describe_context(), m_scramble_data, new_scramble_data);
			m_scramble_data = new_scramble_data;
		}

		if (data & WarnMask)
			logerror("%s: warning, unknown bits written in igs022_execute_w = %02x\n", machine().describe_context(), data);

		m_igs022_latch = data;
	}

	// Palette
	u16 lhzb2a_palette_bitswap(u16 bgr) const;
	u16 mgcs_palette_bitswap(u16 bgr) const;
	u16 slqz2_palette_bitswap(u16 bgr) const;
	u16 tarzan_palette_bitswap(u16 bgr) const;
	u16 tjsb_palette_bitswap(u16 bgr) const;

	// Game Specific I/O
	void incdec_remap_addr_w(offs_t offset, u8 data);

	void cpoker2_lamps_sound_w(u8 data);

	void happyskl_lamps_sound_w(u8 data);

	u8 lhzb2_keys_r();
	void lhzb2_keys_hopper_w(u8 data);
	u8 lhzb2_scramble_data_r();
	void lhzb2_igs022_execute_w(u8 data);

	void lhzb2a_keys_hopper_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 lhzb2a_input_r(offs_t offset);
	void lhzb2a_remap_addr_w(address_space &space, u16 data);

	u8 mgcs_keys_joy_r();
	void mgcs_keys_hopper_igs029_w(u8 data);
	u8 mgcs_scramble_data_r();
	void mgcs_scramble_data_w(u8 data);
	u8 mgcs_igs029_data_r();
	void mgcs_igs029_data_w(u8 data);

	u8 mgdh_keys_r();
	void mgdh_keys_hopper_w(u8 data);
	void mgdh_counter_w(u8 data);

	u8 sdmg2_keys_joy_r();
	void sdmg2_keys_hopper_w(u8 data);

	u8 sdmg2p_keys_r();

	void slqz2_sound_hopper_w(u8 data);
	u8 slqz2_scramble_data_r();

	void starzan_counter_w(u8 data);
	void starzan_lamps_sound_w(u8 data);

	u8 tarzan_keys_joy_r();
	void tarzan_counter_w(u8 data);
	void tarzan_dsw_sound_w(u8 data);
	void tarzan_incdec_remap_addr_w(offs_t offset, u8 data);

	// Machine
	DECLARE_MACHINE_RESET(lhzb2a);

	TIMER_DEVICE_CALLBACK_MEMBER(iqblocka_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(mgcs_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(mgdh_interrupt);

	// Decrypt
	void decrypt_program_rom(int mask, int a7, int a6, int a5, int a4, int a3, int a2, int a1, int a0);

	void mgcs_decrypt_program_rom();
	void mgcsa_decrypt_program_rom();
	void mgcs_igs029_run();
	void starzan_decrypt_program_rom();
	void tarzan_decrypt_program_rom();
	void tarzana_decrypt_program_rom();

	// ROM Patches
//  void lhzb2_patch_rom();
//  void mgcs_patch_rom();
//  void slqz2_patch_rom();
	void mgdh_patch_rom();
	void spkrform_patch_rom();

	// Memory maps
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;

	void igs_bitswap_mux_map(address_map &map) ATTR_COLD;
	void igs_fixed_data_mux_map(address_map &map) ATTR_COLD;
	void igs_string_mux_map(address_map &map) ATTR_COLD;

	void cpoker2_io(address_map &map) ATTR_COLD;
	void cpoker2_map(address_map &map) ATTR_COLD;
	void cpoker2_mux_map(address_map &map) ATTR_COLD;
	void happyskl_io(address_map &map) ATTR_COLD;
	void happyskl_mux_map(address_map &map) ATTR_COLD;
	void iqblocka_io(address_map &map) ATTR_COLD;
	void iqblocka_map(address_map &map) ATTR_COLD;
	void iqblocka_mux_map(address_map &map) ATTR_COLD;
	void iqblockf_mux_map(address_map &map) ATTR_COLD;
	void lhzb2_map(address_map &map) ATTR_COLD;
	void lhzb2_mux_map(address_map &map) ATTR_COLD;
	void lhzb2a_map(address_map &map) ATTR_COLD;
	void lhzb2a_mux_map(address_map &map) ATTR_COLD;
	void mgcs_map(address_map &map) ATTR_COLD;
	void mgcs_mux_map(address_map &map) ATTR_COLD;
	void mgcsa_map(address_map &map) ATTR_COLD;
	void mgdh_mux_map(address_map &map) ATTR_COLD;
	void mgdh_map(address_map &map) ATTR_COLD;
	void mgdha_mux_map(address_map &map) ATTR_COLD;
	void sdmg2_map(address_map &map) ATTR_COLD;
	void sdmg2_mux_map(address_map &map) ATTR_COLD;
	void sdmg2p_map(address_map &map) ATTR_COLD;
	void sdmg2p_mux_map(address_map &map) ATTR_COLD;
	void slqz2_map(address_map &map) ATTR_COLD;
	void slqz2_mux_map(address_map &map) ATTR_COLD;
	void spkrform_io(address_map &map) ATTR_COLD;
	void spkrform_mux_map(address_map &map) ATTR_COLD;
	void starzan_io(address_map &map) ATTR_COLD;
	void starzan_mux_map(address_map &map) ATTR_COLD;
	void tarzan_io(address_map &map) ATTR_COLD;
	void tarzan_mux_map(address_map &map) ATTR_COLD;
	void tjsb_io(address_map &map) ATTR_COLD;
	void tjsb_map(address_map &map) ATTR_COLD;
	void tjsb_mux_map(address_map &map) ATTR_COLD;
};

void igs017_state::machine_start()
{
	m_lamps.resolve();

	save_item(NAME(m_remap_addr));
	save_item(NAME(m_input_select));
	save_item(NAME(m_dsw_select));
	save_item(NAME(m_scramble_data));
	save_item(NAME(m_igs022_latch));

	save_item(NAME(m_igs029_send_data));
	save_item(NAME(m_igs029_recv_data));
	save_item(NAME(m_igs029_send_buf));
	save_item(NAME(m_igs029_recv_buf));
	save_item(NAME(m_igs029_send_len));
	save_item(NAME(m_igs029_recv_len));
	save_item(NAME(m_igs029_mgcs_long));
}

void igs017_state::machine_reset()
{
	m_input_select = m_dsw_select = 0xff;
	m_scramble_data = 0;

	m_igs029_send_len = m_igs029_recv_len = 0;
	m_igs029_mgcs_long = 0;
}

void igs017_state::dsw_select_w(u8 data)
{
	m_dsw_select = data;
}

u8 igs017_state::dsw_r()
{
	u8 ret = 0xff;
	if (!BIT(m_dsw_select, 0)) ret &= m_io_dsw[0]->read();
	if (!BIT(m_dsw_select, 1)) ret &= m_io_dsw[1]->read();
	if (!BIT(m_dsw_select, 2)) ret &= m_io_dsw[2]->read();
	return ret;
}

/***************************************************************************
                                Video Hardware
***************************************************************************/

void igs017_state::video_start()
{
	m_igs017_igs031->video_start();
}

// palette bitswap callbacks
u16 igs017_state::mgcs_palette_bitswap(u16 bgr) const
{
	bgr = ((bgr & 0xff00) >> 8) | ((bgr & 0x00ff) << 8);

	return bitswap<16>(bgr, 7, 8, 9, 2, 14, 3, 13, 15, 12, 11, 10, 0, 1, 4, 5, 6);
}

u16 igs017_state::lhzb2a_palette_bitswap(u16 bgr) const
{
	return bitswap<16>(bgr, 15,9,13,12,11,5,4,8,7,6,0,14,3,2,1,10);
}

u16 igs017_state::tjsb_palette_bitswap(u16 bgr) const
{
	return bitswap<16>(bgr, 15,12,3,6,10,5,4,2,9,13,8,7,11,1,0,14);
}

u16 igs017_state::slqz2_palette_bitswap(u16 bgr) const
{
	return bitswap<16>(bgr, 15,14,9,4,11,10,12,3,7,6,5,8,13,2,1,0);
}

u16 igs017_state::tarzan_palette_bitswap(u16 bgr) const
{
	return bitswap<16>(bgr, 15, 0,1,2,3,4, 10,11,12,13,14, 5,6,7,8,9);
}

/***************************************************************************
                                Decryption
***************************************************************************/

[[maybe_unused]] void save_decrypted_rom(const u8 * const rom, int rom_size)
{
	FILE *f = fopen("igs017_decrypted.bin", "wb");
	fwrite(rom, 1, rom_size, f);
	fclose(f);
}

void igs017_state::decrypt_program_rom(int mask, int a7, int a6, int a5, int a4, int a3, int a2, int a1, int a0)
{
	const int rom_size = memregion("maincpu")->bytes();
	u8 * const rom = memregion("maincpu")->base();
	std::unique_ptr<u8[]> tmp = std::make_unique<u8[]>(rom_size);

	// decrypt the program ROM

	// XOR layer
	for (int i = 0; i < rom_size; i++)
	{
		if (i & 0x2000)
		{
			if ((i & mask) == mask)
				rom[i] ^= 0x01;
		}
		else
		{
			if (i & 0x0100)
			{
				if ((i & mask) == mask)
					rom[i] ^= 0x01;
			}
			else
			{
				if (i & 0x0080)
				{
					if ((i & mask) == mask)
						rom[i] ^= 0x01;
				}
				else
				{
					if ((i & mask) != mask)
						rom[i] ^= 0x01;
				}
			}
		}
	}

	memcpy(tmp.get(),rom,rom_size);

	// address lines swap
	for (int i = 0; i < rom_size; i++)
	{
		int addr = (i & ~0xff) | bitswap<8>(i,a7,a6,a5,a4,a3,a2,a1,a0);
		rom[i] = tmp[addr];
	}

//  save_decrypted_rom(rom, rom_size);
}

// iqblocka, iqblockf, genius6

void igs017_state::init_iqblocka()
{
	decrypt_program_rom(0x11, 7, 6, 5, 4, 3, 2, 1, 0);
}

// tjsb


void igs017_state::init_tjsb()
{
	decrypt_program_rom(0x05, 7, 6, 3, 2, 5, 4, 1, 0);

	m_igs017_igs031->tjsb_decrypt_sprites();

//  m_igs_string->dump("tjsb_string.key", 0x1d24a, 0x1db4, false);
}


// mgcs

void igs017_state::mgcs_decrypt_program_rom()
{
	const int rom_size = memregion("maincpu")->bytes();
	u16 * const rom = (u16 *)memregion("maincpu")->base();

	for (int i = 0; i < rom_size / 2; i++)
	{
		u16 x = rom[i];

		// bit 0 xor layer

		if (i & 0x20/2)
		{
			if (i & 0x02/2)
			{
				x ^= 0x0001;
			}
		}

		if (!(i & 0x4000/2))
		{
			if (!(i & 0x300/2))
			{
				x ^= 0x0001;
			}
		}

		// bit 8 xor layer

		if ((i & 0x2000/2) || !(i & 0x80/2))
		{
			if (i & 0x100/2)
			{
				if (!(i & 0x20/2) || (i & 0x400/2))
				{
					x ^= 0x0100;
				}
			}
		}
		else
		{
			x ^= 0x0100;
		}

		rom[i] = x;
	}
}

void igs017_state::mgcsa_decrypt_program_rom()
{
	const int rom_size = memregion("maincpu")->bytes();
	u16 * const rom = (u16 *)memregion("maincpu")->base();

	for (int i = 0; i < rom_size / 2; i++)
	{
		u16 x = rom[i];

		if (i & 0x20 / 2)
		{
			if (i & 0x02 / 2)
			{
				x ^= 0x0001;
			}
		}

		if (!(i & 0x4000 / 2))
		{
			if (!(i & 0x300 / 2))
			{
				x ^= 0x0001;
			}
		}

		if (!(i & 0x1000 / 2) && !(i & 0x100 / 2))
		{
			if (!(i & 0x20 / 2))
			{
				x ^= 0x0100;
			}
		}
		else if (i & 0x1000 / 2)
		{
			if ((!(i & 0x100 / 2)) || ((i & 0x20 / 2) && (!(i & 0x400 / 2))))
			{
				x ^= 0x0100;
			}
		}

		rom[i] = x;
	}
}


#if 0
void igs017_state::mgcs_patch_rom()
{
	u16 * const rom = (u16 *)memregion("maincpu")->base();

	rom[0x20666/2] = 0x601e; // 020666: 671E    beq $20686 (rom check)

	// IGS029 send command
	rom[0x4dfce/2] = 0x6010; // 04DFCE: 6610    bne $4dfe0
	rom[0x4e00e/2] = 0x4e75;
	rom[0x4e036/2] = 0x6006; // 04E036: 6306    bls     $4e03e
}
#endif

void igs017_state::init_mgcs()
{
	mgcs_decrypt_program_rom();
//  mgcs_patch_rom();

	m_igs017_igs031->mgcs_decrypt_tiles();
	m_igs017_igs031->mgcs_flip_sprites(0);

//  m_igs_string->dump("mgcs_string.key", 0x1424, 0x1338, true);
}

void igs017_state::init_mgcsa()
{
	mgcsa_decrypt_program_rom();

	m_igs017_igs031->mgcs_decrypt_tiles();
	m_igs017_igs031->mgcs_flip_sprites(0);
}


// tarzan, tarzana


void igs017_state::tarzan_decrypt_program_rom()
{
	const int rom_size = memregion("maincpu")->bytes();
	u8 * const rom = memregion("maincpu")->base();

	for (int i = 0; i < rom_size; i++)
	{
		u8 x = rom[i];

		if ((i & 0x00011) == 0x00011) x ^= 0x01;
		if ((i & 0x02180) == 0x00000) x ^= 0x01;
		if ((i & 0x001a0) != 0x00020) x ^= 0x04;
		if ((i & 0x00080) != 0x00080) x ^= 0x10;
		if ((i & 0x000e0) == 0x000c0) x ^= 0x10;

		m_decrypted_opcodes[i] = x;
	}

	for (int i = 0; i < rom_size; i++)
	{
		u8 x = rom[i];

		if ((i & 0x00011) == 0x00011) x ^= 0x01;
		if ((i & 0x02180) == 0x00000) x ^= 0x01;
		if (((i & 0x00020) == 0x00020) || ((i & 0x000260) == 0x00040)) x ^= 0x04;
		if ((i & 0x001a0) != 0x00020) x ^= 0x10;

		rom[i] = x;
	}
}

void igs017_state::init_tarzanc()
{
	tarzan_decrypt_program_rom();
	m_igs017_igs031->tarzan_decrypt_tiles(1);
	m_igs017_igs031->tarzan_decrypt_sprites(0, 0);

//  m_igs_string->dump("tarzan_string.key", 0xa98a, 0xab01, false); // tarzan / tarzanc (same program rom)
}

void igs017_state::tarzana_decrypt_program_rom()
{
	const int rom_size = memregion("maincpu")->bytes();
	u8 * const rom = memregion("maincpu")->base();

	for (int i = 0; i < rom_size; i++)
	{
		u8 x = rom[i];

		if ((i & 0x00011) == 0x00011) x ^= 0x01;
		if ((i & 0x02180) == 0x00000) x ^= 0x01;
		if ((i & 0x00080) != 0x00080) x ^= 0x20;
		if ((i & 0x000e0) == 0x000c0) x ^= 0x20;
		if ((i & 0x00280) != 0x00080) x ^= 0x40;
		if ((i & 0x001a0) != 0x00020) x ^= 0x80;

		m_decrypted_opcodes[i] = x;
	}

	for (int i = 0; i < rom_size; i++) // by iq_132
	{
		u8 x = rom[i];

		if ((i & 0x00011) == 0x00011) x ^= 0x01;
		if ((i & 0x02180) == 0x00000) x ^= 0x01;
		if ((i & 0x001a0) != 0x00020) x ^= 0x20;
		if ((i & 0x00260) != 0x00200) x ^= 0x40;
		if ((i & 0x00060) != 0x00000 && (i & 0x00260) != 0x00240) x ^= 0x80;

		rom[i] = x;
	}

//  save_decrypted_rom(rom, rom_size);
//  save_decrypted_rom(m_decrypted_opcodes, rom_size);
}

void igs017_state::init_tarzan()
{
	tarzan_decrypt_program_rom();
	m_igs017_igs031->tarzan_decrypt_tiles(0);

//  m_igs_string->dump("tarzan_string.key", 0xa98a, 0xab01, false); // tarzan / tarzanc (same program rom)
}

void igs017_state::init_tarzana()
{
	tarzana_decrypt_program_rom();
	m_igs017_igs031->tarzan_decrypt_tiles(0);

//  m_igs_string->dump("tarzana_string.key", 0xaa64, 0xabdb, false); // same data as tarzan / tarzanc
}


// starzan

void igs017_state::starzan_decrypt_program_rom()
{
	const int rom_size = memregion("maincpu")->bytes();
	u8 * const rom = memregion("maincpu")->base();

	for (int i = 0; i < rom_size; i++)
	{
		u8 x = rom[i];

		if ((i & 0x00011) == 0x00011) x ^= 0x01;
		if ((i & 0x02180) == 0x00000) x ^= 0x01;
		if ((i & 0x00020) != 0x00020) x ^= 0x20;
		if ((i & 0x002a0) == 0x00220) x ^= 0x20;
		if ((i & 0x00220) != 0x00200) x ^= 0x40;
		if ((i & 0x001c0) != 0x00040) x ^= 0x80;

		m_decrypted_opcodes[i] = x;
	}

	for (int i = 0; i < rom_size; i++) // by iq_132
	{
		u8 x = rom[i];

		if ((i & 0x00011) == 0x00011) x ^= 0x01;
		if ((i & 0x02180) == 0x00000) x ^= 0x01;
		if ((i & 0x000a0) != 0x00000) x ^= 0x20;
		if ((i & 0x001a0) == 0x00000) x ^= 0x20;
		if ((i & 0x00060) != 0x00020) x ^= 0x40;
		if ((i & 0x00260) == 0x00220) x ^= 0x40;
		if ((i & 0x00020) == 0x00020) x ^= 0x80;
		if ((i & 0x001a0) == 0x00080) x ^= 0x80;

		rom[i] = x;
	}
}

void igs017_state::init_starzan()
{
	starzan_decrypt_program_rom();
	m_igs017_igs031->tarzan_decrypt_tiles(1);
	m_igs017_igs031->starzan_decrypt_sprites(0x200000, 0x400000);

//  m_igs_string->dump("starzan_string.key", 0xa86f, 0xa966, false);
}


void igs017_state::init_happyskl()
{
	const int rom_size = memregion("maincpu")->bytes();
	u8 * const rom = memregion("maincpu")->base();

	for (int i = 0; i < rom_size; i++)
	{
		u8 x = rom[i];

		if ((i & 0x00011) == 0x00011) x ^= 0x01;
		if ((i & 0x02180) == 0x00000) x ^= 0x01;
		if ((i & 0x0280) != 0x00080) x ^= 0x20;
		if ((i & 0x02a0) == 0x00280) x ^= 0x20;
		if ((i & 0x0280) != 0x00080) x ^= 0x40;
		if ((i & 0x01a0) != 0x00080) x ^= 0x80;

		m_decrypted_opcodes[i] = x;
	}

	for (int i = 0; i < rom_size; i++) // adapted from starzan
	{
		u8 x = rom[i];

		if ((i & 0x00011) == 0x00011) x ^= 0x01;
		if ((i & 0x02180) == 0x00000) x ^= 0x01;
		if ((i & 0x000a0) != 0x00000) x ^= 0x20;
		if ((i & 0x001a0) == 0x00000) x ^= 0x20;
		if ((i & 0x00060) != 0x00040) x ^= 0x40;
		if ((i & 0x00260) == 0x00240) x ^= 0x40;
		if ((i & 0x00020) == 0x00020) x ^= 0x80;
		if ((i & 0x00260) == 0x00040) x ^= 0x80;

		rom[i] = x;
	}

	m_igs017_igs031->tarzan_decrypt_tiles(1);
	m_igs017_igs031->starzan_decrypt_sprites(0x200000, 0);
}


void igs017_state::init_cpoker2()
{
	const int rom_size = memregion("maincpu")->bytes();
	u8 * const rom = memregion("maincpu")->base();

	for (int i = 0; i < rom_size; i++)
	{
		u8 x = rom[i];

		if ((i & 0x00011) == 0x00011) x ^= 0x01;
		if ((i & 0x02180) == 0x00000) x ^= 0x01;
		if ((i & 0x001a0) != 0x00020) x ^= 0x20;
		if ((i & 0x00260) != 0x00020) x ^= 0x40;
		if ((i & 0x00020) == 0x00020) x ^= 0x80;
		if ((i & 0x00260) == 0x00240) x ^= 0x80;

		// this hasn't got split data / opcodes encryption
		rom[i] = x;
	}

	m_igs017_igs031->tarzan_decrypt_tiles(1);
	m_igs017_igs031->tarzan_decrypt_sprites(0, 0);
}


// sdmg2

void igs017_state::init_sdmg2()
{
	const int rom_size = memregion("maincpu")->bytes();
	u16 * const rom = (u16 *)memregion("maincpu")->base();

	for (int i = 0; i < rom_size / 2; i++)
	{
		u16 x = rom[i];

		// bit 0 xor layer

		if (i & 0x20/2)
		{
			if (i & 0x02/2)
			{
				x ^= 0x0001;
			}
		}

		if (!(i & 0x4000/2))
		{
			if (!(i & 0x300/2))
			{
				x ^= 0x0001;
			}
		}

		// bit 9 xor layer
		if (i & 0x20000/2)
		{
			x ^= 0x0200;
		}
		else
		{
			if (!(i & 0x400/2))
			{
				x ^= 0x0200;
			}
		}

		// bit 12 xor layer
		if (i & 0x20000/2)
		{
			x ^= 0x1000;
		}

		rom[i] = x;
	}
}

// sdmg2p

void igs017_state::init_sdmg2p()
{
	const int rom_size = memregion("maincpu")->bytes();
	u16 * const rom = (u16 *)memregion("maincpu")->base();

	for (int i = 0; i < rom_size / 2; i++)
	{
		u16 x = rom[i];

		// bit 0 xor layer
		if (((i & 0x4320 / 2) == (0x0000 / 2)) || ((i & 0x4322 / 2)  == (0x0020 / 2)) || ((i & 0x0122 / 2)  == (0x0122 / 2)) || ((i & 0x0222 / 2)  == (0x0222 / 2)) || ((i & 0x4322 / 2)  == (0x4022 / 2)))
			x ^= 0x0001;

		// bit 6 xor layer
		if ((i & 0x4000 / 2) || (i & 0x0200 / 2) || ((i & 0x4b68 / 2) == (0x0048 / 2)) || ((i & 0x4b40 / 2) == (0x0840 / 2)))
			x ^= 0x0040;

		// bit 13 xor layer
		if (((i & 0x60000 / 2 ) == (0x00000 / 2)) || ((i & 0x60000 / 2 ) == (0x60000 / 2)))
			x ^= 0x2000;

		rom[i] = x;
	}

//  m_igs_string->dump("sdmg2p_string.key", 0x7f512, 0x?????, true);
}

// mgdh, mgdha

void igs017_state::init_mgdha()
{
	const int rom_size = memregion("maincpu")->bytes();
	u16 * const rom = (u16 *)memregion("maincpu")->base();

	for (int i = 0; i < rom_size / 2; i++)
	{
		u16 x = rom[i];

		if ((i & 0x20/2) && (i & 0x02/2))
		{
			if ((i & 0x300/2) || (i & 0x4000/2))
				x ^= 0x0001;
		}
		else
		{
			if (!(i & 0x300/2) && !(i & 0x4000/2))
				x ^= 0x0001;
		}

		if ((i & 0x60000/2))
			x ^= 0x0100;

		if ((i & 0x1000/2) || ((i & 0x4000/2) && (i & 0x40/2) && (i & 0x80/2)) || ((i & 0x2000/2) && (i & 0x400/2)))
			x ^= 0x0800;

		rom[i] = x;
	}

	m_igs017_igs031->mgcs_flip_sprites(0);

//  m_igs_string->dump("mgdh_string.key", 0x7b214, 0x7b128, true); // mgdh, mgdha (0x7c5ba, ???)
}

void igs017_state::mgdh_patch_rom()
{
	u16 * const rom = (u16 *)memregion("maincpu")->base();

	// game id check
	rom[0x4ad50/2] = 0x4e71;
}

void igs017_state::init_mgdh()
{
	init_mgdha();

	mgdh_patch_rom();
}


// lhzb2

#if 0
void igs017_state::lhzb2_patch_rom()
{
	u16 * const rom = (u16 *)memregion("maincpu")->base();

	// Prot. checks:
	rom[0x14786/2] = 0x6044; // 014786: 6744    beq $147cc

	// ROM check:
	rom[0x0b48a/2] = 0x604e; // 00B48A: 674E    beq $b4da
}
#endif

void igs017_state::init_lhzb2()
{
	const int rom_size = memregion("maincpu")->bytes();
	u16 * const rom = (u16 *)memregion("maincpu")->base();

	for (int i = 0; i < rom_size / 2; i++)
	{
		u16 x = rom[i];

		// bit 0 xor layer
		if (i & 0x20/2)
		{
			if (i & 0x02/2)
			{
				x ^= 0x0001;
			}
		}

		if (!(i & 0x4000/2))
		{
			if (!(i & 0x300/2))
			{
				x ^= 0x0001;
			}
		}

		// bit 13 xor layer

		if (!(i & 0x1000/2))
		{
			if (i & 0x2000/2)
			{
				if (i & 0x8000/2)
				{
					if (!(i & 0x100/2))
					{
						if (i & 0x200/2)
						{
							if (!(i & 0x40/2))
							{
								x ^= 0x2000;
							}
						}
						else
						{
							x ^= 0x2000;
						}
					}
				}
				else
				{
					if (!(i & 0x100/2))
					{
						x ^= 0x2000;
					}
				}
			}
			else
			{
				if (i & 0x8000/2)
				{
					if (i & 0x200/2)
					{
						if (!(i & 0x40/2))
						{
							x ^= 0x2000;
						}
					}
					else
					{
						x ^= 0x2000;
					}
				}
				else
				{
					x ^= 0x2000;
				}
			}
		}

		rom[i] = x;
	}

	m_igs017_igs031->lhzb2_decrypt_tiles();
	m_igs017_igs031->lhzb2_decrypt_sprites();

//  lhzb2_patch_rom();

//  m_igs_string->dump("lhzb2_string.key", 0x7b214, 0x7b128, true);
}


// lhzb2a

void igs017_state::init_lhzb2a()
{
	const int rom_size = memregion("maincpu")->bytes();
	u16 * const rom = (u16 *)memregion("maincpu")->base();

	for (int i = 0; i < rom_size / 2; i++)
	{
		u16 x = rom[i];

		// bit 0 xor layer
		if (i & 0x20/2)
		{
			if (i & 0x02/2)
			{
				x ^= 0x0001;
			}
		}

		if (!(i & 0x4000/2))
		{
			if (!(i & 0x300/2))
			{
				x ^= 0x0001;
			}
		}

		// bit 5 xor layer

		if (i & 0x4000/2)
		{
			if (i & 0x8000/2)
			{
				if (i & 0x2000/2)
				{
					if (i & 0x200/2)
					{
						if (!(i & 0x40/2) || (i & 0x800/2))
						{
							x ^= 0x0020;
						}
					}
				}
			}
			else
			{
				if (!(i & 0x40/2) || (i & 0x800/2))
				{
					x ^= 0x0020;
				}
			}
		}

		rom[i] = x;
	}

	m_igs017_igs031->lhzb2_decrypt_tiles();
	m_igs017_igs031->lhzb2_decrypt_sprites();

//  m_igs_string->dump("lhzb2a_string.key", 0x6e11c, 0x6e030, true); // same data as lhzb2
}


// slqz2

#if 0
void igs017_state::slqz2_patch_rom()
{
	u16 * const rom = (u16 *)memregion("maincpu")->base();

	// Prot. checks:
	rom[0x1489c/2] = 0x6044; // 01489C: 6744    beq $148e2

	// ROM check:
	rom[0x0b77a/2] = 0x604e; // 00B77A: 674E    beq $b7ca
}
#endif

void igs017_state::init_slqz2()
{
	const int rom_size = memregion("maincpu")->bytes();
	u16 * const rom = (u16 *)memregion("maincpu")->base();

	for (int i = 0; i < rom_size / 2; i++)
	{
		u16 x = rom[i];

		// bit 0 xor layer

		if (i & 0x20/2)
		{
			if (i & 0x02/2)
			{
				x ^= 0x0001;
			}
		}

		if (!(i & 0x4000/2))
		{
			if (!(i & 0x300/2))
			{
				x ^= 0x0001;
			}
		}

		// bit 14 xor layer

		if (i & 0x1000/2)
		{
			if (i & 0x800/2)
			{
				x ^= 0x4000;
			}
			else
			{
				if (i & 0x200/2)
				{
					if (!(i & 0x100/2))
					{
						if (i & 0x40/2)
						{
							x ^= 0x4000;
						}
					}
				}
				else
				{
					x ^= 0x4000;
				}
			}
		}
		else
		{
			if (i & 0x800/2)
			{
				x ^= 0x4000;
			}
			else
			{
				if (!(i & 0x100/2))
				{
					if (i & 0x40/2)
					{
						x ^= 0x4000;
					}
				}
			}
		}

		rom[i] = x;
	}

	m_igs017_igs031->slqz2_decrypt_tiles();
	m_igs017_igs031->lhzb2_decrypt_sprites();

//  slqz2_patch_rom();

//  m_igs_string->dump("slqz2_string.key", 0x7b214, 0x7b128, true);
}


// spkrform

void igs017_state::spkrform_patch_rom()
{
	u8 * const rom = memregion("maincpu")->base();

	rom[0x32ea9] = 0; // enable poker ($e9be = 0)
	rom[0x32ef9] = 0; // start with poker ($e9bf = 0)
}

void igs017_state::init_spkrform()
{
	decrypt_program_rom(0x14, 7, 6, 5, 4, 3, 0, 1, 2);

	m_igs017_igs031->spkrform_decrypt_sprites();

	spkrform_patch_rom();

//  m_igs_string->dump("spkrform_string.key", 0x9dec, 0x9d00, false);
}

/***************************************************************************
                                Memory Maps
***************************************************************************/

// Common MUX maps

// 0x03 r, 0x40 - 0x47 w, 0x48 w, 0x50 w, 0x80 - 0x87 w, 0xa0 w
void igs017_state::igs_bitswap_mux_map(address_map &map)
{
	map(0x03, 0x03).r(m_igs_bitswap, FUNC(igs_bitswap_device::result_r));
	map(0x40, 0x40).w(m_igs_bitswap, FUNC(igs_bitswap_device::word_w));
	map(0x41, 0x47).nopw();
	map(0x48, 0x48).w(m_igs_bitswap, FUNC(igs_bitswap_device::mode_f_w));
	map(0x50, 0x50).w(m_igs_bitswap, FUNC(igs_bitswap_device::mode_3_w));
	map(0x80, 0x87).w(m_igs_bitswap, FUNC(igs_bitswap_device::do_bitswap_w));
	map(0xa0, 0xa0).w(m_igs_bitswap, FUNC(igs_bitswap_device::reset_w));
}

// 0x05 r, 0x20 - 0x27 w, 0x40 r
void igs017_state::igs_string_mux_map(address_map &map)
{
	map(0x05, 0x05).r(m_igs_string, FUNC(igs_string_device::result_r));
	map(0x20, 0x27).w(m_igs_string, FUNC(igs_string_device::do_bitswap_w));
	map(0x40, 0x40).r(m_igs_string, FUNC(igs_string_device::advance_string_offs_r));
}

// 0x20 - 0x34: read fixed data
void igs017_state::igs_fixed_data_mux_map(address_map &map)
{
	map(0x20, 0x34).rom().region("igs_fixed_data", 0);
	map(0x23, 0x23).unmapr();
	map(0x29, 0x29).unmapr();
	map(0x2f, 0x2f).unmapr();
}


// iqblocka, iqblockf, genius6

void igs017_state::iqblocka_map(address_map &map)
{
	map(0x00000, 0x0dfff).rom();
	map(0x0e000, 0x0efff).ram();
	map(0x0f000, 0x0ffff).ram();
	map(0x10000, 0x3ffff).rom();
}

void igs017_state::decrypted_opcodes_map(address_map &map)
{
	map(0x00000, 0x3ffff).readonly().share(m_decrypted_opcodes);
}

void igs017_state::incdec_remap_addr_w(offs_t offset, u8 data)
{
	if (offset == 0)
	{
		// Unmap previous address ranges
		if (m_remap_addr != -1)
		{
			address_space &prg_space = m_maincpu->space(AS_PROGRAM);
			prg_space.unmap_write(m_remap_addr + 0x0, m_remap_addr + 0x0);
			prg_space.unmap_write(m_remap_addr + 0x1, m_remap_addr + 0x1);
			prg_space.unmap_write(m_remap_addr + 0x3, m_remap_addr + 0x3);
			prg_space.unmap_read (m_remap_addr + 0x5, m_remap_addr + 0x5);

			LOGMASKED(LOG_PROT_REMAP, "%s: incdec protection unmapped from %04x\n", machine().describe_context(), m_remap_addr);
		}

		m_remap_addr = (m_remap_addr & 0xff00) | data;
	}
	else
	{
		m_remap_addr = (m_remap_addr & 0x00ff) | (data << 8);

		// Add new memory ranges
		address_space &prg_space = m_maincpu->space(AS_PROGRAM);
		prg_space.install_write_handler(m_remap_addr + 0x0, m_remap_addr + 0x0, write8smo_delegate(*m_igs_incdec, FUNC(igs_incdec_device::reset_w)));
		prg_space.install_write_handler(m_remap_addr + 0x1, m_remap_addr + 0x1, write8smo_delegate(*m_igs_incdec, FUNC(igs_incdec_device::dec_w)));
		prg_space.install_write_handler(m_remap_addr + 0x3, m_remap_addr + 0x3, write8smo_delegate(*m_igs_incdec, FUNC(igs_incdec_device::inc_w)));
		prg_space.install_read_handler (m_remap_addr + 0x5, m_remap_addr + 0x5, read8smo_delegate (*m_igs_incdec, FUNC(igs_incdec_device::result_r)));

		LOGMASKED(LOG_PROT_REMAP, "%s: incdec protection remapped at %04x\n", machine().describe_context(), m_remap_addr);
	}
}

void igs017_state::iqblocka_io(address_map &map)
{
	map(0x0000, 0x7fff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write));
	map(0x0000, 0x003f).ram(); // internal regs

	map(0x2010, 0x2011).w(FUNC(igs017_state::incdec_remap_addr_w));

	map(0x8000, 0x8000).w (m_igs_mux, FUNC(igs_mux_device::address_w));
	map(0x8001, 0x8001).rw(m_igs_mux, FUNC(igs_mux_device::data_r), FUNC(igs_mux_device::data_w));

	map(0x9000, 0x9000).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0xa000, 0xa000).portr("BUTTONS");

	map(0xb000, 0xb001).w("ymsnd", FUNC(ym2413_device::write));
}

void igs017_state::iqblocka_mux_map(address_map &map)
{
	map(0x00, 0x00).portr("PLAYER1").w(NAME((&igs017_state::counter_w<7, 0x7f, 0>))); // coin in
	map(0x01, 0x01).portr("PLAYER2");
	map(0x02, 0x02).portr("COINS");

	igs_bitswap_mux_map(map); // 0x03 r, 0x40 - 0x47 w, 0x48 w, 0x50 w, 0x80 - 0x87 w, 0xa0 w

	igs_fixed_data_mux_map(map); // 0x20 - 0x34: read fixed data
}

void igs017_state::iqblockf_mux_map(address_map &map)
{
	iqblocka_mux_map(map);

	map(0x01, 0x01).w(NAME((&igs017_state::counter_w<7, 0x7f, 1>))); // coin out (in gambling mode, only iqblockf/genius6)
}

// starzan

void igs017_state::starzan_io(address_map &map)
{
	map(0x0000, 0x7fff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write));
	map(0x0000, 0x003f).ram(); // internal regs

	map(0x2010, 0x2011).w(FUNC(igs017_state::incdec_remap_addr_w));

	map(0x8000, 0x8000).w (m_igs_mux, FUNC(igs_mux_device::address_w));
	map(0x8001, 0x8001).rw(m_igs_mux, FUNC(igs_mux_device::data_r), FUNC(igs_mux_device::data_w));

	map(0x9000, 0x9000).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

// cpoker2, happyksl, starzan
void igs017_state::starzan_counter_w(u8 data)
{
	//                                        BIT(data, 0)   // always on in cpoker2/happyskl?
	m_hopper->motor_w(                        BIT(data, 1)); // hopper
	//                                        BIT(data, 2)   // unused?
	machine().bookkeeping().coin_counter_w(0, BIT(data, 3)); // key out
	machine().bookkeeping().coin_counter_w(1, BIT(data, 4)); // payout
	machine().bookkeeping().coin_counter_w(2, BIT(data, 5)); // coin C
	machine().bookkeeping().coin_counter_w(3, BIT(data, 6)); // key in
	machine().bookkeeping().coin_counter_w(4, BIT(data, 7)); // coin A
//  popmessage("COUNTER %02X", data);
	if (BIT(data, 2))
		logerror("%s: warning, unknown bits written in counter_w = %02x\n", machine().describe_context(), data);
}

void igs017_state::starzan_lamps_sound_w(u8 data)
{
	m_lamps[0] = BIT(data, 2); // stop 1
	m_lamps[1] = BIT(data, 0); // stop 2
	m_lamps[2] = BIT(data, 4); // stop 3
	m_lamps[3] = BIT(data, 1); // stop 4
	m_lamps[4] = BIT(data, 3); // bet/stop
	m_lamps[5] = BIT(data, 5); // start
//               BIT(data, 6); // unused?
	m_oki->set_rom_bank(BIT(data, 7));

//  popmessage("LAMPS/SOUND %02X", data);

	if (BIT(data, 6))
		logerror("%s: warning, unknown bits written in lamps_sound_w = %02x\n", machine().describe_context(), data);
}

void igs017_state::starzan_mux_map(address_map &map)
{
	map(0x00, 0x00).w(FUNC(igs017_state::starzan_counter_w));
	map(0x01, 0x01).portr("PLAYER2");
	map(0x02, 0x02).w(FUNC(igs017_state::starzan_lamps_sound_w));
	map(0x03, 0x03).w(FUNC(igs017_state::dsw_select_w));

	igs_string_mux_map(map); // 0x05 r, 0x20 - 0x27 w, 0x40 r
}


// happyksl

void igs017_state::happyskl_io(address_map &map)
{
	map(0x0000, 0x7fff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write));
	map(0x0000, 0x003f).ram(); // internal regs

	map(0x8000, 0x8000).w (m_igs_mux, FUNC(igs_mux_device::address_w));
	map(0x8001, 0x8001).rw(m_igs_mux, FUNC(igs_mux_device::data_r), FUNC(igs_mux_device::data_w));

	map(0x9000, 0x9000).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void igs017_state::happyskl_lamps_sound_w(u8 data)
{
	m_lamps[0] = BIT(data, 4); // hold 1
	m_lamps[1] = BIT(data, 0); // hold 2
	m_lamps[2] = BIT(data, 1); // hold 3
	m_lamps[3] = BIT(data, 2); // hold 4
	m_lamps[4] = BIT(data, 3); // hold 5
	m_lamps[5] = BIT(data, 5); // start
//               BIT(data, 6); // unused?
	m_oki->set_rom_bank(BIT(data, 7));

//  popmessage("LAMPS/SOUND %02X", data);

	if (BIT(data, 6))
		logerror("%s: warning, unknown bits written in lamps_sound_w = %02x\n", machine().describe_context(), data);
}

void igs017_state::happyskl_mux_map(address_map &map)
{
	map(0x00, 0x00).w(FUNC(igs017_state::starzan_counter_w));
	map(0x01, 0x01).portr("PLAYER2");
	map(0x02, 0x02).w(FUNC(igs017_state::happyskl_lamps_sound_w));
	map(0x03, 0x03).w(FUNC(igs017_state::dsw_select_w));
}


// cpoker2

void igs017_state::cpoker2_map(address_map &map)
{
	iqblocka_map(map);

	map(0x0a400, 0x0a400).w(m_igs_inc, FUNC(igs_inc_device::inc_w));
	map(0x0a420, 0x0a420).w(m_igs_inc, FUNC(igs_inc_device::reset_w));
	map(0x0a460, 0x0a460).r(m_igs_inc, FUNC(igs_inc_device::result_r));
}

void igs017_state::cpoker2_io(address_map &map)
{
	map(0x0000, 0x7fff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write));
	map(0x0000, 0x003f).ram(); // internal regs

	map(0x2010, 0x2011).w(FUNC(igs017_state::incdec_remap_addr_w));

	map(0x8000, 0x8000).w (m_igs_mux, FUNC(igs_mux_device::address_w));
	map(0x8001, 0x8001).rw(m_igs_mux, FUNC(igs_mux_device::data_r), FUNC(igs_mux_device::data_w));

	map(0x9000, 0x9000).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void igs017_state::cpoker2_lamps_sound_w(u8 data)
{
	m_lamps[0] = BIT(data, 0); // hold 1
	m_lamps[1] = BIT(data, 1); // hold 2
	m_lamps[2] = BIT(data, 4); // hold 3
	m_lamps[3] = BIT(data, 2); // hold 4
	m_lamps[4] = BIT(data, 3); // hold 5
	m_lamps[5] = BIT(data, 5); // start
//               BIT(data, 6); // unused?
	m_oki->set_rom_bank(BIT(data, 7));

//  popmessage("LAMPS/SOUND %02X", data);

	if (BIT(data, 6))
		logerror("%s: warning, unknown bits written in lamps_sound_w = %02x\n", machine().describe_context(), data);
}

void igs017_state::cpoker2_mux_map(address_map &map)
{
	map(0x00, 0x00).w(FUNC(igs017_state::starzan_counter_w));
	map(0x01, 0x01).portr("PLAYER2");
	map(0x02, 0x02).w(FUNC(igs017_state::cpoker2_lamps_sound_w));
	map(0x03, 0x03).w(FUNC(igs017_state::dsw_select_w));
}


// tarzan

void igs017_state::tarzan_incdec_remap_addr_w(offs_t offset, u8 data)
{
	incdec_remap_addr_w(offset, data ^ ((offset == 1) ? 0x40 : 0));
}

void igs017_state::tarzan_io(address_map &map)
{
	starzan_io(map);

	map(0x2010, 0x2011).w(FUNC(igs017_state::tarzan_incdec_remap_addr_w));
}

u8 igs017_state::tarzan_keys_joy_r()
{
	if (BIT(m_input_select, 3, 5) == 0x1f)  return m_io_joy->read(); // f8 (joystick mode)

	u8 ret = 0xff;
	if (!BIT(m_input_select, 3))    ret &= m_io_key[0]->read();      // f0 (keyboard mode)
	if (!BIT(m_input_select, 4))    ret &= m_io_key[1]->read();      // e8 ""
	if (!BIT(m_input_select, 5))    ret &= m_io_key[2]->read();      // d8 ""
	if (!BIT(m_input_select, 6))    ret &= m_io_key[3]->read();      // b8 ""
	if (!BIT(m_input_select, 7))    ret &= m_io_key[4]->read();      // 78 "" (unused)
	return ret;
}

void igs017_state::tarzan_counter_w(u8 data)
{
	m_hopper->motor_w(                        BIT(data, 1)); // hopper
	machine().bookkeeping().coin_counter_w(0, BIT(data, 2)); // key out
	machine().bookkeeping().coin_counter_w(1, BIT(data, 3)); // key in
//  popmessage("COUNTER %02X", data);
	if (data & ~0x0c)
		logerror("%s: warning, unknown bits written in counter_w = %02x\n", machine().describe_context(), data);
}

void igs017_state::tarzan_dsw_sound_w(u8 data)
{
	// bits 0,1,2: DSW select
	// bit  7: Sound bank
	m_dsw_select = data;
	m_oki->set_rom_bank(BIT(data, 7));
//  popmessage("DSW %02X", data);
	if (data & ~0x87)
		logerror("%s: warning, unknown bits written in dsw_sound_w = %02x\n", machine().describe_context(), data);
}

void igs017_state::tarzan_mux_map(address_map &map)
{
	map(0x00, 0x00).w(FUNC(igs017_state::input_select_w<0x07>));
	map(0x01, 0x01).w(FUNC(igs017_state::tarzan_counter_w));
	map(0x02, 0x02).r(FUNC(igs017_state::dsw_r));
	map(0x03, 0x03).w(FUNC(igs017_state::tarzan_dsw_sound_w));

	igs_string_mux_map(map); // 0x05 r, 0x20 - 0x27 w, 0x40 r
}


// mgcs

// IGS029 appears to be an MCU that receives commands (write port with value, read port, etc.)
// Sound banking and DSW are accessed through it. It also performs some game specific calculations.
void igs017_state::mgcs_igs029_run()
{
	LOGMASKED(LOG_PROT_IGS029, "%s: running igs029 command ", machine().describe_context());
	for (int i = 0; i < m_igs029_send_len; i++)
		LOGMASKED(LOG_PROT_IGS029, "%02x ", m_igs029_send_buf[i]);

	if (m_igs029_send_buf[0] == 0x05 && m_igs029_send_buf[1] == 0x5a) // 'Z'
	{
		u8 data = m_igs029_send_buf[2];
		u8 port = m_igs029_send_buf[3];

		LOGMASKED(LOG_PROT_IGS029, "PORT %02x = %02x\n", port, data);

		switch (port)
		{
			case 0x01:
				m_oki->set_rom_bank(                       BIT(data, 4)); // oki
				machine().bookkeeping().coin_counter_w(0, !BIT(data, 5)); // coin in
				machine().bookkeeping().coin_counter_w(1, !BIT(data, 6)); // coin out

//              popmessage("PORT1 %02X", data);

				if (data & ~0x70)
					logerror("%s: warning, unknown bits written in port %02x = %02x\n", machine().describe_context(), port, data);

				break;

			case 0x03:
				m_dsw_select = data;

//              popmessage("PORT3 %02X", data);

				if (data & ~0x03)
					logerror("%s: warning, unknown bits written in port %02x = %02x\n", machine().describe_context(), port, data);

				break;

			default:
				logerror("%s: warning, unknown port %02x written with %02x\n", machine().describe_context(), port, data);
		}

		m_igs029_recv_len = 0;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x01;
	}
	else if (m_igs029_send_buf[0] == 0x03 && m_igs029_send_buf[1] == 0x55) // 'U'
	{
		LOGMASKED(LOG_PROT_IGS029, "MIN BET?\n");

		// No inputs. Returns 1 long

		u8 min_bets[4] = {1, 2, 3, 5};

		m_igs029_recv_len = 0;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x00;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x00;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x00;
		m_igs029_recv_buf[m_igs029_recv_len++] = min_bets[ (~m_io_dsw[1]->read()) & 3 ];
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x05;
	}
	else if (m_igs029_send_buf[0] == 0x03 && m_igs029_send_buf[1] == 0x39) // '9'
	{
		LOGMASKED(LOG_PROT_IGS029, "READ DSW\n");

		u8 ret = 0xff;
		if      (!BIT(m_dsw_select, 0)) ret &= m_io_dsw[0]->read();
		else if (!BIT(m_dsw_select, 1)) ret &= m_io_dsw[1]->read();
		else logerror("%s: warning, reading dsw with dsw_select = %02x\n", machine().describe_context(), m_dsw_select);

		m_igs029_recv_len = 0;
		m_igs029_recv_buf[m_igs029_recv_len++] = ret;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x02;
	}
	else if (m_igs029_send_buf[0] == 0x07 && m_igs029_send_buf[1] == 0x2c) // ','
	{
		LOGMASKED(LOG_PROT_IGS029, "?? (2C)\n"); // ??

		// 4 inputs. Returns 1 long

		// called when pressing start without betting.
		// Returning high values produces an overflow causing a division by 0, and then the game hangs.
		m_igs029_recv_len = 0;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x00;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x00;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x00;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x01; // ??
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x05;
	}
	else if (m_igs029_send_buf[0] == 0x07 && m_igs029_send_buf[1] == 0x15)
	{
		LOGMASKED(LOG_PROT_IGS029, "SET LONG\n");

		m_igs029_mgcs_long = get_u32be(&m_igs029_send_buf[2]);

		m_igs029_recv_len = 0;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x01;
	}
	else if (m_igs029_send_buf[0] == 0x03 && m_igs029_send_buf[1] == 0x04)
	{
		LOGMASKED(LOG_PROT_IGS029, "GET LONG\n");

		m_igs029_recv_len = 0;
		put_u32le(&m_igs029_recv_buf[m_igs029_recv_len], m_igs029_mgcs_long);
		m_igs029_recv_len += 4;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x05;
	}
	else
	{
		LOGMASKED(LOG_PROT_IGS029, "UNKNOWN\n");

		m_igs029_recv_len = 0;
		m_igs029_recv_buf[m_igs029_recv_len++] = 0x01;
	}

	m_igs029_send_len = 0;
}

void igs017_state::mgcs_keys_hopper_igs029_w(u8 data)
{
	// 7654 3--- Keys
	// ---- -2-- IRQ on IGS029
	// ---- --1-
	// ---- ---0 Hopper Motor
	const bool igs029_irq = !BIT(m_input_select, 2) && BIT(data, 2); // 0 -> 1
	m_input_select = data;
	m_hopper->motor_w(BIT(data, 0));

	if (igs029_irq)
	{
		if (!m_igs029_recv_len)
		{
			// SEND
			if (m_igs029_send_len < sizeof(m_igs029_send_buf))
				m_igs029_send_buf[m_igs029_send_len++] = m_igs029_send_data;

			LOGMASKED(LOG_PROT_IGS029, "%s: igs029 send", machine().describe_context());
			for (int i = 0; i < m_igs029_send_len; i++)
				LOGMASKED(LOG_PROT_IGS029, " %02x", m_igs029_send_buf[i]);
			LOGMASKED(LOG_PROT_IGS029, "\n");

			if (m_igs029_send_buf[0] == m_igs029_send_len)
				mgcs_igs029_run();
		}

		if (m_igs029_recv_len)
		{
			// RECV
			LOGMASKED(LOG_PROT_IGS029, "%s: igs029 recv", machine().describe_context());
			for (int i = 0; i < m_igs029_recv_len; i++)
				LOGMASKED(LOG_PROT_IGS029, " %02x", m_igs029_recv_buf[i]);
			LOGMASKED(LOG_PROT_IGS029, "\n");

			if (m_igs029_recv_len)
				--m_igs029_recv_len;

			m_igs029_recv_data = m_igs029_recv_buf[m_igs029_recv_len];
		}
	}

	if (m_input_select & ~0xfd)
		logerror("%s: warning, unknown bits written in input_select = %02x\n", machine().describe_context(), m_input_select);
}

void igs017_state::mgcs_scramble_data_w(u8 data)
{
	m_scramble_data = data;
	LOGMASKED(LOG_PROT_SCRAMBLE, "%s: writing scrambled data %02x to igs_mux\n", machine().describe_context(), data);
}

u8 igs017_state::mgcs_scramble_data_r()
{
	u8 ret = bitswap<8>( (bitswap<8>(m_scramble_data, 0,1,2,3,4,5,6,7) + 1) & 3, 4,5,6,7, 0,1,2,3 );
	LOGMASKED(LOG_PROT_SCRAMBLE, "%s: reading scrambled data %02x from igs_mux\n", machine().describe_context(), ret);
	return ret;
}

u8 igs017_state::mgcs_igs029_data_r()
{
	u8 ret = m_igs029_recv_data;
	LOGMASKED(LOG_PROT_IGS029, "%s: reading IGS029 data %02x from igs_mux\n", machine().describe_context(), ret);
	return ret;
}

void igs017_state::mgcs_igs029_data_w(u8 data)
{
	m_igs029_send_data = data;
	LOGMASKED(LOG_PROT_IGS029, "%s: writing %02x to igs_mux\n", machine().describe_context(), data & 0xff);
}

u8 igs017_state::mgcs_keys_joy_r()
{
	u8 ret = 0xff;
	if (!BIT(m_input_select, 0, 3)) ret &= (m_io_joy->read() | 0x3f); // f8 (joystick mode, top 2 bits)
	if (!BIT(m_input_select, 3))    ret &= m_io_key[0]->read();       // f7 (keyboard mode)
	if (!BIT(m_input_select, 4))    ret &= m_io_key[1]->read();       // ef ""
	if (!BIT(m_input_select, 5))    ret &= m_io_key[2]->read();       // df ""
	if (!BIT(m_input_select, 6))    ret &= m_io_key[3]->read();       // bf ""
	if (!BIT(m_input_select, 7))    ret &= m_io_key[4]->read();       // f7 ""
	return ret;
}

void igs017_state::mgcs_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x300000, 0x303fff).ram();

	map(0x49c000, 0x49c001).nopr().w(m_igs_mux, FUNC(igs_mux_device::address_w)).umask16(0x00ff); // clr.w dummy read
	map(0x49c002, 0x49c003).rw(m_igs_mux, FUNC(igs_mux_device::data_r), FUNC(igs_mux_device::data_w)).umask16(0x00ff);

	map(0xa00000, 0xa0ffff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write)).umask16(0x00ff);

	map(0xa12001, 0xa12001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	// oki banking through protection (code at $1a350)
}

void igs017_state::mgcs_mux_map(address_map &map)
{
	map(0x00, 0x00).lr8(NAME([this](){ return m_input_select | 0x02; })).w(FUNC(igs017_state::mgcs_keys_hopper_igs029_w));
	map(0x01, 0x01).r(FUNC(igs017_state::mgcs_scramble_data_r)).w(FUNC(igs017_state::mgcs_scramble_data_w));
	map(0x02, 0x02).r(FUNC(igs017_state::mgcs_igs029_data_r));
	map(0x03, 0x03).w(FUNC(igs017_state::mgcs_igs029_data_w));

	igs_string_mux_map(map); // 0x05 r, 0x20 - 0x27 w, 0x40 r
}

void igs017_state::mgcsa_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram();

	map(0x49c000, 0x49c001).nopr().w(m_igs_mux, FUNC(igs_mux_device::address_w)).umask16(0x00ff); // clr.w dummy read
	map(0x49c002, 0x49c003).rw(m_igs_mux, FUNC(igs_mux_device::data_r), FUNC(igs_mux_device::data_w)).umask16(0x00ff);

	map(0x900000, 0x90ffff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write)).umask16(0x00ff);

	map(0x912001, 0x912001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	// oki banking through protection (code at $1a350)
}


// sdmg2

void igs017_state::sdmg2_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

	// incdec protection
	map(0x002001, 0x002001).w(m_igs_incdec, FUNC(igs_incdec_device::reset_w));
	map(0x002003, 0x002003).w(m_igs_incdec, FUNC(igs_incdec_device::dec_w));
	map(0x002007, 0x002007).w(m_igs_incdec, FUNC(igs_incdec_device::inc_w));
	map(0x00200b, 0x00200b).r(m_igs_incdec, FUNC(igs_incdec_device::result_r));

	map(0x1f0000, 0x1fffff).ram();

	map(0x200000, 0x20ffff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write)).umask16(0x00ff);

	map(0x210001, 0x210001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x300000, 0x300001).nopr().w(m_igs_mux, FUNC(igs_mux_device::address_w)).umask16(0x00ff); // clr.w dummy read
	map(0x300002, 0x300003).rw(m_igs_mux, FUNC(igs_mux_device::data_r), FUNC(igs_mux_device::data_w)).umask16(0x00ff);
}

u8 igs017_state::sdmg2_keys_joy_r()
{
	if (BIT(m_input_select, 0, 5) == 0x1f) return m_io_joy->read();    // 1f/uninitialized in test screen (joystick mode)

	u8 ret = 0xff;
	if (!BIT(m_input_select, 0))           ret &= m_io_key[0]->read(); // 1e (keyboard mode)
	if (!BIT(m_input_select, 1))           ret &= m_io_key[1]->read(); // 1d ""
	if (!BIT(m_input_select, 2))           ret &= m_io_key[2]->read(); // 1b ""
	if (!BIT(m_input_select, 3))           ret &= m_io_key[3]->read(); // 17 ""
	if (!BIT(m_input_select, 4))           ret &= m_io_key[4]->read(); // 0f ""
	return ret;
}

void igs017_state::sdmg2_keys_hopper_w(u8 data)
{
	m_input_select =                          BIT(data, 0, 5); // keys
	machine().bookkeeping().coin_counter_w(0, BIT(data, 5));   // coin in
	machine().bookkeeping().coin_counter_w(1, BIT(data, 6));   // coin out
	m_hopper->motor_w(                        BIT(data, 7));   // hopper
}

void igs017_state::sdmg2_mux_map(address_map &map)
{
	map(0x00, 0x00).portr("COINS");
	map(0x01, 0x01).w(FUNC(igs017_state::sdmg2_keys_hopper_w));
	map(0x02, 0x02).r(FUNC(igs017_state::sdmg2_keys_joy_r)).w(NAME((&igs017_state::oki_sound_bank_w<7, 0x7f>)));
}

// mgdh, mgdha

void igs017_state::mgdh_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x600000, 0x603fff).ram();

	map(0x876000, 0x876001).nopr().w(m_igs_mux, FUNC(igs_mux_device::address_w)).umask16(0x00ff); // clr.w dummy read
	map(0x876002, 0x876003).rw(m_igs_mux, FUNC(igs_mux_device::data_r), FUNC(igs_mux_device::data_w)).umask16(0x00ff);

	map(0xa00000, 0xa0ffff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write)).umask16(0x00ff);

	map(0xa10001, 0xa10001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

u8 igs017_state::mgdh_keys_r()
{
	u8 ret = 0xff;
	if (!BIT(m_input_select, 2))    ret &= m_io_key[0]->read(); // f8 (keyboard mode / joystick mode)
	if (!BIT(m_input_select, 3))    ret &= m_io_key[1]->read(); // f4 (keyboard mode)
	if (!BIT(m_input_select, 4))    ret &= m_io_key[2]->read(); // ec ""
	if (!BIT(m_input_select, 5))    ret &= m_io_key[3]->read(); // dc ""
	if (!BIT(m_input_select, 6))    ret &= m_io_key[4]->read(); // bc ""
	return ret;
}

void igs017_state::mgdh_keys_hopper_w(u8 data)
{
	m_input_select = data;
	m_hopper->motor_w(BIT(data, 0));

	if (m_input_select & ~0xfd)
		logerror("%s: warning, unknown bits written in keys_hopper_w = %02x\n", machine().describe_context(), data);
}

void igs017_state::mgdh_counter_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 6)); // coin out
	machine().bookkeeping().coin_counter_w(1, BIT(data, 7)); // coin in

	if (data & ~0xc0)
		logerror("%s: warning, unknown bits written in counter_w = %02x\n", machine().describe_context(), data);
}

void igs017_state::mgdha_mux_map(address_map &map)
{
	map(0x00, 0x00).r(FUNC(igs017_state::mgdh_keys_r)).w(FUNC(igs017_state::mgdh_counter_w));
	map(0x01, 0x01).portr("BUTTONS").w(FUNC(igs017_state::mgdh_keys_hopper_w));
	map(0x02, 0x02).lr8(NAME([this](){ return bitswap<8>(m_io_dsw[1]->read(), 0,1,2,3,4,5,6,7); }));
	map(0x03, 0x03).portr("COINS").w(NAME((&igs017_state::oki_sound_bank_w<6, 0x3f>))); // bit 7? always on
}

void igs017_state::mgdh_mux_map(address_map &map)
{
	mgdha_mux_map(map);

//  igs_string_mux_map(map); // 0x05 r, 0x20 - 0x27 w, 0x40 r (actually unused except for the game id check?)
}

// sdmg2p

u8 igs017_state::sdmg2p_keys_r()
{
	u8 ret = 0xff;
	if (!BIT(m_input_select, 2))    ret &= m_io_key[0]->read();
	if (!BIT(m_input_select, 3))    ret &= m_io_key[1]->read();
	if (!BIT(m_input_select, 4))    ret &= m_io_key[2]->read();
	if (!BIT(m_input_select, 5))    ret &= m_io_key[3]->read();
	if (!BIT(m_input_select, 6))    ret &= m_io_key[4]->read();
	return bitswap<8>(ret, 5, 4, 3, 2, 1, 0, 7, 6);
}

void igs017_state::sdmg2p_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

	map(0x100000, 0x103fff).ram();

	map(0x38d000, 0x38d001).nopr().w(m_igs_mux, FUNC(igs_mux_device::address_w)).umask16(0x00ff); // clr.w dummy read
	map(0x38d002, 0x38d003).rw(m_igs_mux, FUNC(igs_mux_device::data_r), FUNC(igs_mux_device::data_w)).umask16(0x00ff);

	map(0xb00000, 0xb0ffff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write)).umask16(0x00ff);

	map(0xb10001, 0xb10001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void igs017_state::sdmg2p_mux_map(address_map &map) // TODO: hopper motor w
{
	map.unmap_value_high();
	map(0x00, 0x00).r(FUNC(igs017_state::sdmg2p_keys_r));
	map(0x01, 0x01).portr("JOY");
	map(0x02, 0x02).portr("BUTTONS").w(FUNC(igs017_state::mgdh_keys_hopper_w));
	map(0x03, 0x03).portr("COINS").w(FUNC(igs017_state::mgdh_counter_w));

	igs_string_mux_map(map);
}

// tjsb

void igs017_state::tjsb_map(address_map &map)
{
	map(0x00000, 0x0dfff).rom();

	map(0x0e000, 0x0e000).w (m_igs_mux, FUNC(igs_mux_device::address_w));
	map(0x0e001, 0x0e001).rw(m_igs_mux, FUNC(igs_mux_device::data_r), FUNC(igs_mux_device::data_w));

	map(0x0e002, 0x0efff).ram();
	map(0x0f000, 0x0ffff).ram();
	map(0x10000, 0x3ffff).rom();
}

void igs017_state::tjsb_io(address_map &map)
{
	map(0x0000, 0x7fff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write));
	map(0x0000, 0x003f).ram(); // internal regs

	map(0x9000, 0x9000).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0xb000, 0xb001).w("ymsnd", FUNC(ym2413_device::write));
}

void igs017_state::tjsb_mux_map(address_map &map)
{
	map(0x00, 0x00).portr("PLAYER1").w(NAME((&igs017_state::counter_w<7, 0x7f, 0>))); // coin in
	map(0x01, 0x01).portr("PLAYER2").w(NAME((&igs017_state::counter_w<0, 0xfe, 1>))); // coin out
	map(0x02, 0x02).portr("COINS").w(NAME((&igs017_state::oki_sound_bank_w<4, 0xcf>))); // oki bank (0x20/0x30)
	map(0x03, 0x03).portr("BUTTONS").w(NAME((&igs017_state::hopper_motor_w<6, 0xbf>)));

	// used ?
	igs_string_mux_map(map); // 0x05 r, 0x20 - 0x27 w, 0x40 r
}


// spkrform

void igs017_state::spkrform_io(address_map &map)
{
	map(0x0000, 0x7fff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write));
	map(0x0000, 0x003f).ram(); // internal regs

	map(0x2010, 0x2011).w(FUNC(igs017_state::incdec_remap_addr_w));

	map(0x8000, 0x8000).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x9000, 0x9001).w("ymsnd", FUNC(ym2413_device::write));

//  map(0xa000, 0xa0??).ram(); // read/written during poker game enabling at boot (patched out)

	map(0xb000, 0xb000).w (m_igs_mux, FUNC(igs_mux_device::address_w));
	map(0xb001, 0xb001).rw(m_igs_mux, FUNC(igs_mux_device::data_r), FUNC(igs_mux_device::data_w));
}

void igs017_state::spkrform_mux_map(address_map &map)
{
	map(0x00, 0x00).portr("PLAYER1").w(NAME((&igs017_state::counter_w<7, 0x7f, 0>))); // coin in
	map(0x01, 0x01).portr("PLAYER2").w(NAME((&igs017_state::counter_w<7, 0x7f, 1>))); // coin out
	map(0x02, 0x02).portr("COINS").w(NAME((&igs017_state::hopper_motor_w<4, 0xef>))); // bit 5 is related to poker game enabling
	map(0x03, 0x03).portr("BUTTONS");

	igs_string_mux_map(map); // 0x05 r, 0x20 - 0x27 w, 0x40 r
}


// lhzb2

void igs017_state::lhzb2_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

	map(0x100000, 0x103fff).ram().share("igs022:sharedprotram"); // Shared with protection device

	map(0x500000, 0x503fff).ram();

	map(0x910000, 0x910001).nopr().w(m_igs_mux, FUNC(igs_mux_device::address_w)).umask16(0x00ff); // clr.w dummy read
	map(0x910002, 0x910003).rw(m_igs_mux, FUNC(igs_mux_device::data_r), FUNC(igs_mux_device::data_w)).umask16(0x00ff);

	map(0xb00000, 0xb0ffff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write)).umask16(0x00ff);

	map(0xb10001, 0xb10001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

u8 igs017_state::lhzb2_keys_r()
{
	u8 ret = 0xff;
	if (!BIT(m_input_select, 0)) ret &= m_io_key[0]->read();
	if (!BIT(m_input_select, 1)) ret &= m_io_key[1]->read();
	if (!BIT(m_input_select, 2)) ret &= m_io_key[2]->read();
	if (!BIT(m_input_select, 3)) ret &= m_io_key[3]->read();
	if (!BIT(m_input_select, 4)) ret &= m_io_key[4]->read();
	return ret;
}

void igs017_state::lhzb2_keys_hopper_w(u8 data)
{
	m_input_select =                          BIT(data, 0, 5); // keys
	m_hopper->motor_w(                        BIT(data, 5));   // hopper
	machine().bookkeeping().coin_counter_w(1, BIT(data, 6));   // coin out counter
	machine().bookkeeping().coin_counter_w(0, BIT(data, 7));   // coin in  counter
}

u8 igs017_state::lhzb2_scramble_data_r()
{
	u8 ret = bitswap<8>(m_scramble_data, 0,1,2,3,4,5,6,7);
	LOGMASKED(LOG_PROT_SCRAMBLE, "%s: reading scrambled data %02x from igs_mux\n", machine().describe_context(), ret);
	return ret;
}

void igs017_state::lhzb2_igs022_execute_w(u8 data)
{
	m_oki->set_rom_bank(BIT(data, 7));
	igs022_execute_w<6, 0x3f>(data);
}

void igs017_state::lhzb2_mux_map(address_map &map)
{
	map(0x00, 0x00).w(FUNC(igs017_state::lhzb2_keys_hopper_w));
	map(0x01, 0x01).r(FUNC(igs017_state::lhzb2_keys_r)).w(FUNC(igs017_state::lhzb2_igs022_execute_w));
	map(0x02, 0x02).r(FUNC(igs017_state::lhzb2_scramble_data_r));
	map(0x03, 0x03).w(FUNC(igs017_state::mgcs_scramble_data_w));

	igs_string_mux_map(map); // 0x05 r, 0x20 - 0x27 w, 0x40 r
}


// lhzb2a
// To do: what devices are on this PCB?

u16 igs017_state::lhzb2a_input_r(offs_t offset)
{
	switch (offset * 2)
	{
		case 0x00: // Keys
			return (lhzb2_keys_r() << 8) | 0xff;

		case 0x02:
			return (m_io_dsw[0]->read() << 8) | m_io_coins->read();

		case 0x04:
			return 0xff00 | m_io_dsw[1]->read();
	}

	return 0xffff;
}

/***************************************************************************

    An address base register (xx = F0 at reset) determines where the bitswap protection device,
    as well as game inputs and the address base register itself are mapped in memory:
    inputs are mapped at xx8000, protection at xx4000 and address base register at xxc000.

***************************************************************************/

void igs017_state::lhzb2a_remap_addr_w(address_space &space, u16 data)
{
	// Unmap previous address ranges
	if (m_remap_addr != -1)
	{
		space.unmap_write    (m_remap_addr * 0x10000 + 0x4000, m_remap_addr * 0x10000 + 0x4001);
		space.unmap_readwrite(m_remap_addr * 0x10000 + 0x4002, m_remap_addr * 0x10000 + 0x4003);

		space.unmap_read     (m_remap_addr * 0x10000 + 0x8000, m_remap_addr * 0x10000 + 0x8005);
		space.unmap_write    (m_remap_addr * 0x10000 + 0xc000, m_remap_addr * 0x10000 + 0xc001);
	}

	m_remap_addr = data & 0xff;

	// Add new memory ranges
	space.install_write_handler    (m_remap_addr * 0x10000 + 0x4001, m_remap_addr * 0x10000 + 0x4001, write8smo_delegate(*m_igs_mux, FUNC(igs_mux_device::address_w)));
	space.install_readwrite_handler(m_remap_addr * 0x10000 + 0x4003, m_remap_addr * 0x10000 + 0x4003, read8smo_delegate (*m_igs_mux, FUNC(igs_mux_device::data_r)), write8smo_delegate(*m_igs_mux, FUNC(igs_mux_device::data_w)));

	space.install_read_handler     (m_remap_addr * 0x10000 + 0x8000, m_remap_addr * 0x10000 + 0x8005, read16sm_delegate (*this, FUNC(igs017_state::lhzb2a_input_r)));
	space.install_write_handler    (m_remap_addr * 0x10000 + 0xc000, m_remap_addr * 0x10000 + 0xc001, write16mo_delegate(*this, FUNC(igs017_state::lhzb2a_remap_addr_w)));

	LOGMASKED(LOG_PROT_REMAP, "%s: inputs and protection remapped at %02xxxxx\n", machine().describe_context(), m_remap_addr);
}

void igs017_state::lhzb2a_keys_hopper_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_input_select =                          BIT(data, 0, 5); // keys
		m_hopper->motor_w(                        BIT(data, 5));   // hopper
		machine().bookkeeping().coin_counter_w(0, BIT(data, 6));   // coin out counter
		machine().bookkeeping().coin_counter_w(1, BIT(data, 7));   // coin in  counter
	}
	if (ACCESSING_BITS_8_15)
	{
		m_oki->set_rom_bank(BIT(data, 7));

		if (data & 0xfe00)
			logerror("%s: warning, unknown bits written in keys_hopper_w = %04x\n", machine().describe_context(), data);
	}
}

void igs017_state::lhzb2a_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

	// incdec protection
	map(0x003201, 0x003201).w(m_igs_incdec, FUNC(igs_incdec_device::reset_w));
	map(0x003203, 0x003203).w(m_igs_incdec, FUNC(igs_incdec_device::dec_w));
	map(0x003207, 0x003207).w(m_igs_incdec, FUNC(igs_incdec_device::inc_w));
	map(0x00320b, 0x00320b).r(m_igs_incdec, FUNC(igs_incdec_device::result_r));

	map(0x500000, 0x503fff).ram();
//  map(0x910000, 0x910003) accesses appear to be from leftover code where the final checks were disabled

	map(0xb00000, 0xb0ffff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write)).umask16(0x00ff);

	map(0xb10001, 0xb10001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0xb12000, 0xb12001).w(FUNC(igs017_state::lhzb2a_keys_hopper_w));

	// Inputs dynamically mapped at xx8000, protection at xx4000 (xx = f0 initially). xx written to xxc000
}

void igs017_state::lhzb2a_mux_map(address_map &map)
{
	igs_bitswap_mux_map(map); // 0x03 r, 0x40 - 0x47 w, 0x48 w, 0x50 w, 0x80 - 0x87 w, 0xa0 w

	// used ?
	igs_string_mux_map(map); // 0x05 r, 0x20 - 0x27 w, 0x40 r
}


// slqz2

void igs017_state::slqz2_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram();

	map(0x300000, 0x303fff).ram().share("igs022:sharedprotram"); // Shared with protection device

	map(0x602000, 0x602001).nopr().w(m_igs_mux, FUNC(igs_mux_device::address_w)).umask16(0x00ff); // clr.w dummy read
	map(0x602002, 0x602003).rw(m_igs_mux, FUNC(igs_mux_device::data_r), FUNC(igs_mux_device::data_w)).umask16(0x00ff);

	map(0x900000, 0x90ffff).rw(m_igs017_igs031, FUNC(igs017_igs031_device::read), FUNC(igs017_igs031_device::write)).umask16(0x00ff);

	map(0x910001, 0x910001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void igs017_state::slqz2_sound_hopper_w(u8 data)
{
	m_oki->set_rom_bank(                      BIT(data, 0));
	//
	m_hopper->motor_w(                        BIT(data, 5)); // hopper
	machine().bookkeeping().coin_counter_w(1, BIT(data, 6)); // coin out counter
	machine().bookkeeping().coin_counter_w(0, BIT(data, 7)); // coin in  counter

	if (data & 0x1e)
		logerror("%s: warning, unknown bits written in sound_hopper_w = %04x\n", machine().describe_context(), data);
}

u8 igs017_state::slqz2_scramble_data_r()
{
	u8 ret = m_scramble_data;
	LOGMASKED(LOG_PROT_SCRAMBLE, "%s: reading scrambled data %02x from igs_mux\n", machine().describe_context(), ret);
	return ret;
}

void igs017_state::slqz2_mux_map(address_map &map)
{
	map(0x00, 0x00).portr("PLAYER2").w(FUNC(igs017_state::slqz2_sound_hopper_w));
	map(0x01, 0x01).portr("PLAYER1").w(NAME((&igs017_state::igs022_execute_w<6, 0xbf>)));
	map(0x02, 0x02).r(FUNC(igs017_state::slqz2_scramble_data_r));
	map(0x03, 0x03).w(FUNC(igs017_state::mgcs_scramble_data_w));

	igs_string_mux_map(map); // 0x05 r, 0x20 - 0x27 w, 0x40 r
}


/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( iqblocka )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hold Mode" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, "In Win" )
	PORT_DIPSETTING(    0x00, "Always" )
	PORT_DIPNAME( 0x04, 0x04, "Max Credit" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "4000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x38, 0x38, "Cigarette Bet" ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x38, "1" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x28, "20" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x18, "80" )
	PORT_DIPSETTING(    0x10, "100" )
	PORT_DIPSETTING(    0x08, "120" )
	PORT_DIPSETTING(    0x00, "150" )
	PORT_DIPNAME( 0xc0, 0xc0, "Minimum Bet" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x00, "50" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Key In" ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, "10" )
	PORT_DIPSETTING(    0x06, "20" )
	PORT_DIPSETTING(    0x05, "40" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPSETTING(    0x02, "200" )
	PORT_DIPSETTING(    0x01, "250" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x08, 0x08, "Key Out" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Open Mode" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Gaming (Gambling)" )
	PORT_DIPSETTING(    0x00, "Amuse" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Game" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Bonus Base" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0xc0, "100" )
	PORT_DIPSETTING(    0x80, "200" )
	PORT_DIPSETTING(    0x40, "300" )
	PORT_DIPSETTING(    0x00, "400" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Win Up Pool" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(    0x03, "300" )
	PORT_DIPSETTING(    0x02, "500" )
	PORT_DIPSETTING(    0x01, "800" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Double Up" ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(    0x0c, "20000" )
	PORT_DIPSETTING(    0x08, "30000" )
	PORT_DIPSETTING(    0x04, "40000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x10, 0x10, "Number Type" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, "A,J,Q,K" )
	PORT_DIPSETTING(    0x00, "Number" )
	PORT_DIPNAME( 0x20, 0x20, "Show Title" ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x40, 0x40, "Double Up" ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "CG Select" ) PORT_DIPLOCATION("SW3:8") // Switches CG ROM (sprites). Unpopulated in this set
	PORT_DIPSETTING(    0x80, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )

	PORT_START("PLAYER1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1         ) // start, in videogame mode (keep pressed while booting for DSW and inputs test)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_NAME("%p Down / Collect Win")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD1    ) PORT_NAME("Hold 1 / Big / Help") // (1P A in test mode) help = next tile becomes a wildcard (in videogame mode)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD2    ) PORT_NAME("Hold 2 / Double Up")  // (1P B in test mode)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("PLAYER2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL    ) PORT_NAME("Deal / Last Bet") // play current bet or, if null, the last bet (START2 in test mode)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2) // unused? shown in test mode
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2) // ""
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) // ""
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) // ""
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4    ) PORT_NAME("Hold 4 / Half Double")        // (2P A in test mode)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5    ) PORT_NAME("Hold 5 / Tile in Double Up?") // (2P B in test mode)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN   )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1          ) PORT_IMPULSE(5) // impulse prevents coin error in gambling mode
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW       ) // keep pressed while booting
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1      ) PORT_NAME("Toggle Gambling") // this toggles between videogame and gambling

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD3    ) PORT_NAME("Hold 3 / Small") //  (1P C in test mode)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2          ) PORT_IMPULSE(5) // no coin. Hopper sensor? impulse prevents coin error in gambling mode (1P D in test mode)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4        )                 // unused?      (1P E in test mode)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BET     )                 // Bet 1 credit (2P C in test mode)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4        ) PORT_PLAYER(2)  // unused?      (2P D in test mode)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5        ) PORT_PLAYER(2)  // unused?      (2P E in test mode)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )
INPUT_PORTS_END

static INPUT_PORTS_START( iqblockf )
	PORT_INCLUDE( iqblocka )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hold Mode" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, "In Win" )
	PORT_DIPSETTING(    0x00, "Always" )
	PORT_DIPNAME( 0x0c, 0x0c, "Coin In" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x30, 0x30, "Key In" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x20, "100" )
	PORT_DIPSETTING(    0x10, "200" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0xc0, 0xc0, "Minimum Bet" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x00, "10" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Max Bet" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x01, "20" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x04, 0x04, "Register" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Key Out Base" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x10, 0x10, "Open Mode" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Gaming (Gambling)" )
	PORT_DIPSETTING(    0x00, "Amuse" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Game" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_MODIFY("DSW3")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW3:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPNAME( 0x10, 0x10, "Number Type" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, "A,J,Q,K" )
	PORT_DIPSETTING(    0x00, "Number" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW3:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW3:7" )
	PORT_DIPNAME( 0x80, 0x80, "CG Select" ) PORT_DIPLOCATION("SW3:8") // Switches CG ROM (sprites). Unpopulated in this set
	PORT_DIPSETTING(    0x80, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )

	PORT_MODIFY("PLAYER2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL    ) PORT_NAME("Deal / Last Bet / Toggle Gambling (8 Times)") // play current bet or, if null, bet as last time (START2 in test mode)

	PORT_MODIFY("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1          ) PORT_IMPULSE(5) // impulse prevents coin error in gambling mode
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN   )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW       ) // book-keeping after switching to gambling (TEST in test mode)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1      ) PORT_NAME("Start Gambling Toggle (Then Deal x 8)") // this starts toggling between videogame and gambling

	PORT_MODIFY("BUTTONS")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT  ) // (1P E in test mode)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT  ) // (2P E in test mode)
INPUT_PORTS_END

static INPUT_PORTS_START( genius6 )
	PORT_INCLUDE( iqblockf )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Auto Hold" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Coin In" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x04, "20" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x30, 0x30, "Key In" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x20, "100" )
	PORT_DIPSETTING(    0x10, "200" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0xc0, 0xc0, "Minimum Bet" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x00, "10" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Max Bet" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x01, "20" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPNAME( 0x08, 0x08, "Key Out" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x10, 0x10, "Open Mode" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Gaming (Gambling)" )
	PORT_DIPSETTING(    0x00, "Amuse" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Game" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_MODIFY("DSW3")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW3:1" ) // the input test screen prints garbage when not all off
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW3:2" ) // ""
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW3:3" ) // ""
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" ) // ""
	PORT_DIPNAME( 0x10, 0x10, "Number Type" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, "A,J,Q,K" )
	PORT_DIPSETTING(    0x00, "Number" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW3:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW3:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW3:8" )

	PORT_MODIFY("PLAYER2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) // (2P B in test mode)
INPUT_PORTS_END

static INPUT_PORTS_START( lhzb2 )
	PORT_INCLUDE(igs_mahjong_matrix)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Credits Per Note" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Max Note Credits" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "1000" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0x20, 0x20, "Money Type" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x40, 0x40, "Pay Out Type" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "500" )
	PORT_DIPSETTING(    0x02, "1000" )
	PORT_DIPSETTING(    0x01, "1500" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0x0c, 0x0c, "Minimum Bet" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "1 (1)" )
	PORT_DIPSETTING(    0x08, "1 (2)" )
	PORT_DIPSETTING(    0x04, "1 (3)" )
	PORT_DIPSETTING(    0x00, "1 (4)" )
	PORT_DIPNAME( 0x10, 0x10, "Bonus Round" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Number Type" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "Number" )
	PORT_DIPSETTING(    0x00, "Dice" )
	PORT_DIPNAME( 0x40, 0x40, "Symbols" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) ) // pigs, apples
	PORT_DIPNAME( 0x80, 0x80, "Hide Gambling" ) PORT_DIPLOCATION("SW2:8") // press "Hide Gambling" to hide credits and bets
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_CUSTOM      ) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r) // hopper switch
	PORT_SERVICE_NO_TOGGLE( 0x02,   IP_ACTIVE_LOW   ) // test mode (keep pressed during boot too)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_GAMBLE_BOOK ) // press with the above for sound test
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN1       ) PORT_IMPULSE(5) // coin error otherwise
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER       ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE1    ) PORT_NAME("Hide Gambling") // shown in test mode as "clear" (清除)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN     )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN     )
INPUT_PORTS_END

static INPUT_PORTS_START( lhzb2a )
	PORT_INCLUDE( lhzb2 )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN     )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_CUSTOM      ) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r) // hopper switch
	PORT_SERVICE_NO_TOGGLE( 0x04,   IP_ACTIVE_LOW   ) // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_GAMBLE_BOOK ) // press with the above for sound test
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_COIN1       ) PORT_IMPULSE(5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_OTHER       ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1    ) PORT_NAME("Hide Gambling") // shown in test mode as "clear" (清除)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN     )
INPUT_PORTS_END

static INPUT_PORTS_START( mgcs )
	// DSWs are read through a protection device (IGS029). See code at $1cf16

	PORT_START("DSW1") // $3009e2
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Credits Per Note" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Max Note Credits" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x20, 0x20, "Money Type" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x40, 0x40, "Pay Out Type" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x80, 0x80, "Double Up Limit" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "1000" )
	PORT_DIPSETTING(    0x00, "2000" )

	PORT_START("DSW2") // $3009e3
	PORT_DIPNAME( 0x03, 0x03, "Minimum Bet" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x04, "Double Up" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Continue To Play" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING( 0x08, DEF_STR( Yes ) )
	PORT_DIPSETTING( 0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Keyboard" )
	PORT_DIPSETTING(    0x00, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x20, 0x20, "Number Type" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "Number" )
	PORT_DIPSETTING(    0x00, "Tile" )
	PORT_DIPNAME( 0x40, 0x40, "Hide Gambling" ) PORT_DIPLOCATION("SW2:7") // press "Hide Gambling" to hide credits and bets
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	// Joystick mode: the top 2 bits of COINS (port A) and JOY (port B) are read and combined with the bottom 4 bits read from port C (see code at $1c83a)

	PORT_START("JOY")
	// Joystick mode:
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1        ) // take tile or throw (as N in mahjong keyboard)
	// Port C input is 4 bits
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1         )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_CUSTOM      ) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r) // hopper switch
	PORT_SERVICE_NO_TOGGLE( 0x02,   IP_ACTIVE_LOW   ) // test mode (keep pressed during boot too)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_GAMBLE_BOOK ) // press with the above for sound test
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN1       ) PORT_IMPULSE(5)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER       ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE1    ) PORT_NAME("Hide Gambling") // shown in test mode as "clear" (清除)
	// Keyboard mode:
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN      ) PORT_CONDITION("DSW2",0x10,EQUALS,0x10)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN      ) PORT_CONDITION("DSW2",0x10,EQUALS,0x10)
	// Joystick mode:
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON2     ) PORT_CONDITION("DSW2",0x10,EQUALS,0x00) // bet
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON3     ) PORT_CONDITION("DSW2",0x10,EQUALS,0x00) // function

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( sdmg2 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Credits Per Note" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Max Note Credits" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "2000" )
	PORT_DIPSETTING(    0x00, "29999" )
	PORT_DIPNAME( 0x20, 0x20, "Money Type" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x40, 0x40, "Pay Out Type" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Minimum Bet" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "500" )
	PORT_DIPSETTING(    0x02, "1000" )
	PORT_DIPSETTING(    0x01, "1500" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, "Keyboard" )
	PORT_DIPSETTING(    0x00, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x80, 0x80, "Number Type" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "Number" )
	PORT_DIPSETTING(    0x00, "Tile" )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_CUSTOM      ) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r) // hopper switch
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1    ) PORT_NAME("Hide Gambling") // shown in test mode as "clear" (清除), does not work in game?
	PORT_SERVICE_NO_TOGGLE( 0x04,   IP_ACTIVE_LOW   ) // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_GAMBLE_BOOK )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_COIN1       )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_OTHER       ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O)
	// Keyboard mode:
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_SERVICE3    ) PORT_CONDITION("DSW2",0x40,EQUALS,0x40) // shown in test mode ('O' appears, or it might be a 0)
	// Joystick mode:
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON3     ) PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN     )

	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // related to joystick BUTTON3

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( sdmg2p )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Credits Per Note" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Max Note Credits" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "2000" )
	PORT_DIPSETTING(    0x00, "29999" )
	PORT_DIPNAME( 0x20, 0x20, "Money Type" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x40, 0x40, "Pay Out Type" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x80, 0x80, "Hidden Function" ) PORT_DIPLOCATION("SW1:8") // 隐分功能 (Yǐnfēn Gōngnéng) TODO: determine what this does
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Game Title" ) PORT_DIPLOCATION("SW2:1") // 機種名稱 (Jīzhǒng Míngchēng)
	PORT_DIPSETTING(    0x01, "Maque Wangchao" )
	PORT_DIPSETTING(    0x00, "Chaoji Damanguan 2 - Jiaqiang Ban" ) // actually abbreviated in 超二加強 (Chāo èr jiāqiáng)
	PORT_DIPNAME( 0x02, 0x02, "Double Up Limit" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, "500" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x0c, 0x0c, "Minimum Bet" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, "Double Up" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Double-up/Continue" ) PORT_DIPLOCATION("SW2:6") // 比倍續玩 TODO: determine what this does
	PORT_DIPSETTING(    0x20, "Double Up" ) // 比倍
	PORT_DIPSETTING(    0x00, "Continue Game" ) // 續玩
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, "Keyboard" )
	PORT_DIPSETTING(    0x00, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x80, 0x80, "Number Type" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "Number" )
	PORT_DIPSETTING(    0x00, "Tile" )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW3:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW3:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW3:8" )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	// Joystick mode only?:
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3     ) PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN     ) PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER       ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_SERVICE_NO_TOGGLE( 0x20,  IP_ACTIVE_LOW   ) // also keep pressed while booting
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN     )

	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS")
	// Joystick mode only:
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	// Keyboard mode only:
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_CONDITION("DSW2",0x40,EQUALS,0x40) PORT_NAME("Hide Gambling") // shown in test mode as "clear" (清除)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r) // hopper switch, TODO: verify
INPUT_PORTS_END

static INPUT_PORTS_START( mgdh )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Credits Per Note" ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x06, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x02, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x08, 0x08, "Max Note Credits" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "100" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x10, 0x10, "Money Type" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x20, 0x20, "Pay Out Type" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0xc0, 0xc0, "Minimum Bet" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("DSW2") // bitswapped
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, "Keyboard" )
	PORT_DIPSETTING(    0x00, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x04, 0x04, "Continue To Play" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING( 0x04, DEF_STR( Yes ) )
	PORT_DIPSETTING( 0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6,7,8")
	PORT_DIPSETTING(    0xe0, "1" )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0xa0, "5" )
	PORT_DIPSETTING(    0x80, "6" )
	PORT_DIPSETTING(    0x60, "7" )
	PORT_DIPSETTING(    0x40, "8" )
	PORT_DIPSETTING(    0x20, "9" )
	PORT_DIPSETTING(    0x00, "10" )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_CUSTOM      ) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r) // hopper switch
	PORT_SERVICE_NO_TOGGLE( 0x02,   IP_ACTIVE_LOW   ) // test mode (keep pressed during boot too)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_GAMBLE_BOOK ) // press with the above for sound test
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN1       ) PORT_IMPULSE(5) // coin error otherwise
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER       ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE1    ) PORT_NAME("Hide Gambling") // shown in test mode as "clear" (清除), does not work in game?
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN     )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN     )

	PORT_START("KEY0")
	// Keyboard mode:
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A      ) PORT_CONDITION("DSW2",0x02,EQUALS,0x02)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E      ) PORT_CONDITION("DSW2",0x02,EQUALS,0x02)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I      ) PORT_CONDITION("DSW2",0x02,EQUALS,0x02)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M      ) PORT_CONDITION("DSW2",0x02,EQUALS,0x02)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN    ) PORT_CONDITION("DSW2",0x02,EQUALS,0x02)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1         ) PORT_CONDITION("DSW2",0x02,EQUALS,0x02)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN        ) PORT_CONDITION("DSW2",0x02,EQUALS,0x02)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        ) PORT_CONDITION("DSW2",0x02,EQUALS,0x02)
	// Joystick mode:
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1         ) PORT_CONDITION("DSW2",0x02,EQUALS,0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_CONDITION("DSW2",0x02,EQUALS,0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_CONDITION("DSW2",0x02,EQUALS,0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_CONDITION("DSW2",0x02,EQUALS,0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_CONDITION("DSW2",0x02,EQUALS,0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_CONDITION("DSW2",0x02,EQUALS,0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN        ) PORT_CONDITION("DSW2",0x02,EQUALS,0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        ) PORT_CONDITION("DSW2",0x02,EQUALS,0x00)

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )
INPUT_PORTS_END

static INPUT_PORTS_START( slqz2 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Credits Per Note" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Max Note Credits" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "1000" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0x20, 0x20, "Money Type" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x40, 0x40, "Pay Out Type" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Double Up Limit" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "500" )
	PORT_DIPSETTING(    0x02, "1000" )
	PORT_DIPSETTING(    0x01, "1500" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0x0c, 0x0c, "Minimum Bet" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, "Bonus Round" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Number Type" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "Number" )
	PORT_DIPSETTING(    0x00, "Dice" )
	PORT_DIPNAME( 0x40, 0x40, "Symbols" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) ) // pigs, apples
	PORT_DIPNAME( 0x80, 0x80, "Hide Gambling" ) PORT_DIPLOCATION("SW2:8") // press "Hide Gambling" to hide credits and bets
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_CUSTOM      ) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r) // hopper switch
	PORT_SERVICE_NO_TOGGLE( 0x02,   IP_ACTIVE_LOW   ) // test mode (keep pressed during boot too)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_GAMBLE_BOOK ) // press with the above for sound test
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN1       ) PORT_IMPULSE(5) // coin error otherwise
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER       ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN     ) // needs to be 0 for "clear" input below to work
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON2     )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON3     )

	PORT_START("PLAYER1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1         ) PORT_NAME("Start / Don Den")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_NAME("Help / Big")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("PLAYER2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1      ) PORT_NAME("Hide Gambling") // shown in test mode as "clear" (清除)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
INPUT_PORTS_END

static INPUT_PORTS_START( tjsb )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Credits Per Note" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Max Note Credits" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "1000" )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPNAME( 0x20, 0x20, "Money Type" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x40, 0x40, "Pay Out Type" ) PORT_DIPLOCATION("SW1:7") // 2/4
	PORT_DIPSETTING(    0x40, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Minimum Bet" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1000" )
	PORT_DIPSETTING(    0x02, "2000" )
	PORT_DIPSETTING(    0x01, "3000" )
	PORT_DIPSETTING(    0x00, "4000" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x10, 0x10, "Bonus Round" ) PORT_DIPLOCATION("SW2:5") // show bonus round in demo mode -> protection check
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Number Type" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "Number" )
	PORT_DIPSETTING(    0x00, "Dice" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3") // the protection check is skipped if (DSW3 ^ 0xff) & 0x9a == 0x0a
	PORT_DIPNAME( 0xff, 0xf5, "Bonus Round Protection Check" ) PORT_DIPLOCATION("SW3:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(    0xf5, DEF_STR( Off ) )
	PORT_DIPSETTING(    0xff, DEF_STR( On ) )

	PORT_START("PLAYER1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1         ) PORT_NAME("Start / Don Den")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1        ) // choose
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2        ) // bet
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("PLAYER2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_SERVICE_NO_TOGGLE( 0x02,  IP_ACTIVE_LOW   ) // keep pressed while booting
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1       )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_BUTTON3  )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Hide Gambling") // shown in test mode as "clear" (清除), does not work in game?
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER    ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_CUSTOM   ) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r) // hopper switch
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
INPUT_PORTS_END

static INPUT_PORTS_START( spkrform )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPNAME( 0x01c, 0x1c, "Credits Per Coin" ) PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(    0x1c, "1" )
	PORT_DIPSETTING(    0x18, "4" )
	PORT_DIPSETTING(    0x14, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x0c, "20" )
	PORT_DIPSETTING(    0x08, "40" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x20, 0x20, "Hopper" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Minimum Bet" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x1c, 0x1c, "Credits Per Note" ) PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(    0x1c, "10" )
	PORT_DIPSETTING(    0x18, "20" )
	PORT_DIPSETTING(    0x14, "40" )
	PORT_DIPSETTING(    0x10, "50" )
	PORT_DIPSETTING(    0x0c, "100" )
	PORT_DIPSETTING(    0x08, "200" )
	PORT_DIPSETTING(    0x04, "250" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "?" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPSETTING(    0x02, "200" )
	PORT_DIPSETTING(    0x01, "300" )
	PORT_DIPSETTING(    0x00, "400" )
	PORT_DIPNAME( 0x0c, 0x0c, "Win Up Pool" ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(    0x0c, "300" )
	PORT_DIPSETTING(    0x08, "500" )
	PORT_DIPSETTING(    0x04, "800 (1)" )
	PORT_DIPSETTING(    0x00, "800 (2)" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW3:8" )

	PORT_START("PLAYER1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1       ) PORT_NAME("Hide Gambling (Switch To Formosa)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2         ) PORT_NAME("Start (Formosa)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) // up (Formosa)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START          ) PORT_NAME("Start / Draw / Take / %p Down (Formosa)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("PLAYER2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD4    ) PORT_NAME("Hold 4 / Half")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5    ) // hold 5
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BET     ) PORT_NAME("Bet / W-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) // right (Formosa)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD1    ) PORT_NAME("Hold 1 / High / Button (Formosa)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD2    ) PORT_NAME("Hold 2 / Double Up")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) // left (Formosa)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3    ) PORT_NAME("Hold 3 / Low")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN   ) // key in
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT  ) // key out
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1          ) PORT_IMPULSE(5) // coin 1 (impulse prevents coin error in gambling mode)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2          ) PORT_IMPULSE(5) // coin 2 ""
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM         ) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r) // hopper switch
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT  ) // payout
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3       ) PORT_NAME("Return To Gambling (From Formosa). Then Bet, Hold 1..5") // To switch back to poker from Formosa, start the sequence pressing this key (memory $f4a3 holds the sequence number)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK    ) // book
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )
INPUT_PORTS_END

static INPUT_PORTS_START( tarzan )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x0e, "Coin Value" ) PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING( 0x0e, "1" )
	PORT_DIPSETTING( 0x0c, "2" )
	PORT_DIPSETTING( 0x0a, "4" )
	PORT_DIPSETTING( 0x08, "5" )
	PORT_DIPSETTING( 0x06, "10" )
	PORT_DIPSETTING( 0x04, "20" )
	PORT_DIPSETTING( 0x02, "50" )
	PORT_DIPSETTING( 0x00, "100" )
	PORT_DIPNAME( 0x30, 0x30, "Key Value" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING( 0x30, "100" )
	PORT_DIPSETTING( 0x20, "200" )
	PORT_DIPSETTING( 0x10, "500" )
	PORT_DIPSETTING( 0x00, "1000" )
	PORT_DIPNAME( 0xc0, 0xc0, "Point Value" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING( 0xc0, "1" )
	PORT_DIPSETTING( 0x80, "10" )
	PORT_DIPSETTING( 0x40, "100" )
	PORT_DIPSETTING( 0x00, "100 (2)" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Coin Type" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING( 0x01, "Coin" )
	PORT_DIPSETTING( 0x00, "Key" )
	PORT_DIPNAME( 0x0e, 0x0e, "Minimum Bet" ) PORT_DIPLOCATION("SW2:2,3,4")
	PORT_DIPSETTING( 0x0e, "5" )
	PORT_DIPSETTING( 0x0c, "25" )
	PORT_DIPSETTING( 0x0a, "50" )
	PORT_DIPSETTING( 0x08, "75" )
	PORT_DIPSETTING( 0x06, "100" )
	PORT_DIPSETTING( 0x04, "125" )
	PORT_DIPSETTING( 0x02, "125 (2)" )
	PORT_DIPSETTING( 0x00, "125 (3)" )
	PORT_DIPNAME( 0x30, 0x30, "Bonus Bet" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING( 0x30, "75" )
	PORT_DIPSETTING( 0x20, "125" )
	PORT_DIPSETTING( 0x10, "200" )
	PORT_DIPSETTING( 0x00, "250" )
	PORT_DIPNAME( 0xc0, 0xc0, "Continue To Play" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING( 0xc0, "50k" )
	PORT_DIPSETTING( 0x80, "100k" )
	PORT_DIPSETTING( 0x40, "150k" )
	PORT_DIPSETTING( 0x00, "200k" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING( 0x01, "Keyboard" )
	PORT_DIPSETTING( 0x00, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x02, 0x00, "Back Color" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING( 0x02, "Black" )
	PORT_DIPSETTING( 0x00, "Color" )
	PORT_DIPNAME( 0x04, 0x04, "Hide Gambling" ) PORT_DIPLOCATION("SW3:3") // Press "Hide Gambling" to hide credits and bets
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Number Type" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING( 0x08, "Number" )
	PORT_DIPSETTING( 0x00, "Mahjong Tile" )
	PORT_DIPNAME( 0x10, 0x10, "Continue To Play" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Payout" ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING( 0x20, "Hopper" )
	PORT_DIPSETTING( 0x00, "Points" )
	PORT_DIPUNUSED_DIPLOC(0x40, 0x40, "SW3:7")
	PORT_DIPUNUSED_DIPLOC(0x80, 0x80, "SW3:8")

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM         ) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r) // hopper switch
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW       ) // Service
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_GAMBLE_BOOK   ) // Book
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN1         ) PORT_IMPULSE(5) // Coin/Key in (coin error in coin mode)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER         ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O) // Coin/Key out
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE1      ) PORT_NAME("Hide Gambling") // shown in test mode as "clear" (清除)
	// Keyboard mode:
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN       ) PORT_CONDITION("DSW3",0x01,EQUALS,0x01)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN       ) PORT_CONDITION("DSW3",0x01,EQUALS,0x01)
	// Joystick mode:
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON2       ) PORT_CONDITION("DSW3",0x01,EQUALS,0x00) PORT_NAME("%p Button 2 / Bet / Same Double Up"  ) // Button B (test mode)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON3       ) PORT_CONDITION("DSW3",0x01,EQUALS,0x00) PORT_NAME("%p Button 3 / Small / Half Double Up") // Button C (test mode)

	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1         ) PORT_NAME("Start / Take") // Exit (test mode)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) // Up    (test mode)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) // Down  (test mode)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) // Left  (test mode)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) // Right (test mode)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_NAME("%p Button 1 / Help / Big / Double Up") // Button A (test mode)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("KEY0")
	// Keyboard mode:
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A   ) PORT_NAME("%p Mahjong A / Help / Double Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E   )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I   )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M   ) PORT_NAME("%p Mahjong M / Small")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START       ) PORT_NAME("%p Mahjong Start / Take")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN     )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN     )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) // B (test mode)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) // Bet
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_NAME("%p Mahjong C / Half Double Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_NAME("%p Mahjong K / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4") // never read and not shown in test mode
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( starzan )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "System Limit" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING( 0x02, "Unlimited" )
	PORT_DIPSETTING( 0x00, "Limited" )
	PORT_DIPNAME( 0x04, 0x04, "W-Up Game" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Back Color" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING( 0x08, "Black" )
	PORT_DIPSETTING( 0x00, "Color" )
	PORT_DIPNAME( 0x10, 0x10, "Stop Status" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING( 0x10, "Non Stop" )
	PORT_DIPSETTING( 0x00, "Auto Stop" )
	PORT_DIPNAME( 0x20, 0x20, "Key" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING( 0x20, "Mode 1" )
	PORT_DIPSETTING( 0x00, "Mode 2" ) // To Do
	PORT_DIPNAME( 0x40, 0x40, "Credit Level" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Odds Table" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING( 0x80, "Show" )
	PORT_DIPSETTING( 0x00, "No Show" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Normal Level" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING( 0x01, DEF_STR( Low ) )
	PORT_DIPSETTING( 0x00, DEF_STR( High ) )
	PORT_DIPUNUSED_DIPLOC(0x02, 0x02, "SW2:2") // not used from here on according to test mode, PCB does have 3 8-dip banks
	PORT_DIPUNUSED_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNUSED_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNUSED_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNUSED_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNUSED_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNUSED_DIPLOC(0x80, 0x80, "SW2:8")

	PORT_START("DSW3")
	PORT_DIPUNUSED_DIPLOC(0x01, 0x01, "SW3:1")
	PORT_DIPUNUSED_DIPLOC(0x02, 0x02, "SW3:2")
	PORT_DIPUNUSED_DIPLOC(0x04, 0x04, "SW3:3")
	PORT_DIPUNUSED_DIPLOC(0x08, 0x08, "SW3:4")
	PORT_DIPUNUSED_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNUSED_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPUNUSED_DIPLOC(0x40, 0x40, "SW3:7")
	PORT_DIPUNUSED_DIPLOC(0x80, 0x80, "SW3:8")

	PORT_START("PLAYER1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM        ) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r) // hopper switch
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1        ) PORT_NAME("Start / HW-Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP3    ) PORT_NAME("Stop Reel 3 / Low")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Stop All Reels / Bet / 2W-Up")

	PORT_START("PLAYER2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP2    ) PORT_NAME("Stop Reel 2 / High")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SLOT_STOP4    ) PORT_NAME("Stop Reel 4 / W-Up")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP1    ) PORT_NAME("Stop Reel 1 / Take")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2         ) PORT_IMPULSE(5) // 'coin C'
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1         ) PORT_IMPULSE(5) // 'coin A'
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW      ) // keep pressed while booting
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK   ) // enters book-keeping menu
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test
INPUT_PORTS_END

// Test mode is in Italian (probably machine translated given how literal it is)
static INPUT_PORTS_START( happyskl )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1") // 'demo audio'
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "System Limit" ) PORT_DIPLOCATION("SW1:2") // 'lim. sistema'
	PORT_DIPSETTING( 0x02, "Unlimited" )
	PORT_DIPSETTING( 0x00, "Limited" )
	PORT_DIPNAME( 0x04, 0x04, "W-Up Game" ) PORT_DIPLOCATION("SW1:3") // 'gioco radd.'
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Game Speed" ) PORT_DIPLOCATION("SW1:4") // 'velocitá gioco'
	PORT_DIPSETTING( 0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING( 0x00, "Quick" )
	PORT_DIPNAME( 0x10, 0x10, "Replay Game" ) PORT_DIPLOCATION("SW1:5") // not translated for some reason
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Show Table" ) PORT_DIPLOCATION("SW1:6") // 'vedi tabella'
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Skill" ) PORT_DIPLOCATION("SW1:7") // 'abilitá'
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Payment Type" ) PORT_DIPLOCATION("SW1:8") // 'tipo pagam.'
	PORT_DIPSETTING( 0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING( 0x00, "Automatic" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Auto Payment" ) PORT_DIPLOCATION("SW2:1") // 'pagam. autom.'
	PORT_DIPSETTING( 0x01, "1" )
	PORT_DIPSETTING( 0x00, "10" )
	PORT_DIPNAME( 0x06, 0x06, "Enable Payment" ) PORT_DIPLOCATION("SW2:2,3") // 'abilitá pagam.', they probably meant "abilita" instead of "abilitá"
	PORT_DIPSETTING( 0x06, "Everything" ) // 'tutto'
	PORT_DIPSETTING( 0x04, "1/Tickets" )
	PORT_DIPSETTING( 0x02, "10/Tickets" )
	PORT_DIPSETTING( 0x00, "1/Tickets" ) // same as 0x04, why?
	PORT_DIPNAME( 0x08, 0x08, "Select Balls" ) PORT_DIPLOCATION("SW2:4") // 'sel. palloni'
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Select Cubes" ) PORT_DIPLOCATION("SW2:5") // 'sel. cubi'
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Select Cans" ) PORT_DIPLOCATION("SW2:6") // 'sel. lattine'
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Select Cards" ) PORT_DIPLOCATION("SW2:7") // 'sel. carte'
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Credit Limit" ) PORT_DIPLOCATION("SW2:8") // 'lim. crediti'
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Level Limit" ) PORT_DIPLOCATION("SW3:1") // 'lim. livello'
	PORT_DIPSETTING( 0x01, DEF_STR( Low ) )
	PORT_DIPSETTING( 0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x02, 0x02, "Background" ) PORT_DIPLOCATION("SW3:2") // 'sfondo'
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC(0x04, 0x04, "SW3:3") // not used from here on according to test mode
	PORT_DIPUNUSED_DIPLOC(0x08, 0x08, "SW3:4")
	PORT_DIPUNUSED_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNUSED_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPUNUSED_DIPLOC(0x40, 0x40, "SW3:7")
	PORT_DIPUNUSED_DIPLOC(0x80, 0x80, "SW3:8")

	PORT_START("PLAYER1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM        ) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r) // hopper switch
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1        )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD1   )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD5   )

	PORT_START("PLAYER2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2   )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3   )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD4   )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2         ) PORT_IMPULSE(5) // 'gettone C' (shows 'ricarica in corso')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1         ) PORT_IMPULSE(5) // 'gettone A'
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW      ) // keep pressed while booting
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK   ) // enters book-keeping menu
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN       ) // no effects in key test
INPUT_PORTS_END

static INPUT_PORTS_START( cpoker2 )
	PORT_INCLUDE(happyskl)

	PORT_MODIFY("PLAYER1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD3    )

	PORT_MODIFY("PLAYER2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1    )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2    )

	// dips definitions taken from test mode, to be verified when the game will be playable
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "System Limit" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING( 0x02, "Unlimited" )
	PORT_DIPSETTING( 0x00, "Limited" )
	PORT_DIPNAME( 0x04, 0x04, "W-Up Game" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "W-Up Type" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING( 0x08, "Big-Small" )
	PORT_DIPSETTING( 0x00, "Red-Black" )
	PORT_DIPNAME( 0x10, 0x10, "Game Speed" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING( 0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING( 0x00, "Quick" )
	PORT_DIPNAME( 0x20, 0x20, "Card Type" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING( 0x20, "Poker" )
	PORT_DIPSETTING( 0x00, "Symbol" )
	PORT_DIPNAME( 0x40, 0x40, "Sexy Girl" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Show Title" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING( 0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING( 0x80, DEF_STR( No ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Show Hold" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Number Type" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING( 0x02, "Number" )
	PORT_DIPSETTING( 0x00, "A,J,Q,K" )
	PORT_DIPUNUSED_DIPLOC(0x04, 0x04, "SW2:3") // not used from here on according to test mode, PCB does have 3 8-dip banks
	PORT_DIPUNUSED_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNUSED_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNUSED_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNUSED_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNUSED_DIPLOC(0x80, 0x80, "SW2:8")

	PORT_MODIFY("DSW3")
	PORT_DIPUNUSED_DIPLOC(0x01, 0x01, "SW3:1")
	PORT_DIPUNUSED_DIPLOC(0x02, 0x02, "SW3:2")
INPUT_PORTS_END

/***************************************************************************
                                Machine Drivers
***************************************************************************/

void igs017_state::base_machine_oki(machine_config &config, const XTAL &xtal_oki)
{
	// i/o
	IGS_MUX(config, m_igs_mux, 0);

	I8255A(config, m_ppi);

	// video
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 0, 240-1);
	m_screen->set_screen_update("igs017_igs031", FUNC(igs017_igs031_device::screen_update));
	m_screen->set_palette("igs017_igs031:palette");

	IGS017_IGS031(config, m_igs017_igs031, 0);

	// sound
	SPEAKER(config, "mono").front_center();
	OKIM6295(config, m_oki, xtal_oki, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5);
}


// iqblocka, iqblockf, genius6

TIMER_DEVICE_CALLBACK_MEMBER(igs017_state::iqblocka_interrupt)
{
	int scanline = param;

	if (scanline == 240 && m_igs017_igs031->get_irq_enable())
		m_maincpu->set_input_line(0, HOLD_LINE);

	if (scanline == 0 && m_igs017_igs031->get_nmi_enable())
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void igs017_state::iqblocka(machine_config &config)
{
	base_machine_oki(config, 16_MHz_XTAL / 16);

	HD64180RP(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs017_state::iqblocka_map);
	m_maincpu->set_addrmap(AS_IO, &igs017_state::iqblocka_io);
	TIMER(config, "scantimer").configure_scanline(FUNC(igs017_state::iqblocka_interrupt), "screen", 0, 1);

	// i/o
	m_igs_mux->set_addrmap(0, &igs017_state::iqblocka_mux_map);

	m_igs017_igs031->in_pa_callback().set_ioport("DSW1");
	m_igs017_igs031->in_pb_callback().set_ioport("DSW2");
	m_igs017_igs031->in_pc_callback().set_ioport("DSW3");

	// protection
	IGS_BITSWAP(config, m_igs_bitswap, 0);
	m_igs_bitswap->set_val_xor(0x15d6);
	m_igs_bitswap->set_mf_bits(3, 5, 9, 11);
	m_igs_bitswap->set_m3_bits<0>(~5,  8, ~10, ~15);
	m_igs_bitswap->set_m3_bits<1>( 3, ~8, ~12, ~15);
	m_igs_bitswap->set_m3_bits<2>( 2, ~6, ~11, ~15);
	m_igs_bitswap->set_m3_bits<3>( 0, ~1, ~3,  ~15);

	IGS_INCDEC(config, m_igs_incdec, 0);

	// sound
	YM2413(config, "ymsnd", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.5);
}

void igs017_state::iqblockf(machine_config &config)
{
	iqblocka(config);

	m_igs_mux->set_addrmap(0, &igs017_state::iqblockf_mux_map);

	// tweaked protection bitswap
	m_igs_bitswap->set_mf_bits(0, 5, 9, 13);
}

void igs017_state::genius6(machine_config &config)
{
	iqblockf(config);

	// tweaked protection bitswap
	m_igs_bitswap->set_mf_bits(2, 7, 9, 13);
	m_igs_bitswap->set_m3_bits<0>(~5,  6,  ~7, ~15);
	m_igs_bitswap->set_m3_bits<1>( 1, ~6,  ~9, ~15);
	m_igs_bitswap->set_m3_bits<2>( 4, ~8, ~12, ~15);
	m_igs_bitswap->set_m3_bits<3>( 3, ~5,  ~6, ~15);
}


// tarzan

void igs017_state::tarzan(machine_config &config)
{
	base_machine_oki(config, 16_MHz_XTAL / 16);

	HD64180RP(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs017_state::iqblocka_map);
	m_maincpu->set_addrmap(AS_IO, &igs017_state::tarzan_io);
	m_maincpu->set_addrmap(AS_OPCODES, &igs017_state::decrypted_opcodes_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(igs017_state::iqblocka_interrupt), "screen", 0, 1);

	// i/o
	m_igs_mux->set_addrmap(0, &igs017_state::tarzan_mux_map);

	m_igs017_igs031->in_pa_callback().set_ioport("COINS");
	m_igs017_igs031->in_pb_callback().set(FUNC(igs017_state::tarzan_keys_joy_r));

	HOPPER(config, m_hopper, attotime::from_msec(50));

	// protection
	IGS_STRING(config, m_igs_string, 0);

	IGS_INCDEC(config, m_igs_incdec, 0);

	// video
	m_igs017_igs031->set_palette_scramble_cb(FUNC(igs017_state::tarzan_palette_bitswap));

	// sound
	YM2413(config, "ymsnd", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.5);
}


// starzan

void igs017_state::starzan(machine_config &config)
{
	base_machine_oki(config, 16_MHz_XTAL / 16);

	HD64180RP(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs017_state::iqblocka_map);
	m_maincpu->set_addrmap(AS_IO, &igs017_state::starzan_io);
	m_maincpu->set_addrmap(AS_OPCODES, &igs017_state::decrypted_opcodes_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(igs017_state::iqblocka_interrupt), "screen", 0, 1);

	// i/o
	m_igs_mux->set_addrmap(0, &igs017_state::starzan_mux_map);

	m_igs017_igs031->in_pa_callback().set_ioport("COINS");
	m_igs017_igs031->in_pb_callback().set_ioport("PLAYER1");
	m_igs017_igs031->in_pc_callback().set(FUNC(igs017_state::dsw_r));

	HOPPER(config, m_hopper, attotime::from_msec(50));

	// protection
	IGS_STRING(config, m_igs_string, 0);

	IGS_INCDEC(config, m_igs_incdec, 0);

	// video
	m_igs017_igs031->set_palette_scramble_cb(FUNC(igs017_state::tarzan_palette_bitswap));

	// sound
	YM2413(config, "ymsnd", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.5);
}


// happyskl

void igs017_state::happyskl(machine_config &config)
{
	base_machine_oki(config, 16_MHz_XTAL / 16);

	HD64180RP(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs017_state::iqblocka_map);
	m_maincpu->set_addrmap(AS_IO, &igs017_state::happyskl_io);
	m_maincpu->set_addrmap(AS_OPCODES, &igs017_state::decrypted_opcodes_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(igs017_state::iqblocka_interrupt), "screen", 0, 1);

	// i/o
	m_igs_mux->set_addrmap(0, &igs017_state::happyskl_mux_map);

	m_igs017_igs031->in_pa_callback().set_ioport("COINS");
	m_igs017_igs031->in_pb_callback().set_ioport("PLAYER1");
	m_igs017_igs031->in_pc_callback().set(FUNC(igs017_state::dsw_r));

	HOPPER(config, m_hopper, attotime::from_msec(50));

	// video
	m_igs017_igs031->set_palette_scramble_cb(FUNC(igs017_state::tarzan_palette_bitswap));
}


// cpoker2

void igs017_state::cpoker2(machine_config &config)
{
	base_machine_oki(config, 16_MHz_XTAL / 16);

	HD64180RP(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs017_state::cpoker2_map);
	m_maincpu->set_addrmap(AS_IO, &igs017_state::cpoker2_io);
	TIMER(config, "scantimer").configure_scanline(FUNC(igs017_state::iqblocka_interrupt), "screen", 0, 1);

	// i/o
	m_igs_mux->set_addrmap(0, &igs017_state::cpoker2_mux_map);

	m_igs017_igs031->in_pa_callback().set_ioport("COINS");
	m_igs017_igs031->in_pb_callback().set_ioport("PLAYER1");
	m_igs017_igs031->in_pc_callback().set(FUNC(igs017_state::dsw_r));

	HOPPER(config, m_hopper, attotime::from_msec(50));

	// protection
	IGS_INCDEC(config, m_igs_incdec, 0);

	IGS_INC(config, m_igs_inc, 0);

	// video
	m_igs017_igs031->set_palette_scramble_cb(FUNC(igs017_state::tarzan_palette_bitswap));
}


// tjsb

void igs017_state::tjsb(machine_config &config)
{
	base_machine_oki(config, 16_MHz_XTAL / 16);

	HD64180RP(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs017_state::tjsb_map);
	m_maincpu->set_addrmap(AS_IO, &igs017_state::tjsb_io);
	TIMER(config, "scantimer").configure_scanline(FUNC(igs017_state::iqblocka_interrupt), "screen", 0, 1);

	// i/o
	m_igs_mux->set_addrmap(0, &igs017_state::tjsb_mux_map);

	m_igs017_igs031->in_pa_callback().set_ioport("DSW1");
	m_igs017_igs031->in_pb_callback().set_ioport("DSW2");
	m_igs017_igs031->in_pc_callback().set_ioport("DSW3");

	HOPPER(config, m_hopper, attotime::from_msec(50));

	// protection
	IGS_STRING(config, m_igs_string, 0);

	// video
	m_igs017_igs031->set_palette_scramble_cb(FUNC(igs017_state::tjsb_palette_bitswap));

	// sound
	YM2413(config, "ymsnd", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.5);
}


// spkrform

void igs017_state::spkrform(machine_config &config)
{
	base_machine_oki(config, 16_MHz_XTAL / 16);

	HD64180RP(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs017_state::iqblocka_map);
	m_maincpu->set_addrmap(AS_IO, &igs017_state::spkrform_io);
	TIMER(config, "scantimer").configure_scanline(FUNC(igs017_state::iqblocka_interrupt), "screen", 0, 1);

	// i/o
	m_igs_mux->set_addrmap(0, &igs017_state::spkrform_mux_map);

	m_igs017_igs031->in_pa_callback().set_ioport("DSW1");
	m_igs017_igs031->in_pb_callback().set_ioport("DSW2");
	m_igs017_igs031->in_pc_callback().set_ioport("DSW3");

	HOPPER(config, m_hopper, attotime::from_msec(50));

	// protection
	IGS_STRING(config, m_igs_string, 0);

	IGS_INCDEC(config, m_igs_incdec, 0);

	// sound
	YM2413(config, "ymsnd", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.5);
}


// mgcs

TIMER_DEVICE_CALLBACK_MEMBER(igs017_state::mgcs_interrupt)
{
	int scanline = param;

	if (scanline == 240 && m_igs017_igs031->get_irq_enable())
		m_maincpu->set_input_line(1, HOLD_LINE);

	if (scanline == 0 && m_igs017_igs031->get_nmi_enable())
		m_maincpu->set_input_line(2, HOLD_LINE);
}

void igs017_state::mgcs(machine_config &config)
{
	base_machine_oki(config, 8_MHz_XTAL / 8);

	M68000(config, m_maincpu, 22_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs017_state::mgcs_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(igs017_state::mgcs_interrupt), "screen", 0, 1);

	// i/o
	m_igs_mux->set_addrmap(0, &igs017_state::mgcs_mux_map);

	m_igs017_igs031->in_pa_callback().set_ioport("COINS");
	m_igs017_igs031->in_pb_callback().set(FUNC(igs017_state::mgcs_keys_joy_r));
	m_igs017_igs031->in_pc_callback().set_ioport("JOY");

	HOPPER(config, m_hopper, attotime::from_msec(50));

	// protection
	IGS_STRING(config, m_igs_string, 0);

	// video
	m_igs017_igs031->set_palette_scramble_cb(FUNC(igs017_state::mgcs_palette_bitswap));
}

void igs017_state::mgcsa(machine_config &config)
{
	mgcs(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &igs017_state::mgcsa_map);
}


// lhzb2

void igs017_state::lhzb2(machine_config &config)
{
	base_machine_oki(config, 8_MHz_XTAL / 8);

	M68000(config, m_maincpu, 22_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs017_state::lhzb2_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(igs017_state::mgcs_interrupt), "screen", 0, 1);

	// i/o
	m_igs_mux->set_addrmap(0, &igs017_state::lhzb2_mux_map);

	m_igs017_igs031->in_pa_callback().set_ioport("COINS");
	m_igs017_igs031->in_pb_callback().set_ioport("DSW1");
	m_igs017_igs031->in_pc_callback().set_ioport("DSW2");

	HOPPER(config, m_hopper, attotime::from_msec(50));

	// protection
	IGS_STRING(config, m_igs_string, 0);

	IGS022(config, m_igs022, 0);

	// video
	m_igs017_igs031->set_palette_scramble_cb(FUNC(igs017_state::lhzb2a_palette_bitswap));
}


// lhzb2a

MACHINE_RESET_MEMBER(igs017_state, lhzb2a)
{
	machine_reset();
	lhzb2a_remap_addr_w(m_maincpu->space(AS_PROGRAM), 0xf0);
}

void igs017_state::lhzb2a(machine_config &config)
{
	base_machine_oki(config, 8_MHz_XTAL / 8);

	M68000(config, m_maincpu, 22_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs017_state::lhzb2a_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(igs017_state::mgcs_interrupt), "screen", 0, 1);

	MCFG_MACHINE_RESET_OVERRIDE(igs017_state, lhzb2a)

	// i/o
	m_igs_mux->set_addrmap(0, &igs017_state::lhzb2a_mux_map);

	// ppi8255 not used for i/o (just video enable)?

	HOPPER(config, m_hopper, attotime::from_msec(50));

	// protection
	IGS_STRING(config, m_igs_string, 0);

	IGS_BITSWAP(config, m_igs_bitswap, 0);
	m_igs_bitswap->set_val_xor(0x289a);
	m_igs_bitswap->set_mf_bits(4, 7,  10, 13);
	m_igs_bitswap->set_m3_bits<0>(~3,   8, ~12, ~15);
	m_igs_bitswap->set_m3_bits<1>(~3,  ~6,  ~9, ~15);
	m_igs_bitswap->set_m3_bits<2>(~3,   4,  ~5, ~15);
	m_igs_bitswap->set_m3_bits<3>(~9, ~11,  12, ~15);

	IGS_INCDEC(config, m_igs_incdec, 0);

	// video
	m_igs017_igs031->set_palette_scramble_cb(FUNC(igs017_state::lhzb2a_palette_bitswap));
}


// slqz2

void igs017_state::slqz2(machine_config &config)
{
	base_machine_oki(config, 8_MHz_XTAL / 8);

	M68000(config, m_maincpu, 22_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs017_state::slqz2_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(igs017_state::mgcs_interrupt), "screen", 0, 1);

	// i/o
	m_igs_mux->set_addrmap(0, &igs017_state::slqz2_mux_map);

	m_igs017_igs031->in_pa_callback().set_ioport("COINS");
	m_igs017_igs031->in_pb_callback().set_ioport("DSW1");
	m_igs017_igs031->in_pc_callback().set_ioport("DSW2");

	HOPPER(config, m_hopper, attotime::from_msec(50));

	// protection
	IGS_STRING(config, m_igs_string, 0);

	IGS022(config, m_igs022, 0);

	// video
	m_igs017_igs031->set_palette_scramble_cb(FUNC(igs017_state::slqz2_palette_bitswap));
}


// sdmg2

void igs017_state::sdmg2(machine_config &config)
{
	base_machine_oki(config, 22_MHz_XTAL / 22);

	M68000(config, m_maincpu, 22_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs017_state::sdmg2_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(igs017_state::mgcs_interrupt), "screen", 0, 1);

	// i/o
	m_igs_mux->set_addrmap(0, &igs017_state::sdmg2_mux_map);

	m_igs017_igs031->in_pa_callback().set_ioport("DSW1");
	m_igs017_igs031->in_pb_callback().set_ioport("DSW2");
	// DSW3 is read but unused (it's not populated on the PCB)

	HOPPER(config, m_hopper, attotime::from_msec(50));

	// protection
	IGS_INCDEC(config, m_igs_incdec, 0);
}


// mgdh, mgdha

TIMER_DEVICE_CALLBACK_MEMBER(igs017_state::mgdh_interrupt)
{
	int scanline = param;

	if (scanline == 240 && m_igs017_igs031->get_irq_enable())
		m_maincpu->set_input_line(1, HOLD_LINE);

	if (scanline == 0 && m_igs017_igs031->get_nmi_enable())
		m_maincpu->set_input_line(3, HOLD_LINE); // lev 3 instead of 2
}

void igs017_state::mgdha(machine_config &config)
{
	base_machine_oki(config, 22_MHz_XTAL / 22);

	M68000(config, m_maincpu, 22_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs017_state::mgdh_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(igs017_state::mgdh_interrupt), "screen", 0, 1);

	// i/o
	m_igs_mux->set_addrmap(0, &igs017_state::mgdha_mux_map);

	m_igs017_igs031->in_pa_callback().set_ioport("DSW1");

	HOPPER(config, m_hopper, attotime::from_msec(50));
}

void igs017_state::mgdh(machine_config &config)
{
	mgdha(config);

	m_igs_mux->set_addrmap(0, &igs017_state::mgdh_mux_map);

	// protection (only used for the game id check?)
//  IGS_STRING(config, m_igs_string, 0);
}

void igs017_state::sdmg2p(machine_config &config)
{
	base_machine_oki(config, 22_MHz_XTAL / 22);

	M68000(config, m_maincpu, 22_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs017_state::sdmg2p_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(igs017_state::mgcs_interrupt), "screen", 0, 1);

	// i/o
	m_igs_mux->set_addrmap(0, &igs017_state::sdmg2p_mux_map);

	m_igs017_igs031->in_pa_callback().set_ioport("DSW1");
	m_igs017_igs031->in_pb_callback().set_ioport("DSW2");
	m_igs017_igs031->in_pc_callback().set_ioport("DSW3"); // there are 3 DIP banks on PCB but only two are shown in test mode

	HOPPER(config, m_hopper, attotime::from_msec(50));

	IGS_STRING(config, m_igs_string, 0);
}


/***************************************************************************
                                ROMs Loading
***************************************************************************/

// These are newer versions of IQ-Block (see iqblock.cpp) with added gambling

/***************************************************************************

Shuzi Leyuan (V127M)
數字樂園 (Shùzì Lèyuán)
IGS, 1996

PCB Layout
----------

IGS PCB NO-0131-4
|---------------------------------------|
|uPD1242H     VOL    U3567   3.579545MHz|
|                               AR17961 |
|   HD64180RP8                          |
|  16MHz                         BATTERY|
|                                       |
|                         SPEECH.U17    |
|                                       |
|J                        6264          |
|A                                      |
|M      8255              V.U18         |
|M                                      |
|A                                      |
|                                       |
|                                       |
|                      |-------|        |
|                      |       |        |
|       CG.U7          |IGS017 |        |
|                      |       |        |
|       TEXT.U8        |-------|   PAL  |
|            22MHz               61256  |
|                   DSW1  DSW2  DSW3    |
|---------------------------------------|
Notes:
      HD64180RP8 - Hitachi HD64180 CPU. Clocks 16MHz (pins 2 & 3), 8MHz (pin 64)
      61256   - 32k x8 SRAM (DIP28)
      6264    - 8k x8 SRAM (DIP28)
      IGS017  - Custom IGS IC (QFP208)
      AR17961 - == Oki M6295 (QFP44). Clock 1.000MHz [16/16]. pin 7 = high
      U3567   - == YM2413. Clock 3.579545MHz
      VSync   - 60Hz
      HSync   - 15.31kHz

***************************************************************************/

ROM_START( iqblocka )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "v.u18", 0x00000, 0x40000, CRC(2e2b7d43) SHA1(cc73f4c8f9a6e2219ee04c9910725558a80b4eb2) )

	ROM_REGION( 0x100000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "cg.u7", 0x000000, 0x080000, CRC(cb48a66e) SHA1(6d597193d1333a97957d5ceec8179a24bedfd928) ) // FIXED BITS (xxxxxxxx0xxxxxxx)
	ROM_FILL(          0x080000, 0x080000, 0xff ) // unpopulated

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0)
	ROM_LOAD( "text.u8", 0x000000, 0x080000, CRC(48c4f4e6) SHA1(b1e1ca62cf6a99c11a5cc56705eef7e22a3b2740) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "speech.u17", 0x00000, 0x40000, CRC(d9e3d39f) SHA1(bec85d1ac2dfca77453cbca0e7dd53fee8fb438b) )

	ROM_REGION( 0x2dd, "pld", ROMREGION_ERASE )
	ROM_LOAD( "cq.u24", 0x000, 0x2dd, NO_DUMP ) // PAL22CV10

	ROM_REGION( 0x15, "igs_fixed_data", 0 )
	ROM_LOAD( "igs_fixed_data.key", 0x00, 0x15, CRC(9159ecbf) SHA1(b6bdb1f327944dfc7f6f71565a24e89517607490) )
ROM_END

// English title (IQ Block) and year 1997:

ROM_START( iqblockf )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "v113fr.u18", 0x00000, 0x40000, CRC(346c68af) SHA1(ceae4c0143c288dc9c1dd1e8a51f1e3371ffa439) )

	ROM_REGION( 0x100000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "cg.u7", 0x000000, 0x080000, CRC(cb48a66e) SHA1(6d597193d1333a97957d5ceec8179a24bedfd928) ) // FIXED BITS (xxxxxxxx0xxxxxxx)
	ROM_FILL(          0x080000, 0x080000, 0xff ) // unpopulated

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "text.u8", 0x000000, 0x080000, CRC(48c4f4e6) SHA1(b1e1ca62cf6a99c11a5cc56705eef7e22a3b2740) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sp.u17", 0x00000, 0x40000, CRC(71357845) SHA1(25f4f7aebdcc0706018f041d3696322df569b0a3) )

	ROM_REGION( 0x2dd, "pld", ROMREGION_ERASE )
	ROM_LOAD( "cq.u24", 0x000, 0x2dd, NO_DUMP ) // PAL22CV10

	ROM_REGION( 0x15, "igs_fixed_data", 0 )
	ROM_LOAD( "igs_fixed_data.key", 0x00, 0x15, CRC(9159ecbf) SHA1(b6bdb1f327944dfc7f6f71565a24e89517607490) )
ROM_END

// IQ Block V110F also exists (undumped) with IGS003e (not 8255)

/***************************************************************************

Genius 6 (V110F)
IGS, 1998

IGS PCB NO-0131-4
|---------------------------------------|
|uPD1242H     VOL    U3567   3.579545MHz|
|                            K668       |
|   Z8018008PSC                         |
|  16MHz                         BATTERY|
|                                       |
|                         SPEECH.U17    |
|                                       |
|J                        6264          |
|A                                      |
|M      IGS003c        V-110F.U18       |
|M                                      |
|A                                      |
|                                       |
|                                       |
|                      |-------|        |
|                      |       |        |
|       CG.U7          |IGS017 |        |
|                      |       |        |
|       TEXT.U8        |-------|   PAL  |
|            22MHz               61256  |
|                   DSW1  DSW2  DSW3    |
|---------------------------------------|

***************************************************************************/

ROM_START( genius6 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "genius6_v110f.u18", 0x00000, 0x40000, CRC(2630ad44) SHA1(37002fa913ad60c59145f5a7692eef8862b9d6eb) )

	ROM_REGION( 0x80000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "genius6_cg.u7", 0x000000, 0x080000, CRC(1842d021) SHA1(78bfb5108741d39bd19b603cc97623fba7b2a31e) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "text.u8", 0x000000, 0x080000, CRC(48c4f4e6) SHA1(b1e1ca62cf6a99c11a5cc56705eef7e22a3b2740) ) // same as iqblocka

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "speech.u17", 0x00000, 0x40000, CRC(d9e3d39f) SHA1(bec85d1ac2dfca77453cbca0e7dd53fee8fb438b) ) // same as iqblocka

	ROM_REGION( 0x2dd, "pld", ROMREGION_ERASE )
	ROM_LOAD( "cq.u24", 0x000, 0x2dd, NO_DUMP ) // PAL22CV10

	ROM_REGION( 0x15, "igs_fixed_data", 0 )
	ROM_LOAD( "igs_fixed_data.key", 0x00, 0x15, CRC(9159ecbf) SHA1(b6bdb1f327944dfc7f6f71565a24e89517607490) )
ROM_END

// strangely identifies as V133F and has 1997 copyright, while the parent is V110F but with 1998 copyright
ROM_START( genius6a )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "genius6_v133.u18", 0x00000, 0x40000, CRC(b34ce8c6) SHA1(845e6c4f5b1f06229a4046cf085ce08802458bd8) )

	ROM_REGION( 0x80000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "genius6_cg.u7", 0x000000, 0x080000, CRC(1842d021) SHA1(78bfb5108741d39bd19b603cc97623fba7b2a31e) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x40000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "text.u8", 0x000000, 0x040000, CRC(7716b601) SHA1(363cddd930fdec4821ebfaced64276f8fa943eae) ) // half sized if compared to the parent and identical to its 1st half, confirmed correct (27c2048)

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "speech.u17", 0x00000, 0x40000, CRC(d9e3d39f) SHA1(bec85d1ac2dfca77453cbca0e7dd53fee8fb438b) ) // same as iqblocka

	ROM_REGION( 0x2dd, "pld", ROMREGION_ERASE )
	ROM_LOAD( "cq.u24", 0x000, 0x2dd, NO_DUMP ) // PAL22CV10

	ROM_REGION( 0x15, "igs_fixed_data", 0 )
	ROM_LOAD( "igs_fixed_data.key", 0x00, 0x15, CRC(9159ecbf) SHA1(b6bdb1f327944dfc7f6f71565a24e89517607490) )
ROM_END

ROM_START( genius6b ) // PCB NO-0132, identical to V133F but for the main CPU ROM
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "genius6_v-132f.u18", 0x00000, 0x40000, CRC(231be791) SHA1(1684395dc93902893dca32952c236617ccdbc269) )

	ROM_REGION( 0x80000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "genius6_cg.u7", 0x000000, 0x080000, CRC(1842d021) SHA1(78bfb5108741d39bd19b603cc97623fba7b2a31e) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "text.u8", 0x000000, 0x040000, CRC(7716b601) SHA1(363cddd930fdec4821ebfaced64276f8fa943eae) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "speech.u17", 0x00000, 0x40000, CRC(d9e3d39f) SHA1(bec85d1ac2dfca77453cbca0e7dd53fee8fb438b) )

	ROM_REGION( 0x2dd, "pld", ROMREGION_ERASE )
	ROM_LOAD( "cq.u24", 0x000, 0x2dd, NO_DUMP ) // PAL22CV10

	ROM_REGION( 0x15, "igs_fixed_data", 0 )
	ROM_LOAD( "igs_fixed_data.key", 0x00, 0x15, CRC(9159ecbf) SHA1(b6bdb1f327944dfc7f6f71565a24e89517607490) )
ROM_END

/***************************************************************************

Tian Jiang Shen Bing (V137C)
天將神兵 (Tiān Jiāng Shén Bīng)
IGS, 1997

This PCB is almost the same as IQBlock (IGS, 1996)
but the 8255 has been replaced with the IGS025 IC

PCB Layout
----------

IGS PCB NO-0157-2
|---------------------------------------|
|uPD1242H     VOL    U3567   3.579545MHz|
|                         AR17961       |
|   HD64180RP8                   SPDT_SW|
|  16MHz                         BATTERY|
|                                       |
|                         S0703.U15     |
|                                       |
|J     |-------|          6264          |
|A     |       |                        |
|M     |IGS025 |          P0700.U16     |
|M     |       |                        |
|A     |-------|                        |
|                                       |
|                                       |
|                      |-------|        |
|                      |       |   PAL  |
|       A0701.U3       |IGS017 |        |
|                      |       |   PAL  |
|       TEXT.U6        |-------|        |
|            22MHz               61256  |
|                   DSW1  DSW2  DSW3    |
|---------------------------------------|
Notes:
      HD64180RP8 - Hitachi HD64180 CPU. Clocks 16MHz (pins 2 & 3), 8MHz (pin 64)
      61256   - 32k x8 SRAM (DIP28)
      6264    - 8k x8 SRAM (DIP28)
      IGS017  - Custom IGS IC (QFP208)
      IGS025  - Custom IGS IC (PLCC68)
      AR17961 - == Oki M6295 (QFP44). Clock 1.000MHz [16/16]. pin 7 = high
      U3567   - == YM2413. Clock 3.579545MHz
      VSync   - 60Hz
      HSync   - 15.30kHz

***************************************************************************/

ROM_START( tjsb )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "p0700.u16", 0x00000, 0x40000,CRC(1b2a50df) SHA1(95a272e624f727df9523667864f933118d9e633c) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "a0701.u3", 0x00000, 0x400000, CRC(27502a0a) SHA1(cca79e253697f47b688ef781b1b6de9d2945f199) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "text.u6", 0x00000, 0x80000,  CRC(3be886b8) SHA1(15b3624ed076640c1828d065b01306a8656f5a9b) ) // BADADDR --xxxxxxxxxxxxxxxxx

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s0703.u15", 0x00000, 0x80000,  CRC(c6f94d29) SHA1(ec413580240711fc4977dd3c96c288501aa7ef6c) )

	ROM_REGION( 0xec, "igs_string", 0 )
	ROM_LOAD( "tjsb_string.key", 0x00, 0xec, CRC(412c83a0) SHA1(c09618aabecdde4c77d2b5695799fc89dfb325bc) )
ROM_END

/***************************************************************************

Manguan Caishen (V103CS)
满贯财神 (Mǎnguàn Cáishén)
IGS, 1998

PCB Layout
----------

IGS PCB NO-0192-1
|---------------------------------------|
|              JAMMA            uPC1242 |
|                                       |
|               S1502.U10               |
|                          K668    VOL  |
|                                       |
|                                       |
|                       22MHz           |
|1     61256                            |
|8              |-------|      TEXT.U25 |
|W     PAL      |       |               |
|A              |IGS017 |               |
|Y              |       |      M1501.U23|
|               |-------|               |
|   |-------|                           |
|   |       |                           |
|   |IGS025 |   P1500.U8                |
|   |       |              PAL    6264  |
|1  |-------|                           |
|0  |----|                 PAL    6264  |
|W  |IGS |                 PAL          |
|A  |029 |  8MHz                 SPDT_SW|
|Y  |----|                 68000        |
|T DSW1  DSW2                   BATTERY |
|---------------------------------------|
Notes:
      Uses JAMMA & common 10-way/18-way Mahjong pinout
      68000 clock 11.000MHz [22/2]
      K668  == Oki M6295 (QFP44). Clock 1MHz [8/8]. pin7 = High
      VSync - 60Hz
      HSync - 15.3kHz

***************************************************************************/

ROM_START( mgcs )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "p1500.u8", 0x00000, 0x80000, CRC(a8cb5905) SHA1(37be7d926a1352869632d43943763accd4dec4b7) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "m1501.u23", 0x000000, 0x400000, CRC(96fce058) SHA1(6b87f47d646bad9b3061bdc8a9af65467fdbbc9f) ) // FIXED BITS (xxxxxxx0xxxxxxxx)

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "text.u25", 0x00000, 0x80000, CRC(a37f9613) SHA1(812f060ca98a34540c48a180c359c3d0f1c0b5bb) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s1502.u10", 0x00000, 0x80000, CRC(a8a6ba58) SHA1(59276a8ab4a31812600816c2a43b74bd71394419) )

	ROM_REGION( 0xec, "igs_string", 0 )
	ROM_LOAD( "mgcs_string.key", 0x00, 0xec, CRC(6cdadd19) SHA1(c2b4ced5d45d0af1ddeeabd0e352fd5383995d32) )
ROM_END

/*********************************************************************************

Man Guan Cai Shen, IGS 1998

PCB Layout
----------

IGS PCB NO- 0208
|-----------------------------------------|
|          JAMMA              VOL TDA1020 |
|1   F521(x25)             7805           |
|8           S1502.U12     M6295          |
|W             22MHz             M1503.U22|
|A       LM2933                           |
|Y                                        |
|                                         |
|IGS029                                   |
|     8MHz        IGS031                  |
|                                M1501.U21|
|                                         |
|                              27C4096.U24|
|             61256                       |
|1   IGS025             68000       6264  |
|0            PAL                         |
|W            PAL                         |
|A   SW1      PAL                         |
|Y   SW2                    SW3     T518B |
|-----------------------------------------|
Notes:
      68000 - Clock 11.000MHz [22/2]
      M6295 - Clock 1.000MHz [22/22]
      SW1/2 - 8-Position DIP Switch
        SW3 - Reset / NVRAM Clear
       6264 - 8kBx8-bit SRAM
      61256 - 32kBx8-bit SRAM

*********************************************************************************/

ROM_START( mgcsa )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "27c4096.u24", 0x00000, 0x80000, CRC(c41b7530) SHA1(1f9f821658c50b84b2e8cce97ffea8349fdae54f) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "m1501.u21", 0x000000, 0x400000, CRC(96fce058) SHA1(6b87f47d646bad9b3061bdc8a9af65467fdbbc9f) ) // FIXED BITS (xxxxxxx0xxxxxxxx)

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "m1503.u22", 0x00000, 0x80000, CRC(a37f9613) SHA1(812f060ca98a34540c48a180c359c3d0f1c0b5bb) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s1502.u12", 0x00000, 0x80000, CRC(a8a6ba58) SHA1(59276a8ab4a31812600816c2a43b74bd71394419) )

	ROM_REGION( 0xec, "igs_string", 0 )
	ROM_LOAD( "mgcs_string.key", 0x00, 0xec, BAD_DUMP CRC(6cdadd19) SHA1(c2b4ced5d45d0af1ddeeabd0e352fd5383995d32) ) // TODO: seems the same as the parent, but double-check
ROM_END

/***************************************************************************

Chaoji Damanguan II (China, V754C)
超級大滿貫 (Chāojí Dàmǎnguàn)
IGS, 1997

PCB Layout
----------

IGS PCB NO-0147-6
|---------------------------------------|
| uPC1242H          S0903.U15   BATTERY |
|          VOL               SPDT_SW    |
|                                       |
|        K668                    6264   |
|                                       |
|                                6264   |
|                   PAL                 |
|1   8255                               |
|8                            P0900.U25 |
|W                                      |
|A                                      |
|Y                                      |
|                                68000  |
|                                       |
|    M0902.U4       PAL                 |
|                                       |
|                                 PAL   |
|1   M0901.U5       |-------|           |
|0                  |       |     PAL   |
|W                  |IGS031 |           |
|A   TEXT.U6        |       |           |
|Y                  |-------|     62256 |
|T         22MHz  DSW1 DSW2             |
|---------------------------------------|
Notes:
      Uses common 10-way/18-way Mahjong pinout
      68000 clock 11.000MHz [22/2]
      K668 = M6295. clock 1.000MHz [22/22]. pin7 = High
      VSync - 60Hz
      HSync - 15.3kHz

***************************************************************************/

ROM_START( sdmg2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "p0900.u25", 0x00000, 0x80000,CRC(43366f51) SHA1(48dd965dceff7de15b43c2140226a8b17a792dbc) )

	ROM_REGION( 0x280000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "m0901.u5", 0x000000, 0x200000, CRC(9699db24) SHA1(50fc2f173c20b48d10595f01f1e9545f1b13a61b) ) // FIXED BITS (xxxxxxxx0xxxxxxx)
	ROM_LOAD( "m0902.u4", 0x200000, 0x080000, CRC(3298b13b) SHA1(13b21ddeed368b7f4fea1408c8fc511244342faf) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x20000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "text.u6", 0x000000, 0x020000, CRC(cb34cbc0) SHA1(ceedbdda085fd1acc9a575502bdf7cf998f54f05) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s0903.u15", 0x00000, 0x80000, CRC(ae5a441c) SHA1(923774ef73ab0f70e0db1738a4292dcbd70d2384) )
ROM_END

/***************************************************************************

Maque Wangchao / Chaoji Damanguan 2 - Jiaqiang Ban
麻雀王朝 / 超級大滿貫 2 -加強版 (Mahjong Dynasty / Super Grand Slam 2 - Enhanced Edition)

IGS 2000
Hardware info By Guru
---------------------

IGS PCB NO-0271
|--------------------------------------|
|PC817x19        JAMMA                 |
|      |-----|           MA.DY_TEXT.U18|
|      | IGS |                         |
|      | 025 |                         |
|      |-----| 22MHz                   |
|             |-------|                |
|             | IGS031|   IGS_M0906.U20|
|PC817x6      |       |                |
|    22V10    |       | MA.DY_V100C.U21|
| SW1         |-------|                |
| SW2     61256|------|                |
| SW3          |68000 |    22V10   BATT|
|              |      |        6264    |
|     16V8     |------|                |
|LM7805          K668                  |
|                                  SW4 |
|TDA1020  VOL       MA.DY_SP.U14       |
|--------------------------------------|
Notes:
      68000 - Clock 11.0MHz [22/2]
       K668 - ==OKI M6295. Clock 1.0MHz [22/22]. Pin 7 HIGH
      61256 - 32kB x8-bit SRAM
       6262 - 8kB x8-bit SRAM (battery-backed)
      SW1-3 - 8-position DIP Switch
        SW4 - High Score Reset / Back-up Battery RAM Re-initialize / PCB Reset
      PC817 - Sharp PC817 Optocoupler
       16V8 - Atmel ATF16V8B PLD
      22V10 - Atmel ATF22V10B PLD
    TDA1020 - Audio Power Amp
    U18/U21 - 27C4002 EPROM
        U14 - 27C040 EPROM
        U20 - MX23C3210 mask ROM

***************************************************************************/

ROM_START( sdmg2p )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "ma.dy_v100c.u21", 0x00000, 0x80000,CRC(c071270e) SHA1(8b55a80da30f4233c862bb5d8a79a76af634a296) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "igs_m0906.u20", 0x000000, 0x400000, CRC(01ea0a60) SHA1(66f083084f6d9e8dc4d1d50f3c5bcf2b79025fc0) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "ma.dy_text.u18", 0x000000, 0x080000, CRC(e46a3a52) SHA1(7b3f113170904dc474712a6a76162a8ee5dbd318) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "ma.dy_sp.u14", 0x00000, 0x80000, CRC(b31c6349) SHA1(9e8e5b029e1eff47581f99ecf2da3f17bee01f32) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0xec, "igs_string", 0 )
	ROM_LOAD( "sdmg2p_string.key", 0x00, 0xec, NO_DUMP )
ROM_END

/***************************************************************************

Long Hu Zhengba 2 (set 1)
龙虎争霸 (Lóng Hǔ Zhēngbà)
IGS, 1998

PCB Layout
----------

IGS PCB NO-0206
|---------------------------------------|
|    6264             |-------|         |
|    6264      |----| |       |         |
|              |IGS | |IGS025 |         |
|              |022 | |       |  PAL    |
|              |----| |-------|         |
|                       PAL             |
|    M1104.U11          PAL    68000    |
|1                                      |
|8                                      |
|W                                      |
|A   M1101.U6  8MHz          P1100.U30  |
|Y                                      |
|                                  6264 |
|                                       |
|              |-------|                |
|              |       |                |
|              |IGS031 |           61256|
|1             |       |                |
|0   M1103.U8  |-------|                |
|W      22MHz                           |
|A             DSW1   DSW2              |
|Y            K668     BATTERY          |
|TDA1020 VOL          S1102.U23  SPDT_SW|
|---------------------------------------|
Notes:
      Uses common 10-way/18-way Mahjong pinout
      68000 clock 11.000MHz [22/2]
      K668  == Oki M6295 (QFP44). Clock 1MHz [8/8]. pin7 = High
      VSync - 60Hz
      HSync - 15.3kHz

***************************************************************************/

ROM_START( lhzb2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "p1100.u30", 0x00000, 0x80000, CRC(68102b25) SHA1(6c1e8d204be0efda0e9b6c2f49b5c6760712475f) )

	ROM_REGION( 0x10000, "igs022", 0 )
	ROM_LOAD( "m1104.u11",0x0000, 0x10000, CRC(794d0276) SHA1(ac903d2faa3fb315438dc8da22c5337611a8790d) ) // INTERNATIONAL GAMES SYSTEM CO.,LTD

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "m1101.u6", 0x000000, 0x400000, CRC(0114e9d1) SHA1(5b16170d3cd8b8e1662c949b7234fbdd2ca927f7) ) // FIXED BITS (0xxxxxxxxxxxxxxx)

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "m1103.u8", 0x00000, 0x80000, CRC(4d3776b4) SHA1(fa9b311b1a6ad56e136b66d090bc62ed5003b2f2) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s1102.u23", 0x00000, 0x80000, CRC(51ffe245) SHA1(849011b186096add657ab20d49d260ec23363ef3) )

	ROM_REGION( 0xec, "igs_string", 0 )
	ROM_LOAD( "lhzb2_string.key", 0x00, 0xec, CRC(c964dc35) SHA1(81036e0dfa9abad123701ae8939d0d5b6f91b015) )
ROM_END

// VS221M: alt hardware, no IGS022 protection chip

ROM_START( lhzb2a )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "p-4096", 0x00000, 0x80000, CRC(41293f32) SHA1(df4e993f4a458729ade13981e58f32d8116c0082) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "m1101.u6", 0x000000, 0x400000, CRC(0114e9d1) SHA1(5b16170d3cd8b8e1662c949b7234fbdd2ca927f7) ) // FIXED BITS (0xxxxxxxxxxxxxxx)

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "m1103.u8", 0x00000, 0x80000, CRC(4d3776b4) SHA1(fa9b311b1a6ad56e136b66d090bc62ed5003b2f2) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s1102.u23", 0x00000, 0x80000, CRC(51ffe245) SHA1(849011b186096add657ab20d49d260ec23363ef3) )

	ROM_REGION( 0xec, "igs_string", 0 )
	ROM_LOAD( "lhzb2_string.key", 0x00, 0xec, CRC(c964dc35) SHA1(81036e0dfa9abad123701ae8939d0d5b6f91b015) )
ROM_END

/***************************************************************************

Shuang Long Qiang Zhu 2 VS (China, VS203J)
双龙抢珠 (Shuāng Lóng Qiǎng Zhū)
IGS, 1998

PCB Layout
----------

IGS PCB NO-0207
|---------------------------------------|
|                   K668  S1102.U20     |
|     PAL                               |
| 8MHz                     6264         |
|                                       |
|    |----|                6264         |
|    |IGS |                             |
|    |022 |  M1103.U12       PAL        |
|J   |----|                    PAL      |
|A                                      |
|M                                      |
|M      |-------|                       |
|A      |       |                       |
|       |IGS025 |   68000               |
|       |       |                       |
|       |-------|                       |
|                            P1100.U28  |
|                 PAL                   |
|  M1101.U4       |-------|             |
|                 |       |             |
|                 |IGS031 |      6264   |
|  TEXT.U6        |       |             |
|                 |-------|      62256  |
|SPDT_SW   22MHz   DSW1  DSW2  BATTERY  |
|---------------------------------------|
Notes:
      68000 clock 11.000MHz [22/2]
      K668  == Oki M6295 (QFP44). Clock 1.000MHz [8/8]. pin7 = High
      VSync - 60Hz
      HSync - 15.3kHz

***************************************************************************/

ROM_START( slqz2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "p1100.u28", 0x00000, 0x80000, CRC(0b8e5c9e) SHA1(16572bd1163bba4da8a76b10649d2f71e50ad369) )

	ROM_REGION( 0x10000, "igs022", 0 )
	ROM_LOAD( "m1103.u12", 0x00000, 0x10000, CRC(9f3b8d65) SHA1(5ee1ad025474399c2826f21d970e76f25d0fa1fd) ) // INTERNATIONAL GAMES SYSTEM CO.,LTD

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "m1101.u4", 0x000000, 0x400000, CRC(0114e9d1) SHA1(5b16170d3cd8b8e1662c949b7234fbdd2ca927f7) ) // FIXED BITS (0xxxxxxxxxxxxxxx)

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "text.u6", 0x00000, 0x80000, CRC(40d21adf) SHA1(18b202d6330ac89026bec2c9c8224b52540dd48d) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s1102.u20", 0x00000, 0x80000, CRC(51ffe245) SHA1(849011b186096add657ab20d49d260ec23363ef3) ) // = s1102.u23 Long Hu Zhengba 2

	ROM_REGION( 0xec, "igs_string", 0 )
	ROM_LOAD( "slqz2_string.key", 0x00, 0xec, CRC(5ca22f9d) SHA1(a795415016fdcb6329623786dc992ac7b0877ddf) )
ROM_END

/*********************************************************************************

An older version of Shuang Long Qiang Zhu 2 VS

PCB Layout
----------

IGS PCB NO-0182-1
|-----------------------------------------|
|              6264                    SW3|
|1             6264   27C4096.U25         |
|8    IGS025                              |
|W   (LABEL N2)                      6264 |
|A                                        |
|Y                                   6264 |
|                                         |
|                              68000      |
|  IGS022    26C512.U12                   |
|    32.768kHz                            |
|            M1101.U13                PAL |
|   8MHz                              PAL |
|1                                    PAL |
|0         27C4096.U15                PAL |
|W                        IGS017          |
|A  VOL          22MHz              61256 |
|Y     7805                               |
|UPC1242   K668  S1102.U22     SW1  SW2   |
|-----------------------------------------|
Notes:
      68000 - Clock 11.000MHz [22/2]
       K668 - Oki M6295 clone. Clock 1.000MHz [22/22]
      SW1/2 - 8-Position DIP Switch
        SW3 - Reset / NVRAM Clear
      61256 - 32kBx8-bit SRAM
       6264 - 8kBx8-bit SRAM
     IGS022 - IGS custom. 32.768kHz crystal is tied to this IC so it has a real time clock integrated into it.

*********************************************************************************/


ROM_START( slqz2a )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "27c4096.u25", 0x00000, 0x80000, NO_DUMP )// dead ROM

	ROM_REGION( 0x10000, "igs022", 0 )
	ROM_LOAD( "26c512.u12",0x0000, 0x10000, CRC(794d0276) SHA1(ac903d2faa3fb315438dc8da22c5337611a8790d) ) // INTERNATIONAL GAMES SYSTEM CO.,LTD

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "m1101.u13", 0x000000, 0x400000, CRC(0114e9d1) SHA1(5b16170d3cd8b8e1662c949b7234fbdd2ca927f7) ) // FIXED BITS (0xxxxxxxxxxxxxxx)

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "27c4096.u15", 0x00000, 0x80000, CRC(4d3776b4) SHA1(fa9b311b1a6ad56e136b66d090bc62ed5003b2f2) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s1102.u20", 0x00000, 0x80000, CRC(51ffe245) SHA1(849011b186096add657ab20d49d260ec23363ef3) )

	ROM_REGION( 0xec, "igs_string", 0 )
	ROM_LOAD( "slqz2a_string.key", 0x00, 0xec, BAD_DUMP CRC(5ca22f9d) SHA1(a795415016fdcb6329623786dc992ac7b0877ddf) ) // TODO, if / when program ROM is dumped
ROM_END


/***************************************************************************

Manguan Daheng (V123T1)
滿貫大亨 (Mǎnguàn Dàhēng)
(c) 1997 IGS

PCB Layout
----------

IGS PCB NO-0252
|----------------------------------|
|    S1002.U22   6264   62256  SW  |
|TDA1020  FLASH.U19     PAL    BATT|
|  LM7805 M6295                    |
|   VOL                            |
|ULN2004                           |
|J             68000      IGS031   |
|A    DSW2                         |
|M                                 |
|M                                 |
|A           PAL PAL               |
|     IGS025                       |
|                                  |
|               M1001.U4      22MHz|
|                       TEXT.U6    |
|                              DSW1|
|    18WAY               10WAY     |
|----------------------------------|
Notes:
      68000     - Clock 11.000MHz [22/2]
      M6295     - Clock 1.000MHz [22/22]. Pin 7 HIGH
      DSW1/2    - 8-position Dip Switches
      SW        - Backup RAM Clear and Reset Switch
      FLASH.U19 - MX29F400 4M TSOP48 mounted onto a DIP adapter and plugged into a
                  socket. Under the socket is written '27C4096'
      M1001.U4  - 32M DIP42 Mask ROM
      S1002.U22 - 4M DIP32 Mask ROM
      TEXT.U6   - 27C1024 EPROM

***************************************************************************/

ROM_START( mgdha )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "flash.u19", 0x00000, 0x80000, CRC(ff3aed2c) SHA1(829140e6fc7e4dfc039b0e7b647ce26d59b23b3d) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "m1001.u4", 0x000000, 0x400000, CRC(0cfb60d6) SHA1(e099aca730e7fd91a72915c27e569ad3d21f0d8f) ) // FIXED BITS (xxxxxxx0xxxxxxxx)

	ROM_REGION( 0x20000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD16_WORD_SWAP( "text.u6", 0x00000, 0x20000, CRC(db50f8fc) SHA1(e2ce4a42f5bdc0b4b7988ad9e8d14661f17c3d51) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s1002.u22", 0x00000, 0x80000, CRC(ac6b55f2) SHA1(7ff91fd1107272ad6bce071dc9ae2f374ebf5e3e) )
ROM_END

/***************************************************************************

Manguan Daheng (V125T1)
滿貫大亨 (Mǎnguàn Dàhēng)
(c) 1997 IGS

No hardware info, no sprites rom for this set.
It has an additional game id check at the start.

***************************************************************************/

ROM_START( mgdh )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "igs_f4bd.125", 0x00000, 0x80000, CRC(8bb0b870) SHA1(f0313f0b8b7575f4fff1feb99d48699d50556ef5) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	// not in this set
	ROM_LOAD( "m1001.u4", 0x000000, 0x400000, CRC(0cfb60d6) SHA1(e099aca730e7fd91a72915c27e569ad3d21f0d8f) ) // FIXED BITS (xxxxxxx0xxxxxxxx)

	ROM_REGION( 0x20000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "igs_512e.u6", 0x00000, 0x20000, CRC(db50f8fc) SHA1(e2ce4a42f5bdc0b4b7988ad9e8d14661f17c3d51) ) // == text.u6

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "ig2_8836.u14", 0x00000, 0x80000, CRC(ac1f4da8) SHA1(789a2e0b58750292909dabca42c7e5ad72af3db5) )
ROM_END

/***************************************************************************

Tarzan Chuang Tian Guan (V109C)
泰山闯天关 (Tàishān Chuǎng Tiān Guān)
IGS 1999

PCB Layout
----------

IGS PCB NO-0248-1
|------------------------------|
|          J A M M A           |
|1  22MHz  T2105.U5  A2104.U15 |
|8                   DIP42     |
|W                             |
|A          IGS031     U19     |
|Y    24257                    |
|                              |
|           IGS025      PAL.U20|
|     DSW1                     |
|                    Z180      |
|     DSW2          16MHz      |
|1                      PAL.U21|
|0    DSW3                     |
|W                        BATT |
|A     VOL    LM7805     RES_SW|
|Y uPC1242    M6295     U14    |
|------------------------------|
Notes:
      IGS025 - custom IGS chip labelled 'TARZAN 1'
      IGS031 - custom IGS chip
      VSync  - 60.0060Hz
      HSync  - 15.3002kHz
      M6295  - Clock 1.000MHz [16/16], pin 7 high
      Z180   - Clock 16.000MHz
      DIP42  - Empty socket
      U19    - 28F2000 Flash ROM (DIP32)
      U14    - 23C4000 mask ROM (DIP32)
      U5     - 23C2048 mask ROM (DIP40)
      U15    - 23C3210 mask ROM (DIP42)
      24257  - 32kx8 SRAM

***************************************************************************/

// IGS PCB NO-0248-1
ROM_START( tarzanc )
	ROM_REGION( 0x40000, "maincpu", 0 ) // V109C TARZAN C (same as tarzan set)
	ROM_LOAD( "u19", 0x00000, 0x40000, CRC(e6c552a5) SHA1(f156de9459833474c85a1f5b35917881b390d34c) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "igs_a2104_cg_v110.u15", 0x00000, 0x400000, CRC(dcbff16f) SHA1(2bf77ef4448c26124c8d8d18bb7ffe4105cfa940) ) // FIXED BITS (xxxxxxx0xxxxxxxx)

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "igs_t2105_cg_v110.u5", 0x00000, 0x80000, CRC(1d4be260) SHA1(6374c61735144b3ff54d5e490f26adac4a10b14d) ) // 27C4096 (27C2048 printed on the PCB)

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igs_s2102_sp_v102.u14", 0x00000, 0x80000, CRC(90dda82d) SHA1(67fbc1e8d76b85e124136e2f1df09c8b6c5a8f97) )

	ROM_REGION( 0x2dd * 2, "plds", ROMREGION_ERASE )
	ROM_LOAD( "eg.u20", 0x000, 0x2dd, NO_DUMP )
	ROM_LOAD( "eg.u21", 0x2dd, 0x2dd, NO_DUMP )

	ROM_REGION( 0xec, "igs_string", 0 )
	ROM_LOAD( "tarzan_string.key", 0x00, 0xec, CRC(595fe40c) SHA1(0b46983400d237d8bde97a72eaa99b718a03387e) )
ROM_END

// sets below are guesswork, assembled from partial dumps...

// IGS NO-0248-1? Mislabeled?
ROM_START( tarzan )
	ROM_REGION( 0x40000, "maincpu", 0 ) // V109C TARZAN C (same as tarzanc set)
	ROM_LOAD( "0228-u16.bin", 0x00000, 0x40000, CRC(e6c552a5) SHA1(f156de9459833474c85a1f5b35917881b390d34c) )

	ROM_REGION( 0x80000, "igs017_igs031:sprites", ROMREGION_ERASE )
	ROM_LOAD( "sprites.u15", 0x00000, 0x80000, NO_DUMP )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "0228-u6.bin", 0x00000, 0x80000, CRC(55e94832) SHA1(b15409f4f1264b6d1218d5dc51c5bd1de2e40284) )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE )
	ROM_LOAD( "sound.u14", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION( 0x2dd * 2, "plds", 0 )
	ROM_LOAD( "pal1", 0x000, 0x2dd, NO_DUMP )
	ROM_LOAD( "pal2", 0x2dd, 0x2dd, NO_DUMP )

	ROM_REGION( 0xec, "igs_string", 0 )
	ROM_LOAD( "tarzan_string.key", 0x00, 0xec, CRC(595fe40c) SHA1(0b46983400d237d8bde97a72eaa99b718a03387e) )
ROM_END

// IGS NO-0228? This allegedly has IGS029 protection
ROM_START( tarzana )
	ROM_REGION( 0x40000, "maincpu", 0 ) // V107 TAISAN
	ROM_LOAD( "0228-u21.bin", 0x00000, 0x40000, CRC(80aaece4) SHA1(07cad92492c5de36c3915867ed4c6544b1a30c07) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_IGNORE(                        0x40000 )

	ROM_REGION( 0x80000, "igs017_igs031:sprites", ROMREGION_ERASE )
	ROM_LOAD( "sprites.u17", 0x00000, 0x80000, NO_DUMP )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "0228-u6.bin", 0x00000, 0x80000, CRC(55e94832) SHA1(b15409f4f1264b6d1218d5dc51c5bd1de2e40284) )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE )
	ROM_LOAD( "sound.u16", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION( 0x2dd * 2, "plds", ROMREGION_ERASE )
	ROM_LOAD( "pal1", 0x000, 0x2dd, NO_DUMP )
	ROM_LOAD( "pal2", 0x2dd, 0x2dd, NO_DUMP )

	ROM_REGION( 0xec, "igs_string", 0 )
	ROM_LOAD( "tarzan_string.key", 0x00, 0xec, CRC(595fe40c) SHA1(0b46983400d237d8bde97a72eaa99b718a03387e) )
ROM_END

// IGS PCB NO-0248
ROM_START( tarzanb ) // V110 TARZAN C
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "t.z._v110.u19", 0x00000, 0x40000, CRC(16026d12) SHA1(df08641b4bc1437648f0a8cd5f7a8a4786c07041) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", ROMREGION_ERASE00 )
	ROM_LOAD( "igs_a2103_cg_v100f.u15", 0x000000, 0x200000, CRC(afe56ed5) SHA1(656cee6a59f2930eec9acd11b84b416cc7354e01) )

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "t.z._text_u5.u5", 0x00000, 0x80000, CRC(1724e039) SHA1(d628499b61f98f7c9034d70b82ee25e002190ece) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igs_s2102_sp_v102.u14", 0x00000, 0x80000, CRC(90dda82d) SHA1(67fbc1e8d76b85e124136e2f1df09c8b6c5a8f97) ) // not dumped for this set, but same markings as tarzanc's one

	ROM_REGION( 0x2dd * 2, "plds", ROMREGION_ERASE )
	ROM_LOAD( "eg.u20", 0x000, 0x2dd, NO_DUMP )
	ROM_LOAD( "eg.u21", 0x2dd, 0x2dd, NO_DUMP )

	ROM_REGION( 0xec, "igs_string", 0 )
	ROM_LOAD( "tarzanb_string.key", 0x00, 0xec, CRC(595fe40c) SHA1(0b46983400d237d8bde97a72eaa99b718a03387e) )
ROM_END

/***************************************************************************

Super Tarzan (Italy, V100I)
IGS PCB NO-0230-1

      ----|     10/WAY CONN     |--------------------------------|
      |   |---------------------|                       |------| |
     _|                                                 | U23  | |
    |_|  [  U1  ][  U5  ] |--------------|   |-----|    | TDA  | |
    |_|                   | U8 IGS S2102 |   | U17 |    | 1020 | |
    |_|          [  U6  ] |--------------|   |-----|    |------| |
    |_|                   |--------------|                       |
    |_|                   | U9 SP V100I  | [   U15   ]   LM7805  |
 3  |_|                   |--------------| |-------------------| |
 6  |_|       |----------|                 |  U16 Z8018008PSC  | |
 W  |_|       | U4       | [ TAR97 U10-1 ] |-------------------| |
 A  |_|       | IGS025   |                    OSC                |
 Y  |_|       | S_TARZAN |     [   U13   ] 16.0Mhz [ TAR97 U20 ] |
    |_|       |          |                                       |
 C  |_|       |----------|                      [ U18 ]  [ U20 ] |
 O  |_|                    |-----------|                         |
 N  |_|                    | U12       |                   SW1   |
 N  |_|                    | IGS 031   |                         |
    |_|                    | F00030142 |                   SW2   |
    |_|                    |           |        [ U19 ]          |
    |_|                    |-----------|                   SW3   |
    |_| |----------------|                22.00 Mhz              |
    |_| | U2  TBM27C4096 |                                       |
    |_| |----------------|   [ R20 Ohm 5W ]        Battery       |
    |_| |----------------| |----------------------| (---)  Reset |
      | | U3  C0057209   | | U11 IGST2105 CG V110 | (3.6)  SW4   |
      | |----------------| |----------------------| (---)    \   |
      |----------------------------------------------------------|

      U1,U5,U6  ULN2004A               SW4 1pos switch for reset
            U2  TBM TB27C4096          Sw1-2-3  8x2 DSW
            U4  IGS025 (protection?)
            U8  IGS S2102 SP V102 1P1327A6 C000538
           U11  IGS T2105 CG V110 1P1379C1 S000938
           U12  IGS031 F00030142 (graphic array?)
           U17  K668 = Oki M6295 (QFP44). Clock 1.000MHz [8/8]. pin7 = High
           U16  Zilog Z8018008PSC Z180 MPU
           U18  DN74LS14N
           U19  SN74HC132N
           U20  HD74LS161AP
   U10-1,U20-1  PALCE22V10H (read protected)

***************************************************************************/

ROM_START( starzan )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "sp_tarzan_v100i.u9", 0x00000, 0x40000, CRC(64180bff) SHA1(b08dbe8a17ca33024442ebee41f111c8f98a2109) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "igs_a2104_cg_v110.u3", 0x000000, 0x400000, BAD_DUMP CRC(dcbff16f) SHA1(2bf77ef4448c26124c8d8d18bb7ffe4105cfa940) ) // FIXED BITS (xxxxxxx0xxxxxxxx), not dumped for this board, but same label as the one from tarzanc, assuming same contents for now
	ROM_LOAD( "sp_tarzan_cg.u2",      0x200000, 0x080000, CRC(884f95f5) SHA1(2e526aa966e90dc696a8b392a5a99e14f03c4bd4) ) // FIXED BITS (xxxxxxx0xxxxxxxx), overlay (handwritten label)

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "igs_t2105_cg_v110.u11", 0x00000, 0x80000, BAD_DUMP CRC(1d4be260) SHA1(6374c61735144b3ff54d5e490f26adac4a10b14d) ) // not dumped for this board, but same label as the one from tarzanc, assuming same contents for now

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igs_s2102_sp_v102.u8", 0x00000, 0x80000, BAD_DUMP CRC(90dda82d) SHA1(67fbc1e8d76b85e124136e2f1df09c8b6c5a8f97) ) // not dumped for this board, but same label as the one from tarzanc, assuming same contents for now

	ROM_REGION( 0x2dd * 2, "plds", ROMREGION_ERASE )
	ROM_LOAD( "palce22v10h_tar97_u10-1.u10", 0x000, 0x2dd, NO_DUMP ) // read protected
	ROM_LOAD( "palce22v10h_tar97_u20.u20",   0x2dd, 0x2dd, NO_DUMP ) // ""

	ROM_REGION( 0xec, "igs_string", 0 )
	ROM_LOAD( "starzan_string.key", 0x00, 0xec, CRC(b33f5050) SHA1(900d3c48944dbdd95d9e48d74c355e82e00ac012) )
ROM_END


/***************************************************************************

Happy Skill (Italy, V611IT)

IGS PCB NO-0281

Main CPU is a Zilog Z180 clocked @16MHz (XTAL and EXTAL pins directly tied to a 16MHz crystal)
OKI MSM6295 (actually a rebadged one marked 'K668 0003') clocked @1MHz, pin 7 is HIGH
A QFP208 custom ASIC marked 'IGS 031'
A PLCC68 custom IC marked 'IGS025 A9B2201 9931' (label: '590H/S V300')
A Ni-MH 3.6V battery as seen in other IGS hardware

***************************************************************************/

ROM_START( happyskl )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "v611.u9", 0x00000, 0x40000, CRC(1fb3da98) SHA1(60674af9f5c53298b8ef856f1986c905b9bd7b96) ) // handwritten label

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "igs_a2701_cg_v100.u3", 0x000000, 0x400000, CRC(f3756a51) SHA1(8dd4677584f309cec4b068be9f9370a7a172a031) ) // FIXED BITS (xxxxxxx0xxxxxxxx) - 1xxxxxxxxxxxxxxxxxxxxx = 0x00
	ROM_LOAD( "happyskill_cg.u2",     0x200000, 0x080000, CRC(297a1893) SHA1(9be9e2cdaba1615ea376f3fb7087bf990e68b3b4) ) // FIXED BITS (xxxxxxx0xxxxxxxx)

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "happyskill_text.u11", 0x00000, 0x80000, CRC(c6f51041) SHA1(81a9a03e92c1c67f299113dec9e05ba77395ea31) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igs_s2702_sp_v100.u8", 0x00000, 0x80000, CRC(0ec9b1b5) SHA1(b8c7e068ddf6777a184339e6796be33e442a3df4) )

	ROM_REGION( 0x2dd * 2, "plds", ROMREGION_ERASE )
	ROM_LOAD( "atf22v10c.u10", 0x000, 0x2dd, NO_DUMP )
	ROM_LOAD( "et_u20-c.u20",  0x2dd, 0x2dd, NO_DUMP ) // peel22cv10a
ROM_END


// PCB was heavily corroded and not working
ROM_START( cpoker2 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "u9.bin", 0x00000, 0x40000, CRC(8d79eb4d) SHA1(9cad09013f83335ec78c3ff78715bc5d9a989eb7) )

	ROM_REGION( 0x400000, "igs017_igs031:sprites", 0 )
	// the following ROM wasn't readable on this PCB, but it's the same as the one in happyskl. Assuming same contents for now
	ROM_LOAD( "igs_a2701_cg_v100.u3", 0x00000, 0x400000, BAD_DUMP CRC(f3756a51) SHA1(8dd4677584f309cec4b068be9f9370a7a172a031) ) // FIXED BITS (xxxxxxx0xxxxxxxx) - 1xxxxxxxxxxxxxxxxxxxxx = 0x00
	// U2 (overlay) not populated

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "u11.bin", 0x00000, 0x80000, CRC(34475c83) SHA1(376ff68d89c25471483b074dcf7542f42f954e67) ) // 1xxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igs_s2702_sp_v100.u8", 0x00000, 0x80000, CRC(0ec9b1b5) SHA1(b8c7e068ddf6777a184339e6796be33e442a3df4) ) // same as happyskl

	ROM_REGION( 0x2dd * 2, "plds", ROMREGION_ERASE )
	ROM_LOAD( "peel22cv10h.u10", 0x000, 0x2dd, NO_DUMP ) // PALCE22V10H-25P
	ROM_LOAD( "peel22cv10h.u20", 0x2dd, 0x2dd, NO_DUMP ) // PALCE22V10H-25P
ROM_END


/***************************************************************************

Super Poker (V100xD03) / Formosa

PCB NO-0187

CPU Z8018008psc
IGS017
IGS025 (labels: B2 / FORMOSA)
K668 (AD-65)
UM3567 (YM2413)
Audio Xtal 3.579545 MHz
CPU Xtal 16 MHz
3 x DSW8 (SW1-SW3)
Push Button (SW4)
3.6V Battery + Toggle Switch (SW5)

***************************************************************************/

ROM_START( spkrform )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "super2in1_v100xd03.u29", 0x00000, 0x40000, CRC(e8f7476c) SHA1(e20241d68d22ee01a65f5d7921fe2291077f081f) )

	ROM_REGION( 0x100000, "igs017_igs031:sprites", 0 )
	ROM_LOAD( "super2in1.u26", 0x00000, 0x80000, CRC(af3b1d9d) SHA1(ce84b076939d2c9d959cd430d4f5664f32735d60) ) // FIXED BITS (xxxxxxxx0xxxxxxx)
	ROM_LOAD( "super2in1.u25", 0x80000, 0x80000, CRC(7ebaf0a0) SHA1(c278810742cd7e1daa89a93fd7fe82495543ccbf) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x80000, "igs017_igs031:tilemaps", 0 )
	ROM_LOAD( "super2in1.u24", 0x00000, 0x40000, CRC(54d68c49) SHA1(faad78779c3a5b4ecb1c733192d9477ce3324f71) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "super2in1_sp.u28", 0x00000, 0x40000, CRC(33e6089d) SHA1(cd1ad01e92c18bbeab3fe3ea9152f8b0a3eb1b29) )

	ROM_REGION( 0x2dd, "plds", ROMREGION_ERASE )
	ROM_LOAD( "dn.u18", 0x000, 0x2dd, NO_DUMP )

	ROM_REGION( 0xec, "igs_string", 0 )
	ROM_LOAD( "spkrform_string.key", 0x00, 0xec, CRC(17a9021a) SHA1(41943e08f9c9be49fc3705e6f2702d504ec6d078) )
ROM_END

} // anonymous namespace

GAME ( 1996,  iqblocka, iqblock,  iqblocka, iqblocka, igs017_state, init_iqblocka, ROT0, "IGS", "Shuzi Leyuan (China, V127M, gambling)",                             0 ) // 數字樂園
GAME ( 1997,  iqblockf, iqblock,  iqblockf, iqblockf, igs017_state, init_iqblocka, ROT0, "IGS", "IQ Block (V113FR, gambling)",                                       0 )
GAME ( 1997,  mgdh,     0,        mgdh,     mgdh,     igs017_state, init_mgdh,     ROT0, "IGS", "Manguan Daheng (Taiwan, V125T1)",                                   MACHINE_IMPERFECT_COLORS | MACHINE_UNEMULATED_PROTECTION) // 滿貫大亨, wrong colors in betting screen, game id check (patched out)
GAME ( 1997,  mgdha,    mgdh,     mgdha,    mgdh,     igs017_state, init_mgdha,    ROT0, "IGS", "Manguan Daheng (Taiwan, V123T1)",                                   0 ) // 滿貫大亨
GAME ( 1997,  sdmg2,    0,        sdmg2,    sdmg2,    igs017_state, init_sdmg2,    ROT0, "IGS", "Chaoji Damanguan II (China, V754C)",                                0 ) // 超級大滿貫II
GAME ( 1997,  tjsb,     0,        tjsb,     tjsb,     igs017_state, init_tjsb,     ROT0, "IGS", "Tian Jiang Shen Bing (China, V137C)",                               MACHINE_UNEMULATED_PROTECTION ) // 天將神兵, fails the bonus round protection check (if enabled via DSW), see e.g. demo mode
GAME ( 1998,  genius6,  0,        genius6,  genius6,  igs017_state, init_iqblocka, ROT0, "IGS", "Genius 6 (V110F)",                                                  0 ) // shows Chinese text in puzzle game
GAME ( 1997,  genius6a, genius6,  genius6,  genius6,  igs017_state, init_iqblocka, ROT0, "IGS", "Genius 6 (V133F)",                                                  0 ) // clone because it has older copyright year
GAME ( 1997,  genius6b, genius6,  genius6,  genius6,  igs017_state, init_iqblocka, ROT0, "IGS", "Genius 6 (V132F)",                                                  0 ) // "
GAME ( 1998,  mgcs,     0,        mgcs,     mgcs,     igs017_state, init_mgcs,     ROT0, "IGS", "Manguan Caishen (China, V103CS)",                                   MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION ) // 满贯财神, finish IGS029 protection
GAME ( 1998,  mgcsa,    mgcs,     mgcsa,    mgcs,     igs017_state, init_mgcsa,    ROT0, "IGS", "Manguan Caishen (China, V106CS)",                                   MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION ) // 满贯财神, finish IGS029 protection
GAME ( 1998,  lhzb2,    0,        lhzb2,    lhzb2,    igs017_state, init_lhzb2,    ROT0, "IGS", "Long Hu Zhengba 2 (China, set 1)",                                  MACHINE_UNEMULATED_PROTECTION ) // 龙虎争霸2, finish IGS022 protection
GAME ( 1998,  lhzb2a,   lhzb2,    lhzb2a,   lhzb2a,   igs017_state, init_lhzb2a,   ROT0, "IGS", "Long Hu Zhengba 2 (China, VS221M)",                                 0 ) // 龙虎争霸2
GAME ( 1998,  slqz2,    0,        slqz2,    slqz2,    igs017_state, init_slqz2,    ROT0, "IGS", "Shuang Long Qiang Zhu 2 VS (China, VS203J)",                        MACHINE_UNEMULATED_PROTECTION ) // 双龙抢珠, finish IGS022 protection
GAME ( 1998,  slqz2a,   slqz2,    slqz2,    slqz2,    igs017_state, init_slqz2,    ROT0, "IGS", "Shuang Long Qiang Zhu 2 VS (China, set 2)",                         MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION ) // 双龙抢珠, misses program ROM dump, finish IGS022 protection
GAME ( 1999,  tarzanc,  0,        tarzan,   tarzan,   igs017_state, init_tarzanc,  ROT0, "IGS", "Tarzan Chuang Tian Guan (China, V109C, set 1)",                     0 ) // 泰山闯天关
GAME ( 1999,  tarzan,   tarzanc,  tarzan,   tarzan,   igs017_state, init_tarzan,   ROT0, "IGS", "Tarzan Chuang Tian Guan (China, V109C, set 2)",                     MACHINE_NOT_WORKING ) // missing sprites and sound rom, imperfect tiles decryption
GAME ( 1999,  tarzana,  tarzanc,  tarzan,   tarzan,   igs017_state, init_tarzana,  ROT0, "IGS", "Tarzan (V107)",                                                     MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION ) // missing IGS029 protection, missing sprites and sound rom
GAME ( 1999,  tarzanb,  tarzanc,  tarzan,   tarzan,   igs017_state, init_tarzanc,  ROT0, "IGS", "Tarzan Chuang Tian Guan (China, V110)",                             0 )
GAME ( 2000,  sdmg2p,   0,        sdmg2p,   sdmg2p,   igs017_state, init_sdmg2p,   ROT0, "IGS", "Maque Wangchao / Chaoji Damanguan 2 - Jiaqiang Ban (China, V100C)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // 麻雀王朝 / 超級大滿貫 2 -加強版 protection kicks in after starting game, hopper isn't hooked up correctly
GAMEL( 2000?, starzan,  0,        starzan,  starzan,  igs017_state, init_starzan,  ROT0, "IGS (G.F. Gioca license)", "Super Tarzan (Italy, V100I)",                  0, layout_igsslot  )
GAMEL( 2000?, happyskl, 0,        happyskl, happyskl, igs017_state, init_happyskl, ROT0, "IGS", "Happy Skill (Italy, V611IT)",                                       0, layout_igspoker )
GAMEL( 2000?, cpoker2,  0,        cpoker2,  cpoker2,  igs017_state, init_cpoker2,  ROT0, "IGS", "Champion Poker 2 (V100A)",                                          0, layout_igspoker )
GAME ( 2000?, spkrform, spk306us, spkrform, spkrform, igs017_state, init_spkrform, ROT0, "IGS", "Super Poker (V100xD03) / Formosa",                                  MACHINE_UNEMULATED_PROTECTION ) // poker game enabling forced with a patch. Parent spk306us in driver spoker.cpp
