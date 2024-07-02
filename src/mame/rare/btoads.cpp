// license:BSD-3-Clause
// copyright-holders: Aaron Giles

/*************************************************************************

    BattleToads

    driver by Aaron Giles

**************************************************************************/

#include "emu.h"

#include "cpu/tms34010/tms34010.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/bsmt2000.h"
#include "video/tlc34076.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class btoads_state : public driver_device
{
public:
	btoads_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vram_fg(*this, "vram_fg%u", 0U, 0x200000U, ENDIANNESS_LITTLE),
		m_vram_fg_data(*this, "vram_fg_data"),
		m_vram_bg(*this, "vram_bg%u", 0U, 0x400000U, ENDIANNESS_LITTLE),
		m_sprite_scale(*this, "sprite_scale"),
		m_sprite_control(*this, "sprite_control"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_bsmt(*this, "bsmt"),
		m_tlc34076(*this, "tlc34076"),
		m_screen(*this, "screen") { }

	int main_to_sound_r();
	int sound_to_main_r();

	void btoads(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	// shared pointers
	memory_share_array_creator<uint8_t, 2> m_vram_fg;
	required_shared_ptr<uint32_t> m_vram_fg_data;
	memory_share_array_creator<uint16_t, 2> m_vram_bg;
	required_shared_ptr<uint32_t> m_sprite_scale;
	required_shared_ptr<uint32_t> m_sprite_control;

	// devices
	required_device<tms34020_device> m_maincpu;
	required_device<z80_device> m_audiocpu;
	required_device<bsmt2000_device> m_bsmt;
	required_device<tlc34076_device> m_tlc34076;
	required_device<screen_device> m_screen;

	// state
	uint8_t m_main_to_sound_data = 0;
	uint8_t m_main_to_sound_ready = 0;
	uint8_t m_sound_to_main_data = 0;
	uint8_t m_sound_to_main_ready = 0;
	uint8_t m_sound_int_state = 0;
	uint8_t *m_vram_fg_draw = nullptr;
	uint8_t *m_vram_fg_display = nullptr;
	int32_t m_xscroll[2]{};
	int32_t m_yscroll[2]{};
	uint8_t m_screen_control = 0;
	uint16_t m_sprite_source_offs = 0;
	uint8_t *m_sprite_dest_base = nullptr;
	uint16_t m_sprite_dest_offs = 0;
	uint16_t m_misc_control = 0;
	[[maybe_unused]] int m_xcount = 0;
	std::unique_ptr<uint8_t[]> m_nvram_data;
	emu_timer *m_delayed_sound_timer;
	emu_timer *m_audio_sync_timer;

	void nvram_w(offs_t offset, uint8_t data);
	uint8_t nvram_r(offs_t offset);
	void main_sound_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t main_sound_r();
	void sound_data_w(uint8_t data);
	uint8_t sound_data_r();
	uint8_t sound_ready_to_send_r();
	uint8_t sound_data_ready_r();
	void sound_int_state_w(uint8_t data);
	uint8_t bsmt_ready_r();
	void bsmt2000_port_w(offs_t offset, uint8_t data);
	TIMER_CALLBACK_MEMBER(delayed_sound);
	TIMER_CALLBACK_MEMBER(audio_sync) { }

	void misc_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void display_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <uint8_t Which> void scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <uint8_t Which> void vram_bg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <uint8_t Which> uint16_t vram_bg_r(offs_t offset);
	void vram_fg_display_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vram_fg_draw_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t vram_fg_display_r(offs_t offset);
	uint16_t vram_fg_draw_r(offs_t offset);
	void render_sprite_row(uint16_t *sprite_source, uint32_t address);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(scanline_update);

	void main_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);
};


#define BT_DEBUG 0



/*************************************
 *
 *  Video system start
 *
 *************************************/

void btoads_state::video_start()
{
	// initialize the swapped pointers
	m_vram_fg_draw = m_vram_fg[0];
	m_vram_fg_display = m_vram_fg[1];

	save_item(NAME(m_xscroll));
	save_item(NAME(m_yscroll));
	save_item(NAME(m_screen_control));

	save_item(NAME(m_sprite_source_offs));
	save_item(NAME(m_sprite_dest_offs));
	save_item(NAME(m_misc_control));
}



/*************************************
 *
 *  Control registers
 *
 *************************************/

void btoads_state::misc_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_misc_control);

	// bit 3 controls sound reset line
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (m_misc_control & 8) ? CLEAR_LINE : ASSERT_LINE);
}


