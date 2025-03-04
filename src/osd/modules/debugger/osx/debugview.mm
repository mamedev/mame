// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  debugview.m - MacOS X Cocoa debug window handling
//
//============================================================

#import "debugview.h"

#include "emu.h"
#include "debugger.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"

#include "modules/lib/osdobj_common.h"

#include "util/xmlfile.h"

#include <cstring>


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

static NSCharacterSet *NonWhiteCharacters;


static void debugwin_view_update(debug_view &view, void *osdprivate)
{
	NSAutoreleasePool *const pool = [[NSAutoreleasePool alloc] init];
	[(MAMEDebugView *)osdprivate update];
	[pool release];
}


@implementation MAMEDebugView

+ (void)initialize {
	// 10.15 and better get full adaptive Dark Mode support
#if defined(MAC_OS_X_VERSION_10_15) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_15
	DefaultForeground = [[NSColor textColor] retain];
	ChangedForeground = [[NSColor systemRedColor] retain];
	CommentForeground = [[NSColor systemGreenColor] retain];
	// DCA_INVALID and DCA_DISABLED currently are not set by the core, so these 4 are unused
	InvalidForeground = [[NSColor colorWithCalibratedRed:0.0 green:0.0 blue:1.0 alpha:1.0] retain];
	DisabledChangedForeground = [[NSColor colorWithCalibratedRed:0.5 green:0.125 blue:0.125 alpha:1.0] retain];
	DisabledInvalidForeground = [[NSColor colorWithCalibratedRed:0.0 green:0.0 blue:0.5 alpha:1.0] retain];
	DisabledCommentForeground = [[NSColor colorWithCalibratedRed:0.0 green:0.25 blue:0.0 alpha:1.0] retain];

	DefaultBackground = [[NSColor textBackgroundColor] retain];
	VisitedBackground = [[NSColor systemTealColor] retain];
	AncillaryBackground = [[NSColor unemphasizedSelectedContentBackgroundColor] retain];
	SelectedBackground = [[NSColor selectedContentBackgroundColor] retain];
	CurrentBackground = [[NSColor selectedControlColor] retain];
	SelectedCurrentBackground = [[NSColor unemphasizedSelectedContentBackgroundColor] retain];
	InactiveSelectedBackground = [[NSColor unemphasizedSelectedContentBackgroundColor] retain];
	InactiveSelectedCurrentBackground = [[NSColor systemGrayColor] retain];
#else
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
#endif

	NonWhiteCharacters = [[[NSCharacterSet whitespaceAndNewlineCharacterSet] invertedSet] retain];
}


- (NSColor *)foregroundForAttribute:(uint8_t)attrib {
	if (attrib & DCA_COMMENT)
		return (attrib & DCA_DISABLED) ? DisabledCommentForeground : CommentForeground;
	else if (attrib & DCA_INVALID)
		return (attrib & DCA_DISABLED) ? DisabledInvalidForeground : InvalidForeground;
	else if (attrib & DCA_CHANGED)
		return (attrib & DCA_DISABLED) ? DisabledChangedForeground : ChangedForeground;
	else
		return DefaultForeground;
}


