// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    am29000.c
    Core implementation of the Am29000 emulator

    Written by Philip Bennett

    Features missing:
    * MMU
    * Some instructions
    * Various exceptions

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "am29000.h"


const device_type AM29000 = &device_creator<am29000_cpu_device>;


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define PFLAG_FETCH_EN              (1 << 0)
#define PFLAG_DECODE_EN             (1 << 1)
#define PFLAG_EXECUTE_EN            (1 << 2)
#define PFLAG_WRITEBACK_EN          (1 << 3)
#define PFLAG_IRQ                   (1 << 4)
#define PFLAG_LOADSTORE             (1 << 5)
#define PFLAG_MULTI_LOADSTORE       (1 << 6)
#define PFLAG_JUMP                  (1 << 7)
#define PFLAG_JUMP2                 (1 << 8)
#define PFLAG_IRET                  (1 << 9)
#define PFLAG_TIMER_LOADED          (1 << 10)

#define PFLAG_RA_DEPENDENCY         (1 << 26)
#define PFLAG_RB_DEPENDENCY         (1 << 27)

#define PFLAG_MEM_MULTIPLE          (1 << 29)
#define PFLAG_REG_WRITEBACK         (1 << 30)
#define PFLAG_MEM_WRITEBACK         (1 << 31)

#define MMU_PROGRAM_ACCESS          (0)
#define MMU_DATA_ACCESS             (1)

#define FREEZE_MODE                 (m_cps & CPS_FZ)
#define SUPERVISOR_MODE             (m_cps & CPS_SM)
#define USER_MODE                   (~m_cps & CPS_SM)
#define REGISTER_IS_PROTECTED(x)    (m_rbp & (1 << ((x) >> 4)))

#define INST_RB_FIELD(x)            ((x) & 0xff)
#define INST_RA_FIELD(x)            (((x) >> 8) & 0xff)
#define INST_RC_FIELD(x)            (((x) >> 16) & 0xff)
#define INST_SA_FIELD(x)            (((x) >> 8) & 0xff)

#define FIELD_RA                    0
#define FIELD_RB                    1
#define FIELD_RC                    2

#define SIGNAL_EXCEPTION(x)         (signal_exception(x))


#define GET_ALU_FC                  ((m_alu >> ALU_FC_SHIFT) & ALU_FC_MASK)
#define GET_ALU_BP                  ((m_alu >> ALU_BP_SHIFT) & ALU_BP_MASK)
#define GET_CHC_CR                  ((m_chc >> CHC_CR_SHIFT) & CHC_CR_MASK)

#define SET_ALU_FC(x)               do { m_alu &= ~(ALU_FC_MASK << ALU_FC_SHIFT); m_alu |= ((x) & ALU_FC_MASK) << ALU_FC_SHIFT; } while(0)
#define SET_ALU_BP(x)               do { m_alu &= ~(ALU_BP_MASK << ALU_BP_SHIFT); m_alu |= ((x) & ALU_BP_MASK) << ALU_BP_SHIFT; } while(0)
#define SET_CHC_CR(x)               do { m_chc &= ~(CHC_CR_MASK << CHC_CR_SHIFT); m_chc |= ((x) & CHC_CR_MASK) << CHC_CR_SHIFT; } while(0)


/***************************************************************************
    STATE ACCESSORS
***************************************************************************/

am29000_cpu_device::am29000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, AM29000, "AMD Am29000", tag, owner, clock, "am29000", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 32, 32, 0)
	, m_io_config("io", ENDIANNESS_BIG, 32, 32, 0)
	, m_data_config("data", ENDIANNESS_BIG, 32, 32, 0)
{
	memset( m_r, 0, sizeof(m_r) );
	memset( m_tlb, 0, sizeof(m_tlb) );
	m_vab = 0;
	m_ops = 0;
	m_cha = 0;
	m_chd = 0;
	m_chc = 0;
	m_rbp = 0;
	m_tmc = 0;
	m_tmr = 0;
	m_pc0 = 0;
	m_pc1 = 0;
	m_pc2 = 0;
	m_mmu = 0;
	m_lru = 0;
	m_ipc = 0;
	m_ipa = 0;
	m_ipb = 0;
	m_q = 0;
	m_alu = 0;
	m_fpe = 0;
	m_inte = 0;
	m_fps = 0;
	memset( m_exception_queue, 0, sizeof( m_exception_queue) );;
	m_irq_active = 0;
	m_irq_lines = 0;
	m_exec_ir = 0;
	m_next_ir = 0;
	m_pl_flags = 0;
	m_iret_pc = 0;
	m_exec_pc = 0;
	m_next_pc = 0;
}


