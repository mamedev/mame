#include "tms57002.h"
#include "debugger.h"

CPU_DISASSEMBLE(tms57002);

#ifdef __GNUC__
#define noinline __attribute__((noinline))
#else
#define noinline /* */
#endif

enum {
	IN_PLOAD = 0x00000001,
	IN_CLOAD = 0x00000002,
	SU_CVAL  = 0x00000004,
	SU_MASK  = 0x00000018, SU_ST0 = 0x00, SU_ST1 = 0x08, SU_PRG = 0x10,
	S_IDLE   = 0x00000020,
	S_READ   = 0x00000040,
	S_WRITE  = 0x00000080,
	S_BRANCH = 0x00000100,
	S_HOST   = 0x00000200
};

enum {
	ST0_INCS = 0x000001,
	ST0_DIRI = 0x000002,
	ST0_FI   = 0x000004,
	ST0_SIM  = 0x000008,
	ST0_PLRI = 0x000020,
	ST0_PBCI = 0x000040,
	ST0_DIRO = 0x000080,
	ST0_FO   = 0x000100,
	ST0_SOM  = 0x000600,
	ST0_PLRO = 0x000800,
	ST0_PBCO = 0x001000,
	ST0_CNS  = 0x002000,
	ST0_WORD = 0x004000,
	ST0_SEL  = 0x008000,
	ST0_M    = 0x030000, ST0_M_64K = 0x000000, ST0_M_256K = 0x010000, ST0_M_1M = 0x020000,
	ST0_SRAM = 0x200000,

	ST1_AOV  = 0x000001,
	ST1_SFAI = 0x000002,
	ST1_SFAO = 0x000004,
	ST1_MOVM = 0x000020,
	ST1_MOV  = 0x000040,
	ST1_SFMA = 0x000180, ST1_SFMA_SHIFT = 7,
	ST1_SFMO = 0x001800, ST1_SFMO_SHIFT = 11,
	ST1_RND  = 0x038000, ST1_RND_SHIFT = 15,
	ST1_CRM  = 0x0C0000, ST1_CRM_SHIFT = 18, ST1_CRM_32 = 0x000000, ST1_CRM_16H = 0x040000, ST1_CRM_16L = 0x080000,
	ST1_DBP  = 0x100000,
	ST1_CAS  = 0x200000,

	ST1_CACHE = ST1_SFAI|ST1_SFAO|ST1_MOVM|ST1_SFMA|ST1_SFMO|ST1_RND|ST1_CRM|ST1_DBP
};

enum { BR_UB, BR_CB, BR_IDLE };

enum { IBS = 8192, HBS = 4096 };

typedef struct {
	unsigned short op;
	short next;
	unsigned char param;
} icd;

typedef struct {
	unsigned int st1;
	short ipc;
	short next;
} hcd;

typedef struct {
	short hashbase[256];
	hcd hashnode[HBS];
	icd inst[IBS];
	int hused, iused;
} cd;

typedef struct {
	int branch;
	short hnode;
	short ipc;
} cstate;

typedef struct {
	INT64 macc;

	UINT32 cmem[256];
	UINT32 dmem0[256];
	UINT32 dmem1[32];

	UINT32 si[4], so[4];

	UINT32 st0, st1, sti;
	UINT32 aacc, xoa, xba, xwr, xrd, creg;

	UINT8 pc, ca, id, ba0, ba1, rptc, rptc_next, sa;

	UINT32 xm_adr;

	UINT8 host[4], hidx, allow_update;

	cd cache;

	const address_space *program, *data;
	int icount;
	int unsupported_inst_warning;
} tms57002_t;

INLINE tms57002_t *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == CPU);
	assert(cpu_get_type(device) == CPU_TMS57002);
	return (tms57002_t *)device->token;
}

static void tms57002_cache_flush(tms57002_t *s);

WRITE8_DEVICE_HANDLER(tms57002_pload_w)
{
	tms57002_t *s = get_safe_token(device);
	UINT8 olds = s->sti;

	if(data)
		s->sti &= ~IN_PLOAD;
	else
		s->sti |= IN_PLOAD;

	if(olds ^ s->sti)
		s->hidx = 0;
}

WRITE8_DEVICE_HANDLER(tms57002_cload_w)
{
	tms57002_t *s = get_safe_token(device);
	UINT8 olds = s->sti;
	if(data)
		s->sti &= ~IN_CLOAD;
	else
		s->sti |= IN_CLOAD;
	if(olds ^ s->sti)
		s->hidx = 0;
}

