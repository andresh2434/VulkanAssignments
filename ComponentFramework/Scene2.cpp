#include <glew.h>
#include <iostream>
#include "Debug.h"
#include "Scene2.h"
#include <MMath.h>
#include "Debug.h"
#include "VulkanRenderer.h"
#include "OpenGLRenderer.h"

Scene2::Scene2(Renderer* renderer_) :
	Scene(nullptr), renderer(renderer_) {
	Debug::Info("Created Scene0: ", __FILE__, __LINE__);
}

Scene2::~Scene2() {
}

bool Scene2::OnCreate() {
	int width = 0, height = 0;
	float aspectRatio;

	switch (renderer->getRendererType()) {
	case RendererType::VULKAN:
	{
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(renderer);

		lightsUBO = vRenderer->CreateUniformBuffers<LightsData>();
		//cameraUBO = vRenderer->CreateUniformBuffers<Camera>();

		SDL_GetWindowSize(vRenderer->getWindow(), &width, &height);
		aspectRatio = static_cast<float>(width) / static_cast<float>(height);
		camera = new Camera(MMath::perspective(45.0f, aspectRatio, 0.5f, 100.0f), 0.0f, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, -5.0f));
		//camera = new Camera();
		//camera.projectionMatrix = MMath::perspective(45.0f, aspectRatio, 0.5f, 100.0f);
		//camera.projectionMatrix[5] *= -1.0f;
		//camera.viewMatrix = MMath::translate(0.0f, 0.0f, -5.0f);
		camera->setRenderer(vRenderer);
		camera->setUBO();
		camera->updateUBO();

		lights.numLights = 3;
		lights.ambient = Vec4(0.1, 0.1, 0.1, 0.0);

		lights.diffuse[0] = Vec4(0.5, 0.0, 0.0, 0.0);
		lights.specular[0] = Vec4(0.3, 0.0, 0.0, 0.0);
		lights.pos[0] = Vec4(-5.0f, 0.0f, -5.0f, 0.0f);

		lights.diffuse[1] = Vec4(0.0, 0.0, 0.5, 0.0);
		lights.specular[1] = Vec4(0.0, 0.0, 0.3, 0.0);
		lights.pos[1] = Vec4(5.0f, 0.0f, -5.0f, 0.0f);

		lights.diffuse[2] = Vec4(0.0, 0.5, 0.0, 0.0);
		lights.specular[2] = Vec4(0.0, 0.3, 0.0, 0.0);
		lights.pos[2] = Vec4(0.0f, 5.0f, -5.0f, 0.0f);

		//lights.diffuse[3] = Vec4(0.0, 0.5, 0.5, 0.0);
		//lights.specular[3] = Vec4(0.0, 0.3, 0.3, 0.0);
		//lights.pos[3] = Vec4(0.0f, -4.0f, -5.0f, 0.0f);

		vRenderer->UpdateUniformBuffer<LightsData>(lights, lightsUBO); // lights UBO (YOU WILL BE MODIFYING THIS)
		//vRenderer->UpdateUniformBuffer<Camera>(camera, camera->getUBO);

		mariosPants = vRenderer->Create2DTextureImage("./textures/mario_fire.png");
		luigisPants = vRenderer->Create2DTextureImage("./textures/mario_mime.png");
		mariosMesh = vRenderer->LoadModelIndexed("./meshes/Mario.obj");

		//colour picking pipeline
		DescriptorSetBuilder descriptorSetBuilderColourPick(vRenderer->getDevice());
		descriptorSetBuilderColourPick.add(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, camera->getUBO());
		colourPickDescriptorSetInfo = descriptorSetBuilderColourPick.BuildDescriptorSet(vRenderer->getNumSwapchains());
 		colourPickPipelineInfo = vRenderer->CreateGraphicsPipeline(colourPickDescriptorSetInfo.descriptorSetLayout, "shaders/colourPicking.vert.spv", "shaders/colourPicking.frag.spv");


		DescriptorSetBuilder descriptorSetBuilder(vRenderer->getDevice());
		descriptorSetBuilder.add(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, camera->getUBO());
		descriptorSetBuilder.add(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1, lightsUBO);
		descriptorSetBuilder.add(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, &mariosPants);

		DescriptorSetInfo mariosdescriptorSetInfo = descriptorSetBuilder.BuildDescriptorSet(vRenderer->getNumSwapchains());

		DescriptorSetBuilder descriptorSetBuilder2(vRenderer->getDevice());
		descriptorSetBuilder2.add(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, camera->getUBO());
		descriptorSetBuilder2.add(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1, lightsUBO);
		descriptorSetBuilder2.add(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, &luigisPants);

		DescriptorSetInfo luigisdescriptorSetInfo = descriptorSetBuilder2.BuildDescriptorSet(vRenderer->getNumSwapchains());

		

		// test of the ECS
		Ref<Actor> marioActor = std::make_shared<Actor>(nullptr);
		marioActor->AddComponent<TransformComponent>(nullptr, Vec3(0.0f, 0.0f, 0.0f), 0.0f, Vec3(0.0f, 1.0f, 0.0f));
		marioActor->SetDescriptorSetInfo(std::make_shared<DescriptorSetInfo>(mariosdescriptorSetInfo));
		//marioActor->AddComponent<MeshComponent>(mariosMesh);
		//marioActor->AddComponent<MaterialComponent>(nullptr, "./textures/mario_fire.png", &descriptorSetBuilder, renderer);
		//marioActor->AddComponent<MeshComponent>
		marioActor->setColourID(10000);
		marioActor->OnCreate();

		Ref<Actor> luigiActor = std::make_shared<Actor>(nullptr);
		luigiActor->AddComponent<TransformComponent>(nullptr, Vec3(2.0f, 0.0f, 0.0f), 0.0f, Vec3(0.0f, 1.0f, 0.0f));
		luigiActor->SetDescriptorSetInfo(std::make_shared<DescriptorSetInfo>(luigisdescriptorSetInfo));
		luigiActor->setColourID(9445865);
		luigiActor->OnCreate();

		actors.insert({ "Mario", marioActor });
		actors.insert({ "Luigi", luigiActor });

		pipelineInfo = vRenderer->CreateGraphicsPipeline(luigisdescriptorSetInfo.descriptorSetLayout, "shaders/multiPhong.vert.spv", "shaders/multiPhong.frag.spv");

		vRenderer->CreateColourPickerResources();
		vRenderer->CreateColourPickerPipeline("shaders/colourPicking.vert.spv", "shaders/colourPicking.frag.spv", camera->getUBO());
	}
	break;

	case RendererType::OPENGL:
		break;
	}

	return true;
}

