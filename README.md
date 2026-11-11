# Power Poles

Automatic passive power relays in the normal Build menu.

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
