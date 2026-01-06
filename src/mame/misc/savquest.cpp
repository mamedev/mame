// license:BSD-3-Clause
// copyright-holders:R. Belmont, Peter Ferrie
/***************************************************************************

    savquest.cpp

    "Savage Quest" (c) 1999 Interactive Light, developed by Angel Studios.
    Skeleton by R. Belmont

    TODO:
    - Needs proper AWE64 emulation defined as a slot option default, with
      fallbacks to AWE32 and SB16;
    \- to workaround: attrib -R C:\IMMERSIA\BIN\IMMOS.BAT then edit file to swap sound card

    - When switching gfx mode during boot routine it still sets a terminal debug mode (with cut down screen portions)
      instead of normal Voodoo drawing. Culprit may be an I/O port reading or a Voodoo bug;

    - Aforementioned debug mode shows that it can't find several assets on loading;

    - When game boots it does extensive CPUID checks, more copy protection tied to the CPU serial it has been installed on?

    - Convert HASP dongle to a pc_lpt_device friendly device;

    - currently asserts by selecting a s3 video bank above 1M (register 0x6a) Update: fixed?

    - The version is labeled "SQ05" in the filesystem but has the 1999 release year.
      Other components are labeled "v0.5", but the game doesn't boot far enough to see if
      any graphics have version information. There appears to also be a "Savage Quest 2.1" which
      is undumped.

    PCI list:
    Bus no. Device No. Func No. Vendor ID Device ID Device Class          IRQ
    0       7          1        8086      7111      IDE Controller        14
    0       7          2        8086      7112      Serial Bus Controller 11
    0       9          0        5333      8901      Display Controller    10
    0       13         0        121a      0002      Multimedia Device     NA
    - First two are PIIX4/4E/4M IDE Controller / PIIX4/4E/4M USB Interface
      Third is S3 trio64uv+
      Fourth is Voodoo 2 3D Accelerator
      Sound Blaster is ISA/PnP

============================================================================
    H/W is a white-box PC consisting of:
    Pentium II 450 CPU
    DFI P2XBL motherboard (i440BX chipset)
    128 MB RAM
    Guillemot Maxi Gamer 3D2 Voodoo II
    Sound Blaster AWE64

    Protected by a HASP brand parallel port dongle.
    I/O board has a PIC17C43 which is not readable.

    On boot it reports: S3 86C775/86C705 Video BIOS. Version 2.04.11 Copyright 1996 S3 Incorporated.

- update by Peter Ferrie:
- split BIOS region into 16kb blocks and implement missing PAM registers

- HASP emulator by Peter Ferrie


***************************************************************************/

#include "emu.h"

#include "bus/isa/sblaster.h"
#include "bus/isa/isa_cards.h"
#include "bus/pci/pci_slot.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "cpu/i386/i386.h"
#include "machine/fdc37c93x.h"
#include "machine/i82371eb_acpi.h"
#include "machine/i82371eb_ide.h"
#include "machine/i82371eb_isa.h"
#include "machine/i82371eb_usb.h"
#include "machine/i82371sb.h"
#include "machine/i82443bx_host.h"
#include "machine/pci.h"
#include "machine/pci-ide.h"
#include "machine/w83977tf.h"
#include "video/voodoo_2.h"
#include "video/voodoo_pci.h"


namespace {

class savquest_state : public driver_device
{
public:
	savquest_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_voodoo2(*this, "pci:0d.0")
	{
	}

	void savquest(machine_config &config);

protected:
	// driver_device overrides
//  virtual void video_start();

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<voodoo_2_pci_device> m_voodoo2;

	//int m_haspind = 0;
	//int m_haspstate = 0;
	//enum hasp_states
	//{
	//  HASPSTATE_NONE,
	//  HASPSTATE_PASSBEG,
	//  HASPSTATE_PASSEND,
	//  HASPSTATE_READ
	//};
	//int m_hasp_passind = 0;
	//uint8_t m_hasp_tmppass[0x29]{};
	//uint8_t m_port379 = 0;
	//int m_hasp_passmode = 0;
	//int m_hasp_prodind = 0;

	//uint8_t parallel_port_r(offs_t offset);
	//void parallel_port_w(offs_t offset, uint8_t data);

	void savquest_io(address_map &map) ATTR_COLD;
	void savquest_map(address_map &map) ATTR_COLD;