void Scene2::HandleEvents(const SDL_Event& sdlEvent) {

	switch (sdlEvent.type) {
	case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
		printf("size changed %d %d\n", sdlEvent.window.data1, sdlEvent.window.data2);
		float aspectRatio = static_cast<float>(sdlEvent.window.data1) / static_cast<float>(sdlEvent.window.data2);
		///camera->Perspective(45.0f, aspectRatio, 0.5f, 20.0f);
		camera->setProjectionMatrix(MMath::perspective(45.0f, aspectRatio, 0.5f, 100.0f));

		if (renderer->getRendererType() == RendererType::VULKAN) {
			dynamic_cast<VulkanRenderer*>(renderer)->RecreateSwapChain();
			VulkanRenderer* vRenderer;
			vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
			vRenderer->DestroyPipeline(pipelineInfo);
			pipelineInfo = vRenderer->CreateGraphicsPipeline(actors.find("Mario")->second->GetDescriptorSetInfo()->descriptorSetLayout, "shaders/multiPhong.vert.spv", "shaders/multiPhong.frag.spv");
		}
		break;
	}
	case SDL_EVENT_MOUSE_BUTTON_DOWN: {
		//colour picker handling
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
		int32_t x, y;
		x = sdlEvent.button.x;
		y = sdlEvent.button.y;

		vRenderer->cmdColourPickingBeginRecording();
		vRenderer->cmdColourPickingBindPipeline();
		vRenderer->cmdColourPickingBindDescriptor();
		vRenderer->cmdColourPickingBindMesh(mariosMesh);

		for (auto a : actors) {
			vRenderer->cmdColourPickingPushConstant(a.second->GetModelMatrixPushConst());
			vRenderer->cmdColourPickingDrawIndexed(mariosMesh);
		}
		vRenderer->cmdColourPickingEndRenderPass(x, y); // pass in the screen coords (also stop recording)
		int32_t colourID = vRenderer->ReadPixel(x, y); // extract colourID
		std::cout << "ColourID: " << colourID << std::endl << "X: " << x << "Y: " << y << std::endl;
		break;
	}
	}
}

void Scene2::Update(const float deltaTime) {
	static float elapsedTime = 0.0f;
	elapsedTime += deltaTime;
	//mariosModelMatrix = MMath::rotate(elapsedTime * 90.0f, Vec3(0.0f, 1.0f, 0.0f));
	actors.find("Mario")->second->GetComponent<TransformComponent>()->ApplyRotation(1.0f, Vec3(0.0f,1.0f,1.0f));
	actors.find("Luigi")->second->GetComponent<TransformComponent>()->ApplyRotation(-1.0f, Vec3(1.0f, 1.0f, 0.0f));
	//luigisModelMatrix = MMath::translate(2.0, 0.0f, 0.0f) * MMath::rotate(elapsedTime * 90.0f, Vec3(1.0f, 0.0f, 0.0f));
}

