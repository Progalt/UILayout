#include "latteLayout.h"

#include <malloc.h>
#include <string.h>
#include <assert.h>

#define NODE_ASSIGN_VAL(to, val) node->##to = val; _propogateDirty(node);

typedef void(*PropogateFunc)(LatteNode* node);

static int _max(int x, int y) { return (x > y ? x : y); }

static void _propogate(LatteNode* node, PropogateFunc func)
{
	func(node);

	for (int i = 0; i < node->childCount; i++)
		_propogate(node->children[i], func);
}

static void _setDirty(LatteNode* node)
{
	node->dirty = 1;
}

static void _propogateDirty(LatteNode* node)
{
	_propogate(node, _setDirty);
}

LatteNode* latteCreateNode(const char* id, LatteNode* parent, int flags)
{

	LatteNode* node = (LatteNode*)malloc(sizeof(LatteNode));

	if (node == NULL)
		return NULL;

	memset(node, 0, sizeof(LatteNode));

	if (strlen(id) > 0)
	{
		node->id = strdup(id);
	}

	node->layoutDirection = LATTE_DIRECTION_HORIZONTAL;
	node->crossAxisAlignment = LATTE_CONTENT_START;
	node->mainAxisAlignment = LATTE_CONTENT_START;

	latteRelativePositioner(node);

	if (parent)
	{
		latteNodeAddChild(parent, node);
		_propogateDirty(parent);
	}
	else
	{
		_propogateDirty(node);
	}

	node->flags = flags;

	return node;
}

void latteFreeNode(LatteNode* node)
{
	assert(node);

	for (int i = 0; i < node->childCount; i++)
		latteFreeNode(node->children[i]);

	free(node->children);
	node->childCapacity = 0;
	node->childCount = 0;

	if (node->flags & LATTE_NODE_FLAGS_DELETE_USERDATA)
		free(node->userPtr);

	if (node->id)
		free(node->id);
	free(node);
}

void latteUserData(LatteNode* node, void* userData)
{
	assert(node);
	assert(userData);

	if (userData == NULL)
		return;

	node->userPtr = userData;
}

void latteNodeAddFlags(LatteNode* node, int flags)
{
	assert(node);

	node->flags |= flags;
}

void latteNodeAddChild(LatteNode* node, LatteNode* child)
{
	assert(node);
	assert(child);

	child->parent = node;

	if (node->childCount == node->childCapacity) 
	{
		int newCapacity = node->childCapacity ? node->childCapacity * 2 : 2;
		LatteNode** newChildren = (LatteNode**)realloc(node->children, newCapacity * sizeof(LatteNode*));

		if (!newChildren)
			return;

		node->childCapacity = newCapacity;
		node->children = newChildren;
	}
	node->children[node->childCount++] = child;

	_propogateDirty(node);
}

void latteMainAxisDirection(LatteNode* node, LatteLayoutDirection dir)
{
	NODE_ASSIGN_VAL(layoutDirection, dir)
}

void latteSizer(LatteNode* node, float sizerWidth, float sizerHeight)
{
	LatteLayoutSizer newSizer = {
		.widthSizer = sizerWidth,
		.heightSizer = sizerHeight
	};

	NODE_ASSIGN_VAL(sizer, newSizer)
}

void latteRelativePositioner(LatteNode* node)
{
	LatteLayoutPositioner newPositioner = {
		.type = LATTE_POSITIONER_RELATIVE,
		.position = { .x = 0.0f, .y = 0.0f }
	};

	NODE_ASSIGN_VAL(positioner, newPositioner)
}

void latteAbsolutePositioner(LatteNode* node, float relX, float relY)
{
	LatteLayoutPositioner newPositioner = {
		.type = LATTE_POSITIONER_ABSOLUTE,
		.position = { .x = relX, .y = relY }
	};

	NODE_ASSIGN_VAL(positioner, newPositioner)
}

void latteMainAxisAlignment(LatteNode* node, LatteContentAlignment alignment)
{
	NODE_ASSIGN_VAL(mainAxisAlignment, alignment)
}

void latteCrossAxisAlignment(LatteNode* node, LatteContentAlignment alignment)
{
	NODE_ASSIGN_VAL(crossAxisAlignment, alignment)
}

void lattePadding(LatteNode* node, float padding)
{
	lattePaddingAxis(node, padding, padding);
}

