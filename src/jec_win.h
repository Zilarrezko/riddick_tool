#ifndef jec_win_h

#if !defined(offsetof)
#define offsetof(s,memb) ((size_t)((char *)&((s *)0)->memb - (char *)0))
#endif

#if __cplusplus
extern "C"
{
#endif
	int _fltused;

#define WINAPI __stdcall
#define CALLBACK __stdcall
#define APIENTRY WINAPI
#define MAX_PATH 260

#define WINGDIAPI DECLSPEC_IMPORT

#define CONST const

#if defined(_WIN64)
	typedef __int64 LONG_PTR;
	typedef unsigned __int64 UINT_PTR;
	typedef unsigned __int64 ULONG_PTR;
#else
	typedef long LONG_PTR;
	typedef unsigned int UINT_PTR;
	typedef unsigned long ULONG_PTR;
#endif

	typedef unsigned char BYTE;
	typedef unsigned short WORD;
	typedef unsigned long DWORD;
   typedef unsigned long ULONG;
	typedef unsigned int UINT;
   typedef unsigned short USHORT;
	typedef int BOOL;
	typedef long LONG;
	typedef long long LONGLONG;
	typedef void *HANDLE;
	typedef HANDLE HWND;
	typedef HANDLE HMENU;
	typedef HANDLE HINSTANCE;
	typedef HANDLE HGLOBAL;
	typedef HANDLE HDC;
	typedef HANDLE HMONITOR;
	typedef HANDLE HDROP;
	typedef HANDLE HCURSOR;
	typedef HANDLE HICON;
	typedef HANDLE HBRUSH;
	typedef HANDLE HBITMAP;
	typedef HANDLE HGDIOBJ;
	typedef HANDLE HFONT;
   typedef HANDLE HRAWINPUT;
	typedef HINSTANCE HMODULE;
	typedef LONG_PTR LRESULT;
	typedef UINT_PTR WPARAM;
	typedef LONG_PTR LPARAM;
	typedef ULONG_PTR DWORD_PTR;
   typedef DWORD COLORREF;
   typedef DWORD * LPDWORD;
   typedef DWORD * PDWORD;
   typedef void * LPVOID;
   typedef void * PVOID;
   typedef size_t SIZE_T;
   typedef UINT MMRESULT;

#define MEM_COMMIT  0x00001000
#define MEM_RESERVE 0x00002000
#define PAGE_READWRITE         0x04
#define PAGE_EXECUTE_READWRITE 0x40
	void *WINAPI VirtualAlloc(void *Address, size_t Size, DWORD AllocationType, DWORD Protection);

#define MEM_DECOMMIT 0x4000
#define MEM_RELEASE  0x8000
	BOOL WINAPI VirtualFree(void *Address, size_t Size, DWORD FreeType);

	typedef union LARGE_INTEGER
	{
		struct
		{
			DWORD LowPart;
			LONG  HighPart;
		};
		struct
		{
			DWORD LowPart;
			LONG  HighPart;
		} u;
		LONGLONG QuadPart;
	} LARGE_INTEGER;

	BOOL WINAPI QueryPerformanceCounter(LARGE_INTEGER *PerformanceCount);
	BOOL WINAPI QueryPerformanceFrequency(LARGE_INTEGER *Frequency);


#define CONST const

#ifdef UNICODE
	typedef wchar_t TCHAR;
#else
	typedef char TCHAR;
#endif

	typedef TCHAR *LPTSTR, *LPTCH;

	typedef char CHAR;
	typedef CHAR *LPSTR, *LPCH;

	typedef wchar_t WCHAR;
	typedef WCHAR *LPWSTR, *LPWCH;

	typedef wchar_t WCHAR;
	typedef WCHAR *LPCWSTR;
	typedef CHAR *LPCSTR;

#ifdef UNICODE
	typedef LPCWSTR LPCTSTR;
#else
	typedef LPCSTR LPCTSTR;
#endif

	typedef struct SECURITY_ATTRIBUTES
	{
		DWORD  nLength;
		void *lpSecurityDescriptor;
		BOOL   bInheritHandle;
	} SECURITY_ATTRIBUTES;

#define GENERIC_READ  0x80000000ul
#define GENERIC_WRITE 0x40000000ul
#define FILE_SHARE_READ   0x00000001
#define FILE_SHARE_WRITE  0x00000002
#define FILE_SHARE_DELETE 0x00000004
#define CREATE_NEW    1
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define OPEN_ALWAYS   4

#ifdef UNICODE
#define CreateFile CreateFileW
#else
#define CreateFile CreateFileA
#endif
#define INVALID_HANDLE_VALUE (HANDLE)-1
	HANDLE WINAPI CreateFile(LPCTSTR FileName, DWORD DesiredAccess, DWORD ShareMode, SECURITY_ATTRIBUTES *SecurityAttributes, DWORD CreationDisposition, DWORD FlagsAndAttributes, HANDLE TemplateFile);


#if defined(_WIN64)
	typedef unsigned __int64 ULONG_PTR;
#else
	typedef unsigned long ULONG_PTR;
#endif
	typedef struct _OVERLAPPED
	{
		ULONG_PTR Internal;
		ULONG_PTR InternalHigh;
		union
		{
			struct
			{
				DWORD Offset;
				DWORD OffsetHigh;
			};
			void *Pointer;
		};
		HANDLE hEvent;
	} OVERLAPPED;
	BOOL WINAPI ReadFile(HANDLE File, void *Buffer, DWORD NumberOfBytesToRead, DWORD *NumberOfBytesRead, OVERLAPPED *Overlapped);


#define CW_USEDEFAULT ((int)0x80000000)

	HWND WINAPI CreateWindow(LPCTSTR ClassName, LPCTSTR WindowName, DWORD dwStyle, int X, int Y, int Width, int Height, HWND WindowParent, HMENU Menu, HINSTANCE Instance, void *Param);
#define CreateWindowA(ClassName, WindowName, Style, X, Y, Width, Height, WindowParent, Menu, Instance, Param) CreateWindowExA(0L, ClassName, WindowName, Style, X, Y, Width, Height, WindowParent, Menu, Instance, Param)
#define CreateWindowW(ClassName, WindowName, Style, X, Y, Width, Height, WindowParent, Menu, Instance, Param) CreateWindowExW(0L, ClassName, WindowName, Style, X, Y, Width, nHeight, WindowParent, Menu, Instance, Param)
#ifdef UNICODE
#define CreateWindow CreateWindowW
#else
#define CreateWindow CreateWindowA
#endif

#ifdef UNICODE
#define CreateWindowEx CreateWindowExW
#else
#define CreateWindowEx CreateWindowExA
#endif
	HWND WINAPI CreateWindowEx(DWORD ExtendedStyle, LPCTSTR ClassName, LPCTSTR WindowName, DWORD Style, int X, int Y, int Width, int Height, HWND WindowParent, HMENU Menu, HINSTANCE Instance, void *Param);


	DWORD WINAPI GetLastError(void);


#ifdef UNICODE
#define OutputDebugString OutputDebugStringW
#else
#define OutputDebugString OutputDebugStringA
#endif
	void WINAPI OutputDebugString(LPCTSTR OutputString);


	DWORD WINAPI GetFileSize(HANDLE File, DWORD *FileSizeHigh);
	BOOL WINAPI GetFileSizeEx(HANDLE File, LARGE_INTEGER *FileSize);


#define MB_OK 0x00000000L

#ifdef UNICODE
#define MessageBox MessageBoxW
#else
#define MessageBox MessageBoxA
#endif
	int WINAPI MessageBox(HWND WindowHandle, LPCTSTR Text, LPCTSTR Caption, unsigned int Type);

	BOOL WINAPI CloseHandle(HANDLE Handle);

#define FILE_ATTRIBUTE_NORMAL 0x80


	BOOL WINAPI OpenClipboard(HWND WindowHandle);
	BOOL WINAPI EmptyClipboard(void);
	BOOL WINAPI CloseClipboard(void);


#define GMEM_MOVEABLE 0x0002
	HGLOBAL WINAPI GlobalAlloc(unsigned int Flags, size_t Bytes);
	void *WINAPI GlobalLock(HGLOBAL Memory);
	BOOL WINAPI GlobalUnlock(HGLOBAL Memory);


#define CF_TEXT 1
	HANDLE WINAPI SetClipboardData(unsigned int Format, HANDLE Memory);
	HANDLE WINAPI GetClipboardData(unsigned int Format);

#ifdef UNICODE
#define GetCurrentDirectory GetCurrentDirectoryW
#else
#define GetCurrentDirectory GetCurrentDirectoryA
#endif
	DWORD WINAPI GetCurrentDirectory(DWORD BufferLength, LPTSTR Buffer);

	typedef struct FILETIME
	{
		DWORD LowDateTime;
		DWORD HighDateTime;
	} FILETIME;
	typedef struct _WIN32_FIND_DATA
	{
		DWORD    dwFileAttributes;
		FILETIME ftCreationTime;
		FILETIME ftLastAccessTime;
		FILETIME ftLastWriteTime;
		DWORD    nFileSizeHigh;
		DWORD    nFileSizeLow;
		DWORD    dwReserved0;
		DWORD    dwReserved1;
		TCHAR    cFileName[MAX_PATH];
		TCHAR    cAlternateFileName[14];
	} WIN32_FIND_DATA;

#define FILE_ATTRIBUTE_DIRECTORY 0x10
	BOOL WINAPI FindClose(HANDLE FindFile);


	typedef struct tagBITMAPINFOHEADER
	{
		DWORD biSize;
		LONG  biWidth;
		LONG  biHeight;
		WORD  biPlanes;
		WORD  biBitCount;
		DWORD biCompression;
		DWORD biSizeImage;
		LONG  biXPelsPerMeter;
		LONG  biYPelsPerMeter;
		DWORD biClrUsed;
		DWORD biClrImportant;
	} BITMAPINFOHEADER;
	typedef struct tagRGBQUAD
	{
		BYTE rgbBlue;
		BYTE rgbGreen;
		BYTE rgbRed;
		BYTE rgbReserved;
	} RGBQUAD;
	typedef struct tagBITMAPINFO
	{
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD          bmiColors[1];
	} BITMAPINFO;

	typedef struct tagPOINT
	{
		LONG x;
		LONG y;
	} POINT;
	typedef struct _RECT
	{
		LONG left;
		LONG top;
		LONG right;
		LONG bottom;
	} RECT;
	typedef struct tagWINDOWPLACEMENT
	{
		UINT  length;
		UINT  flags;
		UINT  showCmd;
		POINT ptMinPosition;
		POINT ptMaxPosition;
		RECT  rcNormalPosition;
	} WINDOWPLACEMENT;
	typedef struct tagMONITORINFO
	{
		DWORD cbSize;
		RECT  rcMonitor;
		RECT  rcWork;
		DWORD dwFlags;
	} MONITORINFO;
#define MONITOR_DEFAULTTOPRIMARY 0x00000001
#define MONITOR_DEFAULTTONEAREST 0x00000002
	BOOL WINAPI GetWindowPlacement(HWND Window, WINDOWPLACEMENT *WindowPlacement);
	HMONITOR MonitorFromWindow(HWND Window, DWORD Flags);

#define GWL_STYLE -16
#define GWL_USERDATA -21

#define WS_OVERLAPPED 0x00000000L
#define WS_CAPTION 0x00C00000L
#define WS_SYSMENU 0x00080000L
#define WS_THICKFRAME 0x00040000L
#define WS_MINIMIZEBOX 0x00020000L
#define WS_MAXIMIZEBOX 0x00010000L
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)


