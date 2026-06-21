# BallDarkEcho Audio Design

## Purpose

Audio is a core gameplay system in BallDarkEcho, not a presentation layer. Sound must help players locate threats, understand distance, read the arena, identify action type, and judge whether an enemy is exposed or hidden.

The first demo should prioritise competitive readability over physically perfect acoustics. The target is a perceptually convincing hybrid system: Wwise handles spatial playback, attenuation, occlusion, rooms/portals, and selected reflections; Unreal gameplay code emits consistent audio events from movement, scans, weapons, decoys, pickups, death, and respawn.

Steam Audio is not the first-pass dependency. Wwise is the intended main audio middleware.

## Current Implementation Status

Status as of 2026-06-21:

- Wwise 2025.1.8 is integrated into the Unreal project.
- The Wwise project lives at `BallDarkEcho_WwiseProject/BallDarkEcho_WwiseProject.wproj`.
- Generated SoundBanks are output to `BallDarkEcho_WwiseProject/GeneratedSoundBanks`.
- `Main_Gameplay` is the current gameplay SoundBank.
- `Play_Move_Normal` is the first formal gameplay Wwise event.
- `Play_SFX_Echo_Test_Beep` and `SoundBankTest1` are validation-only assets and should not be expanded into production gameplay routing.
- `Content/WwiseAudio/Events/Default_Work_Unit/Play_Move_Normal.uasset` is reconciled and playable from Unreal.
- `EchoAudioEventComponent` is now the C++ audio entry point for gameplay systems.
- `EchoMovementPulseComponent::EmitOneMovementWave` posts `MovementNormal` through `EchoAudioEventComponent`.
- The movement pulse C++ path has been compile-tested with UBT and play-tested in UE.
- The C++ event enum and component properties now cover `MovementSprint`, `MovementSilent`, the three weapon fire events, and `ProjectileImpact`.
- `EchoMovementPulseComponent` now chooses the audio event type from normal/sprint/silent movement state.
- `EchoWeaponComponent` now posts weapon-mode fire audio event types from the muzzle after a projectile is successfully spawned.
- `EchoSoundProjectileActor` now posts `ProjectileImpact` from the impact path.
- These newer events still need player Blueprint / Wwise asset assignment and PIE verification.

Current verified chain:

```text
EchoMovementPulseComponent
  -> EchoAudioEventComponent::PostEchoSoundEvent(MovementNormal, Location, Loudness)
  -> UAkGameplayStatics::PostEventAtLocation
  -> Wwise Play_Move_Normal
  -> Main_Gameplay SoundBank
  -> audible placeholder beep in PIE
```

The sprint, silent walk, weapon fire, and projectile impact hooks now follow this same C++ path, but they are not complete audio features yet. The next work is to create/assign the matching Wwise events, regenerate SoundBanks, and verify playback plus duplicate-playback policy in PIE/listen-server tests. Do not add unrelated Blueprint-only audio calls.

## Design Pillar

Sound is four things at once:

- Information: tells players where geometry, enemies, shots, impacts, pickups, and decoys are.
- Exposure: loud actions reveal the actor who made them.
- Threat: weapon and impact sounds communicate danger level before full visual confirmation.
- Navigation: spatial ambience and action sounds help players understand rooms, corridors, openings, and vertical routes.

Every important action should create a readable tradeoff. Stronger sounds should travel farther, localise more clearly, and create more exposure. Quiet actions should reduce range and clarity, but should not become perfectly silent.

## Echo Sound Event Model

All gameplay audio should be routed through a shared event concept before reaching Wwise. This does not need to be a complex system in the first pass, but every audio hook should be able to describe the same data.

Recommended data fields:

