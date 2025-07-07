#pragma once
#include <string>
#include <windows.h>
#include "qrcodegen.hpp"

using qrcodegen::QrCode;

// Funci�n para generar el c�digo QR
QrCode* GenerateQRCode(const std::wstring& localIp, HWND hEditPort, const std::wstring& filePath, std::string& outUrl);

// Funci�n para dibujar el c�digo QR en la ventana
void DrawQRCode(HDC hdc, const RECT& clientRect, const QrCode* qrCode, const std::wstring& fileName, HFONT hFont);