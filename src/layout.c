#include "layout.h"

#include <malloc.h>
#include <string.h>
#include <assert.h>

#define NODE_ASSIGN_VAL(to, val) node->##to = val; _propogateDirty(node);

typedef void(*PropogateFunc)(Node* node);

static void _propogate(Node* node, PropogateFunc func)
{
	func(node);

	for (int i = 0; i < node->childCount; i++)
		_propogate(node->children[i], func);
}

static void _setDirty(Node* node)
{
	node->dirty = 1;
}

static void _propogateDirty(Node* node)
{
	_propogate(node, _setDirty);
}

Node* CreateNode(const char* id, Node* parent)
{

	Node* node = (Node*)malloc(sizeof(Node));

	if (node == NULL)
		return NULL;

	memset(node, 0, sizeof(Node));

	if (strlen(id) > 0)
	{
		node->id = strdup(id);
	}

	node->layoutDirection = DIRECTION_LEFT_TO_RIGHT;
	node->crossAxisAlignment = ALIGNMENT_START;

	if (parent)
	{
		NodeAddChild(parent, node);
	}

	RelativePositioner(node);

	node->dirty = 1;

	return node;
}

void FreeNode(Node* node)
{
	assert(node);

	if (node->id)
		free(node->id);
	free(node);
}

void NodeAddChild(Node* node, Node* child)
{
	assert(node);
	assert(child);

	child->parent = node;

	if (node->childCount == node->childCapacity) 
	{
		int new_capacity = node->childCapacity ? node->childCapacity * 2 : 2;
		node->children = realloc(node->children, new_capacity * sizeof(Node*));
		node->childCapacity = new_capacity;
	}
	node->children[node->childCount++] = child;

	_propogateDirty(node);
}

void MainAxisDirection(Node* node, LayoutDirection dir)
{
	NODE_ASSIGN_VAL(layoutDirection, dir)
}

void FixedSizer(Node* node, float width, float height)
{
	LayoutSizer newSizer = {
		.type = SIZER_FIXED,
		.size = { .width = width, .height = height },
	};

	NODE_ASSIGN_VAL(sizer, newSizer)
}

void FitSizer(Node* node)
{
	LayoutSizer newSizer = {
		.type = SIZER_FIT
	};

	NODE_ASSIGN_VAL(sizer, newSizer)
}

void RelativePositioner(Node* node)
{
	LayoutPositioner newPositioner = {
		.type = POSITIONER_RELATIVE,
		.position = { .x = 0.0f, .y = 0.0f }
	};

	NODE_ASSIGN_VAL(positioner, newPositioner)
}

void AbsolutePositioner(Node* node, float relX, float relY)
{
	LayoutPositioner newPositioner = {
		.type = POSITIONER_ABSOLUTE,
		.position = { .x = relX, .y = relY }
	};

	NODE_ASSIGN_VAL(positioner, newPositioner)
}

void CrossAxisAlignment(Node* node, Alignment alignment)
{
	NODE_ASSIGN_VAL(crossAxisAlignment, alignment)
}

void Padding(Node* node, float padding)
{
	PaddingAxis(node, padding, padding);
}

void PaddingRLTB(Node* node, float right, float left, float top, float bottom)
{
	Margin margin = {
		.top = top,
		.left = left,
		.bottom = bottom,
		.right = right
	};

	NODE_ASSIGN_VAL(padding, margin)
}

void PaddingAxis(Node* node, float vert, float horz)
{
	PaddingRLTB(node, horz, vert, vert, horz);
}

void Spacing(Node* node, float gap)
{
	NODE_ASSIGN_VAL(spacing, gap)
}

void Layout(Node* node)
{
	if (!node) return;

	if (node->dirty == 0) return;

	for (int i = 0; i < node->childCount; ++i)
		Layout(node->children[i]);

	switch (node->sizer.type)
	{
	case SIZER_FIXED:
		node->size = node->sizer.size;
		break;
	case SIZER_FIT:
	{
		float totalMain = 0, maxCross = 0;
		int numRelChildren = 0;

		// Only consider RELATIVE children for fitting
		for (int i = 0; i < node->childCount; ++i)
		{
			Node* child = node->children[i];
			if (child->positioner.type == POSITIONER_RELATIVE) {
				++numRelChildren;
				if (node->layoutDirection == DIRECTION_LEFT_TO_RIGHT)
				{
					totalMain += child->size.width;
					if (child->size.height > maxCross) maxCross = child->size.height;
				}
				else
				{
					totalMain += child->size.height;
					if (child->size.width > maxCross) maxCross = child->size.width;
				}
			}
		}
		float spacing = (numRelChildren > 0) ? (numRelChildren - 1) * node->spacing : 0;

		if (node->layoutDirection == DIRECTION_LEFT_TO_RIGHT)
		{
			node->size.width = node->padding.left + totalMain + spacing + node->padding.right;
			node->size.height = node->padding.top + maxCross + node->padding.bottom;
		}
		else
		{
			node->size.height = node->padding.top + totalMain + spacing + node->padding.bottom;
			node->size.width = node->padding.left + maxCross + node->padding.right;
		}
		break;
	}
	case SIZER_GROW:
		break;
	}

	float mainPos = (node->layoutDirection == DIRECTION_LEFT_TO_RIGHT) ?
		node->padding.left : node->padding.top;

	for (int i = 0; i < node->childCount; ++i) 
	{
		Node* child = node->children[i];
		if (node->layoutDirection == DIRECTION_LEFT_TO_RIGHT)
		{
			child->position.x = mainPos;

			float crossSpace = node->size.height
				- node->padding.top
				- node->padding.bottom
				- child->size.height;

			switch (node->crossAxisAlignment)
			{
			case ALIGNMENT_LEFT:     // Top
				child->position.y = node->padding.top;
				break;
			case ALIGNMENT_RIGHT:    // Bottom
				child->position.y = node->padding.top + crossSpace;
				break;
			case ALIGNMENT_CENTER:
				child->position.y = node->padding.top + crossSpace / 2;
				break;
			}

			mainPos += child->size.width + node->spacing;
		}
		else // Vertical layout
		{
			child->position.y = mainPos;

			float crossSpace = node->size.width
				- node->padding.left
				- node->padding.right
				- child->size.width;

			switch (node->crossAxisAlignment)
			{
			case ALIGNMENT_LEFT:    // Left
				child->position.x = node->padding.left;
				break;
			case ALIGNMENT_RIGHT:   // Right
				child->position.x = node->padding.left + crossSpace;
				break;
			case ALIGNMENT_CENTER:
				child->position.x = node->padding.left + crossSpace / 2;
				break;
			}

			mainPos += child->size.height + node->spacing;
		}
	}

	node->dirty = 0;
}

Position GetScreenPosition(Node* node)
{
	Position out = { .x = node->position.x, .y = node->position.y };
	Node* parent = node->parent;
	while (parent != NULL)
	{
		out.x += parent->position.x;
		out.y += parent->position.y;

		parent = parent->parent;
	}

	return out;
}