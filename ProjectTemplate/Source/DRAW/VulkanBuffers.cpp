#include "DrawComponents.h"
#include "../CCL.h"
namespace DRAW
{
	//*** HELPERS ***//
	void AddSceneData(entt::registry& registry, entt::entity entity)
	{
		auto& sceneData = registry.emplace<SceneData>(entity,
			SceneData{ { -1.0f, -1.0f, 2.0f }, { 0.9f, 0.9f, 0.9f }, { 0.2f, 0.2f, 0.2f } });

		GW::MATH::GVector::NormalizeF(sceneData.sunDirection, sceneData.sunDirection);
		auto& renderer = registry.get<VulkanRenderer>(entity);
		sceneData.projectionMatrix = renderer.projMatrix;
	}

	//*** SYSTEMS ***//
	// Forward Declare
	void Destroy_VulkanVertexBuffer(entt::registry& registry, entt::entity entity);

	void Update_VulkanVertexBuffer(entt::registry& registry, entt::entity entity) {
		auto& gpuBuffer = registry.get<VulkanVertexBuffer>(entity);
		// upload the buffer to the GPU
		if (registry.all_of<VulkanRenderer, std::vector<H2B::VERTEX>>(entity)) {
			// if there is already a gpu buffer attached, lets delete it
			if (gpuBuffer.buffer != VK_NULL_HANDLE)
				Destroy_VulkanVertexBuffer(registry, entity);
			// if there is a cpu buffer attached, lets upload it to the GPU then delete it
			auto& vkRenderer = registry.get<VulkanRenderer>(entity);
			auto& cpuBuffer = registry.get<std::vector<H2B::VERTEX>>(entity);
			// Transfer triangle data to the vertex buffer. (staging would be preferred here)
			GvkHelper::create_buffer(vkRenderer.physicalDevice, vkRenderer.device, sizeof(H2B::VERTEX) * cpuBuffer.size(),
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &gpuBuffer.buffer, &gpuBuffer.memory);
			GvkHelper::write_to_buffer(vkRenderer.device, gpuBuffer.memory,
				cpuBuffer.data(), sizeof(H2B::VERTEX) * cpuBuffer.size());
			// remove the CPU buffer
			vkDeviceWaitIdle(vkRenderer.device);
			registry.remove<std::vector<H2B::VERTEX>>(entity);
		}
	}

	void Destroy_VulkanVertexBuffer(entt::registry& registry, entt::entity entity) {
		// check if the buffer is allocated, if so, release it
		if (registry.all_of<VulkanVertexBuffer, VulkanRenderer>(entity)) {

			auto& vkRenderer = registry.get<VulkanRenderer>(entity);
			auto& gpuBuffer = registry.get<VulkanVertexBuffer>(entity);
			vkDeviceWaitIdle(vkRenderer.device);
			// Release allocated buffers, shaders & pipeline
			vkDestroyBuffer(vkRenderer.device, gpuBuffer.buffer, nullptr);
			vkFreeMemory(vkRenderer.device, gpuBuffer.memory, nullptr);
		}

	}

	// Forward declare
	void Destroy_VulkanIndexBuffer(entt::registry& registry, entt::entity entity);

	void Update_VulkanIndexBuffer(entt::registry& registry, entt::entity entity) {
		auto& gpuBuffer = registry.get<VulkanIndexBuffer>(entity);
		// upload the buffer to the GPU
		if (registry.all_of<VulkanRenderer, std::vector<unsigned int>>(entity)) {
			// if there is already a gpu buffer attached, lets delete it
			if (gpuBuffer.buffer != VK_NULL_HANDLE)
				Destroy_VulkanIndexBuffer(registry, entity);
			// if there is a cpu buffer attached, lets upload it to the GPU then delete it
			auto& vkRenderer = registry.get<VulkanRenderer>(entity);
			auto& cpuBuffer = registry.get<std::vector<unsigned int>>(entity);

			// Transfer triangle data to the vertex buffer. (staging would be preferred here)
			GvkHelper::create_buffer(vkRenderer.physicalDevice, vkRenderer.device, sizeof(unsigned int) * cpuBuffer.size(),
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &gpuBuffer.buffer, &gpuBuffer.memory);
			GvkHelper::write_to_buffer(vkRenderer.device, gpuBuffer.memory,
				cpuBuffer.data(), sizeof(unsigned int) * cpuBuffer.size());
			// remove the CPU buffer
			vkDeviceWaitIdle(vkRenderer.device);
			registry.remove<std::vector<unsigned int>>(entity);
		}
	}

