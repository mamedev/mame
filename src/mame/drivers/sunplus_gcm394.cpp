// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
    SunPlus unSP based hardware, SPG-??? (6xx?) (die is GCM394)
	
    Compared to vii.cpp this is clearly newer, has extra opcodes, different internal map etc. also scaling and higher resolutions based on Spongebob

        Smart Fit Park
        SpongeBob SquarePants Bikini Bottom 500
        Spiderman - The Masked Menace 'Spider Sense' (pad type with Spiderman model)
        (Wireless Hunting? - maybe, register map looks the same even if it sets stack to 2fff not 6fff)

    as these use newer opcodes in the FExx range they probably need a derived unSP type too
*/

#include "emu.h"

#include "machine/sunplus_gcm394.h"

#include "screen.h"
#include "speaker.h"


class gcm394_game_state : public driver_device
{
public:
	gcm394_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_bank(*this, "cartbank"),
		m_io_p1(*this, "P1"),
		m_io_p2(*this, "P2"),
		m_romregion(*this, "maincpu")
	{
	}

	void base(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void switch_bank(uint32_t bank);

	required_device<sunplus_gcm394_device> m_maincpu;
	required_device<screen_device> m_screen;

	optional_memory_bank m_bank;

	required_ioport m_io_p1;
	required_ioport m_io_p2;

	virtual void mem_map_4m(address_map &map);

	required_region_ptr<uint16_t> m_romregion;
private:

	uint32_t m_current_bank;
	int m_numbanks;

	DECLARE_READ16_MEMBER(porta_r);
	DECLARE_READ16_MEMBER(portb_r);
	DECLARE_WRITE16_MEMBER(porta_w);

	virtual DECLARE_WRITE16_MEMBER(mapping_w) {}

	virtual DECLARE_READ16_MEMBER(read_external_space);
	virtual DECLARE_WRITE16_MEMBER(write_external_space);
};

class wrlshunt_game_state : public gcm394_game_state
{
public:
	wrlshunt_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		gcm394_game_state(mconfig, type, tag),
		m_mapping(0),
		m_mainram(*this, "mainram")
	{
	}

	void wrlshunt(machine_config &config);

protected:
	//virtual void machine_start() override;
	//virtual void machine_reset() override;

	void wrlshunt_map(address_map &map);
	
private:

	DECLARE_READ16_MEMBER(hunt_porta_r);
	DECLARE_WRITE16_MEMBER(hunt_porta_w);

	virtual DECLARE_WRITE16_MEMBER(mapping_w) override;
	uint16_t m_mapping;

	required_shared_ptr<u16> m_mainram;

