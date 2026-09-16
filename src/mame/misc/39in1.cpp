// license:BSD-3-Clause
// copyright-holders:R. Belmont, Ryan Holtz, Andreas Naive
/**************************************************************************
 *
 * 39in1.cpp - bootleg MAME-based "39-in-1" arcade PCB
 * Driver by R. Belmont, thanks to the Guru
 * PXA255 Peripheral hookup by Ryan Holtz
 * Decrypt by Andreas Naive
 *
 * CPU: Intel Xscale PXA255 series @ 200 MHz, configured little-endian
 * Xscale PXA consists of:
 *    ARMv5TE instruction set without the FPU
 *    ARM standard MMU
 *    ARM DSP extensions
 *    VGA-ish frame buffer with some 2D acceleration features
 *    AC'97 stereo audio CODEC
 *
 * PCB also contains a custom ASIC, probably used for the decryption
 *
 * International Amusement Machine (I.A.M.) slots from the second half of the
 * 2000s use very similar PCBs (almost same main components, very similar layout,
 * same encryption).
 *
 * TODO:
 *   - IAM gambling machines need standard gambling inputs figured out and hooked up
 *   - rodent needs the ARM caches emulated.  We have a workaround in place.
 *   - 19in1 needs its unique flash ROM to be dumped.
 *   - 48in1, 48in1a and 48in1c also seem to need a unique flash ROM to be dumped.
 *   - An ARM DRC would be nice so these run at the correct speed.
 *
 **************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "machine/eepromser.h"
#include "machine/pxa255.h"

#define LOG_CPLD (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

// per-set CPLD parameters
struct cpld_prot
{
	u8 sig[8];         // read back from the data port, indexed by A3-A5
	u8 data_bits[8];   // bitswap<8> order applied to a value written to the data port
	u8 state_bits[8];  // bitswap<8> order applied to the current state
	u8 data_xor;       // inverts the result of the data bitswap
	u8 state_mask;     // which bits of the state a write replaces
	bool command;      // false on the earliest boards, which have no command port
};

// The later boards (19in1, 60in1) have a bigger CPLD.  Its state is one byte
// updated a nibble at a time, and each output bit is the XOR of two others, so
// the key is just which bits those are.  A second operand is either a bit of the
// state itself or a bit of the last value the data port handed back.
constexpr u8 prot_state_bit(u8 n) { return n; }
constexpr u8 prot_value_bit(u8 n) { return 8 + n; }

struct cpld2_key
{
	u8 sig[16];         // read back from 04af0000, indexed by A3-A6
	u8 resp[8][2];      // response bit n = state[resp[n][0]] ^ state[resp[n][1]]
	u8 update[8][2];    // state bit n = latch[update[n][0]] ^ update[n][1]
	u8 latch[8];        // bitswap<8> order of the address lines the latch is loaded from
	bool mult;          // the response is scaled by operands written to 04ad/04ae first
};

struct cpld_key
{
	u8 bits[8];                             // bitswap<8> source bit order, MSB first
	u8 xor_always;                          // XOR applied to every even byte
	struct { u8 line; u8 data; } xor_addr[10]; // XOR applied when address line is high, terminated by line 0
};


class _39in1_state : public driver_device
{
public:
	_39in1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pxa_periphs(*this, "pxa_periphs")
		, m_eeprom(*this, "eeprom")
		, m_ram(*this, "ram")
		, m_mcu_ipt(*this, "MCUIPT")
		, m_dsw(*this, "DSW")
	{ }

	void cpld(machine_config &config) ATTR_COLD;
	void cpld2(machine_config &config) ATTR_COLD;
	void iam2(machine_config &config) ATTR_COLD;
	void base(machine_config &config) ATTR_COLD;
	void iam(machine_config &config) ATTR_COLD;

	void init_4in1a() ATTR_COLD;
	void init_4in1b() ATTR_COLD;
	void init_19in1() ATTR_COLD;
	void init_39in1() ATTR_COLD;
	void init_48in1() ATTR_COLD;
	void init_48in1a() ATTR_COLD;
	void init_48in1c() ATTR_COLD;
	void init_60in1() ATTR_COLD;
	void init_rodent() ATTR_COLD;

	// I.A.M. slots
	void init_fruitwld();
	void init_jumanji();
	void init_jumanjia();
	void init_plutus();
	void init_pokrwild();

	DECLARE_INPUT_CHANGED_MEMBER(set_flip_dip);
	DECLARE_INPUT_CHANGED_MEMBER(set_res_dip);
	DECLARE_INPUT_CHANGED_MEMBER(set_hiscore_dip);
	DECLARE_INPUT_CHANGED_MEMBER(set_test_dip);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// CPLD protection
	const cpld_prot *m_prot = nullptr;
	const cpld2_key *m_prot2 = nullptr;
	u8 m_prot_state;
	u8 m_prot_mode;
	u8 m_prot_count;
	u8 m_prot2_latch;
	u8 m_prot2_value;
	u8 m_prot2_armed;
	u8 m_prot2_resp;
	u8 m_prot2_op[3];

	required_device<cpu_device> m_maincpu;
	required_device<pxa255_periphs_device> m_pxa_periphs;
	required_device<eeprom_serial_93c66_16bit_device> m_eeprom;
	required_shared_ptr<u32> m_ram;
	required_ioport m_mcu_ipt;
	required_ioport m_dsw;

	u32 cpld_r(offs_t offset, u32 mem_mask = ~0);
	void cpld_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cpld2_r(offs_t offset, u32 mem_mask = ~0);
	void cpld2_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void set_protection(const cpld_prot &prot) ATTR_COLD;
	void set_protection(const cpld2_key &prot) ATTR_COLD;

	void cpld_map(address_map &map) ATTR_COLD;
	void cpld2_map(address_map &map) ATTR_COLD;
	void base_map(address_map &map) ATTR_COLD;
	void iam_map(address_map &map) ATTR_COLD;
	void iam2_map(address_map &map) ATTR_COLD;

	void decrypt(const cpld_key (&keys)[4], u8 sel_hi, u8 sel_lo) ATTR_COLD;
	void decrypt(const cpld_key &key) ATTR_COLD;
	void decrypt(const cpld_key (&keys)[2], u8 sel) ATTR_COLD;
};

void _39in1_state::machine_start()
{
	save_item(NAME(m_prot_state));
	save_item(NAME(m_prot_mode));
	save_item(NAME(m_prot_count));
	save_item(NAME(m_prot2_latch));
	save_item(NAME(m_prot2_value));
	save_item(NAME(m_prot2_armed));
	save_item(NAME(m_prot2_resp));
	save_item(NAME(m_prot2_op));
}

void _39in1_state::machine_reset()
{
	m_prot_state = 0;
	m_prot_mode = 0;
	m_prot_count = 0;
	m_prot2_latch = 0;
	m_prot2_value = 0;
	m_prot2_armed = 0;
	m_prot2_resp = 0;
	std::fill(std::begin(m_prot2_op), std::end(m_prot2_op), 0);

	m_pxa_periphs->gpio_in<1>(1);

	const u32 dsw = m_dsw->read();
	m_pxa_periphs->gpio_in<53>(BIT(dsw, 0));
	m_pxa_periphs->gpio_in<54>(BIT(dsw, 1));
	m_pxa_periphs->gpio_in<56>(BIT(dsw, 2));
	m_pxa_periphs->gpio_in<57>(BIT(dsw, 3));

	m_eeprom->di_write(ASSERT_LINE);
}

/*
    The CPLD decodes very few address lines:

      A21 A20 A6
       0   0   -   0x20 reads the player inputs, 0x28 is an output latch
       0   1   0   an output latch the game keeps a RAM copy of
       0   1   1   protection data port
       1   -   -   protection command port

    The I.A.M. slots are the same as 4in1a/4in1b, at 04100040-0410007e:
    reads return the challenge state in the high nibble and an 8-entry signature
    indexed by A3-A5 in the low nibble, while writes clock the state.  They check
    the signature eight times and then run four challenge rounds, and a failure
    of either shows 'HW_002'/'HW_003 ERROR'.

    Everything else - A2-A5, A7-A19 - is ignored, and the game randomises those
    bits on every single access to confuse would-be crackers.

    The command port latches two bits of the value written:

      bit 0  what the data port reads back: 0 selects an eight byte signature
             indexed by A3-A5, 1 selects the challenge/response register
      bit 6  what a write to the data port does to that register

    4in1a and 4in1b use an earlier part with no command port at all.  There the
    register is only the top nibble of the data port and the signature is always
    in the bottom nibble, so the games can read both at once and no mode bit is
    needed.

    With bit 6 clear a write mixes the data in:

        state = (perm_d(data) ^ data_xor) ^ perm_s(state)

    where the two bit permutations and which bits get inverted are part of the
    board's key.  With bit 6 set the data is thrown away and the register simply
    counts up, which the game uses to stir it between real challenges; it always
    does exactly one data write per bit 6 command, so whether the count happens
    on the command or on the write cannot be told apart from software.  Reads
    never change it.
*/

