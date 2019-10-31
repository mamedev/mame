// license: BSD-3-Clause
// copyright-holders: David Haywood, Dirk Best
/*
   Dinamic / Inder arcade hardware

   Mega Phoenix
   Hammer Boy

 also known to exist on this hardware:
   Nonamed 2 (ever finished? only code seen has 1991 date and is vastly incomplete) (versions exist for Amstrad CPC, MSX and Spectrum)
   After The War



  trivia: Test mode graphics are the same as Little Robin(?!), TMS is very similar too, suggesting they share a common codebase.

 PIC16C54 info:
 - The PIC has 5 functions:
   * Read dip switches (serially connected) [cmd 0x82 0x86]
   * Read the two start buttons [returned with all commands]
   * Provide 4 security codes. For the dumped PIC those are:
     0x4a 0x6f 0x61 0x6e (Joan). Not used by Mega Phoenix. [cmd 0x8a 0x8e 0x92 0x96]
   * Watchdog enable. Not used by Mega Phoenix. [cmd 9a]
   * Provide PIC software version to the game. This is 0x11 here. [all other cmds]
 - Communication with the game is achieved using a 8255 PPI on port C that is
   connected serially to the PIC. For port assignments see the code below.
 - The game sends an 8-bit command. After each bit, the PIC sends an answer bit.
   The start buttons are always bits 2 and 3 here.
 - All sent commands look like this: 1ccccc10
 - After the command was received, the PIC will send an additional 8 bits
   with the result.



 ToDo:
  - where should roms 6/7 map, they contain the 68k vectors, but the game expects RAM at 0, and it doesn't seem to read any of the other data from those roms.. they contain
    a cross hatch pattern amongst other things?
 Sound:
  - how does banking work? when the irq callbacks happen for each irq level? currently no way to access this because it's a daisy chain setup with the ctc?
  - even if i hack that the title screen speech doesn't work properly - is there a timing register like little robin?
 I/O:
  - Verify when m_ppi_to_pic_command is set and cleared. It's currently guessed but seems to work fine this way.


  --


  Chips of note

  Main board:

  TS68000CP8
  TMS34010FNL-40
  TMP82C55AP-2

  Bt478KPJ35  Palette / RAMDAC

  Actel A1010A-PL68C  (custom blitter maybe?)

  2x 8 DSW, bottom corner, away from everything..

 Sub / Sound board:

  ST Z8430AB1

  custom INDER badged chip 40 pin?  (probably just a z80 - it's in the sound section)
    MODELO: MEGA PHOENIX
    KIT NO. 1.034
    FECHA FABRICACION 08.10.91
    LA MANIPULCION DE LA ETIQUETA O DE LA PLACA ANULA SU SARANTIA
    (this sticker is also present on the other PCB)


*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "machine/74166.h"
#include "machine/i8255.h"
#include "machine/inder_sb.h"
#include "machine/inder_vid.h"



class megaphx_state : public driver_device
{
public:
	megaphx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_indervid(*this, "inder_vid"),
		m_maincpu(*this, "maincpu"),
		m_mainram(*this, "mainram"),
		m_indersb(*this, "inder_sb"),
		m_ppi(*this, "ppi8255_0"),
		m_dsw_shifter(*this, "ttl166_%u", 1U),
		m_dsw_data(0),
		m_ppi_to_pic_command(0), m_ppi_to_pic_clock(0), m_ppi_to_pic_data(0),
		m_pic_to_ppi_clock(0), m_pic_to_ppi_data(0)
	{ }

	void megaphx(machine_config &config);

	void init_megaphx();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<inder_vid_device> m_indervid;

private:
	DECLARE_READ8_MEMBER(pic_porta_r);
	DECLARE_WRITE8_MEMBER(pic_porta_w);
	DECLARE_READ8_MEMBER(pic_portb_r);
	DECLARE_WRITE8_MEMBER(pic_portb_w);
	DECLARE_READ8_MEMBER(ppi_portc_r);
	DECLARE_WRITE8_MEMBER(ppi_portc_w);
	DECLARE_WRITE_LINE_MEMBER(dsw_w);

	void megaphx_68k_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint16_t> m_mainram;
	required_device<inder_sb_device> m_indersb;
	required_device<i8255_device> m_ppi;
	required_device_array<ttl166_device, 2> m_dsw_shifter;

	int m_dsw_data;
	int m_ppi_to_pic_command;
	int m_ppi_to_pic_clock;
	int m_ppi_to_pic_data;
	int m_pic_to_ppi_clock;
	int m_pic_to_ppi_data;
};

class hamboy_state : public megaphx_state
{
public:
	hamboy_state(const machine_config &mconfig, device_type type, const char *tag)
		: megaphx_state(mconfig, type, tag)
	{ }

protected:
	virtual void machine_reset() override;
};

void megaphx_state::machine_start()
{
	uint16_t *src = (uint16_t*)memregion( "boot" )->base();
	// copy vector table? - it must be writable because the game write the irq vector..
	memcpy(m_mainram, src, 0x80);
}

void megaphx_state::machine_reset()
{
	m_indervid->set_bpp(8);
}

void hamboy_state::machine_reset()
{
	m_indervid->set_bpp(4);
}

void megaphx_state::megaphx_68k_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().region("boot", 0x00000); // or the rom doesn't map here? it contains the service mode grid amongst other things..
	map(0x000000, 0x00ffff).ram().share("mainram"); // maps over part of the rom??
	map(0x040000, 0x040007).rw("inder_vid:tms", FUNC(tms34010_device::host_r), FUNC(tms34010_device::host_w));
	map(0x050000, 0x050001).w(m_indersb, FUNC(inder_sb_device::megaphx_0x050000_w));
	map(0x050002, 0x050003).r(m_indersb, FUNC(inder_sb_device::megaphx_0x050002_r));
	map(0x060000, 0x060007).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x800000, 0x8fffff).rom().region("data", 0x00000);
}




static INPUT_PORTS_START( megaphx )
	PORT_START("P1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON2)        PORT_PLAYER(1) // shield
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3)        PORT_PLAYER(1) // unused? (in test mode)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(1) // high score entry
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(1) // high score entry
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON2)        PORT_PLAYER(2) // shield
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3)        PORT_PLAYER(2) // unused? (in test mode)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(2) // high score entry
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(2) // high score entry
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(2)

	PORT_START("START")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START2)

	PORT_START("DSW1")
	PORT_DIPNAME(0x07, 0x03, DEF_STR( Coin_A )) PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(   0x07, DEF_STR( 5C_1C ))
	PORT_DIPSETTING(   0x06, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(   0x05, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(   0x04, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(   0x03, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(   0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(   0x01, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(   0x00, DEF_STR( 1C_4C ))
	PORT_DIPNAME(0x38, 0x18, DEF_STR( Coin_B )) PORT_DIPLOCATION("SW1:!4,!5,!6")
	PORT_DIPSETTING(   0x38, DEF_STR( 5C_1C ))
	PORT_DIPSETTING(   0x30, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(   0x28, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(   0x20, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(   0x18, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(   0x10, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(   0x08, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(   0x00, DEF_STR( 1C_4C ))
	PORT_DIPNAME(0xc0, 0x80, DEF_STR( Lives ))  PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(   0x00, "2")
	PORT_DIPSETTING(   0x40, "3")
	PORT_DIPSETTING(   0x80, "4")
	PORT_DIPSETTING(   0xc0, "5")

	PORT_START("DSW2")
	PORT_DIPNAME(0x01, 0x00, DEF_STR( Demo_Sounds )) PORT_DIPLOCATION("SW2:!1")
	PORT_DIPSETTING(   0x01, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_SERVICE_DIPLOC(0x02, IP_ACTIVE_HIGH, "SW2:!2")
	PORT_DIPNAME(0x1c, 0x10, DEF_STR( Difficulty))   PORT_DIPLOCATION("SW2:!3,!4,!5") // sets the enemy speed
	PORT_DIPSETTING(   0x00, "0")
	PORT_DIPSETTING(   0x04, "1")
	PORT_DIPSETTING(   0x08, "2")
	PORT_DIPSETTING(   0x0c, "3")
	PORT_DIPSETTING(   0x10, "4")
	PORT_DIPSETTING(   0x14, "5")
	PORT_DIPSETTING(   0x18, "6")
	PORT_DIPSETTING(   0x1c, "7")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:!6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:!7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:!8")
INPUT_PORTS_END

static INPUT_PORTS_START( hamboy )
	PORT_INCLUDE(megaphx)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME(0x07, 0x06, DEF_STR( Coin_A )) PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(   0x00, DEF_STR( 5C_1C ))
	PORT_DIPSETTING(   0x03, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(   0x05, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(   0x01, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(   0x06, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(   0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(   0x04, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(   0x07, DEF_STR( 1C_4C ))
	PORT_DIPNAME(0x38, 0x30, DEF_STR( Coin_B )) PORT_DIPLOCATION("SW1:!4,!5,!6")
	PORT_DIPSETTING(   0x00, DEF_STR( 5C_1C ))
	PORT_DIPSETTING(   0x18, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(   0x28, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(   0x08, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(   0x30, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(   0x10, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(   0x20, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(   0x38, DEF_STR( 1C_4C ))
	PORT_DIPNAME(0xc0, 0x80, DEF_STR( Lives ))  PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(   0x00, "1")
	PORT_DIPSETTING(   0x40, "2")
	PORT_DIPSETTING(   0x80, "3")
	PORT_DIPSETTING(   0xc0, "4")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME(0x01, 0x00, DEF_STR( Demo_Sounds )) PORT_DIPLOCATION("SW2:!1")
	PORT_DIPSETTING(   0x01, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_SERVICE_DIPLOC(0x02, IP_ACTIVE_HIGH, "SW2:!2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x00, "SW2:!3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x00, "SW2:!4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x00, "SW2:!5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x00, "SW2:!6") // 6 or 7 needs to be off or the timer runs at ridiculous speed
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x00, "SW2:!7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:!8")
INPUT_PORTS_END


// pic port a
//
// 3---  r   serial clock
// -2--   w  serial clock
// --1-   w  not used
// ---0  rw  serial data

READ8_MEMBER( megaphx_state::pic_porta_r )
{
	uint8_t data = 0;

	// clear when the pic starts reading the command
	m_ppi_to_pic_command = 0;

	data |= m_ppi_to_pic_data << 0;
	data |= m_ppi_to_pic_clock << 3;

	return data;
}

WRITE8_MEMBER( megaphx_state::pic_porta_w )
{
	m_pic_to_ppi_data = BIT(data, 0);
	m_pic_to_ppi_clock = BIT(data, 2);
}

// pic port b
//
// 7-------   w  not used
// -6------   w  watchdog (not enabled by megaphx)
// --5-----  r   start2
// ---4----  r   start1
// ----3---   w  dsw clock
// -----2--   w  dsw shift/load
// ------1-  r   dip switch data
// -------0  r   ppi to pic command incoming

READ8_MEMBER( megaphx_state::pic_portb_r )
{
	uint8_t data = 0;

	data |= m_ppi_to_pic_command << 0;
	data |= m_dsw_data << 1;
	data |= ioport("START")->read() << 4;

	return data;
}

WRITE8_MEMBER( megaphx_state::pic_portb_w )
{

	m_dsw_shifter[0]->shift_load_w(BIT(data, 2));
	m_dsw_shifter[1]->shift_load_w(BIT(data, 2));
	m_dsw_shifter[0]->clock_w(BIT(data, 3));
	m_dsw_shifter[1]->clock_w(BIT(data, 3));
}

// ppi port c
//
// 7-------   w  ppi to pic clock
// -6------   w  ppi to pic data
// --5-----   w  unknown
// ---4----   w  ppi to pic command
// ----3---  r   pic to ppi clock
// -----2--  r   unknown
// ------1-  r   pic to ppi data
// -------0  r   unknown

READ8_MEMBER( megaphx_state::ppi_portc_r )
{
	uint8_t data = 0;

	data |= m_pic_to_ppi_data << 1;
	data |= m_pic_to_ppi_clock << 3;

	return data;
}

WRITE8_MEMBER( megaphx_state::ppi_portc_w )
{
	// avoid bogus write on reset
	if (data == 0xff)
		return;

	// only set, don't clear here. otherwise the pic has no chance to pick it up
	if (BIT(data, 4))
		m_ppi_to_pic_command = BIT(data, 4);

	m_ppi_to_pic_data = BIT(data, 6);
	m_ppi_to_pic_clock = BIT(data, 7);
}

WRITE_LINE_MEMBER( megaphx_state::dsw_w )
{
	m_dsw_data = state;
}

void megaphx_state::megaphx(machine_config &config)
{
	M68000(config, m_maincpu, 16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &megaphx_state::megaphx_68k_map);

	pic16c54_device &pic(PIC16C54(config, "pic", 6_MHz_XTAL)); // correct?
	pic.read_a().set(FUNC(megaphx_state::pic_porta_r));
	pic.write_a().set(FUNC(megaphx_state::pic_porta_w));
	pic.read_b().set(FUNC(megaphx_state::pic_portb_r));
	pic.write_b().set(FUNC(megaphx_state::pic_portb_w));

	config.set_perfect_quantum(m_maincpu);

	TTL166(config, m_dsw_shifter[0]);
	m_dsw_shifter[0]->data_callback().set_ioport("DSW1");
	m_dsw_shifter[0]->qh_callback().set(m_dsw_shifter[1], FUNC(ttl166_device::serial_w));

	TTL166(config, m_dsw_shifter[1]);
	m_dsw_shifter[1]->data_callback().set_ioport("DSW2");
	m_dsw_shifter[1]->qh_callback().set(FUNC(megaphx_state::dsw_w));

	I8255A(config, m_ppi);
	m_ppi->in_pa_callback().set_ioport("P1");
	m_ppi->in_pb_callback().set_ioport("P2");
	m_ppi->in_pc_callback().set(FUNC(megaphx_state::ppi_portc_r));
	m_ppi->out_pc_callback().set(FUNC(megaphx_state::ppi_portc_w));

	INDER_VIDEO(config, m_indervid, 0);

	INDER_AUDIO(config, "inder_sb", 0);
}

ROM_START( megaphx )
	ROM_REGION16_BE( 0x40000, "boot", 0 )  // the majority of the data in these does not get used?! (only the vector table) is it just garbage??
	ROM_LOAD16_BYTE( "mph6.u32", 0x000001, 0x20000, CRC(b99703d4) SHA1(393b6869e71d4c61060e66e0e9e36a1e6ca345d1) )
	ROM_LOAD16_BYTE( "mph7.u21", 0x000000, 0x20000, CRC(f11e7449) SHA1(1017142d10011d68e49d3ccdb1ac4e815c03b17a) )

	ROM_REGION16_BE( 0x100000, "data", 0 )
	ROM_LOAD16_BYTE( "mph0.u38", 0x000001, 0x20000, CRC(b63dd20f) SHA1(c8ce5985a6ba49428d66a49d9d623ccdfce422c2) )
	ROM_LOAD16_BYTE( "mph1.u27", 0x000000, 0x20000, CRC(4dcbf44b) SHA1(a8fa49ecd033f1aeb323e0032ddcf5f8f9463ac0) )
	ROM_LOAD16_BYTE( "mph2.u37", 0x040001, 0x20000, CRC(a0f69c27) SHA1(d0c5c241d94a1f03f51e7e517e2f9dec6abcf75a) )
	ROM_LOAD16_BYTE( "mph3.u26", 0x040000, 0x20000, CRC(4db84cc5) SHA1(dd74acd4b32c7e7553554ac0f9ba13503358e869) )
	ROM_LOAD16_BYTE( "mph4.u36", 0x080001, 0x20000, CRC(c8e0725e) SHA1(b3af315b9a94a692e81e0dbfd4035036c2af4f50) )
	ROM_LOAD16_BYTE( "mph5.u25", 0x080000, 0x20000, CRC(c95ccb69) SHA1(9d14cbfafd943f6ff461a7f373170a35e36eb695) )

	ROM_REGION( 0x200000, "inder_sb:user2", 0 )
	ROM_LOAD( "sonido_mph1.u39", 0x00000, 0x20000, CRC(f5e65557) SHA1(5ae759c2bcef96fbda42f088c02b6dec208030f3) )
	ROM_LOAD( "sonido_mph2.u38", 0x20000, 0x20000, CRC(7444d0f9) SHA1(9739b48993bccea5530533b67808d13d6155ffe3) )

	ROM_REGION( 0x100000, "inder_sb:audiocpu", 0 )
	ROM_LOAD( "sonido_mph0.u35", 0x000000, 0x2000,  CRC(abc1b140) SHA1(8384a162d85cf9ea870d22f44b1ca64001c6a083) )

	ROM_REGION( 0x100000, "pic", 0 )
	ROM_LOAD( "pic16c54-xt.bin", 0x000000, 0x430,  CRC(21f396fb) SHA1(c8badb9b3681e684bced0ced1de4c3a15641de8b) )
	ROM_FILL(0x2c, 1, 0x01) // patch timer length or its too slow (pic issue?)

	ROM_REGION( 0x1000, "pals", 0 ) // protected
	ROM_LOAD( "p31_u31_palce16v8h-25.jed", 0x000, 0xbd4, BAD_DUMP CRC(05ef04b7) SHA1(330dd81a832b6675fb0473868c26fe9bec2da854) )
	ROM_LOAD( "p40_u29_palce16v8h-25.jed", 0x000, 0xbd4, BAD_DUMP CRC(44b7e51c) SHA1(b8b34f3b319d664ec3ad72ed87d9f65701f183a5) )
ROM_END

ROM_START( hamboy )
	ROM_REGION16_BE( 0x40000, "boot", 0 )  // these only contain the boot vectors(!)
	ROM_LOAD16_BYTE( "hb8 - u32.bin", 0x000001, 0x20000, CRC(4f7b142a) SHA1(e6e6cb05672e4f99def69be2f4cbc56f5d37f226) )
	ROM_LOAD16_BYTE( "hb9 - u21.bin", 0x000000, 0x20000, CRC(138e294f) SHA1(671b34395f0773889ddf6aa1f4291df981d1b059) )

	ROM_REGION16_BE( 0x100000, "data", 0 )
	ROM_LOAD16_BYTE( "hb0 - u38.bin", 0x000001, 0x20000, CRC(b946a47f) SHA1(7f78a198fa3c5a00c124ab62473da4cddc0ac31f) )
	ROM_LOAD16_BYTE( "hb1 - u27.bin", 0x000000, 0x20000, CRC(890e1571) SHA1(e4a50a1849e9bc9853070da160e042f13737d8d2) )
	ROM_LOAD16_BYTE( "hb2 - u37.bin", 0x040001, 0x20000, CRC(b71b0aad) SHA1(fb2525a1581e6aa9a60ce76a09947b8d1941951c) )
	ROM_LOAD16_BYTE( "hb3 - u26.bin", 0x040000, 0x20000, CRC(1d0d61b9) SHA1(d747c8c31a81364d85ac00c50cefd695868d916d) )
	ROM_LOAD16_BYTE( "hb4 - u36.bin", 0x080001, 0x20000, CRC(9b81948e) SHA1(9e8bbee7f19e97d81205add7dbe89b353c6ab25a) )
	ROM_LOAD16_BYTE( "hb5 - u25.bin", 0x080000, 0x20000, CRC(23885e08) SHA1(f0233e3e007715d1b0b94fd52ffb597d667dc818) )
	ROM_LOAD16_BYTE( "hb6 - u35.bin", 0x0c0001, 0x20000, CRC(0c479648) SHA1(a07947b0b4e526b853782545bfdf73effa6c6579) )
	ROM_LOAD16_BYTE( "hb7 - u24.bin", 0x0c0000, 0x20000, CRC(297a6944) SHA1(7c5c66412db7905b0302a4e451fc20c20233990e) )

	ROM_REGION( 0x200000, "inder_sb:user2", 0 )
	ROM_LOAD( "sonido hammerboy 0 - u39.bin", 0x00000, 0x20000, CRC(8d94ac97) SHA1(3447e4b5670880a9b222cba84f5630e8ed42c2d3) )
	ROM_LOAD( "sonido hammerboy 1 - u38.bin", 0x20000, 0x20000, CRC(f92e5098) SHA1(10c869c7b1250a119bf201bfc8c586e9340c2a66) )

	ROM_REGION( 0x100000, "inder_sb:audiocpu", 0 )
	ROM_LOAD( "hammerboy - u35.bin", 0x000000, 0x2000,  CRC(cd22f2a4) SHA1(c5cf5b1ce528412493e2b5f565ed38e3e9123d37) )

	ROM_REGION( 0x100000, "pic", 0 )
	ROM_LOAD( "pic16c54-xt.bin", 0x000000, 0x430, CRC(21f396fb) SHA1(c8badb9b3681e684bced0ced1de4c3a15641de8b) )
	ROM_FILL(0x2c, 1, 0x01) // patch timer length or its too slow (pic issue?)
ROM_END

GAME( 1991, megaphx,  0,        megaphx, megaphx, megaphx_state, empty_init, ROT0, "Dinamic / Inder", "Mega Phoenix", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1990, hamboy,   0,        megaphx, hamboy,  hamboy_state,  empty_init, ROT0, "Dinamic / Inder", "Hammer Boy",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
