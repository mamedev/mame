/***************************************************************************

    NEC uPD4990A

    Serial I/O Calendar & Clock IC

***************************************************************************/

#ifndef __PD4990A_OLD_H__
#define __PD4990A_OLD_H__



/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class upd4990a_old_device : public device_t
{
public:
	upd4990a_old_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~upd4990a_old_device() {}

	/* this should be refactored, once RTCs get unified */
	void addretrace();

	DECLARE_READ8_MEMBER( testbit_r );
	DECLARE_READ8_MEMBER( databit_r );
	DECLARE_WRITE16_MEMBER( control_16_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	int m_seconds;    /* seconds BCD */
	int m_minutes;    /* minutes BCD */
	int m_hours;      /* hours   BCD */
	int m_days;       /* days    BCD */
	int m_month;      /* month   Hexadecimal form */
	int m_year;       /* year    BCD */
	int m_weekday;    /* weekday BCD */

	UINT32 m_shiftlo;
	UINT32 m_shifthi;

	int m_retraces;   /* Assumes 60 retraces a second */
	int m_testwaits;
	int m_maxwaits;   /* Switch test every frame*/
	int m_testbit;    /* Pulses a bit in order to simulate test output */

	int m_outputbit;
	int m_bitno;
	INT8 m_reading;
	INT8 m_writing;

	int m_clock_line;
	int m_command_line;   //??

	void increment_month();
	void increment_day();
	void readbit();
	void resetbitstream();
	void writebit( UINT8 bit );
	void nextbit();
	UINT8 getcommand();
	void update_date();
	void process_command();
	void serial_control( UINT8 data );
};

extern ATTR_DEPRECATED const device_type UPD4990A_OLD;


#define MCFG_UPD4990A_OLD_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, UPD4990A_OLD, 0)

#endif  /*__PD4990A_H__*/
