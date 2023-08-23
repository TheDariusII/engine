#include <ParticleSystem.h>
#include <Object.h>

void ParticleSystem::setEmitter(glm::vec3 emitter_in)
{
	emitter = emitter_in;
}

void ParticleSystem::addParticle(FlatMesh&& m, Shader&& shader_in, LightingShaderRoutine&& shaderRoutine_in, glm::vec3 particleSize) noexcept
{
    shader = shader_in;
    shaderRoutine = shaderRoutine_in;
    particle.emplace(std::move(m));

    //particle.emplace(m, shader_in, shaderRoutine_in);

    std::vector<glm::mat4> modelMatrices;
    modelMatrices.resize(amount);
    srand(static_cast<unsigned int>(glfwGetTime())); // initialize random seed
    float radius = 150.0;
    float offset = 25.0f;
    for (int i = 0; i < amount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        // 1. translation: displace along circle with 'radius' in range [-offset, offset]
        /*float angle = (float)i / (float)amount * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float y = displacement * 0.4f; // keep height of asteroid field smaller compared to width of x and z
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));

        // 2. scale: Scale between 0.05 and 0.25f
        float scale = static_cast<float>((rand() % 20) / 100.0 + 0.05);
        model = glm::scale(model, glm::vec3(scale));

        // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
        float rotAngle = static_cast<float>((rand() % 360));
        model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));
        */

        // 4. now add to list of matrices
        modelMatrices[i] = model;
    }

    for(int i = 0; i < modelMatrices.size(); ++i)
    {
        particlesTransfroms.push_back(Transform(modelMatrices[i]));
    }

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);


    //NOTE(darius) looks like problem is here
    unsigned int VAO = particle->getVao().get();
    glBindVertexArray(VAO);
    // set attribute pointers for matrix (4 times vec4)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);

    glBindVertexArray(0);
}

void ParticleSystem::changeShader()
{
    //change shader here
}

void ParticleSystem::addPosition(glm::vec3 pos)
{
    positions.push_back(pos);
}

void ParticleSystem::addPositions(size_t n)
{
    for(int i = 0; i < n; ++i)
        addPosition(glm::vec3{i,i,i});
}

//https://www.youtube.com/watch?v=Y0Ko0kvwfgA&ab_channel=Acerola
void ParticleSystem::renderParticles()
{
    //if(positions.size() < 3)
     //   return;

    //if(particleMaterial)
    //    particleMaterial->setShaderMaterial(*shader);

    /*for (auto& p : positions)
    {
        shaderRoutine->operator()(Transform(p, particle_size));
        //shader->setVec3("aOffset", glm::vec3{0,0,0});
        //particle->Draw(*shader, positions.size());
    }
    */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);   
    Object tmpObj("tmp");

    auto comp = [](glm::vec3 a, glm::vec3 b){return a.z < b.z;};
    std::sort(positions.begin(), positions.end(), comp);

    for(int i = 0; i < particlesTransfroms.size(); ++i)
    {
        if(positions.size() > i){
            particlesTransfroms[i].setPosition(positions[i]);
        }
    }

    int c = 0;
    for (auto& m : particlesTransfroms)
    {
        if(c++ > positions.size()-1)
            break;
        //tmpObj.getTransform().setMatrix(m);
        glm::vec3 pos = m.getPosition();
        tmpObj.getTransform() = Transform(glm::vec3(pos.x, pos.y, pos.z), particle_size);
        shaderRoutine->operator()(&tmpObj);
        particle->Draw(*shader);
    }

    /*tmpObj.getTransform() = Transform(glm::vec3{0,0,0}, particle_size);
    shaderRoutine->operator()(&tmpObj);
	particle->Draw(*shader);

    tmpObj.getTransform() = Transform(glm::vec3{1,1,1}, particle_size);
    shaderRoutine->operator()(&tmpObj);
	particle->Draw(*shader);
    */

    /*tmpObj.getTransform() = particlesTransfroms[0];
    shaderRoutine->operator()(&tmpObj);
    particle->Draw(*shader);
    */

    glDisable(GL_BLEND);

}

void ParticleSystem::updateUniform3DDistribution(float timeValue)
{
    if(pause)
        return;

    if (timeValue - lastTime > 1) {
        lastTime = timeValue;
        for (auto& position : positions)
        {
            int max = static_cast<int>(maxBound) * 100;
            int min = static_cast<int>(minBound) * 100;
            float v1 = static_cast<float>((rand() % (max - min)) + min);
            float v2 = static_cast<float>((rand() % (max - min)) + min);
            float v3 = static_cast<float>((rand() % (max - min)) + min);
            position = {v1/100,v2/100,v3/100};
        }
    }		
}

void ParticleSystem::updateSpawnFromEmitter(float timeValue)
{
    if(pause)
        return;

    if (timeValue - lastTime > 1) {
        lastTime = timeValue;
        for (auto& p: positions)
        {
            int max = static_cast<int>(maxBound);
            int min = static_cast<int>(minBound);
            int v1 = (rand() % (max - min)) + min;
            int v2 = (rand() % (max - 0));
            int v3 = (rand() % (max - min)) + min;

            p = emitter;
        }
    }		
}

void ParticleSystem::update(float timeValue)
{
    if(pause)
        return;
    updateUniform3DDistribution(timeValue);
}