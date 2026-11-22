# Constraints — mimic

**Parent:** store-placeable (creature sold as store item). Spec: [Store.md § Mimic](../../../Design/Store.md#mimic), [store_items.md § Mimic](../../../Design/store_items.md#mimic).

## Must honor

- **Disguise:** reads as **Loot Bait** until opened — every real bait is a gamble.
- **Behavior:** stick opener, bite, slow reveal, reset after wave ([Invaders.md](../../../Design/Invaders.md) — `HelperWeight`, `TrustLoot`, `Knows_MimicRisk`).
- **Not in HUD yet:** catalog row without `StoreItem_*` until wired — table uses `—` for #.
- **Price:** design **35** mana reference.

## Brainstorm checklist

- Distinguish from Loot Bait row, tell for scouts, party memory after witness, pairing with multiple real baits.

## Do not

Collapse Mimic into generic lures-and-bait without calling out disguise + stick mechanics.