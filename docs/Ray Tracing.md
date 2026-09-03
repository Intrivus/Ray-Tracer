# Ray Tracing in One Weekend

## Output

The initial chosen display method is the **PPM** image format.

**Example: `image.ppm`**

``` dart
P3
2 2
255
196  0 0    0 154 0
0   68 1    1 255 0 
```

- `P3` : Specify the color format to be ASCII
- `2 2`: Set the canvas size
- `255`: Set maximum color value to be 8-bit for each channel (R, G, B)
- `196 0 0`: One color pair of RGB for each pixel, divided by whitespace

> **NOTE**: Each pair is separated by any kind of whitespace and displayed in ***Row-Major Order***. Hence, pixels fill out the canvas' rows one by one, regardless of the whitespace chosen.

Image has a fixed `aspect_ratio` that will be used to calculate its height/width relative to each other.

## 3D Vector

A simple `vec3` class is created to serve the purpose of handling the canvas space and colors. It supports basic operations, and a few linear ones: dot / cross product, and normalization.

Using `vec3` to create `point3` and `color` alias, with `color`'s fields of range \[0, 1\] from which it is to 8-bit range \[0, 255\] in `color` class.

## World

### Camera

The point where all scene **rays** originate with the vector in the center being orthogonal to the viewport of a *`focal length`* distance.

**Snippet: `camera.h`**

``` cpp
    auto focal_length = 1.0;
    center = point3(0, 0, 0);
```

### Viewport

A space moving with relative to the camera position where **rays** are shot through, mapping out the image pixel grid into a real floating rectangle in the world space.

**Snippet: `camera.h`**

``` cpp
    auto viewport_height = 2.0;
    auto viewport_width = viewport_height * (double(image_width)/image_height);
```

> **NOTE**: Calculated relative to image's width and height for more accuracy compared to using aspect ratio variable.

Two new vectors are created to describe the viewport rectangle's full width and height and used to map the pixel distance into distance between 3D viewport points, where rays can be shot through to capture an image of the world.

**Snippet: `camera.h`**

``` cpp
    vec3 viewport_u = vec3(viewport_width, 0, 0);
    vec3 viewport_v = vec3(0, -viewport_height, 0);


    vec3 pixel_delta_u = viewport_u / image_width;
    vec3 pixel_delta_v = viewport_v / image_height;
```

### Rays

A basic class with an `origin` from which a ray stems out into a `direction`. Included is a method to return a `point3` somewhere at that ray -\> `P(t) = origin + t * direction`.

### Objects

Declared two classes under `hittable.h` to manages objects (that can be hit by the rays):

- `hit_record`: to store information about a "hit", when a ray intersect with the object's surface: point of intersection, the surface normal, the scalar value of the ray's direction (`t`), and whether the surface is the front or back.

> **NOTE**: The face information will be used to determine the normal vector direction, flipping it if the ray is hitting the back of the surface.

- `hittable`: includes a method `hit()` to check whether a ray hit the surface of the object, and updates a `hit_record` accordingly. The ray intersection only render inside a boundary \[ray_tmin, ray_tmax\], the two parameters of the `hit()` method.

> **NOTE**: such boundary will be encapsulated inside an `interval` class in which we will store multiple boundaries moving onward, namely, the render limit, the ray scaling limit (to only render the closest object), or a static world limit...

**Convention**: - All normal vectors will be of unit length. - Each point hit will result in an outward surface normal vector, then $dot$ the ray's direction and the outward normal vector to decide the normal final direction.

#### Sphere

A sphere, inherited from hittable, can be created by checking whether there is any intersection between the ray and itself by checking whether a point in the ray satisfy its equation.

