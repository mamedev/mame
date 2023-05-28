#include "emu.h"
#include "h8_dma.h"

DEFINE_DEVICE_TYPE(H8_DMA,         h8_dma_device,         "h8_dma",         "H8 DMA controller")
DEFINE_DEVICE_TYPE(H8_DMA_CHANNEL, h8_dma_channel_device, "h8_dma_channel", "H8 DMA channel")

h8_dma_device::h8_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, H8_DMA, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_dmach0(*this, "0"),
	m_dmach1(*this, "1")
{
}

void h8_dma_device::device_start()
{
	m_dmach0->set_id(0<<1);
	m_dmach1->set_id(1<<1);

	save_item(NAME(m_dmabcr));
	save_item(NAME(m_dmawer));
	save_item(NAME(m_dreq));
}

void h8_dma_device::device_reset()
{
	m_dmabcr = 0x0000;
	m_dmawer = 0x00;
	m_dreq[0] = m_dreq[1] = false;
}

bool h8_dma_device::trigger_dma(int vector)
{
	// Don't shortcut! Both dmas may be started
	bool start0 = m_dmach0->start_test(vector);
	bool start1 = m_dmach1->start_test(vector);

	return start0 || start1;
}

void h8_dma_device::count_last(int id)
{
	if(id & 2)
		m_dmach1->count_last(id & 1);
	else
		m_dmach0->count_last(id & 1);
}

void h8_dma_device::count_done(int id)
{
	if(id & 2)
		m_dmach1->count_done(id & 1);
	else
		m_dmach0->count_done(id & 1);
}

void h8_dma_device::clear_dte(int id)
{
	m_dmabcr &= ~(0x0010 << id);
}

void h8_dma_device::set_input(int inputnum, int state)
{
	if(inputnum == H8_INPUT_LINE_DREQ0) {
		if(state == ASSERT_LINE) {
			m_dmach0->start_test(h8_dma_channel_device::DREQ_LEVEL);
			if(!m_dreq[0])
				m_dmach0->start_test(h8_dma_channel_device::DREQ_EDGE);
		}
		m_dreq[0] = (state == ASSERT_LINE);
	} else if(inputnum == H8_INPUT_LINE_DREQ1) {
		if(state == ASSERT_LINE) {
			m_dmach1->start_test(h8_dma_channel_device::DREQ_LEVEL);
			if(!m_dreq[1])
				m_dmach1->start_test(h8_dma_channel_device::DREQ_EDGE);
		}
		m_dreq[1] = (state == ASSERT_LINE);
	} else
		logerror("input line %d not supported for h8_dma_device\n", inputnum);
}

uint8_t h8_dma_device::dmawer_r()
{
	logerror("dmawer_r %02x\n", m_dmawer);
	return m_dmawer;
}

void h8_dma_device::dmawer_w(uint8_t data)
{
	m_dmawer = data;
	logerror("dmawer_w %02x\n", data);
}

uint8_t h8_dma_device::dmatcr_r()
{
	logerror("dmatcr_r %02x\n", m_dmatcr);
	return m_dmatcr;
}

void h8_dma_device::dmatcr_w(uint8_t data)
{
	m_dmatcr = data;
	logerror("dmatcr_w %02x\n", data);
}

uint16_t h8_dma_device::dmabcr_r()
{
	logerror("dmabcr_r %04x\n", m_dmabcr);
	return m_dmabcr;
}

void h8_dma_device::dmabcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dmabcr);
	logerror("dmabcr_w %04x\n", m_dmabcr);
	m_dmach0->set_bcr(m_dmabcr & 0x4000, m_dmabcr & 0x1000, m_dmabcr >>  8, m_dmabcr >> 4, m_dmabcr >> 0);
	m_dmach1->set_bcr(m_dmabcr & 0x8000, m_dmabcr & 0x2000, m_dmabcr >> 10, m_dmabcr >> 6, m_dmabcr >> 2);
}



h8_dma_channel_device::h8_dma_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, H8_DMA_CHANNEL, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_dma(*this, finder_base::DUMMY_TAG),
	m_intc(*this, finder_base::DUMMY_TAG)
{
}

