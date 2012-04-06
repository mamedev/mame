/***************************************************************************

Asterix

TODO:
the konami logo: in the original the outline is drawn, then there's a slight
delay of 1 or 2 seconds, then it fills from the top to the bottom with the
colour, including the word "Konami"

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/eeprom.h"
#include "sound/2151intf.h"
#include "sound/k053260.h"
#include "video/konicdev.h"
#include "includes/konamipt.h"
#include "includes/asterix.h"

static const eeprom_interface eeprom_intf =
{
	7,				/* address bits */
	8,				/* data bits */
	"111000",		/*  read command */
	"111100",		/* write command */
	"1100100000000",/* erase command */
	"1100000000000",/* lock command */
	"1100110000000" /* unlock command */
};

#if 0
READ16_MEMBER(asterix_state::control2_r)
{
	return m_cur_control2;
}
#endif

WRITE16_MEMBER(asterix_state::control2_w)
{

	if (ACCESSING_BITS_0_7)
	{
		m_cur_control2 = data;
		/* bit 0 is data */
		/* bit 1 is cs (active low) */
		/* bit 2 is clock (active high) */
		input_port_write(machine(), "EEPROMOUT", data, 0xff);

		/* bit 5 is select tile bank */
		k056832_set_tile_bank(m_k056832, (data & 0x20) >> 5);
	}
}

static INTERRUPT_GEN( asterix_interrupt )
{
	asterix_state *state = device->machine().driver_data<asterix_state>();

	// global interrupt masking
	if (!k056832_is_irq_enabled(state->m_k056832, 0))
		return;

	device_set_input_line(device, 5, HOLD_LINE); /* ??? All irqs have the same vector, and the mask used is 0 or 7 */
}

static READ8_DEVICE_HANDLER( asterix_sound_r )
{
	return k053260_r(device, 2 + offset);
}

static TIMER_CALLBACK( nmi_callback )
{
	asterix_state *state = machine.driver_data<asterix_state>();
	device_set_input_line(state->m_audiocpu, INPUT_LINE_NMI, ASSERT_LINE);
}

WRITE8_MEMBER(asterix_state::sound_arm_nmi_w)
{

	device_set_input_line(m_audiocpu, INPUT_LINE_NMI, CLEAR_LINE);
	machine().scheduler().timer_set(attotime::from_usec(5), FUNC(nmi_callback));
}

WRITE16_MEMBER(asterix_state::sound_irq_w)
{
	device_set_input_line(m_audiocpu, 0, HOLD_LINE);
}

