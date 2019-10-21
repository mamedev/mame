// license:BSD-3-Clause
// copyright-holders:Carl

// TODO: multibus

#include "emu.h"
#include "isbc_208.h"

#include "formats/pc_dsk.h"


DEFINE_DEVICE_TYPE(ISBC_208, isbc_208_device, "isbc_208", "ISBC 208 Flexible Disk Driver Controller")

isbc_208_device::isbc_208_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISBC_208, tag, owner, clock),
	m_maincpu(*this, finder_base::DUMMY_TAG),
	m_dmac(*this, "dmac"),
	m_fdc(*this, "fdc"),
	m_out_irq_func(*this),
	m_maincpu_mem(nullptr)
{
}

FLOPPY_FORMATS_MEMBER( isbc_208_device::floppy_formats )
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static void isbc_208_floppies(device_slot_interface &device)
{
	device.option_add("8dd", FLOPPY_8_DSDD);
	device.option_add("525dd", FLOPPY_525_DD);
}

void isbc_208_device::device_add_mconfig(machine_config &config)
{
	AM9517A(config, m_dmac, 8_MHz_XTAL/4);
	m_dmac->out_hreq_callback().set(FUNC(isbc_208_device::hreq_w));
	m_dmac->out_eop_callback().set(FUNC(isbc_208_device::out_eop_w));
	m_dmac->in_memr_callback().set(FUNC(isbc_208_device::dma_read_byte));
	m_dmac->out_memw_callback().set(FUNC(isbc_208_device::dma_write_byte));
	m_dmac->in_ior_callback<0>().set(m_fdc, FUNC(i8272a_device::dma_r));
	m_dmac->out_iow_callback<0>().set(m_fdc, FUNC(i8272a_device::dma_w));

	I8272A(config, m_fdc, 8_MHz_XTAL, true);
	m_fdc->intrq_wr_callback().set(FUNC(isbc_208_device::irq_w));
	m_fdc->drq_wr_callback().set(m_dmac, FUNC(am9517a_device::dreq0_w));
	FLOPPY_CONNECTOR(config, "fdc:0", isbc_208_floppies, "525dd", isbc_208_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", isbc_208_floppies, "525dd", isbc_208_device::floppy_formats);
}

void isbc_208_device::map(address_map &map)
{
	map(0x00, 0x0f).rw(m_dmac, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x10, 0x11).m(m_fdc, FUNC(i8272a_device::map));
	map(0x12, 0x15).rw(FUNC(isbc_208_device::stat_r), FUNC(isbc_208_device::aux_w));
}


WRITE_LINE_MEMBER(isbc_208_device::out_eop_w)
{
	m_fdc->tc_w(state);
}

WRITE_LINE_MEMBER(isbc_208_device::irq_w)
{
	m_out_irq_func(state);
}

WRITE_LINE_MEMBER(isbc_208_device::hreq_w)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	/* Assert HLDA */
	m_dmac->hack_w(state);
}

READ8_MEMBER(isbc_208_device::dma_read_byte)
{
	return m_maincpu_mem->read_byte(((offset + (m_seg << 4)) & 0xfffff) | ((m_aux & 0xf0) << 16));
}


WRITE8_MEMBER(isbc_208_device::dma_write_byte)
{
	return m_maincpu_mem->write_byte(((offset + (m_seg << 4)) & 0xfffff) | ((m_aux & 0xf0) << 16), data);
}

READ8_MEMBER(isbc_208_device::stat_r)
{
	if(!offset)
		return m_fdc->get_irq() ? 1 : 0;
	return 0;
}

WRITE8_MEMBER( isbc_208_device::aux_w )
{
	switch(offset)
	{
		case 0:
			m_aux = data;
			m_fdc->subdevice<floppy_connector>("0")->get_device()->mon_w(!(data & 1));
			m_fdc->subdevice<floppy_connector>("1")->get_device()->mon_w(!(data & 2));
			break;
		case 1:
			m_fdc->soft_reset();
			m_dmac->reset();
			break;
		case 2:
			m_seg = (m_seg & 0xff00) | data;
			break;
		case 3:
			m_seg = (m_seg & 0xff) | (data << 8);
			break;
	}
}

void isbc_208_device::device_reset()
{
	m_aux = 0;
	m_seg = 0;
	// set from jumper all drives must be same type
	if(m_fdc->subdevice<floppy_connector>("0")->get_device()->get_form_factor() == floppy_image::FF_8)
		m_fdc->set_rate(500000);
}

void isbc_208_device::device_start()
{
	m_maincpu_mem = &m_maincpu->space(AS_PROGRAM);
	m_out_irq_func.resolve_safe();
}

