#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include "json.hpp"

using json = nlohmann::json;

struct Node {
    std::string id;
    std::string name;
    std::string parent_id;
    std::unordered_set<std::string> children;
};

class HierarchyService {
public:
    json processRequest(const std::string& request);
private:
    json addNode(const json& data);
    json deleteNode(const json& data);
    json moveNode(const json& data);
    json query(const json& data);

    bool hasNameConflict(const std::string& parent_id, const std::string& name);
    bool createsCycle(const std::string& node_id, const std::string& new_parent_id);

    std::unordered_map<std::string, Node> nodes;
    std::string root_id;
};