// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************

    SGI Indigo workstation

    To-Do:
    - IP12 (R3000):
     * Everything
    - IP20 (R4000):
     * Figure out why the keyboard/mouse diagnostic fails
     * Work out a proper RAM mapping, or why the installer bails due
       to trying to access virtual address ffffa02c:
       88002584: lw        $sp,-$5fd4($0)

**********************************************************************/

#include "emu.h"
//#include "cpu/dsp56k/dsp56k.h"
#include "cpu/mips/mips1.h"
#include "cpu/mips/mips3.h"
#include "machine/eepromser.h"
#include "machine/hpc1.h"
#include "machine/sgi.h"
#include "video/light.h"

#define LOG_UNKNOWN     (1 << 0)
#define LOG_INT         (1 << 1)
#define LOG_DSP         (1 << 2)
#define LOG_ALL         (LOG_UNKNOWN | LOG_INT | LOG_DSP)

#define VERBOSE         (LOG_UNKNOWN)
#include "logmacro.h"

class indigo_state : public driver_device
{
public:
	indigo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_hpc(*this, "hpc")
		, m_eeprom(*this, "eeprom")
		, m_light(*this, "lg1")
	{
	}

	void indigo_base(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ32_MEMBER(int_r);
	DECLARE_WRITE32_MEMBER(int_w);
	DECLARE_READ32_MEMBER(dsp_ram_r);
	DECLARE_WRITE32_MEMBER(dsp_ram_w);

	void indigo_map(address_map &map);

	required_device<hpc1_device> m_hpc;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	std::unique_ptr<uint32_t[]> m_dsp_ram;
	required_device<light_video_device> m_light;
	address_space *m_space;
};

class indigo3k_state : public indigo_state
{
public:
	indigo3k_state(const machine_config &mconfig, device_type type, const char *tag)
		: indigo_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void indigo3k(machine_config &config);

protected:
	void mem_map(address_map &map);

	required_device<r3000a_device> m_maincpu;
};

class indigo4k_state : public indigo_state
{
public:
	indigo4k_state(const machine_config &mconfig, device_type type, const char *tag)
		: indigo_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mem_ctrl(*this, "memctrl")
		, m_share1(*this, "share1")
	{
	}

	void indigo4k(machine_config &config);

protected:
	virtual void machine_reset() override;

	void mem_map(address_map &map);

	DECLARE_WRITE64_MEMBER(write_ram);

