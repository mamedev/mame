// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    nds_wifi.cpp
    Nintendo DS wireless module (MAC, baseband and RF chips)
    Emulation by R. Belmont

    This emulates the WiFi hardware of the Nintendo DS sufficiently for
    games like Order of Ecclesia to pass their WiFi checks.  It basically
    acts like a working radio that never detects a signal.

***************************************************************************/

#include "emu.h"
#include "nds_wifi.h"

#include <algorithm>
#include <iterator>

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NDS_WIFI, nds_wifi_device, "nds_wifi", "Nintendo DS wireless module")

namespace
{

enum : offs_t
{
	W_ID = 0x000,
	W_MODE_RST = 0x004,
	W_MODE_WEP = 0x006,
	W_TXSTATCNT = 0x008,
	W_X_00A = 0x00a,
	W_IF = 0x010,
	W_IE = 0x012,
	W_MACADDR_0 = 0x018,
	W_MACADDR_1 = 0x01a,
	W_MACADDR_2 = 0x01c,
	W_BSSID_0 = 0x020,
	W_BSSID_1 = 0x022,
	W_BSSID_2 = 0x024,
	W_AID_LOW = 0x028,
	W_AID_FULL = 0x02a,
	W_TX_RETRYLIMIT = 0x02c,
	W_INTERNAL_02E = 0x02e,
	W_RXCNT = 0x030,
	W_INTERNAL_034 = 0x034,
	W_POWER_US = 0x036,
	W_POWER_TX = 0x038,
	W_POWERSTATE = 0x03c,
	W_POWERFORCE = 0x040,
	W_RANDOM = 0x044,
	W_RXBUF_BEGIN = 0x050,
	W_RXBUF_END = 0x052,
	W_RXBUF_WRCSR = 0x054,
	W_RXBUF_WR_ADDR = 0x056,
	W_RXBUF_RD_ADDR = 0x058,
	W_RXBUF_COUNT = 0x05c,
	W_RXBUF_RD_DATA = 0x060,
	W_RXBUF_GAP = 0x062,
	W_RXBUF_GAPDISP = 0x064,
	W_TXBUF_WR_ADDR = 0x068,
	W_TXBUF_COUNT = 0x06c,
	W_TXBUF_WR_DATA = 0x070,
	W_TXBUF_GAP = 0x074,
	W_TXBUF_GAPDISP = 0x076,
	W_TXBUF_WR_ADDR_MIRROR = 0x078,
	W_TXBUF_BEACON = 0x080,
	W_TXBUF_TIM = 0x084,
	W_LISTENCOUNT = 0x088,
	W_BEACONINT = 0x08c,
	W_LISTENINT = 0x08e,
	W_TXBUF_CMD = 0x090,
	W_TXBUF_REPLY1 = 0x094,
	W_TXBUF_REPLY2 = 0x098,
	W_TXBUF_LOC1 = 0x0a0,
	W_TXBUF_LOC2 = 0x0a4,
	W_TXBUF_LOC3 = 0x0a8,
	W_TXREQ_RESET = 0x0ac,
	W_TXREQ_SET = 0x0ae,
	W_TXREQ_READ = 0x0b0,
	W_TXBUF_RESET = 0x0b4,
	W_TXBUSY = 0x0b6,
	W_TXSTAT = 0x0b8,
	W_PREAMBLE = 0x0bc,
	W_CMD_TOTALTIME = 0x0c0,
	W_CMD_REPLYTIME = 0x0c4,
	W_RXFILTER = 0x0d0,
	W_CONFIG_0D4 = 0x0d4,
	W_RXFILTER2 = 0x0e0,
	W_US_COUNTCNT = 0x0e8,
	W_US_COMPARECNT = 0x0ea,
	W_CONFIG_0EC = 0x0ec,
	W_US_COMPARE0 = 0x0f0,
	W_US_COUNT0 = 0x0f8,
	W_CONTENTFREE = 0x10c,
	W_PRE_BEACON = 0x110,
	W_BEACON_COUNT = 0x11c,
	W_POST_BEACON = 0x134,
	W_BB_CNT = 0x158,
	W_BB_WRITE = 0x15a,
	W_BB_READ = 0x15c,
	W_BB_BUSY = 0x15e,
	W_INTERNAL_178 = 0x178,
	W_RF_DATA2 = 0x17c,
	W_RF_DATA1 = 0x17e,
	W_RF_BUSY = 0x180,
	W_RF_CNT = 0x184,
	W_TX_HDR_CNT = 0x194,
	W_INTERNAL_198 = 0x198,
	W_RF_PINS = 0x19c,
	W_X_1A2 = 0x1a2,
	W_X_1A4 = 0x1a4,
	W_TX_ERR_COUNT = 0x1c0,
	W_TX_SEQNO = 0x210,
	W_RF_STATUS = 0x214,
	W_IF_SET = 0x21c,
	W_INTERNAL_224 = 0x224,
	W_INTERNAL_230 = 0x230,
	W_CONFIG_254 = 0x254,
	W_INTERNAL_278 = 0x278,
	W_INTERNAL_27C = 0x27c
};

// transmit slots, numbered like the W_TXREQ_READ/W_TXBUSY bits
enum : int
{
	TX_LOC1 = 0,
	TX_CMD,
	TX_LOC2,
	TX_LOC3,
	TX_BEACON
};

static constexpr offs_t TX_LOC[5] = { W_TXBUF_LOC1, W_TXBUF_CMD, W_TXBUF_LOC2, W_TXBUF_LOC3, W_TXBUF_BEACON };

static constexpr struct { offs_t address; uint16_t value; } RESET_VALUES[] =
{
	{ W_TX_RETRYLIMIT, 0x0707 }, { W_POWER_US, 0x0001 }, { W_POWER_TX, 0x0003 }, { W_POWERSTATE, 0x0200 },
	{ W_RXBUF_BEGIN, 0x4000 }, { W_RXBUF_END, 0x4800 }, { W_BEACONINT, 0x0064 }, { 0x09c, 0x0050 },
	{ W_TXREQ_READ, 0x0010 }, { W_PREAMBLE, 0x0001 }, { W_RXFILTER, 0x0401 }, { W_CONFIG_0D4, 0x0001 },
	{ 0x0d8, 0x0004 }, { 0x0da, 0x0602 }, { W_RXFILTER2, 0x0008 }, { W_CONFIG_0EC, 0x3f03 },
	{ 0x0ee, 0x0001 }, { 0x0f0, 0xfc00 }, { 0x0f2, 0xffff }, { 0x0f4, 0xffff }, { 0x0f6, 0xffff },
	{ 0x120, 0x0048 }, { 0x122, 0x4840 }, { 0x126, 0x0080 }, { 0x12a, 0x1000 }, { 0x130, 0x0142 },
	{ 0x132, 0x8064 }, { W_POST_BEACON, 0xffff }, { 0x142, 0x2443 }, { 0x144, 0x0042 }, { 0x146, 0x0016 },
	{ 0x148, 0x0016 }, { 0x14a, 0x0016 }, { 0x14c, 0x162c }, { 0x150, 0x0204 }, { 0x154, 0x0058 },
	{ W_BB_CNT, 0x00b5 }, { W_BB_READ, 0x00b5 }, { 0x160, 0x0100 }, { 0x168, 0x800d }, { 0x16a, 0x0001 },
	{ W_RF_DATA2, 0x0800 }, { W_RF_DATA1, 0xc008 }, { W_RF_CNT, 0x0018 }, { W_RF_PINS, 0x0004 },
	{ W_X_1A2, 0x0001 }, { W_RF_STATUS, 0x0009 }, { W_INTERNAL_224, 0x0003 }, { W_INTERNAL_230, 0x0047 },
	{ 0x234, 0x0eff }, { 0x260, 0x0fef }, { 0x268, 0x0005 }, { 0x274, 0x0001 }, { W_INTERNAL_278, 0x000f },
	{ W_INTERNAL_27C, 0x000a }, { 0x290, 0xffff }, { 0x2a2, 0x7fff }, { 0x2ac, 0x0038 }, { 0x2c4, 0x000a }
};

// bits the CPU can write in each plain register
uint16_t write_mask(offs_t address)
{
	switch (address)
	{
		case 0x006: return 0x007f;
		case 0x008: case 0x00a: case 0x012: return 0xffff;
		case 0x018: case 0x01a: case 0x01c: case 0x020: case 0x022: case 0x024: return 0xffff;
		case 0x028: return 0x000f;
		case 0x02a: return 0x07ff;
		case 0x02c: return 0xffff;
		case 0x02e: return 0x0001;
		case 0x030: return 0xff0e;
		case 0x032: return 0xffff;
		case 0x038: return 0x0007;
		case 0x048: return 0x0003;
		case 0x050: case 0x052: return 0xffff;
		case 0x056: return 0x0fff;
		case 0x058: return 0x1ffe;
		case 0x05a: case 0x05c: return 0x0fff;
		case 0x062: return 0x1ffe;
		case 0x064: return 0x0fff;
		case 0x068: return 0x1ffe;
		case 0x06c: return 0x0fff;
		case 0x074: return 0x1ffe;
		case 0x076: return 0x0fff;
		case 0x080: return 0xffff;
		case 0x084: case 0x088: return 0x00ff;
		case 0x08c: return 0x03ff;
		case 0x08e: return 0x00ff;
		case 0x090: case 0x094: case 0x09c: return 0xffff;
		case 0x0a0: case 0x0a4: case 0x0a8: return 0xffff;
		case 0x0bc: return 0x0003;
		case 0x0c0: case 0x0c4: return 0xffff;
		case 0x0d0: return 0x1fff;
		case 0x0d4: return 0x0003;
		case 0x0d8: return 0x0fff;
		case 0x0da: return 0xffff;
		case 0x0e0: return 0x000f;
		case 0x0ea: return 0x0001;
		case 0x0ec: return 0x3f1f;
		case 0x0ee: return 0x0001;
		case 0x0f0: return 0xfc00;
		case 0x0f2: case 0x0f4: case 0x0f6: return 0xffff;
		case 0x110: case 0x118: case 0x11c: return 0xffff;
		case 0x120: return 0x81ff;
		case 0x122: case 0x124: case 0x128: return 0xffff;
		case 0x130: return 0x0fff;
		case 0x132: return 0x8fff;
		case 0x134: case 0x140: case 0x142: return 0xffff;
		case 0x144: case 0x146: case 0x148: case 0x14a: return 0x00ff;
		case 0x14c: return 0xffff;
		case 0x150: return 0xff3f;
		case 0x154: return 0x7a7f;
		case 0x15a: return 0x00ff;
		case 0x160: return 0x4100;
		case 0x168: return 0x800f;
		case 0x17e: return 0xffff;
		case 0x184: return 0x413f;
		case 0x190: return 0xffff;
		case 0x194: return 0x0007;
		case 0x198: return 0x000f;
		case 0x1a0: return 0x0933;
		case 0x1a2: return 0x0003;
		case 0x1a4: case 0x1aa: case 0x1ae: return 0xffff;
		case 0x1b0: return 0x00ff;
		case 0x1b2: return 0xffff;
		case 0x1b4: return 0x00ff;
		case 0x1b6: return 0xffff;
		case 0x1b8: case 0x1ba: return 0x00ff;
		case 0x1bc: case 0x1be: return 0xffff;
		case 0x1c0: return 0x00ff;
		case 0x1d0: return 0xff00;
		case 0x1d2: case 0x1d4: case 0x1d6: case 0x1d8: case 0x1da: case 0x1dc: case 0x1de: return 0xffff;
		case 0x1f0: return 0x0003;
		case 0x220: return 0xffff;
		case 0x224: return 0x0003;
		case 0x230: return 0x00ff;
		case 0x234: return 0x0eff;
		case 0x238: case 0x244: case 0x248: case 0x278: return 0xffff;
		case 0x2a0: return 0xffff;
		case 0x2b4: return 0x0103;
		case 0x2c0: return 0x0001;
		case 0x2f0: case 0x2f2: case 0x2f4: case 0x2f6: return 0xffff;
		default: return 0;
	}
}

} // anonymous namespace


