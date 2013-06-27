/* SHARC memory operations */

static UINT32 pm_read32(SHARC_REGS *cpustate, UINT32 address)
{
	if (address >= 0x20000 && address < 0x28000)
	{
		UINT32 addr = (address & 0x7fff) * 3;

		return (UINT32)(cpustate->internal_ram_block0[addr + 0] << 16) |
						(cpustate->internal_ram_block0[addr + 1]);
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		UINT32 addr = (address & 0x7fff) * 3;

		return (UINT32)(cpustate->internal_ram_block1[addr + 0] << 16) |
						(cpustate->internal_ram_block1[addr + 1]);
	}
	else {
		fatalerror("SHARC: PM Bus Read %08X at %08X\n", address, cpustate->pc);
	}
}

static void pm_write32(SHARC_REGS *cpustate, UINT32 address, UINT32 data)
{
	if (address >= 0x20000 && address < 0x28000)
	{
		UINT32 addr = (address & 0x7fff) * 3;

		cpustate->internal_ram_block0[addr + 0] = (UINT16)(data >> 16);
		cpustate->internal_ram_block0[addr + 1] = (UINT16)(data);
		return;
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		UINT32 addr = (address & 0x7fff) * 3;

		cpustate->internal_ram_block1[addr + 0] = (UINT16)(data >> 16);
		cpustate->internal_ram_block1[addr + 1] = (UINT16)(data);
		return;
	}
	else {
		fatalerror("SHARC: PM Bus Write %08X, %08X at %08X\n", address, data, cpustate->pc);
	}
}

static UINT64 pm_read48(SHARC_REGS *cpustate, UINT32 address)
{
	if (address >= 0x20000 && address < 0x28000)
	{
		UINT32 addr = (address & 0x7fff) * 3;

		return ((UINT64)(cpustate->internal_ram_block0[addr + 0]) << 32) |
				((UINT64)(cpustate->internal_ram_block0[addr + 1]) << 16) |
				((UINT64)(cpustate->internal_ram_block0[addr + 2]) << 0);
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		UINT32 addr = (address & 0x7fff) * 3;

		return ((UINT64)(cpustate->internal_ram_block1[addr + 0]) << 32) |
				((UINT64)(cpustate->internal_ram_block1[addr + 1]) << 16) |
				((UINT64)(cpustate->internal_ram_block1[addr + 2]) << 0);
	}
	else {
		fatalerror("SHARC: PM Bus Read %08X at %08X\n", address, cpustate->pc);
	}

	return 0;
}

static void pm_write48(SHARC_REGS *cpustate, UINT32 address, UINT64 data)
{
	if (address >= 0x20000 && address < 0x28000)
	{
		UINT32 addr = (address & 0x7fff) * 3;

		cpustate->internal_ram_block0[addr + 0] = (UINT16)(data >> 32);
		cpustate->internal_ram_block0[addr + 1] = (UINT16)(data >> 16);
		cpustate->internal_ram_block0[addr + 2] = (UINT16)(data);
		return;
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		UINT32 addr = (address & 0x7fff) * 3;

		cpustate->internal_ram_block1[addr + 0] = (UINT16)(data >> 32);
		cpustate->internal_ram_block1[addr + 1] = (UINT16)(data >> 16);
		cpustate->internal_ram_block1[addr + 2] = (UINT16)(data);
		return;
	}
	else {
		fatalerror("SHARC: PM Bus Write %08X, %04X%08X at %08X\n", address, (UINT16)(data >> 32),(UINT32)data, cpustate->pc);
	}
}

static UINT32 dm_read32(SHARC_REGS *cpustate, UINT32 address)
{
	if (address < 0x100)
	{
		return sharc_iop_r(cpustate, address);
	}
	else if (address >= 0x20000 && address < 0x28000)
	{
		UINT32 addr = (address & 0x7fff) * 2;

		return (UINT32)(cpustate->internal_ram_block0[addr + 0] << 16) |
						(cpustate->internal_ram_block0[addr + 1]);
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		UINT32 addr = (address & 0x7fff) * 2;

		return (UINT32)(cpustate->internal_ram_block1[addr + 0] << 16) |
						(cpustate->internal_ram_block1[addr + 1]);
	}

	// short word addressing
	else if (address >= 0x40000 && address < 0x50000)
	{
		UINT32 addr = address & 0xffff;

		UINT16 r = cpustate->internal_ram_block0[addr ^ 1];
		if (cpustate->mode1 & 0x4000)
		{
			// sign-extend
			return (INT32)(INT16)(r);
		}
		else
		{
			return (UINT32)(r);
		}
	}
	else if (address >= 0x50000 && address < 0x80000)
	{
		// block 1 is mirrored in 0x50000...5ffff, 0x60000...0x6ffff and 0x70000...7ffff
		UINT32 addr = address & 0xffff;

		UINT16 r = cpustate->internal_ram_block1[addr ^ 1];
		if (cpustate->mode1 & 0x4000)
		{
			// sign-extend
			return (INT32)(INT16)(r);
		}
		else
		{
			return (UINT32)(r);
		}
	}

	return cpustate->data->read_dword(address << 2);
}

static void dm_write32(SHARC_REGS *cpustate, UINT32 address, UINT32 data)
{
	if (address < 0x100)
	{
		sharc_iop_w(cpustate, address, data);
		return;
	}
	else if (address >= 0x20000 && address < 0x28000)
	{
		UINT32 addr = (address & 0x7fff) * 2;

		cpustate->internal_ram_block0[addr + 0] = (UINT16)(data >> 16);
		cpustate->internal_ram_block0[addr + 1] = (UINT16)(data);
		return;
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		UINT32 addr = (address & 0x7fff) * 2;

		cpustate->internal_ram_block1[addr + 0] = (UINT16)(data >> 16);
		cpustate->internal_ram_block1[addr + 1] = (UINT16)(data);
		return;
	}

	// short word addressing
	else if (address >= 0x40000 && address < 0x50000)
	{
		UINT32 addr = address & 0xffff;

		cpustate->internal_ram_block0[addr ^ 1] = data;
		return;
	}
	else if (address >= 0x50000 && address < 0x80000)
	{
		// block 1 is mirrored in 0x50000...5ffff, 0x60000...0x6ffff and 0x70000...7ffff
		UINT32 addr = address & 0xffff;

		cpustate->internal_ram_block1[addr ^ 1] = data;
		return;
	}

	cpustate->data->write_dword(address << 2, data);
}
