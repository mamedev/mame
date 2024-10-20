// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor 55 21046-11/-21/-41 5.25"/8" Controller Card emulation

*********************************************************************/

/*

Luxor Conkort

PCB Layout
----------

55 11046-03

|-----------------------------------|
|   LD1 SW1             CON2        |
|                                   |
|                       CON3    S8  |
|                               S9  |
|   LS240   LS174   7406    7406    |
|                                   |
|   LS174   FDC9229 LS107   LS266   |
|                                   |
|                                   |
|       SAB1793                     |
|                   LS368   16MHz   |
|                   S6              |
|       Z80DMA          ROM         |
|                                   |
|                                   |
|       Z80             TC5565      |
|                                   |
|                                   |
|   LS138   LS174   SW2     74374   |
|                                   |
|   LS10    LS266   S5      LS374   |
|                   S3              |
|   PAL*            S1      LS240   |
|                                   |
|   LS125*  LS244   SW3     DM8131  |
|                                   |
|                                   |
|--|-----------------------------|--|
   |------------CON1-------------|

Notes:
    All IC's shown. (* only stocked when used with the ABC 1600 computer)

    ROM     - Toshiba TMM27128D-20 16Kx8 EPROM "CNTR 1.07 6490318-07"
    PAL     - PAL16R4
    TC5565  - Toshiba TC5565PL-15 8Kx8 bit Static RAM
    Z80     - Zilog Z8400APS Z80A CPU
    Z80DMA  - Zilog Z8410APS Z80A DMA
    SAB1793 - Siemens SAB1793-02P Floppy Disc Controller
    FDC9229 - SMC FDC9229BT Floppy Disc Interface Circuit
    DM8131  - National Semiconductor DM8131N 6-Bit Unified Bus Comparator
    CON1    - ABC bus connector
    CON2    - 25-pin D sub floppy connector (AMP4284)
    CON3    - 34-pin header floppy connector
    SW1     - Disk drive type (SS/DS, SD/DD)
    SW2     - Disk drive model
    SW3     - ABC bus address
    S1      - Interface type (A:ABC1600 B:ABCBUS)
    S3      - Interface type (A:ABC1600 B:ABCBUS)
    S5      - Interface type (A:ABC1600 B:ABCBUS)
    S6      - Amount of RAM installed (A:2KB, B:8KB)
    S7      - Number of drives connected (0:3, 1:2) *located on solder side
    S8      - Disk drive type (0:8", 1:5.25")
    S9      - Location of RDY signal (A:8" P2-6, B:5.25" P2-34)
    LD1     - LED

*/

#include "emu.h"
#include "lux21046.h"

#include "formats/abc800_dsk.h"
#include "formats/abc800i_dsk.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80_TAG         "5ab"
#define Z80DMA_TAG      "6ab"
#define SAB1793_TAG     "7ab"
#define FDC9229_TAG     "8b"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(LUXOR_55_21046, luxor_55_21046_device, "lux21046",   "Luxor 55 21046")
DEFINE_DEVICE_TYPE(ABC830,         abc830_device,         "abc830",     "ABC 830")
DEFINE_DEVICE_TYPE(ABC832,         abc832_device,         "abc832",     "ABC 832")
DEFINE_DEVICE_TYPE(ABC834,         abc834_device,         "abc834",     "ABC 834")
DEFINE_DEVICE_TYPE(ABC838,         abc838_device,         "abc838",     "ABC 838")
DEFINE_DEVICE_TYPE(ABC850_FLOPPY,  abc850_floppy_device,  "abc850flop", "ABC 850 Floppy")


//-------------------------------------------------
//  ROM( luxor_55_21046 )
//-------------------------------------------------