nds_wifi_device::nds_wifi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NDS_WIFI, tag, owner, clock)
	, m_irq_cb(*this)
	, m_ram(*this, "ram", RAM_WORDS * 2, ENDIANNESS_LITTLE)
	, m_ms_timer(nullptr)
	, m_pre_beacon_timer(nullptr)
	, m_tx_timer(nullptr)
	, m_regs{}
	, m_bb{}
	, m_rf{}
	, m_us_base(0)
	, m_us_next(0)
	, m_random(0x7ff)
	, m_irq_state(false)
	, m_tx_slot(-1)
	, m_tx_phase(0)
	, m_tx_ok(false)
{
}

void nds_wifi_device::device_start()
{
	m_ms_timer = timer_alloc(FUNC(nds_wifi_device::ms_tick), this);
	m_pre_beacon_timer = timer_alloc(FUNC(nds_wifi_device::pre_beacon), this);
	m_tx_timer = timer_alloc(FUNC(nds_wifi_device::tx_event), this);

	save_item(NAME(m_regs));
	save_item(NAME(m_bb));
	save_item(NAME(m_rf));
	save_item(NAME(m_us_base));
	save_item(NAME(m_us_time));
	save_item(NAME(m_us_next));
	save_item(NAME(m_contentfree_time));
	save_item(NAME(m_random_time));
	save_item(NAME(m_random));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_tx_slot));
	save_item(NAME(m_tx_phase));
	save_item(NAME(m_tx_ok));
}

