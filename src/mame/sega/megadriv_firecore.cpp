// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    Firecore / RedKid 'Mega Drive' and 'Genesis' bootleg/clone based systems.

    Firecore offers some extended video modes over a regular Mega Drive
    The YM/FM implementation in the chip has a number of flaws, MAME's older
    YM/FM core was closer, but still 'too good' in many situations.  Some
    software written for Firecore relies on these flaws to sound correct.

*/

#include "emu.h"

#include "megadriv.h"

#include "megadriv_firecore_vdp.h"

namespace {

class md_firecore_state : public md_ctrl_state
{
public:
	md_firecore_state(const machine_config& mconfig, device_type type, const char* tag) :
		md_ctrl_state(mconfig, type, tag),
		m_bank(0),
		m_externalbank(0),
		m_romsize(0x400000),
		m_rom(*this, "maincpu"),
		m_io_menu(*this, "MENU"),
		m_chunk_mask{ 0, 0, 0, 0 }
	{ }

	void md_firecore(machine_config &config) ATTR_COLD;
	void md_firecore_6b(machine_config &config) ATTR_COLD;

	void init_atgame40() ATTR_COLD;
	void init_mdhh100() ATTR_COLD;
	void init_sarc110() ATTR_COLD;
	void init_reactmd() ATTR_COLD;
	void init_mahg156() ATTR_COLD;
	void init_pxp3() ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(menu_changed);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void md_firecore_map(address_map &map) ATTR_COLD;

	// RedKid (DMC RK2) style boards with up to 128MiB of flash: SoC GPIOs drive the upper flash address lines
	uint16_t redkid_rom_r(offs_t offset);
	uint16_t redkid_gpio_r(offs_t offset);
	void redkid_gpio_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void redkid_ssf2_w(offs_t offset, uint8_t data);
	uint16_t redkid_portc_r();
	uint16_t redkid_sys_r(offs_t offset);
	void redkid_sys_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t redkid_xregs_r(offs_t offset);
	void redkid_xregs_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint32_t screen_update_redkid(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t redkid_flash_chunk() const;
	bool redkid_cart_selected() const;

	void bank_high_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void bank_low_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	int m_bank;
	int m_externalbank;
	int m_romsize;

	required_region_ptr<uint16_t> m_rom;
	optional_ioport m_io_menu;

	uint16_t m_gpio[8];         // 0xa10104 - 0xa10113
	uint8_t m_ssf2_bank[8];     // built-in Sega SSF2-style mapper (0xa130f3 - 0xa130ff)
	uint16_t m_sysreg[0x80];    // 0xb01000 - 0xb010ff
	uint8_t m_vdp_is_pal;
	uint16_t m_vdpxreg[0x80];   // 0xb00000 - 0xb000ff

	uint16_t m_chunk_mask[4];   // GPIO bits (in 0xa10104) driving flash A23, A24, A25, A26
};


void md_firecore_state::bank_high_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	data &= 0x7f;
	mem_mask &= 0x7f;

	m_bank = (m_bank & 0xff80ffff) | (data & mem_mask) << 16;
	logerror("%s: bank_high_w bank is now %08x\n", machine().describe_context(), m_bank);
}

void md_firecore_state::bank_low_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_bank = (m_bank & 0xffff0000) | (data & mem_mask);
	logerror("%s: bank_low_w bank is now %08x\n", machine().describe_context(), m_bank);
}


/*
    RedKid (DMC RK2) handhelds / consoles (mahg156, pxp3, mdhh100 with 1Gbit flash, dcat16 with 8MiB)

    The SoC itself only drives A1-A22 of the flash (8MiB window, selected with 0xb01028 / 0xb0102a),
    the four upper address lines A23-A26 are driven from SoC GPIOs at 0xa10104 (data) / 0xa10106 (direction).
    After reset the GPIOs are inputs and the lines are pulled high, so the system boots from the last 8MiB
    (the MASTER menu).  The menu copies a small routine to RAM (at 0xff006e) which selects one of the 16
    8MiB chunks by driving GPIO bits 0x0002 / 0x0010 / 0x0020 / 0x0400 (the bit-to-line order is board specific).

    GPIO 0x0080 / 0x0800 (with 0xa1010c selecting GPIO or SoC function for those pins) switch the bus between
    the internal flash and the cartridge slot (the menu only does this from code running in RAM).  When probing
    the slot the menu additionally sets 0xa1010c bit 2 and 0xb01030 = 0xa130 / 0xb01032 = 0x3ffc (meaning unknown).

    The 156 games are split into three menus with their own background graphics: the default one (games 1-80) and
    two more (games 81-119 and 120-156) that are selected by pulling 0xa10006 bit 14 or bit 13 low (read inverted as
    a 3-bit value from bits 15-13, values other than 0/1/2 hang the menu).  On the PXP3 this is done with a
    cartridge that only contains a wire jumper, it is exposed as a configuration setting here.
    The 2nd entry of the PXP3 MASTER (and the 'DMC-RK2-SLV' program at 0x6000000) is DMC's Angry Birds.
*/

