#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

static uint32_t rotate_left(uint32_t value, unsigned amount) {
    return (value << amount) | (value >> (32 - amount));
}

static uint32_t rotate_right(uint32_t value, unsigned amount) {
    return (value >> amount) | (value << (32 - amount));
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t low = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t middle = (uint32_t)(challenge >> 16);

    uint32_t first = 5u * (low - high) + UINT32_C(0x215f);
    uint32_t second = (middle ^ UINT32_C(0xfba4)) + 8u * high;
    uint32_t third = 3u * second ^ (low + UINT32_C(0x3ea6));
    values[0] = third ^ first;
    return write_answer(out, cap, values, 1);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    values[0] = 2u * ((uint32_t)(challenge >> 25) & 15u) + 23u;
    for (uint32_t i = 1; i < 6; ++i) {
        unsigned shift = 11u + 2u * (i - 1u);
        uint32_t part = ((uint32_t)(challenge >> shift) & 31u) + i;
        values[i] = 3u * (part ^ values[i - 1]);
    }
    return write_answer(out, cap, values, 6);
}

static uint32_t phase_3_value(uint32_t index, uint32_t value) {
    switch (index) {
        case 0: return (value ^ UINT32_C(0x6278)) + (value >> 2);
        case 1: return 5u * value + UINT32_C(0x1ce80);
        case 2: return 3u * value + UINT32_C(0x0ba3);
        case 3: return 2u * value ^ UINT32_C(0xc6d9);
        case 4: return 4u * value ^ UINT32_C(0x40ee);
        case 5: return 3u * value + UINT32_C(0x57e8);
        case 6: return (value ^ UINT32_C(0x744c)) + (value >> 1);
        default: return 5u * value + UINT32_C(0x2bde0);
    }
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t index = (uint32_t)challenge & 7u;
    uint32_t value = (uint32_t)(challenge >> 3) ^ (uint32_t)(challenge >> 32);
    values[0] = index;
    values[1] = phase_3_value(index, value);
    return write_answer(out, cap, values, 2);
}

static uint32_t phase_4_walk(uint32_t target, uint32_t low, uint32_t high,
                             uint32_t salt) {
    uint32_t middle = low + (high - low) / 2u;
    if (target == middle) return (middle + UINT32_C(0x6d)) ^ salt;
    if (target < middle) {
        uint32_t next = phase_4_walk(target, low, middle - 1u, salt);
        return (3u * next ^ (middle + salt)) ^ UINT32_C(0x8);
    }
    uint32_t next = phase_4_walk(target, middle + 1u, high, salt);
    return (3u * next ^ (middle + salt)) ^ UINT32_C(0xc);
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t target = (uint32_t)challenge & 127u;
    uint32_t salt = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);
    values[0] = phase_4_walk(target, 0, 127, salt);
    return write_answer(out, cap, values, 1);
}

struct phase_5_node {
    unsigned next;
    uint32_t value;
    uint32_t tag;
};

static const struct phase_5_node phase_5_nodes[] = {
    {11, 4,    UINT32_C(0x5495)}, {0,  595,  UINT32_C(0x93d6)},
    {4,  2632, UINT32_C(0xf766)}, {7,  1858, UINT32_C(0x89bb)},
    {3,  1629, UINT32_C(0x8a5b)}, {0,  369,  UINT32_C(0x7e5e)},
    {5,  848,  UINT32_C(0x137a)}, {9,  1088, UINT32_C(0x3106)},
    {2,  1336, UINT32_C(0xccab)}, {1,  2163, UINT32_C(0x5ad8)},
    {8,  2939, UINT32_C(0x4865)}, {10, 2438, UINT32_C(0x6d61)}
};

static const unsigned phase_5_heads[] = {6, 5, 0, 11, 10, 8, 2, 4};

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    unsigned node_index = phase_5_heads[(uint32_t)challenge & 7u];
    const struct phase_5_node *node = &phase_5_nodes[node_index];
    uint32_t total = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    total ^= node->tag ^ UINT32_C(0x2a2a);
    total += node->value;

    unsigned steps = (unsigned)((challenge >> 3) & 3u);
    for (unsigned i = 0; i < steps; ++i) {
        node = &phase_5_nodes[node->next];
        total ^= node->tag;
        total += node->value;
    }
    values[0] = total;
    values[1] = node->value;
    return write_answer(out, cap, values, 2);
}

static uint32_t phase_6_operation(unsigned index, uint32_t value) {
    switch (index) {
        case 0:
        case 3:
            return rotate_left(value, 2) ^ UINT32_C(0x86da);
        case 1:
        case 5:
            return (value ^ (value >> 5)) + UINT32_C(0x3e6c);
        case 2:
            return ((value << 5) + UINT32_C(0x2e95)) ^ (7u * value);
        case 4:
            return rotate_right(value, 2);
        case 6:
            return (value + UINT32_C(0x69bf)) ^ (value >> 1);
        default:
            return (value ^ UINT32_C(0x18ab)) + (value << 7);
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t low = (uint32_t)challenge;
    uint32_t value = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    value = phase_6_operation(low & 7u, value);
    value = phase_6_operation((low >> 3) & 7u, value ^ UINT32_C(0x4613));
    values[0] = value + UINT32_C(0xea73);
    return write_answer(out, cap, values, 1);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t challenge_fold = (uint32_t)challenge ^ (uint32_t)(challenge >> 32);
    uint32_t context_fold = (uint32_t)context ^ (uint32_t)(context >> 32);
    uint32_t first = (uint32_t)context & 3u;
    uint32_t second = rotate_left(challenge_fold ^ context_fold, 7);
    second = (second + UINT32_C(0x7911)) ^ UINT32_C(0x3ea2);
    second ^= first * UINT32_C(0x7911) ^ (uint32_t)(context >> 16);
    uint32_t third = second ^ (context_fold + first + UINT32_C(0x3ea2)
                               + (challenge_fold >> 7));

    int size = snprintf(out, cap,
                        "................................OPEN%" PRIu32 " %" PRIu32 " %" PRIu32,
                        first, second, third);
    return size < 0 || (size_t)size >= cap;
}
