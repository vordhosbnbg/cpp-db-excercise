#include <chrono>
#include <iostream>
#include <ratio>
#include <typeinfo>
#include <functional>
#include "qbrecord.h"
#include "indexdb.h"

#ifdef __GNUG__
#include <cxxabi.h>
#define DEMANGLE(x) abi::__cxa_demangle(x, NULL, NULL, NULL)
#else
#define DEMANGLE(x) x
#endif

/**
    Utility to populate a record collection
    prefix - prefix for the string value for every record
    numRecords - number of records to populate in the collection
*/

template<class Collection, class Record>
Collection populateDummyData(const std::string& prefix, int numRecords, int repeatEach)
{
    Collection data;
    data.reserve(numRecords);

    for (uint i = 0; i < numRecords; i++)
    {
        Record rec = { i, prefix + std::to_string(i), i % repeatEach, std::to_string(i) + prefix };
        data.emplace_back(rec);
    }
    return data;
}



template<class Collection,
         class Record,
         int numRecords>
void doFindTest()
{
    static_assert(numRecords > 1000, "numRecords should be > 1000");
    static_assert(numRecords % 1000 == 0, "numRecords should be divisible to 1000");
    using namespace std::chrono;
    constexpr size_t freq1 = numRecords/20;
    constexpr size_t freq2 = 500;
    constexpr int repeatEach = 1000;
    // populate a bunch of data
    std::cout << "Collection type " << DEMANGLE(typeid(Collection).name())
                  << ",  recordType " << DEMANGLE(typeid(Record).name()) << std::endl;
    auto startTimer = high_resolution_clock::now();
    Collection data = populateDummyData<Collection, Record>("testdata", numRecords, repeatEach);
    auto endTimer = high_resolution_clock::now();
    std::cout << "Time for insertion"
              << double((endTimer - startTimer).count()) *
                 high_resolution_clock::period::num /
                 high_resolution_clock::period::den
              << " sec."
              << std::endl;

    // Find a record that contains and measure the perf

    std::string stringKey = "testdata" + std::to_string(freq1);
    std::string numKey = std::to_string(freq2);
    startTimer = high_resolution_clock::now();
    Collection filteredSet = data.filter("column1", stringKey);
    Collection filteredSet2 = data.filter("column2", numKey);
    endTimer = high_resolution_clock::now();
    bool valid = (filteredSet.size() == 11) && (filteredSet2.size() == numRecords/repeatEach);
    std::cout << "Time for 2 filter operations:"
              << double((endTimer - startTimer).count()) *
                 high_resolution_clock::period::num /
                 high_resolution_clock::period::den
              << " sec."
              << std::endl;

    std::cout << "Result: " << (valid ? "OK" : "NOK") << std::endl;
}


int main()
{

    doFindTest<QBRecordCollection,
               QBRecord,
               100000>();


    doFindTest<IndexDb<5>,
               IndexDb<5>::Record,
               100000>();

    return 0;
}