ROM_START( luxor_55_21046 )
	ROM_REGION( 0x4000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "v207" )
	ROM_SYSTEM_BIOS( 0, "v105", "Luxor v1.05 (1984-10-04)" )
	ROMX_LOAD( "cntr 105.6cd", 0x2000, 0x2000, CRC(44043025) SHA1(17487ca35b399bb49d4015bbeede0809db8e772f), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v106", "Luxor v1.06 (1984-12-13)" )
	ROMX_LOAD( "6490318-06.6cd", 0x2000, 0x2000, CRC(7f4726c9) SHA1(cf576456f135877537c5ee56b00a7259160d2c56), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v107", "Luxor v1.07 (1985-07-03)" )
	ROMX_LOAD( "cntr 1.07 6490318-07.6cd", 0x0000, 0x4000, CRC(db8c1c0e) SHA1(8bccd5bc72124984de529ee058df779f06d2c1d5), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v108", "Luxor v1.08 (1986-03-12)" )
	ROMX_LOAD( "cntr 108.6cd", 0x2000, 0x2000, CRC(229764cb) SHA1(a2e2f6f49c31b827efc62f894de9a770b65d109d), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "v207", "DiAB v2.07 (1987-06-24)" )
	ROMX_LOAD( "diab 207.6cd", 0x2000, 0x2000, CRC(86622f52) SHA1(61ad271de53152c1640c0b364fce46d1b0b4c7e2), ROM_BIOS(4) )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "pal16r4.2a", 0x000, 0x104, NO_DUMP)
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *luxor_55_21046_device::device_rom_region() const
{
	return ROM_NAME( luxor_55_21046 );
}


//-------------------------------------------------
//  ADDRESS_MAP( luxor_55_21046_mem )
//-------------------------------------------------

void luxor_55_21046_device::luxor_55_21046_mem(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x3fff);
	map(0x0000, 0x1fff).rom().region(Z80_TAG, 0x2000); // A13 pull-up
	map(0x2000, 0x3fff).ram();
}


//-------------------------------------------------
//  ADDRESS_MAP( luxor_55_21046_io )
//-------------------------------------------------

void luxor_55_21046_device::luxor_55_21046_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0c, 0x0c).mirror(0xff03).r(FUNC(luxor_55_21046_device::out_r));
	map(0x1c, 0x1c).mirror(0xff03).w(FUNC(luxor_55_21046_device::inp_w));
	map(0x2c, 0x2c).mirror(0xff03).w(FUNC(luxor_55_21046_device::_4b_w));
	map(0x3c, 0x3c).mirror(0xff03).w(FUNC(luxor_55_21046_device::_9b_w));
	map(0x4c, 0x4c).mirror(0xff03).w(FUNC(luxor_55_21046_device::_8a_w));
	map(0x58, 0x58).mirror(0x0007).select(0xff00).r(FUNC(luxor_55_21046_device::_9a_r));
	map(0x68, 0x6b).mirror(0xff00).r(m_fdc, FUNC(fd1793_device::read));
	map(0x78, 0x7b).mirror(0xff00).w(m_fdc, FUNC(fd1793_device::write));
	map(0x80, 0x80).mirror(0xff77).rw(m_dma, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
}


//-------------------------------------------------
//  Z80DMA
//-------------------------------------------------

/*

    DMA Transfer Programs

    READ DAM
    --------
    7D 45 21 05 00 C3 14 28 95 6B 02 8A CF 01 AF CF 87

    7D  transfer mode, port A -> port B, port A starting address follows, block length follows
    45  port A starting address low byte = 45
    21  port A starting address high byte = 21
    05  block length low byte = 05
    00  block length high byte = 00
    C3  reset
    14  port A is memory, port A address increments
    28  port B is I/O, port B address fixed
    95  byte mode, port B starting address low byte follows, interrupt control byte follows
    6B  port B starting address low byte = 6B (FDC DATA read)
    02  interrupt at end of block
    8A  ready active high
    CF  load
    01  transfer B->A
    AF  disable interrupts
    CF  load
    87  enable DMA

    WRITE TO DISK
    -------------
    C3 14 28 95 7B 02 8A CF 05 AF CF 87

    C3  reset
    14  port A is memory, port A address increments
    28  port B is I/O, port B address fixed
    95  byte mode, port B starting address low follows, interrupt control byte follows
    7B  port B starting address 0x007B (FDC DATA write)
    02  interrupt at end of block
    8A  ready active high
    CF  load
    05  transfer A->B
    AF  disable interrupts
    CF  load
    87  enable DMA

    ??
    --
    C3 91 40 8A AB

    C3  reset
    91  byte mode, interrupt control byte follows
    40  interrupt on RDY
    8A  _CE only, ready active high, stop on end of block
    AB  enable interrupts

*/

void luxor_55_21046_device::dma_int_w(int state)
{
	m_dma_irq = state;

	// FDC and DMA interrupts are wire-ORed to the Z80
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_fdc_irq || m_dma_irq);
}

