#include <Shared/Simulation.hh>
#include <Server/Server.hh>

#include <iostream>

int main() {
    std::cout << "Diagnostics: {\n";
    std::cout << "  Simulation Size: " << sizeof(Simulation) << '\n';
    std::cout << "  BVH Size: " << sizeof(SpatialHash) << '\n';
    std::cout << "  Entity Size: " << sizeof(Entity) << '\n';
    std::cout << "}\n";
    srand(std::time(0));
    Server::init();
    return 0;
}

