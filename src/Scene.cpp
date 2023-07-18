#include <Scene.h>
#include <Object.h>
#include <Colider.h>
#include <Model.h>
#include <ParticleSystem.h>

#include <string>
#include <iostream>
#include <fstream>


Scene::Scene()
{
	init_memory();
	//start_scripts();
}

Object* Scene::createEntity(Object* po, std::string path, Shader sv, LightingShaderRoutine shaderRoutine_in, bool rotateTextures = false) {
	Model m = Model(path, shaderRoutine_in, sv, rotateTextures);
	auto meshes = m.loadModel();

	std::vector<Object*> subobjects;
	subobjects.reserve(meshes.size());
	for (int i = 0; i < meshes.size(); ++i) {
		Object* pt = mem_man.construct(po, meshes[i], sv, shaderRoutine_in);
		std::cout << "created pt\n";
		subobjects.push_back(pt);
	}

	po->set_child_objects(std::move(subobjects));

	po->startScript();
	return po;
}

Object* Scene::AddEmpty(int i)
{
	Object* pt;
	pt = mem_man.construct("empty " + std::to_string(i));
	sceneObjects.push_back(pt);
	return pt;
}
Object* Scene::AddObject(const std::string& name) 
{
	Object* pt;
	pt = mem_man.construct(name);
	sceneObjects.push_back(pt);
	return pt;
}

Object* Scene::createSubobject(Object* obj, int i)
{
	Object* pt;
	pt = mem_man.construct("empty " + std::to_string(i));
	obj->addChild(pt);

	return pt;
}

void Scene::destroyObject(size_t id)
{
	auto* ptr = sceneObjects[id];
	mem_man.destroy(ptr);
	sceneObjects[id] = nullptr;
}

void Scene::updateScene() {
	update_objects();
}

void Scene::renderScene()
{
	for (int i = 0; i < sceneObjects.size(); ++i) {
		if (!sceneObjects[i]) // in case sceneObjects[i] was deleted by index
			continue;
		sceneObjects[i]->renderObject();
	}
}

void Scene::renderParticles()
{
	for (int i = 0; i < sceneObjects.size(); ++i) {
		if (!sceneObjects[i]->getParticleSystem())
			continue;

		sceneObjects[i]->getParticleSystem()->updateUniform3DDistribution(static_cast<float>(glfwGetTime()));
		sceneObjects[i]->getParticleSystem()->renderParticles();
	}
}

void Scene::updateSpriteAnimations(float dt) 
{
	for (int i = 0; i < sceneObjects.size(); ++i) 
	{
		sceneObjects[i]->updateSpriteAnimation(dt);
	}
}

void Scene::updateAnimators(float dt)
{
	for (auto& obj : sceneObjects) {
		if (!obj->getAnimator())
			continue;
		obj->updateAnimator(dt);
	}
}

Object* Scene::get_object_at(int i)
{
	return sceneObjects.at(i);
}

std::vector<Object*>& Scene::get_objects()
{
	return sceneObjects;
}

void Scene::init_memory()
{
	mem_man.allocate(1000);
}

void Scene::start_scripts()
{
	std::cout << "Starting scripts..\n";
	for (auto& i : sceneObjects)
	{
		i->startScript();
	}
}

FlatMesh* Scene::createFlatMesh()
{
	return new FlatMesh();//TODO(darius) new bad
}

void Scene::deleteFlatMesh(FlatMesh* mesh)
{
	delete mesh;
}