void btoads_state::display_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		// allow multiple changes during display
		int const scanline = m_screen->vpos();
		if (scanline > 0)
			m_screen->update_partial(scanline - 1);

		// bit 15 controls which page is rendered and which page is displayed
		if (data & 0x8000)
		{
			m_vram_fg_draw = m_vram_fg[1];
			m_vram_fg_display = m_vram_fg[0];
		}
		else
		{
			m_vram_fg_draw = m_vram_fg[0];
			m_vram_fg_display = m_vram_fg[1];
		}

		// stash the remaining data for later
		m_screen_control = data >> 8;
	}
}



/*************************************
 *
 *  Scroll registers
 *
 *************************************/

template <uint8_t Which>
void btoads_state::scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// allow multiple changes during display
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());

	// upper bits are Y scroll, lower bits are X scroll
	if (ACCESSING_BITS_8_15)
		m_yscroll[Which] = data >> 8;
	if (ACCESSING_BITS_0_7)
		m_xscroll[Which] = data & 0xff;
}



/*************************************
 *
 *  Background video RAM
 *
 *************************************/

template <uint8_t Which>
void btoads_state::vram_bg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram_bg[Which][offset & 0x3fcff]);
}


template <uint8_t Which>
uint16_t btoads_state::vram_bg_r(offs_t offset)
{
	return m_vram_bg[Which][offset & 0x3fcff];
}



/*************************************
 *
 *  Foreground video RAM
 *
 *************************************/

void btoads_state::vram_fg_display_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_vram_fg_display[offset] = data;
}


void btoads_state::vram_fg_draw_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_vram_fg_draw[offset] = data;
}


uint16_t btoads_state::vram_fg_display_r(offs_t offset)
{
	return m_vram_fg_display[offset];
}


uint16_t btoads_state::vram_fg_draw_r(offs_t offset)
{
	return m_vram_fg_draw[offset];
}



/*************************************
 *
 *  Sprite rendering
 *
 *************************************/

void btoads_state::render_sprite_row(uint16_t *sprite_source, uint32_t address)
{
	int const flipxor = ((*m_sprite_control >> 10) & 1) ? 0xffff : 0x0000;
	int const width = (~*m_sprite_control & 0x1ff) + 2;
	int const color = (~*m_sprite_control >> 8) & 0xf0;
	int srcoffs = m_sprite_source_offs << 8;
	int const srcend = srcoffs + (width << 8);
	int const srcstep = 0x100 - (m_sprite_scale[0] & 0xffff);
	int const dststep = 0x100 - (m_sprite_scale[4] & 0xffff);
	int dstoffs = m_sprite_dest_offs << 8;

	if (!(m_misc_control & 0x10))
	{
		// non-shadow case
		for ( ; srcoffs < srcend; srcoffs += srcstep, dstoffs += dststep)
		{
			uint16_t src = sprite_source[(srcoffs >> 10) & 0x1ff];
			if (src)
			{
				src = (src >> (((srcoffs ^ flipxor) >> 6) & 0x0c)) & 0x0f;
				if (src)
					m_sprite_dest_base[(dstoffs >> 8) & 0x1ff] = src | color;
			}
		}
	}
	else
	{
		// shadow case
		for ( ; srcoffs < srcend; srcoffs += srcstep, dstoffs += dststep)
		{
			uint16_t src = sprite_source[(srcoffs >> 10) & 0x1ff];
			if (src)
			{
				src = (src >> (((srcoffs ^ flipxor) >> 6) & 0x0c)) & 0x0f;
				if (src)
					m_sprite_dest_base[(dstoffs >> 8) & 0x1ff] = color;
			}
		}
	}

	m_sprite_source_offs += width;
	m_sprite_dest_offs = dstoffs >> 8;
}



/*************************************
 *
 *  Shift register read/write
 *
 *************************************/

