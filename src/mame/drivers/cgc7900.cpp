// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

        Chromatics CGC 7900

        05/04/2010 Skeleton driver.

****************************************************************************/

/*

    TODO:

    . does not boot
    . interrupts
    . vsync interrupt
    . map ROM to 000000-000007 at boot
    - NVRAM (= battery backed SRAM) at 0xE40xxx
    - floppy
    - bitmap display
    - Z mode read/write
    - color status write
    - bitmap roll
    - overlay roll
    . keyboard
    - joystick
    - light pen
    - memory parity
    - sync
    - disk DMA
    - PIO DMA
    - serial port controller
    - HVG
    - OMTI Series 10 SCSI controller (8" Winchester HD)

    docs:

    bitsavers://pdf/chromatics/
    http://z80cpu.eu/mirrors/oldcomputers.dyndns.org/public/pub/manuals/omti10a.pdf

    how to:

    C-u @ -- enter Terminal
    C-u F -- use serial port from Terminal (null_modem has to be set up 9600, 7E1)
    C-u M -- enter Monitor
    C-u T -- enter Thaw (setup utility)
    C-u d -- boot DOS
    C-u Q 1 -- display clock

    See User's Manual appendix A for more control sequences.
*/

#include "emu.h"
#include "includes/cgc7900.h"
#include "speaker.h"

#define VERBOSE 1
#include "logmacro.h"



/***************************************************************************
    PARAMETERS
***************************************************************************/

#define INT_RTC             0x0001
#define INT_RS449_TXRDY     0x0002
#define INT_BINT2           0x0004
#define INT_RS232_TXRDY     0x0008
#define INT_DISK            0x0010
#define INT_BINT3           0x0020
#define INT_BEZEL           0x0040
#define INT_KEYBOARD        0x0080
#define INT_RS449_RXRDY     0x0100
#define INT_LIGHT_PEN       0x0200
#define INT_BINT4           0x0400
#define INT_JOYSTICK        0x0800
#define INT_VSYNC           0x1000
#define INT_BINT5           0x2000
#define INT_BINT1           0x4000
#define INT_RS232_RXRDY     0x8000

/***************************************************************************
    READ/WRITE HANDLERS
***************************************************************************/

void cgc7900_state::kbd_put(u8 data)
{
	kbd_ready = true;
	kbd_data = data;
	irq_encoder(7, ASSERT_LINE);
}

/*-------------------------------------------------
    keyboard_r - keyboard data read
-------------------------------------------------*/

READ16_MEMBER( cgc7900_state::keyboard_r )
{
	u16 data;

	/*

	    bit     description

	     0      key data bit 0
	     1      key data bit 1
	     2      key data bit 2
	     3      key data bit 3
	     4      key data bit 4
	     5      key data bit 5
	     6      key data bit 6
	     7      key data bit 7
	     8      SHIFT key
	     9      CTRL key
	    10      M1 key
	    11      M2 key
	    12
	    13
	    14
	    15

	*/

	if (kbd_ready)
	{
		data = kbd_data | kbd_mods;
		kbd_ready = false;
		irq_encoder(7, CLEAR_LINE);
	}
	else
	{
		data = kbd_mods;
	}

	LOG("kbd == %04x\n", data);

	if (kbd_mods)
		kbd_mods = 0;

	return data;
}

/*-------------------------------------------------
    keyboard_w - keyboard data write
-------------------------------------------------*/

WRITE16_MEMBER( cgc7900_state::keyboard_w )
{
	/*

	    bit     description

	     0      LED select bit 0
	     1      LED select bit 1
	     2      LED select bit 2
	     3      LED select bit 3
	     4      LED select bit 4
	     5
	     6
	     7      LED switch (1=on, 0=off)
	     8
	     9
	    10
	    11
	    12
	    13
	    14
	    15

	*/

	LOG("kbd <- %04x\n", data);
}

/*-------------------------------------------------
    interrupt_mask_w - interrupt mask write
-------------------------------------------------*/

