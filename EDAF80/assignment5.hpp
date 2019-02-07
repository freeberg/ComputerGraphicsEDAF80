#pragma once

#include "core/InputHandler.h"
#include "core/FPSCamera.h"
#include "core/WindowManager.hpp"
#include "core/node.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class Window;


namespace edaf80
{
	//! \brief Wrapper class for Assignment 5
	class Assignment5 {
	public:
		void super_render(glm::mat4 m, const Node* root, GLuint shader, FPSCameraf& camera);
		//! \brief Default constructor.
		//!
		//! It will initialise various modules of bonobo and retrieve a
		//! window to draw to.
		Assignment5();

		//! \brief Default destructor.
		//!
		//! It will release the bonobo modules initialised by the
		//! constructor, as well as the window.
		~Assignment5();

		//! \brief Contains the logic of the assignment, along with the
		//! render loop.
		void run();

		Node* createBox(float width, float length, float height, GLuint texture, GLuint shader, std::function<void (GLuint)> const& uniform);

		bool isOnPlatform(float x, float y);

	private:
		FPSCameraf    mCamera;
		InputHandler  inputHandler;
		WindowManager mWindowManager;
		GLFWwindow    *window;
	};
}
