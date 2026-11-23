// ==============================================================================
// BbdEngine.cpp - 1980 经典模拟桶链（Bucket-Brigade Device）延迟引擎实现
// ==============================================================================

#include "BbdEngine.h"
#include <cmath>

namespace dsp_engines
{

BbdEngine::BbdEngine()
{
    filterClockL.setType(dsp_primitives::TptSvf::Type::LowPass);
    filterClockR.setType(dsp_primitives::TptSvf::Type::LowPass);
    filterLowCutL.setType(dsp_primitives::TptSvf::Type::HighPass);
    filterLowCutR.setType(dsp_primitives::TptSvf::Type::HighPass);
}

void BbdEngine::prepare(double sampleRate, int /*maxBlockSize*/)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 48000.0;

    // 延迟线最大容量支持 5.0 秒 (含足够插值余量)
    const int maxDelaySamples = static_cast<int>(currentSampleRate * 5.0) + 64;
    delayLineL.prepare(maxDelaySamples);
    delayLineR.prepare(maxDelaySamples);

    filterClockL.setSampleRate(currentSampleRate);
    filterClockR.setSampleRate(currentSampleRate);
    filterLowCutL.setSampleRate(currentSampleRate);
    filterLowCutR.setSampleRate(currentSampleRate);

    reset();
}

void BbdEngine::reset()
{
    delayLineL.reset();
    delayLineR.reset();

    filterClockL.reset();
    filterClockR.reset();
    filterLowCutL.reset();
    filterLowCutR.reset();

    lastClockCutHz = -1.0f;
    lastLowCutHz = -1.0f;
}

void BbdEngine::processStereoSample(float inL, float inR,
                                   float& outL, float& outR,
                                   const EngineProcessContext& ctx) noexcept
{
    // 1. 阻尼滤波参数更新（完整支持用户 1000..20000 Hz 高切与 20..1000 Hz 低切）
    const float dynamicClockCut = juce::jlimit(1000.0f, 20000.0f, ctx.dampHighcutHz);

    if (std::abs(dynamicClockCut - lastClockCutHz) > 0.5f)
    {
        lastClockCutHz = dynamicClockCut;
        filterClockL.setCutoff(lastClockCutHz);
        filterClockR.setCutoff(lastClockCutHz);
    }

    if (std::abs(ctx.dampLowcutHz - lastLowCutHz) > 0.1f)
    {
        lastLowCutHz = ctx.dampLowcutHz;
        filterLowCutL.setCutoff(lastLowCutHz);
        filterLowCutR.setCutoff(lastLowCutHz);
    }

    // 2. 从延迟线读取
    const float delayedL = delayLineL.readHermite(ctx.delayTimeSamplesL);
    const float delayedR = delayLineR.readHermite(ctx.delayTimeSamplesR);

    // 3. 环内时钟低通与低切滤波
    float fbL = filterClockL.processSample(delayedL);
    fbL = filterLowCutL.processSample(fbL);

    float fbR = filterClockR.processSample(delayedR);
    fbR = filterLowCutR.processSample(fbR);

    // 4. BBD 模拟压扩非线性染色 (ctx.modParam: 0 ~ 100% -> warmthAmount: 0.0 ~ 1.0)
    const float warmth = ctx.modParam * 0.01f;
    fbL = bbdSaturate(fbL, warmth);
    fbR = bbdSaturate(fbR, warmth);

    outL = fbL;
    outR = fbR;

    // 5. 反馈路由矩阵 (Stereo / Ping-Pong / Cross)
    float inFeedL = fbL;
    float inFeedR = fbR;
    float actualInL = inL;
    float actualInR = inR;

    if (ctx.topology == 1) // PingPong
    {
        inFeedL = fbR;
        inFeedR = fbL;
        const float monoIn = 0.5f * (inL + inR);
        actualInL = monoIn;
        actualInR = 0.0f;
    }
    else if (ctx.topology == 2) // Cross
    {
        inFeedL = 0.75f * fbL + 0.25f * fbR;
        inFeedR = 0.25f * fbL + 0.75f * fbR;
    }

    // 6. 反馈写入
    if (ctx.freeze)
    {
        delayLineL.write(std::tanh(inFeedL * 0.999f));
        delayLineR.write(std::tanh(inFeedR * 0.999f));
    }
    else if (ctx.topology == 1)
    {
        const float pongGain = 0.85f + 0.135f * ctx.feedback;
        delayLineL.write(actualInL + inFeedL * ctx.feedback);
        delayLineR.write(inFeedR * pongGain);
    }
    else
    {
        delayLineL.write(actualInL + inFeedL * ctx.feedback);
        delayLineR.write(actualInR + inFeedR * ctx.feedback);
    }
}

} // namespace dsp_engines
