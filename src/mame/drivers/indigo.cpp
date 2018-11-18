// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************

	SGI Indigo workstation

	To-Do:
	- IP12 (R3000):
	 * Everything
	- IP20 (R4000):
	 * Figure out why the keyboard/mouse diagnostic fails
	 * Work out a proper RAM mapping, or why the installer bails due
	   to trying to access virtual address ffffa02c:
	   88002584: lw        $sp,-$5fd4($0)

**********************************************************************/

#include "emu.h"
//#include "cpu/dsp56k/dsp56k.h"
#include "cpu/mips/mips1.h"
#include "cpu/mips/mips3.h"
#include "machine/eepromser.h"
#include "machine/hpc1.h"
#include "machine/sgi.h"
#include "emupal.h"
#include "screen.h"

#define ENABLE_ENTRY_GFX	(1)

#define LOG_UNKNOWN		(1 << 0)
#define LOG_INT			(1 << 1)
#define LOG_DSP			(1 << 2)
#define LOG_GFX			(1 << 3)
#define LOG_GFX_CMD		(1 << 4)
#define LOG_ALL			(LOG_UNKNOWN | LOG_INT | LOG_DSP | LOG_GFX | LOG_GFX_CMD)

#define VERBOSE			(LOG_UNKNOWN)
#include "logmacro.h"

class indigo_state : public driver_device
{
public:
	indigo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_hpc(*this, "hpc")
		, m_eeprom(*this, "eeprom")
		, m_share1(*this, "share1")
		, m_dsp_ram(*this, "dspram")
		, m_palette(*this, "palette")
	{
	}

	void indigo_base(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ32_MEMBER(int_r);
	DECLARE_WRITE32_MEMBER(int_w);
	DECLARE_READ32_MEMBER(dsp_ram_r);
	DECLARE_WRITE32_MEMBER(dsp_ram_w);
	DECLARE_READ32_MEMBER(entry_r);
	DECLARE_WRITE32_MEMBER(entry_w);

	void do_rex_command();

	void indigo_map(address_map &map);

	uint32_t screen_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	enum
	{
		REX15_PAGE0_SET				= 0x00000000,
		REX15_PAGE0_GO				= 0x00000800,
		REX15_PAGE1_SET				= 0x00004790,
		REX15_PAGE1_GO				= 0x00004f90,

		REX15_P0REG_COMMAND			= 0x00000000,
		REX15_P0REG_XSTARTI			= 0x0000000c,
		REX15_P0REG_YSTARTI			= 0x0000001c,
		REX15_P0REG_XYMOVE			= 0x00000034,
		REX15_P0REG_COLORREDI		= 0x00000038,
		REX15_P0REG_COLORGREENI		= 0x00000040,
		REX15_P0REG_COLORBLUEI		= 0x00000048,
		REX15_P0REG_COLORBACK		= 0x0000005c,
		REX15_P0REG_ZPATTERN		= 0x00000060,
		REX15_P0REG_XENDI			= 0x00000084,
		REX15_P0REG_YENDI			= 0x00000088,

		REX15_P1REG_WCLOCKREV		= 0x00000054,
		REX15_P1REG_CFGDATA			= 0x00000058,
		REX15_P1REG_CFGSEL			= 0x0000005c,
		REX15_P1REG_VC1_ADDRDATA	= 0x00000060,
		REX15_P1REG_CFGMODE			= 0x00000068,
		REX15_P1REG_XYOFFSET		= 0x0000006c,

		REX15_OP_NOP				= 0x00000000,
		REX15_OP_DRAW				= 0x00000001,

		REX15_OP_FLAG_BLOCK			= 0x00000008,
		REX15_OP_FLAG_LENGTH32		= 0x00000010,
		REX15_OP_FLAG_QUADMODE		= 0x00000020,
		REX15_OP_FLAG_XYCONTINUE	= 0x00000080,
		REX15_OP_FLAG_STOPONX		= 0x00000100,
		REX15_OP_FLAG_STOPONY		= 0x00000200,
		REX15_OP_FLAG_ENZPATTERN	= 0x00000400,
		REX15_OP_FLAG_LOGICSRC		= 0x00080000,
		REX15_OP_FLAG_ZOPAQUE		= 0x00800000,
		REX15_OP_FLAG_ZCONTINUE		= 0x01000000,

		REX15_WRITE_ADDR			= 0x00,
		REX15_PALETTE_RAM			= 0x01,
		REX15_PIXEL_READ_MASK		= 0x02,
		REX15_CONTROL				= 0x06
	};

	struct lg1_t
	{
		uint32_t m_config_sel;
		uint32_t m_write_addr;
		uint32_t m_control;

		uint32_t m_command;
		uint32_t m_x_start_i;
		uint32_t m_y_start_i;
		uint32_t m_xy_move;
		uint32_t m_color_red_i;
		uint32_t m_color_green_i;
		uint32_t m_color_blue_i;
		uint32_t m_color_back;
		uint32_t m_z_pattern;
		uint32_t m_x_end_i;
		uint32_t m_y_end_i;
		uint32_t m_x_curr_i;
		uint32_t m_y_curr_i;

		uint8_t m_palette_idx;
		uint8_t m_palette_channel;
		uint8_t m_palette_entry[3];
		uint8_t m_pix_read_mask[256];
	};

	required_device<hpc1_device> m_hpc;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_shared_ptr<uint32_t> m_share1;
	required_shared_ptr<uint32_t> m_dsp_ram;
	required_device<palette_device> m_palette;

	address_space *m_space;

	lg1_t m_lg1;
	std::unique_ptr<uint8_t[]> m_framebuffer;
};

class indigo3k_state : public indigo_state
{
public:
	indigo3k_state(const machine_config &mconfig, device_type type, const char *tag)
		: indigo_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void indigo3k(machine_config &config);

protected:
	void mem_map(address_map &map);