void am29000_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_data = &space(AS_DATA);
	m_datadirect = &m_data->direct();
	m_io = &space(AS_IO);
	m_cfg = (PRL_AM29000 | PRL_REV_D) << CFG_PRL_SHIFT;

	/* Register state for saving */
	save_item(NAME(m_pc));
	save_item(NAME(m_r));
	save_item(NAME(m_tlb));

	save_item(NAME(m_vab));
	save_item(NAME(m_ops));
	save_item(NAME(m_cps));
	save_item(NAME(m_cfg));
	save_item(NAME(m_cha));
	save_item(NAME(m_chd));
	save_item(NAME(m_chc));
	save_item(NAME(m_rbp));
	save_item(NAME(m_tmc));
	save_item(NAME(m_tmr));
	save_item(NAME(m_pc0));
	save_item(NAME(m_pc1));
	save_item(NAME(m_pc2));
	save_item(NAME(m_mmu));
	save_item(NAME(m_lru));

	save_item(NAME(m_ipc));
	save_item(NAME(m_ipa));
	save_item(NAME(m_ipb));
	save_item(NAME(m_q));

	save_item(NAME(m_alu));
	save_item(NAME(m_fpe));
	save_item(NAME(m_inte));
	save_item(NAME(m_fps));

	save_item(NAME(m_exceptions));
	save_item(NAME(m_exception_queue));

	save_item(NAME(m_irq_active));
	save_item(NAME(m_irq_lines));

	save_item(NAME(m_exec_ir));
	save_item(NAME(m_next_ir));

	save_item(NAME(m_pl_flags));
	save_item(NAME(m_next_pl_flags));

	save_item(NAME(m_iret_pc));
	save_item(NAME(m_exec_pc));
	save_item(NAME(m_next_pc));

	// Register state for debugger
	state_add( AM29000_PC,   "PC",   m_pc     ).formatstr("%08X");
	state_add( AM29000_VAB,  "VAB",  m_vab    ).formatstr("%08X");
	state_add( AM29000_OPS,  "OPS",  m_ops    ).formatstr("%08X");
	state_add( AM29000_CPS,  "CPS",  m_cps    ).formatstr("%08X");
	state_add( AM29000_CFG,  "CFG",  m_cfg    ).formatstr("%08X");
	state_add( AM29000_CHA,  "CHA",  m_cha    ).formatstr("%08X");
	state_add( AM29000_CHD,  "CHD",  m_chd    ).formatstr("%08X");
	state_add( AM29000_CHC,  "CHC",  m_chc    ).formatstr("%08X");
	state_add( AM29000_RBP,  "RBP",  m_rbp    ).formatstr("%08X");
	state_add( AM29000_TMC,  "TMC",  m_tmc    ).formatstr("%08X");
	state_add( AM29000_TMR,  "TMR",  m_tmr    ).formatstr("%08X");
	state_add( AM29000_PC0,  "PC0",  m_pc0    ).formatstr("%08X");
	state_add( AM29000_PC1,  "PC1",  m_pc1    ).formatstr("%08X");
	state_add( AM29000_PC2,  "PC2",  m_pc2    ).formatstr("%08X");
	state_add( AM29000_MMU,  "MMU",  m_mmu    ).formatstr("%08X");
	state_add( AM29000_LRU,  "LRU",  m_lru    ).formatstr("%08X");
	state_add( AM29000_IPC,  "IPC",  m_ipc    ).formatstr("%08X");
	state_add( AM29000_IPA,  "IPA",  m_ipa    ).formatstr("%08X");
	state_add( AM29000_IPB,  "IPB",  m_ipb    ).formatstr("%08X");
	state_add( AM29000_Q,    "Q",    m_q      ).formatstr("%08X");
	state_add( AM29000_ALU,  "ALU",  m_alu    ).formatstr("%08X");
