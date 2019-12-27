// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
    SunPlus unSP based hardware, SPG-??? (6xx?) (die is GCM394)

    Compared to vii.cpp this is clearly newer, has extra opcodes, different internal map etc. also scaling and higher resolutions based on Spongebob

	note, these SoC types always have a 128Kwords internal ROM, which the JAKKS games appear to use for basic bootstrap purposes.

    GPL600
        Smart Fit Park
        SpongeBob SquarePants Bikini Bottom 500
        Spiderman - The Masked Menace 'Spider Sense' (pad type with Spiderman model)
        (Wireless Hunting? - maybe, register map looks the same even if it sets stack to 2fff not 6fff)

    GPL800 (== GPL600 with NAND support + maybe more)
        Wireless Air 60
        Golden Tee Golf
        Cars 2
        Toy Story Mania
        V.Baby
        Playskool Heroes Transformers Rescue Bots Beam Box

    GPL500 (unknown, might be GPL600 but without the higher resolution support?)
        The Price is Right
        Bejeweled? (might be GPL600)

    Notes
        smartfp: hold button Circle, Star and Home on startup for Test Menu

    these are both unsp 2.0 type, as they use the extended ocpodes


	NAND types:

	Toy Story Mania H27U518S2C dumped as HY27US08121A (512+16) x 32 x 4096
	Beam Box GPR27P512A dumped as HY27US08121A (512+16) x 32 x 4096
	Golden Tee GPR27P512A dumped as HY27US08121A (512+16) x 32 x 4096
	Cars 2 GPR27P512A dumped as HY27US08121A (512+16) x 32 x 4096

	V.Baby HY27UF081G2A (2048+64) x 64 x 1024

*/

#include "emu.h"

#include "machine/sunplus_gcm394.h"

#include "screen.h"
#include "speaker.h"




class full_memory_device :
	public device_t,
	public device_memory_interface
{
public:
	// construction/destruction
	full_memory_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration helpers
	template <typename... T> full_memory_device& set_map(T &&... args) { set_addrmap(0, std::forward<T>(args)...); return *this; }

	template <typename... T> full_memory_device& map(T &&... args) { set_addrmap(0, std::forward<T>(args)...); return *this; }

	address_space* get_program() { return m_program; }

protected:
	virtual void device_start() override;
	virtual void device_config_complete() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;


private:
	// internal state
	address_space_config m_program_config;
	address_space *m_program;
	int m_shift;
};


// device type definition
DECLARE_DEVICE_TYPE(FULL_MEMORY, full_memory_device)

// device type definition
DEFINE_DEVICE_TYPE(FULL_MEMORY, full_memory_device, "full_memory", "SunPlus Full CS Memory Map")

full_memory_device::full_memory_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, FULL_MEMORY, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_program(nullptr)
{
}

device_memory_interface::space_config_vector full_memory_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

/*
':maincpu' (00F87F):possible DMA operation (7abf) (trigger 0001) with params mode:4009 source:00040000 (word offset) dest:00830000 (word offset) length:00007800 (words)
':maincpu' (002384):possible DMA operation (7abf) (trigger 0001) with params mode:0009 source:00180000 (word offset) dest:00840000 (word offset) length:00160000 (words)

':maincpu' (05048D):possible DMA operation (7abf) (trigger 0001) with params mode:0089 source:00006fa3 (word offset) dest:000025bc (word offset) length:000001e0 (words)
':maincpu' (05048D):possible DMA operation (7abf) (trigger 0001) with params mode:0089 source:00006fa3 (word offset) dest:000024cc (word offset) length:000000f0 (words)
':maincpu' (05048D):possible DMA operation (7abf) (trigger 0001) with params mode:0089 source:00006fa3 (word offset) dest:00000002 (word offset) length:00000400 (words)
':maincpu' (05048D):possible DMA operation (7abf) (trigger 0001) with params mode:0089 source:00006fa3 (word offset) dest:00000402 (word offset) length:00000400 (words)
':maincpu' (05048D):possible DMA operation (7abf) (trigger 0001) with params mode:0089 source:00006fa3 (word offset) dest:00000802 (word offset) length:00000400 (words)

gtg
':maincpu' (005ACE):possible DMA operation (7abf) (trigger 0001) with params mode:1089 source:30007854 (word offset) dest:00030000 (word offset) length:00000200 (words)
':maincpu' (005ACE):possible DMA operation (7abf) (trigger 0001) with params mode:1089 source:30007854 (word offset) dest:00030100 (word offset) length:00000200 (words)
':maincpu' (005ACE):possible DMA operation (7abf) (trigger 0001) with params mode:1089 source:30007854 (word offset) dest:00030200 (word offset) length:00000200 (words)
':maincpu' (005ACE):possible DMA operation (7abf) (trigger 0001) with params mode:1089 source:30007854 (word offset) dest:00030300 (word offset) length:00000200 (words)
':maincpu' (005ACE):possible DMA operation (7abf) (trigger 0001) with params mode:1089 source:30007854 (word offset) dest:00030400 (word offset) length:00000200 (words)

*/

