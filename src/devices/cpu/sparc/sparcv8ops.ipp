// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//================================================================
//
//  sparcv8ops.ipp - Emulation for SPARCv8-class instructions
//
//================================================================


//-------------------------------------------------
//  execute_swap - execute a swap instruction
//-------------------------------------------------

void mb86901_device::execute_swap(uint32_t op)
{
	/* The SPARC Instruction Manual: Version 8, page 169, "Appendix C - ISP Descriptions - Atomic Load-Store Unsigned Byte Instructions" (SPARCv8.pdf, pg. 166)

	if (SWAP) then (
	    address <- r[rs1] + (if (i = 0) then r[rs2] else sign_extend(simm13));
	    addr_space <- (if (S = 0) then 10 else 11)
	) else if (SWAPA) then (
	    if (S = 0) then (
	        trap <- 1;
	        privileged_instruction <- 1
	    ) else if (i = 1) then (
	        trap <- 1;
	        illegal_instruction <- 1
	    ) else (
	        address <- r[rs1] + r[rs1];
	        addr_space <- asi
	    )
	);
	next;
	if (trap = 0) then (
	    temp <- r[rd];
	    while ( (pb_block_ldst_byte = 1) or (pb_block_ldst_word = 1) ) (
	        { wait for lock(s) to be lifted }
	        { an implementation actually need only block when another SWAP is pending on
	          the same word in memory as the one addressed by this SWAP, or a LDSTUB is
	          pending on any byte of the word in memory addressed by this SWAP }
	    );
	    next;
	    pb_block_ldst_word <- 1;
	    next;
	    (word, MAE) <- memory_read(addr_space, address);
	    next;
	    if (MAE = 1) then (
	        trap <- 1;
	        data_access_exception = 1
	    )
	next;
	if (trap = 0) then (
	    MAE <- memory_write(addr_space, address, 1111, temp);
	    next;
	    pb_block_ldst_word <- 0;
	    if (MAE = 1) then ( { MAE = 1 only due to a "non-resumable machine-check error" }
	        trap <- 1;
	        data_access_exception <- 1
	    ) else (
	        if (rd != 0) then r[rd] <- word
	    )
	);
	*/

	uint32_t address = 0;
	uint8_t addr_space = 0;
	if (SWAP)
	{
		address = RS1REG + (USEIMM ? SIMM13 : RS2REG);
		addr_space = (IS_USER ? 10 : 11);
	}
	else if (SWAPA)
	{
		if (IS_USER)
		{
			m_trap = 1;
			m_privileged_instruction = 1;
		}
		else if (USEIMM)
		{
			m_trap = 1;
			m_illegal_instruction = 1;
		}
		else
		{
			address = RS1REG + RS2REG;
			addr_space = ASI;
		}
	}

	uint32_t word = 0;
	uint32_t temp = 0;
	if (!m_trap)
	{
		temp = RDREG;
		while (m_pb_block_ldst_byte || m_pb_block_ldst_word)
		{
			// { wait for lock(s) to be lifted }
			// { an implementation actually need only block when another SWAP is pending on
			//   the same word in memory as the one addressed by this SWAP, or a LDSTUB is
			//   pending on any byte of the word in memory addressed by this SWAP }
		}

		m_pb_block_ldst_word = 1;

		word = read_sized_word(addr_space, address, 4);

		if (MAE)
		{
			m_trap = 1;
			m_data_access_exception = 1;
		}
	}
	if (!m_trap)
	{
		write_sized_word(addr_space, address, temp, 4);

		m_pb_block_ldst_word = 0;
		if (MAE)
		{
			m_trap = 1;
			m_data_access_exception = 1;
		}
		else
		{
			if (RD != 0)
				RDREG = word;
		}
	}
}


//-------------------------------------------------
//  execute_mul - execute a multiply opcode
//-------------------------------------------------

