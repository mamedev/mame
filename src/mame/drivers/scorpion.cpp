// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic

#include "emu.h"
#include "includes/spectrum.h"
#include "includes/spec128.h"

#include "machine/beta.h"
#include "machine/timer.h"
#include "sound/ay8910.h"

#include "formats/tzx_cas.h"


namespace {

class scorpion_state : public spectrum_128_state
{
public:
	scorpion_state(const machine_config &mconfig, device_type type, const char *tag)
		: spectrum_128_state(mconfig, type, tag)
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
		, m_bank4(*this, "bank4")
		, m_beta(*this, BETA_DISK_TAG)
	{ }

	void scorpion(machine_config &config);
	void profi(machine_config &config);
	void quorum(machine_config &config);

protected:
	virtual void machine_reset() override;

private:
	uint8_t beta_neutral_r(offs_t offset);
	uint8_t beta_enable_r(offs_t offset);
	uint8_t beta_disable_r(offs_t offset);
	void scorpion_0000_w(offs_t offset, uint8_t data);
	void scorpion_port_7ffd_w(uint8_t data);
	void scorpion_port_1ffd_w(uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(nmi_check_callback);

	void scorpion_io(address_map &map);
	void scorpion_mem(address_map &map);
	void scorpion_switch(address_map &map);

	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;
	required_device<beta_disk_device> m_beta;

	uint8_t *m_ram_0000;
	address_space *m_program;
	uint8_t *m_p_ram;
	void scorpion_update_memory();

};

/****************************************************************************************************/
/* Zs Scorpion 256 */

/*
port 7ffd. full compatibility with Zx spectrum 128. digits are:

D0-D2 - number of RAM page to put in C000-FFFF
D3    - switch of address for RAM of screen. 0 - 4000, 1 - c000
D4    - switch of ROM : 0-zx128, 1-zx48
D5    - 1 in this bit will block further output in port 7FFD, until reset.
*/

/*
port 1ffd - additional port for resources of computer.

D0    - block of ROM in 0-3fff. when set to 1 - allows read/write page 0 of RAM
D1    - selects ROM expansion. this rom contains main part of service monitor.
D2    - not used
D3    - used for output in RS-232C
D4    - extended RAM. set to 1 - connects RAM page with number 8-15 in
    C000-FFFF. number of page is given in gidits D0-D2 of port 7FFD
D5    - signal of strobe for interface centronics. to form the strobe has to be
    set to 1.
D6-D7 - not used. ( yet ? )
*/

/* rom 0=zx128, 1=zx48, 2 = service monitor, 3=tr-dos */

void scorpion_state::scorpion_update_memory()
{
	uint8_t *messram = m_ram->pointer();

	m_screen_location = messram + ((m_port_7ffd_data & 8) ? (7<<14) : (5<<14));

	m_bank4->set_base(messram + (((m_port_7ffd_data & 0x07) | ((m_port_1ffd_data & 0x10)>>1)) * 0x4000));

	if ((m_port_1ffd_data & 0x01)==0x01)
	{
		m_ram_0000 = messram+(8<<14);
		m_bank1->set_base(messram+(8<<14));
		logerror("RAM\n");
	}
	else
	{
		if ((m_port_1ffd_data & 0x02)==0x02)
			m_ROMSelection = 2;
		else
			m_ROMSelection = BIT(m_port_7ffd_data, 4);

		m_bank1->set_base(&m_p_ram[0x10000 + (m_ROMSelection<<14)]);
	}
}

void scorpion_state::scorpion_0000_w(offs_t offset, uint8_t data)
{
	if ( ! m_ram_0000 )
		return;

	if ((m_port_1ffd_data & 0x01)==0x01)
	{
		if ( ! m_ram_disabled_by_beta )
			m_ram_0000[offset] = data;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(scorpion_state::nmi_check_callback)
{
	if ((m_io_nmi->read() & 1)==1)
	{
		m_port_1ffd_data |= 0x02;
		scorpion_update_memory();
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}

void scorpion_state::scorpion_port_7ffd_w(uint8_t data)
{
	/* disable paging */
	if (m_port_7ffd_data & 0x20)
		return;

	/* store new state */
	m_port_7ffd_data = data;

	/* update memory */
	scorpion_update_memory();
}

void scorpion_state::scorpion_port_1ffd_w(uint8_t data)
{
	m_port_1ffd_data = data;
	scorpion_update_memory();
}

uint8_t scorpion_state::beta_neutral_r(offs_t offset)
{
	return m_program->read_byte(offset);
}

uint8_t scorpion_state::beta_enable_r(offs_t offset)
{
	if(m_ROMSelection == 1) {
		m_ROMSelection = 3;
		if (m_beta->started()) {
			m_beta->enable();
			m_bank1->set_base(&m_p_ram[0x10000 + (m_ROMSelection<<14)]);
			m_ram_disabled_by_beta = 1;
		}
	}
	return m_program->read_byte(offset + 0x3d00);
}

uint8_t scorpion_state::beta_disable_r(offs_t offset)
{
	if (m_beta->started() && m_beta->is_active()) {
		m_ROMSelection = BIT(m_port_7ffd_data, 4);
		m_beta->disable();
		m_bank1->set_base(&m_p_ram[0x10000 + (m_ROMSelection<<14)]);
		m_ram_disabled_by_beta = 1;
	} else
		m_ram_disabled_by_beta = 0;
	return m_program->read_byte(offset + 0x4000);
}

void scorpion_state::scorpion_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankr("bank1").w(FUNC(scorpion_state::scorpion_0000_w));
	map(0x4000, 0x7fff).bankrw("bank2");
	map(0x8000, 0xbfff).bankrw("bank3");
	map(0xc000, 0xffff).bankrw("bank4");
}

void scorpion_state::scorpion_io(address_map &map)
{
	map.unmap_value_high();
	map(0x001f, 0x001f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::status_r), FUNC(beta_disk_device::command_w));
	map(0x003f, 0x003f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::track_r), FUNC(beta_disk_device::track_w));
	map(0x005f, 0x005f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::sector_r), FUNC(beta_disk_device::sector_w));
	map(0x007f, 0x007f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::data_r), FUNC(beta_disk_device::data_w));
	map(0x00fe, 0x00fe).select(0xff00).rw(FUNC(scorpion_state::spectrum_ula_r), FUNC(scorpion_state::spectrum_ula_w));
	map(0x00ff, 0x00ff).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::state_r), FUNC(beta_disk_device::param_w));
	map(0x4021, 0x4021).mirror(0x3fdc).w(FUNC(scorpion_state::scorpion_port_7ffd_w));
	map(0x8021, 0x8021).mirror(0x3fdc).w("ay8912", FUNC(ay8910_device::data_w));
	map(0xc021, 0xc021).mirror(0x3fdc).rw("ay8912", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
	map(0x0021, 0x0021).mirror(0x3fdc).w(FUNC(scorpion_state::scorpion_port_1ffd_w));
}

