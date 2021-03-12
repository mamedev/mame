// license:BSD-3-Clause
// copyright-holders:R. Belmont, O. Galibert

#include "emu.h"

#include "bus/nubus/nubus.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"
#include "cpu/powerpc/ppc.h"
#include "machine/6522via.h"
#include "machine/8530scc.h"
#include "machine/cuda.h"
#include "machine/macadb.h"
#include "machine/mv_sonora.h"
#include "machine/ncr5380.h"
#include "machine/ram.h"
#include "machine/swim3.h"
#include "softlist.h"
#include "sound/awacs.h"
#include "speaker.h"

constexpr auto IO_CLOCK = 31.3344_MHz_XTAL;
[[maybe_unused]] constexpr auto VGA_CLOCK = 25.1750_MHz_XTAL;
[[maybe_unused]] constexpr auto DOT_CLOCK = 57.2832_MHz_XTAL;
constexpr auto ENET_CLOCK = 20_MHz_XTAL;

class macpdm_state : public driver_device
{
public:
	macpdm_state(const machine_config &mconfig, device_type type, const char *tag);

	void macpdm(machine_config &config);

	virtual void driver_init() override;
	virtual void driver_reset() override;

private:
	required_device<ppc_device> m_maincpu;
	required_device<via6522_device> m_via1;
	required_device<awacs_device> m_awacs;
	required_device<cuda_device> m_cuda;
	required_device<macadb_device> m_macadb;
	required_device<ram_device> m_ram;
	required_device<scc8530_legacy_device> m_scc;
	required_device<ncr5380_device> m_ncr5380;
	required_device<applefdintf_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<mac_video_sonora_device> m_video;

	floppy_image_device *m_cur_floppy;

	uint32_t m_model_id;
	uint64_t m_hmc_reg, m_hmc_buffer;
	uint8_t m_hmc_bit;

	uint8_t m_irq_control;

	uint32_t m_dma_badr, m_dma_floppy_adr;
	uint16_t m_dma_berr_en, m_dma_berr_flag;

	uint8_t m_dma_scsi_ctrl, m_dma_floppy_ctrl;
	uint8_t m_dma_scc_txa_ctrl, m_dma_scc_rxa_ctrl, m_dma_scc_txb_ctrl, m_dma_scc_rxb_ctrl;
	uint8_t m_dma_enet_rx_ctrl, m_dma_enet_tx_ctrl;

	void pdm_map(address_map &map);

	DECLARE_WRITE_LINE_MEMBER(nmi_irq);
	DECLARE_WRITE_LINE_MEMBER(dma_irq);
	DECLARE_WRITE_LINE_MEMBER(enet_irq);
	DECLARE_WRITE_LINE_MEMBER(scc_irq);
	DECLARE_WRITE_LINE_MEMBER(via2_irq);
	DECLARE_WRITE_LINE_MEMBER(via1_irq);

	DECLARE_WRITE_LINE_MEMBER(bus_err_irq);
	DECLARE_WRITE_LINE_MEMBER(fdc_irq);
	DECLARE_WRITE_LINE_MEMBER(etx_irq);
	DECLARE_WRITE_LINE_MEMBER(erx_irq);
	DECLARE_WRITE_LINE_MEMBER(txa_irq);
	DECLARE_WRITE_LINE_MEMBER(rxa_irq);
	DECLARE_WRITE_LINE_MEMBER(txb_irq);
	DECLARE_WRITE_LINE_MEMBER(rxb_irq);

	DECLARE_WRITE_LINE_MEMBER(sndo_irq);
	DECLARE_WRITE_LINE_MEMBER(sndi_irq);

	DECLARE_WRITE_LINE_MEMBER(fdc_err_irq);
	DECLARE_WRITE_LINE_MEMBER(etx_err_irq);
	DECLARE_WRITE_LINE_MEMBER(erx_err_irq);
	DECLARE_WRITE_LINE_MEMBER(txa_err_irq);
	DECLARE_WRITE_LINE_MEMBER(rxa_err_irq);
	DECLARE_WRITE_LINE_MEMBER(txb_err_irq);
	DECLARE_WRITE_LINE_MEMBER(rxb_err_irq);

	DECLARE_WRITE_LINE_MEMBER(scsi_err_irq);
	DECLARE_WRITE_LINE_MEMBER(sndo_err_irq);
	DECLARE_WRITE_LINE_MEMBER(sndi_err_irq);

