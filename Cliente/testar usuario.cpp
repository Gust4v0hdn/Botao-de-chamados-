#include <windows.h>
#include <shellapi.h>
#include <string.h>
#include <stdio.h>

#define PASSWORD "guh@@123"  // Substitua pela senha correta

void SimulateKeyPress(const char* text) {
    INPUT inputs[256] = { 0 };
    int len = strlen(text);

    for (int i = 0; i < len; i++) {
        SHORT vk = VkKeyScan(text[i]);
        if (vk == -1) {
            // Se for um caractere especial, simular entrada alternativa
            if (text[i] == '@') {
                keybd_event(VK_SHIFT, 0, 0, 0);
                keybd_event('2', 0, 0, 0);  // SHIFT + 2 = @ no layout ABNT
                keybd_event('2', 0, KEYEVENTF_KEYUP, 0);
                keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
            }
        } else {
            inputs[i].type = INPUT_KEYBOARD;
            inputs[i].ki.wVk = vk;
        }
    }

    SendInput(len, inputs, sizeof(INPUT));
}

void OpenURLAndLogin() {
    char username[256];
    DWORD username_len = sizeof(username);
    
    // Obtém o nome do usuário logado no PC
    if (GetUserName(username, &username_len)) {
        strcat(username, "");  // Adicione o domínio desejado
    } else {
        strcpy(username, "usuario_padrao@dominio.com.br");  // Caso falhe, usa um padrão
    }

    // Abre o navegador na URL
    ShellExecute(NULL, "open", "https://glpi.naturafrig.com.br/marketplace/formcreator/front/formlist.php", NULL, NULL, SW_SHOWNORMAL);
    
    Sleep(3000);  // Aguarda o navegador abrir (ajuste conforme necessário)

    // Simula a digitação do usuário
    SimulateKeyPress(username);
    Sleep(500);  // Aguarda antes de pressionar TAB

    // Simula pressionar TAB para ir para o campo de senha
    keybd_event(VK_TAB, 0, 0, 0);
    keybd_event(VK_TAB, 0, KEYEVENTF_KEYUP, 0);
    Sleep(500);

    // Simula a digitação da senha
    SimulateKeyPress(PASSWORD);
    Sleep(500);

  
}

int main() {
    OpenURLAndLogin();
    return 0;
}