- (NSColor *)backgroundForAttribute:(uint8_t)attrib {
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
		int         attr = -1;
		NSUInteger  start = 0, length = 0;
		for (uint32_t col = origin.x; col < origin.x + size.x; col++)
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


- (void)convertBounds:(NSRect)b toFirstAffectedLine:(int32_t *)f count:(int32_t *)c {
	*f = lround(floor(b.origin.y / fontHeight));
	*c = lround(ceil((b.origin.y + b.size.height) / fontHeight)) - *f;
}


- (void)recomputeVisible {
	// this gets all the lines that are at least partially visible
	debug_view_xy origin(0, 0), size(totalWidth, totalHeight);
	[self convertBounds:[self visibleRect] toFirstAffectedLine:&origin.y count:&size.y];
	size.y = std::min(size.y, totalHeight - origin.y);

	// tell the underlying view how much real estate is available
	view->set_visible_size(size);
	view->set_visible_position(origin);
	originTop = origin.y;
}


- (void)typeCharacterAndScrollToCursor:(char)ch {
	debug_view_xy const oldPos = view->cursor_position();
	view->process_char(ch);
	if (view->cursor_supported())
	{
		debug_view_xy const newPos = view->cursor_position();
		if ((newPos.x != oldPos.x) || (newPos.y != oldPos.y))
		{
			// FIXME - use proper font metrics
			[self scrollRectToVisible:NSMakeRect((newPos.x * fontWidth) + [textContainer lineFragmentPadding],
												 newPos.y * fontHeight,
												 fontWidth,
												 fontHeight)];
		}
	}
}


- (void)adjustSizeAndRecomputeVisible {
	NSScrollView *const scroller = [self enclosingScrollView];
	NSSize const clip = [[scroller contentView] bounds].size;
	NSSize content = NSMakeSize((fontWidth * totalWidth) + (2 * [textContainer lineFragmentPadding]),
								fontHeight * totalHeight);
	if (wholeLineScroll)
		content.height += (fontHeight * 2) - 1;
	[self setFrameSize:NSMakeSize(ceil(std::max(clip.width, content.width)),
								  ceil(std::max(clip.height, content.height)))];
	[self recomputeVisible];
	[scroller reflectScrolledClipView:[scroller contentView]];
}


+ (NSFont *)defaultFontForMachine:(running_machine &)m {
	osd_options const &options = downcast<osd_options const &>(m.options());
	char const *const face = options.debugger_font();
	float const size = options.debugger_font_size();

	NSFont *const result = (('\0' != *face) && (0 != strcmp(OSDOPTVAL_AUTO, face)))
						 ? [NSFont fontWithName:[NSString stringWithUTF8String:face] size:MAX(0, size)]
						 : nil;

	return (nil != result) ? result : [NSFont userFixedPitchFontOfSize:MAX(0, size)];
}


- (id)initWithFrame:(NSRect)f type:(debug_view_type)t machine:(running_machine &)m wholeLineScroll:(BOOL)w {
	if (!(self = [super initWithFrame:f]))
		return nil;
	type = t;
	machine = &m;
	view = machine->debug_view().alloc_view((debug_view_type)type, debugwin_view_update, self);
	if (view == nil) {
		[self release];
		return nil;
	}
	wholeLineScroll = w;
	debug_view_xy const size = view->total_size();
	totalWidth = size.x;
	totalHeight = size.y;
	originTop = 0;

	text = [[NSTextStorage alloc] init];
	textContainer = [[NSTextContainer alloc] init];
	layoutManager = [[NSLayoutManager alloc] init];
	[layoutManager addTextContainer:textContainer];
	[textContainer release];
	[text addLayoutManager:layoutManager];
	[layoutManager release];

	[self setFont:[[self class] defaultFontForMachine:m]];

	NSMenu *contextMenu = [[NSMenu alloc] initWithTitle:@"Context"];
	[self addContextMenuItemsToMenu:contextMenu];
	[self setMenu:contextMenu];
	[contextMenu release];

	return self;
}


- (void)dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	if (view != nullptr) machine->debug_view().free_view(*view);
	if (font != nil) [font release];
	if (text != nil) [text release];
	[super dealloc];
}


- (void)update {
	// resize our frame if the total size has changed
	debug_view_xy const newSize = view->total_size();

	NSScrollView *const scroller = [self enclosingScrollView];
	if (scroller)
	{
		NSSize const clip = [[scroller contentView] bounds].size;
		NSSize content = NSMakeSize((fontWidth * newSize.x) + (2 * [textContainer lineFragmentPadding]),
									fontHeight * newSize.y);
		if (wholeLineScroll)
			content.height += (fontHeight * 2) - 1;
		[self setFrameSize:NSMakeSize(ceil(std::max(clip.width, content.width)),
									  ceil(std::max(clip.height, content.height)))];
		[scroller reflectScrolledClipView:[scroller contentView]];
	}

	totalWidth = newSize.x;
	totalHeight = newSize.y;

	// scroll the view if we're being told to
	debug_view_xy const newOrigin = view->visible_position();
	if (newOrigin.y != originTop)
	{
		NSRect const visible = [self visibleRect];
		NSPoint scroll = NSMakePoint(visible.origin.x, newOrigin.y * fontHeight);
		[self scrollPoint:scroll];
		originTop = newOrigin.y;
	}

	// mark as dirty
	[self setNeedsDisplay:YES];
}


