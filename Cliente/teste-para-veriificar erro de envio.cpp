#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <wininet.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wininet.lib")

#define PORT 5000
#define BUFFER_SIZE 1024
#define UNLEN 256
#define RETRY_DELAY 5

#define TOKEN "7960933300:AAFJwalU-pXrZOOvzRQP2P_wtmr-Z5IDb3E"
#define CHAT_ID "-4667271085"

const char* IPS[] = {
    "172.16.41.109",
    "172.16.41.135",
    "172.16.41.201"
};
#define TOTAL_IPS 3

char mensagemUsuario[512] = "";

void enviarMensagem(const char *token, const char *chat_id, const char *mensagem) {
    char url[512];
    snprintf(url, sizeof(url), "https://api.telegram.org/bot%s/sendMessage?chat_id=%s&text=%s&parse_mode=Markdown", token, chat_id, mensagem);
    HINTERNET net = InternetOpen("bot", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (net) {
        HINTERNET conn = InternetOpenUrl(net, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (conn) InternetCloseHandle(conn);
        InternetCloseHandle(net);
    }
}

void obterHora(char *hora, size_t tam) {
    time_t t = time(NULL);
    strftime(hora, tam, "%d/%m/%Y %H:%M", localtime(&t));
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static BOOL enviado = FALSE;
    static COLORREF currentColor = RGB(128, 111, 111);

    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, CreateSolidBrush(currentColor));
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            HFONT hFont = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
            SelectObject(hdc, hFont);
            RECT rect;
            GetClientRect(hwnd, &rect);
            DrawText(hdc, enviado ? "ENVIADO" : (GetTickCount() / 500 % 2 ? "AGUARDANDO..." : "AGUARDANDO.."), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            DeleteObject(hFont);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_TIMER:
            if (wParam == 1 && !enviado) {
                enviarMensagem(TOKEN, CHAT_ID, mensagemUsuario);
                enviado = TRUE;
                currentColor = RGB(0, 128, 0);
                InvalidateRect(hwnd, NULL, TRUE);
                SetTimer(hwnd, 2, 5000, NULL);
            } else if (wParam == 2) {
                PostQuitMessage(0);
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char *CLASS_NAME = "SimpleWindowClass";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        CLASS_NAME, "Envio Telegram", WS_POPUP,
        1500, 1035, 200, 50,
        NULL, NULL, hInstance, NULL);

    if (!hwnd) {
        MessageBox(NULL, "Erro ao criar janela!", "Erro", MB_OK | MB_ICONERROR);
        return -1;
    }

    // Solicita entrada do usuário
    char entrada[512];
    if (MessageBox(NULL, "Digite a mensagem para enviar", "Entrada", MB_OKCANCEL) == IDOK) {
        GetWindowText(hwnd, entrada, sizeof(entrada));
        strncpy(mensagemUsuario, entrada, sizeof(mensagemUsuario) - 1);
    }

    ShowWindow(hwnd, nCmdShow);
    SetTimer(hwnd, 1, RETRY_DELAY * 1000, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

