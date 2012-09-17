/***************************************************************************

  snessdd1.c

  File to handle emulation of the SNES "S-DD1" add-on chip.

  Based on Andreas Naive Public Domain code.

***************************************************************************/

static UINT8 sdd1_read(running_machine& machine, UINT32 addr);

class SDD1_IM //Input Manager
{
public:
	SDD1_IM(running_machine &machine)
		: m_machine(machine) { }

	running_machine &machine() const { return m_machine; }

	UINT32 byte_ptr;
	UINT8 bit_count;

private:
	running_machine& m_machine;
};

static void SDD1_IM_prepareDecomp(SDD1_IM* thisptr, UINT32 in_buf)
{
	thisptr->byte_ptr = in_buf;
	thisptr->bit_count = 4;
}

static UINT8 SDD1_IM_getCodeword(SDD1_IM* thisptr, const UINT8 code_len)
{
	UINT8 codeword;

	codeword = sdd1_read(thisptr->machine(), thisptr->byte_ptr) << thisptr->bit_count;

	++thisptr->bit_count;

	if (codeword & 0x80)
	{
		codeword |= sdd1_read(thisptr->machine(), thisptr->byte_ptr + 1) >> (9 - thisptr->bit_count);
		thisptr->bit_count += code_len;
	}

	if (thisptr->bit_count & 0x08)
	{
		thisptr->byte_ptr++;
		thisptr->bit_count &= 0x07;
	}

	return codeword;
}

class SDD1_GCD //Golomb-Code Decoder
{
public:
	SDD1_GCD(running_machine &machine, SDD1_IM* associatedIM)
		: IM(associatedIM),
		   m_machine(machine) { }

	running_machine &machine() const { return m_machine; }

	SDD1_IM* IM;

private:
	running_machine& m_machine;
};

static void SDD1_GCD_getRunCount(SDD1_GCD* thisptr, UINT8 code_num, UINT8* MPScount, UINT8* LPSind)
{
	const UINT8 run_count[] =
	{
		0x00, 0x00, 0x01, 0x00, 0x03, 0x01, 0x02, 0x00,
		0x07, 0x03, 0x05, 0x01, 0x06, 0x02, 0x04, 0x00,
		0x0f, 0x07, 0x0b, 0x03, 0x0d, 0x05, 0x09, 0x01,
		0x0e, 0x06, 0x0a, 0x02, 0x0c, 0x04, 0x08, 0x00,
		0x1f, 0x0f, 0x17, 0x07, 0x1b, 0x0b, 0x13, 0x03,
		0x1d, 0x0d, 0x15, 0x05, 0x19, 0x09, 0x11, 0x01,
		0x1e, 0x0e, 0x16, 0x06, 0x1a, 0x0a, 0x12, 0x02,
		0x1c, 0x0c, 0x14, 0x04, 0x18, 0x08, 0x10, 0x00,
		0x3f, 0x1f, 0x2f, 0x0f, 0x37, 0x17, 0x27, 0x07,
		0x3b, 0x1b, 0x2b, 0x0b, 0x33, 0x13, 0x23, 0x03,
		0x3d, 0x1d, 0x2d, 0x0d, 0x35, 0x15, 0x25, 0x05,
		0x39, 0x19, 0x29, 0x09, 0x31, 0x11, 0x21, 0x01,
		0x3e, 0x1e, 0x2e, 0x0e, 0x36, 0x16, 0x26, 0x06,
		0x3a, 0x1a, 0x2a, 0x0a, 0x32, 0x12, 0x22, 0x02,
		0x3c, 0x1c, 0x2c, 0x0c, 0x34, 0x14, 0x24, 0x04,
		0x38, 0x18, 0x28, 0x08, 0x30, 0x10, 0x20, 0x00,
		0x7f, 0x3f, 0x5f, 0x1f, 0x6f, 0x2f, 0x4f, 0x0f,
		0x77, 0x37, 0x57, 0x17, 0x67, 0x27, 0x47, 0x07,
		0x7b, 0x3b, 0x5b, 0x1b, 0x6b, 0x2b, 0x4b, 0x0b,
		0x73, 0x33, 0x53, 0x13, 0x63, 0x23, 0x43, 0x03,
		0x7d, 0x3d, 0x5d, 0x1d, 0x6d, 0x2d, 0x4d, 0x0d,
		0x75, 0x35, 0x55, 0x15, 0x65, 0x25, 0x45, 0x05,
		0x79, 0x39, 0x59, 0x19, 0x69, 0x29, 0x49, 0x09,
		0x71, 0x31, 0x51, 0x11, 0x61, 0x21, 0x41, 0x01,
		0x7e, 0x3e, 0x5e, 0x1e, 0x6e, 0x2e, 0x4e, 0x0e,
		0x76, 0x36, 0x56, 0x16, 0x66, 0x26, 0x46, 0x06,
		0x7a, 0x3a, 0x5a, 0x1a, 0x6a, 0x2a, 0x4a, 0x0a,
		0x72, 0x32, 0x52, 0x12, 0x62, 0x22, 0x42, 0x02,
		0x7c, 0x3c, 0x5c, 0x1c, 0x6c, 0x2c, 0x4c, 0x0c,
		0x74, 0x34, 0x54, 0x14, 0x64, 0x24, 0x44, 0x04,
		0x78, 0x38, 0x58, 0x18, 0x68, 0x28, 0x48, 0x08,
		0x70, 0x30, 0x50, 0x10, 0x60, 0x20, 0x40, 0x00,
	};

	UINT8 codeword = SDD1_IM_getCodeword(thisptr->IM, code_num);

	if (codeword & 0x80)
	{
		*LPSind = 1;
		*MPScount = run_count[codeword >> (code_num ^ 0x07)];
	}
	else
	{
		*MPScount = (1 << code_num);
	}
}