- (NSSize)maximumFrameSize {
	debug_view_xy const max = view->total_size();
	return NSMakeSize(ceil((max.x * fontWidth) + (2 * [textContainer lineFragmentPadding])),
					  ceil((max.y + (wholeLineScroll ? 1 : 0)) * fontHeight));
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


- (BOOL)cursorSupported {
	return view->cursor_supported();
}


- (BOOL)cursorVisible {
	return view->cursor_visible();
}


- (debug_view_xy)cursorPosition {
	return view->cursor_position();
}


- (IBAction)copyVisible:(id)sender {
	debug_view_xy const size = view->visible_size();
	debug_view_char const *data = view->viewdata();
	if (!data)
	{
		NSBeep();
		return;
	}

	for (uint32_t row = 0; row < size.y; row++, data += size.x)
	{
		// add content for the line and set colours
		int         attr = -1;
		NSUInteger  start = [text length], length = start;
		for (uint32_t col = 0; col < size.x; col++)
		{
			[[text mutableString] appendFormat:@"%c", data[col].byte];
			if ((start < length) && (attr != (data[col].attrib & ~DCA_SELECTED)))
			{
				NSRange const run = NSMakeRange(start, length - start);
				[text addAttribute:NSForegroundColorAttributeName
							 value:[self foregroundForAttribute:attr]
							 range:run];
				[text addAttribute:NSBackgroundColorAttributeName
							 value:[self backgroundForAttribute:attr]
							 range:run];
				start = length;
			}
			attr = data[col].attrib & ~DCA_SELECTED;
			length = [text length];
		}

		// clean up trailing whitespace
		NSRange trim = [[text string] rangeOfCharacterFromSet:NonWhiteCharacters
													  options:NSBackwardsSearch
														range:NSMakeRange(start, length - start)];
		if (trim.location != NSNotFound)
			trim = [[text string] rangeOfComposedCharacterSequenceAtIndex:(trim.location + trim.length - 1)];
		else if (start > 0)
			trim = [[text string] rangeOfComposedCharacterSequenceAtIndex:(start - 1)];
		else
			trim = NSMakeRange(start, 0);
		trim.location += trim.length;
		trim.length = length - trim.location;
		[text deleteCharactersInRange:trim];

		// add the line ending and set colours
		[[text mutableString] appendString:@"\n"];
		NSRange const run = NSMakeRange(start, [text length] - start);
		[text addAttribute:NSForegroundColorAttributeName
					 value:[self foregroundForAttribute:attr]
					 range:run];
		[text addAttribute:NSBackgroundColorAttributeName
					 value:[self backgroundForAttribute:attr]
					 range:run];
	}

	// set the font and send it to the pasteboard
	NSRange const run = NSMakeRange(0, [text length]);
	[text addAttribute:NSFontAttributeName value:font range:run];
	NSPasteboard *const board = [NSPasteboard generalPasteboard];
	[board declareTypes:[NSArray arrayWithObject:NSPasteboardTypeRTF] owner:nil];
	[board setData:[text RTFFromRange:run documentAttributes:[NSDictionary dictionary]] forType:NSPasteboardTypeRTF];
	[text deleteCharactersInRange:run];
}


- (IBAction)paste:(id)sender {
	NSPasteboard *const board = [NSPasteboard generalPasteboard];
	NSString *const avail = [board availableTypeFromArray:[NSArray arrayWithObject:NSPasteboardTypeString]];
	if (avail == nil)
	{
		NSBeep();
		return;
	}

	NSData *const data = [[board stringForType:avail] dataUsingEncoding:NSASCIIStringEncoding
												   allowLossyConversion:YES];
	char const *const bytes = (char const *)[data bytes];
	debug_view_xy const oldPos = view->cursor_position();
	for (NSUInteger i = 0, l = [data length]; i < l; i++)
		view->process_char(bytes[i]);
	if (view->cursor_supported() && view->cursor_visible())
	{
		debug_view_xy const newPos = view->cursor_position();
		if ((newPos.x != oldPos.x) || (newPos.y != oldPos.y))
		{
			// FIXME - use proper font metrics
			[self scrollRectToVisible:NSMakeRect((newPos.x * fontWidth) + [textContainer lineFragmentPadding],
												 newPos.y * fontHeight,
												 fontWidth,
												 fontHeight)];
		}
	}
}


- (void)viewBoundsDidChange:(NSNotification *)notification {
	NSView *const changed = [notification object];
	if (changed == [[self enclosingScrollView] contentView])
		[self adjustSizeAndRecomputeVisible];
}


- (void)viewFrameDidChange:(NSNotification *)notification {
	NSView *const changed = [notification object];
	if (changed == [[self enclosingScrollView] contentView])
		[self adjustSizeAndRecomputeVisible];
}


- (void)windowDidBecomeKey:(NSNotification *)notification {
	NSWindow *const win = [notification object];
	if ((win == [self window]) && ([win firstResponder] == self) && view->cursor_supported())
		[self setNeedsDisplay:YES];
}


- (void)windowDidResignKey:(NSNotification *)notification {
	NSWindow *const win = [notification object];
	if ((win == [self window]) && ([win firstResponder] == self) && view->cursor_supported())
		[self setNeedsDisplay:YES];
}


- (void)addContextMenuItemsToMenu:(NSMenu *)menu {
	NSMenuItem  *item;

	item = [menu addItemWithTitle:@"Copy Visible"
						   action:@selector(copyVisible:)
					keyEquivalent:@""];
	[item setTarget:self];

	item = [menu addItemWithTitle:@"Paste"
						   action:@selector(paste:)
					keyEquivalent:@""];
	[item setTarget:self];
}


- (void)saveConfigurationToNode:(util::xml::data_node *)node {
	if (view->cursor_supported())
	{
		util::xml::data_node *const selection = node->add_child(osd::debugger::NODE_WINDOW_SELECTION, nullptr);
		if (selection)
		{
			debug_view_xy const pos = view->cursor_position();
			selection->set_attribute_int(osd::debugger::ATTR_SELECTION_CURSOR_VISIBLE, view->cursor_visible() ? 1 : 0);
			selection->set_attribute_int(osd::debugger::ATTR_SELECTION_CURSOR_X, pos.x);
			selection->set_attribute_int(osd::debugger::ATTR_SELECTION_CURSOR_Y, pos.y);
		}
	}

	util::xml::data_node *const scroll = node->add_child(osd::debugger::NODE_WINDOW_SCROLL, nullptr);
	if (scroll)
	{
		NSRect const visible = [self visibleRect];
		scroll->set_attribute_float(osd::debugger::ATTR_SCROLL_ORIGIN_X, visible.origin.x);
		scroll->set_attribute_float(osd::debugger::ATTR_SCROLL_ORIGIN_Y, visible.origin.y);
	}
}


- (void)restoreConfigurationFromNode:(util::xml::data_node const *)node {
	if (view->cursor_supported())
	{
		util::xml::data_node const *const selection = node->get_child(osd::debugger::NODE_WINDOW_SELECTION);
		if (selection)
		{
			debug_view_xy pos = view->cursor_position();
			view->set_cursor_visible(0 != selection->get_attribute_int(osd::debugger::ATTR_SELECTION_CURSOR_VISIBLE, view->cursor_visible() ? 1 : 0));
			pos.x = selection->get_attribute_int(osd::debugger::ATTR_SELECTION_CURSOR_X, pos.x);
			pos.y = selection->get_attribute_int(osd::debugger::ATTR_SELECTION_CURSOR_Y, pos.y);
			view->set_cursor_position(pos);
		}
	}

	util::xml::data_node const *const scroll = node->get_child(osd::debugger::NODE_WINDOW_SCROLL);
	if (scroll)
	{
		NSRect visible = [self visibleRect];
		visible.origin.x = scroll->get_attribute_float(osd::debugger::ATTR_SCROLL_ORIGIN_X, visible.origin.x);
		visible.origin.y = scroll->get_attribute_float(osd::debugger::ATTR_SCROLL_ORIGIN_Y, visible.origin.y);
		[self scrollRectToVisible:visible];
	}
}


- (BOOL)acceptsFirstResponder {
	return view->cursor_supported();
}


- (BOOL)becomeFirstResponder {
	if (view->cursor_supported())
	{
		view->set_cursor_visible(true);
		[self setNeedsDisplay:YES];
		return [super becomeFirstResponder];
	}
	else
	{
		return NO;
	}
}


- (BOOL)resignFirstResponder {
	if (view->cursor_supported())
		[self setNeedsDisplay:YES];
	return [super resignFirstResponder];
}


- (void)viewDidMoveToSuperview {
	[super viewDidMoveToSuperview];

	[[NSNotificationCenter defaultCenter] removeObserver:self
													name:NSViewBoundsDidChangeNotification
												  object:nil];
	[[NSNotificationCenter defaultCenter] removeObserver:self
													name:NSViewFrameDidChangeNotification
												  object:nil];

	NSScrollView *const scroller = [self enclosingScrollView];
	if (scroller != nil)
	{
		[scroller setLineScroll:fontHeight];
		[[scroller contentView] setPostsBoundsChangedNotifications:YES];
		[[scroller contentView] setPostsFrameChangedNotifications:YES];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(viewBoundsDidChange:)
													 name:NSViewBoundsDidChangeNotification
												   object:[scroller contentView]];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(viewFrameDidChange:)
													 name:NSViewFrameDidChangeNotification
												   object:[scroller contentView]];
		[self adjustSizeAndRecomputeVisible];
	}
}