void nds_wifi_device::device_reset()
{
	// Wifi RAM is not cleared by a reset
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	for (const auto &r : RESET_VALUES)
	{
		reg(r.address) = r.value;
	}

	// BB registers are loaded from the firmware at boot; these read as constants
	std::fill(std::begin(m_bb), std::end(m_bb), 0);
	m_bb[0x00] = 0x6d;
	m_bb[0x5d] = 0x01;
	m_bb[0x64] = 0xff;
	std::fill(std::begin(m_rf), std::end(m_rf), 0);

	m_us_base = 0;
	m_us_time = machine().time();
	m_us_next = 0;
	m_contentfree_time = machine().time();
	m_random_time = machine().time();
	m_random = 0x7ff;
	m_tx_slot = -1;
	m_tx_phase = 0;
	m_tx_ok = false;
	m_ms_timer->adjust(attotime::never);
	m_pre_beacon_timer->adjust(attotime::never);
	m_tx_timer->adjust(attotime::never);

	m_irq_state = false;
	m_irq_cb(0);
}

// WS0 and WS1 each mirror the registers at 0000-0fff and the RAM at 4000-5fff
uint16_t nds_wifi_device::read(offs_t offset)
{
	const offs_t address = (offset << 1) & 0x7ffe;
	if (address < 0x1000)
	{
		return reg_r(address);
	}
	if ((address >= 0x4000) && (address < 0x6000))
	{
		return ram_r16(address);
	}
	return 0xffff;
}

