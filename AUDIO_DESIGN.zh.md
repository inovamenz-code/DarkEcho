# BallDarkEcho 音频设计

## 目的

音频是 BallDarkEcho 的核心玩法系统，而不是表现层。声音必须帮助玩家定位威胁、判断距离、读取竞技场、识别动作类型，并判断敌人是暴露还是隐藏。

第一版 demo 应优先保证竞技可读性，而不是追求完全真实的物理声学。目标是一个感知上可信的混合系统：Wwise 负责空间播放、距离衰减、遮挡、Rooms/Portals 和选定反射；Unreal 玩法代码从移动、扫描、武器、诱饵、拾取、死亡和重生中发出一致的音频事件。

Steam Audio 不是第一版依赖。Wwise 是计划中的主音频中间件。

## 当前实现状态

截至 2026-06-21：

- Wwise 2025.1.8 已集成到 Unreal 项目。
- Wwise 工程位于 `BallDarkEcho_WwiseProject/BallDarkEcho_WwiseProject.wproj`。
- Generated SoundBanks 输出到 `BallDarkEcho_WwiseProject/GeneratedSoundBanks`。
- `Main_Gameplay` 是当前正式 gameplay SoundBank。
- `Play_Move_Normal` 是第一个正式 gameplay Wwise event。
- `Play_SFX_Echo_Test_Beep` 和 `SoundBankTest1` 只作为验证资产保留，不继续扩展成正式玩法路由。
- `Content/WwiseAudio/Events/Default_Work_Unit/Play_Move_Normal.uasset` 已 reconcile，并能在 Unreal 中播放。
- `EchoAudioEventComponent` 已成为 C++ gameplay 音频入口。
- `EchoMovementPulseComponent::EmitOneMovementWave` 已通过 `EchoAudioEventComponent` post `MovementNormal`。
- 移动 pulse 的 C++ 路径已经通过 UBT 编译，并在 UE 中 playtest 成功。
- C++ 事件枚举和组件属性已扩展到 `MovementSprint`、`MovementSilent`、三种武器开火和 `ProjectileImpact`。
- `EchoMovementPulseComponent` 已根据 normal/sprint/silent 状态选择对应音频事件类型。
- `EchoWeaponComponent` 已在成功生成 projectile 后，从枪口位置 post weapon-mode fire 音频事件类型。
- `EchoSoundProjectileActor` 已在 impact 路径 post `ProjectileImpact` 音频事件类型。
- 这些新增事件还需要在玩家 Blueprint / Wwise 资产中绑定，并逐项 PIE 验证。

当前已验证链路：

```text
EchoMovementPulseComponent
  -> EchoAudioEventComponent::PostEchoSoundEvent(MovementNormal, Location, Loudness)
  -> UAkGameplayStatics::PostEventAtLocation
  -> Wwise Play_Move_Normal
  -> Main_Gameplay SoundBank
  -> PIE 中可听 placeholder beep
```

新增 sprint、silent walk、weapon fire 和 projectile impact hook 已沿用这条链路进入 C++，但还不能视为完整音频功能；下一步是创建/绑定对应 Wwise events、重新生成 SoundBank，并在 PIE/listen-server 下逐项确认播放和重复播放策略。不要再添加零散的 Blueprint-only 音频调用。

## 设计支柱

声音同时承担四个功能：

- 信息：告诉玩家几何体、敌人、射击、命中点、拾取物和诱饵在哪里。
- 暴露：响亮动作会暴露发出动作的角色。
- 威胁：武器和命中声音在完整视觉确认前传达危险等级。
- 导航：空间 ambience 和动作声音帮助玩家理解房间、走廊、开口和垂直路线。

每个重要动作都应产生可读的取舍。更强的声音应该传播更远、更容易定位，并带来更高暴露。安静动作应该降低范围和清晰度，但不能变成完全静音。

## Echo Sound Event 模型

所有 gameplay audio 都应先进入一个共享事件概念，再到 Wwise。第一版不需要复杂系统，但每个音频 hook 都应该能描述同一组数据。

推荐数据字段：

