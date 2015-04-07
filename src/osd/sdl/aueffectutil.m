#import <AvailabilityMacros.h>
#import <AudioUnit/AudioUnit.h>
#import <AudioUnit/AUCocoaUIView.h>
#import <AudioToolbox/AudioToolbox.h>
#import <Cocoa/Cocoa.h>
#import <CoreAudio/CoreAudio.h>
#import <CoreAudioKit/CoreAudioKit.h>
#import <CoreFoundation/CoreFoundation.h>
#import <CoreServices/CoreServices.h>

#include <utility>
#include <vector>

#include <stdlib.h>


#ifdef MAC_OS_X_VERSION_MAX_ALLOWED

#if MAC_OS_X_VERSION_MAX_ALLOWED < 1060

@protocol NSWindowDelegate <NSObject>
@end

#endif // MAC_OS_X_VERSION_MAX_ALLOWED < 1060

#endif // MAC_OS_X_VERSION_MAX_ALLOWED


struct EffectInfo
{
	Component   component;
	OSType      type;
	OSType      subtype;
	OSType      manufacturer;
};


static NSString *const AUEffectUtilErrorDomain  = @"AUEffectUtilErrorDomain";

static NSString *const AUEffectDocumentType     = @"AUEffect";
static NSString *const AUPresetDocumentType     = @"AudioUnit Preset";

static NSString *const ComponentTypeKey         = @"ComponentType";
static NSString *const ComponentSubTypeKey      = @"ComponentSubType";
static NSString *const ComponentManufacturerKey = @"ComponentManufacturer";
static NSString *const ClassInfoKey             = @"ClassInfo";


static void UpdateChangeCountCallback(void                      *userData,
									  void                      *object,
									  AudioUnitEvent const      *inEvent,
									  UInt64                    inEventHostTime,
									  AudioUnitParameterValue   inParameterValue)
{
	[(NSDocument *)userData updateChangeCount:NSChangeDone];
}


@interface AUEffectDocument : NSDocument <NSWindowDelegate>
{
	IBOutlet NSWindow           *window;
	NSView                      *view;
	AUParameterListenerRef      listener;

	AudioComponentDescription   description;
	AUGraph                     graph;
	AUNode                      outputNode, sourceNode, effectNode;
	AudioUnit                   outputUnit, sourceUnit, effectUnit;
}

- (void)dealloc;

- (void)makeWindowControllers;
- (BOOL)readFromData:(NSData *)data ofType:(NSString *)type error:(NSError **)error;
- (NSData *)dataOfType:(NSString *)type error:(NSError **)error;

- (void)viewFrameDidChange:(NSNotification *)notification;

@end

@implementation AUEffectDocument

- (void)loadEffectUI {
	if ((0 == effectNode) || (nil == window))
		return;

	OSStatus status;
	UInt32 uiDescSize;
	AudioUnitCocoaViewInfo *viewInfo;
	status = AudioUnitGetPropertyInfo(effectUnit,
									  kAudioUnitProperty_CocoaUI,
									  kAudioUnitScope_Global,
									  0,
									  &uiDescSize,
									  NULL);
	UInt32 const uiClassCount = 1 + ((uiDescSize - sizeof(*viewInfo)) / sizeof(viewInfo->mCocoaAUViewClass[0]));
	if ((noErr == status) && (0 < uiClassCount))
	{
		viewInfo = (AudioUnitCocoaViewInfo *)malloc(uiDescSize);
		status = AudioUnitGetProperty(effectUnit,
									  kAudioUnitProperty_CocoaUI,
									  kAudioUnitScope_Global,
									  0,
									  viewInfo,
									  &uiDescSize);
		if (noErr == status)
		{
			NSBundle *const bundle = [NSBundle bundleWithPath:[(NSURL *)viewInfo->mCocoaAUViewBundleLocation path]];
			Class const viewClass = [bundle classNamed:(NSString *)viewInfo->mCocoaAUViewClass[0]];
			if ((NULL != viewClass)
			 && [viewClass conformsToProtocol:@protocol(AUCocoaUIBase)]
			 && [viewClass instancesRespondToSelector:@selector(uiViewForAudioUnit:withSize:)])
			{
				id const factory = [[viewClass alloc] init];
				view = [factory uiViewForAudioUnit:effectUnit
										  withSize:[[window contentView] bounds].size];
				[factory release];
			}
			CFRelease(viewInfo->mCocoaAUViewBundleLocation);
			for (UInt32 i = 0; i < uiClassCount; i++)
				CFRelease(viewInfo->mCocoaAUViewClass[i]);
		}
		free(viewInfo);
	}
	if (nil == view)
	{
		view = [[[AUGenericView alloc] initWithAudioUnit:effectUnit] autorelease];
		[(AUGenericView *)view setShowsExpertParameters:YES];
	}

	[view setAutoresizingMask:NSViewNotSizable];
	[view setFrameOrigin:NSMakePoint(0, 0)];
	NSRect const oldFrame = [window frame];
	NSRect const desired = [window frameRectForContentRect:[view frame]];
	NSRect const newFrame = NSMakeRect(oldFrame.origin.x,
									   oldFrame.origin.y + oldFrame.size.height - desired.size.height,
									   desired.size.width,
									   desired.size.height);
	[window setFrame:newFrame display:YES animate:NO];
	[[window contentView] addSubview:view];
	[view setPostsFrameChangedNotifications:YES];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(viewFrameDidChange:)
												 name:NSViewFrameDidChangeNotification
											   object:view];
}

