# Components

Components are what make up most UI. 
A component can be made up of many other components. 

Components take props (Properties), these are passed as tables and contain all information to describe how a component should look and behave. 

When making custom components you can decide what props are needed a few functions exist to do some of the work automatically if that is required.

Props also typically contain a sub-table called `style` to describe how the component looks. This might not be used for custom components or might be limited but built-in components take styles like in a CSS-like naming scheme.

## Built-In Components
These components are built-in to LatteUI, they typically have custom logic on the C++ side as well as the Lua side. 
### Text
Found at: `latte.ui.Text`  
Component to render a string of text. 
#### Properties
- `text` -> The text string to display. This can either be passed as the table key `text` or as the positional argument to the table. 
- `style` -> See Styling

#### Styling

- `color` -> Color to use for the text
- `fontSize` -> Font size in pixels

The style table with its default values. 
This will be merged with the props table and overwrite values you specify. 
```lua
style = {
    color = latte.color.hex("#FFFFFF"),
    fontSize = 14
}
```

#### Example

```
latte.ui.Text({
    "Hello Text",
    style = {
        color = latte.colors.cornflowerblue,
        fontSize = 24
    }
})
```

### Container
Found at: `latte.ui.Container`  
A basic layout element can have children. 
Mostly equivalent to a HTML div. 