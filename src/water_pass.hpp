//
// Created by rfdic on 7/22/2024.
//

#ifndef TOYRENDERER_WATER_PASS_HPP
#define TOYRENDERER_WATER_PASS_HPP

namespace obsidian
{

struct Init;

struct WaterUniforms {
	float wave_strength;
	float wave_speed;
	glm::vec2 wave_direction;
};

class WaterPass
{
	  public:
	WaterPass(Init &init);



};

}        // namespace obsidian

#endif        // TOYRENDERER_WATER_PASS_HPP
