#pragma once
#include "Gamestate.h"

struct Interpolation {
	GameState previousState;
	GameState currentState;

	bool hasOneState = false;

	//We add a new state and remove the old one if there are already two states.
	void AddState(GameState state) {
		if (!hasOneState) {
			previousState = state;
			currentState = state;
			hasOneState = true;
		}
		else {
			previousState = currentState;
			currentState = state;
		}
	}

	//Seperate function to calculate the interpolated angle.
	float AngleInterpolate(float prev, float curr) {
		float d = std::fmod(curr - prev, 360.0f);
		if (d > 180.0f) {
			d -= 360.0f;
		}
		if (d < -180.0f) {
			d += 360.0f;
		}

		return d;
	}

	//Simple linear interpolation.
	GameState Interpolate(float dt) {
		GameState output = currentState;

		float t0 = previousState.sendTime;
		float t1 = currentState.sendTime;

		if (t1 - t0 < 0.0001f) {
			return output;
		}

		float inter_t = (dt - t0) / (t1 - t0);
		if (inter_t > 1.0f) {
			inter_t = 1.0f;
		}

		output.position = ((1 - inter_t) * previousState.position) + (inter_t * currentState.position);
		
		float d = AngleInterpolate(previousState.rotation.asDegrees(), currentState.rotation.asDegrees());

		float interpolated_d = previousState.rotation.asDegrees() + d * inter_t;

		output.rotation = sf::degrees(interpolated_d);

		return output;
	}
};
