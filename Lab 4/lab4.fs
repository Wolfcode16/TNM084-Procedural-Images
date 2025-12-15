#version 410 core

in vec2 gsTexCoord;
in vec3 gsNormal;
in float height;    // Task 6

out vec4 out_Color;

uniform float displacementAmount; // Tells us terrain height range

void main(void)
{

    // =================== TAKEN FROM LAB3B ===================
    // Manually set thresholds based on actual observed heights
    // Adjust these values by looking at your terrain
    float waterLevel = 0.90;   // Know water is less than 1.0
    float sandLevel = 0.95;
    float grassLevel = 1.00;
    float rockLevel = 1.05;
    float snowLevel = 1.10;

    // Colors for each terrain type
    vec3 waterColor = vec3(0.1, 0.3, 0.6);   // Blue
    vec3 sandColor = vec3(0.8, 0.7, 0.5);    // Sandy beige
    vec3 grassColor = vec3(0.2, 0.6, 0.2);   // Green
    vec3 rockColor = vec3(0.5, 0.4, 0.3);    // Rocky brown
    vec3 snowColor = vec3(0.95, 0.95, 1.0);  // Snow white

    vec3 terrainColor;

    // Determine color based on height with blending
    if (height < waterLevel)
    {
        terrainColor = waterColor;
    }
    else if (height < sandLevel)
    {
        float blend = (height - waterLevel) / (sandLevel - waterLevel);
        terrainColor = mix(waterColor, sandColor, blend);
    }
    else if (height < grassLevel)
    {
        float blend = (height - sandLevel) / (grassLevel - sandLevel);
        terrainColor = mix(sandColor, grassColor, blend);
    }
    else if (height < rockLevel)
    {
        float blend = (height - grassLevel) / (rockLevel - grassLevel);
        terrainColor = mix(grassColor, rockColor, blend);
    }
    else if (height < snowLevel)
    {
        float blend = (height - rockLevel) / (snowLevel - rockLevel);
        terrainColor = mix(rockColor, snowColor, blend);
    }
    else
    {
        terrainColor = snowColor;
    }

    // Lighting
    vec3 lightDir = normalize(vec3(1.0, 0.4, 0.3));
    vec3 norm = normalize(gsNormal);
    float diffuseStrength = dot(norm, lightDir);
    float ambientLight = 0.2;
    float diffuse = max(diffuseStrength, 0.0) * 0.8 + ambientLight;

    // Apply lighting to terrain color
    vec3 finalColor = terrainColor * diffuse;
    out_Color = vec4(finalColor, 1.0);

}
