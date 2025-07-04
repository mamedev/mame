// license:BSD-3-Clause
// copyright-holders:R. Belmont, O. Galibert

#include "emu.h"

#include "cuda.h"
#include "macadb.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "bus/nubus/nubus.h"
#include "bus/nubus/cards.h"
#include "bus/rs232/rs232.h"
#include "cpu/powerpc/ppc.h"
#include "machine/6522via.h"
#include "machine/mv_sonora.h"
#include "machine/ncr53c90.h"
#include "machine/ram.h"
#include "machine/swim3.h"
#include "machine/timer.h"
#include "machine/z80scc.h"
#include "sound/awacs.h"

#include "softlist_dev.h"
#include "speaker.h"


namespace {

static constexpr auto IO_CLOCK = 31.3344_MHz_XTAL;
static constexpr auto ENET_CLOCK = 20_MHz_XTAL;
static constexpr auto SOUND_CLOCK = 45.1584_MHz_XTAL;

static constexpr u8 DMA2_IRQ_SND_IN             = 0x01;
static constexpr u8 DMA2_IRQ_SND_OUT            = 0x02;

class macpdm_state : public driver_device
{
public:
	macpdm_state(const machine_config &mconfig, device_type type, const char *tag);

	void macpdm(machine_config &config);

	void driver_init();
	virtual void driver_reset() override;

private:
	required_device<ppc_device> m_maincpu;
	required_device<via6522_device> m_via1;
	required_device<awacs_device> m_awacs;
	required_device<cuda_device> m_cuda;
	required_device<macadb_device> m_macadb;
	required_device<ram_device> m_ram;
	required_device<z80scc_device> m_scc;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<ncr53c94_device> m_ncr53c94;
	required_device<swim3_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<mac_video_sonora_device> m_video;

	floppy_image_device *m_cur_floppy = nullptr;

	uint32_t m_model_id = 0;
	uint64_t m_hmc_reg = 0, m_hmc_buffer = 0;
	uint8_t m_hmc_bit = 0;

	uint8_t m_irq_control = 0;

	uint8_t m_dma_irq_1, m_dma_irq_2;

	uint8_t m_via2_ier = 0, m_via2_ifr = 0, m_via2_sier = 0, m_via2_sifr = 0;

	uint64_t m_dma_scsi_buffer = 0;

	uint32_t m_dma_badr = 0, m_dma_floppy_adr = 0;
	uint16_t m_dma_floppy_byte_count = 0, m_dma_floppy_offset = 0;

	uint16_t m_dma_berr_en = 0, m_dma_berr_flag = 0;

	uint32_t m_dma_scsi_a_base_adr = 0, m_dma_scsi_b_base_adr = 0;
	uint32_t m_dma_scsi_a_cur_offset = 0, m_dma_scsi_b_cur_offset = 0;

	uint8_t m_dma_scsi_a_ctrl = 0, m_dma_scsi_b_ctrl = 0, m_dma_floppy_ctrl = 0;
	uint8_t m_dma_scsi_buffer_word_count = 0;

	uint8_t m_dma_scc_txa_ctrl = 0, m_dma_scc_rxa_ctrl = 0, m_dma_scc_txb_ctrl = 0, m_dma_scc_rxb_ctrl = 0;
	uint8_t m_dma_enet_rx_ctrl = 0, m_dma_enet_tx_ctrl = 0;

	bool m_dma_scsi_a_in_step = false, m_dma_floppy_in_step = false, m_floppy_drq = false;

	void pdm_map(address_map &map) ATTR_COLD;

	void nmi_irq(int state);
	void dma_irq(int state);
	void enet_irq(int state);
	void scc_irq(int state);
	void via1_irq(int state);

	void bus_err_irq(int state);
	void fdc_dma_irq(int state);
	void etx_dma_irq(int state);
	void erx_dma_irq(int state);
	void txa_dma_irq(int state);
	void rxa_dma_irq(int state);
	void txb_dma_irq(int state);
	void rxb_dma_irq(int state);

	void sndo_dma_irq(int state);
	void sndi_dma_irq(int state);

	void fdc_err_irq(int state);
	void etx_err_irq(int state);
	void erx_err_irq(int state);
	void txa_err_irq(int state);
	void rxa_err_irq(int state);
	void txb_err_irq(int state);
	void rxb_err_irq(int state);

	void scsi_err_irq(int state);
	void sndo_err_irq(int state);
	void sndi_err_irq(int state);

	void vblank_irq(int state);
	void slot2_irq_w(int state);
	void slot1_irq_w(int state);
	void slot0_irq_w(int state);

	void fdc_irq(int state);
	void fdc_drq(int state);
	void recalc_dma_irqs();
	void scsi_irq(int state);
	void scsi_drq(int state);

	void phases_w(uint8_t phases);
	void sel35_w(int sel35);
	void devsel_w(uint8_t devsel);
	void hdsel_w(int hdsel);

	uint8_t via1_in_a();
	uint8_t via1_in_b();
	void via1_out_a(uint8_t data);
	void via1_out_b(uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(via1_60_15_timer);
	void via1_out_cb2(int state);

	void cuda_reset_w(int state);

	uint8_t via1_r(offs_t offset);
	void via1_w(offs_t offset, uint8_t data);

	uint8_t via2_ier_r();
	void via2_ier_w(uint8_t data);
	uint8_t via2_ifr_r();
	uint8_t via2_sier_r();
	void via2_sier_w(uint8_t data);
	uint8_t via2_sifr_r();
	void via2_sifr_w(uint8_t data);

	uint8_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint8_t data);

	uint8_t scc_r(offs_t offset);
	void scc_w(offs_t offset, uint8_t data);

	uint8_t scsi_r(offs_t offset);
	void scsi_w(offs_t offset, uint8_t data);

	uint8_t hmc_r(offs_t offset);
	void hmc_w(offs_t offset, uint8_t data);

	void ram_size();

	uint32_t id_r();

	uint8_t diag_r(offs_t offset);

	uint8_t irq_control_r(offs_t offset);
	void irq_control_w(offs_t offset, uint8_t data);
	void irq_main_set(uint8_t mask, int state);
	void via2_irq_main_set(uint8_t mask, int state);
	void via2_irq_slot_set(uint8_t mask, int state);

	uint32_t dma_badr_r();
	void dma_badr_w(offs_t, uint32_t data, uint32_t mem_mask);
	uint16_t dma_berr_en_r();
	void dma_berr_en_w(offs_t, uint16_t data, uint16_t mem_mask);
	uint16_t dma_berr_flag_r();
	void dma_berr_flag_w(offs_t, uint16_t data, uint16_t mem_mask);