	void Destroy_VulkanIndexBuffer(entt::registry& registry, entt::entity entity) {
		// check if the buffer is allocated, if so, release it
		if (registry.all_of<VulkanIndexBuffer, VulkanRenderer>(entity)) {

			auto& vkRenderer = registry.get<VulkanRenderer>(entity);
			auto& gpuBuffer = registry.get<VulkanIndexBuffer>(entity);
			
			vkDeviceWaitIdle(vkRenderer.device);
			// Release allocated buffers, shaders & pipeline
			vkDestroyBuffer(vkRenderer.device, gpuBuffer.buffer, nullptr);
			vkFreeMemory(vkRenderer.device, gpuBuffer.memory, nullptr);
		}
	}

	void Construct_VulkanGPUInstanceBuffer(entt::registry& registry, entt::entity entity) {
		auto& bufferComponent = registry.get<VulkanGPUInstanceBuffer>(entity);
		auto& renderer = registry.get<VulkanRenderer>(entity);

		unsigned int frameCount;
		renderer.vlkSurface.GetSwapchainImageCount(frameCount);
		bufferComponent.memory.resize(frameCount);
		bufferComponent.buffer.resize(frameCount);

		for (unsigned int i = 0; i < frameCount; i++)
		{
			GvkHelper::create_buffer(renderer.physicalDevice, renderer.device, sizeof(GPUInstance)*bufferComponent.element_count,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &bufferComponent.buffer[i], &bufferComponent.memory[i]);
		}
		
	}
		
	// Forward declare
	void Destroy_VulkanGPUInstanceBuffer(entt::registry& registry, entt::entity entity);

	void Update_VulkanGPUInstanceBuffer(entt::registry& registry, entt::entity entity) {
		if (!registry.all_of<std::vector<GPUInstance>>(entity))
			return; // No Instances, so nothing to write. Bail

		auto& gpuBuffer = registry.get<VulkanGPUInstanceBuffer>(entity);		
		auto& instances = registry.get<std::vector<GPUInstance>>(entity);
		auto& renderer = registry.get<VulkanRenderer>(entity);

		// Resize buffer if needed
		if (instances.size() > gpuBuffer.element_count)
		{			
			Destroy_VulkanGPUInstanceBuffer(registry, entity);

			gpuBuffer.element_count *= 2; // Double the storage size if we ran out
			
			for (unsigned int i = 0; i < gpuBuffer.buffer.size(); i++)
			{
				GvkHelper::create_buffer(renderer.physicalDevice, renderer.device, sizeof(GPUInstance) * gpuBuffer.element_count,
					VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &gpuBuffer.buffer[i], &gpuBuffer.memory[i]);

				VkDescriptorBufferInfo storageBufferInfo = { gpuBuffer.buffer[i], 0, VK_WHOLE_SIZE };
				VkWriteDescriptorSet storageWrite = {};
				storageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				storageWrite.dstSet = renderer.descriptorSets[i];
				storageWrite.dstBinding = 1; // 1 For the storage buffer
				storageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				storageWrite.descriptorCount = 1;
				storageWrite.pBufferInfo = &storageBufferInfo;

				vkUpdateDescriptorSets(renderer.device, 1, &storageWrite, 0, nullptr);
			}
		}		

		if (instances.size() > 0)
		{
			unsigned int sizeInBytes = sizeof(GPUInstance) * instances.size();
			unsigned int frame;
			renderer.vlkSurface.GetSwapchainCurrentImage(frame);
			vkDeviceWaitIdle(renderer.device);
			GvkHelper::write_to_buffer(renderer.device, gpuBuffer.memory[frame], instances.data(), sizeInBytes);
		}

		vkDeviceWaitIdle(renderer.device);
		registry.remove<std::vector<GPUInstance>>(entity);
	}

