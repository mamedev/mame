/***************************************************************************

  snessdd1.c

  File to handle emulation of the SNES "S-DD1" add-on chip.

  Based on Andreas Naive Public Domain code.

***************************************************************************/


#define SSD1_ADD(addr)\
mmc[(addr >> 20) & 3] + (addr & 0x0fffff)

class SDD1_IM //Input Manager
{
public:
	SDD1_IM() {}

	UINT32 m_byte_ptr;
	UINT8 m_bit_count;

	void IM_prepareDecomp(UINT32 in_buf);
	UINT8 IM_getCodeword(UINT8 *ROM, UINT32 *mmc, const UINT8 code_len);
};

void SDD1_IM::IM_prepareDecomp(UINT32 in_buf)
{
	m_byte_ptr = in_buf;
	m_bit_count = 4;
}

UINT8 SDD1_IM::IM_getCodeword(UINT8 *ROM, UINT32 *mmc, const UINT8 code_len)
{
	UINT8 codeword = ROM[SSD1_ADD(m_byte_ptr)] << m_bit_count;

	++m_bit_count;

	if (codeword & 0x80)
	{
		codeword |= ROM[SSD1_ADD((m_byte_ptr + 1))] >> (9 - m_bit_count);
		m_bit_count += code_len;
	}

	if (m_bit_count & 0x08)
	{
		m_byte_ptr++;
		m_bit_count &= 0x07;
	}

	return codeword;
}

class SDD1_GCD //Golomb-Code Decoder
{
public:
	SDD1_GCD(SDD1_IM* associatedIM)
		: m_IM(associatedIM) { }

	SDD1_IM* m_IM;

	void GCD_getRunCount(UINT8 *ROM, UINT32 *mmc, UINT8 code_num, UINT8* MPScount, UINT8* LPSind);
};

void SDD1_GCD::GCD_getRunCount(UINT8 *ROM, UINT32 *mmc, UINT8 code_num, UINT8* MPScount, UINT8* LPSind)
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

	UINT8 codeword = m_IM->IM_getCodeword(ROM, mmc, code_num);

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
	SDD1_BG(SDD1_GCD* associatedGCD, UINT8 code)
		: m_code_num(code),
			m_GCD(associatedGCD) { }

	UINT8 m_code_num;
	UINT8 m_MPScount;
	UINT8 m_LPSind;
	SDD1_GCD* m_GCD;

	void BG_prepareDecomp();
	UINT8 BG_getBit(UINT8 *ROM, UINT32 *mmc, UINT8* endOfRun);
} ;

void SDD1_BG::BG_prepareDecomp()
{
	m_MPScount = 0;
	m_LPSind = 0;
}

UINT8 SDD1_BG::BG_getBit(UINT8 *ROM, UINT32 *mmc, UINT8* endOfRun)
{
	UINT8 bit;

	if (!(m_MPScount || m_LPSind))
	{
		m_GCD->GCD_getRunCount(ROM, mmc, m_code_num, &(m_MPScount), &(m_LPSind));
	}

	if (m_MPScount)
	{
		bit = 0;
		m_MPScount--;
	}
	else
	{
		bit = 1;
		m_LPSind = 0;
	}

	if (m_MPScount || m_LPSind)
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

static const SDD1_PEM_state PEM_evolution_table[33] =
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
	SDD1_PEM(
			SDD1_BG* associatedBG0, SDD1_BG* associatedBG1,
			SDD1_BG* associatedBG2, SDD1_BG* associatedBG3,
			SDD1_BG* associatedBG4, SDD1_BG* associatedBG5,
			SDD1_BG* associatedBG6, SDD1_BG* associatedBG7)
	{
		m_BG[0] = associatedBG0;
		m_BG[1] = associatedBG1;
		m_BG[2] = associatedBG2;
		m_BG[3] = associatedBG3;
		m_BG[4] = associatedBG4;
		m_BG[5] = associatedBG5;
		m_BG[6] = associatedBG6;
		m_BG[7] = associatedBG7;
	}

	SDD1_PEM_ContextInfo m_contextInfo[32];
	SDD1_BG* m_BG[8];

	void PEM_prepareDecomp();
	UINT8 PEM_getBit(UINT8 *ROM, UINT32 *mmc, UINT8 context);
} ;

void SDD1_PEM::PEM_prepareDecomp()
{
	for (int i = 0; i < 32; i++)
	{
		m_contextInfo[i].status = 0;
		m_contextInfo[i].MPS = 0;
	}
}

