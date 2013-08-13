/***************************************************************************

    Intel 8089 I/O Processor

***************************************************************************/

#include "i8089.h"


//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define VERBOSE      1
#define VERBOSE_DMA  1

// channel control register fields
#define CC_TMC     ((m_r[CC].w >>  0) & 0x07)	// terminate on masked compare
#define CC_TBC     ((m_r[CC].w >>  3) & 0x03)	// terminate on byte count
#define CC_TX      ((m_r[CC].w >>  5) & 0x03)	// terminate on external signal
#define CC_TS      ((m_r[CC].w >>  7) & 0x01)	// terminate on single transfer
#define CC_CHAIN   ((m_r[CC].w >>  8) & 0x01)	// chaining
#define CC_LOCK    ((m_r[CC].w >>  9) & 0x01)	// actuate lock
#define CC_SOURCE  ((m_r[CC].w >> 10) & 0x01)	// source register
#define CC_SYNC    ((m_r[CC].w >> 11) & 0x03)	// synchronization
#define CC_TRANS   ((m_r[CC].w >> 13) & 0x01)	// translation
#define CC_FUNC    ((m_r[CC].w >> 14) & 0x03)	// function


//**************************************************************************
//  I/O CHANNEL
//**************************************************************************

#define MCFG_I8089_CHANNEL_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, I8089_CHANNEL, 0)

