//============================================================
//
//  debugosx.m - MacOS X Cocoa debug window handling
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================


// TODO:
//  * Automatic scrolling for console and log views
//  * Using alpha for disabled foreground colours doesn't really work
//  * New windows created from auxiliary windows should inherit focus rather than pointing at current CPU
//  * Should have map of machine to console for multiple machine support rather than a single console
//  * Improve behaviour of expression history in memory and disassembly windows - the double tap is annoying
//  * Keyboard shortcuts in error log windows
//  * Don't accept keyboard input while the game is running
//  * Interior focus rings - standard/exterior focus rings look really ugly here
//  * Scroll views with content narrower than clipping area are flaky under Tiger - nothing I can do about this


// standard Cocoa headers
#include <AvailabilityMacros.h>
#import <Cocoa/Cocoa.h>

// MAME headers
#include "emu.h"
#include "debug/debugvw.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "debugger.h"

// MAMEOS headers
#include "debugosx.h"



//============================================================
//  LOCAL VARIABLES
//============================================================

static MAMEDebugConsole *main_console = nil;

static BOOL waiting_for_debugger = NO;

static NSString *const MAMEHideDebuggerNotification = @"MAMEHideDebuggerNotification";
static NSString *const MAMEShowDebuggerNotification = @"MAMEShowDebuggerNotification";
static NSString *const MAMEAuxiliaryDebugWindowWillCloseNotification
															= @"MAMEAuxiliaryDebugWindowWillCloseNotification";


//============================================================
//  PROTOTYPES
//============================================================

static void debugwin_view_update(debug_view *view, void *osdprivate);

static void console_create_window(running_machine *machine);


//============================================================
//  osd_wait_for_debugger
//============================================================

void osd_wait_for_debugger(running_device *device, int firststop)
{
	// create a console window
	if (main_console == nil)
		console_create_window(device->machine);

	// make sure the debug windows are visible
	waiting_for_debugger = YES;
	if (firststop) {
		NSDictionary *info = [NSDictionary dictionaryWithObjectsAndKeys:[NSValue valueWithPointer:device],
																		@"MAMEDebugDevice",
																		nil];
		[[NSNotificationCenter defaultCenter] postNotificationName:MAMEShowDebuggerNotification
															object:main_console
														  userInfo:info];
	}

	// get and process messages
	{
		NSEvent *ev = [NSApp nextEventMatchingMask:NSAnyEventMask
										 untilDate:[NSDate distantFuture]
											inMode:NSDefaultRunLoopMode
										   dequeue:YES];
		if (ev != nil)
			[NSApp sendEvent:ev];
	}

	// mark the debugger as active
	waiting_for_debugger = NO;
}


//============================================================
//  debugwin_update_during_game
//============================================================

void debugwin_update_during_game(running_machine *machine)
{
}


//============================================================
//  debugwin_view_update
//============================================================

static void debugwin_view_update(debug_view *view, void *osdprivate)
{
	[(MAMEDebugView *)osdprivate update];
}


//============================================================
//  console_create_window
//============================================================

void console_create_window(running_machine *machine)
{
	main_console = [[MAMEDebugConsole alloc] initWithMachine:machine];
}


//============================================================
//  MAMEDebugView class
//============================================================

@implementation MAMEDebugCommandHistory

+ (NSInteger)defaultLength {
	return 100;
}


- (id)init {
	if (!(self = [super init]))
		return nil;
	length = [[self class] defaultLength];
	position = -1;
	current = nil;
	history = [[NSMutableArray alloc] initWithCapacity:length];
	return self;
}


- (void)dealloc {
	if (current != nil)
		[current release];
	if (history != nil)
		[history release];
	[super dealloc];
}


- (NSInteger)length {
	return length;
}


- (void)setLength:(NSInteger)l {
	length = l;
	while ([history count] > length)
		[history removeLastObject];
}


- (void)add:(NSString *)entry {
	if (([history count] == 0) || ![[history objectAtIndex:0] isEqualToString:entry]) {
		[history insertObject:entry atIndex:0];
		while ([history count] > length)
			[history removeLastObject];
	}
	position = -1;
}


- (NSString *)previous:(NSString *)cur {
	if ((position + 1) < [history count]) {
		if (position < 0) {
			[current autorelease];
			current = [cur copy];
		}
		return [history objectAtIndex:++position];
	} else {
		return nil;
	}
}


- (NSString *)next:(NSString *)cur {
	if (position > 0) {
		return [history objectAtIndex:--position];
	} else if (position == 0) {
		position--;
		return [[current retain] autorelease];
	} else {
		return nil;
	}
}


- (void)reset {
	position = -1;
	if (current != nil) {
		[current release];
		current = nil;
	}
}


- (void)clear {
	position = -1;
	if (current != nil) {
		[current release];
		current = nil;
	}
	[history removeAllObjects];
}

@end


//============================================================
//  MAMEDebugView class
//============================================================

@implementation MAMEDebugView

- (NSColor *)foregroundForAttribute:(UINT8)attrib {
	const CGFloat alpha = (attrib & DCA_DISABLED) ? 0.5 : 1.0;
	if (attrib & DCA_COMMENT)
		return [NSColor colorWithCalibratedRed:0.0 green:0.375 blue:0.0 alpha:1.0];
	else if (attrib & DCA_INVALID)
		return [NSColor colorWithCalibratedRed:0.0 green:0.0 blue:1.0 alpha:alpha];
	else if (attrib & DCA_CHANGED)
		return [NSColor colorWithCalibratedRed:0.875 green:0.0 blue:0.0 alpha:alpha];
	else
		return [NSColor colorWithCalibratedWhite:0.0 alpha:alpha];
}


- (NSColor *)backgroundForAttribute:(UINT8)attrib {
	if ((attrib & DCA_SELECTED) && (attrib & DCA_CURRENT)) {
		if ([[self window] isKeyWindow] && ([[self window] firstResponder] == self))
			return [NSColor colorWithCalibratedRed:0.875 green:0.625 blue:0.875 alpha:1.0];
		else
			return [NSColor colorWithCalibratedRed:0.875 green:0.5 blue:0.625 alpha:1.0];
	} else if (attrib & DCA_CURRENT) {
		return [NSColor colorWithCalibratedRed:1.0 green:0.625 blue:0.625 alpha:1.0];
	} else if (attrib & DCA_SELECTED) {
		if ([[self window] isKeyWindow] && ([[self window] firstResponder] == self))
			return [NSColor colorWithCalibratedRed:0.75 green:0.875 blue:1.0 alpha:1.0];
		else
			return [NSColor colorWithCalibratedWhite:0.875 alpha:1.0];
	} else if (attrib & DCA_ANCILLARY) {
		return [NSColor colorWithCalibratedWhite:0.75 alpha:1.0];
	} else {
		return [NSColor colorWithCalibratedWhite:1.0 alpha:1.0];
	}
}


- (debug_view_xy)convertLocation:(NSPoint)location {
	debug_view_xy position;
	position.x = lround(floor(location.x / fontWidth));
	position.y = lround(floor(location.y / fontHeight));
	if (position.x < 0)
		position.x = 0;
	else if (position.x >= totalSize.x)
		position.x = totalSize.x - 1;
	if (position.y < 0)
		position.y = 0;
	else if (position.y >= totalSize.y)
		position.y = totalSize.y - 1;
	return position;
}


- (void)convertBounds:(NSRect)b toPosition:(debug_view_xy *)origin size:(debug_view_xy *)size {
	origin->x = lround(floor(b.origin.x / fontWidth));
	origin->y = lround(floor(b.origin.y / fontHeight));
	size->x = lround(ceil((b.origin.x + b.size.width) / fontWidth)) - origin->x;
	size->y = lround(ceil((b.origin.y + b.size.height) / fontHeight)) - origin->y;
}


- (void)recomputeVisible {
	if ([self window] != nil) {
		debug_view_xy	origin, size;

		// this gets all the characters that are at least paritally visible
		[self convertBounds:[self visibleRect] toPosition:&origin size:&size];

		// need to render entire lines or we get screwed up characters when widening views
		origin.x = 0;
		size.x = totalSize.x;

		// tell them what we think
		debug_view_set_visible_size(view, size);
		debug_view_set_visible_position(view, origin);
		topLeft = origin;
	}
}


- (void)typeCharacterAndScrollToCursor:(char)ch {
	if (debug_view_get_cursor_supported(view)) {
		debug_view_xy oldPos = debug_view_get_cursor_position(view);
		debug_view_type_character(view, ch);
		{
			debug_view_xy newPos = debug_view_get_cursor_position(view);
			if ((newPos.x != oldPos.x) || (newPos.y != oldPos.y)) {
				[self scrollRectToVisible:NSMakeRect(newPos.x * fontWidth,
													 newPos.y * fontHeight,
													 fontWidth,
													 fontHeight)];
			}
		}
	} else {
		debug_view_type_character(view, ch);
	}
}


+ (NSFont *)defaultFont {
	// maybe we should get the configured system fixed-width font...
	return [NSFont fontWithName:@"Monaco" size:10];
}


- (id)initWithFrame:(NSRect)f type:(int)t machine:(running_machine *)m {
	if (!(self = [super initWithFrame:f]))
		return nil;
	type = t;
	machine = m;
	view = debug_view_alloc(machine, type, debugwin_view_update, self);
	if (view == nil) {
		[self release];
		return nil;
	}
	totalSize.x = totalSize.y = 0;
	topLeft.x = topLeft.y = 0;
	[self setFont:[[self class] defaultFont]];
	return self;
}


- (void)dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	if (view != NULL)
		debug_view_free(view);
	if (font != nil)
		[font release];
	[super dealloc];
}


