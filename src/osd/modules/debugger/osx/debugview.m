// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  debugview.m - MacOS X Cocoa debug window handling
//
//  Copyright (c) 1996-2015, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#import "debugview.h"

#include "debug/debugcpu.h"


static NSColor *DefaultForeground;
static NSColor *ChangedForeground;
static NSColor *InvalidForeground;
static NSColor *CommentForeground;
static NSColor *DisabledChangedForeground;
static NSColor *DisabledInvalidForeground;
static NSColor *DisabledCommentForeground;

static NSColor *DefaultBackground;
static NSColor *VisitedBackground;
static NSColor *AncillaryBackground;
static NSColor *SelectedBackground;
static NSColor *CurrentBackground;
static NSColor *SelectedCurrentBackground;
static NSColor *InactiveSelectedBackground;
static NSColor *InactiveSelectedCurrentBackground;


static void debugwin_view_update(debug_view &view, void *osdprivate)
{
	[(MAMEDebugView *)osdprivate update];
}


@implementation MAMEDebugView

+ (void)initialize {
	DefaultForeground = [[NSColor colorWithCalibratedWhite:0.0 alpha:1.0] retain];
	ChangedForeground = [[NSColor colorWithCalibratedRed:0.875 green:0.0 blue:0.0 alpha:1.0] retain];
	InvalidForeground = [[NSColor colorWithCalibratedRed:0.0 green:0.0 blue:1.0 alpha:1.0] retain];
	CommentForeground = [[NSColor colorWithCalibratedRed:0.0 green:0.375 blue:0.0 alpha:1.0] retain];
	DisabledChangedForeground = [[NSColor colorWithCalibratedRed:0.5 green:0.125 blue:0.125 alpha:1.0] retain];
	DisabledInvalidForeground = [[NSColor colorWithCalibratedRed:0.0 green:0.0 blue:0.5 alpha:1.0] retain];
	DisabledCommentForeground = [[NSColor colorWithCalibratedRed:0.0 green:0.25 blue:0.0 alpha:1.0] retain];

	DefaultBackground = [[NSColor colorWithCalibratedWhite:1.0 alpha:1.0] retain];
	VisitedBackground = [[NSColor colorWithCalibratedRed:0.75 green:1.0 blue:0.75 alpha:1.0] retain];
	AncillaryBackground = [[NSColor colorWithCalibratedWhite:0.75 alpha:1.0] retain];
	SelectedBackground = [[NSColor colorWithCalibratedRed:0.75 green:0.875 blue:1.0 alpha:1.0] retain];
	CurrentBackground = [[NSColor colorWithCalibratedRed:1.0 green:0.75 blue:0.75 alpha:1.0] retain];
	SelectedCurrentBackground = [[NSColor colorWithCalibratedRed:0.875 green:0.625 blue:0.875 alpha:1.0] retain];
	InactiveSelectedBackground = [[NSColor colorWithCalibratedWhite:0.875 alpha:1.0] retain];
	InactiveSelectedCurrentBackground = [[NSColor colorWithCalibratedRed:0.875 green:0.5 blue:0.625 alpha:1.0] retain];
}


- (NSColor *)foregroundForAttribute:(UINT8)attrib {
	if (attrib & DCA_COMMENT)
		return (attrib & DCA_DISABLED) ? DisabledCommentForeground : CommentForeground;
	else if (attrib & DCA_INVALID)
		return (attrib & DCA_DISABLED) ? DisabledInvalidForeground : InvalidForeground;
	else if (attrib & DCA_CHANGED)
		return (attrib & DCA_DISABLED) ? DisabledChangedForeground : ChangedForeground;
	else
		return DefaultForeground;
}


- (NSColor *)backgroundForAttribute:(UINT8)attrib {
	BOOL const active = [[self window] isKeyWindow] && ([[self window] firstResponder] == self);
	if ((attrib & DCA_SELECTED) && (attrib & DCA_CURRENT))
		return active ? SelectedCurrentBackground : InactiveSelectedCurrentBackground;
	else if (attrib & DCA_CURRENT)
		return CurrentBackground;
	else if (attrib & DCA_SELECTED)
		return active ? SelectedBackground : InactiveSelectedBackground;
	else if (attrib & DCA_ANCILLARY)
		return AncillaryBackground;
	else if (attrib & DCA_VISITED)
		return VisitedBackground;
	else
		return DefaultBackground;
}


