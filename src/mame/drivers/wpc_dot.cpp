// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Miodrag Milanovic
/******************************************************************************************
PINBALL
Williams WPC Dot Matrix

Since NVRAM is not working, when it starts factory settings will be applied.
 Press F3, and wait for the game attract mode to commence.

Here are the key codes to enable play:

Game                              NUM  Start game                         End ball
-----------------------------------------------------------------------------------------------
**** Bally (Midway) ****
Gilligan's Island               20003  Hold BC hit 1                      BC
The Party Zone                  20004  Hold PGDN PGUP END hit 1           PGDN PGUP END
**** Williams ****
Hurricane                       50012  Hold BCD hit 1                     BCD and wait
Terminator 2: Judgement Day     50013  Hold ABC hit 1                     ABC
**** Novelty Games ****
Slugfest                        60001  O
Hot Shot                        60017  1 then C then A. Keep hitting I to score a basket.
Slugfest 2                      60021  not emulated but probably same as Slugfest.

ToDo:
- NVRAM
- Outputs
- Mechanical sounds
- Volume control does nothing
- Speech not working

*********************************************************************************************/
#include "emu.h"
#include "includes/wpc_dot.h"
#include "screen.h"
#include "speaker.h"


void wpc_dot_state::wpc_dot_map(address_map &map)
{
	map(0x0000, 0x2fff).rw(FUNC(wpc_dot_state::ram_r), FUNC(wpc_dot_state::ram_w));
	map(0x3000, 0x31ff).bankrw("dmdbank1");
	map(0x3200, 0x33ff).bankrw("dmdbank2");
	map(0x3400, 0x35ff).bankrw("dmdbank3");
	map(0x3600, 0x37ff).bankrw("dmdbank4");
	map(0x3800, 0x39ff).bankrw("dmdbank5");
	map(0x3a00, 0x3bff).bankrw("dmdbank6");
	map(0x3c00, 0x3faf).ram();
	map(0x3fb0, 0x3fff).rw(m_wpc, FUNC(wpc_device::read), FUNC(wpc_device::write)); // WPC device
	map(0x4000, 0x7fff).bankr("cpubank");
	map(0x8000, 0xffff).bankr("fixedbank");
}

static INPUT_PORTS_START( wpc_dot )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Right Flipper")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("Left Flipper")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP15")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP16")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP17")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP18")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE PORT_NAME("Coin Door")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Ticket Dispenser")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD )  // always closed
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP25")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP26")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP27")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP28")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP31")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP32")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP33")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP34")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP35")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP36")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP37")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP38")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP41")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP42")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP45")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("INP48")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP51")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP52")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("INP53")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP54")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP55")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP56")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP57")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP58")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP61")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP62")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP63")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP64")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP65")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP66")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP67")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP68")

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP71")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP72")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP73")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("INP74")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("INP75")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("INP76")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("INP77")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("INP78")

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP81")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP82")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP83")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP84")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP85")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP86")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP87")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP88")

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Service / Escape") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_VOLUME_UP ) PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Begin Test / Enter") PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("DIPS")
	PORT_DIPNAME(0x01,0x01,"Switch 1") PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(0x00,DEF_STR( Off ))
	PORT_DIPSETTING(0x01,DEF_STR( On ))
	PORT_DIPNAME(0x02,0x02,"Switch 2") PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(0x00,DEF_STR( Off ))
	PORT_DIPSETTING(0x02,DEF_STR( On ))
	PORT_DIPNAME(0x04,0x00,"W20") PORT_DIPLOCATION("SWA:3")
	PORT_DIPSETTING(0x00,DEF_STR( Off ))
	PORT_DIPSETTING(0x04,DEF_STR( On ))
	PORT_DIPNAME(0x08,0x00,"W19") PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(0x00,DEF_STR( Off ))
	PORT_DIPSETTING(0x08,DEF_STR( On ))
	PORT_DIPNAME(0xf0,0x00,"Country") PORT_DIPLOCATION("SWA:5,6,7,8")
	PORT_DIPSETTING(0x00,"USA 1")
	PORT_DIPSETTING(0x10,"France 1")
	PORT_DIPSETTING(0x20,"Germany")
	PORT_DIPSETTING(0x30,"France 2")
	PORT_DIPSETTING(0x40,"Unknown 1")
	PORT_DIPSETTING(0x50,"Unknown 2")
	PORT_DIPSETTING(0x60,"Unknown 3")
	PORT_DIPSETTING(0x70,"Unknown 4")
	PORT_DIPSETTING(0x80,"Export 1")
	PORT_DIPSETTING(0x90,"France 3")
	PORT_DIPSETTING(0xa0,"Export 2")
	PORT_DIPSETTING(0xb0,"France 4")
	PORT_DIPSETTING(0xc0,"UK")
	PORT_DIPSETTING(0xd0,"Europe")
	PORT_DIPSETTING(0xe0,"Spain")
	PORT_DIPSETTING(0xf0,"USA 2")
