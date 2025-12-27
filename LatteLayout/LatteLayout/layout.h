
/*
	LatteLayout 

	This is a layout library for UIs. 

	It lays it out in a similar method to how web browsers do it

	It contains no logic for appearence, no way to style, just nodes and their layout properties. 
	For styling I recommend using the ID and a style hashmap or pass a style struct as part of the user data for each node. 

	It is fully contained within latteLayout.h and latteLayout.c

	Features: 
		- Node based layouts where just subtrees can be modified and relayed out
		- CSS style flex sizing/positioning
		- Flexible Simple API (Can be learnt in an afternoon if you know CSS) 

	Versions: 
		- 0.1 -> Initial Version
*/

#ifndef LATTE_LAYOUT_H
#define LATTE_LAYOUT_H

typedef enum LatteNodeFlags
{
	LATTE_NODE_FLAGS_NONE = 0,

	// This flag makes the node delete its user data when the node is freed
	// Could be useful if you only ever associate a user data with a node such as styling
	LATTE_NODE_FLAGS_DELETE_USERDATA = 1

} LatteNodeFlags;

// The node should fit the size of its children on this axis
// Including all padding and spacing
#define LATTE_SIZER_FIT -1.0f

// Should grow to fit the size of the parent
#define LATTE_SIZER_GROW -2.0f

// Fixed pixel size
#define LATTE_SIZER_FIXED(size) size

typedef enum LatteLayoutDirection
{

	LATTE_DIRECTION_HORIZONTAL,
	LATTE_DIRECTION_VERTICAL

} LatteLayoutDirection;

typedef enum LatteContentAlignment
{
	LATTE_CONTENT_START, 
	LATTE_CONTENT_END,
	LATTE_CONTENT_CENTER,
	LATTE_CONTENT_SPACE_BETWEEN,
	LATTE_CONTENT_SPACE_AROUND

} LatteContentAlignment;

typedef enum LattePositionerType
{
	LATTE_POSITIONER_RELATIVE,
	LATTE_POSITIONER_ABSOLUTE

} LattePositionerType;

typedef struct LatteDimension
{
	float width;
	float height;

} LatteDimension;

typedef struct LattePosition
{
	float x;
	float y;

} LattePosition;

/*
	Represents a size constraint

	Has a min and a max dimension
	When a node has a valid constraint it cannot exceed the max or be smaller 
	than the min
*/
typedef struct LatteConstraint
{
	LatteDimension min;
	LatteDimension max;

} LatteConstraint;

typedef struct LatteMargin
{

	float top, left, right, bottom;

} LatteMargin;

typedef struct LatteLayoutSizer
{

	float widthSizer;
	float heightSizer; 

	LatteConstraint constraints;
	LatteDimension size;

} LatteLayoutSizer;

typedef struct LatteLayoutPositioner
{
	LattePositionerType type;

	LattePosition position;

} LatteLayoutPositioner;

typedef void(*LatteUserDataDeleter)(void*);

typedef struct LatteNode
{
	char* id;
	int flags;

	// The direction in which to lay children out on the main axis
	LatteLayoutDirection layoutDirection;

	LatteContentAlignment mainAxisAlignment;
	LatteContentAlignment crossAxisAlignment;

	// Used to determine how this Node should be sized
	LatteLayoutSizer sizer;

	// Used to determine the position
	LatteLayoutPositioner positioner;

	// Padding
	LatteMargin padding;

	// Spacing between child elements
	float spacing;

	//	=================================================
	//					Final Layout
	//	=================================================

	// Is the Node currently visible on the screen
	int visible;

	// Post layout size
	LatteDimension size;

	// Post layout position
	// Relative to its parent node
	LattePosition position;

	//	=================================================
	//					Node Tree
	//	=================================================

	struct LatteNode* parent; 

	struct LatteNode** children;
	int childCount;
	int childCapacity;

	// Has the layout changed and needs to be redone 
	int dirty;

	// User data passed in for anything they may want
	void* userPtr;
	LatteUserDataDeleter userDataDeleter;

} LatteNode;

/*
	Pass in an optional ID and optional parent. 
	Pass NULL to both if you don't care at this point.

	It is recommended to pass in ids for easier identification later
*/
LatteNode* latteCreateNode(const char* id, LatteNode* parent, int flags);

/*
	Free all memory associated with a node and its children
*/
void latteFreeNode(LatteNode* node);

typedef void(*PropogateFunc)(LatteNode* node);

void lattePropogate(LatteNode* node, PropogateFunc func);

void latteSetDirty(LatteNode* node);

void lattePropogateDirty(LatteNode* node);

/*
	Attach some user data to the Node

	if you want this deleted with the node see latteNodeAddFlags
*/
void latteUserData(LatteNode* node, void* userData);

void* latteGetUserData(LatteNode* node);

void latteSetUserDataDeleter(LatteNode* node, LatteUserDataDeleter deleter);

/*
	Pass in some flags to determine how they layout engine might 
	handle the Node.

	See LatteNodeFlags
*/
void latteNodeAddFlags(LatteNode* node, int flags);

/*
	Add a child to the node

	The node takes ownership of memory associated with its children
	So you only need to make sure you free any root nodes
*/
void latteNodeAddChild(LatteNode* node, LatteNode* child);

/*
	Set the main axis direction for laying out children
*/
void latteMainAxisDirection(LatteNode* node, LatteLayoutDirection dir);

/*
	Pass in a sizer for each axis

	This can be either LATTE_SIZER_GROW, LATTE_SIZER_FIT or LATTE_SIZER_FIXED(x)
*/
void latteSizer(LatteNode* node, float sizerWidth, float sizerHeight);

/*
	Set the node to be a relative positioner

	This is the default and is automatically applied on node creation
*/
void latteRelativePositioner(LatteNode* node);

/*
	Set the node to be an absolute positioner

	This takes a position and places it in the absolute position within its parent
*/
void latteAbsolutePositioner(LatteNode* node, float relX, float relY);


void latteMainAxisAlignment(LatteNode* node, LatteContentAlignment alignment);

/*
	Set the alignment on the cross axis, so the opposite direction to the layout 
	direction
*/
void latteCrossAxisAlignment(LatteNode* node, LatteContentAlignment alignment);



/*
	Sets the padding to be same on every side
*/
void lattePadding(LatteNode* node, float padding);

/*
	Individually set the padding for each direction
*/
void lattePaddingRLTB(LatteNode* node, float right, float left, float top, float bottom);

/*
	Apply padding based on the axis
*/
void lattePaddingAxis(LatteNode* node, float vert, float horz);

/*
	Set the spacing between child nodes
*/
void latteSpacing(LatteNode* node, float gap);

// ===========================================
//				Apply Layout
// ===========================================

/*
	Layout the UI from the passed in base node.
*/
void latteLayout(LatteNode* node);

/*
	Returns the screen position of the node

	This is fairly slow as it traverse up the parent list of the node and 
	adds up all relative positions to get the absolute
*/
LattePosition latteGetScreenPosition(LatteNode* node);

void latteGetScreenBoundingBox(LatteNode* node, float bb[4]);

#endif // LATTE_LAYOUT_H