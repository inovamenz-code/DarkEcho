# Project Notes for Codex

This is an Unreal Engine 5 prototype.

Game concept:
A white ball explores a dark sci-fi box-like space. The player uses echo pulses to reveal nearby walls and floors.

Current rules:
- C++ should be used for reusable systems.
- Blueprints should be used for visual setup and quick tuning.
- Do not modify .uasset files.
- Do not rename existing classes unless asked.
- Keep code beginner-friendly.
- Prefer Actor Components.

Current planned systems:
1. EchoRevealComponent: attached to walls/floors.
2. EchoScannerComponent: attached to player ball.
3. Echo visual effect: handled in Blueprint/Niagara.
4. Materials use scalar parameter RevealAmount.