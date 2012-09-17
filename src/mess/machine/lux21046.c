/**********************************************************************

    Luxor 55-21046 "fast" floppy disk controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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
|                   S1      LS240   |
|                                   |
|           LS244   SW3     DM8131  |
|                                   |
|                                   |
|--|-----------------------------|--|
   |------------CON1-------------|

Notes:
    All IC's shown.

    ROM     - Toshiba TMM27128D-20 16Kx8 EPROM "CNTR 1.07 6490318-07"
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
    S1      - Interface type (A:? B:ABCBUS)
    S3      - Interface type (A:? B:ABCBUS)
    S5      - Interface type (A:? B:ABCBUS)
    S6      - Amount of RAM installed (A:2KB, B:8KB)
    S7      - Number of drives connected (0:3, 1:2) *located on solder side
    S8      - Disk drive type (0:8", 1:5.25")
    S9      - Location of RDY signal (A:8" P2-6, B:5.25" P2-34)
    LD1     - LED

*/

/*

    TODO:

    - 8" floppy is not supported, but there are no dumps available either

*/

#include "lux21046.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80_TAG			"5ab"
#define Z80DMA_TAG		"6ab"
#define SAB1793_TAG		"7ab"
#define FDC9229_TAG		"8b"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type LUXOR_55_21046 = &device_creator<luxor_55_21046_device>;


//-------------------------------------------------
//  ROM( luxor_55_21046 )
//-------------------------------------------------

ROM_START( luxor_55_21046 )
	ROM_REGION( 0x4000, Z80_TAG, 0 ) // A13 is always high, thus loading at 0x2000
	ROM_LOAD_OPTIONAL( "cntr 108.6cd", 0x2000, 0x2000, CRC(229764cb) SHA1(a2e2f6f49c31b827efc62f894de9a770b65d109d) ) // 1986-03-12
	ROM_LOAD_OPTIONAL( "diab 207.6cd", 0x2000, 0x2000, CRC(86622f52) SHA1(61ad271de53152c1640c0b364fce46d1b0b4c7e2) ) // 1987-06-24
	ROM_LOAD( "cntr 1.07 6490318-07.6cd", 0x0000, 0x4000, CRC(db8c1c0e) SHA1(8bccd5bc72124984de529ee058df779f06d2c1d5) ) // 1985-07-03
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *luxor_55_21046_device::device_rom_region() const
{
	return ROM_NAME( luxor_55_21046 );
}


//-------------------------------------------------
//  ADDRESS_MAP( luxor_55_21046_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( luxor_55_21046_mem, AS_PROGRAM, 8, luxor_55_21046_device )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x1fff) AM_ROM AM_REGION(Z80_TAG, 0x2000)
	AM_RANGE(0x2000, 0x3fff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( luxor_55_21046_io )
//-------------------------------------------------

static ADDRESS_MAP_START( luxor_55_21046_io, AS_IO, 8, luxor_55_21046_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xff0f) AM_READ(_3d_r)
	AM_RANGE(0x10, 0x10) AM_MIRROR(0xff0f) AM_WRITE(_4d_w)
	AM_RANGE(0x20, 0x20) AM_MIRROR(0xff0f) AM_WRITE(_4b_w)
	AM_RANGE(0x30, 0x30) AM_MIRROR(0xff0f) AM_WRITE(_9b_w)
	AM_RANGE(0x40, 0x40) AM_MIRROR(0xff0f) AM_WRITE(_8a_w)
	AM_RANGE(0x50, 0x50) AM_MIRROR(0xff0f) AM_MASK(0xff00) AM_READ(_9a_r)
	AM_RANGE(0x60, 0x63) AM_MIRROR(0xff0c) AM_DEVREAD_LEGACY(SAB1793_TAG, wd17xx_r)
	AM_RANGE(0x70, 0x73) AM_MIRROR(0xff0c) AM_DEVWRITE_LEGACY(SAB1793_TAG, wd17xx_w)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0xff0f) AM_DEVREADWRITE_LEGACY(Z80DMA_TAG, z80dma_r, z80dma_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  Z80DMA_INTERFACE( dma_intf )
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

WRITE_LINE_MEMBER( luxor_55_21046_device::dma_int_w )
{
	m_dma_irq = state;

	// FDC and DMA interrupts are wire-ORed to the Z80
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_fdc_irq | m_dma_irq);
}

static UINT8 memory_read_byte(address_space &space, offs_t address, UINT8 mem_mask) { return space.read_byte(address); }
static void memory_write_byte(address_space &space, offs_t address, UINT8 data, UINT8 mem_mask) { space.write_byte(address, data); }