void _39in1_state::set_protection(const cpld_prot &prot)
{
	m_prot = &prot;
}

void _39in1_state::set_protection(const cpld2_key &prot)
{
	m_prot2 = &prot;
}

/*
    The CPLD on 19in1 and 60in1 is different, but the idea is similar.

      04a8xxxx  read: update state bits 0-3
      04a9xxxx  read: update state bits 4-7
      04aaxxxx  read: selects what 04af reads back - the signature when A13 is
                set, the response when it is clear
      04abxxxx  read: clear the state
      04acxxxx  read: clear the latch          write: arm the response
      04adxxxx  read: count the state up by one
      04aexxxx  read: load the latch from address lines A4-A6 and A11-A15
      04afxxxx  read: the response latched by the last 04ac write, or the
                signature picked by A3-A6 - and whatever that hands back is
                folded into the next state update, by both sides, so it does
                not matter what it is

    A state update replaces one nibble, each bit being a latch bit XORed with
    either a state bit or a bit of that last value.  The response is built the
    same way out of pairs of state bits.  Both sides start from zero.
*/

u32 _39in1_state::cpld2_r(offs_t offset, u32 mem_mask)
{
	const offs_t addr = offset << 2;

	if ((addr & 0xf00000) == 0x000000)
	{
		return cpld_r(offset, mem_mask);
	}

	if (!m_prot2)
	{
		return 0;
	}

	const auto update = [this] (unsigned first, unsigned last)
			{
				for (unsigned n = first; n <= last; n++)
				{
					const u8 other = m_prot2->update[n][1];
					const u8 second = (other < 8) ? BIT(m_prot_state, other) : BIT(m_prot2_value, other - 8);
					m_prot_state = (m_prot_state & ~(1 << n)) | ((BIT(m_prot2_latch, m_prot2->update[n][0]) ^ second) << n);
				}
			};

	switch ((addr >> 16) & 0xff)
	{
	case 0xa8:
		update(0, 3);
		break;

	case 0xa9:
		update(4, 7);
		break;

	case 0xaa:
		m_prot2_armed = BIT(addr, 13) ? 0 : 1;
		break;

	case 0xab:
		m_prot_state = 0;
		break;

	case 0xac:
		m_prot2_latch = 0;
		break;

	case 0xad:
		m_prot_state++;
		break;

	case 0xae:
		{
			u8 const *const l = m_prot2->latch;
			m_prot2_latch = bitswap<8>(addr, l[0], l[1], l[2], l[3], l[4], l[5], l[6], l[7]);
		}
		break;

	case 0xaf:
		if (m_prot2_armed)
		{
			if (m_prot2->mult)
			{
				u16 k = m_prot2_op[0] * m_prot2_op[1];
				k = (k + (k >> 8)) & 0xff;
				k = m_prot2_op[2] * k;
				k = (k + (k >> 8)) & 0xff;
				const u16 data = m_prot2_resp * k;
				m_prot2_value = data & 0xff;
				return data * 0x00010001;
			}
			m_prot2_value = m_prot2_resp;
			return m_prot2_resp * 0x01010101;
		}
		m_prot2_value = m_prot2->sig[(addr >> 3) & 15];
		return m_prot2_value * 0x01010101;

	default:
		break;
	}

	return 0;
}

