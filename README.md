# Veezy Stats

![Veezy Stats Screenshot](placeholder_![ss](https://github.com/user-attachments/assets/1cb4d711-eda7-42bd-8e64-13b7d69b5a85)
screenshot.png) <!-- <<< Replace with a link to an actual screenshot! -->

A minimalist, lightweight system performance overlay widget for Windows, built with C++, DirectX 11, and Dear ImGui.

## Overview

Veezy Stats provides a simple, always-on-top, semi-transparent overlay displaying key system metrics:

*   **CPU Usage:** Real-time total processor utilization.
*   **RAM Usage:** Current memory load percentage and used/total MB.
*   **Disk Usage (C:):** Usage percentage and free space in GB for the C: drive.
*   **(Placeholder) GPU Info:** Includes fields for GPU usage and temperature (requires vendor SDKs for actual data).

The widget uses a click-through window (`WS_EX_TRANSPARENT`), so it doesn't interfere with mouse interactions with underlying applications. It leverages DirectX 11 for efficient rendering and Dear ImGui for the user interface elements.

## Features

*   Real-time monitoring of CPU, RAM, and C: Drive Disk usage.
*   Minimalist, clean user interface using Dear ImGui.
*   Configurable styling (dark theme, padding, rounding).
*   Always-on-top display to stay visible over other windows.
*   Click-through transparent window.
*   Low resource footprint.
*   Built with modern C++ and standard Windows APIs.

## Technology Stack

*   **Language:** C++
*   **Platform:** Windows (Win32 API)
*   **Rendering:** Direct3D 11
*   **UI:** Dear ImGui (with Win32 + DirectX 11 backends)
*   **System Info:**
    *   Windows API (`GlobalMemoryStatusEx`, `GetDiskFreeSpaceExW`)
    *   Performance Data Helper (PDH) library (`PdhOpenQuery`, etc. for CPU usage)

## Requirements

*   Windows 10 or later (May work on older versions, but untested).
*   A C++ compiler supporting C++11/14/17 (e.g., Visual Studio 2017 or later).
*   Windows SDK (usually included with Visual Studio installation).
*   DirectX SDK components (often part of the Windows SDK) or DirectX End-User Runtimes installed.
*   **Dear ImGui Source Code:** You need to have the Dear ImGui source files (including `backends/imgui_impl_win32.*` and `backends/imgui_impl_dx11.*`) available. The project expects them to be in an `imgui` subdirectory relative to `main.cpp`, or you'll need to adjust the `#include` paths.
*   **(Optional Font):** If using a version with custom fonts, place the required `.ttf` font file (e.g., `Roboto-Regular.ttf`) where the application can find it (like the same directory as the executable) and ensure the path constant in the code is correct.

## Building

The easiest way to build is typically using Visual Studio:

1.  **Clone/Download:** Get the project source code.
2.  **Get ImGui:** Download the Dear ImGui source code and place its contents (including the `backends` folder) into an `imgui` subdirectory within the project, or adjust include paths in `main.cpp`.
3.  **Open Project:** Open the source file (`main.cpp`) or create a new Visual Studio project and add the file to it.
4.  **Configure Linker:** Ensure your project links against the necessary libraries:
    *   `d3d11.lib`
    *   `dxgi.lib`
    *   `pdh.lib`
    (In Visual Studio: Project Properties -> Linker -> Input -> Additional Dependencies)
5.  **Build:** Build the solution (e.g., press F7 or go to Build > Build Solution). An executable (e.g., `VeezyStats.exe`) should be generated in the output directory (e.g., `Debug` or `Release`).

## Running

Simply double-click the compiled `.exe` file. The widget should appear on your screen (by default, near the top-right corner). To close it, you'll need to end the process using Task Manager.

## Customization (Source Code)

You can modify the appearance and behavior by editing `main.cpp`:

*   **Position & Size:** Adjust the `WINDOW_WIDTH`, `WINDOW_HEIGHT`, `windowX`, and `windowY` values in `WinMain`.
*   **Update Speed:** Change the `UPDATE_INTERVAL` constant.
*   **Styling:** Modify the `ImGuiStyle& style = ImGui::GetStyle();` section (or the `ApplyCustomStyle()` function if present) to change colors, padding, rounding, etc.
*   **Monitored Drive:** Change the drive letter passed to `GetDiskInfo` (e.g., `L"D:\\"`).
*   **Font:** (If applicable) Change the `FONT_PATH` and `FONT_SIZE` constants.

## Limitations & Known Issues

*   **GPU Monitoring:** GPU usage and temperature display are **placeholders**. Getting this information reliably requires vendor-specific libraries (like NVIDIA's NVAPI or AMD's AGS) or potentially complex and less reliable WMI queries. This is not implemented.
*   **CPU Counter:** Uses `PdhAddEnglishCounter`. This relies on English performance counter names (`\Processor(_Total)\% Processor Time`). It *might* fail on non-English versions of Windows if the system doesn't have English counters available or if the fallback counter path isn't correct for that specific language version.
*   **Configuration:** Settings (like position, style) are hardcoded. Changes require recompiling the application.
*   **Single Drive:** Only monitors the specified drive (default C:).

## License

This project is licensed under the [MIT License](LICENSE.md). <!-- <<< You should create a LICENSE.md file with the MIT license text -->

## Contributing

Contributions, issues, and feature requests are welcome. Feel free to check [issues page](link/to/your/issues) if you want to contribute. <!-- <<< Update link if you host this on GitHub/GitLab etc. -->
