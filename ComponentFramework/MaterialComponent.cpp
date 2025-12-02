//#include "MaterialComponent.h"
//
//#include <SDL_image.h>
//
//
//
//MaterialComponent::MaterialComponent(Component* parent_, const char* filename_, DescriptorSetBuilder* descriptorSetBuilder_, Renderer* renderer) :
//	Component(parent_), filename(filename_) , descriptorSetBuilder(descriptorSetBuilder_), renderer(renderer)
//{
//}
//
//MaterialComponent::~MaterialComponent() {
//	glDeleteTextures(1, &textureID);
//}
//
//bool MaterialComponent::OnCreate() {
//	if (isCreated == true) return true;
//	isCreated = true;
//	return LoadImage(filename);
//}
//
//bool MaterialComponent::LoadImage(const char* filename) {
//	glGenTextures(1, &textureID);
//	glBindTexture(GL_TEXTURE_2D, textureID);
//	SDL_Surface* textureSurface = IMG_Load(filename);
//	if (textureSurface == nullptr) {
//		return false;
//	}
//	int mode = (textureSurface->format->BytesPerPixel == 4) ? GL_RGBA : GL_RGB;
//	glTexImage2D(GL_TEXTURE_2D, 0, mode, textureSurface->w, textureSurface->h, 0, mode, GL_UNSIGNED_BYTE, textureSurface->pixels);
//
//	SDL_FreeSurface(textureSurface);
//	/// Wrapping and filtering options
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glBindTexture(GL_TEXTURE_2D, 0); /// Unbind the texture
//	vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
//
//	Sampler2D tempTex = vRenderer->Create2DTextureImage(filename);
//	texture = std::make_shared<Sampler2D>(tempTex);
//	descriptorSetBuilder->add(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, texture.get());
//
//	descriptorSetInfo = std::make_shared<DescriptorSetInfo>(descriptorSetBuilder->BuildDescriptorSet(vRenderer->getNumSwapchains()));
//	return true;
//}
//
//
//void MaterialComponent::OnDestroy() {}
//void MaterialComponent::Update(const float deltaTime) {}
//void MaterialComponent::Render()const {}