UINT8 SDD1_PEM::PEM_getBit(UINT8 *ROM, UINT32 *mmc, UINT8 context)
{
	UINT8 endOfRun;
	UINT8 bit;

	SDD1_PEM_ContextInfo *pContInfo = &(m_contextInfo)[context];
	UINT8 currStatus = pContInfo->status;
	const SDD1_PEM_state* pState = &(PEM_evolution_table[currStatus]);
	UINT8 currentMPS = pContInfo->MPS;

	bit = m_BG[pState->code_num]->BG_getBit(ROM, mmc, &endOfRun);

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
	SDD1_CM(SDD1_PEM* associatedPEM)
		: m_PEM(associatedPEM) { }

	UINT8 m_bitplanesInfo;
	UINT8 m_contextBitsInfo;
	UINT8 m_bit_number;
	UINT8 m_currBitplane;
	UINT16 m_prevBitplaneBits[8];
	SDD1_PEM* m_PEM;

	void CM_prepareDecomp(UINT8 *ROM, UINT32 *mmc, UINT32 first_byte);
	UINT8 CM_getBit(UINT8 *ROM, UINT32 *mmc);
} ;

void SDD1_CM::CM_prepareDecomp(UINT8 *ROM, UINT32 *mmc, UINT32 first_byte)
{
	INT32 i = 0;
	m_bitplanesInfo = ROM[SSD1_ADD(first_byte)] & 0xc0;
	m_contextBitsInfo = ROM[SSD1_ADD(first_byte)] & 0x30;
	m_bit_number = 0;
	for (i = 0; i < 8; i++)
	{
		m_prevBitplaneBits[i] = 0;
	}
	switch (m_bitplanesInfo)
	{
		case 0x00:
			m_currBitplane = 1;
			break;
		case 0x40:
			m_currBitplane = 7;
			break;
		case 0x80:
			m_currBitplane = 3;
			break;
	}
}

UINT8 SDD1_CM::CM_getBit(UINT8 *ROM, UINT32 *mmc)
{
	UINT8 currContext;
	UINT16 *context_bits;
	UINT8 bit = 0;

	switch (m_bitplanesInfo)
	{
		case 0x00:
			m_currBitplane ^= 0x01;
			break;
		case 0x40:
			m_currBitplane ^= 0x01;
			if (!(m_bit_number & 0x7f))
				m_currBitplane = ((m_currBitplane + 2) & 0x07);
			break;
		case 0x80:
			m_currBitplane ^= 0x01;
			if (!(m_bit_number & 0x7f))
				m_currBitplane ^= 0x02;
			break;
		case 0xc0:
			m_currBitplane = m_bit_number & 0x07;
			break;
	}

	context_bits = &(m_prevBitplaneBits)[m_currBitplane];

	currContext = (m_currBitplane & 0x01) << 4;
	switch (m_contextBitsInfo)
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

	bit = m_PEM->PEM_getBit(ROM, mmc, currContext);

	*context_bits <<= 1;
	*context_bits |= bit;

	m_bit_number++;

	return bit;
}

class SDD1_OL
{
public:
	SDD1_OL(SDD1_CM* associatedCM)
		: m_CM(associatedCM) { }

	UINT8 m_bitplanesInfo;
	UINT16 m_length;
	UINT8* m_buffer;
	SDD1_CM* m_CM;

	void OL_prepareDecomp(UINT8 *ROM, UINT32 *mmc, UINT32 first_byte, UINT16 out_len, UINT8 *out_buf);
	void OL_launch(UINT8 *ROM, UINT32 *mmc);
} ;

void SDD1_OL::OL_prepareDecomp(UINT8 *ROM, UINT32 *mmc, UINT32 first_byte, UINT16 out_len, UINT8 *out_buf)
{
	m_bitplanesInfo = ROM[SSD1_ADD(first_byte)] & 0xc0;
	m_length = out_len;
	m_buffer = out_buf;
}

void SDD1_OL::OL_launch(UINT8 *ROM, UINT32 *mmc)
{
	UINT8 i;
	UINT8 register1 = 0, register2 = 0;

	switch (m_bitplanesInfo)
	{
		case 0x00:
		case 0x40:
		case 0x80:
			i = 1;
			do
			{   // if length == 0, we output 2^16 bytes
				if (!i)
				{
					*(m_buffer++) = register2;
					i = ~i;
				}
				else
				{
					for (register1 = register2 = 0, i = 0x80; i; i >>= 1)
					{
						if (m_CM->CM_getBit(ROM, mmc))
							register1 |= i;

						if (m_CM->CM_getBit(ROM, mmc))
							register2 |= i;
					}
					*(m_buffer++) = register1;
				}
			} while (--(m_length));
			break;
		case 0xc0:
			do
			{
				for (register1 = 0, i = 0x01; i; i <<= 1)
				{
					if (m_CM->CM_getBit(ROM, mmc))
					{
						register1 |= i;
					}
				}
				*(m_buffer++) = register1;
			} while (--(m_length));
			break;
	}
}

class SDD1emu
{
public:
	SDD1emu(running_machine &machine);

	running_machine &machine() const { return m_machine; }

	SDD1_IM* m_IM;
	SDD1_GCD* m_GCD;
	SDD1_BG* m_BG0;   SDD1_BG* m_BG1;   SDD1_BG* m_BG2;   SDD1_BG* m_BG3;
	SDD1_BG* m_BG4;   SDD1_BG* m_BG5;   SDD1_BG* m_BG6;   SDD1_BG* m_BG7;
	SDD1_PEM* m_PEM;
	SDD1_CM* m_CM;
	SDD1_OL* m_OL;

