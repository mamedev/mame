// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "h8_dma.h"

/*
    h:
  mar[01][ab][hl]
  ioar[01][ab]
  etcr[01][ab]
  dtcr[01][ab]

    s:
  mar[01][ab][hl]
  ioar[01][ab]
  etcr[01][ab]
  dmawer
  dmatcr
  dmacr[01][ab]
  dmabcr[hl]

 */

DEFINE_DEVICE_TYPE(H8H_DMA,         h8h_dma_device,         "h8h_dma",         "H8H DMA controller")
DEFINE_DEVICE_TYPE(H8S_DMA,         h8s_dma_device,         "h8s_dma",         "H8S DMA controller")
DEFINE_DEVICE_TYPE(H8H_DMA_CHANNEL, h8h_dma_channel_device, "h8h_dma_channel", "H8H DMA channel")
DEFINE_DEVICE_TYPE(H8S_DMA_CHANNEL, h8s_dma_channel_device, "h8s_dma_channel", "H8S DMA channel")


// H8 top device, common code

h8gen_dma_device::h8gen_dma_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_dmach(*this, "%u", 0)
{
}

void h8gen_dma_device::device_start()
{
	for(int i=0; i != 4; i++)
		if(m_dmach[i])
			m_dmach[i]->set_id(i<<1);
}

void h8gen_dma_device::device_reset()
{
}

void h8gen_dma_device::count_last(int id)
{
	m_cpu->set_input_line(H8_INPUT_LINE_TEND0 + (id >> 1), ASSERT_LINE);
}

void h8gen_dma_device::count_done(int id)
{
	m_cpu->set_input_line(H8_INPUT_LINE_TEND0 + (id >> 1), CLEAR_LINE);
	m_dmach[id >> 1]->count_done(id & 1);
}

void h8gen_dma_device::set_input(int inputnum, int state)
{
	if(inputnum >= H8_INPUT_LINE_DREQ0 && inputnum <= H8_INPUT_LINE_DREQ3) {
		int idx = inputnum - H8_INPUT_LINE_DREQ0;
		if(m_dmach[idx])
			m_dmach[idx]->set_dreq(state);
	}
}

void h8gen_dma_device::start_stop_test()
{
	u8 chnmap = active_channels();
	for(int i=0; i != 8; i++) {
		if(BIT(chnmap, i)) {
			if(!(m_dmach[i >> 1]->m_state[i & 1].m_flags & h8_dma_state::ACTIVE))
				m_dmach[i >> 1]->start(i & 1);

		} else {
			if(m_dmach[i >> 1] && (m_dmach[i >> 1]->m_state[i & 1].m_flags & h8_dma_state::ACTIVE)) {
				logerror("forced abort %d\n", i);
				m_dmach[i >> 1]->abort(i & 1);
			}
		}
	}
}



// DMA channel, common code

h8gen_dma_channel_device::h8gen_dma_channel_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_intc(*this, finder_base::DUMMY_TAG)
{
}

void h8gen_dma_channel_device::device_start()
{
	save_item(STRUCT_MEMBER(m_state, m_source));
	save_item(STRUCT_MEMBER(m_state, m_dest));
	save_item(STRUCT_MEMBER(m_state, m_incs));
	save_item(STRUCT_MEMBER(m_state, m_incd));
	save_item(STRUCT_MEMBER(m_state, m_count));
	save_item(STRUCT_MEMBER(m_state, m_bcount));
	save_item(STRUCT_MEMBER(m_state, m_flags));
	save_item(STRUCT_MEMBER(m_state, m_id));
	save_item(STRUCT_MEMBER(m_state, m_trigger_vector));

	save_item(NAME(m_mar));
	save_item(NAME(m_ioar));
	save_item(NAME(m_etcr));
	save_item(NAME(m_dreq));
}

void h8gen_dma_channel_device::device_reset()
{
	int base_id = m_state[0].m_id;
	memset(m_state, 0, sizeof(m_state));
	m_state[0].m_id = base_id;
	m_state[1].m_id = base_id+1;

	m_mar[0] = m_mar[1] = 0;
	m_ioar[0] = m_ioar[1] = 0;
	m_etcr[0] = m_etcr[1] = 0;
	m_dreq = false;
}