static const int int_levels[16] = { 5, 4, 5, 4, 4, 5, 4, 5, 5, 4, 5, 4, 4, 5, 4, 5 };
static const int int_vectors[16] = {
	0x4b, 0x44, 0x4c, 0x43, 0x42, 0x4d, 0x45, 0x4a, 0x49, 0x46, 0x4e, 0x41, 0x40, 0x4f, 0x47, 0x48
};

WRITE16_MEMBER( cgc7900_state::interrupt_mask_w )
{
	/*

	    bit     description     vec level

	     0      real time clock 4b  5
	     1      RS-449 Tx ready 44  4
	     2      BINT 2          4c  5
	     3      RS-232 Tx ready 43  4
	     4      disk            42  4
	     5      BINT 3          4d  5
	     6      bezel keys      45  4
	     7      keyboard        4a  5
	     8      RS-449 Rx ready 49  5
	     9      light pen       46  4
	    10      BINT 4          4e  5
	    11      joystick        41  4
	    12      vert. retrace   40  4
	    13      BINT 5          4f  5
	    14      BINT 1          47  4
	    15      RS-232 Rx ready 48  5

	    default mask is 0x7e3f -- RS232 Rx, RS449 Rx, keyboard, bezel.
	*/

	if (m_int_mask != data)
	{
		u16 changed = m_int_mask ^ data;

		LOG("i_mask: changed %04X -> %04X\n", m_int_mask, data);

		for (int i = 0; i < 16; i++)
		{
			if (BIT(changed, i) && BIT(data, i))
			{
				LOG("i_mask: pin %d disabled, clearing irq\n", i);
				irq_encoder(i, CLEAR_LINE);
			}
			if (BIT(changed, i) && !BIT(data, i))
			{
				LOG("i_mask: pin %d enabled, %s irq\n", i, BIT(m_int_active, i) ? "setting" : "clearing");
				irq_encoder(i, BIT(m_int_active, i));
			}
		}
	}

	m_int_mask = data;
}

void cgc7900_state::cpu_space_map(address_map &map)
{
	map(0xfffff2, 0xffffff).lr16("interrupt", [] (offs_t offset) -> u16 { return int_vectors[offset+1]; });
}

void cgc7900_state::irq_encoder(int pin, int state)
{
	if (state == ASSERT_LINE)
		m_int_active |= (1 << pin);
	else
		m_int_active &= ~(1 << pin);

	if (!BIT(m_int_mask, pin))
	{
		m_maincpu->set_input_line(int_levels[pin], state);
	}
}

/*-------------------------------------------------
    disk_data_r - disk data read
-------------------------------------------------*/

READ16_MEMBER( cgc7900_state::disk_data_r )
{
	return 0;
}

/*-------------------------------------------------
    disk_data_w - disk data write
-------------------------------------------------*/

WRITE16_MEMBER( cgc7900_state::disk_data_w )
{
}

/*-------------------------------------------------
    disk_status_r - disk status read
-------------------------------------------------*/

READ16_MEMBER( cgc7900_state::disk_status_r )
{
	/*

	    bit     signal      description

	     0      _I/O        input/output
	     1      _REQ        request
	     2      _BSY        busy
	     3      C/_D        control/data
	     4      _MSG        message
	     5      _RDY        ready
	     6
	     7
	     8
	     9
	    10
	    11
	    12
	    13
	    14
	    15

	*/

	return 0xffff - 0x04;
}

/*-------------------------------------------------
    disk_command_w - disk command write
-------------------------------------------------*/

WRITE16_MEMBER( cgc7900_state::disk_command_w )
{
}

READ16_MEMBER(cgc7900_state::unmapped_r)
{
	return rand();
}

/***************************************************************************
    MEMORY MAPS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_MAP( cgc7900_mem )
-------------------------------------------------*/

