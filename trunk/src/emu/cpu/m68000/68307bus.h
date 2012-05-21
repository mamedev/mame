#define m68307BUS_MADR (0x01)
#define m68307BUS_MFDR (0x03)
#define m68307BUS_MBCR (0x05)
#define m68307BUS_MBSR (0x07)
#define m68307BUS_MBDR (0x09)

READ8_HANDLER( m68307_internal_mbus_r );
WRITE8_HANDLER( m68307_internal_mbus_w );

class m68307_mbus
{
	public:

	UINT16 m_MFCR;

	bool m_busy;
	bool m_intpend;

	void reset(void);
};

