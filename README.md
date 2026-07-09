# GSX Integrator Installer

Installs and updates [GSX Integrator](https://github.com/brunofgmag/gsx-integrator-client) and its [CommBus module](https://github.com/brunofgmag/gsx-integrator-commbus) for Microsoft Flight Simulator.

> **Note:** the CommBus module has only been tested on MSFS 2024. It may also work on MSFS 2020, but that has not been verified.

## How to use

1. Download `gsx-integrator-installer-<version>.zip` from the [latest release](https://github.com/brunofgmag/gsx-integrator-installer/releases) and extract it anywhere. The zip carries the app together with the Qt runtime it needs, so you don't have to install anything else. Each release also publishes a `.sha256` file if you want to verify the download.
2. Run `gsx-integrator-installer.exe` from the extracted `gsx-integrator-installer` folder. It does not ask for admin rights; everything installs per user.
3. The installer checks what you already have, compares it with the latest releases, and lists what it is going to do.
4. Click the main button (`Install everything`, `Update everything` or `Apply changes`). When it finishes, you can open GSX Integrator straight from the final screen.

Updating works the same way: open the installer again, and it offers whatever is newer. The installer also keeps itself up to date. When a newer installer exists, a banner appears at the top with an "Update and restart" button.

## What it installs

- The GSX Integrator client, into `%LOCALAPPDATA%\Programs\gsx-integrator-client`.
- The CommBus module, into the `Community` folder of every MSFS installation it detects (2020 and 2024, Steam and Microsoft Store).

## Options

- You can create a desktop shortcut and Start Menu entries for the client and for the installer.
- Auto-start launches GSX Integrator whenever the simulator starts. You can limit it to MSFS 2020 or 2024, enable it for both, or turn it off.
- Under advanced options you can choose which simulators receive the CommBus module, or force a full reinstall.

## Language

The interface is available in English and Português (BR). It follows your system language by default; use the picker in the window header to switch.

## Uninstalling

Click "Uninstall GSX Integrator" inside the installer, or use *Apps → Installed apps* in Windows Settings. Either way removes the client, the CommBus module from all simulators, and the auto-start entries.
