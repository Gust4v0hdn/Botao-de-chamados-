// ======================== INCLUSÃO DE BIBLIOTECAS ========================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <wininet.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wininet.lib")

// ======================== DEFINIÇÕES DE CONSTANTES ========================
#define PORT 5000

#define BUFFER_SIZE 1024
#define UNLEN 256
#define RETRY_DELAY 5000

#define TOKEN "7960933300:AAFJwalU-pXrZOOvzRQP2P_wtmr-Z5IDb3E"
#define CHAT_ID "-4667271085"

const char* IPS[] = {
    "172.16.41.109",
    "172.16.41.135",
    "172.16.41.201"
};
#define TOTAL_IPS 3

// ======================== VARIÁVEIS GLOBAIS ========================

DWORD WINAPI enviar_mensagem(LPVOID ip);

BOOL enviado = FALSE;
char mensagemGlobal[512];

// ======================== FUNÇÃO PARA ENVIAR MENSAGEM PARA O TELEGRAM ========================

BOOL enviarMensagem(const char *mensagem) {
    char url[1024];
    snprintf(url, sizeof(url),
             "https://api.telegram.org/bot%s/sendMessage?chat_id=%s&text=%s&parse_mode=Markdown",
             TOKEN, CHAT_ID, mensagem);

    HINTERNET net = InternetOpen("bot", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!net) return FALSE;

    HINTERNET conn = InternetOpenUrl(net, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!conn) {
        InternetCloseHandle(net);
        return FALSE;
    }

    char buffer[BUFFER_SIZE + 1];
    DWORD bytesRead;
    BOOL sucesso = FALSE;

    if (InternetReadFile(conn, buffer, BUFFER_SIZE, &bytesRead)) {
        buffer[bytesRead] = '\0';  // Garante fim da string

        // Verifica se a resposta contém "ok":true
        if (strstr(buffer, "\"ok\":true") != NULL) {
            sucesso = TRUE;
        }
    }

    InternetCloseHandle(conn);
    InternetCloseHandle(net);
    return sucesso;
}

// ======================== FUNÇÃO PARA OBTER DATA E HORA ATUAL ========================
void obterHora(char *hora, size_t tam) {
    time_t t = time(NULL);
    strftime(hora, tam, "%d/%m/%Y %H:%M", localtime(&t));
}

// ========================Função para obter o nome do computador========================
void obterHostname(char *hostname) {
    DWORD tamanho = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerName(hostname, &tamanho);
}

// ======================== CALLBACK PARA JANELA DE STATUS ========================

LRESULT CALLBACK AguardandoProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    static COLORREF currentColor = RGB(128, 111, 111);
    
    switch (msg) {
        case WM_CREATE:
            SetTimer(hwnd, 1, RETRY_DELAY, NULL);
            break;
        case WM_TIMER:
            if (!enviado) {
                enviado = enviarMensagem(mensagemGlobal);
                if (enviado) {
                    currentColor = RGB(0, 128, 0);
                    InvalidateRect(hwnd, NULL, TRUE);
                    SetTimer(hwnd, 2, 10000, NULL);
                }
            } else if (wp == 2) {
                DestroyWindow(hwnd);
            }
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, CreateSolidBrush(currentColor));
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 255, 255));
            DrawText(hdc, enviado ? "ENVIADO" : "CARREGANDO", -1, &ps.rcPaint, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

// ========================Executar outro executável da pasta========================
void ExecuteTI() {
    char exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);

    char* lastSlash = strrchr(exePath, '\\');
    if (lastSlash) *(lastSlash + 1) = '\0';

    strcat(exePath, "Recap-botaoescolha.exe");

    if (GetFileAttributes(exePath) != INVALID_FILE_ATTRIBUTES) {
        ShellExecute(NULL, "open", exePath, NULL, NULL, SW_SHOWNORMAL);
        exit(0);
    } else {
        MessageBox(NULL, "O arquivo bot_escreverchamado.exe não foi encontrado na mesma pasta.", "Erro", MB_OK | MB_ICONERROR);
    }
}