// Check the routine at 7f30 in the ead version.
// You're not supposed to laugh.
// This emulation is grossly overkill but hey, I'm having fun.
#if 0
WRITE16_MEMBER(asterix_state::protection_w)
{
	COMBINE_DATA(m_prot + offset);

	if (offset == 1)
	{
		UINT32 cmd = (m_prot[0] << 16) | m_prot[1];
		switch (cmd >> 24)
		{
		case 0x64:
			{
			UINT32 param1 = (read_word(cmd & 0xffffff) << 16) | read_word((cmd & 0xffffff) + 2);
			UINT32 param2 = (read_word((cmd & 0xffffff) + 4) << 16) | read_word((cmd & 0xffffff) + 6);

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

WRITE16_MEMBER(asterix_state::protection_w)
{
	COMBINE_DATA(m_prot + offset);

	if (offset == 1)
	{
		UINT32 cmd = (m_prot[0] << 16) | m_prot[1];
		switch (cmd >> 24)
		{
		case 0x64:
		{
			UINT32 param1 = (space.read_word(cmd & 0xffffff) << 16)
				| space.read_word((cmd & 0xffffff) + 2);
			UINT32 param2 = (space.read_word((cmd & 0xffffff) + 4) << 16)
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

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, asterix_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x107fff) AM_RAM
	AM_RANGE(0x180000, 0x1807ff) AM_DEVREADWRITE_LEGACY("k053244", k053245_word_r, k053245_word_w)
	AM_RANGE(0x180800, 0x180fff) AM_RAM								// extra RAM, or mirror for the above?
	AM_RANGE(0x200000, 0x20000f) AM_DEVREADWRITE_LEGACY("k053244", k053244_word_r, k053244_word_w)
	AM_RANGE(0x280000, 0x280fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_SHARE("paletteram")
	AM_RANGE(0x300000, 0x30001f) AM_DEVREADWRITE_LEGACY("k053244", k053244_lsb_r, k053244_lsb_w)
	AM_RANGE(0x380000, 0x380001) AM_READ_PORT("IN0")
	AM_RANGE(0x380002, 0x380003) AM_READ_PORT("IN1")
	AM_RANGE(0x380100, 0x380101) AM_WRITE(control2_w)
	AM_RANGE(0x380200, 0x380203) AM_DEVREADWRITE8_LEGACY("k053260", asterix_sound_r, k053260_w, 0x00ff)
	AM_RANGE(0x380300, 0x380301) AM_WRITE(sound_irq_w)
	AM_RANGE(0x380400, 0x380401) AM_WRITE(asterix_spritebank_w)
	AM_RANGE(0x380500, 0x38051f) AM_DEVWRITE_LEGACY("k053251", k053251_lsb_w)
	AM_RANGE(0x380600, 0x380601) AM_NOP								// Watchdog
	AM_RANGE(0x380700, 0x380707) AM_DEVWRITE_LEGACY("k056832", k056832_b_word_w)
	AM_RANGE(0x380800, 0x380803) AM_WRITE(protection_w)
	AM_RANGE(0x400000, 0x400fff) AM_DEVREADWRITE_LEGACY("k056832", k056832_ram_half_word_r, k056832_ram_half_word_w)
	AM_RANGE(0x420000, 0x421fff) AM_DEVREAD_LEGACY("k056832", k056832_old_rom_word_r)	// Passthrough to tile roms
	AM_RANGE(0x440000, 0x44003f) AM_DEVWRITE_LEGACY("k056832", k056832_word_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, asterix_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf801, 0xf801) AM_DEVREADWRITE_LEGACY("ymsnd", ym2151_status_port_r, ym2151_data_port_w)
	AM_RANGE(0xfa00, 0xfa2f) AM_DEVREADWRITE_LEGACY("k053260", k053260_r, k053260_w)
	AM_RANGE(0xfc00, 0xfc00) AM_WRITE(sound_arm_nmi_w)
	AM_RANGE(0xfe00, 0xfe00) AM_DEVWRITE_LEGACY("ymsnd", ym2151_register_port_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( asterix )
	PORT_START("IN0")
	KONAMI16_LSB(1, IPT_UNKNOWN, IPT_START1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	KONAMI16_LSB(2, IPT_UNKNOWN, IPT_START2)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_device, read_bit)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_UNUSED )	// EEPROM ready (always 1)
	PORT_SERVICE_NO_TOGGLE(0x0400, IP_ACTIVE_LOW )
	PORT_BIT( 0xf800, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, write_bit)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_cs_line)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_clock_line)
INPUT_PORTS_END


static const k056832_interface asterix_k056832_intf =
{
	"gfx1", 0,
	K056832_BPP_4,
	1, 1,
	KONAMI_ROM_DEINTERLEAVE_2,
	asterix_tile_callback, "none"
};

static const k05324x_interface asterix_k05324x_intf =
{
	"gfx2", 1,
	NORMAL_PLANE_ORDER,
	-3, -1,
	KONAMI_ROM_DEINTERLEAVE_2,
	asterix_sprite_callback
};

static MACHINE_START( asterix )
{
	asterix_state *state = machine.driver_data<asterix_state>();

	state->m_maincpu = machine.device("maincpu");
	state->m_audiocpu = machine.device("audiocpu");
	state->m_k053260 = machine.device("k053260");
	state->m_k056832 = machine.device("k056832");
	state->m_k053244 = machine.device("k053244");
	state->m_k053251 = machine.device("k053251");

	state->save_item(NAME(state->m_cur_control2));
	state->save_item(NAME(state->m_prot));

	state->save_item(NAME(state->m_sprite_colorbase));
	state->save_item(NAME(state->m_spritebank));
	state->save_item(NAME(state->m_layerpri));
	state->save_item(NAME(state->m_layer_colorbase));
	state->save_item(NAME(state->m_tilebanks));
	state->save_item(NAME(state->m_spritebanks));
}

static MACHINE_RESET( asterix )
{
	asterix_state *state = machine.driver_data<asterix_state>();
	int i;

	state->m_cur_control2 = 0;
	state->m_prot[0] = 0;
	state->m_prot[1] = 0;

	state->m_sprite_colorbase = 0;
	state->m_spritebank = 0;
	state->m_layerpri[0] = 0;
	state->m_layerpri[1] = 0;
	state->m_layerpri[2] = 0;

	for (i = 0; i < 4; i++)
	{
		state->m_layer_colorbase[i] = 0;
		state->m_tilebanks[i] = 0;
		state->m_spritebanks[i] = 0;
	}
}

static MACHINE_CONFIG_START( asterix, asterix_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT("screen", asterix_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, 8000000)
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_MACHINE_START(asterix)
	MCFG_MACHINE_RESET(asterix)

	MCFG_EEPROM_ADD("eeprom", eeprom_intf)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_UPDATE_STATIC(asterix)

	MCFG_PALETTE_LENGTH(2048)

	MCFG_K056832_ADD("k056832", asterix_k056832_intf)
	MCFG_K053244_ADD("k053244", asterix_k05324x_intf)
	MCFG_K053251_ADD("k053251")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2151, 4000000)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_SOUND_ADD("k053260", K053260, 4000000)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.75)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.75)
MACHINE_CONFIG_END


ROM_START( asterix )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "068_ea_d01.8c", 0x000000,  0x20000, CRC(61d6621d) SHA1(908a344e9bbce0c7544bd049494258d1d3ad073b) )
	ROM_LOAD16_BYTE( "068_ea_d02.8d", 0x000001,  0x20000, CRC(53aac057) SHA1(7401ca5b70f384688c3353fc1ac9ef0b27814c66) )
	ROM_LOAD16_BYTE( "068a03.7c", 0x080000,  0x20000, CRC(8223ebdc) SHA1(e4aa39e4bc1d210bdda5b0cb41d6c8006c48dd24) )
	ROM_LOAD16_BYTE( "068a04.7d", 0x080001,  0x20000, CRC(9f351828) SHA1(e03842418f08e6267eeea03362450da249af73be) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "068_a05.5f", 0x000000, 0x010000,  CRC(d3d0d77b) SHA1(bfa77a8bf651dc27f481e96a2d63242084cc214c) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "068a12.16k", 0x000000, 0x080000, CRC(b9da8e9c) SHA1(a46878916833923e421da0667e37620ae0b77744) )
	ROM_LOAD( "068a11.12k", 0x080000, 0x080000, CRC(7eb07a81) SHA1(672c0c60834df7816d33d88643e4575b8ca9bcc1) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "068a08.7k", 0x000000, 0x200000, CRC(c41278fe) SHA1(58e5f67a67ae97e0b264489828cd7e74662c5ed5) )
	ROM_LOAD( "068a07.3k", 0x200000, 0x200000, CRC(32efdbc4) SHA1(b7e8610aa22249176d82b750e2549d1eea6abe4f) )

	ROM_REGION( 0x200000, "k053260", 0 )
	ROM_LOAD( "068a06.1e", 0x000000, 0x200000, CRC(6df9ec0e) SHA1(cee60312e9813bd6579f3ac7c3c2521a8e633eca) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
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

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "068a12.16k", 0x000000, 0x080000, CRC(b9da8e9c) SHA1(a46878916833923e421da0667e37620ae0b77744) )
	ROM_LOAD( "068a11.12k", 0x080000, 0x080000, CRC(7eb07a81) SHA1(672c0c60834df7816d33d88643e4575b8ca9bcc1) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "068a08.7k", 0x000000, 0x200000, CRC(c41278fe) SHA1(58e5f67a67ae97e0b264489828cd7e74662c5ed5) )
	ROM_LOAD( "068a07.3k", 0x200000, 0x200000, CRC(32efdbc4) SHA1(b7e8610aa22249176d82b750e2549d1eea6abe4f) )

	ROM_REGION( 0x200000, "k053260", 0 )
	ROM_LOAD( "068a06.1e", 0x000000, 0x200000, CRC(6df9ec0e) SHA1(cee60312e9813bd6579f3ac7c3c2521a8e633eca) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
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

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "068a12.16k", 0x000000, 0x080000, CRC(b9da8e9c) SHA1(a46878916833923e421da0667e37620ae0b77744) )
	ROM_LOAD( "068a11.12k", 0x080000, 0x080000, CRC(7eb07a81) SHA1(672c0c60834df7816d33d88643e4575b8ca9bcc1) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "068a08.7k", 0x000000, 0x200000, CRC(c41278fe) SHA1(58e5f67a67ae97e0b264489828cd7e74662c5ed5) )
	ROM_LOAD( "068a07.3k", 0x200000, 0x200000, CRC(32efdbc4) SHA1(b7e8610aa22249176d82b750e2549d1eea6abe4f) )

	ROM_REGION( 0x200000, "k053260", 0 )
	ROM_LOAD( "068a06.1e", 0x000000, 0x200000, CRC(6df9ec0e) SHA1(cee60312e9813bd6579f3ac7c3c2521a8e633eca) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
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

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "068a12.16k", 0x000000, 0x080000, CRC(b9da8e9c) SHA1(a46878916833923e421da0667e37620ae0b77744) )
	ROM_LOAD( "068a11.12k", 0x080000, 0x080000, CRC(7eb07a81) SHA1(672c0c60834df7816d33d88643e4575b8ca9bcc1) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "068a08.7k", 0x000000, 0x200000, CRC(c41278fe) SHA1(58e5f67a67ae97e0b264489828cd7e74662c5ed5) )
	ROM_LOAD( "068a07.3k", 0x200000, 0x200000, CRC(32efdbc4) SHA1(b7e8610aa22249176d82b750e2549d1eea6abe4f) )

	ROM_REGION( 0x200000, "k053260", 0 )
	ROM_LOAD( "068a06.1e", 0x000000, 0x200000, CRC(6df9ec0e) SHA1(cee60312e9813bd6579f3ac7c3c2521a8e633eca) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
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

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "068a12.16k", 0x000000, 0x080000, CRC(b9da8e9c) SHA1(a46878916833923e421da0667e37620ae0b77744) )
	ROM_LOAD( "068a11.12k", 0x080000, 0x080000, CRC(7eb07a81) SHA1(672c0c60834df7816d33d88643e4575b8ca9bcc1) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "068a08.7k", 0x000000, 0x200000, CRC(c41278fe) SHA1(58e5f67a67ae97e0b264489828cd7e74662c5ed5) )
	ROM_LOAD( "068a07.3k", 0x200000, 0x200000, CRC(32efdbc4) SHA1(b7e8610aa22249176d82b750e2549d1eea6abe4f) )

	ROM_REGION( 0x200000, "k053260", 0 )
	ROM_LOAD( "068a06.1e", 0x000000, 0x200000, CRC(6df9ec0e) SHA1(cee60312e9813bd6579f3ac7c3c2521a8e633eca) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "asterixj.nv", 0x0000, 0x0080, CRC(84229f2c) SHA1(34c7491c731fbf741dfd53bfc559d91201ccfb03) )
ROM_END


static DRIVER_INIT( asterix )
{
#if 0
	*(UINT16 *)(machine.region("maincpu")->base() + 0x07f34) = 0x602a;
	*(UINT16 *)(machine.region("maincpu")->base() + 0x00008) = 0x0400;
#endif
}


GAME( 1992, asterix,    0,       asterix, asterix, asterix, ROT0, "Konami", "Asterix (ver EAD)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1992, asterixeac, asterix, asterix, asterix, asterix, ROT0, "Konami", "Asterix (ver EAC)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1992, asterixeaa, asterix, asterix, asterix, asterix, ROT0, "Konami", "Asterix (ver EAA)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1992, asterixaad, asterix, asterix, asterix, asterix, ROT0, "Konami", "Asterix (ver AAD)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1992, asterixj,   asterix, asterix, asterix, asterix, ROT0, "Konami", "Asterix (ver JAD)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
