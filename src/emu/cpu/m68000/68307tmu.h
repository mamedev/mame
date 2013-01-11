
DECLARE_READ16_HANDLER( m68307_internal_timer_r );
DECLARE_WRITE16_HANDLER( m68307_internal_timer_w );

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
	emu_timer *mametimer;
};


class m68307_timer
{
	public:
	m68307_single_timer singletimer[2];

	emu_timer *wd_mametimer;
	legacy_cpu_device *parent;

	void write_tmr(UINT16 data, UINT16 mem_mask, int which);
	void write_trr(UINT16 data, UINT16 mem_mask, int which);
	void write_ter(UINT16 data, UINT16 mem_mask, int which);
	UINT16 read_tcn(UINT16 mem_mask, int which);

	void init(legacy_cpu_device *device);
	void reset(void);
};
