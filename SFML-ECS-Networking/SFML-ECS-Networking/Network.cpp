#include "Network.h"

Network::Network(bool host)
{
	isHost = host;

	tcpSocket.setBlocking(true);

	udpSocket.bind(sf::Socket::AnyPort);
	udpSocket.setBlocking(false);

	localPort = udpSocket.getLocalPort();

	heartbeatClock.restart();
}

//We will perform a simple handshake for connection.
bool Network::tcpHandshake()
{
	if (isHost) {
		Utils::printMsg("Waiting for the guest to join...", MessageType::debug);

		if (listener.listen(53000) != sf::Socket::Status::Done) {
			Utils::printMsg("Error listening for the other player TwT", error);
			return false;
		}

		Utils::printMsg("Listening on port 53000...", MessageType::debug);

		if (listener.accept(tcpSocket) != sf::Socket::Status::Done) {
			Utils::printMsg("Error accepting the other player TwT", error);
			return false;
		}

		Utils::printMsg("Guest connected, starting handshake.. ", MessageType::debug);

		peerIP = tcpSocket.getRemoteAddress().value();

		//Message related varibales
		std::string msg;
		char buffer[256];
		std::size_t messageSize;
		std::memset(buffer, 0, sizeof(buffer));

		msg = "HELLO_FROM_HOST";
		if (tcpSocket.send(msg.c_str(), msg.size() + 1) != sf::Socket::Status::Done) {
			Utils::printMsg("Error Sending a msg to the other player ", error);
			return false;
		}

		Utils::printMsg("Hello msg sent to guest ", MessageType::debug);

		if (tcpSocket.receive(buffer, sizeof(buffer), messageSize) != sf::Socket::Status::Done) {
			Utils::printMsg("Error receiving a msg from guest", error);
			return false;
		}

		msg = std::string(buffer);
		if (msg != "HELLO_FROM_GUEST") {
			Utils::printMsg("received wrong msg from guest", error);
			return false;
		}

		Utils::printMsg("Received Hello from guest ", MessageType::debug);

		msg = "SEND_UDP_PORT";
		if (tcpSocket.send(msg.c_str(), msg.size() + 1) != sf::Socket::Status::Done) {
			Utils::printMsg("Error Sending a msg to the other player ", error);
			return false;
		}

		Utils::printMsg("UDP Port request sent to guest ", MessageType::debug);

		std::memset(buffer, 0, sizeof(buffer));
		if (tcpSocket.receive(buffer, sizeof(buffer), messageSize) != sf::Socket::Status::Done) {
			Utils::printMsg("Error receiving a msg from guest", error);
			return false;
		}

		peerPort = static_cast<unsigned short>(std::stoi(std::string(buffer)));
		Utils::printMsg("UDP Port received from guest " + std::to_string(peerPort), MessageType::debug);

		msg = std::to_string(localPort);
		if (tcpSocket.send(msg.c_str(), msg.size() + 1) != sf::Socket::Status::Done) {
			Utils::printMsg("Error Sending a msg to the other player ", error);
			return false;
		}

		Utils::printMsg("Sent local port to gues ", MessageType::debug);

		msg = "READY";
		if (tcpSocket.send(msg.c_str(), msg.size() + 1) != sf::Socket::Status::Done) {
			Utils::printMsg("Error Sending a msg to the other player ", error);
			return false;
		}

		std::memset(buffer, 0, sizeof(buffer));
		if (tcpSocket.receive(buffer, sizeof(buffer), messageSize) != sf::Socket::Status::Done) {
			Utils::printMsg("Error receiving a msg from guest", error);
			return false;
		}

		msg = std::string(buffer);
		if (msg != "READY") {
			Utils::printMsg("received wrong msg from guest", error);
			return false;
		}

		Utils::printMsg("Handshake is complete yippee", MessageType::success); 

		//Once the connection is establishe, we change the blocking to false so that hearbeat doesnt always have to wait till it receives soemthing from the other person.

		lastPongReceived = heartbeatClock.getElapsedTime().asSeconds();
		lastPingSent = lastPongReceived;
		tcpSocket.setBlocking(false);
		return true;
	}
	else {

		//We will receive the host ip in order to connect to them.
		std::string hostIpStr;
		std::cout << "Please Enter the Host IP to join their game: " << std::endl;
		std::cin >> hostIpStr;

		auto resolveIP = sf::IpAddress::resolve(hostIpStr);
		if (tcpSocket.connect(resolveIP.value(), 53000) != sf::Socket::Status::Done) {
			Utils::printMsg("Error Connecting to the host ", error);
			return false;
		}

		Utils::printMsg("Successfully connected to the host :P", debug);
		peerIP = resolveIP.value();

		//Message related varibales
		std::string msg;
		char buffer[256];
		std::size_t messageSize;
		std::memset(buffer, 0, sizeof(buffer));

		if (tcpSocket.receive(buffer, sizeof(buffer), messageSize) != sf::Socket::Status::Done) {
			Utils::printMsg("Error receiving a msg from host", error);
			return false;
		}

		msg = std::string(buffer);
		if (msg != "HELLO_FROM_HOST") {
			Utils::printMsg("Received a wrong msg from host", error);
			return false;
		}

		Utils::printMsg("Successfully received hello from host", debug);

		msg = "HELLO_FROM_GUEST";
		if (tcpSocket.send(msg.c_str(), msg.size() + 1) != sf::Socket::Status::Done) {
			Utils::printMsg("Error sending a msg to host", error);
			return false;
		}

		std::memset(buffer, 0, sizeof(buffer));
		if (tcpSocket.receive(buffer, sizeof(buffer), messageSize) != sf::Socket::Status::Done) {
			Utils::printMsg("Error receiving a msg from host", error);
			return false;
		}

		msg = std::string(buffer);
		if (msg != "SEND_UDP_PORT") {
			Utils::printMsg("Received a wrong msg from host", error);
			return false;
		}

		msg = std::to_string(localPort);
		if (tcpSocket.send(msg.c_str(), msg.size() + 1) != sf::Socket::Status::Done) {
			Utils::printMsg("Error sending a msg to host", error);
			return false;
		}

		Utils::printMsg("Successfully sent local port to host", debug);

		std::memset(buffer, 0, sizeof(buffer));
		if (tcpSocket.receive(buffer, sizeof(buffer), messageSize) != sf::Socket::Status::Done) {
			Utils::printMsg("Error receiving a msg from host", error);
			return false;
		}

		peerPort = static_cast<unsigned short>(std::stoi(std::string(buffer)));
		Utils::printMsg("UDP Port received from host " + std::to_string(peerPort), MessageType::debug);

		std::memset(buffer, 0, sizeof(buffer));
		if (tcpSocket.receive(buffer, sizeof(buffer), messageSize) != sf::Socket::Status::Done) {
			Utils::printMsg("Error receiving a msg from host", error);
			return false;
		}

		msg = std::string(buffer);
		if (msg != "READY") {
			Utils::printMsg("Didnt receive ready from host");
			return false;
		}

		msg = "READY";
		if (tcpSocket.send(msg.c_str(), msg.size() + 1) != sf::Socket::Status::Done) {
			Utils::printMsg("Error sending a msg to host", error);
			return false;
		}

		Utils::printMsg("Handshake is complete yippee", MessageType::success);
		
		//Similar to what we did with the host.
		lastPongReceived = heartbeatClock.getElapsedTime().asSeconds();
		lastPingSent = lastPongReceived;
		tcpSocket.setBlocking(false);
		return true;
	}
}