- (void)update {
	debug_view_xy	newSize, newOrigin;

	// resize our frame if the total size has changed
	newSize = debug_view_get_total_size(view);
	if ((newSize.x != totalSize.x) || (newSize.y != totalSize.y)) {
		[self setFrameSize:NSMakeSize(fontWidth * newSize.x, fontHeight * newSize.y)];
		totalSize = newSize;
	}

	// scroll the view if we're being told to
	newOrigin = debug_view_get_visible_position(view);
	if (newOrigin.y != topLeft.y) {
		[self scrollPoint:NSMakePoint([self visibleRect].origin.x, newOrigin.y * fontHeight)];
		topLeft.y = newOrigin.y;
	}

	// recompute the visible area and mark as dirty
	[self recomputeVisible];
	[self setNeedsDisplay:YES];
}


- (NSSize)maximumFrameSize {
	debug_view_xy max = debug_view_get_total_size(view);
	return NSMakeSize(max.x * fontWidth, max.y * fontHeight);
}


- (NSFont *)font {
	return [[font retain] autorelease];
}


- (void)setFont:(NSFont *)f {
	[font autorelease];
	font = [f retain];
	fontWidth = [font maximumAdvancement].width;
	fontHeight = ceil([font ascender] - [font descender]);
	fontAscent = [font ascender];
	[[self enclosingScrollView] setLineScroll:fontHeight];
	totalSize.x = totalSize.y = 0;
	[self update];
}


- (void)windowDidBecomeKey:(NSNotification *)notification {
	NSWindow *win = [notification object];
	if ((win == [self window]) && ([win firstResponder] == self) && debug_view_get_cursor_supported(view))
		[self setNeedsDisplay:YES];
}


- (void)windowDidResignKey:(NSNotification *)notification {
	NSWindow *win = [notification object];
	if ((win == [self window]) && ([win firstResponder] == self) && debug_view_get_cursor_supported(view))
		[self setNeedsDisplay:YES];
}


- (BOOL)acceptsFirstResponder {
	return debug_view_get_cursor_supported(view);
}


- (BOOL)becomeFirstResponder {
	if (debug_view_get_cursor_supported(view)) {
		debug_view_xy pos;
		debug_view_set_cursor_visible(view, TRUE);
		pos = debug_view_get_cursor_position(view);
		[self scrollRectToVisible:NSMakeRect(pos.x * fontWidth, pos.y * fontHeight, fontWidth, fontHeight)];
		[self setNeedsDisplay:YES];
		return [super becomeFirstResponder];
	} else {
		return NO;
	}
}


- (BOOL)resignFirstResponder {
	if (debug_view_get_cursor_supported(view))
		[self setNeedsDisplay:YES];
	return [super resignFirstResponder];
}


- (void)viewDidMoveToSuperview {
	[[self enclosingScrollView] setLineScroll:fontHeight];
	[super viewDidMoveToSuperview];
}


- (void)viewDidMoveToWindow {
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidBecomeKeyNotification object:nil];
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidResignKeyNotification object:nil];
	if ([self window] != nil) {
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(windowDidBecomeKey:)
													 name:NSWindowDidBecomeKeyNotification
												   object:[self window]];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(windowDidResignKey:)
													 name:NSWindowDidResignKeyNotification
												   object:[self window]];
		[self recomputeVisible];
	}
}


- (BOOL)isFlipped {
	return YES;
}


- (void)drawRect:(NSRect)dirtyRect {
	const debug_view_char	*base;
	debug_view_xy			origin, size;
	debug_view_xy			position, clip;
	NSMutableString			*text;
	NSMutableDictionary		*attributes;
	UINT32					pass, row, col;

	// work out how much we need to draw
	[self recomputeVisible];
	origin = debug_view_get_visible_position(view);
	size = debug_view_get_visible_size(view);
	[self convertBounds:dirtyRect toPosition:&position size:&clip];

	// this gets the text for the whole visible area
	base = debug_view_get_chars(view);
	if (!base)
		return;

	text = [[NSMutableString alloc] initWithCapacity:clip.x];
	attributes = [[NSMutableDictionary alloc] initWithObjectsAndKeys:font, NSFontAttributeName, nil];
	for (pass = 0; pass < 2; pass++) {
		const debug_view_char *data = base + ((position.y - origin.y) * size.x);
		for (row = position.y; row < position.y + clip.y; row++, data += size.x) {
			int attr = -1;

			if ((row < origin.y) || (row >= origin.y + size.y))
				continue;

			// render entire lines to get character alignment right
			for (col = origin.x; col < origin.x + size.x; col++) {
				if ((attr != data[col - origin.x].attrib) && ([text length] > 0)) {
					if (pass == 0) {
						[[self backgroundForAttribute:attr] set];
						[NSBezierPath fillRect:NSMakeRect((col - [text length]) * fontWidth,
														  row * fontHeight,
														  [text length] * fontWidth,
														  fontHeight)];
					} else {
						[attributes setObject:[self foregroundForAttribute:attr]
									   forKey:NSForegroundColorAttributeName];
						[text drawAtPoint:NSMakePoint((col - [text length]) * fontWidth, row * fontHeight)
						   withAttributes:attributes];
					}
					[text setString:@""];
				}
				attr = data[col - origin.x].attrib;
				[text appendFormat:@"%c", data[col - origin.x].byte];
			}
			if ([text length] > 0) {
				if (pass == 0) {
					[[self backgroundForAttribute:attr] set];
					[NSBezierPath fillRect:NSMakeRect((col - [text length]) * fontWidth,
													  row * fontHeight,
													  [text length] * fontWidth,
													  fontHeight)];
				} else {
					[attributes setObject:[self foregroundForAttribute:attr]
								   forKey:NSForegroundColorAttributeName];
					[text drawAtPoint:NSMakePoint((col - [text length]) * fontWidth, row * fontHeight)
					   withAttributes:attributes];
				}
				[text setString:@""];
			}
		}
	}
	[attributes release];
	[text release];
}


- (void)mouseDown:(NSEvent *)event {
	NSPoint	location = [self convertPoint:[event locationInWindow] fromView:nil];
	if (debug_view_get_cursor_supported(view)) {
		debug_view_set_cursor_position(view, [self convertLocation:location]);
		debug_view_set_cursor_visible(view, TRUE);
		[self setNeedsDisplay:YES];
	}
}


- (void)mouseDragged:(NSEvent *)event {
	NSPoint	location = [self convertPoint:[event locationInWindow] fromView:nil];
	if (debug_view_get_cursor_supported(view)) {
		[self autoscroll:event];
		debug_view_set_cursor_position(view, [self convertLocation:location]);
		[self setNeedsDisplay:YES];
	}
}


- (void)rightMouseDown:(NSEvent *)event {
	NSPoint	location = [self convertPoint:[event locationInWindow] fromView:nil];
	if (debug_view_get_cursor_supported(view)) {
		debug_view_set_cursor_position(view, [self convertLocation:location]);
		debug_view_set_cursor_visible(view, TRUE);
		[self setNeedsDisplay:YES];
	}
	[super rightMouseDown:event];
}


- (void)keyDown:(NSEvent *)event {
	NSUInteger	modifiers = [event modifierFlags];
	NSString	*str = [event charactersIgnoringModifiers];

	if ([str length] == 1) {
		if (modifiers & NSNumericPadKeyMask) {
			switch ([str characterAtIndex:0]) {
				case NSUpArrowFunctionKey:
					if (modifiers & NSCommandKeyMask)
						debug_view_type_character(view, DCH_CTRLHOME);
					else
						debug_view_type_character(view, DCH_UP);
					return;
				case NSDownArrowFunctionKey:
					if (modifiers & NSCommandKeyMask)
						debug_view_type_character(view, DCH_CTRLEND);
					else
						debug_view_type_character(view, DCH_DOWN);
					return;
				case NSLeftArrowFunctionKey:
					if (modifiers & NSCommandKeyMask)
						[self typeCharacterAndScrollToCursor:DCH_HOME];
					else if (modifiers & NSAlternateKeyMask)
						[self typeCharacterAndScrollToCursor:DCH_CTRLLEFT];
					else
						[self typeCharacterAndScrollToCursor:DCH_LEFT];
					return;
				case NSRightArrowFunctionKey:
					if (modifiers & NSCommandKeyMask)
						[self typeCharacterAndScrollToCursor:DCH_END];
					else if (modifiers & NSAlternateKeyMask)
						[self typeCharacterAndScrollToCursor:DCH_CTRLRIGHT];
					else
						[self typeCharacterAndScrollToCursor:DCH_RIGHT];
					return;
				default:
					[self interpretKeyEvents:[NSArray arrayWithObject:event]];
					return;
			}
		} else if (modifiers & NSFunctionKeyMask) {
			switch ([str characterAtIndex:0]) {
				case NSPageUpFunctionKey:
					if (modifiers & NSAlternateKeyMask) {
						debug_view_type_character(view, DCH_PUP);
						return;
					}
				case NSPageDownFunctionKey:
					if (modifiers & NSAlternateKeyMask) {
						debug_view_type_character(view, DCH_PDOWN);
						return;
					}
				default:
					;
			}
			[super keyDown:event];
			return;
		}
	}
	[self interpretKeyEvents:[NSArray arrayWithObject:event]];
}


- (void)insertTab:(id)sender {
	if ([[self window] firstResponder] == self)
		[[self window] selectNextKeyView:self];
}


- (void)insertBacktab:(id)sender {
	if ([[self window] firstResponder] == self)
		[[self window] selectPreviousKeyView:self];
}


- (void)insertNewline:(id)sender {
	debug_cpu_single_step(machine, 1);
}


- (void)insertText:(id)string {
	NSUInteger	len;
	NSRange		found;
	if ([string isKindOfClass:[NSAttributedString class]])
		string = [string string];
	for (len = [string length], found = NSMakeRange(0, 0);
		 found.location < len;
		 found.location += found.length) {
		found = [string rangeOfComposedCharacterSequenceAtIndex:found.location];
		if (found.length == 1) {
			unichar ch = [string characterAtIndex:found.location];
			if ((ch >= 32) && (ch < 127))
				[self typeCharacterAndScrollToCursor:ch];
		}
	}
}

