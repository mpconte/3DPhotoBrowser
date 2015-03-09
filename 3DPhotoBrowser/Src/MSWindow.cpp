/**
 * @file MSWindow.cpp
 * @brief Microsoft window implementation file
 */

#include "MSWindow.h"
#include "PhotoBrowser.h"

//-----------------------------------------------------------------------------------------------------------------------------
// MSWindow

map<HWND, MSWindow*> MSWindow::sWindowMapping;
map<HWND, PreferenceData*> MSWindow::sControlMapping;
bool MSWindow::sWindowClassRegistered = false;
const char* MSWindow::sWindowClassName = "3DPhotoBrowser";

//-----------------------------------------------------------------------------------------------------------------------------

MSWindow::MSWindow()
: mWindowHandle(NULL)
, mDialogHandle(NULL)
, mDeviceContext(NULL)
{
	// Listen for user preference changes
	UserPreferences::Instance()->AddUserPreferenceListener(this);
}

//-----------------------------------------------------------------------------------------------------------------------------

bool MSWindow::Init(const string& in_Title, int in_SizeX, int in_SizeY)
{
	// Make sure the window class is registered
	if(!RegisterWindowClass())
	{
		logf("Failed to register window class");
		return false;
	}

	DWORD l_ExStyle = NULL;
	DWORD l_Style = WS_OVERLAPPEDWINDOW;

	// Adjust the window size so the client rectangle is equal to the input size
	RECT l_Rect;
	l_Rect.top = 0;
	l_Rect.left = 0;
	l_Rect.right = in_SizeX;
	l_Rect.bottom = in_SizeY;
	AdjustWindowRectEx(&l_Rect, l_Style, false, l_ExStyle);

	// Try to create the window
	mWindowHandle = CreateWindowEx(l_ExStyle, sWindowClassName, in_Title.c_str(), l_Style,
								  CW_USEDEFAULT, 0, l_Rect.right - l_Rect.left, l_Rect.bottom - l_Rect.top, NULL, NULL, GetModuleHandle(NULL), NULL);

	// Check if the window was created successfully
	if(!mWindowHandle)
	{
		logf("Failed to create window");
		return false;
	}

	// Create the use preferences dialog
	if(!CreateUsePreferenceDialog())
	{
		logf("Failed to create user preferences dialog");
		return false;
	}

	// Try to create the device context that will be used for rendering
	if(!CreateDeviceContext())
	{
		logf("Failed to create device context");
		return false;
	}

	// Show and initially update the window
	ShowWindow(mWindowHandle, SW_SHOW);
	UpdateWindow(mWindowHandle);

	// Add a mapping for this window now that it has been intialized
	sWindowMapping[mWindowHandle] = this;

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------------

void MSWindow::SetTitle(const string& in_Title)
{
	SetWindowText(mWindowHandle, in_Title.c_str());
}

//-----------------------------------------------------------------------------------------------------------------------------

bool MSWindow::Destroy()
{
	// Release the device context
	if(mDeviceContext)
	{
		ReleaseDC(mWindowHandle, mDeviceContext);
		mDeviceContext = NULL;
	}

	// Destroy the user preferences dialog
	DestroyWindow(mDialogHandle);
	mDialogHandle = NULL;

	// Remove the mapping once the window is destroyed
	sWindowMapping.erase(mWindowHandle);
	mWindowHandle = NULL;

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------------

void MSWindow::ProcessMessages()
{
	// Process any queued window messages
	MSG l_Msg;
	while(PeekMessage(&l_Msg, NULL, 0, 0, PM_REMOVE))
	{
		// First check if the message is destined for the dialog
		if(!IsDialogMessage(mDialogHandle, &l_Msg))
		{
			// Translate and dispatch to our WNDPROC
			TranslateMessage(&l_Msg);
			DispatchMessage(&l_Msg);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------------

void MSWindow::SwapBuffers()
{
	::SwapBuffers(mDeviceContext);
}

//-----------------------------------------------------------------------------------------------------------------------------

void MSWindow::EnableVerticalSync(bool in_Enable)
{
	typedef BOOL (WINAPI * VSyncProc)(INT);
	static VSyncProc VSyncFunc = (VSyncProc)wglGetProcAddress("wglSwapIntervalEXT");
	if(VSyncFunc)
	{
		VSyncFunc(in_Enable ? 1 : 0);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------

void MSWindow::TranslateKey(unsigned& in_Key)
{
	switch(in_Key)
	{
	case VK_HOME: in_Key = KeyboardListener::VirtualKey_Home; break;
	case VK_SHIFT: in_Key = KeyboardListener::VirtualKey_Shift; break;
	case VK_PRIOR: in_Key = KeyboardListener::VirtualKey_PageUp; break;
	case VK_NEXT: in_Key = KeyboardListener::VirtualKey_PageDown; break;
	case VK_F1: in_Key = KeyboardListener::VirtualKey_F1; break;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------

void MSWindow::ShowUserPreferencesDialog(bool in_Show)
{
	ShowWindow(mDialogHandle, in_Show ? SW_SHOW : SW_HIDE);
}

//-----------------------------------------------------------------------------------------------------------------------------

bool MSWindow::IsUserPreferencesDialogVisible()
{
	return IsWindowVisible(mDialogHandle) == TRUE;
}

//-----------------------------------------------------------------------------------------------------------------------------

GLContext MSWindow::CreateGLContext()
{
	return (GLContext)wglCreateContext(mDeviceContext);
}

//-----------------------------------------------------------------------------------------------------------------------------

bool MSWindow::ShareGLContexts(GLContext in_Context1, GLContext in_Context2)
{
	return wglShareLists((HGLRC)in_Context1, (HGLRC)in_Context2) == TRUE;
}

//-----------------------------------------------------------------------------------------------------------------------------

bool MSWindow::AcquireGLContext(GLContext in_Context)
{
	return wglMakeCurrent(mDeviceContext, (HGLRC)in_Context) == TRUE;
}

//-----------------------------------------------------------------------------------------------------------------------------

bool MSWindow::ReleaseGLContext(GLContext in_Context)
{
	wglMakeCurrent(NULL, NULL);
	return wglDeleteContext((HGLRC)in_Context) == TRUE;
}

//-----------------------------------------------------------------------------------------------------------------------------

void MSWindow::OnUserPreferenceUpdate()
{
	HWND l_LayoutControl = GetDlgItem(mDialogHandle, CurrentLayout);
	SendMessage(l_LayoutControl, CB_SETCURSEL, UserPreferences::Instance()->CurrentLayout(), 0);
}

//-----------------------------------------------------------------------------------------------------------------------------

bool MSWindow::CreateDeviceContext()
{
	// Release the previous device context if it existed
	if(mDeviceContext)
	{
		ReleaseDC(mWindowHandle, mDeviceContext);
	}

	// Create a pixel format based on the template
	PIXELFORMATDESCRIPTOR l_PixelFormat = 
	{
		sizeof(PIXELFORMATDESCRIPTOR),								// WORD nSize;
		1,															// WORD nVersion;
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,	// DWORD dwFlags;
		PFD_TYPE_RGBA,												// BYTE iPixelType;
		32,															// BYTE cColorBits;
		0,															// BYTE cRedBits;
		0,															// BYTE cRedShift;
		0,															// BYTE cGreenBits;
		0,															// BYTE cGreenShift;
		0,															// BYTE cBlueBits;
		0,															// BYTE cBlueShift;
		0,															// BYTE cAlphaBits;
		0,															// BYTE cAlphaShift;
		0,															// BYTE cAccumBits;
		0,															// BYTE cAccumRedBits;
		0,															// BYTE cAccumGreenBits;
		0,															// BYTE cAccumBlueBits;
		0,															// BYTE cAccumAlphaBits;
		16,															// BYTE cDepthBits;
		0,															// BYTE cStencilBits;
		0,															// BYTE cAuxBuffers;
		PFD_MAIN_PLANE,												// BYTE iLayerType;
		0,															// BYTE bReserved;
		0,															// DWORD dwLayerMask;
		0,															// DWORD dwVisibleMask;
		0															// DWORD dwDamageMask;
	};

	// Get the window device context
	mDeviceContext = GetDC(mWindowHandle);
	if(!mDeviceContext)
	{
		logf("Failed to get window device context: ErrorCode=%d", GetLastError());
		return false;
	}

	// Choose a pixel format appropriate for the window
	int l_BestMatchPixelFormat = ChoosePixelFormat(mDeviceContext, &l_PixelFormat);
	if(!l_BestMatchPixelFormat)
	{
		logf("Failed to choose pixel format: ErrorCode=%d", GetLastError());
		return false;
	}

	// Use the selected pixel format
	if(!SetPixelFormat(mDeviceContext, l_BestMatchPixelFormat, &l_PixelFormat))
	{
		logf("Failed to set pixel format: ErrorCode=%d", GetLastError());
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------------

bool MSWindow::CreateUsePreferenceDialog()
{
	mDialogHandle = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_USERPREFERENCESDIALOG),
								 mWindowHandle, DialogProc);

	// Check if the dialog was created successfully
	if(!mDialogHandle)
	{
		return false;
	}

	// Dynamically add the user preferences to the dialog
	int l_StartX = 10;
	int l_StartY = 5;
	int l_PosX = l_StartX;
	int l_PosY = l_StartY;
	int l_ControlWidth = 150;
	int l_ControlHeight = 20;
	vector<PreferenceData>& l_Prefs = UserPreferences::Instance()->GetPreferenceData();
	for(unsigned i = 0; i < l_Prefs.size(); i++)
	{
		PreferenceData& l_PrefData = l_Prefs[i];

		l_PosX = l_StartX;

		// Create the control label
		CreateWindowEx(NULL, "STATIC", l_PrefData.GetDescription(), 
			WS_CHILD | WS_VISIBLE, 
			l_PosX, l_PosY, 190, l_ControlHeight,
			mDialogHandle, NULL, GetModuleHandle(NULL), NULL);

		l_PosX += 190;

		HWND l_ControlHwnd = NULL;

		// Determine the data type, and convert the data to a string
		char l_Buff[64];
		switch(l_PrefData.GetType())
		{
		case PreferenceType_int:

			// Create an edit box
			sprintf_s(l_Buff, 64, "%d", l_PrefData.Get<int>());
			l_ControlHwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", l_Buff, 
				WS_CHILD | WS_VISIBLE | ES_NUMBER, 
				l_PosX, l_PosY, l_ControlWidth, l_ControlHeight,
				mDialogHandle, (HMENU)l_PrefData.GetId(), GetModuleHandle(NULL), NULL);
			break;

		case PreferenceType_float:
		{
			// Create an edit box
			stringstream l_Buff;
			l_Buff << l_PrefData.Get<float>();
			l_ControlHwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", l_Buff.str().c_str(), 
				WS_CHILD | WS_VISIBLE,
				l_PosX, l_PosY, l_ControlWidth, l_ControlHeight,
				mDialogHandle, (HMENU)l_PrefData.GetId(), GetModuleHandle(NULL), NULL);

			break;
		}
		case PreferenceType_bool:

			// Create a check box
			l_ControlHwnd = CreateWindowEx(NULL, "BUTTON", "", 
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				l_PosX, l_PosY, l_ControlWidth, l_ControlHeight,
				mDialogHandle, (HMENU)l_PrefData.GetId(), GetModuleHandle(NULL), NULL);

			// Initialize the checkbox state
			SendMessage(l_ControlHwnd, BM_SETCHECK, l_PrefData.Get<bool>() ? BST_CHECKED : BST_UNCHECKED, 0);

			break;

		case PreferenceType_LayoutIndex:

			l_ControlHwnd = CreateWindowEx(NULL, "COMBOBOX", "", 
				WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
				l_PosX, l_PosY, l_ControlWidth, 100,
				mDialogHandle, (HMENU)l_PrefData.GetId(), GetModuleHandle(NULL), NULL);

			// Initialize the contexts of the layout combo box
			for(unsigned i = 0; i < PhotoBrowser::Instance()->GetRegisteredLayoutCount(); i++)
			{
				const char* l_LayoutName = PhotoBrowser::Instance()->GetRegisteredLayout(i)->GetName();
				SendMessage(l_ControlHwnd, CB_ADDSTRING, 0, (LPARAM)l_LayoutName);
			}

			// Initialize the combo box to display the correct entry in the list
			SendMessage(l_ControlHwnd, CB_SETCURSEL, l_PrefData.Get<LayoutIndex>(), 0);

			break;

		case PreferenceType_FloatSlider:

			float l_LabelPct = 0.25f;
			float l_ControlPct = 1 - l_LabelPct;

			// Create the FloatSlider control
			l_ControlHwnd = CreateWindowEx(NULL, TRACKBAR_CLASS, 0, 
				WS_CHILD | WS_VISIBLE | TBS_NOTICKS,
				l_PosX, l_PosY, int(l_ControlWidth * l_ControlPct), l_ControlHeight,
				mDialogHandle, (HMENU)l_PrefData.GetId(), GetModuleHandle(NULL), NULL);

			// Create the FloatSlider value label. Its id is related to the FloatSlider id so they can be identified with one another
			CreateWindowEx(NULL, "STATIC", "0.0", 
				WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
				int(l_PosX + l_ControlWidth * l_ControlPct), l_PosY, int(l_ControlWidth * l_LabelPct), l_ControlHeight,
				mDialogHandle, (HMENU)(l_PrefData.GetId() | 0x80000000), GetModuleHandle(NULL), NULL);

			// Initialize the FloatSlider granularity settings
			SendMessage(l_ControlHwnd, TBM_SETRANGE, TRUE, MAKELONG(0, MAX_SLIDER_VALUE));
			SendMessage(l_ControlHwnd, TBM_SETPAGESIZE, 0, 1);

			// Initialize the FloatSlider position
			float l_Pos = (l_PrefData.Get<float>() - MIN_IMAGE_SIZE) / (MAX_IMAGE_SIZE - MIN_IMAGE_SIZE);
			SendMessage(l_ControlHwnd, TBM_SETPOS, TRUE, DWORD(l_Pos * MAX_SLIDER_VALUE));

			// Send the WM_HSCROLL message to init the FloatSlider label
			sControlMapping[l_ControlHwnd] = &l_PrefData;
			SendMessage(mDialogHandle, WM_HSCROLL, 0, (LPARAM)l_ControlHwnd);

			break;
		}

		// Store the pref data for this control handle
		sControlMapping[l_ControlHwnd] = &l_PrefData;

		l_PosX += l_ControlWidth;
		l_PosY += l_ControlHeight + 2;
	}

	// Detmine the height of the title bar so we can resize the window correctly
	TITLEBARINFO l_Info;
	l_Info.cbSize = sizeof(TITLEBARINFO);
	GetTitleBarInfo(mDialogHandle, &l_Info);

	// Adjust the vertical size by the title bar size
	l_PosY += l_Info.rcTitleBar.bottom - l_Info.rcTitleBar.top;
	
	// Resize the window to contain all the controls correctly
	// And position it to the right of the parent window
	RECT l_Rect;
	GetWindowRect(mWindowHandle, &l_Rect);
	SetWindowPos(mDialogHandle, NULL, l_Rect.right + 20, l_Rect.top, l_PosX + 20, l_PosY + 10, SWP_NOZORDER);

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------------

bool MSWindow::RegisterWindowClass()
{
	// If the window class has already been registered, then nothing to do
	if(sWindowClassRegistered)
	{
		return true;
	}
	sWindowClassRegistered = true;

	WNDCLASSEX wcex;
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WindowMessageRedirector;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= GetModuleHandle(NULL);
	wcex.hIcon			= 0;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= sWindowClassName;
	wcex.hIconSm		= 0;

	return RegisterClassEx(&wcex) != 0;
}

//-----------------------------------------------------------------------------------------------------------------------------

LRESULT CALLBACK MSWindow::WindowMessageRedirector(HWND in_Hwnd, UINT in_Message, WPARAM in_wParam, LPARAM in_lParam)
{
	// Find the window associate with this window handle
	MSWindow* l_Window = sWindowMapping[in_Hwnd];
	if(!l_Window)
	{
		return DefWindowProc(in_Hwnd, in_Message, in_wParam, in_lParam);
	}

	// For transforming mouse y coordinates
	RECT l_Rect;

	// For tracking mouse leave events
	TRACKMOUSEEVENT l_TrackMouseEvent =
	{
		sizeof(TRACKMOUSEEVENT),
		TME_LEAVE,
		in_Hwnd,
		0 
	};

	switch (in_Message)
	{
	case WM_CREATE:

		// The UPF dialog uses a FloatSlider widget which requires the common controls dll to be loaded
		INITCOMMONCONTROLSEX l_CControls;
		l_CControls.dwSize = sizeof(INITCOMMONCONTROLSEX);
		l_CControls.dwICC = ICC_BAR_CLASSES;
		InitCommonControlsEx(&l_CControls);
		break;

	case WM_SIZE:
		
		l_Window->FireResizeEvent(LOWORD(in_lParam), HIWORD(in_lParam));
		break;

	case WM_MOUSEMOVE:

		GetClientRect(in_Hwnd, &l_Rect);
		l_Window->FireMouseMoveEvent(LOWORD(in_lParam), l_Rect.bottom - HIWORD(in_lParam));
		break;

	case WM_MOUSEWHEEL:

		l_Window->FireMouseWheelEvent((short)HIWORD(in_wParam) / WHEEL_DELTA);
		break;

	case WM_LBUTTONUP:

		GetClientRect(in_Hwnd, &l_Rect);
		l_Window->FireMouseReleaseEvent(MouseListener::MouseButton_Left, LOWORD(in_lParam), l_Rect.bottom - HIWORD(in_lParam));
		break;

	case WM_RBUTTONUP:

		GetClientRect(in_Hwnd, &l_Rect);
		l_Window->FireMouseReleaseEvent(MouseListener::MouseButton_Right, LOWORD(in_lParam), l_Rect.bottom - HIWORD(in_lParam));
		break;

	case WM_MBUTTONUP:

		GetClientRect(in_Hwnd, &l_Rect);
		l_Window->FireMouseReleaseEvent(MouseListener::MouseButton_Middle, LOWORD(in_lParam), l_Rect.bottom - HIWORD(in_lParam));
		break;

	case WM_LBUTTONDOWN:

		// TrackMouseEvent to receive a WM_MOUSELEAVE notify when the mouse leaves the client area
		TrackMouseEvent(&l_TrackMouseEvent);
		GetClientRect(in_Hwnd, &l_Rect);
		l_Window->FireMouseClickEvent(MouseListener::MouseButton_Left, LOWORD(in_lParam), l_Rect.bottom - HIWORD(in_lParam));
		break;

	case WM_RBUTTONDOWN:

		// TrackMouseEvent to receive a WM_MOUSELEAVE notify when the mouse leaves the client area
		TrackMouseEvent(&l_TrackMouseEvent);
		GetClientRect(in_Hwnd, &l_Rect);
		l_Window->FireMouseClickEvent(MouseListener::MouseButton_Right, LOWORD(in_lParam), l_Rect.bottom - HIWORD(in_lParam));
		break;

	case WM_MBUTTONDOWN:

		// TrackMouseEvent to receive a WM_MOUSELEAVE notify when the mouse leaves the client area
		TrackMouseEvent(&l_TrackMouseEvent);
		GetClientRect(in_Hwnd, &l_Rect);
		l_Window->FireMouseClickEvent(MouseListener::MouseButton_Middle, LOWORD(in_lParam), l_Rect.bottom - HIWORD(in_lParam));
		break;

	case WM_KEYUP:

		l_Window->FireKeyUpEvent(in_wParam);
		break;

	case WM_KEYDOWN:

		l_Window->FireKeyDownEvent(in_wParam);
		break;

	case WM_MOUSELEAVE:

		l_Window->FireMouseLeaveEvent();
		break;

	case WM_KILLFOCUS:

		l_Window->FireFocusEvent(false);
		break;

	case WM_SETFOCUS:

		l_Window->FireFocusEvent(true);
		break;

	case WM_CLOSE:

		l_Window->FireCloseEvent();
		break;

	default:

		return DefWindowProc(in_Hwnd, in_Message, in_wParam, in_lParam);
	}

	return 0;
}

//-----------------------------------------------------------------------------------------------------------------------------

BOOL CALLBACK MSWindow::DialogProc(HWND in_Hwnd, UINT in_Message, WPARAM in_wParam, LPARAM in_lParam)
{
	switch(in_Message)
	{
		// Hide the window when the X button is pressed
		case WM_CLOSE:
		{
			ShowWindow(in_Hwnd, SW_HIDE);
			return true;
		}
		// Very special case for closing the preference dialog with F1
		case WM_HELP:
		{
			ShowWindow(in_Hwnd, SW_HIDE);
			return TRUE;
		}
		// Check if the FloatSlider changed
		case WM_HSCROLL:
		{
			HWND l_ControlHwnd = (HWND)in_lParam;

			// Find the associated preference data
			PreferenceData* l_PrefData = sControlMapping[l_ControlHwnd];
			if(!l_PrefData)
			{
				return FALSE;
			}

			// Update the trackbar value
			int l_TrackbackValue = SendMessage(l_ControlHwnd, TBM_GETPOS, 0, 0);
			l_PrefData->Set<FloatSlider>(MIN_IMAGE_SIZE + (MAX_IMAGE_SIZE - MIN_IMAGE_SIZE) * (float)l_TrackbackValue / MAX_SLIDER_VALUE);

			// Update the label value
			char l_Buff[16];
			sprintf_s(l_Buff, 16, "%.2f", l_PrefData->Get<float>());
			SetDlgItemText(in_Hwnd, GetDlgCtrlID(l_ControlHwnd) | 0x80000000, l_Buff);

			return TRUE;
		}
		// Check if this message is for a control
		case WM_COMMAND:
		{
			HWND l_ControlHwnd = (HWND)in_lParam;

			// Find the associated preference data
			PreferenceData* l_PrefData = sControlMapping[l_ControlHwnd];
			if(!l_PrefData)
			{
				return FALSE;
			}

			// Handle the control notification type
			switch(HIWORD(in_wParam))
			{
				// Check if one of the check boxes has been clicked
				case BN_CLICKED:
				{
					// Write the new checkbox state to the preference data
					bool l_Checked = SendMessage(l_ControlHwnd, BM_GETCHECK, 0, 0) ? true : false;
					l_PrefData->Set<bool>(l_Checked);

					return TRUE;
				}
				// Check if the combo box changed
				case CBN_SELENDOK:
				{
					// Update the layout type name
					int l_Selection = SendMessage(l_ControlHwnd, CB_GETCURSEL, 0, 0);
					l_PrefData->Set<LayoutIndex>(l_Selection);

					return TRUE;
				}
				// Check if one of the edit controls has been updated
				case EN_UPDATE:
				{
					// Get the current text
					char l_Buff[16];
					memset(l_Buff, 0, 16);
					GetWindowText(l_ControlHwnd, l_Buff, 16);
					if(l_Buff[0] == NULL)
					{
						return FALSE;
					}

					// Convert the text buffer into a numeric literal
					float l_Value = (float)atof(l_Buff);

					// Write the new value to the preference data appropriately
					switch(l_PrefData->GetType())
					{
					case PreferenceType_int:
						l_PrefData->Set<int>((int)l_Value);
						break;
					case PreferenceType_float:
						l_PrefData->Set<float>(l_Value);
						break;
					}

					return TRUE;
				}
			}
		}
	}

	return FALSE;
}
