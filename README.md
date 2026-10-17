# XShadowBind
A sneaky key bind solution for Linux Xorg/X11 that doesn't suck as much.

## Example usage
my_binds_script.sh
```
xshadowbind \
  --bind Super_L+E thunar \
  --bind Super_L+Shift+S "flameshot -gui -c -s" \
  --bind Super_L+L my_lockscreen_commab
```

Use `xshadowbind --help` to get detailed help.

## Why?
### The problem
`xbindkeys` and desktop environments like `i3`, to implement keybinds, use a X11 function that explicitly grabs the specified key combination.

This is problematic, because whenever the key combination is pressed, the currently focused window gets briefly unfocused, which may cause it to close.
Also, some menus grab every single key, which prevents key binds from being detected at all.

This problem renders screenshotting menus impossible, because they either won't let you launch your screenshotting tool or will close before it can even start.

### The solution
`xshadowbind` instead of grabbing the key combinations, it listens to all input events and matches the currently pressed keys against a list of user-defined combos.

Other applications have no idea about the combos, that's why this program has *shadow* in its name.

### Tradeoff
Since this program doesn't grab the keys, they will still be handled by other applications.

For example if you bind `Control_L+F` (Left Ctrl + F) to a command, it will both run your configured command, but also still open a text search tool in a text editor application.


## Building
You need to have `make` and `gcc` installed on your system.

Then you simply run
```
make
```
in the root directory of the project, and the compiled `xshadowbind` executable will appear.
