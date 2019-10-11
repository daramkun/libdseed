#include <dseed.h>

#if PLATFORM_UWP

#	include "platform.internal.h"

class __uwp_application : public dseed::application
{
public:
	__uwp_application ()
		: _refCount (1), _window (nullptr), _handler (new dummy_handler ())
	{

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
		if (_window == nullptr)
			return dseed::error_invalid_op;
		if (nativeObject == nullptr)
			return dseed::error_invalid_args;
		*nativeObject = (void*)_window.Get ();
		return dseed::error_good;
	}

public:
	virtual dseed::error_t input_device (dseed::inputdevice_t type, dseed::inputdevice** device) override
	{
		if (_window == nullptr)
			return dseed::error_invalid_op;


		return dseed::error_good;
	}

public:
	virtual dseed::error_t set_title (const char* title) override
	{
		if (_window == nullptr)
			return dseed::error_invalid_op;

		//Windows::ApplicationModel::Core::CoreApplication::GetCurrentView()->TitleBar
		return dseed::error_platform_not_support;
	}

public:
	virtual dseed::error_t client_size (dseed::size2i* size) override
	{
		if (_window == nullptr)
			return dseed::error_invalid_op;

		auto bounds = _window->Bounds;
		*size = dseed::size2i ((int)bounds.Width, (int)bounds.Height);
		return dseed::error_good;
	}
	virtual dseed::error_t set_client_size (const dseed::size2i& size) override
	{
		if (_window == nullptr)
			return dseed::error_invalid_op;
		

		return dseed::error_good;
	}

public:
	virtual dseed::error_t window_mode (dseed::windowmode* mode) override
	{
		if (_window == nullptr)
			return dseed::error_invalid_op;

		if (Windows::UI::ViewManagement::ApplicationView::GetForCurrentView ()->IsFullScreenMode)
			* mode = dseed::windowmode_borderless_fullscreen;
		else
			*mode = dseed::windowmode_sizable_windowed;

		return dseed::error_good;
	}
	virtual dseed::error_t set_window_mode (dseed::windowmode mode) override
	{
		if (_window == nullptr)
			return dseed::error_invalid_op;

		if (mode == dseed::windowmode_borderless_fullscreen)
		{
			if (!Windows::UI::ViewManagement::ApplicationView::GetForCurrentView ()->TryEnterFullScreenMode ())
				return dseed::error_fail;
		}
		else if (mode == dseed::windowmode_sizable_windowed)
		{
			Windows::UI::ViewManagement::ApplicationView::GetForCurrentView ()->ExitFullScreenMode ();
		}
		else return dseed::error_invalid_args;

		return dseed::error_good;
	}

public:
	virtual dseed::error_t event_handler (dseed::event_handler** handler)
	{
		if (handler == nullptr) return dseed::error_invalid_args;
		*handler = _handler;
		return dseed::error_good;
	}
	virtual dseed::error_t set_event_handler (dseed::event_handler* handler)
	{
		_handler = handler;
		if (_handler == nullptr)
			_handler = new dummy_handler ();
		return dseed::error_good;
	}

public:
	virtual dseed::error_t run (dseed::event_handler* handler) override;

private:
	std::atomic<int32_t> _refCount;

	dseed::auto_object<dseed::event_handler> _handler;

public:
	Platform::Agile<Windows::UI::Core::CoreWindow> _window;
};

