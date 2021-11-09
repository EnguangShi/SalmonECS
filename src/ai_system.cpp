// internal
#include "ai_system.hpp"
#include "world_init.hpp"
#include "iostream"
#include <unistd.h>

const float fleeingSpeed = 100.f;
bool freeze = false;
uint updateFrequency = 100;
bool upwards = false;
bool downwards = false;
const float turtleSpeed = 50.f;

void AISystem::step(float elapsed_ms)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE FISH AI HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// You will likely want to write new functions and need to create
	// new data structures to implement a more sophisticated Fish AI.
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	auto& motion_registry = registry.motions;
	if (!freeze) {
		downwards = false;
		upwards = false;
	}
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		Motion& motion = motion_registry.components[i];  // position...
		Entity entity = motion_registry.entities[i];  // salmon, fish, turtle
		float step_seconds = 1.0f * (elapsed_ms / 1000.f);
		vec2 salmonPos;
		vec2 salmonVel;

		// A2 CREATIVE PART
		if (advancedMode) {
			if (registry.hardShells.has(entity)) {
				if (motion.counter == updateFrequency) {
					motion.counter = 0;
				}
				if (reset) {
					motion.counter = 0;
					reset = false;
				}
				if (motion.counter == 0){
					float turtleDirectionX = motion.position.x - salmonPos.x;
					float turtleDirectionY = motion.position.y - salmonPos.y;
					float angle = atan2(turtleDirectionX, turtleDirectionY);
					motion.velocity.x = -turtleSpeed*sin(angle);
					motion.velocity.y = -turtleSpeed*cos(angle);
				}
				motion.counter += 1;
			}
		}

		// get salmon's position and velocity
		if (registry.players.has(entity)) {
			salmonPos = motion.position;
			salmonVel = motion.velocity;
		}

		// fish
		if (registry.softShells.has(entity)) {
			if (motion.counter == updateFrequency) {
				motion.counter = 0;
			}
			if (reset) {
					motion.counter = 0;
					reset = false;
				}
			// if the fish gets close to the salmon
			if (motion.position.x <= salmonPos.x+AVOIDING_DISTANCE && motion.position.x >= salmonPos.x-AVOIDING_DISTANCE
				&& motion.position.y <= salmonPos.y+AVOIDING_DISTANCE && motion.position.y >= salmonPos.y-AVOIDING_DISTANCE){
					motion.closeToSalmon = true;
				// if gets close from the right
				if (motion.position.x >= salmonPos.x 
					&& motion.position.y <= salmonPos.y+AVOIDING_DISTANCE-5 && motion.position.y >= salmonPos.y-AVOIDING_DISTANCE+5 ) {
					// set the fish position to stay an avoiding distance to salmon
					motion.position.x = salmonPos.x + AVOIDING_DISTANCE;
					// fish goes back if salmon's approaching from the left
					if (motion.closeToSalmon && salmonVel.x > 0) {
						motion.velocity.x = salmonVel.x;
					}
					// fish flees to the bottom
					if (motion.position.y >= salmonPos.y) {
						if (motion.counter == 0) {
							if (debugging.in_debug_mode) {
								freeze = true;
								downwards = true;
							}	
							motion.velocity.y = fleeingSpeed;
						}
					// fish flees to the top
					} else {  
						if (motion.counter == 0) {
							if (debugging.in_debug_mode) {
								freeze = true;
								upwards = true;
							}	
							motion.velocity.y = -fleeingSpeed;
						}
					}
				}
				// if gets close from the left
				if (motion.position.x <= salmonPos.x 
					&& motion.position.y <= salmonPos.y+AVOIDING_DISTANCE-5 && motion.position.y >= salmonPos.y-AVOIDING_DISTANCE+5 ) {
					// set the fish position to stay an avoiding distance to salmon
					motion.position.x = salmonPos.x - AVOIDING_DISTANCE;
					// fish goes forward if salmon's approaching from the right
					if (motion.closeToSalmon && salmonVel.x < 0) {
						motion.velocity.x = salmonVel.x;
					}
				}
				// if gets close from the bottom
				if (motion.position.y >= salmonPos.y 
					&& motion.position.x <= salmonPos.x+AVOIDING_DISTANCE-5 && motion.position.x >= salmonPos.x-AVOIDING_DISTANCE+5) {
					// set the fish position to stay an avoiding distance to salmon
					motion.position.y = salmonPos.y + AVOIDING_DISTANCE;
					// fish goes down if salmon's approaching from the top
					if (motion.closeToSalmon && salmonVel.y > 0) {
						if (motion.counter == 0) {
							if (debugging.in_debug_mode) {
								freeze = true;
								downwards = true;
							}	
							motion.velocity.y = salmonVel.y;
						}	
					}
				}
				// if gets close from the top
				if (motion.position.y <= salmonPos.y 
					&& motion.position.x <= salmonPos.x+AVOIDING_DISTANCE-5 && motion.position.x >= salmonPos.x-AVOIDING_DISTANCE+5) {
					// set the fish position to stay an avoiding distance to salmon
					motion.position.y = salmonPos.y - AVOIDING_DISTANCE;
					// fish goes up if salmon's approaching from the bottom
					if (motion.closeToSalmon && salmonVel.y < 0) {
						if (motion.counter == 0) {
							if (debugging.in_debug_mode) {
								freeze = true;
								upwards = true;
							}	
							motion.velocity.y = salmonVel.y;
						}	
					}
				}
			
			motion.counter += 1;
			// fish has fleed from salmon
			} else {
				if (motion.closeToSalmon) {
					motion.closeToSalmon = false;
					motion.velocity.x = - fleeingSpeed;
					motion.velocity.y = 0;
				}
			}
		}
		
	}
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: DRAW DEBUG INFO HERE on AI path
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// You will want to use the createLine from world_init.hpp
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	ComponentContainer<Motion> &motion_container = registry.motions;
	if (debugging.in_debug_mode)
	{
		uint size_before_adding_new = (uint)motion_container.components.size();
		for (uint i = 0; i < size_before_adding_new; i++)
		{
			Motion& motion_i = motion_container.components[i];
			Entity entity_i = motion_container.entities[i];

			if (registry.softShells.has(entity_i)){
				vec2 offset = {50, FISH_BB_HEIGHT/2};
				vec2 line_scale = { motion_i.scale.x / 20, FISH_BB_HEIGHT};
				if (downwards) {
					Entity line = createLine(motion_i.position+offset, line_scale);
				}
				if (upwards) {
					Entity line = createLine(motion_i.position-offset, line_scale);
				}
			}
		}
	}

}