// ==============================================================================
// PresetManager.cpp - 预设管理系统实现
// ==============================================================================

#include "PresetManager.h"
#include "../params/UpgradeState.h"

#include <vector>
#include <array>
#include <mutex>

// ==============================================================================
// 工厂预设数据（懒加载 + 线程安全初始化）
// ==============================================================================

static std::vector<PresetData> s_factoryPresets;
static juce::StringArray       s_factoryNames;
static std::once_flag          s_initFlag;

static PresetData createDefaultPreset()
{
    PresetData p;
    p.inputGainL     = 0.0f;
    p.inputGainR     = 0.0f;
    p.outputGainL    = 0.0f;
    p.outputGainR    = 0.0f;
    p.phaseInvert    = false;
    p.processingMode = 0;
    p.characterMode  = 0;
    p.globalBypass   = false;
    p.autoGain       = false;

    for (int i = 0; i < Params::NUM_BANDS; ++i)
    {
        p.present[i]      = false;
        p.enabled[i]      = false;
        p.type[i]         = Params::Defaults::TYPES[i];
        p.freq[i]         = Params::Defaults::FREQUENCIES[i];
        p.gain[i]         = Params::Defaults::GAIN;
        p.q[i]            = Params::Defaults::Q;
        p.slope[i]        = 0; // 12 dB/oct
        p.placement[i]    = Params::PlacementStereo;
        p.solo[i]         = false;
        p.dynamic[i]      = false;
        p.dynAuto[i]      = true;
        p.dynUpward[i]    = false;
        p.dynRange[i]     = 6.0f;
        p.dynThreshold[i] = -24.0f;
    }
    return p;
}

static void setBand(PresetData& p, int idx, int type, float freq, float gain, float q,
                    int slope = 0, int placement = Params::PlacementStereo)
{
    p.present[idx]   = true;
    p.enabled[idx]   = true;
    p.type[idx]      = type;
    p.freq[idx]      = freq;
    p.gain[idx]      = gain;
    p.q[idx]         = q;
    p.slope[idx]     = slope;
    p.placement[idx] = placement;
    p.solo[idx]      = false;
}

static void setBandDyn(PresetData& p, int idx, float range, bool autoThresh = true,
                       bool upward = false, float threshold = -24.0f)
{
    p.dynamic[idx]      = true;
    p.dynAuto[idx]      = autoThresh;
    p.dynUpward[idx]    = upward;
    p.dynRange[idx]     = range;
    p.dynThreshold[idx] = threshold;
}