	required_device<r4000be_device> m_maincpu;
	required_device<sgi_mc_device> m_mem_ctrl;
	required_shared_ptr<uint64_t> m_share1;
};

void indigo_state::machine_start()
{
	m_dsp_ram = std::make_unique<uint32_t[]>(0x8000);
	save_pointer(NAME(&m_dsp_ram[0]), 0x8000);
}

void indigo_state::machine_reset()
{
}

void indigo4k_state::machine_reset()
{
	indigo_state::machine_reset();

	// set up low RAM mirror
	membank("bank1")->set_base(m_share1);
}

READ32_MEMBER(indigo_state::int_r)
{
	LOGMASKED(LOG_INT, "%s: INT Read: %08x & %08x\n", machine().describe_context(), 0x1fbd9000 + offset*4, mem_mask);
	return 0;
}

WRITE32_MEMBER(indigo_state::int_w)
{
	LOGMASKED(LOG_INT, "%s: INT Write: %08x = %08x & %08x\n", machine().describe_context(), 0x1fbd9000 + offset*4, data, mem_mask);
}

READ32_MEMBER(indigo_state::dsp_ram_r)
{
	LOGMASKED(LOG_DSP, "%s: DSP RAM Read: %08x = %08x & %08x\n", machine().describe_context(), 0x1fbe0000 + offset*4, m_dsp_ram[offset], mem_mask);
	return m_dsp_ram[offset];
}

WRITE32_MEMBER(indigo_state::dsp_ram_w)
{
	LOGMASKED(LOG_DSP, "%s: DSP RAM Write: %08x = %08x & %08x\n", machine().describe_context(), 0x1fbe0000 + offset*4, data, mem_mask);
	m_dsp_ram[offset] = data;
}


void indigo_state::indigo_map(address_map &map)
{
	map(0x1f3f0000, 0x1f3fffff).rw(m_light, FUNC(light_video_device::entry_r), FUNC(light_video_device::entry_w));
	map(0x1fb80000, 0x1fb8ffff).rw(m_hpc, FUNC(hpc1_device::read), FUNC(hpc1_device::write));
	map(0x1fbd9000, 0x1fbd903f).rw(FUNC(indigo_state::int_r), FUNC(indigo_state::int_w));
	map(0x1fbe0000, 0x1fbfffff).rw(FUNC(indigo_state::dsp_ram_r), FUNC(indigo_state::dsp_ram_w));
}

void indigo3k_state::mem_map(address_map &map)
{
	indigo_map(map);
	map(0x1fc00000, 0x1fc3ffff).rom().share("share10").region("user1", 0);
}

WRITE64_MEMBER(indigo4k_state::write_ram)
{
	// if banks 2 or 3 are enabled, do nothing, we don't support that much memory
	if (m_mem_ctrl->get_mem_config(1) & 0x10001000)
	{
		// a random perturbation so the memory test fails
		data ^= 0xffffffffffffffffULL;
	}

	// if banks 0 or 1 have 2 membanks, also kill it, we only want 128 MB
	if (m_mem_ctrl->get_mem_config(0) & 0x40004000)
	{
		// a random perturbation so the memory test fails
		data ^= 0xffffffffffffffffULL;
	}
	COMBINE_DATA(&m_share1[offset]);
}

void indigo4k_state::mem_map(address_map &map)
{
	indigo_map(map);
	map(0x00000000, 0x0007ffff).bankrw("bank1");
	map(0x08000000, 0x17ffffff).ram().share("share1").w(FUNC(indigo4k_state::write_ram));     /* 128 MB of main RAM */
	map(0x1fa00000, 0x1fa1ffff).rw(m_mem_ctrl, FUNC(sgi_mc_device::read), FUNC(sgi_mc_device::write));
	map(0x1fc00000, 0x1fc7ffff).rom().share("share5").region("user1", 0);
	map(0x20000000, 0x27ffffff).ram().share("share1").w(FUNC(indigo4k_state::write_ram));     /* 128 MB of main RAM */
}

static INPUT_PORTS_START(indigo)
INPUT_PORTS_END

void indigo_state::indigo_base(machine_config &config)
{
	LIGHT_VIDEO(config, m_light);

	EEPROM_93C56_16BIT(config, m_eeprom);
}

void indigo3k_state::indigo3k(machine_config &config)
{
	indigo_base(config);

	R3000A(config, m_maincpu, 33.333_MHz_XTAL, 32768, 32768);
	downcast<r3000a_device &>(*m_maincpu).set_endianness(ENDIANNESS_BIG);
	m_maincpu->set_addrmap(AS_PROGRAM, &indigo3k_state::mem_map);

	SGI_HPC1(config, m_hpc, m_maincpu, m_eeprom);
}

void indigo4k_state::indigo4k(machine_config &config)
{
	indigo_base(config);

	mips3_device &cpu(R4000BE(config, m_maincpu, 50000000*2));
	cpu.set_icache_size(32768);
	cpu.set_dcache_size(32768);
	cpu.set_addrmap(AS_PROGRAM, &indigo4k_state::mem_map);

	SGI_MC(config, m_mem_ctrl, m_maincpu, m_eeprom);
	SGI_HPC1(config, m_hpc, m_maincpu, m_eeprom);
}

ROM_START( indigo3k )
	ROM_REGION32_BE( 0x40000, "user1", 0 )
	ROM_SYSTEM_BIOS( 0, "401-rev-c", "SGI Version 4.0.1 Rev C LG1/GR2, Jul 9, 1992" ) // dumped over serial connection from boot monitor and swapped
	ROMX_LOAD( "ip12prom.070-8088-xxx.u56", 0x000000, 0x040000, CRC(25ca912f) SHA1(94b3753d659bfe50b914445cef41290122f43880), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "401-rev-d", "SGI Version 4.0.1 Rev D LG1/GR2, Mar 24, 1992" ) // dumped with EPROM programmer
	ROMX_LOAD( "ip12prom.070-8088-002.u56", 0x000000, 0x040000, CRC(ea4329ef) SHA1(b7d67d0e30ae8836892f7170dd4757732a0a3fd6), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(1) )
ROM_END

ROM_START( indigo4k )
	ROM_REGION64_BE( 0x80000, "user1", 0 )
	ROMX_LOAD( "ip20prom.070-8116-004.bin", 0x000000, 0x080000, CRC(940d960e) SHA1(596aba530b53a147985ff3f6f853471ce48c866c), ROM_GROUPDWORD | ROM_REVERSE )
ROM_END

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT   CLASS           INIT        COMPANY                 FULLNAME                                          FLAGS
COMP( 1991, indigo3k, 0,      0,      indigo3k, indigo, indigo3k_state, empty_init, "Silicon Graphics Inc", "IRIS Indigo (R3000, 33MHz)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1993, indigo4k, 0,      0,      indigo4k, indigo, indigo4k_state, empty_init, "Silicon Graphics Inc", "IRIS Indigo (R4400, 150MHz, Ver. 4.0.5D Rev A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
