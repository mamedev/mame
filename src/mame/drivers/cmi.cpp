// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

		Fairlight CMI IIx skeleton
		-----------------------------------

		All currently known information comes from:

		     CMI SYSTEM SERVICE MANUAL
		FAIRLIGHT INSTRUMENTS, FEBRUARY 1985
		          Revision 2.1

		This document is available on archive.org at the following URL:

		https://archive.org/details/fairlight_CMI-IIx_SERVICE_MANUAL

		Summary:

		The Fairlight CMI system conists typically of:
			- One velocity-sensitive unweighted keyboard, with a numeric
			  keypad and several control surfaces
			- (Optionally) one additional keyboard, not velocity-sensitive
			- One alphanumeric keyboard for manual control
			- A 15-inch green-screen monitor and light pen for more direct
			  control
			- A box consisting of:
			  * An audio board including balanced line drivers for eight
			    channels and mixed output
			  * A 500-watt power supply
			  * A 21-slot backplane
			  * Two 8-inch double-density floppy disk drives. The format
			    used is soft-sectored, 128 bytes per sector (single density),
			    or 256 bytes per sector (double density), using FM
			    recording.
			  * And the following cards:
			    Slot  1: Master Card CMI-02
			    Slot  2: General Interface Card CMI-08/28 (Optional)
			    Slots 3-11: 8 Channel Controller Cards & 1 Voice Master Module Card, order unknown
			    Slot 12: 64K System RAM Q-096
			    Slot 13: 256K System RAM Q-256
			    Slot 14: 256K System RAM Q-256
			    Slot 15: 4-Port ACIA Module Q-014 (Optional)
				Slot 16: Processor Control Module Q-133
				Slot 17: Central Processor Module Q-209
				Slot 18: Lightpen/Graphics Interface Q-219
				Slot 19: Floppy Disk Controller QFC-9
				Slot 20: Hard Disk Controller Q-077 (Optional)

		The Master Keyboard
		-------------------

		The master keyboard has the following features:
			- A serial connector for communicating with the CMI mainframe
			- A connector for a slave keyboard
			- A connector for the alphanumeric keyboard
			- Connectors for pedal controls
			- Three slider-type analog controls
			- Two switch controls (one momentary, one toggle on/off)
			- Two lamp indicators for the switches with software-defined
			  control
			- A 12-character LED alphanumeric display
			- A 16-switch keypad

		All communications with all peripherals and controls on the master
		keyboard is handled via the master keyboard's controller, and as
		such there is one single serial link to the "CMI mainframe" box
		itself.

		Q209 Dual 6809 Central Processor Card
		-------------------------------------

		The CPU card has two 6809 processors, with robust inter-CPU
		communications capabilities including:
			- Uninterruptible instructions
			- CPU-specific ID register and memory map registers
			- Interprocessor interrupts
			- Automatic memory map-switching register

		The CPUs are multiplexed onto the address and data buses
		in an interleaved manner such that there is no contention
		on simultaneous memory accesses.

		All system timing is derived from a 40MHz clock crystal, which
		is divided into two opposite-phase 20MHz squre waves.

		Other data entry from service manual to be completed later - RH 12 Aug 2016

****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"

#define Q209_CPU_CLOCK		1000000

class cmi_state : public driver_device
{
public:
	cmi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu1(*this, "maincpu1")
		, m_maincpu2(*this, "maincpu2")
		, m_percpu_rom(*this, "percpurom")
		, m_common_rom(*this, "commonrom")
		, m_percpu_rom_ptr(nullptr)
		, m_common_rom_ptr(nullptr)
	{
	}

	virtual void machine_reset() override;
	virtual void machine_start() override;

	DECLARE_READ8_MEMBER( q209_mmu_r );
	DECLARE_WRITE8_MEMBER( q209_mmu_w );

	DECLARE_DRIVER_INIT( cmi2x );

protected:

	required_device<m6809_device> m_maincpu1;
	required_device<m6809_device> m_maincpu2;

	required_memory_region m_percpu_rom;
	required_memory_region m_common_rom;

	UINT8 *m_percpu_rom_ptr;
	UINT8 *m_common_rom_ptr;
private:
};

/* Input ports */
static INPUT_PORTS_START( cmi2x )
INPUT_PORTS_END

void cmi_state::machine_reset()
{
}

void cmi_state::machine_start()
{
	m_percpu_rom_ptr = (UINT8 *)m_percpu_rom->base();
	m_common_rom_ptr = (UINT8 *)m_common_rom->base();
}

READ8_MEMBER( cmi_state::q209_mmu_r )
{
	int cpuindex = (&space.device() == m_maincpu1 ? 1 : 2);
	if (cpuindex == 1)
	{
		if (offset >= 0xf800)
		{
			if (offset >= 0xffe0)
			{
				return m_common_rom_ptr[0x3e0 + (offset - 0xffe0)];
			}
			else if (offset < 0xfc00)
			{
				return m_common_rom_ptr[0x000 + (offset - 0xf800)];
			}
		}
		else if (offset >= 0xf000)
		{
			return m_common_rom_ptr[offset - 0xf000];
		}
		else
		{
		}
	}
	else
	{
		if (offset >= 0xf800)
		{
			if (offset >= 0xffe0)
			{
				return m_common_rom_ptr[0x3e0 + (offset - 0xffe0)];
			}
			else if (offset < 0xfc00)
			{
				return m_common_rom_ptr[0x000 + (offset - 0xf800)];
			}
		}
		else if (offset >= 0xf000)
		{
			return m_common_rom_ptr[offset - 0xf000];
		}
	}

	logerror("CPU%d unknown read, offset=%04x PC=%04x\n", cpuindex, offset, space.device().safe_pc());
	return 0;
}