void h8gen_dma_channel_device::set_id(int id)
{
	for(int i=0; i != 2; i++) {
		m_state[i].m_id = id | i;
		m_cpu->set_dma_channel(m_state + i);
	}
}

void h8gen_dma_channel_device::set_dreq(int state)
{
	if(m_dreq == state)
		return;

	m_dreq = state;

	// Only subchannel B/1 can react to dreq.
	if(m_dreq) {
		if(((m_state[1].m_flags & (h8_dma_state::ACTIVE|h8_dma_state::SUSPENDED)) == (h8_dma_state::ACTIVE|h8_dma_state::SUSPENDED)) && (m_state[1].m_trigger_vector == DREQ_LEVEL || m_state[1].m_trigger_vector == DREQ_EDGE)) {
			m_state[1].m_flags &= ~h8_dma_state::SUSPENDED;
			m_cpu->update_active_dma_channel();
		}
	} else {
		if(((m_state[1].m_flags & (h8_dma_state::ACTIVE|h8_dma_state::SUSPENDED)) == h8_dma_state::ACTIVE) && m_state[1].m_trigger_vector == DREQ_LEVEL) {
			m_state[1].m_flags |= h8_dma_state::SUSPENDED;
			m_cpu->update_active_dma_channel();
		}
	}
}

u16 h8gen_dma_channel_device::marah_r()
{
	logerror("marah_r %06x\n", m_mar[0]);
	return m_mar[0] >> 16;
}

void h8gen_dma_channel_device::marah_w(offs_t offset, u16 data, u16 mem_mask)
{
	if(ACCESSING_BITS_0_7)
		m_mar[0] = ((data & 0x00ff) << 16) | (m_mar[0] & 0xffff);
	logerror("marah_w %06x\n", m_mar[0]);
}

u16 h8gen_dma_channel_device::maral_r()
{
	logerror("maral_r %06x\n", m_mar[0]);
	return m_mar[0];
}

void h8gen_dma_channel_device::maral_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_mar[0] = (m_mar[0] & ~mem_mask) | (data & mem_mask);
	logerror("maral_w %06x\n", m_mar[0]);
}

u16 h8gen_dma_channel_device::ioara_r()
{
	return m_ioar[0];
}

u8 h8gen_dma_channel_device::ioara8_r()
{
	return m_ioar[0];
}

void h8gen_dma_channel_device::ioara_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_ioar[0]);
	m_ioar[0] &= ~m_ioar_mask;
	logerror("ioara_w %04x\n", m_ioar[0]);
}

void h8gen_dma_channel_device::ioara8_w(u8 data)
{
	m_ioar[0] = data;
	logerror("ioara_w %02x\n", m_ioar[0]);
}

u16 h8gen_dma_channel_device::etcra_r()
{
	logerror("etcra_r %04x\n", m_etcr[0]);
	return m_etcr[0];
}

void h8gen_dma_channel_device::etcra_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_etcr[0]);
	logerror("etcra_w %04x\n", m_etcr[0]);
}

u16 h8gen_dma_channel_device::marbh_r()
{
	logerror("marbh_r %06x\n", m_mar[1]);
	return m_mar[1] >> 16;
}

void h8gen_dma_channel_device::marbh_w(offs_t offset, u16 data, u16 mem_mask)
{
	if(ACCESSING_BITS_0_7)
		m_mar[1] = ((data & 0x00ff) << 16) | (m_mar[1] & 0xffff);
	logerror("marbh_w %06x\n", m_mar[1]);
}

u16 h8gen_dma_channel_device::marbl_r()
{
	logerror("marbl_r %06x\n", m_mar[1]);
	return m_mar[1];
}

void h8gen_dma_channel_device::marbl_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_mar[1] = (m_mar[1] & ~mem_mask) | (data & mem_mask);
	logerror("marbl_w %06x\n", m_mar[1]);
}

u16 h8gen_dma_channel_device::ioarb_r()
{
	return m_ioar[1];
}

u8 h8gen_dma_channel_device::ioarb8_r()
{
	return m_ioar[1];
}