void full_memory_device::device_config_complete()
{
	m_program_config = address_space_config( "program", ENDIANNESS_BIG, 16, 32, -1 );
}

void full_memory_device::device_start()
{
	m_program = &space(AS_PROGRAM);
}


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
		m_romregion(*this, "maincpu"),
		m_memory(*this, "memory")
	{
	}

	void base(machine_config &config);

	void cs_map_base(address_map &map);

	virtual DECLARE_READ16_MEMBER(cs0_r);
	virtual DECLARE_WRITE16_MEMBER(cs0_w);


protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;


	required_device<sunplus_gcm394_base_device> m_maincpu;
	required_device<screen_device> m_screen;

	optional_memory_bank m_bank;

	required_ioport m_io_p1;
	required_ioport m_io_p2;

	void mem_map_4m_base(address_map &map);

	required_region_ptr<uint16_t> m_romregion;
	required_device<full_memory_device> m_memory;

	DECLARE_READ16_MEMBER(porta_r);
	DECLARE_READ16_MEMBER(portb_r);
	DECLARE_WRITE16_MEMBER(porta_w);

	virtual DECLARE_WRITE16_MEMBER(mapping_w) {}

	virtual DECLARE_READ16_MEMBER(read_external_space);
	virtual DECLARE_WRITE16_MEMBER(write_external_space);


	DECLARE_READ16_MEMBER(pre_cs_r);
	DECLARE_WRITE16_MEMBER(pre_cs_w);

private:

};

READ16_MEMBER(gcm394_game_state::pre_cs_r)
{
	return m_maincpu->space(AS_PROGRAM).read_word(offset);
}

WRITE16_MEMBER(gcm394_game_state::pre_cs_w)
{
	m_maincpu->space(AS_PROGRAM).write_word(offset, data);
}


READ16_MEMBER(gcm394_game_state::cs0_r)
{
	return m_romregion[offset & 0x3fffff];
}

WRITE16_MEMBER(gcm394_game_state::cs0_w)
{
	logerror("cs0_w %04x %04x (to ROM!)\n", offset, data);
}


void gcm394_game_state::cs_map_base(address_map &map)
{
	map(0x000000, 0x01ffff).rw(FUNC(gcm394_game_state::pre_cs_r), FUNC(gcm394_game_state::pre_cs_w));
	//map(0x020000, 0x41ffff).rw(FUNC(gcm394_game_state::cs0_r), FUNC(gcm394_game_state::cs0_w));
}

void gcm394_game_state::mem_map_4m_base(address_map &map)
{
	/*  0x000000  0x01ffff - internal area */
	map(0x020000, 0x3fffff).rw(FUNC(gcm394_game_state::cs0_r), FUNC(gcm394_game_state::cs0_w));
}


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




class generalplus_gpac800_game_state : public gcm394_game_state
{
public:
	generalplus_gpac800_game_state(const machine_config& mconfig, device_type type, const char* tag) :
		gcm394_game_state(mconfig, type, tag),
	//	m_mainram(*this, "mainram"),
		m_initial_copy_words(0x2000),
		m_nandreadbase(0)
	{
	}

	void generalplus_gpac800(machine_config &config);

	void nand_init210();
	void nand_init840();
	void nand_wlsair60();
	void nand_vbaby();
	void nand_tsm();

	void cs_map_gpac800(address_map& map);

protected:
	virtual void machine_reset() override;

