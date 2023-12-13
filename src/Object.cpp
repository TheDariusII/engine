#include <Object.h>
#include <fstream>
#include <iostream>
#include <string>
#include <cctype>

Object::Object(const Object& copy_me) = default;

Object::Object(std::string name_in) 
    : name(std::move(name_in))
{
    //model.emplace(std::string(""));

    tr.setPosition({0,0,0});
    tr.setScale({1,1,1});
}

Object::Object(std::string name_in, glm::vec3 pos_in, glm::vec3 scale_in, glm::vec3 collider_in, std::string_view model_path_in,
                                                                                                Scene* scn, EmptyScriptRoutine* routine,
                                                                                                bool gammaShader, bool rotateTextures)
    : name(std::move(name_in))
{
    model.emplace(model_path_in);
    
	//model->loadModel();

    script = Script(scn, this, routine);

    //tr.getPositionRef() = pos_in;
    //tr.getPositionRef()  = scale_in;
    tr.setPosition(pos_in);
    tr.setScale(scale_in);

    rbody.emplace(RigidBody(0.1f, tr, false));
    rbody->create_box_inertia_tensor(10, { 1,1,1 });

    colider.emplace(collider_in, tr);
}

Object::Object(Object* parentObject, Mesh& m)
{
    parent = parentObject;
    model.emplace(m);

    tr = parentObject->getTransform();//TODO(darius) fix it

    rbody.emplace(1,tr,false);
    rbody->get_is_static_ref() = true;

    //apply_force({0.1,0.1,0.1});

    name = parentObject->get_name() + " child " + std::to_string((size_t)this);

    //if(parentObject.getColider())
    //    colider.emplace(parentObject -> getColider()->get_size(), tr, 0, false);

    //script = parentObject->getScript();
    //script -> setParentObject(this);
}

Object::Object(std::string&& name_in, glm::vec3 pos_in, glm::vec3 scale_in, glm::vec3 collider_in, Mesh& m,
                                                                                Scene* scn, EmptyScriptRoutine* routine, bool active)
{
    name = std::move(name_in);
    tr = Transform(pos_in, scale_in);

    rbody.emplace(0.1, tr, false);
    rbody->get_is_static_ref() = true;

    model.emplace(m);

    script = Script(scn, this, routine);

    colider.emplace(collider_in, tr, 0, active);
}

Object::Object(std::string&& name_in, glm::vec3 pos_in, glm::vec3 scale_in, glm::vec3 collider_in, const Model& m, Scene* scn, EmptyScriptRoutine* routine, bool active)
{
    name = std::move(name_in);

    tr = Transform(pos_in, scale_in);

    rbody.emplace(0.1, tr, false);
    rbody->get_is_static_ref() = true;

    model.emplace(m);//Model(m, model_shader, shaderRoutine_in);

    script = Script(scn, this, routine);

    colider.emplace(collider_in, tr, 0, active);
}

void Object::setupScript(EmptyScriptRoutine* r)
{
    script.value().set_scripts(r);
}

void Object::startScript()
{
    if (!script.has_value())
        return;
    script->startScript();
    //traverseChilds([](Object* op) {op->startScript(); });
}

void Object::updateScript() 
{
    if(script.has_value())
        script->updateScript();
    //traverseObjects([](Object* op){op->updateScript()});
}

void Object::renderObject() 
{
    if(!object_hidden && !object_culled && model){
        model->Draw(this, getPointLight(), getMaterial());
    }

    //DANGER! -> traverseObjects([](Object* op) {op->renderObject(); });

    traverseChilds([](Object* op) {op->renderObject(); });
}

void Object::updateParticleSystem(float dt)
{
    if(particles){
        particles->updateUniform3DDistribution(dt);
        particles->renderParticles();
    }
}

void Object::apply_force(glm::vec3 force) 
{
    if(!rbody)
        return;

    rbody->add_force(force);
}

void Object::updatePos() 
{
    if (colider && *colider->get_collision_state() == true)
        return;

    if(rbody.has_value())
        rbody->update(0.01f);

    //DANGER! -> traverseObjects([](Object* op) {op->updatePos(); });
    traverseChilds([](Object* op) {op->updatePos(); });
}

glm::vec3 Object::get_pos() 
{
    //if (parent.has_value())
    //	return parent.value()->get_pos();
    return getTransform().getPosition();
}

