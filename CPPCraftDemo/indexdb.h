#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <unordered_set>
#include <algorithm>

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

    using StorageMap = std::unordered_map<unsigned int, Record>;
    using RecordSet = std::unordered_set<unsigned int>;
    using NumIndex = std::unordered_map<long, RecordSet>;
    using StringIndex = std::unordered_map<std::string, RecordSet>;
    using QueryResult = std::vector<Record>;

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

    void reserve(size_t elements)
    {
        _storage.reserve(elements);
        _col1_idx.reserve(elements);
        _col2_idx.reserve(elements);
        _col3_idx.reserve(elements);
    }

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

    size_t size() const
    {
        return _storage.size();
    }

    QueryResult filter(const std::string& colName, const std::string& value) const
    {
        QueryResult result;
        if(colName == "column0")
        {
            int id = std::stoul(value);
            auto it = _storage.find(id);
            if(it != _storage.end())
            {
                result.emplace_back(it->second);
            }
        }
        else if(colName == "column2")
        {
            getResultsForQuery(_col2_idx, std::stol(value), result);
        }
        else if(value.size() < minIndexSize)
        {
            getResultsForQueryNoIndex(colName, value, result);
        }
        else if(colName == "column1")
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
