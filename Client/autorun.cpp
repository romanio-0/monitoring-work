#include "autorun.h"


bool ItselfSetAutorun(wchar_t *path) {
    HKEY hKey;
    DWORD pathSize = (wcslen(path) + 1) * sizeof(wchar_t);

    if (ERROR_SUCCESS != RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                                      0, KEY_READ | KEY_SET_VALUE, &hKey)) {
        return false;
    }

    if (RegSetValueExW(hKey, L"Monitoring Work", 0, REG_SZ,
                       (PBYTE)path, pathSize) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return false;
    }

//    RegCloseKey(hKey);
    return true;
}