TMS340X0_TO_SHIFTREG_CB_MEMBER(btoads_state::to_shiftreg)
{
	address &= ~0x40000000;

	if (address >= 0xa0000000 && address <= 0xa3ffffff)
	{
		// reads from this first region are usual shift register reads
		memcpy(shiftreg, &m_vram_fg_display[(address & 0x3fffff) >> 4], 0x200);
	}
	else if (address >= 0xa4000000 && address <= 0xa7ffffff)
	{
		// reads from this region set the sprite destination address
		m_sprite_dest_base = &m_vram_fg_draw[(address & 0x3fc000) >> 4];
		m_sprite_dest_offs = (address & 0x003fff) >> 5;
	}
	else if (address >= 0xa8000000 && address <= 0xabffffff)
	{
		// reads from this region set the sprite source address
		const u32 *src = &m_vram_fg_data[(address & 0x7fc000) >> 5];
		u16 *dest = shiftreg;
		for (unsigned int i = 0; i != 0x100; i++)
		{
			*dest++ = *src;
			*dest++ = *src >> 16;
			src++;
		}
		m_sprite_source_offs = (address & 0x003fff) >> 3;
	}
	else
		logerror("%s: to_shiftreg(%08X)\n", machine().describe_context(), address);
}


TMS340X0_FROM_SHIFTREG_CB_MEMBER(btoads_state::from_shiftreg)
{
	address &= ~0x40000000;

	if (address >= 0xa0000000 && address <= 0xa3ffffff)
	{
		// writes to this first region are usual shift register writes
		memcpy(&m_vram_fg_display[(address & 0x3fc000) >> 4], shiftreg, 0x200);
	}
	else if (address >= 0xa4000000 && address <= 0xa7ffffff)
	{
		// writes to this region are ignored for our purposes
	}
	else if (address >= 0xa8000000 && address <= 0xabffffff)
	{
		// writes to this region copy standard data
		const u16 *src = shiftreg;
		u32 *dest = &m_vram_fg_data[(address & 0x7fc000) >> 5];
		for (unsigned int i = 0; i != 0x100; i++)
		{
			*dest++ = src[0] | (src[1] << 16);
			src += 2;
		}
	}
	else if (address >= 0xac000000 && address <= 0xafffffff)
	{
		// writes to this region render the current sprite data
		render_sprite_row(shiftreg, address);
	}
	else
		logerror("%s: from_shiftreg(%08X)\n", machine().describe_context(), address);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

TMS340X0_SCANLINE_RGB32_CB_MEMBER(btoads_state::scanline_update)
{
	uint32_t const fulladdr = ((params->rowaddr << 16) | params->coladdr) >> 4;
	uint16_t *bg0_base = &m_vram_bg[0][(fulladdr + (m_yscroll[0] << 10)) & 0x3fc00];
	uint16_t *bg1_base = &m_vram_bg[1][(fulladdr + (m_yscroll[1] << 10)) & 0x3fc00];
	uint8_t *spr_base = &m_vram_fg_display[fulladdr & 0x3fc00];
	uint32_t *const dst = &bitmap.pix(scanline);
	const pen_t *pens = m_tlc34076->pens();
	int coladdr = fulladdr & 0x3ff;

	// for each scanline, switch off the render mode
	switch (m_screen_control & 3)
	{
		// mode 0: used in ship level, snake boss, title screen (free play)
		/* priority is:
		    1. Sprite pixels with high bit clear
		    2. BG1 pixels with the high bit set
		    3. Sprites
		    4. BG1
		    5. BG0
		*/
		case 0:
			for (int x = params->heblnk; x < params->hsblnk; x += 2, coladdr++)
			{
				uint8_t const sprpix = spr_base[coladdr & 0xff];

				if (sprpix && !(sprpix & 0x80))
				{
					dst[x + 0] = pens[sprpix];
					dst[x + 1] = pens[sprpix];
				}
				else
				{
					uint16_t const bg0pix = bg0_base[(coladdr + m_xscroll[0]) & 0xff];
					uint16_t const bg1pix = bg1_base[(coladdr + m_xscroll[1]) & 0xff];

					if (bg1pix & 0x80)
						dst[x + 0] = pens[bg1pix & 0xff];
					else if (sprpix)
						dst[x + 0] = pens[sprpix];
					else if (bg1pix & 0xff)
						dst[x + 0] = pens[bg1pix & 0xff];
					else
						dst[x + 0] = pens[bg0pix & 0xff];

					if (bg1pix & 0x8000)
						dst[x + 1] = pens[bg1pix >> 8];
					else if (sprpix)
						dst[x + 1] = pens[sprpix];
					else if (bg1pix >> 8)
						dst[x + 1] = pens[bg1pix >> 8];
					else
						dst[x + 1] = pens[bg0pix >> 8];
				}
			}
			break;

		// mode 1: used in snow level, title screen (free play), top part of rolling ball level
		/* priority is:
		    1. Sprite pixels with high bit clear
		    2. BG0
		    3. BG1 pixels with high bit set
		    4. Sprites
		    5. BG1
		*/
		case 1:
			for (int x = params->heblnk; x < params->hsblnk; x += 2, coladdr++)
			{
				uint8_t const sprpix = spr_base[coladdr & 0xff];

				if (sprpix && !(sprpix & 0x80))
				{
					dst[x + 0] = pens[sprpix];
					dst[x + 1] = pens[sprpix];
				}
				else
				{
					uint16_t const bg0pix = bg0_base[(coladdr + m_xscroll[0]) & 0xff];
					uint16_t const bg1pix = bg1_base[(coladdr + m_xscroll[1]) & 0xff];

					if (bg0pix & 0xff)
						dst[x + 0] = pens[bg0pix & 0xff];
					else if (bg1pix & 0x80)
						dst[x + 0] = pens[bg1pix & 0xff];
					else if (sprpix)
						dst[x + 0] = pens[sprpix];
					else
						dst[x + 0] = pens[bg1pix & 0xff];

					if (bg0pix >> 8)
						dst[x + 1] = pens[bg0pix >> 8];
					else if (bg1pix & 0x8000)
						dst[x + 1] = pens[bg1pix >> 8];
					else if (sprpix)
						dst[x + 1] = pens[sprpix];
					else
						dst[x + 1] = pens[bg1pix >> 8];
				}
			}
			break;

		// mode 2: used in EOA screen, jetpack level, first level, high score screen
		/* priority is:
		    1. Sprites
		    2. BG1
		    3. BG0
		*/
		case 2:
			for (int x = params->heblnk; x < params->hsblnk; x += 2, coladdr++)
			{
				uint8_t const sprpix = spr_base[coladdr & 0xff];

				if (sprpix)
				{
					dst[x + 0] = pens[sprpix];
					dst[x + 1] = pens[sprpix];
				}
				else
				{
					uint16_t const bg0pix = bg0_base[(coladdr + m_xscroll[0]) & 0xff];
					uint16_t const bg1pix = bg1_base[(coladdr + m_xscroll[1]) & 0xff];

					if (bg1pix & 0xff)
						dst[x + 0] = pens[bg1pix & 0xff];
					else
						dst[x + 0] = pens[bg0pix & 0xff];

					if (bg1pix >> 8)
						dst[x + 1] = pens[bg1pix >> 8];
					else
						dst[x + 1] = pens[bg0pix >> 8];
				}
			}
			break;

		// mode 3: used in toilet level, toad intros, bottom of rolling ball level
		/* priority is:
		    1. BG1 pixels with the high bit set
		    2. Sprite pixels with the high bit set
		    3. BG1
		    4. Sprites
		    5. BG0
		*/
		case 3:
			for (int x = params->heblnk; x < params->hsblnk; x += 2, coladdr++)
			{
				uint16_t const bg0pix = bg0_base[(coladdr + m_xscroll[0]) & 0xff];
				uint16_t const bg1pix = bg1_base[(coladdr + m_xscroll[1]) & 0xff];
				uint8_t const sprpix = spr_base[coladdr & 0xff];

				if (bg1pix & 0x80)
					dst[x + 0] = pens[bg1pix & 0xff];
				else if (sprpix & 0x80)
					dst[x + 0] = pens[sprpix];
				else if (bg1pix & 0xff)
					dst[x + 0] = pens[bg1pix & 0xff];
				else if (sprpix)
					dst[x + 0] = pens[sprpix];
				else
					dst[x + 0] = pens[bg0pix & 0xff];

				if (bg1pix & 0x8000)
					dst[x + 1] = pens[bg1pix >> 8];
				else if (sprpix & 0x80)
					dst[x + 1] = pens[sprpix];
				else if (bg1pix >> 8)
					dst[x + 1] = pens[bg1pix >> 8];
				else if (sprpix)
					dst[x + 1] = pens[sprpix];
				else
					dst[x + 1] = pens[bg0pix >> 8];
			}
			break;
	}

	// debugging - dump the screen contents to a file
#if BT_DEBUG
	popmessage("screen_control = %02X", m_screen_control & 0x7f);

	if (machine().input().code_pressed(KEYCODE_X))
	{
		char name[10];
		FILE *f;

		while (machine().input().code_pressed(KEYCODE_X)) { }

		sprintf(name, "disp%d.log", m_xcount++);
		f = fopen(name, "w");
		fprintf(f, "screen_control = %04X\n\n", m_screen_control << 8);

		for (int i = 0; i < 3; i++)
		{
			uint16_t *base = (i == 0) ? (uint16_t *)m_vram_fg_display : (i == 1) ? m_vram_bg0 : m_vram_bg1;
			int xscr = (i == 0) ? 0 : (i == 1) ? m_xscroll[0] : m_xscroll[1];
			int yscr = (i == 0) ? 0 : (i == 1) ? m_yscroll[0] : m_yscroll[1];

			for (int y = 0; y < 224; y++)
			{
				uint32_t offs = ((y + yscr) & 0xff) * TOWORD(0x4000);
				for (int x = 0; x < 256; x++)
				{
					uint16_t pix = base[offs + ((x + xscr) & 0xff)];
					fprintf(f, "%02X%02X", pix & 0xff, pix >> 8);
					if (x % 16 == 15) fprintf(f, " ");
				}
				fprintf(f, "\n");
			}
			fprintf(f, "\n\n");
		}
		fclose(f);
	}

	logerror("---VBLANK---\n");
#endif
}


/*************************************
 *
 *  Machine init
 *
 *************************************/

void btoads_state::machine_start()
{
	m_nvram_data = std::make_unique<uint8_t[]>(0x2000);
	subdevice<nvram_device>("nvram")->set_base(m_nvram_data.get(), 0x2000);

	save_item(NAME(m_main_to_sound_data));
	save_item(NAME(m_main_to_sound_ready));
	save_item(NAME(m_sound_to_main_data));
	save_item(NAME(m_sound_to_main_ready));
	save_item(NAME(m_sound_int_state));
	save_pointer(NAME(m_nvram_data), 0x2000);

	m_delayed_sound_timer = timer_alloc(FUNC(btoads_state::delayed_sound), this);
	m_audio_sync_timer = timer_alloc(FUNC(btoads_state::audio_sync), this);
}



/*************************************
 *
 *  NVRAM
 *
 *************************************/

void btoads_state::nvram_w(offs_t offset, uint8_t data)
{
	m_nvram_data[offset] = data;
}


uint8_t btoads_state::nvram_r(offs_t offset)
{
	return m_nvram_data[offset];
}



/*************************************
 *
 *  Main -> sound CPU communication
 *
 *************************************/

TIMER_CALLBACK_MEMBER(btoads_state::delayed_sound)
{
	m_main_to_sound_data = param;
	m_main_to_sound_ready = 1;
	m_audiocpu->signal_interrupt_trigger();

	// use a timer to make long transfers faster
	m_audio_sync_timer->adjust(attotime::from_usec(50));
}

void btoads_state::main_sound_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_delayed_sound_timer->adjust(attotime::zero, data & 0xff);
}


