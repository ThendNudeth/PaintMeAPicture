#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <fstream>
#include <Eigen/Dense>
#include <iostream>

using namespace Eigen;

#define MAGIC_VALUE         0x4D42
#define BMP_HEADER_SIZE			54
#define BMP_INFH_SIZE				40
#define BMP_FILEH_SIZE			14
#define NUM_PLANE           1
#define COMPRESSION         0
#define NUM_COLORS          256
#define IMPORTANT_COLORS    0
#define BITS_PER_BYTE       8
#define RESERVED						0
#define RGBAQUAD						4

struct BMPHeader
{
	#pragma pack(push, 1)
	struct FileHeader
	{
		uint16_t signature;				// Magic identifier: 0x4d42
		uint32_t file_size;				// File size in bytes
		uint16_t reserved1;				// Not used
		uint16_t reserved2;				// Not used
		uint32_t data_offset;			// Offset to image data in bytes from beginning of file (54 bytes)

		FileHeader() :
		signature(MAGIC_VALUE), reserved1(RESERVED), reserved2(RESERVED)
		{
		}

		FileHeader(uint32_t file_size) : 
		signature(MAGIC_VALUE), file_size(file_size), reserved1(RESERVED), reserved2(RESERVED), data_offset(BMP_HEADER_SIZE)
		{
		}

		FileHeader(uint32_t file_size, uint32_t data_offset) : 
		signature(MAGIC_VALUE), file_size(file_size), reserved1(RESERVED), reserved2(RESERVED), data_offset(data_offset)
		{
		}
	};
	#pragma pack(pop)

	#pragma pack(push, 1)
	struct InfoHeader
	{
		uint32_t info_header_size;	// DIB Header size in bytes (40 bytes)
		uint32_t width_px;					// Width of the image
		uint32_t height_px;					// Height of image
		uint16_t num_planes;				// Number of color planes
		uint16_t bits_per_pixel;		// Bits per pixel
		uint32_t compression;				// Compression type
		uint32_t image_size_bytes; 	// Image size in bytes
		uint32_t x_resolution_ppm; 	// Pixels per meter
		uint32_t y_resolution_ppm; 	// Pixels per meter
		uint32_t num_colors;				// Number of colors
		uint32_t important_colors; 	// Important colors

		InfoHeader()
		{
		}

		InfoHeader(uint32_t width, uint32_t height, uint16_t bytespp, uint32_t image_size) :
		info_header_size(BMP_INFH_SIZE), width_px(width), height_px(height), num_planes(NUM_PLANE),
		bits_per_pixel(bytespp*BITS_PER_BYTE), compression(COMPRESSION),
		image_size_bytes(image_size), x_resolution_ppm(0x00),
		y_resolution_ppm(0x00), num_colors(NUM_COLORS), important_colors(IMPORTANT_COLORS)
		{
		}
	};
	#pragma pack(pop)

	FileHeader fileHeader;
	InfoHeader infoHeader;
	
	int paletteSz;
	#pragma pack(push, 1)
	uint8_t* palette;
	#pragma pack(pop)

	BMPHeader()
	{
	}

	BMPHeader(uint32_t width, uint32_t height, uint16_t bytespp, uint32_t fileSize, uint32_t image_size) :
	fileHeader(fileSize), infoHeader(width, height, bytespp, image_size)
	, paletteSz(0)
	{
	}

	BMPHeader(uint32_t width, uint32_t height, uint16_t bytespp, uint32_t fileSize, int table_size, uint32_t image_size) :
	fileHeader(fileSize, BMP_HEADER_SIZE + table_size), infoHeader(width, height, bytespp, image_size)
	, paletteSz(table_size)
	{
		palette = new uint8_t[table_size];
	}

	void printPalette() {
		for (size_t i = 0; i < paletteSz; i++)
		{
			std::cout << palette[i] <<std::endl;
		}
	}
};


class Image
{
	#pragma pack(push, 1)
	struct Palette
	{
		int size;
		uint8_t* data;

		Palette() : size(0), data(NULL)
		{
		}

		Palette(int size) : size(size)
		{
			data = new uint8_t[size];
		}
	};
	#pragma pack(pop)

protected:
	uint8_t* data;
	int width;
	int height;
	int bytespp;
	Palette palette;

public:
	enum Format
	{
		GRAYSCALE = 1,
		RGB = 3,
		RGBA = 4
	};

	Image();
	Image(int w, int h, int bpp);
	Image(const Image &img);

	void read_bmp(const char *filename);
	void write_bmp(const char *filename, bool improvise_palette = false);

	void to_rgb();
	void to_grayscale();

	~Image();
	Image &operator=(const Image &img);
	int get_width();
	int get_height();
	int get_bytespp();
	uint8_t *buffer();
	void clear();
};

#endif //__IMAGE_H__