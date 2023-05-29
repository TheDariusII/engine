#pragma once
#include <vector>
#include <optional>
#include <string_view>
#include <functional>
#include <queue>

#include <glm/glm.hpp>

#include <Model.h>
#include <Colider.h>
#include <RigidBody.h>
#include <Transform.h>
#include <Script.h>
#include <Scene.h>
#include <CubeMesh.h>
#include <PointLight.h>

class Colider;

class Object
{
public:
	Object(std::string name_in, glm::vec3 pos_in, glm::vec3 scale_in, glm::vec3 collider_in, std::string_view model_path_in, Shader model_shader, std::function<void(Transform) > shaderRoutine_in,
																									Scene* scn, std::function<void(ScriptArgument*)>&& st, std::function<void(ScriptArgument*)>&& upd, 
																									bool gammaShader = false, bool rotateTextures = false)
	{
		model = Model(model_path_in, model_shader, shaderRoutine_in, gammaShader, rotateTextures);
		
		if(model.has_value())
			model.value().loadModel();

		script = Script(scn, this, std::move(st), std::move(upd));

		tr.position = pos_in;
		tr.scale = scale_in;

		rbody.emplace(RigidBody(0.1, tr, false));
		rbody.value().create_box_inertia_tensor(10, { 1,1,1 });

		name = name_in;

		colider.emplace(collider_in, tr, rbody.value());
	}

	Object(Object* parentObject, Mesh& m, Shader model_shader, std::function<void(Transform) > shaderRoutine_in)
	{
		parent = parentObject;
		model = Model(m, model_shader, shaderRoutine_in);

		tr = parentObject->getTransform();//TODO(darius) fix it

		rbody.emplace(1,tr,false);
		rbody.value().get_is_static_ref() = true;

		//apply_force({0.1,0.1,0.1});

		name = parentObject->get_name() + " child " + std::to_string((size_t)this);

		colider.emplace(parentObject -> getColider().get_size(), tr, rbody.value(), 0, false);

		script = parentObject->getScript();
		script -> setParentObject(this);

	}

	Object(std::string&& name_in, glm::vec3 pos_in, glm::vec3 scale_in, glm::vec3 collider_in, Mesh& m, Shader model_shader, std::function<void(Transform)> shaderRoutine_in,
																					Scene* scn, std::function<void(ScriptArgument*)>&& st, std::function<void(ScriptArgument*)>&& upd, bool active = true)
	{
		name = std::move(name_in);
		tr = Transform(pos_in, scale_in);
		rbody.emplace(0.1, tr, false);
		rbody.value().get_is_static_ref() = true;
		model = Model(m, model_shader, shaderRoutine_in);
		script = Script(scn, this, std::move(st), std::move(upd));
		colider.emplace(collider_in, tr, rbody.value(), 0, active);
	}

	void setupScript(std::function<void(ScriptArgument*)>&& st, std::function<void(ScriptArgument*)>&& upd)
	{
		script.value().set_scripts(std::move(st), std::move(upd));
	}

	void startScript()
	{
		if (!script.has_value())
			return;
		script.value().startScript();
		//traverseChilds([](Object* op) {op->startScript(); });
	}

	void updateScript() 
	{
		script.value().updateScript();
		//traverseObjects([](Object* op){op->updateScript()});
	}

	void renderObject() 
	{
		if(!object_hidden)
			model.value().Draw(Transform(getTransform()));

		//DANGER! -> traverseObjects([](Object* op) {op->renderObject(); });

		traverseChilds([](Object* op) {op->renderObject(); });
	}

	void apply_force(glm::vec3 force) 
	{
		rbody.value().add_force(force);
	}

	void updatePos() 
	{
		if(rbody.has_value())
			rbody.value().update(0.01);

		//DANGER! -> traverseObjects([](Object* op) {op->updatePos(); });
		traverseChilds([](Object* op) {op->updatePos(); });
	}

	glm::vec3 get_pos() 
	{
		//if (parent.has_value())
		//	return parent.value()->get_pos();
		return getTransform().position;
	}
	
	//TODO(darius) fuck you, incapsulation
	glm::vec3& get_pos_ref()
	{
		if (parent.has_value())
			return parent.value()->get_pos_ref();
		return getTransform().position;
	}

	Colider& getColider() 
	{
		return colider.value();
	}

	RigidBody& getRigidBody()
	{
		return rbody.value();
	}

	Transform& getTransform()
	{
		//if (parent.has_value())
		//	return parent.value()->getTransform(); 
		return tr;
	}

	Transform& getParentTransform()
	{
		if (parent.has_value())
			return parent.value()->getTransform(); 
		return tr;
	}	
	
	Script& getScript()
	{
		return script.value();
	}
	
	void set_child_objects(std::vector<Object*>&& objects)
	{
		child_opbjects = objects;
	}

	std::vector<Object*>& get_child_objects() 
	{
		return child_opbjects;
	}

	void traverseChilds(std::function<void(Object*)> functor)
	{
		for (auto& i : child_opbjects) {
			//std::cout << "tr " << tr.position.x << "|" << tr.position.y << "|" << tr.position.z << "\n";
			//std::cout << "child x: " << i->getTransform().position.x << "\n";
			functor(i);
		}
	}

	void traverseObjects(std::function<void(Object*)> functor, bool include_root = false)
	{
		std::queue<Object*> q;
		if(include_root)
			q.push(this);

		for (auto& i : child_opbjects)
			q.push(i);

		while(!q.empty())
		{
			auto curr = q.front();
			q.pop();

			functor(curr);

			for (auto& i : curr -> get_child_objects()) {
				q.push(i);
			}
		}
	}

	std::string& get_name() 
	{
		return name;
	}

	void hide()
	{
		object_hidden = true;
	}

	void unhide()
	{
		object_hidden = false;
	}

	bool& object_hidden_state()
	{
		return object_hidden;
	}

	void addPointLight(PointLight&& pl)
	{
		pointLight = pl;
	}

	std::optional<PointLight>& getPointLight()
	{
		return pointLight;
	}

private:
	std::optional<Model> model;
	std::optional<RigidBody> rbody;
	std::optional<Colider> colider;
	std::optional<Script> script;
	std::optional<Object*> parent;
	std::optional<PointLight> pointLight;

	Transform tr;
	std::string name;
	std::vector<Object*> child_opbjects = {};
	bool object_hidden = false;
};
