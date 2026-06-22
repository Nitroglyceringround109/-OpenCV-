# Smart Vision Workbench · 智能视觉工作台

<p align="center">
  <img src="docs/screenshots/screenshot.png" alt="智能视觉工作台 — 二维码识别" width="900">
</p>

<p align="center">
  <strong>基于 Qt 6 与 OpenCV 的模块化桌面视觉处理工作台</strong><br>
  可视化管道 · 实时摄像头 · 插件化算法 · 项目保存/加载
</p>

<p align="center">
  <a href="README.md">English</a> ·
  <a href="#功能特性">功能特性</a> ·
  <a href="#处理器一览">处理器一览</a> ·
  <a href="#界面截图">界面截图</a> ·
  <a href="#快速开始">快速开始</a> ·
  <a href="LICENSE">MIT 许可证</a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/license-MIT-blue.svg" alt="MIT License">
  <img src="https://img.shields.io/badge/Qt-6-41cd52?logo=qt" alt="Qt 6">
  <img src="https://img.shields.io/badge/OpenCV-4.x-5c3ee8?logo=opencv" alt="OpenCV 4">
  <img src="https://img.shields.io/badge/C%2B%2B-17-00599c?logo=cplusplus" alt="C++17">
  <img src="https://img.shields.io/badge/处理器-22-555" alt="22 Processors">
</p>

---

**Smart Vision Workbench（SVW）** 是一款**原创开源**的桌面图像处理应用，适用于静态图片与**实时摄像头**视觉算法实验。通过可视化处理管道串联算法步骤，实时调节参数，对比处理前后效果，并将整套配置保存为可复用的项目文件。

