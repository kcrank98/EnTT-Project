#include "GameComponents.h"
#include "../DRAW/DrawComponents.h"
#include "../CCL.h"

void Update_Velocity(entt::registry& registry) {
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

void BounceEnemy(GW::MATH::GVECTORF enemyTrans, GW::MATH::GVECTORF& enemyVelo, GW::MATH::GOBBF& collider) {
	//w = v - (2 * dot(v, n) * n)
	//v = currVelo
	//n = normal of surface youre reflecting across of

	//make our normal
	GW::MATH::GVECTORF point;
	GW::MATH::GCollision::ClosestPointToOBBF(collider, enemyTrans, point);
	point.y = 0.0f; //only 2D movement

	GW::MATH::GVECTORF normal;
	//to make a vector with 2 points, destination - source
	GW::MATH::GVector::SubtractVectorF(enemyTrans, point, normal);
	normal.y = 0.0f; normal.w = 0.0f; //only 2D movement
	GW::MATH::GVector::NormalizeF(normal, normal);

	//dot(v, n)
	GW::MATH::GVECTORF w;
	float dot;
	GW::MATH::GVector::DotF(enemyVelo, normal, dot);
	//2 * dot(v, n)
	dot *= 2.0f;
	//2 * dot(v, n) * n
	GW::MATH::GVector::ScaleF(normal, dot, normal);
	//v - (2 * dot(v, n) * n)
	GW::MATH::GVector::SubtractVectorF(enemyVelo, normal, w);

	enemyVelo = w;
}

void Update_Physics(entt::registry& registry, const double& deltaTime = 0) {
	//set up our view
	auto& colliderView = registry.view<GAME::Transform, DRAW::MESH_COLLECTION, GAME::Collidable>();

	//loop thru view
	for (auto iter = colliderView.begin(); iter != colliderView.end(); iter++) {
		//get copy of collider
		GW::MATH::GOBBF colA = registry.get<DRAW::MESH_COLLECTION>(*iter).collider;
		GW::MATH::GMATRIXF& transA = registry.get<GAME::Transform>(*iter).transform;

		//transform center into world space
		GW::MATH::GMatrix::VectorXMatrixF(transA, colA.center, colA.center);

		//scale our extents by the transform
		GW::MATH::GVECTORF scaleVecA;
		GW::MATH::GMatrix::GetScaleF(transA, scaleVecA);
		colA.extent.x *= scaleVecA.x; colA.extent.y *= scaleVecA.y; colA.extent.z *= scaleVecA.z;

		//rotate collider
		GW::MATH::GQUATERNIONF qA;
		GW::MATH::GQuaternion::SetByMatrixF(transA, qA);
		GW::MATH::GQuaternion::MultiplyQuaternionF(colA.rotation, qA, colA.rotation);

		//inner loop
		auto j = iter;
		for (j++; j != colliderView.end(); j++) {
			//get copy of collider
			GW::MATH::GOBBF colB = registry.get<DRAW::MESH_COLLECTION>(*j).collider;
			GW::MATH::GMATRIXF& transB = registry.get<GAME::Transform>(*j).transform;

			//transform center into world space
			GW::MATH::GMatrix::VectorXMatrixF(transB, colB.center, colB.center);

			//scale our extents by the transform
			GW::MATH::GVECTORF scaleVecB;
			GW::MATH::GMatrix::GetScaleF(transB, scaleVecB);
			colB.extent.x *= scaleVecB.x; colB.extent.y *= scaleVecB.y; colB.extent.z *= scaleVecB.z;

			//rotate collider
			GW::MATH::GQUATERNIONF qB;
			GW::MATH::GQuaternion::SetByMatrixF(transB, qB);
			GW::MATH::GQuaternion::MultiplyQuaternionF(colB.rotation, qB, colB.rotation);

			//check collision
			GW::MATH::GCollision::GCollisionCheck result = GW::MATH::GCollision::GCollisionCheck::NO_COLLISION;
			GW::MATH::GCollision::TestOBBToOBBF(colA, colB, result);
			if (result == GW::MATH::GCollision::GCollisionCheck::COLLISION) {
				if (registry.all_of<GAME::Bullet>(*iter) && registry.all_of<GAME::Obstacle>(*j)) {
					registry.destroy(*iter);
				}
				if (registry.all_of<GAME::Bullet>(*j) && registry.all_of<GAME::Obstacle>(*iter)) {
					registry.destroy(*j);
				}
				if (registry.all_of<GAME::Enemy>(*iter) && registry.all_of<GAME::Obstacle>(*j)) {
					//reflect across normal
					GW::MATH::GVECTORF& enemyVelo = registry.get<GAME::Velocity>(*iter).direction;
					GW::MATH::GVECTORF enemyLocation = transA.row4;

					BounceEnemy(enemyLocation, enemyVelo, colB);
				}
				if (registry.all_of<GAME::Enemy>(*j) && registry.all_of<GAME::Obstacle>(*iter)) {
					//reflect across normal
					GW::MATH::GVECTORF& enemyVelo = registry.get<GAME::Velocity>(*j).direction;
					GW::MATH::GVECTORF enemyLocation = transB.row4;

					BounceEnemy(enemyLocation, enemyVelo, colA);
				}
				if (registry.all_of<GAME::Bullet>(*iter) && registry.all_of<GAME::Enemy>(*j)) {
					registry.destroy(*iter);
					
					GAME::Health* enemyHP = registry.try_get<GAME::Health>(*j);
					if (enemyHP != nullptr) {
						enemyHP->healthAmount -= 1;
					}
					int debug = 0;
				}
				if (registry.all_of<GAME::Bullet>(*j) && registry.all_of<GAME::Enemy>(*iter)) {
					registry.destroy(*j);

					GAME::Health* enemyHP = registry.try_get<GAME::Health>(*iter);
					if (enemyHP != nullptr) {
						enemyHP->healthAmount -= 1;
					}
					int debug = 0;
				}
			}
		}
	}
}

void CreateEnemy(entt::registry& registry, GAME::Shatters* shatterComponent, GW::MATH::GMATRIXF enemyLocation) {
	auto& modelManager = registry.get<DRAW::ModelManager>(registry.view<DRAW::ModelManager>().front());
	std::shared_ptr<const GameConfig> config = registry.get<UTIL::Config>(
		registry.view<UTIL::Config>().front()).gameConfig;

	auto enemyID = registry.create();

	std::string enemyModel = (*config).at("Enemy1").at("model").as<std::string>();
	int enemyHealth = (*config).at("Enemy1").at("hitpoints").as<int>();
	float enemySpeed = (*config).at("Enemy1").at("speed").as<float>();
	float newEnemyScale = (*config).at("Enemy1").at("scaleBy").as<float>();

	auto& enemyIDMeshCollection = registry.emplace<DRAW::MESH_COLLECTION>(enemyID);
	auto& enemyTrans = registry.emplace<GAME::Transform>(enemyID);
	auto& enemyHealthRef = registry.emplace<GAME::Health>(enemyID);

	if (shatterComponent->shatterCount != 0) {
		auto& enemyShatterRef = registry.emplace<GAME::Shatters>(enemyID);
		enemyShatterRef.shatterCount = shatterComponent->shatterCount - 1;
		enemyShatterRef.scaleBy = newEnemyScale;
	}
	enemyHealthRef.healthAmount = enemyHealth;

	auto& enemyVelo = registry.emplace<GAME::Velocity>(enemyID,
		GW::MATH::GVECTORF{0.0f, 0.0f, 0.0f, 0.0f});
	registry.emplace<GAME::Enemy>(enemyID);
	registry.emplace<GAME::Collidable>(enemyID);


	GW::MATH::GVECTORF normalizedVec = UTIL::GetRandomVelocityVector();
	GW::MATH::GVector::ScaleF(normalizedVec, enemySpeed, enemyVelo.direction);

	if (modelManager.MeshCollections.find(enemyModel) != modelManager.MeshCollections.end()) {
		for (int i = 0; i < modelManager.MeshCollections[enemyModel].dynamicEntities.size(); ++i) {
			//grab reference
			entt::entity currEnt = modelManager.MeshCollections[enemyModel].dynamicEntities[i];
			GW::MATH::GOBBF currColl = modelManager.MeshCollections[enemyModel].collider;
			DRAW::GPUInstance currGPU = registry.get<DRAW::GPUInstance>(currEnt);
			DRAW::GeometryData currGeo = registry.get<DRAW::GeometryData>(currEnt);
			GW::MATH::GMATRIXF currTransform = currGPU.transform;

			GW::MATH::GMatrix::ScaleLocalF(enemyLocation, 
				GW::MATH::GVECTORF{newEnemyScale, newEnemyScale, newEnemyScale}, 
				enemyLocation);

			//enemyTrans.transform = currTransform;
			enemyTrans.transform = enemyLocation;

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

void CheckHealth(entt::registry& registry) {
	//make a view of everything with the Health component (should just be the enemies!)
	entt::basic_view view = registry.view<GAME::Health>();
	std::shared_ptr<const GameConfig> config = registry.get<UTIL::Config>(
		registry.view<UTIL::Config>().front()).gameConfig;

	int enemyShatterAmount = (*config).at("Enemy1").at("shatterAmount").as<int>();

	//loop though view
	for (auto [ent, hp] : view.each()) {
		//check if health is 0 or less
		if (hp.healthAmount <= 0) {
			//check if also shatters
			GAME::Shatters* enemyShatter = registry.try_get<GAME::Shatters>(ent);

			if (enemyShatter != nullptr) {
				GW::MATH::GMATRIXF currLocation = registry.get<GAME::Transform>(ent).transform;

				for (int i = 0; i < enemyShatterAmount; ++i) {
					CreateEnemy(registry, enemyShatter, currLocation);
				}
			}
			registry.destroy(ent);
		}
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

	Update_Physics(registry);

	//check health here
	CheckHealth(registry);
}

CONNECT_COMPONENT_LOGIC() {
	registry.on_update<GAME::GameManager>().connect<Update_GameManager>();
}