	void dma_scsi_a_step();
	uint32_t dma_scsi_a_base_adr_r();
	void dma_scsi_a_base_adr_w(offs_t, uint32_t data, uint32_t mem_mask);
	uint32_t dma_scsi_b_base_adr_r();
	void dma_scsi_b_base_adr_w(offs_t, uint32_t data, uint32_t mem_mask);
	uint8_t dma_scsi_a_ctrl_r();
	void dma_scsi_a_ctrl_w(uint8_t data);
	uint8_t dma_scsi_b_ctrl_r();
	void dma_scsi_b_ctrl_w(uint8_t data);
	uint32_t dma_scsi_a_cur_adr_r();
	uint32_t dma_scsi_b_cur_adr_r();

	void dma_floppy_step();
	uint8_t dma_floppy_ctrl_r();
	void dma_floppy_ctrl_w(uint8_t data);
	uint32_t dma_floppy_adr_r();
	void dma_floppy_adr_w(offs_t, uint32_t data, uint32_t mem_mask);
	uint16_t dma_floppy_byte_count_r();
	void dma_floppy_byte_count_w(offs_t, uint16_t data, uint16_t mem_mask);

	uint8_t dma_scc_txa_ctrl_r();
	void dma_scc_txa_ctrl_w(uint8_t data);
	uint8_t dma_scc_rxa_ctrl_r();
	void dma_scc_rxa_ctrl_w(uint8_t data);
	uint8_t dma_scc_txb_ctrl_r();
	void dma_scc_txb_ctrl_w(uint8_t data);
	uint8_t dma_scc_rxb_ctrl_r();
	void dma_scc_rxb_ctrl_w(uint8_t data);

	uint8_t dma_enet_rx_ctrl_r();
	void dma_enet_rx_ctrl_w(uint8_t data);
	uint8_t dma_enet_tx_ctrl_r();
	void dma_enet_tx_ctrl_w(uint8_t data);

	uint32_t sound_dma_output(offs_t offset);
	void sound_dma_input(offs_t offset, uint32_t value);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

macpdm_state::macpdm_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_via1(*this, "via6522_1"),
	m_awacs(*this, "awacs"),
	m_cuda(*this, "cuda"),
	m_macadb(*this, "macadb"),
	m_ram(*this, RAM_TAG),
	m_scc(*this, "scc"),
	m_scsibus(*this, "scsi"),
	m_ncr53c94(*this, "scsi:7:ncr53c94"),
	m_fdc(*this, "fdc"),
	m_floppy(*this, "fdc:%d", 0U),
	m_video(*this, "video"),
	m_dma_irq_1(0),
	m_dma_irq_2(0)
{
	m_cur_floppy = nullptr;
}

void macpdm_state::driver_init()
{
	m_maincpu->space().install_ram(0, m_ram->mask(), 0, m_ram->pointer());

	m_model_id = 0xa55a3011;
	// 7100 = a55a3012
	// 8100 = a55a3013

	save_item(NAME(m_hmc_reg));
	save_item(NAME(m_hmc_buffer));
	save_item(NAME(m_hmc_bit));

	save_item(NAME(m_via2_ier));
	save_item(NAME(m_via2_ifr));
	save_item(NAME(m_via2_sier));
	save_item(NAME(m_via2_sifr));

	save_item(NAME(m_irq_control));

	save_item(NAME(m_dma_badr));
	save_item(NAME(m_dma_berr_en));
	save_item(NAME(m_dma_berr_flag));
	save_item(NAME(m_dma_scsi_buffer));
	save_item(NAME(m_dma_scsi_buffer_word_count));
	save_item(NAME(m_dma_scsi_a_in_step));
	save_item(NAME(m_dma_scsi_a_base_adr));
	save_item(NAME(m_dma_scsi_b_base_adr));
	save_item(NAME(m_dma_scsi_a_ctrl));
	save_item(NAME(m_dma_scsi_b_ctrl));
	save_item(NAME(m_dma_scsi_a_cur_offset));
	save_item(NAME(m_dma_scsi_b_cur_offset));
	save_item(NAME(m_dma_floppy_ctrl));
	save_item(NAME(m_dma_floppy_in_step));
	save_item(NAME(m_dma_scc_txa_ctrl));
	save_item(NAME(m_dma_scc_rxa_ctrl));
	save_item(NAME(m_dma_scc_txb_ctrl));
	save_item(NAME(m_dma_scc_rxb_ctrl));
	save_item(NAME(m_dma_enet_rx_ctrl));
	save_item(NAME(m_dma_enet_tx_ctrl));

	save_item(NAME(m_dma_floppy_adr));
	save_item(NAME(m_dma_floppy_offset));
	save_item(NAME(m_dma_floppy_byte_count));
	save_item(NAME(m_floppy_drq));
}

void macpdm_state::driver_reset()
{
	m_hmc_reg = 0;
	m_hmc_buffer = 0;
	m_hmc_bit = 0;

	m_via2_ier = 0x00;
	m_via2_ifr = 0x00;
	m_via2_sier = 0x00;
	m_via2_sifr = 0x7f;

	m_irq_control = 0;

	m_dma_badr = 0;
	m_dma_berr_en = 0;
	m_dma_berr_flag = 0;
	m_dma_scsi_buffer = 0;
	m_dma_scsi_buffer_word_count = 0;
	m_dma_scsi_a_in_step = false;
	m_dma_scsi_a_base_adr = 0;
	m_dma_scsi_b_base_adr = 0;
	m_dma_scsi_a_ctrl = 0;
	m_dma_scsi_b_ctrl = 0;
	m_dma_scsi_a_cur_offset = 0;
	m_dma_scsi_b_cur_offset = 0;
	m_dma_floppy_ctrl = 0;
	m_dma_scc_txa_ctrl = 0;
	m_dma_scc_rxa_ctrl = 0;
	m_dma_scc_txb_ctrl = 0;
	m_dma_scc_rxb_ctrl = 0;
	m_dma_enet_rx_ctrl = 0;
	m_dma_enet_tx_ctrl = 0;

	m_dma_floppy_adr = 0x15000;
	m_dma_floppy_offset = 0;
	m_dma_floppy_byte_count = 0;
	m_floppy_drq = false;

	m_video->set_vram_base((const u64 *)m_ram->pointer());
	m_video->set_vram_offset(0);

	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

uint8_t macpdm_state::irq_control_r(offs_t offset)
{
	switch (offset)
	{
		case 0x0:
			return m_irq_control;

		case 0x8:
			return m_dma_irq_1;

		case 0xa:
			return m_dma_irq_2;
	}

	return 0;
}

void macpdm_state::irq_control_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
		if((m_irq_control ^ data) & 0x40) {
			m_irq_control = (m_irq_control & ~0xc0) | (data & 0x40);
			m_maincpu->set_input_line(PPC_IRQ, CLEAR_LINE);
		}
		if((data & 0xc0) == 0xc0 && (m_irq_control & 0x80)) {
			m_irq_control &= 0x7f;
			m_maincpu->set_input_line(PPC_IRQ, CLEAR_LINE);
		}
	}
}

void macpdm_state::irq_main_set(uint8_t mask, int state)
{
	if(((m_irq_control & mask) != 0) == state)
		return;

	m_irq_control ^= mask;

	if(m_irq_control & 0x40) {
		m_irq_control |= 0x80;
		m_maincpu->set_input_line(PPC_IRQ, ASSERT_LINE);
	} else {
		if(m_irq_control & 0x3f) {
			m_irq_control |= 0x80;
			m_maincpu->set_input_line(PPC_IRQ, ASSERT_LINE);
		} else {
			m_irq_control &= 0x7f;
			m_maincpu->set_input_line(PPC_IRQ, CLEAR_LINE);
		}
	}

	//  logerror("irq control %02x\n", m_irq_control);
}

void macpdm_state::via2_irq_main_set(uint8_t mask, int state)
{
	if(((m_via2_ifr & mask) != 0) == state)
		return;

	m_via2_ifr ^= mask;
	//  logerror("via2 main %02x / %02x -> %02x\n", m_via2_ifr, m_via2_ier, m_via2_ifr & m_via2_ier);

	irq_main_set(0x02, (m_via2_ifr & m_via2_ier) != 0);
}

void macpdm_state::via2_irq_slot_set(uint8_t mask, int state)
{
	if(((m_via2_sifr & mask) == 0) == state)
		return;

	m_via2_sifr ^= mask;
	via2_irq_main_set(0x02, ((~m_via2_sifr) & m_via2_sier) != 0);
}




// bit 7 = out - scc wait/request
// bit 5 = out - head select, unconnected
// bit 3 = ?   - sync modem (?)
uint8_t macpdm_state::via1_in_a()
{
	return 0x00;
}

void macpdm_state::via1_out_a(uint8_t data)
{
}

// bit 7 = ?   - snd res (?)
// bit 5 = out - sys sess/tip
// bit 4 = out - via full/byte ack
// bit 3 = in  - xcvr sess/treq

uint8_t macpdm_state::via1_in_b()
{
	uint8_t val = 0;

	val |= m_cuda->get_treq() << 3;

	return val;
}

void macpdm_state::via1_out_b(uint8_t data)
{
	m_cuda->set_byteack(BIT(data, 4));
	m_cuda->set_tip(BIT(data, 5));
}

TIMER_DEVICE_CALLBACK_MEMBER(macpdm_state::via1_60_15_timer)
{
	m_via1->write_ca1(1);
	m_via1->write_ca1(0);
}

void macpdm_state::via1_out_cb2(int state)
{
	m_cuda->set_via_data(state & 1);
}


void macpdm_state::cuda_reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
	m_maincpu->set_input_line(INPUT_LINE_RESET, state);
}


