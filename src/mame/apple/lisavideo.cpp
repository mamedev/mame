// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// The LISA video board

#include "emu.h"
#include "lisavideo.h"
#include "screen.h"

DEFINE_DEVICE_TYPE(LISAVIDEO,  lisa_video_device,  "lisavideo",  "Lisa video device")
DEFINE_DEVICE_TYPE(MACXLVIDEO, macxl_video_device, "macxlvideo", "MacXL screen kit video device")

ROM_START( lisa_video )
	ROM_REGION(0x100, "vprom", 0)
	// Dumps vary because the serial number is in bit 7 of every byte
	ROM_LOAD("341-0229-a.rom", 0x00, 0x100, CRC(75904783) SHA1(3b0023bd90f2ca1be0b099160a566b044856885d))
ROM_END

ROM_START( macxl_video )
	ROM_REGION(0x100, "vprom", 0)
	// No serial number in that one
	ROM_LOAD("341-0348-a", 0x00, 0x100, CRC(778fc5d9) SHA1(ec2533a6bd9e75d02fa69754fb82c7c28f0366ab))
ROM_END


lisa_video_device::lisa_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_vint_cb(*this),
	m_vprom(*this, "vprom"),
	m_ram(*this, finder_base::DUMMY_TAG),
	m_screen(*this, "screen")
{
}

lisa_video_device::lisa_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	lisa_video_device(mconfig, LISAVIDEO, tag, owner, clock)
{
}

macxl_video_device::macxl_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	lisa_video_device(mconfig, MACXLVIDEO, tag, owner, clock)
{
}

const tiny_rom_entry *lisa_video_device::device_rom_region() const
{
	return ROM_NAME( lisa_video );
}

const tiny_rom_entry *macxl_video_device::device_rom_region() const
{
	return ROM_NAME( macxl_video );
}

void lisa_video_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(*this, FUNC(lisa_video_device::screen_update));

	// Need to put something, it's impossible to check the video rom
	// before the validity check (which doesn't load roms) wants the
	// screen clock to not be zero.

	m_screen->set_raw(20.37504_MHz_XTAL, 896, 0, 720, 371, 0, 364);
}

void lisa_video_device::device_config_complete()
{

//	m_visible = screen().visible_area();
	//	fprintf(stderr, "COMPLETE size %x %x - %d %d\n", m_visible.left(), m_visible.top(), m_visible.width(), m_visible.height());
}

void lisa_video_device::base_w(u8 data)
{
	m_base = data;
}

u8 lisa_video_device::base_r()
{
	return m_base;
}

void lisa_video_device::device_start()
{
	m_vint_timer = timer_alloc(FUNC(lisa_video_device::vint_timer_tick), this);
	analyze_video_rom();

	save_item(NAME(m_base));
	save_item(NAME(m_vint_raw));
	save_item(NAME(m_vint_masked));
	save_item(NAME(m_vtmsk));

	m_vint_raw = false;
	m_vint_masked = false;
	m_vtmsk = false;
}

void lisa_video_device::device_reset()
{
	m_base = 0x3f;
	tick_0();
}

void lisa_video_device::vtmsk_0()
{
	m_vtmsk = false;
}

void lisa_video_device::vtmsk_1()
{
	m_vtmsk = true;
	if(m_vint_masked) {
		m_vint_masked = false;
		m_vint_cb(false);
	}
}

void lisa_video_device::tick_1()
{
	m_vint_raw = true;
	if(!m_vtmsk) {
		m_vint_masked = true;
		m_vint_cb(true);
	}

	u64 when = machine().time().as_ticks(clock()) / 16;
	u64 whenr = when % m_video_states.size();
	u64 next = m_video_states.size() - whenr;
	m_vint_timer->adjust(attotime::from_ticks(next*16, clock()), 0);

}

void lisa_video_device::tick_0()
{
	m_vint_raw = false;

	u64 when = machine().time().as_ticks(clock()) / 16;
	u64 whenr = when % m_video_states.size();
	u64 next = m_vsint_start;
	if(whenr) {
		if(whenr > m_vsint_start)
			next += m_video_states.size() - whenr;
		else
			next -= whenr;
	}
	m_vint_timer->adjust(attotime::from_ticks(next*16, clock()), 1);
}

