// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    tms57002.c

    TMS57002 "DASP" emulator.

***************************************************************************/


#include "emu.h"
#include "debugger.h"
#include "tms57002.h"

const device_type TMS57002 = &device_creator<tms57002_device>;

// Can't use a DEVICE_ADDRESS_MAP, not yet anyway
static ADDRESS_MAP_START(internal_pgm, AS_PROGRAM, 32, tms57002_device)
	AM_RANGE(0x000, 0x3ff) AM_RAM
ADDRESS_MAP_END

tms57002_device::tms57002_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, TMS57002, "TMS57002", tag, owner, clock, "tms57002", __FILE__),
		device_sound_interface(mconfig, *this), macc(0), st0(0), st1(0), sti(0),
		txrd(0),
		program_config("program", ENDIANNESS_LITTLE, 32, 8, -2, ADDRESS_MAP_NAME(internal_pgm)),
		data_config("data", ENDIANNESS_LITTLE, 8, 20)
{
}


WRITE_LINE_MEMBER(tms57002_device::pload_w)
{
	UINT8 olds = sti;
	if(state)
		sti &= ~IN_PLOAD;
	else
		sti |= IN_PLOAD;
	if(olds ^ sti) {
		if (sti & IN_PLOAD) {
			hidx = 0;
			hpc = 0;
		}
	}
}

WRITE_LINE_MEMBER(tms57002_device::cload_w)
{
	UINT8 olds = sti;
	if(state)
		sti &= ~IN_CLOAD;
	else
		sti |= IN_CLOAD;
	if(olds ^ sti) {
		if (sti & IN_CLOAD) {
			hidx = 0;
			ca = 0;
		}
	}
}

void tms57002_device::device_reset()
{
	sti = (sti & ~(SU_MASK|S_READ|S_WRITE|S_BRANCH|S_HOST)) | (SU_ST0|S_IDLE);
	pc = 0;
	ca = 0;
	hidx = 0;
	id = 0;
	ba0 = 0;
	ba1 = 0;
	st0 &= ~(ST0_INCS | ST0_DIRI | ST0_FI | ST0_SIM | ST0_PLRI |
			ST0_PBCI | ST0_DIRO | ST0_FO | ST0_SOM | ST0_PLRO |
			ST0_PBCO | ST0_CNS);
	st1 &= ~(ST1_AOV | ST1_SFAI | ST1_SFAO | ST1_MOVM | ST1_MOV |
			ST1_SFMA | ST1_SFMO | ST1_RND | ST1_CRM | ST1_DBP);

	xba = 0;
	xoa = 0;
	cache_flush();
}

WRITE8_MEMBER(tms57002_device::data_w)
{
	switch(sti & (IN_PLOAD|IN_CLOAD)) {
	case 0:
		hidx = 0;
		sti &= ~SU_CVAL;
		break;
	case IN_PLOAD:
		host[hidx++] = data;
		if(hidx >= 3) {
			UINT32 val = (host[0]<<16) | (host[1]<<8) | host[2];
			hidx = 0;

			switch(sti & SU_MASK) {
			case SU_ST0:
				st0 = val;
				sti = (sti & ~SU_MASK) | SU_ST1;
				break;
			case SU_ST1:
				st1 = val;
				sti = (sti & ~SU_MASK) | SU_PRG;
				break;
			case SU_PRG:
				program->write_dword(pc++ << 2, val);
				break;
			}
		}
		break;
	case IN_CLOAD:
		if(sti & SU_CVAL) {
			host[hidx++] = data;
			if(hidx >= 4) {
				UINT32 val = (host[0]<<24) | (host[1]<<16) | (host[2]<<8) | host[3];
				cmem[sa] = val;
				sti &= ~SU_CVAL;
				allow_update = 0;
			}
		} else {
			sa = data;
			hidx = 0;
			sti |= SU_CVAL;
		}

		break;
	case IN_PLOAD|IN_CLOAD:
		host[hidx++] = data;
		if(hidx >= 4) {
			UINT32 val = (host[0]<<24) | (host[1]<<16) | (host[2]<<8) | host[3];
			hidx = 0;
			cmem[ca++] = val;
		}
		break;
	};
}

