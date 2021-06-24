#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <unordered_set>
#include <algorithm>


// The IndexDb class uses id-to-Record map as a base storage
// and indexes for the other columns - key-to-id
// with minIndexSize template parameter specifying the minimum size
// for which a partial key index is done
template<size_t minIndexSize = 5>
class IndexDb
{
public:
    struct Record
    {
        unsigned int column0;
        std::string column1;
        long column2;
        std::string column3;
    };

    using StorageMap = std::unordered_map<unsigned int, Record>; // base id->Record storage
    using RecordSet = std::unordered_set<unsigned int>; // a set of record Ids
    using NumIndex = std::unordered_map<long, RecordSet>; // long->id set
    using StringIndex = std::unordered_map<std::string, RecordSet>; // Partial key->id set index
    using QueryResult = std::vector<Record>; // What we return on a query

    // mirrors the std::vector naming
    void emplace_back(const Record& rec)
    {
        auto result = _storage.try_emplace(rec.column0, rec);
        if(result.second)
        {
            insertOrRemoveAllPartialStringKeys(_col1_idx, rec.column1, rec.column0, false);
            _col2_idx[rec.column2].emplace(rec.column0);
            insertOrRemoveAllPartialStringKeys(_col3_idx, rec.column3, rec.column0, false);
        }
    }

    // Reserves storage for the buckets in the unordered_maps
    void reserve(size_t elements)
    {
        _storage.reserve(elements);
        _col1_idx.reserve(elements);
        _col2_idx.reserve(elements);
        _col3_idx.reserve(elements);
    }

    // Delete a record by id and all indexes pointing to it
    void deleteById(int id)
    {
        auto it = _storage.find(id);
        if(it != _storage.end())
        {
            Record& rec = (*it).second;
            insertOrRemoveAllPartialStringKeys(_col1_idx,
                                               rec.column1,
                                               id,
                                               true);
            RecordSet& recSet = _col2_idx[rec.column2];
            recSet.erase(id);
            insertOrRemoveAllPartialStringKeys(_col3_idx,
                                               rec.column3,
                                               id,
                                               true);
            _storage.erase(it);
        }
    }

    // return number of records
    size_t size() const
    {
        return _storage.size();
    }

    // queries the records on different columns, supporting partial key search on string types
    QueryResult filter(const std::string& colName, const std::string& value) const
    {
        QueryResult result;
        if(colName == "column0") // column0 is returned directly from the base map
        {
            int id = std::stoul(value);
            auto it = _storage.find(id);
            if(it != _storage.end())
            {
                result.emplace_back(it->second);
            }
        }
        else if(colName == "column2") // column2 is always returned from index
        {
            getResultsForQuery(_col2_idx, std::stol(value), result);
        }
        else if(value.size() < minIndexSize)
        {
            // for text columns if the key is less than the minIndexSize
            // a normal search is done
            getResultsForQueryNoIndex(colName, value, result);
        }
        else if(colName == "column1") // otherwise the column partial index maps are used
        {
            getResultsForQuery(_col1_idx, value, result);
        }
        else if(colName == "column3")
        {
            getResultsForQuery(_col3_idx, value, result);
        }

        return result;
    }

protected:


    // Iterates trough all records returning only matching records
    void getResultsForQueryNoIndex(const std::string& column,
                                   const std::string& value,
                                   QueryResult& result) const
    {
        if(column == "column1")
        {
            std::for_each(_storage.begin(),
                          _storage.end(),
                          [&value, &result](auto& recPair)
            {
                const Record& rec = recPair.second;
                if (rec.column1.find(value) != std::string::npos)
                {
                    result.emplace_back(rec);
                }
            });
        }
        else if(column == "column2")
        {
            long longValue = std::stol(value);
            std::for_each(_storage.begin(),
                          _storage.end(),
                          [&longValue, &result](auto& recPair)
            {
                const Record& rec = recPair.second;
                if (rec.column2 == longValue)
                {
                    result.emplace_back(rec);
                }
            });

        }
        else if(column == "column3")
        {
            std::for_each(_storage.begin(),
                          _storage.end(),
                          [&value, &result](auto& recPair)
            {
                const Record& rec = recPair.second;
                if (rec.column3.find(value) != std::string::npos)
                {
                    result.emplace_back(rec);
                }
            });
        }
    }

    // Returns matching records directly from the indexed maps
    template<class Index, class Value>
    void getResultsForQuery(const Index& idx,
                            const Value& value,
                            QueryResult& result) const
    {
        auto it = idx.find(value);
        if(it != idx.end())
        {
            const RecordSet& recSet = it->second;
            result.reserve(recSet.size());
            for(unsigned int id : recSet)
            {
                auto itStorage = _storage.find(id);
                if(itStorage != _storage.end())
                {
                    const Record& rec = itStorage->second;
                    result.emplace_back(rec);
                }
            }
        }
    }

    // Used to generate all combination of partial keys and either insert an id or remove it
    void insertOrRemoveAllPartialStringKeys(StringIndex& stringIdx,
                                            const std::string& fullKey,
                                            unsigned int id,
                                            bool remove)
    {
        if(minIndexSize <= fullKey.size())
        {
            for(size_t start = 0; start < fullKey.size(); ++start)
            {
                for(size_t len = minIndexSize; len <= (fullKey.size() - start); ++len)
                {
                    std::string partialKey = fullKey.substr(start, len);
                    if(remove)
                    {
                        auto it = stringIdx.find(partialKey);
                        if(it != stringIdx.end())
                        {
                            RecordSet& recSet = (*it).second;
                            recSet.erase(id);
                        }
                    }
                    else
                    {
                        stringIdx[partialKey].emplace(id);
                    }
                }
            }
        }
    }

    StorageMap _storage;
    StringIndex _col1_idx;
    NumIndex _col2_idx;
    StringIndex _col3_idx;

};
