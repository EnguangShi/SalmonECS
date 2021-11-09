// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include "iostream"

// stlib
#include <chrono>
#include <limits>

using Clock = std::chrono::high_resolution_clock;

// Game configuration
auto last_flipped_time = Clock::now();
float flipped_cooldown = 1.f;

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool collides(const Motion& motion1, const Motion& motion2)
{
	vec2 dp = motion1.position - motion2.position;
	float dist_squared = dot(dp,dp);
	const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
	const float other_r_squared = dot(other_bonding_box, other_bonding_box);
	const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
	const float my_r_squared = dot(my_bonding_box, my_bonding_box);
	const float r_squared = max(other_r_squared, my_r_squared);
	if (dist_squared < r_squared)
		return true;
	return false;
}

void PhysicsSystem::step(float elapsed_ms, float window_width_px, float window_height_px)
{

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: DRAW DEBUG INFO HERE on Salmon mesh collision
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// You will want to use the createLine from world_init.hpp
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// debugging of bounding boxes
	ComponentContainer<Motion> &motion_container = registry.motions;
	if (debugging.in_debug_mode)
	{
		uint size_before_adding_new = (uint)motion_container.components.size();
		for (uint i = 0; i < size_before_adding_new; i++)
		{
			Motion& motion_i = motion_container.components[i];
			Entity entity_i = motion_container.entities[i];

			// salmon's box
			if (registry.players.has(entity_i)){
				std::vector<ColoredVertex> salmon_vertices = registry.meshPtrs.get(entity_i)->vertices;
				float uppermostLineY = std::numeric_limits<float>::max();;
				float downmostLineY = 0.f;
				float leftmostLineX = std::numeric_limits<float>::max();;
				float rightmostLineX = 0.f;

				for (uint i = 0; i < salmon_vertices.size(); i++){
					float x = salmon_vertices[i].position.x * motion_i.scale.x;
					float y = salmon_vertices[i].position.y * motion_i.scale.y;
					float angle = motion_i.angle;
					float rotate_x = sqrt(x*x+y*y)*cos(angle+atan2(y,x));
					float rotate_y = sqrt(x*x+y*y)*sin(angle+atan2(y,x));
					vec2 xy = {rotate_x, rotate_y};

					// show salmon's vertices
					vec2 line_scale = { motion_i.scale.x / 20, motion_i.scale.x / 20 };
					Entity dot = createLine(motion_i.position+xy, line_scale);

					// pick the vertex that is farthest from the salmon's center, record the orthogonal distance as maxXY
					if (rotate_y < uppermostLineY){
						uppermostLineY = rotate_y;
					}
					if (rotate_y > downmostLineY){
						downmostLineY = rotate_y;
					}
					if (rotate_x < leftmostLineX){
						leftmostLineX = rotate_x;
					}
					if (rotate_x > rightmostLineX){
						rightmostLineX = rotate_x;
					}
				}
				float maxY = max(-uppermostLineY, downmostLineY);
				float maxX = max(-leftmostLineX, rightmostLineX);
				float maxXY = max(maxY, maxX);
				
				// create salmon's box
				vec2 maxXYX = {maxXY, 0};
				vec2 maxXYY = {0, maxXY};
				vec2 line_scale1 = { motion_i.scale.x / 20, 2*maxXY };
				Entity line1 = createLine(motion_i.position+maxXYX, line_scale1);
				Entity line3 = createLine(motion_i.position-maxXYX, line_scale1);
				vec2 line_scale2 = { 2*maxXY, motion_i.scale.x / 20};
				Entity line2 = createLine(motion_i.position+maxXYY, line_scale2);
				Entity line4 = createLine(motion_i.position-maxXYY, line_scale2);	

				// create salmon's outer box indicating the avoiding distance of fishes
				vec2 avoidX = {AVOIDING_DISTANCE,0};
				vec2 avoidY = {0, AVOIDING_DISTANCE};
				vec2 line_scale3 = { motion_i.scale.x / 20, 2*AVOIDING_DISTANCE };
				Entity line5 = createLine(motion_i.position+avoidX, line_scale3);
				Entity line6 = createLine(motion_i.position-avoidX, line_scale3);
				vec2 line_scale4 = { 2*AVOIDING_DISTANCE, motion_i.scale.x / 20};
				Entity line7 = createLine(motion_i.position+avoidY, line_scale4);
				Entity line8 = createLine(motion_i.position-avoidY, line_scale4);	
			}

			// fishes' boxes
			if (registry.softShells.has(entity_i)){
				vec2 fish_x = {FISH_BB_WIDTH/2,0};
				vec2 fish_y = {0,FISH_BB_HEIGHT/2};
				vec2 line_scale1 = { motion_i.scale.x / 20, FISH_BB_HEIGHT};
				Entity line1 = createLine(motion_i.position+fish_x, line_scale1);
				Entity line3 = createLine(motion_i.position-fish_x, line_scale1);
				vec2 line_scale2 = { FISH_BB_WIDTH, motion_i.scale.x / 20};
				Entity line2 = createLine(motion_i.position+fish_y, line_scale2);
				Entity line4 = createLine(motion_i.position-fish_y, line_scale2);
			}

			// turtles' boxes
			if (registry.hardShells.has(entity_i)){
				vec2 turtle_x = {TURTLE_BB_WIDTH/2,0};
				vec2 turtle_y = {0,TURTLE_BB_HEIGHT/2};
				vec2 line_scale1 = { motion_i.scale.x / 20, TURTLE_BB_HEIGHT};
				Entity line1 = createLine(motion_i.position+turtle_x, line_scale1);
				Entity line3 = createLine(motion_i.position-turtle_x, line_scale1);
				vec2 line_scale2 = { TURTLE_BB_WIDTH, motion_i.scale.x / 20};
				Entity line2 = createLine(motion_i.position+turtle_y, line_scale2);
				Entity line4 = createLine(motion_i.position-turtle_y, line_scale2);
			}
			// !!! TODO A2: implement debugging of bounding boxes and mesh
		}
	}
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE SALMON - WALL collisions HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	auto& motion_registry = registry.motions;
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		Motion& motion = motion_registry.components[i];  // position...
		Entity entity = motion_registry.entities[i];  // salmon, fish, turtle
		float step_seconds = 1.0f * (elapsed_ms / 1000.f);
		if (registry.players.has(entity)) {
			std::vector<ColoredVertex> salmon_vertices = registry.meshPtrs.get(entity)->vertices;
			float uppermostVertexPosY = std::numeric_limits<float>::max();
			vec2 uppermostVertex;
			float downmostVertexPosY = 0.f;
			vec2 downmostVertex;
			float leftmostVertexPosX = std::numeric_limits<float>::max();
			vec2 leftmostVertex;
			float rightmostVertexPosX = 0.f;
			vec2 rightmostVertex;
			for (uint i = 0; i < salmon_vertices.size(); i++){

				// compute all salmon's mesh vertices' positions given the salmon angle and position
				float x = salmon_vertices[i].position.x * motion.scale.x;
				float y = salmon_vertices[i].position.y * motion.scale.y;
				float angle = motion.angle;
				float rotate_x = sqrt(x*x+y*y)*cos(angle+atan2(y,x));
				float rotate_y = sqrt(x*x+y*y)*sin(angle+atan2(y,x));
				float position_x = motion.position.x + rotate_x;
				float position_y = motion.position.y + rotate_y;

				// pick the uppermost, downmost, leftmost and rightmost vertices from all salmon's mesh vertices
				if (position_y < uppermostVertexPosY){
					uppermostVertexPosY = position_y;
					uppermostVertex = {position_x, position_y};
				}
				if (position_y > downmostVertexPosY){
					downmostVertexPosY = position_y;
					downmostVertex = {position_x, position_y};
				}
				if (position_x < leftmostVertexPosX){
					leftmostVertexPosX = position_x;
					leftmostVertex = {position_x, position_y};
				}
				if (position_x > rightmostVertexPosX){
					rightmostVertexPosX = position_x;
					rightmostVertex = {position_x, position_y};
				}
			}

			// salmon's movement when not colliding with walls
			if (!(leftmostVertexPosX <= 0 && motion.velocity.x < 0) && 
				(!(rightmostVertexPosX >= window_width_px && motion.velocity.x > 0)))
			{
					motion.position.x += step_seconds * motion.velocity.x;
			}
			if (!(uppermostVertexPosY <= 0 && motion.velocity.y < 0) && 
				(!(downmostVertexPosY >= window_height_px && motion.velocity.y > 0)))
			{
					motion.position.y += step_seconds * motion.velocity.y;
			}

			// push salmon away from the wall because of rotation
			if (leftmostVertexPosX <= 0) {
				motion.position.x = abs(motion.position.x - leftmostVertexPosX);
				if (debugging.in_debug_mode) {
					vec2 line_scale = { motion.scale.x / 8, motion.scale.x / 8 };
					Entity dot = createLine(leftmostVertex, line_scale);
				}
			}
			if (rightmostVertexPosX >= window_width_px) {
				motion.position.x = window_width_px - abs(motion.position.x - rightmostVertexPosX);
				if (debugging.in_debug_mode) {
					vec2 line_scale = { motion.scale.x / 8, motion.scale.x / 8 };
					Entity dot = createLine(rightmostVertex, line_scale);
				}
			}
			if (uppermostVertexPosY <= 0) {
				motion.position.y = abs(motion.position.y - uppermostVertexPosY);
				if (debugging.in_debug_mode) {
					vec2 line_scale = { motion.scale.x / 8, motion.scale.x / 8 };
					Entity dot = createLine(uppermostVertex, line_scale);
				}
			}
			if (downmostVertexPosY >= window_height_px) {
				motion.position.y = window_height_px - abs(motion.position.y - downmostVertexPosY);
				if (debugging.in_debug_mode) {
					vec2 line_scale = { motion.scale.x / 8, motion.scale.x / 8 };
					Entity dot = createLine(downmostVertex, line_scale);
				}
			}
		}

		// !!! TODO A1: update motion.position based on step_seconds and motion.velocity
		else {  // !registry.players.has(entity)
			motion.position.x += step_seconds * motion.velocity.x;
			motion.position.y += step_seconds * motion.velocity.y;
		}
		// A2: flip the fish's vertical velocity if its position is near the top and bottom walls
		//     and set a cooldown time of 1s for flipping
		if (registry.softShells.has(entity)){
			if ((motion.position.y >= -3 && motion.position.y <= 3) || (motion.position.y >= window_height_px-3 && motion.position.y <= window_height_px+3)) {
				auto now = Clock::now();
				float elapsed_ms_since_flipped =
					(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - last_flipped_time)).count() / 1000;
				if (elapsed_ms_since_flipped >= flipped_cooldown){
					motion.velocity.y = - motion.velocity.y;
				}
				last_flipped_time = now;
			}
		}
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A3: HANDLE PEBBLE UPDATES HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 3
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Check for collisions between all moving entities
	for(uint i = 0; i<motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		for(uint j = 0; j<motion_container.components.size(); j++) // i+1
		{
			if (i == j)
				continue;

			Motion& motion_j = motion_container.components[j];
			if (collides(motion_i, motion_j))
			{
				Entity entity_j = motion_container.entities[j];
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A3: HANDLE PEBBLE collisions HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 3
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}