#ifndef LATTE_NODE_RENDERER_H
#define LATTE_NODE_RENDERER_H

extern "C" {
#include <LatteLayout/layout.h>
}

#include "../Utils/Singleton.h"

#if defined(ANDROID) || defined(__ANDROID__)
#include <GLES3/gl3.h>
#else
#include "../glad/glad.h"
#endif

#include "../OS/Window.h"

struct NVGcontext;

namespace latte
{

	class RenderInterface : public Singleton<RenderInterface>
	{
	public:

		NVGcontext* getNVGContext();

	private:

		NVGcontext* m_NVGcontext = NULL;
		bool m_LoadedGL = false;
	};

	void renderNode(LatteNode* node, NVGcontext* vg);

	void renderRoot(std::shared_ptr<Window> win);

}

#endif // LATTE_NODE_RENDERER_H