void nds_wifi_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// byte writes are ignored
	if (mem_mask != 0xffff)
	{
		return;
	}

	const offs_t address = (offset << 1) & 0x7ffe;
	if (address < 0x1000)
	{
		reg_w(address, data);
	}
	else if ((address >= 0x4000) && (address < 0x6000))
	{
		ram_w16(address, data);
	}
}

uint16_t nds_wifi_device::ram_r16(offs_t address) const
{
	return m_ram[(address & 0x1ffe) >> 1];
}

void nds_wifi_device::ram_w16(offs_t address, uint16_t data)
{
	m_ram[(address & 0x1ffe) >> 1] = data;
}

uint16_t nds_wifi_device::reg_r(offs_t address)
{
	switch (address)
	{
		case W_ID:
			return 0x1440;

		case W_RANDOM:
		{
			// X = (X AND 1) XOR (X ROL 1) within 11 bits, stepped at the bus clock
			const uint16_t result = m_random;
			if (!machine().side_effects_disabled())
			{
				const uint64_t steps = (machine().time() - m_random_time).as_ticks(clock()) % 0x5fd;
				m_random_time = machine().time();
				for (uint64_t i = 0; i < steps; i++)
				{
					m_random = (m_random & 1) ^ (((m_random << 1) | (m_random >> 10)) & 0x7ff);
				}
			}
			return result;
		}

		case W_RXBUF_RD_DATA:
		{
			const uint16_t data = ram_r16(reg(W_RXBUF_RD_ADDR));
			if (!machine().side_effects_disabled())
			{
				uint16_t addr = (reg(W_RXBUF_RD_ADDR) + 2) & 0x1ffe;
				if (addr == reg(W_RXBUF_GAP))
				{
					addr = (addr + (reg(W_RXBUF_GAPDISP) << 1)) & 0x1ffe;
				}
				reg(W_RXBUF_RD_ADDR) = addr;
				if (reg(W_RXBUF_COUNT) != 0)
				{
					reg(W_RXBUF_COUNT)--;
					if (reg(W_RXBUF_COUNT) == 0)
					{
						set_irq(9);
					}
				}
			}
			return data;
		}

		case W_TXBUF_WR_ADDR_MIRROR:
			return reg(W_TXBUF_WR_ADDR);

		case W_US_COUNT0:
		case W_US_COUNT0 + 2:
		case W_US_COUNT0 + 4:
		case W_US_COUNT0 + 6:
			return uint16_t(us_now() >> ((address - W_US_COUNT0) * 8));

		case W_CONTENTFREE:
		{
			const uint64_t elapsed = (machine().time() - m_contentfree_time).as_ticks(1'000'000);
			return (elapsed >= reg(W_CONTENTFREE)) ? 0 : uint16_t(reg(W_CONTENTFREE) - elapsed);
		}

		case W_BB_BUSY:
		case W_RF_BUSY:
			return 0;

		case W_INTERNAL_178:
			return reg(W_RF_DATA2);

		case W_CONFIG_254:
			return 0xffff;

		case W_TXBUF_WR_DATA:
		case W_TXREQ_RESET:
		case W_TXREQ_SET:
		case W_TXBUF_RESET:
		case W_IF_SET:
			return 0;

		default:
			return reg(address);
	}
}