INPUT_PORTS_END

void wpc_dot_state::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch(id)
	{
	case TIMER_VBLANK:
		if((m_vblank_count % 4) == (m_wpc->get_dmd_firq_line()*4/32))
		{
			m_maincpu->set_input_line(M6809_FIRQ_LINE,ASSERT_LINE);
			m_wpc->set_dmd_firq();
		}
		m_vblank_count++;
		break;
	case TIMER_IRQ:
		m_maincpu->set_input_line(M6809_IRQ_LINE,ASSERT_LINE);
		break;
	}
}

void wpc_dot_state::machine_start()
{
	save_item(NAME(m_vblank_count));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_bankmask));
	save_item(NAME(m_ram));
	save_item(NAME(m_dmdram));
}

void wpc_dot_state::machine_reset()
{
	m_cpubank->set_entry(0);
	m_vblank_count = 0;
	m_irq_count = 0;
}

void wpc_dot_state::init_wpc_dot()
{
	uint8_t *fixed = memregion("code")->base();
	uint32_t codeoff = memregion("code")->bytes() - 0x8000;
	m_cpubank->configure_entries(0, 64, &fixed[0], 0x4000);
	m_cpubank->set_entry(0);
	m_fixedbank->configure_entries(0, 1, &fixed[codeoff],0x8000);
	m_fixedbank->set_entry(0);
	m_dmdbanks[0]->configure_entries(0, 16, &m_dmdram[0x0000],0x200);
	m_dmdbanks[0]->set_entry(0);
	m_dmdbanks[1]->configure_entries(0, 16, &m_dmdram[0x0000],0x200);
	m_dmdbanks[1]->set_entry(1);
	m_dmdbanks[2]->configure_entries(0, 16, &m_dmdram[0x0000],0x200);
	m_dmdbanks[2]->set_entry(2);
	m_dmdbanks[3]->configure_entries(0, 16, &m_dmdram[0x0000],0x200);
	m_dmdbanks[3]->set_entry(3);
	m_dmdbanks[4]->configure_entries(0, 16, &m_dmdram[0x0000],0x200);
	m_dmdbanks[4]->set_entry(4);
	m_dmdbanks[5]->configure_entries(0, 16, &m_dmdram[0x0000],0x200);
	m_dmdbanks[5]->set_entry(5);
	m_vblank_timer = timer_alloc(TIMER_VBLANK);
	m_vblank_timer->adjust(attotime::from_hz(60),0,attotime::from_hz(60*4));
	m_irq_timer = timer_alloc(TIMER_IRQ);
	m_irq_timer->adjust(attotime::from_hz(976),0,attotime::from_hz(976));
	m_bankmask = (memregion("code")->bytes() >> 14) - 1;
	logerror("WPC: ROM bank mask = %02x\n",m_bankmask);
	memset(m_ram,0,0x3000);
	memset(m_dmdram,0,0x2000);
	save_pointer(m_dmdram,"DMD RAM",0x2000);
}

uint8_t wpc_dot_state::ram_r(offs_t offset)
{
	return m_ram[offset];
}

void wpc_dot_state::ram_w(offs_t offset, uint8_t data)
{
	if((!m_wpc->memprotect_active()) || ((offset & m_wpc->get_memprotect_mask()) != m_wpc->get_memprotect_mask()))
		m_ram[offset] = data;
	else
		logerror("WPC: Memory protection violation at 0x%04x (mask=0x%04x)\n",offset,m_wpc->get_memprotect_mask());
}

void wpc_dot_state::wpc_rombank_w(uint8_t data)
{
	m_cpubank->set_entry(data & m_bankmask);
}

