// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    drivers/macquadra700.cpp
    Mac Quadra 700 emulation.  (900/950 are IOP-based and closer to the IIfx)

    By R. Belmont

****************************************************************************/

#include "emu.h"

#include "macadb.h"
#include "macrtc.h"
#include "mactoolbox.h"

#include "bus/nscsi/devices.h"
#include "bus/nubus/cards.h"
#include "bus/nubus/nubus.h"
#include "cpu/m68000/m68000.h"
#include "machine/6522via.h"
#include "machine/applefdintf.h"
#include "machine/dp83932c.h"
#include "machine/ncr5390.h"
#include "machine/nscsi_bus.h"
#include "machine/ram.h"
#include "machine/swim1.h"
#include "machine/timer.h"
#include "machine/z80scc.h"
#include "sound/asc.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/ap_dsk35.h"

#define C32M 31.3344_MHz_XTAL
#define C15M (C32M/2)
#define C7M (C32M/4)


namespace {

class macquadra_state : public driver_device
{
public:
	macquadra_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via1(*this, "via1"),
		m_via2(*this, "via2"),
		m_macadb(*this, "macadb"),
		m_ram(*this, RAM_TAG),
		m_swim(*this, "fdc"),
		m_floppy(*this, "fdc:%d", 0U),
		m_rtc(*this,"rtc"),
		m_scsibus1(*this, "scsi1"),
		m_ncr1(*this, "scsi1:7:ncr5394"),
		m_sonic(*this, "sonic"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_easc(*this, "easc"),
		m_scc(*this, "scc"),
		m_vram(*this, "vram"),
		m_cur_floppy(nullptr),
		m_hdsel(0)
	{
	}

	void macqd700(machine_config &config);
	void quadra700_map(address_map &map);

	void init_macqd700();

private:
	required_device<m68040_device> m_maincpu;
	required_device<via6522_device> m_via1, m_via2;
	optional_device<macadb_device> m_macadb;
	required_device<ram_device> m_ram;
	required_device<applefdintf_device> m_swim;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<rtc3430042_device> m_rtc;
	required_device<nscsi_bus_device> m_scsibus1;
	required_device<ncr53cf94_device> m_ncr1;
	required_device<dp83932c_device> m_sonic;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<asc_device> m_easc;
	required_device<z80scc_device> m_scc;
	required_shared_ptr<uint32_t> m_vram;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void video_reset() override;

	uint32_t screen_update_dafb(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t dafb_r(offs_t offset, uint32_t mem_mask = ~0);
	void dafb_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t dafb_dac_r(offs_t offset, uint32_t mem_mask = ~0);
	void dafb_dac_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void dafb_recalc_ints();

	TIMER_CALLBACK_MEMBER(dafb_vbl_tick);
	TIMER_CALLBACK_MEMBER(dafb_cursor_tick);

	u32 *m_ram_ptr = nullptr, *m_rom_ptr = nullptr;
	u32 m_ram_mask = 0, m_ram_size = 0, m_rom_size = 0;

	emu_timer *m_vbl_timer = nullptr, *m_cursor_timer = nullptr, *m_6015_timer = nullptr;

	uint16_t m_cursor_line = 0;
	uint16_t m_dafb_int_status = 0;
	int m_dafb_scsi1_drq = 0, m_dafb_scsi2_drq = 0;
	uint8_t m_dafb_mode = 0;
	uint32_t m_dafb_base = 0, m_dafb_stride = 0;
	uint32_t m_dafb_colors[3]{}, m_dafb_count = 0, m_dafb_clutoffs = 0, m_dafb_montype = 0, m_dafb_vbltime = 0;
	uint32_t m_dafb_palette[256]{};

	DECLARE_WRITE_LINE_MEMBER(nubus_irq_9_w);
	DECLARE_WRITE_LINE_MEMBER(nubus_irq_a_w);
	DECLARE_WRITE_LINE_MEMBER(nubus_irq_b_w);
	DECLARE_WRITE_LINE_MEMBER(nubus_irq_c_w);
	DECLARE_WRITE_LINE_MEMBER(nubus_irq_d_w);
	DECLARE_WRITE_LINE_MEMBER(nubus_irq_e_w);
	void nubus_slot_interrupt(uint8_t slot, uint32_t state);
	int m_via2_ca1_hack = 0, m_nubus_irq_state = 0;

	WRITE_LINE_MEMBER(adb_irq_w) { m_adb_irq_pending = state; }
	int m_adb_irq_pending = 0;

	DECLARE_WRITE_LINE_MEMBER(irq_539x_1_w);
	[[maybe_unused]] DECLARE_WRITE_LINE_MEMBER(irq_539x_2_w);
	DECLARE_WRITE_LINE_MEMBER(drq_539x_1_w);
	[[maybe_unused]] DECLARE_WRITE_LINE_MEMBER(drq_539x_2_w);

	floppy_image_device *m_cur_floppy = nullptr;
	int m_hdsel = 0;

	uint16_t mac_via_r(offs_t offset);
	void mac_via_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t mac_via2_r(offs_t offset);
	void mac_via2_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t mac_via_in_a();
	uint8_t mac_via_in_b();
	void mac_via_out_a(uint8_t data);
	void mac_via_out_b(uint8_t data);
	uint8_t mac_via2_in_a();
	uint8_t mac_via2_in_b();
	void mac_via2_out_a(uint8_t data);
	void mac_via2_out_b(uint8_t data);
	void mac_via_sync();
	void field_interrupts();
	DECLARE_WRITE_LINE_MEMBER(mac_via_irq);
	DECLARE_WRITE_LINE_MEMBER(mac_via2_irq);
	TIMER_CALLBACK_MEMBER(mac_6015_tick);
	WRITE_LINE_MEMBER(via_cb2_w) { m_macadb->adb_data_w(state); }
	int m_via_interrupt = 0, m_via2_interrupt = 0, m_scc_interrupt = 0, m_last_taken_interrupt = 0;

	uint32_t rom_switch_r(offs_t offset);
	bool m_overlay = 0;

	uint16_t mac_scc_r(offs_t offset)
	{
		mac_via_sync();
		uint16_t result = m_scc->dc_ab_r(offset);
		return (result << 8) | result;
	}
	void mac_scc_2_w(offs_t offset, uint16_t data) { mac_via_sync(); m_scc->dc_ab_w(offset, data >> 8); }

	void phases_w(uint8_t phases);
	void devsel_w(uint8_t devsel);

	uint16_t swim_r(offs_t offset, u16 mem_mask)
	{
		if (!machine().side_effects_disabled())
		{
			m_maincpu->adjust_icount(-5);
		}

		u16 result = m_swim->read((offset >> 8) & 0xf);
		return result << 8;
	}
	void swim_w(offs_t offset, u16 data, u16 mem_mask)
	{
		if (ACCESSING_BITS_0_7)
			m_swim->write((offset >> 8) & 0xf, data & 0xff);
		else
			m_swim->write((offset >> 8) & 0xf, data>>8);
	}

	uint8_t mac_5396_r(offs_t offset);
	void mac_5396_w(offs_t offset, uint8_t data);
};

void macquadra_state::field_interrupts()
{
	int take_interrupt = -1;

	if (m_scc_interrupt)
	{
		take_interrupt = 4;
	}
	else if (m_via2_interrupt)
	{
		take_interrupt = 2;
	}
	else if (m_via_interrupt)
	{
		take_interrupt = 1;
	}

	if (m_last_taken_interrupt > -1)
	{
		m_maincpu->set_input_line(m_last_taken_interrupt, CLEAR_LINE);
		m_last_taken_interrupt = -1;
	}

	if (take_interrupt > -1)
	{
		m_maincpu->set_input_line(take_interrupt, ASSERT_LINE);
		m_last_taken_interrupt = take_interrupt;
	}
}

void macquadra_state::machine_start()
{
	m_ram_ptr = (u32*)m_ram->pointer();
	m_ram_size = m_ram->size()>>1;
	m_ram_mask = m_ram_size - 1;
	m_rom_ptr = (u32*)memregion("bootrom")->base();
	m_rom_size = memregion("bootrom")->bytes();
	m_via_interrupt = m_via2_interrupt = m_scc_interrupt = 0;
	m_last_taken_interrupt = -1;

	m_6015_timer = timer_alloc(FUNC(macquadra_state::mac_6015_tick), this);
	m_6015_timer->adjust(attotime::never);

	save_item(NAME(m_cursor_line));
	save_item(NAME(m_dafb_int_status));
	save_item(NAME(m_dafb_scsi1_drq));
	save_item(NAME(m_dafb_scsi2_drq));
	save_item(NAME(m_dafb_mode));
	save_item(NAME(m_dafb_base));
	save_item(NAME(m_dafb_stride));
	save_item(NAME(m_dafb_colors));
	save_item(NAME(m_dafb_count));
	save_item(NAME(m_dafb_clutoffs));
	save_item(NAME(m_dafb_montype));
	save_item(NAME(m_dafb_vbltime));
	save_item(NAME(m_dafb_palette));
	save_item(NAME(m_via2_ca1_hack));
	save_item(NAME(m_nubus_irq_state));
	save_item(NAME(m_adb_irq_pending));
	save_item(NAME(m_hdsel));
	save_item(NAME(m_via_interrupt));
	save_item(NAME(m_via2_interrupt));
	save_item(NAME(m_scc_interrupt));
	save_item(NAME(m_last_taken_interrupt));
	save_item(NAME(m_overlay));
}

void macquadra_state::machine_reset()
{
	m_nubus_irq_state = 0xff;
	m_via2_ca1_hack = 1;
	m_via2->write_ca1(1);
	m_via2->write_cb1(1);
	m_overlay = true;
	m_via_interrupt = m_via2_interrupt = m_scc_interrupt = 0;
	m_last_taken_interrupt = -1;

	// put ROM mirror at 0
	address_space& space = m_maincpu->space(AS_PROGRAM);
	const u32 memory_size = std::min((u32)0x3fffff, m_rom_size);
	const u32 memory_end = memory_size - 1;
	offs_t memory_mirror = memory_end & ~(memory_size - 1);

	space.unmap_write(0x00000000, memory_end);
	space.install_rom(0x00000000, memory_end & ~memory_mirror, memory_mirror, m_rom_ptr);

	// start 60.15 Hz timer
	m_6015_timer->adjust(attotime::from_hz(60.15), 0, attotime::from_hz(60.15));
}

void macquadra_state::init_macqd700()
{
}

void macquadra_state::nubus_slot_interrupt(uint8_t slot, uint32_t state)
{
	static const uint8_t masks[8] = { 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 };
	uint8_t mask = 0xff;

	slot -= 9;

	if (state)
	{
		m_nubus_irq_state &= ~masks[slot];
	}
	else
	{
		m_nubus_irq_state |= masks[slot];
	}

	if ((m_nubus_irq_state & mask) != mask)
	{
		// HACK: sometimes we miss an ack (possible misbehavior in the VIA?)
		if (m_via2_ca1_hack == 0)
		{
			m_via2->write_ca1(1);
		}
		m_via2_ca1_hack = 0;
		m_via2->write_ca1(0);
	}
	else
	{
		m_via2_ca1_hack = 1;
		m_via2->write_ca1(1);
	}
}

WRITE_LINE_MEMBER(macquadra_state::nubus_irq_9_w) { nubus_slot_interrupt(9, state); }
WRITE_LINE_MEMBER(macquadra_state::nubus_irq_a_w) { nubus_slot_interrupt(0xa, state); }
WRITE_LINE_MEMBER(macquadra_state::nubus_irq_b_w) { nubus_slot_interrupt(0xb, state); }
WRITE_LINE_MEMBER(macquadra_state::nubus_irq_c_w) { nubus_slot_interrupt(0xc, state); }
WRITE_LINE_MEMBER(macquadra_state::nubus_irq_d_w) { nubus_slot_interrupt(0xd, state); }
WRITE_LINE_MEMBER(macquadra_state::nubus_irq_e_w) { nubus_slot_interrupt(0xe, state); }

// DAFB: video for Quadra 700/900

void macquadra_state::dafb_recalc_ints()
{
	if (m_dafb_int_status != 0)
	{
		nubus_slot_interrupt(0xf, ASSERT_LINE);
	}
	else
	{
		nubus_slot_interrupt(0xf, CLEAR_LINE);
	}
}

TIMER_CALLBACK_MEMBER(macquadra_state::dafb_vbl_tick)
{
	m_dafb_int_status |= 1;
	dafb_recalc_ints();

	m_vbl_timer->adjust(m_screen->time_until_pos(480, 0), 0);
}

TIMER_CALLBACK_MEMBER(macquadra_state::dafb_cursor_tick)
{
	m_dafb_int_status |= 4;
	dafb_recalc_ints();

	m_cursor_timer->adjust(m_screen->time_until_pos(m_cursor_line, 0), 0);
}

void macquadra_state::video_start() // DAFB
{
	m_vbl_timer = timer_alloc(FUNC(macquadra_state::dafb_vbl_tick), this);
	m_cursor_timer = timer_alloc(FUNC(macquadra_state::dafb_cursor_tick), this);

	m_vbl_timer->adjust(attotime::never);
	m_cursor_timer->adjust(attotime::never);
}

void macquadra_state::video_reset() // DAFB
{
	m_dafb_count = 0;
	m_dafb_clutoffs = 0;
	m_dafb_montype = 6;
	m_dafb_vbltime = 0;
	m_dafb_int_status = 0;
	m_dafb_mode = 0;
	m_dafb_base = 0x1000;
	m_dafb_stride = 256*4;

	memset(m_dafb_palette, 0, sizeof(m_dafb_palette));
}

uint32_t macquadra_state::dafb_r(offs_t offset, uint32_t mem_mask)
{
//  if (offset != 0x108/4) printf("DAFB: Read @ %x (mask %x PC=%x)\n", offset*4, mem_mask, m_maincpu->pc());

	switch (offset<<2)
	{
		case 0x1c:  // inverse of monitor sense
			return 7;   // 21" color 2-page

		case 0x24: // SCSI 539x #1 status
			return m_dafb_scsi1_drq<<9;

		case 0x28: // SCSI 539x #2 status
			return m_dafb_scsi2_drq<<9;

		case 0x108: // IRQ/VBL status
			return m_dafb_int_status;

		case 0x10c: // clear cursor scanline int
			m_dafb_int_status &= ~4;
			dafb_recalc_ints();
			break;

		case 0x114: // clear VBL int
			m_dafb_int_status &= ~1;
			dafb_recalc_ints();
			break;
	}
	return 0;
}

void macquadra_state::dafb_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
//  if (offset != 0x10c/4) printf("DAFB: Write %08x @ %x (mask %x PC=%x)\n", data, offset*4, mem_mask, m_maincpu->pc());

	switch (offset<<2)
	{
		case 0: // bits 20-9 of base
			m_dafb_base &= 0x1ff;
			m_dafb_base |= (data & 0xffff) << 9;
//          printf("DAFB baseH: %x\n", m_dafb_base);
			break;

		case 4: // bits 8-5 of base
			m_dafb_base &= ~0x1ff;
			m_dafb_base |= (data & 0xf) << 5;
//          printf("DAFB baseL: %x\n", m_dafb_base);
			break;

		case 8:
			m_dafb_stride = data<<2;    // stride in DWORDs
//          printf("DAFB stride: %x %x\n", m_dafb_stride, data);
			break;

		case 0x104:
			if (data & 1)   // VBL enable
			{
				m_vbl_timer->adjust(m_screen->time_until_pos(480, 0), 0);
			}
			else
			{
				m_vbl_timer->adjust(attotime::never);
				m_dafb_int_status &= ~1;
				dafb_recalc_ints();
			}

			if (data & 2)   // aux scanline interrupt enable
			{
				fatalerror("DAFB: Aux scanline interrupt enable not supported!\n");
			}

			if (data & 4)   // cursor scanline interrupt enable
			{
				m_cursor_timer->adjust(m_screen->time_until_pos(m_cursor_line, 0), 0);
			}
			else
			{
				m_cursor_timer->adjust(attotime::never);
				m_dafb_int_status &= ~4;
				dafb_recalc_ints();
			}
			break;

		case 0x10c: // clear cursor scanline int
			m_dafb_int_status &= ~4;
			dafb_recalc_ints();
			break;

		case 0x114: // clear VBL int
			m_dafb_int_status &= ~1;
			dafb_recalc_ints();
			break;
	}
}

uint32_t macquadra_state::dafb_dac_r(offs_t offset, uint32_t mem_mask)
{
//  printf("DAFB: Read DAC @ %x (mask %x PC=%x)\n", offset*4, mem_mask, m_maincpu->pc());
	return 0;
}

void macquadra_state::dafb_dac_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
//  if ((offset > 0) && (offset != 0x10/4)) printf("DAFB: Write %08x to DAC @ %x (mask %x PC=%x)\n", data, offset*4, mem_mask, m_maincpu->pc());

