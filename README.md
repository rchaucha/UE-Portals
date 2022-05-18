
# Plugin : Unreal Engine 4 Portals

---

## Thanks
Thanks to Froyok for giving me the rights to use his work as a starting base : https://www.froyok.fr/blog/2019-03-creating-seamless-portals-in-unreal-engine-4/.

## Context
This plugin has been developed for the study of visual properties only. Thus, although the teleportation aspect has been implemented, it is really secondary and not much work has been put into it.

## Installation

1. Create a "Plugins" folder inside your project folder

2. Copy Portals inside

3. In the projects settings in UE, under "Engine / Rendering / Lighting", check "Support global clip plane for planar reflections"

5. Restart UE

6. Under "Project / Maps & Modes", change "Default Gamemode" in "GameModeImpl"

To start with, a demo map can be found inside Maps and contains a demonstration of all possible portals.

---

## Idea

![](https://lh6.googleusercontent.com/_aH_SL8vD_mVbz1OW_nbgwlGbvXstrVVwre8bwe487IVP2clBwe89-jTdSx52zSsf3LTKrkcsYjH3YXDBcciq4X_yEyRQUfDLQKSgOj2CbD2Mje9jQOdxNG9n5Lpc1JOYq5HNifaMrQ=s0)
When the player (eye) looks at the portal $P_1$ linked to $P_2$, he will see as if $P_1$ was a window to the world located behind $P_2$.  

---

## Features

#### Types of portals

* <u>Teleporter portal</u> : portal that can be linked to another portal (B) and which can be crossed so that the player is teleported to B location. \
You can change the exit direction (front/back)

* <u>Simple portal</u> : object that can either be a portal (and thus identical to Teleporter without teleportation), either a hole (we see through it as if it wasn't there), or a mirror.
  As its name suggests, it's an easy to use object which will be enough in most cases.

* <u>Custom portal</u> : object which doesn't do anything alone. To use it, you can add up to 5 "SceneCapture" component to it.\
Each SceneCapture (SC) can be configured in the same way as a Simple portal, with in addition the possibility to define the refractive indices of the media before and after the portal and a "Weight" parameter that defines the weight (positive real number) that the SC will have in the Custom portal.
  For example, if you have a SC hole with a weight of 7 and a SC mirror with a weight of 1, the rendering will be similar to the one you can have by looking through a window, with a slight reflection (here 7 times less visible than the hole).  

#### Changing SceneCaptures behaviours

It is possible to redefine the behavior of SceneCaptures associated with a Custom portal by redefining the methods "UpdateTransformation" and "UpdateNearClipPlane" which respectively define how the position of the SceneCapture is calculated according to the position of the WatchedActor (see "Internal operation") and how the near plane clip is calculated. 

These two methods can be redefined in Blueprint.

---

## Internal operation

#### General example

![](https://lh3.googleusercontent.com/pJa0kkgFhCT8KEcpIRHkpSTxI85-1e9IiUN9ybs5aQVVcrllfXukNhfUDtr5SusJ535HVtMUFJPhGsNU4Sj0m4-rHZ15n8GSX-NjsPdiFS_v3sorqMdJtp9siCGDKjkVExiKdt56OEM=s0)
Each visible portal (here, only $P_1$ is visible because $P_2$ is only an intermediary and will not be seen by the player) has:

* At least one SceneCapture (here $SC_1$) that belongs to the portal 

* Only one reference on a WatchedActor (here $WA_1$) which is therefore only observed by the portal, and in no case its property.
  It will be either the player or the SceneCapture from another portal.

In this way, the visible portals can update the position and rotation of their SCs according to those of their WA. 

#### Detailed example

![](https://lh5.googleusercontent.com/N17Fsm5j1VN0FgSsvqRHKIbfh3xeox6YIwE6Pz1BJvJ8-Dqma16SRPysvSxLb02gLNMvCkJWJhOGMjvd-8y2ZtpmyCIocmluruuGh1FwXkDfqAV1mGaAxImRsHxBS8WeDRPN1yaNpDM=s0)
Here we have a slightly more complex scene where $P_1$ is linked to $P_2$ and $P_3$ is linked to $P_4$. What the player sees in the end is shown at the bottom left of the image.

Here, two portals are visible: $P_1$ and $P_3$. These two portals will each have their SC and WA.

It is interesting to note that the $P_3$ WA ($WA_3$) is not the player, as the player does not see $P_3$ directly, but through $P_1$. $WA_3$ will in fact be the $SC_1$ because it is from this point of view that $P_3$ will be seen.

---

## Possible evolution

#### Problems to solve

* The major problem is the near clipping plane: when you want to take some freedom with the Scene Capture (for example use the Static Scene Capture for the invisible cube or refraction), a piece of the ground and any elements present before the near clipping plane disappear.

* Another problem is that of total refraction: since the refraction is approximated in the center of the portal, aberrations are visible at the edges, and in particular a graphical glitch can be observed just before the transition to total refraction (mirror) on the edge of the portal. This can be solved by mirroring the switch earlier, but it would imply a flawed calculation even for the center.
  It will therefore be necessary to make a choice between the accuracy of the calculations in the center (particularly important in the case of the subdivision implementation) and the disappearance of the visual glitch.

#### Ideas

* As mentioned just before, the refraction can benefit from the implementation of the subdivision of the portals so that the approximation of the refraction is better. However, visible cuts between the different portals are to be expected, which is generally not desirable.

* Another possibility to dig is that of ray tracing to manage as finely as possible the way we distort the image.

* Consider other changes to the portal properties (e.g., time latency)

* Currently, the size difference of the portals does not have any impact, but it might be interesting to modify the resulting image (enlarge/shrink) according to

* To go further, we could imagine non planar portals (spheres, cylinder,...) and see how this would impact the rendering 

* It may be interesting to try to reproduce current results using less than ideal cameras, in particular:
  
  * A wide-angle camera from which the desired image must be extracted and rendered on the portal according to the player’s position.
  
  * A set of still cameras (for example, 4 cameras located at the angles) whose images should be interpolated in order to obtain the desired one.

* Explore post-production possibilities that can be applied to the SceneCapture image before it is displayed on the portal:
  
  * Augmented reality (analysis of the obtained image, addition of element from above before displaying it,...)
  
  * Special Effects

