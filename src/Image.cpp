#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>
#include <math.h>

#include "Image.h"

using namespace Eigen;

struct ImageOutOfBoundsException : public std::exception {
   const char * what () const throw () {
      return "Inner image exceeds bounds of outer image.";
   }
};

Image::Image() : data(NULL), width(0), height(0), bytespp(0) {
}

Image::Image(int w, int h, int bpp) : data(NULL), width(w), height(h), bytespp(bpp) {
	unsigned long nbytes = width*height*bytespp;
	data = new unsigned char[nbytes];
	memset(data, 0, nbytes);
}

Image::Image(const Image &img) {
	width = img.width;
	height = img.height;
	bytespp = img.bytespp;
	unsigned long nbytes = width*height*bytespp;
	data = new unsigned char[nbytes];
	memcpy(data, img.data, nbytes);
}

Image::~Image() {
	if (data) delete [] data;
}

Image & Image::operator =(const Image &img) {
	if (this != &img) {
		if (data) delete [] data;
		width  = img.width;
		height = img.height;
		bytespp = img.bytespp;
		unsigned long nbytes = width*height*bytespp;
		data = new unsigned char[nbytes];
		memcpy(data, img.data, nbytes);
	}
	return *this;
}

bool Image::read_tga_file(const char *filename) {
	if (data) delete [] data;
	data = NULL;
	std::ifstream in;
	in.open (filename, std::ios::binary);
	if (!in.is_open()) {
		std::cerr << "can't open file " << filename << "\n";
		in.close();
		return false;
	}
	TGA_Header header;
	in.read((char *)&header, sizeof(header));
	if (!in.good()) {
		in.close();
		std::cerr << "an error occured while reading the header\n";
		return false;
	}
	width   = header.width;
	height  = header.height;
	bytespp = header.bitsperpixel>>3;
	if (width<=0 || height<=0 || (bytespp!=GRAYSCALE && bytespp!=RGB && bytespp!=RGBA)) {
		in.close();
		std::cerr << "bad bpp (or width/height) value\n";
		return false;
	}
	unsigned long nbytes = bytespp*width*height;
	data = new unsigned char[nbytes];
	if (3==header.datatypecode || 2==header.datatypecode) {
		in.read((char *)data, nbytes);
		if (!in.good()) {
			in.close();
			std::cerr << "an error occured while reading the data\n";
			return false;
		}
	} else if (10==header.datatypecode||11==header.datatypecode) {
		if (!load_rle_data(in)) {
			in.close();
			std::cerr << "an error occured while reading the data\n";
			return false;
		}
	} else {
		in.close();
		std::cerr << "unknown file format " << (int)header.datatypecode << "\n";
		return false;
	}
	if (!(header.imagedescriptor & 0x20)) {
		flip_vertically();
	}
	if (header.imagedescriptor & 0x10) {
		flip_horizontally();
	}
	std::cerr << width << "x" << height << "/" << bytespp*8 << "\n";
	in.close();
	return true;
}

bool Image::load_rle_data(std::ifstream &in) {
	unsigned long pixelcount = width*height;
	unsigned long currentpixel = 0;
	unsigned long currentbyte  = 0;
	Colour colorbuffer;
	do {
		unsigned char chunkheader = 0;
		chunkheader = in.get();
		if (!in.good()) {
			std::cerr << "an error occured while reading the data\n";
			return false;
		}
		if (chunkheader<128) {
			chunkheader++;
			for (int i=0; i<chunkheader; i++) {
				in.read((char *)colorbuffer.raw, bytespp);
				if (!in.good()) {
					std::cerr << "an error occured while reading the header\n";
					return false;
				}
				for (int t=0; t<bytespp; t++)
					data[currentbyte++] = colorbuffer.raw[t];
				currentpixel++;
				if (currentpixel>pixelcount) {
					std::cerr << "Too many pixels read\n";
					return false;
				}
			}
		} else {
			chunkheader -= 127;
			in.read((char *)colorbuffer.raw, bytespp);
			if (!in.good()) {
				std::cerr << "an error occured while reading the header\n";
				return false;
			}
			for (int i=0; i<chunkheader; i++) {
				for (int t=0; t<bytespp; t++)
					data[currentbyte++] = colorbuffer.raw[t];
				currentpixel++;
				if (currentpixel>pixelcount) {
					std::cerr << "Too many pixels read\n";
					return false;
				}
			}
		}
	} while (currentpixel < pixelcount);
	return true;
}

