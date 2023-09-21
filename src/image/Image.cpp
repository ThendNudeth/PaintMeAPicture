#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

#include "Image.h"

using namespace Eigen;

struct ImageOutOfBoundsException : public std::exception
{
	const char *what() const throw()
	{
		return "Inner image exceeds bounds of outer image.";
	}
};


Image::Image() : data(NULL), width(0), height(0), bytespp(0), palette(0)
{
}

Image::Image(int h, int w, int bpp) : data(NULL), width(w), height(h), bytespp(bpp), palette(0)
{
	uint64_t nbytes = width * height * bytespp;
	data = new uint8_t[nbytes];
	memset(data, 0, nbytes);
	if (bpp == 1)
	{
		set_Palette(BIT8);
	}
	
}

Image::Image(const Image &img)
{
	width = img.width;
	height = img.height;
	bytespp = img.bytespp;
	uint64_t nbytes = width * height * bytespp;
	data = new uint8_t[nbytes];
	memcpy(data, img.data, nbytes);
	palette = img.palette;
}

Image::~Image()
{
	if (data)
		delete[] data;
	
	if (palette.size>0)
		delete[] palette.data;
		
}


Image &Image::operator=(const Image &img)
{
	if (this != &img)
	{
		if (data)
			delete[] data;
		width = img.width;
		height = img.height;
		bytespp = img.bytespp;
		unsigned long nbytes = width * height * bytespp;
		data = new uint8_t[nbytes];
		memcpy(data, img.data, nbytes);
	}
	return *this;
}

void Image::printData()
{
	size_t i = 0;
	for (size_t y = 0; y < height; y++)
	{
		for (size_t x = 0; x < width; x++)
		{			
			printf("%zu: (%zu, %zu) [%d, %d, %d]\n", i, y, x, data[i], data[i+1], data[i+2]);
			i+=3;
		}
	}
}

void Image::write_bmp(const char *filename, bool improvise_palette)
{
	try
	{
		FILE* fp;
		fp = fopen(filename, "w");
		if (fp == NULL)
		{
			throw "Could not open file";
		}
		
		size_t multipleOf4Check = width * bytespp % 4;
		size_t paddingCnt = (multipleOf4Check)==0 ? 0 : 4 - multipleOf4Check;
		uint8_t padding[4] = {0x00, 0x00, 0x00, 0x00};

		size_t fileSize;
		BMPHeader header;
		// int table_size = 0;
		uint32_t image_size = (width * bytespp + paddingCnt) * height;

		if (bytespp > 1)
		{
			fileSize = ((width * bytespp) + paddingCnt) * height + BMP_HEADER_SIZE;
			header = BMPHeader(width, height, bytespp, fileSize, image_size);
		}
		else 
		{
			fileSize = ((width * bytespp) + paddingCnt) * height + BMP_HEADER_SIZE + palette.size;
			header = BMPHeader(width, height, bytespp, fileSize, palette.size,image_size);
			
		}

		if (fwrite(&header.fileHeader, BMP_FILEH_SIZE, 1, fp)!=1)
			throw "Could not write data to file";

		if (fwrite(&header.infoHeader, BMP_INFH_SIZE, 1, fp)!=1)
			throw "Could not write data to file";

		if (palette.size>0)
			if (fwrite(palette.data, palette.size, 1, fp)!=1)
				throw "Could not write data to file";


		for (int i = height - 1; i >= 0; i--)
		{
			int y_pos = i*width*bytespp;
			for (size_t j = 0; j < width; j++)
			{
				int x_pos = j*bytespp;
				for (int k = bytespp - 1; k >= 0; k--)
				{
					// printf("w: [%d] (%d, %zu, %d) %d\n", y_pos + x_pos + k, i, j, k, data[y_pos + x_pos + k]);
					if (fwrite(&data[y_pos + x_pos + k], sizeof(uint8_t), 1, fp)<0)
						throw "Could not write data to file";
				}
				
			}
			if (fwrite(&padding, paddingCnt, 1, fp)<0)
				throw "Could not write data to file";
		}
		fclose(fp);
	}
	catch (const char* msg) 
	{
    std::cerr << msg << std::endl;
  }
}

