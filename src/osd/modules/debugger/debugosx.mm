// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  debugosx.m - MacOS X Cocoa debug window handling
//
//============================================================


// TODO:
//  * Automatic scrolling for console and log views
//  * Keyboard shortcuts in error log and device windows
//  * Don't accept keyboard input while the game is running
//  * Interior focus rings - standard/exterior focus rings look really ugly here
//  * Updates causing debug views' widths to change are sometimes obscured by the scroll views' opaque backgrounds
//  * Scroll views with content narrower than clipping area are flaky under Tiger - nothing I can do about this


// MAME headers
#include "emu.h"
#include "debugger.h"

// MAMEOS headers
#include "modules/lib/osdobj_common.h"
#include "osx/debugosx.h"
#include "osdsdl.h"
#include "debug_module.h"

#import "osx/debugconsole.h"
#import "osx/debugwindowhandler.h"

#include <atomic>


//============================================================
//  MODULE SUPPORT
//============================================================

class debugger_osx : public osd_module, public debug_module
{
public:
	debugger_osx()
	: osd_module(OSD_DEBUG_PROVIDER, "osx"), debug_module(),
	  m_machine(NULL),
	  m_console(nil)
	{
	}

	virtual ~debugger_osx()
	{
		if (m_console != nil)
			[m_console release];
	}

	virtual int init(const osd_options &options);
	virtual void exit();

	virtual void init_debugger(running_machine &machine);
	virtual void wait_for_debugger(device_t &device, bool firststop);
	virtual void debugger_update();

private:
	running_machine *m_machine;
	MAMEDebugConsole *m_console;

	static std::atomic_bool s_added_menus;
};

MODULE_DEFINITION(DEBUG_OSX, debugger_osx)

std::atomic_bool debugger_osx::s_added_menus(false);

//============================================================
//  debugger_osx::init
//============================================================

int debugger_osx::init(const osd_options &options)
{
	return 0;
}

//============================================================
//  debugger_osx::exit
//============================================================

void debugger_osx::exit()
{
	NSAutoreleasePool *const pool = [[NSAutoreleasePool alloc] init];
	if (m_console)
	{
		NSDictionary *info = [NSDictionary dictionaryWithObject:[NSValue valueWithPointer:m_machine]
														forKey:@"MAMEDebugMachine"];
		[[NSNotificationCenter defaultCenter] postNotificationName:MAMEHideDebuggerNotification
															object:m_console
														  userInfo:info];
		[m_console release];
		m_console = nil;
		m_machine = NULL;
	}
	[pool release];
}

//============================================================
//  debugger_osx::init_debugger
//============================================================

void debugger_osx::init_debugger(running_machine &machine)
{
	m_machine = &machine;
}

//============================================================
//  debugger_osx::wait_for_debugger
//============================================================

