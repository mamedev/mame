// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Silicon Graphics Video Controller (VC1).
 *
 * TODO:
 *  - everything; this stub is only enough to pass power-on diagnostics
 */

#include "emu.h"
#include "vc1.h"

//#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_VC1, sgi_vc1_device, "sgi_vc1", "SGI VC1")

sgi_vc1_device::sgi_vc1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SGI_VC1, tag, owner, clock)
{
}

void sgi_vc1_device::device_start()
{
	m_ram = std::make_unique<u8[]>(0x8000); // TC55328J-35 32,768x8 CMOS STATIC RAM

	save_item(NAME(m_addr));
	save_item(NAME(m_reg));
	save_item(NAME(m_xmap));

	save_pointer(NAME(m_ram), 0x8000);
}

void sgi_vc1_device::device_reset()
{
	m_addr = 0;
}

enum vc1_cmd : unsigned
{
	REGISTER  = 0, // video timing, cursor control, display id control, xmap control
	XMAP_MODE = 1,
	EXT_RAM   = 2,
	TEST      = 3, // test registers
	ADDR_LO   = 4,
	ADDR_HI   = 5,
	SYS_CTRL  = 6,
};

u8 sgi_vc1_device::read(offs_t offset)
{
	switch (offset)
	{
	case REGISTER:
		return m_reg[m_addr++];
	case XMAP_MODE:
		return m_xmap[m_addr++];
	case EXT_RAM:
		return m_ram[m_addr & 0x7fffU];
	case TEST:
		return 0; // vc1 revision?
	case ADDR_LO:
		return u8(m_addr);
	case ADDR_HI:
		return m_addr >> 8;
	default:
		LOG("read offset %d\n", offset);
		return 0;
	}
}

enum vc1_reg : unsigned
{
	// video timing generator
	VID_EP       = 0x00,
	VID_LC       = 0x02,
	VID_SC       = 0x04,
	VID_TSA      = 0x06,
	VID_TSB      = 0x07,
	VID_TSC      = 0x08,
	VID_LP       = 0x09,
	VID_LS_EP    = 0x0b,
	VID_LR       = 0x0d,
	VID_FC       = 0x10,
	VID_ENABLE   = 0x14,

	// cursor generator
	CUR_EP       = 0x20,
	CUR_XL       = 0x22,
	CUR_YL       = 0x24,
	CUR_MODE     = 0x26,
	CUR_BX       = 0x27,
	CUR_LY       = 0x28,
	CUR_YC       = 0x2a,
	CUR_XC       = 0x2c,
	CUR_CC       = 0x2e,
	CUR_RC       = 0x30,

	// display id generation
	DID_EP       = 0x40,
	DID_END_EP   = 0x42,
	DID_HOR_DIV  = 0x44,
	DID_HOR_MOD  = 0x45,
	DID_LP       = 0x46,
	DID_LS_EP    = 0x48,
	DID_WC       = 0x4a,
	DID_RC       = 0x4c,
	DID_XL       = 0x4e,
	DID_XD5      = 0x50,
	DID_XM5      = 0x51,
	DID_DID      = 0x52,

	// control register
	BLKOUT_EVEN  = 0x60,
	BLKOUT_ODD   = 0x61,
	AUXVIDEO_MAP = 0x62,
};

static const u8 reg_mask[] =
{
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

	0xff, 0xff, 0x07, 0xff, 0x0f, 0xff, 0xff, 0xff,
	0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

	0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

void sgi_vc1_device::write(offs_t offset, u8 data)
{
	switch (offset)
	{
	case REGISTER:
		if (m_addr < std::size(reg_mask))
		{
			m_reg[m_addr] = data & reg_mask[m_addr];

			if (VERBOSE & LOG_GENERAL)
			{
				static const char *const reg_name[] =
				{
					"VID_EP", "", "VID_LC", "", "VID_SC", "", "VID_TSA", "VID_TSB",
					"VID_TSC", "VID_LP", nullptr, "VID_LS_EP", nullptr, "VID_LR", nullptr, nullptr,
					"VID_FC", "", nullptr, nullptr, "VID_ENABLE", "", nullptr, nullptr,
					nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,

					"CUR_EP", "", "CUR_XL", "", "CUR_YL", "", "CUR_MODE", "CUR_BX",
					"CUR_LY", "", "CUR_YC", "", "CUR_XC", "", "CUR_CC", "",
					"CUR_RC", "", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
					nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,

					"DID_EP", "", "DID_END_EP", "", "DID_HOR_DIV", "DID_HOR_MOD", "DID_LP", "",
					"DID_LS_EP", "", "DID_WC", "", "DID_RC", "", "DID_XL", "",
					"DID_XD5", "DID_XM5", "DID_DID", "", nullptr, nullptr, nullptr, nullptr,
					nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,

					"BLKOUT_EVEN", "BLKOUT_ODD", "AUXVIDEO_MAP", "", nullptr, nullptr, nullptr, nullptr,
					nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
					nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
					nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
				};

				if (!reg_name[m_addr])
					LOG("reg_w %s(0x%02x) data 0x%02x\n", "UNKNOWN", m_addr, m_reg[m_addr]);
				else if (std::string(reg_name[m_addr]).empty())
					LOG("reg_w %s(0x%02x) data 0x%04x\n", reg_name[m_addr - 1], m_addr - 1, u16(m_reg[m_addr - 1]) << 8 | m_reg[m_addr]);
				else if ((m_addr & 1) || !std::string(reg_name[m_addr + 1]).empty())
					LOG("reg_w %s(0x%02x) data 0x%02x\n", reg_name[m_addr], m_addr, m_reg[m_addr]);
			}
		}
		else
			LOG("write unknown register 0x%04x data 0x%02x\n", m_addr, data);
		m_addr++;
		break;
	case XMAP_MODE:
		m_xmap[m_addr++] = data;
		break;
	case EXT_RAM:
		m_ram[m_addr++ & 0x7fffU] = data;
		break;
	case ADDR_LO:
		m_addr = (m_addr & 0xff00U) | (data << 0);
		break;
	case ADDR_HI:
		m_addr = (m_addr & 0x00ffU) | (data << 8);
		break;
	default:
		LOG("write offset %d data 0x%02x\n", offset, data);
		break;
	}
}
