# Smart Vision Workbench v1.0.0

> Copy the section below into [GitHub Releases](https://github.com/yifanmoka-trace/-OpenCV-/releases/new) when publishing **v1.0.0**.

---

## English (paste as release body)

### Smart Vision Workbench v1.0.0 — Initial Open Source Release

**Smart Vision Workbench (SVW)** is an original, MIT-licensed desktop computer-vision workbench built with **Qt 6** and **OpenCV 4**. Chain image processors in a visual pipeline, tune parameters live, compare before/after results, and save your workflow as a reusable project.

<p align="center">
  <img src="https://raw.githubusercontent.com/yifanmoka-trace/-OpenCV-/v1.0.0/docs/screenshots/recognize.png" alt="QR code detection" width="720">
</p>

### Highlights

- **Visual processing pipeline** — add, reorder, enable/disable steps; reprocess with `F5`
- **8 plugin modules, 20+ processors** — edges, filters, morphology, threshold, faces, QR codes, feature matching
- **QR code detection** — standard B/W & colored codes, UTF-8 / Chinese text overlay, WeChat QR DNN fallback
- **Face detection** — Haar cascade + DNN SSD with on-image confidence scores
- **Comparison view** — side-by-side, overlay, and diff modes (`Ctrl+D`)
- **Flexible input** — image files, folder batch, USB camera with FPS display
- **Project files** — save/load `.svw` JSON (pipeline + parameters + image path)
- **Bilingual UI** — English / 简体中文 with localized processor names

### Plugin modules

| Module | Processors |
|--------|------------|
| Color Conversion | Grayscale, HSV Convert |
| Edge Detection | Canny, Sobel, Laplacian |
| Threshold | Binary, Otsu, Adaptive |
| Morphology | Dilate, Erode, Open/Close, Gradient |
| Filtering | Gaussian, Median, Bilateral |
| Face Detection | Haar, DNN SSD |
| Feature Matching | ORB, SIFT |
| QR Detection | QR Code Detection |

### Requirements

- Windows 10/11 (primary), Linux / macOS compatible
- Qt 6.x, OpenCV 4.x, CMake ≥ 3.16, C++17

### Quick start

```bash
git clone https://github.com/yifanmoka-trace/-OpenCV-.git
cd -OpenCV-
scripts\download_dnn_models.bat          # optional — DNN face detection
scripts\download_wechat_qr_models.bat    # optional — colored / WeChat-style QR
cmake -S . -B build -DCMAKE_PREFIX_PATH=C:/Qt/6.x/mingw_64
cmake --build build --config Release
build/src/SmartVisionWorkbench.exe
```

### Documentation

- [README (English)](../README.md)
- [README (中文)](../README.zh-CN.md)

### Third-party components

Qt 6, OpenCV, Haar cascades, DNN SSD face model, WeChat QR models — see README **Third-Party Notices**. Application source code: **MIT License**.

### Known notes

- WeChat personal QR codes typically decode to a `https://u.wechat.com/...` URL (by design).
- Optional DNN / WeChat models are downloaded separately (~11 MB total) and are not required for core features.

---

**Full Changelog**: Initial public release.

---

## 中文（可作为 Release 正文）

### Smart Vision Workbench v1.0.0 — 首次开源发布

**智能视觉工作台（SVW）** 是一款基于 **Qt 6** 与 **OpenCV 4** 的原创桌面视觉处理应用，采用 **MIT** 开源许可证。支持可视化处理管道、实时参数调节、前后对比视图，以及项目保存/加载。

<p align="center">
  <img src="https://raw.githubusercontent.com/yifanmoka-trace/-OpenCV-/v1.0.0/docs/screenshots/screenshot.png" alt="二维码识别" width="720">
</p>

### 主要特性

- **可视化处理管道** — 拖拽排序、启用/禁用步骤，`F5` 实时重处理
- **8 个插件模块、20+ 处理器** — 边缘、滤波、形态学、阈值、人脸、二维码、特征匹配
- **二维码识别** — 标准黑白码 / 彩色码，UTF-8 中文解码，WeChat QR DNN 兜底
- **人脸检测** — Haar 级联 + DNN SSD，显示置信度
- **对比视图** — 并排 / 叠加 / 差异模式（`Ctrl+D`）
- **多种输入** — 图片文件、文件夹批量、USB 摄像头（FPS 显示）
- **项目文件** — `.svw` JSON 保存管道配置与参数
- **双语界面** — 英文 / 简体中文，处理器名称已本地化

### 插件模块

| 模块 | 处理器 |
|------|--------|
| 色彩转换 | 灰度化、HSV 转换 |
| 边缘检测 | Canny、Sobel、Laplacian |
| 阈值分割 | 二值化、Otsu、自适应阈值 |
| 形态学 | 膨胀、腐蚀、开闭运算、梯度 |
| 滤波 | 高斯、中值、双边 |
| 人脸检测 | Haar、DNN SSD |
| 特征匹配 | ORB、SIFT |
| QR 检测 | 二维码检测 |

### 环境要求

- Windows 10/11（主要支持），Linux / macOS 可移植
- Qt 6.x、OpenCV 4.x、CMake ≥ 3.16、C++17

### 快速开始

```bash
git clone https://github.com/yifanmoka-trace/-OpenCV-.git
cd -OpenCV-
scripts\download_dnn_models.bat          # 可选 — DNN 人脸检测
scripts\download_wechat_qr_models.bat    # 可选 — 彩色 / 微信风格二维码
cmake -S . -B build -DCMAKE_PREFIX_PATH=C:/Qt/6.x/mingw_64
cmake --build build --config Release
build/src/SmartVisionWorkbench.exe
```

### 文档

- [README（English）](../README.md)
- [README（中文）](../README.zh-CN.md)

### 第三方组件

Qt 6、OpenCV、Haar 级联、DNN SSD 人脸模型、WeChat QR 模型 — 详见 README **第三方声明**。应用源代码：**MIT License**。

### 说明

- 微信个人二维码解码结果通常为 `https://u.wechat.com/...` 链接，属正常现象。
- DNN / WeChat 可选模型需单独下载（合计约 11 MB），不影响核心功能使用。

---

**完整更新日志**：首次公开发布。

---

## GitHub Release checklist

1. Tag: `v1.0.0`
2. Title: `Smart Vision Workbench v1.0.0 — Initial Release`
3. Body: paste **English** section above (or combine EN + 中文)
4. Attach optional Windows build zip if you have one
5. Verify screenshot URLs after tag is pushed