TIMER_CALLBACK_MEMBER(lisa_video_device::vint_timer_tick)
{
	if(param)
		tick_1();
	else
		tick_0();
}

u32 lisa_video_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u32 offset = ((m_base & 0x3f) << 14) & ((m_ram.bytes() >> 1) - 1);

	for(int y=0; y != m_max_vy; y++) {
		u32 *dest = &bitmap.pix(y);
		for(int x=0; x != m_max_vx; x += 16) {
			u16 v = m_ram[offset++];
			for(int bit = 0; bit != 16; bit++)
				if((v << bit) & 0x8000)
					*dest++ = 0x000000;
				else
					*dest++ = 0xffffff;
		}
	}
	return 0;
}

void lisa_video_device::analyze_video_rom()
{
	// Run a complete frame and track address and signals all along
	// Each state is 16 pixel clocks / 4 cpu clocks
	m_video_states.clear();

	u8 counter = 0;
	u16 address = 0;
	u16 x = 0;
	u16 y = 0;
	for(;;) {
		u8 value = m_vprom[counter | (address & 0x100 ? 0x40 : 0) | (address & 0x4000 ? 0x80 : 0)];

		m_video_states.emplace_back(video_state(address,
												x, y,
												value & 0x02,   // vertical sync
												value & 0x40,   // horizontal sync
												value & 0x10,   // composite sync
												value & 0x04,   // blank
												value & 0x20,   // interrupt request
												value & 0x80)); // serial number

		if(value & 0x04)
			address ++;

		if(value & 0x08)   // Zero address (end of frame)
			break;

		if(value & 0x01) { // Zero counter (end of line)
			counter = 0;
			x = 0;
			y ++;
		} else {
			counter = (counter + 1) & 0x3f; // There's always a zero counter before it overflows in practice)
			x += 16;
		}
	}

	u16 max_x = 0, max_y = 0, max_vx = 0, max_vy = 0;
	u32 time_vsint_start = 0; // Interrupt always ends at start of frame

	for(unsigned int i = 0; i != m_video_states.size(); i++) {
		const auto &s = m_video_states[i];
		max_x = std::max(s.x, max_x);
		max_y = std::max(s.y, max_y);
		if(s.blank && s.vsync) {
			max_vx = std::max(s.x, max_vx);

			// Trick to ensure we're far enough in the line than vsync
			// has actually started if it's going to, since it's not
			// on for the first 4 (lisa) or 6 (xl) words of the first
			// sync line
			if(s.x == 96)
				max_vy = std::max(s.y, max_vy);
		}

		if(!time_vsint_start && !s.vsint)
			time_vsint_start = i;
	}

	max_x += 16;
	max_vx += 16;
	max_y += 1;
	max_vy += 1;

	m_max_vx = max_vx;
	m_max_vy = max_vy;
	m_vsint_start = time_vsint_start;

	m_screen->set_raw(clock(), max_x, 0, max_vx, max_y, 0, max_vy);
	logerror("size (%d %d) (%d %d) vsync %g Hz\n", max_x, max_y, max_vx, max_vy, 20.37504e6 / 16 / m_video_states.size());
}

u32 lisa_video_device::current_state_index() const
{
	u64 when = machine().time().as_ticks(clock()) / 16;
	return when % m_video_states.size();
}

bool lisa_video_device::sn_r()
{
	return m_video_states[current_state_index()].sn;
}

u16 lisa_video_device::status_r()
{
	const auto &s = m_video_states[current_state_index()];
	u16 pixels = m_ram[(((m_base & 0x3f) << 14) & ((m_ram.bytes() >> 1) - 1)) | s.address];

	// We don't exactly know at which pixel time the read happens.
	// Given the alternance between cpu read and video read, pick
	// something in the middle.

	return (m_vint_masked ? 0x00 : 0x04) | (pixels & 0x0100 ? 0x10 : 0x00) | (s.csync ? 0x20 : 0x00) | (0 /* invid */ ? 0x40 : 0x00) | 0x80;
}