uint16_t btoads_state::main_sound_r()
{
	m_sound_to_main_ready = 0;
	return m_sound_to_main_data;
}


int btoads_state::main_to_sound_r()
{
	return m_main_to_sound_ready;
}


int btoads_state::sound_to_main_r()
{
	return m_sound_to_main_ready;
}



/*************************************
 *
 *  Sound -> main CPU communication
 *
 *************************************/

void btoads_state::sound_data_w(uint8_t data)
{
	m_sound_to_main_data = data;
	m_sound_to_main_ready = 1;
}


uint8_t btoads_state::sound_data_r()
{
	m_main_to_sound_ready = 0;
	return m_main_to_sound_data;
}


uint8_t btoads_state::sound_ready_to_send_r()
{
	return m_sound_to_main_ready ? 0x00 : 0x80;
}


uint8_t btoads_state::sound_data_ready_r()
{
	if (m_audiocpu->pc() == 0xd50 && !m_main_to_sound_ready)
		m_audiocpu->spin_until_interrupt();
	return m_main_to_sound_ready ? 0x00 : 0x80;
}



/*************************************
 *
 *  Sound CPU interrupt generation
 *
 *************************************/

void btoads_state::sound_int_state_w(uint8_t data)
{
	// top bit controls BSMT2000 reset
	if (!(m_sound_int_state & 0x80) && (data & 0x80))
		m_bsmt->reset();

	// also clears interrupts
	m_audiocpu->set_input_line(0, CLEAR_LINE);
	m_sound_int_state = data;
}



