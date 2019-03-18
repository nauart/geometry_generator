# geometry_generator

Description
-----------
Generates random geometry data to be used for intersection logic testing in october project. The result of this generator work is a set of AABBs (axis-aligned bounding boxes) represented by their coordinates along with some additional geometry data provided for each AABB: coordinates of all childs, diagonal, coordinates of random rays which intersect / not intersect AABB, distance to intersection and so on (please look at detailed structure below). All coordinates / values except of number of intersected child are presented as floating point values (number of intersected child is an integer).

Accuracy
--------
Precision of floating point values in output is only 6 digits after decimal point, so they should not be taken as absolutely accurate. It means, for example, that ray which is declared as intersecting AABB may not intersect it depending on precision you use for calculations. That's why in october project some generated values are compared only up to 2 digits after point during testing.

Output data description
-----------------------
Generated geometry data has following structure:
```
{
  // Root AABB's min and max points coordinates
  {{-5.638346, -9.215011, -5.073782}, {8.977625, 9.218960, 8.009191}},

  // 8 AABB childs (also by min and max points)
  {
    {{-5.638346, -9.215011, -5.073782}, {1.669640, 0.001974, 1.467704}},
    {{1.669640, -9.215011, -5.073782}, {8.977625, 0.001974, 1.467704}},
    {{-5.638346, 0.001975, -5.073782}, {1.669640, 9.218960, 1.467704}},
    {{1.669640, 0.001975, -5.073782}, {8.977625, 9.218960, 1.467704}},
    {{-5.638346, -9.215011, 1.467704}, {1.669640, 0.001974, 8.009191}},
    {{1.669640, -9.215011, 1.467704}, {8.977625, 0.001974, 8.009191}},
    {{-5.638346, 0.001975, 1.467704}, {1.669640, 9.218960, 8.009191}},
    {{1.669640, 0.001975, 1.467704}, {8.977625, 9.218960, 8.009191}},
  },

  // Diagonal of the root AABB
  26.918434,

  // Random ray which does not intersect root AABB
  {{-91.501472, 9.049576, -15.503174}, {0.223136, 0.966805, -0.124491}},

  // Random ray which intersects root AABB
  {{-0.521263, 8.250023, 14.961449}, {0.455687, -0.596945, -0.660307}},

  // Ray reflected after intersection with ray above
  {{4.276586, 1.964897, 8.009192}, {0.455687, -0.596945, 0.660307}},

  // Number of intersected child
  4,

  // Distance from intersection ray start point to intersection point
  10.528820
},
{
... next AABB data
}
```
