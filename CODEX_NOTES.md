# BallDarkEcho Design Notes

## Project Overview

- Project name: BallDarkEcho.
- Engine: Unreal Engine 5.7.
- Genre target: limited-vision multiplayer echo combat.
- First playable demo target: 4-8 player LAN / Listen Server deathmatch.
- Core fantasy: players fight in a dark arena where sound is both the main information source and the main exposure risk.
- Current avatar direction: keep the simple ball / third-person pawn for the first demo. Later skeletal meshes and animation will replace the placeholder visual without rewriting gameplay systems.
- Main design pillar: every sound-producing action should create a readable tradeoff between information gain, movement/combat pressure, and reveal risk.

This document is the current authoritative design and development note. It replaces older prototype-era assumptions.

## Current Implementation Status

Status updated 2026-06-21.

### Current Player Pawn

- Current player pawn: `BP_ThirdPersonCharacter`.
- The project has moved away from using `BP_EchoPulse` as the main demo pawn.
- `BP_ThirdPersonCharacter` currently owns the main gameplay components:
  - `EchoCombat`
  - `EchoWeapon`
  - `EchoCharacterState`
  - `EchoMovementPulse`
  - `EchoWaveEmitter`
  - `EchoPulseScanner`
  - `EchoExplorationMap`
  - `EchoGameplay`
  - `EchoHUDPresenter`

### Multiplayer Core

Implemented:

- LAN / Listen Server deathmatch foundation.
- `EchoDeathmatchGameMode`
  - Owns kill target logic.
  - Handles kill credit.
  - Handles respawn flow.
- `EchoDeathmatchGameState`
  - Replicates kill target and match result.
  - Tracks winner state.
- `EchoPlayerState`
  - Replicates kills.
  - Replicates deaths.
  - Replicates player number.
  - Replicates player color identity.
- `EchoCombatComponent`
  - Replicated health.
  - Damage receiving through `ReceiveEchoDamage`.
  - Death state.
  - Respawn state.
  - HUD events for health, death, score, and result.

Current multiplayer test setup:

- `DM_Tian` exists as the simple four-room square test map.
- GameMode override is set to `BP_EchoDeathmatchGameMode`.
- Default Pawn is `BP_ThirdPersonCharacter`.
- Main listen-server tests have verified server/client damage, death, respawn, kills, and deaths.

### Weapons and Projectiles

Implemented:

- `EEchoWeaponMode`
  - `Standard`
  - `RapidCloseRange`
  - `LongRangeSnipe`
- `FEchoWeaponTuning`
  - Damage.
  - Projectile speed.
  - Max range.
  - Cooldown.
  - Full damage range.
  - Minimum damage multiplier.
  - Exposure radius.
- `EchoWeaponComponent`
  - Replicated current weapon mode.
  - `Q` cycling support.
  - `1/2/3` direct weapon selection support.
  - Server-authoritative firing.
  - Third-person camera aim trace.
  - Projectile spawns from a character-side muzzle point instead of the camera.
  - Muzzle offset tuning for shoulder-style third-person shooting.
- `EchoSoundProjectileActor`
  - Replicated projectile movement.
  - Server-authoritative hit detection.
  - Damage falloff.
  - Ignores the source actor.
  - Calls `EchoCombatComponent::ReceiveEchoDamage` on hit.
  - Destroys on hit or max range.

Current weapon defaults:

- `Standard`
  - Damage 35.
  - Speed 1800.
  - Range 1800.
  - Cooldown 0.6.
  - General-purpose default weapon.
- `RapidCloseRange`
  - Damage 22.
  - Speed 2200.
  - Range 1000.
  - Cooldown 0.18.
  - Full damage range 450.
  - Minimum damage multiplier 0.35.
  - Fast close-range pressure.
- `LongRangeSnipe`
  - Damage 85 by C++ default, currently tunable in Blueprint.
  - Speed 4200.
  - Range 3600.
  - Cooldown 1.6.
  - Minimum damage multiplier 0.8.
  - High-risk long-range shot.

### Third-Person Shooting Feel

Implemented:

- Camera aim trace finds the crosshair target point.
- Projectile spawns from character-side muzzle offset.
- Projectile direction is calculated from muzzle to aim target.
- CameraBoom has been tuned toward a shoulder-view setup in Blueprint.
- Current useful camera offset direction:
  - Socket Offset roughly `Y = 40`, `Z = 20`.
  - Target Offset roughly `Y = 20`, `Z = 30`.