- (debug_view_xy)convertLocation:(NSPoint)location {
	debug_view_xy position;

	position.y = lround(floor(location.y / fontHeight));
	if (position.y < 0)
		position.y = 0;
	else if (position.y >= totalHeight)
		position.y = totalHeight - 1;

	debug_view_xy const origin = view->visible_position();
	debug_view_xy const size = view->visible_size();
	debug_view_char const *data = view->viewdata();
	if (!data || (position.y < origin.y) || (position.y >= origin.y + size.y))
	{
		// y coordinate outside visible area, x will be a guess
		position.x = lround(floor((location.x - [textContainer lineFragmentPadding]) / fontWidth));
	}
	else
	{
		data += ((position.y - view->visible_position().y) * view->visible_size().x);
		int			attr = -1;
		NSUInteger	start = 0, length = 0;
		for (UINT32 col = origin.x; col < origin.x + size.x; col++)
		{
			[[text mutableString] appendFormat:@"%c", data[col - origin.x].byte];
			if ((start < length) && (attr != data[col - origin.x].attrib))
			{
				NSRange const run = NSMakeRange(start, length - start);
				[text addAttribute:NSFontAttributeName
							 value:font
							 range:NSMakeRange(0, length)];
				[text addAttribute:NSForegroundColorAttributeName
							 value:[self foregroundForAttribute:attr]
							 range:run];
				start = length;
			}
			attr = data[col - origin.x].attrib;
			length = [text length];
		}
		if (start < length)
		{
			NSRange const run = NSMakeRange(start, length - start);
			[text addAttribute:NSFontAttributeName
						 value:font
						 range:NSMakeRange(0, length)];
			[text addAttribute:NSForegroundColorAttributeName
						 value:[self foregroundForAttribute:attr]
						 range:run];
		}
		CGFloat fraction;
		NSUInteger const glyph = [layoutManager glyphIndexForPoint:NSMakePoint(location.x, fontHeight / 2)
												   inTextContainer:textContainer
									fractionOfDistanceThroughGlyph:&fraction];
		position.x = [layoutManager characterIndexForGlyphAtIndex:glyph]; // FIXME: assumes 1:1 character mapping
		[text deleteCharactersInRange:NSMakeRange(0, length)];
	}
	if (position.x < 0)
		position.x = 0;
	else if (position.x >= totalWidth)
		position.x = totalWidth - 1;

	return position;
}


- (void)convertBounds:(NSRect)b toFirstAffectedLine:(INT32 *)f count:(INT32 *)c {
	*f = lround(floor(b.origin.y / fontHeight));
	*c = lround(ceil((b.origin.y + b.size.height) / fontHeight)) - *f;
}


- (void)recomputeVisible {
	if ([self window] != nil)
	{
		// this gets all the lines that are at least partially visible
		debug_view_xy origin(0, 0), size(totalWidth, totalHeight);
		[self convertBounds:[self visibleRect] toFirstAffectedLine:&origin.y count:&size.y];

		// tell them what we think
		view->set_visible_size(size);
		view->set_visible_position(origin);
		originLeft = origin.x;
		originTop = origin.y;
	}
}


- (void)typeCharacterAndScrollToCursor:(char)ch {
	if (view->cursor_supported())
	{
		debug_view_xy const oldPos = view->cursor_position();
		view->process_char(ch);
		debug_view_xy const newPos = view->cursor_position();
		if ((newPos.x != oldPos.x) || (newPos.y != oldPos.y))
		{
			// FIXME - use proper font metrics
			[self scrollRectToVisible:NSMakeRect((newPos.x * fontWidth) + [textContainer lineFragmentPadding],
												 newPos.y * fontHeight,
												 fontWidth,
												 fontHeight)];
		}
	} else {
		view->process_char(ch);
	}
}


+ (NSFont *)defaultFont {
	return [NSFont userFixedPitchFontOfSize:0];
}


