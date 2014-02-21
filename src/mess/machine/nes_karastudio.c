/***********************************************************************************************************


 NES/Famicom cartridge emulation for Bandai Karaoke Studio

 Copyright MESS Team.
 Visit http://mamedev.org for licensing and usage restrictions.


 Here we emulate the following PCBs Bandai Karaoke Studio [mapper 188]

 
 TODO:
 - emulate the actual expansion slot for the Senyou Cassettes

 ***********************************************************************************************************/


#include "emu.h"
#include "machine/nes_karastudio.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)



//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_KARAOKESTUDIO = &device_creator<nes_karaokestudio_device>;


nes_karaokestudio_device::nes_karaokestudio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_KARAOKESTUDIO, "NES Cart Bandai Karaoke Studio PCB", tag, owner, clock, "nes_karaoke", __FILE__),
					m_mic_ipt(*this, "MIC")
{
}


void nes_karaokestudio_device::device_start()
{
	common_start();
}

void nes_karaokestudio_device::pcb_reset()
{
	m_chr_source = m_vrom_chunks ? CHRROM : CHRRAM;
	prg16_89ab(0);
	prg16_cdef((m_prg_chunks - 1) ^ 0x08);
	chr8(0, m_chr_source);
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Bandai Karaoke Studio board emulation

 Games: Karaoke Studio + expansion carts with 
 additional songs

 Note: we currently do not emulate properly the
 expansion slot

 iNES: mapper 188

 -------------------------------------------------*/

READ8_MEMBER(nes_karaokestudio_device::read_m)
{
	LOG_MMC(("karaoke studio read_m, offset: %04x\n", offset));
	return m_mic_ipt->read();
}

WRITE8_MEMBER(nes_karaokestudio_device::write_h)
{
	LOG_MMC(("karaoke studio write_h, offset: %04x, data: %02x\n", offset, data));
	// bit3 1 = M ROM (main unit), 0=E ROM (expansion)
	// HACK(?): currently it is not clear how the unit acknowledges the presence of the expansion
	// cart (when expansion is present, code keeps switching both from the expansion rom and from
	// the main ROM), so we load the expansion after the main PRG and handle banking as follows
	data ^= 8;

	prg16_89ab(data);
}



static INPUT_PORTS_START( karaoke_mic )
	PORT_START("MIC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("A (Mic Select)") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("B (Mic Start)") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Microphone (?)") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor nes_karaokestudio_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( karaoke_mic );
}
