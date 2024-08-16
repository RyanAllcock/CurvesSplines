# Graphical Curves and Splines

## This Project
Generate interpolated graphical splines using control points.

## Usage
- Place Control Point: left mouse click.
- Remove Last Control Point: right mouse click.
- Toggle Curve Or Spline: S key.
- Toggle Piece Continuity: C key, only affects splines.

## Compilation
- External Libraries: uses SDL2 & OpenGL 3.3 & GLM
- Requirements: MinGW C++ compiler "g++" added to system environment variables
- Local Directory Contents: shaders &amp lib &amp util &amp Makefile &amp main source file
- Set Up Directory: "make prepare"
- Compile: "make" produces "curves.exe" 

## Relevant Terminology &amp Properties
- Formula: describes how the curve is generated.
- Control points: direct the curve's progression, together forming a control polytope.
- Knots: direct a spline's progression, giving it's continuity across pieces implicitly through an extended knot vector.
- Continuity: preserves smoothness properties between spline pieces, given by how many derivative orders of the pieces' polynomials are equal.
- Smoothness: ensures spacial properties, like position (C<sup>0</sup>) and tangent (C<sup>1</sup>), change with consistency across the curve.

## Curves

### General Definition
Parametric curves are used to describe smooth functions across space. They can be displayed graphically by sampling points along their formula.

### Types &amp Properties

#### Bezier (https://en.wikipedia.org/wiki/B%C3%A9zier_curve)
- Represents the curve using Bernstein basis polynomials, which can be processed into a summation.
- Endpoint Interpolation: Curves begin on the initial control point, and end on the final control point.
- Variation Diminishing Property: The curve intersects a given plane as many - or fewer - times as the lines connecting the control points do.
- The convex hull of control points encloses the resulting curve.

## Splines (https://en.wikipedia.org/wiki/Spline_(mathematics))

### General Definition
Piecewise polynomials are used to describe an overall curve function. Their flexibility in definition, which leads to finer localised control of the curve, allows them to define complex curves generally easier than singular curve representations, and for varying granular requirements. Each piece can also be isolated and constructed separately, making interpolation easier to manage and parallelise.

## Features
- 2D Space
- Placing control points
- Bezier curves
- Cubic Bezier splines (continuity C<sup>0</sup>, or C<sup>1</sup> using point-defined vectors)

## Future Work

### Documentation
- Types of splines, e.g. https://en.wikipedia.org/wiki/Composite_B%C3%A9zier_curve, https://en.wikipedia.org/wiki/B-spline
- Types of surface, e.g. https://en.wikipedia.org/wiki/B%C3%A9zier_surface

<!-- ### Logical &amp Debugging -->

### Completion
Overall: allow the display of any continuous curves/surfaces representable as data inside wavefront files
- 3D curves
- bezier surfaces
- more fundamental curve/spline types; e.g. Hermite, NURBS (delay ones not used in .obj files)
- read &amp write data from .obj files (wavefront file structure)

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
- data management &amp storage structure, for conducting atomic operations (e.g. add/remove points and curves whilst updating gpu data) in an enclosed/elegant fashion
- smart shading template structures for point-/line-/vector- -renderers/-rendering, to reduce boilerplate renderer &amp gpu data code

### Optimise
- eventually only compute the changed parts of data during input
- pre-compute vectors for vector shader... worth having extra buffer for vector data?
- limit update ranges for spline data by splitting curve vector into spline vector of curve vectors
	- store data indices &amp update signals in separate vectors; use these to flag which curves to update data &amp buffers for after each input
	- interchange curve &amp spline indices; curve indices are on same buffer, spline indices use 

### Optional
- convert window position &amp motion to [-/+ xAspectRatio, -/+ yAspectRatio]; maybe automatically compute this change inside window by holding "viewport" as a member variable?