uint8_t macpdm_state::via1_r(offs_t offset)
{
	return m_via1->read(offset >> 9);
}

void macpdm_state::via1_w(offs_t offset, uint8_t data)
{
	m_via1->write(offset >> 9, data);
}


uint8_t macpdm_state::via2_ier_r()
{
	return m_via2_ier;
}

void macpdm_state::via2_ier_w(uint8_t data)
{
	if(data & 0x80)
		m_via2_ier |= data & 0x3b;
	else
		m_via2_ier &= ~data;

	logerror("via2 ier %s %s %s %s\n",
			 m_via2_ier & 0x20 ? "fdc" : "-",
			 m_via2_ier & 0x10 ? "sound" : "-",
			 m_via2_ier & 0x08 ? "scsi" : "-",
			 m_via2_ier & 0x02 ? "slot" : "-",
			 m_via2_ier & 0x01 ? "scsidrq" : "-");

	irq_main_set(0x02, (m_via2_ifr & m_via2_ier) != 0);
}

uint8_t macpdm_state::via2_ifr_r()
{
	return m_via2_ifr;
}

uint8_t macpdm_state::via2_sier_r()
{
	return m_via2_sier;
}

void macpdm_state::via2_sier_w(uint8_t data)
{
	if(data & 0x80)
		m_via2_sier |= data & 0x78;
	else
		m_via2_sier &= ~data;

	logerror("via2 sier %s %s %s %s\n",
			 m_via2_sier & 0x40 ? "vbl" : "-",
			 m_via2_sier & 0x20 ? "slot2" : "-",
			 m_via2_sier & 0x10 ? "slot1" : "-",
			 m_via2_sier & 0x08 ? "slot0" : "-");

	via2_irq_main_set(0x02, ((~m_via2_sifr) & m_via2_sier) != 0);
}

uint8_t macpdm_state::via2_sifr_r()
{
	return m_via2_sifr;
}

void macpdm_state::via2_sifr_w(uint8_t data)
{
	if(data & (~m_via2_sifr) & 0x40) {
		m_via2_sifr |= 0x40;
		via2_irq_main_set(0x02, ((~m_via2_sifr) & m_via2_sier) != 0);
	}
}


uint8_t macpdm_state::scc_r(offs_t offset)
{
	return m_scc->dc_ab_r(offset >> 1);
}

void macpdm_state::scc_w(offs_t offset, uint8_t data)
{
	m_scc->dc_ab_w(offset, data);
}

uint8_t macpdm_state::fdc_r(offs_t offset)
{
	return m_fdc->read(offset >> 9);
}

void macpdm_state::fdc_w(offs_t offset, uint8_t data)
{
	m_fdc->write(offset >> 9, data);
}

uint8_t macpdm_state::scsi_r(offs_t offset)
{
	return m_ncr53c94->read(offset >> 4);
}

void macpdm_state::scsi_w(offs_t offset, uint8_t data)
{
	m_ncr53c94->write(offset >> 4, data);
}

uint8_t macpdm_state::hmc_r(offs_t offset)
{
	const uint8_t rv = (m_hmc_reg >> m_hmc_bit) & 1;
	m_hmc_bit++;
	return rv;
}

