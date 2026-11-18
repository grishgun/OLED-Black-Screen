// OLED Black Screen - native Win32 utility for Windows x64
// Copyright (c) 2026 OLED Black Screen contributors. MIT license.
#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#define WM_TRAY (WM_APP + 10)
#define WM_BLACK (WM_APP + 11)
#define WM_UNBLACK (WM_APP + 12)
#define MENU_BLACK 1001
#define MENU_STARTUP 1002
#define MENU_QUIT 1003

static const wchar_t *TRAY_CLASS = L"OledBlackScreenTrayWindow.v1";
static const wchar_t *OVERLAY_CLASS = L"OledBlackScreenOverlay.v1";
static const wchar_t *MENU_REG = L"Software\\Classes\\DesktopBackground\\Shell\\OledBlackScreen";
static const wchar_t *RUN_REG = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static HWND tray_window, overlay_window;
static HHOOK keyboard_hook, mouse_hook;
static HICON app_icon;
static HANDLE singleton_mutex;
static UINT taskbar_created;
static int cursor_calls;
static BOOL closing;
static wchar_t exe_path[MAX_PATH];
static NOTIFYICONDATAW nid;

static void registry_string(HKEY key, const wchar_t *name, const wchar_t *value) {
    RegSetValueExW(key, name, 0, REG_SZ, (const BYTE *)value,
                   (DWORD)((lstrlenW(value) + 1) * sizeof(wchar_t)));
}
static BOOL startup_enabled(void) {
    HKEY key;
    DWORD type = 0, size = 0;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, RUN_REG, 0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS)
        return FALSE;
    LONG ret = RegQueryValueExW(key, L"OledBlackScreen", NULL, &type, NULL, &size);
    RegCloseKey(key);
    return ret == ERROR_SUCCESS && type == REG_SZ && size > sizeof(wchar_t);
}
static void toggle_startup(void) {
    HKEY key;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, RUN_REG, 0, NULL, 0,
                        KEY_SET_VALUE, NULL, &key, NULL) != ERROR_SUCCESS) return;
    if (startup_enabled()) {
        RegDeleteValueW(key, L"OledBlackScreen");
    } else {
        wchar_t quoted[MAX_PATH + 4];
        wsprintfW(quoted, L"\"%s\"", exe_path);
        registry_string(key, L"OledBlackScreen", quoted);
    }
    RegCloseKey(key);
}
static void install_desktop_menu(void) {
    HKEY key, command;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, MENU_REG, 0, NULL, 0,
                        KEY_SET_VALUE | KEY_CREATE_SUB_KEY, NULL, &key, NULL) != ERROR_SUCCESS) return;
    wchar_t cmd[MAX_PATH + 30], ico[MAX_PATH + 12];
    wsprintfW(cmd, L"\"%s\" --black", exe_path);
    wsprintfW(ico, L"\"%s\",0", exe_path);
    registry_string(key, NULL, L"Black screen");
    registry_string(key, L"Icon", ico);
    registry_string(key, L"Position", L"Top");
    if (RegCreateKeyExW(key, L"command", 0, NULL, 0,
                        KEY_SET_VALUE, NULL, &command, NULL) == ERROR_SUCCESS) {
        registry_string(command, NULL, cmd);
        RegCloseKey(command);
    }
    RegCloseKey(key);
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
}
static void uninstall_desktop_menu(void) {
    RegDeleteTreeW(HKEY_CURRENT_USER, MENU_REG);
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
}
static void set_tray_icon(DWORD operation) {
    if (operation == NIM_ADD) {
        ZeroMemory(&nid, sizeof nid);
        nid.cbSize = sizeof nid;
        nid.hWnd = tray_window;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_TRAY;
        nid.hIcon = app_icon;
        lstrcpyW(nid.szTip, L"OLED Black Screen");
    }
    Shell_NotifyIconW(operation, &nid);
}
static void set_overlay_size(void) {
    if (overlay_window)
        SetWindowPos(overlay_window, HWND_TOPMOST,
                     GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN),
                     GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN),
                     SWP_SHOWWINDOW);
}
static LRESULT CALLBACK key_hook(int ncode, WPARAM action, LPARAM data) {
    (void)data;
    if (ncode >= 0 && overlay_window &&
        (action == WM_KEYDOWN || action == WM_SYSKEYDOWN)) {
        if (!closing) {
            closing = TRUE;
            PostMessageW(tray_window, WM_UNBLACK, 0, 0);
        }
        return 1;
    }
    return CallNextHookEx(keyboard_hook, ncode, action, data);
}
static LRESULT CALLBACK pointer_hook(int ncode, WPARAM action, LPARAM data) {
    if (ncode >= 0 && overlay_window &&
        (action == WM_LBUTTONDOWN || action == WM_RBUTTONDOWN ||
         action == WM_MBUTTONDOWN || action == WM_XBUTTONDOWN ||
         action == WM_MOUSEWHEEL || action == WM_MOUSEHWHEEL)) {
        if (!closing) {
            closing = TRUE;
            PostMessageW(tray_window, WM_UNBLACK, 0, 0);
        }
        return 1;
    }
    return CallNextHookEx(mouse_hook, ncode, action, data);
}
static void hide_black(void) {
    HWND old = overlay_window;
    if (!old) return;
    overlay_window = NULL;
    if (keyboard_hook) { UnhookWindowsHookEx(keyboard_hook); keyboard_hook = NULL; }
    if (mouse_hook) { UnhookWindowsHookEx(mouse_hook); mouse_hook = NULL; }
    DestroyWindow(old);
    while (cursor_calls > 0) {
        ShowCursor(TRUE);
        --cursor_calls;
    }
    SetThreadExecutionState(ES_CONTINUOUS);
    closing = FALSE;
}
static void show_black(void) {
    if (overlay_window) return;
    HINSTANCE instance = GetModuleHandleW(NULL);
    HWND window = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, OVERLAY_CLASS, L"",
                                  WS_POPUP, GetSystemMetrics(SM_XVIRTUALSCREEN),
                                  GetSystemMetrics(SM_YVIRTUALSCREEN),
                                  GetSystemMetrics(SM_CXVIRTUALSCREEN),
                                  GetSystemMetrics(SM_CYVIRTUALSCREEN),
                                  NULL, NULL, instance, NULL);
    if (!window) return;
    overlay_window = window;
    closing = FALSE;
    keyboard_hook = SetWindowsHookExW(WH_KEYBOARD_LL, key_hook, instance, 0);
    mouse_hook = SetWindowsHookExW(WH_MOUSE_LL, pointer_hook, instance, 0);
    // Never block input behind the overlay when the exit hooks could not be installed.
    if (!keyboard_hook || !mouse_hook) {
        hide_black();
        return;
    }
    SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
    int result = ShowCursor(FALSE);
    ++cursor_calls;
    while (result >= 0 && cursor_calls < 32) {
        result = ShowCursor(FALSE);
        ++cursor_calls;
    }
    set_overlay_size();
    ShowWindow(window, SW_SHOW);
    SetForegroundWindow(window);
    SetFocus(window);
}
static void open_tray_menu(void) {
    HMENU menu = CreatePopupMenu();
    if (!menu) return;
    AppendMenuW(menu, MF_STRING, MENU_BLACK, L"Black screen");
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING | (startup_enabled() ? MF_CHECKED : 0),
                MENU_STARTUP, L"Автозапуск с Windows");
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, MENU_QUIT, L"Выход");
    POINT pt;
    if (GetCursorPos(&pt)) {
        SetForegroundWindow(tray_window);
        TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_RIGHTALIGN | TPM_BOTTOMALIGN,
                       pt.x, pt.y, 0, tray_window, NULL);
        PostMessageW(tray_window, WM_NULL, 0, 0);
    }
    DestroyMenu(menu);
}
static LRESULT CALLBACK overlay_proc(HWND wnd, UINT message, WPARAM wp, LPARAM lp) {
    if (message == WM_SETCURSOR) { SetCursor(NULL); return TRUE; }
    if (message == WM_ERASEBKGND) return 1;
    if (message == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(wnd, &ps);
        RECT rect;
        GetClientRect(wnd, &rect);
        FillRect(dc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
        EndPaint(wnd, &ps);
        return 0;
    }
    if (message == WM_DISPLAYCHANGE) { set_overlay_size(); return 0; }
    if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN ||
        message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN ||
        message == WM_MBUTTONDOWN || message == WM_XBUTTONDOWN ||
        message == WM_MOUSEWHEEL || message == WM_MOUSEHWHEEL) {
        PostMessageW(tray_window, WM_UNBLACK, 0, 0);
        return 0;
    }
    return DefWindowProcW(wnd, message, wp, lp);
}
static LRESULT CALLBACK tray_proc(HWND wnd, UINT message, WPARAM wp, LPARAM lp) {
    if (message == taskbar_created && taskbar_created) {
        set_tray_icon(NIM_ADD);
        return 0;
    }
    switch (message) {
    case WM_BLACK: show_black(); return 0;
    case WM_UNBLACK: hide_black(); return 0;
    case WM_TRAY:
        if (lp == WM_LBUTTONUP || lp == WM_LBUTTONDBLCLK) show_black();
        else if (lp == WM_RBUTTONUP) open_tray_menu();
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case MENU_BLACK: show_black(); break;
        case MENU_STARTUP: toggle_startup(); break;
        case MENU_QUIT: DestroyWindow(wnd); break;
        }
        return 0;
    case WM_DESTROY:
        hide_black();
        set_tray_icon(NIM_DELETE);
        uninstall_desktop_menu();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(wnd, message, wp, lp);
}
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previous, PWSTR command, int show) {
    (void)previous; (void)show;
    if (wcsstr(command, L"--black")) {
        HWND existing = FindWindowW(TRAY_CLASS, NULL);
        if (existing) PostMessageW(existing, WM_BLACK, 0, 0);
        return 0;
    }
    singleton_mutex = CreateMutexW(NULL, TRUE, L"Local\\OledBlackScreenMutex.v1");
    if (!singleton_mutex || GetLastError() == ERROR_ALREADY_EXISTS) return 0;
    SetProcessDPIAware();
    DWORD length = GetModuleFileNameW(instance, exe_path, MAX_PATH);
    if (!length || length >= MAX_PATH) return 1;
    app_icon = (HICON)LoadImageW(instance, MAKEINTRESOURCEW(1), IMAGE_ICON,
                                32, 32, LR_DEFAULTCOLOR);
    if (!app_icon) app_icon = LoadIconW(NULL, IDI_APPLICATION);
    WNDCLASSEXW wc;
    ZeroMemory(&wc, sizeof wc);
    wc.cbSize = sizeof wc;
    wc.hInstance = instance;
    wc.lpfnWndProc = tray_proc;
    wc.lpszClassName = TRAY_CLASS;
    if (!RegisterClassExW(&wc)) return 1;
    wc.lpfnWndProc = overlay_proc;
    wc.lpszClassName = OVERLAY_CLASS;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    if (!RegisterClassExW(&wc)) return 1;
    taskbar_created = RegisterWindowMessageW(L"TaskbarCreated");
    tray_window = CreateWindowExW(0, TRAY_CLASS, L"OLED Black Screen", 0,
                                  CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
                                  NULL, NULL, instance, NULL);
    if (!tray_window) return 1;
    set_tray_icon(NIM_ADD);
    install_desktop_menu();
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    CloseHandle(singleton_mutex);
    return 0;
}
