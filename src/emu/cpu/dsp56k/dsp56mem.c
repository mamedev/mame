// This file contains functions which handle the On-Chip peripheral Memory Map
// as well as the Host Interface and the SSI0/SSI1 Serial Interfaces.


/* IPR Accessor Implementations */
static void IPR_set(dsp56k_core* cpustate, UINT16 value)
{
	/* TODO: Is there anything else? */
	IPR = value;
}

static INT8  irqa_ipl(dsp56k_core* cpustate)	{ return ((IPR & 0x0003) >> 0) - 1;  }
static UINT8 irqa_trigger(dsp56k_core* cpustate){ return  (IPR & 0x0004) >> 2;       }
static INT8  irqb_ipl(dsp56k_core* cpustate)	{ return ((IPR & 0x0018) >> 3) - 1;  }
static UINT8 irqb_trigger(dsp56k_core* cpustate){ return  (IPR & 0x0002) >> 5;       }
static INT8  codec_ipl(dsp56k_core* cpustate)	{ return ((IPR & 0x00c0) >> 6) - 1;  }
static INT8  host_ipl(dsp56k_core* cpustate)	{ return ((IPR & 0x0300) >> 8) - 1;  }
static INT8  ssi0_ipl(dsp56k_core* cpustate)	{ return ((IPR & 0x0c00) >> 10) - 1; }
static INT8  ssi1_ipl(dsp56k_core* cpustate)	{ return ((IPR & 0x3000) >> 12) - 1; }
static INT8  tm_ipl(dsp56k_core* cpustate)		{ return ((IPR & 0xc000) >> 14) - 1; }

static void mem_reset(dsp56k_core* cpustate)
{
	// Reset the HI registers
	dsp56k_host_interface_reset(cpustate);

	// Reset the IO registers
	dsp56k_io_reset(cpustate);
}

/* Work */
static READ16_HANDLER( peripheral_register_r )
{
	dsp56k_core* cpustate = (dsp56k_core*)space->cpu->token;
	// (printf) logerror("Peripheral read 0x%04x\n", O2A(offset));

	switch (O2A(offset))
	{
		// Port B Control Register (PBC)
		case 0xffc0: break;

		// Port C Control Register (PCC)
		case 0xffc1: break;

		// Port B Data Direction Register (PBDDR)
		case 0xffc2: break;

		// Port C Data Direction Register (PCDDR)
		case 0xffc3: break;

		// HCR: Host Control Register
		case 0xffc4: break;

		// COCR
		case 0xffc8: break;

		// reserved for test
		case 0xffc9: break;

		// CRA-SSI0 Control Register A
		case 0xffd0: break;

		// CRB-SSI0 Control Register B
		case 0xffd1: break;

		// CRA-SSI1 Control Register A
		case 0xffd8: break;

		// CRB-SSI1 Control Register B
		case 0xffd9: break;

		// PLCR
		case 0xffdc: break;

		// reserved for future use
		case 0xffdd: break;

		// BCR: Bus Control Register
		case 0xffde: break;

		// IPR: Interrupt Priority Register
		case 0xffdf: break;

		// Port B Data Register (PBD)
		case 0xffe2: break;

		// Port C Data Register (PCD)
		case 0xffe3: break;

		// HSR: Host Status Register
		case 0xffe4: break;

		// HTX/HRX: Host TX/RX Register
		case 0xffe5:
			// 5-5
			if (!HRDF_bit(cpustate))
				return 0xbeef;
			else
			{
				UINT16 value = HRX;		// TODO: Maybe not exactly right?  Just being safe.
				HRDF_bit_set(cpustate, 0);
				return value;
			}
			break;
		// COSR
		case 0xffe8: break;

		// CRX/CTX
		case 0xffe9: break;

		// Timer Control Register (TCR)
		case 0xffec: break;

		// Timer Count Register (TCTR)
		case 0xffed: break;

		// Timer Compare Register (TCPR)
		case 0xffee: break;

		// Timer Preload Register (TPR)
		case 0xffef: break;

		// SR/TSR SSI0 Status Register
		case 0xfff0: break;

		// TX/RX SSI0 Tx/RX Registers
		case 0xfff1: break;

		// RSMA0 SSI0 Register
		case 0xfff2: break;

		// RSMB0 SSI0 Register
		case 0xfff3: break;

		// TSMA0 SSI0 Register
		case 0xfff4: break;

		// TSMB0 SSI0 Register
		case 0xfff5: break;

		// SR/TSR SSI1 Status Register
		case 0xfff8: break;

		// TX/RX SSI1 TX/RX Registers
		case 0xfff9: break;

		// RSMA1 SSI1 Register
		case 0xfffa: break;

		// RSMB1 SSI1 Register
		case 0xfffb: break;

		// TSMA1 SSI1 Register
		case 0xfffc: break;

		// TSMB1 SSI1 Register
		case 0xfffd: break;

		// Reserved for on-chip emulation
		case 0xffff: break;
	}

	// Its primary behavior is RAM
	return dsp56k_peripheral_ram[offset];
}