static void buildFactoryPresets()
{
    // ---- 预设 0：默认（平坦） ----
    {
        PresetData p = createDefaultPreset();
        p.name = "Default";
        s_factoryPresets.push_back(p);
    }

    // ---- 预设 1：人声增强（Vocal Enhancer） ----
    {
        PresetData p = createDefaultPreset();
        p.name = "Vocal Enhancer";
        p.characterMode = 1; // Tube 温润偶次谐波
        p.outputGainL = p.outputGainR = -0.5f;

        setBand(p, 0, Params::LowCut,    80.0f,    0.0f, 0.71f, 0);
        setBand(p, 1, Params::Peak,     220.0f,  -2.2f, 1.2f);
        setBand(p, 2, Params::Peak,    2800.0f,   2.8f, 1.0f);
        setBand(p, 3, Params::Peak,    6500.0f,   0.5f, 2.4f);
        setBandDyn(p, 3, 4.0f, true, false, -24.0f); // 齿音动态控制
        setBand(p, 4, Params::HighShelf, 10000.0f, 2.4f, 0.71f);
        setBand(p, 5, Params::HighCut, 18000.0f,  0.0f, 0.71f);

        s_factoryPresets.push_back(p);
    }

    // ---- 预设 2：低音收紧（Bass Tight） ----
    {
        PresetData p = createDefaultPreset();
        p.name = "Bass Tight";
        p.characterMode = 2; // Discrete 低频紧凑激励

        setBand(p, 0, Params::LowCut,   32.0f,   0.0f, 0.71f, 1);
        setBand(p, 1, Params::Peak,     65.0f,   2.5f, 0.71f);
        setBand(p, 2, Params::Peak,    135.0f,  -2.5f, 1.6f);
        setBand(p, 3, Params::Peak,    420.0f,  -1.5f, 1.1f);
        setBand(p, 4, Params::Peak,   1400.0f,   2.0f, 1.4f);
        setBand(p, 5, Params::HighShelf, 5500.0f, -1.8f, 0.71f);

        s_factoryPresets.push_back(p);
    }

    // ---- 预设 3：空气感（Air & Shine） ----
    {
        PresetData p = createDefaultPreset();
        p.name = "Air & Shine";
        p.characterMode = 1; // Tube 高频泛音光泽
        p.outputGainL = p.outputGainR = -1.2f;

        setBand(p, 0, Params::LowCut,    35.0f,   0.0f, 0.71f);
        setBand(p, 1, Params::LowShelf,  90.0f,   0.8f, 0.71f);
        setBand(p, 2, Params::Peak,    3200.0f,   1.5f, 1.0f);
        setBand(p, 3, Params::Peak,    8500.0f,   2.2f, 0.9f);
        setBand(p, 4, Params::HighShelf, 12500.0f, 3.2f, 0.71f);

        s_factoryPresets.push_back(p);
    }

    // ---- 预设 4：去浑浊（De-Mud） ----
    {
        PresetData p = createDefaultPreset();
        p.name = "De-Mud";

        setBand(p, 0, Params::LowCut,    55.0f,   0.0f, 0.71f);
        setBand(p, 1, Params::LowShelf, 110.0f,   1.2f, 0.71f);
        setBand(p, 2, Params::Peak,     240.0f,  -3.2f, 0.85f);
        setBand(p, 3, Params::Peak,     400.0f,  -1.0f, 1.3f);
        setBandDyn(p, 3, 3.5f, true, false, -24.0f);
        setBand(p, 4, Params::Peak,    3800.0f,   2.0f, 1.1f);
        setBand(p, 5, Params::HighShelf, 10500.0f, 1.5f, 0.71f);

        s_factoryPresets.push_back(p);
    }

    // ---- 预设 5：架子鼓总线（Drum Bus） ----
    {
        PresetData p = createDefaultPreset();
        p.name = "Drum Bus";
        p.characterMode = 2; // Discrete 瞬态粘合
        p.outputGainL = p.outputGainR = -1.5f;

        setBand(p, 0, Params::LowCut,    28.0f,   0.0f, 0.71f);
        setBand(p, 1, Params::Peak,      62.0f,   2.8f, 0.75f);
        setBand(p, 2, Params::Peak,     220.0f,  -2.5f, 1.1f);
        setBand(p, 3, Params::Peak,     750.0f,  -1.5f, 1.4f);
        setBand(p, 4, Params::Peak,    4500.0f,   2.0f, 1.1f);
        setBand(p, 5, Params::HighShelf, 9500.0f, 2.2f, 0.71f);

        s_factoryPresets.push_back(p);
    }

    // ---- 预设 6：母带润色（Master Polish） ----
    {
        PresetData p = createDefaultPreset();
        p.name = "Master Polish";

        setBand(p, 0, Params::LowCut,     24.0f,  0.0f, 0.71f);
        setBand(p, 1, Params::LowShelf,   75.0f,  0.8f, 0.55f);
        setBand(p, 2, Params::Peak,      350.0f, -0.6f, 0.8f);
        setBand(p, 3, Params::Peak,     2200.0f,  0.8f, 0.7f);
        setBand(p, 4, Params::HighShelf, 8000.0f, 1.2f, 0.71f, 0, Params::PlacementSide);
        setBand(p, 5, Params::HighShelf, 13000.0f, 1.0f, 0.71f);

        s_factoryPresets.push_back(p);
    }

    // ---- 预设 7：电话效果（Telephone） ----
    {
        PresetData p = createDefaultPreset();
        p.name = "Telephone";
        p.characterMode = 2;
        p.outputGainL = p.outputGainR = 1.0f;

        setBand(p, 0, Params::HighCut, 3400.0f, 0.0f, 0.71f, 1);
        setBand(p, 1, Params::LowCut,   380.0f, 0.0f, 0.71f, 1);
        setBand(p, 2, Params::Peak,    1100.0f, 3.5f, 0.9f);
        setBand(p, 3, Params::Peak,    2100.0f, 2.0f, 1.1f);

        s_factoryPresets.push_back(p);
    }

    // ---- 预设 8：动态去齿音（Dynamic De-Esser） ----
    {
        PresetData p = createDefaultPreset();
        p.name = "Dynamic De-Esser";

        setBand(p, 0, Params::Peak, 4600.0f, 0.0f, 2.2f);
        setBandDyn(p, 0, 3.0f, true, false, -24.0f);
        setBand(p, 1, Params::Peak, 6800.0f, 0.0f, 2.8f);
        setBandDyn(p, 1, 5.5f, true, false, -24.0f);
        setBand(p, 2, Params::Peak, 9200.0f, 0.0f, 2.0f);
        setBandDyn(p, 2, 3.0f, true, false, -24.0f);

        s_factoryPresets.push_back(p);
    }

    // ---- 预设 9：动态抑浑浊（Dynamic Mud Tamer） ----
    {
        PresetData p = createDefaultPreset();
        p.name = "Dynamic Mud Tamer";

        setBand(p, 0, Params::LowCut,  40.0f,  0.0f, 0.71f);
        setBand(p, 1, Params::Peak,   210.0f, -0.5f, 1.4f);
        setBandDyn(p, 1, 4.5f, true, false, -24.0f);
        setBand(p, 2, Params::Peak,   420.0f, -0.8f, 1.6f);
        setBandDyn(p, 2, 3.2f, true, false, -24.0f);

        s_factoryPresets.push_back(p);
    }

    // ---- 预设 10：向上明亮度提升（Upward Presence Lift） ----
    {
        PresetData p = createDefaultPreset();
        p.name = "Upward Presence Lift";
        p.characterMode = 1;

        setBand(p, 0, Params::LowCut,    60.0f,  0.0f, 0.71f);
        setBand(p, 1, Params::Peak,    2800.0f,  0.5f, 1.1f);
        setBandDyn(p, 1, 3.5f, true, true, -24.0f);
        setBand(p, 2, Params::HighShelf, 10000.0f, 1.0f, 0.71f);
        setBandDyn(p, 2, 3.0f, true, true, -24.0f);

        s_factoryPresets.push_back(p);
    }

    // ---- 预设 11：中侧展宽（Mid-Side Width） ----
    {
        PresetData p = createDefaultPreset();
        p.name = "Mid-Side Width";

        setBand(p, 0, Params::LowCut,   110.0f, 0.0f, 0.71f, 0, Params::PlacementSide);
        setBand(p, 1, Params::LowShelf,  85.0f, 1.2f, 0.71f, 0, Params::PlacementMid);
        setBand(p, 2, Params::Peak,    1800.0f, 1.0f, 1.0f,  0, Params::PlacementMid);
        setBand(p, 3, Params::Peak,    3200.0f, 1.5f, 1.0f,  0, Params::PlacementSide);
        setBand(p, 4, Params::HighShelf, 9000.0f, 2.2f, 0.71f, 0, Params::PlacementSide);

        s_factoryPresets.push_back(p);
    }

    // ---- 预设 12：底鼓冲击（Kick & Sub Punch） ----
    {
        PresetData p = createDefaultPreset();
        p.name = "Kick & Sub Punch";
        p.characterMode = 2;
        p.outputGainL = p.outputGainR = -1.5f;

        setBand(p, 0, Params::LowCut,   28.0f,  0.0f, 0.71f, 1);
        setBand(p, 1, Params::Peak,     55.0f,  3.2f, 1.1f);
        setBand(p, 2, Params::Peak,    180.0f, -3.0f, 1.5f);
        setBand(p, 3, Params::Peak,    420.0f, -1.8f, 1.3f);
        setBand(p, 4, Params::Peak,   2800.0f,  2.5f, 1.3f);
        setBand(p, 5, Params::HighCut, 12000.0f, 0.0f, 0.71f);

        s_factoryPresets.push_back(p);
    }

    // ---- 预设 13：军鼓冲击（Snare Crack & Body） ----
    {
        PresetData p = createDefaultPreset();
        p.name = "Snare Crack & Body";
        p.characterMode = 2;
        p.outputGainL = p.outputGainR = -1.0f;

        setBand(p, 0, Params::LowCut,   90.0f,  0.0f, 0.71f);
        setBand(p, 1, Params::Peak,    185.0f,  2.5f, 1.3f);
        setBand(p, 2, Params::Peak,    480.0f, -2.5f, 1.4f);
        setBand(p, 3, Params::Peak,   3800.0f,  2.8f, 1.1f);
        setBand(p, 4, Params::HighShelf, 9000.0f, 2.0f, 0.71f);

        s_factoryPresets.push_back(p);
    }

    // ---- 预设 14：原声吉他（Acoustic Guitar） ----
    {
        PresetData p = createDefaultPreset();
        p.name = "Acoustic Guitar";
        p.characterMode = 1;

        setBand(p, 0, Params::LowCut,   85.0f,  0.0f, 0.71f);
        setBand(p, 1, Params::Peak,    210.0f, -2.5f, 1.4f);
        setBand(p, 2, Params::Peak,   1800.0f, -1.2f, 1.2f);
        setBand(p, 3, Params::Peak,   5200.0f,  2.0f, 1.1f);
        setBand(p, 4, Params::HighShelf, 11000.0f, 2.5f, 0.71f);

        s_factoryPresets.push_back(p);
    }

    // ---- 预设 15：电吉他清晰度（Electric Guitar） ----
    {
        PresetData p = createDefaultPreset();
        p.name = "Electric Guitar";
        p.characterMode = 2;

        setBand(p, 0, Params::LowCut,   95.0f,  0.0f, 0.71f, 1);
        setBand(p, 1, Params::Peak,    180.0f, -0.5f, 1.4f);
        setBandDyn(p, 1, 3.5f, true, false, -24.0f);
        setBand(p, 2, Params::Peak,    320.0f, -1.8f, 1.3f);
        setBand(p, 3, Params::Peak,   2400.0f,  2.2f, 1.1f);
        setBand(p, 4, Params::HighCut, 8500.0f, 0.0f, 0.71f);

        s_factoryPresets.push_back(p);
    }

    // ---- 预设 16：温暖钢琴与键盘（Warm Piano & Keys） ----
    {
        PresetData p = createDefaultPreset();
        p.name = "Warm Piano & Keys";
        p.characterMode = 1;

        setBand(p, 0, Params::LowCut,   45.0f,  0.0f, 0.71f);
        setBand(p, 1, Params::LowShelf, 120.0f, 1.4f, 0.71f);
        setBand(p, 2, Params::Peak,    360.0f, -1.8f, 1.2f);
        setBand(p, 3, Params::Peak,   2600.0f,  1.5f, 1.0f);
        setBand(p, 4, Params::HighShelf, 9500.0f, 1.8f, 0.71f);

        s_factoryPresets.push_back(p);
    }

    // ---- 预设 17：线性相位母带（Linear Phase Master） ----
    {
        PresetData p = createDefaultPreset();
        p.name = "Linear Phase Master";
        p.processingMode = 1; // Linear Phase
        p.characterMode  = 0;

        setBand(p, 0, Params::LowCut,   22.0f,  0.0f, 0.71f);
        setBand(p, 1, Params::LowCut,   95.0f,  0.0f, 0.71f, 0, Params::PlacementSide);
        setBand(p, 2, Params::LowShelf, 70.0f,  0.8f, 0.6f,  0, Params::PlacementMid);
        setBand(p, 3, Params::Peak,    320.0f, -0.8f, 0.9f);
        setBand(p, 4, Params::Peak,   2500.0f,  0.7f, 0.8f);
        setBand(p, 5, Params::HighShelf, 11000.0f, 1.2f, 0.71f, 0, Params::PlacementSide);
        setBand(p, 6, Params::HighShelf, 14000.0f, 0.8f, 0.71f);

        s_factoryPresets.push_back(p);
    }

    // 构建名称列表
    s_factoryNames.clear();
    for (const auto& preset : s_factoryPresets)
        s_factoryNames.add(preset.name);
}

