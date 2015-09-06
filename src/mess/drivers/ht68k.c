// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Hawthorne Technology TinyGiant HT68k

        29/11/2009 Skeleton driver.

Monitor commands (from ht68k manual) (must be in Uppercase)
B Load binary file from disk.
C Compare memory.
D Display memory in Hex and ASCII.
E Examine and/or change memory (2 bytes). (. to escape)
F Fill a block of memory with a value.
G Go to an address and execute program.
K Peek.
L Load program from serial port.
M Move memory.
P Poke.
R Read binary file from disk.
S System boot.
T Test printer.
W Write binary file to disk.
X Examine long and/or change memory (4 bytes). (. to escape)


Lot of infos available at: http://www.classiccmp.org/cini/ht68k.htm

****************************************************************************/

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68681.h"
#include "machine/wd_fdc.h"

class ht68k_state : public driver_device
{
public:
	ht68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_duart(*this, "duart68681"),
		m_fdc(*this, "wd1770"),
		m_floppy0(*this, "wd1770:0"),
		m_floppy1(*this, "wd1770:1"),
		m_floppy2(*this, "wd1770:2"),
		m_floppy3(*this, "wd1770:3"),
		m_floppy(NULL),
		m_p_ram(*this, "p_ram")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<mc68681_device> m_duart;
	required_device<wd1770_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	floppy_image_device *m_floppy;
	DECLARE_WRITE_LINE_MEMBER(duart_irq_handler);
	DECLARE_WRITE_LINE_MEMBER(duart_txb);
	DECLARE_WRITE8_MEMBER(duart_output);
	required_shared_ptr<UINT16> m_p_ram;
	virtual void machine_reset();
};


static ADDRESS_MAP_START(ht68k_mem, AS_PROGRAM, 16, ht68k_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x0007ffff) AM_RAM AM_SHARE("p_ram") // 512 KB RAM / ROM at boot
	//AM_RANGE(0x00080000, 0x000fffff) // Expansion
	//AM_RANGE(0x00d80000, 0x00d8ffff) // Printer
	AM_RANGE(0x00e00000, 0x00e00007) AM_MIRROR(0xfff8) AM_DEVREADWRITE8("wd1770", wd1770_t, read, write, 0x00ff) // FDC WD1770
	AM_RANGE(0x00e80000, 0x00e800ff) AM_MIRROR(0xff00) AM_DEVREADWRITE8("duart68681", mc68681_device, read, write, 0xff )
	AM_RANGE(0x00f00000, 0x00f07fff) AM_ROM AM_MIRROR(0xf8000) AM_REGION("user1",0)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( ht68k )
INPUT_PORTS_END

void ht68k_state::machine_reset()
{
	UINT8* user1 = memregion("user1")->base();

	memcpy((UINT8*)m_p_ram.target(),user1,0x8000);

	m_maincpu->reset();

	m_fdc->reset();
	m_fdc->set_floppy(NULL);
}

WRITE_LINE_MEMBER(ht68k_state::duart_irq_handler)
{
	m_maincpu->set_input_line_and_vector(M68K_IRQ_3, state, M68K_INT_ACK_AUTOVECTOR);
}

WRITE_LINE_MEMBER(ht68k_state::duart_txb)
{
	//This is the second serial channel named AUX, for modem or other serial devices.
}

WRITE8_MEMBER(ht68k_state::duart_output)
{
	m_floppy = NULL;

	if ((BIT(data, 7)) == 0) { m_floppy = m_floppy0->get_device(); }
	if ((BIT(data, 6)) == 0) { m_floppy = m_floppy1->get_device(); }
	if ((BIT(data, 5)) == 0) { m_floppy = m_floppy2->get_device(); }
	if ((BIT(data, 4)) == 0) { m_floppy = m_floppy3->get_device(); }

	m_fdc->set_floppy(m_floppy);

	if (m_floppy) {m_floppy->ss_w(BIT(data,3) ? 0 : 1);}
}

static SLOT_INTERFACE_START( ht68k_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END


static MACHINE_CONFIG_START( ht68k, ht68k_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(ht68k_mem)

	/* video hardware */
	MCFG_MC68681_ADD( "duart68681", XTAL_8MHz / 2 )
	MCFG_MC68681_SET_EXTERNAL_CLOCKS(500000, 500000, 1000000, 1000000)
	MCFG_MC68681_IRQ_CALLBACK(WRITELINE(ht68k_state, duart_irq_handler))
	MCFG_MC68681_A_TX_CALLBACK(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_MC68681_B_TX_CALLBACK(WRITELINE(ht68k_state, duart_txb))
	MCFG_MC68681_OUTPORT_CALLBACK(WRITE8(ht68k_state, duart_output))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("duart68681", mc68681_device, rx_a_w))

	MCFG_WD1770_ADD("wd1770", XTAL_8MHz )

	MCFG_FLOPPY_DRIVE_ADD("wd1770:0", ht68k_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("wd1770:1", ht68k_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("wd1770:2", ht68k_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("wd1770:3", ht68k_floppies, "525dd", floppy_image_device::default_floppy_formats)

	MCFG_SOFTWARE_LIST_ADD("flop525_list", "ht68k")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ht68k )
	ROM_REGION( 0x8000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ht68k-u4.bin", 0x0000, 0x4000, CRC(3fbcdd0a) SHA1(45fcbbf920dc1e9eada3b7b0a55f5720d08ffdd5))
	ROM_LOAD16_BYTE( "ht68k-u3.bin", 0x0001, 0x4000, CRC(1d85d101) SHA1(8ba01e1595b0b3c4fb128a4a50242f3588b89c43))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                     FULLNAME                    FLAGS */
COMP( 1987, ht68k,  0,       0,      ht68k,     ht68k, driver_device,   0,   "Hawthorne Technology", "TinyGiant HT68k", MACHINE_NO_SOUND)
