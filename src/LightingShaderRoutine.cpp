#include <LightingShaderRoutine.h>
#include <Renderer.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void LightingShaderRoutine::operator() (Object* obj){
    //glUseProgram(sv.getProgram());
    sv = Renderer::shaderLibInstance->getCurrShader();
    glUseProgram(sv.getProgram());

	sv.setVec3("viewPos", GameState::cam.getCameraPos());
    sv.setInt("lightsCount", PointLight::LightsCount);
    sv.setFloat("gammaFactor", 1); 

    //std::cout << "ID: " << obj->getID() << "\n";
    int id = obj->getID() * 25500;
    float r = ((id) & 0x000000FF) >> 0;
    float b = ((id) & 0x0000FF00) >> 8;
    float g = (id & 0x00FF0000) >> 16;

    sv.setVec4("PickingColor", glm::vec4{r/255,g/255,b/255,0});

    /*glm::vec3 lightPos = glm::vec3(-2.0f, 0.0f, -1.0f);
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    float near_plane = 1.0f, far_plane = 7.5f;
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;
    */

    sv.setMat4("lightSpaceMatrix", DirectionalLight::getLightMat());

    glm::mat4 projection = GameState::cam.getPerspective(1600, 900);
    glm::mat4 view = (GameState::cam.getBasicLook());

    sv.setVec3("lightPos", DirectionalLight::lightPos);
    sv.setMat4("projection", projection);
    sv.setMat4("view", view);

    //if(directionalLight)
    //    directionalLight->setShaderLight(sv);
    //if(pointLight)
    //    pointLight->setShaderLight(sv);

    glm::mat4 model = glm::mat4(1.0f);
    glm::vec3 pos = obj->getTransform().position;
    glm::mat4 q = obj->getTransform().get_quatmat();
    glm::vec3 scale = obj->getTransform().scale;


    //TODO(darius) add distanced rendering here. NOTE(darius) Not that hard
    float dist = glm::length(GameState::cam.getCameraPos() - pos);
    if(dist > 99) 
    {
        scale *= 0.5;//2 * GameState::cam.getFov()/dist;
        pos = GameState::cam.getCameraPos() + ((GameState::cam.getCameraPos() - pos));
    }


    model = glm::translate(model, pos);
    model = glm::scale(model, scale);
    model *= q;

    sv.setMat4("model", model);

    /*if (Renderer::shaderLibInstance->stage == ShaderLibrary::STAGE::SHADOWS)
    {
        std::cout << "suka\n";
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, Renderer::shaderLibInstance->depthMap);
    }
    */
}
