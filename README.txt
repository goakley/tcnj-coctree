NBODY SIMULATION
================

Uses the Barnes-Hut tree algorithm to calculate the force of N bodies on 
each other.


=== Calculations ===

This program is meant to be a demonstration of the efficiency of the Barnes-Hut 
algorithm, which runs in O(n log n) time (as opposed to a simple iteration over 
every element twice, which would be O(n**2)).  By treating clumps of far-away 
bodies as one large mass, the number of calculations requried to determine 
force is greatly reduced.

These is a tradeoff on space, however, as an octree must created in addition to 
the bodies already in memory.  It is difficult to determine a general size of 
the octree; in the best case, each node will contain within itself a body 
(space complexity 'n'), while in the worst case, two bodies could be 
indeterminantly close together, resulting in an infinitely tall octree.  Clumps 
of bodies are thus detrimental to the space (and time) complexity of the 
system, as they will occupy a lower position on the graph than points that are 
few and far between.


=== Requirements ===

The program requires GLUT to run.  It can be freely obtained for Windows, Mac, 
and GNU/Linux systems.


=== Controls ===

Up arrow - zoom in
Down arrow - zoom out
Right arrow - rotate righter
Left arrow - rotate lefter
Space - toggle initialization box visual
+/= - Increase size of bodies
-/_ - Decrease size of bodies