//  state_add( AM29000_BP,   "BP",   GET_ALU_BP).formatstr("%08X");
//  state_add( AM29000_FC,   "FC",   GET_ALU_FC).formatstr("%08X");
//  state_add( AM29000_CR,   "CR",   GET_CHC_CR).formatstr("%08X");
	state_add( AM29000_FPE,  "FPE",  m_fpe    ).formatstr("%08X");
	state_add( AM29000_INTE, "INTE", m_inte   ).formatstr("%08X");
	state_add( AM29000_FPS,  "FPS",  m_fps    ).formatstr("%08X");
	state_add( AM29000_R1,   "R1",   m_r[1]   ).formatstr("%08X");
	state_add( AM29000_R64,  "R64",  m_r[64]  ).formatstr("%08X");
	state_add( AM29000_R65,  "R65",  m_r[65]  ).formatstr("%08X");
	state_add( AM29000_R66,  "R66",  m_r[66]  ).formatstr("%08X");
	state_add( AM29000_R67,  "R67",  m_r[67]  ).formatstr("%08X");
	state_add( AM29000_R68,  "R68",  m_r[68]  ).formatstr("%08X");
	state_add( AM29000_R69,  "R69",  m_r[69]  ).formatstr("%08X");
	state_add( AM29000_R70,  "R70",  m_r[70]  ).formatstr("%08X");
	state_add( AM29000_R71,  "R71",  m_r[71]  ).formatstr("%08X");
	state_add( AM29000_R72,  "R72",  m_r[72]  ).formatstr("%08X");
	state_add( AM29000_R73,  "R73",  m_r[73]  ).formatstr("%08X");
	state_add( AM29000_R74,  "R74",  m_r[74]  ).formatstr("%08X");
	state_add( AM29000_R75,  "R75",  m_r[75]  ).formatstr("%08X");
	state_add( AM29000_R76,  "R76",  m_r[76]  ).formatstr("%08X");
	state_add( AM29000_R77,  "R77",  m_r[77]  ).formatstr("%08X");
	state_add( AM29000_R78,  "R78",  m_r[78]  ).formatstr("%08X");
	state_add( AM29000_R79,  "R79",  m_r[79]  ).formatstr("%08X");
	state_add( AM29000_R80,  "R80",  m_r[80]  ).formatstr("%08X");
	state_add( AM29000_R81,  "R81",  m_r[81]  ).formatstr("%08X");
	state_add( AM29000_R82,  "R82",  m_r[82]  ).formatstr("%08X");
	state_add( AM29000_R83,  "R83",  m_r[83]  ).formatstr("%08X");
	state_add( AM29000_R84,  "R84",  m_r[84]  ).formatstr("%08X");
	state_add( AM29000_R85,  "R85",  m_r[85]  ).formatstr("%08X");
	state_add( AM29000_R86,  "R86",  m_r[86]  ).formatstr("%08X");
	state_add( AM29000_R87,  "R87",  m_r[87]  ).formatstr("%08X");
	state_add( AM29000_R88,  "R88",  m_r[88]  ).formatstr("%08X");
	state_add( AM29000_R89,  "R89",  m_r[89]  ).formatstr("%08X");
	state_add( AM29000_R90,  "R90",  m_r[90]  ).formatstr("%08X");
	state_add( AM29000_R91,  "R91",  m_r[91]  ).formatstr("%08X");
	state_add( AM29000_R92,  "R92",  m_r[92]  ).formatstr("%08X");
	state_add( AM29000_R93,  "R93",  m_r[93]  ).formatstr("%08X");
	state_add( AM29000_R94,  "R94",  m_r[94]  ).formatstr("%08X");
	state_add( AM29000_R95,  "R95",  m_r[95]  ).formatstr("%08X");
	state_add( AM29000_R96,  "R96",  m_r[96]  ).formatstr("%08X");
	state_add( AM29000_R97,  "R97",  m_r[97]  ).formatstr("%08X");
	state_add( AM29000_R98,  "R98",  m_r[98]  ).formatstr("%08X");
	state_add( AM29000_R99,  "R99",  m_r[99]  ).formatstr("%08X");
	state_add( AM29000_R100, "R100", m_r[100] ).formatstr("%08X");
	state_add( AM29000_R101, "R101", m_r[101] ).formatstr("%08X");
	state_add( AM29000_R102, "R102", m_r[102] ).formatstr("%08X");
	state_add( AM29000_R103, "R103", m_r[103] ).formatstr("%08X");
	state_add( AM29000_R104, "R104", m_r[104] ).formatstr("%08X");
	state_add( AM29000_R105, "R105", m_r[105] ).formatstr("%08X");
	state_add( AM29000_R106, "R106", m_r[106] ).formatstr("%08X");
	state_add( AM29000_R107, "R107", m_r[107] ).formatstr("%08X");
	state_add( AM29000_R108, "R108", m_r[108] ).formatstr("%08X");
	state_add( AM29000_R109, "R109", m_r[109] ).formatstr("%08X");
	state_add( AM29000_R110, "R110", m_r[110] ).formatstr("%08X");
	state_add( AM29000_R111, "R111", m_r[111] ).formatstr("%08X");
	state_add( AM29000_R112, "R112", m_r[112] ).formatstr("%08X");
	state_add( AM29000_R113, "R113", m_r[113] ).formatstr("%08X");
	state_add( AM29000_R114, "R114", m_r[114] ).formatstr("%08X");
	state_add( AM29000_R115, "R115", m_r[115] ).formatstr("%08X");
	state_add( AM29000_R116, "R116", m_r[116] ).formatstr("%08X");
	state_add( AM29000_R117, "R117", m_r[117] ).formatstr("%08X");
	state_add( AM29000_R118, "R118", m_r[118] ).formatstr("%08X");
	state_add( AM29000_R119, "R119", m_r[119] ).formatstr("%08X");
	state_add( AM29000_R120, "R120", m_r[120] ).formatstr("%08X");
	state_add( AM29000_R121, "R121", m_r[121] ).formatstr("%08X");
	state_add( AM29000_R122, "R122", m_r[122] ).formatstr("%08X");
	state_add( AM29000_R123, "R123", m_r[123] ).formatstr("%08X");
	state_add( AM29000_R124, "R124", m_r[124] ).formatstr("%08X");
	state_add( AM29000_R125, "R125", m_r[125] ).formatstr("%08X");
	state_add( AM29000_R126, "R126", m_r[126] ).formatstr("%08X");
	state_add( AM29000_R127, "R127", m_r[127] ).formatstr("%08X");
	state_add( AM29000_R128, "R128", m_r[128] ).formatstr("%08X");
	state_add( AM29000_R129, "R129", m_r[129] ).formatstr("%08X");
	state_add( AM29000_R130, "R130", m_r[130] ).formatstr("%08X");
	state_add( AM29000_R131, "R131", m_r[131] ).formatstr("%08X");
	state_add( AM29000_R132, "R132", m_r[132] ).formatstr("%08X");
	state_add( AM29000_R133, "R133", m_r[133] ).formatstr("%08X");
	state_add( AM29000_R134, "R134", m_r[134] ).formatstr("%08X");
	state_add( AM29000_R135, "R135", m_r[135] ).formatstr("%08X");
	state_add( AM29000_R136, "R136", m_r[136] ).formatstr("%08X");
	state_add( AM29000_R137, "R137", m_r[137] ).formatstr("%08X");
	state_add( AM29000_R138, "R138", m_r[138] ).formatstr("%08X");
	state_add( AM29000_R139, "R139", m_r[139] ).formatstr("%08X");
	state_add( AM29000_R140, "R140", m_r[140] ).formatstr("%08X");
	state_add( AM29000_R141, "R141", m_r[141] ).formatstr("%08X");
	state_add( AM29000_R142, "R142", m_r[142] ).formatstr("%08X");
	state_add( AM29000_R143, "R143", m_r[143] ).formatstr("%08X");
	state_add( AM29000_R144, "R144", m_r[144] ).formatstr("%08X");
	state_add( AM29000_R145, "R145", m_r[145] ).formatstr("%08X");
	state_add( AM29000_R146, "R146", m_r[146] ).formatstr("%08X");
	state_add( AM29000_R147, "R147", m_r[147] ).formatstr("%08X");
	state_add( AM29000_R148, "R148", m_r[148] ).formatstr("%08X");
	state_add( AM29000_R149, "R149", m_r[149] ).formatstr("%08X");
	state_add( AM29000_R150, "R150", m_r[150] ).formatstr("%08X");
	state_add( AM29000_R151, "R151", m_r[151] ).formatstr("%08X");
	state_add( AM29000_R152, "R152", m_r[152] ).formatstr("%08X");
	state_add( AM29000_R153, "R153", m_r[153] ).formatstr("%08X");
	state_add( AM29000_R154, "R154", m_r[154] ).formatstr("%08X");
	state_add( AM29000_R155, "R155", m_r[155] ).formatstr("%08X");
	state_add( AM29000_R156, "R156", m_r[156] ).formatstr("%08X");
	state_add( AM29000_R157, "R157", m_r[157] ).formatstr("%08X");
	state_add( AM29000_R158, "R158", m_r[158] ).formatstr("%08X");
	state_add( AM29000_R159, "R159", m_r[159] ).formatstr("%08X");
	state_add( AM29000_R160, "R160", m_r[160] ).formatstr("%08X");
	state_add( AM29000_R161, "R161", m_r[161] ).formatstr("%08X");
	state_add( AM29000_R162, "R162", m_r[162] ).formatstr("%08X");
	state_add( AM29000_R163, "R163", m_r[163] ).formatstr("%08X");
	state_add( AM29000_R164, "R164", m_r[164] ).formatstr("%08X");
	state_add( AM29000_R165, "R165", m_r[165] ).formatstr("%08X");
	state_add( AM29000_R166, "R166", m_r[166] ).formatstr("%08X");
	state_add( AM29000_R167, "R167", m_r[167] ).formatstr("%08X");
	state_add( AM29000_R168, "R168", m_r[168] ).formatstr("%08X");
	state_add( AM29000_R169, "R169", m_r[169] ).formatstr("%08X");
	state_add( AM29000_R170, "R170", m_r[170] ).formatstr("%08X");
	state_add( AM29000_R171, "R171", m_r[171] ).formatstr("%08X");
	state_add( AM29000_R172, "R172", m_r[172] ).formatstr("%08X");
	state_add( AM29000_R173, "R173", m_r[173] ).formatstr("%08X");
	state_add( AM29000_R174, "R174", m_r[174] ).formatstr("%08X");
	state_add( AM29000_R175, "R175", m_r[175] ).formatstr("%08X");
	state_add( AM29000_R176, "R176", m_r[176] ).formatstr("%08X");
	state_add( AM29000_R177, "R177", m_r[177] ).formatstr("%08X");
	state_add( AM29000_R178, "R178", m_r[178] ).formatstr("%08X");
	state_add( AM29000_R179, "R179", m_r[179] ).formatstr("%08X");
	state_add( AM29000_R180, "R180", m_r[180] ).formatstr("%08X");
	state_add( AM29000_R181, "R181", m_r[181] ).formatstr("%08X");
	state_add( AM29000_R182, "R182", m_r[182] ).formatstr("%08X");
	state_add( AM29000_R183, "R183", m_r[183] ).formatstr("%08X");
	state_add( AM29000_R184, "R184", m_r[184] ).formatstr("%08X");
	state_add( AM29000_R185, "R185", m_r[185] ).formatstr("%08X");
	state_add( AM29000_R186, "R186", m_r[186] ).formatstr("%08X");
	state_add( AM29000_R187, "R187", m_r[187] ).formatstr("%08X");
	state_add( AM29000_R188, "R188", m_r[188] ).formatstr("%08X");
	state_add( AM29000_R189, "R189", m_r[189] ).formatstr("%08X");
	state_add( AM29000_R190, "R190", m_r[190] ).formatstr("%08X");
	state_add( AM29000_R191, "R191", m_r[191] ).formatstr("%08X");
	state_add( AM29000_R192, "R192", m_r[192] ).formatstr("%08X");
	state_add( AM29000_R193, "R193", m_r[193] ).formatstr("%08X");
	state_add( AM29000_R194, "R194", m_r[194] ).formatstr("%08X");
	state_add( AM29000_R195, "R195", m_r[195] ).formatstr("%08X");
	state_add( AM29000_R196, "R196", m_r[196] ).formatstr("%08X");
	state_add( AM29000_R197, "R197", m_r[197] ).formatstr("%08X");
	state_add( AM29000_R198, "R198", m_r[198] ).formatstr("%08X");
	state_add( AM29000_R199, "R199", m_r[199] ).formatstr("%08X");
	state_add( AM29000_R200, "R200", m_r[200] ).formatstr("%08X");
	state_add( AM29000_R201, "R201", m_r[201] ).formatstr("%08X");
	state_add( AM29000_R202, "R202", m_r[202] ).formatstr("%08X");
	state_add( AM29000_R203, "R203", m_r[203] ).formatstr("%08X");
	state_add( AM29000_R204, "R204", m_r[204] ).formatstr("%08X");
	state_add( AM29000_R205, "R205", m_r[205] ).formatstr("%08X");
	state_add( AM29000_R206, "R206", m_r[206] ).formatstr("%08X");
	state_add( AM29000_R207, "R207", m_r[207] ).formatstr("%08X");
	state_add( AM29000_R208, "R208", m_r[208] ).formatstr("%08X");
	state_add( AM29000_R209, "R209", m_r[209] ).formatstr("%08X");
	state_add( AM29000_R210, "R210", m_r[210] ).formatstr("%08X");
	state_add( AM29000_R211, "R211", m_r[211] ).formatstr("%08X");
	state_add( AM29000_R212, "R212", m_r[212] ).formatstr("%08X");
	state_add( AM29000_R213, "R213", m_r[213] ).formatstr("%08X");
	state_add( AM29000_R214, "R214", m_r[214] ).formatstr("%08X");
	state_add( AM29000_R215, "R215", m_r[215] ).formatstr("%08X");
	state_add( AM29000_R216, "R216", m_r[216] ).formatstr("%08X");
	state_add( AM29000_R217, "R217", m_r[217] ).formatstr("%08X");
	state_add( AM29000_R218, "R218", m_r[218] ).formatstr("%08X");
	state_add( AM29000_R219, "R219", m_r[219] ).formatstr("%08X");
	state_add( AM29000_R220, "R220", m_r[220] ).formatstr("%08X");
	state_add( AM29000_R221, "R221", m_r[221] ).formatstr("%08X");
	state_add( AM29000_R222, "R222", m_r[222] ).formatstr("%08X");
	state_add( AM29000_R223, "R223", m_r[223] ).formatstr("%08X");
	state_add( AM29000_R224, "R224", m_r[224] ).formatstr("%08X");
	state_add( AM29000_R225, "R225", m_r[225] ).formatstr("%08X");
	state_add( AM29000_R226, "R226", m_r[226] ).formatstr("%08X");
	state_add( AM29000_R227, "R227", m_r[227] ).formatstr("%08X");
	state_add( AM29000_R228, "R228", m_r[228] ).formatstr("%08X");
	state_add( AM29000_R229, "R229", m_r[229] ).formatstr("%08X");
	state_add( AM29000_R230, "R230", m_r[230] ).formatstr("%08X");
	state_add( AM29000_R231, "R231", m_r[231] ).formatstr("%08X");
	state_add( AM29000_R232, "R232", m_r[232] ).formatstr("%08X");
	state_add( AM29000_R233, "R233", m_r[233] ).formatstr("%08X");
	state_add( AM29000_R234, "R234", m_r[234] ).formatstr("%08X");
	state_add( AM29000_R235, "R235", m_r[235] ).formatstr("%08X");
	state_add( AM29000_R236, "R236", m_r[236] ).formatstr("%08X");
	state_add( AM29000_R237, "R237", m_r[237] ).formatstr("%08X");
	state_add( AM29000_R238, "R238", m_r[238] ).formatstr("%08X");
	state_add( AM29000_R239, "R239", m_r[239] ).formatstr("%08X");
	state_add( AM29000_R240, "R240", m_r[240] ).formatstr("%08X");
	state_add( AM29000_R241, "R241", m_r[241] ).formatstr("%08X");
	state_add( AM29000_R242, "R242", m_r[242] ).formatstr("%08X");
	state_add( AM29000_R243, "R243", m_r[243] ).formatstr("%08X");
	state_add( AM29000_R244, "R244", m_r[244] ).formatstr("%08X");
	state_add( AM29000_R245, "R245", m_r[245] ).formatstr("%08X");
	state_add( AM29000_R246, "R246", m_r[246] ).formatstr("%08X");
	state_add( AM29000_R247, "R247", m_r[247] ).formatstr("%08X");
	state_add( AM29000_R248, "R248", m_r[248] ).formatstr("%08X");
	state_add( AM29000_R249, "R249", m_r[249] ).formatstr("%08X");
	state_add( AM29000_R250, "R250", m_r[250] ).formatstr("%08X");
	state_add( AM29000_R251, "R251", m_r[251] ).formatstr("%08X");
	state_add( AM29000_R252, "R252", m_r[252] ).formatstr("%08X");
	state_add( AM29000_R253, "R253", m_r[253] ).formatstr("%08X");
	state_add( AM29000_R254, "R254", m_r[254] ).formatstr("%08X");
	state_add( AM29000_R255, "R255", m_r[255] ).formatstr("%08X");

	state_add(STATE_GENPC, "curpc", m_pc).formatstr("%08X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_alu).formatstr("%13s").noshow();

	m_icountptr = &m_icount;
}


