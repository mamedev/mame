

#include "cpu/drcfe.h"
#include "cpu/drcuml.h"

/* size of the execution code cache */
#define CACHE_SIZE                  (32 * 1024 * 1024)

class sh_common_execution
{
public:
	sh_common_execution() :
		m_sh2_state(nullptr)
		, m_cache(CACHE_SIZE + sizeof(internal_sh2_state))
	{ }

	// Data that needs to be stored close to the generated DRC code
	struct internal_sh2_state
	{
		uint32_t  pc;
		uint32_t  pr;
		uint32_t  sr;
		uint32_t  mach;
		uint32_t  macl;
		uint32_t  r[16];
		uint32_t  ea;

		uint32_t  pending_irq;
		uint32_t  pending_nmi;
		int32_t   irqline;
		uint32_t  evec;               // exception vector for DRC
		uint32_t  irqsr;              // IRQ-time old SR for DRC
		uint32_t  target;             // target for jmp/jsr/etc so the delay slot can't kill it
		int     internal_irq_level;
		int     icount;
		uint8_t   sleep_mode;
		uint32_t  arg0;              /* print_debug argument 1 */
		// SH1/2 only?
		uint32_t  gbr;
		uint32_t  vbr;
	};

	internal_sh2_state *m_sh2_state;

protected:
	void ADD(uint32_t m, uint32_t n);
	drc_cache           m_cache;                  /* pointer to the DRC code cache */
};