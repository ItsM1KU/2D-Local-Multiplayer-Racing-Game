#include "game.h"

Game::Game(bool _isHost)
{

	isHost = _isHost;

	float HEIGHT = 768;
	float WIDTH = 576;

	carTexture.loadFromFile("Assets/Cars/BlueCar.png");
	remoteCarTexture.loadFromFile("Assets/Cars/RedCar.png");


	if (isHost) {
		localCar = std::make_unique<sf::Sprite>(carTexture);
		remoteCar = std::make_unique<sf::Sprite>(remoteCarTexture);

		position = { 200.0f, 75.0f };
		remoteCar->setPosition({ 150.0f, 110.0f });
	}
	else {
		localCar = std::make_unique<sf::Sprite>(remoteCarTexture);
		remoteCar = std::make_unique<sf::Sprite>(carTexture);

		position = { 150.0f, 110.0f };
		remoteCar->setPosition({ 200.0f, 75.0f });
	}

	localCar->setScale({ 0.17f, 0.17f });
	localCar->setOrigin({ (sf::Vector2f)localCar->getTextureRect().getCenter() });
	localCar->setPosition(position);
	localCar->setRotation(rotation);

	remoteCar->setScale({ 0.17f, 0.17f });
	remoteCar->setOrigin({ (sf::Vector2f)localCar->getTextureRect().getCenter() });
	remoteCar->setRotation(sf::degrees(0));

	//Also create the sprites for the background.
	grassTexture.loadFromFile("Assets/tileGrass1.png");
	sandTexture.loadFromFile("Assets/tileSand1.png");
	checkeredTexture.loadFromFile("Assets/checkeredTile.png");
	testTexture.loadFromFile("Assets/sandTile.png");
	font.openFromFile("Assets/BearDays.ttf");

	//healthTexture.loadFromFile("Assets/health_bar.png");
	//healthSprite = std::make_unique<sf::Sprite>(healthTexture);
	//healthSprite->setScale({0.1f, 0.1f});

	countdownText = std::optional< sf::Text >(font);
	countdownText->setString("3");
	countdownText->setPosition({ 720.0f, 0.0f });
	countdownText->setCharacterSize(25);
	countdownText->setFillColor(sf::Color::Red);

	currentLapText = std::make_unique< sf::Text >(font);
	currentLapText->setString("Laps:" + std::to_string(currentLap) + "/" + std::to_string(TOTAL_LAPS));
	currentLapText->setCharacterSize(25);
	currentLapText->setPosition({ 600.0f, 0.0f });

	gameOverText = std::optional< sf::Text >(font);
	gameOverText->setString("");
	gameOverText->setPosition({ 250.0f, 0.0f });
	gameOverText->setCharacterSize(25);
	gameOverText->setFillColor(sf::Color::Red);

	winnerText = std::make_unique< sf::Text >(font);
	winnerText->setString("");
	winnerText->setCharacterSize(25);
	winnerText->setPosition({ 150.0f, 26.5f });

	currentHealth = health;

	healthRect.setPosition({10.0f, 10.0f});
	healthRect.setOrigin({0.0f, 5.0f});
	healthRect.setSize({100.0f, 15.0f});
	healthRect.setFillColor(sf::Color::Red);
	healthRect.setOutlineColor(sf::Color::Black);
	healthRect.setOutlineThickness(2.0f);

	gameClock.restart();
}

void Game::HandleEvents(const std::optional<sf::Event> event)
{
	if (gameOver) {
		return;
	}

	if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
		if (keyPressed->scancode == sf::Keyboard::Scancode::W) {
			isMoving.forward = true;
			isMoving.backward = false;
		}
		else if (keyPressed->scancode == sf::Keyboard::Scancode::A) {
			isMoving.left = true;
			isMoving.right = false;
		}
		else if (keyPressed->scancode == sf::Keyboard::Scancode::S) {
			isMoving.backward = true;
			isMoving.forward = false;
		}
		else if (keyPressed->scancode == sf::Keyboard::Scancode::D) {
			isMoving.right = true;
			isMoving.left = false;
		}
	}

	if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
		if (keyReleased->scancode == sf::Keyboard::Scancode::W) {
			isMoving.forward = false;
		}
		else if (keyReleased->scancode == sf::Keyboard::Scancode::A) {
			isMoving.left = false;
		}
		else if (keyReleased->scancode == sf::Keyboard::Scancode::S) {
			isMoving.backward = false;
		}
		else if (keyReleased->scancode == sf::Keyboard::Scancode::D) {
			isMoving.right = false;
		}
	}
}

