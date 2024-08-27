#include "GameComponents.h"
#include "../DRAW/DrawComponents.h"
#include "../UTIL/Utilities.h"
#include "../CCL.h"

void CreateBullet(entt::registry& registry, std::shared_ptr<const GameConfig>& config, GAME::Transform& playerTransform,
	float inputUp, float inputDown, float inputLeft, float inputRight) {
	auto bulletID = registry.create();

	auto& bulletIDMeshCollection = registry.emplace<DRAW::MESH_COLLECTION>(bulletID);
	auto& bulletTrans = registry.emplace<GAME::Transform>(bulletID);
	auto& bulletVelo = registry.emplace<GAME::Velocity>(bulletID,
		GW::MATH::GVECTORF{0.0f, 0.0f, 0.0f, 0.0f});
	registry.emplace<GAME::Bullet>(bulletID);
	registry.emplace<GAME::Collidable>(bulletID);

	std::string bulletModel = (*config).at("Bullet").at("model").as<std::string>();
	float bulletSpeed = (*config).at("Bullet").at("speed").as<float>();

	//float Z_Change = inputUp - inputDown;
	//float X_Change = inputLeft - inputRight;

	//if (Z_Change && X_Change != 0) {
	//	bulletVelo.direction = { X_Change, 0.0f, Z_Change, 0.0f };
	//	//GW::MATH::GVector::NormalizeF(bulletVelo.direction, bulletVelo.direction);
	//	GW::MATH::GVector::ScaleF(bulletVelo.direction, bulletSpeed, bulletVelo.direction);
	//}

	if (inputUp == 1) {
		if (inputLeft == 1) {
			bulletVelo.direction = { -1.0f, 0.0f, 1.0f, 0.0f };
			GW::MATH::GVector::NormalizeF(bulletVelo.direction, bulletVelo.direction);
			GW::MATH::GVector::ScaleF(bulletVelo.direction, bulletSpeed, bulletVelo.direction);
		}
		else if (inputRight == 1) {
			bulletVelo.direction = { 1.0f, 0.0f, 1.0f, 0.0f };
			GW::MATH::GVector::NormalizeF(bulletVelo.direction, bulletVelo.direction);
			GW::MATH::GVector::ScaleF(bulletVelo.direction, bulletSpeed, bulletVelo.direction);
		}
		else {
			bulletVelo.direction = { 0.0f, 0.0f, 1.0f, 0.0f };
			//GW::MATH::GVector::NormalizeF(bulletVelo.direction, bulletVelo.direction);
			GW::MATH::GVector::ScaleF(bulletVelo.direction, bulletSpeed, bulletVelo.direction);
		}
	}
	else if (inputDown == 1) {
		if (inputLeft == 1) {
			bulletVelo.direction = { -1.0f, 0.0f, -1.0f, 0.0f };
			GW::MATH::GVector::NormalizeF(bulletVelo.direction, bulletVelo.direction);
			GW::MATH::GVector::ScaleF(bulletVelo.direction, bulletSpeed, bulletVelo.direction);
		}
		else if (inputRight == 1) {
			bulletVelo.direction = { 1.0f, 0.0f, -1.0f, 0.0f };
			GW::MATH::GVector::NormalizeF(bulletVelo.direction, bulletVelo.direction);
			GW::MATH::GVector::ScaleF(bulletVelo.direction, bulletSpeed, bulletVelo.direction);
		}
		else {
			bulletVelo.direction = { 0.0f, 0.0f, -1.0f, 0.0f };
			GW::MATH::GVector::NormalizeF(bulletVelo.direction, bulletVelo.direction);
			GW::MATH::GVector::ScaleF(bulletVelo.direction, bulletSpeed, bulletVelo.direction);
		}
	}
	else if (inputLeft == 1) {
		if (inputUp == 1) {
			bulletVelo.direction = { -1.0f, 0.0f, 1.0f, 0.0f };
			GW::MATH::GVector::NormalizeF(bulletVelo.direction, bulletVelo.direction);
			GW::MATH::GVector::ScaleF(bulletVelo.direction, bulletSpeed, bulletVelo.direction);
		}
		else if (inputDown == 1) {
			bulletVelo.direction = { -1.0f, 0.0f, -1.0f, 0.0f };
			GW::MATH::GVector::NormalizeF(bulletVelo.direction, bulletVelo.direction);
			GW::MATH::GVector::ScaleF(bulletVelo.direction, bulletSpeed, bulletVelo.direction);
		}
		else {
			bulletVelo.direction = { -1.0f, 0.0f, 0.0f, 0.0f };
			GW::MATH::GVector::NormalizeF(bulletVelo.direction, bulletVelo.direction);
			GW::MATH::GVector::ScaleF(bulletVelo.direction, bulletSpeed, bulletVelo.direction);
		}
	}
	else if (inputRight == 1) {
		if (inputUp == 1) {
			bulletVelo.direction = { 1.0f, 0.0f, 1.0f, 0.0f };
			GW::MATH::GVector::NormalizeF(bulletVelo.direction, bulletVelo.direction);
			GW::MATH::GVector::ScaleF(bulletVelo.direction, bulletSpeed, bulletVelo.direction);
		}
		else if (inputDown == 1) {
			bulletVelo.direction = { 1.0f, 0.0f, -1.0f, 0.0f };
			GW::MATH::GVector::NormalizeF(bulletVelo.direction, bulletVelo.direction);
			GW::MATH::GVector::ScaleF(bulletVelo.direction, bulletSpeed, bulletVelo.direction);
		}
		else {
			bulletVelo.direction = { 1.0f, 0.0f, 0.0f, 0.0f };
			GW::MATH::GVector::NormalizeF(bulletVelo.direction, bulletVelo.direction);
			GW::MATH::GVector::ScaleF(bulletVelo.direction, bulletSpeed, bulletVelo.direction);
		}
	}

	//auto& playerManager = registry.get<DRAW::ModelManager>(playerID);
	auto& modelManager = registry.get<DRAW::ModelManager>(registry.view<DRAW::ModelManager>().front());

	if (modelManager.MeshCollections.find(bulletModel) != modelManager.MeshCollections.end()) {
		for (int i = 0; i < modelManager.MeshCollections[bulletModel].dynamicEntities.size(); ++i) {
			//grab reference
			entt::entity currEnt = modelManager.MeshCollections[bulletModel].dynamicEntities[i];
			GW::MATH::GOBBF currColl = modelManager.MeshCollections[bulletModel].collider;
			DRAW::GPUInstance currGPU = registry.get<DRAW::GPUInstance>(currEnt);
			DRAW::GeometryData currGeo = registry.get<DRAW::GeometryData>(currEnt);
			//GW::MATH::GMATRIXF currTransform = currGPU.transform;
			bulletTrans.transform = playerTransform.transform;

			//playerTrans.transform = currTransform;

			//for each mesh we will create a copy onto a new entity and add it to the players meshcollection
			auto bulletCopy = registry.create();

			registry.emplace<DRAW::GPUInstance>(bulletCopy,
												bulletTrans.transform,
												currGPU.matData);

			registry.emplace<DRAW::GeometryData>(bulletCopy,
												currGeo.indexStart,
												currGeo.indexCount,
												currGeo.vertexStart);

			bulletIDMeshCollection.dynamicEntities.push_back(bulletCopy);
			bulletIDMeshCollection.collider = currColl;
		}
	}
}