std::vector<Plane> Scene2::CreateFrustrum() const
{
	std::vector<Plane> frustrum;
	Matrix4 clip = camera->getProjectionMatrix() * camera->getViewMatrix();
	Plane left, right, top, bottom, near, far;

	return std::vector<Plane>();
}

void Scene2::CheckFrustrum()
{
}


void Scene2::Render() const {
	switch (renderer->getRendererType()) {

	case RendererType::VULKAN:
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(renderer);


		vRenderer->RecordCommandBuffers(Recording::START);

		//vRenderer->BindMesh(mariosMesh);
		//vRenderer->BindDescriptorSet(pipelineInfo.pipelineLayout, actors.find("Mario")->second->GetDescriptorSetInfo()->descriptorSet);
		//vRenderer->BindPipeline(pipelineInfo.pipeline);
		//vRenderer->SetPushConstant(pipelineInfo, actors.find("Mario")->second->GetComponent<TransformComponent>()->GetTransformMatrix()); // draw mario twices
		//vRenderer->DrawIndexed(mariosMesh);
		//vRenderer->BindDescriptorSet(pipelineInfo.pipelineLayout, actors.find("Luigi")->second->GetDescriptorSetInfo()->descriptorSet);
		//vRenderer->SetPushConstant(pipelineInfo, actors.find("Luigi")->second->GetComponent<TransformComponent>()->GetTransformMatrix());
		//vRenderer->DrawIndexed(mariosMesh);

		//vRenderer->BindDescriptorSet(colourPickPipelineInfo.pipelineLayout, colourPickDescriptorSetInfo.descriptorSet);
		//vRenderer->BindPipeline(colourPickPipelineInfo.pipeline);
		//vRenderer->SetPushConstant(colourPickPipelineInfo, actors.find("Mario")->second->GetModelMatrixPushConst());
		//vRenderer->DrawIndexed(mariosMesh);
		//vRenderer->SetPushConstant(colourPickPipelineInfo, actors.find("Luigi")->second->GetModelMatrixPushConst());
		//vRenderer->DrawIndexed(mariosMesh);
		vRenderer->BindPipeline(pipelineInfo.pipeline);

		for (const auto& a : actors) {
			auto actor = a.second;
			if (actor->culled) { continue; }
			vRenderer->BindDescriptorSet(pipelineInfo.pipelineLayout, actor->GetDescriptorSetInfo()->descriptorSet);
			vRenderer->SetPushConstant(pipelineInfo, actor->GetComponent<TransformComponent>()->GetTransformMatrix());
			vRenderer->DrawIndexed(mariosMesh); // hard coded because only two things in scene and both use same model
		}

		vRenderer->RecordCommandBuffers(Recording::STOP);
		vRenderer->Render();
		break;

	case RendererType::OPENGL:
		OpenGLRenderer* glRenderer;
		glRenderer = dynamic_cast<OpenGLRenderer*>(renderer);
		/// Clear the screen
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		/// Draw your scene here
		glUseProgram(0);

		break;
	}
}


void Scene2::OnDestroy() {
	VulkanRenderer* vRenderer;
	vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
	if (vRenderer) {
		vkDeviceWaitIdle(vRenderer->getDevice());
		vRenderer->DestroyCommandBuffers();
		vRenderer->DestroyPipeline(pipelineInfo);
		//vRenderer->DestroyDescriptorSet(mariosdescriptorSetInfo);
		if (actors.find("Mario") != actors.end() && actors.find("Mario")->second->GetDescriptorSetInfo()) {
			vRenderer->DestroyDescriptorSet(*actors.find("Mario")->second->GetDescriptorSetInfo());
		}
		if (actors.find("Luigi") != actors.end() && actors.find("Luigi")->second->GetDescriptorSetInfo()) {
			vRenderer->DestroyDescriptorSet(*actors.find("Luigi")->second->GetDescriptorSetInfo());
		}
		vRenderer->DestroyUBO(lightsUBO);
		//vRenderer->DestroyUBO(cameraUBO);
		vRenderer->DestroySampler2D(mariosPants);
		vRenderer->DestroySampler2D(luigisPants);
		vRenderer->DestroyIndexedMesh(mariosMesh);

		vRenderer->DestroyColourPickerPipeline();
		vRenderer->DestroyColourPickerResources();
	}


}