void Game::Update(float dt)
{
	//iF THE game is over, then just display the results.
	if (gameOver) {
		DisplayGameOver();
		return;
	}

	//Only the game host can check if the game is done.
	if (isHost) {
		CheckForGameOver();
	}

	// Before Race Starts
	if (!raceStarted) {

		float timePassed = clock.getElapsedTime().asSeconds();
		float timeLeft = countdown - timePassed;

		if (timeLeft <= 0) {
			raceStarted = true;
			canMove = true;
			countdownText->setString("GO!");
		}
		else {
			int displayed = static_cast<int>(std::ceil(timeLeft));
			countdownText->setString(std::to_string(displayed));
		}
	}

	// Main Movement
	if (canMove && !gameOver)
	{
		//Basic Movement
		lastPosition = position;

		if (isMoving.left) {
			rotation -= sf::degrees(rotationSpeed * dt);
		}
		else if (isMoving.right) {
			rotation += sf::degrees(rotationSpeed * dt);
		}

		sf::Vector2f direction{
			std::cos(rotation.asRadians()),
			std::sin(rotation.asRadians())
		};

		if (isMoving.forward) {
			position += direction * movementSpeed * dt;
		}
		else if (isMoving.backward) {
			position -= direction * movementSpeed * dt;
		}

		sf::Vector2f playerPos = position;

		//Checking for collisions with grass and inside sand
		float sensorPlacement = 20.0f;
		sf::Vector2f frontBumper = playerPos + sf::Vector2f(std::cos(rotation.asRadians()), std::sin(rotation.asRadians())) * sensorPlacement;

		int tileX = frontBumper.x / tileRes;
		int tileY = frontBumper.y / tileRes;

		if (tileX >= 0 && tileX < columns && tileY >= 0 && tileY < rows) {

			int tileType = gameMap[tileY][tileX];

			if (tileType == 0) {
				//On road
				movementSpeed = 75.0f;
			}
			else if (tileType == 1) {
				//On grass
				movementSpeed = 100.0f;
			}
			else if (tileType == 2) {
				//On sand
				position = lastPosition;
				localCar->setPosition(lastPosition);
				currentHealth -= 0.05f;
				setHealth();

				if (currentHealth <= 0.0f) {
					currentHealth = 0.0f;
				}
				
			}


			if (tileType == 3) {
				//On Checkered Line/Flag

				if (!inFinishZone && crossedLine)
				{
					currentLap++;
					currentLapText->setString("Laps:" + std::to_string(currentLap) + "/" + std::to_string(TOTAL_LAPS));

					if (currentLap >= TOTAL_LAPS)
					{
						raceFinished = true;
					}
					crossedLine = false;
				}

				inFinishZone = true;
			}
			else {
				if (inFinishZone) {
					crossedLine = true;
				}
				inFinishZone = false;
			}
		}

		//Checking for window bounds
		if (playerPos.x < 0 || playerPos.y < 0 || playerPos.x > 768 || playerPos.y > 576) {
			position = lastPosition;
		}

		//Assigning the final values
		localCar->setRotation(rotation);
		localCar->setPosition(position);
	}

	//We calculate the interpolated position and rotation and assign them to the remote car.
	if (remoteCarIPT.hasOneState) {
		float remoteCarT = gameClock.getElapsedTime().asSeconds();
		GameState interpolatedState = remoteCarIPT.Interpolate(remoteCarT);

		remoteCar->setPosition(interpolatedState.position);
		remoteCar->setRotation(interpolatedState.rotation);
	}

	CheckForGameOver();
}

void Game::Render(sf::RenderWindow& window)
{
	SetBackground(window);
	window.draw(*localCar);
	window.draw(*remoteCar);
	window.draw(*countdownText);
	window.draw(*currentLapText);
	window.draw(*gameOverText);
	window.draw(*winnerText);
	window.draw(healthRect);
}

