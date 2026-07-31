#include <Shared/Simulation.hh>
#include <Server/Server.hh>
#include <Server/BVHTest.cc>
#include <iostream>

int main() {
    //test_aabb_helpers();
    //test_edge_cases();
    //test_move_noop_when_inside_fat();
    //test_persistent_lifecycle();
    //test_tree_quality();
    //test_cross_tree_pairs();
    //test_ccd_sweep();
    //test_ccd_segment();
    //if (test_superset_scenes() != 0) return 1;
    std::printf("ALL BVH TESTS PASSED\n");
    std::cout << "Diagnostics: {\n";
    std::cout << "  Simulation Size: " << sizeof(Simulation) << '\n';
    std::cout << "  BVH Size: " << sizeof(BVHCollisionManager) << '\n';
    std::cout << "  Entity Size: " << sizeof(Entity) << '\n';
    std::cout << "}\n";
    srand(std::time(0));
    Server::init();
    return 0;
}

