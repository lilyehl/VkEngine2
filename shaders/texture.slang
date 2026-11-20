// Logger overlay: 2D textured quads drawn directly in screen space.
// Matches Vertex2D (pos, color, texCoord) and the logger's descriptor set
// (binding 0 : combined image sampler).

struct VSInput {
    float2 inPos;      // already in NDC: x, y in [-1, 1], y points down
    float3 inColor;    // tint, multiplied with the texture
    float2 inTexCoord; // uv into the texture
};

// set 0, binding 0 : the logger's texture (image view + sampler)
[[vk::binding(0, 0)]]
Sampler2D overlayTexture;

struct VSOutput
{
    float4 pos : SV_Position;
    float3 fragColor;
    float2 fragTexCoord;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    // no camera: the overlay sits on the screen, not in the world
    output.pos = float4(input.inPos, 0.0, 1.0);
    output.fragColor = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    float4 texel = overlayTexture.Sample(vertIn.fragTexCoord);
    // texture alpha drives the blending (transparent around glyphs)
    return float4(texel.rgb * vertIn.fragColor, texel.a);
}
