// license:BSD-3-Clause
// copyright-holders:R. Belmont, Hans Ostermeyer
/*
    m68kmmu.h - PMMU implementation for 68851/68030/68040
            HMMU implementation for 68020 (II and LC variants)

    By R. Belmont and Hans Ostermeyer
*/

// MMU status register bit definitions

#if 0
#define MMULOG logerror
#else
#define MMULOG(...)
#endif

static constexpr uint16_t M68K_MMU_SR_BUS_ERROR       = 0x8000;
static constexpr uint16_t M68K_MMU_SR_SUPERVISOR_ONLY = 0x2000;
static constexpr uint16_t M68K_MMU_SR_WRITE_PROTECT   = 0x0800;
static constexpr uint16_t M68K_MMU_SR_INVALID         = 0x0400;
static constexpr uint16_t M68K_MMU_SR_MODIFIED        = 0x0200;
static constexpr uint16_t M68K_MMU_SR_TRANSPARENT     = 0x0040;

// MMU translation table descriptor field definitions

static constexpr uint32_t M68K_MMU_DF_DT              = 0x00000003;
static constexpr uint32_t M68K_MMU_DF_DT_INVALID      = 0x00000000;
static constexpr uint32_t M68K_MMU_DF_PAGE            = 0x00000001;
static constexpr uint32_t M68K_MMU_DF_DT_TABLE_4BYTE     = 0x00000002;
static constexpr uint32_t M68K_MMU_DF_DT_TABLE_8BYTE     = 0x00000003;
static constexpr uint32_t M68K_MMU_DF_WP              = 0x00000004;
static constexpr uint32_t M68K_MMU_DF_USED            = 0x00000008;
static constexpr uint32_t M68K_MMU_DF_MODIFIED        = 0x00000010;
static constexpr uint32_t M68K_MMU_DF_CI              = 0x00000040;
static constexpr uint32_t M68K_MMU_DF_SUPERVISOR      = 0000000100;
static constexpr uint32_t M68K_MMU_DF_ADDR_MASK       = 0xfffffff0;
static constexpr uint32_t M68K_MMU_DF_IND_ADDR_MASK   = 0xfffffffc;
// MMU ATC Fields

static constexpr uint32_t M68K_MMU_ATC_BUSERROR       = 0x08000000;
static constexpr uint32_t M68K_MMU_ATC_CACHE_IN       = 0x04000000;
static constexpr uint32_t M68K_MMU_ATC_WRITE_PR       = 0x02000000;
static constexpr uint32_t M68K_MMU_ATC_MODIFIED       = 0x01000000;
static constexpr uint32_t M68K_MMU_ATC_MASK           = 0x00ffffff;
static constexpr uint32_t M68K_MMU_ATC_SHIFT          = 8;
static constexpr uint32_t M68K_MMU_ATC_VALID          = 0x08000000;

// MMU Translation Control register
static constexpr uint32_t M68K_MMU_TC_SRE             = 0x02000000;
static constexpr uint32_t M68K_MMU_TC_FCL             = 0x01000000;