bool Image::write_tga_file(const char *filename, bool rle) {
	unsigned char developer_area_ref[4] = {0, 0, 0, 0};
	unsigned char extension_area_ref[4] = {0, 0, 0, 0};
	unsigned char footer[18] = {'T','R','U','E','V','I','S','I','O','N','-','X','F','I','L','E','.','\0'};
	std::ofstream out;
	out.open (filename, std::ios::binary);
	if (!out.is_open()) {
		std::cerr << "can't open file " << filename << "\n";
		out.close();
		return false;
	}
	TGA_Header header;
	memset((void *)&header, 0, sizeof(header));
	header.bitsperpixel = bytespp<<3;
	header.width  = width;
	header.height = height;
	header.datatypecode = (bytespp==GRAYSCALE?(rle?11:3):(rle?10:2));
	header.imagedescriptor = 0x20; // top-left origin
	out.write((char *)&header, sizeof(header));
	if (!out.good()) {
		out.close();
		std::cerr << "can't dump the tga file\n";
		return false;
	}
	if (!rle) {
		out.write((char *)data, width*height*bytespp);
		if (!out.good()) {
			std::cerr << "can't unload raw data\n";
			out.close();
			return false;
		}
	} else {
		if (!unload_rle_data(out)) {
			out.close();
			std::cerr << "can't unload rle data\n";
			return false;
		}
	}
	out.write((char *)developer_area_ref, sizeof(developer_area_ref));
	if (!out.good()) {
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}
	out.write((char *)extension_area_ref, sizeof(extension_area_ref));
	if (!out.good()) {
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}
	out.write((char *)footer, sizeof(footer));
	if (!out.good()) {
		std::cerr << "can't dump the tga file\n";
		out.close();
		return false;
	}
	out.close();
	return true;
}

// TODO: it is not necessary to break a raw chunk for two equal pixels (for the matter of the resulting size)
bool Image::unload_rle_data(std::ofstream &out) {
	const unsigned char max_chunk_length = 128;
	unsigned long npixels = width*height;
	unsigned long curpix = 0;
	while (curpix<npixels) {
		unsigned long chunkstart = curpix*bytespp;
		unsigned long curbyte = curpix*bytespp;
		unsigned char run_length = 1;
		bool raw = true;
		while (curpix+run_length<npixels && run_length<max_chunk_length) {
			bool succ_eq = true;
			for (int t=0; succ_eq && t<bytespp; t++) {
				succ_eq = (data[curbyte+t]==data[curbyte+t+bytespp]);
			}
			curbyte += bytespp;
			if (1==run_length) {
				raw = !succ_eq;
			}
			if (raw && succ_eq) {
				run_length--;
				break;
			}
			if (!raw && !succ_eq) {
				break;
			}
			run_length++;
		}
		curpix += run_length;
		out.put(raw?run_length-1:run_length+127);
		if (!out.good()) {
			std::cerr << "can't dump the tga file\n";
			return false;
		}
		out.write((char *)(data+chunkstart), (raw?run_length*bytespp:bytespp));
		if (!out.good()) {
			std::cerr << "can't dump the tga file\n";
			return false;
		}
	}
	return true;
}

Colour Image::get(int x, int y) {
	if (!data || x<0 || y<0 || x>=width || y>=height) {
		return Colour();
	}
	return Colour(data+(x+y*width)*bytespp, bytespp);
}

bool Image::set(int x, int y, Colour c) {
	if (!data || x<0 || y<0 || x>=width || y>=height) {
		return false;
	}
	memcpy(data+(x+y*width)*bytespp, c.raw, bytespp);
	return true;
}