| Field | Purpose |
| --- | --- |
| `EventType` | Gameplay source such as movement, sprint, scan, weapon fire, impact, decoy, pickup, respawn. |
| `WorldPosition` | Position where the sound should be emitted. |
| `Loudness` | Relative strength used for Wwise volume/RTPC and gameplay exposure. |
| `AudibleRadius` | Maximum useful hearing range. Usually tied to existing wave radius or weapon exposure radius. |
| `LocalizationClarity` | How precisely the player should identify direction. Sprint and snipe are high; silent walk and decoy tails are lower. |
| `OcclusionSensitivity` | How strongly walls and blockers darken/reduce the sound. |
| `ReflectionAmount` | How much early reflection support the sound should receive. |
| `VisualWavePolicy` | Whether the event also creates a visual wave, map ping, both, or neither. |
| `WwiseEventName` | Wwise event to post. |

Initial event enum candidates:

```cpp
enum class EEchoSoundEventType : uint8
{
    MovementNormal,
    MovementSprint,
    MovementSilent,
    QuickScan,
    ChargedScan,
    WeaponStandard,
    WeaponRapid,
    WeaponSnipe,
    ProjectileImpact,
    DecoyPulse,
    Pickup,
    ShieldHit,
    Death,
    Respawn
};
```

The event model should become the bridge between audio playback, visual echo waves, and future map pings. Avoid adding one-off audio nodes directly into unrelated Blueprints when a gameplay component can emit a structured event instead.

## First-Demo Audio Requirements

### Movement

Normal movement should be readable at medium range, with moderate localisation clarity and moderate exposure. It should support navigation and player tracking without dominating combat.

Sprint should be louder, wider, easier to locate, and more frequent than normal movement. It should clearly communicate speed and risk. Existing sprint wave defaults are a good tuning source: interval 1.2 seconds, max radius 2600, intensity 14.

Silent walk should be quieter, lower range, darker, and less precise. It must remain audible nearby so that silent walk is a risk reduction, not invisibility. Existing silent walk wave defaults are a good tuning source: interval 2.5 seconds, max radius 800, intensity 4.

Implementation hook: `EchoMovementPulseComponent::EmitOneMovementWave`.

### Scans

Quick scan should be short, sharp, and local. It should help nearby awareness without strongly exposing the player.

Charged scan should scale by charge amount. Longer charge should increase audible radius, visual wave strength, reflection amount, and exposure. The charge should also be audible before release if design later needs counterplay.

Implementation hook: `EchoPulseScannerComponent::TriggerEchoPulseAtLocationWithFrequency`, plus the future charged-scan release path.

### Weapons

Standard shot should be the baseline attack identity: medium loudness, medium range, clear projectile/fire signature, and readable muzzle position.

RapidCloseRange should be shorter, sharper, and repeated quickly. Individual shots should be lighter than Standard, but repeated bursts should create obvious local pressure.

LongRangeSnipe should be the loudest and clearest weapon fire. It should expose the shooter strongly and travel far enough that distant players can identify the threat.

Implementation hook: `EchoWeaponComponent::FireOnServer`, after successful projectile spawn and before/with `MulticastTriggerFireExposureWave`.

### Projectile Impact

Impact audio should be positionally clear at the hit point. Impact should differ from weapon fire so players can distinguish shooter location from hit location.

Impacts against players should sound sharper and more dangerous than impacts against static geometry. Future work can route hit-player impacts to a separate Wwise event or switch.

Implementation hook: `EchoSoundProjectileActor::HandleOverlap` and `MulticastTriggerImpactWave`.

### Decoy

Decoy audio should be believable enough to delay enemy decisions, but learnable through careful listening. It should imitate movement/scan qualities without perfectly copying player-owned cues.

First pass: one delayed `DecoyPulse` event at landing/rest location, with medium audible radius, slightly reduced localisation clarity, and a distinct tail or texture.

Future hook: replicated decoy actor activation.

### Pickups and Buffs

Pickup audio should confirm collection locally and create a small world cue for nearby enemies. Overcharge, Silent, and Shield can share a base pickup cue with switch-specific layers.

Buff activation should be readable to the owner. Enemy readability depends on balance:

- Overcharge should be audible enough to warn nearby enemies.
- Silent should be mostly owner-facing and quiet externally.
- Shield should have a clear activation and hit/absorb cue.

Future hook: multiplayer buff pickup actor/system.

