#include "helper_ip.h"
#include <iostream>
#include <string>
#include <vector>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

std::wstring GetLocalIPv4Address() {
    // Inicializar Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
        std::cerr << "Error: Failed to initialize Winsock." << std::endl;
        return L"";
    }

    std::wstring ipAddress = L"";
    ULONG bufferSize = 15000; // Tamaño inicial del buffer

    std::vector<BYTE> adapterInfoBuffer(bufferSize);
    PIP_ADAPTER_ADDRESSES pAdapterAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(adapterInfoBuffer.data());

    DWORD result = GetAdaptersAddresses(AF_INET,
        GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME,
        nullptr, pAdapterAddresses, &bufferSize);

    if (result == ERROR_BUFFER_OVERFLOW) {
        adapterInfoBuffer.resize(bufferSize);
        pAdapterAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(adapterInfoBuffer.data());
        result = GetAdaptersAddresses(AF_INET,
            GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME,
            nullptr, pAdapterAddresses, &bufferSize);
    }

    if (result == NO_ERROR) {
        for (PIP_ADAPTER_ADDRESSES pAdapter = pAdapterAddresses; pAdapter != nullptr; pAdapter = pAdapter->Next) {
            if (pAdapter->OperStatus == IfOperStatusUp) {
                for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pAdapter->FirstUnicastAddress; pUnicast != nullptr; pUnicast = pUnicast->Next) {
                    if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
                        sockaddr_in* pSockAddr = reinterpret_cast<sockaddr_in*>(pUnicast->Address.lpSockaddr);
                        wchar_t ipBuffer[INET_ADDRSTRLEN];
                        if (InetNtopW(AF_INET, &(pSockAddr->sin_addr), ipBuffer, _countof(ipBuffer)) != nullptr) {
                            if (wcscmp(ipBuffer, L"127.0.0.1") != 0) {
                                ipAddress = ipBuffer;
                                WSACleanup();
                                return ipAddress; // Primera IP no loopback
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        std::cerr << "Error: GetAdaptersAddresses failed with error code: " << result << std::endl;
    }

    WSACleanup();
    return ipAddress;
}