static WRITE16_HANDLER( peripheral_register_w )
{
	dsp56k_core* cpustate = (dsp56k_core*)space->cpu->token;

	// Its primary behavior is RAM
	// COMBINE_DATA(&dsp56k_peripheral_ram[offset]);

	// (printf) logerror("Peripheral write 0x%04x = %04x\n", O2A(offset), data);

	// 4-8
	switch (O2A(offset))
	{
		// Port B Control Register (PBC)
		case 0xffc0:
			PBC_set(cpustate, data);
			break;

		// Port C Control Register (PCC)
		case 0xffc1:
			PCC_set(cpustate, data);
			break;

		// Port B Data Direction Register (PBDDR)
		case 0xffc2:
			PBDDR_set(cpustate, data);
			break;

		// Port C Data Direction Register (PCDDR)
		case 0xffc3:
			PCDDR_set(cpustate, data);
			break;

		// HCR: Host Control Register
		case 0xffc4:
			HCR_set(cpustate, data);
			break;

		// COCR
		case 0xffc8: break;

		// reserved for test
		case 0xffc9:
			logerror("DSP56k : Warning write to 0xffc9 reserved for test.\n");
			break;

		// CRA-SSI0 Control Register A
		case 0xffd0: break;

		// CRB-SSI0 Control Register B
		case 0xffd1: break;

		// CRA-SSI1 Control Register A
		case 0xffd8: break;

		// CRB-SSI1 Control Register B
		case 0xffd9: break;

		// PLCR
		case 0xffdc: break;

		// reserved for future use
		case 0xffdd:
			logerror("DSP56k : Warning write to 0xffdd reserved for future use.\n");
			break;

		// BCR: Bus Control Register
		case 0xffde:
			BCR_set(cpustate, data);
			break;

		// IPR: Interrupt Priority Register
		case 0xffdf:
			IPR_set(cpustate, data);
			break;

		// Port B Data Register (PBD)
		case 0xffe2:
			PBD_set(cpustate, data);
			break;

		// Port C Data Register (PCD)
		case 0xffe3:
			PCD_set(cpustate, data);
			break;

		// HSR: Host Status Register
		case 0xffe4: break;

		// HTX/HRX: Host TX/RX Register
		case 0xffe5:
			HTX = data;
			HTDE_bit_set(cpustate, 0);	// 5-5
			break;

		// COSR
		case 0xffe8: break;

		// CRX/CTX
		case 0xffe9: break;

		// Timer Control Register (TCR)
		case 0xffec: break;

		// Timer Count Register (TCTR)
		case 0xffed: break;

		// Timer Compare Register (TCPR)
		case 0xffee: break;

		// Timer Preload Register (TPR)
		case 0xffef: break;

		// SR/TSR SSI0 Status Register
		case 0xfff0: break;

		// TX/RX SSI0 Tx/RX Registers
		case 0xfff1: break;

		// RSMA0 SSI0 Register
		case 0xfff2: break;

		// RSMB0 SSI0 Register
		case 0xfff3: break;

		// TSMA0 SSI0 Register
		case 0xfff4: break;

		// TSMB0 SSI0 Register
		case 0xfff5: break;

		// SR/TSR SSI1 Status Register
		case 0xfff8: break;

		// TX/RX SSI1 TX/RX Registers
		case 0xfff9: break;

		// RSMA1 SSI1 Register
		case 0xfffa: break;

		// RSMB1 SSI1 Register
		case 0xfffb: break;

		// TSMA1 SSI1 Register
		case 0xfffc: break;

		// TSMB1 SSI1 Register
		case 0xfffd: break;

		// Reserved for on-chip emulation
		case 0xffff:
			logerror("DSP56k : Warning write to 0xffff reserved for on-chip emulation.\n");
			break;
	}
}

