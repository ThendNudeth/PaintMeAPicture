#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <fstream>
#include <Eigen/Dense>

using namespace Eigen;

#pragma pack(push,1)
struct TGA_Header {
	char idlength;
	char colormaptype;
	char datatypecode;
	short colormaporigin;
	short colormaplength;
	char colormapdepth;
	short x_origin;
	short y_origin;
	short width;
	short height;
	char  bitsperpixel;
	char  imagedescriptor;
};
#pragma pack(pop)

static constexpr unsigned char WHITE[3] = {255, 255, 255};
static constexpr unsigned char BLACK[3] = {	 0,   0,   0};
static constexpr unsigned char   RED[3] = {255,   0,   0};
static constexpr unsigned char GREEN[3] = {  0, 255,   0};
static constexpr unsigned char  BLUE[3] = {  0,   0, 255};

struct Colour {
	union {
		struct {
			unsigned char r, g, b, a;
		};
		unsigned char raw[4];
		unsigned int val;
	};
	int bytespp;

    // Construct by value
	Colour() : val(0), bytespp(1) {
	}

	Colour(unsigned char R, unsigned char G, unsigned char B, unsigned char A) : b(B), g(G), r(R), a(A), bytespp(4) {
	}

	Colour(int v, int bpp) : val(v), bytespp(bpp) {
	}

	Colour(const Colour &c) : val(c.val), bytespp(c.bytespp) {
	}

	Colour(const unsigned char *p, int bpp) : val(0), bytespp(bpp) {
		for (int i=0; i<bpp; i++) {
			raw[i] = p[i];
		}
	}

	Colour & operator =(const Colour &c) {
		if (this != &c) {
			bytespp = c.bytespp;
			val = c.val;
		}
		return *this;
	}
};




class Image {
protected:
	unsigned char* data;
	int width;
	int height;
	int bytespp;

	bool   load_rle_data(std::ifstream &in);
	bool unload_rle_data(std::ofstream &out);

private:
    bool oct1(int x0, int y0, int x1, int y1, Colour colour);
    bool oct2(int x0, int y0, int x1, int y1, Colour colour);
    bool oct3(int x0, int y0, int x1, int y1, Colour colour);
    bool oct4(int x0, int y0, int x1, int y1, Colour colour);
    bool oct5(int x0, int y0, int x1, int y1, Colour colour);
    bool oct6(int x0, int y0, int x1, int y1, Colour colour);
    bool oct7(int x0, int y0, int x1, int y1, Colour colour);
    bool oct8(int x0, int y0, int x1, int y1, Colour colour);

public:
	enum Format {
		GRAYSCALE=1, RGB=3, RGBA=4
	};

	Image();
	Image(int w, int h, int bpp);
	Image(const Image &img);

	bool read_tga_file(const char *filename);
	bool write_tga_file(const char *filename, bool rle=true);

	bool flip_horizontally();
	bool flip_vertically();
	bool scale(int w, int h);

	bool draw_image(Image image, int x_anchor, int y_anchor);

    bool draw_line(int x0, int y0, int x1, int y1, Colour colour);
	bool draw_line(Vector2i v0, Vector2i v1, Colour colour);
	bool draw_line2(int x0, int y0, int x1, int y1, Colour colour);
	bool draw_line3(int x0, int y0, int x1, int y1, Colour colour);

	bool draw_triangle(Vector2i t0, Vector2i t1, Vector2i t2, Colour colour);
	bool draw_triangle2(Vector2f t0, Vector2f t1, Vector2f t2, Colour colour);

	Colour get(int x, int y);
	bool set(int x, int y, Colour c);

	~Image();
	Image & operator =(const Image &img);
	int get_width();
	int get_height();
	int get_bytespp();
	unsigned char *buffer();
	void clear();
};

#endif //__IMAGE_H__