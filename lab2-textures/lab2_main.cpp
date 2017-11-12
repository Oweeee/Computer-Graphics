
#include <GL/glew.h>

// STB_IMAGE for loading images of many filetypes
#include <stb_image.h>

#include <cstdlib>

#include <labhelper.h>

#include <imgui.h>
#include <imgui_impl_sdl_gl3.h>

// The window we'll be rendering to
SDL_Window* g_window = nullptr;

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

int mag = 1;
int mini = 5;
float anisotropy = 16.0f;

// The shaderProgram holds the vertexShader and fragmentShader
GLuint shaderProgram;

// The vertexArrayObject here will hold the pointers to 
// the vertex data (in positionBuffer) and color data per vertex (in colorBuffer)
GLuint		positionBuffer, colorBuffer, indexBuffer, textureBuffer, vertexArrayObject,  texture;						
GLuint		positionBuffer2, colorBuffer2, indexBuffer2, textureBuffer2, vertexArrayObject2, texture2;


void initGL()
{
	///////////////////////////////////////////////////////////////////////////
	// Create the vertex array object
	///////////////////////////////////////////////////////////////////////////
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	///////////////////////////////////////////////////////////////////////////
	// Create the positions buffer object
	///////////////////////////////////////////////////////////////////////////	
	const float positions[] = {
		// X Y Z
		-10.0f, -10.0f, -30.0f,    // v0
		-10.0f, -10.0f, -330.0f,   // v1
		10.0f, -10.0f, -330.0f,   // v2
		10.0f, -10.0f, -30.0f     // v3
	};
	glGenBuffers(1, &positionBuffer);													// Create a handle for the vertex position buffer
	glBindBuffer( GL_ARRAY_BUFFER, positionBuffer );									// Set the newly created buffer as the current one
	glBufferData( GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW );		// Send the vetex position data to the current buffer
	glVertexAttribPointer(0, 3, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/);
	glEnableVertexAttribArray(0);

	///////////////////////////////////////////////////////////////////////////
	// Create the colors buffer object
	///////////////////////////////////////////////////////////////////////////	
	const float colors[] = {
		//  R     G		B
		1.0f, 1.0f, 1.0f,		// White
		0.5f, 0.5f, 0.8f,		// Lightblue
		0.5f, 0.5f, 0.8f,		// Lightblue
		1.0f, 1.0f, 1.0f		// White
	};
	glGenBuffers(1, &colorBuffer);														// Create a handle for the vertex color buffer
	glBindBuffer( GL_ARRAY_BUFFER, colorBuffer );										// Set the newly created buffer as the current one
	glBufferData( GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW );			// Send the color data to the current buffer
	glVertexAttribPointer(1, 3, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/);
	glEnableVertexAttribArray(1);

	///////////////////////////////////////////////////////////////////////////
	// >>> @task 1 : Create the texture coordinates.
	//				 Create the texture coordinates' buffer object.
	//				 Set up the attrib pointer.
	//				 Enable the vertex attrib array.
	///////////////////////////////////////////////////////////////////////////
	float texcoords[] = {
		0.0f, 0.0f, // (u,v) for v0 
		0.0f, 15.0f, // (u,v) for v1
		1.0f, 15.0f, // (u,v) for v2
		1.0f, 0.0f // (u,v) for v3
	};

	glGenBuffers(1, &textureBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/);
	glEnableVertexAttribArray(2);

	///////////////////////////////////////////////////////////////////////////
	// Create the element array buffer object
	///////////////////////////////////////////////////////////////////////////	
	const int indices[] = {
		0, 1, 3, // Triangle 1
		1, 2, 3  // Triangle 2
	};
	glGenBuffers( 1, &indexBuffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBuffer );										
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW );			


	// The loadShaderProgram and linkShaderProgam functions are defined in glutil.cpp and 
	// do exactly what we did in lab1 but are hidden for convenience
	shaderProgram = labhelper::loadShaderProgram("../lab2-textures/simple.vert", "../lab2-textures/simple.frag");

	//**********************************************

	//************************************
	//			Load Texture
	//************************************
	// >>> @task 2

	int w, h, comp;
	unsigned char* image = stbi_load("../lab2-textures/asphalt.jpg", &w, &h, &comp, STBI_rgb_alpha);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	free(image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glGenerateMipmap(GL_TEXTURE_2D);
	// Sets the type of filtering to be used on magnifying and
	// minifying the active texture. These are the nicest available options.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);


	//Task 6, create explosion

	glGenVertexArrays(1, &vertexArrayObject2);
	glBindVertexArray(vertexArrayObject2);

	const float positions2[] = {
		// X Y Z
		-10.0f,  10.0f, -50.0f,    // v0
		-10.0f, -10.0f, -50.0f,   // v1
		10.0f, -10.0f,  -50.0f,   // v2
		10.0f,  10.0f,  -50.0f     // v3
	};
	glGenBuffers(1, &positionBuffer2);													// Create a handle for the vertex position buffer
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer2);									// Set the newly created buffer as the current one
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions2), positions2, GL_STATIC_DRAW);		// Send the vetex position data to the current buffer
	glVertexAttribPointer(0, 3, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/);
	glEnableVertexAttribArray(0);

	float texcoords2[] = {
		0.0f, 0.0f, // (u,v) for v0 
		0.0f, 1.0f, // (u,v) for v1
		1.0f, 1.0f, // (u,v) for v2
		1.0f, 0.0f  // (u,v) for v3
	};

	glGenBuffers(1, &textureBuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffer2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords2), texcoords2, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/);
	glEnableVertexAttribArray(2);

	const int indices2[] = {
		0, 1, 3, // Triangle 1
		1, 2, 3  // Triangle 2
	};
	glGenBuffers(1, &indexBuffer2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2), indices2, GL_STATIC_DRAW);

	// The loadShaderProgram and linkShaderProgam functions are defined in glutil.cpp and 
	// do exactly what we did in lab1 but are hidden for convenience
	int w2, h2, comp2;
	unsigned char* explosionImage = stbi_load("../lab2-textures/explosion.png", &w2, &h2, &comp2, STBI_rgb_alpha);

	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w2, h2, 0, GL_RGBA, GL_UNSIGNED_BYTE, explosionImage);
	free(explosionImage);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void display(void)
{
	// The viewport determines how many pixels we are rasterizing to
	int w, h;
	SDL_GetWindowSize(g_window, &w, &h);
	glViewport(0, 0, w, h);		// Set viewport

	glClearColor(0.2,0.2,0.8,1.0);						// Set clear color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clears the color buffer and the z-buffer

	// We disable backface culling for this tutorial, otherwise care must be taken with the winding order
	// of the vertices. It is however a lot faster to enable culling when drawing large scenes.
	glDisable(GL_CULL_FACE);
	// Disable depth testing
	glDisable(GL_DEPTH_TEST);
	// Shader Program
	glUseProgram( shaderProgram );				// Set the shader program to use for this draw call

	// Set up a projection matrix
	float fovy = radians(45.0f);
	float aspectRatio = float(w) / float(h);
	float nearPlane = 0.01f;
	float farPlane = 300.0f;
	mat4 projectionMatrix = perspective(fovy, aspectRatio, nearPlane, farPlane);
	// Send it to the vertex shader
	int loc = glGetUniformLocation(shaderProgram, "projectionMatrix");
	glUniformMatrix4fv(loc, 1, false, &projectionMatrix[0].x);

	// >>> @task 3.1

	glBindVertexArray(vertexArrayObject);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	/*glBindVertexArray(vertexArrayObject2);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);*/

	glUseProgram( 0 ); // "unsets" the current shader program. Not really necessary.

}

