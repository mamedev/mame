// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

Asterix

TODO:
 - the konami logo: in the original the outline is drawn, then there's a slight
   delay of 1 or 2 seconds, then it fills from the top to the bottom with the
   colour, including the word "Konami"
 - Verify clocks, PCB has 2 OSCs. 32MHz & 24MHz

***************************************************************************/

#include "emu.h"
#include "includes/asterix.h"
#include "includes/konamipt.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "sound/k053260.h"
#include "sound/ymopm.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


#if 0
uint16_t asterix_state::control2_r()
{
	return m_cur_control2;
}
#endif

void asterix_state::control2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_cur_control2 = data;
		/* bit 0 is data */
		/* bit 1 is cs (active low) */
		/* bit 2 is clock (active high) */
		ioport("EEPROMOUT")->write(data, 0xff);

		/* bit 5 is select tile bank */
		m_k056832->set_tile_bank((data & 0x20) >> 5);
		// TODO: looks like 0xffff is used from time to time for chip selection/reset something, not unlike Jackal
		if((data & 0xff) != 0xff)
		{
			machine().bookkeeping().coin_counter_w(0, data & 0x08);
			machine().bookkeeping().coin_counter_w(1, data & 0x10);
			machine().bookkeeping().coin_lockout_w(0, data & 0x40);
			machine().bookkeeping().coin_lockout_w(1, data & 0x80);
		}
	}
}

INTERRUPT_GEN_MEMBER(asterix_state::asterix_interrupt)
{
	// global interrupt masking
	if (!m_k056832->is_irq_enabled(0))
		return;

	device.execute().set_input_line(5, HOLD_LINE); /* ??? All irqs have the same vector, and the mask used is 0 or 7 */
}

void asterix_state::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TIMER_NMI:
		m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		break;
	default:
		throw emu_fatalerror("Unknown id in asterix_state::device_timer");
	}
}

void asterix_state::sound_arm_nmi_w(uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	timer_set(attotime::from_usec(5), TIMER_NMI);
}

void asterix_state::sound_irq_w(uint16_t data)
{
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

// Check the routine at 7f30 in the ead version.
// You're not supposed to laugh.
// This emulation is grossly overkill but hey, I'm having fun.
#if 0
void asterix_state::protection_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(m_prot + offset);

	if (offset == 1)
	{
		uint32_t cmd = (m_prot[0] << 16) | m_prot[1];
		switch (cmd >> 24)
		{
		case 0x64:
			{
			uint32_t param1 = (read_word(cmd & 0xffffff) << 16) | read_word((cmd & 0xffffff) + 2);
			uint32_t param2 = (read_word((cmd & 0xffffff) + 4) << 16) | read_word((cmd & 0xffffff) + 6);

			switch (param1 >> 24)
			{
				case 0x22:
				{
					int size = param2 >> 24;
					param1 &= 0xffffff;
					param2 &= 0xffffff;
					while(size >= 0)
					{
						write_word(param2, read_word(param1));
						param1 += 2;
						param2 += 2;
						size--;
					}
				break;
				}
			}
			break;
			}
		}
	}
}
#endif

void asterix_state::protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(m_prot + offset);

	if (offset == 1)
	{
		uint32_t cmd = (m_prot[0] << 16) | m_prot[1];
		switch (cmd >> 24)
		{
		case 0x64:
		{
			uint32_t param1 = (space.read_word(cmd & 0xffffff) << 16)
				| space.read_word((cmd & 0xffffff) + 2);
			uint32_t param2 = (space.read_word((cmd & 0xffffff) + 4) << 16)
				| space.read_word((cmd & 0xffffff) + 6);

			switch (param1 >> 24)
			{
			case 0x22:
			{
				int size = param2 >> 24;
				param1 &= 0xffffff;
				param2 &= 0xffffff;
				while(size >= 0)
				{
					space.write_word(param2, space.read_word(param1));
					param1 += 2;
					param2 += 2;
					size--;
				}
				break;
			}
			}
			break;
		}
		}
	}
}

