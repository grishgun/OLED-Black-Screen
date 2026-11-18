# OLED Black Screen

**English** | [Русский](README.ru.md)

A tiny, native Windows utility that displays a solid, truly black (`#000000`) image across all connected monitors without turning the displays off. Designed for OLED users who want to leave the desk without showing a static desktop.

> **Preview release (v0.1.0-preview).** The source has been compiled into an x64 executable, but behavior has **not been validated in a live Windows session**. The executable is unsigned. Please test it before everyday use.

## Features

- Pure RGB `(0, 0, 0)` fullscreen overlay on all monitors.
- Left-click the tray icon to activate; right-click for startup and exit options.
- **Black screen** in the desktop's right-click menu while the application is running. Windows 11 displays this under **Show more options**.
- Dismiss with **any keyboard key, mouse button or wheel movement**. Merely moving the pointer does not dismiss it.
- Keeps the display active while the overlay is on using `SetThreadExecutionState`.
- Optional startup with Windows (current user only).
- No installer, no .NET, no network connection, no telemetry, and no periodic polling.
- Native Win32 implementation: hooks for keyboard/mouse are installed only while black screen is active.

## Requirements

- Windows 11, x64 (other Windows versions are not tested).
- Any connected monitor; optimized for the use case of OLED panels.
- A regular user account; no administrator rights required.

## Download and install

1. Open the repository's **Releases** page and download `OledBlackScreen.exe` from the latest preview release. If there is no release yet, use the **Actions** build artifact or [compile from source](docs/BUILD.en.md).
2. Move the EXE into a permanent directory, for example `C:\Tools\OLED-Black-Screen\`.
3. Run `OledBlackScreen.exe`. Its icon appears in the system tray (possibly behind the chevron).
4. Right-click the tray icon to enable **Автозапуск с Windows** (launch with Windows) if desired.

**Important:** do not move or rename the EXE while autostart is enabled. The registry entry points to its current location. Windows SmartScreen or antivirus software may show a warning for an unsigned new executable; independently review or build the source if you prefer.

## How to use

| Action | Result |
| --- | --- |
| Left-click tray icon | Show black screen |
| Right-click tray icon | Menu: Black screen, launch at startup, Exit |
| Right-click empty desktop space → Show more options → Black screen | Show black screen |
| Press any keyboard key | Exit black screen |
| Press any mouse button or scroll | Exit black screen |
| Move mouse pointer only | Stay on black screen |

The overlay covers the Windows *virtual desktop*, including multiple monitors. It does not change the monitor's brightness, color profile, HDR mode or power state.

## Remove the app

1. Right-click the tray icon and uncheck **Автозапуск с Windows** (if enabled).
2. Choose **Выход** (Exit). This removes the desktop context-menu integration.
3. Delete the EXE.

If Windows was shut down abruptly, or the app was force-terminated before cleanup, the context-menu item may remain. Remove it by running the following command in **Command Prompt**, as the same user who ran the app:

```cmd
reg delete "HKCU\Software\Classes\DesktopBackground\Shell\OledBlackScreen" /f
```

Optionally remove the startup entry:

```cmd
reg delete "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v OledBlackScreen /f
```

## OLED note

OLED pixels do not emit light when showing pure black, but this utility is **not** a substitute for automatic pixel refresh, panel care, screen saver, or turning off the display. Notifications, secure desktops, system dialogs, and other always-on-top software can sometimes appear above the overlay. The display stays powered on; this is intentional.

## Privacy and security

The application performs no network requests and does not collect or transmit user data. While the black screen is visible, it temporarily installs low-level input hooks to dismiss the overlay on key/button/wheel input. It does not store keystrokes or clicks.

The source is small enough to audit. For build instructions and technical details, see [Build](docs/BUILD.en.md), [FAQ](docs/FAQ.en.md) and [Security](SECURITY.md).

## Development

The implementation is in [OledBlackScreen.c](OledBlackScreen.c), with an icon resource and the [Build-Windows.cmd](Build-Windows.cmd) script. GitHub Actions compiles the executable on a Windows runner. Every push to `main` produces an Actions artifact; the first successful main-branch build creates an unsigned v0.1.0-preview release; later releases can be dispatched manually.

## License

[MIT License](LICENSE). Contributions and bug reports are welcome.