#define MCFG_I8089_CHANNEL_SINTR(_sintr) \
	downcast<i8089_channel *>(device)->set_sintr_callback(DEVCB2_##_sintr);

const device_type I8089_CHANNEL = &device_creator<i8089_channel>;

class i8089_channel : public device_t,
                      public device_execute_interface
{
public:
	// construction/destruction
	i8089_channel(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _sintr> void set_sintr_callback(_sintr sintr) { m_write_sintr.set_callback(sintr); }

	void set_control_block(offs_t address);
	void attention();
	bool priority();
	bool bus_load_limit();

	DECLARE_WRITE_LINE_MEMBER( ext_w );
	DECLARE_WRITE_LINE_MEMBER( drq_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void execute_run();

	int m_icount;

private:

	// opcodes
	void add_rm(int r, int m, int o = 0);
	void add_mr(int m, int r, int o = 0);
	void addb_rm(int r, int m, int o = 0);
	void addb_mr(int m, int r, int o = 0);
	void addbi_ri(int r, int i);
	void addbi_mi(int m, int i, int o = 0);
	void addi_ri(int r, int i);
	void addi_mi(int m, int i, int o = 0);
	void and_rm(int r, int m, int o = 0);
	void and_mr(int m, int r, int o = 0);
	void andb_rm(int r, int m, int o = 0);
	void andb_mr(int m, int r, int o = 0);
	void andbi_ri(int r, int i);
	void andbi_mi(int m, int i, int o = 0);
	void andi_ri(int r, int i);
	void andi_mi(int m, int i, int o = 0);
	void call(int m, int d, int o = 0);
	void clr(int m, int b, int o = 0);
	void dec_r(int r);
	void dec_m(int m, int o = 0);
	void decb(int m, int o = 0);
	void hlt();
	void inc_r(int r);
	void inc_m(int m, int o = 0);
	void incb(int m, int o = 0);
	void jbt(int m, int b, int d, int o = 0);
	void jmce(int m, int d, int o = 0);
	void jmcne(int m, int d, int o = 0);
	void jmp(int d);
	void jnbt(int m, int b, int d, int o = 0);
	void jnz_r(int r, int d);
	void jnz_m(int m, int d, int o = 0);
	void jnzb(int m, int d, int o = 0);
	void jz_r(int r, int d);
	void jz_m(int m, int d, int o = 0);
	void jzb(int m, int d, int o = 0);
	void lcall(int m, int d, int o = 0);
	void ljbt(int m, int b, int d, int o = 0);
	void ljmce(int m, int d, int o = 0);
	void ljmcne(int m, int d, int o = 0);
	void ljmp(int d);
	void ljnbt(int m, int b, int d, int o = 0);
	void ljnz_r(int r, int d);
	void ljnz_m(int m, int d, int o = 0);
	void ljnzb(int m, int d, int o = 0);
	void ljz_r(int r, int d);
	void ljz_m(int m, int d, int o = 0);
	void ljzb(int m, int d, int o = 0);
	void lpd(int p, int m, int o = 0);
	void lpdi(int p, int i, int o = 0);
	void mov_mr(int m, int r, int o = 0);
	void mov_rm(int r, int m, int o = 0);
	void mov_mm(int m1, int m2, int o1 = 0, int o2 = 0);
	void movb_mr(int m, int r, int o = 0);
	void movb_rm(int r, int m, int o = 0);
	void movb_mm(int m1, int m2, int o1 = 0, int o2 = 0);
	void movbi_ri(int r, int i);
	void movbi_mi(int m, int i, int o = 0);
	void movi_ri(int r, int i);
	void movi_mi(int m, int i, int o = 0);
	void movp_mp(int m, int p, int o = 0);
	void movp_pm(int p, int m, int o = 0);
	void nop();
	void not_r(int r);
	void not_m(int m, int o = 0);
	void not_rm(int r, int m, int o = 0);
	void notb_m(int m, int o = 0);
	void notb_rm(int r, int m, int o = 0);
	void or_rm(int r, int m, int o = 0);
	void or_mr(int m, int r, int o = 0);
	void orb_rm(int r, int m, int o = 0);
	void orb_mr(int m, int r, int o = 0);
	void orbi_ri(int r, int i);
	void orbi_mi(int m, int i, int o = 0);
	void ori_ri(int r, int i);
	void ori_mi(int m, int i, int o = 0);
	void setb(int m, int b, int o = 0);
	void sintr();
	void tsl(int m, int i, int d, int o = 0);
	void wid(int s, int d);
	void xfer();
	void invalid(int opc);

	// instruction fetch
	UINT16 displacement(int wb);
	UINT8 offset(int aa);
	UINT8 imm8();
	UINT16 imm16();

	void examine_ccw(UINT8 ccw);

	devcb2_write_line m_write_sintr;

	i8089_device *m_iop;

	// dma
	void terminate_dma(int offset);

	bool m_xfer_pending;
	UINT16 m_dma_value;
	int m_dma_state;

	// dma state
	enum
	{
		DMA_IDLE,
		DMA_WAIT_FOR_SOURCE_DRQ,
		DMA_FETCH,
		DMA_TRANSLATE,
		DMA_WAIT_FOR_DEST_DRQ,
		DMA_STORE,
		DMA_STORE_BYTE_HIGH,
		DMA_COMPARE,
		DMA_TERMINATE
	};

	// register
	enum
	{
		GA, // 20-bit general purpose address a
		GB, // 20-bit general purpose address b
		GC, // 20-bit general purpose address c
		BC, // byte count
		TP, // 20-bit task pointer
		IX, // byte count
		CC, // mask compare
		MC, // channel control

		// internal use register
		CP, // 20-bit control block pointer
		PP, // 20-bit parameter pointer
		PSW // program status word
	};

	struct
	{
		int w; // 20-bit address
		int t; // tag-bit
	}
	m_r[11];
};

i8089_channel::i8089_channel(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, I8089_CHANNEL, "Intel 8089 I/O Channel", tag, owner, clock, "i8089_channel", __FILE__),
	device_execute_interface(mconfig, *this),
	m_icount(0),
	m_write_sintr(*this),
	m_xfer_pending(false),
	m_dma_value(0),
	m_dma_state(DMA_IDLE)
{
}

void i8089_channel::device_start()
{
	// get parent device
	m_iop = downcast<i8089_device *>(owner());

	// set our instruction counter
	m_icountptr = &m_icount;

	// resolve callbacks
	m_write_sintr.resolve_safe();

	// register for save states
	save_item(NAME(m_xfer_pending));
	save_item(NAME(m_dma_value));
	save_item(NAME(m_dma_state));

	for (int i = 0; i < ARRAY_LENGTH(m_r); i++)
	{
		save_item(NAME(m_r[i].w), i);
		save_item(NAME(m_r[i].t), i);
	}
}

void i8089_channel::device_reset()
{
	m_xfer_pending = false;

	// initialize registers
	for (int i = 0; i < ARRAY_LENGTH(m_r); i++)
	{
		m_r[i].w = 0;
		m_r[i].t = 0;
	}
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

UINT16 i8089_channel::displacement(int wb)
{
	UINT16 displacement = 0;

	if (wb == 1)
	{
		displacement = m_iop->read_byte(m_r[TP].w);
		m_r[TP].w++;
	}
	else if (wb == 2)
	{
		displacement = m_iop->read_word(m_r[TP].w);
		m_r[TP].w += 2;
	}

	return displacement;
}

UINT8 i8089_channel::offset(int aa)
{
	UINT8 offset = 0;

	if (aa == 1)
	{
		offset = m_iop->read_byte(m_r[TP].w);
		m_r[TP].w++;
	}

	return offset;
}

UINT8 i8089_channel::imm8()
{
	UINT8 imm8 = m_iop->read_byte(m_r[TP].w);
	m_r[TP].w++;
	return imm8;
}

UINT16 i8089_channel::imm16()
{
	UINT16 imm16 = m_iop->read_word(m_r[TP].w);
	m_r[TP].w += 2;
	return imm16;
}

// adjust task pointer and continue execution
void i8089_channel::terminate_dma(int offset)
{
	logerror("%s('%s'): terminating dma transfer\n", shortname(), tag());
	m_r[TP].w += offset;
	m_r[PSW].w |= 1 << 2;
	m_r[PSW].w &= ~(1 << 6);
	m_dma_state = DMA_IDLE;
}

void i8089_channel::execute_run()
{
	do
	{
		// active transfer?
		if (BIT(m_r[PSW].w, 6))
		{
			// new transfer?
			if (BIT(m_r[PSW].w, 2))
			{
				// we are no longer executing task blocks
				m_r[PSW].w &= ~(1 << 2);
				m_xfer_pending = false;

				if (VERBOSE)
				{
					logerror("%s('%s'): ---- starting dma transfer ----\n", shortname(), tag());
					logerror("%s('%s'): ga = %06x, gb = %06x, gc = %06x\n", shortname(), tag(), m_r[GA].w, m_r[GB].w, m_r[GC].w);
					logerror("%s('%s'): bc = %04x, cc = %04x, mc = %04x\n", shortname(), tag(), m_r[BC].w, m_r[CC].w, m_r[MC].w);
				}
			}

			// todo: port transfers
			if (CC_FUNC != 0x03)
				fatalerror("%s('%s'): port transfer\n", shortname(), tag());

			switch (m_dma_state)
			{
			case DMA_IDLE:
				if (VERBOSE_DMA)
					logerror("%s('%s'): entering state: DMA_IDLE (bc = %04x)\n", shortname(), tag(), m_r[BC].w);

				// synchronize on source?
				if (CC_SYNC == 0x01)
					m_dma_state = DMA_WAIT_FOR_SOURCE_DRQ;
				else
					m_dma_state = DMA_FETCH;
				break;

			case DMA_WAIT_FOR_SOURCE_DRQ:
				fatalerror("%s('%s'): wait for source drq not supported\n", shortname(), tag());
				break;

			case DMA_FETCH:
				if (VERBOSE_DMA)
					logerror("%s('%s'): entering state: DMA_FETCH", shortname(), tag());

				// source is 16-bit?
				if (BIT(m_r[PSW].w, 1))
				{
					m_dma_value = m_iop->read_word(m_r[GA + CC_SOURCE].w);
					m_r[GA + CC_SOURCE].w += 2;
					m_r[BC].w -= 2;
				}
				// destination is 16-bit, byte count is even
				else if (BIT(m_r[PSW].w, 0) && !(m_r[BC].w & 1))
				{
					m_dma_value = m_iop->read_byte(m_r[GA + CC_SOURCE].w);
					m_r[GA + CC_SOURCE].w++;
					m_r[BC].w--;
				}
				// destination is 16-bit, byte count is odd
				else if (BIT(m_r[PSW].w, 0) && (m_r[BC].w & 1))
				{
					m_dma_value |= m_iop->read_byte(m_r[GA + CC_SOURCE].w) << 8;
					m_r[GA + CC_SOURCE].w++;
					m_r[BC].w--;
				}
				// 8-bit transfer
				else
				{
					m_dma_value = m_iop->read_byte(m_r[GA + CC_SOURCE].w);
					m_r[GA + CC_SOURCE].w++;
					m_r[BC].w--;
				}

				if (VERBOSE_DMA)
					logerror("[ %04x ]\n", m_dma_value);

				if (BIT(m_r[PSW].w, 0) && (m_r[BC].w & 1))
					m_dma_state = DMA_FETCH;
				else if (CC_TRANS)
					m_dma_state = DMA_TRANSLATE;
				else if (CC_SYNC == 0x02)
					m_dma_state = DMA_WAIT_FOR_DEST_DRQ;
				else
					m_dma_state = DMA_STORE;

				break;

			case DMA_TRANSLATE:
				fatalerror("%s('%s'): dma translate requested\n", shortname(), tag());
				break;

			case DMA_WAIT_FOR_DEST_DRQ:
				fatalerror("%s('%s'): wait for destination drq not supported\n", shortname(), tag());
				break;

			case DMA_STORE:
				if (VERBOSE_DMA)
					logerror("%s('%s'): entering state: DMA_STORE", shortname(), tag());

				// destination is 16-bit?
				if (BIT(m_r[PSW].w, 0))
				{
					m_iop->write_word(m_r[GB - CC_SOURCE].w, m_dma_value);
					m_r[GB - CC_SOURCE].w += 2;

					if (VERBOSE_DMA)
						logerror("[ %04x ]\n", m_dma_value);
				}
				// destination is 8-bit
				else
				{
					m_iop->write_byte(m_r[GB - CC_SOURCE].w, m_dma_value & 0xff);
					m_r[GB - CC_SOURCE].w++;

					if (VERBOSE_DMA)
						logerror("[ %02x ]\n", m_dma_value & 0xff);
				}

				if (CC_TMC & 0x03)
					m_dma_state = DMA_COMPARE;
				else
					m_dma_state = DMA_TERMINATE;

				break;

			case DMA_COMPARE:
				fatalerror("%s('%s'): dma compare requested\n", shortname(), tag());
				break;

			case DMA_TERMINATE:
				if (VERBOSE_DMA)
					logerror("%s('%s'): entering state: DMA_TERMINATE\n", shortname(), tag());

				// terminate on masked compare?
				if (CC_TMC & 0x03)
					fatalerror("%s('%s'): terminate on masked compare not supported\n", shortname(), tag());

				// terminate on byte count?
				else if (CC_TBC && m_r[BC].w == 0)
					terminate_dma((CC_TBC - 1) * 4);

				// terminate on external signal
				else if (CC_TX)
					fatalerror("%s('%s'): terminate on external signal not supported\n", shortname(), tag());

				// terminate on single transfer
				else if (CC_TS)
					fatalerror("%s('%s'): terminate on single transfer not supported\n", shortname(), tag());

				// not terminated, continue transfer
				else
					// do we need to read another byte?
					if (BIT(m_r[PSW].w, 1) && !BIT(m_r[PSW].w, 0))
						if (CC_SYNC == 0x02)
							m_dma_state = DMA_WAIT_FOR_DEST_DRQ;
						else
							m_dma_state = DMA_STORE_BYTE_HIGH;

					// transfer done
					else
						m_dma_state = DMA_IDLE;

				break;

			case DMA_STORE_BYTE_HIGH:
				if (VERBOSE_DMA)
					logerror("%s('%s'): entering state: DMA_STORE_BYTE_HIGH[ %02x ]\n", shortname(), tag(), (m_dma_value >> 8) & 0xff);

				m_iop->write_byte(m_r[GB - CC_SOURCE].w, (m_dma_value >> 8) & 0xff);
				m_r[GB - CC_SOURCE].w++;
				m_dma_state = DMA_TERMINATE;

				break;
			}

			m_icount--;
		}

		// executing task block instructions?
		else if (BIT(m_r[PSW].w, 2))
		{
			// dma transfer pending?
			if (m_xfer_pending)
				m_r[PSW].w |= 1 << 6;

			// fetch first two instruction bytes
			UINT16 op = m_iop->read_word(m_r[TP].w);
			m_r[TP].w += 2;

			// extract parameters
			UINT8 params = op & 0xff;
			UINT8 opcode = (op >> 8) & 0xff;

			int brp = (params >> 5) & 0x07;
			int wb  = (params >> 3) & 0x03;
			int aa  = (params >> 1) & 0x03;
			int w   = (params >> 0) & 0x01;
			int opc = (opcode >> 2) & 0x3f;
			int mm  = (opcode >> 0) & 0x03;

			// fix-up so we can use our register array
			if (mm == BC) mm = PP;

			UINT8 o;
			UINT16 off, seg;

			logerror("%s('%s'): executing %x %x %x %x %02x %x\n", shortname(), tag(), brp, wb, aa, w, opc, mm);

			switch (opc)
			{
			case 0x00: // control
				switch (brp)
				{
				case 0: nop(); break;
				case 1: invalid(opc); break;
				case 2: sintr(); break;
				case 3: xfer(); break;
				default: wid(BIT(brp, 1), BIT(brp, 0));
				}
				break;

			case 0x02: // lpdi
				off = imm16();
				seg = imm16();
				lpdi(brp, seg, off);
				break;

			case 0x08: // add(b)i r, i
				if (w) addi_ri(brp, imm16());
				else   addbi_ri(brp, imm8());
				break;

			case 0x0a: // and(b)i r, i
				if (w) andi_ri(brp, imm16());
				else   andbi_ri(brp, imm8());
				break;

			case 0x0c: // mov(b)i r, i
				if (w) movi_ri(brp, imm16());
				else   movbi_ri(brp, imm8());
				break;

			case 0x0f: // dec r
				dec_r(brp);
				break;

			case 0x12: // hlt
				if (BIT(brp, 0))
					hlt();
				break;

			case 0x22: // lpd
				o = offset(aa);
				lpd(brp, mm, o);
				break;

			case 0x28: // add(b) r, m
				if (w) add_rm(brp, mm, offset(aa));
				else   addb_rm(brp, mm, offset(aa));
				break;

			case 0x2a: // and(b) r, m
				if (w) and_rm(brp, mm, offset(aa));
				else   andb_rm(brp, mm, offset(aa));
				break;

			case 0x27: // call
				o = offset(aa);
				call(mm, displacement(wb), o);
				break;

			case 0x30: // add(b)i m, i
				o = offset(aa);
				if (w) addi_mi(mm, imm16(), o);
				else   addbi_mi(mm, imm8(), o);
				break;

			case 0x32: // and(b)i m, i
				o = offset(aa);
				if (w) andi_mi(mm, imm16());
				else   andbi_mi(mm, imm8());
				break;

			case 0x34: // add(b) m, r
				if (w) add_mr(mm, brp, offset(aa));
				else   addb_mr(mm, brp, offset(aa));
				break;

			case 0x36: // and(b) m, r
				if (w) and_mr(mm, brp, offset(aa));
				else   andb_mr(mm, brp, offset(aa));
				break;

			case 0x3b: // dec(b) m
				if (w) dec_m(mm, offset(aa));
				else   decb(mm, offset(aa));
				break;

			case 0x3e: // clr
				clr(mm, brp, offset(aa));
				break;

			default:
				invalid(opc);
			}

			m_icount--;
		}

		// nothing to do
		else
		{
			m_icount--;
		}
	}
	while (m_icount > 0);
}

void i8089_channel::set_control_block(offs_t address)
{
	m_r[CP].w = address;
}

bool i8089_channel::bus_load_limit()
{
	return BIT(m_r[PSW].w, 5);
}

bool i8089_channel::priority()
{
	return BIT(m_r[PSW].w, 7);
}

void i8089_channel::examine_ccw(UINT8 ccw)
{
	// priority and bus load limit, bit 7 and 5
	m_r[PSW].w = (m_r[PSW].w & 0x5f) | (ccw & 0xa0);

	// acknowledge interrupt
	if (BIT(ccw, 4))
	{
		m_write_sintr(0);
		m_r[PSW].w &= ~(1 << 5);
	}

	// interrupt enable
	if (BIT(ccw, 5))
	{
		if (BIT(ccw, 4))
			m_r[PSW].w &= ~(1 << 4);
		else
			m_r[PSW].w |= 1 << 4;
	}
}

void i8089_channel::attention()
{
	// examine control byte
	UINT8 ccw = m_iop->read_byte(m_r[CP].w);

	switch (ccw & 0x07)
	{
	// no channel command
	case 0:
		if (VERBOSE)
			logerror("%s('%s'): command received: update psw\n", shortname(), tag());

		examine_ccw(ccw);
		break;

	// start channel, tb in local space
	case 1:
		if (VERBOSE)
			logerror("%s('%s'): command received: start channel in local space\n", shortname(), tag());

		examine_ccw(ccw);

		lpd(PP, CP, 2);
		movp_pm(TP, PP);
		movbi_mi(CP, 0xff, 1);

		m_r[PSW].w |= 1 << 2;

		break;

	// reserved
	case 2:
		if (VERBOSE)
			logerror("%s('%s'): command received: invalid command 010\n", shortname(), tag());

		break;

	// start channel, tb in system space
	case 3:
		if (VERBOSE)
			logerror("%s('%s'): command received: start channel in system space\n", shortname(), tag());

		examine_ccw(ccw);

		lpd(PP, CP, 2);
		lpd(TP, PP);
		movbi_mi(CP, 0xff, 1);

		m_r[PSW].w |= 1 << 2;

		if (VERBOSE)
		{
			logerror("%s('%s'): ---- starting channel ----\n", shortname(), tag());
			logerror("%s('%s'): parameter block address: %06x\n", shortname(), tag(), m_r[PP].w);
			logerror("%s('%s'): task pointer: %06x\n", shortname(), tag(), m_r[TP].w);
		}

		break;

	case 4:
		if (VERBOSE)
			logerror("%s('%s'): command received: invalid command 100\n", shortname(), tag());

		break;

	// continue channel processing
	case 5:
		if (VERBOSE)
			logerror("%s('%s'): command received: continue channel processing\n", shortname(), tag());

		// restore task pointer and parameter block
		movp_pm(TP, PP);
		movb_rm(PSW, PP, 3);
		movbi_mi(CP, 0xff, 1);

		m_r[PSW].w |= 1 << 2;

		if (VERBOSE)
		{
			logerror("%s('%s'): ---- continuing channel ----\n", shortname(), tag());
			logerror("%s('%s'): task pointer: %06x\n", shortname(), tag(), m_r[TP].w);
		}

		break;

	// halt channel, save tp
	case 6:
		if (VERBOSE)
			logerror("%s('%s'): command received: halt channel and save tp\n", shortname(), tag());

		// save task pointer and psw to parameter block
		movp_mp(PP, TP);
		movb_mr(PP, PSW, 3);
		hlt();

		break;

	// halt channel, don't save tp
	case 7:
		if (VERBOSE)
			logerror("%s('%s'): command received: halt channel\n", shortname(), tag());

		hlt();

		break;
	}
}

WRITE_LINE_MEMBER( i8089_channel::ext_w )
{
	if (VERBOSE)
		logerror("%s('%s'): ext_w: %d\n", shortname(), tag(), state);
}

WRITE_LINE_MEMBER( i8089_channel::drq_w )
{
	if (VERBOSE)
		logerror("%s('%s'): ext_w: %d\n", shortname(), tag(), state);
}


//**************************************************************************
//  OPCODES
//**************************************************************************

#define UNIMPLEMENTED logerror("%s('%s'): unimplemented opcode: %s\n", shortname(), tag(), __FUNCTION__);

void i8089_channel::add_rm(int r, int m, int o) { UNIMPLEMENTED }
void i8089_channel::add_mr(int m, int r, int o) { UNIMPLEMENTED }
void i8089_channel::addb_rm(int r, int m, int o) { UNIMPLEMENTED }
void i8089_channel::addb_mr(int m, int r, int o) { UNIMPLEMENTED }
void i8089_channel::addbi_ri(int r, int i) { UNIMPLEMENTED }
void i8089_channel::addbi_mi(int m, int i, int o) { UNIMPLEMENTED }
void i8089_channel::addi_ri(int r, int i) { UNIMPLEMENTED }
void i8089_channel::addi_mi(int m, int i, int o) { UNIMPLEMENTED }
void i8089_channel::and_rm(int r, int m, int o) { UNIMPLEMENTED }
void i8089_channel::and_mr(int m, int r, int o) { UNIMPLEMENTED }
void i8089_channel::andb_rm(int r, int m, int o) { UNIMPLEMENTED }
void i8089_channel::andb_mr(int m, int r, int o) { UNIMPLEMENTED }
void i8089_channel::andbi_ri(int r, int i) { UNIMPLEMENTED }
void i8089_channel::andbi_mi(int m, int i, int o) { UNIMPLEMENTED }
void i8089_channel::andi_ri(int r, int i) { UNIMPLEMENTED }
void i8089_channel::andi_mi(int m, int i, int o) { UNIMPLEMENTED }
void i8089_channel::call(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::clr(int m, int b, int o) { UNIMPLEMENTED }
void i8089_channel::dec_r(int r) { UNIMPLEMENTED }
void i8089_channel::dec_m(int m, int o) { UNIMPLEMENTED }
void i8089_channel::decb(int m, int o) { UNIMPLEMENTED }

// halt
void i8089_channel::hlt()
{
	movbi_mi(CP, 0x00, 1);
	m_r[PSW].w &= ~(1 << 2);
}

void i8089_channel::inc_r(int r) { UNIMPLEMENTED }
void i8089_channel::inc_m(int m, int o) { UNIMPLEMENTED }
void i8089_channel::incb(int m, int o) { UNIMPLEMENTED }
void i8089_channel::jbt(int m, int b, int d, int o) { UNIMPLEMENTED }
void i8089_channel::jmce(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::jmcne(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::jmp(int d) { UNIMPLEMENTED }
void i8089_channel::jnbt(int m, int b, int d, int o) { UNIMPLEMENTED }
void i8089_channel::jnz_r(int r, int d) { UNIMPLEMENTED }
void i8089_channel::jnz_m(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::jnzb(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::jz_r(int r, int d) { UNIMPLEMENTED }
void i8089_channel::jz_m(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::jzb(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::lcall(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::ljbt(int m, int b, int d, int o) { UNIMPLEMENTED }
void i8089_channel::ljmce(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::ljmcne(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::ljmp(int d) { UNIMPLEMENTED }
void i8089_channel::ljnbt(int m, int b, int d, int o) { UNIMPLEMENTED }
void i8089_channel::ljnz_r(int r, int d) { UNIMPLEMENTED }
void i8089_channel::ljnz_m(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::ljnzb(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::ljz_r(int r, int d) { UNIMPLEMENTED }
void i8089_channel::ljz_m(int m, int d, int o) { UNIMPLEMENTED }
void i8089_channel::ljzb(int m, int d, int o) { UNIMPLEMENTED }

// load pointer from memory
void i8089_channel::lpd(int p, int m, int o)
{
	UINT16 offset = m_iop->read_word(m_r[m].w + o);
	UINT16 segment = m_iop->read_word(m_r[m].w + o + 2);

	m_r[p].w = ((segment << 4) + offset) & 0xfffff;
	m_r[p].t = 0;
}

// load pointer from immediate data
void i8089_channel::lpdi(int p, int i, int o)
{
	m_r[p].w = (o << 4) + (i & 0xffff);
	m_r[p].t = 0;
}

void i8089_channel::mov_mr(int m, int r, int o) { UNIMPLEMENTED }
void i8089_channel::mov_rm(int r, int m, int o) { UNIMPLEMENTED }
void i8089_channel::mov_mm(int m1, int m2, int o1, int o2) { UNIMPLEMENTED }

// move register to memory byte
void i8089_channel::movb_mr(int m, int r, int o)
{
	m_iop->write_byte(m_r[m].w + o, m_r[r].w & 0xff);
}

// move memory byte to register
void i8089_channel::movb_rm(int r, int m, int o)
{
	UINT8 byte = m_iop->read_byte(m_r[m].w + o);
	m_r[r].w = (BIT(byte, 7) ? 0xfff00 : 0x00000) | byte;
	m_r[r].t = 1;
}

// move memory byte to memory byte
void i8089_channel::movb_mm(int m1, int m2, int o1, int o2)
{
	UINT8 byte = m_iop->read_byte(m_r[m2].w + o2);
	m_iop->write_byte(m_r[m1].w + o1, byte);
}

// move immediate byte to register
void i8089_channel::movbi_ri(int r, int i)
{
	m_r[r].w = (BIT(i, 7) ? 0xfff00 : 0x00000) | (i & 0xff);
	m_r[r].t = 1;
}

// move immediate byte to memory byte
void i8089_channel::movbi_mi(int m, int i, int o)
{
	m_iop->write_byte(m_r[m].w + o, i & 0xff);
}

// move immediate word to register
void i8089_channel::movi_ri(int r, int i)
{
	m_r[r].w = (BIT(i, 15) ? 0xf0000 : 0x00000) | (i & 0xffff);
	m_r[r].t = 1;
}

// move immediate word to memory word
void i8089_channel::movi_mi(int m, int i, int o)
{
	m_iop->write_word(m_r[m].w + o, (BIT(i, 15) ? 0xf0000 : 0x00000) | (i & 0xffff));
}

// move pointer to memory (store)
void i8089_channel::movp_mp(int m, int p, int o)
{
	m_iop->write_word(m_r[m].w + o, m_r[p].w & 0xffff);
	m_iop->write_word(m_r[m].w + o + 2, (m_r[p].w >> 12 & 0xf0) | (m_r[p].t << 3 & 0x01));
}

// move memory to pointer (restore)
void i8089_channel::movp_pm(int p, int m, int o)
{
	UINT16 offset = m_iop->read_word(m_r[m].w + o);
	UINT16 segment = m_iop->read_word(m_r[m].w + o + 2);

	m_r[p].w = ((segment << 4) + offset) & 0xfffff;
	m_r[p].t = segment >> 3 & 0x01;
}

// no operation
void i8089_channel::nop()
{
}

void i8089_channel::not_r(int r) { UNIMPLEMENTED }
void i8089_channel::not_m(int m, int o) { UNIMPLEMENTED }
void i8089_channel::not_rm(int r, int m, int o) { UNIMPLEMENTED }
void i8089_channel::notb_m(int m, int o) { UNIMPLEMENTED }
void i8089_channel::notb_rm(int r, int m, int o) { UNIMPLEMENTED }
void i8089_channel::or_rm(int r, int m, int o) { UNIMPLEMENTED }
void i8089_channel::or_mr(int m, int r, int o) { UNIMPLEMENTED }
void i8089_channel::orb_rm(int r, int m, int o) { UNIMPLEMENTED }
void i8089_channel::orb_mr(int m, int r, int o) { UNIMPLEMENTED }
void i8089_channel::orbi_ri(int r, int i) { UNIMPLEMENTED }
void i8089_channel::orbi_mi(int m, int i, int o) { UNIMPLEMENTED }
void i8089_channel::ori_ri(int r, int i) { UNIMPLEMENTED }
void i8089_channel::ori_mi(int m, int i, int o) { UNIMPLEMENTED }
void i8089_channel::setb(int m, int b, int o) { UNIMPLEMENTED }

// set interrupt service flip-flop
void i8089_channel::sintr()
{
	if (BIT(m_r[PSW].w, 4))
	{
		m_r[PSW].w |= 1 << 5;
		m_write_sintr(1);
	}
}

void i8089_channel::tsl(int m, int i, int d, int o) { UNIMPLEMENTED }

// set source and destination logical widths
void i8089_channel::wid(int s, int d)
{
	m_r[PSW].w &= 0x3f;
	m_r[PSW].w |= d << 0;
	m_r[PSW].w |= s << 1;
}

// enter dma transfer mode after next instruction
void i8089_channel::xfer()
{
	m_xfer_pending = true;
}

void i8089_channel::invalid(int opc)
{
	logerror("%s('%s'): invalid opcode: %02x\n", shortname(), tag(), opc);
}


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type I8089 = &device_creator<i8089_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i8089_device - constructor
//-------------------------------------------------

i8089_device::i8089_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, I8089, "Intel 8089", tag, owner, clock, "i8089", __FILE__),
	m_ch1(*this, "1"),
	m_ch2(*this, "2"),
	m_write_sintr1(*this),
	m_write_sintr2(*this),
	m_16bit_system(false),
	m_16bit_remote(false),
	m_master(false),
	m_request_grant(false),
	m_ca(0),
	m_sel(0)
{
}

void i8089_device::static_set_cputag(device_t &device, const char *tag)
{
	i8089_device &i8089 = downcast<i8089_device &>(device);
	i8089.m_cputag = tag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8089_device::device_start()
{
	// make sure our channels have been setup first
	if (!m_ch1->started() || !m_ch2->started())
		throw device_missing_dependencies();

	// resolve callbacks
	m_write_sintr1.resolve_safe();
	m_write_sintr2.resolve_safe();

	// register for save states
	save_item(NAME(m_16bit_system));
	save_item(NAME(m_16bit_remote));
	save_item(NAME(m_master));
	save_item(NAME(m_request_grant));
	save_item(NAME(m_ca));
	save_item(NAME(m_sel));

	// get pointers to memory
	device_t *cpu = machine().device(m_cputag);
	m_mem = &cpu->memory().space(AS_PROGRAM);
	m_io = &cpu->memory().space(AS_IO);

	// initialize channel clock
	m_ch1->set_unscaled_clock(clock());
	m_ch2->set_unscaled_clock(clock());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8089_device::device_reset()
{
	m_initialized = false;
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( i8089 )
	MCFG_I8089_CHANNEL_ADD("1")
	MCFG_I8089_CHANNEL_SINTR(WRITELINE(i8089_device, ch1_sintr_w))
	MCFG_I8089_CHANNEL_ADD("2")
	MCFG_I8089_CHANNEL_SINTR(WRITELINE(i8089_device, ch2_sintr_w))
MACHINE_CONFIG_END

machine_config_constructor i8089_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( i8089 );
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

// the i8089 actually executes a program from internal rom here:
//
// MOVB SYSBUS from FFFF6
// LPD System Configuration Block from FFFF8
// MOVB SOC from (SCB)
// LPD Control Pointer (CP) from (SCB) + 2
// MOVBI "00" to CP + 1 (clear busy flag)

void i8089_device::initialize()
{
	assert(!m_initialized);

	// get system bus width
	UINT8 sys_bus = m_mem->read_byte(0xffff6);
	m_16bit_system = BIT(sys_bus, 0);

	// get system configuration block address
	UINT16 scb_offset = read_word(0xffff8);
	UINT16 scb_segment = read_word(0xffffa);
	offs_t scb_address = ((scb_segment << 4) + scb_offset) & 0x0fffff;

	// get system operation command
	UINT16 soc = read_word(scb_address);
	m_16bit_remote = BIT(soc, 0);
	m_request_grant = BIT(soc, 1);
	m_master = !m_sel;

	// get control block address
	UINT16 cb_offset = read_word(scb_address + 2);
	UINT16 cb_segment = read_word(scb_address + 4);
	offs_t cb_address = ((cb_segment << 4) + cb_offset) & 0x0fffff;

	// initialize channels
	m_ch1->set_control_block(cb_address);
	m_ch2->set_control_block(cb_address + 8);

	// clear busy
	UINT16 ccw = read_word(cb_address);
	write_word(cb_address, ccw & 0x00ff);

	// done
	m_initialized = true;

	// output some debug info
	if (VERBOSE)
	{
		logerror("%s('%s'): ---- initializing ----\n", shortname(), basetag());
		logerror("%s('%s'): %s system bus\n", shortname(), basetag(), m_16bit_system ? "16-bit" : "8-bit");
		logerror("%s('%s'): system configuration block location: %06x\n", shortname(), basetag(), scb_address);
		logerror("%s('%s'): %s remote bus\n", shortname(), basetag(), m_16bit_remote ? "16-bit" : "8-bit");
		logerror("%s('%s'): request/grant: %d\n", shortname(), basetag(), m_request_grant);
		logerror("%s('%s'): is %s\n", shortname(), basetag(), m_master ? "master" : "slave");
		logerror("%s('%s'): channel control block location: %06x\n", shortname(), basetag(), cb_address);
	}
}

UINT8 i8089_device::read_byte(offs_t address)
{
	assert(m_initialized);
	return m_mem->read_byte(address);
}

UINT16 i8089_device::read_word(offs_t address)
{
	assert(m_initialized);

	UINT16 data = 0xffff;

	if (m_16bit_system && !(address & 1))
	{
		data = m_mem->read_word(address);
	}
	else
	{
		data  = m_mem->read_byte(address);
		data |= m_mem->read_byte(address + 1) << 8;
	}

	return data;
}

void i8089_device::write_byte(offs_t address, UINT8 data)
{
	assert(m_initialized);
	m_mem->write_byte(address, data);
}

void i8089_device::write_word(offs_t address, UINT16 data)
{
	assert(m_initialized);

	if (m_16bit_system && !(address & 1))
	{
		m_mem->write_word(address, data);
	}
	else
	{
		m_mem->write_byte(address, data & 0xff);
		m_mem->write_byte(address + 1, (data >> 8) & 0xff);
	}
}


//**************************************************************************
//  EXTERNAL INPUTS
//**************************************************************************

WRITE_LINE_MEMBER( i8089_device::ca_w )
{
	if (VERBOSE)
		logerror("%s('%s'): ca_w: %u\n", shortname(), basetag(), state);

	if (m_ca == 1 && state == 0)
	{
		if (!m_initialized)
			initialize();
		else
		{
			if (m_sel == 0)
				m_ch1->attention();
			else
				m_ch2->attention();
		}
	}

	m_ca = state;
}

WRITE_LINE_MEMBER( i8089_device::drq1_w ) { m_ch1->drq_w(state); }
WRITE_LINE_MEMBER( i8089_device::drq2_w ) { m_ch2->drq_w(state); }
WRITE_LINE_MEMBER( i8089_device::ext1_w ) { m_ch1->ext_w(state); }
WRITE_LINE_MEMBER( i8089_device::ext2_w ) { m_ch2->ext_w(state); }