void _39in1_state::cpld2_w(offs_t offset, u32 data, u32 mem_mask)
{
	const offs_t addr = offset << 2;

	if ((addr & 0xf00000) == 0x000000)
	{
		cpld_w(offset, data, mem_mask);
		return;
	}

	if (!m_prot2)
	{
		return;
	}

	const u16 value = ACCESSING_BITS_0_15 ? u16(data) : u16(data >> 16);

	switch ((addr >> 16) & 0xff)
	{
	case 0xac:
		// the response is latched here, so counting the state up before
		// reading it (which plutus does, repeatedly) leaves it alone
		m_prot2_resp = 0;
		for (unsigned n = 0; n < 8; n++)
		{
			m_prot2_resp |= (BIT(m_prot_state, m_prot2->resp[n][0]) ^ BIT(m_prot_state, m_prot2->resp[n][1])) << n;
		}
		break;

	case 0xad:
		m_prot2_op[0] = value & 0xff;
		m_prot2_op[1] = value >> 8;
		break;

	case 0xae:
		m_prot2_op[2] = value & 0xff;
		break;

	default:
		break;
	}
}

u32 _39in1_state::cpld_r(offs_t offset, u32 mem_mask)
{
	const offs_t addr = offset << 2;

	switch (addr & 0x300000)
	{
	case 0x000000:
		if ((addr & 0xfc) == 0x20)
		{
			return m_mcu_ipt->read();
		}
		break;

	case 0x100000:
		if (BIT(addr, 6) && m_prot)
		{
			const u8 sig = m_prot->sig[(addr >> 3) & 7];
			const u8 data = m_prot->command
					? (m_prot_mode ? m_prot_state : sig)
					: ((m_prot_state & m_prot->state_mask) | (sig & ~m_prot->state_mask));
			return data * 0x01010101;
		}
		return 0; // the other latch, write only as far as the games are concerned

	default:
		break;
	}

	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_CPLD, "%s: unhandled CPLD read %06x & %08x\n", machine().describe_context(), addr, mem_mask);
	}
	return 0;
}

void _39in1_state::cpld_w(offs_t offset, u32 data, u32 mem_mask)
{
	const offs_t addr = offset << 2;
	const u8 value = ACCESSING_BITS_0_7 ? u8(data) : u8(data >> 16);

	switch (addr & 0x300000)
	{
	case 0x000000:
		if ((addr & 0xfc) == 0x28)
		{
			return; // output latch, nothing hooked up to it yet
		}
		break;

	case 0x100000:
		if (BIT(addr, 6) && m_prot)
		{
			if (m_prot_count)
			{
				m_prot_state++;
			}
			else
			{
				u8 const *const d = m_prot->data_bits;
				u8 const *const t = m_prot->state_bits;
				const u8 mixed = (bitswap<8>(value, d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7]) ^ m_prot->data_xor)
						^ bitswap<8>(m_prot_state, t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7]);
				m_prot_state = (m_prot_state & ~m_prot->state_mask) | (mixed & m_prot->state_mask);
			}
		}
		return;

	case 0x200000:
		m_prot_mode = BIT(value, 0);
		m_prot_count = BIT(value, 6);
		return;

	default:
		break;
	}

	LOGMASKED(LOG_CPLD, "%s: unhandled CPLD write %06x = %08x & %08x\n", machine().describe_context(), addr, data, mem_mask);
}

void _39in1_state::base_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom();
	map(0x00400000, 0x007fffff).rom().region("data", 0);
	map(0x40000000, 0x47ffffff).m(m_pxa_periphs, FUNC(pxa255_periphs_device::map));
	map(0xa0000000, 0xa07fffff).ram().share("ram");
}

void _39in1_state::cpld_map(address_map &map)
{
	base_map(map);

	map(0x04000000, 0x047fffff).rw(FUNC(_39in1_state::cpld_r), FUNC(_39in1_state::cpld_w));
}

void _39in1_state::cpld2_map(address_map &map)
{
	base_map(map);

	map(0x04000000, 0x04ffffff).rw(FUNC(_39in1_state::cpld2_r), FUNC(_39in1_state::cpld2_w));
}

void _39in1_state::iam_map(address_map &map)
{
	cpld_map(map);

	map(0x04800000, 0x04ffffff).ram();
	map(0xa0800000, 0xa3ffffff).ram();
}

void _39in1_state::iam2_map(address_map &map)
{
	cpld2_map(map);

	map(0x04800000, 0x04a7ffff).ram();
	map(0x04b00000, 0x04ffffff).ram();
	map(0xa0800000, 0xa3ffffff).ram();
}


static INPUT_PORTS_START( 39in1 )
	PORT_START("MCUIPT")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x80000000, IP_ACTIVE_LOW )

	PORT_START("DSW")      // 1x 4-position DIP switch labelled SW3
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )    PORT_DIPLOCATION("SW3:1") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(_39in1_state::set_flip_dip), 0)
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Display Mode" )            PORT_DIPLOCATION("SW3:2") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(_39in1_state::set_res_dip), 0)
	PORT_DIPSETTING(    0x02, "CGA 15.75kHz" )
	PORT_DIPSETTING(    0x00, "VGA 31.5kHz" )
	PORT_DIPNAME( 0x04, 0x04, "High Score Saver" )        PORT_DIPLOCATION("SW3:3") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(_39in1_state::set_hiscore_dip), 0)
	PORT_DIPSETTING(    0x04, "Disabled" )
	PORT_DIPSETTING(    0x00, "Enabled" )
	PORT_DIPNAME( 0x08, 0x08, "Test Mode" )               PORT_DIPLOCATION("SW3:4") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(_39in1_state::set_test_dip), 0)
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(_39in1_state::set_flip_dip)
{
	m_pxa_periphs->gpio_in<53>(BIT(m_dsw->read(), 0));
}

INPUT_CHANGED_MEMBER(_39in1_state::set_res_dip)
{
	m_pxa_periphs->gpio_in<54>(BIT(m_dsw->read(), 1));
}

INPUT_CHANGED_MEMBER(_39in1_state::set_hiscore_dip)
{
	m_pxa_periphs->gpio_in<56>(BIT(m_dsw->read(), 2));
}

INPUT_CHANGED_MEMBER(_39in1_state::set_test_dip)
{
	m_pxa_periphs->gpio_in<57>(BIT(m_dsw->read(), 3));
}