void wpc_dot_state::wpc_dmdbank_w(offs_t offset, uint8_t data)
{
	uint8_t const bank(offset & 0x07);
	uint8_t const page(offset >> 4);

	switch (bank)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		m_dmdbanks[bank]->set_entry(data + (page << 4));
	}
}

WRITE_LINE_MEMBER(wpc_dot_state::wpcsnd_reply_w)
{
	if(state)
	{
		m_maincpu->set_input_line(M6809_FIRQ_LINE,ASSERT_LINE);
		m_wpc->set_snd_firq();
	}
}

WRITE_LINE_MEMBER(wpc_dot_state::wpc_irq_w)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE,CLEAR_LINE);
}

WRITE_LINE_MEMBER(wpc_dot_state::wpc_firq_w)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE,CLEAR_LINE);
}

uint32_t wpc_dot_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t offset = (m_wpc->get_visible_page() * 0x200);
	uint32_t col[2] { rgb_t(0x00,0x00,0x00), rgb_t(0xff,0xaa,0x00) };

	for(uint8_t y=0;y<32;y++)  // scanline
	{
		for(uint8_t x=0;x<128;x+=8)  // column
		{
			assert(offset >= 0 && offset < std::size(m_dmdram));
			for(uint8_t bit=0;bit<8;bit++)  // bits
				bitmap.pix(y,x+bit) = col[BIT(m_dmdram[offset], bit)];

			offset++;
		}
	}
	return 0;
}

void wpc_dot_state::wpc_dot(machine_config &config)
{
	/* basic machine hardware */
	M6809(config, m_maincpu, 2000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &wpc_dot_state::wpc_dot_map);

	WPCASIC(config, m_wpc, 0);
	m_wpc->irq_callback().set(FUNC(wpc_dot_state::wpc_irq_w));
	m_wpc->firq_callback().set(FUNC(wpc_dot_state::wpc_firq_w));
	m_wpc->bank_write().set(FUNC(wpc_dot_state::wpc_rombank_w));
	m_wpc->sound_ctrl_read().set(m_wpcsnd, FUNC(wpcsnd_device::ctrl_r)); // ack FIRQ?
	m_wpc->sound_ctrl_write().set(m_wpcsnd, FUNC(wpcsnd_device::ctrl_w));
	m_wpc->sound_data_read().set(m_wpcsnd, FUNC(wpcsnd_device::data_r));
	m_wpc->sound_data_write().set(m_wpcsnd, FUNC(wpcsnd_device::data_w));
	m_wpc->dmdbank_write().set(FUNC(wpc_dot_state::wpc_dmdbank_w));

	SPEAKER(config, "speaker").front_center();
	WPCSND(config, m_wpcsnd);
	m_wpcsnd->set_romregion("sound1");
	m_wpcsnd->reply_callback().set(FUNC(wpc_dot_state::wpcsnd_reply_w));
	m_wpcsnd->add_route(ALL_OUTPUTS, "speaker", 1.0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_native_aspect();
	screen.set_size(128, 32);
	screen.set_visarea(0, 128-1, 0, 32-1);
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(wpc_dot_state::screen_update));
}

/*--------------------------
/ Gilligan's Island #20003
/--------------------------*/
ROM_START(gi_l9)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("gilli_l9.rom", 0x00000, 0x40000, CRC(af07a757) SHA1(29c4f4ac2aed5b36e1d22490d656b1c4acba7f4c))
	ROM_REGION(0x180000, "sound1",0)
	ROM_LOAD("gi_u14.l2", 0x000000, 0x20000, CRC(0e7a4140) SHA1(c6408794120b5e45a48b35c380333879e1f0be78))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("gi_u15.l2", 0x080000, 0x20000, CRC(f8241dc9) SHA1(118a65555b9fff6f94e5e8324ed97d6ddec3d82b))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("gi_u18.l2", 0x100000, 0x20000, CRC(ea53e196) SHA1(5dcf3f44d2d658f6a7b130fa9e48d3cd616b4300))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
ROM_END

ROM_START(gi_l3)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("gi_l3.u6", 0x00000, 0x40000, CRC(d4e26140) SHA1(c2a9f02217071768ec1ef9169d2922c0e1585bee))
	ROM_REGION(0x180000, "sound1",0)
	ROM_LOAD("gi_u14.l2", 0x000000, 0x20000, CRC(0e7a4140) SHA1(c6408794120b5e45a48b35c380333879e1f0be78))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("gi_u15.l2", 0x080000, 0x20000, CRC(f8241dc9) SHA1(118a65555b9fff6f94e5e8324ed97d6ddec3d82b))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("gi_u18.l2", 0x100000, 0x20000, CRC(ea53e196) SHA1(5dcf3f44d2d658f6a7b130fa9e48d3cd616b4300))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