| 字段 | 目的 |
| --- | --- |
| `EventType` | 玩法来源，例如移动、冲刺、扫描、武器开火、命中、诱饵、拾取、重生。 |
| `WorldPosition` | 声音发出的位置。 |
| `Loudness` | 相对强度，用于 Wwise 音量/RTPC 和玩法暴露。 |
| `AudibleRadius` | 有效可听最大范围。通常绑定到现有 wave radius 或 weapon exposure radius。 |
| `LocalizationClarity` | 玩家能多精确地判断方向。冲刺和狙击较高；静步和诱饵尾音较低。 |
| `OcclusionSensitivity` | 墙体和遮挡物对声音变暗/变小的影响强度。 |
| `ReflectionAmount` | 这个声音应接收多少 early reflection 支持。 |
| `VisualWavePolicy` | 这个事件是否同时创建视觉波、地图 ping、两者都有或两者都没有。 |
| `WwiseEventName` | 要 post 的 Wwise event。 |

初始事件枚举候选：

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

事件模型应该成为音频播放、视觉 echo wave 和未来地图 ping 之间的桥梁。不要在无关 Blueprint 里直接加入一次性的音频节点；如果 gameplay component 可以发出结构化事件，就优先让 component 发事件。

## 第一版 Demo 音频需求

### 移动

普通移动应在中距离可读，具备中等定位清晰度和中等暴露。它应支持导航和追踪玩家，但不能压过战斗。

冲刺应比普通移动更响、更宽、更容易定位，并且出现频率更高。它必须清楚传达速度和风险。现有冲刺 wave 默认值可以作为调音来源：间隔 1.2 秒，最大半径 2600，强度 14。

静步应更安静、范围更短、更暗，并且更不精确。它在近处必须仍然可听，这样静步是降低风险，而不是隐身。现有静步 wave 默认值可以作为调音来源：间隔 2.5 秒，最大半径 800，强度 4。

实现 hook：`EchoMovementPulseComponent::EmitOneMovementWave`。

### 扫描

快速扫描应短、锐利、局部。它应该帮助近距离感知，但不能强烈暴露玩家。

蓄力扫描应按蓄力量缩放。更长蓄力应增加可听范围、视觉 wave 强度、反射量和暴露。若后续设计需要反制空间，释放前的蓄力过程也可以发声。

实现 hook：`EchoPulseScannerComponent::TriggerEchoPulseAtLocationWithFrequency`，以及未来的蓄力扫描释放路径。

### 武器

Standard shot 应作为基准攻击身份：中等响度、中等范围、清晰的 projectile/fire 特征，以及可读的枪口位置。

RapidCloseRange 应更短、更锐利，并快速重复。单发应该比 Standard 更轻，但连续射击应制造明显的局部压力。

LongRangeSnipe 应是最响、最清晰的武器开火。它应强烈暴露射手，并传播到足够远，使远处玩家能识别威胁。

实现 hook：`EchoWeaponComponent::FireOnServer`，在 projectile 成功生成后，并在 `MulticastTriggerFireExposureWave` 之前或同时触发。

### Projectile Impact

命中音频应在命中点清楚定位。命中声应不同于开火声，这样玩家能区分射手位置和命中位置。

打到玩家的命中声应比打到静态几何体更锐利、更危险。未来可以把命中玩家路由到单独的 Wwise event 或 switch。

实现 hook：`EchoSoundProjectileActor::HandleOverlap` 和 `MulticastTriggerImpactWave`。

### 诱饵

诱饵音频应足够可信，能延迟敌人决策，但也要允许仔细听的玩家逐渐学会识别。它应该模仿移动/扫描特征，但不能完美复制玩家自身 cue。

第一版：在落点/停止位置触发一次延迟的 `DecoyPulse` 事件，使用中等可听范围、略低定位清晰度，并带有可区分的尾音或质感。

未来 hook：replicated decoy actor activation。

### 拾取物和 Buff

拾取音频应在本地确认拾取，并为附近敌人创建一个较小的世界 cue。Overcharge、Silent 和 Shield 可以共用基础 pickup cue，再用 switch-specific layer 区分。

