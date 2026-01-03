
-- create a router
local router = Router.new()

function home()
    return {
        children = {
           latte.ui.Container({
                mainAxisAlignment = latte.contentAlignment.center,
                crossAxisAlignment = latte.contentAlignment.center,
                size = { latte.size.grow, latte.size.grow },
                spacing = 16, 
                style = {
                    backgroundColor = latte.color.hex("#fafafa"),
                },
                children = {
                    latte.ui.Text({
                        text = "Welcome to LatteUI!",
                        style = {
                            fontSize = 24,
                            color = latte.color.hex("#333333")
                        }
                    })
                }
            })
        }
    }
end

-- Define routes
router:define("/home", home)

-- Since useRouter hasn't been called, this tells the router to start here when it is
router:navigate("/home")

-- Set window properties for the router
router:setWindowData({
    title = "Basic LatteUI App",
    size = { 400, 300 },
})

-- Creates a window and assigns it to the router
latte.useRouter(router)