- This reduced the old issue where projectiles spawned from the camera, passed through the player, and felt wrong in third person.

### HUD

Implemented:

- `EchoHUDWidget` is the C++ parent for the gameplay HUD.
- `WBP_EchoHUD` exists under `Content/UI`.
- `EchoHUDPresenterComponent` creates the HUD and crosshair.
- C++ HUD event hooks exist for:
  - `UpdateCombatHealth`
  - `UpdateDeathState`
  - `UpdateDeathmatchScore`
  - `ShowDeathmatchResult`
  - `UpdateWeaponMode`
- HUD currently supports:
  - Health text.
  - Health progress bar.
  - Kills / deaths / target score.
  - Current weapon text.
  - State panel placeholder.
- C++ crosshair widget exists and is displayed separately from the Blueprint HUD.

### Main Menu and LAN Lobby

Implemented first pass:

- `EchoGameInstance` owns LAN session create/search/join flow.
- `EchoLobbyGameMode` and `EchoLobbyGameState` own room settings, host authority, ready state, max players, and match start travel.
- `EchoLobbyPlayerController` exposes server RPCs for player ID, ready toggle, map selection, player count, and start.
- `EchoPlayerState` now also replicates lobby identity, host state, and ready state.
- `EchoMainMenuWidget` supports login, room list, room creation, in-room lobby, ready/start controls, map selection, max player adjustment, and settings placeholders.
- `EchoRoomRowWidget` and `EchoLobbyPlayerRowWidget` exist for UMG room/player rows.
- `MenuLobby` is set as the current startup/default map in config.

Needs verification:

- Full host/client LAN flow in PIE or multiple standalone windows.
- WBP widget naming and parent classes must match `Docs/MenuUMGSetup.zh.md`.
- Start travel from lobby into the selected deathmatch map needs a multiplayer regression test.

### Movement State and Movement Sound

Implemented:

- `EEchoCharacterAnimState`
  - `Idle`
  - `NormalMove`
  - `Sprint`
  - `SilentWalk`
  - `Jumping`
  - `Firing`
  - `Dead`
- `EchoCharacterStateComponent`
  - Replicates sprint state.
  - Replicates silent-walk state.
  - Replicates dead state.
  - Tracks recent firing.
  - Calculates animation/gameplay state.
  - Provides future animation blueprint state data.
- Blueprint input wiring:
  - `Shift` sets sprint state.
  - `Ctrl` sets silent-walk state.
  - Blueprint also adjusts CharacterMovement max walk speed:
    - Normal about 600.
    - Sprint about 900.
    - Silent walk about 250.

Implemented movement wave behavior:

- Movement start immediately emits a movement wave.
- Continuous movement emits a new wave about every 2 seconds.
- Sprint emits larger, brighter, wider waves more often.
- Silent walk emits smaller, weaker, shorter-range waves less often.
- Switching sprint/silent state while already moving immediately emits a new state-specific wave.

Current movement wave defaults:

- Normal:
  - Speed 2000.
  - Width 70.
  - Intensity 10.
  - Duration 2.0.
  - Max radius 2000.
- Sprint:
  - Interval 1.2.
  - Speed 2200.
  - Width 90.
  - Intensity 14.
  - Duration 2.0.
  - Max radius 2600.
- Silent walk:
  - Interval 2.5.
  - Speed 1600.
  - Width 45.
  - Intensity 4.
  - Duration 1.5.
  - Max radius 800.

### Echo Wave System

Implemented:

- `EchoWaveEmitterComponent` writes echo wave data into `MPC_EchoWaves`.
- Slot selection now prefers expired slots first.
- If all slots are active, it overwrites the oldest slot.
- Slot state is shared per world so multiple players do not each overwrite slot 0 independently.
- `EchoWaveEmitterComponent` now targets 16 simultaneous numbered slots.
- `EchoGameplayComponent` now keeps the active echo wave limit at 16 or higher.
- `Scripts/ensure_echo_wave_mpc_slots.py` and `Scripts/build_echo_wave_16slot_material.py` exist for the 16-slot MPC/material rebuild path.
- `MF_EchoWaveSlot` and backup assets exist for the material expansion pass.

Current limitation / verification needed:

- The 16-slot material/MPC pass still needs Unreal Editor verification and a two-player wave spam test.
- Confirm `M_Env_EchoWave_Test` visually consumes all 16 slots after the generated material graph rebuild.
- High-frequency wave spam can still visually compete for slots if wave duration, interval, and priority are not tuned.

### Weapon Sound / Visual Feedback

Implemented:

- Projectile impact can trigger an echo wave at the hit point.
- `EchoVisualWaveActor` exists as a temporary visual-only wave actor.
- Weapon fire now creates a red local visual wave at the muzzle.
- The red weapon-fire wave is multicast-driven and locally spawned on each client.
- The red weapon-fire wave is not replicated as an actor, avoiding duplicate visuals.
- The strong blue material wave on weapon fire has been temporarily disabled.
- `EchoAudioEventComponent` exists as the shared Wwise C++ gameplay audio entry point.
- Movement audio now chooses normal/sprint/silent event types from movement state.
- Weapon fire now posts weapon-mode audio event types from the muzzle.
- Projectile impact now posts an impact audio event type at the impact point.

Current intent:

- Blue/cyan material echo waves are reserved for movement, scans, and impact/map-reading information.
- Red waves communicate weapon fire / attack exposure.
- Audio event properties still need Wwise asset assignment in Blueprint and PIE verification beyond the already verified normal movement event.

### Skills and Decoy

Implemented first pass:

- `EchoSkillComponent` exists for player skill activation.
- `EEchoCharacterSkill::NoiseDecoy` exists.
- `EchoDecoyActor` exists with trigger sphere, visual mesh, combat/reveal components, lifetime, trigger damage, and periodic noise wave settings.
- Decoy spawn logic exists through `EchoSkillComponent::ActivateNoiseDecoy`.
- Decoy noise currently uses the shared echo/audio event path rather than a separate one-off Blueprint audio call.

Needs verification:

- Bind the skill/decoy input path in the player Blueprint.
- Create or assign final decoy Blueprint visuals and tuning.
- Add a dedicated decoy Wwise event and map ping instead of reusing movement-style placeholder behavior.
- Multiplayer test decoy spawn, trigger, damage, cleanup, and client visibility.

### Maps and Levels

Implemented or present:

- `DM_Tian`
  - Simple square battle test map.
  - Four-room "田" shaped layout.
  - Used for fast multiplayer combat tests.
- `DM_EchoAtrium`
  - Graybox multi-layer indoor deathmatch map.
  - Central atrium and looping paths.
  - Backup map files exist from rebuild iterations.
- `LeveL1`
  - Existing calibration/tutorial-style prototype level.
- `Level2`
  - Existing layered exploration prototype level.

### Assets and Blueprint Work

Current useful Blueprint/assets:

- `BP_ThirdPersonCharacter`
- `BP_EchoDeathmatchGameMode`
- `WBP_EchoHUD`
- `MPC_EchoWaves`
- Existing black environment / reveal / ball materials.
- Existing collectible and energy pickup assets from prototype systems.

## Core Multiplayer Demo Vision

The first complete demo is a 4-8 player deathmatch built around limited vision, echo visibility, and audio positioning.

- Network target: LAN / Listen Server.
- Test target: one machine with multiple UE windows, then local network clients.
- Win condition: first player to 10 kills.
- Match style: free-for-all resource deathmatch.
- Player ability model: all players start with the same base kit.
- Pickup role: temporary advantage only, not direct scoring.

Core play loop:

- Move to gather map and positional information.
- Decide between sprint exposure and silent repositioning.
- Use scan or movement sound to reveal geometry and threats.
- Infer enemy position from sound, impact, projectiles, and fading visibility.
- Fire based on partial information.
- Use pickups and later decoys to create advantage.
- Kill enemies while avoiding becoming the easiest sound source to track.

## Player Actions and Input

Current or planned first-demo inputs:

- Move: standard movement.
- Jump: basic jump.
- `Shift`: sprint.
  - Higher speed.
  - Larger passive movement waves.
  - More exposure.
- `Ctrl`: silent walk.
  - Lower speed.
  - Smaller passive movement waves.
  - Not fully silent.
- `F`: quick local scan.
  - Existing prototype scan behavior remains available.
  - Needs multiplayer tuning pass.
- `E`: charged radial scan.
  - Prototype behavior exists.
  - Needs final charge/range/cooldown pass.
