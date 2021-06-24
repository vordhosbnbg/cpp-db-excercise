#include <chrono>
#include <iostream>
#include <ratio>
#include <typeinfo>
#include <functional>
#include "qbrecord.h"
#include "indexdb.h"
#include "demangle.h"

/**
    Utility to populate a record collection
    prefix - prefix for the string value for every record
    numRecords - number of records to populate in the collection
*/

template<class Collection, class Record>
Collection populateDummyData(const std::string& prefix, unsigned int numRecords, int repeatEach)
{
    Collection data;
    data.reserve(numRecords);

    for (unsigned int id = 0; id < numRecords; ++id)
    {
        Record rec = { id, prefix + std::to_string(id), id % repeatEach, std::to_string(id) + prefix };
        data.emplace_back(rec);
    }
    return data;
}


// Starting from id 0 and increasing by interval,
// attempts to delete records from the collection
template<class Collection>
void deleteEeachIds(Collection& collection, unsigned int numRecords, unsigned int interval)
{
    for(unsigned int id = 0; id < numRecords; id += interval)
    {
        collection.deleteById(id);
    }
}


// Inserts numRecords
// Does 3 filtering operations on the three non-id columns
// Deletes 5% of the records
template<class Collection,
         class Record,
         int numRecords>
void doFindTest()
{
    static_assert(numRecords >= 1000, "numRecords should be > 1000");
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
    std::cout << "Time for insertion of " << data.size() << " records: "
              << double((endTimer - startTimer).count()) *
                 high_resolution_clock::period::num /
                 high_resolution_clock::period::den
              << " sec."
              << std::endl;

    // Find a record that contains and measure the perf

    std::string stringKey = "testdata" + std::to_string(freq1);
    std::string numKey = std::to_string(freq2);
    startTimer = high_resolution_clock::now();
    std::vector<Record> filteredSet = data.filter("column1", stringKey);
    std::vector<Record> filteredSet2 = data.filter("column2", numKey);
    std::vector<Record> filteredSet3 = data.filter("column3", "66testdata");
    endTimer = high_resolution_clock::now();
    bool valid = (filteredSet.size() == 11) &&
                 (filteredSet2.size() == numRecords/repeatEach) &&
                 (filteredSet3.size() == numRecords/100);
    std::cout << "Time for 3 filter operations, returning " << filteredSet.size()
              << ", " << filteredSet2.size() << " and " << filteredSet3.size() << " records : "
              << double((endTimer - startTimer).count()) *
                 high_resolution_clock::period::num /
                 high_resolution_clock::period::den
              << " sec."
              << std::endl;

    startTimer = high_resolution_clock::now();
    deleteEeachIds(data, numRecords, 20);
    endTimer = high_resolution_clock::now();
    valid = valid && (data.size() == numRecords * 0.95);
    std::cout << "Time for deleting 10% of the records (remaining - " << data.size() << ") : "
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


    doFindTest<IndexDb<100>,
               IndexDb<100>::Record,
               100000>();

    doFindTest<IndexDb<10>,
               IndexDb<10>::Record,
               100000>();

    return 0;
}
