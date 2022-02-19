// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, MetalliC
#include "emu.h"
#include "includes/spectrum.h"
#include "includes/spec128.h"

#include "machine/beta.h"
#include "sound/ay8910.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/tzx_cas.h"


namespace {

#define PENTAGON_SCREEN  rectangle{138, 393, 80, 271}

class pentagon_state : public spectrum_128_state
{
public:
	pentagon_state(const machine_config &mconfig, device_type type, const char *tag)
		: spectrum_128_state(mconfig, type, tag)
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
		, m_bank4(*this, "bank4")
		, m_beta(*this, BETA_DISK_TAG)
	{ }

	void pent1024(machine_config &config);
	void pentagon(machine_config &config);

protected:
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

private:
	enum
	{
		TIMER_IRQ_ON,
		TIMER_IRQ_OFF
	};

	void pentagon_port_7ffd_w(uint8_t data);
	void pentagon_scr_w(offs_t offset, uint8_t data);
	void pentagon_scr2_w(offs_t offset, uint8_t data);
	uint8_t beta_neutral_r(offs_t offset);
	uint8_t beta_enable_r(offs_t offset);
	uint8_t beta_disable_r(offs_t offset);
	INTERRUPT_GEN_MEMBER(pentagon_interrupt);
	TIMER_CALLBACK_MEMBER(irq_on);
	TIMER_CALLBACK_MEMBER(irq_off);
	void pentagon_io(address_map &map);
	void pentagon_mem(address_map &map);
	void pentagon_switch(address_map &map);

	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;
	required_device<beta_disk_device> m_beta;

	address_space *m_program;
	uint8_t *m_p_ram;
	void pentagon_update_memory();

