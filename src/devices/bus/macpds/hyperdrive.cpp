// license:BSD-3-Clause
// copyright-holders:R. Belmont

/***************************************************************************

HyperDrive ST-506 HDD Controller for Macintosh 128k, 512k and Plus.
General Computer Corporation (GCC) 1985

Driver by R. Belmont
Board Notes by Guru

PCB Layout
----------

HyperDrive 20 P11-A Rev 1.1
|----------------------------------|
|74LS123   AM26LS32     DP8465     |
|                              |---|
|                              |
|P3     10MHz  74LS126  74LS74 |---|
|                                  |
|                                  |
|      ROM_HIGH.U15  ROM_LOW.U16   |
|                                  |
|      |-|                         |
|      | |        PAL20L10         |
|      | |                         |
|      | |                         |
|      | |     74LS32    74LS245   |
|      | |                         |
|    CN| |                         |
|      | |     74HC4040  6116      |
|      | |                         |
|      | |                         |
|      | |        WD2010A-PL       |
|      | |                         |
|      |-|                         |
|                                  |
|   P2         74LS14    74LS273   |
|                                  |
|   P1         7406      7438      |
|                                  |
| AM26LS31 74LS151 L13-120 74LS174 |
|                                  |
|----------------------------------|
Notes: (All IC's shown)

P1/P2      - 34-pin (P1) and 20-pin (P2) ST-506 control & data cable connectors
             The connected HDD is a ST-506 MMI Model MM112 10MB HDD manufactured by 'Microcomputer Memories Inc.'
             The Official HDD parameters are: 306 Cylinders, 4 Heads, 17 Sectors Per Track and 512 Bytes Per Sector for a
             total formatted capacity of 10,653,696 bytes. Note the parameters and/or capacity may not be the same
             when used with the HyperDrive Controller.
             The HDD was backed up on a 80386-based PC with a plug-in ST-506 controller card using the DOS program available
             here.... http://www.partition-recovery.com/diskimage.htm
             When backing up the drive, the Sectors Per Track was adjusted to 16 to get a good read.
             The image was transferred to a modern PC using a parallel ZIP drive because IDE and ST-506 controllers cannot exist
             together in the same PC due to using the same IRQ and BIOS drive 80h.
P3         - 4-pin power connector for HDD
CN         - 64-pin connector joined with a flat cable to Macintosh motherboard 68000 CPU. The cable has a special socket
             on the end that clips over the top of the 68000 CPU. This is required for end-user installations because the 68000
             CPU is soldered into the Macintosh motherboard.
DP8465     - 8kx8-bit SRAM
6116       - 2kx8-bit SRAM
ROM*       - 2732 4kx8-bit EPROM (HyperDrive Boot ROMs)
WD2010A-PL - Western Digital WD2010A-PL05-02 Winchester Disk Controller
74HC4040   - 74HC4040 12-Stage Binary Ripple Counter with Clock Input, Asynchronous
             Master Reset Input and Twelve Parallel Outputs
AM26LS31   - AMD AM26LS31 Quad EIA-422 Differential Line Driver with Tri-State Outputs
AM26LS32   - AMD AM26LS32 Quad EIA-422 Differential Line Receiver with Tri-State Outputs
L13-120    - possibly a delay line?
PAL20L10   - PAL20L10 Programmable Array Logic marked 'PALCFO'
7406       - 7406 Hex Inverter Buffer/Driver with Open-Collector High-Voltage Outputs
74LS14     - 74LS14 Hex Inverter with Schmitt Trigger Inputs
74LS32     - 74LS32 Quad 2-input OR Gate
74LS38     - 74LS38 Quad 2-input NAND Buffer
74LS74     - 74LS74 Dual JK Flip-Flop
74LS123    - 74LS123 Dual Retriggerable Monostable Multivibrator
74LS126    - 74LS126 Quad Tri-State Buffer
74LS151    - 74LS151 High-Speed 8-Input Digital Multiplexer
74LS174    - 74LS174 Hex D-Type Flip-Flop With Clear
74LS245    - 74LS245 Octal Bus Transceiver With 3-State Outputs
74LS273    - 74LS273 Octal D-Type Flip-Flop With Clear

***************************************************************************/

#include "emu.h"
#include "hyperdrive.h"

#define HYPERDRIVE_ROM_REGION  "hyperd_rom"

ROM_START( hyperdrive )
	ROM_REGION(0x2000, HYPERDRIVE_ROM_REGION, ROMREGION_16BIT|ROMREGION_BE)
	ROM_LOAD16_BYTE( "hyperdrive 52949-h v7.06.u15", 0x000000, 0x001000, CRC(b0ec3dd9) SHA1(bf8cea0ba69f5ddbe0ba50fc709c3ba81728cecf) )
	ROM_LOAD16_BYTE( "hyperdrive 52949-l v7.06.u16", 0x000001, 0x001000, CRC(0ff90c48) SHA1(4ebe1ead72064c470b7262c14e7e6887643816bf) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(PDS_HYPERDRIVE, macpds_hyperdrive_device, "pds_hyper", "GCC HyperDrive")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void macpds_hyperdrive_device::device_add_mconfig(machine_config &config)
{
		WD2010(config, m_hdc, 5000000);
 //       m_hdc->out_bcr_callback().set(FUNC(macpds_hyperdrive_device::clct_w));
		m_hdc->in_bcs_callback().set(FUNC(macpds_hyperdrive_device::hdd_r));
		m_hdc->out_bcs_callback().set(FUNC(macpds_hyperdrive_device::hdd_w));
		m_hdc->in_drdy_callback().set_constant(1);
		m_hdc->in_index_callback().set_constant(1);
		m_hdc->in_wf_callback().set_constant(1);
		m_hdc->in_tk000_callback().set_constant(1);
		m_hdc->in_sc_callback().set_constant(1);

		HARDDISK(config, "hard0", 0);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *macpds_hyperdrive_device::device_rom_region() const
{
	return ROM_NAME( hyperdrive );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  macpds_hyperdrive_device - constructor
//-------------------------------------------------

macpds_hyperdrive_device::macpds_hyperdrive_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	macpds_hyperdrive_device(mconfig, PDS_HYPERDRIVE, tag, owner, clock)
{
}

macpds_hyperdrive_device::macpds_hyperdrive_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_macpds_card_interface(mconfig, *this),
	m_hdc(*this, "wd2010")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void macpds_hyperdrive_device::device_start()
{
	set_macpds_device();

	install_rom(this, HYPERDRIVE_ROM_REGION, 0xf80000);

	// WD200x registers
	m_macpds->install_device(0xfc0000, 0xfc000f, read16_delegate(FUNC(macpds_hyperdrive_device::hyperdrive_r), this), write16_delegate(FUNC(macpds_hyperdrive_device::hyperdrive_w), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void macpds_hyperdrive_device::device_reset()
{
}

WRITE16_MEMBER( macpds_hyperdrive_device::hyperdrive_w )
{
	m_hdc->write(offset, data & 0xff);
}

READ16_MEMBER( macpds_hyperdrive_device::hyperdrive_r )
{
	return m_hdc->read(offset);
}

WRITE8_MEMBER( macpds_hyperdrive_device::hdd_w )
{
//  printf("hdd_w: %02x @ %x\n", data, offset);
}

READ8_MEMBER( macpds_hyperdrive_device::hdd_r )
{
//  printf("hdd_r: @ %x\n", offset);
	return 0;
}