/***************************************************************************
    HOST INTERFACE
***************************************************************************/
/***************/
/* DSP56k SIDE */
/***************/
/************************************/
/* Host Control Register (HCR) Bits */
/************************************/
static void HCR_set(dsp56k_core* cpustate, UINT16 value)
{
	HF3_bit_set (cpustate, (value & 0x0010) >> 4);
	HF2_bit_set (cpustate, (value & 0x0008) >> 3);
	HCIE_bit_set(cpustate, (value & 0x0004) >> 2);
	HTIE_bit_set(cpustate, (value & 0x0002) >> 1);
	HRIE_bit_set(cpustate, (value & 0x0001) >> 0);
}
//static UINT16 HF3_bit(dsp56k_core* cpustate) { return ((HCR & 0x0010) != 0); }
//static UINT16 HF2_bit(dsp56k_core* cpustate) { return ((HCR & 0x0008) != 0); }
static UINT16 HCIE_bit(dsp56k_core* cpustate) { return ((HCR & 0x0004) != 0); }
static UINT16 HTIE_bit(dsp56k_core* cpustate) { return ((HCR & 0x0002) != 0); }
static UINT16 HRIE_bit(dsp56k_core* cpustate) { return ((HCR & 0x0001) != 0); }

static void HF3_bit_set(dsp56k_core* cpustate, UINT16 value)
{
	value = value & 0x01;
	HCR &= ~(0x0010);
	HCR |=  (value << 4);

	HF3_bit_host_set(cpustate, value);
}
static void HF2_bit_set(dsp56k_core* cpustate, UINT16 value)
{
	value = value & 0x01;
	HCR &= ~(0x0008);
	HCR |=  (value << 3);

	HF2_bit_host_set(cpustate, value);
}
static void HCIE_bit_set(dsp56k_core* cpustate, UINT16 value)
{
	value = value & 0x01;
	HCR &= ~(0x0004);
	HCR |=  (value << 2);
}
static void HTIE_bit_set(dsp56k_core* cpustate, UINT16 value)
{
	value = value & 0x01;
	HCR &= ~(0x0002);
	HCR |=  (value << 1);
}
static void HRIE_bit_set(dsp56k_core* cpustate, UINT16 value)
{
	value = value & 0x01;
	HCR &= ~(0x0001);
	HCR |=  (value << 0);
}

/***********************************/
/* Host Status Register (HSR) Bits */
/***********************************/
//static UINT16 DMA_bit(dsp56k_core* cpustate) { return ((HSR & 0x0080) != 0); }
//static UINT16 HF1_bit(dsp56k_core* cpustate) { return ((HSR & 0x0010) != 0); }
//static UINT16 HF0_bit(dsp56k_core* cpustate) { return ((HSR & 0x0008) != 0); }
//static UINT16 HCP_bit(dsp56k_core* cpustate) { return ((HSR & 0x0004) != 0); }
static UINT16 HTDE_bit(dsp56k_core* cpustate) { return ((HSR & 0x0002) != 0); }
static UINT16 HRDF_bit(dsp56k_core* cpustate) { return ((HSR & 0x0001) != 0); }

