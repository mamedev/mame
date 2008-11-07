///////////////////////////////////////////
// All the macros that are fit to print. //
///////////////////////////////////////////

/***************************************************************************
    ALU
***************************************************************************/
#define X		core.ALU.x.d
#define X1		core.ALU.x.w.h
#define X0		core.ALU.x.w.l
#define Y		core.ALU.y.d
#define Y1		core.ALU.y.w.h
#define Y0		core.ALU.y.w.l

#define A		core.ALU.a.q
#define A2		core.ALU.a.b.h4
#define A1		core.ALU.a.w.h
#define A0		core.ALU.a.w.l
#define B		core.ALU.b.q
#define B2		core.ALU.b.b.h4
#define B1		core.ALU.b.w.h
#define B0		core.ALU.b.w.l


/***************************************************************************
    AGU
***************************************************************************/

#define R0		core.AGU.r0
#define R1		core.AGU.r1
#define R2		core.AGU.r2
#define R3		core.AGU.r3

#define N0		core.AGU.n0
#define N1		core.AGU.n1
#define N2		core.AGU.n2
#define N3		core.AGU.n3

#define M0		core.AGU.m0
#define M1		core.AGU.m1
#define M2		core.AGU.m2
#define M3		core.AGU.m3

#define TEMP	core.AGU.temp

/***************************************************************************
    PCU
***************************************************************************/
static void pcu_reset(void);
#define PC  (core.PCU.pc)
#define LA  (core.PCU.la)
#define LC  (core.PCU.lc)
#define SR  (core.PCU.sr)
#define OMR (core.PCU.omr)
#define SP  (core.PCU.sp)
#define SS  (core.PCU.ss)

#define SSH	(SS[SP].w.h)
#define SSL	(SS[SP].w.l)

#define ST0		(SS[0].d)
#define ST1		(SS[1].d)
#define ST2		(SS[2].d)
#define ST3		(SS[3].d)
#define ST4		(SS[4].d)
#define ST5		(SS[5].d)
#define ST6		(SS[6].d)
#define ST7		(SS[7].d)
#define ST8		(SS[8].d)
#define ST9		(SS[9].d)
#define ST10	(SS[10].d)
#define ST11	(SS[11].d)
#define ST12	(SS[12].d)
#define ST13	(SS[13].d)
#define ST14	(SS[14].d)
#define ST15	(SS[15].d)

/* 1-25 STATUS REGISTER (SR) BITS */
/* MR */
static UINT8 LF_bit(void);
static UINT8 FV_bit(void);
//static UINT8 S_bits(void); #define s1BIT ((SR & 0x0800) != 0) #define s0BIT ((SR & 0x0400) != 0)
static UINT8 I_bits(void);
/* CCR */
static UINT8 S_bit(void);
static UINT8 L_bit(void);
static UINT8 E_bit(void);
static UINT8 U_bit(void);
static UINT8 N_bit(void);
static UINT8 Z_bit(void);
static UINT8 V_bit(void);
static UINT8 C_bit(void);

/* MR */
static void LF_bit_set(UINT8 value);
static void FV_bit_set(UINT8 value);
static void S_bits_set(UINT8 value);
static void I_bits_set(UINT8 value);
/* CCR */
static void S_bit_set(UINT8 value);
static void L_bit_set(UINT8 value);
static void E_bit_set(UINT8 value);
static void U_bit_set(UINT8 value);
static void N_bit_set(UINT8 value);
static void Z_bit_set(UINT8 value);
static void V_bit_set(UINT8 value);
static void C_bit_set(UINT8 value);

// TODO: Maybe some functions for Interrupt Mask and Scaling Mode go here?

/* 1-28 OPERATING MODE REGISTER (OMR) BITS */
//static UINT8 CD_bit(void); #define cdBIT ((OMR & 0x0080) != 0)
//static UINT8 SD_bit(void); #define sdBIT ((OMR & 0x0040) != 0)
//static UINT8 R_bit(void);  #define  rBIT ((OMR & 0x0020) != 0)
//static UINT8 SA_bit(void); #define saBIT ((OMR & 0x0010) != 0)
//static UINT8 MC_bit(void); #define mcBIT ((OMR & 0x0004) != 0)
static UINT8 MB_bit(void);
static UINT8 MA_bit(void);

