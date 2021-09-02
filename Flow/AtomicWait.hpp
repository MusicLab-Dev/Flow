/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Atomic Wait
 */

#pragma once

#ifndef _WIN32
# include <atomic_wait>
#else

# include <atomic>

namespace atomic_sync
{
    using std::atomic_wait_explicit;
    using std::atomic_wait;
    using std::atomic_notify_one;
    using std::atomic_notify_all;
}

#endif