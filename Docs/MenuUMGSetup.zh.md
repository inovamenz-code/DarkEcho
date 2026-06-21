# UMG 菜单连接说明

这套菜单的维护方式是：C++ 只负责状态、LAN 房间、按钮事件和数据刷新；UMG 负责位置、大小、颜色、字体、图片和动效。

## 当前状态

截至 2026-06-21：

- `EchoGameInstance` 已有 LAN 房间创建、搜索、加入、离开和菜单状态流的 C++ first pass。
- `EchoLobbyGameMode`、`EchoLobbyGameState`、`EchoLobbyPlayerController` 和 `EchoPlayerState` 已承担房间名、地图选择、最大人数、HOST、READY、开始比赛等 replicated lobby 状态。
- `EchoMainMenuWidget` 已支持登录、模式选择、房间列表、创建房间、加入房间、房间大厅、ready/start、地图按钮、人数调整和设置面板占位。
- `WBP_RoomRow` / `WBP_PlayerRow` 的 C++ parent class 和绑定接口已经准备好。
- `MenuLobby` 当前作为项目启动/default map。

仍需验证：

- 用两个以上 PIE/standalone client 走完整 create -> join -> ready -> start -> travel 流程。
- 确认所有 UMG 控件命名和 Parent Class 与本文一致。
- 确认 host start travel 到目标 battle map 后，玩家 pawn、HUD、GameMode 和 deathmatch 流程仍然正确。

## 需要创建或修改的 WBP

### `/Game/UI/WBP_MainMenu`

Parent Class 设置为 `EchoMainMenuWidget`。

建议根结构：

```text
Root
  Background_DynamicSlot          Overlay，可选，预留动态背景
  MainPanel
    Switcher_Screens             WidgetSwitcher
      Screen_Login
      Screen_ModeSelect
      Screen_RoomList
      Screen_RoomLobby
  Panel_Settings                 Border，默认 Collapsed
```

只要控件名称匹配，C++ 会在 `NativeConstruct` 自动绑定事件，不需要在蓝图里再接 OnClicked。

### 登录页 `Screen_Login`

必须或建议命名：

```text
Input_PlayerId        EditableTextBox
Button_Start          Button
Button_LoginSettings  Button，可选
Button_Quit           Button，可选
```

`Input_PlayerId` 的输入文字会由 C++ 强制设为黑色。你可以把输入框背景做成白色或浅灰。

### 模式选择页 `Screen_ModeSelect`

```text
Button_OpenRoomList   Button
```

右侧随机匹配只做禁用占位，文字写 `Coming Soon` 即可，不需要命名控件。

### 房间列表页 `Screen_RoomList`

```text
Button_BackToMode     Button
Button_RefreshRooms   Button
Button_CreateRoom     Button
Input_RoomName        EditableTextBox
List_RoomRows         ScrollBox 或 VerticalBox
Text_RoomListStatus   TextBlock
```

推荐 `List_RoomRows` 用 `ScrollBox`，以后房间很多时不会撑爆界面。`Button_RefreshRooms` 是主动刷新服务器列表按钮。

### 房间大厅页 `Screen_RoomLobby`

```text
Text_RoomName          TextBlock
Text_SelectedMap       TextBlock
Text_MaxPlayers        TextBlock
Text_LobbyStatus       TextBlock
List_PlayerRows        ScrollBox 或 VerticalBox
Button_MapLevel1       Button
Button_MapLevel2       Button
Button_MapLevelTest    Button
Button_MapBattle1      Button
Button_MapBattle2      Button
Button_MaxPlayersDown  Button
Button_MaxPlayersUp    Button
Button_HostStart       Button
Button_Ready           Button
Button_LeaveRoom       Button
```

地图按钮建议放进一个 `ScrollBox`，但 `Button_HostStart` 和 `Button_LeaveRoom` 不要放进地图滚动区，固定在右侧面板底部。这样地图增多时按钮不会超出框线。

### 设置面板 `Panel_Settings`

```text
Panel_Settings       Border
Slider_SfxVolume     Slider
Slider_MusicVolume   Slider
Text_SfxVolume       TextBlock
Text_MusicVolume     TextBlock
Button_SettingsBack  Button
```

当前音量只是预留接口，C++ 会保存滑条值并更新百分比文字；真正接 Wwise/音频总线时再把 `OnSfxVolumeChanged`、`OnMusicVolumeChanged` 蓝图事件接到音频系统。

## Row Widget

### `/Game/UI/Menu/WBP_RoomRow`

Parent Class 设置为 `EchoRoomRowWidget`。

```text
Text_RoomName       TextBlock
Text_RoomDetails    TextBlock
Button_Join         Button
```

主菜单会自动调用 `SetupRoom` 填充房间名、地图、人数，并自动绑定 Join。

### `/Game/UI/Menu/WBP_PlayerRow`

Parent Class 设置为 `EchoLobbyPlayerRowWidget`。

```text
Text_PlayerId       TextBlock
Text_PlayerState    TextBlock
```

主菜单会自动调用 `SetupPlayer` 填充玩家 ID、HOST、READY、WAITING。

## 视觉建议

- 主背景：黑色或深灰。
- 面板：半透明灰黑。
- 普通文字：白色或浅灰。
- 输入框：白色或浅灰底，输入文字保持黑色。
- 科幻蓝只用于主按钮、刷新按钮、选中状态、状态点、少量分隔线。
- 登录页的 `Background_DynamicSlot` 留空即可，以后可以放视频、材质动画、Niagara 截图或场景捕获图。

## 常见错误

- 如果按钮点击没反应，先检查控件名字是否完全一致。
- 如果创建房间后回登录页，检查 `WBP_MainMenu` 的 Parent Class 是否真的是 `EchoMainMenuWidget`。
- 如果房间行没有 Join 按钮，检查 `WBP_RoomRow` 的 Parent Class 和 `Button_Join` 名字。
- 如果地图列表压住 Start/Leave，把地图按钮放进独立 `ScrollBox`，Start/Leave 固定在滚动区外。