- (id)init {
	if (!(self = [super init])) return nil;

	window = nil;
	view = nil;
	listener = NULL;

	description.componentType = description.componentSubType = description.componentManufacturer = 0;
	description.componentFlags = description.componentFlagsMask = 0;
	graph = NULL;
	outputNode = sourceNode = effectNode = 0;

	AudioComponentDescription const outputDesc = { kAudioUnitType_Output, kAudioUnitSubType_DefaultOutput, kAudioUnitManufacturer_Apple, 0, 0, };
	AudioComponentDescription const sourceDesc = { kAudioUnitType_Generator, kAudioUnitSubType_AudioFilePlayer, kAudioUnitManufacturer_Apple, 0, 0, };
	if ((noErr != NewAUGraph(&graph))
	 || (noErr != AUGraphAddNode(graph, &outputDesc, &outputNode))
	 || (noErr != AUGraphAddNode(graph, &sourceDesc, &sourceNode))
	 || (noErr != AUGraphOpen(graph))
	 || (noErr != AUGraphNodeInfo(graph, outputNode, NULL, &outputUnit))
	 || (noErr != AUGraphNodeInfo(graph, sourceNode, NULL, &sourceUnit))
	 || (noErr != AUGraphInitialize(graph)))
	{
		[self release];
		return nil;
	}

	return self;
}

- (void)dealloc {
	if (NULL != listener)
		AUListenerDispose(listener);

	if (NULL != graph)
	{
		AUGraphClose(graph);
		DisposeAUGraph(graph);
	}

	[super dealloc];
}