uint32_t md_firecore_state::redkid_flash_chunk() const
{
	const uint16_t data = m_gpio[0];
	const uint16_t dir = m_gpio[1];
	uint32_t chunk = 0;
	for (int i = 0; i < 4; i++)
	{
		const uint16_t mask = m_chunk_mask[i];
		const bool level = (dir & mask) ? bool(data & mask) : true; // inputs are pulled high
		if (level)
			chunk |= 1 << i;
	}
	return chunk;
}

bool md_firecore_state::redkid_cart_selected() const
{
	// GPIO 0x0080 driven high = internal flash disabled -> cartridge slot
	return !(m_gpio[4] & 0x0080) && (m_gpio[0] & 0x0080);
}

uint16_t md_firecore_state::redkid_rom_r(offs_t offset)
{
	const uint32_t byteaddr = offset << 1;
	uint32_t chunk = redkid_flash_chunk();

	if (redkid_cart_selected())
		return 0xffff; // empty cartridge slot

	// 512KiB pages of the SSF2 mapper, relative to the bank base
	const uint32_t page = m_ssf2_bank[(byteaddr >> 19) & 7];
	const uint32_t offs = (page << 19) | (byteaddr & 0x7ffff);

	const uint32_t addr = (chunk << 23) | ((m_bank + offs) & 0x7fffff);
	return m_rom[(addr & (m_romsize - 1)) >> 1];
}

uint16_t md_firecore_state::redkid_sys_r(offs_t offset)
{
	return m_sysreg[offset];
}

void md_firecore_state::redkid_sys_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sysreg[offset]);

	switch (offset)
	{
	case 0x0c / 2: // 0xb0100c bit 2: version register reports PAL (set by the menu from the game header region)
	case 0x1e / 2: // 0xb0101e bit 2: version register reports domestic (Japan)
	{
		const bool pal = BIT(m_sysreg[0x0c / 2], 2);
		const bool domestic = BIT(m_sysreg[0x1e / 2], 2);
		m_version_hi_nibble = 0x20 | (domestic ? 0x00 : 0x80) | (pal ? 0x40 : 0x00);

		// the VDP PAL flag and timing follow too (some European games check the VDP status instead of the version register)
		// TODO: does this really change the output, or just fake the return values?
		if (pal != bool(m_vdp_is_pal))
		{
			m_vdp_is_pal = pal;
			m_vdp->set_vdp_pal(pal);
			m_vdp->set_framerate(pal ? 50 : 60);
			m_vdp->set_total_scanlines(pal ? 313 : 262);
			m_screen->configure(m_screen->width(), pal ? 313 : 262, m_screen->visible_area(),
					attotime::from_hz(pal ? (MASTER_CLOCK_PAL.dvalue() / 10 / 313 / 342) : (MASTER_CLOCK_NTSC.dvalue() / 10 / 262 / 342)));
		}
		break;
	}

	case 0x28 / 2:
		bank_low_w(0, data, mem_mask);
		break;

	case 0x2a / 2:
		bank_high_w(0, data, mem_mask);
		break;

	case 0x36 / 2:
	{
		// all games in atgame40 that fail to display anything write 0x0001 here
		// could be coincidence, but could also be enabling the alt display mode?
		logerror("%s: redkid_sys_w %02x %04x %04x (custom mode enable?)\n", machine().describe_context(), offset, data, mem_mask);
		break;
	}

	default:
	{
		logerror("%s: redkid_sys_w %02x %04x %04x (unknown)\n", machine().describe_context(), offset, data, mem_mask);
		break;
	}

	}
}

uint16_t md_firecore_state::redkid_xregs_r(offs_t offset)
{
	logerror("%s: redkid_xregs_r %02x (unknown)\n", machine().describe_context(), offset);
	return m_vdpxreg[offset];
}

void md_firecore_state::redkid_xregs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vdpxreg[offset]);
	logerror("%s: redkid_xregs_w %02x %04x %04x (unknown)\n", machine().describe_context(), offset, data, mem_mask);
}

void md_firecore_state::redkid_ssf2_w(offs_t offset, uint8_t data)
{
	// 0xa130f3, f5 ... ff select the 512KiB page shown at 0x080000, 0x100000 ... 0x380000 (used by Super Street Fighter II)
	if ((offset & 1) && (offset >= 3))
	{
		m_ssf2_bank[offset >> 1] = data & 0x3f;
		logerror("%s: SSF2 mapper page %d = %02x\n", machine().describe_context(), offset >> 1, data);
	}
}

uint16_t md_firecore_state::redkid_gpio_r(offs_t offset)
{
	// the data register reads back the pin levels, pins configured as inputs are pulled high
	// (the menu relies on this: it switches A24 to output with a read-modify-write of the data register beforehand)
	if (offset == 0)
		return (m_gpio[0] & m_gpio[1]) | ~m_gpio[1];

	return m_gpio[offset];
}

