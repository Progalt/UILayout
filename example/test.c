
#include <stdio.h>

#include <LatteLayout/layout.h>


#include <stdio.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <nanovg.h>
#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg_gl.h>

NVGcontext* vg = NULL;

void errorcb(int error, const char* desc)
{
	printf("GLFW error %d: %s\n", error, desc);
}

static void drawNode(LatteNode* node)
{

	nvgBeginPath(vg);

	LattePosition pos = latteGetScreenPosition(node);
	
	nvgStrokeColor(vg, nvgRGB(255, 0, 0));
	nvgStrokeWidth(vg, 1.5f);
	nvgRect(vg, pos.x, pos.y, node->size.width, node->size.height);
	nvgStroke(vg);

	if (node->sizer.widthSizer == LATTE_SIZER_FIT || node->sizer.heightSizer == LATTE_SIZER_FIT)
	{
		nvgFillColor(vg, nvgRGB(255, 255, 0));
		nvgFill(vg);
	}

	nvgClosePath(vg);


	for (int i = 0; i < node->childCount; i++)
		drawNode(node->children[i]);
}

int main()
{
	GLFWwindow* window;
	double prevt = 0, cpuTime = 0;

	if (!glfwInit()) {
		printf("Failed to init GLFW.");
		return -1;
	}

	glfwSetErrorCallback(errorcb);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

	window = glfwCreateWindow(1000, 600, "UI layout", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGL())
	{
		return -1;
	}

	vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG); 

	if (vg == NULL) {
		printf("Could not init nanovg.\n");
		return -1;
	}
	glfwSwapInterval(0);

	glfwSetTime(0);
	prevt = glfwGetTime();

	LatteNode* root = latteCreateNode("Root", NULL, LATTE_NODE_FLAGS_NONE);
	lattePadding(root, 12.0f);
	latteSpacing(root, 16.0f);
	latteMainAxisDirection(root, LATTE_DIRECTION_VERTICAL);
	latteCrossAxisAlignment(root, LATTE_CONTENT_CENTER);
	latteMainAxisAlignment(root, LATTE_CONTENT_SPACE_AROUND);

	LatteNode* parent1 = latteCreateNode("parent", root, LATTE_NODE_FLAGS_NONE);
	latteSizer(parent1, LATTE_SIZER_FIT, LATTE_SIZER_FIT);
	latteSpacing(parent1, 12.0f);
	lattePadding(parent1, 32.0f);
	latteCrossAxisAlignment(parent1, LATTE_CONTENT_CENTER);

	LatteNode* child1 = latteCreateNode("Child1", parent1, LATTE_NODE_FLAGS_NONE);
	LatteNode* child2 = latteCreateNode("Child2", parent1, LATTE_NODE_FLAGS_NONE);
	latteSizer(child1, LATTE_SIZER_FIXED(10.0f), LATTE_SIZER_GROW);
	latteSizer(child2, LATTE_SIZER_FIXED(100.0f), LATTE_SIZER_FIXED(40.0f));

	LatteNode* parent2 = latteCreateNode("parent2", root, LATTE_NODE_FLAGS_NONE);
	latteSizer(parent2, LATTE_SIZER_GROW, LATTE_SIZER_FIXED(24.0f));

	LatteNode* parent3 = latteCreateNode("parent3", root, LATTE_NODE_FLAGS_NONE);
	latteSizer(parent3, LATTE_SIZER_GROW, LATTE_SIZER_FIXED(64.0f));


	while (!glfwWindowShouldClose(window))
	{
		double mx, my, t, dt;
		int winWidth, winHeight;
		int fbWidth, fbHeight;
		float pxRatio;

		t = glfwGetTime();
		dt = t - prevt;
		prevt = t;

		glfwGetCursorPos(window, &mx, &my);
		glfwGetWindowSize(window, &winWidth, &winHeight);
		glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
		// Calculate pixel ration for hi-dpi devices.
		pxRatio = (float)fbWidth / (float)winWidth;

		{
			latteSizer(root, LATTE_SIZER_FIXED(winWidth), LATTE_SIZER_FIXED(winHeight));
			latteLayout(root);
		}

		// Update and render
		glViewport(0, 0, fbWidth, fbHeight);
		glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		nvgBeginFrame(vg, winWidth, winHeight, pxRatio);

		drawNode(root);
		
		nvgEndFrame(vg);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	latteFreeNode(root);

	nvgDeleteGL3(vg);

	glfwTerminate();
	return 0;
}
