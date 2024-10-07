// license:BSD-3-Clause
// copyright-holders:David Haywood, AJR

#include "emu.h"
#include "cpu/m6502/st2205u.h"
#include "bl_handhelds_lcdc.h"
#include "screen.h"
#include "speaker.h"


namespace {

class st22xx_bbl338_state : public driver_device
{
public:
	st22xx_bbl338_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_lcdc(*this, "lcdc")
		, m_input_matrix(*this, "IN%u", 1U)
		, m_portb(0xff)
	{
	}

	void st22xx_bbl338(machine_config &config);
	void st22xx_dphh8213(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	required_device<st2205u_base_device> m_maincpu;

private:
	u8 porta_r();
	void portb_w(u8 data);
	void porta_w(u8 data);

	void st22xx_bbl338_map(address_map &map) ATTR_COLD;
	void st22xx_dphh8213_map(address_map &map) ATTR_COLD;

	required_device<screen_device> m_screen;
	required_device<bl_handhelds_lcdc_device> m_lcdc;
	required_ioport_array<4> m_input_matrix;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u8 m_portb;
};

class st22xx_bbl338_sim_state : public st22xx_bbl338_state
{
public:
	st22xx_bbl338_sim_state(const machine_config& mconfig, device_type type, const char* tag)
		: st22xx_bbl338_state(mconfig, type, tag)
	{
	}

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u8 sim152_r();
	void sim152_w(u8 data);
	u8 m_152_dat = 0;
};

u8 st22xx_bbl338_sim_state::sim152_r()
{
	u16 pc = m_maincpu->pc();

	if (!machine().side_effects_disabled() && pc == 0x150)
	{
		u8 command = (u8)m_maincpu->state_int(M6502_X);
		switch (command)
		{
		case 0x00: break; // command 0x00 = draw jewels in columns
		// 0x02 not seen
		case 0x04: break; // command 0x04 = draw text number
		case 0x06: break; // command 0x06 = draw boxes in sudoku
		case 0x08: break; // command 0x08 = some kind of positioning logic for things (characters in risker)
		case 0x0a:
		{
			// this is related to drawing a graphic element on dphh8213 (skipping one call causes a single menu item to not show)
			// same here?
			address_space& mainspace = m_maincpu->space(AS_PROGRAM);
			u8 param0 = mainspace.read_byte(0x100);
			u8 param1 = mainspace.read_byte(0x101);
			u8 param2 = mainspace.read_byte(0x102);
			u8 param3 = mainspace.read_byte(0x103);
			u8 param4 = mainspace.read_byte(0x104);
			u8 param5 = mainspace.read_byte(0x105);

			// Flags --- --Ff  f = flipX F = flipY, others not checked
			logerror("command 0x0a (draw?) using params Xpos: %02x Ypos: %02x ObjectNum: %02x%02x Flags: %02x Saturation: %02x\n", param0, param1, param3, param2, param4, param5);

			break;
		}
		case 0x0c: break; // command 0x0c = related to drawing platforms in risker?
		case 0x0e: break; // important for king boxing to show anything outside of char select
		case 0x10: break; // command 0x10 = clear out some line buffer for rendering?
		case 0x12: break; // command 0x12 = clear background in angry pigs?
		case 0x14: break; // command 0x14 = draw basic text (dphh8213 test mode, see 0x28 for bbl338)
		case 0x16: break; // important for collisions in risker?
		// 0x18 not seen
		case 0x1a: break; // when pause is pressed? maybe music related?
		case 0x1c: break; // unknown, little effect? maybe play music?
		// 0x1e not seen
		// 0x20 not seen
		case 0x22: break; // command 0x22 = unknown, used before 'shooting zombies' titlescreen
		case 0x24: break; // command 0x24 = play sound
		case 0x26: break; // command 0x26 = force stop sound(s)?

		 case 0x28:
		 {
			// on bbl338 this is used for the 'draw text' functionality in test mode instead of 0x14, see 0x4866 which is
			// the 'draw letter stored in A at position' function, calling this command.
			// The equivalent code in dphh8213 is at 0x43e4 instead and calls 0x14
			address_space& mainspace = m_maincpu->space(AS_PROGRAM);
			u8 param0 = mainspace.read_byte(0x100);
			u8 param1 = mainspace.read_byte(0x101);
			u8 param2 = mainspace.read_byte(0x102);
			u8 param3 = mainspace.read_byte(0x103);

			logerror("command 0x28 (draw text direct) using params Xpos: %02x Ypos: %02x char '%c' unk %02x\n", param0, param1, param2, param3);

			break;
		}

		default:
		{
			logerror("%04x: reached 0x152, need to execute BIOS simulation for command %02x\n", pc, command);
		}

		}

		//if (command == 0x00)
		//  return 0x60;
	}
	return m_152_dat;
}

void st22xx_bbl338_sim_state::sim152_w(u8 data)
{
	m_152_dat = data;
}


void st22xx_bbl338_sim_state::machine_reset()
{
	address_space& mainspace = m_maincpu->space(AS_PROGRAM);

	mainspace.install_readwrite_handler(0x0152, 0x0152, read8smo_delegate(*this, FUNC(st22xx_bbl338_sim_state::sim152_r)), write8smo_delegate(*this, FUNC(st22xx_bbl338_sim_state::sim152_w)));

	// The code that needs to be in RAM doesn't seem to be in the ROM
	const uint8_t ramcode[40] = {

		// this is the 'execute BIOS function' call
		0xa4, 0x33,       // 000150:         ldy $33 |- Push current bank onto stack
		0x5a,             // 000152:         phy     |
		0xa4, 0x32,       // 000153:         ldy $32 |
		0x5a,             // 000155:         phy     /
		0x64, 0x32,       // 000156:         stz $32 | - Zero Bank (manually optimized compared to the dphh8213 implementation to reduce code size so call to 0x0164 is correct)
		0x64, 0x33,       // 000158:         stz $33 /
		//0x20, 0x3d, 0x41, // 000152:         jsr $xxxx   -- this needs to go to a jump table to process the command stored in X
		0xea, 0xea, 0xea, // NOP above out for now as it isn't clear where to jump to
		0x7a,             // 00015d:         ply     |- restore previous bank
		0x84, 0x32,       // 00015e:         sty $32 |
		0x7a,             // 000160:         ply     |
		0x84, 0x33,       // 000161:         sty $33 /
		0x60,             // 000163:         rts

		// this is the 2nd call to RAM, the bank to call is in y/x with the address modified in the code here before calling
		0xa5, 0x33,       // 000164:         lda $33 |- store old bank on stack
		0x48,             // 000166:         pha     |
		0xa5, 0x32,       // 000167:         lda $32 |
		0x48,             // 000169:         pha     /
		0x84, 0x33,       // 00016a:         sty $33 |- set bank from X and Y  (y is always 0 in dphh8213, not here, suggesting ROM doesn't map at 0)
		0x86, 0x32,       // 00016c:         stx $32 /
		0x20, 0x00, 0x00, // 00016e:         jsr xxxx the address here is set before the call with a ldx #$93 / stx $016f, ldx #$42 / stx $0170 type sequence
		0x7a,             // 000171:         ply     |- restore old bank
		0x84, 0x32,       // 000172:         sty $32 |
		0x7a,             // 000174:         ply     |
		0x84, 0x33,       // 000175:         sty $33 /
		0x60              // 000177:         rts
	};

	for (int i = 0; i < 40; i++)
		mainspace.write_byte(0x150+i, ramcode[i]);

	// force it to boot from external space (missing internal code to copy above segment to RAM however)
	mainspace.write_byte(0x32, 0x00);
	mainspace.write_byte(0x33, 0x04);

	// force interrupts to be fetched from external space (complete?)
	mainspace.write_byte(0x30, 0x00);
	mainspace.write_byte(0x31, 0x04);

	// timers / timer interrupt init sequence
	mainspace.write_byte(0x28, 0x08);
	mainspace.write_byte(0x29, 0xc0);
	mainspace.write_byte(0x26, 0xff);
	mainspace.write_byte(0x27, 0xdf);
	mainspace.write_byte(0x3e, 0x10);
	mainspace.write_byte(0x3c, 0x00);
	mainspace.write_byte(0x39, 0x12);
}

void st22xx_bbl338_sim_state::machine_start()
{
	st22xx_bbl338_state::machine_start();
	save_item(NAME(m_152_dat));
}

void st22xx_bbl338_state::machine_start()
{
	save_item(NAME(m_portb));
}

void st22xx_bbl338_state::st22xx_dphh8213_map(address_map &map)
{
	map(0x0000000, 0x01fffff).rom().region("maincpu", 0);

	map(0x0600000, 0x0600000).w(m_lcdc, FUNC(bl_handhelds_lcdc_device::lcdc_command_w));
	map(0x0604000, 0x0604000).rw(m_lcdc, FUNC(bl_handhelds_lcdc_device::lcdc_data_r), FUNC(bl_handhelds_lcdc_device::lcdc_data_w));
}

void st22xx_bbl338_state::st22xx_bbl338_map(address_map &map)
{
	//map(0x0000000, 0x0003fff).rom().region("internal", 0); // not dumped, so ensure any accesses here are logged
	map(0x1000000, 0x11fffff).rom().region("maincpu", 0);

	map(0x0600000, 0x0600000).w(m_lcdc, FUNC(bl_handhelds_lcdc_device::lcdc_command_w));
	map(0x0604000, 0x0604000).rw(m_lcdc, FUNC(bl_handhelds_lcdc_device::lcdc_data_r), FUNC(bl_handhelds_lcdc_device::lcdc_data_w));
}

u32 st22xx_bbl338_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return m_lcdc->render_to_bitmap(screen, bitmap, cliprect);
}

u8 st22xx_bbl338_state::porta_r()
{
	u8 input = 0x3f;

	// irregular port configuration
	if (!BIT(m_portb, 0))
		input &= m_input_matrix[0]->read();
	for (int i = 1; i < 4; i++)
		if (!BIT(m_portb, i * 2 - 1))
			input &= m_input_matrix[i]->read();

	// TODO: bit 7 is I/O for some bitbanged SPI device (used for the menu)
	input |= 0xc0;

	logerror("%s: port a read\n", machine().describe_context());

	return input;
}

void st22xx_bbl338_state::porta_w(u8 data)
{
	// Menu control writes?
	logerror("%s: port a write %02x\n", machine().describe_context(), data);
}

void st22xx_bbl338_state::portb_w(u8 data)
{
//  logerror("%s: port b write %02x\n", machine().describe_context(), data);
	m_portb = data;
}

// input multiplexing not verified for bbl338
static INPUT_PORTS_START(dphh8213)
	// P2 controls work with some of the games, but there was no obvious way to connect a 2nd pad?
	// document them for now, but maybe comment them out later for accuracy.
	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("P1 A")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("P2 B")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // bbl338 - must be IP_ACTIVE_HIGH to avoid system hanging with 'wai' opcode after code turns on port interrupt if not in test mode (power off?)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("P2 A") PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_START2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(2)

