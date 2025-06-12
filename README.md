# QuakeX 1.01

# Story time

This is the base QuakeX 1.01 source code.
That is SDLQuake ported to the original Xbox console by LantusX back in the hayday of Xbox homebrew. (still other excellent projects and developments happening. But, new releases were rapid fire back then.)
I worked with Lantus on adding console friendly and general quality of life improvements to the version of QuakeX that was then labeled 1.01. 

IIRC, Lantus suffered some data loss around that time. I lost a 500GB drive myself while reoganizing my archives and had not backup. 
I had a habit of storing source on my Dev xbox. (modded retail) That xbox died around the same time thanks to trace damage from a bad pogo pin modchip.
That source code was never released and considered lost.

I never trashed that xbox. And... recently recovered the hard drive from it.
On that drive were two zip files. "QuakeXOnlinesrc.zip" and "QuakeX_v0.2-Lantus.zip"

QuakeXOnline.zip was the 1.01 source. The other, the source it was based off of. 
QuakeX 1.00 did not have network support. The v0.2 from Lantus does. 

# About

There was one line of code in the source here that is changed from that zip. It caused the game to crash to console on map load. As it is here, it compiles and runs as it should. A lot of unnecessary files have been removed. But, a lot still remain. Unsure what may be useful during optimizations and updates in the future. Open GL source is included but unusued. As is linux, sun, dos, vga, svga, and other such things. Can the Assembly code be merged in for speedups? I know most modern engines abandoned it. But this is a 25 year old device and 30 year old code.

My own findings are that the newer 1.1+ SDLx versions work, but for some reason framerate is limited to 21FPS. I was able to find an older SDLx 0.9 release and that resolved the issue. I have not yet tried the newer SDLx2 available through RXDK.

# Plans

I will leave this as is and create a new fork for any changes I make. I plan to investigate merging D3DQuakeX into this and also performing various additional modifications and upgrades. No promises though.

# License
Based off GPL licensed Quake source code. So, follow those rules. Except, also relys on the XDK... so do so at your own risk.
