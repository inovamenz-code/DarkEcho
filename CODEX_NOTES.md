# BallDarkEcho Design Notes

## Project Overview

- Project name: BallDarkEcho.
- Engine: Unreal Engine 5.7.
- Genre target: limited-vision multiplayer echo combat.
- First playable demo target: 4-8 player LAN / Listen Server deathmatch.
- Core fantasy: players move through a dark arena where sound is both the main source of information and the main way to reveal themselves.
- Current avatar direction: keep the white ball player for the first demo, then distinguish players through color, outline, sound identity, UI labels, and material tuning.
- Main design pillar: every sound-producing action should create a tradeoff between information gain, combat pressure, and exposure risk.

## Current Implementation Status

The original single-player echo exploration prototype has already moved beyond the old notes. Current useful systems and assets include:

- `BP_EchoPulse` as the current controllable player blueprint.
- Basic movement and white ball visual setup.
- Echo energy and frequency gameplay component.
- Echo pulse scanning component.
- Echo reveal target component for temporary visibility and custom depth reveal.
- Echo wave emitter component using material parameter collection wave slots.
- Movement pulse component for passive movement waves.
- Exploration map component and map widget.
- HUD presenter component and HUD widget hooks.
- Main menu widget and game mode classes.
- Collectible actor, energy pickup behavior, exit actor, and basic progression hooks.
- Level 1 calibration map generation script.
- Level 2 three-layer exploration map generation script.
- Useful assets:
  - `M_Env_Black`
  - `M_Ball_White`
  - `M_Echo_Reveal_White`
  - `M_Echo_Decal_Test`
  - `MPC_EchoWaves`
  - `BP_EchoPulse`
  - `BP_AcousticFragment`
  - `BP_EnergyPickup`
  - `BP_EchoExit`
  - `LeveL1`
  - `Level2`

This document is now the authoritative project design note. Older prototype-era statements about missing reusable echo gameplay are obsolete.

## Core Multiplayer Demo Vision

The first complete demo should be a 4-8 player deathmatch built around echo visibility and audio positioning.

- Network target: LAN / Listen Server.
- Test target: one machine with multiple UE windows, plus local network clients when available.
- Win condition: first player to 10 kills wins.
- Match style: free-for-all resource deathmatch.
- Player ability model: all players begin with the same base abilities.
- Pickup role: pickups grant temporary advantages and do not directly score points.
- Outplay focus:
  - Move loudly to gain speed and information, but reveal yourself.
  - Move quietly to stay hidden, but lose speed and map knowledge.
  - Scan to find walls, players, pickups, decoys, and threats.
  - Attack based on partial information from sound, visible pulses, and memory map pings.
  - Use decoys to create false sound sources and manipulate pursuit decisions.

The first demo should feel competitive before it becomes content-heavy. A small set of clear mechanics is preferred over many under-tuned abilities.

## Player Actions and Input

All first-demo players use the same input set:

- Move: standard movement.
- Jump: basic jump, using the current player movement setup.
- `Shift`: fast movement.
  - Higher speed.
  - Larger passive movement waves.
  - Wider audible range.
  - Stronger exposure risk.
- `Ctrl`: quiet movement.
  - Lower speed.
  - Smaller passive movement waves.
  - Lower audible range.
  - Not fully silent.
- `F`: quick local scan.
  - Short cooldown.
  - Small radius.
  - Low exposure.
  - Used to confirm immediate nearby geometry or threats.
- `E`: charged radial scan.
  - Hold to charge.
  - Release to emit an expanding scan.
  - Longer charge means larger range, stronger visual wave, stronger audio exposure, and higher cost or cooldown.
- `LMB`: fire the current weapon mode.
- `RMB`: throw a decoy sound emitter.
  - The decoy lands or stops at a target point.
  - It emits a delayed sound pulse.
  - It can mislead enemies, reveal nearby surfaces, and create fake pursuit information.
- `Q`: cycle weapon mode.
- `1`: select Standard weapon.
- `2`: select RapidCloseRange weapon.
- `3`: select LongRangeSnipe weapon.

## Echo / Sound Risk-Reward Rules

Sound is both the information system and the exposure system.

- Every important action should produce a sound event:
  - Normal movement.
  - Fast movement.
  - Quiet movement.
  - Quick scan.
  - Charged scan.
  - Weapon fire.
  - Projectile impact.
  - Decoy activation.
  - Pickup collection.
  - Shield hit.
  - Respawn.