void am29000_cpu_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c%c%c%c%c%c|%3d", m_alu & ALU_V ? 'V' : '.',
													m_alu & ALU_Z ? 'Z' : '.',
													m_alu & ALU_N ? 'N' : '.',
													m_alu & ALU_C ? 'C' : '.',
													m_cps & CPS_IP ? 'I' : '.',
													m_cps & CPS_FZ ? 'F' : '.',
													m_cps & CPS_SM ? 'S' : 'U',
													m_cps & CPS_DI ? 'I' : '.',
													m_cps & CPS_DA ? 'D' : '.',
													(m_r[1] >> 2) & 0x7f);
			break;
	}
}


void am29000_cpu_device::device_reset()
{
	m_cps = CPS_FZ | CPS_RE | CPS_PD | CPS_PI | CPS_SM | CPS_DI | CPS_DA;
	m_cfg &= ~(CFG_DW | CFG_CD);
	m_chc &= ~CHC_CV;

	m_pc = 0;
	m_next_pl_flags = 0;
	m_exceptions = 0;
	m_irq_lines = 0;
}


void am29000_cpu_device::signal_exception(UINT32 type)
{
	m_exception_queue[m_exceptions++] = type;
}


void am29000_cpu_device::external_irq_check()
{
	int mask = (m_cps >> CPS_IM_SHIFT) & CPS_IM_MASK;
	int irq_en = !(m_cps & CPS_DI) && !(m_cps & CPS_DA);
	int i;

	/* Clear interrupt pending bit to begin with */
	m_cps &= ~CPS_IP;

	for (i = 0; i < 4; ++i)
	{
		if (!(m_irq_active & (1 << i)) && (m_irq_lines & (1 << i)))
		{
			if (irq_en)
			{
				if (i <= mask)
				{
					m_irq_active |= (1 << i);
					signal_exception(EXCEPTION_INTR0 + i);
					m_pl_flags |= PFLAG_IRQ;
					return;
				}
			}
			/* Set interrupt pending bit if interrupt was disabled */
			m_cps |= CPS_IP;
		}
		else
			m_irq_active &= ~(1 << i);
	}
}