#define HWND_TOP (HWND)0
#define HWND_TOPMOST (HWND)-1

	typedef struct tagWINDOWPOS
	{
		HWND hwnd;
		HWND hwndInsertAfter;
		int  x;
		int  y;
		int  cx;
		int  cy;
		UINT flags;
	} WINDOWPOS;
#define SWP_NOSIZE 0x0001
#define SWP_NOMOVE 0x0002
#define SWP_NOZORDER 0x0002
#define SWP_FRAMECHANGED 0x0020
#define SWP_NOOWNERZORDER 0x0200
	BOOL WINAPI SetWindowPos(HWND Window, HWND WindowInsertAfter,
							 int X, int Y, int cx, int cy, UINT Flags);
	BOOL WINAPI SetWindowPlacement(HWND Window, const WINDOWPLACEMENT *WindowPlacement);


	BOOL WINAPI GetFileTime(HANDLE File, FILETIME *CreationTime, FILETIME *LastAccessTime, FILETIME *LastWriteTime);
	LONG WINAPI CompareFileTime(const FILETIME *FileTime1, const FILETIME *FileTime2);


#define BI_RGB 0L

#define PROC void *
	PROC WINAPI wglGetProcAddress(LPCSTR Procedure);


	typedef struct tagPIXELFORMATDESCRIPTOR
	{
		WORD  nSize;
		WORD  nVersion;
		DWORD dwFlags;
		BYTE  iPixelType;
		BYTE  cColorBits;
		BYTE  cRedBits;
		BYTE  cRedShift;
		BYTE  cGreenBits;
		BYTE  cGreenShift;
		BYTE  cBlueBits;
		BYTE  cBlueShift;
		BYTE  cAlphaBits;
		BYTE  cAlphaShift;
		BYTE  cAccumBits;
		BYTE  cAccumRedBits;
		BYTE  cAccumGreenBits;
		BYTE  cAccumBlueBits;
		BYTE  cAccumAlphaBits;
		BYTE  cDepthBits;
		BYTE  cStencilBits;
		BYTE  cAuxBuffers;
		BYTE  iLayerType;
		BYTE  bReserved;
		DWORD dwLayerMask;
		DWORD dwVisibleMask;
		DWORD dwDamageMask;
	} PIXELFORMATDESCRIPTOR;