@end


//============================================================
//  MAMEMemoryView class
//============================================================

@implementation MAMEMemoryView

- (id)initWithFrame:(NSRect)f machine:(running_machine *)m {
	NSMenu	*contextMenu;

	if (!(self = [super initWithFrame:f type:DVT_MEMORY machine:m]))
		return nil;

	contextMenu = [[NSMenu allocWithZone:[NSMenu menuZone]] initWithTitle:@"Memory"];
	[self insertActionItemsInMenu:contextMenu atIndex:0];
	[self setMenu:contextMenu];
	[contextMenu release];

	return self;
}


- (void)dealloc {
	[super dealloc];
}


- (BOOL)validateMenuItem:(NSMenuItem *)item {
	SEL			action = [item action];
	NSInteger	tag = [item tag];

	if (action == @selector(showChunkSize:)) {
		[item setState:((tag == memory_view_get_bytes_per_chunk(view)) ? NSOnState : NSOffState)];
	} else if (action == @selector(showPhysicalAddresses:)) {
		[item setState:((tag == memory_view_get_physical(view)) ? NSOnState : NSOffState)];
	} else if (action == @selector(showReverseView:)) {
		[item setState:((tag == memory_view_get_reverse(view)) ? NSOnState : NSOffState)];
	} else if (action == @selector(showReverseViewToggle:)) {
		[item setState:(memory_view_get_reverse(view) ? NSOnState : NSOffState)];
	}
	return YES;
}


- (NSSize)maximumFrameSize {
	const int					selected = memory_view_get_subview(view);
	const memory_subview_item	*subitem;
	debug_view_xy				max;

	max.x = max.y = 0;
	for (subitem = memory_view_get_subview_list(view); subitem != NULL; subitem = subitem->next) {
		debug_view_xy	current;
		memory_view_set_subview(view, subitem->index);
		current = debug_view_get_total_size(view);
		if (current.x > max.x)
			max.x = current.x;
		if (current.y > max.y)
			max.y = current.y;
	}
	memory_view_set_subview(view, selected);
	return NSMakeSize(max.x * fontWidth, max.y * fontHeight);
}


- (NSString *)selectedSubviewName {
	const memory_subview_item *subitem = memory_view_get_current_subview(view);
	if (subitem != NULL)
		return [NSString stringWithUTF8String:subitem->name];
	else
		return @"";
}


- (int)selectedSubviewIndex {
	const memory_subview_item *subitem = memory_view_get_current_subview(view);
	if (subitem != NULL)
		return subitem->index;
	else
		return -1;
}


- (void)selectSubviewAtIndex:(int)index {
	const int	selected = memory_view_get_subview(view);
	if (selected != index) {
		memory_view_set_subview(view, index);
		if ([[self window] firstResponder] != self)
			debug_view_set_cursor_visible(view, FALSE);
	}
}


- (void)selectSubviewForCPU:(running_device *)device {
	const int					selected = memory_view_get_subview(view);
	const memory_subview_item	*subitem;
	for (subitem = memory_view_get_subview_list(view); subitem != NULL; subitem = subitem->next) {
		if (subitem->space->cpu == device) {
			if (selected != subitem->index) {
				memory_view_set_subview(view, subitem->index);
				if ([[self window] firstResponder] != self)
					debug_view_set_cursor_visible(view, FALSE);
			}
			break;
		}
	}
}


- (NSString *)expression {
	return [NSString stringWithUTF8String:memory_view_get_expression(view)];
}


- (void)setExpression:(NSString *)exp {
	memory_view_set_expression(view, [exp UTF8String]);
}


- (IBAction)showChunkSize:(id)sender {
	memory_view_set_bytes_per_chunk(view, [sender tag]);
}


- (IBAction)showPhysicalAddresses:(id)sender {
	memory_view_set_physical(view, [sender tag]);
}


- (IBAction)showReverseView:(id)sender {
	memory_view_set_reverse(view, [sender tag]);
}


- (IBAction)showReverseViewToggle:(id)sender {
	memory_view_set_reverse(view, !memory_view_get_reverse(view));
}


- (void)insertActionItemsInMenu:(NSMenu *)menu atIndex:(NSInteger)index {
	{
		NSInteger tag;
		for (tag = 1; tag <= 8; tag <<= 1) {
			NSString	*title = [NSString stringWithFormat:@"%d-byte Chunks", tag];
			NSMenuItem	*chunkItem = [menu insertItemWithTitle:title
														action:@selector(showChunkSize:)
												 keyEquivalent:[NSString stringWithFormat:@"%d", tag]
													   atIndex:index++];
			[chunkItem setTarget:self];
			[chunkItem setTag:tag];
		}
	}
	[menu insertItem:[NSMenuItem separatorItem] atIndex:index++];
	{
		NSMenuItem *logicalItem = [menu insertItemWithTitle:@"Logical Addresses"
													 action:@selector(showPhysicalAddresses:)
											  keyEquivalent:@"v"
													atIndex:index++];
		[logicalItem setTarget:self];
		[logicalItem setTag:FALSE];
	}
	{
		NSMenuItem *physicalItem = [menu insertItemWithTitle:@"Physical Addresses"
													  action:@selector(showPhysicalAddresses:)
											   keyEquivalent:@"y"
													 atIndex:index++];
		[physicalItem setTarget:self];
		[physicalItem setTag:TRUE];
	}
	[menu insertItem:[NSMenuItem separatorItem] atIndex:index++];
	{
		NSMenuItem *reverseItem = [menu insertItemWithTitle:@"Reverse View"
													 action:@selector(showReverseViewToggle:)
											  keyEquivalent:@"r"
													atIndex:index++];
		[reverseItem setTarget:self];
	}
	if (index < [menu numberOfItems])
		[menu insertItem:[NSMenuItem separatorItem] atIndex:index++];
}


- (void)insertSubviewItemsInMenu:(NSMenu *)menu atIndex:(NSInteger)index {
	const memory_subview_item *subitem;
	for (subitem = memory_view_get_subview_list(view); subitem != NULL; subitem = subitem->next) {
		[[menu insertItemWithTitle:[NSString stringWithUTF8String:subitem->name]
							action:NULL
					 keyEquivalent:@""
						   atIndex:index++] setTag:subitem->index];
	}
	if (index < [menu numberOfItems])
		[menu insertItem:[NSMenuItem separatorItem] atIndex:index++];
}

@end


//============================================================
//  MAMEDisassemblyView class
//============================================================

@implementation MAMEDisassemblyView

- (debug_cpu_breakpoint *)findBreakpointAtAddress:(offs_t)address inAddressSpace:(const address_space *)space {
	cpu_debug_data			*cpuinfo = cpu_get_debug_data(space->cpu);
	debug_cpu_breakpoint	*bp;
	for (bp = cpuinfo->bplist; (bp != NULL) && (address != bp->address); bp = bp->next) {}
	return bp;
}

- (void)createContextMenu {
	NSMenu		*contextMenu = [[NSMenu allocWithZone:[NSMenu menuZone]] initWithTitle:@"Disassembly"];
	NSMenuItem	*item;

	item = [contextMenu addItemWithTitle:@"Toggle Breakpoint"
								  action:@selector(debugToggleBreakpoint:)
						   keyEquivalent:[NSString stringWithFormat:@"%C", NSF9FunctionKey]];
	[item setKeyEquivalentModifierMask:0];
	[item setTarget:self];

	item = [contextMenu addItemWithTitle:@"Disable Breakpoint"
								  action:@selector(debugToggleBreakpointEnable:)
						   keyEquivalent:[NSString stringWithFormat:@"%C", NSF9FunctionKey]];
	[item setKeyEquivalentModifierMask:NSShiftKeyMask];
	[item setTarget:self];

	[contextMenu addItem:[NSMenuItem separatorItem]];

	item = [contextMenu addItemWithTitle:@"Run to Cursor"
								  action:@selector(debugRunToCursor:)
						   keyEquivalent:[NSString stringWithFormat:@"%C", NSF4FunctionKey]];
	[item setKeyEquivalentModifierMask:0];
	[item setTarget:self];

	[contextMenu addItem:[NSMenuItem separatorItem]];

	item = [contextMenu addItemWithTitle:@"Raw Opcodes"
								  action:@selector(showRightColumn:)
						   keyEquivalent:@"r"];
	[item setTarget:self];
	[item setTag:DASM_RIGHTCOL_RAW];

	item = [contextMenu addItemWithTitle:@"Encrypted Opcodes"
								  action:@selector(showRightColumn:)
						   keyEquivalent:@"e"];
	[item setTarget:self];
	[item setTag:DASM_RIGHTCOL_ENCRYPTED];

	item = [contextMenu addItemWithTitle:@"Comments"
								  action:@selector(showRightColumn:)
						   keyEquivalent:@"n"];
	[item setTarget:self];
	[item setTag:DASM_RIGHTCOL_COMMENTS];

	[self setMenu:contextMenu];
	[contextMenu release];
}


- (id)initWithFrame:(NSRect)f machine:(running_machine *)m useConsole:(BOOL)uc {
	if (!(self = [super initWithFrame:f type:DVT_DISASSEMBLY machine:m]))
		return nil;
	useConsole = uc;
	[self createContextMenu];
	return self;
}


- (void)dealloc {
	[super dealloc];
}


