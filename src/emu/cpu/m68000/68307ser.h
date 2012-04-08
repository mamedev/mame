
#define m68307SER_USR_UCSR (0x02)
#define m68307SER_URB_UTB (0x06)

#define m68307SER_UISR_UIMR (0x0a)

#define m68307SER_UIVR (0x18)


READ16_HANDLER( m68307_internal_serial_r );
WRITE16_HANDLER( m68307_internal_serial_w );

class m68307_serial
{
	public:
	void reset(void);
	UINT16 m_uivr;

};