#define PFD_TYPE_RGBA      0x00000000
#define PFD_MAIN_PLANE     0x00000000
#define PFD_DOUBLEBUFFER   0x00000001
#define PFD_DRAW_TO_WINDOW 0x00000004
#define PFD_SUPPORT_OPENGL 0x00000020
	int WINAPI ChoosePixelFormat(HDC DeviceContext, const PIXELFORMATDESCRIPTOR *PixelFormat);
	int WINAPI DescribePixelFormat(HDC DeviceContext, int Index, UINT Bytes, PIXELFORMATDESCRIPTOR *PixelFormat);
	BOOL WINAPI SetPixelFormat(HDC DeviceContext, int Index, const PIXELFORMATDESCRIPTOR *PixelFormat);


	struct HGLRC__
	{
		int unsued;
	};
	typedef struct HGLRC__ *HGLRC;
	HGLRC WINAPI wglCreateContext(HDC DeviceContext);
	BOOL WINAPI wglMakeCurrent(HDC DeviceContext, HGLRC GLDeviceContext);


	BOOL WINAPI SwapBuffers(HDC DeviceContext);

	BOOL WINAPI WriteFile(HANDLE File, void *Buffer, DWORD NumberOfBytesToWrite, DWORD *NumberOfBytesWritten, OVERLAPPED *Overlapped);

#ifdef UNICODE
#define PostMessage PostMessageW
#else
#define PostMessage PostMessageA
#endif
#define SIZE_MAXIMIZED 2
	BOOL PostMessage(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);

	int ReleaseDC(HWND Window, HDC DeviceContext);

#define WM_CREATE                       0x0001
#define WM_DESTROY                      0x0002
#define WM_SIZE                         0x0005
#define WM_SETFOCUS                     0x0007
#define WM_KILLFOCUS                    0x0008
#define WM_PAINT                        0x000F
#define WM_CLOSE                        0x0010
#define WM_QUIT                         0x0012
#define WM_SHOWWINDOW                   0x0018
#define WM_ACTIVATEAPP                  0x001C
#define WM_SETCURSOR                    0x0020
#define WM_INPUT                        0x00FF
#define WM_KEYDOWN                      0x0100
#define WM_KEYUP                        0x0101
#define WM_CHAR                         0x0102
#define WM_SYSKEYDOWN                   0x0104
#define WM_SYSKEYUP                     0x0105
#define WM_SYSCHAR                      0x0106
#define WM_UNICHAR                      0x0109
#define WM_MOUSEMOVE                    0x0200
#define WM_LBUTTONDOWN                  0x0201
#define WM_LBUTTONUP                    0x0202
#define WM_LBUTTONDBLCLK                0x0203
#define WM_RBUTTONDOWN                  0x0204
#define WM_RBUTTONUP                    0x0205
#define WM_RBUTTONDBLCLK                0x0206
#define WM_DROPFILES                    0x0233

	typedef struct tagCREATESTRUCT
	{
		void *    lpCreateParams;
		HINSTANCE hInstance;
		HMENU     hMenu;
		HWND      hwndParent;
		int       cy;
		int       cx;
		int       y;
		int       x;
		LONG      style;
		LPCTSTR   lpszName;
		LPCTSTR   lpszClass;
		DWORD     dwExStyle;
	} CREATESTRUCT;

#define GWLP_USERDATA -21

	void WINAPI PostQuitMessage(int ExitCode);

	BOOL WINAPI GetClientRect(HWND Window, RECT *Rectangle);
	BOOL GetWindowRect(HWND WindowHandle, RECT *Rectangle);
	BOOL ValidateRect(HWND Window, RECT *Rectangle);

	UINT DragQueryFile(HDROP DropHandle, UINT FileIndex, LPTSTR FileName, UINT BufferSize);
	BOOL DragQueryPoint(HDROP DropHandle, POINT *Point);
	void DragFinish(HDROP DropHandle);

	BOOL WINAPI GetKeyboardState(BYTE *KeyState);



#ifdef UNICODE
#define DefWindowProc DefWindowProcW
#else
#define DefWindowProc DefWindowProcA
#endif
	LRESULT WINAPI DefWindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);


	BOOL WINAPI ReleaseSemaphore(HANDLE Semaphore, LONG ReleaseCount, LONG *PreviousCount);

#define INFINITE 0xFFFFFFFF
	DWORD WINAPI WaitForSingleObject(HANDLE Handle, DWORD Milliseconds);
	DWORD WINAPI WaitForSingleObjectEx(HANDLE Handle, DWORD Milliseconds, BOOL Alertable);


