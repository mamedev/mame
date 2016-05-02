#include "emu.h"
#include "h8_dtc.h"

// Verbosity level
// 0 = no messages
// 1 = in-memory registers once read
// 2 = everything
const int V = 0;

const device_type H8_DTC         = &device_creator<h8_dtc_device>;

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

h8_dtc_device::h8_dtc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, H8_DTC, "H8 DTC controller", tag, owner, clock, "h8_dtc", __FILE__),
	cpu(*this, DEVICE_SELF_OWNER)
{
}

void h8_dtc_device::set_info(const char *_intc_tag, int _irq)
{
	intc_tag = _intc_tag;
	irq = _irq;
}

void h8_dtc_device::device_start()
{
	intc = siblingdevice<h8_intc_device>(intc_tag);
}

void h8_dtc_device::device_reset()
{
	memset(dtcer, 0x00, sizeof(dtcer));
	memset(states, 0, sizeof(states));
	for(UINT8 i=0; i<sizeof(states)/sizeof(states[0]); i++)
		states[i].id = i;
	dtvecr = 0x00;
	cur_active_vector = -1;
}

READ8_MEMBER(h8_dtc_device::dtcer_r)
{
	if(V>=2) logerror("dtcer_r %x, %02x\n", offset, dtcer[offset]);
	return dtcer[offset];
}

WRITE8_MEMBER(h8_dtc_device::dtcer_w)
{
	dtcer[offset] = data;
	if(V>=2) logerror("dtcer_w %x, %02x\n", offset, data);
}

READ8_MEMBER(h8_dtc_device::dtvecr_r)
{
	if(V>=2) logerror("dtvecr_r %02x\n", dtvecr);
	return dtvecr;
}

WRITE8_MEMBER(h8_dtc_device::dtvecr_w)
{
	dtvecr = data;
	if(V>=2) logerror("dtvecr_w %02x\n", data);
}

void h8_dtc_device::edge(int vector)
{
	for(std::list<int>::const_iterator i = waiting_vector.begin(); i != waiting_vector.end(); i++)
		if(*i == vector)
			return;
	for(std::list<int>::const_iterator i = waiting_writeback.begin(); i != waiting_writeback.end(); i++)
		if(*i == vector)
			return;
	if(waiting_vector.empty() && waiting_writeback.empty())
		cpu->request_state(h8_device::STATE_DTC_VECTOR);
	waiting_vector.push_back(vector);
}

int h8_dtc_device::get_waiting_vector()
{
	assert(!waiting_vector.empty());
	return waiting_vector.front();
}

int h8_dtc_device::get_waiting_writeback()
{
	assert(!waiting_writeback.empty());
	return waiting_writeback.front();
}

void h8_dtc_device::queue(int vector)
{
	int ps = -1;
	int cs = cur_active_vector;
	while(cs != -1 && cs < vector) {
		ps = cs;
		cs = states[cs].next;
	}

	states[vector].next = cs;
	if(ps == -1) {
		cur_active_vector = vector;
		cpu->set_current_dtc(&states[vector]);
	} else
		states[ps].next = vector;
}

void h8_dtc_device::vector_done(int vector)
{
	std::list<int>::iterator wi;
	for(wi = waiting_vector.begin(); wi != waiting_vector.end() && *wi != vector && *wi != vector + DTC_CHAINED; wi++);
	assert(wi != waiting_vector.end());
	waiting_vector.erase(wi);

	h8_dtc_state *state = states + vector;
	UINT32 sra = state->sra;
	UINT32 dar = state->dar;
	UINT32 cr = state->cr;

	UINT32 mode = sra & 0x0c000000;
	if(V>=1) logerror("regs at %08x sra=%08x dar=%08x cr=%08x %s mode\n", state->base, sra, dar, cr,
					  mode == 0x00000000 || mode == 0x0c000000 ? "normal" : mode == 0x04000000 ? "repeat" : "block");
	state->incs = sra & 0x80000000 ?
		sra & 0x40000000 ? sra & 0x01000000 ? -2 : -1 :
		sra & 0x01000000 ? 2 : 1 :
		0;
	state->incd = sra & 0x20000000 ?
		sra & 0x10000000 ? sra & 0x01000000 ? -2 : -1 :
		sra & 0x01000000 ? 2 : 1 :
		0;
		
	switch(mode) {
	case 0x00000000: case 0x0c0000000:
		state->count = 1;
		break;
	case 0x04000000:
		state->count = 1;
		break;
	case 0x08000000:
		state->count = (cr >> 16) & 255;
		if(!state->count)
			state->count = 256;
		break;
	}

	queue(vector);

	if(!waiting_vector.empty())
		cpu->request_state(h8_device::STATE_DTC_VECTOR);
	else if(!waiting_writeback.empty())
		cpu->request_state(h8_device::STATE_DTC_WRITEBACK);

}