	void generalplus_gpac800_map(address_map &map);
	DECLARE_READ8_MEMBER(read_nand);

private:
	void nand_init(int blocksize, int blocksize_stripped);

//	required_shared_ptr<u16> m_mainram;
	std::vector<uint16_t> m_sdram;

	std::vector<uint8_t> m_strippedrom;
	int m_strippedsize;

	int m_initial_copy_words;
	int m_nandreadbase;

	virtual DECLARE_READ16_MEMBER(read_external_space) override;
	virtual DECLARE_WRITE16_MEMBER(write_external_space) override;

	virtual DECLARE_READ16_MEMBER(cs0_r) override;
	virtual DECLARE_WRITE16_MEMBER(cs0_w) override;
};

void generalplus_gpac800_game_state::cs_map_gpac800(address_map &map)
{
	map(0x000000, 0x02ffff).rw(FUNC(generalplus_gpac800_game_state::pre_cs_r), FUNC(generalplus_gpac800_game_state::pre_cs_w));
//	map(0x030000, 0x42ffff).rw(FUNC(generalplus_gpac800_game_state::cs0_r), FUNC(generalplus_gpac800_game_state::cs0_w));
}

void generalplus_gpac800_game_state::generalplus_gpac800_map(address_map &map)
{
	/*  0x000000  0x02ffff - internal area */
	map(0x030000, 0x3fffff).rw(FUNC(generalplus_gpac800_game_state::cs0_r), FUNC(generalplus_gpac800_game_state::cs0_w));
}


READ16_MEMBER(generalplus_gpac800_game_state::cs0_r)
{
	return m_sdram[offset];
}

WRITE16_MEMBER(generalplus_gpac800_game_state::cs0_w)
{
	m_sdram[offset] = data;
}

READ8_MEMBER(generalplus_gpac800_game_state::read_nand)
{
	return m_strippedrom[(offset + m_nandreadbase) & (m_strippedsize - 1)];
}

READ16_MEMBER(generalplus_gpac800_game_state::read_external_space)
{
	//logerror("reading offset %04x\n", offset * 2);
	return m_memory->get_program()->read_word(offset);
}


WRITE16_MEMBER(generalplus_gpac800_game_state::write_external_space)
{
	m_memory->get_program()->write_word(offset, data);
}


READ16_MEMBER(gcm394_game_state::read_external_space)
{
	//logerror("reading offset %04x\n", offset * 2);
	return m_memory->get_program()->read_word(offset);
}

WRITE16_MEMBER(gcm394_game_state::write_external_space)
{
	m_memory->get_program()->write_word(offset, data);
}

WRITE16_MEMBER(wrlshunt_game_state::mapping_w)
{
	m_mapping = data;
	logerror("change mapping %04x\n", data);
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
//  logerror("DMA writing to external space (RAM?) %08x %04x\n", offset, data);

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
	m_maincpu->set_addrmap(AS_PROGRAM, &gcm394_game_state::mem_map_4m_base);
	m_maincpu->porta_in().set(FUNC(gcm394_game_state::porta_r));
	m_maincpu->portb_in().set(FUNC(gcm394_game_state::portb_r));
	m_maincpu->porta_out().set(FUNC(gcm394_game_state::porta_w));
	m_maincpu->space_read_callback().set(FUNC(gcm394_game_state::read_external_space));
	m_maincpu->space_write_callback().set(FUNC(gcm394_game_state::write_external_space));
	m_maincpu->set_irq_acknowledge_callback(m_maincpu, FUNC(sunplus_gcm394_base_device::irq_vector_cb));
	m_maincpu->mapping_write_callback().set(FUNC(gcm394_game_state::mapping_w));
	m_maincpu->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_maincpu->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
	m_maincpu->set_bootmode(1); // boot from external ROM / CS mirror
	
	FULL_MEMORY(config, m_memory).set_map(&gcm394_game_state::cs_map_base);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update("maincpu", FUNC(sunplus_gcm394_device::screen_update));
	m_screen->screen_vblank().set(m_maincpu, FUNC(sunplus_gcm394_device::vblank));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
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

	m_screen->set_size(320*2, 262*2);
	m_screen->set_visarea(0, (320*2)-1, 0, (240*2)-1);
}

