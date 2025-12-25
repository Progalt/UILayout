#ifndef LATTE_NODE_RENDERER_H
#define LATTE_NODE_RENDERER_H

extern "C" {
#include <LatteLayout/layout.h>
}

#include "../Utils/Singleton.h"

#include "../glad/glad.h"

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