namespace dseed
{
	namespace uwp
	{
		ref class DSeedFrameworkView sealed : public Windows::ApplicationModel::Core::IFrameworkView
		{
		internal:
			DSeedFrameworkView (__uwp_application* app)
				: _app (app)
			{
				_app->retain ();
			}

		public:
			virtual ~DSeedFrameworkView ()
			{
				_app->release ();
				_app = nullptr;
			}

		public:
			virtual void Initialize (Windows::ApplicationModel::Core::CoreApplicationView^ appView)
			{
				using namespace Windows::Foundation;
				using namespace Windows::ApplicationModel::Core;
				using namespace Windows::ApplicationModel::Activation;
				appView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^> (this, &DSeedFrameworkView::OnActivated);
			}

			virtual void SetWindow (Windows::UI::Core::CoreWindow^ window)
			{
				_app->_window = window;

				using namespace Windows::Foundation;
				using namespace Windows::UI::Core;
				window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^> (this, &DSeedFrameworkView::OnVisibilityChanged);
				window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^> (this, &DSeedFrameworkView::OnWindowClosed);
				window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^> (this, &DSeedFrameworkView::OnResized);
				window->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^> (this, &DSeedFrameworkView::OnKeyDown);
				window->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^> (this, &DSeedFrameworkView::OnKeyUp);
				window->PointerPressed += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^> (this, &DSeedFrameworkView::OnPointerPressed);
				window->PointerReleased += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^> (this, &DSeedFrameworkView::OnPointerReleased);
				window->PointerMoved += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^> (this, &DSeedFrameworkView::OnPointerMoved);
				window->PointerWheelChanged += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^> (this, &DSeedFrameworkView::OnPointerWheel);
			}

			virtual void Load (Platform::String^ entryPoint) { }

			virtual void Run ()
			{
				dseed::timespan_t lastTime = dseed::timespan_t::current_ticks (), currentTime;
				while (!_windowClosed) {
					dseed::auto_object<dseed::event_handler> handler;
					_app->event_handler (&handler);

					currentTime = dseed::timespan_t::current_ticks ();
					handler->next_frame (currentTime - lastTime);
					lastTime = currentTime;

					_app->_window->Dispatcher->ProcessEvents (Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent);
				}
			}

			virtual void Uninitialize ()
			{

			}

		private:
			void OnActivated (Windows::ApplicationModel::Core::CoreApplicationView^ CoreAppView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ Args)
			{
				Windows::UI::Core::CoreWindow^ window = Windows::UI::Core::CoreWindow::GetForCurrentThread ();
				window->Activate ();
				_windowClosed = false;
			}

		private:
			void OnVisibilityChanged (Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ e)
			{
				_windowVisible = true;
			}
			void OnWindowClosed (Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ e)
			{
				_windowClosed = true;
			}
			void OnResized (Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ e)
			{
				dseed::auto_object<dseed::event_handler> handler;
				_app->event_handler (&handler);
				handler->resized ();
			}
			void OnKeyDown (Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ e)
			{
				//PRApp::sharedApp ()->getGame ()->onKeyDown (keyConv ((int)e->VirtualKey));
			}
			void OnKeyUp (Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ e)
			{
				//PRApp::sharedApp ()->getGame ()->onKeyUp (keyConv ((int)e->VirtualKey));
			}
			void OnPointerPressed (Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ e)
			{
				/*if (PRApp::sharedApp ()->getGame ()) {
					int mouseButton;
					if (e->CurrentPoint->Properties->IsLeftButtonPressed) mouseButton |= PRMButton_Left;
					else if (e->CurrentPoint->Properties->IsRightButtonPressed) mouseButton |= PRMButton_Right;
					else if (e->CurrentPoint->Properties->IsMiddleButtonPressed) mouseButton |= PRMButton_Middle;
					PRApp::sharedApp ()->getGame ()->onMouseDown ((PRMButton)mouseButton,
						(int)e->CurrentPoint->Position.X, (int)e->CurrentPoint->Position.Y);
				}*/
			}
			void OnPointerReleased (Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ e)
			{
				/*if (PRApp::sharedApp ()->getGame ()) {
					int mouseButton;
					if (!e->CurrentPoint->Properties->IsLeftButtonPressed) mouseButton |= PRMButton_Left;
					else if (!e->CurrentPoint->Properties->IsRightButtonPressed) mouseButton |= PRMButton_Right;
					else if (!e->CurrentPoint->Properties->IsMiddleButtonPressed) mouseButton |= PRMButton_Middle;
					PRApp::sharedApp ()->getGame ()->onMouseUp ((PRMButton)mouseButton,
						(int)e->CurrentPoint->Position.X, (int)e->CurrentPoint->Position.Y);
				}*/
			}
			void OnPointerMoved (Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ e)
			{
				/*if (PRApp::sharedApp ()->getGame ()) {
					int mouseButton = PRMButton_None;
					if (e->CurrentPoint->Properties->IsLeftButtonPressed) mouseButton |= PRMButton_Left;
					else if (e->CurrentPoint->Properties->IsRightButtonPressed) mouseButton |= PRMButton_Right;
					else if (e->CurrentPoint->Properties->IsMiddleButtonPressed) mouseButton |= PRMButton_Middle;
					PRApp::sharedApp ()->getGame ()->onMouseMove ((PRMButton)mouseButton,
						(int)e->CurrentPoint->Position.X, (int)e->CurrentPoint->Position.Y);
				}*/
			}

			void OnPointerWheel (Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ e)
			{
				//PRApp::sharedApp ()->getGame ()->onMouseWheel (0, e->CurrentPoint->Properties->MouseWheelDelta);
			}

		private:
			bool _windowClosed;
			bool _windowVisible;

			__uwp_application* _app;
		};

		ref class DSeedFrameworkViewSource sealed : public Windows::ApplicationModel::Core::IFrameworkViewSource
		{
		internal:
			DSeedFrameworkViewSource (__uwp_application* app) : _app (app) { _app->retain (); }

		public:
			virtual ~DSeedFrameworkViewSource () { _app->release (); }

		public:
			virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView ()
			{
				/*Windows::UI::ViewManagement::ApplicationView::PreferredLaunchViewSize = Windows::Foundation::Size (
					800,
					(800 * (GetSystemMetrics (SM_CYSCREEN) / (float)GetSystemMetrics (SM_CXSCREEN)))
				);
				Windows::UI::ViewManagement::ApplicationView::PreferredLaunchWindowingMode = Windows::UI::ViewManagement::ApplicationViewWindowingMode::PreferredLaunchViewSize;*/
				return ref new DSeedFrameworkView (_app);
			}

		private:
			__uwp_application* _app;
		};
	}
}

dseed::error_t __uwp_application::run (dseed::event_handler* handler)
{
	set_event_handler (handler);

	try
	{
		auto frameworkViewSource = ref new dseed::uwp::DSeedFrameworkViewSource (this);
		Windows::ApplicationModel::Core::CoreApplication::Run (frameworkViewSource);
	}
	catch (Platform::InvalidArgumentException^ ex)
	{
		char utf8msg[256];
		dseed::utf16_to_utf8 ((const char16_t*)ex->Message->Data (), utf8msg, sizeof (utf8msg));
		dseed::logger::shared_logger ()->write (dseed::loglevel_error, "DSeed", utf8msg);

		return dseed::error_fail;
	}

	return dseed::error_good;
}

dseed::error_t dseed::create_application (dseed::application** app)
{
	if (g_sharedApp != nullptr) return dseed::error_invalid_op;
	if (app == nullptr) return dseed::error_invalid_args;
	
	*app = g_sharedApp = new __uwp_application ();
	if (*app == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}

#endif
