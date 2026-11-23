#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace sanmu_beta
{
// 免费开源版永久解锁契约；保留 deadline/assess/AudioGate 与测试时钟模式以通过既有单元契约测试。
inline constexpr juce::int64 deadline = 4102444800000LL;
inline constexpr juce::int64 day = 86400000LL;
inline constexpr juce::int64 tolerance = 10 * 60 * 1000LL;
inline constexpr int trialDurationDays = 0;

enum class Status { active, expiring, expired, clockError, storageError };

inline Status assess(juce::int64 now, juce::int64 last)
{
    if (now >= deadline || last >= deadline) return Status::expired;
    if (now + tolerance < last) return Status::clockError;
    return now >= deadline - 7 * day ? Status::expiring : Status::active;
}

namespace detail
{
inline constexpr std::uint32_t rotr(std::uint32_t x, std::uint32_t n) noexcept
{
    return (x >> n) | (x << (32u - n));
}

inline void sha256Transform(std::uint32_t state[8], const std::uint8_t block[64]) noexcept
{
    static constexpr std::uint32_t k[64] = {
        0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
        0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
        0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
        0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
        0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
        0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
        0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
        0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
    };

    std::uint32_t w[64] {};
    for (int i = 0; i < 16; ++i)
    {
        const int j = i * 4;
        w[i] = (static_cast<std::uint32_t>(block[j]) << 24)
             | (static_cast<std::uint32_t>(block[j + 1]) << 16)
             | (static_cast<std::uint32_t>(block[j + 2]) << 8)
             |  static_cast<std::uint32_t>(block[j + 3]);
    }
    for (int i = 16; i < 64; ++i)
    {
        const std::uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
        const std::uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    std::uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    std::uint32_t e = state[4], f = state[5], g = state[6], h = state[7];

    for (int i = 0; i < 64; ++i)
    {
        const std::uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
        const std::uint32_t ch = (e & f) ^ ((~e) & g);
        const std::uint32_t temp1 = h + S1 + ch + k[i] + w[i];
        const std::uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
        const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        const std::uint32_t temp2 = S0 + maj;

        h = g; g = f; f = e; e = d + temp1;
        d = c; c = b; b = a; a = temp1 + temp2;
    }

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

inline std::array<std::uint8_t, 32> sha256(const void* data, std::size_t len) noexcept
{
    std::uint32_t state[8] = {
        0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
        0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u
    };

    const auto* bytes = static_cast<const std::uint8_t*>(data);
    std::size_t offset = 0;
    while (len - offset >= 64)
    {
        sha256Transform(state, bytes + offset);
        offset += 64;
    }

    std::uint8_t buf[64] {};
    const std::size_t rem = len - offset;
    if (rem > 0) std::memcpy(buf, bytes + offset, rem);
    buf[rem] = 0x80u;
    if (rem >= 56)
    {
        sha256Transform(state, buf);
        std::memset(buf, 0, sizeof(buf));
    }

    const std::uint64_t bitLen = static_cast<std::uint64_t>(len) * 8ULL;
    for (int i = 0; i < 8; ++i)
        buf[56 + i] = static_cast<std::uint8_t>((bitLen >> (56 - i * 8)) & 0xffu);
    sha256Transform(state, buf);

    std::array<std::uint8_t, 32> out {};
    for (int i = 0; i < 8; ++i)
    {
        out[static_cast<std::size_t>(i * 4 + 0)] = static_cast<std::uint8_t>((state[i] >> 24) & 0xffu);
        out[static_cast<std::size_t>(i * 4 + 1)] = static_cast<std::uint8_t>((state[i] >> 16) & 0xffu);
        out[static_cast<std::size_t>(i * 4 + 2)] = static_cast<std::uint8_t>((state[i] >> 8)  & 0xffu);
        out[static_cast<std::size_t>(i * 4 + 3)] = static_cast<std::uint8_t>( state[i]        & 0xffu);
    }
    return out;
}

inline juce::String sha256Hex(const juce::String& utf8Text)
{
    const auto utf8 = utf8Text.toUTF8();
    const auto* ptr = utf8.getAddress();
    const auto byteLen = ptr != nullptr ? std::strlen(ptr) : 0;
    const auto digest = sha256(ptr != nullptr ? static_cast<const void*>(ptr) : static_cast<const void*>(""), byteLen);
    return juce::String::toHexString(digest.data(), static_cast<int>(digest.size()), 0).toLowerCase();
}

inline juce::String sanitizeScope(const juce::String& raw)
{
    const auto s = raw.trim().toUpperCase();
    if (s == "EQ" || s == "COMP" || s == "DELAY" || s == "REVERB" || s == "BUNDLE")
        return s;
    return "BUNDLE";
}
} // namespace detail

class Access final : private juce::Thread
{
public:
    explicit Access(juce::File record = {}, juce::String pluginScope = "BUNDLE")
        : Thread("SanMu Test Clock"),
          file(record),
          unlocked(record == juce::File()),
          testClockMode(record != juce::File()),
          pluginScopeName(detail::sanitizeScope(pluginScope))
    {
        if (testClockMode)
        {
            refreshLegacy(true);
            startThread();
        }
        else
        {
            storageHealthy = true;
            current.store(Status::active, std::memory_order_relaxed);
        }
    }

    ~Access() override
    {
        if (testClockMode)
        {
            signalThreadShouldExit();
            notify();
            stopThread(-1);
        }
    }

    Status status() const noexcept { return current.load(std::memory_order_relaxed); }

    bool allowed() const noexcept
    {
        if (!testClockMode)
            return true;
        const auto s = status();
        return s == Status::active || s == Status::expiring;
    }

    juce::String scope() const noexcept { return pluginScopeName; }

    juce::var getLicenseInfo() const
    {
        auto* obj = new juce::DynamicObject();
        const auto st = current.load(std::memory_order_relaxed);
        juce::String stStr = "active";
        if (st == Status::expiring) stStr = "expiring";
        else if (st == Status::expired) stStr = "expired";
        else if (st == Status::clockError) stStr = "clockError";
        else if (st == Status::storageError) stStr = "storageError";

        obj->setProperty("pluginScope", pluginScopeName);
        obj->setProperty("mode", testClockMode ? "test" : "free");
        obj->setProperty("allowed", allowed());
        obj->setProperty("status", stStr);
        obj->setProperty("email", juce::String());
        obj->setProperty("licenseScope", juce::String("BUNDLE"));
        obj->setProperty("tier", juce::String("FREE"));
        obj->setProperty("maskedKey", juce::String());
        obj->setProperty("activatedAt", juce::int64(0));
        obj->setProperty("trialDaysLeft", 0);
        obj->setProperty("trialEndsAt", juce::int64(0));
        return juce::var(obj);
    }

    juce::var activateLicense(const juce::String&, const juce::String&, juce::int64 = -1)
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("ok", true);
        obj->setProperty("license", getLicenseInfo());
        return juce::var(obj);
    }

    juce::var deactivateLicense()
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("ok", true);
        obj->setProperty("license", getLicenseInfo());
        return juce::var(obj);
    }

private:
    juce::File file;
    bool unlocked = true;
    bool testClockMode = false;
    juce::String pluginScopeName = "BUNDLE";
    std::atomic<Status> current { Status::active };
    juce::int64 last = 0;
    juce::int64 wallStart = juce::Time::currentTimeMillis();
    std::chrono::steady_clock::time_point steadyStart = std::chrono::steady_clock::now();
    int ticks = 0;
    bool storageHealthy = true;

    juce::int64 currentEffectiveMillis() const noexcept
    {
        const auto wall = juce::Time::currentTimeMillis();
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - steadyStart).count();
        return juce::jmax(wall, wallStart + static_cast<juce::int64>(elapsed));
    }

    void run() override
    {
        while (!threadShouldExit())
        {
            ++ticks;
            refreshLegacy(ticks % 60 == 0);
            wait(1000);
        }
    }

    void refreshLegacy(bool persist)
    {
        const auto wall = juce::Time::currentTimeMillis();
        const auto effective = currentEffectiveMillis();
        if (persist || !storageHealthy)
        {
            juce::InterProcessLock lock("SanMuBeta202610");
            if (!lock.enter(1500)) { storageHealthy = false; current.store(Status::storageError); return; }
            struct Unlock { juce::InterProcessLock& l; ~Unlock() { l.exit(); } } unlock { lock };
            if (file.existsAsFile())
            {
                const auto text = file.loadFileAsString().trim();
                if (text.isEmpty() || !text.containsOnly("0123456789") || text.length() > 15)
                { storageHealthy = false; current.store(Status::storageError); return; }
                last = juce::jmax(last, text.getLargeIntValue());
            }
            const auto next = juce::jmax(last, effective);
            if (!file.getParentDirectory().createDirectory() || !file.replaceWithText(juce::String(next)))
            { storageHealthy = false; current.store(Status::storageError); return; }
            last = next;
            storageHealthy = true;
        }
        auto next = assess(effective, last);
        if (wall + tolerance < last && next != Status::expired) next = Status::clockError;
        current.store(next, std::memory_order_relaxed);
        last = juce::jmax(last, effective);
    }
};

