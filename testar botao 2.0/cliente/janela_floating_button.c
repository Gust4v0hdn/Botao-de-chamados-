#include <windows.h>
#include "utils.h"

// Variáveis globais específicas do botão flutuante
POINT ptLastMousePos;
BOOL isDragging = FALSE;
COLORREF currentColor = RGB(255, 35, 35); // Cor inicial (vermelho)

// Callback da janela do botão flutuante
LRESULT CALLBACK FloatingButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HBRUSH brush = CreateSolidBrush(currentColor);
            FillRect(hdc, &ps.rcPaint, brush);
            DeleteObject(brush);

            SetTextColor(hdc, RGB(255, 255, 255)); // Texto branco
            SetBkMode(hdc, TRANSPARENT);
            DrawText(hdc, "T.I", -1, &ps.rcPaint, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN:
            currentColor = RGB(128, 128, 128); // Cor cinza ao clicar
            InvalidateRect(hwnd, NULL, TRUE);
            ExecuteTI(); // Chama a função de execução do TI
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Função para iniciar o botão flutuante
void iniciarFloatingButton(HINSTANCE hInstance, int nCmdShow) {
    const char CLASS_NAME[] = "FloatingButton";

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = FloatingButtonProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Falha ao registrar a classe!", "Erro", MB_ICONERROR);
        return;
    }

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        CLASS_NAME,
        "Floating Button",
        WS_POPUP,
        1815, 1020,
        100, 30,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) {
        MessageBox(NULL, "Falha ao criar a janela!", "Erro", MB_ICONERROR);
        return;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}