class SDD1_BG // Bits Generator
{
public:
	SDD1_BG(running_machine &machine, SDD1_GCD* associatedGCD, UINT8 code)
		: code_num(code),
		  GCD(associatedGCD),
		  m_machine(machine) { }

	running_machine &machine() const { return m_machine; }

	UINT8 code_num;
	UINT8 MPScount;
	UINT8 LPSind;
	SDD1_GCD* GCD;

public:
	running_machine& m_machine;
} ;

static void SDD1_BG_prepareDecomp(SDD1_BG* thisptr)
{
	thisptr->MPScount = 0;
	thisptr->LPSind = 0;
}

static UINT8 SDD1_BG_getBit(SDD1_BG* thisptr, UINT8* endOfRun)
{
	UINT8 bit;

	if (!(thisptr->MPScount || thisptr->LPSind))
	{
		SDD1_GCD_getRunCount(thisptr->GCD, thisptr->code_num, &(thisptr->MPScount), &(thisptr->LPSind));
	}

	if (thisptr->MPScount)
	{
		bit = 0;
		thisptr->MPScount--;
	}
	else
	{
		bit = 1;
		thisptr->LPSind = 0;
	}

	if (thisptr->MPScount || thisptr->LPSind)
	{
		(*endOfRun) = 0;
	}
	else
	{
		(*endOfRun) = 1;
	}

	return bit;
}


struct SDD1_PEM_state
{
	UINT8 code_num;
	UINT8 nextIfMPS;
	UINT8 nextIfLPS;
};

static const SDD1_PEM_state SDD1_PEM_evolution_table[33] =
{
	{ 0,25,25},
	{ 0, 2, 1},
	{ 0, 3, 1},
	{ 0, 4, 2},
	{ 0, 5, 3},
	{ 1, 6, 4},
	{ 1, 7, 5},
	{ 1, 8, 6},
	{ 1, 9, 7},
	{ 2,10, 8},
	{ 2,11, 9},
	{ 2,12,10},
	{ 2,13,11},
	{ 3,14,12},
	{ 3,15,13},
	{ 3,16,14},
	{ 3,17,15},
	{ 4,18,16},
	{ 4,19,17},
	{ 5,20,18},
	{ 5,21,19},
	{ 6,22,20},
	{ 6,23,21},
	{ 7,24,22},
	{ 7,24,23},
	{ 0,26, 1},
	{ 1,27, 2},
	{ 2,28, 4},
	{ 3,29, 8},
	{ 4,30,12},
	{ 5,31,16},
	{ 6,32,18},
	{ 7,24,22}
};

