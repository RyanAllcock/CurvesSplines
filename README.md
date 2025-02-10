# Graphical Curves and Splines

## This Project
Generate interpolated graphical splines using points.

## Usage
- Place point: left mouse click.
- Remove last point: right mouse click.
- Move nearby point: left mouse click and hold with cursor over point, drag to move.
- Toggle between Bezier curve, composite cubic Bezier spline, and C<sup>1<s/sup> spline, natural spline, C<sup>∞<s/sup> spline, and C<sup>1<s/sup> cardinal spline: S key.
- Toggle between constant, spatial, and curvature samplers: C key.

## Features
- Placing 2D points
- Bezier curves
- Basis splines
- Spline Properties: continuity and uniformity, enforced by moving control points (not through using separate methods or calculating derivatives etc.)
- Samplers: constant, spatial, and curvature constrained

## Compilation
- External Libraries: uses SDL2 & OpenGL 3.3 & GLM
- Requirements: MinGW C++ compiler "g++" added to system environment variables
- Local directory contents: deploy & lib & util & Makefile & main source file
- Set up directory: console command "make prepare"
- Compile: console command "make" produces "deploy//curves.exe"

## Relevant Terminology & Properties
- Formula: describes how the curve is generated.
- Control points: direct a bezier curve's progression whilst forming a control polytope, provide control over an individual spline piece, or demonstrate the effect on curve structure of enforcing constraints upon a spline.
- Knots: direct a spline's progression through boundary points at the transitions between pieces, providing continuity across pieces implicitly through an extended knot vector (continuity-constrained terms are repeated).
- Continuity: also known as smoothness. Preserves curvature across spline pieces, represented by a smoothness order for each knot summarised in a smoothness vector, showing how many derivative orders of adjacent pieces' polynomials are equal. Constraining spacial properties, including piece-transitional position (C<sup>0</sup>), velocity (C<sup>1</sup>), acceleration (C<sup>2</sup>) and further continuity, minimises curve sharpness and spontaneity at the cost of control freedom.

## Curves

### General Definition
Parametric curves are used to describe smooth functions across space. They can be displayed graphically by sampling points along their formula.

### Types & Properties

#### Bezier (https://en.wikipedia.org/wiki/B%C3%A9zier_curve)
- Represents the curve using a Bernstein polynomial.
- Endpoint Interpolation: Curves begin on the initial control point, and end on the final control point.
- Variation Diminishing Property: The curve intersects a given plane as many - or fewer - times as the lines connecting the control points do.
- The convex hull of control points encloses the resulting curve.

## Splines (https://en.wikipedia.org/wiki/Spline_(mathematics))

### General Definition
Piecewise polynomials are used to describe an overall curve function. Their flexibility in definition, which leads to finer localised control of the curve, allows them to define complex curves generally easier than singular curve representations, and for varying granular requirements. Each piece can also be isolated and constructed separately, making interpolation easier to manage and parallelise.

### Methods

#### Bezier (https://en.wikipedia.org/wiki/Composite_B%C3%A9zier_curve)
- Calculates each piece's curve using a Bernstein polynomial, forming a composite bezier curve.
- Curve is entirely described by control points, leaving continuity to be enforced through those points' positions instead of derivative constraints.

#### Basis (https://en.wikipedia.org/wiki/B-spline)
- Represents a spline piece using a basis function for each point associated with the piece.
- These basis functions can be represented using a row vector for each term, a basis matrix containing the coefficients for each term-point pair, and a vector of geometric points to perform the calculation on: $P(t) = TMG$.
- This allows a spline piece of determined order and method to be precomputed before the points' positions are known.
- Can be made to behave like other curves and splines - including Bezier splines - by performing operations on the basis matrix or enforcing constraints on the points before they are used.
- Any spline can be expressed by a linear combination of B-splines with equal and lower orders.

### Additional Variants
- Natural: The second derivative of the cubic basis function equates to zero at all boundary points, ensuring acceleration continuity across the spline.
- Cardinal: All pairs of neighbouring knots are equally spaced from each other, yielding a uniform sample domain useful for efficient interpolation.
- Rational: Each point is given a weight which determines the knot's relative influence on the curve, presented in rational basis functions.

## Samplers
Samplers are used to give control over how samples are extracted from an analytical curve, to be converted into renderable data points. Harshly-winding curves may appear sharper for basic samplers than for complex samplers, but complex samplers may be less efficient for computations, take more space in memory, or overly-biased towards certain curvatures.

### Types & Properties
- Constant: sample points at fixed intervals along the curve domain.
- Spatial: sample points at constrained distances apart from each other.
- Curvature: sample points by constraining the angles between each set of three adjacent points, as well as the distance from sampled points to the line drawn between their neighbours (the discrepancy in positional correctness, were that point not have been sampled, is thus limited).

### Completion
Allow the display of any continuous curves representable as data, especially as exclusively input points
- implement more fundamental curve/spline types (Hermite, NURBS, Catmull-Rom, direct variant implementations, general-order versions, etc.)
- feature intuitive inputs and interface to user, suitable for editing and altering curves (multiple togglable input types alongside exclusively control point manipulation?)
- provide complete and straightforward infrastructure of computer-generated splines, sufficient for learning and understanding relevant topics and applications

## Future Work

### 3-D Environment
Branch into 3D space to prioritise representing and rasterising free-form surfaces, constructable from data inside .obj files of the wavefront file format.
- 3D curves
- surfaces (types including bezier, NURBS, variants, etc.)
- read & write complete geometric data from .obj files (wavefront file structure)

### Drawing
Enable user to draw 2D vector graphics using a combination of points, polygons, and free-form geometry.
- glyphs/brushes
- vectorised and rasterised output data storage options