- (void)viewDidMoveToWindow {
	[super viewDidMoveToWindow];

	[[NSNotificationCenter defaultCenter] removeObserver:self
													name:NSWindowDidBecomeKeyNotification
												  object:nil];
	[[NSNotificationCenter defaultCenter] removeObserver:self
													name:NSWindowDidResignKeyNotification
												  object:nil];

	if ([self window] != nil)
	{
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(windowDidBecomeKey:)
													 name:NSWindowDidBecomeKeyNotification
												   object:[self window]];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(windowDidResignKey:)
													 name:NSWindowDidResignKeyNotification
												   object:[self window]];
		[self setNeedsDisplay:YES];
	}
}


- (BOOL)isFlipped {
	return YES;
}


- (BOOL)isOpaque {
	return YES;
}


- (NSRect)adjustScroll:(NSRect)proposedVisibleRect {
	if (wholeLineScroll)
	{
		CGFloat const clamp = [self bounds].size.height - fontHeight - proposedVisibleRect.size.height;
		proposedVisibleRect.origin.y = std::min(proposedVisibleRect.origin.y, std::max(clamp, CGFloat(0)));
		proposedVisibleRect.origin.y -= fmod(proposedVisibleRect.origin.y, fontHeight);
	}
	return proposedVisibleRect;
}