- (BOOL)validateMenuItem:(NSMenuItem *)item {
	SEL						action = [item action];
	BOOL					inContextMenu = ([item menu] == [self menu]);
	BOOL					haveCursor = NO, isCurrent = NO;
	debug_cpu_breakpoint	*breakpoint = NULL;

	if (debug_view_get_cursor_visible(view)) {
		const address_space *space = disasm_view_get_current_subview(view)->space;
		isCurrent = (debug_cpu_get_visible_cpu(machine) == space->cpu);
		if (!useConsole || isCurrent) {
			offs_t address = memory_byte_to_address(space, disasm_view_get_selected_address(view));
			haveCursor = YES;
			breakpoint = [self findBreakpointAtAddress:address inAddressSpace:space];
		}
	}

	if (action == @selector(debugToggleBreakpoint:)) {
		if (haveCursor) {
			if (breakpoint != NULL) {
				if (inContextMenu)
					[item setTitle:@"Clear Breakpoint"];
				else
					[item setTitle:@"Clear Breakpoint at Cursor"];
			} else {
				if (inContextMenu)
					[item setTitle:@"Set Breakpoint"];
				else
					[item setTitle:@"Set Breakpoint at Cursor"];
			}
		} else {
			if (inContextMenu)
				[item setTitle:@"Toggle Breakpoint"];
			else
				[item setTitle:@"Toggle Breakpoint at Cursor"];
		}
		return haveCursor;
	} else if (action == @selector(debugToggleBreakpointEnable:)) {
		if ((breakpoint != NULL) && !breakpoint->enabled) {
			if (inContextMenu)
				[item setTitle:@"Enable Breakpoint"];
			else
				[item setTitle:@"Enable Breakpoint at Cursor"];
		} else {
			if (inContextMenu)
				[item setTitle:@"Disable Breakpoint"];
			else
				[item setTitle:@"Disable Breakpoint at Cursor"];
		}
		return (breakpoint != NULL);
	} else if (action == @selector(debugRunToCursor:)) {
		return isCurrent;
	} else if (action == @selector(showRightColumn:)) {
		[item setState:((disasm_view_get_right_column(view) == [item tag]) ? NSOnState : NSOffState)];
		return YES;
	} else {
		return YES;
	}
}


- (NSSize)maximumFrameSize {
	const int					selected = disasm_view_get_subview(view);
	const disasm_subview_item	*subitem;
	debug_view_xy				max;

	max.x = max.y = 0;
	for (subitem = disasm_view_get_subview_list(view); subitem != NULL; subitem = subitem->next) {
		debug_view_xy	current;
		disasm_view_set_subview(view, subitem->index);
		current = debug_view_get_total_size(view);
		if (current.x > max.x)
			max.x = current.x;
		if (current.y > max.y)
			max.y = current.y;
	}
	disasm_view_set_subview(view, selected);
	return NSMakeSize(max.x * fontWidth, max.y * fontHeight);
}


- (NSString *)selectedSubviewName {
	const disasm_subview_item *subitem = disasm_view_get_current_subview(view);
	if (subitem != NULL)
		return [NSString stringWithUTF8String:subitem->name];
	else
		return @"";
}


- (int)selectedSubviewIndex {
	const disasm_subview_item *subitem = disasm_view_get_current_subview(view);
	if (subitem != NULL)
		return subitem->index;
	else
		return -1;
}


- (void)selectSubviewAtIndex:(int)index {
	const int	selected = disasm_view_get_subview(view);
	if (selected != index) {
		disasm_view_set_subview(view, index);
		if ([[self window] firstResponder] != self)
			debug_view_set_cursor_visible(view, FALSE);
	}
}


- (void)selectSubviewForCPU:(running_device *)device {
	const int					selected = disasm_view_get_subview(view);
	const disasm_subview_item	*subitem;
	for (subitem = disasm_view_get_subview_list(view); subitem != NULL; subitem = subitem->next) {
		if (subitem->space->cpu == device) {
			if (selected != subitem->index) {
				disasm_view_set_subview(view, subitem->index);
				if ([[self window] firstResponder] != self)
					debug_view_set_cursor_visible(view, FALSE);
			}
			break;
		}
	}
}


- (NSString *)expression {
	return [NSString stringWithUTF8String:disasm_view_get_expression(view)];
}


- (void)setExpression:(NSString *)exp {
	disasm_view_set_expression(view, [exp UTF8String]);
}


- (IBAction)debugToggleBreakpoint:(id)sender {
	if (debug_view_get_cursor_visible(view)) {
		const address_space *space = disasm_view_get_current_subview(view)->space;
		if (!useConsole || (debug_cpu_get_visible_cpu(machine) == space->cpu)) {
			offs_t				address = memory_byte_to_address(space, disasm_view_get_selected_address(view));
			debug_cpu_breakpoint *bp = [self findBreakpointAtAddress:address inAddressSpace:space];

			// if it doesn't exist, add a new one
			if (useConsole) {
				NSString *command;
				if (bp == NULL)
					command = [NSString stringWithFormat:@"bpset %lX", (unsigned long)address];
				else
					command = [NSString stringWithFormat:@"bpclear %X", (unsigned)bp->index];
				debug_console_execute_command(machine, [command UTF8String], 1);
			} else {
				if (bp == NULL)
					debug_cpu_breakpoint_set(space->cpu, address, NULL, NULL);
				else
					debug_cpu_breakpoint_clear(machine, bp->index);
				debug_view_update_type(machine, DVT_DISASSEMBLY);
			}
		}
	}
}


- (IBAction)debugToggleBreakpointEnable:(id)sender {
	if (debug_view_get_cursor_visible(view)) {
		const address_space *space = disasm_view_get_current_subview(view)->space;
		if (!useConsole || (debug_cpu_get_visible_cpu(machine) == space->cpu)) {
			offs_t				address = memory_byte_to_address(space, disasm_view_get_selected_address(view));
			debug_cpu_breakpoint *bp = [self findBreakpointAtAddress:address inAddressSpace:space];

			if (bp != NULL) {
				NSString *command;
				if (useConsole) {
					if (bp->enabled)
						command = [NSString stringWithFormat:@"bpdisable %X", (unsigned)bp->index];
					else
						command = [NSString stringWithFormat:@"bpenable %X", (unsigned)bp->index];
					debug_console_execute_command(machine, [command UTF8String], 1);
				} else {
					debug_cpu_breakpoint_enable(machine, bp->index, !bp->enabled);
					debug_view_update_type(machine, DVT_DISASSEMBLY);
				}
			}
		}
	}
}


- (IBAction)debugRunToCursor:(id)sender {
	if (debug_view_get_cursor_visible(view)) {
		const address_space *space = disasm_view_get_current_subview(view)->space;
		if (debug_cpu_get_visible_cpu(machine) == space->cpu) {
			offs_t address = memory_byte_to_address(space, disasm_view_get_selected_address(view));
			if (useConsole) {
				NSString *command = [NSString stringWithFormat:@"go %lX", (unsigned long)address];
				debug_console_execute_command(machine, [command UTF8String], 1);
			} else {
				debug_cpu_go(machine, address);
			}
		}
	}
}


- (IBAction)showRightColumn:(id)sender {
	disasm_view_set_right_column(view, (disasm_right_column) [sender tag]);
}


- (void)insertActionItemsInMenu:(NSMenu *)menu atIndex:(NSInteger)index {
	{
		NSMenuItem *breakItem = [menu insertItemWithTitle:@"Toggle Breakpoint at Cursor"
												   action:@selector(debugToggleBreakpoint:)
											keyEquivalent:[NSString stringWithFormat:@"%C", NSF9FunctionKey]
												  atIndex:index++];
		[breakItem setKeyEquivalentModifierMask:0];
		[breakItem setTarget:self];
	}
	{
		NSMenuItem *disableItem = [menu insertItemWithTitle:@"Disable Breakpoint at Cursor"
													 action:@selector(debugToggleBreakpointEnable:)
											  keyEquivalent:[NSString stringWithFormat:@"%C", NSF9FunctionKey]
													atIndex:index++];
		[disableItem setKeyEquivalentModifierMask:NSShiftKeyMask];
		[disableItem setAlternate:YES];
		[disableItem setTarget:self];
	}
	{
		NSMenu		*runMenu = [[menu itemWithTitle:@"Run"] submenu];
		NSMenuItem	*runItem;
		if (runMenu != nil) {
			runItem = [runMenu addItemWithTitle:@"to Cursor"
										 action:@selector(debugRunToCursor:)
								  keyEquivalent:[NSString stringWithFormat:@"%C", NSF4FunctionKey]];
		} else {
			runItem = [menu insertItemWithTitle:@"Run to Cursor"
										 action:@selector(debugRunToCursor:)
								  keyEquivalent:[NSString stringWithFormat:@"%C", NSF4FunctionKey]
										atIndex:index++];
		}
		[runItem setKeyEquivalentModifierMask:0];
		[runItem setTarget:self];
	}
	[menu insertItem:[NSMenuItem separatorItem] atIndex:index++];
	{
		NSMenuItem *rawItem = [menu insertItemWithTitle:@"Show Raw Opcodes"
												 action:@selector(showRightColumn:)
										  keyEquivalent:@"r"
												atIndex:index++];
		[rawItem setTarget:self];
		[rawItem setTag:DASM_RIGHTCOL_RAW];
	}
	{
		NSMenuItem *encItem = [menu insertItemWithTitle:@"Show Encrypted Opcodes"
												 action:@selector(showRightColumn:)
										  keyEquivalent:@"e"
												atIndex:index++];
		[encItem setTarget:self];
		[encItem setTag:DASM_RIGHTCOL_ENCRYPTED];
	}
	{
		NSMenuItem *commentsItem = [menu insertItemWithTitle:@"Show Comments"
													  action:@selector(showRightColumn:)
											   keyEquivalent:@"n"
													 atIndex:index++];
		[commentsItem setTarget:self];
		[commentsItem setTag:DASM_RIGHTCOL_COMMENTS];
	}
	if (index < [menu numberOfItems])
		[menu insertItem:[NSMenuItem separatorItem] atIndex:index++];
}


- (void)insertSubviewItemsInMenu:(NSMenu *)menu atIndex:(NSInteger)index {
	const disasm_subview_item *subitem;
	for (subitem = disasm_view_get_subview_list(view); subitem != NULL; subitem = subitem->next) {
		[[menu insertItemWithTitle:[NSString stringWithUTF8String:subitem->name]
							action:NULL
					 keyEquivalent:@""
						   atIndex:index++] setTag:subitem->index];
	}
	if (index < [menu numberOfItems])
		[menu insertItem:[NSMenuItem separatorItem] atIndex:index++];
}