- Sound events should feed three forms of feedback:
  - Visual echo waves in the world.
  - Temporary reveal or outline on affected actors.
  - Spatial audio cues that travel farther and are understood faster than visual wave information.
- Stronger sound gives stronger information but exposes the source more.
- Quiet actions are not perfectly invisible; they only reduce scan and audio footprint.
- Revealed actors should fade back into darkness instead of becoming permanently visible.
- Walls, floors, players, pickups, projectiles, decoys, and interactable objects should be able to participate in the echo information loop.

## Weapon Modes

The first demo has three weapon modes. These are selected with `Q` or direct number keys.

### 1. Standard

- Default weapon.
- Medium projectile speed.
- Medium range.
- Medium damage.
- Medium cooldown.
- Medium audio exposure.
- Purpose: reliable general combat option for most engagements.

### 2. RapidCloseRange

- Short-range fast sound shooting.
- High fire rate.
- Medium-high projectile speed.
- Strong close-range pressure.
- Clear damage falloff at long range.
- Lower single-shot damage than the sniper.
- Purpose: punish close pursuit, clear tight corridors, and reward aggressive movement.

### 3. LongRangeSnipe

- Long-range directional sound shot.
- Fast projectile speed.
- High damage.
- Long range.
- Slow reload or long cooldown.
- Large audio exposure radius.
- Purpose: high-risk ranged punishment when the shooter has accurate sound or map information.

Weapon hits should briefly reveal the hit player. Weapon fire should reveal or ping the shooter based on weapon loudness.

## Combat, Deathmatch, Respawn, Scoring

First demo combat rules:

- Players have health.
- Players can damage each other with sound weapons.
- A killed player grants one kill to the attacker.
- First player to 10 kills wins.
- Dead players respawn after a short countdown.
- Respawn should reset position, restore health, and clear temporary negative combat state.
- Respawn should produce a recognizable but not overwhelming sound event.
- Suicide, environmental death, or missing killer should not award a kill.
- The match should continue until the kill target is reached.

The first demo does not need ranked scoring, assists, teams, round economy, or long-term progression.

## Pickups and Temporary Buffs

Pickups are map-control incentives. They should create reasons to move, expose yourself, ambush, or contest areas.

Initial pickup set:

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

Existing Acoustic Fragment and Energy Pickup assets can be repurposed or kept for tutorial/single-player style content, but the first multiplayer deathmatch demo treats pickups as temporary combat or stealth advantages.

## Map and Memory System

The map should support limited-vision play rather than replace it.

- The memory map keeps explored geometry.
- Newly explored cells are added through movement and echo hit locations.
- Walls and major arena outlines should appear only after discovery.
- The player marker should include a direction arrow.
- Other players should not be permanently visible on the map.
- Players, pickups, decoys, and important sound events should appear as temporary pings.
- Pings should fade out after a short duration.
- Different ping types should be visually distinct enough for quick reading:
  - Player ping.
  - Pickup ping.
  - Decoy ping.
  - Projectile or attack ping.
  - Loud scan source ping.

The map should reward sound-based exploration and memory, not provide full radar vision.

## Audio Design Goals

Audio is a core gameplay system, not just presentation.

- Spatial audio should allow players to estimate direction and distance.
- Audio feedback should be faster and longer-range than visual echo feedback.
- Sound identity should communicate action type:
  - Foot movement.
  - Fast movement.
  - Quiet movement.
  - Scan.
  - Charged scan.
  - Standard shot.
  - Rapid shot.
  - Sniper shot.
  - Decoy pulse.
  - Pickup.
  - Shield hit.
  - Death and respawn.
- Loud sounds should be easier to locate.
- Quiet sounds should be harder to locate but still readable at close range.
- Weapon modes should have distinct audio signatures so players can infer threat level.
- Decoys should sound believable enough to cause hesitation, but should still be learnable through careful listening.

The demo should be playable with visual feedback, but it should become significantly better when players listen carefully.

## Level Plan

### Tutorial Level

Purpose: teach the full first-demo control loop before multiplayer combat.

Required beats:

- Move and jump.
- Use `Shift` to move fast and see/hear larger waves.
- Use `Ctrl` to move quietly and see/hear smaller waves.
- Use `F` to scan nearby geometry.
- Use charged `E` scan to reveal a larger area.
- Use `1/2/3` and `Q` to switch weapon modes.
- Fire each weapon mode at test targets.
- Throw a decoy with `RMB`.
- Read the minimap and direction arrow.
- Pick up Overcharge, Silent, and Shield.
- Reach an exit or finish marker.