void md_firecore_state::redkid_gpio_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	const uint32_t oldchunk = redkid_flash_chunk();
	const bool oldcart = redkid_cart_selected();
	COMBINE_DATA(&m_gpio[offset]);

	if ((oldchunk != redkid_flash_chunk()) || (oldcart != redkid_cart_selected()))
		logerror("%s: GPIO %06x = %04x -> flash chunk %x (%08x)%s\n", machine().describe_context(), 0xa10104 + offset * 2, m_gpio[offset],
				redkid_flash_chunk(), redkid_flash_chunk() << 23, redkid_cart_selected() ? " [cartridge slot]" : "");
}


uint32_t md_firecore_state::screen_update_redkid(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!dynamic_cast<megadriv_firecore_vdp_device&>(*m_vdp).xvdp_enabled())
		return screen_update_megadriv(screen, bitmap, cliprect);

	bitmap.fill(rgb_t::black(), cliprect);

	dynamic_cast<megadriv_firecore_vdp_device&>(*m_vdp).draw_firecore(bitmap, cliprect);

	return 0;
}

INPUT_CHANGED_MEMBER(md_firecore_state::menu_changed)
{
	// the jumper is only checked by the menu at startup
	machine().schedule_soft_reset();
}

uint16_t md_firecore_state::redkid_portc_r()
{
	// bits 15-13 of the word read are menu select inputs (pulled high)
	uint16_t data = m_ioports[2]->data_r();
	data |= data << 8;
	return (data & 0x1fff) | (m_io_menu ? (m_io_menu->read() & 0xe000) : 0xe000);
}

void md_firecore_state::md_firecore_map(address_map &map)
{
	megadriv_68k_base_map(map);

	map(0x000000, 0x3fffff).r(FUNC(md_firecore_state::redkid_rom_r));

	map(0xc00000, 0xc0001f).rw(m_vdp, FUNC(megadriv_firecore_vdp_device::xvdp_r), FUNC(megadriv_firecore_vdp_device::xvdp_w));

	map(0xa10006, 0xa10007).r(FUNC(md_firecore_state::redkid_portc_r));
	map(0xa10104, 0xa10113).rw(FUNC(md_firecore_state::redkid_gpio_r), FUNC(md_firecore_state::redkid_gpio_w));

	map(0xa130f0, 0xa130ff).w(FUNC(md_firecore_state::redkid_ssf2_w));

	map(0xb00000, 0xb000ff).rw(FUNC(md_firecore_state::redkid_xregs_r), FUNC(md_firecore_state::redkid_xregs_w));
	map(0xb01000, 0xb010ff).rw(FUNC(md_firecore_state::redkid_sys_r), FUNC(md_firecore_state::redkid_sys_w));
}

INPUT_PORTS_START( mympac )
	PORT_START("PAD1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Coin Button 1") // not coin slots
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Coin Button 2")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PAD2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( firecore_6button )
	PORT_INCLUDE( md_common )

	PORT_MODIFY("PAD1") // Extra buttons for Joypad 1 (6 button + start + mode)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("%p Z")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("%p Y")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("%p X")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SELECT )  PORT_PLAYER(1) PORT_NAME("%p Mode")

	PORT_MODIFY("PAD2") // Extra buttons for Joypad 2 (6 button + start + mode)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("%p Z")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("%p Y")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("%p X")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SELECT )  PORT_PLAYER(2) PORT_NAME("%p Mode")
INPUT_PORTS_END

static INPUT_PORTS_START( msi_6b )
	PORT_INCLUDE( firecore_6button )

	PORT_MODIFY("PAD2") // no 2nd pad
	PORT_BIT( 0x0fff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("RESET") // RESET button on controller to the left of START
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1) PORT_NAME("Reset")
INPUT_PORTS_END

static INPUT_PORTS_START( mahg156 )
	PORT_INCLUDE( firecore_6button )

	// read back inverted from 0xa10006 bits 15-13 by the menu at startup, selects which part of the game list is shown
	PORT_START("MENU")
	PORT_CONFNAME( 0xe000, 0xe000, "Menu" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(md_firecore_state::menu_changed), 0)
	PORT_CONFSETTING(      0xe000, "Games 1-156" )
	// this doesn't actually have a cartridge slot, these are leftover menus from the PXP, all games are available in the first menu
	//PORT_CONFSETTING(      0xa000, "Games 81-119" )
	//PORT_CONFSETTING(      0xc000, "Games 120-156" )
INPUT_PORTS_END

static INPUT_PORTS_START( pxp3 )
	PORT_INCLUDE( md_common )

	// read back inverted from 0xa10006 bits 15-13 by the menu at startup, selects which part of the game list is shown
	PORT_START("MENU")
	PORT_CONFNAME( 0xe000, 0xe000, "Menu (cartridge jumper)" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(md_firecore_state::menu_changed), 0)
	PORT_CONFSETTING(      0xe000, "Games 1-80 (no cartridge, 999888-in-1)" )
	PORT_CONFSETTING(      0xa000, "Games 81-119 (cartridge, 999666-in-1)" )
	PORT_CONFSETTING(      0xc000, "Games 120-156 (cartridge, 900000-in-1)" )
INPUT_PORTS_END