	required_device<r3000a_device> m_maincpu;
};

class indigo4k_state : public indigo_state
{
public:
	indigo4k_state(const machine_config &mconfig, device_type type, const char *tag)
		: indigo_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mem_ctrl(*this, "memctrl")
	{
	}

	void indigo4k(machine_config &config);

protected:
	virtual void machine_reset() override;

	void mem_map(address_map &map);

	DECLARE_WRITE32_MEMBER(write_ram);

	required_device<r4000be_device> m_maincpu;
	required_device<sgi_mc_device> m_mem_ctrl;
};

void indigo_state::machine_start()
{
	m_framebuffer = std::make_unique<uint8_t[]>(1024*768);

	save_item(NAME(m_lg1.m_config_sel));
	save_item(NAME(m_lg1.m_write_addr));
	save_item(NAME(m_lg1.m_control));
	save_item(NAME(m_lg1.m_command));
	save_item(NAME(m_lg1.m_x_start_i));
	save_item(NAME(m_lg1.m_y_start_i));
	save_item(NAME(m_lg1.m_xy_move));
	save_item(NAME(m_lg1.m_color_red_i));
	save_item(NAME(m_lg1.m_color_green_i));
	save_item(NAME(m_lg1.m_color_blue_i));
	save_item(NAME(m_lg1.m_color_back));
	save_item(NAME(m_lg1.m_z_pattern));
	save_item(NAME(m_lg1.m_x_end_i));
	save_item(NAME(m_lg1.m_y_end_i));
	save_item(NAME(m_lg1.m_x_curr_i));
	save_item(NAME(m_lg1.m_y_curr_i));
	save_item(NAME(m_lg1.m_palette_idx));
	save_item(NAME(m_lg1.m_palette_channel));
	save_item(NAME(m_lg1.m_palette_entry));
	save_item(NAME(m_lg1.m_pix_read_mask));

	save_pointer(NAME(&m_framebuffer[0]), 1024*768);
}

void indigo_state::machine_reset()
{
	memset(&m_lg1, 0, sizeof(lg1_t));
	memset(&m_framebuffer[0], 0, 1024*768);
}

void indigo4k_state::machine_reset()
{
	indigo_state::machine_reset();

	// set up low RAM mirror
	membank("bank1")->set_base(m_share1);
}

READ32_MEMBER(indigo_state::int_r)
{
	LOGMASKED(LOG_INT, "%s: INT Read: %08x & %08x\n", machine().describe_context(), 0x1fbd9000 + offset*4, mem_mask);
	return 0;
}

WRITE32_MEMBER(indigo_state::int_w)
{
	LOGMASKED(LOG_INT, "%s: INT Write: %08x = %08x & %08x\n", machine().describe_context(), 0x1fbd9000 + offset*4, data, mem_mask);
}

READ32_MEMBER(indigo_state::dsp_ram_r)
{
	LOGMASKED(LOG_DSP, "%s: DSP RAM Read: %08x = %08x & %08x\n", machine().describe_context(), 0x1fbe0000 + offset*4, m_dsp_ram[offset], mem_mask);
	return m_dsp_ram[offset];
}

WRITE32_MEMBER(indigo_state::dsp_ram_w)
{
	LOGMASKED(LOG_DSP, "%s: DSP RAM Write: %08x = %08x & %08x\n", machine().describe_context(), 0x1fbe0000 + offset*4, data, mem_mask);
	COMBINE_DATA(&m_dsp_ram[offset]);
}

READ32_MEMBER(indigo_state::entry_r)
{
	uint32_t ret = 0;
	switch (offset)
	{
	case REX15_PAGE0_GO/4:
		LOGMASKED(LOG_GFX, "%s: LG1 Read: Status(?) (Go) %08x = %08x & %08x\n", machine().describe_context(), 0x1f3f0000 + offset*4, ret, mem_mask);
		do_rex_command();
		break;
	case 0x0014/4:
		ret = 0x033c0000;
		LOGMASKED(LOG_GFX, "%s: LG1 Read: Presence Detect(?) %08x = %08x & %08x\n", machine().describe_context(), 0x1f3f0000 + offset*4, ret, mem_mask);
		break;
	default:
		LOGMASKED(LOG_GFX, "%s: Unknown LG1 Read: %08x = %08x & %08x\n", machine().describe_context(), 0x1f3f0000 + offset*4, ret, mem_mask);
		break;
	}
	return ret;
}

void indigo_state::do_rex_command()
{
	if (m_lg1.m_command == 0)
	{
		return;
	}
	if (m_lg1.m_command == 0x30080329)
	{
		bool xycontinue = (m_lg1.m_command & REX15_OP_FLAG_XYCONTINUE);
		bool copy = (m_lg1.m_command & REX15_OP_FLAG_LOGICSRC);
		const uint32_t start_x = xycontinue ? m_lg1.m_x_curr_i : m_lg1.m_x_start_i;
		const uint32_t start_y = xycontinue ? m_lg1.m_y_curr_i : m_lg1.m_y_start_i;
		const uint32_t end_x = m_lg1.m_x_end_i;
		const uint32_t end_y = m_lg1.m_y_end_i;
		const uint32_t src_start_x = start_x + (m_lg1.m_xy_move >> 16);
		const uint32_t src_start_y = start_y + (uint16_t)m_lg1.m_xy_move;;

		LOGMASKED(LOG_GFX, "LG1: Command %08x: Block copy from %d,%d-%d,%d inclusive.\n", m_lg1.m_command, start_x, start_y, end_x, end_y);
		if (copy)
		{
			for (uint32_t y = start_y, src_y = src_start_y; y <= end_y; y++, src_y++)
				for (uint32_t x = start_x, src_x = src_start_x; x <= end_x; x++, src_x++)
					m_framebuffer[y*1024 + x] = m_framebuffer[src_y*1024 + src_x];
		}
		else
		{
			for (uint32_t y = start_y; y <= end_y; y++)
				for (uint32_t x = start_x; x <= end_x; x++)
					m_framebuffer[y*1024 + x] = m_lg1.m_color_red_i;
		}
	}
	else if (m_lg1.m_command == 0x30000329)
	{
		bool xycontinue = (m_lg1.m_command & REX15_OP_FLAG_XYCONTINUE);
		uint32_t start_x = xycontinue ? m_lg1.m_x_curr_i : m_lg1.m_x_start_i;
		uint32_t start_y = xycontinue ? m_lg1.m_y_curr_i : m_lg1.m_y_start_i;
		uint32_t end_x = m_lg1.m_x_end_i;
		uint32_t end_y = m_lg1.m_y_end_i;

		LOGMASKED(LOG_GFX, "LG1: Command %08x: Block draw from %d,%d-%d,%d inclusive.\n", m_lg1.m_command, start_x, start_y, end_x, end_y);
		for (uint32_t y = start_y; y <= end_y; y++)
		{
			for (uint32_t x = start_x; x <= end_x; x++)
			{
				m_framebuffer[y*1024 + x] = m_lg1.m_color_red_i;
			}
		}
	}
	else if (m_lg1.m_command == 0x300005a1 ||
             m_lg1.m_command == 0x300005a9 ||
             m_lg1.m_command == 0x300005b9)
	{
		bool xycontinue = (m_lg1.m_command & REX15_OP_FLAG_XYCONTINUE);
		uint32_t start_x = xycontinue ? m_lg1.m_x_curr_i : m_lg1.m_x_start_i;
		uint32_t start_y = xycontinue ? m_lg1.m_y_curr_i : m_lg1.m_y_start_i;
		uint32_t end_x = m_lg1.m_x_end_i;
		LOGMASKED(LOG_GFX, "LG1: Command %08x: Pattern draw from %d-%d at %d\n", m_lg1.m_command, start_x, end_x, start_y);
		for (uint32_t x = start_x; x <= end_x && x < (start_x + 32); x++)
		{
			if (BIT(m_lg1.m_z_pattern, 31 - (x - start_x)))
			{
				m_framebuffer[start_y*1024 + x] = m_lg1.m_color_red_i;
			}
			m_lg1.m_x_curr_i++;
		}

		if (m_lg1.m_command & REX15_OP_FLAG_BLOCK)
		{
			if (m_lg1.m_x_curr_i > m_lg1.m_x_end_i)
			{
				m_lg1.m_y_curr_i--;
				m_lg1.m_x_curr_i = m_lg1.m_x_start_i;
			}
		}
	}
	else
	{
		LOGMASKED(LOG_GFX_CMD | LOG_UNKNOWN, "%s: Unknown LG1 command: %08x\n", machine().describe_context(), m_lg1.m_command);
	}
}

WRITE32_MEMBER(indigo_state::entry_w)
{
	bool go = (offset >= REX15_PAGE1_GO/4) || (offset >= REX15_PAGE0_GO/4 && offset < REX15_PAGE1_SET/4);

	switch (offset)
	{
		case (REX15_PAGE0_SET+REX15_P0REG_COMMAND)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_COMMAND)/4:
			m_lg1.m_command = data;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 Command Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			if (go)
				do_rex_command();
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_XSTARTI)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_XSTARTI)/4:
			m_lg1.m_x_start_i = data;
			m_lg1.m_x_curr_i = m_lg1.m_x_start_i;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 XStartI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_YSTARTI)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_YSTARTI)/4:
			m_lg1.m_y_start_i = data;
			m_lg1.m_y_curr_i = m_lg1.m_y_start_i;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 YStartI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_XYMOVE)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_XYMOVE)/4:
			m_lg1.m_xy_move = data;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 XYMove Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			if (go)
				do_rex_command();
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_COLORREDI)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_COLORREDI)/4:
			m_lg1.m_color_red_i = data;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 ColorRedI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_COLORGREENI)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_COLORGREENI)/4:
			m_lg1.m_color_green_i = data;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 ColorGreenI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_COLORBLUEI)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_COLORBLUEI)/4:
			m_lg1.m_color_blue_i = data;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 ColorBlueI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_COLORBACK)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_COLORBACK)/4:
			m_lg1.m_color_back = data;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 ColorBlueI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_ZPATTERN)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_ZPATTERN)/4:
			m_lg1.m_z_pattern = data;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 ZPattern Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			if (go)
				do_rex_command();
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_XENDI)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_XENDI)/4:
			m_lg1.m_x_end_i = data;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 XEndI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			break;
		case (REX15_PAGE0_SET+REX15_P0REG_YENDI)/4:
		case (REX15_PAGE0_GO+REX15_P0REG_YENDI)/4:
			m_lg1.m_y_end_i = data;
			LOGMASKED(LOG_GFX, "%s: LG1 REX1.5 YEndI Write (%s) = %08x\n", machine().describe_context(), (offset & 0x200) ? "Go" : "Set", data);
			break;

		case (REX15_PAGE1_SET+REX15_P1REG_CFGSEL)/4:
		case (REX15_PAGE1_GO+REX15_P1REG_CFGSEL)/4:
			m_lg1.m_config_sel = data;
			switch (data)
			{
				case REX15_WRITE_ADDR:
					LOGMASKED(LOG_GFX, "%s: LG1 ConfigSel Write = %08x (Write Addr)\n", machine().describe_context(), data);
					break;
				case REX15_PALETTE_RAM:
					LOGMASKED(LOG_GFX, "%s: LG1 ConfigSel Write = %08x (Palette RAM)\n", machine().describe_context(), data);
					break;
				case REX15_CONTROL:
					LOGMASKED(LOG_GFX, "%s: LG1 ConfigSel Write = %08x (Control)\n", machine().describe_context(), data);
					break;
				case REX15_PIXEL_READ_MASK:
					LOGMASKED(LOG_GFX, "%s: LG1 ConfigSel Write = %08x (Pixel Read Mask)\n", machine().describe_context(), data);
					break;
				default:
					LOGMASKED(LOG_GFX, "%s: LG1 ConfigSel Write = %08x (Unknown)\n", machine().describe_context(), data);
					break;
			}
			break;
		case (REX15_PAGE1_SET+REX15_P1REG_CFGDATA)/4:
		case (REX15_PAGE1_GO+REX15_P1REG_CFGDATA)/4:
			if (go) // Ignore 'Go' writes for now, unsure what they do
				break;
			switch (m_lg1.m_config_sel)
			{
				case REX15_WRITE_ADDR:
					LOGMASKED(LOG_GFX, "%s: LG1 ConfigSel Write: Setting WriteAddr to %08x\n", machine().describe_context(), data);
					m_lg1.m_write_addr = data;
					if (m_lg1.m_control == 0xf)
					{
						LOGMASKED(LOG_GFX, "%s: LG1 about to set palette entry %02x\n", machine().describe_context(), (uint8_t)data);
						m_lg1.m_palette_idx = (uint8_t)data;
						m_lg1.m_palette_channel = 0;
					}
					break;
				case REX15_PALETTE_RAM:
					LOGMASKED(LOG_GFX, "%s: LG1 ConfigSel Write: Setting Palette RAM %02x entry %d to %02x\n", machine().describe_context(),
						m_lg1.m_palette_idx, m_lg1.m_palette_channel, (uint8_t)data);
					m_lg1.m_palette_entry[m_lg1.m_palette_channel++] = (uint8_t)data;
					if (m_lg1.m_palette_channel == 3)
					{
						m_palette->set_pen_color(m_lg1.m_palette_idx, m_lg1.m_palette_entry[0], m_lg1.m_palette_entry[1], m_lg1.m_palette_entry[2]);
					}
					break;
				case REX15_CONTROL:
					LOGMASKED(LOG_GFX, "%s: LG1 ConfigSel Write: Setting Control to %08x\n", machine().describe_context(), data);
					m_lg1.m_control = data;
					break;
				case REX15_PIXEL_READ_MASK:
					LOGMASKED(LOG_GFX, "%s: LG1 ConfigSel Write = %08x (Pixel Read Mask, entry %02x)\n", machine().describe_context(), data, m_lg1.m_palette_idx);
					m_lg1.m_pix_read_mask[m_lg1.m_palette_idx] = (uint8_t)data;
					break;
				default:
					LOGMASKED(LOG_GFX, "%s: LG1 Unknown ConfigData Write = %08x\n", machine().describe_context(), data);
					break;
			}
			break;
		default:
			LOGMASKED(LOG_GFX, "%s: Unknown LG1 Write: %08x = %08x & %08x\n", machine().describe_context(), 0x1f3f0000 + offset*4, data, mem_mask);
			break;
	}
}