uint8_t luxor_55_21046_device::memory_read_byte(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void luxor_55_21046_device::memory_write_byte(offs_t offset, uint8_t data)
{
	return m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}

uint8_t luxor_55_21046_device::io_read_byte(offs_t offset)
{
	return m_maincpu->space(AS_IO).read_byte(offset);
}

void luxor_55_21046_device::io_write_byte(offs_t offset, uint8_t data)
{
	return m_maincpu->space(AS_IO).write_byte(offset, data);
}

void luxor_55_21046_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ABC800I_FORMAT);
	fr.add(FLOPPY_ABC800_FORMAT);
}

static void abc_floppies(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
	device.option_add("525sd", FLOPPY_525_SD);
	device.option_add("525ssdd", FLOPPY_525_SSDD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("525qd", FLOPPY_525_QD);
	device.option_add("8dsdd", FLOPPY_8_DSDD);
}

void luxor_55_21046_device::fdc_intrq_w(int state)
{
	m_fdc_irq = state;

	// FDC and DMA interrupts are wire-ORed to the Z80
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_fdc_irq || m_dma_irq);
}


//-------------------------------------------------
//  z80_daisy_config z80_daisy_chain
//-------------------------------------------------

static const z80_daisy_config z80_daisy_chain[] =
{
	{ Z80DMA_TAG },
	{ nullptr }
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void luxor_55_21046_device::device_add_mconfig(machine_config & config)
{
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_daisy_config(z80_daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &luxor_55_21046_device::luxor_55_21046_mem);
	m_maincpu->set_addrmap(AS_IO, &luxor_55_21046_device::luxor_55_21046_io);
	m_maincpu->busack_cb().set(m_dma, FUNC(z80dma_device::bai_w));

	Z80DMA(config, m_dma, 16_MHz_XTAL / 4);
	m_dma->out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSRQ);
	m_dma->out_int_callback().set(FUNC(luxor_55_21046_device::dma_int_w));
	m_dma->in_mreq_callback().set(FUNC(luxor_55_21046_device::memory_read_byte));
	m_dma->out_mreq_callback().set(FUNC(luxor_55_21046_device::memory_write_byte));
	m_dma->in_iorq_callback().set(FUNC(luxor_55_21046_device::io_read_byte));
	m_dma->out_iorq_callback().set(FUNC(luxor_55_21046_device::io_write_byte));

	FD1793(config, m_fdc, 16_MHz_XTAL / 16);
	m_fdc->intrq_wr_callback().set(FUNC(luxor_55_21046_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(m_dma, FUNC(z80dma_device::rdy_w));
}

void abc830_device::device_add_mconfig(machine_config &config)
{
	luxor_55_21046_device::device_add_mconfig(config);
	for (auto &floppy : m_floppy)
		FLOPPY_CONNECTOR(config, floppy, abc_floppies, "525ssdd", luxor_55_21046_device::floppy_formats).enable_sound(true);
}

void abc832_device::device_add_mconfig(machine_config &config)
{
	luxor_55_21046_device::device_add_mconfig(config);
	for (auto &floppy : m_floppy)
		FLOPPY_CONNECTOR(config, floppy, abc_floppies, "525qd", luxor_55_21046_device::floppy_formats).enable_sound(true);
}

void abc834_device::device_add_mconfig(machine_config &config)
{
	luxor_55_21046_device::device_add_mconfig(config);
	for (auto &floppy : m_floppy)
		FLOPPY_CONNECTOR(config, floppy, abc_floppies, "525qd", luxor_55_21046_device::floppy_formats).enable_sound(true);
}

void abc838_device::device_add_mconfig(machine_config &config)
{
	luxor_55_21046_device::device_add_mconfig(config);
	for (auto &floppy : m_floppy)
		FLOPPY_CONNECTOR(config, floppy, abc_floppies, "8dsdd", luxor_55_21046_device::floppy_formats).enable_sound(true);
}

void abc850_floppy_device::device_add_mconfig(machine_config &config)
{
	luxor_55_21046_device::device_add_mconfig(config);
	FLOPPY_CONNECTOR(config, m_floppy[0], abc_floppies, "525qd", luxor_55_21046_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], abc_floppies, nullptr, luxor_55_21046_device::floppy_formats).enable_sound(true);
}


//-------------------------------------------------
//  INPUT_PORTS( luxor_55_21046 )
//-------------------------------------------------

INPUT_PORTS_START( luxor_55_21046 )
	PORT_START("SW1")
	// ABC 838
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:1,2,3,4") PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2e)
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) ) PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2e)
	// ABC 830
	PORT_DIPNAME( 0x01, 0x01, "Drive 0 Sides" ) PORT_DIPLOCATION("SW1:1") PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2d)
	PORT_DIPSETTING(    0x01, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, "Double" )
	PORT_DIPNAME( 0x02, 0x02, "Drive 1 Sides" ) PORT_DIPLOCATION("SW1:2") PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2d)
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, "Double" )
	PORT_DIPNAME( 0x04, 0x00, "Drive 0 Density" ) PORT_DIPLOCATION("SW1:3") PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2d)
	PORT_DIPSETTING(    0x04, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, "Double" )
	PORT_DIPNAME( 0x08, 0x00, "Drive 1 Density" ) PORT_DIPLOCATION("SW1:4") PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2d)
	PORT_DIPSETTING(    0x08, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, "Double" )
	// ABC 832/834/850
	PORT_DIPNAME( 0x01, 0x00, "Drive 0 Sides" ) PORT_DIPLOCATION("SW1:1") PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2c)
	PORT_DIPSETTING(    0x01, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, "Double" )
	PORT_DIPNAME( 0x02, 0x00, "Drive 1 Sides" ) PORT_DIPLOCATION("SW1:2") PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2c)
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, "Double" )
	PORT_DIPNAME( 0x04, 0x04, "Drive 0 Tracks" ) PORT_DIPLOCATION("SW1:3") PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2c)
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPSETTING(    0x04, "80" )
	PORT_DIPNAME( 0x08, 0x08, "Drive 1 Tracks" ) PORT_DIPLOCATION("SW1:4") PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2c)
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPSETTING(    0x08, "80" )

	PORT_START("SW2")
	PORT_DIPNAME( 0x0f, 0x01, "Drive Type" ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x01, "TEAC FD55F (ABC 834)" ) PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2c) // 230 7802-01
	PORT_DIPSETTING(    0x02, "BASF 6138 (ABC 850)" ) PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2c) // 230 8440-15
	PORT_DIPSETTING(    0x03, "Micropolis 1015F (ABC 832)" ) PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2c) // 190 9711-15
	PORT_DIPSETTING(    0x04, "BASF 6118 (ABC 832)" ) PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2c) // 190 9711-16
	PORT_DIPSETTING(    0x05, "Micropolis 1115F (ABC 832)" ) PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2c) // 190 9711-17
	PORT_DIPSETTING(    0x08, "BASF 6106/08 (ABC 830)" ) PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2d) // 190 9206-16
	PORT_DIPSETTING(    0x09, "MPI 51 (ABC 830)" ) PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2d) // 190 9206-16
	PORT_DIPSETTING(    0x0e, "BASF 6105 (ABC 838)" ) PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2e)
	PORT_DIPSETTING(    0x0f, "BASF 6106 (ABC 838)" ) PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2e) // 230 8838-15

	PORT_START("SW3")
	PORT_DIPNAME( 0x7f, 0x2c, "Card Address" ) PORT_DIPLOCATION("SW3:1,2,3,4,5,6,7")
	PORT_DIPSETTING(    0x2c, "44 (ABC 832/834/850)" ) // MF0: MF1:
	PORT_DIPSETTING(    0x2d, "45 (ABC 830)" ) // MO0: MO1:
	PORT_DIPSETTING(    0x2e, "46 (ABC 838)" ) // SF0: SF1:

	PORT_START("S1") // also S3,S5
	PORT_DIPNAME( 0x01, 0x01, "Interface Type" )
	PORT_DIPSETTING(    0x00, "ABC 1600" )
	PORT_DIPSETTING(    0x01, "ABC 80/800/802/806" )

	PORT_START("S6")
	PORT_DIPNAME( 0x01, 0x01, "RAM Size" )
	PORT_DIPSETTING(    0x00, "2 KB" )
	PORT_DIPSETTING(    0x01, "8 KB" )

	PORT_START("S8")
	PORT_DIPNAME( 0x01, 0x01, "Drive Type" )
	PORT_DIPSETTING(    0x00, "8\"" )
	PORT_DIPSETTING(    0x01, "5.25\"" )

	PORT_START("S9")
	PORT_DIPNAME( 0x01, 0x01, "RDY Pin" )
	PORT_DIPSETTING(    0x00, "P2-6 (8\")" )
	PORT_DIPSETTING(    0x01, "P2-34 (5.25\")" )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( abc830 )
