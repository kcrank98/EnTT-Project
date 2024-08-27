// main entry point for the application
// enables components to define their behaviors locally in an .hpp file
#include "CCL.h"
#include "UTIL/Utilities.h"
// include all components, tags, and systems used by this program
#include "DRAW/DrawComponents.h"
#include "GAME/GameComponents.h"
#include "APP/Window.hpp"



// Local routines for specific application behavior
void GraphicsBehavior(entt::registry& registry);
void GameplayBehavior(entt::registry& registry);
void MainLoopBehavior(entt::registry& registry);

// Architecture is based on components/entities pushing updates to other components/entities (via "patch" function)
int main()
{

	// All components, tags, and systems are stored in a single registry
	entt::registry registry;	

	// initialize the ECS Component Logic
	CCL::InitializeComponentLogic(registry);

	auto gameConfig = registry.create();
	registry.emplace<UTIL::Config>(gameConfig);

	GraphicsBehavior(registry); // create windows, surfaces, and renderers

	GameplayBehavior(registry); // create entities and components for gameplay
	
	MainLoopBehavior(registry); // update windows and input

	// clear all entities and components from the registry
	// invokes on_destroy() for all components that have it
	// registry will still be intact while this is happening
	registry.clear(); 

	return 0; // now destructors will be called for all components
}

// This function will be called by the main loop to update the graphics
// It will be responsible for loading the Level, creating the VulkanRenderer, and all VulkanInstances
void GraphicsBehavior(entt::registry& registry)
{
	std::shared_ptr<const GameConfig> config = registry.get<UTIL::Config>(
		registry.view<UTIL::Config>().front()).gameConfig;

	int windowWidth = (*config).at("Window").at("width").as<int>();
	int windowHeight = (*config).at("Window").at("height").as<int>();
	int startX = (*config).at("Window").at("xstart").as<int>();
	int startY = (*config).at("Window").at("ystart").as<int>();
	std::string title = (*config).at("Window").at("title").as<std::string>();

	// Add an entity to handle all the graphics data
	auto display = registry.create();
	registry.emplace<APP::Window>(display,
		APP::Window{ startX, startY, windowWidth, windowHeight, GW::SYSTEM::GWindowStyle::WINDOWEDBORDERED, title});

	std::string vertShader = (*config).at("Shaders").at("vertex").as<std::string>();
	std::string pixelShader = (*config).at("Shaders").at("pixel").as<std::string>();
	registry.emplace<DRAW::VulkanRendererInitialization>(display,
		DRAW::VulkanRendererInitialization{ 
			vertShader, pixelShader,
			{ {0.2f, 0.2f, 0.25f, 1} } , { 1.0f, 0u }, 75.f, 0.1f, 100.0f });
	registry.emplace<DRAW::VulkanRenderer>(display);
	
	// TODO : Load the Level then update the Vertex and Index Buffers

	std::string jsonPath = (*config).at("Level1").at("levelFile").as<std::string>();
	std::string modelPath = (*config).at("Level1").at("modelPath").as<std::string>();
	registry.emplace<DRAW::CPULevel>(display, jsonPath, modelPath);

	registry.emplace<DRAW::GPULevel>(display);

	//registry.patch<DRAW::CPULevel>(display);
	//registry.patch<DRAW::GPULevel>(display);

	// Register for Vulkan clean up
	GW::CORE::GEventResponder shutdown;
	shutdown.Create([&](const GW::GEvent& e) {
		GW::GRAPHICS::GVulkanSurface::Events event;
		GW::GRAPHICS::GVulkanSurface::EVENT_DATA data;
		if (+e.Read(event, data) && event == GW::GRAPHICS::GVulkanSurface::Events::RELEASE_RESOURCES) {
			registry.clear<DRAW::VulkanRenderer>();
		}
		});
	registry.get<DRAW::VulkanRenderer>(display).vlkSurface.Register(shutdown);
	registry.emplace<GW::CORE::GEventResponder>(display, shutdown.Relinquish());

	// Create a camera entity and emplace it
	auto camera = registry.create();
	GW::MATH::GMATRIXF initialCamera;
	GW::MATH::GVECTORF translate = { 0.0f,  45.0f, -5.0f };
	GW::MATH::GVECTORF lookat = { 0.0f, 0.0f, 0.0f };
	GW::MATH::GVECTORF up = { 0.0f, 1.0f, 0.0f };
	GW::MATH::GMatrix::TranslateGlobalF(initialCamera, translate, initialCamera);
	GW::MATH::GMatrix::LookAtLHF(translate, lookat, up, initialCamera);
	// Inverse to turn it into a camera matrix, not a view matrix. This will let us do
	// camera manipulation in the component easier
	GW::MATH::GMatrix::InverseF(initialCamera, initialCamera);
	registry.emplace<DRAW::Camera>(camera,
		DRAW::Camera{ initialCamera });
}