static void DMA_bit_set(dsp56k_core* cpustate, UINT16 value)
{
	value = value & 0x01;
	HSR &= ~(0x0080);
	HSR |= (value << 7);
	// TODO: 5-12 When the DMA bit is set, the DMA mode is enabled by the Host Mode bits HM0 & HM1
}
static void HF1_bit_set(dsp56k_core* cpustate, UINT16 value)
{
	value = value & 0x01;
	HSR &= ~(0x0010);
	HSR |= (value << 4);
}
static void HF0_bit_set(dsp56k_core* cpustate, UINT16 value)
{
	value = value & 0x01;
	HSR &= ~(0x0008);
	HSR |= (value << 3);
}
static void HCP_bit_set(dsp56k_core* cpustate, UINT16 value)
{
	value = value & 0x01;
	HSR &= ~(0x0004);
	HSR |=  (value << 2);

	if (value && HCIE_bit(cpustate))
		dsp56k_add_pending_interrupt(cpustate, "Host Command");
}
static void HTDE_bit_set(dsp56k_core* cpustate, UINT16 value)
{
	value = value & 0x01;
	HSR &= ~(0x0002);
	HSR |=  (value << 1);

	// 5-10 If HTIE bit is set, whip out a Host Transmit Data interrupt
	if (value && HTIE_bit(cpustate))
		dsp56k_add_pending_interrupt(cpustate, "Host Transmit Data");

	// 5-5 If both me and RXDF are cleared, transmit data to the host
	if (!value && !RXDF_bit(cpustate))
		dsp56k_host_interface_HTX_to_host(cpustate);
}
static void HRDF_bit_set(dsp56k_core* cpustate, UINT16 value)
{
	value = value & 0x01;
	HSR &= ~(0x0001);
	HSR |=  (value << 0);

	// 5-10 If HRIE is set, whip out a Host Receive Data interrupt
	if (value && HRIE_bit(cpustate))
		dsp56k_add_pending_interrupt(cpustate, "Host Receive Data");

	// 5-5 If both me and TXDE are cleared, transmit data to the dsp56k
	if (!value && !TXDE_bit(cpustate))
		dsp56k_host_interface_host_to_HTX(cpustate);
}



/*************/
/* HOST SIDE */
/*************/
/*****************************************/
/* Interrupt Control Register (ICR) Bits */
/*****************************************/
static void ICR_set(dsp56k_core* cpustate, UINT8 value)
{
	HF1_bit_host_set(cpustate, (value & 0x10) >> 4);
	HF0_bit_host_set(cpustate, (value & 0x08) >> 3);
	TREQ_bit_set(cpustate, (value & 0x02) >> 1);
	RREQ_bit_set(cpustate, (value & 0x01) >> 0);
}

//static UINT8 INIT_bit(dsp56k_core* cpustate); #define x_initBIT ((dsp56k.HI.ICR & 0x0080) != 0)
//static UINT8 HM1_bit(dsp56k_core* cpustate);  #define x_hm1BIT  ((dsp56k.HI.ICR & 0x0040) != 0)
//static UINT8 HM0_bit(dsp56k_core* cpustate);  #define x_hm0BIT  ((dsp56k.HI.ICR & 0x0020) != 0)
//static UINT8 HF1_bit_host(dsp56k_core* cpustate);  #define x_hf1BIT  ((dsp56k.HI.ICR & 0x0010) != 0)
//static UINT8 HF0_bit_host(dsp56k_core* cpustate);  #define x_hf0BIT  ((dsp56k.HI.ICR & 0x0008) != 0)
//static UINT8 TREQ_bit(dsp56k_core* cpustate); #define x_treqBIT ((dsp56k.HI.ICR & 0x0002) != 0)
//static UINT8 RREQ_bit(dsp56k_core* cpustate); #define x_rreqBIT ((dsp56k.HI.ICR & 0x0001) != 0)