struct SDD1_PEM_ContextInfo
{
	UINT8 status;
	UINT8 MPS;
};

class SDD1_PEM //Probability Estimation Module
{
public:
	SDD1_PEM(running_machine& machine,
							   SDD1_BG* associatedBG0, SDD1_BG* associatedBG1,
							   SDD1_BG* associatedBG2, SDD1_BG* associatedBG3,
							   SDD1_BG* associatedBG4, SDD1_BG* associatedBG5,
							   SDD1_BG* associatedBG6, SDD1_BG* associatedBG7)
		: m_machine(machine)
	{
		BG[0] = associatedBG0;
		BG[1] = associatedBG1;
		BG[2] = associatedBG2;
		BG[3] = associatedBG3;
		BG[4] = associatedBG4;
		BG[5] = associatedBG5;
		BG[6] = associatedBG6;
		BG[7] = associatedBG7;
	}

	running_machine &machine() const { return m_machine; }

	SDD1_PEM_ContextInfo contextInfo[32];
	SDD1_BG* BG[8];

private:
	running_machine& m_machine;
} ;

static void SDD1_PEM_prepareDecomp(SDD1_PEM* thisptr)
{
	UINT8 i;
	for(i = 0; i < 32; i++)
	{
		thisptr->contextInfo[i].status = 0;
		thisptr->contextInfo[i].MPS = 0;
	}
}

static UINT8 SDD1_PEM_getBit(SDD1_PEM* thisptr, UINT8 context)
{
	UINT8 endOfRun;
	UINT8 bit;

	SDD1_PEM_ContextInfo *pContInfo = &(thisptr->contextInfo)[context];
	UINT8 currStatus = pContInfo->status;
	const SDD1_PEM_state* pState = &(SDD1_PEM_evolution_table[currStatus]);
	UINT8 currentMPS = pContInfo->MPS;

	bit = SDD1_BG_getBit(thisptr->BG[pState->code_num], &endOfRun);

	if (endOfRun)
	{
		if (bit)
		{
			if (!(currStatus & 0xfe))
			{
				(pContInfo->MPS) ^= 0x01;
			}
			pContInfo->status = pState->nextIfLPS;
		}
		else
		{
			pContInfo->status = pState->nextIfMPS;
		}
	}

	return bit ^ currentMPS;
}

class SDD1_CM
{
public:
	SDD1_CM(running_machine& machine, SDD1_PEM* associatedPEM)
		: PEM(associatedPEM),
		  m_machine(machine) { }

	running_machine &machine() const { return m_machine; }

	UINT8 bitplanesInfo;
	UINT8 contextBitsInfo;
	UINT8 bit_number;
	UINT8 currBitplane;
	UINT16 prevBitplaneBits[8];
	SDD1_PEM* PEM;

private:
	running_machine& m_machine;
} ;

static void SDD1_CM_prepareDecomp(SDD1_CM* thisptr, UINT32 first_byte)
{
	INT32 i = 0;
	thisptr->bitplanesInfo = sdd1_read(thisptr->machine(), first_byte) & 0xc0;
	thisptr->contextBitsInfo = sdd1_read(thisptr->machine(), first_byte) & 0x30;
	thisptr->bit_number = 0;
	for (i = 0; i < 8; i++)
	{
		thisptr->prevBitplaneBits[i] = 0;
	}
	switch(thisptr->bitplanesInfo)
	{
		case 0x00:
			thisptr->currBitplane = 1;
			break;
		case 0x40:
			thisptr->currBitplane = 7;
			break;
		case 0x80:
			thisptr->currBitplane = 3;
			break;
	}
}