ROM_END

ROM_START(gi_l4)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("gi_l4.u6", 0x00000, 0x40000, CRC(2313986d) SHA1(6e0dd293b869ea986ac9cb65b020463a86d955d4))
	ROM_REGION(0x180000, "sound1",0)
	ROM_LOAD("gi_u14.l2", 0x000000, 0x20000, CRC(0e7a4140) SHA1(c6408794120b5e45a48b35c380333879e1f0be78))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("gi_u15.l2", 0x080000, 0x20000, CRC(f8241dc9) SHA1(118a65555b9fff6f94e5e8324ed97d6ddec3d82b))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("gi_u18.l2", 0x100000, 0x20000, CRC(ea53e196) SHA1(5dcf3f44d2d658f6a7b130fa9e48d3cd616b4300))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
ROM_END

ROM_START(gi_l6)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("gi_l6.u6", 0x00000, 0x40000, CRC(7b73eef2) SHA1(fade23019600d84492d5a0fc6f4f5be52ec319be))
	ROM_REGION(0x180000, "sound1",0)
	ROM_LOAD("gi_u14.l2", 0x000000, 0x20000, CRC(0e7a4140) SHA1(c6408794120b5e45a48b35c380333879e1f0be78))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("gi_u15.l2", 0x080000, 0x20000, CRC(f8241dc9) SHA1(118a65555b9fff6f94e5e8324ed97d6ddec3d82b))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("gi_u18.l2", 0x100000, 0x20000, CRC(ea53e196) SHA1(5dcf3f44d2d658f6a7b130fa9e48d3cd616b4300))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
ROM_END

ROM_START(gi_l8)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("gilligans_l8.u6", 0x00000, 0x40000, CRC(d21d3bf8) SHA1(d41447a35b710297786d35aefe235ebd8b354b29))
	ROM_REGION(0x180000, "sound1",0)
	ROM_LOAD("gi_u14.l2", 0x000000, 0x20000, CRC(0e7a4140) SHA1(c6408794120b5e45a48b35c380333879e1f0be78))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("gi_u15.l2", 0x080000, 0x20000, CRC(f8241dc9) SHA1(118a65555b9fff6f94e5e8324ed97d6ddec3d82b))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("gi_u18.l2", 0x100000, 0x20000, CRC(ea53e196) SHA1(5dcf3f44d2d658f6a7b130fa9e48d3cd616b4300))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
ROM_END

/*--------------------------------------------------
/ Hot Shot #60017 (novelty machine - not a pinball)
/--------------------------------------------------*/
ROM_START(hshot_p8)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "code", 0)
	ROM_LOAD("hshot_p8.u6", 0x00000, 0x80000, CRC(26dd6bb2) SHA1(45674885052838b6bd6b3ed0a276a4d9323290c5))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("hshot_l1.u18", 0x100000, 0x20000, CRC(a0e5beba) SHA1(c54a22527d861df54891308752ebdec5829deceb))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
	ROM_LOAD("hshot_l1.u14", 0x000000, 0x80000, CRC(a3ccf557) SHA1(a8e518ea115cd1963544273c45d9ae9a6cab5e1f))
ROM_END

/*-------------------
/  Hurricane #50012
/--------------------*/
ROM_START(hurr_l2)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("hurcnl_2.rom", 0x00000, 0x40000, CRC(fda6155f) SHA1(0088155a2582524d8720d71cd3ff82e8733ef434))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("u14.pp", 0x000000, 0x20000, CRC(51c82899) SHA1(aa6c3d9e7efa3708727b06fb3372638d5245a510))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("u15.pp", 0x080000, 0x20000, CRC(93d02c62) SHA1(203cd6b933822d6d3f70c63e051237e3587568f1))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("u18.pp", 0x100000, 0x20000, CRC(63944b37) SHA1(045f8046ba5bf1c88b65a80737e2d3d017271c04))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
ROM_END