Buff 激活应对拥有者可读。敌人是否能读到取决于平衡：

- Overcharge 应足够可听，用来警告附近敌人。
- Silent 应主要面向拥有者，外部声音较弱。
- Shield 应有清晰的激活声和命中/吸收声。

未来 hook：multiplayer buff pickup actor/system。

### 死亡和重生

死亡应清楚但不冗长。重生如果会暴露 spawn pressure，就应可识别且具有空间性。避免重生声过响，导致玩家还未获得控制就立刻被惩罚。

实现 hook：`EchoCombatComponent` 的死亡/重生状态变化。

## 当前 Gameplay Audio Hook 对照

| 系统 | 当前职责 | 音频 hook |
| --- | --- | --- |
| `EchoMovementPulseComponent` | 发出普通/冲刺/静步视觉 movement waves。 | 当前根据移动状态 post `MovementNormal`、`MovementSprint` 或 `MovementSilent`。新增 sprint/silent 还需要 Wwise event asset 绑定和 PIE 验证。 |
| `EchoWeaponComponent` | 处理武器模式、开火 authority、projectile spawn、红色 fire wave。 | projectile 成功生成后，在枪口位置 post `WeaponStandardFire`、`WeaponRapidFire` 或 `WeaponSnipeFire`。需要 Wwise event asset 绑定和 mix/timing 验证。 |
| `EchoSoundProjectileActor` | 处理 replicated projectile movement、damage、impact wave。 | impact 路径在碰撞点 post `ProjectileImpact`。需要 impact Wwise event asset 绑定，并确认 owning/client 不重复播放。 |
| `EchoPulseScannerComponent` | 发出 scan pulse 和 surface hit callbacks。 | 在 scan origin 发出 quick/charged scan event。 |
| `EchoCombatComponent` | 处理 health、death、respawn。 | 发出 death、damage、shield hit、respawn events。 |
| `EchoDecoyActor` / `EchoSkillComponent` | C++ first pass 已能生成诱饵、触发范围、伤害和周期 noise wave。 | 当前仍应补专用 `DecoyPulse` Wwise event 和 map ping；不要长期复用 movement placeholder。 |
| Future pickup actor/system | 应用临时 buff。 | 发出 pickup 和 buff activation events。 |

第一版现在已经使用 `EchoAudioEventComponent` 作为共享 C++ 入口。后续 gameplay systems 都应该通过这个组件 post 声音，这样事件时机、cooldown、replication policy、debug drawing、RTPC 值和未来 occlusion/reflection 值都能集中管理。

## Wwise 项目结构

### Events

当前 Wwise event 命名使用简短动作名，不额外加 `Echo` 前缀。Work Unit 和 Event 必须区分：Work Unit 使用 `WU_Movement` 这类分组名；Event 使用 `Play_*`。

当前已验证 event：

```text
Play_Move_Normal
```

接下来要创建的正式 placeholder routing：

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

先使用 placeholder audio。第一版里系统比最终素材更重要。不要把 Work Unit 命名成和 Event 一样；Wwise 会自动生成 `Play_Move_Normal1` 这种后缀，导致 UE asset 查找和文档命名不一致。

### Switches 和 States

使用 switches 表示身份变体：

- `WeaponMode`: `Standard`, `RapidCloseRange`, `LongRangeSnipe`。
- `MovementMode`: `Normal`, `Sprint`, `Silent`。
- `SurfaceType`: 如果之后有 physical materials，则使用 `Default`, `Metal`, `Concrete`, `Glass`, `Soft`。
- `HitType`: `World`, `Player`, `Shield`。

谨慎使用 states 做全局混音：

- `MatchState`: `Menu`, `Playing`, `Dead`, `Result`。
- `PlayerState`: `Normal`, `LowHealth`, `Respawning`。

### RTPCs

推荐 RTPC：

