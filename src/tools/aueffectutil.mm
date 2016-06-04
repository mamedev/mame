// license:BSD-3-Clause
// copyright-holders:Vas Crabb
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

typedef ComponentDescription AudioComponentDescription;

@protocol NSApplicationDelegate <NSObject>
@end

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
static NSString *const ForceGenericViewKey      = @"ForceGenericView";
static NSString *const WindowFrameKey           = @"WindowFrame";


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
	IBOutlet NSButton           *genericViewButton;
	IBOutlet NSPopUpButton      *presetButton;
	NSView                      *view;
	NSSize                      headerSize;
	CFArrayRef                  presets;
	AUParameterListenerRef      listener;
	BOOL                        forceGenericView;
	NSString                    *restoreFrame;

	AudioComponentDescription   description;
	AUGraph                     graph;
	AUNode                      outputNode, sourceNode, effectNode;
	AudioUnit                   outputUnit, sourceUnit, effectUnit;
}

- (void)dealloc;

- (void)makeWindowControllers;
- (BOOL)readFromData:(NSData *)data ofType:(NSString *)type error:(NSError **)error;
- (NSData *)dataOfType:(NSString *)type error:(NSError **)error;

- (IBAction)toggleGenericView:(id)sender;
- (IBAction)loadPreset:(id)sender;

- (void)viewFrameDidChange:(NSNotification *)notification;

@end

@implementation AUEffectDocument

- (void)loadEffectUI {
	if ((0 == effectNode) || (nil == window))
		return;

	BOOL customViewValid = NO;
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
				customViewValid = YES;
				if (!forceGenericView)
				{
					id const factory = [[viewClass alloc] init];
					view = [factory uiViewForAudioUnit:effectUnit
											  withSize:[[window contentView] bounds].size];
					[factory release];
				}
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
									   oldFrame.origin.y + oldFrame.size.height - headerSize.height - desired.size.height,
									   desired.size.width,
									   headerSize.height + desired.size.height);
	[window setFrame:newFrame display:YES animate:NO];
	[[window contentView] addSubview:view];
	[view setPostsFrameChangedNotifications:YES];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(viewFrameDidChange:)
												 name:NSViewFrameDidChangeNotification
											   object:view];

	[genericViewButton setEnabled:customViewValid];
	if (!customViewValid)
	{
		forceGenericView = YES;
		[genericViewButton setState:NSOnState];
	}

	CFIndex const presetCount = (NULL != presets) ? CFArrayGetCount(presets) : 0;
	[presetButton setEnabled:(0 < presetCount)];
	while (1 < [presetButton numberOfItems])
		[presetButton removeItemAtIndex:1];
	for (CFIndex i = 0; i < presetCount; i++)
	{
		AUPreset const *preset = (AUPreset const*)CFArrayGetValueAtIndex(presets, i);
		NSMenuItem const *item = [[presetButton menu] addItemWithTitle:(NSString *)preset->presetName
																action:@selector(loadPreset:)
														 keyEquivalent:@""];
		[item setTarget:self];
		[item setTag:i];
	}
}

- (id)init {
	if (!(self = [super init])) return nil;

	window = nil;
	genericViewButton = nil;
	presetButton = nil;
	view = nil;
	presets = NULL;
	listener = NULL;
	forceGenericView = NO;
	restoreFrame = nil;

	description.componentType = description.componentSubType = description.componentManufacturer = 0;
	description.componentFlags = description.componentFlagsMask = 0;
	graph = NULL;
	outputNode = sourceNode = effectNode = 0;

	AudioComponentDescription const outputDesc = { kAudioUnitType_Output,
												   kAudioUnitSubType_DefaultOutput,
												   kAudioUnitManufacturer_Apple,
												   0,
												   0, };
	AudioComponentDescription const sourceDesc = { kAudioUnitType_Generator,
												   kAudioUnitSubType_AudioFilePlayer,
												   kAudioUnitManufacturer_Apple,
												   0,
												   0, };
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
	if (NULL != presets)
		CFRelease(presets);

	if (NULL != listener)
		AUListenerDispose(listener);

	if (nil != restoreFrame)
		[restoreFrame release];

	if (NULL != graph)
	{
		AUGraphClose(graph);
		DisposeAUGraph(graph);
	}

	[super dealloc];
}