static Z80DMA_INTERFACE( dma_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_HALT),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, luxor_55_21046_device, dma_int_w),
	DEVCB_NULL,
	DEVCB_MEMORY_HANDLER(Z80_TAG, PROGRAM, memory_read_byte),
	DEVCB_MEMORY_HANDLER(Z80_TAG, PROGRAM, memory_write_byte),
	DEVCB_MEMORY_HANDLER(Z80_TAG, IO, memory_read_byte),
	DEVCB_MEMORY_HANDLER(Z80_TAG, IO, memory_write_byte)
};


//-------------------------------------------------
//  wd17xx_interface fdc_intf
//-------------------------------------------------

static const floppy_interface lux21046_floppy_interface =
{
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    FLOPPY_STANDARD_5_25_DSDD,
    LEGACY_FLOPPY_OPTIONS_NAME(default),
    "floppy_5_25",
	NULL
};

WRITE_LINE_MEMBER( luxor_55_21046_device::fdc_intrq_w )
{
	m_fdc_irq = state;

	// FDC and DMA interrupts are wire-ORed to the Z80
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_fdc_irq | m_dma_irq);
}

static const wd17xx_interface fdc_intf =
{
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, luxor_55_21046_device, fdc_intrq_w),
	DEVCB_DEVICE_LINE(Z80DMA_TAG, z80dma_rdy_w),
	{ FLOPPY_0, FLOPPY_1, NULL, NULL }
};


//-------------------------------------------------
//  MACHINE_DRIVER( luxor_55_21046 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( luxor_55_21046 )
	// main CPU
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_16MHz/4)
	MCFG_CPU_PROGRAM_MAP(luxor_55_21046_mem)
	MCFG_CPU_IO_MAP(luxor_55_21046_io)

	// devices
	MCFG_Z80DMA_ADD(Z80DMA_TAG, XTAL_16MHz/4, dma_intf)
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(lux21046_floppy_interface)
	MCFG_FD1793_ADD(SAB1793_TAG, fdc_intf)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor luxor_55_21046_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( luxor_55_21046 );
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
	PORT_DIPNAME( 0x01, 0x00, "Drive 0 Sided" ) PORT_DIPLOCATION("SW1:1") PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2d)
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x01, "Double" )
	PORT_DIPNAME( 0x02, 0x00, "Drive 1 Sided" ) PORT_DIPLOCATION("SW1:2") PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2d)
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x02, "Double" )
	PORT_DIPNAME( 0x04, 0x00, "Drive 0 Density" ) PORT_DIPLOCATION("SW1:3") PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2d)
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x04, "Double" )
	PORT_DIPNAME( 0x08, 0x00, "Drive 1 Density" ) PORT_DIPLOCATION("SW1:4") PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2d)
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x08, "Double" )
	// ABC 832/834/850
	PORT_DIPNAME( 0x01, 0x01, "Drive 0 Sided" ) PORT_DIPLOCATION("SW1:1") PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2c)
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x01, "Double" )
	PORT_DIPNAME( 0x02, 0x02, "Drive 1 Sided" ) PORT_DIPLOCATION("SW1:2") PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2c)
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x02, "Double" )
	PORT_DIPNAME( 0x04, 0x00, "Drive 0 Tracks" ) PORT_DIPLOCATION("SW1:3") PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2c)
	PORT_DIPSETTING(    0x00, "80" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPNAME( 0x08, 0x00, "Drive 1 Tracks" ) PORT_DIPLOCATION("SW1:4") PORT_CONDITION("SW3", 0x7f, EQUALS, 0x2c)
	PORT_DIPSETTING(    0x00, "80" )
	PORT_DIPSETTING(    0x08, "40" )

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
	PORT_DIPSETTING(    0x2c, "44 (ABC 832/834/850)" )
	PORT_DIPSETTING(    0x2d, "45 (ABC 830)" )
	PORT_DIPSETTING(    0x2e, "46 (ABC 838)" )

	PORT_START("S6")
	PORT_DIPNAME( 0x01, 0x01, "RAM Size" ) PORT_DIPLOCATION("S6:1")
	PORT_DIPSETTING(    0x00, "2 KB" )
	PORT_DIPSETTING(    0x01, "8 KB" )

	PORT_START("S8")
	PORT_DIPNAME( 0x01, 0x01, "Drive Type" ) PORT_DIPLOCATION("S8:1")
	PORT_DIPSETTING(    0x00, "8\"" )
	PORT_DIPSETTING(    0x01, "5.25\"" )

	PORT_START("S9")
	PORT_DIPNAME( 0x01, 0x01, "RDY Pin" ) PORT_DIPLOCATION("S9:1")
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



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  luxor_55_21046_device - constructor
//-------------------------------------------------

luxor_55_21046_device::luxor_55_21046_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, LUXOR_55_21046, "Luxor 55 21046", tag, owner, clock),
	  device_abcbus_card_interface(mconfig, *this),
	  m_maincpu(*this, Z80_TAG),
	  m_dma(*this, Z80DMA_TAG),
	  m_fdc(*this, SAB1793_TAG),
	  m_image0(*this, FLOPPY_0),
	  m_image1(*this, FLOPPY_1),
	  m_cs(false),
	  m_fdc_irq(0),
	  m_dma_irq(0),
	  m_busy(0),
	  m_force_busy(0)
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
	save_item(NAME(m_data_in));
	save_item(NAME(m_data_out));
	save_item(NAME(m_fdc_irq));
	save_item(NAME(m_dma_irq));
	save_item(NAME(m_busy));
	save_item(NAME(m_force_busy));

	// patch out sector skew table
