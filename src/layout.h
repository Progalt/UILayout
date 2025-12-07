
#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H

typedef enum SizerType
{
	SIZER_FIT,
	SIZER_GROW,
	SIZER_FIXED

} SizerType;

typedef enum LayoutDirection
{

	DIRECTION_LEFT_TO_RIGHT,
	DIRECTION_TOP_TO_BOTTOM

} LayoutDirection;

typedef enum Alignment
{
	ALIGNMENT_LEFT, 
	ALIGNMENT_CENTER,
	ALIGNMENT_RIGHT,

	ALIGNMENT_START = ALIGNMENT_LEFT,
	ALIGNMENT_END = ALIGNMENT_RIGHT
} Alignment;

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
	SizerType type;

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

	// The direction in which to lay children out on the main axis
	LayoutDirection layoutDirection;

	Alignment crossAxisAlignment;

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

	void* userPtr;

} Node;

/*
	Pass in an optional ID and optional parent. 
	Pass NULL to both if you don't care at this point
*/
Node* CreateNode(const char* id, Node* parent);
void FreeNode(Node* node);


void NodeAddChild(Node* node, Node* child);

void MainAxisDirection(Node* node, LayoutDirection dir);

void FixedSizer(Node* node, float width, float height);
void FitSizer(Node* node);


void RelativePositioner(Node* node);
void AbsolutePositioner(Node* node, float relX, float relY);

void CrossAxisAlignment(Node* node, Alignment alignment);

void Padding(Node* node, float padding);
void PaddingRLTB(Node* node, float right, float left, float top, float bottom);
void PaddingAxis(Node* node, float vert, float horz);

/*
	Set the spacing between child nodes
*/
void Spacing(Node* node, float gap);

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