READ8_MEMBER(tms57002_device::data_r)
{
	UINT8 res;
	if(!(sti & S_HOST))
		return 0xff;

	res = host[hidx];
	hidx++;
	if(hidx == 4) {
		hidx = 0;
		sti &= ~S_HOST;
	}

	return res;
}

READ_LINE_MEMBER(tms57002_device::empty_r)
{
	return 1;
}

READ_LINE_MEMBER(tms57002_device::dready_r)
{
	return sti & S_HOST ? 0 : 1;
}

READ_LINE_MEMBER(tms57002_device::pc0_r)
{
	return pc == 0 ? 0 : 1;
}

WRITE_LINE_MEMBER(tms57002_device::sync_w)
{
	if(sti & (IN_PLOAD | IN_CLOAD))
		return;

	allow_update = 1;
	pc = 0;
	ca = 0;
	id = 0;
	if(!(st0 & ST0_INCS)) {
		ba0--;
		ba1++;
	}
	xba = (xba-1) & 0x7ffff;
	st1 &= ~(ST1_AOV | ST1_MOV);
	sti &= ~S_IDLE;
}

void tms57002_device::xm_init()
{
	UINT32 adr = xoa + xba;
	UINT32 mask = 0;

	switch(st0 & ST0_M) {
	case ST0_M_64K:  mask = 0x0ffff; break;
	case ST0_M_256K: mask = 0x3ffff; break;
	case ST0_M_1M:   mask = 0xfffff; break;
	}
	if(st0 & ST0_WORD)
		adr <<= 2;
	else
		adr <<= 1;

	if(!(st0 & ST0_SEL))
		adr <<= 1;

	xm_adr = adr & mask;
}

inline void tms57002_device::xm_step_read()
{
	UINT32 adr = xm_adr;
	UINT8 v = data->read_byte(adr);
	int done;
	if(st0 & ST0_WORD) {
		if(st0 & ST0_SEL) {
			int off = 16 - ((adr & 3) << 3);
			txrd = (txrd & ~(0xff << off)) | (v << off);
			done = off == 0;
		} else {
			int off = 20 - ((adr & 7) << 2);
			txrd = (txrd & ~(0xf << off)) | ((v & 0xf) << off);
			done = off == 0;
		}
	} else {
		if(st0 & ST0_SEL) {
			int off = 16 - ((adr & 1) << 3);
			txrd = (txrd & ~(0xff << off)) | (v << off);
			done = off == 8;
			if(done)
				txrd &= 0xffff00;
		} else {
			int off = 20 - ((adr & 3) << 2);
			txrd = (txrd & ~(0xf << off)) | ((v & 0xf) << off);
			done = off == 8;
			if(done)
				txrd &= 0xffff00;
		}
	}
	if(done) {
		xrd = txrd;
		sti &= ~S_READ;
		xm_adr = 0;
	} else
		xm_adr = adr+1;
}

inline void tms57002_device::xm_step_write()
{
	UINT32 adr = xm_adr;
	UINT8 v;
	int done;
	if(st0 & ST0_WORD) {
		if(st0 & ST0_SEL) {
			int off = 16 - ((adr & 3) << 3);
			v = xwr >> off;
			done = off == 0;
		} else {
			int off = 20 - ((adr & 7) << 2);
			v = (xwr >> off) & 0xf;
			done = off == 0;
		}
	} else {
		if(st0 & ST0_SEL) {
			int off = 16 - ((adr & 1) << 3);
			v = xwr >> off;
			done = off == 8;
		} else {
			int off = 20 - ((adr & 3) << 2);
			v = (xwr >> off) & 0xf;
			done = off == 8;
		}
	}
	data->write_byte(adr, v);
	if(done) {
		sti &= ~S_WRITE;
		xm_adr = 0;
	} else
		xm_adr = adr+1;
}

INT64 tms57002_device::macc_to_output_0(INT64 rounding, UINT64 rmask)
{
	INT64 m = macc;
	UINT64 m1;
	int over = 0;

	// Overflow detection and shifting
	m1 = m & 0xf800000000000ULL;
	if(m1 && m1 != 0xf800000000000ULL)
		over = 1;

	m = (m + rounding) & rmask;

	// Second overflow detection
	m1 = m & 0xf800000000000ULL;
	if(m1 && m1 != 0xf800000000000ULL)
		over = 1;

	// Overflow handling
	if(over) {
		st1 |= ST1_MOV;
	}
	return m;
}