void lattePaddingRLTB(LatteNode* node, float right, float left, float top, float bottom)
{
	LatteMargin margin = {
		.top = top,
		.left = left,
		.bottom = bottom,
		.right = right
	};

	NODE_ASSIGN_VAL(padding, margin)
}

void lattePaddingAxis(LatteNode* node, float vert, float horz)
{
	lattePaddingRLTB(node, horz, vert, vert, horz);
}

void latteSpacing(LatteNode* node, float gap)
{
	NODE_ASSIGN_VAL(spacing, gap)
}

static void _handleSizer(LatteNode* node)
{
	float fitWidth = 0.0f; 
	float fitHeight = 0.0f; 

	// Is doing this first all in one go faster than doing it with two loops seperately? 
	// Potentially for many children
	if (node->sizer.widthSizer == LATTE_SIZER_FIT || node->sizer.heightSizer == LATTE_SIZER_FIT)
	{
		float totalMain = 0, maxCross = 0;
		int numRelChildren = 0;

		for (int i = 0; i < node->childCount; ++i)
		{
			LatteNode* child = node->children[i];
			if (child->positioner.type == LATTE_POSITIONER_RELATIVE)
			{
				++numRelChildren;
				if (node->layoutDirection == LATTE_DIRECTION_HORIZONTAL)
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

		if (node->layoutDirection == LATTE_DIRECTION_HORIZONTAL)
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
	if (node->sizer.widthSizer == LATTE_SIZER_FIT)
	{
		node->size.width = fitWidth;
	}
	else if (node->sizer.widthSizer >= 0.0f)
	{
		node->size.width = node->sizer.widthSizer; 
	}

	if (node->sizer.heightSizer == LATTE_SIZER_FIT)
	{
		node->size.height = fitHeight;
	}
	else if (node->sizer.heightSizer >= 0.0f)
	{
		node->size.height = node->sizer.heightSizer;
	}
}

static void _handleGrowSizers(LatteNode* node)
{
	float maxMain = (node->layoutDirection == LATTE_DIRECTION_HORIZONTAL)
		? node->size.width - (node->padding.left + node->padding.right)
		: node->size.height - (node->padding.top + node->padding.bottom);

	int numFlex = 0;
	float fixedMain = 0.0f;
	int N = node->childCount;

	// count flex/fixed, sum fixed sizes
	for (int i = 0; i < N; i++) 
	{
		LatteNode* child = node->children[i];
		if (node->layoutDirection == LATTE_DIRECTION_HORIZONTAL) 
		{
			if (child->sizer.widthSizer == LATTE_SIZER_GROW)
				numFlex++;
			else if (child->sizer.widthSizer >= LATTE_SIZER_FIT)
				fixedMain += child->size.width;
		}
		else {
			if (child->sizer.heightSizer == LATTE_SIZER_GROW)
				numFlex++;
			else if (child->sizer.heightSizer >= LATTE_SIZER_FIT)
				fixedMain += child->size.height;
		}
	}

	// All spacings are between children: N-1
	float totalSpacing = node->spacing * _max(N - 1, 0);

	// Space for flex children
	float flexTotal = maxMain - fixedMain - totalSpacing;
	float eachFlex = (numFlex > 0 && flexTotal > 0) ? (flexTotal / numFlex) : 0.0f;

	for (int i = 0; i < N; i++) {
		LatteNode* child = node->children[i];
		if (node->layoutDirection == LATTE_DIRECTION_HORIZONTAL) {
			if (child->sizer.widthSizer == LATTE_SIZER_GROW)
				child->size.width = eachFlex;
			if (child->sizer.heightSizer == LATTE_SIZER_GROW)
				child->size.height = node->size.height - node->padding.top - node->padding.bottom;
		}
		else {
			if (child->sizer.heightSizer == LATTE_SIZER_GROW)
				child->size.height = eachFlex;
			if (child->sizer.widthSizer == LATTE_SIZER_GROW)
				child->size.width = node->size.width - node->padding.left - node->padding.right;
		}
	}
}

static void _handlePositioner(LatteNode* node)
{
	int childCount = node->childCount;
	if (childCount == 0) return;

	// 1. Calculate main axis sizes
	float totalChildSize = 0.0f;
	for (int i = 0; i < childCount; ++i) {
		LatteNode* child = node->children[i];
		totalChildSize += (node->layoutDirection == LATTE_DIRECTION_HORIZONTAL)
			? child->size.width
			: child->size.height;
	}
	float totalSpacing = node->spacing * ((childCount > 1) ? (childCount - 1) : 0);
	float mainAxisSize = (node->layoutDirection == LATTE_DIRECTION_HORIZONTAL)
		? node->size.width - node->padding.left - node->padding.right
		: node->size.height - node->padding.top - node->padding.bottom;

	float usedSpace = totalChildSize + totalSpacing;
	float remainingSpace = mainAxisSize - totalChildSize - totalSpacing;

	// 2. Determine mainPos and spacing per alignment
	float mainPos, spacing = node->spacing;

	switch (node->mainAxisAlignment) {
	case LATTE_CONTENT_START:
		mainPos = (node->layoutDirection == LATTE_DIRECTION_HORIZONTAL)
			? node->padding.left
			: node->padding.top;
		break;
	case LATTE_CONTENT_END:
		mainPos = ((node->layoutDirection == LATTE_DIRECTION_HORIZONTAL)
			? node->padding.left
			: node->padding.top) + remainingSpace;
		break;
	case LATTE_CONTENT_CENTER:
		mainPos = ((node->layoutDirection == LATTE_DIRECTION_HORIZONTAL)
			? node->padding.left
			: node->padding.top) + remainingSpace / 2.0f;
		break;
	case LATTE_CONTENT_SPACE_BETWEEN:
		mainPos = (node->layoutDirection == LATTE_DIRECTION_HORIZONTAL)
			? node->padding.left
			: node->padding.top;
		spacing = (childCount > 1) ? (remainingSpace / (childCount - 1)) : 0.0f;
		break;
	case LATTE_CONTENT_SPACE_AROUND:
		spacing = (childCount > 0)
			? (remainingSpace / childCount)
			: 0.0f;
		mainPos = ((node->layoutDirection == LATTE_DIRECTION_HORIZONTAL)
			? node->padding.left
			: node->padding.top) + spacing / 2.0f;
		break;
	default:
		mainPos = (node->layoutDirection == LATTE_DIRECTION_HORIZONTAL)
			? node->padding.left
			: node->padding.top;
		break;
	}

	// 3. Position children
	for (int i = 0; i < childCount; ++i)
	{
		LatteNode* child = node->children[i];

		if (node->layoutDirection == LATTE_DIRECTION_HORIZONTAL)
		{
			child->position.x = mainPos;

			float crossSpace = node->size.height
				- node->padding.top
				- node->padding.bottom
				- child->size.height;
			switch (node->crossAxisAlignment)
			{
			case LATTE_CONTENT_START: // Top
				child->position.y = node->padding.top;
				break;
			case LATTE_CONTENT_END: // Bottom
				child->position.y = node->padding.top + crossSpace;
				break;
			case LATTE_CONTENT_CENTER:
				child->position.y = node->padding.top + crossSpace / 2;
				break;
			default:
				child->position.y = node->padding.top;
				break;
			}

			mainPos += child->size.width + spacing;
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
			case LATTE_CONTENT_START: // Left
				child->position.x = node->padding.left;
				break;
			case LATTE_CONTENT_END: // Right
				child->position.x = node->padding.left + crossSpace;
				break;
			case LATTE_CONTENT_CENTER:
				child->position.x = node->padding.left + crossSpace / 2;
				break;
			default:
				child->position.x = node->padding.left;
				break;
			}

			mainPos += child->size.height + spacing;
		}
	}
}

void latteLayout(LatteNode* node)
{
	if (!node) return;

	if (node->dirty == 0) return;

	for (int i = 0; i < node->childCount; ++i)
		latteLayout(node->children[i]);

	// Calculate the sizes of the elements
	_handleSizer(node);

	// After we have done sizing we can then calculate the grow sizers(flex) 
	_handleGrowSizers(node);

	// Handle the positioning
	_handlePositioner(node);

	node->dirty = 0;
}

LattePosition latteGetScreenPosition(LatteNode* node)
{
	LattePosition out = { .x = node->position.x, .y = node->position.y };
	LatteNode* parent = node->parent;
	while (parent != NULL)
	{
		out.x += parent->position.x;
		out.y += parent->position.y;

		parent = parent->parent;
	}

	return out;
}