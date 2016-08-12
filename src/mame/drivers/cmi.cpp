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
	ROM_LOAD( "f8lmrk5.bin", 0x00000, 0x0800, CRC(12345678) SHA1(1234567812345678123456781234567812345678) )

	ROM_REGION( 0x800, "percpurom", 0 )
	ROM_LOAD( "q9fomrk1.bin", 0x00000, 0x0800, CRC(12345678) SHA1(1234567812345678123456781234567812345678) )
ROM_END

DRIVER_INIT_MEMBER( cmi_state, cmi2x )
{
}

CONS( 1983, cmi2x, 0, 0, cmi2x, cmi2x, cmi_state, cmi2x, "Fairlight", "CMI IIx", MACHINE_NOT_WORKING )
