---
name: firefly-character-prompt
description: >-
  Write Adobe Firefly character prompts under 1024 characters for the Dungeon
  Core pipeline Prompt, Firefly, Hi3D, Character Creator, Unreal. Use when the
  user asks for a Firefly prompt, a character reference sheet, a hand fix, or
  a new intruder or monster 2D ref.
---

# Firefly character prompts

This skill writes the prompt. It does not generate the image. The user runs Firefly.

Pipeline, in order:

1. **This skill** — Firefly prompt, under 1024 characters.
2. **Firefly** — user generates. Save a **new versioned** file under `Design/Reference Images/`. Never overwrite.
3. **`hi3d-image-to-3d`** — GLB from that image.
4. **Character Creator** — visible faces: Hi3D → 3D Wrap (head) → CC. Other humanoids: Hi3D → CC. Then AccuRIG → Unreal. See `Design/Monsters.md` art direction. Do not restate that pipeline inside the prompt.

## Before writing

- Read the matching row in `Design/Monsters.md`.
- Look at the other images in that character's `Design/Reference Images/` folder. Lock palette, background, and costume language to those sheets.
- If the doc and the folder disagree, follow the folder and say so.
- If both are silent, state the assumption in the reply. Do not invent a faction look.

## Deliver

- One fenced prompt, ready to paste.
- The character count on the next line. Count the exact string you deliver, including spaces.
- If the count is 1024 or more, cut costume adjectives. Never cut the hand block.

```powershell
python -c "p = open(r'prompt.txt', encoding='utf-8').read(); print(len(p))"
```

## Every humanoid sheet

- One character. Full body, front view, centered, nothing cropped.
- Strict T-pose: arms straight horizontal at shoulder height. Do not say A-pose.
- Palms facing the camera, empty hands. A weapon in the fist becomes part of the Hi3D mesh. Sheathe it, or write a second prop-only prompt.
- Plain white background, soft studio light, faint floor shadow. Exception: a folder whose existing sheets are dark (Undead) stays dark.
- Grounded stylized realism. End with `not cartoon, not chibi, no text, no watermark`.

## Hands (required)

This block is what made [Invader-Dward_v2.png](../../../Design/Reference%20Images/Invaders/Invader-Dward_v2.png) read as real fingers. Copy it. Do not paraphrase it back into "anatomically correct hands" or "fingers held together."

```
Each hand has exactly five real human fingers: one shorter thumb set apart, plus index, middle, ring, and pinky. Fingers have knuckles, joints, fingernails, and skin creases, slightly separated so each digit reads clearly. No fused fingers, no cone shapes, no mittens, no extra digits, no missing thumbs.
```

Retired wording, do not use: "fingers together like a mannequin", "fingers held together", "clearly separated fingers" with no knuckles or nails. Those lines produced smooth cones.

Palms face the camera so all five digits are visible. AccuRIG needs five fingers.

## New character

Write the full sheet, including the hand block. Name the role in the first words (`T-pose dwarf fighter`). Put identity before flavor: species, build, face, the one costume fact that separates this tier from its siblings.

## Edit (user already has a sheet)

Lock the reference. Change one thing.

```
Same [role] and costume as the reference. Keep [iron cap, beard, vest, bracers, boots], full body front view, white background. Change only the hands. Arms stay out to the sides, palms facing the camera. [hand block]
```

Keep three to six identity locks from the image. Do not restyle the body in a hand fix.

## After the user accepts the image

Append one short line to the matching `Design/Monsters.md` row (ref filename, the costume fact that defines the tier). Do not paste the prompt into that table.