void _39in1_state::decrypt(const cpld_key (&keys)[4], u8 sel_hi, u8 sel_lo)
{
	u8 *const rom = memregion("maincpu")->base();

	for (offs_t offset = 0; offset < 0x80000; offset += 2)
	{
		const cpld_key &key = keys[(BIT(offset, sel_hi) << 1) | BIT(offset, sel_lo)];

		u8 data = bitswap<8>(rom[offset], key.bits[0], key.bits[1], key.bits[2], key.bits[3],
				key.bits[4], key.bits[5], key.bits[6], key.bits[7]) ^ key.xor_always;

		for (auto const &line : key.xor_addr)
		{
			if (!line.line)
				break;
			if (BIT(offset, line.line))
				data ^= line.data;
		}

		rom[offset] = data;
	}
}

void _39in1_state::decrypt(const cpld_key &key)
{
	const cpld_key keys[4] = { key, key, key, key };
	decrypt(keys, 17, 15);
}

// Two configurations selected by a single address line.  Only even bytes are
// scrambled, so A0 can stand in for the unused second selector.
void _39in1_state::decrypt(const cpld_key (&keys)[2], u8 sel)
{
	const cpld_key four[4] = { keys[0], keys[0], keys[1], keys[1] };
	decrypt(four, sel, 0);
}

static const cpld_prot PROT_39IN1 = {
	{ 0x55, 0x93, 0x89, 0xa2, 0x31, 0x75, 0x97, 0xb1 },
	{ 5, 1, 2, 7, 6, 4, 3, 0 }, { 0, 7, 2, 4, 6, 3, 5, 1 }, 0xff, 0xff, true };

// the earlier part only keeps the top nibble and has no command port
static const cpld_prot PROT_4IN1A = {
	{ 0x03, 0x00, 0x0f, 0x04, 0x07, 0x0b, 0x06, 0x05 },
	{ 4, 2, 0, 7, 1, 3, 5, 6 }, { 4, 6, 5, 7, 0, 1, 2, 3 }, 0xd0, 0xf0, false };

static const cpld_prot PROT_4IN1B = {
	{ 0x0e, 0x04, 0x00, 0x02, 0x01, 0x00, 0x0c, 0x01 },
	{ 5, 2, 4, 7, 0, 1, 3, 6 }, { 6, 7, 4, 5, 0, 1, 2, 3 }, 0x20, 0xf0, false };

// the I.A.M. slots use the same part again, at 04100040-0410007e: the low nibble
// is the signature indexed by A3-A5 and the high nibble is the challenge state
static const cpld_prot PROT_FRUITWLD = {
	{ 0x08, 0x09, 0x02, 0x03, 0x0f, 0x0e, 0x05, 0x04 },
	{ 6, 4, 3, 7, 0, 1, 2, 5 }, { 7, 5, 6, 4, 0, 1, 2, 3 }, 0x80, 0xf0, false };

static const cpld_prot PROT_JUMANJIA = {
	{ 0x0c, 0x0f, 0x01, 0x02, 0x0a, 0x09, 0x07, 0x04 },
	{ 4, 5, 6, 7, 0, 1, 2, 3 }, { 7, 5, 6, 4, 0, 1, 2, 3 }, 0x80, 0xf0, false };

static const cpld_prot PROT_POKRWILD = {
	{ 0x02, 0x0e, 0x01, 0x0d, 0x06, 0x0a, 0x05, 0x09 },
	{ 4, 2, 0, 3, 1, 5, 6, 7 }, { 7, 5, 6, 4, 0, 1, 2, 3 }, 0x00, 0xf0, false };

void _39in1_state::init_39in1()
{
	static const cpld_key key = { { 7, 2, 5, 6, 0, 3, 1, 4 }, 0x90, { { 3, 0x02 }, { 4, 0x10 }, { 5, 0x40 }, { 6, 0x80 } } };
	decrypt(key);

	set_protection(PROT_39IN1);
}

static const cpld2_key PROT_48IN1 = {
	{ 0x43, 0xaa, 0x5f, 0xc9, 0xe4, 0xfe, 0xdb, 0x60, 0x3d, 0x15, 0x91, 0x28, 0x06, 0x8c, 0xb7, 0x72 },
	{ { 0, 7 }, { 1, 4 }, { 2, 0 }, { 3, 1 }, { 7, 2 }, { 6, 3 }, { 5, 6 }, { 4, 5 } },
	{ { 3, prot_state_bit(4) }, { 7, prot_value_bit(6) }, { 1, prot_state_bit(7) }, { 6, prot_value_bit(3) },
	  { 0, prot_state_bit(2) }, { 2, prot_value_bit(1) }, { 5, prot_state_bit(0) }, { 4, prot_value_bit(5) } },
	{ 15, 14, 13, 12, 11, 6, 5, 4 }, false };

static const cpld2_key PROT_19IN1 = {
	{ 0x94, 0x81, 0x3a, 0x72, 0x47, 0x29, 0x83, 0xbe, 0xff, 0xad, 0x50, 0xe6, 0xfc, 0x0b, 0x78, 0x65 },
	{ { 5, 2 }, { 6, 1 }, { 7, 6 }, { 0, 5 }, { 1, 0 }, { 2, 7 }, { 3, 4 }, { 3, 4 } },
	{ { 5, prot_state_bit(4) }, { 1, prot_value_bit(6) }, { 6, prot_state_bit(7) }, { 4, prot_value_bit(1) },
	  { 0, prot_value_bit(3) }, { 3, prot_value_bit(2) }, { 7, prot_state_bit(0) }, { 2, prot_value_bit(5) } },
	{ 15, 14, 13, 12, 11, 6, 5, 4 }, false };

static const cpld2_key PROT_60IN1 = {
	{ 0x03, 0x69, 0xce, 0x76, 0xdc, 0x12, 0x8f, 0xa4, 0xf8, 0x97, 0x21, 0xea, 0x5d, 0xb0, 0x3b, 0x45 },
	{ { 2, 7 }, { 6, 1 }, { 5, 6 }, { 4, 5 }, { 3, 0 }, { 2, 7 }, { 1, 3 }, { 0, 4 } },
	{ { 5, prot_state_bit(4) }, { 1, prot_value_bit(6) }, { 6, prot_state_bit(7) }, { 4, prot_state_bit(3) },
	  { 0, prot_state_bit(2) }, { 3, prot_value_bit(1) }, { 7, prot_state_bit(0) }, { 2, prot_value_bit(5) } },
	{ 15, 14, 13, 12, 11, 6, 5, 4 }, false };