static CPU_RESET(tms57002)
{
	tms57002_t *s = get_safe_token(device);

	s->sti = (s->sti & ~(SU_MASK|S_READ|S_WRITE|S_BRANCH|S_HOST)) | (SU_ST0|S_IDLE);
	s->pc = 0;
	s->ca = 0;
	s->hidx = 0;
	s->id = 0;
	s->ba0 = 0;
	s->ba1 = 0;
	s->st0 &= ~(ST0_INCS | ST0_DIRI | ST0_FI | ST0_SIM | ST0_PLRI |
				ST0_PBCI | ST0_DIRO | ST0_FO | ST0_SOM | ST0_PLRO |
				ST0_PBCO | ST0_CNS);
	s->st1 &= ~(ST1_AOV | ST1_SFAI | ST1_SFAO | ST1_MOVM | ST1_MOV |
				ST1_SFMA | ST1_SFMO | ST1_RND | ST1_CRM | ST1_DBP);

	s->xba = 0; // Not sure but makes sense

	tms57002_cache_flush(s);
}


WRITE8_DEVICE_HANDLER(tms57002_data_w)
{
	tms57002_t *s = get_safe_token(device);

	switch(s->sti & (IN_PLOAD|IN_CLOAD)) {
	case 0:
		s->hidx = 0;
		s->sti &= ~SU_CVAL;
		break;
	case IN_PLOAD:
		s->host[s->hidx++] = data;
		if(s->hidx >= 3) {
			UINT32 val = (s->host[0]<<16) | (s->host[1]<<8) | s->host[2];
			s->hidx = 0;

			switch(s->sti & SU_MASK) {
			case SU_ST0:
				s->st0 = val;
				s->sti = (s->sti & ~SU_MASK) | SU_ST1;
				break;
			case SU_ST1:
				s->st1 = val;
				s->sti = (s->sti & ~SU_MASK) | SU_PRG;
				break;
			case SU_PRG:
				memory_write_dword_32le(s->program, (s->pc++) << 2, val);
				break;
			}
		}
		break;
	case IN_CLOAD:
		if(s->sti & SU_CVAL) {
			s->host[s->hidx++] = data;
			if(s->hidx >= 4) {
				UINT32 val = (s->host[0]<<24) | (s->host[1]<<16) | (s->host[2]<<8) | s->host[3];
				s->cmem[s->sa] = val;
				s->sti &= ~SU_CVAL;
				s->allow_update = 0;
			}
		} else {
			s->sa = data;
			s->hidx = 0;
			s->sti |= SU_CVAL;
		}

		break;
	case IN_PLOAD|IN_CLOAD:
		s->host[s->hidx++] = data;
		if(s->hidx >= 4) {
			UINT32 val = (s->host[0]<<24) | (s->host[1]<<16) | (s->host[2]<<8) | s->host[3];
			s->hidx = 0;
			s->cmem[s->ca++] = val;
		}
		break;
	};
}

READ8_DEVICE_HANDLER(tms57002_data_r)
{
	tms57002_t *s = get_safe_token(device);
	UINT8 res;
	if(!(s->sti & S_HOST))
		return 0xff;

	res = s->host[s->hidx];
	s->hidx++;
	if(s->hidx == 4) {
		s->hidx = 0;
		s->sti &= ~S_HOST;
	}

	return res;
}

READ8_DEVICE_HANDLER(tms57002_empty_r)
{
	return 1;
}

READ8_DEVICE_HANDLER(tms57002_dready_r)
{
	tms57002_t *s = get_safe_token(device);
	return s->sti & S_HOST ? 0 : 1;
}

void tms57002_sync(const device_config *device)
{
	tms57002_t *s = get_safe_token(device);

	if(s->sti & (IN_PLOAD | IN_CLOAD))
		return;

	s->allow_update = 1;
	s->pc = 0;
	s->ca = 0;
	s->id = 0;
	if(!(s->st0 & ST0_INCS)) {
		s->ba0--;
		s->ba1++;
	}
	s->xba = (s->xba-1) & 0x7ffff;
	s->st1 &= ~(ST1_AOV | ST1_MOV);
	s->sti &= ~S_IDLE;
}

#ifdef UNUSED_FUNCTION
static UINT32 tms57002_read_c(tms57002_t *s, UINT8 index)
{
	UINT32 v = s->cmem[index];
	if((s->st1 & ST1_CRM) != ST1_CRM_32) {
		if((s->st1 & ST1_CRM) == ST1_CRM_16H)
			v &= 0xffff0000;
		else if((s->st1 & ST1_CRM) == ST1_CRM_16L)
			v <<= 16;
	}
	return v;
}

static void tms57002_write_c(tms57002_t *s, UINT8 index, UINT32 v)
{
	s->cmem[index] = v;
}

static void tms57002_write_d(tms57002_t *s, UINT8 index, UINT32 v)
{
	if(s->st1 & ST1_DBP)
		s->dmem1[(s->ba1 + index) & 0x1f] = v;
	else
		s->dmem0[(s->ba0 + index) & 0xff] = v;
}

static UINT32 tms57002_read_d(tms57002_t *s, UINT8 index)
{
	if(s->st1 & ST1_DBP)
		return s->dmem1[(s->ba1 + index) & 0x1f];
	else
		return s->dmem0[(s->ba0 + index) & 0xff];
}