INT64 tms57002_device::macc_to_output_1(INT64 rounding, UINT64 rmask)
{
	INT64 m = macc;
	UINT64 m1;
	int over = 0;

	// Overflow detection and shifting
	m1 = m & 0xfe00000000000ULL;
	if(m1 && m1 != 0xfe00000000000ULL)
		over = 1;
	m <<= 2;

	m = (m + rounding) & rmask;

	// Second overflow detection
	m1 = m & 0xf800000000000ULL;
	if(m1 && m1 != 0xf800000000000ULL)
		over = 1;

	// Overflow handling
	if(over) {
		st1 |= ST1_MOV;
	}
	return m;
}

INT64 tms57002_device::macc_to_output_2(INT64 rounding, UINT64 rmask)
{
	INT64 m = macc;
	UINT64 m1;
	int over = 0;

	// Overflow detection and shifting
	m1 = m & 0xff80000000000ULL;
	if(m1 && m1 != 0xff80000000000ULL)
		over = 1;
	m <<= 4;

	m = (m + rounding) & rmask;

	// Second overflow detection
	m1 = m & 0xf800000000000ULL;
	if(m1 && m1 != 0xf800000000000ULL)
		over = 1;

	// Overflow handling
	if(over) {
		st1 |= ST1_MOV;
	}
	return m;
}

INT64 tms57002_device::macc_to_output_3(INT64 rounding, UINT64 rmask)
{
	INT64 m = macc;
	UINT64 m1;
	int over = 0;

	// Overflow detection and shifting
	m >>= 8;

	m = (m + rounding) & rmask;

	// Second overflow detection
	m1 = m & 0xf800000000000ULL;
	if(m1 && m1 != 0xf800000000000ULL)
		over = 1;

	// Overflow handling
	if(over) {
		st1 |= ST1_MOV;
	}
	return m;
}

INT64 tms57002_device::macc_to_output_0s(INT64 rounding, UINT64 rmask)
{
	INT64 m = macc;
	UINT64 m1;
	int over = 0;

	// Overflow detection and shifting
	m1 = m & 0xf800000000000ULL;
	if(m1 && m1 != 0xf800000000000ULL)
		over = 1;

	m = (m + rounding) & rmask;

	// Second overflow detection
	m1 = m & 0xf800000000000ULL;
	if(m1 && m1 != 0xf800000000000ULL)
		over = 1;

	// Overflow handling
	if(over) {
		st1 |= ST1_MOV;
		if(m & 0x8000000000000ULL)
			m = 0xffff800000000000ULL;
		else
			m = 0x00007fffffffffffULL;
	}
	return m;
}

INT64 tms57002_device::macc_to_output_1s(INT64 rounding, UINT64 rmask)
{
	INT64 m = macc;
	UINT64 m1;
	int over = 0;

	// Overflow detection and shifting
	m1 = m & 0xfe00000000000ULL;
	if(m1 && m1 != 0xfe00000000000ULL)
		over = 1;
	m <<= 2;

	m = (m + rounding) & rmask;

	// Second overflow detection
	m1 = m & 0xf800000000000ULL;
	if(m1 && m1 != 0xf800000000000ULL)
		over = 1;

	// Overflow handling
	if(over) {
		st1 |= ST1_MOV;
		if(m & 0x8000000000000ULL)
			m = 0xffff800000000000ULL;
		else
			m = 0x00007fffffffffffULL;
	}
	return m;
}

INT64 tms57002_device::macc_to_output_2s(INT64 rounding, UINT64 rmask)
{
	INT64 m = macc;
	UINT64 m1;
	int over = 0;

	// Overflow detection and shifting
	m1 = m & 0xff80000000000ULL;
	if(m1 && m1 != 0xff80000000000ULL)
		over = 1;
	m <<= 4;

	m = (m + rounding) & rmask;

	// Second overflow detection
	m1 = m & 0xf800000000000ULL;
	if(m1 && m1 != 0xf800000000000ULL)
		over = 1;

	// Overflow handling
	if(over) {
		st1 |= ST1_MOV;
		if(m & 0x8000000000000ULL)
			m = 0xffff800000000000ULL;
		else
			m = 0x00007fffffffffffULL;
	}
	return m;
}