	PORT_START("IN4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("P2 B") PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(2)

	PORT_START("PORTC")
	PORT_CONFNAME( 0x01,  0x01, DEF_STR( Language ) )
	PORT_CONFSETTING( 0x00, "Chinese" )
	PORT_CONFSETTING( 0x01, "English" )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xf6, IP_ACTIVE_LOW, IPT_UNUSED) // probably unused
INPUT_PORTS_END


void st22xx_bbl338_state::st22xx_dphh8213(machine_config &config)
{
	ST2302U(config, m_maincpu, 24000000);
	m_maincpu->set_addrmap(AS_DATA, &st22xx_bbl338_state::st22xx_dphh8213_map);
	m_maincpu->in_pa_callback().set(FUNC(st22xx_bbl338_state::porta_r));
	m_maincpu->out_pa_callback().set(FUNC(st22xx_bbl338_state::porta_w));

	m_maincpu->out_pb_callback().set(FUNC(st22xx_bbl338_state::portb_w));
	m_maincpu->in_pc_callback().set_ioport("PORTC");

	SPEAKER(config, "mono").front_center();
	m_maincpu->add_route(0, "mono", 1.00);
	m_maincpu->add_route(1, "mono", 1.00);
	m_maincpu->add_route(2, "mono", 1.00);
	m_maincpu->add_route(3, "mono", 1.00);


	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(160, 128);
	m_screen->set_visarea(0, 160 - 1, 0, 128 - 1);
	m_screen->set_screen_update(FUNC(st22xx_bbl338_state::screen_update));

	BL_HANDHELDS_LCDC(config, m_lcdc, 0);
}

void st22xx_bbl338_state::st22xx_bbl338(machine_config &config)
{
	ST2302U(config, m_maincpu, 24000000);
	m_maincpu->set_addrmap(AS_DATA, &st22xx_bbl338_state::st22xx_bbl338_map);
	m_maincpu->in_pa_callback().set(FUNC(st22xx_bbl338_state::porta_r));
	m_maincpu->out_pb_callback().set(FUNC(st22xx_bbl338_state::portb_w));
	m_maincpu->in_pc_callback().set_ioport("PORTC");

	// incorrect for bbl338
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(160, 128);
	m_screen->set_visarea(0, 160 - 1, 0, 128 - 1);
	m_screen->set_screen_update(FUNC(st22xx_bbl338_state::screen_update));

	// incorrect for bbl338 (or will need changes to support higher resolutions)
	BL_HANDHELDS_LCDC(config, m_lcdc, 0);
}


ROM_START( bbl338 )
	ROM_REGION( 0x4000, "internal", ROMREGION_ERASEFF )
	ROM_LOAD( "internal.rom", 0x000000, 0x4000, NO_DUMP ) // unsure of exact size for this model

	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "en29lv160ab.u1", 0x000000, 0x200000, CRC(2c73e16c) SHA1(e2c69b3534e32ef384c0c2f5618118a419326e3a) )