void mb86901_device::execute_mul(uint32_t op)
{
	/* The SPARC Instruction Manual: Version 8, page 175, "Appendix C - ISP Descriptions - Multiply Instructions" (SPARCv8.pdf, pg. 172)

	operand2 := if (i = 0) then r[rs2] else sign_extend(simm13);

	if (UMUL or UMULScc) then (Y, result) <- multiply_unsigned(r[rs1], operand2)
	else if (SMUL or SMULcc) then (Y, result) <- multiply_signed(r[rs1], operand2)
	next;
	if (rd != 0) then (
	    r[rd] <- result;
	)
	if (UMULcc or SMULcc) then (
	    N <- result<31>;
	    Z <- if (result = 0) then 1 else 0;
	    V <- 0
	    C <- 0
	);
	*/

	uint32_t operand2 = (USEIMM ? SIMM13 : RS2REG);

	uint32_t result = 0;
	if (UMUL || UMULCC)
	{
		uint64_t dresult = (uint64_t)RS1REG * (uint64_t)operand2;
		Y = (uint32_t)(dresult >> 32);
		result = (uint32_t)dresult;
	}
	else if (SMUL || SMULCC)
	{
		int64_t dresult = (int64_t)(int32_t)RS1REG * (int64_t)(int32_t)operand2;
		Y = (uint32_t)(dresult >> 32);
		result = (uint32_t)dresult;
	}

	if (RD != 0)
	{
		RDREG = result;
	}
	if (UMULCC || SMULCC)
	{
		CLEAR_ICC;
		PSR |= BIT31(result) ? PSR_N_MASK : 0;
		PSR |= (result == 0) ? PSR_Z_MASK : 0;
	}
}


//-------------------------------------------------
//  execute_div - execute a divide opcode
//-------------------------------------------------

void mb86901_device::execute_div(uint32_t op)
{
	/* The SPARC Instruction Manual: Version 8, page 176, "Appendix C - ISP Descriptions - Multiply Instructions" (SPARCv8.pdf, pg. 173)

	operand2 := if (i = 0) then r[rs2] else sign_extend(simm13);

	next;
	if (operand2 = 0) then (
	    trap <- 1;
	    division_by_zero <- 1
	) else (
	    if (UDIV or UDIVcc) then (
	        temp_64bit <- divide_unsigned(Y[]r[rs1], operand2);
	        next;
	        result <- temp_64bit<31:0>;
	        temp_V <- if (temp_64bit<63:32> = 0) then 0 else 1;
	    ) else if (SDIV or SDIVcc) then (
	        temp_64bit <- divide_signed(Y[]r[rs1], operand2);
	        next;
	        result <- temp_64bit<31:0>;
	        temp_V <- if (temp_64bit<63:31> = 0) or
	                     (temp_64bit<63:31> = (2^33 - 1)) ) then 0 else 1;
	    ) ;
	    next;

	    if (temp_V) then (
	        { result overflowed 32 bits; return largest appropriate integer }
	        if (UDIV or UDIVcc) then result <- 2^32 - 1;
	        else if (SDIV or SDIVcc) then (
	            if (temp_64bit > 0) then result <- 2^31 - 1;
	            else result <- -2^31
	        )
	    );
	    next;

	    if (rd != 0) then (
	        r[rd] <- result
	    ) ;
	    if (UDIVcc or SDIVcc) then (
	        N <- result<31>;
	        Z <- if (result = 0) then 1 else 0;
	        V <- temp_V;
	        C <- 0
	    )
	);
	*/

	uint32_t operand2 = (USEIMM ? SIMM13 : RS2REG);

	if (operand2 == 0)
	{
		m_trap = 1;
		m_division_by_zero = 1;
	}
	else
	{
		uint32_t result = 0;
		bool temp_v = false;
		int64_t temp_64bit = 0;
		if (UDIV || UDIVCC)
		{
			temp_64bit = int64_t(uint64_t((uint64_t(Y) << 32) | uint64_t(RS1REG)) / operand2);

			result = uint32_t(temp_64bit);

			temp_v = ((temp_64bit & 0xffffffff00000000) == 0) ? false : true;
		}
		else if (SDIV || SDIVCC)
		{
			temp_64bit = int64_t(int64_t((uint64_t(Y) << 32) | uint64_t(RS1REG)) / operand2);

			result = uint32_t(temp_64bit);

			uint64_t shifted = uint64_t(temp_64bit) >> 31;
			temp_v = (shifted == 0 || shifted == 0x1ffffffff) ? false : true;
		}

		if (temp_v)
		{
			if (UDIV || UDIVCC)
			{
				result = 0xffffffff;
			}
			else if (SDIV || SDIVCC)
			{
				if (temp_64bit > 0)
					result = 0x7fffffff;
				else
					result = 0x80000000;
			}
		}

		if (RD != 0)
			RDREG = result;

		if (UDIVCC || SDIVCC)
		{
			CLEAR_ICC;
			PSR |= BIT31(result) ? PSR_N_MASK : 0;
			PSR |= (result == 0) ? PSR_Z_MASK : 0;
			PSR |= temp_v ? PSR_V_MASK : 0;
		}
	}
}