### Death and Respawn

Death should be clear but not long. Respawn should be recognisable and spatial if it reveals spawn pressure. Avoid making respawn so loud that players are immediately punished without control.

Implementation hook: `EchoCombatComponent` death/respawn state changes.

## Current Gameplay Audio Hook Map

| System | Current role | Audio hook |
| --- | --- | --- |
| `EchoMovementPulseComponent` | Emits normal/sprint/silent visual movement waves. | Currently posts `MovementNormal`, `MovementSprint`, or `MovementSilent` based on movement state. Sprint/silent still need Wwise event asset assignment and PIE verification. |
| `EchoWeaponComponent` | Handles weapon mode, fire authority, projectile spawn, red fire wave. | After projectile spawn succeeds, posts `WeaponStandardFire`, `WeaponRapidFire`, or `WeaponSnipeFire` at the muzzle. Needs Wwise event asset assignment plus mix/timing verification. |
| `EchoSoundProjectileActor` | Handles replicated projectile movement, damage, impact wave. | Posts `ProjectileImpact` at the collision point from the impact path. Needs impact Wwise event assignment and duplicate-playback checks. |
| `EchoPulseScannerComponent` | Emits scan pulse and surface hit callbacks. | Emit quick/charged scan event at scan origin. |
| `EchoCombatComponent` | Handles health, death, respawn. | Emit death, damage, shield hit, respawn events. |
| `EchoDecoyActor` / `EchoSkillComponent` | C++ first pass can spawn a decoy with trigger radius, damage, lifetime, and periodic noise waves. | Add a dedicated `DecoyPulse` Wwise event and map ping; do not keep using a movement placeholder long term. |
| Future pickup actor/system | Applies temporary buffs. | Emit pickup and buff activation events. |

The first implementation now uses `EchoAudioEventComponent` as the shared C++ entry point. Keep gameplay systems posting through this component so timing, cooldowns, replication policy, debug drawing, RTPC values, and future occlusion/reflection values stay centralised.

## Wwise Project Structure

### Events

Current Wwise event naming uses short action names without the extra `Echo` prefix. Keep Work Units and Events distinct: Work Units should use grouping names such as `WU_Movement`; Events should use `Play_*`.

Current verified event:

```text
Play_Move_Normal
```

Next formal events to create as placeholder routing:

```text
Play_Move_Sprint
Play_Move_Silent
Play_Weapon_Standard_Fire
Play_Weapon_Rapid_Fire
Play_Weapon_Snipe_Fire
Play_Projectile_Impact
Play_Scan_Quick
Play_Scan_Charged
Play_Decoy_Pulse
Play_Pickup
Play_Shield_Hit
Play_Death
Play_Respawn
```

Use placeholder audio first. The system is more important than final assets in the first pass. Do not name a Work Unit the same as an Event; Wwise will create suffixes such as `Play_Move_Normal1`, which breaks predictable UE asset lookup.

### Switches and States

Use switches for identity variants:

- `WeaponMode`: `Standard`, `RapidCloseRange`, `LongRangeSnipe`.
- `MovementMode`: `Normal`, `Sprint`, `Silent`.
- `SurfaceType`: `Default`, `Metal`, `Concrete`, `Glass`, `Soft` if physical materials are later available.
- `HitType`: `World`, `Player`, `Shield`.

Use states sparingly for global mix:

- `MatchState`: `Menu`, `Playing`, `Dead`, `Result`.
- `PlayerState`: `Normal`, `LowHealth`, `Respawning`.

### RTPCs

Recommended RTPCs:

| RTPC | Range | Use |
| --- | --- | --- |
| `Echo_Loudness` | 0-1 | Scales volume and intensity. |
| `Echo_AudibleRadius` | Unreal units or normalized | Selects attenuation/falloff behaviour. |
| `Echo_LocalizationClarity` | 0-1 | Controls stereo width, transient clarity, or filtering. |
| `Echo_OcclusionAmount` | 0-1 | Controls volume reduction and LPF. |
| `Echo_Exposure` | 0-1 | Mix/gameplay value for how revealing a sound is. |
| `Echo_ChargeAmount` | 0-1 | Scales charged scan layers. |
| `Echo_ReflectionAmount` | 0-1 | Controls early reflection send. |

