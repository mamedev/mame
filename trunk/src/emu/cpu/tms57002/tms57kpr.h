#ifndef TMS57KPRIVATE_H
#define TMS57KPRIVATE_H

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

	address_space *program, *data;
	int icount;
	int unsupported_inst_warning;
} tms57002_t;

void tms57002_decode_cat1(tms57002_t *s, UINT32 opcode, unsigned short *op, cstate *cs);
void tms57002_decode_cat2_pre(tms57002_t *s, UINT32 opcode, unsigned short *op, cstate *cs);
void tms57002_decode_cat3(tms57002_t *s, UINT32 opcode, unsigned short *op, cstate *cs);
void tms57002_decode_cat2_post(tms57002_t *s, UINT32 opcode, unsigned short *op, cstate *cs);

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

#endif