void asterix_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x107fff).ram();
	map(0x180000, 0x1807ff).rw(m_k053244, FUNC(k05324x_device::k053245_word_r), FUNC(k05324x_device::k053245_word_w));
	map(0x180800, 0x180fff).ram();                             // extra RAM, or mirror for the above?
	map(0x200000, 0x20000f).rw(m_k053244, FUNC(k05324x_device::k053244_r), FUNC(k05324x_device::k053244_w));
	map(0x280000, 0x280fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x300000, 0x30001f).rw(m_k053244, FUNC(k05324x_device::k053244_r), FUNC(k05324x_device::k053244_w)).umask16(0x00ff);
	map(0x380000, 0x380001).portr("IN0");
	map(0x380002, 0x380003).portr("IN1");
	map(0x380100, 0x380101).w(FUNC(asterix_state::control2_w));
	map(0x380200, 0x380203).rw("k053260", FUNC(k053260_device::main_read), FUNC(k053260_device::main_write)).umask16(0x00ff);
	map(0x380300, 0x380301).w(FUNC(asterix_state::sound_irq_w));
	map(0x380400, 0x380401).w(FUNC(asterix_state::asterix_spritebank_w));
	map(0x380500, 0x38051f).w(m_k053251, FUNC(k053251_device::write)).umask16(0x00ff);
	map(0x380600, 0x380601).noprw();                             // Watchdog
	map(0x380700, 0x380707).w(m_k056832, FUNC(k056832_device::b_word_w));
	map(0x380800, 0x380803).w(FUNC(asterix_state::protection_w));
	map(0x400000, 0x400fff).rw(m_k056832, FUNC(k056832_device::ram_half_word_r), FUNC(k056832_device::ram_half_word_w));
	map(0x420000, 0x421fff).r(m_k056832, FUNC(k056832_device::old_rom_word_r));   // Passthrough to tile roms
	map(0x440000, 0x44003f).w(m_k056832, FUNC(k056832_device::word_w));
}

void asterix_state::sound_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf801, 0xf801).rw("ymsnd", FUNC(ym2151_device::status_r), FUNC(ym2151_device::data_w));
	map(0xfa00, 0xfa2f).rw("k053260", FUNC(k053260_device::read), FUNC(k053260_device::write));
	map(0xfc00, 0xfc00).w(FUNC(asterix_state::sound_arm_nmi_w));
	map(0xfe00, 0xfe00).w("ymsnd", FUNC(ym2151_device::address_w));
}



static INPUT_PORTS_START( asterix )
	PORT_START("IN0")
	KONAMI16_LSB(1, IPT_UNKNOWN, IPT_START1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	KONAMI16_LSB(2, IPT_UNKNOWN, IPT_START2)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, do_read)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, ready_read)
	PORT_SERVICE_NO_TOGGLE(0x0400, IP_ACTIVE_LOW )
	PORT_BIT( 0xf800, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, di_write)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, cs_write)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, clk_write)
INPUT_PORTS_END


void asterix_state::machine_start()
{
	save_item(NAME(m_cur_control2));
	save_item(NAME(m_prot));

	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_spritebank));
	save_item(NAME(m_layerpri));
	save_item(NAME(m_layer_colorbase));
	save_item(NAME(m_tilebanks));
	save_item(NAME(m_spritebanks));
}

void asterix_state::machine_reset()
{
	m_cur_control2 = 0;
	m_prot[0] = 0;
	m_prot[1] = 0;

	m_sprite_colorbase = 0;
	m_spritebank = 0;
	m_layerpri[0] = 0;
	m_layerpri[1] = 0;
	m_layerpri[2] = 0;

	for (int i = 0; i < 4; i++)
	{
		m_layer_colorbase[i] = 0;
		m_tilebanks[i] = 0;
		m_spritebanks[i] = 0;
	}
}

void asterix_state::asterix(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000)/2); // 12MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &asterix_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(asterix_state::asterix_interrupt));

	Z80(config, m_audiocpu, XTAL(32'000'000)/4); // 8MHz Z80E ??
	m_audiocpu->set_addrmap(AS_PROGRAM, &asterix_state::sound_map);

	EEPROM_ER5911_8BIT(config, "eeprom");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(14*8, (64-14)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(asterix_state::screen_update_asterix));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 2048).enable_shadows();

	K056832(config, m_k056832, 0);
	m_k056832->set_tile_callback(FUNC(asterix_state::tile_callback));
	m_k056832->set_config(K056832_BPP_4, 1, 1);
	m_k056832->set_palette("palette");

	K053244(config, m_k053244, 0);
	m_k053244->set_palette("palette");
	m_k053244->set_offsets(-3, -1);
	m_k053244->set_sprite_callback(FUNC(asterix_state::sprite_callback));

	K053251(config, m_k053251, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	YM2151(config, "ymsnd", XTAL(32'000'000)/8).add_route(0, "lspeaker", 1.0).add_route(1, "rspeaker", 1.0); // 4MHz

	k053260_device &k053260(K053260(config, "k053260", XTAL(32'000'000)/8)); // 4MHz
	k053260.add_route(0, "lspeaker", 0.75);
	k053260.add_route(1, "rspeaker", 0.75);
}


