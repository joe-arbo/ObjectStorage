#include "ObjectStorage.h"

template <typename K, typename T, int N>
bool ObjectStorage<K, T, N>::add(K key, const T* obj)
{
    // Get the hashed index of the key
    int index = hashIndex(key);
    for (int idx = 0; idx < N; ++idx)
    {
        // Try to claim the slot in the hash table and update it
        StorageObj& storageObj = m_objects[index];
        SlotState empty = SlotState::EMPTY; 
        SlotState removed = SlotState::REMOVED;
        if (storageObj.state.compare_exchange_strong(empty, SlotState::LOCKED) ||
            storageObj.state.compare_exchange_strong(removed, SlotState::LOCKED))
        {
            storageObj.key = key;
            storageObj.obj = obj;
            storageObj.state = SlotState::FILLED;
            return true;
        }

        // Report failure for duplicate entries (this is not required)
        if (storageObj.state.load() == SlotState::FILLED && storageObj.key == key)
            return false;

        // Increment the index with the size of the list in mind
        index = (index + 1) % N;
    }

    // Hash table is full
    return false;
}

template <typename K, typename T, int N>
bool ObjectStorage<K, T, N>::remove(K key)
{
    // Get the hashed index of the key
    int index = hashIndex(key);
    for (int idx = 0; idx < N; ++idx)
    {
        // If the slot is empty, report failure, we don't need to keep looking
        StorageObj& storageObj = m_objects[index];
        if (storageObj.state.load() == SlotState::EMPTY)
            return false;
        
        // If the slot is filled and the key matches, make the slot as removed
        if (storageObj.state.load() == SlotState::FILLED && storageObj.key == key)
        {
            storageObj.state = SlotState::REMOVED;
            return true;
        }

        // Increment the index with the size of the list in mind
        index = (index + 1) % N;
    }

    return false;
}

template <typename K, typename T, int N>
const T* ObjectStorage<K, T, N>::get(K key) const
{
    // Get the hashed index of the key
    int index = hashIndex(key);
    for (int idx = 0; idx < N; ++idx)
    {
        // If the slot is empty, this object does not exist in the map
        const StorageObj& storageObj = m_objects[index];
        if (storageObj.state.load() == SlotState::EMPTY)
            return nullptr;

        if (storageObj.state.load() == SlotState::FILLED && storageObj.key == key)
            return storageObj.obj;
        
        // Increment the index with the size of the list in mind
        index = (index + 1) % N;
    }

    return nullptr;
}

template <typename K, typename T, int N>
void ObjectStorage<K, T, N>::getAll(std::array<K, N>& keyArray) const
{
    // Bounds check the input array
    if (keyArray.size() < N)
        return;
    
    // Collect all the filled slots
    for (int idx = 0; idx < N; ++idx)
    {
        const StorageObj& storageObj = m_objects[idx];
        if (storageObj.state.load() == SlotState::FILLED)
            keyArray[idx] = storageObj.key;
    }
}

template <typename K, typename T, int N>
int ObjectStorage<K, T, N>::hashIndex(const K& key) const
{
    return m_hasher(key) % N;
}
