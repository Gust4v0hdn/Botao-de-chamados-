#include <windows.h>
#include <shellapi.h>
#include <stdbool.h>
#include <string.h>

POINT ptLastMousePos;
BOOL isDragging = FALSE;
COLORREF currentColor = RGB(135, 206, 250);
DWORD lastClickTime = 0;
const DWORD doubleClickInterval = GetDoubleClickTime();

HWND hEdit, hPopup;

void OpenURL() {
    ShellExecute(NULL, "open", "https://glpi.naturafrig.com.br/marketplace/formcreator/front/formlist.php", NULL, NULL, SW_SHOWNORMAL);
}

LRESULT CALLBACK PopupProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            CreateWindow("STATIC", "Digite a senha:", WS_VISIBLE | WS_CHILD,
                         10, 10, 120, 20, hwnd, NULL, NULL, NULL);
            hEdit = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD,
                                 10, 40, 120, 20, hwnd, NULL, NULL, NULL);
            CreateWindow("BUTTON", "OK", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                         10, 70, 50, 20, hwnd, (HMENU) 1, NULL, NULL);
                           CreateWindow("BUTTON", "config", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                         80, -10, 50, 20, hwnd, (HMENU) 2, NULL, NULL);
                         
            return 0;

        case WM_COMMAND:
    if (LOWORD(wParam) == 1) {  // Botão OK (verifica senha)
        char senha[10];
        GetWindowText(hEdit, senha, sizeof(senha)); // Garante que não ultrapasse o buffer

        if (strcmp(senha, "1234") == 0) {
            PostQuitMessage(0);
            DestroyWindow(hwnd);
        } else {
            MessageBox(hwnd, "Senha incorreta!", "Erro", MB_OK | MB_ICONERROR);
        }
    } 
    else if (LOWORD(wParam) == 2) {  // Botão Config
        system("cmd.exe /k");
    }
    return 0;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void ShowPasswordPopup(HWND parent) {
    RECT rect;
    GetWindowRect(parent, &rect);
    int x = rect.left;
    int y = rect.top - 100; 

    hPopup = CreateWindowEx(0, "PopupClass", "Senha", WS_OVERLAPPED | WS_SYSMENU,
                            x, y, 170, 130,
                            parent, NULL, NULL, NULL);
    ShowWindow(hPopup, SW_SHOW);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static BOOL resetPending = FALSE;

    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HBRUSH brush = CreateSolidBrush(currentColor);
            FillRect(hdc, &ps.rcPaint, brush);
            DeleteObject(brush);

            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            DrawText(hdc, "T.I", -1, &ps.rcPaint, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_RBUTTONDOWN:
            if (GetTickCount() - lastClickTime <= doubleClickInterval) {
                ShowPasswordPopup(hwnd);
            }
            lastClickTime = GetTickCount();
            return 0;

        case WM_MOUSEMOVE:
            if (isDragging) {
                POINT ptCurrentMousePos;
                GetCursorPos(&ptCurrentMousePos);
                int dx = ptCurrentMousePos.x - ptLastMousePos.x;
                int dy = ptCurrentMousePos.y - ptLastMousePos.y;

                RECT rect;
                GetWindowRect(hwnd, &rect);
                MoveWindow(hwnd, rect.left + dx, rect.top + dy, rect.right - rect.left, rect.bottom - rect.top, TRUE);
                ptLastMousePos = ptCurrentMousePos;

                if (currentColor != RGB(255, 100, 0)) {
                    currentColor = RGB(255, 100, 0);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            return 0;

        case WM_RBUTTONUP:
            isDragging = FALSE;
            ReleaseCapture();
            if (currentColor == RGB(255, 100, 0)) {
                currentColor = RGB(135, 206, 250);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;

        case WM_LBUTTONDOWN:
            currentColor = RGB(128, 128, 128);
            InvalidateRect(hwnd, NULL, TRUE);
            OpenURL();
            SetTimer(hwnd, 1, 200, NULL);
            resetPending = TRUE;
            return 0;

        case WM_TIMER:
            if (resetPending) {
                currentColor = RGB(135, 206, 250);
                InvalidateRect(hwnd, NULL, TRUE);
                KillTimer(hwnd, 1);
                resetPending = FALSE;
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "FloatingButton";
    const char POPUP_CLASS[] = "PopupClass";

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    WNDCLASS pwc = { 0 };
    pwc.lpfnWndProc = PopupProc;
    pwc.hInstance = hInstance;
    pwc.lpszClassName = POPUP_CLASS;
    
    RegisterClass(&wc);
    RegisterClass(&pwc);

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        CLASS_NAME,
        "Floating Button",
        WS_POPUP,
        1600, 1052,
        100, 30,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) {
        MessageBox(NULL, "Falha ao criar a janela!", "Erro", MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

