/*  Konami Ultra Sports hardware

    Driver by Ville Linde

TODO:
- sound cpu irqs generation is unknown and very prone to get broken (i.e. if 4G and 2G returns as bad in POST screen).
- sound is lagged, reason is probably the same as above.

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/powerpc/ppc.h"
#include "sound/k054539.h"
#include "machine/eeprom.h"
#include "machine/konppc.h"
#include "sound/k056800.h"


class ultrsprt_state : public driver_device
{
public:
	ultrsprt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_vram(*this, "vram"),
		m_workram(*this, "workram"){ }

	required_shared_ptr<UINT32> m_vram;
	required_shared_ptr<UINT32> m_workram;
	DECLARE_WRITE32_MEMBER(palette_w);
	DECLARE_READ32_MEMBER(eeprom_r);
	DECLARE_WRITE32_MEMBER(eeprom_w);
	DECLARE_WRITE32_MEMBER(int_ack_w);
	DECLARE_READ16_MEMBER(K056800_68k_r);
	DECLARE_WRITE16_MEMBER(K056800_68k_w);
	DECLARE_CUSTOM_INPUT_MEMBER(analog_ctrl_r);
	virtual void machine_start();
};




static SCREEN_UPDATE_IND16( ultrsprt )
{
	ultrsprt_state *state = screen.machine().driver_data<ultrsprt_state>();
	int i, j;

	UINT8 *ram = reinterpret_cast<UINT8 *>(state->m_vram.target());

	for (j=0; j < 400; j++)
	{
		UINT16 *dest = &bitmap.pix16(j);
		int fb_index = j * 1024;

		for (i=0; i < 512; i++)
		{
			UINT8 p1 = ram[BYTE4_XOR_BE(fb_index + i + 512)];
			if (p1 == 0)
				dest[i] = ram[BYTE4_XOR_BE(fb_index + i)];
			else
				dest[i] = 0x100 + p1;
		}
	}

	return 0;
}

WRITE32_MEMBER(ultrsprt_state::palette_w)
{
	COMBINE_DATA(&m_generic_paletteram_32[offset]);
	data = m_generic_paletteram_32[offset];

	palette_set_color(machine(), (offset*2)+0, MAKE_RGB(pal5bit(data >> 26), pal5bit(data >> 21), pal5bit(data >> 16)));
	palette_set_color(machine(), (offset*2)+1, MAKE_RGB(pal5bit(data >> 10), pal5bit(data >>  5), pal5bit(data >>  0)));
}

READ32_MEMBER(ultrsprt_state::eeprom_r)
{
	UINT32 r = 0;

	if (ACCESSING_BITS_24_31)
		r |= ioport("SERVICE")->read();

	return r;
}

WRITE32_MEMBER(ultrsprt_state::eeprom_w)
{
	if (ACCESSING_BITS_24_31)
		ioport("EEPROMOUT")->write(data, 0xffffffff);
}

CUSTOM_INPUT_MEMBER(ultrsprt_state::analog_ctrl_r)
{
	const char *tag = (const char *)param;
	return ioport(tag)->read() & 0xfff;
}

WRITE32_MEMBER(ultrsprt_state::int_ack_w)
{
	machine().device("maincpu")->execute().set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
}

void ultrsprt_state::machine_start()
{
	/* set conservative DRC options */
	ppcdrc_set_options(machine().device("maincpu"), PPCDRC_COMPATIBLE_OPTIONS);

	/* configure fast RAM regions for DRC */
	ppcdrc_add_fastram(machine().device("maincpu"), 0x80000000, 0x8007ffff, FALSE, m_vram);
	ppcdrc_add_fastram(machine().device("maincpu"), 0xff000000, 0xff01ffff, FALSE, m_workram);
}



static ADDRESS_MAP_START( ultrsprt_map, AS_PROGRAM, 32, ultrsprt_state )
	AM_RANGE(0x00000000, 0x0007ffff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0x70000000, 0x70000003) AM_READWRITE(eeprom_r, eeprom_w)
	AM_RANGE(0x70000020, 0x70000023) AM_READ_PORT("P1")
	AM_RANGE(0x70000040, 0x70000043) AM_READ_PORT("P2")
	AM_RANGE(0x70000080, 0x70000087) AM_DEVWRITE_LEGACY("k056800", k056800_host_w)
	AM_RANGE(0x70000088, 0x7000008f) AM_DEVREAD_LEGACY("k056800", k056800_host_r)
	AM_RANGE(0x700000e0, 0x700000e3) AM_WRITE(int_ack_w)
	AM_RANGE(0x7f000000, 0x7f01ffff) AM_RAM AM_SHARE("workram")
	AM_RANGE(0x7f700000, 0x7f703fff) AM_RAM_WRITE(palette_w) AM_SHARE("paletteram")
	AM_RANGE(0x7f800000, 0x7f9fffff) AM_MIRROR(0x00600000) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END


/*****************************************************************************/


READ16_MEMBER(ultrsprt_state::K056800_68k_r)
{
	device_t *k056800 = machine().device("k056800");
	UINT16 r = 0;

	if (ACCESSING_BITS_8_15)
		r |= k056800_sound_r(k056800, space, (offset*2)+0, 0xffff) << 8;

	if (ACCESSING_BITS_0_7)
		r |= k056800_sound_r(k056800, space, (offset*2)+1, 0xffff) << 0;

	return r;
}

