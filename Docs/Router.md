
# Router

The router is how the UI engine knows what should be displayed and when. It's not too disimilar to react routers or flutter. 
But it is built directly into the UI engine. 


## Using a router

A router must be defined: 
```
local router = Router.new()
```

To then tell the UI engine, this router is to be used. 
```
latte.useRouter(router)
```

`useRouter` first checks if the router is already paired with a window. If it isn't it then creates a window and displays the 
current top of the route stack. 

## Router functions

### `Router:define(pattern, builder)`

Arguments:
 - `pattern` -> A string pattern to match for the route. 
 - `builder` -> A function to build the route. 

This defines a new route for the router, it passes a pattern and a builder function. 
The pattern is a bit like a URL. For instance `/user/dashboard`. 

The function is just a standard lua function. It is fully static so the base function cannot contain any state. You'll need to 
give it a component child to do state. 

---
### `Router:navigate(path, [replace])`

Arguments:
 - `path` -> A string to match against registered routes. 
 - `replace` -> Optional bool, if true it replaces the current path in the route stack with this. Otherwise it just pushes. 

Navigates to the route specified in path as long as it is registered in the router with the `define` function. 
It pushes the route onto the route stack, if replace is passed as true it replaces the route at the top of the stack.  

---
### `Router:back()`

Pops the top route from the stack and goes to the previous one, if a route a exists. 
If a route doesn't exist the window closes. 

---
### `Router:setWindowData(data)`

Arguments: 
 - `data` -> A table containing data to describe the window

Table structure:
 - `size` -> A 2 entry array describing the size, e.g `{ 800, 600 }`
 - `title` -> String to describe the title of the table. Otherwise it is the route name. 
 - `backdrop` -> Optional, Backdrop effect for the window. Platform dependent. 