void h8_dma_channel_device::set_info(int irq_base, int v0, int v1, int v2, int v3, int v4, int v5, int v6, int v7, int v8, int v9, int va, int vb, int vc, int vd, int ve, int vf)
{
	m_irq_base = irq_base;
	m_activation_vectors[ 0] = v0;
	m_activation_vectors[ 1] = v1;
	m_activation_vectors[ 2] = v2;
	m_activation_vectors[ 3] = v3;
	m_activation_vectors[ 4] = v4;
	m_activation_vectors[ 5] = v5;
	m_activation_vectors[ 6] = v6;
	m_activation_vectors[ 7] = v7;
	m_activation_vectors[ 8] = v8;
	m_activation_vectors[ 9] = v9;
	m_activation_vectors[10] = va;
	m_activation_vectors[11] = vb;
	m_activation_vectors[12] = vc;
	m_activation_vectors[13] = vd;
	m_activation_vectors[14] = ve;
	m_activation_vectors[15] = vf;
	memset(m_state, 0, sizeof(m_state));
}

void h8_dma_channel_device::device_start()
{
	save_item(STRUCT_MEMBER(m_state, m_source));
	save_item(STRUCT_MEMBER(m_state, m_dest));
	save_item(STRUCT_MEMBER(m_state, m_incs));
	save_item(STRUCT_MEMBER(m_state, m_incd));
	save_item(STRUCT_MEMBER(m_state, m_count));
	save_item(STRUCT_MEMBER(m_state, m_id));
	save_item(STRUCT_MEMBER(m_state, m_autoreq));
	save_item(STRUCT_MEMBER(m_state, m_suspended));
	save_item(STRUCT_MEMBER(m_state, m_mode_16));
	save_item(NAME(m_mar));
	save_item(NAME(m_ioar));
	save_item(NAME(m_etcr));
	save_item(NAME(m_dmacr));
	save_item(NAME(m_dtcr));
	save_item(NAME(m_dta));
	save_item(NAME(m_dte));
	save_item(NAME(m_dtie));
	save_item(NAME(m_fae));
	save_item(NAME(m_sae));
}

void h8_dma_channel_device::device_reset()
{
	m_dmacr = 0x0000;
	m_mar[0] = m_mar[1] = 0;
	m_ioar[0] = m_ioar[1] = 0;
	m_etcr[0] = m_etcr[1] = 0;
	m_dtcr[0] = m_dtcr[1] = 0;
	m_fae = m_sae = false;
	m_dta = m_dte = m_dtie = 0;
}

void h8_dma_channel_device::set_id(int id)
{
	m_state[0].m_id = id;
	m_state[1].m_id = id | 1;
}

uint16_t h8_dma_channel_device::marah_r()
{
	logerror("marah_r %06x\n", m_mar[0]);
	return m_mar[0] >> 16;
}

void h8_dma_channel_device::marah_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(ACCESSING_BITS_0_7)
		m_mar[0] = ((data & 0x00ff) << 16) | (m_mar[0] & 0xffff);
	logerror("marah_w %06x\n", m_mar[0]);
}

uint16_t h8_dma_channel_device::maral_r()
{
	logerror("maral_r %06x\n", m_mar[0]);
	return m_mar[0];
}

void h8_dma_channel_device::maral_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_mar[0] = (m_mar[0] & ~mem_mask) | (data & mem_mask);
	logerror("maral_w %06x\n", m_mar[0]);
}

uint16_t h8_dma_channel_device::ioara_r()
{
	return m_ioar[0];
}

uint8_t h8_dma_channel_device::ioara8_r()
{
	return m_ioar[0] & 0x00ff;
}

void h8_dma_channel_device::ioara_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ioar[0]);
	logerror("ioara_w %04x\n", m_ioar[0]);
}

void h8_dma_channel_device::ioara8_w(uint8_t data)
{
	m_ioar[0] = data | 0xff00;
	logerror("ioara_w %04x\n", m_ioar[0]);
}

uint16_t h8_dma_channel_device::etcra_r()
{
	logerror("etcra_r %04x\n", m_etcr[0]);
	return m_etcr[0];
}

void h8_dma_channel_device::etcra_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_etcr[0]);
	logerror("etcra_w %04x\n", m_etcr[0]);
}

uint16_t h8_dma_channel_device::marbh_r()
{
	logerror("marbh_r %06x\n", m_mar[1]);
	return m_mar[1] >> 16;
}

void h8_dma_channel_device::marbh_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(ACCESSING_BITS_0_7)
		m_mar[1] = ((data & 0x00ff) << 16) | (m_mar[1] & 0xffff);
	logerror("marbh_w %06x\n", m_mar[1]);
}

