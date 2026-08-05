// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Nokia MikroMikko 2 MMC186 emulation

*********************************************************************/

#include "emu.h"
#include "mmc186.h"

#define UPD765_TAG "upd765"

DEFINE_DEVICE_TYPE(NOKIA_MMC186, mmc186_device, "nokia_mmc186", "Nokia MikroMikko 2 MMC186")

mmc186_device::mmc186_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NOKIA_MMC186, tag, owner, clock),
	device_mikromikko2_expansion_bus_card_interface(mconfig, *this),
	m_dmac(*this, "am9517a"),
	m_sasi(*this, "sasi:7:scsicb"),
	m_fdc(*this, UPD765_TAG),
	m_ctrl(*this, "control"),
	m_irqs(*this, "irqs"),
	m_floppy(*this, UPD765_TAG ":%u:525qd", 0U),
	m_dma_ch(-1)
{
}

void mmc186_device::map(address_map &map)
{
	map(0x00, 0x1f).rw(m_dmac, FUNC(am9517a_device::read), FUNC(am9517a_device::write)).umask16(0x00ff);
	map(0x20, 0x21).mirror(0x10).rw(FUNC(mmc186_device::sasi_status_r), FUNC(mmc186_device::sasi_cmd_w)).umask16(0x00ff);
	map(0x22, 0x23).mirror(0x10).rw(FUNC(mmc186_device::sasi_data_r), FUNC(mmc186_device::sasi_data_w)).umask16(0x00ff);
	map(0x40, 0x41).mirror(0x10).r(m_fdc, FUNC(upd765a_device::msr_r)).umask16(0x00ff);
	map(0x42, 0x43).mirror(0x10).rw(m_fdc, FUNC(upd765a_device::fifo_r), FUNC(upd765a_device::fifo_w)).umask16(0x00ff);
	map(0x60, 0x6f).w(m_ctrl, FUNC(ls259_device::write_d0)).umask16(0x00ff);
	map(0x70, 0x70).mirror(0xe).w(FUNC(mmc186_device::dma_hi_w)).umask16(0x00ff);
}

void mmc186_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
}

static void mm2_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void mmc186_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set(FUNC(mmc186_device::int_w));

	AM9517A(config, m_dmac, XTAL(16'000'000)/4);
	m_dmac->out_hreq_callback().set(FUNC(mmc186_device::hold_w));
	m_dmac->out_eop_callback().set(FUNC(mmc186_device::eop_w));
	m_dmac->in_memr_callback().set(FUNC(mmc186_device::dmac_mem_r));
	m_dmac->out_memw_callback().set(FUNC(mmc186_device::dmac_mem_w));
	m_dmac->in_ior_callback<0>().set(FUNC(mmc186_device::sasi_data_r));
	m_dmac->out_iow_callback<0>().set(FUNC(mmc186_device::sasi_data_w));
	m_dmac->in_ior_callback<1>().set(m_fdc, FUNC(upd765_family_device::dma_r));
	m_dmac->out_iow_callback<1>().set(m_fdc, FUNC(upd765_family_device::dma_w));
	m_dmac->out_dack_callback<0>().set(FUNC(mmc186_device::dack0_w));
	m_dmac->out_dack_callback<1>().set(FUNC(mmc186_device::dack1_w));

	UPD765A(config, m_fdc, XTAL(16'000'000)/2, true, true);
	m_fdc->intrq_wr_callback().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_fdc->drq_wr_callback().set(m_dmac, FUNC(am9517a_device::dreq1_w));

	FLOPPY_CONNECTOR(config, UPD765_TAG ":0", mm2_floppies, "525qd", mmc186_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, UPD765_TAG ":1", mm2_floppies, nullptr, mmc186_device::floppy_formats).enable_sound(true);

	NSCSI_BUS(config, "sasi");
	NSCSI_CONNECTOR(config, "sasi:0", default_scsi_devices, "s1410");
	NSCSI_CONNECTOR(config, "sasi:7", default_scsi_devices, "scsicb", true)
		.option_add_internal("scsicb", NSCSI_CB)
		.machine_config([this](device_t* device) {
			downcast<nscsi_callback_device&>(*device).bsy_callback().set(*this, FUNC(mmc186_device::sasi_bsy_w));
			downcast<nscsi_callback_device&>(*device).req_callback().set(*this, FUNC(mmc186_device::sasi_req_w));
			downcast<nscsi_callback_device&>(*device).io_callback().set(*this, FUNC(mmc186_device::sasi_io_w));
		});

	LS259(config, m_ctrl);
	m_ctrl->q_out_cb<0>().set(FUNC(mmc186_device::sasi_select_w));
	m_ctrl->q_out_cb<1>().set(FUNC(mmc186_device::sasi_ie_w));
	m_ctrl->q_out_cb<3>().set(m_fdc, FUNC(upd765_family_device::reset_w));
	//m_ctrl->q_out_cb<5>().set(); -Mini/Std Select
	m_ctrl->q_out_cb<6>().set(FUNC(mmc186_device::motor_on_w));
	//m_ctrl->q_out_cb<7>().set(); Motor On (Std)

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("mm2_flop");
	SOFTWARE_LIST(config, "hdd_list").set_original("mm2_hdd");
}

