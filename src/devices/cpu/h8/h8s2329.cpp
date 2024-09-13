// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8s2329.cpp

    H8S-2329 family emulation
    Subdevice of h8s2319.cpp

***************************************************************************/

#include "emu.h"
#include "h8s2329.h"

DEFINE_DEVICE_TYPE(H8S2320, h8s2320_device, "h8s2320", "Hitachi H8S/2320")
DEFINE_DEVICE_TYPE(H8S2321, h8s2321_device, "h8s2321", "Hitachi H8S/2321")
DEFINE_DEVICE_TYPE(H8S2322, h8s2322_device, "h8s2322", "Hitachi H8S/2322")
DEFINE_DEVICE_TYPE(H8S2323, h8s2323_device, "h8s2323", "Hitachi H8S/2323")
DEFINE_DEVICE_TYPE(H8S2324, h8s2324_device, "h8s2324", "Hitachi H8S/2324")
DEFINE_DEVICE_TYPE(H8S2326, h8s2326_device, "h8s2326", "Hitachi H8S/2326")
DEFINE_DEVICE_TYPE(H8S2327, h8s2327_device, "h8s2327", "Hitachi H8S/2327")
DEFINE_DEVICE_TYPE(H8S2328, h8s2328_device, "h8s2328", "Hitachi H8S/2328")
DEFINE_DEVICE_TYPE(H8S2329, h8s2329_device, "h8s2329", "Hitachi H8S/2329")


h8s2321_device::h8s2321_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map_delegate, u32 rom_size, u32 ram_size) :
	h8s2319_device(mconfig, type, tag, owner, clock, map_delegate, rom_size, ram_size),
	m_port5(*this, "port5"),
	m_port6(*this, "port6")
{
}

h8s2321_device::h8s2321_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8s2321_device(mconfig, H8S2321, tag, owner, clock, address_map_constructor(FUNC(h8s2321_device::map_2321), this), 0, 0x1000)
{
}


h8s2320_device::h8s2320_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 rom_size, u32 ram_size) :
	h8s2321_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(h8s2320_device::map_2320), this), rom_size, ram_size),
	m_dma(*this, "dma"),
	m_dmac(*this, "dma:%u", 0),
	m_tend_cb(*this)
{
}

h8s2320_device::h8s2320_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8s2320_device(mconfig, H8S2320, tag, owner, clock, 0, 0x1000)
{
}


h8s2322_device::h8s2322_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8s2320_device(mconfig, H8S2322, tag, owner, clock, 0, 0x2000)
{
}

h8s2323_device::h8s2323_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8s2320_device(mconfig, H8S2323, tag, owner, clock, 0x8000, 0x2000)
{
}

h8s2324_device::h8s2324_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8s2320_device(mconfig, H8S2324, tag, owner, clock, 0, 0x8000)
{
}

h8s2326_device::h8s2326_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8s2320_device(mconfig, H8S2326, tag, owner, clock, 0x80000, 0x2000)
{
}

h8s2327_device::h8s2327_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8s2320_device(mconfig, H8S2327, tag, owner, clock, 0x20000, 0x2000)
{
}

h8s2328_device::h8s2328_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8s2320_device(mconfig, H8S2328, tag, owner, clock, 0x40000, 0x2000)
{
}

h8s2329_device::h8s2329_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8s2320_device(mconfig, H8S2329, tag, owner, clock, 0x60000, 0x8000)
{
}

void h8s2321_device::map_2321(address_map &map)
{
	h8s2319_device::map(map);

	map(0xfffeb4, 0xfffeb4).rw(m_port5, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));
	map(0xfffeb5, 0xfffeb5).rw(m_port6, FUNC(h8_port_device::ff_r), FUNC(h8_port_device::ddr_w));

	map(0xffff30, 0xffff35).rw(m_dtc, FUNC(h8_dtc_device::dtcer_r), FUNC(h8_dtc_device::dtcer_w));

	map(0xffff54, 0xffff54).r(m_port5, FUNC(h8_port_device::port_r));
	map(0xffff55, 0xffff55).r(m_port6, FUNC(h8_port_device::port_r));
	map(0xffff64, 0xffff64).rw(m_port5, FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));
	map(0xffff65, 0xffff65).rw(m_port6, FUNC(h8_port_device::dr_r), FUNC(h8_port_device::dr_w));

	map(0xffff88, 0xffff88).rw(m_sci[2], FUNC(h8_sci_device::smr_r), FUNC(h8_sci_device::smr_w));
	map(0xffff89, 0xffff89).rw(m_sci[2], FUNC(h8_sci_device::brr_r), FUNC(h8_sci_device::brr_w));
	map(0xffff8a, 0xffff8a).rw(m_sci[2], FUNC(h8_sci_device::scr_r), FUNC(h8_sci_device::scr_w));
	map(0xffff8b, 0xffff8b).rw(m_sci[2], FUNC(h8_sci_device::tdr_r), FUNC(h8_sci_device::tdr_w));
	map(0xffff8c, 0xffff8c).rw(m_sci[2], FUNC(h8_sci_device::ssr_r), FUNC(h8_sci_device::ssr_w));
	map(0xffff8d, 0xffff8d).r(m_sci[2], FUNC(h8_sci_device::rdr_r));
	map(0xffff8e, 0xffff8e).rw(m_sci[2], FUNC(h8_sci_device::scmr_r), FUNC(h8_sci_device::scmr_w));
}

