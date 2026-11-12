# Power Poles: behavior and implementation

Describes the retained payload **0.6.0**. The behavior prose describes the retained payload; current candidate source and metadata are identified below. Gameplay qualification is a separate requirement.


<!-- source-only-sync -->
## Current source candidate

Source version **0.6.1** is saved from workshop revision `ca32bbf79ce0840d5166a232bdb3a9fdcd761254`. The retained installable payload remains **0.6.0** with its original bytes. This source update does not rebuild or requalify that payload. The current source and metadata define the candidate; gameplay, rendered behavior and co-op remain unqualified for rebuilt bytes.

The authored behavior and lifecycle descriptions below are retained from the published payload. They are not new qualification evidence for the source candidate. The integration table below follows the current candidate metadata.
<!-- /source-only-sync -->

## Behavior

Despite its historical repository name, this version uses automatic native connections. With the hammer, choose Build > Power > Power Pole for $75. The provider registers saved kind 193 through Build Menu Framework and supplies its pole model and power-node metadata. Native 15 m range relays electricity; the pole generates and stores none. Moving/removing a pole uses native tools and the native graph rebuilds connections.

## State and lifecycle

Structures and power-node state follow native saves and require the provider/framework on reload. Definition/model registration happens at startup; framework callbacks own admitted scene updates and release. There is no separate manual edge editor or sidecar connection graph.

## Integration and configuration

Runtime dependencies: [build-menu-framework](https://github.com/swooping-mods/build-menu-framework).

The [module metadata](../source/mods/manual-power-poles/mod.json) owns the version and declared integration contract. Native modules start at `MoonLab_ModStart`. Game/Unity integrations target the pinned Steam Windows x64 IL2CPP build.

| Service | ABI | Table size |
|---|---:|---|
| Requires `build-menu-framework.build-menu` | 1 | at least 24 bytes |

## Implementation sources

- [mod.c](../source/mods/manual-power-poles/mod.c)
- [pole_model.h](../source/mods/manual-power-poles/pole_model.h)
- [shop_admission.h](../source/mods/manual-power-poles/shop_admission.h)
- [building.h](../source/mods/manual-power-poles/building.h)

## Verification boundary

Keep saved kind 193 stable. Cross-build solver compatibility, arbitrary graph cases and matching multiplayer worlds need qualification. The name does not imply a manual wiring control in this implementation.

`python3 build.py --check` verifies the exported source hashes. It does not execute the mod or qualify the behaviors above. Installation and independent build prerequisites are in the [README](../README.md).