//-------------------------------------------------

INPUT_PORTS_START( abc830 )
	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, "Drive 0 Sides" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, "Double" )
	PORT_DIPNAME( 0x02, 0x02, "Drive 1 Sides" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, "Double" )
	PORT_DIPNAME( 0x04, 0x00, "Drive 0 Density" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, "Double" )
	PORT_DIPNAME( 0x08, 0x00, "Drive 1 Density" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, "Double" )

	PORT_START("SW2")
	PORT_DIPNAME( 0x0f, 0x08, "Drive Type" ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x08, "BASF 6106/08" )
	PORT_DIPSETTING(    0x09, "MPI 51" )

	PORT_START("SW3")
	PORT_DIPNAME( 0x7f, 0x2d, "Card Address" ) PORT_DIPLOCATION("SW3:1,2,3,4,5,6,7")
	PORT_DIPSETTING(    0x2d, "45" )

	PORT_START("S1") // also S3,S5
	PORT_DIPNAME( 0x01, 0x01, "Interface Type" )
	PORT_DIPSETTING(    0x00, "ABC 1600" )
	PORT_DIPSETTING(    0x01, "ABC 80/800/802/806" )

	PORT_START("S6")
	PORT_DIPNAME( 0x01, 0x01, "RAM Size" )
	PORT_DIPSETTING(    0x00, "2 KB" )
	PORT_DIPSETTING(    0x01, "8 KB" )

	PORT_START("S8")
	PORT_DIPNAME( 0x01, 0x01, "Drive Type" )
	PORT_DIPSETTING(    0x00, "8\"" )
	PORT_DIPSETTING(    0x01, "5.25\"" )

	PORT_START("S9")
	PORT_DIPNAME( 0x01, 0x01, "RDY Pin" )
	PORT_DIPSETTING(    0x00, "P2-6 (8\")" )
	PORT_DIPSETTING(    0x01, "P2-34 (5.25\")" )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( abc832 )