	void SDD1emu_decompress(UINT8 *ROM, UINT32 *mmc, UINT32 in_buf, UINT16 out_len, UINT8 *out_buf);

private:
	running_machine& m_machine;
};

SDD1emu::SDD1emu(running_machine &machine)
	: m_machine(machine)
{
	m_IM = auto_alloc(machine, SDD1_IM());
	m_GCD = auto_alloc(machine, SDD1_GCD(m_IM));
	m_BG0 = auto_alloc(machine, SDD1_BG(m_GCD, 0));
	m_BG1 = auto_alloc(machine, SDD1_BG(m_GCD, 1));
	m_BG2 = auto_alloc(machine, SDD1_BG(m_GCD, 2));
	m_BG3 = auto_alloc(machine, SDD1_BG(m_GCD, 3));
	m_BG4 = auto_alloc(machine, SDD1_BG(m_GCD, 4));
	m_BG5 = auto_alloc(machine, SDD1_BG(m_GCD, 5));
	m_BG6 = auto_alloc(machine, SDD1_BG(m_GCD, 6));
	m_BG7 = auto_alloc(machine, SDD1_BG(m_GCD, 7));
	m_PEM = auto_alloc(machine, SDD1_PEM(m_BG0, m_BG1, m_BG2, m_BG3,
											m_BG4, m_BG5, m_BG6, m_BG7));
	m_CM = auto_alloc(machine, SDD1_CM(m_PEM));
	m_OL = auto_alloc(machine, SDD1_OL(m_CM));
}

void SDD1emu::SDD1emu_decompress(UINT8 *ROM, UINT32 *mmc, UINT32 in_buf, UINT16 out_len, UINT8 *out_buf)
{
	m_IM->IM_prepareDecomp(in_buf);
	m_BG0->BG_prepareDecomp();
	m_BG1->BG_prepareDecomp();
	m_BG2->BG_prepareDecomp();
	m_BG3->BG_prepareDecomp();
	m_BG4->BG_prepareDecomp();
	m_BG5->BG_prepareDecomp();
	m_BG6->BG_prepareDecomp();
	m_BG7->BG_prepareDecomp();
	m_PEM->PEM_prepareDecomp();
	m_CM->CM_prepareDecomp(ROM, mmc, in_buf);
	m_OL->OL_prepareDecomp(ROM, mmc, in_buf, out_len, out_buf);

	m_OL->OL_launch(ROM, mmc);
}

struct snes_sdd1_t
{
	UINT8 sdd1_enable;  // channel bit-mask
	UINT8 xfer_enable;  // channel bit-mask
	UINT32 mmc[4];      // memory map controller ROM indices

	struct
	{
		UINT32 addr;    // $43x2-$43x4 -- DMA transfer address
		UINT16 size;    // $43x5-$43x6 -- DMA transfer size
	} dma[8];

	SDD1emu* sdd1emu;
	struct
	{
		UINT8 *data;    // pointer to decompressed S-DD1 data (65536 bytes)
		UINT16 offset;  // read index into S-DD1 decompression buffer
		UINT32 size;    // length of data buffer; reads decrement counter, set ready to false at 0
		UINT8 ready;    // 1 when data[] is valid; 0 to invoke sdd1emu.decompress()
	} buffer;
} ;

static snes_sdd1_t snes_sdd1;

static void sdd1_init(running_machine& machine)
{
	snes_sdd1.sdd1_enable = 0x00;
	snes_sdd1.xfer_enable = 0x00;

	snes_sdd1.mmc[0] = 0 << 20;
	snes_sdd1.mmc[1] = 1 << 20;
	snes_sdd1.mmc[2] = 2 << 20;
	snes_sdd1.mmc[3] = 3 << 20;

	for (int i = 0; i < 8; i++)
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

	if ((addr & 0x4380) == 0x4300)
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

	if ((addr & 0x4380) == 0x4300)
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

	if (snes_sdd1.sdd1_enable & snes_sdd1.xfer_enable)
	{
		// at least one channel has S-DD1 decompression enabled...
		for (int i = 0; i < 8; i++)
		{
			if (snes_sdd1.sdd1_enable & snes_sdd1.xfer_enable & (1 << i))
			{
				// S-DD1 always uses fixed transfer mode, so address will not change during transfer
				if ((addr + 0xc00000) == snes_sdd1.dma[i].addr)
				{
					UINT8 data;
					if (!snes_sdd1.buffer.ready)
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
						snes_sdd1.sdd1emu->SDD1emu_decompress(ROM, snes_sdd1.mmc, addr, snes_sdd1.buffer.size, snes_sdd1.buffer.data);
						snes_sdd1.sdd1_enable = temp;

						snes_sdd1.buffer.ready = 1;
					}

					// fetch a decompressed byte; once buffer is depleted, disable channel and invalidate buffer
					data = snes_sdd1.buffer.data[(UINT16)snes_sdd1.buffer.offset++];
					if (snes_sdd1.buffer.offset >= snes_sdd1.buffer.size)
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