/* decodes the effective address */
uint32_t DECODE_EA_32(int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 2:     // (An)
		{
			return REG_A()[reg];
		}
		case 3:     // (An)+
		{
			uint32_t ea = EA_AY_PI_32();
			return ea;
		}
		case 5:     // (d16, An)
		{
			uint32_t ea = EA_AY_DI_32();
			return ea;
		}
		case 6:     // (An) + (Xn) + d8
		{
			uint32_t ea = EA_AY_IX_32();
			return ea;
		}
		case 7:
		{
			switch (reg)
			{
				case 0:     // (xxx).W
				{
					uint32_t ea = OPER_I_16();
					return ea;
				}
				case 1:     // (xxx).L
				{
					uint32_t d1 = OPER_I_16();
					uint32_t d2 = OPER_I_16();
					uint32_t ea = (d1 << 16) | d2;
					return ea;
				}
				case 2:     // (d16, PC)
				{
					uint32_t ea = EA_PCDI_32();
					return ea;
				}
				default:    fatalerror("m68k: DECODE_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, m_pc);
			}
			break;
		}
		default:    fatalerror("m68k: DECODE_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, m_pc);
	}
	return 0;
}

void pmmu_set_buserror(uint32_t addr_in)
{
	if (++m_mmu_tmp_buserror_occurred == 1)
	{
		m_mmu_tmp_buserror_address = addr_in;
		m_mmu_tmp_buserror_rw = m_mmu_tmp_rw;
		m_mmu_tmp_buserror_fc = m_mmu_tmp_fc;
		m_mmu_tmp_buserror_sz = m_mmu_tmp_sz;
	}
}

/*
    pmmu_atc_add: adds this address to the ATC
*/
void pmmu_atc_add(uint32_t logical, uint32_t physical, int fc, const int rw)
{
	// get page size (i.e. # of bits to ignore); is 10 for Apollo
	int ps = (m_mmu_tc >> 20) & 0xf;
	// Note: exact emulation would use (logical >> ps) << (ps-8)

	uint32_t atc_tag = M68K_MMU_ATC_VALID | ((fc & 7) << 24) | logical >> ps;
	uint32_t atc_data = (physical >> ps) << (ps - 8);


	if (m_mmu_tmp_sr & (M68K_MMU_SR_BUS_ERROR|M68K_MMU_SR_INVALID|M68K_MMU_SR_SUPERVISOR_ONLY))
	{
		atc_data |= M68K_MMU_ATC_BUSERROR;
	}

	if (m_mmu_tmp_sr & M68K_MMU_SR_WRITE_PROTECT)
	{
		atc_data |= M68K_MMU_ATC_WRITE_PR;
	}

	if (!rw && !(m_mmu_tmp_sr & M68K_MMU_SR_WRITE_PROTECT))
	{
		atc_data |= M68K_MMU_ATC_MODIFIED;
	}

	// first see if this is already in the cache
	for (int i = 0; i < MMU_ATC_ENTRIES; i++)
	{
		// if tag bits and function code match, don't add
		if (m_mmu_atc_tag[i] == atc_tag)
		{
			MMULOG("%s: hit, old %08x new %08x\n", __func__, m_mmu_atc_data[i], atc_data);
			m_mmu_atc_data[i] = atc_data;
			return;
		}
	}

	// find an open entry
	int found = -1;
	for (int i = 0; i < MMU_ATC_ENTRIES; i++)
	{
		if (!(m_mmu_atc_tag[i] & M68K_MMU_ATC_VALID))
		{
			found = i;
			break;
		}
	}

	// did we find an entry?  steal one by round-robin then
	if (found == -1)
	{
		found = m_mmu_atc_rr++;

		if (m_mmu_atc_rr >= MMU_ATC_ENTRIES)
		{
			m_mmu_atc_rr = 0;
		}
	}

	// add the entry
	MMULOG("ATC[%2d] add: log %08x -> phys %08x (fc=%d) data=%08x\n",
			found, (logical >> ps) << ps, (physical >> ps) << ps, fc, atc_data);
	m_mmu_atc_tag[found] = atc_tag;
	m_mmu_atc_data[found] = atc_data;
}

/*
    pmmu_atc_flush: flush entire ATC

    7fff0003 001ffd10 80f05750 is what should load
*/
void pmmu_atc_flush()
{
	MMULOG("ATC flush: pc=%08x\n", m_ppc);
	std::fill(std::begin(m_mmu_atc_tag), std::end(m_mmu_atc_tag), 0);
	m_mmu_atc_rr = 0;
}


bool pmmu_atc_lookup(const uint32_t addr_in, const int fc, const bool rw,
					 const int ptest, uint32_t& addr_out)
{
	MMULOG("%s: LOOKUP addr_in=%08x, fc=%d, ptest=%d\n", __func__, addr_in, fc, ptest);
	const int ps = (m_mmu_tc >> 20) & 0xf;
	const uint32_t atc_tag = M68K_MMU_ATC_VALID | ((fc & 7) << 24) | (addr_in >> ps);

	for (int i = 0; i < MMU_ATC_ENTRIES; i++)
	{
		if (m_mmu_atc_tag[i] != atc_tag)
		{
			continue;
		}

		if (!ptest && !rw)
		{
			/* According to MC86030UM:
			 * "If the M bit is clear and a write access to this logical
				address is attempted, the MC68030 aborts the access and initiates a table
				search, setting the M bit in the page descriptor, invalidating the old ATC
				entry, and creating a new entry with the M bit set."
			 */
			if (!(m_mmu_atc_data[i] & M68K_MMU_ATC_MODIFIED))
			{
				m_mmu_atc_tag[i] = 0;
				continue;
			}
		}

		m_mmu_tmp_sr = 0;
		if (m_mmu_atc_data[i] & M68K_MMU_ATC_MODIFIED)
		{
			m_mmu_tmp_sr |= M68K_MMU_SR_MODIFIED;
		}

		if (m_mmu_atc_data[i] & M68K_MMU_ATC_WRITE_PR)
		{
			m_mmu_tmp_sr |= M68K_MMU_SR_WRITE_PROTECT;
		}

		if (m_mmu_atc_data[i] & M68K_MMU_ATC_BUSERROR)
		{
			m_mmu_tmp_sr |= M68K_MMU_SR_BUS_ERROR|M68K_MMU_SR_INVALID;
		}
		addr_out = (m_mmu_atc_data[i] << 8) | (addr_in & ~(~0 << ps));
		MMULOG("%s: addr_in=%08x, addr_out=%08x, MMU SR %04x\n",
				__func__, addr_in, addr_out, m_mmu_tmp_sr);
		return true;
	}
	MMULOG("%s: lookup failed\n", __func__);
	if (ptest)
	{
		m_mmu_tmp_sr = M68K_MMU_SR_INVALID;
	}
	return false;
}

inline uint32_t get_dt3_table_entry(uint32_t tptr, uint8_t fc, const bool rw, uint8_t ptest)
{
	uint32_t tbl_entry2 = m_program->read_dword(tptr);
	uint32_t tbl_entry = m_program->read_dword(tptr + 4);
	uint32_t dt = tbl_entry2 & M68K_MMU_DF_DT;

	m_mmu_tmp_sr |= ((tbl_entry2 & 0x0100) && !(fc & 4)) ? M68K_MMU_SR_SUPERVISOR_ONLY : 0;
	m_mmu_tmp_sr |= tbl_entry2 & 0x0004 ? M68K_MMU_SR_WRITE_PROTECT : 0;

	if (!ptest && dt != M68K_MMU_DF_DT_INVALID)
	{
		if (dt == M68K_MMU_DF_PAGE && !rw && !(m_mmu_tmp_sr & M68K_MMU_SR_WRITE_PROTECT))
		{
			// set modified
			m_program->write_dword( tptr, tbl_entry2 | M68K_MMU_DF_USED | M68K_MMU_DF_MODIFIED);
		}
		else if (!(tbl_entry2 & M68K_MMU_DF_USED))
		{
			m_program->write_dword( tptr, tbl_entry2 | M68K_MMU_DF_USED);
		}
	}
	return (tbl_entry & ~M68K_MMU_DF_DT) | dt;
}

bool pmmu_match_tt(uint32_t addr_in, const int fc, const uint32_t tt, const bool rw)
{
	if (!(tt & 0x8000))
	{
		return false;
	}

	// transparent translation enabled
	const uint32_t address_base = tt & 0xff000000;
	const uint32_t address_mask = ((tt << 8) & 0xff000000) ^ 0xff000000;
	const uint32_t fcmask = (~tt) & 7;
	const uint32_t fcbits = (tt >> 4) & 7;
	const bool rwmask = (~tt & 0x100);
	const bool rwbit = (tt & 0x200);

	if ((addr_in & address_mask) != (address_base & address_mask))
	{
		return false;
	}

	if ((fc & fcmask) != (fcbits & fcmask))
	{
		return false;
	}

	if ((rw & rwmask) != (rwbit & rwmask))
	{
		return false;
	}

	m_mmu_tmp_sr |= M68K_MMU_SR_TRANSPARENT;
	return true;
}

void update_table_descriptor(uint32_t tptr, uint32_t tbl_entry, const bool rw)
{
	MMULOG("%s: set U @ %08x\n", __func__, tptr);
	m_program->write_dword( tptr, tbl_entry | M68K_MMU_DF_USED);
}

void update_page_descriptor(uint32_t tptr, uint32_t tbl_entry, const bool rw)
{
	if (!rw && !(m_mmu_tmp_sr & M68K_MMU_SR_WRITE_PROTECT))
	{
		// set used and modified
		MMULOG("%s: set M+U @ %08x\n", __func__, tptr);
		m_program->write_dword( tptr, tbl_entry | M68K_MMU_DF_USED | M68K_MMU_DF_MODIFIED);
	}
	else if (!(tbl_entry & M68K_MMU_DF_USED))
	{
		MMULOG("%s: set U @ %08x\n", __func__, tptr);
		m_program->write_dword( tptr, tbl_entry | M68K_MMU_DF_USED);
	}
}

bool pmmu_walk_table(uint32_t& tbl_entry, int &type, uint32_t addr_in, int shift, int bits, bool ptest, int fc, int &level, const bool rw, uint32_t &addr_out)
{
	// get table offset
	uint32_t tofs;
	int ps = (m_mmu_tc >> 20) & 0xf;
	// get initial shift (# of top bits to ignore)
	int is = (m_mmu_tc >> 16) & 0xf;
	shift += is;
	tofs = (addr_in << shift) >> (32 - bits);

	MMULOG("walk_table: SR %04x type %d, addr_in %08x, addr_out %08x, tbl_entry %08x, tofs %08x shift %d, bits %d, rw %d level %d\n",
			m_mmu_tmp_sr, type, addr_in, addr_out, tbl_entry, tofs, shift, bits, rw, level);

	// abort page table walk immediately on buserror,
	// but accumulate status bits on PTEST
	if ((m_mmu_tmp_sr & M68K_MMU_SR_BUS_ERROR) ||
		(!ptest && !rw && (m_mmu_tmp_sr & M68K_MMU_SR_WRITE_PROTECT)))
	{
		return true;
	}

	m_mmu_tmp_sr &= ~M68K_MMU_DF_DT;
	m_mmu_tmp_sr |= level;

	switch (type)
	{
		case M68K_MMU_DF_DT_INVALID:   // invalid, will cause MMU exception
			m_mmu_tmp_sr |= M68K_MMU_SR_INVALID;
			MMULOG("PMMU: DT0 PC=%x (addr_in %08x -> %08x)\n", m_ppc, addr_in, addr_out);
			return true;

		case M68K_MMU_DF_PAGE:   // page descriptor, will cause direct mapping
			if (tbl_entry & M68K_MMU_DF_MODIFIED)
			{
				MMULOG("%s: set modified in SR\n", __func__);
				m_mmu_tmp_sr |= M68K_MMU_SR_MODIFIED;
			}

			if (tbl_entry & M68K_MMU_DF_WP)
			{
				m_mmu_tmp_sr |= M68K_MMU_SR_WRITE_PROTECT;
			}

			if (!ptest)
			{
				if (level)
					update_page_descriptor(addr_out, tbl_entry, rw);
				tbl_entry &= (~0 << ps);
				addr_out = ((addr_in << shift) >> shift) + tbl_entry;
			}
			MMULOG("PMMU: DT1 PC=%x (addr_in %08x -> %08x)\n", m_ppc, addr_in, addr_out);
			return true;

		case M68K_MMU_DF_DT_TABLE_4BYTE:   // valid 4 byte descriptors
			tofs *= 4;

			if (bits)
			{
				// Short table descriptor
				if (level)
				{
					update_table_descriptor(addr_out, tbl_entry, rw);
				}

				if (tbl_entry & M68K_MMU_DF_WP)
				{
					m_mmu_tmp_sr |= M68K_MMU_SR_WRITE_PROTECT;
				}

				addr_out = (tbl_entry & M68K_MMU_DF_ADDR_MASK) + tofs;
			}
			else
			{
				// Indirect descriptor (no flags, but 30 bit address)
				addr_out = (tbl_entry & M68K_MMU_DF_IND_ADDR_MASK);
			}
			tbl_entry = m_program->read_dword(addr_out);
			type = tbl_entry & M68K_MMU_DF_DT;
			level++;

				return false;

		case M68K_MMU_DF_DT_TABLE_8BYTE: // valid 8 byte descriptors
			tofs *= 8;
			addr_out = (tbl_entry & M68K_MMU_DF_ADDR_MASK) + tofs;
			tbl_entry = get_dt3_table_entry(addr_out, fc,  rw, ptest);
			type = tbl_entry & M68K_MMU_DF_DT;
			level++;
			MMULOG("PMMU: DT3 read table A entries at %08x = %08x\n", addr_out, tbl_entry);
			return false;
	}
	return true;
}

/*
    pmmu_translate_addr_with_fc: perform 68851/68030-style PMMU address translation
*/
uint32_t pmmu_translate_addr_with_fc(uint32_t addr_in, uint8_t fc, bool rw, bool ptest, bool pload, const int limit = 7)
{
	uint32_t addr_out, tbl_entry;


	MMULOG("%s: addr_in=%08x, fc=%d, ptest=%d, rw=%d, limit=%d\n",
			__func__, addr_in, fc, ptest, rw, limit);
	m_mmu_tmp_sr = 0;

	if (pmmu_match_tt(addr_in, fc, m_mmu_tt0, rw) ||
		pmmu_match_tt(addr_in, fc, m_mmu_tt1, rw) ||
		fc == 7)
	{
		return addr_in;
	}

	if ((!ptest || limit == 0) && !pload && pmmu_atc_lookup(addr_in, fc, rw, ptest, addr_out))
	{
		if (m_mmu_tmp_sr & M68K_MMU_SR_BUS_ERROR)
		{
			MMULOG("set atc hit buserror: addr_in=%08x, addr_out=%x, rw=%x, fc=%d, sz=%d\n",
					addr_in, addr_out, m_mmu_tmp_rw, m_mmu_tmp_fc, m_mmu_tmp_sz);
			pmmu_set_buserror(addr_in);
		}
		return addr_out;
	}

	if (ptest && limit == 0)
	{
		return addr_out;
	}

	int type;

	// if SRP is enabled and we're in supervisor mode, use it
	if ((m_mmu_tc & M68K_MMU_TC_SRE) && (fc & 4))
	{
		tbl_entry = m_mmu_srp_aptr & M68K_MMU_DF_ADDR_MASK;
		type = m_mmu_srp_limit & M68K_MMU_DF_DT;
	}
	else    // else use the CRP
	{
		tbl_entry = m_mmu_crp_aptr & M68K_MMU_DF_ADDR_MASK;
		type = m_mmu_crp_limit & M68K_MMU_DF_DT;
	}

	m_mmu_tmp_sr = 0;
	int abits = (m_mmu_tc >> 12) & 0xf;
	int bbits = (m_mmu_tc >> 8) & 0xf;
	int cbits = (m_mmu_tc >> 4) & 0xf;
	int dbits = m_mmu_tc & 0x0f;
	int level = 0;
	int roottype = type;
	m_mmu_tablewalk = true;

	if (m_mmu_tc & M68K_MMU_TC_FCL)
	{
		if (pmmu_walk_table(tbl_entry, type, fc, 0, 32, ptest, fc, level, rw, addr_out))
			goto out;
	}

	if (!pmmu_walk_table(tbl_entry, type, addr_in, 0                            , abits, ptest, fc, level, rw, addr_out) &&
		!pmmu_walk_table(tbl_entry, type, addr_in, abits                        , bbits, ptest, fc, level, rw, addr_out) &&
		!pmmu_walk_table(tbl_entry, type, addr_in, abits + bbits                , cbits, ptest, fc, level, rw, addr_out) &&
		!pmmu_walk_table(tbl_entry, type, addr_in, abits + bbits + cbits        , dbits, ptest, fc, level, rw, addr_out) &&
		!pmmu_walk_table(tbl_entry, type, addr_in, abits + bbits + cbits + dbits,     0, ptest, fc, level, rw, addr_out))
	{
		fatalerror("Table walk did not resolve\n");
	}

out:
	m_mmu_tablewalk = false;
	if (ptest)
	{
		return addr_out;
	}

	if ((m_mmu_tmp_sr & (M68K_MMU_SR_INVALID|M68K_MMU_SR_SUPERVISOR_ONLY)) ||
			((m_mmu_tmp_sr & M68K_MMU_SR_WRITE_PROTECT) && !rw))
	{
		MMULOG("%s: set buserror (SR %04X)\n", __func__, m_mmu_tmp_sr);
		if (!pload)
		{
			pmmu_set_buserror(addr_in);
		}
	}

	// it seems like at least the 68030 sets the M bit in the MMU SR
	// if the root descriptor is of PAGE type, so do a logical and
	// between RW and the root type
	pmmu_atc_add(addr_in, addr_out, fc, rw && roottype != 1);
	MMULOG("PMMU: [%08x] => [%08x] (SR %04x)\n", addr_in, addr_out, m_mmu_tmp_sr);
	return addr_out;
}


// FC bits: 2 = supervisor, 1 = program, 0 = data
// the 68040 is a subset of the 68851 and 68030 PMMUs - the page table sizes are fixed, there is no early termination, etc, etc.
uint32_t pmmu_translate_addr_with_fc_040(uint32_t addr_in, uint8_t fc, uint8_t ptest)
{
	uint32_t addr_out, tt0, tt1;

	addr_out = addr_in;
	m_mmu_tmp_sr = 0;

	// transparent translation registers are always in force even if the PMMU itself is disabled
	// they don't do much in emulation because we never write out of order, but the write-protect and cache control features
	// are emulatable, and apparently transparent translation regions skip the page table lookup.
	if (fc & 1) // data, use DTT0/DTT1
	{
		tt0 = m_mmu_dtt0;
		tt1 = m_mmu_dtt1;
	}
	else if (fc & 2)    // program, use ITT0/ITT1
	{
		tt0 = m_mmu_itt0;
		tt1 = m_mmu_itt1;
	}
	else
	{
		fatalerror("68040: function code %d is neither data nor program!\n", fc & 7);
	}

	if (tt0 & 0x8000)
	{
		static constexpr int fcmask[4] = { 4, 4, 0, 0 };
		static constexpr int fcmatch[4] = { 0, 4, 0, 0 };
		uint32_t mask = (tt0 >> 16) & 0xff;
		mask ^= 0xff;
		mask <<= 24;

		if ((addr_in & mask) == (tt0 & mask) && (fc & fcmask[(tt0 >> 13) & 3]) == fcmatch[(tt0 >> 13) & 3])
		{
			MMULOG("TT0 match on address %08x (TT0 = %08x, mask = %08x)\n", addr_in, tt0, mask);
			if ((tt0 & 4) && !m_mmu_tmp_rw && !ptest)   // write protect?
			{
				pmmu_set_buserror(addr_in);
			}

			return addr_in;
		}
	}

	if (tt1 & 0x8000)
	{
		static int fcmask[4] = { 4, 4, 0, 0 };
		static int fcmatch[4] = { 0, 4, 0, 0 };
		uint32_t mask = (tt1 >> 16) & 0xff;
		mask ^= 0xff;
		mask <<= 24;

		if ((addr_in & mask) == (tt1 & mask) && (fc & fcmask[(tt1 >> 13) & 3]) == fcmatch[(tt1 >> 13) & 3])
		{
			MMULOG("TT1 match on address %08x (TT0 = %08x, mask = %08x)\n", addr_in, tt1, mask);
			if ((tt1 & 4) && !m_mmu_tmp_rw && !ptest)   // write protect?
			{
					pmmu_set_buserror(addr_in);
			}

			return addr_in;
		}
	}

	if (m_pmmu_enabled)
	{
		uint32_t root_idx = (addr_in >> 25) & 0x7f;
		uint32_t ptr_idx = (addr_in >> 18) & 0x7f;
		uint32_t page_idx, page;
		uint32_t root_ptr, pointer_ptr, page_ptr;
		uint32_t root_entry, pointer_entry, page_entry;

		// select supervisor or user root pointer
		if (fc & 4)
		{
			root_ptr = m_mmu_srp_aptr + (root_idx<<2);
		}
		else
		{
			root_ptr = m_mmu_urp_aptr + (root_idx<<2);
		}

		// get the root entry
		root_entry = m_program->read_dword(root_ptr);

		// is UDT marked valid?
		if (root_entry & 2)
		{
			// we're accessing through this root entry, so set the U bit
			if ((!(root_entry & 0x8)) && (!ptest))
			{
				root_entry |= 0x8;
				m_program->write_dword(root_ptr, root_entry);
			}

			// PTEST: any write protect bits set in the search tree will set W in SR
			if ((ptest) && (root_entry & 4))
			{
				m_mmu_tmp_sr |= 4;
			}

			pointer_ptr = (root_entry & ~0x1ff) + (ptr_idx<<2);
			pointer_entry = m_program->read_dword(pointer_ptr);

			// PTEST: any write protect bits set in the search tree will set W in SR
			if ((ptest) && (pointer_entry & 4))
			{
				m_mmu_tmp_sr |= 4;
			}

			// update U bit on this pointer entry too
			if ((!(pointer_entry & 0x8)) && (!ptest))
			{
				pointer_entry |= 0x8;
				m_program->write_dword(pointer_ptr, pointer_entry);
			}

			MMULOG("pointer entry = %08x\n", pointer_entry);

			// write protected by the root or pointer entries?
			if ((((root_entry & 4) && !m_mmu_tmp_rw) || ((pointer_entry & 4) && !m_mmu_tmp_rw)) && !ptest)
			{
				pmmu_set_buserror(addr_in);
				return addr_in;
			}

			// is UDT valid on the pointer entry?
			if (!(pointer_entry & 2) && !ptest)
			{
				logerror("Invalid pointer entry!  PC=%x, addr=%x\n", m_ppc, addr_in);
				pmmu_set_buserror(addr_in);
				return addr_in;
			}

			// (fall out of these ifs into the page lookup below)
		}
		else // throw an error
		{
			logerror("Invalid root entry!  PC=%x, addr=%x\n", m_ppc, addr_in);

			if (!ptest)
			{
				pmmu_set_buserror(addr_in);
			}

			return addr_in;
		}

		// now do the page lookup
		if (m_mmu_tc & 0x4000)  // 8k pages?
		{
			page_idx = (addr_in >> 13) & 0x1f;
			page = addr_in & 0x1fff;
			pointer_entry &= ~0x7f;
			MMULOG("8k pages: index %x page %x\n", page_idx, page);
		}
		else    // 4k pages
		{
			page_idx = (addr_in >> 12) & 0x3f;
			page = addr_in & 0xfff;
			pointer_entry &= ~0xff;
			MMULOG("4k pages: index %x page %x\n", page_idx, page);
		}

		page_ptr = pointer_entry + (page_idx<<2);
		page_entry = m_program->read_dword(page_ptr);
		m_mmu_last_page_entry_addr = page_ptr;

		MMULOG("page_entry = %08x\n", page_entry);

		// resolve indirect page pointers
		while ((page_entry & 3) == 2)
		{
			page_entry = m_program->read_dword(page_entry & ~0x3);
			m_mmu_last_page_entry_addr = (page_entry & ~0x3);
		}
		m_mmu_last_page_entry = page_entry;

		// is the page write protected or supervisor protected?
		if ((((page_entry & 4) && !m_mmu_tmp_rw) || ((page_entry & 0x80) && !(fc & 4))) && !ptest)
		{
			pmmu_set_buserror(addr_in);
			return addr_in;
		}

		switch (page_entry & 3)
		{
			case 0: // invalid
				MMULOG("Invalid page entry!  PC=%x, addr=%x\n", m_ppc, addr_in);
				if (!ptest)
				{
					pmmu_set_buserror(addr_in);
				}

				return addr_in;

			case 1:
			case 3: // normal
				if (m_mmu_tc & 0x4000)  // 8k pages?
				{
					addr_out = (page_entry & ~0x1fff) | page;
				}
				else
				{
					addr_out = (page_entry & ~0xfff) | page;
				}

				if (!(ptest))
				{
					page_entry |= 0x8;  // always set the U bit

					// if we're writing, the M bit comes into play
					if (!m_mmu_tmp_rw)
					{
						page_entry |= 0x10; // set Modified
					}

					// if these updates resulted in a change, write the entry back where we found it
					if (page_entry != m_mmu_last_page_entry)
					{
						m_mmu_last_page_entry = page_entry;
						m_program->write_dword(m_mmu_last_page_entry_addr, m_mmu_last_page_entry);
					}
				}
				else
				{
					// page entry: UR G U1 U0 S CM CM M U W PDT
					// SR:         B  G U1 U0 S CM CM M 0 W T R
					m_mmu_tmp_sr |= ((addr_out & ~0xfff) || (page_entry & 0x7f4));
				}
				break;

			case 2: // shouldn't happen
				fatalerror("68040: got indirect final page pointer, shouldn't be possible\n");
				break;
		}
		//      if (addr_in != addr_out) MMULOG("040MMU: [%08x] => [%08x]\n", addr_in, addr_out);
	}

	return addr_out;
}

/*
    pmmu_translate_addr: perform 68851/68030-style PMMU address translation
*/
uint32_t pmmu_translate_addr(uint32_t addr_in, const bool rw)
{
	uint32_t addr_out;

	if (CPU_TYPE_IS_040_PLUS())
	{
		addr_out = pmmu_translate_addr_with_fc_040(addr_in, m_mmu_tmp_fc, 0);
	}
	else
	{
		addr_out = pmmu_translate_addr_with_fc(addr_in, m_mmu_tmp_fc, m_mmu_tmp_rw, 0, 0);
	}
	return addr_out;
}

/*
    m68851_mmu_ops: COP 0 MMU opcode handling
*/


int fc_from_modes(const uint16_t modes)
{
	if ((modes & 0x1f) == 0)
	{
		return m_sfc;
	}

	if ((modes & 0x1f) == 1)
	{
		return m_dfc;
	}

	if (m_cpu_type & CPU_TYPE_030)
	{
		// 68030 has 3 bits fc, but 68851 4 bits
		if (((modes >> 3) & 3) == 1)
		{
			return REG_D()[modes & 7] & 0x7;
		}

		if (((modes >> 3) & 3) == 2)
		{
			return modes & 7;
		}
	}
	else
	{
		if (((modes >> 3) & 3) == 1)
		{
			return REG_D()[modes & 7] & 0xf;
		}

		if (modes & 0x10)
		{
			return modes & 0xf;
		}
	}


	fatalerror("%s: unknown fc mode: 0x%02xn", __func__, modes & 0x1f);
	return 0;
}

void m68851_pload(const uint32_t ea, const uint16_t modes)
{
	uint32_t ltmp = DECODE_EA_32(ea);
	const int fc = fc_from_modes(modes);
	bool rw = (modes & 0x200);

	MMULOG("%s: PLOAD%c addr=%08x, fc=%d\n", __func__, rw ? 'R' : 'W', ltmp, fc);

	// MC68851 traps if MMU is not enabled, 030 not
	if (m_pmmu_enabled || (m_cpu_type & CPU_TYPE_030))
	{
		if (CPU_TYPE_IS_040_PLUS())
		{
			pmmu_translate_addr_with_fc_040(ltmp, fc, 0);
		}
		else
		{

			pmmu_translate_addr_with_fc(ltmp, fc, rw, 0, 1);
		}
	}
	else
	{
		MMULOG("PLOAD with MMU disabled on MC68851\n");
		m68ki_exception_trap(57);
		return;
	}
}

void m68851_ptest(const uint32_t ea, const uint16_t modes)
{
	uint32_t v_addr = DECODE_EA_32(ea);
	uint32_t p_addr;

	const int level = (modes >> 10) & 7;
	const bool rw = (modes & 0x200);
	const int fc = fc_from_modes(modes);

	MMULOG("PMMU: PTEST%c (%04X) pc=%08x sp=%08x va=%08x fc=%x level=%x a=%d, areg=%d\n",
			rw ? 'R' : 'W', modes, m_ppc, REG_A()[7], v_addr, fc, level,
					(modes & 0x100) ? 1 : 0, (modes >> 5) & 7);

	if (CPU_TYPE_IS_040_PLUS())
	{
		p_addr = pmmu_translate_addr_with_fc_040(v_addr, fc, 1);
	}
	else
	{
		p_addr = pmmu_translate_addr_with_fc(v_addr, fc, rw, 1, 0, level);
	}

	m_mmu_sr = m_mmu_tmp_sr;

	MMULOG("PMMU: PTEST result: %04x pa=%08x\n", m_mmu_sr, p_addr);
	if (modes & 0x100)
	{
		int areg = (modes >> 5) & 7;
		WRITE_EA_32(0x08 | areg, p_addr);
	}
}

void m68851_pmove_get(uint32_t ea, uint16_t modes)
{
	switch ((modes>>10) & 0x3f)
	{
	case 0x02: // transparent translation register 0
		WRITE_EA_32(ea, m_mmu_tt0);
		MMULOG("PMMU: pc=%x PMOVE from mmu_tt0=%08x\n", m_ppc, m_mmu_tt0);
		break;
	case 0x03: // transparent translation register 1
		WRITE_EA_32(ea, m_mmu_tt1);
		MMULOG("PMMU: pc=%x PMOVE from mmu_tt1=%08x\n", m_ppc, m_mmu_tt1);
		break;
	case 0x10:  // translation control register
		WRITE_EA_32(ea, m_mmu_tc);
		MMULOG("PMMU: pc=%x PMOVE from mmu_tc=%08x\n", m_ppc, m_mmu_tc);
		break;

	case 0x12: // supervisor root pointer
		WRITE_EA_64(ea, (uint64_t)m_mmu_srp_limit<<32 | (uint64_t)m_mmu_srp_aptr);
		MMULOG("PMMU: pc=%x PMOVE from SRP limit = %08x, aptr = %08x\n", m_ppc, m_mmu_srp_limit, m_mmu_srp_aptr);
		break;

	case 0x13: // CPU root pointer
		WRITE_EA_64(ea, (uint64_t)m_mmu_crp_limit<<32 | (uint64_t)m_mmu_crp_aptr);
		MMULOG("PMMU: pc=%x PMOVE from CRP limit = %08x, aptr = %08x\n", m_ppc, m_mmu_crp_limit, m_mmu_crp_aptr);
		break;

	default:
		logerror("680x0: PMOVE from unknown MMU register %x, PC %x\n", (modes>>10) & 7, m_pc);
		return;
	}

	if (!(modes & 0x100))   // flush ATC on moves to TC, SRP, CRP, TT with FD bit clear
	{
		pmmu_atc_flush();
	}

}

void m68851_pmove_put(uint32_t ea, uint16_t modes)
{
	uint64_t temp64;
	switch ((modes>>13) & 7)
	{
	case 0:
	{
		uint32_t temp = READ_EA_32(ea);

		if (((modes >> 10) & 7) == 2)
		{
			MMULOG("WRITE TT0 = 0x%08x\n", m_mmu_tt0);
			m_mmu_tt0 = temp;
		}
		else if (((modes >> 10) & 7) == 3)
		{
			MMULOG("WRITE TT1 = 0x%08x\n", m_mmu_tt1);
			m_mmu_tt1 = temp;
		}
		break;

		if (!(modes & 0x100))
		{
			pmmu_atc_flush();
		}
	}
	case 1:
		logerror("680x0: unknown PMOVE case 1, PC %x\n", m_pc);
		break;

	case 2:
		switch ((modes >> 10) & 7)
		{
		case 0: // translation control register
			m_mmu_tc = READ_EA_32(ea);
			MMULOG("PMMU: TC = %08x\n", m_mmu_tc);

			if (m_mmu_tc & 0x80000000)
			{
				int bits = 0;
				for (int shift = 20; shift >= 0; shift -= 4)
				{
					bits += (m_mmu_tc >> shift) & 0x0f;
				}

				if (bits != 32 || !((m_mmu_tc >> 23) & 1))
				{
					logerror("MMU: TC invalid!\n");
					m_mmu_tc &= ~0x80000000;
					m68ki_exception_trap(EXCEPTION_MMU_CONFIGURATION);
				} else {
					m_pmmu_enabled = 1;
				}
				MMULOG("PMMU enabled\n");
			}
			else
			{
				m_pmmu_enabled = 0;
				MMULOG("PMMU disabled\n");
			}

			if (!(modes & 0x100))   // flush ATC on moves to TC, SRP, CRP with FD bit clear
			{
				pmmu_atc_flush();
			}
			break;

		case 2: // supervisor root pointer
			temp64 = READ_EA_64(ea);
			m_mmu_srp_limit = (temp64 >> 32) & 0xffffffff;
			m_mmu_srp_aptr = temp64 & 0xffffffff;
			MMULOG("PMMU: SRP limit = %08x aptr = %08x\n", m_mmu_srp_limit, m_mmu_srp_aptr);
			// SRP type 0 is not allowed
			if ((m_mmu_srp_limit & 3) == 0)
			{
				m68ki_exception_trap(EXCEPTION_MMU_CONFIGURATION);
				return;
			}

			if (!(modes & 0x100))
			{
				pmmu_atc_flush();
			}
			break;

		case 3: // CPU root pointer
			temp64 = READ_EA_64(ea);
			m_mmu_crp_limit = (temp64 >> 32) & 0xffffffff;
			m_mmu_crp_aptr = temp64 & 0xffffffff;
			MMULOG("PMMU: CRP limit = %08x aptr = %08x\n", m_mmu_crp_limit, m_mmu_crp_aptr);
			// CRP type 0 is not allowed
			if ((m_mmu_crp_limit & 3) == 0)
			{
				m68ki_exception_trap(EXCEPTION_MMU_CONFIGURATION);
				return;
			}

			if (!(modes & 0x100))
			{
				pmmu_atc_flush();
			}
			break;

		case 7: // MC68851 Access Control Register
			if (m_cpu_type == CPU_TYPE_020)
			{
				// DomainOS on Apollo DN3000 will only reset this to 0
				uint16_t mmu_ac = READ_EA_16(ea);
				if (mmu_ac != 0)
				{
					MMULOG("680x0 PMMU: pc=%x PMOVE to mmu_ac=%08x\n",
							m_ppc, mmu_ac);
				}
				break;
			}
			// fall through; unknown PMOVE mode unless MC68020 with MC68851

		default:
			logerror("680x0: PMOVE to unknown MMU register %x, PC %x\n", (modes>>10) & 7, m_pc);
			break;
		}
		break;
		case 3: // MMU status
			uint32_t temp = READ_EA_32(ea);
			logerror("680x0: unsupported PMOVE %x to MMU status, PC %x\n", temp, m_pc);
			break;
	}
}


void m68851_pmove(uint32_t ea, uint16_t modes)
{
	switch ((modes >> 13) & 0x7)
	{
	case 0: // MC68030/040 form with FD bit
	case 2: // MC68851 form, FD never set
		if (modes & 0x200)
		{
			m68851_pmove_get(ea, modes);
			break;
		}
		else    // top 3 bits of modes: 010 for this, 011 for status, 000 for transparent translation regs
		{
			m68851_pmove_put(ea, modes);
			break;
		}
	case 3: // MC68030 to/from status reg
		if (modes & 0x200)
		{
			MMULOG("%s: read SR = %04x\n", __func__, m_mmu_sr);
			WRITE_EA_16(ea, m_mmu_sr);
		}
		else
		{
			m_mmu_sr = READ_EA_16(ea);
			MMULOG("%s: write SR = %04X\n", __func__, m_mmu_sr);
		}
		break;

	default:
		logerror("680x0: unknown PMOVE mode %x (modes %04x) (PC %x)\n", (modes >> 13) & 0x7, modes, m_pc);
		break;

	}
}

void m68851_mmu_ops()
{
	uint16_t modes;
	uint32_t ea = m_ir & 0x3f;


	// catch the 2 "weird" encodings up front (PBcc)
	if ((m_ir & 0xffc0) == 0xf0c0)
	{
		logerror("680x0: unhandled PBcc\n");
		return;
	}
	else if ((m_ir & 0xffc0) == 0xf080)
	{
		logerror("680x0: unhandled PBcc\n");
		return;
	}
	else if ((m_ir & 0xffe0) == 0xf500)
	{
		MMULOG("68040 pflush: pc=%08x ir=%04x opmode=%d register=%d\n", m_ppc, m_ir, (m_ir >> 3) & 3, m_ir & 7);
		pmmu_atc_flush();
	}
	else    // the rest are 1111000xxxXXXXXX where xxx is the instruction family
	{
		switch ((m_ir >> 9) & 0x7)
		{
			case 0:
				modes = OPER_I_16();

				if ((modes & 0xfde0) == 0x2000) // PLOAD
				{
					m68851_pload(ea, modes);
					return;
				}
				else if ((modes & 0xe200) == 0x2000)    // PFLUSH
				{
					if (((modes >> 10) & 7) != 1)
						logerror("PFLUSH by fc/ea not supported\n");
					pmmu_atc_flush();
					return;
				}
				else if (modes == 0xa000)   // PFLUSHR
				{
					pmmu_atc_flush();
					return;
				}
				else if (modes == 0x2800)   // PVALID (FORMAT 1)
				{
					logerror("680x0: unhandled PVALID1\n");
					return;
				}
				else if ((modes & 0xfff8) == 0x2c00)    // PVALID (FORMAT 2)
				{
					logerror("680x0: unhandled PVALID2\n");
					return;
				}
				else if ((modes & 0xe000) == 0x8000)    // PTEST
				{
					m68851_ptest(ea, modes);
					return;
				}
				else
				{
					m68851_pmove(ea, modes);
				}
				break;

			default:
				logerror("680x0: unknown PMMU instruction group %d\n", (m_ir >> 9) & 0x7);
				break;
		}
	}
}


/* Apple HMMU translation is much simpler */
inline uint32_t hmmu_translate_addr(uint32_t addr_in)
{
	uint32_t addr_out;

	addr_out = addr_in;

	// check if LC 24-bit mode is enabled - this simply blanks out A31, the V8 ignores A30-24 always
	if (m_hmmu_enabled == M68K_HMMU_ENABLE_LC)
	{
		addr_out = addr_in & 0xffffff;
	}
	else if (m_hmmu_enabled == M68K_HMMU_ENABLE_II) // the original II does a more complex translation
	{
		addr_out = addr_in & 0xffffff;

		if ((addr_out >= 0x800000) && (addr_out <= 0x8fffff))
		{
			addr_out |= 0x40000000; // ROM
		}
		else if ((addr_out >= 0x900000) && (addr_out <= 0xefffff))
		{
			addr_out = 0xf0000000;  // NuBus
			addr_out |= ((addr_in & 0xf00000)<<4);
			addr_out |= (addr_in & 0xfffff);
		}
		else if (addr_out >= 0xf00000)
		{
			addr_out |= 0x50000000; // I/O
		}

		// (RAM is at 0 and doesn't need special massaging)
	}

	return addr_out;
}

public:
void m68851_buserror()
{
	if (m_mmu_tablewalk)
	{
		MMULOG("buserror during table walk\n");
		m_mmu_tmp_sr |= M68K_MMU_SR_BUS_ERROR|M68K_MMU_SR_INVALID;
	}
}