//static void INIT_bit_set(dsp56k_core* cpustate, UINT8 value); #define CLEAR_x_initBIT() (dsp56k.HI.ICR &= (~0x0080))
//static void HM1_bit_set(dsp56k_core* cpustate, UINT8 value);  #define CLEAR_x_hm1BIT()  (dsp56k.HI.ICR &= (~0x0040))
//static void HM0_bit_set(dsp56k_core* cpustate, UINT8 value);  #define CLEAR_x_hm0BIT()  (dsp56k.HI.ICR &= (~0x0020))
static void HF1_bit_host_set(dsp56k_core* cpustate, UINT8 value)
{
	value = value & 0x01;
	ICR &= ~(0x10);
	ICR |=  (value << 4);

	HF1_bit_set(cpustate, value);		// 5-14
}
static void HF0_bit_host_set(dsp56k_core* cpustate, UINT8 value)
{
	value = value & 0x01;
	ICR &= ~(0x08);
	ICR |=  (value << 3);

	HF0_bit_set(cpustate, value);		// 5-13
}
static void TREQ_bit_set(dsp56k_core* cpustate, UINT8 value)
{
	value = value & 0x01;
	ICR &= ~(0x02);
	ICR |=  (value << 1);
}
static void RREQ_bit_set(dsp56k_core* cpustate, UINT8 value)
{
	value = value & 0x01;
	ICR &= ~(0x01);
	ICR |=  (value << 0);

	// 5-12
	if (value)
	{
		// TODO : HREQ_assert();
	}
}



/**************************************/
/* Command Vector Register (CVR) Bits */
/**************************************/
static UINT8 HV_bits(dsp56k_core* cpustate) { return (CVR & 0x1f); }

static void CVR_set(dsp56k_core* cpustate, UINT8 value)
{
	/* A single, unified place to run all callbacks for each of the bits */
	HC_bit_set(cpustate, (value & 0x80) >> 7);
	HV_bits_set(cpustate, (value & 0x1f));
}

static void HC_bit_set(dsp56k_core* cpustate, UINT8 value)
{
	value = value & 0x01;
	CVR &= ~(0x80);
	CVR |=  (value << 7);

	HCP_bit_set(cpustate, value);	// 5-9 & 5-11
}
static void HV_bits_set(dsp56k_core* cpustate, UINT8 value)
{
	value = value & 0x1f;
	CVR &= ~(0x1f);
	CVR |=  (value << 0);
}


/****************************************/
/* Interrupt Status Register (ISR) Bits */
/****************************************/
static UINT8 TXDE_bit(dsp56k_core* cpustate) { return ((ISR & 0x0002) != 0); }
static UINT8 RXDF_bit(dsp56k_core* cpustate) { return ((ISR & 0x0001) != 0); }

static void HF3_bit_host_set(dsp56k_core* cpustate, UINT8 value)
{
	value = value & 0x01;
	ISR &= ~(0x0010);
	ISR |=  (value << 4);
}
static void HF2_bit_host_set(dsp56k_core* cpustate, UINT8 value)
{
	value = value & 0x01;
	ISR &= ~(0x0008);
	ISR |=  (value << 3);
}

static void TXDE_bit_set(dsp56k_core* cpustate, UINT8 value)
{
	value = value & 0x01;
	ISR &= ~(0x0002);
	ISR |=  (value << 1);

	// If both me and the HRDF are cleared, transmit data to the dsp56k
	if (!value && !HRDF_bit(cpustate))
		dsp56k_host_interface_host_to_HTX(cpustate);
}

static void RXDF_bit_set(dsp56k_core* cpustate, UINT8 value)
{
	value = value & 0x01;
	ISR &= ~(0x0001);
	ISR |=  (value << 0);

	// If both me and HTDE are cleared, transmit data to the host
	if (!value && !HTDE_bit(cpustate))
		dsp56k_host_interface_HTX_to_host(cpustate);
}


