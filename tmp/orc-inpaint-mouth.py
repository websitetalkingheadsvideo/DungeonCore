from __future__ import annotations

import cv2
import numpy as np
from PIL import Image

in_path = r"E:\Dungeon Core\Design\Reference Images\Orcs\Orc-Elite-v3-closed-mouth.png"
out_path = in_path
arr = np.array(Image.open(in_path).convert("RGB"))
h, w = arr.shape[:2]

y0, y1, x0, x1 = 108, 172, 300, 465
roi = arr[y0:y1, x0:x1]
bright = (roi.sum(axis=2) > 420) & (roi[:, :, 0] > 140)
bright = bright.astype(np.uint8) * 255
bright = cv2.dilate(bright, np.ones((3, 3), np.uint8), iterations=1)

mask = np.zeros((h, w), np.uint8)
mask[y0:y1, x0:x1] = bright

bgr = cv2.cvtColor(arr, cv2.COLOR_RGB2BGR)
out = cv2.inpaint(bgr, mask, 4, cv2.INPAINT_NS)

Image.fromarray(cv2.cvtColor(out, cv2.COLOR_BGR2RGB)).save(out_path)
print("pass2 px", int(bright.sum() / 255))
