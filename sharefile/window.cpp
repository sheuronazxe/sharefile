#define IDI_ICON1	101
#include "window.h"
#include "window_proc.h"
#include "resource.h"

const wchar_t CLASS_NAME[] = L"MyWindowClass";

bool RegisterMainWindowClass(HINSTANCE hInstance) {
	WNDCLASSEX wc		= {};
	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= WindowProc;
	wc.hInstance		= hInstance;
	wc.hCursor			= LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName	= CLASS_NAME;
	wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hIconSm			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

	if (!RegisterClassEx(&wc)) {
		MessageBoxW(nullptr, L"Window Registration Failed!", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	return true;
}

HWND CreateMainWindow(HINSTANCE hInstance) {
	HWND hwnd = CreateWindowExW(
		0,
		CLASS_NAME,
		L"share.file",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		300,
		344,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	if (!hwnd) {
		MessageBoxW(nullptr, L"Window Creation Failed!", L"Error", MB_OK | MB_ICONERROR);
	}

	return hwnd;
}
