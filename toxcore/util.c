/*
 * Utilities.
 */

/*
 * Copyright © 2016-2017 The TokTok team.
 * Copyright © 2013 Tox project.
 * Copyright © 2013 plutooo
 *
 * This file is part of Tox, the free peer to peer instant messenger.
 * This file is donated to the Tox Project.
 *
 * Tox is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Tox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tox.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif

#include "util.h"

#include "crypto_core.h" /* for CRYPTO_PUBLIC_KEY_SIZE */
#include "network.h" /* for current_time_monotonic */

#include <stdlib.h>
#include <string.h>
#include <time.h>


/* don't call into system billions of times for no reason */
struct Unix_Time {
    uint64_t time;
    uint64_t base_time;
};

Unix_Time *unix_time_new(void)
{
    Unix_Time *unixtime = (Unix_Time *)malloc(sizeof(Unix_Time));

    if (unixtime == nullptr) {
        return nullptr;
    }

    unixtime->time = 0;
    unixtime->base_time = 0;

    return unixtime;
}

void unix_time_free(Unix_Time *unixtime)
{
    free(unixtime);
}

void unix_time_update_r(Unix_Time *unixtime)
{
    if (unixtime->base_time == 0) {
        unixtime->base_time = ((uint64_t)time(nullptr) - (current_time_monotonic() / 1000ULL));
    }

    unixtime->time = (current_time_monotonic() / 1000ULL) + unixtime->base_time;
}

uint64_t unix_time_get(const Unix_Time *unixtime)
{
    return unixtime->time;
}

int unix_time_is_timeout(const Unix_Time *unixtime, uint64_t timestamp, uint64_t timeout)
{
    return timestamp + timeout <= unix_time_get(unixtime);
}

static Unix_Time global_time;

/* XXX: note that this is not thread-safe; if multiple threads call unix_time_update() concurrently, the return value of
 * unix_time() may fail to increase monotonically with increasing time */
void unix_time_update(void)
{
    unix_time_update_r(&global_time);
}
uint64_t unix_time(void)
{
    return unix_time_get(&global_time);
}
int is_timeout(uint64_t timestamp, uint64_t timeout)
{
    return unix_time_is_timeout(&global_time, timestamp, timeout);
}


/* id functions */
bool id_equal(const uint8_t *dest, const uint8_t *src)
{
    return public_key_cmp(dest, src) == 0;
}

uint32_t id_copy(uint8_t *dest, const uint8_t *src)
{
    memcpy(dest, src, CRYPTO_PUBLIC_KEY_SIZE);
    return CRYPTO_PUBLIC_KEY_SIZE;
}

void host_to_net(uint8_t *num, uint16_t numbytes)
{
#ifndef WORDS_BIGENDIAN
    uint32_t i;
    VLA(uint8_t, buff, numbytes);

    for (i = 0; i < numbytes; ++i) {
        buff[i] = num[numbytes - i - 1];
    }

    memcpy(num, buff, numbytes);
#endif
}

int create_recursive_mutex(pthread_mutex_t *mutex)
{
    pthread_mutexattr_t attr;

    if (pthread_mutexattr_init(&attr) != 0) {
        return -1;
    }

    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0) {
        pthread_mutexattr_destroy(&attr);
        return -1;
    }

    /* Create queue mutex */
    if (pthread_mutex_init(mutex, &attr) != 0) {
        pthread_mutexattr_destroy(&attr);
        return -1;
    }

    pthread_mutexattr_destroy(&attr);

    return 0;
}

int32_t max_s32(int32_t a, int32_t b)
{
    return a > b ? a : b;
}

uint64_t min_u64(uint64_t a, uint64_t b)
{
    return a < b ? a : b;
}