	switch (offset<<2)
	{
		case 0:
			m_dafb_clutoffs = data & 0xff;
			m_dafb_count = 0;
			break;

		case 0x10:
			m_dafb_colors[m_dafb_count++] = data&0xff;

			if (m_dafb_count == 3)
			{
				m_palette->set_pen_color(m_dafb_clutoffs, rgb_t(m_dafb_colors[0], m_dafb_colors[1], m_dafb_colors[2]));
				m_dafb_palette[m_dafb_clutoffs] = rgb_t(m_dafb_colors[0], m_dafb_colors[1], m_dafb_colors[2]);
				m_dafb_clutoffs++;
				m_dafb_count = 0;
			}
			break;

		case 0x20:
			switch (data & 0x9f)
			{
				case 0x80:
					m_dafb_mode = 0;    // 1bpp
					break;

				case 0x88:
					m_dafb_mode = 1;    // 2bpp
					break;

				case 0x90:
					m_dafb_mode = 2;    // 4bpp
					break;

				case 0x98:
					m_dafb_mode = 3;    // 8bpp
					break;

				case 0x9c:
					m_dafb_mode = 4;    // 24bpp
					break;
			}
			break;
	}
}

uint32_t macquadra_state::screen_update_dafb(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = util::big_endian_cast<uint8_t const>(m_vram.target()) + m_dafb_base;

	switch (m_dafb_mode)
	{
		case 0: // 1bpp
		{
			for (int y = 0; y < 870; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 1152/8; x++)
				{
					uint8_t const pixels = vram8[(y * m_dafb_stride) + x];

					*scanline++ = m_dafb_palette[(pixels>>7)&1];
					*scanline++ = m_dafb_palette[(pixels>>6)&1];
					*scanline++ = m_dafb_palette[(pixels>>5)&1];
					*scanline++ = m_dafb_palette[(pixels>>4)&1];
					*scanline++ = m_dafb_palette[(pixels>>3)&1];
					*scanline++ = m_dafb_palette[(pixels>>2)&1];
					*scanline++ = m_dafb_palette[(pixels>>1)&1];
					*scanline++ = m_dafb_palette[(pixels&1)];
				}
			}
		}
		break;

		case 1: // 2bpp
		{
			for (int y = 0; y < 870; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 1152/4; x++)
				{
					uint8_t const pixels = vram8[(y * m_dafb_stride) + x];

					*scanline++ = m_dafb_palette[((pixels>>6)&3)];
					*scanline++ = m_dafb_palette[((pixels>>4)&3)];
					*scanline++ = m_dafb_palette[((pixels>>2)&3)];
					*scanline++ = m_dafb_palette[(pixels&3)];
				}
			}
		}
		break;

