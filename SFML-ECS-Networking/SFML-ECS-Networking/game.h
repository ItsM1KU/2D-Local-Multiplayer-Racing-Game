#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Network.h"
#include "Gamestate.h"
#include "interpolation.h"

class Game
{
public:
	Game(bool _isHost);

	// Movement Struct
	struct {
		bool forward = false;
		bool backward = false;
		bool left = false;
		bool right = false;
	} isMoving;

	//Member Functions
	void HandleEvents(const std::optional<sf::Event> event);
	void Update(float dt);
	void Render(sf::RenderWindow& window);
	void SetBackground(sf::RenderWindow& window);
	void setHealth();
	void CheckForGameOver();
	void DisplayGameOver();

	GameState GetUpdate();
	void UpdateRemoteCar(GameState remoteState);

	sf::Vector2f position = { 200.0f, 75.0f };
	sf::Angle rotation = sf::degrees(0);
	sf::Vector2f lastPosition;

	sf::Clock clock;
	sf::Clock gameClock;

	bool raceStarted = false;
	float countdown = 5.0f;
	bool canMove = false;

	// Laps related
	int currentLap = 0;
	const int TOTAL_LAPS = 3;
	bool crossedLine = false;
	bool raceFinished = false;
	bool inFinishZone = false;

	bool IsGameOver() { return gameOver; }

	sf::Font font;
	std::optional<sf::Text> countdownText;
	std::unique_ptr<sf::Text> currentLapText;

	// Mapping the race track.
	const int tileRes = 64;
	const int rows = 9;
	const int columns = 12;

	int gameMap[9][12] = {
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 0},
		{0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0},
		{0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0},
		{0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0},
		{0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0},
		{0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0},
		{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	};


private:
	std::unique_ptr<sf::Sprite> localCar;
	std::unique_ptr<sf::Sprite> remoteCar;
	std::unique_ptr < sf::Sprite> backgroundSprite;
	std::unique_ptr<sf::Sprite> healthSprite;

	sf::Texture carTexture;
	sf::Texture remoteCarTexture;
	sf::Texture grassTexture;
	sf::Texture sandTexture;
	sf::Texture checkeredTexture;
	sf::Texture testTexture;
	sf::Texture healthTexture;

	float movementSpeed = 150.0f;
	float rotationSpeed = 100.0f;

	sf::RectangleShape healthRect;
	float currentHealth;
	float health = 10.0f;

	//Remote Car Details
	sf::Vector2f remotecarPos;
	sf::Angle remotecarRot;
	int remoteLap = 0;
	float remoteHealth = 10.0f;
	bool isRemoteFinished = false;

	bool gameOver = false;
	RaceResult localResult = RaceResult::No;
	RaceResult remoteResult = RaceResult::No;
	std::optional<sf::Text> gameOverText;
	std::unique_ptr<sf::Text> winnerText;

	Interpolation remoteCarIPT;

	//Network network;
	bool isHost;
};