void macpdm_state::hmc_w(offs_t offset, uint8_t data)
{
	if(offset & 8)
		m_hmc_bit = 0;
	else {
		if(data & 1)
			m_hmc_buffer |= u64(1) << m_hmc_bit;
		else
			m_hmc_buffer &= ~(u64(1) << m_hmc_bit);
		m_hmc_bit ++;
		if(m_hmc_bit == 35) {
			m_hmc_reg = m_hmc_buffer & ~3; // csiz is readonly, we pretend there isn't a l2 cache
			m_video->set_vram_offset(m_hmc_reg & 0x200000000 ? 0 : 0x100000);
			ram_size();
			logerror("HMC l2=%c%c%c%c%c vbase=%c%s mbram=%cM size=%x%s romd=%d refresh=%02x w=%c%c%c%c ras=%d%d%d%d\n",
					 m_hmc_reg & 0x008000000 ? '+' : '-',      // l2_en
					 m_hmc_reg & 0x400000000 ? '3' : '2',      // l2_init
					 m_hmc_reg & 0x004000000 ? '1' : '2',      // l2_brst
					 m_hmc_reg & 0x010000000 ? 'I' : 'U',      // l2_inst
					 m_hmc_reg & 0x002000000 ? 'w' : '.',      // l2romw
					 m_hmc_reg & 0x200000000 ? '1' : '0',      // vbase
					 m_hmc_reg & 0x100000000 ? " vtst" : "",   // vtst
					 m_hmc_reg & 0x080000000 ? '8' : '4',      // mb_ram
					 (uint32_t)((m_hmc_reg >> 29) & 3),        // size
					 m_hmc_reg & 0x001000000 ? " nblrom" : "", // nblrom
					 (uint32_t)(12 - 2*((m_hmc_reg >> 22) & 3)), // romd
					 (uint32_t)((m_hmc_reg >> 16) & 0x3f),     // rfsh
					 m_hmc_reg & 0x000000008 ? '3' : '2',      // winit
					 m_hmc_reg & 0x000000004 ? '3' : '2',      // wbrst
					 m_hmc_reg & 0x000008000 ? '1' : '2',      // wcasp
					 m_hmc_reg & 0x000004000 ? '1' : '2',      // wcasd
					 (uint32_t)(3 - ((m_hmc_reg >> 12) & 3)),  // rdac
					 (uint32_t)(6 - ((m_hmc_reg >> 8) & 3)),   // rasd
					 (uint32_t)(5 - ((m_hmc_reg >> 6) & 3)),   // rasp
					 (uint32_t)(4 - ((m_hmc_reg >> 4) & 3)));  // rcasd
		}
	}
}

/*
    PDM uses a variant on the V8/Sonora style memory controller.
    - Motherboard RAM can be 4 or 8 MiB
    - The hardware officially limits SIMMs to 2, 8, or 32 MiB, and SIMMs must be installed in pairs
    - In reality, 128 MiB SIMMs will also work.
    - 6100 has only 2 SIMM slots, so valid sizes are 8MiB (no SIMMs), 12MiB (2 MiB SIMM x2),
      24MiB (8 MiB SIMM x2), 72MiB (32 MiB SIMM x2), and 264MiB (128 MiB SIMM x2)
    - 7100 has 4 SIMM slots, allowing RAM up to 520MiB (128 MiB SIMM x4)
    - 8100 has 8 SIMM slots, which add valid sizes up to 264 MiB (32 MiB SIMM x8)
*/
void macpdm_state::ram_size(){
	static const uint32_t sizes[4] = { 128*1024*1024, 2*1024*1024, 8*1024*1024, 32*1024*1024 };
	const u8 config = (m_hmc_reg >> 29) & 3;        // 0 = 128MiB, 1 = 2MiB, 2 = 8MiB, 3 = 32MiB
	const u8 mb_size = (m_hmc_reg & 0x00800000) ? 1 : 0;
	address_space &space = m_maincpu->space(AS_PROGRAM);
	const u32 total_ram = m_ram->size();
	const u32 mb_ram_size = (mb_size ? 8*1024*1024 : 4*1024*1024);

	// SIMMs must be in identical pairs, so any leftover RAM after the motherboard 8MiB is
	// the number of slots times the SIMM size.
	const u32 simm_size = (total_ram - mb_ram_size) / 2;

	u8 *mb_ram = (u8 *)m_ram->pointer();
	u8 *ram_a = mb_ram + mb_ram_size;
	u8 *ram_b = ram_a + simm_size;

	// unmap the first GB of address space (reserved for RAM)
	space.unmap_readwrite(0x00000000, 0x3fffffff);

	// map the motherboard 8MB
	space.install_ram(0x00000000, 0x007fffff, 0, (void *)mb_ram);

	// install RAM A
	if (simm_size > 0)
	{
		if (simm_size <= 8*1024*1024)
		{
			space.install_ram(mb_ram_size, mb_ram_size + simm_size - 1, 0, (void *)ram_a);
		}
		else
		{
			space.install_ram(mb_ram_size, sizes[config] - 1, 0, (void *)(ram_a + mb_ram_size));
		}

		// install a complete image of RAM A in the slot above the top of memory (which is actually where the ROM code looks for it)
		const u32 alias_base = (sizes[config] * 2);
		space.install_ram(alias_base, alias_base + simm_size - 1, 0, (void *)ram_a);

		// install RAM B
		u32 b_base = sizes[config];

		if (simm_size < (128*1024*1024))
		{
			b_base += mb_ram_size;
		}

		space.install_ram(b_base, b_base + simm_size - 1, 0, (void *)ram_b);
	}
}

uint8_t macpdm_state::diag_r(offs_t offset)
{
	// returning 0 at address 0 gives the 'car crash' sound after the boot bong
	logerror("diag_r %x\n", offset);
	return offset ? 0 : 1;
}

void macpdm_state::phases_w(uint8_t phases)
{
	if(m_cur_floppy)
		m_cur_floppy->seek_phase_w(phases);
}

void macpdm_state::sel35_w(int sel35)
{
	logerror("fdc mac sel35 %d\n", sel35);
}

void macpdm_state::devsel_w(uint8_t devsel)
{
	if(devsel == 1)
		m_cur_floppy = m_floppy[0]->get_device();
	else if(devsel == 2)
		m_cur_floppy = m_floppy[1]->get_device();
	else
		m_cur_floppy = nullptr;
	m_fdc->set_floppy(m_cur_floppy);
}

void macpdm_state::hdsel_w(int hdsel)
{
	if(m_cur_floppy)
		m_cur_floppy->ss_w(hdsel);
}

uint32_t macpdm_state::id_r()
{
	return m_model_id;
}

void macpdm_state::scc_irq(int state)
{
	logerror("scc irq %d\n", state);
}

