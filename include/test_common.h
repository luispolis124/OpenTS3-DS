#ifndef OTS3_TEST_COMMON_H
#define OTS3_TEST_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;
static int g_checks = 0;

#define CHECK(cond) do { g_checks++; if (!(cond)) { g_failures++; \
    fprintf(stderr, "  FALHOU %s:%d: %s\n", __FILE__, __LINE__, #cond); } } while (0)
#define CHECK_EQ(a, b) do { long long _a = (long long)(a), _b = (long long)(b); g_checks++; \
    if (_a != _b) { g_failures++; fprintf(stderr, "  FALHOU %s:%d: %s == %s (%lld != %lld)\n", \
    __FILE__, __LINE__, #a, #b, _a, _b); } } while (0)

static int Test_Finish(const char* name)
{
    printf("%-14s %s  (%d verificacoes, %d falhas)\n", name, g_failures ? "FALHOU" : "ok    ", g_checks, g_failures);
    return g_failures ? 1 : 0;
}

/* gerador pseudo-aleatório do teste (independente do motor) */
static unsigned int g_lcg = 12345u;
static unsigned int Test_Rand(void)
{
    g_lcg = g_lcg * 1664525u + 1013904223u;
    return g_lcg >> 8;
}

#endif
