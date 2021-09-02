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
    template <class _Tp, class _Tv>
    void atomic_wait_explicit(std::atomic<_Tp> const* a, _Tv val, std::memory_order order) { std::atomic_wait_explicit(a, val, order); }
    template <class _Tp, class _Tv>
    void atomic_wait(std::atomic<_Tp> const* a, _Tv val) { std::atomic_wait(a, val, std::memory_order_seq_cst); }
    template <class _Tp>
    void atomic_notify_one(std::atomic<_Tp> const* a) { std::atomic_notify_one(a); }
    template <class _Tp>
    void atomic_notify_all(std::atomic<_Tp> const* a) { std::atomic_notify_all(a); }
}

#endif