void Scene::update_objects() {
	//now its traverse of objects and update. Its much better to do it in one traverse
	//TODO(darius) make it separated threads for collisions and rendering and update?

	for (int i = 0; i < sceneObjects.size(); ++i) {
		if (!sceneObjects[i]) // in case sceneObjects[i] was deleted by index
			continue;


		sceneObjects[i]->updateScript();


		if (sceneObjects[i]->getColider() && !sceneObjects[i]->getColider()->is_active()) {
			sceneObjects[i]->updatePos();
			continue;
		}

		//COLLISIONS RESOLUTION:
		// O(n^2) 
		// sort by tag + traverse in O(Nlg + N)?
		// make it two types of collision detection: 1) important collision inside renderer thread 2) queued colllision that processed in separate thread?

		bool is_there_collision = false;
		for (int j = 0; j < sceneObjects.size(); ++j) {
			if (
				i == j
				|| !sceneObjects[i]->getColider()
				|| !sceneObjects[j]->getColider()
				|| sceneObjects[i]->getColider()->get_tag() != sceneObjects[j]->getColider()->get_tag()
				|| !sceneObjects[j]->getColider()->is_active()
				)	continue;

			//auto collision_state = sceneObjects[i]->getColider()->gjk(&sceneObjects[i]->getColider().value(), &sceneObjects[j]->getColider().value());
			auto collision_state = sceneObjects[i]->getColider()->check_collision(sceneObjects[j]->getColider().value());

			if (collision_state != glm::vec3{0,0,0}) {
				std::cout << "collision of" << sceneObjects[i]->get_name() << "\n";
				////is_there_collision = true;

				//glm::vec3 epa = sceneObjects[i]->getColider()->get_epa();
				//std::cout << epa.x << " " << epa.y << " " << epa.z << "\n";
				std::cout << collision_state.x << " " << collision_state.y << "\n";
				
				//NOTE(darius) this is a trick to check if float is Nan
				//TODO(darius) not good
				/*if (epa.x == epa.x && epa.y == epa.y && epa.z == epa.z)
				{
					sceneObjects[i]->getTransform().position += glm::vec3{ -0.1, -0.1,0};
					*sceneObjects[i]->getColider()->get_collision_state() = false;
				}
				*/

				sceneObjects[i]->getTransform().position += collision_state;
				*sceneObjects[i]->getColider()->get_collision_state() = false;

				/*
				if (sceneObjects[i]->getRigidBody() && sceneObjects[j]->getRigidBody() && epa.x == epa.x && epa.y == epa.y && epa.z == epa.z) {
					sceneObjects[i]->getRigidBody()->tr.position += glm::vec3{ epa.x / 4, epa.y / 4, epa.z / 4 };
				}
				*/

				break;
			}
		}
		if (!is_there_collision) {
			sceneObjects[i]->updatePos();
		}
	}
}

void Scene::update_scripts()
{
	for (auto& i : sceneObjects)
	{
		i->updateScript();
	}
}

void Scene::serialize(std::string_view path)
{
	std::ofstream file(path.data());
    if(!file.is_open())
        return;
    
	for (auto& i : sceneObjects)
	{
		i->serialize(file);

	}

    file.close();
}

//TODO(darius) check for names being different, and check if object that you created dont exist already
void Scene::deserialize(std::string_view path)
{

	std::ifstream file(path.data());
	if (!file.is_open())
		return;
	
	std::string data;
	std::string line;
	//TODO(darius) optimize it
	while (std::getline(file, line)) 
	{
		data += line;
	}

	std::vector<std::string> objectTokens;

	{
		size_t i = 0;

		while (i < data.size()) 
		{
			size_t oPos1 = data.find("Object", i);
			size_t oPos2 = data.find("Object", oPos1+1);
			i = oPos2;
			objectTokens.push_back(data.substr(oPos1, oPos2-oPos1));
		}
	}

	std::vector<std::string> names;
	for (std::string_view tkn : objectTokens) 
	{
		names.push_back(extractNameFromToken(tkn));
	}

	//Recreation here after

	for (std::string_view name : names) 
	{
		AddObject(std::string(name));
	}

	file.close();
}

std::string Scene::extractNameFromToken(std::string_view tkn)
{
	size_t nameStart = tkn.find("Name");
	size_t valueStart = nameStart + 9;
	size_t valueEnd = tkn.find("\t", valueStart);
	return std::string(tkn.substr(valueStart, valueEnd - valueStart));
}

glm::vec3 Scene::extractTransformFromToekn(std::string_view)
{
	return glm::vec3();
}

Colider Scene::extractColliderFromToken(std::string_view)
{
	//DANGER(darius) temp
	Transform tr;
	return Colider(glm::vec3{ 0,0,0 }, tr);
}

