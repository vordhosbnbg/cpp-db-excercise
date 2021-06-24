#include <iostream>
#include "qbrecord.h"
#include "indexdb.h"
#include "demangle.h"

bool checkFn(const std::string& checkText, bool statement)
{
    std::cout << "Check if: " << checkText << (statement ? " --> OK " : " --> NOK") << std::endl;
    return statement;
}

#define TEST_CHECK(x) checkFn(#x, (x))


template<class Collection,
         class Record>
void executeTests()
{
    std::cout << "Testing " << DEMANGLE(typeid(Collection).name()) << std::endl;
    Collection collection;

    std::cout << "Adding record..." << std::endl;
    Record rec = {1, "data", 10, "test"};
    collection.emplace_back(rec);
    TEST_CHECK(collection.size() == 1);

    std::cout << "Adding second record..." << std::endl;
    rec = {2, "01234567890abcdef", 334, "testing"};
    collection.emplace_back(rec);
    TEST_CHECK(collection.size() == 2);

    std::cout << "Filter by column1..." << std::endl;
    std::vector<Record> result = collection.filter("column1", "345");
    TEST_CHECK(result.size() == 1);

    std::cout << "Filter by column2..." << std::endl;
    result = collection.filter("column2", "334");
    TEST_CHECK(result.size() == 1);

    std::cout << "Filter by column3 ..." << std::endl;
    result = collection.filter("column3", "test");
    TEST_CHECK(result.size() == 2);

    std::cout << "Delete 1 record and filter..." << std::endl;
    collection.deleteById(1);
    result = collection.filter("column3", "test");
    TEST_CHECK(result.size() == 1);
}

int main()
{

     executeTests<QBRecordCollection,
                  QBRecord>();


     executeTests<IndexDb<100>,
                  IndexDb<100>::Record>();

     executeTests<IndexDb<1>,
                  IndexDb<1>::Record>();

    return 0;
}
