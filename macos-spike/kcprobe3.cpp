// Diagnostic probe #3: reproduce the PLUGIN's full threading. A poll thread reads the
// tracked Keychron while a SEPARATE discovery thread calls getAll() every second and
// discards the result (exactly what discover_devices(false) does for an already-known
// device). Before the reset() fix this discarded duplicate's IOHIDDeviceClose() shut the
// shared device out from under the poll thread; after the fix, values should flow
// continuously despite the re-discovery.
#include <soup/AnalogueKeyboard.hpp>
#include <cstdio>
#include <thread>
#include <chrono>
#include <atomic>

int main()
{
    auto kbds = soup::AnalogueKeyboard::getAll(true);
    soup::AnalogueKeyboard* kc = nullptr;
    for (auto& k : kbds)
    {
        if (k.hid.vendor_id == 0x3434) kc = &k;
    }
    if (!kc) { printf("No Keychron found.\n"); return 1; }
    printf("found '%s'. poll thread + 1Hz re-discovery thread (mirrors plugin). Press keys ~15s...\n", kc->name.c_str());

    std::atomic<bool> running{true};
    std::atomic<int> samples{0};

    std::thread poll([&]
    {
        while (running.load())
        {
            auto keys = kc->getActiveKeys();
            for (auto& key : keys)
            {
                printf("  ACTIVE soupkey=%d value=%.3f\n", (int)key.getSoupKey(), key.getFValue());
                samples.fetch_add(1);
            }
            if (kc->disconnected) { printf("[poll] DISCONNECTED\n"); break; }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    std::thread rediscover([&]
    {
        while (running.load())
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            // Mirror discover_devices(false): re-enumerate and discard (device already "known").
            auto dup = soup::AnalogueKeyboard::getAll();
            printf("[rediscover] getAll() -> %zu device(s), discarding\n", dup.size());
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(15));
    running.store(false);
    poll.join();
    rediscover.join();
    printf("done — %d analog samples seen (want: many, spanning the whole run)\n", samples.load());
    return 0;
}
