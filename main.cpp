#include <iostream>
#include <ctime>
#include <cstdlib>
#include <vector>

#define MAX_ELEMENTS_COUNT 3

using namespace std;

int threadsCounts[] = {4, 8, 16, 32, 64, 128, 256};
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
    int maxElements[MAX_ELEMENTS_COUNT];
    int maxElementsSum;

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

    maxElementsSum = maxElements[0] + maxElements[1] + maxElements[2];
}

void task () {

    for (int i = 0; i < arrSizes.size(); i++) {

        vector<int> array;

        arrayFilling(array, arrSizes[i]);

        simpleAlgorithm(array);
    }
}

int main() {

    srand(time(0));

     vector<int> array = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    // arrayFilling(array, 100);
     simpleAlgorithm(array);

    //task();

    return 0;
}