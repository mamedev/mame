
DECLARE_READ32_HANDLER( m68340_internal_timer_r );
DECLARE_WRITE32_HANDLER( m68340_internal_timer_w );

class m68340_timer
{
	public:
	void reset(void);
};

