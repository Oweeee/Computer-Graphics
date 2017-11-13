
#include <GL/glew.h>
#include <stb_image.h>
#include <chrono>
#include <iostream>
#include <labhelper.h>
#include <imgui.h>
#include <imgui_impl_sdl_gl3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <Model.h>

using namespace glm;
using namespace std; 

///////////////////////////////////////////////////////////////////////////////
// Various globals
///////////////////////////////////////////////////////////////////////////////
SDL_Window* g_window = nullptr;
float currentTime = 0.0f;
int windowWidth, windowHeight;

///////////////////////////////////////////////////////////////////////////////
// Shader programs
///////////////////////////////////////////////////////////////////////////////
GLuint shaderProgram;
GLuint backgroundProgram; 

///////////////////////////////////////////////////////////////////////////////
// Environment
///////////////////////////////////////////////////////////////////////////////
float environment_multiplier = 1.0f;
GLuint environmentMap;
GLuint irradianceMap; 
GLuint reflectionMap; 
const std::string envmap_base_name = "001";

///////////////////////////////////////////////////////////////////////////////
// Light source
///////////////////////////////////////////////////////////////////////////////
float point_light_intensity_multiplier = 1000.0f; 
vec3 point_light_color = vec3(1.f, 1.f, 1.f);


///////////////////////////////////////////////////////////////////////////////
// Camera parameters.
///////////////////////////////////////////////////////////////////////////////
vec3 cameraPosition(-30.0f, 10.0f, 30.0f);
vec3 cameraDirection = normalize(vec3(0.0f) - cameraPosition);
vec3 worldUp(0.0f, 1.0f, 0.0f);

///////////////////////////////////////////////////////////////////////////////
// Models
///////////////////////////////////////////////////////////////////////////////
const std::string model_filename = "../scenes/NewShip.obj";
//const std::string model_filename = "../scenes/BigSphere.obj";
labhelper::Model *fighterModel = nullptr;
labhelper::Model *sphereModel = nullptr;

///////////////////////////////////////////////////////////////////////////////
// The load shaders function is called once from initialize() and then 
// whenever you press the Reload Shaders button
///////////////////////////////////////////////////////////////////////////////
void loadShaders(bool is_reload)
{
	GLuint shader = labhelper::loadShaderProgram("../lab4-cubemapping/simple.vert", "../lab4-cubemapping/simple.frag", is_reload);
	if (shader != 0) shaderProgram = shader; 
	shader = labhelper::loadShaderProgram("../lab4-cubemapping/background.vert", "../lab4-cubemapping/background.frag", is_reload);
	if (shader != 0) backgroundProgram = shader;
}

