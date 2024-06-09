# Fallout II Visually Enhanced Patch
A patch for the computer game Fallout 2, which modifies the executable in memory for the purpose of improving it visual appearance. 
This is a massive overhaul of my previous patch **The Fallout II High Resolution Patch (HRP)**.

**A work in progress...**

First I have to stress that this patch is not yet complete. It likely contains many bugs and also likely conflicts with some of the modifications made by [sfall](https://github.com/sfall-team/sfall "A set of engine modifications for the classic game Fallout 2 in the form of a DLL, which modifies executable in memory without changing anything in EXE file itself.").
In it's current state it should be used for testing only. You should also only use it with the current version of [sfall](https://github.com/sfall-team/sfall "A set of engine modifications for the classic game Fallout 2 in the form of a DLL, which modifies executable in memory without changing anything in EXE file itself."), as that is what I'm testing it against and I noticed it caused a crash on start when using an older version.

**You must have sfall's directx graphic modes DISABLED when using this patch, not doing so will cause the game to crash on start.**

**To list some of it's features:**
- Rendered with DirectX 11( the HRP used DirectX 9)
- 32 bit graphics, frm's(fallouts graphic) with different palettes can sit side by side. Support for a 32bit frm has also been added, although there is as yet no program to create these. An 8, 24 or 32 bit BMP can be used in place of some of the interface graphics.
- The interface can be scaled to better fit your screen (beyond 2X which was the only option in HRP).
- Game and World maps can be scaled independently, zoomed in and out by way of the mouse wheel.
- A new way to represent lights in-game for a more natural appearance, original lighting is also improved without the palette limitation.
- Improved fading effects between screens.
- Transparency effects when hiding rooves and item and critter visibility when using the fog of war feature(not fully implemented yet).
- Basic map editor, for editing the light colour for objects and their prototypes.
