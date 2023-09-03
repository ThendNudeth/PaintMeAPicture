#include <iostream>
#include <vector>
#include <string>
#include <dirent.h>

#include "Image.h"

using namespace std;
void small8bit() {
	Image bit8(16,16,1);
	for (size_t i = 0; i < 16; i++)
	{
		for (size_t j = 0; j < 16; j++)
		{
			bit8.set(i, j, Colour(i*j, 1));
		}
	}
	
	// bit8.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/8bit_p.bmp", true);
	bit8.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/8bit.bmp");
}
void read_then_write_bmp()
{
	try
	{
		string resourcesPath = "/Users/nic/Cpp-projects/PaintMeAPicture/resources/images/bitmaps";
 
		DIR* d;
		struct dirent *dir;
		vector<string> dirlist;
		int i=0;
		d = opendir(resourcesPath.c_str());
		if (d==nullptr)
			throw "Could not open directory";

		while ((dir = readdir(d)) != NULL)
		{
			i++;
			string s(dir->d_name);
			if (s.rfind(".bmp")!=string::npos)
			{
				dirlist.push_back(s);
			}

		}
		closedir(d);
		for (auto & element : dirlist) 
		{
			Image image;

			string outputPath = "/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps";
			resourcesPath = "/Users/nic/Cpp-projects/PaintMeAPicture/resources/images/bitmaps";
			outputPath.append("/");
			outputPath.append(element);
			resourcesPath.append("/");
			resourcesPath.append(element);
			image.read_bmp(resourcesPath.c_str());
			image.write_bmp(outputPath.c_str());
		}
	}
	catch (const char* msg) 
	{
     cerr << msg << endl;
  }
}

void bmp()
{

	Image image(9, 2, Image::RGBA);

	for (size_t y = 0; y < 2; y++)
	{
		for (size_t x = 0; x < 9; x++)
		{
			unsigned char *v = new unsigned char[4];
			v[0] = 0xff;
			v[1] = 0xff;
			v[2] = 0xff;
			v[3] = 0xff;
			image.set(x, y, Colour(v, image.get_bytespp()));
		}
	}
	image.write_bmp("/Users/nic/Cpp projects/PaintMeAPicture/out/im.bmp");
}

void tga()
{
	Image image(800, 800, Image::RGBA);
	Image innerImage(400, 400, Image::RGBA);

	Vector2i t0(0, 0);
	Vector2i t1(0, 400);
	Vector2i t2(400, 0);

	image.draw_triangle(t0, t1, t2, Colour(WHITE[0], WHITE[1], WHITE[2], WHITE[3]));
	innerImage.draw_triangle(t0, t1, t2, Colour(GREEN[0], GREEN[1], GREEN[2], GREEN[3]));
	image.draw_image(innerImage, 400, 400);
	image.flip_vertically();
	image.write_tga_file("/Users/nic/Cpp projects/PaintMeAPicture/out/im.tga");
}

int main()
{
	cout << "HOO HA!";
	read_then_write_bmp();
	// small8bit();
	// bmp();
}