// plutus's FPGA speaks the same protocol, with the latch fed from A1-A7 and
// A11 instead, and its response scaled by operands the game writes first.
// Everything here is read out of the game's own copy of the calculation at
// image 42b8 (the boot check) and 472c (the periodic one).
static const cpld2_key PROT_PLUTUS = {
	{ 0x47, 0x1c, 0xc5, 0x59, 0x9f, 0xe2, 0x7e, 0xa0, 0xba, 0xf8, 0x23, 0xd4, 0x3b, 0x66, 0x81, 0x0d },
	{ { 4, 2 }, { 1, 6 }, { 7, 3 }, { 5, 7 }, { 1, 6 }, { 3, 0 }, { 2, 5 }, { 0, 4 } },
	{ { 6, prot_state_bit(1) }, { 5, prot_state_bit(6) }, { 3, prot_state_bit(7) }, { 2, prot_value_bit(3) },
	  { 7, prot_value_bit(0) }, { 1, prot_value_bit(5) }, { 4, prot_value_bit(4) }, { 0, prot_state_bit(2) } },
	{ 11, 7, 6, 5, 4, 3, 2, 1 }, true };

void _39in1_state::init_4in1a()
{
	static const cpld_key key = { { 6, 0, 2, 1, 7, 5, 4, 3 }, 0x64, { { 3, 0x40 }, { 4, 0x08 }, { 5, 0x20 }, { 6, 0x80 } } };
	decrypt(key);
	set_protection(PROT_4IN1A);
}

// rodent shares this key: the whole 0a00-0ffff gap in its ROM descrambles to ff
static const cpld_key KEY_4IN1B = { { 2, 4, 0, 6, 7, 3, 1, 5 }, 0x32, { { 3, 0x08 }, { 4, 0x80 }, { 5, 0x10 }, { 6, 0x04 } } };

void _39in1_state::init_4in1b()
{
	decrypt(KEY_4IN1B);
	set_protection(PROT_4IN1B);
}

void _39in1_state::init_19in1()
{
	static const cpld_key keys[4] = {
		{ { 2, 1, 7, 4, 5, 0, 6, 3 }, 0x00, { { 3, 0x80 }, { 4, 0x04 }, { 5, 0x20 }, { 6, 0x02 }, { 11, 0x01 }, { 13, 0x10 }, { 14, 0x40 }, { 16, 0x08 } } },
		{ { 5, 3, 6, 4, 2, 7, 0, 1 }, 0x00, { { 3, 0x80 }, { 4, 0x04 }, { 6, 0x08 }, { 11, 0x10 }, { 12, 0x40 }, { 13, 0x20 }, { 16, 0x02 }, { 18, 0x01 } } },
		{ { 0, 6, 5, 4, 2, 3, 1, 7 }, 0x00, { { 3, 0x40 }, { 4, 0x20 }, { 5, 0x10 }, { 6, 0x02 }, { 11, 0x80 }, { 12, 0x01 }, { 13, 0x08 }, { 14, 0x04 } } },
		{ { 5, 1, 4, 2, 0, 7, 6, 3 }, 0x00, { { 4, 0x10 }, { 5, 0x02 }, { 6, 0x80 }, { 11, 0x40 }, { 12, 0x08 }, { 13, 0x01 }, { 14, 0x04 }, { 16, 0x20 } } } };
	decrypt(keys, 17, 15);
	set_protection(PROT_19IN1);
}

// the 48-in-1s pick their configuration with A16 and A17 instead of A15 and A17,
// which leaves A15 as an ordinary key line.  Applies to 48in1b as well, same ROM.
void _39in1_state::init_48in1()
{
	static const cpld_key keys[4] = {
		{ { 5, 3, 2, 1, 4, 6, 0, 7 }, 0x00, { { 3, 0x02 }, { 4, 0x04 }, { 6, 0x80 }, { 11, 0x01 }, { 12, 0x20 }, { 13, 0x10 }, { 14, 0x40 }, { 15, 0x08 } } },
		{ { 6, 3, 4, 2, 0, 7, 5, 1 }, 0x00, { { 3, 0x40 }, { 5, 0x10 }, { 6, 0x02 }, { 12, 0x01 }, { 13, 0x08 }, { 14, 0x04 }, { 15, 0x20 }, { 18, 0x80 } } },
		{ { 0, 6, 7, 5, 3, 2, 1, 4 }, 0x00, { { 3, 0x80 }, { 4, 0x10 }, { 5, 0x02 }, { 11, 0x40 }, { 13, 0x01 }, { 14, 0x04 }, { 15, 0x20 }, { 18, 0x08 } } },
		{ { 2, 0, 7, 4, 6, 3, 1, 5 }, 0x00, { { 3, 0x80 }, { 5, 0x02 }, { 6, 0x08 }, { 11, 0x10 }, { 13, 0x20 }, { 14, 0x04 }, { 15, 0x40 }, { 18, 0x01 } } } };
	decrypt(keys, 17, 16);
	set_protection(PROT_48IN1);
}

void _39in1_state::init_48in1a() { init_48in1(); } // same encryption as 48in1

void _39in1_state::init_48in1c() { init_48in1(); } // same encryption as 48in1

// rodent is the odd one out: no xZIP container, its program sits uncompressed at
// 010000 and the little loader copies it up to fff10000 and jumps there.  Its
// protection, if it has any, has not been looked at.
//
// It needs the ARM caches, which this ARM7 core does not emulate.  The loader
// maps low RAM twice: virtual 00000000 (where it puts the exception vectors and
// their literal pool) and virtual a0000000 both cover physical a0000000, and
// everything else - including the game itself at fff00000 - is mapped uncached.
// Early on the game clears 34K at a0000000 for a work buffer, which on real
// hardware leaves the vectors intact because the XScale caches are virtually
// tagged, so the lines tagged 00000000 are a different set of lines from the
// ones tagged a0000000, and nothing else running is cacheable enough to evict
// them.  Without a cache the two aliases collapse onto the same memory, the
// vector table is destroyed, and the next FIQ (the LCD end-of-frame) fetches
// garbage and ends up parked on the undefined instruction vector.
//
// HACK: until the core can model that, skip the store that does the damage.
// The fill loop is at fff10480, which is physical a0710480 because L1[fff] maps
// fff00000 to a0700000, so a071048c is its "STRT R3, [R1]".  Opcode fetches go
// through the address space, so a read tap handing back MOV R0, R0 takes out
// that one instruction without touching anything else.
//
// What this gets wrong: on hardware the fill does happen, it is just invisible
// through the cached alias, so anything that reads the work buffer back should
// see 0000ffff and here will see the bootloader's RAM test residue instead.
// The address is also a bare constant with nothing to anchor it - it falls out
// of the page tables the loader happens to build.
void _39in1_state::init_rodent()
{
	decrypt(KEY_4IN1B);

	m_maincpu->space(AS_PROGRAM).install_read_tap(0xa071048c, 0xa071048f, "rodent_vector_wipe",
			[] (offs_t offset, u32 &data, u32 mem_mask) { data = 0xe1a00000; });
}

