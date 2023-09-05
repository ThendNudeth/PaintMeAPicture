#include "Sketch.h"

// Sketch::Sketch(/* args */)
// {
// }

// Sketch::~Sketch()
// {
// }

Colour Sketch::get(int x, int y)
{
	if (!data || x < 0 || y < 0 || x >= width || y >= height)
	{
		return Colour();
	}
	return Colour(data + (x + y * width) * bytespp, bytespp);
}

bool Sketch::set(int x, int y, Colour c)
{
	if (!data || x < 0 || y < 0 || x >= width || y >= height)
	{
		return false;
	}
	memcpy(data + (x + y * width) * bytespp, c.raw, bytespp);
	return true;
}

bool Sketch::flip_horizontally()
{
	if (!data)
		return false;
	int half = width >> 1;
	for (int i = 0; i < half; i++)
	{
		for (int j = 0; j < height; j++)
		{
			Colour c1 = get(i, j);
			Colour c2 = get(width - 1 - i, j);
			set(i, j, c2);
			set(width - 1 - i, j, c1);
		}
	}
	return true;
}

bool Sketch::flip_vertically()
{
	if (!data)
		return false;
	unsigned long bytes_per_line = width * bytespp;
	unsigned char *line = new unsigned char[bytes_per_line];
	int half = height >> 1;
	for (int j = 0; j < half; j++)
	{
		unsigned long l1 = j * bytes_per_line;
		unsigned long l2 = (height - 1 - j) * bytes_per_line;
		memmove((void *)line, (void *)(data + l1), bytes_per_line);
		memmove((void *)(data + l1), (void *)(data + l2), bytes_per_line);
		memmove((void *)(data + l2), (void *)line, bytes_per_line);
	}
	delete[] line;
	return true;
}

bool Sketch::scale(int w, int h)
{
	if (w <= 0 || h <= 0 || !data)
		return false;
	unsigned char *tdata = new unsigned char[w * h * bytespp];
	int nscanline = 0;
	int oscanline = 0;
	int erry = 0;
	unsigned long nlinebytes = w * bytespp;
	unsigned long olinebytes = width * bytespp;
	for (int j = 0; j < height; j++)
	{
		int errx = width - w;
		int nx = -bytespp;
		int ox = -bytespp;
		for (int i = 0; i < width; i++)
		{
			ox += bytespp;
			errx += w;
			while (errx >= (int)width)
			{
				errx -= width;
				nx += bytespp;
				memcpy(tdata + nscanline + nx, data + oscanline + ox, bytespp);
			}
		}
		erry += h;
		oscanline += olinebytes;
		while (erry >= (int)height)
		{
			if (erry >= (int)height << 1) // it means we jump over a scanline
				memcpy(tdata + nscanline + nlinebytes, tdata + nscanline, nlinebytes);
			erry -= height;
			nscanline += nlinebytes;
		}
	}
	delete[] data;
	data = tdata;
	width = w;
	height = h;
	return true;
}

bool Sketch::draw_image(Sketch sketch, int x_anchor, int y_anchor)
{
	try
	{
		if (x_anchor < 0 || y_anchor < 0 ||
			  sketch.get_height() + y_anchor > this->get_height() || 
        sketch.get_width() + x_anchor > this->get_width())
			throw "ImageOutOfBoundsException()";

		for (int y = 0; y < sketch.get_height(); y++)
		{
			for (int x = 0; x < sketch.get_width(); x++)
			{
				set(x + x_anchor, y + y_anchor, sketch.get(x, y));
			}
		}
		return true;
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return false;
	}
}

#pragma region drawLine
bool Sketch::oct1(int x0, int y0, int x1, int y1, Colour colour)
{
	int dx = x1 - x0;
	int dy = y1 - y0;
	int D = 2 * dy - dx;
	int y = y0;

	for (int x = x0; x < x1; ++x)
	{
		if (!set(x, y, colour))
			return false;
		if (D > 0)
		{
			y++;
			D -= 2 * dx;
		}
		D += 2 * dy;
	}
	return true;
}

bool Sketch::oct2(int x0, int y0, int x1, int y1, Colour colour)
{
	int dx = x1 - x0;
	int dy = y1 - y0;
	int D = 2 * dx - dy;
	int x = x0;

	for (int y = y0; y < y1; ++y)
	{
		if (!set(x, y, colour))
			return false;
		if (D > 0)
		{
			x++;
			D -= 2 * (dy);
		}
		D += 2 * dx;
	}
	return true;
}

bool Sketch::oct3(int x0, int y0, int x1, int y1, Colour colour)
{
	int dx = x0 - x1;
	int dy = y1 - y0;
	int D = 2 * dx - dy;
	int x = x0;

	for (int y = y0; y < y1; ++y)
	{
		if (!set(x, y, colour))
			return false;
		if (D > 0)
		{
			x--;
			D -= 2 * (dy);
		}
		D += 2 * dx;
	}
	return true;
}

bool Sketch::oct4(int x0, int y0, int x1, int y1, Colour colour)
{
	int dx = x0 - x1;
	int dy = y1 - y0;
	int D = 2 * dy - dx;
	int y = y0;

	for (int x = x0; x > x1; --x)
	{
		if (!set(x, y, colour))
			return false;
		if (D > 0)
		{
			y++;
			D -= 2 * (dx);
		}
		D += 2 * dy;
	}
	return true;
}