- (id)initWithFrame:(NSRect)f type:(debug_view_type)t machine:(running_machine &)m {
	if (!(self = [super initWithFrame:f]))
		return nil;
	type = t;
	machine = &m;
	view = machine->debug_view().alloc_view((debug_view_type)type, debugwin_view_update, self);
	if (view == nil) {
		[self release];
		return nil;
	}
	totalWidth = totalHeight = 0;
	originLeft = originTop = 0;

	text = [[NSTextStorage alloc] init];
	textContainer = [[NSTextContainer alloc] init];
	layoutManager = [[NSLayoutManager alloc] init];
	[layoutManager addTextContainer:textContainer];
	[textContainer release];
	[text addLayoutManager:layoutManager];
	[layoutManager release];

	[self setFont:[[self class] defaultFont]];

	return self;
}


- (void)dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	if (font != nil) [font release];
	if (text != nil) [text release];
	[super dealloc];
}


- (void)update {
	// resize our frame if the total size has changed
	debug_view_xy const newSize = view->total_size();
	BOOL const resized = (newSize.x != totalWidth) || (newSize.y != totalHeight);
	if (resized)
	{
		[self setFrameSize:NSMakeSize((fontWidth * newSize.x) + (2 * [textContainer lineFragmentPadding]),
									  fontHeight * newSize.y)];
		totalWidth = newSize.x;
		totalHeight = newSize.y;
	}

	// scroll the view if we're being told to
	debug_view_xy const newOrigin = view->visible_position();
	if (newOrigin.y != originTop)
	{
		[self scrollPoint:NSMakePoint([self visibleRect].origin.x, newOrigin.y * fontHeight)];
		originTop = newOrigin.y;
	}

	// recompute the visible area and mark as dirty
	[self recomputeVisible];
	[self setNeedsDisplay:YES];
}


- (NSSize)maximumFrameSize {
	debug_view_xy const max = view->total_size();
	return NSMakeSize((max.x * fontWidth) + (2 * [textContainer lineFragmentPadding]),
					   max.y * fontHeight);
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
	totalWidth = totalHeight = 0;
	[self update];
}


- (void)windowDidBecomeKey:(NSNotification *)notification {
	NSWindow *win = [notification object];
	if ((win == [self window]) && ([win firstResponder] == self) && view->cursor_supported())
		[self setNeedsDisplay:YES];
}


- (void)windowDidResignKey:(NSNotification *)notification {
	NSWindow *win = [notification object];
	if ((win == [self window]) && ([win firstResponder] == self) && view->cursor_supported())
		[self setNeedsDisplay:YES];
}


- (BOOL)acceptsFirstResponder {
	return view->cursor_supported();
}


- (BOOL)becomeFirstResponder {
	if (view->cursor_supported()) {
		debug_view_xy pos;
		view->set_cursor_visible(true);
		pos = view->cursor_position();
		[self scrollRectToVisible:NSMakeRect((pos.x * fontWidth) + [textContainer lineFragmentPadding], pos.y * fontHeight, fontWidth, fontHeight)]; // FIXME: metrics
		[self setNeedsDisplay:YES];
		return [super becomeFirstResponder];
	} else {
		return NO;
	}
}