		case 2: // 4bpp
		{
			for (int y = 0; y < 870; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 1152/2; x++)
				{
					uint8_t const pixels = vram8[(y * m_dafb_stride) + x];

					*scanline++ = m_dafb_palette[(pixels>>4)];
					*scanline++ = m_dafb_palette[(pixels&0xf)];
				}
			}
		}
		break;

		case 3: // 8bpp
		{
			for (int y = 0; y < 870; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x < 1152; x++)
				{
					uint8_t const pixels = vram8[(y * m_dafb_stride) + x];
					*scanline++ = m_dafb_palette[pixels];
				}
			}
		}
		break;

		case 4: // 24 bpp
			for (int y = 0; y < 480; y++)
			{
				uint32_t *scanline = &bitmap.pix(y);
				uint32_t const *base = &m_vram[(y * (m_dafb_stride/4)) + (m_dafb_base/4)];
				for (int x = 0; x < 640; x++)
				{
					*scanline++ = *base++;
				}
			}
			break;
	}

	return 0;
}

WRITE_LINE_MEMBER(macquadra_state::drq_539x_1_w)
{
	m_dafb_scsi1_drq = state;
}

WRITE_LINE_MEMBER(macquadra_state::drq_539x_2_w)
{
	m_dafb_scsi2_drq = state;
}

