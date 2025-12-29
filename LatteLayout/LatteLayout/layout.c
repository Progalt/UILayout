#include "layout.h"

#include <malloc.h>
#include <string.h>
#include <assert.h>

// strdup is non-standard C before C23 and is a posix function
// It exists in MSVC but it complains if it isn't _strdup
// To avoid compiler warnings with MSVC
#ifdef _MSC_VER
#define strdup _strdup
#endif

#define NODE_ASSIGN_VAL(to, val) \
    do {                         \
        node->to = val;          \
        lattePropogateDirty(node);   \
    } while (0);

typedef void(*PropogateFunc)(LatteNode* node);

static int latteMax(int x, int y) { return (x > y ? x : y); }

// Propogates a function call down the tree from the supplied root node. 
void lattePropogate(LatteNode* node, PropogateFunc func)
{
	func(node);

	for (int i = 0; i < node->childCount; i++)
		lattePropogate(node->children[i], func);
}

// Helper to set a node to dirty
// Mostly exists to just pass to lattePropogate
void latteSetDirty(LatteNode* node)
{
	node->dirty = 1;
}

// Propogate a dirty state down the node tree
void lattePropogateDirty(LatteNode* node)
{
	lattePropogate(node, latteSetDirty);
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

		// If this node has been assigned to a parent
		// Make that propogate dirty from that parent
		// As this new node may change how the parent lays out everything
		lattePropogateDirty(parent);
	}
	else
	{
		lattePropogateDirty(node);
	}

	node->flags = flags;

	return node;
}

void latteFreeNode(LatteNode* node)
{
	assert(node);

	latteOrphanNode(node);

	for (int i = 0; i < node->childCount; i++)
		latteFreeNode(node->children[i]);

	free(node->children);
	node->childCapacity = 0;
	node->childCount = 0;

	if (node->flags & LATTE_NODE_FLAGS_DELETE_USERDATA)
	{
		if (node->userDataDeleter)
			node->userDataDeleter(node->userPtr);
		else 
			free(node->userPtr);
	}

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

void latteSetUserDataDeleter(LatteNode* node, LatteUserDataDeleter deleter)
{
	assert(node);

	node->userDataDeleter = deleter;
}

void* latteGetUserData(LatteNode* node)
{
	assert(node);

	if (node->userPtr == NULL)
		return NULL;

	return node->userPtr;
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

	lattePropogateDirty(node);
}

void latteOrphanNode(LatteNode* node)
{
	if (!node || !node->parent)
		return;

	LatteNode* parent = node->parent;
	int foundIdx = -1;

	// Find this node in its parent's children array
	for (int i = 0; i < parent->childCount; ++i)
	{
		if (parent->children[i] == node)
		{
			foundIdx = i;
			break;
		}
	}

	if (foundIdx == -1)
		return; 

	// Shift remaining children down
	for (int i = foundIdx; i < parent->childCount - 1; ++i)
	{
		parent->children[i] = parent->children[i + 1];
	}

	parent->childCount -= 1;
	parent->children[parent->childCount] = NULL;

	// Mark node as orphaned
	node->parent = NULL;
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
	// Handle width (non-fit only)
	if (node->sizer.widthSizer >= 0.0f)
	{
		node->size.width = node->sizer.widthSizer;
	}

	// Handle height (non-fit only)
	if (node->sizer.heightSizer >= 0.0f)
	{
		node->size.height = node->sizer.heightSizer;
	}
}

static void _handleFitSizer(LatteNode* node)
{
	if (node->sizer.widthSizer != LATTE_SIZER_FIT && node->sizer.heightSizer != LATTE_SIZER_FIT) {
		return; // No fit sizing needed
	}

	float fitWidth = 0.0f;
	float fitHeight = 0.0f;
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
				maxCross = latteMax(maxCross, child->size.height);
			}
			else
			{
				totalMain += child->size.height;
				maxCross = latteMax(maxCross, child->size.width);
			}
		}
	}

	float spacing = (numRelChildren > 1) ? (numRelChildren - 1) * node->spacing : 0;

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

	if (node->sizer.widthSizer == LATTE_SIZER_FIT)
		node->size.width = fitWidth;

	if (node->sizer.heightSizer == LATTE_SIZER_FIT)
		node->size.height = fitHeight;
}