void h8gen_dma_channel_device::ioarb_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_ioar[1]);
	m_ioar[1] &= ~m_ioar_mask;
	logerror("ioarb_w %04x\n", m_ioar[1]);
}

void h8gen_dma_channel_device::ioarb8_w(u8 data)
{
	m_ioar[1] = data;
	logerror("ioarb_w %02x\n", m_ioar[1]);
}

u16 h8gen_dma_channel_device::etcrb_r()
{
	logerror("etcrb_r %04x\n", m_etcr[1]);
	return m_etcr[1];
}

void h8gen_dma_channel_device::etcrb_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_etcr[1]);
	logerror("etcrb_w %04x\n", m_etcr[1]);
}

void h8gen_dma_channel_device::start(int submodule)
{
	int mode = channel_mode();
	s8 vector = trigger_vector(submodule);

	m_state[submodule].m_flags = h8_dma_state::ACTIVE | channel_flags(submodule);
	m_state[submodule].m_trigger_vector = vector;

	if(mode == FAE_NORMAL || mode == FAE_BLOCK) {
		m_state[submodule].m_source = m_mar[0];
		m_state[submodule].m_dest = m_mar[1];

	} else {
		if(m_state[submodule].m_flags & h8_dma_state::MAR_IS_DEST) {
			m_state[submodule].m_source = mode == SAE_DACK ? (0x80000000 | (m_state[submodule].m_id >> 1)) : (m_ioar[submodule] | m_ioar_mask);
			m_state[submodule].m_dest = m_mar[submodule];
		} else {
			m_state[submodule].m_source = m_mar[submodule];
			m_state[submodule].m_dest = mode == SAE_DACK ? (0x80000000 | (m_state[submodule].m_id >> 1)) : (m_ioar[submodule] | m_ioar_mask);
		}
	}

	m_state[submodule].m_bcount = 0;

	if(mode == FAE_BLOCK) {
		m_state[submodule].m_count = m_etcr[0] & 0xff ? m_etcr[0] & 0xff : 0x100;
		m_state[submodule].m_bcount = m_etcr[1] ? m_etcr[1] : 0x10000;
		if(m_state[submodule].m_bcount > 1)
			m_state[submodule].m_flags |= h8_dma_state::BLOCK;

	} else if(m_state[submodule].m_flags & h8_dma_state::REPEAT)
		m_state[submodule].m_count = m_etcr[0] & 0xff ? m_etcr[0] & 0xff : 0x100;
	else
		m_state[submodule].m_count = m_etcr[0] ? m_etcr[0] : 0x10000;

	if(!(vector == AUTOREQ_CS || vector == AUTOREQ_B || (vector == DREQ_LEVEL && m_dreq)))
		m_state[submodule].m_flags |= h8_dma_state::SUSPENDED;
	if(!(vector == AUTOREQ_CS || vector == AUTOREQ_B || vector == DREQ_LEVEL || mode == FAE_BLOCK))
		m_state[submodule].m_flags |= h8_dma_state::SUSPEND_AFTER_TRANSFER;

	int step = m_state[submodule].m_flags & h8_dma_state::MODE_16 ? 2 : 1;

	m_state[submodule].m_incs = m_state[submodule].m_flags & h8_dma_state::SOURCE_IDLE ? 0 :  m_state[submodule].m_flags & h8_dma_state::SOURCE_DECREMENT ? -step : step;
	m_state[submodule].m_incd = m_state[submodule].m_flags & h8_dma_state::DEST_IDLE ? 0 :  m_state[submodule].m_flags & h8_dma_state::DEST_DECREMENT ? -step : step;

	logerror("%c: setup src=%s%s dst=%s%s count=%x bcount=%x trigger=%s%s%s%s%s%s%s%s%s\n",
			 'A' + submodule,
			 m_state[submodule].m_source & 0x80000000 ? util::string_format("dack%d", m_state[submodule].m_source & 1) : util::string_format("%06x", m_state[submodule].m_source),
			 m_state[submodule].m_incs > 0 ? util::string_format("+%x", m_state[submodule].m_incs) : m_state[submodule].m_incs < 0 ? util::string_format("-%x", -m_state[submodule].m_incs) : "",
			 m_state[submodule].m_dest & 0x80000000 ? util::string_format("dack%d", m_state[submodule].m_dest & 1) : util::string_format("%06x", m_state[submodule].m_dest),
			 m_state[submodule].m_incd > 0 ? util::string_format("+%x", m_state[submodule].m_incd) : m_state[submodule].m_incd < 0 ? util::string_format("-%x", -m_state[submodule].m_incd) : "",
			 m_state[submodule].m_count,
			 m_state[submodule].m_bcount,
			 vector == AUTOREQ_CS ? "autoreq-cycle-steal" : vector == AUTOREQ_B ? "autoreq-burst" : vector == DREQ_LEVEL ? "dreq-level" : vector == DREQ_EDGE ? "dreq-edge" : util::string_format("%d", vector),
			 m_state[submodule].m_flags & h8_dma_state::SUSPENDED ? " suspended" : "",
			 m_state[submodule].m_flags & h8_dma_state::SUSPEND_AFTER_TRANSFER ? " suspend-after-transfer" : "",
			 m_state[submodule].m_flags & h8_dma_state::BLOCK ? " block" : "",
			 m_state[submodule].m_flags & h8_dma_state::REPEAT ? " repeat" : "",
			 m_state[submodule].m_flags & h8_dma_state::MODE_16 ? " word" : " byte",
			 m_state[submodule].m_flags & h8_dma_state::MAR_IS_DEST ? " mar-is-dest" : " ",
			 m_state[submodule].m_flags & h8_dma_state::FAE ? " fae" : "",
			 m_state[submodule].m_flags & h8_dma_state::EAT_INTERRUPT ? " eat-interrupt" : "",
			 m_state[submodule].m_flags & h8_dma_state::TEND_INTERRUPT ? " tend-interrupt" : "");
}