void scorpion_state::scorpion_switch(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(scorpion_state::beta_neutral_r)); // Overlap with previous because we want real addresses on the 3e00-3fff range
	map(0x3d00, 0x3dff).r(FUNC(scorpion_state::beta_enable_r));
	map(0x4000, 0xffff).r(FUNC(scorpion_state::beta_disable_r));
}

void scorpion_state::machine_reset()
{
	uint8_t *messram = m_ram->pointer();
	m_program = &m_maincpu->space(AS_PROGRAM);
	m_p_ram = memregion("maincpu")->base();

	m_ram_0000 = nullptr;
	m_program->install_read_bank(0x0000, 0x3fff, m_bank1);
	m_program->install_write_handler(0x0000, 0x3fff, write8sm_delegate(*this, FUNC(scorpion_state::scorpion_0000_w)));

	m_beta->disable();

	memset(messram,0,256*1024);

	/* Bank 5 is always in 0x4000 - 0x7fff */
	m_bank2->set_base(messram + (5<<14));

	/* Bank 2 is always in 0x8000 - 0xbfff */
	m_bank3->set_base(messram + (2<<14));

	m_port_7ffd_data = 0;
	m_port_1ffd_data = 0;
	scorpion_update_memory();
}

/* F4 Character Displayer */
static const gfx_layout spectrum_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	96,                 /* 96 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static const gfx_layout quorum_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	160,                    /* 160 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static const gfx_layout profi_8_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	224,                    /* 224 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_scorpion )
	GFXDECODE_ENTRY( "maincpu", 0x17d00, spectrum_charlayout, 0, 8 )
GFXDECODE_END

static GFXDECODE_START( gfx_profi )
	GFXDECODE_ENTRY( "maincpu", 0x17d00, spectrum_charlayout, 0, 8 )
	GFXDECODE_ENTRY( "maincpu", 0x1abfc, profi_8_charlayout, 0, 8 )
	/* There are more characters after this, that haven't been decoded */
GFXDECODE_END

