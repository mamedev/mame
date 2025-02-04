// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "h8_dtc.h"

#include "h8.h"

// Verbosity level
// 0 = no messages
// 1 = in-memory registers once read
// 2 = everything
static constexpr int V = 0;

DEFINE_DEVICE_TYPE(H8_DTC, h8_dtc_device, "h8_dtc", "H8 DTC controller")

const int h8_dtc_device::vector_to_enable[92] = {
	-1, -1, -1, -1, -1, -1, -1, -1, // NMI at 7
	-1, -1, -1, -1, -1, -1, -1, -1,
	0,   1,  2,  3,  4,  5,  6,  7, // IRQ 0-7
	-1, -1, -1, -1, 9,  -1, -1, -1, // SWDTEND, WOVI, CMI, (reserved), ADI
	10, 11, 12, 13, -1, -1, -1, -1, // TGI0A, TGI0B, TGI0C, TGI0D, TGI0V
	14, 15, -1, -1, 16, 17, -1, -1, // TGI1A, TGI1B, TGI1V, TGI1U, TGI2A, TGI2B, TGI2V, TGI2U
	-1, -1, -1, -1, -1, -1, -1, -1, // TGI3A, TGI3B, TGI3C, TGI3D, TGI3V
	-1, -1, -1, -1, -1, -1, -1, -1, // TGI4A, TGI4B, TGI4V, TGI4U, TGI5A, TGI5B, TGI5V, TGI5U
	28, 29, -1, -1, 30, 31, -1, -1, // CMIA0, CMIB0, OVI0, CMIA1, CMIB1, OVI1
	-1, -1, -1, -1, -1, -1, -1, -1, // DEND0A, DEND0B, DEND1B, DEND1B
	-1, 36, 37, -1, -1, 38, 39, -1, // ERI0, RXI0, TXI0, TEI0, ERI1, RXI1, TXI1, TEI1
	-1, 40, 41, -1                  // ERI2, RXI2, TXI2, TEI2
};

h8_dtc_device::h8_dtc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, H8_DTC, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_intc(*this, finder_base::DUMMY_TAG)
{
}

void h8_dtc_device::device_start()
{
	// TODO, probably need to kill the vectors
	save_item(STRUCT_MEMBER(m_states, m_base));
	save_item(STRUCT_MEMBER(m_states, m_sra));
	save_item(STRUCT_MEMBER(m_states, m_dar));
	save_item(STRUCT_MEMBER(m_states, m_cr));
	save_item(STRUCT_MEMBER(m_states, m_incs));
	save_item(STRUCT_MEMBER(m_states, m_incd));
	save_item(STRUCT_MEMBER(m_states, m_count));
	save_item(STRUCT_MEMBER(m_states, m_id));
	save_item(STRUCT_MEMBER(m_states, m_next));

	save_item(NAME(m_dtcer));
	save_item(NAME(m_dtvecr));
	save_item(NAME(m_cur_active_vector));
}

void h8_dtc_device::device_reset()
{
	memset(m_dtcer, 0x00, sizeof(m_dtcer));
	memset(m_states, 0, sizeof(m_states));
	for(u8 i=0; i<sizeof(m_states)/sizeof(m_states[0]); i++)
		m_states[i].m_id = i;
	m_dtvecr = 0x00;
	m_cur_active_vector = -1;
}

u8 h8_dtc_device::dtcer_r(offs_t offset)
{
	if(V>=2) logerror("dtcer_r %x, %02x\n", offset, m_dtcer[offset]);
	return m_dtcer[offset];
}

void h8_dtc_device::dtcer_w(offs_t offset, u8 data)
{
	m_dtcer[offset] = data;
	if(V>=2) logerror("dtcer_w %x, %02x\n", offset, data);
}

u8 h8_dtc_device::dtvecr_r()
{
	if(V>=2) logerror("dtvecr_r %02x\n", m_dtvecr);
	return m_dtvecr;
}

void h8_dtc_device::dtvecr_w(u8 data)
{
	m_dtvecr = data;
	if(V>=2) logerror("dtvecr_w %02x\n", data);
}

void h8_dtc_device::edge(int vector)
{
	for(auto i : m_waiting_vector)
		if(i == vector)
			return;
	for(auto i : m_waiting_writeback)
		if(i == vector)
			return;
	if(m_waiting_vector.empty() && m_waiting_writeback.empty())
		m_cpu->request_state(h8_device::STATE_DTC_VECTOR);
	m_waiting_vector.push_back(vector);
}

int h8_dtc_device::get_waiting_vector()
{
	assert(!m_waiting_vector.empty());
	return m_waiting_vector.front();
}

int h8_dtc_device::get_waiting_writeback()
{
	assert(!m_waiting_writeback.empty());
	return m_waiting_writeback.front();
}

void h8_dtc_device::queue(int vector)
{
	int ps = -1;
	int cs = m_cur_active_vector;
	while(cs != -1 && cs < vector) {
		ps = cs;
		cs = m_states[cs].m_next;
	}

	m_states[vector].m_next = cs;
	if(ps == -1) {
		m_cur_active_vector = vector;
		m_cpu->set_current_dtc(&m_states[vector]);
	} else
		m_states[ps].m_next = vector;
}