static UINT8 SDD1_CM_getBit(SDD1_CM* thisptr)
{
	UINT8 currContext;
	UINT16 *context_bits;
	UINT8 bit = 0;

	switch (thisptr->bitplanesInfo)
	{
		case 0x00:
			thisptr->currBitplane ^= 0x01;
			break;
		case 0x40:
			thisptr->currBitplane ^= 0x01;
			if (!(thisptr->bit_number & 0x7f))
			{
				thisptr->currBitplane = ((thisptr->currBitplane + 2) & 0x07);
			}
			break;
		case 0x80:
			thisptr->currBitplane ^= 0x01;
			if (!(thisptr->bit_number & 0x7f))
			{
				thisptr->currBitplane ^= 0x02;
			}
			break;
		case 0xc0:
			thisptr->currBitplane = thisptr->bit_number & 0x07;
			break;
	}

	context_bits = &(thisptr->prevBitplaneBits)[thisptr->currBitplane];

	currContext = (thisptr->currBitplane & 0x01) << 4;
	switch (thisptr->contextBitsInfo)
	{
		case 0x00:
			currContext |= ((*context_bits & 0x01c0) >> 5) | (*context_bits & 0x0001);
			break;
		case 0x10:
		    currContext |= ((*context_bits & 0x0180) >> 5) | (*context_bits & 0x0001);
			break;
		case 0x20:
			currContext |= ((*context_bits & 0x00c0) >> 5) | (*context_bits & 0x0001);
			break;
		case 0x30:
			currContext |= ((*context_bits & 0x0180) >> 5) | (*context_bits & 0x0003);
			break;
	}

	bit = SDD1_PEM_getBit(thisptr->PEM, currContext);

	*context_bits <<= 1;
	*context_bits |= bit;

	thisptr->bit_number++;

	return bit;
}

class SDD1_OL
{
public:
	SDD1_OL(running_machine& machine, SDD1_CM* associatedCM)
		: CM(associatedCM),
		  m_machine(machine) { }

	running_machine &machine() const { return m_machine; }

	UINT8 bitplanesInfo;
	UINT16 length;
	UINT8* buffer;
	SDD1_CM* CM;

private:
	running_machine& m_machine;
} ;

static void SDD1_OL_prepareDecomp(SDD1_OL* thisptr, UINT32 first_byte, UINT16 out_len, UINT8 *out_buf)
{
	thisptr->bitplanesInfo = sdd1_read(thisptr->machine(), first_byte) & 0xc0;
	thisptr->length = out_len;
	thisptr->buffer = out_buf;
}

static void SDD1_OL_launch(SDD1_OL* thisptr)
{
	UINT8 i;
	UINT8 register1 = 0, register2 = 0;

	switch(thisptr->bitplanesInfo)
	{
		case 0x00:
		case 0x40:
		case 0x80:
			i = 1;
			do
			{	// if length == 0, we output 2^16 bytes
				if(!i)
				{
					*(thisptr->buffer++) = register2;
					i = ~i;
				}
				else
				{
					for(register1 = register2 = 0, i = 0x80; i; i >>= 1)
					{
						if(SDD1_CM_getBit(thisptr->CM))
						{
							register1 |= i;
						}
						if(SDD1_CM_getBit(thisptr->CM))
						{
							register2 |= i;
						}
					}
					*(thisptr->buffer++) = register1;
				}
			} while(--(thisptr->length));
			break;
		case 0xc0:
			do
			{
				for(register1 = 0, i = 0x01; i; i <<= 1)
				{
					if(SDD1_CM_getBit(thisptr->CM))
					{
						register1 |= i;
					}
				}
				*(thisptr->buffer++) = register1;
			} while(--(thisptr->length));
			break;
	}
}

class SDD1emu
{
public:
	SDD1emu(running_machine &machine);

	running_machine &machine() const { return m_machine; }

	SDD1_IM* IM;
	SDD1_GCD* GCD;
	SDD1_BG* BG0;	SDD1_BG* BG1;	SDD1_BG* BG2;	SDD1_BG* BG3;
	SDD1_BG* BG4;	SDD1_BG* BG5;	SDD1_BG* BG6;	SDD1_BG* BG7;
	SDD1_PEM* PEM;
	SDD1_CM* CM;
	SDD1_OL* OL;

private:
	running_machine& m_machine;
};

