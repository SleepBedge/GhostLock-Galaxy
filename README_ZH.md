# GhostLock-Galaxy

> English: [README.md](README.md)

本仓库单独维护，是因为当前路线需要 Android shell 权限（通过 `adb shell`
或 Shizuku 获取）。YuKongA 的[原项目](https://github.com/YuKongA/ghostlock-app)
不使用 shell 权限，采用的是另一条执行路线；两者的运行要求和代码路径并不相同，
不能直接互换。

## 当前支持设备

| 设备 | Kernel |
| ---- | ------ |
| Samsung Galaxy Z Fold6（SM-F9560 / q6q） | `6.1.145-android14-11-3254009-abF9560ZCS4DZG3` |
| Samsung Galaxy Z Fold6（SM-F956U1） | `6.1.145-android14-11-33418572-abF956USQS4DZG3` |

启动时按 `uname -r` 精确匹配 offset 表，未匹配的内核会直接拒绝运行。

## 本仓库相对上游的改动

本仓库 fork 自 [wxxsfxyzm/GhostLock-Galaxy](https://github.com/wxxsfxyzm/GhostLock-Galaxy)。
相对上游 `d082c2d`，功能性改动只有 `SM-F956U1` 目标；全部差异以单个补丁保存在
[`docs/upstream-f956u1.patch`](docs/upstream-f956u1.patch)，便于一次性审阅、应用，
或在将来直接向上游提交。该补丁本身不包含 `docs/upstream-f956u1.patch`。

```bash
git apply -p1 docs/upstream-f956u1.patch
```

补丁内容：

- 新增并注册已在真机验证的 `SM-F956U1` / `F956U1UES4DZG3` 目标
  （`src/devices/f956u1-ues4dzg3/offsets.h`、`src/devices/offsets.h`）。
- `src/core/main.c` 在确认已进入 `pselect6` 阻塞后才触发破坏性的
  `sched_setattr`，避免调度延迟导致内核在复制 fd 集合之前就被 punch，
  把普通的超时失误变成重启。
- `src/core/main.c` 将持久化的 punch delay 轮换改为粘性选择：只复用已验证写入
  成功的延迟，未确认成功（含重启）才推进；触发前会先标记为进行中，
  因此崩溃后下一次启动也会继续推进搜索。
- `tools/extract_target.py` 额外兼容 Samsung 设备树的 `MemLabel` 拼写
  （原先只认 Qualcomm 的 `mem-label`）。
- README 记录该移植。移植细节、真机验证结果以及修正的四个 offset 字段见上文。

## 快速开始

APK 路径需要先通过无线调试启动 Shizuku，并授予 GhostLock 权限，然后点击 **执行**。
Shizuku 会以 Android shell 用户启动 payload；APK 自身并不是利用链的执行身份。

## 命令行调试

命令行路径直接运行已验证的 shell payload，不需要 Shizuku：

```powershell
make ghostlock helper
adb push ghostlock /data/local/tmp/ghostlock
adb push ghostlock-helper /data/local/tmp/ghostlock-helper
adb push app/src/main/assets/ksud-zfold6-F9560ZCS4DZG3-samsung-main-no-patch-text-kdp /data/local/tmp/ksud-zfold6-F9560ZCS4DZG3-samsung-main-no-patch-text-kdp
adb shell chmod 755 /data/local/tmp/ghostlock
adb shell chmod 755 /data/local/tmp/ghostlock-helper
adb shell chmod 755 /data/local/tmp/ksud-zfold6-F9560ZCS4DZG3-samsung-main-no-patch-text-kdp
adb shell /data/local/tmp/ghostlock
```

`ghostlock-helper` 是 UMH root 和 KernelSU late-load 阶段必需的辅助程序。

## KernelSU 6.1 内核编译注意事项

在部分 Samsung/Exynos 6.1 内核上，通用 KernelSU 模块会在
`ksud late-load` 初始化时尝试 live text patching，从而触发 EL2 panic。
对于受影响的目标设备，应针对精确固件版本构建模块，并启用目标 KernelSU
代码树提供的 no-patch-text 选项：

```text
CONFIG_KSU_SAMSUNG_NO_PATCH_TEXT=y
```

`kernelsu.ko` 和 `ksud` 应使用同一目标构建的配套版本，并按完整的
`uname -r` 匹配设备，不要只按 `6.1` KMI 判断。late-load 阶段重启不一定是
`ksud` loader 本身导致，也可能是模块初始化时触发的 panic。本项目当前的
SM-F9560 是 Snapdragon，不能直接套用 Exynos 结论，仍需针对目标内核实测。
目标设备的编译示例可参考
[Root-My-Galaxy-Payloads](https://github.com/BuSung-dev/Root-My-Galaxy-Payloads)。

## 偏移量提取

高通设备可用 `tools/extract_target.py` 从 `boot.img` 和 `xbl_config.img` 解析偏移量，依赖 Python 3 及 kallsyms 来源（`--kallsyms` 文件或 `--kallsyms-finder`）。
传入 `--llvm-objdump`（或确保 `llvm-objdump` 在 PATH/NDK 中）会额外反汇编内核，自动推导 `pselect_waiter_shift` 与 `off_slide_loggers_0_1`：

```powershell
python tools/extract_target.py `
  boot.img `
  --xbl-config xbl_config.img `
  --format c `
  --out offsets.h
```

### pselect 路线可行性

`core_sys_select` 只把 3 份 `FDS_BYTES(nfds)` 的用户 fd_set 拷到内核栈（nfds=320 时为 qword 0..14）。futex waiter 必须落在该可控区：waiter 起始字 + 11（lock 字段）≤ 14，即推导 shift（waiter 相对 fd_set 的 qword 偏移）≤ 3，否则 task/lock 落在内核清零区，路线不可行。脚本在推导出不可行布局时会直接报错。

## 更多详情与贡献

payload 研究过程和配套验证材料请参阅
[Root-My-Galaxy-Payloads](https://github.com/BuSung-dev/Root-My-Galaxy-Payloads)。
欢迎提交 PR。

## 来源与许可证

基于以下项目改写，继承 Apache License 2.0（见 [LICENSE](LICENSE)）：

- [NebuSec/CyberMeowfia](https://github.com/NebuSec/CyberMeowfia)
- [JoinChang/ghostlock-oneplus](https://github.com/JoinChang/ghostlock-oneplus)
- [x-spy/CVE-2026-43499-popsicle](https://github.com/x-spy/CVE-2026-43499-popsicle)
- [YuKongA/ghostlock-app](https://github.com/YuKongA/ghostlock-app)
- [BuSung-dev/Root-My-Galaxy](https://github.com/BuSung-dev/Root-My-Galaxy)