| RTPC | 范围 | 用途 |
| --- | --- | --- |
| `Echo_Loudness` | 0-1 | 缩放音量和强度。 |
| `Echo_AudibleRadius` | Unreal units 或 normalized | 选择 attenuation/falloff 行为。 |
| `Echo_LocalizationClarity` | 0-1 | 控制 stereo width、瞬态清晰度或滤波。 |
| `Echo_OcclusionAmount` | 0-1 | 控制音量衰减和 LPF。 |
| `Echo_Exposure` | 0-1 | 表示声音暴露程度的 mix/gameplay 值。 |
| `Echo_ChargeAmount` | 0-1 | 缩放 charged scan layers。 |
| `Echo_ReflectionAmount` | 0-1 | 控制 early reflection send。 |

第一版只需要 `Echo_Loudness`、`Echo_AudibleRadius` 和 `Echo_OcclusionAmount`。

### Attenuation Sharesets

为可读性创建独立 attenuation sharesets：

- `ATT_Echo_Close`: 静步、拾取、shield hit。
- `ATT_Echo_Mid`: 普通移动、Standard fire、quick scan。
- `ATT_Echo_Far`: 冲刺、charged scan、snipe fire。
- `ATT_Echo_Impact`: projectile impact。
- `ATT_Echo_Decoy`: decoy pulse。

spot sounds 优先使用 logarithmic 或 natural-style falloff。除非是刻意的全局 match feedback，否则避免扁平的长距离声音。

### Busses

当前 Wwise 中已创建的 bus 布局：

```text
Master Audio Bus
  Gameplay_SFX
  Player_Feedback
  Enemy_Cues
  UI
  Ambience
  Reverb_Reflection
```

后续 routing 变细后的目标 bus 布局：

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

将 gameplay-critical SFX 与 ambience 和 music 分离，这样它们能在混音中被保护。

## 音频资源寻找清单

项目可以一边使用 placeholder sounds 推进 gameplay logic，一边逐步替换资源。只有当某个 event hook 和 Wwise routing 稳定后，再替换对应资源。

第一版 demo 需要的资源类别：

| 类别 | 需要资源 | 备注 |
| --- | --- | --- |
| 普通移动 | 3-6 个短 pulse/类似脚步 click | 中距离、瞬态清楚、不要太有音高。 |
| 冲刺移动 | 3-6 个更锐利/更响的变体 | 比普通移动更有 attack 和传播感。 |
| 静步移动 | 3-6 个 muted soft variants | 只服务近距离；更暗、更不精确。 |
| Standard 武器 | 2-4 个 fire layers，可选尾音 | 枪口身份要清楚。 |
| Rapid 武器 | 3-6 个短促轻量 shot | 高频重复时不能刺耳。 |
| Snipe 武器 | 1-3 个响亮高辨识 shot | 强暴露、远距离身份明确。 |
| Projectile impact | 3-6 个 impact | 之后区分 world hit 和 player/shield hit。 |
| Quick scan | 1-3 个短 pulse | 干净 attack，快速衰减。 |
| Charged scan | charge loop/riser + release | 后续接 `Echo_ChargeAmount` RTPC。 |
| Decoy pulse | 1-3 个欺骗性 pulse | 类似移动/扫描，但要有可学习的尾音差异。 |
| Pickup/buff | pickup、overcharge、silent、shield | 先 owner confirmation，再 world cue。 |
| Death/respawn | 短 death cue、respawn cue | 清楚但不能长。 |
| Ambience | 低密度 arena beds | 不能遮住脚步、武器和扫描。 |

选资源规则：

- gameplay 定位 cue 优先找干声 mono source；3D 定位和 reverb 交给 Wwise。
- movement 和 weapon transient 不要用带长 baked reverb 的素材。
- placeholder 文件按意图命名，例如 `move_normal_click_01.wav`，不要保留素材网站原始乱名。
- 每个下载或生成的资源都要记录 license/source，再进入最终 demo。
- 原始声音文件先放在 `.uasset` 流程外，选定后再通过 Wwise import。
- 每次只替换一个 event 类别，并重新 generate `Main_Gameplay`。

## Spatial Audio、遮挡和反射

### 定位

除非有明确理由使用 stereo，否则 gameplay-critical cues 应该是 mono 3D sources。玩家必须能为移动、武器开火、命中、诱饵、拾取、死亡和重生定位到一个精确点。

