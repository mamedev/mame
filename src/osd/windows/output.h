//============================================================
//
//  output.h - Win32 implementation of MAME output routines
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#ifndef __WINDOWS_OUTPUT_H__
#define __WINDOWS_OUTPUT_H__


//============================================================
//  CONSTANTS
//============================================================

// window parameters
#define OUTPUT_WINDOW_CLASS			TEXT("MAMEOutput")
#define OUTPUT_WINDOW_NAME			TEXT("MAMEOutput")

//
// These messages are sent by MAME:
//

// OM_MAME_START: broadcast when MAME initializes
//      WPARAM = HWND of MAME's output window
//      LPARAM = unused
#define OM_MAME_START				TEXT("MAMEOutputStart")

// OM_MAME_STOP: broadcast when MAME shuts down
//      WPARAM = HWND of MAME's output window
//      LPARAM = unused
#define OM_MAME_STOP				TEXT("MAMEOutputStop")

// OM_MAME_UPDATE_STATE: sent to registered clients when the state
// of an output changes
//      WPARAM = ID of the output
//      LPARAM = new value for the output
#define OM_MAME_UPDATE_STATE		TEXT("MAMEOutputUpdateState")


//
// These messages are sent by external clients to MAME:
//

// OM_MAME_REGISTER_CLIENT: sent to MAME to register a client
//      WPARAM = HWND of client's listener window
//      LPARAM = client-specified ID (must be unique)
#define OM_MAME_REGISTER_CLIENT		TEXT("MAMEOutputRegister")

// OM_MAME_UNREGISTER_CLIENT: sent to MAME to unregister a client
//      WPARAM = HWND of client's listener window
//      LPARAM = client-specified ID (must match registration)
#define OM_MAME_UNREGISTER_CLIENT	TEXT("MAMEOutputUnregister")

// OM_MAME_GET_ID_STRING: requests the string associated with a
// given ID. ID=0 is always the name of the game. Other IDs are
// only discoverable from a OM_MAME_UPDATE_STATE message. The
// result will be sent back as a WM_COPYDATA message with MAME's
// output window as the sender, dwData = the ID of the string,
// and lpData pointing to a NULL-terminated string.
//      WPARAM = HWND of client's listener window
//      LPARAM = ID you wish to know about
#define OM_MAME_GET_ID_STRING		TEXT("MAMEOutputGetIDString")


//
// These constants are used to identify WM_COPYDATA messages
// coming from MAME:
//

#define COPYDATA_MESSAGE_ID_STRING	1



//============================================================
//  TYPE DEFINITIONS
//============================================================

typedef struct _copydata_id_string copydata_id_string;
struct _copydata_id_string
{
	UINT32		id;					// ID that was requested
	char		string[1];			// string array containing the data
};



//============================================================
//  FUNCTION PROTOTYPES
//============================================================

void winoutput_init(running_machine *machine);


#endif /* __WINDOWS_OUTPUT_H__ */
