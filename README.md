# Graphical Curves and Splines

## This Project
Generate interpolated graphical splines using points.

## Usage
- Place point: left mouse click.
- Remove last point: right mouse click.
- Toggle between Bezier Curve, Cubic Bezier Spline, and Cubic Bezier Spline with C<sup>1<s/sup> smoothness constraint: S key.
- Toggle between constant, spatial, and curvature sampler constraints: S key.

## Features
- Placing 2D points
- Bezier curves
- Cubic Bezier splines (C<sup>0</sup> and C<sup>1</sup>)
- Samplers (constant, spatial, curvature constrained)

## Compilation
- External Libraries: uses SDL2 & OpenGL 3.3 & GLM
- Requirements: MinGW C++ compiler "g++" added to system environment variables
- Local directory contents: deploy & lib & util & Makefile & main source file
- Set up directory: console command "make prepare"
- Compile: console command "make" produces "deploy//curves.exe"

## Relevant Terminology & Properties
- Formula: describes how the curve is generated.
- Control points: direct a bezier curve's progression, together forming a control polytope.
- Knots: direct a spline's progression, giving it's continuity across pieces implicitly through an extended knot vector (continuity-constrained terms are repeated).
- Continuity \& Smoothness: preserves properties between spline pieces, given by the polynomial order of smoothness across a piecewise change along the curve, which gives how many derivative orders of adjacent pieces' polynomials are equal, represented using a smoothness vector. Constraining spacial properties, including piece-transitional position (C<sup>0</sup>), velocity (C<sup>1</sup>), tangent (C<sup>2</sup>) and further continuity, minimises curve sharpness at the cost of determinism (flexibility).

## Curves

### General Definition
Parametric curves are used to describe smooth functions across space. They can be displayed graphically by sampling points along their formula.

### Types & Properties

#### Bezier (https://en.wikipedia.org/wiki/B%C3%A9zier_curve)
- Represents the curve using Bernstein basis polynomials, which can be processed into a summation.
- Endpoint Interpolation: Curves begin on the initial control point, and end on the final control point.
- Variation Diminishing Property: The curve intersects a given plane as many - or fewer - times as the lines connecting the control points do.
- The convex hull of control points encloses the resulting curve.

## Splines (https://en.wikipedia.org/wiki/Spline_(mathematics))

### General Definition
Piecewise polynomials are used to describe an overall curve function. Their flexibility in definition, which leads to finer localised control of the curve, allows them to define complex curves generally easier than singular curve representations, and for varying granular requirements. Each piece can also be isolated and constructed separately, making interpolation easier to manage and parallelise.

## Samplers
Samplers are used to give control over how samples are extracted from an analytical curve, to be converted into renderable data points. Harshly-winding curves may appear sharper for basic samplers than for complex samplers, but complex samplers may be less efficient for computations, take more space in memory, or overly-biased towards certain curvatures.

### Types & Properties
- Constant: sample points at fixed intervals along the curve domain.
- Spatial: sample points at constrained distances apart from each other.
- Curvature: sample points by constraining the angles between each set of three adjacent points, as well as the distance from sampled points to the line drawn between their neighbours (the discrepancy in positional correctness, were that point not have been sampled, is thus limited).

## Future Work
Branch into 3D space to prioritise representing and rasterising free-form surfaces, constructable from data inside .obj files of the wavefront file format.

### Documentation
- Types of splines, e.g. https://en.wikipedia.org/wiki/Composite_B%C3%A9zier_curve, https://en.wikipedia.org/wiki/B-spline
- Types of surface, e.g. https://en.wikipedia.org/wiki/B%C3%A9zier_surface

<!-- ### Logical & Debugging -->

### Completion
Overall: allow the display of any continuous curves/surfaces representable as data inside wavefront files
- 3D curves
- bezier surfaces
- more fundamental curve/spline types; e.g. Hermite, NURBS (delay ones not used in .obj files)
- read & write data from .obj files (wavefront file structure)

### Feature
- higher continuity options for cubic+ curves... highest-possible continuity for a specific and/or evenly-sized curve size? incremental continuities up to "Cn", n = curve points(?)?
- solve optimal sample point positions along curve, to minimise angle (error) across each pair of adjacent sample lines, to a limit of sample total or maximum error (can set either)
- enable geometric pattern generation, e.g. https://en.wikipedia.org/wiki/String_art

### Quality Of Life
- allow altering of piecePoints variable using input for bezierCurves2
- harsh bezierCurves2 angles still pinched; smoothly represent, possibly by separating each spline piece in graphics? use EBO and geometry shaders to store vertex data more compact?
- billboard shading; always point towards camera
- relative displacement straight line rendering for vector shader
- remove whiteflash on first window frame; prevented on WIN32 by suppressing WM_ERASEBKGND message or setting hdr_background to NULL; how to suppress for SDL?
	- https://stackoverflow.com/questions/10335488/white-flash-when-creating-an-opengl-render-context-on-windows
	- https://stackoverflow.com/questions/12073721/what-is-the-correct-way-to-suppress-wm-erasebkgnd
- data management & storage structure, for conducting atomic operations (e.g. add/remove points and curves whilst updating gpu data) in an enclosed/elegant fashion
- smart shading template structures for point-/line-/vector- -renderers/-rendering, to reduce boilerplate renderer & gpu data code

### Optimise
- eventually only compute the changed parts of data during input
- pre-compute vectors for vector shader... worth having extra buffer for vector data?
- limit update ranges for spline data by splitting curve vector into spline vector of curve vectors
	- store data indices & update signals in separate vectors; use these to flag which curves to update data & buffers for after each input
	- interchange curve & spline indices; curve indices are on same buffer, spline indices use 

### Optional
- convert window position & motion to [-/+ xAspectRatio, -/+ yAspectRatio]; maybe automatically compute this change inside window by holding "viewport" as a member variable?