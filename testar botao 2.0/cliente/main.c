#include <windows.h>
#include "utils.h"

// Declaração das funções principais de cada módulo
void iniciarFloatingButton(HINSTANCE hInstance, int nCmdShow);
void iniciarChamadoTI(HINSTANCE hInstance, int nCmdShow);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)lpCmdLine; // Avoid unused parameter warning
    // Escolha qual funcionalidade executar
    int escolha = MessageBox(NULL, "Escolha a funcionalidade:\n\nSim: Botão Flutuante\nNão: Chamado TI", 
                             "Escolha", MB_YESNO | MB_ICONQUESTION);

    if (escolha == IDYES) {
        iniciarFloatingButton(hInstance, nCmdShow); // Botão flutuante
    } else {
        iniciarChamadoTI(hInstance, nCmdShow); // Chamado TI
    }

    return 0;
}