int Image::get_bytespp() {
	return bytespp;
}

int Image::get_width() {
	return width;
}

int Image::get_height() {
	return height;
}

bool Image::flip_horizontally() {
	if (!data) return false;
	int half = width>>1;
	for (int i=0; i<half; i++) {
		for (int j=0; j<height; j++) {
			Colour c1 = get(i, j);
			Colour c2 = get(width-1-i, j);
			set(i, j, c2);
			set(width-1-i, j, c1);
		}
	}
	return true;
}

bool Image::flip_vertically() {
	if (!data) return false;
	unsigned long bytes_per_line = width*bytespp;
	unsigned char *line = new unsigned char[bytes_per_line];
	int half = height>>1;
	for (int j=0; j<half; j++) {
		unsigned long l1 = j*bytes_per_line;
		unsigned long l2 = (height-1-j)*bytes_per_line;
		memmove((void *)line,      (void *)(data+l1), bytes_per_line);
		memmove((void *)(data+l1), (void *)(data+l2), bytes_per_line);
		memmove((void *)(data+l2), (void *)line,      bytes_per_line);
	}
	delete [] line;
	return true;
}

unsigned char *Image::buffer() {
	return data;
}

void Image::clear() {
	memset((void *)data, 0, width*height*bytespp);
}

bool Image::scale(int w, int h) {
	if (w<=0 || h<=0 || !data) return false;
	unsigned char *tdata = new unsigned char[w*h*bytespp];
	int nscanline = 0;
	int oscanline = 0;
	int erry = 0;
	unsigned long nlinebytes = w*bytespp;
	unsigned long olinebytes = width*bytespp;
	for (int j=0; j<height; j++) {
		int errx = width-w;
		int nx   = -bytespp;
		int ox   = -bytespp;
		for (int i=0; i<width; i++) {
			ox += bytespp;
			errx += w;
			while (errx>=(int)width) {
				errx -= width;
				nx += bytespp;
				memcpy(tdata+nscanline+nx, data+oscanline+ox, bytespp);
			}
		}
		erry += h;
		oscanline += olinebytes;
		while (erry>=(int)height) {
			if (erry>=(int)height<<1) // it means we jump over a scanline
				memcpy(tdata+nscanline+nlinebytes, tdata+nscanline, nlinebytes);
			erry -= height;
			nscanline += nlinebytes;
		}
	}
	delete [] data;
	data = tdata;
	width = w;
	height = h;
	return true;
}

