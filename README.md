# Power Poles

<img src="media/icon.png" alt="Power Poles icon" width="128" height="128">

Automatic passive power relays in the normal Build menu.


<!-- source-only-sync -->
## Current source candidate

Source version **0.6.1** is saved from workshop revision `ca32bbf79ce0840d5166a232bdb3a9fdcd761254`. The retained installable payload remains **0.6.0** with its original bytes. This source update does not rebuild or requalify that payload. The current source and metadata define the candidate; gameplay, rendered behavior and co-op remain unqualified for rebuilt bytes.
<!-- /source-only-sync -->

## Install

Install [Moon Sweepers Mod Loader](https://github.com/swooping-mods/moon-sweepers-mod-loader) first.

Download [manual-power-poles-0.6.0.msmod](content/manual-power-poles-0.6.0.msmod), then drag and drop the `.msmod` file onto the [Moon Sweepers Mod Loader](https://github.com/swooping-mods/moon-sweepers-mod-loader) window. You can also use **Add mods…** (older loader builds: **Install mod package**).

Select the mod and its required frameworks, click **Prepare files**, then **Launch**.

Required mods: [build-menu-framework](https://github.com/swooping-mods/build-menu-framework).

## Controls

- Equip the hammer (2). Build > Power > Power Pole ($75).
- Place poles near power equipment; connections form automatically.
- Poles relay power with native 15 m range and generate/store no electricity.
- Move or remove a pole normally. Connections rebuild automatically after changes and reload.

## Compatibility

Version 0.6.0. Targets the pinned Steam Windows x64 IL2CPP build of Moon Sweepers (Steam app 5335510). On Linux, the game runs through Proton. Experimental: back up saves before using save-affecting mods.

## Build

Run `python3 build.py --check` to verify source hashes, then `python3 build.py` to write a build under `.local/build/`. Requires installed Clang, lld-link and llvm-dlltool.

## Credits

See [CREDITS.txt](CREDITS.txt) and [third-party notices](vendored/THIRD_PARTY_NOTICES.md).