UINT32 am29000_cpu_device::read_program_word(UINT32 address)
{
	/* TODO: ROM enable? */
	if (m_cps & CPS_PI || m_cps & CPS_RE)
		return m_direct->read_dword(address);
	else
	{
		fatalerror("Am29000 instruction MMU translation enabled!\n");
	}
	// never executed
	//return 0;
}

/***************************************************************************
    HELPER FUNCTIONS
***************************************************************************/

UINT32 am29000_cpu_device::get_abs_reg(UINT8 r, UINT32 iptr)
{
	if (r & 0x80)
	{
		/* Stack pointer access */
		r = ((m_r[1] >> 2) & 0x7f) + (r & 0x7f);
		r |= 0x80;
	}
	else if (r == 0)
	{
		/* Indirect pointer access */
		r = (iptr >> IPX_SHIFT) & 0xff;
	}
	else if (r > 1 && r < 64)
	{
		fatalerror("Am29000: Undefined register access (%d)\n", r);
	}
	return r;
}


/***************************************************************************
    CORE INCLUDE
***************************************************************************/

#include "am29ops.h"


/***************************************************************************
    PIPELINE STAGES
***************************************************************************/

void am29000_cpu_device::fetch_decode()
{
	UINT32 inst;
	UINT32 op_flags;

	inst = read_program_word(m_pc);
	m_next_ir = inst;

	op_flags = op_table[inst >> 24].flags;

	/* Illegal instruction */
	/* TODO: This should be checked at this point */
#if 0
	if (op_flags & IFLAG_ILLEGAL)
	{
		fatalerror("Illegal instruction: %x PC:%x PC0:%x PC1:%x\n", inst, m_pc, m_pc0, m_pc1);
		SIGNAL_EXCEPTION(EXCEPTION_ILLEGAL_OPCODE);
		return;
	}
#endif

	/* Privledge violations */
	if (USER_MODE)
	{
		if ((op_flags & IFLAG_SUPERVISOR_ONLY))
		{
			signal_exception(EXCEPTION_PROTECTION_VIOLATION);
			return;
		}

		if ((op_flags & IFLAG_SPR_ACCESS))
		{
			/* TODO: Is this the right place to check this? */
			if (INST_SA_FIELD(inst) < 128)
			{
				SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
				return;
			}
		}

		/* Register bank protection */
		if ((op_flags & IFLAG_RA_PRESENT) && REGISTER_IS_PROTECTED(INST_RA_FIELD(inst)))
		{
			SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
			return;
		}

		if ((op_flags & IFLAG_RB_PRESENT) && REGISTER_IS_PROTECTED(INST_RB_FIELD(inst)))
		{
			SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
			return;
		}

		if ((op_flags & IFLAG_RC_PRESENT) && REGISTER_IS_PROTECTED(INST_RC_FIELD(inst)))
		{
			SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
			return;
		}
	}

	if (m_pl_flags & PFLAG_IRET)
		m_next_pc = m_iret_pc;
	else
		m_next_pc += 4;
}

