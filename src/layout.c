#include "layout.h"

#include <malloc.h>
#include <string.h>
#include <assert.h>

#define NODE_ASSIGN_VAL(to, val) node->##to = val; _propogateDirty(node);

typedef void(*PropogateFunc)(Node* node);

static int _max(int x, int y) { return (x > y ? x : y); }

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
	node->crossAxisAlignment = CONTENT_START;

	RelativePositioner(node);

	if (parent)
	{
		NodeAddChild(parent, node);
		_propogateDirty(parent);
	}
	else
	{
		_propogateDirty(node);
	}

	return node;
}

void FreeNode(Node* node)
{
	assert(node);

	for (int i = 0; i < node->childCount; i++)
		FreeNode(node->children[i]);

	free(node->children);
	node->childCapacity = 0;
	node->childCount = 0;

	if (node->flags & NODE_FLAGS_DELETE_USERDATA)
		free(node->userPtr);

	if (node->id)
		free(node->id);
	free(node);
}

void UserData(Node* node, void* userData)
{
	assert(node);
	assert(userData);

	if (userData == NULL)
		return;

	node->userPtr = userData;
}

void NodeAddFlags(Node* node, int flags)
{
	assert(node);

	node->flags |= flags;
}

void NodeAddChild(Node* node, Node* child)
{
	assert(node);
	assert(child);

	child->parent = node;

	if (node->childCount == node->childCapacity) 
	{
		int newCapacity = node->childCapacity ? node->childCapacity * 2 : 2;
		Node** newChildren = (Node**)realloc(node->children, newCapacity * sizeof(Node*));

		if (!newChildren)
			return;

		node->childCapacity = newCapacity;
		node->children = newChildren;
	}
	node->children[node->childCount++] = child;

	_propogateDirty(node);
}

void MainAxisDirection(Node* node, LayoutDirection dir)
{
	NODE_ASSIGN_VAL(layoutDirection, dir)
}

void Sizer(Node* node, float sizerWidth, float sizerHeight)
{
	LayoutSizer newSizer = {
		.widthSizer = sizerWidth,
		.heightSizer = sizerHeight
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

void CrossAxisAlignment(Node* node, ContentAlignment alignment)
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

static void _handleSizer(Node* node)
{
	float fitWidth = 0.0f; 
	float fitHeight = 0.0f; 

	// Is doing this first all in one go faster than doing it with two loops seperately? 
	// Potentially for many children
	if (node->sizer.widthSizer == SIZER_FIT || node->sizer.heightSizer == SIZER_FIT)
	{
		float totalMain = 0, maxCross = 0;
		int numRelChildren = 0;

		for (int i = 0; i < node->childCount; ++i)
		{
			Node* child = node->children[i];
			if (child->positioner.type == POSITIONER_RELATIVE)
			{
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
			fitWidth = node->padding.left + totalMain + spacing + node->padding.right;
			fitHeight = node->padding.top + maxCross + node->padding.bottom;
		}
		else
		{
			fitHeight = node->padding.top + totalMain + spacing + node->padding.bottom;
			fitWidth = node->padding.left + maxCross + node->padding.right;
		}
	}

	// Handle width
	if (node->sizer.widthSizer == SIZER_FIT)
	{
		node->size.width = fitWidth;
	}
	else if (node->sizer.widthSizer >= 0.0f)
	{
		node->size.width = node->sizer.widthSizer; 
	}

	if (node->sizer.heightSizer == SIZER_FIT)
	{
		node->size.height = fitHeight;
	}
	else if (node->sizer.heightSizer >= 0.0f)
	{
		node->size.height = node->sizer.heightSizer;
	}
}

static void _handleGrowSizers(Node* node)
{
	float maxMain = (node->layoutDirection == DIRECTION_LEFT_TO_RIGHT)
		? node->size.width - (node->padding.left + node->padding.right)
		: node->size.height - (node->padding.top + node->padding.bottom);

	int numFlex = 0;
	float fixedMain = 0.0f;
	int N = node->childCount;

	// count flex/fixed, sum fixed sizes
	for (int i = 0; i < N; i++) 
	{
		Node* child = node->children[i];
		if (node->layoutDirection == DIRECTION_LEFT_TO_RIGHT) 
		{
			if (child->sizer.widthSizer == SIZER_GROW)
				numFlex++;
			else if (child->sizer.widthSizer >= SIZER_FIT)
				fixedMain += child->size.width;
		}
		else {
			if (child->sizer.heightSizer == SIZER_GROW)
				numFlex++;
			else if (child->sizer.heightSizer >= SIZER_FIT)
				fixedMain += child->size.height;
		}
	}

	// All spacings are between children: N-1
	float totalSpacing = node->spacing * _max(N - 1, 0);

	// Space for flex children
	float flexTotal = maxMain - fixedMain - totalSpacing;
	float eachFlex = (numFlex > 0 && flexTotal > 0) ? (flexTotal / numFlex) : 0.0f;

	for (int i = 0; i < N; i++) {
		Node* child = node->children[i];
		if (node->layoutDirection == DIRECTION_LEFT_TO_RIGHT) {
			if (child->sizer.widthSizer == SIZER_GROW)
				child->size.width = eachFlex;
			if (child->sizer.heightSizer == SIZER_GROW)
				child->size.height = node->size.height - node->padding.top - node->padding.bottom;
		}
		else {
			if (child->sizer.heightSizer == SIZER_GROW)
				child->size.height = eachFlex;
			if (child->sizer.widthSizer == SIZER_GROW)
				child->size.width = node->size.width - node->padding.left - node->padding.right;
		}
	}
}

static void _handlePositioner(Node* node)
{
	float mainPos = (node->layoutDirection == DIRECTION_LEFT_TO_RIGHT)
		? node->padding.left
		: node->padding.top;

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
			case CONTENT_START:     // Top
				child->position.y = node->padding.top;
				break;
			case CONTENT_END:    // Bottom
				child->position.y = node->padding.top + crossSpace;
				break;
			case CONTENT_CENTER:
				child->position.y = node->padding.top + crossSpace / 2;
				break;
			case CONTENT_SPACE_AROUND:
			{
				assert(0);
				break;
			}
			case CONTENT_SPACE_BETWEEN:
			{
				assert(0);
				break;
			}
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
			case CONTENT_START:    // Left
				child->position.x = node->padding.left;
				break;
			case CONTENT_END:   // Right
				child->position.x = node->padding.left + crossSpace;
				break;
			case CONTENT_CENTER:
				child->position.x = node->padding.left + crossSpace / 2;
				break;
			case CONTENT_SPACE_AROUND:
			{
				assert(0);
				break;
			}
			case CONTENT_SPACE_BETWEEN:
			{
				assert(0);
				break;
			}
			}

			mainPos += child->size.height + node->spacing;
		}
	}
}

void Layout(Node* node)
{
	if (!node) return;

	if (node->dirty == 0) return;

	for (int i = 0; i < node->childCount; ++i)
		Layout(node->children[i]);

	// Calculate the sizes of the elements
	_handleSizer(node);

	// After we have done sizing we can then calculate the grow sizers(flex) 
	_handleGrowSizers(node);

	// Handle the positioning
	_handlePositioner(node);

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