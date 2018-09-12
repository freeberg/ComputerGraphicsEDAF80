#include "config.hpp"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/Log.h"
#include "core/LogView.h"
#include "core/Misc.h"
#include "core/node.hpp"
#include "core/ShaderProgramManager.hpp"
#include "core/WindowManager.hpp"

#include <imgui.h>
#include <external/imgui_impl_glfw_gl3.h>

#include <stack>
#include <math.h>

#include <cstdlib>


int main()
{
	//
	// Set up the logging system
	//
	Log::Init();
	Log::View::Init();

	//
	// Set up the camera
	//
	InputHandler input_handler;
	FPSCameraf camera(0.5f * glm::half_pi<float>(),
	                  static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	                  0.01f, 1000.0f);
	camera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 6.0f));
	camera.mMouseSensitivity = 0.003f;
	camera.mMovementSpeed = 0.25f * 12.0f;

	//
	// Set up the windowing system and create the window
	//
	WindowManager window_manager;
	WindowManager::WindowDatum window_datum{ input_handler, camera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};
	GLFWwindow* window = window_manager.CreateWindow("EDAF80: Assignment 1", window_datum, config::msaa_rate);
	if (window == nullptr) {
		LogError("Failed to get a window: exiting.");

		Log::View::Destroy();
		Log::Destroy();

		return EXIT_FAILURE;
	}

	//
	// Load the sphere geometry
	//
	std::vector<bonobo::mesh_data> const objects = bonobo::loadObjects("sphere.obj");
	if (objects.empty()) {
		LogError("Failed to load the sphere geometry: exiting.");

		Log::View::Destroy();
		Log::Destroy();

		return EXIT_FAILURE;
	}
	bonobo::mesh_data const& sphere = objects.front();


	//
	// Create the shader program
	//
	ShaderProgramManager program_manager;
	GLuint shader = 0u;
	program_manager.CreateAndRegisterProgram({ { ShaderType::vertex, "EDAF80/default.vert" },
	                                           { ShaderType::fragment, "EDAF80/default.frag" } },
	                                         shader);
	if (shader == 0u) {
		LogError("Failed to generate the shader program: exiting.");

		Log::View::Destroy();
		Log::Destroy();

		return EXIT_FAILURE;
	}


	//
	// Set up the sun node and other related attributes
	//
	Node sun_node;
	sun_node.set_geometry(sphere);
	GLuint const sun_texture = bonobo::loadTexture2D("teletubbies.png");
	sun_node.add_texture("diffuse_texture", sun_texture, GL_TEXTURE_2D);
	float const sun_spin_speed = glm::two_pi<float>() / 6.0f; // Full rotation in six seconds
	glm::vec3 sun_scale_size = glm::vec3(0.5, 0.5, 0.5);

	Node mercury_node;
	mercury_node.set_geometry(sphere);
	GLuint const mercury_texture = bonobo::loadTexture2D("stone47_diffuse.png");
	mercury_node.add_texture("diffuse_texture", mercury_texture, GL_TEXTURE_2D);
	float const mercury_spin_speed = glm::two_pi<float>() / 6.0f; // Full rotation in six seconds
	glm::vec3 mercury_translation = glm::vec3(2.0, 0, 0);
	glm::vec3 mercury_scale_size = glm::vec3(0.09, 0.09, 0.09);

	Node venus_node;
	venus_node.set_geometry(sphere);
	GLuint const venus_texture = bonobo::loadTexture2D("planet_rick.png");
	venus_node.add_texture("diffuse_texture", venus_texture, GL_TEXTURE_2D);
	float const venus_spin_speed = glm::two_pi<float>() / 6.0f; // Full rotation in six seconds
	glm::vec3 venus_translation = glm::vec3(-3.0, 0, 0);
	glm::vec3 venus_scale_size = glm::vec3(0.1, 0.15, 0.1);

	Node earth_node;
	earth_node.set_geometry(sphere);
	GLuint const earth_texture = bonobo::loadTexture2D("earth_diffuse.png");
	earth_node.add_texture("diffuse_texture", earth_texture, GL_TEXTURE_2D);
	float const earth_spin_speed = glm::two_pi<float>() / 6.0f; // Full rotation in six seconds
	glm::vec3 earth_translation = glm::vec3(-4.0, 0, 0);
	glm::vec3 earth_scale_size = glm::vec3(0.13, 0.13, 0.13);

	Node moon_node;
	moon_node.set_geometry(sphere);
	GLuint const moon_texture = bonobo::loadTexture2D("noise.png");
	moon_node.add_texture("diffuse_texture", moon_texture, GL_TEXTURE_2D);
	float const moon_spin_speed = glm::two_pi<float>() / 6.0f; // Full rotation in six seconds
	glm::vec3 moon_translation = glm::vec3(-5.2, 0, 0);
	glm::vec3 moon_scale_size = glm::vec3(0.01, 0.01, 0.01);

	Node mars_node;
	mars_node.set_geometry(sphere);
	GLuint const mars_texture = bonobo::loadTexture2D("noise.png");
	mars_node.add_texture("diffuse_texture", mars_texture, GL_TEXTURE_2D);
	float const mars_spin_speed = glm::two_pi<float>() / 6.0f; // Full rotation in six seconds
	glm::vec3 mars_translation = glm::vec3(5.0, 0, 0);
	glm::vec3 mars_scale_size = glm::vec3(0.12, 0.12, 0.12);

	Node solar_system_node;
	solar_system_node.add_child(&sun_node);
	solar_system_node.add_child(&mercury_node);
	solar_system_node.add_child(&venus_node);
	solar_system_node.add_child(&earth_node);

	float const system_spin_speed = glm::two_pi<float>() / 9.0f; // Full rotation in six seconds	



	//
	// TODO: Create nodes for the remaining of the solar system
	//


	glViewport(0, 0, config::resolution_x, config::resolution_y);
	glClearDepthf(1.0f);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glEnable(GL_DEPTH_TEST);


	size_t fpsSamples = 0;
	double lastTime = GetTimeSeconds();
	double fpsNextTick = lastTime + 1.0;


	bool show_logs = true;
	bool show_gui = true;

	while (!glfwWindowShouldClose(window)) {
		//
		// Compute timings information
		//
		double const nowTime = GetTimeSeconds();
		double const delta_time = nowTime - lastTime;
		lastTime = nowTime;
		if (nowTime > fpsNextTick) {
			fpsNextTick += 1.0;
			fpsSamples = 0;
		}
		++fpsSamples;


		//
		// Process inputs
		//
		glfwPollEvents();

		ImGuiIO const& io = ImGui::GetIO();
		input_handler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);
		input_handler.Advance();
		camera.Update(delta_time, input_handler);

		if (input_handler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
			show_logs = !show_logs;
		if (input_handler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
			show_gui = !show_gui;


		//
		// Start a new frame for Dear ImGui
		//
		ImGui_ImplGlfwGL3_NewFrame();


		//
		// Clear the screen
		//
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


		//
		// Update the transforms
		//
		sun_node.rotate_y(sun_spin_speed * delta_time);
		sun_node.set_scaling(sun_scale_size);

		mercury_node.rotate_y(mercury_spin_speed * delta_time);
		mercury_node.set_scaling(mercury_scale_size);
		//mercury_node.set_translation(mercury_translation);
		mercury_node.set_translation(glm::vec3(1.0 * cos(nowTime / 0.4), 1.0 * sin(nowTime / 0.4), 0.0));

		venus_node.rotate_y(venus_spin_speed * delta_time);
		venus_node.set_scaling(venus_scale_size);
		//venus_node.set_translation(venus_translation);
		venus_node.set_translation(glm::vec3(1.5 * cos(-(nowTime / 0.6)), 1.5 * sin(-(nowTime / 0.6)), 0.0));

		earth_node.rotate_y(earth_spin_speed * delta_time);
		earth_node.set_scaling(earth_scale_size);
		//earth_node.set_translation(earth_translation);
		earth_node.set_translation(glm::vec3(2.0 * cos(nowTime / 1.2), 2.0 * sin(nowTime / 1.2), 0.0));

		mars_node.rotate_y(mars_spin_speed * delta_time);
		mars_node.set_scaling(mars_scale_size);
		//mars_node.set_translation(mars_translation);
		mars_node.set_translation(glm::vec3(2.5 * cos(-(nowTime / 2)), 2.5 * sin(-(nowTime / 2)), 0.0));

		solar_system_node.rotate_z(system_spin_speed * delta_time);
		solar_system_node.set_translation(earth_translation);

		//
		// Traverse the scene graph and render all nodes
		//
		std::stack<Node const*> node_stack({ &solar_system_node });
		std::stack<glm::mat4> matrix_stack({ glm::mat4(1.0f) });
		// TODO: Replace this explicit rendering of the Sun with a
		// traversal of the scene graph and rendering of all its nodes.
		solar_system_node.render(camera.GetWorldToClipMatrix(), solar_system_node.get_transform(), shader, [](GLuint /*program*/){});
		sun_node.render(camera.GetWorldToClipMatrix(), sun_node.get_transform(), shader, [](GLuint /*program*/){});
		mercury_node.render(camera.GetWorldToClipMatrix(), mercury_node.get_transform(), shader, [](GLuint /*program*/){});
		venus_node.render(camera.GetWorldToClipMatrix(), venus_node.get_transform(), shader, [](GLuint /*program*/){});
		earth_node.render(camera.GetWorldToClipMatrix(), earth_node.get_transform(), shader, [](GLuint /*program*/){});
		mars_node.render(camera.GetWorldToClipMatrix(), mars_node.get_transform(), shader, [](GLuint /*program*/){});



		//
		// Display Dear ImGui windows
		//
		if (show_logs)
			Log::View::Render();
		if (show_gui)
			ImGui::Render();


		//
		// Queue the computed frame for display on screen
		//
		glfwSwapBuffers(window);
	}

	glDeleteTextures(1, &sun_texture);


	Log::View::Destroy();
	Log::Destroy();

	return EXIT_SUCCESS;
}