如果可用，耳机测试使用 binaural/HRTF rendering。第一版可以接受 generic HRTF；individualised HRTF 对本项目不现实。

### 遮挡

遮挡应是感知上可信且可控的：

- 直线路径可见：清晰、明亮、精确。
- 一堵墙遮挡：降低音量、降低 LPF cutoff、略微降低定位清晰度。
- 多个遮挡物/厚几何体：更强衰减、更暗。
- 门口/portal：声音应该从开口保持可读，而不是直接消失。

第一版可以使用 Wwise/Unreal 的 obstruction 和 occlusion 值。如果需要自定义逻辑，从 listener 到 source 做 trace，并把 `Echo_OcclusionAmount` 发送给 Wwise。

### Rooms 和 Portals

使用 Wwise Spatial Audio Rooms/Portals 做手工 authoring 的地图可读性。这对 `DM_EchoAtrium` 尤其重要，因为堆叠环路、中央 atrium、楼梯和侧路很容易变得混乱。

第一版只标记主要 acoustic spaces：

- central atrium
- outer loop rooms/corridors
- stair/vertical route openings
- major doorways or gaps

早期不要给每一小段墙做过度 authoring。目标是可读传播，不是穷尽模拟。

### Geometry 和 Early Reflections

只为选定的大型表面使用 Wwise Geometry：

- central atrium walls
- large flat arena blockers
- major metal/concrete surfaces
- 能帮助距离判断的 long corridors

反射应支持定位和空间感。它不能掩盖直接武器或移动 cue。先用低 reflection amount，只有 playtest 仍然可读时再提高。

## 与 Visual Echo 和 Map Pings 的关系

音频、视觉 waves 和地图 pings 通常应该来自同一个 Echo Sound Event，但它们不需要同一个半径。

推荐规则：

- 音频范围通常长于视觉 wave 范围。
- 视觉 wave 更适合读取几何体。
- 音频更适合快速威胁检测。
- 地图 pings 是延迟/渐隐摘要，不是永久 radar。

示例：

| Event | Audio | Visual wave | Map ping |
| --- | --- | --- | --- |
| 普通移动 | 中等 | 中等 blue/cyan | 无，或短暂 self-only memory update |
| 冲刺 | 远且清晰 | 大型 blue/cyan | 可能产生 temporary loud-source ping |
| 静步 | 短且暗 | 小型 blue/cyan | 无 enemy ping |
| 武器开火 | 远且锐利 | 红色 fire wave | Attack ping |
| 命中 | 中等 | Blue/cyan impact wave | Impact ping |
| 诱饵 | 中等、略可疑 | Decoy pulse | Decoy-like ping 或 false attack ping |
| 拾取 | 短/中等 | 可选小 reveal | 如果已发现则 pickup ping |

## 多人规则

Server-authoritative gameplay 应决定事件是否发生。音频播放可以根据事件类型使用 multicast 或本地预测。

推荐第一版策略：

- 移动和开火使用 owner-local immediate feedback，让控制手感及时。
- 武器开火、projectile impact、死亡、重生、拾取和诱饵使用 server/multicast confirmation。
- multicast 到达时避免 owning client 重复播放。
- 除非声音本身需要持续位置跟踪，否则不要 replicate 长生命周期 audio actors。

对于 one-shot events，优先在 location post Wwise event。对于 looping/continuous sources，将 audio component attach 到 actor。

## 实现顺序

1. Wwise integration sanity check。Editor PIE 已完成。
   - 确认 Wwise plugin 已安装并启用。
   - 确认一个 placeholder event 可以在 Unreal 中播放。
   - 确认 soundbanks 在 editor 和 packaged/listen-server tests 中加载。

2. Placeholder 3D event playback。`Play_Move_Normal` 已完成。
   - 添加一个小型 Blueprint 或 C++ helper，用于在 world position post Wwise event。
   - 路由稳定前，所有事件先用一个 placeholder cue。