- (void)makeWindowControllers {
	window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 400, 300)
										 styleMask:(NSTitledWindowMask |
													NSClosableWindowMask |
													NSMiniaturizableWindowMask)
										   backing:NSBackingStoreBuffered
											 defer:YES];
	[window setContentMinSize:NSMakeSize(400, 300)];
	[window setReleasedWhenClosed:NO];
	[window setDelegate:self];
	[window setTitle:@"Effect"];
	[self setWindow:window];

	NSWindowController *const controller = [[NSWindowController alloc] initWithWindow:window];
	[self addWindowController:controller];
	[controller release];
	[window release];

	[self loadEffectUI];
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)type error:(NSError **)error {
	OSStatus status;

	BOOL const hasWrapper = [type isEqualToString:AUEffectDocumentType];
	if (!hasWrapper && ![type isEqualToString:AUPresetDocumentType])
	{
		if (NULL != error)
		{
			NSString *const message = [NSString stringWithFormat:@"Unsupported document type %@", type];
			NSDictionary *const info = [NSDictionary dictionaryWithObjectsAndKeys:message,  NSLocalizedDescriptionKey,
																				  nil];
			*error = [NSError errorWithDomain:AUEffectUtilErrorDomain code:0 userInfo:info];
		}
		return NO;
	}

	NSString *errDesc = nil;
	id const desc = [NSPropertyListSerialization propertyListFromData:data
													 mutabilityOption:0
													 		   format:NULL
													 errorDescription:&errDesc];
	if ((nil == desc) || ![desc isKindOfClass:[NSDictionary class]] || (nil != errDesc))
	{
		if (NULL != error)
		{
			NSString *const message = [NSString stringWithFormat:@"Error in file format (%@)", errDesc];
			NSDictionary *const info = [NSDictionary dictionaryWithObjectsAndKeys:message,  NSLocalizedDescriptionKey,
																				  nil];
			*error = [NSError errorWithDomain:AUEffectUtilErrorDomain code:0 userInfo:info];
		}
		if (nil != errDesc)
			[errDesc release];
		return NO;
	}

	id const typeValue = [desc objectForKey:(hasWrapper ? ComponentTypeKey : @"type")];
	id const subtypeValue = [desc objectForKey:(hasWrapper ? ComponentSubTypeKey : @"subtype")];
	id const manufacturerValue = [desc objectForKey:(hasWrapper ? ComponentManufacturerKey : @"manufacturer")];
	if ((nil == typeValue)          || ![typeValue isKindOfClass:[NSNumber class]]
	 || (nil == subtypeValue)       || ![subtypeValue isKindOfClass:[NSNumber class]]
	 || (nil == manufacturerValue)  || ![manufacturerValue isKindOfClass:[NSNumber class]]
	 || ([typeValue unsignedLongValue] != kAudioUnitType_Effect))
	{
		if (NULL != error)
		{
			NSString *const message = @"Error in effect description file format";
			NSDictionary *const info = [NSDictionary dictionaryWithObjectsAndKeys:message,  NSLocalizedDescriptionKey,
																				  nil];
			*error = [NSError errorWithDomain:AUEffectUtilErrorDomain code:0 userInfo:info];
		}
		return NO;
	}

	if (NULL != listener)
	{
		AUListenerDispose(listener);
		listener = NULL;
	}
	if (nil != view)
	{
		[[NSNotificationCenter defaultCenter] removeObserver:self
														name:NSViewFrameDidChangeNotification
													  object:nil];
		[view removeFromSuperview];
		view = nil;
	}
	if (0 != effectNode)
	{
		view = nil;
		AUGraphRemoveNode(graph, effectNode);
		effectNode = 0;
	}

	description.componentType = [typeValue longValue];
	description.componentSubType = [subtypeValue longValue];
	description.componentManufacturer = [manufacturerValue longValue];
	status = noErr;
	status = AUGraphClearConnections(graph);
	if (noErr == status) status = AUGraphAddNode(graph, &description, &effectNode);
	if (noErr == status) status = AUGraphNodeInfo(graph, effectNode, NULL, &effectUnit);
	if (noErr == status) status = AUGraphConnectNodeInput(graph, sourceNode, 0, effectNode, 0);
	if (noErr == status) status = AUGraphConnectNodeInput(graph, effectNode, 0, outputNode, 0);
	if (noErr == status) status = AUGraphUpdate(graph, NULL);
	if (noErr != status)
	{
		if (NULL != error)
		{
			NSString * const message = @"Error encountered while configuring AudioUnit graph";
			NSError *const underlying = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
			NSDictionary *const info = [NSDictionary dictionaryWithObjectsAndKeys:message,       NSLocalizedDescriptionKey,
																				  underlying,    NSUnderlyingErrorKey,
																				  nil];
			*error = [NSError errorWithDomain:AUEffectUtilErrorDomain code:0 userInfo:info];
		}
		return NO;
	}

	CFPropertyListRef const classInfo = (CFPropertyListRef)(hasWrapper ? [desc objectForKey:ClassInfoKey] : desc);
	if (NULL != classInfo)
	{
		AudioUnitParameter change = { effectUnit, kAUParameterListener_AnyParameter, 0, 0 };
		status = AudioUnitSetProperty(effectUnit,
									  kAudioUnitProperty_ClassInfo,
									  kAudioUnitScope_Global,
									  0,
									  &classInfo,
									  sizeof(classInfo));
		if (noErr == status) status = AUParameterListenerNotify(NULL, NULL, &change);
		if (noErr != status)
		{
			if (NULL != error)
			{
				NSString * const message = @"Error configuring effect";
				NSError *const underlying = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
				NSDictionary *const info = [NSDictionary dictionaryWithObjectsAndKeys:message,       NSLocalizedDescriptionKey,
																					  underlying,    NSUnderlyingErrorKey,
																					  nil];
				*error = [NSError errorWithDomain:AUEffectUtilErrorDomain code:0 userInfo:info];
			}
			return NO;
		}
	}

	UInt32 paramListSize = 0;
	status = AudioUnitGetPropertyInfo(
			effectUnit,
			kAudioUnitProperty_ParameterList,
			kAudioUnitScope_Global,
			0,
			&paramListSize, NULL);
	if (noErr != status)
	{
		if (NULL != error)
		{
			NSString * const message = @"Error getting effect parameters";
			NSError *const underlying = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
			NSDictionary *const info = [NSDictionary dictionaryWithObjectsAndKeys:message,       NSLocalizedDescriptionKey,
																				  underlying,    NSUnderlyingErrorKey,
																				  nil];
			*error = [NSError errorWithDomain:AUEffectUtilErrorDomain code:0 userInfo:info];
		}
		return NO;
	}
	UInt32 const paramCount = paramListSize / sizeof(AudioUnitParameterID);
	if (0U < paramCount)
	{
		status = AUEventListenerCreate(UpdateChangeCountCallback,
									   self,
									   CFRunLoopGetCurrent(),
									   kCFRunLoopDefaultMode,
									   0.05,
									   0.05,
									   &listener);
		if (noErr != status)
		{
			if (NULL != error)
			{
				NSString * const message = @"Error creating AudioUnit event listener";
				NSError *const underlying = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
				NSDictionary *const info = [NSDictionary dictionaryWithObjectsAndKeys:message,       NSLocalizedDescriptionKey,
																					  underlying,    NSUnderlyingErrorKey,
																					  nil];
				*error = [NSError errorWithDomain:AUEffectUtilErrorDomain code:0 userInfo:info];
			}
			return NO;
		}
		AudioUnitParameterID *const params = (AudioUnitParameterID *)malloc(paramListSize);
		AudioUnitGetProperty(
				effectUnit,
				kAudioUnitProperty_ParameterList,
				kAudioUnitScope_Global,
				0,
				params,
				&paramListSize);
		if (noErr != status)
		{
			free(params);
			if (NULL != error)
			{
				NSString * const message = @"Error getting effect parameters";
				NSError *const underlying = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
				NSDictionary *const info = [NSDictionary dictionaryWithObjectsAndKeys:message,       NSLocalizedDescriptionKey,
																					  underlying,    NSUnderlyingErrorKey,
																					  nil];
				*error = [NSError errorWithDomain:AUEffectUtilErrorDomain code:0 userInfo:info];
			}
			return NO;
		}
		for (UInt32 i = 0; (i < paramCount) && (noErr == status); i++)
		{
			AudioUnitEvent event;
			event.mEventType = kAudioUnitEvent_ParameterValueChange;
			event.mArgument.mParameter.mAudioUnit = effectUnit;
			event.mArgument.mParameter.mParameterID = params[i];
			event.mArgument.mParameter.mScope = kAudioUnitScope_Global;
			event.mArgument.mParameter.mElement = 0;
			status = AUEventListenerAddEventType(listener, self, &event);
		}
		free(params);
		if (noErr != status)
		{
			free(params);
			if (NULL != error)
			{
				NSString * const message = @"Error getting effect parameters";
				NSError *const underlying = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
				NSDictionary *const info = [NSDictionary dictionaryWithObjectsAndKeys:message,       NSLocalizedDescriptionKey,
																					  underlying,    NSUnderlyingErrorKey,
																					  nil];
				*error = [NSError errorWithDomain:AUEffectUtilErrorDomain code:0 userInfo:info];
			}
			return NO;
		}
	}

	[self loadEffectUI];

	return YES;
}

