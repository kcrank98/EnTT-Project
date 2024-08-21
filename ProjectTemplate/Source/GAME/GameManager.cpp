#include "GameComponents.h"
#include "../DRAW/DrawComponents.h"
#include "../CCL.h"

void Update_Velocity(entt::registry& registry) {
	//says i need delta time but idk how to use that so maybe this will work?
	auto& deltaTime = registry.get<UTIL::DeltaTime>(registry.view<UTIL::DeltaTime>().front()).dtSec;

	static auto start = std::chrono::steady_clock::now();
	double elapsed = std::chrono::duration<double>(
		std::chrono::steady_clock::now() - start).count();
	start = std::chrono::steady_clock::now();
	deltaTime = elapsed;

	//create view of everything with a transform AND a velocity
	entt::basic_view view = registry.view<GAME::Transform, GAME::Velocity>();

	for (auto& [ent, trans, velo] : view.each()) {

		//copy obj velo
		GW::MATH::GVECTORF veloCopy = velo.direction;

		//scale by deltaTime
		veloCopy.x *= deltaTime;
		veloCopy.z *= deltaTime;

		//translate
		GW::MATH::GMatrix::TranslateGlobalF(trans.transform,
											veloCopy,
											trans.transform);

	}

}

void Update_GameManager(entt::registry& registry, entt::entity entity) {
	//patch the player using a view
	registry.patch<GAME::Player>(registry.view<GAME::Player>().front());

	//call update_velocity here
	Update_Velocity(registry);

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