// ==============================================================================
// 工厂预设查询
// ==============================================================================

void PresetManager::ensureFactoryPresetsBuilt()
{
    std::call_once(s_initFlag, buildFactoryPresets);
}

std::vector<PresetData>& PresetManager::getFactoryPresets()
{
    ensureFactoryPresetsBuilt();
    return s_factoryPresets;
}

int PresetManager::getNumFactoryPresets()
{
    ensureFactoryPresetsBuilt();
    return static_cast<int>(s_factoryPresets.size());
}

const juce::StringArray& PresetManager::getFactoryPresetNames()
{
    ensureFactoryPresetsBuilt();
    return s_factoryNames;
}

const PresetData& PresetManager::getFactoryPreset(int index)
{
    ensureFactoryPresetsBuilt();
    jassert(index >= 0 && index < static_cast<int>(s_factoryPresets.size()));
    return s_factoryPresets[static_cast<size_t>(index)];
}

// ==============================================================================
// 用户预设目录
// ==============================================================================

juce::File PresetManager::getUserPresetDirectory()
{
    auto musicDir = juce::File::getSpecialLocation(juce::File::userMusicDirectory);
    auto dir = musicDir.getChildFile("SanMu EQ").getChildFile("Presets");

    if (!dir.exists())
    {
        dir.createDirectory();
        auto legacyDir = musicDir.getChildFile("SanMu EQ6").getChildFile("Presets");
        if (legacyDir.isDirectory())
        {
            for (const auto& legacyFile : legacyDir.findChildFiles(
                     juce::File::findFiles, false, "*.eqpreset"))
            {
                auto target = dir.getChildFile(legacyFile.getFileName());
                if (!target.existsAsFile())
                    legacyFile.copyFileTo(target);
            }
        }
    }

    return dir;
}

