#pragma once
#include <time.h>

class FPS {
public:
	FPS() {
		fps = 0;
		numFrame = 0;
		lastSec = 0;
	}
	void newFrame() {
		numFrame++;
		double newTime = time(NULL);

		if (newTime >= lastSec + 1) {
			fps = numFrame;
			numFrame = 0;
			lastSec = newTime;
			printf("FPS: %d\n", getFps());
		}
	}
	int getFps() {
		return fps;
	}
private:
	int fps;
	int numFrame;
	double lastSec;
};