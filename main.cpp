#include "RNService.h"

int main() {
    HierarchyService service;
    std::string line;
    while (getline(std::cin, line)) {
        std::cout << service.processRequest(line) << std::endl;
    }
    return 0;
}
