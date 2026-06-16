#pragma once
// ═══════════════════════════════════════════════════════════════════════════
//  su_config.hpp - Configurações multiplataforma (Linux + Embarcados)
// ═══════════════════════════════════════════════════════════════════════════

#include <cstddef>
#include <cstdint>

// ═══════════════════════════════════════════════════════════════════════════
//  DETECÇÃO DE PLATAFORMA
// ═══════════════════════════════════════════════════════════════════════════

// Linux Desktop (x86_64)
#if defined(__linux__) && (defined(__x86_64__) || defined(__i386__))
    #define SU_PLATFORM_LINUX_X86 1
    #define SU_PLATFORM_DESKTOP 1

// Linux ARM (Raspberry Pi)
#elif defined(__linux__) && defined(__arm__)
    #define SU_PLATFORM_LINUX_ARM 1
    #define SU_PLATFORM_DESKTOP 1

// Linux ARM64 (Raspberry Pi 4/5)
#elif defined(__linux__) && defined(__aarch64__)
    #define SU_PLATFORM_LINUX_ARM64 1
    #define SU_PLATFORM_DESKTOP 1

// ESP32
#elif defined(ESP32)
    #define SU_PLATFORM_ESP32 1
    #define SU_PLATFORM_EMBEDDED 1
    #define SU_USE_ARENA 1

// ESP8266
#elif defined(ESP8266)
    #define SU_PLATFORM_ESP8266 1
    #define SU_PLATFORM_EMBEDDED 1
    #define SU_USE_ARENA 1

// Raspberry Pi Pico
#elif defined(ARDUINO_ARCH_RP2040) || defined(RASPBERRY_PI_PICO)
    #define SU_PLATFORM_RP2040 1
    #define SU_PLATFORM_EMBEDDED 1
    #define SU_USE_ARENA 1

// Arduino AVR (Uno/Nano/Mega)
#elif defined(__AVR__)
    #define SU_PLATFORM_ARDUINO_AVR 1
    #define SU_PLATFORM_EMBEDDED 1
    #define SU_USE_ARENA 1

// Padrão (assume Linux)
#else
    #define SU_PLATFORM_LINUX_X86 1
    #define SU_PLATFORM_DESKTOP 1
#endif

// ═══════════════════════════════════════════════════════════════════════════
//  CONFIGURAÇÕES POR PLATAFORMA
// ═══════════════════════════════════════════════════════════════════════════

// Linux Desktop (recursos abundantes)
#ifdef SU_PLATFORM_LINUX_X86
    #define SU_GC_THRESHOLD     (8 * 1024 * 1024)
    #define SU_GC_STEP_BYTES    (16 * 1024)
    #define SU_MAX_OBJECTS      131072
    #define SU_MAX_VARS         65536
    #define SU_MAX_STRING_LEN   65536
    #define SU_ENABLE_BIGINT    1
    #define SU_ENABLE_FLOAT     1
    #define SU_USE_FULL_GC      1
#endif

// Linux ARM
#ifdef SU_PLATFORM_LINUX_ARM
    #define SU_GC_THRESHOLD     (2 * 1024 * 1024)
    #define SU_GC_STEP_BYTES    (4 * 1024)
    #define SU_MAX_OBJECTS      32768
    #define SU_MAX_VARS         16384
    #define SU_MAX_STRING_LEN   16384
    #define SU_ENABLE_BIGINT    1
    #define SU_ENABLE_FLOAT     1
    #define SU_USE_FULL_GC      1
#endif

// Linux ARM64
#ifdef SU_PLATFORM_LINUX_ARM64
    #define SU_GC_THRESHOLD     (4 * 1024 * 1024)
    #define SU_GC_STEP_BYTES    (8 * 1024)
    #define SU_MAX_OBJECTS      65536
    #define SU_MAX_VARS         32768
    #define SU_MAX_STRING_LEN   32768
    #define SU_ENABLE_BIGINT    1
    #define SU_ENABLE_FLOAT     1
    #define SU_USE_FULL_GC      1
#endif

// ESP32
#ifdef SU_PLATFORM_ESP32
    #define SU_GC_THRESHOLD     (64 * 1024)
    #define SU_GC_STEP_BYTES    (512)
    #define SU_MAX_OBJECTS      2048
    #define SU_MAX_VARS         512
    #define SU_MAX_STRING_LEN   4096
    #define SU_ENABLE_BIGINT    1
    #define SU_ENABLE_FLOAT     1
    #define SU_USE_LIGHT_GC     1
    #define SU_ARENA_SIZE       (128 * 1024)
#endif

// ESP8266
#ifdef SU_PLATFORM_ESP8266
    #define SU_GC_THRESHOLD     (16 * 1024)
    #define SU_GC_STEP_BYTES    (256)
    #define SU_MAX_OBJECTS      512
    #define SU_MAX_VARS         256
    #define SU_MAX_STRING_LEN   2048
    #define SU_ENABLE_BIGINT    0
    #define SU_ENABLE_FLOAT     1
    #define SU_USE_LIGHT_GC     1
    #define SU_ARENA_SIZE       (32 * 1024)
#endif

// RP2040
#ifdef SU_PLATFORM_RP2040
    #define SU_GC_THRESHOLD     (32 * 1024)
    #define SU_GC_STEP_BYTES    (256)
    #define SU_MAX_OBJECTS      1024
    #define SU_MAX_VARS         512
    #define SU_MAX_STRING_LEN   4096
    #define SU_ENABLE_BIGINT    1
    #define SU_ENABLE_FLOAT     1
    #define SU_USE_LIGHT_GC     1
    #define SU_ARENA_SIZE       (64 * 1024)
#endif

// Arduino AVR
#ifdef SU_PLATFORM_ARDUINO_AVR
    #define SU_GC_THRESHOLD     (512)
    #define SU_GC_STEP_BYTES    (32)
    #define SU_MAX_OBJECTS      64
    #define SU_MAX_VARS         32
    #define SU_MAX_STRING_LEN   64
    #define SU_ENABLE_BIGINT    0
    #define SU_ENABLE_FLOAT     0
    #define SU_USE_SIMPLE_GC    1
    #define SU_ARENA_SIZE       (2 * 1024)
    #define SU_NO_STL           1
#endif

// ═══════════════════════════════════════════════════════════════════════════
//  VALORES PADRÃO
// ═══════════════════════════════════════════════════════════════════════════

#ifndef SU_GC_THRESHOLD
    #define SU_GC_THRESHOLD (1024 * 1024)
#endif

#ifndef SU_MAX_OBJECTS
    #define SU_MAX_OBJECTS 4096
#endif

#ifndef SU_MAX_VARS
    #define SU_MAX_VARS 1024
#endif

#ifndef SU_MAX_STRING_LEN
    #define SU_MAX_STRING_LEN 4096
#endif

#ifndef SU_ENABLE_BIGINT
    #define SU_ENABLE_BIGINT 1
#endif

#ifndef SU_ENABLE_FLOAT
    #define SU_ENABLE_FLOAT 1
#endif

// ═══════════════════════════════════════════════════════════════════════════
//  MACROS DE COMPATIBILIDADE
// ═══════════════════════════════════════════════════════════════════════════

#ifdef SU_NO_STL
    #define SU_USE_STATIC_VECTORS 1
#else
    #include <vector>
    #include <unordered_map>
    #include <string>
#endif