static void tms57002_opc_write_c(tms57002_t *s, UINT32 opcode, UINT32 v)
{
	if(opcode & 0x400) {
		if(opcode & 0x100)
			tms57002_write_c(s, opcode & 0xff, v);
		else if(opcode & 0x80)
			tms57002_write_c(s, s->ca++, v);
		else
			tms57002_write_c(s, s->ca, v);
	} else if(opcode & 0x200)
		tms57002_write_c(s, s->ca++, v);
	else
		tms57002_write_c(s, s->ca, v);
}

static UINT32 tms57002_opc_read_c(tms57002_t *s, UINT32 opcode)
{
	if(opcode & 0x400) {
		if(opcode & 0x100)
			return tms57002_read_c(s, opcode & 0xff);
		else if(opcode & 0x80)
			return tms57002_read_c(s, s->ca++);
		else
			return tms57002_read_c(s, s->ca);
	} else if(opcode & 0x200)
		return tms57002_read_c(s, s->ca++);
	else
		return tms57002_read_c(s, s->ca);
}

static void tms57002_opc_write_d(tms57002_t *s, UINT32 opcode, UINT32 v)
{
	if(!(opcode & 0x400)) {
		if(opcode & 0x100)
			tms57002_write_d(s, opcode & 0xff, v);
		else if(opcode & 0x80)
			tms57002_write_d(s, s->id++, v);
		else
			tms57002_write_d(s, s->id, v);
	} else if(opcode & 0x200)
		tms57002_write_d(s, s->id++, v);
	else
		tms57002_write_d(s, s->id, v);
}

static UINT32 tms57002_opc_read_d(tms57002_t *s, UINT32 opcode)
{
	if(!(opcode & 0x400)) {
		if(opcode & 0x100)
			return tms57002_read_d(s, opcode & 0xff);
		else if(opcode & 0x80)
			return tms57002_read_d(s, s->id++);
		else
			return tms57002_read_d(s, s->id);
	} else if(opcode & 0x200)
		return tms57002_read_d(s, s->id++);
	else
		return tms57002_read_d(s, s->id);
}
#endif

static void tms57002_xm_init(tms57002_t *s)
{
	UINT32 adr = s->xoa + s->xba;
	UINT32 mask = 0;

	switch(s->st0 & ST0_M) {
	case ST0_M_64K:  mask = 0x0ffff; break;
	case ST0_M_256K: mask = 0x3ffff; break;
	case ST0_M_1M:   mask = 0xfffff; break;
	}
	if(s->st0 & ST0_WORD)
		adr <<= 2;
	else
		adr <<= 1;

	if(!(s->st0 & ST0_SEL))
		adr <<= 1;

	s->xm_adr = adr & mask;
}

static void tms57002_xm_step_read(tms57002_t *s)
{
	UINT32 adr = s->xm_adr;
	UINT8 v = memory_read_byte_8le(s->data, adr);
	int done;
	if(s->st0 & ST0_WORD) {
		if(s->st0 & ST0_SEL) {
			int off = (adr & 3) << 3;
			s->xrd = (s->xrd & ~(0xff << off)) | (v << off);
			done = off == 16;
		} else {
			int off = (adr & 7) << 2;
			s->xrd = (s->xrd & ~(0xf << off)) | ((v & 0xf) << off);
			done = off == 20;
		}
	} else {
		if(s->st0 & ST0_SEL) {
			int off = (adr & 1) << 3;
			s->xrd = (s->xrd & ~(0xff << off)) | (v << off);
			done = off == 8;
			if(done)
				s->xrd &= 0x00ffff;
		} else {
			int off = (adr & 3) << 2;
			s->xrd = (s->xrd & ~(0xf << off)) | ((v & 0xf) << off);
			done = off == 12;
			if(done)
				s->xrd &= 0x00ffff;
		}
	}
	if(done) {
		s->sti &= ~S_READ;
		s->xm_adr = 0;
	} else
		s->xm_adr = adr+1;
}

static void tms57002_xm_step_write(tms57002_t *s)
{
	UINT32 adr = s->xm_adr;
	UINT8 v;
	int done;
	if(s->st0 & ST0_WORD) {
		if(s->st0 & ST0_SEL) {
			int off = (adr & 3) << 3;
			v = s->xwr >> off;
			done = off == 16;
		} else {
			int off = (adr & 7) << 2;
			v = (s->xwr >> off) & 0xf;
			done = off == 20;
		}
	} else {
		if(s->st0 & ST0_SEL) {
			int off = (adr & 1) << 3;
			v = s->xwr >> off;
			done = off == 8;
		} else {
			int off = (adr & 3) << 2;
			v = (s->xwr >> off) & 0xf;
			done = off == 12;
		}
	}
	memory_write_byte_8le(s->data, adr, v);
	if(done) {
		s->sti &= ~S_WRITE;
		s->xm_adr = 0;
	} else
		s->xm_adr = adr+1;
}

