/**
 * @file       BlynkApiSparkCore.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2015
 * @brief
 *
 */

#ifndef BlynkApiSparkCore_h
#define BlynkApiSparkCore_h

#include "BlynkApi.h"
#include "application.h"

#ifndef BLYNK_INFO_DEVICE
    #define BLYNK_INFO_DEVICE  "Spark Core"
#endif

template<class Proto>
void BlynkApi<Proto>::Init()
{
}

template<class Proto>
BLYNK_FORCE_INLINE
void BlynkApi<Proto>::processCmd(const void* buff, size_t len)
{
    BlynkParam param((void*)buff, len);
    BlynkParam::iterator it = param.begin();
    if (it >= param.end())
        return;
    const char* cmd = it.asStr();

    if (!strcmp(cmd, "info")) {
        static const char profile[] BLYNK_PROGMEM =
            BLYNK_PARAM_KV("ver"    , BLYNK_VERSION)
            BLYNK_PARAM_KV("h-beat" , TOSTRING(BLYNK_HEARTBEAT))
            BLYNK_PARAM_KV("buff-in", TOSTRING(BLYNK_MAX_READBYTES))
#ifdef BLYNK_INFO_DEVICE
            BLYNK_PARAM_KV("dev"    , BLYNK_INFO_DEVICE)
#endif
#ifdef BLYNK_INFO_CPU
            BLYNK_PARAM_KV("cpu"    , BLYNK_INFO_CPU)
#endif
#ifdef BLYNK_INFO_CONNECTION
            BLYNK_PARAM_KV("con"    , BLYNK_INFO_CONNECTION)
#endif
        ;
        const size_t profile_len = sizeof(profile)-1;

#ifdef BLYNK_HAS_PROGMEM
        char mem[profile_len];
        memcpy_P(mem, profile, profile_len);
        static_cast<Proto*>(this)->sendCmd(BLYNK_CMD_HARDWARE, 0, mem, profile_len);
#else
        static_cast<Proto*>(this)->sendCmd(BLYNK_CMD_HARDWARE, 0, profile, profile_len);
#endif
        return;
    }

    if (++it >= param.end())
        return;
    unsigned pin = it.asInt();

    if (!strcmp(cmd, "dr")) {
        char mem[16];
        BlynkParam rsp(mem, 0, sizeof(mem));
        rsp.add("dw");
        rsp.add(pin);
        rsp.add(digitalRead(pin));
        static_cast<Proto*>(this)->sendCmd(BLYNK_CMD_HARDWARE, 0, rsp.getBuffer(), rsp.getLength());
    } else if (!strcmp(cmd, "ar")) {
        char mem[16];
        BlynkParam rsp(mem, 0, sizeof(mem));
        rsp.add("aw");
        rsp.add(pin);
        rsp.add(analogRead(pin));
        static_cast<Proto*>(this)->sendCmd(BLYNK_CMD_HARDWARE, 0, rsp.getBuffer(), rsp.getLength());
    } else if (!strcmp(cmd, "vr")) {
        if (WidgetReadHandler handler = GetReadHandler(pin)) {
            BlynkReq req = { 0, BLYNK_SUCCESS, (uint8_t)pin };
            handler(req);
        }
    } else {

        if (!strcmp(cmd, "vw")) {
            ++it;
            if (WidgetWriteHandler handler = GetWriteHandler(pin)) {
                BlynkReq req = { 0, BLYNK_SUCCESS, (uint8_t)pin };
                char* start = (char*)it.asStr();
                BlynkParam param2(start, len - (start - (char*)buff));
                handler(req, param2);
            }
            return;
        }

        if (!strcmp(cmd, "pm")) {
            while (it < param.end()) {
                ++it;
                //BLYNK_LOG("pinMode %u -> %s", pin, it.asStr());
                if (!strcmp(it.asStr(), "in")) {
                    pinMode(pin, INPUT);
                } else if (!strcmp(it.asStr(), "out") || !strcmp(it.asStr(), "pwm")) {
                    pinMode(pin, OUTPUT);
                } else if (!strcmp(it.asStr(), "pu")) {
                    pinMode(pin, INPUT_PULLUP);
                } else {
#ifdef BLYNK_DEBUG
                    BLYNK_LOG("Invalid pinMode %u -> %s", pin, it.asStr());
#endif
                }
                ++it;
            }
        }

        // Should be 1 parameter (value)
        if (++it >= param.end())
            return;

        if (!strcmp(cmd, "dw")) {
            //BLYNK_LOG("digitalWrite %d -> %d", pin, it.asInt());
            digitalWrite(pin, it.asInt() ? HIGH : LOW);
        } else if (!strcmp(cmd, "aw")) {
            //BLYNK_LOG("analogWrite %d -> %d", pin, it.asInt());
            analogWrite(pin, it.asInt());
        } else {
            BLYNK_LOG("Invalid HW cmd: %s", cmd);
        }
    }
}

#endif