WRITE_LINE_MEMBER(macquadra_state::irq_539x_1_w)
{
	if (state)  // make sure a CB1 transition occurs
	{
		m_via2->write_cb2(0);
		m_via2->write_cb2(1);
	}
}

WRITE_LINE_MEMBER(macquadra_state::irq_539x_2_w)
{
}

uint16_t macquadra_state::mac_via_r(offs_t offset)
{
	uint16_t data;

	offset >>= 8;
	offset &= 0x0f;

	if (!machine().side_effects_disabled())
		mac_via_sync();

	data = m_via1->read(offset);

	return (data & 0xff) | (data << 8);
}

void macquadra_state::mac_via_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	mac_via_sync();

	if (ACCESSING_BITS_0_7)
		m_via1->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(offset, (data >> 8) & 0xff);
}

WRITE_LINE_MEMBER(macquadra_state::mac_via_irq)
{
	m_via_interrupt = state;
	field_interrupts();
}

WRITE_LINE_MEMBER(macquadra_state::mac_via2_irq)
{
	m_via2_interrupt = state;
	field_interrupts();
}

uint16_t macquadra_state::mac_via2_r(offs_t offset)
{
	int data;

	offset >>= 8;
	offset &= 0x0f;

	if (!machine().side_effects_disabled())
		mac_via_sync();

	data = m_via2->read(offset);
	return (data & 0xff) | (data << 8);
}

