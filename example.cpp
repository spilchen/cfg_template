#include <iostream>
#include <map>
#include "cfg_template.hpp"


enum class DatabaseConfigParm : int8_t;
enum class ClusterConfigParm : int8_t;

/**
   List of database config parameters.

   Each parameter needs to be added to the ConfigTemplate for this enum in
   order to provide a default value and help text.
*/
enum class DatabaseConfigParm : int8_t
{
    MAX_ROWS_PER_ROWGROUP,
    STRIDESIZE,
    SHARED_FS_TYPE,
    CACHE_MEM_SZ,
};

template <>
ConfigTemplate<DatabaseConfigParm>::ConfigTemplate(std::map<std::string, std::string> overrides)
    : mFactory(overrides)
    , mParms{ { DatabaseConfigParm::MAX_ROWS_PER_ROWGROUP, mFactory.Make_IntReadOnlyCV<int>("MAX_ROWS_PER_ROWGROUP", 10000, "Maximum number of rows per row group.") },
        { DatabaseConfigParm::STRIDESIZE, mFactory.Make_IntReadOnlyCV<int16_t>("STRIDE_SIZE", 512, "Maximum stride size of a table") },
        { DatabaseConfigParm::SHARED_FS_TYPE, mFactory.Make_StrReadOnlyCV("SHARED_FS", "alluxio", "The file system type") },
        { DatabaseConfigParm::CACHE_MEM_SZ, mFactory.Make_IntUpdatableCV<int64_t>("CACHE_MEM_SZ", 0, "Memory size of cache") } }
{
}

using DatabaseConfig = ConfigTemplate<DatabaseConfigParm>;

/**
   List of cluster config parameters.

   Each parameter needs to be added to the ConfigTemplate for this enum in
   order to provide a default value and help text.
*/
enum class ClusterConfigParm : int8_t
{
    NUM_NODES,
    ZK_TIMEOUT,
    QUORUM_WRITE,
    INSERT_FLUSH,
};

template <>
ConfigTemplate<ClusterConfigParm>::ConfigTemplate(std::map<std::string, std::string> overrides)
    : mFactory(overrides)
    , mParms{ { ClusterConfigParm::NUM_NODES, mFactory.Make_IntReadOnlyCV<int8_t>("NUM_NODES", 3, "Number of nodes in the cluster.") },
        { ClusterConfigParm::ZK_TIMEOUT, mFactory.Make_IntReadOnlyCV<int64_t>("ZK_TIMEOUT", 10000, "Zookeeper timeout in milliseconds") },
        { ClusterConfigParm::QUORUM_WRITE, mFactory.Make_StrReadOnlyCV("QUORUM_WRITE", "true", "Is quorum write set") },
        { ClusterConfigParm::INSERT_FLUSH, mFactory.Make_BoolReadOnlyCV("INSERT_FLUSH", true, "Does each insert flush?") } }
{
}

using ClusterConfig = ConfigTemplate<ClusterConfigParm>;

int main()
{
    std::map<std::string, std::string> dbOverrides = { {"MAX_ROWS_PER_ROWGROUP", "512"} };
    DatabaseConfig dbcfg(dbOverrides);
    auto v = dbcfg.as_<std::string>(DatabaseConfigParm::MAX_ROWS_PER_ROWGROUP);
    std::cout << "Max Rows Per Row Group = " << v << "\n";
    auto v2 = dbcfg.as_<std::string>(DatabaseConfigParm::STRIDESIZE);
    std::cout << "Stridesize = " << v2 << "\n";

    std::map<std::string, std::string> clusterOverrides= { {"INSERT_FLUSH", "false"} };
    ClusterConfig clcfg(clusterOverrides);
    auto v3 = clcfg.as_<std::string>(ClusterConfigParm::NUM_NODES);
    std::cout << "Num nodes = " << v3 << "\n";
    auto v4 = clcfg.as_<std::string>(ClusterConfigParm::ZK_TIMEOUT);
    std::cout << "ZK Timeout = " << v4 << "\n";

    auto v5 = dbcfg.as_<std::string>(DatabaseConfigParm::SHARED_FS_TYPE);
    std::cout << "Shared FS Type = " << v5 << "\n";

    auto v6 = dbcfg.as_<int>(DatabaseConfigParm::STRIDESIZE);
    std::cout << "Stridesize = " << v6 << " (" << sizeof(v6) << ")\n";

    auto v7 = dbcfg.as_<int64_t>(DatabaseConfigParm::STRIDESIZE);
    std::cout << "Stridesize = " << v7 << " (" << sizeof(v7) << ")\n";

    auto v8 = clcfg.as_<uint8_t>(ClusterConfigParm::NUM_NODES);
    std::cout << "Num nodes = " << std::to_string(v8) << " (" << sizeof(v8) << ")\n";

    auto v9 = clcfg.as_<bool>(ClusterConfigParm::QUORUM_WRITE);
    std::cout << "Quorum Write = " << std::boolalpha << v9 << " (" << sizeof(v9) << ")\n";

    dbcfg.set(DatabaseConfigParm::CACHE_MEM_SZ, std::to_string(4096 * 1000));
    auto v10 = dbcfg.as_<uint64_t>(DatabaseConfigParm::CACHE_MEM_SZ);
    std::cout << "Cache mem size = " << v10 << "\n";

    auto v11 = clcfg.as_<bool>(ClusterConfigParm::INSERT_FLUSH);
    std::cout << "Insert flush = " << std::boolalpha << v11 << " (" << sizeof(v11) << ")\n";

    auto v12 = clcfg.as_<uint64_t>(ClusterConfigParm::NUM_NODES);
    std::cout << "Num nodes = " << std::to_string(v12) << " (" << sizeof(v12) << ")\n";

    auto v13 = dbcfg.as_<uint8_t>(DatabaseConfigParm::CACHE_MEM_SZ);
    std::cout << "Cache mem size = " << std::to_string(v13) << " (" << sizeof(v13) << ") \n";
}