uint32_t indigo_state::screen_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const rgb_t *pens = m_palette->palette()->entry_list_raw();

	for (int y = 0; y < 768; y++)
	{
		uint32_t *dst = &bitmap.pix32(y);
		uint8_t *src = &m_framebuffer[y*1024];
		for (int x = 0; x < 1024; x++)
		{
			*dst++ = pens[*src++];
		}
	}
	return 0;
}

void indigo_state::indigo_map(address_map &map)
{
	map(0x1f3f0000, 0x1f3fffff).rw(FUNC(indigo_state::entry_r), FUNC(indigo_state::entry_w));
	map(0x1fb80000, 0x1fb8ffff).rw(m_hpc, FUNC(hpc1_device::read), FUNC(hpc1_device::write));
	map(0x1fbd9000, 0x1fbd903f).rw(FUNC(indigo_state::int_r), FUNC(indigo_state::int_w));
	map(0x1fbe0000, 0x1fbfffff).rw(FUNC(indigo_state::dsp_ram_r), FUNC(indigo_state::dsp_ram_w)).share("dspram");
}

void indigo3k_state::mem_map(address_map &map)
{
	indigo_map(map);
	map(0x1fc00000, 0x1fc3ffff).rom().share("share10").region("user1", 0);
}

WRITE32_MEMBER(indigo4k_state::write_ram)
{
	// if banks 2 or 3 are enabled, kill it, we only want 128MB
	if (m_mem_ctrl->read(space, 0xc8/4, 0xffffffff) & 0x10001000)
	{
		// a random perturbation so the memory test fails
		data ^= 0xffffffff;
	}

	// if banks 0 or 1 have 2 membanks, also kill it, we only want 128MB
	if (m_mem_ctrl->read(space, 0xc0/4, 0xffffffff) & 0x40004000)
	{
		// a random perturbation so the memory test fails
		data ^= 0xffffffff;
	}

	COMBINE_DATA(&m_share1[offset & 0x03ffffff]);
}

