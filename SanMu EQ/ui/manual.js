// 帮助手册仅叙述已实现的 EQ 功能，独立于宿主授权与音频预览。
const sections = {
  zh: [
    ['频段与曲线', '在响应曲线上单击或双击即可新建频段：≤ 45 Hz 默认为低切（Low Cut），≥ 10 kHz 默认为高架（High Shelf），中间频段默认为 Bell。拖动节点调整频率与增益，使用频率、增益、Q 旋钮精确调整；最多支持 24 段。低切与高切可选 12／24／36／48 dB/倍频程斜率。'],
    ['动态均衡与监听', '开启动态后，向下模式压低超过阈值的信号，向上模式提升低于阈值的有效信号，并保护静音及底噪。范围限制最大增益变化。AUTO 在内部跟随输入，使用温和的比例和不超过 3 dB 的上限；切换到 MANUAL 后可调节阈值并使用完整范围。可为每个动态频段开启 SC，改用 DAW 外部侧链总线输入作为触发源。按住频段耳机键临时独听，左右拖动扫频，松开即退出。'],
    ['音色与线性相位', 'Clean、Tube、Discrete 是三种音色。在线性相位模式下，可在设置中选择日常、高精度、极限；该模式具有处理延迟，宿主应进行延迟补偿。动态均衡仍可使用，音色处理会暂停。'],
    ['操作与设置', '上下拖动旋钮或滚动鼠标调节，按住 Shift 精细微调；双击或按住 Option/Alt 或 Command/Ctrl 单击复位。聚焦旋钮后可用方向键（Shift 精细）及 Home/End 调节，Enter 键入数值。频谱左下角琴键图标可展开钢琴音高参考（默认关闭），点击琴键即可将选中频段吸附到对应音高。A/B 可比较声音，Command/Ctrl+Z 撤销、Shift+Command/Ctrl+Z（或 Ctrl+Y）重做。主题、语言、提示和显示设置会在本机按产品记忆；点击顶部「峰值」按键可捕捉并拖出频谱峰值。'],
    ['预设与内测', '通过顶部预设按钮浏览或保存预设。浏览器预览不处理宿主音频；请在 DAW 中检查声音、自动化、保存恢复及宿主兼容性。']
  ],
  en: [
    ['Bands and curve', 'Select a band by clicking its node or number. Drag a node to change frequency and gain; use Frequency, Gain and Q for precise adjustment. Higher Q narrows the affected range. Add up to 24 bands. Low Cut and High Cut offer 12/24/36/48 dB/oct slopes; Gain is unavailable for cut filters.'],
    ['Dynamic EQ and audition', 'Down reduces signals above the threshold. Up lifts valid signals below it, with protection against boosting silence and noise. Range limits the gain change. AUTO tracks internally with a gentle ratio and a 3 dB ceiling; MANUAL exposes the threshold and full range. Enable SC on any non-cut band to key its dynamic detector from the DAW’s external Sidechain bus. Hold a band’s headphone control to audition, drag sideways to sweep frequency, and release to stop.'],
    ['Character and Linear Phase', 'Clean, Tube and Discrete provide three characters. In Linear Phase, choose Daily, High precision or Extreme in Settings. The mode adds latency that the host should compensate. Dynamic EQ remains available while character processing pauses.'],
    ['Controls and settings', 'New bands created at ≤ 45 Hz default to Low Cut, ≥ 10 kHz to High Shelf, and in between to Bell. Toggle the piano button at the bottom-left to show the musical pitch strip (off by default) and click any key to snap the selected band. Drag knobs or scroll vertically, hold Shift for fine adjustment, and double-click or Option/Alt-click or Command/Ctrl-click to reset. Focus a knob or node for arrow keys (Shift for fine), Home/End, or Enter entry. A/B compares slots; Command/Ctrl+Z undoes and Shift+Command/Ctrl+Z (or Ctrl+Y) redoes. Use the Peaks button in the top modebar to capture and drag spectrum peaks.'],
    ['Presets and testing', 'Use the top preset control to browse or save presets. Browser preview does not process host audio; test sound, automation, state recall and host compatibility in a DAW.']
  ]
};
export function renderManual(container, language) {
  container.replaceChildren();
  for (const [heading, text] of sections[language === 'en' ? 'en' : 'zh']) {
    const section = document.createElement('section');
    const title = document.createElement('h3');
    const body = document.createElement('p');
    title.textContent = heading;
    body.textContent = text;
    section.append(title, body);
    container.append(section);
  }
}