@end


//============================================================
//  MAMERegistersView class
//============================================================

@implementation MAMERegistersView

- (id)initWithFrame:(NSRect)f machine:(running_machine *)m {
	if (!(self = [super initWithFrame:f type:DVT_REGISTERS machine:m]))
		return nil;
	return self;
}


- (void)dealloc {
	[super dealloc];
}


- (NSSize)maximumFrameSize {
	const int						selected = registers_view_get_subview(view);
	const registers_subview_item	*subitem;
	debug_view_xy					max;

	max.x = max.y = 0;
	for (subitem = registers_view_get_subview_list(view); subitem != NULL; subitem = subitem->next) {
		debug_view_xy	current;
		registers_view_set_subview(view, subitem->index);
		current = debug_view_get_total_size(view);
		if (current.x > max.x)
			max.x = current.x;
		if (current.y > max.y)
			max.y = current.y;
	}
	registers_view_set_subview(view, selected);
	return NSMakeSize(max.x * fontWidth, max.y * fontHeight);
}


- (NSString *)selectedSubviewName {
	const registers_subview_item *subitem = registers_view_get_current_subview(view);
	if (subitem != NULL)
		return [NSString stringWithUTF8String:subitem->name];
	else
		return @"";
}


- (int)selectedSubviewIndex {
	const registers_subview_item *subitem = registers_view_get_current_subview(view);
	if (subitem != NULL)
		return subitem->index;
	else
		return -1;
}


- (void)selectSubviewAtIndex:(int)index {
	const int	selected = registers_view_get_subview(view);
	if (selected != index) {
		registers_view_set_subview(view, index);
		if ([[self window] firstResponder] != self)
			debug_view_set_cursor_visible(view, FALSE);
	}
}


- (void)selectSubviewForCPU:(running_device *)device {
	const int						selected = registers_view_get_subview(view);
	const registers_subview_item	*subitem;
	device_state_interface			*stateintf = device_state(device);
	for (subitem = registers_view_get_subview_list(view); subitem != NULL; subitem = subitem->next) {
		if (subitem->stateintf == stateintf) {
			if (selected != subitem->index) {
				registers_view_set_subview(view, subitem->index);
				if ([[self window] firstResponder] != self)
					debug_view_set_cursor_visible(view, FALSE);
			}
			break;
		}
	}
}

@end


//============================================================
//  MAMEConsoleView class
//============================================================

@implementation MAMEConsoleView

- (id)initWithFrame:(NSRect)f machine:(running_machine *)m {
	if (!(self = [super initWithFrame:f type:DVT_CONSOLE machine:m]))
		return nil;
	return self;
}


- (void)dealloc {
	[super dealloc];
}

@end


//============================================================
//  MAMEErrorLogView class
//============================================================

@implementation MAMEErrorLogView

- (id)initWithFrame:(NSRect)f machine:(running_machine *)m {
	if (!(self = [super initWithFrame:f type:DVT_LOG machine:m]))
		return nil;
	return self;
}


- (void)dealloc {
	[super dealloc];
}

@end


//============================================================
//  MAMEDebugWindowHandler class
//============================================================

@implementation MAMEDebugWindowHandler

+ (void)addCommonActionItems:(NSMenu *)menu {
	{
		NSMenuItem *runParentItem = [menu addItemWithTitle:@"Run"
													action:@selector(debugRun:)
											 keyEquivalent:[NSString stringWithFormat:@"%C", NSF5FunctionKey]];
		NSMenu *runMenu = [[NSMenu allocWithZone:[NSMenu menuZone]] initWithTitle:@"Run"];
		[runParentItem setSubmenu:runMenu];
		[runMenu release];
		[runParentItem setKeyEquivalentModifierMask:0];
		[[runMenu addItemWithTitle:@"and Hide Debugger"
							action:@selector(debugRunAndHide:)
					 keyEquivalent:[NSString stringWithFormat:@"%C", NSF12FunctionKey]]
		 setKeyEquivalentModifierMask:0];
		[[runMenu addItemWithTitle:@"to Next CPU"
							action:@selector(debugRunToNextCPU:)
					 keyEquivalent:[NSString stringWithFormat:@"%C", NSF6FunctionKey]]
		 setKeyEquivalentModifierMask:0];
		[[runMenu addItemWithTitle:@"until Next Interrupt on Current CPU"
							action:@selector(debugRunToNextInterrupt:)
					 keyEquivalent:[NSString stringWithFormat:@"%C", NSF7FunctionKey]]
		 setKeyEquivalentModifierMask:0];
		[[runMenu addItemWithTitle:@"until Next VBLANK"
							action:@selector(debugRunToNextVBLANK:)
					 keyEquivalent:[NSString stringWithFormat:@"%C", NSF8FunctionKey]]
		 setKeyEquivalentModifierMask:0];
	}
	{
		NSMenuItem *stepParentItem = [menu addItemWithTitle:@"Step" action:NULL keyEquivalent:@""];
		NSMenu *stepMenu = [[NSMenu allocWithZone:[NSMenu menuZone]] initWithTitle:@"Step"];
		[stepParentItem setSubmenu:stepMenu];
		[stepMenu release];
		[[stepMenu addItemWithTitle:@"Into"
							 action:@selector(debugStepInto:)
					  keyEquivalent:[NSString stringWithFormat:@"%C", NSF11FunctionKey]]
		 setKeyEquivalentModifierMask:0];
		[[stepMenu addItemWithTitle:@"Over"
							 action:@selector(debugStepOver:)
					  keyEquivalent:[NSString stringWithFormat:@"%C", NSF10FunctionKey]]
		 setKeyEquivalentModifierMask:0];
		[[stepMenu addItemWithTitle:@"Out"
							 action:@selector(debugStepOut:)
					  keyEquivalent:[NSString stringWithFormat:@"%C", NSF10FunctionKey]]
		 setKeyEquivalentModifierMask:NSShiftKeyMask];
	}
	{
		NSMenuItem *resetParentItem = [menu addItemWithTitle:@"Reset" action:NULL keyEquivalent:@""];
		NSMenu *resetMenu = [[NSMenu allocWithZone:[NSMenu menuZone]] initWithTitle:@"Reset"];
		[resetParentItem setSubmenu:resetMenu];
		[resetMenu release];
		[[resetMenu addItemWithTitle:@"Soft"
							  action:@selector(debugSoftReset:)
					   keyEquivalent:[NSString stringWithFormat:@"%C", NSF3FunctionKey]]
		 setKeyEquivalentModifierMask:0];
		[[resetMenu addItemWithTitle:@"Hard"
							  action:@selector(debugHardReset:)
					   keyEquivalent:[NSString stringWithFormat:@"%C", NSF3FunctionKey]]
		 setKeyEquivalentModifierMask:NSShiftKeyMask];
	}
	[menu addItem:[NSMenuItem separatorItem]];
	{
		NSMenuItem *newParentItem = [menu addItemWithTitle:@"New" action:NULL keyEquivalent:@""];
		NSMenu *newMenu = [[NSMenu allocWithZone:[NSMenu menuZone]] initWithTitle:@"New"];
		[newParentItem setSubmenu:newMenu];
		[newMenu release];
		[newMenu addItemWithTitle:@"Memory Window"
						   action:@selector(debugNewMemoryWindow:)
					keyEquivalent:@"d"];
		[newMenu addItemWithTitle:@"Disassembly Window"
						   action:@selector(debugNewDisassemblyWindow:)
					keyEquivalent:@"a"];
		[newMenu addItemWithTitle:@"Error Log Window"
						   action:@selector(debugNewErrorLogWindow:)
					keyEquivalent:@"l"];
	}
	[menu addItem:[NSMenuItem separatorItem]];
	[menu addItemWithTitle:@"Close Window" action:@selector(performClose:) keyEquivalent:@"w"];
	[menu addItemWithTitle:@"Exit" action:@selector(debugExit:) keyEquivalent:@""];
}


+ (NSPopUpButton *)newActionButtonWithFrame:(NSRect)frame {
	NSPopUpButton *actionButton = [[NSPopUpButton alloc] initWithFrame:frame pullsDown:YES];
	[actionButton setTitle:@""];
	[actionButton addItemWithTitle:@""];
	[actionButton setBezelStyle:NSShadowlessSquareBezelStyle];
	[actionButton setFocusRingType:NSFocusRingTypeNone];
	[[actionButton cell] setArrowPosition:NSPopUpArrowAtCenter];
	[[self class] addCommonActionItems:[actionButton menu]];
	return actionButton;
}


- (id)initWithMachine:(running_machine *)m title:(NSString *)t {
	if (!(self = [super init]))
		return nil;

	window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 320, 240)
										 styleMask:(NSTitledWindowMask |
													NSClosableWindowMask |
													NSMiniaturizableWindowMask |
													NSResizableWindowMask)
										   backing:NSBackingStoreBuffered
											 defer:YES];
	[window setReleasedWhenClosed:NO];
	[window setDelegate:self];
	[window setTitle:t];
	[window setContentMinSize:NSMakeSize(320, 240)];

	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(showDebugger:)
												 name:MAMEShowDebuggerNotification
											   object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(hideDebugger:)
												 name:MAMEHideDebuggerNotification
											   object:nil];

	machine = m;

	return self;
}


- (void)dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];

	if (window != nil)
		[window release];

	[super dealloc];
}


- (void)activate {
	[window makeKeyAndOrderFront:self];
}


- (IBAction)debugRun:(id)sender {
	debug_cpu_go(machine, ~0);
}


- (IBAction)debugRunAndHide:(id)sender {
	[[NSNotificationCenter defaultCenter] postNotificationName:MAMEHideDebuggerNotification object:self];
	debug_cpu_go(machine, ~0);
}