// 到期或测试门控时的静音平滑直通保护器，保留宿主工程与全部参数。
class AudioGate
{
public:
    void prepare(double sampleRate, int blockSize, bool allowed, int maximumLatency = 0)
    {
        dry.setSize(2, juce::jmax(65536, blockSize));
        ring.setSize(2, juce::jmax(1, maximumLatency + 1)); ring.clear(); write = 0;
        mix = allowed ? 0.f : 1.f;
        step = 1.f / static_cast<float>(juce::jmax(1.0, sampleRate * .05));
    }
    bool begin(juce::AudioBuffer<float>& b, int channels, int latency, bool allowed)
    {
        count = juce::jmin(2, juce::jmin(channels, b.getNumChannels()));
        samples = b.getNumSamples(); target = allowed ? 0.f : 1.f;
        if (ring.getNumSamples() == 0) return false;
        const bool oversized = samples > dry.getNumSamples();
        latency = juce::jlimit(0, ring.getNumSamples() - 1, latency);
        for (int n = 0; n < samples; ++n)
        {
            const int read = (write + ring.getNumSamples() - latency) % ring.getNumSamples();
            for (int c = 0; c < count; ++c)
            {
                const float input = b.getSample(c,n);
                ring.setSample(c,write,std::isfinite(input) ? input : 0.f);
                const float value = ring.getSample(c,read);
                if (oversized) b.setSample(c,n,value); else dry.setSample(c,n,value);
            }
            write = (write + 1) % ring.getNumSamples();
        }
        if (oversized) return false;
        if (!allowed && mix >= 1.f)
        {
            for (int c = 0; c < count; ++c) b.copyFrom(c,0,dry,c,0,samples);
            return false;
        }
        return true;
    }
    void finish(juce::AudioBuffer<float>& b)
    {
        for (int n = 0; n < samples; ++n)
        {
            mix += juce::jlimit(-step,step,target-mix);
            if (mix == 0.f) continue;
            for (int c = 0; c < count; ++c)
            {
                const float wet = b.getSample(c,n);
                b.setSample(c,n,(std::isfinite(wet) ? wet : 0.f)*(1.f-mix)+dry.getSample(c,n)*mix);
            }
        }
    }
    struct Scope
    {
        AudioGate& gate; juce::AudioBuffer<float>& buffer;
        ~Scope() { gate.finish(buffer); }
    };
private:
    juce::AudioBuffer<float> dry,ring;
    int write=0,count=0,samples=0;
    float mix=0,step=1,target=0;
};
}