/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

void am29000_cpu_device::execute_run()
{
	UINT32 call_debugger = (machine().debug_flags & DEBUG_FLAG_ENABLED) != 0;

	external_irq_check();

	do
	{
		m_next_pl_flags = PFLAG_EXECUTE_EN;

		if (!FREEZE_MODE)
		{
			m_pc1 = m_pc0;
			m_pc0 = m_pc;
		}

		if (m_exceptions)
		{
			m_ops = m_cps;
			m_cps &= ~(CPS_TE | CPS_TP | CPS_TU | CPS_FZ | CPS_LK | CPS_WM | CPS_PD | CPS_PI | CPS_SM | CPS_DI | CPS_DA);
			m_cps |= (CPS_FZ | CPS_PD | CPS_PI | CPS_SM | CPS_DI | CPS_DA);

			if (m_pl_flags & PFLAG_IRET)
			{
				m_pc0 = m_iret_pc;
				m_pc1 = m_next_pc;
			}


			if (m_cfg & CFG_VF)
			{
				UINT32 vaddr = m_vab | m_exception_queue[0] * 4;
				UINT32 vect = m_datadirect->read_dword(vaddr);

				m_pc = vect & ~3;
				m_next_pc = m_pc;
			}
			else
			{
				fatalerror("Am29000: Non vectored interrupt fetch!\n");
			}

			m_exceptions = 0;
			m_pl_flags = 0;
		}

		if (call_debugger)
			debugger_instruction_hook(this, m_pc);

		fetch_decode();

		if (m_pl_flags & PFLAG_EXECUTE_EN)
		{
			if (!FREEZE_MODE)
				m_pc2 = m_pc1;

			(this->*op_table[m_exec_ir >> 24].opcode)();
		}

		m_exec_ir = m_next_ir;
		m_pl_flags = m_next_pl_flags;
		m_exec_pc = m_pc;
		m_pc = m_next_pc;
	} while (--m_icount > 0);
}


void am29000_cpu_device::execute_set_input(int inputnum, int state)
{
	if (state)
		m_irq_lines |= (1 << inputnum);
	else
		m_irq_lines &= ~(1 << inputnum);

	// TODO : CHECK IRQs
}


offs_t am29000_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( am29000 );
	return CPU_DISASSEMBLE_NAME(am29000)(this, buffer, pc, oprom, opram, options);
}
