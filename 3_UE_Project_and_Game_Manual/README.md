# UE project and game manual

The Unreal project remains at the repository root so its relative paths stay valid:

- Project: [`../BallDarkEcho.uproject`](../BallDarkEcho.uproject)
- Source: [`../Source`](../Source)
- Game content: [`../Content`](../Content)
- Configuration: [`../Config`](../Config)
- Wwise authoring project: [`../BallDarkEcho_WwiseProject`](../BallDarkEcho_WwiseProject)
- Full game manual: [`BallDarkEcho_Game_Manual.docx`](BallDarkEcho_Game_Manual.docx)

## Quick start

Host and clients must be on the same LAN. The host creates a room, selects the map and skill, and sets the player limit. Clients refresh the room list, join, and select **READY UP**. The host starts the match when the lobby is ready.

The default goal is 10 kills. A defeated player respawns after approximately three seconds while the match remains active. Cyan/blue waves represent movement, scans, impacts, and navigation information; red waves indicate weapon-fire exposure.