- (NSData *)dataOfType:(NSString *)type error:(NSError **)error {
	CFPropertyListRef classInfo;
	UInt32 infoSize = sizeof(classInfo);
	OSStatus const status = AudioUnitGetProperty(effectUnit,
												 kAudioUnitProperty_ClassInfo,
												 kAudioUnitScope_Global,
												 0,
												 &classInfo,
												 &infoSize);
	if (noErr != status)
	{
		if (NULL != error)
		{
			NSString const *message = @"Error getting effect settings";
			NSError const *underlying = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
			NSDictionary *const info = [NSDictionary dictionaryWithObjectsAndKeys:message,      NSLocalizedDescriptionKey,
																				  underlying,   NSUnderlyingErrorKey,
																				  nil];
			*error = [NSError errorWithDomain:AUEffectUtilErrorDomain code:0 userInfo:info];
		}
		return nil;
	}
	NSDictionary *desc = nil;
	if ([type isEqualToString:AUEffectDocumentType])
	{
		NSNumber const *typeVal = [NSNumber numberWithUnsignedLong:description.componentType];
		NSNumber const *subtypeVal = [NSNumber numberWithUnsignedLong:description.componentSubType];
		NSNumber const *manufacturerVal = [NSNumber numberWithUnsignedLong:description.componentManufacturer];
		desc = [NSDictionary dictionaryWithObjectsAndKeys:typeVal,          ComponentTypeKey,
														  subtypeVal,       ComponentSubTypeKey,
														  manufacturerVal,  ComponentManufacturerKey,
														  classInfo,        ClassInfoKey,
														  nil];
	}
	else if ([type isEqualToString:AUPresetDocumentType])
	{
		desc = [NSDictionary dictionaryWithDictionary:(NSDictionary *)classInfo];
	}
	CFRelease(classInfo);
	if (nil == desc)
	{
			NSDictionary *const info = [NSDictionary dictionaryWithObjectsAndKeys:@"Unsupported document type", NSLocalizedDescriptionKey,
																				  nil];
			*error = [NSError errorWithDomain:AUEffectUtilErrorDomain code:0 userInfo:info];
		return nil;
	}

	NSString *errDesc = nil;
	NSData *const data = [NSPropertyListSerialization dataFromPropertyList:desc
																	format:NSPropertyListXMLFormat_v1_0
														  errorDescription:&errDesc];
	if ((nil == data) || (nil != errDesc))
	{
		if (NULL != error)
		{
			NSString *message;
			if (nil != errDesc)
				message = [NSString stringWithFormat:@"Error serialising effect settings: %@", errDesc];
			else
				message = @"Error serialising effect settings";
			NSDictionary *const info = [NSDictionary dictionaryWithObjectsAndKeys:message,  NSLocalizedDescriptionKey,
																				  nil];
			*error = [NSError errorWithDomain:AUEffectUtilErrorDomain code:0 userInfo:info];
		}
		if (nil != errDesc) [errDesc release];
		return nil;
	}
	return data;
}