INT64 tms57002_device::macc_to_output_3s(INT64 rounding, UINT64 rmask)
{
	INT64 m = macc;
	UINT64 m1;
	int over = 0;

	// Overflow detection and shifting
	m >>= 8;

	m = (m + rounding) & rmask;

	// Second overflow detection
	m1 = m & 0xf800000000000ULL;
	if(m1 && m1 != 0xf800000000000ULL)
		over = 1;

	// Overflow handling
	if(over) {
		st1 |= ST1_MOV;
		if(m & 0x8000000000000ULL)
			m = 0xffff800000000000ULL;
		else
			m = 0x00007fffffffffffULL;
	}
	return m;
}

INT64 tms57002_device::check_macc_overflow_0()
{
	INT64 m = macc;
	UINT64 m1;

	// Overflow detection
	m1 = m & 0xf800000000000ULL;
	if(m1 && m1 != 0xf800000000000ULL) {
		st1 |= ST1_MOV;
	}
	return m;
}

INT64 tms57002_device::check_macc_overflow_1()
{
	INT64 m = macc;
	UINT64 m1;

	// Overflow detection
	m1 = m & 0xfe00000000000ULL;
	if(m1 && m1 != 0xfe00000000000ULL) {
		st1 |= ST1_MOV;
	}
	return m;
}

INT64 tms57002_device::check_macc_overflow_2()
{
	INT64 m = macc;
	UINT64 m1;

	// Overflow detection
	m1 = m & 0xff80000000000ULL;
	if(m1 && m1 != 0xff80000000000ULL) {
		st1 |= ST1_MOV;
	}
	return m;
}

INT64 tms57002_device::check_macc_overflow_3()
{
	return macc;
}

INT64 tms57002_device::check_macc_overflow_0s()
{
	INT64 m = macc;
	UINT64 m1;

	// Overflow detection
	m1 = m & 0xf800000000000ULL;
	if(m1 && m1 != 0xf800000000000ULL) {
		st1 |= ST1_MOV;
		if(m & 0x8000000000000ULL)
			m = 0xffff800000000000ULL;
		else
			m = 0x00007fffffffffffULL;
	}
	return m;
}

INT64 tms57002_device::check_macc_overflow_1s()
{
	INT64 m = macc;
	UINT64 m1;

	// Overflow detection
	m1 = m & 0xfe00000000000ULL;
	if(m1 && m1 != 0xfe00000000000ULL) {
		st1 |= ST1_MOV;
		if(m & 0x8000000000000ULL)
			m = 0xffff800000000000ULL;
		else
			m = 0x00007fffffffffffULL;
	}
	return m;
}

INT64 tms57002_device::check_macc_overflow_2s()
{
	INT64 m = macc;
	UINT64 m1;

	// Overflow detection
	m1 = m & 0xff80000000000ULL;
	if(m1 && m1 != 0xff80000000000ULL) {
		st1 |= ST1_MOV;
		if(m & 0x8000000000000ULL)
			m = 0xffff800000000000ULL;
		else
			m = 0x00007fffffffffffULL;
	}
	return m;
}

INT64 tms57002_device::check_macc_overflow_3s()
{
	return macc;
}

void tms57002_device::cache_flush()
{
	int i;
	cache.hused = cache.iused = 0;
	for(i=0; i != 256; i++)
		cache.hashbase[i] = -1;
	for(i=0; i != HBS; i++) {
		cache.hashnode[i].st1 = 0;
		cache.hashnode[i].ipc = -1;
		cache.hashnode[i].next = -1;
	}
	for(i=0; i != IBS; i++) {
		cache.inst[i].op = 0;
		cache.inst[i].next = -1;
		cache.inst[i].param = 0;
	}
}

void tms57002_device::add_one(cstate *cs, unsigned short op, UINT8 param)
{
	short ipc = cache.iused++;
	cache.inst[ipc].op = op;
	cache.inst[ipc].param = param;
	cache.inst[ipc].next = -1;
	if(cs->ipc != -1)
		cache.inst[cs->ipc].next = ipc;
	cs->ipc = ipc;
	if(cs->hnode != -1) {
		cache.hashnode[cs->hnode].ipc = ipc;
		cs->hnode = -1;
	}
}

