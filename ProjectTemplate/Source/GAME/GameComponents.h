#ifndef GAME_COMPONENTS_H_
#define GAME_COMPONENTS_H_

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

	///*** Components ***///
	
	struct GameManager
	{

	};
	
	struct Transform
	{
		GW::MATH::GMATRIXF transform;
	};

	struct FiringState
	{
		float coolDown;
	};

}// namespace GAME
#endif // !GAME_COMPONENTS_H_