#pragma once
#include <windows.h>
#include <string>

// Función para extraer solo el nombre del archivo sin el path
std::wstring GetFileNameFromPath(const std::wstring& fullPath);

// Función para convertir wstring a string UTF-8
std::string WStringToString(const std::wstring& wstr);