void cgc7900_state::cgc7900_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x1fffff).ram().share("chrom_ram");
	map(0x800000, 0x80ffff).rom().region(M68000_TAG, 0);
	map(0x810000, 0x9fffff).r(FUNC(cgc7900_state::unmapped_r));
	map(0xa00000, 0xbfffff).rw(FUNC(cgc7900_state::z_mode_r), FUNC(cgc7900_state::z_mode_w));
	map(0xc00000, 0xdfffff).ram().share("plane_ram");
	map(0xe00000, 0xe1ffff).w(FUNC(cgc7900_state::color_status_w));
//  AM_RANGE(0xe20000, 0xe23fff) Raster Processor
	map(0xe30000, 0xe303ff).ram().share("clut_ram");
	map(0xe38000, 0xe3bfff).ram().share("overlay_ram");
	map(0xe40000, 0xe40001).ram().share("roll_bitmap");
	map(0xe40002, 0xe40003).ram().share("pan_x");
	map(0xe40004, 0xe40005).ram().share("pan_y");
	map(0xe40006, 0xe40007).ram().share("zoom");
	map(0xe40008, 0xe40009).ram();
	map(0xe4000a, 0xe4000f).ram(); // Raster Processor
	map(0xe40010, 0xe40011).ram().share("blink_select");
	map(0xe40012, 0xe40013).ram().share("plane_select");
	map(0xe40014, 0xe40015).ram().share("plane_switch");
	map(0xe40016, 0xe40017).ram().share("color_status_fg");
	map(0xe40018, 0xe40019).ram().share("color_status_bg");
	map(0xe4001a, 0xe4001b).ram().share("roll_overlay");
	map(0xe4001c, 0xe40fff).ram();
//  AM_RANGE(0xefc440, 0xefc441) HVG Load X
//  AM_RANGE(0xefc442, 0xefc443) HVG Load Y
//  AM_RANGE(0xefc444, 0xefc445) HVG Load dX
//  AM_RANGE(0xefc446, 0xefc447) HVG Load dY
//  AM_RANGE(0xefc448, 0xefc449) HVG Load Pixel Color
//  AM_RANGE(0xefc44a, 0xefc44b) HVG Load Trip
	map(0xff8000, 0xff8003).rw(m_i8251_0, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0xff8040, 0xff8043).rw(m_i8251_1, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0xff8080, 0xff8081).rw(FUNC(cgc7900_state::keyboard_r), FUNC(cgc7900_state::keyboard_w));
//  AM_RANGE(0xff80c6, 0xff80c7) Joystick X axis
//  AM_RANGE(0xff80ca, 0xff80cb) Joystick Y axis
//  AM_RANGE(0xff80cc, 0xff80cd) Joystick Z axis
	map(0xff8100, 0xff8101).rw(FUNC(cgc7900_state::disk_data_r), FUNC(cgc7900_state::disk_data_w));
	map(0xff8120, 0xff8121).rw(FUNC(cgc7900_state::disk_status_r), FUNC(cgc7900_state::disk_command_w));
	map(0xff8140, 0xff8141).portr("BEZEL");
	map(0xff8180, 0xff8180).w(K1135A_TAG, FUNC(com8116_device::stt_str_w));
	map(0xff81c0, 0xff81ff).rw(MM58167_TAG, FUNC(mm58167_device::read), FUNC(mm58167_device::write)).umask16(0x00ff);
	map(0xff8200, 0xff8201).w(FUNC(cgc7900_state::interrupt_mask_w));
//  AM_RANGE(0xff8240, 0xff8241) Light Pen enable
//  AM_RANGE(0xff8242, 0xff8243) Light Pen X value
//  AM_RANGE(0xff8244, 0xff8245) Light Pen Y value
//  AM_RANGE(0xff8246, 0xff8247) Buffer memory parity check
//  AM_RANGE(0xff8248, 0xff8249) Buffer memory parity set/reset
	map(0xff824a, 0xff824b).r(FUNC(cgc7900_state::sync_r));
	map(0xff83c0, 0xff83c0).w(AY8910_TAG, FUNC(ay8910_device::address_w));
	map(0xff83c2, 0xff83c2).r(AY8910_TAG, FUNC(ay8910_device::data_r));
	map(0xff83c4, 0xff83c4).w(AY8910_TAG, FUNC(ay8910_device::data_w));
	// DDMA option board
