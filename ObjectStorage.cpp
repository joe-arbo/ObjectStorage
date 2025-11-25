#include "ObjectStorage.h"

/// @brief Adds an object to the storage
/// @param key The key to insert the object with
/// @param obj The object pointer to be inserted
/// @return True if the object was successfully added, false otherwise
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

/// @brief Marks an object as removed such that subsequent get calls fail
/// @param key The key of the object to be removed
/// @return True if removal was successful, false otherwise
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

/// @brief Retrieves an object from storage
/// @param key The key of the object to be retrieved
/// @return A pointer to the retrieved object
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

/// @brief Returns all keys of non-removed objects by reference
/// @param keyArray The array to be filled with the keys, avoids dynamic memory allocation
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

/// @brief Generate a hash index from the given key
/// @param key The key to be hashed
/// @return The hashed index, wrapped to fit within the bounds of the storage
template <typename K, typename T, int N>
int ObjectStorage<K, T, N>::hashIndex(const K& key) const
{
    return m_hasher(key) % N;
}
