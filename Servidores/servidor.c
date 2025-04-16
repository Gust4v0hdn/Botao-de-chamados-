#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <wininet.h>
#include <lmcons.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wininet.lib")

#define PORT 5000
#define BUFFER_SIZE 1024
#define HISTORICO_MAX 8192

char historico[HISTORICO_MAX] = "";

void adicionarAoHistorico(const char *mensagem) {
    char dataHora[64];
    time_t agora = time(NULL);
    struct tm *tm_info = localtime(&agora);
    strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", tm_info);

    char entrada[BUFFER_SIZE + 128];
    snprintf(entrada, sizeof(entrada), "[%s] %s\n", dataHora, mensagem);

    if (strlen(historico) + strlen(entrada) < HISTORICO_MAX) {
        strcat(historico, entrada);
    } else {
        strcat(historico, "\n[Histórico cheio]\n");
    }
}

void enviarTelegram(const char *mensagem) {
    const char *token = "7960933300:AAFJwalU-pXrZOOvzRQP2P_wtmr-Z5IDb3E";
    const char *chat_id = "-4667271085";
    char url[1024];
    snprintf(url, sizeof(url), "https://api.telegram.org/bot%s/sendMessage?chat_id=%s&text=%s", token, chat_id, mensagem);

    HINTERNET hInternet = InternetOpen("TelegramBot", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet) {
        HINTERNET hConnect = InternetOpenUrl(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (hConnect) {
            InternetCloseHandle(hConnect);
        }
        InternetCloseHandle(hInternet);
    }
}

void obterUsuario(char *usuario, DWORD tamanho) {
    GetUserName(usuario, &tamanho);
}

void mostrarNotificacao(const char *mensagem) {
    adicionarAoHistorico(mensagem);

    char texto[BUFFER_SIZE + 100];
    snprintf(texto, sizeof(texto), "%s\n\nDeseja Aceitar ou Recusar?", mensagem);
    int resposta = MessageBox(NULL, texto, "Alerta da Industria", MB_YESNO | MB_ICONERROR | MB_SYSTEMMODAL);
    char usuario[UNLEN + 1];
    DWORD tam = UNLEN + 1;
    obterUsuario(usuario, tam);

    if (resposta == IDYES) {
        char msgTelegram[512];
        snprintf(msgTelegram, sizeof(msgTelegram), "#Atendido por %s", usuario);
        enviarTelegram(msgTelegram);
    } else if (resposta == IDNO) {
        char msgTelegram[512];
        snprintf(msgTelegram, sizeof(msgTelegram), "#Recusado por %s", usuario);
        enviarTelegram(msgTelegram);
    }
}

// Hook de teclado
HHOOK hook;

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;
        if (p->vkCode == VK_F12 &&
            (GetAsyncKeyState(VK_LWIN) & 0x8000 || GetAsyncKeyState(VK_RWIN) & 0x8000)) {

            // Mostrar histórico das mensagens com datas
            MessageBox(NULL, historico[0] ? historico : "Nenhuma mensagem recebida ainda.",
                       "Histórico de Mensagens", MB_OK | MB_ICONINFORMATION);
        }
    }
    return CallNextHookEx(hook, nCode, wParam, lParam);
}

DWORD WINAPI tecladoThread(LPVOID lpParam) {
    hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(hook);
    return 0;
}

int main() {
    ShowWindow(GetConsoleWindow(), SW_HIDE); // Oculta console
    CreateThread(NULL, 0, tecladoThread, NULL, 0, NULL); // Atalho

    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int client_len = sizeof(client);
    char buffer[BUFFER_SIZE];

    WSAStartup(MAKEWORD(2, 2), &wsa);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) return 1;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) return 1;

    listen(server_socket, 3);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client, &client_len);
        if (client_socket == INVALID_SOCKET) continue;

        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (strlen(buffer) > 0) {
            mostrarNotificacao(buffer);
        }

        closesocket(client_socket);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}