// TODO: 5-11 What is the host processor Initialize function?

static void dsp56k_host_interface_reset(dsp56k_core* cpustate)
{
	// Hook up the CPU-side pointers properly.
	cpustate->HI.hcr = &dsp56k_peripheral_ram[A2O(0xffc4)];
	cpustate->HI.hsr = &dsp56k_peripheral_ram[A2O(0xffe4)];
	cpustate->HI.htrx = &dsp56k_peripheral_ram[A2O(0xffe5)];

	// The Bootstrap hack is initialized to write to address 0x0000
	cpustate->HI.bootstrap_offset = 0x0000;

	/* HCR */
	HCR_set(cpustate, 0x0000);	// 5-10

	/* HSR */
	HRDF_bit_set(cpustate, 0);	// 5-11
	HTDE_bit_set(cpustate, 1);	// 5-11
	HCP_bit_set(cpustate, 0);	// 5-11
	HF0_bit_set(cpustate, 0);	// 5-12
	HF1_bit_set(cpustate, 0);	// 5-12
	DMA_bit_set(cpustate, 0);	// 5-12

	/* CVR*/
	HV_bits_set(cpustate, 0x16);	// 5-7
	HC_bit_set(cpustate, 0);		// 5-9

	/* TODO: ISR (at least) */
}

/* These two functions are exposed to the outside world */
/* They represent the host side of the dsp56k's host interface */
void dsp56k_host_interface_write(running_device* device, UINT8 offset, UINT8 data)
{
	dsp56k_core* cpustate = get_safe_token(device);

	/* Not exactly correct since the bootstrap hack doesn't need this to be true */
	/*
    if (!host_interface_active())
        logerror("Dsp56k : Host interface write called without HI being set active by the PBC.\n");
    */

	switch (offset)
	{
		// Interrupt Control Register (ICR)
		case 0x00:
			// HACK
			if (cpustate->bootstrap_mode == BOOTSTRAP_HI)
			{
				// A-4 If they set HF0 while in bootstrap mode, it stops the bootstrap short.
				if (data & 0x08)
				{
					cpustate->bootstrap_mode = BOOTSTRAP_OFF;
					PC = 0x0000;
					// TODO: Do we set HF0 then, or let it slide?
					// TODO: Do I allow it to do an ICR_set(), or intercept it and throw everything away?
					break;
				}
			}
			ICR_set(cpustate, data);
			break;

		// Command Vector Register (CVR)
		case 0x01:
			CVR_set(cpustate, data);
			break;

		// Interrupt status register (ISR) - Read only!
		case 0x02:
			logerror("DSP56k : Interrupt status register is read only.\n");
			break;

		// Interrupt vector register (IVR)
		case 0x03: break;

		// Not used
		case 0x04:
			logerror("DSP56k : Address 0x4 on the host side of the host interface is not used.\n");
			break;

		// Reserved
		case 0x05: break;
			logerror("DSP56k : Address 0x5 on the host side of the host interface is reserved.\n");
			break;

		// Transmit byte register - high byte (TXH)
		case 0x06:
			// HACK
			if (cpustate->bootstrap_mode == BOOTSTRAP_HI)
			{
				dsp56k_program_ram[cpustate->HI.bootstrap_offset] &= 0x00ff;
				dsp56k_program_ram[cpustate->HI.bootstrap_offset] |= (data << 8);
				break;	/* Probably the right thing to do, given this is a hack */
			}

			if (TXDE_bit(cpustate))	// 5-5
			{
				TXH = data;
			}
			break;

		// Transmit byte register - low byte (TXL)
		case 0x07:
			// HACK
			if (cpustate->bootstrap_mode == BOOTSTRAP_HI)
			{
				dsp56k_program_ram[cpustate->HI.bootstrap_offset] &= 0xff00;
				dsp56k_program_ram[cpustate->HI.bootstrap_offset] |= data;
				cpustate->HI.bootstrap_offset++;

				if (cpustate->HI.bootstrap_offset == 0x800)
				{
					cpustate->bootstrap_mode = BOOTSTRAP_OFF;
				}
				break;	/* Probably the right thing to do, given this is a hack */
			}

			if (TXDE_bit(cpustate))	// 5-5
			{
				TXL = data;
				TXDE_bit_set(cpustate, 0);
			}
			break;

		default: logerror("DSP56k : dsp56k_host_interface_write called with invalid address 0x%02x.\n", offset);
	}
}