// ==============================================================================
// 用户预设名称列表
// ==============================================================================

juce::StringArray PresetManager::getUserPresetNames()
{
    juce::StringArray names;
    auto dir = getUserPresetDirectory();

    for (const auto& file : dir.findChildFiles(
             juce::File::findFiles, false, "*.eqpreset"))
        names.add(file.getFileNameWithoutExtension());

    return names;
}

// ==============================================================================
// 保存/加载用户预设
// ==============================================================================

bool PresetManager::saveUserPreset(const juce::String& name,
                                   juce::AudioProcessorValueTreeState& apvts)
{
    auto file = getUserPresetDirectory().getChildFile(name + ".eqpreset");

    // 从 APVTS 获取完整的参数树 XML
    auto state = apvts.copyState();
    auto xml = state.createXml();
    if (xml == nullptr)
        return false;

    // 嵌入预设名称与版本号
    xml->setAttribute("presetName", name);
    xml->setAttribute("schemaVersion", "1.1");

    return xml->writeTo(file);
}

bool PresetManager::loadUserPreset(const juce::String& name,
                                   juce::AudioProcessorValueTreeState& apvts)
{
    auto file = getUserPresetDirectory().getChildFile(name + ".eqpreset");
    if (!file.existsAsFile())
        return false;

    auto xml = juce::parseXML(file);
    if (xml == nullptr)
        return false;

    auto valueTree = juce::ValueTree::fromXml(*xml);
    if (!valueTree.isValid())
        return false;

    // 检查是否为旧版 6 段预设，执行自动迁移（R08）
    juce::String schema = xml->getStringAttribute("schemaVersion", "");
    if (schema.isEmpty())
    {
        for (int i = 1; i <= Params::LEGACY_NUM_BANDS; ++i)
        {
            auto pid = Params::IDs::bandPresent(i);
            if (!valueTree.hasProperty(pid))
                valueTree.setProperty(pid, 1.0f, nullptr);
        }
        for (int i = Params::LEGACY_NUM_BANDS + 1; i <= Params::MAX_BANDS; ++i)
        {
            valueTree.setProperty(Params::IDs::bandPresent(i), 0.0f, nullptr);
            valueTree.setProperty(Params::IDs::bandEnabled(i), 0.0f, nullptr);
        }
    }

    Params::supplyUpgradeDefaults(valueTree);
    apvts.replaceState(valueTree);
    return true;
}