void nds_wifi_device::reg_w(offs_t address, uint16_t data)
{
	LOG("%s: reg %03X = %04X\n", machine().describe_context(), address, data);

	switch (address)
	{
		case W_MODE_RST:
			mode_reset_w(data);
			break;

		case W_IF:
			reg(W_IF) &= ~data;
			update_irq();
			break;

		case W_IF_SET:
			reg(W_IF) |= data & 0xfbff;
			update_irq();
			break;

		case W_IE:
			reg(W_IE) = data;
			update_irq();
			break;

		case W_RXCNT:
			reg(W_RXCNT) = data & 0xff0e;
			if (BIT(data, 0))
			{
				reg(W_RXBUF_WRCSR) = reg(W_RXBUF_WR_ADDR);
			}
			if (BIT(data, 7))
			{
				reg(W_TXBUF_REPLY2) = reg(W_TXBUF_REPLY1);
				reg(W_TXBUF_REPLY1) = 0;
			}
			break;

		case W_POWER_US:
			us_latch();
			reg(W_POWER_US) = data & 0x0003;
			us_schedule();
			check_power_request();
			break;

		case W_POWERSTATE:
			reg(W_POWERSTATE) = (reg(W_POWERSTATE) & 0x0300) | (data & 0x0003);
			check_power_request();
			break;

		case W_POWERFORCE:
			reg(W_POWERFORCE) = data & 0x8001;
			if (BIT(data, 15) && BIT(data, 0) && !BIT(reg(W_POWERSTATE), 9))
			{
				reg(W_TXREQ_READ) = 0;
				power_down();
			}
			break;

		case W_TXBUF_WR_DATA:
		{
			ram_w16(reg(W_TXBUF_WR_ADDR), data);
			uint16_t addr = (reg(W_TXBUF_WR_ADDR) + 2) & 0x1ffe;
			if (addr == reg(W_TXBUF_GAP))
			{
				addr = (addr + (reg(W_TXBUF_GAPDISP) << 1)) & 0x1ffe;
			}
			reg(W_TXBUF_WR_ADDR) = addr;
			if (reg(W_TXBUF_COUNT) != 0)
			{
				reg(W_TXBUF_COUNT)--;
				if (reg(W_TXBUF_COUNT) == 0)
				{
					set_irq(8);
				}
			}
			break;
		}

		case W_TXBUF_CMD:
		case W_TXBUF_LOC1:
		case W_TXBUF_LOC2:
		case W_TXBUF_LOC3:
			reg(address) = data;
			tx_check();
			break;

		case W_TXREQ_RESET:
			reg(W_TXREQ_READ) &= ~(data & 0x000f);
			break;

		case W_TXREQ_SET:
			reg(W_TXREQ_READ) |= data & 0x000f;
			tx_check();
			break;

		case W_TXBUF_RESET:
			for (int slot = TX_LOC1; slot <= TX_LOC3; slot++)
			{
				if (BIT(data, slot))
				{
					reg(TX_LOC[slot]) &= 0x7fff;
				}
			}
			if (BIT(data, 6))
			{
				reg(W_TXBUF_REPLY2) &= 0x7fff;
			}
			if (BIT(data, 7))
			{
				reg(W_TXBUF_REPLY1) &= 0x7fff;
			}
			break;

		case W_PREAMBLE:
			reg(W_PREAMBLE) = data & 0x0007;
			break;

		case W_US_COUNTCNT:
			us_latch();
			reg(W_US_COUNTCNT) = data & 0x0001;
			us_schedule();
			break;

		case W_US_COMPARECNT:
			reg(W_US_COMPARECNT) = data & 0x0001;
			if (BIT(data, 1))
			{
				beacon_event(false);
			}
			break;

		case W_US_COUNT0:
		case W_US_COUNT0 + 2:
		case W_US_COUNT0 + 4:
		case W_US_COUNT0 + 6:
		{
			us_latch();
			const int shift = (address - W_US_COUNT0) * 8;
			m_us_base = (m_us_base & ~(uint64_t(0xffff) << shift)) | (uint64_t(data) << shift);
			us_schedule();
			break;
		}

		case W_CONTENTFREE:
			reg(W_CONTENTFREE) = data;
			m_contentfree_time = machine().time();
			break;

		case W_BB_CNT:
		{
			const uint8_t index = data & 0xff;
			switch (data >> 12)
			{
				case 5:
					bb_write(index, reg(W_BB_WRITE) & 0xff);
					break;

				case 6:
					reg(W_BB_READ) = bb_read(index);
					break;

				default:
					logerror("%s: unknown BB transfer %04X\n", machine().describe_context(), data);
					break;
			}
			break;
		}

		case W_RF_DATA2:
			reg(W_RF_DATA2) = data;
			rf_transfer();
			break;

		default:
			reg(address) = (reg(address) & ~write_mask(address)) | (data & write_mask(address));
			break;
	}
}

void nds_wifi_device::set_irq(int bit)
{
	reg(W_IF) |= 1 << bit;
	update_irq();
}

