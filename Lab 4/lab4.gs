#version 410 core

layout(triangles) in;
// Use line_strip for visualization and triangle_strip for solids
layout(triangle_strip, max_vertices = 3) out;
//layout(line_strip, max_vertices = 3) out;
in vec2 teTexCoord[3];
in vec3 teNormal[3];
out vec2 gsTexCoord;
out vec3 gsNormal;
out float height;

uniform sampler2D tex;
uniform mat4 projMatrix;
uniform mat4 mdlMatrix;
uniform mat4 camMatrix;

uniform float disp;
uniform int texon;

uniform float displacementAmount;

vec2 random2(vec2 st)
{
    st = vec2( dot(st,vec2(127.1,311.7)),
              dot(st,vec2(269.5,183.3)) );
    return -1.0 + 2.0*fract(sin(st)*43758.5453123);
}

// Gradient Noise by Inigo Quilez - iq/2013
// https://www.shadertoy.com/view/XdXGW8
float noise(vec2 st)
{
    vec2 i = floor(st);
    vec2 f = fract(st);

    vec2 u = f*f*(3.0-2.0*f);

    return mix( mix( dot( random2(i + vec2(0.0,0.0) ), f - vec2(0.0,0.0) ),
                     dot( random2(i + vec2(1.0,0.0) ), f - vec2(1.0,0.0) ), u.x),
                mix( dot( random2(i + vec2(0.0,1.0) ), f - vec2(0.0,1.0) ),
                     dot( random2(i + vec2(1.0,1.0) ), f - vec2(1.0,1.0) ), u.x), u.y);
}

vec3 random3(vec3 st)
{
    st = vec3( dot(st,vec3(127.1,311.7, 543.21)),
              dot(st,vec3(269.5,183.3, 355.23)),
              dot(st,vec3(846.34,364.45, 123.65)) ); // Haphazard additional numbers by IR
    return -1.0 + 2.0*fract(sin(st)*43758.5453123);
}

// Gradient Noise by Inigo Quilez - iq/2013
// https://www.shadertoy.com/view/XdXGW8
// Trivially extended to 3D by Ingemar
float noise(vec3 st)
{
    vec3 i = floor(st);
    vec3 f = fract(st);

    vec3 u = f*f*(3.0-2.0*f);

    return mix(
    			mix( mix( dot( random3(i + vec3(0.0,0.0,0.0) ), f - vec3(0.0,0.0,0.0) ),
                     dot( random3(i + vec3(1.0,0.0,0.0) ), f - vec3(1.0,0.0,0.0) ), u.x),
                mix( dot( random3(i + vec3(0.0,1.0,0.0) ), f - vec3(0.0,1.0,0.0) ),
                     dot( random3(i + vec3(1.0,1.0,0.0) ), f - vec3(1.0,1.0,0.0) ), u.x), u.y),

    			mix( mix( dot( random3(i + vec3(0.0,0.0,1.0) ), f - vec3(0.0,0.0,1.0) ),
                     dot( random3(i + vec3(1.0,0.0,1.0) ), f - vec3(1.0,0.0,1.0) ), u.x),
                mix( dot( random3(i + vec3(0.0,1.0,1.0) ), f - vec3(0.0,1.0,1.0) ),
                     dot( random3(i + vec3(1.0,1.0,1.0) ), f - vec3(1.0,1.0,1.0) ), u.x), u.y), u.z

          	);
}

// ================= TASK 4 =================
// Helper function for noise
void getDisplacedPosition(vec3 pos, out vec3 outPos)
{
    // Normalize to sphere first
    pos = normalize(pos);

    vec3 offset = vec3(123.456, 789.012, 345.678);  // Arbitrary values

    float noiseValue = 0.0;
    float frequency = 4.0;      // Starting freq
    float amplitude = 1.0;      // Starting amp
    float totalAmplitude = 0.0; // For normalization

    // Number of octaves of noise layers
    int octaves = 45;

    for (int i = 0; i < octaves; i++)
    {
        // Sample noise at current frequency
        noiseValue += noise((pos + offset) * frequency) * amplitude;

        // Keep track of total amplitude for normalization
        totalAmplitude += amplitude;

        // For next octave: double freq, halve amp
        frequency *= 2.0;
        amplitude *= 0.5;
    }

        // Normalize
        noiseValue /= totalAmplitude;

        float displacement = noiseValue * displacementAmount;
        float planetScale = 1.0;
        outPos = pos * (1.0 + displacement) * planetScale;
}


void computeVertex(int nr)
{
	vec3 p, v1, v2, v3, p1, p2, p3, s1, s2, n;

	// Gets original position from tessellation shader
	p = vec3(gl_in[nr].gl_Position);

	// ===== TASK 2 =====
	// Forces all points to be length = 1.0
    vec3 sphereNormal = normalize(p);

    // ===== TASK 5 (HELP/EXPLANATION FROM AI) =====
    // Find two perpendicular tangent vectors
    vec3 tangent1, tangent2;

    // Pick a vector that's not parallel to sphereNormal
    if (abs(sphereNormal.x) > 0.1)      // If normal points in the x-direction
        tangent1 = vec3(0.0, 1.0, 0.0); // Use the Y axis
    else
        tangent1 = vec3(1.0, 0.0, 0.0); // Otherwise use X axis

    // Make tangents perpendicular to sphereNormal
    tangent1 = normalize(cross(sphereNormal, tangent1));    // Perpendicular to sphereNormal + chosen axis
    tangent2 = normalize(cross(sphereNormal, tangent1));    // Perpendicular to sphereNormal + tangent1

    // Sample 3 points: Center + two neighbors along the tangent
    float delta = 0.01; // small step

    vec3 p0;                // Center point
    vec3 p1_pos, p1_neg;    // Along tangent1
    vec3 p2_pos, p2_neg;    // Along tangent2

    getDisplacedPosition(p, p0);                         // Center point
    getDisplacedPosition(p + tangent1 * delta, p1_pos);
    getDisplacedPosition(p - tangent1 * delta, p1_neg);
    getDisplacedPosition(p + tangent2 * delta, p2_pos);
    getDisplacedPosition(p - tangent2 * delta, p2_neg);

    // Create two vectors from p0 to the neighbors
    v1 = p1_pos - p1_neg;   // Motsatta vektorer skillnad :D
    v2 = p2_pos - p2_neg;

    // Cross product to get normal
    n = normalize(cross(v1,v2));

    height = length(p0);

	gl_Position = projMatrix * camMatrix * mdlMatrix * vec4(p0, 1.0);

    gsTexCoord = teTexCoord[nr];

	// n = teNormal[nr]; // This is not the normal you are looking for. Move along!
    gsNormal = mat3(camMatrix * mdlMatrix) * n;
    EmitVertex();
}

void main()
{
	computeVertex(0);
	computeVertex(1);
	computeVertex(2);
}