	void Destroy_VulkanGPUInstanceBuffer(entt::registry& registry, entt::entity entity) {
		auto& gpuBuffer = registry.get<VulkanGPUInstanceBuffer>(entity);
		auto& renderer = registry.get<VulkanRenderer>(entity);

		vkDeviceWaitIdle(renderer.device);
		for (auto handle : gpuBuffer.buffer)
		{
			vkDestroyBuffer(renderer.device, handle, nullptr);
		}
		for (auto data : gpuBuffer.memory)
		{
			vkFreeMemory(renderer.device, data, nullptr);
		}
	}

	void Construct_VulkanUniformBuffer(entt::registry& registry, entt::entity entity) {

		auto& bufferComponent = registry.get<VulkanUniformBuffer>(entity);
		auto& renderer = registry.get<VulkanRenderer>(entity);

		// If there isn't a SceneData, add one
		if (!registry.all_of<SceneData>(entity))
		{
			AddSceneData(registry, entity);
		}

		unsigned int frameCount;
		renderer.vlkSurface.GetSwapchainImageCount(frameCount);
		bufferComponent.memory.resize(frameCount);
		bufferComponent.buffer.resize(frameCount);

		for (unsigned int i = 0; i < frameCount; i++)
		{
			GvkHelper::create_buffer(renderer.physicalDevice, renderer.device, sizeof(SceneData),
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &bufferComponent.buffer[i], &bufferComponent.memory[i]);
		}
	}

	void Update_VulkanUniformBuffer(entt::registry& registry, entt::entity entity) {
		auto& gpuBuffer = registry.get<VulkanUniformBuffer>(entity);
		auto& renderer = registry.get<VulkanRenderer>(entity);
		auto& data = registry.get<SceneData>(entity);

		// TODO : Update Dynamic parts of the Scene Data here
		// We really only support one camera, so use the first one
		auto& camera = registry.get<Camera>(registry.view<Camera>().front());

		data.camPos = camera.camMatrix.row4;
		GW::MATH::GMatrix::InverseF(camera.camMatrix, data.viewMatrix);

		unsigned int frame;
		renderer.vlkSurface.GetSwapchainCurrentImage(frame);
		GvkHelper::write_to_buffer(renderer.device, gpuBuffer.memory[frame], &data, sizeof(SceneData));
	}

	void Destroy_VulkanUniformBuffer(entt::registry& registry, entt::entity entity) {
		auto& gpuBuffer = registry.get<VulkanUniformBuffer>(entity);
		auto& renderer = registry.get<VulkanRenderer>(entity);

		for (auto handle : gpuBuffer.buffer)
		{
			vkDestroyBuffer(renderer.device, handle, nullptr);
		}
		for (auto data : gpuBuffer.memory)
		{
			vkFreeMemory(renderer.device, data, nullptr);
		}
	}

	void Construct_CPULevel(entt::registry& registry, entt::entity entity) {
		auto& cpuLevelData = registry.get<CPULevel>(entity);
		GW::SYSTEM::GLog log;
		log.Create("CPULevel log");
		log.EnableConsoleLogging(true);

		bool result = cpuLevelData.loadedData.LoadLevel(cpuLevelData.jsonPath.c_str(), cpuLevelData.modelPath.c_str(), log);
	}
	