### Multiplayer Combat Map

Purpose: support 4-8 player deathmatch.

Required structure:

- Multiple spawn points with safe initial spacing.
- Central contested area with good sound propagation.
- Side routes for quiet repositioning.
- Obstacle cover that blocks direct line of sight but still supports echo reading.
- Pickup points in exposed or contestable areas.
- Some verticality or layered paths if it can be kept readable.
- Enough darkness and occlusion that sound matters.
- No required objective besides kills for the first demo.

### Existing Levels

- `LeveL1` can become the tutorial / calibration level.
- `Level2` can inform vertical and layered arena ideas, but may need simplification for first multiplayer readability.

## Future Modes

These are design directions to keep, but they are not first-demo requirements.

### Asymmetric Funeral Player Mode

- 1 or 2 stronger hidden-role players become hunters.
- Other players must identify, evade, gather resources, and eventually counterattack.
- Needs identity ambiguity, stronger hunter abilities, social deduction, and careful balance.

### Team Base Mode

- 2v2, 3v3, or 4v4.
- Teams attempt to touch, control, or destroy the enemy echo base.
- Needs team UI, base health or capture logic, respawns, and team-colored sound identity.

### Single-Player Survival

- Zombie-like wave mode.
- Player gains echo energy and upgrades through kills and pickups.
- Enemy density and sound pressure increase over time.

### Single-Player Stealth Puzzle

- Avoid hunters and reach the exit.
- Uses echo, quiet movement, decoys, and map memory for navigation and evasion.

### Rogue Ability Draft

- Players collect sound items or submit three sound artifacts to unlock ability choices.
- Possible abilities include dash, silent state, stillness invisibility, traps, decoys, wall cling, sniper echo, shield, terrain block, light field, healing, and large interference.

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
- Permanent map radar for all players.
- Direct editing of `.uasset` files outside the Unreal Editor.

## Implementation Roadmap

### Phase 1: Documentation and Design Lock

- Keep this document as the current design source.
- Avoid relying on old prototype-era echo notes.
- Keep initial demo scope focused on multiplayer deathmatch.

### Phase 2: Multiplayer Core

- Add deathmatch GameMode.
- Add PlayerState for kills, deaths, player color, and display identity.
- Add replicated combat component for health, damage, death, respawn, and kill credit.
- Add replicated match winner event and HUD hooks.

### Phase 3: Sound Action Framework

- Add a sound event or noise data structure with radius, audible range, visual intensity, duration, and source type.
- Connect movement states to passive sound waves.
- Add fast movement and quiet movement tuning.
- Add quick scan and charged scan behavior.

### Phase 4: Weapons and Decoys

- Add weapon mode enum: `Standard`, `RapidCloseRange`, `LongRangeSnipe`.
- Add replicated sound projectile actor.
- Add mode-specific damage, range, speed, cooldown, falloff, and exposure.
- Add throwable decoy actor with delayed sound pulse.
- Add HUD update for current weapon mode.

### Phase 5: Pickups and Buffs

- Add buff enum: `Overcharge`, `Silent`, `Shield`.
- Add replicated pickup actor or extend existing collectible logic for multiplayer buffs.
- Add duration, expiration, and HUD feedback.
- Tune pickup placement for combat map control.

### Phase 6: Map and HUD

- Add player direction arrow to exploration map.
- Add temporary pings for players, pickups, decoys, and loud sound sources.
- Add kill target, current kills, deaths, current weapon, current buff, and respawn countdown to HUD.

### Phase 7: Levels and Audio Pass

- Convert or rebuild `LeveL1` as tutorial / calibration.
- Create one readable 4-8 player combat map.
- Add placeholder spatial audio cues for each sound event.
- Tune audio distance, falloff, and action identity before visual polish.

## Development Rules

- Do not modify `.uasset` files directly outside the Unreal Editor.
- Do not delete or rename existing assets unless explicitly requested.
- Prefer reusable C++ Actor Components for gameplay logic.
- Use Blueprint for visual setup, mesh/material assignment, audio asset wiring, Niagara, and tuning values.
- Keep changes small, compilable, and testable.
- Preserve existing working prototype systems unless replacing them is necessary for multiplayer behavior.
- After code changes, document required Unreal Editor setup steps.
- Build and test frequently, especially after adding replicated classes.
