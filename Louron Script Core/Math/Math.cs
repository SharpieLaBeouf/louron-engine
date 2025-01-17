using System;

namespace Louron
{
    public struct LMath
    {

        public static float PerlinNoise(float x, float y)
        {
            int xi, yi;                // integer component
            float xf, yf;              // floating point component
            float xs, ys;              // s-curve function results
            float l00, l01, l10, l11;  // uninterpolated values
            float l0, l1;              // partially interpolated values

            xi = (int)x;
            xf = x - xi;
            xs = SCurve(xf);
            yi = (int)y;
            yf = y - yi;
            ys = SCurve(yf);

            l00 = SimpleNoise2D(xi, yi);
            l01 = SimpleNoise2D(xi + 1, yi);
            l10 = SimpleNoise2D(xi, yi + 1);
            l11 = SimpleNoise2D(xi + 1, yi + 1);

            l0 = Lerp(xs, l00, l01);  // interpolate across the x axis using the s curve
            l1 = Lerp(xs, l10, l11);

            return Lerp(ys, l0, l1);  // interpolate across the y axis using the s curve
        }

        public static float SCurve(float t)
        {
            return t * t * (3f - 2f * t);  // S-curve function (smoothstep)
        }

        public static float Lerp(float t, float a, float b)
        {
            return a + t * (b - a);  // Linear interpolation function
        }

        public static float SimpleNoise2D(int x, int y)
        {
            int n = x + y * 257;
            n = (n << 13) ^ n;
            return 1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f;
        }

        public static Vector3 Normalize(Vector3 v)
        {
            float length = MathF.Sqrt(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
            if (length > 0.0f)
            {
                return v / length;
            }
            return Vector3.Zero;
        }

        public static float Dot(Vector2 a, Vector2 b)
        {
            return a.X * b.X + a.Y * b.Y;
        }

        public static float Dot(Vector3 a, Vector3 b)
        {
            return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
        }

        public static float Dot(Vector4 a, Vector4 b)
        {
            return a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W;
        }

    }
}