void h8s2320_device::map_2320(address_map &map)
{
	map_2321(map);

	map(0xfffee0, 0xfffee1).rw(m_dmac[0], FUNC(h8s_dma_channel_device::marah_r), FUNC(h8s_dma_channel_device::marah_w));
	map(0xfffee2, 0xfffee3).rw(m_dmac[0], FUNC(h8s_dma_channel_device::maral_r), FUNC(h8s_dma_channel_device::maral_w));
	map(0xfffee4, 0xfffee5).rw(m_dmac[0], FUNC(h8s_dma_channel_device::ioara_r), FUNC(h8s_dma_channel_device::ioara_w));
	map(0xfffee6, 0xfffee7).rw(m_dmac[0], FUNC(h8s_dma_channel_device::etcra_r), FUNC(h8s_dma_channel_device::etcra_w));
	map(0xfffee8, 0xfffee9).rw(m_dmac[0], FUNC(h8s_dma_channel_device::marbh_r), FUNC(h8s_dma_channel_device::marbh_w));
	map(0xfffeea, 0xfffeeb).rw(m_dmac[0], FUNC(h8s_dma_channel_device::marbl_r), FUNC(h8s_dma_channel_device::marbl_w));
	map(0xfffeec, 0xfffeed).rw(m_dmac[0], FUNC(h8s_dma_channel_device::ioarb_r), FUNC(h8s_dma_channel_device::ioarb_w));
	map(0xfffeee, 0xfffeef).rw(m_dmac[0], FUNC(h8s_dma_channel_device::etcrb_r), FUNC(h8s_dma_channel_device::etcrb_w));
	map(0xfffef0, 0xfffef1).rw(m_dmac[1], FUNC(h8s_dma_channel_device::marah_r), FUNC(h8s_dma_channel_device::marah_w));
	map(0xfffef2, 0xfffef3).rw(m_dmac[1], FUNC(h8s_dma_channel_device::maral_r), FUNC(h8s_dma_channel_device::maral_w));
	map(0xfffef4, 0xfffef5).rw(m_dmac[1], FUNC(h8s_dma_channel_device::ioara_r), FUNC(h8s_dma_channel_device::ioara_w));
	map(0xfffef6, 0xfffef7).rw(m_dmac[1], FUNC(h8s_dma_channel_device::etcra_r), FUNC(h8s_dma_channel_device::etcra_w));
	map(0xfffef8, 0xfffef9).rw(m_dmac[1], FUNC(h8s_dma_channel_device::marbh_r), FUNC(h8s_dma_channel_device::marbh_w));
	map(0xfffefa, 0xfffefb).rw(m_dmac[1], FUNC(h8s_dma_channel_device::marbl_r), FUNC(h8s_dma_channel_device::marbl_w));
	map(0xfffefc, 0xfffefd).rw(m_dmac[1], FUNC(h8s_dma_channel_device::ioarb_r), FUNC(h8s_dma_channel_device::ioarb_w));
	map(0xfffefe, 0xfffeff).rw(m_dmac[1], FUNC(h8s_dma_channel_device::etcrb_r), FUNC(h8s_dma_channel_device::etcrb_w));
	map(0xffff00, 0xffff00).rw(m_dma, FUNC(h8s_dma_device::dmawer_r), FUNC(h8s_dma_device::dmawer_w));
	map(0xffff01, 0xffff01).rw(m_dma, FUNC(h8s_dma_device::dmatcr_r), FUNC(h8s_dma_device::dmatcr_w));
	map(0xffff02, 0xffff03).rw(m_dmac[0], FUNC(h8s_dma_channel_device::dmacr_r), FUNC(h8s_dma_channel_device::dmacr_w));
	map(0xffff04, 0xffff05).rw(m_dmac[1], FUNC(h8s_dma_channel_device::dmacr_r), FUNC(h8s_dma_channel_device::dmacr_w));
	map(0xffff06, 0xffff07).rw(m_dma, FUNC(h8s_dma_device::dmabcr_r), FUNC(h8s_dma_device::dmabcr_w));
}

void h8s2321_device::notify_standby(int state)
{
	h8s2319_device::notify_standby(state);
	m_sci[2]->notify_standby(state);
}

void h8s2321_device::device_add_mconfig(machine_config &config)
{
	h8s2319_device::device_add_mconfig(config);

	H8_PORT(config, m_port5, *this, h8_device::PORT_5, 0x00, 0xf0);
	H8_PORT(config, m_port6, *this, h8_device::PORT_6, 0x00, 0x00);
	H8_PORT(config.replace(), m_porta[0], *this, h8_device::PORT_A, 0x00, 0x00);

	H8_SCI(config, m_sci[2], 2, *this, m_intc, 88, 89, 90, 91);
}

void h8s2320_device::device_add_mconfig(machine_config &config)
{
	h8s2321_device::device_add_mconfig(config);

	H8S_DMA(config, m_dma, *this);
	H8S_DMA_CHANNEL(config, m_dmac[0], *this, m_dma, m_intc);
	H8S_DMA_CHANNEL(config, m_dmac[1], *this, m_dma, m_intc);
}

void h8s2320_device::execute_set_input(int inputnum, int state)
{
	// TEND and DREQ pins are not supported on H8S/2321
	if(inputnum == H8_INPUT_LINE_TEND0 || inputnum == H8_INPUT_LINE_TEND1)
		m_tend_cb[inputnum - H8_INPUT_LINE_TEND0](state);
	else if(inputnum == H8_INPUT_LINE_DREQ0 || inputnum == H8_INPUT_LINE_DREQ1)
		m_dma->set_input(inputnum, state);
	else
		h8s2319_device::execute_set_input(inputnum, state);
}

void h8s2320_device::device_start()
{
	h8s2321_device::device_start();
	m_dma_device = m_dma;
}
