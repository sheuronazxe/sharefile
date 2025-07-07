#include "helper.h"
#include "helper_qr.h"

QrCode* GenerateQRCode(const std::wstring& localIp, HWND hEditPort, const std::wstring& filePath, std::string& outUrl) {
    // Obtener el puerto del campo de texto
    wchar_t portText[10];
    GetWindowTextW(hEditPort, portText, 10);

    // Construir la URL completa
    std::wstring fullUrl = L"http://" + localIp + L":" + std::wstring(portText) + L"/";
    outUrl = WStringToString(fullUrl);

    // Generar y retornar el QR code
    return new QrCode(QrCode::encodeText(outUrl.c_str(), QrCode::Ecc::MEDIUM));
}

void DrawQRCode(HDC hdc, const RECT& clientRect, const QrCode* qrCode, const std::wstring& fileName, HFONT hFont) {
    if (!qrCode) return;

    int qrSize = qrCode->getSize();

    // Definir el margen uniforme para superior, derecho e izquierdo
    const int MARGIN = 25; // Puedes ajustar este valor según necesites

    // Calcular el área disponible para el QR considerando los márgenes
    int availableWidth = clientRect.right - clientRect.left - (2 * MARGIN); // Margen izquierdo y derecho
    int availableHeight = clientRect.bottom - clientRect.top - MARGIN;

    // Calcular el tamaño del QR manteniendo proporción cuadrada
    int qrDisplaySize = min(availableWidth, availableHeight);
    int moduleSize = qrDisplaySize / qrSize;
    qrDisplaySize = moduleSize * qrSize; // Ajustar al tamaño exacto de módulos

    // Posicionar el QR con márgenes uniformes
    int qrStartX = clientRect.left + MARGIN; // Margen izquierdo
    int qrStartY = clientRect.top + MARGIN;  // Margen superior

    // Si el QR es más pequeño que el ancho disponible, centrarlo horizontalmente
    if (qrDisplaySize < availableWidth) {
        qrStartX = clientRect.left + (clientRect.right - clientRect.left - qrDisplaySize) / 2;
    }

    // Crear brush para los módulos del QR
    HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
    HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));

    // Dibujar fondo blanco del QR
    RECT qrRect = { qrStartX, qrStartY, qrStartX + qrDisplaySize, qrStartY + qrDisplaySize };
    FillRect(hdc, &qrRect, whiteBrush);

    // Dibujar los módulos del QR
    for (int y = 0; y < qrSize; y++) {
        for (int x = 0; x < qrSize; x++) {
            if (qrCode->getModule(x, y)) {
                RECT moduleRect = {
                    qrStartX + x * moduleSize,
                    qrStartY + y * moduleSize,
                    qrStartX + (x + 1) * moduleSize,
                    qrStartY + (y + 1) * moduleSize
                };
                FillRect(hdc, &moduleRect, blackBrush);
            }
        }
    }

    // Dibujar el nombre del archivo debajo del QR
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(60, 60, 60));
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    RECT textRect = {
        clientRect.left + MARGIN,
        qrStartY + qrDisplaySize + 10,
        clientRect.right - MARGIN,
        clientRect.bottom - 10
    };

    SetTextColor(hdc, RGB(200, 80, 120));
    DrawTextW(hdc, fileName.c_str(), -1, &textRect, DT_SINGLELINE | DT_CENTER | DT_TOP);
    SelectObject(hdc, hOldFont);

    // Limpiar brushes
    DeleteObject(blackBrush);
    DeleteObject(whiteBrush);
}