- (IBAction)debugRunToNextCPU:(id)sender {
	debug_cpu_next_cpu(machine);
}


- (IBAction)debugRunToNextInterrupt:(id)sender {
	debug_cpu_go_interrupt(machine, -1);
}


- (IBAction)debugRunToNextVBLANK:(id)sender {
	debug_cpu_go_vblank(machine);
}


- (IBAction)debugStepInto:(id)sender {
	debug_cpu_single_step(machine, 1);
}


- (IBAction)debugStepOver:(id)sender {
	debug_cpu_single_step_over(machine, 1);
}


- (IBAction)debugStepOut:(id)sender {
	debug_cpu_single_step_out(machine);
}


- (IBAction)debugSoftReset:(id)sender {
	mame_schedule_soft_reset(machine);
}


- (IBAction)debugHardReset:(id)sender {
	mame_schedule_hard_reset(machine);
}


- (IBAction)debugExit:(id)sender {
	mame_schedule_exit(machine);
}


- (void)showDebugger:(NSNotification *)notification {
	running_device *device = (running_device *) [[[notification userInfo] objectForKey:@"MAMEDebugDevice"] pointerValue];
	if (device->machine == machine) {
		if (![window isVisible] && ![window isMiniaturized])
			[window orderFront:self];
	}
}


- (void)hideDebugger:(NSNotification *)notification {
	[window orderOut:self];
}

@end



//============================================================
//  MAMEDebugConsole class
//============================================================

@implementation MAMEDebugConsole

- (id)initWithMachine:(running_machine *)m {
	NSSplitView		*regSplit, *dasmSplit;
	NSScrollView	*regScroll, *dasmScroll, *consoleScroll;
	NSView			*consoleContainer;
	NSPopUpButton	*actionButton;
	NSRect			rct;

	// initialise superclass
	if (!(self = [super initWithMachine:m title:@"Debug"]))
		return nil;
	history = [[MAMEDebugCommandHistory alloc] init];
	auxiliaryWindows = [[NSMutableArray alloc] init];

	// create the register view
	regView = [[MAMERegistersView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100) machine:machine];
	regScroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
	[regScroll setDrawsBackground:YES];
	[regScroll setHasHorizontalScroller:YES];
	[regScroll setHasVerticalScroller:YES];
	[regScroll setAutohidesScrollers:YES];
	[regScroll setBorderType:NSBezelBorder];
	[regScroll setDocumentView:regView];
	[regView release];

	// create the disassembly view
	dasmView = [[MAMEDisassemblyView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)
												  machine:machine
											   useConsole:YES];
	[dasmView setExpression:@"curpc"];
	dasmScroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
	[dasmScroll setDrawsBackground:YES];
	[dasmScroll setHasHorizontalScroller:YES];
	[dasmScroll setHasVerticalScroller:YES];
	[dasmScroll setAutohidesScrollers:YES];
	[dasmScroll setBorderType:NSBezelBorder];
	[dasmScroll setDocumentView:dasmView];
	[dasmView release];

	// create the console view
	consoleView = [[MAMEConsoleView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100) machine:machine];
	consoleScroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
	[consoleScroll setDrawsBackground:YES];
	[consoleScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[consoleScroll setHasHorizontalScroller:YES];
	[consoleScroll setHasVerticalScroller:YES];
	[consoleScroll setAutohidesScrollers:YES];
	[consoleScroll setBorderType:NSBezelBorder];
	[consoleScroll setDocumentView:consoleView];
	[consoleView release];

	// create the command field
	commandField = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 100, 19)];
	[commandField setAutoresizingMask:(NSViewWidthSizable | NSViewMaxYMargin)];
	[commandField setFont:[[MAMEDebugView class] defaultFont]];
	[commandField setFocusRingType:NSFocusRingTypeNone];
	[commandField setTarget:self];
	[commandField setAction:@selector(doCommand:)];
	[commandField setDelegate:self];
	rct = [commandField frame];
	[commandField setFrame:NSMakeRect(rct.size.height, 0, rct.size.width - rct.size.height, rct.size.height)];

	// create the action pull-down button
	actionButton = [[self class] newActionButtonWithFrame:NSMakeRect(0, 0, rct.size.height, rct.size.height)];
	[actionButton setAutoresizingMask:(NSViewMaxXMargin | NSViewMaxYMargin)];
	[dasmView insertActionItemsInMenu:[actionButton menu] atIndex:1];

	// create the container for the console and command input field
	consoleContainer = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
	[consoleScroll setFrame:NSMakeRect(0,
									   rct.size.height,
									   100,
									   [consoleContainer bounds].size.height - rct.size.height)];
	[consoleContainer addSubview:consoleScroll];
	[consoleContainer addSubview:commandField];
	[consoleContainer addSubview:actionButton];
	[consoleScroll release];
	[commandField release];
	[actionButton release];

	// create the split between the disassembly and the console
	dasmSplit = [[NSSplitView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
	[dasmSplit setDelegate:self];
	[dasmSplit setVertical:NO];
	[dasmSplit addSubview:dasmScroll];
	[dasmSplit addSubview:consoleContainer];
	[dasmScroll release];
	[consoleContainer release];

	// create the split between the registers and the console
	regSplit = [[NSSplitView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
	[regSplit setDelegate:self];
	[regSplit setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[regSplit setVertical:YES];
	[regSplit addSubview:regScroll];
	[regSplit addSubview:dasmSplit];
	[regScroll release];
	[dasmSplit release];

	// put the split views in the window and get them into a half-reasonable state
	[window setContentView:regSplit];
	[regSplit release];
	[regSplit adjustSubviews];
	[dasmSplit adjustSubviews];

	// keyboard focus should start on the command field
	[window makeFirstResponder:commandField];

	// calculate the optimal size for everything
	{
		NSRect	available = [[NSScreen mainScreen] visibleFrame];
		NSRect	windowFrame = [window frame];
		NSSize	regCurrent = [regScroll frame].size;
		NSSize	regSize = [NSScrollView frameSizeForContentSize:[regView maximumFrameSize]
										 hasHorizontalScroller:YES
										   hasVerticalScroller:YES
													borderType:[regScroll borderType]];
		NSSize	dasmCurrent = [dasmScroll frame].size;
		NSSize	dasmSize = [NSScrollView frameSizeForContentSize:[dasmView maximumFrameSize]
										  hasHorizontalScroller:YES
											hasVerticalScroller:YES
													 borderType:[dasmScroll borderType]];
		NSSize	consoleCurrent = [consoleContainer frame].size;
		NSSize	consoleSize = [NSScrollView frameSizeForContentSize:[consoleView maximumFrameSize]
											 hasHorizontalScroller:YES
											   hasVerticalScroller:YES
														borderType:[consoleScroll borderType]];
		NSSize	adjustment;
		NSRect	lhsFrame, rhsFrame;

		consoleSize.width += consoleCurrent.width - [consoleScroll frame].size.width;
		consoleSize.height += consoleCurrent.height - [consoleScroll frame].size.height;
		adjustment.width = regSize.width - regCurrent.width;
		adjustment.height = regSize.height - regCurrent.height;
		adjustment.width += MAX(dasmSize.width - dasmCurrent.width, consoleSize.width - consoleCurrent.width);

		windowFrame.size.width += adjustment.width;
		windowFrame.size.height += adjustment.height; // not used - better to go for fixed height
		windowFrame.size.height = MIN(512.0, available.size.height);
		windowFrame.size.width = MIN(windowFrame.size.width, available.size.width);
		windowFrame.origin.x = available.origin.x + available.size.width - windowFrame.size.width;
		windowFrame.origin.y = available.origin.y;
		[window setFrame:windowFrame display:YES];

		lhsFrame = [regScroll frame];
		rhsFrame = [dasmSplit frame];
		adjustment.width = MIN(regSize.width, ([regSplit frame].size.width - [regSplit dividerThickness]) / 2);
		rhsFrame.origin.x -= lhsFrame.size.width - adjustment.width;
		rhsFrame.size.width += lhsFrame.size.width - adjustment.width;
		lhsFrame.size.width = adjustment.width;
		[regScroll setFrame:lhsFrame];
		[dasmSplit setFrame:rhsFrame];
	}

	// select the current processor
	[self setCPU:machine->firstcpu];

	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(auxiliaryWindowWillClose:)
												 name:MAMEAuxiliaryDebugWindowWillCloseNotification
											   object:nil];

	// don't forget the return value
	return self;
}


- (void)dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];

	if (history != nil)
		[history release];
	if (auxiliaryWindows != nil)
		[auxiliaryWindows release];

	[super dealloc];
}


- (void)setCPU:(running_device *)device {
	[regView selectSubviewForCPU:device];
	[dasmView selectSubviewForCPU:device];
	[window setTitle:[NSString stringWithFormat:@"Debug: %s - %@",
												device->machine->gamedrv->name,
												[regView selectedSubviewName]]];
}


- (IBAction)doCommand:(id)sender {
	NSString *command = [sender stringValue];
	if ([command length] == 0) {
		debug_cpu_single_step(machine, 1);
	} else {
		debug_console_execute_command(machine, [command UTF8String], 1);
		[history add:command];
	}
	[sender setStringValue:@""];
}


- (IBAction)debugNewMemoryWindow:(id)sender {
	MAMEMemoryViewer *win = [[MAMEMemoryViewer alloc] initWithMachine:machine console:self];
	[auxiliaryWindows addObject:win];
	[win release];
	[win activate];
}


- (IBAction)debugNewDisassemblyWindow:(id)sender {
	MAMEDisassemblyViewer *win = [[MAMEDisassemblyViewer alloc] initWithMachine:machine console:self];
	[auxiliaryWindows addObject:win];
	[win release];
	[win activate];
}


- (IBAction)debugNewErrorLogWindow:(id)sender {
	MAMEDisassemblyViewer *win = [[MAMEErrorLogViewer alloc] initWithMachine:machine console:self];
	[auxiliaryWindows addObject:win];
	[win release];
	[win activate];
}


- (void)showDebugger:(NSNotification *)notification {
	running_device *device = (running_device * )[[[notification userInfo] objectForKey:@"MAMEDebugDevice"] pointerValue];
	if (device->machine == machine) {
		[self setCPU:device];
		[window makeKeyAndOrderFront:self];
	}
}


- (void)auxiliaryWindowWillClose:(NSNotification *)notification {
	[auxiliaryWindows removeObjectIdenticalTo:[notification object]];
}


- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)command {
	if (control == commandField) {
		if (command == @selector(cancelOperation:)) {
			[commandField setStringValue:@""];
			[history reset];
			return YES;
		} else if (command == @selector(moveUp:)) {
			NSString *hist = [history previous:[commandField stringValue]];
			if (hist != nil) {
				[commandField setStringValue:hist];
				[commandField selectText:self];
				[(NSText *)[window firstResponder] setSelectedRange:NSMakeRange([hist length], 0)];
			}
			return YES;
		} else if (command == @selector(moveDown:)) {
			NSString *hist = [history next:[commandField stringValue]];
			if (hist != nil) {
				[commandField setStringValue:hist];
				[commandField selectText:self];
				[(NSText *)[window firstResponder] setSelectedRange:NSMakeRange([hist length], 0)];
			}
			return YES;
		}
    }
	return NO;
}


