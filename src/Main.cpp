#include <iostream>
#include <vector>
#include <string>

// #include "Camera.h"
#include "Image.h"

using namespace std;

int main()
{

    Image image(800, 800, Image::RGB);
    Image innerImage(400, 400, Image::RGB);

    Vector2i t0(0,0);
    Vector2i t1(0,400);
    Vector2i t2(400,0);

    image.draw_triangle(t0, t1, t2, Colour(WHITE, Image::RGB));
    innerImage.draw_triangle(t0, t1, t2, Colour(RED, Image::RGB));
    image.draw_image(innerImage, 400, 400);
    image.flip_vertically();
    image.write_tga_file("/Users/nic/Cpp projects/PaintMeAPicture/out/im.tga");
}
