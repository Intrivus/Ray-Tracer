struct Input {
    [[vk::location(0)]] float3 pos   : TEXCOORD0;
    [[vk::location(1)]] float3 color : TEXCOORD1;
};

struct Output {
    float4 color : TEXCOORD0;
    float4 pos   : SV_Position;
};

Output main(Input i) {
    Output o;
    o.pos = float4(i.pos, 1.0);
    o.color = float4(i.color, 1.0);
    return o;
}