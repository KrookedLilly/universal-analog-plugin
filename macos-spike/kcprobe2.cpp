// Diagnostic probe #2: mirror the PLUGIN's threading. getAll() on the main thread
// (like the plugin's discovery), then poll getActiveKeys() on a BACKGROUND thread
// (like the plugin's per-device poll thread). Isolates whether macOS CFRunLoop HID
// delivery breaks on a secondary thread vs the main thread (kcprobe worked on main).
#include <soup/AnalogueKeyboard.hpp>
#include <cstdio>
#include <cstring>
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
    printf("found '%s'. Polling on a BACKGROUND thread (mirrors the plugin). Press keys ~12s...\n", kc->name.c_str());

    std::atomic<bool> running{true};
    std::atomic<int> samples{0};
    std::thread th([&]
    {
        while (running.load())
        {
            auto keys = kc->getActiveKeys();
            for (auto& key : keys)
            {
                printf("  ACTIVE soupkey=%d value=%.3f\n", (int)key.getSoupKey(), key.getFValue());
                samples.fetch_add(1);
            }
            if (kc->disconnected) { printf("[poll-thread] DISCONNECTED\n"); break; }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(12));
    running.store(false);
    th.join();
    printf("done — %d analog samples seen on the background thread\n", samples.load());
    return 0;
}