WRITE16_MEMBER(ultrsprt_state::K056800_68k_w)
{
	device_t *k056800 = machine().device("k056800");

	if (ACCESSING_BITS_8_15)
		k056800_sound_w(k056800, space, (offset*2)+0, (data >> 8) & 0xff, 0x00ff);

	if (ACCESSING_BITS_0_7)
		k056800_sound_w(k056800, space, (offset*2)+1, (data >> 0) & 0xff, 0x00ff);
}

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 16, ultrsprt_state )
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM
	AM_RANGE(0x00100000, 0x00101fff) AM_RAM
	AM_RANGE(0x00200000, 0x00200007) AM_WRITE(K056800_68k_w)
	AM_RANGE(0x00200008, 0x0020000f) AM_READ(K056800_68k_r)
	AM_RANGE(0x00400000, 0x004002ff) AM_DEVREADWRITE8("konami", k054539_device, read, write, 0xffff)
ADDRESS_MAP_END

/*****************************************************************************/

static INPUT_PORTS_START( ultrsprt )
	PORT_START("P1")
	PORT_BIT( 0x00000fff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, ultrsprt_state,analog_ctrl_r, "STICKY1")
	PORT_BIT( 0x0fff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, ultrsprt_state,analog_ctrl_r, "STICKX1")
	PORT_BIT( 0x40000000, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20000000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x00000fff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, ultrsprt_state,analog_ctrl_r, "STICKY2")
	PORT_BIT( 0x0fff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, ultrsprt_state,analog_ctrl_r, "STICKX2")
	PORT_BIT( 0x40000000, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x20000000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("SERVICE")
	PORT_BIT( 0x02000000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_device, read_bit)
	PORT_SERVICE_NO_TOGGLE( 0x08000000, IP_ACTIVE_LOW )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01000000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, write_bit)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_clock_line)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_cs_line)

	PORT_START("STICKX1")
	PORT_BIT( 0xfff, 0x800, IPT_AD_STICK_X ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("STICKY1")
	PORT_BIT( 0xfff, 0x800, IPT_AD_STICK_Y ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("STICKX2")
	PORT_BIT( 0xfff, 0x800, IPT_AD_STICK_X ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("STICKY2")
	PORT_BIT( 0xfff, 0x800, IPT_AD_STICK_Y ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)
INPUT_PORTS_END


static INTERRUPT_GEN( ultrsprt_vblank )
{
	device->execute().set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
}

static void sound_irq_callback(running_machine &machine, int irq)
{
	if (irq == 0)
		/*generic_pulse_irq_line(machine.device("audiocpu"), INPUT_LINE_IRQ5, 1)*/;
	else
		machine.device("audiocpu")->execute().set_input_line(INPUT_LINE_IRQ6, HOLD_LINE);
}

static const k056800_interface ultrsprt_k056800_interface =
{
	sound_irq_callback
};

static k054539_interface k054539_config;

static MACHINE_CONFIG_START( ultrsprt, ultrsprt_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PPC403GA, 25000000)		/* PowerPC 403GA 25MHz */
	MCFG_CPU_PROGRAM_MAP(ultrsprt_map)
	MCFG_CPU_VBLANK_INT("screen", ultrsprt_vblank)

	MCFG_CPU_ADD("audiocpu", M68000, 8000000)		/* Not sure about the frequency */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_PERIODIC_INT(irq5_line_hold, 1)	// ???

	MCFG_QUANTUM_TIME(attotime::from_hz(12000))

	MCFG_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 399)
	MCFG_SCREEN_UPDATE_STATIC(ultrsprt)

	MCFG_PALETTE_LENGTH(8192)

	/* sound hardware */
	MCFG_K056800_ADD("k056800", ultrsprt_k056800_interface)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_K054539_ADD("konami", 48000, k054539_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


/*****************************************************************************/

ROM_START( fiveside )
	ROM_REGION(0x200000, "user1", 0)	/* PowerPC program roms */
	ROM_LOAD32_BYTE("479uaa01.bin", 0x000003, 0x80000, CRC(1bc4893d) SHA1(2c9df38ecb7efa7b686221ee98fa3aad9a63e152))
	ROM_LOAD32_BYTE("479uaa02.bin", 0x000002, 0x80000, CRC(ae74a6d0) SHA1(6113c2eea1628b22737c7b87af0e673d94984e88))
	ROM_LOAD32_BYTE("479uaa03.bin", 0x000001, 0x80000, CRC(5c0b176f) SHA1(9560259bc081d4cfd72eb485c3fdcecf484ba7a8))
	ROM_LOAD32_BYTE("479uaa04.bin", 0x000000, 0x80000, CRC(01a3e4cb) SHA1(819df79909d57fa12481698ffdb32b00586131d8))

	ROM_REGION(0x20000, "audiocpu", 0)		/* M68K program */
	ROM_LOAD("479_a05.bin", 0x000000, 0x20000, CRC(251ae299) SHA1(5ffd74357e3c6ddb3a208c39a3b32b53fea90282))

	ROM_REGION(0x100000, "konami", 0)	/* Sound roms */
	ROM_LOAD("479_a06.bin", 0x000000, 0x80000, CRC(8d6ac8a2) SHA1(7c4b8bd47cddc766cbdb6a486acc9221be55b579))
	ROM_LOAD("479_a07.bin", 0x080000, 0x80000, CRC(75835df8) SHA1(105b95c16f2ce6902c2e4c9c2fd9f2f7a848c546))

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "fiveside.nv", 0x0000, 0x0080, CRC(aad11072) SHA1(8f777ee47801faa7ce8420c3052034720225aae7) )
ROM_END

GAME(1995, fiveside, 0, ultrsprt, ultrsprt, driver_device, 0, ROT90, "Konami", "Five a Side Soccer (ver UAA)", GAME_IMPERFECT_SOUND)