//TODO(darius) fuck you, incapsulation

std::optional<Colider>& Object::getColider() 
{
    return colider;
}

void Object::addRigidBody(float mass)
{
    rbody.emplace(mass, tr, true);
}

std::optional<RigidBody>& Object::getRigidBody()
{
    return rbody;
}

std::optional<Model>& Object::getModel()
{
    return model;
}

void Object::frozeObject()
{
    rbody.value().is_static = true;
}

void Object::unfrozeObject()
{
    rbody.value().is_static = false;
}

Transform& Object::getTransform()
{
    //if (parent.has_value())
    //	return parent.value()->getTransform(); 
    return tr;
}

Transform& Object::getParentTransform()
{
    return tr;
}

void Object::moveTransform(glm::vec3 v)
{
    if (colider && *colider->get_collision_state() == true)
        return;

    //getTransform().getPositionRef() += v;
    getTransform().addToPosition(v);
}

void Object::addScript(Scene* scn, EmptyScriptRoutine* routine)
{
    if (script.has_value())
        return;
    script = Script(scn, this, routine);
    //NOTE(darius) Should I start it here?
    //script->startScript();
}

std::optional<Script>& Object::getScript()
{
    return script;
}

void Object::set_child_objects(std::vector<Object*>&& objects)
{
    child_opbjects = objects;
}

std::vector<Object*>& Object::get_child_objects() 
{
    return child_opbjects;
}

void Object::addChild(Object* obj)
{
    child_opbjects.push_back(obj);
}

void Object::traverseChilds(std::function<void(Object*)> functor)
{
    for (auto& i : child_opbjects) {
        //std::cout << "tr " << tr.position.x << "|" << tr.position.y << "|" << tr.position.z << "\n";
        //std::cout << "child x: " << i->getTransform().position.x << "\n";
        functor(i);
    }
}

void Object::traverseObjects(std::function<void(Object*)> functor, bool include_root)
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

std::string& Object::get_name() 
{
    return name;
}

void Object::hide()
{
    object_hidden = true;
}

void Object::unhide()
{
    object_hidden = false;
}

bool Object::is_hidden()
{
    return object_hidden;
}

bool& Object::object_hidden_state()
{
    return object_hidden;
}

void Object::cull()
{
    object_culled = true;   
}

void Object::uncull()
{
    object_culled = false;  
}

bool Object::is_culled()
{
    return object_culled;   
}

void Object::addPointLight(PointLight&& pl, glm::vec3 pos)
{
    if(pointLight || !model)
        return;

    pointLight = pl;
    pointLight->addLight();
    //pointLight.position = pos;
}

void Object::addCollider(glm::vec3 size, glm::vec3 shift)
{
    if(colider)
        return;
    Colider col(size, tr);
    col.set_shift(shift);
    colider.emplace(std::move(col));
}

void Object::addModel()
{
    if (model)
        return;
    model = Model();
}

void Object::addModel(Mesh&& m)
{
    if(model)
        return;
    //setDefaultMaterial();
    model.emplace(std::move(m));
}

void Object::addModel(std::string_view path)
{
    if(model)
        return;
    //setDefaultMaterial();
    model.emplace(path);
}

void Object::addParticleSystem(ParticleSystem&& ps)
{
    if(particles)
        return;

    particles.emplace(std::move(ps));	
}

std::optional<ParticleSystem>& Object::getParticleSystem()
{
    return particles;	
}

std::optional<PointLight>& Object::getPointLight()
{
    return pointLight;
}

bool& Object::shadowCasterRef()
{
    return shadowCaster;
}

void Object::setDefaultMaterial()
{
    setMaterial(Material(32));
}

void Object::setMaterial(const Material& m)
{
    if(material)
        return;
    material = m;
}

std::optional<Material>& Object::getMaterial()
{
    return material;
}

void Object::setID(int id)
{
    ID = id;
}

int Object::getID()
{
    return ID;
}

void Object::addSpriteAnimation(SpriteAnimation&& anim)
{
    //TODO(darius) its actually ugly.
    if(spriteAnimation)
        return;
    
    spriteAnimation.emplace(std::move(anim));
    if (model && model->meshes.size() > 0)
        spriteAnimation->setSprite((FlatMesh*) &model->meshes[0]);
}