static GFXDECODE_START( gfx_quorum )
	GFXDECODE_ENTRY( "maincpu", 0x1fb00, quorum_charlayout, 0, 8 )
GFXDECODE_END

void scorpion_state::scorpion(machine_config &config)
{
	spectrum_128(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &scorpion_state::scorpion_mem);
	m_maincpu->set_addrmap(AS_IO, &scorpion_state::scorpion_io);
	m_maincpu->set_addrmap(AS_OPCODES, &scorpion_state::scorpion_switch);

	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_scorpion);

	BETA_DISK(config, m_beta, 0);

	/* internal ram */
	m_ram->set_default_size("256K");

	TIMER(config, "nmi_timer").configure_periodic(FUNC(scorpion_state::nmi_check_callback), attotime::from_hz(50));

	config.device_remove("exp");
}

void scorpion_state::profi(machine_config &config)
{
	scorpion(config);
	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_profi);
}

void scorpion_state::quorum(machine_config &config)
{
	scorpion(config);
	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_quorum);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(scorpio)
	ROM_REGION(0x90000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v1", "V.2.92")
	ROMX_LOAD("scorp0.rom", 0x010000, 0x4000, CRC(0eb40a09) SHA1(477114ff0fe1388e0979df1423602b21248164e5), ROM_BIOS(0))
	ROMX_LOAD("scorp1.rom", 0x014000, 0x4000, CRC(9d513013) SHA1(367b5a102fb663beee8e7930b8c4acc219c1f7b3), ROM_BIOS(0))
	ROMX_LOAD("scorp2.rom", 0x018000, 0x4000, CRC(fd0d3ce1) SHA1(07783ee295274d8ff15d935bfd787c8ac1d54900), ROM_BIOS(0))
	ROMX_LOAD("scorp3.rom", 0x01c000, 0x4000, CRC(1fe1d003) SHA1(33703e97cc93b7edfcc0334b64233cf81b7930db), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v2", "V.2.92 joined")
	ROMX_LOAD( "scorpion.rom", 0x010000, 0x10000, CRC(fef73c28) SHA1(66cecdadf992d8adb9c66deee929eb56600dc9bc), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v3", "V.2.94")
	ROMX_LOAD( "scorp294.rom", 0x010000, 0x10000, CRC(99f57ce1) SHA1(083bb57ad52cc871b92d3e1794fd9790872c3584), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v4", "SMUC V.4.02")
	ROMX_LOAD( "scorp402.rom", 0x010000, 0x20000, CRC(9fcf893d) SHA1(0cc7ba60f5cfc36e75bd3a5c90e26b2a1905a970), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v5", "ProfROM V.3.9f")
	ROMX_LOAD( "prof_39f.rom", 0x010000, 0x20000, CRC(c55e64da) SHA1(cec7770fe26350f57f6c325a29db78787dc4521e), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "v6", "ProfROM V.4.01")
	ROMX_LOAD( "profrom.rom", 0x010000, 0x80000, CRC(b02d89de) SHA1(4cb85341e2a400e0e88869304d80af430266cdd1), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "v7", "NeOS 256")
	ROMX_LOAD("neos_256.rom", 0x010000, 0x4000, CRC(364ae09a) SHA1(bb6db1947415503a6bc48f33c603fb3a0dbb3690), ROM_BIOS(6))
	ROMX_LOAD("scorp1.rom",   0x014000, 0x4000, CRC(9d513013) SHA1(367b5a102fb663beee8e7930b8c4acc219c1f7b3), ROM_BIOS(6))
	ROMX_LOAD("scorp2.rom",   0x018000, 0x4000, CRC(fd0d3ce1) SHA1(07783ee295274d8ff15d935bfd787c8ac1d54900), ROM_BIOS(6))
	ROMX_LOAD("scorp3.rom",   0x01c000, 0x4000, CRC(1fe1d003) SHA1(33703e97cc93b7edfcc0334b64233cf81b7930db), ROM_BIOS(6))

	ROM_REGION(0x01000, "keyboard", 0)
	ROM_LOAD( "scrpkey.rom", 0x0000, 0x1000, CRC(e938a510) SHA1(2753993c97ff0fc6cff26ed792929abc1288dc6f))

	ROM_REGION(0x010000, "gsound", 0)
	ROM_LOAD( "gs104.rom", 0x0000, 0x8000, CRC(7a365ba6) SHA1(c865121306eb3a7d811d82fbcc653b4dc1d6fa3d))
	ROM_LOAD( "gs105a.rom", 0x8000, 0x8000, CRC(1cd490c6) SHA1(1ba78923dc7d803e7995c165992e14e4608c2153))
ROM_END

ROM_START(profi)
	ROM_REGION(0x020000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v1", "ver 1")
	ROMX_LOAD( "profi.rom", 0x010000, 0x10000, CRC(285a0985) SHA1(2b33ab3561e7bc5997da7f0d3a2a906fe7ea960f), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v2", "ver 2")
	ROMX_LOAD( "profi_my.rom", 0x010000, 0x10000, CRC(2ffd6cd9) SHA1(1b74a3251358c5f102bb87654f47b02281e15e9c), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v3", "ver 3")
	ROMX_LOAD( "profi-p1.rom", 0x010000, 0x10000, CRC(537ddb81) SHA1(00a23e8dc722b248d4f98cb14a600ce7487f2b9c), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v4", "ver 4")
	ROMX_LOAD( "profi_1.rom", 0x010000, 0x10000, CRC(f07fbee8) SHA1(b29c81a94658a4d50274ba953775a49e855534de), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v5", "T-Rex")
	ROMX_LOAD( "profi-p.rom", 0x010000, 0x10000, CRC(314f6b57) SHA1(1507f53ec64dcf5154b5cfce6922f69f70296a53), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "v6", "JV Kramis V0.2")
	ROMX_LOAD( "profi32.rom", 0x010000, 0x10000, CRC(77327f52) SHA1(019bd00cc7939741d99b99beac6ae1298652e652), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "v7", "Power Of Sound Group")
	ROMX_LOAD( "profi1k.rom", 0x010000, 0x10000, CRC(a932676f) SHA1(907ac56219f325949a7c2fe8168799d9cdd5ba6c), ROM_BIOS(6))
ROM_END

ROM_START(quorum)
	ROM_REGION(0x020000, "maincpu", 0)
	ROM_LOAD("qu7v42.rom",   0x010000, 0x10000, CRC(e950eee5) SHA1(f8e22672722b0038689c6c8bc4acf5392acc9d8c))
ROM_END

ROM_START(bestzx)
	ROM_REGION(0x020000, "maincpu", 0)
	ROM_LOAD( "bestzx.rom", 0x010000, 0x10000, CRC(fc7936e8) SHA1(0d6378c51b2f08a3e2b4c75e64c76c15ae5dc76d))
ROM_END

ROM_START( kay1024 )
	ROM_REGION(0x020000, "maincpu", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "v1", "ver 1")
	ROMX_LOAD( "kay98.rom",    0x010000, 0x08000, CRC(7fbf2d43) SHA1(e555f2ed01ecf2231d493bd70a4d79b436e9f10e), ROM_BIOS(0))
	ROMX_LOAD( "trd503.rom",   0x01c000, 0x04000, CRC(10751aba) SHA1(21695e3f2a8f796386ce66eea8a246b0ac44810c), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v2", "Kramis V0.2")
	ROMX_LOAD( "kay1024b.rom", 0x010000, 0x10000, CRC(ab99c31e) SHA1(cfa9e6553aea72956fce4f0130c007981d684734), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v3", "Kramis V0.3")
	ROMX_LOAD( "kay1024s.rom", 0x010000, 0x10000, CRC(67351caa) SHA1(1d9c0606b380c000ca1dfa33f90a122ecf9df1f1), ROM_BIOS(2))
ROM_END

} // Anonymous namespace


//    YEAR  NAME     PARENT   COMPAT  MACHINE   INPUT      CLASS           INIT        COMPANY              FULLNAME           FLAGS
COMP( 1994, scorpio, spec128, 0,      scorpion, spec_plus, scorpion_state, empty_init, "Zonov and Co.",     "Scorpion ZS-256", 0 )
COMP( 1991, profi,   spec128, 0,      profi,    spec_plus, scorpion_state, empty_init, "Kondor and Kramis", "Profi",           MACHINE_NOT_WORKING )
COMP( 1998, kay1024, spec128, 0,      scorpion, spec_plus, scorpion_state, empty_init, "NEMO",              "Kay 1024",        MACHINE_NOT_WORKING )
COMP( 19??, quorum,  spec128, 0,      quorum,   spec_plus, scorpion_state, empty_init, "<unknown>",         "Quorum",          MACHINE_NOT_WORKING )
COMP( 19??, bestzx,  spec128, 0,      scorpion, spec_plus, scorpion_state, empty_init, "<unknown>",         "BestZX",          MACHINE_NOT_WORKING )