ROM_START( asterix )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "068_ea_d01.8c", 0x000000,  0x20000, CRC(61d6621d) SHA1(908a344e9bbce0c7544bd049494258d1d3ad073b) )
	ROM_LOAD16_BYTE( "068_ea_d02.8d", 0x000001,  0x20000, CRC(53aac057) SHA1(7401ca5b70f384688c3353fc1ac9ef0b27814c66) )
	ROM_LOAD16_BYTE( "068a03.7c", 0x080000,  0x20000, CRC(8223ebdc) SHA1(e4aa39e4bc1d210bdda5b0cb41d6c8006c48dd24) )
	ROM_LOAD16_BYTE( "068a04.7d", 0x080001,  0x20000, CRC(9f351828) SHA1(e03842418f08e6267eeea03362450da249af73be) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "068_a05.5f", 0x000000, 0x010000,  CRC(d3d0d77b) SHA1(bfa77a8bf651dc27f481e96a2d63242084cc214c) )

	ROM_REGION( 0x100000, "k056832", 0 )
	ROM_LOAD32_WORD( "068a12.16k", 0x000000, 0x080000, CRC(b9da8e9c) SHA1(a46878916833923e421da0667e37620ae0b77744) )
	ROM_LOAD32_WORD( "068a11.12k", 0x000002, 0x080000, CRC(7eb07a81) SHA1(672c0c60834df7816d33d88643e4575b8ca9bcc1) )

	ROM_REGION( 0x400000, "k053244", 0 )
	ROM_LOAD32_WORD( "068a08.7k", 0x000000, 0x200000, CRC(c41278fe) SHA1(58e5f67a67ae97e0b264489828cd7e74662c5ed5) )
	ROM_LOAD32_WORD( "068a07.3k", 0x000002, 0x200000, CRC(32efdbc4) SHA1(b7e8610aa22249176d82b750e2549d1eea6abe4f) )

	ROM_REGION( 0x200000, "k053260", 0 )
	ROM_LOAD( "068a06.1e", 0x000000, 0x200000, CRC(6df9ec0e) SHA1(cee60312e9813bd6579f3ac7c3c2521a8e633eca) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "asterix.nv", 0x0000, 0x0080, CRC(490085c8) SHA1(2a79e7c79db4b4fb0e6a7249cfd6a57e74b170e3) )
ROM_END

ROM_START( asterixeac )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "068_ea_c01.8c", 0x000000,  0x20000, CRC(0ccd1feb) SHA1(016d642e3a745f0564aa93f0f66d5c0f37962990) )
	ROM_LOAD16_BYTE( "068_ea_c02.8d", 0x000001,  0x20000, CRC(b0805f47) SHA1(b58306164e8fec69002656993ae80abbc8f136cd) )
	ROM_LOAD16_BYTE( "068a03.7c", 0x080000,  0x20000, CRC(8223ebdc) SHA1(e4aa39e4bc1d210bdda5b0cb41d6c8006c48dd24) )
	ROM_LOAD16_BYTE( "068a04.7d", 0x080001,  0x20000, CRC(9f351828) SHA1(e03842418f08e6267eeea03362450da249af73be) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "068_a05.5f", 0x000000, 0x010000,  CRC(d3d0d77b) SHA1(bfa77a8bf651dc27f481e96a2d63242084cc214c) )

	ROM_REGION( 0x100000, "k056832", 0 )
	ROM_LOAD32_WORD( "068a12.16k", 0x000000, 0x080000, CRC(b9da8e9c) SHA1(a46878916833923e421da0667e37620ae0b77744) )
	ROM_LOAD32_WORD( "068a11.12k", 0x000002, 0x080000, CRC(7eb07a81) SHA1(672c0c60834df7816d33d88643e4575b8ca9bcc1) )

	ROM_REGION( 0x400000, "k053244", 0 )
	ROM_LOAD32_WORD( "068a08.7k", 0x000000, 0x200000, CRC(c41278fe) SHA1(58e5f67a67ae97e0b264489828cd7e74662c5ed5) )
	ROM_LOAD32_WORD( "068a07.3k", 0x000002, 0x200000, CRC(32efdbc4) SHA1(b7e8610aa22249176d82b750e2549d1eea6abe4f) )

	ROM_REGION( 0x200000, "k053260", 0 )
	ROM_LOAD( "068a06.1e", 0x000000, 0x200000, CRC(6df9ec0e) SHA1(cee60312e9813bd6579f3ac7c3c2521a8e633eca) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "asterixeac.nv", 0x0000, 0x0080, CRC(490085c8) SHA1(2a79e7c79db4b4fb0e6a7249cfd6a57e74b170e3) )