void Update_Player(entt::registry& registry, entt::entity entity) {
	std::shared_ptr<const GameConfig> config = registry.get<UTIL::Config>(
		registry.view<UTIL::Config>().front()).gameConfig;

	auto& input = registry.get<UTIL::Input>(registry.view<UTIL::Input>().front());
	auto& deltaTime = registry.get<UTIL::DeltaTime>(registry.view<UTIL::DeltaTime>().front()).dtSec;

	static auto start = std::chrono::steady_clock::now();
	double elapsed = std::chrono::duration<double>(
		std::chrono::steady_clock::now() - start).count();
	start = std::chrono::steady_clock::now();
	deltaTime = elapsed;
	
	float playerSpeed = (*config).at("Player").at("speed").as<float>();
	float cooldown = (*config).at("Player").at("firerate").as<float>();

	float playerElapsed = playerSpeed * deltaTime;
	float isPressed[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	//cooldown *= deltaTime;

	input.immediateInput.GetState(G_KEY_W, isPressed[0]);
	input.immediateInput.GetState(G_KEY_S, isPressed[1]);
	input.immediateInput.GetState(G_KEY_A, isPressed[2]);
	input.immediateInput.GetState(G_KEY_D, isPressed[3]);

	float Z_Change = isPressed[0] - isPressed[1];
	float X_Change = isPressed[2] - isPressed[3];

	GAME::Transform& playerTrans = registry.get<GAME::Transform>(entity);
	GAME::Transform enemyTran = playerTrans;

	GAME::FiringState* hasFired = registry.try_get<GAME::FiringState>(entity);
	if (hasFired == nullptr) {
		input.immediateInput.GetState(G_KEY_UP, isPressed[4]);
		input.immediateInput.GetState(G_KEY_DOWN, isPressed[5]);
		input.immediateInput.GetState(G_KEY_LEFT, isPressed[6]);
		input.immediateInput.GetState(G_KEY_RIGHT, isPressed[7]);

		if (isPressed[4] == 1 || isPressed[5] == 1 || isPressed[6] == 1 || isPressed[7] == 1) {
			CreateBullet(registry, config, enemyTran, isPressed[4], isPressed[5], isPressed[6], isPressed[7]);
			registry.emplace<GAME::FiringState>(entity, cooldown);
		}
	}
	else {
		hasFired->coolDown -= deltaTime;
		if (hasFired->coolDown <= 0) {
			registry.remove<GAME::FiringState>(entity);
		}
	}

	GW::MATH::GVECTORF playerSpeedNormalized = { (-X_Change), 0, Z_Change, 0 };
	GW::MATH::GVector::NormalizeF(playerSpeedNormalized, playerSpeedNormalized);
	GW::MATH::GVector::ScaleF(playerSpeedNormalized, playerElapsed, playerSpeedNormalized);
	GW::MATH::GMatrix::TranslateLocalF(playerTrans.transform,
									playerSpeedNormalized,
									playerTrans.transform);

	//reset input states
	for (auto& state : isPressed) {
		state = 0.0f;
	}
}


CONNECT_COMPONENT_LOGIC() {
	registry.on_update<GAME::Player>().connect<Update_Player>();
}