	virtual DECLARE_READ16_MEMBER(read_external_space) override;
	virtual DECLARE_WRITE16_MEMBER(write_external_space) override;
};

READ16_MEMBER(gcm394_game_state::read_external_space)
{
	//logerror("reading offset %04x\n", offset * 2);
	return m_romregion[offset];
}

WRITE16_MEMBER(gcm394_game_state::write_external_space)
{
	logerror("DMA writing to external space (RAM?) %08x %04x\n", offset, data);
}

WRITE16_MEMBER(wrlshunt_game_state::mapping_w)
{
	m_mapping = data;
	printf("change mapping %04x\n", data);
}

READ16_MEMBER(wrlshunt_game_state::read_external_space)
{
	if (m_mapping == 0x7f8a)
	{
	//logerror("reading offset %04x\n", offset * 2);
		return m_romregion[offset];
	}
	else if (m_mapping == 0x008a)
	{
		address_space& mem = m_maincpu->space(AS_PROGRAM);
		uint16_t retdata = mem.read_word(offset + 0x20000);
		logerror("reading from RAM instead offset %08x returning %04x\n", offset * 2, retdata);
		return retdata;
	}
	else
	{
		uint16_t retdata = 0x0000;
		logerror("reading from unknown source instead offset %08x returning %04x\n", offset * 2, retdata);
		return retdata;
	}
}




WRITE16_MEMBER(wrlshunt_game_state::write_external_space)
{
//	logerror("DMA writing to external space (RAM?) %08x %04x\n", offset, data);

	if (offset & 0x0800000)
	{
		offset &= 0x03fffff;

		if (offset < 0x03d0000)
		{
			m_mainram[offset] = data;
			//logerror("DMA writing to external space (RAM?) %08x %04x\n", offset, data);

		}
		else
		{
			logerror("DMA writing to external space (RAM?) (out of bounds) %08x %04x\n", offset, data);
		}
	}
	else
	{
		logerror("DMA writing to external space (RAM?) (unknown handling) %08x %04x\n", offset, data);
	}
}



READ16_MEMBER(gcm394_game_state::porta_r)
{
	uint16_t data = m_io_p1->read();
	logerror("Port A Read: %04x\n", data);
	return data;
}

READ16_MEMBER(gcm394_game_state::portb_r)
{
	uint16_t data = m_io_p2->read();
	logerror("Port B Read: %04x\n", data);
	return data;
}

WRITE16_MEMBER(gcm394_game_state::porta_w)
{
	logerror("%s: Port A:WRITE %04x\n", machine().describe_context(), data);
}	


void gcm394_game_state::base(machine_config &config)
{
	GCM394(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &gcm394_game_state::mem_map_4m);
	m_maincpu->porta_in().set(FUNC(gcm394_game_state::porta_r));
	m_maincpu->portb_in().set(FUNC(gcm394_game_state::portb_r));
	m_maincpu->porta_out().set(FUNC(gcm394_game_state::porta_w));
	m_maincpu->space_read_callback().set(FUNC(gcm394_game_state::read_external_space));
	m_maincpu->space_write_callback().set(FUNC(gcm394_game_state::write_external_space));
	m_maincpu->set_irq_acknowledge_callback(m_maincpu, FUNC(sunplus_gcm394_base_device::irq_vector_cb));
	m_maincpu->mapping_write_callback().set(FUNC(gcm394_game_state::mapping_w));	

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update("maincpu", FUNC(sunplus_gcm394_device::screen_update));
	m_screen->screen_vblank().set(m_maincpu, FUNC(sunplus_gcm394_device::vblank));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	m_maincpu->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_maincpu->add_route(ALL_OUTPUTS, "rspeaker", 0.5);

}

READ16_MEMBER(wrlshunt_game_state::hunt_porta_r)
{
	uint16_t data = m_io_p1->read();
	logerror("%s: Port A Read: %04x\n",  machine().describe_context(), data);
	return data;
}

WRITE16_MEMBER(wrlshunt_game_state::hunt_porta_w)
{
	logerror("%s: Port A:WRITE %04x\n", machine().describe_context(), data);

	// skip check (EEPROM?)
	if (m_mainram[0x5b354 - 0x30000] == 0xafd0)
		m_mainram[0x5b354 - 0x30000] = 0xB403;
}


void wrlshunt_game_state::wrlshunt(machine_config &config)
{
	gcm394_game_state::base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &wrlshunt_game_state::wrlshunt_map);

	m_maincpu->porta_in().set(FUNC(wrlshunt_game_state::hunt_porta_r));
	m_maincpu->porta_out().set(FUNC(wrlshunt_game_state::hunt_porta_w));

}

void gcm394_game_state::switch_bank(uint32_t bank)
{
	if (!m_bank)
		return;

	if (bank != m_current_bank)
	{
		m_current_bank = bank;
		m_bank->set_entry(bank);
		m_maincpu->invalidate_cache();
	}
}

void gcm394_game_state::machine_start()
{
	if (m_bank)
	{
		int i;
		for (i = 0; i < (m_romregion.bytes() / 0x800000); i++)
		{
			m_bank->configure_entry(i, &m_romregion[i * 0x800000]);
		}

		m_numbanks = i;

		m_bank->set_entry(0);
	}
	else
	{
		m_numbanks = 0;
	}

	save_item(NAME(m_current_bank));
}

void gcm394_game_state::machine_reset()
{
	m_current_bank = 0;
}

void gcm394_game_state::mem_map_4m(address_map &map)
{
	map(0x000000, 0x00ffff).rom().region("maincpu", 0); // non-banked area on this SoC?

	// smartfp really expects the ROM at 0 to map here, so maybe this is how the newer SoC works
	map(0x020000, 0x3fffff).bankr("cartbank");
}

void wrlshunt_game_state::wrlshunt_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom().region("maincpu", 0); // non-banked area on this SoC?
	map(0x030000, 0x3fffff).ram().share("mainram");
}

