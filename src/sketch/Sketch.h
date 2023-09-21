#include "Image.h"

static constexpr unsigned char WHITE[4] = {255, 255, 255, 255};
static constexpr unsigned char BLACK[4] = {0, 0, 0, 255};
static constexpr unsigned char L_GRAY[4] = {85, 85, 85, 255};
static constexpr unsigned char D_GRAY[4] = {170, 170, 170, 255};
static constexpr unsigned char RED[4] = {255, 0, 0, 255};
static constexpr unsigned char GREEN[4] = {0, 255, 0, 255};
static constexpr unsigned char BLUE[4] = {0, 0, 255, 255};
// static constexpr unsigned char ORANGE[4] = {0, 0, 255, 255};

struct Colour
{
	union
	{
		struct
		{
			uint8_t r, g, b, a;
		};
		uint8_t raw[4];
		uint32_t val;
	};
	int bytespp;

	// Construct by value
	Colour() : val(0), bytespp(1)
	{
	}

	Colour(uint8_t R, uint8_t G, uint8_t B, uint8_t A) : r(R), g(G), b(B), a(A), bytespp(4)
	{
	}

	Colour(uint32_t v, uint32_t bpp) : val(v), bytespp(bpp)
	{
	}

	Colour(const Colour &c) : val(c.val), bytespp(c.bytespp)
	{
	}

	Colour(const uint8_t *p, int bpp) : val(0), bytespp(bpp)
	{
		for (int i = 0; i < bpp; i++)
		{
			raw[i] = p[i];
		}
	}

	Colour &operator=(const Colour &c)
	{
		if (this != &c)
		{
			bytespp = c.bytespp;
			val = c.val;
		}
		return *this;
	}
};

class Sketch : public Image
{
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
  using Image::Image;

  bool flip_horizontally();
	bool flip_vertically();
	bool scale(int w, int h);

  bool draw_line(int x0, int y0, int x1, int y1, Colour colour);
	bool draw_line(Vector2i v0, Vector2i v1, Colour colour);
	bool draw_line2(int x0, int y0, int x1, int y1, Colour colour);
	bool draw_line3(int x0, int y0, int x1, int y1, Colour colour);

	bool draw_triangle(Vector2i t0, Vector2i t1, Vector2i t2, Colour colour);

  bool draw_image(Sketch sketch, int x_anchor, int y_anchor);

	Colour get(int x, int y);
	bool set(int x, int y, Colour c);
};