- (BOOL)resignFirstResponder {
	if (view->cursor_supported())
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
	INT32 position, clip;

	// work out how much we need to draw
	[self recomputeVisible];
	debug_view_xy const origin = view->visible_position();
	debug_view_xy const size = view->visible_size();
	[self convertBounds:dirtyRect toFirstAffectedLine:&position count:&clip];

	// this gets the text for the whole visible area
	debug_view_char const *data = view->viewdata();
	if (!data)
		return;

	data += ((position - origin.y) * size.x);
	for (UINT32 row = position; row < position + clip; row++, data += size.x)
	{
		if ((row < origin.y) || (row >= origin.y + size.y))
			continue;

		// render entire lines to get character alignment right
		int			attr = -1;
		NSUInteger	start = 0, length = 0;
		for (UINT32 col = origin.x; col < origin.x + size.x; col++)
		{
			[[text mutableString] appendFormat:@"%c", data[col - origin.x].byte];
			if ((start < length) && (attr != data[col - origin.x].attrib))
			{
				NSRange const run = NSMakeRange(start, length - start);
				[text addAttribute:NSFontAttributeName
							 value:font
							 range:NSMakeRange(0, length)];
				[text addAttribute:NSForegroundColorAttributeName
							 value:[self foregroundForAttribute:attr]
							 range:run];
				NSRange const glyphs = [layoutManager glyphRangeForCharacterRange:run
															 actualCharacterRange:NULL];
				NSRect const box = [layoutManager boundingRectForGlyphRange:glyphs
															inTextContainer:textContainer];
				[[self backgroundForAttribute:attr] set];
				[NSBezierPath fillRect:NSMakeRect(box.origin.x,
												  row * fontHeight,
												  box.size.width,
												  fontHeight)];
				start = length;
			}
			attr = data[col - origin.x].attrib;
			length = [text length];
		}
		if (start < length)
		{
			NSRange const run = NSMakeRange(start, length - start);
			[text addAttribute:NSFontAttributeName
						 value:font
						 range:NSMakeRange(0, length)];
			[text addAttribute:NSForegroundColorAttributeName
						 value:[self foregroundForAttribute:attr]
						 range:run];
			NSRange const glyphs = [layoutManager glyphRangeForCharacterRange:run
														 actualCharacterRange:NULL];
			NSRect const box = [layoutManager boundingRectForGlyphRange:glyphs
														inTextContainer:textContainer];
			[[self backgroundForAttribute:attr] set];
			[NSBezierPath fillRect:NSMakeRect(box.origin.x,
											  row * fontHeight,
											  box.size.width,
											  fontHeight)];
		}
		[layoutManager drawGlyphsForGlyphRange:[layoutManager glyphRangeForTextContainer:textContainer]
									   atPoint:NSMakePoint(0, row * fontHeight)];
		[text deleteCharactersInRange:NSMakeRange(0, length)];
	}
}


- (void)mouseDown:(NSEvent *)event {
	NSPoint const location = [self convertPoint:[event locationInWindow] fromView:nil];
	NSUInteger const modifiers = [event modifierFlags];
	view->process_click((modifiers & NSCommandKeyMask) ? DCK_RIGHT_CLICK
					  : (modifiers & NSAlternateKeyMask) ? DCK_MIDDLE_CLICK
					  : DCK_LEFT_CLICK,
						[self convertLocation:location]);
	[self setNeedsDisplay:YES];
}


- (void)rightMouseDown:(NSEvent *)event {
	NSPoint const location = [self convertPoint:[event locationInWindow] fromView:nil];
	if (view->cursor_supported())
	{
		view->set_cursor_position([self convertLocation:location]);
		view->set_cursor_visible(true);
		[self setNeedsDisplay:YES];
	}
	[super rightMouseDown:event];
}


- (void)keyDown:(NSEvent *)event {
	NSUInteger	modifiers = [event modifierFlags];
	NSString	*str = [event charactersIgnoringModifiers];

	if ([str length] == 1)
	{
		if (modifiers & NSNumericPadKeyMask)
		{
			switch ([str characterAtIndex:0])
			{
			case NSUpArrowFunctionKey:
				if (modifiers & NSCommandKeyMask)
					view->process_char(DCH_CTRLHOME);
				else
					view->process_char(DCH_UP);
				return;
			case NSDownArrowFunctionKey:
				if (modifiers & NSCommandKeyMask)
					view->process_char(DCH_CTRLEND);
				else
					view->process_char(DCH_DOWN);
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
		}
		else if (modifiers & NSFunctionKeyMask)
		{
			switch ([str characterAtIndex:0])
			{
				case NSPageUpFunctionKey:
					if (modifiers & NSAlternateKeyMask)
					{
						view->process_char(DCH_PUP);
						return;
					}
				case NSPageDownFunctionKey:
					if (modifiers & NSAlternateKeyMask)
					{
						view->process_char(DCH_PDOWN);
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
	debug_cpu_get_visible_cpu(*machine)->debug()->single_step();
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
