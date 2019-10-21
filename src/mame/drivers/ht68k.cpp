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

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "imagedev/floppy.h"
#include "machine/mc68681.h"
#include "machine/wd_fdc.h"
#include "softlist.h"

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
		m_floppy(nullptr),
		m_p_ram(*this, "p_ram")
	{
	}

	void ht68k(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<mc68681_device> m_duart;
	required_device<wd1770_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	floppy_image_device *m_floppy;
	DECLARE_WRITE_LINE_MEMBER(duart_irq_handler);
	DECLARE_WRITE_LINE_MEMBER(duart_txb);
	DECLARE_WRITE8_MEMBER(duart_output);
	required_shared_ptr<uint16_t> m_p_ram;
	virtual void machine_reset() override;
	void ht68k_mem(address_map &map);
};


void ht68k_state::ht68k_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x0007ffff).ram().share("p_ram"); // 512 KB RAM / ROM at boot
	//map(0x00080000, 0x000fffff) // Expansion
	//map(0x00d80000, 0x00d8ffff) // Printer
	map(0x00e00000, 0x00e00007).mirror(0xfff8).rw(m_fdc, FUNC(wd1770_device::read), FUNC(wd1770_device::write)).umask16(0x00ff); // FDC WD1770
	map(0x00e80000, 0x00e800ff).mirror(0xff00).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0x00f00000, 0x00f07fff).rom().mirror(0xf8000).region("user1", 0);
}

/* Input ports */
static INPUT_PORTS_START( ht68k )
INPUT_PORTS_END

void ht68k_state::machine_reset()
{
	uint8_t* user1 = memregion("user1")->base();

	memcpy((uint8_t*)m_p_ram.target(),user1,0x8000);

	m_fdc->reset();
	m_fdc->set_floppy(nullptr);
}

WRITE_LINE_MEMBER(ht68k_state::duart_irq_handler)
{
	m_maincpu->set_input_line(M68K_IRQ_3, state);
}

WRITE_LINE_MEMBER(ht68k_state::duart_txb)
{
	//This is the second serial channel named AUX, for modem or other serial devices.
}

WRITE8_MEMBER(ht68k_state::duart_output)
{
	m_floppy = nullptr;

	if ((BIT(data, 7)) == 0) { m_floppy = m_floppy0->get_device(); }
	if ((BIT(data, 6)) == 0) { m_floppy = m_floppy1->get_device(); }
	if ((BIT(data, 5)) == 0) { m_floppy = m_floppy2->get_device(); }
	if ((BIT(data, 4)) == 0) { m_floppy = m_floppy3->get_device(); }

	m_fdc->set_floppy(m_floppy);

	if (m_floppy) {m_floppy->ss_w(BIT(data,3) ? 0 : 1);}
}

static void ht68k_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}


void ht68k_state::ht68k(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ht68k_state::ht68k_mem);

	/* video hardware */
	MC68681(config, m_duart, 8_MHz_XTAL / 2);
	m_duart->set_clocks(500000, 500000, 1000000, 1000000);
	m_duart->irq_cb().set(FUNC(ht68k_state::duart_irq_handler));
	m_duart->a_tx_cb().set("rs232", FUNC(rs232_port_device::write_txd));
	m_duart->b_tx_cb().set(FUNC(ht68k_state::duart_txb));
	m_duart->outport_cb().set(FUNC(ht68k_state::duart_output));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_duart, FUNC(mc68681_device::rx_a_w));

	WD1770(config, m_fdc, 8_MHz_XTAL);

	FLOPPY_CONNECTOR(config, "wd1770:0", ht68k_floppies, "525dd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, "wd1770:1", ht68k_floppies, "525dd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, "wd1770:2", ht68k_floppies, "525dd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, "wd1770:3", ht68k_floppies, "525dd", floppy_image_device::default_floppy_formats);

	SOFTWARE_LIST(config, "flop525_list").set_original("ht68k");
}

/* ROM definition */
ROM_START( ht68k )
	ROM_REGION( 0x8000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ht68k-u4.bin", 0x0000, 0x4000, CRC(3fbcdd0a) SHA1(45fcbbf920dc1e9eada3b7b0a55f5720d08ffdd5))
	ROM_LOAD16_BYTE( "ht68k-u3.bin", 0x0001, 0x4000, CRC(1d85d101) SHA1(8ba01e1595b0b3c4fb128a4a50242f3588b89c43))
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                 FULLNAME           FLAGS
COMP( 1987, ht68k, 0,      0,      ht68k,   ht68k, ht68k_state, empty_init, "Hawthorne Technology", "TinyGiant HT68k", MACHINE_NO_SOUND)