- (void)windowWillClose:(NSNotification *)notification {
	if ([notification object] != window)
		return;
	[[NSNotificationCenter defaultCenter] postNotificationName:MAMEHideDebuggerNotification object:self];
	debug_cpu_go(machine, ~0);
}


- (CGFloat)splitView:(NSSplitView *)sender constrainMinCoordinate:(CGFloat)min ofSubviewAt:(NSInteger)offs {
	return (min < 100) ? 100 : min;
}


- (CGFloat)splitView:(NSSplitView *)sender constrainMaxCoordinate:(CGFloat)max ofSubviewAt:(NSInteger)offs {
	NSSize	sz = [sender bounds].size;
	CGFloat	allowed = ([sender isVertical] ? sz.width : sz.height) - 100 - [sender dividerThickness];
	return (max > allowed) ? allowed : max;
}


- (BOOL)splitView:(NSSplitView *)sender canCollapseSubview:(NSView *)subview {
	// allow registers or disassembly to be collapsed, but not console
	return [[sender subviews] indexOfObjectIdenticalTo:subview] == 0;
}


- (void)splitView:(NSSplitView *)sender resizeSubviewsWithOldSize:(NSSize)oldSize {
	// This can only deal with a single split, but that's all we use, anyway
	NSRect first, second;
	[sender adjustSubviews];
	first = [[[sender subviews] objectAtIndex:0] frame];
	second = [[[sender subviews] objectAtIndex:1] frame];
	if ([sender isVertical]) {
		if (first.size.width < 100) {
			CGFloat diff = 100 - first.size.width;
			first.size.width = 100;
			second.origin.x += diff;
			second.size.width -= diff;
		} else if (second.size.width < 100) {
			CGFloat diff = 100 - second.size.width;
			second.size.width = 100;
			second.origin.x -= diff;
			first.size.width -= diff;
		}
	} else {
		if (first.size.height < 100) {
			CGFloat diff = 100 - first.size.height;
			first.size.height = 100;
			second.origin.y += diff;
			second.size.height -= diff;
		} else if (second.size.height < 100) {
			CGFloat diff = 100 - second.size.height;
			second.size.height = 100;
			second.origin.y -= diff;
			first.size.height -= diff;
		}
	}
	[[[sender subviews] objectAtIndex:0] setFrame:first];
	[[[sender subviews] objectAtIndex:1] setFrame:second];
}

@end


//============================================================
//  MAMEAuxiliaryDebugWindowHandler class
//============================================================

@implementation MAMEAuxiliaryDebugWindowHandler

+ (void)cascadeWindow:(NSWindow *)window {
	static NSPoint lastPosition = { 0, 0 };
	if (NSEqualPoints(lastPosition, NSZeroPoint)) {
		NSRect available = [[NSScreen mainScreen] visibleFrame];
		lastPosition = NSMakePoint(available.origin.x + 12, available.origin.y + available.size.height - 8);
	}
	lastPosition = [window cascadeTopLeftFromPoint:lastPosition];
}


- (id)initWithMachine:(running_machine *)m title:(NSString *)t console:(MAMEDebugConsole *)c {
	if (!(self = [super initWithMachine:m title:t]))
		return nil;
	console = c;
	return self;
}


- (void)dealloc {
	[super dealloc];
}


- (IBAction)debugNewMemoryWindow:(id)sender {
	[console debugNewMemoryWindow:sender];
}


- (IBAction)debugNewDisassemblyWindow:(id)sender {
	[console debugNewDisassemblyWindow:sender];
}


- (IBAction)debugNewErrorLogWindow:(id)sender {
	[console debugNewErrorLogWindow:sender];
}


- (void)windowWillClose:(NSNotification *)notification {
	[[NSNotificationCenter defaultCenter] postNotificationName:MAMEAuxiliaryDebugWindowWillCloseNotification
														object:self];
}

- (void)cascadeWindowWithDesiredSize:(NSSize)desired forView:(NSView *)view {
	NSRect	available = [[NSScreen mainScreen] visibleFrame];
	NSRect	windowFrame = [window frame];
	NSSize	current = [view frame].size;

	desired.width -= current.width;
	desired.height -= current.height;

	windowFrame.size.width += desired.width;
	windowFrame.size.height += desired.height;
	windowFrame.size.height = MIN(MIN(windowFrame.size.height, 240), available.size.height);
	windowFrame.size.width = MIN(windowFrame.size.width, available.size.width);
	windowFrame.origin.x = available.origin.x + available.size.width - windowFrame.size.width;
	windowFrame.origin.y = available.origin.y;
	[window setFrame:windowFrame display:YES];
	[[self class] cascadeWindow:window];

	windowFrame = [[window contentView] frame];
	desired = [window contentMinSize];
	[window setContentMinSize:NSMakeSize(MIN(windowFrame.size.width, desired.width),
										 MIN(windowFrame.size.height, desired.height))];
}

@end


//============================================================
//  MAMEExpreesionAuxiliaryDebugWindowHandler class
//============================================================

@implementation MAMEExpressionAuxiliaryDebugWindowHandler

- (id)initWithMachine:(running_machine *)m title:(NSString *)t console:(MAMEDebugConsole *)c {
	if (!(self = [super initWithMachine:m title:t console:c]))
		return nil;
	history = [[MAMEDebugCommandHistory alloc] init];
	return self;
}


- (void)dealloc {
	if (history != nil)
		[history release];
	[super dealloc];
}


- (id <MAMEDebugViewExpressionSupport>)documentView {
	return nil;
}


- (IBAction)doExpression:(id)sender {
	NSString *expr = [sender stringValue];
	if ([expr length] > 0) {
		[history add:expr];
		[[self documentView] setExpression:expr];
	} else {
		[sender setStringValue:[[self documentView] expression]];
		[history reset];
	}
	[sender selectText:self];
}


- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)command {
	if (control == expressionField) {
		if (command == @selector(cancelOperation:)) {
			[history reset];
			[expressionField setStringValue:[[self documentView] expression]];
			[expressionField selectText:self];
			return YES;
		} else if (command == @selector(moveUp:)) {
			NSString *hist = [history previous:[expressionField stringValue]];
			if (hist != nil) {
				[expressionField setStringValue:hist];
				[expressionField selectText:self];
			}
			return YES;
		} else if (command == @selector(moveDown:)) {
			NSString *hist = [history next:[expressionField stringValue]];
			if (hist != nil) {
				[expressionField setStringValue:hist];
				[expressionField selectText:self];
			}
			return YES;
		}
    }
	return NO;
}

@end


//============================================================
//  MAMEMemoryViewer class
//============================================================

@implementation MAMEMemoryViewer