UINT8 dsp56k_host_interface_read(running_device* device, UINT8 offset)
{
	dsp56k_core* cpustate = get_safe_token(device);

	/* Not exactly correct since the bootstrap hack doesn't need this to be true */
	/*
    if (!host_interface_active())
        logerror("Dsp56k : Host interface write called without HI being set active by the PBC.\n");
    */

	switch (offset)
	{
		// Interrupt Control Register (ICR)
		case 0x00:
			return ICR;

		// Command Vector Register (CVR)
		case 0x01:
			return CVR;

		// Interrupt status register (ISR)
		case 0x02:
			return ISR;

		// Interrupt vector register (IVR)
		case 0x03:
			return IVR;

		// Read zeroes
		case 0x04:
			return 0x00;

		// Reserved
		case 0x05:
			logerror("DSP56k : Address 0x5 on the host side of the host interface is reserved.\n");
			break;

		// Receive byte register - high byte (RXH)
		case 0x06:
			// 5-5
			if (!RXDF_bit(cpustate))
				return 0xbf;
			else
				return RXH;
			break;

		// Receive byte register - low byte (RXL)
		case 0x07:
			// 5-5
			if (!RXDF_bit(cpustate))
				return 0xbf;
			else
			{
				UINT8 value = RXL;	// TODO: Maybe not exactly right?  I'm just being safe.
				RXDF_bit_set(cpustate, 0);
				return value;
			}
			break;

		default: logerror("DSP56k : dsp56k_host_interface_read called with invalid address 0x%02x.\n", offset);
	}

	/* Shouldn't get here */
	return 0xff;
}

static void dsp56k_host_interface_HTX_to_host(dsp56k_core* cpustate)
{
	RXH = ((HTX & 0xff00) >> 8);
	RXL = ((HTX & 0x00ff));
	RXDF_bit_set(cpustate, 1);
	HTDE_bit_set(cpustate, 1);
}

static void dsp56k_host_interface_host_to_HTX(dsp56k_core* cpustate)
{
	HRX &= 0x00ff;
	HRX |= (TXH << 8);
	HRX &= 0xff00;
	HRX |= TXL;
	TXDE_bit_set(cpustate, 1);
	HRDF_bit_set(cpustate, 1);
}


/***************************************************************************
    I/O INTERFACE
***************************************************************************/
/* BCR */
static void BCR_set(dsp56k_core* cpustate, UINT16 value)
{
	RH_bit_set(cpustate, (value & 0x8000) >> 15);
	BS_bit_set(cpustate, (value & 0x4000) >> 14);
	external_x_wait_states_set(cpustate, (value & 0x03e0) >> 5);
	external_p_wait_states_set(cpustate, (value & 0x001f) >> 0);
}

//static UINT16 RH_bit(dsp56k_core* cpustate);
//static UINT16 BS_bit(dsp56k_core* cpustate);
//static UINT16 external_x_wait_states(dsp56k_core* cpustate);
//static UINT16 external_p_wait_states(dsp56k_core* cpustate);