void tms57002_device::decode_one(UINT32 opcode, cstate *cs, void (tms57002_device::*dec)(UINT32 opcode, unsigned short *op, cstate *cs))
{
	unsigned short op = 0;
	(this->*dec)(opcode, &op, cs);
	if(!op)
		return;
	add_one(cs, op, opcode & 0xff);
}

short tms57002_device::get_hash(unsigned char adr, UINT32 st1, short *pnode)
{
	short hnode;
	st1 &= ST1_CACHE;
	*pnode = -1;
	hnode = cache.hashbase[adr];
	while(hnode != -1) {
		if(cache.hashnode[hnode].st1 == st1)
			return cache.hashnode[hnode].ipc;
		*pnode = hnode;
		hnode = cache.hashnode[hnode].next;
	}
	return -1;
}

short tms57002_device::get_hashnode(unsigned char adr, UINT32 st1, short pnode)
{
	short hnode = cache.hused++;
	cache.hashnode[hnode].st1 = st1 & ST1_CACHE;
	cache.hashnode[hnode].ipc = -1;
	cache.hashnode[hnode].next = -1;
	if(pnode == -1)
		cache.hashbase[adr] = hnode;
	else
		cache.hashnode[pnode].next = hnode;
	return hnode;
}

int tms57002_device::decode_get_pc()
{
	short pnode, res;
	cstate cs;
	UINT8 adr = pc;

	res = get_hash(adr, st1, &pnode);
	if(res != -1)
		return res;

	if(HBS - cache.hused < 256 || IBS - cache.iused < 256*3) {
		cache_flush();
		pnode = -1;
	}

	cs.hnode = res = get_hashnode(adr, st1, pnode);
	cs.ipc = -1;
	cs.branch = 0;

	for(;;) {
		short ipc;
		UINT32 opcode = program->read_dword(adr << 2);

		cs.inc = 0;

		if((opcode & 0xfc0000) == 0xfc0000)
			decode_one(opcode, &cs, &tms57002_device::decode_cat3);
		else {
			decode_one(opcode, &cs, &tms57002_device::decode_cat2_pre);
			decode_one(opcode, &cs, &tms57002_device::decode_cat1);
			decode_one(opcode, &cs, &tms57002_device::decode_cat2_post);
		}
		add_one(&cs, cs.inc, 0);

		if(cs.branch)
			break;

		adr++;
		ipc = get_hash(adr, st1, &pnode);
		if(ipc != -1) {
			cache.inst[cs.ipc].next = ipc;
			break;
		}
		cs.hnode = get_hashnode(adr, st1, pnode);
	}

	return cache.hashnode[res].ipc;
}

void tms57002_device::execute_run()
{
	int ipc = -1;

	while(icount > 0 && !(sti & (S_IDLE | IN_PLOAD | IN_CLOAD))) {
		int iipc;

		debugger_instruction_hook(this, pc);

		if(ipc == -1)
			ipc = decode_get_pc();

		iipc = ipc;

		if(sti & (S_READ|S_WRITE)) {
			if(sti & S_READ)
				xm_step_read();
			else
				xm_step_write();
		}

		for(;;) {
			UINT32 c, d;
			INT64 r;
			const icd *i = cache.inst + ipc;

			ipc = i->next;
			switch(i->op) {
			case 0:
				goto inst;

			case 1:
				++ca;
				goto inst;

			case 2:
				++id;
				goto inst;

			case 3:
				++ca, ++id;
				goto inst;

#define CINTRP
#include "cpu/tms57002/tms57002.inc"
#undef CINTRP

			default:
				fatalerror("Unhandled opcode in tms57002_execute\n");
			}
		}
	inst:
		icount--;

		if(rptc) {
			rptc--;
			ipc = iipc;
		} else if(sti & S_BRANCH) {
			sti &= ~S_BRANCH;
			ipc = -1;
		} else
			pc++; // Wraps if it reaches 256, next wraps too

		if(rptc_next) {
			rptc = rptc_next;
			rptc_next = 0;
		}
	}

	if(icount > 0)
		icount = 0;
}

