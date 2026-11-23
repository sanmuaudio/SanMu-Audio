#include "BetaAccess.h"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
using namespace sanmu_beta;
void check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
int main()
{
    check(assess(deadline-8*day,0)==Status::active,"正常期限");
    check(assess(deadline-7*day,0)==Status::expiring,"七天提醒边界");
    check(assess(deadline-1,0)==Status::expiring,"截止前一毫秒仍可用");
    check(assess(deadline,0)==Status::expired,"截止时刻到期");
    check(assess(deadline-day,deadline)==Status::expired,"到期记录不可回拨恢复");
    check(assess(deadline-9*day,deadline-8*day)==Status::clockError,"时间回拨检测");
    check(assess(deadline-8*day-tolerance,deadline-8*day)==Status::active,"容许系统校时误差");
    for (const double rate : {44100.,48000.,96000.,192000.})
    for (int channels : {1,2})
    for (int latency : {0,512,8192,600000})
    {
        AudioGate gate; gate.prepare(rate,256,true);
        juce::AudioBuffer<float> block(channels,256);
        int sample=0;
        for (int k=0;k<70;++k)
        {
            for (int c=0;c<channels;++c) for (int n=0;n<256;++n)
                block.setSample(c,n,float((sample+n)%997)/997.f);
            const bool process=gate.begin(block,channels,latency,true);
            check(process,"到期前应处理音频");
            block.clear(); gate.finish(block);
            check(block.getMagnitude(0,256)==0.f,"到期前输出不受限时混合器影响");
            sample+=256;
        }
        // 播放中到期，湿路为零、原声为常量，必须连续渐变。
        gate.prepare(rate,256,true);
        float previous=0.f;
        for (int k=0;k<100;++k)
        {
            for (int c=0;c<channels;++c) for (int n=0;n<256;++n) block.setSample(c,n,1.f);
            const bool process=gate.begin(block,channels,0,false);
            if (process) { block.clear(); gate.finish(block); }
            for (int n=0;n<256;++n)
            {
                const float value=block.getSample(0,n);
                check(value>=previous-1e-6f && value-previous<.001f,"到期平滑切换"); previous=value;
            }
        }
        check(previous==1.f,"到期后原声直通");
        // 已到期实例必须立即旁通，保持宿主上报的延迟。
        gate.prepare(rate,256,false,latency); sample=0;
        for (int k=0;k<(latency+20000)/256+1;++k)
        {
            for (int c=0;c<channels;++c) for (int n=0;n<256;++n) block.setSample(c,n,sample+n==0?1.f:0.f);
            check(!gate.begin(block,channels,latency,false),"已到期不运行效果器");
            for (int c=0;c<channels;++c) for (int n=0;n<256;++n)
                check(block.getSample(c,n)==(sample+n==latency?1.f:0.f),"到期直通延迟与脉冲位置");
            sample+=256;
        }
        juce::AudioBuffer<float> empty(channels,0);
        check(!gate.begin(empty,channels,latency,false),"零长度块");
    }
    AudioGate gate; gate.prepare(48000,128,false);
    juce::AudioBuffer<float> huge(2,70000);
    for(int c=0;c<2;++c)for(int n=0;n<70000;++n)huge.setSample(c,n,.25f);
    check(!gate.begin(huge,2,0,false),"异常大块安全直通");
    check(huge.getSample(1,69999)==.25f,"大块数据完整");
    const auto dir=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("sanmu-beta-test", "", false);
    dir.createDirectory();
    const auto record=dir.getChildFile("clock");
    {
        Access access(record);
        check(access.allowed(),"新用户正常期限与记录创建");
        check(record.existsAsFile(),"记录落盘");
    }
    record.replaceWithText(juce::String(deadline));
    { Access access(record); check(access.status()==Status::expired,"重启后已到期记录生效"); }
    record.replaceWithText("invalid");
    { Access access(record); juce::Thread::sleep(1200); check(!access.allowed(),"损坏记录不能自动绕过"); }
    dir.deleteRecursively();

    // SHA-256 NIST 标准测试向量验证
    check(detail::sha256Hex("") == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855", "SHA-256 empty vector");
    check(detail::sha256Hex("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", "SHA-256 abc vector");
    check(detail::sha256Hex("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq")
        == "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1", "SHA-256 56-byte boundary vector");

    // 验证四款插件在无时钟记录文件时处于完全免费、无试用到期且不创建 licenses.json 的状态
    const auto licTestDir = juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("sanmu-lic-test", "", false);
    licTestDir.createDirectory();
    ::setenv("SANMU_TEST_PREFERENCES_DIR", licTestDir.getFullPathName().toRawUTF8(), 1);

    for (const auto* scopeName : { "EQ", "COMP", "DELAY", "REVERB" })
    {
        Access plug({}, scopeName);
        const auto info = plug.getLicenseInfo();
        check(plug.allowed(), "免费开源版应始终允许运行");
        check(plug.status() == Status::active, "免费开源版状态应始终为 active");
        check(info["mode"].toString() == "free", "免费模式应标记为 free");
        check(int(info["trialDaysLeft"]) == 0, "免费模式不应有试用剩余天数");
    }
    check(!licTestDir.getChildFile("Licenses").getChildFile("licenses.json").exists(),
          "免费开源版不得读写或创建 licenses.json");

    ::unsetenv("SANMU_TEST_PREFERENCES_DIR");
    licTestDir.deleteRecursively();
    std::cout << "期限边界、回拨、四采样率、单声道/立体声、播放中到期、延迟补偿、完全免费模式与大块验证通过\n";
}