**We have**: $(C_x - x)^2 + (C_y - y)^2 + (C_z - z)^2 = r^2$ , a sphere with an arbitrary center. The vector from $\mathbf{P} = (x,y,z)$ to that center $\mathbf{C} = (C_x,C_y,C_z)$ is $(\mathbf{C}-\mathbf{P})$. The length of $(\mathbf{C}-\mathbf{P})$ is $r$, therefore by dot product definition: $(\mathbf{C}-\mathbf{P}) \cdot (\mathbf{C}-\mathbf{P}) = r^2$ . Point `P` is one that satisfies the equation, and we want to check whether any points in our ray satisfy this equation, so we insert `P(t)`: $$(\mathbf{C}-\mathbf{P}(t)) \cdot (\mathbf{C}-\mathbf{P}(t)) = r^2$$ Expand $P(t)$, we have: 

$$(\mathbf{C}-(\mathbf{Q}+t\mathbf{d})) \cdot (\mathbf{C}-(\mathbf{Q}+t\mathbf{d})) = r^2
$$

$$
t^2\mathbf{d} \cdot \mathbf{d} - 2t\mathbf{d} \cdot (\mathbf{C}-\mathbf{Q}) + (\mathbf{C}-\mathbf{Q}) \cdot (\mathbf{C}-\mathbf{Q}) - r^2 = 0
$$

Forming a **quadratic equation** $ax^2 + bx + c = 0$: 

$$
\begin{gather}
a = \mathbf{d} \cdot \mathbf{d}\\
b = -2 \mathbf{d} \cdot (\mathbf{C} - \mathbf{Q})\\
c = (\mathbf{C} - \mathbf{Q}) \cdot (\mathbf{C} - \mathbf{Q}) - r^2
\end{gather}
$$

Notice that $b$ has the form: $b = -2h$. 
$$
\frac{-b \pm \sqrt{b^2+4ac}}{2a} \implies \frac{h \pm \sqrt{h^2+ac}}{a}
$$ 

$$
\Delta = h^2+ac
$$ 

From here we get the number of roots, getting $t$, to check whether a ray hits, and if these roots are bounded in \[ray_tmin, ray_tmax\].

The outward normal of the sphere surface at that point can be calculated with: $(\mathbf{P} - \mathbf{C}) / r$ , avoiding square root length calculation entirely.

### Object List

The `hittable_list` class inheriting `hittable` to store all objects, and check if the rays hit any of its object while deciding the object's visibility by limiting the interval to the prior closest one for each object checked.

## Anti-aliasing

Each pixel displayed should be the result of averaging out a large sample of colors acquired with multiple rays shooting around the 3D point inside its specified pixel in the viewport to smooth out that pixel.

For each ray shot, there will be a random offset \[-0.5, -0.5\]-\[+0.5, +0.5\] around its destination point to obtain a range of color inside that pixel of the image. There, we get the average of the colors from these samples to produce a blending and continuous pixel.

## Material

A class containing a method that decides whether a ray hits scatter and processes the scattered ray reference accordingly, outputting the attenuation for each hit, return false with no processing by default.

### Lambertian : Material

A material that scatters lights uniformly in all direction. The scatter direction is calculated with the surface normal ($\mathbf{n}$) at the point hit ($\mathbf{P}$), and a random point ($\mathbf{S}$) at the unit sphere whose center is at $(\mathbf{P}+\mathbf{n})$. From here, the ray's direction is the vector $(\mathbf{S} - \mathbf{P})$.

![Randomly generating a vector according to Lambertian distribution.](Images/Figure_1.png){#fig-1}

Basically: `scatter_direction = rec.normal + random_unit_vector()` with `rec` basically being a `hit_record`, the information of a ray's hit.

This approach will create a random scattered ray that biases towards the normal vector's direction.

### Metal : Material

A material that scatters lights rays out of the surface following the reflection function: 

$$\mathbf{v} - 2 * \mathbf{v} \cdot \mathbf{n} * \mathbf{n}$$

With $\mathbf{v}$ and $\mathbf{n}$ being the ray and surface normal vector respectively, this will output the reflected ray direction to continue scatterings.

#### Fuzzy Reflection

Fuzziness works by generating a new random direction for a normalized reflected ray. 

```cpp
vec3 reflected = reflect(r_in.direction(), rec.normal);
reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
```

This vector addition is two force an outward bias to the generated vector, and the `fuzz` double decides the strength of the deviation from the original direction.