//-------------------------------------------------

INPUT_PORTS_START( abc832 )
	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x00, "Drive 0 Sides" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, "Double" )
	PORT_DIPNAME( 0x02, 0x00, "Drive 1 Sides" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, "Double" )
	PORT_DIPNAME( 0x04, 0x04, "Drive 0 Tracks" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPSETTING(    0x04, "80" )
	PORT_DIPNAME( 0x08, 0x08, "Drive 1 Tracks" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPSETTING(    0x08, "80" )

	PORT_START("SW2")
	PORT_DIPNAME( 0x0f, 0x04, "Drive Type" ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x03, "Micropolis 1015F" )
	PORT_DIPSETTING(    0x04, "BASF 6118" )
	PORT_DIPSETTING(    0x05, "Micropolis 1115F" )

	PORT_START("SW3")
	PORT_DIPNAME( 0x7f, 0x2c, "Card Address" ) PORT_DIPLOCATION("SW3:1,2,3,4,5,6,7")
	PORT_DIPSETTING(    0x2c, "44" )

	PORT_START("S1") // also S3,S5
	PORT_DIPNAME( 0x01, 0x01, "Interface Type" )
	PORT_DIPSETTING(    0x00, "ABC 1600" )
	PORT_DIPSETTING(    0x01, "ABC 80/800/802/806" )

	PORT_START("S6")
	PORT_DIPNAME( 0x01, 0x01, "RAM Size" )
	PORT_DIPSETTING(    0x00, "2 KB" )
	PORT_DIPSETTING(    0x01, "8 KB" )

	PORT_START("S8")
	PORT_DIPNAME( 0x01, 0x01, "Drive Type" )
	PORT_DIPSETTING(    0x00, "8\"" )
	PORT_DIPSETTING(    0x01, "5.25\"" )

	PORT_START("S9")
	PORT_DIPNAME( 0x01, 0x01, "RDY Pin" )
	PORT_DIPSETTING(    0x00, "P2-6 (8\")" )
	PORT_DIPSETTING(    0x01, "P2-34 (5.25\")" )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( abc834 )
//-------------------------------------------------

INPUT_PORTS_START( abc834 )
	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x00, "Drive 0 Sides" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, "Double" )
	PORT_DIPNAME( 0x02, 0x00, "Drive 1 Sides" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, "Double" )
	PORT_DIPNAME( 0x04, 0x04, "Drive 0 Tracks" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPSETTING(    0x04, "80" )
	PORT_DIPNAME( 0x08, 0x08, "Drive 1 Tracks" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPSETTING(    0x08, "80" )

	PORT_START("SW2")
	PORT_DIPNAME( 0x0f, 0x01, "Drive Type" ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x01, "TEAC FD55F" )

	PORT_START("SW3")
	PORT_DIPNAME( 0x7f, 0x2c, "Card Address" ) PORT_DIPLOCATION("SW3:1,2,3,4,5,6,7")
	PORT_DIPSETTING(    0x2c, "44" )

	PORT_START("S1") // also S3,S5
	PORT_DIPNAME( 0x01, 0x01, "Interface Type" )
	PORT_DIPSETTING(    0x00, "ABC 1600" )
	PORT_DIPSETTING(    0x01, "ABC 80/800/802/806" )

	PORT_START("S6")
	PORT_DIPNAME( 0x01, 0x01, "RAM Size" )
	PORT_DIPSETTING(    0x00, "2 KB" )
	PORT_DIPSETTING(    0x01, "8 KB" )

	PORT_START("S8")
	PORT_DIPNAME( 0x01, 0x01, "Drive Type" )
	PORT_DIPSETTING(    0x00, "8\"" )
	PORT_DIPSETTING(    0x01, "5.25\"" )

	PORT_START("S9")
	PORT_DIPNAME( 0x01, 0x01, "RDY Pin" )
	PORT_DIPSETTING(    0x00, "P2-6 (8\")" )
	PORT_DIPSETTING(    0x01, "P2-34 (5.25\")" )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( abc838 )
//-------------------------------------------------

INPUT_PORTS_START( abc838 )
	PORT_START("SW1")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )

	PORT_START("SW2")
	PORT_DIPNAME( 0x0f, 0x0e, "Drive Type" ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x0e, "BASF 6105" )
	PORT_DIPSETTING(    0x0f, "BASF 6106" )

	PORT_START("SW3")
	PORT_DIPNAME( 0x7f, 0x2e, "Card Address" ) PORT_DIPLOCATION("SW3:1,2,3,4,5,6,7")
	PORT_DIPSETTING(    0x2e, "46" )

	PORT_START("S1") // also S3,S5
	PORT_DIPNAME( 0x01, 0x01, "Interface Type" )
	PORT_DIPSETTING(    0x00, "ABC 1600" )
	PORT_DIPSETTING(    0x01, "ABC 80/800/802/806" )

	PORT_START("S6")
	PORT_DIPNAME( 0x01, 0x01, "RAM Size" )
	PORT_DIPSETTING(    0x00, "2 KB" )
	PORT_DIPSETTING(    0x01, "8 KB" )

	PORT_START("S8")
	PORT_DIPNAME( 0x01, 0x00, "Drive Type" )
	PORT_DIPSETTING(    0x00, "8\"" )
	PORT_DIPSETTING(    0x01, "5.25\"" )

	PORT_START("S9")
	PORT_DIPNAME( 0x01, 0x00, "RDY Pin" )
	PORT_DIPSETTING(    0x00, "P2-6 (8\")" )
	PORT_DIPSETTING(    0x01, "P2-34 (5.25\")" )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( abc850 )
//-------------------------------------------------

INPUT_PORTS_START( abc850 )
	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, "Drive 0 Sides" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x01, "Double" )
	PORT_DIPNAME( 0x02, 0x02, "Drive 1 Sides" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x02, "Double" )
	PORT_DIPNAME( 0x04, 0x04, "Drive 0 Tracks" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPSETTING(    0x04, "80" )
	PORT_DIPNAME( 0x08, 0x08, "Drive 1 Tracks" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPSETTING(    0x08, "80" )

	PORT_START("SW2")
	PORT_DIPNAME( 0x0f, 0x02, "Drive Type" ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x01, "TEAC FD55F" )
	PORT_DIPSETTING(    0x02, "BASF 6138" )

	PORT_START("SW3")
	PORT_DIPNAME( 0x7f, 0x2c, "Card Address" ) PORT_DIPLOCATION("SW3:1,2,3,4,5,6,7")
	PORT_DIPSETTING(    0x2c, "44" )

	PORT_START("S1") // also S3,S5
	PORT_DIPNAME( 0x01, 0x01, "Interface Type" )
	PORT_DIPSETTING(    0x00, "ABC 1600" )
	PORT_DIPSETTING(    0x01, "ABC 80/800/802/806" )

	PORT_START("S6")
	PORT_DIPNAME( 0x01, 0x01, "RAM Size" )
	PORT_DIPSETTING(    0x00, "2 KB" )
	PORT_DIPSETTING(    0x01, "8 KB" )

	PORT_START("S8")
	PORT_DIPNAME( 0x01, 0x01, "Drive Type" )
	PORT_DIPSETTING(    0x00, "8\"" )
	PORT_DIPSETTING(    0x01, "5.25\"" )

	PORT_START("S9")
	PORT_DIPNAME( 0x01, 0x01, "RDY Pin" )
	PORT_DIPSETTING(    0x00, "P2-6 (8\")" )
	PORT_DIPSETTING(    0x01, "P2-34 (5.25\")" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor luxor_55_21046_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( luxor_55_21046 );
}

ioport_constructor abc830_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( abc830 );
}

ioport_constructor abc832_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( abc832 );
}

ioport_constructor abc834_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( abc834 );
}

