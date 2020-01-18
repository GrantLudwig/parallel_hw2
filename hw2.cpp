//hw2.cpp
//Grant Ludwig

#include <iostream>
#include <vector>
#include <chrono>
#include <future>
using namespace std;

const int N = 1<<26;  // FIXME must be power of 2 for now
//const int N = 256;
typedef vector<int> Data;

class Heaper {
public:
    Heaper(const Data *data) : n(data->size()), data(data) {
        interior = new Data(n-1, 0);
    }

    virtual ~Heaper(){
        delete interior;
    }

protected:
    int n;
    const Data *data;
    Data *interior;

    virtual int size(){
        return (n-1) + n;
    }

    virtual int value(int i){
        if (i < n-1)
            return interior->at(i);
        else
            return data->at(i - (n-1));
    }

    virtual int parent(int i) {
        return (int) (i-1)/2;
    }

    virtual int left(int i) {
        return 2*i+1;
    }

    virtual int right(int i) {
        return 2*i+2;
    }

    virtual bool isLeaf(int i){
        return !(i < n-1);
    }
};

class SumHeap : public Heaper {
public:
    SumHeap(const Data *data) : Heaper(data) {
        calcSum(0, 0);
    }

    int sum(int node=0){
        return value(node);
    }

    void prefixSums(Data *prefix){
        calcPrefix(0, 0, prefix, 0);
    }

private:
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

    void calcPrefix(int i, int sumPrior, Data *prefix, int level) {
        if (isLeaf(i)){
            //sumPrior + self
            prefix->at(i-(n-1)) = sumPrior + value(i);
            return;
        }
        if (level > 2) {
            // left 0 + sumPrior
            calcPrefix(left(i), sumPrior, prefix, level+1);
            // right sumPrior + left sibling
            //int rightPrefix = sumPrior + value(left(i));
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
    // cout << "Data Vector:" << endl;
    // for (int elem: data) {
    //     cout << elem << " ";
    // }
    // cout << endl << "Prefix Vector:" << endl;
    // for (int elem: prefix) {
    //     cout << elem << " ";
    // }
    // cout << endl;

    // start timer
    auto start = chrono::steady_clock::now();

    // cout << "Here" << endl;
    SumHeap heap(&data);
    // cout << "Created heap" << endl;
    heap.prefixSums(&prefix);

    // stop timer
    auto end = chrono::steady_clock::now();
    auto elpased = chrono::duration<double,milli>(end-start).count();

    int check = 1;
    for (int elem: prefix) {
        //cout << "Run: " << elem << endl;
        if (elem != check++) {
            cout << "FAILED RESULT at " << check-1;
            break;
        }
    }

    cout << "in " << elpased << "ms" << endl;
    return 0;
}