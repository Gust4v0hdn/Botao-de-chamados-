#include <windows.h>
#include <stdio.h>
#include <string.h>

int getAppPath(char* exePath, DWORD size) {
    // Obtém caminho do executável atual
    if (!GetModuleFileName(NULL, exePath, size)) {
        MessageBox(NULL, "Erro ao obter caminho do executável.", "Erro", MB_ICONERROR);
        return 0;
    }

    // Encontra última barra para separar diretório
    char* lastSlash = strrchr(exePath, '\\');
    if (!lastSlash) {
        MessageBox(NULL, "Caminho inválido do executável.", "Erro", MB_ICONERROR);
        return 0;
    }
    
    // Substitui nome do executável pelo desejado
    strcpy(lastSlash + 1, "BOTAO FL.exe");

    // Verifica existência do arquivo
    if (GetFileAttributes(exePath) == INVALID_FILE_ATTRIBUTES) {
        MessageBox(NULL, "Arquivo não encontrado: BOTAOFL.exe", "Erro", MB_ICONERROR);
        return 0;
    }
    
    return 1;
}

int main() {
    HKEY hKey;
    const char* regPath = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    const char* appName = "BOTAO FL";
    char appPath[MAX_PATH];

    // 1. Obter caminho completo do aplicativo
    if (!getAppPath(appPath, MAX_PATH)) {
        return 1;
    }

    // 2. Abrir chave do registro
    LONG result = RegOpenKeyEx(
        HKEY_CURRENT_USER,
        regPath,
        0,
        KEY_WRITE,
        &hKey
    );

    if (result != ERROR_SUCCESS) {
        MessageBox(NULL, "Erro ao abrir chave do registro.", "Erro", MB_ICONERROR);
        return 1;
    }

    // 3. Escrever valor no registro
    result = RegSetValueEx(
        hKey,
        appName,
        0,
        REG_SZ,
        (const BYTE*)appPath,
        strlen(appPath) + 1
    );

    if (result != ERROR_SUCCESS) {
        MessageBox(NULL, "Erro ao escrever no registro.", "Erro", MB_ICONERROR);
        RegCloseKey(hKey);
        return 1;
    }

    RegCloseKey(hKey);
    MessageBox(NULL, "Configuração realizada com sucesso!", "Sucesso", MB_ICONINFORMATION);
    return 0;
}

