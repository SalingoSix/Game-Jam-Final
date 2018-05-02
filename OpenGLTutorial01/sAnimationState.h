#ifndef _HG_sAnimationState_
#define _HG_sAnimationState_

#include <string>
#include <vector>

class sAnimationState
{
public:
	sAnimationState() {};

	struct sStateDetails
	{
		sStateDetails() :
			currentTime(0.0f),
			totalTime(0.0f),
			frameStepTime(0.0f) {};
		std::string name;
		float currentTime;		// Time (frame) in current animation
		float totalTime;		// Total time animation goes
		float frameStepTime;	// Number of seconds to 'move' the animation	
								// Returns true if time had to be reset
								// (for checking to see if the animation has finished or not)
								// TODO: Deal with running the animation backwards, perhaps?? 
		bool IncrementTime(bool bResetToZero = true);
	};

	sStateDetails defaultAnimation;
};
#endif