- (void)viewFrameDidChange:(NSNotification *)notification {
	NSRect const oldFrame = [window frame];
	NSRect const desired = [window frameRectForContentRect:[[notification object] frame]];
	NSRect const newFrame = NSMakeRect(oldFrame.origin.x,
									   oldFrame.origin.y + oldFrame.size.height - desired.size.height,
									   desired.size.width,
									   desired.size.height);
	[window setFrame:newFrame display:YES animate:NO];
}

@end


@interface AUEffectUtilAppDelegate : NSObject
{
	EffectInfo      *effects;

	IBOutlet NSMenu *newEffectMenu;
}

- (id)init;
- (void)dealloc;

- (IBAction)newEffect:(id)sender;

- (void)applicationWillFinishLaunching:(NSNotification *)notification;
- (void)applicationDidFinishLaunching:(NSNotification *)notification;
- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender;

@end

@implementation AUEffectUtilAppDelegate

- (void)appendApplicationMenu:(NSMenu *)parent {
	NSMenuItem *item;
	NSMenu *submenu;
	NSString *const appName = [(NSDictionary *)CFBundleGetInfoDictionary(CFBundleGetMainBundle()) objectForKey:@"CFBundleName"];

	NSMenu *const menu = [[NSMenu allocWithZone:[NSMenu zone]] initWithTitle:@"Application"];
	item = [parent addItemWithTitle:@"Application" action:NULL keyEquivalent:@""];
	[parent setSubmenu:menu forItem:item];
	[menu release];
	[menu setValue:@"NSAppleMenu" forKey:@"name"];

	item = [menu addItemWithTitle:[NSString stringWithFormat:@"About %@", appName] action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];
	[item setTarget:NSApp];

	[menu addItem:[NSMenuItem separatorItem]];

	item = [menu addItemWithTitle:@"Services" action:NULL keyEquivalent:@""];
	submenu = [[NSMenu allocWithZone:[NSMenu zone]] initWithTitle:@"Services"];
	[menu setSubmenu:submenu forItem:item];
	[submenu release];
	[NSApp setServicesMenu:submenu];

	[menu addItem:[NSMenuItem separatorItem]];
	
	item = [menu addItemWithTitle:[NSString stringWithFormat:@"Hide %@", appName] action:@selector(hide:) keyEquivalent:@"h"];
	item = [menu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
	[item setKeyEquivalentModifierMask:NSCommandKeyMask | NSAlternateKeyMask];
	item = [menu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];

	[menu addItem:[NSMenuItem separatorItem]];

	item = [menu addItemWithTitle:[NSString stringWithFormat:@"Quit %@", appName] action:@selector(terminate:) keyEquivalent:@"q"];
	[item setTarget:NSApp];
}

- (void)appendFileMenu:(NSMenu *)parent {
	NSMenuItem *item;

	NSMenu *const menu = [[NSMenu allocWithZone:[NSMenu zone]] initWithTitle:@"File"];
	item = [parent addItemWithTitle:@"File" action:NULL keyEquivalent:@""];
	[parent setSubmenu:menu forItem:item];
	[menu release];

	item = [menu addItemWithTitle:@"New" action:NULL keyEquivalent:@""];
	newEffectMenu = [[NSMenu allocWithZone:[NSMenu zone]] initWithTitle:@"New"];
	[menu setSubmenu:newEffectMenu forItem:item];
	[newEffectMenu release];
	item = [menu addItemWithTitle:@"Open…" action:@selector(openDocument:) keyEquivalent:@"o"];
	
	[menu addItem:[NSMenuItem separatorItem]];
	
	item = [menu addItemWithTitle:NSLocalizedString(@"Close", nil) action:@selector(performClose:) keyEquivalent:@"w"];
	item = [menu addItemWithTitle:NSLocalizedString(@"Save", nil) action:@selector(saveDocument:) keyEquivalent:@"s"];
	item = [menu addItemWithTitle:NSLocalizedString(@"Save As…", nil) action:@selector(saveDocumentAs:) keyEquivalent:@"S"];
	item = [menu addItemWithTitle:NSLocalizedString(@"Save All", nil) action:@selector(saveAllDocuments:) keyEquivalent:@""];
	item = [menu addItemWithTitle:NSLocalizedString(@"Revert to Saved", nil) action:@selector(revertDocumentToSaved:) keyEquivalent:@""];
}

- (void)appendEditMenu:(NSMenu *)parent {
	NSMenuItem *item;

	NSMenu *const menu = [[NSMenu allocWithZone:[NSMenu zone]] initWithTitle:@"Edit"];
	item = [parent addItemWithTitle:@"Edit" action:NULL keyEquivalent:@""];
	[parent setSubmenu:menu forItem:item];
	[menu release];

	item = [menu addItemWithTitle:@"Undo" action:@selector(undo:) keyEquivalent:@"z"];
	item = [menu addItemWithTitle:@"Redo" action:@selector(redo:) keyEquivalent:@"Z"];

	[menu addItem:[NSMenuItem separatorItem]];

	item = [menu addItemWithTitle:@"Cut" action:@selector(cut:) keyEquivalent:@"x"];
	item = [menu addItemWithTitle:@"Copy" action:@selector(copy:) keyEquivalent:@"c"];
	item = [menu addItemWithTitle:@"Paste" action:@selector(paste:) keyEquivalent:@"v"];
	item = [menu addItemWithTitle:@"Delete" action:@selector(delete:) keyEquivalent:@""];
	item = [menu addItemWithTitle:@"Select All" action:@selector(selectAll:) keyEquivalent:@"a"];
}

- (void)appendWindowMenu:(NSMenu *)parent {
	NSMenuItem *item;

	NSMenu *const menu = [[NSMenu allocWithZone:[NSMenu zone]] initWithTitle:@"Window"];
	item = [parent addItemWithTitle:@"Window" action:NULL keyEquivalent:@""];
	[parent setSubmenu:menu forItem:item];
	[menu release];
	[NSApp setWindowsMenu:menu];

	item = [menu addItemWithTitle:@"Minimize" action:@selector(performMinimize:) keyEquivalent:@"m"];
	item = [menu addItemWithTitle:@"Zoom" action:@selector(performZoom:) keyEquivalent:@""];

	[menu addItem:[NSMenuItem separatorItem]];

	item = [menu addItemWithTitle:@"Bring All to Front" action:@selector(arrangeInFront:) keyEquivalent:@""];
}

- (void)appendHelpMenu:(NSMenu *)parent {
	NSMenuItem *item;
	NSString *const appName = [(NSDictionary *)CFBundleGetInfoDictionary(CFBundleGetMainBundle()) objectForKey:@"CFBundleName"];

	NSMenu *const menu = [[NSMenu allocWithZone:[NSMenu zone]] initWithTitle:@"Help"];
	item = [parent addItemWithTitle:@"Help" action:NULL keyEquivalent:@""];
	[parent setSubmenu:menu forItem:item];
	[menu release];
	[NSApp setHelpMenu:menu];

	item = [menu addItemWithTitle:[NSString stringWithFormat:@"%@ Help", appName] action:@selector(showHelp:) keyEquivalent:@"?"];
}

- (id)init {
	if (!(self = [super init])) return nil;
	effects = NULL;
	return self;
}

- (void)dealloc {
	if (effects) free(effects);
	[super dealloc];
}

- (IBAction)newEffect:(id)sender {
	int const index = [sender tag];
	if ((0 > index) || (0 == effects[index].component))
	{
		NSAlert *const alert = [[NSAlert alloc] init];
		[alert setAlertStyle:NSWarningAlertStyle];
		[alert setMessageText:@"Invalid effect component"];
		[alert addButtonWithTitle:@"OK"];
		[alert runModal];
		[alert release];
		return;
	}

	NSNumber *const typeValue = [NSNumber numberWithUnsignedLong:effects[index].type];
	NSNumber *const subtypeValue = [NSNumber numberWithUnsignedLong:effects[index].subtype];
	NSNumber *const manufacturerValue = [NSNumber numberWithUnsignedLong:effects[index].manufacturer];
	NSDictionary *const desc = [NSDictionary dictionaryWithObjectsAndKeys:typeValue,            ComponentTypeKey,
																		  subtypeValue,         ComponentSubTypeKey,
																		  manufacturerValue,    ComponentManufacturerKey,
																		  nil];
	NSString *errDesc = nil;
	NSData *const data = [NSPropertyListSerialization dataFromPropertyList:desc
																	format:NSPropertyListXMLFormat_v1_0
														  errorDescription:&errDesc];
	if ((nil == data) || (nil != errDesc))
	{
		NSAlert *const alert = [[NSAlert alloc] init];
		[alert setAlertStyle:NSWarningAlertStyle];
		[alert setMessageText:@"Error serialising properties for new effect"];
		if (nil != errDesc) [alert setInformativeText:[errDesc autorelease]];
		[alert addButtonWithTitle:@"OK"];
		[alert runModal];
		[alert release];
		return;
	}

	NSError *err = nil;
	AUEffectDocument *const document = [[AUEffectDocument alloc] init];
	if ((nil == document) || ![document readFromData:data ofType:AUEffectDocumentType error:&err])
	{
		[document release];
		if (nil != err)
		{
			[[NSAlert alertWithError:err] runModal];
		}
		else
		{
			NSAlert *const alert = [[NSAlert alloc] init];
			[alert setAlertStyle:NSWarningAlertStyle];
			[alert setMessageText:@"Error creating new effect document"];
			[alert addButtonWithTitle:@"OK"];
			[alert runModal];
			[alert release];
		}
		return;
	}

	[document makeWindowControllers];
	[document showWindows];
	[[NSDocumentController sharedDocumentController] addDocument:document];
	[document release];
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification {
	NSMenu *const menubar = [[NSMenu allocWithZone:[NSMenu zone]] initWithTitle:@"MainMenu"];
	[NSApp setMainMenu:menubar];
	[menubar release];
	[self appendApplicationMenu:menubar];
	[self appendFileMenu:menubar];
	[self appendEditMenu:menubar];
	[self appendWindowMenu:menubar];
	[self appendHelpMenu:menubar];

	ProcessSerialNumber const serial = { 0, kCurrentProcess };
	OSStatus const status = TransformProcessType(&serial, kProcessTransformToForegroundApplication);
	if (noErr != status)
	{
		NSLog(@"Error transforming to foreground application (%ld)", (long)status);
		[NSApp terminate:self];
	}
	else
	{
		[NSApp activateIgnoringOtherApps:YES];
	}
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
	ComponentDescription effectFilter = { kAudioUnitType_Effect, 0, 0, 0, 0 };
	long const count = CountComponents(&effectFilter);
	if (0 == count)
	{
		NSAlert *const alert = [[NSAlert alloc] init];
		[alert setAlertStyle:NSWarningAlertStyle];
		[alert setMessageText:@"No AudioUnit effects found"];
		[alert addButtonWithTitle:@"OK"];
		[alert runModal];
		[alert release];
	}

	std::vector<std::pair<Component, OSStatus> > failed;
	effects = (EffectInfo *)malloc(count * sizeof(*effects));
	Component effect = FindNextComponent(0, &effectFilter);
	for (long i = 0; (i < count) && (effect != 0); i++, effect = FindNextComponent(effect, &effectFilter))
	{
		ComponentDescription effectDesc;
		Handle const nameHandle = NewHandle(4);
		OSStatus const err = GetComponentInfo(effect, &effectDesc, nameHandle, NULL, NULL);
		if (noErr == err)
		{
			effects[i].component = effect;
			effects[i].type = effectDesc.componentType;
			effects[i].subtype = effectDesc.componentSubType;
			effects[i].manufacturer = effectDesc.componentManufacturer;
			HLock(nameHandle);
			CFStringRef name = CFStringCreateWithPascalString(NULL,
															  (unsigned char const *)*nameHandle,
															  kCFStringEncodingMacRoman);
			HUnlock(nameHandle);
			NSMenuItem *const item = [newEffectMenu addItemWithTitle:(NSString *)name
															  action:@selector(newEffect:)
													   keyEquivalent:@""];
			[item setTag:i];
			[item setTarget:self];
			CFRelease(name);
		}
		else
		{
			effects[i].component = 0;
			failed.push_back(std::make_pair(effect, err));
		}
		DisposeHandle(nameHandle);
	}

	if (!failed.empty())
	{
		NSString *const message = [NSString stringWithFormat:@"Failed to get info for %lu effect%s",
															 (unsigned long)failed.size(),
															 ((1U == failed.size()) ? "" : "s")];
		NSMutableString *const detail = [NSMutableString stringWithCapacity:(16 * failed.size())];
		std::vector<std::pair<Component, OSStatus> >::const_iterator it = failed.begin();
		[detail appendFormat:@"%lu (%ld)", (unsigned long)it->first, (long)it->second];
		++it;
		while (failed.end() != it)
		{
			[detail appendFormat:@", %lu (%ld)", (unsigned long)it->first, (long)it->second];
			++it;
		}
		NSAlert *const alert = [[NSAlert alloc] init];
		[alert setAlertStyle:NSWarningAlertStyle];
		[alert setMessageText:message];
		[alert setInformativeText:[NSString stringWithString:detail]];
		[alert addButtonWithTitle:@"OK"];
		[alert runModal];
		[alert release];
	}
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender {
	return NO;
}

@end


int main(int argc, char *argv[])
{
	NSAutoreleasePool *pool;

	// Initialise NSApplication
	pool = [[NSAutoreleasePool alloc] init];
	[NSApplication sharedApplication];
	AUEffectUtilAppDelegate *const delegate = [[AUEffectUtilAppDelegate alloc] init];
	[NSApp setDelegate:delegate];
	[pool release];

	// Let's go!
	pool = [[NSAutoreleasePool alloc] init];
	[NSApp run];
	[delegate release];
	[pool release];
	return 0;
}
