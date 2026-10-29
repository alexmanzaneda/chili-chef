#ifndef _ANIMATION_KEYFRAMES
#define _ANIMATION_KEYFRAMES


#include <vector>


using namespace std;


// AnimKeyframes contains all information related to a single animation.
// These are the animation speed measured by millisecsPerKeyframe
// and texture coordinates for all keyframes.


struct AnimKeyframes
{
	float millisecsPerKeyframe;
	vector<glm::vec2> keyframeDispl;
	bool once; // Played a single time and stops on the last keyframe (false: loops)
};


#endif // _ANIMATION_KEYFRAMES


