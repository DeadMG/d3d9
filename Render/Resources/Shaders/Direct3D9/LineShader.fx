uniform shared extern float4x4 CameraVP;

struct OutputVS {
    float4 posH : POSITION0;
    float4 VertexColour : COLOR0;
};

OutputVS BasicVertexShader(
    float3 ObjectSpacePosition : POSITION0,
    float4 VertexColour : COLOR0,
    float4 WorldMatrixRow1 : TEXCOORD0,
    float4 WorldMatrixRow2 : TEXCOORD1,
    float4 WorldMatrixRow3 : TEXCOORD2,
    float4 WorldMatrixRow4 : TEXCOORD3
) {
    OutputVS output;
    float4x4 World = { WorldMatrixRow1, WorldMatrixRow2, WorldMatrixRow3, WorldMatrixRow4 };
    output.posH = mul(mul(float4(ObjectSpacePosition, 1.0f), World), CameraVP);
    output.VertexColour = VertexColour;
    return output;
}

float4 BasicPixelShader(
    float4 Colour : COLOR0
) : COLOR0 {
    return Colour;
}

technique PrimaryTechnique
{
      pass P0
      {
            // Specify the vertex and pixel shader associated
            // with this pass.
            vertexShader = compile vs_3_0 BasicVertexShader();
            pixelShader  = compile ps_3_0 BasicPixelShader();
      }
}