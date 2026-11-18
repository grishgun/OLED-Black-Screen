# Frequently asked questions — English

[Русский](FAQ.ru.md) | **English**

**Does the monitor turn off?** No. It remains powered on and receives a black image. This intentionally differs from display sleep.

**Is black really black?** The application paints solid RGB `(0, 0, 0)` via a Win32 black brush. The panel's actual behavior may depend on its firmware, calibration, HDR and signal processing.

**Can it prevent OLED burn-in completely?** No. It avoids leaving static Windows content on screen while active, but you should still follow the manufacturer's OLED care guidance.

**What exits black screen?** Any key, mouse button, or wheel event. Moving the pointer alone does not. A Ctrl+Alt+Del secure screen and other system dialogs are outside the overlay's control.

**Does it use a lot of resources?** There is no polling loop or periodic timer; the application waits for Win32 messages when idle. Exact RAM and CPU use has not been measured on a Windows test system.

**Why isn't the menu item at the top of the Windows 11 context menu?** The app registers a classic shell command under the current user's registry hive. Windows 11 normally puts classic commands under **Show more options**.

**What if the black screen becomes stuck?** First try a keyboard key or mouse button. If needed, use Windows system shortcuts such as Ctrl+Alt+Del to access Task Manager and terminate `OledBlackScreen.exe`. Force termination may leave the desktop menu item behind; see the removal instructions.

**Is it safe to use with multiple monitors?** The implementation spans the virtual desktop, but multi-monitor runtime testing is pending. Please report reproducible issues.

**Does it send any data?** No; the code contains no network or telemetry functionality.