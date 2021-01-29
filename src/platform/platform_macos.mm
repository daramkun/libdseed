#include <dseed.h>

#include "common.hxx"

#if PLATFORM_MACOS

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#define WINDOW_STYLE_WINDOWED               (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable)
#define WINDOW_STYLE_SIZABLE_WINDOWED       (NSWindowStyleMaskResizable | NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable)

@interface DseedApplicationDelegate : NSObject<NSApplicationDelegate, NSWindowDelegate, NSWindowDelegate>
{
    NSWindow* _window;
    BOOL isRunning;
}

- (id) init;
- (BOOL) running;
- (NSWindow*) window;

- (void) exit;

@end

@implementation DseedApplicationDelegate

- (id) init
{
    if (self = [super init])
    {
        
    }
    return self;
}

- (BOOL) running
{
    return isRunning;
}

- (NSWindow*) window
{
    return _window;
}

- (void) applicationWillFinishLaunching:(NSNotification*)notification
{
    NSMenuItem* quitMenuItem = [[NSMenuItem alloc]
                                initWithTitle:@"Quit"
                                action:@selector(terminate:)
                                keyEquivalent:@"q"];
    
    NSMenu* appMenu = [NSMenu new];
    [appMenu addItem:quitMenuItem];
    
    NSMenuItem* appMenuItem = [NSMenuItem new];
    [appMenuItem setSubmenu:appMenu];
    
    NSMenu* menuBar = [NSMenu new];
    [menuBar addItem:appMenuItem];
    
    [NSApp setMainMenu: menuBar];
    
    _window = [[NSWindow alloc]
                initWithContentRect:CGRectMake(0, 0, 1280, 720)
                styleMask:WINDOW_STYLE_WINDOWED
                backing:NSBackingStoreBuffered
                defer:NO];
    _window.collectionBehavior = NSWindowCollectionBehaviorFullScreenPrimary;
    _window.delegate = self;
    _window.title = @"My Window";
    [_window center];
    
    [_window makeKeyAndOrderFront:self];
    
    isRunning = YES;
}

- (void) windowDidResize:(NSNotification*)notification
{
    NSLog(@"Did Resize");
}

- (BOOL) windowShouldClose:(NSWindow*)sender
{
    [NSApp terminate:nil];
    
    isRunning = NO;
    return YES;
}

- (void) exit
{
    [_window close];
}

@end

class __macos_application : public dseed::platform::application
{
public:
    __macos_application()
        : _refCount (1)
    {
        *&_inputDevices[dseed::io::inputdevicetype_keyboard] = new keyboard_common ();
		*&_inputDevices[dseed::io::inputdevicetype_mouse] = nullptr;
		*&_inputDevices[dseed::io::inputdevicetype_gamepad] = nullptr;
		*&_inputDevices[dseed::io::inputdevicetype_touchpanel] = nullptr;
		*&_inputDevices[dseed::io::inputdevicetype_accelerometer] = nullptr;
		*&_inputDevices[dseed::io::inputdevicetype_gyroscope] = nullptr;
		*&_inputDevices[dseed::io::inputdevicetype_gps] = nullptr;
    }

public:
	virtual int32_t retain () override { return ++_refCount; }
	virtual int32_t release () override
	{
		auto ret = --_refCount;
		if (ret == 0)
		{
			delete this;
			g_sharedApp = nullptr;
		}
		return ret;
	}

public:
	virtual dseed::error_t native_object (void** nativeObject) override
	{
		if (_appDelegate == nil)
			return dseed::error_invalid_op;
		if (nativeObject == nullptr)
			return dseed::error_invalid_args;
		*nativeObject = _appDelegate.window;
		return dseed::error_good;
	}

public:
	virtual dseed::error_t input_device (dseed::io::inputdevicetype type, dseed::io::inputdevice** device) override
	{
		if (_appDelegate == nil)
			return dseed::error_invalid_op;

		if (type > dseed::io::inputdevicetype_gps || type < dseed::io::inputdevicetype_keyboard)
			return dseed::error_invalid_args;

		if (_inputDevices[(int)type] == nullptr)
			return dseed::error_not_support;

		*device = _inputDevices[(int)type];
		(*device)->retain ();

		return dseed::error_good;
	}

public:
	virtual dseed::error_t set_title (const char* title) override
	{
		if (_appDelegate == nil) return dseed::error_invalid_op;
		if (title == nullptr) title = "";
		_appDelegate.window.title = [NSString stringWithUTF8String:title];
		return dseed::error_good;
	}

public:
	virtual dseed::error_t client_size (dseed::size2i* size) override
	{
		if (_appDelegate == nil) return dseed::error_invalid_op;
		if (size == nullptr) return dseed::error_invalid_args;

        NSRect viewSize = _appDelegate.window.contentView.frame;

		size->width = viewSize.size.width;
		size->height = viewSize.size.height;

		return dseed::error_good;
	}