ROM_END

ROM_START( asterixeaa )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "068_ea_a01.8c", 0x000000,  0x20000, CRC(85b41d8e) SHA1(e1326f6d61b8097f5201d5bd37e4d2a357d17b47) )
	ROM_LOAD16_BYTE( "068_ea_a02.8d", 0x000001,  0x20000, CRC(8e886305) SHA1(41a9de2cdad8c1185b4d13ea5b4a9309716947c5) )
	ROM_LOAD16_BYTE( "068a03.7c", 0x080000,  0x20000, CRC(8223ebdc) SHA1(e4aa39e4bc1d210bdda5b0cb41d6c8006c48dd24) )
	ROM_LOAD16_BYTE( "068a04.7d", 0x080001,  0x20000, CRC(9f351828) SHA1(e03842418f08e6267eeea03362450da249af73be) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "068_a05.5f", 0x000000, 0x010000,  CRC(d3d0d77b) SHA1(bfa77a8bf651dc27f481e96a2d63242084cc214c) )

	ROM_REGION( 0x100000, "k056832", 0 )
	ROM_LOAD32_WORD( "068a12.16k", 0x000000, 0x080000, CRC(b9da8e9c) SHA1(a46878916833923e421da0667e37620ae0b77744) )
	ROM_LOAD32_WORD( "068a11.12k", 0x000002, 0x080000, CRC(7eb07a81) SHA1(672c0c60834df7816d33d88643e4575b8ca9bcc1) )

	ROM_REGION( 0x400000, "k053244", 0 )
	ROM_LOAD32_WORD( "068a08.7k", 0x000000, 0x200000, CRC(c41278fe) SHA1(58e5f67a67ae97e0b264489828cd7e74662c5ed5) )
	ROM_LOAD32_WORD( "068a07.3k", 0x000002, 0x200000, CRC(32efdbc4) SHA1(b7e8610aa22249176d82b750e2549d1eea6abe4f) )

	ROM_REGION( 0x200000, "k053260", 0 )
	ROM_LOAD( "068a06.1e", 0x000000, 0x200000, CRC(6df9ec0e) SHA1(cee60312e9813bd6579f3ac7c3c2521a8e633eca) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "asterixeaa.nv", 0x0000, 0x0080, CRC(30275de0) SHA1(4bbf90a4e5b20406153329e9e7c4c2bf72676f8d) )
ROM_END

ROM_START( asterixaad )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "068_aa_d01.8c", 0x000000,  0x20000, CRC(3fae5f1f) SHA1(73ef65dac8e1cd4d9a3695963231e3a2a860b486) )
	ROM_LOAD16_BYTE( "068_aa_d02.8d", 0x000001,  0x20000, CRC(171f0ba0) SHA1(1665f23194da5811e4708ad0495378957b6e6251) )
	ROM_LOAD16_BYTE( "068a03.7c", 0x080000,  0x20000, CRC(8223ebdc) SHA1(e4aa39e4bc1d210bdda5b0cb41d6c8006c48dd24) )
	ROM_LOAD16_BYTE( "068a04.7d", 0x080001,  0x20000, CRC(9f351828) SHA1(e03842418f08e6267eeea03362450da249af73be) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "068_a05.5f", 0x000000, 0x010000,  CRC(d3d0d77b) SHA1(bfa77a8bf651dc27f481e96a2d63242084cc214c) )

	ROM_REGION( 0x100000, "k056832", 0 )
	ROM_LOAD32_WORD( "068a12.16k", 0x000000, 0x080000, CRC(b9da8e9c) SHA1(a46878916833923e421da0667e37620ae0b77744) )
	ROM_LOAD32_WORD( "068a11.12k", 0x000002, 0x080000, CRC(7eb07a81) SHA1(672c0c60834df7816d33d88643e4575b8ca9bcc1) )

	ROM_REGION( 0x400000, "k053244", 0 )
	ROM_LOAD32_WORD( "068a08.7k", 0x000000, 0x200000, CRC(c41278fe) SHA1(58e5f67a67ae97e0b264489828cd7e74662c5ed5) )
	ROM_LOAD32_WORD( "068a07.3k", 0x000002, 0x200000, CRC(32efdbc4) SHA1(b7e8610aa22249176d82b750e2549d1eea6abe4f) )

	ROM_REGION( 0x200000, "k053260", 0 )
	ROM_LOAD( "068a06.1e", 0x000000, 0x200000, CRC(6df9ec0e) SHA1(cee60312e9813bd6579f3ac7c3c2521a8e633eca) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "asterixaad.nv", 0x0000, 0x0080, CRC(bcca86a7) SHA1(1191b0011749e2516df723c9d63da9c2304fa594) )
