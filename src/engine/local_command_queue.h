// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstddef>
#include <functional>
#include <utility>

namespace swgb {

// Original normal-local command storage: 431662, 446970 and 446b90.
// Dispatch scans physical slots, so wrapping does not preserve FIFO order.
// Recording, replay and network command ordering are outside this helper.
class LocalCommandQueue {
public:
    using Command = std::function<void()>;
    static constexpr std::size_t capacity = 2000;

    bool enqueue(Command command) {
        if (!command) return false;
        auto &slot = slots_[nextWrite_];
        slot.command = std::move(command);
        slot.pending = true;
        nextWrite_ = (nextWrite_ + 1) % capacity;
        return true;
    }

    void drain() {
        for (;;) {
            auto slot = slots_.begin();
            while (slot != slots_.end() && !slot->pending) ++slot;
            if (slot == slots_.end()) return;
            // A callback can enqueue over its own slot, clear, or reset the queue.
            // Keep its payload alive independently of that slot while invoking it.
            Command command = slot->command;
            slot->pending = false;
            command();
        }
    }

    // Original 4468a0 retains payloads and the write cursor.
    void clear() {
        for (auto &slot : slots_) slot.pending = false;
    }

    // New session: release all captured state as well as resetting the cursor.
    void reset() {
        for (auto &slot : slots_) {
            slot.pending = false;
            slot.command = {};
        }
        nextWrite_ = 0;
    }

private:
    struct Slot {
        Command command;
        bool pending = false;
    };
    std::array<Slot, capacity> slots_{};
    std::size_t nextWrite_ = 0;
};

} // namespace swgb