	// Redefined here as POC of improved screen porocessing. Intended to update original implementation.
	void to_display(unsigned int &x, unsigned int &y);
	void spectrum_UpdateScreenBitmap(bool eof = false) override;
	// Following 2 are obsolete
	u32 screen_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	void spectrum_UpdateBorderBitmap() override;
};

void pentagon_state::pentagon_update_memory()
{
	uint8_t *messram = m_ram->pointer();

	m_screen_location = messram + ((m_port_7ffd_data & 8) ? (7<<14) : (5<<14));

	if (strcmp(machine().system().name, "pent1024") != 0)
	{
		m_bank4->set_base(messram + ((m_port_7ffd_data & 0x07) * 0x4000));
	}
	else
	{
		// currently 512Kb ram expansion supported
		m_bank4->set_base(messram + (((m_port_7ffd_data & 0x07) | ((m_port_7ffd_data & 0xc0) >> 3)) * 0x4000));
	}

	if (m_beta->started() && m_beta->is_active() && !( m_port_7ffd_data & 0x10 ) )
	{
		/* GLUK */
		if (strcmp(machine().system().name, "pent1024")==0)
			m_ROMSelection = 2;
		else
			m_ROMSelection = BIT(m_port_7ffd_data, 4);
	}
	else
		/* ROM switching */
		m_ROMSelection = BIT(m_port_7ffd_data, 4);

	/* rom 0 is 128K rom, rom 1 is 48 BASIC */
	m_bank1->set_base(&m_p_ram[0x10000 + (m_ROMSelection<<14)]);
}

void pentagon_state::pentagon_port_7ffd_w(uint8_t data)
{
	/* disable paging */
	if (m_port_7ffd_data & 0x20)
		return;

	if ((m_port_7ffd_data ^ data) & 0x08)
		spectrum_UpdateScreenBitmap();

	/* store new state */
	m_port_7ffd_data = data;

	/* update memory */
	pentagon_update_memory();
}

void pentagon_state::pentagon_scr_w(offs_t offset, uint8_t data)
{
	spectrum_UpdateScreenBitmap();

	*((uint8_t*)m_bank2->base() + offset) = data;
}

void pentagon_state::pentagon_scr2_w(offs_t offset, uint8_t data)
{
	if ((m_port_7ffd_data & 0x0f) == 0x0f || (m_port_7ffd_data & 0x0f) == 5)
		spectrum_UpdateScreenBitmap();

	*((uint8_t*)m_bank4->base() + offset) = data;
}

// This one not needed as we draw directly at screen's bitmap.
u32 pentagon_state::screen_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void pentagon_state::to_display(unsigned int &x, unsigned int &y)
{
	rectangle va = m_screen->visible_area();
	if(y < va.top() || y > va.bottom()) {
		x = va.left();
		y = va.top();
	} else if(x < va.left()) {
		x = va.left();
	} else if(x > va.right()) {
		x = va.left();
		y++;
		to_display(x, y);
	}
}

void pentagon_state::spectrum_UpdateScreenBitmap(bool eof)
{
	unsigned int to_x = m_screen->hpos();
	unsigned int to_y = m_screen->vpos();
	to_display(to_x, to_y);

	if ((m_previous_screen_x == to_x) && (m_previous_screen_y == to_y) && !eof)
		return;

	bitmap_ind16 *bm = &m_screen->curbitmap().as_ind16();
	if (bm->valid())
	{
		u16 border_color = get_border_color();
		do
		{
			u16 x = m_previous_screen_x - PENTAGON_SCREEN.left();
			u16 y = m_previous_screen_y - PENTAGON_SCREEN.top();

			if(PENTAGON_SCREEN.contains(m_previous_screen_x, m_previous_screen_y))
			{
				// this can/must be optimised
				if ((x & 7) == 0)
				{
					u16 *pix = &bm->pix(m_previous_screen_y, m_previous_screen_x);
					u8 attr = *(m_screen_location + ((y & 0xF8) << 2) + (x >> 3) + 0x1800);
					u8 scr = *(m_screen_location + ((y & 7) << 8) + ((y & 0x38) << 2) + ((y & 0xC0) << 5) + (x >> 3));
					u16 ink = (attr & 0x07) + ((attr >> 3) & 0x08);
					u16 pap = (attr >> 3) & 0x0f;

					if (m_flash_invert && (attr & 0x80))
						scr = ~scr;

					for (uint8_t b = 0x80; b != 0; b >>= 1)
						*pix++ = (scr & b) ? ink : pap;
				}
			}
			else
			{
				bm->pix(m_previous_screen_y, m_previous_screen_x) = border_color;
			}

			to_display(++m_previous_screen_x, m_previous_screen_y);
		} while (!((m_previous_screen_x == to_x) && (m_previous_screen_y == to_y)));
	}
}

// Any calls must be replaced with spectrum_UpdateScreenBitmap()
void pentagon_state::spectrum_UpdateBorderBitmap()
{
	spectrum_UpdateScreenBitmap();
}

void pentagon_state::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TIMER_IRQ_ON:
		irq_on(param);
		break;
	case TIMER_IRQ_OFF:
		irq_off(param);
		break;
	default:
		throw emu_fatalerror("Unknown id in pentagon_state::device_timer");
	}
}

TIMER_CALLBACK_MEMBER(pentagon_state::irq_on)
{
	m_maincpu->set_input_line(0, HOLD_LINE);
	timer_set(attotime::from_ticks(32, XTAL(14'000'000) / 4), TIMER_IRQ_OFF, 0);
}

TIMER_CALLBACK_MEMBER(pentagon_state::irq_off)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

INTERRUPT_GEN_MEMBER(pentagon_state::pentagon_interrupt)
{
	timer_set(attotime::zero, TIMER_IRQ_ON, 0);
}

uint8_t pentagon_state::beta_neutral_r(offs_t offset)
{
	return m_program->read_byte(offset);
}