void debugger_osx::wait_for_debugger(device_t &device, bool firststop)
{
	NSAutoreleasePool *const pool = [[NSAutoreleasePool alloc] init];

	// create a console window
	if (m_console == nil)
	{
		if (!s_added_menus.exchange(true, std::memory_order_relaxed))
		{
			NSMenuItem *item;

			NSMenu *const debugMenu = [[NSMenu allocWithZone:[NSMenu menuZone]] initWithTitle:@"Debug"];
			item = [[NSApp mainMenu] insertItemWithTitle:@"Debug" action:NULL keyEquivalent:@"" atIndex:1];
			[item setSubmenu:debugMenu];
			[debugMenu release];

			[debugMenu addItemWithTitle:@"New Memory Window"
								 action:@selector(debugNewMemoryWindow:)
						  keyEquivalent:@"d"];
			[debugMenu addItemWithTitle:@"New Disassembly Window"
								 action:@selector(debugNewDisassemblyWindow:)
						  keyEquivalent:@"a"];
			[debugMenu addItemWithTitle:@"New Error Log Window"
								 action:@selector(debugNewErrorLogWindow:)
						  keyEquivalent:@"l"];
			[debugMenu addItemWithTitle:@"New (Break|Watch)points Window"
								 action:@selector(debugNewPointsWindow:)
						  keyEquivalent:@"b"];
			[debugMenu addItemWithTitle:@"New Devices Window"
								 action:@selector(debugNewDevicesWindow:)
						  keyEquivalent:@"D"];

			[debugMenu addItem:[NSMenuItem separatorItem]];

			[[debugMenu addItemWithTitle:@"Soft Reset"
								  action:@selector(debugSoftReset:)
						   keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF3FunctionKey]]
			 setKeyEquivalentModifierMask:0];
			[[debugMenu addItemWithTitle:@"Hard Reset"
								  action:@selector(debugHardReset:)
						   keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF3FunctionKey]]
			 setKeyEquivalentModifierMask:NSShiftKeyMask];

			NSMenu *const runMenu = [[NSMenu allocWithZone:[NSMenu menuZone]] initWithTitle:@"Run"];
			item = [[NSApp mainMenu] insertItemWithTitle:@"Run"
												  action:NULL
										   keyEquivalent:@""
												 atIndex:([[NSApp mainMenu] indexOfItemWithSubmenu:debugMenu] + 1)];
			[item setSubmenu:runMenu];
			[runMenu release];

			[runMenu addItemWithTitle:@"Break"
							   action:@selector(debugBreak:)
						keyEquivalent:@""];

			[runMenu addItem:[NSMenuItem separatorItem]];

			[[runMenu addItemWithTitle:@"Run"
								action:@selector(debugRun:)
						 keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF5FunctionKey]]
			 setKeyEquivalentModifierMask:0];
			[[runMenu addItemWithTitle:@"Run and Hide Debugger"
								action:@selector(debugRunAndHide:)
						 keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF12FunctionKey]]
			 setKeyEquivalentModifierMask:0];
			[[runMenu addItemWithTitle:@"Run to Next CPU"
								action:@selector(debugRunToNextCPU:)
						 keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF6FunctionKey]]
			 setKeyEquivalentModifierMask:0];
			[[runMenu addItemWithTitle:@"Run until Next Interrupt on Current CPU"
								action:@selector(debugRunToNextInterrupt:)
						 keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF7FunctionKey]]
			 setKeyEquivalentModifierMask:0];
			[[runMenu addItemWithTitle:@"Run until Next VBLANK"
								action:@selector(debugRunToNextVBLANK:)
						 keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF8FunctionKey]]
			 setKeyEquivalentModifierMask:0];
			[[runMenu addItemWithTitle:@"Run to Cursor"
								action:@selector(debugRunToCursor:)
						 keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF4FunctionKey]]
			 setKeyEquivalentModifierMask:0];

			[runMenu addItem:[NSMenuItem separatorItem]];

			[[runMenu addItemWithTitle:@"Step Into"
								action:@selector(debugStepInto:)
						 keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF11FunctionKey]]
			 setKeyEquivalentModifierMask:0];
			[[runMenu addItemWithTitle:@"Step Over"
								action:@selector(debugStepOver:)
						 keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF10FunctionKey]]
			 setKeyEquivalentModifierMask:0];
			[[runMenu addItemWithTitle:@"Step Out"
								action:@selector(debugStepOut:)
						 keyEquivalent:[NSString stringWithFormat:@"%C", (short)NSF10FunctionKey]]
			 setKeyEquivalentModifierMask:NSShiftKeyMask];
		}
		m_console = [[MAMEDebugConsole alloc] initWithMachine:*m_machine];
	}

	// make sure the debug windows are visible
	if (firststop)
	{
		NSDictionary *info = [NSDictionary dictionaryWithObjectsAndKeys:[NSValue valueWithPointer:&device],
																		@"MAMEDebugDevice",
																		[NSValue valueWithPointer:m_machine],
																		@"MAMEDebugMachine",
																		nil];
		[[NSNotificationCenter defaultCenter] postNotificationName:MAMEShowDebuggerNotification
															object:m_console
														  userInfo:info];
	}

	// get and process messages
	NSEvent *ev = [NSApp nextEventMatchingMask:NSAnyEventMask
									 untilDate:[NSDate distantFuture]
										inMode:NSDefaultRunLoopMode
									   dequeue:YES];
	if (ev != nil)
		[NSApp sendEvent:ev];

	[pool release];
}


//============================================================
//  debugger_update
//============================================================

void debugger_osx::debugger_update()
{
}