// This function will be called by the main loop to update the gameplay
// It will be responsible for updating the VulkanInstances and any other gameplay components
void GameplayBehavior(entt::registry& registry)
{
	std::shared_ptr<const GameConfig> config = registry.get<UTIL::Config>(
		registry.view<UTIL::Config>().front()).gameConfig;

	// Create the input objects
	auto input = registry.create();
	registry.emplace<UTIL::Input>(input);
	
	// Seed the rand
	unsigned int time = std::chrono::steady_clock::now().time_since_epoch().count();
	srand(time);

	//Lab 2 part 2
	//create player and enemy entities
	auto gameManagerID = registry.create();
	registry.emplace<GAME::GameManager>(gameManagerID);

	auto playerID = registry.create();
	auto enemyID = registry.create();
	
	auto& playerIDMeshCollection = registry.emplace<DRAW::MESH_COLLECTION>(playerID);
	auto& playerTrans = registry.emplace<GAME::Transform>(playerID);
	//auto& playerHealth = registry.emplace<GAME::Health>(playerID);
	registry.emplace<GAME::Player>(playerID);
	registry.emplace<GAME::Collidable>(playerID);
	auto& enemyIDMeshCollection = registry.emplace<DRAW::MESH_COLLECTION>(enemyID);
	auto& enemyTrans = registry.emplace<GAME::Transform>(enemyID);
	auto& enemyHealthRef = registry.emplace<GAME::Health>(enemyID);
	auto& enemyShatterRef = registry.emplace<GAME::Shatters>(enemyID);
	auto& enemyVelo = registry.emplace<GAME::Velocity>(enemyID,
		GW::MATH::GVECTORF{0.0f, 0.0f, 0.0f, 0.0f});
	registry.emplace<GAME::Enemy>(enemyID);
	registry.emplace<GAME::Collidable>(enemyID);

	std::string playerModel = (*config).at("Player").at("model").as<std::string>();
	std::string enemyModel = (*config).at("Enemy1").at("model").as<std::string>();
	int enemyHealth = (*config).at("Enemy1").at("hitpoints").as<int>();
	float enemySpeed = (*config).at("Enemy1").at("speed").as<float>();
	float newEnemyScale = (*config).at("Enemy1").at("scaleBy").as<float>();
	//int enemyShatterAmount = (*config).at("Enemy1").at("shatterAmount").as<int>();
	int enemyShatterCount = (*config).at("Enemy1").at("initialShatterCount").as<int>();

	enemyHealthRef.healthAmount = enemyHealth;
	enemyShatterRef.shatterCount = enemyShatterCount;
	enemyShatterRef.scaleBy = newEnemyScale;

	GW::MATH::GVECTORF normalizedVec = UTIL::GetRandomVelocityVector();
	GW::MATH::GVector::ScaleF(normalizedVec, enemySpeed, enemyVelo.direction);

	//auto& playerManager = registry.get<DRAW::ModelManager>(playerID);
	auto& modelManager = registry.get<DRAW::ModelManager>(registry.view<DRAW::ModelManager>().front());

	if (modelManager.MeshCollections.find(playerModel) != modelManager.MeshCollections.end()) {
		for (int i = 0; i < modelManager.MeshCollections[playerModel].dynamicEntities.size(); ++i) {
			//grab reference
			entt::entity currEnt = modelManager.MeshCollections[playerModel].dynamicEntities[i];
			GW::MATH::GOBBF currColl = modelManager.MeshCollections[playerModel].collider;
			DRAW::GPUInstance currGPU = registry.get<DRAW::GPUInstance>(currEnt);
			DRAW::GeometryData currGeo = registry.get<DRAW::GeometryData>(currEnt);
			GW::MATH::GMATRIXF currTransform = currGPU.transform;

			playerTrans.transform = currTransform;

			//for each mesh we will create a copy onto a new entity and add it to the players meshcollection
			auto playerCopy = registry.create();

			registry.emplace<DRAW::GPUInstance>(playerCopy,
												playerTrans.transform,
												currGPU.matData);

			registry.emplace<DRAW::GeometryData>(playerCopy,
												currGeo.indexStart,
												currGeo.indexCount,
												currGeo.vertexStart);

			playerIDMeshCollection.dynamicEntities.push_back(playerCopy);
			playerIDMeshCollection.collider = currColl;
		}
	}

	if (modelManager.MeshCollections.find(enemyModel) != modelManager.MeshCollections.end()) {
		for (int i = 0; i < modelManager.MeshCollections[enemyModel].dynamicEntities.size(); ++i) {
			//grab reference
			entt::entity currEnt = modelManager.MeshCollections[enemyModel].dynamicEntities[i];
			GW::MATH::GOBBF currColl = modelManager.MeshCollections[enemyModel].collider;
			DRAW::GPUInstance currGPU = registry.get<DRAW::GPUInstance>(currEnt);
			DRAW::GeometryData currGeo = registry.get<DRAW::GeometryData>(currEnt);
			GW::MATH::GMATRIXF currTransform = currGPU.transform;

			//more refs for getting the collider
			/*auto& currBlenderObj = gpuLevelData.loadedData.blenderObjects[i];
			auto& currModel = gpuLevelData.loadedData.levelModels[currBlenderObj.modelIndex];*/

			enemyTrans.transform = currTransform;

			//for each mesh we will create a copy onto a new entity and add it to the players meshcollection
			auto enemyCopy = registry.create();

			registry.emplace<DRAW::GPUInstance>(enemyCopy,
												enemyTrans.transform,
												currGPU.matData);

			registry.emplace<DRAW::GeometryData>(enemyCopy,
												currGeo.indexStart,
												currGeo.indexCount,
												currGeo.vertexStart);

			enemyIDMeshCollection.dynamicEntities.push_back(enemyCopy);
			enemyIDMeshCollection.collider = currColl;
		}
	}
}

// This function will be called by the main loop to update the main loop
// It will be responsible for updating any created windows and handling any input
void MainLoopBehavior(entt::registry& registry)
{	
	// main loop
	int closedCount; // count of closed windows
	auto winView = registry.view<APP::Window>(); // for updating all windows
	auto& deltaTime = registry.emplace<UTIL::DeltaTime>(registry.create()).dtSec;
	// for updating all windows
	do {
		static auto start = std::chrono::steady_clock::now();
		double elapsed = std::chrono::duration<double>(
			std::chrono::steady_clock::now() - start).count();
		start = std::chrono::steady_clock::now();
		deltaTime = elapsed;

		// TODO : Update Game
		registry.patch<GAME::GameManager>(registry.view<GAME::GameManager>().front());

		closedCount = 0;
		// find all Windows that are not closed and call "patch" to update them
		for (auto entity : winView) {
			if (registry.any_of<APP::WindowClosed>(entity))
				++closedCount;
			else
				registry.patch<APP::Window>(entity); // calls on_update()
		}
	} while (winView.size() != closedCount); // exit when all windows are closed
}