- (void)makeWindowControllers {
	genericViewButton = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 100, 18)];
	[genericViewButton setAutoresizingMask:NSViewNotSizable];
	[[genericViewButton cell] setControlSize:NSSmallControlSize];
	[genericViewButton setButtonType:NSSwitchButton];
	[genericViewButton setBordered:NO];
	[genericViewButton setAllowsMixedState:NO];
	[genericViewButton setState:(forceGenericView ? NSOnState : NSOffState)];
	[genericViewButton setTitle:@"Use generic editor view"];
	[genericViewButton setAction:@selector(toggleGenericView:)];
	[genericViewButton setTarget:self];
	[genericViewButton sizeToFit];

	presetButton = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(0, 0, 100, 22) pullsDown:YES];
	[presetButton setAutoresizingMask:NSViewNotSizable];
	[[presetButton cell] setControlSize:NSSmallControlSize];
	[[presetButton cell] setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSSmallControlSize]]];
	[presetButton setTitle:@"Load preset"];
	[[[presetButton menu] addItemWithTitle:@"Load preset" action:NULL keyEquivalent:@""] setHidden:YES];
	[presetButton sizeToFit];

	CGFloat const controlWidth = MAX(NSWidth([genericViewButton frame]), NSWidth([presetButton frame]));
	NSRect const presetFrame = NSMakeRect(17,
										  8,
										  controlWidth,
										  NSHeight([presetButton frame]));
	NSRect const genericViewFrame = NSMakeRect(17,
											   NSMaxY(presetFrame) + 9,
											   controlWidth,
											   NSHeight([genericViewButton frame]));
	[genericViewButton setFrame:genericViewFrame];
	[presetButton setFrame:presetFrame];

	headerSize = NSMakeSize((2 * 17) + controlWidth, 18 + NSMaxY(genericViewFrame));
	NSView *const container = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, headerSize.width, headerSize.height)];
	[container setAutoresizingMask:(NSViewMinXMargin | NSViewMaxXMargin | NSViewMinYMargin)];
	[container addSubview:genericViewButton];
	[genericViewButton release];
	[container addSubview:presetButton];
	[presetButton release];

	window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, headerSize.width, headerSize.height)
										 styleMask:(NSTitledWindowMask |
													NSClosableWindowMask |
													NSMiniaturizableWindowMask)
										   backing:NSBackingStoreBuffered
											 defer:YES];
	[window setReleasedWhenClosed:NO];
	[window setDelegate:self];
	[window setTitle:@"Effect"];
	[[window contentView] addSubview:container];
	[container release];
	[self setWindow:window];

	NSWindowController *const controller = [[NSWindowController alloc] initWithWindow:window];
	[self addWindowController:controller];
	[controller release];
	[window release];

	[self loadEffectUI];
	if (nil != restoreFrame)
	{
		[window setFrameFromString:restoreFrame];
	}
	else
	{
		NSRect const available = [[NSScreen mainScreen] visibleFrame];
		NSRect frame = [window frame];
		frame.origin.x = (NSWidth(available) - NSWidth(frame)) / 4;
		frame.origin.y = (NSHeight(available) - NSHeight(frame)) * 3 / 4;
		[window setFrame:frame display:YES animate:NO];
	}
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)type error:(NSError **)error {
	OSStatus status;
	UInt32 propertySize;

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

	id const typeValue = [desc objectForKey:(hasWrapper ? ComponentTypeKey : (NSString *)CFSTR(kAUPresetTypeKey))];
	id const subtypeValue = [desc objectForKey:(hasWrapper ? ComponentSubTypeKey : (NSString *)CFSTR(kAUPresetSubtypeKey))];
	id const manufacturerValue = [desc objectForKey:(hasWrapper ? ComponentManufacturerKey : (NSString *)CFSTR(kAUPresetManufacturerKey))];
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

	if (NULL != presets)
	{
		CFRelease(presets);
		presets = NULL;
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

	propertySize = 0;
	status = AudioUnitGetPropertyInfo(
			effectUnit,
			kAudioUnitProperty_ParameterList,
			kAudioUnitScope_Global,
			0,
			&propertySize,
			NULL);
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
	UInt32 const paramCount = propertySize / sizeof(AudioUnitParameterID);
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
		AudioUnitParameterID *const params = (AudioUnitParameterID *)malloc(propertySize);
		AudioUnitGetProperty(
				effectUnit,
				kAudioUnitProperty_ParameterList,
				kAudioUnitScope_Global,
				0,
				params,
				&propertySize);
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

	propertySize = sizeof(presets);
	status = AudioUnitGetProperty(effectUnit,
								  kAudioUnitProperty_FactoryPresets,
								  kAudioUnitScope_Global,
								  0,
								  &presets,
								  &propertySize);
	if ((noErr != status) && (NULL != presets))
	{
		CFRelease(presets);
		presets = NULL;
	}

	if (hasWrapper)
	{
		if ((nil != [desc objectForKey:ForceGenericViewKey])
		 && [[desc objectForKey:ForceGenericViewKey] respondsToSelector:@selector(boolValue)])
		{
			forceGenericView = [[desc objectForKey:ForceGenericViewKey] boolValue];
			[genericViewButton setState:(forceGenericView ? NSOnState : NSOffState)];
		}
		if ((nil != [desc objectForKey:WindowFrameKey])
		 && [[desc objectForKey:WindowFrameKey] isKindOfClass:[NSString class]])
		{
			if (nil != restoreFrame) [restoreFrame release];
			restoreFrame = [[NSString alloc] initWithString:[desc objectForKey:WindowFrameKey]];
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
		NSNumber const *forceGenericViewVal = [NSNumber numberWithBool:forceGenericView];
		NSString const *windowFrameVal = [window stringWithSavedFrame];
		desc = [NSDictionary dictionaryWithObjectsAndKeys:typeVal,              ComponentTypeKey,
														  subtypeVal,           ComponentSubTypeKey,
														  manufacturerVal,      ComponentManufacturerKey,
														  classInfo,            ClassInfoKey,
														  forceGenericViewVal,  ForceGenericViewKey,
														  windowFrameVal,       WindowFrameKey,
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

- (IBAction)toggleGenericView:(id)sender {
	forceGenericView = (NSOnState == [sender state]);
	if (nil != view)
	{
		[[NSNotificationCenter defaultCenter] removeObserver:self
														name:NSViewFrameDidChangeNotification
													  object:nil];
		[view removeFromSuperview];
		view = nil;
	}
	if (0 != effectNode)
		[self loadEffectUI];
}

- (IBAction)loadPreset:(id)sender {
	OSStatus status;

	CFIndex const idx = [sender tag];
	CFIndex const total = (NULL == presets) ? 0 : CFArrayGetCount(presets);
	if ((0 > idx) || (total <= idx))
	{
		NSAlert const *alert = [[NSAlert alloc] init];
		[alert setMessageText:@"Invalid preset selected"];
		[alert setInformativeText:[NSString stringWithFormat:@"Tried to select preset %ld of %ld",
															 (long)idx + 1,
															 (long)total]];
		[alert beginSheetModalForWindow:window modalDelegate:nil didEndSelector:NULL contextInfo:NULL];
		return;
	}

	AUPreset const *preset = (AUPreset const *)CFArrayGetValueAtIndex(presets, idx);
	status = AudioUnitSetProperty(effectUnit,
								  kAudioUnitProperty_PresentPreset,
								  kAudioUnitScope_Global,
								  0,
								  preset,
								  sizeof(AUPreset));
	if (noErr != status)
	{
		NSAlert const *alert = [[NSAlert alloc] init];
		[alert setMessageText:[NSString stringWithFormat:@"Error loading preset %@", preset->presetName]];
		[alert setInformativeText:[NSString stringWithFormat:@"Error %ld encountered while setting AudioUnit property",
															 (long)status]];
		[alert beginSheetModalForWindow:window modalDelegate:nil didEndSelector:NULL contextInfo:NULL];
		return;
	}

	AudioUnitParameter change = { effectUnit, kAUParameterListener_AnyParameter, 0, 0 };
	status = AUParameterListenerNotify(NULL, NULL, &change);
	if (noErr != status)
	{
		NSAlert const *alert = [[NSAlert alloc] init];
		[alert setMessageText:[NSString stringWithFormat:@"Error notifying of parameter changes for preset %@",
														 preset->presetName]];
		[alert setInformativeText:[NSString stringWithFormat:@"Error %ld encountered while sending notification",
															 (long)status]];
		[alert beginSheetModalForWindow:window modalDelegate:nil didEndSelector:NULL contextInfo:NULL];
		return;
	}
}

- (void)viewFrameDidChange:(NSNotification *)notification {
	NSRect const oldFrame = [window frame];
	NSRect const desired = [window frameRectForContentRect:[[notification object] frame]];
	NSRect const newFrame = NSMakeRect(oldFrame.origin.x,
									   oldFrame.origin.y + oldFrame.size.height - headerSize.height- desired.size.height,
									   desired.size.width,
									   headerSize.height + desired.size.height);
	[window setFrame:newFrame display:YES animate:NO];
}

@end


@interface AUEffectUtilAppDelegate : NSObject <NSApplicationDelegate>
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
	item = [menu addItemWithTitle:[NSString stringWithFormat:@"Open%C", (unichar)0x2026] action:@selector(openDocument:) keyEquivalent:@"o"];
	
	[menu addItem:[NSMenuItem separatorItem]];
	
	item = [menu addItemWithTitle:@"Close" action:@selector(performClose:) keyEquivalent:@"w"];
	item = [menu addItemWithTitle:@"Save" action:@selector(saveDocument:) keyEquivalent:@"s"];
	item = [menu addItemWithTitle:[NSString stringWithFormat:@"Save As%C", (unichar)0x2026] action:@selector(saveDocumentAs:) keyEquivalent:@"S"];
	item = [menu addItemWithTitle:@"Save All" action:@selector(saveAllDocuments:) keyEquivalent:@""];
	item = [menu addItemWithTitle:@"Revert to Saved" action:@selector(revertDocumentToSaved:) keyEquivalent:@"u"];
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
	[menu setValue:@"NSHelpMenu" forKey:@"name"];
	if ([NSApp respondsToSelector:@selector(setHelpMenu:)])
		[NSApp performSelector:@selector(setHelpMenu:) withObject:menu];

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
	[[NSApplication sharedApplication] setDelegate:delegate];
	[pool release];

	// Let's go!
	pool = [[NSAutoreleasePool alloc] init];
	[NSApp run];
	[delegate release];
	[pool release];
	return 0;
}