void h8_dtc_device::writeback_done(int vector)
{
	std::list<int>::iterator wi;
	for(wi = waiting_writeback.begin(); wi != waiting_writeback.end() && *wi != vector; wi++);
	assert(wi != waiting_writeback.end());
	waiting_writeback.erase(wi);


	h8_dtc_state *state = states + vector;
	bool done = false;
	switch(state->sra & 0x0c000000) {
	case 0x00000000: case 0x0c0000000:
		done = !(state->cr & 0xffff0000);
		break;

	case 0x04000000:
		break;

	case 0x08000000:
		done = !(state->cr & 0x0000ffff);
		break;
	}

	if(done && state->dar & 0x80000000) {
		cpu->request_state(h8_device::STATE_DTC_VECTOR);
		waiting_vector.push_back(vector + DTC_CHAINED);
		return;
	}

	if(done || (state->dar & 0x40000000)) {
		if(vector) {
			int slot = vector_to_enable[vector];
			assert(slot != -1);
			dtcer[slot >> 3] &= ~(0x01 << (7-(slot & 7)));
			intc->internal_interrupt(vector);
		} else {
			logerror("Software dtc done\n");
			exit(0);
		}
	}

	if(!waiting_vector.empty())
		cpu->request_state(h8_device::STATE_DTC_VECTOR);
	else if(!waiting_writeback.empty())
		cpu->request_state(h8_device::STATE_DTC_WRITEBACK);
}

bool h8_dtc_device::trigger_dtc(int vector)
{
	int slot = vector_to_enable[vector];
	if(slot == -1)
	   return false;
	if(dtcer[slot >> 3] & (0x01 << (7-(slot & 7)))) {
		edge(vector);
		return true;
	}
	return false;
}

void h8_dtc_device::count_done(int id)
{
	assert(cur_active_vector == id);
	cur_active_vector = states[id].next;
	if(cur_active_vector != -1)
		cpu->set_current_dtc(states + cur_active_vector);

	h8_dtc_state *state = states + id;
	switch(state->sra & 0x0c000000) {
	case 0x00000000: case 0x0c0000000:
		state->cr -= 0x00010000;
		break;

	case 0x04000000:
		state->cr = (state->cr & 0xff00ffff) | ((state->cr - 0x00010000) & 0x00ff0000);
		if(!(state->cr & 0x00ff0000)) {
			int cnt = (state->cr >> 24) & 0xff;
			if(!cnt)
				cnt = 256;
			if(state->sra & 0x02000000)
				state->sra = (state->sra & 0xff000000) | ((state->sra - cnt*state->incs) & 0xffffff);
			else
				state->dar = (state->dar & 0xff000000) | ((state->dar - cnt*state->incd) & 0xffffff);
			state->cr |= (state->cr >> 8) & 0x00ff0000;
		}
		break;

	case 0x08000000: {
		int cnt = (state->cr >> 16) & 0xff;
		if(!cnt)
			cnt = 256;
		if(state->sra & 0x02000000)
			state->sra = (state->sra & 0xff000000) | ((state->sra - cnt*state->incs) & 0xffffff);
		else
			state->dar = (state->dar & 0xff000000) | ((state->dar - cnt*state->incd) & 0xffffff);
		state->cr = (state->cr & 0xff000000) | ((state->cr >> 8) & 0x00ff0000) | ((state->cr - 0x00000001) & 0x0000ffff);
		break;
	}
	}
	if(waiting_vector.empty() && waiting_writeback.empty())
		cpu->request_state(h8_device::STATE_DTC_WRITEBACK);
	waiting_writeback.push_back(id);
}