void Network::performHearbeat()
{
	//Simple heartbeat logic to check every 2 secs.
	float currentTime = heartbeatClock.getElapsedTime().asSeconds();

	if (currentTime - lastPingSent >= 2.0f) {
		sf::Packet packet;
		std::string msg = "HEARTBEAT";

		packet << msg;
		if (tcpSocket.send(packet) == sf::Socket::Status::Done) {
			lastPingSent = currentTime;
		}
	}

	std::string msg;
	sf::Packet packet;
	sf::Socket::Status status = tcpSocket.receive(packet);

	if(status == sf::Socket::Status::Done) {
		packet >> msg;

		if (msg == "HEARTBEAT") {
			lastPongReceived = currentTime;
		}
	
	}

}

bool Network::isConnectionAlive() const
{
	//if the last message is longer than 6 seconds then we know the connection is lost
	float currentTime = heartbeatClock.getElapsedTime().asSeconds();

	if (currentTime - lastPongReceived > 6.0f) {
		return false;
	}

	return true;
}

bool Network::sendGameState(GameState sendState)
{
	sf::Packet sendPacket;

	//we send the gamestate.

	if (sendPacket << sendState.position.x << sendState.position.y << sendState.rotation.asDegrees() << sendState.lap << sendState.health << sendState.isRaceFinished << sendState.sendTime << static_cast<int>(sendState.result)) {
		if (udpSocket.send(sendPacket, peerIP, peerPort) != sf::Socket::Status::Done) {
			Utils::printMsg("We couldnt send the packet to the other person", error);
			return false;
		}
	}
	else {
		Utils::printMsg("We couldnt pass the data in the packet", warning);
	}

	return true;
}

bool Network::receiveGameState(GameState& receiveState)
{
	sf::Packet receivePacket;
	std::optional<sf::IpAddress> senderIP;
	unsigned short senderPort;
	
	//We receive the game state.

	sf::Socket::Status status = udpSocket.receive(receivePacket, senderIP, senderPort);
	if (status != sf::Socket::Status::Done) {
		//Utils::printMsg("We couldnt receive the packet from the other person", error);
		return false;
	}
	float rotDegrees;
	int resultInt;
	if (receivePacket >> receiveState.position.x >> receiveState.position.y >> rotDegrees >> receiveState.lap >> receiveState.health >> receiveState.isRaceFinished >> receiveState.sendTime >> resultInt) {
		receiveState.rotation = sf::degrees(rotDegrees);
		receiveState.result = static_cast<RaceResult>(resultInt);
	}
	else {
		Utils::printMsg("We couldnt pass the data from the packet", warning);
	}
	return true;
}


