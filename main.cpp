#include <iostream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

#define MAX_ELEMENTS_COUNT 3

using namespace std;

vector<int> threadsCount = {4, 8, 16, 32, 64, 128, 256};
vector<int> arrSizes = {1000, 5000, 10000};

void arrayFilling (vector<int> & array, int arrSize) {
    for (int i = 0; i < arrSize; i++) {
        array.push_back(rand() % 10000);
    }
}

// void arrayPrint(vector<int> *array) {
//     for (int i = 0; i < array->size(); i++) {
//         cout << (*array)[i] << " ";
//     }
//     cout << endl;
// }

void arraySorting (int * array) {
    for (int i = 0; i < MAX_ELEMENTS_COUNT - 1; i++) {
        for (int j = 0; j < MAX_ELEMENTS_COUNT - i - 1; j++) {
            if (array[j] > array[j + 1]) {
                swap(array[j], array[j + 1]);
            }
        }
    }
}

void simpleAlgorithm (vector<int> &array) {
    vector<int> maxElements;
    maxElements.resize(MAX_ELEMENTS_COUNT);
    int maxElementsSum = 0;

    for (int i = 0; i < MAX_ELEMENTS_COUNT; i++) {
        maxElements[i] = array[i];
    }

    arraySorting(&maxElements[0]);

    for (int i = MAX_ELEMENTS_COUNT; i < array.size(); i++) {

        if (array[i] > maxElements[0]) {
            maxElements[0] = array[i];
            arraySorting(&maxElements[0]);
        }
    }

    for (int i = 0; i < MAX_ELEMENTS_COUNT; i++) {
        maxElementsSum += maxElements[i];
    }
}

void localMutexAlgorithm (vector<int> &array, vector<int> &maxElements, int startIndex, int endIndex, mutex &mtx) { 

    vector<int> localMaxElements;
    localMaxElements.resize(MAX_ELEMENTS_COUNT);

    for (int i = 0; i < MAX_ELEMENTS_COUNT; i++) {
        localMaxElements[i] = array[i + startIndex];
    }

    arraySorting(&localMaxElements[0]);

    for (int i = MAX_ELEMENTS_COUNT + startIndex; i < endIndex; i++) {

        if (array[i] > localMaxElements[0]) {
            localMaxElements[0] = array[i];
            arraySorting(&localMaxElements[0]);
        }
    }

    lock_guard<mutex> lock(mtx);

    for (int i = 0; i < MAX_ELEMENTS_COUNT; i++) {
        if (localMaxElements[localMaxElements.size() - 1 - i] > maxElements[0]) {
            maxElements[0] = localMaxElements[localMaxElements.size() - 1 - i];
            arraySorting(&maxElements[0]);
        }

        else {
            break;
        }
    }
}

void mutexAlgorithm (vector<int> &array, int threadsCount) {

    vector <int> maxElements;
    maxElements.resize(MAX_ELEMENTS_COUNT);
    int maxElementsSum = 0;

    mutex mtx;
    vector<thread> threads;

    int elementsPerThread = array.size() / threadsCount;
    int remainingElements = array.size() % threadsCount;

    int startIndex = 0;

    for (int i = 0; i < threadsCount; i++) {

        int endIndex = startIndex + elementsPerThread;
        if (i < remainingElements) {
            endIndex++;
        }

        threads.emplace_back(localMutexAlgorithm, ref(array), ref(maxElements), startIndex, endIndex, ref(mtx));
        startIndex = endIndex;
    }

    for (int i = 0; i < threads.size(); i++) {
        threads[i].join();
    }

    for (int i = 0; i < MAX_ELEMENTS_COUNT; i++) {
        maxElementsSum += maxElements[i];
    }
}

// void localCASAlgorithm (vector<int> &array, vector<atomic<int>> &maxElements, int startIndex, int endIndex) {
//     vector<int> localMaxElements;
//     localMaxElements.resize(MAX_ELEMENTS_COUNT);

//     for (int i = 0; i < MAX_ELEMENTS_COUNT; i++) {
//         localMaxElements[i] = array[i + startIndex];
//     }

//     arraySorting(&localMaxElements[0]);

//     for (int i = MAX_ELEMENTS_COUNT + startIndex; i < endIndex; i++) {

//         if (array[i] > localMaxElements[0]) {
//             localMaxElements[0] = array[i];
//             arraySorting(&localMaxElements[0]);
//         }
//     }

//     for (int val : localMaxElements) {
//         for (int i = 0; i < MAX_ELEMENTS_COUNT; i++) {
//             int current = maxElements[i].load();
//             if (val > current) {
//                 // CAS: намагаємося оновити maxElements[i]
//                 while (!maxElements[i].compare_exchange_weak(current, val)) {
//                     // Якщо не вдалось — оновити current та повторити
//                     if (val <= current) break; // Якщо хтось уже вставив краще — не треба далі
//                 }
//                 break; // вставили — далі не йдемо вниз по списку
//             }
//         }
//     }

// }

void CASAlgorithm (vector<int> &array, int threadsCount) {

    vector<atomic<int>> maxElements;

    maxElements.resize(MAX_ELEMENTS_COUNT);
    int maxElementsSum = 0;

    vector<thread> threads;

    int elementsPerThread = array.size() / threadsCount;
    int remainingElements = array.size() % threadsCount;

    int startIndex = 0;

    for (int i = 0; i < threadsCount; i++) {

        int endIndex = startIndex + elementsPerThread;
        if (i < remainingElements) {
            endIndex++;
        }

        // threads.emplace_back(localCASAlgorithm, ref(array), ref(maxElements), startIndex, endIndex);

        startIndex = endIndex;
    }

    for (int i = 0; i < threads.size(); i++) {
        threads[i].join();
    }

    for (int i = 0; i < MAX_ELEMENTS_COUNT; i++) {
        maxElementsSum += maxElements[i].load();
    }
}

void task () {

    for (int i = 0; i < arrSizes.size(); i++) {

        vector<int> array;

        arrayFilling(array, arrSizes[i]);

        simpleAlgorithm(array);

        for (int j = 0; j < threadsCount.size(); j++) {

            mutexAlgorithm(array, threadsCount[j]);

            // CASAlgorithm(array, threadsCount[j]);
        }
    }
}

int main() {

    srand(time(0));

    task();

    return 0;
}