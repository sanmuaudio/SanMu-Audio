# SanMu Audio — Open-Source Mixing Plug-ins (JUCE + AI Agent Co-Created)

四款日常混音效果器开源代码库（**VST3 / AU / AAX Ready**），完全借助多个 AI Agent 与大模型协同构建，欢迎下载免费体验、二次开发、定制自己的界面皮肤与 DSP 效果！

- 官网下载体验 & 投票共创：**[www.sanmuaudio.com](https://www.sanmuaudio.com)**
- 抖音主页：**[SanMu Audio on Douyin](https://v.douyin.com/hYrgb-th21I/)**
- 小红书主页：**[SanMu Audio on Xiaohongshu](https://xhslink.cn/o/4oL4WLDXR40)**

---

## 包含插件

| 插件 | 目录 | 核心定位 |
|---|---|---|
| **SanMu EQ** | `SanMu EQ/` | 最多 24 频段动态参量均衡，支持 Pre/Post、Mid/Side、外部侧链、Zero Latency & Linear Phase |
| **SanMu Comp** | `SanMu Comp/` | LEV / VCA / FET / Opto 四模式总线与人声动态压缩器，6s 双通道时域监视与智能分析 |
| **SanMu Delay** | `SanMu Delay/` | Modern / Tape / BBD / Diffused 四引擎立体声延迟，Independent / Ping-Pong / Cross 路由与 Ducking |
| **SanMu Reverb** | `SanMu Reverb/` | Plate / Room / Hall / Chamber 四类算法空间混响，可视化声场预览 |

---

## 从源码构建（macOS / Windows）

> 本仓库通过 git submodule 引用 [JUCE](https://github.com/juce-framework/JUCE)（`external/JUCE`）。构建前请确保已安装 CMake >= 3.25 与 Node.js（用于打包各插件内嵌 Web UI 资源）。

### 1. 拉取仓库与 JUCE 子模块

```bash
git clone --recursive https://github.com/sanmuaudio/SanMu-Audio.git
cd SanMu-Audio
```

若已克隆但未初始化子模块：

```bash
git submodule update --init --recursive
```

### 2. macOS 构建（Universal 2 · VST3 & AU）

以 **SanMu EQ** 为例（其余三款替换对应目录与 Target 名称：`SanMuComp` / `SanMuDelay` / `SanMuReverb`）：

```bash
cmake -S "SanMu EQ" -B "SanMu EQ/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "SanMu EQ/build" --config Release --target SanMuQ6_VST3 SanMuQ6_AU -j 8
```

产物位于 `SanMu EQ/build/source/SanMuQ6_artefacts/Release/{VST3,AU}/`。

### 3. Windows x64 构建（VST3）

Windows 需安装 Visual Studio 2022（C++ 桌面工作负载）、Node.js 与 Microsoft WebView2 NuGet SDK。仓库已附带一键打包脚本：

```powershell
nuget install Microsoft.Web.WebView2 -Version 1.0.3485.44 -OutputDirectory C:\SanMuDependencies -Source https://api.nuget.org/v3/index.json -NonInteractive
.\packaging\windows\build_sanmu_windows.ps1 -Products All -WebView2PackageLocation C:\SanMuDependencies
```

> 注：因 Avid AAX SDK 受保密协议限制未包含在本公开仓库内；当本地不存在 `aax-sdk-2-9-0/` 时，CMake 会自动跳过 AAX 目标，完整构建 **VST3** 与 **AU**（macOS）。

---

## 用 AI Agent 一起魔改与共创

每款插件采用清晰的 **C++ DSP + HTML/CSS/JS WebBrowserComponent UI** 架构，非常适合配合 Codex、Claude Code、Antigravity、Hermes 等 AI Agent 快速实验你的新想法：

- 界面配色、布局与交互逻辑直接位于各插件目录的 `ui/`（`SanMu Reverb` 位于 `SanMu Reverb/prototypes/particle-life/src/`），兼容现代 WebView2 与旧版 macOS Catalina Safari 13.1（共享兼容运行时位于 `packaging/web/`）。
- 音频算法位于各插件 `source/dsp/`。
- 欢迎通过 Issue / Pull Request 提交你的改进、主题皮肤或新 DSP 思路，也欢迎去 [www.sanmuaudio.com/#vote](https://www.sanmuaudio.com/zh/#vote) 参与下一款插件开发投票！

---

## 开源协议

本项目基于 **GNU Affero General Public License v3.0 (AGPLv3)** 开源（见 [LICENSE](./LICENSE)）：你可以免费使用、学习、按自己的风格魔改分享，如基于本代码公开发布或分发衍生版本，请遵循 AGPLv3 保持相应源码开放。
