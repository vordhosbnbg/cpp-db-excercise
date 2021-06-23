#pragma once
#include <string>
#include <vector>
#include <algorithm>
/**
    Represents a Record Object
*/
struct QBRecord
{
    unsigned int column0; // unique id column
    std::string column1;
    long column2;
    std::string column3;
};


/**
Represents a Record Collections
*/
using QBRecordCollection = std::vector<QBRecord>;


/**
    Return records that contains a string in the StringValue field
    records - the initial set of records to filter
    matchString - the string to search for
*/

QBRecordCollection QBFindMatchingRecords(const QBRecordCollection& records,
                                         const std::string& columnName,
                                         const std::string& matchString)
{
    QBRecordCollection result;
    std::copy_if(records.begin(),
                 records.end(),
                 std::back_inserter(result),
                 [&](QBRecord rec)
    {
        if (columnName == "column0")
        {
            uint matchValue = std::stoul(matchString);
            return matchValue == rec.column0;
        }
        else if (columnName == "column1")
        {
            return rec.column1.find(matchString) != std::string::npos;
        }
        else if (columnName == "column2")
        {
            long matchValue = std::stol(matchString);
            return matchValue == rec.column2;
        }
        else if (columnName == "column3")
        {
            return rec.column3.find(matchString) != std::string::npos;
        }
        else
        {
            return false;
        }
    });
    return result;
}
