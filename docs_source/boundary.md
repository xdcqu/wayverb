---
layout: page
title: Boundary modelling
navigation_weight: 8
---

---
reference-section-title: References
...

# Boundary Modelling {.major}

## Introduction

The ideal boundary model would allow complete control over the frequency- and
direction-dependent absorption and scattering of a surface.  Though this is
reasonably straightforward to implement in geometric models, it is far from a
solved problem for the digital waveguide mesh (DWM).  Several possible
implementations are discussed in the literature, each with unique drawbacks.

This chapter relies on terms defined in the [Theory]({{ site.baseurl }}{% link
theory.md %}) section.  It begins by discussing the ideal behaviour of modelled
acoustic boundaries.  Then, the implementation for geometric models is
discussed.  Possibilities for DWM boundary models are investigated, and the
final choice of method explained.  The geometric and DWM implementations are
evaluated and compared, to ensure equivalence.

## Geometric Implementation

In Wayverb, surfaces may have different absorptions in each frequency band.
Each ray starts with the same pressure in each band. During a specular
reflection, the per-band absorptions are converted into per-band reflection
factors. Then, the pressure in each band is adjusted using that band's
reflection coefficient. This is similar to the approach taken in graphical ray
tracing, in which each ray carries separate red, green, and blue components.
These components are modified independently, depending on the colour of the
reflective surface.

By definition, image-source models find only specular reflections (i.e. image
sources), so scattering is not implemented in these models.  Scattering can be
implemented in ray tracers, but there is no consensus on the optimum method.
One option is to spawn two rays at every reflection: a specular ray, and a
diffuse ray with random direction.  Though this properly replicates the theory,
it leads to an explosion in the number of rays which must be traced, so is
impractical in most cases.  A second option is to decide, using the scattering
coefficient as a probability, whether the reflection should be specular or
diffuse [@savioja_overview_2015].  This solves the ray-explosion problem, but
requires an additional random number to be generated per-reflection, which can
be costly for large numbers of rays.  An elegant solution is to simply mix the
specular and diffuse rays together, using the scattering coefficient as a
weighting [@rindel_use_2000], a technique known as *vector based scattering*
[@christensen_new_2005].  This is the approach taken by Wayverb.  A major
drawback of all these scattering methods is that the scattering coefficient can
only be frequency-dependent if a separate ray is traced for each band.  If a
single ray is used to carry all frequency components, then each component must
be scattered in exactly the same way.

The plain scattering model affects only the ongoing ray direction and
amplitude.  However, it is worth considering that, at each reflection, the
scattered energy may be directly visible to the receiver.  This fact is
exploited by the *diffuse rain* technique [@schroder_physically_2011, pp.
61-66], in which each reflection is considered to spawn a "secondary source"
which emits scattered energy towards the receiver.  This scattered energy is
recorded only if the secondary source is visible from the receiver.

The magnitude of scattered energy is proportional to the scattering
coefficient, and the Lambert diffusion coefficient.  It is also proportional to
the fraction of the available hemispherical output area which is covered by the
receiver volume.  The absolute area covered by a spherical receiver
$A_{\text{intersection}}$ is found using the equation for the surface area of a
spherical cap.
 
