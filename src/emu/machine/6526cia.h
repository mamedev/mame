/**********************************************************************

    MOS 6526/8520 CIA interface and emulation

    This function emulates all the functionality of up to 2 MOS6526 or
    MOS8520 complex interface adapters.

**********************************************************************/

#ifndef __6526CIA_H__
#define __6526CIA_H__


/***************************************************************************
    MACROS
***************************************************************************/

#define CIA6526			DEVICE_GET_INFO_NAME(cia6526)
#define CIA8520			DEVICE_GET_INFO_NAME(cia8520)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _cia6526_interface cia6526_interface;
struct _cia6526_interface
{
	void (*irq_func)(const device_config *device, int state);
	int clock;
	int tod_clock;

	struct
	{
		UINT8	(*read)(void);
		void	(*write)(UINT8);
	} port[2];
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO(cia6526);
DEVICE_GET_INFO(cia8520);

/* configuration */
void cia_set_port_mask_value(const device_config *device, int port, int data);

/* reading and writing */
READ8_DEVICE_HANDLER( cia_r );
WRITE8_DEVICE_HANDLER( cia_w );
void cia_clock_tod(const device_config *device);
void cia_issue_index(const device_config *device);
void cia_set_input_cnt(const device_config *device, int data);
void cia_set_input_sp(const device_config *device, int data);

/* accessors */
UINT8 cia_get_output_a(const device_config *device);
UINT8 cia_get_output_b(const device_config *device);
int cia_get_irq(const device_config *device);

#endif /* __6526CIA_H__ */
