// license:BSD-3-Clause
// copyright-holders:R. Justice
/*********************************************************************

    vistaa800.c

    Implementation of the Vista A800 8" disk Controller Card for the Apple II

    This supported up to four double sided/double density 8inch drives.
    With DMA support for the data transfers, and booting from the first 8inch drive.

    The card looks like it was released in 1981. The schematic is dated 19th Mar 1981 for the initial drawing, with a later revision date marked, however the month is not readable for it.

    Manual available here:
    http://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Interface%20Cards/Disk%20Drive%20Controllers/Vista%20A800%20Disk%20Controller/Manuals/Vista%20A800%20Disk%20Controller%20Manual.pdf

    I/O address details:
    Addr    Write                       Read
    ----    -----                       ----
    C0n0    1797 cmd reg                1797 status reg
    C0n1    1797 track reg              1797 track reg
    C0n2    1797 sector reg             1797 sector reg
    C0n3    1797 data reg               1797 data reg
    C0n4            --same as 0--
    C0n5            --same as 1--
    C0n6            --same as 2--
    C0n7            --same as 3--
    C0n8    low DMA address             not allowed
    C0n9    high DMA address            not allowed
    C0nA    DMA ON:Disk read            (same as write)
    C0nB    DMA ON:Disk write           (same as write)
    C0nC    DMA OFF                     (same as write)

            bit
            7    6    5 4  3  2  1  0
    C0nD    sngl side x x fd fd fd fd   not allowed
            dens sel  x x  3  2  1  0
    C0nE            --spare--

                                        bit
                                        7   6    543210
    C0nF    not allowed                 DMA one  xxxxxx
                                        on  side


*********************************************************************/

#include "emu.h"
#include "vistaa800.h"

#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"