uint16_t h8_dma_channel_device::marbl_r()
{
	logerror("marbl_r %06x\n", m_mar[1]);
	return m_mar[1];
}

void h8_dma_channel_device::marbl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_mar[1] = (m_mar[1] & ~mem_mask) | (data & mem_mask);
	logerror("marbl_w %06x\n", m_mar[1]);
}

uint16_t h8_dma_channel_device::ioarb_r()
{
	return m_ioar[1];
}

uint8_t h8_dma_channel_device::ioarb8_r()
{
	return m_ioar[1] & 0x00ff;
}

void h8_dma_channel_device::ioarb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ioar[1]);
	logerror("ioarb_w %04x\n", m_ioar[1]);
}

void h8_dma_channel_device::ioarb8_w(uint8_t data)
{
	m_ioar[1] = data | 0xff00;
	logerror("ioarb_w %04x\n", m_ioar[1]);
}

uint16_t h8_dma_channel_device::etcrb_r()
{
	logerror("etcrb_r %04x\n", m_etcr[1]);
	return m_etcr[1];
}

void h8_dma_channel_device::etcrb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_etcr[1]);
	logerror("etcrb_w %04x\n", m_etcr[1]);
}

uint16_t h8_dma_channel_device::dmacr_r()
{
	logerror("dmacr_r %04x\n", m_dmacr);
	return m_dmacr;
}

void h8_dma_channel_device::dmacr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dmacr);
	logerror("dmacr_w %04x\n", m_dmacr);
	start_test(-1);
}

// H8H DMA
uint8_t h8_dma_channel_device::dtcra_r()
{
	logerror("dtcra_r %02x\n", m_dtcr[0]);
	return m_dtcr[0];
}

void h8_dma_channel_device::dtcra_w(uint8_t data)
{
	m_dtcr[0] = data;
	logerror("dtcra_w %02x\n", m_dtcr[0]);
	if((m_dtcr[0] & 0x80) && (m_dtcr[1] & 0x80)) { // if both DTME and DTE are set, start  DMA
		h8h_sync();
	}
}

uint8_t h8_dma_channel_device::dtcrb_r()
{
	logerror("dtcrb_r %02x\n", m_dtcr[1]);
	return m_dtcr[1];
}

void h8_dma_channel_device::dtcrb_w(uint8_t data)
{
	m_dtcr[1] = data;
	logerror("dtcrb_w %02x\n", m_dtcr[1]);
	if((m_dtcr[0] & 0x80) && (m_dtcr[1] & 0x80)) { // if both DTME and DTE are set, start  DMA
		h8h_sync();
	}
}

