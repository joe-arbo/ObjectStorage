#pragma once

#include <array>
#include <atomic>
#include <functional>

// Thread safe, non-locking, object storage class that takes its size at creation
template <typename K, typename T, int N>
class ObjectStorage
{
public:
    ObjectStorage() = default;
    ~ObjectStorage() = default;

    // Access methods
    bool add(K key, const T* obj);
    bool remove(K key);
    const T* get(K key) const;
    void getAll(std::array<K, N>& keyArray) const;

private:
    int hashIndex(const K& key) const;

    // Used to keep track of the slot state
    enum class SlotState
    {
        EMPTY,
        LOCKED,
        REMOVED,
        FILLED
    };

    // Object to be stored in the map
    struct StorageObj
    {
        std::atomic<K> key;
        std::atomic<const T*> obj;
        std::atomic<SlotState> state = SlotState::EMPTY;
    };

    std::array<StorageObj, N> m_objects;
    std::hash<K> m_hasher;
};