For the first pass, `Echo_Loudness`, `Echo_AudibleRadius`, and `Echo_OcclusionAmount` are enough.

### Attenuation Sharesets

Create separate attenuation sharesets for readability:

- `ATT_Echo_Close`: silent walk, pickup, shield hit.
- `ATT_Echo_Mid`: normal movement, Standard fire, quick scan.
- `ATT_Echo_Far`: sprint, charged scan, snipe fire.
- `ATT_Echo_Impact`: projectile impact.
- `ATT_Echo_Decoy`: decoy pulse.

Prefer logarithmic or natural-style falloff for spot sounds. Avoid flat long-range sounds except for deliberate global match feedback.

### Busses

Current bus layout created in Wwise:

```text
Master Audio Bus
  Gameplay_SFX
  Player_Feedback
  Enemy_Cues
  UI
  Ambience
  Reverb_Reflection
```

Target bus layout after routing becomes more detailed:

```text
Master Audio Bus
  Gameplay
    EchoMovement
    EchoWeapons
    EchoScans
    EchoImpacts
    EchoPickups
  UI
  Ambience
  Music
  ReverbAndReflections
```

Keep gameplay-critical SFX separate from ambience and music so they can be protected in the mix.

## Asset Sourcing Checklist

The project can progress with placeholder sounds while gameplay logic is being built. Replace each placeholder only when the event hook and Wwise routing are stable.

Required first-demo source categories:

| Category | Needed assets | Notes |
| --- | --- | --- |
| Movement normal | 3-6 short pulses/footstep-like clicks | Medium range, readable transient, not too tonal. |
| Movement sprint | 3-6 sharper/louder variants | More attack and range than normal movement. |
| Movement silent | 3-6 muted soft variants | Close-range only; darker and less precise. |
| Weapon Standard | 2-4 fire layers plus optional tail | Clear muzzle identity. |
| Weapon Rapid | 3-6 short light shots | Must survive repetition without harshness. |
| Weapon Snipe | 1-3 loud high-clarity shots | Strong exposure, long distance identity. |
| Projectile impact | 3-6 impacts | Separate world hit and player/shield hit later. |
| Scan quick | 1-3 short pulses | Clean attack, fast decay. |
| Scan charged | charge loop/riser plus release | Needs later `Echo_ChargeAmount` RTPC support. |
| Decoy pulse | 1-3 deceptive pulses | Similar to movement/scan but with a learnable tail. |
| Pickup/buff | pickup, overcharge, silent, shield | Owner confirmation first, world cue second. |
| Death/respawn | short death cue, respawn cue | Clear but not long. |
| Ambience | low-density arena beds | Must not mask footsteps, shots, or scans. |

Asset selection rules:

- Prefer dry mono source sounds for positional gameplay cues; Wwise should handle 3D placement and reverb.
- Avoid long baked reverb tails on movement and weapon transient sounds.
- Keep placeholder files named by intent, for example `move_normal_click_01.wav`, not by source website filename.
- Track license/source for every downloaded or generated asset before it enters the final demo.
- Put raw source files outside `.uasset` workflow first, then import through Wwise when selected.
- Replace one event category at a time and regenerate `Main_Gameplay` after each change.

## Spatial Audio, Occlusion, and Reflection

### Positioning

Gameplay-critical cues should be mono 3D sources unless there is a specific reason to use stereo. The player must be able to locate a precise point for movement, weapon fire, impact, decoy, pickup, death, and respawn.

Use binaural/HRTF rendering for headphone-focused testing where available. Generic HRTF is acceptable for first pass; individualised HRTF is not practical for this project.

### Occlusion

Occlusion should be perceptual and controllable:

- Direct line of sight: clear, bright, precise.
- One wall blocked: lower volume, lower LPF cutoff, slightly lower localisation clarity.
- Multiple blockers/thick geometry: stronger reduction, darker tone.
- Doorways/portals: sound should remain readable from the opening rather than vanish.