- `LMB`: fire current weapon mode.
- `RMB`: planned / first-pass C++ decoy throw path, still needs Blueprint input and multiplayer verification.
- `Q`: cycle weapon mode.
- `1`: select Standard.
- `2`: select RapidCloseRange.
- `3`: select LongRangeSnipe.

## Echo / Sound Risk-Reward Rules

Sound is both information and exposure.

Every important action should eventually create an audio/echo event:

- Normal movement.
- Sprint movement.
- Silent movement.
- Quick scan.
- Charged scan.
- Weapon fire.
- Projectile impact.
- Decoy activation.
- Pickup collection.
- Shield hit.
- Respawn.

Current implementation already supports movement waves, weapon fire red waves, and impact waves. Full audio and reveal integration is still pending.

Rules:

- Stronger sound gives more information and more exposure.
- Quiet actions reduce footprint but never become perfect invisibility.
- Revealed actors should fade back into darkness.
- The memory map should remember discovered layout, not provide permanent live radar.

## Weapon Modes

### 1. Standard

- Default weapon.
- Medium projectile speed.
- Medium range.
- Medium damage.
- Medium cooldown.
- Purpose: reliable general combat option.

### 2. RapidCloseRange

- Short-range fast sound shooting.
- High fire rate.
- Strong close-range pressure.
- Long-range damage falloff.
- Purpose: aggressive close combat and corridor pressure.

### 3. LongRangeSnipe

- Long-range directional sound shot.
- High projectile speed.
- High damage.
- Slow cooldown.
- Largest attack exposure.
- Purpose: punish enemies from range if the shooter has accurate sound/map information.

## Combat, Deathmatch, Respawn, Scoring

First demo combat rules:

- Players have health.
- Weapon projectiles damage players.
- A killed player grants one kill to the attacker.
- First player to 10 kills wins.
- Dead players respawn after a countdown.
- Respawn restores health and control.
- Missing killer or invalid death should not award a kill.

Implemented baseline:

- Damage.
- Death.
- Respawn.
- Kills/deaths.
- Kill target.
- Victory/defeat event path.

Needs more tuning:

- Respawn safety.
- Spawn point distribution.
- Post-respawn invulnerability if needed.
- Clearer death/respawn HUD feedback.

## Pickups and Temporary Buffs

Planned initial pickup set:

### Overcharge

- Temporarily increases scan range and weapon damage.
- Also increases sound exposure.
- Best for aggressive pushes.

### Silent

- Temporarily reduces movement and scan exposure.
- Does not make the player completely silent.
- Best for repositioning, ambush, and escape.

### Shield

- Absorbs one hit or a limited amount of damage.
- Does not prevent reveal from scan or hit feedback.
- Best for crossing dangerous spaces or surviving close fights.

Current status:

- Prototype collectible and energy pickup assets exist.
- Multiplayer buff pickup system is not implemented yet.

## Map and Memory System

Current status:

- Exploration map component and map widget exist.
- Memory-map style exploration exists from prototype work.
- Direction arrow requirement is part of the demo goal.

Needed for multiplayer demo:

- Player direction arrow should be clearly visible.
- Own explored geometry should remain remembered.
- Other players should only appear as temporary pings.
- Pickups, decoys, loud shots, and impact points should become temporary fading pings.
- Ping types should be visually distinct:
  - Player.
  - Pickup.
  - Decoy.
  - Attack.
  - Scan source.

## Audio Design Goals

Audio is a core gameplay system, not just presentation.

Goals:

- Spatial audio should allow direction and distance estimation.
- Audio should be faster and longer-range than visual echo.
- Action type should be identifiable by sound:
  - Normal movement.
  - Sprint.
  - Silent walk.
  - Quick scan.
  - Charged scan.
  - Standard shot.
  - Rapid shot.
  - Snipe shot.
  - Impact.
  - Decoy.
  - Pickup.
  - Death/respawn.
- Loud sounds should be easier to locate.
- Quiet sounds should be harder to locate but readable nearby.

Current status:

- Visual sound logic is progressing.
- Real spatial audio pass is still pending.

## Skeletal Mesh and Animation Direction

Future implementation should keep `BP_ThirdPersonCharacter` as the player pawn and replace only the visual layer.

Planned structure:

- Keep Capsule, CharacterMovement, camera, combat, weapon, HUD, map, and sound components.
- Add or replace visible mesh with a Skeletal Mesh.
- Keep the ball mesh temporarily as debug if useful.
- Animation Blueprint reads replicated gameplay state from `EchoCharacterStateComponent`.