ROM_END

ROM_START( asterixj )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "068_ja_d01.8c", 0x000000,  0x20000, CRC(2bc10940) SHA1(e25cc97435f157bed9c28d9e9277c9f47d4fb5fb) )
	ROM_LOAD16_BYTE( "068_ja_d02.8d", 0x000001,  0x20000, CRC(de438300) SHA1(8d72988409e6c28a06fb2325087d27ebd2d02c92) )
	ROM_LOAD16_BYTE( "068a03.7c", 0x080000,  0x20000, CRC(8223ebdc) SHA1(e4aa39e4bc1d210bdda5b0cb41d6c8006c48dd24) )
	ROM_LOAD16_BYTE( "068a04.7d", 0x080001,  0x20000, CRC(9f351828) SHA1(e03842418f08e6267eeea03362450da249af73be) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "068_a05.5f", 0x000000, 0x010000,  CRC(d3d0d77b) SHA1(bfa77a8bf651dc27f481e96a2d63242084cc214c) )

	ROM_REGION( 0x100000, "k056832", 0 )
	ROM_LOAD32_WORD( "068a12.16k", 0x000000, 0x080000, CRC(b9da8e9c) SHA1(a46878916833923e421da0667e37620ae0b77744) )
	ROM_LOAD32_WORD( "068a11.12k", 0x000002, 0x080000, CRC(7eb07a81) SHA1(672c0c60834df7816d33d88643e4575b8ca9bcc1) )

	ROM_REGION( 0x400000, "k053244", 0 )
	ROM_LOAD32_WORD( "068a08.7k", 0x000000, 0x200000, CRC(c41278fe) SHA1(58e5f67a67ae97e0b264489828cd7e74662c5ed5) )
	ROM_LOAD32_WORD( "068a07.3k", 0x000002, 0x200000, CRC(32efdbc4) SHA1(b7e8610aa22249176d82b750e2549d1eea6abe4f) )

	ROM_REGION( 0x200000, "k053260", 0 )
	ROM_LOAD( "068a06.1e", 0x000000, 0x200000, CRC(6df9ec0e) SHA1(cee60312e9813bd6579f3ac7c3c2521a8e633eca) )

	ROM_REGION( 0x80, "eeprom", 0 )
	ROM_LOAD( "asterixj.nv", 0x0000, 0x0080, CRC(84229f2c) SHA1(34c7491c731fbf741dfd53bfc559d91201ccfb03) )
ROM_END


void asterix_state::init_asterix()
{
#if 0
	*(uint16_t *)(memregion("maincpu")->base() + 0x07f34) = 0x602a;
	*(uint16_t *)(memregion("maincpu")->base() + 0x00008) = 0x0400;
#endif
}


GAME( 1992, asterix,    0,       asterix, asterix, asterix_state, init_asterix, ROT0, "Konami", "Asterix (ver EAD)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1992, asterixeac, asterix, asterix, asterix, asterix_state, init_asterix, ROT0, "Konami", "Asterix (ver EAC)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1992, asterixeaa, asterix, asterix, asterix, asterix_state, init_asterix, ROT0, "Konami", "Asterix (ver EAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1992, asterixaad, asterix, asterix, asterix, asterix_state, init_asterix, ROT0, "Konami", "Asterix (ver AAD)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1992, asterixj,   asterix, asterix, asterix, asterix_state, init_asterix, ROT0, "Konami", "Asterix (ver JAD)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