WRITE8_MEMBER( cmi_state::q209_mmu_w )
{
	const int cpuindex = (&space.device() == m_maincpu1 ? 1 : 2);
	logerror("CPU%d unknown write, offset=%04x data=%02x PC=%04x\n", cpuindex, offset, data, space.device().safe_pc());
}

static ADDRESS_MAP_START(q209_6809_mmu_map, AS_PROGRAM, 8, cmi_state)
	AM_RANGE(0x0000, 0xffff) AM_READWRITE( q209_mmu_r, q209_mmu_w )
ADDRESS_MAP_END

static MACHINE_CONFIG_START( cmi2x, cmi_state )
	MCFG_CPU_ADD("maincpu1", M6809, Q209_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(q209_6809_mmu_map)

	MCFG_CPU_ADD("maincpu2", M6809, Q209_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(q209_6809_mmu_map)
MACHINE_CONFIG_END

ROM_START( cmi2x )
	ROM_REGION( 0x800, "commonrom", 0 )
	ROM_LOAD( "f8lmrk5.bin",  0x000, 0x800, CRC(cfc7967f) SHA1(0695cc757cf6fab35414dc068dd2a3e50084685c) )

	ROM_REGION( 0x800, "percpurom", 0 )
	ROM_LOAD( "q9fomrk1.bin", 0x000, 0x800, CRC(16f195cc) SHA1(fcc4be370ba60ae5a4145c36cdbdc97a7be91f8f) )

	/* QFC9 Floppy disk controller driver */
	ROM_REGION( 0x800, "qfc9", 0 )
	ROM_LOAD( "dqfc911.bin", 0x00, 0x800, CRC(5bc38db2) SHA1(bd840e19e51a336e669c40b9e18cdaf6b3c62a8a) )

	/* Musical keyboard CPU */
	// Both of these dumps have been trimmed to size from within a roughly 2x-bigger file.
	// The actual size is known based on the format apparently used by the dumping device, shared with the prom
	// dumps and cmikeys4.bin dump.
	ROM_REGION( 0x10000, "muskeys", 0 )
	ROM_LOAD( "velkeysd.bin", 0xb000, 0x0400, CRC(9b636781) SHA1(be29a72a1d6d313dafe0b63951b5e3e18ddb9a21) )
	ROM_LOAD( "kbdioa.bin",   0xfc00, 0x0400, CRC(a5cbe218) SHA1(bc6784aaa5697c28eab126e20500139b8d0c1f50) )

	/* Alphanumeric keyboard CPU */
	// This dump has been trimmed to size from within a roughly 2x-bigger file. The actual size is known based
	// on the format apparently used by the dumping device, shared with the prom dumps and music keys dump.
	ROM_REGION( 0x10000, "alphakeys", 0 )
	ROM_LOAD( "cmikeys4.bin", 0xc000, 0x400, CRC(b214fbe9) SHA1(8c404f58ba3e5a50aa42f761e966c74374e96cc9) )

	/* General Interface (SMPTE/MIDI) CPU */
	ROM_REGION( 0x4000, "smptemidi", 0 )
	ROM_LOAD16_BYTE( "mon1110e.bin", 0x0000, 0x2000, CRC(476f7d5f) SHA1(9af21e0072eaa58cae42947c20dca05d35dfadd0) )
	ROM_LOAD16_BYTE( "mon1110o.bin", 0x0001, 0x2000, CRC(150c8ebe) SHA1(bbd371bebac29628f60537832d0587e83323ad01) )

	// All of these PROM dumps have been trimmed to size from within a roughly 2x-bigger file.
	// The actual sizes are known from the schematics and the starting address of the actual PROM data was obvious
	// based on repeated data in some of the 256x4 PROMs, but it would be nice to get redumps, in the extremely
	// unlikely event that someone finds a CMI IIx for sale.
	ROM_REGION( 0x420, "proms", 0 )
	ROM_LOAD( "brom.bin",   0x000, 0x100, CRC(3f730d15) SHA1(095df6eee95b9ad6418b910fb5d2ae46913750f9) ) // Unknown use, lightgun/graphics card
	ROM_LOAD( "srom.bin",   0x100, 0x100, CRC(a1b4b71b) SHA1(6ea96480af2f1e43967f209218a74fc17972ce0e) ) // Used to generate signal timing for lightpen
	ROM_LOAD( "mrom.bin",   0x200, 0x100, CRC(dc26642c) SHA1(49b207ff80d1b055c3b855dc954129846c49bfe3) ) // Unknown use, master card
	ROM_LOAD( "timrom.bin", 0x300, 0x100, CRC(a426e4a2) SHA1(6b7ea128c730f5afd1042820ccd55bbda683afd8) ) // Unknown use, master card
	ROM_LOAD( "wrom.bin",   0x400, 0x020, CRC(68a9e17f) SHA1(c3364a37a8d19a1882d7910add1c1df9b63ee32c) ) // Unknown use, lightgun/graphics card
ROM_END

DRIVER_INIT_MEMBER( cmi_state, cmi2x )
{
}

CONS( 1983, cmi2x, 0, 0, cmi2x, cmi2x, cmi_state, cmi2x, "Fairlight", "CMI IIx", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
