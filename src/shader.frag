#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightDir;    // spotlight direction
uniform float cutOff;     // cosine of inner angle
uniform float outerCutOff;// cosine of outer angle

uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    // ===== Ambient =====
    float baseAmbientStrength = 0.05;    // default ambient everywhere
    float outerAmbientBoost = 0.10;      // extra ambient beyond outer cone
    vec3 ambient = baseAmbientStrength * lightColor;

    // ===== Diffuse =====
    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor;

    // ===== Specular =====
    float specularStrength = 0.3;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDirection, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    vec3 specular = specularStrength * spec * lightColor;

    // ===== Spotlight intensity =====
    vec3 spotDir = normalize(-lightDir); // lightDir points out from camera
    float theta = dot(lightDirection, spotDir);

    float epsilon = cutOff - outerCutOff;
    float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);

    if (theta > cutOff) {
        // Fully lit inside inner cone → no change
    } 
    else if (theta > outerCutOff) {
        // Between inner and outer cone → fade
        diffuse *= intensity;
        specular *= intensity;
    } 
    else {
        // Beyond outer cone → dim but still shaped
        float outerDiffuseFactor = 0.1;  // reduced diffuse outside cone
        float outerSpecularFactor = 0.1; // tiny specular outside cone

        diffuse *= outerDiffuseFactor;
        specular *= outerSpecularFactor;
        ambient += outerAmbientBoost * lightColor;
    }

    // ===== Final Color =====
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