void _39in1_state::init_60in1()
{
	static const cpld_key keys[4] = {
		{ { 5, 1, 4, 2, 0, 7, 6, 3 }, 0x00, { { 3, 0x02 }, { 4, 0x20 }, { 5, 0x04 }, { 6, 0x80 }, { 11, 0x01 }, { 13, 0x10 }, { 14, 0x40 }, { 16, 0x08 } } },
		{ { 2, 1, 7, 4, 5, 0, 6, 3 }, 0x00, { { 3, 0x40 }, { 4, 0x20 }, { 5, 0x10 }, { 6, 0x02 }, { 11, 0x80 }, { 12, 0x01 }, { 13, 0x08 }, { 14, 0x04 } } },
		{ { 0, 6, 4, 5, 2, 3, 1, 7 }, 0x00, { { 4, 0x10 }, { 5, 0x02 }, { 6, 0x80 }, { 11, 0x40 }, { 12, 0x08 }, { 13, 0x01 }, { 14, 0x04 }, { 16, 0x20 } } },
		{ { 5, 3, 6, 4, 2, 1, 0, 7 }, 0x00, { { 3, 0x80 }, { 4, 0x04 }, { 6, 0x08 }, { 11, 0x10 }, { 12, 0x40 }, { 13, 0x20 }, { 16, 0x02 }, { 18, 0x01 } } } };
	decrypt(keys, 17, 15);
	set_protection(PROT_60IN1);
}

// I.A.M. slots
void _39in1_state::init_fruitwld()
{
	static const cpld_key key = { { 5, 1, 7, 4, 3, 2, 0, 6 }, 0x48, { { 3, 0x80 }, { 4, 0x20 } } };
	decrypt(key);

	set_protection(PROT_FRUITWLD);
}

void _39in1_state::init_jumanji()
{
	// Two configurations, selected by A17.  The program inflates to exactly its
	// declared size and passes the loader's checksum with these.
	static const cpld_key keys[2] = {
		{ { 1, 0, 6, 2, 5, 3, 4, 7 }, 0x00, { { 3, 0x80 }, { 5, 0x20 }, { 6, 0x04 }, { 11, 0x08 }, { 12, 0x10 }, { 13, 0x40 }, { 14, 0x01 }, { 15, 0x02 } } },
		{ { 4, 6, 1, 5, 2, 3, 0, 7 }, 0x00, { { 4, 0x08 }, { 5, 0x01 }, { 6, 0x80 }, { 11, 0x20 }, { 12, 0x10 }, { 13, 0x02 }, { 14, 0x40 } } } };
	decrypt(keys, 17);
}

void _39in1_state::init_jumanjia()
{
	static const cpld_key key = { { 3, 5, 1, 4, 7, 6, 2, 0 }, 0x00, { { 3, 0x04 }, { 4, 0x10 }, { 5, 0x80 }, { 6, 0x02 } } };
	decrypt(key);

	set_protection(PROT_JUMANJIA);
}

void _39in1_state::init_plutus()
{
	// Four configurations, selected by A16 and A14.  Each maps eight address
	// lines onto the eight data bits.  The program inflates to exactly its
	// declared size and passes the loader's checksum with these.
	static const cpld_key keys[4] = {
		{ { 6, 4, 0, 5, 7, 3, 2, 1 }, 0x00, { { 1, 0x80 }, { 2, 0x04 }, { 3, 0x20 }, { 5, 0x02 }, { 6, 0x08 }, { 7, 0x01 }, { 12, 0x10 }, { 15, 0x40 } } },
		{ { 5, 2, 6, 4, 0, 3, 7, 1 }, 0x00, { { 1, 0x80 }, { 2, 0x04 }, { 3, 0x02 }, { 4, 0x20 }, { 5, 0x10 }, { 6, 0x40 }, { 11, 0x01 }, { 13, 0x08 } } },
		{ { 1, 5, 4, 0, 6, 3, 2, 7 }, 0x00, { { 1, 0x20 }, { 2, 0x02 }, { 3, 0x80 }, { 4, 0x08 }, { 5, 0x10 }, { 13, 0x04 }, { 15, 0x40 } } },
		{ { 6, 4, 5, 2, 1, 0, 7, 3 }, 0x00, { { 1, 0x01 }, { 2, 0x20 }, { 4, 0x02 }, { 6, 0x40 }, { 7, 0x10 }, { 12, 0x08 }, { 13, 0x80 } } } };
	decrypt(keys, 16, 14);

	set_protection(PROT_PLUTUS);
}

void _39in1_state::init_pokrwild()
{
	static const cpld_key key = { { 6, 5, 3, 1, 0, 7, 2, 4 }, 0x40, { { 3, 0x80 }, { 4, 0x20 } } };
	decrypt(key);

	set_protection(PROT_POKRWILD);
}

void _39in1_state::base(machine_config &config)
{
	PXA255(config, m_maincpu, 200'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &_39in1_state::base_map);

	EEPROM_93C66_16BIT(config, m_eeprom);
	m_eeprom->do_callback().set(m_pxa_periphs, FUNC(pxa255_periphs_device::gpio_in<5>));

	PXA255_PERIPHERALS(config, m_pxa_periphs, 200'000'000, m_maincpu);
	m_pxa_periphs->gpio_out<4>().set(m_eeprom, FUNC(eeprom_serial_93c66_16bit_device::di_write));
	m_pxa_periphs->gpio_out<2>().set(m_eeprom, FUNC(eeprom_serial_93c66_16bit_device::cs_write));
	m_pxa_periphs->gpio_out<3>().set(m_eeprom, FUNC(eeprom_serial_93c66_16bit_device::clk_write));
}

void _39in1_state::cpld(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &_39in1_state::cpld_map);
}

void _39in1_state::cpld2(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &_39in1_state::cpld2_map);
}

void _39in1_state::iam(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &_39in1_state::iam_map);
}