static void CD_bit_set(UINT8 value);
static void SD_bit_set(UINT8 value);
static void R_bit_set(UINT8 value);
static void SA_bit_set(UINT8 value);
static void MC_bit_set(UINT8 value);
static void MB_bit_set(UINT8 value);
static void MA_bit_set(UINT8 value);

/* 1-27 STACK POINTER (SP) BITS */
static UINT8 UF_bit(void);
static UINT8 SE_bit(void);

//static void UF_bit_set(UINT8 value) {};
//static void SE_bit_set(UINT8 value) {};


// HACK - Bootstrap modes
#define BOOTSTRAP_OFF (0)
#define BOOTSTRAP_SSIX (1)
#define BOOTSTRAP_HI (2)


/* PROTOTYPES */
/* PCU IRQ goodies */
static void pcu_service_interrupts(void);

static void dsp56k_irq_table_init(void);
static void dsp56k_set_irq_source(UINT8 irq_num, UINT16 iv, const char* source);
static int dsp56k_get_irq_index_by_tag(const char* tag);

static void dsp56k_add_pending_interrupt(const char* name);		// Call me to add an interrupt to the queue

static void dsp56k_clear_pending_interrupts(void);
static int dsp56k_count_pending_interrupts(void);
static void dsp56k_sort_pending_interrupts(int num);
static INT8 dsp56k_get_irq_priority(int index);




/***************************************************************************
    MEMORY
***************************************************************************/

// Adjusts the documented address to match the offset in peripheral RAM
#define A2O(a) (a-0xffc0)

// Adjusts the offset in peripheral RAM to match the documented address
#define O2A(a) (a+0xffc0)

// The memory 'registers'
#define PBC      (dsp56k_peripheral_ram[A2O(0xffc0)])
#define PCC      (dsp56k_peripheral_ram[A2O(0xffc1)])
#define PBDDR    (dsp56k_peripheral_ram[A2O(0xffc2)])
#define PCDDR    (dsp56k_peripheral_ram[A2O(0xffc3)])
#define HCR      (dsp56k_peripheral_ram[A2O(0xffc4)])
#define COCR     (dsp56k_peripheral_ram[A2O(0xffc8)])
#define CRASSI0  (dsp56k_peripheral_ram[A2O(0xffd0)])
#define CRBSSI0  (dsp56k_peripheral_ram[A2O(0xffd1)])
#define CRASSI1  (dsp56k_peripheral_ram[A2O(0xffd8)])
#define CRBSSI1  (dsp56k_peripheral_ram[A2O(0xffd9)])
#define PLCR     (dsp56k_peripheral_ram[A2O(0xffdc)])
#define BCR      (dsp56k_peripheral_ram[A2O(0xffde)])
#define IPR      (dsp56k_peripheral_ram[A2O(0xffdf)])
#define PBD      (dsp56k_peripheral_ram[A2O(0xffe2)])
#define PCD      (dsp56k_peripheral_ram[A2O(0xffe3)])
#define HSR      (dsp56k_peripheral_ram[A2O(0xffe4)])
#define HTXHRX   (dsp56k_peripheral_ram[A2O(0xffe5)])
#define COSR     (dsp56k_peripheral_ram[A2O(0xffe8)])
#define CRXCTX   (dsp56k_peripheral_ram[A2O(0xffe9)])
#define TCR      (dsp56k_peripheral_ram[A2O(0xffec)])
#define TCTR     (dsp56k_peripheral_ram[A2O(0xffed)])
#define TCPR     (dsp56k_peripheral_ram[A2O(0xffee)])
#define TPR      (dsp56k_peripheral_ram[A2O(0xffef)])
#define TSRSSI0  (dsp56k_peripheral_ram[A2O(0xfff0)])
#define TRXSSI0  (dsp56k_peripheral_ram[A2O(0xfff1)])
#define RSMA0    (dsp56k_peripheral_ram[A2O(0xfff2)])
#define RSMB0    (dsp56k_peripheral_ram[A2O(0xfff3)])
#define TSMA0    (dsp56k_peripheral_ram[A2O(0xfff4)])
#define TSMB0    (dsp56k_peripheral_ram[A2O(0xfff5)])
#define TSRSSI1  (dsp56k_peripheral_ram[A2O(0xfff8)])
#define TRXSSI1  (dsp56k_peripheral_ram[A2O(0xfff9)])
#define RSMA1    (dsp56k_peripheral_ram[A2O(0xfffa)])
#define RSMB1    (dsp56k_peripheral_ram[A2O(0xfffb)])
#define TSMA1    (dsp56k_peripheral_ram[A2O(0xfffc)])
#define TSMB1    (dsp56k_peripheral_ram[A2O(0xfffd)])

