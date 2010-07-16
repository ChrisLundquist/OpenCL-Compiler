
__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void image_filli(write_only image2d_t image, int4 value)
{
    write_imagei(image, (int2)(get_global_id(0), get_global_id(1)), value);
}

__kernel void down_sample(write_only image2d_t output, read_only image2d_t input)
{
    const int2 position_down_scaled = (int2)(get_global_id(0), get_global_id(1));
    const int2 position = position_down_scaled * 2;
    const uint a = read_imageui(input, sampler, position).x;
    const uint b = read_imageui(input, sampler, position + (int2)(0,1)).x;
    const uint c = read_imageui(input, sampler, position + (int2)(1,0)).x;
    const uint d = read_imageui(input, sampler, position + (int2)(1,1)).x;
    const uint result = (a + b + c + d) / 4;
    write_imageui(output, position_down_scaled, (uint4)(result, result, result, result));
}


int median(int a, int b, int c)
{
    int t = a;
    a = max( a, b );
    b = min( t, b );
    a = min( c, a );
    a = max( b, a );
    return a;
}

#define LAMBDA 4

// leave in float? need the precision of log2?
// mv must be in qpel
uint delta_cost2(int2 mv)
{
    float2 mvc_lg2 = native_log2( convert_float2( abs( mv ) + (uint2)( 1 ) ) );
    float2 rounding = (float2)(!!mv.x, !!mv.y);
    uint2 mvc = convert_uint2(round(mvc_lg2 * 2.0f + 1.218f /*0.718f + .5f*/ + rounding));
    return LAMBDA * (mvc.x + mvc.y);
}
inline uint simple_sad( image2d_t pix1, image2d_t pix2, int2 block, int2 motion_vector, uchar size )
{
    uint sum = 0;
    int2 pos;

    for (pos.y = block.y; pos.y < block.y + size; pos.y++)
        for (pos.x = block.x; pos.x < block.x + size; pos.x++) {
            // TODO: read 4 byte at a time and 
            uint a = read_imageui(pix1, sampler, (int2)(pos)).s0;
            uint b = read_imageui(pix2, sampler, (int2)(pos + motion_vector)).s0;
            sum += abs_diff(a, b);
        }
    return sum;
}

#define MAX_SEARCH 32
inline int2 find_best_motion_vector(
        read_only image2d_t imageA,
        read_only image2d_t imageB,
        int2 block,
        uchar block_size
        ){
    // This will look for a match in imageB
    short diameter = 0; // length of side of spiral
    int2 test_block = (int2)(0,0);
    int2 best_block = (int2)(0,0);
    uint best_value = UINT_MAX;
    uint test_value = UINT_MAX;

    while( diameter < MAX_SEARCH )
    {
        // First we scan horizontally
        // We need to test the origin of the spiral, so we use a do .. while loop here
        // the corners will be scanned in the vertical section
        short i = 0;
        do {
            // Compare
            test_value = simple_sad(imageA, imageB, test_block, block, block_size);

            if(test_value < best_value)
            {
                best_value = test_value;
                best_block = test_block;
            } else{
                // Early Termination
                return best_block;
            }

            if (diameter & 1) // Alternate direction of scan for even and odd iteratorations so we remain centered on the origin
                test_block.x++; // odd
            else
                test_block.x--; // even
        } while( i++ < diameter );

        // Scan vertial sides of spiral
        i = 0;
        do {
            // Compare
            test_value = simple_sad(imageA,imageB,test_block,block,block_size);

            if(test_value < best_value)
            {
                best_value = test_value;
                best_block = test_block;
            } else{
                // Early Termination
                return best_block;
            }

            if (diameter & 1)
                test_block.y++; // odd
            else
                test_block.y--; // even
        } while( i++ < diameter );
    }
    return best_block;
}

__kernel void motion_estimation(
        read_only image2d_t imageA,
        read_only image2d_t imageB,
        write_only image2d_t result,
        int block_size
        )
{
    int2 block = (int2)(get_global_id(0),get_global_id(1));
    // For each block in imageA ...
    int2 motion_vector = find_best_motion_vector(imageA,imageB,block,block_size);
    write_imagei(result,(int2)(block),(int4)(motion_vector,0,0));
}