/*--------------------
/  Party Zone #20004
/---------------------*/
ROM_START(pz_f4)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("pzonef_4.rom", 0x00000, 0x40000, CRC(041d7d15) SHA1(d40e7010caa3bc664dc985c748309fe84ae17dac))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("pz_u14.l1", 0x000000, 0x40000, CRC(4d8897ce) SHA1(7a4ac9e849dae93078ddd60adbd34f3930e4cd46))
	ROM_RELOAD( 0x000000 + 0x40000, 0x40000)
	ROM_LOAD("pz_u15.l1", 0x080000, 0x20000, CRC(168bcc52) SHA1(0bae89278cd24950b2e247bba48eaa636f7b566c))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("pz_u18.l1", 0x100000, 0x20000, CRC(b7fbba98) SHA1(6533a1474dd335419331d37d4a4447951171412b))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
ROM_END

ROM_START(pz_l1)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("u6-l1.rom", 0x00000, 0x40000, CRC(48023444) SHA1(0c14f5902c6c0b3466fb4265a2e1fc6a1050f8d7))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("pz_u14.l1", 0x000000, 0x40000, CRC(4d8897ce) SHA1(7a4ac9e849dae93078ddd60adbd34f3930e4cd46))
	ROM_RELOAD( 0x000000 + 0x40000, 0x40000)
	ROM_LOAD("pz_u15.l1", 0x080000, 0x20000, CRC(168bcc52) SHA1(0bae89278cd24950b2e247bba48eaa636f7b566c))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("pz_u18.l1", 0x100000, 0x20000, CRC(b7fbba98) SHA1(6533a1474dd335419331d37d4a4447951171412b))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
ROM_END

ROM_START(pz_l2)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("pz_u6.l2", 0x00000, 0x40000, CRC(200455a9) SHA1(d0f9a2227c67ddc73111a120a6a19dc5ac218baa))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("pz_u14.l1", 0x000000, 0x40000, CRC(4d8897ce) SHA1(7a4ac9e849dae93078ddd60adbd34f3930e4cd46))
	ROM_RELOAD( 0x000000 + 0x40000, 0x40000)
	ROM_LOAD("pz_u15.l1", 0x080000, 0x20000, CRC(168bcc52) SHA1(0bae89278cd24950b2e247bba48eaa636f7b566c))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("pz_u18.l1", 0x100000, 0x20000, CRC(b7fbba98) SHA1(6533a1474dd335419331d37d4a4447951171412b))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
ROM_END

ROM_START(pz_l3)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("pzonel_3.rom", 0x00000, 0x40000, CRC(156f158f) SHA1(73a31deee6b299e5f5479b43210a822009e116d0))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("pz_u14.l1", 0x000000, 0x40000, CRC(4d8897ce) SHA1(7a4ac9e849dae93078ddd60adbd34f3930e4cd46))
	ROM_RELOAD( 0x000000 + 0x40000, 0x40000)
	ROM_LOAD("pz_u15.l1", 0x080000, 0x20000, CRC(168bcc52) SHA1(0bae89278cd24950b2e247bba48eaa636f7b566c))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("pz_u18.l1", 0x100000, 0x20000, CRC(b7fbba98) SHA1(6533a1474dd335419331d37d4a4447951171412b))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
ROM_END

/*-----------------------------------------------------------
/ Slugfest baseball #60001 (novelty machine - not a pinball)
/-----------------------------------------------------------*/
ROM_START(sf_l1)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("sf_u6.l1", 0x00000, 0x40000, CRC(ada93967) SHA1(90094d207dafdacfaf7d259c6cc3dc2b552c8588))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("sf_u14.l1", 0x000000, 0x20000, CRC(b830b419) SHA1(c59980a78d8cb1d979de21dfc5ad3d671d8486e7))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("sf_u15.l1", 0x080000, 0x20000, CRC(adcaeaa1) SHA1(27aa9526c628634c395161f4966d9943bdf1f120))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("sf_u18.l1", 0x100000, 0x20000, CRC(78092c83) SHA1(7c922dfd8be4bb5e23d4c86b6eb18a29cc034338))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
ROM_END