uint8_t pentagon_state::beta_enable_r(offs_t offset)
{
	if (!(machine().side_effects_disabled())) {
		if (m_ROMSelection == 1) {
			m_ROMSelection = 3;
			if (m_beta->started()) {
				m_beta->enable();
				m_bank1->set_base(memregion("beta:beta")->base());
			}
		}
	}

	return m_program->read_byte(offset + 0x3d00);
}

uint8_t pentagon_state::beta_disable_r(offs_t offset)
{
	if (!(machine().side_effects_disabled())) {
		if (m_beta->started() && m_beta->is_active()) {
			m_ROMSelection = BIT(m_port_7ffd_data, 4);
			m_beta->disable();
			m_bank1->set_base(&m_p_ram[0x10000 + (m_ROMSelection << 14)]);
		}
	}

	return m_program->read_byte(offset + 0x4000);
}

void pentagon_state::pentagon_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankr("bank1");
	map(0x4000, 0x7fff).bankrw("bank2");
	map(0x8000, 0xbfff).bankrw("bank3");
	map(0xc000, 0xffff).bankrw("bank4");
}

void pentagon_state::pentagon_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0000).mirror(0x7ffd).w(FUNC(pentagon_state::pentagon_port_7ffd_w));  // (A15 | A1) == 0
	map(0x001f, 0x001f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::status_r), FUNC(beta_disk_device::command_w));
	map(0x003f, 0x003f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::track_r), FUNC(beta_disk_device::track_w));
	map(0x005f, 0x005f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::sector_r), FUNC(beta_disk_device::sector_w));
	map(0x007f, 0x007f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::data_r), FUNC(beta_disk_device::data_w));
	map(0x00fe, 0x00fe).select(0xff00).rw(FUNC(pentagon_state::spectrum_ula_r), FUNC(pentagon_state::spectrum_ula_w));
	map(0x00ff, 0x00ff).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::state_r), FUNC(beta_disk_device::param_w));
	map(0x8000, 0x8000).mirror(0x3ffd).w("ay8912", FUNC(ay8910_device::data_w));
	map(0xc000, 0xc000).mirror(0x3ffd).rw("ay8912", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
}

void pentagon_state::pentagon_switch(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(pentagon_state::beta_neutral_r)); // Overlap with next because we want real addresses on the 3e00-3fff range
	map(0x3d00, 0x3dff).r(FUNC(pentagon_state::beta_enable_r));
	map(0x4000, 0xffff).r(FUNC(pentagon_state::beta_disable_r));
}

void pentagon_state::machine_reset()
{
	uint8_t *messram = m_ram->pointer();
	m_program = &m_maincpu->space(AS_PROGRAM);
	m_p_ram = memregion("maincpu")->base();

	m_program->install_write_handler(0x4000, 0x5aff, write8sm_delegate(*this, FUNC(pentagon_state::pentagon_scr_w)));
	m_program->install_write_handler(0xc000, 0xdaff, write8sm_delegate(*this, FUNC(pentagon_state::pentagon_scr2_w)));

	if (m_beta->started())
	{
		if (strcmp(machine().system().name, "pent1024")==0)
			m_beta->enable();
	}
	memset(messram,0,128*1024);

	/* Bank 5 is always in 0x4000 - 0x7fff */
	m_bank2->set_base(messram + (5<<14));

	/* Bank 2 is always in 0x8000 - 0xbfff */
	m_bank3->set_base(messram + (2<<14));

	m_port_7ffd_data = 0;
	m_port_1ffd_data = -1;
	pentagon_update_memory();
}

void pentagon_state::video_start()
{
	m_frame_invert_count = 16;
	m_frame_number = 0;
	m_flash_invert = 0;

	m_previous_border_x = m_previous_border_y = 0;
	m_previous_screen_x = m_previous_screen_y = 0;

	m_screen_location = m_ram->pointer() + (5 << 14);
}