void h8gen_dma_channel_device::dma_done(int submodule)
{
	m_state[submodule].m_flags &= ~h8_dma_state::ACTIVE;
	m_cpu->update_active_dma_channel();
	if(m_state[submodule].m_flags & h8_dma_state::TEND_INTERRUPT)
		m_intc->internal_interrupt(m_irq_base + (m_state[submodule].m_flags & h8_dma_state::FAE ? m_state[0].m_id : m_state[submodule].m_id));
}

void h8gen_dma_channel_device::count_done(int submodule)
{
	if(m_state[submodule].m_flags & h8_dma_state::BLOCK) {
		if(m_state[submodule].m_flags & h8_dma_state::MAR_IS_DEST)
			m_state[submodule].m_dest = m_mar[1];
		else
			m_state[submodule].m_source = m_mar[0];
		m_state[submodule].m_count = m_etcr[0] & 0xff00 ? m_etcr[0] >> 8 : 0x100;

		m_state[submodule].m_bcount--;
		if(m_state[submodule].m_bcount == 1)
			m_state[submodule].m_flags &= ~h8_dma_state::BLOCK;

		if(m_state[submodule].m_trigger_vector != DREQ_LEVEL) {
			m_state[submodule].m_flags |= h8_dma_state::SUSPENDED;
			m_cpu->update_active_dma_channel();
		}

	} else if(m_state[submodule].m_flags & h8_dma_state::REPEAT) {
		if(m_state[submodule].m_flags & h8_dma_state::MAR_IS_DEST)
			m_state[submodule].m_dest = m_mar[submodule];
		else
			m_state[submodule].m_source = m_mar[submodule];
		m_state[submodule].m_count = m_etcr[submodule] & 0xff ? m_etcr[submodule] & 0xff : 0x100;

	} else
		dma_done(submodule);
}


// H8H top device specifics

h8h_dma_device::h8h_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8gen_dma_device(mconfig, H8H_DMA, tag, owner, clock)
{
}

u8 h8h_dma_device::active_channels() const
{
	u8 res = 0;
	for(int i=0; i != 4; i++)
		if(m_dmach[i])
			res |= downcast<h8h_dma_channel_device *>(m_dmach[i].target())->active_channels() << (2*i);
	return res;
}


// H8S top device specifics

