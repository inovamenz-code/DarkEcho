# BallDarkEcho Audio Source Library

Put raw audio source files here before importing selected assets into Wwise.

Rules:

- Keep raw downloaded/generated files here, not directly in `Content/Audio`.
- Import selected `.wav` files into Wwise with `Import Audio Files...`.
- Track every source in `_licenses/sources.md` before using it in a final demo.
- Prefer dry, short, mono files for gameplay-positioned cues.
- Keep placeholder files until the gameplay hook is verified, then replace one category at a time.

Recommended naming examples:

```text
move_normal_click_01.wav
move_sprint_hard_01.wav
move_silent_soft_01.wav
weapon_standard_fire_01.wav
projectile_impact_default_01.wav
scan_quick_ping_01.wav
```
