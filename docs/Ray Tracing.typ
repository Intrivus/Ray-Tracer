#import "style.typ": *

#show: document.with(
  title: [Dynamic Ray Tracing],
  authors: "Intrivus Nguyen",
  date: datetime(year: 2026, month: 8, day: 21)
)

= Ray Tracing in One Weekend

Below are the summary of Peter Shirley's book Ray Tracing in One Weekend @raytracing1weekend written from my own understanding. Admittedly, this section may not comprehensively address every category from the book in detail and mostly consists of the information or explanation I thought need clarification from the authors.

== Output

The initial chosen display method is the *PPM* image format.

*Example: `image.ppm`*

```dart
P3
2 2
255
196  0 0    0 154 0
0   68 1    1 255 0 
```

- `P3` : Specify the color format to be ASCII.
- `2 2`: Set the canvas size.
- `255`: Set maximum color value to be 8-bit for each channel (R, G, B).
- `196 0 0`: One color pair of RGB for each pixel, divided by whitespace.

*NOTE*: Each pair is separated by any kind of whitespace and displayed in _*Row-Major Order*_. Hence, pixels fill out the canvas' rows one by one, regardless of the whitespace chosen.

Image has a fixed `aspect_ratio` that will be used to calculate its height/width relative to each other.

== 3D Vector

A simple `vec3` class is created to handle the world space and colors. It supports basic operations, and a few uniquely linear ones: dot / cross product, and vector normalization.

Using `vec3` to create `point3` and `color` alias, with `color`'s fields ranging [0, 1] which is then converted to 8-bit range [0, 255] before writing into the image.

*NOTE*: This choice in color range is to make accumulation, blending, and lighting calculations (like attenuation) intuitive later on.

*Rays:* A basic class with an `origin` from which a ray stems out into a `direction`. Included is a method to return a `point3` somewhere at that ray: `P(t) = origin + t * direction`.

== World

*Camera:*

The point where all scene *rays* originate with the vector in the center and shoot out to the viewport at a *`focal length`* distance. Both the `focal length` and camera angle is dictated by the initial value of the point `lookfrom` and `lookat`, from which the orthonormal (*ortho*\gonal & *normal*\ized) basis of the viewport is structured.  

The direction (or angle) of the camera can then be simply calculated as: `lookat - lookfrom`, to which the viewport is perpendicular at its center (see @camera-basis).

*NOTE*: This program uses right-hand coordinate, hence, the facing of the positive z-axis towards us.

*Viewport:*

A plane, whose surface normal is collinear with the camera center (`lookfrom`), moving relative to the camera position and angle. This is where *rays* are shot through, mapping out the image pixel grid into a real floating rectangle in the world space.

Its height is dependent on the vertical field of view (fov); this is the visual angle from the top and bottom edge of the viewport.

#figure(
  image("Images/Vertical FOV.svg", width: 100%),
  caption: [Deriving viewport height from vertical FOV $theta$ (2D & 3D representation).]
)

It can be seen: $tan theta/2 = h/("focal_length")$ , therefore:

*Snippet: `camera.h`*

```cpp
double vfov = 90.0;
...

auto focal_length = (lookfrom - lookat).length();
auto theta = degrees_to_radians(vfov);
auto h = std::tan(theta/2);
auto viewport_height = 2 * h * focal_length;
auto viewport_width = viewport_height * (double(image_width)/image_height);
```


From the direction that the camera angle specifies above, it is possible to devise the vector basis $(u, v, w)$ of the camera's coordinate system, provided an upward direction vector $italic("vup")$ has been specified to determine how vertical uprightness _looks like_ 

(imagine if $italic("vup")$ is tilted, the viewport will similarly rotate). 

#figure(
  image("Images/Camera Basis.svg", width: 100%),
  caption: [Vector basis of the viewport without normalization with $italic("vup")(0,1,0).$]
) <camera-basis>

*NOTE*: Angles and vector lengths are illustrative, not geometrically precise.

*Snippet: `camera.h`*
```cpp
  w = unit_vector(lookfrom - lookat);
  u = unit_vector(cross(vup, w));
  v = cross(w, u);
```

Next, two vectors are created to describe the viewport rectangle's full width and height and used to map the pixel gap into distance between 3D viewport points where rays can be shot through to capture an image of the world.

*Snippet: `camera.h`*

```cpp
// Vector across viewport horizontal edge
vec3 viewport_u = viewport_width * u;
// Vector down viewport vertical edge
vec3 viewport_v = viewport_height * -v;

vec3 pixel_delta_u = viewport_u / image_width;
vec3 pixel_delta_v = viewport_v / image_height;
```

== Objects