3. 移动、武器和命中音频。C++ hook 已扩展，Wwise 绑定/验证进行中。
   - movement wave emission 已接通，当前根据状态选择 normal/sprint/silent event type。
   - successful weapon fire 已在 muzzle post weapon-mode event type。
   - projectile impact 已在 collision point post impact event type。
   - 下一步：创建或 reconcile 对应 Wwise events，绑定到 Blueprint properties，重新 generate `Main_Gameplay`，并在 PIE/listen-server 下确认播放。

4. 遮挡和距离调音。
   - 创建 close/mid/far attenuation sharesets。
   - 添加基础 occlusion 和 LPF 行为。
   - 把 silent walk、sprint、Standard、Rapid 和 Snipe 作为一组一起调。

5. 扫描和诱饵音频。
   - hook quick scan 和 charged scan。
   - charged scan 完成后添加 charge amount RTPC。
   - decoy actor C++ first pass 已存在；下一步添加专用 decoy event、绑定 asset、确认 map ping 与 false-source 读感。

6. Wwise Spatial Audio rooms/portals。
   - 先从 `DM_Tian` 开始做简单验证。
   - 简单地图可读后再迁移到 `DM_EchoAtrium`。
   - 第一版只 author major rooms 和 portals。

7. Early reflection pass。
   - 在主要测试地图中添加选定 geometry。
   - 只在真正受益的声音上启用 reflection。
   - 保持 weapon direct sound 占主导。

8. Mix 和 readability playtest。
   - 至少用两个 clients 测试。
   - 在替换最终素材前，先调整 action identity、distance 和 occlusion。

## Debug 和调音检查表

实现期间添加临时 debug 输出：

- sound event type
- world position
- audible radius
- loudness
- occlusion amount
- Wwise event name
- owning actor
- playback 是 owner-local、multicast 还是 server-triggered

Gameplay 测试场景：

- 一个玩家站着不动，另一个玩家在墙后 walk、sprint、silent-walk。
- 一个玩家分别在 direct line of sight、一堵墙后、门口后方发射每种武器。
- Projectile impacts 出现在近处、远处和几何体后方。
- 两个玩家同时 sprint 和 fire，检查 mix masking。
- `DM_Tian` 在两名玩家下保持可读。
- `DM_EchoAtrium` 在垂直路线和 central atrium reflections 下保持可读。

验收标准：

- 每个列出的 gameplay action 都准确触发一个预期 audio event。
- 武器开火和命中可区分。
- 冲刺比普通移动更容易定位。
- 静步更安静，但近处仍然可听。
- 直达声音比遮挡声音更清楚。
- Doorways/portals 保留可信方向。
- Early reflections 增加空间感，但不掩盖 direct source。
- Gameplay-critical sounds 在 ambience 和 music 上方仍然可听。

## 第一版调音默认值

这些只是起点。

| Event | Loudness | Audible radius | Localization clarity | Occlusion sensitivity | Reflection amount |
| --- | --- | --- | --- | --- | --- |
| 普通移动 | 0.45 | 2000 | 0.60 | 0.70 | 0.20 |
| 冲刺 | 0.75 | 2600 | 0.85 | 0.65 | 0.30 |
| 静步 | 0.20 | 800 | 0.35 | 0.85 | 0.05 |
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

## 实现备注

- 不要在 Unreal Editor 外直接编辑 `.uasset` 文件。
- 使用 Blueprint 分配 Wwise event asset 和做每张地图的音频 authoring。
- 使用 C++ components 处理事件时机、authority、gameplay state 和共享事件数据。
- `EchoAudioEventComponent` 是当前 C++ gameplay audio event posting 的负责组件。
- `MoveNormalEvent` 必须在玩家 Blueprint 中绑定到 `Play_Move_Normal`。
- `MoveSprintEvent`、`MoveSilentEvent`、三种 weapon fire event 和 `ProjectileImpactEvent` 需要继续绑定到对应 Wwise assets。
- 保持 audio hooks 小而可测试。
- 在添加 ambience polish 前，先保护 gameplay-critical sounds 的混音位置。
- AI/procedural music 作为未来工作。第一版 demo 用 Wwise states 和 RTPCs 做 adaptive mix behavior 已经足够。
