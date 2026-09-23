# GSX Integrator Installer

Installs and updates [GSX Integrator](https://github.com/brunofgmag/gsx-integrator-client) and its [CommBus module](https://github.com/brunofgmag/gsx-integrator-commbus) for Microsoft Flight Simulator.

The CommBus module has only been tested on MSFS 2024. It may work on MSFS 2020, but nobody has checked.

## Using it

1. Download `gsx-integrator-installer-<version>.zip` from the [latest release](https://github.com/brunofgmag/gsx-integrator-installer/releases) and extract it anywhere. The zip includes the Qt runtime, so there is nothing else to install. Each release also has a `.sha256` file if you want to verify the download.
2. Run `gsx-integrator-installer.exe` from the extracted `gsx-integrator-installer` folder. It installs per user and doesn't ask for admin rights.
3. The installer looks at what you already have, compares it with the latest releases and lists what it will do.
4. Click the main button: Install everything, Update everything or Apply changes. When it finishes, the Open GSX Integrator button on the last screen starts the client.

To update, open the installer again and it offers whatever is newer. It also updates itself: when a newer installer is out, a banner at the top shows an Update and restart button.

## What goes where

- The client goes to `%LOCALAPPDATA%\Programs\gsx-integrator-client`.
- The CommBus module goes to the `Community` folder of every MSFS it finds: 2020 and 2024, Steam and Microsoft Store.

## Options

- Shortcuts: a desktop shortcut, and Start Menu entries for the client and for the installer.
- Auto-start: launch GSX Integrator when the simulator starts, for MSFS 2020, 2024 or both, or turn it off.
- Advanced options: pick which simulators get the CommBus module, or force a reinstall.

The interface is in English and Brazilian Portuguese. It follows the system language, and the picker in the window header switches it.

## Uninstalling

Click Uninstall GSX Integrator in the installer, or use Settings > Apps > Installed apps in Windows. Both remove the client, the CommBus module from every simulator, the shortcuts and the auto-start entries.