/* Interrupt priority register (IPR) bits */
static void IPR_set(UINT16 value);

/* A return value of -1 means disabled */
static INT8 irqa_ipl(void);
static INT8 irqb_ipl(void);
static UINT8 irqa_trigger(void);
static UINT8 irqb_trigger(void);

static INT8 codec_ipl(void);
static INT8 host_ipl(void);
static INT8 ssi0_ipl(void);
static INT8 ssi1_ipl(void);
static INT8 tm_ipl(void);


/***************************************************************************
    HOST INTERFACE
***************************************************************************/
static void dsp56k_host_interface_reset(void);
#define HTX (HTXHRX)
#define HRX (HTXHRX)

#define ICR (core.HI.icr)
#define CVR (core.HI.cvr)
#define ISR (core.HI.isr)
#define IVR (core.HI.ivr)
#define TXH (core.HI.trxh)
#define TXL (core.HI.trxl)
#define RXH (core.HI.trxh)
#define RXL (core.HI.trxl)

/***************/
/* DSP56k SIDE */
/***************/
/* Host Control Register (HCR) Bits */
static void HCR_set(UINT16 value);

//static UINT16 HF3_bit(void);  #define hf3BIT  ((HCR & 0x0010) != 0)
//static UINT16 HF2_bit(void);  #define hf2BIT  ((HCR & 0x0008) != 0)
static UINT16 HCIE_bit(void);
static UINT16 HTIE_bit(void);
static UINT16 HRIE_bit(void);

static void HF3_bit_set(UINT16 value);
static void HF2_bit_set(UINT16 value);
static void HCIE_bit_set(UINT16 value);
static void HTIE_bit_set(UINT16 value);
static void HRIE_bit_set(UINT16 value);

/* Host Status Register (HSR) Bits */
//static void HSR_set(UINT16 value);

//static UINT16 DMA_bit(void);  #define dmaBIT  ((HSR & 0x0080) != 0)
//static UINT16 HF1_bit(void);  #define hf1BIT  ((HSR & 0x0010) != 0)
//static UINT16 HF0_bit(void);  #define hf0BIT  ((HSR & 0x0008) != 0)
//static UINT16 HCP_bit(void);  #define hcpBIT  ((HSR & 0x0004) != 0)
static UINT16 HTDE_bit(void);
static UINT16 HRDF_bit(void);

static void DMA_bit_set(UINT16 value);
static void HF1_bit_set(UINT16 value);
static void HF0_bit_set(UINT16 value);
static void HCP_bit_set(UINT16 value);
static void HTDE_bit_set(UINT16 value);
static void HRDF_bit_set(UINT16 value);

/*************/
/* HOST SIDE */
/*************/
/* Interrupt Control Register (ICR) Bits */
static void ICR_set(UINT8 value);

//static UINT8 INIT_bit(void); #define x_initBIT ((dsp56k.HI.ICR & 0x0080) != 0)
//static UINT8 HM1_bit(void);  #define x_hm1BIT  ((dsp56k.HI.ICR & 0x0040) != 0)
//static UINT8 HM0_bit(void);  #define x_hm0BIT  ((dsp56k.HI.ICR & 0x0020) != 0)
//static UINT8 HF1_bit_host(void);  #define x_hf1BIT  ((dsp56k.HI.ICR & 0x0010) != 0)
//static UINT8 HF0_bit_host(void);  #define x_hf0BIT  ((dsp56k.HI.ICR & 0x0008) != 0)
//static UINT8 TREQ_bit(void); #define x_treqBIT ((dsp56k.HI.ICR & 0x0002) != 0)
//static UINT8 RREQ_bit(void); #define x_rreqBIT ((dsp56k.HI.ICR & 0x0001) != 0)

