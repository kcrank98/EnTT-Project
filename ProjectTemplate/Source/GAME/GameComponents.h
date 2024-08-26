#ifndef GAME_COMPONENTS_H_
#define GAME_COMPONENTS_H_
#include "../UTIL/Utilities.h"

namespace GAME
{
	/// TODO: Create the tags and components for the game
	/// 
	///*** Tags ***///
	
	struct Player
	{

	};

	struct Enemy
	{

	};

	struct Bullet
	{

	};

	struct Collidable
	{

	};

	struct Obstacle
	{

	};

	///*** Components ***///
	
	struct GameManager
	{

	};
	
	struct Transform
	{
		GW::MATH::GMATRIXF transform;
	};

	struct Velocity
	{
		GW::MATH::GVECTORF direction;
	};

	struct FiringState
	{
		float coolDown;
	};

	struct Health
	{
		int healthAmount;
	};

}// namespace GAME
#endif // !GAME_COMPONENTS_H_