#include "CCL.h"

namespace
{
	// Component logic functions
	std::list<std::function<void(entt::registry& reg)>> componentLogic;
}

namespace CCL {
	

	ComponentLogic::ComponentLogic(std::function<void(entt::registry& reg)> logic){
		componentLogic.push_back(logic);
	}

	void InitializeComponentLogic(entt::registry& registry){
		for (auto& logic : componentLogic) {
			logic(registry);
		}
	}
}