# cpp-db-excercise

## Portability

The Visual Studio project files have been removed and a CMakeLists.txt definition has been added to make the code build system / platform agnostic.

Some windows specific headers are removed since they were not used. 

A `demangle.h` header contains utilities for demangling C++ symbol names. Currently only GCC implementation exists, but a MSVC can be added easily.


## Refactoring of existing code

The initial implementation is extracted in a separate class/file - `QBRecordCollection`/`qbrecord.h`, so there can be a base for comparison with different implementations.

In it the filtering function is as implemented a class method and a naive delete by id functionality is added (just for comparison).


## Unit testing

Binary: `unit_test`

A simple template for running a test scenario - add, filter and delete records from a database are tested.


## Benchmarking

Binary: `benchmark`

The benchmarking was made more generic and expanded as a scenario.

It measures insertion, filtering and deletion separately

 - `numRecords` number of records are inserted into the collection following the data patterns from the initial code: `{id, prefix+id, id % 1000, id+prefix}`
 - a query for `"testdata" + numRecords/20` on `column1` is executed, expecting to return 11 results;
 - a query for `"500"` is executed on `column2`, and `numRecords/1000` records are expected
 - a query for `"66testdata"` is executed on `column3` and `numRecords/100` records are expected
 - deletes for 5% of the record ids are executed

## Proposed implementation

`IndexDb` is a header only, templated class that uses a `std::unordered_map` as a base storage container.

In addition a one-to-many index is created on the `column2` field.

Optionally any partial key for data in `column1` and `column3` above a set `minIndexSize` template parameter can be indexed as well at the cost of insertion time, memory usage and deletion time.

Note: In the original example, the `QBFindMatchingRecords()` method returns the same type as the initial collection, but my assumption in attempts to optimize it, was that the intention for the filter function is to return a container of records, not necessarily a filterable collection of its own, so a `std::vector<Record>` is expected everywhere.

### Advantages
 - very fast query times (50x times the original)
 - configurable, so the partial string indexing can be limited
 - still faster if no partial string indexing is used (7x faster)

### Disadvantages
 - slower insertion, especially when partial index is used
 - high memory usage
 
Using the minIndexSize a balance between read/write/delete performance can be achieved.
 

## Performance
The benchmarking is done on three implementation:

 - `QBRecordCollection` - the original
 - `IndexDb<100>` - without partial string key indexing
 - `IndexDb<10>` with `minIndexSize` just covering our use case

Results follow for i7-5820K@4.00GHz, times are in seconds.

### Insertion

Note: the original implementation does not guarantee a uniqueness of `column0` - providing such guarantee will affect insertion times negatively

Number of records | 10000 | 20000 | 30000 | 40000 | 50000
------------------|-------|-------|-------|-------|------
`QBRecordCollection` | 0.000901 | 0.001262 | 0.002812 | 0.003215 | 0.0041
`IndexDb<100>` | 0.001983 | 0.003681 | 0.005961 | 0.007828 | 0.009604
`IndexDb<10>` | 0.027749 | 0.100623 | 0.16485 | 0.235566 | 0.29328


### Filtering 3 times

Number of records | 10000 | 20000 | 30000 | 40000 | 50000
------------------|-------|-------|-------|-------|------
`QBRecordCollection` | 0.001809 | 0.002332 | 0.005187 | 0.006819 | 0.008449
`IndexDb<100>` | 0.000242 | 0.000404 | 0.000621 | 0.000842 | 0.001035
`IndexDb<10>` | 0.000027 | 0.000065 | 0.000082 | 0.00014 | 0.000156


### Deletion of 5% of recrods

Number of records | 10000 | 20000 | 30000 | 40000 | 50000
------------------|-------|-------|-------|-------|------
`QBRecordCollection` | 0.026053 | 0.084524 | 0.181818 | 0.372403 | 0.505524
`IndexDb<100>` | 0.000054 | 0.000108 | 0.000175 | 0.000238 | 0.000296
`IndexDb<10>` | 0.00134 | 0.005105 | 0.007851 | 0.011826 | 0.015339
