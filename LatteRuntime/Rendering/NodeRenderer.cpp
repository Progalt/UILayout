#include "NodeRenderer.h"

#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_gl.h>

#include "../Components/Component.h"
#include "Color.h"

namespace latte
{
	void GLAPIENTRY
		MessageCallback(GLenum source,
			GLenum type,
			GLuint id,
			GLenum severity,
			GLsizei length,
			const GLchar* message,
			const void* userParam)
	{
		fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
			(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
			type, severity, message);
	}

	NVGcontext* RenderInterface::getNVGContext()
	{

		if (!m_NVGcontext)
		{
			if (!m_LoadedGL)
			{
				if (!gladLoadGL())
				{
					throw std::runtime_error("GLAD Init Err");
				}
				m_LoadedGL = true;
			}

			m_NVGcontext = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);

			nvgCreateFont(m_NVGcontext, "Roboto-Regular", "Roboto-Regular.ttf");

			glEnable(GL_STENCIL_TEST);


			// glEnable(GL_DEBUG_OUTPUT);
			// glDebugMessageCallback(MessageCallback, 0);
		}

		return m_NVGcontext;
	}

	void renderNode(LatteNode* node, NVGcontext* vg)
	{
		nvgBeginPath(vg);

		LattePosition pos = latteGetScreenPosition(node);

		ComponentData* data = (ComponentData*)latteGetUserData(node);

		if (data)
		{
			bool shouldPaint = true;
			{
				auto itr = data->eventCallbacks.find(COMPONENT_EVENT_PAINT);
				if (itr != data->eventCallbacks.end())
					shouldPaint = itr->second();
			}

			if (shouldPaint)
			{
				if (data->type == latte::WIDGET_TYPE_BOX)
				{
					float btr = 0.0f, btl = 0.0f, bbr = 0.0f, bbl = 0.0f;
					bool hasBorder = false;
					bool hasFill = false;

					if (data->style.valid())
					{
						sol::table style = data->style;

						if (style["backgroundColor"].valid())
						{
							float r = style["backgroundColor"][1];
							float g = style["backgroundColor"][2];
							float b = style["backgroundColor"][3];
							float a = style["backgroundColor"][4];

							nvgFillColor(vg, nvgRGBAf(r, g, b, a));
							hasFill = true;
						}

						if (style["borderRadius"].valid())
						{
							btl = style["borderRadius"][1];
							btr = style["borderRadius"][2];
							bbr = style["borderRadius"][3];
							bbl = style["borderRadius"][4];
						}

						if (style["border"].valid())
						{
							sol::table border = style["border"].get<sol::table>();
							float width = border.get_or("width", 0.0f);

							if (width > 0.0f)
							{
								hasBorder = true;

								// TODO: Not this
								nvgStrokeWidth(vg, width);

								if (border["color"].valid())
								{
									float r = border["color"][1];
									float g = border["color"][2];
									float b = border["color"][3];
									float a = border["color"][4];

									nvgStrokeColor(vg, nvgRGBAf(r, g, b, a));
								}
							}
						}
					}

					float halfWidth = node->size.width / 2.0f;
					float halfHeight = node->size.height / 2.0f;
					

					if (btl == btr && btl == bbr && btl == bbl)
					{
						btl = min(btl, min(halfWidth, halfHeight));

						if (btl == 0.0f)
							nvgRect(vg, std::roundf(pos.x), std::roundf(pos.y), std::roundf(node->size.width), std::roundf(node->size.height));
						else
							nvgRoundedRect(vg, std::roundf(pos.x), std::roundf(pos.y), std::roundf(node->size.width), std::roundf(node->size.height), btl);
					}
					else
					{
						// This restricts rounding to be half the size in any direction
						btl = min(btl, min(halfWidth, halfHeight));
						btr = min(btr, min(halfWidth, halfHeight));
						bbl = min(bbl, min(halfWidth, halfHeight));
						bbr = min(bbr, min(halfWidth, halfHeight));

						nvgRoundedRectVarying(vg, std::roundf(pos.x), std::roundf(pos.y), std::roundf(node->size.width), std::roundf(node->size.height), btl, btr, bbr, bbl);
					}

					if(hasFill)
						nvgFill(vg);

					if (hasBorder)
						nvgStroke(vg);
				}
				else
				{
					float fontSize = 14.0f;
					if (data->style.valid())
					{
						sol::table style = data->style;

						if (style["color"].valid())
						{
							float r = style["color"][1];
							float g = style["color"][2];
							float b = style["color"][3];
							float a = style["color"][4];

							nvgFillColor(vg, nvgRGBAf(r, g, b, a));
						}
						else
						{
							nvgFillColor(vg, nvgRGB(0, 0, 0));
						}

						fontSize = style.get_or("fontSize", 14.0f);
					}
					nvgFontFace(vg, "Roboto-Regular");
					nvgFontSize(vg, fontSize);

					float ascender, descender, lineh;
					nvgTextMetrics(vg, &ascender, &descender, &lineh);

					nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
					// TODO: Nasty hacky descender stuff
					// Works for my current font but probably not all the time? FIX 
					nvgText(vg, std::roundf(pos.x), std::roundf(pos.y - (descender / 2.0f)), data->text.c_str(), NULL);
				}
			}
			
		}

	/*	nvgStrokeColor(vg, nvgRGB(255, 0, 0));
		nvgStrokeWidth(vg, 2.0f);
		nvgRect(vg, pos.x, pos.y, node->size.width, node->size.height);
		nvgStroke(vg);*/

		nvgClosePath(vg);

		for (int i = 0; i < node->childCount; i++)
		{
			renderNode(node->children[i], vg);
		}

	}

	void renderRoot(std::shared_ptr<Window> win)
	{
		win->makeCurrent();

		NVGcontext* vg = RenderInterface::getInstance().getNVGContext();

		// Reset the state as nanovg might've touched it
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, win->getWidth(), win->getHeight());
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		nvgBeginFrame(vg, win->getWidth(), win->getHeight(), 1.0f);

		renderNode(win->getRootNode(), vg);

		nvgEndFrame(vg);
	}
}