void md_firecore_state::machine_start()
{
	md_ctrl_state::machine_start();
	m_vdp->stop_timers();
	save_item(NAME(m_bank));
	save_item(NAME(m_externalbank));
	save_item(NAME(m_romsize));
	save_item(NAME(m_gpio));
	save_item(NAME(m_ssf2_bank));
	save_item(NAME(m_sysreg));
	save_item(NAME(m_vdpxreg));
	save_item(NAME(m_vdp_is_pal));
}

void md_firecore_state::machine_reset()
{
	m_vdp_is_pal = 0;
	m_bank = 0;
	std::fill(std::begin(m_gpio), std::end(m_gpio), 0);
	for (int i = 0; i < 8; i++)
		m_ssf2_bank[i] = i;
	std::fill(std::begin(m_sysreg), std::end(m_sysreg), 0);
	std::fill(std::begin(m_vdpxreg), std::end(m_vdpxreg), 0);

	md_ctrl_state::machine_reset();
}


void md_firecore_state::md_firecore(machine_config &config)
{
	md_ntsc(config);

	FIRECORE_VDP(config.replace(), m_vdp, MASTER_CLOCK_NTSC, m_maincpu);
	m_vdp->set_is_pal(false);
	m_vdp->vint_cb().set(FUNC(md_firecore_state::vdp_vint_cb));
	m_vdp->hint_cb().set(FUNC(md_firecore_state::vdp_hint_cb));
	m_vdp->set_screen("megadriv");
	m_vdp->snd_irq().set_inputline(m_z80snd, 0);
	m_vdp->add_route(ALL_OUTPUTS, "speaker", 0.50, 0);
	m_vdp->add_route(ALL_OUTPUTS, "speaker", 0.50, 1);
	m_vdp->set_use_cram(1);
	m_vdp->set_vdp_pal(false);
	m_vdp->set_framerate(60);
	m_vdp->set_total_scanlines(262);

	ctrl1_3button(config);
	ctrl2_3button(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &md_firecore_state::md_firecore_map);
	m_screen->set_screen_update(FUNC(md_firecore_state::screen_update_redkid));
}

void md_firecore_state::md_firecore_6b(machine_config &config)
{
	md_firecore(config);

	ctrl1_6button(config);
	ctrl2_6button(config);
}



ROM_START( matet )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "tetrismyarcade_s29gl032n90tfi04_0001227e.bin", 0x000000, 0x400000, CRC(b6ccb0b2) SHA1(12ee130b7e332910c0bbbedd93d55779c237a66d) )
ROM_END

ROM_START( mateta )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "testris_s29gl032m90tfir4_0001227e.bin", 0x000000, 0x400000, CRC(5af6c866) SHA1(2988af5c7c0133aeced289d8a0a3b59c9b20a372) )
ROM_END

ROM_START( mypacgld )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "am29lv160db.u2", 0x000000, 0x200000, CRC(7a01cf42) SHA1(c9fb66f1d38710aae6b94c1fbc99a8763730cfce) )
ROM_END

ROM_START( mypac )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "en29lb160bb.bin", 0x000000, 0x200000, CRC(d741a601) SHA1(a8d89034458b14c5cea83980be5400b82081b274) )
ROM_END

ROM_START( mypaca )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "pacmanarcade_s29gl064n90tfi04_0001227e.bin", 0x000000, 0x800000, CRC(7beb8dd1) SHA1(040d93f392b7c113a812e620641f48998701efba) )
ROM_END

ROM_START( mympac )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "m29w640ft.bin", 0x000000, 0x800000, CRC(d6ceda9e) SHA1(c897f8d5661fea0c030daf9c5e92524eb4e71d52) )
ROM_END

ROM_START( mympaca )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "s29gl064n90tfi04.u2", 0x000000, 0x800000, CRC(0a16c487) SHA1(f6f32ae43e42c16a39a81bff7e7167833d4e738a) )
ROM_END

ROM_START( mygalag )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "galaga_standup_s29al016d70tfi02_00012249.bin", 0x000000, 0x200000, CRC(8f3d2e05) SHA1(8f6a54e5a8ee55e7a6cae3e72b8e70c4eee2c1ef) )
ROM_END

ROM_START( mygalaga )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "galaga_s29jl032h70tfi01_0001227e.bin", 0x000000, 0x400000, CRC(e775089a) SHA1(0938afa8e92a8c77b4fb86e0ec044fbb2b572570) )
ROM_END

ROM_START( mygalagb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "s29gl032n90tfi03.u2", 0x000000, 0x400000, CRC(b59a22fb) SHA1(55abc30f1502ccdb5b2f3a85b23f7762df3f63c9) )

	// has a P24C64C (for storing scores?)
ROM_END

ROM_START( mysinv )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "spaceinvaders_s29gl0640n90tfi04_0001227e.bin", 0x000000, 0x800000, CRC(1d062cf5) SHA1(bc6bcce398c37e19be0d72e0e8fabb245fb54043) )
ROM_END

ROM_START( mysinva )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "s29gl032n90tfi03.u2", 0x000000, 0x400000, CRC(57480d73) SHA1(743b4a56ddbf61eef46ee4b2d4581cd44a67ba9b) )

	// has a P24C64C (for storing scores?)