void macpdm_state::via1_irq(int state)
{
	irq_main_set(0x01, state);
}

void macpdm_state::nmi_irq(int state)
{
	irq_main_set(0x20, state);
}

void macpdm_state::recalc_dma_irqs()
{
	if ((m_dma_irq_1 | m_dma_irq_2) != 0)
	{
		irq_main_set(0x10, ASSERT_LINE);
	}
	else
	{
		irq_main_set(0x10, CLEAR_LINE);
	}
}

void macpdm_state::vblank_irq(int state)
{
	via2_irq_slot_set(0x40, state);
}

void macpdm_state::slot2_irq_w(int state)
{
	via2_irq_slot_set(0x20, state);
}

void macpdm_state::slot1_irq_w(int state)
{
	via2_irq_slot_set(0x10, state);
}

void macpdm_state::slot0_irq_w(int state)
{
	via2_irq_slot_set(0x08, state);
}

void macpdm_state::sndo_dma_irq(int state)
{
	m_dma_irq_2 &= ~DMA2_IRQ_SND_OUT;
	m_dma_irq_2 |= state ? DMA2_IRQ_SND_OUT : 0;
	recalc_dma_irqs();
}

void macpdm_state::sndi_dma_irq(int state)
{
	m_dma_irq_2 &= ~DMA2_IRQ_SND_IN;
	m_dma_irq_2 |= state ? DMA2_IRQ_SND_IN : 0;
	recalc_dma_irqs();
}

uint32_t macpdm_state::dma_badr_r()
{
	return m_dma_badr;
}

void macpdm_state::dma_badr_w(offs_t, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dma_badr);
	m_dma_badr &= 0xfffc0000;

	logerror("dma base address %08x\n", m_dma_badr);

	m_dma_floppy_adr = (m_dma_badr | 0x10000) + (m_dma_floppy_adr & 0xffff);
}

uint16_t macpdm_state::dma_berr_en_r()
{
	return m_dma_berr_en;
}

void macpdm_state::dma_berr_en_w(offs_t, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dma_berr_en);
	logerror("dma bus error enable %08x\n", m_dma_berr_en);
}

uint16_t macpdm_state::dma_berr_flag_r()
{
	return m_dma_berr_flag;
}

void macpdm_state::dma_berr_flag_w(offs_t, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dma_berr_flag);
	logerror("dma bus error flag %08x\n", m_dma_berr_flag);
}


// SCSI management

void macpdm_state::dma_scsi_a_step()
{
	m_dma_scsi_a_in_step = true;

	if(m_dma_scsi_a_ctrl & 0x40) {
		while(m_via2_ifr & 0x01) {
			if(m_dma_scsi_buffer_word_count == 0) {
				m_dma_scsi_buffer_word_count = 4;
				m_dma_scsi_buffer = m_maincpu->space().read_qword(m_dma_scsi_a_base_adr + m_dma_scsi_a_cur_offset);
				m_dma_scsi_a_cur_offset += 8;
			}
			m_dma_scsi_buffer_word_count --;
			m_ncr53c94->dma16_swap_w(m_dma_scsi_buffer >> (16*m_dma_scsi_buffer_word_count));
		}

	} else {
		while(m_via2_ifr & 0x01) {
			uint16_t w = m_ncr53c94->dma16_swap_r();
			m_dma_scsi_buffer = (m_dma_scsi_buffer & ~(u64(0xffff) << (48 - 16*m_dma_scsi_buffer_word_count))) | (u64(w) << (48 - 16*m_dma_scsi_buffer_word_count));
			m_dma_scsi_buffer_word_count ++;
			if(m_dma_scsi_buffer_word_count == 4) {
				m_dma_scsi_buffer_word_count = 0;
				m_maincpu->space().write_qword(m_dma_scsi_a_base_adr + m_dma_scsi_a_cur_offset, m_dma_scsi_buffer);
				m_dma_scsi_a_cur_offset += 8;
			}
		}
	}

	m_dma_scsi_a_in_step = false;
}

void macpdm_state::scsi_irq(int state)
{
	via2_irq_main_set(0x08, state);
}

void macpdm_state::scsi_drq(int state)
{
	via2_irq_main_set(0x01, state);
	if((m_dma_scsi_a_ctrl & 0x02) && (m_via2_ifr & 0x01) && !m_dma_scsi_a_in_step)
		dma_scsi_a_step();
}

uint32_t macpdm_state::dma_scsi_a_base_adr_r()
{
	return m_dma_scsi_a_base_adr;
}

void macpdm_state::dma_scsi_a_base_adr_w(offs_t, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dma_scsi_a_base_adr);
	m_dma_scsi_a_base_adr &= ~7;
	m_dma_scsi_a_cur_offset = 0;
	m_dma_scsi_buffer_word_count = 0;
	logerror("dma_scsi_a_base_adr_w %08x\n", m_dma_scsi_a_base_adr);
}

uint32_t macpdm_state::dma_scsi_b_base_adr_r()
{
	return m_dma_scsi_b_base_adr;
}

void macpdm_state::dma_scsi_b_base_adr_w(offs_t, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dma_scsi_b_base_adr);
	m_dma_scsi_b_base_adr &= ~7;
	m_dma_scsi_a_cur_offset = 0;
	logerror("dma_scsi_b_base_adr_w %08x\n", m_dma_scsi_b_base_adr);
}

uint8_t macpdm_state::dma_scsi_a_ctrl_r()
{
	return m_dma_scsi_a_ctrl;
}

void macpdm_state::dma_scsi_a_ctrl_w(uint8_t data)
{
	m_dma_scsi_a_ctrl = data & 0x42;
	if(data & 1) {
		m_dma_scsi_a_ctrl &= 0x40;
		m_dma_scsi_a_cur_offset = 0;
		m_dma_scsi_buffer_word_count = 0;
	}
	if(data & 0x10) {
		while(m_via2_ifr & 0x01) {
			uint16_t w = m_ncr53c94->dma16_swap_r();
			m_dma_scsi_buffer = (m_dma_scsi_buffer & ~(u64(0xffff) << (48 - 16*m_dma_scsi_buffer_word_count))) | (u64(w) << (48 - 16*m_dma_scsi_buffer_word_count));
			m_dma_scsi_buffer_word_count ++;
			if(m_dma_scsi_buffer_word_count == 4) {
				m_dma_scsi_buffer_word_count = 0;
				m_maincpu->space().write_qword(m_dma_scsi_a_base_adr + m_dma_scsi_a_cur_offset, m_dma_scsi_buffer);
				m_dma_scsi_a_cur_offset += 8;
			}
		}
		if(m_dma_scsi_buffer_word_count) {
			m_maincpu->space().write_qword(m_dma_scsi_a_base_adr + m_dma_scsi_a_cur_offset, m_dma_scsi_buffer);
			m_dma_scsi_buffer_word_count = 0;
		}
	}

	if((m_dma_scsi_a_ctrl & 0x02) && (m_via2_ifr & 0x01) && !m_dma_scsi_a_in_step)
		dma_scsi_a_step();

	logerror("dma_scsi_a_ctrl_w %02x\n", m_dma_scsi_a_ctrl);
}