First pass can use Wwise/Unreal obstruction and occlusion values. If custom logic is needed, use a trace from listener to source and send `Echo_OcclusionAmount` to Wwise.

### Rooms and Portals

Use Wwise Spatial Audio Rooms/Portals for authored map readability. This is especially important for `DM_EchoAtrium`, where stacked rings, central atrium, stairs, and side routes can otherwise become confusing.

First pass should mark only major acoustic spaces:

- central atrium
- outer loop rooms/corridors
- stair/vertical route openings
- major doorways or gaps

Do not over-author every small wall segment early. The goal is readable propagation, not exhaustive simulation.

### Geometry and Early Reflections

Use Wwise Geometry for selected major surfaces only:

- central atrium walls
- large flat arena blockers
- major metal/concrete surfaces
- long corridors where reflections help distance reading

Reflections should support location and space. They must not mask direct weapon or movement cues. Start with low reflection amount and raise only if playtests remain readable.

## Relationship to Visual Echo and Map Pings

Audio, visual waves, and map pings should usually come from the same Echo Sound Event, but they do not need the same radius.

Recommended rule:

- Audio range is usually longer than visual wave range.
- Visual wave is clearer for geometry reading.
- Audio is faster for threat detection.
- Map pings are delayed/fading summaries, not permanent radar.

Examples:

| Event | Audio | Visual wave | Map ping |
| --- | --- | --- | --- |
| Normal movement | Medium | Medium blue/cyan | No or short self-only memory update |
| Sprint | Far and clear | Large blue/cyan | Possible temporary loud-source ping |
| Silent walk | Short and dark | Small blue/cyan | No enemy ping |
| Weapon fire | Far and sharp | Red fire wave | Attack ping |
| Impact | Medium | Blue/cyan impact wave | Impact ping |
| Decoy | Medium, slightly suspicious | Decoy pulse | Decoy-like ping or false attack ping |
| Pickup | Short/medium | Optional small reveal | Pickup ping if discovered |

## Multiplayer Rules

Server-authoritative gameplay should decide whether an event happened. Audio playback can be multicast or locally predicted depending on event type.

Recommended first-pass policy:

- Owner-local immediate feedback for movement and firing so controls feel responsive.
- Server/multicast confirmation for weapon fire, projectile impact, death, respawn, pickups, and decoys.
- Avoid duplicate playback for the owning client when multicast arrives.
- Do not replicate long-lived audio actors unless the sound itself needs continuous position tracking.

For one-shot events, prefer posting a Wwise event at a location. For looping/continuous sources, attach an audio component to the actor.

## Implementation Order

1. Wwise integration sanity check. Done for editor PIE.
   - Confirm Wwise plugin is installed and enabled.
   - Confirm one placeholder event can play in Unreal.
   - Confirm soundbanks load in editor and packaged/listen-server tests.

2. Placeholder 3D event playback. Done for `Play_Move_Normal`.
   - Add a small Blueprint or C++ helper to post a Wwise event at world position.
   - Use one placeholder cue for all events until routing is stable.

3. Movement, weapon, and impact audio. C++ hooks are expanded; Wwise assignment and verification are in progress.
   - Movement wave emission is connected and now chooses normal/sprint/silent event types.
   - Successful weapon fire now posts weapon-mode event types at the muzzle.
   - Projectile impact now posts an impact event type at the collision point.
   - Next: create or reconcile the matching Wwise events, bind them to Blueprint properties, regenerate `Main_Gameplay`, and verify playback in PIE/listen-server tests.

4. Occlusion and distance tuning.
   - Create close/mid/far attenuation sharesets.
   - Add basic occlusion and LPF behaviour.
   - Tune silent walk, sprint, Standard, Rapid, and Snipe as a set.

5. Scan and decoy audio.
   - Hook quick scan and charged scan.
   - Add charge amount RTPC when charged scan is finalised.
   - Decoy actor C++ first pass exists; next add a dedicated decoy event, bind the asset, and verify map ping plus false-source readability.

