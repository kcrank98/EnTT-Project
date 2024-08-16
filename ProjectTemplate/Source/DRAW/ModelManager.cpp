//need this include for the CONNECT_COMPONENT_LOGIC()
#include "DrawComponents.h"
#include "../CCL.h"

//struct MESH_COLLECTION
//{
//	std::vector<entt::entity> dynamicEntities;
//};
//
//struct ModelManager
//{
//	std::map<std::string, MESH_COLLECTION> MeshCollections;
//};

//*** Helper Methods ***

void Destroy_ModelManager(entt::registry& registry, entt::entity entity) {
	//get the ModelManager from the passed in entity
	DRAW::ModelManager modelManager = registry.get<DRAW::ModelManager>(entity);

	//loop thru the MeshCollection and destroy all the entities
	for (const auto& [key, val] : modelManager.MeshCollections) {
		auto& meshVec = modelManager.MeshCollections[key];

		for (int i = 0; i < meshVec.dynamicEntities.size(); ++i) {
			
			registry.destroy(meshVec.dynamicEntities[i]);
		}
	}
}

// Use this MACRO to connect the EnTT Component Logic
CONNECT_COMPONENT_LOGIC() {
	//need this or weird stuff will happen...
	registry.on_destroy<DRAW::ModelManager>().connect<Destroy_ModelManager>();
}