ioport_constructor abc838_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( abc838 );
}

ioport_constructor abc850_floppy_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( abc850 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  luxor_55_21046_device - constructor
//-------------------------------------------------

luxor_55_21046_device::luxor_55_21046_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	luxor_55_21046_device(mconfig, LUXOR_55_21046, tag, owner, clock)
{
}

luxor_55_21046_device::luxor_55_21046_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_abcbus_card_interface(mconfig, *this),
	m_floppy(*this, SAB1793_TAG":%u", 0U),
	m_maincpu(*this, Z80_TAG),
	m_dma(*this, Z80DMA_TAG),
	m_fdc(*this, SAB1793_TAG),
	m_selected_floppy(nullptr),
	m_sw1(*this, "SW1"),
	m_sw2(*this, "SW2"),
	m_sw3(*this, "SW3"),
	m_cs(false),
	m_status(0),
	m_out(0),
	m_inp(0),
	m_fdc_irq(0),
	m_dma_irq(0),
	m_busy(0),
	m_force_busy(0)
{
}

abc830_device::abc830_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	luxor_55_21046_device(mconfig, ABC830, tag, owner, clock)
{
}

abc832_device::abc832_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	luxor_55_21046_device(mconfig, ABC832, tag, owner, clock)
{
}

abc834_device::abc834_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	luxor_55_21046_device(mconfig, ABC834, tag, owner, clock)
{
}

abc838_device::abc838_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	luxor_55_21046_device(mconfig, ABC838, tag, owner, clock)
{
}

abc850_floppy_device::abc850_floppy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	luxor_55_21046_device(mconfig, ABC850_FLOPPY, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void luxor_55_21046_device::device_start()
{
	// state saving
	save_item(NAME(m_cs));
	save_item(NAME(m_status));
	save_item(NAME(m_out));
	save_item(NAME(m_inp));
	save_item(NAME(m_fdc_irq));
	save_item(NAME(m_dma_irq));
	save_item(NAME(m_busy));
	save_item(NAME(m_force_busy));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void luxor_55_21046_device::device_reset()
{
	m_cs = false;
	m_out = 0;

	m_maincpu->reset();

	_4b_w(0);
	_9b_w(0);
	_8a_w(0);
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_cs -
//-------------------------------------------------

void luxor_55_21046_device::abcbus_cs(uint8_t data)
{
	m_cs = (data == m_sw3->read());
}


//-------------------------------------------------
//  abcbus_csb -
//-------------------------------------------------

int luxor_55_21046_device::abcbus_csb()
{
	return m_cs ? 0 : 1;
}


//-------------------------------------------------
//  abcbus_stat -
//-------------------------------------------------

uint8_t luxor_55_21046_device::abcbus_stat()
{
	/*

	    bit     description

	    0       3A pin 8
	    1       4B Q1
	    2       4B Q2
	    3       4B Q3
	    4       1
	    5       PAL16R4 pin 17
	    6       S1/A: PREN*, S1/B: 4B Q6
	    7       S5/A: PAL16R4 pin 16 inverted, S5/B: 4B Q7

	*/

	uint8_t data = 0;

	if (m_cs)
	{
		data = 0x30 | (m_status & 0xce) | m_busy;
	}

	// LS240 inverts the data
	return data ^ 0xff;
}


//-------------------------------------------------
//  abcbus_inp -
//-------------------------------------------------

uint8_t luxor_55_21046_device::abcbus_inp()
{
	uint8_t data = 0xff;

	if (m_cs)
	{
		data = m_inp;
		m_busy = 1;
	}

	return data;
}


//-------------------------------------------------
//  abcbus_out -
//-------------------------------------------------

void luxor_55_21046_device::abcbus_out(uint8_t data)
{
	if (m_cs)
	{
		m_out = data;
		m_busy = 1;
	}
}


//-------------------------------------------------
//  abcbus_c1 -
//-------------------------------------------------

void luxor_55_21046_device::abcbus_c1(uint8_t data)
{
	if (m_cs)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
}


//-------------------------------------------------
//  abcbus_c3 -
//-------------------------------------------------

void luxor_55_21046_device::abcbus_c3(uint8_t data)
{
	if (m_cs)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  abcbus_c4 -
//-------------------------------------------------

void luxor_55_21046_device::abcbus_c4(uint8_t data)
{
	// TODO connected to PAL16R4 pin 2
}



//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  3d_r -
//-------------------------------------------------

uint8_t luxor_55_21046_device::out_r()
{
	if (m_busy && !machine().side_effects_disabled())
	{
		m_busy = 0;
	}

	return m_out;
}


//-------------------------------------------------
//  4d_w -
//-------------------------------------------------

void luxor_55_21046_device::inp_w(uint8_t data)
{
	if (m_busy)
	{
		m_busy = 0;
	}

	m_inp = data;
}


//-------------------------------------------------
//  4b_w -
//-------------------------------------------------

void luxor_55_21046_device::_4b_w(uint8_t data)
{
	/*

	    bit     description

	    0       force busy
	    1
	    2
	    3
	    4       N/C
	    5       N/C
	    6
	    7

	*/

	m_status = data & 0xce;

	// busy
	if (!BIT(data, 0))
	{
		m_busy = 1;
	}
}


//-------------------------------------------------
//  9b_w -
//-------------------------------------------------

void luxor_55_21046_device::_9b_w(uint8_t data)
{
	/*

	    bit     signal      description

	    0       DS0         drive select 0
	    1       DS1         drive select 1
	    2       DS2         drive select 2
	    3       MTRON       motor on
	    4       TG43        track > 43
	    5       SIDE1       side 1 select
	    6
	    7

	*/

	// drive select
	m_selected_floppy = nullptr;

	for (int n = 0; n < 2; n++)
	{
		if (BIT(data, n))
			m_selected_floppy = m_floppy[n]->get_device();
	}

	m_fdc->set_floppy(m_selected_floppy);

	if (m_selected_floppy)
	{
		// motor enable
		m_selected_floppy->mon_w(!BIT(data, 3));

		// side select
		m_selected_floppy->ss_w(BIT(data, 5));
	}
}


//-------------------------------------------------
//  8a_w -
//-------------------------------------------------

void luxor_55_21046_device::_8a_w(uint8_t data)
{
	/*

	    bit     signal                          description

	    0       FD1793 _MR                      FDC master reset
	    1       FD1793 _DDEN, FDC9229 DENS      density select
	    2       FDC9229 MINI
	    3       FDC9229 P1
	    4       FDC9229 P2
	    5       READY signal polarity           (0=inverted)
	    6
	    7

	    FDC9229 P0 is grounded

	*/

	// FDC master reset
	m_fdc->mr_w(BIT(data, 0));

	// density select
	m_fdc->dden_w(BIT(data, 1));

	m_fdc->set_unscaled_clock(16_MHz_XTAL / (BIT(data, 2) ? 16 : 8));
}


//-------------------------------------------------
//  9a_r -
//-------------------------------------------------

uint8_t luxor_55_21046_device::_9a_r(offs_t offset)
{
	/*

	    bit     description

	    0       busy
	    1       _FD2S
	    2       SW2
	    3       S1/A: PAL16R4 pin 15, S1/B: GND
	    4       SW1-1 or DCG
	    5       SW1-2
	    6       SW1-3
	    7       SW1-4

	*/

	uint8_t data = 0;

	// busy
	data |= m_busy;

	// floppy
	data |= (m_selected_floppy ? m_selected_floppy->twosid_r() : 1) << 1;
	//data |= (m_selected_floppy ? m_selected_floppy->dskchg_r() : 1) << 4;

	// SW2
	uint8_t sw2 = m_sw2->read() & 0x0f;

	int sw2_1 = BIT(sw2, 0) ? 1 : BIT(offset, 8);
	int sw2_2 = BIT(sw2, 1) ? 1 : BIT(offset, 9);
	int sw2_3 = BIT(sw2, 2) ? 1 : BIT(offset, 10);
	int sw2_4 = BIT(sw2, 3) ? 1 : BIT(offset, 11);
	int sw2_data = !(sw2_1 && sw2_2 && !(sw2_3 ^ sw2_4));

	data |= sw2_data << 2;

	// SW1
	data |= (m_sw1->read() & 0x0f) << 4;

	return data ^ 0xff;
}
