#include <windows.h>
#include <time.h>
#include "utils.h"

// Função para obter data e hora atual
void obterHora(char *hora, size_t tam) {
    time_t t = time(NULL);
    strftime(hora, tam, "%d/%m/%Y %H:%M", localtime(&t));
}

// Função para executar o TI
void ExecuteTI() {
    char exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);

    char* lastSlash = strrchr(exePath, '\\');
    if (lastSlash) {
        *(lastSlash + 1) = '\0';
    }

    strcat(exePath, "00 Recap-botaoescolha.exe");

    if (GetFileAttributes(exePath) != INVALID_FILE_ATTRIBUTES) {
        ShellExecute(NULL, "open", exePath, NULL, NULL, SW_SHOWNORMAL);
    } else {
        MessageBox(NULL, "O arquivo não foi encontrado na mesma pasta.", "Erro", MB_ICONERROR);
    }
}