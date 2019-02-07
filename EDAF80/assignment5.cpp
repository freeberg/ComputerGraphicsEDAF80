#include "assignment5.hpp"
#include "interpolation.hpp"
#include "parametric_shapes.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/Log.h"
#include "core/LogView.h"
#include "core/Misc.h"
#include "core/ShaderProgramManager.hpp"
#include "core/node.hpp"

#include <imgui.h>
#include <external/imgui_impl_glfw_gl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tinyfiledialogs.h>

#include <stdexcept>

float fou_width = 80.0;
float fou_length = 10.0;
float fou_height = 4.0;
float plat_width = 6.0;
float plat_length = 10.0;
float plat_height = 2.0;

enum class polygon_mode_t : unsigned int {
	fill = 0u,
	line,
	point
};

static polygon_mode_t get_next_mode(polygon_mode_t mode)
{
	return static_cast<polygon_mode_t>((static_cast<unsigned int>(mode) + 1u) % 3u);
}

void
edaf80::Assignment5::super_render(glm::mat4 m, const Node* root, GLuint shader, FPSCameraf& camera)
{	
	for(int i = 0; i < root->get_children_nb(); i++){
		super_render(m*root->get_transform(), root->get_child(i), shader, camera);
	}
	if (root->get_children_nb() == 0){
		root->render(camera.GetWorldToClipMatrix(), m*root->get_transform(), shader, [](GLuint /*program*/){});
	}
}

edaf80::Assignment5::Assignment5() :
	mCamera(0.5f * glm::half_pi<float>(),
	        static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	        0.01f, 1000.0f),
	inputHandler(), mWindowManager(), window(nullptr)
{
	Log::View::Init();

	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};

	window = mWindowManager.CreateWindow("EDAF80: Assignment 5", window_datum, config::msaa_rate);
	if (window == nullptr) {
		Log::View::Destroy();
		throw std::runtime_error("Failed to get a window: aborting!");
	}
}

edaf80::Assignment5::~Assignment5()
{
	Log::View::Destroy();
}