void Game::SetBackground(sf::RenderWindow& window)
{
	int spriteWidth = 64;
	int spriteHeight = 64;

	// Since the width is 800 and divided by image pixels which is 64
	const int mapRows = 9;
	const int mapColumns = 12;

	int map[mapRows][mapColumns] = {
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

	for (int i = 0; i < mapRows; i++) {
		for (int j = 0; j < mapColumns; j++) {
			if (map[i][j] == 0) {
				backgroundSprite = std::make_unique<sf::Sprite>(grassTexture);
			}
			else if (map[i][j] == 1) {
				backgroundSprite = std::make_unique<sf::Sprite>(sandTexture);
			}
			else if(map[i][j] == 3) {
				backgroundSprite = std::make_unique<sf::Sprite>(checkeredTexture);
			}
			else if (map[i][j] == 2) {
				backgroundSprite = std::make_unique<sf::Sprite>(testTexture);
			}
			backgroundSprite->setPosition(sf::Vector2f(j * static_cast<float>(spriteWidth), i * static_cast<float>(spriteHeight)));
			window.draw(*backgroundSprite);
		}
	}
}

void Game::setHealth() {
	
	if (currentHealth >= 0) {
		healthRect.setScale({static_cast<float>(currentHealth) / health, 1.0f});
	}
}

void Game::CheckForGameOver()
{
	if (!isHost || gameOver) {
		return;
	}

	if (raceFinished && localResult == RaceResult::No) {
		localResult = RaceResult::WonWithLaps;
		remoteResult = RaceResult::LostWithLaps;
		gameOver = true;
		return;
	}

	if (isRemoteFinished && localResult == RaceResult::No) {
		localResult = RaceResult::LostWithLaps;
		remoteResult = RaceResult::WonWithLaps;
		gameOver = true;
		return;
	}

	if (currentHealth <= 0 && localResult == RaceResult::No) {
		localResult = RaceResult::LostWithHealth;
		remoteResult = RaceResult::WonWithHealth;
		gameOver = true;
		return;
	}

	if (remoteHealth <= 0 && localResult == RaceResult::No) {
		localResult = RaceResult::WonWithHealth;
		remoteResult = RaceResult::LostWithHealth;
		gameOver = true;
		return;
	}
}

void Game::DisplayGameOver()
{
	if (!gameOver && localResult == RaceResult::No) {
		return;
	}

	gameOverText->setString("Game Over!!!");
	std::string winnerString;

	switch (localResult)
	{
		case RaceResult::WonWithLaps:
			winnerString = "You win!! You finished first... DU DU DU";
			break;
		case RaceResult::LostWithLaps:
			winnerString = "You lost!! player nowins lol";
			break;
		case RaceResult::WonWithHealth:
			winnerString = "The other player might be Stroll, You win!!";
			break;
		case RaceResult::LostWithHealth:
			winnerString = "You crashed too much kiddo!, You lose";
			break;
	}


	winnerText->setString(winnerString);
}

GameState Game::GetUpdate()
{
	GameState output;

	output.position = position;
	output.rotation = rotation;
	output.lap = currentLap;
	output.health = currentHealth;
	output.isRaceFinished = raceFinished;
	output.sendTime = gameClock.getElapsedTime().asSeconds();	
	
	//If we are the host, we can send the result so the guest knows if the game ended.
	if (isHost) {
		output.result = remoteResult;
	}
	else {
		output.result = RaceResult::No;
	}

	return output;
}

void Game::UpdateRemoteCar(GameState remoteState)
{
	remotecarPos = remoteState.position;
	remotecarRot = remoteState.rotation;
	remoteLap = remoteState.lap;
	remoteHealth = remoteState.health;
	isRemoteFinished = remoteState.isRaceFinished;


	//Helps the guest know what happened with the game result.
	if (remoteState.result != RaceResult::No) {
		if (isHost) {
			remoteResult = remoteState.result;
		}
		else {
			localResult = remoteState.result;
			gameOver = true;
		}
	}

	remoteCarIPT.AddState(remoteState);
}