/* F4 Character Displayer */
static const gfx_layout spectrum_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	96,                 /* 96 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_pentagon )
	GFXDECODE_ENTRY( "maincpu", 0x17d00, spectrum_charlayout, 0, 8 )
GFXDECODE_END



void pentagon_state::pentagon(machine_config &config)
{
	spectrum_128(config);
	m_maincpu->set_clock(XTAL(14'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &pentagon_state::pentagon_mem);
	m_maincpu->set_addrmap(AS_IO, &pentagon_state::pentagon_io);
	m_maincpu->set_addrmap(AS_OPCODES, &pentagon_state::pentagon_switch);
	m_maincpu->set_vblank_int("screen", FUNC(pentagon_state::pentagon_interrupt));

	//m_screen->set_raw(XTAL(14'000'000) / 2, 448, 0, 352,  320, 0, 304);
	m_screen->set_raw(XTAL(14'000'000) / 2, 448, PENTAGON_SCREEN.left() - 48, PENTAGON_SCREEN.right() + 49,  320, PENTAGON_SCREEN.top() - 48, PENTAGON_SCREEN.bottom() + 49);

	BETA_DISK(config, m_beta, 0);
	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_pentagon);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ay8912_device &ay8912(AY8912(config.replace(), "ay8912", XTAL(14'000'000)/8));
	ay8912.add_route(0, "lspeaker", 0.50);
	ay8912.add_route(1, "lspeaker", 0.25);
	ay8912.add_route(1, "rspeaker", 0.25);
	ay8912.add_route(2, "rspeaker", 0.50);

	config.device_remove("exp");

	SOFTWARE_LIST(config, "cass_list_pen").set_original("pentagon_cass");
	SOFTWARE_LIST(config, "betadisc_list").set_original("spectrum_betadisc_flop");
}

void pentagon_state::pent1024(machine_config &config)
{
	pentagon(config);
	/* internal ram */
	m_ram->set_default_size("1024K");
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(pentagon)
	ROM_REGION(0x01c000, "maincpu", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "v1", "Pentagon 128K")
	ROMX_LOAD("128p-0.rom", 0x010000, 0x4000, CRC(124ad9e0) SHA1(d07fcdeca892ee80494d286ea9ea5bf3928a1aca), ROM_BIOS(0))
	ROMX_LOAD("128p-1.rom", 0x014000, 0x4000, CRC(b96a36be) SHA1(80080644289ed93d71a1103992a154cc9802b2fa), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v2", "Pentagon 128K 93")
	ROMX_LOAD("128tr93.rom",0x010000, 0x4000, CRC(08ad241c) SHA1(16daba547c644ef01ce76d2686ccfbff72e13dbe), ROM_BIOS(1))
	ROMX_LOAD("128p-1.rom", 0x014000, 0x4000, CRC(b96a36be) SHA1(80080644289ed93d71a1103992a154cc9802b2fa), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v3", "Pentagon 128K (joined)")
	ROMX_LOAD("pentagon.rom", 0x010000, 0x8000, CRC(aa1ce4bd) SHA1(a584272f21dc82c14b7d4f1ed440e23a976e71f0), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v4", "Pentagon 128K Spanish")
	ROMX_LOAD("pent-es.rom", 0x010000, 0x8000, CRC(34d04bae) SHA1(6782c8c0ee77c40d6d3170a254894dae44ddc93e), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v5", "Pentagon 128K SOS89R Monitor")
	ROMX_LOAD("128p-0.rom", 0x010000, 0x4000, CRC(124ad9e0) SHA1(d07fcdeca892ee80494d286ea9ea5bf3928a1aca), ROM_BIOS(4))
	ROMX_LOAD("sos89r.rom", 0x014000, 0x4000, CRC(09c9e7e1) SHA1(29c567921abd377d2f9c088352c392a5a0858651), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "v6", "Pentagon 128K 1990 Monitor")
	ROMX_LOAD("128p-0.rom",  0x010000, 0x4000, CRC(124ad9e0) SHA1(d07fcdeca892ee80494d286ea9ea5bf3928a1aca), ROM_BIOS(5))
	ROMX_LOAD("basic90.rom", 0x014000, 0x4000, CRC(a41575ba) SHA1(44c5de86e765172b0af154fe3934643ce40bf378), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "v7", "Pentagon 128K RaK(c) 1991 Monitor")
	ROMX_LOAD("128p-0.rom", 0x010000, 0x4000, CRC(124ad9e0) SHA1(d07fcdeca892ee80494d286ea9ea5bf3928a1aca), ROM_BIOS(6))
	ROMX_LOAD("sos48.rom",  0x014000, 0x4000, CRC(ceb4005d) SHA1(d56c01ea7abdca178efb2b1c6b2866a9a38274ee), ROM_BIOS(6))
	ROM_SYSTEM_BIOS(7, "v8", "Pentagon 128K Dynaelectronics 1989")
	ROMX_LOAD("128p-0.rom", 0x010000, 0x4000, CRC(124ad9e0) SHA1(d07fcdeca892ee80494d286ea9ea5bf3928a1aca), ROM_BIOS(7))
	ROMX_LOAD("m48a.rom",   0x014000, 0x4000, CRC(a3b4def6) SHA1(7ad59ca373876d452b0cf0ed5edb0e93c3176f1a), ROM_BIOS(7))
	ROM_SYSTEM_BIOS(8, "v9", "ZXVGS v0.22 by Yarek")
	ROMX_LOAD("zxvgs-22-0.rom", 0x010000, 0x4000, CRC(63041c61) SHA1(f6718097d939afa8881b4436741a5a23d7e93d78), ROM_BIOS(8))
	ROMX_LOAD("zxvgs-22-1.rom", 0x014000, 0x4000, CRC(f3736047) SHA1(f3739bf460a57e3f10e8dfb1e7120842938d27ea), ROM_BIOS(8))
	ROM_SYSTEM_BIOS(9, "v10", "ZXVGS v0.29 by Yarek")
	ROMX_LOAD("zxvg-29-0.rom", 0x010000, 0x4000, CRC(3b66f433) SHA1(d21df9e7f1ee99d8b38c2e6a32727aac0f1d5dc6), ROM_BIOS(9))
	ROMX_LOAD("zxvg-1.rom",    0x014000, 0x4000, CRC(a8baca3e) SHA1(f2f131eaa4de832eda76290e48f86e465d28ded7), ROM_BIOS(9))
	ROM_SYSTEM_BIOS(10, "v11", "ZXVGS v0.30 by Yarek")
	ROMX_LOAD("zxvg-30-0.rom", 0x010000, 0x4000, CRC(533e0f26) SHA1(b5f157c5d0da414ec77e445fdc40b78450129709), ROM_BIOS(10))
	ROMX_LOAD("zxvg-1.rom",    0x014000, 0x4000, CRC(a8baca3e) SHA1(f2f131eaa4de832eda76290e48f86e465d28ded7), ROM_BIOS(10))
	ROM_SYSTEM_BIOS(11, "v12", "ZXVGS v0.31 by Yarek")
	ROMX_LOAD("zxvg-31-0.rom", 0x010000, 0x4000, CRC(76f43500) SHA1(1c7cd52894847668418876d55b93b213d89d92ee), ROM_BIOS(11))
	ROMX_LOAD("zxvg-1.rom",    0x014000, 0x4000, CRC(a8baca3e) SHA1(f2f131eaa4de832eda76290e48f86e465d28ded7), ROM_BIOS(11))
	ROM_SYSTEM_BIOS(12, "v13", "ZXVGS v0.35 by Yarek")
	ROMX_LOAD("zxvg-35-0.rom", 0x010000, 0x4000, CRC(5cc8b3b1) SHA1(6c6d0ef1b65d7dc4f607d17204488264575ce48c), ROM_BIOS(12))
	ROMX_LOAD("zxvg-1.rom",    0x014000, 0x4000, CRC(a8baca3e) SHA1(f2f131eaa4de832eda76290e48f86e465d28ded7), ROM_BIOS(12))
	ROM_SYSTEM_BIOS(13, "v14", "NeOS 512")
	ROMX_LOAD("neos_512.rom", 0x010000, 0x4000, CRC(1657fa43) SHA1(647545f06257bce9b1919fcb86b2a49a21c851a7), ROM_BIOS(13))
	ROMX_LOAD("128p-1.rom",   0x014000, 0x4000, CRC(b96a36be) SHA1(80080644289ed93d71a1103992a154cc9802b2fa), ROM_BIOS(13))
ROM_END

ROM_START(pent1024)
	ROM_REGION(0x01c000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("128p-0.rom", 0x010000, 0x4000, CRC(124ad9e0) SHA1(d07fcdeca892ee80494d286ea9ea5bf3928a1aca))
	ROM_LOAD("128p-1.rom", 0x014000, 0x4000, CRC(b96a36be) SHA1(80080644289ed93d71a1103992a154cc9802b2fa))
	ROM_SYSTEM_BIOS(0, "v1", "Gluk 6.3r")
	ROMX_LOAD("gluk63r.rom",0x018000, 0x4000, CRC(ca321d79) SHA1(015eb96dafb273d4f4512c467e9b43c305fd1bc4), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v2", "Gluk 5.2i")
	ROMX_LOAD("gluk52i.rom", 0x018000, 0x4000, CRC(fe44b86a) SHA1(9099d8a0f99a818849ca67ae1a8d3e7eacf06e65), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v3", "Gluk 5.3")
	ROMX_LOAD("gluk53.rom",  0x018000, 0x4000, CRC(479515ef) SHA1(ed656cd4faa36de2e31b38102bcbd8cee12e7976), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v4", "Gluk 5.4")
	ROMX_LOAD("gluk54r.rom", 0x018000, 0x4000, CRC(f4c1e975) SHA1(7e9e116750e1398572695b9cf8a120e47066256e), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v5", "Gluk 5.5r")
	ROMX_LOAD("gluk55r.rom", 0x018000, 0x4000, CRC(3658c1ee) SHA1(4a5c8ca1e090cfb0168796f0d695310fa5c955d3), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "v6", "Gluk 5.5rr")
	ROMX_LOAD("gluk55rr.rom",0x018000, 0x4000, CRC(6b60b818) SHA1(9d606275d17770c9341b33b43f40aee227078827), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "v7", "Gluk 6.0r")
	ROMX_LOAD("gluk60r.rom", 0x018000, 0x4000, CRC(d114a032) SHA1(5db3462ce7a51b473a3a7056e67c11a62cc1cc2a), ROM_BIOS(6))
	ROM_SYSTEM_BIOS(7, "v8", "Gluk 6.0-1r")
	ROMX_LOAD("gluk601r.rom", 0x018000, 0x4000, CRC(daf6310b) SHA1(b8945168d4d136b731b33ec4758f8510c47fb8c4), ROM_BIOS(7))
	ROM_SYSTEM_BIOS(8, "v9", "Gluk 5.1")
	ROMX_LOAD("gluk51.rom",   0x018000, 0x4000, CRC(ea8c760b) SHA1(adaab28066ca46fbcdcf084c3b53d5a1b82d94a9), ROM_BIOS(8))
ROM_END

} // Anonymous namespace


//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT      CLASS           INIT        COMPANY      FULLNAME           FLAGS
COMP( 1991, pentagon, spec128, 0,      pentagon, spec_plus, pentagon_state, empty_init, "<unknown>", "Pentagon 128K",   0 )
COMP( 2005, pent1024, spec128, 0,      pent1024, spec_plus, pentagon_state, empty_init, "<unknown>", "Pentagon 1024SL", 0 )
