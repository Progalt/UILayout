
/*
	This is a layout library for UIs. 

	It lays it out in a similar method to how web browsers do it

	It contains no logic for appearence, no way to style, just nodes and their layout properties. 
	For styling I recommend using the ID and a style hashmap or pass a style struct as part of the user data for each node. 

	It is fully contained within layout.h and layout.c

	Features: 
		- Node based layouts where just subtrees can be modified and relayed out
		- CSS style flex sizing/positioning
		- Flexible Simple API (Can be learnt in an afternoon if you know CSS) 

	Versions: 
		- 0.1 -> Initial Version
*/

#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H

typedef enum NodeFlags
{
	NODE_FLAGS_NONE = 0,

	// This flag makes the node delete its user data when the node is freed
	// Could be useful if you only ever associate a user data with a node such as styling
	NODE_FLAGS_DELETE_USERDATA = 1

} NodeFlags;

// The node should fit the size of its children on this axis
// Including all padding and spacing
#define SIZER_FIT -1.0f

// Should grow to fit the size of the parent
#define SIZER_GROW -2.0f

// Fixed pixel size
#define SIZER_FIXED(size) size

typedef enum LayoutDirection
{

	DIRECTION_LEFT_TO_RIGHT,
	DIRECTION_TOP_TO_BOTTOM

} LayoutDirection;

//typedef enum Alignment
//{
//	ALIGNMENT_LEFT, 
//	ALIGNMENT_CENTER,
//	ALIGNMENT_RIGHT,
//
//	ALIGNMENT_START = ALIGNMENT_LEFT,
//	ALIGNMENT_END = ALIGNMENT_RIGHT
//} Alignment;

typedef enum ContentAlignment
{
	CONTENT_START, 
	CONTENT_END,
	CONTENT_CENTER,
	CONTENT_SPACE_BETWEEN,
	CONTENT_SPACE_AROUND

} ContentAlignment;

typedef enum PositionerType
{
	POSITIONER_RELATIVE,
	POSITIONER_ABSOLUTE
} PositionerType;

typedef struct Dimension
{
	float width;
	float height;
} Dimension;

typedef struct Position
{
	float x;
	float y;
} Position;

/*
	Represents a size constraint

	Has a min and a max dimension
	When a node has a valid constraint it cannot exceed the max or be smaller 
	than the min
*/
typedef struct Constraint
{
	Dimension min;
	Dimension max;

} Constraint;

typedef struct Margin
{

	float top, left, right, bottom;

} Margin;

typedef struct LayoutSizer
{

	float widthSizer;
	float heightSizer; 

	Constraint constraints;
	Dimension size;

} LayoutSizer;

typedef struct LayoutPositioner
{
	PositionerType type;

	Position position;

} LayoutPositioner;

typedef struct Node
{
	char* id;
	int flags;

	// The direction in which to lay children out on the main axis
	LayoutDirection layoutDirection;

	ContentAlignment crossAxisAlignment;

	// Used to determine how this Node should be sized
	LayoutSizer sizer;

	// Used to determine the position
	LayoutPositioner positioner;

	// Padding
	Margin padding;

	// Spacing between child elements
	float spacing;

	//	=================================================
	//					Final Layout
	//	=================================================

	// Is the Node currently visible on the screen
	int visible;

	// Post layout size
	Dimension size;

	// Post layout position
	// Relative to its parent node
	Position position;

	//	=================================================
	//					Node Tree
	//	=================================================

	struct Node* parent; 

	struct Node** children;
	int childCount;
	int childCapacity;

	// Has the layout changed and needs to be redone 
	int dirty;

	// User data passed in for anything they may want
	void* userPtr;

} Node;

/*
	Pass in an optional ID and optional parent. 
	Pass NULL to both if you don't care at this point.

	It is recommended to pass in ids for easier identification later
*/
Node* CreateNode(const char* id, Node* parent);

/*
	Free all memory associated with a node and its children
*/
void FreeNode(Node* node);

/*
	Attach some user data to the Node

	if you want this deleted with the node see NodeAddFlags
*/
void UserData(Node* node, void* userData);

/*
	Pass in some flags to determine how they layout engine might 
	handle the Node.

	See NodeFlags
*/
void NodeAddFlags(Node* node, int flags);

/*
	Add a child to the node

	The node takes ownership of memory associated with its children
	So you only need to make sure you free any root nodes
*/
void NodeAddChild(Node* node, Node* child);

/*
	Set the main axis direction for laying out children
*/
void MainAxisDirection(Node* node, LayoutDirection dir);

/*
	Pass in a sizer for each axis

	This can be either SIZER_GROW, SIZER_FIT or SIZER_FIXED(x)
*/
void Sizer(Node* node, float sizerWidth, float sizerHeight);

/*
	Set the node to be a relative positioner

	This is the default and is automatically applied on node creation
*/
void RelativePositioner(Node* node);

/*
	Set the node to be an absolute positioner

	This takes a position and places it in the absolute position within its parent
*/
void AbsolutePositioner(Node* node, float relX, float relY);



/*
	Set the alignment on the cross axis, so the opposite direction to the layout 
	direction
*/
void CrossAxisAlignment(Node* node, ContentAlignment alignment);



/*
	Sets the padding to be same on every side
*/
void Padding(Node* node, float padding);

/*
	Individually set the padding for each direction
*/
void PaddingRLTB(Node* node, float right, float left, float top, float bottom);

/*
	Apply padding based on the axis
*/
void PaddingAxis(Node* node, float vert, float horz);

/*
	Set the spacing between child nodes
*/
void Spacing(Node* node, float gap);

// ===========================================
//				Apply Layout
// ===========================================

/*
	Layout the UI from the passed in base node.
*/
void Layout(Node* node);

/*
	Returns the screen position of the node

	This is fairly slow as it traverse up the parent list of the node and 
	adds up all relative positions to get the absolute
*/
Position GetScreenPosition(Node* node);

#endif // UI_LAYOUT_H