void macquadra_state::mac_via2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	mac_via_sync();

	if (ACCESSING_BITS_0_7)
		m_via2->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via2->write(offset, (data >> 8) & 0xff);
}

void macquadra_state::mac_via_sync()
{
	// The via runs at 783.36KHz while the main cpu runs at 15MHz or
	// more, so we need to sync the access with the via clock.  Plus
	// the whole access takes half a (via) cycle and ends when synced
	// with the main cpu again.

	// Get the main cpu time
	u64 cycle = m_maincpu->total_cycles();

	// Get the number of the cycle the via is in at that time
	u64 via_cycle = cycle * m_via1->clock() / m_maincpu->clock();

	// The access is going to start at via_cycle+1 and end at
	// via_cycle+1.5, compute what that means in maincpu cycles (the
	// +1 rounds up, since the clocks are too different to ever be
	// synced).
	u64 main_cycle = (via_cycle * 2 + 3) * m_maincpu->clock() / (2 * m_via1->clock()) + 1;

	// Finally adjust the main cpu icount as needed.
	m_maincpu->adjust_icount(-int(main_cycle - cycle));
}

uint32_t macquadra_state::rom_switch_r(offs_t offset)
{
	// disable the overlay
	if (m_overlay)
	{
		address_space& space = m_maincpu->space(AS_PROGRAM);
		const u32 memory_end = m_ram->size() - 1;
		void *memory_data = m_ram->pointer();
		offs_t memory_mirror = memory_end & ~memory_end;

		space.install_ram(0x00000000, memory_end & ~memory_mirror, memory_mirror, memory_data);
		m_overlay = false;
	}

	//printf("rom_switch_r: offset %08x ROM_size -1 = %08x, masked = %08x\n", offset, m_rom_size-1, offset & ((m_rom_size - 1)>>2));

	return m_rom_ptr[offset & ((m_rom_size - 1)>>2)];
}

