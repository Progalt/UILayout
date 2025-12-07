
#include <stdio.h>

#include <layout.h>


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

static void drawNode(Node* node)
{

	nvgBeginPath(vg);

	Position pos = GetScreenPosition(node);
	
	nvgStrokeColor(vg, nvgRGB(255, 0, 0));
	nvgStrokeWidth(vg, 1.5f);
	nvgRect(vg, pos.x, pos.y, node->size.width, node->size.height);
	nvgStroke(vg);

	if (node->sizer.type == SIZER_FIT)
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

	int font = nvgCreateFont(vg, "Roboto", "example/data/Roboto-Regular.ttf");

	Node* root = CreateNode("Root", NULL);
	Padding(root, 12.0f);
	Spacing(root, 16.0f);
	MainAxisDirection(root, DIRECTION_TOP_TO_BOTTOM);
	CrossAxisAlignment(root, ALIGNMENT_CENTER);

	Node* parent1 = CreateNode("parent", root);
	FitSizer(parent1);
	Spacing(parent1, 12.0f);
	Padding(parent1, 4.0f);
	CrossAxisAlignment(parent1, ALIGNMENT_END);

	Node* child1 = CreateNode("Child1", parent1);
	Node* child2 = CreateNode("Child2", parent1);
	FixedSizer(child1, 10.0f, 10.0f);
	FixedSizer(child2, 100.0f, 40.0f);


	while (!glfwWindowShouldClose(window))
	{
		double mx, my, t, dt;
		int winWidth, winHeight;
		int fbWidth, fbHeight;
		float pxRatio;
		float gpuTimes[3];
		int i, n;

		t = glfwGetTime();
		dt = t - prevt;
		prevt = t;

		glfwGetCursorPos(window, &mx, &my);
		glfwGetWindowSize(window, &winWidth, &winHeight);
		glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
		// Calculate pixel ration for hi-dpi devices.
		pxRatio = (float)fbWidth / (float)winWidth;

		{
			FixedSizer(root, winWidth, winHeight);
			Layout(root);
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

	FreeNode(root);

	nvgDeleteGL3(vg);

	glfwTerminate();
	return 0;
}