#ifdef UNUSED_FUNCTION
static UINT32 tms57002_aacc_to_output(tms57002_t *s)
{
	if(s->st1 & ST1_SFAO)
		return s->aacc << 7;
	else
		return s->aacc;
}

static INT64 tms57002_macc_to_output(tms57002_t *s)
{
	INT64 m = s->macc;
	UINT64 m1;
	int over = 0, rmode;
	static const INT64 rounding[8] = {
		0,
		1LL << (48-32-1),
		1LL << (48-24-1),
		1LL << (48-30-1),
		1LL << (48-16-1),
		0, 0, 0
	};
	static const UINT64 rmask[8] = {
		~0ULL,
		(~0ULL) << (48-32),
		(~0ULL) << (48-24),
		(~0ULL) << (48-30),
		(~0ULL) << (48-16),
		~0ULL, ~0ULL, ~0ULL
	};

	// Overflow detection and shifting
	switch((s->st1 & ST1_SFMO) >> ST1_SFMO_SHIFT) {
	case 0: // sfmo 0
		m1 = m & 0xf800000000000ULL;
		if(m1 && m1 != 0xf800000000000ULL)
			over = 1;
		break;
	case 1: // sfmo 2
		m1 = m & 0xfe00000000000ULL;
		if(m1 && m1 != 0xfe00000000000ULL)
			over = 1;
		m <<= 2;
		break;
	case 2: // sfmo 4
		m1 = m & 0xff80000000000ULL;
		if(m1 && m1 != 0xff80000000000ULL)
			over = 1;
		m <<= 4;
		break;
	case 3: // sfmo -8
		m >>= 8;
		break;
	}

	// Rounder
	rmode = (s->st1 & ST1_RND) >> ST1_RND_SHIFT;
	m = (m + rounding[rmode]) & rmask[rmode];

	// Second overflow detection
	m1 = m & 0xf800000000000ULL;
	if(m1 && m1 != 0xf800000000000ULL)
		over = 1;

	// Overflow handling
	if(over) {
		s->st1 |= ST1_MOV;
		if(s->st1 & ST1_MOVM) {
			if(m & 0x8000000000000ULL)
				m = 0xffff800000000000ULL;
			else
				m = 0x00007fffffffffffULL;
		}
	}
	return m;
}
#endif

static noinline INT64 tms57002_macc_to_output_0(tms57002_t *s, INT64 rounding, UINT64 rmask)
{
	INT64 m = s->macc;
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
		s->st1 |= ST1_MOV;
	}
	return m;
}

static noinline INT64 tms57002_macc_to_output_1(tms57002_t *s, INT64 rounding, UINT64 rmask)
{
	INT64 m = s->macc;
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
		s->st1 |= ST1_MOV;
	}
	return m;
}

static noinline INT64 tms57002_macc_to_output_2(tms57002_t *s, INT64 rounding, UINT64 rmask)
{
	INT64 m = s->macc;
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
		s->st1 |= ST1_MOV;
	}
	return m;
}

static noinline INT64 tms57002_macc_to_output_3(tms57002_t *s, INT64 rounding, UINT64 rmask)
{
	INT64 m = s->macc;
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
		s->st1 |= ST1_MOV;
	}
	return m;
}

static noinline INT64 tms57002_macc_to_output_0s(tms57002_t *s, INT64 rounding, UINT64 rmask)
{
	INT64 m = s->macc;
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
		s->st1 |= ST1_MOV;
		if(m & 0x8000000000000ULL)
			m = 0xffff800000000000ULL;
		else
			m = 0x00007fffffffffffULL;
	}
	return m;
}

static noinline INT64 tms57002_macc_to_output_1s(tms57002_t *s, INT64 rounding, UINT64 rmask)
{
	INT64 m = s->macc;
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
		s->st1 |= ST1_MOV;
		if(m & 0x8000000000000ULL)
			m = 0xffff800000000000ULL;
		else
			m = 0x00007fffffffffffULL;
	}
	return m;
}

static noinline INT64 tms57002_macc_to_output_2s(tms57002_t *s, INT64 rounding, UINT64 rmask)
{
	INT64 m = s->macc;
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
		s->st1 |= ST1_MOV;
		if(m & 0x8000000000000ULL)
			m = 0xffff800000000000ULL;
		else
			m = 0x00007fffffffffffULL;
	}
	return m;
}

static noinline INT64 tms57002_macc_to_output_3s(tms57002_t *s, INT64 rounding, UINT64 rmask)
{
	INT64 m = s->macc;
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
		s->st1 |= ST1_MOV;
		if(m & 0x8000000000000ULL)
			m = 0xffff800000000000ULL;
		else
			m = 0x00007fffffffffffULL;
	}
	return m;
}