void Image::read_bmp(const char *filename)
{
	try
	{
		FILE* fp = fopen(filename, "r");
		if (fp==NULL)
			throw "Could not open file";

		BMPHeader header;
		uint32_t data_offset;

		fseek(fp, 10, SEEK_SET);

		if (fread(&data_offset, 4, 1, fp)!=1)
			throw "Could not read data from file";

		rewind(fp);

		if (fread(&header.fileHeader, BMP_FILEH_SIZE, 1, fp)!=1)
			throw "Could not read data from file";
		
		if (fread(&header.infoHeader, BMP_INFH_SIZE, 1, fp)!=1)
			throw "Could not read data from file";

		if (header.infoHeader.bits_per_pixel<=8)
		{
			if (palette.size==0)
			{
				palette.size = (1 << header.infoHeader.bits_per_pixel) * RGBAQUAD;
				palette.data = new uint8_t[palette.size];
			}
			else 
			{
				palette.size = (1 << header.infoHeader.bits_per_pixel) * RGBAQUAD;
			}
			
			if (fread(&palette.data[0], palette.size, 1, fp)!=1)
				throw "Could not read data from file";
		}

		fseek(fp, data_offset, SEEK_SET);

		height = header.infoHeader.height_px;
		width = header.infoHeader.width_px;
		bytespp = header.infoHeader.bits_per_pixel/BITS_PER_BYTE;
		
		uint32_t scanline_len = width * bytespp;
		uint32_t nbytes = height * scanline_len;
		uint32_t padding = 4 - (scanline_len % 4);
		
		data = new uint8_t[nbytes];

		for (int i = height - 1; i >= 0; i--)
		{
			int y_pos = i*width*bytespp;
			for (size_t j = 0; j < width; j++)
			{
				int x_pos = j*bytespp;
				for (int k = bytespp - 1; k >= 0; k--)
				{
					if (fread(&data[y_pos + x_pos + k], sizeof(uint8_t), 1, fp)!=1)
						throw "Could not read data from file";
				}
			}
			if (padding!=4)
				fseek(fp, 4 - (scanline_len % 4), SEEK_CUR);
		}
		fclose(fp);
	}
	catch (const char* msg) 
	{
    std::cerr << msg << std::endl;
  }
}

void Image::to_rgb()
{
	if (bytespp<1)
		throw "This method does not support Sub-8-bit images.";

	if (bytespp>4)
		throw "This method does not support Super-32-bit images.";

	if (bytespp==2)
		throw "Not yet supported.";

	uint8_t* newData = new uint8_t[width*height*RGB];

	if (bytespp==1)
		if (palette.size==0)
		{
			int nbytes = width*height;

			for (size_t i = 0; i < nbytes; i++)
			{
				uint8_t px = data[i];
				
				newData[i * RGB		 ] = palette.data[px * RGBA];
				newData[i * RGB + 1] = palette.data[px * RGBA + 1];
				newData[i * RGB + 2] = palette.data[px * RGBA + 2];
			}

			delete[] data;
			data = newData;
			bytespp = 3;
			delete[] palette.data;
			palette.size = 0;
		}

	if (bytespp==4)
	{
		
	}

}


void Image::to_rgba()
{

}

void Image::to_grayscale()
{

}

int Image::get_bytespp()
{
	return bytespp;
}

void Image::set_Palette(PaletteDefault p)
{
	if (p == PaletteDefault::BIT8)
	{
		if (palette.size == 0)
		{
			palette.size = NUM_COLORS*RGBA;
			palette.data = new uint8_t[palette.size];
		}
			
		for (size_t i = 0; i < NUM_COLORS; i++)
		{
			palette.data[4 * i] = i;
			palette.data[4 * i + 1] = i;
			palette.data[4 * i + 2] = i;
			palette.data[4 * i + 3] = 0x00;
		}
		
	}
}

int Image::get_width()
{
	return width;
}

int Image::get_height()
{
	return height;
}

uint8_t* Image::buffer()
{
	return data;
}

void Image::clear()
{
	memset((void *)data, 0, width * height * bytespp);
}