	static void winbond_superio_config(device_t *device);
};


// TODO: move into parallel port device
#if 0

uint8_t savquest_state::parallel_port_r(offs_t offset)
{
	if (offset == 1)
	{
		if ((m_haspstate == HASPSTATE_READ)
			&& (m_hasp_passmode == 3)
			)
		{
			/* passmode 3 is used to retrieve the product(s) information
			   it comes in two parts: header and product
			   the header has this format:
			   offset  range      purpose
			   00      01         header type
			   01      01-05      count of used product slots, must be 2
			   02      01-05      count of unused product slots
			                      this is assumed to be 6-(count of used slots)
			                      but it is not enforced here
			                      however a total of 6 structures will be checked
			   03      01-02      unknown
			   04      01-46      country code
			   05-0f   00         reserved
			   the used product slots have this format:
			   (the unused product slots must be entirely zeroes)
			   00-01   0001-000a  product ID, one must be 6, the other 0a
			   02      0001-0003  unknown but must be 0001
			   04      01-05      HASP plug country ID
			   05      01-02      unknown but must be 01
			   06      05         unknown
			   07-0a   any        unknown, not used
			   0b      ff         unknown
			   0c      ff         unknown
			   0d-0f   00         reserved

			   the read is performed by accessing an array of 16-bit big-endian values
			   and returning one bit at a time into bit 5 of the result
			   the 16-bit value is then XORed with 0x534d and the register index
			*/

			if (m_hasp_prodind <= (sizeof(m_hasp_prodinfo) * 8))
			{
				m_port379 = ((m_hasp_prodinfo[(m_hasp_prodind - 1) >> 3] >> ((8 - m_hasp_prodind) & 7)) & 1) << 5; /* return defined info */
			}
			else
			{
				m_port379 = (((0x534d ^ ((m_hasp_prodind - 1) >> 4)) >> ((16 - m_hasp_prodind) & 15)) & 1) << 5; /* then just alternate between the two key values */
			}

			++m_hasp_prodind;
		}

		return m_port379;
	}

	return 0;
}

void savquest_state::parallel_port_w(offs_t offset, uint8_t data)
{
	if (!offset)
	{
		uint8_t data8 = (uint8_t) (data & 0xff);

		/* state machine to determine when password is about to be entered */

		switch (m_haspind)
		{
			case 0:
			{
				if (data8 == 0xc6)
				{
					++m_haspind;
					break;
				}

				m_haspind = 0;
				break;
			}

			case 1:
			{
				if (data8 == 0xc7)
				{
					++m_haspind;
					break;
				}

				m_haspind = 0;
				break;
			}

			case 2:
			{
				if (data8 == 0xc6)
				{
					++m_haspind;
					break;
				}

				m_haspind = 0;
				m_haspstate = HASPSTATE_NONE;
				break;
			}

			case 3:
			{
				m_haspind = 0;

				if (data8 == 0x80)
				{
					m_haspstate = HASPSTATE_PASSBEG;
					m_hasp_passind = 0;
					return;
				}

				break;
			}

			default:
			{
			}
		}

		m_port379 = 0x00;

		if (m_haspstate == HASPSTATE_READ)
		{
			/* different passwords cause different values to be returned
			   but there are really only two passwords of interest
			   passmode 2 is used to verify that the dongle is responding correctly
			*/

			if (m_hasp_passmode == 2)
			{
				/* in passmode 2, some values remain unknown: 96, 9a, c4, d4, ec, f8
				   they all return 00, but if that's wrong then there will be failures to start
				*/

				if ((data8 == 0x94)
					|| (data8 == 0x9e)
					|| (data8 == 0xa4)
					|| (data8 == 0xb2)
					|| (data8 == 0xbe)
					|| (data8 == 0xd0)
					)
				{
					return;
				}

				if ((data8 == 0x8a)
					|| (data8 == 0x8e)
					|| (data8 == 0xca)
					|| (data8 == 0xd2)
					|| (data8 == 0xe2)
					|| (data8 == 0xf0)
					|| (data8 == 0xfc)
					)
				{
					/* someone with access to the actual dongle could dump the true values
					   I've never seen it so I just determined the relevant bits instead
					   from the disassembly of the software
					   some of the keys are verified explicitly, the others implicitly
					   I guessed the implicit ones with a bit of trial and error
					*/

					m_port379 = 0x20;
					return;
				}
			}

			switch (data8)
			{
				/* in passmode 0, some values remain unknown: 8a, 8e (inconclusive), 94, 96, 9a, a4, b2, be, c4, d2, d4 (inconclusive), e2, ec, f8, fc
				   this is less of a concern since the contents seem to decrypt correctly
				*/

				case 0x88:
				case 0x94:
				case 0x98:
				case 0x9c:
				case 0x9e:
				case 0xa0:
				case 0xa4:
				case 0xaa:
				case 0xae:
				case 0xb0:
				case 0xb2:
				case 0xbc:
				case 0xbe:
				case 0xc2:
				case 0xc6:
				case 0xc8:
				case 0xce:
				case 0xd0:
				case 0xd6:
				case 0xd8:
				case 0xdc:
				case 0xe0:
				case 0xe6:
				case 0xea:
				case 0xee:
				case 0xf2:
				case 0xf6:
				{
					/* again, just the relevant bits instead of the true values */

					m_port379 = 0x20;
					break;
				}

				default:
				{
				}
			}
		}
		else if (m_haspstate == HASPSTATE_PASSEND)
		{
			if (data8 & 1)
			{
				if ((m_hasp_passmode == 1)
					&& (data8 == 0x9d)
					)
				{
					m_hasp_passmode = 2;
				}

				m_haspstate = HASPSTATE_READ;
			}
			else if (m_hasp_passmode == 1)
			{
				m_hasp_tmppass[m_hasp_passind] = data8;

				if (++m_hasp_passind == sizeof(m_hasp_tmppass))
				{
					if ((m_hasp_tmppass[0] == 0x9c)
						&& (m_hasp_tmppass[1] == 0x9e)
						)
					{
						int i;

						i = 2;
						m_hasp_prodind = 0;

						do
						{
							m_hasp_prodind = (m_hasp_prodind << 1) + ((m_hasp_tmppass[i] >> 6) & 1);
						}
						while ((i += 3) < sizeof(m_hasp_tmppass));

						m_hasp_prodind = (m_hasp_prodind - 0xc08) << 4;

						if (m_hasp_prodind < (0x38 << 4))
						{
							m_hasp_passmode = 3;
						}
					}

					m_haspstate = HASPSTATE_READ;
				}
			}
		}
		else if ((m_haspstate == HASPSTATE_PASSBEG)
				&& (data8 & 1)
			)
		{
			m_hasp_tmppass[m_hasp_passind] = data8;

			if (++m_hasp_passind == sizeof(m_hasp_cmppass))
			{
				m_haspstate = HASPSTATE_PASSEND;
				m_hasp_passind = 0;
				m_hasp_passmode = (int) !memcmp(m_hasp_tmppass, m_hasp_cmppass, sizeof(m_hasp_cmppass));
			}
		}
	}
}
#endif

void savquest_state::savquest_map(address_map &map)
{
	map.unmap_value_high();
}

void savquest_state::savquest_io(address_map &map)
{
	map.unmap_value_high();
}

static INPUT_PORTS_START( savquest )
INPUT_PORTS_END

void savquest_state::machine_start()
{
}

void savquest_state::machine_reset()
{
}

static void isa_internal_devices(device_slot_interface &device)
{
	device.option_add("w83977tf", W83977TF);
}

void savquest_state::winbond_superio_config(device_t *device)
{
	w83977tf_device &fdc = *downcast<w83977tf_device *>(device);
//  fdc.set_sysopt_pin(1);
	fdc.gp20_reset().set_inputline(":maincpu", INPUT_LINE_RESET);
	fdc.gp25_gatea20().set_inputline(":maincpu", INPUT_LINE_A20);
	fdc.irq1().set(":pci:07.0", FUNC(i82371eb_isa_device::pc_irq1_w));
	fdc.irq8().set(":pci:07.0", FUNC(i82371eb_isa_device::pc_irq8n_w));
//  fdc.txd1().set(":serport0", FUNC(rs232_port_device::write_txd));
//  fdc.ndtr1().set(":serport0", FUNC(rs232_port_device::write_dtr));
//  fdc.nrts1().set(":serport0", FUNC(rs232_port_device::write_rts));
//  fdc.txd2().set(":serport1", FUNC(rs232_port_device::write_txd));
//  fdc.ndtr2().set(":serport1", FUNC(rs232_port_device::write_dtr));
//  fdc.nrts2().set(":serport1", FUNC(rs232_port_device::write_rts));

	auto *lpt = device->subdevice<pc_lpt_device>("lpt");
	auto *centronics = lpt->subdevice<centronics_device>("centronics");
	centronics->set_default_option("hasp_savquest");
	centronics->set_fixed(true);
}


void savquest_state::savquest(machine_config &config)
{
	pentium2_device &maincpu(PENTIUM2(config, "maincpu", 66'000'000)); // actually Pentium II @ 450 MHz
	maincpu.set_addrmap(AS_PROGRAM, &savquest_state::savquest_map);
	maincpu.set_addrmap(AS_IO, &savquest_state::savquest_io);
	maincpu.set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));
	maincpu.smiact().set("pci:00.0", FUNC(i82443bx_host_device::smi_act_w));

	PCI_ROOT(config, "pci", 0);
	I82443BX_HOST(config, "pci:00.0", 0, "maincpu", 128*1024*1024);
	I82443BX_BRIDGE(config, "pci:01.0", 0 );

	i82371eb_isa_device &isa(I82371EB_ISA(config, "pci:07.0", 0, "maincpu"));
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);

	i82371eb_ide_device &ide(I82371EB_IDE(config, "pci:07.1", 0, "maincpu"));
	ide.irq_pri().set("pci:07.0", FUNC(i82371eb_isa_device::pc_irq14_w));
	ide.irq_sec().set("pci:07.0", FUNC(i82371eb_isa_device::pc_mirq0_w));

	I82371EB_USB (config, "pci:07.2", 0);
	I82371EB_ACPI(config, "pci:07.3", 0);
	LPC_ACPI     (config, "pci:07.3:acpi", 0);
	SMBUS        (config, "pci:07.3:smbus", 0);

	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "w83977tf", true).set_option_machine_config("w83977tf", winbond_superio_config);
	// TODO: awe64 by default
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, "sblaster_16", false);
	ISA16_SLOT(config, "isa2", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

	VOODOO_2_PCI(config, m_voodoo2, 0, "maincpu", "voodoo_screen");
	m_voodoo2->set_fbmem(4);
	m_voodoo2->set_tmumem(4, 4); /* this is the 12Mb card */
	m_voodoo2->set_status_cycles(1000); // optimization to consume extra cycles when polling status

//  m_voodoo2->vblank_callback().set(FUNC(savquest_state::vblank_assert));

	screen_device &screen(SCREEN(config, "voodoo_screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57);
	screen.set_size(800, 262);
	screen.set_visarea(0, 512 - 1, 0, 240 - 1);
	screen.set_screen_update("pci:0d.0", FUNC(voodoo_2_pci_device::screen_update));

	PCI_SLOT(config, "pci:01.0:1", agp_cards, 1, 0, 1, 2, 3, nullptr);

	// TODO: trio64
	PCI_SLOT(config, "pci:1", pci_cards, 9, 0, 1, 2, 3, "virge").set_fixed(true);
	PCI_SLOT(config, "pci:2", pci_cards, 10, 1, 2, 3, 0, nullptr);
	PCI_SLOT(config, "pci:3", pci_cards, 11, 2, 3, 0, 1, nullptr);
//  PCI_SLOT(config, "pci:4", pci_cards, 12, 3, 0, 1, 2, "voodoo2");
}

ROM_START( savquest )
	ROM_REGION32_LE(0x40000, "pci:07.0", 0)
	ROM_LOAD( "p2xbl_award_451pg.bin", 0x00000, 0x040000, CRC(37d0030e) SHA1(c6773d0e02325116f95c497b9953f59a9ac81317) )

//  ROM_REGION32_LE( 0x10000, "video_bios", 0 ) // 1st half is 2.04.14, second half is 2.01.11
//  ROM_LOAD( "vgabios.bin",   0x000000, 0x010000, CRC(a81423d6) SHA1(a099af621ce7fbaa55a2d9947d9f07e04f1b5fca) )

	ROM_REGION( 0x080, "rtc", 0 )    /* default NVRAM */
	ROM_LOAD( "savquest_ds12885.bin", 0x0000, 0x080, BAD_DUMP CRC(e9270019) SHA1(4d900ca317d93c915c80a9053528b741746f08a1) )

	DISK_REGION( "pci:07.1:ide1:0:hdd" )
	DISK_IMAGE( "savquest", 0, SHA1(b7c8901172b66706a7ab5f5c91e6912855153fa9) )
ROM_END

} // Anonymous namespace


GAME(1999, savquest, 0, savquest, savquest, savquest_state, empty_init, ROT0, "Interactive Light", "Savage Quest", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