#ifdef UNUSED_FUNCTION
static INT64 tms57002_check_macc_overflow(tms57002_t *s)
{
	INT64 m = s->macc;
	UINT64 m1;
	int over = 0;

	// Overflow detection
	switch((s->st1 & ST1_SFMO) >> ST1_SFMO_SHIFT) {
	case 0: // sfmo 0
		m1 = m & 0xf800000000000ULL;
		if(m1 && m1 != 0xf800000000000ULL)
			over = 1;
		break;
	case 1: // sfmo 2
		m1 = m & 0xfe00000000000ULL;
		if(m1 && m1 != 0xfe00000000000ULL)
			over = 1;
		break;
	case 2: // sfmo 4
		m1 = m & 0xff80000000000ULL;
		if(m1 && m1 != 0xff80000000000ULL)
			over = 1;
		break;
	case 3: // sfmo -8
		break;
	}

	// Overflow handling
	if(over) {
		s->st1 |= ST1_MOV;
		if(s->st1 & ST1_MOVM) {
			if(m & 0x8000000000000ULL)
				m = 0xffff800000000000ULL;
			else
				m = 0x00007fffffffffffULL;
		}
	}
	return m;
}
#endif

static noinline INT64 tms57002_check_macc_overflow_0(tms57002_t *s)
{
	INT64 m = s->macc;
	UINT64 m1;

	// Overflow detection
	m1 = m & 0xf800000000000ULL;
	if(m1 && m1 != 0xf800000000000ULL) {
		s->st1 |= ST1_MOV;
	}
	return m;
}

static noinline INT64 tms57002_check_macc_overflow_1(tms57002_t *s)
{
	INT64 m = s->macc;
	UINT64 m1;

	// Overflow detection
	m1 = m & 0xfe00000000000ULL;
	if(m1 && m1 != 0xfe00000000000ULL) {
		s->st1 |= ST1_MOV;
	}
	return m;
}

static noinline INT64 tms57002_check_macc_overflow_2(tms57002_t *s)
{
	INT64 m = s->macc;
	UINT64 m1;

	// Overflow detection
	m1 = m & 0xff80000000000ULL;
	if(m1 && m1 != 0xff80000000000ULL) {
		s->st1 |= ST1_MOV;
	}
	return m;
}

static INT64 tms57002_check_macc_overflow_3(tms57002_t *s)
{
	return s->macc;
}

static noinline INT64 tms57002_check_macc_overflow_0s(tms57002_t *s)
{
	INT64 m = s->macc;
	UINT64 m1;

	// Overflow detection
	m1 = m & 0xf800000000000ULL;
	if(m1 && m1 != 0xf800000000000ULL) {
		s->st1 |= ST1_MOV;
		if(m & 0x8000000000000ULL)
			m = 0xffff800000000000ULL;
		else
			m = 0x00007fffffffffffULL;
	}
	return m;
}

static noinline INT64 tms57002_check_macc_overflow_1s(tms57002_t *s)
{
	INT64 m = s->macc;
	UINT64 m1;

	// Overflow detection
	m1 = m & 0xfe00000000000ULL;
	if(m1 && m1 != 0xfe00000000000ULL) {
		s->st1 |= ST1_MOV;
		if(m & 0x8000000000000ULL)
			m = 0xffff800000000000ULL;
		else
			m = 0x00007fffffffffffULL;
	}
	return m;
}

static noinline INT64 tms57002_check_macc_overflow_2s(tms57002_t *s)
{
	INT64 m = s->macc;
	UINT64 m1;

	// Overflow detection
	m1 = m & 0xff80000000000ULL;
	if(m1 && m1 != 0xff80000000000ULL) {
		s->st1 |= ST1_MOV;
		if(m & 0x8000000000000ULL)
			m = 0xffff800000000000ULL;
		else
			m = 0x00007fffffffffffULL;
	}
	return m;
}

static INT64 tms57002_check_macc_overflow_3s(tms57002_t *s)
{
	return s->macc;
}

#ifdef UNUSED_FUNCTION
static INT64 tms57002_macc_to_loop(tms57002_t *s)
{
	INT64 m = s->macc;

	// sfma shifting
	switch((s->st1 & ST1_SFMA) >> ST1_SFMA_SHIFT) {
	case 0: // sfma 0
		break;
	case 1: // sfma 2
		m <<= 2;
		break;
	case 2: // sfma 4
		m <<= 4;
		break;
	case 3: // sfma -16
		if(m & 0x8000000000000ULL)
			m |= 0xfff0000000000000ULL;
		m >>= 16;
		break;
	}

	return m;
}

static void tms57002_execute_cat1(tms57002_t *s, UINT32 opcode)
{
	UINT32 c, d;
	INT64 r;
	switch(opcode >> 18) {
	case 0x00: // nop
		break;

#define INTRP1
#include "cpu/tms57002/tms57002.inc"
#undef INTRP1

	default:
		fatalerror("Unhandled case in tms57002_execute_cat1");
	}
}

static void tms57002_execute_cat2_pre(tms57002_t *s, UINT32 opcode)
{
	switch((opcode >> 11) & 0x7f) {
	case 0x00: // nop
		break;

#define INTRP2A
#include "cpu/tms57002/tms57002.inc"
#undef INTRP2A

	default:
		fatalerror("Unhandled case in tms57002_execute_cat2_pre");
	}
}

