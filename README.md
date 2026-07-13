# BallDarkEcho

BallDarkEcho is a Windows LAN deathmatch built with Unreal Engine 5.7 and Wwise 2025.1.8. Players navigate a low-light arena by reading temporary echo waves and spatial audio, balancing information against the exposure created by movement, scanning, and weapon fire.

本仓库按交付内容分为三部分。UE 工程保留在仓库根目录，以维持 Unreal、Wwise、Content、Config 和 Source 之间的相对路径。

## 1. 演示视频 / Demo video

- [`1_Demo_Video/BallDarkEcho_Final_Demo.mp4`](1_Demo_Video/BallDarkEcho_Final_Demo.mp4)

## 2. 最终文件 / Final build

- [`2_Final_Build/BallDarkEcho_Windows_Development.zip`](2_Final_Build/BallDarkEcho_Windows_Development.zip)
- 解压后运行 `Windows/BallDarkEcho.exe`。
- Windows 64-bit；当前版本面向局域网多人游戏。

## 3. UE 项目与游戏手册 / UE project and game manual

- UE 项目入口：[`BallDarkEcho.uproject`](BallDarkEcho.uproject)
- 游戏手册：[`3_UE_Project_and_Game_Manual/BallDarkEcho_Game_Manual.docx`](3_UE_Project_and_Game_Manual/BallDarkEcho_Game_Manual.docx)
- 简明说明：[`3_UE_Project_and_Game_Manual/README.md`](3_UE_Project_and_Game_Manual/README.md)

### 打开工程

1. 安装 Unreal Engine 5.7、Visual Studio C++ 工具链，以及与项目匹配的 Wwise Unreal Integration 2025.1.8。
2. 将本地 Wwise 插件放入 `Plugins/`；该 SDK 体积较大，不纳入 Git。
3. 打开 `BallDarkEcho.uproject`，需要时重新生成项目文件并编译 `BallDarkEchoEditor Win64 Development`。

### 核心操作

| 按键 | 操作 |
| --- | --- |
| W A S D | 移动 |
| 鼠标 | 视角 / 瞄准 |
| Space | 跳跃 |
| 鼠标左键 | 开火 |
| Shift（按住） | 冲刺 |
| Ctrl（按住） | 静步 |
| Q / 1 / 2 / 3 | 循环或直接选择武器 |
| R | 使用大厅中选择的技能 |
| Tab（按住） | 大地图 |

## 仓库范围

仓库包含运行所需的 UE 源码、配置、Content 资产、Wwise 工程与 Windows SoundBanks。缓存、Intermediate、Saved、IDE 文件、自动备份、Codex 工作数据、未采用的临时资源和原始未精简打包目录均被排除。