	void phases_w(uint8_t phases);
	void sel35_w(int sel35);
	void devsel_w(uint8_t devsel);
	void hdsel_w(int hdsel);

	uint8_t via1_in_a();
	uint8_t via1_in_b();
	void via1_out_a(uint8_t data);
	void via1_out_b(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(via1_out_cb2);

	DECLARE_WRITE_LINE_MEMBER(cuda_reset_w);

	uint8_t via1_r(offs_t offset);
	void via1_w(offs_t offset, uint8_t data);

	uint8_t via2_r(offs_t offset);
	void via2_w(offs_t offset, uint8_t data);

	uint8_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint8_t data);

	uint8_t scc_r(offs_t offset);
	void scc_w(offs_t offset, uint8_t data);

	uint8_t scsi_r(offs_t offset);
	void scsi_w(offs_t offset, uint8_t data);

	uint8_t hmc_r(offs_t offset);
	void hmc_w(offs_t offset, uint8_t data);

	uint32_t id_r();

	uint8_t diag_r(offs_t offset);

	uint8_t irq_control_r();
	void irq_control_w(uint8_t data);
	void irq_main_set(uint8_t mask, int state);

	uint32_t dma_badr_r();
	void dma_badr_w(offs_t, uint32_t data, uint32_t mem_mask);
	uint16_t dma_berr_en_r();
	void dma_berr_en_w(offs_t, uint16_t data, uint16_t mem_mask);
	uint16_t dma_berr_flag_r();
	void dma_berr_flag_w(offs_t, uint16_t data, uint16_t mem_mask);

	uint8_t dma_scsi_ctrl_r();
	void dma_scsi_ctrl_w(uint8_t data);

