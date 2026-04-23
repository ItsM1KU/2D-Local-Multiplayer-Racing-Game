#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

//We use this to keep a track on the gamestate
enum class RaceResult {
	No,
	WonWithLaps,
	LostWithLaps,
	WonWithHealth,
	LostWithHealth
};

struct GameState {

	sf::Vector2f position = {0.0f, 0.0f};
	sf::Angle rotation = sf::degrees(0);
	int lap = 0;
	float health = 0;
	bool isRaceFinished;
	float sendTime = 0;
	RaceResult result = RaceResult::No;
};
