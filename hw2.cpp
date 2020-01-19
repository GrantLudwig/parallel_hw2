/** 
 * @file hw2.cpp
 * @author Grant Ludwig
 * @date 1/21/20
 * @brief This is an implementation of the Ladner-Ficher parallel prefix sum algorithm
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <future> // for async
using namespace std;

const int N = 1<<26;
typedef vector<int> Data;

/**
 * Heaper Class
 * @brief Creates a heap like structure where the original data is preserved at the end of the "created" array
 */
class Heaper {
public:
    /**
     * @brief Heaper constructor
     * @param Data Pointer to a vector of data
     */
    Heaper(const Data *data) : n(data->size()), data(data) {
        interior = new Data(n-1, 0);
    }

    virtual ~Heaper(){
        delete interior;
    }

protected:
    int n;
    const Data *data;
    Data *interior; // the interior array

    /**
     * @brief Gets the value of the passed in node
     * @param i Index of the node we want the value of
     */
    virtual int value(int i){
        if (i < n-1)
            return interior->at(i);
        else
            return data->at(i - (n-1));
    }

    /**
     * @brief Returns the index of the left child node of the passed in node
     * @param i Index of the node we want the left child of
     */
    virtual int left(int i) {
        return 2*i+1;
    }

    /**
     * @brief Returns the index of the right child node of the passed in node
     * @param i Index of the node we want the right child of
     */
    virtual int right(int i) {
        return 2*i+2;
    }

    /**
     * @brief Returns true if the node is a leaf node
     * @param i Index of the node we want to know is a leaf node
     */
    virtual bool isLeaf(int i){
        return !(i < n-1);
    }
};

/**
 * SumHeap Class
 * @brief Extends from the Heaper class
 * Calculates the sum and prefixSum
 */
class SumHeap : public Heaper {
public:
    /**
     * @brief SumHeap constructor
     * @param Data Pointer to a vector of data
     * Will calculate the sum once constructed
     */
    SumHeap(const Data *data) : Heaper(data) {
        calcSum(0, 0);
    }

    /**
     * @brief Returns the sum at that node
     * @param node Index of the node that we want to know the sum of
     */
    int sum(int node=0) {
        return value(node);
    }

    /**
     * @brief Sets the prefix array to the prefix sum
     * @param prefix Pointer to a vector to set the prefix sum to
     */
    void prefixSums(Data *prefix){
        calcPrefix(0, 0, prefix, 0);
    }

private:
    /**
     * @brief Calulates the sum of the heap through threading and recursion.
     * Will only use up to 8 threads.
     * @param i The index of the node
     * @param level Level of the current node
     */
    void calcSum(int i, int level) {
        if (isLeaf(i)){
            return;
        }
        if (level > 2) {
            calcSum(left(i), level+1);
            calcSum(right(i), level+1);
        }
        else {
            auto handle = async(launch::async, &SumHeap::calcSum, this, left(i), level+1);
            calcSum(right(i), level+1);
            handle.get();
        }
        interior->at(i) = value(left(i)) + value(right(i));
    }

    /**
     * @brief Calulates the prefix, placing it in the passed in prefix vector.
     * Works through threading and recursion.
     * Will only use up to 8 threads.
     * @param i The index of the node
     * @param sumPrior Sum of the prior node
     * @param prefix Pointer to a vector to set the prefix
     * @param level Level of the current node
     */
    void calcPrefix(int i, int sumPrior, Data *prefix, int level) {
        if (isLeaf(i)){
            //sumPrior + self
            prefix->at(i-(n-1)) = sumPrior + value(i);
            return;
        }
        if (level > 2) {
            // left: 0 + sumPrior
            calcPrefix(left(i), sumPrior, prefix, level+1);
            // right: sumPrior + left sibling
            // rightPrefix = sumPrior + value(left(i));
            calcPrefix(right(i), sumPrior + value(left(i)), prefix, level+1);
        }
        else {
            auto handle = async(launch::async, &SumHeap::calcPrefix, this, left(i), sumPrior, prefix, level+1);
            calcPrefix(right(i), sumPrior + value(left(i)), prefix, level+1);
            handle.get();
        }
    }
};

int main() {
    Data data(N, 1);  // put a 1 in each element of the data array
    Data prefix(N, 1);

    // start timer
    auto start = chrono::steady_clock::now();

    SumHeap heap(&data);
    heap.prefixSums(&prefix);

    // stop timer
    auto end = chrono::steady_clock::now();
    auto elpased = chrono::duration<double,milli>(end-start).count();

    int check = 1;
    for (int elem: prefix) {
        if (elem != check++) {
            cout << "FAILED RESULT at " << check-1;
            break;
        }
    }

    cout << "in " << elpased << "ms" << endl;
    return 0;
}