// license:BSD-3-Clause
// copyright-holders:David Haywood, R.Belmont

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "screen.h"
#include "speaker.h"
#include "machine/bankdev.h"

class radicasi_state : public driver_device
{
public:
	radicasi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, "ram"),
		m_bank(*this, "bank")
	{ }

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER(radicasi_500c_w);
	DECLARE_WRITE8_MEMBER(radicasi_500d_w);

	DECLARE_READ8_MEMBER(radicasi_500d_r);
	DECLARE_READ8_MEMBER(radicasi_5041_r);
	DECLARE_READ8_MEMBER(radicasi_50a8_r);

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_ram;
	required_device<address_map_bank_device> m_bank;

	uint8_t m_500d_data;

};

void radicasi_state::video_start()
{
}

uint32_t radicasi_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}

WRITE8_MEMBER(radicasi_state::radicasi_500c_w)
{
	// written with the banking?
	logerror("%s: radicasi_500c_w %02x\n", machine().describe_context().c_str(), data);
}

READ8_MEMBER(radicasi_state::radicasi_500d_r)
{
	return m_500d_data;
}

WRITE8_MEMBER(radicasi_state::radicasi_500d_w)
{
	logerror("%s: radicasi_500d_w %02x\n", machine().describe_context().c_str(), data);
	m_500d_data = data;
	m_bank->set_bank(m_500d_data);
}

READ8_MEMBER(radicasi_state::radicasi_50a8_r)
{
	logerror("%s: radicasi_50a8_r\n", machine().describe_context().c_str());
	return 0x3f;
}

READ8_MEMBER(radicasi_state::radicasi_5041_r)
{
	logerror("%s: radicasi_5041_r\n", machine().describe_context().c_str());
	return machine().rand();
	//return 0x00; // inputs? (causes further banking writes if random is returned at least)
}

static ADDRESS_MAP_START( radicasi_map, AS_PROGRAM, 8, radicasi_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAM AM_SHARE("ram") // ends up copying code to ram, but could be due to banking issues
	AM_RANGE(0x4800, 0x49ff) AM_RAM

	AM_RANGE(0x500c, 0x500c) AM_WRITE(radicasi_500c_w)
	AM_RANGE(0x500d, 0x500d) AM_READWRITE(radicasi_500d_r, radicasi_500d_w)

	AM_RANGE(0x5041, 0x5041) AM_READ(radicasi_5041_r)

	AM_RANGE(0x50a8, 0x50a8) AM_READ(radicasi_50a8_r)

	AM_RANGE(0x6000, 0xdfff) AM_DEVICE("bank", address_map_bank_device, amap8)

	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("maincpu", 0xf8000)
ADDRESS_MAP_END


static ADDRESS_MAP_START( radicasi_bank_map, AS_PROGRAM, 8, radicasi_state )
	AM_RANGE(0x300000, 0x3fffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x400000, 0x40ffff) AM_RAM // could be framebuffer access (256x256 at 4bpp)
ADDRESS_MAP_END

static INPUT_PORTS_START( radicasi )
INPUT_PORTS_END

void radicasi_state::machine_start()
{
	uint8_t *rom = memregion("maincpu")->base();
	/* both NMI and IRQ vectors just point to RTI
	   there is a table of jumps just before that, those appear to be the real interrupt functions?

	   patch the main IRQ to be the one that decreases an address the code is waiting for
	   the others look like they might be timer service routines
	*/
	rom[0xf9ffe] = 0xd4;
	rom[0xf9fff] = 0xff;

	/*
		d8000-dffff maps to 6000-dfff
		e0000-e7fff maps to 6000-dfff
		e8000-effff maps to 6000-dfff
		f0000-f7fff maps to 6000-dfff
		f8000-fffff maps to 6000-dfff (but f8000-f9fff mapping to 6000-7fff isn't used, because it's the fixed area below - make sure nothing else gets mapped there instead)

		-- fixed
		f8000-f9fff maps to e000-ffff
	*/

	m_bank->set_bank(0x7f);
}

void radicasi_state::machine_reset()
{
}

static MACHINE_CONFIG_START( radicasi )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6502,8000000) // unknown frequency
	MCFG_CPU_PROGRAM_MAP(radicasi_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", radicasi_state,  irq0_line_hold)

	MCFG_DEVICE_ADD("bank", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(radicasi_bank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATA_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDR_WIDTH(32)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x8000)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(radicasi_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 256)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END



ROM_START( radicasi )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "spaceinvadersrom.bin", 0x000000, 0x100000, CRC(5ffb2c8f) SHA1(9bde42ec5c65d9584a802de7d7c8b842ebf8cbd8) )
ROM_END

CONS( 200?, radicasi,  0,   0,  radicasi,  radicasi, radicasi_state, 0, "Radica (licensed from Taito)", "Space Invaders (Radica, Arcade Legends TV Game)", MACHINE_IS_SKELETON )
