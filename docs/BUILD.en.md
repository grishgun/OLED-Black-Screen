# Building OLED Black Screen — English

[Русский](BUILD.ru.md) | **English**

## Prerequisites

- Windows 11 x64 build environment.
- Visual Studio Build Tools with **MSVC v143 or newer** and a **Windows SDK**.
- An **x64 Native Tools Command Prompt for Visual Studio**.

## Compile

In the repository directory:

```cmd
Build-Windows.cmd
```

The script compiles `OledBlackScreen.c`, compiles `OledBlackScreen.rc` (embedding `icon.ico`), and links `OledBlackScreen.exe` for Windows x64.

The program uses standard Windows DLL imports, a custom entry point, and no C runtime dependency. The `/NODEFAULTLIB` option means adding C library functions in the future may require updating the build.

## CI / publishing a preview

The workflow is at `.github/workflows/release.yml`:

- Every push to `main` builds the app and uploads `OLED-Black-Screen-Windows-x64` under the **Actions** run.
- The first successful build on `main` automatically creates `v0.1.0-preview` (if not already present). For later releases, open **Actions → Windows build and release → Run workflow** and supply a version such as `v0.1.0-preview`.
- The workflow builds again and attaches `OledBlackScreen.exe` to a **prerelease**. It needs GitHub Actions enabled and `GITHUB_TOKEN` with `contents: write`.

Do not describe a CI build as a tested release: compiling successfully does not prove correctness at runtime.

## Sanity-check checklist (Windows)

1. Launch the app; verify the tray icon appears and the process consumes negligible CPU when idle.
2. Toggle autostart and verify it works after signing in again.
3. Invoke **Black screen** from both the tray and the classic desktop context menu.
4. Verify true black on every attached monitor, with HDR enabled and disabled if applicable.
5. Verify pointer movement alone does not dismiss, but a key, button or wheel does.
6. Check that keyboard and mouse work normally after closing the overlay.
7. Exit and verify that the tray icon and desktop menu item disappear.
8. Test recovery after Explorer restarts and after changing display configuration.

The project cannot guarantee behavior under elevated secure desktops, lock screens, exclusive full-screen apps, Remote Desktop, or multiple conflicting overlays.