// ======================== FUNÇÃO PARA CRIAR JANELA DE STATUS ========================
void abrirAguardandoPopup() {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = AguardandoProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "AguardandoClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST, "AguardandoClass", "Status", WS_POPUP | WS_VISIBLE,
         0, 450,110, 30, NULL, NULL, wc.hInstance, NULL); //local e tamanho do botao flutuante: eixo X , Y tamanho Largura ,Comprimento
    ShowWindow(hwnd, SW_SHOW);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    static HWND TextBox;
    char user[UNLEN + 1];
    DWORD tam = UNLEN + 1;
    GetUserName(user, &tam);

    switch (msg) {
    	
    	case WM_KEYDOWN:
    if (wp == VK_RETURN) {
        SendMessage(hwnd, WM_COMMAND, 1, 0);
    }
    break;
        case WM_CREATE:
            TextBox = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_LEFT, 10, 10, 300, 60, hwnd, (HMENU)3, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Enviar", WS_VISIBLE | WS_CHILD | WS_TABSTOP, 320, 5, 70, 30, hwnd, (HMENU)1, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Cancelar", WS_VISIBLE | WS_CHILD | WS_TABSTOP, 320, 44, 70, 30, hwnd, (HMENU)2, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("STATIC", "Digite...", WS_VISIBLE | WS_CHILD, 11, 11, 298, 58, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

            
            SetFocus(TextBox);
            break;
        case WM_COMMAND:
            if (LOWORD(wp) == 1) {
               char texto[256], hora[64];
			GetWindowText(TextBox, texto, sizeof(texto));
			if (strlen(texto) > 0) {
    			obterHora(hora, sizeof(hora));
    		   char hostname[MAX_COMPUTERNAME_LENGTH + 1];
    			obterHostname(hostname);
    			snprintf(mensagemGlobal, sizeof(mensagemGlobal), "Erro: %s%%0AUsuario: %s%%0AComputador: %s%%0AData e Hora: %s", texto, user, hostname, hora);
    				DestroyWindow(hwnd);
    				abrirAguardandoPopup();
}
 else {
                    MessageBox(hwnd, "Digite algo!", "Erro", MB_OK | MB_ICONERROR);
                }
            } else if (LOWORD(wp) == 2) {
                ExecuteTI();
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

// ======================== FUNÇÃO PRINCIPAL DO PROGRAMA ========================

int WINAPI WinMain(HINSTANCE h, HINSTANCE p, LPSTR l, int n) {
	
	// ======================== INCLUSÃO DE FUNÇÃO WNDPROC ======================
	
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = h;
    wc.lpszClassName = "BotJanela";
    RegisterClass(&wc);

    HWND hwnd = CreateWindow("BotJanela", "Chamado TI", WS_OVERLAPPED | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 420, 110, NULL, NULL, h, NULL);
    ShowWindow(hwnd, n);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
  // ======================== ENVIO DE MENSAGEM PARA MÚLTIPLOS IPs ========================
  
    int i;
    HANDLE threads[TOTAL_IPS];

    for (i = 0; i < TOTAL_IPS; i++) {
        threads[i] = CreateThread(NULL, 0, enviar_mensagem, (LPVOID)IPS[i], 0, NULL);
    }

    WaitForMultipleObjects(TOTAL_IPS, threads, TRUE, INFINITE);

    return 0;
}

// ======================== FUNÇÃO PARA ENVIAR MENSAGEM PARA IPs ========================

DWORD WINAPI enviar_mensagem(LPVOID ip) {
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;
    char mensagem[] = "TELEGRAM: Alerta recebido";

    WSAStartup(MAKEWORD(2, 2), &wsa);
    s = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr((char*)ip);

    connect(s, (struct sockaddr*)&server, sizeof(server));
    send(s, mensagem, strlen(mensagem), 0);
    closesocket(s);
    WSACleanup();
    return 0;
}