static void _handleGrowSizers(LatteNode* node)
{
	float maxMain = (node->layoutDirection == LATTE_DIRECTION_HORIZONTAL)
		? node->size.width - (node->padding.left + node->padding.right)
		: node->size.height - (node->padding.top + node->padding.bottom);

	int numFlex = 0;
	int numRelChildren = 0;
	float fixedMain = 0.0f;
	int N = node->childCount;

	// Count flex/fixed, sum fixed sizes - only for relatively positioned children
	for (int i = 0; i < N; i++)
	{
		LatteNode* child = node->children[i];

		// Only consider relatively positioned children
		if (child->positioner.type != LATTE_POSITIONER_RELATIVE)
			continue;

		numRelChildren++;

		if (node->layoutDirection == LATTE_DIRECTION_HORIZONTAL)
		{
			if (child->sizer.widthSizer == LATTE_SIZER_GROW)
				numFlex++;
			else if (child->sizer.widthSizer > LATTE_SIZER_GROW) 
				fixedMain += child->size.width;
		}
		else
		{
			if (child->sizer.heightSizer == LATTE_SIZER_GROW)
				numFlex++;
			else if (child->sizer.heightSizer > LATTE_SIZER_GROW) 
				fixedMain += child->size.height;
		}
	}

	// All spacings are between relatively positioned children
	float totalSpacing = node->spacing * latteMax(numRelChildren - 1, 0);

	// Space for flex children
	float flexTotal = maxMain - fixedMain - totalSpacing;
	float eachFlex = (numFlex > 0 && flexTotal > 0) ? (flexTotal / numFlex) : 0.0f;

	for (int i = 0; i < N; i++) {
		LatteNode* child = node->children[i];

		// Only apply grow sizing to relatively positioned children
		if (child->positioner.type != LATTE_POSITIONER_RELATIVE)
			continue;

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

	// 1. Calculate main axis sizes (only for relative children)
	float totalChildSize = 0.0f;
	int relativeChildCount = 0;

	for (int i = 0; i < childCount; ++i) {
		LatteNode* child = node->children[i];
		if (child->positioner.type == LATTE_POSITIONER_RELATIVE) {
			totalChildSize += (node->layoutDirection == LATTE_DIRECTION_HORIZONTAL)
				? child->size.width
				: child->size.height;
			relativeChildCount++;
		}
	}

	float totalSpacing = node->spacing * ((relativeChildCount > 1) ? (relativeChildCount - 1) : 0);
	float mainAxisSize = (node->layoutDirection == LATTE_DIRECTION_HORIZONTAL)
		? node->size.width - node->padding.left - node->padding.right
		: node->size.height - node->padding.top - node->padding.bottom;

	float usedSpace = totalChildSize + totalSpacing;
	float remainingSpace = mainAxisSize - totalChildSize - totalSpacing;

	// 2. Determine mainPos and spacing per alignment (only affects relative children)
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
		spacing = (relativeChildCount > 1) ? (remainingSpace / (relativeChildCount - 1)) : 0.0f;
		break;
	case LATTE_CONTENT_SPACE_AROUND:
		spacing = (relativeChildCount > 0)
			? (remainingSpace / relativeChildCount)
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

		if (child->positioner.type == LATTE_POSITIONER_ABSOLUTE) {
			// Absolute positioning - use specified position relative to parent
			child->position.x = child->positioner.position.x;
			child->position.y = child->positioner.position.y;
			continue;
		}

		// Relative positioning (existing logic)
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

	// First, calculate THIS node's sizes
	_handleSizer(node);

	// Then handle grow sizers for THIS node's children
	// This ensures children have correct sizes before positioning
	_handleGrowSizers(node);

	// Now recursively layout children with their correct sizes
	for (int i = 0; i < node->childCount; ++i)
		latteLayout(node->children[i]);

	_handleFitSizer(node);

	// Finally, position the children based on the finalized sizes
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

void latteGetScreenBoundingBox(LatteNode* node, float bb[4])
{
	LattePosition pos = latteGetScreenPosition(node);
	bb[0] = pos.x;
	bb[1] = pos.y;
	bb[2] = bb[0] + node->size.width;
	bb[3] = bb[1] + node->size.height;
}