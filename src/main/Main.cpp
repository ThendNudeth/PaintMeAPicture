#include <iostream>
#include <vector>
#include <string>
#include <dirent.h>

#include "Sketch.h"

using namespace std;

void small8bit() {
	Sketch bit8(16,16,1);
	for (size_t i = 0; i < 16; i++)
	{
		for (size_t j = 0; j < 16; j++)
		{
			bit8.set(i, j, Colour(i*j, 1));
		}
	}
	// bit8.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/8bit_p.bmp", true);
	bit8.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/8bit.bmp");
	bit8.to_rgb();
	bit8.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/8bit24bit.bmp");
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
			
			resourcesPath.append("/");
			resourcesPath.append(element);

			image.read_bmp(resourcesPath.c_str());
			
			if (image.get_bytespp()==1)
			{
				string outputPath2 = outputPath;

				outputPath2.append("MOD_");
				outputPath.append(element);
				outputPath2.append(element);

				image.write_bmp(outputPath.c_str());
				image.to_rgb();
				image.write_bmp(outputPath2.c_str());

			}
			else 
			{
				outputPath.append(element);
				image.write_bmp(outputPath.c_str());
			}
		}
	}
	catch (const char* msg) 
	{
     cerr << msg << endl;
  }
}

void drawPic()
{
	Sketch sketch8bit(8,8,1);
	sketch8bit.clear();
	
	// sketch.draw_line(0,0,7,7, Colour(WHITE, 1));
	sketch8bit.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/black_sketch_8bit.bmp");
	for (size_t i = 0; i < sketch8bit.get_height(); i++)
	{
		sketch8bit.draw_line(0, i, sketch8bit.get_width(), i, Colour(WHITE, 1));
	}
	sketch8bit.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/white_sketch_8bit.bmp");
	sketch8bit.clear();
	sketch8bit.draw_line(0, 0, 8, 8, Colour(WHITE, 1));
	sketch8bit.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/line_sketch_8bit.bmp");

	Sketch inner8bit(4,4,1);
	inner8bit.clear();
	for (size_t i = 0; i < inner8bit.get_height(); i++)
	{
		inner8bit.draw_line(0, i, inner8bit.get_width(), i, Colour(L_GRAY, 1));
	}
	sketch8bit.draw_image(inner8bit, 2, 2);
	sketch8bit.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/inner_sketch_8bit.bmp");

	Sketch sketch24bit(8,8,3);
	sketch24bit.clear();
	
	sketch24bit.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/black_sketch_24bit.bmp");
	for (size_t i = 0; i < sketch24bit.get_height(); i++)
	{
		sketch24bit.draw_line(0, i, sketch24bit.get_width(), i, Colour(BLUE, 3));
	}
	sketch24bit.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/blue_sketch_24bit.bmp");
	sketch24bit.clear();
	sketch24bit.draw_line(0, 0, 8, 8, Colour(WHITE, 3));
	sketch24bit.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/line_sketch_24bit.bmp");
	
	Sketch inner24bit(4,4,3);
	inner24bit.clear();
	for (size_t i = 0; i < inner24bit.get_height(); i++)
	{
		inner24bit.draw_line(0, i, inner24bit.get_width(), i, Colour(BLUE, 3));
	}
	sketch24bit.draw_image(inner24bit, 2, 2);
	sketch24bit.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/inner_sketch_24bit.bmp");
}

void uniPix()
{
	Sketch sketch24bit(1,1,3);
	sketch24bit.clear();
	
	sketch24bit.draw_line(0,0,1,1, Colour(RED, 3));
	sketch24bit.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/red_pixel.bmp");
	sketch24bit.draw_line(0,0,1,1, Colour(GREEN, 3));
	sketch24bit.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/green_pixel.bmp");
	sketch24bit.draw_line(0,0,1,1, Colour(BLUE, 3));
	sketch24bit.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/blue_pixel.bmp");
}

void twoPix()
{
	Sketch sketch24bit(1,2,3);
	sketch24bit.clear();
	
	sketch24bit.draw_line(0,0,2,0, Colour(RED, 3));
	sketch24bit.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/red_l2.bmp");
	sketch24bit.draw_line(0,0,2,0, Colour(GREEN, 3));
	sketch24bit.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/green_l2.bmp");
	sketch24bit.draw_line(0,0,2,0, Colour(BLUE, 3));
	sketch24bit.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/blue_l2.bmp");
}

void treePix()
{
	Sketch sketch24bit(3,3,3);
	sketch24bit.clear();
	
	sketch24bit.draw_line(0,0,3,0, Colour(RED, 3));
	// sketch24bit.printData();
	sketch24bit.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/red_l3.bmp");
	sketch24bit.draw_line(0,0,3,0, Colour(GREEN, 3));
	sketch24bit.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/green_l3.bmp");
	sketch24bit.draw_line(0,0,3,0, Colour(BLUE, 3));
	sketch24bit.write_bmp("/Users/nic/Cpp-projects/PaintMeAPicture/out/images/bitmaps/blue_l3.bmp");
}


int main()
{
	cout << "HOO HA!\n";
	treePix();
	read_then_write_bmp();
	drawPic();
}