h8s_dma_device::h8s_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8gen_dma_device(mconfig, H8S_DMA, tag, owner, clock)
{
}

void h8s_dma_device::device_start()
{
	h8gen_dma_device::device_start();
	save_item(NAME(m_dmabcr));
	save_item(NAME(m_dmatcr));
	save_item(NAME(m_dmawer));
}

void h8s_dma_device::device_reset()
{
	h8gen_dma_device::device_reset();
	m_dmabcr = 0x0000;
	m_dmatcr = 0x00;
	m_dmawer = 0x00;
}

u8 h8s_dma_device::active_channels() const
{
	u8 res = 0;
	for(int i=0; i != 2; i++)
		if(BIT(m_dmabcr, 14+i)) {
			if(((m_dmabcr >> (4+2*i)) & 3) == 3)
				res |= 2 << (2*i);
		} else {
			if(BIT(m_dmabcr, 4+2*i))
				res |= 1 << (2*i);
			if(BIT(m_dmabcr, 5+2*i))
				res |= 2 << (2*i);
		}
	return res;
}


u8 h8s_dma_device::dmawer_r()
{
	logerror("dmawer_r %02x\n", m_dmawer);
	return m_dmawer;
}

void h8s_dma_device::dmawer_w(u8 data)
{
	m_dmawer = data;
	logerror("dmawer_w %02x\n", data);
}

u8 h8s_dma_device::dmatcr_r()
{
	logerror("dmatcr_r %02x\n", m_dmatcr);
	return m_dmatcr;
}

void h8s_dma_device::dmatcr_w(u8 data)
{
	m_dmatcr = data;
	logerror("dmatcr_w %02x\n", data);
}

u16 h8s_dma_device::dmabcr_r()
{
	logerror("dmabcr_r %04x\n", m_dmabcr);
	return m_dmabcr;
}

void h8s_dma_device::dmabcr_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_dmabcr);
	logerror("dmabcr_w %04x\n", m_dmabcr);
	start_stop_test();
}

void h8s_dma_device::channel_done(int id)
{
	m_dmabcr &= ~(0x0010 << id);
}

int h8s_dma_device::channel_mode(int id, bool block) const
{
	if(BIT(m_dmabcr, 14+id)) {
		// fae mode
		return block ? h8h_dma_channel_device::FAE_BLOCK : h8h_dma_channel_device::FAE_NORMAL;

	} else {
		// sae mode
		return BIT(m_dmabcr, 12+id) ? h8h_dma_channel_device::SAE_DACK : h8h_dma_channel_device::SAE;
	}
}

 std::tuple<bool, bool, bool> h8s_dma_device::get_fae_dtie_dta(int id) const
{
	bool fae = BIT(m_dmabcr, 14+(id >> 1));
	bool dtie = BIT(m_dmabcr, id);
	bool dta = BIT(m_dmabcr, 8+id);
	return std::tie(fae, dtie, dta);
}


// H8H channels specifics

h8h_dma_channel_device::h8h_dma_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8gen_dma_channel_device(mconfig, H8H_DMA_CHANNEL, tag, owner, clock),
	m_dma(*this, finder_base::DUMMY_TAG)
{
	m_irq_base = 44;
	m_ioar_mask = 0xffff00;
}

void h8h_dma_channel_device::device_start()
{
	h8gen_dma_channel_device::device_start();
	save_item(NAME(m_dtcr));
}

void h8h_dma_channel_device::device_reset()
{
	h8gen_dma_channel_device::device_reset();
	m_dtcr[0] = m_dtcr[1] = 0;
}

u8 h8h_dma_channel_device::dtcra_r()
{
	logerror("dtcra_r %02x\n", m_dtcr[0]);
	return m_dtcr[0];
}

void h8h_dma_channel_device::dtcra_w(u8 data)
{
	m_dtcr[0] = data;
	logerror("dtcra_w %02x\n", m_dtcr[0]);
	m_dma->start_stop_test();
}

u8 h8h_dma_channel_device::dtcrb_r()
{
	logerror("dtcrb_r %02x\n", m_dtcr[1]);
	return m_dtcr[1];
}

