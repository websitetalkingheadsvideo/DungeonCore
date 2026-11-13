import os, re, sys, json, shutil
DST = r'E:\Dungeon Core\DungeonCore\Content'
H = r'H:\Unreal Assets'
ROOTS = {
 'GothicCathedral': H + r'\Gothic_Cathedral_5.4\Content',
 'FantasyDungeon': H + r'\MedievalFantasyDungeon_5.4\MedievalFantasyDungeon_Cinematic\Content',
 'GreenCave': H + r'\Lost_Cave\Content',
 'Crystal_Cave': H + r'\Crystal_Cave\Content',
 'CavernMahal': H + r'\CavernMahal\Content',
 'Mayan_Cave': H + r'\MayanCave\Content',
 'JungleRuins': H + r'\JungleRuins_5.1\JungleRuins_5.1\Content',
}
mv = [H + r'\MedievalVillage_5.0\Content', H + r'\MedievalCastleVillage\Content']
ROOTS['MedievalVillage'] = next((r for r in mv if os.path.exists(r + r'\MedievalVillage\Meshes\House\SM_Door_01.uasset')), mv[0])
WANT = '''GothicCathedral/Meshes/SM_Entrance_Door GothicCathedral/Meshes/SM_Entrance_Module GothicCathedral/Meshes/SM_Altar_A
GothicCathedral/Meshes/SM_Pillar_A FantasyDungeon/Meshes/SM_Chest FantasyDungeon/Materials/MI_Chest FantasyDungeon/Meshes/SM_Cage01
MedievalVillage/Meshes/House/SM_Door_01 MedievalVillage/Meshes/House/SM_Frame_Door_01 GreenCave/Meshes/SM_MineArch
GreenCave/Meshes/SM_LargeArch GreenCave/Meshes/SM_LargeMushrooms_a GreenCave/Meshes/SM_MiddleMushrooms_a
GreenCave/Meshes/SM_SmallMushrooms_a GreenCave/Meshes/SM_GroundMushrooms_a Crystal_Cave/Meshes/SM_Crystalls_A
Crystal_Cave/Meshes/SM_CrystalCluster_D_01 Crystal_Cave/Materials/MI_GlowCrystalls GreenCave/Meshes/SM_SingleCrystal_Blue_a
CavernMahal/Meshes/SM_EntranceArch_01 Mayan_Cave/Meshes/SM_Door_01 Mayan_Cave/Meshes/SM_RubblesKit_01
JungleRuins/Meshes/SM_ChestMd JungleRuins/Materials/MI_ChestMd'''.split()
PAT = re.compile(rb'/Game/[A-Za-z0-9_\-/\.]+')
PAT16 = re.compile(rb'(?:/\x00G\x00a\x00m\x00e\x00/\x00)(?:[A-Za-z0-9_\-/\.]\x00)+')
def refs(path):
    b = open(path, 'rb').read()
    out = set(m.group().decode() for m in PAT.finditer(b))
    out |= set(m.group().decode('utf-16-le') for m in PAT16.finditer(b))
    return set(r.split('.')[0][6:] for r in out)
def closure(root, start):
    seen, todo, missing = set(), [start], set()
    while todo:
        rel = todo.pop()
        if rel in seen: continue
        f = os.path.join(root, rel.replace('/', os.sep) + '.uasset')
        if not os.path.exists(f):
            missing.add(rel); continue
        seen.add(rel)
        todo.extend(refs(f) - seen)
    return seen, missing
plan, report = {}, {}
for w in WANT:
    pack = w.split('/')[0]; root = ROOTS[pack]
    seen, missing = closure(root, w)
    if not seen: report.setdefault('NOT_FOUND', []).append(w); continue
    for rel in seen: plan[rel] = root
    ext = sorted(m for m in missing if not m.startswith(('Engine', 'Script')))
    if ext: report.setdefault('unresolved', {})[w] = ext[:6]
size = 0; conflicts = []; copied = 0
DO = len(sys.argv) > 1 and sys.argv[1] == 'copy'
for rel, root in sorted(plan.items()):
    base = os.path.join(root, rel.replace('/', os.sep))
    for e in ('.uasset', '.uexp', '.ubulk'):
        s = base + e
        if not os.path.exists(s): continue
        d = os.path.join(DST, rel.replace('/', os.sep)) + e
        size += os.path.getsize(s)
        if os.path.exists(d): conflicts.append(rel); continue
        if DO:
            os.makedirs(os.path.dirname(d), exist_ok=True); shutil.copy2(s, d); copied += 1
tops = {}
for rel in plan: tops[rel.split('/')[0]] = tops.get(rel.split('/')[0], 0) + 1
report.update({'assets': len(plan), 'MB': round(size / 1e6, 1), 'top_folders': tops, 'conflicts': sorted(set(conflicts))[:15], 'copied_files': copied, 'village_root': ROOTS['MedievalVillage']})
print(json.dumps(report, indent=1))
