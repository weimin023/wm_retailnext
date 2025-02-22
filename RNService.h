#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <memory>
#include "json.hpp"

using json = nlohmann::json;

struct Node {
    Node(const std::string &id, const std::string &name):id(id), name(name) {}
    std::string id;
    std::string name;
    std::weak_ptr<Node> parent;
    std::vector<std::shared_ptr<Node>> children;
};

class TreeHierarchy {
public:
    bool addNode(const std::string& id, const std::string& name, std::string parentId = "");
    bool deleteNode(const std::string& id);
    bool moveNode(const std::string& id, const std::string& newParentId);

    json query(const json& data);

private:
    bool hasNameConflict(const std::string& parent_id, const std::string& name);
    bool createsCycle(const std::string& node_id, const std::string& new_parent_id);
    
    void queryDFS(const std::shared_ptr<Node>& node, int depth, 
                    int min_depth, int max_depth, 
                    const std::unordered_set<std::string>& name_filter, 
                    const std::unordered_set<std::string>& id_filter,
                    std::vector<std::shared_ptr<Node>>& results);

    std::unordered_map<std::string, std::shared_ptr<Node>> id_map;
    std::shared_ptr<Node> root;
};

class HierarchyService {
public:
    void run();
private:
    json processRequest(const std::string& request);

    TreeHierarchy m_tree;
};