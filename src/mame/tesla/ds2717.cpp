// license:BSD-3-Clause
// copyright-holders:Felipe Corrêa da Silva Sanches
/*******************************************************************************

    Consul 2717 DS 2717 floppy disk controller

    The board has no processor of its own: an 8272 FDC, an 8253 timer, a
    74LS174 control latch and decoding and data separator logic (schematic
    616 143).  The Consul's monitor ROM drives it through ports C8-CF and
    boots CP/M from the disc in drive A.  Drives A and B are FDC units 2
    and 3.

    C8  in   FDC main status
    C9  r/w  FDC data
    CA  in   bit 0: FDC interrupt
    CA  out  control latch
    CC-CF    8253

    The FDC runs in DMA mode, but the board has no DMA controller: its DRQ
    drives the 8080 INTR line, and with no vector on the bus the CPU takes
    RST 7 for every byte.  8253 OUT0 drives the FDC's TC input.  The monitor
    loads counter 0 with the sector size less one, so it is taken to count
    the bytes transferred.

    Counters 1 and 2 are cascaded.  The disc BIOS loads counter 1 with
    10000 and counter 2 with 0, and reads counter 2 back as its keyboard
    repeat timebase.

    TODO:
    - control latch (drive select, motor, data separator)
    - the classroom network

*******************************************************************************/

#include "emu.h"
#include "ds2717.h"

#include "formats/ds2717_dsk.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(DS2717, ds2717_device, "ds2717", "Consul 2717 DS 2717 floppy disk controller")


ds2717_device::ds2717_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DS2717, tag, owner, clock)
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 2U)
	, m_pit(*this, "pit")
	, m_int_cb(*this)
	, m_drq(false)
	, m_intrq(false)
	, m_tc(false)
{
}


void ds2717_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_DS2717_FORMAT);
}

static void ds2717_floppies(device_slot_interface &device)
{
	device.option_add("8sssd", FLOPPY_8_SSSD);
}

void ds2717_device::device_add_mconfig(machine_config &config)
{
	I8272A(config, m_fdc, 8_MHz_XTAL);
	m_fdc->intrq_wr_callback().set(FUNC(ds2717_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(ds2717_device::fdc_drq_w));

	FLOPPY_CONNECTOR(config, m_floppy[0], ds2717_floppies, "8sssd", ds2717_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], ds2717_floppies, "8sssd", ds2717_device::floppy_formats).enable_sound(true);

	PIT8253(config, m_pit);
	m_pit->out_handler<0>().set(FUNC(ds2717_device::tc_w));
	m_pit->set_clk<1>(clock());
	m_pit->out_handler<1>().set(m_pit, FUNC(pit8253_device::write_clk2));
}


void ds2717_device::device_start()
{
	save_item(NAME(m_drq));
	save_item(NAME(m_intrq));
	save_item(NAME(m_tc));
}

void ds2717_device::device_reset()
{
	// 8" single density
	m_fdc->set_rate(500'000);
}


void ds2717_device::fdc_intrq_w(int state)
{
	m_intrq = state;
}

void ds2717_device::fdc_drq_w(int state)
{
	bool const old_int = m_drq && !m_tc;
	m_drq = state;
	update_int(old_int);
}

void ds2717_device::tc_w(int state)
{
	bool const old_int = m_drq && !m_tc;
	m_tc = state;
	m_fdc->tc_line_w(state);
	update_int(old_int);
}

void ds2717_device::update_int(bool old_int)
{
	bool const new_int = m_drq && !m_tc;
	if (new_int != old_int)
		m_int_cb(new_int);
}

void ds2717_device::count_byte()
{
	m_pit->write_clk0(1);
	m_pit->write_clk0(0);
}


uint8_t ds2717_device::read(offs_t offset)
{
	switch (offset & 7)
	{
	case 0:
		return m_fdc->msr_r();

	case 1:
		if (m_drq && !m_tc)
		{
			uint8_t const data = m_fdc->dma_r();
			if (!machine().side_effects_disabled())
				count_byte();
			return data;
		}
		return m_fdc->fifo_r();

	case 2:
		return m_intrq ? 0x01 : 0x00;

	case 4: case 5: case 6: case 7:
		return m_pit->read(offset & 3);

	default:
		return 0xff;
	}
}

void ds2717_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 7)
	{
	case 1:
		if (m_drq && !m_tc)
		{
			m_fdc->dma_w(data);
			count_byte();
		}
		else
			m_fdc->fifo_w(data);
		break;

	case 2:
		LOG("%s: control %02X\n", machine().describe_context(), data);
		break;

	case 4: case 5: case 6: case 7:
		m_pit->write(offset & 3, data);
		break;

	default:
		break;
	}
}