	uint8_t dma_floppy_ctrl_r();
	void dma_floppy_ctrl_w(uint8_t data);
	uint32_t dma_floppy_adr_r();
	void dma_floppy_adr_w(offs_t, uint32_t data, uint32_t mem_mask);

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
	m_ncr5380(*this, "ncr5380"),
	m_fdc(*this, "fdc"),
	m_floppy(*this, "fdc:%d", 0U),
	m_video(*this, "video")
{
	m_cur_floppy = nullptr;
}

void macpdm_state::driver_init()
{
	m_maincpu->space().install_ram(0, m_ram->mask(), 0x3000000,m_ram->pointer());
	m_maincpu->space().nop_readwrite(m_ram->size(), 0xffffff, 0x3000000);
	m_model_id = 0xa55a3011;
	// 7100 = a55a3012
	// 8100 = a55a3013

	m_awacs->set_dma_base(m_maincpu->space(AS_PROGRAM), 0x10000, 0x12000);

	save_item(NAME(m_hmc_reg));
	save_item(NAME(m_hmc_buffer));
	save_item(NAME(m_hmc_bit));

	save_item(NAME(m_irq_control));

	save_item(NAME(m_dma_badr));
	save_item(NAME(m_dma_berr_en));
	save_item(NAME(m_dma_berr_flag));
	save_item(NAME(m_dma_scsi_ctrl));
	save_item(NAME(m_dma_floppy_ctrl));
	save_item(NAME(m_dma_scc_txa_ctrl));
	save_item(NAME(m_dma_scc_rxa_ctrl));
	save_item(NAME(m_dma_scc_txb_ctrl));
	save_item(NAME(m_dma_scc_rxb_ctrl));
	save_item(NAME(m_dma_enet_rx_ctrl));
	save_item(NAME(m_dma_enet_tx_ctrl));

	save_item(NAME(m_dma_floppy_adr));

	m_maincpu->space().install_read_tap(0x4000c2e0, 0x4000c2e7, 0, "cuda", [this](offs_t offset, u64 &data, u64 mem_mask) {
											if(mem_mask == 0xffff000000000000) {
												offs_t badr = m_maincpu->state_int(PPC_R16);
												m_maincpu->translate(AS_PROGRAM, TRANSLATE_READ_DEBUG, badr);
												logerror("cuda packet %08x : type %02x cmd %02x - %02x %02x %02x %02x bytecnt %04x\n",
														 badr,
														 m_maincpu->space().read_byte(badr),
														 m_maincpu->space().read_byte(badr+1),
														 m_maincpu->space().read_byte(badr+2),
														 m_maincpu->space().read_byte(badr+3),
														 m_maincpu->space().read_byte(badr+4),
														 m_maincpu->space().read_byte(badr+5),
														 m_maincpu->space().read_word(badr+6));
											}
										});
}

void macpdm_state::driver_reset()
{
	m_hmc_reg = 0;
	m_hmc_buffer = 0;
	m_hmc_bit = 0;

	m_irq_control = 0;

	m_dma_badr = 0;
	m_dma_berr_en = 0;
	m_dma_berr_flag = 0;
	m_dma_scsi_ctrl = 0;
	m_dma_floppy_ctrl = 0;
	m_dma_scc_txa_ctrl = 0;
	m_dma_scc_rxa_ctrl = 0;
	m_dma_scc_txb_ctrl = 0;
	m_dma_scc_rxb_ctrl = 0;
	m_dma_enet_rx_ctrl = 0;
	m_dma_enet_tx_ctrl = 0;

	m_dma_floppy_adr = 0x15000;

	m_video->set_vram_base((const u64 *)m_ram->pointer());
	m_video->set_vram_offset(0);
}

uint8_t macpdm_state::irq_control_r()
{
	return m_irq_control;
}

void macpdm_state::irq_control_w(uint8_t data)
{
	if((m_irq_control ^ data) & 0x40) {
		m_irq_control = (m_irq_control & ~0xc0) | (data & 0x40);
		m_maincpu->set_input_line(PPC_IRQ, CLEAR_LINE);
	}
	if((data & 0xc0) == 0xc0 && (m_irq_control & 0x80)) {
		m_irq_control &= 0x7f;
		m_maincpu->set_input_line(PPC_IRQ, CLEAR_LINE);
	}	

	logerror("irq control %02x\n", m_irq_control);
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

	logerror("irq control %02x\n", m_irq_control);
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

WRITE_LINE_MEMBER(macpdm_state::via1_out_cb2)
{
	m_cuda->set_via_data(state & 1);
}


WRITE_LINE_MEMBER(macpdm_state::cuda_reset_w)
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

uint8_t macpdm_state::via2_r(offs_t offset)
{
	logerror("via2_r %x\n", offset);
	return 0;
}

void macpdm_state::via2_w(offs_t offset, uint8_t data)
{
	logerror("via2_w %x, %02x\n", offset, data);
}

uint8_t macpdm_state::scc_r(offs_t offset)
{
	return m_scc->reg_r(offset >> 1);
}

void macpdm_state::scc_w(offs_t offset, uint8_t data)
{
	m_scc->reg_w(offset, data);
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
	return m_ncr5380->ncr5380_read_reg(offset >> 4);
}

void macpdm_state::scsi_w(offs_t offset, uint8_t data)
{
	return m_ncr5380->ncr5380_write_reg(offset >> 4, data);
}

uint8_t macpdm_state::hmc_r(offs_t offset)
{
	return (m_hmc_reg >> m_hmc_bit) & 1 ? 0x80 : 0x00;
}

void macpdm_state::hmc_w(offs_t offset, uint8_t data)
{
	if(offset & 8)
		m_hmc_bit = 0;
	else {
		if(data & 0x80)
			m_hmc_buffer |= u64(1) << m_hmc_bit;
		else
			m_hmc_buffer &= ~(u64(1) << m_hmc_bit);
		m_hmc_bit ++;
		if(m_hmc_bit == 35) {
			m_hmc_reg = m_hmc_buffer & ~3; // csiz is readonly, we pretend there isn't a l2 cache
			m_video->set_vram_offset(m_hmc_reg & 0x200000000 ? 0x100000 : 0);
			logerror("HMC l2=%c%c%c%c%c vbase=%c%s mbram=%cM size=%x%s romd=%d refresh=%02x w=%c%c%c%c ras=%d%d%d%d\n",
					 m_hmc_reg & 0x008000000 ? '+' : '-',      // l2_en
					 m_hmc_reg & 0x400000000 ? '3' : '2',      // l2_init
					 m_hmc_reg & 0x004000000 ? '1' : '2',      // l2_brst
					 m_hmc_reg & 0x010000000 ? 'I' : 'U',      // l2_inst
					 m_hmc_reg & 0x002000000 ? 'w' : '.',      // l2romw
					 m_hmc_reg & 0x200000000 ? '1' : '0',      // vbase
					 m_hmc_reg & 0x100000000 ? " vtst" : "",   // vtst
					 m_hmc_reg & 0x080000000 ? '8' : '4',      // mb_ram
					 (m_hmc_reg >> 29) & 3,                    // size
					 m_hmc_reg & 0x001000000 ? " nblrom" : "", // nblrom
					 12 - 2*((m_hmc_reg >> 22) & 3),           // romd
					 (m_hmc_reg >> 16) & 0x3f,                 // rfsh
					 m_hmc_reg & 0x000000008 ? '3' : '2',      // winit
					 m_hmc_reg & 0x000000004 ? '3' : '2',      // wbrst
					 m_hmc_reg & 0x000008000 ? '1' : '2',      // wcasp
					 m_hmc_reg & 0x000004000 ? '1' : '2',      // wcasd
					 3 - ((m_hmc_reg >> 12) & 3),              // rdac
					 6 - ((m_hmc_reg >> 8) & 3),               // rasd
					 5 - ((m_hmc_reg >> 6) & 3),               // rasp
					 4 - ((m_hmc_reg >> 4) & 3));              // rcasd
		}
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

WRITE_LINE_MEMBER(macpdm_state::scc_irq)
{
	logerror("scc irq %d\n", state);
}

WRITE_LINE_MEMBER(macpdm_state::via1_irq)
{
	irq_main_set(0x01, state);
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


uint8_t macpdm_state::dma_scsi_ctrl_r()
{
	return m_dma_scsi_ctrl;
}

void macpdm_state::dma_scsi_ctrl_w(uint8_t data)
{
	m_dma_scsi_ctrl = data;
	logerror("dma_scsi_ctrl_w %02x\n", m_dma_scsi_ctrl);
}


uint8_t macpdm_state::dma_floppy_ctrl_r()
{
	return m_dma_floppy_ctrl;
}

void macpdm_state::dma_floppy_ctrl_w(uint8_t data)
{
	m_dma_floppy_ctrl = data;
	logerror("dma_floppy_ctrl_w %02x\n", m_dma_floppy_ctrl);
}

uint32_t macpdm_state::dma_floppy_adr_r()
{
	return m_dma_floppy_adr;
}

void macpdm_state::dma_floppy_adr_w(offs_t, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dma_floppy_adr);
	m_dma_floppy_adr = (m_dma_badr | 0x10000) + (m_dma_floppy_adr & 0xffff);
	logerror("dma floppy adr %08x\n", m_dma_floppy_adr);
}


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



void macpdm_state::pdm_map(address_map &map)
{
	map(0x40000000, 0x403fffff).rom().region("bootrom", 0).mirror(0x0fc00000);

	map(0x50f00000, 0x50f00000).rw(FUNC(macpdm_state::via1_r), FUNC(macpdm_state::via1_w)).select(0x1e00);
	map(0x50f04000, 0x50f04000).rw(FUNC(macpdm_state::scc_r), FUNC(macpdm_state::scc_w)).select(0x000e);
	// 50f08000 = ethernet ID PROM
	// 50f0a000 = MACE ethernet controller
	map(0x50f10000, 0x50f10000).rw(FUNC(macpdm_state::scsi_r), FUNC(macpdm_state::scsi_w)).select(0xf0);
	// 50f14000 = sound registers (AWACS)
	map(0x50f14000, 0x50f1401f).rw(m_awacs, FUNC(awacs_device::read), FUNC(awacs_device::write));
	map(0x50f16000, 0x50f16000).rw(FUNC(macpdm_state::fdc_r), FUNC(macpdm_state::fdc_w)).select(0x1e00);
	map(0x50f24000, 0x50f24003).w(m_video, FUNC(mac_video_sonora_device::dac_w));
	map(0x50f26000, 0x50f2601f).rw(FUNC(macpdm_state::via2_r), FUNC(macpdm_state::via2_w));
	map(0x50f28000, 0x50f28007).rw(m_video, FUNC(mac_video_sonora_device::vctrl_r), FUNC(mac_video_sonora_device::vctrl_w));

	map(0x50f2a000, 0x50f2a000).rw(FUNC(macpdm_state::irq_control_r), FUNC(macpdm_state::irq_control_w));

	map(0x50f2c000, 0x50f2dfff).r(FUNC(macpdm_state::diag_r));

	map(0x50f31000, 0x50f31003).rw(FUNC(macpdm_state::dma_badr_r), FUNC(macpdm_state::dma_badr_w));
	map(0x50f31c20, 0x50f31c20).rw(FUNC(macpdm_state::dma_enet_tx_ctrl_r), FUNC(macpdm_state::dma_enet_tx_ctrl_w));
	map(0x50f32008, 0x50f32008).rw(FUNC(macpdm_state::dma_scsi_ctrl_r), FUNC(macpdm_state::dma_scsi_ctrl_w));
	map(0x50f32028, 0x50f32028).rw(FUNC(macpdm_state::dma_enet_rx_ctrl_r), FUNC(macpdm_state::dma_enet_rx_ctrl_w));
	map(0x50f32060, 0x50f32063).rw(FUNC(macpdm_state::dma_floppy_adr_r), FUNC(macpdm_state::dma_floppy_adr_w));
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

	MAC_VIDEO_SONORA(config, m_video);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	AWACS(config, m_awacs, 44100);
	m_awacs->add_route(0, "lspeaker", 1.0);
	m_awacs->add_route(1, "rspeaker", 1.0);

	scsi_port_device &scsibus(SCSI_PORT(config, "scsi"));
	scsibus.set_slot_device(1, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_6));
	scsibus.set_slot_device(2, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_5));

	NCR5380(config, m_ncr5380, ENET_CLOCK/2);
	m_ncr5380->set_scsi_port("scsi");

	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");

	SWIM3(config, m_fdc, IO_CLOCK/2);
	m_fdc->hdsel_cb().set(FUNC(macpdm_state::hdsel_w));
	m_fdc->devsel_cb().set(FUNC(macpdm_state::devsel_w));
	m_fdc->phases_cb().set(FUNC(macpdm_state::phases_w));
	m_fdc->sel35_cb().set(FUNC(macpdm_state::sel35_w));
	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_nc(config, m_floppy[1]);

	// pclk is maincpu:60MHz/4, RTxCA is IO_CLOCK*2/17 or GPI input, RTxCB is IO_CLOCK*2/17
	SCC8530(config, m_scc, 60000000/4);
	m_scc->intrq_callback().set(FUNC(macpdm_state::scc_irq));

	R65NC22(config, m_via1, IO_CLOCK/2);
	m_via1->readpa_handler().set(FUNC(macpdm_state::via1_in_a));
	m_via1->readpb_handler().set(FUNC(macpdm_state::via1_in_b));
	m_via1->writepa_handler().set(FUNC(macpdm_state::via1_out_a));
	m_via1->writepb_handler().set(FUNC(macpdm_state::via1_out_b));
	m_via1->cb2_handler().set(FUNC(macpdm_state::via1_out_cb2));
	m_via1->irq_handler().set(FUNC(macpdm_state::via1_irq));

	RAM(config, m_ram);
	m_ram->set_default_size("8M");
	m_ram->set_extra_options("16M,32M,64M,128M");

	MACADB(config, m_macadb, IO_CLOCK/2);
	CUDA(config, m_cuda, CUDA_341S0060);
	m_cuda->reset_callback().set(FUNC(macpdm_state::cuda_reset_w));
	m_cuda->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_cuda->via_clock_callback().set(m_via1, FUNC(via6522_device::write_cb1));
	m_cuda->via_data_callback().set(m_via1, FUNC(via6522_device::write_cb2));
	m_macadb->set_mcu_mode(true);
	m_macadb->adb_data_callback().set(m_cuda, FUNC(cuda_device::set_adb_line));
	config.set_perfect_quantum(m_maincpu);
}

static INPUT_PORTS_START( macpdm )
INPUT_PORTS_END

ROM_START( pmac6100 )
	ROM_REGION64_BE(0x400000, "bootrom", 0)
	ROM_LOAD( "9feb69b3.rom", 0x000000, 0x400000, CRC(a43fadbc) SHA1(6fac1c4e920a077c077b03902fef9199d5e8f2c3) )
ROM_END


COMP( 1994, pmac6100,  0, 0, macpdm, macpdm, macpdm_state, driver_init, "Apple Computer", "Power Macintosh 6100/60",  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