Declared two classes under `hittable.h` to manages objects (that can be hit by the rays):

- `hit_record`: to store information about a "hit", when a ray intersect with the object's surface: point of intersection, the surface normal, the scalar value of the ray's direction (`t`), and whether the surface is the front or back.

*NOTE*: The face information will be used to determine the normal vector direction, flipping it if the ray is hitting the back of the surface.

- `hittable`: includes a method `hit()` to check whether a ray hit the surface of the object, and updates a `hit_record` accordingly. The ray intersection only render inside a boundary [ray_tmin, ray_tmax], the two parameters of the `hit()` method.

*NOTE*: such boundary will be encapsulated inside an `interval` class in which multiple boundaries are stored moving onward, namely, the render limit, the ray scaling limit (to only render the closest object), or a static world limit...

*Convention*: 
- All normal vectors will be of unit length. 
- Each point hit will result in an outward surface normal vector, then _dot_ the ray's direction and the outward normal vector to decide the normal final direction.

*Sphere:*

A sphere, inherited from hittable, can be created by checking whether there is any intersection between the ray and itself by checking whether a point in the ray satisfy its equation.

*We have*: $(C_x - x)^2 + (C_y - y)^2 + (C_z - z)^2 = r^2$ , a sphere with an arbitrary center. The vector from $pt(P) = (x,y,z)$ to that center $pt(C) = (C_x,C_y,C_z)$ is $(pt(C)-pt(P))$.

The length of $(pt(C)-pt(P))$ is $r$, therefore by dot product definition: $(pt(C)-pt(P)) dot (pt(C)-pt(P)) = r^2$ . 

Point `P` is one that satisfies the equation, and the program needs to check whether any points from a ray satisfy this equation. Insert `P(t)` (from `ray` class): 

$ (pt(C) - op("P")(t)) dot (pt(C) - op("P")(t)) = r^2 $ 

Expand $P(t)$, we have: 

$
(pt(C) - (pt(Q) + t bold(d))) dot (pt(C) - (pt(Q) + t bold(d))) = r^2
$

$
t^2bold(d) dot bold(d) - 2t bold(d) dot (pt(C) - pt(Q)) + (pt(C) - pt(Q)) dot (pt(C) - pt(Q)) - r^2 = 0
$

Forming a *quadratic equation* $a x^2 + b x + c = 0$:

$
a = bold(d) dot bold(d) \
b = -2 bold(d) dot (pt(C) - pt(Q)) \
c = (pt(C) - pt(Q)) dot (pt(C) - pt(Q)) - r^2
$

Notice that $b$ has the form: $b = -2h$.
$
frac(-b plus.minus sqrt(b^2 + 4a c), 2a) => frac(h plus.minus sqrt(h^2 + a c), a)
$

$
Delta = h^2 + a c
$

From here it returns the number of roots, getting $t$, to check whether a ray hits and if these roots are bounded in [ray_tmin, ray_tmax].

The outward normal of the sphere surface at that point can be calculated with: $(pt(P) - pt(C)) / r$ , avoiding square root length calculation entirely.

*Object List:*

The `hittable_list` class inheriting `hittable` to store all objects, and check if the rays hit any of its object while deciding the object's visibility by limiting the interval to the prior closest one for each object checked.

== Anti-aliasing

Each pixel displayed should be the result of averaging out a large sample of colors acquired with multiple rays shooting around the 3D point inside its specified pixel in the viewport to smooth out that pixel.

For each ray shot, there will be a random offset [-0.5, -0.5]-[+0.5, +0.5] around its destination point to obtain a range of color inside that pixel of the image. There, the average of the colors from these samples returns a blending and continuous pixel.

== Material

A class containing a method that decides whether a ray hits scatter and processes the scattered ray reference accordingly, outputting the attenuation for each hit, return false with no processing by default.

*Lambertian : Material*

A material that scatters lights uniformly in all direction. The scatter direction is calculated with the surface normal ($bold(n)$) at the point hit ($pt(P)$), and a random point ($pt(S)$) at the unit sphere whose center is at $(pt(P)+bold(n))$. From here, the ray's direction is the vector $(pt(S) - pt(P))$.

#figure(
  image("Images/Lambertian Random.png", width: 100%),
  caption: [Randomly generating a vector according to Lambertian distribution.]
)

Basically: `scatter_direction = rec.normal + random_unit_vector()` with `rec` basically being a `hit_record`, the information of a ray's hit.

This approach will create a random scattered ray that biases towards the normal vector's direction.

*Metal : Material*

A material that scatters lights rays out of the surface following the reflection function: 

$ bold(v) - 2 * bold(v) dot bold(n) * bold(n) $