TIMER_CALLBACK_MEMBER(macquadra_state::mac_6015_tick)
{
	/* handle ADB keyboard/mouse */
	m_macadb->adb_vblank();
}

uint8_t macquadra_state::mac_5396_r(offs_t offset)
{
	if (offset < 0x100)
	{
		return m_ncr1->read(offset>>4);
	}
	else    // pseudo-DMA: read from the FIFO
	{
		return m_ncr1->dma_r();
	}

	// never executed
	return 0;
}

void macquadra_state::mac_5396_w(offs_t offset, uint8_t data)
{
	if (offset < 0x100)
	{
		m_ncr1->write(offset>>4, data);
	}
	else    // pseudo-DMA: write to the FIFO
	{
		m_ncr1->dma_w(data);
	}
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/
void macquadra_state::quadra700_map(address_map &map)
{
	map(0x40000000, 0x400fffff).r(FUNC(macquadra_state::rom_switch_r)).mirror(0x0ff00000);

	map(0x50000000, 0x50001fff).rw(FUNC(macquadra_state::mac_via_r), FUNC(macquadra_state::mac_via_w)).mirror(0x00fc0000);
	map(0x50002000, 0x50003fff).rw(FUNC(macquadra_state::mac_via2_r), FUNC(macquadra_state::mac_via2_w)).mirror(0x00fc0000);
// 50008000 = Ethernet MAC ID PROM
// 5000a000 = Sonic (DP83932) ethernet
// 5000f000 = SCSI cf96, 5000f402 = SCSI #2 cf96
	map(0x5000f000, 0x5000f401).rw(FUNC(macquadra_state::mac_5396_r), FUNC(macquadra_state::mac_5396_w)).mirror(0x00fc0000);
	map(0x5000c000, 0x5000dfff).rw(FUNC(macquadra_state::mac_scc_r), FUNC(macquadra_state::mac_scc_2_w)).mirror(0x00fc0000);
	map(0x50014000, 0x50015fff).rw(m_easc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x00fc0000);
	map(0x5001e000, 0x5001ffff).rw(FUNC(macquadra_state::swim_r), FUNC(macquadra_state::swim_w)).mirror(0x00fc0000);

	// f9800000 = VDAC / DAFB
	map(0xf9000000, 0xf91fffff).ram().share("vram");
	map(0xf9800000, 0xf98001ff).rw(FUNC(macquadra_state::dafb_r), FUNC(macquadra_state::dafb_w));
	map(0xf9800200, 0xf980023f).rw(FUNC(macquadra_state::dafb_dac_r), FUNC(macquadra_state::dafb_dac_w));
}

uint8_t macquadra_state::mac_via_in_a()
{
	return 0xc1;
}

uint8_t macquadra_state::mac_via_in_b()
{
	int val = m_macadb->get_adb_state()<<4;
	val |= m_rtc->data_r();

	if (!m_adb_irq_pending)
	{
		val |= 0x08;
	}

//  printf("%s VIA1 IN_B = %02x\n", machine().describe_context().c_str(), val);

	return val;
}

void macquadra_state::mac_via_out_a(uint8_t data)
{
	int hdsel = BIT(data, 5);
	if (hdsel != m_hdsel)
	{
		if (m_cur_floppy)
		{
			m_cur_floppy->ss_w(hdsel);
		}
	}
	m_hdsel = hdsel;
}

void macquadra_state::mac_via_out_b(uint8_t data)
{
//  printf("%s VIA1 OUT B: %02x\n", machine().describe_context().c_str(), data);
	m_macadb->mac_adb_newaction((data & 0x30) >> 4);

	m_rtc->ce_w((data & 0x04)>>2);
	m_rtc->data_w(data & 0x01);
	m_rtc->clk_w((data >> 1) & 0x01);
}

uint8_t macquadra_state::mac_via2_in_a()
{
	return 0x80 | m_nubus_irq_state;
}

uint8_t macquadra_state::mac_via2_in_b()
{
	return 0xcf;        // indicate no NuBus transaction error
}

void macquadra_state::mac_via2_out_a(uint8_t data)
{
}

void macquadra_state::mac_via2_out_b(uint8_t data)
{
	// chain 60.15 Hz to VIA1
	m_via1->write_ca1(data>>7);
}

void macquadra_state::phases_w(uint8_t phases)
{
	if (m_cur_floppy)
		m_cur_floppy->seek_phase_w(phases);
}

void macquadra_state::devsel_w(uint8_t devsel)
{
	if (devsel == 1)
		m_cur_floppy = m_floppy[0]->get_device();
	else if (devsel == 2)
		m_cur_floppy = m_floppy[1]->get_device();
	else
		m_cur_floppy = nullptr;

	m_swim->set_floppy(m_cur_floppy);
	if (m_cur_floppy)
		m_cur_floppy->ss_w(m_hdsel);
}

/***************************************************************************
    DEVICE CONFIG
***************************************************************************/

static INPUT_PORTS_START( macadb )
INPUT_PORTS_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void macquadra_state::macqd700(machine_config &config)
{
	/* basic machine hardware */
	M68040(config, m_maincpu, 50_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &macquadra_state::quadra700_map);
	m_maincpu->set_dasm_override(std::function(&mac68k_dasm_override), "mac68k_dasm_override");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(75.08);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1260));
	m_screen->set_size(1152, 870);
	m_screen->set_visarea(0, 1152-1, 0, 870-1);
	m_screen->set_screen_update(FUNC(macquadra_state::screen_update_dafb));

	PALETTE(config, m_palette).set_entries(256);

	RTC3430042(config, m_rtc, XTAL(32'768));
	m_rtc->cko_cb().set(m_via1, FUNC(via6522_device::write_ca2));

	SWIM1(config, m_swim, C15M);
	m_swim->phases_cb().set(FUNC(macquadra_state::phases_w));
	m_swim->devsel_cb().set(FUNC(macquadra_state::devsel_w));

	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_nc(config, m_floppy[1]);

	SCC8530N(config, m_scc, C7M);
//  m_scc->intrq_callback().set(FUNC(macquadra_state::set_scc_interrupt));

	// SCSI bus and devices
	NSCSI_BUS(config, m_scsibus1);
	NSCSI_CONNECTOR(config, "scsi1:0", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:1", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:2", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:3", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:4", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:5", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:6", mac_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi1:7").option_set("ncr5394", NCR53CF94).clock(50_MHz_XTAL / 2).machine_config(
		[this] (device_t *device)
		{
			ncr53cf94_device &adapter = downcast<ncr53cf94_device &>(*device);

			adapter.set_busmd(ncr53cf94_device::BUSMD_0);
			adapter.irq_handler_cb().set(*this, FUNC(macquadra_state::irq_539x_1_w));
			adapter.drq_handler_cb().set(*this, FUNC(macquadra_state::drq_539x_1_w));
		});

	DP83932C(config, m_sonic, 40_MHz_XTAL / 2);
	m_sonic->set_bus(m_maincpu, 0);

	nubus_device &nubus(NUBUS(config, "nubus", 40_MHz_XTAL / 4));
	nubus.set_space(m_maincpu, AS_PROGRAM);
	nubus.out_irq9_callback().set(FUNC(macquadra_state::nubus_irq_9_w));
	nubus.out_irqa_callback().set(FUNC(macquadra_state::nubus_irq_a_w));
	nubus.out_irqb_callback().set(FUNC(macquadra_state::nubus_irq_b_w));
	nubus.out_irqc_callback().set(FUNC(macquadra_state::nubus_irq_c_w));
	nubus.out_irqd_callback().set(FUNC(macquadra_state::nubus_irq_d_w));
	nubus.out_irqe_callback().set(FUNC(macquadra_state::nubus_irq_e_w));
	NUBUS_SLOT(config, "nbd", "nubus", mac_nubus_cards, nullptr);
	NUBUS_SLOT(config, "nbe", "nubus", mac_nubus_cards, nullptr);

	R65NC22(config, m_via1, C7M/10);
	m_via1->readpa_handler().set(FUNC(macquadra_state::mac_via_in_a));
	m_via1->readpb_handler().set(FUNC(macquadra_state::mac_via_in_b));
	m_via1->writepa_handler().set(FUNC(macquadra_state::mac_via_out_a));
	m_via1->writepb_handler().set(FUNC(macquadra_state::mac_via_out_b));
	m_via1->irq_handler().set(FUNC(macquadra_state::mac_via_irq));
	m_via1->cb2_handler().set(FUNC(macquadra_state::via_cb2_w));

	R65NC22(config, m_via2, C7M/10);
	m_via2->readpa_handler().set(FUNC(macquadra_state::mac_via2_in_a));
	m_via2->readpb_handler().set(FUNC(macquadra_state::mac_via2_in_b));
	m_via2->writepa_handler().set(FUNC(macquadra_state::mac_via2_out_a));
	m_via2->writepb_handler().set(FUNC(macquadra_state::mac_via2_out_b));
	m_via2->irq_handler().set(FUNC(macquadra_state::mac_via2_irq));

	MACADB(config, m_macadb, C15M);
	m_macadb->via_clock_callback().set(m_via1, FUNC(via6522_device::write_cb1));
	m_macadb->via_data_callback().set(m_via1, FUNC(via6522_device::write_cb2));
	m_macadb->adb_irq_callback().set(FUNC(macquadra_state::adb_irq_w));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	ASC(config, m_easc, 22.5792_MHz_XTAL, asc_device::asc_type::EASC);
	m_easc->irqf_callback().set(m_via2, FUNC(via6522_device::write_cb1)).invert();
	m_easc->add_route(0, "lspeaker", 1.0);
	m_easc->add_route(1, "rspeaker", 1.0);

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("4M");
	m_ram->set_extra_options("8M,16M,32M,64M,68M,72M,80M,96M,128M");

	SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");
	SOFTWARE_LIST(config, "flop35hd_list").set_original("mac_hdflop");
}

ROM_START( macqd700 )
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD( "420dbff3.rom", 0x000000, 0x100000, CRC(88ea2081) SHA1(7a8ee468d16e64f2ad10cb8d1a45e6f07cc9e212) )
ROM_END

} // anonymous namespace

COMP( 1991, macqd700, 0, 0, macqd700, macadb, macquadra_state, init_macqd700,  "Apple Computer", "Macintosh Quadra 700", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