6. Wwise Spatial Audio rooms/portals.
   - Start with `DM_Tian` for simple validation.
   - Move to `DM_EchoAtrium` after the simple map is readable.
   - Author only major rooms and portals first.

7. Early reflection pass.
   - Add selected geometry in the main test map.
   - Enable reflection only on sounds that benefit from it.
   - Keep weapon direct sound dominant.

8. Mix and readability playtest.
   - Test with at least two clients.
   - Tune action identity, distance, and occlusion before replacing placeholder assets with final sounds.

## Debug and Tuning Checklist

During implementation, add temporary debug output for:

- sound event type
- world position
- audible radius
- loudness
- occlusion amount
- Wwise event name
- owning actor
- whether playback is owner-local, multicast, or server-triggered

Gameplay test scenarios:

- One player stands still while another walks, sprints, and silent-walks around a wall.
- One player fires each weapon from direct line of sight, behind one wall, and through a doorway.
- Projectile impacts occur near, far, and behind geometry.
- Two players sprint and fire at the same time to check mix masking.
- `DM_Tian` remains readable with two players.
- `DM_EchoAtrium` remains readable with vertical routes and central atrium reflections.

Acceptance criteria:

- Every listed gameplay action triggers exactly one intended audio event.
- Weapon fire and impact are distinguishable.
- Sprint is easier to locate than normal movement.
- Silent walk is quieter but still audible nearby.
- Direct sounds are clearer than occluded sounds.
- Doorways/portals preserve believable direction.
- Early reflections add space without hiding the direct source.
- Gameplay-critical sounds remain audible over ambience and music.

## First-Pass Tuning Defaults

These are starting points only.

| Event | Loudness | Audible radius | Localisation clarity | Occlusion sensitivity | Reflection amount |
| --- | --- | --- | --- | --- | --- |
| Normal movement | 0.45 | 2000 | 0.60 | 0.70 | 0.20 |
| Sprint | 0.75 | 2600 | 0.85 | 0.65 | 0.30 |
| Silent walk | 0.20 | 800 | 0.35 | 0.85 | 0.05 |
| Quick scan | 0.45 | 1200 | 0.65 | 0.70 | 0.25 |
| Charged scan | 0.55-1.00 | charge-scaled | 0.75 | 0.65 | 0.35 |
| Standard fire | 0.70 | 900 | 0.85 | 0.60 | 0.20 |
| Rapid fire | 0.55 | 750 | 0.75 | 0.65 | 0.15 |
| Snipe fire | 1.00 | 1700 | 0.95 | 0.55 | 0.30 |
| Projectile impact | 0.65 | radius-scaled | 0.80 | 0.70 | 0.25 |
| Decoy pulse | 0.55 | 1400 | 0.45 | 0.65 | 0.25 |
| Pickup | 0.35 | 900 | 0.55 | 0.75 | 0.10 |
| Shield hit | 0.80 | 1200 | 0.80 | 0.60 | 0.20 |
| Death | 0.75 | 1500 | 0.70 | 0.60 | 0.20 |
| Respawn | 0.50 | 1000 | 0.55 | 0.70 | 0.15 |

## Implementation Notes

- Do not directly edit `.uasset` files outside Unreal Editor.
- Use Blueprint for Wwise event asset assignment and per-map audio authoring.
- Use C++ components for event timing, authority, gameplay state, and shared event data.
- `EchoAudioEventComponent` is the current C++ owner for gameplay audio event posting.
- `MoveNormalEvent` must be assigned in the player Blueprint to `Play_Move_Normal`.
- `MoveSprintEvent`, `MoveSilentEvent`, the three weapon fire events, and `ProjectileImpactEvent` still need assignment to matching Wwise assets.
- Keep audio hooks small and testable.
- Protect gameplay-critical sounds in the mix before adding ambience polish.
- Treat AI/procedural music as future work. For the first demo, Wwise states and RTPCs are enough for adaptive mix behaviour.
