#include "PresetManager.h"
#include "HallPresets.h"
#include "PlatePresets.h"
#include "../params/Parameters.h"
#include "../params/StateCompatibility.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace
{
ReverbPreset makePreset(const juce::String& name, const juce::String& purpose,
                        const juce::String& category, int categoryIndex, int mode,
                        float decay, float predelay, float mix,
                        float highFrequency, float highShelf, float bassFrequency, float bassMultiplier,
                        float size, float attack, float early, float late,
                        float modRate, float modDepth, float lowCut, float highCut)
{
    ReverbPreset preset;
    preset.name = name;
    preset.categoryName = category;
    preset.purpose = purpose;
    preset.values[Params::IDs::CATEGORY] = static_cast<float>(categoryIndex);
    preset.values[Params::IDs::MODE] = static_cast<float>(mode);
    preset.values[Params::IDs::DECAY] = decay;
    preset.values[Params::IDs::PREDELAY] = predelay;
    preset.values[Params::IDs::MIX] = mix;
    preset.values[Params::IDs::HIGH_FREQ] = highFrequency;
    preset.values[Params::IDs::HIGH_SHELF] = highShelf;
    preset.values[Params::IDs::BASS_FREQ] = bassFrequency;
    preset.values[Params::IDs::BASS_MULT] = bassMultiplier;
    preset.values[Params::IDs::SIZE] = size;
    preset.values[Params::IDs::ATTACK] = attack;
    preset.values[Params::IDs::EARLY_DIFF] = early;
    preset.values[Params::IDs::LATE_DIFF] = late;
    preset.values[Params::IDs::MOD_RATE] = modRate;
    preset.values[Params::IDs::MOD_DEPTH] = modDepth;
    preset.values[Params::IDs::LOW_CUT] = lowCut;
    preset.values[Params::IDs::HIGH_CUT] = highCut;
    return preset;
}

void appendDefinition(std::vector<ReverbPreset>& result, const char* name, const char* purpose,
                      const char* category, int categoryIndex, int mode,
                      const std::array<float, 15>& values)
{
    result.push_back(makePreset(juce::String::fromUTF8(name), juce::String::fromUTF8(purpose), category,
        categoryIndex, mode, values[0], values[1], values[2], values[3], values[4], values[5],
        values[6], values[7], values[8], values[9], values[10], values[11], values[12], values[13], values[14]));
}
}