void h8_dma_channel_device::h8h_sync()
{
	// update DMACR
	m_dmacr = 0;
	if(BIT(m_dtcr[0], 6)) m_dmacr |= 1 << 15; // DTSZ
	m_dmacr |= ((m_dtcr[0] & 0b110000) >> 4) << 13; // SAID/DTID, SAIDE/RPE
	m_dmacr |= ((m_dtcr[1] & 0b110000) >> 4) << 5; // DAID/DTID, DAIDE/RPE

	uint8_t dte = 0;
	if(BIT(m_dtcr[0], 7)) dte |= 0b01; // DTE
	if(BIT(m_dtcr[1], 7)) dte |= 0b10; // DTME/DTE

	bool fae = (m_dtcr[0] & 0b110) == 0b110; // A channel operates in full address mode when DTS2A and DTS1A are both set to 1.
	bool sae = false; // don't support
	uint8_t dta = 0; // don't support
	uint8_t dtie = 0;
	if(fae) {
		// Full address mode
		if(BIT(m_dtcr[0], 3)) dtie = 0b11;
		if(BIT(m_dtcr[0], 0)) {
			// Block Transfer Mode
			m_dmacr |= 1 << 11; // BLKE
			if(BIT(m_dtcr[1], 3)) m_dmacr |= 1 << 12; // BLKDIR (TMS)
			switch(m_dtcr[1] & 0b111) { // DTP (DTS)
			case 0b000: m_dmacr |= 0b1000; break; // ITU channel 0
			case 0b001: m_dmacr |= 0b1001; break; // ITU channel 1
			case 0b010: m_dmacr |= 0b1010; break; // ITU channel 2
			case 0b011: m_dmacr |= 0b1011; break; // ITU channel 3
			case 0b110: m_dmacr |= 0b1000; break; // DREQ falling edge
			}
		} else {
			// Normal Mode
			switch(m_dtcr[1] & 0b111) { // DTP (DTS)
			case 0b000: m_dmacr |= 0b0111; break; // Auto-request (burst mode)
			case 0b010: m_dmacr |= 0b0110; break; // Auto-request (cycle-steal mode)
			case 0b110: m_dmacr |= 0b0010; break; // DREQ falling edge
			case 0b111: m_dmacr |= 0b0011; break; // DREQ low-level
			}
		}
	} else {
		// Short address mode
		if(BIT(m_dtcr[0], 3)) dtie |= 0b01;
		if(BIT(m_dtcr[1], 3)) dtie |= 0b10;
		for(int submodule = 0; submodule < 2; submodule++) {
			switch(m_dtcr[submodule] & 0b111) { // DTP, DTDIR (DTS)
			case 0b000: m_dmacr |= 0b01000 << (submodule ? 0 : 8); break; // ITU channel 0
			case 0b001: m_dmacr |= 0b01001 << (submodule ? 0 : 8); break; // ITU channel 1
			case 0b010: m_dmacr |= 0b01010 << (submodule ? 0 : 8); break; // ITU channel 2
			case 0b011: m_dmacr |= 0b01011 << (submodule ? 0 : 8); break; // ITU channel 3
			//case 0b011: m_dmacr |= 0b10001 << (submodule ? 0 : 8); break; // A/D converter conversion end (H8/3006)
			case 0b100: m_dmacr |= 0b00100 << (submodule ? 0 : 8); break; // SCI channel 0 transmission data empty
			case 0b101: m_dmacr |= 0b10101 << (submodule ? 0 : 8); break; // SCI channel 0 receive data full
			case 0b110: m_dmacr |= 0b00010 << (submodule ? 0 : 8); break; // DREQ falling edge (B only)
			case 0b111: m_dmacr |= 0b00011 << (submodule ? 0 : 8); break; // DREQ low-level (B only)
			}
		}
	}

	set_bcr(fae, sae, dta, dte, dtie);

	start_test(-1);
}

void h8_dma_channel_device::set_bcr(bool fae, bool sae, uint8_t dta, uint8_t dte, uint8_t dtie)
{
	m_fae  = fae;
	m_sae  = sae;
	m_dta  = dta  & 3;
	m_dte  = dte  & 3;
	m_dtie = dtie & 3;
	logerror("fae=%d sae=%d dta=%d dte=%d dtie=%d\n", fae, sae, dta & 3, dte & 3, dtie & 3);
	start_test(-1);
}

bool h8_dma_channel_device::start_test(int vector)
{
	if(m_fae) {
		if(m_dte != 3)
			return false;

		if(m_dmacr & 0x0800) {
			throw emu_fatalerror("%s: DMA startup test in full address/block mode unimplemented.\n", tag());

		} else {
			// Normal Mode
			if(vector == -1) {
				start(0);
				return true;
			} else {
				// DREQ trigger
				if(((m_dmacr & 0b111) == 0b0010 && vector == DREQ_EDGE) || ((m_dmacr & 0b111) == 0b0011 && vector == DREQ_LEVEL)) {
					m_state[0].m_suspended = false;
					return true;
				}
			}
			return false;
		}
	} else {
		if(m_dte == 0)
			return false;

		if(vector == -1) {
			// A has priority over B
			if(m_dte & 2)
				start(1);
			if(m_dte & 1)
				start(0);
			return true;
		} else if(m_dte & 2) {
			// DREQ trigger (B only)
			if(((m_dmacr & 0b111) == 0b0010 && vector == DREQ_EDGE) || ((m_dmacr & 0b111) == 0b0011 && vector == DREQ_LEVEL)) {
				m_state[1].m_suspended = false;
				return true;
			}
		}
		return false;
	}
}