ROM_END

ROM_START( mysf2 )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "s29gl128.u2", 0x000000, 0x1000000, CRC(7938a212) SHA1(306cffd048fb271fa26cd2f61cbd2c3a94116292) )
ROM_END


/*

As the atgame40 is not running on standard MegaDrive hardware quite a few of these games rely
on an unsupported video mode (they still play sounds and resond to inputs / do colour fades)

00 Menu                             located at 00800000  WORKS
--
01 Air Hockey                       located at 0000c800  WORKS
02 Black Sheep                      located at 0002935a  BOOTS - NO DISPLAY
03 Bomber                           located at 000b6f5a  WORKS
04 Bottle Taps Race                 located at 000d6f5a  WORKS
05 Brain Switch                     located at 000ec760  BOOTS - NO DISPLAY
06 Bulls and Cows                   located at 00800000  BOOTS - NO DISPLAY
07 Cannon                           located at 0013e360  WORKS
08 Checker                          located at 0016301e  WORKS
09 Chess                            located at 001aa232  WORKS
10 Colour Puzzle                    located at 001f0a32  BOOTS - NO DISPLAY
11 Cross The Road                   located at 008a2800  BOOTS - NO DISPLAY
12 Curling 2010                     located at 00953486  BOOTS - NO DISPLAY
13 Fight or Lose                    located at 009f002e  WORKS
14 Fire Fly Glow                    located at 00a09ede  BOOTS - NO DISPLAY
15 Fish Story                       located at 00aa2ede  BOOTS - NO DISPLAY
16 Flash Memory                     located at 00285632  BOOTS - NO DISPLAY
17 Formula Challenge                located at 00310232  BOOTS - NO DISPLAY
18 Hexagons                         located at 00394a32  WORKS
19 Jacks Pea                        located at 003b4a32  BOOTS - NO DISPLAY
20 Jewel Magic                      located at 00b4a874  BOOTS - NO DISPLAY
21 Logic Dial                       located at 0040b4ac  BOOTS - NO DISPLAY
22 Table Magic                      located at 00be8f8c  BOOTS - NO DISPLAY
23 Mahjong                          located at 0049e8ac  WORKS
24 Match Eleven                     located at 004ae8ac  BOOTS - NO DISPLAY
25 Mega Brain Switch                located at 005390ac  BOOTS - NO DISPLAY
26 Memory                           located at 0058a0ac  WORKS
27 Memory Match                     located at 00c8538c  BOOTS - NO DISPLAY
28 Mirror Mirror                    located at 00d2178c  BOOTS - NO DISPLAY
29 Mr Balls                         located at 0059a0ac  WORKS
30 Navel Power                      located at 005c0a08  WORKS
31 Panic Lift                       located at 00d86f8c  BOOTS - NO DISPLAY
32 Reaction Match                   located at 00e2038c  BOOTS - NO DISPLAY
33 Snake                            located at 005dbbd4  WORKS
34 Space Hunter                     located at 005ebbd4  BOOTS - NO DISPLAY
35 Spider                           located at 006817d4  WORKS
36 Sudoku Quiz                      located at 0069efb4  BOOTS - NO DISPLAY
37 Treasure Hunt                    located at 00eb9b8c  BOOTS - NO DISPLAY
38 UFO Sighting                     located at 00f5938c  BOOTS - NO DISPLAY
39 Warehouse Keeper                 located at 00730fb4  WORKS
40 Whack a Wolf                     located at 00740fb4  BOOTS - NO DISPLAY
*/

ROM_START( atgame40 )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "40bonusgamesin1.bin", 0x000000, 0x1000000, CRC(4eba6e83) SHA1(b8edf1b6ecb70a136b551f1454ba8afa45bd8bc1) )
ROM_END


ROM_START(dcat16)
	ROM_REGION(0x1000000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "mg6025.u1", 0x0000,  0x800000, CRC(5453d673) SHA1(b9f8d849cbed81fe73525229f4897ccaeeb7a833) )
	ROM_RELOAD(0x800000,0x800000)
ROM_END


ROM_START(mahg156)
	ROM_REGION(0x8000000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "md156.u3", 0x0000,  0x8000000, CRC(665fc68c) SHA1(6b765f96716c4a0abf3d27252ec82be6b0d9a985) )
