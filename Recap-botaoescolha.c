#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <wininet.h>
#include <time.h>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "ws2_32.lib")

#define UNLEN 256
#define PORT 5000
#define BUFFER_SIZE 1024
#define RETRY_DELAY 5000

#define TOKEN "7960933300:AAFJwalU-pXrZOOvzRQP2P_wtmr-Z5IDb3E"
#define CHAT_ID "-4667271085"


const char *choices[] = {"FALHA DE IMPRESSAO", "ROLETE GRUDANDO","BALANCA","FALHA SISTEMA", "COMPUTADOR", "OUTRO (Digite a mensagem)", "CANCELAR"};
const char* IPS[] = {
    "172.16.41.109",
    "172.16.41.135",
    "172.16.41.201"
};
#define TOTAL_IPS 3

// ======================== VARIÁVEIS GLOBAIS ========================
typedef struct {
    char tipoErro[128];
    char userName[UNLEN + 1];
} DadosErro;

void ExecuteTI();

DadosErro dadosErroGlobal = {0};

DWORD WINAPI enviar_mensagem_thread(LPVOID ip);
BOOL enviado = FALSE;
char mensagemGlobal[512];

//--------------------------------------------------------------
// Função para enviar mensagem pelo Telegram
BOOL enviarMensagem(const char *mensagem) {
    char url[1024];
    snprintf(url, sizeof(url),
             "https://api.telegram.org/bot%s/sendMessage?chat_id=%s&text=%s&parse_mode=Markdown",
             TOKEN, CHAT_ID, mensagem);

    HINTERNET net = InternetOpen("TelegramBot", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
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

//--------------------------------------------------------------
// Função para obter data e hora atual
void obterDataHoraAtual(char *dataHora) {
    time_t agora = time(NULL);
    struct tm *info = localtime(&agora);
    snprintf(dataHora, 64, "%02d/%02d/%04d %02d:%02d",
             info->tm_mday, info->tm_mon + 1, info->tm_year + 1900,
             info->tm_hour, info->tm_min);
}

//--------------------------------------------------------------
// Função para obter o nome do computador
void obterHostname(char *hostname) {
    DWORD tamanho = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerName(hostname, &tamanho);
}

// ======================== CALLBACK PARA JANELA DE STATUS ========================possui funçao de teste de envio
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
                    SetTimer(hwnd, 2, 10000, NULL); // X,XESSA FUNÇAO MUDA O TEMPO DE ESPERA DO BOTAO
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
//--------------------------------------------------------------
// Função para exibir janela de carregamento
void abrirAguardandoPopup() {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = AguardandoProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "AguardandoClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST,
        "AguardandoClass",
        "Status",
        WS_POPUP | WS_VISIBLE,
        0, 450, 110, 30,
        NULL, NULL, wc.hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

//--------------------------------------------------------------
// Função para processar erro e enviar mensagem
void processarErro(const char *tipoErro, const char *userName){

    char dataHora[64], hostname[MAX_COMPUTERNAME_LENGTH + 1];
    obterDataHoraAtual(dataHora);
    obterHostname(hostname);
    snprintf(mensagemGlobal, sizeof(mensagemGlobal),
        "Erro: %s%%0AUsuario: %s%%0AComputador: %s%%0AData e hora: %s%%0AT.I acionado.",
        tipoErro, userName, hostname, dataHora);
    abrirAguardandoPopup();
  HANDLE threads[TOTAL_IPS];

for (int i = 0; i < TOTAL_IPS; i++) {
    threads[i] = CreateThread(NULL, 0, enviar_mensagem_thread, (LPVOID)IPS[i], 0, NULL);
}

// Espera todas as threads terminarem
WaitForMultipleObjects(TOTAL_IPS, threads, TRUE, INFINITE);

// Fecha os handles (boa prática)
for (int i = 0; i < TOTAL_IPS; i++) {
    CloseHandle(threads[i]);
}


}

//--------------------------------------------------------------
// Função de janela para processar comandos
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static char userName[UNLEN + 1];
    static DWORD userNameLen = UNLEN + 1;
  
    switch (uMsg) {
        case WM_CREATE:
            GetUserName(userName, &userNameLen);
            break;

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
          if (wmId >= 1 && wmId <= 5) {
    strcpy(dadosErroGlobal.tipoErro, choices[wmId - 1]);
    strcpy(dadosErroGlobal.userName, userName);
    DestroyWindow(hwnd);  // Fecha a janela, e o resto vai rodar no WM_DESTROY
}


            if (wmId == 6) ExecuteTI();
            if (wmId == 7) {
            	exit(0);
			}
            break;
        }

        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            HBRUSH hBrush = CreateSolidBrush(RGB(200, 200, 200)); // Cinza claro
            FillRect(hdc, &rc, hBrush);
            DeleteObject(hBrush);
            return 1; // Fundo tratado
        }

       case WM_DESTROY:
    // Após a janela principal ser destruída, processa o erro
    if (strlen(dadosErroGlobal.tipoErro) > 0) {
        processarErro(dadosErroGlobal.tipoErro, dadosErroGlobal.userName);
    }
    PostQuitMessage(0);
    break;

    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


//--------------------------------------------------------------
// Executar outro executável da pasta
void ExecuteTI() {
    char exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);

    char* lastSlash = strrchr(exePath, '\\');
    if (lastSlash) *(lastSlash + 1) = '\0';

    strcat(exePath, "bot_escreverchamado.exe");

    if (GetFileAttributes(exePath) != INVALID_FILE_ATTRIBUTES) {
        ShellExecute(NULL, "open", exePath, NULL, NULL, SW_SHOWNORMAL);
        exit(0);
    } else {
        MessageBox(NULL, "O arquivo bot_escreverchamado.exe não foi encontrado na mesma pasta.", "Erro", MB_OK | MB_ICONERROR);
    }
}

//--------------------------------------------------------------
// Função principal para criação da janela
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "FloatingMenuClass";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        CLASS_NAME,
        "Escolha o Motivo do Erro",
        WS_OVERLAPPEDWINDOW,
        800, 400, 230, 330,
        NULL, NULL, hInstance, NULL);

    if (!hwnd) return 0;

    for (int i = 0; i < 7; i++) {
        CreateWindow("BUTTON", choices[i], WS_VISIBLE | WS_CHILD,
                     10, 10 + (i * 40), 200, 30,
                     hwnd, (HMENU)(UINT_PTR)(i + 1), hInstance, NULL);
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

//--------------------------------------------------------------
// Função para enviar mensagem para IPs
DWORD WINAPI enviar_mensagem_thread(LPVOID ip) {
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;
    char mensagem[] = "TELEGRAM: Alerta recebido";

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr((char*)ip);

    connect(s, (struct sockaddr*)&server, sizeof(server));
    send(s, mensagem, strlen(mensagem), 0);
    closesocket(s);
    WSACleanup();

    return 0;
}

