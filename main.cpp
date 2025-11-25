#include "ObjectStorage.h"
#include <iostream>
#include <thread>

#define NUM_ELEMS 10

void print(ObjectStorage<int, int, NUM_ELEMS>& objects)
{
    for (int idx = 0; idx < NUM_ELEMS; ++idx)
        std::cout << "Key: " << idx << " | Obj: " << (objects.get(idx) ? *objects.get(idx) : -1) << std::endl;
}

int main()
{
    ObjectStorage<int, int, NUM_ELEMS> objects;
    std::array<int, NUM_ELEMS> testArr = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    objects.add(0, &testArr[0]);
    objects.add(1, &testArr[1]);
    objects.add(2, &testArr[2]);

    print(objects);

    if (!objects.add(1, &testArr[1]))
        std::cout << "Add not allowed, duplicate entry" << std::endl;

    objects.remove(1);
    print(objects);

    if (objects.add(1, &testArr[1]))
        std::cout << "Add allowed, entry removed first" << std::endl;

    print(objects);

    // Let NUM_ELEMS threads add an entry to the list concurrently
    std::cout << "Testing threads:" << std::endl;
    std::vector<std::thread> threads;
    for (int idx = 0; idx < NUM_ELEMS; ++idx)
        threads.emplace_back([&objects, &testArr, idx](){ objects.add(idx, &testArr[idx]); });

    // Wait for the threads to finish
    for (auto& thread : threads)
    {
        if (thread.joinable())
            thread.join();
    }

    print(objects);
}