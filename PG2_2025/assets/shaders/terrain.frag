#version 460 core

in vec3 FragPos;   // Interpolated position from vertex shader
in vec3 Normal;    // Interpolated normal
in vec2 TexCoords; // Interpolated texture coordinates

uniform sampler2D tex0; // Texture sampler
uniform vec3 lightPos;  // Light position
uniform vec3 viewPos;   // Camera position

out vec4 FragColor;

void main()
{
    vec3 lightColor = vec3(1.0, 1.0, 1.0); // White light

    // Ambient lighting
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular lighting
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    // Combine lighting components
    vec3 lighting = ambient + diffuse + specular;

    // Sample texture
    vec4 texColor = texture(tex0, TexCoords);

    // Final fragment color
    FragColor = vec4(lighting, 1.0) * texColor;
}