//  the Megadrive ROMs for the most part appear to be hacked versions of the games / old scene dumps
//  some are region locked to differing regions (not all games present in ROM appear on the menu)
//  ROM_COPY( "maincpu", 0x0000000, 0, 0x080000) // FORGOTTEN WORLDS
//  ROM_COPY( "maincpu", 0x0080000, 0, 0x080000) // FIRE PRO WRESTLING
//  ROM_COPY( "maincpu", 0x0100000, 0, 0x080000) // GHOST BUSTERS
//  ROM_COPY( "maincpu", 0x0180000, 0, 0x080000) // DICK TRACY
//  ROM_COPY( "maincpu", 0x0200000, 0, 0x080000) // DEVIL CRASH
//  ROM_COPY( "maincpu", 0x0280000, 0, 0x080000) // DECAP ATTACK
//  ROM_COPY( "maincpu", 0x0300000, 0, 0x080000) // DARWIN 4081
//  ROM_COPY( "maincpu", 0x0380000, 0, 0x080000) // CRACK DOWN
//  ROM_COPY( "maincpu", 0x0400000, 0, 0x080000) // CAPTAIN PLANET
//  ROM_COPY( "maincpu", 0x0480000, 0, 0x080000) // CALIFORNIA GAMES
//  ROM_COPY( "maincpu", 0x0500000, 0, 0x080000) // CADASH
//  ROM_COPY( "maincpu", 0x0580000, 0, 0x080000) // BOOGIE WOOGIE BOWLING
//  ROM_COPY( "maincpu", 0x0600000, 0, 0x080000) // BIMINI RUN
//  ROM_COPY( "maincpu", 0x0700000, 0, 0x080000) // BATTLE TOADS
//  ROM_COPY( "maincpu", 0x0780000, 0, 0x080000) // TROUBLE SHOOTER
//  ROM_COPY( "maincpu", 0x0800000, 0, 0x080000) // BURNING FORCE
//  ROM_COPY( "maincpu", 0x0880000, 0, 0x080000) // FAERY TALE ADVENTURE
//  ROM_COPY( "maincpu", 0x0900000, 0, 0x080000) // E-SWAT
//  ROM_COPY( "maincpu", 0x0980000, 0, 0x080000) // ELEMENTAL MASTER
//  ROM_COPY( "maincpu", 0x0a00000, 0, 0x080000) // EA HOCKEY
//  ROM_COPY( "maincpu", 0x0a80000, 0, 0x080000) // DARK CASTLE
//  ROM_COPY( "maincpu", 0x0b00000, 0, 0x080000) // CYBORG JUSTICE (CENSOR)
//  ROM_COPY( "maincpu", 0x0b80000, 0, 0x080000) // LITTLE MERMAID
//  ROM_COPY( "maincpu", 0x0c00000, 0, 0x080000) // DORAEMON
//  ROM_COPY( "maincpu", 0x0c80000, 0, 0x080000) // SONIC
//  ROM_COPY( "maincpu", 0x0d00000, 0, 0x080000) // WANI WANI WORLD
//  ROM_COPY( "maincpu", 0x0d80000, 0, 0x080000) // GOLDEN AXE 2
//  etc.
ROM_END

// PXP3 16-bit handhelds with cartridge slot, several revisions with different game selections (dumps named after the case colour)
ROM_START(pxp3)
	ROM_REGION(0x8000000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "pxp3_black.bin", 0x0000,  0x8000000, CRC(5c29ad76) SHA1(a377cf704d17a084537211e22c94f086628cd642) )
ROM_END

ROM_START(pxp3a)
	ROM_REGION(0x8000000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "pxp3_blue.bin", 0x0000,  0x8000000, CRC(f5e36263) SHA1(98f551358be92591fa34304d97fbf48482801123) )
ROM_END

ROM_START(mdhh100)
	ROM_REGION(0x8000000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "s29gl01gp11tfir1.u13", 0x0000,  0x8000000, CRC(564ab33a) SHA1(e455aaa9ed6f302d1ebe55b5202f983af612c415) )
ROM_END

ROM_START( msi_sf2 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "29lv320.bin", 0x000000, 0x400000, CRC(465b12f0) SHA1(7a058f6feb4f08f56ae0f7369c2ca9a9fe2ed40e) )
ROM_END

ROM_START( bfmk )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "s29gl032.u2", 0x000000, 0x400000, CRC(d5346d32) SHA1(7aec036436e9941050ae33be3ac7d734cedf2de7) )
ROM_END

ROM_START( reactmd )
	ROM_REGION( 0x2000000, "maincpu", ROMREGION_ERASE00 ) // this contains the MD games and main boot menu
	ROM_LOAD16_WORD_SWAP( "reactormd.bin", 0x0000, 0x2000000, CRC(fe9664a4) SHA1(d475b524f576c9d1d90aed20c7467cc652396baf) )

	ROM_REGION( 0x4000000, "sunplus", ROMREGION_ERASE00 ) // this contains the SunPlus games
	ROM_LOAD16_WORD_SWAP( "reactor_md_sunplus-full.bin", 0x0000, 0x4000000, CRC(843aa58c) SHA1(07cdc6d4aa0057939c145ece01a9aca73c7f1f2b) )
	ROM_IGNORE(0x4000000) // the 2nd half of the ROM can't be accessed by the PCB (address line tied low) (contains garbage? data)
ROM_END

ROM_START( sarc110 )
	ROM_REGION( 0x1000000, "maincpu", 0 ) // Mega Drive part
	ROM_LOAD16_WORD_SWAP( "superarcade.bin", 0x000000, 0x1000000, CRC(be732867) SHA1(3857b2fbddd6a548c81caf64122e47a0df079be5) )

	ROM_REGION( 0x400000, "mainrom", 0 ) // VT02/03 part
	ROM_LOAD( "ic1.prg", 0x00000, 0x400000, CRC(de76f71f) SHA1(ff6b37a76c6463af7ae901918fc008b4a2863951) )
