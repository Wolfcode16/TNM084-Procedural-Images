#version 150

out vec4 outColor;

in float height; // receive height from vertex shader
in vec2 texCoord;
in vec3 exNormal;
uniform sampler2D tex;

void main(void)
{
    // Height thresholds for different terrain types
    float waterLevel = -1.0;     // Water
    float sandLevel = 0.0;       // Beach/sand
    float grassLevel = 2.0;      // Grass
    float rockLevel = 3.0;       // Rocky mountain
    float snowLevel = 4.0;       // Snow cap

    // Colors for each terrain type
    vec3 waterColor = vec3(0.0, 0.3, 0.7);   // Blue
    vec3 sandColor = vec3(0.8, 0.7, 0.5);    // Sandy beige
    vec3 grassColor = vec3(0.2, 0.6, 0.2);   // Green
    vec3 rockColor = vec3(0.5, 0.4, 0.3);    // Rocky
    vec3 snowColor = vec3(0.9, 0.9, 1.0);    // Snow

    vec3 terrainColor;

    // Determine color based on height with blending
    if (height < waterLevel)
    {
        terrainColor = waterColor;
    }
    else if (height < sandLevel)
    {
        // ==== BLENDING HELP BY AI ====
        // Blend between water and sand
        float blend = (height - waterLevel) / (sandLevel - waterLevel);
        terrainColor = mix(waterColor, sandColor, blend);
    }
    else if (height < grassLevel)
    {
        // Blend between sand and grass
        float blend = (height - sandLevel) / (grassLevel - sandLevel);
        terrainColor = mix(sandColor, grassColor, blend);
    }
    else if (height < rockLevel)
    {
        // Blend between grass and rock
        float blend = (height - grassLevel) / (rockLevel - grassLevel);
        terrainColor = mix(grassColor, rockColor, blend);
    }
    else if (height < snowLevel)
    {
        // Blend between rock and snow
        float blend = (height - rockLevel) / (snowLevel - rockLevel);
        terrainColor = mix(rockColor, snowColor, blend);
    }
    else
    {
        terrainColor = snowColor;
    }

    // Lighting
    vec3 lightDir = normalize(vec3(1.0, 0.4, 0.3));  // Light coming from above and slightly to the side
    vec3 norm = normalize(exNormal);

    float diffuseStrength = dot(norm, lightDir);
    float ambientLight = 0.2;

    float diffuse = max(diffuseStrength, 0.0) * 0.8 + ambientLight;

    // Apply lighting to terrain color
    vec3 finalColor = terrainColor * diffuse;

    outColor = vec4(finalColor, 1.0);
}