const std::vector<ReverbPreset>& PresetManager::getFactoryPresetsInternal()
{
    // 只展示已按当前八种模式设计的正式预设。旧架构预设保存在历史候选包中，
    // 不再混入产品菜单。插件没有将这里的序号暴露为宿主Program。
    static const std::vector<ReverbPreset> presets = []
    {
        std::vector<ReverbPreset> result;
        result.reserve(28);

        // PLATE / Modern Silk
        for (const auto& definition : plate_presets::definitions)
            appendDefinition(result, definition.name, definition.purpose, "Plate", 0, 0, definition.values);
        // PLATE / Classic Steel
        result.push_back(makePreset(juce::String::fromUTF8("经典钢板 · 基准"),
            juce::String::fromUTF8("保留颗粒与轮廓，中低频尾音温和收束。"), "Plate", 0, 1,
            2.20f, 16, 22, 6800, -8, 280, .95f, 100, 25, 95, 100, .7f, 18, 80, 10500));
        result.push_back(makePreset(juce::String::fromUTF8("经典钢板 · 亮丽人声"),
            juce::String::fromUTF8("32ms 预延迟推开字头，钢板高频亮泽让主唱穿透混音。"), "Plate", 0, 1,
            2.45f, 32, 23, 7200, -6, 260, .9f, 95, 30, 100, 100, .65f, 16, 120, 11500));
        result.push_back(makePreset(juce::String::fromUTF8("经典钢板 · 铜管瞬态"),
            juce::String::fromUTF8("干脆明快的钢板密度与反射，突出铜管与打击乐存在感。"), "Plate", 0, 1,
            1.45f, 18, 20, 7000, -8, 250, .85f, 88, 12, 90, 95, .55f, 12, 100, 11000));

        // ROOM / Studio Room
        result.push_back(makePreset(juce::String::fromUTF8("录音室 · 自然基准"),
            juce::String::fromUTF8("紧凑自然的公共空间，保留音头与近处反射。"), "Room", 1, 0,
            .68f, 4, 18, 6800, -6, 250, .9f, 85, 18, 100, 100, .45f, 12, 55, 11500));
        result.push_back(makePreset(juce::String::fromUTF8("录音室 · 贴近人声"),
            juce::String::fromUTF8("短而贴近的人声空间，减少低频与齿音堆积。"), "Room", 1, 0,
            .48f, 10, 15, 6200, -8, 250, .85f, 68, 12, 100, 100, .35f, 8, 95, 10000));
        result.push_back(makePreset(juce::String::fromUTF8("录音室 · 原声吉他"),
            juce::String::fromUTF8("贴耳且具木质共鸣的拾音房间感，增强原声弦乐融合度。"), "Room", 1, 0,
            .92f, 14, 17, 6500, -7, 260, .92f, 78, 20, 92, 98, .42f, 10, 85, 10800));
        // ROOM / Live Room
        result.push_back(makePreset(juce::String::fromUTF8("现场房间 · 开阔基准"),
            juce::String::fromUTF8("更开阔的反射间距和鲜活尾音。"), "Room", 1, 1,
            1.35f, 6, 20, 6800, -6, 250, 1, 100, 32, 90, 100, .55f, 15, 50, 11500));
        result.push_back(makePreset(juce::String::fromUTF8("现场房间 · 鼓组空间"),
            juce::String::fromUTF8("保留鼓组瞬态与宽度，控制房间金属感。"), "Room", 1, 1,
            .88f, 3, 22, 7200, -6, 250, .9f, 82, 10, 78, 100, .5f, 10, 70, 11500));
        result.push_back(makePreset(juce::String::fromUTF8("现场房间 · 钢琴拾音"),
            juce::String::fromUTF8("立体声远距话筒的生动房间余韵，保持中音区清晰透明。"), "Room", 1, 1,
            1.55f, 14, 19, 6400, -7, 260, .98f, 92, 24, 90, 96, .48f, 12, 60, 10500));

        // HALL / Concert Hall 与 Cathedral
        for (const auto& definition : hall_presets::definitions)
            appendDefinition(result, definition.name, definition.purpose, "Hall", 2, definition.mode, definition.values);

        // CHAMBER / Velvet Chamber
        result.push_back(makePreset(juce::String::fromUTF8("柔暖回声室 · 自然基准"),
            juce::String::fromUTF8("柔暖细密的回声室基准，适合先判断整体空间。"), "Chamber", 3, 0,
            2.1f, 15, 22, 6200, -9, 280, 1.08f, 100, 35, 100, 100, .6f, 20, 60, 10000));
        result.push_back(makePreset(juce::String::fromUTF8("柔暖回声室 · 人声包裹"),
            juce::String::fromUTF8("让人声后方形成柔和包裹，保留字头。"), "Chamber", 3, 0,
            1.85f, 28, 20, 6000, -10, 260, .98f, 90, 42, 100, 100, .45f, 15, 95, 9400));
        result.push_back(makePreset(juce::String::fromUTF8("柔暖回声室 · 弦乐余韵"),
            juce::String::fromUTF8("较慢展开的连续尾声，融合弦乐声部。"), "Chamber", 3, 0,
            2.8f, 18, 24, 6200, -9, 280, 1.12f, 100, 65, 100, 100, .4f, 22, 65, 10000));
        // CHAMBER / Stone Chamber
        result.push_back(makePreset(juce::String::fromUTF8("石质回声室 · 开阔基准"),
            juce::String::fromUTF8("清晰墙面轮廓与更大的反射跨度。"), "Chamber", 3, 1,
            2.3f, 12, 21, 7200, -8, 280, 1.02f, 100, 30, 90, 100, .55f, 15, 65, 11500));
        result.push_back(makePreset(juce::String::fromUTF8("石质回声室 · 军鼓轮廓"),
            juce::String::fromUTF8("短尾、鲜明起音和受控的鼓身。"), "Chamber", 3, 1,
            1.05f, 8, 19, 7500, -8, 240, .85f, 80, 15, 80, 95, .7f, 12, 105, 11500));
        result.push_back(makePreset(juce::String::fromUTF8("石质回声室 · 钢琴空间"),
            juce::String::fromUTF8("在敲击轮廓后展开清晰、稳定的空间。"), "Chamber", 3, 1,
            1.75f, 18, 18, 6600, -8, 260, .98f, 95, 45, 100, 100, .4f, 12, 75, 10500));
        return result;
    }();
    return presets;
}

