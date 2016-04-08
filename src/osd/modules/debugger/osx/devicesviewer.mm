// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  devicesviewer.m - MacOS X Cocoa debug window handling
//
//============================================================

#import "devicesviewer.h"

#import "debugconsole.h"


@interface MAMEDeviceWrapper : NSObject
{
	running_machine	*machine;
	device_t		*device;
	NSArray			*children;
}

- (id)initWithMachine:(running_machine &)m device:(device_t &)d;

- (running_machine &)machine;
- (device_t &)device;
- (NSString *)tag;
- (NSString *)name;
- (NSUInteger)children;
- (MAMEDeviceWrapper *)childAtIndex:(NSUInteger)index;

@end


@implementation MAMEDeviceWrapper

- (void)wrapChildren {
	NSMutableArray *const tmp = [[NSMutableArray alloc] init];
	for (device_t *child = device->subdevices().first(); child != NULL; child = child->next())
	{
		MAMEDeviceWrapper *const wrap = [[MAMEDeviceWrapper alloc] initWithMachine:*machine 
																			device:*child];
		[tmp addObject:wrap];
		[wrap release];
	}
	children = [[NSArray alloc] initWithArray:tmp];
	[tmp release];
}


- (id)initWithMachine:(running_machine &)m device:(device_t &)d {
	if (!(self = [super init]))
		return nil;
	machine = &m;
	device = &d;
	children = nil;
	return self;
}


- (void)dealloc {
	if (children != nil)
		[children release];
	[super dealloc];
}


- (running_machine &)machine {
	return *machine;
}


- (device_t &)device {
	return *device;
}


- (NSString *)tag {
	return (device == &machine->root_device()) ? @"<root>"
											   : [NSString stringWithUTF8String:device->basetag()];
}


- (NSString *)name {
	return [NSString stringWithUTF8String:device->name()];
}


- (NSUInteger)children {
	if (children == nil)
		[self wrapChildren];
	return [children count];
}


- (MAMEDeviceWrapper *)childAtIndex:(NSUInteger)index {
	if (children == nil)
		[self wrapChildren];
	return (index < [children count]) ? [children objectAtIndex:index] : nil;
}

@end


@implementation MAMEDevicesViewer

- (id)initWithMachine:(running_machine &)m console:(MAMEDebugConsole *)c {
	NSScrollView	*devicesScroll;
	NSTableColumn	*tagColumn, *nameColumn;

	if (!(self = [super initWithMachine:m title:@"All Devices" console:c]))
		return nil;
	root = [[MAMEDeviceWrapper alloc] initWithMachine:m device:m.root_device()];

	// create the devices view
	devicesView = [[NSOutlineView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
	[devicesView setUsesAlternatingRowBackgroundColors:YES];
	[devicesView setAllowsColumnReordering:YES];
	[devicesView setAllowsColumnResizing:YES];
	[devicesView setColumnAutoresizingStyle:NSTableViewUniformColumnAutoresizingStyle];
	[devicesView setAllowsEmptySelection:YES];
	[devicesView setAllowsMultipleSelection:NO];
	[devicesView setAllowsColumnSelection:NO];
	tagColumn = [[NSTableColumn alloc] initWithIdentifier:@"tag"];
	[[tagColumn headerCell] setStringValue:@"Tag"];
	[tagColumn setEditable:NO];
	[tagColumn setMinWidth:100];
	[tagColumn setWidth:120];
	[tagColumn setResizingMask:(NSTableColumnAutoresizingMask | NSTableColumnUserResizingMask)];
	[devicesView addTableColumn:tagColumn];
	[tagColumn release];
	nameColumn = [[NSTableColumn alloc] initWithIdentifier:@"name"];
	[[nameColumn headerCell] setStringValue:@"Name"];
	[nameColumn setEditable:NO];
	[nameColumn setMinWidth:100];
	[nameColumn setMinWidth:360];
	[nameColumn setResizingMask:(NSTableColumnAutoresizingMask | NSTableColumnUserResizingMask)];
	[devicesView addTableColumn:nameColumn];
	[nameColumn release];
	[devicesView setOutlineTableColumn:tagColumn];
	[devicesView setAutoresizesOutlineColumn:YES];
	[devicesView setDoubleAction:@selector(showDeviceDetail:)];
	[devicesView setDataSource:self];
	devicesScroll = [[NSScrollView alloc] initWithFrame:[[window contentView] bounds]];
	[devicesScroll setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[devicesScroll setHasHorizontalScroller:YES];
	[devicesScroll setHasVerticalScroller:YES];
	[devicesScroll setAutohidesScrollers:YES];
	[devicesScroll setBorderType:NSNoBorder];
	[devicesScroll setDocumentView:devicesView];
	[devicesView release];
	[[window contentView] addSubview:devicesScroll];
	[devicesScroll release];

	// set default state
	[devicesView expandItem:root expandChildren:YES];
	[window makeFirstResponder:devicesView];
	[window setTitle:@"All Devices"];

	// calculate the optimal size for everything
	NSSize const desired = [NSScrollView frameSizeForContentSize:NSMakeSize(480, 320)
										   hasHorizontalScroller:YES
											 hasVerticalScroller:YES
													  borderType:[devicesScroll borderType]];
	[self cascadeWindowWithDesiredSize:desired forView:devicesScroll];

	// don't forget the result
	return self;
}


- (void)dealloc {
	if (root != nil)
		[root release];
	[super dealloc];
}


- (IBAction)showDeviceDetail:(id)sender {
	[console debugNewInfoWindowForDevice:[(MAMEDeviceWrapper *)[sender itemAtRow:[sender clickedRow]] device]];
}


- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item {
	return [(MAMEDeviceWrapper *)item children] > 0;
}


- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item {
	if (item != nil)
		return [(MAMEDeviceWrapper *)item children];
	else
		return 1;
}


- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item {
	if (item != nil)
		return [(MAMEDeviceWrapper *)item childAtIndex:index];
	else if (index == 0)
		return root;
	else
		return nil;
}


- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item {
	if ([[tableColumn identifier] isEqualToString:@"tag"])
		return [(MAMEDeviceWrapper *)item tag];
	else if ([[tableColumn identifier] isEqualToString:@"name"])
		return [(MAMEDeviceWrapper *)item name];
	else
		return nil;
}

@end
