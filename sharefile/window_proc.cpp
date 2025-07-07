#include "window_proc.h"
#include <memory>
#include <string>
#include <shellapi.h>
#include <vector>
#include "helper.h"
#include "helper_ip.h"
#include "helper_qr.h"
#include "helper_httpd.h"

HPEN hPen = nullptr;
HBRUSH hBrush = nullptr;
HFONT hFont = nullptr;
HFONT hFontMini = nullptr;
static std::wstring localIp;
static HWND hIpPort = nullptr;
static HWND hEditPort = nullptr;

// Variables para el modo QR
static bool showQrMode = false;
static std::wstring droppedFileName;
static std::string qrUrl;
//static QrCode* qrCode = nullptr;
static std::unique_ptr<QrCode> qrCode = nullptr;
static std::vector<std::wstring> droppedFiles;
static size_t currentFileIndex = 0;

template<typename T>
void SafeDeleteObject(T* obj) {
    if (obj && *obj) {
        DeleteObject(*obj);
        *obj = nullptr;
    }
}

// Función para volver al modo normal
void ReturnToNormalMode(HWND hwnd) {
    showQrMode = false;
    droppedFiles.clear();
    currentFileIndex = 0;

    ShowWindow(hIpPort, SW_SHOW);
    ShowWindow(hEditPort, SW_SHOW);
    InvalidateRect(hwnd, NULL, TRUE);
}

// Función para manejar el archivo arrastrado
void HandleDroppedFile(HWND hwnd, const std::wstring& filePath) {

    // Obtener puerto desde el control de edición
    wchar_t portBuffer[6] = {};
    GetWindowTextW(hEditPort, portBuffer, 6);
    int port = _wtoi(portBuffer);
    if (port < 1024 || port > 65535) {
        MessageBoxW(hwnd,
            L"Valid ports are from 1024 up to 65535.",
            L"Port not allowed",
            MB_ICONWARNING | MB_OK);
        ReturnToNormalMode(hwnd);
        return;
    }

    // Iniciar servidor HTTP
    StopHTTPServer();
    StartHTTPServer((wchar_t*)filePath.c_str(), port);

    qrCode = std::unique_ptr<QrCode>(GenerateQRCode(localIp, hEditPort, filePath, qrUrl));

    // Construir nombre con índice si hay múltiples archivos
    std::wstring baseName = GetFileNameFromPath(filePath);
    if (droppedFiles.size() > 1) {
        wchar_t suffix[32];
        swprintf(suffix, 32, L" [%zu/%zu]", currentFileIndex + 1, droppedFiles.size());
        droppedFileName = baseName + suffix;
    }
    else {
        droppedFileName = baseName;
    }

    // Cambiar al modo QR
    showQrMode = true;

    // Ocultar controles
    ShowWindow(hIpPort, SW_HIDE);
    ShowWindow(hEditPort, SW_HIDE);

    // Redibujar la ventana
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        localIp = GetLocalIPv4Address();

        hPen = CreatePen(PS_DASH, 1, RGB(200, 200, 200));
        hBrush = CreateSolidBrush(RGB(250, 250, 250));

        hFont = CreateFontW(
            24, 0, 0, 0, FW_DEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");

        hFontMini = CreateFontW(
            16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");

        RECT rect;
        GetClientRect(hwnd, &rect);

        const wchar_t* ipText = localIp.empty() ? L"Unable to get IP" : localIp.c_str();
        hIpPort = CreateWindowW(L"EDIT", ipText,
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER | ES_READONLY,
            (rect.left + 10), rect.top + 32, 120, 27,
            hwnd, nullptr, nullptr, nullptr);
        SendMessageW(hIpPort, WM_SETFONT, (WPARAM)hFont, TRUE);
        

        hEditPort = CreateWindowW(L"EDIT", L"8080",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL | ES_CENTER,
            (rect.right - 70), rect.top + 32, 60, 27,
            hwnd, nullptr, nullptr, nullptr);
        SendMessageW(hEditPort, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessageW(hEditPort, EM_SETLIMITTEXT, (WPARAM)5, 0);

        DragAcceptFiles(hwnd, TRUE);
        return 0;
    }

    case WM_DROPFILES: {
        HDROP hDrop = (HDROP)wParam;
        UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);

        droppedFiles.clear();
        currentFileIndex = 0;

        for (UINT i = 0; i < fileCount; ++i) {
            wchar_t filePath[MAX_PATH];
            if (DragQueryFileW(hDrop, i, filePath, MAX_PATH)) {
                DWORD attrs = GetFileAttributesW(filePath);
                if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                    droppedFiles.push_back(filePath);
                }
            }
        }

        DragFinish(hDrop);

        if (!droppedFiles.empty()) {
            HandleDroppedFile(hwnd, droppedFiles[0]);
        }

        return 0;
    }

    case WM_LBUTTONDOWN: {
        if (showQrMode && droppedFiles.size() > 1) {
            currentFileIndex = (currentFileIndex + 1) % droppedFiles.size();
            HandleDroppedFile(hwnd, droppedFiles[currentFileIndex]);
        }
        return 0;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rect;
        GetClientRect(hwnd, &rect);

        if (showQrMode) {
            DrawQRCode(hdc, rect, qrCode.get(), droppedFileName, hFont);
        }
        else {
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
            Rectangle(hdc, rect.left + 10, rect.top + 69, rect.right - 10, rect.bottom - 10);
            SelectObject(hdc, hOldBrush);
            SelectObject(hdc, hOldPen);

            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(200, 80, 120));
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
            RECT dragTextRect = { rect.left, rect.top + 69, rect.right, rect.bottom };
            DrawTextW(hdc, L"Drag file here to share", -1, &dragTextRect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
            SelectObject(hdc, hOldFont);

            SetTextColor(hdc, RGB(120, 120, 120));
            HFONT hOldFontMini = (HFONT)SelectObject(hdc, hFontMini);
            RECT ipLabelRect = { rect.left + 10, rect.top + 10, rect.right - 10, rect.top + 26 };
            DrawTextW(hdc, L"Local IP", -1, &ipLabelRect, DT_SINGLELINE | DT_LEFT);
            RECT portLabelRect = { rect.left, rect.top + 10, rect.right - 10, rect.top + 26 };
            DrawTextW(hdc, L"HTTP port", -1, &portLabelRect, DT_SINGLELINE | DT_RIGHT);
            SelectObject(hdc, hOldFontMini);
        }

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_KEYDOWN: {
        if (wParam == VK_ESCAPE && showQrMode) {
            ReturnToNormalMode(hwnd);
        }
        return 0;
    }

    case WM_DESTROY:
		qrCode.reset();
        SafeDeleteObject(&hPen);
        SafeDeleteObject(&hBrush);
        SafeDeleteObject(&hFont);
        SafeDeleteObject(&hFontMini);
        StopHTTPServer();
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
