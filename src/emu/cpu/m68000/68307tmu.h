
READ16_HANDLER( m68307_internal_timer_r );
WRITE16_HANDLER( m68307_internal_timer_w );

#define m68307TIMER_TMR (0x0)
#define m68307TIMER_TRR (0x1)
#define m68307TIMER_TCR (0x2)
#define m68307TIMER_TCN (0x3)
#define m68307TIMER_TER (0x4)
#define m68307TIMER_WRR (0x5)
#define m68307TIMER_WCR (0x6)
#define m68307TIMER_XXX (0x7)

struct m68307_single_timer
{
	UINT16 regs[0x8];
	bool enabled;
};

class m68307_timer
{
	public:
	m68307_single_timer timer[2];
	void reset(void);
};