	virtual dseed::error_t set_client_size (const dseed::size2i& size) override
	{
		if (_appDelegate == nil) return dseed::error_invalid_op;

		[_appDelegate.window setContentSize:CGSizeMake(size.width, size.height)];

		return dseed::error_good;
	}

public:
    virtual dseed::error_t window_mode (dseed::platform::windowmode* mode) override
	{
		if (_appDelegate == nil) return dseed::error_invalid_op;

		int style = _appDelegate.window.styleMask;

        if ((style & NSWindowStyleMaskFullScreen) != 0)
            *mode = dseed::platform::windowmode::borderless_fullscreen;
		else if ((style & WINDOW_STYLE_WINDOWED) == WINDOW_STYLE_WINDOWED)
			*mode = dseed::platform::windowmode::windowed;
		else if ((style & WINDOW_STYLE_SIZABLE_WINDOWED) == WINDOW_STYLE_SIZABLE_WINDOWED)
			*mode = dseed::platform::windowmode::sizable_windowed;
		else
			*mode = dseed::platform::windowmode::unknown;

		return dseed::error_good;
	}

	virtual dseed::error_t set_window_mode (dseed::platform::windowmode mode) override
	{
		if (_appDelegate == nil) return dseed::error_invalid_op;

		switch (mode)
		{
			case dseed::platform::windowmode::windowed:
				_appDelegate.window.styleMask = WINDOW_STYLE_WINDOWED;
				break;

			case dseed::platform::windowmode::sizable_windowed:
				_appDelegate.window.styleMask =  WINDOW_STYLE_SIZABLE_WINDOWED;
				break;

			case dseed::platform::windowmode::borderless_fullscreen:
                [_appDelegate.window toggleFullScreen:_appDelegate];
				break;

			default:
				return dseed::error_fail;
		}

		return dseed::error_good;
	}

public:
	virtual dseed::error_t dpi(dseed::size2i* dpi) override
    {
		if (_appDelegate == nil) return dseed::error_invalid_op;

        NSDictionary* deviceDescription = [_appDelegate.window deviceDescription];
        NSSize resolution = [[deviceDescription objectForKey:NSDeviceResolution] sizeValue];

        float backingScaleFactor = [_appDelegate.window backingScaleFactor];

        dpi->width = (int)(resolution.width * backingScaleFactor);
        dpi->height = (int)(resolution.height * backingScaleFactor);

        return dseed::error_good;
    }

public:
	virtual dseed::error_t event_handler (dseed::platform::event_handler** handler) override
	{
		if (handler == nullptr) return dseed::error_invalid_args;
		*handler = _handler;
		return dseed::error_good;
	}
	virtual dseed::error_t set_event_handler (dseed::platform::event_handler* handler) override
	{
		_handler = handler;
		if (_handler == nullptr)
			_handler = new dummy_handler ();
		return dseed::error_good;
	}

public:
	virtual dseed::error_t run (dseed::platform::event_handler* handler) override
    {
        if (_appDelegate != nil)
			return dseed::error_invalid_op;

		set_event_handler (handler);

        @autoreleasepool {
            [NSApplication sharedApplication];
            [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
            [NSApp setPresentationOptions:NSApplicationPresentationDefault];
            [NSApp activateIgnoringOtherApps:YES];
            
            _appDelegate = [[DseedApplicationDelegate alloc] init];
            NSApp.delegate = _appDelegate;
            [NSApp finishLaunching];
            
		    dseed::timespan lastTime = dseed::timespan::current_ticks (), currentTime;
            while (_appDelegate.running) {
                doEvents();
                
                for (dseed::autoref<inputdevice_internal>& inputdevice : _inputDevices)
                    if (inputdevice != nullptr)
                        inputdevice->update ();

                currentTime = dseed::timespan::current_ticks ();
                _handler->next_frame (currentTime - lastTime);

                for (dseed::autoref<inputdevice_internal>& inputdevice : _inputDevices)
                    if (inputdevice != nullptr)
                        inputdevice->cleanup ();

                lastTime = currentTime;

                [NSThread sleepForTimeInterval:0];
            }
            
            NSApp.delegate = nil;
            _appDelegate = nil;
        }

        return dseed::error_good;
    }

public:
	virtual dseed::error_t exit () override
	{
		if (_appDelegate == nil)
			return dseed::error_invalid_op;

        [_appDelegate exit];

		return dseed::error_good;
	}

private:
    void doEvents()
    {
        @autoreleasepool {
            NSEvent* event;
            while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                            untilDate:nil
                                            inMode:NSDefaultRunLoopMode
                                            dequeue:YES]) != nil)
            {
                if (event != nil)
                {
                    internalDoEvents();
                    [NSApp sendEvent:event];
                    [NSApp updateWindows];
                }
                
                event = nil;
            }
        }
    }

    void internalDoEvents()
    {

    }

private:
    std::atomic<int32_t> _refCount;

    DseedApplicationDelegate* _appDelegate;
	dseed::autoref<inputdevice_internal> _inputDevices[8];

	dseed::autoref<dseed::platform::event_handler> _handler;
};

dseed::error_t dseed::platform::create_application (dseed::platform::application** app)
{
	if (g_sharedApp != nullptr) return dseed::error_invalid_op;
	if (app == nullptr) return dseed::error_invalid_args;

	*app = g_sharedApp = new __macos_application ();
	if (*app == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}

#endif