> **原创项目** — 应用源代码采用 MIT 许可证。第三方模型与依赖库说明见 [第三方声明](#第三方声明)。

---

## 功能特性

| | |
|---|---|
| **可视化管道** | 添加、排序、启用/禁用步骤；逐步耗时统计；`F5` 实时重处理 |
| **22 个处理器 · 8 个插件** | 色彩、边缘、阈值、形态学、滤波、人脸、特征匹配、二维码 |
| **实时摄像头** | USB 摄像头预览，异步管道 + 丢帧保流畅（约 28 ms） |
| **二维码识别** | 标准黑白码 / 彩色码 · UTF-8 中文叠加 · WeChat DNN 兜底 |
| **人脸检测** | Haar 级联 + DNN SSD，图像上显示置信度 |
| **特征匹配** | ORB / SIFT，当前图与参考图之间匹配 |
| **对比视图** | 并排 / 叠加 / 差异模式（`Ctrl+D`） |
| **项目文件** | `.svw` JSON 保存管道配置、参数与图片路径 |
| **国际化** | 英文 / 简体中文，处理器名称与参数均已本地化 |
| **现代界面** | 深色主题、可停靠面板、参数控件自动生成 |

---

## 界面截图

### 主界面

可停靠布局：左侧 **源**（摄像头 / 文件）、中央 **画布**（平移缩放）、右侧 **参数** 与 **历史**、底部 **处理流水线**。

<p align="center">
  <img src="docs/screenshots/main-ui.png" alt="主窗口：画布与停靠面板" width="780">
</p>
<p align="center"><em>主窗口 — 等待加载图片或启动摄像头</em></p>

### 实时摄像头模式

连接 USB 摄像头，叠加处理器，实时查看处理效果。管道在后台异步运行，必要时丢帧以保持界面流畅。
喜欢像素化的用户可以试试调整自适应阙值。

<p align="center">
  <img src="docs/screenshots/xiangsuhua.png" alt="摄像头实时处理 — 自适应阈值" width="780">
</p>
<p align="center"><em>摄像头 0 — DNN 人脸检测 + 二维码检测 + 自适应阈值 · 28 毫秒</em></p>

### 处理流水线

从 8 个插件模块构建多步处理链，逐步启用/禁用，通过分类菜单添加处理器。

<p align="center">
  <img src="docs/screenshots/pipeline.png" alt="处理管道与添加处理器菜单" width="780">
</p>
<p align="center"><em>处理 → 添加处理器 — 流水线示例：DNN 人脸检测 + 膨胀</em></p>

### 二维码识别

定位二维码、绘制绿色边框、解码内容，在画布与状态栏显示 UTF-8 文本（含中文）。

<p align="center">
  <table>
    <tr>
      <td align="center" width="50%">
        <img src="docs/screenshots/screenshot.png" alt="二维码识别 — 中文界面" width="100%"><br>
        <sub><b>简体中文</b></sub>
      </td>
      <td align="center" width="50%">
        <img src="docs/screenshots/recognize.png" alt="QR detection — English UI" width="100%"><br>
        <sub><b>English UI</b></sub>
      </td>
    </tr>
  </table>
</p>
<p align="center"><em>边框标注、解码文字叠加与状态栏 · 微信风格二维码支持 DNN 兜底识别</em></p>

> 微信个人二维码解码结果通常为 `https://u.wechat.com/...` 链接，属正常现象。

### 人脸检测（DNN）

基于 SSD 的人脸检测，可调置信度阈值，每张人脸显示置信度分数。

<p align="center">
  <img src="docs/screenshots/face-detection.png" alt="DNN 人脸检测框选效果" width="780">
</p>
<p align="center"><em>DNN 人脸检测 — 高分辨率照片上的置信度标注</em></p>

### 前后对比

在调试多步流水线时，对比原图与处理结果。

<p align="center">
  <img src="docs/screenshots/comparison.png" alt="前后对比视图" width="780">
</p>
<p align="center"><em>对比视图 — 10 步流水线（高斯模糊选中）· 处理结果 vs 原图</em></p>

---

## 处理器一览

所有处理器由 `plugins/` 目录在运行时加载。选中流水线中的某一步后，右侧 **参数** 面板会自动显示可调项。

### 色彩转换 · Color Conversion

| 处理器 | 说明 |
|--------|------|
| **灰度化** | 将 BGR 彩色图转为单通道灰度图 |
| **HSV 转换** | 将 BGR 转为 HSV 色彩空间，便于按色调/饱和度分析 |

### 边缘检测 · Edge Detection

| 处理器 | 说明 |
|--------|------|
| **Canny 边缘检测** | 经典 Canny 算法，可调高低阈值 |
| **Sobel 边缘检测** | 基于梯度的边缘，支持 X / Y / 双轴 |
| **Laplacian 边缘检测** | 二阶导数边缘增强 |

### 阈值分割 · Threshold

| 处理器 | 说明 |
|--------|------|
| **二值化阈值** | 固定阈值二值分割 |
| **Otsu 阈值** | Otsu 法自动求最优全局阈值 |
| **自适应阈值** | 局部自适应二值化，适合光照不均 — **摄像头场景常用** |

### 形态学 · Morphology

| 处理器 | 说明 |
|--------|------|
| **膨胀** | 扩展亮区域 — 可调核大小、形状、迭代次数 |
| **腐蚀** | 收缩亮区域 |
| **开运算 / 闭运算** | 开运算去噪、闭运算填洞 |
| **形态学梯度** | 膨胀与腐蚀之差 — 轮廓提取 |

### 滤波 · Filtering

| 处理器 | 说明 |
|--------|------|
| **高斯模糊** | 可调核大小与 Sigma 的平滑滤波 |
| **中值模糊** | 去除椒盐噪声 |
| **双边滤波** | 保边平滑 |

### 人脸检测 · Face Detection

| 处理器 | 说明 | 模型 |
|--------|------|------|
| **Haar 人脸检测** | 级联分类器快速检测（正脸 + 侧脸） | 内置 Haar XML |
| **DNN 人脸检测** | Caffe SSD 深度网络，显示置信度 | `scripts\download_dnn_models.bat` |

### 特征匹配 · Feature Matching

需先通过 `文件 → 打开参考图…` 加载参考图像。

| 处理器 | 说明 |
|--------|------|
| **ORB 特征匹配** | 快速二进制特征检测与两图匹配 |
| **SIFT 特征匹配** | 尺度不变特征匹配（需 OpenCV SIFT 模块） |

### QR 检测 · QR Detection

| 处理器 | 说明 | 模型 |
|--------|------|------|
| **二维码检测** | 检测并解码二维码；UTF-8 文字叠加；彩色/微信码兜底 | 可选：`scripts\download_wechat_qr_models.bat` |

**二维码能力：** 标准黑白码 · 彩色/带 Logo 码（多阶段二值化 + 透视矫正）· 微信风格码（`opencv_wechat_qrcode` DNN）· 画布标签 + 状态栏预览。

---

## 环境要求

| 组件 | 版本 |
|------|------|
| **系统** | Windows 10/11（主要支持），Linux / macOS 可移植 |
| **编译器** | C++17（MinGW 64-bit / MSVC / GCC / Clang） |
| **Qt** | 6.x（Widgets、Concurrent） |
| **OpenCV** | 4.x（含 wechat_qrcode 模块） |
| **CMake** | ≥ 3.16 |

---

## 快速开始

### 1. 克隆仓库

```bash
git clone https://github.com/yifanmoka-trace/-OpenCV-.git
cd -OpenCV-
```

### 2. 下载可选模型

| 模型 | 脚本 | 用途 |
|------|------|------|
| DNN 人脸（约 10 MB） | `scripts\download_dnn_models.bat` | DNN 人脸检测 |
| WeChat QR（约 1 MB） | `scripts\download_wechat_qr_models.bat` | 彩色 / 微信风格二维码 |

CMake 构建时会自动复制到 exe 同目录。

### 3. 编译

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=C:/Qt/6.x/mingw_64
cmake --build build --config Release
```

**Qt Creator：** 打开根目录 `CMakeLists.txt` → 选择 Qt 6 套件 → 构建。

### 4. 运行

```
build/src/SmartVisionWorkbench.exe
```

请确保 exe 同目录下有 `plugins/` 与 `models/`。

---

## 使用说明

| 操作 | 快捷键 / 位置 |
|------|---------------|
| 打开图片 | `Ctrl+O` · 左侧源面板 |
| 启动摄像头 | 源面板 → 摄像头 → **启动** |
| 添加处理器 | `处理 → 添加处理器` |
| 调节参数 | 右侧 **参数** 面板 |
| 重新处理 | `F5` |
| 对比视图 | `Ctrl+D` · `视图 → 对比模式` |
| 参考图（特征匹配） | `文件 → 打开参考图…` |
| 切换语言 | `视图 → 语言` |
| 保存项目 | `Ctrl+S` |
| 导出结果 | `文件 → 导出结果…` |
| 快照 | 源面板 · **快照** |

**图片流程：** 打开图片 → 添加处理器 → 调节参数 → `F5` → 导出或保存项目。

**摄像头流程：** 选择摄像头 → **启动** → 添加处理器 → 实时预览 · **快照** 保存当前帧。

---

## 目录结构

```
SmartVisionWorkbench/
├── src/                    # 主程序
│   ├── app/                # 主窗口、应用入口
│   ├── core/               # 管道、插件管理、ImageData
│   ├── capture/            # 摄像头、文件加载、帧抓取
│   ├── ui/                 # 画布、面板、对比视图
│   └── utils/              # 设置、路径、国际化、模型路径
├── plugins/                # 8 个处理器插件（动态库）
├── resources/              # 级联分类器、DNN / WeChat 模型
├── translations/           # Qt 翻译源文件（zh_CN）
├── docs/screenshots/       # README 截图
├── scripts/                # 模型下载脚本
└── CMakeLists.txt
```

---

## 插件开发

插件需实现 `IImageProcessor` 接口（可选 `IPluginFactory`）：

- **加载** — `PluginManager` 运行时扫描 `plugins/` 目录
- **UI** — 通过 `ParameterDescriptor` 自动生成 SpinBox、Slider 等控件
- **执行** — 管道在后台线程顺序调用，摄像头模式支持丢帧
- **本地化** — 处理器名称与描述通过 Qt Linguist 翻译

新增插件：复制现有插件目录 → 实现处理器 → 在根 `CMakeLists.txt` 注册。

---

## 发布打包

```bash
cmake --install build --prefix ./dist
```

Release 说明见 [docs/RELEASE_v1.0.0.md](docs/RELEASE_v1.0.0.md)。

---

## 偏好设置

**编辑 → 偏好设置** 可配置：主题、插件目录、模型目录、摄像头分辨率、历史记录条数、默认对比模式、界面语言。

---

## 第三方声明

| 组件 | 许可 | 说明 |
|------|------|------|
| [Qt 6](https://www.qt.io/) | LGPL / 商业 | GUI 框架 |
| [OpenCV](https://opencv.org/) | Apache 2.0 | 视觉算法库 |
| Haar 级联 | OpenCV | 人脸检测 |
| DNN SSD 人脸模型 | OpenCV 3rdparty | [opencv_3rdparty](https://github.com/opencv/opencv_3rdparty) |
| WeChat QR 模型 | OpenCV 3rdparty | [WeChatCV/opencv_3rdparty](https://github.com/WeChatCV/opencv_3rdparty/tree/wechat_qrcode) |

**本仓库源代码**采用 [MIT License](LICENSE)。
本项目只用于学术研究，未涉及任何商业领域。

---

## 贡献

欢迎 Issue 与 Pull Request。较大改动建议先开 Issue 讨论。

---

## 许可证

MIT License — 详见 [LICENSE](LICENSE)。

---

## 作者

**yifan** — [GitHub @yifanmoka-trace](https://github.com/yifanmoka-trace)

如果这个项目对你有帮助，欢迎点个 ⭐ Star 支持一下。