void nds_wifi_device::update_irq()
{
	const bool state = (reg(W_IF) & reg(W_IE)) != 0;
	if (state != m_irq_state)
	{
		m_irq_state = state;
		m_irq_cb(state ? 1 : 0);
	}
}

void nds_wifi_device::mode_reset_w(uint16_t data)
{
	const uint16_t old = reg(W_MODE_RST);
	reg(W_MODE_RST) = data & 0x9fff;

	if (!BIT(old, 0) && BIT(data, 0))
	{
		reg(W_INTERNAL_034) = 0x0002;
		reg(W_RF_PINS) = 0x0046;
		reg(W_RF_STATUS) = 0x0009;
		reg(W_INTERNAL_27C) = 0x0005;
	}
	else if (BIT(old, 0) && !BIT(data, 0))
	{
		reg(W_INTERNAL_27C) = 0x000a;
	}

	if (BIT(data, 13))
	{
		reg(W_RXBUF_WR_ADDR) = 0;
		reg(W_CMD_TOTALTIME) = 0;
		reg(W_CMD_REPLYTIME) = 0;
		reg(W_X_1A4) = 0;
		reg(W_INTERNAL_278) = 0x000f;
	}

	if (BIT(data, 14))
	{
		reset_mac();
	}

	check_power_request();
	tx_check();
}

void nds_wifi_device::reset_mac()
{
	static constexpr offs_t CLEARED[] =
	{
		W_MODE_WEP, W_TXSTATCNT, W_X_00A, W_MACADDR_0, W_MACADDR_1, W_MACADDR_2, W_BSSID_0, W_BSSID_1, W_BSSID_2,
		W_AID_LOW, W_AID_FULL, W_INTERNAL_02E, W_TXBUF_TIM, W_TX_HDR_CNT, W_INTERNAL_198
	};
	for (const offs_t address : CLEARED)
	{
		reg(address) = 0;
	}
	reg(W_TX_RETRYLIMIT) = 0x0707;
	reg(W_RXBUF_BEGIN) = 0x4000;
	reg(W_RXBUF_END) = 0x4800;
	reg(W_PREAMBLE) = 0x0001;
	reg(W_RXFILTER) = 0x0401;
	reg(W_CONFIG_0D4) = 0x0001;
	reg(W_RXFILTER2) = 0x0008;
	reg(W_CONFIG_0EC) = 0x3f03;
	reg(W_X_1A2) = 0x0001;
	reg(W_INTERNAL_224) = 0x0003;
	reg(W_INTERNAL_230) = 0x0047;
}

// a queued power-up request is honoured once the clock runs and the MAC is enabled
void nds_wifi_device::check_power_request()
{
	if (BIT(reg(W_POWERSTATE), 1) && BIT(reg(W_POWERSTATE), 9) && !BIT(reg(W_POWER_US), 0) && BIT(reg(W_MODE_RST), 0))
	{
		power_up(true);
	}
}

void nds_wifi_device::power_down()
{
	reg(W_INTERNAL_034) = 0x0002;
	reg(W_POWERSTATE) |= 0x0200;
	reg(W_RF_PINS) = 0x0046;
	reg(W_RF_STATUS) = 0x0009;
}

void nds_wifi_device::power_up(bool wakeup_irq)
{
	reg(W_POWERSTATE) &= ~0x0302;
	reg(W_INTERNAL_034) = 0x0000;
	reg(W_RF_PINS) = (reg(W_RF_PINS) & ~0x0040) | 0x0080;
	reg(W_RF_STATUS) = 0x0001;
	if (wakeup_irq)
	{
		set_irq(11);
	}
	tx_check();
}

bool nds_wifi_device::us_running() const
{
	return BIT(reg(W_US_COUNTCNT), 0) && !BIT(reg(W_POWER_US), 0);
}

uint64_t nds_wifi_device::us_now() const
{
	if (!us_running())
	{
		return m_us_base;
	}
	return m_us_base + (machine().time() - m_us_time).as_ticks(1'000'000);
}

void nds_wifi_device::us_latch()
{
	m_us_base = us_now();
	m_us_time = machine().time();
}

void nds_wifi_device::us_schedule()
{
	m_pre_beacon_timer->adjust(attotime::never);
	if (!us_running())
	{
		m_ms_timer->adjust(attotime::never);
		return;
	}

	// the millisecond counters step when the low 10 bits of the counter wrap
	m_us_next = (m_us_base | 0x3ff) + 1;
	m_ms_timer->adjust(attotime::from_usec(m_us_next - m_us_base));
}