SDD1emu::SDD1emu(running_machine &machine)
	: m_machine(machine)
{
	IM = auto_alloc(machine, SDD1_IM(machine));
	GCD = auto_alloc(machine, SDD1_GCD(machine, IM));
	BG0 = auto_alloc(machine, SDD1_BG(machine, GCD, 0));
	BG1 = auto_alloc(machine, SDD1_BG(machine, GCD, 1));
	BG2 = auto_alloc(machine, SDD1_BG(machine, GCD, 2));
	BG3 = auto_alloc(machine, SDD1_BG(machine, GCD, 3));
	BG4 = auto_alloc(machine, SDD1_BG(machine, GCD, 4));
	BG5 = auto_alloc(machine, SDD1_BG(machine, GCD, 5));
	BG6 = auto_alloc(machine, SDD1_BG(machine, GCD, 6));
	BG7 = auto_alloc(machine, SDD1_BG(machine, GCD, 7));
	PEM = auto_alloc(machine, SDD1_PEM(machine, BG0, BG1, BG2, BG3,
										   BG4, BG5, BG6, BG7));
	CM = auto_alloc(machine, SDD1_CM(machine, PEM));
	OL = auto_alloc(machine, SDD1_OL(machine, CM));
}

static void SDD1emu_decompress(SDD1emu* thisptr, UINT32 in_buf, UINT16 out_len, UINT8 *out_buf)
{
	SDD1_IM_prepareDecomp(thisptr->IM, in_buf);
	SDD1_BG_prepareDecomp(thisptr->BG0);
	SDD1_BG_prepareDecomp(thisptr->BG1);
	SDD1_BG_prepareDecomp(thisptr->BG2);
	SDD1_BG_prepareDecomp(thisptr->BG3);
	SDD1_BG_prepareDecomp(thisptr->BG4);
	SDD1_BG_prepareDecomp(thisptr->BG5);
	SDD1_BG_prepareDecomp(thisptr->BG6);
	SDD1_BG_prepareDecomp(thisptr->BG7);
	SDD1_PEM_prepareDecomp(thisptr->PEM);
	SDD1_CM_prepareDecomp(thisptr->CM, in_buf);
	SDD1_OL_prepareDecomp(thisptr->OL, in_buf, out_len, out_buf);

	SDD1_OL_launch(thisptr->OL);
}

struct snes_sdd1_t
{
	UINT8 sdd1_enable;	// channel bit-mask
	UINT8 xfer_enable;	// channel bit-mask
	UINT32 mmc[4];		// memory map controller ROM indices

	struct
	{
		UINT32 addr;	// $43x2-$43x4 -- DMA transfer address
		UINT16 size;	// $43x5-$43x6 -- DMA transfer size
	} dma[8];

	SDD1emu* sdd1emu;
	struct
	{
		UINT8 *data;	// pointer to decompressed S-DD1 data (65536 bytes)
		UINT16 offset;	// read index into S-DD1 decompression buffer
		UINT32 size;	// length of data buffer; reads decrement counter, set ready to false at 0
		UINT8 ready;	// 1 when data[] is valid; 0 to invoke sdd1emu.decompress()
	} buffer;
} ;

static snes_sdd1_t snes_sdd1;

static void sdd1_init(running_machine& machine)
{
	UINT8 i;

	snes_sdd1.sdd1_enable = 0x00;
	snes_sdd1.xfer_enable = 0x00;

	snes_sdd1.mmc[0] = 0 << 20;
	snes_sdd1.mmc[1] = 1 << 20;
	snes_sdd1.mmc[2] = 2 << 20;
	snes_sdd1.mmc[3] = 3 << 20;

	for(i = 0; i < 8; i++)
	{
		snes_sdd1.dma[i].addr = 0;
		snes_sdd1.dma[i].size = 0;
	}

	snes_sdd1.sdd1emu = auto_alloc(machine, SDD1emu(machine));

	snes_sdd1.buffer.data = (UINT8*)auto_alloc_array(machine, UINT8, 0x10000);
	snes_sdd1.buffer.ready = 0;
}

static UINT8 sdd1_mmio_read(address_space &space, UINT32 addr)
{
	addr &= 0xffff;

	if((addr & 0x4380) == 0x4300)
	{
		return snes_r_io(space, addr & 0x7f);
	}

	switch(addr)
	{
		case 0x4804:
			return (snes_sdd1.mmc[0] >> 20) & 7;
		case 0x4805:
			return (snes_sdd1.mmc[1] >> 20) & 7;
		case 0x4806:
			return (snes_sdd1.mmc[2] >> 20) & 7;
		case 0x4807:
			return (snes_sdd1.mmc[3] >> 20) & 7;
	}

	return snes_open_bus_r(space, 0);
}

