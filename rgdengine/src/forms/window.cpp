#include "precompiled.h"
// forms
#include "window.h"
#include "drawing.h"

#pragma warning(disable: 4311) // warning C4311: 'type cast' : pointer truncation from '' to ''
#pragma warning(disable: 4312) // warning C4312: 'type cast' : conversion from '' to '' of greater size

namespace forms
{
	std::map<HWND, Window*> Window::ms_windows;

	template <typename T>
	void SetWndLong(HWND hWnd, int index, const T &value)
	{
		::SetWindowLong(hWnd, index, reinterpret_cast<LONG>(value));
	}

	template <typename T>
	T GetWndLong(HWND hWnd, int index)
	{
		return reinterpret_cast<T>(::GetWindowLong(hWnd, index));
	}

	LRESULT CALLBACK Window::MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Message msg(uMsg, wParam, lParam);

		if (uMsg == WM_NCCREATE)
		{
			//CREATESTRUCT *cs= (CREATESTRUCT *)(lParam);
			//SetWndLong<void*>(hWnd, GWL_USERDATA, cs->lpCreateParams);
			return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		Window *wnd	= ms_windows[hWnd];//GetWndLong<Window*>(hWnd, GWL_USERDATA);

		if (wnd)
		{
			switch (uMsg)
			{
			case WM_COMMAND:
				{
					MessageT<WM_COMMAND> command_msg(msg);
					if (!wnd->OnCommand(command_msg))
					{
						wnd->MessageDispatcher(msg);
						//return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
						//if (Window * subwnd = GetWndLong<Window*>(command_msg.Handle(), GWL_USERDATA))
						//{
						//	subwnd->OnCommand(command_msg);
						//}
					}
				}
				break;

			case WM_NOTIFY:
				{
					MessageT<WM_NOTIFY> notify_msg(msg);
					if (!wnd->OnNotify(notify_msg))
					{
						wnd->MessageDispatcher(msg);
						//return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
						//if (Window * subwnd = GetWndLong<Window*>(notify_msg.Handle(), GWL_USERDATA))
						//{
						//	subwnd->OnNotify(notify_msg);
						//}
					}
				}
				break;

			default:
				{
					wnd->MessageDispatcher(msg);
				}
			}
		}
		else 
			return ::DefWindowProc(hWnd, uMsg, wParam, lParam);


		return msg.lResult;
	}

	//LRESULT CALLBACK Window::SubWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	//{
	//	Message msg(uMsg, wParam, lParam);

	//	Window *wnd	= GetWndLong<Window*>(hWnd, GWL_USERDATA);
	//	if (wnd)
	//	{
	//		wnd->MessageDispatcher(msg);
	//	}

	//	return msg.lResult;
	//}

	void Window::MessageDispatcher(Message &msg)
	{
		MessageEventMap::iterator it;
		if ((it = m_MessageEventMap.find(msg.uMsg)) != m_MessageEventMap.end())
		{
			it->second(msg);
		}
		else
		{
			msg.lResult = ::CallWindowProc(m_SuperWindowProc, m_hwnd, msg.uMsg, msg.wParam, msg.lParam);
		}
	}

	bool Window::OnCommand(MessageT<WM_COMMAND> &msg)
	{
		return false;
	}

	bool Window::OnNotify(MessageT<WM_NOTIFY> &msg)
	{
		return false;
	}

	Window::Window()
		: m_hwnd(NULL),
		m_SuperWindowProc(::DefWindowProc)
	{
	}

	Window::~Window()
	{
		if (m_hwnd)
		{
			ms_windows.erase(m_hwnd);
			::DestroyWindow(m_hwnd);
		}
	}

	void Window::RegisterCls(const std::wstring ClassName, UINT Style, HICON hIcon, HCURSOR hCursor, HBRUSH hbrBackground, const std::wstring MenuName, HICON hIconSmall)
	{
		WNDCLASSEX wndclass	=
		{
			sizeof(WNDCLASSEX), Style, MainWindowProc, 0, 0, ::GetModuleHandle(NULL), hIcon, hCursor, hbrBackground, MenuName.c_str(), ClassName.c_str(), hIconSmall
		};
		::RegisterClassEx(&wndclass);
	}

	void Window::CreateWnd(HWND Parent, const std::wstring ClassName, const std::wstring Name, DWORD Style, DWORD ExStyle, UINT ID, const Drawing::Rectangle &Rect)
	{
		int x = Rect.left;
		int y = Rect.top;
		int width = Rect.GetWidth();
		int height = Rect.GetHeight();
		m_hwnd = ::CreateWindowExW(ExStyle, ClassName.c_str(), Name.c_str(), Style, Rect.left, Rect.top, width, height, Parent, (HMENU)ID, ::GetModuleHandle(NULL), this);
		ms_windows[m_hwnd]	= this;
	}

	//void Window::Subclass(Window *wnd)
	//{
	//	SetWndLong<void*>(wnd->m_hwnd, GWL_USERDATA, wnd);
	//	wnd->m_SuperWindowProc = GetWndLong<WNDPROC>(wnd->m_hwnd, GWL_WNDPROC);
	//	SetWndLong<WNDPROC>(wnd->m_hwnd, GWL_WNDPROC, SubWindowProc);
	//}

	void Window::SetMessageEvent(UINT uMsg, const MessageEvent &Event)
	{
		m_MessageEventMap[uMsg] = Event;
	}

	void Window::Show(bool flag)
	{
		ShowWindow(Handle(), flag ? SW_SHOW : SW_HIDE);
	}

	void Window::update()
	{
		UpdateWindow( Handle() );
	}
}