/*
 * Standalone Wooting Analog SDK read harness — macOS spike verification.
 *
 * Links the same wrapper the HE-Keyboard mod uses via JNA
 * (libwooting_analog_wrapper). Its job: prove the forked Universal Analog
 * Plugin is loaded by the SDK and that a real keypress on a Hall-Effect board
 * produces a non-zero analog value — WITHOUT involving the JVM/Minecraft, so
 * the macOS Input Monitoring permission is isolated to this tiny binary.
 *
 * Success = pressing a key on the Keychron Q5 HE prints "ANALOG READ OK".
 */
#include <stdio.h>
#include <unistd.h>

/* Wooting Analog wrapper C API (subset). In C the symbols are unprefixed;
 * the dylib exports them as _wooting_analog_* per Mach-O mangling. */
extern int   wooting_analog_initialise(void);
extern int   wooting_analog_is_initialised(void);
extern int   wooting_analog_uninitialise(void);
extern int   wooting_analog_read_full_buffer(unsigned short *code_buffer,
                                             float *analog_buffer,
                                             unsigned int len);

#define MAX_KEYS 16
#define POLL_MS  50
#define ITERS    600   /* ~30s at 50ms */

int main(void) {
    int rc = wooting_analog_initialise();
    /* On success the wrapper returns the number of connected devices (>= 0);
     * a negative value is a WootingAnalogResult error code. */
    printf("wooting_analog_initialise -> %d  (>=0 ok / negative = error code)\n", rc);
    if (rc < 0) {
        printf("SDK init failed. Likely the plugin was not found/loaded, or no\n"
               "device. Check the plugin dir and RUST_LOG=debug output.\n");
        return 1;
    }
    printf("is_initialised -> %d, connected devices -> %d\n",
           wooting_analog_is_initialised(), rc);

    unsigned short codes[MAX_KEYS];
    float values[MAX_KEYS];
    printf("\nPress keys on the Keychron Q5 HE. Watching ~30s for non-zero analog values...\n");
    int saw_any = 0;
    for (int i = 0; i < ITERS; i++) {
        int n = wooting_analog_read_full_buffer(codes, values, MAX_KEYS);
        for (int k = 0; k < n; k++) {
            if (values[k] > 0.0f) {
                printf("KEY code=0x%04x value=%.3f   <-- ANALOG READ OK\n",
                       codes[k], values[k]);
                saw_any = 1;
            }
        }
        usleep(POLL_MS * 1000);
    }
    printf("\n%s\n", saw_any
        ? "RESULT: SUCCESS — analog values read through the SDK + forked plugin."
        : "RESULT: no analog values seen. Plugin loaded? Input Monitoring granted? Device matched?");
    wooting_analog_uninitialise(); /* clean SDK/plugin shutdown (joins plugin threads) */
    return saw_any ? 0 : 2;
}