#ifdef UNICODE
#define GetWindowsDirectory GetWindowsDirectoryW
#else
#define GetWindowsDirectory GetWindowsDirectoryA
#endif
	UINT WINAPI GetWindowsDirectory(LPTSTR Buffer, UINT Size);

	typedef LRESULT (CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

	typedef struct tagWNDCLASSEXA
	{
		UINT        cbSize;
		/* Win 3.x */
		UINT        style;
		WNDPROC     lpfnWndProc;
		int         cbClsExtra;
		int         cbWndExtra;
		HINSTANCE   hInstance;
		HICON       hIcon;
		HCURSOR     hCursor;
		HBRUSH      hbrBackground;
		LPCSTR      lpszMenuName;
		LPCSTR      lpszClassName;
		/* Win 4.0 */
		HICON       hIconSm;
	} WNDCLASSEXA;
	typedef struct tagWNDCLASSEXW
	{
		UINT        cbSize;
		/* Win 3.x */
		UINT        style;
		WNDPROC     lpfnWndProc;
		int         cbClsExtra;
		int         cbWndExtra;
		HINSTANCE   hInstance;
		HICON       hIcon;
		HCURSOR     hCursor;
		HBRUSH      hbrBackground;
		LPCWSTR     lpszMenuName;
		LPCWSTR     lpszClassName;
		/* Win 4.0 */
		HICON       hIconSm;
	} WNDCLASSEXW;
#ifdef UNICODE
#define WNDCLASSEX WNDCLASSEXW 
#else
#define WNDCLASSEX WNDCLASSEXA
#endif // UNICODE

	typedef struct tagWNDCLASSA
	{
		UINT        style;
		WNDPROC     lpfnWndProc;
		int         cbClsExtra;
		int         cbWndExtra;
		HINSTANCE   hInstance;
		HICON       hIcon;
		HCURSOR     hCursor;
		HBRUSH      hbrBackground;
		LPCSTR      lpszMenuName;
		LPCSTR      lpszClassName;
	} WNDCLASSA;
	typedef struct tagWNDCLASSW
	{
		UINT        style;
		WNDPROC     lpfnWndProc;
		int         cbClsExtra;
		int         cbWndExtra;
		HINSTANCE   hInstance;
		HICON       hIcon;
		HCURSOR     hCursor;
		HBRUSH      hbrBackground;
		LPCWSTR     lpszMenuName;
		LPCWSTR     lpszClassName;
	} WNDCLASSW;
#ifdef UNICODE
#define WNDCLASS WNDCLASSW
#else
#define WNDCLASS WNDCLASSA
#endif // UNICODE


#define CS_VREDRAW         0x0001
#define CS_HREDRAW         0x0002
#define CS_DBLCLKS         0x0008
#define CS_OWNDC           0x0020
#define CS_CLASSDC         0x0040
#define CS_PARENTDC        0x0080
#define CS_NOCLOSE         0x0200
#define CS_SAVEBITS        0x0800
#define CS_BYTEALIGNCLIENT 0x1000
#define CS_BYTEALIGNWINDOW 0x2000
#define CS_GLOBALCLASS     0x4000
#define CS_DROPSHADOW      0x00020000


	typedef WORD ATOM;
#ifdef UNICODE
#define RegisterClass RegisterClassW
#else
#define RegisterClass RegisterClassA
#endif
	ATOM WINAPI RegisterClass(WNDCLASS *WindowClass);

   ATOM RegisterClassExA(const WNDCLASSEXA *window_class);

#define SM_CXSCREEN 0
#define SM_CYSCREEN 1
	int WINAPI GetSystemMetrics(int Index);


#define WS_VISIBLE 0x10000000L


	HDC GetDC(HWND Window);


	void DragAcceptFiles(HWND Window, BOOL Accept);


	typedef struct tagMSG
	{
		HWND   hwnd;
		UINT   message;
		WPARAM wParam;
		LPARAM lParam;
		DWORD  time;
		POINT  pt;
	} MSG;

#define PM_REMOVE 0x0001


	BOOL WINAPI TranslateMessage(MSG *Message);
#ifdef UNICODE
#define DispatchMessage DispatchMessageW
#else
#define DispatchMessage DispatchMessageA
#endif
	LRESULT WINAPI DispatchMessage(MSG *Message);


	void WINAPI Sleep(DWORD Milliseconds);


#ifdef UNICODE
#define FindFirstFile FindFirstFileW
#else
#define FindFirstFile FindFirstFileA
#endif
	HANDLE WINAPI FindFirstFile(LPCTSTR FileName, WIN32_FIND_DATA *FindFileData);
#ifdef UNICODE
#define FindNextFile FindNextFileW
#else
#define FindNextFile FindNextFileA
#endif
	BOOL WINAPI FindNextFile(HANDLE FindFile, WIN32_FIND_DATA *FindFileData);
#ifdef UNICODE
#define GetMonitorInfo GetMonitorInfoW
#else
#define GetMonitorInfo GetMonitorInfoA
#endif
	BOOL GetMonitorInfo(HMONITOR Monitor, MONITORINFO *MonitorInfo);
#ifdef UNICODE
#define SetWindowLong SetWindowLongW
#else
#define SetWindowLong SetWindowLongA
#endif
	LONG WINAPI SetWindowLong(HWND Window, int Index, LONG NewLong);
#ifdef UNICODE
#define GetWindowLong GetWindowLongW
#else
#define GetWindowLong GetWindowLongA
#endif
	LONG WINAPI GetWindowLong(HWND Window, int Index);
#ifdef UNICODE
#define SetWindowLongPtr SetWindowLongPtrW
#else
#define SetWindowLongPtr SetWindowLongPtrA
#endif
	LONG_PTR WINAPI SetWindowLongPtr(HWND Window, int Index, LONG_PTR NewLong);
#ifdef UNICODE
#define GetWindowLongPtr GetWindowLongPtrW
#else
#define GetWindowLongPtr GetWindowLongPtrA
#endif
	LONG_PTR WINAPI GetWindowLongPtr(HWND Window, int Index);

#define LOWORD(l) ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l) ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define LOBYTE(w) ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w) ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))

#define InterlockedIncrement _InterlockedIncrement
	LONG __cdecl InterlockedIncrement(LONG volatile *Addend);

#ifdef UNICODE
#define PeekMessage PeekMessageW
#else
#define PeekMessage PeekMessageA
#endif
	BOOL WINAPI PeekMessage(MSG *Message, HWND Window, UINT MessageFilterMin, UINT MessageFilterMax, UINT RemoveMsg);

#define MAKEINTRESOURCEA(i) ((LPSTR)((ULONG_PTR)((WORD)(i))))
#define MAKEINTRESOURCEW(i) ((LPWSTR)((ULONG_PTR)((WORD)(i))))
#ifdef UNICODE
#define MAKEINTRESOURCE  MAKEINTRESOURCEW
#else
#define MAKEINTRESOURCE  MAKEINTRESOURCEA
#endif

#define IDC_APPSTARTING MAKEINTRESOURCE(32650) // Standard arrow and small hourglass
#define IDC_ARROW MAKEINTRESOURCE(32512)       // Standard arrow
#define IDC_CROSS MAKEINTRESOURCE(32515)       // Crosshair
#define IDC_HAND MAKEINTRESOURCE(32649)        // Hand
#define IDC_HELP MAKEINTRESOURCE(32651)        // Arrow and question mark
#define IDC_IBEAM MAKEINTRESOURCE(32513)       // I-beam
#define IDC_ICON MAKEINTRESOURCE(32641)        // Obsolete for applications marked version 4.0 or later.
#define IDC_NO MAKEINTRESOURCE(32648)          // Slashed circle
#define IDC_SIZE MAKEINTRESOURCE(32640)        // Obsolete for applications marked version 4.0 or later.Use IDC_SIZEALL.
#define IDC_SIZEALL MAKEINTRESOURCE(32646)     // Four-pointed arrow pointing north, south, east, and west
#define IDC_SIZENESW MAKEINTRESOURCE(32643)    // Double-pointed arrow pointing northeast and southwest
#define IDC_SIZENS MAKEINTRESOURCE(32645)      // Double-pointed arrow pointing north and south
#define IDC_SIZENWSE MAKEINTRESOURCE(32642)    // Double-pointed arrow pointing northwest and southeast
#define IDC_SIZEWE MAKEINTRESOURCE(32644)      // Double-pointed arrow pointing west and east
#define IDC_UPARROW MAKEINTRESOURCE(32516)     // Vertical arrow
#define IDC_WAIT MAKEINTRESOURCE(32514)        // Hourglass
#ifdef UNICODE
#define LoadCursor LoadCursorW
#else
#define LoadCursor LoadCursorA
#endif
	HCURSOR WINAPI LoadCursor(HINSTANCE Instance, LPCTSTR CursorName);
	HCURSOR WINAPI SetCursor(HCURSOR Cursor);

#define SW_HIDE          0
#define SW_MAXIMIZE      3
#define SW_SHOWMAXIMIZED 3
#define SW_SHOW          5
#define SW_MINIMIZE      6
	BOOL WINAPI ShowWindow(HWND Window, int ShowCommand);

#ifdef UNICODE
#define GetModuleHandle GetModuleHandleW
#else
#define GetModuleHandle GetModuleHandleA
#endif
	HMODULE WINAPI GetModuleHandle(LPCTSTR ModuleName);

	void WINAPI ExitProcess(UINT ExitCode);

#define MAPVK_VSC_TO_VK 1
#ifdef UNICODE
#define MapVirtualKey MapVirtualKeyW
#else
#define MapVirtualKey MapVirtualKeyA
#endif
	UINT WINAPI MapVirtualKey(UINT Code, UINT MapType);

	int StretchDIBits(HDC DeviceContext, int XDest, int YDest, int DestWidth, int DestHeight, int XSrc, int YSrc, int SrcWidth, int SrcHeight, 
					      const void *Bits, const BITMAPINFO *BitmapInfo, UINT UsageFlags, DWORD RasterOpCode);
#define DIB_RGB_COLORS 0x0
#define SRCCOPY 0x00CC0020

	BOOL MoveWindow(HWND Window, int X, int Y, int Width, int Height, BOOL Repaint);

	HMODULE LoadLibraryA(LPCSTR LibraryFileName);

#if _WIN64
	typedef __int64 INT_PTR;
#else
	typedef int INT_PTR;
#endif
#define FAR
	typedef INT_PTR(FAR WINAPI *FARPROC)();
	FARPROC GetProcAddress(HMODULE Module, LPCSTR  ProcedureName);

	HDC CreateCompatibleDC(HDC DeviceContext);
	HBITMAP CreateCompatibleBitmap(HDC DeviceContext, int Width, int Height);
	HGDIOBJ SelectObject(HDC DeviceContext, HGDIOBJ Object);

#ifdef UNICODE
#define CreateFont CreateFontW
#else
#define CreateFont CreateFontA
#endif
	HFONT CreateFont(int Height, int Width,
					     int Escapement, int Oreintation,
					     int Weight,
					     DWORD Italic, DWORD Underline, DWORD StrikeOut,
					     DWORD Charset,
					     DWORD OutPrecision, DWORD ClipPrecision,
					     DWORD Quality, DWORD PitchFamily, 
					     LPCTSTR Facename);
#define FW_NORMAL 400
#define TRUE  1
#define FALSE 0
#define ANSI_CHARSET 0
#define OUT_DEFAULT_PRECIS 0 
#define CLIP_DEFAULT_PRECIS 0
#define DEFAULT_QUALITY           0
#define DRAFT_QUALITY             1
#define PROOF_QUALITY             2
#define NONANTIALIASED_QUALITY    3
#define ANTIALIASED_QUALITY       4
#define CLEARTYPE_QUALITY         5
#define CLEARTYPE_NATURAL_QUALITY 6
#define DEFAULT_PITCH 0
#define FF_DONTCARE (0<<4)

	typedef struct tagTEXTMETRICW {
		LONG  tmHeight;
	    LONG  tmAscent;
		LONG  tmDescent;
		LONG  tmInternalLeading;
		LONG  tmExternalLeading;
		LONG  tmAveCharWidth;
		LONG  tmMaxCharWidth;
		LONG  tmWeight;
		LONG  tmOverhang;
		LONG  tmDigitizedAspectX;
		LONG  tmDigitizedAspectY;
		WCHAR tmFirstChar;
		WCHAR tmLastChar;
		WCHAR tmDefaultChar;
		WCHAR tmBreakChar;
		BYTE  tmItalic;
		BYTE  tmUnderlined;
		BYTE  tmStruckOut;
		BYTE  tmPitchAndFamily;
		BYTE  tmCharSet;
	} TEXTMETRICW, *PTEXTMETRICW, *NPTEXTMETRICW, *LPTEXTMETRICW;
	typedef struct tagTEXTMETRICA {
		LONG tmHeight;
		LONG tmAscent;
		LONG tmDescent;
		LONG tmInternalLeading;
		LONG tmExternalLeading;
		LONG tmAveCharWidth;
		LONG tmMaxCharWidth;
		LONG tmWeight;
		LONG tmOverhang;
		LONG tmDigitizedAspectX;
		LONG tmDigitizedAspectY;
		BYTE tmFirstChar;
		BYTE tmLastChar;
		BYTE tmDefaultChar;
		BYTE tmBreakChar;
		BYTE tmItalic;
		BYTE tmUnderlined;
		BYTE tmStruckOut;
		BYTE tmPitchAndFamily;
		BYTE tmCharSet;
	} TEXTMETRICA, *PTEXTMETRICA, *NPTEXTMETRICA, *LPTEXTMETRICA;

#if UNICODE
#define TEXTMETRIC TEXTMETRICW
#else
#define TEXTMETRIC TEXTMETRICA
#endif

BOOL WINAPI GetTextMetricsA(HDC DeviceContext, TEXTMETRICA *TextMetric);
BOOL WINAPI GetTextMetricsW(HDC DeviceContext, TEXTMETRICW *TextMetric);
#ifdef UNICODE
#define GetTextMetrics  GetTextMetricsW
#else
#define GetTextMetrics  GetTextMetricsA
#endif
	
	typedef struct tagSIZE {
	   LONG cx;
		LONG cy;
	} SIZE, *PSIZE, *LPSIZE;
   
   BOOL GetTextExtentPoint32W(HDC DeviceContext, LPCWSTR String, int Length, SIZE *Size);
   BOOL GetTextExtentPoint32A(HDC DeviceContext, LPCSTR String, int Length, SIZE *Size);
#if UNICODE
#define GetTextExtentPoint32 GetTextExtentPoint32W
#else
#define GetTextExtentPoint32 GetTextExtentPoint32A
#endif
   
   BOOL PatBlt(HDC DeviceContext, 
               int X, int Y,
               int Width, int Height,
               DWORD RasterOpcode);
#define BLACKNESS (DWORD)0x00000042 /* dest = BLACK */
#define WHITENESS (DWORD)0x00FF0062 /* dest = WHITE */
#define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

   COLORREF SetBkColor(HDC DeviceContext, COLORREF Color);

   int AddFontResourceExA(LPCSTR Filename, DWORD Security, void *Reserved);
#define FR_PRIVATE  0x10
#define FR_NOT_ENUM 0x20

   COLORREF SetTextColor(HDC DeviceContext, COLORREF Color);

   BOOL TextOutW(HDC DeviceContext, 
                 int X, int Y, 
                 LPCWSTR String, int Length);
   BOOL TextOutA(HDC DeviceContext, 
                 int X, int Y, 
                 LPCSTR String, int Length);

   COLORREF GetPixel(HDC DeviceContext,
                     int X, int Y);

   HBITMAP CreateDIBSection(HDC DeviceContext, const BITMAPINFO *BitmapInfo, UINT Usage, void **Bits, HANDLE Section, DWORD Offset);

   BOOL DeleteObject(HGDIOBJ ObjectHandle);

   typedef struct tagKERNINGPAIR 
   {
     WORD wFirst;
     WORD wSecond;
     int  iKernAmount;
   } KERNINGPAIR, *LPKERNINGPAIR;

   DWORD GetKerningPairsA(HDC DeviceContext, DWORD Pairs, KERNINGPAIR *KerningPair);
   DWORD GetKerningPairsW(HDC DeviceContext, DWORD Pairs, KERNINGPAIR *KerningPair);

   typedef struct _ABC {
     int  abcA;
     UINT abcB;
     int  abcC;
   } ABC, *PABC, *NPABC, *LPABC;
   BOOL GetCharABCWidthsA(HDC DeviceContext, UINT First, UINT Last, ABC *ABC);
   BOOL GetCharABCWidthsW(HDC DeviceContext, UINT First, UINT Last, ABC *ABC);

   typedef struct _ABCFLOAT {
     float abcfA;
     float abcfB;
     float abcfC;
   } ABCFLOAT, *PABCFLOAT, *NPABCFLOAT, *LPABCFLOAT;
   BOOL GetCharABCWidthsFloatA(HDC DeviceContext, UINT First, UINT Last, ABCFLOAT *ABC);
   BOOL GetCharABCWidthsFloatW(HDC DeviceContext, UINT First, UINT Last, ABCFLOAT *ABC);

   BOOL GetCharWidth32A(HDC DeviceContext, UINT First, UINT Last, int *Out);
   BOOL GetCharWidth32W(HDC DeviceContext, UINT First, UINT Last, int *Out);
   BOOL GetCharWidthFloatA(HDC DeviceContext, UINT First, UINT Last, float *Out);
   BOOL GetCharWidthFloatW(HDC DeviceContext, UINT First, UINT Last, float *Out);

   BOOL GetCursorPos(POINT *Point);

   HWND SetCapture(HWND Window);
   BOOL ReleaseCapture();

#ifdef UNICODE
#define GetFullPathName  GetFullPathNameW
#else
#define GetFullPathName GetFullPathNameA
#endif

   DWORD WINAPI GetFullPathNameW(wchar_t *FileName, DWORD BufferLength, char *Buffer, char **FilePart);
   DWORD WINAPI GetFullPathNameA(char *lpFileName, DWORD BufferLength, char *Buffer, char **FilePart);

   HANDLE GetCurrentProcess();

   void *VirtualAllocEx(HANDLE process, void *address, size_t size, DWORD alloc_type, DWORD protect);
   BOOL VirtualFreeEx(HANDLE process, void *address, size_t size, DWORD free_type);

   typedef DWORD (__stdcall *LPTHREAD_START_ROUTINE)(void *thread_param);

   HANDLE CreateRemoteThread(HANDLE process, SECURITY_ATTRIBUTES *thread_attributes, size_t stack_size, LPTHREAD_START_ROUTINE start_address, void *param, DWORD creation_flag, DWORD *thread_id);

   typedef struct tagRAWINPUTDEVICE {
      USHORT usUsagePage;
      USHORT usUsage;
      DWORD  dwFlags;
      HWND   hwndTarget;
   } RAWINPUTDEVICE;

#define RIDEV_NOLEGACY    0x00000030
#define RIDEV_INPUTSINK   0x00000100
#define RIDEV_EXINPUTSINK 0x00001000
   BOOL RegisterRawInputDevices(RAWINPUTDEVICE *input_devices, UINT device_count, UINT size);

   typedef struct tagRAWINPUTHEADER {
      DWORD  dwType;
      DWORD  dwSize;
      HANDLE hDevice;
      WPARAM wParam;
   } RAWINPUTHEADER, *PRAWINPUTHEADER, *LPRAWINPUTHEADER;

#define RIM_TYPEMOUSE    0
#define RIM_TYPEKEYBOARD 1
#define RIM_TYPEHID      2

#define RI_KEY_MAKE  0
#define RI_KEY_BREAK 1
#define RI_KEY_E0    2
#define RI_KEY_E1    4

   typedef struct tagRAWMOUSE {
      USHORT usFlags;
      union {
         ULONG ulButtons;
         struct {
            USHORT usButtonFlags;
            USHORT usButtonData;
         } DUMMYSTRUCTNAME;
      } DUMMYUNIONNAME;
      ULONG  ulRawButtons;
      LONG   lLastX;
      LONG   lLastY;
      ULONG  ulExtraInformation;
   } RAWMOUSE, *PRAWMOUSE, *LPRAWMOUSE;
   typedef struct tagRAWKEYBOARD {
      USHORT MakeCode;
      USHORT Flags;
      USHORT Reserved;
      USHORT VKey;
      UINT   Message;
      ULONG  ExtraInformation;
   } RAWKEYBOARD, *PRAWKEYBOARD, *LPRAWKEYBOARD;
   typedef struct tagRAWHID {
      DWORD dwSizeHid;
      DWORD dwCount;
      BYTE  bRawData[1];
   } RAWHID, *PRAWHID, *LPRAWHID;

   typedef struct tagRAWINPUT {
      RAWINPUTHEADER header;
      union {
         RAWMOUSE    mouse;
         RAWKEYBOARD keyboard;
         RAWHID      hid;
      } data;
   } RAWINPUT, *PRAWINPUT, *LPRAWINPUT;

#define RID_INPUT  0x10000003
#define RID_HEADER 0x10000005

   UINT GetRawInputData(HRAWINPUT raw_input, UINT command, void *data, UINT *size, UINT size_header);

   BOOL GetExitCodeProcess(HANDLE  hProcess, LPDWORD lpExitCode);

#define PROCESS_CREATE_PROCESS 0x0080
#define PROCESS_CREATE_THREAD 0x0002
#define PROCESS_DUP_HANDLE 0x0040
#define PROCESS_QUERY_INFORMATION 0x0400
#define PROCESS_QUERY_LIMITED_INFORMATION 0x1000
#define PROCESS_SET_INFORMATION 0x0200
#define PROCESS_SET_QUOTA 0x0100
#define PROCESS_SUSPEND_RESUME 0x0800
#define PROCESS_TERMINATE 0x0001
#define PROCESS_VM_OPERATION 0x0008
#define PROCESS_VM_READ 0x0010
#define PROCESS_VM_WRITE 0x0020
#define SYNCHRONIZE 0x00100000L
#define PROCESS_ALL_ACCESS PROCESS_CREATE_PROCESS|PROCESS_CREATE_THREAD|PROCESS_DUP_HANDLE|PROCESS_QUERY_INFORMATION|PROCESS_QUERY_LIMITED_INFORMATION|PROCESS_SET_INFORMATION| \
                           PROCESS_SET_QUOTA|PROCESS_SUSPEND_RESUME|PROCESS_TERMINATE|PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE|SYNCHRONIZE
   HANDLE OpenProcess(DWORD desired_access, BOOL inherit_handle, DWORD process_id);

   BOOL ReadProcessMemory(HANDLE process, void *base_address, void *buffer, size_t size, size_t *bytes_read);
   BOOL WriteProcessMemory(HANDLE process, void *base_address, void *buffer, size_t size, size_t *bytes_written);

   MMRESULT timeBeginPeriod(UINT period);


#define VK_LBUTTON 0x01
#define VK_RBUTTON 0x02
#define VK_CANCEL 0x03
#define VK_MBUTTON 0x04
#define VK_XBUTTON1 0x05
#define VK_XBUTTON2 0x06
#define VK_BACK 0x08 // BACKSPACE key
#define VK_TAB 0x09
#define VK_CLEAR 0x0C
#define VK_RETURN 0x0D 
#define VK_SHIFT 0x10
#define VK_CONTROL 0x11
#define VK_MENU 0x12
#define VK_PAUSE 0x13
#define VK_CAPITAL 0x14 // CAPS LOCK key
#define VK_KANA 0x15 // IME Kana mode
#define VK_HANGUEL 0x15 // IME Hanguel mode (maintained for compatibility; use VK_HANGUL)
#define VK_HANGUL 0x15 // IME Hangul mode
#define VK_JUNJA 0x17 // IME Junja mode
#define VK_FINAL 0x18 // IME final mode ??
#define VK_HANJA 0x19 // IME Hanja mode
#define VK_KANJI 0x19 // IME Kanji mode
#define VK_ESCAPE 0x1B // ESC key
#define VK_CONVERT 0x1C // IME convert
#define VK_NONCONVERT 0x1D // IME nonconvert
#define VK_ACCEPT 0x1E // IME accept
#define VK_MODECHANGE 0x1F // IME mode change request
#define VK_SPACE 0x20 // SPACEBAR
#define VK_PRIOR 0x21 // PAGE UP key
#define VK_NEXT 0x22 // PAGE DOWN key
#define VK_END 0x23 // END key
#define VK_HOME 0x24 // HOME key
#define VK_LEFT 0x25 // LEFT ARROW key
#define VK_UP 0x26 // UP ARROW key
#define VK_RIGHT 0x27 // RIGHT ARROW key
#define VK_DOWN 0x28 // DOWN ARROW key
#define VK_SELECT 0x29 // SELECT key
#define VK_PRINT 0x2A // PRINT key
#define VK_EXECUTE 0x2B // EXECUTE key
#define VK_SNAPSHOT 0x2C // PRINT SCREEN key
#define VK_INSERT 0x2D // INS key
#define VK_DELETE 0x2E // DEL key
#define VK_HELP 0x2F // HELP key

#define VK_A 0x41 // A key
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_M 0x4D
#define VK_N 0x4E
#define VK_O 0x4F
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x55
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5A

#define VK_LWIN 0x5B
#define VK_RWIN 0x5C
#define VK_APPS 0x5D // Applications key (Natural keyboard)
#define VK_SLEEP 0x5F

#define VK_NUMPAD0 0x60
#define VK_NUMPAD1 0x61
#define VK_NUMPAD2 0x62
#define VK_NUMPAD3 0x63
#define VK_NUMPAD4 0x64
#define VK_NUMPAD5 0x65
#define VK_NUMPAD6 0x66
#define VK_NUMPAD7 0x67
#define VK_NUMPAD8 0x68
#define VK_NUMPAD9 0x69
#define VK_MULTIPLY 0x6A
#define VK_ADD 0x6B
#define VK_SEPARATOR 0x6C // Note: Division on numpad
#define VK_SUBTRACT 0x6D
#define VK_DECIMAL 0x6E
#define VK_DIVIDE 0x6F

#define VK_F1 0x70
#define VK_F2 0x71
#define VK_F3 0x72
#define VK_F4 0x73
#define VK_F5 0x74
#define VK_F6 0x75
#define VK_F7 0x76
#define VK_F8 0x77
#define VK_F9 0x78
#define VK_F10 0x79
#define VK_F11 0x7A
#define VK_F12 0x7B
#define VK_F13 0x7C
#define VK_F14 0x7D
#define VK_F15 0x7E
#define VK_F16 0x7F
#define VK_F17 0x80
#define VK_F18 0x81
#define VK_F19 0x82
#define VK_F20 0x83
#define VK_F21 0x84
#define VK_F22 0x85
#define VK_F23 0x86
#define VK_F24 0x87

#define VK_NUMLOCK 0x90
#define VK_SCROLL 0x91 // SCROLL LOCK key

#define VK_LSHIFT 0xA0
#define VK_RSHIFT 0xA1

#define VK_LCONTROL 0xA2
#define VK_RCONTROL 0xA3

#define VK_LMENU 0xA4 // Alt
#define VK_RMENU 0xA5

#define VK_BROWSER_BACK 0xA6
#define VK_BROWSER_FORWARD 0xA7
#define VK_BROWSER_REFRESH 0xA8
#define VK_BROWSER_STOP 0xA9
#define VK_BROWSER_SEARCH 0xAA
#define VK_BROWSER_FAVORITES 0xAB
#define VK_BROWSER_HOME 0xAC

#define VK_VOLUME_MUTE 0xAD
#define VK_VOLUME_DOWN 0xAE
#define VK_VOLUME_UP 0xAF

#define VK_MEDIA_NEXT_TRACK 0xB0
#define VK_MEDIA_PREV_TRACK 0xB1
#define VK_MEDIA_STOP 0xB2
#define VK_MEDIA_PLAY_PAUSE 0xB3

#define VK_LAUNCH_MAIL 0xB4
#define VK_LAUNCH_MEDIA_SELECT 0xB5
#define VK_LAUNCH_APP1 0xB6
#define VK_LAUNCH_APP2 0xB7

#define VK_OEM_1 0xBA // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ';:' key
#define VK_OEM_PLUS 0xBB // For any country/region, the '+' key
#define VK_OEM_COMMA 0xBC // For any country/region, the ',' key
#define VK_OEM_MINUS 0xBD // For any country/region, the '-' key
#define VK_OEM_PERIOD 0xBE // For any country/region, the '.' key
#define VK_OEM_2 0xBF // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '/?' key
#define VK_OEM_3 0xC0 // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '`~' key
#define VK_OEM_4 0xDB // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '[{' key
#define VK_OEM_5 0xDC // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '\|' key
#define VK_OEM_6 0xDD // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ']}' key
#define VK_OEM_7 0xDE // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the 'single-quote/double-quote' key
#define VK_OEM_8 0xDF // Used for miscellaneous characters; it can vary by keyboard.
#define VK_OEM_102 0xE2 // Either the angle bracket key or the backslash key on the RT 102-key keyboard

#define VK_PROCESSKEY 0xE5 // IME PROCESS key
#define VK_PACKET 0xE7 // Used to pass Unicode characters as if they were keystrokes.The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods.For more information, see Remark in KEYBDINPUT, SendInput, WM_KEYDOWN, and WM_KEYUP
#define VK_ATTN 0xF6 // Attn key ??
#define VK_CRSEL 0xF7 // CrSel key ??
#define VK_EXSEL 0xF8 // ExSel key ???
#define VK_EREOF 0xF9 // Erase EOF key ???? People have these?
#define VK_PLAY 0xFA
#define VK_ZOOM 0xFB
#define VK_NONAME 0xFC // Reserved Alright...
#define VK_PA1 0xFD // PA1 key... What is this?

#define VK_OEM_CLEAR 0xFE // Clear key... I'm guessing this is seperate from the CLEAR key, huh?
#if __cplusplus
}
#endif

#define jec_win_h
#endif