void gui() {
	// Inform imgui of new frame
	ImGui_ImplSdlGL3_NewFrame(g_window);

        // ----------------- Set variables --------------------------  
        ImGui::PushID("mag");
        ImGui::Text("Magnification");
		if (ImGui::RadioButton("GL_NEAREST", &mag, 0)) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		if(ImGui::RadioButton("GL_LINEAR", &mag, 1)) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
        ImGui::PopID();

        ImGui::PushID("mini");
        ImGui::Text("Minification");
		if (ImGui::RadioButton("GL_NEAREST", &mini, 0)) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		if (ImGui::RadioButton("GL_LINEAR", &mini, 1)) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		if (ImGui::RadioButton("GL_NEAREST_MIPMAP_NEAREST", &mini, 2)) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		}
		if (ImGui::RadioButton("GL_NEAREST_MIPMAP_LINEAR", &mini, 3)) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		}
		if (ImGui::RadioButton("GL_LINEAR_MIPMAP_NEAREST", &mini, 4)) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		}
		if (ImGui::RadioButton("GL_LINEAR_MIPMAP_LINEAR", &mini, 5)) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}

        ImGui::PopID();

		if (ImGui::SliderFloat("Anisotropic filtering", &anisotropy, 1.0, 16.0, "Number of samples: %.0f")) {
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
		}
		
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	// ----------------------------------------------------------


	// Render the GUI.
	ImGui::Render();
}

int main(int argc, char *argv[])
{
	g_window = labhelper::init_window_SDL("OpenGL Lab 2");

	initGL();

	// render-loop
	bool stopRendering = false;
	while (!stopRendering) {
		// render to window
		display();
		
		
                // Render overlay GUI.
                gui();

		// Swap front and back buffer. This frame will now been displayed.
		SDL_GL_SwapWindow(g_window);			

		// check events (keyboard among other)
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			// Allow ImGui to capture events.
			ImGui_ImplSdlGL3_ProcessEvent(&event);

			if (event.type == SDL_QUIT || (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)) {
				stopRendering = true;
			}
		}
	}

	// Shut down everything. This includes the window and all other subsystems.
	labhelper::shutDown(g_window);
	return 0;          
}