AnimBP should read:

- Speed.
- In-air state.
- Sprint state.
- Silent-walk state.
- Dead state.
- Current weapon mode.
- Recently fired state.

State machine target:

- Idle.
- Walk.
- Run.
- Silent walk.
- Jump.
- Dead.

Montage target:

- Fire.
- Hit reaction.
- Respawn.

## Level Plan

### Tutorial Level

Purpose: teach the full first-demo control loop.

Required beats:

- Move and jump.
- Sprint and see/hear larger waves.
- Silent walk and see/hear smaller waves.
- Quick scan.
- Charged scan.
- Switch weapons with `Q` and `1/2/3`.
- Fire each weapon mode.
- Read HUD and crosshair.
- Later: throw decoy.
- Later: pick up Overcharge, Silent, and Shield.
- Later: read minimap direction arrow and pings.

### Multiplayer Combat Map

Purpose: support 4-8 player deathmatch.

Required structure:

- Multiple spawn points.
- Central contested area.
- Side routes for quiet repositioning.
- Obstacles that block line of sight but support echo reading.
- Pickup points in exposed or contestable positions.
- Some verticality if readability is preserved.
- Enough darkness and occlusion that sound matters.

Current maps:

- `DM_Tian`: quick combat test layout.
- `DM_EchoAtrium`: larger graybox candidate.

## Future Modes

These are retained as later directions, not first-demo requirements.

### Asymmetric Funeral Player Mode

- 1 or 2 stronger hidden-role players become hunters.
- Other players identify, evade, gather resources, and counterattack.
- Requires identity ambiguity and careful balance.

### Team Base Mode

- 2v2, 3v3, or 4v4.
- Teams touch, control, or destroy enemy echo bases.
- Requires team UI, base objective logic, and team-colored sound identity.

### Single-Player Survival

- Enemy wave mode.
- Player gains echo energy and upgrades through kills and pickups.

### Single-Player Stealth Puzzle

- Avoid hunters and reach an exit.
- Uses echo, quiet movement, decoys, and memory map.

### Rogue Ability Draft

- Players collect sound items or submit sound artifacts to unlock ability choices.
- Candidate abilities include dash, silent state, stillness invisibility, traps, decoys, wall cling, sniper echo, shield, terrain block, light field, healing, and large interference.

## Out of Scope for First Demo

Do not include these in the first implementation pass:

- Online matchmaking.
- Dedicated server deployment.
- Account system.
- Ranked progression.
- Full character modeling replacement.
- Multiple character classes.
- Asymmetric hidden-role mode.
- Team base mode.
- Single-player survival mode.
- Single-player stealth puzzle mode.
- Rogue card drafting.
- Complex AI enemies.
- Permanent live radar for all players.
- Direct editing of `.uasset` files outside the Unreal Editor.

## Current Next Goals

### Goal 1: Make Weapon Fire Feedback Readable

Current state:

- Weapon fire creates a red visual wave.
- Strong blue fire wave is disabled.

Next:

- Tune red wave thickness, duration, radius, and visibility.
- Decide whether impact waves should stay blue/cyan or use weapon-specific color.
- Add short weapon-specific spatial audio placeholders.
- Add hit feedback on damaged player.

### Goal 2: Stabilize Echo Wave Capacity

Current state:

- C++ slot policy is improved.
- C++ and generated asset workflow now target 16 slots.
- MPC/material assets have been modified for the 16-slot pass, with backups created.

Next:

- Verify `MPC_EchoWaves` and `M_Env_EchoWave_Test` consume all 16 slots in Unreal Editor.
- If 16 slots is too expensive or visually noisy, reduce to 8 or 12 after profiling/readability testing.
- Verify two players sprinting, scanning, and shooting do not immediately overwrite important waves.

### Goal 3: Finish First-Pass Scan and Reveal Rules

Next:

- Make `F` quick scan clearly short-range and low-risk.
- Make `E` charged scan scale range and exposure by charge time.
- Ensure scanned players briefly reveal/fade.
- Ensure scanned pickups and important objects briefly reveal/fade.

### Goal 4: Add Decoy

Current state:

- C++ first pass exists through `EchoSkillComponent` and `EchoDecoyActor`.
- Decoy has trigger/damage/lifetime/noise wave behavior.

Next:

- Bind `RMB` or the selected skill input to activate the decoy path.
- Add or assign decoy Blueprint visuals and tuning values.
- Give decoy a dedicated spatial audio event.
- Decoy creates temporary map ping.
- Decoy should mislead but remain learnable.

### Goal 5: Add Multiplayer Pickups

Next:

- Implement replicated buff pickup actor.
- Add `Overcharge`, `Silent`, and `Shield`.
- Add duration/expiration.
- Add HUD display.
- Place pickups in `DM_Tian` and later `DM_EchoAtrium`.

### Goal 6: HUD and Map Polish

Next:

- Improve `WBP_EchoHUD` visual layout.
- Make state panel show death/respawn/victory/defeat cleanly.
- Add current buff display.
- Add map direction arrow if not already stable.
- Add fading map pings for shots, impacts, pickups, decoys, and players.

### Goal 7: Audio Pass

Current state:

- Wwise integration is complete enough for `Play_Move_Normal` playback from C++.
- C++ event types/hooks now exist for movement variants, weapon fire, and projectile impact.

Next:

- Assign and verify placeholder spatial audio for sprint and silent movement.
- Assign and verify weapon-mode audio events.
- Assign and verify impact audio event.
- Add scan audio.
- Add decoy audio.
- Add pickup and respawn audio.
- Tune loudness and falloff so audio gives more range than visual wave feedback.

### Goal 8: Animation Readiness

Next:

- Keep `EchoCharacterStateComponent` as the source of movement/combat state.
- Later attach Skeletal Mesh and Animation Blueprint.
- Do not rewrite gameplay components for animation.

## Implementation Roadmap

### Phase 1: Documentation and Scope

Status: mostly complete.

- Keep this document updated as the current design source.
- Keep first demo focused on 4-8 player deathmatch.

### Phase 2: Multiplayer Combat Core

Status: implemented baseline.

- Deathmatch GameMode.
- PlayerState kills/deaths/player identity.
- Combat component health/death/respawn.
- Winner event path.
- HUD hooks.

Needs:

- More spawn safety and final score presentation.

### Phase 3: Weapon Combat

Status: implemented baseline.

- Weapon mode enum.
- Weapon component.
- Replicated projectile.
- Third-person aim correction.
- Crosshair.
- Red fire wave.
- Impact wave.

Needs:

- Better hit visuals.
- Audio.
- Weapon tuning pass.

### Phase 4: Movement Sound

Status: implemented baseline; audio hook expansion in progress.

- Normal/sprint/silent movement states.
- Movement speed Blueprint tuning.
- State-specific passive waves.
- Immediate state-change movement wave.
- C++ movement audio event type selection for normal/sprint/silent.

Needs:

- Wwise event asset assignment and PIE verification for sprint/silent.
- Multiplayer readability tests.
- 16-slot material verification.

### Phase 5: Scan / Reveal / Map Pings

Status: partially inherited from prototype.

Needs:

- Multiplayer-tuned quick scan.
- Multiplayer-tuned charged scan.
- Temporary player/pickup/decoy pings.
- Direction arrow verification.

### Phase 6: Pickups and Decoys

Status: decoy C++ first pass exists; multiplayer pickups are not implemented yet.

Needs:

- Decoy input binding, Blueprint visual setup, dedicated audio, map ping, and multiplayer verification.
- Buff pickup actor/system.
- HUD buff state.
- Map placement.

### Phase 7: Level and Audio Pass

Status: graybox maps, menu/lobby maps, and first Wwise gameplay route exist.

Needs:

- Choose `DM_Tian` or `DM_EchoAtrium` as primary short-term test map.
- Add spawn and pickup layout.
- Complete placeholder spatial audio assignment for movement, weapons, impact, scan, decoy, pickups, death, and respawn.
- Tune readability and combat pacing.

## Development Rules

- Do not directly edit `.uasset` files outside Unreal Editor.
- Do not delete or rename existing assets unless explicitly requested.
- Prefer reusable C++ Actor Components for gameplay logic.
- Use Blueprint for visual setup, mesh/material assignment, audio asset wiring, widget layout, Niagara, and tuning values.
- Keep changes small, compilable, and testable.
- Preserve working prototype systems unless replacing them is necessary for multiplayer behavior.
- After code changes, document required Unreal Editor setup steps.
- Use UnrealBuildTool / UE compile for C++ builds, not `dotnet build` through generated Visual Studio project files.
