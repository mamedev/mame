/***********************************************************************************

Emulation for the MSX BM-012 Midi cartridge that was sold together with Midisaurus.

TODO:
- hook up all the other signals for the CTC, SIO
- which type of SIO hookup is used? tmpz84c015af supports SIO/0, SIO/1, and SIO/2
- since the SIO signals are not hooked up, the midi in/thru/out ports are also not
  implemented yet. Channel A seems to be used for sending midi data.
- proper irq handling taking the irq priority into account is not implemented
- the hookup between 2 PIOs is educated guess work; it could be incorrect

***********************************************************************************/

#include "emu.h"
#include "bm_012.h"
#include "cpu/z80/z80.h"


const device_type MSX_CART_BM_012 = &device_creator<msx_cart_bm_012>;


msx_cart_bm_012::msx_cart_bm_012(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_BM_012, "MSX Cartridge - BM-012", tag, owner, clock, "msx_cart_bm_012", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_tmpz84c015af_pio(*this, "tmpz84_pio")
	, m_tmpz84c015af_ctc(*this, "tmpz84_ctc")
	, m_tmpz84c015af_sio(*this, "tmpz84_sio")
	, m_irq_priority(0)
	, m_bm012_pio(*this, "bm012_pio")
	, m_mdthru(*this, "mdthru")
{
}


static ADDRESS_MAP_START( bm_012_memory_map, AS_PROGRAM, 8, msx_cart_bm_012 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( bm_012_io_map, AS_IO, 8, msx_cart_bm_012 )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)

	// 10-13 - CTC channels 0-3
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("tmpz84_ctc", z80ctc_device, read, write)

	// 18-1B - SIO
	AM_RANGE(0x18, 0x1b) AM_DEVREADWRITE("tmpz84_sio", z80dart_device, ba_cd_r, ba_cd_w)

	// 1C-1F - PIO
	AM_RANGE(0x1c, 0x1f) AM_DEVREADWRITE("tmpz84_pio", z80pio_device, read_alt, write_alt)

	// F0-F1 - WDT
	// F4    - IRQ priority
	AM_RANGE(0xf4, 0xf4) AM_WRITE(tmpz84c015af_f4_w)
ADDRESS_MAP_END


static const z80_daisy_config bm_012_daisy_chain[] =
{
	{ "tmpz84_pio" },
	{ "tmpz84_sio" },
	{ "tmpz84_ctc" },
	{ NULL }
};


static MACHINE_CONFIG_FRAGMENT( msx_cart_bm_012 )
	// 12MHz XTAL @ X1
	// Toshiba TMPZ84C015AF-6 (@U5) components:
	// - Z80
	// - CTC
	// - SIO
	// - PIO
	// - CGC
	// - WDT
	MCFG_CPU_ADD("tmpz84_cpu", Z80, XTAL_12MHz/2)         /* 6 MHz */
	MCFG_CPU_PROGRAM_MAP(bm_012_memory_map)
	MCFG_CPU_IO_MAP(bm_012_io_map)
	MCFG_CPU_CONFIG(bm_012_daisy_chain)

	MCFG_DEVICE_ADD("tmpz84_pio", Z80PIO, XTAL_12MHz/2)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("tmpz84_cpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(DEVREAD8("bm012_pio", z80pio_device, pa_r))
	MCFG_Z80PIO_OUT_PA_CB(DEVWRITE8("bm012_pio", z80pio_device, pa_w))
	MCFG_Z80PIO_IN_PB_CB(DEVREAD8("bm012_pio", z80pio_device, pb_r))
	MCFG_Z80PIO_OUT_PB_CB(DEVWRITE8("bm012_pio", z80pio_device, pb_w))
	MCFG_Z80PIO_OUT_BRDY_CB(DEVWRITELINE("bm012_pio", z80pio_device, strobe_b))

	MCFG_DEVICE_ADD("tmpz84_ctc", Z80CTC, XTAL_12MHz/2)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("tmpz84_cpu", INPUT_LINE_IRQ0))

	MCFG_Z80SIO0_ADD("tmpz84_sio", XTAL_12MHz/2, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("tmpz84_cpu", INPUT_LINE_IRQ0))
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE("mdout", midi_port_device, write_txd))

	// Sony CXK5864BSP-10L  (8KB ram)
	// Sharp LH0081A Z80A-PIO-0 - For communicating between the MSX and the TMP
	MCFG_DEVICE_ADD("bm012_pio", Z80PIO, XTAL_3_579545MHz)  // ?????
	MCFG_Z80PIO_OUT_PA_CB(DEVWRITE8("tmpz84_pio", z80pio_device, pa_w))
	MCFG_Z80PIO_IN_PA_CB(DEVREAD8("tmpz84_pio", z80pio_device, pa_r))
	MCFG_Z80PIO_OUT_PB_CB(DEVWRITE8("tmpz84_pio", z80pio_device, pb_w))
	MCFG_Z80PIO_IN_PB_CB(DEVREAD8("tmpz84_pio", z80pio_device, pb_r))
	MCFG_Z80PIO_OUT_BRDY_CB(DEVWRITELINE("tmpz84_pio", z80pio_device, strobe_b))

	// MIDI ports
	MCFG_MIDI_PORT_ADD("mdin", midiin_slot, "midiin")
	MCFG_MIDI_RX_HANDLER(WRITELINE(msx_cart_bm_012, midi_in))

	MCFG_MIDI_PORT_ADD("mdthru", midiout_slot, "midiout")

	MCFG_MIDI_PORT_ADD("mdout", midiout_slot, "midiout")
MACHINE_CONFIG_END


machine_config_constructor msx_cart_bm_012::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( msx_cart_bm_012 );
}


ROM_START( msx_cart_bm_012 )
	ROM_REGION(0x8000, "tmpz84_cpu", 0)
	// The rom chip at U4 is a 27256, but it contains the same 8KB duplicated 4 times
	ROM_LOAD("midi_v1.00.u4", 0x0, 0x8000, CRC(840c9e74) SHA1(6d07637ad3a61b509221ed4650eed18442371010))
ROM_END


const rom_entry *msx_cart_bm_012::device_rom_region() const
{
	return ROM_NAME( msx_cart_bm_012 );
}


void msx_cart_bm_012::device_start()
{
	// Install IO read/write handlers
	address_space &space = machine().device<cpu_device>("maincpu")->space(AS_IO);
	space.install_write_handler(0x70, 0x73, write8_delegate(FUNC(z80pio_device::write_alt), m_bm012_pio.target()));
	space.install_read_handler(0x70, 0x73, read8_delegate(FUNC(z80pio_device::read_alt), m_bm012_pio.target()));
}


void msx_cart_bm_012::device_reset()
{
}


WRITE8_MEMBER(msx_cart_bm_012::tmpz84c015af_f4_w)
{
	m_irq_priority = data;
}


WRITE_LINE_MEMBER(msx_cart_bm_012::midi_in)
{
	m_mdthru->write_txd(state);
	m_tmpz84c015af_sio->rxb_w(state);
}