	void Construct_GPULevel(entt::registry& registry, entt::entity entity) {
		CPULevel* gpuLevelData = registry.try_get<CPULevel>(entity);

		if (gpuLevelData == nullptr)
			return;

		std::cout << "PASSED\n";

		//emplace the buffer components
		registry.emplace<DRAW::VulkanVertexBuffer>(entity);
		registry.emplace<DRAW::VulkanIndexBuffer>(entity);

		//emplace the data for the buffers
		registry.emplace<std::vector<H2B::VERTEX>>(entity, gpuLevelData->loadedData.levelVertices);
		registry.emplace<std::vector<unsigned>>(entity, gpuLevelData->loadedData.levelIndices);

		//call the patch() to update the data onto the GPU
		registry.patch<DRAW::VulkanVertexBuffer>(entity);
		registry.patch<DRAW::VulkanIndexBuffer>(entity);

		std::cout << "buffers are updated\n";

		//Lab 2 part 1C, creating new entity to emplace ModelManager
		auto entityID_ModelManager = registry.create();
		registry.emplace<ModelManager>(entityID_ModelManager);

		//get map MeshCollections
		auto& modelManager = registry.get<ModelManager>(entityID_ModelManager);

		//getting rendering data
		for (int i = 0; i < gpuLevelData->loadedData.blenderObjects.size(); ++i) {
			auto& currBlenderObj = gpuLevelData->loadedData.blenderObjects[i];
			auto& currModel = gpuLevelData->loadedData.levelModels[currBlenderObj.modelIndex];

			//Lab 2 part 1C, for each obj start a new MeshCollection
			MESH_COLLECTION meshCollection;

			//check loadedData.levelModels to see if isDynamic
			//if yes, emplace the DoNotRender tag onto entity
			bool dynamic = false;
			if (currModel.isDynamic) {
				dynamic = true;
			}

			for (int j = 0; j < currModel.meshCount; ++j) {
				auto& currMesh = gpuLevelData->loadedData.levelMeshes[currModel.meshStart + j];

				auto entityID = registry.create();

				if (dynamic) {
					registry.emplace<DRAW::DoNotRender>(entityID);

					//if dynamic, add entity to meshCollections
					meshCollection.dynamicEntities.push_back(entityID);
				}

				registry.emplace<DRAW::GeometryData>(entityID, 
					currModel.indexStart,
					currMesh.drawInfo.indexCount,
					currModel.vertexStart);

				registry.emplace<DRAW::GPUInstance>(entityID,
					gpuLevelData->loadedData.levelTransforms[currBlenderObj.transformIndex],
					gpuLevelData->loadedData.levelMaterials[(currMesh.materialIndex + currModel.materialStart)].attrib);
			}

			//once all meshes are created add collection to modelManager using blendername as Key
			//only if dynamic
			if (dynamic) {
				modelManager.MeshCollections[currBlenderObj.blendername] = meshCollection;
			}
		}
	}

	// Use this MACRO to connect the EnTT Component Logic
	CONNECT_COMPONENT_LOGIC() {
		// register the Window component's logic
		registry.on_update<VulkanVertexBuffer>().connect<Update_VulkanVertexBuffer>();
		registry.on_destroy<VulkanVertexBuffer>().connect<Destroy_VulkanVertexBuffer>();

		registry.on_update<VulkanIndexBuffer>().connect<Update_VulkanIndexBuffer>();
		registry.on_destroy<VulkanIndexBuffer>().connect<Destroy_VulkanIndexBuffer>();

		registry.on_construct<VulkanGPUInstanceBuffer>().connect<Construct_VulkanGPUInstanceBuffer>();
		registry.on_update<VulkanGPUInstanceBuffer>().connect<Update_VulkanGPUInstanceBuffer>();
		registry.on_destroy<VulkanGPUInstanceBuffer>().connect<Destroy_VulkanGPUInstanceBuffer>();

		registry.on_construct<VulkanUniformBuffer>().connect<Construct_VulkanUniformBuffer>();
		registry.on_update<VulkanUniformBuffer>().connect<Update_VulkanUniformBuffer>();
		registry.on_destroy<VulkanUniformBuffer>().connect<Destroy_VulkanUniformBuffer>();

		// register the CPU and GPU level data
		registry.on_construct<CPULevel>().connect<Construct_CPULevel>();
		registry.on_construct<GPULevel>().connect<Construct_GPULevel>();
	}

} // namespace DRAW