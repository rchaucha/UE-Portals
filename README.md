# Portals plugin for Unreal Engine  
This plugin adds customizable portals to Unreal Engine.  
It is based on the code of [froyok](https://www.froyok.fr/blog/2019-03-creating-seamless-portals-in-unreal-engine-4/) which has been heaviliy modified.

Altough the plugin offer a teleporter portal, the main focus has been on rendering effects, so the majority of the features are only aesthetics.

## General idea
Here you can see a basic application with a TeleporterPortal :

![ezgif-4-b61e5af2590c](https://user-images.githubusercontent.com/18093026/131849252-89a1910c-9216-426e-b3bc-7504c96efe6f.gif)

I created the exact same scene twice with different textures for each element and both portals are linked, giving this cool effect of "parallel worlds".

## Advanced features
This plugin offers 3 types of portals:
* **Teleporter portal** : Portal that can be linked to another (B) and which teleport the player to B when crossed.  
  It's possible to change the exit way (in front of or behind B).
  
* **Simple portal** : Actor that can either be a portal (thus identical to Teleporter except doesn't manage teleportation), either a hole or a mirror.  
  It is, as its name suggests, a simple object which will be enough in most cases.
  
* **Custom portal** : Actor which is useless alone. To use it, it requires PortalSceneCaptures components attached to it (up to 5).
  Each PortalSceneCapture can be customized in the exact same way a SimplePortal can be, added to the ability to set the refraction index of mediums in front of and "behind" the portal, and a weight parameter.  
  For instance, if a PortalSceneCapture hole has a weight of 7 and a PSC mirror a weight of 1, the player will see through the CustomPortal with a subtle reflection (7 times les visible), giving a glazed impression.
  
A lot of work has been made to support recursion (portals inside portals) even in the trickiest cases, and to improve performance by making sure only visible portals are rendered. 

## Demo of refraction
Here you can see a CustomPortal with PortalSceneCapture of type hole with a refractive index of 1 in front of the portal and 2 behind.

![ezgif-4-e3bd6109eff2](https://user-images.githubusercontent.com/18093026/131854202-aec29dd9-80bb-4cc0-95ea-4280d1d0d664.gif)