ROM_END

ROM_START( sarc110a )
	ROM_REGION( 0x1000000, "maincpu", 0 ) // Mega Drive part
	ROM_LOAD16_WORD_SWAP( "superarcade.bin", 0x000000, 0x1000000, CRC(be732867) SHA1(3857b2fbddd6a548c81caf64122e47a0df079be5) )

	ROM_REGION( 0x400000, "mainrom", 0 ) // VT02/03 part
	ROM_LOAD( "ic1_ver2.prg", 0x00000, 0x400000, CRC(b97a0dc7) SHA1(bace32d73184df914113de5336e29a7a6f4c03fa) )
ROM_END

void md_firecore_state::init_atgame40()
{
	m_romsize = memregion("maincpu")->bytes();
	init_megadrie();
}

void md_firecore_state::init_sarc110()
{
	m_romsize = memregion("maincpu")->bytes();
	m_externalbank = 0x800000;
	init_megadrie();
}

void md_firecore_state::init_reactmd()
{
	m_romsize = memregion("maincpu")->bytes();
	m_externalbank = 0x1800000;
	init_megadrie();

	// decryption of the SunPlus part
	uint16_t *ROM = (uint16_t*)memregion("sunplus")->base();
	int size = memregion("sunplus")->bytes();

	for (int i = 0; i < size/2; i++)
	{
		ROM[i] = bitswap<16>(ROM[i], 15, 13, 14, 12,  7,  6,  5,  4,
									 11, 10, 9,  8,   3,  1,  2,  0);

		ROM[i] = ROM[i] ^ 0xa5a5;
	}
}

void md_firecore_state::init_mahg156()
{
	m_romsize = memregion("maincpu")->bytes();
	m_chunk_mask[0] = 0x0400; // A23
	m_chunk_mask[1] = 0x0020; // A24
	m_chunk_mask[2] = 0x0002; // A25
	m_chunk_mask[3] = 0x0010; // A26
	init_megadriv();
}

void md_firecore_state::init_pxp3() // also mdhh100, dcat16
{
	m_romsize = memregion("maincpu")->bytes();
	m_chunk_mask[0] = 0x0020; // A23
	m_chunk_mask[1] = 0x0400; // A24
	m_chunk_mask[2] = 0x0002; // A25
	m_chunk_mask[3] = 0x0010; // A26
	init_megadriv();
}

void md_firecore_state::init_mdhh100()
{
	m_romsize = memregion("maincpu")->bytes();
	m_externalbank = m_romsize-0x800000;
	init_megadriv();
}

} // anonymous namespace

// Games below have a device at b0102x which appears to either be able to select ROM base on a byte boundary
// OR maybe are running from RAM instead of ROM (with an auto-copy at the start?) with that being a DMA operation.

// Technically this is a MD type cartridge, but it doesn't seem to be designed for use with a standard MD as it contains
// nothing but the 16Mbyte ROM and a 5v to 3.3v converter yet the code clearly requires some extensive banking logic.
// Testing it on a real MD shows nothing, not even the menu.
//
// We don't seem to emulate the system it's designed for, so for now just treat it as its own thing (which may become
// the basis of a driver for that console)
//
// due to differences in the SoC compared to real MD hardware (including sound + new video modes) these have been left
// as NOT WORKING for now although some games run to a degree

CONS( 2020, mypacgld,  0,        0, md_firecore,    mympac,    md_firecore_state, init_megadriv, "dreamGEAR", "My Arcade Pac-Man 40th Anniversary (DGUNL-3290 / TUV-01, Micro Player)", MACHINE_NOT_WORKING )

CONS( 2021, mypac,     0,        0, md_firecore,    mympac,    md_firecore_state, init_megadriv, "dreamGEAR", "My Arcade Pac-Man (DGUNL-4198, Pocket Player Pro)", MACHINE_NOT_WORKING | ROT270 )
CONS( 2021, mypaca,    mypac,    0, md_firecore,    mympac,    md_firecore_state, init_megadriv, "dreamGEAR", "My Arcade Pac-Man (DGUNL-4194, Micro Player Pro)", MACHINE_NOT_WORKING | ROT270 )

CONS( 2021, mympac,    0,        0, md_firecore,    mympac,    md_firecore_state, init_megadriv, "dreamGEAR", "My Arcade Ms. Pac-Man (DGUNL-7010, Pocket Player Pro)", MACHINE_NOT_WORKING | ROT270 )
CONS( 2021, mympaca,   mympac,   0, md_firecore,    mympac,    md_firecore_state, init_megadriv, "dreamGEAR", "My Arcade Ms. Pac-Man (DGUNL-7023, Nano Player Pro)", MACHINE_NOT_WORKING | ROT270 )

