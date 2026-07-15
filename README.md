# BallDarkEcho

BallDarkEcho is a Windows LAN deathmatch built with Unreal Engine 5.7 and Wwise 2025.1.8. Players navigate a low-light arena by reading temporary echo waves and spatial audio, balancing information against the exposure created by movement, scanning, and weapon fire.

The repository is organised into three deliverable sections. The Unreal project remains at the repository root to preserve the relative paths between Unreal, Wwise, Content, Config, and Source.

## 1. Demo video

- [`1_Demo_Video/BallDarkEcho_Final_Demo.mp4`](1_Demo_Video/BallDarkEcho_Final_Demo.mp4)

## 2. Final build

- [`2_Final_Build/BallDarkEcho_Windows_Development.zip`](2_Final_Build/BallDarkEcho_Windows_Development.zip)
- Extract the archive and run `Windows/BallDarkEcho.exe`.
- The current build targets Windows 64-bit LAN multiplayer.

## 3. UE project and game manual

- UE project: [`BallDarkEcho.uproject`](BallDarkEcho.uproject)
- Game manual: [`3_UE_Project_and_Game_Manual/BallDarkEcho_Game_Manual.docx`](3_UE_Project_and_Game_Manual/BallDarkEcho_Game_Manual.docx)
- Quick guide: [`3_UE_Project_and_Game_Manual/README.md`](3_UE_Project_and_Game_Manual/README.md)

### Open the project

1. Install Unreal Engine 5.7, the Visual Studio C++ toolchain, and Wwise Unreal Integration 2025.1.8.
2. Place the local Wwise plug-ins in `Plugins/`. The large SDK installation is intentionally excluded from Git.
3. Open `BallDarkEcho.uproject`. Regenerate project files if required, then build `BallDarkEchoEditor Win64 Development`.

### Core controls

| Input | Action |
| --- | --- |
| W A S D | Move |
| Mouse | Look / aim |
| Space | Jump |
| Left mouse button | Fire |
| Shift (hold) | Sprint |
| Ctrl (hold) | Silent walk |
| Q / 1 / 2 / 3 | Cycle or directly select a weapon |
| R | Use the skill selected in the lobby |
| Tab (hold) | Large map |

## Repository scope

The repository contains the UE source code, configuration, Content assets, Wwise authoring project, and Windows SoundBanks required by the project. Caches, Intermediate, Saved, IDE files, automatic backups, local assistant workspace data, unused temporary resources, and the uncurated package output are excluded.