void generalplus_gpac800_game_state::generalplus_gpac800(machine_config &config)
{
	GPAC800(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &generalplus_gpac800_game_state::generalplus_gpac800_map);
	m_maincpu->porta_in().set(FUNC(generalplus_gpac800_game_state::porta_r));
	m_maincpu->portb_in().set(FUNC(generalplus_gpac800_game_state::portb_r));
	m_maincpu->porta_out().set(FUNC(generalplus_gpac800_game_state::porta_w));
	m_maincpu->space_read_callback().set(FUNC(generalplus_gpac800_game_state::read_external_space));
	m_maincpu->space_write_callback().set(FUNC(generalplus_gpac800_game_state::write_external_space));
	m_maincpu->set_irq_acknowledge_callback(m_maincpu, FUNC(sunplus_gcm394_base_device::irq_vector_cb));
	m_maincpu->mapping_write_callback().set(FUNC(generalplus_gpac800_game_state::mapping_w));
	m_maincpu->add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	m_maincpu->add_route(ALL_OUTPUTS, "rspeaker", 0.5);
	m_maincpu->set_bootmode(0); // boot from internal ROM (NAND bootstrap)

	m_maincpu->nand_read_callback().set(FUNC(generalplus_gpac800_game_state::read_nand));

	FULL_MEMORY(config, m_memory).set_map(&generalplus_gpac800_game_state::cs_map_gpac800);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320*2, 262*2);
	m_screen->set_visarea(0, (320*2)-1, 0, (240*2)-1);
	m_screen->set_screen_update("maincpu", FUNC(sunplus_gcm394_device::screen_update));
	m_screen->screen_vblank().set(m_maincpu, FUNC(sunplus_gcm394_device::vblank));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}



void gcm394_game_state::machine_start()
{
	if (m_bank)
	{
		m_bank->configure_entry(0, &m_romregion[0]);
		m_bank->set_entry(0);
	}
}

void gcm394_game_state::machine_reset()
{
	m_memory->get_program()->unmap_readwrite(0x020000, 0x42ffff);
	m_memory->get_program()->install_readwrite_handler( 0x020000, 0x41ffff, read16_delegate(*this, FUNC(gcm394_game_state::cs0_r)), write16_delegate(*this, FUNC(gcm394_game_state::cs0_w)));

	m_maincpu->reset(); // reset CPU so vector gets read etc.
}


/*
	map info

	map(0x000000, 0x006fff) internal RAM
	map(0x007000, 0x007fff) internal peripherals
	map(0x008000, 0x00ffff) internal ROM (lower 32kwords) - can also be configured to mirror CS0 308000 area with external pin for boot from external ROM
	map(0x010000, 0x027fff) internal ROM (upper 96kwords) - can't be switched
	map(0x028000, 0x02ffff) reserved

	map(0x030000, 0x0.....) view into external spaces (CS0 area starts here. followed by CS1 area, CS2 area etc.)
	
	map(0x200000, 0x3fffff) continued view into external spaces, but this area is banked with m_membankswitch_7810 (valid bank values 0x00-0x3f)
*/


void wrlshunt_game_state::wrlshunt_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom().region("maincpu", 0); // non-banked area on this SoC?
	map(0x030000, 0x1fffff).ram().share("mainram");
}


static INPUT_PORTS_START( gcm394 )
	PORT_START("P1")
	// entirely non-standard mat based controller (0-11 are where your feet are placed normally, row of selection places to step above those)
	// no sensible default mapping unless forced
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_Q) PORT_NAME("0")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_W) PORT_NAME("1")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_CODE(KEYCODE_E) PORT_NAME("2")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_CODE(KEYCODE_R) PORT_NAME("3")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_CODE(KEYCODE_T) PORT_NAME("4")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_Y) PORT_NAME("5")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_U) PORT_NAME("6")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_CODE(KEYCODE_I) PORT_NAME("7")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_CODE(KEYCODE_O) PORT_NAME("8")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_CODE(KEYCODE_P) PORT_NAME("9")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("10")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON16 ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("11")

	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_A) PORT_NAME("Circle / Red")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_S) PORT_NAME("Square / Orange")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_D) PORT_NAME("Triangle / Yellow")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_F) PORT_NAME("Star / Blue")

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
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("HOME")
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

static INPUT_PORTS_START( jak_car2 )
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
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) // unused
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
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

