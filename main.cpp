#include <iostream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

#define MAX_ELEMENTS_COUNT 3

using namespace std;
using namespace chrono;

vector<int> threadsCount = {4, 8, 16, 32, 64, 128, 256};
vector<int> arrSizes = {10000, 100000, 1000000, 10000000, 100000000/*, 1000000000*/};

vector<int> currentAnswers;

void arrayFilling (vector<int> & array, int arrSize) {
    for (int i = 0; i < arrSize; i++) {
        array.push_back(rand()); // is not limit for data
    }
}

void arraySorting (int * array) {
    for (int i = 0; i < MAX_ELEMENTS_COUNT - 1; i++) {
        for (int j = 0; j < MAX_ELEMENTS_COUNT - i - 1; j++) {
            if (array[j] > array[j + 1]) {
                swap(array[j], array[j + 1]);
            }
        }
    }
}

void atomicIntArraySorting(atomic<int> *array) {
    for (int i = 0; i < MAX_ELEMENTS_COUNT - 1; i++) {
        for (int j = 0; j < MAX_ELEMENTS_COUNT - i - 1; j++) {
            int j_val = array[j].load();
            int j_plus_1_val = array[j + 1].load();

            if (j_val > j_plus_1_val) {

                while (!array[j].compare_exchange_weak(j_val, j_plus_1_val)) {}
                while (!array[j + 1].compare_exchange_weak(j_plus_1_val, j_val)) {}
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

void localCASAlgorithm(vector<int> &array, vector<atomic<int>> &maxElements, int startIndex, int endIndex) {
    vector<int> localMaxElements(MAX_ELEMENTS_COUNT);

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

    for (int i = 0; i < MAX_ELEMENTS_COUNT; i++) {

        int localValue = localMaxElements[localMaxElements.size() - 1 - i];
        int currValue = maxElements[0].load();

        if (localValue > currValue) {

            while (!maxElements[0].compare_exchange_weak(currValue, localValue)) { }

                atomicIntArraySorting(&maxElements[0]);
            }

            else {

                break;
            }
    }
}

void CASAlgorithm (vector<int> &array, int threadsCount) {

    vector<atomic<int>> maxElements(MAX_ELEMENTS_COUNT);

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

        threads.emplace_back(localCASAlgorithm, ref(array), ref(maxElements), startIndex, endIndex);

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
        cout << "=========================" << endl;

        cout << "Array size: " << arrSizes[i] << endl;
        cout << "-------------------------" << endl;

        vector<int> array;
        arrayFilling(array, arrSizes[i]);

        auto start = high_resolution_clock::now();

        simpleAlgorithm(array);

        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        cout << "Simple algorithm: " << duration.count() << " ms" << endl;

        cout << "- - - - - - - - - - - - -" << endl;

        cout << "Threads\t| Mutex\t| CAS" << endl;

        cout << "- - - - - - - - - - - - -" << endl;

        for (int j = 0; j < threadsCount.size(); j++) {

            cout << threadsCount[j] << "\t| ";

            start = high_resolution_clock::now();

            mutexAlgorithm(array, threadsCount[j]);

            end = high_resolution_clock::now();
            duration = duration_cast<milliseconds>(end - start);
            cout << duration.count() << " ms\t| ";

            start = high_resolution_clock::now();

            CASAlgorithm(array, threadsCount[j]);

            end = high_resolution_clock::now();
            duration = duration_cast<milliseconds>(end - start);
            cout << duration.count() << " ms" << endl;
        }
        cout << "=========================" << endl << endl;
    }
}

int main() {

    srand(time(0));

    task();

    return 0;
}