static void tms57002_execute_cat2_post(tms57002_t *s, UINT32 opcode)
{
	UINT32 c;
	switch((opcode >> 11) & 0x7f) {
	case 0x00: // nop
		break;

#define INTRP2B
#include "cpu/tms57002/tms57002.inc"
#undef INTRP2B

	default:
		fatalerror("Unhandled case in tms57002_execute_cat2_post");
	}
}

static void tms57002_execute_cat3(tms57002_t *s, UINT32 opcode)
{
	switch((opcode >> 11) & 0x7f) {
	case 0x00: // nop
		break;

#define INTRP3
#include "cpu/tms57002/tms57002.inc"
#undef INTRP3

	default:
		fatalerror("Unhandled case in tms57002_execute_cat3");
	}
}

void tms57002_execute(tms57002_t *s)
{
	while(!(s->sti & (S_IDLE | IN_PLOAD | IN_CLOAD))) {
		UINT32 opcode = memory_read_dword_32le(s->program, s->pc << 2);

		if(s->sti & (S_READ|S_WRITE)) {
			if(s->sti & S_READ)
				tms57002_xm_step_read(s);
			else
				tms57002_xm_step_write(s);
		}

		if((opcode & 0xfc0000) == 0xfc0000)
			tms57002_execute_cat3(s, opcode);
		else {
			tms57002_execute_cat2_pre(s, opcode);
			tms57002_execute_cat1(s, opcode);
			tms57002_execute_cat2_post(s, opcode);
		}

		if(s->rptc)
			s->rptc--;
		else if(s->sti & S_BRANCH)
			s->sti &= ~S_BRANCH;
		else
			s->pc++; // Wraps if it reaches 256

		if(s->rptc_next) {
			s->rptc = s->rptc_next;
			s->rptc_next = 0;
		}
	}
}
#endif

INLINE int xmode(UINT32 opcode, char type)
{
	if(((opcode & 0x400) && (type == 'c')) || (!(opcode & 0x400) && (type == 'd'))) {
		if(opcode & 0x100)
			return 0;
		else if(opcode & 0x80)
			return 2;
		else
			return 1;
	} else if(opcode & 0x200)
		return 2;
	else
		return 1;
}

INLINE int sfao(UINT32 st1)
{
	return st1 & ST1_SFAO ? 1 : 0;
}

INLINE int dbp(UINT32 st1)
{
	return st1 & ST1_DBP ? 1 : 0;
}

INLINE int crm(UINT32 st1)
{
	return (st1 & ST1_CRM) >> ST1_CRM_SHIFT;
}

INLINE int sfai(UINT32 st1)
{
	return st1 & ST1_SFAI ? 1 : 0;
}

INLINE int sfmo(UINT32 st1)
{
	return (st1 & ST1_SFMO) >> ST1_SFMO_SHIFT;
}

INLINE int rnd(UINT32 st1)
{
	return (st1 & ST1_RND) >> ST1_RND_SHIFT;
}

INLINE int movm(UINT32 st1)
{
	return st1 & ST1_MOVM ? 1 : 0;
}

INLINE int sfma(UINT32 st1)
{
	return (st1 & ST1_SFMA) >> ST1_SFMA_SHIFT;
}

static void tms57002_cache_flush(tms57002_t *s)
{
	int i;
	s->cache.hused = s->cache.iused = 0;
	for(i=0; i != 256; i++)
		s->cache.hashbase[i] = -1;
	for(i=0; i != HBS; i++) {
		s->cache.hashnode[i].st1 = 0;
		s->cache.hashnode[i].ipc = -1;
		s->cache.hashnode[i].next = -1;
	}
	for(i=0; i != IBS; i++) {
		s->cache.inst[i].op = 0;
		s->cache.inst[i].next = -1;
		s->cache.inst[i].param = 0;
	}
}

static void tms57002_decode_error(tms57002_t *s, UINT32 opcode)
{
	char buf[256];
	UINT8 opr[3];
	if(s->unsupported_inst_warning)
		return;

	s->unsupported_inst_warning = 1;
	opr[0] = opcode;
	opr[1] = opcode >> 8;
	opr[2] = opcode >> 16;

	CPU_DISASSEMBLE_NAME(tms57002)(0, buf, s->pc, opr, opr, 0);
	popmessage("tms57002: %s - Contact Mamedev", buf);
}

static void tms57002_decode_cat1(tms57002_t *s, UINT32 opcode, unsigned short *op, cstate *cs)
{
	switch(opcode >> 18) {
	case 0x00: // nop
		break;

#define CDEC1
#include "cpu/tms57002/tms57002.inc"
#undef CDEC1

	default:
		tms57002_decode_error(s, opcode);
		break;
	}
}

