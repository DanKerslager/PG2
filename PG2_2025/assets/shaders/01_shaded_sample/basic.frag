#version 460 core

in vec3 fragPos;
in vec3 normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D tex0;
uniform float alpha; // manuální průhlednost (0.0 - 1.0)

// Light settings
uniform vec3 lightDir = normalize(vec3(-1.0, -1.0, -1.0));
uniform vec3 lightColor = vec3(1.0);
uniform vec3 viewPos = vec3(0.0, 0.0, 5.0);

// Material
uniform vec3 ambientColor = vec3(0.2);
uniform vec3 diffuseColor = vec3(0.8);
uniform vec3 specularColor = vec3(1.0);
uniform float shininess = 32.0;

void main()
{
    vec3 norm = normalize(normal);
    vec3 lightDirection = normalize(-lightDir);

    // Ambient
    vec3 ambient = ambientColor * lightColor;

    // Diffuse
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diffuseColor * diff * lightColor;

    // Specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDirection, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularColor * spec * lightColor;

    // Sample texture
    vec4 texColor = texture(tex0, TexCoords);

    // Final color with lighting
    vec3 litColor = (ambient + diffuse + specular) * texColor.rgb;
    float finalAlpha = texColor.a * alpha; // combine texture alpha and manual override

    FragColor = vec4(litColor, finalAlpha);
}
