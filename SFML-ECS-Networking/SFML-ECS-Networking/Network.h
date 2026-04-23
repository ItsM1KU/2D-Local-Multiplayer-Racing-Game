#pragma once
#include <SFML/Network.hpp>
#include <iostream>
#include "utils.h"
#include "Gamestate.h"

class Network
{
public:
	Network(bool host);
	bool tcpHandshake();
	void performHearbeat();
	bool isConnectionAlive() const;

	bool sendGameState(GameState sendState);
	bool receiveGameState(GameState& receiveState);

private:
	bool isHost;
	sf::TcpListener listener;
	sf::TcpSocket tcpSocket;
	sf::UdpSocket udpSocket;

	sf::IpAddress peerIP = sf::IpAddress::Any;
	unsigned short peerPort;

	unsigned short localPort;

	sf::Clock heartbeatClock;
	float lastPingSent = 0.0f;
	float lastPongReceived = 0.0f;
};

