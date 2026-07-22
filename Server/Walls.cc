// Static wall layout for the map. Call init_walls() once after Simulation
// is constructed (from GameInstance::init) to register all walls into the
// BVHCollisionManager's stationary tree.

#include <Server/Walls.hh>
#include <Shared/Simulation.hh>
#include <Server/Wall.hh>
#include <Helpers/Math.hh>
#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream>
#include <iostream>
#include <string>
// Convenience: add a wall by center, length, angle-in-degrees.
static void add_from_length(Simulation *sim, float x, float y, float length, float angle) {
    sim->bvh_collision_manager.add_wall(Wall(x, y, length, angle * (float)M_PI / 180.0f));
}
static void add(Simulation *sim, float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float length = sqrtf(dx * dx + dy * dy);
    if (length < EPSILON) return;
    float rotation = atan2f(dy, dx);
    sim->bvh_collision_manager.add_wall(Wall((x1+x2)/2, (y1+y2)/2, length, rotation));
}
void init_walls(Simulation *sim) {
    std::ifstream file("garden.txt");
    if (!file.is_open()) {
        std::cerr << "错误：无法打开 garden.txt" << std::endl;
        return;
    }
    std::string line;
    int count = 0;
    while (std::getline(file, line)) {
        // 跳过空行和注释行
        if (line.empty() || line[0] == '#') continue;

        // 解析 (x1,y1,x2,y2) 格式
        float x1, y1, x2, y2;
        char c; // 用于吃掉括号和逗号
        std::stringstream ss(line);
        ss >> c >> x1 >> c >> y1 >> c >> x2 >> c >> y2 >> c;  // ( float , float , float , float )

        add(sim, x1, y1, x2, y2);
        count++;
    }

    file.close();
    std::cout << "共执行了 " << count << " 次 add 调用" << std::endl;
}