void h8h_dma_channel_device::dtcrb_w(u8 data)
{
	m_dtcr[1] = data;
	logerror("dtcrb_w %02x\n", m_dtcr[1]);
	m_dma->start_stop_test();
}

void h8h_dma_channel_device::dma_done(int submodule)
{
	m_dtcr[submodule] &= ~0x80;
	h8gen_dma_channel_device::dma_done(submodule);
}

int h8h_dma_channel_device::channel_mode() const
{
	switch(m_dtcr[0] & 7) {
	case 6: return FAE_NORMAL;
	case 7: return FAE_BLOCK;
	default: return SAE;
	}
}

u16 h8h_dma_channel_device::channel_flags(int submodule) const
{
	u16 res = h8_dma_state::EAT_INTERRUPT;

	if((m_dtcr[0] & 6) == 6) {
		// FAE mode, expect submodule==1
		res |= h8_dma_state::FAE;

		if(BIT(m_dtcr[0], 6))
			res |= h8_dma_state::MODE_16;
		if(BIT(m_dtcr[0], 3))
			res |= h8_dma_state::TEND_INTERRUPT;

		if(!BIT(m_dtcr[0], 4))
			res |= h8_dma_state::SOURCE_IDLE;
		else if(BIT(m_dtcr[0], 5))
			res |= h8_dma_state::SOURCE_DECREMENT;

		if(!BIT(m_dtcr[1], 4))
			res |= h8_dma_state::DEST_IDLE;
		else if(BIT(m_dtcr[1], 5))
			res |= h8_dma_state::DEST_DECREMENT;

		if(!BIT(m_dtcr[1], 3))
			res |= h8_dma_state::MAR_IS_DEST;

	} else {
		if(BIT(m_dtcr[submodule], 6))
			res |= h8_dma_state::MODE_16;
		if(BIT(m_dtcr[submodule], 3))
			res |= h8_dma_state::TEND_INTERRUPT;

		int vector = trigger_vector(submodule);
		if(!(vector == 23 || vector >= 52)) // adc & sci
			res |= h8_dma_state::MAR_IS_DEST | h8_dma_state::SOURCE_IDLE;
		else
			res |= h8_dma_state::DEST_IDLE;

		if(BIT(m_dtcr[submodule], 5))
			res |= (res & h8_dma_state::MAR_IS_DEST) ? h8_dma_state::DEST_DECREMENT : h8_dma_state::SOURCE_DECREMENT;

		if(BIT(m_dtcr[submodule], 4)) {
			if(BIT(m_dtcr[submodule], 3))
				res |= h8_dma_state::DEST_IDLE | h8_dma_state::SOURCE_IDLE;
			else
				res |= h8_dma_state::REPEAT;
		}
	}
	return res;
}

u8 h8h_dma_channel_device::active_channels() const
{
	u8 res = 0;
	if((m_dtcr[0] & 6) == 6) {
		if(BIT(m_dtcr[0], 7) && BIT(m_dtcr[1], 7))
			res |= 2;
	} else {
		if(BIT(m_dtcr[0], 7))
			res |= 1;
		if(BIT(m_dtcr[1], 7))
			res |= 2;
	}
	return res;
}

s8 h8h_dma_channel_device::trigger_vector(int submodule) const
{
	static const s8 faen[8] = { AUTOREQ_B, NONE, AUTOREQ_CS, NONE, NONE, NONE, DREQ_EDGE, DREQ_LEVEL };
	static const s8 faeb[8] = { 24, 28, 32, 36, NONE, NONE, DREQ_EDGE, NONE };
	static const s8 sae[8]  = { 24, 28, 32, 36, 54, 53, DREQ_EDGE, DREQ_LEVEL };
	s8 vector = NONE;
	switch(channel_mode()) {
	case FAE_NORMAL: vector = faen[m_dtcr[submodule] & 7]; break;
	case FAE_BLOCK:  vector = faeb[m_dtcr[submodule] & 7]; break;
	case SAE:        vector = sae [m_dtcr[submodule] & 7]; break;
	}

	if(m_has_adc && vector == 36)
		vector = 23;
	if(m_targets_sci1 && (vector == 53 || vector == 54))
		vector += 4;
	return vector;
}