bool Sketch::oct5(int x0, int y0, int x1, int y1, Colour colour)
{
	return oct1(x1, y1, x0, y0, colour);
}

bool Sketch::oct6(int x0, int y0, int x1, int y1, Colour colour)
{
	return oct2(x1, y1, x0, y0, colour);
}

bool Sketch::oct7(int x0, int y0, int x1, int y1, Colour colour)
{
	return oct3(x1, y1, x0, y0, colour);
}

bool Sketch::oct8(int x0, int y0, int x1, int y1, Colour colour)
{
	return oct4(x1, y1, x0, y0, colour);
}

bool Sketch::draw_line(int x0, int y0, int x1, int y1, Colour colour)
{
	int dy = y1 - y0;
	int dx = x1 - x0;

	// Points and lines parallel to axis
	if (dy == 0)
	{
		if (dx == 0)
		{
			return set(x0, y0, colour);
		}
		// printf("hline");
		if (dx > 0)
		{
			int start = x0;
			int end = x1;
			for (int x = start; x < end; x++)
			{
				set(x, y0, colour);
			}
		}
		else
		{
			int start = x1;
			int end = x0;
			for (int x = start; x < end; x++)
			{
				set(x, y0, colour);
			}
		}
		return true;
	}

	if (dx == 0)
	{
		// printf("vline");
		if (dy > 0)
		{
			int start = y0;
			int end = y1;
			for (int y = start; y < end; y++)
			{
				set(x0, y, colour);
			}
		}
		else
		{
			int start = y1;
			int end = y0;
			for (int y = start; y < end; y++)
			{
				set(x0, y, colour);
			}
		}
		return true;
	}

	if ((dy < 0) == (dx < 0))
	{ // positive (downwards) slope
		if (abs(dy) > abs(dx))
		{ // steep slope
			if (x0 < x1)
				oct2(x0, y0, x1, y1, colour);
			else
				oct6(x0, y0, x1, y1, colour);
		}
		else
		{ // shallow slope
			if (x0 < x1)
				oct1(x0, y0, x1, y1, colour);
			else
				oct5(x0, y0, x1, y1, colour);
		}
	}
	else
	{ // negative (upwards) slope
		if (abs(dy) > abs(dx))
		{ // steep slope
			if (x0 > x1)
				oct3(x0, y0, x1, y1, colour);
			else
				oct7(x0, y0, x1, y1, colour);
		}
		else
		{ // shallow slope
			if (x0 > x1)
				oct4(x0, y0, x1, y1, colour);
			else
				oct8(x0, y0, x1, y1, colour);
		}
	}
	return true;
}

#pragma endregion drawLine

bool Sketch::draw_line(Vector2i v0, Vector2i v1, Colour colour)
{
	return draw_line(v0(0), v0(1), v1(0), v1(1), colour);
}

bool Sketch::draw_line2(int x0, int y0, int x1, int y1, Colour colour)
{
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1))
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = y0;
	for (int x = x0; x <= x1; x++)
	{
		if (steep)
		{
			set(y, x, colour);
		}
		else
		{
			set(x, y, colour);
		}
		error2 += derror2;
		if (error2 > dx)
		{
			y += (y1 > y0 ? 1 : -1);
			error2 -= dx * 2;
		}
	}
	return true;
}

bool Sketch::draw_line3(int x0, int y0, int x1, int y1, Colour colour)
{
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1))
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = y0;
	if (steep)
	{
		for (int x = x0; x <= x1; ++x)
		{
			set(y, x, colour);
			error2 += derror2;
			if (error2 > dx)
			{
				y += (y1 > y0 ? 1 : -1);
				error2 -= dx * 2;
			}
		}
	}
	else
	{
		for (int x = x0; x <= x1; ++x)
		{
			set(x, y, colour);
			error2 += derror2;
			if (error2 > dx)
			{
				y += (y1 > y0 ? 1 : -1);
				error2 -= dx * 2;
			}
		}
	}
	return true;
}

bool Sketch::draw_triangle(Vector2i t0, Vector2i t1, Vector2i t2, Colour colour)
{
	if (t0(1) == t1(1) && t0(1) == t2(1))
		return true;

	if (t0(1) > t1(1))
		std::swap(t0, t1);
	if (t0(1) > t2(1))
		std::swap(t0, t2);
	if (t1(1) > t2(1))
		std::swap(t1, t2);

	double m01 = double(t1(1) - t0(1)) / (t1(0) - t0(0));
	double m02 = double(t2(1) - t0(1)) / (t2(0) - t0(0));
	double m12 = double(t2(1) - t1(1)) / (t2(0) - t1(0));

	for (size_t y = t0(1); y < t1(1); y++)
	{
		int xi = y / m02 - t0(1) / m02 + t0(0);
		int xf = y / m01 - t0(1) / m01 + t0(0);

		if (xi > xf)
			std::swap(xi, xf);

		draw_line(xi, y,
							xf + 1, y, colour);
	}
	for (size_t y = t1(1); y < t2(1); y++)
	{
		int xi = y / m02 - t0(1) / m02 + t0(0);
		int xf = y / m12 - t1(1) / m12 + t1(0);

		if (xi > xf)
			std::swap(xi, xf);

		draw_line(xi, y,
							xf + 1, y, colour);
	}

	return true;
}