void _39in1_state::iam2(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &_39in1_state::iam2_map);
}


ROM_START( 39in1 )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "27c4096_plz-v001_ver.300.bin", 0x000000, 0x080000, CRC(9149dbc4) SHA1(40efe1f654f11474f75ae7fee1613f435dbede38) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )  // CGC-NP203 string

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c66_eeprom.bin", 0x000, 0x200, CRC(a423a969) SHA1(4c68654c81e70367209b9f6c712564aae89a3122) )
ROM_END

ROM_START( 48in1 )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hph_ver309",   0x000000, 0x080000, CRC(27023186) SHA1(a2b3770c4b03d6026c6a0ff2e62ab17c3b359b12) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )  // CGC-NP203 string

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "48in1_93c66_eeprom.bin", 0x000, 0x200, NO_DUMP )
ROM_END


ROM_START( 48in1b )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hph_ver309",   0x000000, 0x080000, CRC(27023186) SHA1(a2b3770c4b03d6026c6a0ff2e62ab17c3b359b12) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "48_flash.u19", 0x000000, 0x400000, CRC(a975db44) SHA1(5be6520b2ba7728e9e2de3c62ae7c3b88b25172a) )  // CGC-NP205 string

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "48_93c66.u32", 0x000, 0x200, CRC(cec06912) SHA1(2bc2e45602c5b1e8a3e031dd384e9f16be4e2ddb) )
ROM_END


ROM_START( 48in1a )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ver302.u2",    0x000000, 0x080000, CRC(5ea25870) SHA1(66edc59a3d355bc3462e98d2062ada721c371af6) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )  // CGC-NP203 string

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "48in1_93c66_eeprom.bin", 0x000, 0x200, NO_DUMP )
ROM_END


ROM_START( 48in1c )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "48in1_hph_ver308.u2", 0x000000, 0x080000, CRC(5d42beb0) SHA1(f21d1923b588cca1a6cd48a8ea6f3b5b996ebc1a) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )  // CGC-NP203 string

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "48in1_93c66_eeprom.bin", 0x000, 0x200, NO_DUMP )
ROM_END


ROM_START( 60in1 )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hph_ver300.u8",   0x000000, 0x080000, CRC(6fba84c4) SHA1(28881e51227e94a80c8449d9c00a1a675f008d64) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "flash.u19", 0x000000, 0x400000, CRC(0cfed2a0) SHA1(9aac23f5267af56255e6f8aefade9f00bc106325) )  // CGC-NP206 string

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "60in1_eeprom.u32", 0x000, 0x200, CRC(54af5973) SHA1(30aca7790458f4be906f7fa7c74206e16d9fc36f) )
ROM_END

ROM_START( 4in1a )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "plz-v014_ver300.bin", 0x000000, 0x080000, CRC(775f101d) SHA1(8a299a67b487518ba2e2cb5334347b93f8640190) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) ) // confirmed same flash rom as 39 in 1,   CGC-NP203 string

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "4in1_eeprom.bin", 0x000, 0x200, CRC(df1724f7) SHA1(07814aee3622f4bb8bada938f2a93fae791d6e31) )
ROM_END

ROM_START( 4in1b )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pzv001-4.bin", 0x000000, 0x080000, CRC(7679a95f) SHA1(56c20fa7d086560b76477b42208cb43d42adba41) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )  // CGC-NP203 string

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c66-4.bin", 0x000, 0x200, CRC(84d1c26a) SHA1(de823adddf949bf77d8478762720fe0b56fba8ea) )
ROM_END

// 19-in-1 is visibly different hardware, extent of differences unknown due to lack of quality pictures/scans
// it is also the only one of these that runs horizontally, which is very likely why it has its own data ROM
// also, there is a bootleg of the 19-in-1 which may have less or different protection
ROM_START( 19in1 )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "19in1.u8",    0x000000, 0x080000, CRC(87b0506c) SHA1(c43ae4b403864a28e56370685572fa02e7572e66) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, BAD_DUMP CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) ) // not the same flash rom as the vertical games

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "19in1_eeprom.bin", 0x000, 0x200, NO_DUMP )
ROM_END

ROM_START( rodent )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "exterminator.u2", 0x00000, 0x80000, CRC(23c1d21f) SHA1(349565b0f0a015196827707cabb8d9ce6560d2cc) )

	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "m29w160db.u19", 0x000000, 0x200000, CRC(665ee79c) SHA1(35896b97378e8cd78e99d4527b9dc7392e545e17) )

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "93c66.u32", 0x000, 0x200, CRC(c311c7bc) SHA1(8328002b7f6a8b7a3ffca079b7960bc990211d7b) )
ROM_END


// The following are dumps from I.A.M. slot machines

ROM_START( fruitwld ) // PCB451 - FRUIT WORLD FP101 sticker on PCB outside the lid
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "fruit world v111.u2", 0x00000, 0x80000, CRC(44092be5) SHA1(a579455c4581fc2f6be37979d651f3f685353e8e) )

	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "m5m29gt320.u19", 0x000000, 0x400000, CRC(8cc9799a) SHA1(5bec178d11c722e26bf380c19d99118e7223bcd7) ) // 1xxxxxxxxxxxxxxxxxxxxx = 0xFF, fruit-FPv101 string

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "at93c66a.u32", 0x000, 0x200, CRC(11245518) SHA1(6363568facbe12f7be95e994c551815e7c3682f4) )
ROM_END

ROM_START( fruitwlda ) // PCB383 - FRUIT WORLD FP101 sticker on PCB outside the lid, FRUIT WORLD V102 on another sticker on big FPGA
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "fruit world v110.u2", 0x00000, 0x80000, CRC(d81bdd3c) SHA1(79ec9d12bb94537655778ac1138d3611bda9179e) )

	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "m5m29gt320.u19", 0x000000, 0x400000, CRC(8cc9799a) SHA1(5bec178d11c722e26bf380c19d99118e7223bcd7) ) // 1xxxxxxxxxxxxxxxxxxxxx = 0xFF, fruit-FPv101 string

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "at93c66a.u32", 0x000, 0x200, CRC(9e2e676e) SHA1(c3bda9c63118e9efb41445d941be44d4499b694f) )
ROM_END