void indigo4k_state::mem_map(address_map &map)
{
	indigo_map(map);
	map(0x00000000, 0x0007ffff).bankrw("bank1");
	map(0x08000000, 0x17ffffff).ram().share("share1").w(FUNC(indigo4k_state::write_ram));     /* 128 MB of main RAM */
	map(0x1fa00000, 0x1fa1ffff).rw(m_mem_ctrl, FUNC(sgi_mc_device::read), FUNC(sgi_mc_device::write));
	map(0x1fc00000, 0x1fc7ffff).rom().share("share5").region("user1", 0);
	map(0x20000000, 0x27ffffff).ram().share("share1").w(FUNC(indigo4k_state::write_ram));     /* 128 MB of main RAM */
}

static INPUT_PORTS_START(indigo)
INPUT_PORTS_END

void indigo_state::indigo_base(machine_config &config)
{
	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(1024, 768);
	screen.set_visarea(0, 1024-1, 0, 768-1);
	screen.set_screen_update(FUNC(indigo_state::screen_update));

	PALETTE(config, m_palette, 256);

	EEPROM_93C56_16BIT(config, m_eeprom);
}

void indigo3k_state::indigo3k(machine_config &config)
{
	indigo_base(config);

	R3000A(config, m_maincpu, 33.333_MHz_XTAL, 32768, 32768);
	downcast<r3000a_device &>(*m_maincpu).set_endianness(ENDIANNESS_BIG);
	m_maincpu->set_addrmap(AS_PROGRAM, &indigo3k_state::mem_map);

	SGI_HPC1(config, m_hpc, m_maincpu, m_eeprom);
}

