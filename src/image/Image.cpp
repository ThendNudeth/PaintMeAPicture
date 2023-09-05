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

Image::Image() : data(NULL), width(0), height(0), bytespp(0)
{
}

Image::Image(int w, int h, int bpp) : data(NULL), width(w), height(h), bytespp(bpp)
{
	unsigned long nbytes = width * height * bytespp;
	data = new uint8_t[nbytes];
	memset(data, 0, nbytes);
}

Image::Image(const Image &img)
{
	width = img.width;
	height = img.height;
	bytespp = img.bytespp;
	unsigned long nbytes = width * height * bytespp;
	data = new uint8_t[nbytes];
	memcpy(data, img.data, nbytes);
}

Image::~Image()
{
	if (data)
		delete[] data;
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
			// table_size = (1 << (bytespp*8)) * RGBAQUAD;
			fileSize = ((width * bytespp) + paddingCnt) * height + BMP_HEADER_SIZE + palette.size;
			header = BMPHeader(width, height, bytespp, fileSize, palette.size,image_size);
			// header.palette = (uint8_t *)malloc(table_size);
			// for (size_t i = 0; i < table_size/RGBAQUAD; i++)
			// {
			// 	header.palette[4*i] = (uint8_t) i;
			// 	header.palette[(4*i)+1] = (uint8_t) i;
			// 	header.palette[(4*i)+2] = (uint8_t) i;
			// 	header.palette[(4*i)+3] = 0;

			// 	std::cout << header.palette[4*i] << std::endl;
			// }
			
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
			if (fwrite(&data[i*width*bytespp], width*bytespp*sizeof(unsigned char), 1, fp)<0)
				throw "Could not write data to file";
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

		// if (nbytes!=header.image_size_bytes)
		// 	throw "nbytes doesn't match header.image_size_bytes";
		
		data = new uint8_t[nbytes];

		for (int i = height - 1; i >= 0; i--)
		{
			if (fread(&data[i * scanline_len], scanline_len * sizeof(uint8_t), 1, fp)!=1)
				throw "Could not read data from file";
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

int Image::get_bytespp()
{
	return bytespp;
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