ROM_START( jumanji ) // PCB383 - CHZ FP100 sticker on RAM under the lid. Dump was presented as Jumanji but has CHZ both on stickers and in ROM strings.
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u2", 0x00000, 0x80000, CRC(45bd43c7) SHA1(0da61fc1f5f17b2b9531ccfc69495a61aa272efd) ) // no label

	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "flash.u19", 0x000000, 0x400000, NO_DUMP )

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "93c66.u32", 0x000, 0x200, NO_DUMP )
ROM_END

ROM_START( jumanjia ) // PCB383 - Jumanji FP101 sticker on RAM under the lid.
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "jumanji_113_fp101.u2", 0x00000, 0x80000, CRC(40a66c50) SHA1(8909db087a8527af8ce229b03e2f7f16160db6f0) )

	ROM_REGION32_LE( 0x400000, "data", 0 )
	ROM_LOAD( "m5m29gt320.u19", 0x000000, 0x400000, CRC(4cd694fe) SHA1(723c0ed2af994cc584654dbf0d779e1c90827c7b) )

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "at93c66.u32", 0x000, 0x200, CRC(60930a27) SHA1(9222b23d64d85f664037ba180f79045c108fed9c) )
ROM_END

ROM_START( plutus ) // PCB451 - PLUTUS FP100 sticker on PCB outside the lid
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "plutus v100.u2", 0x00000, 0x80000, CRC(3ac49895) SHA1(6de3dcac42afc4d9f927c9c9accf592b3d974fd3) )

	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "m5m29gt320.u19", 0x000000, 0x400000, CRC(352387e7) SHA1(24e2d98681791f42033a58721b5af9cc6a04ebe4) ) // PLUTUS-FP100 string

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "at93c66a.u32", 0x000, 0x200, CRC(625eb014) SHA1(f1fb0777ce8a12ee09d882cd843c07daea14145a) )
ROM_END

ROM_START( pokrwild ) // PCB451 - POKER'S WILD FP102 sticker on PCB outside the lid
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pokers wild v117.u2", 0x00000, 0x80000, CRC(96e18540) SHA1(b8fbf0a78a496e4ebea3e4603f4e3a52823c1f31) )

	ROM_REGION32_LE( 0x400000, "data", ROMREGION_ERASEFF )
	ROM_LOAD( "m5m29gt320.u19", 0x000000, 0x400000, CRC(824fd188) SHA1(38517a78e853a600abcb6256ff77482a250c6ee6) ) // 11xxxxxxxxxxxxxxxxxxxx = 0xFF, PKWILD-FP103 string

	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD( "at93c66a.u32", 0x000, 0x200, CRC(27c7a209) SHA1(4d8e0ab18adb882362d800e2c247b3e27e6949e1) )
ROM_END

} // anonymous namespace


GAME(2004, 4in1a,     39in1,    cpld,   39in1, _39in1_state, init_4in1a,    ROT90, "bootleg", "4 in 1 MAME bootleg (ver 3.00, PLZ-V014)",             MACHINE_IMPERFECT_SOUND)
GAME(2004, 4in1b,     39in1,    cpld,   39in1, _39in1_state, init_4in1b,    ROT90, "bootleg", "4 in 1 MAME bootleg (PLZ-V001)",                       MACHINE_IMPERFECT_SOUND)
GAME(2004, 19in1,     39in1,    cpld2,  39in1, _39in1_state, init_19in1,    ROT0,  "bootleg", "19 in 1 MAME bootleg (BAR-V000)",                      MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(2004, 39in1,     0,        cpld,   39in1, _39in1_state, init_39in1,    ROT90, "bootleg", "39 in 1 MAME bootleg (GNO-V000)",                      MACHINE_IMPERFECT_SOUND)
GAME(2004, 48in1,     39in1,    cpld2,  39in1, _39in1_state, init_48in1,    ROT90, "bootleg", "48 in 1 MAME bootleg (ver 3.09, HPH-V000)",            MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(2004, 48in1b,    39in1,    cpld2,  39in1, _39in1_state, init_48in1,    ROT90, "bootleg", "48 in 1 MAME bootleg (ver 3.09, HPH-V000, alt flash)", MACHINE_IMPERFECT_SOUND)
GAME(2004, 48in1a,    39in1,    cpld2,  39in1, _39in1_state, init_48in1a,   ROT90, "bootleg", "48 in 1 MAME bootleg (ver 3.02, HPH-V000)",            MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(2004, 48in1c,    39in1,    cpld2,  39in1, _39in1_state, init_48in1c,   ROT90, "bootleg", "48 in 1 MAME bootleg (ver 3.08, HPH-V000)",            MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(2004, 60in1,     39in1,    cpld2,  39in1, _39in1_state, init_60in1,    ROT90, "bootleg", "60 in 1 MAME bootleg (ver 3.00, ICD-V000)",            MACHINE_IMPERFECT_SOUND)
GAME(2005, rodent,    0,        cpld,   39in1, _39in1_state, init_rodent,   ROT0,  "The Game Room", "Rodent Exterminator",                            MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// I.A.M. slots. Versions are taken from program ROM stickers or ROM strings, where available
GAME(2008, fruitwld,  0,        iam,    39in1, _39in1_state, init_fruitwld, ROT0,  "I.A.M.",  "Fruit World (V111)",                                   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) // FRUIT_V111.BIN 2008-04-30 15:59:21
GAME(2007, fruitwlda, fruitwld, iam,    39in1, _39in1_state, init_fruitwld, ROT0,  "I.A.M.",  "Fruit World (V110)",                                   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) // FRUIT_V110.BIN 2007-07-26 13:46:30
GAME(2007, jumanji,   0,        iam,    39in1, _39in1_state, init_jumanji,  ROT0,  "I.A.M.",  "Jumanji (V502)",                                       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) // CHZ_V502.BIN 2007-07-26 13:49:35 in clear text at the end of the main CPU ROM
GAME(2007, jumanjia,  jumanji,  iam,    39in1, _39in1_state, init_jumanjia, ROT0,  "I.A.M.",  "Jumanji (V113)",                                       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) // JUMANJI_V113.BIN 2007-07-25 10:54:33
GAME(200?, plutus,    0,        iam2,   39in1, _39in1_state, init_plutus,   ROT0,  "I.A.M.",  "Plutus (V100)",                                        MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) // no string
GAME(200?, pokrwild,  0,        iam,    39in1, _39in1_state, init_pokrwild, ROT0,  "I.A.M.",  "Poker's Wild (V117)",                                  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) // no string