static void tms57002_decode_cat2_pre(tms57002_t *s, UINT32 opcode, unsigned short *op, cstate *cs)
{
	switch((opcode >> 11) & 0x7f) {
	case 0x00: // nop
		break;

#define CDEC2A
#include "cpu/tms57002/tms57002.inc"
#undef CDEC2A

	default:
		tms57002_decode_error(s, opcode);
		break;
	}
}

static void tms57002_decode_cat2_post(tms57002_t *s, UINT32 opcode, unsigned short *op, cstate *cs)
{
	switch((opcode >> 11) & 0x7f) {
	case 0x00: // nop
		break;

#define CDEC2B
#include "cpu/tms57002/tms57002.inc"
#undef CDEC2B

	default:
		tms57002_decode_error(s, opcode);
		break;
	}
}

static void tms57002_decode_cat3(tms57002_t *s, UINT32 opcode, unsigned short *op, cstate *cs)
{
	switch((opcode >> 11) & 0x7f) {
	case 0x00: // nop
		break;

#define CDEC3
#include "cpu/tms57002/tms57002.inc"
#undef CDEC3

	default:
		tms57002_decode_error(s, opcode);
		break;
	}
}

static void tms57002_add_one(tms57002_t *s, cstate *cs, unsigned short op, UINT8 param)
{
	short ipc = s->cache.iused++;
	s->cache.inst[ipc].op = op;
	s->cache.inst[ipc].param = param;
	s->cache.inst[ipc].next = -1;
	if(cs->ipc != -1)
		s->cache.inst[cs->ipc].next = ipc;
	cs->ipc = ipc;
	if(cs->hnode != -1) {
		s->cache.hashnode[cs->hnode].ipc = ipc;
		cs->hnode = -1;
	}
}

static void tms57002_decode_one(tms57002_t *s, UINT32 opcode, cstate *cs, void (*dec)(tms57002_t *s, UINT32 opcode, unsigned short *op, cstate *cs))
{
	unsigned short op = 0;
	dec(s, opcode, &op, cs);
	if(!op)
		return;
	tms57002_add_one(s, cs, op, opcode & 0xff);
}

static short tms57002_get_hash(tms57002_t *s, unsigned char adr, UINT32 st1, short *pnode)
{
	short hnode;
	st1 &= ST1_CACHE;
	*pnode = -1;
	hnode = s->cache.hashbase[adr];
	while(hnode != -1) {
		if(s->cache.hashnode[hnode].st1 == st1)
			return s->cache.hashnode[hnode].ipc;
		*pnode = hnode;
		hnode = s->cache.hashnode[hnode].next;
	}
	return -1;
}

static short tms57002_get_hashnode(tms57002_t *s, unsigned char adr, UINT32 st1, short pnode)
{
	short hnode = s->cache.hused++;
	s->cache.hashnode[hnode].st1 = st1 & ST1_CACHE;
	s->cache.hashnode[hnode].ipc = -1;
	s->cache.hashnode[hnode].next = -1;
	if(pnode == -1)
		s->cache.hashbase[adr] = hnode;
	else
		s->cache.hashnode[pnode].next = hnode;
	return hnode;
}

static int tms57002_decode_get_pc(tms57002_t *s)
{
	UINT32 st1 = s->st1;
	short pnode, res;
	cstate cs;
	UINT8 adr = s->pc;

	res = tms57002_get_hash(s, adr, st1, &pnode);
	if(res != -1)
		return res;

	if(HBS - s->cache.hused < 256 || IBS - s->cache.iused < 256*3) {
		tms57002_cache_flush(s);
		pnode = -1;
	}

	cs.hnode = res = tms57002_get_hashnode(s, adr, st1, pnode);
	cs.ipc = -1;
	cs.branch = 0;

	for(;;) {
		short ipc;
		UINT32 opcode = memory_read_dword_32le(s->program, adr << 2);

		if((opcode & 0xfc0000) == 0xfc0000)
			tms57002_decode_one(s, opcode, &cs, tms57002_decode_cat3);
		else {
			tms57002_decode_one(s, opcode, &cs, tms57002_decode_cat2_pre);
			tms57002_decode_one(s, opcode, &cs, tms57002_decode_cat1);
			tms57002_decode_one(s, opcode, &cs, tms57002_decode_cat2_post);
		}
		tms57002_add_one(s, &cs, 0, 0);

		if(cs.branch)
			break;

		adr++;
		ipc = tms57002_get_hash(s, adr, st1, &pnode);
		if(ipc != -1) {
			s->cache.inst[cs.ipc].next = ipc;
			break;
		}
		cs.hnode = tms57002_get_hashnode(s, adr, s->st1, pnode);
	}

	s->st1 = st1;
	return s->cache.hashnode[res].ipc;
}

