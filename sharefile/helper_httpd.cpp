#include "helper_httpd.h"
#include "mongoose.h"
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <cstdio>
#include <fstream>
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <mutex>
#include "helper.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

static std::string g_filePath;
static std::thread g_serverThread;
static std::atomic<bool> g_shouldStop(false);
static struct mg_mgr g_mgr;

#define CHUNK_SIZE 262144  // 256KB chunks for better performance
#define SEND_BUFFER_THRESHOLD 32768  // 32KB threshold for sending

struct FileContext {
    std::ifstream file;
    std::string filename;
    size_t fileSize;
    size_t totalSent;
    std::vector<char> buffer;  // Buffer reutilizable para cada contexto

    FileContext(const std::string& filepath) : totalSent(0), buffer(CHUNK_SIZE) {
        file.open(filepath, std::ios::binary);
        if (file.good()) {
            filename = std::filesystem::path(filepath).filename().string();
            file.seekg(0, std::ios::end);
            fileSize = file.tellg();
            file.seekg(0, std::ios::beg);
        }
    }

    ~FileContext() {
        if (file.is_open()) {
            file.close();
        }
    }

    bool isValid() const {
        return file.good() && fileSize > 0;
    }
};

// Mapa para almacenar contextos de archivo por conexión con protección thread-safe
static std::unordered_map<struct mg_connection*, std::unique_ptr<FileContext>> g_fileContexts;
static std::mutex g_fileContextsMutex;

// Funciones auxiliares thread-safe para manejar contextos
static void AddFileContext(struct mg_connection* c, std::unique_ptr<FileContext> ctx) {
    std::lock_guard<std::mutex> lock(g_fileContextsMutex);
    g_fileContexts[c] = std::move(ctx);
}

static FileContext* GetFileContext(struct mg_connection* c) {
    std::lock_guard<std::mutex> lock(g_fileContextsMutex);
    auto it = g_fileContexts.find(c);
    return (it != g_fileContexts.end()) ? it->second.get() : nullptr;
}

static void RemoveFileContext(struct mg_connection* c) {
    std::lock_guard<std::mutex> lock(g_fileContextsMutex);
    auto it = g_fileContexts.find(c);
    if (it != g_fileContexts.end()) {
        g_fileContexts.erase(it);
    }
}

static void ev_handler(struct mg_connection* c, int ev, void* ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message* hm = (struct mg_http_message*)ev_data;
        const auto* filepath = static_cast<const std::string*>(c->fn_data);

        // Validar que el puntero sea válido
        if (!filepath) {
            mg_http_reply(c, 500, "Content-Type: text/plain\r\n", "Internal server error.");
            return;
        }

        // Crear contexto de archivo
        auto fileCtx = std::make_unique<FileContext>(*filepath);

        if (!fileCtx->isValid()) {
            mg_http_reply(c, 404, "Content-Type: text/plain\r\n", "File not found or permission denied.");
            return;
        }

        // Preparar headers para streaming
        std::string headers = "Content-Type: application/octet-stream\r\n";
        headers += "Content-Disposition: attachment; filename=\"" + fileCtx->filename + "\"\r\n";
        headers += "Content-Length: " + std::to_string(fileCtx->fileSize) + "\r\n";
        headers += "Accept-Ranges: bytes\r\n";

        // Enviar headers
        mg_printf(c, "HTTP/1.1 200 OK\r\n%s\r\n", headers.c_str());

        // Guardar contexto en el mapa de forma thread-safe
        AddFileContext(c, std::move(fileCtx));

    }
    else if (ev == MG_EV_POLL) {
        // Continuar enviando datos del archivo
        FileContext* fileCtx = GetFileContext(c);

        if (fileCtx && fileCtx->isValid() && fileCtx->totalSent < fileCtx->fileSize) {
            // Verificar si hay espacio en el buffer de salida (más agresivo)
            if (c->send.len < SEND_BUFFER_THRESHOLD) {
                size_t remaining = fileCtx->fileSize - fileCtx->totalSent;
                size_t toRead = (remaining < CHUNK_SIZE) ? remaining : CHUNK_SIZE;

                // Usar el buffer reutilizable del contexto
                fileCtx->file.read(fileCtx->buffer.data(), toRead);
                size_t bytesRead = fileCtx->file.gcount();

                if (bytesRead > 0) {
                    mg_send(c, fileCtx->buffer.data(), bytesRead);
                    fileCtx->totalSent += bytesRead;
                }
                else {
                    // Error de lectura o fin de archivo
                    fileCtx->totalSent = fileCtx->fileSize;
                }
            }

            // Si hemos enviado todo el archivo, cerrar la conexión
            if (fileCtx->totalSent >= fileCtx->fileSize) {
                c->is_draining = 1;  // Marcar para cerrar después de enviar todo
            }
        }

    }
    else if (ev == MG_EV_CLOSE) {
        // Limpiar contexto del archivo al cerrar la conexión de forma thread-safe
        RemoveFileContext(c);
    }
}

void ServerThreadFunction(const std::string* filepath_utf8, int port) {
    mg_mgr_init(&g_mgr);

    std::string url = "http://0.0.0.0:" + std::to_string(port);
    struct mg_connection* c = mg_http_listen(&g_mgr, url.c_str(), ev_handler, (void*)filepath_utf8);

    if (!c) {
        fprintf(stderr, "Error: no se pudo iniciar el servidor en %s\n", url.c_str());
        mg_mgr_free(&g_mgr);
        return;
    }

    printf("Servidor HTTP iniciado en %s\n", url.c_str());
    printf("Sirviendo archivo: %s\n", filepath_utf8->c_str());

    while (!g_shouldStop.load()) {
        mg_mgr_poll(&g_mgr, 1);  // Polling más agresivo: 1ms
    }

    // Limpiar contextos restantes de forma thread-safe
    {
        std::lock_guard<std::mutex> lock(g_fileContextsMutex);
        g_fileContexts.clear();
    }
    mg_mgr_free(&g_mgr);
}

void StartHTTPServer(wchar_t* filepath, int port) {
    StopHTTPServer();
    g_filePath = WStringToString(filepath);
    g_shouldStop = false;
    g_serverThread = std::thread(ServerThreadFunction, &g_filePath, port);
}

void StopHTTPServer() {
    if (g_serverThread.joinable()) {
        g_shouldStop = true;
        g_serverThread.join();

        // Limpiar todos los contextos al parar el servidor
        std::lock_guard<std::mutex> lock(g_fileContextsMutex);
        g_fileContexts.clear();
    }
}