//todo(darius) check NRVO
SpriteAnimation Object::excnahgeSpriteAnimation(SpriteAnimation&& anim)
{
    assert(spriteAnimation);

    SpriteAnimation&& tmp = std::move(spriteAnimation.value());
    spriteAnimation.emplace(std::move(anim));

    return tmp;
}

void Object::setSpriteAnimation(SpriteAnimation& anim)
{
    spriteAnimation.emplace(anim);

    if (model && model->meshes.size() > 0)
        spriteAnimation->setSprite((FlatMesh*)&model->meshes[0]);
}

std::optional<SpriteAnimation>& Object::getSpriteAnimation()
{
    return spriteAnimation;
}

void Object::updateSpriteAnimation(float dt) 
{
    if (animator && spriteAnimation) {
        animator->update(dt);
    }
}

void Object::updateAnimator(float dt)
{
    if(!skeletAnim || !animator)
        return;
    //NOTE(darius) you update copy inside animator
    // fix it in shaderLibrary
    animator->update(dt);
    //skeletAnim->update(dt);
}

void Object::setAnimator(SkeletalAnimation* anim)
{
    if(skeletAnim)
        return;
    skeletAnim.emplace(*anim);
}

std::optional<SkeletalAnimation>& Object::getSkeletalAnimation()
{
    return skeletAnim;
}

std::optional<Animator>& Object::getAnimator()
{
    return animator; 
}

void Object::initAnimator()
{
    animator.emplace(Animator());
    if(spriteAnimation)
        animator->addAnimation(*spriteAnimation);
    if(skeletAnim)
        animator->addAnimation(*skeletAnim);
}

void Object::serializeAsPrefab(std::ofstream& file)
{
    //file << "Prefab: {\n";

    serialize(file);

    //file << "}\n";
}

