# Changelog

## [0.5.4](https://github.com/brunofgmag/gsx-integrator-installer/compare/v0.5.3...v0.5.4) (2026-10-18)


* give the client the session display when the watcher started first ([#57](https://github.com/brunofgmag/gsx-integrator-installer/issues/57)) ([da4d6a5](https://github.com/brunofgmag/gsx-integrator-installer/commit/da4d6a5ff15f1e686337365bd9cd12adeefd2859))


### Bug Fixes

* give the client the session display when the watcher started first ([7fe319c](https://github.com/brunofgmag/gsx-integrator-installer/commit/7fe319ccb6ea2e01b6fe147376b228683e52143c)) ([da4d6a5](https://github.com/brunofgmag/gsx-integrator-installer/commit/da4d6a5ff15f1e686337365bd9cd12adeefd2859))

## [0.5.3](https://github.com/brunofgmag/gsx-integrator-installer/compare/v0.5.2...v0.5.3) (2026-10-17)


* scan the process list with one grep per watcher poll ([#54](https://github.com/brunofgmag/gsx-integrator-installer/issues/54)) ([2cc8c04](https://github.com/brunofgmag/gsx-integrator-installer/commit/2cc8c046f6fd4e992d5be10948e17a959ad0fcaa))


### Performance Improvements

* scan the process list with one grep per watcher poll ([7caa245](https://github.com/brunofgmag/gsx-integrator-installer/commit/7caa2457790ccddc61ca5effa4096039d0d261fe)) ([2cc8c04](https://github.com/brunofgmag/gsx-integrator-installer/commit/2cc8c046f6fd4e992d5be10948e17a959ad0fcaa))

## [0.5.2](https://github.com/brunofgmag/gsx-integrator-installer/compare/v0.5.1...v0.5.2) (2026-10-17)


* restart the simulator watcher when its script changes ([#51](https://github.com/brunofgmag/gsx-integrator-installer/issues/51)) ([9f5876a](https://github.com/brunofgmag/gsx-integrator-installer/commit/9f5876afa8003a966cd16bb17f775de147af1582))


### Bug Fixes

* restart the simulator watcher when its script changes ([42e0e62](https://github.com/brunofgmag/gsx-integrator-installer/commit/42e0e6245fb0c563fe5cdef903bf48f3050dfb8b)) ([9f5876a](https://github.com/brunofgmag/gsx-integrator-installer/commit/9f5876afa8003a966cd16bb17f775de147af1582))

## [0.5.1](https://github.com/brunofgmag/gsx-integrator-installer/compare/v0.5.0...v0.5.1) (2026-10-17)


* keep the simulator watcher alive when a process exits mid-read ([#48](https://github.com/brunofgmag/gsx-integrator-installer/issues/48)) ([cf66760](https://github.com/brunofgmag/gsx-integrator-installer/commit/cf66760d1f8000a42e50127a657a501c6142fe49))


### Bug Fixes

* keep the simulator watcher alive when a process exits mid-read ([c597d75](https://github.com/brunofgmag/gsx-integrator-installer/commit/c597d75315b01441cd0ddcef9b1ba447adbe3d2d)) ([cf66760](https://github.com/brunofgmag/gsx-integrator-installer/commit/cf66760d1f8000a42e50127a657a501c6142fe49))

## [0.5.0](https://github.com/brunofgmag/gsx-integrator-installer/compare/v0.4.0...v0.5.0) (2026-10-16)


* run the installer natively on linux ([#44](https://github.com/brunofgmag/gsx-integrator-installer/issues/44)) ([f6d90c7](https://github.com/brunofgmag/gsx-integrator-installer/commit/f6d90c75d7f09391e394efb1a35b52a8e472064e))


### Features

* confirm a Linux --uninstall in a window when there is no terminal ([6a78fbc](https://github.com/brunofgmag/gsx-integrator-installer/commit/6a78fbc4104f8414a999604a551249b87307d002)) ([f6d90c7](https://github.com/brunofgmag/gsx-integrator-installer/commit/f6d90c75d7f09391e394efb1a35b52a8e472064e))
* install the client as an AppImage with desktop entries and a systemd watcher on Linux ([40f177c](https://github.com/brunofgmag/gsx-integrator-installer/commit/40f177ca68addf3416890d8d27f01b13ce12f536)) ([f6d90c7](https://github.com/brunofgmag/gsx-integrator-installer/commit/f6d90c75d7f09391e394efb1a35b52a8e472064e))
* resolve Proton prefixes and detect simulators and processes on Linux ([1276fa1](https://github.com/brunofgmag/gsx-integrator-installer/commit/1276fa1acda26be6ac9f0e72daa077869cf22952)) ([f6d90c7](https://github.com/brunofgmag/gsx-integrator-installer/commit/f6d90c75d7f09391e394efb1a35b52a8e472064e))
* run on Linux with its own menu texts, auto-start notice and uninstall command ([de4b537](https://github.com/brunofgmag/gsx-integrator-installer/commit/de4b53771ac406ab197e9cbf1d2445005b252202)) ([f6d90c7](https://github.com/brunofgmag/gsx-integrator-installer/commit/f6d90c75d7f09391e394efb1a35b52a8e472064e))
* update the installer on Linux by replacing its AppImage and pick the AppImage release assets ([eae1c96](https://github.com/brunofgmag/gsx-integrator-installer/commit/eae1c969c6d31a6ef6d5092eed104a67450221c4)) ([f6d90c7](https://github.com/brunofgmag/gsx-integrator-installer/commit/f6d90c75d7f09391e394efb1a35b52a8e472064e))


### Bug Fixes

* keep the terminal hint and exit 2 for a Linux --uninstall with no terminal and no display ([491636b](https://github.com/brunofgmag/gsx-integrator-installer/commit/491636b5c0231f55fdb0ee3051a052d722236924)) ([f6d90c7](https://github.com/brunofgmag/gsx-integrator-installer/commit/f6d90c75d7f09391e394efb1a35b52a8e472064e))
* **linux:** bundle the wayland shell plugins in the appimage ([4295d54](https://github.com/brunofgmag/gsx-integrator-installer/commit/4295d54c3ea11ea6b3766bf70d50155b1a5cfe78)) ([f6d90c7](https://github.com/brunofgmag/gsx-integrator-installer/commit/f6d90c75d7f09391e394efb1a35b52a8e472064e))
* **linux:** give the appimage window its icon on wayland ([0315888](https://github.com/brunofgmag/gsx-integrator-installer/commit/0315888818925f569be2549c0a822fd0116b91bf)) ([f6d90c7](https://github.com/brunofgmag/gsx-integrator-installer/commit/f6d90c75d7f09391e394efb1a35b52a8e472064e))
* match the client by its program name instead of any command-line argument ([65a02ad](https://github.com/brunofgmag/gsx-integrator-installer/commit/65a02ad86842c3ac75e3119060ec7cbbcad6cb2f)) ([f6d90c7](https://github.com/brunofgmag/gsx-integrator-installer/commit/f6d90c75d7f09391e394efb1a35b52a8e472064e))
* read the executable name from Windows paths whatever the host separator is ([c718caf](https://github.com/brunofgmag/gsx-integrator-installer/commit/c718caf76d09d9ec99be07e46f9542f7774e107e)) ([f6d90c7](https://github.com/brunofgmag/gsx-integrator-installer/commit/f6d90c75d7f09391e394efb1a35b52a8e472064e))
* refuse to uninstall while a simulator is open and report what could not be removed ([7df9e5f](https://github.com/brunofgmag/gsx-integrator-installer/commit/7df9e5fe9151bcac9f7f6dba21e62e167286fed5)) ([f6d90c7](https://github.com/brunofgmag/gsx-integrator-installer/commit/f6d90c75d7f09391e394efb1a35b52a8e472064e))
* report a client folder that could not be removed as an uninstall leftover ([69ed25e](https://github.com/brunofgmag/gsx-integrator-installer/commit/69ed25e2a0c6dbf09a01daeaeb793bd541f89239)) ([f6d90c7](https://github.com/brunofgmag/gsx-integrator-installer/commit/f6d90c75d7f09391e394efb1a35b52a8e472064e))
* show the reason when the installer update fails to unpack, prepare or start ([2885f63](https://github.com/brunofgmag/gsx-integrator-installer/commit/2885f631a91267f1e6f5357c7fba3dd6ac333e77)) ([f6d90c7](https://github.com/brunofgmag/gsx-integrator-installer/commit/f6d90c75d7f09391e394efb1a35b52a8e472064e))

## [0.4.0](https://github.com/brunofgmag/gsx-integrator-installer/compare/v0.3.2...v0.4.0) (2026-09-26)


* correct the user-facing texts and keep translation in the viewmodel ([#40](https://github.com/brunofgmag/gsx-integrator-installer/issues/40)) ([c674612](https://github.com/brunofgmag/gsx-integrator-installer/commit/c674612a2c113f47f7a9126b33de382b82d63678))
* remove only the junction when replacing the commbus package ([#34](https://github.com/brunofgmag/gsx-integrator-installer/issues/34)) ([c0520a9](https://github.com/brunofgmag/gsx-integrator-installer/commit/c0520a9c931f0b34744866f8d316f0d2454be682))


### Features

* draw the installer window with the software backend ([#25](https://github.com/brunofgmag/gsx-integrator-installer/issues/25)) ([03d058c](https://github.com/brunofgmag/gsx-integrator-installer/commit/03d058ce47284a8446ce7c5d0d4be6ba89074096))
* stamp the installed commbus version into the package ([0b864bb](https://github.com/brunofgmag/gsx-integrator-installer/commit/0b864bb29ba7e12a7589aa19a6bc61d03f6a2687))


### Bug Fixes

* allow only one instance of the installer ([02ce4a0](https://github.com/brunofgmag/gsx-integrator-installer/commit/02ce4a02efbc26f74bd974e9c4b5312bc2e51fe6))
* allow only one instance of the installer ([#13](https://github.com/brunofgmag/gsx-integrator-installer/issues/13)) ([8bd896a](https://github.com/brunofgmag/gsx-integrator-installer/commit/8bd896a4dc9cfe9477a530941e46f96fd7c4d1ff))
* auto update not working ([#10](https://github.com/brunofgmag/gsx-integrator-installer/issues/10)) ([29d92e6](https://github.com/brunofgmag/gsx-integrator-installer/commit/29d92e6e400781b7068298a107ecc56b26a91898))
* client update ([#3](https://github.com/brunofgmag/gsx-integrator-installer/issues/3)) ([20bcb41](https://github.com/brunofgmag/gsx-integrator-installer/commit/20bcb41eb202f32c9b82dbb46662e0622f57501e))
* correct the user-facing texts and keep translation in the viewmodel ([2f8923f](https://github.com/brunofgmag/gsx-integrator-installer/commit/2f8923fcb5be17835c8bdea81c080de38e5958e6)) ([c674612](https://github.com/brunofgmag/gsx-integrator-installer/commit/c674612a2c113f47f7a9126b33de382b82d63678))
* install client with tray param ([#20](https://github.com/brunofgmag/gsx-integrator-installer/issues/20)) ([a054475](https://github.com/brunofgmag/gsx-integrator-installer/commit/a0544757605f8901b9359d2d91ad4136d6ee3d56))
* installer auto-update hang ([#16](https://github.com/brunofgmag/gsx-integrator-installer/issues/16)) ([11e2754](https://github.com/brunofgmag/gsx-integrator-installer/commit/11e27541c44ed1308decfd541c97f98863d7fb6b))
* remove only the junction when replacing the commbus package ([50a0273](https://github.com/brunofgmag/gsx-integrator-installer/commit/50a02730821da134c43bdce468c53e7065bd9923)) ([c0520a9](https://github.com/brunofgmag/gsx-integrator-installer/commit/c0520a9c931f0b34744866f8d316f0d2454be682))
* simplify the user-facing texts ([eb90a38](https://github.com/brunofgmag/gsx-integrator-installer/commit/eb90a382dc020f87b8c34df5e434ec2127e43b0c))

## [0.3.2](https://github.com/brunofgmag/gsx-integrator-installer/compare/v0.3.1...v0.3.2) (2026-09-23)


### Bug Fixes

* simplify the user-facing texts ([eb90a38](https://github.com/brunofgmag/gsx-integrator-installer/commit/eb90a382dc020f87b8c34df5e434ec2127e43b0c))

## [0.3.1](https://github.com/brunofgmag/gsx-integrator-installer/compare/v0.3.0...v0.3.1) (2026-08-29)


* remove only the junction when replacing the commbus package ([#34](https://github.com/brunofgmag/gsx-integrator-installer/issues/34)) ([c0520a9](https://github.com/brunofgmag/gsx-integrator-installer/commit/c0520a9c931f0b34744866f8d316f0d2454be682))


### Bug Fixes

* remove only the junction when replacing the commbus package ([50a0273](https://github.com/brunofgmag/gsx-integrator-installer/commit/50a02730821da134c43bdce468c53e7065bd9923)) ([c0520a9](https://github.com/brunofgmag/gsx-integrator-installer/commit/c0520a9c931f0b34744866f8d316f0d2454be682))

## [0.3.0](https://github.com/brunofgmag/gsx-integrator-installer/compare/v0.2.0...v0.3.0) (2026-08-23)


### Features

* stamp the installed commbus version into the package ([0b864bb](https://github.com/brunofgmag/gsx-integrator-installer/commit/0b864bb29ba7e12a7589aa19a6bc61d03f6a2687))

## [0.2.0](https://github.com/brunofgmag/gsx-integrator-installer/compare/v0.1.8...v0.2.0) (2026-08-17)


### Features

* draw the installer window with the software backend ([#25](https://github.com/brunofgmag/gsx-integrator-installer/issues/25)) ([03d058c](https://github.com/brunofgmag/gsx-integrator-installer/commit/03d058ce47284a8446ce7c5d0d4be6ba89074096))

## [0.1.8](https://github.com/brunofgmag/gsx-integrator-installer/compare/v0.1.7...v0.1.8) (2026-07-12)


### Bug Fixes

* install client with tray param ([#20](https://github.com/brunofgmag/gsx-integrator-installer/issues/20)) ([a054475](https://github.com/brunofgmag/gsx-integrator-installer/commit/a0544757605f8901b9359d2d91ad4136d6ee3d56))

## [0.1.7](https://github.com/brunofgmag/gsx-integrator-installer/compare/v0.1.6...v0.1.7) (2026-07-10)


### Bug Fixes

* allow only one instance of the installer ([02ce4a0](https://github.com/brunofgmag/gsx-integrator-installer/commit/02ce4a02efbc26f74bd974e9c4b5312bc2e51fe6))
* allow only one instance of the installer ([#13](https://github.com/brunofgmag/gsx-integrator-installer/issues/13)) ([8bd896a](https://github.com/brunofgmag/gsx-integrator-installer/commit/8bd896a4dc9cfe9477a530941e46f96fd7c4d1ff))
* auto update not working ([#10](https://github.com/brunofgmag/gsx-integrator-installer/issues/10)) ([29d92e6](https://github.com/brunofgmag/gsx-integrator-installer/commit/29d92e6e400781b7068298a107ecc56b26a91898))
* client update ([#3](https://github.com/brunofgmag/gsx-integrator-installer/issues/3)) ([20bcb41](https://github.com/brunofgmag/gsx-integrator-installer/commit/20bcb41eb202f32c9b82dbb46662e0622f57501e))
* installer auto-update hang ([#16](https://github.com/brunofgmag/gsx-integrator-installer/issues/16)) ([11e2754](https://github.com/brunofgmag/gsx-integrator-installer/commit/11e27541c44ed1308decfd541c97f98863d7fb6b))

## [0.1.6](https://github.com/brunofgmag/gsx-integrator-installer/compare/v0.1.5...v0.1.6) (2026-07-10)


### Bug Fixes

* allow only one instance of the installer ([02ce4a0](https://github.com/brunofgmag/gsx-integrator-installer/commit/02ce4a02efbc26f74bd974e9c4b5312bc2e51fe6))
* allow only one instance of the installer ([#13](https://github.com/brunofgmag/gsx-integrator-installer/issues/13)) ([8bd896a](https://github.com/brunofgmag/gsx-integrator-installer/commit/8bd896a4dc9cfe9477a530941e46f96fd7c4d1ff))
* auto update not working ([#10](https://github.com/brunofgmag/gsx-integrator-installer/issues/10)) ([29d92e6](https://github.com/brunofgmag/gsx-integrator-installer/commit/29d92e6e400781b7068298a107ecc56b26a91898))
* client update ([#3](https://github.com/brunofgmag/gsx-integrator-installer/issues/3)) ([20bcb41](https://github.com/brunofgmag/gsx-integrator-installer/commit/20bcb41eb202f32c9b82dbb46662e0622f57501e))
* installer auto-update hang ([#16](https://github.com/brunofgmag/gsx-integrator-installer/issues/16)) ([11e2754](https://github.com/brunofgmag/gsx-integrator-installer/commit/11e27541c44ed1308decfd541c97f98863d7fb6b))

## [0.1.5](https://github.com/brunofgmag/gsx-integrator-installer/compare/v0.1.4...v0.1.5) (2026-07-10)


### Bug Fixes

* allow only one instance of the installer ([02ce4a0](https://github.com/brunofgmag/gsx-integrator-installer/commit/02ce4a02efbc26f74bd974e9c4b5312bc2e51fe6))
* allow only one instance of the installer ([#13](https://github.com/brunofgmag/gsx-integrator-installer/issues/13)) ([8bd896a](https://github.com/brunofgmag/gsx-integrator-installer/commit/8bd896a4dc9cfe9477a530941e46f96fd7c4d1ff))
* auto update not working ([#10](https://github.com/brunofgmag/gsx-integrator-installer/issues/10)) ([29d92e6](https://github.com/brunofgmag/gsx-integrator-installer/commit/29d92e6e400781b7068298a107ecc56b26a91898))
* client update ([#3](https://github.com/brunofgmag/gsx-integrator-installer/issues/3)) ([20bcb41](https://github.com/brunofgmag/gsx-integrator-installer/commit/20bcb41eb202f32c9b82dbb46662e0622f57501e))

## [0.1.4](https://github.com/brunofgmag/gsx-integrator-installer/compare/v0.1.3...v0.1.4) (2026-07-09)


### Bug Fixes

* auto update not working ([#10](https://github.com/brunofgmag/gsx-integrator-installer/issues/10)) ([29d92e6](https://github.com/brunofgmag/gsx-integrator-installer/commit/29d92e6e400781b7068298a107ecc56b26a91898))
* client update ([#3](https://github.com/brunofgmag/gsx-integrator-installer/issues/3)) ([20bcb41](https://github.com/brunofgmag/gsx-integrator-installer/commit/20bcb41eb202f32c9b82dbb46662e0622f57501e))

## [0.1.3](https://github.com/brunofgmag/gsx-integrator-installer/compare/v0.1.2...v0.1.3) (2026-07-09)


### Bug Fixes

* client update ([#3](https://github.com/brunofgmag/gsx-integrator-installer/issues/3)) ([20bcb41](https://github.com/brunofgmag/gsx-integrator-installer/commit/20bcb41eb202f32c9b82dbb46662e0622f57501e))

## [0.1.2](https://github.com/brunofgmag/gsx-integrator-installer/compare/v0.1.1...v0.1.2) (2026-07-09)


### Bug Fixes

* client update ([#3](https://github.com/brunofgmag/gsx-integrator-installer/issues/3)) ([20bcb41](https://github.com/brunofgmag/gsx-integrator-installer/commit/20bcb41eb202f32c9b82dbb46662e0622f57501e))

## [0.1.1](https://github.com/brunofgmag/gsx-integrator-installer/compare/v0.1.0...v0.1.1) (2026-07-09)


### Bug Fixes

* client update ([#3](https://github.com/brunofgmag/gsx-integrator-installer/issues/3)) ([20bcb41](https://github.com/brunofgmag/gsx-integrator-installer/commit/20bcb41eb202f32c9b82dbb46662e0622f57501e))
