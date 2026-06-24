// Diagnostic probe: drive the Keychron 0xFF60 analog interface directly via Soup
// on the MAIN thread with bounded, non-blocking polling. Reveals where macOS HID
// I/O dies: sendReport success? does any input report ever arrive?
#include <soup/AnalogueKeyboard.hpp>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <thread>
#include <chrono>

int main()
{
    auto kbds = soup::AnalogueKeyboard::getAll(true); // include no-permission too
    printf("found %zu analog keyboard interface(s)\n", kbds.size());

    soup::AnalogueKeyboard* kc = nullptr;
    for (auto& k : kbds)
    {
        printf("  name='%s' vid=%04x pid=%04x usage_page=%04x usage=%02x havePermission=%d in=%u out=%u feat=%u\n",
            k.name.c_str(), k.hid.vendor_id, k.hid.product_id, k.hid.usage_page, k.hid.usage,
            (int)k.hid.havePermission(),
            (unsigned)k.hid.input_report_byte_length,
            (unsigned)k.hid.output_report_byte_length,
            (unsigned)k.hid.feature_report_byte_length);
        if (k.hid.vendor_id == 0x3434) kc = &k;
    }
    if (!kc) { printf("\nNo Keychron (0x3434) analog interface found.\n"); return 1; }

    // --- Manually drive the 0xa9/0x01 GET_VERSION handshake with a bounded wait ---
    printf("\n[handshake] havePermission=%d ; sending 0xa9/0x01 GET_VERSION...\n", (int)kc->hid.havePermission());
    uint8_t data[33];
    memset(data, 0, sizeof(data));
    data[1] = 0xa9; // KC_HE
    data[2] = 0x01; // AMC_GET_VERSION
    bool sent = kc->hid.sendReport(data, sizeof(data));
    printf("[handshake] sendReport returned %d\n", (int)sent);

    bool got = false;
    for (int i = 0; i < 2000; i++) // up to ~2s
    {
        if (kc->hid.hasReport())
        {
            const auto& r = kc->hid.receiveReport();
            printf("[handshake] GOT report size=%zu : ", (size_t)r.size());
            for (size_t j = 0; j < (size_t)r.size() && j < 12; j++) printf("%02x ", (uint8_t)r[j]);
            printf("\n");
            got = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (!got) printf("[handshake] NO response within 2s (input-report callback never fired)\n");

    // --- High-level path on the MAIN thread for ~10s ---
    printf("\n[getActiveKeys] polling on main thread ~10s. PRESS Keychron keys now...\n");
    int reads = 0;
    for (int i = 0; i < 1000; i++)
    {
        auto keys = kc->getActiveKeys();
        for (auto& key : keys)
        {
            printf("  ACTIVE soupkey=%d value=%.3f\n", (int)key.getSoupKey(), key.getFValue());
            reads++;
        }
        if (kc->disconnected) { printf("[getActiveKeys] keyboard marked DISCONNECTED\n"); break; }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    printf("done (%d active-key samples seen)\n", reads);
    return 0;
}