void tms57002_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	assert(samples == 1);

	if(st0 & ST0_SIM) {
		si[0] = (inputs[0][0] << 8) & 0xffffff;
		si[1] = (inputs[1][0] << 8) & 0xffffff;
		si[2] = (inputs[2][0] << 8) & 0xffffff;
		si[3] = (inputs[3][0] << 8) & 0xffffff;
	} else {
		si[0] = inputs[0][0] & 0xffffff;
		si[1] = inputs[1][0] & 0xffffff;
		si[2] = inputs[2][0] & 0xffffff;
		si[3] = inputs[3][0] & 0xffffff;
	}
	outputs[0][0] = INT16(so[0] >> 8);
	outputs[1][0] = INT16(so[1] >> 8);
	outputs[2][0] = INT16(so[2] >> 8);
	outputs[3][0] = INT16(so[3] >> 8);

	sync_w(1);
}

void tms57002_device::device_start()
{
	sti = S_IDLE;
	program = &space(AS_PROGRAM);
	data    = &space(AS_DATA);

	state_add(STATE_GENPC,    "GENPC",  pc).noshow();
	state_add(TMS57002_PC,    "PC",     pc);
	state_add(TMS57002_ST0,   "ST0",    st0);
	state_add(TMS57002_ST1,   "ST1",    st1);
	state_add(TMS57002_RPTC,  "RPTC",   rptc);
	state_add(TMS57002_AACC,  "AACC",   aacc);
	state_add(TMS57002_MACC,  "MACC",   macc).mask(U64(0xfffffffffffff));
	state_add(TMS57002_BA0,   "BA0",    ba0);
	state_add(TMS57002_BA1,   "BA1",    ba1);
	state_add(TMS57002_CREG,  "CREG",   creg);
	state_add(TMS57002_CA,    "CA",     ca);
	state_add(TMS57002_ID,    "ID",     id);
	state_add(TMS57002_XBA,   "XBA",    xba);
	state_add(TMS57002_XOA,   "XOA",    xoa);
	state_add(TMS57002_XRD,   "XRD",    xrd);
	state_add(TMS57002_XWR,   "XWR",    xwr);
	state_add(TMS57002_HIDX,  "HIDX",   hidx);
	state_add(TMS57002_HOST0, "HOST0",  host[0]);
	state_add(TMS57002_HOST1, "HOST1",  host[1]);
	state_add(TMS57002_HOST2, "HOST2",  host[2]);
	state_add(TMS57002_HOST3, "HOST3",  host[3]);

	m_icountptr = &icount;

	stream_alloc(4, 4, STREAM_SYNC);

	save_item(NAME(macc));

	save_item(NAME(cmem));
	save_item(NAME(dmem0));
	save_item(NAME(dmem1));

	save_item(NAME(si));
	save_item(NAME(so));

	save_item(NAME(st0));
	save_item(NAME(st1));
	save_item(NAME(sti));
	save_item(NAME(aacc));
	save_item(NAME(xoa));
	save_item(NAME(xba));
	save_item(NAME(xwr));
	save_item(NAME(xrd));
	save_item(NAME(txrd));
	save_item(NAME(creg));

	save_item(NAME(pc));
	save_item(NAME(ca));
	save_item(NAME(id));
	save_item(NAME(ba0));
	save_item(NAME(ba1));
	save_item(NAME(rptc));
	save_item(NAME(rptc_next));
	save_item(NAME(sa));

	save_item(NAME(xm_adr));

	save_item(NAME(host));
	save_item(NAME(hidx));
	save_item(NAME(allow_update));
}

UINT32 tms57002_device::execute_min_cycles() const
{
	return 1;
}

UINT32 tms57002_device::execute_max_cycles() const
{
	return 3;
}

UINT32 tms57002_device::execute_input_lines() const
{
	return 0;
}

UINT32 tms57002_device::disasm_min_opcode_bytes() const
{
	return 4;
}

UINT32 tms57002_device::disasm_max_opcode_bytes() const
{
	return 4;
}

offs_t tms57002_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( tms57002 );
	return CPU_DISASSEMBLE_NAME(tms57002)(this, buffer, pc, oprom, opram, options);
}

const address_space_config *tms57002_device::memory_space_config(address_spacenum spacenum) const
{
	switch(spacenum) {
	case AS_PROGRAM: return &program_config;
	case AS_DATA: return &data_config;
	default: return nullptr;
	}
}