void mmc186_device::device_start()
{
	m_bus->iospace().install_device(0xfa00, 0xfaff, *this, &mmc186_device::map);

	// state saving
	save_item(NAME(m_dma_ch));
	save_item(NAME(m_dma_hi));
	save_item(NAME(m_sasi_select));
	save_item(NAME(m_sasi_data));
	save_item(NAME(m_sasi_ie));
	save_item(NAME(m_sasi_irq));
	save_item(NAME(m_sasi_req));
}

void mmc186_device::sasi_select_w(int state)
{
	m_sasi_select = state;

	if (!m_sasi_select)
	{
		m_sasi->write(0);

		m_dmac->dreq0_w(CLEAR_LINE);

		m_sasi_irq = CLEAR_LINE;
		update_sasi_irq();
	}
}

uint8_t mmc186_device::sasi_status_r(offs_t offset)
{
	uint8_t data = 0;

	data |= m_sasi->bsy_r();
	data |= m_sasi->rst_r() << 1;
	data |= m_sasi->msg_r() << 2;
	data |= m_sasi->cd_r() << 3;
	data |= m_sasi->req_r() << 4;
	data |= m_sasi->io_r() << 5;
	data |= m_sasi->ack_r() << 7;

	return data;
}

void mmc186_device::sasi_cmd_w(offs_t offset, uint8_t data)
{
	if (data == 0x00) {
		m_sasi_irq = CLEAR_LINE;
		update_sasi_irq();
	}

	bool rst = BIT(data, 1);
	if (rst) {
		m_sasi_irq = ASSERT_LINE;
		update_sasi_irq();
	}

	m_sasi->sel_w(rst ? 0 : BIT(data, 0));
	m_sasi->rst_w(BIT(data, 1));
	m_sasi->atn_w(rst ? 0 : BIT(data, 2));
}

uint8_t mmc186_device::sasi_data_r(offs_t offset)
{
	uint8_t data = m_sasi->read();

	if (m_sasi_req && !m_sasi_irq)
	{
		m_sasi->ack_w(1);
	}

	return data;
}

void mmc186_device::sasi_data_w(offs_t offset, uint8_t data)
{
	m_sasi_data = data;

	if (m_sasi_select && !m_sasi->io_r())
	{
		m_sasi->write(data);
	}

	if (m_sasi_req && !m_sasi_irq)
	{
		m_sasi->ack_w(1);
	}
}

void mmc186_device::sasi_bsy_w(int state)
{
	if (state)
	{
		m_sasi->sel_w(0);
	}
}

void mmc186_device::sasi_req_w(int state)
{
	m_sasi_req = state;

	if (state)
	{
		if (!m_sasi->cd_r())
		{
			m_dmac->dreq0_w(ASSERT_LINE);
		}

		if (m_sasi->cd_r() && m_sasi->io_r())
		{
			m_sasi_irq = ASSERT_LINE;
			update_sasi_irq();
		}
	}
	else
	{
		m_sasi->ack_w(0);
	}
}

void mmc186_device::sasi_io_w(int state)
{
	if (state)
	{
		m_sasi->write(0);
	}
	else
	{
		m_sasi->write(m_sasi_data);
	}
}

void mmc186_device::bhlda_w(int state, int bcas)
{
	if (state == CLEAR_LINE || bcas == 3)
	{
		m_dmac->hack_w(state);
	}
}