static void sdd1_mmio_write(address_space &space, UINT32 addr, UINT8 data)
{
	addr &= 0xffff;

	if((addr & 0x4380) == 0x4300)
	{
		UINT8 channel = (addr >> 4) & 7;
		switch(addr & 15)
		{
			case 2:
				snes_sdd1.dma[channel].addr = (snes_sdd1.dma[channel].addr & 0xffff00) + (data <<  0);
				break;
			case 3:
				snes_sdd1.dma[channel].addr = (snes_sdd1.dma[channel].addr & 0xff00ff) + (data <<  8);
				break;
			case 4:
				snes_sdd1.dma[channel].addr = (snes_sdd1.dma[channel].addr & 0x00ffff) + (data << 16);
				break;

			case 5:
				snes_sdd1.dma[channel].size = (snes_sdd1.dma[channel].size &   0xff00) + (data <<  0);
				break;
			case 6:
				snes_sdd1.dma[channel].size = (snes_sdd1.dma[channel].size &   0x00ff) + (data <<  8);
				break;
		}
		snes_ram[addr] = data;
		snes_io_dma_w(space, addr, data);
		return;
	}

	switch(addr)
	{
		case 0x4800:
			snes_sdd1.sdd1_enable = data;
			break;
		case 0x4801:
			snes_sdd1.xfer_enable = data;
			break;

		case 0x4804:
			snes_sdd1.mmc[0] = (data & 7) << 20;
			break;
		case 0x4805:
			snes_sdd1.mmc[1] = (data & 7) << 20;
			break;
		case 0x4806:
			snes_sdd1.mmc[2] = (data & 7) << 20;
			break;
		case 0x4807:
			snes_sdd1.mmc[3] = (data & 7) << 20;
			break;
	}
}

static UINT8 sdd1_read(running_machine& machine, UINT32 addr)
{
	unsigned char *ROM = machine.root_device().memregion("cart")->base();

	if(snes_sdd1.sdd1_enable & snes_sdd1.xfer_enable)
	{
		// at least one channel has S-DD1 decompression enabled...
		UINT32 i;
		for(i = 0; i < 8; i++)
		{
			if(snes_sdd1.sdd1_enable & snes_sdd1.xfer_enable & (1 << i))
			{
				// S-DD1 always uses fixed transfer mode, so address will not change during transfer
				if((addr + 0xc00000) == snes_sdd1.dma[i].addr)
				{
					UINT8 data;
					if(!snes_sdd1.buffer.ready)
					{
						UINT8 temp;
						// first byte read for channel performs full decompression.
						// this really should stream byte-by-byte, but it's not necessary since the size is known
						snes_sdd1.buffer.offset = 0;
						snes_sdd1.buffer.size = snes_sdd1.dma[i].size ? snes_sdd1.dma[i].size : 65536;

						// sdd1emu calls this function; it needs to access uncompressed data;
						// so temporarily disable decompression mode for decompress() call.
						temp = snes_sdd1.sdd1_enable;
						snes_sdd1.sdd1_enable = 0;
						SDD1emu_decompress(snes_sdd1.sdd1emu, addr, snes_sdd1.buffer.size, snes_sdd1.buffer.data);
						snes_sdd1.sdd1_enable = temp;

						snes_sdd1.buffer.ready = 1;
					}

					// fetch a decompressed byte; once buffer is depleted, disable channel and invalidate buffer
					data = snes_sdd1.buffer.data[(UINT16)snes_sdd1.buffer.offset++];
					if(snes_sdd1.buffer.offset >= snes_sdd1.buffer.size)
					{
						snes_sdd1.buffer.ready = 0;
						snes_sdd1.xfer_enable &= ~(1 << i);
					}

					return data;
				} // address matched
			} // channel enabled
		} // channel loop
	} // S-DD1 decompressor enabled

	return ROM[snes_sdd1.mmc[(addr >> 20) & 3] + (addr & 0x0fffff)];
}

static void sdd1_reset(running_machine &machine)
{
}