uint8_t macpdm_state::dma_scsi_b_ctrl_r()
{
	return m_dma_scsi_b_ctrl;
}

void macpdm_state::dma_scsi_b_ctrl_w(uint8_t data)
{
	// Channel B is not actually connected to anything
	m_dma_scsi_b_ctrl = data & 0x42;
	if(data & 1) {
		m_dma_scsi_b_ctrl &= 0x40;
		m_dma_scsi_b_cur_offset = 0;
	}
	logerror("dma_scsi_b_ctrl_w %02x\n", m_dma_scsi_b_ctrl);
}

uint32_t macpdm_state::dma_scsi_a_cur_adr_r()
{
	return m_dma_scsi_a_base_adr + m_dma_scsi_a_cur_offset;
}

uint32_t macpdm_state::dma_scsi_b_cur_adr_r()
{
	return m_dma_scsi_b_base_adr + m_dma_scsi_b_cur_offset;
}


// Floppy management

uint8_t macpdm_state::dma_floppy_ctrl_r()
{
	return m_dma_floppy_ctrl;
}

void macpdm_state::dma_floppy_ctrl_w(uint8_t data)
{
	m_dma_floppy_ctrl = (m_dma_floppy_ctrl & 0x80) | (data & 0x4a);
	if(data & 0x01) {
		m_dma_floppy_ctrl &= 0x7f;
		m_dma_floppy_offset = 0;
	}

	if(data & 0x80)
		m_dma_floppy_ctrl &= 0x7f;

	logerror("dma floppy ctrl %02x\n", m_dma_floppy_ctrl);
}

uint16_t macpdm_state::dma_floppy_byte_count_r()
{
	return m_dma_floppy_byte_count;
}

void macpdm_state::dma_floppy_byte_count_w(offs_t, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dma_floppy_byte_count);
	logerror("dma floppy count %04x\n", m_dma_floppy_byte_count);
}

uint32_t macpdm_state::dma_floppy_adr_r()
{
	return m_dma_floppy_adr;
}

void macpdm_state::dma_floppy_adr_w(offs_t, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dma_floppy_adr);
	m_dma_floppy_adr = (m_dma_badr | 0x10000) + (m_dma_floppy_adr & 0xffff);
	m_dma_floppy_offset = 0;
	logerror("dma floppy adr %08x\n", m_dma_floppy_adr);
}

void macpdm_state::fdc_irq(int state)
{
	via2_irq_main_set(0x20, state);
}

void macpdm_state::dma_floppy_step()
{
	m_dma_floppy_in_step = true;

	if(m_dma_floppy_ctrl & 0x40) {
		fatalerror("floppy dma write\n");

	} else {
		while(m_floppy_drq) {
			u8 r = m_fdc->dma_r();
			m_maincpu->space().write_byte(m_dma_floppy_adr + m_dma_floppy_offset, r);
			m_dma_floppy_offset ++;
			m_dma_floppy_byte_count --;
			//          logerror("dma_w %03x, %02x\n", m_dma_floppy_offset, r);
			if(m_dma_floppy_byte_count == 0) {
				m_dma_floppy_ctrl &= ~0x02;
				m_dma_floppy_ctrl |= 0x80;
				logerror("dma floppy done\n");
				// todo irq dma
				break;
			}
		}
	}

	m_dma_floppy_in_step = false;
}

void macpdm_state::fdc_drq(int state)
{
	m_floppy_drq = state;
	if((m_dma_floppy_ctrl & 0x02) && m_floppy_drq && !m_dma_floppy_in_step)
		dma_floppy_step();
}



// SCC management

uint8_t macpdm_state::dma_scc_txa_ctrl_r()
{
	return m_dma_scc_txa_ctrl;
}

void macpdm_state::dma_scc_txa_ctrl_w(uint8_t data)
{
	m_dma_scc_txa_ctrl = data;
	logerror("dma_scc_txa_ctrl_w %02x\n", m_dma_scc_txa_ctrl);
}

uint8_t macpdm_state::dma_scc_rxa_ctrl_r()
{
	return m_dma_scc_rxa_ctrl;
}

void macpdm_state::dma_scc_rxa_ctrl_w(uint8_t data)
{
	m_dma_scc_rxa_ctrl = data;
	logerror("dma_scc_rxa_ctrl_w %02x\n", m_dma_scc_rxa_ctrl);
}

uint8_t macpdm_state::dma_scc_txb_ctrl_r()
{
	return m_dma_scc_txb_ctrl;
}

void macpdm_state::dma_scc_txb_ctrl_w(uint8_t data)
{
	m_dma_scc_txb_ctrl = data;
	logerror("dma_scc_txb_ctrl_w %02x\n", m_dma_scc_txb_ctrl);
}

uint8_t macpdm_state::dma_scc_rxb_ctrl_r()
{
	return m_dma_scc_rxb_ctrl;
}

void macpdm_state::dma_scc_rxb_ctrl_w(uint8_t data)
{
	m_dma_scc_rxb_ctrl = data;
	logerror("dma_scc_rxb_ctrl_w %02x\n", m_dma_scc_rxb_ctrl);
}

uint8_t macpdm_state::dma_enet_rx_ctrl_r()
{
	return m_dma_enet_rx_ctrl;
}

void macpdm_state::dma_enet_rx_ctrl_w(uint8_t data)
{
	m_dma_enet_rx_ctrl = data;
	logerror("dma_enet_rx_ctrl_w %02x\n", m_dma_enet_rx_ctrl);
}

uint8_t macpdm_state::dma_enet_tx_ctrl_r()
{
	return m_dma_enet_tx_ctrl;
}

void macpdm_state::dma_enet_tx_ctrl_w(uint8_t data)
{
	m_dma_enet_tx_ctrl = data;
	logerror("dma_enet_tx_ctrl_w %02x\n", m_dma_enet_tx_ctrl);
}