void h8_dma_channel_device::start(int submodule)
{
	if(m_fae) {
		if(m_dmacr & 0x0800)
			throw emu_fatalerror("%s: DMA start in full address/block mode unimplemented.\n", tag());
		else {
			m_state[submodule].m_source = m_mar[0];
			m_state[submodule].m_dest = m_mar[1];
			m_state[submodule].m_count = m_etcr[0] ? m_etcr[0] : 0x10000;
			m_state[submodule].m_mode_16 = m_dmacr & 0x8000;
			m_state[submodule].m_autoreq = (m_dmacr & 6) == 6 || (m_dmacr & 7) == 3; // autoreq or dreq level
			m_state[submodule].m_suspended = (m_dmacr & 6) != 6; // non-auto-request transfers start suspended
			int32_t step = m_state[submodule].m_mode_16 ? 2 : 1;
			m_state[submodule].m_incs = m_dmacr & 0x2000 ? m_dmacr & 0x4000 ? -step : step : 0;
			m_state[submodule].m_incd = m_dmacr & 0x0020 ? m_dmacr & 0x0040 ? -step : step : 0;
			m_cpu->set_current_dma(m_state + submodule);
		}
	} else {
		uint8_t cr = submodule ? m_dmacr & 0x00ff : m_dmacr >> 8;
		m_state[submodule].m_mode_16 = cr & 0x80;
		m_state[submodule].m_autoreq = false;
		m_state[submodule].m_suspended = true;
		int32_t step = m_state[submodule].m_mode_16 ? 2 : 1;
		if(!(cr & 0x20)) {
			// Sequential mode
			m_state[submodule].m_count = m_etcr[submodule] ? m_etcr[submodule] : 0x10000;
			m_state[submodule].m_incs = cr & 0x40 ? -step : step;
		} else if(m_dtie & (1 << submodule)) {
			// Idle mode
			m_state[submodule].m_count = m_etcr[submodule] ? m_etcr[submodule] : 0x10000;
			m_state[submodule].m_incs = 0;
		} else {
			// Repeat mode
			m_state[submodule].m_count = m_etcr[submodule] & 0x00ff ? m_etcr[submodule] & 0x00ff : 0x100;
			m_state[submodule].m_incs = cr & 0x40 ? -step : step;
		}
		if(cr & 0x10) {
			m_state[submodule].m_source = 0xff0000 | m_ioar[submodule];
			m_state[submodule].m_dest = m_mar[submodule];
			m_state[submodule].m_incd = m_state[submodule].m_incs;
			m_state[submodule].m_incs = 0;
		} else {
			m_state[submodule].m_source = m_mar[submodule];
			m_state[submodule].m_dest = 0xff0000 | m_ioar[submodule];
			m_state[submodule].m_incd = 0;
		}
		m_cpu->set_current_dma(m_state + submodule);
	}
}

void h8_dma_channel_device::count_last(int submodule)
{
	logerror("count last on %d\n", submodule);
	if(!m_state[submodule].m_autoreq) // "The TEND signal goes low during the last write cycle."
		m_cpu->set_input_line(H8_INPUT_LINE_TEND0 + (m_state[submodule].m_id >> 1), ASSERT_LINE);
}

void h8_dma_channel_device::count_done(int submodule)
{
	if(!m_state[submodule].m_autoreq)
		m_cpu->set_input_line(H8_INPUT_LINE_TEND0 + (m_state[submodule].m_id >> 1), CLEAR_LINE);
	if(m_fae) {
		if(m_dmacr & 0x0800)
			throw emu_fatalerror("%s: DMA count done full address/block mode unimplemented.\n", tag());
		else {
			m_dte &= ~1;
			m_dma->clear_dte(m_state[0].m_id);
			m_dtcr[0] &= ~0x80; // clear DTE (for H8H)
			if(m_dtie & 1)
				m_intc->internal_interrupt(m_irq_base + submodule);
		}
	} else {
		uint8_t cr = submodule ? m_dmacr & 0x00ff : m_dmacr >> 8;
		if((cr & 0x20) && !(m_dtie & (1 << submodule))) {
			// Repeat mode
			m_state[submodule].m_count = m_etcr[submodule] & 0x00ff ? m_etcr[submodule] & 0x00ff : 0x100;
			if(cr & 0x10)
				m_state[submodule].m_dest = m_mar[submodule];
			else
				m_state[submodule].m_source = m_mar[submodule];
		} else {
			m_dte &= ~(1 << submodule);
			m_dma->clear_dte(m_state[0].m_id + submodule);
			m_dtcr[submodule] &= ~0x80; // clear DTE (for H8H)
			if(m_dtie & (1 << submodule))
				m_intc->internal_interrupt(m_irq_base + submodule);
		}
	}
}