TIMER_CALLBACK_MEMBER(nds_wifi_device::ms_tick)
{
	m_us_base = m_us_next;
	m_us_time = machine().time();
	m_us_next = m_us_base + 0x400;
	m_ms_timer->adjust(attotime::from_usec(0x400));

	const uint64_t compare = (uint64_t(reg(W_US_COMPARE0 + 6)) << 48) | (uint64_t(reg(W_US_COMPARE0 + 4)) << 32) |
			(uint64_t(reg(W_US_COMPARE0 + 2)) << 16) | reg(W_US_COMPARE0);
	if (BIT(reg(W_US_COMPARECNT), 0) && ((m_us_base >> 10) == (compare >> 10)))
	{
		beacon_event(true);
	}

	if (reg(W_BEACON_COUNT) != 0)
	{
		reg(W_BEACON_COUNT)--;
		if (reg(W_BEACON_COUNT) == 0)
		{
			if (reg(W_PRE_BEACON) == 0)
			{
				pre_beacon(0);
			}
			beacon_event(true);
		}

		// IRQ15 comes W_PRE_BEACON microseconds ahead of the beacon slot
		const uint32_t until = uint32_t(reg(W_BEACON_COUNT)) * 0x400;
		const uint32_t pre = reg(W_PRE_BEACON);
		if ((pre != 0) && (pre <= until) && (pre > (until - 0x400)))
		{
			m_pre_beacon_timer->adjust(attotime::from_usec(until - pre));
		}
	}

	if (reg(W_POST_BEACON) != 0)
	{
		reg(W_POST_BEACON)--;
		if (reg(W_POST_BEACON) == 0)
		{
			set_irq(13);
			if (!BIT(reg(W_POWER_TX), 1))
			{
				power_down();
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(nds_wifi_device::pre_beacon)
{
	if (BIT(reg(W_US_COMPARECNT), 0))
	{
		set_irq(15);
	}
	if (BIT(reg(W_POWER_TX), 0) && BIT(reg(W_POWERSTATE), 9))
	{
		power_up(false);
	}
}

void nds_wifi_device::beacon_event(bool reload)
{
	if (reload)
	{
		reg(W_BEACON_COUNT) = reg(W_BEACONINT);
	}

	if (!BIT(reg(W_US_COMPARECNT), 0))
	{
		return;
	}

	set_irq(14);
	reg(W_POST_BEACON) = 0xffff;
	reg(W_TXREQ_READ) &= 0xfff2;
	if (reg(W_LISTENCOUNT) == 0)
	{
		reg(W_LISTENCOUNT) = reg(W_LISTENINT);
	}
	reg(W_LISTENCOUNT) = (reg(W_LISTENCOUNT) - 1) & 0xff;

	if (BIT(reg(W_TXBUF_BEACON), 15) && (m_tx_slot < 0) && !BIT(reg(W_POWERSTATE), 9))
	{
		tx_start(TX_BEACON);
	}
}

uint8_t nds_wifi_device::bb_read(uint8_t index) const
{
	if (index > 0x68)
	{
		return 0;
	}
	return m_bb[index];
}

void nds_wifi_device::bb_write(uint8_t index, uint8_t data)
{
	const bool writable = ((index >= 0x01) && (index <= 0x0c)) || ((index >= 0x13) && (index <= 0x15)) ||
			((index >= 0x1b) && (index <= 0x26)) || ((index >= 0x28) && (index <= 0x4c)) ||
			((index >= 0x4e) && (index <= 0x5c)) || (index == 0x62) || (index == 0x63) ||
			(index == 0x65) || (index == 0x67) || (index == 0x68);
	if (writable)
	{
		m_bb[index] = data;
	}
}

void nds_wifi_device::rf_transfer()
{
	const uint16_t data1 = reg(W_RF_DATA1);
	const uint16_t data2 = reg(W_RF_DATA2);
	if (!BIT(reg(W_RF_CNT), 8))
	{
		// RF9008: 18-bit registers addressed by DATA2 bits 2-6
		const int index = (data2 >> 2) & 0x1f;
		if (BIT(data2, 7))
		{
			reg(W_RF_DATA1) = m_rf[index] & 0xffff;
			reg(W_RF_DATA2) = (data2 & ~0x0003) | ((m_rf[index] >> 16) & 0x0003);
		}
		else
		{
			m_rf[index] = ((data2 & 0x0003) << 16) | data1;
		}
	}
	else
	{
		// MM3218: 8-bit registers addressed by DATA1 bits 8-13
		const int index = (data1 >> 8) & 0x3f;
		switch (data2 & 0x000f)
		{
			case 5:
				m_rf[index] = data1 & 0xff;
				break;

			case 6:
				reg(W_RF_DATA1) = (data1 & 0xff00) | (m_rf[index] & 0xff);
				break;
		}
	}
}

// nobody answers: group frames succeed, unicast frames fail after their retries
void nds_wifi_device::tx_check()
{
	if ((m_tx_slot >= 0) || !BIT(reg(W_MODE_RST), 0) || BIT(reg(W_POWERSTATE), 9))
	{
		return;
	}

	// LOC3 goes first and LOC1 last
	static constexpr int TX_ORDER[4] = { TX_LOC3, TX_LOC2, TX_LOC1, TX_CMD };
	for (const int slot : TX_ORDER)
	{
		if (BIT(reg(W_TXREQ_READ), slot) && BIT(reg(TX_LOC[slot]), 15))
		{
			tx_start(slot);
			return;
		}
	}
}

void nds_wifi_device::tx_start(int slot)
{
	const offs_t header = (reg(TX_LOC[slot]) & 0x0fff) << 1;
	const uint8_t rate = ram_r16(header + 8) & 0xff;
	const uint32_t length = ram_r16(header + 0x0a) & 0x3fff;
	const bool fast = (rate == 0x14);
	const uint32_t preamble = (fast && BIT(reg(W_PREAMBLE), 2)) ? 96 : 192;

	// broadcast and group frames need no acknowledgement
	const bool group = BIT(ram_r16(header + 12 + 4), 0);
	m_tx_ok = group || (slot == TX_BEACON) || (slot == TX_CMD);

	m_tx_slot = slot;
	m_tx_phase = 0;
	reg(W_TXBUSY) |= 1 << slot;
	reg(W_RF_PINS) = (reg(W_RF_PINS) & ~0x0080) | 0x0040;
	reg(W_RF_STATUS) = 0x0003;

	uint32_t duration = preamble + (length * (fast ? 4 : 8));
	if (!m_tx_ok)
	{
		duration *= 1 + (reg(W_TX_RETRYLIMIT) & 0xff);
	}

	LOG("TX slot %d header %04X rate %02X length %d %s\n", slot, header, rate, length, m_tx_ok ? "ok" : "no ack");
	m_tx_timer->adjust(attotime::from_usec(preamble), duration - preamble);
}

TIMER_CALLBACK_MEMBER(nds_wifi_device::tx_event)
{
	const int slot = m_tx_slot;
	const offs_t header = (reg(TX_LOC[slot]) & 0x0fff) << 1;

	if (m_tx_phase == 0)
	{
		// the preamble is out: the frame body starts
		set_irq(7);
		reg(W_RF_PINS) |= 0x0002;
		const bool own_seqno = (slot == TX_LOC1) || (slot == TX_LOC2) || (slot == TX_LOC3);
		if ((slot == TX_BEACON) || (own_seqno && !BIT(reg(TX_LOC[slot]), 13) && ((ram_r16(header + 4) & 0xff) == 0) && !BIT(reg(W_TX_HDR_CNT), 2)))
		{
			ram_w16(header + 12 + 22, reg(W_TX_SEQNO) << 4);
			reg(W_TX_SEQNO) = (reg(W_TX_SEQNO) + 1) & 0x0fff;
		}
		m_tx_phase = 1;
		m_tx_timer->adjust(attotime::from_usec(param));
		return;
	}

	ram_w16(header, m_tx_ok ? 0x0001 : 0x0003);
	ram_w16(header + 4, ram_r16(header + 4) & 0x00ff);
	if (slot != TX_BEACON)
	{
		reg(TX_LOC[slot]) &= 0x7fff;
	}
	reg(W_TXBUSY) &= ~(1 << slot);
	reg(W_RF_PINS) = (reg(W_RF_PINS) & ~0x0042) | 0x0080;
	reg(W_RF_STATUS) = 0x0001;
	m_tx_slot = -1;

	switch (slot)
	{
		case TX_BEACON:
			if (BIT(reg(W_TXSTATCNT), 15))
			{
				reg(W_TXSTAT) = 0x0301;
				set_irq(1);
			}
			break;

		case TX_CMD:
			if (BIT(reg(W_TXSTATCNT), 14))
			{
				reg(W_TXSTAT) = 0x0800;
				set_irq(1);
			}
			set_irq(12);
			break;

		default:
		{
			const int loc = (slot == TX_LOC1) ? 0 : (slot == TX_LOC2) ? 1 : 2;
			reg(W_TXSTAT) = 0x0001 | (m_tx_ok ? 0 : 0x0002) | (BIT(reg(TX_LOC[slot]), 12) ? 0x0700 : 0) | (loc << 12);
			set_irq(1);
			break;
		}
	}

	if (!m_tx_ok)
	{
		reg(W_TX_ERR_COUNT) = (reg(W_TX_ERR_COUNT) + 1) & 0xff;
		set_irq(3);
	}

	tx_check();
}