void
edaf80::Assignment5::run()
{

	std::vector<bonobo::mesh_data> const objects = bonobo::loadObjects("bb8.obj");
	if (objects.empty()) {
		LogError("Failed to load the sphere geometry: exiting.");

		Log::View::Destroy();
		Log::Destroy();

		return;
	}
	bonobo::mesh_data const& bb8 = objects.front();


	// Set up the camera
	mCamera.mWorld.SetTranslate(glm::vec3(40.0f, 6.0f, 80.0f));
	mCamera.mMouseSensitivity = 0;//0.012f;
	mCamera.mMovementSpeed = 0;//.01;

	// Create the shader programs
	ShaderProgramManager program_manager;
	auto my_cube_map_id1 = bonobo::loadTextureCubeMap("cloudyhills/posx.png","cloudyhills/negx.png","cloudyhills/posy.png","cloudyhills/negy.png", "cloudyhills/posz.png","cloudyhills/negz.png", true);
	// auto my_cube_map_id2 = bonobo::loadTextureCubeMap("bb8/white_dirt_map.jpg","bb8/white_dirt_map.jpg","bb8/white_dirt_map.jpg","bb8/white_dirt_map.jpg", "bb8/white_dirt_map.jpg","bb8/white_dirt_map.jpg", true);
	// auto my_cube_map_id3 = bonobo::loadTextureCubeMap("/bb8/metal_brushed.jpg","/bb8/metal_brushed.jpg","/bb8/metal_brushed.jpg","/bb8/metal_brushed.jpg", "/bb8/metal_brushed.jpg","/bb8/metal_brushed.jpg", true);

	GLuint found_texture = bonobo::loadTexture2D("stone47_diffuse.png");
	GLuint const plat_texture = bonobo::loadTexture2D("stone47_diffuse.png");

	// std::vector<GLuint const> bb8_textures(objects_size);
	// std::vector<GLuint const> bb8_bumps(objects_size);

	// GLuint const line_bump = bonobo::loadTexture2D("bb8/b8_bump_line.jpg");
	GLuint const body1_texture = bonobo::loadTexture2D("tiles.png");
	GLuint const body_bump = bonobo::loadTexture2D("tiles.png"); // bb8/tiles.png"); // bb8_bump_map.jpg");
	GLuint const body2_texture = bonobo::loadTexture2D("tiles.png"); // bb8/tiles.png"); // bb8_displace_map.jpg");
	GLuint const head_line_bump = bonobo::loadTexture2D("tiles.png"); // bb8/tiles.png"); // bb8_head_bump_line.jpg");
	GLuint const head_bump = bonobo::loadTexture2D("tiles.png"); // bb8/tiles.png"); // bb8_head_bump_map.jpg");
	GLuint const head_texture = bonobo::loadTexture2D("tiles.png"); // bb8/tiles.png"); // bb8_head_diff_map.jpg");
	GLuint const head_ring_bump = bonobo::loadTexture2D("tiles.png"); // bb8/tiles.png"); // bb8_head_ring_bump_line.jpg");
	GLuint const head_ring_texture = bonobo::loadTexture2D("tiles.png"); // bb8/tiles.png"); // bb8_head_ring_diff_map.jpg");
	GLuint const head_top_bump = bonobo::loadTexture2D("tiles.png"); // bb8/tiles.png"); // bb8_head_top_bump_line.jpg");
	GLuint const head_top_texture = bonobo::loadTexture2D("tiles.png"); // bb8/tiles.png"); // bb8_head_top_diff_map.jpg");
	GLuint const self_ill_texture = bonobo::loadTexture2D("tiles.png"); // bb8/tiles.png"); // bb8_self_ill_map.jpg");


	GLuint fallback_shader = 0u;
	program_manager.CreateAndRegisterProgram({ { ShaderType::vertex, "EDAF80/fallback.vert" },
	                                           { ShaderType::fragment, "EDAF80/fallback.frag" } },
	                                         fallback_shader);
	if (fallback_shader == 0u) {
		LogError("Failed to load fallback shader");
		return;
	}

	GLuint cube_shader = 0u;
	program_manager.CreateAndRegisterProgram({ { ShaderType::vertex, "EDAF80/cubemapping.vert" },
	                                           { ShaderType::fragment, "EDAF80/cubemapping.frag" } },
	                                         cube_shader);
	if (cube_shader == 0u){
		LogError("Failed to load cube shader");
		return;
	}

	GLuint default_shader = 0u;
	program_manager.CreateAndRegisterProgram({ { ShaderType::vertex, "EDAF80/default.vert" },
	                                           { ShaderType::fragment, "EDAF80/default.frag" } },
	                                         default_shader);
	if (default_shader == 0u) {
		LogError("Failed to load default shader");
	}

	GLuint bump_shader = 0u;
	program_manager.CreateAndRegisterProgram({ { ShaderType::vertex, "EDAF80/bump.vert" },
	                                           { ShaderType::fragment, "EDAF80/bump.frag" } },
	                                         bump_shader);
	if (bump_shader == 0u)
		LogError("Failed to load cube shader");

	auto camera_position = mCamera.mWorld.GetTranslation();
	auto light_position = glm::vec3(-2.0f, 4.0f, 2.0f);
	auto const set_uniforms = [&light_position](GLuint program){
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
	};

	auto ambient = glm::vec3(0.8f, 0.8f, 0.8f);
	auto diffuse = glm::vec3(1.2f, 0.7f, 0.9f);
	auto specular = glm::vec3(1.0f, 1.0f, 1.0f);
	auto shininess = 1.0f;
	auto const many_set_uniforms = [&light_position,&camera_position,&ambient,&diffuse,&specular,&shininess](GLuint program){
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform3fv(glGetUniformLocation(program, "ambient"), 1, glm::value_ptr(ambient));
		glUniform3fv(glGetUniformLocation(program, "diffuse"), 1, glm::value_ptr(diffuse));
		glUniform3fv(glGetUniformLocation(program, "specular"), 1, glm::value_ptr(specular));
		glUniform1f(glGetUniformLocation(program, "shininess"), shininess);
	};

	auto polygon_mode = polygon_mode_t::fill;
	
// ================ level blocks ====================== 

	Node* foundation = createBox(fou_width, fou_length, fou_height, found_texture, cube_shader, set_uniforms);
	// foundation->add_texture("diffuse_texture", bonobo::loadTexture2D("marble.png"), GL_TEXTURE_2D);

	Node* plat1 = createBox(plat_width, plat_length, plat_height, plat_texture, default_shader, set_uniforms);
	Node* plat2 = createBox(plat_width, plat_length, plat_height, plat_texture, default_shader, set_uniforms);
	Node* plat3 = createBox(plat_width, plat_length, plat_height, plat_texture, default_shader, set_uniforms);
	Node* plat4 = createBox(plat_width, plat_length, plat_height, plat_texture, default_shader, set_uniforms);
	plat1->set_translation(glm::vec3(30.0f, 10.0f, 0.0f));
	plat2->set_translation(glm::vec3(47.0f, 10.0f, 0.0f));
	plat3->set_translation(glm::vec3(17.0f, 17.0f, 0.0f));
	plat4->set_translation(glm::vec3(60.0f, 17.0f, 0.0f));


// ================ Player Characters ====================== 
	int objects_size = objects.size();
	std::vector<Node> boll1(objects_size);
	std::vector<Node> boll2(objects_size);
	for (int i = 0; i < objects_size; i++){

		boll1[i].set_geometry(objects[i]); //parametric_shapes::createSphere(30, 15, 3.0f)));

		boll2[i].set_geometry(objects[i]); //parametric_shapes::createSphere(30, 15, 3.0f));
	
		boll1[i].set_scaling(glm::vec3(0.05, 0.05, 0.05));
		boll1[i].set_translation(glm::vec3(5.0f, 0.0f, 5.0f));
		boll2[i].set_scaling(glm::vec3(0.05, 0.05, 0.05));
		boll2[i].set_translation(glm::vec3(75.0f, 0.0f, 5.0f));
			
		// if(i == 0) {
		// 	boll1[i].add_texture("my_diffuse_map", head_top_texture, GL_TEXTURE_2D);
		// 	boll1[i].add_texture("my_bump_map", head_top_bump, GL_TEXTURE_2D);
		// 	boll2[i].add_texture("my_diffuse_map", head_top_bump, GL_TEXTURE_2D);
		// 	boll2[i].add_texture("my_bump_map", head_top_bump, GL_TEXTURE_2D);
		// 	boll1[i].set_program(&bump_shader,many_set_uniforms);
		// 	boll2[i].set_program(&bump_shader,many_set_uniforms);
		// } else if(i == 6) {
		// 	boll1[i].add_texture("my_diffuse_map", body1_texture, GL_TEXTURE_2D);
		// 	boll1[i].add_texture("my_bump_map", body_bump, GL_TEXTURE_2D);
		// 	boll2[i].add_texture("my_diffuse_map", body2_texture, GL_TEXTURE_2D);
		// 	boll2[i].add_texture("my_bump_map", body_bump, GL_TEXTURE_2D);
		// 	boll1[i].set_program(&bump_shader,many_set_uniforms);
		// 	boll2[i].set_program(&bump_shader,many_set_uniforms);
		// } else {
			boll1[i].set_program(&fallback_shader,set_uniforms);
			boll2[i].set_program(&fallback_shader,set_uniforms);
		// }


		
	}


// ================ Background ====================== 

	auto background = Node();
	background.set_geometry(parametric_shapes::createSphere(60u, 60u, 150.0f));
	background.translate(glm::vec3(0.0f,15.0f,0.0f));
	background.set_program(&cube_shader, set_uniforms);
	background.add_texture("my_cube_map", my_cube_map_id1, GL_TEXTURE_CUBE_MAP);

	glEnable(GL_DEPTH_TEST);

	// Enable face culling to improve performance:
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glCullFace(GL_BACK);


	f64 ddeltatime;
	size_t fpsSamples = 0;
	double nowTime, lastTime = GetTimeMilliseconds();
	double fpsNextTick = lastTime + 1000.0;

	bool show_logs = true;
	bool show_gui = true;
	bool shader_reload_failed = false;


	bool dead1 = false;
	bool dead2 = false;
	bool jump1 = false;
	bool jump2 = false;
	float fall_speed1 = 0;
	float fall_speed2 = 0;
	float x1 = 5;
	float y1 = 0;
	float x2 = 75;
	float y2 = 0;
	int score1 = 0;
	int score2 = 0;

	const float gravity = 0.05f;


	while (!glfwWindowShouldClose(window)) {
		nowTime = GetTimeMilliseconds();
		ddeltatime = nowTime - lastTime;
		if (nowTime > fpsNextTick) {
			fpsNextTick += 1000.0;
			fpsSamples = 0;
		}
		fpsSamples++;
	

		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();
		mCamera.Update(ddeltatime, inputHandler);

		if (inputHandler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
			show_logs = !show_logs;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
			show_gui = !show_gui;
		if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
			shader_reload_failed = !program_manager.ReloadAllPrograms();
			if (shader_reload_failed)
				tinyfd_notifyPopup("Shader Program Reload Error",
				                   "An error occurred while reloading shader programs; see the logs for details.\n"
				                   "Rendering is suspended until the issue is solved. Once fixed, just reload the shaders again.",
				                   "error");
			dead1 = false;
			dead2 = false;
			for (int i = 0; i < objects_size; i++){
				boll1[i].set_translation(glm::vec3(5.0f, 0.0f, 5.0f));
				boll2[i].set_translation(glm::vec3(75.0f, 0.0f, 5.0f));
			}
			x1 = 5.0f;
			x2 = 75.0f;
			y1 = 0.0f;
			y2 = 0.0f;
			fall_speed1 = 0;
			fall_speed2 = 0;
		}
		if (inputHandler.GetKeycodeState(GLFW_KEY_Z) & JUST_PRESSED) {
			polygon_mode = get_next_mode(polygon_mode);
		}

// ================ Player1 Controls ====================== 

		if (inputHandler.GetKeycodeState(GLFW_KEY_D) & PRESSED) {
			if(!(y1<0 && x1<-3 && y1> -7)){
				for (int i = 0; i < objects_size; i++){
					boll1[i].translate(glm::vec3(0.3f,0.0f,0.0f));
				}
				x1+=0.3f;
			}
		}
		if (inputHandler.GetKeycodeState(GLFW_KEY_A) & PRESSED) {
			if(!(y1<0 && x1>83 && y1> -7)){
				for (int i = 0; i < objects_size; i++){
					boll1[i].translate(glm::vec3(-0.2f,0.0f,0.0f));
				}
				x1-=0.2f;
			}
		}
		if (inputHandler.GetKeycodeState(GLFW_KEY_W) & PRESSED && !jump1) {
			jump1 = true;
			fall_speed1=1.0f;
		}
		
		for (int i = 0; i < objects_size; i++){
			boll1[i].translate(glm::vec3(0.0f,fall_speed1,0.0f));
		}
		y1+=fall_speed1;
		// if(y1 > 0 && jump1 || x1 < -1 || x1 >81 || y1 <-5){
		// 	fall_speed1 -= gravity;
		// } else {
		// 	for (int i = 0; i < objects_size; i++){
		// 		boll1[i].set_translation(glm::vec3(x1, 0.0f, 5.0f));
		// 	}
		// 	y1 = 0;
		// 	fall_speed1 = 0;
		// 	jump1 = false;
		// }
		
		if(isOnPlatform(x1,y1)){
			int ylevel = 0;
			if(y1 <=0){
				ylevel = 0;
			} else if (y1<=10){
				ylevel = 10;
			} else if (y1<=17){
				ylevel=17;
			}
			for (int i = 0; i < objects_size; i++){
				boll1[i].set_translation(glm::vec3(x1, ylevel, 5.0f));
			}
			y1 = ylevel;
			fall_speed1 = 0;
			jump1 = false;
		} else {
			jump1 = true;
			fall_speed1 -= gravity;
		}

// ================ Player2	 Controls ====================== 

		if (inputHandler.GetKeycodeState(GLFW_KEY_L) & PRESSED) {
			if(!(y2<0 && x2<-3 && y2> -7)){
				for (int i = 0; i < objects_size; i++){
					boll2[i].translate(glm::vec3(0.2f,0.0f,0.0f));
				}
				x2+=0.2f;
			}
		}
		if (inputHandler.GetKeycodeState(GLFW_KEY_J) & PRESSED) {
			if(!(y2<0 && x2>83 && y2> -7)){
				for (int i = 0; i < objects_size; i++){
					boll2[i].translate(glm::vec3(-0.3f,0.0f,0.0f));
				}
				x2-=0.3f;
			} 
		}
		if (inputHandler.GetKeycodeState(GLFW_KEY_I) & PRESSED && !jump2) {
			jump2 = true;
			fall_speed2=1.0f;
		}

		for (int i = 0; i < objects_size; i++){
			boll2[i].translate(glm::vec3(0.0f,fall_speed2,0.0f));
		}
		y2+=fall_speed2;
		if(isOnPlatform(x2,y2)){
			int ylevel = 0;
			if(y2 <=0){
				ylevel = 0;
			} else if (y2<=10){
				ylevel = 10;
			} else if (y2<=17){
				ylevel=17;
			}
			for (int i = 0; i < objects_size; i++){
				boll2[i].set_translation(glm::vec3(x2, ylevel, 5.0f));
			}
			y2 = ylevel;
			fall_speed2 = 0;
			jump2 = false;
		} else {
			jump2 = true;
			fall_speed2 -= gravity;
		}

		if(glm::distance(glm::vec2(x1,y1),glm::vec2(x2,y2)) < 6.0f){
			if(y1>y2 && !dead2 && !dead1){
				fall_speed1 = 0.8*-fall_speed1;
				dead2 = true;
				score1++;
				std::string scoretext1 = "Score: " + std::to_string(score1) + " - " + std::to_string(score2);
				tinyfd_notifyPopup("Player 1 WINS!!", scoretext1.c_str(),"");
			}
			if(y2>y1 && !dead1 && !dead2){
				fall_speed2 = 0.8 *-fall_speed2;
				dead1 = true;
				score2++;
				std::string scoretext2 = "Score: " + std::to_string(score1) + " - " + std::to_string(score2);
				tinyfd_notifyPopup("Player 2 WINS!!", scoretext2.c_str(),"");
			}
		}

// ==================== Diverse Operation =====================

		ImGui_ImplGlfwGL3_NewFrame();


		int framebuffer_width, framebuffer_height;
		glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
		glViewport(0, 0, framebuffer_width, framebuffer_height);
		glClearDepthf(1.0f);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


		if (!shader_reload_failed) {
			super_render(glm::mat4(1.0f), foundation, default_shader, mCamera);
			super_render(glm::mat4(1.0f), plat1, default_shader, mCamera);
			super_render(glm::mat4(1.0f), plat2, default_shader, mCamera);
			super_render(glm::mat4(1.0f), plat3, default_shader, mCamera);
			super_render(glm::mat4(1.0f), plat4, default_shader, mCamera);
			background.render(mCamera.GetWorldToClipMatrix(), background.get_transform());

			if (!dead1) {
				for (int i = 0; i < objects_size; i++){
					boll1[i].render(mCamera.GetWorldToClipMatrix(), boll1[i].get_transform());
				}
			}
			if (!dead2) {
				for (int i = 0; i < objects_size; i++){
					boll2[i].render(mCamera.GetWorldToClipMatrix(), boll2[i].get_transform());
				}
			}
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//
		// Todo: If you want a custom ImGUI window, you can set it up
		//       here
		//

		switch (polygon_mode) {
			case polygon_mode_t::fill:
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				break;
			case polygon_mode_t::line:
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				break;
			case polygon_mode_t::point:
				glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
				break;
		}

		if (show_logs)
			Log::View::Render();
		if (show_gui)
			ImGui::Render();

		glfwSwapBuffers(window);
		lastTime = nowTime;
	}
}

Node*
edaf80::Assignment5::createBox(float width, float length, float height, GLuint texture, GLuint shader, std::function<void (GLuint)> const& uniform){
	Node* posy = new Node();
	Node* negy = new Node();
	Node* posz = new Node();
	Node* negz = new Node();
	Node* posx = new Node();
	Node* negx = new Node();

	posy->set_geometry(parametric_shapes::createManyQuads(width, length));
	negy->set_geometry(parametric_shapes::createManyQuads(width, length));
	negy->translate(glm::vec3(0.0f,-height,0.0f));
	posy->add_texture("diffuse_texture", texture, GL_TEXTURE_2D);
	negy->add_texture("diffuse_texture", texture, GL_TEXTURE_2D);	
	posy->set_program(&shader, uniform);	
	negy->set_program(&shader, uniform);


	posz->set_geometry(parametric_shapes::createManyQuads(width, height));
	negz->set_geometry(parametric_shapes::createManyQuads(width, height));
	posz->rotate_x(glm::two_pi<float>()/4);
	negz->rotate_x(glm::two_pi<float>()/4);
	negz->translate(glm::vec3(0.0f,0.0f,length));
	posz->add_texture("diffuse_texture", texture, GL_TEXTURE_2D);
	negz->add_texture("diffuse_texture", texture, GL_TEXTURE_2D);
	posz->set_program(&shader, uniform);	
	negz->set_program(&shader, uniform);

	posx->set_geometry(parametric_shapes::createManyQuads(height, length));
	negx->set_geometry(parametric_shapes::createManyQuads(height, length));
	posx->rotate_z(-glm::two_pi<float>()/4);
	negx->rotate_z(-glm::two_pi<float>()/4);
	negx->translate(glm::vec3(width, 0.0f,0.0f));
	posx->add_texture("diffuse_texture", texture, GL_TEXTURE_2D);
	negx->add_texture("diffuse_texture", texture, GL_TEXTURE_2D);
	posx->set_program(&shader, uniform);	
	negx->set_program(&shader, uniform);	

	Node* master = new Node();
	master->add_child(posy);
	master->add_child(negy);
	master->add_child(posz);
	master->add_child(negz);
	master->add_child(posx);
	master->add_child(negx);

	return master;
}

bool
edaf80::Assignment5::isOnPlatform(float x, float y)//, std::string platform)
{
		return (x > -1 && x < 81) && (y > -5 && y <= 0) ||
			   (x > 29 && x < 37) && (y > 9.2 && y <= 10) ||
			   (x > 46 && x < 54) && (y > 9.2 && y <= 10) ||
			   (x > 16 && x < 24) && (y > 16.2 && y <= 17) ||
			   (x > 59 && x < 67) && (y > 16.2 && y <= 17);
}

int main()
{
	Bonobo::Init();
	try {
		edaf80::Assignment5 assignment5;
		assignment5.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
	Bonobo::Destroy();
}