/*-----------------------------------
/  Terminator 2: Judgment Day #50013
/-----------------------------------*/
ROM_START(t2_l8)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "code", 0)
	ROM_LOAD("t2_l8.rom", 0x00000, 0x80000, CRC(c00e52e9) SHA1(830c1a7eabf3c8e4fa6242421587b398e21449e8))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("t2_u14.l3", 0x000000, 0x20000, CRC(9addc9dc) SHA1(847bb027f6b9167cbbaa13f1af50d61e0c69f01f))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("t2_u15.l3", 0x080000, 0x20000, CRC(dad03ad1) SHA1(7c200f9a6564d751e5aa9b1ba84363b221502770))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("t2_u18.l3", 0x100000, 0x20000, CRC(2280bdd0) SHA1(ea94265cb8291ee427e0a2119d901ba1eb50d8ee))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
ROM_END

ROM_START(t2_l6)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("t2_l6.u6", 0x00000, 0x40000, CRC(0d714b35) SHA1(050fd2b3afbecbbd03d58ab206ff6cfac8780a2b))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("t2_u14.l3", 0x000000, 0x20000, CRC(9addc9dc) SHA1(847bb027f6b9167cbbaa13f1af50d61e0c69f01f))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("t2_u15.l3", 0x080000, 0x20000, CRC(dad03ad1) SHA1(7c200f9a6564d751e5aa9b1ba84363b221502770))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("t2_u18.l3", 0x100000, 0x20000, CRC(2280bdd0) SHA1(ea94265cb8291ee427e0a2119d901ba1eb50d8ee))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
ROM_END

ROM_START(t2_p2f)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("u6-nasty.rom", 0x00000, 0x40000, CRC(add685a4) SHA1(d1ee7eb620864b017495e52ea8fe8db18508c3eb))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("u14-nsty.rom", 0x000000, 0x20000, CRC(b4d64152) SHA1(03a828cef8b067d4da058fd3a1e972265a72f10a))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("t2_u15.l3", 0x080000, 0x20000, CRC(dad03ad1) SHA1(7c200f9a6564d751e5aa9b1ba84363b221502770))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("t2_u18.l3", 0x100000, 0x20000, CRC(2280bdd0) SHA1(ea94265cb8291ee427e0a2119d901ba1eb50d8ee))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
ROM_END

ROM_START(t2_l4)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("u6-l4.rom", 0x00000, 0x40000, CRC(4d8b894d) SHA1(218b3628e7709c329c2030a5391ded60301aad26))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("t2_u14.l3", 0x000000, 0x20000, CRC(9addc9dc) SHA1(847bb027f6b9167cbbaa13f1af50d61e0c69f01f))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("t2_u15.l3", 0x080000, 0x20000, CRC(dad03ad1) SHA1(7c200f9a6564d751e5aa9b1ba84363b221502770))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("t2_u18.l3", 0x100000, 0x20000, CRC(2280bdd0) SHA1(ea94265cb8291ee427e0a2119d901ba1eb50d8ee))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
ROM_END

ROM_START(t2_l3)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("u6-l3.rom", 0x00000, 0x40000, CRC(7520398a) SHA1(862881481dc7b617f3b14bbb35d48cffb0ce950e))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("t2_u14.l3", 0x000000, 0x20000, CRC(9addc9dc) SHA1(847bb027f6b9167cbbaa13f1af50d61e0c69f01f))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("t2_u15.l3", 0x080000, 0x20000, CRC(dad03ad1) SHA1(7c200f9a6564d751e5aa9b1ba84363b221502770))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("t2_u18.l3", 0x100000, 0x20000, CRC(2280bdd0) SHA1(ea94265cb8291ee427e0a2119d901ba1eb50d8ee))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
ROM_END

ROM_START(t2_l2)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("u6-l2.rom", 0x00000, 0x40000, CRC(efe49c18) SHA1(9f91081c384990eac6e3c57f318a2639626929f9))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("t2_u14.l3", 0x000000, 0x20000, CRC(9addc9dc) SHA1(847bb027f6b9167cbbaa13f1af50d61e0c69f01f))
	ROM_RELOAD( 0x000000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x000000 + 0x60000, 0x20000)
	ROM_LOAD("t2_u15.l3", 0x080000, 0x20000, CRC(dad03ad1) SHA1(7c200f9a6564d751e5aa9b1ba84363b221502770))
	ROM_RELOAD( 0x080000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x080000 + 0x60000, 0x20000)
	ROM_LOAD("t2_u18.l3", 0x100000, 0x20000, CRC(2280bdd0) SHA1(ea94265cb8291ee427e0a2119d901ba1eb50d8ee))
	ROM_RELOAD( 0x100000 + 0x20000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x40000, 0x20000)
	ROM_RELOAD( 0x100000 + 0x60000, 0x20000)
