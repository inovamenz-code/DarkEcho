# Project Notes for Codex

## Current Project Status

- Project name: BallDarkEcho.
- Engine: Unreal Engine 5.7.
- This is a prototype for an echo-based exploration game.
- The current player is a white ball, using BP_EchoPulse as the main controllable pawn/blueprint.
- Basic player movement already works.
- The white ball appearance has already been set up.
- Existing useful assets include:
  - M_Env_Black
  - M_Ball_White
  - M_Echo_Reveal_White
  - BP_EchoPulse
  - Level1_AB
- There is currently an EchoPulseActor C++ class, but it is mostly empty/template-like.
- The real reusable echo reveal/scanning system has not been implemented in C++ yet.

## Game Concept

A white ball explores a dark sci-fi box-like space. The player uses echo pulses to reveal nearby walls and floors.

## Desired Visual Style

- Dark sci-fi box-like environment.
- The player should not permanently light the whole wall.
- Echo should appear as visible wave/ripple traces.
- Walls/floors should only be temporarily revealed where echo interacts with them.

Important preference:

- Do not make the whole wall become permanently white.
- Prefer localized visible echo traces or temporary material reveal.
- Keep the first implementation simple and testable.

## Development Rules

- Do not modify .uasset files directly.
- Do not delete or rename existing assets.
- C++ should implement reusable logic.
- Blueprint should handle visual setup, material assignment, Niagara, and tuning.
- Prefer Actor Components instead of large inherited classes.
- Every task should be small, compilable, and testable.
- After code changes, explain exactly what needs to be done in the Unreal Editor.

## Planned Systems

1. EchoRevealComponent: attached to walls/floors.
2. EchoScannerComponent: attached to the player ball.
3. Echo visual effect: handled in Blueprint/Niagara.
4. Materials use scalar parameter RevealAmount.