/*  UINT8 *rom = memregion(Z80_TAG)->base();

    for (int i = 0; i < 16; i++)
        rom[0x2dd3 + i] = i + 1;*/
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void luxor_55_21046_device::device_reset()
{
	m_cs = false;

	floppy_mon_w(m_image0, ASSERT_LINE);
	floppy_mon_w(m_image1, ASSERT_LINE);
	floppy_drive_set_ready_state(m_image0, 1, 1);
	floppy_drive_set_ready_state(m_image1, 1, 1);
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_cs -
//-------------------------------------------------

void luxor_55_21046_device::abcbus_cs(UINT8 data)
{
	m_cs = (data == ioport("SW3")->read());
}


//-------------------------------------------------
//  abcbus_rst -
//-------------------------------------------------

void luxor_55_21046_device::abcbus_rst(int state)
{
	if (!state)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  abcbus_stat -
//-------------------------------------------------

UINT8 luxor_55_21046_device::abcbus_stat()
{
	UINT8 data = 0;

	if (m_cs)
	{
		data = (m_status & 0xce) | m_busy;
	}

	// LS240 inverts the data
	return data ^ 0xff;
}


//-------------------------------------------------
//  abcbus_inp -
//-------------------------------------------------

UINT8 luxor_55_21046_device::abcbus_inp()
{
	UINT8 data = 0xff;

	if (m_cs)
	{
		data = m_data_out;
		m_busy = 1;
	}

	return data;
}


//-------------------------------------------------
//  abcbus_utp -
//-------------------------------------------------

void luxor_55_21046_device::abcbus_utp(UINT8 data)
{
	if (m_cs)
	{
		m_data_in = data;
		m_busy = 1;
	}
}


//-------------------------------------------------
//  abcbus_c1 -
//-------------------------------------------------

void luxor_55_21046_device::abcbus_c1(UINT8 data)
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

void luxor_55_21046_device::abcbus_c3(UINT8 data)
{
	if (m_cs)
	{
		m_maincpu->reset();
	}
}



//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  3d_r -
//-------------------------------------------------

READ8_MEMBER( luxor_55_21046_device::_3d_r )
{
	if (BIT(m_status, 0))
	{
		m_busy = 0;
	}

	return m_data_in;
}


//-------------------------------------------------
//  4d_w -
//-------------------------------------------------

WRITE8_MEMBER( luxor_55_21046_device::_4d_w )
{
	if (BIT(m_status, 0))
	{
		m_busy = 0;
	}

	m_data_out = data;
}


//-------------------------------------------------
//  4b_w -
//-------------------------------------------------

WRITE8_MEMBER( luxor_55_21046_device::_4b_w )
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

	m_status = data;

	// busy
	if (!BIT(m_status, 0))
	{
		m_busy = 1;
	}
}


//-------------------------------------------------
//  9b_w -
//-------------------------------------------------

WRITE8_MEMBER( luxor_55_21046_device::_9b_w )
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
	if (BIT(data, 0)) wd17xx_set_drive(m_fdc, 0);
	if (BIT(data, 1)) wd17xx_set_drive(m_fdc, 1);
	//if (BIT(data, 2)) wd17xx_set_drive(m_fdc, 2);

	// motor enable
	int mtron = BIT(data, 3);
	floppy_mon_w(m_image0, !mtron);
	floppy_mon_w(m_image1, !mtron);
	floppy_drive_set_ready_state(m_image0, mtron, 1);
	floppy_drive_set_ready_state(m_image1, mtron, 1);

	// side select
	wd17xx_set_side(m_fdc, BIT(data, 5));
}


//-------------------------------------------------
//  8a_w -
//-------------------------------------------------