- (void)drawRect:(NSRect)dirtyRect {
	// work out what's available
	[self recomputeVisible];
	debug_view_xy const origin = view->visible_position();
	debug_view_xy const size = view->visible_size();

	// work out how much we need to draw
	int32_t row, clip;
	[self convertBounds:dirtyRect toFirstAffectedLine:&row count:&clip];
	clip += row;
	row = std::max(row, origin.y);
	clip = std::min(clip, origin.y + size.y);

	// this gets the text for the whole visible area
	debug_view_char const *data = view->viewdata();
	if (!data)
		return;

	// clear any space above the available content
	data += ((row - origin.y) * size.x);
	if (dirtyRect.origin.y < (row * fontHeight))
	{
		[DefaultBackground set];
		[NSBezierPath fillRect:NSMakeRect(0,
										  dirtyRect.origin.y,
										  [self bounds].size.width,
										  (row * fontHeight) - dirtyRect.origin.y)];
	}

	// render entire lines to get character alignment right
	for ( ; row < clip; row++, data += size.x)
	{
		int         attr = -1;
		NSUInteger  start = 0, length = 0;
		for (uint32_t col = origin.x; col < origin.x + size.x; col++)
		{
			[[text mutableString] appendFormat:@"%C", unichar(data[col - origin.x].byte)];
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
				NSRect box = [layoutManager boundingRectForGlyphRange:glyphs
													  inTextContainer:textContainer];
				if (start == 0)
				{
					box.size.width += box.origin.x;
					box.origin.x = 0;
				}
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
		NSRange const run = NSMakeRange(start, length - start);
		[text addAttribute:NSFontAttributeName
					 value:font
					 range:NSMakeRange(0, length)];
		[text addAttribute:NSForegroundColorAttributeName
					 value:[self foregroundForAttribute:attr]
					 range:run];
		NSRange const glyphs = [layoutManager glyphRangeForCharacterRange:run
													 actualCharacterRange:NULL];
		NSRect box = [layoutManager boundingRectForGlyphRange:glyphs
											  inTextContainer:textContainer];
		if (start == 0)
			box.origin.x = 0;
		box.size.width = std::max([self bounds].size.width - box.origin.x, CGFloat(0));
		[[self backgroundForAttribute:attr] set];
		[NSBezierPath fillRect:NSMakeRect(box.origin.x,
										  row * fontHeight,
										  box.size.width,
										  fontHeight)];
		[layoutManager drawGlyphsForGlyphRange:[layoutManager glyphRangeForTextContainer:textContainer]
									   atPoint:NSMakePoint(0, row * fontHeight)];
		[text deleteCharactersInRange:NSMakeRange(0, length)];
	}

	// clear any space below the available content
	if ((dirtyRect.origin.y + dirtyRect.size.height) > (row * fontHeight))
	{
		[DefaultBackground set];
		[NSBezierPath fillRect:NSMakeRect(0,
										  row * fontHeight,
										  [self bounds].size.width,
										  (dirtyRect.origin.y + dirtyRect.size.height) - (row * fontHeight))];
	}
}


- (void)mouseDown:(NSEvent *)event {
	NSPoint const location = [self convertPoint:[event locationInWindow] fromView:nil];
	NSUInteger const modifiers = [event modifierFlags];
	view->process_click(((modifiers & NSEventModifierFlagCommand) && [[self window] isMainWindow]) ? DCK_RIGHT_CLICK
					  : (modifiers & NSEventModifierFlagOption) ? DCK_MIDDLE_CLICK
					  : DCK_LEFT_CLICK,
						[self convertLocation:location]);
}


- (void)mouseDragged:(NSEvent *)event {
	[self autoscroll:event];
	NSPoint const location = [self convertPoint:[event locationInWindow] fromView:nil];
	NSUInteger const modifiers = [event modifierFlags];
	if (view->cursor_supported()
	 && !(modifiers & NSEventModifierFlagOption)
	 && (!(modifiers & NSEventModifierFlagCommand) || ![[self window] isMainWindow]))
	{
		view->set_cursor_position([self convertLocation:location]);
		view->set_cursor_visible(true);
		[self setNeedsDisplay:YES];
	}
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
	NSUInteger  modifiers = [event modifierFlags];
	NSString    *str = [event charactersIgnoringModifiers];

	if ([str length] == 1)
	{
		if (modifiers & NSEventModifierFlagNumericPad)
		{
			switch ([str characterAtIndex:0])
			{
			case NSUpArrowFunctionKey:
				if (modifiers & NSEventModifierFlagCommand)
					view->process_char(DCH_CTRLHOME);
				else
					view->process_char(DCH_UP);
				return;
			case NSDownArrowFunctionKey:
				if (modifiers & NSEventModifierFlagCommand)
					view->process_char(DCH_CTRLEND);
				else
					view->process_char(DCH_DOWN);
				return;
			case NSLeftArrowFunctionKey:
				if (modifiers & NSEventModifierFlagCommand)
					[self typeCharacterAndScrollToCursor:DCH_HOME];
				else if (modifiers & NSEventModifierFlagOption)
					[self typeCharacterAndScrollToCursor:DCH_CTRLLEFT];
				else
					[self typeCharacterAndScrollToCursor:DCH_LEFT];
				return;
			case NSRightArrowFunctionKey:
				if (modifiers & NSEventModifierFlagCommand)
					[self typeCharacterAndScrollToCursor:DCH_END];
				else if (modifiers & NSEventModifierFlagOption)
					[self typeCharacterAndScrollToCursor:DCH_CTRLRIGHT];
				else
					[self typeCharacterAndScrollToCursor:DCH_RIGHT];
				return;
			default:
				[self interpretKeyEvents:[NSArray arrayWithObject:event]];
				return;
			}
		}
		else if (modifiers & NSEventModifierFlagFunction)
		{
			switch ([str characterAtIndex:0])
			{
				case NSPageUpFunctionKey:
					if (modifiers & NSEventModifierFlagOption)
					{
						view->process_char(DCH_PUP);
						return;
					}
				case NSPageDownFunctionKey:
					if (modifiers & NSEventModifierFlagOption)
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


- (void)keyUp:(NSEvent *)event {
	if (view->cursor_supported() && view->cursor_visible())
	{
		debug_view_xy const pos = view->cursor_position();
		[self scrollRectToVisible:NSMakeRect((pos.x * fontWidth) + [textContainer lineFragmentPadding],
											 pos.y * fontHeight,
											 fontWidth,
											 fontHeight)]; // FIXME: metrics
	}
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
	machine->debugger().console().get_visible_cpu()->debug()->single_step();
}


- (void)insertText:(id)string {
	NSUInteger  len;
	NSRange     found;
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


- (BOOL)validateMenuItem:(NSMenuItem *)item {
	SEL action = [item action];

	if (action == @selector(paste:))
	{
		NSPasteboard *const board = [NSPasteboard generalPasteboard];
		return [board availableTypeFromArray:[NSArray arrayWithObject:NSPasteboardTypeString]] != nil;
	}
	else
	{
		return YES;
	}
}

@end