// H8S channels specifics

h8s_dma_channel_device::h8s_dma_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h8gen_dma_channel_device(mconfig, H8S_DMA_CHANNEL, tag, owner, clock),
	m_dma(*this, finder_base::DUMMY_TAG)
{
	m_irq_base = 72;
	m_ioar_mask = 0xff0000;
}

void h8s_dma_channel_device::device_start()
{
	h8gen_dma_channel_device::device_start();
	save_item(NAME(m_dmacr));
}

void h8s_dma_channel_device::device_reset()
{
	h8gen_dma_channel_device::device_reset();
	m_dmacr = 0;
}

u16 h8s_dma_channel_device::dmacr_r()
{
	logerror("dmacr_r %04x\n", m_dmacr);
	return m_dmacr;
}

void h8s_dma_channel_device::dmacr_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_dmacr);
	logerror("dmacr_w %04x\n", m_dmacr);
}

void h8s_dma_channel_device::dma_done(int submodule)
{
	m_dma->channel_done(m_state[submodule].m_id);
	h8gen_dma_channel_device::dma_done(submodule);
}

int h8s_dma_channel_device::channel_mode() const
{
	return m_dma->channel_mode(m_state[0].m_id >> 1, BIT(m_dmacr, 11));
}

u16 h8s_dma_channel_device::channel_flags(int submodule) const
{
	auto [fae, dtie, dta] = m_dma->get_fae_dtie_dta(m_state[submodule].m_id);
	u16 res = 0;

	if(fae) {
		// FAE mode, expect submodule==1
		res |= h8_dma_state::FAE;

		if(BIT(m_dmacr, 15))
			res |= h8_dma_state::MODE_16;
		if(!BIT(m_dmacr, 13))
			res |= h8_dma_state::SOURCE_IDLE;
		else if(!BIT(m_dmacr, 14))
			res |= h8_dma_state::SOURCE_DECREMENT;

		if(!BIT(m_dmacr, 5))
			res |= h8_dma_state::DEST_IDLE;
		else if(!BIT(m_dmacr, 6))
			res |= h8_dma_state::DEST_DECREMENT;

		if(!BIT(m_dmacr, 12))
			res |= h8_dma_state::MAR_IS_DEST;

	} else {
		u8 cr = submodule ? m_dmacr : m_dmacr >> 8;
		if(BIT(cr, 7))
			res |= h8_dma_state::MODE_16;

		if(BIT(cr, 4))
			res |= h8_dma_state::MAR_IS_DEST | h8_dma_state::SOURCE_IDLE;
		else
			res |= h8_dma_state::DEST_IDLE;

		if(BIT(cr, 6))
			res |= (res & h8_dma_state::MAR_IS_DEST) ? h8_dma_state::DEST_DECREMENT : h8_dma_state::SOURCE_DECREMENT;

		if(BIT(cr, 5)) {
			if(dtie)
				res |= h8_dma_state::DEST_IDLE | h8_dma_state::SOURCE_IDLE;
			else
				res |= h8_dma_state::REPEAT;
		}
	}

	if(dtie)
		res |= h8_dma_state::TEND_INTERRUPT;
	if(dta)
		res |= h8_dma_state::EAT_INTERRUPT;

	return res;
}

s8 h8s_dma_channel_device::trigger_vector(int submodule) const
{
	static const s8 vectors[0x10] = { NONE, 28, DREQ_EDGE, DREQ_LEVEL, 82, 81, 86, 85, 32, 40, 44, 48, 56, 60, NONE, NONE };
	static const s8 vectorsn[0x10] = { NONE, NONE, DREQ_EDGE, DREQ_LEVEL, NONE, NONE, AUTOREQ_CS, AUTOREQ_B, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE };

	if(submodule) {
		// fae normal mode has a special table
		if(channel_mode() == FAE_NORMAL)
			return vectorsn[m_dmacr & 15];
		else
			return vectors[m_dmacr & 15];

	} else {
		s8 vector = vectors[(m_dmacr >> 8) & 15];
		// subchannel A doesn't do dreq
		if(vector < NONE)
			vector = NONE;
		return vector;
	}
}
