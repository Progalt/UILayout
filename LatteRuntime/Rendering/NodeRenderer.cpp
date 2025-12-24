#include "NodeRenderer.h"

#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_gl.h>

#include "../Binding/Component.h"
#include "Color.h"

namespace latte
{
	NVGcontext* RenderInterface::getNVGContext()
	{

		if (!m_NVGcontext)
		{
			if (!gladLoadGL())
			{
				throw std::runtime_error("GLAD Init Err");
			}

			m_NVGcontext = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);

			nvgCreateFont(m_NVGcontext, "Roboto-Regular", "Roboto-Regular.ttf");

			glEnable(GL_STENCIL_TEST);
		}

		return m_NVGcontext;
	}

	void renderNode(LatteNode* node, NVGcontext* vg)
	{
		nvgBeginPath(vg);

		LattePosition pos = latteGetScreenPosition(node);

		WidgetData* data = (WidgetData*)latteGetUserData(node);

		if (data)
		{
			bool shouldPaint = true;
			if (data->paint)
				shouldPaint = data->paint();

			if (shouldPaint)
			{
				if (data->type == latte::WIDGET_TYPE_BOX)
				{
					float btr = 0.0f, btl = 0.0f, bbr = 0.0f, bbl = 0.0f;

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
						}
						else
						{
							nvgFillColor(vg, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.0f));
						}

						if (style["borderRadius"].valid())
						{
							btl = style["borderRadius"][1];
							btr = style["borderRadius"][2];
							bbr = style["borderRadius"][3];
							bbl = style["borderRadius"][4];
						}
					}

					

					if (btl == btr && btl == bbr && btl == bbl)
					{
						if (btl == 0.0f)
							nvgRect(vg, pos.x, pos.y, node->size.width, node->size.height);
						else
							nvgRoundedRect(vg, pos.x, pos.y, node->size.width, node->size.height, btl);
					}
					else
					{
						nvgRoundedRectVarying(vg, pos.x, pos.y, node->size.width, node->size.height, btl, btr, bbr, bbl);
					}


					nvgFill(vg);
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

					nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
					nvgText(vg, pos.x, pos.y, data->text.c_str(), NULL);
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
		NVGcontext* vg = RenderInterface::getInstance().getNVGContext();

		win->makeCurrent();

		glViewport(0, 0, win->getWidth(), win->getHeight());
		glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		nvgBeginFrame(vg, win->getWidth(), win->getHeight(), 1.0f);

		renderNode(win->getRootNode(), vg);

		nvgEndFrame(vg);
	}
}