int PresetManager::getNumFactoryPresets()
{
    return static_cast<int>(getFactoryPresetsInternal().size());
}

const ReverbPreset& PresetManager::getFactoryPreset(int index)
{
    const auto& presets = getFactoryPresetsInternal();
    return presets[static_cast<std::size_t>(juce::jlimit(0, static_cast<int>(presets.size()) - 1, index))];
}

juce::StringArray PresetManager::getFactoryPresetNames()
{
    juce::StringArray names;
    for (const auto& preset : getFactoryPresetsInternal()) names.add(preset.name);
    return names;
}

void PresetManager::applyPreset(const ReverbPreset& preset, juce::AudioProcessorValueTreeState& apvts)
{
    for (const auto& [id, value] : preset.values)
        if (auto* parameter = apvts.getParameter(id))
            parameter->setValueNotifyingHost(parameter->getNormalisableRange().convertTo0to1(value));
}

ReverbPreset PresetManager::snapshotCurrent(const juce::String& name, const juce::String& category,
                                            juce::AudioProcessorValueTreeState& apvts)
{
    ReverbPreset preset;
    preset.name = name;
    preset.categoryName = category;
    for (const auto& id : Params::getAllKnobParamIds())
        if (auto* raw = apvts.getRawParameterValue(id)) preset.values[id] = raw->load();
    if (auto* raw = apvts.getRawParameterValue(Params::IDs::CATEGORY)) preset.values[Params::IDs::CATEGORY] = raw->load();
    if (auto* raw = apvts.getRawParameterValue(Params::IDs::MODE)) preset.values[Params::IDs::MODE] = raw->load();
    return preset;
}

juce::File PresetManager::getUserPresetDirectory()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("SanMu Audio").getChildFile("SanMu Reverb").getChildFile("Presets");
}

juce::File PresetManager::saveUserPreset(const juce::String& name, juce::AudioProcessorValueTreeState& apvts,
                                         const juce::File& directory)
{
    const auto safeName = juce::File::createLegalFileName(name.trim()).substring(0, 80);
    if (safeName.isEmpty() || !directory.createDirectory()) return {};
    const auto file = directory.getNonexistentChildFile(safeName, ".sanmureverb", true);
    auto state = apvts.copyState();
    state_compatibility::markCurrent(state);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    return xml != nullptr && xml->writeTo(file) ? file : juce::File();
}

bool PresetManager::loadUserPreset(const juce::File& file, juce::AudioProcessorValueTreeState& apvts)
{
    if (!file.existsAsFile()) return false;
    const auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr) return false;
    auto state = juce::ValueTree::fromXml(*xml);
    if (!state_compatibility::isValid(state, apvts)) return false;
    state_compatibility::migrate(state);
    apvts.replaceState(state);
    return true;
}

juce::Array<juce::File> PresetManager::getUserPresetFiles(const juce::File& directory)
{
    auto files = directory.findChildFiles(juce::File::findFiles, false, "*.sanmureverb");
    std::sort(files.begin(), files.end(), [](const auto& a, const auto& b)
    { return a.getFileName().compareNatural(b.getFileName()) < 0; });
    return files;
}