ROM_END

ROM_START( dphh8213 )
	// internal area not used

	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "mx29lv160cb.u1", 0x000000, 0x200000, CRC(c8e7e355) SHA1(726f28c2c9ab012a6842f9f30a0a71538741ba14) )
	ROM_FILL( 0x00009f, 2, 0xea ) // NOP out SPI check
ROM_END

ROM_START( class200 )
	// uncertain if internal area is used

	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "m29w320eb.bin", 0x000000, 0x400000, CRC(3067b5f6) SHA1(9a159b16898054a74cfb32b5c597b505132f004e) )
ROM_END

} // anonymous namespace


// this is uses a higher resolution display than the common units, but not as high as the SunPlus based ones
COMP( 201?, bbl338,   0,      0,      st22xx_bbl338, dphh8213, st22xx_bbl338_sim_state, empty_init, "BaoBaoLong", "Portable Game Player BBL-338 (BaoBaoLong, 48-in-1)", MACHINE_IS_SKELETON )
// also appears to be the higher resolution display
COMP( 201?, class200, 0,      0,      st22xx_dphh8213, dphh8213, st22xx_bbl338_state, empty_init, "<unknown>", "Color LCD Classic Game 200-in-1", MACHINE_IS_SKELETON ) // no manufacturer name or product code anywhere

// Language controlled by port bit, set at factory, low resolution
COMP( 201?, dphh8213, 0,      0,      st22xx_dphh8213, dphh8213, st22xx_bbl338_state, empty_init, "<unknown>", "Digital Pocket Hand Held System 20-in-1 - Model 8213", MACHINE_IS_SKELETON )