//static void INIT_bit_set(UINT8 value); #define CLEAR_x_initBIT() (dsp56k.HI.ICR &= (~0x0080))
//static void HM1_bit_set(UINT8 value);  #define CLEAR_x_hm1BIT()  (dsp56k.HI.ICR &= (~0x0040))
//static void HM0_bit_set(UINT8 value);  #define CLEAR_x_hm0BIT()  (dsp56k.HI.ICR &= (~0x0020))
static void HF1_bit_host_set(UINT8 value);
static void HF0_bit_host_set(UINT8 value);
static void TREQ_bit_set(UINT8 value);
static void RREQ_bit_set(UINT8 value);

/* Command Vector Register (CVR) Bits */
static void CVR_set(UINT8 value);

//static UINT8 HC_bit();
static UINT8 HV_bits(void);

static void HC_bit_set(UINT8 value);
static void HV_bits_set(UINT8 value);

/* Interrupt Status Register (ISR) Bits */
// static void ISR_set(UINT8 value);

//static UINT8 HREQ_bit(void); #define x_hreqBIT ((dsp56k.HI.ISR & 0x0080) != 0)
//static UINT8 DMA_bit(void);  #define x_dmaBIT  ((dsp56k.HI.ISR & 0x0040) != 0)
//static UINT8 HF3_bit_host(void);  #define x_hf3BIT  ((dsp56k.HI.ISR & 0x0010) != 0)
//static UINT8 HF2_bit_host(void);  #define x_hf2BIT  ((dsp56k.HI.ISR & 0x0008) != 0)
//static UINT8 TRDY_bit(void); #define x_trdyBIT ((dsp56k.HI.ISR & 0x0004) != 0)
static UINT8 TXDE_bit(void);
static UINT8 RXDF_bit(void);

//static void HREQ_bit_set(UINT8 value); #define CLEAR_x_hreqBIT() (dsp56k.HI.ISR &= (~0x0080))
//static void DMA_bit_set(UINT8 value);  #define CLEAR_x_dmaBIT()  (dsp56k.HI.ISR &= (~0x0040))
static void HF3_bit_host_set(UINT8 value);
static void HF2_bit_host_set(UINT8 value);
//static void TRDY_bit_set(UINT8 value); #define CLEAR_x_trdyBIT() (dsp56k.HI.ISR &= (~0x0004))
static void TXDE_bit_set(UINT8 value);
static void RXDF_bit_set(UINT8 value);

/* Interrupt Vector Register (IVR) Bits */
//static void IVR_set(UINT8 value);

//static UINT8 IV7_bit(void);
//static UINT8 IV6_bit(void);
//static UINT8 IV5_bit(void);
//static UINT8 IV4_bit(void);
//static UINT8 IV3_bit(void);
//static UINT8 IV2_bit(void);
//static UINT8 IV1_bit(void);
//static UINT8 IV0_bit(void);

//static void IV7_bit_set(UINT8 value);
//static void IV6_bit_set(UINT8 value);
//static void IV5_bit_set(UINT8 value);
//static void IV4_bit_set(UINT8 value);
//static void IV3_bit_set(UINT8 value);
//static void IV2_bit_set(UINT8 value);
//static void IV1_bit_set(UINT8 value);
//static void IV0_bit_set(UINT8 value);


/* PROTOTYPES */
static void dsp56k_host_interface_HTX_to_host(void);
static void dsp56k_host_interface_host_to_HTX(void);


/***************************************************************************
    I/O INTERFACE
***************************************************************************/
static void dsp56k_io_reset(void);

/* Port A Bus Control Register (BCR) */
static void BCR_set(UINT16 value);

//static UINT16 RH_bit(void);
//static UINT16 BS_bit(void);
//static UINT16 external_x_wait_states(void);
//static UINT16 external_p_wait_states(void);

static void RH_bit_set(UINT16 value);
static void BS_bit_set(UINT16 value);
static void external_x_wait_states_set(UINT16 value);
static void external_p_wait_states_set(UINT16 value);

/* Port B Control Register (PBC) */
static void PBC_set(UINT16 value);
//static int host_interface_active(void);

/* Port B Data Direction Register (PBDDR) */
static void PBDDR_set(UINT16 value);

/* Port B Data Register (PBD) */
static void PBD_set(UINT16 value);

/* Port C Control Register (PCC) */
static void PCC_set(UINT16 value);

/* Port C Data Direction Register (PCDDR) */
static void PCDDR_set(UINT16 value);

/* Port C Dtaa Register (PCD) */
static void PCD_set(UINT16 value);