namespace {

class a2bus_vistaa800_device : public device_t, public device_a2bus_card_interface
{
public:
	a2bus_vistaa800_device(machine_config const &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_a2bus_card_interface implementation
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual uint8_t read_c800(uint16_t offset) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	static void floppy_formats(format_registration &fr);

	// fdc handlers
	void fdc_intrq_w(uint8_t state);
	void fdc_drq_w(uint8_t state);

	required_device<fd1797_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	required_region_ptr<uint8_t> m_rom;

	uint16_t m_dmaaddr;
	bool m_dmaenable_read;
	bool m_dmaenable_write;
	uint8_t m_density;
	uint8_t m_side;
	uint8_t m_twosided;
};



static void vistaa800_floppies(device_slot_interface &device)
{
	device.option_add("8sssd",  FLOPPY_8_SSSD);       // 77 trks ss sd 8"
	device.option_add("8dssd",  FLOPPY_8_DSSD);       // 77 trks ds sd 8"
	device.option_add("8ssdd",  FLOPPY_8_SSDD);       // 77 trks ss dd 8"
	device.option_add("8dsdd",  FLOPPY_8_DSDD);       // 77 trks ds dd 8"
}

ROM_START(vistaa800)
	ROM_REGION(0x0800, "vistaa800_rom", 0)
	ROM_LOAD( "vista a800 boot 3.1.bin", 0x000000, 0x000800, CRC(22df90b8) SHA1(8e0b4f4c6c467e2280bd250ee54f471728640964) )
ROM_END


a2bus_vistaa800_device::a2bus_vistaa800_device(machine_config const &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, A2BUS_VISTAA800, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_fdc(*this, "fdc"),
	m_floppy0(*this, "fdc:0"),
	m_floppy1(*this, "fdc:1"),
	m_floppy2(*this, "fdc:2"),
	m_floppy3(*this, "fdc:3"),
	m_rom(*this, "vistaa800_rom")
{
}

//----------------------------------------------
//  device_t implementation
//----------------------------------------------

const tiny_rom_entry *a2bus_vistaa800_device::device_rom_region() const
{
	return ROM_NAME(vistaa800);
}

void a2bus_vistaa800_device::device_add_mconfig(machine_config &config)
{
	FD1797(config, m_fdc, 2'000'000);
	FLOPPY_CONNECTOR(config, m_floppy0, vistaa800_floppies, "8dsdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, vistaa800_floppies, "8dsdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy2, vistaa800_floppies, "8dsdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy3, vistaa800_floppies, "8dsdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	m_fdc->intrq_wr_callback().set(FUNC(a2bus_vistaa800_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(a2bus_vistaa800_device::fdc_drq_w));
}

void a2bus_vistaa800_device::device_start()
{
	save_item(NAME(m_dmaaddr));
	save_item(NAME(m_dmaenable_read));
	save_item(NAME(m_dmaenable_write));
	save_item(NAME(m_density));
	save_item(NAME(m_side));
	save_item(NAME(m_twosided));
}

void a2bus_vistaa800_device::device_reset()
{
	m_dmaaddr = 0;
	m_dmaenable_read = false;
	m_dmaenable_write = false;
	m_density = 0;
	m_side = 0;
	m_twosided = 0;
}

//----------------------------------------------
//  device_a2bus_card_interface implementation
//----------------------------------------------

uint8_t a2bus_vistaa800_device::read_c0nx(uint8_t offset)
{
	uint8_t result = 0;

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			result = m_fdc->fd1797_device::read(offset & 0x03);
			break;

		case 0xa:
			if (!machine().side_effects_disabled())
				m_dmaenable_read = true;
			break;

		case 0xb:
			if (!machine().side_effects_disabled())
				m_dmaenable_write = true;
			break;

		case 0xc:
			if (!machine().side_effects_disabled())
			{
				m_dmaenable_read = false;
				m_dmaenable_write = false;
			}
			break;

		case 0xf:
			if (m_dmaenable_read || m_dmaenable_write)
				result |= 0x80;

			result |= (m_twosided << 6); // 0 = two sided disk
			break;

		default:
			logerror("Read c0n%x (%s)\n", offset, machine().describe_context());
			break;
	}

	return result;
}

void a2bus_vistaa800_device::write_c0nx(uint8_t offset, uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			m_fdc->fd1797_device::write(offset & 0x03, data);
			break;

		case 8:
			m_dmaaddr = (m_dmaaddr & 0xff00) + data;
			break;

		case 9:
			m_dmaaddr = (m_dmaaddr & 0x00ff) + (data << 8);
			break;

		case 0xa:
			m_dmaenable_read = true;
			break;

		case 0xb:
			m_dmaenable_write = true;
			break;

		case 0xc:
			m_dmaenable_read = false;
			m_dmaenable_write = false;
			break;

		case 0xd:
			m_density = BIT(data, 7);
			m_fdc->dden_w(m_density);

			m_side = BIT(data, 6);

			if (BIT(data, 0)) floppy = m_floppy0->get_device();
			if (BIT(data, 1)) floppy = m_floppy1->get_device();
			if (BIT(data, 2)) floppy = m_floppy2->get_device();
			if (BIT(data, 3)) floppy = m_floppy3->get_device();
			m_fdc->set_floppy(floppy);

			if (floppy)
			{
				floppy->ss_w(m_side);
				floppy->mon_w(0);
				m_twosided = floppy->twosid_r();
			}
			break;

		default:
			logerror("Write %02x to c0n%x (%s)\n", data, offset, machine().describe_context());
			break;
	}
}

uint8_t a2bus_vistaa800_device::read_cnxx(uint8_t offset)
{
	return m_rom[offset];
}

uint8_t a2bus_vistaa800_device::read_c800(uint16_t offset)
{
	return m_rom[offset];
}

//-------------------------------------------------
//    fdc handlers
//-------------------------------------------------

void a2bus_vistaa800_device::fdc_intrq_w(uint8_t state)
{
	m_dmaenable_read = false;
	m_dmaenable_write = false;
}

void a2bus_vistaa800_device::fdc_drq_w(uint8_t state)
{
	if (state)
	{
		if (m_dmaenable_read) // read has priority if both were enabled
		{
			uint8_t data = m_fdc->data_r();
			slot_dma_write(m_dmaaddr, data);
			m_dmaaddr++;
		}
		else if (m_dmaenable_write)
		{
			uint8_t data = slot_dma_read(m_dmaaddr);
			m_fdc->data_w(data);
			m_dmaaddr++;
		}
	}
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_VISTAA800, device_a2bus_card_interface, a2bus_vistaa800_device, "a2vistaa800", "Vista A800 8inch disk Controller Card")
