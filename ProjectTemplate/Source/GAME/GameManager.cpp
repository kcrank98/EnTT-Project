#include "GameComponents.h"
#include "../DRAW/DrawComponents.h"
#include "../CCL.h"

void Update_GameManager(entt::registry& registry, entt::entity entity) {
	//patch the player using a group
	//entt::basic_group group = registry.group<>(entt::get<GAME::Player>);
	//entt::entity playerID = registry.group<>(entt::get<GAME::Player>).front();
	registry.patch<GAME::Player>(registry.view<GAME::Player>().front());

	//update the position of all the models
	//create a view for all the entities that have a transform and a meshcollection
	entt::basic_view view = registry.view<GAME::Transform, DRAW::MESH_COLLECTION>();

	//loop for each entity and copy the transform to the GPUInstance
	for (auto [ent, transorm, meshes] : view.each()) {
		
		for (auto meshID : meshes.dynamicEntities) {
			//GAME::Transform currTransform = registry.get<GAME::Transform>(meshID);
			DRAW::GPUInstance& currGPU = registry.get<DRAW::GPUInstance>(meshID);

			currGPU.transform = transorm.transform;
		}
	}

	//for (auto& iter = view.begin(); iter != view.end(); iter++) {
	//	DRAW::MESH_COLLECTION currMeshCollection = registry.get<DRAW::MESH_COLLECTION>(*iter);
	//	//auto& currTransform = registry.get<GAME::Transform>(*iter);
	//	//auto& currGPU = registry.get<DRAW::GPUInstance>(*iter);
	//	
	//	for (int i = 0; i < currMeshCollection.dynamicEntities.size(); ++i) {
	//		entt::entity currMesh = currMeshCollection.dynamicEntities[i];

	//		GAME::Transform currTransform = registry.get<GAME::Transform>(currMesh);
	//		DRAW::GPUInstance currGPU = registry.get<DRAW::GPUInstance>(currMesh);

	//		currGPU.transform = currTransform.transform;
	//	}
	//}
}

CONNECT_COMPONENT_LOGIC() {
	registry.on_update<GAME::GameManager>().connect<Update_GameManager>();
}