void indigo4k_state::indigo4k(machine_config &config)
{
	indigo_base(config);

	mips3_device &cpu(R4000BE(config, m_maincpu, 50000000*2));
	cpu.set_icache_size(32768);
	cpu.set_dcache_size(32768);
	cpu.set_addrmap(AS_PROGRAM, &indigo4k_state::mem_map);

	SGI_MC(config, m_mem_ctrl, m_maincpu, m_eeprom);
	SGI_HPC1(config, m_hpc, m_maincpu, m_eeprom);
}

ROM_START( indigo3k )
	ROM_REGION32_BE( 0x40000, "user1", 0 )
	ROM_SYSTEM_BIOS( 0, "401-rev-c", "SGI Version 4.0.1 Rev C LG1/GR2, Jul 9, 1992" ) // dumped over serial connection from boot monitor and swapped
	ROMX_LOAD( "ip12prom.070-8088-xxx.u56", 0x000000, 0x040000, CRC(25ca912f) SHA1(94b3753d659bfe50b914445cef41290122f43880), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "401-rev-d", "SGI Version 4.0.1 Rev D LG1/GR2, Mar 24, 1992" ) // dumped with EPROM programmer
	ROMX_LOAD( "ip12prom.070-8088-002.u56", 0x000000, 0x040000, CRC(ea4329ef) SHA1(b7d67d0e30ae8836892f7170dd4757732a0a3fd6), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(1) )
ROM_END

ROM_START( indigo4k )
	ROM_REGION32_BE( 0x80000, "user1", 0 )
	ROMX_LOAD( "ip20prom.070-8116-004.bin", 0x000000, 0x080000, CRC(940d960e) SHA1(596aba530b53a147985ff3f6f853471ce48c866c), ROM_GROUPDWORD | ROM_REVERSE )
ROM_END

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT   CLASS           INIT        COMPANY                 FULLNAME                                          FLAGS
COMP( 1991, indigo3k, 0,      0,      indigo3k, indigo, indigo3k_state, empty_init, "Silicon Graphics Inc", "IRIS Indigo (R3000, 33MHz)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1993, indigo4k, 0,      0,      indigo4k, indigo, indigo4k_state, empty_init, "Silicon Graphics Inc", "IRIS Indigo (R4400, 150MHz, Ver. 4.0.5D Rev A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