/*************************************
 *
 *  Sound CPU BSMT2000 communication
 *
 *************************************/

uint8_t btoads_state::bsmt_ready_r()
{
	return m_bsmt->read_status() << 7;
}


void btoads_state::bsmt2000_port_w(offs_t offset, uint8_t data)
{
	m_bsmt->write_reg(offset >> 8);
	m_bsmt->write_data(((offset & 0xff) << 8) | data);
}



/*************************************
 *
 *  Main CPU memory map
 *
 *************************************/

void btoads_state::main_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram();
	map(0x20000000, 0x2000007f).portr("P1");
	map(0x20000080, 0x200000ff).portr("P2");
	map(0x20000100, 0x2000017f).portr("P3");
	map(0x20000180, 0x200001ff).portr("UNK");
	map(0x20000200, 0x2000027f).portr("SPECIAL");
	map(0x20000280, 0x200002ff).portr("SW1");
	map(0x20000000, 0x200000ff).writeonly().share(m_sprite_scale);
	map(0x20000100, 0x2000017f).writeonly().share(m_sprite_control);
	map(0x20000180, 0x200001ff).w(FUNC(btoads_state::display_control_w));
	map(0x20000200, 0x2000027f).w(FUNC(btoads_state::scroll_w<0>));
	map(0x20000280, 0x200002ff).w(FUNC(btoads_state::scroll_w<1>));
	map(0x20000300, 0x2000037f).rw(m_tlc34076, FUNC(tlc34076_device::read), FUNC(tlc34076_device::write)).umask32(0x000000ff);
	map(0x20000380, 0x200003ff).rw(FUNC(btoads_state::main_sound_r), FUNC(btoads_state::main_sound_w));
	map(0x20000400, 0x2000047f).w(FUNC(btoads_state::misc_control_w));
	map(0x40000000, 0x4000001f).nopw();    // watchdog?
	map(0x60000000, 0x6003ffff).rw(FUNC(btoads_state::nvram_r), FUNC(btoads_state::nvram_w)).umask32(0x000000ff);
	map(0xa0000000, 0xa03fffff).rw(FUNC(btoads_state::vram_fg_display_r), FUNC(btoads_state::vram_fg_display_w));
	map(0xa4000000, 0xa43fffff).rw(FUNC(btoads_state::vram_fg_draw_r), FUNC(btoads_state::vram_fg_draw_w));
	map(0xa8000000, 0xa87fffff).ram().share("vram_fg_data");
	map(0xa8800000, 0xa8ffffff).nopw();
	map(0xb0000000, 0xb03fffff).rw(FUNC(btoads_state::vram_bg_r<0>), FUNC(btoads_state::vram_bg_w<0>));
	map(0xb4000000, 0xb43fffff).rw(FUNC(btoads_state::vram_bg_r<1>), FUNC(btoads_state::vram_bg_w<1>));
	map(0xfc000000, 0xffffffff).rom().region("maincpu", 0);
}



