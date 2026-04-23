#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Main.hpp>
#include <SFML/System.hpp>
#include <SFML/Network.hpp>
#include "game.h"
#include "utils.h"
#include "Network.h"


int main() {

	bool isHost = false;

	bool canGameStart = false;
	
	std::string windowName;

	//We will first check if the person wants to be the host or guest.
	if (!canGameStart) {

		Utils::printMsg("Are you the host (Player 1) ? Enter 1 if Yes and 2 if No");
		std::string input;
		std::cin >> input;
		std::cout << "HOST INPUT RECEIVED: " << input << std::endl;

		if(input == "1") {
			isHost = true;
			Utils::printMsg("You are the host.");
			windowName = "F125 - Host";
		}
		else if (input == "2") {
			isHost = false;
			Utils::printMsg("You are a guest.");
			windowName = "F125 - Guest";
		}
		else {
			Utils::printMsg("Invalid input. Exiting...");
			return 1;
		}	
	}

	Network network(isHost);

	//IF the tcp handshake returns true, we can start the game as that is the indication that the connection is good.

	if(network.tcpHandshake()) {
		canGameStart = true;
	}
	else {
		Utils::printMsg("TCP Handshake failed. Exiting...");
		return 1;
	}

	if(canGameStart) {

		unsigned int width = 768;
		unsigned int height = 576;

		sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode({ width, height }), windowName);

		window->setFramerateLimit(60);

		sf::Clock clock;
		sf::Clock sendClock;
		sf::Clock receiveClock;
		Game game(isHost);
		float gameSpeed = 1.0f;
		int frameCounter = 0;
		float sendRate = 0.02f;
		float receiveRate = 0.01f;


		while (window->isOpen()) {

			float dt = clock.restart().asSeconds() * gameSpeed;
			frameCounter++;

			//We keep checking every 30 frames for a hearbeat and if there is none, we close the window as we lost the connection.
			if (frameCounter >= 30) {
				network.performHearbeat();

				if (!network.isConnectionAlive()) {
					Utils::printMsg("Connection lost, closing window...", error);

					window->close();
					break;
				}

				frameCounter = 0;
			}

			//We will send the msgs every 20ms in order to have a smooth motion.
			float sendtimer = sendClock.getElapsedTime().asSeconds();
			if (sendtimer >= sendRate) {
				sendClock.restart();

				network.sendGameState(game.GetUpdate());

			}

			//We receive the state seperately so that one window is not delayed compared to the other and both can receive at the same time.
			float receiveTimer = receiveClock.getElapsedTime().asSeconds();
			if (receiveTimer >= receiveRate) {
				GameState gamestate;
				if (network.receiveGameState(gamestate)) {
					game.UpdateRemoteCar(gamestate);
				}
				receiveClock.restart();
			}

			while (const std::optional event = window->pollEvent()) {

				if (event->is<sf::Event::Closed>()) {
					window->close();
				}
				else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
					if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
						window->close();
					}
				}
				game.HandleEvents(event);

			}
			game.Update(dt);

			window->clear(sf::Color::Red);

			game.Render(*window);

			window->display();
		}

		delete window;
	}
	return 0;
}