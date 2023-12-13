#pragma once
#include <vector>
#include <optional>
#include <string_view>
#include <functional>
#include <queue>
#include <memory>
#include <fstream>

#include <glm/glm.hpp>

#include <Model.h>
#include <Colider.h>
#include <RigidBody.h>
#include <Transform.h>
#include <Script.h>
#include <Scene.h>
#include <CubeMesh.h>
#include <PointLight.h>
#include <ParticleSystem.h>
#include <Animation.h>

class Object
{
public:
	Object(std::string name_in);

	//NOTE(darius) not sure about this stuff anymore. clean up constructors that we dont need and create new constructors that we need?
	Object(std::string name_in, Shader model_shader);

	Object(std::string name_in, glm::vec3 pos_in, glm::vec3 scale_in, glm::vec3 collider_in, std::string_view model_path_in,
																									Scene* scn, EmptyScriptRoutine* routine,
																									bool gammaShader = false, bool rotateTextures = false);

	Object(Object* parentObject, Mesh& m);

	Object(std::string&& name_in, glm::vec3 pos_in, glm::vec3 scale_in, glm::vec3 collider_in, Mesh& m,
																					Scene* scn, EmptyScriptRoutine* routine, bool active = true);

	Object(std::string&& name_in, glm::vec3 pos_in, glm::vec3 scale_in, glm::vec3 collider_in, const Model& m, Scene* scn, EmptyScriptRoutine* routine, bool active);

	Object(const Object& copy_me);

	Object& operator =(const Object& obj) = delete;

	void setupScript(EmptyScriptRoutine* r);

	void startScript();

	void updateScript(); 

	void renderObject(); 

	void updateParticleSystem(float dt);

	void apply_force(glm::vec3 force); 

	void updatePos(); 

	glm::vec3 get_pos(); 
	
	std::optional<Colider>& getColider(); 

	void addRigidBody(float mass = 0.0);

	std::optional<RigidBody>& getRigidBody();

	std::optional<Model>& getModel();

	void frozeObject();

	void unfrozeObject();

	Transform& getTransform();

	Transform& getParentTransform();

	void moveTransform(glm::vec3 v);

	void addScript(Scene* scn, EmptyScriptRoutine* routine);
	
	std::optional<Script>& getScript();
	
	void set_child_objects(std::vector<Object*>&& objects);

	std::vector<Object*>& get_child_objects(); 

	void addChild(Object* obj);

	void traverseChilds(std::function<void(Object*)> functor);

	void traverseObjects(std::function<void(Object*)> functor, bool include_root = false);
	
	std::string& get_name(); 

	void hide();

	void unhide();

	bool is_hidden();

	bool& object_hidden_state();

	void cull();

	void uncull();

	bool is_culled();

	void serialize(std::ostream& file);

	void serializeAsPrefab(std::ofstream& file);

	void deserialize(std::ofstream& file);

	void addPointLight(PointLight&& pl = {}, glm::vec3 pos = { 0,0,0 });

	void addCollider(glm::vec3 size = {0,0,0}, glm::vec3 shift = { 0,0,0 });

	void addModel();

	void addModel(Mesh&& m);

	//TODO(darius) when loading big models add threading
	void addModel(std::string_view path);
	
	void addParticleSystem(ParticleSystem&& ps);

	std::optional<ParticleSystem>& getParticleSystem();

	std::optional<PointLight>& getPointLight();

	bool& shadowCasterRef();

    void setDefaultMaterial();

	void setMaterial(const Material& m);

	std::optional<Material>& getMaterial();

	void setID(int id);

	int getID();

	void setAnimator(SkeletalAnimation* anim);

	std::optional<SkeletalAnimation>& getSkeletalAnimation();

	std::optional<Animator>& getAnimator();

	void initAnimator();

    void addSpriteAnimation(SpriteAnimation&& anim);

	SpriteAnimation excnahgeSpriteAnimation(SpriteAnimation&& anim);

	void setSpriteAnimation(SpriteAnimation& anim);

    std::optional<SpriteAnimation>& getSpriteAnimation();

	void updateSpriteAnimation(float dt);

	void updateAnimator(float dt);

private:
	//TODO(darius) make it Component system
	// Obvious solution is to make use of virtual functions and stuff. And just store vector<Cmponent>
	// and then just call component[i] -> virtualUpdateFunction(). But Virtual function dispatch for each scne update frame its to much work.
	// So the idea is to allow object only one component of each type. And if you want more - just add subobject with this component.
	// NOTE(darius) dont forget optional cant contain reference or heap object
	// use indexing?
	// make it vector pointer to all components of specific type, for each component type?

	//NOTE(darius) How bad this is?
	//suppose O(amount of objects) = 10000 
	//sizeof(Object) ~= 1616b => 10000 * 1616b = 16Mb. Thats very little of waste!

	std::optional<Model> model;
	std::optional<RigidBody> rbody;
	std::optional<Colider> colider;
	std::optional<Script> script;//NOTE(darius) there shouldnt be many scripts, its bad
	std::optional<PointLight> pointLight;
	std::optional<Material> material;
	std::optional<ParticleSystem> particles;
	std::optional<SkeletalAnimation> skeletAnim;
    std::optional<SpriteAnimation> spriteAnimation;
    std::optional<Animator> animator;

	Transform tr;
	std::string name;

	//NOTE(darius) maybe get rid of it?
	std::vector<Object*> child_opbjects = {};
	Object* parent = nullptr;

	//TODO(darius) looks like cull is same as hide, or is it
	bool object_hidden = false;
	bool object_culled = false;
	bool shadowCaster = false;

	int ID = 0;
};