/*************************************
 *
 *  Sound CPU memory map
 *
 *************************************/

void btoads_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xffff).ram();
}

void btoads_state::sound_io_map(address_map &map)
{
	map(0x0000, 0x7fff).w(FUNC(btoads_state::bsmt2000_port_w));
	map(0x8000, 0x8000).rw(FUNC(btoads_state::sound_data_r), FUNC(btoads_state::sound_data_w));
	map(0x8002, 0x8002).w(FUNC(btoads_state::sound_int_state_w));
	map(0x8004, 0x8004).r(FUNC(btoads_state::sound_data_ready_r));
	map(0x8005, 0x8005).r(FUNC(btoads_state::sound_ready_to_send_r));
	map(0x8006, 0x8006).r(FUNC(btoads_state::bsmt_ready_r));
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( btoads )
	PORT_START("P1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNK")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SPECIAL")
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(btoads_state, sound_to_main_r)
	PORT_SERVICE_NO_TOGGLE( 0x0002, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(btoads_state, main_to_sound_r)
	PORT_BIT( 0xffffff7c, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds )) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Stereo ))      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0004, 0x0000, "Common Coin Mech")     PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0008, 0x0008, "Three Players")        PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Free_Play ))   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0020, 0x0020, "Blood Free Mode")      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0040, 0x0040, "Credit Retention")     PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPUNKNOWN_DIPLOC(0x0080, 0x0080, "SW1:8")
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void btoads_state::btoads(machine_config &config)
{
	constexpr XTAL CPU_CLOCK = XTAL(64'000'000);
	constexpr XTAL VIDEO_CLOCK = XTAL(20'000'000);
	constexpr XTAL SOUND_CLOCK = XTAL(24'000'000);

	TMS34020(config, m_maincpu, CPU_CLOCK / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &btoads_state::main_map);
	m_maincpu->set_halt_on_reset(false);
	m_maincpu->set_pixel_clock(VIDEO_CLOCK / 2);
	m_maincpu->set_pixels_per_clock(1);
	m_maincpu->set_scanline_rgb32_callback(FUNC(btoads_state::scanline_update));
	m_maincpu->set_shiftreg_in_callback(FUNC(btoads_state::to_shiftreg));
	m_maincpu->set_shiftreg_out_callback(FUNC(btoads_state::from_shiftreg));

	Z80(config, m_audiocpu, SOUND_CLOCK / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &btoads_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &btoads_state::sound_io_map);
	m_audiocpu->set_periodic_int(FUNC(btoads_state::irq0_line_assert), attotime::from_hz(SOUND_CLOCK / 4 / 32768));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	// video hardware
	TLC34076(config, m_tlc34076, tlc34076_device::TLC34076_6_BIT);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(VIDEO_CLOCK / 2, 640, 0, 512, 257, 0, 224);
	m_screen->set_screen_update("maincpu", FUNC(tms34020_device::tms340x0_rgb32));

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	BSMT2000(config, m_bsmt, SOUND_CLOCK);
	m_bsmt->add_route(0, "lspeaker", 1.0);
	m_bsmt->add_route(1, "rspeaker", 1.0);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( btoads )
	ROM_REGION32_LE( 0x800000, "maincpu", 0 ) // M27C322 ROMs
	ROM_LOAD32_WORD( "btc0-p0.u120", 0x000000, 0x400000, CRC(0dfd1e35) SHA1(733a0a4235bebd598c600f187ed2628f28cf9bd0) )
	ROM_LOAD32_WORD( "btc0-p1.u121", 0x000002, 0x400000, CRC(df7487e1) SHA1(67151b900767bb2653b5261a98c81ff8827222c3) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // M27C256B ROM
	ROM_LOAD( "bt.u102", 0x0000, 0x8000, CRC(a90b911a) SHA1(6ec25161e68df1c9870d48cc2b1f85cd1a49aba9) )

	ROM_REGION( 0x1000000, "bsmt", 0 ) // M27C160 ROM
	ROM_LOAD( "btc0-s.u109", 0x00000, 0x200000, CRC(d9612ddb) SHA1(f186dfb013e81abf81ba8ac5dc7eb731c1ad82b6) )

	ROM_REGION( 0x080a, "plds", 0 )
	ROM_LOAD( "u10.bin",  0x0000, 0x0157, CRC(b1144178) SHA1(15fb047adee4125e9fcf04171e0a502655e0a3d8) ) // GAL20V8A-15LP Located at U10.
	ROM_LOAD( "u11.bin",  0x0000, 0x0157, CRC(7c6beb96) SHA1(2f19d21889dd765b344ad7d257ea7c244baebec2) ) // GAL20V8A-15LP Located at U11.
	ROM_LOAD( "u57.bin",  0x0000, 0x0157, CRC(be355a56) SHA1(975238bb1ea8fef14458d6f264a82aa77ecf865d) ) // GAL20V8A-15LP Located at U57.
	ROM_LOAD( "u58.bin",  0x0000, 0x0157, CRC(41ed339c) SHA1(5853c805a902e6d12c979958d878d1cefd6908cc) ) // GAL20V8A-15LP Located at U58.
	ROM_LOAD( "u90.bin",  0x0000, 0x0157, CRC(a0d0c3f1) SHA1(47464c2ef9fadbba933df27767f377e0c29158aa) ) // GAL20V8A-15LP Located at U90.
	ROM_LOAD( "u144.bin", 0x0000, 0x0157, CRC(8597017f) SHA1(003d7b5de57e48f831ab211e2783fff338ce2f32) ) // GAL20V8A-15LP Located at U144.
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1994, btoads, 0, btoads, btoads, btoads_state, empty_init, ROT0, "Rare / Electronic Arts", "Battletoads", MACHINE_SUPPORTS_SAVE )
