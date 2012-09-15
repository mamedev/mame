/*
    990_tap.h: include file for 990_tap.c
*/
extern READ16_DEVICE_HANDLER(ti990_tpc_r);
extern WRITE16_DEVICE_HANDLER(ti990_tpc_w);

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct ti990_tpc_interface
{
	void (*interrupt_callback)(running_machine &machine, int state);
};
/***************************************************************************
    MACROS
***************************************************************************/

class tap_990_device : public device_t
{
public:
	tap_990_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tap_990_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual machine_config_constructor device_mconfig_additions() const;
private:
	// internal state
	void *m_token;
};

extern const device_type TI990_TAPE_CTRL;


#define MCFG_TI990_TAPE_CTRL_ADD(_tag, _intrf)	\
	MCFG_DEVICE_ADD((_tag),  TI990_TAPE_CTRL, 0)\
	MCFG_DEVICE_CONFIG(_intrf)

