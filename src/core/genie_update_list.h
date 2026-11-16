// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

namespace swgb {
// Original4aef50/4aeff0/4af380: append, swap-last removal and live-count traversal.
// Payloads identify stable backing storage; they are never borrowed across callbacks.
class GenieUpdateList {
public:
    static constexpr size_t npos=std::numeric_limits<size_t>::max();
    size_t append(size_t payload) {
        assert(payload!=npos);
        const size_t slot=entries_.size();
        entries_.push_back(payload);
        return slot;
    }
    // Returns the payload moved into slot, or npos when removing the last entry.
    // The owner repairs that payload's stored slot; this class does not own objects.
    size_t remove(size_t slot) {
        assert(slot<entries_.size());
        const size_t moved=slot+1<entries_.size()?entries_.back():npos;
        if(moved!=npos)entries_[slot]=moved;
        entries_.pop_back();
        return moved;
    }
    size_t size() const { return entries_.size(); }
    bool empty() const { return entries_.empty(); }
    size_t operator[](size_t slot) const { assert(slot<entries_.size());return entries_[slot]; }
    void clear() { entries_.clear(); }

    // A true update result requires finalize to remove the current object.
    // False advances even after self-removal: original deferred sleep skips the swapped tail.
    template<class Update,class Finalize>
    void traverse(Update &&update,Finalize &&finalize) {
        size_t index=0;
        while(index<entries_.size()) {
            const size_t payload=entries_[index];
            if(update(payload))finalize(payload);
            else ++index;
        }
    }
private:
    std::vector<size_t> entries_;
};
}
