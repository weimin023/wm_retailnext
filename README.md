## Introduction
The **Hierarchy Service** contains three `classes`:
1. `Node`: Define the structure of a basic node.
2. `TreeHierarchy`: Define the structure of a tree.
3. `HierarchyService`: The hierarchy service.

The only third-party library used in the task is `nlohmann::json` for JSON parsing and serialization.
## Build
```
mkdir build
cd build
cmake ..
make -j12
```

## Run the service
```
./retailnext
```

## Run with testcases provided
```
(in the directory of CMakeLists.txt)
./test_client_linux build/retailnext
```
