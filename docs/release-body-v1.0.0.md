### Smart Vision Workbench v1.0.0 — Initial Open Source Release

**Smart Vision Workbench (SVW)** is an original, MIT-licensed desktop computer-vision workbench built with **Qt 6** and **OpenCV 4**. Chain image processors in a visual pipeline, tune parameters live, compare before/after results, and save your workflow as a reusable project.

![QR code detection](https://raw.githubusercontent.com/yifanmoka-trace/-OpenCV-/v1.0.0/docs/screenshots/recognize.png)

### Highlights

- **Visual processing pipeline** — add, reorder, enable/disable steps; reprocess with `F5`
- **8 plugin modules, 22 processors** — edges, filters, morphology, threshold, faces, QR codes, feature matching
- **Live camera mode** — USB camera with async pipeline (~28 ms)
- **QR code detection** — standard B/W & colored codes, UTF-8 / Chinese text overlay, WeChat QR DNN fallback
- **Face detection** — Haar cascade + DNN SSD with on-image confidence scores
- **Comparison view** — side-by-side, overlay, and diff modes (`Ctrl+D`)
- **Bilingual UI** — English / 简体中文 with localized processor names

### Quick start

```bash
git clone https://github.com/yifanmoka-trace/-OpenCV-.git
cd -OpenCV-
scripts\download_dnn_models.bat          # optional
scripts\download_wechat_qr_models.bat    # optional
cmake -S . -B build -DCMAKE_PREFIX_PATH=C:/Qt/6.x/mingw_64
cmake --build build --config Release
```

**Docs:** [README (English)](README.md) · [README (中文)](README.zh-CN.md)

**License:** MIT
