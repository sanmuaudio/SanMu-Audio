#pragma once
#include <array>

namespace plate_presets
{
struct Definition
{
    const char* name;
    const char* purpose;
    // Decay、PreDelay、Mix、四个阻尼、Size、Attack、两个扩散、两个调制、LowCut、HighCut。
    std::array<float,15> values;
};
inline constexpr std::array<Definition,3> definitions {{
    {"丝滑板式 · 基准", "清晰起音与温暖细密的丝滑板式基准；插入比例 24%，可按需要调节 Mix。",
     {2.39f,15,24,6500,-10,280,1.0f,100,30,100,100,.8f,20,60,10500}},
    {"丝滑板式 · 温暖人声", "保留字头间隔，控制中低频积累，适合主唱插入或发送起手。",
     {2.1f,28,22,6000,-10,260,.85f,90,35,100,100,.65f,15,110,9500}},
    {"丝滑板式 · 军鼓短板", "较快建立与较短尾音，保留鼓身和沙带纹理。",
     {1.15f,7,20,6800,-8,240,.9f,80,10,90,100,.75f,12,90,11000}}
}};
} // namespace plate_presets
