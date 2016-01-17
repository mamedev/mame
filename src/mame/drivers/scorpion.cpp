// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic

#include "emu.h"
#include "includes/spectrum.h"
#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "sound/ay8910.h"
#include "sound/speaker.h"
#include "formats/tzx_cas.h"
#include "machine/beta.h"
#include "machine/ram.h"

class scorpion_state : public spectrum_state
{
public:
	scorpion_state(const machine_config &mconfig, device_type type, std::string tag)
		: spectrum_state(mconfig, type, tag)
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
		, m_bank4(*this, "bank4")
		, m_beta(*this, BETA_DISK_TAG)
	{ }

	DECLARE_DIRECT_UPDATE_MEMBER(scorpion_direct);
	DECLARE_WRITE8_MEMBER(scorpion_0000_w);
	DECLARE_WRITE8_MEMBER(scorpion_port_7ffd_w);
	DECLARE_WRITE8_MEMBER(scorpion_port_1ffd_w);
	DECLARE_MACHINE_START(scorpion);
	DECLARE_MACHINE_RESET(scorpion);
	TIMER_DEVICE_CALLBACK_MEMBER(nmi_check_callback);
protected:
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;
	required_device<beta_disk_device> m_beta;
private:
	UINT8 *m_p_ram;
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
	UINT8 *messram = m_ram->pointer();

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

WRITE8_MEMBER(scorpion_state::scorpion_0000_w)
{
	if ( ! m_ram_0000 )
		return;

	if ((m_port_1ffd_data & 0x01)==0x01)
	{
		if ( ! m_ram_disabled_by_beta )
			m_ram_0000[offset] = data;
	}
}


DIRECT_UPDATE_MEMBER(scorpion_state::scorpion_direct)
{
	UINT16 pc = m_maincpu->device_t::safe_pcbase(); // works, but...

	m_ram_disabled_by_beta = 0;
	if (m_beta->is_active() && (pc >= 0x4000))
	{
		m_ROMSelection = BIT(m_port_7ffd_data, 4);
		m_beta->disable();
		m_ram_disabled_by_beta = 1;
		m_bank1->set_base(&m_p_ram[0x10000 + (m_ROMSelection<<14)]);
	}
	else
	if (((pc & 0xff00) == 0x3d00) && (m_ROMSelection==1))
	{
		m_ROMSelection = 3;
		m_beta->enable();
	}

	if(address<=0x3fff)
	{
		m_ram_disabled_by_beta = 1;
		direct.explicit_configure(0x0000, 0x3fff, 0x3fff, &m_p_ram[0x10000 + (m_ROMSelection<<14)]);
		m_bank1->set_base(&m_p_ram[0x10000 + (m_ROMSelection<<14)]);
		return ~0;
	}

	return address;
}

TIMER_DEVICE_CALLBACK_MEMBER(scorpion_state::nmi_check_callback)
{
	if ((m_io_nmi->read() & 1)==1)
	{
		m_port_1ffd_data |= 0x02;
		scorpion_update_memory();
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
}

WRITE8_MEMBER(scorpion_state::scorpion_port_7ffd_w)
{
	/* disable paging */
	if (m_port_7ffd_data & 0x20)
		return;

	/* store new state */
	m_port_7ffd_data = data;

	/* update memory */
	scorpion_update_memory();
}

WRITE8_MEMBER(scorpion_state::scorpion_port_1ffd_w)
{
	m_port_1ffd_data = data;
	scorpion_update_memory();
}

static ADDRESS_MAP_START (scorpion_io, AS_IO, 8, scorpion_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x001f, 0x001f) AM_DEVREADWRITE(BETA_DISK_TAG, beta_disk_device, status_r, command_w) AM_MIRROR(0xff00)
	AM_RANGE(0x003f, 0x003f) AM_DEVREADWRITE(BETA_DISK_TAG, beta_disk_device, track_r, track_w) AM_MIRROR(0xff00)
	AM_RANGE(0x005f, 0x005f) AM_DEVREADWRITE(BETA_DISK_TAG, beta_disk_device, sector_r, sector_w) AM_MIRROR(0xff00)
	AM_RANGE(0x007f, 0x007f) AM_DEVREADWRITE(BETA_DISK_TAG, beta_disk_device, data_r, data_w) AM_MIRROR(0xff00)
	AM_RANGE(0x00fe, 0x00fe) AM_READWRITE(spectrum_port_fe_r,spectrum_port_fe_w) AM_MIRROR(0xff00) AM_MASK(0xffff)
	AM_RANGE(0x00ff, 0x00ff) AM_DEVREADWRITE(BETA_DISK_TAG, beta_disk_device, state_r, param_w) AM_MIRROR(0xff00)
	AM_RANGE(0x4021, 0x4021) AM_WRITE(scorpion_port_7ffd_w)  AM_MIRROR(0x3fdc)
	AM_RANGE(0x8021, 0x8021) AM_DEVWRITE("ay8912", ay8910_device, data_w) AM_MIRROR(0x3fdc)
	AM_RANGE(0xc021, 0xc021) AM_DEVREADWRITE("ay8912", ay8910_device, data_r, address_w) AM_MIRROR(0x3fdc)
	AM_RANGE(0x0021, 0x0021) AM_WRITE(scorpion_port_1ffd_w) AM_MIRROR(0x3fdc)
ADDRESS_MAP_END


MACHINE_RESET_MEMBER(scorpion_state,scorpion)
{
	UINT8 *messram = m_ram->pointer();
	address_space &space = m_maincpu->space(AS_PROGRAM);
	m_p_ram = memregion("maincpu")->base();

	m_ram_0000 = nullptr;
	space.install_read_bank(0x0000, 0x3fff, "bank1");
	space.install_write_handler(0x0000, 0x3fff, write8_delegate(FUNC(scorpion_state::scorpion_0000_w),this));

	m_beta->disable();
	space.set_direct_update_handler(direct_update_delegate(FUNC(scorpion_state::scorpion_direct), this));

	memset(messram,0,256*1024);

	/* Bank 5 is always in 0x4000 - 0x7fff */
	m_bank2->set_base(messram + (5<<14));

	/* Bank 2 is always in 0x8000 - 0xbfff */
	m_bank3->set_base(messram + (2<<14));

	m_port_7ffd_data = 0;
	m_port_1ffd_data = 0;
	scorpion_update_memory();
}
MACHINE_START_MEMBER(scorpion_state,scorpion)
{
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

static GFXDECODE_START( scorpion )
	GFXDECODE_ENTRY( "maincpu", 0x17d00, spectrum_charlayout, 0, 8 )
GFXDECODE_END

static GFXDECODE_START( profi )
	GFXDECODE_ENTRY( "maincpu", 0x17d00, spectrum_charlayout, 0, 8 )
	GFXDECODE_ENTRY( "maincpu", 0x1abfc, profi_8_charlayout, 0, 8 )
	/* There are more characters after this, that haven't been decoded */
GFXDECODE_END

static GFXDECODE_START( quorum )
	GFXDECODE_ENTRY( "maincpu", 0x1fb00, quorum_charlayout, 0, 8 )
GFXDECODE_END

static MACHINE_CONFIG_DERIVED_CLASS( scorpion, spectrum_128, scorpion_state )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(scorpion_io)

	MCFG_MACHINE_START_OVERRIDE(scorpion_state, scorpion )
	MCFG_MACHINE_RESET_OVERRIDE(scorpion_state, scorpion )
	MCFG_GFXDECODE_MODIFY("gfxdecode", scorpion)

	MCFG_BETA_DISK_ADD(BETA_DISK_TAG)

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("256K")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("nmi_timer", scorpion_state, nmi_check_callback, attotime::from_hz(50))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( profi, scorpion )
	MCFG_GFXDECODE_MODIFY("gfxdecode", profi)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( quorum, scorpion )
	MCFG_GFXDECODE_MODIFY("gfxdecode", quorum)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(scorpio)
	ROM_REGION(0x90000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v1", "V.2.92")
	ROMX_LOAD("scorp0.rom", 0x010000, 0x4000, CRC(0eb40a09) SHA1(477114ff0fe1388e0979df1423602b21248164e5), ROM_BIOS(1))
	ROMX_LOAD("scorp1.rom", 0x014000, 0x4000, CRC(9d513013) SHA1(367b5a102fb663beee8e7930b8c4acc219c1f7b3), ROM_BIOS(1))
	ROMX_LOAD("scorp2.rom", 0x018000, 0x4000, CRC(fd0d3ce1) SHA1(07783ee295274d8ff15d935bfd787c8ac1d54900), ROM_BIOS(1))
	ROMX_LOAD("scorp3.rom", 0x01c000, 0x4000, CRC(1fe1d003) SHA1(33703e97cc93b7edfcc0334b64233cf81b7930db), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v2", "V.2.92 joined")
	ROMX_LOAD( "scorpion.rom", 0x010000, 0x10000, CRC(fef73c28) SHA1(66cecdadf992d8adb9c66deee929eb56600dc9bc), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v3", "V.2.94")
	ROMX_LOAD( "scorp294.rom", 0x010000, 0x10000, CRC(99f57ce1) SHA1(083bb57ad52cc871b92d3e1794fd9790872c3584), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "v4", "SMUC V.4.02")
	ROMX_LOAD( "scorp402.rom", 0x010000, 0x20000, CRC(9fcf893d) SHA1(0cc7ba60f5cfc36e75bd3a5c90e26b2a1905a970), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "v5", "ProfROM V.3.9f")
	ROMX_LOAD( "prof_39f.rom", 0x010000, 0x20000, CRC(c55e64da) SHA1(cec7770fe26350f57f6c325a29db78787dc4521e), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(5, "v6", "ProfROM V.4.01")
	ROMX_LOAD( "profrom.rom", 0x010000, 0x80000, CRC(b02d89de) SHA1(4cb85341e2a400e0e88869304d80af430266cdd1), ROM_BIOS(6))
	ROM_SYSTEM_BIOS(6, "v7", "NeOS 256")
	ROMX_LOAD("neos_256.rom", 0x010000, 0x4000, CRC(364ae09a) SHA1(bb6db1947415503a6bc48f33c603fb3a0dbb3690), ROM_BIOS(7))
	ROMX_LOAD("scorp1.rom",   0x014000, 0x4000, CRC(9d513013) SHA1(367b5a102fb663beee8e7930b8c4acc219c1f7b3), ROM_BIOS(7))
	ROMX_LOAD("scorp2.rom",   0x018000, 0x4000, CRC(fd0d3ce1) SHA1(07783ee295274d8ff15d935bfd787c8ac1d54900), ROM_BIOS(7))
	ROMX_LOAD("scorp3.rom",   0x01c000, 0x4000, CRC(1fe1d003) SHA1(33703e97cc93b7edfcc0334b64233cf81b7930db), ROM_BIOS(7))

	ROM_REGION(0x01000, "keyboard", 0)
	ROM_LOAD( "scrpkey.rom", 0x0000, 0x1000, CRC(e938a510) SHA1(2753993c97ff0fc6cff26ed792929abc1288dc6f))

	ROM_REGION(0x010000, "gsound", 0)
	ROM_LOAD( "gs104.rom", 0x0000, 0x8000, CRC(7a365ba6) SHA1(c865121306eb3a7d811d82fbcc653b4dc1d6fa3d))
	ROM_LOAD( "gs105a.rom", 0x8000, 0x8000, CRC(1cd490c6) SHA1(1ba78923dc7d803e7995c165992e14e4608c2153))
ROM_END

ROM_START(profi)
	ROM_REGION(0x020000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v1", "ver 1")
	ROMX_LOAD( "profi.rom", 0x010000, 0x10000, CRC(285a0985) SHA1(2b33ab3561e7bc5997da7f0d3a2a906fe7ea960f), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v2", "ver 2")
	ROMX_LOAD( "profi_my.rom", 0x010000, 0x10000, CRC(2ffd6cd9) SHA1(1b74a3251358c5f102bb87654f47b02281e15e9c), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v3", "ver 3")
	ROMX_LOAD( "profi-p1.rom", 0x010000, 0x10000, CRC(537ddb81) SHA1(00a23e8dc722b248d4f98cb14a600ce7487f2b9c), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "v4", "ver 4")
	ROMX_LOAD( "profi_1.rom", 0x010000, 0x10000, CRC(f07fbee8) SHA1(b29c81a94658a4d50274ba953775a49e855534de), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "v5", "T-Rex")
	ROMX_LOAD( "profi-p.rom", 0x010000, 0x10000, CRC(314f6b57) SHA1(1507f53ec64dcf5154b5cfce6922f69f70296a53), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(5, "v6", "JV Kramis V0.2")
	ROMX_LOAD( "profi32.rom", 0x010000, 0x10000, CRC(77327f52) SHA1(019bd00cc7939741d99b99beac6ae1298652e652), ROM_BIOS(6))
	ROM_SYSTEM_BIOS(6, "v7", "Power Of Sound Group")
	ROMX_LOAD( "profi1k.rom", 0x010000, 0x10000, CRC(a932676f) SHA1(907ac56219f325949a7c2fe8168799d9cdd5ba6c), ROM_BIOS(7))
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
	ROMX_LOAD( "kay98.rom",    0x010000, 0x08000, CRC(7fbf2d43) SHA1(e555f2ed01ecf2231d493bd70a4d79b436e9f10e), ROM_BIOS(1))
	ROMX_LOAD( "trd503.rom",   0x01c000, 0x04000, CRC(10751aba) SHA1(21695e3f2a8f796386ce66eea8a246b0ac44810c), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v2", "Kramis V0.2")
	ROMX_LOAD( "kay1024b.rom", 0x010000, 0x10000, CRC(ab99c31e) SHA1(cfa9e6553aea72956fce4f0130c007981d684734), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v3", "Kramis V0.3")
	ROMX_LOAD( "kay1024s.rom", 0x010000, 0x10000, CRC(67351caa) SHA1(1d9c0606b380c000ca1dfa33f90a122ecf9df1f1), ROM_BIOS(3))
ROM_END

/*    YEAR  NAME      PARENT    COMPAT  MACHINE     INPUT   CLASS         INIT      COMPANY     FULLNAME */
COMP( 1994, scorpio,  spec128,   0, scorpion,   spec_plus, driver_device,   0,      "Zonov and Co.",        "Scorpion ZS-256", 0 )
COMP( 1991, profi,    spec128,   0, profi,      spec_plus, driver_device,   0,      "Kondor and Kramis",        "Profi", MACHINE_NOT_WORKING )
COMP( 1998, kay1024,  spec128,   0, scorpion,   spec_plus, driver_device,   0,      "NEMO",     "Kay 1024", MACHINE_NOT_WORKING )
COMP( 19??, quorum,   spec128,   0, quorum,     spec_plus, driver_device,   0,      "<unknown>",        "Quorum", MACHINE_NOT_WORKING )
COMP( 19??, bestzx,   spec128,   0, scorpion,   spec_plus, driver_device,   0,      "<unknown>",        "BestZX", MACHINE_NOT_WORKING )
