#include <PT.h>
#include "Renderer.h"

namespace PT::Renderer
{
	namespace
	{
		Window* window;

		VAO outputVAO;
		Image outputImg;
		Image accumulatorImg;

		GLBuffer uniformBuffer, dispatchBuffer, atomicBuffer, extend_buffer, shadowBuffer, hitBuffer, pathBuffer;
		GLBuffer sceneBuffer;
		GLBuffer meshBuffer;

		GLuint in_offset = 0;
		GLuint out_offset = MAX_WIDTH * MAX_HEIGHT;
		GLBuffer uniform_swap;

		ComputeShader generateKernel;
		ComputeShader extendKernel;
		ComputeShader shadeKernel;
		ComputeShader connectKernel;
		ComputeShader imageKernel;
		PixelShader outputKernel;

		AccumulatorProfiler accProfiler;
		uint32_t frame = 0;

		Scene* scene;
	}

	void Init(const Settings& settings, Window& t_window)
	{
		window = &t_window;
		
		// === Program shaders ===
		generateKernel.ComputeShaderProgram("src/shaders/generate.glsl");
		extendKernel.ComputeShaderProgram("src/shaders/extend.glsl");
		shadeKernel.ComputeShaderProgram("src/shaders/shade.glsl");
		connectKernel.ComputeShaderProgram("src/shaders/connect.glsl");
		imageKernel.ComputeShaderProgram("src/shaders/image.glsl");
		outputKernel.PixelShaderProgram("src/shaders/output.glsl");

		// === Render target textures ===
		outputVAO.Init();
		outputImg.Init(settings.videoSettings.width, settings.videoSettings.height);
		outputImg.LoadData(GL_RGBA32F, GL_RGBA, GL_FLOAT, nullptr);
		outputImg.BindTextureUnit(GL_RGBA32F, GL_WRITE_ONLY, OUTPUT_TEX_BINDING);
		accumulatorImg.Init(settings.videoSettings.width, settings.videoSettings.height);
		accumulatorImg.LoadData(GL_RGBA32F, GL_RGBA, GL_FLOAT, nullptr);
		accumulatorImg.BindTextureUnit(GL_RGBA32F, GL_WRITE_ONLY, ACCUMULATOR_TEX_BINDING);

		// === Buffers ===
		uniformBuffer.InitBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
		dispatchBuffer.InitBuffer(GL_DISPATCH_INDIRECT_BUFFER, GL_DYNAMIC_DRAW);
		atomicBuffer.InitBuffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);