bool Image::draw_image(Image image, int x_anchor, int y_anchor) {
	try
	{
		if (x_anchor < 0 || y_anchor < 0 ||
				image.height + y_anchor > height || image.width + x_anchor > width)
        throw ImageOutOfBoundsException();
    
    for (int y = 0; y < image.height; y++)
    {
        for (int x = 0; x < image.width; x++)
        {
            set(x + x_anchor,y + y_anchor,image.get(x, y));
        }
    }
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}

#pragma region drawLine
bool Image::oct1(int x0, int y0, int x1, int y1, Colour colour) {
  int dx = x1 - x0;
  int dy = y1 - y0;
  int D = 2 * dy - dx;
  int y = y0;

  for (int x = x0; x < x1; ++x) {
    if (!set(x, y, colour))
        return false;
    if (D > 0) {
      y++;
      D -= 2 * dx;
    }
    D += 2 * dy;
  }
  return true;
}

bool Image::oct2(int x0, int y0, int x1, int y1, Colour colour) {
  int dx = x1 - x0;
  int dy = y1 - y0;
  int D = 2 * dx - dy;
  int x = x0;

  for (int y = y0; y < y1; ++y) {
    if (!set(x, y, colour))
        return false;
    if (D > 0) {
      x++;
      D -= 2 * (dy);
    }
    D += 2 * dx;
  }
  return true;
}

bool Image::oct3(int x0, int y0, int x1, int y1,  Colour colour) {
  int dx = x0 - x1;
  int dy = y1 - y0;
  int D = 2 * dx - dy;
  int x = x0;

  for (int y = y0; y < y1; ++y) {
    if (!set(x, y, colour))
        return false;
    if (D > 0) {
      x--;
      D -= 2 * (dy);
    }
    D += 2 * dx;
  }
  return true;
}

bool Image::oct4(int x0, int y0, int x1, int y1, Colour colour) {
  int dx = x0 - x1;
  int dy = y1 - y0;
  int D = 2 * dy - dx;
  int y = y0;

  for (int x = x0; x > x1; --x) {
    if (!set(x, y, colour))
        return false;
    if (D > 0) {
      y++;
      D -= 2 * (dx);
    }
    D += 2 * dy;
  }
  return true;
}

bool Image::oct5(int x0, int y0, int x1, int y1, Colour colour) {
  return oct1(x1, y1, x0, y0, colour);
}

bool Image::oct6(int x0, int y0, int x1, int y1, Colour colour) {
  return oct2(x1, y1, x0, y0, colour);
}

bool Image::oct7(int x0, int y0, int x1, int y1, Colour colour) {
  return oct3(x1, y1, x0, y0, colour);
}

bool Image::oct8(int x0, int y0, int x1, int y1, Colour colour) {
  return oct4(x1, y1, x0, y0, colour);
}

bool Image::draw_line(int x0, int y0, int x1, int y1, Colour colour) {
  int dy = y1 - y0;
  int dx = x1 - x0;

  // Points and lines parallel to axis
  if (dy==0)
  {
    if (dx==0)
    {
      return set(x0, y0, colour);
    }
    // printf("hline");
    if (dx>0)
    {
      int start = x0;
      int end = x1;
      for (int x = start; x < end; x++)
      {
        set(x, y0, colour);
      }
    } else {
      int start = x1;
      int end = x0;
      for (int x = start; x < end; x++)
      {
        set(x, y0, colour);
      }
    }
    return true;
  }

  if (dx==0)
  {
    // printf("vline");
    if (dy>0)
    {
      int start = y0;
      int end = y1;
      for (int y = start; y < end; y++)
      {
        set(x0, y, colour);
      }
    } else {
      int start = y1;
      int end = y0;
      for (int y = start; y < end; y++)
      {
        set(x0, y, colour);
      }
    }
    return true;
  }
  
  if ((dy<0) == (dx<0)) { // positive (downwards) slope
    if (abs(dy)> abs(dx)) { // steep slope
      if (x0<x1) oct2(x0, y0, x1, y1, colour);
      else oct6(x0, y0, x1, y1, colour);
    } else { // shallow slope
      if (x0<x1) oct1(x0, y0, x1, y1, colour);
      else oct5(x0, y0, x1, y1, colour);
    }
  } else { // negative (upwards) slope
    if (abs(dy)> abs(dx)) { // steep slope
      if (x0>x1) oct3(x0, y0, x1, y1, colour);
      else oct7(x0, y0, x1, y1, colour);
    } else { // shallow slope
      if (x0>x1) oct4(x0, y0, x1, y1, colour);
      else oct8(x0, y0, x1, y1, colour);
    }
  }
  return true;
}

#pragma endregion drawLine

bool Image::draw_line(Vector2i v0, Vector2i v1, Colour colour) {
	return draw_line(v0(0), v0(1), v1(0), v1(1), colour);
}

bool Image::draw_line2(int x0, int y0, int x1, int y1, Colour colour) { 
    bool steep = false; 
    if (std::abs(x0-x1)<std::abs(y0-y1)) { 
        std::swap(x0, y0); 
        std::swap(x1, y1); 
        steep = true; 
    } 
    if (x0>x1) { 
        std::swap(x0, x1); 
        std::swap(y0, y1); 
    } 
    int dx = x1-x0; 
    int dy = y1-y0; 
    int derror2 = std::abs(dy)*2; 
    int error2 = 0; 
    int y = y0; 
    for (int x=x0; x<=x1; x++) { 
        if (steep) { 
            set(y, x, colour); 
        } else { 
            set(x, y, colour); 
        } 
        error2 += derror2; 
        if (error2 > dx) { 
            y += (y1>y0?1:-1); 
            error2 -= dx*2; 
        } 
    } 
	return true;
}

bool Image::draw_line3(int x0, int y0, int x1, int y1, Colour colour) { 
    bool steep = false; 
    if (std::abs(x0-x1)<std::abs(y0-y1)) { 
        std::swap(x0, y0); 
        std::swap(x1, y1); 
        steep = true; 
    } 
    if (x0>x1) { 
        std::swap(x0, x1); 
        std::swap(y0, y1); 
    } 
    int dx = x1-x0; 
    int dy = y1-y0; 
    int derror2 = std::abs(dy)*2; 
    int error2 = 0; 
    int y = y0; 
    if(steep) {
        for(int x = x0; x<=x1; ++x) {
            set(y, x, colour);
            error2 += derror2;
            if(error2 > dx) {
                y += (y1>y0? 1 : -1);
                error2 -= dx*2;
            }
        }
    } else {
        for(int x = x0; x<=x1; ++x) {
            set(x, y, colour);
            error2 += derror2;
            if(error2 > dx) {
                y += (y1>y0? 1 : -1);
                error2 -= dx*2;
            }
        }
    }
	return true;
}

bool Image::draw_triangle(Vector2i t0, Vector2i t1, Vector2i t2, Colour colour) {
	if (t0(1)==t1(1) && t0(1)==t2(1)) return true;

	if (t0(1)>t1(1)) std::swap(t0, t1); 
    if (t0(1)>t2(1)) std::swap(t0, t2); 
    if (t1(1)>t2(1)) std::swap(t1, t2); 

	double m01 = double (t1(1)-t0(1))/(t1(0)-t0(0));
	double m02 = double (t2(1)-t0(1))/(t2(0)-t0(0));
	double m12 = double (t2(1)-t1(1))/(t2(0)-t1(0));

	for (size_t y = t0(1); y < t1(1); y++)
	{
		int xi = y/m02 - t0(1)/m02 + t0(0);
		int xf = y/m01 - t0(1)/m01 + t0(0);

		if (xi > xf) std::swap(xi, xf);
		
		draw_line(xi, y,
		 xf+1, y, colour);
	}
	for (size_t y = t1(1); y < t2(1); y++)
	{
		int xi = y/m02 - t0(1)/m02 + t0(0);
		int xf = y/m12 - t1(1)/m12 + t1(0);

		if (xi > xf) std::swap(xi, xf);
		
		draw_line(xi, y,
		 xf+1, y, colour);
	}
	
	return true;
}

// bool Image::draw_triangle2(Vector2f t0, Vector2f t1, Vector2f t2, Colour colour) { 
//     if (t0(1)==t1(1) && t0(1)==t2(1)) return true; 

//     if (t0(1)>t1(1)) std::swap(t0, t1); 
//     if (t0(1)>t2(1)) std::swap(t0, t2); 
//     if (t1(1)>t2(1)) std::swap(t1, t2); 

//     int total_height = t2(1)-t0(1); 
//     for (int i=0; i<total_height; i++) { 
//         bool second_half = i>t1(1)-t0(1) || t1(1)==t0(1); 
//         int segment_height = second_half ? t2(1)-t1(1) : t1(1)-t0(1); 
//         float alpha = (float)i/total_height; 
//         float beta  = (float)(i-(second_half ? t1(1)-t0(1) : 0))/segment_height; // be careful: with above conditions no division by zero here 

//         Vector2f A =               t0 + (t2-t0)*alpha; 
//         Vector2f B = second_half ? t1 + (t2-t1)*beta : t0 + (t1-t0)*beta; 
//         if (A(0)>B(0)) std::swap(A, B); 
//         for (int j=A(0); j<=B(0); j++) { 
//             set(j, t0(1)+i, colour); // attention, due to int casts t0.y+i != A.y 
//         } 
//     } 
// 	return true;
// }