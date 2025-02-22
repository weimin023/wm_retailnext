#include "RNService.h"

bool TreeHierarchy::addNode(const std::string& id, const std::string& name, std::string parentId) {
    // Invalid Node
    if (id.empty() || name.empty()) return false;
    if (id_map.find(id) != id_map.end()) return false;

    auto newNode = std::make_shared<Node>(id, name);

    if (parentId.empty()) {          // This newNode is a root
        if (root) return false;
        root = newNode;
    } else {
        auto parentIt = id_map.find(parentId);
        if (parentIt == id_map.end()) return false;

        auto parent = parentIt->second;
        if (hasNameConflict(parent->id, name)) return false;

        newNode->parent = parent;
        parent->children.emplace_back(newNode);
    }

    id_map[id] = newNode;
    return true;
}

bool TreeHierarchy::deleteNode(const std::string& id) {
    // ID must be specified and not an empty string.
    if (id.empty()) return false;

    auto it = id_map.find(id);

    // Node must exist
    if (it == id_map.end()) return false;

    auto node = it->second;

    // Node must not have children.
    if (!node->children.empty()) return false;

    // Remove from parent's children list
    auto parentPtr = node->parent.lock();
    if (parentPtr) {
        auto &siblings = parentPtr->children;
        siblings.erase(
            std::remove_if(siblings.begin(), siblings.end(), [&](const auto &child){ return child->id == id; }),
            siblings.end()
        );
    } else {
        // No parent, this is root
        root.reset();
    }

    id_map.erase(id);
    return true;

}

bool TreeHierarchy::moveNode(const std::string& id, const std::string& newParentId) {
    // ID and new parent ID must be specified and not empty strings
    if (id.empty() || newParentId.empty()) return false;

    // Both nodes must exist
    auto nodeIt = id_map.find(id);
    auto newParentIt = id_map.find(newParentId);
    if (nodeIt == id_map.end() || newParentIt == id_map.end()) return false;

    auto node = nodeIt->second;
    auto newParent = newParentIt->second;

    // The name of the node to be moved must not be the same as those of any of the new parent's other children.
    if (hasNameConflict(newParent->id, node->name)) return false;

    // Move must not create a cycle in the tree
    if (createsCycle(node->id, newParent->id)) return false;

    // remove from old parent
    if (auto oldParentPtr = node->parent.lock()) {
        auto &oldSiblings = oldParentPtr->children;
        oldSiblings.erase(
            std::remove_if(oldSiblings.begin(), oldSiblings.end(), [&](const auto &child){ return child->id == id; }),
            oldSiblings.end()
        );
    } else { // this is root
        root.reset();
    }

    node->parent = newParent;
    newParent->children.emplace_back(node);

    return true;
}

bool TreeHierarchy::hasNameConflict(const std::string& parent_id, const std::string& name) {
    // no parent_id provided, check if root exists
    if (parent_id.empty()) return root != nullptr;

    // find parent
    auto parentIt = id_map.find(parent_id);
    if (parentIt == id_map.end()) return false;

    // check if any siblings have the same name
    auto parent = parentIt->second;
    for (const auto &child:parent->children) {
        if (child->name == name) return true;
    }

    return false;
}

bool TreeHierarchy::createsCycle(const std::string& node_id, const std::string& new_parent_id) {
    // If one of them don't exist
    if (id_map.find(node_id) == id_map.end() || id_map.find(new_parent_id) == id_map.end()) return false;

    if (node_id == new_parent_id) return true;

    std::string curr_id = new_parent_id;
    std::unordered_set<std::string> visited;

    while (!curr_id.empty()) {

        // infinite loop
        if (visited.find(curr_id) != visited.end()) {
            return true;
        }
        visited.insert(curr_id);

        if (curr_id == node_id) return true;

        // move up to parent
        auto curr_node = id_map[curr_id];
        auto parent = curr_node->parent.lock();
        if (!parent) break; // root

        curr_id = parent->id;
    }

    return false;
}