$$A_{\text{intersection}} = 2\pi r^2(1-\cos\gamma)$$ {#eq:}

Then, the detected scattered energy can be derived [@schroder_physically_2011, p. 64]:

$$ \begin{aligned}
E_{\text{scattered}} & = E_{\text{incident}} \cdot s(1-\alpha) \cdot 2\cos\theta \cdot \left( \frac{A_{\text{intersection}}}{2\pi r^2} \right) \\
                     & = E_{\text{incident}} \cdot s(1-\alpha) \cdot 2\cos\theta \cdot \left( \frac{2\pi r^2(1-\cos\gamma)}{2\pi r^2} \right) \\
                     & = E_{\text{incident}} \cdot s(1-\alpha) \cdot 2\cos\theta \cdot (1-\cos\gamma)
\end{aligned}
$$ {#eq:}

Here, $\theta$ is the angle from secondary source to receiver relative against
the surface normal, and $\gamma$ is the opening angle (shown in
+@fig:diffuse_rain).  The magnitude of the scattered energy depends on the
direction from the secondary source to the receiver (by Lambert's cosine law),
and also on the solid angle covered by the receiver.

![Angles used in the diffuse rain equation for a spherical
receiver.](images/diffuse_rain){#fig:diffuse_rain}

## DWM Implementation

### Possible Methods

Two methods from the literature were considered for use in Wayverb. A brief
overview of each will be given here.

#### KW-Pipe Technique

This method is described in [@murphy_kw-boundary_2007] and
[@kelloniemi_frequency-dependent_2006].

As described in the [Digital Waveguide Mesh]({{ site.baseurl }}{% link
waveguide.md %}) section, there are two technically-equivalent formulations of
digital waveguides meshes, known as *W-models* and *K-models*. W-models allow
for straightforward interaction with a variety of termination types, such as
wave digital filters, which can be used to model frequency-dependent boundaries
and air absorption.  However, W-models use more than twice the memory of the
equivalent K-model [@beeson_roomweaver:_2004].  For large-scale simulations,
K-models are preferable for their reduced memory usage. However, K-models
cannot interact directly with wave digital filters.

The KW-pipe is a "converter" between wave- and Kirchhoff- variables, which is
designed to allow the majority of a model (that is, the air-filled space inside
it) to be constructed as a K-model waveguide mesh.  At the boundaries of the
model, the KW-pipe is used to connect K-model nodes to W-model nodes.  These
W-model nodes can then be connected to wave digital filters to simulate
frequency-dependent absorption of wave energy.  The complete model retains both
the memory-efficiency of the K-model and the termination flexibility of the
W-model, with the drawback of additional implementation complexity at the
interface between the two model types.

This sounds extremely promising, but has a major drawback, as described by
Kowalczyk and Van Walstijn [@kowalczyk_modeling_2008]: while the inside of the mesh
will be 2- or 3-dimensional, the boundary termination afforded by the
wave-variable boundary is 1-dimensional.  Each boundary node connects to just
the closest interior node.  As a result, the edges and corners are not
considered to be part of the model, as these nodes do not have a directly
adjacent interior node.  Additionally, the 1D boundary termination equation
implies a smaller inter-nodal distance than that of the 2D or 3D mesh interior.
This means that when updating an interior node next to a boundary, the
inter-nodal distance is greater than when updating the boundary node itself.
For these reasons, the 1D termination is unphysical and can lead to large
errors in the phase and amplitude of reflections [@kowalczyk_modeling_2008].

#### Locally Reactive Surfaces Technique

This method, described in [@kowalczyk_modeling_2008], aims to create physically
correct higher-dimensional boundaries by combining a boundary condition,
defined by a boundary impedance, with the multidimensional wave equation.  This
leads to a model for a *locally reacting surface* (LRS), in which boundary
impedance is represented by an infinite-impulse-response (IIR) filter.

As noted above, a surface is locally reacting if the normal component of the
particle velocity on the boundary surface is dependent solely upon the sound
pressure in front of the boundary.  In most physical surfaces, the velocity at
the surface boundary will also be influenced by the velocity at adjacent points
on the boundary, so LRS is not a realistic physical model in the vast majority
of cases.

However, despite that it is not a realistic physical model, the implementation
of the LRS modelling technique is both stable and accurate, as opposed to the
1D KW-pipe termination, which does not accurately model even locally-reacting
surfaces.

The LRS model leads to an implementation that is efficient (as it is based
completely on the K-model/FDTD formulation) and tunable (boundaries are defined
by arbitrary IIR filters).

### Choice of Boundary Technique for the DWM

The LRS technique was chosen, as it represented the best compromise between
memory efficiency, customization and tuning, and physically-based behaviour
(i.e. edges and corners are considered as well as flat surfaces).  The
particular strengths of this model are its performance and tunability, though
as mentioned previously it is not physically accurate in many cases.  That
being said, neither of the boundary models considered are particularly
realistic, so even for applications where physical modelling is the most
important consideration, the LRS model seems to be the most appropriate.

### LRS Implementation

See [@kowalczyk_modeling_2008] and [@kowalczyk_modelling_2008] for a more
detailed explanation.

The reflectance of a LRS has been defined earlier, in terms of the
normal-incidence specific impedance $\xi_0$ (+@eq:r_normal_incidence).  For the
geometric implementation, $\xi_0$ was defined in terms of a single
normal-incidence reflection coefficient $R_0$ (+@eq:xi_0). If $R_0$ is replaced
by a digital filter $R_0(z)$, then the specific impedance may also be expressed
as a filter $\xi_0(z)$:

$$\xi_0(z)=\frac{1+R_0(z)}{1-R_0(z)}$$ {#eq:xi_filter}

To create the filter $R_0$, per-band normal reflection magnitudes are found
using the relationship $|R|=\sqrt{1-\alpha}$. Then, the Yule-Walker method is
used to find *infinite impulse response* (IIR) coefficients for a filter with
an approximately-matched frequency response. Then, this filter is substituted
into +@eq:xi_filter to find IIR coefficients for the specific impedance filter.
This impedance filter will eventually be "embedded" into the boundary nodes of
the waveguide.

Surfaces with detailed frequency responses will require high-order filters.
This generally leads to numerical instability in IIR filters. The usual
solution to this problem would be to split the high-order filter into a
series-combination of lower-order filters. However, the LRS requires access to
intermediate values from the filter delay-line which makes this approach
impossible.  An alternative solution is suggested in
[@oxnard_frequency-dependent_2015], which suggests running the entire
simulation multiple times, once for each octave band.  This means that the
boundary filters can be single-order, and resistant to accumulated numerical
error.  Compared to high-order boundary filters, this method gives much
improved accuracy, but at the cost of running the entire simulation multiple
times.  In Wayverb, both approaches are possible, allowing the user to choose
between a fast, inaccurate single-run simulation with high-order filters; or a
slow, accurate multi-run simulation with low-order filters.

To implement the waveguide boundaries, the computed impedance filter
coefficients are inserted into three special update equations, which are found
by combining the discrete 3D wave equation with the discrete LRS boundary
condition. These equations are used instead of the standard update equation
when updating the boundary nodes.  The exact update equations have not been
reproduced here due to space constraints, but they can be found in
[@kowalczyk_modeling_2008], alongside a thorough derivation and explanation.
The implementation in Wayverb does not make any modifications to these
equations.

The three different update equations are chosen depending on the placement of
the boundary nodes.  In the case of a flat wall, the boundary node is adjacent
to a single inner-node, and a "1D" update equation is used.  Where two
perpendicular walls meet, the nodes along the edge will each be adjacent to two
"1D" nodes, and a "2D" update equation is used for these nodes.  Where three
walls meet, the corner node will be directly adjacent to three "2D" nodes, and
a "3D" update equation is used for this node.  The three types of boundary
nodes are shown in +@fig:boundary_type_diagram.  Note that this method is only
capable of modelling mesh-aligned surfaces.  Other sloping or curved surfaces
must be approximated as a group of narrow mesh-aligned surfaces separated by
"steps".  For example, a wall tilted at 45 degrees to the mesh axes will be
approximated as a staircase-like series of "2D" edge nodes.

![The three types of boundary nodes, used to model reflective planes, edges,
and corners. 1D nodes are adjacent to inner nodes, 2D nodes are adjacent to two
1D nodes, and 3D nodes are adjacent to three 2D
nodes.](images/boundary_diagram){#fig:boundary_type_diagram}

## Testing of the LRS Boundary for the DWM

The LRS waveguide boundary is complex to implement, as it embeds IIR filters
into the waveguide boundaries, so it is worth ensuring that the boundary nodes
behave as expected.

Although the 3D boundary equations are presented in [@kowalczyk_modeling_2008],
only 2D boundaries are tested. Therefore the test shown in this thesis is a
novel contribution, as no previous empirical evidence exists for the 3D LRS
boundary implementation in the waveguide mesh. The test used here is an
extension of the test procedure presented in  [@kowalczyk_modeling_2008], but
extended to three dimensions. 

### Method

A mesh with dimensions $300 \times 300 \times 300$ nodes, and a sampling
frequency of 8kHz, was set up.  A source and receiver were placed at a distance
of 37 node-spacings from the centre of one wall. The source position was
dictated by an azimuth and elevation relative to the centre of the wall, with
the receiver placed directly in the specular reflection path.  The simulation
was run for 420 steps. The first output, $r_f$, contained a direct and a
reflected response.  Then, the room was doubled in size along the plane of the
wall being tested.  The simulation was run again, recording just the direct
response at the receiver ($r_d$).  Finally, the receiver position was reflected
in the boundary under test, creating a *receiver image*, and the simulation was
run once more, producing a free-field response at the image position ($r_i$).
*@fig:boundary_test_setup shows the testing setup.

When testing on-axis reflections (where azimuth and elevation are both 0), the
position of the receiver will exactly coincide with the position of the source.
If a hard source is used, the recorded pressures at the receiver ($r_f$ and
$r_d$) will always exactly match the input signal, and will be unaffected by
reflections from the boundary under test. It is imperative that the signal at
the receiver contains the reflected response, so a hard source is not viable
for this test. Instead, a transparent dirac source is used, which allows the
receiver node to respond to the reflected pressure wave, even when the source
and receiver positions match.  The main drawback of the transparent source,
solution growth, is not a concern here as the simulations are only run for
420 steps. The tests in the [Digital Waveguide Mesh]({{ site.baseurl }}{%
link waveguide.md %}) section showed that solution growth generally only
becomes evident after several thousand steps.

![The setup of the two room-sizes, and the positions of sources and receivers
inside.](images/boundary_testing_setup){#fig:boundary_test_setup}

The reflected response was isolated by subtracting $r_d$ from $r_f$, cancelling
out the direct response.  This isolated reflection is $r_r$.  To find the
effect of the frequency-dependent boundary, the frequency content of the
reflected response was compared to the free-field response $r_i$.  This was
achieved by windowing $r_r$ and $r_i$ with the right half of a Hann window,
then taking FFTs of each.  The experimentally determined numerical reflectance
was determined by dividing the magnitude values of the two FFTs.

To find the accuracy of the boundary model, the numerical reflectance was
compared to the theoretical reflection of the digital impedance filter being
tested, which is defined as:

$$R_{\theta, \phi}(z) = \frac{\xi(z)\cos\theta\cos\phi -
1}{\xi(z)\cos\theta\cos\phi + 1}$$ {#eq:theoretical_reflection}

where $\theta$ and $\phi$ are the reflection azimuth and elevation
respectively.

The test was run for three different angles of incidence, with matched azimuth
and elevation angles of 0, 30, and 60 degrees respectively.  Three different
sets of surface absorption coefficients were used, giving a total of nine
combinations of source position and absorption coefficients.  The specific
absorption coefficients are those suggested in
[@oxnard_frequency-dependent_2015], shown in +@tbl:absorption.

Table: Absorption coefficients of different materials at different frequencies,
taken from [@oxnard_frequency-dependent_2015]. {#tbl:absorption}

-------------------------------------------------------------------------------
band centre frequency / Hz  31      73      173     411     974
--------------------------- ------- ------- ------- ------- -------------------
plaster                     0.08    0.08    0.2     0.5     0.4

wood                        0.15    0.15    0.11    0.1     0.07

concrete                    0.02    0.02    0.03    0.03    0.03
-------------------------------------------------------------------------------

The boundary filter for each material was generated by converting the
absorption coefficients to per-band reflectance coefficients using the
relationship $R=\sqrt{1-\alpha}$.  Then, the Yule-Walker method from the ITPP
library [@_itpp_2013] was used to calculate coefficients for a sixth-order IIR
filter which approximated the per-band reflectance.  This filter was converted
to an impedance filter by $\xi(z)=\frac{1+R_0(z)}{1-R_0(z)}$, which was then
used in the boundary update equations for the DWM.

### Results

The results are shown in +@fig:reflectance.  The lines labelled "measured" show
the measured boundary reflectance, and the lines labelled "predicted" show the
theoretical reflectance, obtained by substituting the impedance filter
coefficients and angles of incidence into +@eq:theoretical_reflection.
Although the waveguide mesh has a theoretical upper frequency limit of 0.25 of
the mesh sampling rate, the 3D FDTD scheme has a cutoff frequency of 0.196 of
the mesh sampling rate for axial directions.  This point has been marked as a
vertical line on the result graphs.

![Measured boundary reflectance is compared against the predicted reflectance,
for three different materials and three different angles of
incidence.](images/reflectance){#fig:reflectance}

### Evaluation

The most concerning aspect of the results is the erratic high-frequency
behaviour. Even though the cutoff of the 3D FDTD scheme is at 0.196 of the mesh
sampling rate, deviations from the predictions are seen below this cutoff in
all the presented results. The cause of this error is unclear. One possibility
is numerical dispersion, which is known to become more pronounced as frequency
increases.  However, the rapid onset of error around the cutoff suggests that
the cause is not dispersion alone.  Another possible explanation might be extra
unwanted reflections in the outputs, although this seems unlikely. The use of a
transparent source means that the source node does not act as a reflector, as
would be the case with a hard source. In addition, the dimensions of the test
domain were chosen to ensure that only reflections from the surface under test
are present in the output. The recordings are truncated before reflections from
other surfaces or edges are able to reach the receivers. Finally, it seems
likely that such interference would affect the entire spectrum, rather than
just the area around the cutoff.  A final, more likely, explanation for the
volatile high-frequency behaviour is that the filters used in this test are of
much higher order than those tested in [@kowalczyk_modeling_2008], leading to
accumulated numerical error.

Whatever the cause of the poor performance at the top of the spectrum, the
implications for Wayverb are minor, as the waveguide mesh is designed to
generate low-frequency content.  If wideband results are required, then the
mesh can simply be oversampled.  To prevent boundary modelling error affecting
the results of impulse response synthesis in the Wayverb app, the mesh cutoff
frequency is locked to a maximum of 0.15 of the mesh sampling rate.

Some of the results (especially concrete and wood at 60 degrees) show minor
artefacts below 0.01 of the mesh sampling rate, where the measured responses
diverge from the predictions.  Kowalczyk and Van Walstijn note that some of
their results display similar low-frequency artefacts, and suggest that the
cause is that the simulated wave-front is not perfectly flat.  However, flat
wave-fronts are not easily accomplished.  The experiments in
[@kowalczyk_modeling_2008] use large meshes (around 3000 by 3000 nodes, nine
million in total) and place the sources a great distance away from the boundary
being studied in order to maintain a mostly-flat wave-front.  This is only
practical because the experiments are run in two dimensions.  For the 3D case,
no experimental results are given.  This is probably because running a 3D
simulation on a similar scale would require a mesh of twenty-seven billion
nodes, which in turn would require gigabytes of memory and hours of simulation
time.

In conclusion, for the most part, the results presented adhere closely to the
expected results, with the caveat that the surface reflectance is only accurate
at low frequencies, below around 0.15 of the mesh sampling rate.  Different
absorption coefficients lead to clearly-different reflectance coefficients,
which are additionally accurate at multiple angles of incidence.  Whilst a more
accurate method would be preferable, this model is both fast and tunable,
making it a good candidate for boundary modelling in room acoustics
simulations.

## Summary

In the image source model, outgoing pressure from a boundary can be found as a
function of incident angle and surface attenuation; scattering cannot be
modelled.  The ray tracer, however, is able to model scattering in two ways.
Firstly, the average scattering coefficient across all frequency bands is used
to govern the magnitude of random offset of the outgoing ray direction.
Secondly, the diffuse rain technique is used to model frequency-dependent
scattering.

Boundary models in the waveguide are more involved. Wayverb uses the LRS
technique, as it is a better model of physical behaviour than the alternative
KW-pipe technique. The LRS model uses special update equations, which contain
embedded IIR filters, to calculate pressure values at designated boundary
nodes. This is effective for boundaries which are aligned to the mesh, but for
curved or angled surfaces the boundary will be quantised into steps. The effect
of this quantisation has not been investigated.

The waveguide boundary implementation has been tested to see whether the
measured reflectance matches the theoretical surface reflectance. In all tests,
the match is reasonably close around the middle of the valid spectrum, between
0.01 and 0.1 of the mesh sampling rate. Outside this range, the results tend to
deviate somewhat. As a result, the model crossover frequency in Wayverb has a
maximum of 0.15 of the waveguide sampling frequency, and it is recommended to
oversample the mesh if highly accurate boundary results are required. It is
hoped that the low frequency artefacts will always occur below the audible
range, although it is unclear whether this is really the case as tests have
only been conducted at a single sampling frequency. Only 1D boundaries have
been tested, as it is unclear how to test 2D and 3D boundaries in isolation.
Given that all boundary types (1D, 2D and 3D) have the same derivation, it may
be enough to assume that if the 1D boundaries function correctly, then the 2D
and 3D boundaries will work too. However, this does not rule out the
possibility of implementation mistakes in the higher-order boundaries.

Finally, the boundary model has not been tested in the context of an extended
waveguide simulation. It would be expected that the absorption of surfaces in a
room would have an effect on the overall reverb time in that room. This
relationship has not been shown to be true of Wayverb's waveguide. A
particularly interesting test would be to set different absorption coefficients
in each of the waveguide frequency bands, to estimate Sabine reverb times in
each of those bands, and to see whether the measured reverb time in each band
matches the predictions. Such a test was not possible due to time constraints,
but would help to clarify some of the results shown in the [Evaluation]({{
  site.baseurl }}{% link evaluation.md %}).
