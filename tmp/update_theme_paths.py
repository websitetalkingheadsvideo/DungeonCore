import io
p = r'E:\Dungeon Core\Design\Theme.md'
s = io.open(p, encoding='utf-8').read()
n = {}
for pack in ['GothicCathedral', 'FantasyDungeon', 'MedievalVillage', 'GreenCave', 'Crystal_Cave', 'CavernMahal', 'Mayan_Cave', 'JungleRuins']:
    a, b = '`/Game/%s/' % pack, '`/Game/Environments/%s/' % pack
    n[pack] = s.count(a); s = s.replace(a, b)
R = [
 ('Library paths (on `H:\\Unreal Assets`, not migrated):', 'Migrated from `H:\\Unreal Assets` on 2026-11-12, with their materials and textures:'),
 ('Rows marked **H:** are on `H:\\Unreal Assets` and are not in the project yet. Where the cell says **none**, that mesh does not exist on H: either.',
  'Rows marked **H:** came from `H:\\Unreal Assets`. The meshes listed under "Migrated from" below are now in the project under `Environments/<pack>`. Alternates mentioned in passing (Viking `SM_Chest_01`, `SM_Entrance_Door_Big_Asset`, Lost Cave ivy) were not migrated. Where the cell says **none**, that mesh does not exist on H: either.'),
 ('`SM_LargeMushrooms_a` (Lost Cave, not in the project yet).', '`SM_LargeMushrooms_a` (Lost Cave, now in the project).'),
 ('The green source is Lost Cave `SM_LargeMushrooms_a`, still on H:.', 'The green source is Lost Cave `SM_LargeMushrooms_a`, now in the project.'),
 ('In-project `SM_Gate` until this is migrated', 'In-project `SM_Gate` is the fallback'),
]
for a, b in R:
    n[a[:30]] = s.count(a); s = s.replace(a, b)
io.open(p, 'w', encoding='utf-8', newline='').write(s)
print(n)