json TreeHierarchy::query(const json& data) {

    std::vector<std::shared_ptr<Node>> results;

    std::vector<std::shared_ptr<Node>> root_nodes;
    std::unordered_set<std::string> name_filter, id_filter;
    int min_depth = -1, max_depth = -1;
    
    if (data.contains("min_depth")) min_depth = data["min_depth"].get<int>();
    if (data.contains("max_depth")) max_depth = data["max_depth"].get<int>();
    if (data.contains("names")) {
        for (const auto &name:data["names"]) {
            name_filter.insert(name.get<std::string>());
        }
    }
    if (data.contains("ids")) {
        for (const auto &id:data["ids"]) {
            id_filter.insert(id.get<std::string>());
        }
    }
    if (data.contains("root_ids")) {
        for (const auto& root_id : data["root_ids"]) {
            std::string id = root_id.get<std::string>();
            if (id_map.count(id)) root_nodes.emplace_back(id_map[id]);
        }
    } else if (root) {
        root_nodes.push_back(root);
    }

    // Start DFS for each node in the list
    for (const auto& root_node : root_nodes) {
        queryDFS(root_node, 0, min_depth, max_depth, name_filter, id_filter, results);
    }
    
    // Prepare JSON response
    json response;
    response["nodes"] = json::array();

    for (const auto &node:results) {
        auto parent = node->parent.lock();
        std::string parent_id = (parent)? parent->id: "";
        response["nodes"].push_back({{"name", node->name}, {"id", node->id}, {"parent_id", parent_id}});
    }

    return response;
}

void TreeHierarchy::queryDFS(const std::shared_ptr<Node>& node, int depth, 
                             int min_depth, int max_depth, 
                             const std::unordered_set<std::string>& name_filter, 
                             const std::unordered_set<std::string>& id_filter,
                             std::vector<std::shared_ptr<Node>>& results) {

    if (!node) return;

    if ((min_depth == -1 || depth >= min_depth) &&
        (max_depth == -1 || depth <= max_depth) &&
        (name_filter.empty() || name_filter.count(node->name)) &&
        (id_filter.empty() || id_filter.count(node->id))) {
        results.emplace_back(node);
    }

    // Sort children by name before traversal
    std::vector<std::shared_ptr<Node>> sorted_children = node->children;
    std::sort(sorted_children.begin(), sorted_children.end(), [](const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) { return a->name < b->name; });
    for (const auto &child:sorted_children) {
        queryDFS(child, depth+1, min_depth, max_depth, name_filter, id_filter, results);
    }
}

void HierarchyService::run() {
    std::string line;
    while (std::getline(std::cin, line)) {
        json response = processRequest(line);
        std::cout << response.dump() << std::endl;
    }
}

json HierarchyService::processRequest(const std::string& request) {
    
    json req_json;
    bool success = false;
    
    try {
        req_json = json::parse(request);

        if (req_json.contains("add_node")) {
            const auto &params = req_json["add_node"];

            std::string parentId = params.contains("parent_id") ? params["parent_id"].get<std::string>() : "";
            std::string id = params["id"].get<std::string>();
            std::string name = params["name"].get<std::string>();
            success = m_tree.addNode(id, name, parentId);
        }
        else if (req_json.contains("delete_node")) {
            const auto &params = req_json["delete_node"];

            std::string id = params["id"].get<std::string>();
            success = m_tree.deleteNode(id);
        }
        else if (req_json.contains("move_node")) {
            const auto& params = req_json["move_node"];

            std::string id = params["id"].get<std::string>();
            std::string new_parent_id = params["new_parent_id"].get<std::string>();
            success = m_tree.moveNode(id, new_parent_id);
        }
        else if (req_json.contains("query")) {
            const auto &params = req_json["query"];

            return m_tree.query(params);
        }
    } catch (const json::exception&) {
        success = false;
    }
    
    return {{"ok", success}};
}




