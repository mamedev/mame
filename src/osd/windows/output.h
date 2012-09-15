//============================================================
//
//  output.h - Win32 implementation of MAME output routines
//
//============================================================
//
//  Copyright Aaron Giles
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the
//  following conditions are met:
//
//    * Redistributions of source code must retain the above
//      copyright notice, this list of conditions and the
//      following disclaimer.
//    * Redistributions in binary form must reproduce the
//      above copyright notice, this list of conditions and
//      the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//    * Neither the name 'MAME' nor the names of its
//      contributors may be used to endorse or promote
//      products derived from this software without specific
//      prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
//  EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGE (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
//  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

struct copydata_id_string
{
	UINT32		id;					// ID that was requested
	char		string[1];			// string array containing the data
};



//============================================================
//  FUNCTION PROTOTYPES
//============================================================

void winoutput_init(running_machine &machine);


#endif /* __WINDOWS_OUTPUT_H__ */