static INPUT_PORTS_START( jak_gtg )
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
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 )
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


ROM_START(smartfp)
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "intenral.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("smartfitpark.bin", 0x000000, 0x800000, CRC(ada84507) SHA1(a3a80bf71fae62ebcbf939166a51d29c24504428))
ROM_END

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
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	//ROM_LOAD16_WORD_SWAP( "intenral.rom", 0x00000, 0x40000, NO_DUMP ) // not used, configured to external ROM boot mode

	ROM_REGION(0x8000000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("wireless.bin", 0x0000, 0x8000000, CRC(a6ecc20e) SHA1(3645f23ba2bb218e92d4560a8ae29dddbaabf796))
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
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "intenral.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x8400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "wlsair60.nand", 0x0000, 0x8400000, CRC(eec23b97) SHA1(1bb88290cf54579a5bb51c08a02d793cd4d79f7a) )
ROM_END

ROM_START( jak_gtg )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "intenral.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "goldentee.bin", 0x0000, 0x4200000, CRC(87d5e815) SHA1(5dc46cd753b791449cc41d5eff4928c0dcaf35c0) )
ROM_END

ROM_START( jak_car2 )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "intenral.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "cars2.bin", 0x0000, 0x4200000, CRC(4d610e09) SHA1(bc59f5f7f676a8f2a78dfda7fb62c804bbf850b6) )
ROM_END

ROM_START( jak_tsm )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "intenral.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "toystorymania.bin", 0x0000, 0x4200000, CRC(183b20a5) SHA1(eb4fa5ee9dfac58f5244d00d4e833b1e461cc52c) )
ROM_END

ROM_START( vbaby )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "intenral.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x8400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "vbaby.bin", 0x0000, 0x8400000, CRC(d904441b) SHA1(3742bc4e1e403f061ce2813ecfafc6f30a44d287) )
ROM_END

ROM_START( beambox )
	ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "intenral.rom", 0x00000, 0x40000, NO_DUMP ) // used as bootstrap only

	ROM_REGION( 0x4200000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "beambox.bin", 0x0000, 0x4200000, CRC(a486f04e) SHA1(73c7d99d8922eba58d94e955e254b9c3baa4443e) )
ROM_END

// the JAKKS ones of these seem to be known as 'Generalplus GPAC500' hardware?
CONS(2011, wrlshunt, 0, 0, wrlshunt, wrlshunt, wrlshunt_game_state, empty_init, "Hamy / Kids Station Toys Inc", "Wireless Hunting Video Game System", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

CONS(2009, smartfp, 0, 0, base, gcm394, gcm394_game_state, empty_init, "Fisher-Price", "Fun 2 Learn Smart Fit Park (Spain)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
// Fun 2 Learn 3-in-1 SMART SPORTS  ?


void generalplus_gpac800_game_state::machine_reset()
{
	m_memory->get_program()->unmap_readwrite(0x030000, 0x42ffff);
	m_memory->get_program()->install_readwrite_handler( 0x030000, 0x42ffff, read16_delegate(*this, FUNC(gcm394_game_state::cs0_r)), write16_delegate(*this, FUNC(gcm394_game_state::cs0_w)));

	// simulate bootstrap / internal ROM

	address_space& mem = m_maincpu->space(AS_PROGRAM);

	/* Offset(h) 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
	   00000000 (50 47 61 6E 64 6E 61 6E 64 6E)-- -- -- -- -- --  PGandnandn------
	   00000010  -- -- -- -- -- bb -- -- -- -- -- -- -- -- -- --  ----------------

	   bb = where to copy first block

	   The header is GPnandnand (byteswapped) then some params
	   one of the params appears to be for the initial code copy operation done
	   by the bootstrap
	*/

	// probably more bytes are used
	int dest = m_strippedrom[0x15] << 8;

	// copy a block of code from the NAND to RAM
	for (int i = 0; i < m_initial_copy_words; i++)
	{
		uint16_t word = m_strippedrom[(i * 2) + 0] | (m_strippedrom[(i * 2) + 1] << 8);

		mem.write_word(dest+i, word);
	}

	// these vectors must either directly point to RAM, or at least redirect there after some code
	uint16_t* internal = (uint16_t*)memregion("maincpu:internal")->base();
	internal[0x7ff5] = 0x6fea;
	internal[0x7ff6] = 0x6fec;
	internal[0x7ff7] = dest+0x20; // point boot vector at code in RAM (probably in reality points to internal code that copies the first block)
	internal[0x7ff8] = 0x6ff0;
	internal[0x7ff9] = 0x6ff2;
	internal[0x7ffa] = 0x6ff4;
	internal[0x7ffb] = 0x6ff6;
	internal[0x7ffc] = 0x6ff8;
	internal[0x7ffd] = 0x6ffa;
	internal[0x7ffe] = 0x6ffc;
	internal[0x7fff] = 0x6ffe;

	internal[0x8000] = 0xb00b;


	m_maincpu->reset(); // reset CPU so vector gets read etc.
}


void generalplus_gpac800_game_state::nand_init(int blocksize, int blocksize_stripped)
{
	m_sdram.resize(0x400000); // 0x400000 bytes, 0x800000 words


	uint8_t* rom = memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();

	int numblocks = size / blocksize;
	m_strippedsize = numblocks * blocksize_stripped;
	m_strippedrom.resize(m_strippedsize);

	for (int i = 0; i < numblocks; i++)
	{
		const int base = i * blocksize;
		const int basestripped = i * blocksize_stripped;

		for (int j = 0; j < blocksize_stripped; j++)
		{
			m_strippedrom[basestripped + j] = rom[base + j];
		}
	}

	// debug to allow for easy use of unidasm.exe
	if (0)
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"stripped_%s", machine().system().name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(&m_strippedrom[0], blocksize_stripped * numblocks, 1, fp);
			fclose(fp);
		}
	}
}