void Object::serialize(std::ostream& file)
{
    file << "Object: {\n";
    file << "\tName: {\n\t\t" << get_name().c_str() << "\n\t}\n";
    file << "\tHidden: {\n\t\t" << (object_hidden == true ? "true" : "false") << "\n\t}\n";
    //CM std::to_string() to convert digit to string
    {
        file << "\tTransform: {\n";
        file << "\t\tPosition: {" << std::to_string(tr.getPosition().x) << " " << std::to_string(tr.getPosition().y) << " " << std::to_string(tr.getPosition().z) << "}\n";
        file << "\t\tScale: {" << std::to_string(tr.getScale().x) << " " << std::to_string(tr.getScale().y) << " " << std::to_string(tr.getScale().z) << "}\n";
        auto quatVal = tr.matrixQuaternion();
        file << "\t\tQuaternion: {" << std::to_string(quatVal.x) << " " << std::to_string(quatVal.y) << " " << std::to_string(quatVal.z) << " " << std::to_string(quatVal.w) << "}\n";

        file << "\t\tMatrix: {";
        auto matrixVector = tr.matrixVector();

        for(int mi = 0; mi < matrixVector.size(); ++mi)
        {
            file << std::to_string(matrixVector[mi]) << " ";
        }

        file << "}\n";

        file << "\t}\n";
    }

    if (rbody)
    {
        file << "\tRigidBody: {" << std::to_string(rbody->mass) << " " << std::to_string(rbody->is_static) << " " << 0 << "}\n";
    }

    if (colider)
    {
        file << "\tCollider: {\n";
        file << "\t\tSize: {" << std::to_string(colider->get_size().x) << " " << std::to_string(colider->get_size().y) << " " << std::to_string(colider->get_size().z) << "}\n";
        file << "\t\tShift: {" << std::to_string(colider->get_render_shift().x) << " " << std::to_string(colider->get_render_shift().y) << " " << std::to_string(colider->get_render_shift().z) << "}\n";
        file << "\t}\n";
    }

    if (model)
    {
        //NOTE(darius) in case it was created without path - store its meshes 
        //NOTE(darius) mesh seriliztion is just 1) storing vector of Vertices
                                              //2) indices
                                              //3) textures, which is just storing path => FlatMesh.setexture()

        file << "\tModel: {\n";
        model->path.shrink_to_fit();
        file << "\t\tPath: {" << model->path << "}\n";

        //NOTE(darius) basicly its a start for implementation of my own .obj file. But its slow asfuck and i wont get to it now. 
        if (model->meshes.size() < 2) {
            file << "\t\tMeshes: {\n";

            for (auto& i : model->meshes)
            {
                //TODO(darius) optim copy
                auto vertices = i.getVertices();
                auto indices = i.getIndices();
                auto textures = i.getTextures();


                file << "\t\t\tVertices: {\n";
                for (auto& v : vertices)
                {
                    file << "\t\t\t\tPosition: {" << std::to_string(v.Position.x) << " " << std::to_string(v.Position.y) << " " << std::to_string(v.Position.z) << "}\n";
                    file << "\t\t\t\tNormal: {" << std::to_string(v.Normal.x) << " " << std::to_string(v.Normal.y) << " " << std::to_string(v.Normal.z) << "}\n";
                    file << "\t\t\t\tTexCoords: {" << std::to_string(v.TexCoords.x) << " " << std::to_string(v.TexCoords.y) << " " << std::to_string(0) << "}\n";
                    file << "\n";
                }
                file << "\t\t\t}\n";

                file << "\t\t\tIndices: {\n";
                for (auto& i : indices)
                {
                    file << "\t\t\t\tIndice: {" << i << "}\n";
                }
                file << "\t\t\t}\n";

                file << "\t\t\tTextures: {\n";
                for (auto& t : textures)
                {
                    file << "\t\t\t\tTexture: {"; //<< t.get_path() << "}\n";
                    
                    std::string tkn = t.get_path(); 
                    std::cout << tkn << "\n";
                    for(auto c : tkn)
                        if(c > -1 && c < 255 && c != '\0' && !std::isspace(c))
                            file << c; 
                    file << "}\n";

                    file << "\t\t\t\tType: {" << t.get_type() << "}\n";
                }

                file << "\t\t\t}\n";

                file << "\t\t\tDrawMode: {" << (unsigned int)i.getDrawMode() << "}\n";
            }

            file << "\t\t}\n";
        }
        file << "\t}\n";
    }

    if (spriteAnimation)
    {
        file << "\tSpriteAnimation: {\n";

        file << "\t\tDelay: {" << std::to_string((long)*spriteAnimation->getDelay()) << "}\n"; 

        if (!spriteAnimation->isAnimationUsesMultipleTextures())
        {
            for (auto& p : spriteAnimation->getPoints())
            {
                file << "\t\t Point: {" << std::to_string(p.x) << " " << std::to_string(p.y) << " " << std::to_string(p.z) << " " << std::to_string(p.w) << "}\n";
                file << "\n";
            }


        }
        else
        {
			file << "\t\t AnimationPath: {" << spriteAnimation->getAnimationFolderPath() << "}\n";
        }

        file << "\t}\n";
    }

    if(script)
    {
        file << "\tScript: {\n";
        file << "\t\t Routine: {" << script->getRoutine()->getPath() << "}\n";
        file << "\t}\n";
    }

    if(pointLight)
    {
        file << "\tPointLight: {\n";
        file << "\t\t Pos: {" << std::to_string(pointLight->position.x) << " " << std::to_string(pointLight->position.y) << " " << std::to_string(pointLight->position.z) << "}\n";
        file << "\t\t Color: {" <<std::to_string(pointLight->color.x) << " " << std::to_string(pointLight->color.y) << " " << std::to_string(pointLight->color.z) << "}\n";
        file << "\t}\n";
    }

    if(particles)
    {
        file << "\tParticles: {\n";
        
        file << "\t\tMesh: {";

        file << "\t\t\tPath: {" << particles->particle->getTexturesRef()[0].get_path() << "}\n";

        file << "\t\t}";

        file << "\t}\n";

        file << "\t\tEmitter: {" << std::to_string(particles->emitter.x) << " " << std::to_string(particles->emitter.y) << " " << std::to_string(particles->emitter.z) << "}\n";

        file << "\t\tParticleSize: {" << std::to_string(particles->particle_size.x) << " " << std::to_string(particles->particle_size.y) << " " << std::to_string(particles->particle_size.z) << "}\n";

        file << "\t\tBoundaries: {" << std::to_string(particles->minBound) << " " << std::to_string(particles->maxBound) << "0" << "}\n";

        file << "\t\tParticlesAmount: {" << std::to_string(particles->amount) << "}\n";
    }

    file << "\n\t\t}\n";
}