void h8_dtc_device::vector_done(int vector)
{
	std::vector<int>::iterator wi;
	for(wi = m_waiting_vector.begin(); wi != m_waiting_vector.end() && *wi != vector && *wi != vector + DTC_CHAINED; ++wi) {};
	assert(wi != m_waiting_vector.end());
	m_waiting_vector.erase(wi);

	h8_dtc_state *state = m_states + vector;
	u32 sra = state->m_sra;
	u32 dar = state->m_dar;
	u32 cr = state->m_cr;

	u32 mode = sra & 0x0c000000;
	if(V>=1) logerror("regs at %08x sra=%08x dar=%08x cr=%08x %s mode\n", state->m_base, sra, dar, cr,
						mode == 0x00000000 || mode == 0x0c000000 ? "normal" : mode == 0x04000000 ? "repeat" : "block");
	state->m_incs = sra & 0x80000000 ?
		sra & 0x40000000 ? sra & 0x01000000 ? -2 : -1 :
		sra & 0x01000000 ? 2 : 1 :
		0;
	state->m_incd = sra & 0x20000000 ?
		sra & 0x10000000 ? sra & 0x01000000 ? -2 : -1 :
		sra & 0x01000000 ? 2 : 1 :
		0;

	switch(mode) {
	case 0x00000000: case 0x0c0000000:
		state->m_count = 1;
		break;
	case 0x04000000:
		state->m_count = 1;
		break;
	case 0x08000000:
		state->m_count = (cr >> 16) & 255;
		if(!state->m_count)
			state->m_count = 256;
		break;
	}

	queue(vector);

	if(!m_waiting_vector.empty())
		m_cpu->request_state(h8_device::STATE_DTC_VECTOR);
	else if(!m_waiting_writeback.empty())
		m_cpu->request_state(h8_device::STATE_DTC_WRITEBACK);

}

void h8_dtc_device::writeback_done(int vector)
{
	std::vector<int>::iterator wi;
	for(wi = m_waiting_writeback.begin(); wi != m_waiting_writeback.end() && *wi != vector; ++wi) {};
	assert(wi != m_waiting_writeback.end());
	m_waiting_writeback.erase(wi);


	h8_dtc_state *state = m_states + vector;
	bool done = false;
	switch(state->m_sra & 0x0c000000) {
	case 0x00000000: case 0x0c0000000:
		done = !(state->m_cr & 0xffff0000);
		break;

	case 0x04000000:
		break;

	case 0x08000000:
		done = !(state->m_cr & 0x0000ffff);
		break;
	}

	if(done && state->m_dar & 0x80000000) {
		m_cpu->request_state(h8_device::STATE_DTC_VECTOR);
		m_waiting_vector.push_back(vector + DTC_CHAINED);
		return;
	}

	if(done || (state->m_dar & 0x40000000)) {
		if(vector) {
			int slot = vector_to_enable[vector];
			assert(slot != -1);
			m_dtcer[slot >> 3] &= ~(0x01 << (7-(slot & 7)));
			m_intc->internal_interrupt(vector);
		} else {
			logerror("Software dtc done\n");
		}
	}

	if(!m_waiting_vector.empty())
		m_cpu->request_state(h8_device::STATE_DTC_VECTOR);
	else if(!m_waiting_writeback.empty())
		m_cpu->request_state(h8_device::STATE_DTC_WRITEBACK);
}

bool h8_dtc_device::trigger_dtc(int vector)
{
	int slot = vector_to_enable[vector];
	if(slot == -1)
		return false;
	if(m_dtcer[slot >> 3] & (0x01 << (7-(slot & 7)))) {
		edge(vector);
		return true;
	}
	return false;
}

void h8_dtc_device::count_done(int id)
{
	assert(m_cur_active_vector == id);
	m_cur_active_vector = m_states[id].m_next;
	if(m_cur_active_vector != -1)
		m_cpu->set_current_dtc(m_states + m_cur_active_vector);

	h8_dtc_state *state = m_states + id;
	switch(state->m_sra & 0x0c000000) {
	case 0x00000000: case 0x0c0000000:
		state->m_cr -= 0x00010000;
		break;

	case 0x04000000:
		state->m_cr = (state->m_cr & 0xff00ffff) | ((state->m_cr - 0x00010000) & 0x00ff0000);
		if(!(state->m_cr & 0x00ff0000)) {
			int cnt = (state->m_cr >> 24) & 0xff;
			if(!cnt)
				cnt = 256;
			if(state->m_sra & 0x02000000)
				state->m_sra = (state->m_sra & 0xff000000) | ((state->m_sra - cnt*state->m_incs) & 0xffffff);
			else
				state->m_dar = (state->m_dar & 0xff000000) | ((state->m_dar - cnt*state->m_incd) & 0xffffff);
			state->m_cr |= (state->m_cr >> 8) & 0x00ff0000;
		}
		break;

	case 0x08000000: {
		int cnt = (state->m_cr >> 16) & 0xff;
		if(!cnt)
			cnt = 256;
		if(state->m_sra & 0x02000000)
			state->m_sra = (state->m_sra & 0xff000000) | ((state->m_sra - cnt*state->m_incs) & 0xffffff);
		else
			state->m_dar = (state->m_dar & 0xff000000) | ((state->m_dar - cnt*state->m_incd) & 0xffffff);
		state->m_cr = (state->m_cr & 0xff000000) | ((state->m_cr >> 8) & 0x00ff0000) | ((state->m_cr - 0x00000001) & 0x0000ffff);
		break;
	}
	}
	if(m_waiting_vector.empty() && m_waiting_writeback.empty())
		m_cpu->request_state(h8_device::STATE_DTC_WRITEBACK);
	m_waiting_writeback.push_back(id);
}
