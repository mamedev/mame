
#define m68307BUS_MBSR (0x06)

READ16_HANDLER( m68307_internal_mbus_r );
WRITE16_HANDLER( m68307_internal_mbus_w );

class m68307_mbus
{
	public:
	void reset(void);
};