//  AM_RANGE(0xff8500, 0xff8501) Disk DMA Command Register
//  AM_RANGE(0xff8502, 0xff8503) Disk DMA Address Register
//  AM_RANGE(0xff8507, 0xff8507) Disk DMA Control/Status Register
}

/*-------------------------------------------------
    ADDRESS_MAP( keyboard_mem )
-------------------------------------------------*/

void cgc7900_state::keyboard_mem(address_map &map)
{
	map(0x000, 0x7ff).rom();
}

/***************************************************************************
    INPUT PORTS
***************************************************************************/

/*-------------------------------------------------
    INPUT_PORTS( cgc7900 )
-------------------------------------------------*/

static INPUT_PORTS_START( cgc7900 )
	PORT_START("BEZEL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Bezel 7")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Bezel 8")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Bezel 5")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Bezel 6")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Bezel 3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Bezel 4")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Bezel 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Bezel 2")

	PORT_START("JOYSTICK_X")
	PORT_BIT( 0x3ff, 0x200, IPT_AD_STICK_X ) PORT_MINMAX(0x000, 0x3ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(50)

	PORT_START("JOYSTICK_Y")
	PORT_BIT( 0x3ff, 0x200, IPT_AD_STICK_Y ) PORT_MINMAX(0x000, 0x3ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(50)

	PORT_START("JOYSTICK_Z")
	PORT_BIT( 0x3ff, 0x200, IPT_AD_STICK_Z ) PORT_MINMAX(0x000, 0x3ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(50)

	PORT_START("LIGHT_PEN_X")
	PORT_BIT( 0x1ff, 0x100, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0, 511) PORT_SENSITIVITY(20) PORT_KEYDELTA(25)

	PORT_START("LIGHT_PEN_Y")
	PORT_BIT( 0x1ff, 0x0c0, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0, 383) PORT_SENSITIVITY(20) PORT_KEYDELTA(25)
INPUT_PORTS_END


/***************************************************************************
    MACHINE INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    MACHINE_START( cgc7900 )
-------------------------------------------------*/

void cgc7900_state::machine_start()
{
	/* register for state saving */
	save_pointer(NAME(m_overlay_ram.target()), 0x4000);
}

void cgc7900_state::machine_reset()
{
	uint8_t *user1 = memregion(M68000_TAG)->base();

	memcpy((uint8_t *)m_chrom_ram.target(), user1, 8); // not really what happens but...

	m_maincpu->reset();

	kbd_mods = 0x300; // forces cold boot -- initializes SRAM contents
	kbd_data = 0;
	kbd_ready = false;

	m_i8251_0->write_cts(0);

	m_int_active = 0;
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

/*-------------------------------------------------
    MACHINE_DRIVER( cgc7900 )
-------------------------------------------------*/

void cgc7900_state::cgc7900(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(28'480'000)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &cgc7900_state::cgc7900_mem);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &cgc7900_state::cpu_space_map);

	i8035_device &kbmcu(I8035(config, I8035_TAG, 1000000));
	kbmcu.set_addrmap(AS_PROGRAM, &cgc7900_state::keyboard_mem);
	kbmcu.set_disable();

//  am2910_device &am2910(AM2910(config, AM2910_TAG, XTAL(17'360'000)));
//  am2910.set_addrmap(AS_PROGRAM, &cgc7900_state::omti10_mem);


	/* video hardware */
	cgc7900_video(config);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8910(config, AY8910_TAG, XTAL(28'480'000)/16).add_route(ALL_OUTPUTS, "mono", 0.25);

	/* devices */
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(cgc7900_state::kbd_put));

	mm58167_device &rtc(MM58167(config, MM58167_TAG, XTAL(32'768)));
	rtc.irq().set(FUNC(cgc7900_state::irq<0x0>));

	com8116_device &k1135a(COM8116(config, K1135A_TAG, XTAL(5'068'800)));
	k1135a.fr_handler().set(m_i8251_0, FUNC(i8251_device::write_txc));
	k1135a.fr_handler().append(m_i8251_0, FUNC(i8251_device::write_rxc));
	k1135a.ft_handler().set(m_i8251_1, FUNC(i8251_device::write_txc));
	k1135a.ft_handler().append(m_i8251_1, FUNC(i8251_device::write_rxc));

	I8251(config, m_i8251_0, 0);
	m_i8251_0->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_i8251_0->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_i8251_0->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_i8251_0->rxrdy_handler().set(FUNC(cgc7900_state::irq<0xf>));
	m_i8251_0->txrdy_handler().set(FUNC(cgc7900_state::irq<0x3>));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "null_modem"));
	rs232.rxd_handler().set(m_i8251_0, FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set(m_i8251_0, FUNC(i8251_device::write_dsr));

	I8251(config, m_i8251_1, 0);
	m_i8251_1->txd_handler().set("rs449", FUNC(rs232_port_device::write_txd));
	m_i8251_1->dtr_handler().set("rs449", FUNC(rs232_port_device::write_dtr));
	m_i8251_1->rts_handler().set("rs449", FUNC(rs232_port_device::write_rts));
	m_i8251_1->rxrdy_handler().set(FUNC(cgc7900_state::irq<0x8>));
	m_i8251_1->txrdy_handler().set(FUNC(cgc7900_state::irq<0x1>));

	rs232_port_device &rs449(RS232_PORT(config, "rs449", default_rs232_devices, nullptr));
	rs449.rxd_handler().set(m_i8251_1, FUNC(i8251_device::write_rxd));
	rs449.dsr_handler().set(m_i8251_1, FUNC(i8251_device::write_dsr));
}

/***************************************************************************
    ROMS
***************************************************************************/

ROM_START( cgc7900 )
	ROM_REGION16_BE( 0x10000, M68000_TAG, 0 )
	ROM_LOAD16_BYTE( "210274 800k even a3ee term 1.4.ue24",                     0x0000, 0x1000, CRC(5fa8f368) SHA1(120dbcfedce0badd38bf5b23e1fbc99667eb286c) )
	ROM_LOAD16_BYTE( "210275 800k odd bbb3 term 1.4.uf24",                      0x0001, 0x1000, CRC(4d479457) SHA1(5fa96a1eadfd9ba493d28691286e2e001a489a19) )
	ROM_LOAD16_BYTE( "210276 802k even 0c22 term 1.4.ue22",                     0x2000, 0x1000, CRC(c88c44ec) SHA1(f39d8a3cf7aaefd815b4426348965b076c1f2265) )
	ROM_LOAD16_BYTE( "210277 802k odd b58c term 1.4.uf22",                      0x2001, 0x1000, CRC(52ffe01f) SHA1(683aa71c2eb17b7ad639b888487db73d7684901a) )
	ROM_LOAD16_BYTE( "210278 804k even eaf4 term 1.4.ue21",                     0x4000, 0x1000, CRC(7fe326db) SHA1(d39d05e008160fb8afa62e0d4cfb1d813f2296d4) )
	ROM_LOAD16_BYTE( "210279 804k odd 3f6d term 1.4.uf21",                      0x4001, 0x1000, CRC(6c12d81c) SHA1(efe7c20e567c02b9c5b66a2d18e035d5f5f56b28) )
	ROM_LOAD16_BYTE( "210280 806k even 679d term 1.4.ue20",                     0x6000, 0x1000, CRC(70930d74) SHA1(a5ab1c0bd8bd829408961107e01598cd71a97fec) )
	ROM_LOAD16_BYTE( "210281 1.4 806k odd sum 611e.uf20",                       0x6001, 0x1000, CRC(8080aa2a) SHA1(c018a23e6f2158e2d63723cade0a3ad737090921) )
	ROM_LOAD16_BYTE( "su7700 210282 808k even opmod term 1.4 sum 2550.ue18",    0x8000, 0x1000, CRC(8f5834cd) SHA1(3cd03c82aa85c30cbc8e954f5a9fc4e9034f913b) )
	ROM_LOAD16_BYTE( "su7700 210283 808k odd opmod term 1.4 sum faca.uf18",     0x8001, 0x1000, CRC(e593b133) SHA1(6c641df844706e0d990b5fd544e98594171080a1) )
	ROM_LOAD16_BYTE( "210258 80ak even b2f6 mon 1.3.ue15",                      0xa000, 0x1000, CRC(ec5a1250) SHA1(ffef73d35f172ac610b35bdf729d51eb6f9346ba) )
	ROM_LOAD16_BYTE( "210259 80ak odd cd66 mon 1.3.uf15",                       0xa001, 0x1000, CRC(61eb43c6) SHA1(baaae0b787798147da453aac1f815589ea4ed921) )
	ROM_LOAD16_BYTE( "210244 80c even ce40 dos 1.6b.ue13",                      0xc000, 0x1000, CRC(3b64e4cb) SHA1(71b28d160b101ea6165e602ff1c54272b7b30ece) )
	ROM_LOAD16_BYTE( "210245 80c odd 1ac3 dos 1.6b.uf13",                       0xc001, 0x1000, CRC(0b6539ca) SHA1(d49e6d3307e5d634a412fd80b59492f31e29f7e0) )
	ROM_LOAD16_BYTE( "210290 idris even rel3 sum 0cce.ue12",                    0xe000, 0x1000, CRC(07065772) SHA1(620ea5d55021e5c38efc010722ddbd852cd49e39) )
	ROM_LOAD16_BYTE( "210291 idris odd rel3 sum 5d11.uf12",                     0xe001, 0x1000, CRC(d81b30da) SHA1(228f9b4e39d430ce4aaa81ea63f4516a51e6d001) )

	ROM_REGION( 0x800, "i8035", 0 )
	ROM_LOAD( "keyboard controller i8035", 0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "norm chrset 4b40.ua13",  0x0000, 0x0400, CRC(55eb7b87) SHA1(768cea80597e7396d9e26f8cd09e6b480a526fce) )
	ROM_LOAD( "alt 46a7.ua14",          0x0400, 0x0400, CRC(be22b7e4) SHA1(83ef252c7fab33b4d3821a3049b89d044df35de8) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "rp0a.ub8",  0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "rp1a.ub7",  0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "rp2a.ub9",  0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "ha-5.ub1",  0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "03c0.ua16", 0x0000, 0x0020, NO_DUMP )
	ROM_LOAD( "03c0.ua11", 0x0000, 0x0020, NO_DUMP )

	/* OMTI Series 10 SCSI controller */
	ROM_REGION( 0x1400, AM2910_TAG, 0 )
	ROM_LOAD( "35 1.7b", 0x0000, 0x0400, NO_DUMP ) /* 82S137N */
	ROM_LOAD( "35 2.7b", 0x0400, 0x0400, NO_DUMP ) /* 82S137N */
	ROM_LOAD( "35 3.7b", 0x0800, 0x0400, NO_DUMP ) /* 82S137N */
	ROM_LOAD( "35 4.7b", 0x0c00, 0x0400, NO_DUMP ) /* 82S137N */
	ROM_LOAD( "35 5.7b", 0x1000, 0x0400, NO_DUMP ) /* 82S137N */
ROM_END

/***************************************************************************
    SYSTEM DRIVERS
***************************************************************************/

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY       FULLNAME    FLAGS */
COMP( 1980, cgc7900, 0,      0,      cgc7900, cgc7900, cgc7900_state, empty_init, "Chromatics", "CGC 7900", MACHINE_NOT_WORKING)
