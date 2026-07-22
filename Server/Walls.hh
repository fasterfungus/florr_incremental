#pragma once

class Simulation;

// Register all static wall obstacles into sim's BVHCollisionManager.
// Call once after Simulation is constructed, before the first tick.
void init_walls(Simulation *sim);