void generalplus_gpac800_game_state::nand_init210()
{
	nand_init(0x210, 0x200);
}

void generalplus_gpac800_game_state::nand_init840()
{
	nand_init(0x840, 0x800);
}

void generalplus_gpac800_game_state::nand_wlsair60()
{
	nand_init840();
	m_initial_copy_words = 0x2800;
}

void generalplus_gpac800_game_state::nand_vbaby()
{
	nand_init840();
	m_initial_copy_words = 0x1000;
}

void generalplus_gpac800_game_state::nand_tsm()
{
	nand_init210();

	// something odd must be going on with the bootloader?
	// structure has the first 0x4000 block repeated 3 times (must appear in RAM on startup?)
	// then it has a 0x10000 block repeated 4 times (must get copied to 0x30000 by code)
	// then it has the larger, main payload, just the once.

	// the addresses written to the NAND device don't compensate for these data repeats, however dump seems ok as no other data is being repeated?
	// reads after startup still need checking
	m_nandreadbase = (0x2000 + 0x2000 + 0x8000 + 0x8000 + 0x8000) * 2;
}



// NAND dumps w/ internal bootstrap (and u'nSP 2.0 extended opcodes)  (have gpnandnand strings)
// the JAKKS ones seem to be known as 'Generalplus GPAC800' hardware
CONS(2010, wlsair60, 0, 0, generalplus_gpac800, jak_car2, generalplus_gpac800_game_state, nand_wlsair60, "Jungle Soft / Kids Station Toys Inc", "Wireless Air 60",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS(200?, jak_gtg,  0, 0, generalplus_gpac800, jak_gtg,  generalplus_gpac800_game_state, nand_init210,  "JAKKS Pacific Inc", "Golden Tee Golf (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS(200?, jak_car2, 0, 0, generalplus_gpac800, jak_car2, generalplus_gpac800_game_state, nand_init210,  "JAKKS Pacific Inc", "Cars 2 (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS(200?, jak_tsm , 0, 0, generalplus_gpac800, jak_car2, generalplus_gpac800_game_state, nand_tsm,      "JAKKS Pacific Inc", "Toy Story Mania (JAKKS Pacific TV Game)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS(200?, vbaby,    0, 0, generalplus_gpac800, jak_car2, generalplus_gpac800_game_state, nand_vbaby,    "VTech", "V.Baby",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
CONS(200?, beambox,  0, 0, generalplus_gpac800, jak_car2, generalplus_gpac800_game_state, nand_init210,  "Hasbro", "Playskool Heroes Transformers Rescue Bots Beam Box (Spain)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