ROM_END

/*-------------------------------------
/ Test Fixture DMD generation (#584-T)
/-------------------------------------*/
ROM_START(tfdmd_l3)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x20000, "code", 0)
	ROM_LOAD("u6_l3.rom", 0x00000, 0x20000, CRC(bd43e28c) SHA1(df0a64a9fddbc59e3edde56ae12b68f76e44ba2e))
	ROM_REGION(0x180000, "sound1", ROMREGION_ERASE00)
ROM_END


GAME(1991,  tfdmd_l3,   0,      wpc_dot,    wpc_dot, wpc_dot_state, init_wpc_dot, ROT0, "Bally",        "WPC Test Fixture: DMD (L-3)",                  MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  gi_l9,      0,      wpc_dot,    wpc_dot, wpc_dot_state, init_wpc_dot, ROT0, "Bally",        "Gilligan's Island (L-9)",                      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  gi_l3,      gi_l9,  wpc_dot,    wpc_dot, wpc_dot_state, init_wpc_dot, ROT0, "Bally",        "Gilligan's Island (L-3)",                      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  gi_l4,      gi_l9,  wpc_dot,    wpc_dot, wpc_dot_state, init_wpc_dot, ROT0, "Bally",        "Gilligan's Island (L-4)",                      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  gi_l6,      gi_l9,  wpc_dot,    wpc_dot, wpc_dot_state, init_wpc_dot, ROT0, "Bally",        "Gilligan's Island (L-6)",                      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  gi_l8,      gi_l9,  wpc_dot,    wpc_dot, wpc_dot_state, init_wpc_dot, ROT0, "Bally",        "Gilligan's Island (L-8)",                      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  hshot_p8,   0,      wpc_dot,    wpc_dot, wpc_dot_state, init_wpc_dot, ROT0, "Midway",       "Hot Shot Basketball (P-8)",                    MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  hurr_l2,    0,      wpc_dot,    wpc_dot, wpc_dot_state, init_wpc_dot, ROT0, "Williams",     "Hurricane (L-2)",                              MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  pz_f4,      0,      wpc_dot,    wpc_dot, wpc_dot_state, init_wpc_dot, ROT0, "Bally",        "The Party Zone (F-4)",                         MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  pz_l1,      pz_f4,  wpc_dot,    wpc_dot, wpc_dot_state, init_wpc_dot, ROT0, "Bally",        "The Party Zone (L-1)",                         MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  pz_l2,      pz_f4,  wpc_dot,    wpc_dot, wpc_dot_state, init_wpc_dot, ROT0, "Bally",        "The Party Zone (L-2)",                         MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  pz_l3,      pz_f4,  wpc_dot,    wpc_dot, wpc_dot_state, init_wpc_dot, ROT0, "Bally",        "The Party Zone (L-3)",                         MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  sf_l1,      0,      wpc_dot,    wpc_dot, wpc_dot_state, init_wpc_dot, ROT0, "Williams",     "Slugfest (L-1)",                               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  t2_l8,      0,      wpc_dot,    wpc_dot, wpc_dot_state, init_wpc_dot, ROT0, "Williams",     "Terminator 2: Judgment Day (L-8)",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  t2_l6,      t2_l8,  wpc_dot,    wpc_dot, wpc_dot_state, init_wpc_dot, ROT0, "Williams",     "Terminator 2: Judgment Day (L-6)",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  t2_p2f,     t2_l8,  wpc_dot,    wpc_dot, wpc_dot_state, init_wpc_dot, ROT0, "Williams",     "Terminator 2: Judgment Day (P-2F) Profanity",  MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  t2_l4,      t2_l8,  wpc_dot,    wpc_dot, wpc_dot_state, init_wpc_dot, ROT0, "Williams",     "Terminator 2: Judgment Day (L-4)",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  t2_l3,      t2_l8,  wpc_dot,    wpc_dot, wpc_dot_state, init_wpc_dot, ROT0, "Williams",     "Terminator 2: Judgment Day (L-3)",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  t2_l2,      t2_l8,  wpc_dot,    wpc_dot, wpc_dot_state, init_wpc_dot, ROT0, "Williams",     "Terminator 2: Judgment Day (L-2)",             MACHINE_IS_SKELETON_MECHANICAL)
