// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    corvfdc02.c

    Implemention of the Corvus Systems CORVUS02 floppy controller
    aka the "Buffered Floppy Controller"

    Boot PROM 0.8 says 8" SSDD or 5.25" DSDD; we stick with 5.25" here
    and let the FDC01 handle 8".

*********************************************************************/

#include "emu.h"
#include "corvfdc02.h"

#include "imagedev/floppy.h"
#include "machine/upd765.h"

#include "formats/concept_dsk.h"


namespace {

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

#define FDC02_ROM_REGION    "fdc02_rom"
#define FDC02_FDC_TAG       "fdc02_fdc"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_corvfdc02_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_corvfdc02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_corvfdc02_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void reset_from_bus() override;

	TIMER_CALLBACK_MEMBER(tc_tick);

	required_region_ptr<uint8_t> m_rom;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 4> m_con;

private:
	void intrq_w(int state);
	void drq_w(int state);

	static void corv_floppy_formats(format_registration &fr);

	uint8_t m_fdc_local_status, m_fdc_local_command;
	uint16_t m_bufptr;
	uint8_t m_buffer[2048];   // 1x6116 SRAM
	floppy_image_device *m_curfloppy;
	bool m_in_drq;
	emu_timer *m_timer;
};


void a2bus_corvfdc02_device::corv_floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_CONCEPT_525DSDD_FORMAT);
}

static void corv_floppies(device_slot_interface &device)
{
	device.option_add("525dsqd", FLOPPY_525_QD);
}

ROM_START( fdc02 )
	ROM_REGION(0x20, FDC02_ROM_REGION, 0)
	ROM_LOAD( "bfc00.bin", 0x000000, 0x000020, CRC(98d1a765) SHA1(d27c3c6921e1bb3778a3f78decf106275bc0add1) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_corvfdc02_device::device_add_mconfig(machine_config &config)
{
	UPD765A(config, m_fdc, 16_MHz_XTAL / 2, true, false); // clocked through FDC9229BT
	m_fdc->intrq_wr_callback().set(FUNC(a2bus_corvfdc02_device::intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(a2bus_corvfdc02_device::drq_w));
	for (auto &con : m_con)
		FLOPPY_CONNECTOR(config, con, corv_floppies, "525dsqd", a2bus_corvfdc02_device::corv_floppy_formats);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_corvfdc02_device::device_rom_region() const
{
	return ROM_NAME( fdc02 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_corvfdc02_device::a2bus_corvfdc02_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_rom(*this, FDC02_ROM_REGION),
	m_fdc(*this, FDC02_FDC_TAG),
	m_con(*this, FDC02_FDC_TAG":%u", 0U),
	m_fdc_local_status(0), m_fdc_local_command(0), m_bufptr(0), m_curfloppy(nullptr), m_in_drq(false), m_timer(nullptr)
{
}

a2bus_corvfdc02_device::a2bus_corvfdc02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_corvfdc02_device(mconfig, A2BUS_CORVFDC02, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_corvfdc02_device::device_start()
{
	m_timer = timer_alloc(FUNC(a2bus_corvfdc02_device::tc_tick), this);

	save_item(NAME(m_fdc_local_status));
	save_item(NAME(m_fdc_local_command));
	save_item(NAME(m_bufptr));
	save_item(NAME(m_buffer));
}

void a2bus_corvfdc02_device::device_reset()
{
	m_fdc_local_status = 2;
	m_fdc_local_command = 0;
	m_curfloppy = nullptr;
	m_in_drq = false;
	m_timer->adjust(attotime::never);
}

void a2bus_corvfdc02_device::reset_from_bus()
{
	m_fdc->reset();
}

TIMER_CALLBACK_MEMBER(a2bus_corvfdc02_device::tc_tick)
{
	m_fdc->tc_w(true);
	m_fdc->tc_w(false);
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_corvfdc02_device::read_c0nx(uint8_t offset)
{
	switch (offset)
	{
		case 0: // 765 FIFO
			return m_fdc->fifo_r();

		case 1: // 765 MSR
			return m_fdc->msr_r();

		case 2: // buffer address
			return (m_bufptr>>1) & 0xff;

		case 3:
//          printf("Read buffer @ %x = %02x\n", m_bufptr, m_buffer[m_bufptr]);
			return m_buffer[m_bufptr--];

		case 4: // local status
			if (m_curfloppy)
			{
				m_fdc_local_status &= ~(1 | 0x40);
				m_fdc_local_status |= m_curfloppy->dskchg_r() ? 1 : 0;
				m_fdc_local_status |= m_curfloppy->ready_r() ? 0x40 : 0;
			}
			return m_fdc_local_status;
			break;
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_corvfdc02_device::write_c0nx(uint8_t offset, uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	switch (offset)
	{
		case 0:    // FDC FIFO write
			m_fdc->fifo_w(data);
			break;

		case 1:    // FDC ???
			break;

		case 2: // buffer address
			m_bufptr = (data << 1) | (data & 1);
//          printf("%02x to buffer address yields %x\n", data, m_bufptr);
			break;

		case 3: // buffer write
//          printf("%02x to buffer[%x]\n", data, m_bufptr);
			m_buffer[m_bufptr--] = data;
			break;

		case 4:     // LOCAL COMMAND REG
			m_fdc_local_command = data;

			// drive select enabled?
			if (data & 4)
			{
				floppy = m_con[data & 3] ? m_con[data & 3]->get_device() : nullptr;

				logerror("corvfdc02: selecting drive %d: %p\n", data & 3, (void *)floppy);

				if (floppy != m_curfloppy)
				{
					m_fdc->set_floppy(floppy);
					m_curfloppy = floppy;
				}
			}

			if (m_curfloppy != nullptr)
			{
				// motor control (active low)
				m_curfloppy->mon_w((data & 8) ? 1 : 0);
//              printf("Cur drive %p motor %s\n", m_curfloppy, (data & 8) ? "OFF" : "ON");
			}

			if (data & 0x80)
			{
//              printf("Reset NEC765\n");
				m_fdc->reset();
			}
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_corvfdc02_device::read_cnxx(uint8_t offset)
{
	return m_rom[offset & 0x1f];
}

void a2bus_corvfdc02_device::intrq_w(int state)
{
	if (state)
	{
		m_fdc_local_status &= ~2;   // indicate IRQ occurred
		if (m_fdc_local_command & 0x20)
		{
			raise_slot_irq();
		}
	}
	else
	{
		m_fdc_local_status |= 2;    // clear IRQ
		lower_slot_irq();
	}
}

void a2bus_corvfdc02_device::drq_w(int state)
{
	if (state)
	{
		// pseudo-DMA direction?
		if (m_fdc_local_command & 0x40)
		{
			m_buffer[m_bufptr] = m_fdc->dma_r();
//          printf("DMA %02x to buffer[%x]\n", m_buffer[m_bufptr], m_bufptr);

			if (!m_bufptr)
			{
				m_timer->adjust(attotime::zero);
			}

			m_bufptr--;
			m_bufptr &= 0x1ff;
		}
		else
		{
			m_fdc->dma_w(m_buffer[m_bufptr++]);
			m_bufptr &= 0x1ff;
		}
	}
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_CORVFDC02, device_a2bus_card_interface, a2bus_corvfdc02_device, "crvfdc02", "Corvus Systems Buffered Floppy Controller")