- (id)initWithMachine:(running_machine *)m console:(MAMEDebugConsole *)c {
	NSScrollView	*memoryScroll;
	NSView			*expressionContainer;
	NSPopUpButton	*actionButton, *subviewButton;
	NSRect			contentBounds, expressionFrame;

	if (!(self = [super initWithMachine:m title:@"Memory" console:c]))
		return nil;
	contentBounds = [[window contentView] bounds];

	// create the expression field
	expressionField = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 100, 19)];
	[expressionField setAutoresizingMask:(NSViewWidthSizable | NSViewMaxXMargin | NSViewMinYMargin)];
	[expressionField setFont:[[MAMEDebugView class] defaultFont]];
	[expressionField setFocusRingType:NSFocusRingTypeNone];
	[expressionField setTarget:self];
	[expressionField setAction:@selector(doExpression:)];
	[expressionField setDelegate:self];
	expressionFrame = [expressionField frame];
	expressionFrame.size.width = (contentBounds.size.width - expressionFrame.size.height) / 2;
	[expressionField setFrameSize:expressionFrame.size];

	// create the subview popup
	subviewButton = [[NSPopUpButton alloc] initWithFrame:NSOffsetRect(expressionFrame,
																	  expressionFrame.size.width,
																	  0)];
	[subviewButton setAutoresizingMask:(NSViewWidthSizable | NSViewMinXMargin | NSViewMinYMargin)];
	[subviewButton setBezelStyle:NSShadowlessSquareBezelStyle];
	[subviewButton setFocusRingType:NSFocusRingTypeNone];
	[subviewButton setFont:[[MAMEDebugView class] defaultFont]];
	[subviewButton setTarget:self];
	[subviewButton setAction:@selector(changeSubview:)];
	[[subviewButton cell] setArrowPosition:NSPopUpArrowAtBottom];

	// create a container for the expression field and subview popup
	expressionFrame = NSMakeRect(expressionFrame.size.height,
								 contentBounds.size.height - expressionFrame.size.height,
								 contentBounds.size.width - expressionFrame.size.height,
								 expressionFrame.size.height);
	expressionContainer = [[NSView alloc] initWithFrame:expressionFrame];
	[expressionContainer setAutoresizingMask:(NSViewWidthSizable | NSViewMinYMargin)];
	[expressionContainer addSubview:expressionField];
	[expressionField release];
	[expressionContainer addSubview:subviewButton];
	[subviewButton release];
	[[window contentView] addSubview:expressionContainer];
	[expressionContainer release];

	// create the memory view
	memoryView = [[MAMEMemoryView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)
											   machine:machine];
	[memoryView insertSubviewItemsInMenu:[subviewButton menu] atIndex:0];
	memoryScroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0,
																  0,
																  contentBounds.size.width,
																  expressionFrame.origin.y)];
	[memoryScroll setDrawsBackground:YES];
	[memoryScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[memoryScroll setHasHorizontalScroller:YES];
	[memoryScroll setHasVerticalScroller:YES];
	[memoryScroll setAutohidesScrollers:YES];
	[memoryScroll setBorderType:NSNoBorder];
	[memoryScroll setDocumentView:memoryView];
	[memoryView release];
	[[window contentView] addSubview:memoryScroll];
	[memoryScroll release];

	// create the action popup
	actionButton = [[self class] newActionButtonWithFrame:NSMakeRect(0,
																	 expressionFrame.origin.y,
																	 expressionFrame.size.height,
																	 expressionFrame.size.height)];
	[actionButton setAutoresizingMask:(NSViewMaxXMargin | NSViewMinYMargin)];
	[memoryView insertActionItemsInMenu:[actionButton menu] atIndex:1];
	[[window contentView] addSubview:actionButton];
	[actionButton release];

	// set default state
	[memoryView selectSubviewForCPU:debug_cpu_get_visible_cpu(machine)];
	[memoryView setExpression:@"0"];
	[expressionField setStringValue:@"0"];
	[expressionField selectText:self];
	[subviewButton selectItemAtIndex:[subviewButton indexOfItemWithTag:[memoryView selectedSubviewIndex]]];
	[window makeFirstResponder:expressionField];
	[window setTitle:[NSString stringWithFormat:@"Memory: %@", [memoryView selectedSubviewName]]];

	// calculate the optimal size for everything
	{
		NSSize	desired = [NSScrollView frameSizeForContentSize:[memoryView maximumFrameSize]
										  hasHorizontalScroller:YES
											hasVerticalScroller:YES
													 borderType:[memoryScroll borderType]];
		[self cascadeWindowWithDesiredSize:desired forView:memoryScroll];
	}

	// don't forget the result
	return self;
}


- (void)dealloc {
	[super dealloc];
}


- (id <MAMEDebugViewExpressionSupport>)documentView {
	return memoryView;
}


- (IBAction)changeSubview:(id)sender {
	[memoryView selectSubviewAtIndex:[[sender selectedItem] tag]];
	[window setTitle:[NSString stringWithFormat:@"Memory: %@", [memoryView selectedSubviewName]]];
}

@end


//============================================================
//  MAMEDisassemblyViewer class
//============================================================

@implementation MAMEDisassemblyViewer

- (id)initWithMachine:(running_machine *)m console:(MAMEDebugConsole *)c {
	NSScrollView	*dasmScroll;
	NSView			*expressionContainer;
	NSPopUpButton	*actionButton, *subviewButton;
	NSRect			contentBounds, expressionFrame;

	if (!(self = [super initWithMachine:m title:@"Disassembly" console:c]))
		return nil;
	contentBounds = [[window contentView] bounds];

	// create the expression field
	expressionField = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 100, 19)];
	[expressionField setAutoresizingMask:(NSViewWidthSizable | NSViewMaxXMargin | NSViewMinYMargin)];
	[expressionField setFont:[[MAMEDebugView class] defaultFont]];
	[expressionField setFocusRingType:NSFocusRingTypeNone];
	[expressionField setTarget:self];
	[expressionField setAction:@selector(doExpression:)];
	[expressionField setDelegate:self];
	expressionFrame = [expressionField frame];
	expressionFrame.size.width = (contentBounds.size.width - expressionFrame.size.height) / 2;
	[expressionField setFrameSize:expressionFrame.size];

	// create the subview popup
	subviewButton = [[NSPopUpButton alloc] initWithFrame:NSOffsetRect(expressionFrame,
																	  expressionFrame.size.width,
																	  0)];
	[subviewButton setAutoresizingMask:(NSViewWidthSizable | NSViewMinXMargin | NSViewMinYMargin)];
	[subviewButton setBezelStyle:NSShadowlessSquareBezelStyle];
	[subviewButton setFocusRingType:NSFocusRingTypeNone];
	[subviewButton setFont:[[MAMEDebugView class] defaultFont]];
	[subviewButton setTarget:self];
	[subviewButton setAction:@selector(changeSubview:)];
	[[subviewButton cell] setArrowPosition:NSPopUpArrowAtBottom];

	// create a container for the expression field and subview popup
	expressionFrame = NSMakeRect(expressionFrame.size.height,
								 contentBounds.size.height - expressionFrame.size.height,
								 contentBounds.size.width - expressionFrame.size.height,
								 expressionFrame.size.height);
	expressionContainer = [[NSView alloc] initWithFrame:expressionFrame];
	[expressionContainer setAutoresizingMask:(NSViewWidthSizable | NSViewMinYMargin)];
	[expressionContainer addSubview:expressionField];
	[expressionField release];
	[expressionContainer addSubview:subviewButton];
	[subviewButton release];
	[[window contentView] addSubview:expressionContainer];
	[expressionContainer release];

	// create the disassembly view
	dasmView = [[MAMEDisassemblyView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)
												  machine:machine
											   useConsole:NO];
	[dasmView insertSubviewItemsInMenu:[subviewButton menu] atIndex:0];
	dasmScroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0,
																0,
																contentBounds.size.width,
																expressionFrame.origin.y)];
	[dasmScroll setDrawsBackground:YES];
	[dasmScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[dasmScroll setHasHorizontalScroller:YES];
	[dasmScroll setHasVerticalScroller:YES];
	[dasmScroll setAutohidesScrollers:YES];
	[dasmScroll setBorderType:NSNoBorder];
	[dasmScroll setDocumentView:dasmView];
	[dasmView release];
	[[window contentView] addSubview:dasmScroll];
	[dasmScroll release];

	// create the action popup
	actionButton = [[self class] newActionButtonWithFrame:NSMakeRect(0,
																	 expressionFrame.origin.y,
																	 expressionFrame.size.height,
																	 expressionFrame.size.height)];
	[actionButton setAutoresizingMask:(NSViewMaxXMargin | NSViewMinYMargin)];
	[dasmView insertActionItemsInMenu:[actionButton menu] atIndex:1];
	[[window contentView] addSubview:actionButton];
	[actionButton release];

	// set default state
	[dasmView selectSubviewForCPU:debug_cpu_get_visible_cpu(machine)];
	[dasmView setExpression:@"curpc"];
	[expressionField setStringValue:@"curpc"];
	[expressionField selectText:self];
	[subviewButton selectItemAtIndex:[subviewButton indexOfItemWithTag:[dasmView selectedSubviewIndex]]];
	[window makeFirstResponder:expressionField];
	[window setTitle:[NSString stringWithFormat:@"Disassembly: %@", [dasmView selectedSubviewName]]];

	// calculate the optimal size for everything
	{
		NSSize	desired = [NSScrollView frameSizeForContentSize:[dasmView maximumFrameSize]
										  hasHorizontalScroller:YES
											hasVerticalScroller:YES
													 borderType:[dasmScroll borderType]];
		[self cascadeWindowWithDesiredSize:desired forView:dasmScroll];
	}

	// don't forget the result
	return self;
}


- (void)dealloc {
	[super dealloc];
}


- (id <MAMEDebugViewExpressionSupport>)documentView {
	return dasmView;
}


- (IBAction)changeSubview:(id)sender {
	[dasmView selectSubviewAtIndex:[[sender selectedItem] tag]];
	[window setTitle:[NSString stringWithFormat:@"Disassembly: %@", [dasmView selectedSubviewName]]];
}

@end


//============================================================
//  MAMEErrorLogViewer class
//============================================================

@implementation MAMEErrorLogViewer

- (id)initWithMachine:(running_machine *)m console:(MAMEDebugConsole *)c {
	NSScrollView	*logScroll;
	NSString		*title;

	title = [NSString stringWithFormat:@"Error Log: %@ [%@]",
									   [NSString stringWithUTF8String:m->gamedrv->description],
									   [NSString stringWithUTF8String:m->gamedrv->name]];
	if (!(self = [super initWithMachine:m title:title console:c]))
		return nil;

	// create the error log view
	logView = [[MAMEErrorLogView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100) machine:machine];
	logScroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
	[logScroll setDrawsBackground:YES];
	[logScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[logScroll setHasHorizontalScroller:YES];
	[logScroll setHasVerticalScroller:YES];
	[logScroll setAutohidesScrollers:YES];
	[logScroll setBorderType:NSNoBorder];
	[logScroll setDocumentView:logView];
	[logView release];
	[window setContentView:logScroll];
	[logScroll release];

	// calculate the optimal size for everything
	{
		NSSize	desired = [NSScrollView frameSizeForContentSize:[logView maximumFrameSize]
										  hasHorizontalScroller:YES
											hasVerticalScroller:YES
													 borderType:[logScroll borderType]];

		// this thing starts with no content, so its prefered height may be very small
		desired.height = MAX(desired.height, 240);
		[self cascadeWindowWithDesiredSize:desired forView:logScroll];
	}

	// don't forget the result
	return self;
}


- (void)dealloc {
	[super dealloc];
}

@end
