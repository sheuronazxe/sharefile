#pragma once
#include <string>
#include <windows.h>
#include "qrcodegen.hpp"

using qrcodegen::QrCode;

// Función para generar el código QR
QrCode* GenerateQRCode(const std::wstring& localIp, HWND hEditPort, const std::wstring& filePath, std::string& outUrl);

// Función para dibujar el código QR en la ventana
void DrawQRCode(HDC hdc, const RECT& clientRect, const QrCode* qrCode, const std::wstring& fileName, HFONT hFont);