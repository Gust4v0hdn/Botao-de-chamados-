#include <windows.h>
#include "utils.h"

// Callback da janela de chamado TI
LRESULT CALLBACK ChamadoTIProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    static HWND TextBox;
    char user[UNLEN + 1];
    DWORD tam = UNLEN + 1;
    GetUserName(user, &tam);

    switch (msg) {
        case WM_CREATE:
            TextBox = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_LEFT, 10, 10, 300, 60, hwnd, (HMENU)3, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Enviar", WS_VISIBLE | WS_CHILD | WS_TABSTOP, 320, 10, 70, 30, hwnd, (HMENU)1, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Cancelar", WS_VISIBLE | WS_CHILD | WS_TABSTOP, 320, 50, 70, 30, hwnd, (HMENU)2, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            SetFocus(TextBox);
            break;
        case WM_COMMAND:
            if (LOWORD(wp) == 1) {
                char texto[256], hora[64];
                GetWindowText(TextBox, texto, sizeof(texto));
                if (strlen(texto) > 0) {
                    obterHora(hora, sizeof(hora));
                    snprintf(mensagemGlobal, sizeof(mensagemGlobal), "Erro: %s%%0AUsuario: %s%%0AData e Hora: %s", texto, user, hora);
                    DestroyWindow(hwnd);
                    abrirAguardandoPopup();
                } else {
                    MessageBox(hwnd, "Digite algo!", "Erro", MB_OK | MB_ICONERROR);
                }
            } else if (LOWORD(wp) == 2) {
                exit(0);
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

// Função para iniciar a janela de chamado TI
void iniciarChamadoTI(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = ChamadoTIProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "ChamadoTI";
    RegisterClass(&wc);

    HWND hwnd = CreateWindow("ChamadoTI", "Chamado TI", WS_OVERLAPPED | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 420, 110, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}