///////////////////////////////////////////////////////////////////////////////
// Load shaders, environment maps, models and so on
///////////////////////////////////////////////////////////////////////////////
void initialize()
{
	///////////////////////////////////////////////////////////////////////////
	// Load shaders first time. Do not allow errors. 
	///////////////////////////////////////////////////////////////////////////
	loadShaders(false);

	///////////////////////////////////////////////////////////////////////////
	// Local helper struct for loading HDR images
	///////////////////////////////////////////////////////////////////////////
	struct HDRImage {
		int width, height, components; 
		float * data = nullptr; 
		// Constructor
		HDRImage(const string & filename) {
			stbi_set_flip_vertically_on_load(false);
			data = stbi_loadf(filename.c_str(), &width, &height, &components, 3);
			stbi_set_flip_vertically_on_load(true);
			if (data == NULL) {
				std::cout << "Failed to load image: " << filename << ".\n";
				exit(1);
			}
		};
		// Destructor
		~HDRImage() {
			stbi_image_free(data);
		};
	};

	///////////////////////////////////////////////////////////////////////////
	// Load environment map
	// NOTE: You can safely ignore this until you start Task 4.
	///////////////////////////////////////////////////////////////////////////
	{ // Environment map
		HDRImage image("../scenes/envmaps/" + envmap_base_name + ".hdr");
		glGenTextures(1, &environmentMap);
		glBindTexture(GL_TEXTURE_2D, environmentMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, image.width, image.height, 0, GL_RGB, GL_FLOAT, image.data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	{ // Irradiance map
		HDRImage image("../scenes/envmaps/" + envmap_base_name + "_irradiance.hdr");
		glGenTextures(1, &irradianceMap);
		glBindTexture(GL_TEXTURE_2D, irradianceMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, image.width, image.height, 0, GL_RGB, GL_FLOAT, image.data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	{ // Reflection map
		glGenTextures(1, &reflectionMap);
		glBindTexture(GL_TEXTURE_2D, reflectionMap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		const int roughnesses = 8;
		for (int i = 0; i < roughnesses; i++) {
			HDRImage image("../scenes/envmaps/" + envmap_base_name + "_dl_" + std::to_string(i) + ".hdr");
			glTexImage2D(GL_TEXTURE_2D, i, GL_RGB32F, image.width, image.height, 0, GL_RGB, GL_FLOAT, image.data);
			if (i == 0) { glGenerateMipmap(GL_TEXTURE_2D); }
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// Load .obj models
	///////////////////////////////////////////////////////////////////////////
	fighterModel = labhelper::loadModelFromOBJ(model_filename);
	sphereModel = labhelper::loadModelFromOBJ("../scenes/sphere.obj");
}

void debugDrawLight(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix, const glm::vec3 &worldSpaceLightPos)
{
	mat4 modelMatrix = glm::translate(worldSpaceLightPos);
	labhelper::setUniformSlow(shaderProgram, "modelViewProjectionMatrix", projectionMatrix * viewMatrix * modelMatrix);
	labhelper::render(sphereModel);
	labhelper::setUniformSlow(shaderProgram, "modelViewProjectionMatrix", projectionMatrix * viewMatrix);
	labhelper::debugDrawLine(viewMatrix, projectionMatrix, worldSpaceLightPos);
}

void display(void)
{
	///////////////////////////////////////////////////////////////////////////
	// Set up OpenGL stuff
	///////////////////////////////////////////////////////////////////////////
	glViewport(0, 0, windowWidth, windowHeight);
	glClearColor(0.1,0.1,0.6,1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	SDL_GetWindowSize(g_window, &windowWidth, &windowHeight);

	///////////////////////////////////////////////////////////////////////////
	// Bind the environment map(s) to unused texture units
	///////////////////////////////////////////////////////////////////////////
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, environmentMap);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, irradianceMap);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, reflectionMap);

	///////////////////////////////////////////////////////////////////////////
	// Set up the view and projection matrix for the camera
	///////////////////////////////////////////////////////////////////////////
	mat4 viewMatrix, projectionMatrix;
	{
		vec3 cameraRight = normalize(cross(cameraDirection, worldUp));
		vec3 cameraUp = normalize(cross(cameraRight, cameraDirection));
		mat3 cameraBaseVectorsWorldSpace(cameraRight, cameraUp, -cameraDirection);
		mat4 cameraRotation = mat4(transpose(cameraBaseVectorsWorldSpace));
		viewMatrix = cameraRotation * translate(-cameraPosition);
		projectionMatrix = perspective(radians(45.0f), float(windowWidth) / float(windowHeight), 0.01f, 300.0f);
	}

	///////////////////////////////////////////////////////////////////////////
	// Task 4 - Render a fullscreen quad, to generate the background from the 
	//          environment map. 
	///////////////////////////////////////////////////////////////////////////
	glUseProgram(backgroundProgram);
	labhelper::setUniformSlow(backgroundProgram, "environment_multiplier", environment_multiplier);
	labhelper::setUniformSlow(backgroundProgram, "inv_PV", inverse(projectionMatrix * viewMatrix));
	labhelper::setUniformSlow(backgroundProgram, "camera_pos", cameraPosition);
	labhelper::drawFullScreenQuad();
	///////////////////////////////////////////////////////////////////////////
	// Render the .obj models
	///////////////////////////////////////////////////////////////////////////
	glUseProgram( shaderProgram );
	// The model matrix is just identity
	mat4 modelMatrix(1.0f);
	// Matrices
	labhelper::setUniformSlow(shaderProgram, "modelViewProjectionMatrix", projectionMatrix * viewMatrix * modelMatrix);
	labhelper::setUniformSlow(shaderProgram, "viewInverse", inverse(viewMatrix));
	labhelper::setUniformSlow(shaderProgram, "modelViewMatrix", viewMatrix * modelMatrix);
	labhelper::setUniformSlow(shaderProgram, "normalMatrix", inverse(transpose(viewMatrix * modelMatrix)));
	// Light source
	vec4 lightStartPosition = vec4(20.0f, 20.0f, 0.0f, 1.0f);
	vec4 lightPosition = rotate(currentTime, worldUp) * lightStartPosition;
	vec4 viewSpaceLightPosition = viewMatrix * lightPosition;
	labhelper::setUniformSlow(shaderProgram, "point_light_color", point_light_color);
	labhelper::setUniformSlow(shaderProgram, "point_light_intensity_multiplier", point_light_intensity_multiplier);
	labhelper::setUniformSlow(shaderProgram, "viewSpaceLightPosition", vec3(viewSpaceLightPosition));
	// Environment
	labhelper::setUniformSlow(shaderProgram, "environment_multiplier", environment_multiplier);

	// Render the main object
	labhelper::render(fighterModel); 
	// Render the light source
	debugDrawLight(viewMatrix, projectionMatrix, vec3(lightPosition));

	glUseProgram( 0 );	
}

bool handleEvents(void)
{
	// check events (keyboard among other)
	SDL_Event event;
	bool quitEvent = false;

	// Allow ImGui to capture events.
	ImGuiIO& io = ImGui::GetIO();

	while (SDL_PollEvent(&event)) {
		ImGui_ImplSdlGL3_ProcessEvent(&event);

		if (event.type == SDL_QUIT || (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)) {
			quitEvent = true;
		}
		
		if (!io.WantCaptureMouse) {
			if (event.type == SDL_MOUSEMOTION) {
				static int prev_xcoord = event.motion.x;
				static int prev_ycoord = event.motion.y;
				int delta_x = event.motion.x - prev_xcoord;
				int delta_y = event.motion.y - prev_ycoord;
				if (event.button.button & SDL_BUTTON(SDL_BUTTON_LEFT)) {
					float rotationSpeed = 0.005f;
					mat4 yaw = rotate(rotationSpeed * -delta_x, worldUp);
					mat4 pitch = rotate(rotationSpeed * -delta_y, normalize(cross(cameraDirection, worldUp)));
					cameraDirection = vec3(pitch * yaw * vec4(cameraDirection, 0.0f));
				}
				prev_xcoord = event.motion.x;
				prev_ycoord = event.motion.y;
			}
		}
	}

	if (!io.WantCaptureKeyboard)
	{
		// check keyboard state (which keys are still pressed)
		const uint8_t *state = SDL_GetKeyboardState(nullptr);
		vec3 cameraRight = cross(cameraDirection, worldUp);
		if (state[SDL_SCANCODE_W]) {
			cameraPosition += 0.2f * cameraDirection;
		}
		if (state[SDL_SCANCODE_S]) {
			cameraPosition -= 0.2f * cameraDirection;
		}
		if (state[SDL_SCANCODE_A]) {
			cameraPosition -= 0.2f * cameraRight;
		}
		if (state[SDL_SCANCODE_D]) {
			cameraPosition += 0.2f * cameraRight;
		}
		if (state[SDL_SCANCODE_Q]) {
			cameraPosition -= 0.2f * worldUp;
		}
		if (state[SDL_SCANCODE_E]) {
			cameraPosition += 0.2f * worldUp;
		}
	}

	return quitEvent;
}

void gui() {
	// Inform imgui of new frame
	ImGui_ImplSdlGL3_NewFrame(g_window);

	///////////////////////////////////////////////////////////////////////////
	// Helpers for getting lists of materials and meshes into widgets
	///////////////////////////////////////////////////////////////////////////
	static auto mesh_getter = [](void * vec, int idx, const char ** text){
		auto& vector = *static_cast<std::vector<labhelper::Mesh>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*text = vector[idx].m_name.c_str();
		return true;
	};

	static auto material_getter = [](void * vec, int idx, const char ** text){
		auto& vector = *static_cast<std::vector<labhelper::Material>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*text = vector[idx].m_name.c_str();
		return true;
	};

	///////////////////////////////////////////////////////////////////////////
	// List all meshes in the model and show properties for the selected
	///////////////////////////////////////////////////////////////////////////
	static int mesh_index = 0;
	static int material_index = fighterModel->m_meshes[mesh_index].m_material_idx;

	if (ImGui::CollapsingHeader("Meshes", "meshes_ch", true, true))
	{
		if (ImGui::ListBox("Meshes", &mesh_index, mesh_getter, 
			(void*)&fighterModel->m_meshes, fighterModel->m_meshes.size(), 8)) {
			material_index = fighterModel->m_meshes[mesh_index].m_material_idx;
		}

		labhelper::Mesh & mesh = fighterModel->m_meshes[mesh_index];
		char name[256];
		strcpy(name, mesh.m_name.c_str());
		if (ImGui::InputText("Mesh Name", name, 256)) { mesh.m_name = name; }
		labhelper::Material & selected_material = fighterModel->m_materials[material_index];
		if (ImGui::Combo("Material", &material_index, material_getter, 
			(void *)&fighterModel->m_materials, fighterModel->m_materials.size())) {
			mesh.m_material_idx = material_index;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// List all materials in the model and show properties for the selected
	///////////////////////////////////////////////////////////////////////////
	if (ImGui::CollapsingHeader("Materials", "materials_ch", true, true))
	{
		ImGui::ListBox("Materials", &material_index, material_getter, 
			(void*)&fighterModel->m_materials, fighterModel->m_materials.size(), 8);
		labhelper::Material & material = fighterModel->m_materials[material_index];
		char name[256];
		strcpy(name, material.m_name.c_str());
		if (ImGui::InputText("Material Name", name, 256)) { material.m_name = name; }
		ImGui::ColorEdit3("Color", &material.m_color.x);
		ImGui::SliderFloat("Reflectivity", &material.m_reflectivity, 0.0f, 1.0f);
		ImGui::SliderFloat("Metalness", &material.m_metalness, 0.0f, 1.0f);
		ImGui::SliderFloat("Fresnel", &material.m_fresnel, 0.0f, 1.0f);
		ImGui::SliderFloat("shininess", &material.m_shininess, 0.0f, 25000.0f);
		ImGui::SliderFloat("Emission", &material.m_emission, 0.0f, 10.0f);
	}

	///////////////////////////////////////////////////////////////////////////
	// Light and environment map
	///////////////////////////////////////////////////////////////////////////
	if (ImGui::CollapsingHeader("Light sources", "lights_ch", true, true))
	{
		ImGui::SliderFloat("Environment multiplier", &environment_multiplier, 0.0f, 10.0f);
		ImGui::ColorEdit3("Point light color", &point_light_color.x);
		ImGui::SliderFloat("Point light intensity multiplier", &point_light_intensity_multiplier, 0.0f, 1000.0f);
	}

	///////////////////////////////////////////////////////////////////////////
	// A button for saving your results
	///////////////////////////////////////////////////////////////////////////
	if (ImGui::Button("Save Materials")) {
		labhelper::saveModelToOBJ(fighterModel, model_filename); 
	}

	///////////////////////////////////////////////////////////////////////////
	// A button for reloading the shaders
	///////////////////////////////////////////////////////////////////////////
	if (ImGui::Button("Reload Shaders")) {
		loadShaders(true); 
	}

	// Render the GUI.
	ImGui::Render();
}

int main(int argc, char *argv[])
{
	g_window = labhelper::init_window_SDL("OpenGL Lab 4", 1280, 720);

	initialize();

	bool stopRendering = false;
	auto startTime = std::chrono::system_clock::now();

	while (!stopRendering) {
		//update currentTime
		std::chrono::duration<float> timeSinceStart = std::chrono::system_clock::now() - startTime;
		currentTime = timeSinceStart.count();
		//currentTime = 5.0f;
		// render to window
		display();

		// Then render overlay GUI.
		gui();

		// Swap front and back buffer. This frame will now be displayed.
		SDL_GL_SwapWindow(g_window);  

		// check events (keyboard among other)
		stopRendering = handleEvents();
	}

	// Delete Models
	labhelper::freeModel(sphereModel);
	labhelper::freeModel(fighterModel);
	// Shut down everything. This includes the window and all other subsystems.
	labhelper::shutDown(g_window);
	return 0;          
}