uint32_t macpdm_state::sound_dma_output(offs_t offset)
{
	offs_t adr = m_dma_badr + (offset & 0x10000 ? 0x12000 : 0x10000) + 4*(offset & 0x7ff);
	return m_maincpu->space().read_dword(adr);
}

void macpdm_state::sound_dma_input(offs_t offset, uint32_t value)
{
	offs_t adr = m_dma_badr + (offset & 0x10000 ? 0x0e000 : 0x0c000) + 4*(offset & 0x7ff);
	m_maincpu->space().write_dword(adr, value);
}


void macpdm_state::pdm_map(address_map &map)
{
	map(0x40000000, 0x403fffff).rom().region("bootrom", 0).mirror(0x0fc00000);

	map(0x50f00000, 0x50f00000).rw(FUNC(macpdm_state::via1_r), FUNC(macpdm_state::via1_w)).select(0x1e00);
	map(0x50f04000, 0x50f04000).rw(FUNC(macpdm_state::scc_r), FUNC(macpdm_state::scc_w)).select(0x000e);
	// 50f08000 = ethernet ID PROM
	// 50f0a000 = MACE ethernet controller
	map(0x50f10000, 0x50f10000).rw(FUNC(macpdm_state::scsi_r), FUNC(macpdm_state::scsi_w)).select(0xf0);
	map(0x50f10100, 0x50f10101).rw(m_ncr53c94, FUNC(ncr53c94_device::dma16_swap_r), FUNC(ncr53c94_device::dma16_swap_w));
	map(0x50f14000, 0x50f1401f).rw(m_awacs, FUNC(awacs_device::read), FUNC(awacs_device::write));
	map(0x50f16000, 0x50f16000).rw(FUNC(macpdm_state::fdc_r), FUNC(macpdm_state::fdc_w)).select(0x1e00);

	map(0x50f24000, 0x50f24003).rw(m_video, FUNC(mac_video_sonora_device::dac_r), FUNC(mac_video_sonora_device::dac_w));

	map(0x50f26002, 0x50f26002).rw(FUNC(macpdm_state::via2_sifr_r), FUNC(macpdm_state::via2_sifr_w)).mirror(0x1fe0);
	map(0x50f26003, 0x50f26003).r(FUNC(macpdm_state::via2_ifr_r)).mirror(0x1fe0);
	map(0x50f26012, 0x50f26012).rw(FUNC(macpdm_state::via2_sier_r), FUNC(macpdm_state::via2_sier_w)).mirror(0x1fe0);
	map(0x50f26013, 0x50f26013).rw(FUNC(macpdm_state::via2_ier_r), FUNC(macpdm_state::via2_ier_w)).mirror(0x1fe0);

	map(0x50f28000, 0x50f28007).rw(m_video, FUNC(mac_video_sonora_device::vctrl_r), FUNC(mac_video_sonora_device::vctrl_w));

	map(0x50f2a000, 0x50f2a00f).rw(FUNC(macpdm_state::irq_control_r), FUNC(macpdm_state::irq_control_w));

	map(0x50f2c000, 0x50f2dfff).r(FUNC(macpdm_state::diag_r));

	map(0x50f31000, 0x50f31003).rw(FUNC(macpdm_state::dma_badr_r), FUNC(macpdm_state::dma_badr_w));
	map(0x50f31c20, 0x50f31c20).rw(FUNC(macpdm_state::dma_enet_tx_ctrl_r), FUNC(macpdm_state::dma_enet_tx_ctrl_w));

	map(0x50f32000, 0x50f32003).rw(FUNC(macpdm_state::dma_scsi_a_base_adr_r), FUNC(macpdm_state::dma_scsi_a_base_adr_w));
	map(0x50f32004, 0x50f32007).rw(FUNC(macpdm_state::dma_scsi_b_base_adr_r), FUNC(macpdm_state::dma_scsi_b_base_adr_w));
	map(0x50f32008, 0x50f32008).rw(FUNC(macpdm_state::dma_scsi_a_ctrl_r), FUNC(macpdm_state::dma_scsi_a_ctrl_w));
	map(0x50f32009, 0x50f32009).rw(FUNC(macpdm_state::dma_scsi_b_ctrl_r), FUNC(macpdm_state::dma_scsi_b_ctrl_w));
	map(0x50f32010, 0x50f32013).r(FUNC(macpdm_state::dma_scsi_a_cur_adr_r));
	map(0x50f32014, 0x50f32017).r(FUNC(macpdm_state::dma_scsi_b_cur_adr_r));

	map(0x50f32028, 0x50f32028).rw(FUNC(macpdm_state::dma_enet_rx_ctrl_r), FUNC(macpdm_state::dma_enet_rx_ctrl_w));

	map(0x50f32060, 0x50f32063).rw(FUNC(macpdm_state::dma_floppy_adr_r), FUNC(macpdm_state::dma_floppy_adr_w));
	map(0x50f32064, 0x50f32065).rw(FUNC(macpdm_state::dma_floppy_byte_count_r), FUNC(macpdm_state::dma_floppy_byte_count_w));
	map(0x50f32068, 0x50f32068).rw(FUNC(macpdm_state::dma_floppy_ctrl_r), FUNC(macpdm_state::dma_floppy_ctrl_w));

	map(0x50f32088, 0x50f32088).rw(FUNC(macpdm_state::dma_scc_txa_ctrl_r), FUNC(macpdm_state::dma_scc_txa_ctrl_w));
	map(0x50f32098, 0x50f32098).rw(FUNC(macpdm_state::dma_scc_rxa_ctrl_r), FUNC(macpdm_state::dma_scc_rxa_ctrl_w));
	map(0x50f320a8, 0x50f320a8).rw(FUNC(macpdm_state::dma_scc_txb_ctrl_r), FUNC(macpdm_state::dma_scc_txb_ctrl_w));
	map(0x50f320b8, 0x50f320b8).rw(FUNC(macpdm_state::dma_scc_rxb_ctrl_r), FUNC(macpdm_state::dma_scc_rxb_ctrl_w));

	map(0x50f32100, 0x50f32101).rw(FUNC(macpdm_state::dma_berr_en_r), FUNC(macpdm_state::dma_berr_en_w));
	map(0x50f32102, 0x50f32103).rw(FUNC(macpdm_state::dma_berr_flag_r), FUNC(macpdm_state::dma_berr_flag_w));

	map(0x50f40000, 0x50f4000f).rw(FUNC(macpdm_state::hmc_r), FUNC(macpdm_state::hmc_w));
	map(0x5ffffff8, 0x5fffffff).r(FUNC(macpdm_state::id_r));

	map(0xffc00000, 0xffffffff).rom().region("bootrom", 0);
}

