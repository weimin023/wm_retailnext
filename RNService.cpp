#include "RNService.h"


json HierarchyService::processRequest(const std::string& request) {
    try {
        json req_json = json::parse(request);
        if (req_json.contains("add_node")) return addNode(req_json["add_node"]);
        if (req_json.contains("delete_node")) return deleteNode(req_json["delete_node"]);
        if (req_json.contains("move_node")) return moveNode(req_json["move_node"]);
        if (req_json.contains("query")) return query(req_json["query"]);
    } catch (const json::exception&) {
        return {{"ok", false}};
    }
    return {{"ok", false}};
}

json HierarchyService::addNode(const json& data) {
    // Invalid Node
    if (!data.contains("id") || !data.contains("name")) return {{"ok", false}};

    std::string id = data["id"];
    std::string name = data["name"];
    std::string parent_id = data.value("parent_id", "");

    // Invalid Node
    if (id.empty() || name.empty() || nodes.count(id)) return {{"ok", false}};
    
    if (parent_id.empty()) {
        if (!root_id.empty()) return {{"ok", false}};
        root_id = id;
    } else {
        if (!nodes.count(parent_id) || hasNameConflict(parent_id, name)) return {{"ok", false}};
        nodes[parent_id].children.insert(id);
    }

    nodes[id] = {id, name, parent_id, {}};
    return {{"ok", true}};
}

json HierarchyService::deleteNode(const json& data) {
    if (!data.contains("id")) return {{"ok", false}};
    
    std::string id = data["id"];
    if (!nodes.count(id) || !nodes[id].children.empty()) return {{"ok", false}};
    
    std::string parent_id = nodes[id].parent_id;
    if (!parent_id.empty()) nodes[parent_id].children.erase(id);
    if (id == root_id) root_id = "";

    nodes.erase(id);
    return {{"ok", true}};
}

json HierarchyService::moveNode(const json& data) {
    if (!data.contains("id") || !data.contains("new_parent_id")) return {{"ok", false}};

    std::string id = data["id"];
    std::string new_parent_id = data["new_parent_id"];

    if (id.empty() || new_parent_id.empty() || !nodes.count(id) || !nodes.count(new_parent_id)) return {{"ok", false}};
    if (hasNameConflict(new_parent_id, nodes[id].name)) return {{"ok", false}};
    if (createsCycle(id, new_parent_id)) return {{"ok", false}};

    std::string old_parent_id = nodes[id].parent_id;
    if (!old_parent_id.empty()) nodes[old_parent_id].children.erase(id);

    nodes[new_parent_id].children.insert(id);
    nodes[id].parent_id = new_parent_id;

    return {{"ok", true}};
}

json HierarchyService::query(const json& data) {
    if (!data.contains("id")) return {{"ok", false}};
    
    std::string id = data["id"];
    if (!nodes.count(id)) return {{"ok", false}};
    
    Node& node = nodes[id];
    json result = {{"id", node.id}, {"name", node.name}, {"children", json::array()}};
    for (const auto& child_id : node.children) {
        result["children"].push_back({{"id", child_id}, {"name", nodes[child_id].name}});
    }
    return result;
}

bool HierarchyService::hasNameConflict(const std::string& parent_id, const std::string& name) {
    for (const auto& child_id : nodes[parent_id].children) {
        if (nodes[child_id].name == name) return true;
    }
    return false;
}

bool HierarchyService::createsCycle(const std::string& node_id, const std::string& new_parent_id) {
    std::string current = new_parent_id;
    while (!current.empty()) {
        if (current == node_id) return true;
        current = nodes[current].parent_id;
    }
    return false;
}