With $bold(v)$ and $bold(n)$ being the ray and surface normal vector respectively, this will output the reflected ray direction to continue scatterings.

*Fuzzy Reflection:*

Fuzziness works by generating a new random direction for a normalized reflected ray. 

```cpp
vec3 reflected = reflect(r_in.direction(), rec.normal);
reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
```

This vector addition is to force an outward bias onto the generated vector, and the double `fuzz` decides the strength of the deviation from the original direction.

*Dielectric : Material*

A material that refract or reflect rays out of the surface with regard to viewing angle. Light rays refraction direction is governed by Snell's Law:

$
  eta dot sin theta = eta' dot sin theta'
$

- $eta, eta'$: the refractive indices of the inner, the object's, and the outer material, respectively.
- $theta$: the angle of incidence relative to the surface normal $bold(n)$,
- $theta'$: the angle of refraction.

*NOTE*: The full function is implemented per the book's derivation, #link("https://raytracing.github.io/books/RayTracingInOneWeekend.html#dielectrics/snell'slaw")[#underline[see source]].


For each ray hit, the refractive index is decided by whether the hit is registered from the outside or from the inside, which should always be recorded in `hit_record`.

```cpp
double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;
```

#figure(
  image("Images/Refraction Ratio Flip.png", width: 100%),
  caption: [Flip refractive index ratio based on the face hit.]
)

*Schlick's Approximation*: A cheap, but accurate, polynomial approximation for the viewing angle which determine the reflectance.

== Defocus Blur (Depth of Field)

Everything up to this point utilized a single ray origin per pixel, which is optically a pinhole camera, resulting in the world being in perfect focus, unlike a real camera lens. Instead of one ray origin, sample a random point on a small disk, or the lens in the real world counterpart, for each ray's origin. Regardless of which point on the disk a ray starts from, it's always aimed at the same fixed point, the pixel's sample point on the focus plane, located at `focus_distance` from the camera. 

Objects sitting exactly on the focus plane get hit by every sample ray at the same point, generating a sharp image. However, objects nearer or farther get hit at different points by different samples creating a blurry effect when these samples averaged.

#figure(
  image("Images/Lens Spread.svg", width: 100%),
  caption: [Image result for objects hit at each distance.]
)

A wider disk, or a larger defocus angle, spreads possible rays' origins further, which causes more dispersion in where non-focus-plane objects get hit and amplifies the blur.

The disk is constructed from the same camera vector basis $(u, v, w)$ as the viewport, albeit scaled differently. In simple terms, this program leverages the viewport for the focus plane, and replaces `focal_length` with `focus_dist` respectively.

*NOTE*: The implementation of vertical FOV to dynamically adjust the image regardless of the actual `focal_length` makes this replacement possible.

```cpp
vec3 defocus_disk_u;
vec3 defocus_disk_v;
...

auto defocus_radius = 
focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));

defocus_disk_u = u * defocus_radius;
defocus_disk_v = v * defocus_radius;
...

point3 defocus_disk_sample() const
{
    // Returns a random point in the camera defocus disk.
    auto p = random_in_unit_disk();
    return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
}
```

= Beyond "One Weekend"

"Ray Tracing in One Weekend" has shown graphic enthusiasts how the computer can produce realistic images through utter creativity and the application of rudimentary mathematical knowledge in a supposedly short amount of time. Yet, to me, the computer can go beyond a beautiful static render that resembles the nature of that paintings on canvases possess with its capability for immersive and real-time interaction. This is the thought that prompted my diversion from the book's path, and endorse the creative freedom that its authors have laid the foundation to build upon.


== Refactor

The book's original design uses `shared_ptr<hittable>` throughout, allowing multiple owners to reference the same object simultaneously via reference counting. This flexibility is unnecessary, for the scene's object list is the sole structure that ever holds these objects; nothing else in the program needs concurrent access to the same `hittable`. `unique_ptr` enforces this single-ownership relationship directly at the type level, removing the small overhead of reference counting and making the ownership model explicit. A `unique_ptr` in the list is the object's only reference that will exist for the span of the program.

*Snippet: `hittable_list.h`*
```cpp
hittable_list(unique_ptr<hittable> object) { add(move(object)); }

void add(unique_ptr<hittable> object)
{
    objects.push_back(move(object));
}
```

Since copying is disallowed by design with this approach, objects can only be `move()` and transferred to a new owner.

Next, the original single flat directory is separated into folders for specific header files in order to categorize the structure for both tidiness and future extension. From here, I created `app/` directory to hold the third-party SDK and libraries, distinct from any established logic so that only `app/` is aware of the additional display pipeline.
















#bibliography("refs.yaml", style: "ieee")