bool PresetManager::deleteUserPreset(const juce::String& name)
{
    auto file = getUserPresetDirectory().getChildFile(name + ".eqpreset");
    return file.deleteFile();
}

// ==============================================================================
// 从 APVTS 快照当前参数状态（完整状态，R07）
// ==============================================================================

PresetData PresetManager::snapshotFromApvts(
    juce::AudioProcessorValueTreeState& apvts)
{
    PresetData data;
    data.name = "User Preset";

    for (int i = 0; i < Params::NUM_BANDS; ++i)
    {
        int bandNum = i + 1;

        if (auto* p = apvts.getRawParameterValue(Params::IDs::bandPresent(bandNum)))
            data.present[i] = p->load() > 0.5f;
        else
            data.present[i] = false;

        if (auto* p = apvts.getRawParameterValue(Params::IDs::bandEnabled(bandNum)))
            data.enabled[i] = p->load() > 0.5f;
        if (auto* p = apvts.getRawParameterValue(Params::IDs::bandType(bandNum)))
            data.type[i] = static_cast<int>(p->load());
        if (auto* p = apvts.getRawParameterValue(Params::IDs::bandFreq(bandNum)))
            data.freq[i] = p->load();
        if (auto* p = apvts.getRawParameterValue(Params::IDs::bandGain(bandNum)))
            data.gain[i] = p->load();
        if (auto* p = apvts.getRawParameterValue(Params::IDs::bandQ(bandNum)))
            data.q[i] = p->load();
        if (auto* p = apvts.getRawParameterValue(Params::IDs::bandSlope(bandNum)))
            data.slope[i] = static_cast<int>(p->load());
        if (auto* p = apvts.getRawParameterValue(Params::IDs::bandPlacement(bandNum)))
            data.placement[i] = static_cast<int>(p->load());
        if (auto* p = apvts.getRawParameterValue(Params::IDs::bandSolo(bandNum)))
            data.solo[i] = p->load() > 0.5f;
        if (auto* p = apvts.getRawParameterValue(Params::IDs::bandDynamic(bandNum)))
            data.dynamic[i] = p->load() > 0.5f;
        if (auto* p = apvts.getRawParameterValue(Params::IDs::bandDynAuto(bandNum)))
            data.dynAuto[i] = p->load() > 0.5f;
        if (auto* p = apvts.getRawParameterValue(Params::IDs::bandDynUpward(bandNum)))
            data.dynUpward[i] = p->load() > 0.5f;
        if (auto* p = apvts.getRawParameterValue(Params::IDs::bandDynExt(bandNum)))
            data.dynExt[i] = p->load() > 0.5f;
        if (auto* p = apvts.getRawParameterValue(Params::IDs::bandDynRange(bandNum)))
            data.dynRange[i] = p->load();
        if (auto* p = apvts.getRawParameterValue(Params::IDs::bandDynThreshold(bandNum)))
            data.dynThreshold[i] = p->load();
    }

    if (auto* p = apvts.getRawParameterValue(Params::IDs::INPUT_GAIN_L))   data.inputGainL     = p->load();
    if (auto* p = apvts.getRawParameterValue(Params::IDs::INPUT_GAIN_R))   data.inputGainR     = p->load();
    if (auto* p = apvts.getRawParameterValue(Params::IDs::OUTPUT_GAIN_L))  data.outputGainL    = p->load();
    if (auto* p = apvts.getRawParameterValue(Params::IDs::OUTPUT_GAIN_R))  data.outputGainR    = p->load();
    if (auto* p = apvts.getRawParameterValue(Params::IDs::PHASE_INVERT))   data.phaseInvert    = p->load() > 0.5f;
    if (auto* p = apvts.getRawParameterValue(Params::IDs::EQ_MODE))        data.processingMode = static_cast<int>(p->load());
    if (auto* p = apvts.getRawParameterValue(Params::IDs::CHARACTER_MODE)) data.characterMode  = static_cast<int>(p->load());
    if (auto* p = apvts.getRawParameterValue(Params::IDs::GLOBAL_BYPASS))  data.globalBypass   = p->load() > 0.5f;
    if (auto* p = apvts.getRawParameterValue(Params::IDs::AUTO_GAIN))      data.autoGain       = p->load() > 0.5f;

    return data;
}