static CPU_EXECUTE(tms57002)
{
	tms57002_t *s = get_safe_token(device);
	int initial_cycles = cycles;
	int ipc = -1;

	while(cycles > 0 && !(s->sti & (S_IDLE | IN_PLOAD | IN_CLOAD))) {
		int iipc;

		debugger_instruction_hook(device, s->pc);

		if(ipc == -1)
			ipc = tms57002_decode_get_pc(s);

		iipc = ipc;

		if(s->sti & (S_READ|S_WRITE)) {
			if(s->sti & S_READ)
				tms57002_xm_step_read(s);
			else
				tms57002_xm_step_write(s);
		}

		for(;;) {
			UINT32 c, d;
			INT64 r;
			const icd *i = s->cache.inst + ipc;

			ipc = i->next;
			switch(i->op) {
			case 0:
				goto inst;

#define CINTRP
#include "cpu/tms57002/tms57002.inc"
#undef CINTRP

			default:
				fatalerror("Unhandled opcode in tms57002_execute");
			}
		}
	inst:
		cycles--;

		if(s->rptc) {
			s->rptc--;
			ipc = iipc;
		} else if(s->sti & S_BRANCH) {
			s->sti &= ~S_BRANCH;
			ipc = -1;
		} else
			s->pc++; // Wraps if it reaches 256, next wraps too

		if(s->rptc_next) {
			s->rptc = s->rptc_next;
			s->rptc_next = 0;
		}
	}

	if(cycles > 0)
		cycles = 0;
	return initial_cycles - cycles;
}

static CPU_INIT(tms57002)
{
	tms57002_t *s = get_safe_token(device);
	tms57002_cache_flush(s);
	s->sti = S_IDLE;
	s->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	s->data    = memory_find_address_space(device, ADDRESS_SPACE_DATA);
}


static CPU_SET_INFO(tms57002)
{
}

static ADDRESS_MAP_START(internal_pgm, ADDRESS_SPACE_PROGRAM, 32)
	AM_RANGE(0x000, 0x3ff) AM_RAM
ADDRESS_MAP_END

CPU_GET_INFO(tms57002)
{
	tms57002_t *s = (device != NULL && device->token != NULL) ? get_safe_token(device) : NULL;

	switch(state) {
	case CPUINFO_INT_CONTEXT_SIZE:				info->i = sizeof(tms57002_t); break;
	case CPUINFO_INT_INPUT_LINES:				info->i = 0; break;
	case DEVINFO_INT_ENDIANNESS:				info->i = ENDIANNESS_LITTLE; break;
	case CPUINFO_INT_CLOCK_MULTIPLIER:			info->i = 1; break;
	case CPUINFO_INT_CLOCK_DIVIDER:				info->i = 1; break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:		info->i = 4; break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:		info->i = 4; break;
	case CPUINFO_INT_MIN_CYCLES:				info->i = 1; break;
	case CPUINFO_INT_MAX_CYCLES:				info->i = 3; break;
	case CPUINFO_INT_DATABUS_WIDTH_PROGRAM:		info->i = 32; break;
	case CPUINFO_INT_ADDRBUS_WIDTH_PROGRAM:		info->i = 8; break;
	case CPUINFO_INT_ADDRBUS_SHIFT_PROGRAM:		info->i = -2; break;
	case CPUINFO_INT_DATABUS_WIDTH_DATA:		info->i = 8; break;
	case CPUINFO_INT_ADDRBUS_WIDTH_DATA:		info->i = 20; break;
	case CPUINFO_INT_ADDRBUS_SHIFT_DATA:		info->i = 0; break;
	case CPUINFO_INT_DATABUS_WIDTH_IO:			info->i = 0; break;
	case CPUINFO_INT_ADDRBUS_WIDTH_IO:			info->i = 0; break;
	case CPUINFO_INT_ADDRBUS_SHIFT_IO:			info->i = 0; break;
	case CPUINFO_FCT_SET_INFO:					info->setinfo = CPU_SET_INFO_NAME(tms57002); break;
	case CPUINFO_FCT_INIT:						info->init = CPU_INIT_NAME(tms57002); break;
	case CPUINFO_FCT_RESET:						info->reset = CPU_RESET_NAME(tms57002); break;
	case CPUINFO_FCT_EXECUTE:					info->execute = CPU_EXECUTE_NAME(tms57002); break;
	case CPUINFO_FCT_DISASSEMBLE:				info->disassemble = CPU_DISASSEMBLE_NAME(tms57002); break;
	case CPUINFO_PTR_INSTRUCTION_COUNTER:		info->icount = &s->icount; break;
	case CPUINFO_PTR_INTERNAL_MEMORY_MAP_PROGRAM:info->internal_map32 = ADDRESS_MAP_NAME(internal_pgm); break;
	case DEVINFO_STR_NAME:						strcpy( info->s, "TMS57002" ); break;
	case DEVINFO_STR_FAMILY:				strcpy( info->s, "Texas Instruments TMS57002 (DASP)" ); break;
	case DEVINFO_STR_VERSION:				strcpy( info->s, "1.0" ); break;
	case DEVINFO_STR_SOURCE_FILE:					strcpy( info->s, __FILE__ ); break;
	case DEVINFO_STR_CREDITS:				strcpy( info->s, "Copyright Olivier Galibert" ); break;
	}
}