		uniform_swap.InitBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
		extend_buffer.InitBuffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);

		shadowBuffer.InitBuffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);
		hitBuffer.InitBuffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);
		pathBuffer.InitBuffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);

		uniformBuffer.InitData(sizeof(Uniforms), 1, UNIFORM_BUFFER_BINDING_INDEX);
		dispatchBuffer.InitData(sizeof(uint32_t), 3, DISPATCH_BUFFER_BINDING_INDEX);
		atomicBuffer.InitData(sizeof(Atomics), 1, ATOMIC_BUFFER_BINDING_INDEX);

		extend_buffer.InitData(sizeof(RayBuffer), MAX_WIDTH * MAX_HEIGHT * 2, 3);
		uniform_swap.InitData(sizeof(uint32_t), 2, 4);
		
		// TODO: Refactor
		shadowBuffer.InitData(sizeof(RayBuffer), MAX_WIDTH * MAX_HEIGHT, SHADOW_BUFFER_BINDING_INDEX);
		hitBuffer.InitData(76, MAX_WIDTH * MAX_HEIGHT, HIT_BUFFER_BINDING_INDEX);
		pathBuffer.InitData(84, MAX_WIDTH * MAX_HEIGHT, PATH_BUFFER_BINDING_INDEX);

		SetEventCallback(EventType::ResetAccumulator, Renderer::OnEvent);

		LoadScene("resources/scenes/default.scene");
	}

	void Update()
	{
		ResetAccumulator();
		ResetWorkBuffers();
		SetDynamicUniforms();

		atomicBuffer.Bind();
		dispatchBuffer.Bind();
		outputImg.Bind();

		generateKernel.Use();
		generateKernel.Dispatch(GetNumWorkGroups(outputImg.GetWidth() * outputImg.GetHeight()), 1, 1);
		generateKernel.Barrier(GL_SHADER_STORAGE_BARRIER_BIT);
		glCopyBufferSubData(GL_SHADER_STORAGE_BUFFER, GL_DISPATCH_INDIRECT_BUFFER, offsetof(Atomics, extendWorkGroup), 0, sizeof(uint32_t));

		atomicBuffer.Unbind();

		for (uint32_t i = 0; i < MAX_BOUNCES; ++i)
		{
			atomicBuffer.Bind();

			extendKernel.Use();
			glDispatchComputeIndirect(NULL);
			extendKernel.Barrier(GL_SHADER_STORAGE_BARRIER_BIT);
			glCopyBufferSubData(GL_SHADER_STORAGE_BUFFER, GL_DISPATCH_INDIRECT_BUFFER, offsetof(Atomics, shadeWorkGroup), 0, sizeof(uint32_t));
			atomicBuffer.LoadData(1, offsetof(Atomics, extendWorkGroup)); 
			atomicBuffer.LoadData(0, offsetof(Atomics, extendThreadCounter));

			shadeKernel.Use();
			shadeKernel.SetUniformUInt("u_depth", i);
			glDispatchComputeIndirect(NULL);
			shadeKernel.Barrier(GL_SHADER_STORAGE_BARRIER_BIT);
			glCopyBufferSubData(GL_SHADER_STORAGE_BUFFER, GL_DISPATCH_INDIRECT_BUFFER, offsetof(Atomics, connectWorkGroup), 0, sizeof(uint32_t));
			atomicBuffer.LoadData(1, offsetof(Atomics, shadeWorkGroup));
			atomicBuffer.LoadData(0, offsetof(Atomics, shadeThreadCounter));

			connectKernel.Use();
			glDispatchComputeIndirect(NULL);
			connectKernel.Barrier(GL_SHADER_STORAGE_BARRIER_BIT);
			glCopyBufferSubData(GL_SHADER_STORAGE_BUFFER, GL_DISPATCH_INDIRECT_BUFFER, offsetof(Atomics, extendWorkGroup), 0, sizeof(uint32_t));
			atomicBuffer.LoadData(1, offsetof(Atomics, connectWorkGroup));
			atomicBuffer.LoadData(0, offsetof(Atomics, connectThreadCounter));

			atomicBuffer.Unbind();
			SwapBuffers();
		}

		imageKernel.Use();
		imageKernel.Dispatch(GetNumWorkGroups(outputImg.GetWidth() * outputImg.GetHeight()), 1, 1);
		imageKernel.Barrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		outputKernel.Use();
		glDrawArrays(GL_TRIANGLES, 0, 6);

		dispatchBuffer.Unbind();
		outputImg.Unbind();
	}

	void Shutdown()
	{
		delete scene;
	}

	namespace
	{
		void SetDynamicUniforms()
		{
			uniformBuffer.Bind();
			uniformBuffer.LoadData(scene->camera->GetView(), offsetof(Uniforms, camView));
			uniformBuffer.LoadData(scene->camera->GetWorldPosition(), offsetof(Uniforms, camWorldPos));
			uniformBuffer.LoadData(scene->camera->GetFieldOfView(), offsetof(Uniforms, FOV));
			uniformBuffer.LoadData(++frame, offsetof(Uniforms, frame));
			uniformBuffer.Unbind();
		}

		void ResetWorkBuffers()
		{
			atomicBuffer.Bind();
			dispatchBuffer.Bind();

			// TODO: create this wrapper
			uint32_t ones[3]{ 1, 1, 1 };
			glBufferSubData(GL_DISPATCH_INDIRECT_BUFFER, 0, sizeof(uint32_t) * 3, &ones[0]);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER,	 0, sizeof(uint32_t) * 3, &ones[0]);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER,	12, sizeof(uint32_t) * 3, (void*)0);

			atomicBuffer.Unbind();
			dispatchBuffer.Unbind();
		}

		void ResetAccumulator()
		{
			if (accProfiler.reset)
			{
				if (accProfiler.resetTimer)
					accProfiler.totalTime = 0.0;

				double currentTime = glfwGetTime();
				double deltaTime = currentTime - accProfiler.lastTime;
				accProfiler.lastTime = currentTime;
				accProfiler.totalTime += deltaTime;

				if (accProfiler.totalTime > accProfiler.timeLimit)
				{
					accProfiler.reset = false;
					accProfiler.totalTime = 0.0;
					imageKernel.Use();
					imageKernel.SetUniformBool("u_resetAccumulator", false);
				}

				accProfiler.resetTimer = false;
			}
		}

		void SwapBuffers()
		{
			std::swap(in_offset, out_offset);
			uniform_swap.Bind();
			uniform_swap.LoadData(in_offset, 0);
			uniform_swap.LoadData(out_offset, 4);
			uniform_swap.Unbind();
		}

		void OnEvent(Event* e)
		{
			switch (e->type)
			{
				case EventType::ResetAccumulator:
				{
					auto resetEvent = static_cast<ResetAccumulatorEvent*>(e);
					accProfiler.lastTime = resetEvent->time;
					accProfiler.reset = true;
					accProfiler.resetTimer = true;
					imageKernel.Use();
					imageKernel.SetUniformBool("u_resetAccumulator", true);
					break;
				}
				default:
					LOG_WARNING("Renderer doesn't support this kind of event.\n");
				}
		}

		// === Load scene data into GPU memory ===
		void LoadScene(const std::string& filePath)
		{
			scene = new Scene(filePath);

			// Camera
			scene->camera->SetResolution(outputImg.GetWidth(), outputImg.GetHeight());
		
			// HDRI texture
			scene->HDRItexture.BindTextureUnit(GL_RGBA16F, GL_READ_ONLY, SCENE_TEX_BINDING);
			shadeKernel.Use();
			shadeKernel.SetUniformInt("u_HDRI", 2);

			// Scene uniforms
			uniformBuffer.Bind();
			uniformBuffer.LoadData(scene->camera->GetView(),		  offsetof(Uniforms, camView));
			uniformBuffer.LoadData(scene->camera->GetWorldPosition(), offsetof(Uniforms, camWorldPos));
			uniformBuffer.LoadData(scene->camera->GetFieldOfView(),	  offsetof(Uniforms, FOV));
			uniformBuffer.LoadData(scene->sphereLights.size(),		  offsetof(Uniforms, nSphereLights));
			uniformBuffer.LoadData(scene->spheres.size(),			  offsetof(Uniforms, nSpheres));
			uniformBuffer.LoadData(scene->models.size(),			  offsetof(Uniforms, nModels));
			uniformBuffer.Unbind();

			// TODO: Improve
			sceneBuffer.InitBuffer(GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW);
			sceneBuffer.InitData(sceneBufferSize, 1, SCENE_BUFFER_BINDING_INDEX);

			size_t offset = 0;
			sceneBuffer.Bind();
			sceneBuffer.LoadData(scene->materials.front(),	  size_t(offset), scene->materials.size());	   offset += sizeof(Material) * MAX_MATERIALS;
			sceneBuffer.LoadData(scene->spheres.front(),	  size_t(offset), scene->spheres.size());	   offset += sizeof(Sphere) * MAX_SPHERES;
			sceneBuffer.LoadData(scene->sphereLights.front(), size_t(offset), scene->sphereLights.size()); offset += sizeof(SphereLight) * MAX_SPHERE_LIGHTS;

			for (size_t i = 0; i < scene->models.size(); ++i) 
			{
				sceneBuffer.LoadData(scene->models[i].gpuTriangles.front(), size_t(offset), scene->models[i].gpuTriangles.size());
				offset += sizeof(GPUTriangle) * MAX_TRIANGLES;
				sceneBuffer.LoadData(scene->models[i].gpuNodes.front(), size_t(offset), scene->models[i].gpuNodes.size());
				offset += sizeof(GPUBVHNode) * MAX_NODES;
				sceneBuffer.LoadData(scene->models[i].matid, size_t(offset), sizeof(uint32_t));
				offset += sizeof(uint32_t) * 4;
			}

			sceneBuffer.Unbind();
		}
	}
}