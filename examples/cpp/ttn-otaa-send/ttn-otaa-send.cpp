#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <wiringPi.h>
#include <lmic.h>
#include <hal.h>
#include <local_hal.h>

#define DEFAULT_DATA_RATE 2
#define DEFAULT_TX_POWER 20
#define SESSION_PORT 1

#define RFM95_PIN_NSS 6
#define RFM95_PIN_RST 0
#define RFM95_PIN_D0 4
#define RFM95_PIN_D1 5
#define STATUS_PIN_LED 2
#define DATA_SENT_LED 3

static u1_t g_devEui[8];
static u1_t g_appEui[8];
static u1_t g_appKey[16];
static volatile int send_complete = 0;
static volatile int join_complete = 0;
static volatile int join_failed = 0;
static int useLeds = 1;

void os_getArtEui(u1_t *buf) { memcpy(buf, g_appEui, 8); }
void os_getDevEui(u1_t *buf) { memcpy(buf, g_devEui, 8); }
void os_getDevKey(u1_t *buf) { memcpy(buf, g_appKey, 16); }

static int hex_value(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int parse_hex(const char *hex, uint8_t *out, size_t out_len)
{
    if (hex == NULL || strlen(hex) != out_len * 2) return -1;

    for (size_t i = 0; i < out_len; ++i) {
        int high = hex_value(hex[i * 2]);
        int low = hex_value(hex[i * 2 + 1]);
        if (high < 0 || low < 0) return -1;
        out[i] = (uint8_t)((high << 4) | low);
    }
    return 0;
}

static u4_t msbf4_read(const uint8_t *data)
{
    return ((u4_t)data[0] << 24) | ((u4_t)data[1] << 16) | ((u4_t)data[2] << 8) | data[3];
}

static int elapsed_ms(const struct timespec *start)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (int)((now.tv_sec - start->tv_sec) * 1000 +
                 (now.tv_nsec - start->tv_nsec) / 1000000);
}

void onEvent(ev_t ev)
{
    switch (ev) {
    case EV_JOINED:
        printf("Event EV_JOINED\n");
        join_complete = 1;
        break;
    case EV_JOIN_FAILED:
        printf("Event EV_JOIN_FAILED\n");
        join_failed = 1;
        break;
    case EV_TXCOMPLETE:
        printf("Event EV_TXCOMPLETE\n");
        send_complete = 1;
        break;
    default:
        printf("Event %d\n", ev);
        break;
    }
}

static void enable_leds(void)
{
    pinMode(STATUS_PIN_LED, OUTPUT);
    pinMode(DATA_SENT_LED, OUTPUT);
    if (useLeds) {
        digitalWrite(STATUS_PIN_LED, HIGH);
        delay(100);
        digitalWrite(STATUS_PIN_LED, LOW);
    }
}

static void configure_lmic(void)
{
    LMIC_reset();
    LMIC_setAdrMode(0);
    LMIC_setLinkCheckMode(0);
    LMIC_disableTracking();
    LMIC_stopPingable();
    LMIC.dn2Dr = 8;
    LMIC_setDrTxpow(DEFAULT_DATA_RATE, DEFAULT_TX_POWER);
}

static int wait_for_join(int timeout_ms)
{
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (!join_complete && !join_failed) {
        if (timeout_ms > 0 && elapsed_ms(&start) >= timeout_ms) {
            return -1;
        }
        if (!os_runloop_once()) {
            usleep(1000);
        }
    }
    return join_complete ? 0 : -2;
}

static int wait_for_send(int timeout_ms)
{
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (!send_complete) {
        if (timeout_ms > 0 && elapsed_ms(&start) >= timeout_ms) {
            return -1;
        }
        if (!os_runloop_once()) {
            usleep(1000);
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 6) {
        fprintf(stderr, "Usage: %s <DevEUI> <AppEUI> <AppKey> <Rain> <UseLeds>\n", argv[0]);
        return 1;
    }

    if (parse_hex(argv[1], g_devEui, sizeof(g_devEui)) < 0 ||
        parse_hex(argv[2], g_appEui, sizeof(g_appEui)) < 0 ||
        parse_hex(argv[3], g_appKey, sizeof(g_appKey)) < 0) {
        fprintf(stderr, "Invalid DevEUI/AppEUI/AppKey hex format\n");
        return 1;
    }

    float rain = strtof(argv[4], NULL);
    useLeds = atoi(argv[5]);

    uint8_t payload[2];
    int fixed = (int)(rain * 100.0f);
    payload[0] = (fixed >> 8) & 0xFF;
    payload[1] = fixed & 0xFF;

    wiringPiSetup();
    os_init();
    join_complete = 0;
    join_failed = 0;
    send_complete = 0;

    enable_leds();
    configure_lmic();
    if (!LMIC_startJoining()) {
        fprintf(stderr, "Already joined or already joined state\n");
    }

    if (wait_for_join(60000) != 0) {
        fprintf(stderr, "Join failed or timed out\n");
        return 2;
    }

    if (LMIC.opmode & OP_TXRXPEND) {
        fprintf(stderr, "TX pending after join, cannot send\n");
        return 3;
    }

    LMIC_setTxData2(1, payload, sizeof(payload), 0);
    if (useLeds) {
        digitalWrite(DATA_SENT_LED, HIGH);
    }

    if (wait_for_send(30000) != 0) {
        fprintf(stderr, "TX failed or timed out\n");
        return 4;
    }

    if (useLeds) {
        digitalWrite(DATA_SENT_LED, LOW);
    }

    if (LMIC.dataLen) {
        printf("Downlink (%d bytes):", LMIC.dataLen);
        for (int i = 0; i < LMIC.dataLen; ++i) {
            printf(" 0x%02x", LMIC.frame[LMIC.dataBeg + i]);
        }
        printf("\n");
    }

    return 0;
}