static void RH_bit_set(dsp56k_core* cpustate, UINT16 value)
{
	value = value & 0x0001;
	BCR &= ~(0x8000);
	BCR |= (value << 15);

	// TODO: 4-6 Assert BR pin?
}
static void BS_bit_set(dsp56k_core* cpustate, UINT16 value)
{
	value = value & 0x0001;
	BCR &= ~(0x4000);
	BCR |= (value << 14);

	// TODO: 4-6 Respond to BR pin?
}
static void external_x_wait_states_set(dsp56k_core* cpustate, UINT16 value)
{
	value = value & 0x001f;
	BCR &= ~(0x03e0);
	BCR |= (value << 5);
}
static void external_p_wait_states_set(dsp56k_core* cpustate, UINT16 value)
{
	value = value & 0x001f;
	BCR &= ~(0x001f);
	BCR |= (value << 0);
}


/* Port B Control Register PBC */
static void PBC_set(dsp56k_core* cpustate, UINT16 value)
{
	if (value & 0xfffe)
		logerror("Dsp56k : Attempting to set reserved bits in the PBC.  Ignoring.\n");

	value = value & 0x0001;
	PBC &= ~(0x0001);
	PBC |= (value << 0);
}

#ifdef UNUSED_FUNCTION
static int host_interface_active(dsp56k_core* cpustate)
{
	/* The host interface is active if the 0th bit in the PBC is set */
	return PBC & 0x0001;
}
#endif

/* Port B Data Direction Register (PBDDR) */
static void PBDDR_set(dsp56k_core* cpustate, UINT16 value)
{
	if (value & 0x8000)
		logerror("Dsp56k : Attempting to set reserved bits in the PBDDR.  Ignoring.\n");

	value = value & 0x7fff;
	PBDDR &= ~(0x7fff);
	PBDDR |= (value << 0);

	/* TODO: Implement dsp56k io restrictions, etc. */
}

/* Port B Data Register (PBD) */
static void PBD_set(dsp56k_core* cpustate, UINT16 value)
{
	if (value & 0x8000)
		logerror("Dsp56k : Attempting to set reserved bits in the PBD.  Ignoring.\n");

	value = value & 0x7fff;
	PBD &= ~(0x7fff);
	PBD |= (value << 0);

	/* TODO: Implement dsp56k io restrictions, etc. */
}

/* Port C Control Register (PCC) */
static void PCC_set(dsp56k_core* cpustate, UINT16 value)
{
	if (value & 0xf000)
		logerror("Dsp56k : Attempting to set reserved bits in the PCC.  Ignoring.\n");

	value = value & 0x0fff;
	PCC &= ~(0x0fff);
	PCC |= (value << 0);

	/* TODO: Implement dsp56k timer and control glue */
}

/* Port C Data Direction Register (PCDDR) */
static void PCDDR_set(dsp56k_core* cpustate, UINT16 value)
{
	if (value & 0xf000)
		logerror("Dsp56k : Attempting to set reserved bits in the PCDDR.  Ignoring.\n");

	value = value & 0x0fff;
	PCDDR &= ~(0x0fff);
	PCDDR |= (value << 0);

	/* TODO: Implement dsp56k io restrictions, etc. */
}

/* Port C Data Register (PCD) */
static void PCD_set(dsp56k_core* cpustate, UINT16 value)
{
	if (value & 0xf000)
		logerror("Dsp56k : Attempting to set reserved bits in the PCD.  Ignoring.\n");

	/* TODO: Temporary */
	logerror("Dsp56k : Setting general output port C data to 0x%04x\n", value);

	value = value & 0x0fff;
	PCD &= ~(0x0fff);
	PCD |= (value << 0);
}

static void dsp56k_io_reset(dsp56k_core* cpustate)
{
	/* The BCR = 0x43ff */
	RH_bit_set(cpustate, 0);
	BS_bit_set(cpustate, 1);
	external_x_wait_states_set(cpustate, 0x1f);
	external_p_wait_states_set(cpustate, 0x1f);
}


/* MISC*/
UINT16 dsp56k_get_peripheral_memory(running_device* device, UINT16 addr)
{
	// TODO // THIS COMES BACK dsp56k_core* cpustate = get_safe_token(device);
	return dsp56k_peripheral_ram[A2O(addr)];
}