static INPUT_PORTS_START( gcm394 )
	PORT_START("P1")
	PORT_DIPNAME( 0x0001, 0x0001, "P1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) // hold button 1 and 4 on startup for test screen
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 )

	PORT_START("P2")
	PORT_DIPNAME( 0x0001, 0x0001, "P2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wrlshunt )
	PORT_START("P1")
	PORT_START("P2")
INPUT_PORTS_END

/*
Wireless Hunting Video Game System
(info provided with dump)

System: Wireless Hunting Video Game System
Publisher: Hamy / Kids Station Toys Inc
Year: 2011
ROM: FDI MSP55LV100G
RAM: Micron Technology 48LC8M16A2

Games:

Secret Mission
Predator
Delta Force
Toy Land
Dream Forest
Trophy Season
Freedom Force
Be Careful
Net Power
Open Training
Super Archer
Ultimate Frisbee
UFO Shooting
Happy Darts
Balloon Shoot
Avatair
Angry Pirate
Penguin War
Ghost Shooter
Duck Hunt


ROM Board:

Package: SO44
Spacing: 1.27 mm
Width: 16.14 mm
Length: 27.78 mm
Voltage: 3V
Pinout:

          A25  A24
            |  |
      +--------------------------+
A21 --|==   #  # `.__.'        ==|-- A20
A18 --|==                      ==|-- A19
A17 --|==                      ==|-- A8
 A7 --|==                      ==|-- A9
 A6 --|==                  o   ==|-- A10
 A5 --|==  +----------------+  ==|-- A11
 A4 --|==  |                |  ==|-- A12
 A3 --|==  |  MSP55LV100G   |  ==|-- A13
 A2 --|==  |  0834 M02H     |  ==|-- A14
 A1 --|==  |  JAPAN         |  ==|-- A15
 A0 --|==  |                |  ==|-- A16
#CE --|==  |                |  ==|-- A23
GND --|==  |                |  ==|-- A22
#OE --|==  |                |  ==|-- Q15
 Q0 --|==  |                |  ==|-- Q7
 Q8 --|==  |                |  ==|-- Q14
 Q1 --|==  +----------------+  ==|-- Q6
 Q9 --|==                      ==|-- Q13
 Q2 --|==       M55L100G       ==|-- Q5
Q10 --|==                      ==|-- Q12
 Q3 --|==                      ==|-- Q4
Q11 --|==                      ==|-- VCC
      +--------------------------+


The only interesting string in this ROM is SPF2ALP,
which is also found in the Wireless Air 60 ROM.

*/

ROM_START(wrlshunt)
	ROM_REGION(0x8000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("wireless.bin", 0x0000, 0x8000000, CRC(a6ecc20e) SHA1(3645f23ba2bb218e92d4560a8ae29dddbaabf796))
ROM_END

ROM_START(smartfp)
	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("smartfitpark.bin", 0x000000, 0x800000, CRC(ada84507) SHA1(a3a80bf71fae62ebcbf939166a51d29c24504428))
ROM_END


/*
Wireless Air 60
(info provided with dump)

System: Wireless Air 60
ROM: Toshiba TC58NVG0S3ETA00
RAM: ESMT M12L128168A

This is a raw NAND flash dump

Interesting Strings:

GPnandnand; (GP is General Plus, which is Sunplus by another name)
GLB_GP-F_5B_USBD_1.0.0
SP_ToneMaker
GLB_GP-FS1_0405L_SPU_1.0.2.3
SPF2ALP

"GPnandnand" as a required signature appears to be referenced right here, in page 19 of a GeneralPlus document;
https://web.archive.org/web/20180106005235/http://www.lcis.com.tw/paper_store/paper_store/GPL162004A-507A_162005A-707AV10_code_reference-20147131205102.pdf

*/

ROM_START( wlsair60 )
	ROM_REGION( 0x8400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "wlsair60.nand", 0x0000, 0x8400000, CRC(eec23b97) SHA1(1bb88290cf54579a5bb51c08a02d793cd4d79f7a) )
ROM_END


CONS(2011, wrlshunt, 0, 0, wrlshunt, wrlshunt, wrlshunt_game_state, empty_init, "Hamy / Kids Station Toys Inc", "Wireless Hunting Video Game System", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

CONS(2009, smartfp, 0, 0, base, gcm394, gcm394_game_state, empty_init, "Fisher-Price", "Fun 2 Learn Smart Fit Park (Spain)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
// Fun 2 Learn 3-in-1 SMART SPORTS  ?

// NAND dumps w/ internal bootstrap (and u'nSP 2.0 extended opcodes)
CONS(2010, wlsair60, 0, 0, base, gcm394, gcm394_game_state, empty_init, "Jungle Soft / Kids Station Toys Inc", "Wireless Air 60",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