WRITE8_MEMBER( luxor_55_21046_device::_8a_w )
{
	/*

        bit     signal                          description

        0       FD1793 _MR                      FDC master reset
        1       FD1793 _DDEN, FDC9229 DENS      density select
        2       FDC9229 MINI
        3       READY signal polarity           (0=inverted)
        4       FDC9229 P2
        5       FDC9229 P1
        6
        7

        FDC9229 P0 is grounded

    */

	// FDC master reset
	wd17xx_mr_w(m_fdc, BIT(data, 0));

	// density select
	wd17xx_dden_w(m_fdc, BIT(data, 1));
}


//-------------------------------------------------
//  9a_r -
//-------------------------------------------------

READ8_MEMBER( luxor_55_21046_device::_9a_r )
{
	/*

        bit     signal      description

        0       busy        controller busy
        1       _FD2S       double-sided disk
        2       SW2
        3       _DCG ?      disk changed
        4       SW1-1
        5       SW1-2
        6       SW1-3
        7       SW1-4

    */

	UINT8 data = 0;

	// busy
	data |= m_busy;

	// SW1
	UINT8 sw1 = ioport("SW1")->read() & 0x0f;

	data |= sw1 << 4;

	// SW2
	UINT8 sw2 = ioport("SW2")->read() & 0x0f;

	// TTL inputs float high so DIP switch in off position equals 1
	int sw2_1 = BIT(sw2, 0) ? 1 : BIT(offset, 8);
	int sw2_2 = BIT(sw2, 1) ? 1 : BIT(offset, 9);
	int sw2_3 = BIT(sw2, 2) ? 1 : BIT(offset, 10);
	int sw2_4 = BIT(sw2, 3) ? 1 : BIT(offset, 11);
	int sw2_data = !(sw2_1 & sw2_2 & !(sw2_3 ^ sw2_4));

	data |= sw2_data << 2;

	return data ^ 0xff;
}


//**************************************************************************
//  LUXOR 55 21046 DEVICE INPUT DEFAULTS
//**************************************************************************

//-------------------------------------------------
//  DEVICE_INPUT_DEFAULTS( abc830_fast_intf )
//-------------------------------------------------

DEVICE_INPUT_DEFAULTS_START( abc830_fast )
	DEVICE_INPUT_DEFAULTS("SW1", 0x0f, 0x03)
	DEVICE_INPUT_DEFAULTS("SW2", 0x0f, DRIVE_BASF_6106_08)
	DEVICE_INPUT_DEFAULTS("SW3", 0x7f, ADDRESS_ABC830)
DEVICE_INPUT_DEFAULTS_END


//-------------------------------------------------
//  DEVICE_INPUT_DEFAULTS( abc832_fast )
//-------------------------------------------------

DEVICE_INPUT_DEFAULTS_START( abc832_fast )
	DEVICE_INPUT_DEFAULTS("SW1", 0x0f, 0x03)
	DEVICE_INPUT_DEFAULTS("SW2", 0x0f, DRIVE_TEAC_FD55F)
	DEVICE_INPUT_DEFAULTS("SW3", 0x7f, ADDRESS_ABC832)
DEVICE_INPUT_DEFAULTS_END


//-------------------------------------------------
//  DEVICE_INPUT_DEFAULTS( abc834_fast )
//-------------------------------------------------

DEVICE_INPUT_DEFAULTS_START( abc834_fast )
	DEVICE_INPUT_DEFAULTS("SW1", 0x0f, 0x03)
	DEVICE_INPUT_DEFAULTS("SW2", 0x0f, DRIVE_TEAC_FD55F)
	DEVICE_INPUT_DEFAULTS("SW3", 0x7f, ADDRESS_ABC832)
DEVICE_INPUT_DEFAULTS_END


//-------------------------------------------------
//  DEVICE_INPUT_DEFAULTS( abc838_fast )
//-------------------------------------------------

DEVICE_INPUT_DEFAULTS_START( abc838_fast )
	DEVICE_INPUT_DEFAULTS("SW1", 0x0f, 0x03)
	DEVICE_INPUT_DEFAULTS("SW2", 0x0f, DRIVE_BASF_6105)
	DEVICE_INPUT_DEFAULTS("SW3", 0x7f, ADDRESS_ABC838)
DEVICE_INPUT_DEFAULTS_END


//-------------------------------------------------
//  DEVICE_INPUT_DEFAULTS( abc850_fast )
//-------------------------------------------------

DEVICE_INPUT_DEFAULTS_START( abc850_fast )
	DEVICE_INPUT_DEFAULTS("SW1", 0x0f, 0x03)
	DEVICE_INPUT_DEFAULTS("SW2", 0x0f, DRIVE_BASF_6138)
	DEVICE_INPUT_DEFAULTS("SW3", 0x7f, ADDRESS_ABC830)
DEVICE_INPUT_DEFAULTS_END