void macpdm_state::macpdm(machine_config &config)
{
	PPC601(config, m_maincpu, 60000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &macpdm_state::pdm_map);
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);

	MAC_VIDEO_SONORA(config, m_video);
	m_video->set_PDM();
	m_video->screen_vblank().set(FUNC(macpdm_state::vblank_irq));

	SPEAKER(config, "speaker", 2).front();

	AWACS(config, m_awacs, SOUND_CLOCK/2);
	m_awacs->irq_out_cb().set(FUNC(macpdm_state::sndo_dma_irq));
	m_awacs->irq_in_cb().set(FUNC(macpdm_state::sndi_dma_irq));
	m_awacs->dma_output().set(FUNC(macpdm_state::sound_dma_output));
	m_awacs->dma_input().set(FUNC(macpdm_state::sound_dma_input));

	m_awacs->add_route(0, "speaker", 1.0, 0);
	m_awacs->add_route(1, "speaker", 1.0, 1);

	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3").option_set("cdrom", NSCSI_CDROM_APPLE).machine_config(
		[](device_t *device)
		{
			device->subdevice<cdda_device>("cdda")->add_route(0, "^^speaker", 1.0, 0);
			device->subdevice<cdda_device>("cdda")->add_route(1, "^^speaker", 1.0, 1);
		});
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr53c94", NCR53C94).machine_config(
		[this] (device_t *device)
		{
			auto &ctrl = downcast<ncr53c94_device &>(*device);
			ctrl.set_clock(ENET_CLOCK/2);
			ctrl.set_busmd(ncr53c94_device::BUSMD_3);
			ctrl.drq_handler_cb().set(*this, FUNC(macpdm_state::scsi_drq));
			ctrl.irq_handler_cb().set(*this, FUNC(macpdm_state::scsi_irq));
		});

	SOFTWARE_LIST(config, "flop_mac35_orig").set_original("mac_flop_orig");
	SOFTWARE_LIST(config, "flop_mac35_clean").set_original("mac_flop_clcracked");
	SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");
	SOFTWARE_LIST(config, "flop35hd_list").set_original("mac_hdflop");
	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
	SOFTWARE_LIST(config, "cd_list").set_original("mac_cdrom").set_filter("PPC601");

	SWIM3(config, m_fdc, IO_CLOCK);
	m_fdc->irq_cb().set(FUNC(macpdm_state::fdc_irq));
	m_fdc->drq_cb().set(FUNC(macpdm_state::fdc_drq));
	m_fdc->hdsel_cb().set(FUNC(macpdm_state::hdsel_w));
	m_fdc->devsel_cb().set(FUNC(macpdm_state::devsel_w));
	m_fdc->phases_cb().set(FUNC(macpdm_state::phases_w));
	m_fdc->sel35_cb().set(FUNC(macpdm_state::sel35_w));
	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_nc(config, m_floppy[1]);

	// pclk is maincpu:60MHz/4, RTxCA is IO_CLOCK*2/17 or GPI input, RTxCB is IO_CLOCK*2/17
	// IO_CLOCK*2/17 is 3'686'400
	SCC85C30(config, m_scc, 60000000/4);
	m_scc->configure_channels(3'686'400, 3'686'400, 3'686'400, 3'686'400);
	m_scc->out_int_callback().set(FUNC(macpdm_state::scc_irq));
	m_scc->out_txda_callback().set("printer", FUNC(rs232_port_device::write_txd));
	m_scc->out_txdb_callback().set("modem", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232a(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(m_scc, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(m_scc, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "modem", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_scc, FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(m_scc, FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(m_scc, FUNC(z80scc_device::ctsb_w));

	R65NC22(config, m_via1, IO_CLOCK/40);
	m_via1->readpa_handler().set(FUNC(macpdm_state::via1_in_a));
	m_via1->readpb_handler().set(FUNC(macpdm_state::via1_in_b));
	m_via1->writepa_handler().set(FUNC(macpdm_state::via1_out_a));
	m_via1->writepb_handler().set(FUNC(macpdm_state::via1_out_b));
	m_via1->cb2_handler().set(FUNC(macpdm_state::via1_out_cb2));
	m_via1->irq_handler().set(FUNC(macpdm_state::via1_irq));

	RAM(config, m_ram);
	m_ram->set_default_size("8M");
	m_ram->set_extra_options("12M,24M,72M,264M");

	nubus_device &nubus(NUBUS(config, "nubus", 0));
	nubus.set_space(m_maincpu, AS_PROGRAM);
	nubus.out_irqc_callback().set(FUNC(macpdm_state::slot0_irq_w));
	nubus.out_irqd_callback().set(FUNC(macpdm_state::slot1_irq_w));
	nubus.out_irqe_callback().set(FUNC(macpdm_state::slot2_irq_w));

	// 6100 with the NuBus adapter has one slot, slot $E
	NUBUS_SLOT(config, "nbe", "nubus", powermac_nubus_cards, nullptr);

	MACADB(config, m_macadb, IO_CLOCK/2);
	CUDA_V2XX(config, m_cuda, XTAL(32'768));
	m_cuda->set_default_bios_tag("341s0060");
	m_cuda->reset_callback().set(FUNC(macpdm_state::cuda_reset_w));
	m_cuda->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_cuda->via_clock_callback().set(m_via1, FUNC(via6522_device::write_cb1));
	m_cuda->via_data_callback().set(m_via1, FUNC(via6522_device::write_cb2));
	m_cuda->nmi_callback().set(FUNC(macpdm_state::nmi_irq));
	m_macadb->adb_data_callback().set(m_cuda, FUNC(cuda_device::set_adb_line));
	m_macadb->adb_power_callback().set(m_cuda, FUNC(cuda_device::set_adb_power));
	config.set_perfect_quantum(m_maincpu);

	TIMER(config, "beat_60_15").configure_periodic(FUNC(macpdm_state::via1_60_15_timer), attotime::from_double(1/60.15));
}

static INPUT_PORTS_START( macpdm )
INPUT_PORTS_END

ROM_START( pmac6100 )
	ROM_REGION64_BE(0x400000, "bootrom", 0)
	ROM_LOAD( "9feb69b3.rom", 0x000000, 0x400000, CRC(a43fadbc) SHA1(6fac1c4e920a077c077b03902fef9199d5e8f2c3) )
ROM_END

} // anonymous namespace


COMP( 1994, pmac6100,  0, 0, macpdm, macpdm, macpdm_state, driver_init, "Apple Computer", "Power Macintosh 6100/60",  MACHINE_NOT_WORKING )