// menu uses unsupported extended mode
CONS( 2021, mygalag,   0,        0, md_firecore,    mympac,    md_firecore_state, init_megadriv, "dreamGEAR", "My Arcade Galaga (DGUNL-4195, Micro Player Pro)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | ROT270 )
CONS( 2021, mygalaga,  mygalag,  0, md_firecore,    mympac,    md_firecore_state, init_megadriv, "dreamGEAR", "My Arcade Galaga (DGUNL-4199, Pocket Player Pro)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | ROT270 )
CONS( 2021, mygalagb,  mygalag,  0, md_firecore,    mympac,    md_firecore_state, init_megadriv, "dreamGEAR", "My Arcade Galaga (DGUNL-7132, Pixel Pocket Pro)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | ROT270 )

CONS( 2021, mysinv,    0,        0, md_firecore,    mympac,    md_firecore_state, init_megadriv, "dreamGEAR", "My Arcade Space Invaders (DGUNL-7006, Pocket Player Pro)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | ROT90 )
CONS( 2021, mysinva,   mysinv,   0, md_firecore,    mympac,    md_firecore_state, init_megadriv, "dreamGEAR", "My Arcade Space Invaders (DGUNL-7133, Pixel Pocket Pro)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | ROT90 )

CONS( 2021, mysf2,     0,        0, md_firecore_6b, msi_6b,    md_firecore_state, init_mdhh100,  "dreamGEAR", "My Arcade Street Fighter II (DGUNL-4184, Nano Player Pro)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2012, atgame40,  0,        0, md_firecore,    md_common, md_firecore_state, init_atgame40, "AtGames",   "40 Bonus Games in 1 (AtGames)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2021, matet,     0,        0, md_firecore,    md_common, md_firecore_state, init_megadriv, "dreamGEAR", "My Arcade Tetris (DGUNL-7028, Pocket Player Pro)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2021, mateta,    matet,    0, md_firecore,    md_common, md_firecore_state, init_megadriv, "dreamGEAR", "My Arcade Tetris (DGUNL-7025, Micro Player Pro)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// has an SD card slot? (the menu has an 'SD CARTRIDGE' entry, not emulated)
// 8MiB flash only, menu at 0 (DMC-RK2-MST), the built-in games use the Firecore extended video mode
CONS( 200?, dcat16,    0,        0, md_firecore,    md_common, md_firecore_state, init_pxp3,     "Firecore",  "D-CAT16 (Firecore handheld)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// RedKid (DMC RK2) based, 1Gbit flash, menu at 0x7800000
// there are PXP3 Slim Stations on other SoCs too
// these ones have the volume control and power switch at the bottom
// game code is from the first menu, other menus have their own codes (which seem to be the same between units, even when the menu differs)
CONS( 201?, pxp3,      0,        0, md_firecore,    pxp3,      md_firecore_state, init_pxp3,     "<unknown>", "PXP3 Slim Station (Firecore, NFAQ877666)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 201?, pxp3a,     pxp3,     0, md_firecore,    pxp3,      md_firecore_state, init_pxp3,     "<unknown>", "PXP3 Slim Station (Firecore, NFS877666)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// the 156 games are split into three menus selected with a 'jumper' cartridge (see notes above)
CONS( 201?, mahg156,   0,        0, md_firecore_6b, mahg156,   md_firecore_state, init_mahg156,  "<unknown>", "Mini Arcade Handheld Game Console 2.8 Inch Screen Built in 156 Retro Games (Firecore)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// game-boy like handheld, pink in colour, 6 button controller (+ home select, start, vol buttons)
CONS( 201?, mdhh100,   0,        0, md_firecore_6b, msi_6b,    md_firecore_state, init_pxp3,     "<unknown>", "unknown 100-in-1 handheld (Firecore)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// From a European unit but NTSC? - code is hacked from original USA Genesis game with region check still intact? (does the clone hardware always identify as such? or does the bypassed boot code skip the check?)
CONS( 2018, msi_sf2,   0,        0, md_firecore_6b, msi_6b,    md_firecore_state, init_megadriv, "MSI / Capcom / Sega", "Street Fighter II: Special Champion Edition (MSI Plug & Play) (Europe)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// the Firecore video mode part doesn't work correctly, game can boot if you skip over it
CONS( 2019, bfmk,      0,        0, md_firecore_6b, msi_6b,    md_firecore_state, init_megadriv, "Basic Fun", "Mortal Kombat: Arcade Classics", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// Two systems in one unit - Firecore Genesis and SunPlus
CONS( 2009, reactmd,   0,        0, md_firecore,    md_common, md_firecore_state, init_reactmd,  "AtGames / Sega / Waixing", "Reactor MD (Firecore/SunPlus hybrid, PAL)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// Two systems in one unit - Firecore Genesis and VT02/VT03
CONS( 200?, sarc110,   0,        0, md_firecore,    md_common, md_firecore_state, init_sarc110,  "<unknown>", "Super Arcade 101-in-1 (Firecore/VT hybrid, set 1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, sarc110a,  sarc110,  0, md_firecore,    md_common, md_firecore_state, init_sarc110,  "<unknown>", "Super Arcade 101-in-1 (Firecore/VT hybrid, set 2)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
