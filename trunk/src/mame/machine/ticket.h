/***************************************************************************

    Function prototypes and constants for the ticket dispenser emulator

***************************************************************************/

#include "devlegcy.h"


#define TICKET_MOTOR_ACTIVE_LOW    0    /* Ticket motor is triggered by D7=0 */
#define TICKET_MOTOR_ACTIVE_HIGH   1    /* Ticket motor is triggered by D7=1 */

#define TICKET_STATUS_ACTIVE_LOW   0    /* Ticket is done dispensing when D7=0 */
#define TICKET_STATUS_ACTIVE_HIGH  1    /* Ticket is done dispensing when D7=1 */



typedef struct _ticket_config ticket_config;
struct _ticket_config
{
	UINT8	motorhigh;
	UINT8	statushigh;
};


#define MCFG_TICKET_DISPENSER_ADD(_tag, _clock, _motorhigh, _statushigh) \
	MCFG_DEVICE_ADD(_tag, TICKET_DISPENSER, _clock) \
	MCFG_DEVICE_CONFIG_DATA32(ticket_config, motorhigh, _motorhigh) \
	MCFG_DEVICE_CONFIG_DATA32(ticket_config, statushigh, _statushigh)


READ8_DEVICE_HANDLER( ticket_dispenser_r );
WRITE8_DEVICE_HANDLER( ticket_dispenser_w );

READ_LINE_DEVICE_HANDLER( ticket_dispenser_line_r );

DECLARE_LEGACY_DEVICE(TICKET_DISPENSER, ticket);