// ==============================================================================
// 将 PresetData 应用到 APVTS（完整声音状态恢复，R07）
// ==============================================================================

static void setApvtsParam(juce::AudioProcessorValueTreeState& apvts,
                         const juce::String& id, float value)
{
    auto* param = apvts.getParameter(id);
    if (param != nullptr)
        param->setValueNotifyingHost(param->convertTo0to1(value));
}

void PresetManager::applyPreset(const PresetData& preset,
                                juce::AudioProcessorValueTreeState& apvts)
{
    Params::markCurrentLinearState(apvts.state, apvts.undoManager);
    for (int i = 0; i < Params::NUM_BANDS; ++i)
    {
        int bandNum = i + 1;

        setApvtsParam(apvts, Params::IDs::bandPresent(bandNum),   preset.present[i] ? 1.0f : 0.0f);
        setApvtsParam(apvts, Params::IDs::bandEnabled(bandNum),   preset.enabled[i] ? 1.0f : 0.0f);
        setApvtsParam(apvts, Params::IDs::bandType(bandNum),      static_cast<float>(preset.type[i]));
        setApvtsParam(apvts, Params::IDs::bandFreq(bandNum),      preset.freq[i]);
        setApvtsParam(apvts, Params::IDs::bandGain(bandNum),      preset.gain[i]);
        setApvtsParam(apvts, Params::IDs::bandQ(bandNum),         preset.q[i]);
        setApvtsParam(apvts, Params::IDs::bandSlope(bandNum),     static_cast<float>(preset.slope[i]));
        setApvtsParam(apvts, Params::IDs::bandPlacement(bandNum), static_cast<float>(preset.placement[i]));
        setApvtsParam(apvts, Params::IDs::bandSolo(bandNum),         preset.solo[i] ? 1.0f : 0.0f);
        setApvtsParam(apvts, Params::IDs::bandDynamic(bandNum),      preset.dynamic[i] ? 1.0f : 0.0f);
        setApvtsParam(apvts, Params::IDs::bandDynAuto(bandNum),      preset.dynAuto[i] ? 1.0f : 0.0f);
        setApvtsParam(apvts, Params::IDs::bandDynUpward(bandNum),    preset.dynUpward[i] ? 1.0f : 0.0f);
        setApvtsParam(apvts, Params::IDs::bandDynExt(bandNum),       preset.dynExt[i] ? 1.0f : 0.0f);
        setApvtsParam(apvts, Params::IDs::bandDynRange(bandNum),     preset.dynRange[i]);
        setApvtsParam(apvts, Params::IDs::bandDynThreshold(bandNum), preset.dynThreshold[i]);
    }

    setApvtsParam(apvts, Params::IDs::INPUT_GAIN_L,   preset.inputGainL);
    setApvtsParam(apvts, Params::IDs::INPUT_GAIN_R,   preset.inputGainR);
    setApvtsParam(apvts, Params::IDs::OUTPUT_GAIN_L,  preset.outputGainL);
    setApvtsParam(apvts, Params::IDs::OUTPUT_GAIN_R,  preset.outputGainR);
    setApvtsParam(apvts, Params::IDs::PHASE_INVERT,   preset.phaseInvert ? 1.0f : 0.0f);
    setApvtsParam(apvts, Params::IDs::EQ_MODE,        static_cast<float>(preset.processingMode));
    setApvtsParam(apvts, Params::IDs::GLOBAL_BYPASS,  preset.globalBypass ? 1.0f : 0.0f);
    setApvtsParam(apvts, Params::IDs::AUTO_GAIN,      preset.autoGain ? 1.0f : 0.0f);
    setApvtsParam(apvts, Params::IDs::CHARACTER_MODE, static_cast<float>(preset.characterMode));
}
